/*
 * Menu system
 */
#ifndef _MENU_H
#define _MENU_H

#define IDLE_WAIT		(60*60)	// Minute idle wait time
#define MENU_Y_SPACING	3		// 3 pixels between entries
#define MENU_HEIGHT		32		// menu entries are 32 pixels high

#define MENU_BLUR_FRAMES 8
#define BLUR_STEP		4
#define DAMPING_MENU	(PAL ? 0.261f : 0.2f)
#define MAX_MENU_MOVE	10000
/*
 * Structures 
 */

#define MH_SELECTED		0
#define MH_DESELECTED	1
#define MH_DRAW			2
#define MH_LEFT			3
#define MH_RIGHT		4
typedef void MenuHandler (int reason, struct tagMenuEntry *entry, void *data);

// An entry in a menu
typedef struct tagMenuEntry
{
	// Filled in by user:
	char		*text;			// The text of this menu
	struct tagMenu *subMenu;	// Pointer to submenu if any
	MenuHandler	*handler;		// Selection/deselection handler if any
	void		*data;			// Specific data
	// Filled in by code:
	Bool		selected;		// Is this item currently selected?
	float		selAmt;			// Selection 'amount' (0 - 1)
	Bool		selectable;		// Is it selectable?
} MenuEntry;

// A menu
typedef struct tagMenu
{
	Point2		pos, targetPos;	// Position and target position
	Point2		posHistory[MENU_BLUR_FRAMES];
	int			numEntries;		// The number of entries
	int			topEntry;		// The top entry in a scrolling menu
	int			visEntries;		// The number of entries visible at a time in a scrolling menu
	struct tagMenu	*parent;	// The parent menu
	int			parentEntry;	// The entry in the parent this is open from
	int			defaultEntry;	// Which entry to call as default, -1 if none
	MenuEntry	*entry;			// Pointer to array of entries
} Menu;

/*
 * Functions
 */

/*
 * Initialise the menu system
 */
void mInit (void);

/*
 * Is there a menu open?
 */
Bool mIsOpen (void);

/*
 * Open up a new root menu
 */
void mOpen (Menu *menu, float cX, float cY);

/*
 * Draw the menu tree
 */
void mDrawMenu (void);

/*
 * Process input in terms of physical button presses
 */
void mProcessInput (Uint32 padAll);

/*
 * Create a menu (fills in the data structures properly)
 */
void mMakeMenu (Menu *menu, int nEnts, MenuEntry *entry);

/*
 * Macros
 */
// Use to make a menu if you're as lazy as I am
#define MAKE_MENU(MENU) mMakeMenu (&MENU##Menu, asize (MENU##Entries), MENU##Entries);

#endif
