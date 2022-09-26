#include "..\RedDog.h"
#include "..\View.h"
#include "..\Coll.h"
#include "..\GameState.h"
#include "..\Input.h"
#include "Internal.h"
#include "Rasterisers.h"
#include "..\Layers.h"

void rFXinit()
{
	/*CLEAN THE BAND SYSTEM*/
	bClean();

	/*RESET VIS MASKS*/
	rScapeVisMask = 0xffffffff;

}

/*
 * Fades the screen to colour Col
 */
bool OverrideMPFadeHack;
void rFade (Colour colour, float amt)
{
	KMVERTEX0		vert[4];
	KMSTATUS		result;
	Colour			realCol;
	extern Material fadeMat, addMat;
	float z0, z1;

	if (Multiplayer && !OverrideMPFadeHack) {
		z0 = W_LAYER_FLARE_MP;
		z1 = W_LAYER_FADE_MP;
	} else {
		z0 = W_LAYER_FLARE;
		z1 = W_LAYER_FADE;
	}

	// Do the flares first
	if (rFlareR >= 0.01f || rFlareG >= 0.01f || rFlareB >= 0.01f) {
		rFlareR = RANGE (0, rFlareR, 1);
		rFlareG = RANGE (0, rFlareG, 1);
		rFlareB = RANGE (0, rFlareB, 1);

		rSetMaterial (&addMat);
		realCol.argb.a = 255;
		realCol.argb.r = rFlareR * 255.f;
		realCol.argb.g = rFlareG * 255.f;
		realCol.argb.b = rFlareB * 255.f;

		/* Begin the triangle strip */
		kmxxGetCurrentPtr (&vertBuf);
		kmxxStartVertexStrip (&vertBuf);

		kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL, 0, 0, z0, realCol.colour);
		kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL, 0, PHYS_SCR_Y, z0, realCol.colour);
		kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL, PHYS_SCR_X, 0, z0, realCol.colour);
		kmxxSetVertex_0 (KM_VERTEXPARAM_ENDOFSTRIP, PHYS_SCR_X, PHYS_SCR_Y, z0, realCol.colour);
		
		kmxxReleaseCurrentPtr (&vertBuf);
	}

	if (amt >= 0.01f) {
		realCol			= colour;
		realCol.argb.a	= ((char)(RANGE(0, amt, 1) * 255.f));

		rSetMaterial (&fadeMat);

		/* Begin the triangle strip */
		kmxxGetCurrentPtr (&vertBuf);
		kmxxStartVertexStrip (&vertBuf);

		kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL, 0, 0, z1, realCol.colour);
		kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL, 0, PHYS_SCR_Y, z1, realCol.colour);
		kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL, PHYS_SCR_X, 0, z1, realCol.colour);
		kmxxSetVertex_0 (KM_VERTEXPARAM_ENDOFSTRIP, PHYS_SCR_X, PHYS_SCR_Y, z1, realCol.colour);
		
		kmxxReleaseCurrentPtr (&vertBuf);
	}
}

static Fragment fragArray[MAX_FRAGMENTS];
static Uint32 fragHighWaterMark = 0; /* Highest allocated frag */
static Uint32 fragLowWaterMark = 0; /* Lowest free frag */

/*
 * ResetFragments - reset the fragment buffer
 */

Material	fragMats[FT_MAX] =
{
	{ 0, 0, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA | MF_TRANSPARENCY_SUBTRACTIVE | MF_TWO_SIDED },
	{ 0, 0, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA | MF_TRANSPARENCY_ADDITIVE | MF_TWO_SIDED },
	{ 0, 0, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA | MF_TWO_SIDED },
	{ 0, 0, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA | MF_TRANSPARENCY_ADDITIVE | MF_TWO_SIDED },
};

void rResetFragments (void)
{
	memset (fragArray, 0, sizeof (Fragment) * MAX_FRAGMENTS);
	fragHighWaterMark = 0;
	fragLowWaterMark = 0;
	rFixUpMaterial (&fragMats[3], &sfxContext);
}

/*
 * Create fragments (give centre, radius and amount)
 */
void rCreateFragments (const Point3 *centre, float radius, Uint32 amount)
{
	Uint32 i;

	i = fragLowWaterMark;
	while (amount--) {
		float randAmt;
		/* Find a free fragment */
		for (; i < MAX_FRAGMENTS; ++i) {
			if (fragArray[i].lifeTimer == 0)
				break;
		}
		/* Did we find a fragment? */
		if (i>=MAX_FRAGMENTS)
			return;

		if (i > fragHighWaterMark)
			fragHighWaterMark = i;
		
		if (i == fragLowWaterMark)
			fragLowWaterMark++;
		
		randAmt = (rand() & 127) * (1.f / 255.f);

		fragArray[i].lifeTimer = (Uint32)(30.f + 5.f * randAmt);
		v3Rand (&fragArray[i].pos);
		v3Scale (&fragArray[i].posV, &fragArray[i].pos, 0.25f);

		v3Scale (&fragArray[i].pos, &fragArray[i].pos, radius * randAmt);
		v3Inc (&fragArray[i].pos, centre);

		fragArray[i].rot.x = PI2 * randAmt;
		fragArray[i].rot.y = fragArray[i].rot.x * 0.373f;
		fragArray[i].rot.z = fragArray[i].rot.x * 0.731f;
		v3Scale (&fragArray[i].rotV, &fragArray[i].rot, 0.1f);

		fragArray[i].col.colour = 0xffffffff;
		fragArray[i].type = FT_FALL;
	}
}

/*
 * Create trail fragments (give centre, radius and amount)
 */
void rCreateTrailFragments (const Point3 *centre, float radius, Uint32 amount)
{
	Uint32 i;
	Uint8 col;

	i = fragLowWaterMark;
	while (amount--) {
		float randAmt;
		/* Find a free fragment */
		for (; i < MAX_FRAGMENTS; ++i) {
			if (fragArray[i].lifeTimer == 0)
				break;
		}
		/* Did we find a fragment? */
		if (i>=MAX_FRAGMENTS)
			return;

		if (i > fragHighWaterMark)
			fragHighWaterMark = i;
		
		if (i == fragLowWaterMark)
			fragLowWaterMark++;
		
		randAmt = (rand() & 127) * (1.f / 255.f);

		fragArray[i].lifeTimer = (Uint32)(10.f);
		v3Rand (&fragArray[i].pos);
		v3Scale (&fragArray[i].posV, &fragArray[i].pos, 0.001f);

		v3Scale (&fragArray[i].pos, &fragArray[i].pos, radius * randAmt);
		v3Inc (&fragArray[i].pos, centre);

		fragArray[i].rot.x = PI2 * randAmt;
		fragArray[i].rot.y = fragArray[i].rot.x * 0.373f;
		fragArray[i].rot.z = fragArray[i].rot.x * 0.731f;
		v3Scale (&fragArray[i].rotV, &fragArray[i].rot, 0.1f);

		col = 0xc0 + (rand() & 0x3f);
		fragArray[i].col.colour = 0xff000000 | col | (col<<8) | (col << 16);
		fragArray[i].type = FT_NOGRAV;
	}
}

/*
 * Create cone frags for steam / sprinklers etc
 */
void rCreateConeFrags (const Point3 *centre, const Vector3 *dir, float cosAngle, float velMax, int amount)
{
	Uint32 i;

	i = fragLowWaterMark;
	while (amount--) {
		float randAmt;
		/* Find a free fragment */
		for (; i < MAX_FRAGMENTS; ++i) {
			if (fragArray[i].lifeTimer == 0)
				break;
		}
		/* Did we find a fragment? */
		if (i>=MAX_FRAGMENTS)
			return;

		if (i > fragHighWaterMark)
			fragHighWaterMark = i;
		
		if (i == fragLowWaterMark)
			fragLowWaterMark++;
		
		randAmt = (rand() & 127) * (1.f / 255.f);

		fragArray[i].lifeTimer = 25;
		fragArray[i].pos = *centre;
		do {
			v3Rand (&fragArray[i].posV);
		} while (v3Dot (&fragArray[i].posV, dir) < cosAngle);

		v3Scale (&fragArray[i].posV, &fragArray[i].posV, velMax * randAmt);

		fragArray[i].rot.x = PI2 * randAmt;
		fragArray[i].rot.y = fragArray[i].rot.x * 0.673f;
		fragArray[i].rot.z = fragArray[i].rot.x * 0.731f;
		v3Scale (&fragArray[i].rotV, &fragArray[i].rot, 0.2f);

/*		fragArray[i].col.colour = 0xff2030f0 + (((Uint32)(randAmt * 0xff))<<8);*/
		fragArray[i].col.colour = 0xffffffff;
		fragArray[i].type = FT_WATER;
	}
}

/*
 * Create explosion fragments (give centre, radius and amount)
 */
void rCreateExpFragments (const Point3 *centre, float radius, Uint32 amount)
{
	Uint32 i;

	i = fragLowWaterMark;
	while (amount--) {
		float randAmt;
		/* Find a free fragment */
		for (; i < MAX_FRAGMENTS; ++i) {
			if (fragArray[i].lifeTimer == 0)
				break;
		}
		/* Did we find a fragment? */
		if (i>=MAX_FRAGMENTS)
			return;

		if (i > fragHighWaterMark)
			fragHighWaterMark = i;
		
		if (i == fragLowWaterMark)
			fragLowWaterMark++;
		
		randAmt = (rand() & 127) * (1.f / 255.f);

		fragArray[i].lifeTimer = (Uint32)(30.f + 5.f * randAmt);
		v3Rand (&fragArray[i].pos);
		v3Scale (&fragArray[i].posV, &fragArray[i].pos, 0.12f);

		v3Scale (&fragArray[i].pos, &fragArray[i].pos, radius * randAmt);
		v3Inc (&fragArray[i].pos, centre);

		fragArray[i].rot.x = PI2 * randAmt;
		fragArray[i].rot.y = fragArray[i].rot.x * 0.373f;
		fragArray[i].rot.z = fragArray[i].rot.x * 0.731f;
		v3Scale (&fragArray[i].rotV, &fragArray[i].rot, 0.1f);

		fragArray[i].col.colour = 0xffffff00 | (int)(randAmt * 0xff);
		fragArray[i].type = FT_FALL;
	}
}

void rCreateSplash (const Point3 *centre, float radius, Uint32 amount)
{
	Uint32 i;
	int col;

	i = fragLowWaterMark;
	while (amount--) {
		float randAmt;
		/* Find a free fragment */
		for (; i < MAX_FRAGMENTS; ++i) {
			if (fragArray[i].lifeTimer == 0)
				break;
		}
		/* Did we find a fragment? */
		if (i>=MAX_FRAGMENTS)
			return;

		if (i > fragHighWaterMark)
			fragHighWaterMark = i;
		
		if (i == fragLowWaterMark)
			fragLowWaterMark++;
		
		randAmt = (rand() & 127) * (1.f / 255.f);

		fragArray[i].lifeTimer = (Uint32)(10.f + 15.f * randAmt);
		v3Rand (&fragArray[i].pos);
		if (fragArray[i].pos.z < 0)
			fragArray[i].pos.z = -fragArray[i].pos.z;
		v3Scale (&fragArray[i].posV, &fragArray[i].pos, 0.15f);

		v3Scale (&fragArray[i].pos, &fragArray[i].pos, radius * randAmt);
		v3Inc (&fragArray[i].pos, centre);

		fragArray[i].rot.x = PI2 * randAmt;
		fragArray[i].rot.y = fragArray[i].rot.x * 0.373f;
		fragArray[i].rot.z = fragArray[i].rot.x * 0.731f;
		v3Scale (&fragArray[i].rotV, &fragArray[i].rot, 0.1f);

		col = (int) (0xd0 + randAmt * 0x20);
		fragArray[i].col.colour = 0xff0000e0 | (col<<16) | (col << 8);
		fragArray[i].type = FT_FALL;
	}
}

void rCreateLavaBits (const Point3 *centre, Uint32 amount)
{
	Uint32 i;
	int col;

	i = fragLowWaterMark;
	while (amount--) {
		float randAmt;
		/* Find a free fragment */
		for (; i < MAX_FRAGMENTS; ++i) {
			if (fragArray[i].lifeTimer == 0)
				break;
		}
		/* Did we find a fragment? */
		if (i>=MAX_FRAGMENTS)
			return;

		if (i > fragHighWaterMark)
			fragHighWaterMark = i;
		
		if (i == fragLowWaterMark)
			fragLowWaterMark++;
		
		randAmt = (rand() & 127) * (1.f / 255.f);

		fragArray[i].lifeTimer = (Uint32)(25.f + 30.f * randAmt);
		v3Rand (&fragArray[i].pos);
		fragArray[i].pos.z = fabs(fragArray[i].pos.z) - 0.2f;
		fragArray[i].posV.x = ((rand() & 255) - 128) * (1.f / 255.f) * 0.1f;
		fragArray[i].posV.y = ((rand() & 255) - 128) * (1.f / 255.f) * 0.1f;
		fragArray[i].posV.z = 0.4f;

		v3Inc (&fragArray[i].pos, centre);

		fragArray[i].rot.x = PI2 * randAmt;
		fragArray[i].rot.y = fragArray[i].rot.x * 0.373f;
		fragArray[i].rot.z = fragArray[i].rot.x * 0.731f;
		v3Scale (&fragArray[i].rotV, &fragArray[i].rot, 0.1f);

		fragArray[i].col.colour = 0xffff0000;
		fragArray[i].type = FT_LAVA;
	}
}

/*
 * Draws a frag
 */
#define FR_S 0.1f
static const Point3 frag[4] = { 
	{ -FR_S,  0.0f, -FR_S },
	{  FR_S, -FR_S, -FR_S },
	{  FR_S,  FR_S, -FR_S },
	{  0.0f,  0.0f,  FR_S }
};
void rDrawFrag (Colour col, int timer)
{
	Matrix modelToScreen;
	KMVERTEX3		vert;
	KMSTATUS		result;
	Point p;
	float x0, y0, z0;
	float x1, y1, z1;
	float x2, y2, z2;
	float x3, y3, z3;

	/* Ensure modelToScreen matrix is loaded into XMTRX */
	mLoadModelToScreenScaled();

	/* Vertex 0 */
	*(Point3 *)&p = frag[0];
	p.w = 1.f;
	mPoint (&p, &p);
	if (p.z < 0 || p.z > p.w)
		return;

	z0 = 1.f / p.w;
	x0 = X_MID + p.x * z0;
	y0 = Y_MID + p.y * z0;

	/* Vertex 1 */
	*(Point3 *)&p = frag[1];
	p.w = 1.f;
	mPoint (&p, &p);
	if (p.z < 0 || p.z > p.w)
		return;

	z1 = 1.f / p.w;
	x1 = X_MID + p.x * z1;
	y1 = Y_MID + p.y * z1;

	/* Vertex 2 */
	*(Point3 *)&p = frag[2];
	p.w = 1.f;
	mPoint (&p, &p);
	if (p.z < 0 || p.z > p.w)
		return;

	z2 = 1.f / p.w;
	x2 = X_MID + p.x * z2;
	y2 = Y_MID + p.y * z2;

	/* Vertex 3 */
	*(Point3 *)&p = frag[3];
	p.w = 1.f;
	mPoint (&p, &p);
	if (p.z < 0 || p.z > p.w)
		return;

	z3 = 1.f / p.w;
	x3 = X_MID + p.x * z3;
	y3 = Y_MID + p.y * z3;

	if (timer < 20) {
		col.argb.a = (int)(timer * (255.f/20.f));
	} else {
		col.argb.a = 0xff;
	}

	/* Begin the triangle strip */
	kmxxGetCurrentPtr (&vertBuf);

	kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		x0, y0, z0, 0.0f, 0.5f, col.colour, 0); 
	kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		x1, y1, z1, 1.0f, 0.0f, col.colour, 0); 
	kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		x2, y2, z2, 1.0f, 1.0f, col.colour, 0); 
	kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		x3, y3, z3, 0.5f, 0.5f, col.colour, 0); 
	kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		x0, y0, z0, 0.0f, 0.5f, col.colour, 0); 
	kmxxSetVertex_3 (KM_VERTEXPARAM_ENDOFSTRIP,	x1, y1, z1, 1.0f, 0.0f, col.colour, 0); 

#if COUNT_GEOMETRY
	nDrawn += 4;
#endif

	kmxxReleaseCurrentPtr (&vertBuf);
}

void rDrawFragNonTex (Colour col, int timer)
{
	Matrix modelToScreen;
	KMVERTEX3		vert;
	KMSTATUS		result;
	Point p;
	float x0, y0, z0;
	float x1, y1, z1;
	float x2, y2, z2;
	float x3, y3, z3;

	/* Ensure modelToScreen matrix is loaded into XMTRX */
	mLoadModelToScreenScaled();

	/* Vertex 0 */
	*(Point3 *)&p = frag[0];
	p.w = 1.f;
	mPoint (&p, &p);
	if (p.z < 0 || p.z > p.w)
		return;

	z0 = 1.f / p.w;
	x0 = X_MID + p.x * z0;
	y0 = Y_MID + p.y * z0;

	/* Vertex 1 */
	*(Point3 *)&p = frag[1];
	p.w = 1.f;
	mPoint (&p, &p);
	if (p.z < 0 || p.z > p.w)
		return;

	z1 = 1.f / p.w;
	x1 = X_MID + p.x * z1;
	y1 = Y_MID + p.y * z1;

	/* Vertex 2 */
	*(Point3 *)&p = frag[2];
	p.w = 1.f;
	mPoint (&p, &p);
	if (p.z < 0 || p.z > p.w)
		return;

	z2 = 1.f / p.w;
	x2 = X_MID + p.x * z2;
	y2 = Y_MID + p.y * z2;

	/* Vertex 3 */
	*(Point3 *)&p = frag[3];
	p.w = 1.f;
	mPoint (&p, &p);
	if (p.z < 0 || p.z > p.w)
		return;

	z3 = 1.f / p.w;
	x3 = X_MID + p.x * z3;
	y3 = Y_MID + p.y * z3;

	if (timer < 80) {
		col.argb.a = (int)(timer * (255.f/80.f));
	} else {
		col.argb.a = 0xff;
	}

	/* Begin the triangle strip */
	kmxxGetCurrentPtr (&vertBuf);
	kmxxStartVertexStrip(&vertBuf);

	kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		x0, y0, z0, col.colour); 
	kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		x1, y1, z1, col.colour); 
	kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		x2, y2, z2, col.colour); 
	kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		x3, y3, z3, col.colour); 
	kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		x0, y0, z0, col.colour); 
	kmxxSetVertex_0 (KM_VERTEXPARAM_ENDOFSTRIP,	x1, y1, z1, col.colour); 

#if COUNT_GEOMETRY
	nDrawn += 4;
#endif

	kmxxReleaseCurrentPtr (&vertBuf);
}

/*
 * Updates the fragments
 */
void rUpdateFragments (void)
{
	Uint32 i;
	Fragment *frag;
	Material *curFragMat = NULL;
	void (*fragDraw) (Colour col, int timer);

	for (i = 0, frag = fragArray; i < fragHighWaterMark; ++i, ++frag) {
		register float x, y, z, rX, rY, rZ;
		register float *suck;
		if (frag->lifeTimer) {
			frag->lifeTimer--;
			if (frag->lifeTimer == 0) {
				/* Last frame of this badger */
				if (i < fragLowWaterMark)
					fragLowWaterMark = i;
				if (i == fragHighWaterMark && i != 0)
					fragHighWaterMark--;
				continue;
			}
			/*
			 * Update the rotation amount
			 */
			suck = &frag->rot.x;
			rX = *suck++;
			rY = *suck++;
			rZ = *suck++;
			suck = &frag->rotV.x;
			rX += *suck++;
			rY += *suck++;
			rZ += *suck++;
			/* Update the position onscreen */
			suck = &frag->pos.x;
			x = *suck++;
			y = *suck++;
			z = *suck++;
			suck = &frag->posV.x;
			x += *suck++;
			y += *suck++;
			z += *suck++;
			/*
			 * Update fragment behaviour
			 */
			switch (frag->type) {
			default:
				dAssert (FALSE, "Invalid fragment type");
				break;
			case FT_FALL:
			case FT_WATER:
				frag->posV.z -= 0.0218f;
				break;
			case FT_NOGRAV:
				break;
			case FT_LAVA:
				if (frag->col.argb.r)
					frag->col.argb.r -= 1;
				frag->posV.z -= 0.0218f;
				break;
			}
			/* 
			 * Store back the updated parameters
			 */
			suck = &frag->pos.x + 3;
			*--suck = z;
			*--suck = y;
			*--suck = x;
			suck = &frag->rot.x + 3;
			*--suck = rZ;
			*--suck = rY;
			*--suck = rX;
		}
	}
}

/*
 * Draws the fragments
 */
void rDrawFragments (void)
{
	Uint32 i;
	Fragment *frag;
	Material *curFragMat = NULL;
	void (*fragDraw) (Colour col, int timer);

	for (i = 0, frag = fragArray; i < fragHighWaterMark; ++i, ++frag) {
		register float x, y, z, rX, rY, rZ;
		register float *suck;
		if (frag->lifeTimer) {
			/*
			 * Read the rotation amount
			 */
			suck = &frag->rot.x;
			rX = *suck++;
			rY = *suck++;
			rZ = *suck++;
			/* Read the position onscreen */
			suck = &frag->pos.x;
			x = *suck++;
			y = *suck++;
			z = *suck++;

			mRotateXYZ (&mCurMatrix, rX, rY, rZ);
			mPostTranslate (&mCurMatrix, x, y, z);

			/*
			 * Set the material up
			 */
			if (&fragMats[frag->type] != curFragMat) {
				curFragMat = &fragMats[frag->type];

				rSetMaterial (curFragMat);

				kmxxGetCurrentPtr (&vertBuf);
				kmxxStartVertexStrip (&vertBuf);
				kmxxReleaseCurrentPtr (&vertBuf);

				fragDraw = (curFragMat->flags & MF_TEXTURE_FILTER_MASK) ? rDrawFrag : rDrawFragNonTex;
			}

			fragDraw (frag->col, frag->lifeTimer);
		}
	}
}

#define FIRE_SIZE 64
#define NUM_FIRES 24
#define FIRE_EXTRA_LINES 2
#define FIRE_ATTEN 1.5f
#pragma section Matrices
static Uint16 fireBufferLookedUpUnaligned[FIRE_SIZE*FIRE_SIZE / 2 + 32];
static Uint16 *fireBufferLookedUp;
#pragma section
static Uint8 fireBuffer[(FIRE_SIZE+FIRE_EXTRA_LINES)*FIRE_SIZE];
static Uint8 fireBufferSub[(FIRE_SIZE+FIRE_EXTRA_LINES)*FIRE_SIZE];
static int fireX[NUM_FIRES];
typedef struct {
	float x, y, vx, vy;
} Gel;
Gel GelArray[FIRE_SIZE/16][FIRE_SIZE/16];
Uint16 fireOffset[FIRE_SIZE*FIRE_SIZE];

static int fireTex;
//static Uint16 fireBufferPal[256];
KMPALETTEDATA paletteData;

#define FIRE_R 229
#define FIRE_G 189
#define FIRE_B 111
#define FIRE_TO_WHITE 160

// Set up a fire palette in a palette bank
void rSetFirePalette (int bank, int r1, int g1, int b1, 
					  int halfway,
					  int r2, int g2, int b2)
{
	int i;
	dAssert (bank >= 0 && bank < 4, "Invalid palette bank");
	bank *= 256;
	for (i = 0 ; i < halfway; ++i) {
		float fireAmount = i * (1.f / (float)halfway);
		int r, g, b;
		r = (int)(r1 * fireAmount)>>3;
		g = (int)(g1 * fireAmount * fireAmount)>>2;
		b = (int)(b1 * fireAmount * fireAmount)>>3;
		paletteData.wPaletteData[i + bank] = (r<<11) | (g<<5) | b;
	}
	for (;i < 0x100; ++i) {
		float amt = (i - halfway) * (1.f / (float)(256-halfway));
		int r, g, b;
		r = (int)(r1 + amt * (r2-r1))>>3;
		g = (int)(g1 + amt * (g2-g1))>>2;
		b = (int)(b1 + amt * (b2-b1))>>3;
		paletteData.wPaletteData[i + bank] = (r<<11) | (g<<5) | b;
	}

	kmSetPaletteData (&paletteData);
}

// Process the gel simulation
static float Norms[2][2] =
{
	{ 0, 1 },
	{ 1, 1.414213652f }
};
static void ProcessGel(void)
{
	register int x, y, X, Y;
	Gel *g;
	float n, xS, yS, length;
	for (y = 1; y < (FIRE_SIZE / 16)-1; ++y) {
		g = &GelArray[y][1];
		for (x = 1; x < (FIRE_SIZE / 16)-1; ++x) {
			float xPos, yPos;
			xPos = g->x;
			yPos = g->y;
			for (X = x-1; X < x+1; ++X) {
				for (Y = y-1; Y < y+1; ++Y) {
					if (x==X && y==Y)
						continue;
					n = Norms[abs(y-Y)][abs(x-X)];
					xS = GelArray[Y][X].x - xPos;
					yS = GelArray[Y][X].y - yPos;
					length = sqrt(SQR(xS) + SQR(yS));
					g->vx += (n - length) * 0.001f * xS;
					g->vy += (n - length) * 0.001f * yS;
				}
			}
			g++;
		}
	}
	for (y = 1; y < (FIRE_SIZE / 16)-1; ++y) {
		g = &GelArray[y][1];
		for (x = 1; x < (FIRE_SIZE / 16)-1; ++x) {
			g->x += g->vx;
			g->y += g->vy;
			g->vx *= 0.9998f;
			g->vy *= 0.9998f;
		}
	}
}

// Calculate the offset values
static void CalcOffset(void)
{
	register int x, y;
	register Uint16 *offset;

	offset = &fireOffset[0];
	for (y = 0; y < FIRE_SIZE/16-1; ++y) {
		for (x = 0; x < FIRE_SIZE/16-1; ++x) {
			*offset++ = x + y * FIRE_SIZE;
		}
	}
}

// Initialise the fire effect - also sets up the palette so must
// be called before any other palette initialisation
void rInitFire(void)
{
	int i, x, y;
	KMSTATUS ok;
	Uint16 *palette;
	static TexAnim fireAnim;

	// Set up the palette setings
	kmSetPaletteMode (KM_PALETTE_16BPP_RGB565);
	memset (&paletteData, 0, sizeof (paletteData));

	// Initialise the fire 
	memset (fireBuffer, 0, sizeof (fireBuffer));
	for (i = 0; i < NUM_FIRES; ++i)
		fireX[i] = rand() & (FIRE_SIZE-1);
	texPalette[nTextures].GBIX = 0x013ef13e; /* FIREFIRE */
	texPalette[nTextures].animation = NULL;
	fireTex = nTextures;
	ok = kmCreateTextureSurface (&texPalette[nTextures++].desc, FIRE_SIZE, FIRE_SIZE, KM_TEXTURE_PALETTIZE8);
	dAssert (ok == KMSTATUS_SUCCESS, "Unable to create fire surface!");
	// Initialise the fire with the default colours
	rSetFirePalette (0, FIRE_R, FIRE_G, FIRE_B, FIRE_TO_WHITE, 255, 255, 255);
	rSetFirePalette (1, 32, 255, 64, 200, 0, 255, 0);
	// Point the looked up version at an aligned address
	fireBufferLookedUp = (Uint16 *)(((int)&fireBufferLookedUpUnaligned[0] + 31) & ~31);
	// Now to set up the subtract buffer
	memset (fireBufferSub, FIRE_ATTEN, sizeof (fireBufferSub));
	for (i = 0; i < 3000; ++i) {
		int rand = RandRange (0, (FIRE_SIZE+FIRE_EXTRA_LINES)*FIRE_SIZE);
		fireBufferSub[rand] = (FIRE_ATTEN * 2 * frand());
	}
	for (i = 0; i < 12; ++i) {
		int x, y;
		for (y = 1; y < FIRE_SIZE+FIRE_EXTRA_LINES - 2; ++y) {
			for (x = 0; x < FIRE_SIZE; ++x) {
				float boograh;
				int x0 = x-1, x1 = x + 1;
				if (x0 < 0)
					x0 = FIRE_SIZE-1;
				if (x1 == FIRE_SIZE)
					x1 = 0;
#define FIRE(x,y) fireBufferSub[(x)+(y)*FIRE_SIZE]
				boograh = (float)(FIRE(x,y)<<1) + 
					(float)FIRE(x1, y) + (float)FIRE(x0, y) + (float)FIRE(x, y-1) + (float)FIRE(x, y+1) +
					(float)FIRE(x0,y-1) * 0.7f + 
					(float)FIRE(x0,y+1) * 0.7f + 
					(float)FIRE(x1,y-1) * 0.7f + 
					(float)FIRE(x1,y+1) * 0.7f;
				boograh = boograh / (2.f + 4.f + 4.f*0.7f);
				FIRE(x,y) = (Uint8)ceil(boograh);
			}
		} 
	}
	// Initialise the gel
	for (y = 0; y < FIRE_SIZE/16; ++y) {
		for (x = 0; y < FIRE_SIZE/16; ++y) {
			if (x==0 || x==((FIRE_SIZE/16)-1) ||
				y==0 || y==((FIRE_SIZE/16)-1)) {
				GelArray[y][x].x = x;
				GelArray[y][x].y = y;
			} else {
				GelArray[y][x].x = x + RandRange(-3, 3);
				GelArray[y][x].y = y + RandRange(-3, 3);
			}
			GelArray[y][x].vx = GelArray[y][x].vy = 0;
		}
	}
	ProcessGel();
	CalcOffset();
}

void rProcessFire(void)
{
	register Uint8 *p, *lbelow, *rbelow, *below, *pAtten;
	Uint16 *pOut;
	register int result;
	register int i, j, offsetX, offsetY;

	/*
	 * Firstly, light the fires
	 */
	p = &fireBuffer[(FIRE_SIZE+FIRE_EXTRA_LINES-1)*FIRE_SIZE];
	for (i = 0; i < NUM_FIRES; ++i) {
		result = p[fireX[i]];
		result += 0x40 + rand() & 0x4f;
		if (result > 0x100)
			result = 0x100;
		p[fireX[i]] = result;

		result = p[(fireX[i] - 1) & (FIRE_SIZE - 1)];
		result += 0x20 + rand() & 0x1f;
		if (result > 0x100)
			result = 0x100;
		p[(fireX[i] - 1) & (FIRE_SIZE - 1)] = result;

		result = p[(fireX[i] + 1) & (FIRE_SIZE - 1)];
		result += 0x20 + rand() & 0x1f;
		if (result > 0x100)
			result = 0x100;
		p[(fireX[i] + 1) & (FIRE_SIZE - 1)] = result;
		switch (rand() & 3) {
		case 0:
			fireX[i] = (fireX[i] + 1) & (FIRE_SIZE-1);
			break;
		case 1:
		case 2:
			fireX[i] = (fireX[i] - 1) & (FIRE_SIZE-1);
			break;
		}
	}

	/*
	 * Now process the cellular automata
	 */
	p = fireBuffer;	
	pAtten = fireBufferSub;
	for (i = 0; i < (FIRE_SIZE+FIRE_EXTRA_LINES-1); ++i) {
		lbelow = p + FIRE_SIZE;
		below = p + (FIRE_SIZE + 1);
		rbelow = p + (FIRE_SIZE + 2);
		/*
		 * First and last pixel special case (wrapped) :
		 * First pixel
		 */
		result = ((p[FIRE_SIZE+FIRE_SIZE-1] + (p[FIRE_SIZE]<<1) + p[FIRE_SIZE+1])>>2) - *pAtten;
		if (result < 0)
			result = 0;
		*p++ = result;
		pAtten++;
		/*
		 * Pixels 1 to FIRE_SIZE-2 :
		 */
		for (j = 1; j < FIRE_SIZE-1; ++j) {
			result = ((*lbelow + (*below << 1) + *rbelow)>>2) - *pAtten;
			lbelow++;
			if (result < 0)
				result = 0;
			below++;
			rbelow++;
			*p++ = result;
			pAtten++;
		}
		/* 
		 * Last pixel
		 */
		result = ((p[FIRE_SIZE+FIRE_SIZE+1] + (p[FIRE_SIZE]<<1) + p[FIRE_SIZE-1])>>2) - *pAtten;
		if (result < 0)
			result = 0;
		*p++ = result;
		pAtten++;
	}
	/*
	 * Final row
	 */
	for (i = 0; i < FIRE_SIZE; ++i) {
		result = *p - *pAtten;
		*p++ = result < 0 ? 0 : result;
		pAtten++;
	}
}

/*
 * Fire callback
 */
void rFireCallback(void)
{
	register Uint8 *p, *below;
	register int x, y, i, j, plus, and;
	register Uint16 *pOut;

	/*
	 * Copy it into the twiddled buffer
	 */

	plus = 0xaaaaaaab;
	and =  0x55555555;
	p = fireBuffer;
	below = fireBuffer + FIRE_SIZE;
	pOut = (Uint16 *) (((int)&fireBufferLookedUp[0] & 0x1fffffff)| 0xa0000000);
	y = 0;
	for (i = FIRE_SIZE / 2; i != 0; --i) {
		x = 0;
		for (j = FIRE_SIZE / 2; j != 0; --j) {
			pOut[x | y] = *p++ | (*below++ << 8);
			x = (x + plus) & and;
			pOut[x | y] = *p++ | (*below++ << 8);
			x = (x + plus) & and;
		}
		y = (y + 0x55555556) & 0xaaaaaaaa;
		p += FIRE_SIZE;
		below += FIRE_SIZE;
	}

	kmLoadTexture (&texPalette[fireTex].desc, (KMDWORD *)fireBufferLookedUp, FALSE, FALSE);
}

/*
 * Weather effects
 */
#define MAX_WEATHER_PARTICLES	512
static int nWeatherParticles = 0;

typedef struct tagWPart
{
	float		x, y, z;
	float		vx, vy, vz;
	int			lifeTimer;
	CollBBox	box;
} WPart;

WPart wPart[MAX_WEATHER_PARTICLES];

static Material rainMaterial = { 0, 0, 0.f, 0.f, MF_TWO_SIDED | MF_ALPHA};
Material snowMaterial = { 0, 0, 0.f, 0.f, MF_TWO_SIDED | MF_ALPHA | MF_TEXTURE_BILINEAR};

int rainOn = 0, lensFlareOn = 1;
int maxRainInside = 0, maxRainOutside = 0;
static float lensFlareAmt = 0.f;
Matrix rainSpace;
bool SnowOn = FALSE;

void SetRainAmount (STRAT *strat, int amountIn, int amountOut, bool snow)
{
	if (amountIn > 0 || amountOut > 0) {
		Matrix m;
		rainOn = 1;
		maxRainInside = MIN(amountIn, MAX_WEATHER_PARTICLES);
		maxRainOutside = MIN(amountOut, MAX_WEATHER_PARTICLES);
		m = strat->m;
		mPreScale(&m, strat->scale.x, strat->scale.y, strat->scale.z);
		mPostTranslate(&m, strat->pos.x, strat->pos.y, strat->pos.z);
		mInvert3d (&rainSpace, &m);
	} else {
		rainOn = 0;
	}
	SnowOn = snow;
}

void IAmTheSun (STRAT *strat)
{
	v3Sub ((Point3 *)&StratSunDir, &strat->pos, &player[currentPlayer].Player->pos);
	v3Normalise ((Point3 *)&StratSunDir);
}

float windX = 0.2f, windY = 0.2f;
void rResetWeather (void)
{
	register int i;
	register WPart *s;

	for (i = MAX_WEATHER_PARTICLES, s = wPart; i != 0; --i, ++s)
		s->lifeTimer = 0;

	windX = windY = 0.2f;

	rainMaterial.surf.GBIX = 0;
	rFixUpMaterial (&rainMaterial, &sfxContext);
	rainOn = 0;
	lensFlareOn = 1;
	lensFlareAmt = 0;
}

static Bool IsInside(Point3 *p)
{
	Point3 p2;
	mPoint3Multiply3 (&p2, p, &rainSpace);
	if (p2.x > 0.f && p2.x < 1.f &&
		p2.y > 0.f && p2.y < 1.f &&
		p2.z > 0.f && p2.z < 1.f)
		return TRUE;
	else
		return FALSE;
}

static void rRainUpdate (void)
{
	register int i;
	register WPart *rain;
	register float *update, *read, x, y, z;
	register float xx, yy, zz;
	float CreationCircle;
	Uint32 a, b, c;
	int maxRain;

	CreationCircle = currentCamera->farZ * (1.f/3.f);
	if (SnowOn)
		CreationCircle *= 0.5f;

	rain = wPart;
	maxRain = MAX(maxRainInside, maxRainOutside);
	for (i = maxRain; i != 0; --i, ++rain) {
		prefetch ((char *)rain + 32);
		if (rain->lifeTimer == 0) {
			float t;
			Bool in;
			if (SnowOn) {
				rain->vx = windX*0.2f + RandRange(-.05f, 0.05f);
				rain->vy = windY*0.2f + RandRange(-.05f, 0.05f);
				rain->vz = (RandRange(-0.3f, -0.5f));
			} else {
				rain->vx = windX + RandRange(-.01f, 0.01f);
				rain->vy = windY + RandRange(-.01f, 0.01f);
				rain->vz = (RandRange(-2.1f, -2.0f));
			}
			t = RandRange(0.f, 1.f);
			rain->x = SQR(t) * CreationCircle;
			t = RandRange(0.f, 1.f);
			rain->y = SQR(t) * CreationCircle;
			switch (rand() & 3) {
			case 1:
				rain->x = -rain->x;
				break;
			case 3:
				rain->x = -rain->x;
				// Falls to:
			case 2:
				rain->y = -rain->y;
				break;
			}
			rain->x +=currentCamera->pos.x; 
			rain->y +=currentCamera->pos.y; 
			rain->z = currentCamera->pos.z + RandRange(10.f, 70.f);
			in = IsInside ((Point3 *)&rain->x);
			if (i > maxRainInside) {
				// Rain only those outside
				if (in)
					continue;
			} else if (i > maxRainOutside) {
				// Rain only those inside
				if (!in)
					continue;
			}
			findCollBBox ((Point3 *)&rain->x, &rain->box);
			rain->lifeTimer = 1;
			rain->box.leaf = NULL;
		}
		// Perform the positional update
		update= &rain->x; read = &rain->vx;
		xx = *update++; x = xx + *read++;
		yy = *update++; y = yy + *read++;
		zz = *update++ ; z = zz + *read;

		// Every 16 rain drops has collision on it
		if ((i & 15)==0) {
			Point3 top, bottom;
			// Are we still in the same collbox?
			if (!(x > rain->box.l &&
				x < rain->box.r &&
				y > rain->box.b &&
				y < rain->box.t)) {
				findCollBBox ((Point3 *)&rain->x, &rain->box);
			}
			top.x = xx;
			top.y = yy;
			top.z = zz;
			bottom.x = x;
			bottom.y = y;
			bottom.z = z;
			if (lineBBocColl (rain->box.leaf, &top, &bottom, 0.f)) {
				rain->lifeTimer = 0;
				rCreateConeFrags (&GlobalCollP, &GlobalCollPolyN, 0.7f, 0.1f, 2);
				continue;
			}
		}

		// Write back the details
		*--update = z;
		*--update = y;
		*--update = x;
	}
}
static void rRainDraw(void)
{
	register int i;
	register WPart *rain;
	register float *update, *read, x, y, z, w;
	register float xx, yy, zz, ww;
	float sX, sY, mX, mY, delX, delY, scale;
	Point temp;
	int nActive = 0;
	Uint32 a, b, c;
	int maxRain;

	/*
	 * Draw the rain streaks
	 */
	if (SnowOn)
		rSetMaterial (&snowMaterial);
	else
		rSetMaterial (&rainMaterial);
	kmxxGetCurrentPtr (&vertBuf);
	
	mLoad (&mWorldToScreen);
	mX = pViewMidPoint.x;
	mY = pViewMidPoint.y;
	sX = pViewSize.x;
	sY = pViewSize.y;

	rain = wPart;
	maxRain = MAX(maxRainInside, maxRainOutside);
	for (i = maxRain; i != 0; --i, ++rain) {
		prefetch ((char *)rain + 32);
		if (rain->lifeTimer == 0)
			continue;
		// Get the position
		update= &rain->x; read = &rain->vx;
		xx = *update++; x = xx + *read++;
		yy = *update++; y = yy + *read++;
		zz = *update++; z = zz + *read;

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
			// The rain is offscreen - don't draw it
			// If it's off the bottom of the screen, or off by a lot, kill it
			if (y > w || z < -10.f)
				rain->lifeTimer = 0;
/*			w *= 1.2f;
			if (x > w ||
				x < -w)
				rain->lifeTimer = 0;*/
		} else {
			// The rain is visible - so draw it!
			w = 1.f / w;	// Get scale factor
			if (nActive++ == 0)
				kmxxStartVertexStrip (&vertBuf);	// If it's the first rain bit, then start the strip
			if (SnowOn) {
				// Snow effect
				x  = mX + sX * w * x;
				y  = mY + sY * w * y;
				delX = delY = w * 64.f;
				kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		x  - delX  , y  + delY  , w, 0, 1, 0x80ffffff, 0);
				kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		x  + delY  , y  + delX  , w, 1, 1, 0x80ffffff, 0);
				kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		x  - delY  , y  - delX  , w, 0, 0, 0x80ffffff, 0);
				kmxxSetVertex_3 (KM_VERTEXPARAM_ENDOFSTRIP,	x  + delX  , y  - delY  , w, 1, 0, 0x80ffffff, 0);
			} else {
				// Rain effect: Transform the end point
				mPointXYZ (&temp, xx, yy, zz);
				x  = mX + sX * w * x;
				ww = 1.f / temp.w;
				y  = mY + sY * w * y;
				xx = mX + sX * ww * temp.x;
				yy = mY + sY * ww * temp.y;

				delX = xx - x;
				delY = y - yy;
				scale = isqrt (SQR(delX) + SQR(delY));

				ww = (250.f - temp.z) * (1.f / 250.f);
				if (ww < 0)
					w = 0;
				a = ((Uint8)((float)0x30 * ww))<<24;
				b = ((Uint8)((float)0x80 * ww))<<24;
				c = ((Uint8)((float)0xa0 * ww))<<24;

				delX *= scale;
				delY *= scale;

				kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		x  - delX*4, y  + delY*4, w, a | 0xa8acb0);
				kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		x  + delY  , y  + delX  , w, b | 0xb0b9c2);
				kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		x  - delY  , y  - delX  , w, b | 0xb0b9c2);
				kmxxSetVertex_0 (KM_VERTEXPARAM_ENDOFSTRIP,	xx         , yy         , w, c | 0xdde0e3);
			}
		}

	}
#if COUNT_GEOMETRY
	if (SnowOn) {
		nDrawn += 6 * nActive;
	} else {
		nDrawn += 2 * nActive;
	}
#endif

	kmxxReleaseCurrentPtr (&vertBuf);
}

// Lens flare effect
#define FLARE_MIN_ANGLE 0.86f			// 30 degrees
#define FLARE_WHITE_OUT_ANGLE 0.966f	// 15 degrees
#define FLARE_POWER		35
#define FLARE_OUT_MAX	(40.f/256.f)
Material lensFlareMat[8] = {
	{ 0xffffffff, 0, 0.f, 0.f, MF_TWO_SIDED | MF_ALPHA | MF_TRANSPARENCY_ADDITIVE | MF_TEXTURE_BILINEAR},
	{ 0xffffffff, 0, 0.f, 0.f, MF_TWO_SIDED | MF_ALPHA | MF_TRANSPARENCY_ADDITIVE | MF_TEXTURE_BILINEAR},
	{ 0xffffffff, 0, 0.f, 0.f, MF_TWO_SIDED | MF_ALPHA | MF_TRANSPARENCY_ADDITIVE | MF_TEXTURE_BILINEAR},
	{ 0xffffffff, 0, 0.f, 0.f, MF_TWO_SIDED | MF_ALPHA | MF_TRANSPARENCY_ADDITIVE | MF_TEXTURE_BILINEAR},
	{ 0xffffffff, 0, 0.f, 0.f, MF_TWO_SIDED | MF_ALPHA | MF_TRANSPARENCY_ADDITIVE | MF_TEXTURE_BILINEAR},
	{ 0xffffffff, 0, 0.f, 0.f, MF_TWO_SIDED | MF_ALPHA | MF_TRANSPARENCY_ADDITIVE | MF_TEXTURE_BILINEAR},
	{ 0xffffffff, 0, 0.f, 0.f, MF_TWO_SIDED | MF_ALPHA | MF_TRANSPARENCY_ADDITIVE | MF_TEXTURE_BILINEAR},
	{ 0xffffffff, 0, 0.f, 0.f, MF_TWO_SIDED | MF_ALPHA | MF_TRANSPARENCY_ADDITIVE | MF_TEXTURE_BILINEAR},
};
Material sunMat = 
	{ 0xffffffff, 0, 0.f, 0.f, MF_TWO_SIDED | MF_ALPHA | MF_TEXTURE_BILINEAR};
Material expMat = 
	{ 0xffffffff, 0, 0.f, 0.f, MF_TWO_SIDED | MF_ALPHA | MF_TEXTURE_BILINEAR | MF_TRANSPARENCY_ADDITIVE };

static const struct {
	int		mtl;
	float	dist;
	float	size;
} lensLookup[] = 
{
	{ 1, -1.30f, 0.80f },
	{ 0, -1.00f, 1.50f },
	{ 1, -0.50f, 1.00f },
	{ 1, -0.45f, 0.50f },
	{ 2, -0.20f, 0.20f },
	{ 3,  0.00f, 0.10f },
	{ 4,  0.35f, 0.20f },
	{ 5,  0.55f, 0.65f },
	{ 5,  0.60f, 1.00f },
	{ 6,  0.90f, 0.80f },
	{ 7,  1.20f, 2.00f },
	{ 7,  1.50f, 1.30f }
};

static void rDrawLensFlare (void)
{
	// Calculate the camera direction . sun direction
	register int i;
	float dotProduct, wS, dist, power;
	Vector3 dir;
	Point  sunPos, sunPosWorld;
	Point3 collPt;

	dir = *(Point3 *)&StratSunDir;
	v3Normalise (&dir);

	dotProduct =	dir.x * mWorldToView.m[0][2] +
					dir.y * mWorldToView.m[1][2] +
					dir.z * mWorldToView.m[2][2];

	dist = 2000.f;
	sunPosWorld.x = currentCamera->pos.x + dist * dir.x;
	sunPosWorld.y = currentCamera->pos.y + dist * dir.y;
	sunPosWorld.z = currentCamera->pos.z + dist * dir.z;
	sunPosWorld.w = 1.f;

	power = pow (dotProduct, FLARE_POWER);

	mUnit (&mCurMatrix);
	rSpriteMtl ((Point3 *)&sunPosWorld, 90.f, 90.f, 0, &sunMat);

	// Are we out of range?
	if (dotProduct < FLARE_MIN_ANGLE)
		return;

	mLoad (&mWorldToScreen);
	mPoint (&sunPos, &sunPosWorld);

	// Offscreen?
	wS = sunPos.w * 1.3f;
	if (sunPos.x < -wS || sunPos.x > wS ||
		sunPos.y < -wS || sunPos.y > wS)
		return;

	if (obscured (&currentCamera->pos, (Point3 *)&sunPos, &collPt, -1, NULL)) {
		lensFlareAmt = MAX(0, lensFlareAmt - 0.1f);
	} else {
		lensFlareAmt = MIN (1, lensFlareAmt + 0.15f);
	}
	
	if (lensFlareAmt == 0)
		return;

	sunPos.w = 1.f / sunPos.w;
	sunPos.x *= sunPos.w * pViewSize.x;
	sunPos.y *= sunPos.w * pViewSize.y;

	for (i = 0; i < asize(lensLookup); ++i) {
		float amt, x, y;
		amt = lensLookup[i].dist;
		x = sunPos.x * amt;
		y = sunPos.y * amt;
		lensFlareMat[lensLookup[i].mtl].diffuse.argb.a = (0x10 + (0x80-0x10) * power) * lensFlareAmt;
		rSprite2DMtl (pViewMidPoint.x - x, pViewMidPoint.y - y, &lensFlareMat[lensLookup[i].mtl], lensLookup[i].size, power * 2 * PI + i * 0.3f);
	}

	// Are we pointing almost exactly at the light?
	if (dotProduct > FLARE_WHITE_OUT_ANGLE) {
		float flareScreenAmount;
		flareScreenAmount = lensFlareAmt * FLARE_OUT_MAX * Delta ((dotProduct - FLARE_WHITE_OUT_ANGLE) * (1.f / (1.f - FLARE_WHITE_OUT_ANGLE)));
		rFlareR += flareScreenAmount;
		rFlareG += flareScreenAmount;
		rFlareB += flareScreenAmount;
	}
}

// Strat glue function
void DrawLensFlare (STRAT *strat, int objId, float x, float y, float z, float r, float g, float b, float intensity)
{
	Point pos, posWorld;
	Vector3 dirWorld;
	Outcode code;
	float rW, dotProduct, power;
	register int i;

	// If objID is 0, treat x,y,z as a world position
	if (objId) {
		mLoad (&strat->objId[objId]->wm);

		pos.x = x; pos.y = y; pos.z = z; pos.w = 1.f;

		mPoint (&posWorld, &pos);
	} else {
		posWorld.x = x; posWorld.y = y; posWorld.z = z; posWorld.w = 1.f;
	}

	// Take the world position into screenspace
	mLoad (&mWorldToScreen);
	code = mPointXYZOut (&pos, posWorld.x, posWorld.y, posWorld.z);

	rW = 1.f / pos.w;

	// Is it offscreen?
	if (code & OC_OFF)
		return;

	// Perspective divide to find the actual position relative to the centre of the screen
	x = pos.x * rW * pViewSize.x;
	y = pos.y * rW * pViewSize.y;

	v3Sub (&dirWorld, (Vector3 *) &posWorld, &currentCamera->pos);
	v3Normalise (&dirWorld);

	dotProduct =	dirWorld.x * mWorldToView.m[0][2] +
					dirWorld.y * mWorldToView.m[1][2] +
					dirWorld.z * mWorldToView.m[2][2];

	intensity *= dotProduct;

	power = pow (intensity, FLARE_POWER);

	for (i = 0; i < asize(lensLookup); ++i) {
		float amt, pX, pY;
		amt = lensLookup[i].dist;
		pX = x * amt;
		pY = y * amt;
		lensFlareMat[lensLookup[i].mtl].diffuse.argb.a = (0x10 + (0x80-0x10) * power);
		lensFlareMat[lensLookup[i].mtl].diffuse.argb.r = 255.f * r;
		lensFlareMat[lensLookup[i].mtl].diffuse.argb.g = 255.f * g;
		lensFlareMat[lensLookup[i].mtl].diffuse.argb.b = 255.f * b;
		rSprite2DMtl (pViewMidPoint.x - pX, pViewMidPoint.y - pY, &lensFlareMat[lensLookup[i].mtl], lensLookup[i].size, power * 2 * PI + i * 0.3f);
	}

	// Are we pointing almost exactly at the light?
	if (dotProduct > FLARE_WHITE_OUT_ANGLE) {
		float flareScreenAmount;
		flareScreenAmount = FLARE_OUT_MAX * Delta ((dotProduct - FLARE_WHITE_OUT_ANGLE) * (1.f / (1.f - FLARE_WHITE_OUT_ANGLE)));
		rFlareR += flareScreenAmount * r;
		rFlareG += flareScreenAmount * g;
		rFlareB += flareScreenAmount * b;
	}

}

void DrawFlare (STRAT *strat, float amt)
{
	Point pos;
	Outcode code;
	float rW, x, y;
	int i;

	// Take the world position into screenspace
	mLoad (&mWorldToScreen);
	code = mPointXYZOut (&pos, strat->pos.x, strat->pos.y, strat->pos.z);

	rW = 1.f / pos.w;

	// Is it offscreen?
	if (code & OC_OFF)
		return;

	// Perspective divide to find the actual position
	x = pos.x * rW * pViewSize.x + pViewMidPoint.x;
	y = pos.y * rW * pViewSize.y + pViewMidPoint.y;

	i = 1;
	lensFlareMat[lensLookup[i].mtl].diffuse.argb.a = 255.f * amt;
	lensFlareMat[lensLookup[i].mtl].diffuse.argb.r = 255.f;
	lensFlareMat[lensLookup[i].mtl].diffuse.argb.g = 255.f;
	lensFlareMat[lensLookup[i].mtl].diffuse.argb.b = 255.f;
	rSprite2DMtl (x, y, &lensFlareMat[lensLookup[i].mtl], lensLookup[i].size, 0);

}

void rUpdateWeatherEffects (void)
{
	// Update the wind position
	windX += frand() * 0.002f - 0.001f;
	windY += frand() * 0.002f - 0.001f;
	windX = RANGE(-1, windX, 1);
	windY = RANGE(-1, windY, 1);

	if (rainOn)
		rRainUpdate();

}

void rDrawWeatherEffects (void)
{
	if (lensFlareOn)
		rDrawLensFlare();
	if (rainOn)
		rRainDraw();
}

/*
 * Decal system
 */

#define MAX_DECALS 64 // power of two

typedef struct tagDecal
{
	Sint8		material;
	Uint8		r;
	Uint8		g;
	Uint8		b;
	Point3		pos[4];
} Decal;

// The decal ring buffer
Decal decal[MAX_DECALS];
static int nDecals = 0;

// The list of materials for bullet holes
Material	bulletHoleMat[]	= 
{
	{ 0, 0, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA },			// 0 is ground hit
	{ 0, 0, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA },			// 1 is gun hit
	{ 0, 0, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA },			// 2 is gun hit number 2
	{ 0, 0, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA },			// 3 is gun hit number 4
};

static int SurfTypeLookup[][2] =
{
	{ 0, 2 },			// Rock
	{ 0, 2 },			// Grass
	{ 1, 3 },			// Metal
	{ 0, 2 },			// Mud
	{ 0, 2 },			// Sand
	{ 0, 2 },			// Loop the loop
	{ 0, 2 },			// Ice
	{ 0, 2 },			// Water
	{ 0, 2 },			// SNOW
	{ 0, 2 },			// RealGravity
	{ 0, 2 },			// WaterSlide Water
	{ 0, 2 },			// Spider Man
	{ 0, 2 },			// KOTH (King of the Hill)
	{ 0, 2 },			// DANGER
};

static Uint8 DecalFadeArray[256];

void rInitDecals (void)
{
	register int i;
	nDecals = 0;
	for (i = 0; i < MAX_DECALS; ++i)
		decal[i].material = -1;
	for (i = 0; i < 256; ++i) {
		DecalFadeArray[i] = (Uint8) ((float)i * 0.96f);
		if (DecalFadeArray[i] == i)
			DecalFadeArray[i] = i-1;
	}
	DecalFadeArray[0] = 0;
}

// Run through the decal ring buffer
void rDrawDecals (void)
{
	register int i, num;
	register float sX, sY, mX, mY, trans, r, g, b;
	Point p[4];
	register Outcode outcode;
	register Decal *d;
	register Sint16 m;

	mLoad (&mWorldToScreen);

	sX = pViewSize.x; sY = pViewSize.y;
	mX = pViewMidPoint.x; mY = pViewMidPoint.y;

	d = decal;
	// Start with material 0
	rSetMaterial (&bulletHoleMat[0]);
	m = -1;
	kmxxGetCurrentPtr (&vertBuf);

	for (i = 0; i < MAX_DECALS; ++i, ++d) {
		if (d->material == -1)
			continue;
		// Transform the four points
		outcode = mPointXYZOut (&p[0], d->pos[0].x, d->pos[0].y, d->pos[0].z);
		p[0].w = 1.f / p[0].w;
		outcode|= mPointXYZOut (&p[1], d->pos[1].x, d->pos[1].y, d->pos[1].z);
		p[1].w = 1.f / p[1].w;
		outcode|= mPointXYZOut (&p[2], d->pos[2].x, d->pos[2].y, d->pos[2].z);
		p[2].w = 1.f / p[2].w;
		outcode|= mPointXYZOut (&p[3], d->pos[3].x, d->pos[3].y, d->pos[3].z);
		p[3].w = 1.f / p[3].w;

		// Check for being completely offscreen
		if ((outcode & OC_NOT) != OC_NOT || (outcode & OC_OFF_NEAR))
			continue;

		// Set the material
		num = (i - nDecals) & (MAX_DECALS - 1);
		trans = num < 16 ? num * (1.f / 15.f) : 1.f;
		if (d->material != m) {
			rSetMaterial (&bulletHoleMat[d->material]);
			kmxxStartVertexStrip (&vertBuf);
			m = d->material;
		}
		r = d->r * (1.f/255.f);
		g = d->g * (1.f/255.f);
		b = d->b * (1.f/255.f);

		// Draw it!
#define DO_VERT(A,I,U,V) \
		kmxxSetVertex_5 (A, \
			mX + p[I].x * p[I].w * sX, \
			mY + p[I].y * p[I].w * sY, \
			p[I].w, \
			U, V,  \
			trans, 1, 1, 1, 0, r, g, b)
		DO_VERT(KM_VERTEXPARAM_NORMAL,		0,0,0);
		DO_VERT(KM_VERTEXPARAM_NORMAL,		1,0,1);
		DO_VERT(KM_VERTEXPARAM_NORMAL,		3,1,0);
		DO_VERT(KM_VERTEXPARAM_ENDOFSTRIP,	2,1,1);
		
	}
	kmxxReleaseCurrentPtr (&vertBuf);
	// Update if not paused
	if (!PauseMode) {
		d = decal;
		for (i = 0; i < MAX_DECALS; ++i, ++d) {
			if (d->material == -1)
				continue;
			d->r = DecalFadeArray[d->r];
			d->g = DecalFadeArray[d->g];
			d->b = DecalFadeArray[d->b];
		}
	}
}

// Create a decal, given a material, centre point, face normal and x and y size
#define DECAL_RAISE_AMOUNT 0.001f
void rMakeDecal (Point3 *cPoint, Vector3 *faceNormal, Vector3 *vVector, float xSize, float ySize, Sint16 mat, float r, float g, float b)
{
	Vector3 x, y;
	Decal *d = decal + nDecals;
	Matrix m;
	float height;

	if (v3SqrLength(faceNormal) < 0.001f)
		return;

	nDecals = (nDecals + 1) & (MAX_DECALS-1);

	height = nDecals * 0.00005f + DECAL_RAISE_AMOUNT;

	d->material = mat;
	d->r = r*255;
	d->g = g*255;
	d->b = b*255;

	// Calculate the four points
	// We need a matrix which brings us into decal-space, where Z is along the face normal,
	// X and Y are in decal-uv space
	mTranslate (&m, cPoint->x, cPoint->y, cPoint->z);
	v3Cross (&x, faceNormal, vVector);
	v3Cross (&y, faceNormal, &x);

	v3Normalise (&x);
	v3Normalise (&y);

	m.m[0][0] = x.x; m.m[0][1] = x.y; m.m[0][2] = x.z;
	m.m[1][0] = y.x; m.m[1][1] = y.y; m.m[1][2] = y.z;
	m.m[2][0] = faceNormal->x; m.m[2][1] = faceNormal->y; m.m[2][2] = faceNormal->z;

	mLoad (&m);

	// Now to calculate the four points
	d->pos[0].x = -xSize; d->pos[0].y = +ySize; d->pos[0].z = height;
	mPoint3 (&d->pos[0], &d->pos[0]);
	d->pos[1].x = -xSize; d->pos[1].y = -ySize; d->pos[1].z = height;
	mPoint3 (&d->pos[1], &d->pos[1]);
	d->pos[2].x = +xSize; d->pos[2].y = -ySize; d->pos[2].z = height;
	mPoint3 (&d->pos[2], &d->pos[2]);
	d->pos[3].x = +xSize; d->pos[3].y = +ySize; d->pos[3].z = height;
	mPoint3 (&d->pos[3], &d->pos[3]);
}

// Strat glue code
void AddDecal (STRAT *strat, int subId, int dNum, float xSize, float ySize, float r, float g, float b)
{
	Vector3 v;
	extern Material flareMat;
	Object *subObj = subId ? strat->objId[subId] : strat->obj;

	v = strat->vel;
	v3Normalise (&v);

	dAssert (dNum >= 0 && dNum < 2, "Out of range decal number");
	if (dNum < 0 || dNum > 1)
		dNum = 0;

	

	if (OBJECT_GET_COLL_ID(subObj->collFlag)) {
		Collision *coll = collArray + OBJECT_GET_COLL_ID(subObj->collFlag);
		if (GET_SURF_TYPE(coll->type) == NODECAL)
			return;
		dNum = SurfTypeLookup[GET_SURF_TYPE(coll->type)][dNum];
		if (dNum > asize(bulletHoleMat))
			dNum = 0;
		subObj->collFlag = OBJECT_CLEAR_COLL_ID(subObj->collFlag);
		rMakeDecal (&coll->pos, &coll->normal, &v, xSize * RandRange(0.4, 0.6), ySize * RandRange(0.4, 0.6), dNum, r, g, b);
	}
}

/*
 * Skidding warez
 */
Material skidMat = { 0xffffffff, 0, 0.f, 0.f, MF_ALPHA | MF_TEXTURE_BILINEAR | MF_TWO_SIDED | MF_MIPMAP_SET(1) | MF_TRANSPARENCY_SUBTRACTIVE };

void rStartSkid (int pn)
{
    register int i;
	extern Uint32 lLightBufferZerod[MAX_LIT_VERTICES];
    mLoadWithXYScale (&mWorldToScreen, pViewSize.x, pViewSize.y);

	// Set up the madness
	theStripRasterContext.midX = pViewMidPoint.x;
	theStripRasterContext.midY = pViewMidPoint.y;
	theStripRasterContext.mat = -1;
	theStripRasterContext.scaleX = pViewSize.x;
	theStripRasterContext.scaleY = pViewSize.y;
	theStripRasterContext.rWnearVal = rWnearVal;
	theStripRasterContext.lightingBuffer = lLightBufferZerod;
}

#define MAX_STRIP_AMT	512
//static StripPos		stripPosBuffer[MAX_STRIP_AMT];
//static StripEntry	stripEntBuffer[MAX_STRIP_AMT];
void rDrawSkid (int numPoints, StripEntry *stripEntBuffer, StripPos *stripPosBuffer)
{
	register int i;
	register float f, *fSuck;
	register float lX, lY, lZ, hX, hY, hZ;
	Point3 lowPoint, hiPoint;
	Uint32 read;
	StripEntry *s;
	CLIPTYPE onScreen;

	dAssert (numPoints < MAX_STRIP_AMT, "Too many vertices to convert strip amt");

	// Calculate the axis aligned bounding box of the points
	s = stripEntBuffer;
	lX = hX = stripPosBuffer[(s->vertRef)>>2].p.x;
	lY = hY = stripPosBuffer[(s->vertRef)>>2].p.y;
	lZ = hZ = stripPosBuffer[(s->vertRef)>>2].p.z;
	s++;

	for (i = numPoints-1; i != 0; --i) 
	{
		prefetch ((char *)s + 64);
		read = s->vertRef<<2;
		s++;
		dAssert (read < (MAX_STRIP_AMT<<4), "Too many vertices to convert strip amt");
		fSuck = (float *)((char *)stripPosBuffer + read);

		f = *fSuck++;
		if (f > hX)
			hX = f;
		if (f < lX)
			lX = f;

		f = *fSuck++;
		if (f > hY)
			hY = f;
		if (f < lY)
			lY = f;

		f = *fSuck++;
		if (f > hZ)
			hZ = f;
		if (f < lZ)
			lZ = f;
	}
	fSuck = (&lowPoint.z) + 1;
	*--fSuck = lZ;
	*--fSuck = lY;
	*--fSuck = lX;
	fSuck = (&hiPoint.z) + 1;
	*--fSuck = hZ;
	*--fSuck = hY;
	*--fSuck = hX;

	onScreen = rOnScreen (&mWorldToScreen, &lowPoint, &hiPoint);
	if (onScreen == CLIP_OFFSCREEN)
		return;

	if (onScreen == CLIP_NEARCLIPPED) {
		extern void rSetMaterialUV (Material *uv);
		mLoad (&mWorldToScreen);
	    rSetMaterialUV (&skidMat);
		kmxxGetCurrentPtr (&vertBuf);
		kmxxStartVertexStrip (&vertBuf);
		theStripRasterContext.pkmCurrentPtr = pkmCurrentPtr;
		(void)texturedStripRasteriserClipped (stripPosBuffer, stripEntBuffer, numPoints, &theStripRasterContext);
		pkmCurrentPtr = theStripRasterContext.pkmCurrentPtr;
	    kmxxReleaseCurrentPtr (&vertBuf);
	} else {
	    rSetMaterial (&skidMat);
		mLoadWithXYScaleMirrored (&mWorldToScreen, pViewSize.x, pViewSize.y);
		kmxxGetCurrentPtr (&vertBuf);
		kmxxStartVertexStrip (&vertBuf);
		theStripRasterContext.pkmCurrentPtr = pkmCurrentPtr;
		(void)texturedStripRasteriser		 (stripPosBuffer, stripEntBuffer, numPoints, &theStripRasterContext);
		pkmCurrentPtr = theStripRasterContext.pkmCurrentPtr;
	    kmxxReleaseCurrentPtr (&vertBuf);
	}
}

static Bool IsInvisible (Object *obj)
{
	if (obj->collFlag & OBJECT_INVISIBLE)
		return TRUE;
	else if (obj->parent == NULL)
		return FALSE;
	else
		return IsInvisible (obj->parent);
}
/*
 * Support functions :
 * Fill in a Point3 structure with a world-space reddog vertex
 * If normal is non-null, a world-space normal will be returned too
 * Returns objId<<16 | vert
 */
static int FindRedDogVertex (STRAT *strat,Point3 *pos, Vector3 *normal)
{
	int nObj, nVerts;
	Object *obj;
	int vert, oNum=0;

	dAssert (strat &&
		strat->pnode,
		"Foo barson");


	// Choose a sub-object at random


	if (strat == player[currentPlayer].Player)
	{
		do {
			nObj = strat->pnode->noObjId;
		   	oNum = (int)RandRange (5, nObj - 0.001f);
			obj = strat->objId[oNum]; 
		} while (IsInvisible (obj) || !obj->model ||
			(nObj >= 10 && nObj <= 12));
	}
	else
	{
		do {
			nObj = strat->pnode->noObjId;
			if (nObj)
			{
			   //	oNum = (int)RandRange (1, nObj - 0.001f);
				oNum = (int)RandRange (1, nObj);
				obj = strat->objId[oNum];
			}
			else
				obj = strat->obj;
		} while (!obj->model || !obj->model->vertex );
	   //	} while (IsInvisible (obj) || !obj->model || !obj->model->vertex );
	}

	// Choose a vertex within that object
	nVerts = obj->model->nVerts;
	vert = RandRange (0, nVerts);

	// Transform into world space
	*pos = obj->model->vertex[vert];
	mPoint3Apply (pos, &obj->wm);

	// Get the normal, if necessary
	if (normal) {
		*normal = obj->model->vertexNormal[vert];
		mVector3Apply(normal, &obj->wm);
		v3Normalise (normal);
	}
	return vert | (oNum<<16);
}

/*
 * Fractal lightning
 *
 * Takes two world-space co-ordinates, a depth, a noise amount and two colours
 * Draws lightning between the two co-ordinates, breaks it into 2^depth pieces, each
 * perturbed by up to noise, and colour interpolated between the two colours
 */
static Uint32 lCol;
static PKMDWORD rDrawLightningInternal (register PKMDWORD pkmCurrentPtr,
										Point3 *start, Point3 *end, Colour startCol, Colour endCol, 
										Colour oStartCol, Colour oEndCol, 
										int depth, float noise, float startScale, float endScale, int madness)
{
	// First case: depth == 0, we must draw the lightning itself
	if (depth <= 0) {
		Point p0, p1;
		float x0, y0, w0;
		float x1, y1, w1;
		float x2, y2, w2;
		float delX, delY, length, shiftX, shiftY, scale, sAddx, sAddy, dAddx, dAddy;
		Uint32 oCol, eCol;
		int i;

		p0.x = start->x;
		p0.y = start->y;
		p0.z = start->z;
		p0.w = 1.f;

		// Extend by 5%
		p1.x = (end->x - start->x) * 1.05f + start->x;
		p1.y = (end->y - start->y) * 1.05f + start->y;
		p1.z = (end->z - start->z) * 1.05f + start->z;
		p1.w = 1.f;

		mPoint (&p0, &p0);
		mPoint (&p1, &p1);

		// Check if it's on screen
		if (p0.z < 0 || p1.z < 0)
		 	return pkmCurrentPtr;

		w0 = 1.f / p0.w;
		w1 = 1.f / p1.w;

		x0 = pViewMidPoint.x + w0 * p0.x;
		y0 = pViewMidPoint.y + w0 * p0.y;

		x1 = pViewMidPoint.x + w1 * p1.x;
		y1 = pViewMidPoint.y + w1 * p1.y;

		x2 = 0.7f * x0 + 0.3f * x1;
		y2 = 0.7f * y0 + 0.3f * y1;
		w2 = 0.7f * w0 + 0.3f * w1;

		delX = x1 - x0;
		delY = y0 - y1;
		length = SQR(delY) + SQR(delX);
		scale = startScale * isqrt (length);
		shiftX = scale * delY;
		shiftY = scale * delX;

		oCol = startCol.colour;
		eCol = endCol.colour;

		kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		x2 - shiftX, y2 - shiftY, w2, oCol);
		kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		x0         , y0         , w0, oCol);
		kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		x1         , y1         , w1, eCol);
		kmxxSetVertex_0 (KM_VERTEXPARAM_ENDOFSTRIP,	x2 + shiftX, y2 + shiftY, w2, oCol);

		sAddx = shiftX * 1.15f;
		sAddy = shiftY * 1.15f;
		dAddx = delX * 0.05f;
		dAddy = delY * 0.05f;
		for (i = 0; i < madness; ++i) {
			shiftX += sAddx;
			shiftY += sAddy;
			delX += dAddx;
			delY += dAddy;
			x1 = delX + x0;
			y1 = y0 - delY ;
			x0 = x1 - delX;
			y0 = delY + y1;
			w0-= 0.01f;
			w1-= 0.01f;
			
			kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		x2 - shiftX, y2 - shiftY, w2, lCol);
			kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		x0         , y0         , w0, lCol);
			kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		x1         , y1         , w1, lCol);
			kmxxSetVertex_0 (KM_VERTEXPARAM_ENDOFSTRIP,	x2 + shiftX, y2 + shiftY, w2, lCol);
#if 0
//will fix when I have time
#ifdef _DEBUG
			*pkmCurrentPtr = ((KMDWORD)(*pkmCurrentPtr) & 0xFC000000) + ((KMDWORD)pkmCurrentPtr & 0x0FFFFFFF); 
	   
			if (((PKMCURRENTLISTSTATE)(desc)->pCurrentListState)->ListType != 4) 
				dAssert (*pkmCurrentPtr < (KMDWORD)((&(desc)->ppBuffer[0]->pOpaquePolygonBuffer)[((PKMCURRENTLISTSTATE)(desc)->pCurrentListState)->ListType + 1]), "Poly overflow"); 
#endif
#endif
		}
	} else {
		// Otherwise, get a midpoint, perturb it and recurse onware
		Point3 midPoint;
		Colour midCol, oMidCol;
		float midScale = (startScale + endScale) * 0.5f;
		v3Add (&midPoint, start, end);
		v3Scale (&midPoint, &midPoint, 0.5f);
		midCol.argb.a = (startCol.argb.a + endCol.argb.a)>>1;
		midCol.argb.r = (startCol.argb.r + endCol.argb.r)>>1;
		midCol.argb.g = (startCol.argb.g + endCol.argb.g)>>1;
		midCol.argb.b = (startCol.argb.b + endCol.argb.b)>>1;
		oMidCol.argb.a = (oStartCol.argb.a + oEndCol.argb.a)>>1;
		oMidCol.argb.r = (oStartCol.argb.r + oEndCol.argb.r)>>1;
		oMidCol.argb.g = (oStartCol.argb.g + oEndCol.argb.g)>>1;
		oMidCol.argb.b = (oStartCol.argb.b + oEndCol.argb.b)>>1;
		midPoint.x += RandRange (-noise, noise);
		midPoint.y += RandRange (-noise, noise);
		midPoint.z += RandRange (-noise, noise);
		pkmCurrentPtr = rDrawLightningInternal (pkmCurrentPtr, start, &midPoint, startCol, midCol, oStartCol, oMidCol, 
			depth-1, noise, startScale, midScale, madness);
		pkmCurrentPtr = rDrawLightningInternal (pkmCurrentPtr, &midPoint, end, midCol, endCol, oMidCol, oEndCol, 
			depth-1, noise, midScale, endScale, madness);
		// Fork that lightning!!!
		if ((madness > 1) && (rand() & 7) == 0) {
			Point3 endPoint;
		  	//if depth was 1, newdepth becomes 0, and then 1
			int newDepth = (depth-1)>>1;
			if (newDepth == 0)
				newDepth = 1;
			endPoint.x = midPoint.x + RandRange (-0.5, 0.5);
			endPoint.y = midPoint.y + RandRange (-0.5, 0.5);
			endPoint.z = midPoint.z + RandRange (-0.5, 0.5);
			pkmCurrentPtr = rDrawLightningInternal (pkmCurrentPtr, &midPoint, &endPoint, midCol, endCol, oMidCol, oEndCol, 
				newDepth, noise, midScale, endScale, madness-1);
		}
	}
   //	depth = depth - 1;
	return pkmCurrentPtr;
}
Material lightningMat =  { 0, 0, 0.f, 0.f, MF_ALPHA | MF_TRANSPARENCY_ADDITIVE };
void rDrawLightning (Point3 *start, Point3 *end, Colour startCol, Colour endCol,
					 Colour oStartCol, Colour oEndCol,
					 int depth, float noise,
					 float startScale, float endScale, int madness)
{
	float multer, dist;
	dAssert (start && end, "NULL points");
	dAssert (depth > 1, "No lightning to draw");
	
	if (PauseMode)
		ResetRandom();

	multer = (dist = sqrt (pSqrDist (start, end))) / 10.f;

	multer = RANGE(1, multer, 50);
	noise *= multer;

	multer = dist / 20.f;
	multer = RANGE(1, multer, 7);
	if (multer > depth)
		depth = (int)(multer + 0.5f);

	rSetMaterial (&lightningMat);
	startCol.argb.a = 0xff;
	endCol.argb.a = 0xff;
	lCol = (startCol.colour & 0xffffff) | 0x10000000;

	kmxxGetCurrentPtr (&vertBuf);
	kmxxStartVertexStrip (&vertBuf);
	
	mLoadWithXYScale (&mWorldToScreen, pViewSize.x, pViewSize.y);
	pkmCurrentPtr = rDrawLightningInternal (pkmCurrentPtr, start, end, startCol, endCol, oStartCol, oEndCol, depth, noise, startScale, endScale, madness);

	kmxxReleaseCurrentPtr (&vertBuf);
}

static int LightPosPlayer(Point3 *pos, Point3 *pos2,int vertID)
{
	Vector3 vector, normal;
   
	
	if (vertID == 0 || (rand() & 7) == 0) {
		do {
			vertID = FindRedDogVertex (player[currentPlayer].Player,pos2, &normal);
			v3Sub (&vector, pos, pos2);
			v3Normalise (&vector);
		} while (v3Dot (&normal, &vector) < 0.7f);
	} else {
		int subObj = vertID>>16;
		int vert = vertID & 0xffff;
		Object *obj = player[currentPlayer].Player->objId[subObj];
		if (vert >= obj->model->nVerts)
			vert = obj->model->nVerts;
		*pos2 = obj->model->vertex[vert];
		mPoint3Apply (pos2, &obj->wm);

	}
	// Shake the player about a bit if not paused
	if (PauseMode) {
		srand(5284);
//		XXX arse - the frand() isn't obviously seedable...
	} else {
		if ((rand() & 1) == 0)
			MakeSpark (NULL, pos2->x, pos2->y, pos2->z);
		mPreRotateXYZ (&player[currentPlayer].Player->m, RandRange(-0.01f, 0.01f), RandRange(-0.01f, 0.01f),RandRange(-0.01f, 0.01f));	
		puruVibrate (currentPlayer, 5, 1, 0.8f, 0);
	}



	return(vertID);		
}

static int LightPosStrat(STRAT *strat,Point3 *pos, Point3 *pos2,int vertID,Object *own)
{
	Vector3 vector, normal;
   
	
	if (vertID == 0 || (rand() & 7) == 0) {
	//	do {
			vertID = FindRedDogVertex (strat,pos2, &normal);
	  //		v3Sub (&vector, pos, pos2);
	  //		v3Normalise (&vector);
	  //	} while (v3Dot (&normal, &vector) < 0.7f);
	} else
	{
		int subObj = vertID>>16;
		int vert = vertID & 0xffff;
   
		Object *obj;

		
		if (own)
			obj = own;
		else
		{
			if ((!strat->pnode->noObjId) || (strat->pnode->noObjId < subObj) || (!subObj))
				obj = strat->obj;
			else
			{
				 if (subObj > strat->pnode->noObjId)
					subObj = strat->pnode->noObjId - 1;
				obj = strat->objId[subObj];
			}
		}

		if (vert > obj->model->nVerts)
			vert = obj->model->nVerts;

		*pos2 = obj->model->vertex[vert];
		mPoint3Apply (pos2, &obj->wm);

	}

	if ((rand() & 1) == 0)
		MakeSpark (NULL, pos2->x, pos2->y, pos2->z);

	return(vertID);		
}

// Strat wrapper

//mode 0 - to player STRAT
//mode 1 - to parent STRAT
//mode 2 - to MY OWN STRAT 
//mode 3 - within the same objid
int DrawLightning (STRAT *strat, int objId, float x, float y, float z, int vertID, int mode)
{
	extern void findTotalMatrixInterp (Matrix *mat, Object *obj);
	Point3 pos, pos2;
	Colour startcol, endcol;
	Colour ostartcol, oendcol;
	Matrix matrix;
	float random;
	int madness = 1;
	
	matrix = strat->m;
	mPreScale(&matrix, strat->scale.x, strat->scale.y, strat->scale.z);
	
	if ((objId) && (strat->pnode->noObjId))
		findTotalMatrixInterp(&matrix, strat->objId[objId]);
	else
		findTotalMatrixInterp(&matrix, strat->obj);

	mPostTranslate(&matrix, strat->pos.x, strat->pos.y, strat->pos.z);
	
	pos.x = x; pos.y = y; pos.z = z;

	mPoint3Multiply3(&pos, &pos, &matrix);


	switch (mode)
	{
		case 0:
			vertID = LightPosPlayer(&pos,&pos2,vertID);
			madness = 3;
			break;
		case 1:
			vertID = LightPosStrat(strat->parent,&pos,&pos2,vertID,NULL);
			madness = 2;
			break;
		case 3:
			vertID = LightPosStrat(strat,&pos,&pos2,vertID,strat->objId[objId]);
			break;
		
		case 2:
		default:
			vertID = LightPosStrat(strat,&pos,&pos2,vertID,NULL);
			madness = 2;
			break;
	
	}


	startcol.colour = 0xcf9090ff;
	endcol.colour = 0xaf3030b0;
	ostartcol.colour = 0x409090ff;
	oendcol.colour = 0x3f3030b0;
	
//	startcol.colour = 0xcf000000 | (251<<16) | (255<<8) | 171;
//	endcol.colour = 0xaf000000 | (204<<16) | (216<<8) | 11;
//	ostartcol.colour = 0x40000000 | (255<<16) | (56<<8) | 56;
//	oendcol.colour = 0x40000000 | (255<<16) | (56<<8) | 56;
	random = pow(frand(), 5.f);
	
	rDrawLightning (&pos, &pos2, startcol, endcol, 
					ostartcol, oendcol, 
					3, random + 0.4f, 5.f, 3.f, madness);

	return vertID;
}

/*
 * All-new singing and dancing water texture effect
 */

#define WATER_SIZE	64
/*
 * The water effect double-buffers are purposely extended by one pixel
 * in each direction.  The inner WATER_SIZE x WATER_SIZE pixels are the actual
 * texture itself, and the outer edge is copied from the texture so
 * that we don't have to check for wrapping at the edges
 */
static Sint8 wArray[2][(WATER_SIZE+2)*(WATER_SIZE+2)];
static int wActive = 0;

void rInitWaterTexture (void)
{
	// Clear to non-perturbed 0
	memset (wArray, 0, sizeof (wArray));
	wActive = 0;
}

void rWaterTexture (void)
{
	// Loop counters
	int y;
	register int x;
	// Three pointers to the current line, one above and one below for both A and B
	register Sint8 *curLineA, *aboveLineA, *belowLineA;
	register Sint8 *curLineB, *aboveLineB, *belowLineB;

	// Set up the pointers appropriately
	if (wActive) {
		curLineA = &wArray[0][WATER_SIZE+2 + 1];
		curLineB = &wArray[1][WATER_SIZE+2 + 1];
	} else {
		curLineA = &wArray[1][WATER_SIZE+2 + 1];
		curLineB = &wArray[0][WATER_SIZE+2 + 1];
	}
	aboveLineA = curLineA - WATER_SIZE * 2;
	aboveLineB = curLineB - WATER_SIZE * 2;
	belowLineA = curLineA + WATER_SIZE * 2;
	belowLineB = curLineB + WATER_SIZE * 2;

	// Now loop doing our little algorithm
	// Loop is unrolled by two to give the compiler something to work with
	for (y = WATER_SIZE; y; --y) {
		for (x = (WATER_SIZE/2); x; --x) {
//			*curLineA =
//				be cunning geeza!!!
		}
	}

}

extern Matrix	mWorldToView;
extern ModelContext context;
extern int TimerShow;
float SmellyFloatingPointVariable = 24.f;
void DrawArrowToTarget (STRAT *strat, float dir,Uint32 colour,int CheckVis)
{

	float y;
	extern float CurrentStratIntensity;
	float pushTrans, pushIntensity, trans;
	Point3 pushSunDir;
	Uint32 pushFlags;

	if ((CheckVis) && (strat->flag & STRAT_HIDDEN))
		return;

	pushSunDir = *(Point3 *)&sunDir;
	pushIntensity = CurrentStratIntensity;

	CurrentStratIntensity = 0.5f;
	rSetModelContext();

	mInvert3d (&mCurMatrix, &mWorldToView);
	mPreTranslate (&mCurMatrix, 0, -0.075, 0.3f);
	mPreRotateX (&mCurMatrix, -(3.141f + 1.1f));
	mPreRotateZ (&mCurMatrix, dir);
	mPreScale (&mCurMatrix, 0.03f * strat->scale.x, 0.03f * strat->scale.y, 0.03f * strat->scale.z);
	strat->pnode->obj->model->material[0].diffuse.colour = colour;
	y = context.midY;
	if (TimerShow)
		context.midY += SmellyFloatingPointVariable;
	pushFlags = rGlobalModelFlags;
	pushTrans = rGlobalTransparency;
	trans = ((float)(colour & 0xff000000))/255.0f;
	rGlobalTransparency = trans;

	rGlobalModelFlags |= MODELFLAG_STATIC_LIGHT;

	if (trans < 0.95f)
		rGlobalModelFlags |= MODELFLAG_GLOBALTRANS;

	rDrawObject(strat->pnode->obj);
	rGlobalModelFlags = pushFlags;
	rGlobalTransparency = pushTrans;

	*(Point3 *)&sunDir = pushSunDir;
	CurrentStratIntensity = pushIntensity;
	rSetModelContext();

	context.midY = y;
}

void DrawArrowToPoint(Point3 *p,Uint32 colour)
{
	Vector3 forward, playerToPoint;
	float ang;

	if (!ArrowStrat)
		return;

	forward.x = player[0].CameraStrat->m.m[1][0];
	forward.y = player[0].CameraStrat->m.m[1][1];
	forward.z = 0.0f;

	v3Sub(&playerToPoint, p, &player[0].Player->pos);
	playerToPoint.z = 0.0f;
	v3Normalise(&playerToPoint);
	v3Normalise(&forward);
	ang = acos(v3Dot(&playerToPoint, &forward));

	if (playerToPoint.x * forward.y - playerToPoint.y * forward.x > 0.0f)
		ang *= -1.0f;

	DrawArrowToTarget(ArrowStrat, ang,colour,0);
}

float LightningAmount;
int LightningTime;
int NumStrikes;

static void InitLightning (void)
{
	LightningAmount = 1 + frand() * 0.5f;
	LightningTime = 0;
	NumStrikes = (int)(pow(frand(), 5.f) * 3.f);
}


#define LIGHTNINGINIT 1


float LightningFallOff = 0.75f;
int ProcessLightning (int mode)
{
	bool finished = FALSE;

	if (mode == LIGHTNINGINIT)
	{
		InitLightning();
		return(1);

	}




	if (!PauseMode) {
		LightningTime++;
		LightningAmount *= LightningFallOff;
		if (LightningAmount < 0.5f && NumStrikes) {
			NumStrikes--;
			LightningAmount += 0.4f + frand() * 0.2f;
		}
		if (LightningAmount < 0.05f)
			finished = TRUE;
	}

	if (finished) {
		rFlareB = rFlareG = rFlareR = 0.f;
	} else {
		rFlareB = rFlareG = rFlareR = LightningAmount;
	}

	return finished;
}
