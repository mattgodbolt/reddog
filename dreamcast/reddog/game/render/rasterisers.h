#ifndef _RASTERISERS_H
#define _RASTERISERS_H

/* 
 * Rasteriser support 
 */

extern Point		input;
extern Point		tri[3];

/*
 * Checks to see if a model is on-screen
 */
typedef enum {	
	CLIP_OFFSCREEN			= 0,
	CLIP_PARTIALLYONSCREEN	= 1,
	CLIP_COMPLETELYONSCREEN	= 2,
	CLIP_NEARCLIPPED		= 3
} CLIPTYPE;

CLIPTYPE rOnScreen (const Matrix *modelToScreen, 
					const Point3 *lowPoint, const Point3 *hiPoint);

/*
 * Declarations of all the rasterisers
 */

StripRasteriser texturedStripRasteriser;
StripRasteriser texturedStripRasteriserClipped;
StripRasteriser nontexturedStripRasteriser;
StripRasteriser nontexturedStripRasteriserClipped;

typedef PKMDWORD ModelStripRasteriser(ModelContext *context, VertRef numStrip, VertRef matNum, PKMDWORD pkm, 
									   float lX, float lY, float lZ);

ModelStripRasteriser texMStripDraw,  MStripDraw;
ModelStripRasteriser texMStripDrawC, MStripDrawC;
ModelStripRasteriser texMStripDrawU,  MStripDrawU;
ModelStripRasteriser texMStripDrawCU, MStripDrawCU;
ModelStripRasteriser envMStripDraw, envMStripDrawC;
ModelStripRasteriser SkidDraw, SkidDrawC;

/* LERP a colour */
Uint32 ColLerp (Uint32 a, Uint32 b, float alpha);
/* LERP a UV value */
Uint32 UVLerp (Uint32 a, Uint32 b, float alpha, float *U, float *V);

#endif
