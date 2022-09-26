// ------------------------------------------------------------------------------------------------
//
// 
//
// 
// ------------------------------------------------------------------------------------------------

#include "RedDog.h"
#include "View.h"
#include "Render\Internal.h"
#include "sndutls.h"

#if !AUDIO64
#error ("ONLY AUDIO 64 NOW SUPPORTED")
#else
#include <ac.h>
#include <shinobi.h>
#include <sg_chain.h>
#include <sg_syg2.h>

#include "localdefs.h"
#include "sndutls.h"
#include "soundint.h"

#define SND_FILE_MAX_HANDLES		(1)
#define SND_MAX_DISTANCE			(200)
#define SND_MIN_DISTANCE			(SND_MAX_DISTANCE>>5)
#define SPU_VOICECH(x)				(0x1L<<(x))
#define SND_FRAME_UPDATE			1
#define SND_MAX_DOPPLER				0.24f
#define SND_REVERB_OUTPUT			23
#define SND_REVERB_PROGRAM			24

GDFS			fileHandleArray[SND_FILE_MAX_HANDLES];
					
static float	sfxCDVolume			= 1.f;
static float	sfxShotVolume		= 0.8f;
static float	sfxShotGlobalVolume	= 1.f;

int				SFXIndirTab[MAXSOUNDS];

#define alPrintf	dPrintf
#define alRand()	rand()

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static ALGlobals		*	alGlobals			=	0;
static ALGlobals		*	alDriver			=	0;
static ALMallocBlock	*	alHeap				=	0;

#ifdef _AL_SND_PLAYER_
static ALSndPlayer		*	alSoundPlayer		=	0;
#endif
#ifdef _AL_SEQ_PLAYER_
static ALCSPlayer		*	alSequencePlayer	=	0;
#endif
#ifdef _AL_STRM_PLAYER_
static ALStrmPlayer		*	alStreamPlayer		=	0;
#endif

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

//###################################################################################
//
//	AUDIO 64 LIBRARY 
//
//###################################################################################

static KTU8 volumeLookUp[256] = {
0x00,0x04,0x08,0x0c,0x10,0x14,0x18,0x1c,0x20,0x24,0x28,0x2e,0x34,0x3a,0x3e,0x44,
0x48,0x4c,0x4e,0x52,0x56,0x5a,0x5c,0x60,0x62,0x66,0x68,0x6a,0x6c,0x6e,0x70,0x72,
0x74,0x76,0x78,0x7a,0x7c,0x7e,0x80,0x82,0x84,0x85,0x86,0x88,0x8a,0x8b,0x8c,0x8e,
0x90,0x91,0x92,0x93,0x94,0x96,0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f,0xa0,0xa1,
0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf,0xb0,0xb1,
0xb2,0xb2,0xb3,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb8,0xb9,0xb9,0xba,0xbb,0xbc,0xbd,
0xbe,0xbe,0xbf,0xbf,0xc0,0xc1,0xc2,0xc2,0xc3,0xc3,0xc4,0xc4,0xc5,0xc5,0xc6,0xc7,
0xc8,0xc8,0xc9,0xc9,0xca,0xca,0xcb,0xcb,0xcc,0xcc,0xcd,0xcd,0xce,0xce,0xcf,0xcf,
0xd0,0xd0,0xd1,0xd1,0xd2,0xd2,0xd3,0xd3,0xd4,0xd4,0xd5,0xd5,0xd6,0xd6,0xd7,0xd7,
0xd8,0xd8,0xd9,0xd9,0xda,0xda,0xdb,0xdb,0xdc,0xdc,0xdc,0xdd,0xdd,0xdd,0xde,0xde,
0xdf,0xdf,0xe0,0xe0,0xe1,0xe1,0xe2,0xe2,0xe2,0xe3,0xe3,0xe3,0xe4,0xe4,0xe4,0xe5,
0xe5,0xe5,0xe6,0xe6,0xe7,0xe7,0xe8,0xe8,0xe8,0xe9,0xe9,0xe9,0xea,0xea,0xea,0xeb,
0xeb,0xeb,0xec,0xec,0xec,0xed,0xed,0xed,0xee,0xee,0xee,0xef,0xef,0xef,0xf0,0xf0,
0xf0,0xf1,0xf1,0xf1,0xf2,0xf2,0xf2,0xf2,0xf3,0xf3,0xf3,0xf3,0xf4,0xf4,0xf4,0xf5,
0xf5,0xf5,0xf6,0xf6,0xf6,0xf7,0xf7,0xf7,0xf8,0xf8,0xf8,0xf8,0xf9,0xf9,0xf9,0xf9,
0xfa,0xfa,0xfa,0xfa,0xfb,0xfb,0xfb,0xfb,0xfc,0xfc,0xfd,0xfd,0xfe,0xfe,0xff,0xff
};

static ALStreamDMAPool dmaPool[AC_MAX_TRANSFER_QUEUE];

//##########################################################################
//
// Global Stream Variables
// 
//##########################################################################

#define BEAT(a, b)					((double)a/(double)b)
#define BEAT_TO_TIME(b, tempo)		((double)BEAT(b, 4.0) * 240000.0 / ((double)tempo/1000.0))

ALStreamFile gStreamFiles[] = 
{
	"TITLE.STR",		1,	0, 0, (KTU32)(BEAT_TO_TIME(1, 120)), 0, AF_STRM_RESTART, 0,
	"BRIEFING.STR",		1,	0, 0, (KTU32)(BEAT_TO_TIME(1, 120)), 0, AF_STRM_RESTART, 0,
	"LEVELCOM.STR",		1,	0, 0, (KTU32)(BEAT_TO_TIME(1, 120)), 0, 0, 0,
	"VOLCANO.STR",		3,	0, 0, (KTU32)(BEAT_TO_TIME(1, 110.02128)), 0, AF_STRM_LOOPED, 0,
	"CITY.STR",			3,	0, 0, (KTU32)(BEAT_TO_TIME(1, 118.05004)), 0, AF_STRM_LOOPED, 0,
	"CANYON.STR",		3,	0, 0, (KTU32)(BEAT_TO_TIME(1, 115.00859)), 0, AF_STRM_LOOPED, 0,
	"ICE.STR",			3,	0, 0, (KTU32)(BEAT_TO_TIME(1, 124.03125)), 0, AF_STRM_LOOPED, 0,
	"HYDRO.STR",		3,	0, 0, (KTU32)(BEAT_TO_TIME(1, 122.02996)), 0, AF_STRM_LOOPED, 0,
	"VOLCANOBOSS.STR",	1,	0, 0, (KTU32)(BEAT_TO_TIME(1, 120)), 0, AF_STRM_LOOPED, 0,
	"HYDROBOSS.STR",	1,	0, 0, (KTU32)(BEAT_TO_TIME(1, 120)), 0, AF_STRM_LOOPED, 0,
	"CANYONBOSS.STR",	1,	0, 0, (KTU32)(BEAT_TO_TIME(1, 120)), 0, AF_STRM_LOOPED, 0,
	"ALIENBASE.STR",	3,	0, 0, (KTU32)(BEAT_TO_TIME(1, 134.01539)), 0, AF_STRM_LOOPED, 0,
	"ALIENBOSS.STR",	1,	0, 0, (KTU32)(BEAT_TO_TIME(1, 120)), 0, AF_STRM_LOOPED, 0,
	"ICEBOSS.STR",		1,	0, 0, (KTU32)(BEAT_TO_TIME(1, 120)), 0, AF_STRM_LOOPED, 0,
	"CITYBOSS.STR",		1,	0, 0, (KTU32)(BEAT_TO_TIME(1, 120)), 0, AF_STRM_LOOPED, 0,
	"FIBS.STR",			1,	0, 0, (KTU32)(BEAT_TO_TIME(1, 120)), 0, AF_STRM_LOOPED, 0,
	"FREAKTOR.STR",		1,	0, 0, (KTU32)(BEAT_TO_TIME(1, 120)), 0, AF_STRM_LOOPED, 0,
	"TRIP2.STR",		1,	0, 0, (KTU32)(BEAT_TO_TIME(1, 120)), 0, AF_STRM_LOOPED, 0,
	"HAPPY.STR",		1,	0, 0, (KTU32)(BEAT_TO_TIME(1, 120)), 0, AF_STRM_LOOPED, 0
};
#define STRM_MAX_TRACKS (asize(gStreamFiles))

//##########################################################################
//
// Internal File and Interrupt Routines and Handlers
//
//##########################################################################

////////////////////////////////////////////////////////////////////////////////
// Func:	alShowDriverRev
// Desc:	
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
void alShowDriverRev(volatile KTU32 *driverImage)
{
#ifdef _DEBUG
	KTU8 	majorVersion;
	KTU8 	minorVersion;
	KTCHAR 	localVersion;
	extern const char * audio64Build;
	
	acSystemGetDriverRevision((KTU8 *)driverImage,&majorVersion,&minorVersion,&localVersion);
	
	dPrintf("----------------------------------------------------------------------------------------");
	dPrintf("Lib Revision       -%s", audio64Build);
	dPrintf("Lib Expects Driver - Major: %u Minor: %u Local: %c",AC_MAJOR_REVISION,AC_MINOR_REVISION,AC_LOCAL_REVISION);
	dPrintf("Driver Revision    - Major: %u Minor: %u Local: %c",majorVersion,minorVersion,localVersion);
	dPrintf("----------------------------------------------------------------------------------------");
#endif
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Func:	alSh4Alloc
// Desc:	
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
KTBOOL alSh4Alloc(volatile KTU32 ** base, volatile KTU32 ** aligned, KTU32 size, KTU32 alignment)
{
	volatile KTU32 *	baseAddress		=	KTNULL;
	volatile KTU32 *	alignedAddress	=	KTNULL;

	if ((baseAddress = (volatile KTU32 *)syMalloc(size)) == 0x00)
		return KTFALSE;
	
	if ((KTU32)baseAddress % alignment) {
		syFree((void *)baseAddress);

		if ((baseAddress = (volatile KTU32 *)syMalloc(size + (alignment-1))) == 0x00)
			return KTFALSE;

		alignedAddress = (volatile KTU32 *)(((KTU32)baseAddress) + (alignment - ((KTU32)baseAddress) % alignment));
	} else {
		alignedAddress = (volatile KTU32 *)((KTU32)baseAddress);
	}
	
	*base		=	baseAddress;
	*aligned	=	alignedAddress;
	
	return KTTRUE;
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alSh4Free
// Desc:	
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
void alSh4Free(volatile KTU32 * block)
{
	syFree((void *)block);
}

////////////////////////////////////////////////////////////////////////////////
// Func:	Quick Sh4Alloc function
// Desc:	
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
void * cxalloc(KTU32 size)
{
	volatile KTU32 * 	base		=	KTNULL;
	volatile KTU32 * 	aligned		=	KTNULL;
	KTU32				alignment	=	32;
	
	alSh4Alloc(&base, &aligned, size, alignment);
	
	return (void *)aligned;
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alFileAllocateHandle
// Desc:	
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
KTBOOL alFileAllocateHandle(ACFILE *fd)
{
	KTU32 i = 0;
	
	for (i=0; i<SND_FILE_MAX_HANDLES; i++) {
		if (fileHandleArray[i] == KTNULL) {
			*fd	= (ACFILE)i;
			return KTTRUE;
		}
	}
		
	return KTFALSE;
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alFileFreeHandle
// Desc:	
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
void alFileFreeHandle(ACFILE fd)
{
	fileHandleArray[fd]	= KTNULL;
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alFileInit
// Desc:	
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
void alFileInit(void)
{
	memset((void *)&fileHandleArray, 0, sizeof(fileHandleArray));
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alFileRead
// Desc:	
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
KTBOOL alFileRead(ACFILE fd, KTU8 * buffer, KTU32 size)
{
	KTU32	numberOfSectors	=	0;
	KTU32	sectorSize		=	2048;
	KTU32	moduloSize		=	0;
	KTU32	remainder		=	0;
	KTU8	tempBuffer[(2048+32)];		// temp read buffer
	KTU8 *	tempPtr			=	&tempBuffer[0];
	KTU32	tempOff			=	0;
	KTU8 *	writeCursor		=	buffer;
	Sint32	error			=	0;
	
	dAssert(fileHandleArray[fd] != KTNULL, "SND_FileRead - fd is NULL");
	dAssert(buffer != KTNULL, "SND_FileRead - buffer is NULL");
	dAssert(!((KTU32)buffer % 32), "SND_FileRead - buffer is misaligned");	
	dAssert(size != 0,"SND_FileRead - size is 0");

	remainder	=	(size % sectorSize);
	
	if (remainder) {	// works
		moduloSize		=	(size-remainder) + sectorSize;
		numberOfSectors	=	moduloSize/sectorSize;
	} else {
		numberOfSectors	=	size/sectorSize;
	}
	
	if (remainder == 0)	{	// it is an even number of sectors read once
		if ((error = gdFsRead(fileHandleArray[fd], numberOfSectors, buffer)) != GDD_ERR_OK)	{
			dPrintf("SND_FileRead - gdFsRead failed\n");
			return KTFALSE;
		}
	} else {
		memset(tempPtr, 0x00, remainder); // dont set the whole buffer if not necessary
	
		if (gdFsRead(fileHandleArray[fd], numberOfSectors-1, buffer) != GDD_ERR_OK)	{
			dPrintf("SND_FileRead - gdFsRead failed\n");
			return KTFALSE;
		}
	
		if (((KTU32)tempPtr) % 32) {	// works
			tempOff	= 32 - 	(((KTU32)tempPtr) % 32);
			tempPtr += tempOff;
		}
	
		if (gdFsRead(fileHandleArray[fd], 1, tempPtr) != GDD_ERR_OK) {
			dPrintf("SND_FileRead - gdFsRead failed\n");
			return KTFALSE;
		}
	
		writeCursor	=	buffer + ((numberOfSectors-1) * 2048);
	
		memcpy(writeCursor, tempPtr, remainder); // paste it at the end of the last read
	}
	
	return KTTRUE;
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alFileOpen
// Desc:	
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
KTBOOL alFileRewind(ACFILE fd)
{
	if(gdFsSeek(fileHandleArray[fd], 0, GDD_SEEK_SET) != GDD_ERR_OK) {
		dPrintf("alFileRewind - gdFsSeek fail\n");
		return KTFALSE;
	}

	return KTTRUE;
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alFileOpen
// Desc:	
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
KTBOOL alFileOpen(KTSTRING fileName, ACFILE *fd, KTS32 *fileSize)
{
	KTU32 free = 0;
		
	dAssert(fileName != KTNULL, "SND_FileOpen - fileName is NULL");
	dAssert(fd != KTNULL, "SND_FileOpen - fd is NULL");
	
	if (alFileAllocateHandle(&free) == KTFALSE) {
		dPrintf("%s", "SND_FileOpen() Internal error, no more free file handles...");
		return KTFALSE;
	}
	
	fileHandleArray[free] = gdFsOpen(fileName, NULL);
	
	if (fileHandleArray[free] == NULL) {
		alFileFreeHandle(free);
		dPrintf("alFileOpen - file not found\n");
		return KTFALSE;
	} else {
		*fd	=	free;
	}
			
	if (fileSize) {
		if (gdFsGetFileSize(fileHandleArray[free], fileSize) != TRUE) {
			dPrintf("SND_FileGetSize - gdFsGetFileSize fail\n");
			gdFsClose(fileHandleArray[free]);
			return KTFALSE;
		}
	}

	return KTTRUE;
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alFileClose
// Desc:	
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
KTBOOL alFileClose(ACFILE fd)
{
	dAssert(fileHandleArray[fd] != KTNULL, "FileClose - fd is NULL");
	
	gdFsClose(fileHandleArray[fd]);

	alFileFreeHandle(fd);
	
	return KTTRUE;
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alFileGetSize
// Desc:	
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
KTBOOL alFileGetSize(KTSTRING fileName, KTU32 * size)
{
	GDFS	fp			=	NULL;
	Sint32	fileSize	=	0;
	
	dAssert(size != NULL, "SND_FileGetSize - size is NULL");
	dAssert(fileName != KTNULL, "SND_FileGetSize - fileName is NULL");
	
	if ((fp = gdFsOpen(fileName, NULL)) == NULL) {
		dPrintf("SND_FileGetSize - gdFsOpen fail (file not found)\n");
		return KTFALSE;
	}
	
	if (gdFsGetFileSize(fp, &fileSize) != TRUE) {
		dPrintf("SND_FileGetSize - gdFsGetFileSize fail\n");
		gdFsClose(fp);
		return KTFALSE;
	}
		
	gdFsClose(fp);
	
	*size = fileSize;
	
	return KTTRUE;   
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alFileLoad
// Desc:	
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
KTBOOL alFileLoad(KTSTRING fileName, volatile KTU32 *buffer)
{
	KTU32 		fileSize	=	0;
	ACFILE		gd			=	KTNULL;
	
	dAssert((fileName != KTNULL), "SND_FileLoad - fileName is NULL");
	dAssert((buffer != KTNULL), "SND_FileLoad - buffer is NULL");
	
	if (alFileGetSize(fileName, &fileSize) == KTFALSE)
		return KTFALSE;
	
	if (alFileOpen(fileName, &gd, 0) == KTFALSE)
		return KTFALSE;
	
	if (alFileRead(gd, (KTU8 *)buffer, fileSize) == KTFALSE)
		return KTFALSE;
	
	if (alFileClose(gd) == KTFALSE)
		return KTFALSE;
	
	return KTTRUE;
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alAsyncFileSeek
// Desc:	
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
KTBOOL alAsyncFileSeek(ACFILE sd, KTU32 offset)
{
	dAssert(fileHandleArray[sd] != KTNULL, "AsyncFileRewind: File Handle is NULL");
	
	if (alGlobals->drvr.vblankFlag == KTTRUE) {
		if(gdFsSeek(fileHandleArray[sd], offset, GDD_SEEK_SET) != GDD_ERR_OK) {
			dPrintf("AsyncFileRewind: gdsfSeekFailed\n");
			return KTFALSE;
		}	
	}

	return KTTRUE;
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alVBlankHandler
// Desc:	
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
void alVBlankHandler(void *arg)
{	
	if (arg == (void *)AL_FILE_VBLANK_ID) {
		alGlobals->drvr.vblankFlag = KTTRUE;
		
//		if (alStreamPlayer) {
//			SoundUpdatePostDraw();
//		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alAsyncFileGetErrorText
// Desc:	
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
void alAsyncFileGetErrorText(KTS32 drvStat,char *drvStatTxt,KTS32 errStat,char *errStatTxt,KTS32 trnStat,char *trnStatTxt)
{
	// complete list of status conditions
	if(drvStat==GDD_DRVSTAT_CANTREAD)		strcpy(drvStatTxt,"GDD_DRVSTAT_CANTREAD");
	else if(drvStat==GDD_DRVSTAT_BUSY)		strcpy(drvStatTxt,"GDD_DRVSTAT_BUSY");
	else if(drvStat==GDD_DRVSTAT_PAUSE)		strcpy(drvStatTxt,"GDD_DRVSTAT_PAUSE");
	else if(drvStat==GDD_DRVSTAT_STANDBY)	strcpy(drvStatTxt,"GDD_DRVSTAT_STANDBY");
	else if(drvStat==GDD_DRVSTAT_PLAY)		strcpy(drvStatTxt,"GDD_DRVSTAT_PLAY");
	else if(drvStat==GDD_DRVSTAT_SEEK)		strcpy(drvStatTxt,"GDD_DRVSTAT_SEEK");
	else if(drvStat==GDD_DRVSTAT_SCAN)		strcpy(drvStatTxt,"GDD_DRVSTAT_SCAN");
	else if(drvStat==GDD_DRVSTAT_OPEN)		strcpy(drvStatTxt,"GDD_DRVSTAT_OPEN");
	else if(drvStat==GDD_DRVSTAT_NODISC)	strcpy(drvStatTxt,"GDD_DRVSTAT_NODISC");
	else if(drvStat==GDD_DRVSTAT_RETRY)		strcpy(drvStatTxt,"GDD_DRVSTAT_RETRY");
	else if(drvStat==GDD_DRVSTAT_ERROR)		strcpy(drvStatTxt,"GDD_DRVSTAT_ERROR");
	else									strcpy(drvStatTxt,"huh ???");
	
	// complete list of transfer conditions
	if(trnStat==GDD_FS_TRANS_READY)			strcpy(trnStatTxt,"GDD_FS_TRANS_READY");
	else if(trnStat==GDD_FS_TRANS_BUSY)		strcpy(trnStatTxt,"GDD_FS_TRANS_BUSY");
	else if(trnStat==GDD_FS_TRANS_COMPLETE)	strcpy(trnStatTxt,"GDD_FS_TRANS_COMPLETE");
	else if(trnStat==GDD_FS_TRANS_ERROR)	strcpy(trnStatTxt,"GDD_FS_TRANS_ERROR");
	else									strcpy(trnStatTxt,"huh ???");
	
	// just de facts
	if(errStat==GDD_ERR_OK)					strcpy(errStatTxt,"GDD_ERR_OK");
	else									strcpy(errStatTxt,"See error numbers");
}
#endif	// DEBUG

////////////////////////////////////////////////////////////////////////////////
// Func:	alAsyncFileShowErrors
// Desc:	
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
void alAsyncFileShowErrors(GDFS gd, char *message)
{
	KTS32	gdfsDrvStat	=	0;
	KTS32	gdfsErrStat	=	0;
	KTS32	gdfsTranStat	=	0;
	KTS8	drive[255];
	KTS8	transfer[255];
	KTS8	error[255];
	
	if (gd != KTNULL) {
		gdfsDrvStat			=	gdFsGetDrvStat();
		gdfsErrStat			=	gdFsGetErrStat(gd);
		gdfsTranStat		=	gdFsGetTransStat(gd);
	}
	
	alAsyncFileGetErrorText(gdfsDrvStat, drive, gdfsErrStat, error, gdfsTranStat, transfer);
	
	dPrintf("%s drvStat:%d %s errStat:%d %s tranStat:%d %s",message,gdfsDrvStat,drive,gdfsErrStat,error,gdfsTranStat,transfer);
}
#endif	// DEBUG

////////////////////////////////////////////////////////////////////////////////
// Func:	alAsyncFileRead
// Desc:	
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
KTBOOL alAsyncFileRead(ACFILE sd, KTU8 * buffer, KTU32 size, KTU32 callback)
{
	KTU32	numberOfSectors	=	0;
	KTS32	gdFsError		=	0;
	
	dAssert(fileHandleArray[sd] != KTNULL, "AsyncFileRead: File Handle is NULL");
	dAssert(buffer != KTNULL,"MyAsyncFileRead - buffer is NULL");
	dAssert(!((KTU32)buffer % 32),"MyAsyncFileRead - buffer is misaligned");	
	dAssert(size != 0,"MyAsyncFileRead - size is 0");
	dAssert(size % 2048 == 0, "MyAsyncFileRead - size must be divisable by 2048");

	if (alGlobals->drvr.vblankFlag == KTFALSE) {
		dPrintf("AsyncRead: VBlank Not set\n");
		return KTFALSE;
	} 
	
	numberOfSectors = size>>11;
	
	if (callback) {
		gdFsEntryRdEndFunc(fileHandleArray[sd], alStreamProcessChunk, 0);
	}

	if ((gdFsError = gdFsReqRd32(fileHandleArray[sd], numberOfSectors, (void *)buffer)) != numberOfSectors) {
		dPrintf("Sectors requested: %u Confusing dual error value: %d",numberOfSectors,gdFsError);
#ifdef _DEBUG
		alAsyncFileShowErrors(fileHandleArray[sd],"gdFsReqRd32() FAIL");
#endif
		return KTFALSE;
	}
	
	// clear the flag, this will force it to wait until the next vblank
	alGlobals->drvr.vblankFlag= KTFALSE;
	
	return KTTRUE;
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alAsyncFileOpen
// Desc:	
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
KTBOOL alAsyncFileOpen(KTSTRING fileName, ACFILE *sd, KTS32	*fileSize)
{		
	ACFILE	descriptor;
		
	dAssert(fileName != KTNULL,"MyAsyncFileOpen - fileName is NULL");
	dAssert(sd != KTNULL,"MyAsyncFileOpen - sd is NULL");
	
	if (alGlobals->drvr.vblankFlag == KTFALSE) {
		// Not ready yet
		dPrintf("VBlank not ready\n");
		return KTFALSE;
	}
	
	if (alFileAllocateHandle(&descriptor) == KTFALSE){
		dPrintf("MyAsyncFileOpen() Internal error, no more free file handles\n");
		return KTFALSE;
	}
		
	fileHandleArray[descriptor] = gdFsOpen(fileName,NULL);
	if (fileHandleArray[descriptor] == KTNULL) {	// do the open
		dPrintf("MyAsyncFileOpen() Could not open file\n");
		fileHandleArray[descriptor] = KTNULL;
		return KTFALSE;
	}
	
	if (fileSize) {
		if (gdFsGetFileSize(fileHandleArray[descriptor], fileSize) != TRUE) {
			dPrintf("MyAsyncFileGetSize() Could not open file\n");
			gdFsClose(fileHandleArray[descriptor]);
			return KTFALSE;
		}
	}

	alGlobals->drvr.vblankFlag = KTFALSE;
	
	*sd	= descriptor;
	
	return KTTRUE;
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alAsyncFileGetTransferStatus
// Desc:	
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
KTBOOL alAsyncFileGetTransferStatus(ACFILE sd)
{
	if (!gdFsGetWorkHn()) {
		switch (gdFsGetStat(fileHandleArray[sd]))
		{
		case (GDD_STAT_READ):
		case (GDD_STAT_SEEK):
		case (GDD_STAT_BUSY):
			return KTFALSE;
		case (GDD_STAT_ERR):
			alPrintf("FileError\n");
			gdFsReqDrvStat();	// call cd status
			return (KTBOOL)(-1);
		default:	return KTTRUE;
		}
	}
	return KTFALSE;
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alAsyncFileClose
// Desc:	
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
KTBOOL alAsyncFileClose(ACFILE sd)
{
	dAssert(fileHandleArray[sd] != KTNULL,"MyAsyncFileClose - sd is NULL");
	
	while (!alAsyncFileGetTransferStatus(sd));

	gdFsClose(fileHandleArray[sd]);
	
	fileHandleArray[sd]			=	KTNULL;
	alGlobals->drvr.vblankFlag	=	KTFALSE;
	
	return KTTRUE;
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alArmInterruptHandler
// Desc:	
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
void alArmInterruptHandler(void *arg)
{
	volatile KTU32 *writeCursor;
	volatile KTU32 writeIndex;
	volatile KTU32 fifoCount	=  0;
	volatile KTU32 i			=  0;
	volatile KTU32 *target		= (volatile KTU32 *)&gAcSystem.intArray[0];
	volatile KTU32 *source		=  gAcSystem.intArrayStartPtr;
	volatile KTU32 oldImask		=  get_imask();	// get the old imask value

	if ((KTU32)arg != AC_ARM_INTERRUPT_HANDLER_ID && 
		(KTU32)arg != AC_ARM_POLLING_HANDLER_ID) return;

	set_imask(15);							// disable interrupts
	alDmaSuspendAll();						// suspend all DMA
	acG2FifoCheck();						// wait for the AICA/HOLLY FIFO's to empty
	
	*gAcSystem.intResetFlag	= 0x20;

	writeCursor	= (KTU32 *)(gAcSystem.soundMemoryBaseAddress + AC_INT_ARRAY_OFFSET);
	writeIndex	= (KTU32)(*writeCursor) - gAcSystem.intArrayStartOffset; // byte array in terms of ptrs
	
	while (i < 16) {
		*(target+i) = *(source+i);			// write a long
		++i;								// inc dword counter
		++fifoCount;						// inc FIFO counter
		if (fifoCount > 7) {				// is FIFO full ?
			fifoCount = 0;					// clear FIFO counter
			acG2FifoCheck();				// wait for the AICA/HOLLY FIFO's to empty
		}										
	}
	
	acG2FifoCheck();		// wait for the AICA/HOLLY FIFO's to empty
	alDmaResumeAll();		// resume all DMA
	set_imask(oldImask);	// enable interrupts

	while (gAcSh4ReadIndex != writeIndex) {
		gAcCallbackHandler(gAcSystem.intArray[gAcSh4ReadIndex]);
		gAcSh4ReadIndex = (gAcSh4ReadIndex + 1) & 63;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alDmaSuspendAll
// Desc:	
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
KTBOOL alDmaSuspendAll(void)
{
	return (KTBOOL)(syG2DmaSuspendAll() == SYE_G2_ERR_NOTHING);
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alDmaResumeAll
// Desc:	
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
KTBOOL alDmaResumeAll(void)
{
	return (KTBOOL)(syG2DmaResumeAll() == SYE_G2_ERR_NOTHING);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Func:	alMallocInit
// Desc:	Initialises the SPU memory management
// In:		
// Out:		
// Cost:	
// Notes:	Uses a small table in main memory
////////////////////////////////////////////////////////////////////////////////
void alMallocInit(void)
{
	alHeap = (ALMallocBlock *)cxalloc(sizeof(ALMallocBlock));
	alHeap->start   = (char *)(((KTU32)acSystemGetFirstFreeSoundMemory() + 63) & ~63);
	alHeap->end     = (char *)AC_BASE_OF_SOUND_MEMORY + (2*1024*1024);
	alHeap->current = alHeap->start;
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alMalloc
// Desc:	Allocates a block in sound memory
// In:		
// Out:		Next free address in sound memory
// Cost:	
// Notes:	Sound DMA's are in 64 byte blocks
////////////////////////////////////////////////////////////////////////////////
void * alMalloc(Uint32 size)
{	
	Uint32	xsize = ((size + 63) & ~63);
	void *	temp  = alHeap->current;

	if (((Uint32)alHeap->current + xsize) > (Uint32)alHeap->end) {
		alPrintf(AL_ERR_MALLOC);

		CrashOut ("No Sound Memory", 0xffffff, 0x00ff00);

		return 0;
	}

	alHeap->current += xsize;

	return temp;
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alFree
// Desc:	Frees the end of the sound heap
// In:		Address in Sound memory
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
void alFree(void *ptr)
{
	if ((char *)ptr < alHeap->start || (char *)ptr > alHeap->end) {
		alPrintf(AL_ERR_MALLOC);
		return;
	}

	alHeap->current = ptr;
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alLink
// Desc:	Links an event to a linked list
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
void alLink(ALLink *ln, ALLink *to) 
{
	ln->next = to->next;			
	ln->prev = to;					
	if (to->next)						
		to->next->prev = ln;		
	to->next = ln;					
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alUnlink
// Desc:	Frees and event from a linked list
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
void alUnlink(ALLink *ln)	
{
	if (ln->next)						
		ln->next->prev = ln->prev;	
	if (ln->prev)						
		ln->prev->next = ln->next;	
}

void alSynNew(ALSynth *drvr, ALSynConfig *c);

////////////////////////////////////////////////////////////////////////////////
// Func:	alInit
// Desc:	Initialises SPU and SPU volume
// In:		Address of the global sound variable
// Out:		
// Cost:	
// Notes:	Only needs to be called once
////////////////////////////////////////////////////////////////////////////////
Sint32 alInit(ALGlobals *g, ALSynConfig *c)
{
    if (!alGlobals) {
		volatile KTU32 * 	base				=	KTNULL;
		volatile KTU32 * 	driverImage			=	KTNULL;
		KTU32 				driverSize			=	0;
		char *				driverFileName		=	"Audio64.drv";
		
		alFileInit();
		
		acIntInstallCallbackHandler (alStreamCallbackHandler);
		acIntInstallArmInterruptHandler (alArmInterruptHandler);

		if (!acIntInit()) {
			return 0;
		}
		
		if (!alFileGetSize(driverFileName, &driverSize)) {
			return 0;
		}
		
		if (!alSh4Alloc(&base, &driverImage, driverSize, 32)) {
			return 0;
		}
		
		if (!alFileLoad(driverFileName, driverImage)) {
			return 0;
		}

		if (!acSystemInit(AC_DRIVER_DA, driverImage, driverSize, KTFALSE)) {
			return 0;
		}
		
#ifdef _DES_
		alShowDriverRev(driverImage);
#endif

		// Set the transfer mode to be with with DMA
		acSetTransferMode(AC_TRANSFER_DMA);
		
		// Wait for the Audio System to reset itself
		acSystemDelay(50000000);	
		
//		acSystemSetMasterVolume(AC_MAX_SEND_LEVEL);	

		alSh4Free(base);

		memset(dmaPool, 0, AC_MAX_TRANSFER_QUEUE * sizeof(ALStreamDMAPool));

		alGlobals = g;		
		alMallocInit();        
		alSynNew(&alGlobals->drvr, c);
    }

	return 1;
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alClose
// Desc:	Shutdowns SPU and turns off SPU volume
// In:		Address of the global sound variable
// Out:		
// Cost:	
// Notes:	Only needs to be called for demos, must set reverb to OFF
////////////////////////////////////////////////////////////////////////////////
void alClose(ALGlobals *g)
{						
	(void)g;

	if (alGlobals) {
        alGlobals->drvr.head = 0;
        alGlobals = 0;
		acSystemShutdown();
    }
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alEvtqNew
// Desc:	Creates a new event queue
// In:		Ptr to the event queue, Ptr to the items to link
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
void alEvtqNew(ALEventQueue *evtq, ALEventListItem *items, Sint32 itemCount)
{
	Sint32 i;

	evtq->eventCount = 0;
	evtq->maxEventCount = 0;
	evtq->allocList.next = 0;
	evtq->allocList.prev = 0;
	evtq->freeList.next = 0;
	evtq->freeList.prev = 0;

	for (i=0; i<itemCount; i++) {
		alLink((ALLink *)&items[i], &evtq->freeList);
	}
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alEvtqNextEvent
// Desc:	Retrieves the next event from a queue
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
Sint32 alEvtqNextEvent(ALEventQueue *evtq, ALEvent *evt)
{
	ALEventListItem *item;
	Sint32 time;

	item = (ALEventListItem *)evtq->allocList.next;

	if (item) {
		alUnlink((ALLink *)item);
		memcpy(evt, &item->evt, sizeof(*evt));
		alLink((ALLink *)item, &evtq->freeList);
		time = item->delta;
#ifdef DES_DEBUG	
		evtq->eventCount--;
#endif
	} else {
		evt->type = AL_EVENT_FLUSH;
		alPrintf(AL_ERR_EVENT_OVERFLOW);
		time = 0;
	}

	return time;
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alEvtqPostEvent
// Desc:	Adds an event to the event queue
// In:		Ptr to the event queue, Ptr to the event, time to start event
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
void alEvtqPostEvent(ALEventQueue *evtq, ALEvent *evt, Sint32 time)
{
	ALEventListItem *item, *nextItem;
	ALLink *node;
    Sint32	postAtEnd=0;

	item = (ALEventListItem *)evtq->freeList.next;
	if (!item) {
#ifdef _DEBUG
		alPrintf(AL_ERR_EVENT_OVERFLOW);
#endif										 	
		return;
	}

	alUnlink((ALLink *)item);
	memcpy(&item->evt, evt, sizeof(*evt));

    if (time == AL_EVTQ_END) {
		postAtEnd = -1;
	}

	for (node = &evtq->allocList; node != 0; node = node->next) {
		if (!node->next) {
            if (postAtEnd)
                item->delta = 0;
            else
                item->delta = time;
			break;
		} else {
			nextItem = (ALEventListItem *)node->next;
			if (time < nextItem->delta) {
				item->delta = time;
				nextItem->delta -= time;
				break;
			}
			time -= nextItem->delta;
		}
	}				  

#ifdef DES_DEBUG	
	evtq->eventCount++;
	if (evtq->eventCount > evtq->maxEventCount)
		evtq->maxEventCount = evtq->eventCount;
#endif

	alLink((ALLink *)item, node);
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alEvtqFlush
// Desc:	Removes all events from an existing queue
// In:		Ptr to the event queue to be flushed
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
void alEvtqFlush(ALEventQueue *evtq)
{
	ALLink *thisNode;
	ALLink *nextNode;

	thisNode = evtq->allocList.next;
	while (thisNode != 0) {
		nextNode = thisNode->next;
		alUnlink(thisNode);
		alLink(thisNode, &evtq->freeList);
		thisNode = nextNode;
#ifdef DES_DEBUG	
		evtq->eventCount--;
#endif
	}
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alEvtqFlushState
// Desc:	Removes events of a certain state from an existing queue
// In:		Ptr to the event queue to be flushed
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
void alEvtqFlushState(ALEventQueue *evtq, ALVoiceState *state)
{
	ALLink              *thisNode;
	ALLink              *nextNode;
	ALEventListItem     *thisItem, *nextItem;
    ALEvent				*thisEvent;

	thisNode = evtq->allocList.next;
	while (thisNode != 0) {
		nextNode = thisNode->next;
        thisItem = (ALEventListItem *)thisNode;
        nextItem = (ALEventListItem *)nextNode;
		thisEvent = (ALEvent *)&thisItem->evt;
        if (thisEvent->msg.state == state) {
            if (nextItem)
                nextItem->delta += thisItem->delta;
            alUnlink(thisNode);
            alLink(thisNode, &evtq->freeList);
#ifdef DES_DEBUG	
			evtq->eventCount--;
#endif
        }
		thisNode = nextNode;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alEvtqFlushType
// Desc:	Removes events of a certain type from an existing queue
// In:		Ptr to the event queue to be flushed
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
void alEvtqFlushType(ALEventQueue *evtq, Sint16 type)
{
	ALLink              *thisNode;
	ALLink              *nextNode;
	ALEventListItem     *thisItem, *nextItem;
    ALEvent				*thisEvent;

	thisNode = evtq->allocList.next;
	while (thisNode != 0) {
		nextNode = thisNode->next;
        thisItem = (ALEventListItem *)thisNode;
        nextItem = (ALEventListItem *)nextNode;
		thisEvent = (ALEvent *)&thisItem->evt;
        if (thisEvent->type == type) {
            if (nextItem)
                nextItem->delta += thisItem->delta;
            alUnlink(thisNode);
            alLink(thisNode, &evtq->freeList);
#ifdef DES_DEBUG	
			evtq->eventCount--;
#endif
        }
		thisNode = nextNode;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alEvtqFlushStateType
// Desc:	Removes events of a certain type from an existing queue
// In:		Ptr to the event queue to be flushed
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
void alEvtqFlushStateType(ALEventQueue *evtq, ALVoiceState *state, Sint16 type)
{
	ALLink              *thisNode;
	ALLink              *nextNode;
	ALEventListItem     *thisItem, *nextItem;
    ALEvent				*thisEvent;

	thisNode = evtq->allocList.next;
	while (thisNode != 0) {
		nextNode = thisNode->next;
        thisItem = (ALEventListItem *)thisNode;
        nextItem = (ALEventListItem *)nextNode;
		thisEvent = (ALEvent *)&thisItem->evt;
        if ((thisEvent->msg.state == state) && (thisEvent->type == type)) {
            if (nextItem)
                nextItem->delta += thisItem->delta;
            alUnlink(thisNode);
            alLink(thisNode, &evtq->freeList);
#ifdef DES_DEBUG	
			evtq->eventCount--;
#endif
        }
		thisNode = nextNode;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alPostEvent
// Desc:	
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
void alPostEvent(ALEventQueue *evtq, Sint16 type, ALVoiceState *state)
{
    ALEvent evt;

    evt.type      = type;
    evt.msg.state = state;
	evt.data1     = 0;
    alEvtqPostEvent(evtq, &evt, 0);
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alPostEventTime
// Desc:	
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
void alPostEventTime(ALEventQueue *evtq, Sint16 type, ALVoiceState *state, Sint32 time)
{
    ALEvent evt;

    evt.type      = type;
    evt.msg.state = state;
	evt.data1     = 0;
    alEvtqPostEvent(evtq, &evt, time);
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alPostEventValue
// Desc:	
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
void alPostEventValue(ALEventQueue *evtq, Sint16 type, ALVoiceState *state, Sint32 value)
{
    ALEvent evt;

    evt.type      = type;
    evt.data1     = value;
    evt.msg.state = state;
    alEvtqPostEvent(evtq, &evt, 0);
}

////////////////////////////////////////////////////////////////////////////////
// Func:	_alSynCollectPVoices
// Desc:	Executes calls to the SPU registers in one swoop
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
void _alSynCollectPVoices(ALSynth *drvr) 
{
	ALLink		*dl;
	ALPVoice	*pv;

	while ((dl = drvr->usedList.next) != 0) {
		pv = (ALPVoice *)dl;
		alUnlink(dl);
		alLink(dl, &drvr->freeList);        
	}
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alSynReserveVoice
// Desc:	Sets aside a SPU voice 
// In:		
// Out:		
// Cost:	
// Notes:	For some strange reason that I have yet to assertain, my SPU stream
//			system only works on voice 24. so this call unlinks the voice from 
//			the voice manager so that only the stream player can use it
////////////////////////////////////////////////////////////////////////////////
ALPVoice * alSynReserveVoice(ALSynth *drvr, Uint32 voice) 
{
	ALLink *dl;

	for (dl = drvr->freeList.next; dl != 0; dl = dl->next) {
		ALPVoice *pv = (ALPVoice *)dl;
		if (pv->offset == voice) {
			alUnlink(dl);
			return pv;
		}
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alSynAddPlayer
// Desc:	
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
void alSynAddPlayer(ALSynth *drvr, ALPlayer *client, Uint32 postdraw)
{
    client->next	 = drvr->head;
    drvr->head		 = client;
	client->postdraw = postdraw;
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alSynNew
// Desc:	Clears an event queue
// In:		Ptr to the event queue to be flushed
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
void alSynNew(ALSynth *drvr, ALSynConfig *c)
{
    Sint32		i, j;
    ALPVoice	*pvoices;

    drvr->head              = 0;
    drvr->numPhysicalVoices = c->maxPVoices;

	//
	// Build the physical voice lists
	//
    drvr->freeList.next = drvr->freeList.prev = 0;
    drvr->usedList.next = drvr->usedList.prev = 0;
    drvr->allocList.next = drvr->allocList.prev = 0;

	pvoices = cxalloc(c->maxPVoices * sizeof(ALPVoice));
    for (i = 0; i < c->maxPVoices; i++) {
		ALPVoice *pv = &pvoices[i];
        alLink((ALLink *)pv, &drvr->freeList);
		pv->vvoice = 0;
		pv->offset = i;
		pv->stolen = 0;
	}

	drvr->timer		  = 0;

	drvr->dmaFlags    = 0;
	drvr->bankSelect  = AL_AUDIO_BANK_FRONTEND;
	
	// Read and set up mono/stereo setting
	syCfgGetSoundMode (&drvr->speakerType);
	acSystemSetStereoOrMono(drvr->speakerType);
	
	drvr->sfxVolume   = 128;
	drvr->cdVolume    = 100;
	drvr->midiVolume  = 100;

	drvr->voiceMaskOnLoopHi	= 0;
	drvr->voiceMaskOnLoopLo	= 0;
	drvr->voiceMaskOnHi		= 0;
	drvr->voiceMaskOnLo		= 0;
	drvr->voiceMaskOffHi	= 0;
	drvr->voiceMaskOffLo	= 0;
	drvr->fxCurrent			= 0;
	drvr->fxSendLevel		= 0;
	drvr->fxType			= 0;

#ifdef _AL_STRM_PLAYER_		// Init the stream environment if needed	
	drvr->streamSize    = (c->streamSize);
	drvr->mainRamSize	= (c->mainRamSize);
	drvr->maxStreams    = (c->maxStreams<<1) + 1;	// <<1 for stereo, +1 for IRQ channel
	drvr->streamsLoaded = 0;

	if (drvr->streamSize) {
		ALStream *strm;
		Uint32 mramBuffer, value = 0x80808080L;

		drvr->streams = (ALStream **)cxalloc(drvr->maxStreams * sizeof(ALStream *));		

		drvr->vblankFlag = KTFALSE;

		for (i = 0; i<drvr->maxStreams; i++) {
			drvr->streams[i] = (ALStream *)cxalloc(sizeof(ALStream));
			pvoices = alSynReserveVoice(drvr, i);
			drvr->streams[i]->pvoice		=  pvoices;
			drvr->streams[i]->channel		=  pvoices->offset;
			drvr->streams[i]->sfile			=  0;
			drvr->streams[i]->statusFlags	=  0;
			if (!i) {
				drvr->streams[i]->sramStart			= (Uint32)alMalloc(STRM_IRQ_SIZE);
				drvr->streams[i]->sramPlayPos		= STRM_IRQ_SIZE;
				drvr->streams[i]->sramWritePos		= 0;
				drvr->streams[i]->sramUsedSize		= 0;
				drvr->streams[i]->cdReadLen			= 0;
				drvr->streams[i]->cdReadPos			= 0;
				drvr->streams[i]->interrupts		= 0;
				drvr->streams[i]->dmaRequest		= 0;
				drvr->streams[i]->mramSize			= 0;

				for (j = 0; j<STRM_MAINRAM_BUFFERS; j++)
					drvr->streams[i]->mramUsedSize[j] = 0;

				mramBuffer = drvr->streams[i]->sramStart;
				
				for (j = 0; j < STRM_IRQ_SIZE; j+=sizeof(Uint32)) {
					acG2WriteLong((volatile KTU32 *)(mramBuffer+j), value);
				}

			} else {
				drvr->streams[i]->sramStart	= (Uint32)alMalloc(drvr->streamSize);
				drvr->streams[i]->mramSize	= drvr->mainRamSize / STRM_MAINRAM_BUFFERS;
			}
		}

		value = (drvr->mainRamSize/STRM_MAINRAM_BUFFERS) * (drvr->maxStreams-1);

		for (i = 0; i<STRM_MAINRAM_BUFFERS; i++) {
			mramBuffer = (Uint32)cxalloc (value);
			
			// First main ram buffer
			for (j = 1; j<drvr->maxStreams; j++) {
				strm = drvr->streams[j];
			 	strm->mramStart[i] = (mramBuffer + ((j-1) * strm->mramSize));
			}
		}
	}

	//
	// Initialise some useful vars
	//
#ifdef _AL_SND_PLAYER_
	if (c->maxBanks) {
		drvr->sfxCount = (Uint32 *)cxalloc(c->maxBanks * sizeof(Uint32));
		drvr->spuAddr  = (Uint32 *)cxalloc(c->maxBanks * sizeof(Uint32));
		drvr->dramAddr = (ALSound **)cxalloc(c->maxBanks * sizeof(ALSound *));
		for (i = 0; i<c->maxBanks; i++) {
			drvr->sfxCount[i] = 0;
			drvr->spuAddr [i] = 0;
			drvr->dramAddr[i] = 0;
		}
	}
#endif


#endif
}

////////////////////////////////////////////////////////////////////////////////
// Func:	_alSynHandler
// Desc:	Executes calls to the SPU registers in one swoop
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
void _alSynHandler(ALSynth *drvr)
{
	if (drvr->voiceMaskOffHi || drvr->voiceMaskOffLo) {
		if (acDigiMultiStop(drvr->voiceMaskOffHi, drvr->voiceMaskOffLo)) {
			drvr->voiceMaskOffHi = 0;
			drvr->voiceMaskOffLo = 0;
		}
	}

	if (drvr->voiceMaskOnHi || drvr->voiceMaskOnLo) {
		if (acDigiMultiPlay(AC_LOOP_OFF, drvr->voiceMaskOnHi, drvr->voiceMaskOnLo)) {
			drvr->voiceMaskOnHi = 0;
			drvr->voiceMaskOnLo = 0;
		}
	}
	
	if (drvr->voiceMaskOnLoopHi || drvr->voiceMaskOnLoopLo) {
		if (acDigiMultiPlay(AC_LOOP_ON, drvr->voiceMaskOnLoopHi, drvr->voiceMaskOnLoopLo)) {
			drvr->voiceMaskOnLoopHi = 0;
			drvr->voiceMaskOnLoopLo = 0;
		}
	}

	drvr->timer++;

	// Collect free physical voices
    _alSynCollectPVoices(drvr); 
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alAudioFrame
// Desc:	Peforms pending actions on each Player
// In:		
// Out:		
// Cost:	
// Notes:	Called once per frame
////////////////////////////////////////////////////////////////////////////////
void alAudioFrame(ALSynth *drvr, Uint32 postdraw)
{
	ALPlayer *client;
		
	if ((!drvr) || (drvr->head == 0)) {	// Is there anything to do?
		return;
    }    

	//
	// Run through all players setting the voice attributes
	//
	for (client = drvr->head; client != 0; client = client->next) {
		if (postdraw) {
			if (client->postdraw) {
				(*client->handler)(client);
			}
		} else {
			if (!client->postdraw) {
				(*client->handler)(client);
			}
		}
    }

	//
	// Execute the commands to the SPU registers
	//
	if (!postdraw) {
		_alSynHandler(drvr);
	}
}

Sint32 _alAllocVoice(ALSynth	*drvr, ALPVoice **pvoice, Sint16 priority)
{		
	ALLink		*dl;
	Sint32		stolen = 0;

	if ((dl = drvr->usedList.next) != 0) {			// check the lame list first
		*pvoice = (ALPVoice *) dl;
        alUnlink(dl);
        alLink(dl, &drvr->allocList);        
    } else if ((dl = drvr->freeList.next) != 0) {	// then from the free list
        *pvoice = (ALPVoice *) dl;
        alUnlink(dl);
        alLink(dl, &drvr->allocList);        
    } else {										// else steal one 
//        for (dl = drvr->allocList.next; dl != 0; dl = dl->next) {
//            ALPVoice *pv = (ALPVoice *)dl;
//            if (pv->vvoice->priority <= priority) {
//				*pvoice		= pv;
//                priority	= pv->vvoice->priority;
//                stolen		= 1;
//            }
//        }
    }
	return stolen;
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alSynAllocVoice
// Desc:	Allocates a free SPU voice
// In:		
// Out:		
// Cost:	
// Notes:	Stealing is not implemented correctly yet
////////////////////////////////////////////////////////////////////////////////
Sint32 alSynAllocVoice(ALSynth *drvr, ALVoice *voice, Sint16 priority)
{
	ALPVoice	*pvoice = 0;
    Sint32		stolen;
    
	voice->priority     = priority;
	voice->state        = AL_STATE_STOPPED;        
	voice->pvoice       = 0;

	stolen = _alAllocVoice(drvr, &pvoice, priority);

	// If we were able to allocate a voice
	if (pvoice) {    
        if (stolen) {	// Stop stolen voice
            pvoice->stolen = 1;
            pvoice->vvoice->pvoice = 0;		// zero stolen voice
        } else {
            pvoice->stolen = 0;
        }
            
        pvoice->vvoice = voice;
        voice->pvoice  = pvoice;
    }
    
    return (Sint32)(pvoice != 0);    
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alSynFreeVoice
// Desc:	Deallocates a SPU voice
// In:		
// Out:		
// Cost:	
// Notes:	Stealing is not implemented correctly yet
////////////////////////////////////////////////////////////////////////////////
void alSynFreeVoice(ALSynth *drvr, ALVoice *voice)
{
	ALPVoice *pvoice;
	if (voice->pvoice) {
		pvoice = voice->pvoice;
		alUnlink((ALLink *)pvoice);
		alLink((ALLink *)pvoice, &drvr->usedList);
        voice->pvoice = 0;
    }
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alSynStartVoice
// Desc:	Sets the Voice playing
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
void alSynStartVoice(ALSynth *drvr, ALPVoice *pv, Uint32 loop)
{
    if (pv) {
		if (loop) {
			if (pv->offset & 0x20) {
				drvr->voiceMaskOnLoopHi |= SPU_VOICECH(pv->offset & 0x1f);
			} else {
				drvr->voiceMaskOnLoopLo |= SPU_VOICECH(pv->offset);
			}
		} else {
			if (pv->offset & 0x20) {
				drvr->voiceMaskOnHi     |= SPU_VOICECH(pv->offset & 0x1f);
			} else {
				drvr->voiceMaskOnLo     |= SPU_VOICECH(pv->offset);
			}
		}
    }
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alSynStopVoice
// Desc:	Stops the Voice playing
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
Sint32 alSynStopVoice(ALSynth *drvr, ALPVoice *pv)
{
	Uint32 voiceMask;
    if (pv) {
		if (pv->offset & 0x20) {
			voiceMask = SPU_VOICECH(pv->offset & 0x1f);
			if (voiceMask & drvr->voiceMaskOnHi) {
				drvr->voiceMaskOnHi &= ~voiceMask;
				return 0;
			} else if (voiceMask & drvr->voiceMaskOnLoopHi) {
				drvr->voiceMaskOnLoopHi &= ~voiceMask;
				return 0;
			} else {
				drvr->voiceMaskOffHi |= voiceMask;
			}
		} else {
			voiceMask = SPU_VOICECH(pv->offset);
			if (voiceMask & drvr->voiceMaskOnLo) {
				drvr->voiceMaskOnLo &= ~voiceMask;
				return 0;
			} else if (voiceMask & drvr->voiceMaskOnLoopLo) {
				drvr->voiceMaskOnLoopLo &= ~voiceMask;
				return 0;
			} else {
				drvr->voiceMaskOffLo |= voiceMask;
			}
		}
    }
	return 1;
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alSynSetVolume
// Desc:	Sets the volume for a physical voice
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
Sint32 alSynSetVolume(ALPVoice *pv, KTU32 volume)
{
    if (pv) {
		return (Sint32)acDigiSetVolume(pv->offset, (KTU32)volumeLookUp[volume]);
    }
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alSynSetDspMixerLevel
// Desc:	Sets the effects for a voice
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
Sint32 alSynSetDspMixerLevel(ALPVoice *pv, Uint32 channel, Uint32 send, Uint32 direct)
{
    if (pv) {
		return (Sint32)acDspSetMixerChannel(pv->offset, channel, send, direct);
    }
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alSynSetFilterEnvelope
// Desc:	Sets the effects for a voice
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
Sint32 alSynSetFilterEnvelope(ALPVoice *pv, Uint32 on_off)
{
    if (pv) {
		return (Sint32)acDigiSetFilterEnvelope(pv->offset, on_off, 8, 8194, 5982, 5982, 6680, 8, 14, 18, 0, 17);
    }
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alSynSetPan
// Desc:	Sets the volume for a physical voice
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
Sint32 alSynSetPan(ALPVoice *pv, KTU32 pan)
{
    if (pv) {
		return (Sint32)acDigiSetPan(pv->offset, pan);
    }
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alSynSetPitch
// Desc:	Sets the pitch for a physical voice
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
Sint32 alSynSetPitch(ALPVoice *pv, KTS32 pitch)
{
    if (pv) {
		return (Sint32)acDigiSetSampleRate(pv->offset, pitch);
    }
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alSynOpenPort
// Desc:	Sets the start address for a physical voice
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
Sint32 alSynOpenPort(ALPVoice *pv, KTU32 addr, KTU32 size, AC_AUDIO_TYPE type, KTS32 rate)
{
    if (pv) {
		return (Sint32)acDigiOpen(pv->offset, addr, size, type, rate);
    }
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alSynSetStartAddress
// Desc:	Sets the start address for a physical voice
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
Sint32 alSynSetLoopPoints(ALPVoice *pv, Uint32 start, Uint32 end)
{
    if (pv) {
		return (Sint32)acDigiSetLoopPoints(pv->offset, start, end);
    }
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alSynSetPitch
// Desc:	Sets the pitch for a physical voice
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
Sint32 alSynClosePort(ALPVoice *pv)
{
    if (pv) {
		return (Sint32)acDigiClose(pv->offset);
    }
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#ifdef _AL_STRM_PLAYER_

////////////////////////////////////////////////////////////////////////////////
// Func:	alStreamSetVolume
// Desc:	
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
void alStreamSetVolume(ALVoiceState *state, KTU32 volume)
{
	ALVoice *voice = &state->voice;
	if (state->state != AL_STATE_STOPPED) {
		alSynSetVolume(voice->pvoice, volume);
	}
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alStreamMixVolume
// Desc:	Sets this stream volume to volume and the others to 0
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
void alStreamMixVolume(KTU32 channels, KTU32 volume)
{
	Uint32 i, maxStreams = alStreamPlayer->maxStreams;
	ALVoiceState *strmState = alStreamPlayer->strmState;
	ALEvent evt;

	alEvtqFlushType(&alStreamPlayer->evtq, AL_EVENT_VOLUME);
	evt.type = AL_EVENT_VOLUME;
	for (i = 1; i<maxStreams; i++) {
		if (SPU_VOICECH(i) & channels) {
			strmState[i].acVolume = volume;
		} else {
			strmState[i].acVolume = 0;
		}
		evt.msg.state = &strmState[i];
		alEvtqPostEvent(&alStreamPlayer->evtq, &evt, 0);
	}
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alStreamBuffersFull
// Desc:	
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
Uint32 alStreamBuffersFull(ALStrmPlayer *player, Uint32 buffer)
{
	Uint32	i, maxStreams = player->maxStreams;
	Uint32	count = 0;
	for (i = 1; i<maxStreams; i++) {
		if (player->strmState[i].stream) {
			if (player->strmState[i].stream->mramUsedSize[buffer]) {
				count++;
			}
		}
	}
	return count;
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alStreamProcessChunk
// Desc:	
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
void alStreamProcessChunk(void *obj)
{
	ALVoiceState *	sState = alStreamPlayer->strmState;
	Uint32			i, maxStreams = alStreamPlayer->maxStreams;

	for (i = 1; i < maxStreams; i++) {

		ALStream *strm = sState[i].stream;

		if (strm == 0) continue;
		
		strm->cdReadPos++;
		strm->mramUsedSize[strm->mramBuffer] += (strm->readSize);
		strm->mramBuffer = (strm->mramBuffer + 1) % STRM_MAINRAM_BUFFERS;
	}

	alStreamPlayer->flags &= ~AF_STRM_CDREAD;
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alStreamSeek
// Desc:	
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
void alStreamSeek(ALStrmPlayer *player, ACFILE fp, ALVoiceState *state)
{
	ALStream * strm = state->stream;

	if (!alAsyncFileGetTransferStatus(fp) || 
		alStreamBuffersFull(player, strm->mramBuffer)) {
		return;
	}
	
	if (strm->cdReadPos == strm->cdReadLen) {
		Uint32 i, maxStreams = player->maxStreams;
		alAsyncFileSeek(fp, 1);
		for (i = 1; i<maxStreams; i++) {
			if (player->strmState[i].stream) {
				player->strmState[i].stream->cdReadPos = 1;
			}
		}
		return;
	}

	player->flags |= AF_STRM_CDREAD;
	alAsyncFileRead(fp, (Uint8 *)strm->mramStart[strm->mramBuffer], alStreamPlayer->readLength, 1);
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alAllocRequest
// Desc:	
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
void alAllocDMARequest(ALStream *stream, AC_TRANSFER_REQUEST **request)
{
	KTU32 i, imask = acCkset_imask(AC_CRITICAL_IML);
	for (i = 0; i < AC_MAX_TRANSFER_QUEUE; i++) {
		if (!dmaPool[i].stream) {
			dmaPool[i].stream = (Uint32)stream;
			*request = (AC_TRANSFER_REQUEST *)&dmaPool[i];
			set_imask(imask);
			return;
		}
	}
	set_imask(imask);
	alPrintf("DMA request failed\n");
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alFreeDMARequest
// Desc:	
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
void alFreeDMARequest(AC_TRANSFER_REQUEST *transfer)
{
	KTU32		imask		= acCkset_imask(AC_CRITICAL_IML);
	ALStream *	strm		= (ALStream *)((ALStreamDMAPool *)transfer)->stream;

	if (transfer->status == AC_TRANSFER_COMPLETE) {
		if (alStreamPlayer->flags & AF_STRM_INTERRUPT) {
			strm->mramUsedSize[strm->sramBuffer] -= transfer->size;
			
			if (strm->mramUsedSize[strm->sramBuffer] == 0)
				strm->sramBuffer = (strm->sramBuffer + 1) % STRM_MAINRAM_BUFFERS;
			
			strm->sramWritePos  = (strm->sramWritePos + transfer->size) % STRM_SNDRAM_SIZE;
			strm->sramUsedSize += transfer->size;
		}
	} else {
		strm->dmaRequest   += transfer->size;
	}

	transfer->aicaMem	= 0;
	transfer->sh4Mem	= 0;
	transfer->size		= 0;
	transfer->direction = 0;
	transfer->status	= AC_NO_ERROR;
	transfer->callback	= KTNULL;

	((ALStreamDMAPool *)transfer)->stream = 0;

	set_imask(imask);
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alStreamStartDMA
// Desc:	
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
void alStreamStartDMA(ALVoiceState *state, Sint32 size)
{
	ALStream *strm = state->stream;
	AC_TRANSFER_REQUEST *request;
	
	if (!strm->mramUsedSize[strm->sramBuffer])
		return;
	if ((strm->sramUsedSize+size) > STRM_SNDRAM_SIZE)
		return;
	
//	if (state->acVolume) {
		alAllocDMARequest(strm, &request);
		
		request->aicaMem	= (KTU32 *)(strm->sramStart + strm->sramWritePos);
		request->sh4Mem		= (KTU32 *)(strm->mramStart[strm->sramBuffer]);
		request->size		= size;
		request->direction	= 0;
		request->status		= AC_NO_ERROR;
		request->callback	= alFreeDMARequest;

		strm->dmaRequest   -= size;

		if (!acTransfer(request)) {
			alFreeDMARequest(request);
			return;
		}
//	}
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alStreamCallbackHandler
// Desc:	
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
void alStreamCallbackHandler(volatile KTU32 messageNumber)
{
	KTU32 i, maxStreams = alStreamPlayer->maxStreams;
	ALVoiceState *sState = alStreamPlayer->strmState;

	sState[0].stream->interrupts++;
	if (!acDigiRequestEvent(sState[0].stream->channel, STRM_IRQ_SIZE)) {
		sState[0].flags |= AF_STRM_INTERRUPT;
	}

	for (i = 1; i<maxStreams; i++) {
		ALStream *strm = sState[i].stream;
		if (!strm) continue;
		if (strm->statusFlags & AF_STRM_STARTED) {

			if (!strm->interrupts)
				strm->statusFlags |= AF_STRM_FINISHED;
			else
				strm->interrupts--;			

			strm->sramLastPlayPos = strm->sramPlayPos;
			acDigiGetCurrentReadPosition(strm->channel, (Uint32 *)&strm->sramPlayPos);
		
			if (strm->sramLastPlayPos <= strm->sramPlayPos)
				strm->sramPlayedSize = strm->sramPlayPos - strm->sramLastPlayPos;
			else 
				strm->sramPlayedSize = (STRM_SNDRAM_SIZE<<1) - strm->sramLastPlayPos + strm->sramPlayPos;

			strm->sramPlayedSize >>= 1;

			strm->dmaRequest   += strm->sramPlayedSize;
			strm->sramUsedSize -= strm->sramPlayedSize;
			if (strm->sramUsedSize <= 0)
				strm->sramUsedSize  = 0;
		} 
	}

	if (alStreamPlayer->voiceStateHi || alStreamPlayer->voiceStateLo) {

		i = syTmrCountToMicro(syTmrDiffCount(alStreamPlayer->currentTime, syTmrGetCount()));

		if (i >= alStreamPlayer->currentTempo) {

			i = i - alStreamPlayer->currentTempo;
			i = syTmrCountToMicro(syTmrGetCount()) - i;
		 	alStreamPlayer->currentTime = syTmrMicroToCount(i);

			alStreamPlayer->currentBeat++;
			if (alStreamPlayer->currentBeat == 5) {
				alStreamPlayer->currentBeat = 1;
				alStreamPlayer->currentBar++;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alStreamStop
// Desc:	
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
void alStreamStop(void)
{
	Uint32			i, maxStreams = alStreamPlayer->maxStreams;
	ALVoiceState *	state, *strmState = alStreamPlayer->strmState;
	ALStream *		strm;
	ALEvent			evt;

	alEvtqFlush(&alStreamPlayer->evtq);

	for (i = 1; i<maxStreams; i++) {
		state = &strmState[i];

		alEvtqFlushState(&alStreamPlayer->evtq, state);

		if (state->state != AL_STATE_STOPPED) {

			strm = state->stream;
			if (!strm) continue;

			strm->statusFlags = 0;
				
			if (alStreamPlayer->target == state->offset) {
				acDigiRequestEvent(0, 0);
				acDigiMultiStop(alStreamPlayer->voiceMaskHi, alStreamPlayer->voiceMaskLo);
			}
				
			alSynClosePort(strm->pvoice);
				
			if (strm->channel & 0x20) {
				alStreamPlayer->voiceStateHi &= ~SPU_VOICECH(strm->channel & 0x1f);
				alStreamPlayer->voiceMaskHi  &= ~SPU_VOICECH(strm->channel & 0x1f);
			} else {
				alStreamPlayer->voiceStateLo &= ~SPU_VOICECH(strm->channel);
				alStreamPlayer->voiceMaskLo  &= ~SPU_VOICECH(strm->channel);
			}
								
			alStrmpDeAllocateStream(strm);
		}
			
		state->stream     =  0;	// deallocate the sound
		state->strat      =  0;
		state->state      =  AL_STATE_STOPPED;			
	}

	alStreamPlayer->flags &= ~AF_STRM_CDREAD;

	for (i = 1; i<maxStreams; i++) {
		alGlobals->drvr.streams[i]->statusFlags	= 0;
	}

	// Start responding to API events
	evt.type = AL_EVENT_API;
	evt.msg.state = 0;
	alEvtqPostEvent(&alStreamPlayer->evtq, (ALEvent *)&evt, alStreamPlayer->frameTime);
	alStreamPlayer->nextDelta = alEvtqNextEvent(&alStreamPlayer->evtq, &alStreamPlayer->nextEvent);
}

void alStrmpHandleEvent(ALStrmPlayer *player, ALEvent *event) 
{
	ALVoiceState *	state =  event->msg.state;
	ALStream	 *	strm =  state->stream;	

	if (player->strmState[0].flags & AF_STRM_INTERRUPT) {
		if (acDigiRequestEvent(player->strmState[0].stream->channel, STRM_IRQ_SIZE)) {
			player->strmState[0].flags &= ~AF_STRM_INTERRUPT;
		}
	}

	switch (event->type) {
	case AL_EVENT_PLAY:
	{
		ALVoice *voice = &state->voice;
		
		if (state->state != AL_STATE_STOPPED) {
			alPrintf("STREAM HAS NOT BEEN STOPPED\n");
			return;
		}

		strm->pvoice->vvoice = voice;
		voice->pvoice = strm->pvoice;

		if (!alSynOpenPort(voice->pvoice, strm->sramStart, STRM_SNDRAM_SIZE, AC_ADPCM_LOOP, STRM_SAMPLE_RATE)) {
			if (gAcError.number == AC_CANT_SEND_DRIVER_COMMAND) {
				alEvtqPostEvent(&player->evtq, event, player->frameTime);
			}
			return;
		}

		alSynSetVolume(voice->pvoice, state->acVolume);

		if (state->offset & 1) {
			alSynSetPan(voice->pvoice, AC_PAN_LEFT);
		} else {
			alSynSetPan(voice->pvoice, AC_PAN_RIGHT);
		}

		state->state = AL_STATE_PLAYING;
		event->type  = AL_EVENT_READY;
		alEvtqPostEvent(&player->evtq, event, player->frameTime);
	}
	break;
	case AL_EVENT_READY:
		if (strm->mramUsedSize[strm->sramBuffer]) {
			alStreamStartDMA(state, STRM_SECTOR_SIZE);
			
			if (strm->sramUsedSize == (STRM_SNDRAM_SIZE>>1)) {
				player->voiceStateLo |= SPU_VOICECH(strm->channel);
				if (state->offset == player->target)
					event->type = AL_EVENT_DECAY;
				else return;
			}
		}		
		alEvtqPostEvent(&player->evtq, event, player->frameTime);
	break;
	case AL_EVENT_DECAY:
	{		 
		Uint32 pos, i, maxStreams = player->maxStreams;

		if (player->voiceMaskLo != player->voiceStateLo) {
			alEvtqPostEvent(&player->evtq, event, player->frameTime);
			return;
		}

		for (i = 1; i<maxStreams; i++) {
			state = &player->strmState[i];
			strm = state->stream;
			
			if (!strm) continue;
			
			if (!(strm->statusFlags & AF_STRM_STARTED)) {
				if (player->target == state->offset) {
					event->msg.state = state;

					if (!acDigiRequestEvent(0, STRM_IRQ_SIZE)) {
						if (gAcError.number == AC_CANT_SEND_DRIVER_COMMAND) {
							alEvtqPostEvent(&player->evtq, event, player->frameTime);
						}
						return;
					}

					if (!acDigiMultiPlay(AC_LOOP_ON, player->voiceMaskHi, player->voiceMaskLo)) {
						if (gAcError.number == AC_CANT_SEND_DRIVER_COMMAND) {
							alEvtqPostEvent(&player->evtq, event, player->frameTime);
						}
						return;
					}

					alEvtqPostEvent(&player->evtq, event, player->frameTime);
				}
				strm->statusFlags  |= AF_STRM_STARTED;
				continue;
			}

			if (strm->dmaRequest >= STRM_SECTOR_SIZE) {
				alStreamStartDMA(state, STRM_SECTOR_SIZE);
			}

			if (strm->statusFlags & AF_STRM_FINISHED) {
				if (strm->sfile->flags & AF_STRM_LOOPED) {
					strm->interrupts   =  player->headerBuffer->interruptsTillEnd - 1;
					strm->statusFlags &= ~AF_STRM_FINISHED;
					if (player->target == state->offset) {
						player->currentBar = player->currentBeat = 1;
						event->msg.state = state;
						alEvtqPostEvent(&player->evtq, event, player->frameTime);
					}
					// Set the loop end point for the last buffer of the track
//					alSynSetLoopPoints(voice->pvoice, strm->sramStart, strm->sramStart+STRM_SNDRAM_SIZE);
				} else {
					//alSynSetVolume(voice->pvoice, 0);
					event->type = AL_EVENT_STOP;
					event->msg.state = state;
					alEvtqPostEvent(&player->evtq, event, player->frameTime);
				}
				continue;
			}
			
			if (player->target == state->offset) {
				event->msg.state = state;
				alEvtqPostEvent(&player->evtq, event, player->frameTime);
			}
		}
	}
	break;
	case AL_EVENT_CDSEEK:
		if (strm) {
			alEvtqPostEvent(&player->evtq, event, player->frameTime);
			alStreamSeek(player, strm->sfile->fp, state);
		}
	break;
	case AL_EVENT_VOLUME:
	{
		ALVoice *voice = &state->voice;
		if (state->state != AL_STATE_STOPPED) {
			if (!alSynSetVolume(voice->pvoice, state->acVolume)) {
				alEvtqPostEvent(&player->evtq, event, player->frameTime);
			}
		}
	}
	break;
	case AL_EVENT_STOP:
		if (state->state != AL_STATE_STOPPED) {

			if (player->target == state->offset) {
				acDigiRequestEvent(0, 0);
				if (!acDigiMultiStop(player->voiceMaskHi, player->voiceMaskLo)) {
					if (gAcError.number == AC_CANT_SEND_DRIVER_COMMAND) {
						alEvtqPostEvent(&player->evtq, event, player->frameTime);
						return;
					}
				}
			}
			
			if (!alSynClosePort(strm->pvoice)) {
				if (gAcError.number == AC_CANT_SEND_DRIVER_COMMAND) {
					alEvtqPostEvent(&player->evtq, event, player->frameTime);
					return;
				}
			}

			strm->statusFlags &= ~AF_STRM_STARTED;
		}
	// Fall through	
	case AL_EVENT_END:
		if (state->state != AL_STATE_STOPPED) {
			alEvtqFlushState(&player->evtq, state);

			player->voiceMaskLo   &= ~SPU_VOICECH(strm->channel);
			player->voiceStateLo  &= ~SPU_VOICECH(strm->channel);
			
			player->currentStream = 0;
			
    		if (player->target == state->offset) {
				player->flags &= ~AF_STRM_CDREAD;
				alAsyncFileClose(strm->sfile->fp);
			}

			alStrmpDeAllocateStream(strm);
			
			state->stream     =  0;	// deallocate the sound
			state->strat      =  0;
			state->state      =  AL_STATE_STOPPED;			
		}
	break;
	case (AL_EVENT_FLUSH):
		alEvtqFlushState(&player->evtq, state);
	break;
	default: break;
	}
}

Sint32 _alStrmVoiceHandler( void *node )
{
	ALStrmPlayer *	player = (ALStrmPlayer *) node;

	do {
		switch (player->nextEvent.type) {
            case (AL_EVENT_API):
				alEvtqPostEvent(&player->evtq, &player->nextEvent, player->frameTime);
            	break;

            default:
				alStrmpHandleEvent(player, &player->nextEvent);
                break;
        }                
    } while (alEvtqNextEvent(&player->evtq, &player->nextEvent) == 0);

#ifdef DES_DEBUG
	tPrintf(0, 1, "Msgs %d, Max %d %d,%d, %d,%d", player->evtq.eventCount, player->evtq.maxEventCount, player->currentBar, player->currentBeat, player->currentDanger, player->currentStream);
#endif

    return player->nextDelta;
}

void alStrmpNew(ALStrmPlayer *player, ALStrmpConfig *c) 
{    
	ALSynth			*drvr = &alGlobals->drvr;
	ALVoiceState	*streamState;
	Uint8			*ptr;
	ALEvent			evt;
	Uint32	   		i;

	player->maxStreamsFiles		=	(c->maxStreamFiles);
	player->maxStreams			=	(c->maxStreams<<1) + 1;	// <<1 for stereo, +1 for IRQ
	player->frameTime			=	(c->frameTime);
	player->maxDistance			=	(c->maxDistance);
	player->mainVolume			=   128;
	player->target				=  -1;
	streamState					= (ALVoiceState *)cxalloc(player->maxStreams * sizeof(ALVoiceState));
	player->strmState			=	streamState;
	player->com					=	0;
	player->flags				=	0;
	player->voiceMaskHi			=	0;
	player->voiceMaskLo			=	0;
	player->voiceStateHi		=	0;
	player->voiceStateLo		=	0;
	player->streamFiles			=	0;
	player->currentTrackId		=	0;
	player->currentIntensity	=	0;
	player->currentDanger		=	0;
	player->currentStream		=	0;
	player->currentBar			=	1;
	player->currentBeat			=	1;


#if 0	// Stream File Table is declared Globally
	player->streamFiles = (ALStreamFile **)cxalloc(c->maxStreamFiles * sizeof(ALStreamFile *));		

	for (i = 0; i < c->maxStreamFiles; i++) {
		player->streamFiles[i] = (ALStreamFile *)cxalloc(sizeof(ALStreamFile));	
		player->streamFiles[i]->numEntries = 0;
	}
#endif

	player->headerBuffer = (AM_STREAM_FILE_HEADER *)cxalloc(STRM_SECTOR_SIZE);

	for (i = 0; i < player->maxStreams; i++) {
		if (!i) {
			ALStream *strm = drvr->streams[i];
			ALVoice *voice = &streamState[i].voice;

			strm->pvoice->vvoice = voice;
			voice->pvoice = strm->pvoice;

			streamState[i].state    = AL_STATE_PLAYING;
			streamState[i].stream   = strm;
			streamState[i].offset   = i;
			streamState[i].acVolume = 0;
   			streamState[i].pitch    = STRM_IRQ_RATE;
			streamState[i].flags    = 0;
			
			alSynOpenPort(strm->pvoice, strm->sramStart, STRM_IRQ_SIZE, AC_ADPCM_LOOP, STRM_IRQ_RATE);

			acSystemEnableArmInterrupts();
			player->flags |= AF_STRM_INTERRUPT;

			if (strm->channel & 0x20) {
				player->voiceMaskHi  |= SPU_VOICECH(strm->channel & 0x1f);
				player->voiceStateHi |= SPU_VOICECH(strm->channel & 0x1f);
			} else {
				player->voiceMaskLo  |= SPU_VOICECH(strm->channel);
				player->voiceStateLo |= SPU_VOICECH(strm->channel);
			}
		} else {
			streamState[i].state    = AL_STATE_STOPPED;
			streamState[i].stream   = 0;
			streamState[i].acVolume = 0;
			streamState[i].flags    = 0;
			streamState[i].offset   = i;
		}
	}
	
	// Init the event queue
	ptr = cxalloc(c->maxEvents * sizeof(ALEventListItem));
	alEvtqNew(&player->evtq, (ALEventListItem *)ptr, c->maxEvents);
    
	// Add ourselves to the driver
	player->drvr			= drvr;
	player->node.next		= NULL;
	player->node.handler	= _alStrmVoiceHandler;
	player->node.clientData	= player;
	alSynAddPlayer(player->drvr, &player->node, 1);

	// Start responding to API events
	evt.type = AL_EVENT_API;
	evt.msg.state = 0;
	alEvtqPostEvent(&player->evtq, (ALEvent *)&evt, player->frameTime);
	player->nextDelta = alEvtqNextEvent(&player->evtq, &player->nextEvent);

	// Add in the Vblank handler
	player->drvr->vblankId = syChainAddHandler(AC_INT_AICA_POLL_CHAIN_ID, 
												alVBlankHandler, 
												AL_FILE_VBLANK_PRIORITY, (void*)AL_FILE_VBLANK_ID);		
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alStrmpAllocateStream
// Desc:	Gets the Sound Header Info from the soundId
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
ALStream * alStrmpAllocateStream(ALStreamFile *sfile, Uint32 track)
{
	ALStream *strm;
	Uint32 i, j, maxStreams = alStreamPlayer->maxStreams;
	Uint32 length;
	
	sfile->size		   -= 1;
	sfile->startSector	= 1;

	length = (sfile->size / alStreamPlayer->headerBuffer->numberOfTracks) >> 12;

	for (i = 1; i < maxStreams; i++) {
		strm = alStreamPlayer->drvr->streams[i];
		if (strm->statusFlags == 0) {

			strm->statusFlags		=  AF_STRM_ALLOCATED;
			strm->sfile				=  sfile;
			strm->sramPlayPos		=  strm->sramWritePos =  0;
			strm->sramUsedSize		=  0;
			strm->dmaRequest		=  STRM_SNDRAM_SIZE;// - STRM_SECTOR_SIZE;
			strm->sramBuffer		=  strm->mramBuffer	= 0;			
			strm->readSize			=  alStreamPlayer->headerBuffer->halfPlayBufferSizeInBytes;
			strm->interrupts		=  alStreamPlayer->headerBuffer->interruptsTillEnd - 1;
			strm->cdReadPos			=  sfile->startSector;
			strm->cdReadLen			= (strm->cdReadPos + length) - STRM_INT_LOOP;
			
			for (j = 0; j<STRM_MAINRAM_BUFFERS; j++)
				strm->mramUsedSize[j] = 0;

			return strm;
		}
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alStrmpDeAllocateStream
// Desc:	
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
void alStrmpDeAllocateStream(ALStream *strm)
{
	strm->statusFlags = 0;
	strm->sfile       = 0;
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alStrmpAllocateVoice
// Desc:	Gets the Sound Header Info from the soundId
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
ALVoiceState * alStrmpAllocateVoice(ALStream *strm, Uint32 volume, Uint16 track, Uint16 loop)
{
    ALVoiceState *	sState = alStreamPlayer->strmState;
	Uint32			i, maxVoices = alStreamPlayer->maxStreams;

	//
	// Find a free voice for the stream
	//
	for (i = 1; i < maxVoices; i++) {		
   		if (!sState[i].stream) {
   			sState[i].stream		= strm;
   			sState[i].state			= AL_STATE_STOPPED;
   			sState[i].loop			= loop;
   			sState[i].pitch			= STRM_SAMPLE_RATE;
   			sState[i].acVolume		= volume;
   			sState[i].channel		= track;
			sState[i].flags			= 0;
			sState[i].fxMix			= 0;
   			return &sState[i];
   		}
	}
	return 0;
}

#endif // _AL_STRM_PLAYER_

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#ifdef _AL_SND_PLAYER_

////////////////////////////////////////////////////////////////////////////////
// Func:	_alSndpHandleEvent
// Desc:	
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
void _alSndpHandleEvent(ALSndPlayer *player, ALEvent *event) 
{
	ALVoiceState*	state =  event->msg.state;
    ALSound		*	snd   =  state->sound;
	ALVoice		*	voice = &state->voice;

	switch (event->type) {

		case (AL_EVENT_PLAY):	
		{            
			if (state->state != AL_STATE_PREPARED) {
				alEvtqFlushState(&player->evtq, state);
				state->sound = 0;
				state->strat = 0;
				return;
			}
							
			if (!alSynAllocVoice(player->drvr, voice, state->priority)) {
				alEvtqFlushState(&player->evtq, state);
				state->sound = 0;
				state->strat = 0;
				return;
			}
			
			state->state      = AL_STATE_PLAYING;
			state->lastVolume = state->acVolume;

			if (!alSynOpenPort(voice->pvoice, snd->asset.offset, snd->asset.size, state->type, snd->sampleRate)) {
				if (gAcError.number != AC_CANT_SEND_DRIVER_COMMAND) {
					event->type = AL_EVENT_END;
				}
				alEvtqPostEvent(&player->evtq, event, player->frameTime);
				return;
			}

			alSynSetPan   (voice->pvoice, state->pan);
			alSynSetPitch (voice->pvoice, state->pitch);
			alSynSetVolume(voice->pvoice, state->acVolume);
			alSynSetDspMixerLevel(voice->pvoice, 0, state->fxMix, AC_MAX_SEND_LEVEL);
			if (state->filter)
				alSynSetFilterEnvelope(voice->pvoice, AC_MAX_SEND_LEVEL);
			alSynStartVoice(player->drvr, voice->pvoice, snd->loop);

			if (state->pitch) {
				// (((Size in 16bit samples) / (16bit samples per sec)) * FPS) + (1 sf)
				if (snd->bitDepth & 4) {
					state->endTime = ((FRAMES_PER_SECOND * (snd->asset.size<<2)) / (state->pitch));
				} else {
					state->endTime = ((FRAMES_PER_SECOND * (snd->asset.size))    / (state->pitch));
				}
			} else {
				state->endTime = player->drvr->timer + FRAMES_PER_SECOND;
			}

			if (state->strat == 0) {
				event->type = AL_EVENT_DECAY;
				alEvtqPostEvent(&player->evtq, event, state->endTime);
			}

			state->endTime = player->drvr->timer + state->endTime + 1;
		}
		break;
		case (AL_EVENT_PAN):	
			if (state->state == AL_STATE_PLAYING) {
				state->pan = event->data1;
				if (player->drvr->speakerType == SYD_CFG_STEREO) {
					alSynSetPan(voice->pvoice, state->pan);
				} else {
					alSynSetPan(voice->pvoice, AC_PAN_CENTER);
				}
			}
		break;
		case (AL_EVENT_VOLUME):
			if (state->state > AL_STATE_PREPARED) {
				if (state->state == AL_STATE_STOPPING) {
					event->data1 = event->data1 - 10;
					if (event->data1 < 0) {
						event->data1 = 0;
						event->type = AL_EVENT_STOP;
					} 	
					alEvtqPostEvent(&player->evtq, event, player->frameTime);
					alSynSetVolume(voice->pvoice, event->data1);
				} else {
					if (!alSynSetVolume(voice->pvoice, event->data1)) {
						if (gAcError.number == AC_CANT_SEND_DRIVER_COMMAND) {
							event->type = AL_EVENT_VOLUME;
							alEvtqPostEvent(&player->evtq, event, player->frameTime);
						}
					} 
				}
				state->lastVolume = state->acVolume;
				state->acVolume   = event->data1;
			}
		break;
		case (AL_EVENT_PITCH):	
			if (state->state == AL_STATE_PLAYING) {
				if (!alSynSetPitch(voice->pvoice, event->data1)) {
					if (gAcError.number == AC_CANT_SEND_DRIVER_COMMAND) {
						alEvtqPostEvent(&player->evtq, event, player->frameTime);
					}
				} else {
					state->lastPitch = state->pitch;
					state->pitch     = event->data1;
				}
			}
		break;
		case (AL_EVENT_DECAY):	
			if (player->drvr->timer > state->endTime) {
				event->type  = AL_EVENT_STOP;
				alEvtqPostEvent(&player->evtq, event, player->frameTime);
				state->state = AL_STATE_STOPPING;
			} else {
				alEvtqPostEvent(&player->evtq, event, player->frameTime);
			}
		break;
		case (AL_EVENT_STOP):	
			if (state->state > AL_STATE_PREPARED) {
				if (state->acVolume && snd->loop) {
					alEvtqFlushStateType(&player->evtq, state, AL_EVENT_VOLUME);
					event->type  = AL_EVENT_VOLUME;
					event->data1 = (state->acVolume - 10);
				} else {
    				event->type = AL_EVENT_END;
					alSynStopVoice(player->drvr, voice->pvoice);
				}
			} else {
				event->type = AL_EVENT_END;
			}
			alEvtqPostEvent(&player->evtq, event, player->frameTime);
			state->state = AL_STATE_STOPPING;
		break;
        case (AL_EVENT_END):	
			alEvtqFlushState(&player->evtq, state);
			if (state->state > AL_STATE_PREPARED) {
				if (!alSynClosePort(voice->pvoice)) {
					if (gAcError.number == AC_CANT_SEND_DRIVER_COMMAND) {
						alEvtqPostEvent(&player->evtq, event, player->frameTime);
						return;
					}
				}
				alSynFreeVoice(player->drvr, voice);
				state->state = AL_STATE_STOPPED;			
				state->sound = 0;	// deallocate the sound
				state->strat = 0;
			}
		break;
		case (AL_EVENT_FLUSH):
			alEvtqFlushState(&player->evtq, state);
		break;
        default:	break;
    }
}

////////////////////////////////////////////////////////////////////////////////
// Func:	_alSndpVoiceHandler
// Desc:	
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
Sint32 _alSndpVoiceHandler( void *node )
{
	ALSndPlayer * player = (ALSndPlayer *)node;

	do {
		switch (player->nextEvent.type) {
            case (AL_EVENT_API):
				alEvtqPostEvent(&player->evtq, &player->nextEvent, player->frameTime);
            	break;

            default:
				_alSndpHandleEvent(player, &player->nextEvent);
                break;
        }
    } while (alEvtqNextEvent(&player->evtq, &player->nextEvent) == 0);

#ifdef DES_DEBUG
	tPrintf(0, 2, "Msgs %d, Max %d", player->evtq.eventCount, player->evtq.maxEventCount);
#endif
	
    return player->nextDelta;
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alSndpNew
// Desc:	Init the sound fx player
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
void alSndpNew(ALSndPlayer *player, ALSndpConfig *c) 
{    
	ALSynth			*drvr = &alGlobals->drvr;
	ALVoiceState	*soundState;
	Uint8			*ptr;
	ALEvent			evt;
	Uint32	   		i;

	player->maxSounds		=	c->maxSounds;
	player->target			=  -1;
	player->frameTime		=	c->frameTime;
	player->maxDistance		=	c->maxDistance;
	player->mainVolume		=   128;
	soundState				= (ALVoiceState *)cxalloc(c->maxSounds  * sizeof(ALVoiceState));
	player->sndState		=	soundState;

	for (i = 0; i < c->maxSounds; i++) {
		soundState[i].state  = AL_STATE_STOPPED;
		soundState[i].sound  = 0;
		soundState[i].offset = i;
	}
	
	// Init the event queue
	ptr = cxalloc(c->maxEvents * sizeof(ALEventListItem));
	alEvtqNew(&player->evtq, (ALEventListItem *)ptr, c->maxEvents);
    
	// Add ourselves to the driver
	player->drvr				= drvr;
	player->node.next			= NULL;
	player->node.handler		= _alSndpVoiceHandler;
	player->node.clientData		= player;
	alSynAddPlayer(player->drvr, &player->node, 0);

	// Start responding to API events
	evt.type = AL_EVENT_API;
	evt.msg.state = 0;
	alEvtqPostEvent(&player->evtq, (ALEvent *)&evt, player->frameTime);
	player->nextDelta = alEvtqNextEvent(&player->evtq, &player->nextEvent);
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alSndpGetSound
// Desc:	Gets the Sound Header Info from the soundId
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
ALSound * alSndpGetSound(ALSndPlayer *player, Sint32 soundId)
{
	ALSynth *drvr = player->drvr;

	if (soundId < BASELEVELSFX) {
		return drvr->dramAddr[AL_AUDIO_BANK_GENERIC] + (soundId);
	} else {
		return drvr->dramAddr[AL_AUDIO_BANK_LEVELSPECIFIC] + (SFXIndirTab[soundId]);
	}
}

////////////////////////////////////////////////////////////////////////////////
// Func:	alSndpAllocateVoice
// Desc:	Gets the Sound Header Info from the soundId
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
ALVoiceState * alSndpAllocateVoice(ALSound *sound, ALStrategy *strat, Uint32 flags, Sint32 *free)
{
    ALVoiceState *	sState = alSoundPlayer->sndState;
	Uint32			i, maxSounds = alSoundPlayer->maxSounds;
//	Uint32			pri, max_pri = 255;

	if (strat == 0) {
		flags |= SBFLAG_NOPOSITION;
	}
	
	//
	// Find a free voice for the sound
	//
	for (i = 0; i<maxSounds; i++) {		
   		if (!sState[i].sound) {
			*free					= i;
   			sState[i].sound			= sound;
			sState[i].state			= AL_STATE_PREPARED;
			sState[i].priority		= AL_PRIORITY_DEFAULT;
   			sState[i].strat			= strat;
   			sState[i].loop			= sound->loop;
   			sState[i].pitch			= sound->sampleRate;
   			sState[i].type			= (sound->bitDepth == 4) ? (sound->loop ? AC_ADPCM_LOOP : AC_ADPCM) : AC_16BIT;
   			sState[i].acVolume		= AC_MAX_VOLUME;
   			sState[i].pan			= AC_PAN_CENTER;
   			sState[i].fxMix			= 0;
   			sState[i].filter		= 0;
			sState[i].pitchOffset	= 0;
			sState[i].startTime	    = alSoundPlayer->drvr->timer;
			sState[i].endTime	    = alSoundPlayer->drvr->timer + 5;
   			return &sState[i];
		} else {
			if (sState[i].sound == sound) {
				if (sState[i].startTime	== alSoundPlayer->drvr->timer) {
					return 0;
				}
			}
		}
	}

	return 0;
}

Uint32 alSndpIsPlaying(ALVoiceState *state)
{
	if (state->state == AL_STATE_STOPPED)
		return 0;

	if (state->sound == 0)	return 0;
	if (state->loop)		return 1;

	return (alGlobals->drvr.timer >= state->endTime ? 0 : 1);
}

void alSndpSetVolume(ALVoiceState *state, Uint32 volume)
{
	alEvtqFlushStateType(&alSoundPlayer->evtq, state, AL_EVENT_VOLUME);
	alPostEventValue(&alSoundPlayer->evtq, AL_EVENT_VOLUME, state, (Sint32)volume);
}

void alSndpBankLoad(ACFILE *fp, KTU32 **base, KTU32 bankSize, KTU32 bank)
{
	KTU32		*	baseBuffer	=	KTNULL;
	KTU32		*	tempBuffer	=	KTNULL;
	KTU32		*	aicaBuffer	=	KTNULL;
	KTU32 			i, firstOffset;
	ALSound		*	wave;
				   
	if (fp) {
		if (!alSh4Alloc(&baseBuffer, &tempBuffer, bankSize, 32)) {
			return;
		}

		if (!alFileRead(*fp, (KTU8 *)tempBuffer, bankSize)) {
			sbExitSystem();
			return;
		}
	} else if (base) {
		tempBuffer = *base;
	}	

	if (alGlobals->drvr.spuAddr[bank] != 0) {
		alFree((void *)alGlobals->drvr.spuAddr[bank]);
		alGlobals->drvr.spuAddr[bank] = 0;
	}
	
	alGlobals->drvr.sfxCount[bank] = (KTU32)*tempBuffer++;
 	alGlobals->drvr.dramAddr[bank] = (ALSound *)tempBuffer;

	// Get a pointer to the start of the data
	bankSize   = bankSize - (alGlobals->drvr.sfxCount[bank] * sizeof(ALSound));
	tempBuffer = (KTU32 *)((KTU8 *)tempBuffer + (alGlobals->drvr.sfxCount[bank] * sizeof(ALSound)));
	aicaBuffer = (KTU32 *)alMalloc(bankSize);

	acG2Write(aicaBuffer, tempBuffer, bankSize);
	
	alGlobals->drvr.spuAddr[bank] = (KTU32)aicaBuffer;

	if (fp) {
		alSh4Free(baseBuffer);
		alSh4Alloc(&baseBuffer, &tempBuffer, (alGlobals->drvr.sfxCount[bank] * sizeof(ALSound)), 32);
		if (!alFileRewind(*fp)) {
			sbExitSystem();
			return;
		}
		if (!alFileRead(*fp, (KTU8 *)tempBuffer, (alGlobals->drvr.sfxCount[bank] * sizeof(ALSound)))) {
			sbExitSystem();
			return;
		}
		alGlobals->drvr.sfxCount[bank] = (KTU32)*tempBuffer++;
		alGlobals->drvr.dramAddr[bank] = (ALSound *)tempBuffer;
	} else {
		*base = tempBuffer;
	}
	
	wave = alGlobals->drvr.dramAddr[bank];
	firstOffset = wave[0].asset.offset;

	for (i = 0; i < alGlobals->drvr.sfxCount[bank]; i++) {
		wave[i].asset.offset += ((KTU32)aicaBuffer - firstOffset);
#ifdef _DEBUG	// Check that 16 bit samples are 2 byte aligned!
		if (wave[i].bitDepth == 16) {
			dAssert((wave[i].asset.offset % 2) == 0, "ERROR: Sound is not correctly aligned\n");
		}
#endif
	}

	if (bank == AL_AUDIO_BANK_GENERIC) {
		if (!acDspInstallProgram(wave[SND_REVERB_PROGRAM].asset.offset, wave[SND_REVERB_PROGRAM].asset.size)) {
			alPrintf("Failed to Load DSP Program Bank\n");
		}
		
		if (!acDspInstallOutputMixer(wave[SND_REVERB_OUTPUT].asset.offset, wave[SND_REVERB_OUTPUT].asset.size)) {
			alPrintf("Failed to Load DSP Output bank\n");
		}
	}
}

#endif // _AL_SND_PLAYER_

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Func:	SoundInitialise
// Desc:	
// In:		
// Out:		
// Cost:	
// Notes:	
////////////////////////////////////////////////////////////////////////////////
void SoundInitialize(char *arg1, char *arg2) 
{
#if SOUND
	ALSynConfig	synConfig;			

	alClose(alDriver);

	//
	// Initialise the voice manager
	//
	synConfig.maxPVoices	= 64;					// 24 SPU channels
	synConfig.maxUpdates	= 128;					// Max messages
	synConfig.maxVVoices	= 64;					// 24 virtual voices
	synConfig.maxBanks		= AL_AUDIO_BANK_MAX;
	synConfig.maxStreams	= 3;					// num of simultaneous streams
	synConfig.streamSize	= STRM_SNDRAM_SIZE;
	synConfig.mainRamSize	= STRM_MAINRAM_SIZE;
	synConfig.params		= 0;

	alDriver = (ALGlobals *)cxalloc(sizeof(ALGlobals));
	if (!alInit(alDriver, &synConfig)) {
		sbExitSystem ();
	}

	sfxSetStereo ((alDriver->drvr.speakerType == SYD_CFG_STEREO) ? true : false);

#ifdef _AL_SND_PLAYER_
	{
	ALSndpConfig	sndpConfig;
	ACFILE			fp;
	KTS32			bankSize;
	//
	// Initialise the sound fx player
	//
	sndpConfig.maxEvents        = 512;
	sndpConfig.maxSounds        = 48;
	sndpConfig.maxDistance      = SND_MAX_DISTANCE;
	sndpConfig.frameTime        = FRAMES_PER_SECOND;

	alSoundPlayer = (ALSndPlayer *)cxalloc(sizeof(ALSndPlayer));
	alSndpNew(alSoundPlayer, &sndpConfig);

	if (alFileOpen("GenericSoundFX.Kat", &fp, &bankSize) == KTFALSE) {
		sbExitSystem();
		return;
	}

	alSndpBankLoad(&fp, 0, bankSize, AL_AUDIO_BANK_GENERIC);

	alFileClose(fp);
	}
#endif

# ifdef _AL_STRM_PLAYER_
	{
	ALStrmpConfig	strmpConfig;
	//
	// Initialise the stream player
	//
	strmpConfig.maxStreamFiles	= STRM_MAX_TRACKS;
	strmpConfig.maxStreams		= synConfig.maxStreams;			
	strmpConfig.maxEvents		= strmpConfig.maxStreams * 48;
	strmpConfig.maxDistance		= SND_MAX_DISTANCE;
	strmpConfig.frameTime		= FRAMES_PER_SECOND;

	alStreamPlayer = (ALStrmPlayer *)cxalloc(sizeof(ALStrmPlayer));
	alStrmpNew(alStreamPlayer, &strmpConfig);
	}
# endif
#endif
}

KTU32 dangerMap[3] = { AL_STRM_DANGER_LOW, AL_STRM_DANGER_MED, AL_STRM_DANGER_HI };

KTU32 Danger(void)
{
	STRAT *	strat;
	float	amount = 0.f;

	for (strat = FirstStrat; strat; strat = strat->next) {
		float dist;
		
		if (!(strat->flag & STRAT_ENEMY) || (strat->flag & STRAT_BULLET))
			continue;	// ignore non-enemy strats
		
		if (!(strat->SoundBlock))
			continue;	// ignore strats without soundblocks

		dist = pSqrDist (&strat->pos, &player[0].Player->pos);

		if (strat->health >= 0.f)
			amount += (strat->health * strat->SoundBlock->danger) / dist;
	}
		
	return dangerMap[RANGE(0, (KTU32)amount, 2)];
}

void SoundUpdatePreDraw(void)
{
#if SOUND
	if ( alStreamPlayer->currentStream && 
		(alStreamPlayer->numStreams > 2) && 
		(alStreamPlayer->flags & AF_STRM_INTERRUPT) &&
		(!(alStreamPlayer->flags & AF_STRM_IN_CUTSCENE)) ) {
		if ((alStreamPlayer->currentBar != alStreamPlayer->lastBar) && (alStreamPlayer->currentBeat == 1)) {

			alStreamPlayer->lastBar = alStreamPlayer->currentBar;

			if (alStreamPlayer->currentIntensity) {
				if (alStreamPlayer->currentIntensity != alStreamPlayer->currentStream) {
					alStreamPlayer->currentStream = alStreamPlayer->currentIntensity;
					alStreamMixVolume(alStreamPlayer->currentStream, (sfxCDVolume * AC_MAX_VOLUME));
				}
			} else {
				alStreamPlayer->currentDanger = STRM_CHANNEL(Danger());
				switch (alStreamPlayer->currentStream)
				{
				case STRM_CHANNEL(AL_STRM_DANGER_LOW):
					if (alStreamPlayer->delayLeft) {
						alStreamPlayer->delayLeft--;
					} else {
						if (alStreamPlayer->currentDanger != alStreamPlayer->currentStream) {
							alStreamPlayer->currentStream = STRM_CHANNEL(AL_STRM_DANGER_MED);
							alStreamMixVolume(alStreamPlayer->currentStream, (sfxCDVolume * AC_MAX_VOLUME));
						}
						alStreamPlayer->delayLeft = (AL_STRM_MIX_DELAY >> 1) - 1;
					}
				break;
				case STRM_CHANNEL(AL_STRM_DANGER_MED):
					if (alStreamPlayer->delayLeft) {
						alStreamPlayer->delayLeft--;
					} else {
						if (alStreamPlayer->currentDanger & STRM_CHANNEL(AL_STRM_DANGER_HI)) {
							alStreamPlayer->currentStream = STRM_CHANNEL(AL_STRM_DANGER_HI);
							alStreamMixVolume(alStreamPlayer->currentStream, (sfxCDVolume * AC_MAX_VOLUME));
						} else 
						if (alStreamPlayer->currentDanger & STRM_CHANNEL(AL_STRM_DANGER_LOW)) {
							alStreamPlayer->currentStream = STRM_CHANNEL(AL_STRM_DANGER_LOW);
							alStreamMixVolume(alStreamPlayer->currentStream, (sfxCDVolume * AC_MAX_VOLUME));
						}
						alStreamPlayer->delayLeft = (AL_STRM_MIX_DELAY) - 1;
					}
				break;
				case STRM_CHANNEL(AL_STRM_DANGER_HI):
					if (alStreamPlayer->delayLeft) {
						alStreamPlayer->delayLeft--;
					} else {
						alStreamPlayer->delayLeft = (AL_STRM_MIX_DELAY << 1) - 1;
						alStreamPlayer->currentStream = STRM_CHANNEL(AL_STRM_DANGER_MED);
						alStreamMixVolume(alStreamPlayer->currentStream, (sfxCDVolume * AC_MAX_VOLUME));
					}
				break;
				}
			}
		}
	}

	if (!alStreamPlayer->currentStream) {
		gdFsReqDrvStat();
		if (alStreamPlayer->target != -1) {
			if (gStreamFiles[alStreamPlayer->currentTrackId].flags & AF_STRM_RESTART) {
				SoundPlayTrack(alStreamPlayer->currentTrackId);
			}
		}
	} 
	
	//
	//	Service the players
	//
	if (alGlobals) {
		alAudioFrame(&alGlobals->drvr, 0);
	}
#endif
}

void SoundUpdatePostDraw(void)
{
#if SOUND
	//
	//	Peform post draw service for the players
	//
	if (alGlobals) {
		alAudioFrame(&alGlobals->drvr, 1);
	}
#endif
}					

void SoundPlayTrack (int track)
{
#ifdef _AL_STRM_PLAYER_
	KTU32			i, volume;
	ALStream	*	strm;
	ALStreamFile *	sfile;
	ALVoiceState *	vstate;
	
	if (!alStreamPlayer) return;

	if (alStreamPlayer->currentStream) {
		SoundStopTrack();
	}

	sfile = &gStreamFiles[track];

	if (alFileOpen((KTSTRING)sfile->name, &sfile->fp, &sfile->size) == KTFALSE) {
		dPrintf("Failed to open %s\n", sfile->name);
		return;
	}

	alFileRead(sfile->fp, (KTU8 *)alStreamPlayer->headerBuffer, STRM_SECTOR_SIZE);

	alStreamPlayer->numStreams = (alStreamPlayer->headerBuffer->numberOfTracks << 1);	// Stereo streams only
	alStreamPlayer->target				= -1;
	alStreamPlayer->currentTrackId		= track;
	alStreamPlayer->currentIntensity	= 0;
	alStreamPlayer->currentDanger		= STRM_CHANNEL(AL_STRM_DANGER_LOW);
	alStreamPlayer->currentBar			= 1;
	alStreamPlayer->lastBar				= 0;
	alStreamPlayer->currentBeat			= 1;
	alStreamPlayer->delayLeft			= AL_STRM_MIX_DELAY;
	alStreamPlayer->repeatCount			= 0;

	if (alStreamPlayer->numStreams > 2) {
		alStreamPlayer->currentStream = STRM_CHANNEL(AL_STRM_DANGER_LOW);
	} else {
		alStreamPlayer->currentStream = STRM_CHANNEL(AL_STRM_DANGER_NONE);
	}

	if (!(alStreamPlayer->flags & AF_STRM_INTERRUPT)) {
		acG2FifoCheck();
		acSystemEnableArmInterrupts();
		alStreamPlayer->flags |= AF_STRM_INTERRUPT;
	}
	
	for (i = 0; i<alStreamPlayer->numStreams; i++) {
		
		// Get a pointer to the stream handle
		strm = alStrmpAllocateStream(sfile, track);
		if (!strm) {
			alPrintf(AL_ERR_NO_CHANNEL); return;
		}
		
		volume = (KTU32)((alStreamPlayer->currentStream & SPU_VOICECH(strm->channel)) ? (sfxCDVolume * AC_MAX_VOLUME) : 0);

		// Get a pointer to a free voice slot
		vstate = alStrmpAllocateVoice(strm, volume, track, 1);
		if (!vstate) {
			alPrintf(AL_ERR_NO_VOICE);
			alStrmpDeAllocateStream(strm); return;
		}

		if (!i) {	// Set the CD head to the start location as early as possible
			alStreamPlayer->target = vstate->offset;
			alStreamPlayer->readLength = (alStreamPlayer->headerBuffer->numberOfTracks * strm->readSize) << 1;
			alPostEvent(&alStreamPlayer->evtq, AL_EVENT_CDSEEK, vstate);
		}
		
		if (strm->channel & 0x20) {
			alStreamPlayer->voiceMaskHi |= SPU_VOICECH(strm->channel & 0x1f);
		} else {
			alStreamPlayer->voiceMaskLo |= SPU_VOICECH(strm->channel);
		}
			
		alPostEventTime(&alStreamPlayer->evtq, AL_EVENT_PLAY, vstate, alStreamPlayer->frameTime);
	}

	alStreamPlayer->currentTime  = syTmrGetCount();
	alStreamPlayer->currentTempo = gStreamFiles[track].tempo;
#endif
}		

void SoundStopTrack(void)
{
#ifdef _AL_STRM_PLAYER_
	ALStreamFile *	sfile;

	if (!alStreamPlayer) return;

	if (!alStreamPlayer->currentStream) {
		return;
	}

	alStreamStop();

    sfile = &gStreamFiles[alStreamPlayer->currentTrackId];

	alStreamPlayer->currentTempo  =  0;
	alStreamPlayer->currentStream =  0;
	alStreamPlayer->target		  = -1;

	alAsyncFileClose(sfile->fp);
#endif
}

void MusicSetIntensity(int intensity)
{
#ifdef _AL_STRM_PLAYER_
	if (intensity) {
		alStreamPlayer->currentIntensity = STRM_CHANNEL(dangerMap[intensity - 1]);
	} else {
		alStreamPlayer->currentIntensity = 0;
		alStreamPlayer->delayLeft		 = 2;
	}
#endif
}

void SoundFade (float amt, int music)
{
#ifdef _AL_STRM_PLAYER_
	ALVoiceState *strmState = alStreamPlayer->strmState;
	Uint32 i, volume, maxStreams = alStreamPlayer->maxStreams;
#endif
	
	amt = RANGE(0.f, amt, 1.f);
	
	sfxShotGlobalVolume = 1.f - amt;

#ifdef _AL_STRM_PLAYER_
	if (music) {
		if (alStreamPlayer->currentStream && (alStreamPlayer->flags & AF_STRM_INTERRUPT)) {
			for (i = 1; i<maxStreams; i++) {
				volume = (Uint32)((alStreamPlayer->currentStream & SPU_VOICECH(i)) ? sfxCDVolume * (AC_MAX_VOLUME - (AC_MAX_VOLUME * amt)) : 0);
				alStreamSetVolume(&strmState[i], volume);
			}
		}
	}
#endif
}

void MusicFade (float amt)
{
#ifdef _AL_STRM_PLAYER_
	ALVoiceState *strmState = alStreamPlayer->strmState;
	Uint32 i, volume, maxStreams = alStreamPlayer->maxStreams;
	
	amt = RANGE(0.f, amt, 1.f);
		
	if (alStreamPlayer->currentStream && (alStreamPlayer->flags & AF_STRM_INTERRUPT)) {
		for (i = 1; i<maxStreams; i++) {
		 	volume = (Uint32)((alStreamPlayer->currentStream & SPU_VOICECH(i)) ? sfxCDVolume * (AC_MAX_VOLUME - (AC_MAX_VOLUME * amt)) : 0);
		 	alStreamSetVolume(&strmState[i], volume);
		}
	}
#endif
}

void SoundPause(void)
{
#ifdef _AL_SND_PLAYER_
	{
	ALVoiceState *sndState = alSoundPlayer->sndState;
	Uint32 i, maxSounds = alSoundPlayer->maxSounds;
	SoundBlock *block;

	for (i = 0; i<maxSounds; i++) {
		if (sndState[i].state != AL_STATE_STOPPED) {
			if (sndState[i].sound && sndState[i].sound->loop) {
				alSndpSetVolume(&sndState[i], 0);
			} else { 
				block = (SoundBlock *)sndState[i].strat;
				if (block) {
					sfxStop(block->handle, 0.0);
					block->handle = -1;
					block->flags  =  0;
				} else {
					alSndpSetVolume(&sndState[i], 0);									
				}
			}
		}
	}
	}
#endif
#ifdef _AL_STRM_PLAYER_
	{
	ALVoiceState *strmState = alStreamPlayer->strmState;
	Uint32 i, j, voiceMask, maxStreams = alStreamPlayer->maxStreams;
	ALStream *strm;
	ALVoice	*voice;
	
	if (alStreamPlayer->flags & AF_STRM_INTERRUPT) {
		acSystemDisableArmInterrupts();
		alStreamPlayer->flags &= ~AF_STRM_INTERRUPT;
	}

	while (alStreamPlayer->flags & AF_STRM_CDREAD) {}

	if (alStreamPlayer->voiceMaskLo != alStreamPlayer->voiceStateLo) {
		SoundStopTrack();
	} else {
		alEvtqFlush(&alStreamPlayer->evtq);

		voiceMask = 0;
		
		for (i = 1; i<maxStreams; i++) {
		
			if (strmState[i].state == AL_STATE_STOPPED)
				continue;
		
			strm = strmState[i].stream;
		
			if (!strm) continue;
		
			voice = &strmState[i].voice;
		
			alSynStopVoice(&alGlobals->drvr, voice->pvoice);
		
			strm->statusFlags &= ~AF_STRM_STARTED;
		
			// Reset the stream buffer flow
//			j = (strm->cdReadPos-1) % 4;
//			if ((strm->cdReadPos - j) > 1)
//				strm->cdReadPos	= (strm->cdReadPos - j);
//			else
			strm->cdReadPos		=  1;
			strm->interrupts    =  alStreamPlayer->headerBuffer->interruptsTillEnd - 1;
			strm->sramPlayPos	=  strm->sramLastPlayPos = 0;
			strm->sramWritePos	=  0;
			strm->sramUsedSize	=  0;
			strm->dmaRequest	=  STRM_SNDRAM_SIZE;
			strm->sramBuffer	=  strm->mramBuffer	= 0;			
		
			if (strmState[i].offset == alStreamPlayer->target) {
				alStreamPlayer->currentBar = alStreamPlayer->currentBeat = 1;
				if (strm->sfile) {
					alAsyncFileSeek(strm->sfile->fp, strm->cdReadPos);
				}
			}
					
			for (j = 0; j<STRM_MAINRAM_BUFFERS; j++)
				strm->mramUsedSize[j] = 0;		
		
			voiceMask |= SPU_VOICECH(strm->channel);
		}
			
		alStreamPlayer->voiceStateLo &= ~voiceMask;
	}
	}
#endif
}

void SoundResume(void)
{
#ifdef _AL_SND_PLAYER_
	{
	ALVoiceState *sndState = alSoundPlayer->sndState;
	Uint32 i, maxSounds = alSoundPlayer->maxSounds;

	for (i = 0; i<maxSounds; i++) {
		if (sndState[i].state != AL_STATE_STOPPED) {
			alSndpSetVolume(&sndState[i], (sndState[i].lastVolume * sfxShotVolume));
		}
	}
	}
#endif
#ifdef _AL_STRM_PLAYER_
	{
	ALVoiceState *strmState = alStreamPlayer->strmState;
	Uint32 i, j, volume, maxStreams = alStreamPlayer->maxStreams;
	ALStream *strm;
	ALVoice *voice;
	ALEvent		evt;
		
	if (!alStreamPlayer->currentStream) {
		SoundPlayTrack(alStreamPlayer->currentTrackId);
	} else {
		// Start responding to API events
		evt.type = AL_EVENT_API;
		evt.msg.state = 0;
		alEvtqPostEvent(&alStreamPlayer->evtq, (ALEvent *)&evt, alStreamPlayer->frameTime);
		alStreamPlayer->nextDelta = alEvtqNextEvent(&alStreamPlayer->evtq, &alStreamPlayer->nextEvent);
		
		for (i = 1; i<maxStreams; i++) {
		
			if (strmState[i].state == AL_STATE_STOPPED)
				continue;
			
			strm = strmState[i].stream;
		
			if (!strm) continue;
		
			alEvtqFlushState(&alStreamPlayer->evtq, &strmState[i]);
		
			voice = &strmState[i].voice;
		
			// Fill the play buffers with silence
			acG2Fill((volatile KTU32 *)(strm->sramStart), 0x80, STRM_SNDRAM_SIZE);
		
			volume = (Uint32)((alStreamPlayer->currentStream & SPU_VOICECH(i)) ? (sfxCDVolume * AC_MAX_VOLUME) : 0);
			alSynSetVolume(voice->pvoice, volume);
		
			// Restart the CD reading
			if (alStreamPlayer->target == strmState[i].offset) {
				alPostEvent(&alStreamPlayer->evtq, AL_EVENT_CDSEEK, &strmState[i]);
			}
		
			// Restart the DMA's
			alPostEventTime(&alStreamPlayer->evtq, AL_EVENT_READY, &strmState[i], alStreamPlayer->frameTime);
		}
		
		alStreamPlayer->currentTime = syTmrGetCount();
		alStreamPlayer->delayLeft   = (AL_STRM_MIX_DELAY);
			
		if ((alStreamPlayer->flags & AF_STRM_INTERRUPT) == 0) {
			acG2FifoCheck();		
			acSystemEnableArmInterrupts();
			alStreamPlayer->flags |= AF_STRM_INTERRUPT;
		}
	}
	}
#endif
}

void SoundReset(void)
{
#ifdef _AL_SND_PLAYER_
	{
	Uint32		i, maxSounds = alSoundPlayer->maxSounds;
	ALVoiceState *sndState = alSoundPlayer->sndState;
	ALVoice		*voice;
	ALEvent		evt;

	// Shouldn't have to do this, but for paranoia.
	alEvtqFlush(&alSoundPlayer->evtq);

	for (i = 0; i<maxSounds; i++) {
		
		if (sndState[i].state > AL_STATE_PREPARED) {	
			voice = &sndState[i].voice;

			alSynStopVoice(alSoundPlayer->drvr, voice->pvoice);
			alSynClosePort(voice->pvoice);
			alSynFreeVoice(alSoundPlayer->drvr, voice);
		}

		sndState[i].state = AL_STATE_STOPPED;			
		sndState[i].sound = 0;
		sndState[i].strat = 0;
	}

	// Start responding to API events
	evt.type = AL_EVENT_API;
	evt.msg.state = 0;
	alEvtqPostEvent(&alSoundPlayer->evtq, (ALEvent *)&evt, alSoundPlayer->frameTime);
	alSoundPlayer->nextDelta = alEvtqNextEvent(&alSoundPlayer->evtq, &alSoundPlayer->nextEvent);
	}
#endif
}

Uint32 * SoundLoadBank(void *filePtr, int size)
{
	KTU32 *	aicaBuffer = KTNULL;
	KTU32	bankSize;
	void *	base;	
	void *	temp;
	
	bankSize = (size <0) ? (KTU32)(*(KTU32 *)filePtr) : (KTU32)size;
	
	base = temp = SHeapAlloc (MissionHeap, bankSize + 32);
	temp = (char *)(((int)temp + 31) &~31);
	
	if (!temp) {
		dPrintf("FAIL ALLOC IN LEVEL SOUND\n");
		return KTNULL;
   	}

	sRead(temp, bankSize, filePtr);
			
	alSndpBankLoad(0, (KTU32 **)&temp, bankSize, AL_AUDIO_BANK_LEVELSPECIFIC);

	SHeapRewind(MissionHeap, temp);

	return 0;
}

void sfxSetFXVolume(float volume)
{
	sfxShotVolume = RANGE(0.f, volume, 1.f);
}

void sfxSetCDVolume(float volume)
{
	sfxCDVolume = RANGE(0.f, volume, 1.f);
#ifdef _AL_STRM_PLAYER_
	alStreamMixVolume(alStreamPlayer->currentStream, (sfxCDVolume * AC_MAX_VOLUME));
#endif
}

float sfxGetFXVolume(void)
{
	return sfxShotVolume;
}

float sfxGetCDVolume(void)
{
	return sfxCDVolume;	
}

bool sfxIsStereo(void)
{
	return (alGlobals ? (alGlobals->drvr.speakerType == SYD_CFG_STEREO) : false);
}

void sfxSetStereo (bool isStereo)
{
	Sint32 newMode = (isStereo) ? SYD_CFG_STEREO : SYD_CFG_MONO;
	if (alGlobals->drvr.speakerType != newMode) {
		alGlobals->drvr.speakerType  = newMode;
		acSystemSetStereoOrMono(newMode);
	}
}

int sfxPlay (int sfxNum, float volume, float pitchAdj)
{
	Sint32			handle = -1;
#ifdef _AL_SND_PLAYER_
	ALSound *		sound;
	ALVoiceState *	state;

	volume = (volume * sfxShotVolume * AC_MAX_VOLUME);

	if (volume == 0.f)
		return (int)handle;

	sound = alSndpGetSound(alSoundPlayer, sfxNum);
	if (!sound) {
		return (int)handle;
	}

	state = alSndpAllocateVoice(sound, 0, 0, &handle);
	if (!state) {
		return (int)handle;
	}

	state->acVolume = (KTU32)volume;
	state->pitch   *= (1.f + pitchAdj);

	alPostEvent(&alSoundPlayer->evtq, AL_EVENT_PLAY, state);
#endif
	return (int)handle;
}

void sfxStop (int channel, float time)
{
#ifdef _AL_SND_PLAYER_
	Uint32 maxSounds = alSoundPlayer->maxSounds;
	ALVoiceState *sState = alSoundPlayer->sndState;

	if (channel > -1 && channel < maxSounds) {
		if (sState[channel].sound) {
			alPostEventValue(&alSoundPlayer->evtq, AL_EVENT_STOP, &sState[channel], (Sint32)time);
		}
	} 
#endif
}

KTU32 sfxGetPosition(SoundBlock *block, Point3 *pos, float *volume, float *distance)
{
	Vector3 vector;
	float	scaleVolume, angle, temp = 0.f;

	scaleVolume = (block->volume);

	if (Multiplayer) {
		if (volume)		*volume   = scaleVolume;
		if (distance)	*distance = 20.f;
		return AC_PAN_CENTER;
	}

	mPoint3Multiply3 (&vector, pos, &mWorldToView);

	if ((block->flags & SBFLAG_NODIRECTION) && (currentCamera))	{	// non-directional
		temp = pDist (pos, &currentCamera->pos);
	} else {
		temp = 1.f / (v3Normalise(&vector));
	
		if (vector.z < 0) {	// Is the sound behind you?
			temp *= ((1 + vector.z) * 0.2) + 0.8;
		}
	}

	if (distance)	{
		*distance = temp;
	}
	
	if (temp > block->innerRange) {
		angle = (float)(block->outerRange - block->innerRange);
		if ((temp < (float)block->outerRange) && (angle > 0.f)) {
			*volume = (scaleVolume * (block->outerRange - temp)) / angle;
		} else { 
			*volume = 0.f;
		}
	} else {
		*volume = scaleVolume;
		return AC_PAN_CENTER;
	}

   	angle  = (fatan2(fabs(vector.z), vector.x));
	angle *= (PI2);
	angle -= (PI/2);
	angle  = (angle * (float)-(AC_PAN_CENTER)/PI) + (float)AC_PAN_CENTER;
	
	return AC_PAN_LEFT - (KTU32)RANGE(AC_PAN_RIGHT, (KTU32)angle, AC_PAN_LEFT);
}

int sfxPlayPos (SoundBlock *block, Point3 *pos)
{
	Sint32			handle = -1;
#ifdef _AL_SND_PLAYER_
	ALSound *		sound;
	ALVoiceState *	state;
	Uint32			pan;
	float			volume;

	if (block->flags & (SBFLAG_HARDLEFT|SBFLAG_HARDRIGHT|SBFLAG_NOPOSITION)) {
		volume = block->volume;
		pan =	(block->flags & SBFLAG_HARDLEFT)  ? AC_PAN_LEFT  : 
				(block->flags & SBFLAG_HARDRIGHT) ? AC_PAN_RIGHT : AC_PAN_CENTER;
	} else {
		pan = sfxGetPosition(block, pos, &volume, 0);
	}

	volume = (volume * sfxShotVolume * sfxShotGlobalVolume * AC_MAX_VOLUME);

	if (volume == 0.f)
		return (int)handle;

	sound = alSndpGetSound(alSoundPlayer, block->num);
	if (!sound) {
		return (int)handle;
	}
	
	if (sound->loop) {
		block->flags |= SBFLAG_CONTINUOUS;
	}

	state = alSndpAllocateVoice(sound, (ALStrategy *)block, 0, &handle);
	if (!state) {
		return (int)handle;
	}

	state->acVolume = (KTU32)volume;
	state->pan      = (pan);
	state->pitch   *= (block->pitch);
	state->fxMix    = (block->flags & SBFLAG_NOREVERB) ? 0 : 4;
	state->filter   = (block->flags & SBFLAG_FILTERENV);

	alPostEvent(&alSoundPlayer->evtq, AL_EVENT_PLAY, state);
#endif
	return (int)handle;
}


void UpdateStratSound (STRAT *strat)
{
#ifdef _AL_SND_PLAYER_
	StratSoundHeader *	header	= strat->SoundBlock;
	ALVoiceState	 *	state;
	SoundBlock		 *	block;
	int					i;
	Matrix				matrix;

	if (!header || !header->nSounds) return;

	if (header->frameTime & SND_FRAME_UPDATE)
	{		
		matrix = strat->m;
		mPreScale(&matrix, strat->scale.x, strat->scale.y, strat->scale.z);
		mPostTranslate(&matrix, strat->pos.x, strat->pos.y, strat->pos.z);
		
		for (i = 0; i < header->nSounds; ++i)
		{			
			float		distance, attenuation;
			Uint32		val;
			Point3		pos;
			
			block = &header->block[i];
			
			if (!(block->flags & SBFLAG_ACTIVE) || (block->flags & SBFLAG_DEACTIVATED))
				continue;

			mPoint3Multiply3 (&pos, &block->offset, &matrix);
		
			// Is this a continous sound which is out of range?
			if (block->flags & SBFLAG_RELEASED)
			{
				if (!Multiplayer)
				{
					distance = pDist(&pos, &currentCamera->pos);
					
					if (distance >= block->outerRange)
						continue;
				}
		
				block->handle = sfxPlayPos (block, &pos);
		
				if (block->handle < 0)
					continue;
		
				block->flags &= ~SBFLAG_RELEASED;
				continue;
			} 
				
			dAssert (block->handle >= 0, "Sound slipped through the net");
		
			if (block->flags & (SBFLAG_HARDLEFT|SBFLAG_HARDRIGHT|SBFLAG_NOPOSITION|SBFLAG_NOUPDATE))
			{
				val =	(block->flags & SBFLAG_HARDLEFT)  ? AC_PAN_LEFT  : 
						(block->flags & SBFLAG_HARDRIGHT) ? AC_PAN_RIGHT : AC_PAN_CENTER;
				distance = 0.f;
				attenuation = block->volume;
			} else
			{
				val = sfxGetPosition(block, &pos, &attenuation, &distance);
			}
		
			attenuation = (attenuation * sfxShotVolume * sfxShotGlobalVolume * AC_MAX_VOLUME);

			state = &alSoundPlayer->sndState[block->handle];

			// Check to see if the sound has gone out of range or has finished
			if ((attenuation == 0.f) || !alSndpIsPlaying(state))
			{
				sfxStop (block->handle, 0.f);
				block->handle = -1;
				if (block->flags & SBFLAG_CONTINUOUS) 	// Continous noises are kept around for restarting
					block->flags |=  SBFLAG_RELEASED;
				else 									// Noncontinous noises are just released and stopped
					block->flags &= ~SBFLAG_ACTIVE;
				continue;
			}		
		
			if ((block->flags & SBFLAG_NOUPDATE) || (state->state == AL_STATE_STOPPING))
				continue;
		
			//
			// Set up the pan amount
			//
			if (val != state->pan)
			{
				alPostEventValue(&alSoundPlayer->evtq, AL_EVENT_PAN, state, val);
			}
			
			//
			// Set up the volume
			//
			val = (KTU32)attenuation;
			if (val != state->acVolume)
			{
				alPostEventValue(&alSoundPlayer->evtq, AL_EVENT_VOLUME, state, val);
			}  
			
			//
			// Doppler effect
			//
			if (block->flags & (SBFLAG_NOPOSITION|SBFLAG_NODOPPLER))
			{
				attenuation = 0.f;
			} 
			else
			{
				attenuation = RANGE(-SND_MAX_DOPPLER, ((block->lastDist - distance) * (float)FRAMES_PER_SECOND) / (SND_FRAME_UPDATE * 512.f), SND_MAX_DOPPLER);
			}
			if (state->sound)	// for saftey - should now be caught by alSndpIsPlaying
				val = (KTU32)((block->pitch + attenuation) * state->sound->sampleRate);
			block->lastDist = distance;
		
			//
			// Set the pitch
			//
			if (val != state->pitch)
			{
				alPostEventValue(&alSoundPlayer->evtq, AL_EVENT_PITCH, state, val);
			}
		}
	}
	header->frameTime++;
#endif
}

void AllocStratSoundBlock (STRAT *strat, int nSound)
{
	StratSoundHeader *retVal;
	int i;

	if (strat->SoundBlock) {
		FreeStratSoundBlock (strat);
		strat->SoundBlock = NULL;	
	}

	dAssert (strat->SoundBlock == NULL, "Strat already has a soundblock");
	dAssert (nSound > 0, "Silly call to AllocStratSoundBlock");

	retVal = SoundAlloc (sizeof (StratSoundHeader) + sizeof (SoundBlock) * (nSound - 1));
	dAssert (retVal != NULL, "Out of sound space");
	
	//put in by MATTP to ensure alloc cannot cause 0 address writes
	if (retVal)
	{
		retVal->nSounds		= nSound;
		retVal->danger		= 0.f;
		retVal->frameTime	= 0;

		for (i = 0; i < nSound; ++i) {
			retVal->block[i].handle		=  -1;
			retVal->block[i].innerRange	=	SND_MIN_DISTANCE;
			retVal->block[i].outerRange	=	SND_MAX_DISTANCE;
			retVal->block[i].flags		=	0;
			retVal->block[i].pitch		=	1.f;
			retVal->block[i].lastDist	=	0.0f;
			retVal->block[i].num		=	0;
			retVal->block[i].offset.x	=	0.0f;
			retVal->block[i].offset.y	=	0.0f;
			retVal->block[i].offset.z	=	0.0f;
			retVal->block[i].volume		=	0.0f;
		}

		strat->SoundBlock = retVal;
	}
}

void FreeStratSoundBlock (STRAT *strat)
{
	StratSoundHeader *header = strat->SoundBlock;
	int i;

	dAssert (header != NULL, "Trying to free a soundblock which doesn't exist");
	if (!header) return;

	for (i = 0; i < header->nSounds; i++)
		if (header->block[i].handle >= 0)
			StopSound(strat, i, 1.f);
	
	SoundFree ((void *)header);
}

void PlaySound (STRAT *strat, int channel, int soundNum, float volume, float x, float y, float z, int flags)
{
	SoundBlock *block;
	Point3		pos;
	Matrix		matrix;

	dAssert (strat->SoundBlock != NULL, "No sound block");
	
	//if (strat->SoundBlock == NULL)
	//	CrashOut ("No sound block", (0x1f) << 8, 0xff0000);

   	if (!strat->SoundBlock)
		return;
	
	dAssert (channel < strat->SoundBlock->nSounds, "Invalid channel");
   
	if (channel >= strat->SoundBlock->nSounds) return;

	block = &strat->SoundBlock->block[channel];
		
	if (block->flags & SBFLAG_ACTIVE) {
		StopSound(strat, channel, 0.f);
	}

	// If the channel is deactivated then only 'keep' it if its continuous
	if (block->flags & SBFLAG_DEACTIVATED) {
		if (!(block->flags & SBFLAG_CONTINUOUS))
			return;
		block->flags = (SBFLAG_DEACTIVATED | SBFLAG_ACTIVE | flags);
	} else {
		block->flags = (SBFLAG_ACTIVE | flags);
	}
	block->num      =  soundNum;
	block->offset.x =  x;
	block->offset.y =  y;
	block->offset.z =  z;
	block->volume   =  volume;
	block->pitch   += (flags & SBFLAG_RANDOM ? RandRange(-0.04f, 0.04f) : 0.f);
	block->handle   = -1;
		
	matrix = strat->m;
		
	mPreScale(&matrix, strat->scale.x, strat->scale.y, strat->scale.z);
	mPostTranslate(&matrix, strat->pos.x, strat->pos.y, strat->pos.z);
	mPoint3Multiply3(&pos, &block->offset, &matrix);
		
	if (!Multiplayer && currentCamera && !(flags & SBFLAG_NOPOSITION))
		block->lastDist = pDist (&pos, &currentCamera->pos);
	else
		block->lastDist = 0.f;
		
	if ((block->lastDist < block->outerRange) && (!(block->flags & SBFLAG_DEACTIVATED))) {
		block->handle = sfxPlayPos (block, &pos);
	}
	if (block->handle < 0) {
		if (block->flags & SBFLAG_CONTINUOUS)
			block->flags |=  SBFLAG_RELEASED;
		else
			block->flags &= ~SBFLAG_ACTIVE;
	}
}

void StopSound (STRAT *strat, int channel, float time)
{
	StratSoundHeader *header;
	STRAT *curstrat = strat;
	SoundBlock *block;
	Uint32 i;

	//first check, see if it's a request to globally kill off all exisiting
	//strat sound, (for the purpose of cut-scenes)
	if (channel < 0) {
		if (channel == -1) {
			// deactivate all sounds
			strat = FirstStrat;
			while (strat) {
				header = strat->SoundBlock;
				if ((header) && (curstrat != strat)) {
					for (i = 0; i < header->nSounds; i++) {
						block = &header->block[i];					
						if (block->flags & SBFLAG_ACTIVE)
							sfxStop (block->handle, 0);					
						block->handle = -1;
						if (block->flags &  SBFLAG_CONTINUOUS) {
							block->flags |= (SBFLAG_DEACTIVATED|SBFLAG_RELEASED);
						} else {
							block->flags = SBFLAG_DEACTIVATED;
						}
					}
				}
				strat = strat->next;
			}
			alStreamPlayer->flags |= AF_STRM_IN_CUTSCENE;
		} else {
			// reactivate looped sounds
			strat = FirstStrat;
			while (strat) {
				header = strat->SoundBlock;
				if (header) {
					for (i = 0; i < header->nSounds; i++) {
						block = &header->block[i];					
						if (block->flags &   SBFLAG_DEACTIVATED) {
							block->flags &= ~SBFLAG_DEACTIVATED;
						}						
					}
				}
				strat = strat->next;
			}			
			alStreamPlayer->flags &= ~AF_STRM_IN_CUTSCENE;
		}
	} else {
		if (strat->SoundBlock == NULL)	return;
		dAssert (channel < strat->SoundBlock->nSounds, "Invalid channel");
		if (channel >= strat->SoundBlock->nSounds) return;
		
		block = &strat->SoundBlock->block[channel];
		
		if (block->flags & SBFLAG_ACTIVE)
			sfxStop (block->handle, time);
		
		block->handle = -1;
		if (block->flags & SBFLAG_DEACTIVATED)
			block->flags = SBFLAG_DEACTIVATED;
		else 
			block->flags = 0;
	}
}


void SetVolume (STRAT *strat, int channel, float volume)
{
	SoundBlock *block;

	dAssert (strat->SoundBlock != NULL, "No sound block");
	dAssert (channel < strat->SoundBlock->nSounds, "Invalid channel");
	if (!strat->SoundBlock) return;
	if (channel >= strat->SoundBlock->nSounds) return;

	block = &strat->SoundBlock->block[channel];

	block->volume = RANGE(0.f, volume, 1.f);
}

void SetPitch (STRAT *strat, int channel, float pitch)
{
	SoundBlock *block;

	dAssert (strat->SoundBlock != NULL, "No sound block");
	dAssert (channel < strat->SoundBlock->nSounds, "Invalid channel");
	if (!strat->SoundBlock) return;
	if (channel >= strat->SoundBlock->nSounds) return;
	
	block = &strat->SoundBlock->block[channel];

	block->pitch = RANGE(0.01f, (1.f + pitch), 2.5f);
}

Bool IsPlaying (STRAT *strat, int channel)
{
	SoundBlock *block;

	dAssert (strat->SoundBlock != NULL, "No sound block");
	dAssert (channel < strat->SoundBlock->nSounds, "Invalid channel");
	if (!strat->SoundBlock) return;
	if (channel >= strat->SoundBlock->nSounds) return;
	
	block = &strat->SoundBlock->block[channel];

	return (Bool)(block->flags & SBFLAG_ACTIVE);
}

void SetAudibleRange(STRAT *strat, int channel, Sint16 inner, Sint16 outer)
{
	SoundBlock *block;

	dAssert (strat->SoundBlock != NULL, "No sound block");
	dAssert (channel < strat->SoundBlock->nSounds, "Invalid channel");
	if (!strat->SoundBlock)	return;
	if (channel >= strat->SoundBlock->nSounds) return;

	block = &strat->SoundBlock->block[channel];

	block->innerRange = RANGE(0, inner, 0x7fff);
	block->outerRange = RANGE(0, outer, 0x7fff);
}

void RegisterDanger(STRAT *strat, float dangerVal)
{
	if (!strat->SoundBlock)	return;

	strat->SoundBlock->danger = dangerVal;
}

#endif	// audio 64
//##########################################################################
//
//
//##########################################################################
/* End of SndUtls.c */

