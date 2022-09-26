/*
 * main.c
 * The main game loop
 */

#include "RedDog.h"
#include "GameState.h"
#include "Backup.h"
#include "Render\Internal.h"
#include "sg_syCbl.h"
#include "Input.h"
#if GINSU
#include "level.h"
#include <ginsu.h>
#include "view.h"
#endif

static void ScreenSave(float seconds);

bool sbExitSystemFlag = false;

/*
 * The current game state, and the next game state
 */
GameState	currentGameState	= 0,
			nextGameState		= 0;

/*
 * Fade information
 */
static Bool		fadeFinished = TRUE;
static FadeType	currentFade = FADE_NONE, nextFade = FADE_NONE;
static Uint32	fadeStartFrame;

/*
 * PAL/NTSC toggle
 * This is a fairly bad misnomer, as it's really a 50/60Hz toggle
 */
Bool PAL = FALSE;

/* Lookup for inverting the fade type */
static FadeType InverseFadeTable[] = 
{
	FADE_NONE,
	FADE_FROM_BLACK,
	FADE_NONE,
	FADE_FROM_BLACK,
	FADE_NONE,
	FADE_FROM_WHITE,
	FADE_NONE,
	FADE_FROM_WHITE,
	FADE_NONE,
	FADE_FROM_BLACK,
	FADE_FROM_BLACK
};

static void advertHandler (GameState state, GameStateReason reason)
{
#if GINSU
	static Camera *fEndCam;
	static int counter;
	Stream *s;
	static Material splashMtl = { 0, 0, 0, 0, MF_TEXTURE_BILINEAR };

	switch (reason) {
	case GSR_INITIALISE:
		fEndCam = CameraCreate();
		fEndCam->type = CAMERA_LOOKAROUND;
		fEndCam->farZ = GlobalFogFar = 5000.f;
		fEndCam->fogNearZ = 0.99f * fEndCam->farZ;
		fEndCam->u.lookDir.rotX = fEndCam->u.lookDir.rotY = fEndCam->u.lookDir.rotZ = 0;
		fEndCam->pos.y = -5.f;
		fEndCam->pos.z = 1.f;
		fEndCam->fogColour.colour = 0;
		CameraSet (fEndCam, 0 );

		texInit();
		s = sOpen ("DemoSplash.pvr");
		texSetDMABuffer (GameHeap->end - 1500*1024, 1500*1024 - 256*1024); 
		splashMtl.surf.GBIX = texLoadSingleTexture (s);
		rFixUpMaterial (&splashMtl, &objContext);
		sClose (s);
		texSetDMABuffer (NULL, 0); 

		// Wait for the DMA to finish
		while (kmQueryFinishLastTextureDMA() != KMSTATUS_SUCCESS)
			;

		counter = 0;

		break;
	case GSR_RUN:
		CameraSet (fEndCam, 0 );

		rSetMaterial (&splashMtl);
		kmxxGetCurrentPtr (&vertBuf);
		kmxxStartVertexStrip (&vertBuf);
		
		kmxxSetVertex_5 (KM_VERTEXPARAM_NORMAL,		0, 0, 0.001,	
			0, 0, 1, 1, 1, 1, 0, 0, 0, 0);
		kmxxSetVertex_5 (KM_VERTEXPARAM_NORMAL,		0, PHYS_SCR_Y, 0.001,	
			0, 1, 1, 1, 1, 1, 0, 0, 0, 0);
		kmxxSetVertex_5 (KM_VERTEXPARAM_NORMAL,		PHYS_SCR_X, 0, 0.001,	
			1, 0, 1, 1, 1, 1, 0, 0, 0, 0);
		kmxxSetVertex_5 (KM_VERTEXPARAM_ENDOFSTRIP, PHYS_SCR_X, PHYS_SCR_Y, 0.001,	
			1, 1, 1, 1, 1, 1, 0, 0, 0, 0);
		kmxxReleaseCurrentPtr (&vertBuf);

//		tPrintf (8, 8, "GO BUY REDDOG FOOL!");
		if (++counter > FRAMES_PER_SECOND * 10)
			sbExitSystem();
		break;
	}
#endif
}

/*
 * Game states - these *must* tally with the header GameState.h
 */
extern GameStateHandler testHandler;
static GameStateTableEntry gameStateTable[] = {
	{ startupHandler,		"Initialisation" },
#if NEW_FRONT_END
	{ testHandler,			"Title page" },
#else
	{ titlePageHandler,		"Title page" },
#endif
	{ gameHandler,			"In-game" },
	{ advertHandler,		"End of GINSU madness" },
};

static void MainLoop (void);

/*
 * Fade handling routine
 */
static Uint32 fadeFrame = 0;
void DoFades (void)
{
	static const Colour black = { 0x0 }, white = { 0xffffff };
	Uint32 fadeFrames;
	float sixty = PAL ? 50.f : 60.f;
	extern bool OverrideMPFadeHack;

	if (IsLoading())
		return;

	// Turn off clipping for the fade polys etc
	kmSetUserClipping (&vertBuf, KM_OPAQUE_POLYGON, 0, 0, 255, 255);
	kmSetUserClipping (&vertBuf, KM_OPAQUE_MODIFIER, 0, 0, 255, 255);
	kmSetUserClipping (&vertBuf, KM_TRANS_POLYGON, 0, 0, 255, 255);

	OverrideMPFadeHack = true;

	fadeFrame++;

	fadeFrames = (fadeFrame - fadeStartFrame) * 2;

	switch (currentFade)
	{
	default:
		dAssert (FALSE, "Invalid fade type");
		break;
	case FADE_NONE:
	case FADE_JET_BLACK:
		rFade (black, 0); // Ensures flares are drawn
		fadeFinished = TRUE;
		break;
	case FADE_TO_BLACK:
		if (fadeFrames < (sixty+2))
			rFade (black, (float)fadeFrames / sixty);
		else {
			rFade (black, 1.f);
			if (fadeFrames >= (sixty+6))
				fadeFinished = TRUE;
		}
		SoundFade((float)fadeFrames / sixty, 1);
		break;
	case FADE_FROM_BLACK:
		if (fadeFrames < 4)
			fadeFrames = 0;
		else
			fadeFrames -= 4;
		if (fadeFrames < (sixty+6))
			rFade (black, (sixty - fadeFrames) / sixty);
		else {
			fadeFinished = TRUE;
		}
//		SoundFade(0.f, 0);
		break;
	case FADE_TO_WHITE:
		if (fadeFrames < (sixty+2))
			rFade (white, (float)fadeFrames / sixty);
		else {
			rFade (white, 1.f);
			if (fadeFrames >= (sixty+6))
				fadeFinished = TRUE;
		}
		SoundFade((float)fadeFrames / sixty, 1);
		break;
	case FADE_FROM_WHITE:
		if (fadeFrames < (sixty+2))
			rFade (white, (sixty - fadeFrames) / sixty);
		else {
			fadeFinished = TRUE;
		}
		break;
	case FADE_TO_BLACK_FAST:
		if (fadeFrames < 4)
			fadeFrames = 0;
		else
			fadeFrames -= 4;
		if (fadeFrames < ((sixty/2)+2))
			rFade (black, (float)fadeFrames / (sixty/2));
		else {
			rFade (black, 1.f);
			if (fadeFrames >= ((sixty/2)+6))
				fadeFinished = TRUE;
		}
		SoundFade((float)fadeFrames / (sixty/2), 1);
		break;
	case FADE_FROM_BLACK_FAST:
		if (fadeFrames < ((sixty/2)+2))
			rFade (black, ((sixty/2) - fadeFrames) / (sixty/2));
		else {
			fadeFinished = TRUE;
		}
//		SoundFade(0.f, 0);
		break;
	case FADE_TO_WHITE_FAST:
		if (fadeFrames < ((sixty/2)+2))
			rFade (white, (float)fadeFrames / (sixty/2));
		else {
			rFade (white, 1.f);
			if (fadeFrames >= ((sixty/2)+6))
				fadeFinished = TRUE;
		}
		SoundFade((float)fadeFrames / (sixty/2), 1);
		break;
	case FADE_FROM_WHITE_FAST:
		if (fadeFrames < ((sixty/2)+5))
			rFade (white, ((sixty/2) - fadeFrames) / (sixty/2));
		else {
			fadeFinished = TRUE;
		}
//		SoundFade(0.f, 0);
		break;
	case FADE_SPIN_OUT:
		if (fadeFrames < 35) {
/*			rSpin (40.f);*/
			if (fadeFrames > 15)
				rFade (black, (fadeFrames - 15.f) / 15.f);
		} else {
			rFade (black, 1.f);
			fadeFinished = TRUE;
		}
		break;
	}
	OverrideMPFadeHack = false;
}

/*
 * Let's get the show on the road...
 */
char GlobalStack[16*1024];
void main (void)
{
	Uint32 *gS;
	int i;
	/*
	 * Before anything else, set up the stack
	 */
	set_imask(15);
	gS = (Uint32 *) &GlobalStack[0];
	for (i = 0; i < asize(GlobalStack)/4; ++i)
		*gS++ = 0xcafebabe;
	MoveStack (GlobalStack + asize(GlobalStack)); // stick it at the end of RAM
	set_imask(0);

	/*
	 * Next set up the GBR
	 */
//	InitGBR(); no longer used

	/*
	 * Now initialise the Shinobi libraries - this gives us
	 * access to the syMalloc function, which InitMemory() uses
	 */

	PAL = 0;
	if (syCblCheckCable() == SYE_CBL_CABLE_VGA) {
		sbInitSystem(KM_DSPMODE_VGA, KM_DSPBPP_RGB565, 2);
	} else {
		switch (syCblCheckBroadcast()) {
#ifndef _PAL
		default:
#endif
		case SYE_CBL_BROADCAST_NTSC:    /* U.S./North America NTSC (60Hz). */
		case SYE_CBL_BROADCAST_PALM:    /* Brazil PAL-M (60Hz). */
			sbInitSystem (KM_DSPMODE_NTSCNI640x480,	KM_DSPBPP_RGB565, 2);
			break;
#ifdef _PAL
		case SYE_CBL_BROADCAST_PAL:     /* Europe PAL (50Hz). */
		case SYE_CBL_BROADCAST_PALN:    /* Argentina PAL-N (50Hz).*/
			sbInitSystem (KM_DSPMODE_PALNI640x480EXT, KM_DSPBPP_RGB565, 2);
			PAL = 1;
			break;
#endif
		}
	}

	// Check to ensure the drive door hasn't been sneakily opened by a nasty person
	gdFsReqDrvStat();

#if GINSU
	gsInit();
#endif

	/*
	 * Initialise the subsystems
	 */
	InitSubSystems();
	ResetRandom();
#if 0
	// VMU madness
	{
		volatile int bar = 0;
		char filename[256];
		BUS_FILEINFO info, info2;
		BUS_DISKINFO diskInfo;
		char *foo = (char *)MapHeap;

		buSetCompleteCallback(0);

		// Read disk info
		buGetDiskInfo (0, &diskInfo);
		while(buStat(0) == BUD_STAT_BUSY);

		// Fiddle around a bit
		ReseedRandom();
		diskInfo.time.hour += (rand() & 3) - 2;
		diskInfo.time.minute = rand() % 60;
		diskInfo.time.second = rand() % 60;

		// Format other disk
		buFormatDisk (BUD_DRIVE_B1, &diskInfo.volume[0], diskInfo.icon_no, &diskInfo.time, FALSE);
		while(buStat(BUD_DRIVE_B1) == BUD_STAT_BUSY);

		buFindExecFile(0,filename);
		buGetFileInfo (0, filename, &info);
		buLoadFile (0, filename, foo, 0);
		while(buStat(0) == BUD_STAT_BUSY);

		// Change that magic number
		{ 
			char *baz;
			baz = foo + 0x249e;
			*baz++ = rand() % 3;
			*baz++ = rand() % 10;
			*baz++ = rand() % 10;
		}


		// Huzzah
		buSaveExecFile (BUD_DRIVE_B1, filename, foo, info.blocks, &info.time, info.copyflag);
		while(buStat(BUD_DRIVE_B1) == BUD_STAT_BUSY);

		while(1);
	}
#endif

	/* Set up the timers */
	syTmrInit();

	/* Initialise the C++ stuff */
	InitCPlusPlus();

	/* Perform compiler/code regression tests */
#ifndef _RELEASE
 	RegressionTest();
#endif

	/* And loop for a little while */
	do {
		MainLoop();
#if GINSU
		if (gsExec() == GS_FORCE_APP_EXIT)
			break;
#endif
	} while (!sbExitSystemFlag);

	sbExitSystem();	
	while(1); // Should never return!
}

/*
 * Main loop routine
 */
static void MainLoop (void)
{
	bool DontRender = (currentGameState == 0);
	/* Increment the frame count, except for paused games */
	if (currentGameState != GS_GAME || !PauseMode)
		currentFrame++;

	/* Run the current handler */
	dAssert (currentGameState >=0 && currentGameState < asize (gameStateTable),
		"Invalid game state");

	InputCheck();

	gameStateTable[currentGameState].handler (currentGameState, GSR_RUN);

	BupRun();

#if GINSU
	/*
	 * If we've been totally inactive for 60 seconds in the game then reset to menu screen
	 */
	{
		extern short DemoOn;
		int nFrames = 90*FRAMES_PER_SECOND;
		if (GetGameState() == GS_TITLEPAGE)
			nFrames *= 2;
		if (GetGameState() != GS_ADVERT) {
			
			if (!ChangingState() && !DemoOn && InactiveFrames > nFrames) {
				FadeToNewState (FADE_TO_BLACK, GS_ADVERT);
			}
		}
	}
#endif

	/* Check to see if the game state has changed, and the fade has finished */
	if (nextGameState != 0 && fadeFinished) {
		int i;
		dAssert (nextGameState >0 && nextGameState < asize (gameStateTable),
			"Invalid game state");
		/* Finalise the previous game state */
		gameStateTable[currentGameState].handler (currentGameState, GSR_FINALISE);
		/* Cancel pending resets */
		PadReset = FALSE;
		/* Take the oppurtunity to flush the backup system */
		BupFlush();
		/* Initialise the new game state */
		currentGameState = nextGameState;
		gameStateTable[currentGameState].handler (currentGameState, GSR_INITIALISE);
		/* No longer changing state */
		nextGameState = 0;
		/* Run for two frames at the current fade settings to clear up */
		for (i = 0; i < 2; ++i) {
			DoFades();
			rEndFrame();
		}
		/* Start the fade-in */
		StartFade (nextFade);
		nextFade = FADE_NONE;
	}

	if (!DontRender)
		DoFades();

#if !AUDIO64 || GDDA_MUSIC
	// Get the CD status
	gdFsReqDrvStat();
#endif

	/*
	 * Screensaver craziness
	 */
	{
		static float SSaveAmount = 0;
		float multiplier = 1;
		if (GetGameState() == GS_TITLEPAGE)
			multiplier = 2;
		if (InactiveFrames > (300 * FRAMES_PER_SECOND * multiplier)) {
			if (GetGameState() == GS_GAME && (Multiplayer || player[0].PlayerState == PS_NORMAL)) {
				extern int PausedBy;
				PauseMode = 1;
				PausedBy = -1;
			}
			SSaveAmount += 0.01f;
		} else
			SSaveAmount -= 0.02f;
		SSaveAmount = RANGE(0, SSaveAmount, 1);
		ScreenSave(SSaveAmount);
	}

	if (!DontRender)
		rEndFrame();
}

/*
 * Game state changing
 */

void ChangeGameState (GameState newState)
{
	dAssert (newState > 0 && newState < asize (gameStateTable),
		"Attempted changing to an invalid state");

	nextGameState = newState;
}

#if 0
char *GetGameStateName (void)
{
	return gameStateTable[currentGameState].name;
}
#endif

/*
 * The bootstrap initialisation routine
 * This should simply ChangeGameState unconditionally
 * to the actual starting point
 */
void startupHandler (GameState state, GameStateReason reason)
{
	if (reason==GSR_RUN) {
#ifdef LEVEL_HACK
		ChangeGameState (GS_TITLEPAGE);
#else
		if (!ChangingState()) {
			FadeToNewState (FADE_JET_BLACK, GS_TITLEPAGE);
			fadeFinished = TRUE;
		}
#endif
	}
}

/*
 * Starts a fade
 */
void StartFade (FadeType type)
{
	dAssert (type >= FADE_NONE && type < FADE_MAX, "Invalid fade type");
	currentFade = type;
	fadeFinished = FALSE;
	nextFade = InverseFadeTable[type];
	fadeStartFrame = fadeFrame;
}

/*
 * Starts a fade, and changes to a new GameState when the fade is over
 */
void FadeToNewState (FadeType type, GameState state)
{
	StartFade (type);
	ChangeGameState (state);
}

/*
 * Has the current fade finished?
 */
Bool hasFadeFinished (void)
{
	return fadeFinished;
}

/*
 * Are we changing state?
 */
Bool ChangingState (void)
{
	return nextGameState != 0;
}


/*
 * Save that screen!!
 */
#define DO_COL(a,r,g,b) \
	RR = 64.f + 63.f * sin(s*r); \
	GG = 64.f + 63.f * cos(s*g); \
	BB = 64.f + 63.f * sin(s*b); \
	col[a]=(((int)RR)<<16) | (((int)GG)<<8) | (((int)BB)<<0); \
	col[a]=alpha | ColLerp (0xffffff, col[a], amount) 
float BANDIT = 1;
static void ScreenSave(float amount)
{
	extern Material fadeMat;
	Material myMat;
	Uint32 col[4], alpha;
	static int myFrame = 0;
	float s = BANDIT*((float)(myFrame++) / (float)(FRAMES_PER_SECOND));
	float RR, GG, BB;

	if (GetGameState() != GS_GAME)
		s *= 0.5f;

	alpha = ((int)(0xa0 * amount)) << 24;

	// Evil jiggery pokery
	myMat = fadeMat;
	myMat.info.TSPPARAMBUFFER = (myMat.info.TSPPARAMBUFFER & ~0xfc000000) | 0x40000000;

	// Work out the colours
	DO_COL(0, 0.1,  0.2,  0.3);
	DO_COL(1, 0.2,  0.05, 0.1);
	DO_COL(2, 0.3,  0.4,  0.2);
	DO_COL(3, 0.05, 0.3,  0.4);

	// Turn off clipping
	kmSetUserClipping (&vertBuf, KM_OPAQUE_POLYGON, 0, 0, 255, 255);
	kmSetUserClipping (&vertBuf, KM_OPAQUE_MODIFIER, 0, 0, 255, 255);
	kmSetUserClipping (&vertBuf, KM_TRANS_POLYGON, 0, 0, 255, 255);

	rSetMaterial (&myMat);
	kmxxGetCurrentPtr (&vertBuf);
	kmxxStartVertexStrip (&vertBuf);

	kmxxSetVertex_0 (0xe0000000, 0, 0, 1e9, col[0]);
	kmxxSetVertex_0 (0xe0000000, 0, PHYS_SCR_Y, 1e9, col[1]);
	kmxxSetVertex_0 (0xe0000000, PHYS_SCR_X, 0, 1e9, col[2]);
	kmxxSetVertex_0 (0xf0000000, PHYS_SCR_X, PHYS_SCR_Y, 1e9, col[3]);

	kmxxReleaseCurrentPtr (&vertBuf);
}
