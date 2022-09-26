/*
 * Shadow.c
 * Does all the nasty shadow casting
 */

#include "..\RedDog.h"
#include "Internal.h"
#include "Lighting.h"
#include "Shadow.h"

/* number of bits in a word */
#define NUMBITS	   32

/* get the integral form of a floating point number */
#define v(x)	   (*(Sint32 *) &(x))

/* get the sign bit of an integer as a value 0 or 1 */
#define s(x)	   (((Uint32)(x)) >> (NUMBITS-1))

/* get the absolute value of a floating point number in integral form */
#define a(x)	   ((x) & ~(1 << (NUMBITS-1)))

static float NewShadLength;
#define NEW_SHAD_LENGTH 5.f

#define DEBUG_SHAD	0
#define END_CAPS	0

#define NUM_DIVS 2
#define MAX_SHAD_POLY	768
#define MAX_SHAD_HULLS	32
#define MAX_END_POINTS	1024

// The prepared vertex arrays
typedef struct {
	Point		viewPos;
	Point3		scrnPos;
	Outcode		outcode;
} PrepShadVert;

static PrepShadVert prepArray[MAX_SHADOW_VERTS * (NUM_DIVS+1)];

static Material	mat	= { 0, 0, 0.f, 0.f , MF_TWO_SIDED };

ShadModel *wheelShadow, *backChassisShadow, *gunShadow, *frontChassisShadow, *cubeShadow, *sphereShadow;
int maxVerts = 0;

void shInit (void)
{
	Stream *s;

	mat.surf.GBIX = 0;
	rFixUpMaterial (&mat, &sfxContext);

	s = sOpen ("SHADOW.LVL");
	wheelShadow = rLoadShadModel (s);
	backChassisShadow = rLoadShadModel (s);
	gunShadow = rLoadShadModel (s);
	frontChassisShadow = rLoadShadModel (s);
	cubeShadow = rLoadShadModel (s);
	sphereShadow = rLoadShadModel (s);
	sClose (s);
}

void shReInit(void)
{
//	NewShadLength = (Multiplayer) ? 4.f : 10.f;
}

// Load a shadow model into permananent residence
ShadModel *rLoadShadModel (Stream *s)
{
	ShadModel *sModel;
	int i, j;

	// Load base model
	sModel = syMalloc (sizeof (ShadModel));
	dAssert (sModel, "Out of RAM");
	sRead (sModel, sizeof (ShadModel), s);
	sModel->hull = syMalloc (sizeof (ShadHull) * sModel->nHulls);
	dAssert (sModel->hull, "Out of RAM");
	dAssert (sModel->nVertices < MAX_SHADOW_VERTS, "Too many shadow vertices");
	sModel->vertex = syMalloc (sizeof (Point3) * sModel->nVertices);
	dAssert (sModel->vertex, "Out of RAM");

	for (j = 0; j < sModel->nHulls; ++j) {
		ShadHull *sHull;

		sHull = sModel->hull + j;
		
		sRead (sHull, sizeof (ShadHull), s);

		// Allocate faces and edges
		sHull->face = syMalloc (sizeof (ShadFace) * sHull->nFaces);
		sHull->edge = syMalloc (sizeof (ShadEdge) * sHull->nEdges);
		dAssert (sHull->face && sHull->edge, "Out of RAM");
		
		// Load faces and edges
		sRead (sHull->face, (sizeof (ShadFace) * sHull->nFaces), s);
		sRead (sHull->edge, (sizeof (ShadEdge) * sHull->nEdges), s);
		
		// Fix up the model
		for (i = 0; i < sHull->nFaces; ++i) {
			sHull->face[i].edges[0] = sHull->edge + (int)sHull->face[i].edges[0];
			sHull->face[i].edges[1] = sHull->edge + (int)sHull->face[i].edges[1];
			sHull->face[i].edges[2] = sHull->edge + (int)sHull->face[i].edges[2];
		}
		for (i = 0; i < sHull->nEdges; ++i) {
			dAssert ((int)sHull->edge[i].face[0] != -1, "Broken shadow model");
			sHull->edge[i].face[0] = sHull->face + (int)sHull->edge[i].face[0];
			if ((int)sHull->edge[i].face[1] == -1) 
				sHull->edge[i].face[1] = NULL;
			else
				sHull->edge[i].face[1] = sHull->face + (int)sHull->edge[i].face[1];
		}
	}

	// Load the vertices in
	if (sModel->nVertices > maxVerts)
		maxVerts = sModel->nVertices;
	dAssert (maxVerts < MAX_SHADOW_VERTS, "Out of shadow verts");
	sRead (sModel->vertex, sizeof (Point3) * sModel->nVertices, s);

	sSkip (s);
	return sModel;
}

// Structure used to hold renderable calculate shadow model in model space
typedef struct {
	PrepShadVert *a, *b, *c;
} ShadPoly;

ShadPoly shadArray[MAX_SHAD_POLY];
ShadPoly *shadPtrEnd[MAX_SHAD_HULLS];	
ShadPoly *shadPtr;

#if END_CAPS
typedef struct {
	float x, y;
	float ang;
} EndPoint;
EndPoint endBuffer[MAX_END_POINTS];
EndPoint *endOffset;
#endif

Point3 pointBuffer[MAX_SHAD_POLY*3];
Point3 *pointOffset;

// Calculate a shadow hull using mCurMatrix
static void rCalcShadHull (ShadHull *model, register float lX, register float lY, register float lZ, int hiQuality)
{
	register int i;
	int kAdd;
	register ShadFace *face;
	register ShadEdge *edge;
	register float *read;
	register PrepShadVert **write;

	// Now do front-face culling for the shadow volume
	for (i = model->nFaces, face = model->face; i != 0; --i)
	{
		prefetch (((char *)face)+32);
		if (lX*face->faceNormal.x + lY*face->faceNormal.y + lZ*face->faceNormal.z > 0.f)
			face->culled = TRUE;
		else
			face->culled = FALSE;
		face++;
	}

	// Choose whether to do all the sub-pieces or just the one
	if (hiQuality) {
		kAdd = MAX_SHADOW_VERTS;
	} else {
		kAdd = MAX_SHADOW_VERTS * NUM_DIVS;
	}

	// We need to create a side polygon for every edge with one culled face
	for (i = model->nEdges, edge = model->edge; i != 0; --i)
	{
		int j;
		Bool cull1, cull2;
		// Find out if there is a face on each side of this edge
		dAssert (edge->face[0], "Shadow edge problem");
		cull1 = (!edge->face[0]->culled);
		cull2 = (edge->face[1] && !edge->face[1]->culled);
		// Is this a shadow edge?
		if (cull1 ^ cull2) {
			int k, p0, p1;
			// Get the data ready
			prefetch (shadPtr);
			// Choose p0 and p1 such that p0, p1x, p0x; p1, p1x, p0 are the two needed tris
			p0 = edge->face[0]->p[edge->v0];
			p1 = edge->face[0]->p[edge->v1];

			if (hiQuality) {
				j = NUM_DIVS;
			} else {
				j = 1;
			}
			k = 0;
			for (; j > 0 ; --j) {
				// Now to store in the two polys 
				// Firstly (p0, p1x, p0x)
				write = (PrepShadVert **)shadPtr;
				prefetch (((char *)write) + 32); // 32 bytes ahead prefetch, ready
				*write++	= prepArray + p0 + k;
				*write++	= prepArray + p1 + kAdd + k;
				*write++	= prepArray + p0 + kAdd + k;

				// Now (p1, p1x, p0)
				*write++	 = prepArray + p1 + k;
				*write++	 = prepArray + p1 + kAdd + k;
				*write++	 = prepArray + p0 + k;

				shadPtr+=2;
				k+=kAdd;
			}
		}
		edge++;
	}

	// Lastly, for every non-culled face, we must create two shadow caps
	write = (PrepShadVert **)shadPtr;
	for (i = model->nFaces, face = model->face; i != 0; --i, ++face)
	{
		prefetch (write + 8); // 8*4 == 32
		if (face->culled)
			continue;
		// Add top cap
		*write++ = prepArray + face->p[2];
		*write++ = prepArray + face->p[1];
		*write++ = prepArray + face->p[0];
		// Create bottom cap
		*write++ = prepArray + MAX_SHADOW_VERTS * NUM_DIVS + face->p[0];
		*write++ = prepArray + MAX_SHADOW_VERTS * NUM_DIVS + face->p[1];
		*write++ = prepArray + MAX_SHADOW_VERTS * NUM_DIVS + face->p[2];
	}
	shadPtr = (ShadPoly *)write;
}

void rCalcShadModel (ShadModel *model, int hiQuality)
{
	register int i;
	int startJ;
	register float rW, midX, midY, sizeX, sizeY, x, y, z, *read, *wrPt;
	PrepShadVert *write, *write2;
	Vector3 mSunDir, tempSunDir;
	float lX, lY, lZ, rLength;

	// Try and trap this schplatte
	ASSERT_VALID(model);
	ASSERT_VALID(model->vertex);

	mInvert3d (&worldToModel, &mCurMatrix);
	mLoadForVector (&worldToModel);
	tempSunDir.x = sunDir.x;
	tempSunDir.y = sunDir.y;
	tempSunDir.z = RANGE(0.8, sunDir.z, 1.0);
	mPoint3 (&mSunDir, (Vector3 *)&tempSunDir);

	read = &mSunDir.x;
	lX = *read++;
	lY = *read++;
	lZ = *read++;
	

	// Get the normalising factor
	rLength = isqrt (SQR(lX) + SQR(lY) + SQR(lZ));

	// Normalise the light vector
	lX *= rLength;
	lY *= rLength;
	lZ *= rLength;

	// Reset the shadow pointer
	shadPtr = shadArray;

	// Calculate each hull
	for (i = 0; i < model->nHulls; ++i) {
		rCalcShadHull (model->hull + i, lX, lY, lZ, hiQuality);
		shadPtrEnd[i] = shadPtr;
		dAssert (shadPtr < (shadArray + MAX_SHAD_POLY), "Not enough shad madness");
	}

	// Transform and calculate the vertices
	// Load in the model to screen matrix
	mLoadModelToScreen();
	// Read screen mid
	midX = pViewMidPoint.x;
	midY = pViewMidPoint.y;
	sizeX = pViewSize.x;
	sizeY = pViewSize.y;
	read = (float *)model->vertex;
	write = prepArray;
	if (hiQuality) {
		lX *= NEW_SHAD_LENGTH / NUM_DIVS;
		lY *= NEW_SHAD_LENGTH / NUM_DIVS;
		lZ *= NEW_SHAD_LENGTH / NUM_DIVS;
		startJ = 1;
	} else {
		lX *= NEW_SHAD_LENGTH * 1.5f;
		lY *= NEW_SHAD_LENGTH * 1.5f;
		lZ *= NEW_SHAD_LENGTH * 1.5f;
		startJ = NUM_DIVS;
	}

	for (i = 0; i < model->nVertices; i++) {
		int j;
		prefetch (read+8);
		x = *read++;
		y = *read++;
		z = *read++;
		write->outcode = mPointXYZOut (&write->viewPos, x, y, z);
		rW = 1.f / write->viewPos.w;
		wrPt = & write->scrnPos.x;
		*wrPt++ = midX + write->viewPos.x * rW * sizeX;
		*wrPt++ = midY + write->viewPos.y * rW * sizeY;
		*wrPt = rW;
		write2 = write + startJ * MAX_SHADOW_VERTS;
		for (j = startJ; j <= NUM_DIVS; ++j) {
			x -= lX;
			y -= lY;
			z -= lZ;
			write2->outcode = mPointXYZOut (&write2->viewPos, x, y, z);
			rW = 1.f / write2->viewPos.w;
			write2->scrnPos.x = midX + write2->viewPos.x * rW * sizeX;
			write2->scrnPos.y = midY + write2->viewPos.y * rW * sizeY;
			write2->scrnPos.z = rW;
			write2 += MAX_SHADOW_VERTS;
		}
		write++;
	}
}

#if DEBUG_SHAD
Uint32 bar = 0, foo, Dbug = 1;
#define DRAW_SHAD_TRI(A,B,C,D,E,F,G,H,I) \
	if (Dbug) {\
		bar++; \
		foo = 0;\
		if (bar & 1) foo |= 0xff;\
		if (bar & 2) foo |= 0xff00;\
		if (bar & 4) foo |= 0xff0000;\
		kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		A, B, C, 0xff000000 | foo); \
		kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		D, E, F, 0xff000000 | foo); \
		kmxxSetVertex_0 (KM_VERTEXPARAM_ENDOFSTRIP, G, H, I, 0xff000000 | foo); \
	} else {\
		kmxxSetVertex_17 (KM_VERTEXPARAM_ENDOFSTRIP, A, B, C, D, E, F, G, H, I);\
	}
#else
#define DRAW_SHAD_TRI(A,B,C,D,E,F,G,H,I) \
	kmxxSetVertex_17 (KM_VERTEXPARAM_ENDOFSTRIP, A, B, C, D, E, F, G, H, I)
#endif

static void MakeEndCap(void);
static void DrawClippedShadTri (Point *a, Point *b, Point *c, Outcode oA, Outcode oB, Outcode oC);
// Draw the currently calculated shadow with mCurMatrix
void rDrawShadModel (ShadModel *model)
{
	int i;
	register PrepShadVert **read;
	register PrepShadVert **endPtr;
	register float *pDraw;
	Outcode oA, oB, oC;
	PrepShadVert *p1, *p2, *p3;
	ShadPoly **shadPtrEndPtr;

	// Reset the endBuffer pointer, and the point buffer
#if END_CAPS
	endOffset = endBuffer;
#endif
	pointOffset = pointBuffer;
		
	shadPtrEndPtr = shadPtrEnd;
	read = (PrepShadVert **)shadArray;
	for (i = model->nHulls; i != 0; --i) 
	{
		prefetch (read);
		endPtr = (PrepShadVert **) *shadPtrEndPtr++;
		// Output all of the points, transforming as we go
		while (read < endPtr)
		{
			Outcode outcode;
			// Read in the points
			p1 = *read++;
			p2 = *read++;
			p3 = *read++;
			oA = p1->outcode;
			oB = p2->outcode;
			oC = p3->outcode;
			outcode = oA | oB | oC;

			// Is this triangle totally offscreen?
			prefetch (pointOffset);
			if ((outcode & OC_NOT) != OC_NOT)
				continue;
			// See if we need to do any clipping at all
			if (outcode & OC_OFF_NEAR) { 
				DrawClippedShadTri (&p1->viewPos, &p2->viewPos, &p3->viewPos, oA, oB, oC);
			} else {
				register float *write = (float *)pointOffset;
				register float *rd;
				rd = &p1->scrnPos.x;
				*write++ = *rd++;
				*write++ = *rd++;
				*write++ = *rd++;
				rd = &p2->scrnPos.x;
				*write++ = *rd++;
				*write++ = *rd++;
				*write++ = *rd++;
				rd = &p3->scrnPos.x;
				*write++ = *rd++;
				*write++ = *rd++;
				*write++ = *rd++;
				pointOffset = (Point3 *)write;
			}
		}

#if END_CAPS
		// Create an end cap using Sav's algorithm
		if (endOffset > endBuffer)
			MakeEndCap();
#endif

		// Early out if nothing to draw
		if (pointOffset == pointBuffer)
			continue;
		
		// Set up the material
#if DEBUG_SHAD
		if (Dbug)
			rSetMaterial (&mat);
		else
			kmSetVertexRenderState (&shadowContext[0]);
#else
		kmSetVertexRenderState (&shadowContext[0]);
#endif

#if COUNT_GEOMETRY
		nTris += (pointOffset - pointBuffer) / 3;
#endif
		pointOffset-=3;
		pDraw = &pointBuffer->x;

		kmxxGetCurrentPtr (&vertBuf);
		kmxxStartVertexStrip (&vertBuf);

		do {
#if !DEBUG_SHAD
			prefetch(pDraw + 8);
			*pkmCurrentPtr++  = 0xf0000000;
			*(PKMFLOAT)pkmCurrentPtr++  = *pDraw++;
			*(PKMFLOAT)pkmCurrentPtr++  = *pDraw++;
			*(PKMFLOAT)pkmCurrentPtr++  = *pDraw++;
			*(PKMFLOAT)pkmCurrentPtr++  = *pDraw++;
			*(PKMFLOAT)pkmCurrentPtr++  = *pDraw++;
			*(PKMFLOAT)pkmCurrentPtr++  = *pDraw++;
			*(PKMFLOAT)pkmCurrentPtr    = *pDraw++;
			prefetch((void *)pkmCurrentPtr);
			pkmCurrentPtr++;
			*(PKMFLOAT)pkmCurrentPtr++  = *pDraw++;
			*(PKMFLOAT)pkmCurrentPtr++  = *pDraw++;
			prefetch((void *)pkmCurrentPtr);
			pkmCurrentPtr += 6;
#else
			DRAW_SHAD_TRI (pDraw[0].x, pDraw[0].y, pDraw[0].z,
				pDraw[1].x, pDraw[1].y, pDraw[1].z,
				pDraw[2].x, pDraw[2].y, pDraw[2].z);
			pDraw += 3;
#endif
		} while (pDraw < &pointOffset->x);
		
		// Deal with the last one:
#if DEBUG_SHAD
		if (!Dbug)
#endif
		{
			kmSetVertexRenderState (&shadowContext[2]);
			kmxxStartVertexStrip (&vertBuf);
		}
#if !DEBUG_SHAD
		*pkmCurrentPtr++  = 0xf0000000;
		*(PKMFLOAT)pkmCurrentPtr++  = *pDraw++;
		*(PKMFLOAT)pkmCurrentPtr++  = *pDraw++;
		*(PKMFLOAT)pkmCurrentPtr++  = *pDraw++;
		*(PKMFLOAT)pkmCurrentPtr++  = *pDraw++;
		*(PKMFLOAT)pkmCurrentPtr++  = *pDraw++;
		*(PKMFLOAT)pkmCurrentPtr++  = *pDraw++;
		*(PKMFLOAT)pkmCurrentPtr    = *pDraw++;
		prefetch((void *)pkmCurrentPtr);
		pkmCurrentPtr++;
		*(PKMFLOAT)pkmCurrentPtr++  = *pDraw++;
		*(PKMFLOAT)pkmCurrentPtr++  = *pDraw++;
		prefetch((void *)pkmCurrentPtr);
		pkmCurrentPtr += 6;
#else
		DRAW_SHAD_TRI (pDraw[0].x, pDraw[0].y, pDraw[0].z,
			pDraw[1].x, pDraw[1].y, pDraw[1].z,
			pDraw[2].x, pDraw[2].y, pDraw[2].z);
#endif
		kmxxReleaseCurrentPtr (&vertBuf);
		pointOffset+=3;
	}
}

// At least one vertex is off the near side
static void DrawClippedShadTri (Point *a, Point *b, Point *c, Outcode oA, Outcode oB, Outcode oC)
{
	register float rW, midX, midY, sizeX, sizeY;
	register float *pBuf = (float *)pointOffset;

	midX = pViewMidPoint.x;
	midY = pViewMidPoint.y;
	sizeX = pViewSize.x;
	sizeY = pViewSize.y;

	// Is 'a' onscreen?
	if (!(oA & OC_OFF_NEAR)) {
		rW = 1.f / a->w;
		*pBuf++ = a->x * rW * sizeX + midX;
		*pBuf++ = a->y * rW * sizeY + midY;
		*pBuf++ = rW;
	}
	// Is the edge 'a'->'b' in need of an alpha vertex?
	if ((oA ^ oB) & OC_OFF_NEAR) {
		rW = a->z / (a->z - b->z);
		*pBuf++ = LERP(a->x, b->x, rW) * rWnearVal * sizeX + midX;
		*pBuf++ = LERP(a->y, b->y, rW) * rWnearVal * sizeY + midY;
		*pBuf++ = rWnearVal;
	}

	// Is 'b' onscreen?
	if (!(oB & OC_OFF_NEAR)) {
		rW = 1.f / b->w;
		*pBuf++ = b->x * rW * sizeX + midX;
		*pBuf++ = b->y * rW * sizeY + midY;
		*pBuf++ = rW;
	}
	// Is the edge 'b'->'c' in need of an alpha vertex?
	if ((oB ^ oC) & OC_OFF_NEAR) {
		rW = b->z / (b->z - c->z);
		*pBuf++ = LERP(b->x, c->x, rW) * rWnearVal * sizeX + midX;
		*pBuf++ = LERP(b->y, c->y, rW) * rWnearVal * sizeY + midY;
		*pBuf++ = rWnearVal;
	}

	// Is 'c' onscreen?
	if (!(oC & OC_OFF_NEAR)) {
		rW = 1.f / c->w;
		*pBuf++ = c->x * rW * sizeX + midX;
		*pBuf++ = c->y * rW * sizeY + midY;
		*pBuf++ = rW;
	}
	// Is the edge 'c'->'a' in need of an alpha vertex?
	if ((oA ^ oC) & OC_OFF_NEAR) {
		rW = c->z / (c->z - a->z);
		*pBuf++ = LERP(c->x, a->x, rW) * rWnearVal * sizeX + midX;
		*pBuf++ = LERP(c->y, a->y, rW) * rWnearVal * sizeY + midY;
		*pBuf++ = rWnearVal;
	}

	// Now we need to output the triangle/quad
	if (((Point3 *)pBuf - pointOffset) == 4) {
		Point3 *buff = (Point3 *)pBuf;
		// We've outputted ABCD - in order to make that two triangles
		// we need to output AC to complete the triangle DAC
		buff[0] = buff[-4];
		buff[1] = buff[-2];
		pointOffset+=6;
	} else {
		// Already output
		pointOffset+=3;
	}
}

#if END_CAPS
// End cap craziness
#define SWAP_PT2(a,b) \
	temp = a->x; a->x = b->x; b->x = temp; \
	temp = a->y; a->y = b->y; b->y = temp
static int epComp(void *a, void *b)
{
	return (((EndPoint *)b)->ang - ((EndPoint *)a)->ang);
}
static void MakeEndCap(void)
{
	EndPoint *rightMost;
	float x = -1e9, temp, y;
	EndPoint *p, *q, *r;
	int dir;

	// Find the rightmost point in the end array
	for (p = endBuffer; p < endOffset; ++p) {
		if (p->x > x) 
			x = p->x, rightMost = p;
	}

	// Swap the rightmost with 0
	SWAP_PT2(rightMost, endBuffer);

	y = p->y;
	// Calculate the angle to rightmost for each point
	for (p = endBuffer + 1; p < endOffset; ++p) {
		float dx, dy;
		dx = p->x - x;
		dy = p->y - y;
		p->ang = isqrt(dx*dx+dy*dy) * dy;
	}

	// Now go through all the rest of the points and sort them by order of
	// angle from rightMost
	qsort (endBuffer + 1, endOffset - (endBuffer + 1), sizeof (EndPoint), epComp);

	// Using that information, generate triangles appropriately
	p = r = endBuffer;
	q = endOffset - 1;
	dir = 0;
	while (p != q) {
		// Move either p on or q on
		if (dir) {
			r = p;
			p++;
		} else {
			r = q;
			--q;
		}
		pointOffset->x = r->x;
		pointOffset->y = r->y;
		pointOffset->z = rWnearVal;
		pointOffset++;
		pointOffset->x = p->x;
		pointOffset->y = p->y;
		pointOffset->z = rWnearVal;
		pointOffset++;
		pointOffset->x = q->x;
		pointOffset->y = q->y;
		pointOffset->z = rWnearVal;
		pointOffset++;
	}
}
#endif

void DoTankShads(int pn)
{
	int i;
	if (player[pn].Player &&
		player[pn].Player->objId && 
		!(player[pn].Player->flag & STRAT_HIDDEN)) {
		// Draw the wheel shadows
		for (i = 1; i < 5; ++i) {
			if (player[pn].Player->objId[i]->collFlag & OBJECT_INVISIBLE)
				continue;
			mCurMatrix = player[pn].Player->objId[i]->wm;
			rCalcShadModel (wheelShadow, TRUE);
			rDrawShadModel (wheelShadow);
		}
		
		if (!(player[pn].Player->objId[5]->collFlag & OBJECT_INVISIBLE)) {
			mCurMatrix = player[pn].Player->objId[5]->wm;
			rCalcShadModel (gunShadow, TRUE);
			rDrawShadModel (gunShadow);
		}
		
		if (!(player[pn].Player->objId[13]->collFlag & OBJECT_INVISIBLE)) {
			mCurMatrix = player[pn].Player->objId[13]->wm;
			rCalcShadModel (frontChassisShadow, TRUE);
			rDrawShadModel (frontChassisShadow);
		}
		
		if (!(player[pn].Player->objId[6]->collFlag & OBJECT_INVISIBLE)) {
			mCurMatrix = player[pn].Player->objId[6]->wm;
			rCalcShadModel (backChassisShadow, TRUE);
			rDrawShadModel (backChassisShadow);
		}
	}
}


/*
 * Draws a shadow for a bbox
 * mCurMatrix = modelToWorld
 */
void shBBox (BBox *bounds)
{
	// We need to get mCurMatrix such that a unit cube is bounds
	float width, height, length, size;

	if (Multiplayer)
		return;

	mPush (&mCurMatrix);

	size = rGlobalTransparency * 1.02f; // Add 2% to allow for box-shaped madness
	width  = (bounds->hi.x - bounds->low.x) * size;
	length = (bounds->hi.y - bounds->low.y) * size;
	height = (bounds->hi.z - bounds->low.z) * size;

	mPreTranslate (&mCurMatrix, 
		(bounds->hi.x + bounds->low.x) * 0.5f,
		(bounds->hi.y + bounds->low.y) * 0.5f,
		(bounds->hi.z + bounds->low.z) * 0.5f);
	mPreScale (&mCurMatrix, width, length, height);

	rCalcShadModel (cubeShadow, FALSE);
	rDrawShadModel (cubeShadow);

	mPop (&mCurMatrix);
}

/*
 * Draws a round shadow for a bbox
 * mCurMatrix = modelToWorld
 */
void shBBoxRound (BBox *bounds)
{
	// We need to get mCurMatrix such that a unit cube is bounds
	float width, height, length;

	if (Multiplayer)
		return;

	mPush (&mCurMatrix);

	width  = (bounds->hi.x - bounds->low.x) * rGlobalTransparency;
	length = (bounds->hi.y - bounds->low.y) * rGlobalTransparency;
	height = (bounds->hi.z - bounds->low.z) * rGlobalTransparency;

	mPreTranslate (&mCurMatrix, 
		(bounds->hi.x + bounds->low.x) * 0.5f,
		(bounds->hi.y + bounds->low.y) * 0.5f,
		(bounds->hi.z + bounds->low.z) * 0.5f);
	mPreScale (&mCurMatrix, width, length, height);

	rCalcShadModel (sphereShadow, FALSE);
	rDrawShadModel (sphereShadow);

	mPop (&mCurMatrix);
}

/*
 * Draws a dummy shadow offscreen to try and work around a seemingly hardware bug
 */
void rShadowWorkaround(void)
{
	kmSetVertexRenderState (&shadowContext[0]);
	kmxxGetCurrentPtr (&vertBuf);
	kmxxStartVertexStrip (&vertBuf);
	kmxxSetVertex_17 (KM_VERTEXPARAM_ENDOFSTRIP, 
		-1000, 0, 1,
		-1000, 100, 1,
		-1100, 0, 1);
	kmxxSetVertex_17 (KM_VERTEXPARAM_ENDOFSTRIP, 
		-1100, 0, 1,
		-1100, 100, 1,
		-1200, 0, 1);
	kmSetVertexRenderState (&shadowContext[2]);
	kmxxStartVertexStrip (&vertBuf);
	kmxxSetVertex_17 (KM_VERTEXPARAM_ENDOFSTRIP, 
		-1200, 0, 1,
		-1200, 100, 1,
		-1300, 0, 1);
	kmxxReleaseCurrentPtr (&vertBuf);
}

