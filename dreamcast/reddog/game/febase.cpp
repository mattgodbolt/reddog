#include "FE.h"
#include "Menus.h"
#include "MiscScreens.h"
#include "VMSstruct.h"
#include "LevelDepend.h"
#include "Layers.h"

extern int PausedBy;
extern short DemoOn;
extern int ExitedThroughReset;
// Draw a cursor
#define CURS_COLOUR 0xffff0000
#define PAD_TIMEOUT (220) // just over three seconds at 60fps
static Material	feNoise = { 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA };
static Material	bgPicture= { 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_BILINEAR };
Material	vmuMaterial = { 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA | MF_CLAMP_V | MF_CLAMP_U };
Material	testCard = { 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_BILINEAR };
Material	titlePageMtl = { 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_BILINEAR };
Material	intelligence = { 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA | MF_CLAMP_V | MF_CLAMP_U };
Material	tank = { 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA | MF_CLAMP_V | MF_CLAMP_U };
Material	tankInside = { 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_CLAMP_V | MF_CLAMP_U };
Material	padMat = { 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA | MF_CLAMP_V | MF_CLAMP_U };
Material	techMat= { 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA | MF_CLAMP_V | MF_CLAMP_U };
Material	segaMat= { 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_NO_FILTER | MF_ALPHA | MF_CLAMP_V | MF_CLAMP_U };
Material	argoMat= { 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA | MF_CLAMP_V | MF_CLAMP_U };
Material	zoomMat[MAX_MISSIONS] = {
	{ 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA | MF_CLAMP_V | MF_CLAMP_U },
	{ 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA | MF_CLAMP_V | MF_CLAMP_U },
	{ 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA | MF_CLAMP_V | MF_CLAMP_U },
	{ 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA | MF_CLAMP_V | MF_CLAMP_U },
	{ 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA | MF_CLAMP_V | MF_CLAMP_U },
	{ 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA | MF_CLAMP_V | MF_CLAMP_U },
	{ 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA | MF_CLAMP_V | MF_CLAMP_U },
	{ 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA | MF_CLAMP_V | MF_CLAMP_U },
	{ 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA | MF_CLAMP_V | MF_CLAMP_U },
	{ 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA | MF_CLAMP_V | MF_CLAMP_U },
	{ 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA | MF_CLAMP_V | MF_CLAMP_U },
	{ 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA | MF_CLAMP_V | MF_CLAMP_U },
	{ 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA | MF_CLAMP_V | MF_CLAMP_U }
};

static bool OnTitlePage = false, ShowVersion = false, ShownLogo = false;
extern "C" void CancelledTitleLoop() { ShownLogo = true; }
bool SoundOverride = false;
void LookForCheats(Uint32 press);
Uint32 CalculateSpecials(void);

#ifndef _RELEASE
static void MemStats(char *s);
#else
#define MemStats(s)
#endif

bool Menu::InitialDelayFlag = true;

int SpecialCheatingNumber = -1, NumReady = 0;
bool FrontEndStillInitialised = false;

PNode *ArgoLogo, *World;

// Unlock all of the profiles
static void UnlockAllProfiles()
{
	int i,j;
	for (i = 0; i < 8; ++i) {
		for (j = 0; j < 4; ++j)
			if (BupIsProfileLocked(i, j))
				BupUnlockProfile(i, j);
		if (BupIsVMSLocked(i))
			BupUnlockVMS(i);
	}
}

bool MEM_CHECK = false;
VMUManager *theVMUManager;
bool OverrideStatic = false;
/////////////////////////////////////////////////////////
// Window class
// Holds all the information pertaining to screens for
// a viewport
/////////////////////////////////////////////////////////

WindowPos Window::validPositions[] =
{
	{ 1, 433 + PHYS_SCR_X/2, 439 + PHYS_SCR_Y/2 },
	{ 2, 256 + PHYS_SCR_X/4, 14 + PHYS_SCR_Y/4 },
	{ 2, 14 + PHYS_SCR_X/4, 754 + PHYS_SCR_Y/4 },
	{ 4, 160 + PHYS_SCR_X/8, 384 + PHYS_SCR_Y/8 },
	{ 4, 774 + PHYS_SCR_X/8, 128 + PHYS_SCR_Y/8 },
	{ 4, 856 + PHYS_SCR_X/8, 876 + PHYS_SCR_Y/8 },
};
int	Window::nPositions = asize (Window::validPositions);

float Window::flashTab[16] = 
{
	0.5,		// 0
	1.0,		// 1
	1.0,		// 2
	1.0,		// 3
	1.0,		// 4
	0.75,		// 5
	0.25,		// 6
	0.0,		// 7
	0.0,		// 8
	0.0,		// 9
	0.0,		// 10
	0.0,		// 11
	0.0,		// 12
	0.0,		// 13
	0.0,		// 14
	0.0,		// 15
};

float Window::scaleTab[43][2] =
{
	// Shrink
	{ 1.f,		1.f },
	{ 1.f,		0.7f },
	{ 1.f,		0.52f },
	{ 1.f,		0.36f },
	{ 1.f,		0.22f },
	{ 1.f,		0.1f },
	{ 1.f,		0.03f },
	{ 1.f,		0.03f },
	{ 1.f,		0.08f },	
	{ 1.f,		0.01f },
	{ 0.9f,		0.01f },
	{ 0.7f,		0.01f },
	{ 0.52f,	0.01f },
	{ 0.36f,	0.01f },
	{ 0.22f,	0.01f },
	{ 0.1f,		0.01f },
	{ 0.03f,	0.01f },
	{ 0.03f,	0.01f },
	{ 0.08f,	0.01f },	
	{ 0.00f,	0.00f },
	// Grow
	{ 0.01f,	0.05f },
	{ 0.01f,	0.1f },
	{ 0.01f,	0.2f },
	{ 0.01f,	0.35f },
	{ 0.01f,	0.45f },
	{ 0.01f,	0.55f },
	{ 0.01f,	0.68f },
	{ 0.01f,	0.75f },
	{ 0.01f,	0.83f },
	{ 0.05f,	0.90f },
	{ 0.1f,		0.96f },
	{ 0.2f,		1.02f },
	{ 0.35f,	1.04f },
	{ 0.45f,	1.02f },
	{ 0.55f,	1.f },
	{ 0.68f,	1.f },
	{ 0.75f,	1.f },
	{ 0.83f,	1.f },
	{ 0.90f,	1.f },
	{ 0.96f,	1.f },
	{ 1.02f,	1.f },
	{ 1.04f,	1.f },
	{ 1.02f,	1.f },
};

Window::PlayerState	Window::playerState[4];					// The state each player is currently in
int	Window::PadPanic[4];
int	Window::PlayerVMU[4] = { -1, -1, -1, -1} ;		// The VMU a player has selected
int Window::PlayerProfile[4] = { -1, -1, -1, -1} ;	// The profile number a player has selected


// Construct a window
Window::Window (int player, ScreenID &initialScreen)
{
	this->player = player;
	curScreen = firstScreen = initialScreen.Create (this);
	nextScreen = NULL;

	CreateCamera();

	if (player == -1)
		lerp = 0.f;
	else
		lerp = RandRange(-60.f, 0);

	flashAmt = 0.f;
	motion = X_MOVE;
	curPos = validPositions[0];
	targetPos = validPositions[0];
	stackPtr = 0;

	status = POWERING_UP;
	scaleIdx = 0;
	scaleAdd = 0;

	deleteIt = deletePadPanic = false;

	nFramesToDist = 120 + (rand() & 0x3f);
	curDistraction = NULL;

	curScreen->Initialise();
}

// Create the camera
void Window::CreateCamera()
{
	extern float GlobalFogRed, GlobalFogBlue, GlobalFogGreen;
	ASSERT_VALID(this);
	camera = CameraCreate();
	camera->screenAngle = (PI/4.f) / (640.f/480.f); // 45 degrees over 1.33
	camera->type = tagCamera::CAMERA_LOOKAROUND;
	if (GetGameState() != GS_GAME) {
		camera->farZ = GlobalFogFar = 10000.f;
		camera->fogNearZ = 0.99f * camera->farZ;
	} else {
		extern float GlobalFogNear;
		camera->farZ = GlobalFogFar;
		camera->fogNearZ = GlobalFogNear;
	}
	camera->u.lookDir.rotX = camera->u.lookDir.rotY = camera->u.lookDir.rotZ = 0;
	camera->fogColour.colour = (0xff<<24)|(((int)(GlobalFogRed*255.f))<<16)|(((int)(GlobalFogGreen*255.f))<<8)|(((int)(GlobalFogBlue*255.f)));
	// player madness here
	if (player != -1) {
		SetWindow (camera, WINDOW_TL + player);
	}
	CameraSet (camera, 0);
}

// Destroys a window
Window::~Window ()
{
	ASSERT_VALID(this);
	// If we're pending a new screen, it may not be chained on to the end
	// of curScreen, so delete it first.  It will NULL its link in curScreen
	// Degenerate case: Where Proceed() has been called this frame, nextScreen == firstScreen
	if (nextScreen && nextScreen != firstScreen) {
		delete nextScreen;
	}
	curScreen->Finalise();
	delete firstScreen;
	// Kill of our distraction
	if (curDistraction) {
		delete curDistraction;
	}
}

// Runs a frame in a window
void Window::Run()
{
	ASSERT_VALID(this);
	float uMin, uMax, vMin, vMax, realLerp;
	float top, left, bottom, right;
	float xScale, yScale;
	Uint32 padAll, bgCol;
	bool inGame;

	inGame = (GetGameState() == GS_GAME);
	if (!inGame) {
		CameraSet (camera, 0);
		if (player >= 2) {
			float f[4];
			psGetWindow (f);
			psSetWindow (f[0]-64.f, f[1]-8.f, f[2]-8.f, f[3]-64.f);
		}
	}

	xScale = scaleTab[scaleIdx][0];
	yScale = scaleTab[scaleIdx][1];

	// Potentially create another distraction if bored long enough
	if (curDistraction == NULL &&
		nFramesToDist-- <= 0 && !inGame)
		curDistraction = WindowDistraction::MakeRandom();

	if (curDistraction) {
		if (curDistraction->Update()) {
			delete curDistraction;
			curDistraction = NULL;
			nFramesToDist = 150 + (rand() & 0x7f);
		}
	}

	scaleIdx += scaleAdd;
	if (scaleIdx == 20) {
		// We've reached the smallest point, so swap screens now
		curScreen->Finalise();
		if (deleteIt)
			delete curScreen;
		else if (deletePadPanic)
			delete nextScreen->GetNext();
		deleteIt = deletePadPanic = false;
		nextScreen->Initialise();
		curScreen = nextScreen;
		nextScreen = NULL;
		curScreen->Setup (this, curScreen->GetPrev());
		if (curScreen->SquashOverride())
			scaleIdx = scaleAdd = 0;
	} else if (scaleIdx == 43) {
		// Finished
		scaleIdx = 0;
		scaleAdd = 0;
	}

	mUnit32 (psGetMatrix());
	mPostTranslate32 (psGetMatrix(), -PHYS_SCR_X/2, -PHYS_SCR_Y/2);
	mPostScale32 (psGetMatrix(), xScale, yScale);
	mPostTranslate32 (psGetMatrix(), PHYS_SCR_X/2, PHYS_SCR_Y/2);
	if (player >= 0)
		mPostScale32 (psGetMatrix(), 0.65f, 0.65f);

	// Are we logging on?
	if (status == POWERING_UP) {
		status = READY;
		lerp = 0;
		if (player == -1) {
			if (inGame) {
				scaleIdx = 43;
				scaleAdd = 0;
			} else {
				scaleIdx = 20;
				scaleAdd = 1;
			}
		}
		return;
	} else {
		status = READY;
	}


	if (MEM_CHECK) {
		Uint32 whole,biggest;
		syMallocStat (&whole, &biggest);
		psDrawString (psFormat ("%.2fK", whole/1024.f, biggest/1024.f), 0, 0);
	}

	// Check for a pad panic in multiplayer
	if (player >= 0) {
		if (playerState[player] != NOT_PLAYING && FindPhysicalController (player) == NULL) {
			PadPanic[player]++;
		} else
			PadPanic[player] = 0;
	}

	if (ShowVersion) {
		Matrix32 m = *psGetMatrix();
		mPostScale32 (psGetMatrix(), 0.75f, 0.75f);
		psSetColour (0xffffff);
		psSetPriority(10e7);
		psSetAlpha (1.f);
		psDrawString (REDDOG_VERSION, 0, 0);
		psSetMatrix(&m);
	}

	// Draw screen
	if (player >= 0)
		psSetMultiplier (0.65f);
	if (player >=0 && PadPanic[player]) {
		if (PadPanic[player] < PAD_TIMEOUT) {
			psSetColour (COL_TEXT);
			#if USRELEASE
				psDrawCentred ("`USSCANNING", 128);
			#else
				psDrawCentred ("`SCANNING", 128);
			#endif
		} else {
			// Rewind back to the initial state
			if (scaleAdd == 0) {
				// Are we already on the first screen?
				if (curScreen == firstScreen) {
					psSetColour (COL_TEXT);
					#if USRELEASE
						psDrawCentred ("`USSCANNING", 128);
					#else
						psDrawCentred ("`SCANNING", 128);
					#endif
				} else {
					scaleIdx = 0;
					scaleAdd = 1;
					deleteIt = false;
					deletePadPanic = true;
					nextScreen = firstScreen;
				}
			}
		}
	} else
		curScreen->Draw();
	psSetMultiplier (1.f);

	// Put the background picture underneath
	WindowPos pos = curPos;
	if (!(curPos == targetPos)) {
		float palNum = PAL?5:6;
		float realLerp = (1.f/palNum) * lerp;
		switch (motion) {
		case X_MOVE:
			pos.u = LERP (curPos.u, targetPos.u, realLerp);
			break;
		case X_PAUSE:
			if (lerp == 0)
				flashAmt = 0;
			pos.u = targetPos.u;
			break;
		case Y_MOVE:
			pos.u = targetPos.u;
			pos.v = LERP (curPos.v, targetPos.v, realLerp);
			break;
		case Y_PAUSE:
			if (lerp == 0)
				flashAmt = 0;
			pos.u = targetPos.u;
			pos.v = targetPos.v;
			break;
		case ZOOM:
			pos.u = targetPos.u;
			pos.v = targetPos.v;
			pos.zoom = LERP (curPos.zoom, targetPos.zoom, realLerp);
			break;
		case ZOOM_3:
			pos.zoom = LERP (curPos.zoom, targetPos.zoom, realLerp);
			break;
		case Y_PAUSE_3:
			if (lerp == 0)
				flashAmt = 0;
			pos.zoom = targetPos.zoom;
			break;
		case Y_MOVE_3:
			pos.zoom = targetPos.zoom;
			pos.v = LERP (curPos.v, targetPos.v, realLerp);
			break;
		case X_PAUSE_3:
			if (lerp == 0)
				flashAmt = 0;
			pos.zoom = targetPos.zoom;
			pos.v = targetPos.v;
			break;
		case X_MOVE_3:
			pos.zoom = targetPos.zoom;
			pos.v = targetPos.v;
			pos.u = LERP (curPos.u, targetPos.u, realLerp);
			break;
		default:
			curPos = targetPos;
			pos = targetPos;
			break;
		}
		if (motion == X_PAUSE ||
			motion == Y_PAUSE)
			lerp += 2;
		else
			lerp += 2;
		if (lerp >= int(palNum)) {
			motion = MotionType(int(motion) + 1);
			lerp = 0;
		}
		if (curPos == targetPos)
			flashAmt = 0;
	} else {
		lerp = 0;
		if (curPos.zoom == 4)
			motion = ZOOM_3;
		else
			motion = X_MOVE;
	}

	if (curScreen->BackgroundOverride() == NULL) {
		float wid, hei;
		wid = (PHYS_SCR_X*(1.f/2048.f)) / (pos.zoom);
		hei = (PHYS_SCR_Y*(1.f/2048.f)) / (pos.zoom);
		
		uMin = pos.u*(1.f/1024.f) - wid;
		vMin = pos.v*(1.f/1024.f) - hei;
		uMax = pos.u*(1.f/1024.f) + wid;
		vMax = pos.v*(1.f/1024.f) + hei;

		// Do our little widgets on S.P.
		if (player == -1 && curDistraction &&
			scaleAdd == 0) {
			Matrix32 m = *psGetMatrix();
			SoundOverride = true;
			curDistraction->Draw();
			SoundOverride = false;
			psSetMatrix(&m);
		}
	} else {
		UVSet *set = curScreen->BackgroundOverride();
		uMin = set->uMin;
		uMax = set->uMax;
		vMin = set->vMin;
		vMax = set->vMax;
		/*
		if (OverrideStatic) {
			uMin = vMin = 0;
			uMax = 1.f;
			vMax = 1.f;
		} else {
			uMin = vMin = 0;
			uMax = 640.f/1024.f;
			vMax = 480.f/1024.f;
		}
		*/
	}

	if (flashAmt < 16)
		flashAmt++;

	Uint32 lerpedColour;
	if (nextScreen) {
		lerpedColour = ColLerp(curScreen->BackgroundColour(), nextScreen->BackgroundColour(), 
			RANGE(0, scaleIdx / 20.f, 1));
	} else {
		lerpedColour = curScreen->BackgroundColour();
	}

	if (flashAmt >= 16 || lerpedColour == 0)
		bgCol = 0xff000000 | lerpedColour;
	else
		bgCol = 0xff000000 | ColLerp (lerpedColour, 0xffffff, flashTab[flashAmt]);

	if (player >= 0)
		bgCol = ColLerp (bgCol, 0, 0.75f);

	// Calculate the bg picture position
	if (player < 2) {
		top		= currentCamera->screenMidY - yScale*currentCamera->screenY*0.5f + rGlobalYAdjust;
		bottom	= currentCamera->screenMidY + yScale*currentCamera->screenY*0.5f + rGlobalYAdjust;
	} else {
		top		= currentCamera->screenMidY - yScale*currentCamera->screenY*0.5f;
		bottom	= currentCamera->screenMidY + yScale*currentCamera->screenY*0.5f;
	}
	left	= currentCamera->screenMidX - xScale*currentCamera->screenX*0.5f;
	right	= currentCamera->screenMidX + xScale*currentCamera->screenX*0.5f;

	if (!inGame) {
		rSetMaterial (curScreen->Background());
		kmxxGetCurrentPtr (&vertBuf);
		kmxxStartVertexStrip (&vertBuf);
		kmxxSetVertex_3 (0xe0000000, left, top, 0.1f, uMin, vMin, bgCol, 0);
		kmxxSetVertex_3 (0xe0000000, left, bottom, 0.1f, uMin, vMax, bgCol, 0);
		kmxxSetVertex_3 (0xe0000000, right, top, 0.1f, uMax, vMin, bgCol, 0);
		kmxxSetVertex_3 (0xf0000000, right, bottom, 0.1f, uMax, vMax, bgCol, 0);
		kmxxReleaseCurrentPtr (&vertBuf);
		
		// Apply the static over the top
		if (!OverrideStatic) {
			feNoise.info.TSPPARAMBUFFER = (feNoise.info.TSPPARAMBUFFER & 0x03ffffff) | 0x40000000;
			rSetMaterial (&feNoise);
			kmxxGetCurrentPtr (&vertBuf);
			kmxxStartVertexStrip (&vertBuf);
			float wrapAmt = (player==-1) ? 128 : 64;
			kmxxSetVertex_3 (0xe0000000, 0, 0, 1e7, 0, 0, 0xffffffff, 0);
			kmxxSetVertex_3 (0xe0000000, 0, PHYS_SCR_Y, 1e7, 0, PHYS_SCR_Y/wrapAmt, 0xffffffff, 0);
			kmxxSetVertex_3 (0xe0000000, PHYS_SCR_X, 0, 1e7, PHYS_SCR_X/wrapAmt, 0, 0xffffffff, 0);
			kmxxSetVertex_3 (0xf0000000, PHYS_SCR_X, PHYS_SCR_Y, 1e7, PHYS_SCR_X/wrapAmt, PHYS_SCR_Y/wrapAmt, 0xffffffff, 0);
			kmxxReleaseCurrentPtr (&vertBuf);
		}
	}
	// Then process input, in case of updates
	if (scaleAdd == 0) {
		if (!theWindowSequence->IsSinglePlayer() && player == -1) {
			if (GetGameState() == GS_GAME) {
				if (PausedBy == -1) {
					padAll = GetPadAll();
				} else {
					padAll = GetPadFE(LogToPhys(PausedBy));
				}
			} else
				padAll = GetPadAll();
			LookForCheats(padAll);
		} else {
			if (theWindowSequence->IsSinglePlayer())
				padAll = GetPadFE(-1);
			else
				padAll = GetPadFE(player);
		}
		if (PadReset)
			padAll = 0;
		if (curScreen == NULL)
			CrashOut ("No curScreen in Menu::Run", 0xffff00, 0x00ffff);
		curScreen->ProcessInput (padAll);
		if (curScreen == NULL)
			CrashOut ("No curScreen in Menu::Run", 0xffff00, 0x00ffff);
		if (inGame)
			curScreen->ProcessInput(0); // Twice as fast don't you know
	}
	DoGlobalTexAnims();
}

// Change to a new screen
Screen *Window::Change(ScreenID &screen, Screen *prev)
{
	ASSERT_VALID(this);
	nextScreen = screen.Create(this, prev);
	if (prev && nextScreen->HasShutdown()) {
		if (curScreen->SquashOverride()) {
			scaleIdx = 19;
			scaleAdd = 1;
		} else {
			scaleIdx = 0;
			scaleAdd = 1;
		}
	} else {
		curScreen->Finalise();
		nextScreen->Initialise();
		curScreen = nextScreen;

		// Push window position and move on
		posStack[stackPtr++] = targetPos;
		MoveBacking();
	}

	deleteIt = deletePadPanic = false;
	return nextScreen;
}

void Window::Back()
{
	ASSERT_VALID(this);
	if (curScreen->GetPrev() == NULL)
		return;
	// Are we currently transitioning?
	if (scaleAdd)
		return;
	if (curScreen->HasShutdown()) {
		nextScreen = curScreen->GetPrev();
		if (curScreen->SquashOverride())
			scaleIdx = 19;
		else
			scaleIdx = 0;
		scaleAdd = 1;
		deleteIt = true;
	} else {
		nextScreen = curScreen->GetPrev();
		curScreen->Finalise();
		delete curScreen;
		deleteIt = false;
		curScreen = nextScreen;
		nextScreen = NULL;
		dAssert (curScreen, "Illegal call to back()");
		curScreen->Setup (this, curScreen->GetPrev());
		curScreen->Initialise();
		dAssert (stackPtr > 0, "Illegal call to back()");;
		targetPos = posStack[--stackPtr];
	}
}

// Moves to a new area of backing
void Window::MoveBacking(void)
{
	do {
		targetPos = validPositions[rand() % nPositions];
	} while (curPos == targetPos || curPos.zoom == targetPos.zoom);
}

// Profile handling
void Window::LockProfile (int vmu, int subProfile)
{
	// Selected from single player?
	dPrintf ("Locking profile for player %d [%d:%d]", player, vmu, subProfile);
	if (BupIsProfileLocked (vmu, subProfile))
		dPrintf ("  * This profile is already locked");
	if (player == -1) {
		BupLockProfile (vmu, subProfile);
		GameVMS = vmu;
		GameProfile = subProfile;
		BupGetProfile (vmu, subProfile, &CurProfile[0]);
	} else {
		BupLockProfile (vmu, subProfile);
		PlayerVMU[player] = vmu;
		PlayerProfile[player] = subProfile;
		BupGetProfile (vmu, subProfile, &CurProfile[player]);
	}
}

void Window::UnlockProfile ()
{
	dAssert (player >= 0, "Not a player window!");
	if (PlayerVMU[player] >= 0 && PlayerProfile[player] >= 0) {
		dPrintf ("Unlocking profile for player %d [%d:%d]", player, PlayerVMU[player], PlayerProfile[player]);
		WriteProfile();
		BupUnlockProfile (PlayerVMU[player], PlayerProfile[player]);
	}
	PlayerVMU[player] = -1;
	PlayerProfile[player] = -1;
}

void Window::UnlockAllProfiles()
{
	dPrintf ("Unlocking all profiles:");
	for (int player = 0; player < 4; ++player) {
		if (PlayerVMU[player]>=0 && PlayerProfile[player]>=0 &&
			BupIsProfileLocked(PlayerVMU[player], PlayerProfile[player])) {
			dPrintf ("  * player %d", player);
			WriteProfile (player);
			BupUnlockProfile (PlayerVMU[player], PlayerProfile[player]);
		}
		PlayerVMU[player] = PlayerProfile[player] = -1;
	}
}
void Window::WriteProfile ()
{
	if (player >= 0 && PlayerVMU[player] >= 0) {
		if (!BupIsProfileLocked(PlayerVMU[player], PlayerProfile[player]))
			BupLockProfile (PlayerVMU[player], PlayerProfile[player]);
		BupSetProfile (PlayerVMU[player], PlayerProfile[player], &CurProfile[player]);
	} else if (GameVMS >= 0 && GameProfile >= 0) {
		BupSetProfile (GameVMS, GameProfile, CurProfile);
	}
}

void Window::WriteProfile(int player)
{
	if (player >= 0 && PlayerVMU[player] >= 0) {
		BupSetProfile (PlayerVMU[player], PlayerProfile[player], &CurProfile[player]);
	} else if (GameVMS >= 0 && GameProfile >= 0) {
		BupSetProfile (GameVMS, GameProfile, CurProfile);
	}
}

void Window::RelockProfile (int player)
{
	if (player >= 0 && PlayerVMU[player] >= 0)
		BupLockProfile (PlayerVMU[player], PlayerProfile[player]);
}

VMU *Window::GetCurVMU()
{
	ASSERT_VALID(this);
	dAssert (player >= 0 && PlayerVMU[player] != -1, "Bad VMU");
	return theVMUManager->GetVMU(PlayerVMU[player]);
}
int Window::GetCurVMUNum()
{
	ASSERT_VALID(this);
	dAssert (player >= 0 && PlayerVMU[player] != -1, "Bad VMU");
	return PlayerVMU[player];
}
int Window::GetCurProfileNum()
{
	ASSERT_VALID(this);
	dAssert (player >= 0 && PlayerProfile[player] != -1, "Bad VMU");
	return PlayerProfile[player];
}

GameSave *Window::GetCurProfile()
{
	ASSERT_VALID(this);
	dAssert (player >= 0 && PlayerVMU[player] != -1, "Bad VMU");
	return &CurProfile[player];
}

void Window::SetTargetPos (const WindowPos &p)
{
	ASSERT_VALID(this);
	targetPos = p;
}


/////////////////////////////////////////////////////////
// Screen Factory class
/////////////////////////////////////////////////////////
Screen	*ScreenID::Create (Window *parent, Screen *prev)
{
	Screen *retVal = NULL;

	switch (type) {
	case MENU:
		retVal = new Menu (u.menu);
		break;
	case BY_EXAMPLE:
		retVal = u.example();
		break;
	case BY_EXAMPLE_WITH_PARM:
		retVal = u.exampleParm(parm);
		break;
	default:
		dAssert (FALSE, "Bad ID");
		break;
	}

	if (retVal) {
		retVal->Setup (parent, prev);
	}

	return retVal;
}

// Default background material
Material *Screen::Background()
{
	return &bgPicture;
}

// Destruction!
Screen::~Screen()
{ 
	if (prevScreen)
		prevScreen->nextScreen = NULL;
	if (nextScreen) delete nextScreen; 
}

bool Screen::DoesColourClash(int colour) const
{
	int player = GetPlayer();
	for (int i = 0; i < 4; ++i) {
		if (i == player)
			continue;
		if (Window::GetPlayerState(i) != Window::NOT_PLAYING &&
			CurProfile[i].multiplayerStats.colour == colour)
			return true;
	}
	return false;
}

// Init/Final current screen
void Window::FinaliseScreen() 
{ 
	curScreen->Finalise(); 
	// Shut down the madness
	if (curDistraction) {
		delete curDistraction;
		curDistraction = NULL;
	}
}
void Window::InitialiseScreen() 
{ curScreen->Initialise(); }


// Replacement
void Screen::Replace(ScreenID &screenID)
{
	nextScreen = rootWindow->Change(screenID, GetPrev());
	if (GetPrev())
		GetPrev()->nextScreen = nextScreen;
	else
		rootWindow->SetFirst (nextScreen);
	nextScreen = NULL;
	Finalise();

	// Ugh!
	delete this;
}

/////////////////////////////////////////////////////////
// Menu - a selection of MenuEntries
/////////////////////////////////////////////////////////

// Constructor
Menu::Menu (MenuTemplate *templ)
{
	ASSERT_VALID(this);
	Screen::Screen();
	// Build the menu
	BuildMenu (templ);

	curEntry = 0; // Default is the top entry
	fadeAmount = 1.f; // Don't fade out at all
	nextScreenID = NULL; // Not going to another screen
	baseEntry = 0; // First page
}

// Destructor
Menu::~Menu ()
{
	ASSERT_VALID(this);
	FreeEntries();
}

// Free the menu structures
void Menu::FreeEntries()
{
	ASSERT_VALID(this);
	// Do we have any entries to free?
	if (pEntry == NULL)
		return;

	for (int i = 0; i < nEntries; ++i)
		delete pEntry[i];
	delete[] pEntry;

	pEntry = NULL;
}

// Build the menu structures
void Menu::BuildMenu(MenuTemplate *templ)
{
	ASSERT_VALID(this);
	int i;
	ScreenID id;

	nEntries = templ->nDescriptors;
	pEntry = new MenuEntry *[nEntries];

	for (i = 0; i < nEntries; ++i) {
		MenuEntry *newEnt;
		const MenuEntryTemplate *ent;

		ent = &templ->templ[i];

		newEnt = NULL;
		if (ent->cParms)
			id = ScreenID(ent->cParms);
		else
			id = ScreenID(ent->parms);

		if (ent->clss == NULL) {
			newEnt = new MenuEntry (ent->text, id);
		} else {
			newEnt = ent->clss(ent->text, id);
		}

		dAssert (newEnt != NULL, "Unknown menu entry class");

		pEntry[i] = newEnt;
		if (GetRoot())
			pEntry[i]->SetWindow (GetRoot());
	}
	// We may not have been fully initialised with our root menu, so this may need deferring:
	if (GetRoot()) {
		if (GetRoot()->GetPlayer() == -1)
			maxEntries = MIN (nEntries, 9);
		else
			maxEntries = MIN (nEntries, 6);
	}
}

void Menu::Setup (Window *w, Screen *prev)
{
	ASSERT_VALID(this);
	// Call our parent to get GetRoot() ready
	Screen::Setup (w, prev);

	// Set our root in all the entries
	for (int i = 0; i < nEntries; ++i)
		pEntry[i]->SetWindow (GetRoot());

	// Deferred from the constructor
	if (GetRoot()->GetPlayer() == -1)
		maxEntries = MIN (nEntries, 9);
	else
		maxEntries = MIN (nEntries, 6);
}

// Refresh a menu with a new template
void Menu::Refresh(MenuTemplate *templ, bool textValid /* = true */)
{
	ASSERT_VALID(this);
	// Take note of where we were before 
	// * of course, we may not be able to - durrrr
	char tempBuf[64];
	if (textValid)
		strcpy (tempBuf, pEntry[curEntry]->GetText());
	else
		tempBuf[0] = '\0';

	FreeEntries();
	BuildMenu (templ);

	// Try and find the same text, if possible
	int i;
	for (i = 0; i < nEntries; ++i) {
		if (!strcmp (tempBuf, pEntry[i]->GetText())) {
			curEntry = i;
			break;
		}
	}

	// Ensure menu limits are in place
	if (curEntry >= nEntries)
		curEntry = nEntries - 1;
	// Check to see if we need to flip scroll the menu
	if ((curEntry >= (baseEntry + maxEntries)) ||
		(curEntry < baseEntry)) {
		baseEntry = (curEntry / maxEntries) * maxEntries;
	}

	// Force a redraw
	timer = 0;
}

// Initialise a screen prior to drawing
void Menu::Initialise()
{
	ASSERT_VALID(this);
	if (!InitialDelayFlag)
		timer = -40.f;	// Just under a second for a continued menu
	else
		timer = -120.f;		// One and a half seconds of cursor blinking (ish)

	InitialDelayFlag = false;

	// Ensure menu is visible to start with
	fadeAmount = 1.f;
}

extern "C" {
	void FE_Sound(void)
	{
		sfxPlay (15, 0.9f, RandRange(-0.02f, 0.f));
	}
	void FE_MoveSound(void)
	{
		sfxPlay (GENERIC_MOVE_SOUND, 0.9f, 0);
	}
	void FE_ACK(void)
	{
		sfxPlay (GENERIC_ACCEPT_SOUND, 1.f, 0);
	}
	void FE_NACK(void)
	{
		sfxPlay (GENERIC_REJECT_SOUND, 1.f, 0);
	}
}
// Draws a cursor
void Menu::Cursor (float x, float y, bool sound /* = true */)
{
	psSetColour (CURS_COLOUR);
	psSetAlpha (1);
	psSetPriority (20000);
	psDrawString ("#", x , y);
	if (sound && !SoundOverride)
		FE_Sound();
}
void Menu::PendingCursor()
{
	if (currentFrame & 16)
		return;

	if (currentCamera->screenX < 600)
		Cursor(5.f,PHYS_SCR_Y - (128.f/0.65f), false);
	else
		Cursor(5.f,PHYS_SCR_Y - 128.f, false);
}
// Evil C glue code
extern "C" void DrawCursor (float x, float y) { Menu::Cursor (x, y); }

// Draws a screen 
void Menu::Draw() const
{
	if ((timer < 0)) {
		if (timer & 16) { 
			Cursor(0.f,0.f, false);
		}
	} else {
		int nToDraw;
		float adjustment = 0;
		Matrix32 mat;
		memcpy (&mat, psGetMatrix(), sizeof (mat));

		// Move down a shade
		mPreTranslate32 (psGetMatrix(), 30, 35);
		// Is there a title to draw?
		if (GetTitle()) {
			psSetColour (COL_TEXT);
			psSetAlpha(1);
			psSetPriority(0);
			psDrawCentredFE (GetTitle(), 0, timer - adjustment);
			adjustment += strlen (GetTitle());
			mPreTranslate32 (psGetMatrix(), 0, 50);
		}

		// Is an up tag needed?
		if (baseEntry > 0) {
			psSetColour (0xff0000);
			psSetAlpha(1);
			psSetPriority(0);
			psDrawString ("^", -25, -35);
		}

		nToDraw = MIN(nEntries - baseEntry, maxEntries);
		for (int j = 0; j < nToDraw; ++j) {
			int i = j + baseEntry;
			// Is this the currently selected entry?
			if (i == curEntry && (fadeAmount == 1.f || nextScreenID)) {
				// Draw this entry
				adjustment += pEntry[i]->Draw(1, 1-pow(1-fadeAmount, 2), timer - adjustment);
				psDrawString ("*", -25, 0);
			} else
				adjustment += pEntry[i]->Draw(0, fadeAmount, timer - adjustment);
			mPreTranslate32 (psGetMatrix(), 0, 35);
		}
		// Is a down tag needed?
		if (((nToDraw + baseEntry) < nEntries) &&
			((timer - adjustment) > 0)) {
			psSetColour (0xff0000);
			psSetAlpha(1);
			psSetPriority(0);
			psDrawString ("@", -25, 0);
		}

		memcpy (psGetMatrix(), &mat, sizeof (mat));
		// Deal with the extra lines
		mPreTranslate32 (psGetMatrix(), 30.f, (PHYS_SCR_Y - 128.f) - (35.f * NumExtra()));
		for (j = 0; j < NumExtra(); ++j) {
			adjustment += DrawExtra (j, fadeAmount, timer - adjustment);
			mPreTranslate32 (psGetMatrix(), 0, 35);
		}
		memcpy (psGetMatrix(), &mat, sizeof (mat));
		// Flash the cursor at the bottom
		if (timer > adjustment)
			PendingCursor();
	}
}
// Processes input
void Menu::ProcessInput(Uint32 press)
{
	// Move the drawing on
	timer+=2;

	if (fadeAmount != 1.f) {
		fadeAmount = fadeAmount - 0.10f;
		if (fadeAmount <= 0) {
			fadeAmount = 0;
			if (nextScreenID)
				Change (*nextScreenID);
			else
				Back();
		}
	} else {
		// First check to see if the cursor is in a valid position
		if (!pEntry[curEntry]->IsSelectable()) {
			int i;
			i = curEntry;
			do {
				i = (i + 1) % nEntries;
				if (pEntry[i]->IsSelectable())
					break;
			} while (i != curEntry);
			dAssert (i != curEntry, "No valid options");
			// Panic handler
			if (i == curEntry) {
				nextScreenID = NULL;
				fadeAmount = 0.99f;
				return;
			}
			curEntry = i;
		}
		// Check for motion
		if (press & PDD_DGT_KU) {
			do {
				if (curEntry != 0)
					curEntry--;
				else
					curEntry = nEntries-1;
			} while (!pEntry[curEntry]->IsSelectable());
			FE_MoveSound();
		} else if (press & PDD_DGT_KD) {
			do {
				curEntry++;
				if (curEntry == nEntries)
					curEntry = 0;
			} while (!pEntry[curEntry]->IsSelectable());
			FE_MoveSound();
		} else if (press & PDD_DGT_KL) {
			if (maxEntries != nEntries) {
				int offset;
				offset = curEntry - baseEntry;
				baseEntry -= maxEntries;
				if (baseEntry < 0) {
					baseEntry = (nEntries/maxEntries) * maxEntries;
				}
				curEntry = baseEntry + offset;
				if (curEntry >= nEntries)
					curEntry = nEntries - 1;
				// Check for selectable-ness
				while (!pEntry[curEntry]->IsSelectable()) {
					if (curEntry != 0)
						curEntry--;
					else
						curEntry = nEntries-1;
				}
				timer = 0;
				FE_MoveSound();
			} else
				pEntry[curEntry]->Left();
		} else if (press & PDD_DGT_KR) {
			if (maxEntries != nEntries) {
				int offset;
				offset = curEntry - baseEntry;
				baseEntry += maxEntries;
				if (baseEntry >= nEntries) {
					baseEntry = 0;
				}
				curEntry = baseEntry + offset;
				if (curEntry >= nEntries)
					curEntry = nEntries - 1;
				// Check for selectable-ness
				while (!pEntry[curEntry]->IsSelectable()) {
					curEntry++;
					if (curEntry == nEntries)
						curEntry = 0;
				}
				timer = 0;
				FE_MoveSound();
			} else
				pEntry[curEntry]->Right();
		} else if (press & PDD_OK) {
			// Dealt with elsewhere
			if (!pEntry[curEntry]->IsSelectable())
				return;
			if (!VetoSelection(curEntry)) {
				nextScreenID = pEntry[curEntry]->Selected();
				if (nextScreenID) {
					FE_ACK();
					fadeAmount = 0.99f;
				}
			}
		} else if (press & PDD_CANCEL) {
			if (GetPrev()) {
				FE_ACK();
				nextScreenID = NULL;
				fadeAmount = 0.99f;
			} else {
				FE_NACK();
			}
		}
		// Check to see if we need to flip scroll the menu
		if ((curEntry >= (baseEntry + maxEntries)) ||
			(curEntry < baseEntry)) {
			baseEntry = (curEntry / maxEntries) * maxEntries;
			timer = 0;
		}
	}
}

// Finalises a screen
void Menu::Finalise()
{
}

// Delete an entry
void Menu::DeleteEntry(int ent)
{
	delete pEntry[ent];
	if (curEntry > ent)
		curEntry--;
	if (ent != nEntries-1) {
		memmove (&pEntry[ent], &pEntry[ent+1], (nEntries - ent) * sizeof (MenuEntry*));
	}
	nEntries--;
	if (GetRoot()) {
		if (GetRoot()->GetPlayer() == -1)
			maxEntries = MIN (nEntries, 9);
		else
			maxEntries = MIN (nEntries, 6);
	}
}

/////////////////////////////////////////////////////////
// MenuEntry - a selectable item in a menu
/////////////////////////////////////////////////////////

// Draw the entry
float MenuEntry::Draw(float selAmt, float alpha, float nChars)
{
	int len = strlen(text);
	if (nChars < 0)
		return len;
	// Madness here
	psSetAlpha (alpha);
	psSetColour (IsSelectable() ? 
					ColLerp (COL_TEXT, COL_SELTEXT, selAmt ? ((currentFrame & 8) ? 1.f : 0.5f) : 0.f) :
					COL_UNSELECTABLE);
	psDrawStringFE (text, 0, 0, nChars);

	return len;
}

// Entry has been selected
ScreenID *MenuEntry::Selected()
{
	if (!subMenu.IsNull()) {
		return &subMenu;
	} else {
		return NULL;
	}
}

void Transform(Point2 *p, int nPts)
{
	float f[4];

	psGetWindow(f);

	for (int i = 0; i < nPts; ++i) {
		mPoint2Multiply (p+i, p+i, psGetMatrix());
		p[i].x += f[1];
		p[i].y += f[0];
	}
}


// Title page bridge
class RootMenu : public Menu
{
private:
	int ohNoYouCant;
	int disableOnTP;
public:
	RootMenu() : Menu(&newrootMenu) {}
	virtual void Initialise()
	{
		UnlockAllProfiles();
		Menu::Initialise();
		ohNoYouCant = 0;
		disableOnTP = 10;
		// Make sure all pads are responding
		for (int i = 0; i < 4; ++ i)
			PadMapTable[i] = i;
	}
	virtual void ProcessInput (Uint32 press)
	{
		if (ohNoYouCant)
			--ohNoYouCant;
		if (disableOnTP)
			--disableOnTP;
		else
			OnTitlePage = false;
		Menu::ProcessInput(press);
	}
	virtual void Draw() const 
	{
		Menu::Draw();
		if (ohNoYouCant) {
			psSetColour (COL_TEXT);
			if (ohNoYouCant < 10)
				psSetAlpha (ohNoYouCant * 0.1f * GetFadeAmount());
			else
				psSetAlpha (GetFadeAmount());
			psSetPriority (0);
			mUnit32 (psGetMatrix());
			psDrawWordWrap ("`CANTSTARTONTHATPAD", 48, PHYS_SCR_Y - 128 - 70, PHYS_SCR_X-(48*2));
		}
	}
	virtual bool HasShutdown() { return true; }
	virtual bool VetoSelection (int selection) {
#ifdef _DEBUG
		selection--;
#endif
		if (selection < 2) {
			// Make sure it's the first pad selecting
			if (pad[0] && (pad[0]->on & (PDD_OK))==0) {
				FE_NACK();
				ohNoYouCant = FRAMES_PER_SECOND * 4;
				return true;
			}
		}
		return false;
	}
};
static Screen *rootMenu() { return new RootMenu; }

// Title page handler
int DemoType = 0;

class TitlePage : public Screen
{
private:
	int timer, WaitTimer;
	UVSet set;
public:
	// Initialise a screen prior to drawing
	void Initialise()
	{
		set = UVSet(0, 0, 1, 1);
		timer = WaitTimer = 0;
		OverrideStatic = true;
		// Write back any changed profiles
		Window::UnlockAllProfiles();
		// After that ensure all others are unlocked
		UnlockAllProfiles();
		GameVMS = GameProfile = -1;
		for (int i = 0; i < 4; ++i) {
			CurHandicap[i] = 100;
			ResetGame (CurProfile + i);
			sprintf (CurProfile[i].name, "%s %d", LangLookup ("PLAYER"), i+1);
			CurProfile[i].multiplayerStats.colour = i;
		}
		InactiveFrames = 0;
		theWindowSequence->SetSinglePlayer(false);
	}
	// Draws a screen 
	void Draw() const
	{
		psSetAlpha(1);
		psSetPriority(0);
		psSetColour (0xffffff);
		if (currentFrame & 16) {
			psDrawTitle ("`HITSTART", PHYS_SCR_Y-48-64-24-16, 0x000000, 0xff000000);
		}
	}
	// Processes input
	void ProcessInput(Uint32 press)
	{
		if (press)
			WaitTimer = 0;
		else
			WaitTimer ++;
		if (PadReset) {
			theWindowSequence->Reset(false);
			sbExitSystem ();
			while (1);
		}
#ifndef _ARTIST
		if ((GinsuAutoDemo && timer > 8*50) || (WaitTimer > 30*FRAMES_PER_SECOND*2))
			press |= PDD_DGT_ST;
#endif
		if ((press & (PDD_DGT_ST|PDD_DGT_TX|PDD_DGT_TY|PDD_DGT_TA|PDD_DGT_TB)) == PDD_DGT_ST) {
#ifndef _ARTIST
			if (GinsuAutoDemo || (WaitTimer > 30*FRAMES_PER_SECOND*2)) {
				if (!ChangingState())
				{
					ReseedRandom();
					
					DemoOn = TRUE;
					DemoType = 1 - DemoType;
					switch (DemoType) {
					case 0:
						Multiplayer = 0;
						NumberOfPlayers = 1;
						LevelNum = 0; // volcano demo
						break;
					//case 2:
					//	Multiplayer = 0;
						//LevelNum = 3; // hydro demo
						//NumberOfPlayers = 1;
						//break;
					case 1:
						Multiplayer = 1;
						LevelNum = 0; // multiplayer demo
						NumberOfPlayers = 4;
						break;
					}
					FadeToNewState (FADE_TO_BLACK, GS_GAME);
				}
			} else {
#endif

				FE_ACK();
				Replace (ScreenID (rootMenu));
#ifndef _ARTIST
			}
#endif
		}
		timer += 2;
	}
	// Finalises a screen
	void Finalise()
	{
		OverrideStatic = false;
	}
	virtual bool HasShutdown() { return true; }
	virtual Material *Background() { return &titlePageMtl; }
	virtual Uint32 BackgroundColour() { return 0xffffff; }
	virtual UVSet *BackgroundOverride() { return &set; }
};
Screen *newtitlePage() { return new TitlePage; }

/////////////////////////////////////////////////////////
// Window sequence handler
/////////////////////////////////////////////////////////
WindowSequence *theWindowSequence;

WindowSequence::WindowSequence()
{
	ChangeSemaphore = singlePlayer = false;
	NeedsReset = false;
	setPtr = 0;
}

// Proceed to the next window set
void WindowSequence::Proceed (ScreenID *list, int nWindows)
{
	ChangeSemaphore = true;
	setStack[setPtr].nWindows = nWindows;
	setStack[setPtr].window = new Window *[nWindows];
	for (int i = 0; i < nWindows; ++i) {
		if (!list[i].IsNull())
			setStack[setPtr].window[i] = new Window ((nWindows == 1) ? -1 : i, list[i]);
		else
			setStack[setPtr].window[i] = NULL;
	}

	setPtr++;
}

// Return to the previous set
void WindowSequence::Back()
{
	dAssert (setPtr != 0, "Illegal call of Back()");
	ChangeSemaphore = true;
	setPtr--;
	for (int i = 0; i < setStack[setPtr].nWindows; ++i) {
		if (setStack[setPtr].window[i]) {// Don't trust the compiler to do this...
			delete setStack[setPtr].window[i];
		}
	}
	delete setStack[setPtr].window;
	if (setPtr) {
		Reinitialise();
	}
}

// Reset the whole she-bang - just notes it to do as the Run() thread exits
void WindowSequence::Reset(bool restart)
{
	ChangeSemaphore = true;
	NeedsReset = true;
	// If we're not going to restart, do the doo now, and let everything finish
	if (!restart)
		_Reset(false);
}
void WindowSequence::_Reset(bool restart)
{
	singlePlayer = false;
	Shutdown();
	while (setPtr)
		Back();
	if (restart) {
		MemStats ("After reset");
		ScreenID init[] = { ScreenID(newtitlePage) };
		OnTitlePage = true;
		theWindowSequence->Proceed (init, 1);
	}
	NeedsReset = false;
}

void WindowSequence::Shutdown()
{
	if (setPtr) {
		ChangeSemaphore = true;
		for (int i = 0; i < setStack[setPtr-1].nWindows; ++i) {
			if (setStack[setPtr-1].window[i])
				setStack[setPtr-1].window[i]->FinaliseScreen();
		}
	}
}

void WindowSequence::Reinitialise()
{
	if (setPtr) {
		ChangeSemaphore = true;
		for (int i = 0; i < setStack[setPtr-1].nWindows; ++i) {
			if (setStack[setPtr-1].window[i])
				setStack[setPtr-1].window[i]->InitialiseScreen();
		}
	}
}

void WindowSequence::Run()
{
	static bool DebouncePadReset = FALSE;

	// No work to do?  Resetting
	if (setPtr) {
			// Lock the internal semaphore
		// If there was a change, break out as the finalisation has been called
		// for each of the remaining windows, or the window information is out
		// of date
		ChangeSemaphore = false;
		for (int i = 0; i < setStack[setPtr-1].nWindows; ++i) {
			if (setStack[setPtr-1].window[i])
				setStack[setPtr-1].window[i]->Run();
			if (ChangeSemaphore)
				break;
		}
		
		// Check for resets after the window has run
		if (PadReset && !DebouncePadReset) {
			DebouncePadReset = true;
			PadReset = false;
			// Back to title page if within frontend
			if (!OnTitlePage) {
				Reset(true);
			} else {
				// Otherwise that's it folks
				Reset(false);
				sbExitSystem();
				while (1);
			}
		} else if (!PadReset)
			DebouncePadReset = false;
	}
	// Check to see if the whole system needs resetting
	if (NeedsReset)
		_Reset(true);
	dAssert (!NeedsReset, "erm");
}

// Recreate the cameras after a switchout
void WindowSequence::Recreate()
{
	for (int j = 0; j < setPtr; ++j) {
		for (int i = 0; i < setStack[j].nWindows; ++i) {
			if (setStack[j].window[i])
				setStack[j].window[i]->CreateCamera();
		}
	}
}

// The one and only sequence handler
extern WindowSequence *theWindowSequence;

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
// C linkage layer
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

extern "C" void InitCPlusPlus()
{
	// Create singletons
	theVMUManager = new VMUManager;
	theWindowSequence = new WindowSequence;
}

#define MAT { {0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA | MF_CLAMP_V | MF_CLAMP_U }
static struct {
	Material m;
	Uint32 GBIX;
} MPMaps[NUM_MP_LEVELS+1] =
{
	MAT,
#include "P:\game\texmaps\frontend\mpmaps\acidbath.h"
 },
	MAT,
#include "P:\game\texmaps\frontend\mpmaps\blackice.h"
 },
	MAT,
#include "P:\game\texmaps\frontend\mpmaps\dod.h"
 },
	MAT,
#include "P:\game\texmaps\frontend\mpmaps\dod(expert).h"
 },
	MAT,
#include "P:\game\texmaps\frontend\mpmaps\dune.h"
 },
	MAT,
#include "P:\game\texmaps\frontend\mpmaps\ganymede.h"
 },
	MAT,
#include "P:\game\texmaps\frontend\mpmaps\helterskelter.h" 
},
	MAT,
#include "P:\game\texmaps\frontend\mpmaps\industrialZone.h" 
},
	MAT,
#include "P:\game\texmaps\frontend\mpmaps\medieval.h" 
},
	MAT,
#include "P:\game\texmaps\frontend\mpmaps\moltern.h" 
},
	MAT,
#include "P:\game\texmaps\frontend\mpmaps\ruined.h" 
},
	MAT,
#include "P:\game\texmaps\frontend\mpmaps\spacestation.h" 
},
	MAT,
#include "P:\game\texmaps\frontend\mpmaps\stormdrain.h" 
},
	MAT,
#include "P:\game\texmaps\frontend\mpmaps\circus.h"
 }, // THE CIRCUS
	MAT,
#include "P:\game\texmaps\frontend\mpmaps\blokfort.h"
 }, // VIRTUAM
	MAT,
#include "P:\game\texmaps\frontend\mpmaps\wasted.h" 
},
	MAT,
#include "P:\game\texmaps\frontend\mpmaps\random.h" 
}
};
#undef MAT
Material *GetLevelMat(int i) { return &MPMaps[i].m; }

Material weapMat[10] =
{
	{ 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA | MF_CLAMP_V | MF_CLAMP_U },
	{ 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA | MF_CLAMP_V | MF_CLAMP_U },
	{ 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA | MF_CLAMP_V | MF_CLAMP_U },
	{ 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA | MF_CLAMP_V | MF_CLAMP_U },
	{ 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA | MF_CLAMP_V | MF_CLAMP_U },
	{ 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA | MF_CLAMP_V | MF_CLAMP_U },
	{ 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA | MF_CLAMP_V | MF_CLAMP_U },
	{ 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA | MF_CLAMP_V | MF_CLAMP_U },
	{ 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA | MF_CLAMP_V | MF_CLAMP_U },
	{ 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA | MF_CLAMP_V | MF_CLAMP_U },
};
Material bigXMat = { 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA | MF_CLAMP_V | MF_CLAMP_U };

static struct {
	Material		*mat;
	KMVERTEXCONTEXT	*bC;
	Uint32			GBIX;
} FEFixUpList[] = {
	{ &feNoise, &sfxContext,
#include "P:\game\texmaps\FRONTEND\noise0001.h"
	},
	{ &testCard, &sfxContext,
#include "P:\game\texmaps\FRONTEND\testcard1.h"
	},
	{ &titlePageMtl, &sfxContext,
#include "p:\game\texmaps\frontend\titlepage.h"
	},
	{ &vmuMaterial, &sfxContext,
#include "p:\game\texmaps\frontend\vms.h"
	},
	{ &weapMat[0], &sfxContext,
#include "p:\game\texmaps\frontend\iconsprites\bomb.h"
	},
	{ &weapMat[1], &sfxContext,
#include "p:\game\texmaps\frontend\iconsprites\electro.h"
	},
	{ &weapMat[2], &sfxContext,
#include "p:\game\texmaps\frontend\iconsprites\health.h"
	},
	{ &weapMat[3], &sfxContext,
#include "p:\game\texmaps\frontend\iconsprites\homing.h"
	},
	{ &weapMat[4], &sfxContext,
#include "p:\game\texmaps\frontend\iconsprites\lockon.h"
	},
	{ &weapMat[5], &sfxContext,
#include "p:\game\texmaps\frontend\iconsprites\mine.h"
	},
	{ &weapMat[6], &sfxContext,
#include "p:\game\texmaps\frontend\iconsprites\stealth.h"
	},
	{ &weapMat[7], &sfxContext,
#include "p:\game\texmaps\frontend\iconsprites\stormer.h"
	},
	{ &weapMat[8], &sfxContext,
#include "p:\game\texmaps\frontend\iconsprites\ultra.h"
	},
	{ &weapMat[9], &sfxContext,
#include "p:\game\texmaps\frontend\iconsprites\vulcan.h"
	},
	{ &bigXMat, &sfxContext,
#include "p:\game\texmaps\frontend\iconsprites\bigx.h"
	},
	{ &intelligence, &sfxContext,
#include "p:\game\texmaps\frontend\mpmaps\intelligence.h"
	},
	{ &tank, &sfxContext,
#include "p:\game\texmaps\frontend\tank.h"
	},
	{ &tankInside, &sfxContext,
#include "p:\game\texmaps\frontend\tankinside.h"
	},
	{ &padMat, &sfxContext,
#include "p:\game\texmaps\frontend\pad.h"
	},
	// The level zoomeries LEVEL DEPENDENCY
	{ &zoomMat[0], &sfxContext,
#include "p:\game\texmaps\frontend\volcano\volcano01.h"
	},
	{ &zoomMat[1], &sfxContext,
#include "p:\game\texmaps\frontend\ice\ice00.h"
	},
	{ &zoomMat[2], &sfxContext,
#include "p:\game\texmaps\frontend\canyon\canyon00.h"
	},
	{ &zoomMat[3], &sfxContext,
#include "p:\game\texmaps\frontend\hydro\hydro01.h"
	},
	{ &zoomMat[4], &sfxContext,
#include "p:\game\texmaps\frontend\city\city01.h"
	},
	{ &zoomMat[5], &sfxContext,
#include "p:\game\texmaps\frontend\alienbase\alienbase0001.h"
	},
	{ &zoomMat[LEV_VEHICLE_MANOEUVRING], &sfxContext,
#include "p:\game\texmaps\frontend\challenges\1\one1.h"
	},
	{ &zoomMat[LEV_SEARCH_AND_COLLECT], &sfxContext,
#include "p:\game\texmaps\frontend\challenges\2\two1.h"
	},
	{ &zoomMat[LEV_FOOTSOLDIER_RAMPAGE], &sfxContext,
#include "p:\game\texmaps\frontend\challenges\3\three1.h"
	},
	{ &zoomMat[LEV_SHOOTING_RANGE], &sfxContext,
#include "p:\game\texmaps\frontend\challenges\4\four1.h"
	},
	{ &zoomMat[LEV_HEAVY_DUTY_RAMPAGE], &sfxContext,
#include "p:\game\texmaps\frontend\challenges\5\five1.h"
	},
	{ &zoomMat[LEV_DEFENSIVE], &sfxContext,
#include "p:\game\texmaps\frontend\challenges\6\six1.h"
	},
	{ &zoomMat[LEV_ADV_VEHICLE_MANOEUVRING], &sfxContext,
#include "p:\game\texmaps\frontend\challenges\7\seven00.h"
	},
	{ &techMat, &sfxContext,
#include "p:\game\texmaps\frontend\RedDogTech.h"
	},
	{ &segaMat, &sfxContext,
#include "p:\game\texmaps\FRONTEND\SEGALOGO.h"
	},
	{ &argoMat, &sfxContext,
#include "p:\game\texmaps\FRONTEND\ARGOLOGO.h"
	},
};


void ShutdownFrontEnd()
{
	suppressDebug = FALSE;
	// Reset the renderer
	kmSetWaitVsyncCount (2);
	rReset();
	SHeapClear (GameHeap);
	// Reinitialise input
	InitInput();

	// Turn translucent autosort back on
	kmSetAutoSortMode (TRUE);

	// Cease thy music, cretin
	SoundStopTrack();
}

void InitialiseFrontEnd()
{
	Stream *s;
	int i, nToRead, theTex;

	// Make sure we're not in multiplayer mode still
	kmSetUserClipLevelAdjust (KM_LEVEL_ADJUST_NORMAL, &rGlobalYAdjust);
	
	// Run at 60Hz for the title page cos we can
	kmSetWaitVsyncCount (0);
	
	// Suppress debug text etc
	suppressDebug = TRUE;
	NoLoadingScreen = TRUE;
	
	// Set the whiteout back to normal
	SetWhiteOut (0,0,0);
	
	// Reseed the random number generator
	ReseedRandom();
	
	// Start the loading procedure
	BeginLoading();

	// Make sure that all sounds are inaudible
	SoundFade(1.f, 1);
	
	// Load the base title page textures
	s = sOpen ("TITLE.TXP");
	sSetBuf (s, GameHeap->end - 2048*1024, 2048*1024);
	texSetDMABuffer (GameHeap->end - 2048*1024 - 1500*1024, 1500*1024); 
	texInit();
	texLoad(s); 
	sClose (s);

	// Load in the models for the title page
	s = sOpen ("TITLE.LVL");
	sSetBuf (s, GameHeap->end - 2048*1024, 2048*1024);
	ArgoLogo = PNodeLoad (NULL, s, (Allocator *)SHeapAlloc, MissionHeap);
	World = PNodeLoad (NULL, s, (Allocator *)SHeapAlloc, MissionHeap);
	sClose (s);
	
	// Load a single, random backdrop picture
	s = sOpen ("BACKDROP.TXP");
	sSetBuf (s, GameHeap->end - 2048*1024, 2048*1024);
	texSetDMABuffer (GameHeap->end - 2048*1024 - 1500*1024, 1500*1024);

	sRead (&nToRead, 4, s);
	sSkip (s);
	// Choose a random texture
	theTex = rand() % nToRead;
	for (i = 0; i < nToRead; ++i) {
		if (i == theTex) {
			bgPicture.surf.GBIX = texLoadSingleTexture(s);
			break;
		} else {
			// Skip this texture
			struct {
				Uint32 tag, tagSize;
			} tag;
			int foundPVRT = 0;
			
			do {
				sRead (&tag, sizeof (tag), s);
				switch (tag.tag) {
				default:
					sIgnore (tag.tagSize, s);
					break;
				case 0x54525650:	/* PVRT */
					sIgnore (tag.tagSize, s);
					foundPVRT++;
					break;
				}
				
			} while (foundPVRT==0);
			
		}
		sSkip (s);
	}
	sClose (s);
	
	texSetDMABuffer (NULL, 0);
	
	EndLoading();
	
	// Ensure all profiles are readable
	Window::UnlockAllProfiles();
	UnlockAllProfiles();

	for (i = 0; i < asize(FEFixUpList); ++i) {
		FEFixUpList[i].mat->surf.GBIX = FEFixUpList[i].GBIX;
		rFixUpMaterial (FEFixUpList[i].mat, FEFixUpList[i].bC);
	}
	for (i = 0; i < (NUM_MP_LEVELS+1); ++i) {
		MPMaps[i].m.surf.GBIX = MPMaps[i].GBIX;
		rFixUpMaterial (&MPMaps[i].m, &sfxContext);
	}
	
	rFixUpMaterial (&bgPicture, &sfxContext);
	
	NoLoadingScreen = FALSE;
	
	// Set up the initial delay flag
	Menu::SetInitialFlag();
	
	// Play some music warez
	if (NoLoadingScreen == FALSE && ShownLogo) {
		SoundFade(0.f, 0);
		SoundPlayTrack(0);
	}
	// Make sure all pads are responding
	for (i = 0; i < 4; ++ i)
		PadMapTable[i] = i;
}

// Memstats
#ifndef _RELEASE
static void MemStats(char *s)
{	
	Uint32 whole,biggest;
	syMallocStat (&whole, &biggest);
	dPrintf ("%s:%.2fK", s, whole/1024.f);
}
#endif

extern "C" void testHandler (GameState thisState, GameStateReason reason)
{
	int i;
	extern float TargetGlobalFogRed, TargetGlobalFogBlue, TargetGlobalFogGreen;
	extern float GlobalFogRed, GlobalFogBlue, GlobalFogGreen;

	switch (reason) {
	case GSR_INITIALISE:
		// Totally reset this
		rShutdownRenderer();
		rInitRenderer(TRUE, FE_TEXRAM); // Three meg

		// Have we just come from the credits?  If so patch to alien base
		if (LevelNum == 13) {
			LevelNum = 5; // Make it appear as if we came from the alien base
			LevelComplete = true;
		}

		TargetGlobalFogRed = TargetGlobalFogGreen = TargetGlobalFogBlue = 0.f;
		GlobalFogRed = GlobalFogGreen = GlobalFogBlue = 0.f;

		InitialiseFrontEnd();
		theVMUManager->Poll();

		// Reset the custom tournament
		CustomTournament.nGames = MIN_TOURNAMENT_GAMES;
		CustomTournament.tType = CUSTOM;
		ResetGameParms (&CustomTournament.game[0].parms);
		ResetGameParms (&CustomTournament.game[1].parms);
		ResetGameParms (&CustomTournament.game[2].parms);
		CustomTournament.game[0].gameType = TAG;
		CustomTournament.game[0].map = 2; // tag on DOD
		CustomTournament.game[1].gameType = KNOCKOUT;
		CustomTournament.game[0].map = 4; // ko on dune
		CustomTournament.game[2].gameType = DEATHMATCH;
		CustomTournament.game[0].map = 15; // dm on wasted
		
		// Set up the map name
		MPMapName[numMPMaps] = "Random";
		MPLevOK[numMPMaps] = 0xffffffff;
		
		// If we exited via the medium of pad reset, guidelines state
		// that we have to sit on the title page. Grrr :)
		if (ExitedThroughReset == -1) {
			MemStats ("Resetting on entry due to pad reset");
			FrontEndStillInitialised = false;
			ExitedThroughReset = 1;
		}

		// Are we still initialised from before??
		if (!FrontEndStillInitialised) {
			MemStats ("Before start");
			theWindowSequence->_Reset(true);
		} else {
			// We've returned from the game itself
			theWindowSequence->Recreate();
			theWindowSequence->Reinitialise();
			MemStats ("On re-entry");

			if (Multiplayer) {
				// Lock profiles if necessary
				for (i = 0; i < NumberOfPlayers; ++i) {
					Window::RelockProfile(i);
				}
				if (!IsTeamGame()) {
					// Increment all the profile information
					int WinScore = GetPlayerScore (PlayerOrder[0]);
					for (i = 0; i < NumberOfPlayers; ++i) {
						if (!ExitedThroughReset) {
							if (GetPlayerScore(i) == WinScore)
								CurProfile[LogToPhys(i)].multiplayerStats.Game[GetCurrentGameType()].won++;
							CurProfile[LogToPhys(i)].multiplayerStats.Game[GetCurrentGameType()].played++;
						}
						Window::WriteProfile(i);
					}
				} else {
					for (i = 0; i < NumberOfPlayers; ++i) {
						if (!ExitedThroughReset) {
							if (player[i].team == player[Winner].team)
								CurProfile[LogToPhys(i)].multiplayerStats.Game[GetCurrentGameType()].won++;
							CurProfile[LogToPhys(i)].multiplayerStats.Game[GetCurrentGameType()].played++;
						}
						Window::WriteProfile(i);
					}
				}
			} else {
				if (GameVMS >= 0 && GameProfile >= 0)
					BupLockProfile (GameVMS, GameProfile);
				// Came back from a single-player game: looky what we got!! (only if we didn't cheat though)
				if (LevelComplete && (CurProfile[0].activeSpecials & SPECIAL_CHEATING_MASK) == 0 && CurProfile[0].cheatWeapon == 0) {
					// Mark as completed
					MarkAsCompleted (LevelNum, &CurProfile[0].missionData[GameDifficulty][LevelNum]);
					// Sort out the profiles
					CurProfile[0].unlockedSpecials = CalculateSpecials();
					// Are we saving to a VMU?  If so re-lock it
					if (GameVMS >= 0 && GameProfile >= 0) {
						// Write back
						Window::WriteProfile(-1);
					}
				}
			}
		}
		if (!ShownLogo) {
			InitLogo();
			OnTitlePage = true;
		}

		SoundOverride = false;

		break;
	case GSR_FINALISE:
		if (DemoOn)
			ShownLogo = FALSE;
		if (Cheating) {
			theWindowSequence->Reset();
		} else {
			FrontEndStillInitialised = true;
			theWindowSequence->Shutdown();
			MemStats ("On exit");
		}
		suppressDebug = FALSE;
		// Reset the renderer
		kmSetWaitVsyncCount (2);
		rReset();
		SHeapClear (GameHeap);
		// Reinitialise input
		InitInput();
		
		// Turn translucent autosort back on
		kmSetAutoSortMode (TRUE);
		
		// Cease thy music, cretin
		SoundStopTrack();
		break;
	case GSR_RUN:
		theVMUManager->Poll();
		// Unless we're logoing, run windows
		if (ShownLogo)
			theWindowSequence->Run();
		else {
			ShownLogo = RunLogo();
			if (ShownLogo) {
				SoundFade(0.f,0);
				SoundPlayTrack(0);
			}
		}
		break;
	}
}

// GINSU stuff:
class GinsuScreen : public Screen
{
private:
public:
	GinsuScreen() : Screen() {}
	virtual void Initialise() {}
	virtual void Finalise() {}
	virtual void Draw() const {}
	virtual void ProcessInput(Uint32 press)
	{
		if (!ChangingState()) {
			LevelNum = 0;
			Multiplayer = false;
			FadeToNewState (FADE_TO_BLACK, GS_GAME);
		}
	}
};
Screen *ginsuScreen() { return new GinsuScreen; }

class GinsuExit : public Screen
{
private:
public:
	GinsuExit() : Screen() {}
	virtual void Initialise() {}
	virtual void Finalise() {}
	virtual void Draw() const {}
	virtual void ProcessInput(Uint32 press)
	{
		if (!ChangingState()) {
			LevelNum = 0;
			Multiplayer = false;
			FadeToNewState (FADE_TO_BLACK, GS_ADVERT);
		}
	}
};
Screen *ginsuExit() { return new GinsuExit; }


//////////////////////////////////////////////////////////////
// Specials and cheats
//////////////////////////////////////////////////////////////

// Sort out the cheats
static int sVCounter = 0;
#define PRESSED(AA) ((press & (PDD_DGT_TX|PDD_DGT_TY|PDD_DGT_TB|PDD_DGT_TA))==(AA))
void LookForCheats(Uint32 press)
{
	if (press) {
		// Show version == YYXXBY
		switch (sVCounter) {
		case 0:
		case 1:
			if (PRESSED(PDD_DGT_TY))
				sVCounter++;
			else
				sVCounter = 0;
			break;
		case 2:
		case 3:
			if (PRESSED(PDD_DGT_TX))
				sVCounter++;
			else
				sVCounter = 0;
			break;
		case 4:
			if (PRESSED(PDD_DGT_TB))
				sVCounter++;
			else
				sVCounter = 0;
			break;
		case 5:
			if (PRESSED(PDD_DGT_TY))
				sVCounter = 0, ShowVersion = true;
			else
				sVCounter = 0;
			break;
		}
	}
}

// Given we've just completed level LevelNum, what should the specials be?
Uint32 CalculateSpecials()
{
	Uint32 retVal = 0;
	int curArmour = 0,
		curWeapon = 0,
		i, GameDifficulty;

	for (GameDifficulty = 0; GameDifficulty < 2; ++GameDifficulty) {
		for (i = 0; i < MAX_MISSIONS; ++i) {
			Mission *m = &CurProfile[0].missionData[GameDifficulty][i];
			if (!m->completed)
				continue;
			switch (i) {
			case 0: // Volcano
				if (m->grading == 0) {
					retVal |= SPECIAL_EXTRA_LEVEL(0);
					if (GameDifficulty)
						retVal |= SPECIAL_ASSISTANCE_BOT_1;
				}
				break;
			case 1: // Ice
				if (m->grading == 0) {
					retVal |= SPECIAL_EXTRA_LEVEL(1);
					if (GameDifficulty)
						retVal |= SPECIAL_PERMANENT_BOOST;
				}
				break;
			case 2: // Canyon
				if (m->grading == 0) {
					retVal |= SPECIAL_EXTRA_LEVEL(2);
					if (GameDifficulty)
						retVal |= SPECIAL_INFINITE_SHIELD;
				}
				break;
			case 3: // Hydro
				if (m->grading == 0) {
					retVal |= SPECIAL_EXTRA_LEVEL(3);
					if (GameDifficulty)
						retVal |= SPECIAL_TRIPPY_TRAILS;
				}
				break;
			case 4: // City
				if (m->grading == 0) {
					retVal |= SPECIAL_EXTRA_LEVEL(4);
					if (GameDifficulty)
						retVal |= SPECIAL_ASSISTANCE_BOT_2;
				}
				break;
			case 5: // Alien base
				if (m->grading == 0) {
					retVal |= SPECIAL_EXTRA_LEVEL(5);
					if (GameDifficulty)
						retVal |= SPECIAL_INF_WEAPON_CHARGE;
				}
				retVal |= SPECIAL_KINGOFTHEHILL;
				retVal |= SPECIAL_HARD_DIFFICULTY;
				if (GameDifficulty)
					retVal |= SPECIAL_INVULNERABILITY;
				break;
				
			case 6: // Challenge 1
				curArmour++;
				if (ChallengeSmashed (i))
					retVal |= SPECIAL_WEAP_SHELL;
				break;
			case 7: // Challenge 2
				curWeapon++;
				if (ChallengeSmashed (i))
					retVal |= SPECIAL_WEAP_VULCAN;
				break;
			case 8: // Challenge 3
				SPECIAL_SHIELD_SET(retVal, 1);
				if (ChallengeSmashed (i))
					retVal |= SPECIAL_WEAP_HOMING;
				break;
			case 9: // Challenge 4
				SPECIAL_MISSILE_SET(retVal, 1);
				if (ChallengeSmashed (i))
					retVal |= SPECIAL_WEAP_LOCKON;
				break;
			case 10: // Challenge 5
				curWeapon++;
				if (ChallengeSmashed (i))
					retVal |= SPECIAL_WEAP_SWARM;
				break;
			case 11: // Challenge 6
				curArmour++;
				if (ChallengeSmashed (i))
					retVal |= SPECIAL_WEAP_ULTRALASER;
				break;
			case 12: // Challenge 7
				curWeapon++;
				if (ChallengeSmashed (i))
					retVal |= SPECIAL_WEAP_CHARGE;
				break;
			}
		}
	}

	// The whole game completed on normal difficulty
	for (i = 0; i < 6; ++i) {
		Mission *m = &CurProfile[0].missionData[0][i];
		if (!m->completed || m->grading != 0)
			break;
	}
	if (i == 6)
		retVal |= SPECIAL_60HZRACE;

	SPECIAL_ARMOUR_SET(retVal, curArmour);
	SPECIAL_WEAPON_SET(retVal, curWeapon);

	retVal |= CurProfile[0].unlockedSpecials & (1<<(SPECIAL_EXTRA_LEVEL(6)));

	return retVal;
}

///////////////////////////////////////////////////////////////////

class RestartScreen : public Screen
{
public:
	virtual void Initialise() {}
	virtual void Finalise() {}
	virtual void ProcessInput(Uint32 press) 
	{
		if (!ChangingState()) {
			if (Multiplayer)
				--CurrentTournamentGame; // replay this tournament game
			FadeToNewState (FADE_TO_BLACK_FAST, GS_GAME);
		}
	}
	virtual void Draw() const {}
};
Screen *restartScreen() { return new RestartScreen; }

class ExitScreen : public Screen
{
public:
	virtual void Initialise() {}
	virtual void Finalise() {}
	virtual void ProcessInput(Uint32 press) 
	{
		ExitedThroughReset = 1;
		if (!ChangingState())
			FadeToNewState (FADE_TO_BLACK, GS_TITLEPAGE);
	}
	virtual void Draw() const {}
};
Screen *exitScreen() { return new ExitScreen; }

static bool hideMenu;
class PauseMenu : public Menu
{
private:
	Uint32 COL_PAUSE_FADE;
	bool debounce;
public:
	PauseMenu() : Menu(&pauseTemplate)
	{
		debounce = hideMenu = false;
	}
	virtual void ProcessInput(Uint32 padAll)
	{
		Menu::ProcessInput(padAll &~PDD_CANCEL);
		if (((pad[0] && (pad[0]->on & (PDD_DGT_TY|PDD_DGT_TX)) == (PDD_DGT_TY|PDD_DGT_TX))) ||
			((pad[1] && (pad[1]->on & (PDD_DGT_TY|PDD_DGT_TX)) == (PDD_DGT_TY|PDD_DGT_TX))) ||
			((pad[2] && (pad[2]->on & (PDD_DGT_TY|PDD_DGT_TX)) == (PDD_DGT_TY|PDD_DGT_TX))) ||
			((pad[3] && (pad[3]->on & (PDD_DGT_TY|PDD_DGT_TX)) == (PDD_DGT_TY|PDD_DGT_TX)))) {
			if (!debounce)
				hideMenu = !hideMenu, debounce = true;
		} else
			debounce = false;
	}
	virtual void Draw() const
	{
		if (!hideMenu)
			Menu::Draw();
	}
	virtual bool SquashOverride() { return true; }
	virtual bool VetoSelection (int selection)
	{
		switch (selection)
		{
		case 0:
			PauseMode = 0;
			break;
		case 5:
			Change (ScreenID (twoAnswers, new TA_Block ("`AYSRESTART", "`AYSRESTART_YES", "`AYSRESTART_NO", ScreenID(restartScreen))));
			break;
		case 6:
			Change (ScreenID (twoAnswers, new TA_Block ("`AYSEXIT", "`AYSEXIT_YES", "`AYSRESTART_NO", ScreenID(exitScreen))));
			break;
		}
		return true;
	}
};

Screen *pauseMenu() { return new PauseMenu; }

// Pause menu madness
static Window *pauseWindow;
static float floaty = 0;
extern "C" bool PadNeeded(int i);
extern "C" void InitPauseMenu(void)
{
	pauseWindow = new Window (-1, ScreenID (pauseMenu));
	floaty = 0;
	SoundPause();

	(void)GetPadAll(); // Start debouncing nooo
}

static const char *GetVMUName(int drive)
{
	static char buf[3] = { 0, 0, 0 };
	buf[0] = (drive>>1) + 'A';
	buf[1] = (drive & 1) + '1';
	return buf;
}

extern "C" void RunPauseMenu(void)
{
	Uint32 padAll;
	Uint32 COL_PAUSE_FADE = 0;

	if (!hideMenu) {
		// Fade out the background
		rSetMaterial (&fadeMat);
		
		floaty += (220-floaty) * 0.2f;
		COL_PAUSE_FADE = int(floaty)<<24;
		
		/* Begin the triangle strip */
		kmxxGetCurrentPtr (&vertBuf);
		kmxxStartVertexStrip (&vertBuf);
		
		kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL, 0, 0, W_LAYER_GREYOUTWHOLE, COL_PAUSE_FADE);
		kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL, 0, PHYS_SCR_Y, W_LAYER_GREYOUTWHOLE, COL_PAUSE_FADE);
		kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL, PHYS_SCR_X, 0, W_LAYER_GREYOUTWHOLE, COL_PAUSE_FADE);
		kmxxSetVertex_0 (KM_VERTEXPARAM_ENDOFSTRIP, PHYS_SCR_X, PHYS_SCR_Y, W_LAYER_GREYOUTWHOLE, COL_PAUSE_FADE);
		
		kmxxReleaseCurrentPtr (&vertBuf);
	}

	mUnit32 (psGetMatrix());
	psSetColour (0xffffff);
	psSetAlpha (1);
	psSetPriority(0);
	
	// Check to see if we're in need-VMU trouble
	int nKnackeredVMUs = 0, aKnackeredVMU;
	for (int v = 0; v < 8; ++v) {
		if (BupDrivePanic[v]) {
			aKnackeredVMU = v;
			nKnackeredVMUs++;
		}
	}
	if (PadOK && nKnackeredVMUs) {
		float y = 48;
		if (nKnackeredVMUs == 1) {
		#if USRELEASE
			psDrawWordWrap (psFormat (LangLookup("USVMUMESG"), GetVMUName(aKnackeredVMU)), 48, y, PHYS_SCR_X-(48*2));
		#else
			psDrawWordWrap (psFormat (LangLookup("VMUMESG"), GetVMUName(aKnackeredVMU)), 48, y, PHYS_SCR_X-(48*2));
		#endif
		} else {
		   	#if USRELEASE
				y += 1.1f * psDrawWordWrap ("`USVMUMESGMULTIPLE", 48, y, PHYS_SCR_X-(48*2));
			#else
				y += 1.1f * psDrawWordWrap ("`VMUMESGMULTIPLE", 48, y, PHYS_SCR_X-(48*2));
			#endif
			for (int v = 0; v < 8; ++v) {
				if (BupDrivePanic[v]) {
					psDrawString (GetVMUName(v), 48, y);
					y += 35.f;
				}
			}
		}
	} else {
		if (!hideMenu) {
			if (Multiplayer && PausedBy != -1) {
				char *name = CurProfile[LogToPhys (PausedBy)].name;
				psDrawCentred (psFormat ("`PAUSEDBY %s", name), 8.f);
			} else {
				psDrawCentred ("`PAUSEMODE", 8.f);
			}
		}
		
		if (PadOK) {
			psSetWindow (64, 64, PHYS_SCR_X-64, PHYS_SCR_Y-32);
			
			if (pauseWindow != NULL) // We're unpausing
				pauseWindow->Run();
			else
				PauseMode = 0;
			if (PauseMode == 0) {
				delete pauseWindow;
				SoundResume();
				pauseWindow = NULL;
			}
		} else {
			float y = 48;
			int numNeeded = 0, mesI;
			for (int i = 0; i < NumberOfPlayers; ++i) {
				if (PadNeeded(LogToPhys(i)))
					numNeeded ++, mesI = i;
			}
			if (numNeeded == 1) {
#if USRELEASE
				psDrawWordWrap (psFormat ("`USCTRLMESG %c", LogToPhys(mesI) + 'A'), 48, y, PHYS_SCR_X-(48*2));
#else
				psDrawWordWrap (psFormat ("`CTRLMESG %c", LogToPhys(mesI) + 'A'), 48, y, PHYS_SCR_X-(48*2));
#endif
			} else {
#if USRELEASE
				y += 1.1f * psDrawWordWrap ("`USCTRLMESGMULTIPLE", 48, y, PHYS_SCR_X-(48*2));
#else
				y += 1.1f * psDrawWordWrap ("`CTRLMESGMULTIPLE", 48, y, PHYS_SCR_X-(48*2));
#endif
				for (i = 0; i < NumberOfPlayers; ++i) {
					if (PadNeeded(LogToPhys(i))) {
						psDrawString (psFormat ("%c", LogToPhys(i) + 'A'), 48, y);
						y += 35.f;
					}
				}
			}
		}
	}
}

////////////////////////////////////////////////
// Get pad mapping for multiplayer
////////////////////////////////////////////////
extern "C" int GetLogToPhysFE(int pNum)
{
	for (int i = 0; i < 4; ++i) {
		// Find player pNum
		if (Window::GetPlayerState(i) == Window::WAITING_FOR_OTHERS) {
			if (pNum-- == 0)
				return i;
		}
	}
	return -1;
}
