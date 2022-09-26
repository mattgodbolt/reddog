#ifndef _MENUS_H
#define _MENUS_H

#include "FE.h"

extern MenuTemplate newrootMenu, chooseLevelMenu, tvOptions, toggleOptions, preProfileTemplate, gameSelect,
					newskirmishMenu, newteamMenu, newtournMenu, tournEditMenu,
					editGameMenu, tournOverMenu, mpOptionsMenu, mainMenuTemplate,
					viewWeapTemplate, pauseTemplate, bonusTemplate;

CreateFunc		adjustScreen, hzScreen, test60Screen, pressStart, 
				preProfile, mpWaitScreen, stateChanger, backStateChanger, mpSetupChanger,
				selectProfile, vmuSelectScreen, mpVMUSelectScreen, reselectChanger, exitMPChanger,
				skirmishScreen, teamScreen, tournScreen, tournEditScreen, saveTournScreen,
				weaponScreen, ginsuScreen, mpStartGame, ginsuExit,
				mpOptionChanger, mpStatsChanger, tankColourScreen, tankControlScreen,
				readyScreen, mpStatsScreen,
				newGameScreen, noVMUWarning, ovrGame, spEnterName, mainMenu,
				loadGame, missionSelectScreen, viewTechScreen, bonusScreen,
				challengeSelectScreen, pauseMenu;
#ifndef NO_PROTOTYPES
CreateFuncParm	singleVMUProfile, twoAnswers, enterNameMP, editGameScreen, chooseTournOverwrite,
				enterTournName;
EntryCreateFunc gameTypeMaker, chooseMapMaker, parmMaker, ffMaker, ttMaker, sgMaker, handiMaker, teamMaker;
EntryCreateFunc gameEditMaker, gameNumMaker, teamGameMaker, boostMaker, flipMaker, weapMaker, bonusOnOffMaker;
EntryCreateFunc initDroneMaker, stereoMaker, vibMaker;
#endif

#endif