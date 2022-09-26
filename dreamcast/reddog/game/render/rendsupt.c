/*
 * Renderer support functions
 */

#include "..\RedDog.h"
#include "Internal.h"
#include "Rasterisers.h"

/* LERP a colour */
#if 0
Uint32 ColLerp (Uint32 a, Uint32 b, float alpha)
{
	Uint8 r1, g1, b1;
	Uint8 r2, g2, b2;
	float R, G, B;

	r1 = (a & 0xff0000)>>16;
	r2 = (b & 0xff0000)>>16;
	g1 = (a & 0xff00)>>8;
	g2 = (b & 0xff00)>>8;
	b1 = (a & 0xff);
	b2 = (b & 0xff);

	R = LERP (r1, r2, alpha);
	G = LERP (g1, g2, alpha);
	B = LERP (b1, b2, alpha);

	return (((Uint8)R<<16) | ((Uint8)G<<8) | ((Uint8)B));
}

/* LERP a UV value */
Uint32 UVLerp (Uint32 a, Uint32 b, float alpha)
{
	union {
		Uint32	uVal;
		float	fVal;
	} ua, ub, va, vb;

	ua.uVal = a & 0xffff0000;
	ub.uVal = b & 0xffff0000;
	va.uVal = (a & 0xffff) << 16;
	vb.uVal = (b & 0xffff) << 16;

	ua.fVal += (ub.fVal-ua.fVal) * alpha;
	va.fVal += (vb.fVal-va.fVal) * alpha;

	return ((ua.uVal & 0xffff0000) | ((va.uVal & 0xffff0000)>>16));
}

/*
 * Generates a plane for the purpose of onscreen checking
 */
#define rPreparePlaneAdd(modelToScreen, row) \
	x	= modelToScreen->m[0][3]	+ modelToScreen->m[0][row];\
	y	= modelToScreen->m[1][3]	+ modelToScreen->m[1][row];\
	z	= modelToScreen->m[2][3]	+ modelToScreen->m[2][row];\
	w	= -(modelToScreen->m[3][3]  + modelToScreen->m[3][row])

#define rPreparePlaneSub(modelToScreen, row) \
	x	= modelToScreen->m[0][3] - modelToScreen->m[0][row];\
	y	= modelToScreen->m[1][3] - modelToScreen->m[1][row];\
	z	= modelToScreen->m[2][3] - modelToScreen->m[2][row];\
	w	= -(modelToScreen->m[3][3] - modelToScreen->m[3][row])

#define rOffScreen(lowPoint, hiPoint) \
	(   (x * ((x>0) ? hiPoint##X : lowPoint##X) + \
		 y * ((y>0) ? hiPoint##Y : lowPoint##Y) + \
		 z * ((z>0) ? hiPoint##Z : lowPoint##Z)) < w)

/*
 * Checks to see if a model is on-screen
 */
CLIPTYPE rOnScreen (const Matrix *modelToScreen, 
					const Point3 *lowPoint, const Point3 *hiPoint)
{
	register float x;
	register float y;
	register float z;
	register float w;
	register float lowX = lowPoint->x;
	register float lowY = lowPoint->y;
	register float lowZ = lowPoint->z;
	register float hiX = hiPoint->x;
	register float hiY = hiPoint->y;
	register float hiZ = hiPoint->z;

	Bool	accept = TRUE;
	Bool	nearClipped = FALSE;

	/* Check the near plane (Z==0 PLANE) */
	x = modelToScreen->m[0][2];
	y = modelToScreen->m[1][2];
	z = modelToScreen->m[2][2];
	w = -modelToScreen->m[3][2];
	/* Cull off the near plane */
	if (rOffScreen (low, hi))
		return CLIP_OFFSCREEN;
	/* Totally onscreen? */
	if (rOffScreen (hi, low))
		accept = FALSE, nearClipped = TRUE;

	/* Check the far plane */
	rPreparePlaneSub (modelToScreen, 2);
	/* Cull off the far plane */
	if (rOffScreen (low, hi))
		return CLIP_OFFSCREEN;
	/* Totally onscreen? */
	if (accept && rOffScreen (hi, low))
		accept = FALSE;

	/* Check the left plane */
	rPreparePlaneAdd (modelToScreen, 0);
	/* Cull off the left plane */
	if (rOffScreen (low, hi))
		return CLIP_OFFSCREEN;
	/* Totally onscreen? */
	if (accept && rOffScreen (hi, low))
		accept = FALSE;

	/* Check the right plane */
	rPreparePlaneSub (modelToScreen, 0);
	/* Cull off the right plane */
	if (rOffScreen (low, hi))
		return CLIP_OFFSCREEN;
	/* Totally onscreen? */
	if (accept && rOffScreen (hi, low))
		accept = FALSE;

	/* Check the top plane */
	rPreparePlaneAdd (modelToScreen, 1);
	/* Cull off the top plane */
	if (rOffScreen (low, hi))
		return CLIP_OFFSCREEN;
	/* Totally onscreen? */
	if (accept && rOffScreen (hi, low))
		accept = FALSE;

	/* Check the bottom plane */
	rPreparePlaneSub (modelToScreen, 1);
	/* Cull off the bottom plane */
	if (rOffScreen (low, hi))
		return CLIP_OFFSCREEN;
	/* Totally onscreen? */
	if (accept && rOffScreen (hi, low))
		accept = FALSE;

	return (accept == TRUE) ? CLIP_COMPLETELYONSCREEN : (nearClipped == TRUE) ? CLIP_NEARCLIPPED : CLIP_PARTIALLYONSCREEN;
}
#endif