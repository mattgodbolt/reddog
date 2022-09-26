#define NO_PROTOTYPES
#include "Menus.h"
#include "MiscScreens.h"

// Handy map number address routine
MapNum *GetPTMap()
{
	switch (mpSettings.playMode) {
	default:
	case SKIRMISH:
		return &mpSettings.game.skirmish.map;
	case TEAM:
		return &mpSettings.game.team.map;
	case TOURNAMENT:
		return &CustomTournament.game[CurrentTournamentGame].map;
	}
}

//////////////////////////////////////////////////////////////
// A profile selection screen, selecting from a single VMU
//////////////////////////////////////////////////////////////
class EnterNameMP : public EnterName
{
private:
	int vmu, prof;
public:
	EnterNameMP() : EnterName() 
	{
		vmu = prof = -2;
	}
	EnterNameMP(int vmu, int prof) : EnterName() 
	{
		this->vmu = vmu;
		this->prof = prof;
	}
	virtual ~EnterNameMP()
	{
		GetRoot()->UnlockProfile();
	}
	virtual void Initialise()
	{
		if (vmu == -2) {
			vmu = GetRoot()->GetCurVMUNum();
			prof = GetRoot()->GetCurProfileNum();
		}
		EnterName::Initialise();
		if (vmu != -1)
			GetRoot()->LockProfile (vmu, prof);
	}
	virtual const char *GetTitle() const 
	{
		return LangLookup("ENTERNAME");
	}
	virtual void Select(const char *name) 
	{
		// Check for duplicate names on a single VMU
		for (int i = 0; i < 4; ++i) {
			if (i == GetRoot()->GetCurProfileNum() || GetRoot()->GetCurVMU()->GetProfileName(i) == NULL)
				continue;
			if (strcmp(GetRoot()->GetCurVMU()->GetProfileName(i), name) == 0) {
				break;
			}
		}
		if (i != 4) {
			FE_NACK();
			return;
		}
		ResetGame (GetRoot()->GetCurProfile());
		CurProfile[GetPlayer()].multiplayerStats.colour = GetPlayer();
		strcpy (GetRoot()->GetCurProfile()->name, name);
		Change (ScreenID (mpWaitScreen));
		GetRoot()->WriteProfile();
	}
};

Screen *enterNameMP(void *parm) 
{ 
	int i = (int)parm;
	if (i == 0xffffffff)
		return new EnterNameMP;
	else
		return new EnterNameMP(i>>8, i & 0xff);
}

//////////////////////////////////////////////////////////////
// The skirmish handler
//////////////////////////////////////////////////////////////
#define LINE_EXTRA 32
void DrawLineFE (float x0, float y0, float x1, float y1, float len, float dist, float alpha)
{
	float dx, dy;
	Colour green = { (int(alpha*255)<<24) | COL_TEXT };
	if (dist < 0)
		return;

	if (dist < len && (currentFrame & 1))
		FE_Sound();

	if (GameDifficulty)
		green.colour = (int(alpha*255)<<24) | 0xff2020;

	dx = (x1 - x0) / len;
	dy = (y1 - y0) / len;

	if (dist > len + LINE_EXTRA) {
		rLine2D (x0, y0, x1, y1, 1.f, green, green);
	} else {
		float len1 = dist - LINE_EXTRA;
		if (len1 < 0)
			len1 = 0;
		if (dist > LINE_EXTRA) {
			rLine2D (x0, y0, x0 + dx * len1, y0 + dy * len1, 1.f, green, green);
		}
		Colour lerped;
		float visAmt;
		visAmt = 1.f - RANGE(0, dist - len, 1) / float(LINE_EXTRA);
		dist = MIN(dist, len);

		lerped.colour = ColLerp (0, COL_TEXT, visAmt) | (int(alpha*255)<<24);
		rLine2D (x0 + dx * len1, y0 + dy * len1, x0 + dx * dist, y0 + dy * dist, 1.f, green, lerped);
	}
}

// Useful helper function
#define MUNGE(a) a=GameType((int)a+amount); if (a<0) a = GameType((int)a + ((int)maxGameType + 1)); else if (a>maxGameType) a=GameType((int)a-((int)maxGameType + 1))
static void ChangeCurrentGameType (int amount)
{
	int maxGameType = (mpSettings.playMode == TOURNAMENT) ? int(KING_OF_THE_HILL)+1 : int(KING_OF_THE_HILL);

LetsTryThatAgainShallWe:
	switch (mpSettings.playMode) {
	case TOURNAMENT:
		MUNGE(CustomTournament.game[CurrentTournamentGame].gameType);
	case SKIRMISH:
		MUNGE(mpSettings.game.skirmish.gameType);
	case TEAM:
		MUNGE(mpSettings.game.team.gameType);
	}
	if (GetCurrentGameType() == KING_OF_THE_HILL && !(theVMUManager->GetSpecials() & SPECIAL_KINGOFTHEHILL))
		goto LetsTryThatAgainShallWe;
	// Adjust the map if necessary
	if (GetCurrentGameType() != RANDOM) {
		MapNum *ptMap = GetPTMap();
		while (!(MPLevOK[*ptMap] & (1<<GetCurrentGameType()))) {
			(*ptMap)++;
			if (*ptMap >= NUM_MP_LEVELS)
				*ptMap = 0;
		}
	}
}

// Game type menu entry
#define GT_LEN_1	(224 - len)
#define GT_LEN_2	(wordLen+32)

void GameTypeEntry::SetText (char *text)
{
	len = psStrWidth(text) + 16.f;
	MenuEntry::SetText(text);
}
float GameTypeEntry::Draw(float selAmt, float alpha, float nChars)
{
	float retVal = MenuEntry::Draw(selAmt, alpha, nChars);
	float wordLen = psStrWidth(GetText()) * GetScale();
	float LengthOne = GT_LEN_1;
	
	if (LengthOne < 16)
		LengthOne = 16;
	
	nChars -= retVal;
	if (nChars > 0) {
		Colour c = { 0xffffffff };
		Point2 p[7];
		p[0].x = len; p[0].y = 16;
		p[1].x = len + LengthOne; p[1].y = 16;
		p[2].x = len + LengthOne; p[2].y = -2;
		p[3].x = len + LengthOne; p[3].y = 34;
		p[4].x = len + LengthOne + GT_LEN_2; p[4].y = -2;
		p[5].x = len + LengthOne + GT_LEN_2; p[5].y = 34;
		p[6].x = len + LengthOne + GT_LEN_2; p[6].y = 16;
		Transform (p, 7);
		DrawLineFE (p[0].x, p[0].y, p[1].x, p[1].y, LengthOne, nChars*24, alpha);
		nChars -= LengthOne/24;
		DrawLineFE (p[1].x, p[1].y, p[2].x, p[2].y, 18, nChars*24, alpha);
		DrawLineFE (p[1].x, p[1].y, p[3].x, p[3].y, 18, nChars*24, alpha);
		nChars -= 1;
		mPreScale32 (psGetMatrix(), GetScale(), GetScale());
		psDrawStringFE (GetText(), (len + LengthOne + 16)/GetScale(), 16 - 16*GetScale(), nChars);
		mPreScale32 (psGetMatrix(), 1.f / GetScale(), 1.f / GetScale());
		DrawLineFE (p[2].x, p[2].y, p[4].x, p[4].y, GT_LEN_2, nChars*24, alpha);
		DrawLineFE (p[3].x, p[3].y, p[5].x, p[5].y, GT_LEN_2, nChars*24, alpha);
		nChars -= GT_LEN_2/24;
		DrawLineFE (p[4].x, p[4].y, p[6].x, p[6].y, 18, nChars*24, alpha);
		DrawLineFE (p[5].x, p[5].y, p[6].x, p[6].y, 18, nChars*24, alpha);
	}
	return retVal + (LengthOne/24) + 1 + 1 + GT_LEN_2/24;
}
void GameTypeEntry::Left()
{
	ChangeCurrentGameType(-1);
	FE_MoveSound();
}
void GameTypeEntry::Right()
{
	ChangeCurrentGameType(1);
	FE_MoveSound();
}
const char *GameTypeEntry::GetText() 
{
#if GINSU
	static char buf[64];
	if (GetCurrentGameType() != DEATHMATCH &&
		GetCurrentGameType() != TAG) {
		sprintf (buf, "%s (n/a)", GameTypeName[GetCurrentGameType()]);
		return buf;
	}
#endif
	return GameTypeName[GetCurrentGameType()];
}
float GameTypeEntry::GetScale()
{
	return 1.f;
}

MenuEntry *gameTypeMaker(const char *text, ScreenID &sub)
{
	return new GameTypeEntry (text, sub);
}

// Choose map as a menu handler
#define SPR_SIZE 196.f

#define CHEAT_MAP_UNLOCK 3 // Drive or Die Expert

class ChooseMapEntry : public MenuEntry
{
private:
	float len;
	MapNum *ptMap, origMap;
	int cheatCheck;
public:
	ChooseMapEntry(const char *text, ScreenID &sub) : MenuEntry(text, sub) 
	{
		ptMap = GetPTMap();
#if GINSU
		*ptMap = 10; // Ruined!
#endif
		len = psStrWidth(text) + 16.f;
		mapOK = true;
		origMap = *ptMap;
		cheatCheck = 0;
	}
	// Destructor ensures map is 'ok'
	virtual ~ChooseMapEntry();
	virtual float Draw(float selAmt, float alpha, float nChars)
	{
		float retVal = MenuEntry::Draw(selAmt, alpha, nChars);
		float wordLen = psStrWidth(GameTypeName[GetCurrentGameType()]);
		float adj = 0;
		float w[4];
		psGetWindow (w);

		CheatCheck();

		ptMap = GetPTMap();

		nChars -= retVal;
		Colour c = { 0xffffffff };

		Point2 p[5];

		p[0].x = len; p[0].y = 16;
		p[1].x = currentCamera->screenX - 48.f - SPR_SIZE/2; p[1].y = 16;
		p[2].x = currentCamera->screenX - 48.f - SPR_SIZE/2; p[2].y = 80;
		p[3].x = currentCamera->screenX - 48.f - SPR_SIZE; p[3].y = 80;
		p[4].x = currentCamera->screenX - 48.f; p[4].y = 80;
		Transform (p, 5);
		DrawLineFE (p[0].x, p[0].y, p[1].x, p[1].y, p[1].x - p[0].x, nChars*16 - adj, alpha);
		adj += p[1].x - p[0].x;
		DrawLineFE (p[1].x, p[1].y, p[2].x, p[2].y, p[2].y - p[1].y, nChars*16 - adj, alpha);
		adj += p[2].y - p[1].y;
		DrawFESprite (GetLevelMat(*ptMap), p[3].x-w[1], p[3].y-w[0], SPR_SIZE, SPR_SIZE, (nChars*16 - adj)/2, 1, 0, alpha);
		if (!MapIsSelectable())
			DrawFESprite (&intelligence, p[3].x-w[1], p[3].y-w[0], SPR_SIZE, SPR_SIZE, (nChars*16 - adj)/2, 1, 0, alpha);
		adj += SPR_SIZE/2.f;

		return retVal + adj/16;
	}
	bool MapIsValid()
	{
		ptMap = GetPTMap();
		if (*ptMap == NUM_MP_LEVELS)
			return true;
		if ((1<<GetCurrentGameType()) & MPLevOK[*ptMap])
			return true;
		else
			return false;
	}
	bool MapIsSelectable();
	void Left()
	{
		ptMap = GetPTMap();
		do {
			*ptMap = *ptMap - 1;
			if (*ptMap == -1)
				*ptMap = NUM_MP_LEVELS;
		} while (!MapIsValid());
		FE_MoveSound();
		mapOK = MapIsSelectable();
	}
	void Right()
	{
		ptMap = GetPTMap();
		do {
			*ptMap = *ptMap + 1;
			if (*ptMap == (NUM_MP_LEVELS+1))
				*ptMap = 0;
		} while (!MapIsValid());
		FE_MoveSound();
		mapOK = MapIsSelectable();
	}
	void CheatCheck();
};

ChooseMapEntry::~ChooseMapEntry()
{
	ptMap = GetPTMap();
	if (!MapIsSelectable() || !MapIsValid())
		*ptMap = origMap;
}

bool ChooseMapEntry::MapIsSelectable()
{
	ptMap = GetPTMap();
#if GINSU
	if (*ptMap != 10) // Ruined only
		return false;
	else
		return true;
#else
	if (*ptMap == origMap)
		return true;
	if (MPUnlockCode[*ptMap]) {
		if (theVMUManager->IsUnlocked (*ptMap))
			return true;
		else
			return false;
	} else
		return true;
#endif
}
void ChooseMapEntry::CheatCheck()
{
	ptMap = GetPTMap();
	if (*ptMap == CHEAT_MAP_UNLOCK && !theVMUManager->SpecialUnlocked) {
		Uint32 allButtons = PadPress[0] | PadPress[1] | PadPress[2] | PadPress[3];
		if (allButtons & (PADSHOULDERL | PADSHOULDERR)) {
			cheatCheck++;
		} else
			cheatCheck = 0;

		if (cheatCheck >= (3*FRAMES_PER_SECOND)) {
			FE_ACK();
			theVMUManager->SpecialUnlocked = true;
		}
	} else if (*ptMap == CHEAT_MAP_UNLOCK)
		cheatCheck = 0;
	mapOK = MapIsSelectable();
}

MenuEntry *chooseMapMaker(const char *text, ScreenID &sub)
{
	return new ChooseMapEntry (text, sub);
}


// Helpful funkshun
static void AdjustParms(int dir)
{
	GameParms *g = GetCurrentGameParms();
	GameParms c = *g;
	switch (GetCurrentGameType())
	{
	case PREDATOR:
		g->predatorParms.fragLimit = RANGE(1, (g->predatorParms.fragLimit+dir), 30);
		break;
	case DEATHMATCH:
		g->dmParms.fragLimit = RANGE(1, (g->dmParms.fragLimit+dir), 99);
		break;
	case KNOCKOUT:
		g->koParms.startLives= RANGE(1, (g->koParms.startLives+dir), 99);
		break;
	case TAG:
		g->bombParms.bombTimer=RANGE(30, (g->bombParms.bombTimer+(dir*5)), MAX_MP_GAME_TIME);
		break;
	case SUICIDE_TAG:
		g->suicideParms.bombTimer=RANGE(30, (g->suicideParms.bombTimer+(dir*5)), MAX_MP_GAME_TIME);
		break;
	case KEEP_THE_FLAG:
		g->ktfParms.winTime=RANGE(5, (g->ktfParms.winTime+(dir*5)), MAX_MP_GAME_TIME);
		break;
	case KING_OF_THE_HILL:
		g->kothParms.winTime=RANGE(5, (g->kothParms.winTime+(dir*5)), MAX_MP_GAME_TIME);
		break;
	}
	if (memcmp (&c, g, sizeof(c)) != 0) {
		FE_MoveSound();
	} else {
		FE_NACK();
	}
}

// Multiplayer parameter maker
class ParmEntry : public GameTypeEntry
{
private:
	char		buf[64];
public:
	ParmEntry(const char *text, ScreenID &sub) : GameTypeEntry(text, sub) 
	{
	}
	virtual float Draw(float selAmt, float alpha, float nChars)
	{
		// pure laziness:
		SetText ((char *)LangLookup(GameParmName[GetCurrentGameType()]));
		return GameTypeEntry::Draw(selAmt, alpha, nChars);
	}
	virtual void Left()
	{
		AdjustParms(-1);
	}
	virtual void Right()
	{
		AdjustParms(1);
	}
	virtual const char *GetText()
	{
		int mins, secs;
		switch (GetCurrentGameType()) {
		case RANDOM:
			strcpy (buf, LangLookup ("NOPARMS"));
			break;
		case PREDATOR:
			sprintf (buf, "%d %s", GetCurrentGameParms()->predatorParms.fragLimit, LangLookup ((GetCurrentGameParms()->predatorParms.fragLimit==1) ? "KILL" : "KILLS"));
			break;
		case DEATHMATCH:
			sprintf (buf, "%d %s", GetCurrentGameParms()->dmParms.fragLimit, LangLookup ((GetCurrentGameParms()->predatorParms.fragLimit==1) ? "KILL" : "KILLS"));
			break;
		case KNOCKOUT:
			sprintf (buf, "%d %s", GetCurrentGameParms()->koParms.startLives, LangLookup ((GetCurrentGameParms()->koParms.startLives==1) ? "LIFE" : "LIVES"));
			break;
		case TAG:
			mins = GetCurrentGameParms()->bombParms.bombTimer / 60;
			secs = GetCurrentGameParms()->bombParms.bombTimer % 60;
			goto Laziness;
		case SUICIDE_TAG:
			mins = GetCurrentGameParms()->suicideParms.bombTimer / 60;
			secs = GetCurrentGameParms()->suicideParms.bombTimer % 60;
Laziness:
			if (mins) {
				if (mins == 1)
					sprintf (buf, LangLookup("MINSECS"), mins, secs);
				else
					sprintf (buf, LangLookup("MINSSECS"), mins, secs);
			} else
				sprintf (buf, LangLookup ("SECS"), secs);
			break;
		case KEEP_THE_FLAG:
			mins = GetCurrentGameParms()->ktfParms.winTime / 60;
			secs = GetCurrentGameParms()->ktfParms.winTime % 60;
			goto Laziness;
		case KING_OF_THE_HILL:
			mins = GetCurrentGameParms()->kothParms.winTime / 60;
			secs = GetCurrentGameParms()->kothParms.winTime % 60;
			goto Laziness;
		}
		return buf;
	}
#if GINSU
	virtual bool IsSelectable() { return false; }
#endif
};

MenuEntry *parmMaker(const char *text, ScreenID &sub)
{
	return new ParmEntry (text, sub);
}


// Friendly fire action
class FFEntry : public GameTypeEntry
{
public:
	FFEntry(const char *text, ScreenID &sub) : GameTypeEntry(text, sub) 
	{
	}
	virtual ScreenID *Selected()
	{
		Right();
		return NULL;
	}
	virtual void Left()
	{
		Right();
	}
	virtual void Right()
	{
		FE_MoveSound();
		mpSettings.game.team.friendlyFire = !mpSettings.game.team.friendlyFire;
	}
	virtual const char *GetText()
	{
		return mpSettings.game.team.friendlyFire ? LangLookup("YES") : LangLookup("NO");
	}
};
MenuEntry *ffMaker(const char *text, ScreenID &sub)
{
	return new FFEntry (text, sub);
}

////////////////////////////////////////////////////////
// The skirmish screen proper
////////////////////////////////////////////////////////
class SkirmishScreen : public Menu
{
private:
public:
	SkirmishScreen(MenuTemplate *templ = &newskirmishMenu) : Menu(templ)
	{
	}
	virtual void Initialise() {
		Menu::Initialise();
		mpSettings.playMode = SKIRMISH;
	}
	Uint32 BackgroundColour() { return 0x204020; }
};

Screen *skirmishScreen() { return new SkirmishScreen; }

////////////////////////////////////////////////////////
// The team screen
////////////////////////////////////////////////////////
class TeamScreen : public SkirmishScreen
{
public:
	TeamScreen() : SkirmishScreen(&newteamMenu)
	{
	}
	virtual void Initialise() {
		Menu::Initialise();
		mpSettings.playMode = TEAM;
	}
};
Screen *teamScreen() { return new TeamScreen; }

////////////////////////////////////////////////////////
// The tournament screen
////////////////////////////////////////////////////////
char *TGameName(TournamentGame *t, int j)
{
	static char buf[64];
	sprintf (buf, "%s %s %s '%s'", 
		t->team[j] ? LangLookup("TEAM") : "",
		GameTypeName[t->game[j].gameType],
		LangLookup("TYPEONMAP"),
		MPMapName[t->game[j].map]
		);
	return buf;
}

class TournScreen : public SkirmishScreen
{
private:
	int timer;
public:
	TournScreen() : SkirmishScreen(&newtournMenu)
	{
		SetUpForTournament(&mpSettings.game.tournament);
	}
	virtual void Initialise()
	{
		Menu::Initialise();
		mpSettings.playMode = TOURNAMENT;
		SetUpForTournament(&mpSettings.game.tournament);
		timer = 0;
	}
	virtual void Draw() const
	{
		Matrix32 mat;
		int adj = 0;
		memcpy (&mat, psGetMatrix(), sizeof (mat));
		SkirmishScreen::Draw();
		memcpy (psGetMatrix(), &mat, sizeof (mat));

		float alpha = GetFadeAmount();
		mPreTranslate32 (psGetMatrix(), PHYS_SCR_X/2, PHYS_SCR_Y - 128.f);
		mPreScale32 (psGetMatrix(), 0.6f, 0.6f);
		mPreTranslate32 (psGetMatrix(), -PHYS_SCR_X/2 , - 35 * mpSettings.game.tournament.nGames);
		for (int line = 0; line < mpSettings.game.tournament.nGames ; ++line) {
			char buf[64];
			sprintf (buf, "%d - %s", line+1, TGameName (&mpSettings.game.tournament, line));
			char *name = buf;
			psSetAlpha (alpha);
			psSetPriority(0);
			psSetColour (COL_TEXT);
			psDrawCentredFE (name, 0, timer - adj);
			adj += strlen(name);
			mPreTranslate32 (psGetMatrix(), 0, 35);
		}
		memcpy (psGetMatrix(), &mat, sizeof (mat));
	}
	void ProcessInput (Uint32 press)
	{
		SkirmishScreen::ProcessInput(press);
		timer += 2;
	}
};
Screen *tournScreen() { return new TournScreen; }

//////////////////////////////////////////////////////////
// The bonus menu itself
//////////////////////////////////////////////////////////
class BonusOnOff : public GameTypeEntry
{
private:
	Uint32 bit;
public:
	BonusOnOff(const char *text, ScreenID &sub) : GameTypeEntry(text, ScreenID()) 
	{
		bit = (Uint32) sub.GetUser();
	}
	virtual ScreenID *Selected()
	{
		Right();
		return NULL;
	}
	virtual void Left()
	{
		Right();
	}
	virtual void Right()
	{
		FE_MoveSound();
		CurProfile[0].activeSpecials ^= bit;
	}
	virtual const char *GetText()
	{
		return (CurProfile[0].activeSpecials & bit) ? LangLookup("ON") : LangLookup("OFF");
	}
};
MenuEntry *bonusOnOffMaker(const char *text, ScreenID &sub)
{
	return new BonusOnOff (text, sub);
}

class InitDrone : public GameTypeEntry
{
private:
	int curWeapon;
	static const char *weapNames[7];
public:
	InitDrone(const char *text, ScreenID &sub) : GameTypeEntry(text, ScreenID()) 
	{
		curWeapon = CurProfile[0].cheatWeapon - 1;
	}
	void SortItOut();
	virtual void Left()
	{
		curWeapon--;
		if (curWeapon == -2) {
			curWeapon = 6;
			FE_MoveSound();
		}
		if (curWeapon >= 0) {
			if (!(CurProfile[0].unlockedSpecials & (1<<(curWeapon + SPECIAL_FIRST_WEAP_BIT))))
				Left();
			else
				FE_MoveSound();
		}
		SortItOut();
	}
	virtual void Right()
	{
		curWeapon++;
		if (curWeapon == 7) {
			curWeapon = -1;
			FE_MoveSound();
		} else {
			if (!(CurProfile[0].unlockedSpecials & (1<<(curWeapon + SPECIAL_FIRST_WEAP_BIT))))
				Right();
			else
				FE_MoveSound();
		}
		SortItOut();
	}
	virtual const char *GetText()
	{
		if (curWeapon == -1)
			return "`NONE";
		else return weapNames[curWeapon];
	}
};
const char *InitDrone::weapNames[] =
{
	"Shell",
	"Charge",
	"Homing",
	"Lockon",
	"Swarm",
	"UltraLaser",
	"Vulcan cannon" //xxxnonfinal names
};
MenuEntry *initDroneMaker(const char *text, ScreenID &sub)
{
	return new InitDrone (text, sub);
}


void InitDrone::SortItOut()
{
	CurProfile[0].cheatWeapon = curWeapon + 1; //(CurProfile[0].activeSpecials &~ SPECIAL_WEAP_ALL);
}


class StereoSetter : public GameTypeEntry
{
public:
	StereoSetter(const char *text, ScreenID &sub) : GameTypeEntry(text, ScreenID()) 
	{
	}
	virtual void Left()
	{
		Right();
	}
	virtual void Right()
	{
		sfxSetStereo(!sfxIsStereo());
		FE_MoveSound();
	}
	virtual const char *GetText()
	{
		return (sfxIsStereo() ? "`STEREO" : "`MONO");
	}
};
MenuEntry *stereoMaker(const char *text, ScreenID &sub)
{
	return new StereoSetter (text, sub);
}
extern bool vibrationDisabled;
class Vibrator : public GameTypeEntry
{
public:
	Vibrator(const char *text, ScreenID &sub) : GameTypeEntry(text, ScreenID()) 
	{
	}
	virtual void Left()
	{
		Right();
	}
	virtual void Right()
	{
		vibrationDisabled = !vibrationDisabled;
		FE_MoveSound();
	}
	virtual const char *GetText()
	{
		return (vibrationDisabled ? "`OFF" : "`ON");
	}
};
MenuEntry *vibMaker(const char *text, ScreenID &sub)
{
	return new Vibrator (text, sub);
}
