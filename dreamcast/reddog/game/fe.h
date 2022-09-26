#ifndef _FE_H
#define _FE_H

extern "C" {
#include "RedDog.h"
#include "GameState.h"
#include "View.h"
#include "Render\Internal.h"
#include "Render\Rasterisers.h"
#include "Loading.h"
#include "Input.h"
#include "VMSStruct.h"
#include "level.h"
#include "backup.h"
// Temporary hacks : XXX
Uint32 GetPadAll(void);
extern Bool InBigPALMode;
extern Material fadeMat;
extern Bool suppressDebug;
extern void rReset(void);
extern void sbReInitMode (int mode);
extern int NumberOfControllers;
extern const char *LangLookup (const char *token);
extern Uint32 GetPadFE(int i);
extern int TAST_Person;
extern void ResetGame (GameSave *g);
extern char *GameParmName[];
extern TournamentGame CustomTournament;
extern int TournamentScore[4];
extern int CurHandicap [4];
extern int PlayerOrder[4], TeamOrder[2];
extern int GameVMS, GameProfile, GameDifficulty;
extern ModelContext context;
extern PNode *World;
int GetTimeRating (int level, float time);
int GetScoreRating (int level, int score);
int GetLivesRating (int level, int dam);
int GetBossTimeRating (int level, float time);
void MarkAsCompleted (int level, Mission *m);
void InitLogo(void);
bool RunLogo(void);
// Has a challenge level been well beaten?
bool ChallengeSmashed(int level);
}

// Too lazy to write a decent front end
inline size_t FE_strlen(const char *str)
{
	if (str[0] == '`')
		return strlen (LangLookup(str+1));
	else
		return strlen (str);
}
#define strlen FE_strlen

// Shared materials:
extern Material	testCard, vmuMaterial, bigXMat, weapMat[10], intelligence, tank, tankInside, padMat,
		zoomMat[MAX_MISSIONS], techMat;
// A cheating global variable
extern int SpecialCheatingNumber, NumReady;
// Are we a-cheatin' ?
extern bool Cheating, mapOK;

// A distraction on the main window
class WindowDistraction
{
public:
	// Factory method
	static WindowDistraction *MakeRandom();

	// Draw for a frame
	virtual void Draw() const = 0;
	// Update internally, returns true when done
	virtual bool Update() = 0;
};

class Screen;
class Window;
class ScreenID;
class VMU;
struct WindowPos
{
	float			zoom;					// Zoom amount
	float			u, v;					// U and V of top left co-ordinate
	inline bool operator == (const WindowPos &other) const
	{ return zoom == other.zoom && u == other.u && v == other.v; }
};
struct UVSet
{
	float uMin, uMax;
	float vMin, vMax;
	UVSet() {}
	UVSet(float u0, float v0, float u1, float v1) : uMin(u0), uMax(u1), vMin(v0), vMax(v1) {}
};
#define MAX_STACK 64
// A window onto a screen
class Window
{
private:
	static WindowPos validPositions[];		// Allowable positions
	static int	nPositions;					// Number of allowable positions
	static float	flashTab[16];			// Flashing amount!
	static float	scaleTab[43][2];		// Screen scale amount
	Camera			*camera;				// The camera
	Screen			*firstScreen;			// First screen in linked list
	Screen			*curScreen;				// Current screen (last in linked list)
	Screen			*nextScreen;			// The next screen to go to when shutdown successfully
	int				player;					// Which player this is for, -1 is no player, 0-3 is windowed
	WindowPos		curPos;					// Current window position
	WindowPos		targetPos;				// Target window position
	WindowPos		posStack[MAX_STACK];	// Position stack
	int				stackPtr;
	WindowDistraction *curDistraction;		// Current distraction
	int				nFramesToDist;			// Number of frames b4 next distraction
	enum MotionType {
		X_MOVE,
		X_PAUSE,
		Y_MOVE,
		Y_PAUSE,
		ZOOM,
		GAP,
		ZOOM_3,
		Y_PAUSE_3,
		Y_MOVE_3,
		X_PAUSE_3,
		X_MOVE_3
	};
	enum Status {
		POWERING_UP,
		READY
	};

	MotionType		motion;					// What motion we're doing at the moment
	Status			status;					// The status of the window
	int				lerp;					// Number of frames into current motion
	int				flashAmt;				// Flash amount
	float			addAmount;				// how much is added to lerp a frame
	int				scaleIdx;				// Scale index
	int				scaleAdd;				// How much to add to the scale value
	bool			deleteIt;				// Delete a screen after we've changed
	bool			deletePadPanic;			// Delete all screens after curScreen after a change

public:
	enum PlayerState {
		NOT_PLAYING,
		FIDDLING_AROUND,
		WAITING_FOR_OTHERS
	};
private:
	static PlayerState	playerState[4];		// The state each player is currently in
	static int			PadPanic[4];		// Whether we're in a pad panic at the moment
	static int			PlayerVMU[4];		// The VMU a player has selected
	static int			PlayerProfile[4];	// The profile number a player has selected

	// Private functions:

	// Move to a new area of screen
	void MoveBacking(void);
public:
	// Initialise the whole she-bang
	Window(int player, ScreenID &initialScreen);
	// Finalise the whole thing
	~Window();

	// Run for a frame
	void Run();

	// Change to a new screen (returns the new screen)
	Screen *Change(ScreenID &screen, Screen *prev);
	// Go back a step
	void Back();

	// Get the current screen
	inline const Screen *GetCurScreen() { return curScreen; }
	inline bool IsOnFirst() { return curScreen == firstScreen; }

	// Which player is this window for?
	inline int GetPlayer() const { return player; }

	// Get and set player states
	inline static void SetPlayerState(int i, PlayerState s) { dAssert(i>=0&&i<4,"Range"); playerState[i] = s; }
	inline static PlayerState GetPlayerState(int i) { dAssert(i>=0&&i<4,"Range"); return playerState[i]; }

	// Set the target position
	void SetTargetPos (const WindowPos &p);

	// Finalise and Initialise the current screens
	void FinaliseScreen();
	void InitialiseScreen();

	// Set the first screen in the window
	inline void SetFirst (Screen *s) { firstScreen = s; }

	// Lock and unlock a profile
	void LockProfile (int vmu, int subProfile);
	static void RelockProfile (int player);
	VMU *GetCurVMU();
	int GetCurVMUNum();
	int GetCurProfileNum();
	GameSave *GetCurProfile();
	void UnlockProfile ();
	static void UnlockAllProfiles();
	void WriteProfile ();
	static void WriteProfile (int player);

	// Create the camera
	void CreateCamera();
};

// A screen in the front end
class Screen
{
private:
	// Instance variables
	Window			*rootWindow;			// Which window this is embedded in
	Screen			*nextScreen;			// Next screen in linked list
	Screen			*prevScreen;			// Previous screen in linked list
public:
	// Constructor
	Screen() :rootWindow(NULL), nextScreen(NULL), prevScreen(NULL) {}
	// Virtual destructor
	virtual ~Screen();
	// Change to a new screen; delegates to parent Window
	inline void Change(ScreenID &screenID) { nextScreen = rootWindow->Change(screenID, this); }
	// Replace this screen with another
	void Replace(ScreenID &screenID);
	// Go back to the previous screen
	inline void Back() { 
		// Detach ourselves from our previous screen
		if (prevScreen)
			prevScreen->nextScreen = NULL;
		rootWindow->Back(); 
	}
	inline Screen *GetPrev() { return prevScreen; }
	inline Screen *GetNext() { return nextScreen; }
	inline Window *GetRoot() { return rootWindow; }
	// Get the player number this screen is serving
	inline int GetPlayer() const { return rootWindow->GetPlayer(); }

	// Set up next prev etc
	virtual void Setup (Window *w, Screen *prev) { rootWindow = w; prevScreen = prev; nextScreen = NULL; }

	bool DoesColourClash(int colour) const;

	// Overridables:

	// Initialise a screen prior to drawing
	virtual void Initialise() = 0;
	// Draws a screen 
	virtual void Draw() const = 0;
	// Processes input
	virtual void ProcessInput(Uint32 press) = 0;
	// Finalises a screen
	virtual void Finalise() = 0;
	// Shutdown/startup madness
	virtual bool HasShutdown() { return false; }
	// What background to use
	virtual Material *Background();
	// What colour to draw the background in
	virtual Uint32 BackgroundColour() { return 0x4dda21; }
	// Whether to override the stretching/squashing madness
	virtual bool SquashOverride() { return false; }
	// Position override
	virtual UVSet *BackgroundOverride() { return NULL; }
};

class MenuEntry;
struct MenuTemplate;

// Functions which creates a screen and a menu entry respectively
typedef Screen *CreateFunc();
typedef Screen *CreateFuncParm(void *parm);
typedef MenuEntry *EntryCreateFunc(const char *text, ScreenID &sub);

struct MenuEntryTemplate
{
	const char		*text;
	EntryCreateFunc	*clss;
	CreateFunc		*cParms;
	MenuTemplate	*parms;
};

struct MenuTemplate
{
	int nDescriptors;
	MenuEntryTemplate *templ;
};

class Menu : public Screen
{
private:
	// Instance variables
	MenuEntry		**pEntry;				// Array of menu entry pointers
	int				baseEntry;				// The base entry
	Sint32			timer;					// Timer which counts up to bring the menu on
	float			fadeAmount;				// The amount we're fading by
	ScreenID		*nextScreenID;			// The ScreenID to transition to next
	static bool		InitialDelayFlag;		// Is this the first menu?

	// Private functions:
	// Free the entry array
	void FreeEntries();
	// Build a menu from a template
	void BuildMenu(MenuTemplate *templ);

protected:
	int				curEntry;				// The currently selected entry
	int				nEntries;				// Number of entries
	int				maxEntries;				// The maximum number of entries on screen at any one time
public:
	// Constructor/destructor
	Menu (MenuTemplate *templ);
	virtual ~Menu();

	// Initialise a screen prior to drawing
	virtual void Initialise();
	// Draws a screen 
	virtual void Draw() const;
	// Processes input
	virtual void ProcessInput(Uint32 press);
	// Finalises a screen
	virtual void Finalise();
	// We need to override the default behaviour here
	virtual void Setup (Window *w, Screen *prev);

	// Refresh a menu from another template
	void Refresh(MenuTemplate *templ, bool textValid = true);
	// Get the faded amount
	inline float GetFadeAmount() const { return fadeAmount; }
	// Delete an entry
	void DeleteEntry(int ent);

	// Overridable:
	// Draw a extra lines of text
	virtual float DrawExtra(int line, float alpha, float nChars) const { return 0; }
	// The number of extra lines
	virtual int NumExtra () const { return 0; }
	virtual const char *GetTitle() const { return NULL; }
	// Veto a menu selection
	virtual bool VetoSelection(int selection) { return false; }

	// Class function; set initial delay flag
	static void SetInitialFlag() { InitialDelayFlag = true; }
	// Class function: draw a cursor
	static void Cursor (float x, float y, bool sound = true);
	static void PendingCursor();

};

// A UID for a screen
class ScreenID
{
private:
	enum Class {
		NULL_CLASS,
		MENU,
		BY_EXAMPLE,
		BY_EXAMPLE_WITH_PARM
	};
	Class		type;
	union {
		MenuTemplate	*menu;
		CreateFunc	*example;
		CreateFuncParm	*exampleParm;
	} u;
	void *parm;
public:
	ScreenID()
	{ type = NULL_CLASS; }
	ScreenID(MenuTemplate *menu)
	{
		type = MENU;
		u.menu = menu;
		if (menu == NULL)
			type = NULL_CLASS;
	}
	ScreenID(CreateFunc *example)
	{
		type = BY_EXAMPLE; 
		u.example = example;
	}
	ScreenID(CreateFuncParm *example, void *parm)
	{
		type = BY_EXAMPLE_WITH_PARM; 
		u.exampleParm = example;
		this->parm = parm;
	}
	// Get the user field, suitable for storing special information in the menu *
	int GetUser() { return int(u.menu); }
	// Create a new menu based on this ID
	Screen	*Create (Window *parent, Screen *prev = NULL);
	// Is this a NULL id
	bool IsNull() { return type == NULL_CLASS; }
	bool operator == (const ScreenID &other)
	{
		if (other.type != type ||
			other.parm != parm)
			return false;
		if (other.u.menu == u.menu)
			return true;
		else
			return false;
	}
	bool operator != (const ScreenID &other)
	{
		return !((*this) == other);
	}
};

// An entry in a basic menu
class MenuEntry
{
private:
	const char		*text;
	bool			selectable;
protected:
	ScreenID		subMenu;
	virtual void SetText (const char *text)
	{
		if (text[0] == '`')
			this->text = LangLookup(text+1);
		else
			this->text = text;
		selectable = true;
	}
public:
	MenuEntry(const char *text, ScreenID &sub) : subMenu(sub)
	{
		SetText (text);
	}
	virtual ~MenuEntry() {}

	// Draw an entry with given transparency and selected amount
	// Number of characters to draw increments up
	// Returns number of characters in the string
	virtual float Draw (float selAmt = 0.f, float alpha = 1.f, float nChars = 10000.f);
	// Item has been selected
	virtual ScreenID *Selected();
	// Item has had left pressed on it
	virtual void Left() {}
	// Item has had right pressed on it
	virtual void Right() {}
	// Is this entry selectable?
	virtual bool IsSelectable() { return selectable; }
	// Returns the text, not to be displayed; but for search action on a refreshed menu
	virtual const char *GetText() { return text; }
	// Sets the window this entry is within (default ignores this)
	virtual void SetWindow (Window *rootWindow) {}
};

// Front end state
enum CurFEState {
	SINGLEPLAYER,
	MP,
	MP_SETUP,
	MP_STATS,
	MP_OPTIONS
};

// VMU class, manages a single VMU
class VMU
{ 
private:
	int	physPort;
	const BACKUPINFO *info;
	MenuTemplate	profile;

	// Private functions
	void			FreeProfileTemplate();
	void			CreateProfileTemplate();

public:
	// Manage a particular port
	VMU(int port);
	// Scan on a potential change, returns specials bitmask
	Uint32 Scan();
	// Returns a profile name, or NULL if none
	const char *GetProfileName(int subPort);
	// Returns a tournament name, or NULL if non
	const char *GetTournamentName(int subPort);
	// Return the tournament
	const TournamentGame *GetTournament(int subPort);
	// Is the VMU physically attached?
	bool Connected();
	// Return the current menu template
	MenuTemplate *GetProfileTemplate() { return &profile; }
	// Find the first free slot, or -1 if none
	int FirstFreeSlot();
	// Save a tournament
	void SaveTournament (int subPort, TournamentGame *g);
};

// VMU manager class, manages all the VMUs
class VMUManager
{
private:
	VMU				*vmu[8];
	bool			driveChange;
	bool			dataChange;
	MenuTemplate	profile;
	Uint32			AllSpecials;
	Uint32			ExtraBits;
	ScreenID		curScreenID; // What we're currently set up to go to in menu options

	// Private functions
	void			FreeProfileTemplate();
	void			CreateProfileTemplate();
	friend class ProfileEntry;

public:
	VMUManager();
	// Poll the manager, call once a frame
	void Poll();
	// Has the number or configuration of drives changed since the last poll?
	bool HasDriveChanged() { return driveChange; }
	// Has the data on the drives changed?
	bool HasDataChanged() { return dataChange; }
	// Return the current menu template
	MenuTemplate *GetProfileTemplate();
	// Return a VMU
	inline VMU *GetVMU(int i) { return vmu[i]; }
	// Return all the specials in one
	Uint32 GetSpecials() const
	{ return AllSpecials | ExtraBits; }
	void SetSelectedScreenID(ScreenID id);
	void SetExtra(Uint32 eb) { ExtraBits = eb; }
	// Is a particular multiplayer map unlocked?
	bool IsUnlocked (int mapNum);
	// Special unlocked (DOD expert?)
	bool			SpecialUnlocked;
};
extern VMUManager *theVMUManager;

// Window sequence handler
// Handles the sequences of windows, and their reinitialisation from the front end
#define MAX_SETS 16
class WindowSequence
{
private:
	struct WindowSet {
		int nWindows;
		Window **window;
	};
	WindowSet setStack[MAX_SETS];
	int setPtr;
	bool ChangeSemaphore, singlePlayer;
	bool NeedsReset;
public:
	WindowSequence();

	// Proceed to the next window set
	void Proceed (ScreenID *list, int nWindows);

	// Return to the previous set
	void Back();

	// Reset the whole she-bang
	void Reset(bool restart = false);
	// Does the work of above
	void _Reset(bool restart);

	// Run it for a frame
	void Run();

	// Shutdown
	void Shutdown();

	// Reinitialise
	void Reinitialise();

	// Recreate the cameras after a switchout
	void Recreate();

	// Single playeredness
	inline void SetSinglePlayer(bool is = true)
	{ singlePlayer = is; }
	inline bool IsSinglePlayer() const
	{ return singlePlayer; }
};
// The one and only sequence handler
extern WindowSequence *theWindowSequence;

// Menu hacks
#define BEGIN_MENU(name)  MenuEntryTemplate name##Entries[] = {
#define ADD_OPTION(a,b,c) { a, b, NULL, c },
#define ADD_SUBSCREEN(a,b,c) { a, b, c, NULL },
#define END_MENU(name)	  }; MenuTemplate name = { asize(name##Entries), name##Entries };

#define PDD_CANCEL (PDD_DGT_TB|PDD_DGT_TX)
#define PDD_OK (PDD_DGT_ST|PDD_DGT_TA)

#define COL_TEXT			0x40ff40
#define COL_SELTEXT			0xc0ffc0
#define COL_UNSELECTABLE	0x303030

// Two horrid functions
void ShutdownFrontEnd();
void InitialiseFrontEnd();

// A sprite routine with a difference
void DrawFESprite (Material *mat, float x, float y, float width, float height, float cutoff, float visAmt, float addAmt, float alpha = 1.f);

// Make the FE sound
extern "C" void FE_Sound(void);
extern "C" void FE_MoveSound(void);
extern "C" void FE_ACK(void);
extern "C" void FE_NACK(void);
// Transform a set of point2s
void Transform(Point2 *p, int nPts);

// Get a mp level material
#define NUM_MP_LEVELS 16
extern Material *GetLevelMat(int i);

#endif

