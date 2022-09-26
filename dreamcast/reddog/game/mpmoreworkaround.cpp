#define NO_PROTOTYPES
#include "Menus.h"
#include "MiscScreens.h"

// Handy map number address routine
extern MapNum *GetPTMap();
extern char *TGameName(TournamentGame *t, int j);


// Tournament type menu entry
class TTEntry : public GameTypeEntry
{
private:
	int VMU, subTournament;

	void TournTypeInc(int amount);
public:
	TTEntry(const char *text, ScreenID &sub) : GameTypeEntry(text, sub) 
	{
		VMU = subTournament = 0;
		// Check for LOOKUPFROMVMS
		if (mpSettings.game.tournament.tType == LOOKUPFROMVMS)
			TournTypeInc(0);
	}
	virtual void Left()
	{
		TournTypeInc(-1);
		FE_MoveSound();
	}
	virtual void Right()
	{
		TournTypeInc(1);
		FE_MoveSound();
	}
	virtual float Draw(float selAmt, float alpha, float nChars)
	{
		// Check for LOOKUPFROMVMS
		if (mpSettings.game.tournament.tType == LOOKUPFROMVMS)
			TournTypeInc(0);
		// Before we draw we ought to check we're making sense
		if (mpSettings.game.tournament.tType == FROMVMS &&
			!theVMUManager->GetVMU(VMU)->Connected())
			TournTypeInc(1);
		return GameTypeEntry::Draw(selAmt, alpha, nChars);
	}
	virtual const char *GetText()
	{
		return mpSettings.game.tournament.name;
	}
};
MenuEntry *ttMaker(const char *text, ScreenID &sub)
{
	return new TTEntry (text, sub);
}

void TTEntry::TournTypeInc(int amount)
{
	int num = int(mpSettings.game.tournament.tType);
	// Special case - skip through and find save matching CustomTournament
	if (num == LOOKUPFROMVMS) {
		mpSettings.game.tournament.tType = FROMVMS;
		for (VMU = 0; VMU < 4; ++VMU) {
			for (subTournament = 0; subTournament < 4; ++subTournament) {
				const char *name = theVMUManager->GetVMU(VMU)->GetTournamentName(subTournament);
				if (name && !stricmp (name, CustomTournament.name))
					return;
			}
		}
		// Fail safe
		mpSettings.game.tournament.tType = BRONZE;
		return;
	}
	if (num != FROMVMS) {
		num += amount;
		if (num < 0) {
			num = FROMVMS;
			VMU = 8;
			subTournament = 0;
		} else if (num == FROMVMS) {
			VMU = -1; subTournament = 3;
		}
	}
	if (num == FROMVMS) {
		int startVMU, startSub;
		bool breakOut = false;
		startVMU = VMU; startSub = subTournament;
		// Move to the next VMU
		do {
			subTournament += amount;
			if (subTournament < 0) {
				subTournament = 3;
				VMU = VMU - 1;
				if (VMU < 0)
					breakOut = true;
			} else if (subTournament > 3) {
				subTournament = 0;
				VMU += 1;
				if (VMU > 7)
					breakOut = true;
			}
			// Have we looped all around?
			if (breakOut || (startVMU == VMU && startSub == subTournament))
				break;
		} while (theVMUManager->GetVMU(VMU)->GetTournamentName(subTournament) == NULL);
		if (breakOut || (startVMU == VMU && startSub == subTournament)) {
			num += amount;
			if (num > FROMVMS)
				num = 0;
		} else {
			memcpy (&mpSettings.game.tournament, theVMUManager->GetVMU(VMU)->GetTournament(subTournament), 
				sizeof (mpSettings.game.tournament));
		}
	}
	// Update the copy
	mpSettings.game.tournament.tType = TournamentType(num);
	if (mpSettings.game.tournament.tType == CUSTOM)
		memcpy (&mpSettings.game.tournament, &CustomTournament, sizeof (CustomTournament));
	SetUpForTournament(&mpSettings.game.tournament);
}

///////////////////////////////////////////////////////////
// The weapon select screen
///////////////////////////////////////////////////////////
#define SPR_GAP 8
class WeaponScreen : public Screen
{
private:
	int timer, cursor;
	float fadeAmount;
	Uint32 DenyBitField;
public:
	WeaponScreen() : Screen()
	{
		cursor = 0;
	}

	virtual void Initialise()
	{
		timer = 0;
		DenyBitField = 0;
		fadeAmount = 1.f;
		if (mpSettings.playMode != TOURNAMENT) {
			if (GetCurrentGameType() == PREDATOR)
				DenyBitField |= WEAPON_ULTRA_LASER;
			if (GetCurrentGameType() != DEATHMATCH && GetCurrentGameType() != KNOCKOUT)
				DenyBitField |= WEAPON_STEALTH;
			if (!(GetCurrentGameType() == DEATHMATCH || GetCurrentGameType() == KNOCKOUT))
				DenyBitField |= WEAPON_STORMING_SHIELD;
		}
	}
	virtual void Finalise() {}
	float GetFadeAmount() const { return fadeAmount; }

	bool IsDenied(int i) const
	{
		if (DenyBitField & (1<<i))
			return true;
		else 
			return false;
	}

	virtual void Draw() const
	{
		float adj = 0;
		for (int i = 0; i < 10; ++i) {
			float x, y;
			x = ((640 - 512 - (3 * SPR_GAP)) / 2) + (i & 3) * (128.f + SPR_GAP) + (SPR_GAP/2) - 8.f;
			y = ((480 - 384 - (2 * SPR_GAP)) / 2) + (i / 4) * (128.f + SPR_GAP) + (SPR_GAP/2) - 32.f;
			DrawFESprite (&weapMat[i], x, y, 128, 128, timer-adj, 
				(!IsDenied(i) && mpSettings.weapons & (1<<i)) ? 1.f : 0.2f, 
				(cursor==i) ? ((currentFrame & 16) ? 0.75f : 0.25f) : 0.f, GetFadeAmount());
			if (IsDenied(i)) {
				DrawFESprite (&bigXMat, x, y, 128, 128, timer-adj, 
					1, 0, GetFadeAmount()); 
			}
			adj += 128.f;
		}
	}
	virtual void ProcessInput(Uint32 press)
	{
		int motion = 0;
		timer += 43.f;
		if (press & PDD_CANCEL) {
			FE_ACK();
			fadeAmount = 0.99f;
		} else if (press & PDD_DGT_KL) {
			motion = 3;
		} else if (press & PDD_DGT_KR) {
			motion = 1;
		} else if (press & PDD_DGT_KU) {
			motion = 12;
		} else if (press & PDD_DGT_KD) {
			motion = 4;
		} else if (press & PDD_OK) {
			FE_ACK();
			mpSettings.weapons ^= (1<<cursor);
		}
		while (motion) {
			int xBits = 3, yBits = 12;
			if (cursor & 2)
				yBits = 4;
			if (cursor > 7)
				xBits = 1;

			cursor = (((cursor & xBits) + (motion & xBits)) & xBits) |
				(((cursor & yBits) + (motion & yBits)) & yBits);
			if (cursor > 11) {
				if (press & PDD_DGT_KD)
					cursor -= 12;
				else
					cursor -= 4;
			}
			if (!IsDenied(cursor)) {
				FE_MoveSound();
				motion = 0;
			}
		}
		if (fadeAmount < 1.f)
			fadeAmount -= 0.1f;
		if (fadeAmount < 0) {
			fadeAmount = 0, Back();
			return;
		}
	}
	virtual bool HasShutdown() { return true; }
	virtual Uint32 BackgroundColour() { return 0x203020; }
};
Screen *weaponScreen() { return new WeaponScreen; }


///////////////////////////////////////////////////////////
// The tournament edit screen
///////////////////////////////////////////////////////////
class TournEditScreen : public Menu
{
public:
	TournEditScreen() : Menu(&tournEditMenu)
	{
		switch (mpSettings.game.tournament.tType) {
		case GOLD:
		case SILVER:
		case BRONZE:
			memcpy (&CustomTournament, &mpSettings.game.tournament, sizeof (CustomTournament));
			break;
		default:
		case CUSTOM:
			break;
		case FROMVMS:
			memcpy (&CustomTournament, &mpSettings.game.tournament, sizeof (CustomTournament));
			break;
		}
		mpSettings.game.tournament.tType = CustomTournament.tType = CUSTOM;
	}
	~TournEditScreen()
	{
		bool saveKludge = (mpSettings.game.tournament.tType == LOOKUPFROMVMS);
		memcpy (&mpSettings.game.tournament, &CustomTournament, sizeof (CustomTournament));
		mpSettings.game.tournament.tType = saveKludge ? LOOKUPFROMVMS : CUSTOM;
	}
	virtual void ProcessInput(Uint32 press)
	{
		nEntries = maxEntries = 2 + CustomTournament.nGames;
		Menu::ProcessInput(press);
	}
	virtual Uint32 BackgroundColour() { return 0x203020; }
};
Screen *tournEditScreen() { return new TournEditScreen; }
\

// Enter tournament name
class EnterTournName : public EnterName
{
private:
	int vmu, slot;
public:
	EnterTournName(int vmu, int slot) : EnterName()
	{
		this->vmu = vmu;
		this->slot = slot;
	}
	virtual void Initialise()
	{
		EnterName::Initialise();
		BupLockVMS (vmu);
	}
	virtual void Finalise()
	{
		EnterName::Finalise();
		BupUnlockVMS (vmu);
	}
	virtual const char *GetTitle() const { return "`ENTTOURNNAME"; }
	virtual void Select(const char *name)
	{
		VMU *v = theVMUManager->GetVMU(vmu);
		// Huzzah; check for duplicate tournament names
		for (int i = 0; i < 4; ++i) {
			if (v->GetTournamentName(i) &&
				slot != i &&
				!strcmp (v->GetTournamentName(i), name))
				break;
		}
		if (i != 4 || 
			!strcmp (name, "Bronze") ||
			!strcmp (name, "Silver") ||
			!strcmp (name, "Gold")) {
			FE_NACK();
		} else {
			strcpy (CustomTournament.name, name);
			v->SaveTournament (slot, &CustomTournament);
			memcpy (&mpSettings.game.tournament, &CustomTournament, sizeof (CustomTournament));
			mpSettings.game.tournament.tType = LOOKUPFROMVMS;
			for (i = 0; i < 3; ++i) // Back 3 times: this screen; slot screen; edit screen;
				Back();
		}
	};
	virtual const char *GetInitialName() const
	{
		if (theVMUManager->GetVMU(vmu)->GetTournamentName(slot))
			return theVMUManager->GetVMU(vmu)->GetTournamentName(slot);
		else
			return "";
	}
};
Screen *enterTournName(void *vo) 
{ 
	int i = (int)vo;
	return new EnterTournName(i & 0xff, i>>8); 
}

// Choose a tournament to overwrite screen
class ChooseTournOverwrite : public Menu
{
private:
	int vmu, nInits;
	bool straightOn;
public:
	ChooseTournOverwrite(int vmu) : Menu(&tournOverMenu)
	{
		int nEmpty;
		this->vmu = vmu; 
		MenuEntryTemplate ents[4];
		MenuTemplate temp;
		temp.nDescriptors = 4;
		temp.templ = ents;
		nEmpty = 0;
		for (int i = 0; i < 4; ++i) {
			ents[i].clss = NULL;
			ents[i].cParms = NULL;
			ents[i].parms = NULL;
			if (theVMUManager->GetVMU(vmu)->GetTournamentName(i))
			ents[i].text = theVMUManager->GetVMU(vmu)->GetTournamentName(i);
			else {
				ents[i].text = "`EMPTY_SLOT";
				nEmpty++;
			}
		}
		Refresh (&temp);
		straightOn = (nEmpty == 4);
		nInits = 0;
	}
	virtual const char *GetTitle() const { return "`OW_TOURNAMENT"; }
	virtual void Initialise ()
	{
		nInits++;
		Menu::Initialise();
	}
	virtual void ProcessInput (Uint32 press)
	{
		if (straightOn) {
			if (nInits == 1) // Move on
				Change (ScreenID (enterTournName, (void *)((0 << 8) | vmu)));
			else {
				Back();
				return;
			}
		} else
			Menu::ProcessInput(press);
	}
	virtual bool VetoSelection(int selection) 
	{ 
		if (theVMUManager->GetVMU(vmu)->GetTournamentName(selection) == NULL)
			Change (ScreenID (enterTournName, (void *)((selection << 8) | vmu)));
		else {
			// Overwrite madness
			extern CreateFuncParm twoAnswers;
			Change (ScreenID (twoAnswers, 
				new TA_Block(
				"`OW_TOURN", 
				"`OW_TOURN_YES", 
				"`OW_TOURN_NO", 
				ScreenID(enterTournName, (void *)((selection << 8) | vmu)))));
		}
	}
};
Screen *chooseTournOverwrite(void *parm) { return new ChooseTournOverwrite((int)parm); }

////////////////////////////////////////////////////////////
// Save a tournament - woohoo!
////////////////////////////////////////////////////////////
class SaveTournScreen : public VMUSelectScreen
{
public:
	// Overridables:
	virtual const char *TitleMessage() const
	{ 
		#if USRELEASE
			return "`USTOURNSELECT";
		#else
			return "`TOURNSELECT";
		#endif
	}
	virtual void SelectVMU(int number, bool replace) 
	{
		if (replace) {
			Replace (ScreenID(chooseTournOverwrite, (void*)number));
		} else {
			Change (ScreenID(chooseTournOverwrite, (void*)number));
		}
	};

};
Screen *saveTournScreen() { return new SaveTournScreen; }


////////////////////////////////////////////////////////////
// Edit game in a tournament screen
////////////////////////////////////////////////////////////
class EditGameScreen : public Menu
{
private:
	int game, timer;
	MapNum origMap;
public:
	EditGameScreen (int parm) : Menu(&editGameMenu)
	{
		game = parm;
		CurrentTournamentGame = game;
		// Reinitialise the menu now we've set up CurrentTournGme
		Refresh(&editGameMenu);
		origMap = *GetPTMap();
	}
	virtual void ProcessInput (Uint32 press)
	{
		MapNum *ptMap = GetPTMap();
		// Filter cancels on invalid maps
		if (press & PDD_CANCEL) {
			if (MPUnlockCode[*ptMap]) {
				if ((*ptMap == origMap) || theVMUManager->IsUnlocked (*ptMap))
					Menu::ProcessInput (press);
				else
					FE_NACK();
			} else
				Menu::ProcessInput (press);
		} else
			Menu::ProcessInput (press);
	}
};
Screen *editGameScreen(void *parm)
{
	return new EditGameScreen((int)parm);
}

// Edit a game entry
class GameEditEntry : public GameTypeEntry
{
private:
	int num;
public:
	GameEditEntry (const char *text, ScreenID &sub) : GameTypeEntry(text, ScreenID(editGameScreen, (void*)sub.GetUser())) 
	{
		num = sub.GetUser();
	}
	virtual const char *GetText() 
	{
		return TGameName (&CustomTournament, num);
	}
	virtual float GetScale()
	{ return 0.55f; }
	virtual void Left() {}
	virtual void Right() {}
};
MenuEntry *gameEditMaker(const char *text, ScreenID &sub)
{
	return new GameEditEntry (text, sub);
}


// Edit number of games entry
class GameNumEntry : public GameTypeEntry
{
private:
	char buf[64];
public:
	GameNumEntry (const char *text, ScreenID &sub) : GameTypeEntry(text, sub) 
	{
	}
	virtual const char *GetText() 
	{
		sprintf (buf, "%d", CustomTournament.nGames);
		return buf;
	}
	virtual void Left() 
	{
		if (CustomTournament.nGames > 3)
			CustomTournament.nGames--;
		FE_MoveSound();
	}
	virtual void Right() 
	{
		if (CustomTournament.nGames < 6)
			CustomTournament.nGames++;
		FE_MoveSound();
	}
};
MenuEntry *gameNumMaker(const char *text, ScreenID &sub)
{
	return new GameNumEntry (text, sub);
}

// Tournament team game
class TeamGameEntry : public GameTypeEntry
{
public:
	TeamGameEntry(const char *text, ScreenID &sub) : GameTypeEntry(text, sub) 
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
		CustomTournament.team[CurrentTournamentGame] = !CustomTournament.team[CurrentTournamentGame];
		FE_MoveSound();
	}
	virtual const char *GetText()
	{
		return CustomTournament.team[CurrentTournamentGame] ? LangLookup("YES") : LangLookup("NO");
	}
	virtual bool IsSelectable ()
	{
		return (NumberOfPlayers > 3);
	}
};
MenuEntry *teamGameMaker(const char *text, ScreenID &sub)
{
	return new TeamGameEntry (text, sub);
}

// Team game
class TeamEntry : public MenuEntry
{
public:
	TeamEntry(const char *text, ScreenID &sub) : MenuEntry(text, sub) 
	{
	}
	virtual bool IsSelectable ()
	{
		return (NumberOfPlayers > 2);
	}
};
MenuEntry *teamMaker(const char *text, ScreenID &sub)
{
	return new TeamEntry (text, sub);
}

// Handicap settings
class HandicapEntry : public GameTypeEntry
{
private:
	Window	*root;
public:
	HandicapEntry(const char *text, ScreenID &sub) : GameTypeEntry(text, sub) 
	{
	}
	virtual void Left()
	{
		CurHandicap[root->GetPlayer()] = MAX(CurHandicap[root->GetPlayer()] - 10, 10);
		FE_MoveSound();
	}
	virtual void Right()
	{
		CurHandicap[root->GetPlayer()] = MIN(CurHandicap[root->GetPlayer()] + 10, 200);
		FE_MoveSound();
	}
	virtual const char *GetText()
	{
		static char text[8];
		sprintf (text, "%d%%", CurHandicap[root->GetPlayer()]);
		return text;
	}
	virtual void SetWindow (Window *rootWindow)
	{
		root = rootWindow;
	}
};
MenuEntry *handiMaker(const char *text, ScreenID &sub)
{
	return new HandicapEntry (text, sub);
}

// Always boost action
class BoostEntry : public GameTypeEntry
{
private:
	Window	*root;
public:
	BoostEntry (const char *text, ScreenID &sub) : GameTypeEntry(text, sub) 
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
		CurProfile[root->GetPlayer()].controller ^= CTRL_NOTDBL;
		FE_MoveSound();
	}
	virtual const char *GetText()
	{
		return (CurProfile[root->GetPlayer()].controller & CTRL_NOTDBL) ? LangLookup ("NO") : LangLookup("YES");
	}
	virtual void SetWindow (Window *rootWindow)
	{
		root = rootWindow;
	}
};
MenuEntry *boostMaker(const char *text, ScreenID &sub)
{
	return new BoostEntry (text, sub);
}

// Flip Y axis madness
class FlipEntry : public GameTypeEntry
{
private:
	Window	*root;
public:
	FlipEntry (const char *text, ScreenID &sub) : GameTypeEntry(text, sub) 
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
		int player = (root->GetPlayer() == -1) ? 0 : root->GetPlayer();
		CurProfile[player].controller ^= CTRL_NOTINV;
		FE_MoveSound();
	}
	virtual const char *GetText()
	{
		int player = (root->GetPlayer() == -1) ? 0 : root->GetPlayer();
		return (CurProfile[player].controller & CTRL_NOTINV) ? "`NOTFLIPPED" : "`FLIPPED";
	}
	virtual void SetWindow (Window *rootWindow)
	{
		root = rootWindow;
	}
};
MenuEntry *flipMaker(const char *text, ScreenID &sub)
{
	return new FlipEntry (text, sub);
}


///////////////////////////////////////////////////////////////
// Choose the colour of the tank
///////////////////////////////////////////////////////////////

class TankColourScreen : public Screen
{
private:
	int count;
	int colour;
public:
	TankColourScreen() : Screen()
	{
	}
	virtual void Initialise()
	{
		count = 0;
		colour = CurProfile[GetPlayer()].multiplayerStats.colour;
	}
	virtual void Finalise() 
	{
		CurProfile[GetPlayer()].multiplayerStats.colour = colour;
	}
	virtual void Draw() const
	{
		extern Uint32 FESpriteColour;
		extern float FEZ;
		int lag;

		psSetColour (COL_TEXT);
		psSetAlpha(1);
		psSetPriority(0);

		psDrawCentredFE ("`SELTANKCOLOUR", 6,  count);
		lag = (count - strlen ("`SELTANKCOLOUR"))*3;

		DrawFESprite(&tank, 92, 45, 128, 128, lag, 1, 0, 1);
		FESpriteColour = TankPalette[colour];
		FEZ = 8;
		DrawFESprite(&tankInside, 92, 45, 128, 128, lag, 1, 0, 1);
		FESpriteColour = COL_TEXT;
		FEZ = 100;
		if (DoesColourClash (colour))
			psDrawCentredFE ("`CLASH", 266, lag-128);
	}
	virtual void ProcessInput (Uint32 press)
	{
		count += 2;
		if (press & (PDD_CANCEL|PDD_OK)) {
			FE_ACK();
			Back();
		} else if (press & PDD_DGT_KL) {
			FE_MoveSound();
			colour--;
			if (colour < 0)
				colour = 6;
		} else if (press & PDD_DGT_KR) {
			FE_MoveSound();
			colour++;
			if (colour > 6)
				colour = 0;
		}
	}
};

Screen *tankColourScreen() { return new TankColourScreen; }

///////////////////////////////////////////////////////////////
// We iz ready what for to carry on innit
///////////////////////////////////////////////////////////////
class ReadyScreen : public Screen
{
private:
	int count;
public:
	ReadyScreen() : Screen()
	{
	}
	virtual void Initialise()
	{
		count = 0;
		NumReady++;
	}
	virtual void Finalise()
	{
		NumReady--;
	}
	virtual void Draw() const
	{
		int nChars = count;
		psSetAlpha(1);
		psSetPriority(0);
		psSetColour (TankPalette[CurProfile[GetPlayer()].multiplayerStats.colour]);
		psDrawCentredFE (CurProfile[GetPlayer()].name, 120-64.f, nChars);
		nChars -= strlen (CurProfile[GetPlayer()].name);
		psSetColour (COL_TEXT);
		psDrawCentredFE ("`WAITINGPLAYERS", 120, nChars);
	}
	virtual void ProcessInput (Uint32 press)
	{
		count += 2;
		if (press & PDD_CANCEL) {
			FE_ACK();
			Back();
			return;
		}
		
		int countNum = 0;
		for (int i = 0; i < 4; ++i) {
			if (Window::GetPlayerState(i) == Window::WAITING_FOR_OTHERS) {
				countNum++;
			}
		}
		if (NumReady == countNum) {
			theWindowSequence->Back();
		}
	}
	virtual Uint32 BackgroundColour() { return 0; }
	virtual bool SquashOverride() { return true; }
	virtual bool HasShutdown() { return true; }
};
Screen *readyScreen() { return new ReadyScreen; }
