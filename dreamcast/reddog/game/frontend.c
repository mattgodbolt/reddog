/*
 * Front-end handler for Red Dog
 * Also menu system
 */

#include "RedDog.h"
#include "view.h"
#include "GameState.h"
#include "Render\internal.h"
#include "Input.h"
#include "Menu.h"
#include "level.h"
#include "MPlayer.h"
#include "VMSStruct.h"
#include "backup.h"
#include "Loading.h"
#include "LevelDepend.h"
#if GINSU
#include <ginsu.h>
#endif
#define TIME(MINS,SECS) ((float)((MINS)*60.f + (SECS)))

extern int GameVMS, GameProfile, GameDifficulty;
// Tweak these routines!
/*************************************************
 ** Sefton the par times and scores lurk below: **
 ** The TIME() macro accepts a minutes, seconds **
 ** parameter list, eg TIME(1,30) is one minute **
 ** thirty seconds.  Nice, eh?                  **
 *************************************************/

// Level par times : 
int GetTimeRating (int level, float time)
{
	float topTime = 6*60.f, relTime;
	switch (level) {
	case 0: // Volcano
		//Tweaked
		topTime = TIME(3,00);
		break;
	case 1: // Ice
		//Tweaked
		topTime = TIME(5,30);
		break;
	case 2: // Canyon
		//Tweaked
		topTime = TIME(4,00);
		break;
	case 3: // Hydro
		//Tweaked
		topTime = TIME(5,30);
		break;
	case 4: // City
		//Tweaked
		topTime = TIME(4,30);
		break;
	case 5: // Alien
		//Tweaked
		topTime = TIME(4,30);
		break;
	default:
		dAssert (FALSE, "uck");
		break;
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
		return 0;
	}
	if (time < topTime)
		return 25;
	relTime = time - topTime;
	return RANGE(0, 25 - ((relTime/(4*60.f))*25), 25); // Linear falloff to four minutes later
}


// Level par scores : 
int GetScoreRating (int level, int score)
{
	float topScore = 20000.f;
	switch (level) {
	case 0: // Volcano
		//Tweaked
		topScore = 70000;
		break;
	case 1: // Ice
		//Tweaked
		topScore = 90000;
		break;
	case 2: // Canyon
		//Tweaked
		topScore = 100000;
		break;
	case 3: // Hydro
		//Tweaked
		topScore = 150000;
		break;
	case 4: // City
		//Tweaked
		topScore = 215000;
		break;
	case 5: // Alien
		//Tweaked
		topScore = 180000;
		break;
	default:
	//	dAssert (FALSE, "uck");
		break;
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
		if (!IsScoreImportant(level))
			return 0;
		else
			return RANGE(0, score, 25);
		break;
	}
	return RANGE(0, ((float)score / topScore)*25, 25);
}


// Boss par times : 
int GetBossTimeRating (int level, float time)
{
	float topTime = 60.f, relTime;
	switch (level) {
	case 0: // Volcano
		//Tweaked
		if (GameDifficulty==0)
			topTime = TIME(0,20);
		else
			topTime = TIME(1,00);
		break;
  	case 1: // Ice
		//Tweaked
		if (GameDifficulty==0)
			topTime = TIME(0,35);
		else
			topTime = TIME(1,30);
		break;
	case 2: // Canyon
		//Tweaked
		if (GameDifficulty==0)
			topTime = TIME(1,20);
		else
			topTime = TIME(1,30);
		break;
	case 3: // Hydro
		//Tweaked
		topTime = TIME(1,20);
		break;
	case 4: // City
		//Tweaked
		topTime = TIME(2,00);
		break;
	case 5: // Alien
		//Tweaked
		topTime = TIME(4,30);
		break;
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
		return 0;
	}
	if (time < topTime)
		return 25;
	relTime = time - topTime;
	return RANGE(0, 25 - ((relTime/120)*25), 25); // Linear falloff to two minutes later
}
// End of Sefton editable section


void LockCurrentProfile (void);

#define ANY_BUTTON (PDD_DGT_TA|PDD_DGT_TB|PDD_DGT_TX|PDD_DGT_TY|PDD_DGT_ST)

/*
 * Variables
 */

static Camera *fEndCam;
static enum { 
	FE_START, 
	FE_TRANSITION, 
	FE_MP, 
	FE_MP_SETUP,
	FE_GAME_PRE_MAP,
	FE_WORLD_MAP,
	FE_SIXTY_TEST
 } FrontEndState, NextFrontEndState;
extern Bool suppressDebug;
static Bool NeedToUnloadFrontEnd = FALSE;

#define VOLCANODEMO 0
#define CANYONDEMO 5
#define SINGLEPLAYERDEMO 0
static int DemoCount = 0, CrntDemo;
static struct {
	int multiplayer;
	int level;
} Demo[] =
{

	{SINGLEPLAYERDEMO,VOLCANODEMO},
	{SINGLEPLAYERDEMO,CANYONDEMO},
	{ 1, 0},
	{SINGLEPLAYERDEMO,VOLCANODEMO},
	{SINGLEPLAYERDEMO,CANYONDEMO},
	
};

extern void rUpdateAnims (const PNode *node, StratAnim *curAnim, Object *objs);
static Uint32 IdleTicker = 0;
Options curOptions;

static Material	vmsMat=		  { 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA | MF_CLAMP_U|MF_CLAMP_V };
static Material	padMat=		  { 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA | MF_CLAMP_U|MF_CLAMP_V };

int CurEntry = 0, CurPlayer, StartPerson, TAST_Person, TournamentVMS, TournamentSlot;


static float OffPos, OnPos;

#define PAD_TIMEOUT (220) // just over three seconds at 60fps

#define PROFILE_NAME_LEN 8

extern Material fadeMat, opaqueNonTextured;
extern int suppressReset;

extern PNode *ArgoLogo;
extern Material segaMat, argoMat;
PNode *TempAnim;

PNode *mpMap[MAX_MP_MAPS];

static float MenuPri = -10.f;
char *GameParmName[] = { "FRAGLIMIT", "MPLIVES", "BTIME", "BTIME",
 						"FRAGLIMIT", "FTIME", "KTIME", "NOPARMS" };
static MapNum *GlobalMapNum;
static bool PostGameNewGame = FALSE;
TournamentGame CustomTournament;

static LightingValue TitleBGColour, TargetTitleBGColour;
static LightingValue TitleStripeColourBright, TargetTitleStripeColourBright;
static LightingValue TitleStripeColourDark, TargetTitleStripeColourDark;
static LightingValue TitleLineColourNear, TargetTitleLineColourNear;
static LightingValue TitleLineColourFar, TargetTitleLineColourFar;

static bool DebouncePadReset = FALSE;
extern const char *LangLookup (char *token);

bool GinsuAutoDemo = false;

#define RGB(r,g,b) (0xff000000|((r)<<16)|((g)<<8)|(b))
Uint32 TankPalette[] =
{
	RGB(228,73,42),
	RGB(230,230,60),
	RGB(100,201,37),
	RGB(80,127,206),
	RGB(148,77,157),
	RGB(237,147,25),
	RGB(62,62,62)
};

static STRAT AnimStrat;
static struct AnimInstance AnimStratInstance;
static Animation AnimStratAnimation;
static float BarSlidePos, BarTargetPos;
static void ResetPos (int i);
void MarkAsCompleted (int level, Mission *m);

#define PUSH_MATRIX() { Matrix32 pushedMatrix; memcpy (&pushedMatrix, psGetMatrix(), sizeof (Matrix32))
#define POP_MATRIX() memcpy (psGetMatrix(), &pushedMatrix, sizeof (Matrix32)); }

static void SetupAnim(PNode *TitlePage)
{
	AnimStrat.anim = &AnimStratInstance;
	AnimStrat.pnode = TitlePage;
	AnimStrat.obj = TitlePage->obj;
	AnimStrat.objId = TitlePage->objId;
	AnimStratInstance.next = NULL;
	AnimStratInstance.anim.anim = &AnimStratAnimation;
	AnimStratAnimation.typeData = (int*)0xffffffff;
	// Initialise the anim
	SetModelAnim (&AnimStrat, 0, 0);
	AnimStrat.anim->anim.flags = 0; // Stop it
}

bool Cheating = FALSE;

/*
 * Structures used to hold each player's state in the frontend
 */
typedef enum {
	FER_INIT, FER_DRAW, FER_RUN, FER_WAIT, FER_FINAL
} FEReason;
typedef bool FEHandler (FEReason reason, float top, float left, float offset, float width, 
						float height, float alpha,
						Uint32 padAll);

static void InitMapScreen(void);
static void RunMapScreen(void);

typedef struct
{
	Menu *rootMenu;
	Menu *otherMenu;
	int	CurEntry;
	float offset;
	float targetOffset;
	float lastOffset[MENU_BLUR_FRAMES];
	int   vms;
	Menu	*dynMenu;
	FEHandler	*handler;
	FEHandler	*nextHandler, *prevHandler;
	bool  waiting;
	int		padTimeout;
	int		profile;
	bool	inMPGame;
	int		curStats;
} FEState;

FEState feState[4];

// The profile being used by each player
GameSave	CurProfile[4];

// Integer handicaps
int			CurHandicap[4];

// Tournament scores
int			TournamentScore[4];

static struct {
	Material		*mat;
	KMVERTEXCONTEXT	*bC;
	Uint32			GBIX;
} FixUpList[] = {
	{ &vmsMat,		&sfxContext, 
#include "p:\game\texmaps\frontend\vms.h"
	},
	{ &padMat,		&sfxContext, 
#include "p:\game\texmaps\frontend\ccontrol.h"
	},
};

typedef struct {
	char	name[32];
	Sint16	vms;
	Sint16	profile;
} ProfileData;
static ProfileData ProfData[8*4];

// Colours
#define COL_EXPLAIN		COL_NORMAL		// Explanatory text
#define SCALE_EXPLAIN	0.75f			// Explanatory text scale
#define	COL_HEADING		0xff00ff00		// Heading colour
#define COL_SELECTED	0xff00ff00		// Selected text colour
#define COL_NORMAL		0xff229900		// Normal text colour
#define COL_GREYEDOUT	0xff304030		// Greyed out text
#define COL_BULLET		0xffff0000		// Bullet point colour
#define COL_INPUT		0xffcfffcf		// Text input colour
#define COL_TITLE_1		0x007fcfcf		// Title background 1
#define COL_TITLE_2		0x207fcfcf		// Title background 2
#define COL_BACKGROUND	0x00000028		// Background colour
#define COL_MP_BAR		0x40000000		// Multiplayer bar colour
// Lines and stripes
#define LINE_COLOUR		0xff003050
#define LINE_COLOUR_FAR	0xff001020
#define STRIPE_BRIGHT	0xff00ffff
#define STRIPE_DARK		0xff000030

extern void RecurseBodgeColours (Object *o, Uint32 colour);


int GetLivesRating (int level, int dam)
{
	switch (dam) {
	case 0:
		return 25;
	case 1:
		return 15;
	case 2:
		return 5;
	}
	return 0;
}
int GetGrading (int Score)
{
	if (Score >= 90) {
		return 0;
	} else if (Score >= 80) {
		return 1;
	} else if (Score >= 70) {
		return 2;
	} else if (Score >= 60) {
		return 3;
	} else if (Score >= 40) {
		return 4;
	} else {
		return 5;
	}
}
static GameType GetGameTypeFE(void)
{
	switch (mpSettings.playMode) {
	case TOURNAMENT:
		return CustomTournament.game[CurrentTournamentGame].gameType;
	case SKIRMISH:
		return mpSettings.game.skirmish.gameType;
	case TEAM:
		return mpSettings.game.team.gameType;
	}

}

static GameParms *GetFEGameParms(void)
{
	switch (mpSettings.playMode) {
	case TOURNAMENT:
		return &CustomTournament.game[CurrentTournamentGame].parms;
	case SKIRMISH:
		return &mpSettings.game.skirmish.parms;
	case TEAM:
		return &mpSettings.game.team.parms;
	}

}

#define WAIT_AMOUNT_FIRST	35
#define WAIT_AMOUNT_AFTER	5
static Uint32 FEPressedLastFrame[4];
static Uint32 FEWaitAmt[4];
static Uint32 FEHeld[4];
static Uint32 StoredPad[4];

const PDS_PERIPHERAL *FindLogicalController(int i)
{
	extern const PDS_PERIPHERAL *pad[4];
	return pad[i];
}
// Get a debounced, auto-repeated key press
Uint32 GetPadFE(int i)
{
	Uint32 pressed;
	Uint32 retVal;
	const PDS_PERIPHERAL *pad;
	
	if (i < 0) {
		i = (-i) - 1;
		pad = FindLogicalController(i);
	} else
		pad = FindPhysicalController(i);

	if (pad == NULL)
		return 0;

	// Get the buttons being held down
	pressed = pad->on;

	// Make the analogue appear as a digital input
	if (pad->x1 > 75.f) {
		pressed |= PDD_DGT_KR;
	} else if (pad->x1 < -75.f) {
		pressed |= PDD_DGT_KL;
	} else if (pad->y1 > 75.f) {
		pressed |= PDD_DGT_KD;
	} else if (pad->y1 < -75.f) {
		pressed |= PDD_DGT_KU;
	}

	// Find any changes
	retVal = (FEPressedLastFrame[i] ^ pressed) & pressed;
	FEPressedLastFrame[i] = pressed;

	// Any change in buttons?
	if (retVal || (pressed == 0)) {
		FEWaitAmt[i] = WAIT_AMOUNT_FIRST;
		FEHeld[i] = 0;

		if (retVal & (PDD_DGT_TA | PDD_DGT_ST))
			TAST_Person = i;

		StoredPad[i] = retVal;

		return retVal;
	} else {
		// No change - see if any button is being held down
		if (--FEWaitAmt[i] == 0) {
			FEWaitAmt[i] = WAIT_AMOUNT_AFTER;
			FEHeld[i]++;
			StoredPad[i] = pressed;
			return pressed;
		}
	}
	StoredPad[i] = 0;
	return 0;
}


Uint32 GetPadAll(void)
{
	return GetPadFE(0) | GetPadFE(1) | GetPadFE(2) | GetPadFE(3);
}

int GetLevLives (int level)
{
	switch (level) {
	default:
		return 2;
		// XXXXXXXXXXXXXX Add other levels here
	}
}

void MarkAsCompleted (int level, Mission *m)
{
	int Score;
	int prevCompleted = m->completed;
	float bTime, gTime;
	bool better = false;
	m->completed = TRUE;

	// Paranoid madness, can't complete the credits level
	if (level == 13)
		return;

	// Convert BossTime and GameTime to seconds
	bTime = ((float)BossTime / FRAMES_PER_SECOND);
	gTime = ((float)GameTime / FRAMES_PER_SECOND);

	if (level >= 6) {
		// Challenge level: key off of time (mostly)
		if (prevCompleted) {
			// LEVEL DEPENENDENCY
			if (level == LEV_DEFENSIVE) // special case: key off of score
				better = score > m->bestScore;
			else
				better = gTime < m->bestTime;
		}
	} else {
		Score = GetTimeRating (level, gTime) + GetScoreRating (level, score) + GetLivesRating (level, GetLevLives(level) - PlayerLives) + GetBossTimeRating (level, bTime);
		if (prevCompleted)
			better = (Score > (GetBossTimeRating(level, m->bestBossTime) + GetTimeRating(level, m->bestTime) + GetScoreRating(level, m->bestScore) + GetLivesRating(level, m->livesLost)));
	}
	if (!prevCompleted || better) {
		m->bestBossTime = bTime;
		m->bestScore = score;
		m->livesLost = (GetLevLives(level) - PlayerLives);
		m->bestTime = gTime;
		m->grading = GetGrading(Score);
	}
}


#define LOGO_TIME_OUT	(6*60)
#define MIN_LOGO_TIME	(2*60)
static void RecurseSub(Object *obj)
{
	int i;
	if (obj->model) {
		for (i = 0; i < obj->model->nMats;++i)
			obj->model->material[i].pasted.transparency *= 0.95f;
	}
	for (i = 0; i < obj->no_child; ++i)
		RecurseSub(&obj->child[i]);
}
static int LogoTimeOut, timer, segaLogo, logoFadeCounter, logoFadeAdd;
static float argoExtraSize = 0, argoSpeed;
static bool FirstTime = true;
void InitLogo(void)
{
	int i, nOK, nVMUs;

	// Make sure all pads are responding
	for (i = 0; i < 4; ++i)
		PadMapTable[i] = i;
	timer = -30;
	LogoTimeOut = 0;
	SetupAnim(ArgoLogo);
	SetModelAnim (&AnimStrat, 0, 0);
	CurrentStratIntensity = 0.75f;
	rSetModelContext();
	sunDir.x = 1.f;
	sunDir.y = -1.f;
	sunDir.z = 1.f;
	for (i = 0; i < 300; ++i)
		RecurseSub(ArgoLogo->obj);
	segaLogo = TRUE;
	logoFadeCounter = 0;
	logoFadeAdd = 1;
	argoExtraSize = 0;
	argoSpeed = PAL ? 0.25f : 0.2f;
}
bool RunLogo(void)
{
	int i, nOK, nVMUs;
	// Scan and look for a VMU
	nVMUs = nOK = 0;
	for (i = 0; i < 8; ++i) {
		if (BupGetInfo(i)->Colour) {
			nVMUs++;
			if (BupIsEnoughSpace(i))
				nOK++;
		}
	}
	if (PadReset) {
		sbExitSystem();
		while (1);
	}
	if (!FirstTime) {
		if (GetPadAll() & PDD_DGT_ST)
			return true;
	}
	if (segaLogo) {
		if (logoFadeCounter < (FRAMES_PER_SECOND*2)) {
			float fadeAmt = (float)logoFadeCounter / (FRAMES_PER_SECOND*2);
			Uint32 colour;
			fadeAmt = RANGE(0, fadeAmt, 1);
			colour = ((int)((1-fadeAmt) * 255.f) << 24);
			rSetMaterial (&fadeMat);
			kmxxGetCurrentPtr (&vertBuf);
			kmxxStartVertexStrip (&vertBuf);
			kmxxSetVertex_0 (0xe0000000, 0, 0, 100, colour);
			kmxxSetVertex_0 (0xe0000000, 0, PHYS_SCR_Y, 100, colour);
			kmxxSetVertex_0 (0xe0000000, PHYS_SCR_X, 0, 100, colour);
			kmxxSetVertex_0 (0xf0000000, PHYS_SCR_X, PHYS_SCR_Y, 100, colour);
			kmxxReleaseCurrentPtr (&vertBuf);
			rSetBackgroundColour (0xffffff);
		}
		logoFadeCounter += logoFadeAdd;

		if (logoFadeAdd == 1 && logoFadeCounter >= (FRAMES_PER_SECOND*3.5)) {
			logoFadeAdd = -1;
		} else if (logoFadeCounter <= 0) {
			logoFadeAdd = 1;
			segaLogo = false;
		}

#define LOGO_SIZE 128
#define LOGO_TOP ((PHYS_SCR_Y - LOGO_SIZE)*0.5f)
#define LOGO_BOT ((PHYS_SCR_Y + LOGO_SIZE)*0.5f)
#define LOGO_LEF ((PHYS_SCR_X - LOGO_SIZE)*0.5f)
#define LOGO_RIG ((PHYS_SCR_X + LOGO_SIZE)*0.5f)
		rSetMaterial (&segaMat);
		kmxxGetCurrentPtr (&vertBuf);
		kmxxStartVertexStrip (&vertBuf);
		kmxxSetVertex_3 (0xe0000000, LOGO_LEF, LOGO_TOP, 0.1f, 0, 0, 0xffffffff, 0);
		kmxxSetVertex_3 (0xe0000000, LOGO_LEF, LOGO_BOT, 0.1f, 0, 1, 0xffffffff, 0);
		kmxxSetVertex_3 (0xe0000000, LOGO_RIG, LOGO_TOP, 0.1f, 1, 0, 0xffffffff, 0);
		kmxxSetVertex_3 (0xf0000000, LOGO_RIG, LOGO_BOT, 0.1f, 1, 1, 0xffffffff, 0);
		kmxxReleaseCurrentPtr (&vertBuf);
		return false;
	} else {
		if (logoFadeCounter < (FRAMES_PER_SECOND*2)) {
			float fadeAmt = (float)logoFadeCounter / (FRAMES_PER_SECOND*2);
			Uint32 colour;
			fadeAmt = RANGE(0, fadeAmt, 1);
			colour = ((int)((1-fadeAmt) * 255.f) << 24);
			rSetMaterial (&fadeMat);
			kmxxGetCurrentPtr (&vertBuf);
			kmxxStartVertexStrip (&vertBuf);
			kmxxSetVertex_0 (0xe0000000, 0, 0, 100, colour);
			kmxxSetVertex_0 (0xe0000000, 0, PHYS_SCR_Y, 100, colour);
			kmxxSetVertex_0 (0xe0000000, PHYS_SCR_X, 0, 100, colour);
			kmxxSetVertex_0 (0xf0000000, PHYS_SCR_X, PHYS_SCR_Y, 100, colour);
			kmxxReleaseCurrentPtr (&vertBuf);
			rSetBackgroundColour (-1);
		}
		logoFadeCounter += logoFadeAdd;

		if (logoFadeAdd == 1 && logoFadeCounter >= (FRAMES_PER_SECOND*3.5)) {
			logoFadeAdd = -1;
		} else if (logoFadeCounter <= -40) {
			FirstTime = false;
			return true;
		}
#undef LOGO_SIZE
#undef LOGO_TOP
#undef LOGO_BOT
#undef LOGO_LEF
#undef LOGO_RIG
#define LOGO_SIZE (384+argoExtraSize)
#define LOGO_TOP ((PHYS_SCR_Y - LOGO_SIZE/2)*0.5f)
#define LOGO_BOT ((PHYS_SCR_Y + LOGO_SIZE/2)*0.5f)
#define LOGO_LEF ((PHYS_SCR_X - LOGO_SIZE)*0.5f)
#define LOGO_RIG ((PHYS_SCR_X + LOGO_SIZE)*0.5f)
		argoExtraSize += argoSpeed;
		argoSpeed *= 0.995f;
		rSetMaterial (&argoMat);
		kmxxGetCurrentPtr (&vertBuf);
		kmxxStartVertexStrip (&vertBuf);
		kmxxSetVertex_3 (0xe0000000, LOGO_LEF, LOGO_TOP, 0.1f, 0, 0, 0xffffffff, 0);
		kmxxSetVertex_3 (0xe0000000, LOGO_LEF, LOGO_BOT, 0.1f, 0, 1, 0xffffffff, 0);
		kmxxSetVertex_3 (0xe0000000, LOGO_RIG, LOGO_TOP, 0.1f, 1, 0, 0xffffffff, 0);
		kmxxSetVertex_3 (0xf0000000, LOGO_RIG, LOGO_BOT, 0.1f, 1, 1, 0xffffffff, 0);
		kmxxReleaseCurrentPtr (&vertBuf);
		return false;
/*		rSetBackgroundColour (0);
		mUnit (&mCurMatrix);
		mTranslate (&mCurMatrix, 0, 0.f, 1.5f);
		rUpdateAnims (ArgoLogo, &AnimStrat.anim->anim, ArgoLogo->obj);
		rDrawObject (ArgoLogo->obj);
		
		LogoTimeOut++;
		if (LogoTimeOut > (PAL ? (3.75*50) : (3.75*60))) {
			extern bool SoundOverride;
			psSetColour (0xffffff);
			psSetAlpha (1.f);
			psSetPriority(0);
			mUnit32 (psGetMatrix());
			SoundOverride	 = true;
			psDrawCentredFE ("Argonaut Software Ltd", 350.f, timer);
			SoundOverride	 = false;
			timer += 2;
			RecurseSub(ArgoLogo->obj);
		}
		if (!ChangingState() && (((GetPadAll() & ANY_BUTTON) && LogoTimeOut > MIN_LOGO_TIME) || (LogoTimeOut > LOGO_TIME_OUT)))
			return true;
		else
			return false;*/
	}
}

void LockCurrentProfile(void)
{
	if (GameVMS>=0 && GameProfile>=0 &&
		!BupIsProfileLocked (GameVMS, GameProfile))
		BupLockProfile (GameVMS, GameProfile);
}
void UnlockCurrentProfile(void)
{
	if (GameVMS>=0 && GameProfile>=0 &&
		BupIsProfileLocked (GameVMS, GameProfile))
		BupUnlockProfile (GameVMS, GameProfile);
}

// Initialise an options block to sensible defaults
void ResetGame (GameSave *g)
{
	memset (g, 0, sizeof (*g));
	strcpy (g->name, EMPTY_PROFILE);
	g->difficulty = 1;
	g->controller = 0;
	g->multiplayerStats.colour = rand() % asize (TankPalette);
	//g->missionData is zero'd above
	//g->multiplayerStats likewise
	g->musicVolume = sfxGetCDVolume();
	g->soundVolume = sfxGetFXVolume();
}

void InitOptions (Options *o)
{
	int i, j;
	memset (o, 0, sizeof (Options));

	memcpy (o->argo, "Argonaut", 8);
	o->version = OPTIONS_VERSION;

	for (i = 0; i < MAX_TOURNAMENTS; ++i) {
		for (j = 0; j < MAX_TOURNAMENT_GAMES; ++j) {
			strcpy (o->tournament[i].name, EMPTY_PROFILE);
			o->tournament[i].nGames = MIN_TOURNAMENT_GAMES;
			o->tournament[i].tType = CUSTOM;
			ResetGameParms (&o->tournament[i].game[j].parms);
			o->tournament[i].game[0].gameType = TAG;
			o->tournament[i].game[1].gameType = KING_OF_THE_HILL;
			o->tournament[i].game[2].gameType = DEATHMATCH;
		}
	}

	for (i = 0; i < 4; ++i) {
		ResetGame (&o->game[i]);
	}
}
