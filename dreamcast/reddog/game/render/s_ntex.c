/*
 * Strip-type nontextured rasterisers
 */

#include "..\RedDog.h"
#include "Internal.h"
#include "Rasterisers.h"

#pragma section SCAPERAST

/* The C-version nontextured, non-clipped strip rasteriser */
StripHeader *nontexturedStripRasteriser(StripPos *v, StripEntry *strip, Uint32 nStrip, StripRasterContext *context)
{
	KMSTATUS result;
	int strips = nStrip;
	StripHeader	*head;
	register PKMDWORD pkmCurrentPtr = context->pkmCurrentPtr;
	Uint32 mat = context->mat;

#if COUNT_GEOMETRY
	nDrawn += nStrip - 2;
#endif

	for (;;) {
#if COUNT_GEOMETRY
		nDrawn += (strips-2);
#endif
		do {
			float x, y, rW;
			StripPos *vert = (StripPos *)((Uint32)v + (strip->vertRef<<2));
			(*(Point3 *)&input) = vert->p;
			input.w = 1.f;
			mPointMirrored (&input, &input);
			
			rW = 1.f / input.w;
			x = input.x * rW/* * X_MID */+ X_MID;
			y = input.y * rW/* * Y_MID */+ Y_MID;
			
			kmxxSetVertex_0 (
				(strips==1)?KM_VERTEXPARAM_ENDOFSTRIP : KM_VERTEXPARAM_NORMAL,
				x, y, rW,
				vert->colour);

			strip++;
			
		} while (--strips != 0);
		
		head = (StripHeader *)strip;
		if (head->nStrip && head->material == mat) {
			strips = head->nStrip;
			strip++;
		} else
			break;
	}

	context->pkmCurrentPtr = pkmCurrentPtr;
	return head;
}

/* 
 * The C-version textured, clipped strip rasteriser 
 */

/* Emits the clipped vertex between e0 and e1 */
#define EMIT_EDGE(e0,e1) _EMIT_EDGE(e0,e1,KM_VERTEXPARAM_NORMAL)
#define EMIT_EDGE_FINAL(e0,e1) _EMIT_EDGE(e0,e1, KM_VERTEXPARAM_ENDOFSTRIP); RESET
#define _EMIT_EDGE(e0,e1,pcw) \
	alpha = tri[e1].z / (tri[e1].z - tri[e0].z); \
	colourAcc					= ColLerp (colour[e1], colour[e0], alpha); \
	X							= LERP (tri[e1].x, tri[e0].x, alpha); \
	Y							= LERP (tri[e1].y, tri[e0].y, alpha); \
	X							= (X * rWnearVal * pViewSize.x) + X_MID; \
	Y							= (Y * rWnearVal * pViewSize.y) + Y_MID; \
	kmxxSetVertex_0 (pcw, X, Y, rWnearVal, colourAcc)

/* Emits the non-clipped vertex e */
#define EMIT(e) _EMIT(e,KM_VERTEXPARAM_NORMAL)
#define EMIT_FINAL(e) _EMIT(e, KM_VERTEXPARAM_ENDOFSTRIP); RESET
#define _EMIT(e,pcw) \
	W							= 1.f / tri[e].w; \
	X							= (W * tri[e].x * pViewSize.x) + X_MID; \
	Y							= (W * tri[e].y * pViewSize.y) + Y_MID; \
	kmxxSetVertex_0 (pcw, X, Y, W, colour[e])

#define RESET \
	strip -= 2; nStrip += 2; \
	goto TopOfLoop
#define FLUSH \
	(void)0

/* Kludge to make case statements tally with my notes */
#define CLIPCASE(x) ((((x) & 1)<<2) | (((x) & 4)>>2) | (((x) & 2)))
StripHeader *nontexturedStripRasteriserClipped(StripPos *v, StripEntry *strip, Uint32 nStrip, StripRasterContext *context)
{
	KMSTATUS result;
	enum { ANTICLOCKWISE = 0, CLOCKWISE = 1 } parity = CLOCKWISE;
	Bool firstPoly;
	Uint8 clipped;
	Uint32 colour[3];
	float alpha, X, Y, W;
	Uint32 colourAcc, mat = context->mat;
	register PKMDWORD pkmCurrentPtr = context->pkmCurrentPtr;
	register StripPos *vert;

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

		if (tri[0].z > 0)
			clipped |= 1;
		strip++;

		/* Point 1 */
		vert = (StripPos *)((Uint32)v + (strip->vertRef<<2));
		(*(Point3 *)&input) = vert->p;
		input.w = 1.f;
		mPoint (&tri[1], &input);
		colour[1] = vert->colour;
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
			colour[0] = colour[1];
			colour[1] = colour[2];
			clipped>>=1;
		} /* End inner loop */
	} /* End Outer loop */

	context->pkmCurrentPtr = pkmCurrentPtr;
	return (StripHeader *)strip + nStrip;
}
