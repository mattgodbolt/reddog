/*
 * MapLoad.c
 * Loads a .RDM format file
 */

#include "RedDog.h"
#include "Memory.h"
/*#include "Map.h"*/
/*#include "MapLoad.h"*/
#include "Dirs.h"
#include "view.h"
#include "strat.h"
extern LEVELSETTINGS *LevelSettings;

short LevelNum = 0;
#if MUSIC
	char *Levels[] = { "MUSIC"};
# else
		/*
		 * EDIT ME HERE for the final level WAD names:
		 */
		char *Levels[] = {
			// Normal levels
			"VOLCANO",
			"ICE",
			"CANYON",
			"HYDRO",
			"CITY",
			"ALIEN",
			// Challenge levels
			"CHAL1",
			"CHAL2",
			"CHAL3",
			"CHAL4",
			"CHAL5",
			"CHAL6",
			"CHAL7",
			"CREDITS",
#ifndef FINAL
			"DAVET",
			"SEFTON",
			"NICKC",
			"DAVIDH",
			"JEFF",
			"MARK",
			"DES",
			"LEON",
			"SOO",
			"MEL",
			"MATTPLEVEL",
			"SAVLEVEL",
			"SAVLEVEL2",
			"MOOG"
#endif
		};
#endif
char *SkillLevels[] = {	"EASY","HARD"};

float GlobalFogRed, GlobalFogGreen, GlobalFogBlue;
float TargetGlobalFogRed, TargetGlobalFogGreen, TargetGlobalFogBlue;
Uint8 GlobalAmbientLightRed, GlobalAmbientLightGreen, GlobalAmbientLightBlue;
float TargetGlobalFogNear, GlobalFogNear, GlobalFogFar, TargetGlobalFogFar, GlobalFieldOfView, GlobalFarZ, TargetGlobalFarZ, GlobalAmbientLightIntensityScape, GlobalAmbientLightIntensityStrat;
float GlobalFogDensity, TargetGlobalFogDensity;
short NumLevels = asize (Levels);


/*MAPINIT WILL SET UP A LEVELSETTINGS RECORD HOLDING THE START VALUES OF */
/*THE VARS SET BELOW. IF NONE HAVE BEEN SET FOR THE LEVEL, IN MAX, THEN DEFAULT */
/*VALUES WILL BE SET */

void SetLevelVars(LEVELSETTINGS *Map)
{
	if (Map)
	{

		GlobalAmbientLightRed = Map->ambientred;
		GlobalAmbientLightGreen = Map->ambientgreen;
		GlobalAmbientLightBlue = Map->ambientblue;
		
		GlobalAmbientLightIntensityScape = Map->scapeintensity;
		GlobalAmbientLightIntensityStrat = Map->stratintensity;

		GlobalFogRed	= TargetGlobalFogRed = Map->fogred / 255.f;
		GlobalFogGreen	= TargetGlobalFogGreen = Map->foggreen / 255.f;
		GlobalFogBlue	= TargetGlobalFogBlue = Map->fogblue / 255.f;
		GlobalFogNear	= TargetGlobalFogNear = Map->fognear;
		GlobalFogFar	= TargetGlobalFogFar = Map->fogfar;

		GlobalFieldOfView = Map->fov;
		GlobalFarZ = TargetGlobalFarZ = Map->farz;
	}
	else
		dAssert(0, "SetLevel Flags in max");


	Map->NormalCamDist[THIRD_PERSON_FAR] = -12.0f;
	Map->NormalCamHeight[THIRD_PERSON_FAR] = 2.0;
	Map->NormalCamlookHeight[THIRD_PERSON_FAR] =3.0f;

	Map->NormalCamDist[THIRD_PERSON_CLOSE] = -7.5f;
	Map->NormalCamHeight[THIRD_PERSON_CLOSE] = 1.5f;
	Map->NormalCamlookHeight[THIRD_PERSON_CLOSE] =1.4f;

	Map->NormalCamDist[FIRST_PERSON] = 0.0f;
	Map->NormalCamHeight[FIRST_PERSON] = 0.6f;
	Map->NormalCamlookHeight[FIRST_PERSON] =1.7f;

	Map->SideStepCamDist[THIRD_PERSON_CLOSE] = -7.5f;
	Map->SideStepCamHeight[THIRD_PERSON_CLOSE] = 1.5f;
	Map->SideStepCamlookHeight[THIRD_PERSON_CLOSE] =1.4f;


}

extern float TargetDomeFadeAmount;
extern float DomeFadeAmount;

void RestartEnvParams(int getOrSet)
{
	static float savedFogRed, savedFogBlue, savedFogGreen,
				 savedFarZ, savedFogNear, savedFogFar, savedDomeFade;
	if (getOrSet) {
		GlobalFogRed	= TargetGlobalFogRed = savedFogRed;
		GlobalFogGreen	= TargetGlobalFogGreen = savedFogGreen;
		GlobalFogBlue	= TargetGlobalFogBlue = savedFogBlue;
		GlobalFogNear	= TargetGlobalFogNear = savedFogNear;
		GlobalFogFar	= TargetGlobalFogFar = savedFogFar;

		GlobalFarZ = TargetGlobalFarZ = savedFarZ;
		DomeFadeAmount = TargetDomeFadeAmount = savedDomeFade;
	} else {
		savedFogRed = TargetGlobalFogRed;
		savedFogGreen = TargetGlobalFogGreen;
		savedFogBlue = TargetGlobalFogBlue;
		savedFogNear = TargetGlobalFogNear;
		savedFogFar = TargetGlobalFogFar;
		savedFarZ = TargetGlobalFarZ;
		savedDomeFade = TargetDomeFadeAmount;

	}
}

#define DOIT(a) a += (Target##a - a) * 0.05f
void UpdateEnvironmentParms(Camera *thisCam)
{
	DOIT(GlobalFogRed);
	DOIT(GlobalFogGreen);
	DOIT(GlobalFogBlue);
	DOIT(GlobalFogFar);
	DOIT(GlobalFogNear);
   //	DOIT(GlobalFogFar);
	DOIT(GlobalFarZ);
	DOIT(GlobalFogDensity);
	
	// Check for sensibility on the fog
	if (GlobalFogNear >= GlobalFogFar)
		GlobalFogNear = GlobalFogFar-1;

	thisCam->nearZ				= 0.25f;
	thisCam->farZ				= GlobalFarZ;

	thisCam->fogNearZ			= GlobalFogNear;
	thisCam->fogColour.colour	= (0xff<<24)|(((int)(GlobalFogRed*255.f))<<16)|(((int)(GlobalFogGreen*255.f))<<8)|(((int)(GlobalFogBlue*255.f)));
	
	if (!Multiplayer || currentFrame < 5)
		thisCam->screenAngle	= GlobalFieldOfView;
}

void setLevelCam(Camera *thisCam)
{
	TargetDomeFadeAmount = DomeFadeAmount = 1.f;
	TargetGlobalFogDensity = GlobalFogDensity = 1.f;
	thisCam->pos.x = thisCam->pos.y = thisCam->pos.z = 0.f;
	UpdateEnvironmentParms(thisCam);
}

// < 0 leaves it as it is
//global param int value of 99 passed in will make values immediate
void SetEnvironmentParms(float nearFog, float farFog, float farCam, int DomeOn,
						 float fogRed, float fogGreen, float fogBlue)
{


	// Grossmaster hackery
	if (GlobalParamInt[0] == 100)
	{
		extern int lensFlareOn;
		lensFlareOn = 0;
	}


	if (DomeOn == -2)
	{
		//RESET ALL BACK TO ORIGINAL VALUES
		TargetGlobalFogNear = LevelSettings->fognear;
		TargetGlobalFogFar = LevelSettings->fogfar;;
		TargetGlobalFarZ = LevelSettings->farz;
		TargetGlobalFogRed = LevelSettings->fogred / 255.f;
		TargetGlobalFogGreen = LevelSettings->foggreen / 255.f;
		TargetGlobalFogBlue = LevelSettings->fogblue / 255.f;
		TargetDomeFadeAmount = 1.f;

	}
	else
	{
		if (nearFog >= 0)
			TargetGlobalFogNear = nearFog;
		if (farFog >= 0)
			TargetGlobalFogFar = farFog;
		if (farCam >= 0)
			TargetGlobalFarZ = farCam;
		if (fogRed >= 0)
			TargetGlobalFogRed = fogRed;
		if (fogGreen >= 0)
			TargetGlobalFogGreen = fogGreen;
		if (fogBlue >= 0)
			TargetGlobalFogBlue = fogBlue;
		if (DomeOn >= 0)
		{
			if (DomeOn)
				TargetDomeFadeAmount = 1.f;
			else
				TargetDomeFadeAmount = 0.f;
		}

	}
   	if (GlobalParamInt[0] == 99)
   	{
		GlobalFogRed = TargetGlobalFogRed;
		GlobalFogGreen = TargetGlobalFogGreen;
		GlobalFogBlue = TargetGlobalFogBlue;
		GlobalFogFar = TargetGlobalFogFar;
		GlobalFogNear = TargetGlobalFogNear;
		GlobalFarZ = TargetGlobalFarZ;
   	}

}

void BossDarkness (float den)
{
	TargetGlobalFogDensity = RANGE(0,den,1);
	TargetDomeFadeAmount = RANGE(0,1.f-den,1);
}