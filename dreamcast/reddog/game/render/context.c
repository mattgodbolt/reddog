/*
 * Operations on rendering contexts
 */

#include "..\RedDog.h"
#include "Internal.h"
#include "Rasterisers.h"

KMVERTEXCONTEXT *rLoadContext = NULL;

/* Initialise a context with some sensible default values */
void InitContext (KMVERTEXCONTEXT *context)
{
	context->RenderState		=	KM_PARAMTYPE					|	KM_LISTTYPE					|
									KM_STRIPLENGTH					|	KM_USERCLIPMODE				|
									KM_COLORTYPE					|	KM_UVFORMAT					|
									KM_DEPTHMODE					|	KM_CULLINGMODE				|
									KM_SCREENCOORDINATION			|	KM_SHADINGMODE				|
									KM_MODIFIER						|	KM_ZWRITEDISABLE			|
									KM_SRCBLENDINGMODE				|	KM_DSTBLENDINGMODE			|
									KM_SRCSELECT					|	KM_DSTSELECT				|
									KM_FOGMODE						|	KM_USESPECULAR				|
									KM_USEALPHA						|	KM_IGNORETEXTUREALPHA		|
									KM_FLIPUV						|	KM_CLAMPUV					|
									KM_FILTERMODE					|	KM_SUPERSAMPLE				|
									KM_MIPMAPDADJUST				|	KM_TEXTURESHADINGMODE		|
									KM_COLORCLAMP					|	KM_PALETTEBANK				|
									KM_DCALCEXACT;

	context->ParamType			= KM_POLYGON;
	context->ListType			= KM_OPAQUE_POLYGON;
	context->ColorType			= KM_FLOATINGCOLOR;
	context->UVFormat			= KM_16BITUV;
	context->DepthMode			= KM_GREATEREQUAL;
	context->CullingMode		= KM_CULLCW;
	context->ScreenCoordination	= KM_SCREEN;
	context->ShadingMode		= KM_NOTEXTUREGOURAUD;
	context->SelectModifier		= KM_NOMODIFIER;
	context->bZWriteDisable		= FALSE;
	context->SRCBlendingMode	= KM_SRCALPHA;
	context->DSTBlendingMode	= KM_INVSRCALPHA;
	context->bSRCSel			= FALSE;
	context->bDSTSel			= FALSE;
	context->FogMode			= KM_FOGTABLE;
	context->bUseSpecular		= FALSE;
	context->bUseAlpha			= FALSE;
	context->bIgnoreTextureAlpha= TRUE;
	context->FlipUV				= KM_NOFLIP;
	context->ClampUV			= KM_NOCLAMP;
	context->FilterMode			= KM_BILINEAR;
	context->bSuperSample		= FALSE;
	context->MipMapAdjust		= 4;
	context->TextureShadingMode	= KM_MODULATE_ALPHA;
	context->bColorClamp		= TRUE;
	context->PaletteBank		= 0;
	context->pTextureSurfaceDesc= NULL;

	context->fFaceColorAlpha	= 1.f;
	context->fFaceColorRed		= 1.f;
	context->fFaceColorBlue		= 1.f;
	context->fFaceColorGreen	= 1.f;
	context->fOffsetColorAlpha	= 0.f;
	context->fOffsetColorRed	= 0.f;
	context->fOffsetColorBlue	= 0.f;
	context->fOffsetColorGreen	= 0.f;

	context->ModifierInstruction= KM_MODIFIER_NORMAL_POLY;
	context->bDCalcExact		= FALSE;
	context->StripLength		= KM_STRIP_06;
	context->UserClipMode		= KM_USERCLIP_INSIDE;

	context->fBoundingBoxXmin	=   0.0f;
	context->fBoundingBoxYmin	=   0.0f;
	context->fBoundingBoxXmax	= 639.0f;
	context->fBoundingBoxYmax	= 479.0f;
}

/*
 * The rendering contexts
 */
KMVERTEXCONTEXT		bkContext;				/* Used for plotting the background */
KMVERTEXCONTEXT		objContext;				/* Used for drawing objects */
KMVERTEXCONTEXT		scapeContext;			/* Used for drawing landscapes */
KMVERTEXCONTEXT		sfxContext;				/* Used for special effects */
KMVERTEXCONTEXT		newObjContext;			/* Used for special effects */
KMVERTEXCONTEXT		spriteContext;			/* Used for sprites */
KMVERTEXCONTEXT		shadowContext[3];		/* Used for shadows */
#pragma section Context
KMVERTEXCONTEXT		dummyContext;			/* Used for fast setting */
#pragma section

KMVERTEXCONTEXT		*currentVertexContext = NULL;	/* The currently selected context */

/* Initialise the vertex render states */
void InitContexts (void)
{
	KMSTATUS	result;
	int i;

	/* Background context */
	InitContext (&bkContext);
	bkContext.FogMode = KM_NOFOG;
	bkContext.SRCBlendingMode = KM_ONE;
	bkContext.DSTBlendingMode = KM_ZERO;
	bkContext.TextureShadingMode	= KM_MODULATE;
	bkContext.bColorClamp			= FALSE;
	result = kmProcessVertexRenderState (&bkContext);
	dAssert (result == KMSTATUS_SUCCESS, "Unable to initialise background vertex state");

	result = kmSetBackGroundRenderState (&bkContext);
	dAssert (result == KMSTATUS_SUCCESS, "Unable to set background context");

	/* Object context */
	InitContext (&objContext);
	objContext.UVFormat = KM_32BITUV;
	result = kmProcessVertexRenderState (&objContext);
	dAssert (result == KMSTATUS_SUCCESS, "Unable to initialise object vertex state");

	/* New, intensity object context */
	InitContext (&newObjContext);
	newObjContext.UVFormat = KM_32BITUV;
	newObjContext.ColorType = KM_INTENSITY;
	result = kmProcessVertexRenderState (&newObjContext);
	dAssert (result == KMSTATUS_SUCCESS, "Unable to initialise newObject vertex state");

	/* Landscape context */
	InitContext (&scapeContext);
	scapeContext.ColorType = KM_PACKEDCOLOR;
	scapeContext.StripLength = KM_STRIP_04;
	result = kmProcessVertexRenderState (&scapeContext);
	dAssert (result == KMSTATUS_SUCCESS, "Unable to initialise landscape vertex state");

	/* Special FX and sprite contexts */
	InitContext (&sfxContext);
	sfxContext.ColorType = KM_PACKEDCOLOR;
	sfxContext.UVFormat = KM_32BITUV;
	result = kmProcessVertexRenderState (&sfxContext);
	dAssert (result == KMSTATUS_SUCCESS, "Unable to initialise SFX vertex state");
	InitContext (&spriteContext);
	spriteContext.ParamType = KM_SPRITE;
	spriteContext.UVFormat = KM_16BITUV;
	result = kmProcessVertexRenderState (&spriteContext);
	dAssert (result == KMSTATUS_SUCCESS, "Unable to initialise sprite vertex state");

	/* Shadow contexts */
	for (i = 0; i < 3; ++i) {
		memset (&shadowContext[i], sizeof (shadowContext[i]), 0);
		shadowContext[i].ListType		= KM_OPAQUE_MODIFIER;
		shadowContext[i].ParamType		= KM_MODIFIERVOLUME;
		shadowContext[i].UserClipMode	= KM_USERCLIP_INSIDE;
		shadowContext[i].CullingMode	= KM_NOCULLING;
		shadowContext[i].RenderState	= KM_PARAMTYPE | KM_LISTTYPE | KM_USERCLIPMODE | KM_CULLINGMODE;
		switch (i) {
		case 0:
			shadowContext[0].ModifierInstruction = KM_MODIFIER_INCLUDE_FIRST_POLY;
			break;
		case 1:
			shadowContext[1].ModifierInstruction = KM_MODIFIER_NORMAL_POLY;
			break;
		case 2:
			shadowContext[2].ModifierInstruction = KM_MODIFIER_INCLUDE_LAST_POLY;
			break;
		}
		result = kmProcessVertexRenderState (&shadowContext[i]);
		dAssert (result == KMSTATUS_SUCCESS, "Unable to initialise shadow state");
	}
	
	/* Dummy context */
	dummyContext.RenderState = 0;
	dummyContext.ShadingMode = KM_TEXTUREGOURAUD;
	dummyContext.fOffsetColorRed = 0;
	dummyContext.fOffsetColorGreen = 0;
	dummyContext.fOffsetColorBlue = 0;
	dummyContext.fOffsetColorAlpha = 0;

}

/*
 * Prepares a context
 * Doesn't matter how slow this is now...within reason
 */
void rPrepareContext (KMVERTEXCONTEXT *context, Material *mat, Bool Pasted)
{
	register KMRENDERSTATE changed = 0;
	Bool texChanged;
	register Uint32 flags = Pasted ? MF_ALPHA | mat->pasted.flags : mat->flags;
	KMSURFACEDESC *desc = Pasted ? mat->pasted.surf.surfaceDesc : mat->surf.surfaceDesc;

	/* Check for a texture change */
	if (context->pTextureSurfaceDesc != desc ||  /* different pointers */
		(context->pTextureSurfaceDesc && desc && context->pTextureSurfaceDesc->pSurface != desc->pSurface)) /* different surfaces */
		context->pTextureSurfaceDesc = desc, texChanged = TRUE;
	else
		texChanged = FALSE;

	/* Bump mapping changes */
	if (context->pTextureSurfaceDesc && (context->pTextureSurfaceDesc->PixelFormat & KM_PIXELFORMAT_BUMP)) {
		flags &= ~(MF_IGNORE_TEXTURE_ALPHA);
		flags |= MF_ALPHA;
	}

	/* Mipmap adjust changes */
	if (context->MipMapAdjust != MF_MIPMAP_GET(flags)) {
		context->MipMapAdjust = MF_MIPMAP_GET(flags);
		changed |= KM_MIPMAPDADJUST;
	}

	if ((flags & MF_TEXTURE_FILTER_MASK)==0) {	/* no texture */
		if (flags & MF_FLAT_SHADED) {
			if (context->ShadingMode != KM_NOTEXTUREFLAT)
				context->ShadingMode = KM_NOTEXTUREFLAT, changed |= KM_SHADINGMODE;
		} else {
			if (context->ShadingMode != KM_NOTEXTUREGOURAUD)
				context->ShadingMode = KM_NOTEXTUREGOURAUD, changed |= KM_SHADINGMODE;
		}
	} else {
		if (flags & MF_FLAT_SHADED) {
			if (context->ShadingMode != KM_TEXTUREFLAT)
				context->ShadingMode = KM_TEXTUREFLAT, changed |= KM_SHADINGMODE;
		} else {
			if (context->ShadingMode != KM_TEXTUREGOURAUD)
				context->ShadingMode = KM_TEXTUREGOURAUD, changed |= KM_SHADINGMODE;
		}
	}
		
	/* Check the twosidedness */
	if (flags & MF_TWO_SIDED) {
		if (context->CullingMode != KM_NOCULLING)
			context->CullingMode = KM_NOCULLING, changed |= KM_CULLINGMODE;
	} else {
		if (context->CullingMode != KM_CULLCW)
			context->CullingMode = KM_CULLCW, changed |= KM_CULLINGMODE;
	}

	/* Check the transparency type */
	if (flags & MF_TRANSPARENCY_ADDITIVE) {
		flags |= MF_ALPHA | MF_IGNORE_TEXTURE_ALPHA;
		/* Additive: Use KM_SRCALPHA, KM_ONE unless trilinear pass two*/
		if (flags & (MF_SECOND_PASS)) {
			if (context->SRCBlendingMode != KM_ONE)
				context->SRCBlendingMode = KM_ONE, changed |= KM_SRCBLENDINGMODE;
		} else {
			if (context->SRCBlendingMode != KM_SRCALPHA)
				context->SRCBlendingMode = KM_SRCALPHA, changed |= KM_SRCBLENDINGMODE;
		}
		if (context->DSTBlendingMode != KM_ONE)
			context->DSTBlendingMode = KM_ONE, changed |= KM_DSTBLENDINGMODE;
		if (context->FogMode != KM_NOFOG)
			context->FogMode = KM_NOFOG, changed |= KM_FOGMODE;
	} else if (flags & MF_TRANSPARENCY_SUBTRACTIVE) {
		/* Subtractive: Use KM_ZERO, KM_INVSRCCOLOR */
		flags |= MF_ALPHA | MF_IGNORE_TEXTURE_ALPHA;
		if (context->SRCBlendingMode != KM_ZERO)
			context->SRCBlendingMode = KM_ZERO, changed |= KM_SRCBLENDINGMODE;
		if (context->DSTBlendingMode != KM_INVSRCCOLOR)
			context->DSTBlendingMode = KM_INVSRCCOLOR, changed |= KM_DSTBLENDINGMODE;
		if (context->FogMode != KM_NOFOG)
			context->FogMode = KM_NOFOG, changed |= KM_FOGMODE;
	} else if (flags & MF_TRANSPARENCY_FROM_SCREEN) {
		// Pick up alpha values from the screen: Use KM_INVDESTALPHA, KM_DESTALPHA
		if (context->SRCBlendingMode != KM_INVDESTALPHA)
			context->SRCBlendingMode = KM_INVDESTALPHA, changed |= KM_SRCBLENDINGMODE;
		if (context->DSTBlendingMode != KM_DESTALPHA)
			context->DSTBlendingMode = KM_DESTALPHA, changed |= KM_DSTBLENDINGMODE;
	} else {
		if (flags & (MF_ALPHA | MF_TRANSPARENCY_PUNCHTHRU)) {
			/* Normal transparency: Use KM_SRCALPHA, KM_INVSRCALPHA */
			if (context->SRCBlendingMode != KM_SRCALPHA)
				context->SRCBlendingMode = KM_SRCALPHA, changed |= KM_SRCBLENDINGMODE;
			if (context->DSTBlendingMode != KM_INVSRCALPHA)
				context->DSTBlendingMode = KM_INVSRCALPHA, changed |= KM_DSTBLENDINGMODE;
		} else {
			// No transparency: Use KM_ONE, KM_ZERO 
			if (context->SRCBlendingMode != KM_ONE)
				context->SRCBlendingMode = KM_ONE, changed |= KM_SRCBLENDINGMODE;
			if (context->DSTBlendingMode != KM_ZERO)
				context->DSTBlendingMode = KM_ZERO, changed |= KM_DSTBLENDINGMODE;
			flags &= ~MF_IGNORE_TEXTURE_ALPHA;
		}
		if (context->FogMode != KM_FOGTABLE)
			context->FogMode = KM_FOGTABLE, changed |= KM_FOGMODE;
	}

	// Colour clamping
	if (flags & (MF_TRANSPARENCY_ADDITIVE | MF_TRANSPARENCY_SUBTRACTIVE | MF_FRONT_END)) {
		if (context->bColorClamp == TRUE)
			context->bColorClamp = FALSE, changed |= KM_COLORCLAMP;
	} else {
		if (context->bColorClamp == FALSE)
			context->bColorClamp = TRUE, changed |= KM_COLORCLAMP;
	}

	/* Check the texture clamping type */
	if (flags & (MF_CLAMP_U|MF_CLAMP_V)) {
		int bar = 0;
		if (flags & MF_CLAMP_V)
			bar = 1;
		if (flags & MF_CLAMP_U)
			bar |= 2;
		if (context->ClampUV != bar)
			context->ClampUV = bar, changed |= KM_CLAMPUV;
	} else {
		if (context->ClampUV != KM_NOCLAMP)
			context->ClampUV = KM_NOCLAMP, changed |= KM_CLAMPUV;
	}

	/* Check for alpha textures */
	if (flags & (MF_ALPHA | MF_TRANSPARENCY_PUNCHTHRU)) {
		if (context->bUseAlpha == FALSE)
			context->bUseAlpha = TRUE, changed |= KM_USEALPHA;
		if (context->bZWriteDisable != TRUE)
			context->bZWriteDisable = TRUE, changed |= KM_ZWRITEDISABLE;
		/* Check for punch-thru DISABLED! */
/*		if (flags & MF_TRANSPARENCY_PUNCHTHRU) {
			if (context->ListType != KM_PUNCHTHROUGH_POLYGON)
				context->ListType = KM_PUNCHTHROUGH_POLYGON, changed |= KM_LISTTYPE;
		} else */{
			if (context->ListType != KM_TRANS_POLYGON)
				context->ListType = KM_TRANS_POLYGON, changed |= KM_LISTTYPE;
		}
		if (context->SelectModifier != KM_NOMODIFIER)
			context->SelectModifier = KM_NOMODIFIER, changed |= KM_MODIFIER;
	} else {
		if (context->bUseAlpha == TRUE)
			context->bUseAlpha = FALSE, changed |= KM_USEALPHA;
		if (context->ListType != KM_OPAQUE_POLYGON)
			context->ListType = KM_OPAQUE_POLYGON, changed |= KM_LISTTYPE;
		if (context->SelectModifier != KM_MODIFIER_A)
			context->SelectModifier = KM_MODIFIER_A, changed |= KM_MODIFIER;
		if (context->bZWriteDisable != FALSE)
			context->bZWriteDisable = FALSE, changed |= KM_ZWRITEDISABLE;
	}

	/* Check for ignoring texture's alpha */
	if (flags & MF_IGNORE_TEXTURE_ALPHA ) {
		if (context->bIgnoreTextureAlpha == FALSE)
			context->bIgnoreTextureAlpha = TRUE, changed |= KM_IGNORETEXTUREALPHA;
	} else {
		if (context->bIgnoreTextureAlpha == TRUE)
			context->bIgnoreTextureAlpha = FALSE, changed |= KM_IGNORETEXTUREALPHA;
	}
	
	/* Check the filter status */
	switch ((flags & MF_TEXTURE_FILTER_MASK)) {
	case MF_TEXTURE_NO_FILTER:
		if (context->FilterMode != KM_POINT_SAMPLE)
			context->FilterMode = KM_POINT_SAMPLE, changed |= KM_FILTERMODE;
		break;
	case MF_TEXTURE_BILINEAR:
		if (context->FilterMode != KM_BILINEAR)
			context->FilterMode = KM_BILINEAR, changed |= KM_FILTERMODE;
		break;
	case MF_TEXTURE_TRILINEAR:
		// Trilinear is a two-pass operation
		if (flags & MF_SECOND_PASS) {
			// Second pass is the transparent pass
			if (context->FilterMode != KM_TRILINEAR_B)
				context->FilterMode = KM_TRILINEAR_B, changed |= KM_FILTERMODE;
		} else {
			if (context->FilterMode != KM_TRILINEAR_A)
				context->FilterMode = KM_TRILINEAR_A, changed |= KM_FILTERMODE;
		}
		break;
		
	}

	/* Check anisotropic bit */
	if (flags & MF_ANISOTROPIC) {
		if (context->bSuperSample != TRUE)
			context->bSuperSample = TRUE, changed |= KM_SUPERSAMPLE;
	} else {
		if (context->bSuperSample != FALSE)
			context->bSuperSample = FALSE, changed |= KM_SUPERSAMPLE;
	}

	/* Check the decal bit */
	if (flags & MF_DECAL) {
		if (context->TextureShadingMode != KM_DECAL_ALPHA)
			context->TextureShadingMode = KM_DECAL_ALPHA, changed |= KM_TEXTURESHADINGMODE;
	} else {
		if (context->TextureShadingMode != KM_MODULATE_ALPHA)
			context->TextureShadingMode = KM_MODULATE_ALPHA, changed |= KM_TEXTURESHADINGMODE;
	}

	// Cheesemaster
	if (context == &scapeContext &&
		(flags & MF_ALPHA))
		flags |= MF_NO_LIGHTING;

	/* Check specular status */
	if (!(flags & MF_NO_LIGHTING) && (flags & MF_TEXTURE_FILTER_MASK)) {
		if (context->bUseSpecular != TRUE)
			context->bUseSpecular = TRUE, changed |= KM_USESPECULAR;
	} else {
		if (context->bUseSpecular != FALSE)
			context->bUseSpecular = FALSE, changed |= KM_USESPECULAR;
	}

	if (context->PaletteBank != MF_PALETTE_GET(flags)*16) {
		context->PaletteBank = MF_PALETTE_GET(flags)*16;
		changed |= KM_PALETTEBANK;
	}

	if (flags & MF_FRONT_END) {
		if (context->SelectModifier != KM_NOMODIFIER) {
			context->SelectModifier = KM_NOMODIFIER, changed |= KM_MODIFIER;
		}
//		if (context->UserClipMode != KM_USERCLIP_DISABLE)
//			context->UserClipMode = KM_USERCLIP_DISABLE, changed |= KM_USERCLIPMODE;
	} else {
		if (context->UserClipMode != KM_USERCLIP_INSIDE)
			context->UserClipMode = KM_USERCLIP_INSIDE, changed |= KM_USERCLIPMODE;
	}

	if (changed || texChanged) {
		KMSTATUS result;

		currentVertexContext = NULL; /* Invalidate current state */

		context->RenderState = changed;
		result = kmProcessVertexRenderState (context);
		dAssert (result == KMSTATUS_SUCCESS, "Unable to initialise vertex state");
	}
}

/*
 * Sets the current context, if necessary
 */
void rSetContext (KMVERTEXCONTEXT *context)
{
	KMSTATUS			result;

	if (currentVertexContext != context) {
		result = kmSetVertexRenderState (context);
		dAssert (result == KMSTATUS_SUCCESS, "Unable to set vertex render state");
		currentVertexContext = context;
	}
}

/*
 * Loads a material from a given stream
 */
void rLoadMaterial (Material *mat, Stream *s)
{
	dAssert (mat != NULL && s != NULL, "Invalid parms");
	/*
	 * Read the material structure in
	 */
	sRead (mat, sizeof (Material), s);
	rFixUpMaterial (mat, rLoadContext);
	/* And that's it! */
	// apart from this patch:
	if (!(mat->flags & MF_TEXTURE_PASTE))
		mat->flags &= ~(MF_ENVIRONMENT);
}

/*
 * All of the rasterisers are noted here
 */
#define REND_STANDARD (MF_BUMP_MAPPED | MF_NO_LIGHTING | MF_FLAT_SHADED | MF_TEXTURE_PASTE)
struct {
	Uint32 and, eor;
	Rasteriser rTex, rnonTex;
} rasterList[] = 
{ 
	/* The environment map second pass */
	{
		(MF_TEXTURE_PASTE | MF_ENVIRONMENT), (MF_TEXTURE_PASTE | MF_ENVIRONMENT),
		{ texturedStripRasteriser, texturedStripRasteriserClipped,
		  envMStripDraw, texMStripDrawC }, 
		{ nontexturedStripRasteriser, nontexturedStripRasteriserClipped,
		  MStripDrawU, MStripDrawCU }, 			
	},
	/* The unlit rasteriser */
	{
		MF_NO_LIGHTING, MF_NO_LIGHTING,
		{ texturedStripRasteriser, texturedStripRasteriserClipped,
		  texMStripDrawU, texMStripDrawCU }, 
		{ nontexturedStripRasteriser, nontexturedStripRasteriserClipped,
		  MStripDrawU, MStripDrawCU }, 			
	},
	/* The standard rasteriser */
	{
		0, 0,
		{ texturedStripRasteriser, texturedStripRasteriserClipped,
		  texMStripDraw, texMStripDrawC }, 
		{ nontexturedStripRasteriser, nontexturedStripRasteriserClipped,
		  MStripDraw, MStripDrawC }, 
	}
};

#define nRasterisers (asize (rasterList))

/*
 * Find the appropriate renderer for a given material
 */
static void *rFindRenderer (Material *mat, float transparency, Uint32 flags, KMVERTEXCONTEXT *baseContext)
{
	int i;
	for (i = 0; i < nRasterisers; ++i) {
		Uint32 f = (flags & rasterList[i].and) ^ rasterList[i].eor;
		if (f == 0) {
			if (flags & MF_TEXTURE_FILTER_MASK)
				return (void *)&rasterList[i].rTex;
			else
				return (void *)&rasterList[i].rnonTex;
		}
	}
}

/*
 * Fix up a material after loading all textures
 */
void rFixUpMaterial (Material *mat, KMVERTEXCONTEXT *baseContext)
{
	extern int rLastFoundAnimated;

	if (MF_MIPMAP_GET(mat->flags) == 0)
		mat->flags |= MF_MIPMAP_SET(4);
	if (MF_MIPMAP_GET(mat->pasted.flags) == 0)
		mat->pasted.flags |= MF_MIPMAP_SET(4);

	// No lighting on invisible polygons
	if (mat->diffuse.argb.a == 0)
		mat->flags |= MF_NO_LIGHTING;

	/* Set up the MF_WHITE hint flags */
	if (mat->diffuse.argb.r > 240 &&
		mat->diffuse.argb.g > 240 &&
		mat->diffuse.argb.b > 240)
		mat->flags |= MF_WHITE;

	if (mat->diffuse.argb.b < 10 &&
		mat->diffuse.argb.g < 10 &&
		mat->diffuse.argb.r < 10) {
		static int i = 0;
		i++;
	}

	if (mat->surf.GBIX) {
		mat->surf.surfaceDesc = texFind (mat->surf.GBIX);
		if (rLastFoundAnimated)
			mat->flags |= MF_TEXTURE_ANIMATED;
	}
	mat->info.renderer = rFindRenderer (mat, (float)mat->diffuse.argb.a * (1.f/255.f), mat->flags, baseContext);
	if (mat->pasted.surf.GBIX) {
		mat->pasted.surf.surfaceDesc = texFind (mat->pasted.surf.GBIX);
		if (rLastFoundAnimated)
			mat->flags |= MF_TEXTURE_ANIMATED;
		mat->pasted.info.renderer = rFindRenderer (mat, mat->pasted.transparency, (mat->flags & ~(MF_TEXTURE_PASTE | MF_ENVIRONMENT)) | MF_ALPHA, baseContext);
	}
	rPrepareContext (baseContext, mat, FALSE);
	memcpy (&mat->info, &baseContext->GLOBALPARAMBUFFER, 16);
	rPrepareContext (baseContext, mat, TRUE);
	memcpy (&mat->pasted.info, &baseContext->GLOBALPARAMBUFFER, 16);
}

#pragma section RASTERISERS
void rSetMaterial (const Material *mat)
{
#if 0
	register KMDWORD *dest	= (PKMDWORD)&dummyContext.GLOBALPARAMBUFFER,
		*src	= (PKMDWORD)&mat->info;
	*dest++ = *src++;
	*dest++ = *src++;
	*dest++ = *src++;
	if (mat->flags & MF_TEXTURE_ANIMATED) {
		*dest = (*src & 0xfff00000) | ((int)mat->surf.surfaceDesc->pSurface >>3);
	} else {
		*dest = *src;
	}
	
	rColToLVMul ((LightingValue *) &dummyContext.fFaceColorAlpha, mat->diffuse.colour, &lColour.r);
	dummyContext.fFaceColorAlpha *= rGlobalTransparency;
	kmSetVertexRenderState (&dummyContext);
	if ((mat->flags & MF_TEXTURE_FILTER_MASK) == 0) {
		int a;
		a = 0;
		a ++;
	}
	if ((mat->info.GLOBALPARAMBUFFER & 0x34) == 0x24) {
		dAssert (vertBuf.pCurrentListState->GlobalParamSize == 0x40, "erk!");
	} else {
		dAssert (vertBuf.pCurrentListState->GlobalParamSize == 0x20, "erk!");
	}
#else
	extern KMDWORD usrGlobalWork;
	register KMDWORD *dest	= &usrGlobalWork,
					 *src	= (PKMDWORD)&mat->info,
					 listType;

	*dest++ = listType = *src++;
	*dest++ = *src++;
	*dest++ = *src++;
	if (mat->flags & MF_TEXTURE_ANIMATED) {
		*dest++ = (*src & 0xfff00000) | ((int)mat->surf.surfaceDesc->pSurface >>3);
	} else {
		*dest++ = *src;
	}
	if ((listType & 0x34) == 0x24) {
		dest+=4;	// Skip madness
		rColToLVMul ((LightingValue *) dest, mat->diffuse.colour, &lColour.r);
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
	}
	vertBuf.pCurrentListState->ListType = (listType >> 24) & 7;
#endif
}

void rSetMaterialPasted (const Material *mat)
{
	extern KMDWORD usrGlobalWork;
	register KMDWORD *dest	= &usrGlobalWork,
					 *src	= (PKMDWORD)&mat->pasted.info,
					 listType;

	*dest++ = listType = *src++;
	*dest++ = *src++;
	*dest++ = *src++;
	if (mat->flags & MF_TEXTURE_ANIMATED) {
		*dest++ = (*src & 0xfff00000) | ((int)mat->pasted.surf.surfaceDesc->pSurface >>3);
	} else {
		*dest++ = *src;
	}
	if ((listType & 0x34) == 0x24) {
		dest+=4;	// Skip madness
		rColToLVMul ((LightingValue *) dest, mat->specular.colour, &lColour.r);
		*(float *)dest *= mat->pasted.transparency;
		dest += 4;
		src = (PKMDWORD)&dummyContext.fOffsetColorRed;
		*dest++ = 0.f;
		*dest++ = *src++;
		*dest++ = *src++;
		*dest++ = *src++;
		
		vertBuf.pCurrentListState->GlobalParamSize = 0x40;
	} else {
		vertBuf.pCurrentListState->GlobalParamSize = 0x20;
	}
	vertBuf.pCurrentListState->ListType = (listType >> 24) & 7;
}

/*
static void rSetMaterialPasted (const Material *mat)
{
	register KMDWORD *dest	= (PKMDWORD)&dummyContext.GLOBALPARAMBUFFER,
					 *src	= (PKMDWORD)&mat->pasted.info;
	*dest++ = *src++;
	*dest++ = *src++;
	*dest++ = *src++;
	if (mat->flags & MF_TEXTURE_ANIMATED) {
		*dest = (*src & 0xfff00000) | ((int)mat->pasted.surf.surfaceDesc->pSurface >>3);
	} else {
		*dest = *src;
	}
	rColToLVMul ((LightingValue *) &dummyContext.fFaceColorAlpha, mat->specular.colour, &lColour.r);
	dummyContext.fFaceColorAlpha *= mat->pasted.transparency;
	kmSetVertexRenderState (&dummyContext);
}
*/