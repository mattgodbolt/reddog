/*
 * Gratuitously nicked from the Sega libraries
 */
#include "RedDog.h"
#include <shinobi.h>
#include "backup.h"
#include "Render\Internal.h"

// Globals
Bool bupDriveChange = FALSE, bupDataChange = FALSE;

#define bupPrintf dPrintf

/* Some callback stuff */
static Sint32 BupComplete(Sint32 drive, Sint32 op, Sint32 stat, Uint32 param);
static Sint32 BupProgress(Sint32 drive, Sint32 op, Sint32 count, Sint32 max);
static void BupInitCallback(void);
static Bool _BupIsEnoughSpace(Sint32 drive);

extern int SaveOKCount[4], SaveFailedCount[4];

volatile Uint8 gLoadQueue, gSaveQueue, gCheckQueue;
volatile Sint32 gLockedDrive;
static void *gSaveBuf[8];
static char gMountBuf[BUM_WORK_SIZE(BUD_CAPACITY_1MB, 1)];

static BACKUPINFO gBupInfo[8];
static bool IsEnoughSpace[8];

Bool BupDrivePanic[8];
static BUS_FILEINFO BupWaitDrive[8];
int bupNumBlocks = 11;
extern int suppressReset;
bool DriveRescan(int drive);

char *lastBupError = NULL;
static Uint8 SaveGraphic[] =
{
#include "GameSave.h"
};
/*static Uint8 MattGPic[] =
{
#include "MattHead.h"
};
static Uint8 MattPPic[] =
{
#include "MatPHead.h"
};
static Uint8 NikLPic[] =
{
#include "NikLHead.h"
};
static Uint8 NikCPic[] =
{
#include "NikCHead.h"
};
static Uint8 DavePic[] =
{
#include "DaveHead.h"
};
static Uint8 LeonPic[] =
{
#include "LeonHead.h"
};

static Uint8 *heads[] =
{
	MattGPic,
	MattPPic,
	NikLPic,
	NikCPic,
	DavePic,
	LeonPic,
};
*/
int SaveFailed = 0;

static void bupDumpDisk (BUS_FILEINFO *disk)
{
	bupPrintf ("*   File dump:");
	bupPrintf ("*   time                   = %d, %d, %d, %d, %d, %d", 
		disk->time.year, 
		disk->time.month,
		disk->time.day,
		disk->time.hour,
		disk->time.minute,
		disk->time.second);
}
/* Initialise the backup system */
void BupInit(void)
{
	int i;
	gLoadQueue = gSaveQueue = gCheckQueue = 0;
	gLockedDrive = -1;

	memset(gBupInfo, 0, sizeof(gBupInfo));
	buInit(BUD_CAPACITY_1MB, BUD_USE_DRIVE_ALL, NULL, BupInitCallback);
	// Work out how many blocks we need
	bupNumBlocks = buCalcBackupFileSize(1, BUD_VISUALTYPE_NONE, sizeof (Options));

	for (i = 0; i < 8; ++i)
		gSaveBuf[i] = syMalloc (512 * bupNumBlocks);

}


/* Finalise the backup system */
void BupExit(void)
{
	int i;
	extern Material fadeMat, opaqueNonTextured, addMat;
	// Firstly wait for all pending transactions to finish
	do { 
		BupRun();

		// Turn off clipping
		kmSetUserClipping (&vertBuf, KM_OPAQUE_POLYGON, 0, 0, 255, 255);
		kmSetUserClipping (&vertBuf, KM_OPAQUE_MODIFIER, 0, 0, 255, 255);
		kmSetUserClipping (&vertBuf, KM_TRANS_POLYGON, 0, 0, 255, 255);

		// Register some dummy polys
		for (i = 0; i < 10; ++i) {
			rSetMaterial (&fadeMat);
			kmxxGetCurrentPtr (&vertBuf);
			kmxxStartVertexStrip (&vertBuf);
			
			kmxxSetVertex_0 (0xe0000000, 0, 0, 1, 0);
			kmxxSetVertex_0 (0xe0000000, 0, PHYS_SCR_Y, 1, 0);
			kmxxSetVertex_0 (0xe0000000, PHYS_SCR_X, 0, 1, 0);
			kmxxSetVertex_0 (0xf0000000, PHYS_SCR_X, PHYS_SCR_Y, 1, 0);
			
			kmxxReleaseCurrentPtr (&vertBuf);

			rSetMaterial (&opaqueNonTextured);
			kmxxGetCurrentPtr (&vertBuf);
			kmxxStartVertexStrip (&vertBuf);
			
			kmxxSetVertex_0 (0xe0000000, 0, 0, 1, 0);
			kmxxSetVertex_0 (0xe0000000, 0, PHYS_SCR_Y, 1, 0);
			kmxxSetVertex_0 (0xe0000000, PHYS_SCR_X, 0, 1, 0);
			kmxxSetVertex_0 (0xf0000000, PHYS_SCR_X, PHYS_SCR_Y, 1, 0);
			
			kmxxReleaseCurrentPtr (&vertBuf);
		}

		kmRender();
		kmFlipFrameBuffer();
		pdExecPeripheralServer();

	} while (gSaveQueue);
	do {} while (buExit() != BUD_ERR_OK);
	// Allow quits now, if they were disabled before
	suppressReset = FALSE;
}

// My unmount
void MyUnmount(Sint32 drive)
{
	bupPrintf ("Unmounting drive %d", drive);
	gLoadQueue &= ~ (1<<drive);
	gSaveQueue &= ~ (1<<drive);
	gCheckQueue &= ~ (1<<drive);
	if (gLockedDrive == drive) {
		gLockedDrive = -1;
		buUnmount (drive);
	}
}

/* Flush the backup system */
void BupFlush(void)
{
/*	int nActive;
	// We must wait for all current transfers to finish
	do {
		int i;
		nActive = 0;
		for (i = 0; i < 8; ++i)
			if (gBupInfo[i].Colour &&
				gBupInfo[i].Operation)
				nActive++;
	} while (nActive);
	*/
}

/*
 * Run the backup system for a frame
 */
void BupRun(void)
{
	int both, i;
	// Are we busy doing something?
	if (gLockedDrive != -1)
		return;
	both = gSaveQueue | gLoadQueue | gCheckQueue;
	// Otherwise find the first drive to mount
	for (i = 0; i < 8; ++i) {
		if (both & (1<<i)) {
			Sint32 err;
			bupPrintf ("Mounting disk %d", i);
			if ((err = buMountDisk (i, &gMountBuf, BUM_WORK_SIZE(BUD_CAPACITY_1MB, 1))) != BUD_ERR_OK) {
				bupPrintf ("Mount failed: %s", BupGetErrorString(err));
				MyUnmount (i);
			} else {
				gLockedDrive = -2;
			}
			return;
		}
	}
}

/* Return the info for a drive */
const BACKUPINFO* BupGetInfo(Sint32 drive)
{
	return (const BACKUPINFO*)&gBupInfo[drive];
}


/* Load a file */
Sint32 BupLoad(Sint32 drive, const char* fname, void* buf)
{
	return buLoadFile(drive, fname, buf, 0);
}

// Called whenever a disk change has been made to keep the disk info up to date
static void BupUpdateDiskInfo (Sint32 drive)
{
	Sint32 ret;
	BACKUPINFO *info = gBupInfo + drive;
	dAssert (gLockedDrive == drive, "erk");
	ret = buGetFileInfo(drive, VMS_FILENAME, &info->DiskInfo);
	if (ret == BUD_ERR_OK) {
		bupPrintf ("Updating data on drive %d", drive);
		memcpy (&BupWaitDrive[drive], &info->DiskInfo, sizeof (BUS_FILEINFO));
	} else {
		bupPrintf ("Error while updating data on %d: %s", drive, BupGetErrorString(ret));
		memset (&BupWaitDrive[drive], 0, sizeof (BUS_FILEINFO));
	}
}

/* Day enumeration */
enum {
	Sunday = 0,
	Monday,
	Tuesday,
	Wednesday,
	Thursday,
	Friday,
	Saturday
};

static BUS_TIME gBupTime = {
	1998, 12, 31,	
	23, 59, 59,		
	Thursday,		
};

Sint32 BupSave(Sint32 drive, const char* fname, void* buf, Sint32 nblock)
{
	SYS_RTC_DATE dateAndTime;
	Sint32 retVal;
	if (syRtcGetDate (&dateAndTime) == 0) {
		gBupTime.year		= dateAndTime.year;
		gBupTime.month		= dateAndTime.month;
		gBupTime.day		= dateAndTime.day;
		gBupTime.hour		= dateAndTime.hour;
		gBupTime.minute		= dateAndTime.minute;
		gBupTime.second		= dateAndTime.second;
		gBupTime.dayofweek	= dateAndTime.dayofweek;
	}
	retVal = buSaveFile(drive, fname, buf, nblock, &gBupTime,
							BUD_FLAG_VERIFY | BUD_FLAG_COPY(0));
	if (retVal == BUD_ERR_OK) {
		suppressReset++;
	} else {
		gSaveQueue &= ~ (1<<drive);
	}
	return retVal;
}

static Uint32 CalcTamper(Uint8 *data, Uint8 *end)
{
	Uint32 ckSum;
	static const Uint8 *str = "f0pessdf9843n&^g923trh`1u9f9wf2387hjk^｣4gbhji";
	Uint8 *s;
	ckSum = 0;
	s = str;
	while (data < end) {
		int i;
		Uint8 c;
		if (*s=='\0')
			s = str;
		c = *data++ ^ *s++;
		for (i = 0; i < 8; ++i) {
			ckSum = (ckSum<<1) ^ ((ckSum & 0x80000000) ? 0x04c11db7L : 0) ^ (c&1);
			c>>=1;
		}
	}
	return ckSum;
}

/*
 * Save some data to the drive as Red Dog data
 */

Bool SaveData(Sint32 drive, Options *buf, int nBytes)
{
	BUS_BACKUPFILEHEADER hdr;
	int nBlock;
	void *image;
	Sint32 retVal;
	int random, profile;
	SYS_RTC_DATE dateAndTime;

	// Ensure there isn't any panic action
	if (BupDrivePanic[drive]) {
		bupPrintf ("Attempt to write to a panicked drive (%d) averted", drive);
		SaveFailedCount[drive>>1] = WARN_FRAMES/32;
		gSaveQueue &= ~ (1<<drive);
		return false;
	}

	// Before we go any further, we need to wait for any current operations on this VMU to finish
	// Just for total paranoia stakes!
	while (gSaveQueue & (1<<drive)) {
		BupRun();		
	}

	// At this point, update the Anti Tamper Checksum for each player
	for (profile = 0; profile < 4; ++profile) {
		buf->game[profile].AntiTamperCheck = CalcTamper(
			(Uint8 *)&buf->game[profile], 
			(Uint8 *)&buf->game[profile].AntiTamperCheck);
	}

	// Clear to zero
	memset (&hdr, 0, sizeof (BUS_BACKUPFILEHEADER));

	// Set up the strings
	strcpy (hdr.vms_comment,		"RED DOG GAME DATA");
	strcpy (hdr.btr_comment,		"Red Dog");
	strcpy ((char *)hdr.game_name,	"Red Dog");

	// Graphics
	hdr.icon_palette		= SaveGraphic;
	hdr.icon_data			= SaveGraphic + 32;
	hdr.icon_num			= 1;
	hdr.icon_speed			= 1;
	hdr.visual_data			= NULL;
	hdr.visual_type			= BUD_VISUALTYPE_NONE;
	hdr.save_data			= buf;
	hdr.save_size			= nBytes;

	// Work out how many blocks we need
	nBlock = buCalcBackupFileSize(hdr.icon_num, hdr.visual_type, hdr.save_size);
	if (nBlock < 0) {
		lastBupError = BupGetErrorString (nBlock);
		return FALSE;
	}

	// Allocate some global memory
	image = gSaveBuf[drive];
	memset (image, 0, nBlock * 512);

	retVal = buMakeBackupFileImage (image, &hdr);
	if (retVal < 0) {
		lastBupError = BupGetErrorString (nBlock);
		return FALSE;
	}

	// Add to the save queue
	gSaveQueue |= (1<<drive);
//	retVal = BupSave (drive, VMS_FILENAME, image, nBlock);
}

/*
 * Load data
 */
Bool LoadData (Sint32 drive, Options *buf, int nBytes)
{
	int nBlocks, retVal;
	void *image;
	BACKUPINFO *loadInfo = &gBupInfo[drive];

	// Well, we know it's an options block
	InitOptions (buf);

	// Check there's a VMS present
	if (loadInfo->Colour == 0)
		return FALSE;

	// Wait for the drive to become ready XXX needs a timeout
	while (!loadInfo->Ready);

	// Load in the appropriate number of blocks
	nBlocks = buGetFileSize (drive, VMS_FILENAME);
	if (nBlocks <= 0 || nBlocks > bupNumBlocks) {
		lastBupError = BupGetErrorString (nBlocks);
		return FALSE; // Unable to load - file not found or similar
	}

	image = gSaveBuf[drive];

	retVal = BupLoad (drive, VMS_FILENAME, image);
	if (retVal < 0) {
		lastBupError = BupGetErrorString (retVal);
		return FALSE;
	}

	// Did we get an error?
	if (loadInfo->LastError != 0) {
		lastBupError = BupGetErrorString (loadInfo->LastError);
		return FALSE;
	}

	return TRUE;
}

/* Delete a file */
Sint32 BupDelete(Sint32 drive, const char* fname)
{
	return buDeleteFile(drive, fname);
}


/* Error strings */
const char* BupGetErrorString(Sint32 err)
{
	switch (err) {
		case BUD_ERR_OK:				return "OK";
		case BUD_ERR_BUSY:				return "BUSY";
		case BUD_ERR_INVALID_PARAM:		return "INVALID PARAMETER";
		case BUD_ERR_ILLEGAL_DISK:		return "ILLEGAL DISK";
		case BUD_ERR_UNKNOWN_DISK:		return "UNKNOWN DISK";
		case BUD_ERR_NO_DISK:			return "NO DISK";
		case BUD_ERR_UNFORMAT:			return "UNFORMAT";
		case BUD_ERR_DISK_FULL:			return "DISK FULL";
		case BUD_ERR_FILE_NOT_FOUND:	return "FILE NOT FOUND";
		case BUD_ERR_FILE_EXIST:		return "FILE EXIST";
		case BUD_ERR_CANNOT_OPEN:		return "CANNOT OPEN";
		case BUD_ERR_CANNOT_CREATE:		return "CANNOT CREATE";
		case BUD_ERR_EXECFILE_EXIST:	return "EXECUTABLE FILE EXIST";
		case BUD_ERR_ACCESS_DENIED:		return "ACCESS DENIED";
		case BUD_ERR_VERIFY:			return "VERIFY ERROR";
		default:						return "GENERIC ERROR";
	}
}


/* Current operation strings */
const char* BupGetOperationString(Sint32 op)
{
	switch (op) {
		case BUD_OP_MOUNT:			return "CONNECTED";
		case BUD_OP_UNMOUNT:		return "DISCONNECTED";
		case BUD_OP_FORMATDISK:		return "FORMATDISK";
		case BUD_OP_DELETEFILE:		return "DELETEFILE";
		case BUD_OP_LOADFILE:		return "LOADFILE";
		case BUD_OP_SAVEFILE:		return "SAVEFILE";
		default:					return "UNKNOWN OPERATION";
	}
}


/* Setup callbacks */
static void BupInitCallback(void)
{
	Sint32 i;
#if !GINSU
	/* 完了、経過コールバック関数を設定する */
	buSetCompleteCallback(BupComplete);
	buSetProgressCallback(BupProgress);
#endif
}


/* Completion callback */
static Sint32 BupComplete(Sint32 drive, Sint32 op, Sint32 stat, Uint32 param)
{
	BACKUPINFO* info;
	Sint32 ret;

	info = &gBupInfo[drive];

	switch (op) {
	case BUD_OP_CONNECT:// Request connection of a drive
		bupPrintf ("Connect on drive %d", drive);
		gCheckQueue |= (1<<drive);
//		ret = buMountDisk (drive, info->ScratchRAM, BUM_WORK_SIZE(BUD_CAPACITY_1MB, 1));
//		info->LastError = ret;
		break;
	case BUD_OP_MOUNT:			// We've finished mounting the drive (oo-er) 
		bupPrintf ("Mounted drive %d", drive);
		gLockedDrive = drive;
		info->LastError = BUD_ERR_OK;
		ret = buGetFileInfo(drive, VMS_FILENAME, &info->DiskInfo);
		if (ret == BUD_ERR_OK || ret == BUD_ERR_FILE_NOT_FOUND) {
			info->IsFormat = TRUE;
		} else {
			info->IsFormat = FALSE;
			bupPrintf ("Disk error: %s", BupGetErrorString (ret));
			if (gSaveQueue & (1<<drive))
				SaveFailed += WARN_FRAMES;
		}
		if (info->IsFormat && stat == BUD_ERR_OK) {
			info->Ready = TRUE;
			/* Clear last error */
			info->LastError = BUD_ERR_OK;
			info->Colour = 0xffffff; // Woo a drive
			bupDriveChange = TRUE;
			// Check the free space action
			IsEnoughSpace[drive] = _BupIsEnoughSpace(drive);
			/* Now its time to see what we wanted to do with this drive */
			if (gLoadQueue & (1<<drive)) {
				if (!LoadData (drive, &info->save, sizeof (info->save))) {
					// Unable to load data, unlock the drive and unmount it
					MyUnmount (drive);
				}
			} else if (gSaveQueue & (1<<drive)) {
				ret = BupSave (drive, VMS_FILENAME, gSaveBuf[drive], bupNumBlocks);
			} else if (gCheckQueue & (1<<drive)) {
				bupPrintf ("Requesting info on drive %d", drive);
				if (DriveRescan (drive)) {
					bupPrintf ("* Drive has new data, loading");
					if (!LoadData (drive, &info->save, sizeof (info->save))) {
						// Unable to load data, unlock the drive and unmount it
						bupPrintf ("* Unable to load from drive");
						MyUnmount (drive);
					} else {
						bupPrintf ("* Load issued OK");
					}
				} else {
					bupPrintf ("* Same data as already loaded, or problem detected");
					MyUnmount(drive);
				}
			} else {
				dAssert (FALSE, "Unknown op");
			}	
		} else {
			IsEnoughSpace[drive] = false;
			MyUnmount(drive);
		}
		break;
		
	case BUD_OP_UNMOUNT:		// Drive wrested from controller
		bupPrintf ("Forceably unmounted drive %d", drive);
		gLoadQueue &= ~ (1<<drive);
		gSaveQueue &= ~ (1<<drive);
		if (gLockedDrive == drive)
			gLockedDrive = -1;
		info->Colour = 0; // Drive gone
		info->Ready = FALSE;
		info->IsFormat = FALSE;
		info->ProgressCount = 0;
		info->ProgressMax = 0;
		info->Operation = 0;
		memset(&info->DiskInfo, 0, sizeof(BUS_FILEINFO));
		// Data could potentially have changed too
		bupDriveChange = bupDataChange = TRUE;
		// Let the notification code do its stuff
		DriveRescan (drive);
		// No longer have or need this VMU, so zero its saved disk info
		if (!BupDrivePanic[drive])
			memset (&BupWaitDrive[drive], 0, sizeof (BupWaitDrive[drive]));
		break;

	case BUD_OP_SAVEFILE:
		bupPrintf ("Save complete on drive %d", drive);
		info->LastError = stat;
		if (stat != BUD_ERR_OK) {
			SaveFailed += WARN_FRAMES;
			SaveFailedCount[drive>>1] = WARN_FRAMES/32;
		} else {
			SaveOKCount[drive>>1] = WARN_FRAMES/32;
		}
		suppressReset--;

		// Update the disk info
		bupPrintf ("Updating after save on disk %d", drive);
		BupUpdateDiskInfo(drive);

		dAssert (gLockedDrive == drive, "erk");
		MyUnmount (drive);

		break;

	case BUD_OP_LOADFILE:
		bupPrintf ("Load complete on drive %d", drive);
		{
			BUS_BACKUPFILEHEADER hdr;
			
			info->LastError = stat;

			if (stat == 0) {
				info->saveValid = TRUE;
				// Check for CRC issues etc
				if ((buAnalyzeBackupFileImage (&hdr, gSaveBuf[drive])) != 0) {
					info->saveValid = FALSE;
				} else {
					int profile;
					// Copy in as much as possible
					memcpy (&info->save, hdr.save_data, MIN (hdr.save_size, sizeof(info->save)));
				
					// Now check it's all valid
					if (memcmp (info->save.argo, "Argonaut", 8) != 0 ||
						info->save.version != OPTIONS_VERSION) 
						info->saveValid = FALSE;
					// Check each profile for tampering
					if (info->saveValid) {
						for (profile = 0; profile < 4; ++profile) {
							Uint32 shouldBe = CalcTamper((Uint8 *)&info->save.game[profile], (Uint8 *)&info->save.game[profile].AntiTamperCheck);
							if (strcmp (info->save.game[profile].name, EMPTY_PROFILE) && info->save.game[profile].AntiTamperCheck != shouldBe) {
								dAssert (FALSE, "Oopsie - detected a tamper!");
								ResetGame (&info->save.game[profile]);
							}
						}
					}
				}
				if (!info->saveValid) {
					InitOptions (&info->save);
				}
				bupDataChange = TRUE;
			}

			// Update the madness of the drive
			bupPrintf ("* Updating after load on drive %d", drive);
			BupUpdateDiskInfo(drive);

			dAssert (gLockedDrive == drive, "erk");
			MyUnmount (drive);
		}
		break;

	default:
		/* Otherwise, set up last error */
		info->LastError = stat;
		
		buGetFileInfo(drive, VMS_FILENAME, &info->DiskInfo);
	}

	/* No more operations */
	info->Operation = 0;

	return BUD_CBRET_OK;
}


/* Progress callback */
static Sint32 BupProgress(Sint32 drive, Sint32 op, Sint32 count, Sint32 max)
{
	BACKUPINFO* info;

	info = &gBupInfo[drive];

	info->ProgressCount = count;
	info->ProgressMax = max;
	info->Operation = op;

	return BUD_CBRET_OK;
}

/**********************************************************************************/

void BupRescan(void)
{
}

// Drive rescan logic
// Returns TRUE if new data needs to be loaded
bool DriveRescan(int drive)
{
	BACKUPINFO* info;
	Sint32 ret;
	
	info = &gBupInfo[drive];
	if (!BupDrivePanic[drive]) {
		// Has a locked drive been removed
		// (ie was previously inserted and then removed while in use)
		if (BupIsVMSLocked(drive) &&
			info->Colour == 0x00) {
			BupDrivePanic[drive] = TRUE;
			bupPrintf ("* No disk yet on rescan");
			return false; // Don't load
		} else {
			static BUS_FILEINFO dInfo;
			// Try and load a red dog save from the drive, if the data is different
			ret = buGetFileInfo(drive, VMS_FILENAME, &dInfo);
			if (ret == BUD_ERR_FILE_NOT_FOUND)
				return true;
			if (ret != BUD_ERR_OK) {
				bupPrintf ("* Error encountered on rescan of NONpanicked drive: %s", BupGetErrorString (ret));
				return false;
			}
			if (memcmp (&dInfo, &BupWaitDrive[drive], sizeof (BUS_FILEINFO)) != 0) {
				bupPrintf ("* New data detected on NONpanicked drive");
				bupDumpDisk (&dInfo);
				return true;
			}
			bupPrintf ("* Identical data detected on NONpanicked drive");
			BupDrivePanic[drive] = FALSE;
			// Take a copy of the disk info
			bupPrintf ("* Updating after deciding disk is the same");
			BupUpdateDiskInfo(drive);
			return false; // No loading needed
		}
	} else {
		// Drive panic!  Has the correct drive been reinserted
		static BUS_FILEINFO info;
		ret = buGetFileInfo(drive, VMS_FILENAME, &info);
		if (ret == BUD_ERR_FILE_NOT_FOUND)
			return true;
		if (ret == BUD_ERR_OK) {
			// Is it the right disk?
			if (memcmp (&info, &BupWaitDrive[drive], sizeof (BUS_FILEINFO)) == 0) {
				bupPrintf ("* Identical data detected on panicked drive");
				bupDumpDisk (&info);
				// Hooray..it's the same disk!
				BupDrivePanic[drive] = false;
				return false; // No loading
			} else {
				bupPrintf ("* Wrong disk inserted");
				return false; // For God's sake don't load
			}
		} else {
			bupPrintf ("* Error encountered on rescan of panicked drive: %s", BupGetErrorString (ret));
		}
	}
	return false;
}

static Bool ProfileLock[8][4];
static Bool vmsLock[8];

Bool BupIsVMSLocked (Sint32 drive)
{
	int i;
	dAssert (drive >= 0 && drive < 8, "Invalid drive");
	for (i = 0; i < 4; ++i) {
		if (ProfileLock[drive][i])
			return TRUE;
	}
	return vmsLock[drive];
}

Bool BupLockVMS (Sint32 drive)
{
	dAssert (drive >= 0 && drive < 8, "Invalid drive");
	vmsLock[drive] = TRUE;
	bupDataChange = TRUE;
}
Bool BupUnlockVMS (Sint32 drive)
{
	dAssert (drive >= 0 && drive < 8, "Invalid drive");
	if (vmsLock[drive]) {
		bupPrintf ("Previously panicked VM unlocked - reloading data on drive %d", drive);
		gCheckQueue |= (1<<drive);
	}
	vmsLock[drive] = FALSE;
	bupDataChange = TRUE;
}

Bool BupIsProfileLocked (Sint32 drive, Sint32 profile)
{
	dAssert (drive >= 0 && drive < 8 && profile >= 0 && profile < 4, "Invalid profile");
	return ProfileLock[drive][profile];
}

Bool BupLockProfile (Sint32 drive, Sint32 profile)
{
	dAssert (drive >= 0 && drive < 8 && profile >= 0 && profile < 4, "Invalid profile");
	if (ProfileLock[drive][profile] == FALSE && gBupInfo[drive].Colour) {
		ProfileLock[drive][profile] = TRUE;
		bupDataChange = TRUE;
		return TRUE;
	} else {
		return FALSE;
	}

}

void BupUnlockProfile (Sint32 drive, Sint32 profile)
{
	dAssert (drive >= 0 && drive < 8 && profile >= 0 && profile < 4, "Invalid profile");
	ProfileLock[drive][profile] = FALSE;
	bupDataChange = TRUE;
	// Now re-check the locks to see if a panicked drive has now been released
	if (BupDrivePanic[drive]) {
		int i;
		for (i = 0; i < 4; ++i) {
			if (ProfileLock[drive][i])
				break;
		}
		if (i == 4 && !vmsLock[drive]) {	// No locks on this drive
			BupDrivePanic[drive] = FALSE;
			// re-scan this drive
			gCheckQueue |= (1<<drive);
			bupPrintf ("Previously panicked VM unlocked - reloading data on drive %d", drive);
		}
	}
}

void BupGetProfile (Sint32 drive, Sint32 profile, GameSave *dest)
{
	dAssert (drive >= 0 && drive < 8 && profile >= 0 && profile < 4, "Invalid profile");
	dAssert (BupIsProfileLocked (drive, profile), "Attempt to read a non-locked profile");
	memcpy (dest, 
		&BupGetInfo (drive)->save.game[profile],
		sizeof (GameSave));
	// Sefton he say - no remember cheaties
	dest->activeSpecials &= ~SPECIAL_CHEATING_MASK;
	dest->cheatWeapon = 0;
}

Bool BupSetProfile (Sint32 drive, Sint32 profile, GameSave *src)
{
	dAssert (drive >= 0 && drive < 8 && profile >= 0 && profile < 4, "Invalid profile");
	dAssert (BupIsProfileLocked (drive, profile), "Attempt to write a non-locked profile");
	dAssert (!(
		src->name[0] == 'P' &&
		src->name[1] == 'l' &&
		src->name[2] == 'a' &&
		src->name[3] == 'y' &&
		src->name[4] == 'e' &&
		src->name[5] == 'r'
		), "Looks like we're trying to blat a profile!");
	dAssert (!(
		src->name[0] == '<' &&
		src->name[1] == 'E' &&
		src->name[2] == 'M' &&
		src->name[3] == 'P' &&
		src->name[4] == 'T' &&
		src->name[5] == 'Y'
		), "Looks like we're trying to blat a profile!");
	memcpy (&BupGetInfo (drive)->save.game[profile],
		src, 
		sizeof (GameSave));

	return (SaveData (drive, &BupGetInfo(drive)->save, sizeof (Options)));
}


Bool BupSaveTournament (Sint32 drive, Sint32 profile, TournamentGame *game)
{
	dAssert (drive >= 0 && drive < 8 && profile >= 0 && profile < 4, "Invalid profile");
	dAssert (BupIsVMSLocked (drive), "Attempt to write a non-locked profile");

	memcpy (&BupGetInfo (drive)->save.tournament[profile], game, sizeof (*game));

	return (SaveData (drive, &BupGetInfo(drive)->save, sizeof (Options)));
}

/*
 * Check to see if there's enough space, or an existing file on the VMU
 */
static Bool _BupIsEnoughSpace(Sint32 drive)
{
	Sint32 erVal;
	if (gBupInfo[drive].Colour == 0)
		return FALSE;
	switch (erVal = buIsExistFile (drive, VMS_FILENAME)) {
	case BUD_ERR_OK:
		// File exists: we can save
		bupPrintf ("Disk %d has an existing Red Dog save", drive);
		return TRUE;
	case BUD_ERR_FILE_NOT_FOUND:
		{
			Sint32 free;
			free = buGetDiskFree (drive, BUD_FILETYPE_NORMAL);
			bupPrintf ("Disk %d has %d free blocks", drive, free);
			if (free < bupNumBlocks)
				return FALSE; // Not enough space, or an error
			return TRUE; // There's enough space
		}
		break;
	default:
		bupPrintf ("Disk %d error on BupIsEnoughSpace() : %s", drive, BupGetErrorString(erVal));
		return FALSE; // Some other error...
	}

}

Bool BupIsEnoughSpace (Sint32 drive)
{
	dAssert (drive>=0 && drive<8, "Ack");
	return IsEnoughSpace[drive];
}