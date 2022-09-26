#include "RedDog.h"
#include "view.h"
#include "GameState.h"
#include "input.h"
#include "render\map.h"
#include "Level.h"
#include "Render\internal.h"
#include "camera.h"
#include "backup.h"
#include "Render\Shadow.h"
#include "Render\SpecialFX.h"
#include "coll.h"
#include "mplayer.h"
#include "Loading.h"
#include "LevelDepend.h"
#include "Layers.h"




extern const char *LangLookup (const char *token);
extern float adjustedX, adjustedY;
static int NumPlayedMPGames = 0;
int ExitedThroughReset;
bool choosed;

/*float a, b, c;*/
//float FPS, calcFPS;
//extern int nDrawn;
//extern Uint32 ColLerp (Uint32, Uint32, float);
/*int id, i, tumble = 1;*/
//PNode *node[5], *head;
/*void gameHandler (GameState thisState, GameStateReason reason) {}*/
//int		LevTextureNum;

Uint32 SavedPadPress, SavedPadPush;
float SavedJoyY;
int	frameCount = 0;
static bool MapWasRandom, GameWasRandom, CanPause;
STRAT DummCam, DummPlayer;
extern bool suppressDebug;
static float RotAmt = 0, TargetRot = 0, SpinSpeed, SpinAcc = 0, NumBombs, BombStart, BombTeam;
extern int BombCounter;
static int MPCountIn, magicWaitCount;
static bool BeginningOfTheEnd;
static int EndCounter;
bool BeingBriefed = false;
extern int		currentDrawTarget;
int PausedBy = 0;
Uint32 GetTankColour (int i);
extern volatile int DisableTexDMAs;
typedef struct {
	int		player;
	Point2	pos;
	Point2	targetPos;
	Point2	startPos;
} Bomb;

static Bomb BombArray[4];
static bool PauseMenuNeedsInitialising = TRUE;
static int DebriefPage = 0;
static void	RunDebriefingAction();

extern GameSave	CurProfile[4];

#define STARTROT (-PI/2)

#if 0
static char buggeryLog[200*1024], *buggeryLogPtr;
void RandLog(char *rout)
{
	if (frameCount < 0x10)
		buggeryLogPtr+=sprintf(buggeryLogPtr, "%d:%s:%d\n",frameCount, rout, rand());
} 
#define ddLog(a,b) //buggeryLogPtr+=sprintf(buggeryLogPtr, a,b);
#define dddLog(a,b,c) //buggeryLogPtr+=sprintf(buggeryLogPtr, a,b,c);
#endif

Stream *s;
Map *m;
float health = 0.5f;

int	score = 0;
int PauseMode = 0;
int FramesNeedingProcessing = 0;

int TimerDirection = 1, TimerTime, TimerShow = 0;
void RecurseBodgeColours (Object *o, Uint32 colour);

int DamageTaken = 0;
static int DamLastFrame;

PNode	*MultiTank[MAX_TANK][3];			// The four loaded multiplayer tanks
PNode	*MPArrow;							// The arrow

#define TIMEOUT (30*30000)

bool Multiplayer = FALSE;
int GameTime, BossTime;

short	DemoOn = 0;

extern	float	ambientL;
extern	Uint8	hLightState;

#define PUSH_MATRIX() { Matrix32 pushedMatrix; memcpy (&pushedMatrix, psGetMatrix(), sizeof (Matrix32))
#define POP_MATRIX() memcpy (psGetMatrix(), &pushedMatrix, sizeof (Matrix32)); }

Map		*map;
Map		*Savmap;
//short   ATTRACTSEQ=0;
//Bool	dayTime;

//short int	Attract[4]={0,1,2,3};

#if (!AUDIO64 || GDDA_MUSIC)
static int MusicNumber[] = 
{ 3, 3, 3, 3, 3, 4, 4, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
#else
static int MusicNumber[] = 
{ 0, 3, 0, 0, 0, 1, 2, 3, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3 };
#endif

static Material titlePageMat = { 0, 0, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_FRONT_END };
Material	bombMat	= { 0, 0, 0.f, 0.f, MF_ALPHA | MF_TEXTURE_NO_FILTER };
Material	amPred	= { 0, 0, 0.f, 0.f, MF_ALPHA | MF_TEXTURE_NO_FILTER };
Material	amKing	= { 0, 0, 0.f, 0.f, MF_ALPHA | MF_TEXTURE_NO_FILTER };
Material	amFlag	= { 0, 0, 0.f, 0.f, MF_ALPHA | MF_TEXTURE_NO_FILTER };
Material	amBomb	= { 0, 0, 0.f, 0.f, MF_ALPHA | MF_TEXTURE_NO_FILTER };
Material	textureSecondaryWeapon1	= { 0xa5ffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_NO_FILTER | MF_ALPHA };
Material	textureSecondaryWeapon2	= { 0xa5ffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_NO_FILTER | MF_ALPHA };
Material	textureSecondaryWeapon3	= { 0xa5ffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_NO_FILTER | MF_ALPHA };
Material	textureSecondaryWeapon4	= { 0xa5ffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_NO_FILTER | MF_ALPHA };
Material	textureSecondaryWeapon5	= { 0xa5ffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_NO_FILTER | MF_ALPHA };
Material	textureSecondaryWeapon6	= { 0xa5ffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_NO_FILTER | MF_ALPHA };
Material	textureSecondaryWeapon7	= { 0xa5ffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_NO_FILTER | MF_ALPHA };
Material	textureSecondaryWeapon9	= { 0xa5ffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_NO_FILTER | MF_ALPHA };
Material	textureSecondaryWeapon10	= { 0xa5ffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_NO_FILTER | MF_ALPHA };
Material	textureSecondaryWeapon11	= { 0xa5ffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_NO_FILTER | MF_ALPHA };
Material	textureSecondaryWeapon12	= { 0xa5ffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_NO_FILTER | MF_ALPHA };
Material	textureSecondaryWeapon13	= { 0xa5ffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_NO_FILTER | MF_ALPHA };

bool ChallengeSmashed(int level)
{
	char buf[64];
	int limit;
	sprintf(buf, "MISSION%d_LIMIT", level+1);
	limit = atoi((char *)LangLookup(buf));
	// ONLY ON EASY - HARD CHALLENGES NO NO NO!!
	// XXX LEVEL DEPENDENCY
	if (level == LEV_DEFENSIVE) {
		if (CurProfile[0].missionData[0][level].bestScore >= limit)
			return true;
	} else {
		if (CurProfile[0].missionData[0][level].bestTime < limit)
			return true;
	}
	return false;
}

static void Jingle(void)
{
	SFX_PassBombFE();
}
static bool Bingled;
static void Bingle(void)
{
	if (!Bingled)
		sfxPlay (GENERIC_ACCEPT_SOUND, 1, 0);
	Bingled = true;
}

static void MPChosen(void)
{
	choosed = true;
	sfxPlay (GENERIC_TOGGLEEND, 1, 0);
}

static Uint32 TimeColour (int time)
{
	if (time > 10*FRAMES_PER_SECOND) {
		return 0xff00ff00;
	} else if (time > 5*FRAMES_PER_SECOND) {
		return 0xffff0000;
	} else {
		if (currentFrame & 4)
			return 0xffff0000;
		else
			return 0x40ff0000;
	}
}

static Uint32 MPTimeColour (int time)
{
	int i;
	if (GetCurrentGameType() == KING_OF_THE_HILL) {
		for (i = 0; i < NumberOfPlayers; ++i)
			if (player[i].Player && player[i].special && player[i].timer == 1)
				break;
	} else if (GetCurrentGameType() != SUICIDE_TAG) {
		for (i = 0; i < NumberOfPlayers; ++i)
			if (player[i].Player && player[i].special)
				break;
	} else {
		for (i = 0; i < NumberOfPlayers; ++i)
			if (player[i].Player && !player[i].special)
				break;
	}
	if (i == NumberOfPlayers)
		return -1; // Bright white
	if (time > 5*FRAMES_PER_SECOND) {
		return 0xff000000 | GetTankColour (i);
	} else {
		if (currentFrame & 4)
			return 0xff000000 | GetTankColour (i);
		else
			return 0x40000000 | (0xffffff & GetTankColour (i));
	}
}

extern Material youWinMat, mpHUDMat, textureGreenBar, textureCharging, textureTargettingMessage, 
		textureOrangeBar, textureRedBar, textureLockOnMessage, textureNoTargetMessage;

int MPStartupState = 0;

static char *GameBlurb[14][2] =
{
	{ "MP_DM_PLURAL", "MP_DM_SINGLE" },
	{ "MP_KO_PLURAL", "MP_KO_SINGLE" },
	{ "MP_BT_PLURAL", "MP_BT_SINGLE" },
	{ "MP_SB_PLURAL", "MP_SB_SINGLE" },
	{ "MP_SA_PLURAL", "MP_SA_SINGLE" },
	{ "MP_FR_PLURAL", "MP_FR_SINGLE" },
	{ "MP_KH_PLURAL", "MP_KH_SINGLE" },
	// Teams
	{ "MPT_DM_PLURAL", "MPT_DM_SINGLE" },
	{ "MPT_KO_PLURAL", "MPT_KO_SINGLE" },
	{ "MPT_BT_PLURAL", "MPT_BT_SINGLE" },
	{ "MPT_SB_PLURAL", "MPT_SB_SINGLE" },
	{ "MPT_SA_PLURAL", "MPT_SA_SINGLE" },
	{ "MPT_FR_PLURAL", "MPT_FR_SINGLE" },
	{ "MPT_KH_PLURAL", "MPT_KH_SINGLE" },
};

static struct {
	Material		*mat;
	Uint32			GBIX;
} FixUpList[] = {	
	{ &bombMat,
#include "p:\game\texmaps\HUD\Multiplaya\Exclam0000.h"
	},
	{ &amPred,
#include "p:\game\texmaps\HUD\Multiplaya\Predator0000.h"
	},
	{ &amKing,
#include "p:\game\texmaps\HUD\Multiplaya\AmKing.h"
	},
	{ &amFlag,
#include "p:\game\texmaps\HUD\Multiplaya\AmFlag.h"
	},
	{ &amBomb,
#include "p:\game\texmaps\HUD\Multiplaya\AmBomb.h"
	},
	{ &mpHUDMat,
#include "p:\game\texmaps\HUD\newHUD\MPHudAll.h"
	},
	{ &youWinMat,
#include "p:\game\texmaps\HUD\youwin.h"
	},
	{ &textureGreenBar,
#include "p:\game\texmaps\HUD\greenBar.h"
	},
	{ &textureCharging,
#include "p:\game\texmaps\HUD\Charging.h"
	},
	{ &textureTargettingMessage, 
#include "p:\game\texmaps\hud\TargettingMessage.h"
	},
	{ &textureRedBar, 
#include "p:\game\texmaps\hud\redBar.h"
	},
	{ &textureOrangeBar, 
#include "p:\game\texmaps\hud\orangeBar.h"
	},
	{ &textureLockOnMessage,
#include "p:\game\texmaps\hud\LockOnMessage.h"
	},
	{ &textureNoTargetMessage, 
#include "p:\game\texmaps\hud\NoTargetMessage.h"
	},

};

static struct {
	Material		*mat;
	Uint32			GBIX;
} FixUpListObj[] = {	
	{ &textureSecondaryWeapon1, 
#include "p:\game\texmaps\hud\secondaryweapon1.h"
	},
	{ &textureSecondaryWeapon2, 
#include "p:\game\texmaps\hud\secondaryweapon2.h"
	},
	{ &textureSecondaryWeapon3, 
#include "p:\game\texmaps\hud\secondaryweapon3.h"
	},
/*	{ &textureSecondaryWeapon4, 
#include "p:\game\texmaps\hud\secondaryweapon4.h"
	},*/
	{ &textureSecondaryWeapon5, 
#include "p:\game\texmaps\hud\secondaryweapon5.h"
	},
	{ &textureSecondaryWeapon6, 
#include "p:\game\texmaps\hud\secondaryweapon6.h"
	},
	{ &textureSecondaryWeapon7, 
#include "p:\game\texmaps\hud\secondaryweapon7.h"
	},
/*	{ &textureSecondaryWeapon9, 
#include "p:\game\texmaps\hud\secondaryweapon9.h"
	},*/
	{ &textureSecondaryWeapon10, 
#include "p:\game\texmaps\hud\secondaryweapon10.h"
	},
	{ &textureSecondaryWeapon11, 
#include "p:\game\texmaps\hud\secondaryweapon11.h"
	},
	{ &textureSecondaryWeapon12, 
#include "p:\game\texmaps\hud\secondaryweapon12.h"
	},
	{ &textureSecondaryWeapon13, 
#include "p:\game\texmaps\hud\secondaryweapon13.h"
	},
};

void FadeFromBlack(int pn, int n)
{
	
	fadeN[pn] = 0;
	fadeLength[pn] = n;
	fadeMode[pn] = FADE_FROM_BLACK;
}

static void ShowCameraInView(int view)
{
	extern STRAT *CamTracks[MAXCAM];
	STRAT *parent;

	parent = CamTracks[3 - view];
	if (parent == NULL)
		parent = CamTracks[0];
	dAssert (parent != NULL, "No camtracks!");

	player[view].CameraStrat = &DummCam;
	player[view].reward = 0;

	if (player[view].CameraStrat->parent == parent)
		return;

	player[view].CameraStrat->parent = parent;
	
	player[view].CameraStrat->parent->flag |= STRAT_HIDDEN;
	
	if (player[view].CameraStrat->parent && player[view].CameraStrat->parent->route &&
		player[view].CameraStrat->parent->route->path == NULL)
		InitPath(player[view].CameraStrat->parent);
	FadeFromBlack(view, 15);
}

static void FireHerUp(void)
{	
	// DES: this is where I put the code; it's called to start up the game
	// from the multiplayer
	SoundFade(0.f,0);
	if (MPStartupState != MP_FADE_OUT_TO_BLACK) {
		MPStartupState = MP_FADE_OUT_TO_BLACK;
		fadeN[0] = 0;
		fadeLength[0] = 15;
		fadeMode[0] = FADE_TO_BLACK;
	} else if (fadeN[0] > 18) {
		int i;
		MPCountIn = -15;
		for (i = 0; i < 4; ++i) { // yes, four
			player[i].active = FALSE; // disable the dummy player
		}
		for (i = 0; i < NumberOfPlayers; ++i) {
			int lod;
			Uint32 colour = GetTankColour(i);
			for (lod = 0; lod < 3; ++lod)
				RecurseBodgeColours (MultiTank[i][lod]->obj, colour);
		}
		// Set up the camera track in the last window 
		if (NumberOfPlayers == 3) {
			ShowCameraInView (3);
		}
		MPStartupState = MP_TRANSITION_TO_GAME;
		for (i = 0; i < 120; ++i) {
			bool nar = suppressDebug;
			suppressDebug = true;
			MoveAllStrats();
			StratProc();
			ProcessAllStrats();
			suppressDebug = nar;
		}
		for (i = 0; i < 4; ++i)
			FadeFromBlack(i, 15);
	}
}

void TimeFunction (int what)
{
	static int inc = -1;
	static int SavedBossTime, SavedGameTime;
	static float SavedGunJuice;
	// Ignore on credits
	if (LevelNum == 13)
		return;
	switch (what) {
	case -1: // Reset
		BossTime = GameTime = 0;
		SavedGunJuice = 0;
		inc = -1;
		break;
	case -2: // New frame
		if (MPStartupState == MP_IN_GAME &&
			player[0].Player &&
			player[0].Player->health > 0 &&
			!PauseMode) {
			if (inc == 0)
				GameTime++;
			else if (inc == 1)
				BossTime++;
		}
		break;
		// XXX Taken out at Thon's request Thon.
	case -3: // Store times at a save point, if not on a challenge level)
		// Store gun jewss
		if (LevelNum < 6) {
			SavedGunJuice = player[0].PlayerWeaponEnergy;
		}
//		if (LevelNum < 6) {
//			SavedBossTime = BossTime;
//			SavedGameTime = GameTime;
//		}
		break;
	case -4: // Retore times on recreation
		if (LevelNum < 6) {
			player[0].PlayerWeaponEnergy = SavedGunJuice;
		}
//		if (LevelNum < 6) {
//			BossTime = SavedBossTime;
//			GameTime = SavedGameTime;
//		}
		break;
	case 0: // Start game
//		BossTime = 0;
		inc = 0;
		break;
	case 1: // Start boss
		//BossTime = 0;
		inc = 1;
		break;
	case 2: // Stop timer
		inc = -1;
		break;
	case 3: // restart game
		inc = 0;
		break;
	}
}

#define BOMB_SIZE	96

static Point2 GetBombPos (int i)
{
	Point2 retVal;
	if (NumberOfPlayers == 2) {
		retVal.x = PHYS_SCR_X/2 - BOMB_SIZE/2;
		if (i == 0) {
			retVal.y = BOMB_SIZE-16;
		} else {
			retVal.y = BOMB_SIZE-16+PHYS_SCR_Y/2;
		}
	} else {
		retVal.x = PHYS_SCR_X/4 + ((i & 1) ? PHYS_SCR_X/2 : 0) - BOMB_SIZE/2;
		retVal.y = BOMB_SIZE-16 + ((i & 2) ? PHYS_SCR_Y/2 : 0);
	}
	return retVal;
}

static void SetBombNum(int i)
{
	int j;
	NumBombs = i;
	BombStart = TRUE;
	for (j = 0; j < i; ++j) {
		int k;
		for (;;) {
			int l;
			k = rand() % NumberOfPlayers;
			for (l = 0; l < i; ++l) {
				if (BombArray[l].player == k)
					break;
			}
			if (l == i)
				break;
		}
		BombArray[j].player = k;
		BombArray[j].startPos = BombArray[j].pos = BombArray[j].targetPos = GetBombPos (k);
	}
}

static void DoBombs(void)
{
	Point2 p[4];
	int i,j; 
	float win[4];
	float dist, dist2;
	extern Material fadeMat;

	if (magicWaitCount == -11)
		MPChosen();		

	if (magicWaitCount < -10 && (currentFrame & 1))
		return;

	psSetWindow (-1, -1, -1, -1);
	psGetWindow (win);
	rSetMaterial (&amBomb);
	kmxxGetCurrentPtr (&vertBuf);
	kmxxStartVertexStrip (&vertBuf);

	// Team bomb mode?
	if (NumBombs == 0) {
		for (i = 0; i < NumberOfPlayers; ++i) {
			if (player[i].team != BombTeam)
				continue;
			p[0] = GetBombPos(i);
			p[1].x = p[0].x; p[1].y = p[0].y + BOMB_SIZE;
			p[2].x = p[0].x + BOMB_SIZE; p[2].y = p[0].y;
			p[3].x = p[0].x + BOMB_SIZE; p[3].y = p[0].y + BOMB_SIZE;
			for (j = 0; j < 4; ++j) {
				mPoint2Multiply (p+j, p+j, psGetMatrix());
				p[j].x += win[1];
				p[j].y += win[0];
			}
			
			kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,			p[0].x, p[0].y, 500, 0, 0, 0xffffffff, 0);
			kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,			p[1].x, p[1].y, 500, 0, 1, 0xffffffff, 0);
			kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,			p[2].x, p[2].y, 500, 1, 0, 0xffffffff, 0);
			kmxxSetVertex_3 (KM_VERTEXPARAM_ENDOFSTRIP,		p[3].x, p[3].y, 500, 1, 1, 0xffffffff, 0);
		}
	} else {
		// Normal bomb mode	
		for (i = 0; i < NumBombs; ++i) {
			p[0] = BombArray[i].pos;
			p[1].x = p[0].x; p[1].y = p[0].y + BOMB_SIZE;
			p[2].x = p[0].x + BOMB_SIZE; p[2].y = p[0].y;
			p[3].x = p[0].x + BOMB_SIZE; p[3].y = p[0].y + BOMB_SIZE;
			for (j = 0; j < 4; ++j) {
				mPoint2Multiply (p+j, p+j, psGetMatrix());
				p[j].x += win[1];
				p[j].y += win[0];
			}
			
			kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,			p[0].x, p[0].y, 500, 0, 0, 0xffffffff, 0);
			kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,			p[1].x, p[1].y, 500, 0, 1, 0xffffffff, 0);
			kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,			p[2].x, p[2].y, 500, 1, 0, 0xffffffff, 0);
			kmxxSetVertex_3 (KM_VERTEXPARAM_ENDOFSTRIP,		p[3].x, p[3].y, 500, 1, 1, 0xffffffff, 0);
			
			dist  = sqrt (SQR(BombArray[i].pos.x - BombArray[i].targetPos.x) + SQR(BombArray[i].pos.y - BombArray[i].targetPos.y));
			dist2 = sqrt (SQR(BombArray[i].pos.x - BombArray[i].startPos.x) + SQR(BombArray[i].pos.y - BombArray[i].startPos.y));
			
			dist = (dist + dist2) * 0.5f * 0.002f;
			if (PAL)
				dist = pow(dist, 50.f/60.f);
			
			BombArray[i].pos.x += (BombArray[i].targetPos.x - BombArray[i].pos.x) * dist;
			BombArray[i].pos.y += (BombArray[i].targetPos.y - BombArray[i].pos.y) * dist;
		}
	}
	kmxxReleaseCurrentPtr (&vertBuf);
}

void BombProc (int bomb, int pad)
{
	float dist;
	int oldP;

	dist = sqrt (SQR(BombArray[bomb].pos.x - BombArray[bomb].targetPos.x) + SQR(BombArray[bomb].pos.y - BombArray[bomb].targetPos.y));
	if (dist < 6.5f) {
		oldP = BombArray[bomb].player;
		if (pad & (PADUP|PADDOWN)) {
			if (NumberOfPlayers == 2)
				BombArray[bomb].player = 1 - BombArray[bomb].player;
			else {
				switch (BombArray[bomb].player) {
				case 0:
					BombArray[bomb].player = 2;
					break;
				case 1:
					if (NumberOfPlayers == 4)
						BombArray[bomb].player = 3;
					else
						BombArray[bomb].player = 2;
					break;
				case 2:
					BombArray[bomb].player = 0;
					break;
				case 3:
					BombArray[bomb].player = 1;
					break;
				}
			}
		} else if (pad & (PADLEFT|PADRIGHT)) {
			if (NumberOfPlayers != 2) {
				switch (BombArray[bomb].player) {
				case 0:
					BombArray[bomb].player = 1;
					break;
				case 1:
					BombArray[bomb].player = 0;
					break;
				case 2:
					if (NumberOfPlayers == 4)
						BombArray[bomb].player = 3;
					else
						BombArray[bomb].player = 1;
					break;
				case 3:
					BombArray[bomb].player = 2;
					break;
				}
			}
		}
		if (oldP != BombArray[bomb].player) {
			// Check to see if the new player hasn't already got a bomb;
			int i;
			for (i = 0; i < NumBombs; ++i) {
				if (i == bomb)
					continue;
				if (BombArray[i].player == BombArray[bomb].player) {
					// Same as the player - don't allow it to be thrown
					BombArray[bomb].player = oldP;
					return;
				}
			}
			if (BombStart) {
				magicWaitCount += RandRange (2*60, 4*60);
				BombStart = FALSE;
			}
			BombArray[bomb].startPos = GetBombPos (oldP);
			BombArray[bomb].targetPos = GetBombPos (BombArray[bomb].player);
			SFX_PassBombFE();
		}
	}
}

//static char loadBuffer[512];

// Takes a player number j, and maps it back to a physical port on the machine
int LogToPhysTab[4];
extern int GetLogToPhysFE(int);
void MakeLogToPhysTab(void)
{
	int i;
	if (DemoOn) {
		for (i = 0;i < 4; ++i)
			LogToPhysTab[i] = i;
	} else {
		if (Multiplayer) {
			for (i = 0; i < 4; ++i) {
				LogToPhysTab[i] = GetLogToPhysFE(i);
			}
		} else {
			// already set up
			dAssert (LogToPhysTab[0] != -1, "Failed to set a port");
		}
	}
}
int LogToPhys (int j)
{
	int i;
	i = LogToPhysTab[j];
	dAssert (i >= 0, "Unable to map logical to physical player number");
	return i;
}

Uint32 GetTankColour (int i)
{
	if (i == -1)
		return 0xffffffff;
	if (IsTeamGame()) {
		dAssert (player[i].team != 0, "Erk");
		if (player[i].team == 1)
			return 0xffff0000;
		else
			return 0xff0000ff;
	} else {
		return TankPalette[CurProfile[LogToPhys(i)].multiplayerStats.colour];
	}
}


static setUpPlayerVisibility(int pn, int vis)
{
	STRAT *strat;
	if (player[pn].cameraPos == FIRST_PERSON)
	{
		strat = player[pn].Player;
		if (vis == 0)
		{
			HideObject(strat, 1);
			HideObject(strat, 2);
			HideObject(strat, 3);
			HideObject(strat, 4);
			HideObject(strat, 5);
			HideObject(strat, 6);
			HideObject(strat, 7);
			HideObject(strat, 8);
			HideObject(strat, 9);
			HideObject(strat, 10);
			HideObject(strat, 13);
			if (!Multiplayer)
			{
				HideObject(strat, 14);
				HideObject(strat, 15);
				HideObject(strat, 16);
			}
			HideObject(strat, 17);
			HideObject(strat, 20);
			HideObject(strat, 21);
			HideObject(strat, 22);
			HideObject(strat, 23);
			HideObject(strat, 24);
			switch(player[0].PlayerWeaponType)
			{
			case 0:
				HideObject(strat, 23);
				HideObject(strat, 24);
				break;
			case 1:
				HideObject(strat, 25);
				HideObject(strat, 29);
				HideObject(strat, 30);
				break;
			case 2:
				HideObject(strat, 23);
				HideObject(strat, 31);
				HideObject(strat, 32);

				break;
			case 3:
				HideObject(strat, 26);
				HideObject(strat, 27);
				HideObject(strat, 28);
				break;
			}
		}
		else
		{
			UnhideObject(strat, 1);
			UnhideObject(strat, 2);
			UnhideObject(strat, 3);
			UnhideObject(strat, 4);
			UnhideObject(strat, 5);
			UnhideObject(strat, 6);
			UnhideObject(strat, 7);
			UnhideObject(strat, 8);
			UnhideObject(strat, 9);
			UnhideObject(strat, 10);
			UnhideObject(strat, 13);
			if (!Multiplayer)
			{
				UnhideObject(strat, 14);
				UnhideObject(strat, 15);
				UnhideObject(strat, 16);
			}
			UnhideObject(strat, 17);
			UnhideObject(strat, 20);
			UnhideObject(strat, 21);
			UnhideObject(strat, 22);
			UnhideObject(strat, 23);
			UnhideObject(strat, 24);
			switch(player[0].PlayerWeaponType)
			{
			case 0:
				UnhideObject(strat, 23);
				UnhideObject(strat, 24);
				break;
			case 1:
				UnhideObject(strat, 25);
				UnhideObject(strat, 29);
				UnhideObject(strat, 30);
				break;
			case 2:
				UnhideObject(strat, 23);
				UnhideObject(strat, 31);
				UnhideObject(strat, 32);

				break;
			case 3:
				UnhideObject(strat, 26);
				UnhideObject(strat, 27);
				UnhideObject(strat, 28);
				break;
			}
		}
	}
}

static void FreeDome (void)
{
	/* Dome is allocated from the mission heap and is therefore cleared automatically */
	Dome = NULL;
}


static void DrawShockArray(void)
{
	int i=0;

	for (i=0; i<noShocks; i++)
		DrawShock(shockArray[i*2].x, shockArray[i*2].y, shockArray[i*2].z, shockArray[i*2+1].x, shockArray[i*2+1].y, shockArray[i*2+1].z, 0.5, 2.0, 2.0);
}

static void DrawLineNormal(Point3 *a, Point3 *b, Uint32 colour)
{
	Colour	col;

	col.argb.a = (colour&0xff000000)>>24;
	col.argb.b = (colour&0x00ff0000)>>16;
	col.argb.g = (colour&0x0000ff00)>>8;
	col.argb.r = (colour&0x000000ff);
	mPush(&mCurMatrix);
	mUnit(&mCurMatrix);
	rLine(a, b, col, col);
	mPop(&mCurMatrix);
}

static void DrawWorld (int pn)
{
	int i;
	STRAT *strat;
	

	if (player[pn].Player == NULL)
		return;

	/* gulp I'm sure this is in the wrong plaice */
#if 0
	if (PauseMode)
		CameraSet (player[pn].cam, 0);
	else {
		doCam(pn);
	}
#endif
	doTarget(pn);

	if (
		((player[pn].CameraStrat && (!player[pn].CameraStrat->parent)) &&
		((player[pn].PlayerState & (PS_SIDESTEP | PS_TOWERLIFT | PS_WATERSLIDE | PS_ONTRAIN | PS_NORMAL)))))
		drawHud(pn);

	else
		currentDrawTarget = 0;

	UpdateEnvironmentParms(currentCamera);
	DrawDome(currentCamera);

	rSparks();
	DrawShockArray();

	pbMark (0xff4040, TRUE, PB_SFX);

	mUnit (&mCurMatrix);
   	rDrawScape(m, &currentCamera->pos);
	
	pbMark (0xff0000, TRUE, PB_SCAPE_DRAW);

	setUpPlayerVisibility(pn, 0); //if  player is in first person mode then hide the relevant objects

	DrawAllStrats(PauseMode, pn);

	setUpPlayerVisibility(pn, 1); //unhide player for the other players

	pbMark (0x0000ff, TRUE, PB_STRAT_DRAW);
	

	if (Multiplayer)
	{
		for (i=0; i<NumberOfPlayers; i++)
		{
			DrawSkid(i);	
			if ((i != pn) && (player[i].aimLine == 1))
			{
				DrawLineNormal(&player[i].Player->pos,&player[i].aimedPoint, 0x400000ff);
			}
		}
	}
	else
		DrawSkid(0);

	doControlFade(pn);
	pbMark (0x4040ff, TRUE, PB_SFX);

	sfxDraw();
	if (Multiplayer && player[pn].mpChargeTime)
		rCharging(pn, player[pn].mpChargeTime);

	//singleplayer ? arrowstrat ? arrowstrat not from city boss ? then draw
	if ((!Multiplayer) && ArrowStrat && (!ArrowStrat->var))
		DrawArrowToTarget(ArrowStrat, ArrowStrat->damage, 0xffff0000,1);
 	pbMark (0x00ff00, TRUE, PB_SFX);
	
	// Congratulate if winning
	if (Multiplayer && (pn < NumberOfPlayers) && (Winner == pn || (player[Winner].team && player[Winner].team == player[pn].team))
		&& MPStartupState == MP_IN_GAME)
		rYouWin();

	// Commiserate if dead
	if (Multiplayer && player[pn].Player &&
		Winner < 0 &&
		player[pn].Player->var == 1234.f &&
		player[pn].Player->IVar[0] > 0 &&
		!(GetCurrentGameType()==KNOCKOUT && player[pn].lives == 0)) {
		psSetAlpha (1);
		psSetColour (0xffffff);
		psSetPriority (1e6);
		mUnit32 (psGetMatrix());
		psDrawCentred ("`MPHITSTART", 64.f);
	}
	psSetPriority(0);

#if GINSU
	if ((Multiplayer && 
		((NumberOfPlayers > 2 && pn == 2) ||
		 (NumberOfPlayers == 2 && pn == 2)))
		||!Multiplayer)
		tLogo();
#endif

	if (!Multiplayer) {
		if (player[pn].cameraPos != FIRST_PERSON)
		{
			if (LevelNum == 6 && (CurProfile[0].activeSpecials & SPECIAL_60HZRACE)) {
				// don't do shadows
			} else
				DoTankShads(pn);
			pbMark (0xff0000, TRUE, PB_SHADOW);
		}
	} else {
		// Check to see if we're 'special'
		if ((MPStartupState == MP_IN_GAME || MPStartupState == MP_TRANSITION_TO_GAME) && 
			(player[pn].special || player[pn].cloaked) && Winner==-1) {
			float yBadger = 0;
			Uint32 colour = 0xffffffff;
			Material *m = NULL;
			switch (GetCurrentGameType()) {
			case TAG:
			case SUICIDE_TAG:
				m = &amBomb;
				break;
			case PREDATOR:
				m = &amPred;
				yBadger = -10;
				break;
			case KEEP_THE_FLAG:
				m = &amFlag;
				break;
			case KING_OF_THE_HILL:
				m = &amKing;
				break;
			}
			if (player[pn].cloaked) {
				float amt;
				m = &amPred;
				amt = (player[pn].cloaked > 0 ? RANGE(0, (float)(30 - player[pn].cloaked)*(1.f/30.f), 1.f) : 1.f);
				colour = (colour & 0xffffff) | (((int)(amt*255.f))<<24);
			}
			if (m) {
				float x, y;
				float PICKUP_SIZE = (NumberOfPlayers > 2) ? 48.f : 64.f;
				if (NumberOfPlayers > 2 && (pn & 1)) {
					x = currentCamera->screenMidX - currentCamera->screenX/2 + 8.f;
				} else {
					x = currentCamera->screenMidX + currentCamera->screenX/2 - 8.f - PICKUP_SIZE;
				}
				if (NumberOfPlayers > 2 && (pn & 2)) {
					y = currentCamera->screenMidY - (currentCamera->screenY/2 + yBadger);
				} else {
					y = currentCamera->screenMidY + (currentCamera->screenY/2 - 40.f - PICKUP_SIZE + yBadger);
				}
				y += (((pn<<1) >= NumberOfPlayers) ? 16.f : 0.f);

				rSetMaterial (m);
				kmxxGetCurrentPtr (&vertBuf);
				kmxxStartVertexStrip (&vertBuf);

				kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		x              , y              , W_LAYER_AMMO, 0, 0, colour, 0);
				kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		x              , y + PICKUP_SIZE, W_LAYER_AMMO, 0, 1, colour, 0);
				kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		x + PICKUP_SIZE, y              , W_LAYER_AMMO, 1, 0, colour, 0);
				kmxxSetVertex_3 (KM_VERTEXPARAM_ENDOFSTRIP, x + PICKUP_SIZE, y + PICKUP_SIZE, W_LAYER_AMMO, 1, 1, colour, 0);

				kmxxReleaseCurrentPtr (&vertBuf);
			}
		}

		if (MPStartupState == MP_IN_GAME || MPStartupState == MP_TRANSITION_TO_GAME) {
			float midY, midX;
			// Draw some black lines
			extern Material fadeMat;
			rSetMaterial (&fadeMat);
			kmxxGetCurrentPtr (&vertBuf);
			kmxxStartVertexStrip (&vertBuf);
			
			// Horizontal bar
#define BAR_SIZE 2.f
			midY = PHYS_SCR_Y * 0.5f + rGlobalYAdjust;
			kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL, 0, midY - (BAR_SIZE/2), 4.95f, 0xff000000);
			kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL, 0, midY + (BAR_SIZE/2), 4.95f, 0xff000000);
			kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL, PHYS_SCR_X, midY - (BAR_SIZE/2), 4.95f, 0xff000000);
			kmxxSetVertex_0 (KM_VERTEXPARAM_ENDOFSTRIP, PHYS_SCR_X, midY + (BAR_SIZE/2), 4.95f, 0xff000000);
			
			// Vertical bar
			if (NumberOfPlayers > 2) {
				midX = PHYS_SCR_X * 0.5f;
				kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL, midX - (BAR_SIZE/2), 0, 4.95f, 0xff000000);
				kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL, midX - (BAR_SIZE/2), PHYS_SCR_Y, 4.95f, 0xff000000);
				kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL, midX + (BAR_SIZE/2), 0, 4.95f, 0xff000000);
				kmxxSetVertex_0 (KM_VERTEXPARAM_ENDOFSTRIP, midX + (BAR_SIZE/2), PHYS_SCR_Y, 4.95f, 0xff000000);
			}
			
			// Now draw the marker above the other tanks
			for (i = 0; i < NumberOfPlayers; ++i) {
				Point3 p;
				Point pHom;
				float x, y, rW;
				Uint32 tagColour;
				if (i == pn)
					continue;
				if (!player[i].active || player[i].cloaked < 0 || player[i].cloaked > 30)
					continue;
				if (player[i].Player == NULL || player[i].Player->var == 1234.f)
					continue;
				// Special cases
				switch (GetCurrentGameType()) {
				default:
					if (obscured(&player[pn].CameraStrat->pos, &player[i].Player->pos, &p, pn, NULL))
						continue;
					break;
				case DEATHMATCH:
				case KNOCKOUT:
					break;
				case SUICIDE_TAG:
				case TAG:
					// Pl with bombs see the non-bomb geeza
					if (player[pn].special && !player[i].special)
						break;
					if (obscured(&player[pn].CameraStrat->pos, &player[i].Player->pos, &p, pn, NULL))
						continue;
					break;
				case PREDATOR:
					if (!player[pn].special && obscured(&player[pn].CameraStrat->pos, &player[i].Player->pos, &p, pn, NULL))
						continue;
					break;
				case KEEP_THE_FLAG:
					if (!player[pn].special && player[i].special && !(IsTeamGame() && player[pn].team == player[i].team))
						break;
					if (obscured(&player[pn].CameraStrat->pos, &player[i].Player->pos, &p, pn, NULL))
						continue;
					break;
				}
				mLoadModelToScreen();
				mPointXYZ(&pHom, player[i].Player->pos.x, player[i].Player->pos.y, player[i].Player->pos.z);
				rW = 1.f / pHom.w;
				if (pHom.z < 0)
					continue;
				if (pHom.x > pHom.w ||
					pHom.x < -pHom.w ||
					pHom.y > pHom.w ||
					pHom.y < -pHom.w)
					continue;
				x = pHom.x * rW * pViewSize.x + pViewMidPoint.x;
				y = pHom.y * rW * pViewSize.y + pViewMidPoint.y;
				
				// Move up a bit
				y -= 350*rW/player[pn].cam->screenAngle;
				
#define BOMB_SIZE_2_X 32
#define BOMB_SIZE_2_Y 64
				if (GetCurrentGameType() != PREDATOR &&
					player[i].special &&
					!(GetCurrentGameType() == KING_OF_THE_HILL && player[i].timer == 0)) {
					Uint32 col = GetTankColour (i);
					rSetMaterial (&bombMat);
					kmxxStartVertexStrip(&vertBuf);
					kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		x - BOMB_SIZE_2_X/2, y - BOMB_SIZE_2_Y, 5.f, 0, 0, col, 0);
					kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		x - BOMB_SIZE_2_X/2, y     , 5.f, 0, 1, col, 0);
					kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		x + BOMB_SIZE_2_X/2, y - BOMB_SIZE_2_Y, 5.f, 1, 0, col, 0);
					kmxxSetVertex_3 (KM_VERTEXPARAM_ENDOFSTRIP, x + BOMB_SIZE_2_X/2, y     , 5.f, 1, 1, col, 0);
				} else {
					
					tagColour = GetTankColour (i);
					if (player[i].cloaked) {
						int alpha = (int) ((255.f/30.f) * (30.f - player[i].cloaked));
						tagColour = tagColour & 0xffffff | (alpha << 24);
					}
					
#define TAG_H1 6.f
#define TAG_H2 (TAG_H1 + 6.f)
#define TAG_W  (10.f/2)
					kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL, x + TAG_W, y - TAG_H2, 5.f, tagColour);
					kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL, x - TAG_W, y - TAG_H2, 5.f, tagColour);
					kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL, x + TAG_W, y - TAG_H1, 5.f, tagColour);
					kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL, x - TAG_W, y - TAG_H1, 5.f, tagColour);
					kmxxSetVertex_0 (KM_VERTEXPARAM_ENDOFSTRIP, x    , y         , 5.f, tagColour);
				}
			}
			
			kmxxReleaseCurrentPtr (&vertBuf);
		}
	} // End of multiplayer IF
	// Do the 'GO' if necessary
	if ((Multiplayer || IsChallengeLevel()) && (MPStartupState == MP_IN_GAME && MPCountIn < 5*FRAMES_PER_SECOND)) {
		float alpha, scale;
		mUnit32 (psGetMatrix());
		mPostTranslate32 (psGetMatrix(), -currentCamera->screenX/2, 0);
		scale = 1.5f;
		mPostScale32 (psGetMatrix(), scale, scale);
		mPostTranslate32 (psGetMatrix(), currentCamera->screenX/2, 0);
		psSetColour (0xffffff);
		alpha = 1.f - ((float)MPCountIn - 4.f*FRAMES_PER_SECOND)/(float)FRAMES_PER_SECOND;
		psSetAlpha (RANGE(0, alpha, 1));
		psSetPriority (W_LAYER_AMMO - 1000.f);
		psDrawTitle ("`GO", 32.f/scale, 0, 0xff000000);
	}
}

static void RunPreGameMadness(void)
{
	float moveAmt;
	extern Material fadeMat;

	MoveAllStrats();
	StratProc();
	pbMark (0xff0000, FALSE, PB_STRAT);
	ProcessAllStrats();
	pbMark (0x00ff00, FALSE, PB_COLLISION);
	doCam(0);
	DrawWorld(0);
	psSetAlpha (1);
	psSetPriority (2);
	psSetColour (0xcfcfff);
	
	moveAmt = (TargetRot - RotAmt) * (PAL ? 0.287 : 0.224f);
	moveAmt = RANGE (-0.08f, moveAmt, 0.08f);
	RotAmt += moveAmt;

	moveAmt = (TargetRot - RotAmt) * (PAL ? 0.287 : 0.224f);
	moveAmt = RANGE (-0.08f, moveAmt, 0.08f);
	RotAmt += moveAmt;

	mUnit32 (psGetMatrix());
//	psGetMatrix()->m[2][1] = -RotAmt * ((float)PHYS_SCR_Y/STARTROT);
	psSetAlpha (1.f - RotAmt * (1.f/STARTROT));
	if (InputMode == NORMALINPUT)
		ReseedRandom();
	rSetMaterial (&fadeMat);
	kmxxGetCurrentPtr (&vertBuf);
	kmxxStartVertexStrip (&vertBuf);
	kmxxSetVertex_0 (0xe0000000, 0, 0, 5, 0x80000000);
	kmxxSetVertex_0 (0xe0000000, 0, PHYS_SCR_Y, 5, 0x80000000);
	kmxxSetVertex_0 (0xe0000000, PHYS_SCR_X, 0, 5, 0x80000000);
	kmxxSetVertex_0 (0xf0000000, PHYS_SCR_X, PHYS_SCR_Y, 5, 0x80000000);
	kmxxReleaseCurrentPtr (&vertBuf);
}

static const char *TeamHandler(int nPlayer)
{
	return (LangLookup ((player[nPlayer].team==1) ? "RED" : "BLUE"));
}

static char *PredHandler(int nPlayer)
{
	if (SpinSpeed < 0.05f && (currentFrame & 1))
		return "";
	if (player[nPlayer].special) {
		int j;
		Point2 p[4];
		float win[4], old[4];
		psGetWindow (old);
		psSetWindow (-1, -1, -1, -1);
		psGetWindow (win);

		p[0] = GetBombPos(nPlayer);
		p[1].x = p[0].x; p[1].y = p[0].y + BOMB_SIZE;
		p[2].x = p[0].x + BOMB_SIZE; p[2].y = p[0].y;
		p[3].x = p[0].x + BOMB_SIZE; p[3].y = p[0].y + BOMB_SIZE;
		for (j = 0; j < 4; ++j) {
			mPoint2Multiply (p+j, p+j, psGetMatrix());
			p[j].x += win[1];
			p[j].y += win[0];
		}
		rSetMaterial (&amPred);
		kmxxGetCurrentPtr (&vertBuf);
		kmxxStartVertexStrip (&vertBuf);
		kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,			p[0].x, p[0].y, 500, 0, 0, 0xffffffff, 0);
		kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,			p[1].x, p[1].y, 500, 0, 1, 0xffffffff, 0);
		kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,			p[2].x, p[2].y, 500, 1, 0, 0xffffffff, 0);
		kmxxSetVertex_3 (KM_VERTEXPARAM_ENDOFSTRIP,		p[3].x, p[3].y, 500, 1, 1, 0xffffffff, 0);
		kmxxReleaseCurrentPtr (&vertBuf);

		psSetWindow (old[0] - 32, old[1] - 8, old[2], old[3]);
	}
	return "";
}

static char *TeamPredHandler(int nPlayer)
{
	int i;
	if (SpinSpeed < 0.05f && (currentFrame & 1))
		return "";
	for (i = 0; i < NumberOfPlayers; ++i) {
		int j;
		Point2 p[4];
		float win[4], old[4];

		if (!(player[i].team == player[nPlayer].team && player[i].special))
			continue;

		psGetWindow (old);
		psSetWindow (-1, -1, -1, -1);
		psGetWindow (win);

		p[0] = GetBombPos(nPlayer);
		p[1].x = p[0].x; p[1].y = p[0].y + BOMB_SIZE;
		p[2].x = p[0].x + BOMB_SIZE; p[2].y = p[0].y;
		p[3].x = p[0].x + BOMB_SIZE; p[3].y = p[0].y + BOMB_SIZE;
		for (j = 0; j < 4; ++j) {
			mPoint2Multiply (p+j, p+j, psGetMatrix());
			p[j].x += win[1];
			p[j].y += win[0];
		}
		rSetMaterial (&amPred);
		kmxxGetCurrentPtr (&vertBuf);
		kmxxStartVertexStrip (&vertBuf);
		kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,			p[0].x, p[0].y, 500, 0, 0, 0xffffffff, 0);
		kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,			p[1].x, p[1].y, 500, 0, 1, 0xffffffff, 0);
		kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,			p[2].x, p[2].y, 500, 1, 0, 0xffffffff, 0);
		kmxxSetVertex_3 (KM_VERTEXPARAM_ENDOFSTRIP,		p[3].x, p[3].y, 500, 1, 1, 0xffffffff, 0);
		kmxxReleaseCurrentPtr (&vertBuf);

		psSetWindow (old[0] - 32, old[1] - 8, old[2], old[3]);

		break;
	}
	return "";
}

static char *NullHandler(int ignored) { return ""; }

static void DrawMadness(const char *CYT, const char *(*handler)(int))
{
	Uint32 col;
	if (handler == NULL)
		handler = NullHandler;
	if (CYT[0] == '`')
		CYT = LangLookup(CYT + 1);
	if (NumberOfPlayers == 2) {
		col = TankPalette[CurProfile[LogToPhys(0)].multiplayerStats.colour];
		psDrawTitle (psFormat (CYT, CurProfile[LogToPhys(0)].name), 32, col & 0xffffff, col);
		psDrawCentred (handler(0), 32+40);
		col = TankPalette[CurProfile[LogToPhys(1)].multiplayerStats.colour];
		psDrawTitle (psFormat (CYT, CurProfile[LogToPhys(1)].name), 32+240, col & 0xffffff, col);
		psDrawCentred (handler(1), 32+240+40);
	} else {
		psSetWindow (0, 0, PHYS_SCR_X/2, PHYS_SCR_Y);
		col = TankPalette[CurProfile[LogToPhys(0)].multiplayerStats.colour];
		psDrawTitle (psFormat (CYT, CurProfile[LogToPhys(0)].name), 32, col & 0xffffff, col);
		psDrawCentred (handler(0), 32+40);
		col = TankPalette[CurProfile[LogToPhys(2)].multiplayerStats.colour];
		psDrawTitle (psFormat (CYT, CurProfile[LogToPhys(2)].name), 32+240, col & 0xffffff, col);
		psDrawCentred (handler(2), 32+240+40);
		
		psSetWindow (0, PHYS_SCR_X/2, PHYS_SCR_X, PHYS_SCR_Y);
			col = TankPalette[CurProfile[LogToPhys(1)].multiplayerStats.colour];
		psDrawTitle (psFormat (CYT, CurProfile[LogToPhys(1)].name), 32, col & 0xffffff, col);
		psDrawCentred (handler(1), 32+40);
		if (NumberOfPlayers==4) {
			col = TankPalette[CurProfile[LogToPhys(3)].multiplayerStats.colour];
			psDrawTitle (psFormat (CYT, CurProfile[LogToPhys(3)].name), 32+240, col & 0xffffff, col);
			psDrawCentred (handler(3), 32+240+40);
		}
	}
}

void RecurseBodgeColours (Object *o, Uint32 colour)
{
	int i;
	if (o->model) {
		for (i = 0; i < o->model->nMats; ++i) {
			if (o->model->material[i].flags & MF_DECAL) {
				o->model->material[i].diffuse.colour = colour;
			}
		}
	}
	for (i = 0; i < o->no_child; ++i)
		RecurseBodgeColours (o->child + i, colour);
}

void levelInitialisationCode(void)
{
	extern float CurrentStratIntensity;
	extern float TargetStratIntensity;
	extern float CurrentRedDogIntensity;
	extern float TargetRedDogIntensity;
	extern float DomeFadeAmount;
	extern float TargetDomeFadeAmount;

	int i;

  
	if (Multiplayer)
	{
		for (i=0; i<4; i++)
		{
			PlayerCameraState[i] = THIRD_PERSON_CLOSE;
			player[i].rdhm = NULL;
			player[i].ThonGravity = 1.f;
		}
	}
	else
	{
		player[0].ThonGravity = 1.f;
		player[0].rdhm = NULL;
		if (PlayerLives == 2)
			PlayerCameraState[0] = THIRD_PERSON_CLOSE;
	}

#ifndef _RELEASE
//	TrapDiv0();
#endif
	TargetDomeFadeAmount = DomeFadeAmount = 1.f;

	if (player[currentPlayer].cam)
		CameraSet (player[currentPlayer].cam, 0);

	NoMainGun = 0;
	noHits = 0;
	for (i=0; i<16; i++)
	{
		hitStratArray[i] = NULL;
		hitObjectArray[i] = NULL;
		hitCollWithArray[i] = NULL;
	}

	rFXinit();
	if (!Multiplayer)
		lInit(); // paranoia check
	// lInit moved elsewhere - as reinitialisation should leave lights going for multiplayer only!

	TargetStratIntensity = CurrentStratIntensity = RANGE(0, GlobalAmbientLightIntensityStrat, 0.8);
	TargetRedDogIntensity = CurrentRedDogIntensity = RANGE(0, GlobalAmbientLightIntensityStrat, 0.8);

	SetWhiteOut(0,0,0);
	DamLastFrame = -1;

	sunDir.x = 0.5f;
	sunDir.y = -0.5f;
	sunDir.z = 1.f;
	sunDir.w = 0.f;

	if (Multiplayer)
	{
		for (i=0; i<4; i++)
			resetCamera(i);
		
		// Set up the cameras
		for (i = 0; i < 4; ++i) { // should be four to allow extra cams
			SetPlayer(&DummPlayer);
			player[i].CameraStrat = &DummCam;
			player[i].reward = 0;
		}
		StratProc();
		StratProc();
		ChangeCam(1,0);
		ResetRandom();
	}
	else
	{
		// Let the C++ front end stick its oar in before anything else
		extern void SetupSpecials();
		SetupSpecials();

		player[0].Player->health = 100.0f;
		if (player[0].reward & UPGRADE_ARMOUR_1)
			player[0].Player->health = UPGRADE_ARMOUR_1_HEALTH;
		if (player[0].reward & UPGRADE_ARMOUR_2)
			player[0].Player->health = UPGRADE_ARMOUR_2_HEALTH;
		player[0].maxHealth = player[0].Player->health;
		resetCamera(currentPlayer);
	}
	
	BeginningOfTheEnd = false;
	BeingBriefed = false;
	{
		extern int lensFlareOn;
		lensFlareOn = 1;
	}
}


void FadeToBlack(int pn, int n)
{
	
	fadeN[pn] = 0;
	fadeLength[pn] = n;
	fadeMode[pn] = FADE_TO_BLACK;		
}

extern int FindRandMap(int gameType);

static bool OLDPAL;

void gameHandler (GameState state, GameStateReason reason)
{
   	char myString[64], *levelName;
	int i, team, readyToStart = 0;
	float y, moveAmt;
	static Colour fadecol;

	switch (reason) {

	case GSR_INITIALISE:
#ifdef MATTG_LEV_HACK
		for (i = 0; i < NumLevels; ++i)
//			if (i != 13)
				Levels[i] = MATTG_LEV_HACK;
#endif

		// Sort out pal madness
		if (PAL && Multiplayer) {
			kmAdjustDisplayCenter (adjustedX, RANGE(-64, adjustedY + 0, 64));
		}

		PauseMenuNeedsInitialising = TRUE;

		// We're active innit
		InactiveFrames = 0;
		// Increment the tournament game, only if not on the last (ie replay)
		if (Multiplayer && (CurrentTournamentGame < mpSettings.game.tournament.nGames-1
			|| mpSettings.playMode != TOURNAMENT))
			CurrentTournamentGame++;
	  //	globalStratID = 0;
	  	OLDPAL = PAL;
		DisableTexDMAs = TRUE;
		MakeLogToPhysTab();
#if 0
		buggeryLogPtr = buggeryLog;
#endif

		ExitedThroughReset = 0;
	  //	InitMemory();
		// Shutdown the renderer ready to start up in the appropriate mode
		rShutdownRenderer();
		// In multiplayer, swap to less textures, more polys
		if (Multiplayer) {
			rInitRenderer (Multiplayer, 3*1024*1024);
		} else {
			// Right - non mp work out how much RAM we need
			int size = 0;
			Sint32 fsize;
			s = sOpen ("SYSTEM.TXP");
			gdFsGetFileSize (s->gdfs, &fsize);
			sClose(s);
			size += fsize;

			sprintf(myString, "%s.TXP", Levels[LevelNum]);
			s = sOpen (myString);
			sIgnore (s->bytesLeft - 4, s);
			sRead (&fsize, 4, s);
			sClose(s);
			size += fsize;

			rInitRenderer (Multiplayer, size * 1.01f);
		}

		//CHEAP BUT ENSURES DEMOS ARE SAME

	   	if (DemoOn)
	   		PAL = FALSE;
			
		for (i = 0; i < NumberOfPlayers; ++i) {
			fadeN[i] = 0;
			fadeLength[i] = 0;
			fadeMode[i] = 0;
		}

		frameCount = currentFrame = 0;
		player[currentPlayer].aimedPoint.x = player[currentPlayer].aimedPoint.y = player[currentPlayer].aimedPoint.z = 0;
	  	PauseMode = 0;
		
		if (!DemoOn)
			CapturePort(0);
		/*rResetFragments();
		clearTargetArray(0);
		clearTargetArray(1);
		clearTargetArray(2);
		clearTargetArray(3);
		InitArrowArray();*/

		TimeFunction (-1);
		DamageTaken = 0;
		/*LevTextureNum = GetTextureNum();*/

	 //	rRegionCentre.x = 0.0f;
	 //	rRegionCentre.y = 0.0f;
	 //	rRegionCentre.z = 0.0f;
	 //	rRegionSqrRadius = 100000000.0f;		

		BeginLoading();
		s = sOpen ("SYSTEM.LVL");
		targetTLSpriteHolder = PNodeLoad (NULL, s, (Allocator *)SHeapAlloc, MissionHeap);
		targetTRSpriteHolder = PNodeLoad (NULL, s, (Allocator *)SHeapAlloc, MissionHeap);
		targetBRSpriteHolder = PNodeLoad (NULL, s, (Allocator *)SHeapAlloc, MissionHeap);
		targetBLSpriteHolder = PNodeLoad (NULL, s, (Allocator *)SHeapAlloc, MissionHeap);
		targetSpriteHolder = PNodeLoad (NULL, s, (Allocator *)SHeapAlloc, MissionHeap);

		sClose (s);

		MapWasRandom = GameWasRandom = FALSE;

		CanPause = 0;
		TimerTime = 0;
		if (!Multiplayer) {
			BeingBriefed = false; // just in case
			levelName = Levels[LevelNum];
			// Challenge levels have a countdown
			if (LevelNum == LEV_SHOOTING_RANGE)
				MPCountIn = -((7.6)*FRAMES_PER_SECOND);
			else
				MPCountIn = -30;
			MPStartupState = IsChallengeLevel() ? MP_TRANSITION_TO_GAME : MP_IN_GAME;
		} else {
			if (DemoOn || DEMOREC) {
				int i;
				for (i = 0; i < 4; ++i) {
					extern int CurHandicap[4];
					CurHandicap[i] = 100;
					CurProfile[i].multiplayerStats.colour = i;
				}
				if (DemoOn)
					levelName = MPWadName[LevelNum];
				else
					levelName = MPWadName[mpSettings.game.skirmish.map];
#ifdef MATTG_LEV_HACK
				dPrintf ("*** Would have loaded map '%s' ***", levelName);
				levelName = MATTG_LEV_HACK;
#endif
				MPStartupState = MP_IN_GAME;
				mpSettings.playMode = SKIRMISH;
				mpSettings.game.skirmish.map = LevelNum;
				mpSettings.game.skirmish.gameType = DEATHMATCH;
				mpSettings.game.skirmish.parms.dmParms.fragLimit = 10;
				ResetRandom();
			} else {
				// Have we played enough MP games in a row?
				if (++NumPlayedMPGames > NUFF_MP_GAMES) {
					for (i = 0; i < 4; ++i) {
						CurProfile[i].unlockedSpecials |= SPECIAL_EXTRA_LEVEL(6);
					}
				}
				ReseedRandom();
				switch (mpSettings.playMode) {
				case SKIRMISH:
					// Is this a random map?
					if (mpSettings.game.skirmish.map == numMPMaps) {
						// Choose a random map which fits in with the game type
						MapWasRandom = TRUE;
						mpSettings.game.skirmish.map = FindRandMap(mpSettings.game.skirmish.gameType);
					}
					levelName = MPWadName[mpSettings.game.skirmish.map];
					break;
				case TEAM:
					if (mpSettings.game.team.map == numMPMaps) {
						// Choose a random map which fits in with the game type
						MapWasRandom = TRUE;
						mpSettings.game.team.map = FindRandMap(mpSettings.game.team.gameType);
					}
					levelName = MPWadName[mpSettings.game.team.map];
					break;
				case TOURNAMENT:
					// Is this a random type of game?
					if (mpSettings.game.tournament.game[CurrentTournamentGame].gameType == RANDOM) {
						bool ok;
						ResetGameParms(&mpSettings.game.tournament.game[CurrentTournamentGame].parms);

						// Ensure we get an appropriate game type
						ok = false;
						do {
							mpSettings.game.tournament.game[CurrentTournamentGame].gameType = rand() % NUM_MPLAYER_TYPES;
							// Non-random map?
							if (mpSettings.game.tournament.game[CurrentTournamentGame].map < numMPMaps) {
								ok = (MPLevOK[mpSettings.game.tournament.game[CurrentTournamentGame].map] 
									& (1<<mpSettings.game.tournament.game[CurrentTournamentGame].gameType));
							} else
								ok = true;
						} while (!ok);
						GameWasRandom = TRUE;
					}
					// Is this a random map?
					if (mpSettings.game.tournament.game[CurrentTournamentGame].map == numMPMaps) {
						// Choose a random map which fits in with the game type
						MapWasRandom = TRUE;
						mpSettings.game.tournament.game[CurrentTournamentGame].map = FindRandMap (mpSettings.game.tournament.game[CurrentTournamentGame].gameType);
					}
					levelName = MPWadName[mpSettings.game.tournament.game[CurrentTournamentGame].map];
					break;
				}
				if (IsTeamGame()) {
					for (i = 0; i < 4; ++i) {
						player[i].team = 1+(i&1);
					}
				}
				MPStartupState = MP_CAMERA_TRACK;
				RotAmt = STARTROT;
				TargetRot = 0;
				LevelNum = 0;
#ifdef MATTG_LEV_HACK
				dPrintf ("*** Would have loaded map '%s' ***", levelName);
				levelName = MATTG_LEV_HACK;
#endif
			}
		}

		// Dodgy hack to allow gameflow testing
		sprintf(myString, "%s.TXP", levelName);
		s = sOpen (myString);
		LoadSetWadAmount(Multiplayer ? 40.f : 50.f);
		sSetBuf (s, GameHeap->end - 1024*1024, 1024*1024);
		texInit();
		texLoad(s);
		sClose (s);

		sprintf(myString, "%s.LVL", levelName);
		s = sOpen (myString);
		LoadSetWadAmount(Multiplayer ? 40.f : 50.f);
		StratInit();

		if (LevelNum != 13) // Don't zero the score on credits
			score = 0;
		MissionBriefingDone = FALSE;
		//player[currentPlayer].adrelanin = player[currentPlayer].adrelaninGlow = 0;

		if (DEMOREC)		
			InputMode = DEMOWRITE;
		else
		{
			if (DemoOn)
				InputMode = DEMOREAD;
			else
				InputMode = NORMALINPUT;
		}
		DEMOREC = DemoOn =0;
	
		InitInput();
		MapInit(s);
		m = MapLoad (s);
		sClose (s);

		// In multiplayer, load in the tanks
		if (Multiplayer) {
			int tank, lod;
			s = sOpen ("MPTANKS.TXP");
			LoadSetWadAmount(10.f);
			sSetBuf (s, GameHeap->end - 1024*1024, 1024*1024);
			texLoad (s);
			sClose (s);
			s = sOpen ("MPTANKS.LVL");
			LoadSetWadAmount(10.f);
			for (tank = 0; tank < MAX_TANK; ++tank) {
				for (lod = 0; lod < 3; ++lod) {
					MultiTank[tank][lod] = PNodeLoad (NULL, s, (Allocator *)SHeapAlloc, MissionHeap);
					// Adjust colours accordingly
					if (tank < NumberOfPlayers) {
						Uint32 colour = GetTankColour(tank);//TankPalette[CurProfile[LogToPhys(tank)].multiplayerStats.colour];						
						RecurseBodgeColours (MultiTank[tank][lod]->obj, colour);
					}
				}
			}
			MPArrow = PNodeLoad (NULL, s, (Allocator *)SHeapAlloc, MissionHeap);
			sClose (s);
			MPInit();
			choosed= false;

			// Do some fixing up
			for (tank = 0; tank < asize (FixUpList); ++tank) {
				FixUpList[tank].mat->surf.GBIX = FixUpList[tank].GBIX;
				rFixUpMaterial (FixUpList[tank].mat, &sfxContext);
			}
			for (tank = 0; tank < asize (FixUpListObj); ++tank) {
				FixUpListObj[tank].mat->surf.GBIX = FixUpListObj[tank].GBIX;
				rFixUpMaterial (FixUpListObj[tank].mat, &objContext);
			}

		}


		EndLoading();

		SoundFade(1.f,0);

		if (Multiplayer) {
			// Select a Multiplayer tune randomly			
			if (InputMode == NORMALINPUT) 
				ReseedRandom();
			SoundPlayTrack(15 + (rand() % 4));
		} else {
			SoundPlayTrack(1);	// Mission Briefing
		}

		Savmap = m;

		rInitOverlays();

		player[currentPlayer].cam = CameraCreate();
		player[currentPlayer].cam->type = CAMERA_LOOKAROUND;
		player[currentPlayer].cam->u.lookDir.rotX = player[currentPlayer].cam->u.lookDir.rotY = player[currentPlayer].cam->u.lookDir.rotZ = 0;
		
		setLevelCam(player[currentPlayer].cam);
		if (Multiplayer)
		{
			for (i=1; i<4; i++)
			{
				player[i].cam = CameraCreate();
				player[i].cam->type = CAMERA_LOOKAROUND;
				player[i].cam->u.lookDir.rotX = player[i].cam->u.lookDir.rotY = player[i].cam->u.lookDir.rotZ = 0;
				setLevelCam(player[i].cam);
			}
		}
		
		rSetBackgroundColour (0);
		
		fadecol.argb.a=fadecol.argb.r=fadecol.argb.b=fadecol.argb.g = 0;
		lInit();
		levelInitialisationCode();
		TimerShow = 0;
		
		Bingled = false;

		if (Multiplayer && (InputMode != NORMALINPUT)) {
			// Tricksey innit
			MPStartupState = MP_FADE_OUT_TO_BLACK;
			fadeN[0] = 100;
			FireHerUp();
		}
		// Huzzah - blasphemy mungerissimo para profillisimus
		if (InputMode == NORMALINPUT)
			LockCurrentProfile();

		psSetPriority(0);
		ResetTexSys();
		DisableTexDMAs = FALSE;
		if (LevelNum == 6 && (CurProfile[0].activeSpecials & SPECIAL_60HZRACE)) {
			extern float TargetGlobalFogFar, TargetGlobalFogNear;
			extern float GlobalFogFar, GlobalFogNear;
			extern float TargetGlobalFogRed, TargetGlobalFogBlue, TargetGlobalFogGreen;
			extern float GlobalFogRed, GlobalFogBlue, GlobalFogGreen;
			kmSetWaitVsyncCount(0);
			TargetGlobalFogFar = GlobalFogFar = 250.f;
			TargetGlobalFogNear = GlobalFogNear = 50.f;
			TargetGlobalFogRed = GlobalFogRed = 104.f/256.f;
			TargetGlobalFogBlue = GlobalFogBlue = 120.f/256.f;
			TargetGlobalFogGreen = GlobalFogGreen = 144.f/256.f;
		}
		break;

	case GSR_RUN:
#if !GINSU
		if (PadReset && !ChangingState()) {
			ExitedThroughReset = -1;
			FadeToNewState (FADE_TO_BLACK_FAST, GS_TITLEPAGE);
		} else {		// Are we playing a demo back?
			if (PadReset && InputMode == DEMOREAD) {
				sbExitSystem();
				while (1);
			}
		}

#endif
		frameCount++;
		TimeFunction(-2); // Tick the timers
		switch (MPStartupState) {
		default:
		case MP_IN_GAME:
			// To get the missionbriefing to work
			SavedPadPress = PadPress[0];
			SavedPadPush = PadPush[0];
			SavedJoyY = joyState[0].y;

			// In multiplayer tag and knockout, prevent dead player from doing anything
			if (Multiplayer &&
				(GetCurrentGameType() == TAG || GetCurrentGameType() == KNOCKOUT)) {
				for (i = 0; i < NumberOfPlayers; ++i) {
					if (player[i].lives == 0)
						PadPress[i] = PadPush[i] = 0, joyState[i].x = joyState[i].y = 0.f;
				}
			}

			// Check for VMS warez
			if (bupDriveChange) {
				BupRescan();
				bupDriveChange = FALSE;
			} 
			// Force pause mode when pad pulled
			if (!PadOK) {
				PauseMode = 1;
				PausedBy = -1;
			}
#ifdef MEMORY_CHECK
			tPrintf (0, 3, "%d Obj", ObjHeap->nAlloced);
#endif
			// Force pause mode when VMS yanked too
			{
				int drive;
				for (drive = 0; drive < 8; ++drive) {
					if (BupDrivePanic[drive]) {
						PauseMode = 1;
						PausedBy = -1;
					}
				}
			}
			if ((player[0].PlayerState == PS_INDROPSHIP) || 
				(BeingBriefed) ||
				(!Multiplayer && player[0].CameraStrat && player[0].CameraStrat->parent)
				)// Put other states where you can't pause here
				CanPause = 0;
			else
				CanPause = 1;

			if (!PauseMode && !ChangingState() && CanPause) {
				int i;
				for (i = 0; i < NumberOfPlayers; ++i) {
					if ((PadPush[i] & PADSTART) &&
						player[i].Player &&
						player[i].Player->health) {
						if (Multiplayer)
							PausedBy = i;
						PauseMode = 1;
					}
				}
			}
			
			if (player[0].Player) {
				if (DamLastFrame == -1)
					DamLastFrame = player[0].Player->health;
				else {
					DamageTaken += MAX(0, player[0].Player->health - DamLastFrame);
					DamLastFrame = player[0].Player->health;
				}
			}

			// Update timer
			if (!Multiplayer) {
				if (!PauseMode &&
					(player[0].Player && player[0].Player->health > 0)) {
					if (TimerDirection < 0) {
						if (TimerTime != 0)
							TimerTime += TimerDirection;
					} else {
						TimerTime+= TimerDirection;
					}
				}
			}
			
			// Draw the timer
			if (TimerShow && !(currentCamera->flags & CAMERA_BLACKBARS)) {
				char buf[32];
				float wid;
				static int lastsec;
				int sec, milli, min;
				float mins = (((float)TimerTime * (1.f / FRAMES_PER_SECOND)) / 60.f);

				// If we're dead, don't show it
				if (player[0].Player && player[0].Player->var == 12345) {
				} else {
					const char *time;
					min = (int)mins;
					sec = (int)((mins - (float)min) * 60);
					milli = (int)((mins - (float)(min + (1.f / 60.f) * sec)) * 6000.f);
					milli = MIN (milli, 99);
					time = psFormatTimeMilli (min, sec, milli);
					wid = 24 * psLastLength;
					if (!Multiplayer && !min && (sec < 11) && (lastsec != sec)) {
						if (sec > 5) {
							sfxPlay(21, 0.9f, 0.f);
						} else {
							sfxPlay(22, 0.9f, 0.f);
						}
					}
					lastsec = sec;
					tScore ((PHYS_SCR_X - wid) * 0.5f, 32, time, TimeColour(TimerTime));
				}
			}
			
			// Game update
			if (PauseMode) {
				extern Uint32 _BSG_END;
				KMUINT32 texFree, texBlock;
				// The music will have been paused; request drive status here
				gdFsReqDrvStat();
				// New:
				CameraSet (player[0].cam, 0);
				// Paused update
				StratProcPaused();
				// Game drawing
				DrawWorld (0);
				if (Multiplayer)
				{
					int i,j;
					if (NumberOfPlayers == 3)
						j = 4;
					else j = NumberOfPlayers;
					for (i = 1; i < j; ++i)
					{
						CameraSet (player[i].cam, 0);
						DrawWorld (i);
					}
				}
				if (PauseMenuNeedsInitialising) {
					InitPauseMenu();
					PauseMenuNeedsInitialising = FALSE;
				}
				
				kmSetUserClipping (&vertBuf, KM_OPAQUE_POLYGON, 0, 0, 255, 255);
				kmSetUserClipping (&vertBuf, KM_OPAQUE_MODIFIER, 0, 0, 255, 255);
				kmSetUserClipping (&vertBuf, KM_TRANS_POLYGON, 0, 0, 255, 255);
				psSetWindow (0,0, PHYS_SCR_X, PHYS_SCR_Y);
				RunPauseMenu();

#if defined(_DEBUG) || defined(_PROFILE)
				if (PadPress[0] & PADY) {
					Uint32 whole, biggest;
					tPrintf (1, 1, "Code size    %.2fMb", (_BSG_END - 0x8c000000)* (1.f / (1024.f*1024.f)));
					tPrintf (1, 2, "Game free    %.2fMb", (GameHeap->end - GameHeap->curPtr) * (1.f / (1024.f*1024.f)));
					CHeapGetStats (ObjHeap, (int *)&texFree, (int *)&texBlock);
					tPrintf (1, 3, "Object heap  %.2fMb", texFree * (1.f / (1024.f*1024.f)));
					tPrintf (1, 4, "   lgst blck %.2fMb", texBlock * (1.f / (1024.f*1024.f)));
					kmGetFreeTextureMem (&texFree, &texBlock);
					tPrintf (1, 5, "TexRAM free  %.2fMb", texFree * (1.f / (1024.f*1024.f)));
					tPrintf (1, 6, "   lgst blck %.2fMb", texBlock * (1.f / (1024.f*1024.f)));
					syMallocStat (&whole, &biggest);
#ifndef ARTIST
					tPrintf (1, 7, "syFree       %.2fMb", whole * (1.f / (1024.f*1024.f)));
					tPrintf (1, 8, "syBiggest    %.2fMb", biggest * (1.f / (1024.f*1024.f)));
#else		
					if (player[0].cam->u.lookDir.rotX < 0.0f)
						tPrintf (1, 7, "CamRotX %.2f", 90.0f - (57.3f * (player[0].cam->u.lookDir.rotX + 3.14159f)));
					else
						tPrintf (1, 7, "CamRotX %.2f", 90.0f + (57.3f * (1.14159f - player[0].cam->u.lookDir.rotX)));
					
					tPrintf (1, 8, "CamRotZ %.2f", - (57.3f * (player[0].cam->u.lookDir.rotY - 3.14159f)));
					tPrintf (1, 9, "CamPosX %.2f", player[0].CameraStrat->pos.x);
					tPrintf (1, 10, "CamPosY %.2f", player[0].CameraStrat->pos.y);
					tPrintf (1, 11, "CamPosZ %.2f", player[0].CameraStrat->pos.z);
#endif
				}
#endif
			} else {
				MPCountIn++;
				PauseMenuNeedsInitialising = TRUE;

				// Update the landscape lights
				lUpdateScapeLights();
				// Camera processed before strat action now:
				doCam(0);
				pbMark (0xffffff, FALSE, PB_CAM_AND_LIGHT);

				//MATTP TO ENSURE THAT STRATS WON'T TRY TO ACCESS A NULL PLAYER 
				//WHEN GAME IS OVER, ALTERNATIVELY, WHERE GAMEOVERFLAG IS SET
				//PLAYER[CURRENTPLAYER]->PLAYER COULD BE SET TO &DUMMSTRAT
				//BUT I THINK THIS IS SAFER
				if (!GameOverFlag)
				{
//						int temp, temp2;
//						extern int CollTime, NumCollisions;
					MoveAllStrats();
					pbMark (0x00ffff, FALSE, PB_STRAT);
					StratProc();
					pbMark (0xff0000, FALSE, PB_STRAT);
//						temp = syTmrGetCount();
//						NumCollisions = 0;
					ProcessAllStrats();
//						temp2 = syTmrGetCount();
//						CollTime = syTmrCountToMicro(temp2 - temp);
					pbMark (0x00ff00, FALSE, PB_COLLISION);
//						tPrintf (4, 10, "%.3fkCPS", ((float)NumCollisions * (30.f * 1e6 / 1000.f)) / (float)CollTime);
				}
				// Evil cheat here - Sav turns off cloaking on all players in strat, I turn it back on here in predator
				if (Multiplayer && GetCurrentGameType() == PREDATOR) {
					for (i = 0; i < NumberOfPlayers; ++i)
						if (player[i].special)
							player[i].cloaked = -1;
				}
				// Game drawing
				processSparks();
				pbMark (0x808080, TRUE, PB_SFX);

				DrawWorld (0);
				
				if (Multiplayer)
				{
					int i,j;
					if (NumberOfPlayers == 3)
						j = 4;
					else j = NumberOfPlayers;
					for (i = 1; i < j; ++i)
					{
						doCam(i);
						DrawWorld (i);
					}
				}
				noShocks = 0;
				
				
				for (i = 0; i < NumberOfPlayers; i++) {
					player[i].aimLine = 0;	
					if (!GameOverFlag) {
						// player->var becomes 12345 on a failed challenge to fade back in 
						if (fadeMode[i] == 0 &&
							(player[i].Player == NULL ||
							(player[i].Player->health <= 0.f && player[i].Player->var != 12345))) {
							if (Multiplayer &&
								((GetCurrentGameType() == KNOCKOUT || GetCurrentGameType() == TAG) &&
								player[i].lives == 0)) {
								// Leave the camera there
							} else {
								fadeN[i] = -FRAMES_PER_SECOND;
								fadeLength[i] = 10;
								fadeMode[i] = FADE_TO_BLACK;
							}
						} else if (fadeMode[i] == FADE_TO_BLACK &&
							fadeN[i] >= fadeLength[i]) {
							if (((Multiplayer || player[0].PlayerState == PS_NORMAL) && 
								player[i].Player &&
								player[i].Player->health > 0) ||
								(player[i].Player && player[i].Player->var == 12345)) {
								fadeN[i] = -15;
								fadeLength[i] = 15;
								fadeMode[i] = FADE_FROM_BLACK;
							}
						}
					}
				}

			}
			
			// Update SFX
			sfxUpdate();

			pbMark (0xffff00, FALSE, PB_SFX);
			
#if GINSU
#define WHICH_STATE	GS_ADVERT
#else
#define WHICH_STATE	GS_TITLEPAGE
#endif
			
#if GINSU
			if (PadReset && !ChangingState()) {
					if (InputMode != DEMOWRITE)
						sbExitSystem();
					else 
						FadeToNewState (FADE_TO_BLACK_FAST, WHICH_STATE);
			}
#endif
			if (GameOverFlag && !ChangingState()) {
				if (Multiplayer) {
#if GINSU
					FadeToNewState (FADE_TO_BLACK, GS_ADVERT);
#else
					// New stylee - fade to black and then shungle to the debrief
					if (!BeginningOfTheEnd) {
						int i;
						for (i = 0; i < 4; ++i) {
							if (fadeMode[i] != FADE_TO_BLACK) {
								fadeN[i] = 0;
								fadeMode[i] = FADE_TO_BLACK;
								fadeLength[i] = 15;
							}
						}
						BeginningOfTheEnd = true;
						EndCounter = 0;
					} else if (EndCounter++ > 20) {
						extern STRAT *CamTracks[MAXCAM];
						int i;
						// Go through and kill off all the player strats
						for (i=0; i<NumberOfPlayers; i++) {
							if (player[i].Player) {
								Delete (player[i].Player);
							}
						}
						// Run for a frame to allow the deletion to take hold
						StratProc();

						player[0].cam = CameraCreate();
						player[0].cam->type = CAMERA_LOOKAROUND;
						player[0].cam->u.lookDir.rotX = player[0].cam->u.lookDir.rotY = player[0].cam->u.lookDir.rotZ = 0;
						
						setLevelCam(player[0].cam);
						
						// Go through and ensure the players are created correctly
						for (i=0; i<NumberOfPlayers; i++) {
							player[i].active = false; // deactivate
							player[i].lives = 1; // Just to make sure
						}

						levelInitialisationCode();
						
						MPStartupState = MP_DEBRIEF;
						DebriefPage = 0;
						fadeN[0] = 0;
						fadeMode[0] = FADE_FROM_BLACK;
						fadeLength[0] = 15;
					}
#endif
				} else {
					// End of credits?
					if (LevelNum == 13) {
						FadeToNewState (FADE_TO_BLACK_FAST, WHICH_STATE);
					} else if (LevelNum == 5 && LevelComplete) {
						LevelNum = 13; // Credits level
						FadeToNewState (FADE_TO_BLACK_FAST, GS_GAME);
					} else 
						FadeToNewState (FADE_TO_BLACK_FAST, WHICH_STATE);
				}
			}
			// Some mad timer craziness
			if (Multiplayer) {
				kmSetUserClipping (&vertBuf, KM_OPAQUE_POLYGON, 0, 0, 255, 255);
				kmSetUserClipping (&vertBuf, KM_OPAQUE_MODIFIER, 0, 0, 255, 255);
				kmSetUserClipping (&vertBuf, KM_TRANS_POLYGON, 0, 0, 255, 255);

				if (GetCurrentGameType() == TAG || GetCurrentGameType() == SUICIDE_TAG) {
					if (Winner == -1) {
						const char *time = psFormatTimeMilli ((Uint32)(BombCounter / (FRAMES_PER_SECOND*60)), (int)((BombCounter % (FRAMES_PER_SECOND*60)/FRAMES_PER_SECOND)), (BombCounter % FRAMES_PER_SECOND) * 100 / FRAMES_PER_SECOND);
						tScoreTime (PHYS_SCR_X/2 - 12 * psLastLength,
							PHYS_SCR_Y/2 - 20 + rGlobalYAdjust,
							time, MPTimeColour (BombCounter));
						DoTimeBacking();
					}
				} else if (GetCurrentGameType() == KING_OF_THE_HILL) {
					if (Winner == -1) {
						const char *time = psFormatTimeMilli ((int)(BombCounter / (FRAMES_PER_SECOND*60)), (int)((BombCounter % (FRAMES_PER_SECOND*60)/FRAMES_PER_SECOND)), (BombCounter % FRAMES_PER_SECOND) * 100 / FRAMES_PER_SECOND);
						tScoreTime (PHYS_SCR_X/2 - 12 * psLastLength,
							PHYS_SCR_Y/2 - 20 + rGlobalYAdjust,
							time, MPTimeColour (BombCounter));		
						DoTimeBacking();
					}
				} else if (GetCurrentGameType() == KEEP_THE_FLAG) {
					int pong;
					
					for (pong = 0; pong < NumberOfPlayers; ++pong) {
						if (player[pong].special) {
							const char *time = psFormatTimeMilli ((int)(player[pong].timer / (FRAMES_PER_SECOND*60)), (int)((player[pong].timer % (FRAMES_PER_SECOND*60)/FRAMES_PER_SECOND)), (player[pong].timer % FRAMES_PER_SECOND) * 100 / FRAMES_PER_SECOND);
							tScoreTime (PHYS_SCR_X/2 - 12 * psLastLength,
								PHYS_SCR_Y/2 - 20 + rGlobalYAdjust,
								time, MPTimeColour (player[pong].timer));
							DoTimeBacking();
						}
					}
				}

			}

			// Do we need to start dealing with end-game situations?
			if (LevelReset)
			{
				//ClearCollGlobals();
				DamLastFrame = -1;
				//rFXinit();
				ResetMap();
				lInit();
				levelInitialisationCode();
				StartFade (FADE_FROM_BLACK);
			}
			
			break; // end of normal in game processing
			
		case MP_CAMERA_TRACK:
			RunPreGameMadness();
			y = 8;

			PUSH_MATRIX();

			{
				Matrix32 mat;
				mUnit32 (&mat);
				mPostTranslate32 (&mat, -PHYS_SCR_X/2,0);
				mPostScale32 (&mat, 1.5f, 1.5f);
				mPostTranslate32 (&mat, PHYS_SCR_X/2,0);
		
				mMultiply32 (psGetMatrix(), &mat, psGetMatrix());
			}

			if (mpSettings.playMode == TOURNAMENT) {
				psDrawTitle (psFormat ("`ROUNDN", CurrentTournamentGame+1), y, 0, 0xff000000); 
				y+=40;
			}

			team = FALSE;
			if (mpSettings.playMode == TEAM)
				team = TRUE;
			if (mpSettings.playMode == TOURNAMENT &&
				mpSettings.game.tournament.team[CurrentTournamentGame] &&
				NumberOfPlayers == 4)
				team = TRUE;
			if (NumberOfPlayers < 3)
				team = FALSE; // Paranoia
			psDrawTitle (
				psFormat ("%s %s", team ? LangLookup("TEAM") : "",
				GameTypeName[GetCurrentGameType()]), y, 0, 0xff000000);
			y+=64;

			POP_MATRIX();
			y*=2;

			{
				char blurbBuf[512];
				char **blurb = GameBlurb[GetCurrentGameType() + (team * NUM_MPLAYER_TYPES)];
#define SQUIB(foo) \
				sprintf (blurbBuf, LangLookup(blurb[(GetCurrentGameParms()->foo == 1) ? 1 : 0]), GetCurrentGameParms()->foo)
				switch (GetCurrentGameType()) {
				case DEATHMATCH:
					SQUIB(dmParms.fragLimit);
					break;
				case KNOCKOUT:
					SQUIB(koParms.startLives);
					break;
				case PREDATOR:
					SQUIB (predatorParms.fragLimit);
					break;
				case KEEP_THE_FLAG:
					sprintf (blurbBuf, LangLookup(blurb[0]), (int)(GetCurrentGameParms()->ktfParms.winTime / 60), 
						GetCurrentGameParms()->ktfParms.winTime % 60);
					break;
				case KING_OF_THE_HILL:
					sprintf (blurbBuf, LangLookup(blurb[0]), (int)(GetCurrentGameParms()->kothParms.winTime / 60),
						GetCurrentGameParms()->kothParms.winTime % 60);
					break;
				case TAG:
					sprintf (blurbBuf, LangLookup(blurb[0]), (int)(GetCurrentGameParms()->bombParms.bombTimer / 60),
						GetCurrentGameParms()->bombParms.bombTimer % 60);
					break;
				case SUICIDE_TAG:
					sprintf (blurbBuf, LangLookup(blurb[0]), (int)(GetCurrentGameParms()->suicideParms.bombTimer / 60),
						GetCurrentGameParms()->suicideParms.bombTimer % 60						);
					break;
				}
				y += psCentreWordWrap (blurbBuf, y, 400);
			}

			if (TargetRot != STARTROT) {
				psSetColour (ColLerp (0xffffff,0x202030, sin(currentFrame*0.05f)*0.5f +0.5f));
				psDrawCentred ("`HITSTART", 360);

				readyToStart = 0;

				for (i = 0; i < NumberOfPlayers; ++i)
				{
					if (PadPress[i] & PADSTART)
					{
						Bingle();
						readyToStart = 1;
					}
				}

				if (readyToStart)
				{
					readyToStart = 0;
					TargetRot = STARTROT;

					if (team)
						player[i].team = 1+(i&1);
				}
			} else {
				if (fabs (RotAmt - TargetRot) < 0.1f) {
					// Do we need to choose teams?
					if (team) {
						MPStartupState  = MP_CHOOSE_TEAMS;
						// Are we in a tournament game - if so randomise the team selection
						if (mpSettings.playMode == TOURNAMENT) {
							int nInTeam;
							ReseedRandom();
							do {
								nInTeam = 0;
								for (i = 0; i < NumberOfPlayers; ++i) {
									player[i].team = (rand() & 1) + 1;
									if (player[i].team == 1)
										nInTeam++;
								}
							} while (nInTeam != 2);
						}
						TargetRot = 0;
					} else {
						TargetRot = 0;
						switch (GetCurrentGameType()) {
						default:
							FireHerUp();
							break;
						case PREDATOR:
							MPStartupState = MP_PREDATOR;
							break;
						case TAG:
							SetBombNum (1);
							SpinSpeed = 20.f+frand() * 30.f;
							MPStartupState = MP_BOMB;
							break;
						case SUICIDE_TAG:
							SetBombNum (NumberOfPlayers-1);
							SpinSpeed = 20.f+frand() * 30.f;
							MPStartupState = MP_BOMB;
							break;
						}
					}

				}
			}

			break;


		case MP_TEAM_SUICIDE_BOMB:
			RunPreGameMadness();
			DrawMadness ("%s", NULL);
			DoBombs();
			if (TargetRot != 0) {
				// Actually hand out the bombs
				for (i = 0; i < NumberOfPlayers; ++i) {
					if (player[i].team == BombTeam)
						player[i].special = TRUE;
				}
				BombCounter = GetCurrentGameParms()->suicideParms.bombTimer * FRAMES_PER_SECOND;
				FireHerUp();
			} else if (SpinSpeed < 0.05f) {
				Bingle();
				if (--magicWaitCount > 0) {
					// Do nothing
				} else {
					TargetRot = STARTROT;
				}
			} else {
				// Spin the wheel a bit
				SpinAcc+=MIN (SpinSpeed, 4);
				magicWaitCount = 30;
				while (SpinAcc > 10) {
					SpinAcc-=10;
					BombTeam = 3 - BombTeam;
					Jingle();
				}
			}
			SpinSpeed -= MIN (1.f, SpinSpeed * 0.02f);
			
			break;

		case MP_BOMB:
		case MP_TEAM_BOMB:
			RunPreGameMadness();
			DrawMadness ("%s", NULL);
			DoBombs();

			if (TargetRot != 0) {
				// Actually hand out the bombs
				for (i = 0; i < NumBombs; ++i) {
					player[BombArray[i].player].special = TRUE;
				}
				if (GetCurrentGameType() == TAG)
					BombCounter = GetCurrentGameParms()->bombParms.bombTimer * FRAMES_PER_SECOND;
				else
					BombCounter = GetCurrentGameParms()->suicideParms.bombTimer * FRAMES_PER_SECOND;
				FireHerUp();
			} else if (SpinSpeed < 0.05f) {
				Bingle();
				if (--magicWaitCount > 0) {
					for (i = 0; i < NumBombs; ++i)
						BombProc (i, PadPush[BombArray[i].player]);
				} else {
					float distMax = 1e9f;
					for (i = 0; i < NumBombs; ++i) { 
						float dist  = sqrt (SQR(BombArray[i].pos.x - BombArray[i].targetPos.x) + SQR(BombArray[i].pos.y - BombArray[i].targetPos.y));
						if (dist < distMax)
							distMax = dist;
					}
					if (distMax < 0.75f && magicWaitCount < -100) {
						TargetRot = STARTROT;
					}
				}
			} else {
				// Spin the wheel a bit
				SpinAcc+=MIN (SpinSpeed, 4);
				magicWaitCount = 30;
				while (SpinAcc > 10) {
					Jingle();
					SpinAcc-=10;
					if (NumberOfPlayers == 2) {
						BombArray[0].player = 1 - BombArray[0].player;
					} else {
						for (i = 0; i < NumBombs; ++i) {
							switch (BombArray[i].player) {
							case 0:
								BombArray[i].player = 1;
								break;
							case 1:
								if (NumberOfPlayers == 3) 
									BombArray[i].player = 2;
								else
									BombArray[i].player = 3;
								break;
							case 2:
								BombArray[i].player = 0;
								break;
							case 3:
								BombArray[i].player = 2;
								break;
							}
						}
					}
				}
				for (i = 0; i < NumBombs; ++i)
					BombArray[i].pos = BombArray[i].targetPos = BombArray[i].startPos = GetBombPos(BombArray[i].player);
			}
			SpinSpeed -= MIN (1.f, SpinSpeed * 0.02f);
			break;

		case MP_CHOOSE_TEAMS:
			RunPreGameMadness();
			DrawMadness("`SOMEONESTEAM", TeamHandler);
			
			if (TargetRot != STARTROT) {
				for (i = 0; i < NumberOfPlayers; ++i) {
					if (PadPush[i] & (PADLEFT|PADRIGHT) && mpSettings.playMode != TOURNAMENT) {
						player[i].team = 3-player[i].team;
					} else if (PadPush[i] & PADSTART) {
						int j, k=0;
						for (j = 0; j < NumberOfPlayers; ++j)
							k|= player[j].team;
						if (k == 3) {
							TargetRot = STARTROT;
							Bingle();
						}
					}
				}
			} else if (fabs (RotAmt - TargetRot) < 0.1f) {
				TargetRot = 0;
				switch (GetCurrentGameType()) {
				default:
					FireHerUp();
					break;
				case PREDATOR:
					MPStartupState = MP_TEAM_PREDATOR;
					break;
				case TAG:
					SetBombNum (1);
					SpinSpeed = 30.f+frand() * 40.f;
					MPStartupState = MP_BOMB;
					break;
				case SUICIDE_TAG:
					SetBombNum(0);
					BombTeam = (rand() & 1)+1;
					SpinSpeed = 30.f+frand() * 40.f;
					MPStartupState = MP_TEAM_SUICIDE_BOMB;
					break;
				}
			}
			break;

		case MP_PREDATOR:
			// Find the current predator
			for (i = 0; i < NumberOfPlayers; ++i) {
				if (player[i].special)
					break;
			}
			if (i == NumberOfPlayers) {
				// No predator yet, make one
				i = rand() % NumberOfPlayers;
				SpinSpeed = frand() * 60.f + 40.f;
			}
			if (SpinSpeed < 0.1f) {
				if (!choosed && SpinSpeed < 0.05f)
					MPChosen();		

				if (SpinSpeed < 0.02f) {
					TargetRot = STARTROT;
					if (fabs (RotAmt - TargetRot) < 0.1f) {
						MakePredator (i);
						FireHerUp();
					}
				}
			} else {
				// Spin the wheel a bit
				player[i].special = FALSE;
				SpinAcc+=MIN (SpinSpeed, 4);
				// In 4 player, swap 3 and 4
				if (NumberOfPlayers==4 && (i==2 || i == 3))
					i = 5-i;
				while (SpinAcc > 10) {
					Jingle();
					SpinAcc-=10;
					i = (i+1) % NumberOfPlayers;
				}
				// In 4 player, swap 3 and 4 back
				if (NumberOfPlayers==4 && (i==2 || i == 3))
					i = 5-i;
				player[i].special = TRUE;

			}
			SpinSpeed -= MIN (1.f, SpinSpeed * 0.02f);
			RunPreGameMadness();
			DrawMadness("%s", PredHandler);
			break;

		case MP_TEAM_PREDATOR:
			// Find the current predator
			for (i = 0; i < NumberOfPlayers; ++i) {
				if (player[i].special)
					break;
			}
			if (i == NumberOfPlayers) {
				// No predator yet, make one
				i = rand() % NumberOfPlayers;
				SpinSpeed = frand() * 60.f + 40.f;
			}
			if (SpinSpeed < 0.1f) {
				if (!choosed && SpinSpeed < 0.05f)
					MPChosen();		
				if (SpinSpeed < 0.02f) { 
					TargetRot = STARTROT;
					if (fabs (RotAmt - TargetRot) < 0.1f) {
						MakePredator (i);
						FireHerUp();
					}
				}
			} else {
				int j;
				// Spin the wheel a bit
				// Ensure any other players are no longer predators
				for (j = 0; j < NumberOfPlayers; ++j)
					player[j].special = FALSE;
				SpinAcc+=MIN (SpinSpeed, 4);
				while (SpinAcc > 10) {
					SpinAcc-=10;
					Jingle();
					// Find someone on the other team
					for (j = 0; j < NumberOfPlayers; ++j)
						if (player[j].team != player[i].team) {
							i = j;
							break;
						}
				}
				player[i].special = TRUE;

			}
			SpinSpeed -= MIN (1.f, SpinSpeed * 0.02f);
			RunPreGameMadness();
			DrawMadness("%s", TeamPredHandler);
			break;

		case MP_TRANSITION_TO_GAME:
			if (NumberOfPlayers == 2) {
				SetWindow (player[0].cam, WINDOW_TOP);
				if (player[1].cam)
					SetWindow (player[1].cam, WINDOW_BOTTOM);
			} else if (NumberOfPlayers != 1) {
				SetWindow (player[0].cam, WINDOW_TL);
				
				if (player[1].cam)
					SetWindow (player[1].cam, WINDOW_TR);
				if (player[2].cam)
					SetWindow (player[2].cam, WINDOW_BL);
				if (player[3].cam)
					SetWindow (player[3].cam, WINDOW_BR);
			}
			
			if (Multiplayer || !BeingBriefed) {
				if (MPCountIn++ > 3 * FRAMES_PER_SECOND) {
					// Play 'Go' sound
					sfxPlay (GENERIC_GO, 1, 0);
					MPStartupState = MP_IN_GAME;
					currentFrame = 0;
				}
			}
			
			if (MPCountIn >= 0 && (MPCountIn % FRAMES_PER_SECOND) == 2) {
				sfxPlay (GENERIC_READY, 1, 0);
			}
			
			for (i = 0; i < NumberOfPlayers; ++i) {
				int time;
				time = ((3*FRAMES_PER_SECOND) - MPCountIn) / (int)FRAMES_PER_SECOND;
				time = RANGE(0, time+1, 3);
				doCam(i);
				DrawWorld (i);
				
				mUnit32 (psGetMatrix());

				psSetAlpha (1.f);
				psSetColour (0xffffff);
				psSetPriority (W_LAYER_AMMO - 1000.f);
				if ((Multiplayer || !BeingBriefed) && MPCountIn>=0) {
					psDrawTitle (psFormatNum (time), 32.f, 0, 0xff000000);
					if (fadeMode[i] == FADE_TO_BLACK && fadeN[i] >= fadeLength[i]) {
						fadeN[i] = 0;
						fadeLength[i] = 5;
						fadeMode[i] = FADE_FROM_BLACK;
					}
				}
			}
			if (NumberOfPlayers == 3) {
				doCam(3);
				DrawWorld(3);
			}

			SavedPadPress = PadPress[0];
			SavedPadPush = PadPush[0];
			SavedJoyY = joyState[0].y;
			for (i = 0; i < NumberOfPlayers; ++i) {
				PadPress[i] = PadPush[i] = 0;
				joyState[i].x = joyState[i].y = 0.f;
				player[i].PlayerAcc=0.0f;
			}
			MoveAllStrats();
			StratProc();
			ProcessAllStrats();

			break;

		case MP_FADE_OUT_TO_BLACK:
			RunPreGameMadness();
			FireHerUp();
			break;

		case MP_DEBRIEF:
			RunPreGameMadness();
			RunDebriefingAction();
			break;
		} // end switch of multiplayer subtype


		break;
	case GSR_FINALISE:
		kmAdjustDisplayCenter (adjustedX, adjustedY);
		// Unmap all the ports
		for (i = 0; i < 4; ++i)
			ReleasePort (i);
		PAL = OLDPAL;
		DisableTexDMAs = TRUE;
		UnlockCurrentProfile();
		if (GameWasRandom) {
			mpSettings.game.tournament.game[CurrentTournamentGame].gameType = NUM_MPLAYER_TYPES;
		} 
		if (MapWasRandom) {
			switch (mpSettings.playMode) {
			case TOURNAMENT:
				mpSettings.game.tournament.game[CurrentTournamentGame].map = numMPMaps;
				break;
			case TEAM:
				mpSettings.game.team.map = numMPMaps;
				break;
			case SKIRMISH:
				mpSettings.game.skirmish.map = numMPMaps;
				break;
			}
		}
		SoundStopTrack();
		if (!Multiplayer)
			ReleasePort(0);
		MapClean();
	   	InputClose();
		InputMode = NORMALINPUT;

		if (Multiplayer)
			MPFinalise();

		MapReset(map);
		FreeDome();
		player[currentPlayer].cam = NULL;
		Savmap = map = NULL;
		rReset();
		SHeapClear (GameHeap);
		DisableTexDMAs = FALSE;
		DoFades();
		rEndFrame();
		PauseMode = 0;
		break;
	}

}

void GetPlayerColour(STRAT *strat, int pn)
{
	Uint32 col;
	if (IsTeamGame()) {
		if (player[pn].team == 1) {
			strat->CheckPos.x = 0.5f + (0.5f/3.f)*pn;
			strat->CheckPos.z = 0;
		} else {
			strat->CheckPos.x = 0;
			strat->CheckPos.z = 0.5f + (0.5f/3.f)*pn;
		}
		strat->CheckPos.y = 0;
	} else {
		col = TankPalette[CurProfile[LogToPhys(pn)].multiplayerStats.colour];
		strat->CheckPos.x = ((float)((col & 0x00ff0000)>>16)) * (1.f / 255.0f);
		strat->CheckPos.y = ((float)((col & 0x0000ff00)>>8)) * (1.f / 255.0f);
		strat->CheckPos.z = ((float)((col & 0x000000ff)>>0)) * (1.f / 255.0f);
	}
}

static int PlayerComparer (void *v1, void *v2)
{
	int p1, p2;
	int p1Score, p2Score;

	p1 = *(int *)v1;
	p2 = *(int *)v2;

	p1Score = GetPlayerScore(p1);
	p2Score = GetPlayerScore(p2);

	return (p2Score - p1Score);
}
extern int			TournamentScore[4];

static int TournamentComparer (void *v1, void *v2)
{
	int p1, p2;
	int p1Score, p2Score;

	p1 = *(int *)v1;
	p2 = *(int *)v2;

	p1Score = TournamentScore[p1];
	p2Score = TournamentScore[p2];

	return (p2Score - p1Score);
}
static void psDrawTab (const char *c, float x, float y)
{
	float wid = psStrWidth (c) * 0.5f;
	psDrawString (c, x - wid, y);
}
int PlayerOrder[4], TeamOrder[2];
static void	RunDebriefingAction()
{
	static float alpha[3] = { 0.f, 0.f, 0.f };
	static float colWid[6], tableScale, tableOffset;
	static int WaitCounter, TournOrder[4];
	static const char *Position[4] = { "`WINNER", "`SECOND", "`THIRD", "`FOURTH" };
	int i, j, score;
	Matrix32 mat;
	float x, xSize;
	Uint32 press = 0;
	bool teamResults = ((IsTeamGame()) && (DebriefPage != 3));

	for (i = 0; i < NumberOfPlayers; ++i)
		press |= GetPadFE(LogToPhys(i));

	if (teamResults)
		psSetColour ((TeamOrder[i]==0) ? 0xff0000 : 0x0000ff);
	else
		psSetColour (GetTankColour(Winner));
	psSetAlpha(1);
	psSetPriority(0);
	if (teamResults) {
		psDrawTitle (psFormat("`TEAMWIN", LangLookup((player[Winner].team == 0) ? "RED" : "BLUE")), 16, 0, 0xff000000);
	} else {
		if (DebriefPage == 3) {
			// A tournament game: is this the last game?
			if (CurrentTournamentGame == (mpSettings.game.tournament.nGames-1)) {
				if (TournamentScore[TournOrder[0]] == TournamentScore[TournOrder[1]]) {
					psSetColour (0xffffff);
					psDrawTitle ("`ITSADRAW", 16, 0, 0xff000000);
				} else {
					psSetColour (GetTankColour (TournOrder[0]));
					psDrawTitle (psFormat("`TOURNOVERALLWINNER", CurProfile[LogToPhys(TournOrder[0])].name), 16, 0, 0xff000000);
				}
			} else {
				psSetColour (0xffffff);
				if (CurrentTournamentGame)
					psDrawTitle (psFormat("`RESULTAFTERNGAMES", CurrentTournamentGame+1), 16, 0, 0xff000000);
				else
					psDrawTitle (psFormat("`RESULTAFTERNGAME", CurrentTournamentGame+1), 16, 0, 0xff000000);
			}
		} else {
			psDrawTitle (psFormat("`SINGLEWIN", CurProfile[LogToPhys(Winner)].name), 16, 0, 0xff000000);
		}
	}

	if (DebriefPage == 1) {
		alpha[0] += (1-alpha[0]) * 0.1f;
		alpha[1] += (0-alpha[1]) * 0.2f;
		alpha[2] += (0-alpha[2]) * 0.2f;
	} else if (DebriefPage == 2) {
		alpha[0] += (0-alpha[0]) * 0.2f;
		alpha[1] += (1-alpha[1]) * 0.1f;
		alpha[2] += (0-alpha[2]) * 0.2f;
	} else {
		alpha[0] += (0-alpha[0]) * 0.2f;
		alpha[1] += (0-alpha[1]) * 0.2f;
		alpha[2] += (1-alpha[2]) * 0.1f;
	}

	memcpy (&mat, psGetMatrix(), sizeof (mat));

	if (DebriefPage == 0) {
		float width;
		// Initialise all this malarky
		alpha[0] = alpha[1] = 0.f;
		DebriefPage = 1;
		// Time to sort our people:
		for (i = 0; i < NumberOfPlayers; ++i)
			PlayerOrder[i] = i;
		qsort (PlayerOrder, NumberOfPlayers, sizeof (PlayerOrder[0]), PlayerComparer);

		if (IsTeamGame()) {
			if (player[Winner].team == 1) {
				TeamOrder[0] = 0;
				TeamOrder[1] = 1;
			} else {
				TeamOrder[0] = 1;
				TeamOrder[1] = 0;
			}
		}

		// For the in-depth madness screen we shurd work out what scale we need
		// First column is 'Player Name',
		colWid[0] = psStrWidth("`NAME") + 16.f;
		// Next 'np' columns are the player name widths
		for (i = 1; i < NumberOfPlayers+1; ++i) {
			colWid[i] = psStrWidth (CurProfile[LogToPhys(i-1)].name) + 16.f;
			if (colWid[i] > colWid[0])
				colWid[0] = colWid[i];
		}
		// Last column is 'Total kills'
		colWid[i] = psStrWidth ("`TOTAL") + 16.f;

		width = 0;
		for (i = 1; i < 5; ++i) 
			width = MAX(width, colWid[i]);
		for (i = 1; i < 5; ++i) 
			colWid[i] = width;

		memset (alpha, 0, sizeof(alpha));

		// Right, now to find the total width
		width = 0;
		for (i = 0; i < 2+NumberOfPlayers; ++i)
			width += colWid[i];

		// Do we need to apply some evil scaling?
		if (width > (640-32)) {
			tableScale = (640.f-32.f) / width;
			tableOffset = 8;
		} else {
			tableScale = 1.f;
			tableOffset = 320 - width/2 - 8;
		}

		WaitCounter = 0;

		// Tournament score calculation
		if (mpSettings.playMode == TOURNAMENT) {
			// Now to recalculate the scores and suchlike
			if (IsTeamGame()) {
				// Everyone on the winning team gets points
				int WinningTeam = player[Winner].team;
				int points;
				
				// In a three player game, 2 points to the winners
				if (NumberOfPlayers == 3) {
					points = 2;
				} else {
					// 3 points to the winner in a 3v1 or 2v2 game
					points = 3;
				}
				for (i = 0; i < NumberOfPlayers; ++i)
					if (player[i].team == WinningTeam)
						TournamentScore[i]+=points;
			} else {
				// Work down the ranks, dishing out points for equal places
				int points;
				points = NumberOfPlayers - 1;
				i = 0;
				while (i < NumberOfPlayers) {
					int score;
					// Give this player points points
					TournamentScore[PlayerOrder[i]]+= points;
					score = GetPlayerScore (PlayerOrder[i]);
					i++;
					// Dish out the same points to anyone with the same score
					while (i < NumberOfPlayers && GetPlayerScore(PlayerOrder[i]) == score) {
						TournamentScore[PlayerOrder[i]]+= points;
						i++;
					}
					points --;
				}
			}
			
			// Now sort TournOrder by tournament order
			TournOrder[0] = 0; TournOrder[1] = 1; TournOrder[2] = 2; TournOrder[3] = 3;
			qsort (TournOrder, NumberOfPlayers, sizeof (TournOrder[0]), TournamentComparer);		
		}

	}
	// Page one
	if (alpha[0] > (1.f/256.f)) {
		bool team = IsTeamGame();

		switch (GetCurrentGameType()) {
		case KNOCKOUT:
		case TAG:
		case KING_OF_THE_HILL:
			team = false;
			break;
		}

		psSetAlpha(alpha[0]);

		switch (GetCurrentGameType()) {
		default:
			x = 112;
			xSize = 208;
			break;
		case KEEP_THE_FLAG:
		case KING_OF_THE_HILL:
			x = 164;
			xSize = 312;
			break;
		}

		for (i = 0; i < (team ? 2 : NumberOfPlayers); ++i) {
			float y = 128 + 50 * i;

			psSetColour (0xffffff);
			psDrawTab (Position[i], x, y);
			
			psSetColour (team ? ((TeamOrder[i]==0) ? 0xff0000 : 0x0000ff) : GetTankColour(PlayerOrder[i]));
			if (!team)
				psDrawTab (CurProfile[LogToPhys(PlayerOrder[i])].name, x+xSize, y);
			else
				psDrawTab ((TeamOrder[i]==0) ? "`RED" : "`BLUE", x+xSize, y);

			switch (GetCurrentGameType()) {
			case DEATHMATCH:
			case PREDATOR:
				score = 0;
				if (team) {
					int k;
					for (j = 0; j < NumberOfPlayers; ++j) {
						if (player[j].team == TeamOrder[i]+1) {
							for (k = 0; k < NumberOfPlayers; ++k) {
								score += GetPlayerKills(j, k);
							}
						}
					}
				} else {
					for (j = 0; j < NumberOfPlayers; ++j)
						score += GetPlayerKills(PlayerOrder[i], PlayerOrder[j]);
				}
				psDrawTab (psFormatNum(score), x+(xSize*2), y);
				break;
			case KNOCKOUT:
			case TAG:
			case SUICIDE_TAG:
				if (i == 0 ||
					(team && player[PlayerOrder[i]].team == player[Winner].team))
					break;
				score = GetPlayerFramesAlive(PlayerOrder[i]);
				{
					GameParms *g = GetCurrentGameParms();
					if (GetCurrentGameType() == TAG)
						score = MIN (score, FRAMES_PER_SECOND * g->suicideParms.bombTimer);
					if (GetCurrentGameType() == SUICIDE_TAG)
						score = MIN (score, FRAMES_PER_SECOND * g->suicideParms.bombTimer);
				}
				psDrawTab (psFormatTimeMilli (
					score / (60 * FRAMES_PER_SECOND),
					(score / FRAMES_PER_SECOND) % 60,
					(int)((float)(score%FRAMES_PER_SECOND) * 100/FRAMES_PER_SECOND)), x+(xSize*2), y);
				break;
			}
		}
	}

	// Page two
	if (alpha[1] > (1.f/256.f)) {
		float pos;

		psSetAlpha(alpha[1]);
		// Table action!
		// Apply the scale, innit
		memcpy (psGetMatrix(), &mat, sizeof (mat));
		mPostScale32 (psGetMatrix(), tableScale, tableScale);
		mPostTranslate32 (psGetMatrix(), tableOffset + 8, 100);
		// Heading column
		psSetColour(0xffffff);
		psDrawTab ("`NAME", colWid[0]*0.5f, 0);
		pos = colWid[0];
		for (i = 0; i < NumberOfPlayers; ++i) {
			psSetColour (GetTankColour(PlayerOrder[i]));
			psDrawTab (CurProfile[LogToPhys(PlayerOrder[i])].name, pos + colWid[PlayerOrder[i]+1]*0.5f, 0);
			pos += colWid[PlayerOrder[i]+1];
		}
		psSetColour (0xffffff);
		psDrawTab ("`TOTAL", pos + colWid[i+1]*0.5f, 0);

		for (i = 0; i < NumberOfPlayers; ++i) {
			float y = (50+50*i)/tableScale;
			int j;
			psSetColour (GetTankColour(PlayerOrder[i]));
			psDrawTab (CurProfile[LogToPhys(PlayerOrder[i])].name, colWid[0]*0.5f, y);
			psSetColour (0xffffff);
			pos = colWid[0];
			score = 0;
			for (j = 0; j < NumberOfPlayers; ++j) {
				psDrawTab (psFormatNum (GetPlayerKills(PlayerOrder[i], PlayerOrder[j])), pos + colWid[j+1]*0.5f, y);
				pos += colWid[PlayerOrder[j]+1];
				score += GetPlayerKills(PlayerOrder[i], PlayerOrder[j]);
			}
			psDrawTab (psFormatNum (score), pos + colWid[j+1]*0.5f, y);
		}
	}

	// Tournament page 3
	if (alpha[2] > (1.f/256.f)) {
		psSetAlpha(alpha[2]);
		memcpy (psGetMatrix(), &mat, sizeof (mat));
		x = 112;
		xSize = 208;
		
		for (j = 0; j < NumberOfPlayers; ++j) {
			float y = 128 + 50 * j;
			
			i = TournOrder[j];
			psSetColour (0xffffff);
			psDrawTab (Position[j], x, y);
			
			psSetColour (GetTankColour(i));
			psDrawTab (CurProfile[LogToPhys(i)].name, x+xSize, y);
			
			psDrawTab (psFormatNum (TournamentScore[i]), x+(xSize*2), y);
		}
	}

	memcpy (psGetMatrix(), &mat, sizeof (mat));
	psSetColour(0xffffff);
	psSetAlpha(1);
	mPostTranslate32 (psGetMatrix(), -PHYS_SCR_X/2, 0);
	mPostScale32 (psGetMatrix(), 0.75f, 0.75f);
	mPostTranslate32 (psGetMatrix(), PHYS_SCR_X/2, PHYS_SCR_Y - 100);

	if (WaitCounter++ > 2*FRAMES_PER_SECOND) {
		switch (GetCurrentGameType()) {
		case DEATHMATCH:
		case KNOCKOUT:
		case PREDATOR:
		case KING_OF_THE_HILL:
			if (DebriefPage == 1 && (press & PDD_DGT_KR)) {
				FE_MoveSound();
				DebriefPage = 2;
			}
			
			if (DebriefPage == 2 && (press & PDD_DGT_KL)) {
				FE_MoveSound();
				DebriefPage = 1;
			}
			
			if (DebriefPage != 3)
				psDrawCentred ("`DPADNEXTPAGE", -35*2);
			break;
		}
		if (mpSettings.playMode == TOURNAMENT) {
			if (DebriefPage != 3) {
				psDrawTitle ("`TOURNCONTINUE", 0, 0, 0xff000000);
				if (press & PDD_DGT_ST) {
					DebriefPage = 3;
					if (CurrentTournamentGame == mpSettings.game.tournament.nGames-1 &&
						TournamentScore[TournOrder[0]] != TournamentScore[TournOrder[1]])
						SFX_WinTournament();
					else
						FE_ACK();
						
				}
			} else {
				if (CurrentTournamentGame == mpSettings.game.tournament.nGames-1) {
					// Was there a draw?
					if (TournamentScore[TournOrder[0]] != TournamentScore[TournOrder[1]]) {
						psDrawTitle ("`TOURNBACK", 0, 0, 0xff000000);
						if (!ChangingState()) {
							if (press & PDD_DGT_ST) {
								FE_ACK();
								FadeToNewState (FADE_TO_BLACK, GS_TITLEPAGE);
							}
						}
					} else {
						psDrawTitle ("`TOURNDRAW", 0, 0, 0xff000000);
						if (!ChangingState()) {
							if (press & PDD_DGT_ST) {
								FE_ACK();
								FadeToNewState (FADE_TO_BLACK, GS_GAME);
							}
						}
					}
				} else {
					psDrawTitle ("`TOURNMID", 0, 0, 0xff000000);
					if (!ChangingState()) {
						if (press & PDD_DGT_ST) {
							FE_ACK();
							FadeToNewState (FADE_TO_BLACK, GS_GAME);
						}
					}
				}
			}
		} else {
			psDrawCentred ("`YREPLAY", -35);
			psDrawCentred ("`ASTARTBACKSETUP", 0);
			if (!ChangingState()) {
				if (press & PDD_DGT_TY) {
					FE_ACK();
					FadeToNewState (FADE_TO_BLACK, GS_GAME);
				}
				if (press & (PDD_DGT_ST|PDD_DGT_TA)) {
					FE_ACK();
					FadeToNewState (FADE_TO_BLACK, GS_TITLEPAGE);
				}
			}
		}
	}
}