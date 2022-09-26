/*-------------------------------------------------------------------
	DREAMCAST INITIALIZATION ROUTINES

	Thomas Szirtes / Sega Europe 1998
	modified from sbinit.c version 0.52

   Copyright (C) 1998 SEGA Enterprises Co.,Ltd
  --------------------------------------------------------------------*/



/*
 *		COMPILER SWITCHES
 */

/* Do you wish to use GDFS (GDROM) system? (0 - no, 1 - yes) */
#define USE_GDFS 	1

/* Place heap at end of memory (ie. don't care about size)? (0-no,1-yes) */
#define USE_B_END 	0

/*
 *		INCLUDE HEADER FILES
 */
#include "RedDog.h"
#include	<shinobi.h>
#include	<sg_syhw.h>
#include	<sg_cache.h>
#include	<kamui.h>
#if GINSU
#include	<ginsu.h>
#endif

/*
 *		DEFINTIONS
 */

#define P1AREA   0x80000000

#define LAST_SYMBOL _BSG_END
extern Uint8* LAST_SYMBOL;

#define WORK_END (((Uint32)LAST_SYMBOL) & 0xe0000000 | 0x0d000000)

/* if using gdfs - specify maximum number of open files, and buffers */
#if	USE_GDFS
# ifdef GINSU
# define FILES (MAX_STREAMS + 2)
# else
 # define FILES MAX_STREAMS
# endif
 #define BUFFERS 128
#endif

/* if not using b_end then specify size of heap */
#if !USE_B_END
 #define HEAP_SIZE (12.5*1024*1024 + 128*1024)
 #define HEAP_AREA ((void*)((Uint32)WORK_END - (Uint32)HEAP_SIZE))
#else
 #define HEAP_AREA ((void*)((((Uint32)LAST_SYMBOL | P1AREA) & 0xffffffe0) + 0x20))
 #define HEAP_SIZE (WORK_END - (Uint32)HEAP_AREA)
#endif


/* 
 *		SYSTEM DATA
 */

/* for maple bus peripheral data */
Uint8 gMapleRecvBuf[1024 * 24 * 2 + 32];
Uint8 gMapleSendBuf[1024 * 24 * 2 + 32];

#if USE_GDFS
 Uint8 gdfswork[GDFS_WORK_SIZE(FILES) + 32];
 Uint8 gdfscurdir[GDFS_DIRREC_SIZE(BUFFERS) + 32];
#endif

static char *configBuffer;

static void GdfsErrHandler (void *obj, Sint32 err)
{
    switch (err)  {
	case GDD_ERR_TRAYOPEND:
	case GDD_ERR_UNITATTENT: // media changed
		if (!IsLoading())
		{
			extern bool sbExitSystemFlag;
			sbExitSystemFlag = true;
		} else {
			sbExitSystem();
			while(1);
		}
		break;
    }
}

int _mode, _frame;

void sbReInitSystem(void)
{
	kmStopDisplayFrameBuffer ();
	kmSetDisplayMode (_mode, _frame, TRUE, FALSE);
}

#ifdef _PAL
Bool InBigPALMode = FALSE;
Uint32 PalMode = KM_PALEXT_HEIGHT_RATIO_1_066;
void PALCallback (void *args)
{
	KMPALEXTINFO *pInfo = (KMPALEXTINFO *)args;
	pInfo->nPALExtMode = PalMode;
	InBigPALMode = (PalMode > KM_PALEXT_HEIGHT_RATIO_1_066) ? true : false;
}
void sbReInitMode (int mode)
{
	_mode = mode;
	sbReInitSystem();
}
#endif

/*
 *		sbInitSystem();
 *
 *		Initialize hardware and shinobi libraries, also initialize Kamui/Ninja as specified
 *
 *		INPUTS :
 *			mode		-		screen resolution and display mode
 *			frame		-		frame buffer format
 *			count		-		vsync count
 */

void sbInitSystem(int mode, int frame, int count)
{
	/* mask interrupts */
	set_imask(15);

	/* first part of hardware initialization */
	syHwInit();

	/* initialize heap for malloc */
	syMallocInit (HEAP_AREA, HEAP_SIZE);	

	kmInitDevice ((mode&(KM_PAL|KM_NTSC|KM_VGA)));
#if ANTIALIAS
	kmSetDisplayMode (mode, frame, TRUE, TRUE);
#else
#ifdef _PAL
	kmSetPALEXTCallback (PALCallback, NULL);
#endif
	_mode = mode;
	_frame = frame;
	kmSetDisplayMode (mode, frame, TRUE, FALSE);
//	kmSetDisplayMode (KM_DSPMODE_PALNI320x240, KM_DSPBPP_RGB888, FALSE, TRUE);
#endif
		/* Set the vsync count */
	kmSetWaitVsyncCount (count);
	/* second part of hardware initialization */
	syHwInit2();

	/* Initilize peripheral input */
	pdInitPeripheral(PDD_PLOGIC_ACTIVE, gMapleRecvBuf, gMapleSendBuf);

    syRtcInit();

	syCacheInit (	SYD_CACHE_FORM_IC_ENABLE | 
					SYD_CACHE_FORM_OC_ENABLE |
					SYD_CACHE_FORM_P1_CB);


	/* release interrupts */
	set_imask(0);

	/* initialize file system */
#if USE_GDFS
	{
		Uint8* wk;
		Uint8* dir;
        Sint32 err;

		wk  = (Uint8*)(((Uint32)gdfswork & 0xffffffe0) + 0x20);
		dir = (Uint8*)(((Uint32)gdfscurdir & 0xffffffe0) + 0x20);

        do {
            err = gdFsInit(FILES, wk, BUFFERS, dir);
        } while (err != GDD_ERR_OK);

		// Install our GDFS error handler
		gdFsEntryErrFuncAll (GdfsErrHandler, NULL);

	}
#endif

	// Backup subsystem
	BupInit();

	// Configuration system
	configBuffer = syMalloc (16*1024);
	dAssert (configBuffer, "Out of RAM");
	syCfgInit (configBuffer);
}



void sbExitSystem(void)
{

	// Stop music playing if it is
	SoundStopTrack();

	// Configuration system
	syCfgExit ();
	syFree (configBuffer);

	BupExit();

#if USE_GDFS
	gdFsFinish();
#endif

    syRtcFinish();

	pdExitPeripheral();

#ifndef _PAL
	njExitSystem();
#endif

	syMallocFinish();

	syHwFinish();

	set_imask(15);

#if !GINSU
	syBtExit();
#else
	gsExit();	
#endif
}


/******************************* end of file *******************************/

