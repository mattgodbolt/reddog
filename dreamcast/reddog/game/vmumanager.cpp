// VMU Manager, manages data in the VMUs
#include "Menus.h"

// The menu entry for a profile menu
class ProfileEntry : public MenuEntry
{
private:
	int vmu;
	int subProfile;
	Window *root;
public:
	ProfileEntry(const char *text, ScreenID &sub) : MenuEntry(text, ScreenID())
	{
		vmu = (sub.GetUser() / 4);
		subProfile = (sub.GetUser() & 3);
	}
	virtual bool IsSelectable()
	{
		MenuTemplate *templ = theVMUManager->GetProfileTemplate();
/*		for (int i = 0; i < templ->nDescriptors; ++i) {
			if (GetText() == templ->templ[i].text)
				break;
		}
		dAssert (i != templ->nDescriptors, "Erk");*/
		if (BupIsProfileLocked (vmu, subProfile))
			return false;
		else
			return true;
	}
	virtual ScreenID *Selected ()
	{
		root->LockProfile (vmu, subProfile);
		return &theVMUManager->curScreenID;
	}
	virtual void SetWindow (Window *rootWindow) { root = rootWindow; }
};

MenuEntry *profileEntryMaker(const char *text, ScreenID &sub)
{
	return new ProfileEntry (text, sub);
}

/////////////////////////////////////////////////////////
// Single VMU stuff
/////////////////////////////////////////////////////////

VMU::VMU(int port)
{
	physPort = port;
	profile.nDescriptors = 4;
	profile.templ = new MenuEntryTemplate[4];
	info = BupGetInfo (physPort);
	CreateProfileTemplate();
}

Uint32 VMU::Scan()
{
	info = BupGetInfo (physPort);
	FreeProfileTemplate();
	CreateProfileTemplate();
	return (info->save.game[0].unlockedSpecials | 
		info->save.game[1].unlockedSpecials | 
		info->save.game[2].unlockedSpecials | 
		info->save.game[3].unlockedSpecials);
}

const char *VMU::GetProfileName(int subPort)
{
	if (subPort < 0 || subPort >= 4 || !Connected())
		return NULL;
	if (strcmp (info->save.game[subPort].name, EMPTY_PROFILE) != 0)
		return info->save.game[subPort].name;
	else
		return NULL;
}

const char *VMU::GetTournamentName(int subPort)
{
	if (subPort < 0 || subPort >= 4 || !Connected())
		return NULL;
	if (strcmp (info->save.tournament[subPort].name, EMPTY_PROFILE) != 0)
		return info->save.tournament[subPort].name;
	else
		return NULL;
}

const TournamentGame *VMU::GetTournament(int subPort)
{
	dAssert (Connected(), "Not connected!");
	return (&info->save.tournament[subPort]);
}

bool VMU::Connected()
{
	return (info->Colour != 0);
}

void VMU::FreeProfileTemplate()
{
	int i;
	for (i = 0; i < profile.nDescriptors; ++i) {
		delete [] (char *)profile.templ[i].text;
	}
}

void VMU::CreateProfileTemplate()
{
	int i;
	for (i = 0; i < 4; ++i) {
		const char *name = GetProfileName(i);
		if (name == NULL)
			name = LangLookup("EMPTY_PROFILE");
		int len = strlen (name);
		profile.templ[i].text = new char[len + 1];
		profile.templ[i].clss = profileEntryMaker;
		profile.templ[i].cParms = NULL;
		profile.templ[i].parms = (MenuTemplate *)(physPort*4 + i);
		strcpy ((char *)profile.templ[i].text, name);
	}
}

int VMU::FirstFreeSlot()
{
	for (int i = 0; i < 4; ++i)
		if (GetProfileName(i) == NULL)
			return i;
	return -1;
}

void VMU::SaveTournament (int subPort, TournamentGame *g)
{
	if (!Connected())
		return;
	(void)BupSaveTournament (physPort, subPort, g);
}

/////////////////////////////////////////////////////////
// VMU Manager
/////////////////////////////////////////////////////////

// Create the one and only VMU manager
VMUManager::VMUManager()
{
	// Create a VMU for every port
	for (int i = 0; i < 8; ++i)
		vmu[i] = new VMU(i);

	// Create the profile template for the first time
	CreateProfileTemplate();

	AllSpecials = ExtraBits = SpecialUnlocked = 0;
	// Default
	SetSelectedScreenID(ScreenID());
}

// Poll the manager, looking for status changes et al
void VMUManager::Poll()
{
	driveChange = dataChange = false;

	// Check for changes in the VMU system
	if (bupDriveChange) {
		BupRescan();
		bupDriveChange = FALSE;
		driveChange = true;
//		dPrintf ("Backup layer says drive changed");
	} 
	if (bupDataChange) {
		// Rebuild the profile menus
		bupDataChange = FALSE;
		dataChange = true;
//		dPrintf ("Backup layer says data changed");
	}

	// Poll all of the drives if something has changed
	if (dataChange || driveChange) {
		AllSpecials = 0;
		for (int i = 0; i < 8; ++i)
			AllSpecials |= vmu[i]->Scan();
		// Now to rebuild the profile
		FreeProfileTemplate();
		CreateProfileTemplate();
	}
}

// Free the profile template
void VMUManager::FreeProfileTemplate()
{
	int i;
	for (i = 0; i < profile.nDescriptors; ++i) {
		delete [] (char *)profile.templ[i].text;
	}
	delete profile.templ;
	profile.nDescriptors = 0;
	profile.templ = NULL;
}

// Just a little profile thingy used in the sort process
struct VMUProfile {
	int vmu;
	int subProfile;
	const char *text;
};
static int VMUCompareFunc(const void *v0, const void *v1)
{
	VMUProfile *t0 = (VMUProfile *)v0;
	VMUProfile *t1 = (VMUProfile *)v1;
	return strcmp (t0->text, t1->text);
}

// Create the profile template
void VMUManager::CreateProfileTemplate()
{
	// Room for the profile names
	VMUProfile t[8*4];
	int nProfiles;

	nProfiles = 0;
	for (int i = 0; i < 8; ++i) {
		for (int j = 0; j < 4; ++j) {
			const char *profileName = vmu[i]->GetProfileName(j);
			if (profileName) {
				t[nProfiles].text = profileName;
				t[nProfiles].vmu = i;
				t[nProfiles].subProfile = j;
				nProfiles++;
			}
		}
	}

	// Sort them alphabetically
	qsort ((void *)t, nProfiles, sizeof (VMUProfile), VMUCompareFunc);

	// Create a menu template for these
	profile.nDescriptors = nProfiles;
	profile.templ = new MenuEntryTemplate[nProfiles];
	for (i = 0; i < nProfiles; ++i) {
		char tempBuf[64];
		int nBytes;
		nBytes = sprintf (tempBuf, "%s <%c:%d>", t[i].text, 'A' + (t[i].vmu>>1), 1 + (t[i].vmu & 1)) + 1; 
		profile.templ[i].text = new char[nBytes];
		strcpy ((char *)profile.templ[i].text, tempBuf);
		profile.templ[i].clss = profileEntryMaker;
		profile.templ[i].cParms = NULL;
		profile.templ[i].parms = (MenuTemplate *)(t[i].vmu*4 + t[i].subProfile);
	}
	// Ensure rebuild of the menus
	dataChange = true;
//	dPrintf ("Created template");
}

MenuTemplate *VMUManager::GetProfileTemplate()
{
	return &profile; 
}

void VMUManager::SetSelectedScreenID(ScreenID id)
{
	if (id.IsNull())
		id = ScreenID(mpWaitScreen);
	curScreenID = id;
}

bool VMUManager::IsUnlocked (int mapNum)
{
	if (MPUnlockCode[mapNum]) {
		if (MPUnlockCode[mapNum] == 8)
			return SpecialUnlocked;
		if (!(theVMUManager->GetSpecials() & (SPECIAL_EXTRA_LEVEL(MPUnlockCode[mapNum]-1))))
			return false;
	}
	return true;
}