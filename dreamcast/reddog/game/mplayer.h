#ifndef _MPLAYER_H
#define _MPLAYER_H

#define MIN_TOURNAMENT_GAMES	3
#define MAX_TOURNAMENT_GAMES	6

#define MAX_MP_GAME_TIME		(60*5) // five minutes is plenty too
#define MAX_MP_MAPS				24

typedef enum { SKIRMISH, TEAM, TOURNAMENT } PlayMode; 
typedef enum { DEATHMATCH, KNOCKOUT, TAG, SUICIDE_TAG, PREDATOR, KEEP_THE_FLAG, KING_OF_THE_HILL, RANDOM } GameType;
typedef enum { BRONZE, SILVER, GOLD, CUSTOM, FROMVMS, LOOKUPFROMVMS } TournamentType;

typedef Sint16 MapNum;		// Map is just the arena number
typedef Sint16 Weapons;		// Weapons is a bitmask of available weapons
// List of weapon availabilities :
#define WEAPON_BOMB				(1<<0)
#define WEAPON_ELECTRO			(1<<1)
#define WEAPON_HEALTH			(1<<2)
#define WEAPON_HOMING_CANNON	(1<<3)
#define WEAPON_HOMING_MISSILE	(1<<4)
#define WEAPON_MINE				(1<<5)
#define WEAPON_STEALTH			(1<<6)
#define WEAPON_STORMING_SHIELD	(1<<7)
#define WEAPON_ULTRA_LASER		(1<<8)
#define WEAPON_VULCAN			(1<<9)
#define WEAPON_MAX				(1<<10)
#define WEAPON_ALL				(WEAPON_MAX-1)

#define PREDATOR_HEALTH			20.f

typedef struct {
	int			fragLimit;
} DMParms, PredatorParms;

typedef struct {
	int			startLives;
} KOParms;

typedef struct {
	int			bombTimer;			// Bomb timer setting in seconds
} BombParms;

typedef struct {
	int			winTime;			// Time in seconds holding the flag/on hill to win
} KTFParms, KOTHParms;

typedef struct {
	DMParms				dmParms;
	KOParms				koParms;
	BombParms			bombParms;
	BombParms			suicideParms;
	PredatorParms		predatorParms;
	KTFParms			ktfParms;
	KOTHParms			kothParms;
} GameParms;

typedef struct {
	GameType			gameType;	// The type of game this is
	GameParms			parms;		// The parameters for this game
	MapNum				map;		// Where this game will take place
} SkirmishGame;

typedef struct {
	GameType			gameType;	// The type of game this is
	GameParms			parms;		// The parameters for this game
	Bool				friendlyFire;// Whether friendly fire hurts
	MapNum				map;		// Where this team game will take place
} TeamGame;

typedef struct {
	int					nGames;		// Number of games in the tournament
	TournamentType		tType;		// The type of tournament
	SkirmishGame		game[MAX_TOURNAMENT_GAMES];	// The games themself (as they will be played)
	bool				team[MAX_TOURNAMENT_GAMES]; // Is this game a team game?
	char				name[16];	// The name of the tournament
} TournamentGame;

typedef struct {
	int					nPlayers;	// Number of people playing this game
	PlayMode			playMode;	// Type of game mode
	struct {
		SkirmishGame	skirmish;
		TeamGame		team;
		TournamentGame	tournament;
	}					game;		// PlayMode-specific parameters
	Weapons				weapons;	// The available weapons
} MPSettings;

// The global settings
extern MPSettings mpSettings;
extern int CurrentTournamentGame;

// Reset the global settings to something sensible
void ResetMPSettings(void);

// Is this a team game?
bool IsTeamGame (void);

// Get the current game type
GameType GetCurrentGameType(void);
// Get the current game parms
GameParms *GetCurrentGameParms(void);

void ResetGameParms (GameParms *p);

void SetUpForTournament (TournamentGame *t);

extern char *MPMapName[MAX_MP_MAPS];
extern char *MPWadName[MAX_MP_MAPS];
extern Uint32 MPLevOK[MAX_MP_MAPS];
extern Uint32 MPUnlockCode[MAX_MP_MAPS];
extern int numMPMaps;

// Gets the number of frames alive for each player
int GetPlayerFramesAlive (int i);

// Get the score of a player, taking into account which game type we are playing and whether their team won
// Don't display this value, it's for sorting only
int GetPlayerScore (int i);
int GetPlayerKills(int i, int j);
int GetPlayerMaxFrames(int i);
int GetPlayerTotFrames(int i);

// Get the handicap of a player taking into account team gates et al
float GetHandicap(int pNum);

// How many games to unlock Mr Skelterous Mapae
#define NUFF_MP_GAMES 20

// Sounds:
void SFX_WinGame(void);
// A tournament has been won
void SFX_WinTournament(void);
// Bomb passed on in-game
void SFX_PassBomb(void);
// Bomb passed on in mini-game at beginning
void SFX_PassBombFE(void);
// Get the flag
void SFX_PassFlag(void);
// Become predator
void SFX_BecomePredator(void);
// Become king
void SFX_BecomeKing(void);
// Being eliminated from knockout
void SFX_Eliminated(void);
// Time stuff
void SFX_TimeBeep(int time);

#endif
