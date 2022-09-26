#include "RedDog.h"
#include "GameState.h"
#include "MPlayer.h"
#include "Render\Internal.h"
#include "Level.h"
#include "View.h"
#include "VMSStruct.h"
#include "Layers.h"
#include "Input.h"

int NumberOfPlayers = 4;
int Winner = -1;
int BombCounter = 0;
extern int NumSpawnPoints;
extern Uint32 GetTankColour (int i);
static int FlagSpawned = FALSE;
static int NextPredator = 0;
int CurrentTournamentGame = 0;
Point3 FlagPos;

/////////////////////////////////////////////////////////////////
// Some sound action for multiplayer
/////////////////////////////////////////////////////////////////

// A game has been won (should be in now)
void SFX_WinGame(void)
{
	sfxPlay (GENERIC_WIN_SOUND, 1, 0);
}

// A tournament has been won
void SFX_WinTournament(void)
{
	sfxPlay (GENERIC_TOURN_VICT, 1, 0);
}

// Bomb passed on in-game
void SFX_PassBomb(void)
{
	sfxPlay (GENERIC_BOMB_PASS, 1, 0);
}

// Bomb passed on in mini-game at beginning
void SFX_PassBombFE(void)
{
	sfxPlay (GENERIC_TOGGLE_BEEP, 1, 0);
}

// Get the flag
void SFX_PassFlag(void)
{
	sfxPlay (GENERIC_FLAG, 1, 0);
}

// Become predator
void SFX_BecomePredator(void)
{
	sfxPlay (GENERIC_STEALTH, 1, 0);
}

// Become king
void SFX_BecomeKing(void)
{
	sfxPlay (GENERIC_KINGHILL, 1, 0);
}

// Being eliminated from knockout
void SFX_Eliminated(void)
{
	sfxPlay (GENERIC_ELIMINATE, 1, 0);
}

// Time stuff
void SFX_TimeBeep(int time)
{
	if ((time % FRAMES_PER_SECOND) == 0) {
		if (time <= (5*FRAMES_PER_SECOND)) {
			sfxPlay (GENERIC_TIMER_BLEEP_2, 1, 0);
		} else if (time <= (20*FRAMES_PER_SECOND)) {
			sfxPlay (GENERIC_TIMER_BLEEP_1, 1, 0);
		}
	}
}

void KillSelf (int pNum);

char *GameTypeName[] = { "Deathmatch", "Knockout", "Bomb Tag", "Suicide Bomb Tag",
								"Stealth Assassin", "Flag Runner", "King of the Hill",
								"Random" };

struct {
	int		kills[MAX_PLAYER];
	int		framesAlive;
	int		maxFrames;
	int		totFrames;
} PlayerStats[MAX_PLAYER];

// The global settings
MPSettings mpSettings;

static void RecordStats(void)
{
	int i, j;
	for (i = 0; i < MAX_PLAYER; ++i) {
		for (j = 0; j < MAX_PLAYER; ++j) {
			PlayerStats[i].kills[j] = player[i].kills[j];
			PlayerStats[i].framesAlive = player[i].framesAlive;
			PlayerStats[i].maxFrames = player[i].maxFrames;
			PlayerStats[i].totFrames = player[i].totFrames;
		}
	}
}

static int AdjustTimer (int BombCounter)
{
	if (GetCurrentGameType() == KING_OF_THE_HILL) {
		if (currentFrame > (FRAMES_PER_SECOND*1*60)) { // One minute
			int over;
			BombCounter -= 5*FRAMES_PER_SECOND;
			over = currentFrame - (FRAMES_PER_SECOND*1*60);
			BombCounter -= 5 * FRAMES_PER_SECOND * (int)(over / (FRAMES_PER_SECOND*60));
		}
		return MAX(5*FRAMES_PER_SECOND, BombCounter);
	} else {
		if (currentFrame > (FRAMES_PER_SECOND*2*60)) { // Two minutes
			int over;
			BombCounter -= 5*FRAMES_PER_SECOND;
			over = currentFrame - (FRAMES_PER_SECOND*2*60);
			BombCounter -= 5 * FRAMES_PER_SECOND * (int)(over / (FRAMES_PER_SECOND*60));
		}
		if (GetCurrentGameType() == KEEP_THE_FLAG)
			return MAX(5*FRAMES_PER_SECOND, BombCounter);
		else
			return MAX(10*FRAMES_PER_SECOND, BombCounter);
	}
}

void DoTimeBacking(void)
{
	extern Material fadeMat;
	float x, y;

	x = PHYS_SCR_X/2 - 12 * 8;
	y = PHYS_SCR_Y/2 - 17.5 + rGlobalYAdjust - 4;
	
	rSetMaterial (&fadeMat);
	kmxxGetCurrentPtr (&vertBuf);
	kmxxStartVertexStrip (&vertBuf);
	
	kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL, x,				y, W_LAYER_TIME_BACKING_2, 0xff000000);
	kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL, x,				y + 35, W_LAYER_TIME_BACKING_2, 0xff000000);
	kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL, x + 12 * 8 * 2,		y, W_LAYER_TIME_BACKING_2, 0xff000000);
	kmxxSetVertex_0 (KM_VERTEXPARAM_ENDOFSTRIP, x + 12 * 8 * 2,	y + 35, W_LAYER_TIME_BACKING_2, 0xff000000);

	kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL, x-1,				y-1, W_LAYER_TIME_BACKING, 0xff00ff00);
	kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL, x-1,				y + 36, W_LAYER_TIME_BACKING, 0xff00ff00);
	kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL, x+1 + 12 * 8 * 2,		y-1, W_LAYER_TIME_BACKING, 0xff00ff00);
	kmxxSetVertex_0 (KM_VERTEXPARAM_ENDOFSTRIP, x+1 + 12 * 8 * 2,	y + 36, W_LAYER_TIME_BACKING, 0xff00ff00);

	kmxxReleaseCurrentPtr (&vertBuf);
}

// Gets the number of frames alive for each player
int GetPlayerFramesAlive (int i)
{
	return PlayerStats[i].framesAlive;
}

int GetPlayerKills(int i, int j)
{
	return PlayerStats[i].kills[j];
}

int GetPlayerMaxFrames(int i)
{
	return PlayerStats[i].maxFrames;
}

int GetPlayerTotFrames(int i)
{
	return PlayerStats[i].totFrames;
}

// Get the score of a player, taking into account which game type we are playing and whether their team won
// Don't display this value, it's for sorting only
int GetPlayerScore (int i)
{
	int j, retVal = 0;
	switch (GetCurrentGameType()) {
	case DEATHMATCH:
	case PREDATOR:
		for (j = 0; j < NumberOfPlayers; ++j)
			retVal += PlayerStats[i].kills[j];
		break;

	case KNOCKOUT:
	case TAG:
	case SUICIDE_TAG:
		// Ten frame boost for the winner
		retVal = PlayerStats[i].framesAlive + ((i == Winner) ? 10 : 0);
		break;

	case KEEP_THE_FLAG:
	case KING_OF_THE_HILL:
		retVal = PlayerStats[i].maxFrames + ((i == Winner) ? 0x100000 : 0);
		break;
	default:
		dAssert (FALSE, "My dog ate it");
		break;
	}
	// Give the winning team a + 0x10000 boost
	if (IsTeamGame() && (player[Winner].team && player[Winner].team == player[i].team))
		return retVal + 0x10000; // Special 'won' flag
	else
		return retVal;
}

extern TournamentGame CustomTournament;
GameType GetCurrentGameType(void)
{
	switch (mpSettings.playMode) {
	case TOURNAMENT:
		if (GetGameState() == GS_GAME)
			return mpSettings.game.tournament.game[CurrentTournamentGame].gameType;
		else
			return CustomTournament.game[CurrentTournamentGame].gameType;
	case SKIRMISH:
		return mpSettings.game.skirmish.gameType;
	case TEAM:
		return mpSettings.game.team.gameType;
	}
}

GameParms *GetCurrentGameParms(void)
{
	switch (mpSettings.playMode) {
	case TOURNAMENT:
		if (GetGameState() == GS_GAME)
			return &mpSettings.game.tournament.game[CurrentTournamentGame].parms;
		else
			return &CustomTournament.game[CurrentTournamentGame].parms;
	case SKIRMISH:
		return &mpSettings.game.skirmish.parms;
	case TEAM:
		return &mpSettings.game.team.parms;
	}
}

// Is this a team game?
bool IsTeamGame (void)
{
	return (NumberOfPlayers > 2 && mpSettings.playMode == TEAM || 
		(mpSettings.playMode == TOURNAMENT && 
		 mpSettings.game.tournament.team[CurrentTournamentGame]));
}

// Make a player a predator
void MakePredator (int pn)
{
	// If we're not already predator, play dat noise
	if (!player[pn].special) {
		if (MPStartupState == MP_IN_GAME)
			SFX_BecomePredator();
		else
			SFX_PassBombFE();
	}
	if (player[pn].team) {
		// Make teammates predators too
		int i;
		for (i = 0; i < NumberOfPlayers; ++i) {
			if (/*player[i].Player && player[i].Player->var != 1234.f &&*/
				player[i].team == player[pn].team) {
					player[i].maxHealth = GetHandicap(i) * PREDATOR_HEALTH;
					player[i].cloaked = -1;
					player[i].special = TRUE;
					player[i].PlayerSecondaryWeapon.type = LASER;
					player[i].PlayerSecondaryWeapon.amount = 0;
			} else {
				// If the player was a predator before, then adjust their health accordingly
				if (player[i].special && player[i].active && player[i].Player && player[i].Player->var != 1234.f) {
					player[i].Player->health *= (100.f / PREDATOR_HEALTH);
					player[i].PlayerSecondaryWeapon.type = 0;
					player[i].PlayerSecondaryWeapon.amount = 0;
				}
				player[i].cloaked = 0;
				player[i].special = FALSE;
			}
		}
	} else {
		player[pn].maxHealth = GetHandicap(pn) * PREDATOR_HEALTH;
		player[pn].cloaked = -1;
		player[pn].special = TRUE;
		player[pn].PlayerSecondaryWeapon.type = LASER;
		player[pn].PlayerSecondaryWeapon.amount = 0;
		// In case this player has karked it, record it
		NextPredator = pn;
	}
}

// See if the multiplayer game is over
int CheckEndGameStatus (void)
{
	int i, j, k, hi, num;
	GameParms *p = GetCurrentGameParms();

	// Return false with no side-effect if not in-game yet
	if (MPStartupState != MP_IN_GAME || Winner != -1)
		return FALSE;

	// Update the cloaking amounts and counters
	for (i = 0; i < NumberOfPlayers; ++i) {
		if (player[i].cloaked > 0)
			player[i].cloaked--;
		if (player[i].special) {
			player[i].curFrames++;
			player[i].totFrames++;
			if (player[i].curFrames > player[i].maxFrames)
				player[i].maxFrames = player[i].curFrames;
		} else {
			player[i].curFrames = 0;
		}
	}


	switch (GetCurrentGameType()) {
	case DEATHMATCH:
		for (i = 0; i < NumberOfPlayers; ++i) {
			int kills = 0, pl;
			for (pl = 0; pl < NumberOfPlayers; ++pl) {
				if (pl == i || (player[pl].team && player[pl].team == player[i].team)) {
					for (j = 0; j < NumberOfPlayers; ++j)
						kills+= player[pl].kills[j];
				}
			}
			if (kills >= p->dmParms.fragLimit) {
				// Record the stats
				RecordStats();
				// Set the winner up
				Winner = i;
				SFX_WinGame();
				return TRUE;
			}
		}
		break;
	case KEEP_THE_FLAG:
		for (i = 0; i < NumberOfPlayers; ++i) {
			if (player[i].special) {
				if (player[i].timer)
					player[i].timer--;
				SFX_TimeBeep(player[i].timer);
			}
			if (player[i].special && player[i].timer == 0) {
				Winner = i;
				RecordStats();
				SFX_WinGame();
				return TRUE;
			}
		}
		break;
	case PREDATOR:
		if (IsTeamGame()) {
			int pT;
			// Check we have a predator, if not make one
			num = k = 0;
			for (j = 0; j < NumberOfPlayers; ++j) {
				if (player[j].Player && player[j].active && player[j].Player->var != 1234.f) {
					k++;
					if (player[j].special)
						num++, pT = player[j].team;
				}
			}
			if (num == 0) {
				// No predators
				int team = (rand() % 1) + 1;
				for (j = 0; j < NumberOfPlayers; ++j) {
					if (player[j].team == team && player[j].active && player[j].Player && player[j].Player->var != 1234.f &&
						!player[j].special)
						MakePredator (j);
				}
			} else {
				// Check all live teammates are predators too
				for (j = 0; j < NumberOfPlayers; ++j) {
					if (player[j].team == pT && !player[k].special &&
						player[j].active && player[i].Player && player[j].Player->var != 1234.f)
						MakePredator (j);
				}
			}

		} else {
			// Check we have a predator, if not make one
			num = k = 0;
			for (j = 0; j < NumberOfPlayers; ++j) {
				if (player[j].Player && player[j].active && player[j].Player->var != 1234.f) {
					k++;
					if (player[j].special)
						num++;
				}
			}
			dAssert (num < 2, "Two predators!!!");
			if (num == 0 && k==NumberOfPlayers) {
				num = NextPredator;
				for (j = 0; j < NumberOfPlayers; ++j) {
					if (player[j].Player && player[j].active && player[j].Player->var != 1234.f) {
						num--;
						if (num == 0) {
							MakePredator (j);
							break;
						}
					}
					
				}
			}
		}
		for (i = 0; i < NumberOfPlayers; ++i) {
			int kills = 0, pl;
			for (pl = 0; pl < NumberOfPlayers; ++pl) {
				if (pl == i || (player[pl].team && player[pl].team == player[i].team)) {
					for (j = 0; j < NumberOfPlayers; ++j)
						kills+= player[pl].kills[j];
				}
			}
			if (kills >= p->predatorParms.fragLimit) {
				// Record the stats
				RecordStats();
				// Set the winner up
				Winner = i;
				SFX_WinGame();
				return TRUE;
			}
		}
		break;
	case TAG:
	case SUICIDE_TAG:
		{
			int teamAlive[2];
			Point3 bombPos;
			// Update bomb first
			if (BombCounter) {
				BombCounter--;
				SFX_TimeBeep(BombCounter);
			} else {
				// Get the maximum time
				if (GetCurrentGameType() == TAG)
					j = GetCurrentGameParms()->bombParms.bombTimer;
				else
					j = GetCurrentGameParms()->suicideParms.bombTimer;
				// Blow up anyone with a bomb
				for (i = 0; i < NumberOfPlayers; ++i) {
					if (player[i].special && player[i].Player) {
						player[i].Player->health = 0;
						KillSelf(i);
						bombPos = player[i].Player->pos;
					}
					if (player[i].Player == NULL ||
						player[i].Player->health <= 0.f ||
						player[i].Player->var == 1234.f)
						j-= 30;
				}
				// Clamp to 30 second minimum
				if (j < 30)
					j = 30;
				// Reset bomb counter
				BombCounter = j * FRAMES_PER_SECOND;
				// In the case of tag, choose a new player to be bomb-worthy
				if (GetCurrentGameType() == TAG) {
					float dist = 0;
					int furthest = 0;

					for (i = 0; i < NumberOfPlayers; ++i) {
						if (player[i].Player && player[i].Player->var != 1234.f && player[i].Player->health) {
							float thisDist = pSqrDist (&bombPos, &player[i].Player->pos);
							if (thisDist > dist) {
								dist = thisDist;
								furthest = i;
							}
						}
					}

					player[furthest].special = TRUE;
					/*
					j = 0;
					for (i = 0; i < NumberOfPlayers; ++i)
						if (player[i].Player && player[i].Player->health)
							j++;
						k = rand() % j;
						j = 0;
						for (i = 0; i < NumberOfPlayers; ++i)
							if (player[i].Player && player[i].Player->health) {
								if (j++ == k)
									player[i].special = TRUE;
							}
				*/			
				}
			}
			j = k = 0;
			teamAlive[0] = teamAlive[1] = 0;
			for (i = 0; i < NumberOfPlayers; ++i) {
				if (player[i].lives) {
					j++;
					k = i;
					if (player[i].team)
						teamAlive[player[i].team-1] = 1;
				}
			}
			if (j < 2 || (player[0].team && (teamAlive[0] ^ teamAlive[1]))) {
				RecordStats();
				Winner = k;
				SFX_WinGame();
				return TRUE;
			}
		}
		break;
	case KNOCKOUT:
		{
			int teamAlive[2];
			teamAlive[0] = teamAlive[1] = 0;
			j = hi = 0;
			for (i = 0; i < NumberOfPlayers; ++i) {
				if (player[i].lives) {
					j++;
					if (player[i].team)
						teamAlive[player[i].team-1] = 1;
				}
				if (player[i].framesAlive > hi && player[i].lives) {
					hi = player[i].framesAlive;
					k = i;
				}
			}
			if (j < 2 || (player[0].team && (teamAlive[0] ^ teamAlive[1]))) {
				RecordStats();
				Winner = k;
				SFX_WinGame();
				return TRUE;
			}
		}
		break;
	case KING_OF_THE_HILL:
		{
			int teamKing[2];
			teamKing[0] = teamKing[1] = 0;
			j = k = 0;
			for (i = 0; i < NumberOfPlayers; ++i) {
				if (player[i].Player && player[i].Player->var != 1234.f && player[i].special) {
					j++;
					k = i;
					if (player[i].team)
						teamKing[player[i].team-1] = 1;
				} else
					player[i].timer = 0;
			}
			// J is the number of 'kings' of the hill
			if (Winner == -1) {
				if (j == 0) {
					// No kings, reset counter
					BombCounter = GetCurrentGameParms()->kothParms.winTime * FRAMES_PER_SECOND;
					// Take into account how long the game has gone on
					BombCounter = AdjustTimer (BombCounter);
				} else if (j == 1 || (teamKing[0] ^ teamKing[1])) {
					// One true king only - is this the first frame of their kingdom?
					if (j == 1 && player[k].timer == 0) {
						player[k].Player->health = player[k].maxHealth;
						BombCounter = GetCurrentGameParms()->kothParms.winTime * FRAMES_PER_SECOND;
						BombCounter = AdjustTimer (BombCounter);
						player[k].timer ++;
						SFX_BecomeKing();
					}
					// Update the counter
					BombCounter--;
					SFX_TimeBeep(BombCounter);
					if (BombCounter == 0) {
						RecordStats();
						Winner = k;
						SFX_WinGame();
						return TRUE;
					}
				}
			}
		}
		break;
	}
	return FALSE;
}

static char *TTypeText[] = { "Bronze", "Silver", "Gold", "Custom", "Random" };
static int hiLight;

static float posn[3][16] = {
	{ 2, 4, 5, 6, 8, 9, 10, 11, 12 },
	{ 2, 4, 5, 6, 8, 9, 10, 11, 12, 13 },
	{ 2, 4, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 }
};

static int nHi[3] = { 9, 10, 8 };

static Material barMat = { 0, 0, 0.f, 0.f, MF_FRONT_END };

static Camera *fEndCam;

// Reset the mp block
void ResetGameParms (GameParms *p)
{
	p->bombParms.bombTimer = 60;
	p->dmParms.fragLimit = 5;
	p->koParms.startLives = 3;
	p->kothParms.winTime = 20;
	p->ktfParms.winTime = 30;
	p->predatorParms.fragLimit = 5;
	p->suicideParms.bombTimer = 120;
}

void ResetMPSettings(void)
{
	int i;
	memset (&mpSettings, 0, sizeof(mpSettings));
	mpSettings.game.tournament.nGames = MIN_TOURNAMENT_GAMES;
	mpSettings.weapons = WEAPON_ALL;
	ResetGameParms (&mpSettings.game.skirmish.parms);
	ResetGameParms (&mpSettings.game.team.parms);
	for (i = 0; i < MAX_TOURNAMENT_GAMES; ++i) {
		ResetGameParms (&mpSettings.game.tournament.game[i].parms);
	}
}

// SEFTON EDITABLE HERE :)
// Up to six maps per tournament
// Map numbers for bronze tournament
static int BronzeMaps[] = 
{
	12, 2, 13, 2, 2, 2
};

// Map numbers for silver tournament
static int SilverMaps[] = 
{
	10, 4, 15, 9, 11, 2
};

// Map numbers for gold tournament
static int GoldMaps[] = 
{
	13, 11, 10, 2, 4, 15
};


void SetUpForTournament (TournamentGame *t)
{
	int i, *p, n;
	switch (t->tType) {
	case CUSTOM:
		strcpy (t->name, TTypeText[t->tType]);
		break;
	case FROMVMS:
		break;
	case BRONZE:
		t->nGames = 3;
		t->game[0].gameType = DEATHMATCH;
		t->game[1].gameType = KEEP_THE_FLAG;
		t->game[2].gameType = TAG;
		strcpy (t->name, TTypeText[t->tType]);
		n = asize(BronzeMaps);
		p = BronzeMaps;
		goto flatulence;
	case SILVER:
		t->nGames = 5;
		t->game[0].gameType = DEATHMATCH;
		t->game[1].gameType = SUICIDE_TAG;
		t->game[2].gameType = PREDATOR;
		t->game[3].gameType = KEEP_THE_FLAG;
		t->game[4].gameType = KNOCKOUT;
		strcpy (t->name, TTypeText[t->tType]);
		n = asize(SilverMaps);
		p = SilverMaps;
		goto flatulence;
	case GOLD:
		t->nGames = 6;
		t->game[0].gameType = TAG;
		t->game[1].gameType = KNOCKOUT;
		t->game[2].gameType = PREDATOR;
		t->game[3].gameType = SUICIDE_TAG;
		t->game[4].gameType = KEEP_THE_FLAG;
		t->game[5].gameType = DEATHMATCH;
		strcpy (t->name, TTypeText[t->tType]);
		n = asize(GoldMaps);
		p = GoldMaps;
		// Falls to
flatulence:
		for (i = 0; i < n; ++i) {
			ResetGameParms (&t->game[i].parms);
			t->game[i].map = *p++;
		}
	
		for (; i < MAX_TOURNAMENT_GAMES; ++i) {
			ResetGameParms (&t->game[i].parms);
			t->game[i].map = i;
		}
		break;
	}
}

void KillPlayer (int pNum, int otherPlayer)
{
	int increment;
	// Check for friendly kills
	if (player[pNum].team && player[pNum].team == player[otherPlayer].team)
		increment = -1;
	else
		increment = 1;

	switch (GetCurrentGameType()) {
	default:
	case DEATHMATCH:
		player[pNum].kills[otherPlayer]+=increment;
		break;
	case PREDATOR:
		// If you're not the predator, then don't count the kill
		if (!player[pNum].special) {
			// If the other guy was the predator, then you are now!
			if (player[otherPlayer].special && !player[pNum].special) {
				MakePredator (pNum);
			}
		} else {
			player[pNum].kills[otherPlayer]+=increment;
		}
		break;
	case KEEP_THE_FLAG:
		player[pNum].kills[otherPlayer]+=increment;
		// If the other guy had the flag, you get it now
		if (player[otherPlayer].special) {
			GiveFlagToPlayer (pNum);
		}
		break;
	case KNOCKOUT:
		if (player[otherPlayer].lives == 1)
			SFX_Eliminated();
		// fall thru
	case TAG:
	case SUICIDE_TAG:
		player[pNum].kills[otherPlayer]+=increment;
		if (player[otherPlayer].lives)
			player[otherPlayer].lives--;
		break;
	}
	player[otherPlayer].special = FALSE;
	player[otherPlayer].maxHealth = 100.f * GetHandicap(pNum);
	player[otherPlayer].cloaked = 0;
}

void KillSelf (int pNum)
{
	player[pNum].kills[pNum]--;
	switch (GetCurrentGameType()) {
	case PREDATOR:
		if (!player[pNum].special) {
			// Undo the kill and return
			player[pNum].kills[pNum]++;
			return;
		}
		break;
	case DEATHMATCH:
		break;
	case SUICIDE_TAG:
	case TAG:
		// Round to nearest five seconds alive
		{
			int fiveSex = 5 * FRAMES_PER_SECOND;
			player[pNum].framesAlive = (int)(player[pNum].framesAlive / fiveSex);
			player[pNum].framesAlive = player[pNum].framesAlive * fiveSex;
		}
	case KNOCKOUT:
		if (player[pNum].lives == 1)
			SFX_Eliminated();
		if (player[pNum].lives)
			player[pNum].lives--;
		break;
	}
	if (GetCurrentGameType() != PREDATOR) {
		player[pNum].special = FALSE;
		player[pNum].cloaked = 0;
	} else if (player[pNum].special) {
		// Re-ensure this player is a predator still
		MakePredator (pNum);
	}
	player[pNum].maxHealth = 100.f * GetHandicap(pNum);
}

// Strat glue
void PlayerKillSelf(STRAT *s)
{
	dAssert (s->Player, "Not a player");
	KillSelf (s->Player->n);
}

float PhatBoy = -0.045;
static void MPDrawArrow (float dir, Uint32 colour, float shift)
{
	float y;
	extern ModelContext context;
	mInvert3d (&mCurMatrix, &mWorldToView);
	mPreTranslate (&mCurMatrix, 0, PhatBoy, 0.3f);
	mPreRotateX (&mCurMatrix, -(3.141f + 1.1f));
	mPreRotateZ (&mCurMatrix, dir);
	mPreScale (&mCurMatrix, 0.04f, 0.04f, 0.04f);
	MPArrow->obj->model->material[0].diffuse.colour = colour;
	y = context.midY;
	context.midY += shift;
	rDrawObject (MPArrow->obj);
	context.midY = y;
}

static void PassBomb(int from, int to)
{
	SFX_PassBomb();
	if (IsTeamGame() && GetCurrentGameType() == SUICIDE_TAG) {
		int i;
		for (i = 0; i < NumberOfPlayers; ++i) {
			if (player[i].team == player[from].team) {
				player[i].special = FALSE;
			} else {
				player[i].special = TRUE;
			}
		}
	} else {
		player[to].special = TRUE;
		player[from].special = FALSE;
	}
	player[from].invuln = 30 * 5;
}
void DoMultiScores(int pNum)
{
	char buf[9];
	int num, j, k;
	float dist, t, yShift = ((pNum<<1) >= NumberOfPlayers) ? -24.f : 0.f;
	bool RHS, TOP;
	float rightPos = currentCamera->screenX - 16.f;

	// Print up the player's name
	psSetColour (GetTankColour(pNum));
	psSetAlpha (0.8f);
	psSetPriority(W_LAYER_AMMO - 1000.f);
	mUnit32 (psGetMatrix());
	mPostScale32 (psGetMatrix(), 0.8f, 0.8f);
	RHS = (NumberOfPlayers > 2 && (pNum & 1));
	TOP = !(NumberOfPlayers > 2 && (pNum & 2)); // Change here if we need symmetry in two player game
	if (RHS) {
		float length = psStrWidth(CurProfile[LogToPhys(pNum)].name);
		psDrawString (CurProfile[LogToPhys(pNum)].name, rightPos/0.8f - length, yShift / 0.8f);
	} else {
		psDrawString (CurProfile[LogToPhys(pNum)].name, 4/0.8f, yShift / 0.8f);
	}

	switch (GetCurrentGameType()) {
	case PREDATOR:
	case DEATHMATCH:
		{
			int kills = 0;
			int p;
			for (p = 0; p < NumberOfPlayers; ++p) {
				if (p== pNum || 
					(player[p].team == player[pNum].team && player[pNum].team)) {
					for (j = 0; j < MAX_PLAYER; ++j)
						kills += player[p].kills[j];
				}
			}
			
			sprintf (buf, "%d", kills);
			if (TOP)
				yShift += (PHYS_SCR_Y/2) - 64;
			if (RHS) {
				psDrawString (buf, 4/0.8f, yShift/0.8f);
			} else {
				float length = psStrWidth(buf);
				psDrawString (buf, (rightPos)/0.8f - (length+8), yShift / 0.8f);
			}
		}
		break;
	case KNOCKOUT:
		sprintf (buf, "%d", player[pNum].lives);
		if (TOP)
			yShift += (PHYS_SCR_Y/2) - 64;
		if (RHS) {
			psDrawString (buf, 4/0.8f, yShift/0.8f);
		} else {
			float length = psStrWidth(buf);
			psDrawString (buf, (rightPos)/0.8f - (length+8), yShift / 0.8f);
		}
		break;
	case SUICIDE_TAG:
	case TAG:
		// Check for collision between tanks
		if (player[pNum].Player->flag2 & STRAT2_PLAYERHIT) {
			int other;
			player[pNum].Player->flag2 &=~ STRAT2_PLAYERHIT;
			other = player[pNum].Player->flag2 & 3;
			player[other].Player->flag2 &=~ STRAT2_PLAYERHIT;
			// Quick team-mate check
			if (!(player[pNum].team && player[pNum].team == player[other].team &&
				!mpSettings.game.team.friendlyFire))
			{
				// See whether either has a bomb, and the other not
				if (player[pNum].special) {
					if (!player[other].special &&
						!player[other].invuln) {
						PassBomb(pNum, other);
					}
				} else {
					if (player[other].special &&
						!player[pNum].invuln) {
						PassBomb (other, pNum);
					}
				}
			}
		}
		
		if (!PauseMode && player[pNum].invuln) {
			player[pNum].invuln--;
			if (player[pNum].invuln & 1) {
				player[pNum].Player->flag |= STRAT_HIDDEN;
			} else {
				player[pNum].Player->flag &= ~STRAT_HIDDEN;
			}
		}

		player[pNum].Player->health = MIN (player[pNum].Player->health + 0.5f, player[pNum].maxHealth);

		// Draw the arrow
		if (player[pNum].active && Winner == -1) {
			num = 0;
			dist = 1e9;
			for (j = 0; j < NumberOfPlayers; ++j) {
				if (player[j].Player == NULL || j==pNum || (player[j].team && player[pNum].team == player[j].team) || player[pNum].special == player[j].special || !player[j].active || player[j].Player->var == 1234.f)
					continue;
				t = pSqrDist (&player[pNum].Player->pos, &player[j].Player->pos);
				if (t < dist) {
					dist = t;
					num = j;
				}
			}
			dist = sqrt(dist);
			if (player[pNum].special)
			{
				Point3 p;
				Uint32 col, bonga;
				mLoad (&mWorldToView);
				mPoint3 (&p, &player[num].Player->pos);
				col = GetTankColour (num);
				bonga = currentFrame;
				if (dist > 100.f) {
					// Do bugger all
					bonga = 0;
				} else if (dist > 75.f) {
					bonga>>= 2;
				} else if (dist > 50.f) {
					bonga>>= 1;
				} else if (dist > 25.f) {
					// Flash 30fps
				}
				if (bonga & 1) {
					Colour c;
					c.colour = col;
					c.argb.r = MIN(255, (c.argb.r + 0x20));
					c.argb.g = MIN(255, (c.argb.g + 0x20));
					c.argb.b = MIN(255, (c.argb.b + 0x20));
					col = c.colour;
				}
				MPDrawArrow (- (( 2 * PI) * fatan2(p.x, p.z)), col, yShift);
			}
		}
		break;
	case KING_OF_THE_HILL:
		break;
	case KEEP_THE_FLAG:
		if (player[pNum].active && Winner == -1) {
			Point3 pos;

			// In team mode, teammates can pass the flag by driving
			if (IsTeamGame()) {
				if (player[pNum].special && player[pNum].Player->flag2 & STRAT2_PLAYERHIT) {
					int other;
					player[pNum].Player->flag2 &=~ STRAT2_PLAYERHIT;
					other = player[pNum].Player->flag2 & 3;
					player[other].Player->flag2 &=~ STRAT2_PLAYERHIT;
					// Same team?
					if (player[pNum].team == player[other].team && !player[other].invuln)
					{
						// We can pass the flag provided no invuln action
						player[pNum].special = FALSE;
						player[other].special = TRUE;
						player[other].timer = player[pNum].timer;
						player[other].invuln = player[pNum].invuln = FRAMES_PER_SECOND/2;
					}
				}

			}

			if (!PauseMode && player[pNum].invuln) {
				player[pNum].invuln--;
			}

			// Draw the arrow
			pos = FlagPos;
			num = -1;
			for (j = 0; j < NumberOfPlayers; ++j) {
				if (player[j].special && player[j].Player)
					pos = player[j].Player->pos, num = j;
			}
			if (!player[pNum].special)
			{
				Point3 p;
				mLoad (&mWorldToView);
				mPoint3 (&p, &pos);
			  	MPDrawArrow (- ((2 * PI) * fatan2(p.x, p.z)), GetTankColour(num), yShift);
			}
		}
		break;
	}
	// Update alive counter
	if (player[pNum].lives && Winner == -1 && MPStartupState == MP_IN_GAME)
		player[pNum].framesAlive++;
}

void MPInit(void)
{
	int i, j;
	NumSpawnPoints = 0;
	Winner = -1;
	NextPredator = rand() % NumberOfPlayers;
	BombCounter = 0;
	for (j = 0; j < 4; ++j) {
		extern int LogToPhys (int j);
		extern int CurHandicap[4];
		if (j < NumberOfPlayers)
			player[j].handicap = (float)CurHandicap[LogToPhys(j)] / 100.f;
		player[j].n = j;
		player[j].active = FALSE;
		player[j].Player = NULL;
		for (i = 0; i < MAX_PLAYER; ++i) {
			player[j].kills[i] = 0;
		}
		player[j].shotsFired = player[j].shotsHit = 0;
		player[j].team = 0;
		player[j].framesAlive = player[j].maxFrames = player[j].curFrames = player[j].totFrames = 0;
		player[j].special = player[j].cloaked = FALSE;
		if (InputMode == NORMALINPUT)
			ReseedRandom();
		else
			ResetRandom();
		switch (GetCurrentGameType()) {
		case DEATHMATCH:
			player[j].lives = 0;
			break;
		case KNOCKOUT:
			player[j].lives = GetCurrentGameParms()->koParms.startLives;
			break;
		case TAG:
			player[j].lives = 1;
			break;
		case SUICIDE_TAG:
			player[j].lives = 1;
			break;
		}
		for (i = 0; i < 3; ++i) {
			player[j].lod[i] = MultiTank[MultiTankType[j]][i];
			player[j].lodObject[i] = (Object *)CHeapAlloc(ObjHeap,sizeof(Object));
			newObject(MultiTank[MultiTankType[j]][i]->obj, player[j].lodObject[i], 0);
			setParent(player[j].lodObject[i], NULL);
			player[j].lodObjId[i] = (Object **)CHeapAlloc(ObjHeap,sizeof(Object *) * (player[j].lod[0]->noObjId + 1));
			memset (player[j].lodObjId[i], 0, sizeof(Object *) * (player[j].lod[0]->noObjId + 1));
			setObjId(player[j].lodObjId[i], 
				player[j].lod[i]->objId,
				player[j].lodObject[i],
				player[j].lod[i]->obj,
				player[j].lod[0]->noObjId);
		}
	}
	if (IsTeamGame()) {
		player[0].team = 1;
		player[1].team = 2;
		player[2].team = 1;
		player[3].team = 2;
	} else {
		player[0].team = player[1].team = player[2].team = player[3].team = 0;
	}
	FlagSpawned = FALSE;
}

void MPFinalise (void)
{
	int i, j;
	for (j = 0; j < 4; ++j) {
		for (i = 0; i < 3; ++i) {
			newObjectFree (player[j].lodObject[i]);
			CHeapFree (ObjHeap, player[j].lodObject[i]);
			CHeapFree (ObjHeap, player[j].lodObjId[i]);
		}
	}
}

void IncShotsFired (STRAT *strat)
{
	dAssert (strat->Player, "Not a player!");
	strat->Player->shotsFired++;
}

int FindDeadPlayer (void)
{
	int i;
	if (MPStartupState != MP_IN_GAME &&
		MPStartupState != MP_TRANSITION_TO_GAME)
		return 0;
	for (i = 0; i < NumberOfPlayers; ++i) {
		if (!player[i].active) {
			switch (GetCurrentGameType()) {
			default:
				return (i+1);
			case KNOCKOUT:
			case TAG:
				if (player[i].lives)
					return (i+1);
				break;
			}
		}
	}
	return 0;
}

/*
 * Was this bullet fired by a stealth assassin?
 * Or, was it fired by the King of the Hill?
 */
int  FiredByAssassin(STRAT *strat)
{
	if (!strat->parent || !strat->parent->Player) // Fired by a player?
		return FALSE;
	if (Multiplayer && 
		strat->parent->Player->special && // Make sure the player is 'special'
		(GetCurrentGameType() == PREDATOR || // Predator game
			(GetCurrentGameType() == KING_OF_THE_HILL && strat->parent->Player->timer == 1))) // KOTH and rightful king?
		return TRUE;
	return FALSE;
}

/*
 * Should the spawn block create a flag?
 */
int	 ShallISpawnAFlag (void)
{
	int i;
	if (!Multiplayer || GetCurrentGameType() != KEEP_THE_FLAG || FlagSpawned)
		return FALSE;
	for (i = 0; i < NumberOfPlayers; ++i) {
		if (player[i].special)
			return FALSE;
	}
	// No-one has the flag - spawn it
	return TRUE;
}

/*
 * Give the flag to a player
 */
void GiveFlagToPlayer (int pNum)
{
	player[pNum].special = TRUE;
	player[pNum].timer = AdjustTimer(GetCurrentGameParms()->ktfParms.winTime * FRAMES_PER_SECOND);
	SFX_PassFlag();
}

/*
 * Register the flag as active
 */
void IAmFlag (STRAT *strat, int trueness)
{
	FlagPos = strat->pos;
	FlagSpawned = trueness;
}

/*
 * Reads the multiplayer levels from the stream
 */
char *strdup (char *str)
{
	char *retVal = (char *)syMalloc (strlen(str)+1);
	dAssert (retVal, "Out of RAM");
	return strcpy (retVal, str);
}

char *MPMapName[MAX_MP_MAPS];
char *MPWadName[MAX_MP_MAPS];
Uint32 MPLevOK[MAX_MP_MAPS];
Uint32 MPUnlockCode[MAX_MP_MAPS];
int numMPMaps = 0;

void ReadMPMapNames(Stream *s)
{
	char tempBuffer[256];
	char c, *ptr;
	int nMaps;

	// Skip whitespace
	do {
		sRead (&c, 1, s);
	} while (isspace(c));

	dAssert (c == '[', "Missing open square bracket");

	do {
		// Skip whitespace
		do {
			sRead (&c, 1, s);
		} while (isspace(c));
		if (c == ']')
			break;
		dAssert (c == '\"', "Missing quote");

		// Read the level's name between the quotes

		sRead (&c, 1, s);
		ptr = tempBuffer;
		do {
			*ptr++ = c;
			sRead (&c, 1, s);
		} while (c != '"');
		*ptr = '\0';
		MPMapName[numMPMaps] = strdup(tempBuffer);

		// Skip whitespace
		do {
			sRead (&c, 1, s);
		} while (isspace(c));

		// Read the WAD base name
		ptr = tempBuffer;
		do {
			*ptr++ = c;
			sRead (&c, 1, s);
		} while (!isspace(c));
		*ptr = '\0';
		MPWadName[numMPMaps] = strdup(tempBuffer);

		// Skip whitespace
		do {
			sRead (&c, 1, s);
		} while (isspace(c));

		// Read the flags
		MPLevOK[numMPMaps] = 0;
		do {
			MPLevOK[numMPMaps] |= 1<<(tolower (c) - 'a');
			sRead (&c, 1, s);
		} while (!isspace(c));

		// Skip whitespace
		do {
			sRead (&c, 1, s);
		} while (isspace(c));

		// Read the level number
		MPUnlockCode[numMPMaps] = 0;
		do {
			MPUnlockCode[numMPMaps] = (MPUnlockCode[numMPMaps]*10) + (c - '0');
			sRead (&c, 1, s);
		} while (!isspace(c));

		numMPMaps++;
	} while (1);
}

float GetHandicap (int pNum)
{
	if (Multiplayer) {
		if (IsTeamGame()) {
			int i, nP = 0;
			if (NumberOfPlayers == 2)
				return player[pNum].handicap;
			// Count the number of players on this team
			for (i = 0; i < NumberOfPlayers; ++i) {
				if (player[i].team == player[pNum].team)
					nP++;
			}
			if (NumberOfPlayers == 3) {
				if (nP == 1)
					return player[pNum].handicap * 1.25f;
			} else {
				if (nP == 1)
					return player[pNum].handicap * 1.50f;
			}
			return player[pNum].handicap;
		} else {
			return player[pNum].handicap;
		}
	} else {
		return 1.f;
	}
}

bool DontDieJustSitAround(STRAT *s)
{
	if (!Multiplayer)
		return false;
	if (Winner >= 0)
		return false;
	if (s->Player == NULL)
		return false;
	if (player[s->Player->n].lives == 0 &&
		(GetCurrentGameType() == TAG || GetCurrentGameType() == KNOCKOUT))
		return true;
	else
		return false;
}

bool KingOfTheHillGame(void)
{
	return (GetCurrentGameType() == KING_OF_THE_HILL);
}