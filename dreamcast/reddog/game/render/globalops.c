/*
 * Render/GlobalOps.c
 * Global operations on the renderer
 */

#include "..\RedDog.h"
#include "Internal.h"
#include "Rasterisers.h"
#include "..\View.h"
#include "Bands.h"
#include "Sparks.h"
#include "kmutil.h"
#include "..\backup.h"
#include "..\GameState.h"
#include "..\Input.h"
#include "Hardware.h"

#define SKY_DEMO 0

extern int FramesNeedingProcessing;
int RenderOnlyTime = 0;
Bool suppressDebug = TRUE;
static volatile bool OKToTexLoad = FALSE;
volatile bool TexturesReadyForDMA = FALSE;
int PipePercent[5];

/*
 * Set the background colour
 */
void rSetBackgroundColour (Uint32 colour)
{
	KMVERTEX1	vert[3];
	void		*v[3];
	KMSTATUS	result;
	KMPACKEDARGB col;
	float r, g, b;
	r =  ((colour>>16)&0xff) * (1.f/256.f);
	g =  ((colour>>8)&0xff) * (1.f/256.f);
	b =  ((colour>>0)&0xff) * (1.f/256.f);

	v[0] = &vert[0];
	v[1] = &vert[1];
	v[2] = &vert[2];

	vert[0].ParamControlWord	= KM_VERTEXPARAM_NORMAL;
	vert[0].fX					= 0.0f;
	vert[0].fY					= 0.0f;
	vert[0].u.fInvW				= 0.00001f;
	vert[0].fBaseAlpha			= 1.f;
	vert[0].fBaseRed			= r;
	vert[0].fBaseGreen			= g;
	vert[0].fBaseBlue			= b;

	vert[1].ParamControlWord	= KM_VERTEXPARAM_NORMAL;
	vert[1].fX					= PHYS_SCR_X;
	vert[1].fY					= 0.0f;
	vert[1].u.fInvW				= 0.00001f;
	vert[1].fBaseAlpha			= 1.f;
	vert[1].fBaseRed			= r;
	vert[1].fBaseGreen			= g;
	vert[1].fBaseBlue			= b;

	vert[2].ParamControlWord	= KM_VERTEXPARAM_ENDOFSTRIP;
	vert[2].fX					= 0.0f;
	vert[2].fY					= PHYS_SCR_Y;
	vert[2].u.fInvW				= 0.00001f;
	vert[2].fBaseAlpha			= 1.f;
	vert[2].fBaseRed			= r;
	vert[2].fBaseGreen			= g;
	vert[2].fBaseBlue			= b;
	
	result = kmSetBackGroundPlane (v, KM_VERTEXTYPE_01, sizeof (KMVERTEX1));
	dAssert (result == KMSTATUS_SUCCESS, "Unable to set background colour!");

	col.dwPacked = 0;
	kmSetBorderColor (col);

}

/*
 * Set the perspectivization matrix
 * Notes the near-val for W
 * FOV is measured from top to bottom
 */
float WnearVal, rWnearVal, farDist;
void rSetPerspective (float FOV, float aspectRatio, float nearPlane, float farPlane)
{
	mPerspective (&mPersp, FOV, aspectRatio / X_SCALE, nearPlane, farPlane);
	farDist = farPlane;
	WnearVal = - mPersp.m[3][2] / mPersp.m[2][2];
	rWnearVal = 1.f / WnearVal;
}

/*
 * Sets up the fog parameters
 */
static Uint8 _fog[129];
static Uint16 _fogDensity = 0xff09;
static KMPACKEDARGB	_col;
void rSetFog (Uint32 colour, float nearFog, float farFog, float fogScale)
{
	/*
	 * Scalefactor should be the same as, or less than farFog
	 */
	register int i;
	Uint8 exp, mant, *write;
	float scaleFactor, dist, fogAmt, fogGradient;

	dAssert (farFog > nearFog, "Silly values");

	exp = (Uint8) ceil((log(farFog) * (1.f / 0.69314718f))); /* log(2) */
	mant = (Uint8) (128.f * (float)(1<<exp) / farFog);
	scaleFactor = (float)mant / 128.f * (float)(1 << exp);

	fogGradient = 1.f / (farFog - nearFog);

	write = _fog;
	for (i = 0; i < 128; ++i) {
		dist = scaleFactor / ((float) (1<<(i>>4)) * (float)((i & 0xf) + 16) / 16.f);
		if (dist > nearFog) {
			fogAmt = (dist - nearFog) * fogGradient;
			if (fogAmt > 1.f)
				fogAmt = 1.f;
		} else {
			fogAmt = 0.f;
		}
		*write++ = (Uint8)(fogAmt * fogScale * 255);
	}
	_col.dwPacked = colour;
	_fogDensity = (mant << 8) | exp;
}

/*
 * Draw and update the SFX respectively
 */
bool WeatherNeedsDoing = TRUE, BandsNeedDoing = TRUE, FragmentsNeedDoing = TRUE, SparksNeedDoing = TRUE, SavsSparksNeedDoing = TRUE;
void sfxUpdate (void)
{
	if (!PauseMode) {
		DoGlobalTexAnims();
		if (WeatherNeedsDoing)
			rUpdateWeatherEffects();
		if (BandsNeedDoing)
			bUpdate();
		if (!Multiplayer) {
			if (FragmentsNeedDoing)
				rUpdateFragments();
			if (fireUsed)
				rProcessFire();
			if (SparksNeedDoing)
				rUpdateSparks();
		}
		WeatherNeedsDoing = BandsNeedDoing = FragmentsNeedDoing = SparksNeedDoing = SavsSparksNeedDoing = FALSE;
	}
}
void sfxDraw (void)
{
	bDraw();
 	rDrawWeatherEffects();
	if (!Multiplayer) {
		rDrawSparks();
		rDrawWater();
		rDrawFragments();
	}
	rDrawDecals();
}

/*
 * End of frame function
 */
Uint32 genCalls = 0;
static const BBox dummyBounds = { {0,0,0}, {1,1,1} };
float rFlareR, rFlareG, rFlareB;
void rEndFrame (void)
{
	static Uint32 calcT, drawT, time;
	static float smoothFPS = 0.f, smoothCalcFPS = 0, smoothRenderPSec = 0;
	static const float alpha = 0.4f;
	extern int suppressReset;
	float calcFPS, FPS, RenderPSec;
	int i;

	calcT = syTmrDiffCount (time, syTmrGetCount());	

	pbMark(0x00ffff, TRUE, PB_POSTAMBLE);
	SoundUpdatePreDraw();
	pbMark(0x80ff80, TRUE, PB_SOUND);
#if !SKY_DEMO
	if (InputMode == DEMOREAD && currentFrame & 16)
	{
		kmSetUserClipping (&vertBuf, KM_OPAQUE_POLYGON, 0, 0, 255, 255);
		kmSetUserClipping (&vertBuf, KM_OPAQUE_MODIFIER, 0, 0, 255, 255);
		kmSetUserClipping (&vertBuf, KM_TRANS_POLYGON, 0, 0, 255, 255);
		mUnit32 (psGetMatrix());
		psSetWindow (0, 0, PHYS_SCR_X, PHYS_SCR_Y);
		psSetPriority(10000);
		psSetAlpha(1);
		psSetColour(0xffffff);
		psDrawCentred ("`DEMOHITSTART", 400-16);

	}
#endif
	// Pop something at the top left if we're VMSing
	if (suppressReset || SaveFailed) {
		Matrix32 savedM;
		memcpy (&savedM, psGetMatrix(), sizeof (savedM));
		psSetWindow (32, 8, PHYS_SCR_X-8, PHYS_SCR_Y-32);
		psSetPriority(10000);
		psSetAlpha(1);
		psSetColour(0x00ff00);
		mUnit32 (psGetMatrix());
		mPostScale32(psGetMatrix(), 0.75f, 0.75f);
		if (SaveFailed) {
			SaveFailed--;
			psDrawString ("`VMUSAVEFAIL", 0, 0);
		} else 
			psDrawString ("`VMUACCESS", 0, 0);
		memcpy (psGetMatrix(), &savedM, sizeof (savedM));
	}

	pbDraw();

#ifdef _DEBUG
	// Read out the pipe values
	for (i = 1; i < 4; ++i) {
		int bytes = vertBuf.pCurrentPtr[i] - (&vertBuf.ppBuffer[0]->pOpaquePolygonBuffer)[i];
		int size = (&vertBuf.ppBuffer[0]->pOpaquePolygonBuffer)[i+1] - (&vertBuf.ppBuffer[0]->pOpaquePolygonBuffer)[i];
		PipePercent[i] = (100 * bytes) / size;
	}
#endif

	kmRender();
	pbEnd();
	pbReset();
	kmFlipFrameBuffer();
	pbMark (0xff407f, FALSE, PB_POSTAMBLE);
	pdExecPeripheralServer();

	// Moved to the vblank in sndutls.c
	SoundUpdatePostDraw();
	pbMark(0x80ff80, TRUE, PB_SOUND);

	rFlareR = rFlareG = rFlareB = 0.f;

	drawT = syTmrDiffCount (time, syTmrGetCount());
	
	FPS = 1e6f / ((float) syTmrCountToMicro(drawT));
	calcFPS = 1e6f / ((float) syTmrCountToMicro(calcT));
	smoothFPS = alpha * smoothFPS + (1.f - alpha) * FPS;

	smoothCalcFPS = alpha * smoothCalcFPS + (1.f - alpha) * calcFPS;

	time = syTmrGetCount();

	// Animate one of the VMU graphics
	{
		static int InputChecks = 0;
		int checkNum;
		InputChecks ++;
		checkNum = InputChecks;
		if (checkNum & 1) {
			extern int PadFrame[4], nLogoFrames;
			i = (checkNum>>1) & 3;
			PadFrame[i] = (PadFrame[i]+1) % nLogoFrames;
			lcdClear (i);
			lcdUpdate(i);
		}
	}

	/*
#ifdef _DEBUG
	if (!suppressDebug) {
		for (i = 1; i < 4; ++i) {
			static char *PipeName[5] = { "", "Shad", "Trans", "TransShad" };
			tPrintf (0, 3+i, "%s:%d%%", PipeName[i], PipePercent[i]);	
		}
	}
#endif
	*/

	if (FPS < 29.5f)
		FramesNeedingProcessing++;

	if (currentFrame > 10 && !suppressDebug) {
#if SHOW_FPS
		tPrintf (0, 0, "%.1f/%.1f FPS", smoothCalcFPS, smoothFPS);
#if COUNT_GEOMETRY
		tPrintf (0, 1, "%d/%d tPS", (int)(smoothCalcFPS*(nDrawn+nTris)), (int)(smoothFPS*(nDrawn+nTris)));
/*		{
		extern int nBoxesDrawn, nBoxesClipped;
		tPrintf (0, 5, "%d/%d box", nBoxesDrawn, nBoxesClipped);
		}*/
#endif
#endif
#if COUNT_GEOMETRY
		tPrintf (20, 0, "%d totl", nDrawn);	
		tPrintf (20, 1, "%d obj", nDrawn - nScape);	
		tPrintf (20, 2, "%d scape", nScape);	
		tPrintf (20, 3, "%d shad", nTris);	
		tPrintf (20, 4, "%d lit", nLit);	
		tPrintf (20, 5, "%d light%s", nLights, (nLights==1)?"":"s");	
#endif
	}

	genCalls = 0;
#if COUNT_GEOMETRY
	nDrawn = nTris = nScape = nLit = nLights = 0;
#endif
}

int rFindAnim (const PNode *node, const char *name)
{
	Uint16 i;

	dAssert (node && node->anim, "No animations");
	for (i=0; i < node->anim->nAnims; ++i)
	{
		if (!stricmp (node->anim->anim[i].name, name))
			return i;
	}
	return -1;
}

void mLoadModelToScreen (void)
{
	mLoad (&mWorldToScreen);
	mMul (&modelToScreen, &mCurMatrix);
	mLoad (&modelToScreen);
}

void mLoadModelToScreenScaled (void)
{
	mLoad (&mWorldToScreen);
	mMul (&modelToScreen, &mCurMatrix);
	mLoadWithXYScale (&modelToScreen, pViewSize.x, pViewSize.y);
}

Uint32 rColClampMin = 0x00000000;
Uint32 rColClampMax = 0xffffffff;

void EORCallback(void *ignored)
{
	KMPACKEDARGB min, max;
	KMPACKEDARGB phogge;
	register int i;
	register volatile Uint32 *ptr;
	Uint32 t;
	register Uint8 *read;

	(void)ignored;

	/* Do the Colour Clamping */
	min.dwPacked = rColClampMin;
	max.dwPacked = rColClampMax;

	rSetRegister (HW_COLCLAMP_MIN, rColClampMin);
	rSetRegister (HW_COLCLAMP_MAX, rColClampMax);

	/* Do the Phogge */
	rSetRegister (HW_FOG_DENSITY, _fogDensity);

	ptr = (volatile Uint32 *)(HW_BASE + HW_FOG_TABLE);
	read = _fog+1;
	t = _fog[0];
	for (i = 128; i > 0; --i) {
		t<<=8;
		t |= *read++;
		*ptr++ = (t & 0xffff);
	}
	/* Clamp said Phogge to Colour Clamp */
	phogge.byte.bAlpha = _col.byte.bAlpha;
	phogge.byte.bRed = RANGE(min.byte.bRed, _col.byte.bRed, max.byte.bRed);
	phogge.byte.bGreen = RANGE(min.byte.bGreen, _col.byte.bGreen, max.byte.bGreen);
	phogge.byte.bBlue = RANGE(min.byte.bBlue, _col.byte.bBlue, max.byte.bBlue);

	rSetRegister (HW_FOG_TABLE_COLOUR, phogge.dwPacked);

	/* Fire support */
	if (fireUsed)
		rFireCallback();

	/* Text uploading */
//	tCallback();

	OKToTexLoad = TRUE;

	EORMark();
}

volatile int DisableTexDMAs;
void WaitCallback (void *ignore)
{
	Uint32 TimerVal, timeOut;
	extern volatile nTexturesToDMA;

	//SoundUpdatePostDraw();
	// If we're not actually running the animated textures, then return
	if (DisableTexDMAs || PauseMode) {
		nTexturesToDMA = 0;
		WeatherNeedsDoing = BandsNeedDoing = FragmentsNeedDoing = SparksNeedDoing = SavsSparksNeedDoing = TRUE;
	} else if (nTexturesToDMA && TexturesReadyForDMA && OKToTexLoad) {
		DoGlobalTexAnimDMA();
		OKToTexLoad = TexturesReadyForDMA = FALSE;
		WeatherNeedsDoing = BandsNeedDoing = FragmentsNeedDoing = SparksNeedDoing = SavsSparksNeedDoing = TRUE;
	} else if (GetGameState() == GS_GAME && !PauseMode) {
		if (WeatherNeedsDoing) {
			rUpdateWeatherEffects();
			WeatherNeedsDoing = FALSE;
		} else if (BandsNeedDoing) {
			bUpdate();
			BandsNeedDoing = FALSE;
		} else if (!Multiplayer) {
			if (FragmentsNeedDoing) {
				rUpdateFragments();
				FragmentsNeedDoing = FALSE;
			} else if (SparksNeedDoing) {
				rUpdateSparks();
				SparksNeedDoing = FALSE;
			} else if (SavsSparksNeedDoing) {
//				processSparks();
//				SavsSparksNeedDoing = FALSE;
			}
		}
	}
}

void ResetTexSys(void)
{
	OKToTexLoad = TexturesReadyForDMA = FALSE;
}

float WobbleAmount = 8.f;
float WobbleFrequency = (2*PI/64);
float WobbleSpeed = (30*2*PI/2);

void HSyncInterrupt (void *ignored)
{
	int line;
	float amount;
	SinCosVal v;
	kmGetCurrentScanline(&line);
	SinCos (line * WobbleFrequency + (currentFrame * WobbleSpeed), &v);
	amount = WobbleAmount * sin(v.sin);
	kmAdjustDisplayCenter ((int)amount, 0);
	line+=2;
	if (line >= PHYS_SCR_Y)
		line -= PHYS_SCR_Y;
	kmSetHSyncLine (line);
}

/***************************************************/

// Reset the whole rendering subsystem
void rReset(void)
{
	// Reset the textures! Otherwise we have stale pointers in it to texture anims
	texInit();
	// Clear the mission heap
	SHeapClear (MissionHeap);
	lInit();
	rSeaEffectOff();
	bReset();
	rResetSparks();
	rResetWeather();
	rInitDecals();
}

/*
 * Sets the specular flash action craziness warez
 */
static void RecurseMadness(Object *o, Uint32 col)
{
	int i;
	o->specularCol.colour = col;
	for (i = 0;i < o->no_child; ++i)
		RecurseMadness (o->child + i, col);
}

void SetSpecularColour(STRAT *s, int objId, float r, float g, float b)
{
	Uint32 col = ((int)(r*255)<<16) | ((int)(g*255)<<8) | ((int)(b*255));
	if (objId) {
		s->objId[objId]->specularCol.colour = col;
	} else {
		RecurseMadness (s->obj, col);
	}
}

float GetSpecularColour(STRAT *s, int objId, int mode)
{
	if (s)
	{
		if (objId)
		{
			switch(mode)
			{
				case 0:
					return (((float)s->objId[objId]->specularCol.argb.r) * (1.0/255.0));
				case 1:
					return (((float)s->objId[objId]->specularCol.argb.g) * (1.0/255.0));
				case 2:
					return (((float)s->objId[objId]->specularCol.argb.b) * (1.0/255.0));
			}
		}
		else
		{
			switch (mode)
			{
				case 0:
					return ((float)s->obj->specularCol.argb.r) * (1.0/255.0);
				case 1:
					return ((float)s->obj->specularCol.argb.g) * (1.0/255.0);
				case 2:
					return ((float)s->obj->specularCol.argb.b) * (1.0/255.0);
			}
		}
	}

	return 0.0f;
}

void CrashOut (char *message, Uint32 col1, Uint32 col2)
{
   	int i, j, k;
	strcpy ((char*)0x8c000000, message);
	k = 0;
	while (1) {
		for (j = 0; j < 2000; ++j) {
			for (i = 0; i < 1000; ++i)
				rSetRegister (HW_BG_COLOUR, col1);
			
			for (i = 0; i < 1000; ++i)
				rSetRegister (HW_BG_COLOUR, col2);
		}
		k = !k;
		kmBlankScreen(k);
	}
}
