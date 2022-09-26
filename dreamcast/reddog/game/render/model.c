/*
 * Model.c
 * Model handling routines
 */

#include "..\RedDog.h"
#include "..\View.h"
#include "..\Loading.h"
#include "Internal.h"
#include "Rasterisers.h"

/*
 * Bump mapped lighting array
 */
#pragma aligndata8(prepBumpArray)
//LightingValue	prepBumpArray[MAX_VERTICES];
//LightingValue   maybe[1];

// Where the lasted pasted operation ended
static PKMDWORD kmPastedStart, kmPastedEnd;

// Some prepped UV's
static UV prepUVArray[MAX_VERTICES];
static int overwrite;

extern float CurrentStratIntensity;

// The front-end light
Point3 lightPos;

/*
 * Load a model from the given stream
 */
Model *ModelLoad (Stream *s, Allocator *alloc, void *ptr)
{
	ModelHeader header;
	Model *retVal;
	int BumpNeeded;
	int nPolys, nEdges, temp;
	UV *uv;
   
	/* Set the material type accordingly */
	rLoadContext = &newObjContext;

	/* Read the header */
	sRead (&header, sizeof (header), s);
	dAssert (header.tag == MODELHEADER_TAG, "Corrupt model");

	/* Read the model itself, after allocating it */
	retVal = alloc (ptr, sizeof (Model));
	sRead (retVal, sizeof (Model), s);

	/* Read the material array */
	if (retVal->nMats) {
		Uint32 matFlags = 0, notFlags = 0xffffffff, nonTex = 0;
		retVal->material = alloc (ptr, sizeof (Material) * retVal->nMats);
		for (temp = 0; temp < retVal->nMats; ++temp) {
			rLoadMaterial (&retVal->material[temp], s);
			matFlags |= retVal->material[temp].flags;
			notFlags &= ~retVal->material[temp].flags;
			if ((retVal->material[temp].flags & MF_TEXTURE_FILTER_MASK) != MF_TEXTURE_NO_FILTER &&
				(retVal->material[temp].flags & MF_TEXTURE_FILTER_MASK) != MF_TEXTURE_BILINEAR)
				nonTex++;
		}
		if (matFlags & MF_ENVIRONMENT)
			retVal->modelFlags |= MODELFLAG_ENVIRONMENTMAP;
		if ((matFlags & (MF_ENVIRONMENT|MF_TEXTURE_PASTE|MF_ALPHA|MF_TEXTURE_ANIMATED|MF_TRANSPARENCY_PUNCHTHRU|MF_TRANSPARENCY_SUBTRACTIVE|MF_TRANSPARENCY_FROM_SCREEN|MF_SECOND_PASS|MF_NO_LIGHTING)) == 0 &&
			((notFlags & MF_WHITE) == 0) && nonTex == 0
			&& (retVal->modelFlags & (MODELFLAG_FLAG_EFFECT|MODELFLAG_SHADOW_BBOX)==0))
			retVal->modelFlags |= MODELFLAG_CHEAPDRAW;

	}

	/* Read in the vertices and vertex normal array */
	retVal->vertex			= alloc (ptr, sizeof (Point3) * retVal->nVerts);
	retVal->vertexNormal	= alloc (ptr, sizeof (Vector3) * retVal->nVerts);
	sRead (retVal->vertex, sizeof (Point3) * retVal->nVerts, s);
	sRead (retVal->vertexNormal, sizeof (Point3) * retVal->nVerts, s);

	dAssert (retVal->nVerts < MAX_VERTICES, "Too many vertices in a model!");

#ifdef _DEBUG
	{
		int i;
		for (i = 0; i < retVal->nVerts; ++i) {
			dAssert (
				retVal->vertex[i].x >= retVal->bounds.low.x &&
				retVal->vertex[i].y >= retVal->bounds.low.y &&
				retVal->vertex[i].z >= retVal->bounds.low.z &&
				retVal->vertex[i].x <= retVal->bounds.hi.x &&
				retVal->vertex[i].y <= retVal->bounds.hi.y &&
				retVal->vertex[i].z <= retVal->bounds.hi.z
			, "Oops");
		}
	}
#endif

	/* Derive some data */
	nPolys = retVal->nQuads + retVal->nTris + retVal->nStripPolys;
	nEdges = (retVal->nQuads * 4) + (retVal->nTris * 3) + (retVal->nStrips * 2) + retVal->nStripPolys;

	/* Read in the plane equations */
 	if (retVal->plane) {
		retVal->plane = alloc (ptr, sizeof (Plane) * nPolys);
		sRead (retVal->plane, nPolys * sizeof (Plane), s);
	}
	
	/* Read in the UV values */
	retVal->uv = alloc (ptr, sizeof (UV) * nEdges);
	sRead (retVal->uv, nEdges * sizeof (UV), s);

	/* Read in any bumpmap info */
	if (retVal->bump)  {
		retVal->bump = alloc (ptr, sizeof (BumpMapInfo) * retVal->nVerts);
		sRead (retVal->bump, sizeof (BumpMapInfo) * retVal->nVerts, s);
	}

	/* Finally, read in the strip info */
	temp = (int)retVal->strip;
	retVal->strip = alloc (ptr, sizeof (VertRef) * temp);
	sRead (retVal->strip, temp * sizeof (VertRef), s);

	{
		VertRef *v;
		int numStrip;
		v = retVal->strip;
		v++;
		numStrip = *v++;
		while (numStrip) {
			int i;
			for (i = 0; i < numStrip; ++i) {
				*v++ *= sizeof (Point3);
			}
			v++;
			numStrip = *v++;
		}
	}

	LoadPoll();

	return retVal;
}

/*
 * Draw a model!
 */
#pragma aligndata8(context)
ModelContext context;

Uint32 rGlobalModelFlagMask = 0xffffffff;
Uint32 rGlobalModelFlags = 0;

void rSetMaterialTrans (const Material *mat)
{
#if 1
	extern KMDWORD usrGlobalWork;
	register KMDWORD *dest	= &usrGlobalWork,
					 *src	= (PKMDWORD)&mat->info,
					 listType;

	if (mat->flags & MF_ALPHA) {
		*dest++ = listType = *src++;
		*dest++ = *src++;
		*dest++ = *src++;
	} else {
		*dest++ = listType = (*src++ | 0x02000000) & ~0x80;
		*dest++ = *src++;
		*dest++ = (*src++ & 0x00ffffff) | 0x94100000;
	}
	if (mat->flags & MF_TEXTURE_ANIMATED) {
		*dest++ = (*src & 0xfff00000) | ((int)mat->surf.surfaceDesc->pSurface >>3);
	} else {
		*dest++ = *src;
	}
	if ((listType & 0x34) == 0x24) {
		dest+=4;	// Skip madness
		rColToLVMul ((LightingValue *) dest, mat->diffuse.colour, &lColour.r);
		*(float *)dest *= rGlobalTransparency;
		dest += 4;
		src = (PKMDWORD)&dummyContext.fOffsetColorRed;
		*dest++ = 0.f;
		*dest++ = *src++;
		*dest++ = *src++;
		*dest++ = *src++;
		
		vertBuf.pCurrentListState->GlobalParamSize = 0x40;
	} else {
		vertBuf.pCurrentListState->GlobalParamSize = 0x20;
		if (!(listType & 0x04))
			rColToLVMul ((LightingValue *) dest, mat->diffuse.colour, &lColour.r);
		*(float *)dest *= rGlobalTransparency;
	}
	vertBuf.pCurrentListState->ListType = (listType >> 24) & 7;
#else
	register KMDWORD *dest	= (PKMDWORD)&dummyContext.GLOBALPARAMBUFFER,
					 *src	= (PKMDWORD)&mat->info;
	if (mat->flags & MF_ALPHA) {
		*dest++ = *src++;
		*dest++ = *src++;
		*dest++ = *src++;
	} else {
		*dest++ = (*src++ | 0x02000000) & ~0x80;
		*dest++ = *src++;
		*dest++ = (*src++ & 0x00ffffff) | 0x94100000;
	}
	if (mat->flags & MF_TEXTURE_ANIMATED) {
		*dest = (*src & 0xfff00000) | ((int)mat->surf.surfaceDesc->pSurface >>3);
	} else {
		*dest = *src;
	}
	
	rColToLVMul ((LightingValue *) &dummyContext.fFaceColorAlpha, mat->diffuse.colour, &lColour.r);
	dummyContext.fFaceColorAlpha *= rGlobalTransparency;
	kmSetVertexRenderState (&dummyContext);
#endif
}

// Mad subtractive sheeyit
void rSetMaterialTransSub (const Material *mat)
{
	register KMDWORD *dest	= (PKMDWORD)&dummyContext.GLOBALPARAMBUFFER,
					 *src	= (PKMDWORD)&mat->info;
	*dest++ = (*src++ | 0x02000000) & ~0x80;
	*dest++ = *src++;
	*dest++ = (*src++ & 0x00ffffff) | 0x8c100000; // KM_????, KM_INVSRCCOLOR

	if (mat->flags & MF_TEXTURE_ANIMATED) {
		*dest = (*src & 0xfff00000) | ((int)mat->surf.surfaceDesc->pSurface >>3);
	} else {
		*dest = *src;
	}
	
	rColToLVMul ((LightingValue *) &dummyContext.fFaceColorAlpha, mat->diffuse.colour, &lColour.r);
	dummyContext.fFaceColorAlpha *= rGlobalTransparency;
	kmSetVertexRenderState (&dummyContext);
}

/*
 * Cheese environment map a model
 */
PKMDWORD envMStripDraw (ModelContext *context, VertRef numStrip, VertRef matNum, PKMDWORD pkm, 
									   float lX, float lY, float lZ)
{
	float a, b, c, e, f, g;
	float x, y, z;
	UV *uv = context->uvArray;
	Vector3 *normalBase;
	float *pkmCurrentPtr = (float *)pkm;
	float *read, *update, *normal;
	register VertRef *strip = context->strip;

	read	= (float *)(((int)kmPastedStart & 0x1fffffff) | 0x80000000);
	update	= (float *)(((int)read & 0x1fffffff) | 0xa0000000);

	update += 4; // Point update at u/v

	a = modelToScreen.m[0][0];
	b = modelToScreen.m[1][0];
	c = modelToScreen.m[2][0];
	x = isqrt(SQR(a) + SQR(b) + SQR(c)) * 0.5f;
	e = modelToScreen.m[0][1];
	a *= x;
	f = modelToScreen.m[1][1];
	b *= x;
	g = modelToScreen.m[2][1];
	c *= x;
	x = isqrt(SQR(e) + SQR(f) + SQR(g)) * 0.5f;
	e *= x;
	normalBase = context->vertexNormal;
	f *= x;
	g *= x;

	do {
		uv += numStrip;
		while (numStrip--) {
			// Start copying the vertex data across
			*pkmCurrentPtr++ = *read++; // param
			// Now decode the vertex offset and read out the normal
			normal = (float *)((char *)normalBase + *strip++);
			*pkmCurrentPtr++ = *read++; // x
			*pkmCurrentPtr++ = *read++; // y
			*pkmCurrentPtr++ = *read++; // z
			*pkmCurrentPtr++ = *read++; // u
			x = *normal++;
			*pkmCurrentPtr++ = *read++; // v
			y = *normal++;
			*pkmCurrentPtr++ = *read++; // base
			z = *normal++;
			*pkmCurrentPtr   = *read++; // offset
			*update++ = 0.5f + x*a+y*b+z*c;
			prefetch (pkmCurrentPtr++);
			// Update the UVs of the pasted object
			*update   = 0.5f + x*e+y*f+z*g;
			update += 7; 
		}
		// Check for strip continuation
		if (*strip == matNum &&
			(numStrip = strip[1]) != 0) {
			strip += 2;
		} else
			break;
	} while (1);

	context->strip = strip;
	context->uvArray = uv;
	return (PKMDWORD) pkmCurrentPtr;
}

// Prepares a whole model's worth of UVs
void DoEMapping (Model *m)
{
	float a, b, c, e, f, g;
	float x, y, z;
	int numStrip;
	Vector3 *normalBase;
	register VertRef *strip = m->strip;
	float *out = (float *)prepUVArray, *normal;

	a = modelToScreen.m[0][0];
	b = modelToScreen.m[1][0];
	c = modelToScreen.m[2][0];
	x = isqrt(SQR(a) + SQR(b) + SQR(c)) * 0.5f;
	e = modelToScreen.m[0][1];
	a *= x;
	f = modelToScreen.m[1][1];
	b *= x;
	g = modelToScreen.m[2][1];
	c *= x;
	x = isqrt(SQR(e) + SQR(f) + SQR(g)) * 0.5f;
	e *= x;
	normalBase = m->vertexNormal;
	f *= x;
	g *= x;

	numStrip = strip[1];
	strip+=2;
	do {
		while (numStrip--) {
			normal = (float *)((char *)normalBase + *strip++);
			// Update the UVs of the pasted object
			x = *normal++;
			y = *normal++;
			z = *normal++;
			*out++ = 0.5f + x*a+y*b+z*c;
			*out++ = 0.5f + x*e+y*f+z*g;
		}
		numStrip = strip[1];
		strip+=2;
	} while (numStrip);
}

void rSetModelContext (void)
{
	context.midX = pViewMidPoint.x;
	context.midY = pViewMidPoint.y;
	context.intensityOffset = RANGE(0,CurrentStratIntensity,1);
}

#pragma section RASTERISERS
void rDrawModel (Model *model)
{
	register float lX, lY, lZ;
	register float *pRead;
	Point3 lightP;
	register VertRef *stripSuck;
	register Material *mat;
	register VertRef material;
	register Uint32 numStrip;
	ModelStripRasteriser *rasta;
	Uint32 modelFlags, onScreen, rNum, *pWrite;
	void (*matSetter)(const Material *mat) = rSetMaterial;

	// Get the model flags
	modelFlags = (model->modelFlags & rGlobalModelFlagMask) | rGlobalModelFlags;

	// Calculate modelToScreen
	mMultiplyAligned (&modelToScreen, &mCurMatrix, &mWorldToScreen);

	/* Check visibility */
	if (modelFlags & MODELFLAG_NOCLIPPING) {
		rNum = RASTER_MODELSTRIP;
	} else {
		onScreen = rOnScreen (&modelToScreen, &model->bounds.low, &model->bounds.hi);
		if (onScreen == CLIP_OFFSCREEN)
			return;
		if (onScreen == CLIP_NEARCLIPPED) {
			rNum = RASTER_MODELSTRIP_NEARCLIPPED;
			if (modelFlags & MODELFLAG_ENVIRONMENTMAP) {
				DoEMapping (model);
			}
		} else {
			rNum = RASTER_MODELSTRIP;
		}
	}

	/*
	 * Early out to check to see if we need to do some more cunning checks
	 */
	if (modelFlags & ~MODELFLAG_NOCLIPPING) {
		// Apply the water effect if necessary
		if (modelFlags & MODELFLAG_WATER_EFFECT) {
			if (modelFlags & MODELFLAG_LAVA_EFFECT) {
				rFlagModel (model);
			} else {
				rWaterModel (model);
			}
		} else if (modelFlags & MODELFLAG_LAVA_EFFECT) {
			rLavaModel (model);
		}

		if (modelFlags & MODELFLAG_GLOBALTRANS) {
			if (modelFlags & MODELFLAG_GLOBALSUBTRACTIVE)
				matSetter = rSetMaterialTransSub;
			else
				matSetter = rSetMaterialTrans;
		}

#if 0
		/* Special case front-end stuff */
		if (modelFlags & MODELFLAG_FRONTEND) {
			// Calculate the sunDir from the model position and light position
			Point3 centre;
			centre.x = (model->bounds.low.x + model->bounds.hi.x) * 0.5f;
			centre.y = (model->bounds.low.y + model->bounds.hi.y) * 0.5f;
			centre.z = (model->bounds.low.z + model->bounds.hi.z) * 0.5f;
			mPoint3Multiply3 (&centre, &centre, &mCurMatrix);
			v3Sub ((Point3 *)&sunDir, &lightPos, &centre);
		}
#endif
		
		/* 'You there, in the shadows - I want you to accompany me...' */
		if (modelFlags & MODELFLAG_SHADOW_BBOX) {
			/* 'I get around (get around round round I get around)' */
			mPush (&modelToScreen);
			if (modelFlags & MODELFLAG_SHADOW_ROUND)
				shBBoxRound (&model->bounds);
			else
				shBBox (&model->bounds);
			mPop (&modelToScreen);
		}

	} /* End of special handling */

	// Calculate the sun direction in world space
	// Note that here I cheat something chronic :
	// I ignore non-uniform scales, because frankly they hardly ever
	// are used, and when they are you generally don't notice that the
	// lighting is a bit suspicious.
	// So, I FIPR the lighting values with the transpose of the modelToWorld
	// matrix, which is near enough the inverse of the matrix, barring scale.
	// Then as l* is normalised, the scale factors shouldn't matter too much.
	if (rGlobalModelFlags & MODELFLAG_STATIC_LIGHT) {
		pRead = &sunDir.x;
	} else {
		pRead = &lightP.x;
		CalcLightPos (&mCurMatrix.m[0][0], &sunDir.x, pRead);
	}
	mLoadWithXYScale (&modelToScreen, pViewSize.x, pViewSize.y);
	lX = *pRead++;
	lY = *pRead++;
	lZ = *pRead++;

	// Count some geometry
#if COUNT_GEOMETRY
	nDrawn += model->nStripPolys;
#endif

	// Check for old format models
	dAssert (model->nTris == 0 &&
			 model->nQuads == 0, "Model needs re-exporting");

	// Fill in the model context;
	pWrite = (Uint32 *)&context.strip;
	*pWrite++ = (Uint32)(model->strip);
	*pWrite++ = (Uint32)(model->uv);
	*pWrite++ = (Uint32)(model->vertexNormal);
	*pWrite++ = (Uint32)(model->vertex);
//	context.strip = model->strip;
//	context.uvArray = model->uv;
//	context.vertexNormal = model->vertexNormal;
//	context.vertex = model->vertex;

	// Loop drawing the strips
	for (;;) {
		stripSuck = context.strip;
		
		material = *stripSuck++;
		numStrip = *stripSuck++;
		
		context.strip = stripSuck;
		
		if (numStrip == 0)
			break;
		
		// Get the matereial in question
		mat = &model->material[material];
		prefetch (stripSuck);

		// Check to see if this is a pasting operation
		if (mat->flags & MF_TEXTURE_PASTE) {
			UV *saveUv = context.uvArray;
			if ((mat->flags & MF_ENVIRONMENT) && rNum == RASTER_MODELSTRIP_NEARCLIPPED)
				context.uvArray = prepUVArray + (context.uvArray - model->uv);
			// Prepare and set the pasted material
			rSetMaterialPasted (mat);
			rasta = (ModelStripRasteriser *)mat->pasted.info.renderer->r[rNum];
			kmxxGetCurrentPtr (&vertBuf);
			kmxxStartVertexStrip (&vertBuf);
			kmPastedStart = (PKMDWORD)(((KMDWORD)(*pkmCurrentListPtr) & 0xFC000000) + ((KMDWORD)pkmCurrentPtr & 0x0FFFFFFF));
			pkmCurrentPtr = rasta (&context, numStrip, material, pkmCurrentPtr, lX, lY, lZ);
			kmPastedEnd = pkmCurrentPtr;
			kmxxReleaseCurrentPtr (&vertBuf);
			// Ensure we reset the strip pointer, and uvArray back to the start
			context.strip = stripSuck;
			context.uvArray = saveUv;
		}
		// Prepare the base material
		matSetter (mat);		
		rasta = (ModelStripRasteriser *)mat->info.renderer->r[rNum];
		kmxxGetCurrentPtr (&vertBuf);
		kmxxStartVertexStrip (&vertBuf);
		pkmCurrentPtr = rasta (&context, numStrip, material, pkmCurrentPtr, lX, lY, lZ);
		kmxxReleaseCurrentPtr (&vertBuf);
	}
}

// Simple version, no alpha or clippin' or nuffin
void rDrawModelSimple (Model *model)
{
	register float lX, lY, lZ;
	register float *pRead;
	Point3 lightP;
	register PKMDWORD pkmCurrentPtr;														
	register VertRef *stripSuck;
	register MaterialInfo *mInfo;
	VertRef material;
	Uint32 numStrip;

	// Calculate modelToScreen
	mMultiplyAligned (&modelToScreen, &mCurMatrix, &mWorldToScreen);

	// Calculate the sun direction in world space
	// Note that here I cheat something chronic :
	// I ignore non-uniform scales, because frankly they hardly ever
	// are used, and when they are you generally don't notice that the
	// lighting is a bit suspicious.
	// So, I FIPR the lighting values with the transpose of the modelToWorld
	// matrix, which is near enough the inverse of the matrix, barring scale.
	// Then as l* is normalised, the scale factors shouldn't matter too much.
	pRead = &lightP.x;
	CalcLightPos (&mCurMatrix.m[0][0], &sunDir.x, pRead);
	mLoadWithXYScale (&modelToScreen, pViewSize.x, pViewSize.y);
	lX = *pRead++;
	lY = *pRead++;
	lZ = *pRead++;

	// Count some geometry
#if COUNT_GEOMETRY
	nDrawn += model->nStripPolys;
#endif

	// Check for old format models
	dAssert (model->nTris == 0 &&
			 model->nQuads == 0, "Model needs re-exporting");

	// Fill in the model context;
	pkmCurrentPtr = (Uint32 *)&context.strip;
	*pkmCurrentPtr++ = (Uint32)(model->strip);
	*pkmCurrentPtr++ = (Uint32)(model->uv);
	*pkmCurrentPtr++ = (Uint32)(model->vertexNormal);
	*pkmCurrentPtr++ = (Uint32)(model->vertex);

	// We can get the pointer here as we know it's all opaque
	pkmCurrentPtr = (PKMDWORD)vertBuf.pCurrentPtr[0];
	*((volatile PKMDWORD)0xFF000038) = (KMDWORD)pkmCurrentPtr >> 24;
	*((volatile PKMDWORD)0xFF00003C) = (KMDWORD)pkmCurrentPtr >> 24;
	pkmCurrentPtr = (PKMDWORD)(((KMDWORD)pkmCurrentPtr & 0x03FFFFFF) | 0xE0000000);

	// Loop drawing the strips
	for (;;) {
		stripSuck = context.strip;
		
		material = *stripSuck++;
		numStrip = *stripSuck++;

		if (numStrip == 0)
			break;

		context.strip = stripSuck;
				
		// Get the matereial in question
		prefetch (stripSuck);
		mInfo = &model->material[material].info;

		// Set up the material
		pkmCurrentPtr = CopyMaterialData64 (pkmCurrentPtr, &mInfo->GLOBALPARAMBUFFER, &lColour.r);
		// Direct call to texMStripDraw as we know what kind of material this is
		pkmCurrentPtr = texMStripDraw (&context, numStrip, material, pkmCurrentPtr, lX, lY, lZ);
	}
	vertBuf.pCurrentPtr[0] = (PKMDWORD)(((KMDWORD)(vertBuf.pCurrentPtr[0]) & 0xFC000000) + ((KMDWORD)pkmCurrentPtr & 0x0FFFFFFF));
}

#pragma section

void ModelRelease (Model *model, Deallocator *dealloc, void *ptr)
{
	dAssert (model != NULL, "NULL model passed");

	if (dealloc) {
		if (model->material)
			dealloc (ptr, model->material);
		if (model->vertex)
			dealloc (ptr, model->vertex);
		if (model->vertexNormal)
			dealloc (ptr, model->vertexNormal);
		if (model->plane)
			dealloc (ptr, model->plane);
		if (model->uv)
			dealloc (ptr, model->uv);
		if (model->strip)
			dealloc (ptr, model->strip);
		dealloc (ptr, model);
	}
}
