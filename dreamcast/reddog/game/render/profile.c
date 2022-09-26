/*
 * Profile bar routines
 */

#include "..\Reddog.h"
#include "..\input.h"
#include "Internal.h"

Uint32 pbTime = 0, pbRenderTime = 0, pbRest = 0, pbStartRender;
int toggle = 0;
static float posX = 10.f, posY = PHYS_SCR_Y-48;
static Material	barMat = { 0, 0, 0.f, 0.f, MF_ALPHA | MF_FRONT_END };
#define ONE_FRAME (PHYS_SCR_X/4)

Uint32 TypeTime[PB_MAX];
bool drawCumulative;

static float GetLen (Uint32 time)
{
	Uint32 ms = syTmrCountToMicro (time);
	float seconds = (float)ms / 1e6;

	return seconds * 2 * FRAMES_PER_SECOND * ONE_FRAME;
}

static void DrawBar (Uint32 colour, float length)
{
	colour &= 0xffffff;

	rSetMaterial (&barMat);

	kmxxGetCurrentPtr (&vertBuf);
	kmxxStartVertexStrip (&vertBuf);
	kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		posX, posY, 10e7f, colour | 0x80000000);
	kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		posX, posY+8, 10e7f, colour | 0x80000000);
	kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		posX+length, posY, 10e7f, colour | 0x80000000);
	kmxxSetVertex_0 (KM_VERTEXPARAM_ENDOFSTRIP,	posX+length, posY+8, 10e7f, colour | 0x80000000);
	posX += length;
#ifdef _ARTIST
	/*
	 * Special case speed warning
	 */
	if (colour == 0xff0000 && length > (ONE_FRAME / 2)) {
		kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		0, 0, 10000, colour | 0x20000000);
		kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		0, PHYS_SCR_Y, 10000, colour | 0x30000000);
		kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		PHYS_SCR_X, 0, 10000, colour | 0x20000000);
		kmxxSetVertex_0 (KM_VERTEXPARAM_ENDOFSTRIP,	PHYS_SCR_X, PHYS_SCR_Y, 10000, colour | 0x30000000);
	}
#endif
	kmxxReleaseCurrentPtr (&vertBuf);
}

/*
 * Called to reset the profile bar every frame
 */
void pbReset (void)
{
	Uint32 now = syTmrGetCount();
	rFixUpMaterial (&barMat, &sfxContext);
	posX = 10.f;
	pbTime = now;
}

/*
 * Called at the end of a frame
 */
void pbEnd (void)
{
	Uint32 now = syTmrGetCount();
	pbRest = syTmrDiffCount (pbTime, now);
}

static void DoBlob(float pos)
{
	rSetMaterial (&barMat);

	kmxxGetCurrentPtr (&vertBuf);
	kmxxStartVertexStrip (&vertBuf);
	kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		pos, PHYS_SCR_Y-40, 10000, 0xffffffff);
	kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		pos-3, PHYS_SCR_Y-36, 10000, 0xffffffff);
	kmxxSetVertex_0 (KM_VERTEXPARAM_ENDOFSTRIP,	pos+3, PHYS_SCR_Y-36, 10000, 0xffffffff);
	kmxxReleaseCurrentPtr (&vertBuf);
}

/*
 * Called to end the profile bar, and add on a bar commesurate with the draw time last frame
 */
extern Bool suppressDebug;
typedef struct {
	ProfileType t;
	Uint32 time;
} pSortType;

pSortType sortIt[PB_MAX];

char *pbTypeName[] =
{
	"Camera/ScapeLight",
	"Strat",
	"Collision",
	"Sfx",
	"ScapeDraw",
	"StratDraw",
	"Shadow",
	"Postamble",
	"Sound",
	"<max>"
};

int sortRout (void *a, void *b)
{
	pSortType *pA = (pSortType *)a;
	pSortType *pB = (pSortType *)b;

	return pB->time - pA->time;
}

void pbDraw (void)
{
#if PROFILE_BARS
	if (pbRenderTime && pbRest && !suppressDebug) {
		DrawBar (0xffffff, GetLen (pbRest));
		posY = PHYS_SCR_Y-48;
		posX = 10.f;
		DrawBar (0xff8040, GetLen(pbRenderTime));
		posY = PHYS_SCR_Y-56;
	}
	if (!suppressDebug) {
		DoBlob(ONE_FRAME+10.f);
		DoBlob(ONE_FRAME*2+10.f);
		DoBlob(ONE_FRAME*3+10.f);
	}
//	if (PadPress[1] & PADX)
//		drawCumulative = !drawCumulative;
	if (PadPress[1] & PADY)
		memset (TypeTime, 0, sizeof (TypeTime));
		
	if (drawCumulative) {
		Uint32 total;
		int i;
		for (i = 0, total = 0; i < PB_MAX; ++i) {
			total += TypeTime[i];
			sortIt[i].t = i;
			sortIt[i].time = TypeTime[i];
		}

		qsort (sortIt, PB_MAX, sizeof (pSortType), sortRout);

		for (i = 0 ; i < PB_MAX; ++i) {
			psDrawString (psFormat ("%s %.1f%%", pbTypeName[sortIt[i].t], ((float)sortIt[i].time/(float)total)*100.f), 16, 60+32*i);
		}

	}
	pbStartRender = syTmrGetCount();
#else
	return;
#endif
}

void pbMark (Uint32 colour, Bool isRendering, ProfileType type)
{
#if PROFILE_BARS
	if (!suppressDebug) {
		extern int RenderOnlyTime;
		Uint32 now = syTmrGetCount();
		Uint32 time = syTmrDiffCount (pbTime, now);
		TypeTime[type] += time;

		if (isRendering)
			RenderOnlyTime += time;
		pbTime = now;
		DrawBar (colour, GetLen(time));
	}
#else
	return;
#endif
}

/*
 * Called on end of render
 */
void EORMark (void)
{
#if PROFILE_BARS
	Uint32 now = syTmrGetCount();
	rFixUpMaterial (&barMat, &sfxContext);

	pbRenderTime = syTmrDiffCount (pbStartRender, now);
#endif
}
