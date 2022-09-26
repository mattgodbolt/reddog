/*
 * StripRenderClipped.c
 * The clipped model strip drawing routines
 */

#include "..\RedDog.h"
#include "Internal.h"
#include "Rasterisers.h"

#pragma section RASTERISERS
#if !OLD_RENDER

// Defines are deliberately disabled here as otherwise the (ms) preprocessor
// tries to expand them...which doesn't quite work

// Read in a vertex macro
!TEX	#define READ_UV(n) \
!TEX	pUV[n].u = *uv++; \
!TEX	pUV[n].v = *uv++

!LIT	#define LIGHT(n) { i[n] = FIPR (vX, vY, vZ, 0, lX, lY, lZ, 0); if (i[n]<0)i[n]=0; i[n]+= iAdd; }
!UNL	#define LIGHT(n) i[n] = 1.f

#define READ_VERTEX(n) \
	v = *strip++; \
	vertex = (float *)((char *)vBase + v); \
	vX = *vertex++; \
	normal = (float *)((char *)nBase + v); \
	vY = *vertex++; \
	vZ = *vertex++; \
	mPointXYZ (&p[n], vX, vY, vZ); \
	p[n].w = 1.f / p[n].w; \
	vX = *normal++; \
	vY = *normal++; \
	vZ = *normal++; \
	LIGHT(n); \
	out[n] = (p[n].z < 0) ? 2 : 1; \
!TEX	READ_UV(n)

#define VERT		lastVert.ParamControlWord = 0xe0000000; kmiMemCopy8 (pkmCurrentPtr, &lastVert, sizeof (lastVert)); prefetch (pkmCurrentPtr); pkmCurrentPtr += 8
#define VERT_LAST	lastVert.ParamControlWord = 0xf0000000; kmiMemCopy8 (pkmCurrentPtr, &lastVert, sizeof (lastVert)); prefetch (pkmCurrentPtr); pkmCurrentPtr += 8

	// Flush a vertex out
#define FLUSH \
	if (needsFlushing) { \
		needsFlushing = FALSE; \
		VERT_LAST; \
	}

	// Sets a vertex, flushing out a previous one if necesary
#define SET_VERT(num) \
	if (needsFlushing) { \
		VERT; \
	} \
	needsFlushing = TRUE; \
	lastVert.fX			= context->midX + p[num].w * p[num].x; \
	lastVert.fY			= context->midY + p[num].w * p[num].y; \
	lastVert.u.fInvW	= p[num].w; \
	lastVert.fBaseIntensity = i[num]; \
!TEX	lastVert.fU			= pUV[num].u; \
!TEX	lastVert.fV			= pUV[num].v; \
!TEX	lastVert.fOffsetIntensity = 1.f

	// Sets a vertex in the clipped array
#define SET_VERT_NONCLIPPED(num) \
	clipBuf[vert].ParamControlWord = 0xe0000000; \
	clipBuf[vert].fX			= context->midX + p[num].w * p[num].x; \
	clipBuf[vert].fY			= context->midY + p[num].w * p[num].y; \
	clipBuf[vert].u.fInvW	= p[num].w; \
	clipBuf[vert].fBaseIntensity = i[num]; \
!TEX	clipBuf[vert].fU			= pUV[num].u; \
!TEX	clipBuf[vert].fV			= pUV[num].v; \
!TEX	clipBuf[vert].fOffsetIntensity = 1.f

#define SET_VERT_ALPHA(num,num2) \
	alpha = p[num].z / (p[num].z - p[num2].z); \
	clipBuf[vert].ParamControlWord = 0xe0000000; \
	clipBuf[vert].fX			= context->midX + rWnearVal * LERP (p[num].x, p[num2].x, alpha); \
	clipBuf[vert].fY			= context->midY + rWnearVal * LERP (p[num].y, p[num2].y, alpha); \
	clipBuf[vert].u.fInvW	= rWnearVal; \
	clipBuf[vert].fBaseIntensity = LERP (i[num], i[num2], alpha); \
!TEX	clipBuf[vert].fU			= LERP (pUV[num].u, pUV[num2].u, alpha); \
!TEX	clipBuf[vert].fV			= LERP (pUV[num].v, pUV[num2].v, alpha); \
!TEX	clipBuf[vert].fOffsetIntensity = 1.f

#pragma aligndata32(lastVert,clipBuf)
!TEX	static KMVERTEX7	lastVert, clipBuf[4];
!UNT	static KMVERTEX2	lastVert, clipBuf[4];
PKMDWORD ROUTINE (ModelContext *context, VertRef numStrip, VertRef mat, PKMDWORD pkm, 
				  float lX, float lY, float lZ)
{
	register VertRef *strip = context->strip;
	register PKMDWORD pkmCurrentPtr = pkm;
	register float *vertex, *normal;
	register float vX, vY, vZ;
	register VertRef v;
	Point3   *vBase = context->vertex;
	Vector3  *nBase = context->vertexNormal;
	Point p[3];
	float i[3];
	Sint8 out[3], all;
	static const int pNumNext[3] = { 1, 2, 0 };
	int pNum;
	VertRef newMat;
	float iAdd = context->intensityOffset;
	enum { CLOCK = 0, ANTICLOCK = 1 } parity;
	Bool needsFlushing;
	register float *uv = (float *)context->uvArray;
!TEX	UV pUV[3];

	do {
		// The zeroth polygon is CLOCK, so the first is ANTICLOCK
		parity = CLOCK;
		pNum = 0; // start filling in from vertex '0' in the history buffer
		needsFlushing = FALSE;
!UNT	uv += (numStrip*2); // Move strip on
		// Read in the next two vertices
		READ_VERTEX(pNum);
		pNum = pNumNext[pNum];
		READ_VERTEX(pNum);
		pNum = pNumNext[pNum];
		numStrip -= 2;
		do {
			FLUSH; // Flush any previous vertices
			READ_VERTEX(pNum);
			pNum = pNumNext[pNum];
			// Toggle the parity of this polygon
			parity = 1 - parity; 
			// Get the triangle's outcode
			all = out[0] | out[1] | out[2];
			// Account for having dealt with three vertices
			numStrip --;
			// Is this polygon completely offscreen?
			if ((all & 1) == 0) {
				// Loop until either we run out of vertices,
				// or we have an onscreen polygon
				while (numStrip) {
					// Get the next vertex
					READ_VERTEX(pNum);
					all = out[pNum]; // Get the last vertex's outcode
					pNum = pNumNext[pNum];
					parity = 1 - parity;
					numStrip--;
					if (all & 1)
						goto NearClip;
				}
			}
			// Is it completely onscreen?
			else if ((all & 2) == 0) {
				// Output the first vertex
				SET_VERT(pNum);
				// If the parity is opposite, then re-output the first vertex
				if (parity == CLOCK) {
					SET_VERT(pNum);
				}
				pNum = pNumNext[pNum];
				// Output the second vertex
				SET_VERT(pNum);
				pNum = pNumNext[pNum];
				// Output the third, and potentially last vertex
				SET_VERT(pNum);
				pNum = pNumNext[pNum];
				// Now to continue outputting onscreen vertices until
				// we run out of vertices, or hit an offscreen vertex;
				while (numStrip) {
					// Get the next vertex
					READ_VERTEX(pNum);
					all = out[pNum]; // Get the last vertex's outcode
					if (all & 1) {
						SET_VERT (pNum);
					}
					pNum = pNumNext[pNum];
					parity = 1 - parity;
					numStrip--;
					if (all & 2)
						goto NearClip;
				}
			} else {
				// It requires near-clipping then
				// If we got here, then p[pNum], p[pNum+1], p[pNum+2] is a triangle
				// of parity 'parity' needing clipping.  At least one vertex is onscreen
				register int ii;
				int nextP, vert;
NearClip:
				// We may well have been outputting vertices before this, so flush any remaining
				FLUSH;
				vert = 0;
				// Do the near clipping for the edges
				for (ii = 0; ii < 3; ++ii) {
					nextP = pNumNext[pNum];
					if (out[pNum] & 1) {
						// Vertex i is onscreen - output
						SET_VERT_NONCLIPPED(pNum);
						vert++;
					}
					// Is there an alpha vertex?
					if ((out[pNum] | out[nextP]) == 3) {
						register float alpha;
						SET_VERT_ALPHA(pNum, nextP);
						vert++;
					}
					pNum = nextP;
				}
				dAssert (vert == 3 || vert == 4, "Erk in the clipping");
				// Output the resultant polygon
				if (parity == ANTICLOCK) {
					if (vert == 3) {
						clipBuf[2].ParamControlWord = 0xf0000000;
						pkmCurrentPtr = (PKMDWORD)AlignedCopy8SQ(pkmCurrentPtr, clipBuf, sizeof(lastVert)*3);
					} else {
						clipBuf[2].ParamControlWord = 0xf0000000;
						pkmCurrentPtr = (PKMDWORD)AlignedCopy8SQ(pkmCurrentPtr, clipBuf, sizeof(lastVert)*2);
						pkmCurrentPtr = (PKMDWORD)AlignedCopy8SQ(pkmCurrentPtr, clipBuf+3, sizeof(lastVert));
						pkmCurrentPtr = (PKMDWORD)AlignedCopy8SQ(pkmCurrentPtr, clipBuf+2, sizeof(lastVert));
					}
				} else {
					if (vert == 3) {
						clipBuf[1].ParamControlWord = 0xf0000000;
						pkmCurrentPtr = (PKMDWORD)AlignedCopy8SQ(pkmCurrentPtr, clipBuf, sizeof(lastVert));
						pkmCurrentPtr = (PKMDWORD)AlignedCopy8SQ(pkmCurrentPtr, clipBuf+2, sizeof(lastVert));
						pkmCurrentPtr = (PKMDWORD)AlignedCopy8SQ(pkmCurrentPtr, clipBuf+1, sizeof(lastVert));
					} else {
						clipBuf[3].ParamControlWord = 0xf0000000;
						pkmCurrentPtr = (PKMDWORD)AlignedCopy8SQ(pkmCurrentPtr, clipBuf+1, sizeof(lastVert));
						pkmCurrentPtr = (PKMDWORD)AlignedCopy8SQ(pkmCurrentPtr, clipBuf+0, sizeof(lastVert));
						pkmCurrentPtr = (PKMDWORD)AlignedCopy8SQ(pkmCurrentPtr, clipBuf+2, sizeof(lastVert)*2);
					}
				}
				// Go round again
			}

		} while (numStrip);
		// The end of a strip
EndStrip:
		// Flush any remaining vertices
		FLUSH;
		newMat = *strip++;
		numStrip = *strip++;
	} while ((newMat == mat) && (numStrip != 0));
	context->strip = strip - 2;

	context->uvArray = (UV *)uv;
	
	return pkmCurrentPtr;
}
#endif
