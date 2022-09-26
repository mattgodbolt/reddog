#ifndef _BACKUP_H_
#define _BACKUP_H_

#include <sg_bup.h>
#include "VMSStruct.h"

#define VMS_FILENAME "RED_DOG__SYS"

/* Info structure */
typedef struct {
	Uint16 Ready;               /* Is it ready? */
	Uint16 IsFormat;            /* Is it formatted?*/
	Uint32 Colour;				/* The colour of the VMS, or 0 if none */
	Uint32 LastError;           /* Number of the last error */
	Uint32 ProgressCount;       /* how finished we are */
	Uint32 ProgressMax;         /* What 'finished' is */
	Uint32 Operation;           /* What we're doing atm */
	BUS_FILEINFO DiskInfo;      /* Disk info */
	Bool	saveValid;			/* Is the following save data valid? */
	Options	save;				/* The Red Dog save on this drive */
} BACKUPINFO;

#define EMPTY_PROFILE ((const char *)LangLookup("EMPTY_PROFILE"))

void BupInit(void);
void BupExit(void);
void BupFlush(void);
const BACKUPINFO* BupGetInfo(Sint32 drive);
const char* BupGetErrorString(Sint32 err);
const char* BupGetOperationString(Sint32 op);
Sint32 BupLoad(Sint32 drive, const char* fname, void* buf);
Sint32 BupSave(Sint32 drive, const char* fname, void* buf, Sint32 nblock);
Sint32 BupDelete(Sint32 drive, const char* fname);
extern Bool bupDriveChange, bupDataChange, BupDrivePanic[8];
extern int bupNumBlocks;

Bool BupIsProfileLocked (Sint32 drive, Sint32 profile);
Bool BupLockProfile (Sint32 drive, Sint32 profile);
void BupUnlockProfile (Sint32 drive, Sint32 profile);

Bool BupSetProfile (Sint32 drive, Sint32 profile, GameSave *src);
void BupGetProfile (Sint32 drive, Sint32 profile, GameSave *dest);
Bool BupSaveTournament (Sint32 drive, Sint32 profile, TournamentGame *game);

Bool BupIsEnoughSpace(Sint32 drive);
Bool BupIsVMSLocked (Sint32 drive);
Bool BupLockVMS (Sint32 drive);
Bool BupUnlockVMS (Sint32 drive);

void BupRun(void);
void BupRescan(void);
extern int SaveFailed;
#define WARN_FRAMES	600

#endif /* _BACKUP_H_ */

/******************************* end of file *******************************/
