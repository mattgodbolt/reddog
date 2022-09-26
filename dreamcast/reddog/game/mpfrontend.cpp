#include "Menus.h"
#include <math.h>
#include "MiscScreens.h"

bool mapOK = false;
// Multiplayer screens

float XXXX = 0, YYYY = 0;

///////////////////////////////////////////////
// Press start to join/insert pad to play
///////////////////////////////////////////////

class PressStart : public Screen
{
private:
	bool	valid, ignoreSound;
	int		timer;
public:
	virtual void Initialise() 
	{ 
		valid = FALSE; 
		ignoreSound = false;
		if (SpecialCheatingNumber == GetPlayer()) {
			ignoreSound = true;
			ProcessInput (PDD_DGT_ST);
			ignoreSound = false;
			SpecialCheatingNumber = -1;
		} else {
			ProcessInput(0); 
			GetRoot()->SetPlayerState(GetPlayer(), Window::NOT_PLAYING);
		}
		timer = 0; 
	}
	virtual void Finalise() {}
	virtual void Draw() const
	{
		const char *text = valid ? LangLookup("STARTTOJOIN") : LangLookup("INSERTPADPLAY");
		psSetAlpha(1);
		psSetPriority(0);
		psSetColour (COL_TEXT);
		psDrawCentredFE (text, 128, timer);
	}
	virtual void ProcessInput(Uint32 press)
	{
		valid = FindPhysicalController(GetPlayer()) ? true : false;
		timer+=2;
		if (press & PDD_DGT_ST) {
			if (!ignoreSound)
				FE_ACK();
			Change (ScreenID(preProfile));
			GetRoot()->SetPlayerState(GetPlayer(), Window::FIDDLING_AROUND);
			// Reset player options to default
			CurHandicap[GetPlayer()] = 100;
			ResetGame (CurProfile + GetPlayer());
			sprintf (CurProfile[GetPlayer()].name, "%s %d", LangLookup ("PLAYER"), GetPlayer()+1);
			CurProfile[GetPlayer()].multiplayerStats.colour = GetPlayer();
		}

		// Check for MP exit unless we haven't started up yet
		if (SpecialCheatingNumber == -1) {
			int nReady = 0, nFiddling = 0;
			for (int i = 0; i < 4; ++i) {
				switch (GetRoot()->GetPlayerState(i)) {
				case Window::WAITING_FOR_OTHERS:
					nReady++;
					break;
				case Window::FIDDLING_AROUND:
					nFiddling++;
					break;
				}
			}
			if (nFiddling == 0 && nReady == 0) {
				Change (ScreenID (backStateChanger));
			}
		}
	}
	virtual Uint32 BackgroundColour() { return 0; }
	virtual bool SquashOverride() { return true; }
};
Screen *pressStart() { return new PressStart; }

///////////////////////////////////////////////
// Pre-profile selection
///////////////////////////////////////////////

class PreProfile : public Menu
{
private:
public:
	PreProfile() : Menu (&preProfileTemplate) {}
	void Initialise()
	{
		Menu::Initialise();
		GetRoot()->UnlockProfile();
	}
	virtual bool HasShutdown() { return true; }
};
Screen *preProfile() { return new PreProfile; }

///////////////////////////////////////////////
// Wait for all of the other players to become
// ready
///////////////////////////////////////////////

class MPWaitScreen : public Screen
{
private:
	int firstTime;
	int	timer;
	bool countingDown;
	static int waitStart;
public:
	MPWaitScreen() : Screen()
	{
		firstTime = 0;
		countingDown = false;
	}
	virtual void Initialise() 
	{ 
		timer = 0;
		countingDown = false;
		firstTime++;
	}
	virtual void Finalise() 
	{
	}
	virtual void Draw() const
	{
		int nChars = timer;
		if (countingDown && currentFrame < waitStart) {
			psSetAlpha(1);
			psSetPriority(0);
			psSetColour (TankPalette[CurProfile[GetPlayer()].multiplayerStats.colour]);
			psDrawCentredFE (CurProfile[GetPlayer()].name, 128-64.f, nChars);
			nChars -= strlen (CurProfile[GetPlayer()].name);
			psSetColour (COL_TEXT);
			psDrawCentredFE ("`WAITINGPLAYERS", 128, nChars);
			psDrawCentredFE (psFormatNum (RANGE(0, 1+int((waitStart-currentFrame)/(2*FRAMES_PER_SECOND)), 5)), 128+35, nChars - strlen ("`WAITINGPLAYERS"));
		} else {
			char *text = "`WAITINGPLAYERS";
			psSetAlpha(1);
			psSetPriority(0);
			psSetColour (TankPalette[CurProfile[GetPlayer()].multiplayerStats.colour]);
			psDrawCentredFE (CurProfile[GetPlayer()].name, 128-64.f, nChars);
			nChars -= strlen (CurProfile[GetPlayer()].name);
			psSetColour (COL_TEXT);
			psDrawCentredFE (text, 128, nChars);
		}
	}
	virtual void ProcessInput(Uint32 press)
	{ 
		int i;

		if (firstTime == 1)
			Window::SetPlayerState(GetPlayer(), Window::WAITING_FOR_OTHERS);
		else {
			dAssert (firstTime == 2, "ack");
			Window::SetPlayerState(GetPlayer(), Window::FIDDLING_AROUND);
			Back();
			return;
		}

		timer+=2;
		if (press & PDD_CANCEL) {
			Window::SetPlayerState(GetPlayer(), Window::FIDDLING_AROUND);
			FE_ACK();
			Back();
			return;
		}

		int nReady = 0, nFiddling = 0, nPressStarters = 0;
		int lowestWaiter = -1;
		for (i = 0; i < 4; ++i) {
			switch (GetRoot()->GetPlayerState(i)) {
			case Window::WAITING_FOR_OTHERS:
				if (lowestWaiter == -1)
					lowestWaiter = i;
				nReady++;
				break;
			case Window::FIDDLING_AROUND:
				nFiddling++;
				break;
			default:
				if (FindPhysicalController(i))
					nPressStarters++;
				break;
			}
		}
		if (nFiddling == 0 && nReady >= 2) {
			if (GetPlayer() == lowestWaiter && nPressStarters == 0) {
				Change (ScreenID (mpSetupChanger));
			} else {
				if (!countingDown) {
					countingDown = true;
					waitStart = currentFrame + 10 * FRAMES_PER_SECOND; // five seconds
				} else {
					if (GetPlayer() == lowestWaiter && waitStart < currentFrame) {
						Change (ScreenID (mpSetupChanger));
					}
				}
			}
		} else {
			countingDown = false;
		}

	}
	virtual Uint32 BackgroundColour() { return 0; }
	virtual bool SquashOverride() { return true; }
	virtual bool HasShutdown() { return true; }
};
int MPWaitScreen::waitStart;

Screen *mpWaitScreen() { return new MPWaitScreen; }

///////////////////////////////////////////////
// A profile selection screen
///////////////////////////////////////////////
// EVIL EVIL EVIL EVIL HACK AS THE FUCKING COMPILER DOESN'T UNDERSTAND 'mutable'
bool needsRefresh[4];
class SelectProfile : public Menu
{
private:
	bool noMenu;
public:
	SelectProfile() : Menu(theVMUManager->GetProfileTemplate())
	{
		noMenu = (theVMUManager->GetProfileTemplate()->nDescriptors == 0);
//		needsRefresh[GetPlayer()] = false;  can't call GetPlayer() here!
	}
	virtual void Initialise ()
	{
		GetRoot()->UnlockProfile();
		Menu::Initialise();
		// Ensure any changes are reflected
		Refresh (theVMUManager->GetProfileTemplate(), false);
//		dPrintf ("Refreshing player %d due to reinitialisation of SelectProfile", GetPlayer());
		needsRefresh[GetPlayer()] = false;
	}
	virtual void Draw() const
	{
		if (theVMUManager->HasDataChanged())
			needsRefresh[GetPlayer()] = true;
		if (!noMenu && !needsRefresh[GetPlayer()])
			Menu::Draw();
	}
	virtual void ProcessInput (Uint32 press)
	{
		if (needsRefresh[GetPlayer()] || theVMUManager->HasDataChanged()) {
//			if (needsRefresh[GetPlayer()])
//				dPrintf ("Refreshing player %d due to needing refreshment", GetPlayer());
//			else
//				dPrintf ("Refreshing player %d due to change notification", GetPlayer());
			needsRefresh[GetPlayer()] = false;
			Refresh (theVMUManager->GetProfileTemplate(), false);
			noMenu = (theVMUManager->GetProfileTemplate()->nDescriptors == 0);
		}
		if (!noMenu)
			Menu::ProcessInput (press);
		else
			Back();
	}
};

Screen *selectProfile() { return new SelectProfile; }

//////////////////////////////////////////////////////////////
// A profile selection screen, selecting from a single VMU
//////////////////////////////////////////////////////////////
class SingleVMUProfile : public Menu
{
private:
	int VMU;
	int selectedProfile;
	static char passedOnBuf[4][64];
public:
	SingleVMUProfile(int VMU) : Menu (theVMUManager->GetVMU(VMU)->GetProfileTemplate())
	{
		selectedProfile = theVMUManager->GetVMU(VMU)->FirstFreeSlot();
		this->VMU = VMU;
		needsRefresh[0] = false;
	}
	virtual void Initialise ()
	{
		GetRoot()->UnlockProfile();
		Menu::Initialise();
		needsRefresh[0] = false;
	}
	virtual void Draw() const
	{
		if (theVMUManager->HasDataChanged())
			needsRefresh[0] = true;
		if (!needsRefresh[0])
			Menu::Draw();
	}
	virtual void ProcessInput (Uint32 press)
	{
		if (!theVMUManager->GetVMU(VMU)->Connected()) {
			Back();
			return;
		}
		if (needsRefresh[0] || theVMUManager->HasDataChanged()) {
			needsRefresh[0] = false;
			Refresh (theVMUManager->GetVMU(VMU)->GetProfileTemplate(), false);
		}
		if (selectedProfile == -1)
			Menu::ProcessInput (press);
		else {
			// Traverse onward
			GetRoot()->LockProfile(VMU, selectedProfile);
			Replace (ScreenID(enterNameMP, (void *)0xffffffff));
		}
	}
	virtual const char *GetTitle() const { return "`OWWHICHPROF"; }
	virtual bool VetoSelection(int selection)
	{
		// Doh!!!
		sprintf (passedOnBuf[GetPlayer()], LangLookup("OWWHICHPROF"), theVMUManager->GetVMU(VMU)->GetProfileName(selection));
		// Ensure we lock now to prevent race condition
		GetRoot()->LockProfile(VMU, selection);
		Replace (ScreenID(twoAnswers, new TA_Block (passedOnBuf[GetPlayer()],
			"`OW_YES", "`OW_NO", ScreenID (enterNameMP, (void *)(VMU<<8 | selection)))));

		return true;
	}
};
Screen *singleVMUProfile(void *parm) 
{ 
	return new SingleVMUProfile((int)parm); 
}
char SingleVMUProfile::passedOnBuf[4][64];

/////////////////////////////////////////
// Go back to front end
/////////////////////////////////////////

class ExitMPChanger : public Screen
{
	virtual void Initialise () {}
	virtual void Finalise () {}
	virtual void Draw() const {}
	virtual void ProcessInput (Uint32 press) 
	{
		// Unlock and flush all of the profiles out
		GetRoot()->UnlockAllProfiles();
		Change (ScreenID (backStateChanger));
	}	
};
Screen *exitMPChanger() 
{ 
	return new ExitMPChanger;
}

class SGEntry : public MenuEntry
{
public:
	SGEntry(const char *text, ScreenID &sub) : MenuEntry(text, sub) 
	{
	}
	virtual bool IsSelectable()
	{
#if GINSU
			return (!(!mapOK || (GetCurrentGameType() != DEATHMATCH && GetCurrentGameType() != TAG)));
#else
			return mapOK;
#endif
	}
};

MenuEntry *sgMaker(const char *text, ScreenID &sub)
{
	return new SGEntry(text, sub);
}


////////////////////////////////////////////
// Start a game under multiplayer
////////////////////////////////////////////
class MPStartGame : public Screen
{
private:
	int init, UnableTo;
public:
	MPStartGame() : Screen()
	{
		UnableTo = 0;
		init = 0;
	}
	virtual void Initialise () { init ++; }
	virtual void Finalise () {}
	virtual void Draw() const 
	{
		psSetColour (COL_TEXT);
		psSetPriority(0);
		psSetAlpha(1);
		if (UnableTo) 
			psDrawWordWrap ("`REINSERTCANTSTART", 64.f, 96.f, PHYS_SCR_X-16-128);
	}
	virtual void ProcessInput (Uint32 press) 
	{
		if (UnableTo) {
			UnableTo--;
			if (UnableTo == 0)
				Back();
			return;
		} 
		if (init == 2) {
			Back();
			return;
		}
		if (!ChangingState()) {
#if GINSU
			if (!mapOK || (GetCurrentGameType() != DEATHMATCH && GetCurrentGameType() != TAG)) {
#else
			if (0) {
#endif
				Back();
				return;

			} else {
				Multiplayer = TRUE;
				// Count the number of players
				NumberOfPlayers = 0;
				for (int i = 0; i < 4; ++i) {
					if (Window::GetPlayerState(i) == Window::WAITING_FOR_OTHERS)
						NumberOfPlayers++;
				}
				
				// Pre-First tournament game
				CurrentTournamentGame = -1;
				
				// Build the physical to logical pad map table
				int player = 0; 
				int pad = 0;
				PadMapTable[0] = PadMapTable[1] = PadMapTable[2] = PadMapTable[3] = -1;
				TournamentScore[0] = TournamentScore[1] = TournamentScore[2] = TournamentScore[3] = 0;
				ReleasePort(0);
				ReleasePort(1);
				ReleasePort(2);
				ReleasePort(3);
				// PadMapTable is the **PhysToLog** table well nearly
				for (i = 0; i < 4; ++i) {
					if (FindPhysicalController(i)) {
						if (Window::GetPlayerState(i) == Window::WAITING_FOR_OTHERS) {
							PadMapTable[pad] = player++;
							CapturePort(pad);
						}
						pad++;
					}
				}

				// Could we successfully capture all the ports?
				if (player != NumberOfPlayers) {
					UnableTo = 6 * FRAMES_PER_SECOND;
					FE_NACK();
				} else {
					// Yes, go on to the game
					FadeToNewState (FADE_TO_BLACK, GS_GAME);
				}
			}
		}
	}	
};
Screen *mpStartGame() { return new MPStartGame; }

//////////////////////////////////////////////
// Multiplayer statistics
//////////////////////////////////////////////

class MPStatsScreen : public Screen
{
private:
	int count, screen;
public:
	MPStatsScreen() : Screen() {}
	virtual void Initialise()
	{
		count = 0;
		screen = NUM_MPLAYER_TYPES;
	}
	virtual void Finalise() {}
	virtual void Draw() const 
	{
		int lag, nChars, i;
		char buf[64];
		int played, won;

		psSetPriority(0);
		psSetColour (COL_TEXT);
		psSetAlpha(1);

		nChars = count;
		lag = sprintf(buf, "< ~c%6x~%s~x~'s %s >", TankPalette[CurProfile[GetPlayer()].multiplayerStats.colour], CurProfile[GetPlayer()].name, LangLookup("STATS"));
		psDrawCentredFE (buf, 8, nChars);
		nChars -= (lag - 12);

		// Choose the requisite statistics to display
		if (screen == NUM_MPLAYER_TYPES) {
			psDrawCentredFE ("`OVERALL", 55, nChars);
			nChars -= strlen ("`OVERALL");
			played = won = 0;
			for (i = 0; i < NUM_MPLAYER_TYPES; ++i) {
				played += CurProfile[GetPlayer()].multiplayerStats.Game[i].played;
				won += CurProfile[GetPlayer()].multiplayerStats.Game[i].won;
			}
		} else {
			psDrawCentredFE (GameTypeName[screen], 55, nChars);
			nChars -= strlen (GameTypeName[screen]);
			played = CurProfile[GetPlayer()].multiplayerStats.Game[screen].played;
			won = CurProfile[GetPlayer()].multiplayerStats.Game[screen].won;
		}

		const char *temp;
		psDrawCentredFE ((temp = psFormat("`PLAYED %d", played)), 90, nChars);
		nChars -= strlen (temp);
		psDrawCentredFE ((temp = psFormat("`WON %d (%d%%)", won, (100*won)/played)), 125, nChars);
		nChars -= strlen (temp);

		psDrawCentredFE ("`HITSTART", 200, nChars);
		nChars -= strlen("`HITSTART");
	}
	virtual void ProcessInput(Uint32 press)
	{
		count += 2;
		if (press & PDD_DGT_ST) {
			Change (ScreenID(readyScreen));
		} else if (press & PDD_DGT_KL) {
			screen--;
			if (screen < 0)
				screen = NUM_MPLAYER_TYPES;
		} else if (press & PDD_DGT_KR) {
			screen++;
			if (screen > NUM_MPLAYER_TYPES)
				screen = 0;
		}
	}
	virtual Uint32 BackgroundColour() { return 0x203020; }
};
Screen *mpStatsScreen() { return new MPStatsScreen; }


////////////////////////////////////////////////////////
// Control madness
////////////////////////////////////////////////////////
static const struct {
	char *a, *b, *x;
} CtrlName[] = {
#define F "`FIRE"
#define S "`SHIELD"
#define D "`PICKUPFIRE"

	{ F, D, S },
	{ F, S, D },
	{ S, D, F },
	{ D, S, F },
	{ S, F, D },
	{ D, F, S }	
};
#undef F
#undef S
#undef D

class TankControlScreen : public Screen
{
private:
	int count;
	bool Changed;
public:
	TankControlScreen() : Screen()
	{
	}
	virtual void Initialise ()
	{
		count = 0;
		Changed = false;
	}
	virtual void Finalise() 
	{ 
		if (Changed && GetPlayer() >= 0) {
			GetRoot()->WriteProfile();
		}
	}
	virtual void Draw() const 
	{
		float top, left, scale, xAdd, yAdd;
		Matrix32 mat;
		int pNum = GetPlayer() >= 0 ? GetPlayer() : 0;

		if (GetPlayer() == -1) {
			scale = 1.7f;
			xAdd = yAdd = 0;
		} else {
			float w[4];
			scale = 1.f;
			psGetWindow(w);
			xAdd = w[1] - 8;
			yAdd = w[0] - 32;
		}
		memcpy (&mat, psGetMatrix(), sizeof (mat));

		psSetPriority(0);
		psSetAlpha(1);
		psSetColour (COL_TEXT);

		left = 16 + 8;
		top = (GetPlayer() == -1) ? 120 : 74;

		psDrawCentredFE (psFormat ("`CONFIGN", (CurProfile[pNum].controller & CTRL_MASK)+1), 8*scale, count);
		int lag = (count - 15)*3;
		DrawFESprite (&padMat, (left - 8), (top - 32), 128*scale, 128*scale, lag, 1, 0, 1);
		lag -= 128;

		float LinePos = 196;
		float SmallGap = 24;

		lag *= 3;
		if (GetPlayer() == -1) {
			// Y button
			DrawLineFE (xAdd + left + 194/2*scale, yAdd + top+90/2*scale, xAdd + left+194/2*scale, yAdd + top+(90/2 - SmallGap)*scale, SmallGap*scale, lag, 1);
			lag -= SmallGap;
			DrawLineFE (xAdd + left+194/2*scale, yAdd + top+(90/2-SmallGap)*scale, xAdd + LinePos*scale, yAdd + top+(90/2-SmallGap)*scale, (LinePos-(16+194/2))*scale, lag, 1);
			lag -= LinePos-(left+194/2);
			
			mPostTranslate32 (psGetMatrix(), LinePos*scale, top - 32 + (90/2 - SmallGap - 10)*scale);
			psDrawStringFE ("`CHARGE", 0, 0, lag);
			lag -= strlen("`CHARGE");
		}

		// B button
		DrawLineFE (xAdd + left + 215/2*scale, yAdd + top+110/2*scale, xAdd + LinePos*scale, yAdd + top+110/2*scale, (LinePos-(left+215/2))*scale, lag, 1);
		lag -= LinePos-(left+215/2);

		memcpy (psGetMatrix(), &mat, sizeof (mat));
		mPostTranslate32 (psGetMatrix(), LinePos*scale, top - 32 +(110/2- 10)*scale);
		psDrawStringFE (CtrlName[CurProfile[pNum].controller & CTRL_MASK].b, 0, 0, lag);
		lag -= strlen(CtrlName[CurProfile[pNum].controller & CTRL_MASK].b);

		// A button
		DrawLineFE (xAdd + left + 194/2*scale, yAdd + top+131/2*scale, xAdd + left+194/2*scale, yAdd + top+(131/2 + SmallGap)*scale, SmallGap*scale, lag, 1);
		lag -= SmallGap;
		DrawLineFE (xAdd + left+194/2*scale, yAdd + top+(131/2+SmallGap)*scale, xAdd + LinePos*scale, yAdd + top+(131/2+SmallGap)*scale, (LinePos-(16+194/2))*scale, lag, 1);
		lag -= LinePos-(left+194/2);

		memcpy (psGetMatrix(), &mat, sizeof (mat));
		mPostTranslate32 (psGetMatrix(), LinePos*scale, top - 32 + (131/2 + SmallGap - 10)*scale);
		psDrawStringFE (CtrlName[CurProfile[pNum].controller & CTRL_MASK].a, 0, 0, lag);
		lag -= strlen(CtrlName[CurProfile[pNum].controller & CTRL_MASK].a);

		// X button
		DrawLineFE (xAdd + left + 173/2*scale, yAdd + top+110/2*scale, xAdd + left+173/2*scale, yAdd + top+(110/2 + 64)*scale, 64*scale, lag, 1);
		lag -= 64;
		DrawLineFE (xAdd + left+(173/2)*scale, yAdd + top+(110/2+64)*scale, xAdd + LinePos*scale, yAdd + top+(110/2+64)*scale, (LinePos-(16+173/2))*scale, lag, 1);
		lag -= LinePos-(left+173/2);

		memcpy (psGetMatrix(), &mat, sizeof (mat));
		mPostTranslate32 (psGetMatrix(), LinePos*scale, top-32+(110/2+64- 10)*scale);
		psDrawStringFE (CtrlName[CurProfile[pNum].controller & CTRL_MASK].x, 0, 0, lag);
		lag -= strlen(CtrlName[CurProfile[pNum].controller & CTRL_MASK].x);

		// Flippedness on single player
		if (GetPlayer() == -1) {
			char buf[64];
			int l = sprintf (buf, "^@ : %s : ", LangLookup ("FLIPYAXIS"));
			mUnit32 (psGetMatrix());
			strcpy (&buf[l], LangLookup ((CurProfile[0].controller & CTRL_NOTINV) ? "NOTFLIPPED" : "FLIPPED"));
			psDrawCentredFE (buf, 350, lag);
		}

	}
	virtual void ProcessInput (Uint32 press)
	{
		int pNum = GetPlayer() >= 0 ? GetPlayer() : 0;
		if (press & (PDD_CANCEL|PDD_OK)) {
			FE_ACK();
			Back();
			return;
		} else if (press & PDD_DGT_KR) {
			int mask = CurProfile[pNum].controller & CTRL_MASK;
			mask ++;
			if (mask >= 6)
				mask = 0;
			CurProfile[pNum].controller = (CurProfile[pNum].controller &~ CTRL_MASK) | mask;
			Changed = true;
			FE_MoveSound();
		} else if (press & PDD_DGT_KL) {
			int mask = CurProfile[pNum].controller & CTRL_MASK;
			mask --;
			if (mask < 0)
				mask = 5;
			CurProfile[pNum].controller = (CurProfile[pNum].controller &~ CTRL_MASK) | mask;
			Changed = true;
			FE_MoveSound();
		} else if (GetPlayer() == -1) {
			if (press & (PDD_DGT_KU|PDD_DGT_KD)) {
				CurProfile[0].controller ^= CTRL_NOTINV;
				FE_MoveSound();
				Changed = true;
			}
		}

		count+=2;
	}
	virtual Uint32 BackgroundColour() { return 0x203020; }
};
Screen *tankControlScreen() { return new TankControlScreen; }


extern "C" int FindRandMap(int gameType)
{
	int retVal;
	bool unlocked;
	do {
		retVal = rand() % numMPMaps;
		unlocked = theVMUManager->IsUnlocked (retVal);
	} while (!(MPLevOK[retVal] & (1<<gameType)) ||
		     !unlocked);
	return retVal;
}

