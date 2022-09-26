/*
 * Band.c
 * Controls 'rubber-banded' realtime extruded shapes
 * For example, the trails behind missiles are extruded triangles
 * Also used for tyre tracks etc etc
 */

#include "..\RedDog.h"
#include "..\View.h"
#include "Internal.h"
#include "Bands.h"

void DrawTrails(void);
void moveTrailsBack(float amount);
float	moveTrailsDistance = 0.0f;

// A 2d shape
typedef struct tagShape
{
	int			nVerts; // Number of vertices in the shape
	Point2		*data;	// Array of data points for the shape
} Shape;

// Defining a type of band
// Each band is represented by a set of matrices, where each
// matrix moves the 2d representation into 3d space
// The matrix at matList[curBands] is the endpoint of the band
typedef struct tagBand
{
	Shape			*shape;				// The base shape for this band (NULL if empty slot)
	int				maxSegs;			// The maximum number of segments (matrices)
	int				curSegs;			// The current number of segments (matrices)
	Matrix43		*matList;			// The array of matrices describing the band
	Matrix43		incMat;				// An incremental matrix to apply to each bit
	Material		*material;			// The material to draw the bands with
	LightingValue	colour;				// The colour the band is drawn in
	LightingValue	cInc;				// The increments per band to change the colour by
	int				deleteMe;			// Band should be decremented, then deleted
#ifdef _DEBUG
	STRAT			*strat;
#endif
} Band;

// The shape data for the enumerations
// A single poly (tyre track)
static const Point2 poly[2] = { {-0.5, 0}, {0.5, 0} };
static const Point2 triangle[3] = { {0, 0.5}, {-0.5, -0.5}, {0.5, -0.5} };
static const Point2 tube[8] = 
{ 
	{ 0.000f, 1.000f },
	{ 0.707f, 0.707f },
	{ 1.000f, 0.000f },
	{ 0.707f,-0.707f },
	{ 0.000f,-1.000f },
	{-0.707f,-0.707f },
	{-1.000f, 0.000f },
	{-0.707f, 0.707f }
};
static Shape shapes[] =
{
	// The single poly
	{ 2, poly },
	// The triangle
	{ 3, triangle },
	// A nice tube
	{ 8, tube }
};

// The set of currently active bands
static Band band[MAX_BANDS];

// A stash of matrices (43s to save space)
static Matrix43 bandMatrix[MAX_BAND_MATRICES];
// The high water mark of matrices (low is always 0)
static int bandMatrixHWM;

// The material used
static Material bandMat = 
{ 0, 0, 0.f, 0.f, MF_TRANSPARENCY_ADDITIVE | MF_ALPHA | MF_TWO_SIDED };
Material flareMat = 
{ 0, 0, 0.f, 0.f, MF_TRANSPARENCY_ADDITIVE | MF_ALPHA | MF_TWO_SIDED | MF_TEXTURE_BILINEAR };
// Trail definitions

typedef struct tagTrail {
	int				nPts;
	int				curPt;
	float			size;
	Point2			*pt;
	LightingValue	colour;
	LightingValue	cInc;
#ifdef _DEBUG
	STRAT			*strat;
#endif
} Trail;

#define TRAIL_SIZE 128
#define TRAIL_MUL (1.f / 0.98f)
#define TRAIL_GAP_MIN 0.1f
#define TRAIL_GAP_MAX 4.f
#define TRAIL_MAX_DEPTH 3
#define TRAIL_SKIP_MAX 1
static Point2 p2Array[MAX_TRAIL_POINTS];
static Trail trail[MAX_TRAILS];
static int maxPt = 0;

// Functions :

// Create a band
BandHandle bCreate (ShapeType sType, int mType, int maxSegs, LightingValue colour[2], Matrix43 *incMat)
{
	register int i;
	register Band *b;
	float rMaxSegs;
	Material *material;

	// Sanity checks
	dAssert (sType >= 0 && sType < ST_MAX, "Invalid shape passed to bCreateBand");
	dAssert (maxSegs >= 2, "Silly amount of segs passed to bCreateBand");

	// XXX
	material = &bandMat;

	// Firstly seek to find a spare entry in the band table
	for (i = 0; i < MAX_BANDS; ++i)
		if (band[i].shape == NULL)
			break;
	if (i == MAX_BANDS)
		return -1;	// No spare band could be found

  
	// Get a pointer to our chosen band element
	b = band + i;

	// Check to see if there is enough room in the matrix stash to hold 'maxSegs'
	// If not, reduce maxSegs
	// If maxSegs < 3, then return NULL - not enough room for even the most primitive band
	if ((MAX_BAND_MATRICES - 3) < (bandMatrixHWM + maxSegs))
		return -1;

	// Set up the band structure
	b->matList	= bandMatrix + bandMatrixHWM;
	b->incMat	= *incMat;
	b->shape	= &shapes[sType];
	b->curSegs	= 0;
	b->maxSegs	= maxSegs;
	b->material	= material;
	b->colour	= colour[0];
	b->deleteMe = 0;
	// Calculate the increments
	rMaxSegs	= 1.f / maxSegs;
	b->cInc.a	= (colour[0].a - colour[1].a) * rMaxSegs;
	b->cInc.r	= (colour[0].r - colour[1].r) * rMaxSegs;
	b->cInc.g	= (colour[0].g - colour[1].g) * rMaxSegs;
	b->cInc.b	= (colour[0].b - colour[1].b) * rMaxSegs;

	// Move the HWM on
	bandMatrixHWM += maxSegs;

	return i;
}

// Reset the band structure
void bReset (void)
{
	register int i;
	for (i = 0; i < MAX_BANDS; ++i)
		band[i].shape = NULL;
	for (i = 0; i < MAX_TRAILS; ++i)
		trail[i].pt = NULL;
	maxPt = 0;
	bandMatrixHWM = 0;
	bandMat.surf.GBIX = 0;
	rFixUpMaterial (&bandMat, &objContext);
}

// Destroy a band
static void bDelete (BandHandle bandH)
{
	register int i;
	register Band *b;
	int mStart;

	// Sanity checks
	dAssert (bandH >= 0 && bandH < MAX_BANDS, "Band out of range");
	b = band + bandH;
	dAssert (b->shape != NULL, "Band already freed");

	// Mark as free
	b->shape = NULL;

	// Find the entry in the matrix table
	mStart = b->matList - bandMatrix;

	// Is it the last band in the matrix stash?
	if ((mStart + b->maxSegs) == bandMatrixHWM) {
		// Move the HWM to mStart
		bandMatrixHWM = mStart;
	} else {
		// Adjust bandMatrixHWM
		bandMatrixHWM -= b->maxSegs;
		// Move the matrix stash down
		memmove (b->matList, bandMatrix + (mStart + b->maxSegs), sizeof (Matrix43) * (bandMatrixHWM - mStart));
		// We now have to adjust all of the other bands' matList pointers
		for (i = 0; i < MAX_BANDS; ++i) {
			if (band[i].shape) {
				if (band[i].matList > b->matList)
					band[i].matList -= b->maxSegs;
			}
		}
	}
}

void bClean()
{
	register int i;
	register Band *b = band;

	for (i = 0; i < MAX_BANDS; ++i, ++b)
	{
		if (b->shape)
			bDelete (i);
	}
	bReset();

}

// Draw a quad, clipping if necessary
// The material must already be set
// a, b, c, d is a strip-ordered quad such that abdc is a 'true' quad
// beginStrip is a pointer which says whether this is the beginning of a strip
// - it is updated if necessary
// vertex is the destination, which is left unterminated
// XXX - only deals with non-textured quads at present
#define TOP_BIT_TO_BIT(a,b) ((*(Uint32 *)&(a) & (1<<31))>>(31-(b)))
static KMVERTEX1 *bQuad (KMVERTEX1 *vertex,
						 Point *a, Point *b, Point *c, Point *d,
						 float a0, float r0, float g0, float b0,
						 float a1, float r1, float g1, float b1,
						 Bool *beginStrip)
{
	// Trick the l4 macros into using our vertex array
	register PKMDWORD pkmCurrentPtr = (PKMDWORD)vertex;
	register float x, y;
	register float	cx = pViewMidPoint.x,
					cy = pViewMidPoint.y,
					sx = pViewSize.x,
					sy = pViewSize.y;

	// If the previous quad was clipped, then beginStrip will be TRUE
	// If it wasn't, then we only need worry about clipping from a-c, b-d
	if (*beginStrip) {
		// We are at the start of a strip and therefore need to take into 
		// account all the points
		// The top bit of an FP variable is the sign - and therefore
		// this gets a value where each bit 0-3 corresponds to offscreen
		// when set, or onscreen if not
		switch (TOP_BIT_TO_BIT(a->z,0) | TOP_BIT_TO_BIT(b->z, 1) |
				TOP_BIT_TO_BIT(c->z,2) | TOP_BIT_TO_BIT(d->z, 3))
		{
		case 0:	// All points on screen - wicked
			*beginStrip = FALSE; // We can continue from here
			// Vertex A
			x = a->x * a->w * sx + cx;
			y = a->y * a->w * sy + cy;
			kmxxSetVertex_1 (KM_VERTEXPARAM_NORMAL, x, y, a->w,
				a0, r0, g0, b0);
			// Vertex B
			x = b->x * b->w * sx + cx;
			y = b->y * b->w * sy + cy;
			kmxxSetVertex_1 (KM_VERTEXPARAM_NORMAL, x, y, b->w,
				a1, r1, g1, b1);
			// Vertex C
			x = c->x * c->w * sx + cx;
			y = c->y * c->w * sy + cy;
			kmxxSetVertex_1 (KM_VERTEXPARAM_NORMAL, x, y, c->w,
				a0, r0, g0, b0);
			// Vertex D
			x = d->x * d->w * sx + cx;
			y = d->y * d->w * sy + cy;
			kmxxSetVertex_1 (KM_VERTEXPARAM_NORMAL, x, y, d->w,
				a1, r1, g1, b1);
			break;
		default: // Any offscreen - end for now XXX
			break;
		}
	} else {
		// We are continuing a strip - we only need to check to see the c and d vertices
		switch (TOP_BIT_TO_BIT(c->z,0) | TOP_BIT_TO_BIT(d->z, 1))
		{
		case 0: // All point on screen - even more wicked
			// Vertex C
			x = c->x * c->w * sx + cx;
			y = c->y * c->w * sy + cy;
			kmxxSetVertex_1 (KM_VERTEXPARAM_NORMAL, x, y, c->w,
				a0, r0, g0, b0);
			// Vertex D
			x = d->x * d->w * sx + cx;
			y = d->y * d->w * sy + cy;
			kmxxSetVertex_1 (KM_VERTEXPARAM_NORMAL, x, y, d->w,
				a1, r1, g1, b1);
			break;
		default: // Any offscreen - end for now XXX
			*beginStrip = TRUE;
			break;
		}

	} // beginStrip

#if COUNT_GEOMETRY
	nDrawn+=2;
#endif

	return (KMVERTEX1 *)pkmCurrentPtr;
}

// Updates all the bands
void bUpdate (void)
{
	register int i, j, k;
	register Band *b = band;
	register float aa, rr, gg, bb, aInc, rInc, gInc, bInc;
	float *suck;
	int nVerts;
	Point	startPoint[MAX_SHAPE_SIDES];
	Point	endPoint[MAX_SHAPE_SIDES];
	Point	p;
	Bool	beginStrip; 
	KMVERTEX1	bVertBuf[MAX_SHAPE_SIDES * 5], *v;
#ifdef _DEBUG
	int BANDAMOUNT = 0;
#endif
	for (i = 0; i < MAX_BANDS; ++i, ++b) {
		if (b->shape == NULL)
			continue;

#ifdef _DEBUG
	BANDAMOUNT++;
#endif
		// Check for autonomous update
		if (b->deleteMe) {
			// Finished?
			if (--b->deleteMe <= 0) {
				// Delete the band
				bDelete (i);
				// Account for the fact that the bands have been moved down
				--i, --b;
				continue;
			}
			b->colour.a -= b->cInc.a;
			b->colour.r -= b->cInc.r;
			b->colour.g -= b->cInc.g;
			b->colour.b -= b->cInc.b;
		}
		if (b->curSegs < 2)
			continue;
		// Update the matrices
		for (j = 0; j < b->curSegs; ++j) {
			m43to44 (&mCurMatrix, &b->incMat);
			mLoad43 (&b->matList[j]);
			mMul (&mCurMatrix, &mCurMatrix);
			b->matList[j].m[0][0] = mCurMatrix.m[0][0];
			b->matList[j].m[0][1] = mCurMatrix.m[0][1];
			b->matList[j].m[0][2] = mCurMatrix.m[0][2];
			b->matList[j].m[1][0] = mCurMatrix.m[1][0];
			b->matList[j].m[1][1] = mCurMatrix.m[1][1];
			b->matList[j].m[1][2] = mCurMatrix.m[1][2];
			b->matList[j].m[2][0] = mCurMatrix.m[2][0];
			b->matList[j].m[2][1] = mCurMatrix.m[2][1];
			b->matList[j].m[2][2] = mCurMatrix.m[2][2];
			b->matList[j].m[3][0] = mCurMatrix.m[3][0];
			b->matList[j].m[3][1] = mCurMatrix.m[3][1];
			b->matList[j].m[3][2] = mCurMatrix.m[3][2];
		}
	} // foreach band
#ifdef _DEBUG
	{extern int suppressDebug;
	if (!suppressDebug)
		tPrintf (20, 6, "%d BAND", BANDAMOUNT); 
	}
#endif

}

// Draws all the bands
void bDraw(void)
{
	register int i, j, k;
	register Band *b = band;
	register float aa, rr, gg, bb, aInc, rInc, gInc, bInc;
	float *suck;
	int nVerts;
	Point	startPoint[MAX_SHAPE_SIDES];
	Point	endPoint[MAX_SHAPE_SIDES];
	Point	p;
	Bool	beginStrip; 
	KMVERTEX1	bVertBuf[MAX_SHAPE_SIDES * 5], *v;

	for (i = 0; i < MAX_BANDS; ++i, ++b) {
		if (b->shape == NULL)
			continue;
		if (b->curSegs < 2)
			continue;
		// Work out the start colour and increments
		// Bear in mind we start at the end and work to the front
		suck = &b->cInc.a;
		aInc = *suck++; rInc = *suck++; gInc = *suck++; bInc = *suck++;
		suck = &b->colour.a;
		aa = *suck++; rr = *suck++; gg = *suck++; bb = *suck++;
		// Now to adjust to being at the 'end' of the pipe
		aa -= aInc * b->curSegs;
		rr -= rInc * b->curSegs;
		gg -= gInc * b->curSegs;
		bb -= bInc * b->curSegs;

		// Set the material
		rSetMaterial (b->material);

		// Now to start drawing it
		nVerts = b->shape->nVerts;
		// Transform the first set of vertices
		mLoad (&mWorldToScreen);
		m43to44 (&mCurMatrix, b->matList);
		mMul (&mCurMatrix, &mCurMatrix);
		mLoad (&mCurMatrix);
		for (j = 0; j < nVerts; ++j) {
			p.x = b->shape->data[j].x;
			p.y = b->shape->data[j].y;
			p.z = 0.f;
			p.w = 1.f;
			mPoint (&endPoint[j], &p);
			endPoint[j].w = 1.f / endPoint[j].w;
		}

		// Now to loop for the rest of the points, transforming and drawing
		for (k = 1; k < b->curSegs; ++k) {
			// Calculate the points to screen matrix
			mLoad (&mWorldToScreen);
			m43to44 (&mCurMatrix, &b->matList[k]);
			mMul (&mCurMatrix, &mCurMatrix);
			mLoad (&mCurMatrix);

			// Calculate each point
			for (j = 0; j < nVerts; ++j) {
				p.x = b->shape->data[j].x;
				p.y = b->shape->data[j].y;
				p.z = 0.f;
				p.w = 1.f;
				startPoint[j] = endPoint[j]; // Preserve previous point
				mPoint (&endPoint[j], &p);
				endPoint[j].w = 1.f / endPoint[j].w;
			}

			// Now to draw each poly in turn
			v = bVertBuf;
			beginStrip = TRUE;
			for (j = 0; j < nVerts; ++j) {
				int l = (j+1);
				if (l == nVerts) l = 0;
				v = bQuad (v, &startPoint[j], &endPoint[j], &startPoint[l], &endPoint[l],
					aa, rr, gg, bb, aa + aInc, rr + rInc, gg + gInc, bb + bInc, &beginStrip);
			}
			aa += aInc;
			rr += rInc;
			gg += gInc;
			bb += bInc;
			// Any polygons to draw? - terminate the list
			if (v != bVertBuf) {
				register PKMDWORD ptr = (PKMDWORD) bVertBuf;
				v[-1].ParamControlWord = KM_VERTEXPARAM_ENDOFSTRIP;
				kmxxGetCurrentPtr (&vertBuf);
				kmxxStartVertexStrip (&vertBuf);

				while (ptr < (PKMDWORD)v) 
				{
					prefetch (ptr + 8);
					*pkmCurrentPtr++ = *ptr++;
					*pkmCurrentPtr++ = *ptr++;
					*pkmCurrentPtr++ = *ptr++;
					*pkmCurrentPtr++ = *ptr++;
					prefetch (ptr + 8);
					*pkmCurrentPtr++ = *ptr++;
					*pkmCurrentPtr++ = *ptr++;
					*pkmCurrentPtr++ = *ptr++;
					*pkmCurrentPtr   = *ptr++;
					prefetch (pkmCurrentPtr);
					pkmCurrentPtr++;
				}
				
				kmxxReleaseCurrentPtr (&vertBuf);
			}
		}
	} // foreach band
	DrawTrails();
}

// Add a matrix to a band
static void bAdd (BandHandle bandH, Matrix *m)
{
	Band *b;
	Matrix43 *mat;
	dAssert (bandH >= 0 && bandH < MAX_BANDS, "Invalid band handle");
	dAssert (m != NULL, "Invalid matrix");

	b = &band[bandH];

	// Is there an overflow?
	if (b->curSegs == b->maxSegs) {
		// Yes: shift down the matrices and bung in on the end
		memmove (b->matList, b->matList + 1, sizeof (Matrix43) * (b->maxSegs - 1));
		mat = &b->matList[b->maxSegs - 1];
	} else {
		mat = &b->matList[b->curSegs];
		b->curSegs++;
	}
	mat->m[0][0] = m->m[0][0]; mat->m[0][1] = m->m[0][1]; mat->m[0][2] = m->m[0][2];
	mat->m[1][0] = m->m[1][0]; mat->m[1][1] = m->m[1][1]; mat->m[1][2] = m->m[1][2];
	mat->m[2][0] = m->m[2][0]; mat->m[2][1] = m->m[2][1]; mat->m[2][2] = m->m[2][2];
	mat->m[3][0] = m->m[3][0]; mat->m[3][1] = m->m[3][1]; mat->m[3][2] = m->m[3][2];
}

// Alter the top level of the band structure
static void bRubberBand (BandHandle bandH, Matrix *m)
{
	Band *b;
	Matrix43 *mat;
	dAssert (bandH >= 0 && bandH < MAX_BANDS, "Invalid band handle");
	dAssert (m != NULL, "Invalid matrix");

	b = &band[bandH];
	
	// Is there a toplevel matrix to edit
	if (b->curSegs == 0)
		return;
	mat = &b->matList[b->curSegs-1];
	mat->m[0][0] = m->m[0][0]; mat->m[0][1] = m->m[0][1]; mat->m[0][2] = m->m[0][2];
	mat->m[1][0] = m->m[1][0]; mat->m[1][1] = m->m[1][1]; mat->m[1][2] = m->m[1][2];
	mat->m[2][0] = m->m[2][0]; mat->m[2][1] = m->m[2][1]; mat->m[2][2] = m->m[2][2];
	mat->m[3][0] = m->m[3][0]; mat->m[3][1] = m->m[3][1]; mat->m[3][2] = m->m[3][2];
}

//////////////////////////
// Strat glue code
int CreateBand (STRAT *strat, int shapeType, int maxSegs, int mType,
				float a0, float r0, float g0, float b0,
				float a1, float r1, float g1, float b1)
{
	BandHandle bandH;
	LightingValue col[2];
	Matrix43 mat;

	col[0].a = a0; col[0].r = r0; col[0].g = g0; col[0].b = b0;
	col[1].a = a1; col[1].r = r1; col[1].g = g1; col[1].b = b1;

	mat.m[0][0] = 1.0f; mat.m[0][1] = 0.f; mat.m[0][2] = 0.f;
	mat.m[1][0] = 0.f; mat.m[1][1] = 1.0f; mat.m[1][2] = 0.f;
	mat.m[2][0] = 0.f; mat.m[2][1] = 0.f; mat.m[2][2] = 1.0f;
	mat.m[3][0] = 0.f; mat.m[3][1] = 0.f; mat.m[3][2] = 0.f;

	bandH = bCreate (shapeType, mType, maxSegs, col, &mat);
#ifdef _DEBUG
	dAssert (band[bandH].strat == NULL, "Attempt to re-use locked band");
	band[bandH].strat = strat;
#endif

	return bandH;
}

void DeleteBand (STRAT *strat, BandHandle bandH)
{
	if (bandH != -1) {
		dAssert (bandH >= 0 && bandH < MAX_BANDS, "Band out of range");
#ifdef _DEBUG
		dAssert (band[bandH].strat != NULL && band[bandH].strat == strat, "Attempt to delete already free band");
		band[bandH].strat = NULL;
#endif
		band[bandH].deleteMe = band[bandH].curSegs + 1;
	}
}

#ifdef _DEBUG
void BandCleanDebug(STRAT *strat)
{
	int i;
	for (i = 0; i < MAX_BANDS; ++i) {
		if (band[i].strat == strat) {
			dAssert (NULL, "Strat leaks a band");
			DeleteBand (strat, i);
		}
	}
	for (i = 0; i < MAX_TRAILS; ++i) {
		if (band[i].strat == strat) {
			dAssert (NULL, "Strat leaks a trail");
			DeleteTrail (strat, i);
		}
	}
}
#endif

int AnyFreeJonnies(int nBands, int nSegmentsPerBand)
{
	int i;
	// Firstly seek to find a spare entry in the band table
	for (i = 0; i < MAX_BANDS; ++i) {
		if (band[i].shape == NULL)
			nBands--;
		if (nBands == 0)
			break; // There are enough bands free
	}
	if (i == MAX_BANDS)
		return 0;	// No spare band could be found
	else {
		// Are there enough matrices to hold the segments?
		if (bandMatrixHWM + nBands * nSegmentsPerBand > MAX_BAND_MATRICES)
			return 0;
		else
			return 1;
	}
}


void AddBandSegment (STRAT *strat, BandHandle bandH, float x, float y, float z, float scale)
{
	Matrix m;
	if (bandH == -1)
		return;

	mTranslate(&m, strat->pos.x, strat->pos.y, strat->pos.z);
	mPreMultiply(&m, &strat->m);
	mPreTranslate (&m, x, y, z);
	mPreRotateX(&m, PI/2);
	mPreScale (&m, scale, scale, scale);

	bAdd (bandH, &m);
}

void RubberBand (STRAT *strat, BandHandle bandH, float x, float y, float z, float scale)
{
	Matrix m;
	if (bandH == -1)
		return;

	mTranslate(&m, strat->pos.x, strat->pos.y, strat->pos.z);
	mPreMultiply(&m, &strat->m);
	mPreTranslate (&m, x, y, z);
	mPreRotateX(&m, PI/2);
	mPreScale (&m, scale, scale, scale);

	bRubberBand (bandH, &m);
}

void SetBandColour (BandHandle bandH,
				float a0, float r0, float g0, float b0,
				float a1, float r1, float g1, float b1)
{
	Band *b;
	float rMaxSegs;

	if (bandH == -1)
		return;
	dAssert (bandH >= 0 && bandH < MAX_BANDS, "Band out of range");

	b = &band[bandH];

	// Store first colour
	b->colour.a = a0;
	b->colour.r = r0;
	b->colour.g = g0;
	b->colour.b = b0;

	// Calculate the increments
	rMaxSegs	= 1.f / b->maxSegs;
	b->cInc.a	= (a0 - a1) * rMaxSegs;
	b->cInc.r	= (r0 - r1) * rMaxSegs;
	b->cInc.g	= (g0 - g1) * rMaxSegs;
	b->cInc.b	= (b0 - b1) * rMaxSegs;
}

void SetBandRotXYZandScaling (BandHandle bandH, float rX, float rY, float rZ,
							  float sX, float sY, float sZ)
{
	Matrix m;
	Band *b;

	if (bandH == -1)
		return;
	dAssert (bandH >= 0 && bandH < MAX_BANDS, "Band out of range");

	b = &band[bandH];
	mRotateXYZ (&m, rX, rY, rZ);
	b->incMat.m[0][0] = m.m[0][0] * sX; b->incMat.m[0][1] = m.m[0][1] * sY; b->incMat.m[0][2] = m.m[0][2] * sZ;
	b->incMat.m[1][0] = m.m[1][0] * sX; b->incMat.m[1][1] = m.m[1][1] * sY; b->incMat.m[1][2] = m.m[1][2] * sZ;
	b->incMat.m[2][0] = m.m[2][0] * sX; b->incMat.m[2][1] = m.m[2][1] * sY; b->incMat.m[2][2] = m.m[2][2] * sZ;

}

////////////////////////
// New 2d effect
////////////////////////

// Returns whether a point (int world space) is visible
static Bool IsPointVisible (Point3 *p)
{
	Point3 collPt;
	return !lineLandscapeCollision (&currentCamera->pos, p, &collPt, NULL, NULL);
}

int CreateTrail (STRAT *strat, 
				 int type, int nSegs,
				 float a0, float r0, float g0, float b0,
				 float a1, float r1, float g1, float b1)
{
	register int i;
	float rSegs;
	for (i = 0; i < MAX_TRAILS; ++i)
		if (trail[i].pt == NULL)
			break;
	if (i==MAX_TRAILS)
		return -1;

	if (maxPt + nSegs > MAX_TRAIL_POINTS)
		return -1;

	rSegs = 1.f / nSegs;

	trail[i].pt = &p2Array[maxPt];
	trail[i].curPt = 0;
	trail[i].nPts = nSegs;
	trail[i].colour.a = a0;
	trail[i].colour.r = r0;
	trail[i].colour.g = g0;
	trail[i].colour.b = b0;
	trail[i].cInc.a = (a0 - a1) * rSegs;
	trail[i].cInc.r = (r0 - r1) * rSegs;
	trail[i].cInc.g = (g0 - g1) * rSegs;
	trail[i].cInc.b = (b0 - b1) * rSegs;
#ifdef _DEBUG
	dAssert (trail[i].strat == NULL, "Ack");
	trail[i].strat = strat;
#endif
	maxPt += nSegs;

	return i;
}
static PKMDWORD RecurseDrawTrailSeg(float xx, float yy, float lastX, float lastY,
									float a0, float r0, float g0, float b0,
									float aInc, float rInc, float gInc, float bInc, 
									float dist, float curSize, float prevSize, 
									PKMDWORD foo, int depth)
{
	float midX, midY, size;
	register PKMDWORD pkmCurrentPtr = foo;
	if (dist < SQR(TRAIL_GAP_MAX))
		return pkmCurrentPtr;
	// Find the midpoint
	midX = (xx + lastX) * 0.5f;
	midY = (yy + lastY) * 0.5f;
	aInc *= 0.5f;
	rInc *= 0.5f;
	gInc *= 0.5f;
	bInc *= 0.5f;
	dist *= 0.5f;
	size = (curSize + prevSize) * 0.5f;
	
	// Draw the midpoint
	kmxxSetVertex_5 (KM_VERTEXPARAM_NORMAL, midX - size, midY - size, 100.f, 0, 0,
		a0 + aInc, r0 + rInc, g0 + gInc, b0 + bInc, 0, 0 ,0 ,0);
	kmxxSetVertex_5 (KM_VERTEXPARAM_NORMAL, midX + size, midY - size, 100.f, 1, 0,
		a0 + aInc, r0 + rInc, g0 + gInc, b0 + bInc, 0, 0 ,0 ,0);
	kmxxSetVertex_5 (KM_VERTEXPARAM_NORMAL, midX - size, midY + size, 100.f, 0, 1,
		a0 + aInc, r0 + rInc, g0 + gInc, b0 + bInc, 0, 0 ,0 ,0);
	kmxxSetVertex_5 (KM_VERTEXPARAM_ENDOFSTRIP, midX + size, midY + size, 100.f, 1, 1,
		a0 + aInc, r0 + rInc, g0 + gInc, b0 + bInc, 0, 0 ,0 ,0);

	if (depth++ == TRAIL_MAX_DEPTH)
		return pkmCurrentPtr;

	// Recurse into the two halves
	pkmCurrentPtr = RecurseDrawTrailSeg (xx, yy, midX, midY,
		a0, r0, g0, b0,
		aInc, rInc, gInc, bInc,
		dist, curSize, size, pkmCurrentPtr, depth);
	pkmCurrentPtr = RecurseDrawTrailSeg (midX, midY, lastX, lastY,
		a0 + aInc, r0 + rInc, g0 + gInc, b0 + bInc,
		aInc, rInc, gInc, bInc,
		dist, size, prevSize, pkmCurrentPtr, depth);
	return pkmCurrentPtr;
}
void DrawTrails (void)
{
	Trail *t;
	int i;
	register float a0, r0, g0, b0, xx, yy, size, lastX, lastY;
	float dist, lastSize;

	for (i = 0, t = trail; i < MAX_TRAILS; ++i, ++t) {
		int lastDrawn = 0;
		// XXX needs optimizing
		if (t->pt && t->curPt) {
			register int i;
			size = t->size * pow((1.f / TRAIL_MUL), t->curPt);
			a0 = t->colour.a - t->cInc.a * t->nPts;
			r0 = t->colour.r - t->cInc.r * t->nPts;
			g0 = t->colour.g - t->cInc.g * t->nPts;
			b0 = t->colour.b - t->cInc.b * t->nPts;
			rSetMaterial (&flareMat);
			kmxxGetCurrentPtr (&vertBuf);
			kmxxStartVertexStrip (&vertBuf);
			for (i = 0; i < t->curPt; ++i) {
				xx = t->pt[i].x;
				yy = t->pt[i].y;
				dist = SQR(lastX - xx) + SQR(lastY - yy);
//				if (i && ((i - lastDrawn) < TRAIL_SKIP_MAX) && dist < SQR(TRAIL_GAP_MIN))
//					continue;
				kmxxSetVertex_5 (KM_VERTEXPARAM_NORMAL, xx - size, yy - size, 100.f, 0, 0,
					a0, r0, g0, b0, 0, 0 ,0 ,0);
				kmxxSetVertex_5 (KM_VERTEXPARAM_NORMAL, xx + size, yy - size, 100.f, 1, 0,
					a0, r0, g0, b0, 0, 0 ,0 ,0);
				kmxxSetVertex_5 (KM_VERTEXPARAM_NORMAL, xx - size, yy + size, 100.f, 0, 1,
					a0, r0, g0, b0, 0, 0 ,0 ,0);
				kmxxSetVertex_5 (KM_VERTEXPARAM_ENDOFSTRIP, xx + size, yy + size, 100.f, 1, 1,
					a0, r0, g0, b0, 0, 0 ,0 ,0);
				if (i && dist > SQR(TRAIL_GAP_MAX))
					pkmCurrentPtr = RecurseDrawTrailSeg (xx, yy, lastX, lastY, a0, r0, g0, b0, 
										 t->cInc.a, t->cInc.r, t->cInc.g, t->cInc.r, 
										 dist, size, lastSize, pkmCurrentPtr, 0);
				lastX = xx;
				lastY = yy;
				lastDrawn = i;
				lastSize = size;
				a0 += t->cInc.a;
				r0 += t->cInc.r;
				g0 += t->cInc.g;
				b0 += t->cInc.b;
				size *= TRAIL_MUL;
			}
			kmxxReleaseCurrentPtr (&vertBuf);
		}
	}
}

void UpdateTrail (STRAT *strat, int trailNo, float x, float y, float z)
{
	float rW;
	Trail *t;
	Matrix m;
	Point p;

	if (trailNo == -1)
		return;

	dAssert (trailNo >= 0 && trailNo < MAX_TRAILS, "Invalid trail handle");

	t = trail + trailNo;

	mTranslate(&m, strat->pos.x, strat->pos.y, strat->pos.z);
	mPreMultiply(&m, &strat->m);
	mPreTranslate (&m, x, y, z);

	mLoad (&mWorldToScreen);
	mPointXYZ (&p, m.m[3][0], m.m[3][1], m.m[3][2]);
	rW = 1.f / p.w;
	if ((p.z > 0) && IsPointVisible ((Point3 *)&m.m[3][0])) {
		if (t->curPt == t->nPts) {
			memmove (&t->pt[0], &t->pt[1], sizeof (Point2) * (t->nPts-1));
			t->curPt--;
		}
		t->pt[t->curPt].x = rW * p.x * pViewSize.x + pViewMidPoint.x;
		t->pt[t->curPt].y = rW * p.y * pViewSize.y + pViewMidPoint.y;
		t->size = (TRAIL_SIZE/2) * rW;
		t->curPt ++;
	} else {
		if (t->curPt) {
			memmove (&t->pt[0], &t->pt[1], sizeof (Point2) * (t->curPt-1));
			t->curPt--;
		}
	}
}

void DeleteTrail (STRAT *strat, int trailNo)
{
	Trail *t;
	int start;

	if (trailNo == -1)
		return;

	dAssert (trailNo >= 0 && trailNo < MAX_TRAILS, "Invalid trail handle");
#ifdef _DEBUG
	dAssert (trail[trailNo].strat && trail[trailNo].strat == strat, "Invalid trail handle");
	trail[trailNo].strat = NULL;
#endif

	t = &trail[trailNo];
	start = t->pt - p2Array;
	if ((start + t->nPts) == maxPt) {
		t->pt = NULL;
		maxPt = start;
	} else {
		int i;
		maxPt -= t->nPts;
		memmove (t->pt, t->pt + t->nPts, (maxPt - start) * sizeof (Point2));
		t->pt = NULL;
		for (i = 0; i < MAX_TRAILS; ++i) {
			if (trail[i].pt) {
				if ((trail[i].pt - p2Array) > start) {
					trail[i].pt -= t->nPts;
				}
			}
		}
	}

}

void moveTrailsBack(float amount)
{
	int i, j;
	
	
	for (i=0; i<MAX_BANDS; i++)
	{
		if (band[i].shape == NULL)
			continue;

		if (band[i].curSegs < 2)
			continue;

		for (j=0; j<band[i].curSegs; j++)
		{
			band[i].matList[j].m[3][1] -= amount;
		}
	}
}