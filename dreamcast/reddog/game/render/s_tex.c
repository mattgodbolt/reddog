/*
 * Strip-type textured rasterisers
 */

#include "..\RedDog.h"
#include "Internal.h"
#include "Rasterisers.h"

#pragma section SCAPERAST

/* The C-version textured, non-clipped strip rasteriser */
#if 0
StripHeader *texturedStripRasteriser(StripPos *v, StripEntry *strip, Uint32 nStrip, Uint32 mat, PKMDWORD *pkm)
{
	KMSTATUS result;
	int strips = nStrip;
	StripHeader	*head;
	register PKMDWORD pkmCurrentPtr = *pkm;
	float dist;
	Uint32 add;

#ifdef COUNT_GEOMETRY
	nDrawn += nStrip - 2;
#endif

	for (;;) {
		nDrawn += (strips-2);
		do {
			float x, y, rW;
			StripPos *vert = (StripPos *)((Uint32)v + strip->vertRef);
			(*(Point3 *)&input) = vert->p;
			input.w = 1.f;
			mPointMirrored (&input, &input);
			
			rW = 1.f / input.w;
			x = X_MID + input.x * rW/* * X_MID*/;
			y = Y_MID + input.y * rW/* * Y_MID*/;

			dist = pSqrDist (&player[0].Player->pos, &vert->p) * (1.f / SQR(80));
			dist = 1 - RANGE(0, dist, 1);
			add = 255 * dist;
			add<<=16;

			kmxxSetVertex_4 (
				(strips==1)?KM_VERTEXPARAM_ENDOFSTRIP : KM_VERTEXPARAM_NORMAL,
				x, y, rW,
				strip->uv,
				vert->colour, add);

			strip++;
			
		} while (--strips != 0);
		
		head = (StripHeader *)strip;
		if (head->nStrip && head->material == mat) {
			strips = head->nStrip;
			strip++;
		} else
			break;
	}

	*pkm = pkmCurrentPtr;
	return head;
}
#endif

#if 0

/* 
 * The C-version textured, clipped strip rasteriser 
 */

/* Emits the clipped vertex between e0 and e1 */
#define EMIT_EDGE(e0,e1) _EMIT_EDGE(e0,e1,KM_VERTEXPARAM_NORMAL)
#define EMIT_EDGE_FINAL(e0,e1) _EMIT_EDGE(e0,e1, KM_VERTEXPARAM_ENDOFSTRIP); RESET
#define _EMIT_EDGE(e0,e1,pcw) \
	alpha = tri[e1].z / (tri[e1].z - tri[e0].z); \
	colourAcc					= ColLerp (colour[e1], colour[e0], alpha); \
	prelitAcc					= ColLerp (prelit[e1], prelit[e0], alpha); \
	X							= LERP (tri[e1].x, tri[e0].x, alpha); \
	Y							= LERP (tri[e1].y, tri[e0].y, alpha); \
	UVLerp(uv[e1], uv[e0], alpha, &U, &V); \
	X							= (X * rWnearVal * pViewSize.x) + X_MID; \
	Y							= (Y * rWnearVal * pViewSize.y) + Y_MID; \
	kmxxSetVertex_3 (pcw, X, Y, rWnearVal, U, V, colourAcc, prelitAcc)

/* Emits the non-clipped vertex e */
#define EMIT(e) _EMIT(e,KM_VERTEXPARAM_NORMAL)
#define EMIT_FINAL(e) _EMIT(e, KM_VERTEXPARAM_ENDOFSTRIP); RESET
#define _EMIT(e,pcw) \
	W							= 1.f / tri[e].w; \
	UVLerp(uv[e], 0, 0.f, &U, &V); \
	X							= (W * tri[e].x * pViewSize.x) + X_MID; \
	Y							= (W * tri[e].y * pViewSize.y) + Y_MID; \
	kmxxSetVertex_3 (pcw, X, Y, W, U, V, colour[e], prelit[e])

#define RESET \
	strip -= 2; nStrip += 2; \
	goto TopOfLoop
#define FLUSH \
	(void)0

/* Kludge to make case statements tally with my notes */
#define CLIPCASE(x) ((((x) & 1)<<2) | (((x) & 4)>>2) | (((x) & 2)))
StripHeader *texturedStripRasteriserClipped(StripPos *v, StripEntry *strip, Uint32 nStrip, StripRasterContext *context)
{
	KMSTATUS result;
	enum { ANTICLOCKWISE = 0, CLOCKWISE = 1 } parity = CLOCKWISE;
	Bool firstPoly;
	Sint8 clipped;
	Uint32 colour[3], prelit[3];
	Uint32 uv[3];
	float alpha, X, Y, W, U, V;
	Uint32 colourAcc, uvAcc, prelitAcc, mat = context->mat;
	register PKMDWORD pkmCurrentPtr = context->pkmCurrentPtr;
	register StripPos *vert;
	Uint32 *lLightBuffer = context->lightingBuffer;

#if COUNT_GEOMETRY
	nDrawn += nStrip - 2;
#endif

TopOfLoop:	
	while (nStrip >= 3) {
		/* Firstly, perspectivize the first triangle of the strip, storing the clippedness
		 * of each vertex in the clipped bitmask
		 */
		clipped = 0;
		/* Point 0 */
		vert = (StripPos *)((Uint32)v + (strip->vertRef<<2));
		(*(Point3 *)&input) = vert->p;
		input.w = 1.f;
		mPoint (&tri[0], &input);
		colour[0] = vert->colour;
		prelit[0] = lLightBuffer[strip->vertRef>>2];
		uv[0] = strip->uv;
		if (tri[0].z > 0)
			clipped |= 1;
		strip++;

		/* Point 1 */
		vert = (StripPos *)((Uint32)v + (strip->vertRef<<2));
		(*(Point3 *)&input) = vert->p;
		input.w = 1.f;
		mPoint (&tri[1], &input);
		colour[1] = vert->colour;
		prelit[1] = lLightBuffer[strip->vertRef>>2];
		uv[1] = strip->uv;
		if (tri[1].z > 0)
			clipped |= 2;
		strip++;

		nStrip -= 2;		/* We've processed nearly a whole triangle now */
		firstPoly = TRUE;

		/* Inner loop - do as many polys in this strip at once */
		while (nStrip) {
			/* Point 2 */
			vert = (StripPos *)((Uint32)v + (strip->vertRef<<2));
			(*(Point3 *)&input) = vert->p;
			input.w = 1.f;
			mPoint (&tri[2], &input);
			colour[2] = vert->colour;
			prelit[2] = lLightBuffer[strip->vertRef>>2];
			uv[2] = strip->uv;
			if (tri[2].z > 0)
				clipped |= 4;
			strip++;
			nStrip--;
			parity = 1 - parity;
			
			/* Now to make a decision on what to do about this triangle */
			switch (clipped | ((parity)?8:0)) {
				/* Anticlockwise definitions */
			case CLIPCASE(0):		/* Triangle is completely offscreen */
				FLUSH;
				break;

			case CLIPCASE(1):		/* Out Out In - beta alpha 2 */
				FLUSH;
				EMIT_EDGE (2, 0);		/* Beta */
				EMIT_EDGE (1, 2);		/* Alpha */
				EMIT_FINAL(2);			/* 2 */
				break;

			case CLIPCASE(2):		/* Out In Out - beta alpha 1*/
				FLUSH;
				EMIT_EDGE (1, 2);		/* Beta */
				EMIT_EDGE (0, 1);		/* Alpha */
				EMIT_FINAL(1);			/* 1 */
				break;

			case CLIPCASE(3):		/* Out In In - beta alpha 2 1 */
				FLUSH;
				EMIT_EDGE (2, 0);		/* Beta */
				EMIT_EDGE(0, 1);		/* Alpha */
				EMIT	  (2);			/* 2 */
				EMIT_FINAL (1);			/* 1 and EOS */
				break;

			case CLIPCASE(4):		/* In Out Out - alpha Beta 0 */
				FLUSH;
				EMIT_EDGE(0, 1);		/* Alpha */
				EMIT_EDGE(2, 0);		/* Beta */
				EMIT_FINAL(0);			/* 0 */
				break;

			case CLIPCASE(5):		/* In Out In - alpha beta 0 2*/
				FLUSH;
				EMIT_EDGE (0, 1);		/* Alpha */
				EMIT_EDGE (2, 1);		/* beta */
				EMIT (0);
				EMIT_FINAL(2);
				break;

			case CLIPCASE(6):		/* In In Out - alpha beta 1 0*/
				FLUSH;
				EMIT_EDGE(1, 2);		/* Alpha */
				EMIT_EDGE(2, 0);		/* beta */
				EMIT	  (1);			/* 1 */
				EMIT_FINAL(0);			/* 0 */
				break;

			case CLIPCASE(7):		/* In In In - [0 1] 2 */
				if (firstPoly) {
					EMIT(0);
					EMIT(1);
					firstPoly = FALSE;
				}
				EMIT_FINAL(2); /* xxx */
				break;

				/* Clockwise definitions */
#undef CLIPCASE
#define CLIPCASE(x) (((((x) & 1)<<2) | (((x) & 4)>>2) | (((x) & 2))) | 8)
			case CLIPCASE(0):		/* Triangle is completely offscreen */
				FLUSH;
				break;

			case CLIPCASE(1):		/* Out Out In - alpha beta 2 */
				FLUSH;
				EMIT_EDGE (1, 2);		/* Alpha */
				EMIT_EDGE (2, 0);		/* Beta */
				EMIT_FINAL(2);			/* 2 */
				break;

			case CLIPCASE(2):		/* Out In Out - alpha beta 1 */
				FLUSH;
				EMIT_EDGE (0, 1);		/* Alpha */
				EMIT_EDGE (1, 2);		/* Beta */
				EMIT_FINAL(1);			/* 1 */
				break;

			case CLIPCASE(3):		/* Out In In - alpha beta 1 2*/
				FLUSH;
				EMIT_EDGE (0, 1);		/* Alpha */
				EMIT_EDGE (2, 0);		/* Beta */
				EMIT(1);				/* 1 */
				EMIT_FINAL(2);			/* 2 */
				break;

			case CLIPCASE(4):		/* In Out Out - beta alpha 0 */
				FLUSH;
				EMIT_EDGE (2, 0);		/* Beta */
				EMIT_EDGE (0, 1);		/* Alpha */
				EMIT_FINAL(0);			/* 0 */
				break;

			case CLIPCASE(5):		/* In Out In - beta alpha 2 0*/
				FLUSH;
				EMIT_EDGE (2, 1);		/* beta */
				EMIT_EDGE (0, 1);		/* Alpha */
				EMIT	  (2);
				EMIT_FINAL(0);
				break;

			case CLIPCASE(6):		/* In In Out - beta alpha 0 1*/
				FLUSH;
				EMIT_EDGE(2, 0);		/* beta */
				EMIT_EDGE(1, 2);		/* Alpha */
				EMIT	  (0);			/* 0 */
				EMIT_FINAL(1);			/* 1 */
				break;

			case CLIPCASE(7):		/* In In In - [1 0] 2 */
				if (firstPoly) {
					EMIT(1);
					EMIT(0);
					firstPoly = FALSE;
				}
				EMIT_FINAL(2); /* xxx */
				break;
			} /* end case */

			/*
			 * If we got here, then the strip can be continued
			 */
			tri[0] = tri[1];
			tri[1] = tri[2];
			uv[0] = uv[1];
			uv[1] = uv[2];
			colour[0] = colour[1];
			colour[1] = colour[2];
			prelit[0] = prelit[1];
			prelit[1] = prelit[2];
			clipped>>=1;
		} /* End inner loop */
	} /* End Outer loop */

	context->pkmCurrentPtr = pkmCurrentPtr;
#if 0
	/* XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX 
	 * Frighteningly, this fixes a compiler bug! */
	{
		StripHeader *retVal = (StripHeader *)strip + nStrip;
		while ((int)retVal & 0x3);
	}
#endif
	return (StripHeader *)strip + nStrip;
}
#endif
