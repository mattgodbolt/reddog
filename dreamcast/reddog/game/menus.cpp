// All the Menus in one place - innit nice!
#include "Menus.h"

//////////////////////////////////////////////////////////
// Definitions of the functions
////////////////////////////////////////////////////////// 

// Deal with the Hz swapping
class HZEntry : public MenuEntry
{
private:
	char		textBuf[64];
	const char	*origText;
public:
	HZEntry(const char *text, ScreenID &sub) : MenuEntry(textBuf, sub)
	{
		if (text[0] != '`')
			origText = text;
		else
			origText = LangLookup(text+1);
	}

	virtual float Draw (float selAmt = 0.f, float alpha = 1.f, float nChars = 10000.f)
	{
		sprintf (textBuf, origText, PAL ? 60:50);
		MenuEntry::Draw (selAmt, alpha, nChars);
	}

	virtual ScreenID *Selected()
	{
		// *gulp* Restart everything
		ShutdownFrontEnd();
		rShutdownRenderer();
		PAL = !PAL;
		sbReInitMode (PAL ? KM_DSPMODE_PALNI640x480EXT : KM_DSPMODE_NTSCNI640x480);
		rInitRenderer(TRUE, FE_TEXRAM);
		InitialiseFrontEnd();
		theWindowSequence->Recreate();
		return NULL;
	}
};

MenuEntry *hzMaker (const char *text, ScreenID &sub)
{
	return new HZEntry (text, sub);
}

// Deal with monitors
class SwitchEntry : public MenuEntry
{
public:
	SwitchEntry(const char *text, ScreenID &sub) : MenuEntry(text, sub) {}
	virtual bool IsSelectable()
	{
		return (syCblCheckCable() != SYE_CBL_CABLE_VGA);
	}
};
MenuEntry *switchMaker(const char *text, ScreenID &sub)
{
	return new SwitchEntry (text, sub);
}

// Multiplayer selection
class MPEntry : public MenuEntry
{
public:
	MPEntry(const char *text, ScreenID &sub) : MenuEntry(text, sub) {}
	virtual bool IsSelectable()
	{
		return (NumberOfControllers > 1);
	}
};
MenuEntry *mpEntryMaker(const char *text, ScreenID &sub)
{
	return new MPEntry (text, sub);
}

// Level warp selection
class LevEntry : public MenuEntry
{
public:
	LevEntry(const char *text, ScreenID &sub) : MenuEntry(text, sub) {}
	virtual ScreenID *Selected()
	{
		LevelNum = subMenu.GetUser();
		Cheating = TRUE;
		player[0].handicap = 1.f;
		NumberOfPlayers = 1;
		Multiplayer = 0;
			 			extern int LogToPhysTab[4];

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
		return NULL;
	}
};

MenuEntry *levMaker(const char *text, ScreenID &sub)
{
	return new LevEntry (text, sub);
}

// Only allow profile selection if there are in fact profiles
class SelProf : public MenuEntry
{
public:
	SelProf(const char *text, ScreenID &sub) : MenuEntry(text, sub) {}
	virtual bool IsSelectable()
	{
		if (theVMUManager->GetProfileTemplate()->nDescriptors == 0)
			return false;
		else
			return true;
	}
};
MenuEntry *selProfMaker(const char *text, ScreenID &sub)
{
	return new SelProf (text, sub);
}

// Only allow profile creation if there are in fact vmus attached
class CreateProf : public MenuEntry
{
public:
	CreateProf(const char *text, ScreenID &sub) : MenuEntry(text, sub) {}
	virtual bool IsSelectable()
	{
		for (int i = 0; i < 8; ++i) {
			if (theVMUManager->GetVMU(i)->Connected())
				return true;
		}
		return false;
	}
};
MenuEntry *createProfMaker(const char *text, ScreenID &sub)
{
	return new CreateProf (text, sub);
}

// Permanently unselectable menu
class GreyedOutEntry : public MenuEntry
{
public:
	GreyedOutEntry(const char *text, ScreenID &sub) : MenuEntry(text, sub) {}
	virtual bool IsSelectable()
	{ return false; }
};
MenuEntry *greyOutMaker(const char *text, ScreenID &sub)
{
	return new GreyedOutEntry (text, sub);
}

// Bonus features menu
class BonusEntry : public MenuEntry
{
public:
	BonusEntry(const char *text, ScreenID &sub) : MenuEntry(text, sub) {}
	virtual bool IsSelectable()
	{ return ((CurProfile[0].unlockedSpecials & (SPECIAL_CHEATING_MASK | SPECIAL_TRIPPY_TRAILS | SPECIAL_60HZRACE)) != 0); }
};
MenuEntry *bonusMaker(const char *text, ScreenID &sub)
{
	return new BonusEntry (text, sub);
}

// Exit the single player 
class ExitEntry : public MenuEntry
{
public:
	ExitEntry(const char *text, ScreenID &sub) : MenuEntry(text, sub) {}
	virtual ScreenID *Selected()
	{
		theWindowSequence->Reset(true);
	}
};
MenuEntry *meExitHole(const char *text, ScreenID &sub)
{
	return new ExitEntry (text, sub);
}

// Volume handlers
class VolumeHandler : public MenuEntry
{
public:
	VolumeHandler(const char *text, ScreenID &sub) : MenuEntry(text, sub) {}
	float Draw(float selAmt, float alpha, float nChars)
	{
		char buf[11];
		int nBits = (int)((GetVolume() * 10.f + 0.5f));
		float retVal = MenuEntry::Draw(selAmt, alpha, nChars);
		nChars -= retVal;
		buf[10] = '\0';
		if (nBits) {
			memset(buf, '*', 10);
			buf[nBits] = '\0';
			psSetColour(0xff0000);
			psSetPriority(1);
			psDrawStringFE (buf, 300, 0, nChars);
			retVal += nBits;
			nChars -= nBits;
		}
		memset(buf, '*', 10);
		psSetPriority(0);
		psSetColour(COL_TEXT);
		psDrawStringFE (buf, 300, 0, nChars);
		retVal += nBits;
		nChars -= nBits;
		return retVal;
	}

	virtual void Left()
	{
		int nBits = (int)((GetVolume() * 10.f) + 0.5f), nOldBits;
		nOldBits = nBits;
		nBits = MAX(nBits-1, 0);
		SetVolume (nBits * 0.1f);
		if (nOldBits != nBits)
			FE_MoveSound();
		else
			FE_NACK();
	}

	virtual void Right()
	{
		int nBits = (int)((GetVolume() * 10.f) + 0.5f), nOldBits;
		nOldBits = nBits;
		nBits = MIN(nBits+1, 10);
		SetVolume (nBits * 0.1f);
		if (nOldBits != nBits)
			FE_MoveSound();
		else
			FE_NACK();
	}

	// Overridable:
	virtual float GetVolume() const = 0;
	virtual void SetVolume(float f) const = 0;
};
class FXVolumeHandler : public VolumeHandler
{
public:
	FXVolumeHandler(const char *text, ScreenID &sub) : VolumeHandler(text, sub) {}
	virtual float GetVolume() const { return sfxGetFXVolume(); }
	virtual void SetVolume(float f) const { sfxSetFXVolume(f); }
};
MenuEntry *fxVolumeMaker(const char *text, ScreenID &sub)
{
	return new FXVolumeHandler (text, sub);
}

class CDVolumeHandler : public VolumeHandler
{
public:
	CDVolumeHandler(const char *text, ScreenID &sub) : VolumeHandler(text, sub) {}
	virtual float GetVolume() const { return sfxGetCDVolume(); }
	virtual void SetVolume(float f) const { sfxSetCDVolume(f); }
};
MenuEntry *musicVolumeMaker(const char *text, ScreenID &sub)
{
	return new CDVolumeHandler (text, sub);
}

// Difficulty madness
class DifEntry : public MenuEntry
{
public:
	DifEntry(const char *text, ScreenID &sub) : MenuEntry(text, sub) 
	{
	}
	virtual ScreenID *Selected()
	{
		GameDifficulty = 1;
		return MenuEntry::Selected();
	}
};
MenuEntry *difMaker(const char *text, ScreenID &sub)
{
	return new DifEntry (text, sub);
}

//////////////////////////////////////////////////////////
// The Menus themselves :
//////////////////////////////////////////////////////////

BEGIN_MENU(newrootMenu)
#if !GINSU && !GODMODEMACHINE
ADD_OPTION ("Choose Level", NULL, &chooseLevelMenu)
#endif
#if GINSU
ADD_SUBSCREEN ("New Game", NULL, ginsuScreen)
ADD_OPTION ("Load Game", greyOutMaker, NULL)
#else
ADD_SUBSCREEN ("`NEWGAME", NULL, newGameScreen)
ADD_SUBSCREEN ("`LOADGAME", selProfMaker, loadGame)
#endif
ADD_SUBSCREEN ("`MULTIPLAYER", mpEntryMaker, stateChanger)
ADD_OPTION ("`TVOPTIONS", NULL, &tvOptions)
#if GINSU
ADD_SUBSCREEN ("Exit", NULL, ginsuExit)
#endif
END_MENU(newrootMenu)

BEGIN_MENU(chooseLevelMenu)
#if GODMODE
ADD_OPTION ("VOLCANO", levMaker, (MenuTemplate *)0)
ADD_OPTION ("ICE", levMaker, (MenuTemplate *)1)
ADD_OPTION ("CANYON", levMaker, (MenuTemplate *)2)
ADD_OPTION ("CITY", levMaker, (MenuTemplate *)3)
ADD_OPTION ("HYDRO", levMaker, (MenuTemplate *)4)
ADD_OPTION ("ALIEN", levMaker, (MenuTemplate *)5)
ADD_OPTION ("CHALLENGE 1", levMaker, (MenuTemplate *)6)
ADD_OPTION ("CHALLENGE 2", levMaker, (MenuTemplate *)7)
ADD_OPTION ("CHALLENGE 3", levMaker, (MenuTemplate *)8)
ADD_OPTION ("CHALLENGE 4", levMaker, (MenuTemplate *)9)
ADD_OPTION ("CHALLENGE 5", levMaker, (MenuTemplate *)10)
ADD_OPTION ("CHALLENGE 6", levMaker, (MenuTemplate *)11)
ADD_OPTION ("CHALLENGE 7", levMaker, (MenuTemplate *)12)
#else
ADD_OPTION ("DAVET", levMaker, (MenuTemplate *)14)
ADD_OPTION ("SEFTON", levMaker, (MenuTemplate *)15)
ADD_OPTION ("NICKC", levMaker, (MenuTemplate *)16)
ADD_OPTION ("DAVIDH", levMaker, (MenuTemplate *)17)		
ADD_OPTION ("JEFF", levMaker, (MenuTemplate *)18)
ADD_OPTION ("MARK", levMaker, (MenuTemplate *)19)
ADD_OPTION ("DES", levMaker, (MenuTemplate *)20)
ADD_OPTION ("LEON", levMaker, (MenuTemplate *)21)
ADD_OPTION ("SOO", levMaker, (MenuTemplate *)22)
ADD_OPTION ("MEL", levMaker, (MenuTemplate *)23)
ADD_OPTION ("MATTPLEVEL", levMaker, (MenuTemplate *)24)
ADD_OPTION ("SAVLEVEL", levMaker, (MenuTemplate *)25)
ADD_OPTION ("CARL (SAVLEVEL2)", levMaker, (MenuTemplate *)26)
ADD_OPTION ("MOOG", levMaker, (MenuTemplate *)27)
#endif
END_MENU(chooseLevelMenu)

BEGIN_MENU(tvOptions)
ADD_OPTION ("`FXVOLUME", fxVolumeMaker, NULL)
ADD_OPTION ("`MUSICVOLUME", musicVolumeMaker, NULL)
ADD_OPTION ("`SOUNDTYPE", stereoMaker, NULL)
ADD_OPTION ("`VIBRATION", vibMaker, NULL)
ADD_SUBSCREEN ("`ADJUSTSCREEN", NULL, adjustScreen)
ADD_SUBSCREEN ("`SWITCH50", switchMaker, hzScreen)
END_MENU(tvOptions)

BEGIN_MENU(toggleOptions)
ADD_SUBSCREEN ("`TEST60", NULL, test60Screen)
ADD_OPTION ("`RUN60", hzMaker, NULL)
END_MENU(toggleOptions)

// Multiplayer menus
BEGIN_MENU(preProfileTemplate)
ADD_SUBSCREEN("`JUSTPLAY", NULL, mpWaitScreen)
#if GINSU
ADD_OPTION("Select profile", greyOutMaker, NULL)
ADD_OPTION("Create profile", greyOutMaker, NULL)
#else
ADD_SUBSCREEN("`SELPROF", selProfMaker, selectProfile)
ADD_SUBSCREEN("`CREATEPROF", createProfMaker, mpVMUSelectScreen)
#endif
END_MENU(preProfileTemplate)


BEGIN_MENU(gameSelect)
ADD_SUBSCREEN ("`SKIRMISH", NULL, skirmishScreen)
#if GINSU
ADD_OPTION ("Team", greyOutMaker, NULL)
ADD_OPTION ("Tournament", greyOutMaker, NULL)
ADD_OPTION ("Player Options", greyOutMaker, NULL)
ADD_OPTION ("Player Statistics", greyOutMaker, NULL)
#else
ADD_SUBSCREEN ("`TEAM", teamMaker, teamScreen)
ADD_SUBSCREEN ("`TOURNAMENT", NULL, tournScreen)
ADD_SUBSCREEN ("`PLAYEROPTS", NULL, mpOptionChanger)
ADD_SUBSCREEN ("`PLAYERSTATS", NULL, mpStatsChanger)
#endif
ADD_SUBSCREEN ("`RESELECTPLAYERS", NULL, reselectChanger)
ADD_SUBSCREEN ("`EXITMP", NULL, exitMPChanger)
END_MENU(gameSelect)

BEGIN_MENU(newskirmishMenu)
ADD_SUBSCREEN ("`STARTGAME", sgMaker, mpStartGame)
ADD_OPTION ("`GAMETYPE", gameTypeMaker, NULL)
ADD_OPTION ("`CHOOSEMAP", chooseMapMaker, NULL)
ADD_OPTION ("<parm>", parmMaker, NULL)
ADD_SUBSCREEN ("`WEPPS", NULL, weaponScreen)
END_MENU(newskirmishMenu)

BEGIN_MENU(newteamMenu)
ADD_SUBSCREEN ("`STARTGAME", sgMaker, mpStartGame)
ADD_OPTION ("`GAMETYPE", gameTypeMaker, NULL)
ADD_OPTION ("`CHOOSEMAP", chooseMapMaker, NULL)
ADD_OPTION ("<parm>", parmMaker, NULL)
ADD_SUBSCREEN ("`WEPPS", NULL, weaponScreen)
ADD_OPTION ("`TEAMFIRE", ffMaker, NULL)
END_MENU(newteamMenu)

BEGIN_MENU(newtournMenu)
ADD_SUBSCREEN ("`STARTTOURN", NULL, mpStartGame)
ADD_OPTION ("`TTYPE", ttMaker, NULL)
ADD_SUBSCREEN ("`TEDIT", NULL, tournEditScreen)
ADD_OPTION ("`TEAMFIRE", ffMaker, NULL)
ADD_SUBSCREEN ("`WEPPS", NULL, weaponScreen)
END_MENU(newtournMenu)

BEGIN_MENU(tournEditMenu)
ADD_OPTION ("`GAMENUMBER", gameNumMaker, NULL)
ADD_SUBSCREEN ("`SAVETOURN", createProfMaker, saveTournScreen)
ADD_OPTION ("`GAME1", gameEditMaker, (MenuTemplate *)0)
ADD_OPTION ("`GAME2", gameEditMaker, (MenuTemplate *)1)
ADD_OPTION ("`GAME3", gameEditMaker, (MenuTemplate *)2)
ADD_OPTION ("`GAME4", gameEditMaker, (MenuTemplate *)3)
ADD_OPTION ("`GAME5", gameEditMaker, (MenuTemplate *)4)
ADD_OPTION ("`GAME6", gameEditMaker, (MenuTemplate *)5)
END_MENU(tournEditMenu)

BEGIN_MENU(editGameMenu)
ADD_OPTION ("`GAMETYPE", gameTypeMaker, NULL)
ADD_OPTION ("`CHOOSEMAP", chooseMapMaker, NULL)
ADD_OPTION ("<parm>", parmMaker, NULL)
ADD_OPTION ("`TEAMGAME", teamGameMaker, NULL)
END_MENU(editGameMenu)

BEGIN_MENU(tournOverMenu)
ADD_OPTION ("One - ha ha ha", NULL, NULL)
ADD_OPTION ("Two - ha ha ha", NULL, NULL)
ADD_OPTION ("Three - ha ha ha", NULL, NULL)
ADD_OPTION ("Four - ha ha ha, I love to count", NULL, NULL)
END_MENU(tournOverMenu)

BEGIN_MENU(mpOptionsMenu)
ADD_SUBSCREEN("`READY", NULL, readyScreen)
ADD_SUBSCREEN("`CONTROLS", NULL, tankControlScreen)
ADD_OPTION ("`FLIPYAXIS", flipMaker, NULL)
ADD_SUBSCREEN("`TANKCOLOUR", NULL, tankColourScreen)
ADD_OPTION("`HANDICAP", handiMaker, NULL)
ADD_OPTION("`ALWAYSBOOST", boostMaker, NULL)
END_MENU(mpOptionsMenu)

BEGIN_MENU(mainMenuTemplate)
ADD_SUBSCREEN("`SELMISSION", NULL, missionSelectScreen)
ADD_SUBSCREEN("`SELHARD", difMaker, missionSelectScreen)
ADD_SUBSCREEN("`CHALMISSION", NULL, challengeSelectScreen)
ADD_SUBSCREEN("`VIEWTECH", NULL, viewTechScreen)
ADD_SUBSCREEN("`BONUSFEATURES", bonusMaker, bonusScreen)
ADD_SUBSCREEN("`CTRLCONFIG", NULL, tankControlScreen)
ADD_OPTION ("`TVOPTIONS", NULL, &tvOptions)
ADD_OPTION ("`EXIT", meExitHole, NULL)
END_MENU(mainMenuTemplate)

BEGIN_MENU(viewWeapTemplate)
ADD_OPTION ("`GUN", weapMaker, (MenuTemplate *)0)
ADD_OPTION ("`MISSILE", weapMaker, (MenuTemplate *)1)
ADD_OPTION ("`ARMOUR", weapMaker, (MenuTemplate *)2)
ADD_OPTION ("`SHIELD", weapMaker, (MenuTemplate *)3)
END_MENU(viewWeapTemplate)

BEGIN_MENU(pauseTemplate)
ADD_OPTION ("`RETGAME", NULL, NULL)
ADD_OPTION ("`FXVOLUME", fxVolumeMaker, NULL)
ADD_OPTION ("`MUSICVOLUME", musicVolumeMaker, NULL)
ADD_OPTION ("`SOUNDTYPE", stereoMaker, NULL)
ADD_OPTION ("`VIBRATION", vibMaker, NULL)
ADD_OPTION ("`RESTARTLEV", NULL, NULL)
ADD_OPTION ("`EXITLEV", NULL, NULL)
END_MENU(pauseTemplate)

BEGIN_MENU(bonusTemplate)
ADD_OPTION ("`TRIPPYTRAILS", bonusOnOffMaker, (MenuTemplate *)SPECIAL_TRIPPY_TRAILS)
ADD_OPTION ("`PERMBOOST", bonusOnOffMaker, (MenuTemplate *)SPECIAL_PERMANENT_BOOST)
ADD_OPTION ("`PERMSHIELD", bonusOnOffMaker, (MenuTemplate *)SPECIAL_INFINITE_SHIELD)
ADD_OPTION ("`BOT1", bonusOnOffMaker, (MenuTemplate *)SPECIAL_ASSISTANCE_BOT_1)
ADD_OPTION ("`BOT2", bonusOnOffMaker, (MenuTemplate *)SPECIAL_ASSISTANCE_BOT_2)
ADD_OPTION ("`INFWEAPCHARGE", bonusOnOffMaker, (MenuTemplate *)SPECIAL_INF_WEAPON_CHARGE)
ADD_OPTION ("`INVULN", bonusOnOffMaker, (MenuTemplate *)SPECIAL_INVULNERABILITY)
ADD_OPTION ("`CHAL60", bonusOnOffMaker, (MenuTemplate *)SPECIAL_60HZRACE)
ADD_OPTION ("`INITDRONE", initDroneMaker, NULL)
END_MENU(bonusTemplate)
