/*
 * Loading.c
 * Do something while we load
 */

#include "RedDog.h"
#include "View.h"
#include "Render\Internal.h"
#include "Loading.h"

static Bool active = FALSE;
Bool NoLoadingScreen = FALSE;

extern int suppressReset;
Camera LoadingCamera;
static int num = 0;
static int timer = 0;
extern void DrawHudSymbol(float rot, float addX, float addY);

float LoadBaseAmount = 0;
float LoadScaleAmount = 0;
extern float sGetPC(void);

bool IsLoading(void) { return active; }

void LoadPoll (void)
{
	// 3fps
	if (active && (syTmrCountToMicro(syTmrDiffCount (timer, syTmrGetCount())) >= ((1000*1000)/10))) {
		SoundUpdatePreDraw();
		if (!NoLoadingScreen) {
			CameraSet (&LoadingCamera, 0);
//			psDrawCentred (psFormat ("`LOADING", num), 300);
			psDrawCentred ("`LOADING", 200);
			DrawHudSymbol (((sGetPC() * LoadScaleAmount) + LoadBaseAmount) * 0.01f, PHYS_SCR_X - 80, PHYS_SCR_Y - 83);
			timer = syTmrGetCount();
			kmRender();
			kmFlipFrameBuffer();
		}
	}
}

void BeginLoading(void)
{
	active = TRUE;

	LoadingCamera.type = CAMERA_LOOKAROUND;

	LoadingCamera.nearZ				= 0.25f;
	LoadingCamera.farZ				= 10000.f;

	LoadingCamera.fogNearZ			= 0.1f;
	LoadingCamera.fogColour.colour	= 0;
	
	SetWindow (&LoadingCamera, WINDOW_FULLSCREEN);
	LoadingCamera.screenAngle		= 1.1f;

	LoadingCamera.flags = 0;

	LoadingCamera.pos.x = LoadingCamera.pos.y = LoadingCamera.pos.z = 0.f;

	psSetColour(0xffffff);
	psSetAlpha (1);
	mUnit32 (psGetMatrix());

	suppressReset++;
	LoadBaseAmount = LoadScaleAmount = 0;

	kmSetEORCallback(NULL, NULL);
	kmSetWaitVsyncCallback(NULL, NULL);

	kmRender();
	kmFlipFrameBuffer();
	timer = 0;
	LoadPoll();
	timer = 0;
	LoadPoll();

	LoadBaseAmount = LoadScaleAmount = 0;
}

void LoadSetWadAmount(float amount)
{
	LoadBaseAmount += LoadScaleAmount * 100.f;
	LoadScaleAmount = amount / 100.f;
}

void EndLoading (void)
{
	active = FALSE;
	suppressReset--;

	/* Initialise the EOR callback routines */
	kmSetEORCallback (EORCallback, 0);

	/* Initialise the EOR callback routines */
	{
		extern void WaitCallback (void *);
		kmSetWaitVsyncCallback (WaitCallback, 0);
	}
}

