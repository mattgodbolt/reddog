/*
 * ScapeModel.c
 * The C renderer for Landscape models
 */

#include "..\RedDog.h"
#include "..\View.h"
#include "Internal.h"
#include "ScapeModel.h"
#include "Rasterisers.h"
#include "..\Loading.h"
#include <sg_cache.h>

#pragma section Matrices
Point	input;
Point	tri[3];
#pragma section

#pragma section SCAPERAST

Uint32 rScapeVisMask = 0xffffffff;
static bool MirroredMatrix = FALSE;

StripRasterContext theStripRasterContext;

Uint32 GetVisMask (Map *m, Point3 *pos)
{
	register float x, y, z;
	register VisBox *box = m->visBox;
	register int nBox = m->numVisBoxes;
	register Uint32 retVal = 0;
	register float *posSuck = (float *)pos;
	register Uint32 retVal2 = 0;
	
	x = *posSuck++;
	y = *posSuck++;
	z = *posSuck++;

	for (;nBox; --nBox, ++box) {
		mLoad (&box->worldToBoxSpace);
		if (VisIsInside (x, y, z)) {
			retVal |= 1<<(box->ID) | box->canAlsoSee;
		}
	}
	return retVal;
}

/*
 * Sets this material as current, with a float UV rather than 16bit
 */
void rSetMaterialUV (const Material *mat)
{
#if 1
	extern KMDWORD usrGlobalWork;
	register KMDWORD *dest	= &usrGlobalWork,
					 *src	= (PKMDWORD)&mat->info,
					 listType;

	*dest++ = listType = *src++ & ~1;
	*dest++ = *src++ & ~0x00400000;
	*dest++ = *src++;
	if (mat->flags & MF_TEXTURE_ANIMATED) {
		*dest++ = (*src & 0xfff00000) | ((int)mat->surf.surfaceDesc->pSurface >>3);
	} else {
		*dest++ = *src;
	}
	dAssert ((listType & 0x34) != 0x24, "Problem in MattG's new stuff");
	vertBuf.pCurrentListState->GlobalParamSize = 0x20;
	vertBuf.pCurrentListState->ListType = (listType >> 24) & 7;
#else
	register KMDWORD *dest	= (PKMDWORD)&dummyContext.GLOBALPARAMBUFFER,
					 *src	= (PKMDWORD)&mat->info;
	/* Obscene hack!!!!! XXX */
	*dest++ = *src++ & ~1;
	*dest++ = *src++ & ~0x00400000;
	*dest++ = *src++;
	if (mat->flags & MF_TEXTURE_ANIMATED) {
		*dest = (*src & 0xfff00000) | ((int)mat->surf.surfaceDesc->pSurface >>3);
	} else {
		*dest = *src;
	}
	rColToLV ((LightingValue *) &dummyContext.fFaceColorAlpha, mat->diffuse.colour);
	dummyContext.fFaceColorAlpha *= mat->pasted.transparency;
	kmSetVertexRenderState (&dummyContext);
#endif
}

// float UV version of the pasted setter
void rSetMaterialPastedUV (const Material *mat)
{
#if 1
	extern KMDWORD usrGlobalWork;
	register KMDWORD *dest	= &usrGlobalWork,
					 *src	= (PKMDWORD)&mat->pasted.info,
					 listType;

	*dest++ = listType = *src++ & ~1;
	*dest++ = *src++ & ~0x00400000;
	*dest++ = *src++;
	if (mat->flags & MF_TEXTURE_ANIMATED) {
		*dest++ = (*src & 0xfff00000) | ((int)mat->pasted.surf.surfaceDesc->pSurface >>3);
	} else {
		*dest++ = *src;
	}
	dAssert ((listType & 0x34) != 0x24, "Problem in MattG's new stuff");
	vertBuf.pCurrentListState->GlobalParamSize = 0x20;
	vertBuf.pCurrentListState->ListType = (listType >> 24) & 7;
#else
	register KMDWORD *dest	= (PKMDWORD)&dummyContext.GLOBALPARAMBUFFER,
					 *src	= (PKMDWORD)&mat->pasted.info;
	/* Obscene hack!!!!! XXX */
	*dest++ = *src++ & ~1;
	*dest++ = *src++ & ~0x00400000;
	*dest++ = *src++;
	if (mat->flags & MF_TEXTURE_ANIMATED) {
		*dest = (*src & 0xfff00000) | ((int)mat->pasted.surf.surfaceDesc->pSurface >>3);
	} else {
		*dest = *src;
	}
	rColToLV ((LightingValue *) &dummyContext.fFaceColorAlpha, mat->diffuse.colour);
	dummyContext.fFaceColorAlpha *= mat->pasted.transparency;
	kmSetVertexRenderState (&dummyContext);
#endif
}

#if COUNT_GEOMETRY
int nBoxesDrawn, nBoxesClipped;
#endif

// Processes a strip list and generates environment-mapped scape data
#define MAX_EMAPPED_STRIPS	512
#define PACK_UV(fU, fV)  ((*((Uint32 *) &fU) & 0xFFFF0000) | (*((Uint32 *) &fV) >> 16))
static StripEntry eMappedStripBuffer[MAX_EMAPPED_STRIPS];
StripEntry *DoScapeEMapping (StripPos *vertArray, StripEntry *drawFrom, int nStrip, register int mat)
{
	register int i = nStrip, v, num;
	register float *pullIt;
	register float invSqr;
	register StripEntry *out = eMappedStripBuffer;
	Point p;
	register float cX, cY, cZ, x, y, z, s;
	float xx, yy;
	float prevU[2];
#ifdef _DEBUG
	float debugU[3];
#endif

	cX = currentCamera->pos.x;
	cY = currentCamera->pos.y;
	cZ = currentCamera->pos.z;

	for (;;) {
		num = 0;
		while (i--) {
			out->vertRef = v = drawFrom->vertRef;
			pullIt = (float *)((char *)&vertArray[0] + ((int)v<<2));
			x = *pullIt++ - cX;
			y = *pullIt++ - cY;
			z = *pullIt++ - cZ;
			s = isqrt (SQR(x)+SQR(y)+SQR(z));

			xx = 1.f - (fatan2(x*s, y*s));
			yy = 1.f + z*s;

			// Awkward, awkward bloody international dateline problem
			// I only fix problems in U
			switch (num) {
			case 0:
				break;
			case 1:
				// Check to see if we're crossing the break
				if (fabs (xx - prevU[1]) > 0.5f) {
					// We're crossing the break - adjust xx accordingly
					if (xx > prevU[1])
						xx -= 1.f;
					else
						xx += 1.f;
				}
				break;
			default:
				// Check to see if we're crossing the break with the previous point
				if (fabs (xx - prevU[1]) > 0.5f) {
					// Adjust so we're within the same quadrant as 
					// We're crossing the break - adjust xx accordingly
					if (xx > prevU[1])
						xx -= 1.f;
					else
						xx += 1.f;
				} else if (fabs (xx - prevU[0]) > 0.5f) { // Check the line 2 -- 0
					if (xx > prevU[0])
						xx -= 1.f;
					else
						xx += 1.f;
				}
				
			}


			out->uv = PACK_UV(xx, yy);
#ifdef _DEBUG
			debugU[0] = debugU[1];
			debugU[1] = debugU[2];
			debugU[2] = xx;
#endif
			prevU[0] = prevU[1]; prevU[1] = xx;
			out++;
			drawFrom++;
			num++;
		}
		dAssert (out - eMappedStripBuffer < MAX_EMAPPED_STRIPS, "Too many emapped polygons at once");
		// Check for continuation
		i = *(Uint32 *)drawFrom;
		if (i==0)
			break;
		if (*((Uint32 *)drawFrom + 1) != mat)
			break;
		*out++ = *drawFrom++;
	}

	*(Uint32 *)out = 0; // terminate the list!!!!

	return eMappedStripBuffer;
}

static StripHeader *SkipStrip (register StripHeader *head)
{
	register int matNum = head->material, numStrip = head->nStrip;
	do {
		head += (numStrip+1);
		numStrip = head->nStrip;
		if (matNum != head->material)
			break;
	} while (numStrip);

	return head;
}

static Uint32 visMask;

static void rDrawScapeModelCOS (ScapeModel *model, Material *matList)
{
	register StripRasteriser *rasta;
	register Uint32 *header;
	register StripPos *vertArray = model->vertex;

	// Get that badger into the cache
	prefetch (header = (Uint32 *)(((Uint32)model->strip) | 0x80000000));

	dAssert (MirroredMatrix, "Erk!");

	do {
		StripEntry *drawFrom;
		Uint32 nStrip = *header++;
		Uint32 matNum = *header++;
		Material *mat;
		if (nStrip==0)
			break;
		
		/* Prepare the material */
		mat = &matList[matNum];
		/* Check for visibility */
		if (!(mat->hideID & rScapeVisMask)) {
			header = (Uint32 *)SkipStrip ((StripHeader *)(header-2));
			continue;
		}
		// Set up the context
		theStripRasterContext.mat = matNum;
		rSetMaterial (mat);
		
		prefetch (header);
		
		/* Now loop for every poly in the strip */
		kmxxGetCurrentPtr (&vertBuf);
		rasta = (StripRasteriser *)mat->info.renderer->r[RASTER_STRIP];				
		kmxxStartVertexStrip (&vertBuf);
		
		theStripRasterContext.pkmCurrentPtr = pkmCurrentPtr;
		drawFrom = (StripEntry *)(header);
		header = (Uint32 *)rasta(vertArray, drawFrom, nStrip, &theStripRasterContext);
		pkmCurrentPtr = theStripRasterContext.pkmCurrentPtr;
		kmxxReleaseCurrentPtr (&vertBuf);			
		
		// Pasted pass
		if (mat->flags & MF_TEXTURE_PASTE) {
			rSetMaterialPasted (mat);
			// Do we need to process the environment map?
			if (mat->flags & MF_ENVIRONMENT) {
				drawFrom = DoScapeEMapping (vertArray, drawFrom, nStrip, matNum);
			}
			kmxxGetCurrentPtr (&vertBuf);
			kmxxStartVertexStrip (&vertBuf);
			theStripRasterContext.pkmCurrentPtr = pkmCurrentPtr;
			(void) rasta(vertArray, drawFrom, nStrip, &theStripRasterContext);
			pkmCurrentPtr = theStripRasterContext.pkmCurrentPtr;
			kmxxReleaseCurrentPtr (&vertBuf);			
		}
		
	} while (1);
}

void rDrawScapeModelSimpleCOS (ScapeModel *model, register Material *matList)
{
	register StripRasteriser *rasta;
	register Uint32 *header;
	register MaterialInfo *mInfo;
	register Uint32 nStrip, matNum;
	register StripPos *vertArray = model->vertex;
	register PKMDWORD pkmCurrentPtr;

	dAssert (MirroredMatrix, "Erk!");

	// Get that badger into the cache
	prefetch (header = (Uint32 *)(((Uint32)model->strip) | 0x80000000));

	// We can get the pointer here as we know it's all opaque
	pkmCurrentPtr = (PKMDWORD)vertBuf.pCurrentPtr[0];
	*((volatile PKMDWORD)0xFF000038) = (KMDWORD)pkmCurrentPtr >> 24;
	*((volatile PKMDWORD)0xFF00003C) = (KMDWORD)pkmCurrentPtr >> 24;
	pkmCurrentPtr = (PKMDWORD)(((KMDWORD)pkmCurrentPtr & 0x03FFFFFF) | 0xE0000000);

	nStrip = *header++;
	matNum = *header++;
	while (nStrip) {
		prefetch (header);
		/* Prepare the material */
		theStripRasterContext.mat = matNum;
		mInfo = &matList[matNum].info;
		pkmCurrentPtr = CopyMaterialData32 (pkmCurrentPtr, &mInfo->GLOBALPARAMBUFFER);
		
		/* Now loop for every poly in the strip */
		rasta = (StripRasteriser *)mInfo->renderer->r[RASTER_STRIP];				
		theStripRasterContext.pkmCurrentPtr = pkmCurrentPtr;
		header = (Uint32 *)rasta(vertArray, (StripEntry *)(header), nStrip, &theStripRasterContext);
		pkmCurrentPtr = theStripRasterContext.pkmCurrentPtr;

		nStrip = *header++;
		matNum = *header++;
	}
	vertBuf.pCurrentPtr[0] = (PKMDWORD)(((KMDWORD)(vertBuf.pCurrentPtr[0]) & 0xFC000000) + ((KMDWORD)pkmCurrentPtr & 0x0FFFFFFF));
}

static void rDrawScapeModelClipped (ScapeModel *model, Material *matList)
{
	register StripRasteriser *rasta;
	register StripHeader *header;
	register StripPos *vertArray = model->vertex;

	// Get that badger into the cache
	prefetch (header = (StripHeader *)(((Uint32)model->strip) | 0x80000000));
//	dAssert (header->nStrip, "Broken scape model");

	dAssert (!MirroredMatrix, "Erk!");

	do {
		StripEntry *drawFrom;
		Uint32 nStrip = header->nStrip, matNum = header->material;
		Material *mat;
		if (nStrip==0)
			break;
		
		/* Prepare the material */
		mat = &matList[matNum];
		/* Check for visibility */
		if (!(mat->hideID & rScapeVisMask)) {
			header = SkipStrip (header);
			continue;
		}
		rSetMaterialUV (mat);
		theStripRasterContext.mat = matNum;
		
		prefetch (header+1);
		
		/* Now loop for every poly in the strip */
		kmxxGetCurrentPtr (&vertBuf);
		rasta = (StripRasteriser *)mat->info.renderer->r[RASTER_CLIPPEDSTRIP];				
		kmxxStartVertexStrip (&vertBuf);
		
		theStripRasterContext.pkmCurrentPtr = pkmCurrentPtr;
		drawFrom = (StripEntry *)(header + 1);
		header = rasta(vertArray, drawFrom, nStrip, &theStripRasterContext);
		pkmCurrentPtr = theStripRasterContext.pkmCurrentPtr;
		kmxxReleaseCurrentPtr (&vertBuf);			
		
		// Pasted pass
		if (mat->flags & MF_TEXTURE_PASTE) {
			rSetMaterialPastedUV (mat);
			// Do we need to process the environment map?
			if (mat->flags & MF_ENVIRONMENT) {
				drawFrom = DoScapeEMapping (vertArray, drawFrom, nStrip, matNum);
			}
			kmxxGetCurrentPtr (&vertBuf);
			kmxxStartVertexStrip (&vertBuf);
			theStripRasterContext.pkmCurrentPtr = pkmCurrentPtr;
			(void) rasta(vertArray, drawFrom, nStrip, &theStripRasterContext);
			pkmCurrentPtr = theStripRasterContext.pkmCurrentPtr;
			kmxxReleaseCurrentPtr (&vertBuf);			
		}
		
	} while (1);
}

static void rDrawTreeCOS (BoxTree *tree, Material *matList)
{

	// Is it a leaf?
	if (tree->leaf.tag == NULL) {
		// It's a leaf (ie model)
		ScapeModel *model = tree->leaf.model;
		/* Is the box included in the visible box list? */
		if (model->visGroup & visMask) {
			lLightVertices (model);
			// Now check to see if we can call the ultra-fast routine
			if ((Uint32)model->strip & 0x80000000)
				rDrawScapeModelCOS (model, matList);
			else if (rScapeVisMask & 1)
				rDrawScapeModelSimpleCOS (model, matList);
		}

#if COUNT_GEOMETRY
		nBoxesDrawn++;
#endif
	} else {
		register int i;
		for (i = 0; i < 4; ++i) {
			if (tree->node.child[i])
				rDrawTreeCOS(tree->node.child[i], matList);
		}
	}			
}

static void rDrawTree (BoxTree *tree, Material *matList)
{
	// Is it a leaf?
	if (tree->leaf.tag == NULL) {
		// It's a leaf (ie model)
		ScapeModel *model = tree->leaf.model;
		int onScreen;
		/* Is the box included in the visible box list? */
		if (model->visGroup & visMask) {
			onScreen = rOnScreen (&modelToScreen, &model->bounds.low, &model->bounds.hi);
			if (onScreen == CLIP_OFFSCREEN)
				return;
			
#if COUNT_GEOMETRY
			nBoxesDrawn++;
#endif
			
			/* Now invoke the appropriate rasterisers for the first material */
#if COUNT_GEOMETRY
			if (onScreen == CLIP_NEARCLIPPED)
				nBoxesClipped++;
#endif
			lLightVertices (model);
			if (onScreen==CLIP_NEARCLIPPED) {
				if (MirroredMatrix) {
					mLoad (&modelToScreen);
					MirroredMatrix = FALSE;
				}
				rDrawScapeModelClipped (model, matList);
			} else {
				if (!MirroredMatrix) {
					mLoadWithXYScaleMirrored (&modelToScreen, pViewSize.x, pViewSize.y);
					MirroredMatrix = TRUE;
				}
				if ((Uint32)model->strip & 0x80000000)
					rDrawScapeModelCOS (model, matList);
				else if (rScapeVisMask & 1)
					rDrawScapeModelSimpleCOS (model, matList);
			}
		}
	} else {
		// Its a node
		int onScreen = rOnScreen (&modelToScreen, &tree->node.bounds.low, &tree->node.bounds.hi);
		register int i;
		switch (onScreen) {
		case CLIP_OFFSCREEN:
			// Offscreen - no need to recurse down
			return;
		case CLIP_COMPLETELYONSCREEN:
			// Totally onscreen - recurse down in rDrawTreeCOS
			if (!MirroredMatrix) {
				mLoadWithXYScaleMirrored (&modelToScreen, pViewSize.x, pViewSize.y);
				MirroredMatrix = TRUE;
			}
			for (i = 0; i < 4; ++i)
				if (tree->node.child[i])
					rDrawTreeCOS(tree->node.child[i], matList);
			break;
		default:
			// Partially onscreen...carry on as normal
			for (i = 0; i < 4; ++i)
				if (tree->node.child[i])
					rDrawTree(tree->node.child[i], matList);
			break;
		}
	}
}

// Draws the scape
void rDrawScape (struct _MAP *m, Point3 *camPos)
{
	KMSTATUS	result;
	register StripEntry	*strip;
	int			onScreen;
	StripRasteriser	*rasta;
	StripHeader *header;
	Uint32		rNum;
	Uint32		curMat;
	ScapeModel	*model;
	register Material	*matList;
	int			modNum;

#if COUNT_GEOMETRY
	int nDrawnBefore = nDrawn;
	nBoxesDrawn = nBoxesClipped = 0;
#endif

	/* Set up the context */
	theStripRasterContext.midX = pViewMidPoint.x;
	theStripRasterContext.midY = pViewMidPoint.y;
	theStripRasterContext.scaleX = pViewSize.x;
	theStripRasterContext.scaleY = pViewSize.y;
	theStripRasterContext.rWnearVal = rWnearVal;

	model = m->box;
	matList = m->material;

	/* Calculate the visibility mask */
	visMask = GetVisMask (m, camPos);

	/* Ensure modelToScreen matrix is loaded into XMTRX */
	mLoad (&mWorldToScreen);
	mMul (&modelToScreen, &mCurMatrix);
	mLoadWithXYScaleMirrored (&modelToScreen, pViewSize.x, pViewSize.y);
	MirroredMatrix = TRUE;

	rDrawTree (m->hierarchy, matList);
#if COUNT_GEOMETRY
	nScape = nDrawn - nDrawnBefore;
#endif
}

#pragma section

/*
 * Loads a scapemodel
 * Memory allocated from the given allocation routines
 */
ScapeModel *rLoadScapeModel (ScapeModel *retVal, Stream *s, Allocator *alloc, void *ptr)
{
	Uint32 nBytes;
	dAssert (alloc != NULL && s != NULL, "Silly parms");

	if (retVal == NULL)
		retVal = alloc (ptr, sizeof (ScapeModel));
	sRead (retVal, sizeof (ScapeModel), s);
	nBytes = (Uint32) retVal->strip;
#ifdef VERBOSE
	dLog ("* Loading scape model with dataSize = 0x%x\n", nBytes);
#endif
	/* Load in both datasets at once */
	retVal->strip	= alloc (ptr, nBytes);
	sRead (retVal->strip, nBytes, s);

	// Load in the new vertex information
	nBytes = (int)retVal->vertex;
	retVal->vertex = alloc(ptr, nBytes + 4);
	if ((int)retVal->vertex & 7)
		retVal->vertex = (StripPos *)((char *)retVal->vertex + 4);
	sRead (retVal->vertex, nBytes, s);

	LoadPoll();

	return retVal;
}
