/*
 * GameState.h
 * The global game state, and game state tables
 */

#ifndef _GAMESTATE_H
#define _GAMESTATE_H

/*
 * A list of all game states.
 * If you change this, you *must* alter the matching table in Main.c,
 * or very bad things will happen
 */

typedef enum
{
	GS_STARTUP,
	GS_TITLEPAGE,
	GS_GAME,
	GS_ADVERT
} GameState;

/* The reason for calling a game state handler */
typedef enum { GSR_INITIALISE, GSR_RUN, GSR_FINALISE } GameStateReason;
/* A game state handler */
typedef void GameStateHandler (GameState thisState, GameStateReason reason);

/* An entry in the game state table */
typedef struct tagGameStateTableEntry
{
	GameStateHandler	*handler;	/* The routine that handles this state */
	char				*name;		/* The name of this game state */
} GameStateTableEntry;

/*
 * A routine to change the current game state
 * Game state changing will acutally occur the next time around the main loop
 */
void ChangeGameState (GameState newState);

/*
 * Get the current game state and name
 */
extern GameState currentGameState;
#define GetGameState() currentGameState
char *GetGameStateName (void);


/*
 * Starts a fade
 */
void StartFade (FadeType type);

/*
 * Starts a fade, and changes to a new GameState when the fade is over
 */
void FadeToNewState (FadeType type, GameState state);

/*
 * Has the current fade finished?
 */
Bool hasFadeFinished (void);

/*
 * Are we changing state?
 */
Bool ChangingState (void);

/*
 * A list of the game states available
 */
extern GameStateHandler startupHandler, titlePageHandler, gameHandler, mpDebriefHandler, mpSetupHandler;
extern GameStateHandler logoSequenceHandler;

/*
 * The in-game substates
 */
#define MP_CAMERA_TRACK			0
#define MP_TRANSITION_TO_GAME	1
#define MP_CHOOSE_TEAMS			2
#define MP_PREDATOR				3
#define MP_BOMB					4
#define MP_SUICIDE_BOMB			5
#define MP_TEAM_PREDATOR		6
#define MP_TEAM_BOMB			7
#define MP_TEAM_SUICIDE_BOMB	8
#define MP_FADE_OUT_TO_BLACK	9
#define MP_DEBRIEF				10

#define MP_IN_GAME				16
extern int MPStartupState;
extern char *GameTypeName[];
extern Uint32 TankPalette[];

#endif
