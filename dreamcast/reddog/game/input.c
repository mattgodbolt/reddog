/*
 * Input.c
 */
#include "RedDog.h"
#include "Input.h"
#include "player.h"
#include "level.h"
#include "GameState.h"
#include "VmsStruct.h"
#include <sg_lcd.h>
#include <sg_pdvib.h>







#define OUR_SUPPORT (PDD_DEV_SUPPORT_TX | PDD_DEV_SUPPORT_TY | PDD_DEV_SUPPORT_TA | PDD_DEV_SUPPORT_TB |\
					 PDD_DEV_SUPPORT_ST | PDD_DEV_SUPPORT_AL | PDD_DEV_SUPPORT_AR |\
					 PDD_DEV_SUPPORT_AX1 | PDD_DEV_SUPPORT_AY1)

#define RESET_CONDITION (PDD_DGT_ST | PDD_DGT_TA | PDD_DGT_TB | PDD_DGT_TX | PDD_DGT_TY)

#define LCD_FUDGE_FRAMES (2)

bool vibrationDisabled = false;

const Uint8 RedDogLogo[][48*32] = {
	{
#include "p:\graphics\vms\rd00.h"
	},
	{
#include "p:\graphics\vms\rd01.h"
	},
	{
#include "p:\graphics\vms\rd02.h"
	},
	{
#include "p:\graphics\vms\rd03.h"
	},
	{
#include "p:\graphics\vms\rd04.h"
	},
	{
#include "p:\graphics\vms\rd05.h"
	},
	{
#include "p:\graphics\vms\rd06.h"
	},
	{
#include "p:\graphics\vms\rd07.h"
	},
	{
#include "p:\graphics\vms\rd08.h"
	},
	{
#include "p:\graphics\vms\rd09.h"
	},
	{
#include "p:\graphics\vms\rd10.h"
	},
	{
#include "p:\graphics\vms\rd11.h"
	},
	{
#include "p:\graphics\vms\rd12.h"
	},
	{
#include "p:\graphics\vms\rd13.h"
	},
	{
#include "p:\graphics\vms\rd14.h"
	},
};

const Uint8 vmsSaveFailed[] = {
#include "p:\graphics\vms\SAVEFAIL.h"
};

const Uint8 vmsSaveOK[] = {
#include "p:\graphics\vms\SAVEOK.h"
};


int nLogoFrames = asize (RedDogLogo);

int PadFrame[4] = { 0, 3, 6, 9 };
const PDS_PERIPHERAL *pad[4];
Uint32 PadPress[4] = {0,0,0,0}, PadPush[4] = {0,0,0,0}, PadRelease[4] = {0,0,0,0};
Point2 joyState[4];
int		InputMode = NORMALINPUT;
int		PadOK = FALSE, PadReset = FALSE;
float	PlayerControlJoyX = 0.0f;
float	PlayerControlJoyY = 0.0f;
float	PlayerControlPlayerAcc = 0.0f;
Uint32	PlayerControlPadPress = 0;
Uint32	PlayerControlPadPush = 0;
static int lcdPort[4] = {-1,-1,-1,-1}, puruPort[4] = {-1,-1,-1,-1}, ctrlPort[4] = {-1,-1,-1,-1}, lcdOrientation[4];
static int lcdNeedsUpdate[4] = {0,0,0,0}, puruNeedsUpdate[4] = {0,0,0,0};
static PDS_VIBPARAM		puruData[4];
static PDS_VIBINFO		puruInfo[4];
#define MAX_VIBS 4
static PDS_VIBUNITINFO	puruVibInfo[4][MAX_VIBS];
static int puruCurPort;
static bool PadBroken[4];
int NumberOfControllers = 0;

static int puruPri[4], puruLength[4];
int SaveOKCount[4], SaveFailedCount[4];

static char lcdData[4][32*48];
static int lcdTimer[4];
static int CapturedPorts = 0;

INPUT *InputBase=NULL;	 
INPUT *InputBuffer=NULL;
CAMCOMMAND *CamBuffer=NULL;

extern	PLAYER player[4];

int CamMode;
int	CamCommands;
int	CurrentCamFrame;
int DEMOREC;
int    RecordedFrame = 0;
int	InactiveFrames = 0;
int suppressReset = 0;

const PDS_PERIPHERAL *FindPhysicalController(int i)
{
	int j;
	i*=6;
	for (j = 0; j < NumberOfControllers; ++j)
		if (ctrlPort[j] == i)
			return pad[j];
	return NULL;
}

void InitInput (void)
{
	int i=0;


	for (i=0; i<4; i++)
	{
		PadPress[i] = 0;
		PadPush[i] = 0;
		PadRelease[i] = 0;
		player[i].PlayerAcc=0.0f;
		joyState[i].x = 0.0f;
		joyState[i].y = 0.0f;
		PlayerControlClean(i);
		SaveOKCount[i] = 0;
		SaveFailedCount[i] = 0;
	}

	RecordedFrame = InactiveFrames = 0;
	InputClose();
	InputBase = InputBuffer = NULL;
	CamBuffer = NULL;
	CamMode = 0;
	CurrentCamFrame = 0;
	/*	DEMOREC = 0;*/
	PadReset = FALSE;
	PadOK = TRUE;
	/*	InputMode=NORMALINPUT;
	*/
}


static Sint32 puruEnumerator (Uint32 unit, Sint32 stat, 
							  PDS_VIBUNITINFO* info, Uint32 param)
{
	PDS_VIBUNITINFO *pInfo = (PDS_VIBUNITINFO *) param;
	if (unit > MAX_VIBS)
		return PDD_VIBRET_OK;

	pInfo[unit] = *info;

	return PDD_VIBRET_OK;
}

static void ScanBus(bool findNewCtrllers)
{
	int port, logPort, i, j;
	logPort = 0;
	if (findNewCtrllers) {
		/* Try and find a compatible controller */
		for (port = PDD_PORT_A0; port <= PDD_PORT_D5; ++port) {
			const PDS_PERIPHERALINFO *info;
			info = pdGetPeripheralInfo (port);
			if (info && info->type & PDD_DEVTYPE_CONTROLLER) {
				const PDS_PERIPHERAL *pad;
				/* Is it compatible? */
				pad = pdGetPeripheral (port);
				if (pad && ((pad->support & OUR_SUPPORT) == OUR_SUPPORT)) {
					// Is it a different controller?
					if (ctrlPort[logPort] != port) {
						lcdPort[logPort] = puruPort[logPort] = -1;
						ctrlPort[logPort] = port;
					}
					logPort++;
				}
			}
		}
		for (i = logPort; i < 4; ++i) {
			ctrlPort[i] = -1;
			lcdPort[i] = puruPort[i] = -1;
			pad[i] = NULL;
			lcdNeedsUpdate[i] = 0;
		}
	} else {
		logPort = NumberOfPlayers;
	}
	/* Search for an LCD/memory unit and Puru-puru pack for each controller */
	for (j = 0; j < logPort; ++j) {
		if (!findNewCtrllers) {
			i = LogToPhys(j);
			if (i == -1)
				continue; // Ignore this port
		} else {
			i = j;
		}
		for (port = ctrlPort[i] + 1; (port % 6); ++port) {
			const PDS_PERIPHERALINFO *info;
			info = pdGetPeripheralInfo (port);
			if (findNewCtrllers) {
				if (info->type & PDD_DEVTYPE_LCD) {
					Sint32 dir;
					Uint32 flag;
					/* Do we already know about this LCD device? */
					if (lcdPort[i] == port)
						break;
					lcdNeedsUpdate[i] = 1;
					lcdPort[i] = port;
					/* Set up the LCD device */
					dir = pdLcdGetDirection (port);
					switch (dir) {
					case PDD_LCD_DIRECTION_NORMAL: /* Normal */
						flag = PDD_LCD_FLAG_NOFLIP;
						break;
					case PDD_LCD_DIRECTION_FLIP: /* Because the LCD is upside down, */
						flag = PDD_LCD_FLAG_HVFLIP; /* the data is sent upside down */
						break;
					default: /* If the LCD is oriented sideways, */
						flag = PDD_LCD_FLAG_NOFLIP; /* the data is sent as is */
						break; /* as is */
					}
					lcdOrientation[i] = flag;
				} else {
					/* Not an LCD device: if it was the last recorded LCD device, -1 it */
					if (lcdPort[i] == port)
						lcdPort[i] = -1;
				}
			}
			if (info->type & PDD_DEVTYPE_VIBRATION) {
				Sint32 dir;
				Uint32 flag;
				/* Do we already know about this purupuru device? */
				if (puruPort[i] == port)
					break;
				puruPort[i] = port;
				// Get the puru information
				pdVibGetInfo(port, &puruInfo[i]);
				puruCurPort = port;
				pdVibEnumerateUnit (port, puruEnumerator, (Uint32) &puruVibInfo[i]);
			} else {
				/* Not a vibration device: if it was the last recorded vibration device, -1 it */
				if (puruPort[i] == port)
					puruPort[i] = -1;
			}
		}
	}
}

void lcdClear (int pad)
{
	/*memset (lcdData[pad], sizeof (lcdData[0]), 0);*/
	if (SaveOKCount[pad]) {
		SaveOKCount[pad]--;
		memcpy (lcdData[pad], vmsSaveOK, sizeof (RedDogLogo[PadFrame[pad]]));
	} else if (SaveFailedCount[pad]) {
		SaveFailedCount[pad]--;
		memcpy (lcdData[pad], vmsSaveFailed, sizeof (RedDogLogo[PadFrame[pad]]));
	} else
		memcpy (lcdData[pad], RedDogLogo[PadFrame[pad]], sizeof (RedDogLogo[PadFrame[pad]]));
}

static void lcdUpdateInternal (int logPort)
{
	const PDS_PERIPHERALINFO *info;

	if (lcdPort[logPort] == -1 || !lcdNeedsUpdate[logPort])
		return;
	if (lcdTimer[logPort] == 0) {
		lcdTimer[logPort] = LCD_FUDGE_FRAMES;
		return;
	}
	/* Check there's an lcd there still */
	info = pdGetPeripheralInfo (lcdPort[logPort]);
	if (info->type & PDD_DEVTYPE_LCD) {
		if (lcdPort[logPort] != -1) {
			if (lcdTimer[logPort] && --lcdTimer[logPort])
				return;
			if (pdVmsLcdIsReady (lcdPort[logPort]) < 0)
				return;
			if (pdVmsLcdWrite (lcdPort[logPort], lcdData[logPort], lcdOrientation[logPort]) == PDD_LCDERR_OK)
				lcdNeedsUpdate[logPort] = 0;	/* If write fails, try again next time */
		}
	} else {
		lcdPort[logPort] = -1;
		lcdNeedsUpdate[logPort] = 1;
	}
}

static void puruUpdateInternal (int logPort)
{
	const PDS_PERIPHERALINFO *info;

	puruPri[logPort] = -10000;

	if (puruLength[logPort]) {
		if (--puruLength[logPort] == 0) {
			puruNeedsUpdate[logPort] = 1;
			puruData[logPort].unit = 1;
			puruData[logPort].flag = 0;
			puruData[logPort].power = 0;
			puruData[logPort].freq = 0;
			puruData[logPort].inc = 0;
		}
	}

	if (puruPort[logPort] == -1 || !puruNeedsUpdate[logPort])
		return;

	/* Check there's an puru there still */
	info = pdGetPeripheralInfo (puruPort[logPort]);
	if (info->type & PDD_DEVTYPE_VIBRATION) {
		if (puruPort[logPort] != -1) {
			if (pdVibIsReady (puruPort[logPort]) < 0)
				return;
			if (pdVibStart (puruPort[logPort], &puruData[logPort], 1) == PDD_VIBERR_OK)
				puruNeedsUpdate[logPort] = 0;	/* If write fails, try again next time */
		}
	} else {
		puruPort[logPort] = -1;
		puruNeedsUpdate[logPort] = 1;
	}
}

void lcdUpdate (int logPort)
{
	lcdNeedsUpdate[logPort] = 1;
}

void lcdSetPixel (int i, int x, int y)
{
	dAssert (x>=0 && x < 48 && y>=0 && y <32, "Out of range");
	lcdData[i][x+48*y] = 0x08;
}

void lcdClrPixel (int i, int x, int y)
{
	dAssert (x>=0 && x < 48 && y>=0 && y <32, "Out of range");
	lcdData[i][x+48*y] = 0x00;
}
// Strat glue function
void Vibrate (STRAT *strat, int priority, float freq, float power, float length)
{
	int i = currentPlayer;
	if (strat->Player)
		i = strat->Player->n;
	puruVibrate (i, priority, freq, power, length);
}
void puruVibrate (int logPort, int priority, float freq, float power, float length)
{
	int newPow;
	float minFreq, maxFreq;
	Uint32 frames;

	if (vibrationDisabled || InputMode != NORMALINPUT)
		return;

	logPort = LogToPhys (logPort);

	if (priority <= puruPri[logPort])
		return;
	frames = (int)(length * (float)FRAMES_PER_SECOND);
	if (frames >= puruLength[logPort])
		puruLength[logPort] = frames;
	puruPri[logPort] = priority;

	minFreq = puruVibInfo[logPort][1].min_freq;
	maxFreq = puruVibInfo[logPort][1].max_freq;

	newPow = power * 8;
	if (newPow < -7)
		newPow = -7;
	if (newPow > 7)
		newPow = 7;
	puruNeedsUpdate[logPort] = 1;
	puruData[logPort].unit = 1;
	puruData[logPort].flag = puruLength[logPort] ? PDD_VIB_FLAG_CONTINUOUS : 0;
	puruData[logPort].power = newPow;
	puruData[logPort].freq = minFreq + (maxFreq - minFreq) * freq;
	puruData[logPort].inc = 0;
}

static Uint32 PhysToLog (Uint32 phys, Uint32 ctrl)
{
	Uint32 retVal = 0;

	// Direct mapped keys :
	if (phys & PDD_DGT_KU)
		retVal |= PADUP;
	if (phys & PDD_DGT_KD)
		retVal |= PADDOWN;
	if (phys & PDD_DGT_KL)
		retVal |= PADLEFT;
	if (phys & PDD_DGT_KR)
		retVal |= PADRIGHT;
	if (phys & PDD_DGT_TL)
		retVal |= PADBACKWARD | PADSHOULDERL;
	if (phys & PDD_DGT_TR)
		retVal |= PADFORWARD | PADSHOULDERR;
	if (phys & PDD_DGT_ST)
		retVal |= PADSTART;
	if (phys & PDD_DGT_TY)
		retVal |= PADY;

	// Configurable keys
	switch (ctrl) {
	case CTRL_AXB:
	default:
		if (phys & PDD_DGT_TA)
			retVal |= PADFIRE;
		if (phys & PDD_DGT_TX)
			retVal |= PADX;
		if (phys & PDD_DGT_TB)
			retVal |= PADB;
		break;
	case CTRL_ABX:
		if (phys & PDD_DGT_TA)
			retVal |= PADFIRE;
		if (phys & PDD_DGT_TB)
			retVal |= PADX;
		if (phys & PDD_DGT_TX)
			retVal |= PADB;
		break;
	case CTRL_XAB:
		if (phys & PDD_DGT_TX)
			retVal |= PADFIRE;
		if (phys & PDD_DGT_TA)
			retVal |= PADX;
		if (phys & PDD_DGT_TB)
			retVal |= PADB;
		break;
	case CTRL_XBA:
		if (phys & PDD_DGT_TX)
			retVal |= PADFIRE;
		if (phys & PDD_DGT_TB)
			retVal |= PADX;
		if (phys & PDD_DGT_TA)
			retVal |= PADB;
		break;
	case CTRL_BAX:
		if (phys & PDD_DGT_TB)
			retVal |= PADFIRE;
		if (phys & PDD_DGT_TA)
			retVal |= PADX;
		if (phys & PDD_DGT_TX)
			retVal |= PADB;
		break;
	case CTRL_BXA:
		if (phys & PDD_DGT_TB)
			retVal |= PADFIRE;
		if (phys & PDD_DGT_TX)
			retVal |= PADX;
		if (phys & PDD_DGT_TA)
			retVal |= PADB;
		break;
	}

	return retVal;
}

static void InputCheckInternal(int logPort)
{
	float tempf;
	int physPort;

	if (ctrlPort[logPort] != -1) {
		
		/* First, try to get the peripheral info */
		pad[logPort] = pdGetPeripheral (ctrlPort[logPort]);
		
		if (pad[logPort] == NULL || pad[logPort]->id == 0 || ((pad[logPort]->support & OUR_SUPPORT) != OUR_SUPPORT)) {
			ctrlPort[logPort] = -1;
		} else {
			
			// Check for reset condition on *any* compatible controller
			if (!suppressReset) {
				if (GetGameState() != GS_GAME ||
					(InputMode == DEMOREAD) ||
					(CapturedPorts & (1<<ctrlPort[logPort]))) {
					if ((pad[logPort]->on & RESET_CONDITION) == RESET_CONDITION)
						PadReset = TRUE;
				}
			}
#if !GINSU
			if (InputMode == DEMOREAD &&
				!ChangingState() && 
				(pad[logPort]->on & PDD_DGT_ST)) {
				CancelledTitleLoop();
				FadeToNewState(FADE_TO_BLACK_FAST, GS_TITLEPAGE);
			}
#else
			if (InputMode == DEMOREAD &&
				!ChangingState() && 
				(pad[logPort]->on))
				FadeToNewState(FADE_TO_BLACK_FAST, GS_ADVERT);
#endif
			// Reset InactiveFrames if necessary
			if (pad[logPort]->on ||
				(pad[logPort]->l > 16) ||
				(pad[logPort]->r > 16) ||
				(abs(pad[logPort]->x1) > 16) ||
				(abs(pad[logPort]->y1) > 16))
				InactiveFrames = 0;
			
			lcdUpdateInternal(logPort);
			puruUpdateInternal(logPort);

			physPort = logPort;
			logPort = MapPhysicalToLogical (logPort);
			if (logPort == -1)
				return;
			
			PadPress[logPort]	= PhysToLog (pad[physPort]->on, CurProfile[physPort].controller & CTRL_MASK);
			PadPush[logPort]	= PhysToLog (pad[physPort]->press, CurProfile[physPort].controller & CTRL_MASK);
			PadRelease[logPort]	= PhysToLog (pad[physPort]->release, CurProfile[physPort].controller & CTRL_MASK);
			joyState[logPort].x	= ((float)(pad[physPort]->x1)) / 128.0f;
			joyState[logPort].y	= ((float)(pad[physPort]->y1)) / 128.0f * ((CurProfile[physPort].controller & CTRL_NOTINV) ? -1 : 1);
			tempf = ((((float)(pad[physPort]->r)) * 0.003921568f) - (((float)(pad[physPort]->l)) * 0.003921568f));
			if (tempf > 0.0f)
				player[logPort].PlayerAcc = pow(tempf, 4);
			else
				player[logPort].PlayerAcc = -pow(tempf, 4);
			
			if (MP_TRANSITION_TO_GAME == MPStartupState) {
				if (!Multiplayer) {
					PadPress[logPort] &= (PADUP|PADDOWN|PADSTART);
					PadPush[logPort] &= (PADUP|PADDOWN|PADSTART);
					PadRelease[logPort] &= (PADUP|PADDOWN|PADSTART);
				} else {
					PadPress[logPort] = PadPush[logPort] = PadRelease[logPort] = 0;
					joyState[logPort].x	= 0.f;
					joyState[logPort].y	= 0.f;
				}
				player[logPort].PlayerAcc = 0;
			} else if ((player[logPort].PlayerState & PS_ONPATH) || (player[logPort].PlayerState & PS_SCRIPTHELD))
				
			{
				joyState[currentPlayer].x = PlayerControlJoyX;
				joyState[currentPlayer].y = PlayerControlJoyY;
				PadPress[currentPlayer] = PlayerControlPadPress;
				if (!(player[logPort].PlayerState & PS_SCRIPTHELD))
					PadPush[currentPlayer] = PlayerControlPadPush;
				
				player[currentPlayer].PlayerAcc = PlayerControlPlayerAcc;
				PlayerControlJoyX = 0.0f;
				PlayerControlJoyY = 0.0f;
				PlayerControlPadPress = 0;
				PlayerControlPadPush = 0;
			}
		}
	}

	switch (InputMode)
	{
	case (DEMOREAD):
		if (InputBuffer)
		{
			PadPush[logPort] = InputBuffer->padpush;
			PadPress[logPort] = InputBuffer->padpress;
			joyState[logPort] = InputBuffer->joystate;
			player[logPort].PlayerAcc = InputBuffer->PlayerAcc;
			if (InputBuffer[100].padpush == 0xffffffff || (currentFrame > 200 && (InputBuffer[100].padpush & PADSTART))) {
				if (!ChangingState()) {
#if !GINSU
					FadeToNewState (FADE_TO_BLACK, GS_TITLEPAGE);
#else
					FadeToNewState (FADE_TO_BLACK, GS_ADVERT);
#endif
					
				}
			} else
				InputBuffer++;
		}
		if (CamMode == CAMSCRIPT)
		{
			if (CurrentCamFrame <= 0)
			{
				PadPress[1] = (1<<CamBuffer->camnum);
				CurrentCamFrame = CamBuffer->numframes;
				CamBuffer++;
			}
			else
			{
//				if (PadPress[1]) wtf?
//					CamMode = 0;
				
			}
			
			CurrentCamFrame--;
			
		}
		break;
		
	case (DEMOWRITE):
		if (!InputBase)
			InputBase = InputBuffer=(INPUT*)SHeapAlloc (MapHeap,sizeof(INPUT)*MAXKEYS);
		
		if (RecordedFrame < MAXKEYS)
		{
			if (PadReset)
				InputBuffer->padpush = 0xffffffff;
			else
				InputBuffer->padpush = PadPush[logPort];
			
			InputBuffer->padpress = PadPress[logPort];
			InputBuffer->joystate = joyState[logPort];
			InputBuffer->PlayerAcc = player[logPort].PlayerAcc;
			InputBuffer++;
		}
		else
			tPrintf(10,25, "DEMO OVER");
		
		RecordedFrame++;
		
		
		break;
	default :
		break;		
	}

	player[logPort].smoothJoyX += (joyState[logPort].x - player[logPort].smoothJoyX) * 0.2f;
	player[logPort].smoothJoyY += (joyState[logPort].y - player[logPort].smoothJoyY) * 0.2f;
	player[logPort].smooothJoyX += (joyState[logPort].x - player[logPort].smooothJoyX) * 0.05f;
	player[logPort].smooothJoyY += (joyState[logPort].y - player[logPort].smooothJoyY) * 0.05f;

	

}

void CapturePort (int port)
{
	CapturedPorts |= (1<<ctrlPort[port]);
}
void ReleasePort (int port)
{
	CapturedPorts &= ~(1<<ctrlPort[port]);
}

void InputCheck (void)
{
	int i, numConnected = 0, j;

	InactiveFrames ++;

	if (!ChangingState() || (InputMode == DEMOREAD))
	{
		/* Does the bus need scanning? ..yes it always does outside of the game*/
		if (GetGameState() != GS_GAME ||
			CapturedPorts == 0 ||
			PadOK == FALSE)
			ScanBus(true);
		else
			ScanBus(false);
		
		PadOK = TRUE;
		PadReset = FALSE;
		for (i = 0; i < 4; ++i)
		{
			const PDS_PERIPHERAL *padTmp;
			int j;
			
			InputCheckInternal(i);
			
			if (InputMode == DEMOREAD) {
				if (ctrlPort[i] == -1) {
					PadRelease[i] = 0;
					ctrlPort[i] = lcdPort[i] = -1;
					lcdNeedsUpdate[i] = 1;
				}
			} else {
				int physCtrl;
				if (ctrlPort[i] != -1)
					numConnected++;
				else
				{
					PadPress[i] = PadPush[i] = PadRelease[i] = 0;
					ctrlPort[i] = lcdPort[i] = -1;
					lcdNeedsUpdate[i] = 1;
				}
			}
		}	 
		// Check the physical pads against the captured ports
		for (i = 0; i < 4*6; i+=6) {
			if (CapturedPorts & (1<<i)) {
				int j;
				for (j = 0; j < 4; ++j) {
					if (ctrlPort[j] == i)
						break;
				}
				if (j==4) {
					PadOK = FALSE;
					PadBroken[i/6] = TRUE;
				} else
					PadBroken[i/6] = FALSE;
			}
		}
		
		/*
		 * If there are no controllers at all present, then PadOK = FALSE;
		 */
		if (numConnected == 0 && (InputMode != DEMOREAD))
			PadOK = FALSE;
		NumberOfControllers = numConnected;
	} else {
		// Changing state - release all the buttons
		for (i = 0; i < 4; ++i)
			PadPress[i] = PadPush[i] = PadRelease[i] = 0;
	}
	if (NoMainGun)
	{
		PadPush[0] &= ~PADFIRE;
		PadPress[0] &= ~PADFIRE;
	}
}

// Is a pad needed to get PadOK TRUE?
bool PadNeeded(int i)
{
	return PadBroken[i];
}

void InputClose()
{
	register int i;


	if (InputBase && InputMode == DEMOWRITE)
	{
		int nBytes, fd;
		char *ptr;
		char buffer[64];
#ifndef _RELEASE
		// Now the data is saved to c:\Dreamcast\reddog\game\<level>.log
#if GODMODEMACHINE
		_chdir ("d:\\dreamcast\\reddog\\game");
#else
	#ifndef _ARTIST
		_chdir ("c:\\dreamcast\\reddog\\game");
	#endif
#endif
		if (Multiplayer) {
			extern char *MPWadName[];
			sprintf (buffer, "%s.log", MPWadName[LevelNum]);
		} else
			sprintf (buffer, "%s.log", Levels[LevelNum]);
		#ifndef _ARTIST
			fd = _open (buffer, 0x261); // 261 == Write only | create | truncate | binary
			dAssert (fd != -1, "Unable to open output file");
			nBytes = (int)((char *)InputBuffer - (char *)InputBase);
			ptr = (char *)InputBase;
			do {
				int nToDo = MIN(32*1024, nBytes);
				_write (fd, ptr, nToDo);
				nBytes-= nToDo;
				ptr+= nToDo;
			} while (nBytes);
			_close (fd);
		#endif
#endif

		InputBase = NULL;
		RecordedFrame = 0;
		InputMode = 0;
	}

	InputBuffer = NULL;
	for (i = 0; i < 4; ++i)
		PlayerControlClean(i);

}

int PadMapTable[4];

int MapPhysicalToLogical (int physical)
{
	return PadMapTable[physical];
}

// Given a number between 0 and NumberOfControllers,
// return the actual physical port number [0..3]
int GetPortNumber (int i)
{
	return ctrlPort[i] / 6;
}