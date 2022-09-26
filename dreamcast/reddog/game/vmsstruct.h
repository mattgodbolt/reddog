#ifndef _VMSSTRUCT_H
#define _VMSSTRUCT_H

#include "MPlayer.h" // For the tournament action

// The options as stored in the VMS
#define MAX_MISSIONS		13		// Number of missions in the game
#define MAX_TOURNAMENTS		4	
#define DIF_EASY			0
#define DIF_MEDIUM			1
#define DIF_HARD			2
#define DIF_TIME_ATTACK		3
#define MAX_DIFFICULTY		2		// Normal and hard mode
#define NUM_MPLAYER_TYPES	7

typedef Sint16 Grading;

typedef struct
{
	Sint8		completed;			// Has this continent been completed?
	Grading		grading;			// Grading for this level
	Uint32		bestScore;			// Best score
	float		bestTime;			// Best time in seconds!!!!  Stops PAL/NTSC dependency
	int			livesLost;			// The least lives lost
	float		bestBossTime;		// The best time taken to defeat the boss
} Mission;

#define	CTRL_AXB	0
#define	CTRL_ABX	1
#define	CTRL_XAB	2
#define	CTRL_XBA	3
#define	CTRL_BAX	4
#define	CTRL_BXA	5
#define	CTRL_MASK	7
#define CTRL_NOTINV	8	// Don't invert y axis
#define CTRL_NOTDBL	16	// Don't boost in multiplayer

typedef struct
{
	Sint8	colour;		// Tank colour
	Sint8	reserved0;
	Sint16	reserved1;	// Reserved for future expansion
	struct {
		Sint16	played;
		Sint16	won;
	} Game[NUM_MPLAYER_TYPES];
} MPStats;

///////////////////////////////////////////////////////////////////////////
// IF YOU CHANGE THE FOLLOWING UPDATE SINGLEPLAYER.CPP's table!!!!!!!!!!!!!
///////////////////////////////////////////////////////////////////////////

// Selectable specials : cheats
#define SPECIAL_TRIPPY_TRAILS		(1<<0)
#define SPECIAL_PERMANENT_BOOST		(1<<1)
#define SPECIAL_INFINITE_SHIELD		(1<<2)
#define SPECIAL_ASSISTANCE_BOT_1	(1<<3)
#define SPECIAL_ASSISTANCE_BOT_2	(1<<4)
#define SPECIAL_INF_WEAPON_CHARGE	(1<<5)
#define SPECIAL_INVULNERABILITY		(1<<6)
#define SPECIAL_60HZRACE			(1<<7)
// reserved bits, then
// Selectable specials : upgrades
#define SPECIAL_ARMOUR_GET(a)		(((a) & (3<<8))>>8)
#define SPECIAL_ARMOUR_SET(a,n)		a = (((a)&~(3<<8)) | (((n)&3)<<8))
#define SPECIAL_WEAPON_GET(a)		(((a) & (3<<10))>>10)
#define SPECIAL_WEAPON_SET(a,n)		a = (((a)&~(3<<10)) | (((n)&3)<<10))
#define SPECIAL_ARMOUR_AND_WEAPON_MASK (15<<8)
#define SPECIAL_MISSILE_GET(a)		(((a) & (1<<12))>>12)
#define SPECIAL_MISSILE_SET(a,n)	a = (((a)&~(1<<12)) | (((n)&1)<<12))
#define SPECIAL_SHIELD_GET(a)		(((a) & (1<<13))>>13)
#define SPECIAL_SHIELD_SET(a,n)	a = (((a)&~(1<<13)) | (((n)&1)<<13))
// Permanent specials
#define SPECIAL_KINGOFTHEHILL		(1<<16)
#define SPECIAL_HARD_DIFFICULTY		(1<<17)
#define SPECIAL_FIRST_WEAP_BIT		18
#define SPECIAL_EXTRA_LEVEL(n)		(1<<((n)+25))

#define SPECIAL_CHEATING_MASK		(SPECIAL_WEAP_ALL | SPECIAL_PERMANENT_BOOST | SPECIAL_INFINITE_SHIELD | SPECIAL_ASSISTANCE_BOT_1 | SPECIAL_ASSISTANCE_BOT_2 | SPECIAL_INF_WEAPON_CHARGE | SPECIAL_INVULNERABILITY)

typedef struct 
{
	// Personal information
	char		name[16];			// Character name
	// The mission data
	Mission		missionData[MAX_DIFFICULTY][MAX_MISSIONS];
	// Personal options
	Sint8		difficulty;			// Difficulty setting
	Sint8		controller;			// Controller configuration
	Sint16		activeSpecials;		// Bits of unlocked specials that are active (cheats & upgrades)
	Sint8		cheatWeapon;		// Current cheat weapon, or 0 if none
	Sint32		unlockedSpecials;	// Unlocked special bits - specials that are available
	float		soundVolume;		// Sound volume 0 - 1
	float		musicVolume;		// Music volume 0 - 1
	// Multiplayer:
	MPStats		multiplayerStats;	// MP statistics
	// And a per-profile checksum, which sits at the end
	Uint32		AntiTamperCheck;
} GameSave;

typedef struct 
{
	char		argo[8];			// 'Argonaut' for posterity
	Uint32		version;			// Version
	GameSave	game[4];			// The four game save slots (profiles)
	// Multiplayer configuration
	TournamentGame	tournament[MAX_TOURNAMENTS];
} Options;

#define OPTIONS_VERSION		0x150

// The current options
extern Options curOptions;
// The current profiles being used
extern GameSave	CurProfile[4];

int LogToPhys (int j);

void InitOptions (Options *o);

#endif
