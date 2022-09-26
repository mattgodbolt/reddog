/*
 * VertPrep.c
 * Prepares vertices
 */

#include "..\RedDog.h"
#include "Internal.h"
#include "Rasterisers.h"

/* void rPrepCOSVertices (PrepVtx *dest, Point3 *src, int numVerts); */
#if 0
void rPrepCOSVertices (PrepVtx *dest, Point3 *src, int numVerts)
{ rPrepVertices (dest, src, numVerts); }
#endif

/*
 * Prepare vertices
 */
void rPrepVertices (PrepVtx *dest, Point3 *src, int numVerts)
{
	/* Use a store queue */
	register PrepVtx *d = (PrepVtx *)(((int)dest & 0x03ffffff) | 0xe0000000);
	Sint32 w;
	Sint16 outcode;
	float rW;

	*((volatile PKMDWORD)0xFF000038) = (KMDWORD)d >> 24;
	*((volatile PKMDWORD)0xFF00003C) = (KMDWORD)d >> 24;

	syCacheOCBI (dest, sizeof (PrepVtx) * numVerts);

	do {
		(*(Point3 *)&input) = *src++;
		input.w = 1.f;
		mPoint (&input, &input);

		dest->vX = input.x;
		dest->vY = input.y;
		dest->vZ = input.z;

		w		= (*(Sint32 *)&input.w);
		rW = 1.f / input.w;
		outcode = OC_NOT;

		if (input.z < 0.f)
			outcode ^= (OC_OFF_NEAR | OC_N_NEAR);

		dest->x		= X_MID + /*X_MID * */rW * input.x;
		dest->y		= Y_MID + /*Y_MID * */rW * input.y;
		dest->rW	= rW;
		dest->outcode = outcode;
		dest->hasBeenLit = 0;

		/* Send out the first batch to the store queue */
		prefetch ((void *)dest);

		dest->lighting.specular.a = 0.f;
		dest->lighting.specular.r = 0.f;
		dest->lighting.specular.g = 0.f;
		dest->lighting.specular.b = 0.f;

		/* Send out the second batch to the store queue */
		prefetch ((void *)&dest->lighting.diffuse.a);

		dest++;

	} while (--numVerts);
}