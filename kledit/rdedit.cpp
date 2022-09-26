/*
 * RDEdit.cpp - the 3DS MAX 2 plugin register
 */

#include "StdAfx.h"
#include "ObjFlags.h"
#include "LevelFlags.h"
#include "camcontrol.h"
#include "animcontrol.h"
#include "CoM.h"
#include "ApplyVC\ApplyVC.h"
#include "VisBox.h"
#include "CampPoint.h"
#include "SplineMod.h"

HINSTANCE hInstance;
int controlsInit = FALSE;

// This function is called by Windows when the DLL is loaded.  This 
// function may also be called many times during time critical operations
// like rendering.  Therefore developers need to be careful what they
// do inside this function.  In the code below, note how after the DLL is
// loaded the first time only a few statements are executed.

BOOL WINAPI DllMain (HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved)
{
	hInstance = hinstDLL;				// Hang on to this DLL's instance handle.

	if (!controlsInit) {
		controlsInit = TRUE;
		InitCustomControls(hInstance);	// Initialize MAX's custom controls
		InitCommonControls();			// Initialize Win95 controls
		ClearAnims();					//ENSURE THE ANIM TABLE IS RESET
	
	
	}
			
	return (TRUE);
}

// This function returns a string that describes the DLL and where the user
// could purchase the DLL if they don't have it.
__declspec(dllexport) const TCHAR* LibDescription()
{
	return GetString (IDS_LIBDESCRIPTION);
}

// This function returns the number of plug-in classes this DLL
//TODO: Must change this number when adding a new class
__declspec(dllexport) int LibNumberClasses()
{
	return 17;
}


extern ClassDesc* GetBoxobjDesc();
extern ClassDesc* GetShadDesc();

// This function returns the number of plug-in classes this DLL
__declspec(dllexport) ClassDesc* LibClassDesc (int i)
{
	extern ClassDesc* GetGSphereDesc();
	switch (i) {
	   	case 0: return GetRDExportDesc();
	   	case 1: return GetRDObjectDesc();
	   	case 2: return GetRDObjectExportDesc();
	   	case 3: return GetPreviewDesc();
	   	case 4: return GetCollDesc();
	  	case 5: return GetFlagDesc();
	  	case 6: return GetRDStratExportDesc();
	  	case 7: return GetCOMDesc();
	   	case 8: return GetLevelFlagDesc();
	   	case 9: return GetMaterialDesc();
	   	case 10: return GetVISDesc();
	   	case 11: return GetcamcontrolDesc();
	   	case 12: return GetanimcontrolDesc();
		case 13: return GetBoxobjDesc();
	   	case 14: return GetCampDesc();
		case 15: return GetSplineModDesc(); 
		case 16: return GetShadDesc();
	
		default: return NULL;
	}
}

// This function returns a pre-defined constant indicating the version of 
// the system under which it was compiled.  It is used to allow the system
// to catch obsolete DLLs.
__declspec(dllexport) ULONG LibVersion()
{
	return VERSION_3DSMAX;
}
