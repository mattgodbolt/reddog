/*
 * Initialisation routines for the Renderer
 */

#include "..\RedDog.h"
#include "Internal.h"
#include "Hardware.h"

#define NATIVE_BUFFER_SIZE		(1*1024*1024 + 512*1024 + 128*1024)
#define TEXTURE_RAM_SIZE_SINGLE	(4*1024*1024 + 512*1024)
#define TEXTURE_RAM_SIZE_MULTI	(3*1024*1024 + 256*1024)

/*
 * Global variables
 */

float	rGlobalTransparency = 1.f;
float	rGlobalPastedOverride = 1.f;
int		rGlobalYAdjust = 0;

/*
 * Border colour changer
 */
#define DEBUG_BORDER(colour) //rSetRegister(HW_BG_COLOUR, colour)

/*
 * The vertex buffer
 */
KMVERTEXBUFFDESC	vertBuf;

/*
 * The front and back screen surfaces
 */
KMSURFACEDESC		screenDesc, backScreenDesc;

// The memory that pVertexBuffer lives in
void *pVertexBuffer;

/*
 * The number of polys drawn
 */
int nDrawn, nTris, nScape, nLit, nLights;

/*
 * Initialises the renderer
 * Called once in the whole game
 * sbInitSystem has already been called
 */
PKMSURFACEDESC 			pArray[2];			/* array of pointers to surfaces */
KMSYSTEMCONFIGSTRUCT config = {
		sizeof(KMSYSTEMCONFIGSTRUCT),		
		KM_CONFIGFLAG_ENABLE_CLEAR_FRAMEBUFFER | /* set any special flags = clear frame buffer */
		KM_CONFIGFLAG_ENABLE_2V_LATENCY |
		KM_CONFIGFLAG_NOWAIT_FINISH_TEXTUREDMA
												,	
		pArray,									/* pointer to array of surface pointers */
		2,										/* number of buffers = 2 */
		PHYS_SCR_X,PHYS_SCR_Y,					/* width and height of surface */
		KM_DSPBPP_RGB565,						/* bits per pixel */
		TEXTURE_RAM_SIZE_SINGLE/4,				/* allocate amount of VRAM for texture */
		&vertBuf,								/* pointer to vertex buffer descriptor */
		0,										/* pointer to vertex buffer - ** malloc now on application side */
		(NATIVE_BUFFER_SIZE)/4,					/* size in DWORDS of vertex buffer - must be multiple of 32 */
		{1.f,60.f,37.f,1.f,1.f},				/* percentage of vbuf to use for each type of vertex */
		0,0,0,0,0,0,0,							/* reserved */
		KM_VERTEXBUFMODEL_NOBUF_OPAQUE			/* latency model = 2V */
	}; 

bool RendererUp = false;

void rInitRenderer (Bool Multiplayer, Uint32 texAmt)
{
	KMSTATUS	result;
	int i, j;
	extern ModelContext context;
	extern UV prepEMapArray[];
	extern LightingValue prepBumpArray[];

	RendererUp = true;

	// Border starts black
	DEBUG_BORDER(0);

	if (Multiplayer) {
		config.nTextureMemorySize = texAmt/4;
#ifdef _PAL
		PalMode = KM_PALEXT_HEIGHT_RATIO_1_166; // Bigger screen on multiplayer
#endif
	} else {
		config.nTextureMemorySize = texAmt/4;
#ifdef _PAL
		PalMode = KM_PALEXT_HEIGHT_RATIO_1_066; // Moderate size for single player
#endif
	}

	// Restart the screen up
	sbReInitSystem();

	DEBUG_BORDER(0x000030); // dark blue is after sbReInitSystem()

	/* Initialise the system */	
	pArray[0] = &screenDesc; pArray[1] = &backScreenDesc;
	pVertexBuffer = syMalloc (NATIVE_BUFFER_SIZE);
	dAssert (pVertexBuffer, "Out of RAM");
	config.pVertexBuffer	=	(void *)((((int)pVertexBuffer) & 0x1fffffff) | 0xa0000000);

	result = kmSetSystemConfiguration (&config);
	dAssert (result == KMSTATUS_SUCCESS, "Unable to set system configuration");

	DEBUG_BORDER(0x0000ff); // bright blue is after kmSetSystemConfiguration()

	/* Set up the small polygon culling */
	result = kmSetCullingRegister (0.75f);
	dAssert (result == KMSTATUS_SUCCESS, "Unable to set culling register");
	
	/* Set the autosort mode on */
	kmSetAutoSortMode (TRUE);

	// Set up the visibility mask
	rScapeVisMask = 0xffffffff;

	// Set up the cheap shadow mode
	kmSetCheapShadowMode(160);

	DEBUG_BORDER(0x003000); // dark green is after kmSet* 

	/* Reset the matrix stack */
	mResetStack();

	DEBUG_BORDER(0x00ff00); // bright green after mResetStack
	
	/* Initialise the various contexts */
	InitContexts();
	
	DEBUG_BORDER(0x300000); // dark red after InitContexts

	/* Initialise text routines */
	tInit();

	DEBUG_BORDER(0xff0000); // bright red after tInit

	/* Initialise the noise routines */
	rInitNoise();

	DEBUG_BORDER(0x00ffff); // bright cyan after noise

	/* Initialise the lighting subsystem */
	lInit ();

	DEBUG_BORDER(0xff00ff); // bright magenta after linit

	/* Reset the profile bar system */
	pbReset();

	DEBUG_BORDER(0xffff00); // bright yellow after pdReset

	// Reinitialise the shadows
	shReInit();

	DEBUG_BORDER(0x303030); // grey after shReInit

	/* Initialise the EOR callback routines */
	kmSetEORCallback (EORCallback, 0);

	DEBUG_BORDER(0xffffff); // bright white after setEOR

	/* Initialise the EOR callback routines */
	{
		extern void WaitCallback (void *);
		kmSetWaitVsyncCallback (WaitCallback, 0);
	}

	DEBUG_BORDER(0x123456); // mad colour after set vsync (bluey grey)
#if 0
	/* Initialise the vsync callback routines */
	{ 
		extern void DoLoadVSync (void *);
		kmSetVsyncCallback (DoLoadVSync, 0);
	}

#endif
#if 0
	{
		extern void HSyncInterrupt(void*);
		kmSetHSyncCallback (HSyncInterrupt, 0);
	}
#endif
	/* Set the clipping adjustment */
	if (Multiplayer) {
		kmSetUserClipLevelAdjust (KM_LEVEL_ADJUST_HALF, &rGlobalYAdjust);
	} else {
		kmSetUserClipLevelAdjust (KM_LEVEL_ADJUST_NORMAL, &rGlobalYAdjust);
	}

	DEBUG_BORDER(0x654321); // another mad colour after set clip

	/* Finally, set the background */
	rSetBackgroundColour (0xff000000);
}

/*
 * Shuts down the renderer gracefully
 */
void rShutdownRenderer (void)
{
	int i, j;

	if (!RendererUp)
		return;
	RendererUp = false;

	// Fade the madness
	DoFades();
	rEndFrame();
	DoFades();
	rEndFrame();

	// Disable all the callbacks
	kmSetWaitVsyncCallback (NULL, 0);
	kmSetEORCallback (NULL, 0);

	// Wait for any pending texture up/downloads
	while (kmQueryFinishLastTextureDMA() != KMSTATUS_SUCCESS)
		;
	// Reset kamui's renderer
	kmResetRenderer();

	// Free all the textures
	texReset();

	// Free the native buffer
	syFree (pVertexBuffer);
	pVertexBuffer = NULL;

	// Free the screen

}
