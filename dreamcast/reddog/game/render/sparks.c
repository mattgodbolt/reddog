// Spark particle system
#include "..\Reddog.h"
#include "Internal.h"

#define MAX_SPARKS			128
#define MAX_SHIELD_SPARKS	16
// A spark is just a position and velocity, and a counter
typedef struct tagSpark
{
	int		counter;
	float	x, y, z;
	float	vx, vy, vz;
} Spark;

typedef struct tagShieldSpark
{
	Sint8	counter;
	Sint8	countDirChange;
	Sint8	curVertex;
	Sint8	prevVertex;
	float	x, y, z;
	float	x1, y1, z1;
	float	x2, y2, z2;
	float	vx, vy, vz;
} ShieldSpark;

// The array of sparks
static Spark spark[MAX_SPARKS];
static ShieldSpark shieldSpark[MAX_SHIELD_SPARKS];
Matrix ShieldSparkMatrix;
static Point3 *shieldVertex;
static int shieldNVerts;

static Material sparkMaterial = { 0, 0, 0.f, 0.f, MF_TRANSPARENCY_ADDITIVE | MF_ALPHA | MF_TWO_SIDED };
static Material shieldSparkMaterial = { 0, 0, 0.f, 0.f, MF_ALPHA | MF_TWO_SIDED };

#define MAX_SHIELD_VERTICES 256
#define MAX_SHIELD_ADJ 4 // pow2
static int shieldArray[MAX_SHIELD_ADJ][MAX_SHIELD_VERTICES];

// Reset/reinitialise the sparks
void rResetSparks (void)
{
	register int i;
	register Spark *s;
	register ShieldSpark *ss;

	for (i = MAX_SPARKS, s = spark; i != 0; --i, ++s)
		s->counter = 0;
	for (i = MAX_SHIELD_SPARKS, ss = shieldSpark; i != 0; --i, ++ss)
		ss->counter = 0;

	sparkMaterial.surf.GBIX = 0;
	rFixUpMaterial (&sparkMaterial, &sfxContext);
}

// Update the sparks
void rUpdateSparks (void)
{
	register int i;
	register Spark *s;
	register float *update, *read;
	register float x, y, z, xx, yy, zz;

	// Find the first spark and draw it
	for (i = MAX_SPARKS, s = spark; i != 0; --i, ++s) {
		if (s->counter == 0)
			continue;
		update= &s->x; read = &s->vx;
		xx = *update; *update++ = x = xx + *read++;
		yy = *update; *update++ = y = yy + *read++;
		zz = *update; *update   = z = zz + (*read -= 0.03f);
	}

}	
// Draw the sparks
void rDrawSparks (void)
{
	register int i;
	register Spark *s;
	register float *update, *read, x, y, z, w;
	register float xx, yy, zz, ww;
	float sX, sY, mX, mY, delX, delY, scale;
	int nSparks, visAmt;
	Uint32 colour = 0x00c8ff, colour2 = 0xffffff;
	Point temp;
	Matrix m;

	nSparks = 0;

	rSetMaterial (&sparkMaterial);
	kmxxGetCurrentPtr (&vertBuf);

	mLoad (&mWorldToScreen);
	mX = pViewMidPoint.x;
	mY = pViewMidPoint.y;
	sX = pViewSize.x;
	sY = pViewSize.y;

	// Find the first spark and draw it
	for (i = MAX_SPARKS, s = spark; i != 0; --i, ++s) {
		if (s->counter == 0)
			continue;
		// Perform the positional update
		update= &s->x; read = &s->vx;
		xx = *update++;	x = xx + *read++;
		yy = *update++;	y = yy + *read++;
		zz = *update;	z = zz + *read;

		// Transform into screen space
		mPointXYZ (&temp, x, y, z);
		read = &temp.x;
		x = *read++; y = *read++; z = *read++; w = *read++;
		if (z < 0 ||
			x < -w ||
			x > w ||
			y < -w ||
			y > w) 
		{
			// The spark is offscreen - make it disappear much quicker, and don't draw it
			s->counter -= 4;
			if (s->counter < 0)
				s->counter = 0;
		} else {
			// The spark is visible - so draw it!
			w = 1.f / w;	// Get scale factor
			if (nSparks++ == 0)
				kmxxStartVertexStrip (&vertBuf);	// If it's the first spark, then start the strip
			// Transform the end point
			mPointXYZ (&temp, xx, yy, zz);
			// For cheesey cheat points I use the same w value for both ends of the spark
			x  = mX + sX * w * x;
			y  = mY + sY * w * y;
			xx = mX + sX * w * temp.x;
			yy = mY + sY * w * temp.y;

			delY = xx - x;
			delX = y - yy;
			if (delX == 0.0f)
				delX = 0.01f;
			if (delY == 0.0f)
				delY = 0.01f;

			scale = 1.1f * isqrt (SQR(delX) + SQR(delY));

			delX *= scale;
			delY *= scale;

			visAmt = (s->counter) * (256/20);
			if (visAmt > 255)
				visAmt = 255;
			colour = (colour & ~0xff000000) | (visAmt << 24);
			colour2 = (colour2 & 0xffff) | (visAmt << 24) | ((int)(s->counter * 255)<<16);
			
			kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		x  + delX, y  + delY, w, colour);
			kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		xx + delX, yy + delY, w, colour2);
			kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		x  - delX, y  - delY, w, colour);
			kmxxSetVertex_0 (KM_VERTEXPARAM_ENDOFSTRIP,	xx - delX, yy - delY, w, colour2);
			
			s->counter--;
		}

	}

	// Shield sparks
	{
		ShieldSpark *s;
		Uint32 colour3, colour4;
		colour = 0xe0e0ff, colour2 = 0x2080b0;
		colour3 = 0x0040a0, colour4 = 0x001080;
		
		mMultiply (&m, &ShieldSparkMatrix, &mWorldToScreen);
		mLoad (&m);
		// Find the first spark and draw it
		for (i = MAX_SHIELD_SPARKS, s = shieldSpark; i != 0; --i, ++s) {
			if (s->counter == 0)
				continue;
			// Perform the positional update
			update= &s->x; read = &s->vx;
			if (!PauseMode) {
				xx = *update; *update++ = x = xx + *read++;
				yy = *update; *update++ = y = yy + *read++;
				zz = *update; *update   = z = zz + *read++;
			} else {
				xx = *update++;	x = xx + *read++;
				yy = *update++;	y = yy + *read++;
				zz = *update;	z = zz + *read;
			}
			// Transform into screen space
			mPointXYZ (&temp, x, y, z);
			read = &temp.x;
			x = *read++; y = *read++; z = *read++; w = *read++;
			// Have we reached our target?
			if (--s->countDirChange == 0) {
				float rC;
				Sint8 newVert;
				Point3 *to;
				// Choose a new vertex to head towards
				do {
					newVert = shieldArray[rand() & (MAX_SHIELD_ADJ-1)][s->curVertex];
				} while (newVert == s->prevVertex);
				s->prevVertex = s->curVertex;
				s->curVertex = newVert;
				to = &shieldVertex[s->curVertex];
				s->countDirChange = RandRange(2,4);
				rC = 1.f / s->countDirChange;
				s->vx = (to->x - s->x) * rC;
				s->vy = (to->y - s->y) * rC;
				s->vz = (to->z - s->z) * rC;
			}
			if (z < 0 ||
				x < -w ||
				x > w ||
				y < -w ||
				y > w) 
			{
				// The spark is offscreen - make it disappear much quicker, and don't draw it
				s->counter -= 4;
				if (s->counter < 0)
					s->counter = 0;
			} else {
				// The spark is visible - so draw it!
				w = 1.f / w;	// Get scale factor
				if (nSparks++ == 0)
					kmxxStartVertexStrip (&vertBuf);	// If it's the first spark, then start the strip
				// Transform the end point
				mPointXYZ (&temp, xx, yy, zz);
				// For cheesey cheat points I use the same w value for both ends of the spark
				x  = mX + sX * w * x;
				y  = mY + sY * w * y;
				xx = mX + sX * w * temp.x;
				yy = mY + sY * w * temp.y;
				
				delY = xx - x;
				delX = y - yy;
				if (delX == 0.0f)
					delX = 0.01f;
				if (delY == 0.0f)
					delY = 0.01f;
				scale = isqrt (SQR(delX) + SQR(delY));
				delX *= scale;
				delY *= scale;
				
				visAmt = (s->counter) * (256/40);
				if (visAmt > 128)
					visAmt = 128;
				colour = (colour & ~0xff000000) | (visAmt << 24);
				colour2 = (colour2 & ~0xff000000) | (visAmt << 24);
				colour3 = (colour3 & ~0xff000000) | (visAmt << 24);
				colour4 = (colour4 & ~0xff000000) | (visAmt << 24);

				// First line
				kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		x  + delX, y  + delY, w, colour);
				kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		xx + delX, yy + delY, w, colour2);
				kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		x  - delX, y  - delY, w, colour);
				kmxxSetVertex_0 (KM_VERTEXPARAM_ENDOFSTRIP,	xx - delX, yy - delY, w, colour2);
				
				// Second line
				mPointXYZ (&temp, s->x1, s->y1, s->z1);
				xx = mX + sX * w * temp.x;
				yy = mY + sY * w * temp.y;
				
				delY = xx - x;
				delX = y - yy;
				if (delX == 0.0f)
					delX = 0.01f;
				if (delY == 0.0f)
					delY = 0.01f;
				scale = isqrt (SQR(delX) + SQR(delY));
				delX *= scale;
				delY *= scale;

				kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		x  + delX, y  + delY, w, colour);
				kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		xx + delX, yy + delY, w, colour2);
				kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		x  - delX, y  - delY, w, colour);
				kmxxSetVertex_0 (KM_VERTEXPARAM_ENDOFSTRIP,	xx - delX, yy - delY, w, colour2);

				// Third line
				mPointXYZ (&temp, s->x2, s->y2, s->z2);
				x = mX + sX * w * temp.x;
				y = mY + sY * w * temp.y;
				
				delY = xx - x;
				delX = y - yy;
				if (delX == 0.0f)
					delX = 0.01f;
				if (delY == 0.0f)
					delY = 0.01f;
				scale = isqrt (SQR(delX) + SQR(delY));
				delX *= scale;
				delY *= scale;

				kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		x  + delX, y  + delY, w, colour);
				kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		xx + delX, yy + delY, w, colour2);
				kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		x  - delX, y  - delY, w, colour);
				kmxxSetVertex_0 (KM_VERTEXPARAM_ENDOFSTRIP,	xx - delX, yy - delY, w, colour2);


				if (player[currentPlayer].ShieldHold)
					s->counter--;
				else
					s->counter = MAX(0, s->counter - 8);

				s->x2 = s->x1;
				s->y2 = s->y1;
				s->z2 = s->z1;
				s->x1 = s->x;
				s->y1 = s->y;
				s->z1 = s->z;
			}
		}
	}
#if COUNT_GEOMETRY
	nDrawn += 2 * nSparks;
#endif

	kmxxReleaseCurrentPtr (&vertBuf);
}

// Strat glue code
void MakeSpark (STRAT *strat, float x, float y, float z)
{
	Matrix m;
	float velocity;
	Vector3 vector;
	register int i;

	if (strat) {
		mTranslate(&m, strat->pos.x, strat->pos.y, strat->pos.z);
		mPreMultiply(&m, &strat->m);
		mPreTranslate (&m, x, y, z);
		x = m.m[3][0];
		y = m.m[3][1];
		z = m.m[3][2];
	}

	v3Rand (&vector);
	v3Scale (&vector, &vector, 8.f / FRAMES_PER_SECOND);
	vector.z = fabs (vector.z);

	velocity = RandRange (0.3, 0.8);
	for (i = 0; i < MAX_SPARKS; ++i) {
		if (spark[i].counter)
			continue;
		spark[i].counter = 20;
		spark[i].vx = vector.x * velocity;
		spark[i].vy = vector.y * velocity;
		spark[i].vz = fabs(vector.z);
		spark[i].x = x;
		spark[i].y = y;
		spark[i].z = z;
		break;
	}
}

// Shield sparks
void MakeShieldSpark (void)
{
	register float rC;
	register int i, j, k;
	float tX, tY, tZ, x, y, z;
	float multiply;

	j = RandRange (0, shieldNVerts);
	k = shieldArray[rand() & (MAX_SHIELD_ADJ-1)][j];

	if (player[currentPlayer].ShieldHold && player[currentPlayer].ShieldHold < 10)
		multiply = 1.f + ((10.f - player[currentPlayer].ShieldHold) * 0.2f);
	else
		multiply = 1.f;

	x = shieldVertex[j].x * multiply;
	y = shieldVertex[j].y;
	z = shieldVertex[j].z * multiply;
	tX = shieldVertex[k].x;
	tY = shieldVertex[k].y;
	tZ = shieldVertex[k].z;

	for (i = 0; i < MAX_SHIELD_SPARKS; ++i) {
		if (shieldSpark[i].counter)
			continue;
		shieldSpark[i].counter = RandRange(20, 40);
		shieldSpark[i].countDirChange = RandRange(2, 4);
		shieldSpark[i].curVertex = j;
		shieldSpark[i].prevVertex = -1;
		rC = 1.f / shieldSpark[i].countDirChange;
		shieldSpark[i].vx = (tX - x) * rC;
		shieldSpark[i].vy = (tY - y) * rC;
		shieldSpark[i].vz = (tZ - z) * rC;
		shieldSpark[i].x = x;
		shieldSpark[i].y = y;
		shieldSpark[i].z = z;
		shieldSpark[i].x1 = x;
		shieldSpark[i].y1 = y;
		shieldSpark[i].z1 = z;
		shieldSpark[i].x2 = x;
		shieldSpark[i].y2 = y;
		shieldSpark[i].z2 = z;
		break;
	}

}

// Create the lookup table for the shield 
static void AddAdjacency (int shieldN[], int a, int b)
{
	register int i;
	float dX, dZ;

	if (shieldN[a] == MAX_SHIELD_ADJ)
		return;

	for (i = 0; i < shieldN[a]; ++i)
		if (shieldArray[i][a] == b)
			return;

	dX = fabs (shieldVertex[a].x - shieldVertex[b].x);
	dZ = fabs (shieldVertex[a].z - shieldVertex[b].z);

	if (dX < dZ) {
		float temp = dX;
		dX = dZ;
		dZ = temp;
	}

	if ((dX / dZ) > 2.f)
		shieldArray[shieldN[a]++][a] = b;
}
void MakeShieldSparkTable (Model *shieldModel)
{
	register int i, numStrip, prevVert, vert, prevPrevVert;
	register Point3 *vertex = shieldModel->vertex;
	register float xD, zD;
	VertRef *ref;
	int shieldN[MAX_SHIELD_VERTICES];

	dAssert (shieldModel->nVerts < MAX_SHIELD_VERTICES, "Ook");

	shieldVertex = vertex;
	shieldNVerts = shieldModel->nVerts;

	for (i = 0; i < MAX_SHIELD_VERTICES; ++i)
		shieldN[i] = 0;

	ref = shieldModel->strip;
	for (;;) {
		// Ignore material
		ref++;
		numStrip = *ref++;
		if (numStrip == 0)
			break;
		prevVert = prevPrevVert = -1;
		for (i = numStrip; i != 0; --i) {
			vert = *ref++ / sizeof (Point3);
			if (prevVert != -1) {
				AddAdjacency (shieldN, vert, prevVert);
				AddAdjacency (shieldN, prevVert, vert);
			}
			if (prevPrevVert != -1) {
				AddAdjacency (shieldN, vert, prevPrevVert);
				AddAdjacency (shieldN, prevPrevVert, vert);
			}
			prevPrevVert = prevVert;
			prevVert = vert;
		}
	}
	for (i = 0; i < shieldNVerts; ++i) {
		int j;
		dAssert (shieldN[i] >= 2, "Oops");
		if (shieldN[i] != MAX_SHIELD_ADJ) {
			for (j = 0; j < (MAX_SHIELD_ADJ - shieldN[i]); ++j)
				shieldArray[j+shieldN[i]][i] = shieldArray[j][i];
		}
	}
}
