// Huzzah - single player front end
#include "Menus.h"
#include "MiscScreens.h"
#include "LevelDepend.h"

int GameVMS = -1, GameProfile = -1, GameDifficulty = 0;
float FUCK = 1.f, FUCKX=0, FUCKY = 3, FUCKZ=0;
extern "C" bool IsScoreImportant(int level);

Uint32 Global_NewSpecials;
Uint32 Global_NewChalMaps;

///////////////////////////////////////////
// Ain't enough room here mate
///////////////////////////////////////////
class NotEnoughSpaceScreen : public Screen
{
private:
	int nChars;
public:
	virtual void Initialise()
	{ nChars = 0; }
	virtual void Finalise() {}
	virtual void ProcessInput (Uint32 padPress)
	{
		nChars+=2;
		if (padPress & (PDD_OK|PDD_CANCEL))
			Replace (ScreenID(noVMUWarning));
	}
	virtual void Draw() const
	{
		#if USRELEASE
			const char *text = psFormat ("`USNOTENOUGHROOM", bupNumBlocks);
		#else
			const char *text = psFormat ("`NOTENOUGHROOM", bupNumBlocks);
		#endif
		psSetColour(COL_TEXT);
		psSetAlpha(1);
		psSetPriority(0);
		psCentreWordWrapFE (
			text, 
			40, 196, PHYS_SCR_X - 64, PHYS_SCR_Y-256-96, 1.f, nChars);
		int frog = nChars - strlen (text);
		if (frog > 0) {
			psSetColour(COL_TEXT);
			psDrawCentredFE ("`PRESSABX", PHYS_SCR_Y-128-96, frog);
		}
	}
	virtual Uint32 BackgroundColour() { return 0x203020; }
};
Screen *notEnoughSpaceScreen() { return new NotEnoughSpaceScreen; }

///////////////////////////////////////////
// Create a new game
///////////////////////////////////////////
class NewGameScreen : public VMUSelectScreen
{
public:
	NewGameScreen()
	{
		GameVMS = GameProfile = -1;
		ResetGame (&CurProfile[0]);
		theWindowSequence->SetSinglePlayer();
	}
	virtual ~NewGameScreen()
	{
		theWindowSequence->SetSinglePlayer(false);
	}
	virtual void Initialise()
	{
		VMUSelectScreen::Initialise();
		if (GameVMS != -1)
			BupUnlockVMS (GameVMS);
		GameVMS = GameProfile = -1;
	}
	virtual const char *TitleMessage() const
	{ 
		#if USRELEASE
			return "`USWHICHVMU"; 
		#else
			return "`WHICHVMU"; 
		#endif
	}

	virtual void SelectVMU(int number, bool replace) 
	{
		ScreenID id(ovrGame);
		GameVMS = number;
		// Do we need to check for overwriting?
		for (int i = 0; i < 4; ++i) {
			if (theVMUManager->GetVMU(GameVMS)->GetProfileName(i) == NULL) {
				GameProfile = i;
				id = ScreenID(spEnterName);
				break;
			}
		}
		// Check to see if there's enough room on the VMU
		if (!BupIsEnoughSpace(GameVMS)) {
			id = ScreenID(notEnoughSpaceScreen);
		}
		if (!replace)
			Change (id);
		else
			Replace (id);
	};
	virtual void NoVMUs()
	{
		Replace (ScreenID(noVMUWarning));
	}
};

Screen *newGameScreen() { return new NewGameScreen; }

///////////////////////////////////////////
// You ain't got no VMUs buddy
///////////////////////////////////////////
class NoVMUWarning : public Screen
{
private:
	int count;
public:
	NoVMUWarning() {}
	virtual void Initialise()
	{
		count = 0;
		GameVMS = GameProfile = -1;
	}
	virtual void Draw() const
	{
		int foo = count;

		const char *text;
		#if USRELEASE
			text = psFormat (LangLookup ("USVMUWARNING"), bupNumBlocks);
		#else
			text = psFormat (LangLookup ("VMUWARNING"), bupNumBlocks);
		#endif
		psSetColour (COL_TEXT);
		psSetAlpha(1);
		psSetPriority(0);
		psCentreWordWrapFE (
			text, 
			40, 196, PHYS_SCR_X - 64, PHYS_SCR_Y-256-96, 1.f, foo);

		foo -= strlen(text) - 1;

		psSetColour (COL_TEXT);
		text = LangLookup ("VMUCONTINUE");
		psDrawCentredFE (text, PHYS_SCR_Y-256, foo);
		foo -= strlen (text);

		if (foo>0)
			Menu::PendingCursor();
	}
	virtual void ProcessInput (Uint32 press)
	{
		count += 2;
		if (press & PDD_OK) {
			ResetGame (CurProfile);
			strcpy (CurProfile[0].name, LangLookup("PLAYER"));
			Replace (ScreenID (mainMenu));
		} else if (press & PDD_CANCEL) {
			Back();
		}
	}
	virtual Uint32 BackgroundColour() { return 0x203020; }
	virtual void Finalise() {}
};
Screen *noVMUWarning() { return new NoVMUWarning; }

///////////////////////////////////////////
// Choose a game to overwrite
///////////////////////////////////////////
class OverwriteGame : public Menu
{
private:
	int VMU;
	static char passedOnBuf[64];
public:
	OverwriteGame() : Menu(theVMUManager->GetVMU(GameVMS)->GetProfileTemplate())
	{
		VMU = GameVMS;
	}
	virtual void ProcessInput (Uint32 press)
	{
		if (!theVMUManager->GetVMU(VMU)->Connected()) {
			Back();
			return;
		}
		if (theVMUManager->HasDataChanged()) {
			Refresh (theVMUManager->GetVMU(GameVMS)->GetProfileTemplate(), false);
		}
		Menu::ProcessInput (press);
	}
	virtual bool VetoSelection(int selection)
	{
		GameProfile = selection;
		sprintf (passedOnBuf, LangLookup("OWWHICHSLOT"), theVMUManager->GetVMU(VMU)->GetProfileName(selection));
		Change (ScreenID(twoAnswers, new TA_Block (passedOnBuf,
			"`OW_YES", "`OW_NO", ScreenID (spEnterName))));

		return true;
	}
	virtual const char *GetTitle() const
	{
		return "`OWWHICHPROF";
	}
};
char OverwriteGame::passedOnBuf[64];

Screen *ovrGame() { return new OverwriteGame; }

///////////////////////////////////////////
// Single player enter name madness
///////////////////////////////////////////
static const char *SillyNameList[] =
{
   //0123456789
	"TheMoog",
	"Thon",
	"UK:R",
	"Bingo",
	"Rudie",
	"Zandapah",
	"MattUtd",
	"Snowflake",
	"Arsenal",
	"Xania",
};

class SPEnterName : public EnterName
{
public:
	SPEnterName() 
	{ 
		BupLockProfile (GameVMS, GameProfile);
	}
	virtual ~SPEnterName() 
	{
		BupUnlockProfile (GameVMS, GameProfile);
	}
	virtual const char *GetTitle() const 
	{
		return LangLookup("ENTERNAME");
	}
	virtual void Select(const char *name) 
	{
		// Check for duplicate names on a single VMU
		for (int i = 0; i < 4; ++i) {
			if (i == GameProfile || theVMUManager->GetVMU(GameVMS)->GetProfileName(i) == NULL)
				continue;
			if (strcmp(theVMUManager->GetVMU(GameVMS)->GetProfileName(i), name) == 0) {
				break;
			}
		}
		if (i != 4) {
			FE_NACK();
			return;
		}
		ResetGame (CurProfile);
		strcpy (CurProfile[0].name, name);
		// CHEAT XXXXXXXXXXXX only in debug
#ifdef _DEBUG
		if (!stricmp (name, "REDDOG")) {
			if ((PadPress[0] & (PADSHOULDERL|PADSHOULDERR)) == (PADSHOULDERL|PADSHOULDERR)) {
				strcpy (CurProfile[0].name, "~!~REDDOG~x~");
				for (i = 0; i < MAX_MISSIONS; ++i) {
					for (int j = 0; j < MAX_DIFFICULTY; ++j) {
						CurProfile[0].missionData[j][i].completed = 1;
						CurProfile[0].missionData[j][i].bestScore = 60000;
					}
				}
				CurProfile[0].unlockedSpecials = -1;
			}
		}
#endif
		// If you're holding down L and R, check the list of madness
		if ((PadPress[0] & (PADSHOULDERL|PADSHOULDERR)) == (PADSHOULDERL|PADSHOULDERR)) {
			int i;
			for (i = 0; i < asize(SillyNameList); ++i) {
				if (!stricmp(name, SillyNameList[i])) {
					sprintf (CurProfile[0].name, "~!~%s~x~", SillyNameList[i]);
					break;
				}
			}
		}
		GetRoot()->WriteProfile();
		Change (ScreenID (mainMenu));
	}
};
Screen *spEnterName() { return new SPEnterName; }

static bool needsRefresh;
class LoadGame : public Menu
{
private:
public:
	LoadGame() : Menu(theVMUManager->GetProfileTemplate())
	{
		needsRefresh = false;
		theWindowSequence->SetSinglePlayer();
	}
	virtual ~LoadGame()
	{
		theWindowSequence->SetSinglePlayer(false);
	}
	virtual void Initialise()
	{
		theVMUManager->SetSelectedScreenID(ScreenID(mainMenu));
		Refresh (theVMUManager->GetProfileTemplate(), false);
		needsRefresh = false;
	}
	virtual void Finalise()
	{
		theVMUManager->SetSelectedScreenID(ScreenID());
		Refresh (theVMUManager->GetProfileTemplate(), false);
	}
	virtual void Draw() const
	{
		if (theVMUManager->HasDataChanged())
			needsRefresh = true;
		if (!needsRefresh)
			Menu::Draw();

	}
	virtual void ProcessInput (Uint32 press)
	{
		if (theVMUManager->GetProfileTemplate()->nDescriptors == 0) {
			Back();
			return;
		}
		if (needsRefresh || theVMUManager->HasDataChanged()) {
			needsRefresh = false;
			Refresh (theVMUManager->GetProfileTemplate(), false);
		}
		Menu::ProcessInput (press);
	}

	virtual const char *GetTitle() const
	{
		return "`LOADWHICHPROF";
	}
};

Screen *loadGame() { return new LoadGame; }


///////////////////////////////////////////
// The main menu for single player!
///////////////////////////////////////////
class MainMenu : public Menu
{
private:
	char titleBuf[128];
	bool copyIt;
	GameSave gBuf;
public:
	MainMenu() : Menu(&mainMenuTemplate)
	{
		theWindowSequence->SetSinglePlayer();
		if (GameVMS >= 0 && GameProfile >= 0) {
			if (!BupIsProfileLocked (GameVMS, GameProfile))
				BupLockProfile (GameVMS, GameProfile);
		}
		GameDifficulty = 0;
		if (!CurProfile[0].missionData[0][0].completed)
			DeleteEntry (2);
		if (!(CurProfile[0].unlockedSpecials & SPECIAL_HARD_DIFFICULTY))
			DeleteEntry (1);
		// At this point set up the volumes appropriately
		sfxSetFXVolume (CurProfile[0].soundVolume);
		sfxSetCDVolume (CurProfile[0].musicVolume);
		memcpy (&gBuf, &CurProfile[0], sizeof (GameSave));
	}
	~MainMenu()
	{
		if (GameVMS >= 0)
			BupUnlockProfile (GameVMS, GameProfile);
		theWindowSequence->SetSinglePlayer(false);
	}
	virtual void Initialise()
	{
		int nC = sprintf (titleBuf, LangLookup("WHOSEPROFILE"), CurProfile[0].name);

		// Is we cheatin'?
		if ((CurProfile[0].activeSpecials & SPECIAL_CHEATING_MASK) ||
			CurProfile[0].cheatWeapon)
			sprintf (&titleBuf[nC], " %s", LangLookup("CHEATSON_PROF"));

		Menu::Initialise();

		GameDifficulty = 0;

		// store any changes to volumes
		CurProfile[0].soundVolume = sfxGetFXVolume ();
		CurProfile[0].musicVolume = sfxGetCDVolume ();
		if (memcmp(&gBuf, &CurProfile[0], sizeof (GameSave))) {
			memcpy (&gBuf, &CurProfile[0], sizeof (GameSave));
			GetRoot()->WriteProfile();
		}

		Refresh(&mainMenuTemplate);
		if (!CurProfile[0].missionData[0][0].completed)
			DeleteEntry (2);
		if (!(CurProfile[0].unlockedSpecials & SPECIAL_HARD_DIFFICULTY))
			DeleteEntry (1);
	}
	virtual void ProcessInput (Uint32 press)
	{
		Menu::ProcessInput (press &~ (PDD_CANCEL));
	}
	virtual Uint32 BackgroundColour() { return (GameDifficulty == 0) ? 0x203020 : 0x302020; }
	virtual const char *GetTitle() const { return titleBuf; }
};
Screen *mainMenu() { return new MainMenu; }

///////////////////////////////////////////
// New specials screen
///////////////////////////////////////////
class NewSpecialsScreen : public Screen
{
private:
	int nChars;
	static const char *NewSpecials[32];
	static const char *NewChallenge[7];
public:
	virtual void Initialise() 
	{
		nChars = 0;
	}
	virtual void Finalise() {}
	virtual void Draw() const 
	{
		const char *ch;
		int lag = nChars;
		psSetColour (0xff0000);
		psSetAlpha(1);
		psSetPriority(0);

		psSetMultiplier (1.5f);
		mPostScale32 (psGetMatrix(), 1.5f, 1.5f);
		psDrawCentredFE ((ch = "`CONGRATS"), 0, lag);
		lag -= strlen (ch);
		mPostScale32 (psGetMatrix(), 1.f / 1.5f, 1.f / 1.5f);
		psSetMultiplier (1.f);

		psSetColour (COL_TEXT);
		psDrawCentredFE ((ch = "`YOUVEUNLOCKED"), 60, lag);
		lag -= strlen (ch);

		float y = 140;
		for (int i = 0; i < 32; ++i) {
			if ((Global_NewSpecials & 
				~SPECIAL_ARMOUR_AND_WEAPON_MASK) & (1<<i)) {
				psDrawCentredFE ((ch = NewSpecials[i]), y, lag);
				lag -= strlen (ch);
				y+=35;
			}
		}
		if (SPECIAL_ARMOUR_GET(Global_NewSpecials)) {
			char buf[256];
			sprintf (buf, "GOT_ARMOUR%d", SPECIAL_ARMOUR_GET(CurProfile[0].unlockedSpecials));
			psDrawCentredFE ((ch = LangLookup(buf)), y, lag);
			lag -= strlen (ch);
			y+=35;
			// Automatically set the new armour as default
			SPECIAL_ARMOUR_SET(CurProfile[0].activeSpecials, SPECIAL_ARMOUR_GET(CurProfile[0].unlockedSpecials));
		}
		if (SPECIAL_WEAPON_GET(Global_NewSpecials)) {
			char buf[256];
			sprintf (buf, "GOT_WEAPON%d", SPECIAL_WEAPON_GET(CurProfile[0].unlockedSpecials));
			psDrawCentredFE ((ch = LangLookup(buf)), y, lag);
			lag -= strlen (ch);
			y+=35;
			// Automatically set the new armour as default
			SPECIAL_WEAPON_SET(CurProfile[0].activeSpecials, SPECIAL_WEAPON_GET(CurProfile[0].unlockedSpecials));
		}

		// Set up new shield and missiles accordingly
		if (SPECIAL_MISSILE_GET(Global_NewSpecials))
			SPECIAL_MISSILE_SET(CurProfile[0].activeSpecials, 1);
		if (SPECIAL_SHIELD_GET(Global_NewSpecials))
			SPECIAL_SHIELD_SET(CurProfile[0].activeSpecials, 1);

		for (i = 6; i < 13; ++i) {
			if (Global_NewChalMaps & (1<<i)) {
				psDrawCentredFE ((ch = NewChallenge[i-6]), y, lag);
				lag -= strlen (ch);
				y+=35;
			}
		}
		psDrawCentredFE ("`MPHITSTART", PHYS_SCR_Y-96, lag);
		Menu::PendingCursor();

	}
	virtual void ProcessInput(Uint32 press) 
	{
		nChars += 2;
		if (press & (PDD_DGT_ST|PDD_DGT_TA|PDD_DGT_TB|PDD_DGT_TX|PDD_DGT_TY)) {
			FE_ACK();
			Back();
		}
	}
	virtual Uint32 BackgroundColour() { return 0x203020; }
};
Screen *newSpecialsScreen() { return new NewSpecialsScreen; }
const char *NewSpecialsScreen::NewSpecials[32] =
{
	"`GOT_TRIPPYTRAILS",
	"`GOT_PERMBOOST",
	"`GOT_PERMSHIELD",
	"`GOT_BOT1",
	"`GOT_BOT2",
	"`GOT_INFWEAPCHARGE",
	"`GOT_INVULN",
	"`GOT_60HZRACE",
	"armourbit",
	"armourbit",
	"weaponbit",
	"weaponbit",
	"`GOT_MISSILE",
	"`GOT_SHIELD",
	"reserved",
	"reserved",
	"`GOT_KOTH",
	"`GOT_HARD",
	"`GOT_SHELL",
	"`GOT_ELECTRO",
	"`GOT_HOMING",
	"`GOT_LOCKON",
	"`GOT_SWARM",
	"`GOT_ULTRA",
	"`GOT_VULCAN",
	"`GOT_MP1",
	"`GOT_MP2",
	"`GOT_MP3",
	"`GOT_MP4",
	"`GOT_MP5",
	"`GOT_MP6",
	"`GOT_MP7"
};

const char *NewSpecialsScreen::NewChallenge[7] =
{
	"`GOT_CHAL1",
	"`GOT_CHAL2",
	"`GOT_CHAL3",
	"`GOT_CHAL4",
	"`GOT_CHAL5",
	"`GOT_CHAL6",
	"`GOT_CHAL7",
};
///////////////////////////////////////////
// The mission select screen
///////////////////////////////////////////
float GlobalRotAmt[13] = 
{
	1.98f,				// volcano
	-0.3f,				// ice
	-0.125f,			// canyon
	3.8f,				// hydro 
	0.25f,				// city
	0,					// alien bass with Tim Simenon
	2.55f,				// challenge 1
	3.f,				// challenge 2
	3.5f,				// challenge 3
	4.35f,				// challenge 4
	2.5f,				// challenge 5
	1.8f,				// challenge 6
	-0.45f,				// challenge 7
};
class MissionSelectScreen : public Screen
{
protected:
	int nInits;
	int count;
	int level;
	float zoomAmt, worldRot, targetRot;
	float worldXScale, worldYScale, worldVX, worldVY, scaleAdj, targetScaleAdj;
	char titleBuf[64];
	Uint32 storedUnlocked, chalLevelsAvail;
	float titleScale;
public:
	MissionSelectScreen()
	{
		nInits = 0;
		count = 0;
		zoomAmt = worldRot = targetRot = 0.f;
		scaleAdj = targetScaleAdj = 1;
		SetupLevel(); // Workaround
		worldRot = targetRot = GetRotation(level);
		storedUnlocked = CurProfile[0].unlockedSpecials;
		// Find all the available MP levels
		chalLevelsAvail = 0;
		for (int i = 6; i < 13; ++i)
			if (LevAvailable(i))
				chalLevelsAvail |= (1<<i);
	}
	virtual void SetupLevel()
	{
		
		// Find the first level yet to be completed
		for (int l = MinLevel(); l < MaxLevel(); ++l)
			if (LevAvailable(l))
				level = l;
	}
	virtual float GetRotation (int level)
	{
		char temp[64];
		float width;
		// Also sets up the title buffer
		sprintf (temp, "MISSION%d_NAME", level+1);
		sprintf (titleBuf, "%s%s%s", 
			(level == NewLevel(-1)) ? "" : "< ",
			LangLookup(temp),
			(level == NewLevel(1)) ? "" : " >");
		width = psStrWidth (titleBuf);
		if (width > (PHYS_SCR_X - 32))
			titleScale = (PHYS_SCR_X - 32) / width;
		else
			titleScale = 1;
		return GlobalRotAmt[level];
	}
	virtual void Initialise ()
	{
		count = 0;
		zoomAmt = -5;
		worldXScale = worldYScale = 0.f;
		worldVX = worldVY = 0;
		nInits++;
	}
	virtual void Finalise() {}
	virtual bool IsChallenge() const { return false; }
	virtual void Draw() const
	{
		int lag = count, storedLag;
		const char *str;
		float w[4];
		float flashAmt;

		TexPaletteEntry *ent = (TexPaletteEntry *)
			((char *)zoomMat[level].surf.surfaceDesc - offsetof(TexPaletteEntry, desc));

		if (IsChallenge()) {
			int zAmt = int(fmod(zoomAmt, ent->animation->anim.nFrames));
			ent->desc = ent->animation->descArray[MAX(0, zAmt)];
			flashAmt = MAX(0, (zoomAmt - (int)zoomAmt) - 0.5f);
		} else {
			if (zoomAmt < (ent->animation->anim.nFrames-1)) {
				ent->desc = ent->animation->descArray[MAX(0, (int)zoomAmt)];
				flashAmt = MAX(0, (zoomAmt - (int)zoomAmt) - 0.5f);
			} else {
				ent->desc = ent->animation->descArray[(ent->animation->anim.nFrames-1)];
				flashAmt = 0;
			}
		}

		psSetAlpha (1);
		psSetPriority(0);
		psSetColour (COL_TEXT);
		psGetWindow(w);

		// Title
		str = titleBuf;
		{
			Matrix32 m = *psGetMatrix();
			mPostTranslate32 (psGetMatrix(), -(PHYS_SCR_X/2-8), -16);
			mPostScale32 (psGetMatrix(), titleScale, 1);
			mPostTranslate32 (psGetMatrix(), PHYS_SCR_X/2-8, 16);
			psDrawCentredFE (str, 0, lag);
			lag -= strlen (str);
			psSetMatrix (&m);
		}

		float top = 80.f;
		float bottom = top + 256.f;
		float left = 48.f;
		float right = left + 256.f;

		lag *=12;

		// World box
		DrawLineFE (left, top, right, top, 1, lag, 1);
		lag--;
		DrawLineFE (left, top, left, bottom, bottom - top, lag, 1);
		DrawLineFE (right, top, right, bottom, bottom - top, lag, 1);
		lag -= (bottom - top);
		DrawLineFE (left, bottom, right, bottom, 1, lag, 1);
		lag--;

		storedLag = lag;

		// Info box setup
		float infotop = bottom + 25.f;
		float infobot = PHYS_SCR_Y - 32.f - 16.f;
		float infoleft = left;
		float inforight = PHYS_SCR_X - 48.f;

		// Line round to info box
		float linetop = bottom - 25.f;
		float lineleft = left - 25.f;
		float linemid = infotop + 25.f;;

		DrawLineFE(left, linetop, lineleft, linetop, left - lineleft, lag, 1);
		lag -= (left-lineleft);
		DrawLineFE(lineleft, linetop, lineleft, linemid, linemid - linetop, lag, 1);
		lag -= (linemid - linetop);
		DrawLineFE(lineleft, linemid, infoleft, linemid, infoleft - linemid, lag, 1);
		lag -= (infotop - linemid);

		// Info box draw
		DrawLineFE(infoleft, infotop, inforight, infotop, 1, lag, 1);
		lag--;
		DrawLineFE(infoleft, infotop, infoleft, infobot, infobot - infotop, lag, 1);
		DrawLineFE(inforight, infotop, inforight, infobot, infobot - infotop, lag, 1);
		lag-=(infobot-infotop);
		DrawLineFE(infoleft, infobot, inforight, infobot, 1, lag, 1);
		lag--;

		lag /= 12;

		char buf[64];
		if (level >= 6 && // Challenge level
			CurProfile[0].missionData[GameDifficulty][level].completed) {
			if (ChallengeSmashed (level))
				strcpy (buf, "`MISSION_DONE");
			else
				sprintf (buf, "`MISSION%d_DESC2", level+1);
		} else {
			sprintf (buf, "`MISSION%d_DESC", level+1);
		}
		psCentreWordWrapFE (buf,
			infoleft+12, infotop+6, inforight-12, infobot-6, 
			0.65f, lag);

		lag = storedLag;

		// Now for the zoombox and the gradebox
		float zoomleft = right + 32.f;
		float zoomright = PHYS_SCR_X - 48.f;
		float zoomtop = top;
		float zoombot = top + int((bottom - top) * 3.f / 5.f) - 8.f;

		float otherLineMid = (zoomleft + right)*0.5f;
		float otherLineTop = zoomtop + 25.f;

		DrawLineFE (right, linetop, otherLineMid , linetop, otherLineMid *0.5f - right, lag, 1);
		lag -= otherLineMid - right;
		DrawLineFE (otherLineMid, linetop, otherLineMid, otherLineTop, linetop - otherLineTop, lag, 1);
		lag -= linetop - otherLineTop;
		DrawLineFE (otherLineMid, otherLineTop, zoomleft, otherLineTop, zoomleft - otherLineMid, lag, 1);
		lag -= zoomleft - otherLineMid;

		DrawLineFE (zoomleft, zoomtop, zoomright, zoomtop, 1, lag, 1);
		lag--;
		DrawLineFE (zoomleft, zoomtop, zoomleft, zoombot, zoombot-zoomtop, lag, 1);
		DrawLineFE (zoomright, zoomtop, zoomright, zoombot, zoombot-zoomtop, lag, 1);
		DrawFESprite (&zoomMat[level], zoomleft-w[1], zoomtop-w[0], zoomright-zoomleft, zoombot - zoomtop, lag, 1, flashAmt*flashAmt * 0.8f, 1);
		lag -= (zoombot-zoomtop);
		DrawLineFE (zoomleft, zoombot, zoomright, zoombot, 1, lag, 1);
		lag--;
		
		// Stats box
		float statsTop = zoombot + 16.f;
		float statsBot = bottom;
		float statsDiv = zoomleft + (IsChallenge() ? 128.f : 64.f);

		DrawLineFE ((zoomleft+zoomright)*0.5f, zoombot, (zoomleft+zoomright)*0.5f, statsTop, statsTop - zoombot, lag, 1);
		lag -= statsTop - zoombot;

		DrawLineFE (zoomleft, statsTop, zoomright, statsTop, 1, lag, 1);
		lag--;
		DrawLineFE (zoomleft, statsTop, zoomleft, statsBot, statsBot-statsTop, lag, 1);
		DrawLineFE (zoomright, statsTop, zoomright, statsBot, statsBot-statsTop, lag, 1);
		if (CurProfile[0].missionData[GameDifficulty][level].completed)
			DrawLineFE (statsDiv, statsTop, statsDiv, statsBot, statsBot-statsTop, lag, 1);
		lag -= (statsBot-statsTop);
		DrawLineFE (zoomleft, statsBot, zoomright, statsBot, 1, lag, 1);
		lag--;

		// Text in the stats box
		lag /= 12;
		if (CurProfile[0].missionData[GameDifficulty][level].completed) {
			Matrix32 mat;
			char buf[2] = { CurProfile[0].missionData[GameDifficulty][level].grading + 'A', 0 };
			psCentreWordWrapFE (IsChallenge() ? LangLookup("PASS") : buf,
				zoomleft, statsTop, statsDiv, statsBot, 
				1.5f, lag);
			lag--;

			mat = *psGetMatrix();
			mPostScale32 (psGetMatrix(), 0.65f, 0.65f);
			mPostTranslate32 (psGetMatrix(), statsDiv - 8 + 4, statsTop - 32 + 4);

			const char *ch;
			if (!IsChallenge()) {
				ch = psFormat ("S: %d (%d)", 
					CurProfile[0].missionData[GameDifficulty][level].bestScore,
					GetScoreRating (level, CurProfile[0].missionData[GameDifficulty][level].bestScore));
			} else {
				ch = IsScoreImportant(level) ? psFormat("S: %d", CurProfile[0].missionData[GameDifficulty][level].bestScore) : "S: N/A";
			}
			psDrawStringFE (ch, 0, 35*0, lag);
			lag -= strlen(ch);

			if (!IsChallenge()) {
				ch = psFormat ("L: %d (%d)", 
					3 - CurProfile[0].missionData[GameDifficulty][level].livesLost,
					GetLivesRating (level, CurProfile[0].missionData[GameDifficulty][level].livesLost));
			} else {
				ch = psFormat ("L: %d", 
					3 - CurProfile[0].missionData[GameDifficulty][level].livesLost);
			}
			psDrawStringFE (ch, 0, 35*1, lag);
			lag -= strlen(ch);

			if (!IsChallenge()) {
				ch = psFormat ("T: %d:%02d (%d)", 
					int(CurProfile[0].missionData[GameDifficulty][level].bestTime) / 60,
					int(CurProfile[0].missionData[GameDifficulty][level].bestTime) % 60,
					GetTimeRating (level, CurProfile[0].missionData[GameDifficulty][level].bestTime));
			} else {
				ch = psFormat ("T: %d:%02d", 
					int(CurProfile[0].missionData[GameDifficulty][level].bestTime) / 60,
					int(CurProfile[0].missionData[GameDifficulty][level].bestTime) % 60);
			}
			psDrawStringFE (ch, 0, 35*2, lag);
			lag -= strlen(ch);
			if (!IsChallenge()) {
				ch = psFormat ("B: %d:%02d (%d)", 
					int(CurProfile[0].missionData[GameDifficulty][level].bestBossTime) / 60,
					int(CurProfile[0].missionData[GameDifficulty][level].bestBossTime) % 60,
					GetBossTimeRating (level, CurProfile[0].missionData[GameDifficulty][level].bestBossTime));
			} else {
				ch = "B: N/A";
			}
			psDrawStringFE (ch, 0, 35*3, lag);
			lag -= strlen(ch);

			psSetMatrix (&mat);
		} else {
			char *ch = "`MISNOTCOMPL";
			psSetColour (COL_TEXT);
			psCentreWordWrapFE (ch,
				zoomleft, statsTop, zoomright, statsBot, 
				0.65f, lag);
			lag-= strlen (ch);
		}

		float x, y;
		mUnit (&mCurMatrix);
		x = context.midX; y = context.midY;
		context.midX = (left+right)/2;
		context.midY = (top+bottom)/2;
		mRotateZ (&mCurMatrix, worldRot);
		mPostScale (&mCurMatrix, worldXScale*0.85f*scaleAdj, 0.85f*scaleAdj, worldYScale*0.85f*scaleAdj);
		mPostRotateX (&mCurMatrix, 0.3f);
		mPostTranslate (&mCurMatrix, 0.f, 3, 0.01);
		rDrawObject (World->obj);
	}

	int NewLevel(int increment)
	{
		int oldLevel = level;
		int newLevel = level;
		do {
			newLevel += increment;
		} while (!LevAvailable(newLevel) && 
			newLevel >= MinLevel() &&
			newLevel < MaxLevel());
		if (!LevAvailable(newLevel) || newLevel < MinLevel() || newLevel >= MaxLevel()) {
			return oldLevel;
		} else {
			return newLevel;
		}
	}

	virtual void ProcessInput (Uint32 press)
	{
		if (nInits > 1) {
			// We have come back from the game itself 
			// any new facilities?
			Uint32 newChalLevels = 0;
			for (int i = 6; i < 13; ++i)
				if (LevAvailable(i))
					newChalLevels |= (1<<i);

			if (storedUnlocked != CurProfile[0].unlockedSpecials
				||
				chalLevelsAvail != newChalLevels) {
				Global_NewSpecials = storedUnlocked ^ CurProfile[0].unlockedSpecials;
				Global_NewChalMaps = chalLevelsAvail ^ newChalLevels;
				Replace (ScreenID(newSpecialsScreen));
			} else
				Back();
			return;
		}
		if (press & PDD_CANCEL) {
			FE_ACK();
			Back();
			return;
		} else if ((press & PDD_OK) && !ChangingState()) {
			extern int LogToPhysTab[4];

			FE_ACK();
			LevelNum = level;
			player[0].handicap = 1.f;
			NumberOfPlayers = 1;
			Multiplayer = 0;

			int port;
			for (port = 0; port < 4; ++port) {
				if (FindPhysicalController(port))
					break;
			}
			dAssert (port != 4, "ack");
			LogToPhysTab[0] = port;
			LogToPhysTab[1] = -1;
			LogToPhysTab[2] = -1;
			LogToPhysTab[3] = -1;

			FadeToNewState (FADE_TO_BLACK, GS_GAME);
		} else if (press & (PDD_DGT_KL|PDD_DGT_KR)) {
			int newLevel = NewLevel((press & PDD_DGT_KR) ? 1 : -1);
			if (newLevel == level) {
				FE_NACK();
			} else {
				level = newLevel;
				targetRot = GetRotation(level);
				zoomAmt = -5;
				FE_MoveSound();
			}
		}
		count += 2;
		zoomAmt += IsChallenge() ? 0.065f : 0.075f;
		if (level == 5) {
			worldRot += 0.001f;
			if (worldRot > 2*PI)
				worldRot -= 2*PI;
		} else {
			worldRot += (targetRot - worldRot) * 0.3f;
		}
		scaleAdj += (targetScaleAdj - scaleAdj) * 0.15f;
		if (count > 120) {
			worldVX = (worldVX + (1.f - worldXScale) * 0.18f) * 0.75f;
			worldVY = (worldVY + (1.f - worldYScale) * 0.1f) * 0.75f;
			worldXScale += worldVX;
			worldYScale += worldVY;
		}
		// Globe objId madness
		for (int i = 1; i <= World->noObjId; ++i) {
			if (World->objId[i]) {
				if (level == (i-1))
					World->objId[i]->collFlag &= ~OBJECT_INVISIBLE;
				else
					World->objId[i]->collFlag |= OBJECT_INVISIBLE;
				World->objId[i]->model->material->diffuse.colour = ColLerp (0xffffff, 0xff0000, 0.5f + 0.5f * sin(currentFrame*0.1f)) | 0xff000000;
			}
		}
		if (level == 5)
			targetScaleAdj = 0.8f;
		else
			targetScaleAdj = 1.f;
		World->objId[6]->crntRot.z -= 0.0005f;
	}

	// Overridable :
	virtual Uint32 BackgroundColour() { return 0x203020; }
	virtual int MinLevel() { return 0; }
	virtual int MaxLevel() { return 6; }
	bool LevAvailable(int levl);
};

Screen *missionSelectScreen()
{ return new MissionSelectScreen;}


bool MissionSelectScreen::LevAvailable(int levl)
{
	if (levl == 0)
		return true;
	else if (levl < 6)
		return CurProfile[0].missionData[GameDifficulty][levl-1].completed;
	else {
		static int AvailArray[7] =
		{
			// 1-based for simplicity
			1,			// Challenge 1 dependant on level 1 (volcano)
			2,			// Challenge 2 dependant on level 2 (ice)
			3,			// Challenge 3 dependant on level 3 (canyon)
			9,			// Challenge 4 dependant on level 9 (challenge 3)
			4,			// Challenge 5 dependant on level 4 (hydro)
			11,			// Challange 6 dependant on level 11 (challenge 5)
			5,			// Challenge 7 dependant on level 5 (city)
		};
		// Dependant on easy level only
		return CurProfile[0].missionData[0][AvailArray[levl-6]-1].completed;
	}
}

///////////////////////////////////////////
// The CHALLENGE mission select screen
///////////////////////////////////////////
class ChallengeSelectScreen : public MissionSelectScreen
{
private:
public:
	virtual int MinLevel() { return 6; }
	virtual int MaxLevel() { return 13; }
	virtual bool IsChallenge() const { return true; }
};
Screen *challengeSelectScreen() 
{ return new ChallengeSelectScreen; }

///////////////////////////////////////////
// View tech screen
///////////////////////////////////////////
class ViewTechScreen : public Menu
{
private:
	static const WindowPos gunPos[4];
	static const WindowPos missilePos[2];
	static const WindowPos armourPos[3];
	static const WindowPos shieldPos[2];
#define GUN 0
#define MISSILE 1
#undef ARMOUR
#define ARMOUR 2
#define SHIELD 3
	int nChars;
	const char *descrText;
	bool changed;
public:
	ViewTechScreen() : Menu (&viewWeapTemplate)
	{
		changed = false;
	}
	virtual void Initialise()
	{
		nChars = -50;
		Menu::Initialise();
		MoveBacking(0, 0);
	}
	virtual void Finalise() { 
		Menu::Finalise(); 
		if (changed && GetPlayer() >= 0) {
			GetRoot()->WriteProfile();
		}
	}
	virtual void Draw() const 
	{
		int lag = nChars;
		Matrix32 m = *psGetMatrix();
		Uint32 colour;

		colour = int(0xc0 * GetFadeAmount()) << 24;

		mPostScale32 (psGetMatrix(), 0.75f, 0.75f);
		mPostTranslate32 (psGetMatrix(), 40, 6);
		Menu::Draw();
		psSetMatrix(&m);

		float left, right, top, bottom;

		psSetColour (COL_TEXT);
		psSetAlpha (1*GetFadeAmount());
		psSetPriority(0);

		lag *= 12;

		left = 32 + 8 - 8;
		right = PHYS_SCR_X - 8 - 8;
		top = 32 + 32 - 8;
		bottom = top + 4*35 * 0.75f + 16;

		DrawLineFE (left, top, right, top, 1, lag, GetFadeAmount());
		lag--;

		DrawLineFE (left, top, left, bottom, (bottom - top), lag, GetFadeAmount());
		DrawLineFE (right, top, right, bottom, (bottom - top), lag, GetFadeAmount());
		bottom = top + RANGE(0, lag, (bottom-top));
		rSetMaterial (&fadeMat);
		kmxxGetCurrentPtr (&vertBuf);
		kmxxStartVertexStrip (&vertBuf);
		kmxxSetVertex_0 (0xe0000000, left, top, 10, colour);
		kmxxSetVertex_0 (0xe0000000, left, bottom, 10, colour);
		kmxxSetVertex_0 (0xe0000000, right, top, 10, colour);
		kmxxSetVertex_0 (0xf0000000, right, bottom, 10, colour);
		kmxxReleaseCurrentPtr (&vertBuf);

		lag -= (bottom-top);
		DrawLineFE (left, bottom, right, bottom, 1, lag, GetFadeAmount());
		lag--;

		bottom = PHYS_SCR_Y - 32 - 8;
		top = bottom - 100;

		DrawLineFE (left, top, right, top, 1, lag, GetFadeAmount());
		lag--;
		
		DrawLineFE (left, top, left, bottom, (bottom - top), lag, GetFadeAmount());
		DrawLineFE (right, top, right, bottom, (bottom - top), lag, GetFadeAmount());
		bottom = top + RANGE(0, lag, (bottom-top));
		rSetMaterial (&fadeMat);
		kmxxGetCurrentPtr (&vertBuf);
		kmxxStartVertexStrip (&vertBuf);
		kmxxSetVertex_0 (0xe0000000, left, top, 10, colour);
		kmxxSetVertex_0 (0xe0000000, left, bottom, 10, colour);
		kmxxSetVertex_0 (0xe0000000, right, top, 10, colour);
		kmxxSetVertex_0 (0xf0000000, right, bottom, 10, colour);
		kmxxReleaseCurrentPtr (&vertBuf);
		
		lag -= (bottom-top);
		DrawLineFE (left, bottom, right, bottom, 1, lag, GetFadeAmount());
		lag--;

		lag /= 12;

		psCentreWordWrapFE (
			descrText, 
			left + 8, top + 8.f, right - 8.f, bottom - 8.f, 0.75f, lag);
		lag -= strlen (descrText);
	}
	virtual void ProcessInput(Uint32 press)
	{
		Menu::ProcessInput(press);
		nChars += 2;
	}
	void MoveBacking(int curType, int cursor);
	virtual Material *Background() { return &techMat; }
	virtual bool HasShutdown() { return true; }
	inline void SetChanged() { changed = true; }
};

void ViewTechScreen::MoveBacking(int curType, int cursor)
{
	const char *typeName;
	const WindowPos *p;
	char tmpBuf[64];
	switch (curType) {
	case GUN:
		typeName = "GUN";
		p = gunPos + cursor;
		break;
	case MISSILE:
		typeName = "MISSILE";
		p = missilePos + cursor;
		break;
	case ARMOUR:
		typeName = "ARMOUR";
		p = armourPos + cursor;
		break;
	case SHIELD:
		typeName = "SHIELD";
		p = shieldPos + cursor;
		break;
	}
	GetRoot()->SetTargetPos(*p);
	sprintf (tmpBuf, "%s_%d", typeName, cursor+1);
	descrText = LangLookup (psFormat("%s_INFO", tmpBuf));
}

const WindowPos ViewTechScreen::gunPos[4] =
{
	{ 2, 11 + PHYS_SCR_X/4, 4 + PHYS_SCR_Y/4 },
	{ 2, 400 + PHYS_SCR_X/4, 2 + PHYS_SCR_Y/4 },
	{ 1, 5 + PHYS_SCR_X/2, 247 + PHYS_SCR_Y/2 },
	{ 1, 385 + PHYS_SCR_X/2, 541 + PHYS_SCR_Y/2 }
};
const WindowPos ViewTechScreen::missilePos[2] =
{ 
	{ 4, 286 + PHYS_SCR_X/8, 868 + PHYS_SCR_Y/8 },
	{ 2, 5 + PHYS_SCR_X/4, 779 + PHYS_SCR_Y/4 }
};
const WindowPos ViewTechScreen::shieldPos[2] =
{ 
	{ 2, 772 + PHYS_SCR_X/4, 5 + PHYS_SCR_Y/4 },
	{ 4, 29 + PHYS_SCR_X/8, 635 + PHYS_SCR_Y/8 }
};
const WindowPos ViewTechScreen::armourPos[3] =
{
	{ 2, 706 + PHYS_SCR_X/4, 353 + PHYS_SCR_Y/4 },
	{ 4, 565 + PHYS_SCR_X/8, 446 + PHYS_SCR_Y/8 },
	{ 2, 706 + PHYS_SCR_X/4, 353 + PHYS_SCR_Y/4 }
};

Screen *viewTechScreen()
{ return new ViewTechScreen;}

// Weapon settings
class WeapEntry : public GameTypeEntry
{
private:
	ViewTechScreen *screen;
	const char *typeName;
	int maxCurs;
	int type, cursor;
public:
	WeapEntry(const char *text, ScreenID &sub) : GameTypeEntry(text, ScreenID()) 
	{
		type = int(sub.GetUser());
		cursor = 0;
		switch (type) {
		case GUN:
			typeName = "GUN";
			maxCurs = SPECIAL_WEAPON_GET(CurProfile[0].unlockedSpecials);
			if (maxCurs > 3)
				maxCurs = 3;
			cursor = SPECIAL_WEAPON_GET(CurProfile[0].activeSpecials);
			break;
		case MISSILE:
			typeName = "MISSILE";
			maxCurs = SPECIAL_MISSILE_GET(CurProfile[0].unlockedSpecials);
			if (maxCurs > 1)
				maxCurs = 1;
			cursor = SPECIAL_MISSILE_GET(CurProfile[0].activeSpecials);
			break;
		case ARMOUR:
			typeName = "ARMOUR";
			maxCurs = SPECIAL_ARMOUR_GET(CurProfile[0].unlockedSpecials);
			if (maxCurs > 2)
				maxCurs = 2;
			cursor = SPECIAL_ARMOUR_GET(CurProfile[0].activeSpecials);
			break;
		case SHIELD:
			typeName = "SHIELD";
			maxCurs = SPECIAL_SHIELD_GET(CurProfile[0].unlockedSpecials);
			if (maxCurs > 1)
				maxCurs = 1;
			cursor = SPECIAL_SHIELD_GET(CurProfile[0].activeSpecials);
			break;
		}
	}
	virtual float Draw(float selAmt, float alpha, float nChars)
	{
		if (selAmt > 0.1f)
			screen->MoveBacking (type, cursor);
		return GameTypeEntry::Draw(selAmt, alpha, nChars);
	}
	void DoTheSetting();
	virtual void Left()
	{
		if (cursor > 0) {
			cursor--;
			FE_MoveSound();
			DoTheSetting();
		} else {
			FE_NACK();
			cursor = 0;
		}
	}
	virtual void Right()
	{
		if (cursor < maxCurs) {
			cursor++;
			FE_MoveSound();
			DoTheSetting();
		} else {
			FE_NACK();
			cursor = maxCurs;
		}
	}
	virtual const char *GetText()
	{
		char tmpBuf[64];
		sprintf (tmpBuf, "%s_%d", typeName, cursor+1);
		return LangLookup(tmpBuf);
	}
	virtual void SetWindow (Window *rootWindow)
	{
		screen = (ViewTechScreen *)rootWindow->GetCurScreen();
	}
};
void WeapEntry::DoTheSetting()
{
	screen->SetChanged();
	switch (type) {
	case GUN:
		SPECIAL_WEAPON_SET(CurProfile[0].activeSpecials, cursor);
		break;
	case ARMOUR:
		SPECIAL_ARMOUR_SET(CurProfile[0].activeSpecials, cursor);
		break;
	case SHIELD:
		SPECIAL_SHIELD_SET(CurProfile[0].activeSpecials, cursor);
		break;
	case MISSILE:
		SPECIAL_MISSILE_SET(CurProfile[0].activeSpecials, cursor);
		break;
	}
}

MenuEntry *weapMaker(const char *text, ScreenID &sub)
{
	return new WeapEntry(text, sub);
}

//////////////////////////////////////////////////////
// Called by the C game layer just before beginning
// to allow us to set up all the cheats and specials
//////////////////////////////////////////////////////
extern "C" void SetupSpecials(void)
{
	int reward = 0;
	// Set up player[0].rewards : cheats
	if (CurProfile[0].activeSpecials & SPECIAL_PERMANENT_BOOST)
		reward |= CHEAT_PERMANENT_BOOST;
	if (CurProfile[0].activeSpecials & SPECIAL_INFINITE_SHIELD)
		reward |= CHEAT_INFINITE_SHIELD;
	if (CurProfile[0].activeSpecials & SPECIAL_INVULNERABILITY)
		reward |= CHEAT_INVUL;
	if (CurProfile[0].activeSpecials & SPECIAL_INF_WEAPON_CHARGE)
		reward |= CHEAT_INF_CHARGE;
	if (CurProfile[0].activeSpecials & SPECIAL_ASSISTANCE_BOT_1)
		reward |= CHEAT_BOT_1;
	if (CurProfile[0].activeSpecials & SPECIAL_ASSISTANCE_BOT_2)
		reward |= CHEAT_BOT_2;

	// Active upgrades:
	int armour = SPECIAL_ARMOUR_GET(CurProfile[0].activeSpecials);
	switch (armour) {
	case 0:
		break;
	case 1:
		reward |= UPGRADE_ARMOUR_1;
		break;
	case 2:
		reward |= UPGRADE_ARMOUR_2;
		break;
	}

	if (SPECIAL_MISSILE_GET(CurProfile[0].activeSpecials))
		reward |= UPGRADE_MISSILE;
	if (SPECIAL_SHIELD_GET(CurProfile[0].activeSpecials))
		reward |= UPGRADE_SHIELD;

	if (CurProfile[0].cheatWeapon)
		reward |= 1 << (SPECIAL_FIRST_WEAP_BIT + (CurProfile[0].cheatWeapon-1));

	player[0].reward = reward;

	int guns = SPECIAL_WEAPON_GET(CurProfile[0].activeSpecials);
	player[0].PlayerWeaponType = guns;

	// XXX other cheat madnesses go here!
}
class BonusMenu : public Menu
{
private:
	Uint32 activeSpecials;
public:
	BonusMenu() : Menu(&bonusTemplate)
	{
		int delCtr = 0;
		// Set up based on those specials we can actually see
		for (int i = 0; i < 8; ++i)
			if (!(CurProfile[0].unlockedSpecials & (1<<i))) {
				DeleteEntry (i-delCtr);
				delCtr++;
			}
		if ((CurProfile[0].unlockedSpecials & SPECIAL_WEAP_ALL) == 0)
			DeleteEntry(8-delCtr);
	}
	virtual void Initialise()
	{
		Menu::Initialise();
		activeSpecials = CurProfile[0].activeSpecials;
	}
	virtual void Finalise()
	{
	}
	virtual int NumExtra() const
	{
		if ((CurProfile[0].activeSpecials & SPECIAL_CHEATING_MASK) || CurProfile[0].cheatWeapon)
			return 1;
		else
			return 0;
	}
	virtual float DrawExtra(int line, float alpha, float nChars) const
	{
		if ((CurProfile[0].activeSpecials & SPECIAL_CHEATING_MASK) || CurProfile[0].cheatWeapon) {
			psSetAlpha (alpha);
			psSetColour (0xffffff);
			psDrawStringFE ("`CHEATSON", 0, 35, nChars);
		}
	}
	virtual Uint32 BackgroundColour() { return 0x203020; }
};
Screen *bonusScreen()
{ return new BonusMenu;}

////////////////////////////////////////////////
// XXX needs filling in with appropriate levels
////////////////////////////////////////////////
// XXX LEVEL DEPENDENCY
extern "C" bool IsScoreImportant(int level)
{
	switch (level)
	{
	default:
		return true;
	case LEV_SHOOTING_RANGE:
	case LEV_DEFENSIVE:
	case LEV_SEARCH_AND_COLLECT:
	case LEV_FOOTSOLDIER_RAMPAGE:
	case LEV_HEAVY_DUTY_RAMPAGE:
		return true;
	case LEV_VEHICLE_MANOEUVRING:
	case LEV_ADV_VEHICLE_MANOEUVRING:
		return false;
	}
}
