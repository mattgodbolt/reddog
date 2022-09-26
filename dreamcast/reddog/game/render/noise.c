/*
 * Noise.c
 * Noise function
 */
#include "..\RedDog.h"
#include "Internal.h"
#include "Rasterisers.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

static Material	noiseMat = { 0, 0, 0.f, 0.f, MF_ALPHA | MF_TWO_SIDED | MF_TEXTURE_BILINEAR };

float SeaNoise (float x, float y);
float WatNoise (float x, float y);
float LavaNoise (float x, float y);

float rSeaLevel = 73.f;
float rWaveHeight = 3.f;
float rSeaChoppiness = 0.04f;

float rLavaHeight		= 1.f;
float rLavaChoppiness	= 0.02f;
float rWaterHeight		= 1.5f;
float rWaterChoppiness	= 0.06f;
Bool rSeaOn = FALSE;

#define WATER_UNITS_X		8
#define WATER_UNITS_Y		32
#define NOISE_SCALE_SEA		0.022f
#define NOISE_SCALE_WATER	0.088f
#define NOISE_SCALE_LAVA	0.044f

struct {
	Point3	p;
	float	brightness;
} WaterHeight[WATER_UNITS_Y][WATER_UNITS_X];

typedef struct {
	/* Static stuff */
	float	x, y;			/* Position in world space */
	float	waveHeight;		/* The height of the frame */
	float	waveLength;		/* The wavelength of the wave itself */
	float	waveSpeed;		/* The transverse speed of the wave */
	float	attenSub;		/* Amount to increment attenuation each frame */
	/* Dynamic & derived stuff */
	float	rWaveLength;	/* 1 / the wavelength of the wave itself */
	float	distAtten;		/* Distance from radius attenuation */
	float	radius;			/* Current frame's radius */
	float	attenuation;	/* Attenuation of the wave */
} WaterSplash;

#define MAX_SPLASHES 4
static WaterSplash splashes[MAX_SPLASHES];
static int nSplashes = 0;

/*
 * A one tonne thing crashing down into the sea at 30 m/s should generate
 * a 4 metre splash travelling at 10 metres per second
 */
void rMakeWave (float x, float y, float momentum, float radius)
{
	WaterSplash *s;
	if (nSplashes == MAX_SPLASHES)
		return;

	s = &splashes[nSplashes++];
	s->x			= x;
	s->y			= y;
	s->attenSub		= 0.002f * (60.f / (float)FRAMES_PER_SECOND);
	s->attenuation	= 1.f;
	s->waveHeight	= momentum;
	s->waveLength	= 20.f;
	s->rWaveLength	= 1.f / s->waveLength;
	s->distAtten	= 0.05f;
	s->waveSpeed	= 0.5f;
	s->radius		= radius;

}
// Strat glue function
void MakeWaterRipple (STRAT *strat, float momentum, float radius)
{
	rMakeWave (strat->pos.x, strat->pos.y, momentum, radius);
}

PKMDWORD rPoint (KMDWORD param, Point3 *pIn, PKMDWORD pkmCurrentPtr, float brightness)
{
	Point p;
	float diff, spec;
	float u, v;

	p.x = pIn->x;
	p.y = pIn->y;
	p.z = pIn->z;
	p.w = 1.f;

	u = pIn->x * 0.06f;
	v = pIn->y * 0.06f;

	mPoint (&p, &p);

/*	dAssert (p.z > 0, "Erk");*/

	p.z = 1.f / p.w;
	p.x = X_MID + X_MID * p.z * p.x;
	p.y = Y_MID + Y_MID * p.z * p.y;

	diff = 0.3 * brightness + 0.9;
	spec = 0.3 * brightness * brightness;
/*	r = g = b = rand() & 0xff;*/
	kmxxSetVertex_7 (param, p.x, p.y, p.z, u, v, diff, spec);

	return pkmCurrentPtr;
}

#pragma section Matrices
Matrix viewToWorld;
#pragma section

void rDrawWater (void)
{
	float x, dist, stepSize, increment;
	float multiplicand;
	int i, j;
	Point3 p;

	/*
	 * Update the wave positions
	 */
	for (i = 0; i < nSplashes; ++i) {
		WaterSplash *s = &splashes[i];
		s->attenuation -= s->attenSub;
		if (s->attenuation < 0.f) {
			if (i != (MAX_SPLASHES-1))
				memmove(s, &splashes[i+1], ((MAX_SPLASHES-1)-i) * sizeof (WaterSplash));
			--i;
			--nSplashes;
		} else {
			s->radius += s->waveSpeed;
		}
	}
	
	if (rSeaOn) {
		/*
		 * Take points sampled exponentially from screen space
		 * into world space to discover their x and y position.
		 * From the X & Y position, a height can be calculated, which is then
		 * thrown back through the worldToScreen matrix to yield screen-space points to
		 * draw
		 */
		mInvert3d (&viewToWorld, &mWorldToView);
		mLoad (&viewToWorld);
		increment = farDist / (float)(WATER_UNITS_Y);
		multiplicand = 1.1f / mPersp.m[0][0];
		for (j = 0; j < WATER_UNITS_Y; ++j) {
			dist = increment * (float)j + 4.0f;
			stepSize = dist * (2.f / (WATER_UNITS_X-1)) * multiplicand;
			for (i = 0; i < WATER_UNITS_X; ++i) {
				float noise;
				x = -dist * multiplicand + i * stepSize;
				p.x = x;
				p.y = 0;
				p.z = dist;
				mPoint3 (&p, &p);
				noise = SeaNoise (p.x, p.y);
				WaterHeight[j][i].p.x = p.x;
				WaterHeight[j][i].p.y = p.y;
				WaterHeight[j][i].p.z = rSeaLevel + noise;
				noise = noise + 1.8f;
				WaterHeight[j][i].brightness = RANGE(0,noise,1);
				/*
				DrawMarker (p.x, p.y, WaterHeight[j][i].p.z);
				mLoad (&viewToWorld);*/
			}
		}
		
		rSetMaterial (&noiseMat);
		mLoad (&mWorldToScreen);

		kmxxGetCurrentPtr (&vertBuf);
		kmxxStartVertexStrip (&vertBuf);
		for (j = 0; j < (WATER_UNITS_Y-1); ++j) {
			for (i = 0; i < (WATER_UNITS_X-1); ++i) {
				pkmCurrentPtr = rPoint (KM_VERTEXPARAM_NORMAL, &WaterHeight[j][i].p, pkmCurrentPtr, WaterHeight[j][i].brightness);
				pkmCurrentPtr = rPoint (KM_VERTEXPARAM_NORMAL, &WaterHeight[j+1][i].p, pkmCurrentPtr, WaterHeight[j+1][i].brightness);
			}
			pkmCurrentPtr = rPoint (KM_VERTEXPARAM_NORMAL, &WaterHeight[j][i].p, pkmCurrentPtr, WaterHeight[j][i].brightness);
			pkmCurrentPtr = rPoint (KM_VERTEXPARAM_ENDOFSTRIP, &WaterHeight[j+1][i].p, pkmCurrentPtr, WaterHeight[j+1][i].brightness);
		}
		kmxxReleaseCurrentPtr (&vertBuf);

#if 0
		for (j = 0; j < (WATER_UNITS_Y-1); ++j) {
			for (i = 0; i < (WATER_UNITS_X-1); ++i) {
				DrawTriangle (&WaterHeight[j][i].p, &WaterHeight[j+1][i].p, &WaterHeight[j][i+1].p);
				DrawTriangle (&WaterHeight[j+1][i].p, &WaterHeight[j][i+1].p, &WaterHeight[j+1][i+1].p);
			}
		}
#endif
	}
}

/*
 * WaterCollision
 * Returns 0 if above water, else number of metres under water
 */
float rWaterCollision (Point3 *p)
{
	float height;
	/*
	 * Early out
	 */
	if (p->z > (rSeaLevel + rWaveHeight))
		return 0.f;

	height = rSeaLevel + SeaNoise(p->x, p->y);
	if (p->z > height)
		return 0;
	return height - p->z;
}

/* -------------------------- */

#define B 0x100
#define BM 0xff

#define N 0x1000
#define NP 12   /* 2^N */
#define NM 0xfff

static p[B + B + 2];
static float g3[B + B + 2][3];

#define s_curve(t) ( t * t * (3. - 2. * t) )

#define lerp(t, a, b) ( a + t * (b - a) )

#define setup(i,b0,b1,r0,r1)\
	t = vec[i] + N;\
	b0 = ((int)t) & BM;\
	b1 = (b0+1) & BM;\
	r0 = t - (int)t;\
r1 = r0 - 1.;

float rNoise (float vec[3])
{
	register int i, j;
	register float *q;
	register float t, u, v, a, b, c, d;
	float rx0, rx1, ry0, ry1, rz0, rz1, sy, sz;
	int bx0, bx1, by0, by1, bz0, bz1, b00, b10, b01, b11;
	
	setup(0, bx0,bx1, rx0,rx1);
	setup(1, by0,by1, ry0,ry1);
	setup(2, bz0,bz1, rz0,rz1);
	
	i = p[ bx0 ];
	j = p[ bx1 ];
	
	b00 = p[ i + by0 ];
	b10 = p[ j + by0 ];
	b01 = p[ i + by1 ];
	b11 = p[ j + by1 ];
	
	t  = s_curve(rx0);
	sy = s_curve(ry0);
	sz = s_curve(rz0);
	
#define at3(u,rx,ry,rz) { u = rx * *q++; u += ry * *q++; u += rz * *q; }
	
	q = g3[ b00 + bz0 ] ; at3(u,rx0,ry0,rz0);
	q = g3[ b10 + bz0 ] ; at3(v,rx1,ry0,rz0);
	a = lerp(t, u, v);
	
	q = g3[ b01 + bz0 ] ; at3(u,rx0,ry1,rz0);
	q = g3[ b11 + bz0 ] ; at3(v,rx1,ry1,rz0);
	b = lerp(t, u, v);
	
	c = lerp(sy, a, b);
	
	q = g3[ b00 + bz1 ] ; at3(u,rx0,ry0,rz1);
	q = g3[ b10 + bz1 ] ; at3(v,rx1,ry0,rz1);
	a = lerp(t, u, v);
	
	q = g3[ b01 + bz1 ] ; at3(u,rx0,ry1,rz1);
	q = g3[ b11 + bz1 ] ; at3(v,rx1,ry1,rz1);
	b = lerp(t, u, v);
	
	d = lerp(sy, a, b);
	
	return lerp(sz, c, d);
}

float SeaNoise (float x, float y)
{
	float v[3];
	float dist, ringDist;
	float base;
	float attenuation, h;
	SinCosVal val;
	int i;

	v[0] = x * NOISE_SCALE_SEA;
	v[1] = y * NOISE_SCALE_SEA;
	v[2] = rSeaChoppiness * currentFrame;

	base = rNoise(v) * rWaveHeight;

	for (i = 0; i < nSplashes; ++i) {
		WaterSplash *s = &splashes[i];
		dist = sqrt (SQR(x - s->x) + SQR(y - s->y));
#if 0
		ringDist = fabs (dist - s->radius);
		attenuation = 1 - (ringDist * s->distAtten);
		if (attenuation < 0)
			continue;
		base += s->waveHeight * attenuation * s->attenuation;
#else
		ringDist = fabs (dist - s->radius);
		attenuation = 1 - (ringDist * s->distAtten);
		if (attenuation < 0)
			continue;
		h = sin ((dist - s->radius) * s->rWaveLength);
		base += s->waveHeight * h * attenuation * s->attenuation;
#endif
	}

	return base;
}

float WatNoise (float x, float y)
{
	return SeaNoise(x,y);
#if 0
	float v[3];

	v[0] = x * NOISE_SCALE_WATER;
	v[1] = y * NOISE_SCALE_WATER;
	v[2] = rWaterChoppiness * currentFrame;

	return rNoise(v) * rWaterHeight;
#endif
}

float LavaNoise (float x, float y)
{
	float v[3];

	v[0] = x * NOISE_SCALE_LAVA;
	v[1] = y * NOISE_SCALE_LAVA;
	v[2] = rLavaChoppiness * currentFrame;

	return rNoise(v);
}

static void normalize3(float v[3])
{
	float s;
	
	s = isqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	v[0] = v[0] * s;
	v[1] = v[1] * s;
	v[2] = v[2] * s;
}

void rInitNoise(void)
{
	int i, j, k;
	
	srand(898555555);

	for (i = 0 ; i < B ; i++) {
		p[i] = i;
				
		for (j = 0 ; j < 3 ; j++)
			g3[i][j] = (float)((rand() % (B + B)) - B) / B;
		normalize3(g3[i]);
	}
	
	while (--i) {
		k = p[i];
		p[i] = p[j = rand() % B];
		p[j] = k;
	}
	
	for (i = 0 ; i < B + 2 ; i++) {
		p[B + i] = p[i];
		for (j = 0 ; j < 3 ; j++)
			g3[B + i][j] = g3[i][j];
	}

}

void rSeaEffectOn(Material *m)
{
/*	noiseMat.surf.GBIX = texFindFromDesc (m->surf.surfaceDesc);
	rFixUpMaterial (&noiseMat, &sfxContext);*/
	memcpy (&noiseMat.info, &m->info, sizeof (MaterialInfo));
	rSeaOn = TRUE;
}

void rSeaEffectOff(void)
{
	rSeaOn = FALSE;
}

/*
 * Applies the sea effect to a model
 * mCurMatrix must be up to date
 */
void rWaterModel (Model *model)
{
	register int i;
	register Point3 *v;
	Point3 p;
	register float minZ = 100000.f, maxZ = -100000.f, height;

	dAssert (model != NULL, "NULL model passed to r*Model");

	mLoad (&mCurMatrix);

	v = model->vertex;
	for (i = model->nVerts; i != 0; --i)
	{
		p = *v;
		mPoint3 (&p, &p);
		height = WatNoise (p.x, p.y);
		v->z = height;
		if (height < minZ)
			minZ = height;
		if (height > maxZ)
			maxZ = height;
		v++;
	}
	model->bounds.low.z = minZ;
	model->bounds.hi.z = maxZ;
}

/*
 * Applies the flag effect to a model
 * Z axis is changed only
 * Flag should extend along the Y axis, with Y=0 at the top of the flag
 */
void rFlagModel (Model *model)
{
	register int i;
	register Point3 *v;
	Point3 p;
	register Vector3 *norm;
	register float minZ = 100000.f, maxZ = -100000.f, height, offset, clampedY;
	register float yScale, yOff;
	SinCosVal valX, valY;

	dAssert (model != NULL, "NULL model passed to r*Model");

	offset = (currentFrame & 0x3f) * (2 * PI * (1.f/64.f));

	// I offset with the x and y world position
	offset += mCurMatrix.m[3][0] + mCurMatrix.m[3][1];

	yOff = model->bounds.low.y;
	yScale = 1.2f / (model->bounds.hi.y - model->bounds.low.y);
	v = model->vertex;
	norm = model->vertexNormal;
	for (i = model->nVerts; i != 0; --i)
	{
		SinCos (offset + v->x, &valX);
		clampedY = (v->y - yOff) * yScale;
		SinCos (offset + v->y, &valY);
		if (clampedY > 1.f)
			clampedY = 1.f;
		height = (valX.sin * 0.2f + valY.cos) * clampedY;
		v->z = height;
		if (height < minZ)
			minZ = height;
		if (height > maxZ)
			maxZ = height;
		v++;
		p.x = valX.cos * 0.2f;
		p.y = valY.cos * 0.35f;
		p.z = 1.f;
		v3Normalise(&p);
		*norm++ = p;
	}
	model->bounds.low.z = minZ;
	model->bounds.hi.z = maxZ;
}


/*
 * Applies the lava effect to a model
 * mCurMatrix must be up to date
 */
Point2 lavaUVArray[MAX_VERTICES];
void rFlame (Point3 *pos, float height, float radius);
void rLavaModel (Model *model)
{
	register int i;
	register Point3 *v;
	register Point2 *uvPtr;
	Point3 p;
	register float x, y;
	float posX, posY;
	float minZ = 100000.f, maxZ = -100000.f, height, *uv;
	VertRef *vert;
	SinCosVal val;

	dAssert (model != NULL, "NULL model passed to r*Model");

	mLoad (&mCurMatrix);

	SinCos (currentFrame * 0.01f, &val);

	posX = 0.03f * val.cos;
	posY = -0.1f * val.sin;

	v = model->vertex;
	uvPtr = lavaUVArray;
	for (i = model->nVerts; i != 0; --i)
	{
		p = *v;
		x = (p.x) * (1.f / 40.f) + posX;
		y = (p.y) * (1.f / 40.f) + posY;
		mPoint3 (&p, &p);
		height = LavaNoise (p.x, p.y);
		height *= rLavaHeight;
		v->z = height;

		uvPtr->x = x;
		uvPtr->y = y;
		if (height < minZ)
			minZ = height;
		uvPtr++;
		if (height > maxZ)
			maxZ = height;
		v++;
	}
	model->bounds.low.z = minZ;
	model->bounds.hi.z = maxZ;

	uv = (float *)model->uv;

	vert = model->strip;
	do {
		vert++;
		i = *vert++;
		if (i==0)
			break;
		for (; i > 0 ; --i) {
			uvPtr = (Point2 *)((int)&lavaUVArray[0] + (((*vert++)/12)*8));
			*uv++ = uvPtr->x;
			*uv++ = uvPtr->y;
		}
	} while (1);
}
