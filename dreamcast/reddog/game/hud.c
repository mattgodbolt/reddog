/*
 * HUD.c
 * Head-up display functions
 */

#include "RedDog.h"
#include "Dirs.h"
#include "HUD.h"
#include "camera.h"
#include "View.h"
#include "GameState.h"
#include "Render\Internal.h"
#include "LevelDepend.h"
#include "Layers.h"

#define MISSILE_X	2.2f
#define MISSILE_Y	1.6f
#define MISSILE_GAP	0.4f

#define BAR_HEIGHT	2.2f

#define MP_BAR_HEIGHT 20.f

/*static Object	*ispriteHolder = NULL; */
/*static Object	*adrenalineBar = NULL; */
/*static Object	*missileAmt = NULL; */
/*static PNode	*radarNode = NULL; */
/*static Object	*armourBar = NULL; */
/*
PNode	*spriteHolder = NULL;
PNode	*spriteLock = NULL;	 */
/*static PNode	*adrenalineBar = NULL;
static PNode	*missileAmt = NULL;
static PNode	*radarNode = NULL;
static PNode	*armourBar = NULL;	*/
/*
extern	int	DebugVCollideCount;
extern	int	DebugVCollideMaxCount;
extern	int	DebugVCollideAfterMaxCount;
extern	int	DebugVCollideAfterFromToCount;
extern	int	DebugObjectCollisionCount;
*/
//extern	int	CamNo;

//int nMissiles;
float direction;
extern Material fadeMat, opaqueNonTextured, addMat;
//static const Uint32 dodgyArray[7] = { 1, 3, 4, 5, 7, 2, 6 };

#define GAP 4.f
#define NUM_HUD_POINTS	24
static Point2 theHud[NUM_HUD_POINTS];
int hudDrawList[] =
{
	3,		10, 11, 12,
	4,		0, 1, 5, 4,
	4,		2, 13, 14, 3,
	4,		6, 7, 9, 8,
	4,		15, 16, 17, 18,
	4,		19, 20, 21, 22,
	2,		10, 23,
	0
};
int mpHudDrawList[] =
{
	4,		0, 1, 2, 3,
	4,		4, 5, 6, 7,
	4,		8, 9,10,11,
	0
};

float hudXOffset = 114, hudYOffset = PHYS_SCR_Y - 108;
float hudRot = 0.f;
float hudXScale = 1, hudYScale = 1;
bool hudDone = false, mpState;
static void GenerateHUD(void);
static PKMDWORD HudQuad (register PKMDWORD pkmCurrentPtr, int a, int b, int c, int d, float amount, Uint32 colour, float addX, float addY);

extern Point3	drawTargetArray[64];
extern int		currentDrawTarget;
extern float health;
extern	int	score;
extern int frameCount;
extern void rLockOn(int pNum, int time);

int playerTargeted(int pn, int *n)
{
	int i, j, ct = 0, found = 0;

	for (i=0; i<4; i++)
	{
		if (i == pn)
			continue;

		for (j=0; j<MAX_TARGETS; j++)
		{
			if (player[i].target[j].strat == player[pn].Player)
			{
				if (player[pn].lockedOnTime[j] >= ct)
				{
					ct = player[pn].lockedOnTime[j];
					found = 1;
					*n = i;
				}
			}
		}
	}

#define		PADFIRE			64
#define		PADB			128

	if (found)
	{
		if (PadPress[*n] & PADB)
			return 3;
		else if (PadPress[*n] & PADFIRE)
			return 1;
	}
	else
		return 0;
}

void drawHud(int pNum)
{
	float x;
	Vector3 tempV;
	int bypn = -1, i;
	int *vert, nVerts;
	float w[4], addX, addY;
	/*
	tPrintf(2, 3, "VCC    %d", DebugVCollideCount);
	tPrintf(2, 4, "VCMC   %d", DebugVCollideMaxCount);
	tPrintf(2, 5, "VCAMC  %d", DebugVCollideAfterMaxCount);
	tPrintf(2, 6, "VCAFTC %d", DebugVCollideAfterFromToCount);
	tPrintf(2, 7, "VCOC   %d", DebugObjectCollisionCount);
	tPrintf(2, 8, "GV   %f", globalVar);
	DebugVCollideCount = 0;
	DebugVCollideMaxCount = 0;
	DebugVCollideAfterMaxCount = 0;
	DebugVCollideAfterFromToCount = 0;
	DebugObjectCollisionCount = 0;
	*/
	/*return;*/

	if (!(MPStartupState == MP_IN_GAME || MPStartupState == MP_TRANSITION_TO_GAME))
		return;

	// Some score action
	if (!Multiplayer) {
		char buf[9];
		char *sBuf = buf + 7;
		int whore = score;
		extern short LevelNum;
		char *pleg = buf + 7;
		float shufty;
		buf[8] = '\0';
		while (pleg >= buf) {
			*pleg = '0' + (whore % 10);
			if (whore % 10)
				sBuf = pleg;
			pleg--;
			whore/=10;
		}
		// LEVEL DEPENDENCY
		switch (LevelNum) {
		default:
			shufty = (sBuf - buf) * 24.f;
			tScore (PHYS_SCR_X-24*8-8 + shufty, 32, sBuf, 0xff00ff00);
			break;
		case LEV_DEFENSIVE:
		case LEV_VEHICLE_MANOEUVRING:
		case LEV_ADV_VEHICLE_MANOEUVRING:
			break; 
		}
	} else {
		DoMultiScores (pNum);
	}


	if (Multiplayer)
	{
		if (player[pNum].PlayerSecondaryWeapon.type == ROCKET)
		{
			if ((player[pNum].lostTargetTime > 15) && 
				 player[pNum].playerBButtonHold)
				rNoTarget(pNum);
			if (player[pNum].playerTargetTime)
				rTargetting(pNum, player[pNum].playerTargetTime);
		}

		if (player[pNum].playerTargettedbyMissile == 3)
		{
			rLockOn(pNum, -1);
		}
		else if (playerTargeted(pNum, &bypn) == 3)
		{
			player[pNum].lockedOnTime[bypn]++;
			if (player[pNum].lockedOnTime[bypn] > LOCKON_WAIT)
				player[pNum].lockedOnTime[bypn] = LOCKON_WAIT + 1;
			if (player[bypn].PlayerSecondaryWeapon.type == ROCKET)
				rLockOn(pNum, player[pNum].lockedOnTime[bypn]);
		}
		else
		{
			player[pNum].lockedOnTime[0] = 0;
			player[pNum].lockedOnTime[1] = 0;
			player[pNum].lockedOnTime[2] = 0;
			player[pNum].lockedOnTime[3] = 0;
		}

		for (i=0; i<currentDrawTarget; i++)
			DrawTarget(&drawTargetArray[i], 0.3f);

		if (pNum == NumberOfPlayers - 1)
			currentDrawTarget = 0;
	}
	else
	{
		for (i=0; i<currentDrawTarget; i++)
			DrawTarget(&drawTargetArray[i], 0.3f);

		currentDrawTarget = 0;
	}

	x = 0.5f + (sin(((float)(frameCount)) * 0.01f) / 2.0f);

	if (player[pNum].maxHealth != player[pNum].Player->health)
		player[pNum].Player->health = RANGE (0, player[pNum].Player->health, player[pNum].maxHealth);

//	rHealth(pNum);
//	if (!Multiplayer)
//		rKmxxSetVertex_3(pNum);

	// New HUD drawing action!!!
	if (!hudDone || Multiplayer != mpState) {
		if (Multiplayer) {
			hudXOffset = 24, hudYOffset = 158+48;
			if (NumberOfPlayers == 2)
				hudXScale = 0.6f, hudYScale = 0.6f;
			else
				hudXScale = 0.4f, hudYScale = 0.4f;
		} else {
			hudXOffset = PHYS_SCR_X - 80, hudYOffset = PHYS_SCR_Y - 83;
			hudXScale = 2.f/3.f, hudYScale = 2.f/3.f;
		}
		hudRot = 0;
		GenerateHUD();
	}

	if (!(Multiplayer && NumberOfPlayers > 2 && (pNum & 1))) {
		addX = currentCamera->screenMidX - (currentCamera->screenX/2);
	} else {
		addX = currentCamera->screenMidX + (currentCamera->screenX/2) - 92.f;
	}
	addY = currentCamera->screenMidY - (currentCamera->screenY/2);
	if (Multiplayer && ((NumberOfPlayers == 2 && pNum == 0) || (NumberOfPlayers != 2 && pNum < 2)))
		addY += rGlobalYAdjust;

	// Draw the skeleton of the system
	vert = Multiplayer ? mpHudDrawList : hudDrawList;
	do {
		int i;
		Colour col = { 0xff40ff40 };
		Point2 *lastP, *firstP;
		nVerts = *vert++ - 1;
		lastP = firstP = theHud + *vert++;
		do {
			Point2 *thisP = theHud + *vert++;
			rLine2D (lastP->x + addX, lastP->y + addY, thisP->x + addX, thisP->y + addY, W_LAYER_HUD_LINE, col, col);
			lastP = thisP;
		} while (--nVerts);
		rLine2D (lastP->x + addX, lastP->y + addY, firstP->x + addX, firstP->y + addY, W_LAYER_HUD_LINE, col, col);
	} while (*vert);

	if (!Multiplayer) {
		rSetMaterial (&addMat);
		
		kmxxGetCurrentPtr (&vertBuf);
		kmxxStartVertexStrip (&vertBuf);
		
		// Live indicators
		if (!Multiplayer) {
			if (PlayerLives >= 2) {
				kmxxSetVertex_0 (0xe0000000, theHud[23].x + addX, theHud[23].y + addY, W_LAYER_HUD_SOLID, 0xffffff00);
				kmxxSetVertex_0 (0xe0000000, theHud[12].x + addX, theHud[12].y + addY, W_LAYER_HUD_SOLID, 0xffffff00);
				kmxxSetVertex_0 (0xf0000000, theHud[10].x + addX, theHud[10].y + addY, W_LAYER_HUD_SOLID, 0xffffff00);
			}
			if (PlayerLives >= 1) {
				kmxxSetVertex_0 (0xe0000000, theHud[10].x + addX, theHud[10].y + addY, W_LAYER_HUD_SOLID, 0xffffff00);
				kmxxSetVertex_0 (0xe0000000, theHud[11].x + addX, theHud[11].y + addY, W_LAYER_HUD_SOLID, 0xffffff00);
				kmxxSetVertex_0 (0xf0000000, theHud[23].x + addX, theHud[23].y + addY, W_LAYER_HUD_SOLID, 0xffffff00);
			}
		}
		{
			static float flashRot = 0;
			Uint32 colour = 0xffff2020;
			float freq = RANGE(0, player[0].engineHeat, 1) * 0.7f;
			float amt = RANGE(0, player[0].engineHeat, 1);
			flashRot += freq;
			if (flashRot > (2*PI))
				flashRot -= (2*PI);
			amt = amt * (sin(flashRot)*0.5f + 0.5f);
			colour = 0xff000000 | ColLerp (0xcf1010, 0xffffff, amt);
			pkmCurrentPtr = HudQuad (pkmCurrentPtr, 19, 22, 20, 21, player[pNum].Player ? player[pNum].Player->health / (float)player[pNum].maxHealth: 0, colour, addX, addY);
		}
		// Shield
		{
			float bottom, top;
			Uint32 col;
			bottom = player[pNum].PlayerShieldEnergy * (1.0 / SHIELD_CUTOFF);
			top = (player[pNum].PlayerShieldEnergy - SHIELD_CUTOFF) * (1.0 / (1.0 - SHIELD_CUTOFF));
			col = 0xff0080ff;
			if (bottom < 1.0f) {
				float madness;
				madness = 0.5f + 0.5f * sin((currentFrame & 15) * PI / 8.f);
				if (bottom > 0.9f) {
					madness *= (1.f - bottom) * 10.f;
				}
				col = ColLerp (0x0080ff, 0xffffff, madness) | 0xff000000;
			}
			pkmCurrentPtr = HudQuad (pkmCurrentPtr, 17, 16, 18, 15, bottom, col, addX, addY);
			pkmCurrentPtr = HudQuad (pkmCurrentPtr, 14, 13, 3, 2, top, 0xff0080ff, addX, addY);
		}
		// Gun juice
		if (player[0].reward & CHEAT_INF_CHARGE)
			pkmCurrentPtr = HudQuad (pkmCurrentPtr, 8, 9, 6, 7, 1.f, 0xff10ff20, addX, addY);
		else
			pkmCurrentPtr = HudQuad (pkmCurrentPtr, 8, 9, 6, 7, player[pNum].PlayerWeaponEnergy * 0.5f, 0xff10ff20, addX, addY);
		{
			Uint32 l1, l2;
			l1 = 0xfff06b00;
			l2 = 0xff0011f3;
			if (player[pNum].PlayerWeaponCharge < 1.f) {
				// Level one weapon
				pkmCurrentPtr = HudQuad (pkmCurrentPtr, 4, 5, 0, 1, player[pNum].PlayerWeaponCharge, l1, addX, addY);
			} else {
				// Level one weapon
				pkmCurrentPtr = HudQuad (pkmCurrentPtr, 1, 0, 5, 4, 2.f - player[pNum].PlayerWeaponCharge, l1, addX, addY);
				// Level two weapon
				pkmCurrentPtr = HudQuad (pkmCurrentPtr, 4, 5, 0, 1, player[pNum].PlayerWeaponCharge - 1.f, l2, addX, addY);
			}
		}
		kmxxReleaseCurrentPtr (&vertBuf);
	} else {
		int amt = player[pNum].PlayerSecondaryWeapon.amount;
		extern void rSecondaryWeapon(int a, float x0, float y0, float x1, float y1);
		extern void tAmmo (float x, float y, int amt);
		float argonautOnTheStockExchange;

		if (player[pNum].PlayerSecondaryWeapon.type == BOMB &&
			player[pNum].PlayerArmageddon == 1)
			amt--;

		rSetMaterial (&addMat);
		
		kmxxGetCurrentPtr (&vertBuf);
		kmxxStartVertexStrip (&vertBuf);

		pkmCurrentPtr = HudQuad (pkmCurrentPtr, 3, 0, 2, 1, 
			player[pNum].Player ? player[pNum].Player->health / (float)player[pNum].maxHealth: 0 , 0xffff2020, addX, addY);

		argonautOnTheStockExchange = RANGE(0, player[pNum].PlayerShieldEnergy * 4, 1);

		pkmCurrentPtr = HudQuad (pkmCurrentPtr, 4, 5, 7, 6, 
			argonautOnTheStockExchange, 0xff0080ff, addX, addY);

		argonautOnTheStockExchange = RANGE(0, (player[pNum].PlayerShieldEnergy-0.25) * 4.f/3.f, 1);

		pkmCurrentPtr = HudQuad (pkmCurrentPtr, 8, 9, 11, 10, 
			argonautOnTheStockExchange, 0xff0080ff, addX, addY);

		kmxxReleaseCurrentPtr (&vertBuf);

		if (player[pNum].PlayerSecondaryWeapon.type != 0) {
			rSecondaryWeapon(player[pNum].PlayerSecondaryWeapon.type, 
				theHud[12].x + addX, theHud[12].y + addY,
				theHud[13].x + addX, theHud[13].y + addY);
			if (amt)
				tAmmo (theHud[13].x + addX - 8.f, theHud[13].y + addY - 16.f, amt);
		}
	}


	v3Sub(&tempV, &player[pNum].Player->pos, &player[pNum].Player->oldPos);
	v3Scale(&tempV, &tempV, 3.6f*FRAMES_PER_SECOND);
#if SHOWSPEED
	tPrintf(0, 2, "speed %3.0f KM/H", sqrt(v3Dot(&tempV, &tempV)));
#endif
}

#define SET(AA,BB,CC) theHud[AA].x = (BB); theHud[AA].y = (CC)
#define OFFSET(AA,BB,CC,DD) theHud[AA].x = theHud[BB].x + (CC); theHud[AA].y = theHud[BB].y + (DD)
#define REFLECT(AA,BB) theHud[AA].x = -theHud[BB].x; theHud[AA].y = theHud[BB].y
#define DO_GAP(AA,BB,CC,GAPLEN) \
	dx = theHud[CC].x - theHud[BB].x; \
	dy = theHud[CC].y - theHud[BB].y; \
	rLen = isqrt(SQR(dx)+SQR(dy)); \
	dx *= rLen; dy *= rLen; \
	theHud[AA].x = theHud[BB].x + (GAPLEN) * dx; \
	theHud[AA].y = theHud[BB].y + (GAPLEN) * dy

// Fill in the HUD structure
static void GenerateHUD(void)
{
	float m1, m2, c1, c2;
	float x, y, dx, dy, rLen;
	int i;
	SinCosVal val;

	SinCos (hudRot, &val);
	if (hudRot = 0) {
		val.sin = 0.f;
		val.cos = 1.f;
	}

	if (!Multiplayer) {
		float sizeAdj;

		// Central triangle
		SET(10, 0, -70);
		SET(11, -50, 30);
		SET(12, 50, 30);
		
		// Point 1 is diagonally up left from point 10
		OFFSET(1, 10, -GAP, -GAP);
		// Point 0 is 10 units above point 1
		OFFSET(0, 1, 0, -10);
		// Points 2 and 3 are reflections of 1 and 2
		REFLECT(2, 0);
		REFLECT(3, 1);
		
		// Point 5 is 130 units in the direction 10>11 from 1
		dx = theHud[11].x - theHud[10].x;
		dy = theHud[11].y - theHud[10].y;
		rLen = isqrt(SQR(dx)+SQR(dy));
		dx *= rLen;
		dy *= rLen;
		theHud[5].x = theHud[1].x + 130 * dx;
		theHud[5].y = theHud[1].y + 130 * dy;
		
		// Point 4 is twenty units up left from 5
		OFFSET(4, 5, -20, -20);
		
		// Point 9 is behind the line 5<->1
		DO_GAP(9, 5, 1, -2*GAP-20);
		
		// Point 8 is on the intersection of lines 1> 0<->4 and 2> [0,100]<->9
		m1 = (theHud[4].y - theHud[0].y) / (theHud[4].x - theHud[0].x);
		m2 = (theHud[9].y - 100.f)       / (theHud[9].x - 0);
		c1 = theHud[4].y - m1 * theHud[4].x;
		c2 = theHud[9].y - m2 * theHud[9].x;
		x = (c2 - c1) / (m1 - m2);
		y = m1 * x + c1;
		theHud[8].x = x;
		theHud[8].y = y;
		
		// Point 6 is a bit along the line 4<->8
		DO_GAP(6, 4, 8, 2*GAP);
		// Point 7 is a bit along the line 5<->9
		DO_GAP(7, 5, 9, 2*GAP);
		
		// Reflections for 16 and 17
		REFLECT(16, 8);
		REFLECT(17, 9);
		
		// Time to shuffle 4,6 and 7, 5 up their lines a bit (35 units in fact)
		dx = theHud[1].x - theHud[5].x;
		dy = theHud[1].y - theHud[5].y;
		rLen = isqrt(SQR(dx)+SQR(dy));
		dx *= rLen;
		dy *= rLen;
		theHud[5].x += dx * 35.f;
		theHud[5].y += dy * 35.f;
		theHud[7].x += dx * 35.f;
		theHud[7].y += dy * 35.f;
		
		dx = theHud[0].x - theHud[4].x;
		dy = theHud[0].y - theHud[4].y;
		rLen = isqrt(SQR(dx)+SQR(dy));
		dx *= rLen;
		dy *= rLen;
		theHud[4].x += dx * 35.f;
		theHud[4].y += dy * 35.f;
		theHud[6].x += dx * 35.f;
		theHud[6].y += dy * 35.f;

		// OK now move 15 and 18 to be SHIELD_CUTOFF up the line 17->3 and 16->2 respectively
		{
			float shieldCutoff = SHIELD_CUTOFF;
			if (IsLoading())
				shieldCutoff = 0.5f;
			dx = theHud[3].x - theHud[17].x;
			dy = theHud[3].y - theHud[17].y;
			theHud[18].x = theHud[17].x + shieldCutoff * dx;
			theHud[18].y = theHud[17].y + shieldCutoff * dy;
			dx = theHud[2].x - theHud[16].x;
			dy = theHud[2].y - theHud[16].y;
			theHud[15].x = theHud[16].x + shieldCutoff * dx;
			theHud[15].y = theHud[16].y + shieldCutoff * dy;
		}

		// 14 and 13 are a bit up 18->3 etc
		DO_GAP(14, 18, 3, 2*GAP);
		DO_GAP(13, 15, 2, 2*GAP);
		
		// Bottom bit: 19 is a little behind the line 11<->10
		DO_GAP(19, 11, 10, -2*GAP);
		// 22 is further behind than that
		if (player[0].maxHealth < UPGRADE_ARMOUR_1_HEALTH)
			sizeAdj = 15;
		else if (player[0].maxHealth < UPGRADE_ARMOUR_2_HEALTH)
			sizeAdj = 23;
		else
			sizeAdj = 34;
		DO_GAP(22, 11, 10, -2*GAP - sizeAdj);
		// And 20 and 21 are reflections
		REFLECT(20, 19);
		REFLECT(21, 22);
		
		// Point 23 is bottom of the life indicator
		SET(23, 0, theHud[11].y);
	} else {
		// Multiplayer HUD, such as it is
		SET(0, 2*GAP, 0);
		OFFSET(1, 0, 130, 0);
		OFFSET(2, 1, -MP_BAR_HEIGHT, -MP_BAR_HEIGHT);
		OFFSET(3, 0, MP_BAR_HEIGHT, -MP_BAR_HEIGHT);

		SET(4, 0, -2*GAP);
		OFFSET(5, 4, MP_BAR_HEIGHT, -MP_BAR_HEIGHT);
		OFFSET(7, 4, 0, -130*0.25f);
		OFFSET(6, 7, MP_BAR_HEIGHT, 0);

		OFFSET(8, 7, 0, -8);
		OFFSET(9, 6, 0, -8);
		OFFSET(11, 4, 0, -130);
		OFFSET(10, 11, MP_BAR_HEIGHT, MP_BAR_HEIGHT);

		SET(12,15 + 2 * GAP, -15 - 2 * GAP);
		OFFSET(13, 12, 100, -100);

	}

	// Now to translate
	for (i = 0; i < NUM_HUD_POINTS; ++i) {
		float x, y;
		x = theHud[i].x * hudXScale;
		y = theHud[i].y * hudYScale;
		theHud[i].x =  val.cos * x - val.sin * y + hudXOffset;
		theHud[i].y =  val.sin * x + val.cos * y + hudYOffset;
	}

	hudDone = true;
	mpState = Multiplayer;
}

static PKMDWORD HudQuad (register PKMDWORD pkmCurrentPtr, int a, int b, int c, int d, 
						 float amount, Uint32 colour, float addX, float addY)
{
	float dx1, dy1, dx2, dy2;
	float x1, y1, x2, y2;
	amount = RANGE(0, amount, 1);

	if (amount == 0.f)
		return pkmCurrentPtr;

	if (amount == 1.f) {
		kmxxSetVertex_0 (0xe0000000, theHud[a].x + addX, theHud[a].y + addY, W_LAYER_HUD_SOLID, colour);
		kmxxSetVertex_0 (0xe0000000, theHud[b].x + addX, theHud[b].y + addY, W_LAYER_HUD_SOLID, colour);
		kmxxSetVertex_0 (0xe0000000, theHud[c].x + addX, theHud[c].y + addY, W_LAYER_HUD_SOLID, colour);
		kmxxSetVertex_0 (0xf0000000, theHud[d].x + addX, theHud[d].y + addY, W_LAYER_HUD_SOLID, colour);
	} else {
		dx1 = theHud[c].x - theHud[a].x;
		dy1 = theHud[c].y - theHud[a].y;
		
		dx2 = theHud[d].x - theHud[b].x;
		dy2 = theHud[d].y - theHud[b].y;
		
		x1 = theHud[a].x + dx1 * amount + addX;
		y1 = theHud[a].y + dy1 * amount + addY;
		
		x2 = theHud[b].x + dx2 * amount + addX;
		y2 = theHud[b].y + dy2 * amount + addY;

		kmxxSetVertex_0 (0xe0000000, theHud[a].x + addX, theHud[a].y + addY, W_LAYER_HUD_SOLID, colour);
		kmxxSetVertex_0 (0xe0000000, theHud[b].x + addX, theHud[b].y + addY, W_LAYER_HUD_SOLID, colour);
		kmxxSetVertex_0 (0xe0000000, x1, y1, W_LAYER_HUD_SOLID, colour);
		kmxxSetVertex_0 (0xf0000000, x2, y2, W_LAYER_HUD_SOLID, colour);
	}
	return pkmCurrentPtr; 
}

void DrawHudSymbol(float amt, float addX, float addY)
{
	int *vert, nVerts;
	float w[4];
	bool savedMP;

	hudXOffset = hudYOffset = 0;
	hudXScale = hudYScale = 2.f / 3.f;
	savedMP = Multiplayer;
	Multiplayer = false;
	player[0].maxHealth = 100.f;
	GenerateHUD();
	Multiplayer = savedMP;

	vert = hudDrawList;
	do {
		int i;
		Colour col = { 0xff40ff40 };
		Point2 *lastP, *firstP;
		nVerts = *vert++ - 1;
		lastP = firstP = theHud + *vert++;
		do {
			Point2 *thisP = theHud + *vert++;
			rLine2D (lastP->x + addX, lastP->y + addY, thisP->x + addX, thisP->y + addY, 5, col, col);
			lastP = thisP;
		} while (--nVerts);
		rLine2D (lastP->x + addX, lastP->y + addY, firstP->x + addX, firstP->y + addY, 5, col, col);
	} while (*vert);

	rSetMaterial (&opaqueNonTextured);

	kmxxGetCurrentPtr (&vertBuf);
	kmxxStartVertexStrip (&vertBuf);

	pkmCurrentPtr = HudQuad (pkmCurrentPtr, 19, 22, 20, 21, amt, 0xffff2020, addX, addY);

	kmxxReleaseCurrentPtr (&vertBuf);

	hudDone = false;
}

#define STRATBAR_START_COLOUR	0xffff00
#define STRATBAR_END_COLOUR		0xff0000

void DrawStratBar(bool Enemy, float amount, float flash, float alpha)
{
	float x;
	Colour col = { 0xff40ff40 };
	Uint32 barCol, barCol2;
	static float lastAmount[2] = { 0, 0 };
	static float flashAdd[2] = { 0, 0 };
	float left, top, height, width;

	// No strat bars on black barred cameras
	if ((currentCamera->flags & CAMERA_BLACKBARS))
		return;

	if (lastAmount[Enemy] != amount) {
		flashAdd[Enemy] += 0.75f;
		flashAdd[Enemy] = RANGE(0, flashAdd[Enemy], 1);
	}
	lastAmount[Enemy] = amount;

	flash = RANGE (0, flash+flashAdd[Enemy], 1);
	amount = RANGE (0, amount, 1);
	alpha = RANGE(0, alpha, 1);
	flashAdd[Enemy] *= 0.7f;

	if (!Enemy) {
		float bAmt = 0.333f;
		float width1, width2, left2, skew, am1, am2;
		skew = 16.f;
		left = 16.f;
		top = PHYS_SCR_Y - 64.f - 12.f;
		height = 32.f;
		width = 200.f;
		x = (amount * width);
		width1 = width * bAmt - 4.f;
		width2 = (width * (1.f-bAmt)) - 4.f;
		left2 = left + width1 + 8;
		// Draw the HUD outline
		rLine2D (left+skew, top, left+width1+skew, top, W_LAYER_HUD_LINE, col, col);
		rLine2D (left+width1+skew, top, left+width1, top+height, W_LAYER_HUD_LINE, col, col);
		rLine2D (left+width1, top+height, left, top+height, W_LAYER_HUD_LINE, col, col);
		rLine2D (left, top+height, left+skew, top, W_LAYER_HUD_LINE, col, col);
		
		rLine2D (left2+skew, top, left2+width2+skew, top, W_LAYER_HUD_LINE, col, col);
		rLine2D (left2+width2+skew, top, left2+width2, top+height, W_LAYER_HUD_LINE, col, col);
		rLine2D (left2+width2, top+height, left2, top+height, W_LAYER_HUD_LINE, col, col);
		rLine2D (left2, top+height, left2+skew, top, W_LAYER_HUD_LINE, col, col);

		// Draw the inside bit
		rSetMaterial (&addMat);
		
		kmxxGetCurrentPtr (&vertBuf);
		kmxxStartVertexStrip (&vertBuf);

		am1 = RANGE(0, amount/bAmt, 1);
		am2 = RANGE(0, (amount-bAmt)/(1-bAmt), 1);

		x = width1 * am1;
		barCol = ColLerp (STRATBAR_END_COLOUR, 0xffffff, flash) | 0xff000000;
		if (am1 < 0.99f)
			barCol = ColLerp (barCol, 0xffffff, 0.5f + sin(currentFrame*0.01f) * 0.5f) | 0xff000000;
			
		kmxxSetVertex_0 (0xe0000000, left+skew, top, W_LAYER_HUD_SOLID, barCol);
		kmxxSetVertex_0 (0xe0000000, left, top+height, W_LAYER_HUD_SOLID, barCol);
		kmxxSetVertex_0 (0xe0000000, left+x+skew, top, W_LAYER_HUD_SOLID, barCol);
		kmxxSetVertex_0 (0xf0000000, left+x, top+height, W_LAYER_HUD_SOLID, barCol);

		if (am2 > 0) {
			x = width2 * am2;
			barCol2 = ColLerp (STRATBAR_END_COLOUR, STRATBAR_START_COLOUR, am2) | 0xff000000;
			barCol2 = ColLerp (barCol2, 0xffffff, flash) | 0xff000000;
			
			kmxxSetVertex_0 (0xe0000000, left2+skew, top, W_LAYER_HUD_SOLID, barCol);
			kmxxSetVertex_0 (0xe0000000, left2, top+height, W_LAYER_HUD_SOLID, barCol);
			kmxxSetVertex_0 (0xe0000000, left2+x+skew, top, W_LAYER_HUD_SOLID, barCol2);
			kmxxSetVertex_0 (0xf0000000, left2+x, top+height, W_LAYER_HUD_SOLID, barCol2);
		}

		kmxxReleaseCurrentPtr (&vertBuf);
	} else {
		float left = 20.f;
		float top = 48.f;
		float height = PHYS_SCR_Y - 32 - top;
		float width = 48.f;
		float skew = 48.f;
		float y1, y2;

			// Draw the HUD outline
		rLine2D (left, top, left+width, top, 5, col, col);
		rLine2D (left+width, top, left+width, top+height-skew, 5, col, col);
		rLine2D (left+width, top+height-skew, left, top+height, 5, col, col);
		rLine2D (left, top+height, left, top, 5, col, col);

		barCol = ColLerp (STRATBAR_START_COLOUR, STRATBAR_END_COLOUR, 1-amount);
		barCol = ColLerp (barCol, 0xffffff, flash) | 0xff000000;

		barCol2 = ColLerp (STRATBAR_END_COLOUR, 0xffffff, flash) | 0xff000000;

		rSetMaterial (&addMat);
		
		kmxxGetCurrentPtr (&vertBuf);
		kmxxStartVertexStrip (&vertBuf);

		y1 = top + height * (1-amount);
		y2 = top + (height-skew) * (1-amount);

		kmxxSetVertex_0 (0xe0000000, left, y1, 4.9, barCol);
		kmxxSetVertex_0 (0xe0000000, left, top+height, 4.9, barCol2);
		kmxxSetVertex_0 (0xe0000000, left+width, y2, 4.9, barCol);
		kmxxSetVertex_0 (0xf0000000, left+width, top+height-skew, 4.9, barCol2);

		kmxxReleaseCurrentPtr (&vertBuf);
	}
}

