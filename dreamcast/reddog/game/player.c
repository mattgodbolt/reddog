#include "RedDog.h"
#include "command.h"
#include "process.h"
#include "input.h"
#include "view.h"
#include "camera.h"
#include "Player.h"
#include "coll.h"
#include "MPlayer.h"
#include "Render\Internal.h"
#include "GameState.h"

#define MAX_NORMAL_SPEED 35.0f /*mph*/
#define MAX_BOOST_SPEED 70.0f  /*mph*/
#define MAX_ANG_NORMAL 30		/*deg*/	
#define MAX_ANG_BOOST  40		/*deg*/
#define MAX_ANG_LOOP  40		/*deg*/
#define PFINC	0.05f
#define PFDEC	0.05f
#define OUTERBOOSTSCALE 0.01f
#define INNERBOOSTSCALE 0.01f
extern float smoothSideStepSum(int pn);
extern	float Gravity;

//#define FRAMES_PER_SECOND FRAMES_PER_SECOND
STRAT *HoldPlayerMain;

static float playerOuterBoostStartScale[50] = {
									112.484, 112.484, 255.219, 149.097, 379.463, 182.223, 
									485.216, 211.862, 572.479, 238.014, 641.251, 260.679, 
									691.532, 279.857, 723.322, 295.548, 736.622, 307.752, 
									731.431, 316.469, 707.749, 321.699, 650.368, 322.571, 
									604.914, 321.699, 639.880, 321.699, 671.455, 321.699, 
									608.942, 321.699, 541.496, 321.699, 550.228, 323.335, 
									568.570, 321.699, 552.615, 313.710, 516.622, 299.179, 
									458.895, 272.626, 349.600, 216.762, 199.060, 138.568,
									065.809, 068.971
								};
static float playerOuterBoostEndScale[18] = {	
									248.121, 167.266, 541.479, 321.702, 530.455, 306.750,
									437.349, 247.463, 377.998, 211.228, 313.790, 175.583,
									244.724, 140.528, 170.801, 106.062, 092.021, 072.186
								};

static float playerInnerBoostStartScale[60] = {
									100.000, 100.000, 100.000, 100.000, 100.000, 100.000,
									100.000, 100.000, 100.000, 100.000, 100.000, 100.000,
									100.000, 100.000, 100.000, 100.000, 100.000, 100.000,
									100.000, 100.000, 100.000, 100.000, 100.000, 100.000,
									103.057, 103.578, 111.641, 113.553, 124.874, 128.788, 
									141.877, 148.145, 161.772, 170.487, 183.680, 194.677,
									206.723, 219.578, 230.022, 244.052, 252.699, 266.962,
									273.875, 287.170, 292.672, 303.540, 308.211, 314.933,
									319.614, 320.213, 326.002, 318.242, 324.876, 308.271,
									319.413, 293.582, 317.244, 279.087, 326.002, 269.696
								};

static float playerInnerBoostMiddleScale[14] = {	
									355.981, 267.384, 394.343, 268.540, 414.021, 269.696,
									396.109, 269.696, 359.513, 269.696, 333.888, 269.696,
									334.989, 269.696
								};

static float playerInnerBoostEndScale[28] = {	
									418.698, 269.696, 388.445, 269.696, 357.971, 269.696, 
									326.425, 269.696, 295.402, 269.696, 264.798, 269.696, 
									236.539, 269.696, 208.228, 286.726, 182.262, 303.756,
									167.759, 269.695, 167.759, 167.513, 176.684, 115.255, 
									167.759, 084.602, 114.211, 047.234
								};



PLAYER	player[4];
int		currentPlayer=0;
JPOINT	jumpPoint[32];
int		noJumpPoints = 0, noSwarmTargets = 0;
STRAT	*swarmStratTargetArray[20];
int		swarmObjectTargetArray[20];

static void crntToIdlePos(Object *obj, float amount)
{
 	if (PAL)
 		amount *= PAL_MOVESCALE;
	obj->crntPos.x += amount *(obj->idlePos.x - obj->crntPos.x);
	obj->crntPos.y += amount *(obj->idlePos.y - obj->crntPos.y);
	obj->crntPos.z += amount *(obj->idlePos.z - obj->crntPos.z);
}


static void polyTypeCheck(int pn)
{	
	int	camno;

	// Set defaults
	if (player[pn].Player)
		if (GET_SURF_TYPE(player[pn].Player->polyType) != AIR)
			player[pn].ThonGravity = 1.f;
	if (Multiplayer && GetCurrentGameType() == KING_OF_THE_HILL)
		player[pn].special = FALSE;

	// Check the surface type for deviation from defaults
	switch (GET_SURF_TYPE(player[pn].Player->polyType)) {
	case ICE:
		player[pn].DoPlayerFriction = 0.025f;
		break;
	case SNOW:
		player[pn].DoPlayerFriction = 0.05f;
		break;
	case DANGER:
		player[pn].Player->health -= 3.0f;
		// For MP stuff
		if (player[pn].Player->health < 0.005f) {
			KillSelf (pn);
		}
		break;
	case REALGRAVITY:
		player[pn].ThonGravity = 0.5f;
		break;
	case KOTH:
		if (Multiplayer && GetCurrentGameType() == KING_OF_THE_HILL)
			player[pn].special = TRUE;
	default:
	 	break;
	}

	if ((player[pn].Player->polyType & CAMERA) || 
		((player[pn].oldPolyType & CAMERA) && (GET_SURF_TYPE(player[pn].Player->polyType) == AIR)))
	{
		camno = (player[pn].Player->polyType)>>10; 								   

		player[pn].CamPosOff.x = player[pn].CamPosOff.y = 0;
		player[pn].CamPosOff.z = -(CamModArray[camno].amount) * (player[pn].Player->m.m[1][2]);
		player[pn].CamLookOff.z = -(player[pn].CamPosOff.z /0.5f);
	}
	else
	{
		player[pn].CamPosOff.z = 0.0f;
		player[pn].CamLookOff.z =0.0f;
	}
}

static void playerAccForward(int pn, float	amount)
{
	Vector3	force, cp;
	STRAT *strat;

 	if (PAL)
 		amount *= PAL_ACCSCALE;

	
	if (pn == 0)
	{
		if (player[0].onBoat == 1)
			strat = BoatStrat;
		else
			strat = player[0].Player;
	}
	else
		strat = player[pn].Player;

	if ((player[pn].zooming == 0.0f) || !(PadPress[pn] & PADB))
	{
		force.x = amount * strat->pnode->mass * strat->m.m[1][0];
		force.y = amount * strat->pnode->mass * strat->m.m[1][1];
		force.z = amount * strat->pnode->mass * strat->m.m[1][2];
		v3Scale(&force, &force, 0.01f);
		cp.x = cp.y = cp.z = 0.0f;
		ApplyForce(strat, &force, &cp, 1.0f);
	}
} 



#define PROCESS_ANG(a) (1.0 - cos (a * 0.01745329252))

void playerBotControl(int pn)
{
	Matrix m;

	if (!Multiplayer)
	{
		player[pn].leftGunPos.x = -3.0f;
		player[pn].leftGunPos.y = 2.0f;
		player[pn].leftGunPos.z = 0.0f;
		player[pn].rightGunPos.x = 3.0f;
		player[pn].rightGunPos.y = 2.0f;
		player[pn].rightGunPos.z = 0.0f;
		player[pn].topGunPos.x = 0.0f;
		player[pn].topGunPos.y = 2.0f;
		player[pn].topGunPos.z = 2.6f;

		if (player[0].PlayerState == PS_ONTRAIN)
		{
			m = player[pn].Player->m;
			mPreRotateZ(&m, player[0].Player->objId[5]->crntRot.z);
		}
		else
		{
			m = player[pn].Player->m;
		
			if (player[pn].playerSkidDir)
				mPreRotateZ(&m, smoothSideStepSum(pn));
			else
				mPreRotateZ(&m, -smoothSideStepSum(pn));
		}

		mPoint3Apply(&player[pn].leftGunPos, &m);
		mPoint3Apply(&player[pn].rightGunPos, &m);
		mPoint3Apply(&player[pn].topGunPos, &m);
		v3Inc(&player[pn].leftGunPos, &player[pn].Player->pos);
		v3Inc(&player[pn].rightGunPos, &player[pn].Player->pos);
		v3Inc(&player[pn].topGunPos, &player[pn].Player->pos);
	}
}

static float	limitValue(float val, float topVal)
{
	float	newval;

	/*newval = 0.5f * (1.0f - myPow(((val / topVal) - 1.0f), 0.2f));*/
	newval = RANGE(0.0f, val, topVal);
	newval = val / topVal;
	newval *= newval;
	newval *= newval;
	newval *= newval;
	newval = 1.0f - newval;

	return RANGE(0.0f, newval, 1.0f);
}

void RedDogMoveProcess(int pn)
{
	float rotA, lv, tempf;
	int		atw = 0, i;
	Vector3	tempv;
	Point3	tempp, *p1, *p2;
	Matrix m;
	STRAT *tempStrat, *strat;
	HDStrat	*hdb;
	float SCALEFAC;

	if (PAL)
		SCALEFAC = PAL_MOVESCALE;
	else
		SCALEFAC = 1.0f;
	if (player[pn].onBoat == 1)
		strat = BoatStrat;
	else
		strat = player[pn].Player;
	/*if (!Multiplayer)
		player[pn].Player->health = 100.0f;*/

	if (player[pn].zooming != 0.0f)
	{
		player[pn].DoPlayerFriction = 1.0;
		strat->friction.x = 0.0f;
		strat->friction.y = 0.0f;
		strat->friction.z = 0.0f;
	}

	if (player[pn].Player->health <= 0.0f)
		return;

	// Under water check for camera
	/*if (player[pn].CameraStrat->flag & STRAT_UNDER_WATER) {
		rFlareR += 20.f/256.f;
		rFlareG += 39.f/256.f;
		rFlareB += 45.f/256.f;
	}*/

	/*if (PadPush[pn] & PADFIRE)
	{
		tempv.x = 0.0f;
		tempv.y = 0.0f;
		tempv.z = 1.0f;
		addSpark(100, &player[0].Player->pos, &tempv, 0.25, 2.5f);
	}*/

	if (strat == BoatStrat)
	{
		if (strat->objId[1]->collFlag & COLL_ON_WATER) 
		{
			atw = 4;
			player[0].Player->polyType = ROCK;
			strat->polyType = ROCK;
		}
	}
	else
	{
		for (i=1; i<5; i++)
			if (player[pn].Player->objId[i]->collFlag & COLL_ON_FLOOR)
				atw++;
	}

	player[pn].PlayerBreaking = 0;
	player[pn].PlayerWheelSpin = 0;

	player[pn].playerSideStep = 0;

	if (player[pn].PlayerOnHoldPlayer)
	{
		//v3Sub(&player[pn].PlayerOldVel, &player[pn].Player->oldPos, &player[pn].Player->oldOldPos);
		hdb = player[pn].PlayerOnHoldPlayer;
		//tempStrat = hdb->strat;
		//p1 = &tempStrat->pos;
		//p2 = &tempStrat->oldPos;
		//v3Sub(&tempv, p1, p2);
		//v3Dec(&player[pn].PlayerOldVel, &tempv);
	}
	else 
	{
		Point3 p;
		float jolt;
		p = player[pn].PlayerOldVel;
		v3Sub(&player[pn].PlayerOldVel, &player[pn].Player->oldPos, &player[pn].Player->oldOldPos);

		jolt = pSqrDist (&p, &player[pn].PlayerOldVel);
		if (jolt > 0.015f) 
		{
			jolt = (jolt - 0.015f) * 2.5f;
			puruVibrate (pn, 1, 1, SQR(jolt+1)-1, RANGE(0.1,SQR(jolt),0.3));
		}

	}

	//if (player[pn].Player->flag & COLL_ON_FLOOR)
	if (atw > 2)
	{
		if ((player[pn].PlayerState == PS_SIDESTEP) && (!player[pn].playerSkidTurn))
		{
			lv = limitValue( fabs(strat->relVel.y), ((MAX_NORMAL_SPEED + (MAX_NORMAL_SPEED * player[pn].speedFactor)) / FRAMES_PER_SECOND) * 0.448f);

			if (joyState[pn].x < 0.0f)
			{
				if (player[pn].playerSkidDir)
					lv *= limitValue( -player[pn].Player->m.m[1][2], 0.7f);
				else
					lv *= limitValue( player[pn].Player->m.m[1][2], 0.7f);
			}
			else
			{
				if (!player[pn].playerSkidDir)
					lv *= limitValue( player[pn].Player->m.m[1][2], 0.7f);
				else
					lv *= limitValue( -player[pn].Player->m.m[1][2], 0.7f);
			}

			/*if (PadPress[pn] & PADLEFT)
				Yaw(strat, 0.05f);
			else if (PadPress[pn] & PADRIGHT)
				Yaw(strat, -0.05f);
				*/

			if (player[pn].DoPlayerFriction <= 1.0f - PFINC)
					player[pn].DoPlayerFriction += PFINC;

			rotA = joyState[pn].x;
			rotA *= rotA;
			rotA *= rotA;
			if (joyState[pn].x < 0.0f)
				rotA *= -1.0f;

			if (atw > 2)
			{
				if (player[pn].playerSkidDir)
					playerAccForward(pn, 4.0f * lv * rotA);
				else
					playerAccForward(pn, -4.0f * lv * rotA);
			}

			if (joyState[pn].x != 0)
				player[pn].playerSideStep = 1;
		}
		else if ((player[pn].PlayerState != PS_SIDESTEP) && (player[pn].PlayerState != PS_INDROPSHIP))
		{
			rotA = joyState[pn].x;
			rotA *= rotA;
			rotA *= rotA;

			if (joyState[pn].x < 0.0f)
				rotA *= 0.3f;
			else
				rotA *= -0.3f;
			
			if ((player[pn].playerBoostButtonCount == 2) || (player[pn].playerRevBoostButtonCount == 2))
			{	
				if (player[pn].PlayerLoop)
					Yaw(strat, rotA * 0.4f);
				else
					Yaw(strat, rotA * 0.28f);
			}
			else
			{
				if (player[pn].onBoat == 0)
					Yaw(strat, rotA * 0.2f * (1.0f + strat->Player->zooming) * 1.0);
				else
					Yaw(strat, rotA * 0.2f);
			}

			if ((player[pn].onBoat == 0) && (player[pn].PlayerState != PS_ONTRAIN))
				RotateObjectXYZ(player[pn].Player, 6, 0.0f, 0.0f, -rotA * 0.2f);
		}

		if (fabs(player[pn].PlayerAcc) < 0.0001f)
		{
			if (player[pn].PlayerState != PS_SIDESTEP)
			{
				if (v3Length(&strat->vel) > 0.05f)
					player[pn].PlayerBreaking = 1;
			}
		}

		if (player[pn].PlayerAcc > 0.0f)
		{
			if (strat->relVel.y < -0.1f)
			{
				player[pn].PlayerWheelSpin = 1;
			}

			if ((player[pn].playerBoostButtonCount == 2))
			{
				lv = limitValue( fabs(strat->relVel.y), ((MAX_BOOST_SPEED + (MAX_BOOST_SPEED * player[pn].speedFactor)) / FRAMES_PER_SECOND) * 0.448f);

				if (Multiplayer && 
					(GetCurrentGameType() == TAG || GetCurrentGameType() == SUICIDE_TAG))
					lv *= player[pn].Player->health * 0.01f;
				if (Multiplayer && (GetCurrentGameType() == TAG && player[pn].special))
					lv *= 1.1f;

				if ((GET_SURF_TYPE(player[pn].Player->polyType) == WALLOFDEATH) ||
					(GET_SURF_TYPE(player[pn].Player->polyType) == LOOPTHELOOP))
				{
					tempf = strat->m.m[1][2];
					if (tempf > 0.0f)
					{
						tempf *= tempf;
						tempf *= tempf;
						tempf *= strat->m.m[1][2];
						lv *= limitValue(tempf, PROCESS_ANG(MAX_ANG_BOOST));
					}
				}
				else
				{
					if (strat->m.m[1][2] > 0.0f)
						lv *= limitValue( 1.0 - strat->m.m[2][2], PROCESS_ANG(MAX_ANG_BOOST));
				}

				if ((GET_SURF_TYPE(player[pn].Player->polyType) == WALLOFDEATH) ||
					(GET_SURF_TYPE(player[pn].Player->polyType) == LOOPTHELOOP))
				{
					if (player[pn].DoPlayerFriction < 0.99)
						player[pn].DoPlayerFriction += 0.01f;
				}
				else
				{
					tempf = RANGE(0.0f, 0.4f - fabs(joyState[pn].x), 1.0f);

					if (player[pn].DoPlayerFriction > tempf)
						player[pn].DoPlayerFriction = tempf;
					else
						player[pn].DoPlayerFriction += 0.01f;
				}
				
				if (atw > 2)
					playerAccForward(pn, 4.0f * lv * player[pn].PlayerAcc);

			}
			else
			{
				lv = limitValue( fabs(strat->relVel.y), ((MAX_NORMAL_SPEED + (MAX_NORMAL_SPEED * player[pn].speedFactor)) / FRAMES_PER_SECOND) * 0.448f);

				if (GET_SURF_TYPE(player[pn].Player->polyType) == LOOPTHELOOP)
				{
					tempf = strat->m.m[1][2];
					if (tempf > 0.0f)
					{
						tempf *= tempf;
						tempf *= tempf;
						tempf *= strat->m.m[1][2];
						lv *= limitValue(tempf, PROCESS_ANG(MAX_ANG_NORMAL));
					}
				}
				else
				{
					if (strat->m.m[1][2] > 0.0f)
						lv *= limitValue( 1.0 - strat->m.m[2][2], PROCESS_ANG(MAX_ANG_NORMAL));
				}
					/*if (player[pn].Player->m.m[1][2] > 0.0f)
						lv *= limitValue( 1.0 - player[pn].Player->m.m[2][2], PROCESS_ANG(MAX_ANG_NORMAL));*/

				if (player[pn].DoPlayerFriction <= 1.0f - PFINC)
					player[pn].DoPlayerFriction += PFINC;
				
				if (atw > 2)
					playerAccForward(pn, 4.0f * lv * player[pn].PlayerAcc);
			}
		}
		else if (player[pn].PlayerAcc < 0.0f)
		{
			if (strat->relVel.y > 0.1f)
			{
				player[pn].PlayerWheelSpin = 1;
			}
			if (player[pn].playerRevBoostButtonCount == 2)
			{
				lv = limitValue( fabs(strat->relVel.y), ((MAX_BOOST_SPEED + (MAX_BOOST_SPEED * player[pn].speedFactor)) / FRAMES_PER_SECOND) * 0.448f);
				
				if (Multiplayer && 
					(GetCurrentGameType() == TAG || GetCurrentGameType() == SUICIDE_TAG))
					lv *= player[pn].Player->health * 0.01f;
				if (Multiplayer && (GetCurrentGameType() == TAG && player[pn].special))
					lv *= 1.1f;

				if ((GET_SURF_TYPE(player[pn].Player->polyType) == WALLOFDEATH) ||
					(GET_SURF_TYPE(player[pn].Player->polyType) == LOOPTHELOOP))
				{
					tempf = player[pn].Player->m.m[1][2];
					if (tempf < 0.0f)
					{
						tempf *= tempf;
						tempf *= tempf;
						lv *= limitValue(tempf, PROCESS_ANG(MAX_ANG_BOOST));
					}
				}
				else
				{
					if (player[pn].Player->m.m[1][2] < 0.0f)
						lv *= limitValue( 1.0 - player[pn].Player->m.m[2][2], PROCESS_ANG(MAX_ANG_BOOST));
				}

				if ((GET_SURF_TYPE(player[pn].Player->polyType) == WALLOFDEATH) ||
					(GET_SURF_TYPE(player[pn].Player->polyType) == LOOPTHELOOP))
				{
					if (player[pn].DoPlayerFriction < 0.99)
						player[pn].DoPlayerFriction += 0.01f;
				}
				else
				{
					tempf = RANGE(0.0f, 0.4f - fabs(joyState[pn].x), 1.0f);

					if (player[pn].DoPlayerFriction > tempf)
						player[pn].DoPlayerFriction = tempf;
					else
						player[pn].DoPlayerFriction += 0.01f;
				}

				if (atw > 2)
					playerAccForward(pn, 4.0f * lv * player[pn].PlayerAcc);

			}
			else
			{
				lv = limitValue( fabs(strat->relVel.y), ((MAX_NORMAL_SPEED + (MAX_NORMAL_SPEED * player[pn].speedFactor)) / FRAMES_PER_SECOND) * 0.448f);

				if (GET_SURF_TYPE(player[pn].Player->polyType) == LOOPTHELOOP)
				{
					tempf = strat->m.m[1][2];
					if (tempf < 0.0f)
					{
						tempf *= tempf;
						tempf *= tempf;
						tempf *= strat->m.m[1][2];
						lv *= limitValue(tempf, PROCESS_ANG(MAX_ANG_NORMAL));
					}
				}
				else
				{
					if (strat->m.m[1][2] < 0.0f)
						lv *= limitValue( 1.0 - strat->m.m[2][2], PROCESS_ANG(MAX_ANG_NORMAL));
				}


				if (player[pn].DoPlayerFriction <= 1.0f - PFINC)
					player[pn].DoPlayerFriction += PFINC;

				if (atw > 2)
					playerAccForward(pn, 4.0f * lv * player[pn].PlayerAcc);
			}
		}
		else
		{
			if (player[pn].DoPlayerFriction <= 1.0f - PFINC)
				player[pn].DoPlayerFriction += PFINC;
		}
	}
	else
	{
		player[pn].DoPlayerFriction = 0.0f;
		rotA = joyState[pn].x;
		rotA *= rotA;
		rotA *= rotA;
		if (!(player[pn].PlayerState & PS_SIDESTEP))
		{

			if (PAL)
			{
				if (joyState[pn].x > 0.0f)
					strat->turn.z -= rotA * 0.01 * 0.825f * SCALEFAC;
				else
					strat->turn.z += rotA * 0.01 * 0.825f * SCALEFAC;
			}
			else
			{

			 	if (joyState[pn].x > 0.0f)
					strat->turn.z -= rotA * 0.01;
				else
					strat->turn.z += rotA * 0.01;

		   
			}
		}
	}

	/*if ((player[pn].PlayerEnergy < 1.0f) && (player[pn].ShieldHold == 0))
		player[pn].PlayerEnergy += ENERGY_RECHARGE_RATE;*/
	
	

	if (!player[pn].PlayerHeld)
		mPreRotateZ(&strat->m, strat->turn.z);
	

	if (player[pn].PlayerState == PS_SIDESTEP)
	{
		tempv.x = strat->m.m[1][0];
		tempv.y = strat->m.m[1][1];
		tempv.z = strat->m.m[1][2];

		tempp.x = strat->m.m[1][0];
		tempp.y = strat->m.m[1][1];
		tempp.z = 0.0f;

		v3Normalise(&tempp);

		tempf = v3Dot(&tempv, &tempp);
		tempf = RANGE(-1.0f, tempf, 1.0f);
		if (strat->m.m[1][2] < 0.0f)
			tempf = -acos(tempf);
		else
			tempf = acos(tempf);

		if (!player[pn].playerSkidDir)
			tempf = -tempf;

		player[pn].rollAng += (tempf - player[pn].rollAng) * 0.1f;

	}
	else
	{
		tempv.x = strat->m.m[0][0];
		tempv.y = strat->m.m[0][1];
		tempv.z = strat->m.m[0][2];

		tempp.x = strat->m.m[0][0];
		tempp.y = strat->m.m[0][1];
		tempp.z = 0.0f;

		v3Normalise(&tempp);

		tempf = v3Dot(&tempv, &tempp);
		tempf = RANGE(-1.0f, tempf, 1.0f);
		if (strat->m.m[0][2] < 0.0f)
			tempf = -acos(tempf);
		else
			tempf = acos(tempf);

		player[pn].rollAng += (tempf - player[pn].rollAng) * 0.1f;
	}



	if (player[pn].rollAng != player[pn].rollAng)
		dAssert(0, "hello\n");
	tempp.x = tempp.y = 0.0f;
	tempp.z = 10.0f;
	v3Scale(&tempv, &player[pn].HitDir, 1.0f);
	//ApplyForce(player[pn].Player, &tempv, &tempp, player[pn].Player->pnode->collForceRatio);
	player[pn].HitDir.x = player[pn].HitDir.y = player[pn].HitDir.z = 0.0f;
//	if (player[pn].RedDogHudStrat)
//		player[pn].RedDogHudStrat->pos = player[pn].Player->pos;

	if (strat == BoatStrat)
	{
		tempv = BoatStrat->vel;
		lv = limitValue( 1.0 - strat->m.m[2][2], PROCESS_ANG(20));
		v3Scale(&tempv, &tempv, lv);
		tempp.x = 0.0f;
		tempp.y = 0.0f;
		tempp.z = -10.0f;
		ApplyForce(BoatStrat, &tempv, &tempp, player[pn].Player->pnode->collForceRatio * 0.05f);
	}

	if (player[pn].PlayerState == PS_INDROPSHIP)
	{
		if (player[pn].Player)
		{
			player[pn].Player->vel.x = 0.0f;
			player[pn].Player->vel.y = 0.0f;
		}
	}

	if ((player[pn].Player->vel.x * player[pn].Player->vel.x + player[pn].Player->vel.y * player[pn].Player->vel.y) < 0.0005f)
	{
		player[pn].Player->vel.x = 0.0f;
		player[pn].Player->vel.y = 0.0f;
	}
		
}


static void sortOutTheGunWarez(int pn)
{
	if (Multiplayer) {
		player[pn].currentMainGun = 5;
	} else {
		
		switch (player[pn].PlayerWeaponType)
		{
		case 0:
			player[pn].currentMainGun = 24;
			HideObject(player[pn].Player, 5);
			HideObject(player[pn].Player, 23);
			HideObject(player[pn].Player, 25);
			HideObject(player[pn].Player, 26);

			HideObject(player[pn].Player, 27);
			HideObject(player[pn].Player, 28);
			HideObject(player[pn].Player, 29);
			HideObject(player[pn].Player, 30);
			HideObject(player[pn].Player, 31);
			HideObject(player[pn].Player, 32);

			UnhideObject(player[pn].Player, 24);
			UnhideObject(player[pn].Player, 33);
			break;
		case 1:
			player[pn].currentMainGun = 25;
			HideObject(player[pn].Player, 23);
			HideObject(player[pn].Player, 24);
			HideObject(player[pn].Player, 26);
			HideObject(player[pn].Player, 5);

			HideObject(player[pn].Player, 27);
			HideObject(player[pn].Player, 28);
			HideObject(player[pn].Player, 31);
			HideObject(player[pn].Player, 32);
			HideObject(player[pn].Player, 33);

			UnhideObject(player[pn].Player, 29);
			UnhideObject(player[pn].Player, 30);
			UnhideObject(player[pn].Player, 25);
			break;
		case 2:
			player[pn].currentMainGun = 23;
			HideObject(player[pn].Player, 24);
			HideObject(player[pn].Player, 25);
			HideObject(player[pn].Player, 26);
			HideObject(player[pn].Player, 5);

			HideObject(player[pn].Player, 27);
			HideObject(player[pn].Player, 28);
			HideObject(player[pn].Player, 29);
			HideObject(player[pn].Player, 30);
			HideObject(player[pn].Player, 33);

			UnhideObject(player[pn].Player, 31);
			UnhideObject(player[pn].Player, 32);
			UnhideObject(player[pn].Player, 23);
			break;
		case 3:
			player[pn].currentMainGun = 26;
			HideObject(player[pn].Player, 24);
			HideObject(player[pn].Player, 25);
			HideObject(player[pn].Player, 23);
			HideObject(player[pn].Player, 5);

			HideObject(player[pn].Player, 29);
			HideObject(player[pn].Player, 30);
			HideObject(player[pn].Player, 31);
			HideObject(player[pn].Player, 32);
			HideObject(player[pn].Player, 33);

			UnhideObject(player[pn].Player, 27);
			UnhideObject(player[pn].Player, 28);
			UnhideObject(player[pn].Player, 26);
			break;
		}
	}
}

#define NTSC_SIXTHSEC 5
#define PAL_SIXTHSEC 4
#define NTSC_FIVESIXTHSEC 25
#define PAL_FIVESIXTHSEC 20
#define NTSC_THIRDSEC 10
#define PAL_THIRDSEC 8
#define NTSC_ONESEC 30
#define PAL_ONESEC 25
#define NTSC_HALFSEC 15
#define PAL_HALFSEC 12

static void RedDogObjectMoveProcess(int pn)
{
	int		i;
	int		PLAYERFIREBUTHOLDTIME;
	int		FIVESIXTHSEC;
	int     THIRDSEC;
	int		ONESEC;
	int		HALFSEC;
	float	amount, rotA, tempf, frontWheelTurnAmount;
	float SCALEFAC;
	Object *obj;

   //	return;


	//player->turn.z already up by PAL_MOVESCALE IN REDDOGMOVEPROCESS
	//player->playeracc left untouched
	//player->relvel left untouched
	if (PAL)
	{
		PLAYERFIREBUTHOLDTIME = PAL_SIXTHSEC;
		FIVESIXTHSEC = PAL_FIVESIXTHSEC;
		THIRDSEC = PAL_THIRDSEC;
		ONESEC = PAL_ONESEC;
		HALFSEC = PAL_HALFSEC;
		SCALEFAC = PAL_MOVESCALE;
	  //	SCALEFAC = 1.0f;
	}
	else
	{
		PLAYERFIREBUTHOLDTIME = NTSC_SIXTHSEC;
		FIVESIXTHSEC = NTSC_FIVESIXTHSEC;
		THIRDSEC = NTSC_THIRDSEC;
		ONESEC = NTSC_ONESEC;
		HALFSEC = NTSC_HALFSEC;
		SCALEFAC = 1.0f;
	}





	
	if (player[pn].Player->health <= 0.0f)
		return;

	amount = (2.0f * 3.14159f) * (player[pn].Player->relVel.y * 0.22222f);
	if (PAL)
		amount *= PAL_MOVESCALE;

	if (player[pn].PlayerState != PS_SIDESTEP)
	{
		if (fabs(player[pn].PlayerAcc) < 0.0001f)
			amount = 0.0f;
		if (fabs(amount) < fabs(player[pn].PlayerAcc * 0.4f * SCALEFAC))
			amount = SCALEFAC * player[pn].PlayerAcc * 0.4f + (player[pn].PlayerAcc * player[pn].PlayerWheelSpin * 0.4f);
	}

	if ((player[pn].PlayerState != PS_SIDESTEP) && (!player[pn].PlayerHeld))
	{
		if (player[pn].PlayerAcc >= 0.0f)
			frontWheelTurnAmount = joyState[pn].x * joyState[pn].x * joyState[pn].x * 0.7f;
		else
			frontWheelTurnAmount = -joyState[pn].x * joyState[pn].x * joyState[pn].x * 0.7f;

	}
	else
		frontWheelTurnAmount = 0.0f;

  	if (PAL)
  		frontWheelTurnAmount *= PAL_MOVESCALE;

	//SCALEFAC = 1.0f;
	player[pn].playerBoostStopTime++;
	for (i=1; i<=player[pn].Player->pnode->noObjId; i++)
	{
		obj = player[pn].Player->objId[i];
		switch(i)
		{
			case 1:
				
				if ((player[pn].PlayerHeld) || (player[pn].onBoat))
				{
					amount = 0.0f;
					obj->crntPos.z = -0.6f;
				}
				else
					player[pn].Player->objId[i]->crntPos.z -= SCALEFAC * (player[pn].Player->objId[i]->crntPos.z - player[pn].Player->objId[i]->idlePos.z) * 0.1f;

				if (player[pn].onBoat == 1)
					break;

				if ((player[pn].PlayerState != PS_ONTRAIN) && (!player[pn].PlayerHeld))
					player[pn].Player->objId[i]->idleRot.x -= ((amount) - (player[pn].Player->turn.z * 5.0f));
				player[pn].Player->objId[i]->crntRot.x = player[pn].Player->objId[i]->idleRot.x;
				player[pn].Player->objId[i]->crntRot.z =  -frontWheelTurnAmount ;
				break;
			case 2:
				if ((player[pn].PlayerHeld) || (player[pn].onBoat))
				{
					amount = 0.0f;
					obj->crntPos.z = -0.6f;
				}
				else
					player[pn].Player->objId[i]->crntPos.z -= SCALEFAC * (player[pn].Player->objId[i]->crntPos.z - player[pn].Player->objId[i]->idlePos.z) * 0.1f;

				if (player[pn].onBoat == 1)
					break;
				if ((player[pn].PlayerState != PS_ONTRAIN) && (!player[pn].PlayerHeld))
					player[pn].Player->objId[i]->idleRot.x -= ((amount) + (player[pn].Player->turn.z * 5.0f));
				player[pn].Player->objId[i]->crntRot.x = player[pn].Player->objId[i]->idleRot.x;
				player[pn].Player->objId[i]->crntRot.z = -frontWheelTurnAmount;
				break;
			case 3:
				if ((player[pn].PlayerHeld) || (player[pn].onBoat))
				{
					amount = 0.0f;
					obj->crntPos.z = -0.6f;
				}
				else
					player[pn].Player->objId[i]->crntPos.z -= SCALEFAC * (player[pn].Player->objId[i]->crntPos.z - player[pn].Player->objId[i]->idlePos.z) * 0.1f;

				if (player[pn].onBoat == 1)
					break;

				if ((player[pn].PlayerState != PS_ONTRAIN) && (!player[pn].PlayerHeld))
					player[pn].Player->objId[i]->idleRot.x -= ((amount) + (player[pn].Player->turn.z * 5.0f));

				player[pn].Player->objId[i]->crntRot.x = player[pn].Player->objId[i]->idleRot.x;
				player[pn].Player->objId[i]->crntRot.z += SCALEFAC * (player[pn].Player->objId[i]->idleRot.z - player[pn].Player->objId[i]->crntRot.z) * 0.1f;
				break;
			case 4:
				if ((player[pn].PlayerHeld) || (player[pn].onBoat))
				{
					amount = 0.0f;	
					obj->crntPos.z = -0.6f;
				}
				else
					player[pn].Player->objId[i]->crntPos.z -= SCALEFAC * (player[pn].Player->objId[i]->crntPos.z - player[pn].Player->objId[i]->idlePos.z) * 0.1f;

				if (player[pn].onBoat == 1)
					break;

				if ((player[pn].PlayerState != PS_ONTRAIN) && (!player[pn].PlayerHeld))
					player[pn].Player->objId[i]->idleRot.x -= ((amount) - (player[pn].Player->turn.z * 5.0f));

				player[pn].Player->objId[i]->crntRot.x = player[pn].Player->objId[i]->idleRot.x;
				player[pn].Player->objId[i]->crntRot.z += SCALEFAC * (player[pn].Player->objId[i]->idleRot.z - player[pn].Player->objId[i]->crntRot.z) * 0.1f;
				break;
			case 5:
			case 23:
			case 24:
			case 25:
			case 26:
				crntToIdlePos(player[pn].Player->objId[i], 0.3f);
				if (player[pn].Player->objId[i]->idleRot.x > 1.2f)
					player[pn].Player->objId[i]->idleRot.x = 1.2f;
				if (player[pn].Player->objId[i]->idleRot.x < -0.4f)
					player[pn].Player->objId[i]->idleRot.x = -0.4f;
				rotA = (player[pn].Player->objId[i]->idleRot.x - player[pn].Player->objId[i]->crntRot.x) * 0.7f;

				if ((player[pn].Player->objId[i]->crntRot.x + rotA > -0.4f) &&
					(player[pn].Player->objId[i]->crntRot.x + rotA < 1.2f))
					player[pn].Player->objId[i]->crntRot.x += SCALEFAC * rotA;
				else if (((player[pn].Player->objId[i]->crntRot.x < -0.4f) &&
					 (rotA + player[pn].Player->objId[i]->crntRot.x > -0.4f)) ||
					 ((player[pn].Player->objId[i]->crntRot.x > 1.2f) &&
					 (rotA + player[pn].Player->objId[i]->crntRot.x < 1.2f)))
					 player[pn].Player->objId[i]->crntRot.x += SCALEFAC * rotA;
				if ((player[pn].PlayerState == PS_SIDESTEP) && (PadPress[pn] & PADX))
				{
					if (player[pn].playerSkidDir)
						player[pn].Player->objId[i]->crntRot.z += SCALEFAC * ((player[pn].Player->objId[i]->idleRot.z ) - player[pn].Player->objId[i]->crntRot.z) * 0.7f;
					else
						player[pn].Player->objId[i]->crntRot.z += SCALEFAC * ((player[pn].Player->objId[i]->idleRot.z ) - player[pn].Player->objId[i]->crntRot.z) * 0.7f;
				}
				else
					player[pn].Player->objId[i]->crntRot.z += SCALEFAC * (player[pn].Player->objId[i]->idleRot.z - player[pn].Player->objId[i]->crntRot.z) * 0.7f;

				player[pn].Player->objId[i]->crntPos.y += SCALEFAC * (player[pn].Player->objId[i]->idlePos.y - player[pn].Player->objId[i]->crntPos.y) * 0.5f;
				player[pn].Player->objId[i]->crntPos.z += SCALEFAC * (player[pn].Player->objId[i]->idlePos.z - player[pn].Player->objId[i]->crntPos.z) * 0.5f;
				break;
			case 6:
				if (player[pn].PlayerHeld)
				{
					player[pn].Player->objId[6]->crntRot.x = 0.0f;
					player[pn].Player->objId[6]->crntRot.z = 0.0f;
				}
				else
				{
					crntToIdlePos(player[pn].Player->objId[i], 0.3f);
					rotA = v3Length(&player[pn].Player->vel) * 0.3f;
					player[pn].Player->objId[6]->crntRot.z += SCALEFAC * (player[pn].Player->objId[6]->idleRot.z - player[pn].Player->objId[6]->crntRot.z) * rotA;
					player[pn].Player->objId[6]->crntRot.x += SCALEFAC * (player[pn].Player->objId[6]->idleRot.x - player[pn].Player->objId[6]->crntRot.x) * 0.3f;
					player[pn].Player->objId[6]->crntRot.z = RANGE(-0.8f, player[pn].Player->objId[6]->crntRot.z, 0.8f);
					tempf = player[pn].Player->objId[6]->crntRot.z;
					player[pn].com.x = (tempf * 0.1f);
					player[pn].com.y = (0.2f * tempf * tempf);
				}
				break;
			case 7:
				player[pn].Player->objId[7]->crntRot.y = (0.7 + player[pn].Player->objId[4]->crntPos.z);
				break;
			case 8:
				player[pn].Player->objId[8]->crntRot.y = (-0.7 - player[pn].Player->objId[3]->crntPos.z);
				break;
			case 9:
				crntToIdlePos(player[pn].Player->objId[i], 0.3f);
				rotA = (float)atan((player[pn].Player->objId[3]->crntPos.z - player[pn].Player->objId[4]->crntPos.z) * 0.37f);
				player[pn].Player->objId[9]->crntRot.y = -rotA;
				rotA = (player[pn].Player->objId[3]->crntPos.z + player[pn].Player->objId[4]->crntPos.z) * 0.5f;
				player[pn].Player->objId[9]->crntPos.z = rotA;
				break;
			case 10:
				crntToIdlePos(player[pn].Player->objId[i], 0.3f);
				rotA = (float)atan((player[pn].Player->objId[2]->crntPos.z - player[pn].Player->objId[1]->crntPos.z) * 0.37f);
				player[pn].Player->objId[10]->crntRot.y = -rotA;
				rotA = (player[pn].Player->objId[2]->crntPos.z + player[pn].Player->objId[1]->crntPos.z) * 0.5f;
				player[pn].Player->objId[10]->crntPos.z = rotA;
				break;
			case 14:
				if (Multiplayer)
					break;
				if (player[pn].playerFireButtonHold > PLAYERFIREBUTHOLDTIME)
				{
					player[pn].Player->objId[14]->crntRot.y = (RANGE(3, player[pn].playerFireButtonHold, 15) - 5) * 0.16f;
				}
				else if (player[pn].playerFireButtonHold <= PLAYERFIREBUTHOLDTIME)
				{
					player[pn].Player->objId[14]->crntRot.y *= SCALEFAC * 0.8f;
				}
				break;
			case 15:
				if (Multiplayer)
					break;
				if (player[pn].playerFireButtonHold > PLAYERFIREBUTHOLDTIME)
				{
					player[pn].Player->objId[15]->crntRot.y = -(RANGE(3, player[pn].playerFireButtonHold, 15) - 5) * 0.22f;
				}
				else if (player[pn].playerFireButtonHold <= PLAYERFIREBUTHOLDTIME)
				{
					player[pn].Player->objId[15]->crntRot.y *= SCALEFAC * 0.8f;
				}
				break;
			case 16:
				if (Multiplayer)
					break;
				if (player[pn].playerFireButtonHold > PLAYERFIREBUTHOLDTIME)
				{
					player[pn].Player->objId[16]->crntRot.y = -(RANGE(3, player[pn].playerFireButtonHold, 15) - 5) * 0.16f;
				}
				else if (player[pn].playerFireButtonHold <= PLAYERFIREBUTHOLDTIME)
				{
					player[pn].Player->objId[16]->crntRot.y *= SCALEFAC * 0.8f;
				}
				break;
			case 17:
				if (player[pn].playerFireButtonHold > PLAYERFIREBUTHOLDTIME)
				{
					player[pn].Player->objId[17]->crntRot.y = (RANGE(3, player[pn].playerFireButtonHold, 15) - 5) * 0.22f;
				}
				else if (player[pn].playerFireButtonHold <= PLAYERFIREBUTHOLDTIME)
				{
					player[pn].Player->objId[17]->crntRot.y *= SCALEFAC * 0.8f;
				}
				break;
			case 20:
				if (player[pn].onBoat == 1)
				{
					HideObject(player[pn].Player, i);
					break;
				}
				if (player[pn].playerBoostButtonCount == 2)
				{
					player[pn].playerBoostStopTime = 0;
					if (player[pn].playerBoostButtonHold < FIVESIXTHSEC)
					{
						UnhideObject(player[pn].Player, i);
						player[pn].Player->objId[i]->scale.x = SCALEFAC * OUTERBOOSTSCALE * playerOuterBoostStartScale[player[pn].playerBoostButtonHold * 2];
						player[pn].Player->objId[i]->scale.z = SCALEFAC * OUTERBOOSTSCALE * playerOuterBoostStartScale[player[pn].playerBoostButtonHold * 2];
						player[pn].Player->objId[i]->scale.y = SCALEFAC * OUTERBOOSTSCALE * playerOuterBoostStartScale[player[pn].playerBoostButtonHold * 2 + 1];
					}		
					else
						HideObject(player[pn].Player, i);
				}
				else if (player[pn].playerRevBoostButtonCount == 2)
				{
					player[pn].playerBoostStopTime = 0;
					if (player[pn].playerRevBoostButtonHold < FIVESIXTHSEC)
					{
						UnhideObject(player[pn].Player, i);
						player[pn].Player->objId[i]->scale.x = SCALEFAC * OUTERBOOSTSCALE * playerOuterBoostStartScale[player[pn].playerRevBoostButtonHold * 2];
						player[pn].Player->objId[i]->scale.z = SCALEFAC * OUTERBOOSTSCALE * playerOuterBoostStartScale[player[pn].playerRevBoostButtonHold * 2];
						player[pn].Player->objId[i]->scale.y = SCALEFAC * OUTERBOOSTSCALE * playerOuterBoostStartScale[player[pn].playerRevBoostButtonHold * 2 + 1];
					}		
					else
						HideObject(player[pn].Player, i);
				}
				else if ((player[pn].playerBoostStopTime < THIRDSEC) && (player[pn].playerBoostStopTime > 0))
				{
					UnhideObject(player[pn].Player, i);
					player[pn].Player->objId[i]->scale.x = SCALEFAC * OUTERBOOSTSCALE * playerOuterBoostEndScale[(player[pn].playerBoostStopTime - 1) * 2];
					player[pn].Player->objId[i]->scale.z = SCALEFAC * OUTERBOOSTSCALE * playerOuterBoostEndScale[(player[pn].playerBoostStopTime - 1) * 2];
					player[pn].Player->objId[i]->scale.y = SCALEFAC * OUTERBOOSTSCALE * playerOuterBoostEndScale[(player[pn].playerBoostStopTime - 1) * 2 + 1];
				}
				else
					HideObject(player[pn].Player, i);
				break;
			case 21:
			case 22:
				if (player[pn].onBoat == 1)
				{
					HideObject(player[pn].Player, i);
					break;
				}
				if (player[pn].playerBoostButtonCount == 2)
				{
					player[pn].playerBoostStopTime = 0;
					if (player[pn].playerBoostButtonHold < ONESEC)
					{
						UnhideObject(player[pn].Player, i);
						player[pn].Player->objId[i]->scale.x = SCALEFAC * INNERBOOSTSCALE * playerInnerBoostStartScale[player[pn].playerBoostButtonHold * 2];
						player[pn].Player->objId[i]->scale.z = SCALEFAC * INNERBOOSTSCALE * playerInnerBoostStartScale[player[pn].playerBoostButtonHold * 2];
						player[pn].Player->objId[i]->scale.y = SCALEFAC * INNERBOOSTSCALE * playerInnerBoostStartScale[player[pn].playerBoostButtonHold * 2 + 1];
					}		
					else 
					{
						UnhideObject(player[pn].Player, i);
						player[pn].Player->objId[i]->scale.x = SCALEFAC * INNERBOOSTSCALE * playerInnerBoostMiddleScale[((player[pn].playerBoostButtonHold - 30) % 7) * 2];
						player[pn].Player->objId[i]->scale.z = SCALEFAC * INNERBOOSTSCALE * playerInnerBoostMiddleScale[((player[pn].playerBoostButtonHold - 30) % 7) * 2];
						player[pn].Player->objId[i]->scale.y = SCALEFAC * INNERBOOSTSCALE * playerInnerBoostMiddleScale[((player[pn].playerBoostButtonHold - 30) % 7) * 2 + 1];
					}		
				}
				else if ((player[pn].playerBoostStopTime < HALFSEC) && (player[pn].playerBoostStopTime > 0))
				{
					UnhideObject(player[pn].Player, i);
					player[pn].Player->objId[i]->scale.x = SCALEFAC * INNERBOOSTSCALE * playerInnerBoostEndScale[(player[pn].playerBoostStopTime - 1) * 2];
					player[pn].Player->objId[i]->scale.z = SCALEFAC * INNERBOOSTSCALE * playerInnerBoostEndScale[(player[pn].playerBoostStopTime - 1) * 2];
					player[pn].Player->objId[i]->scale.y = SCALEFAC * INNERBOOSTSCALE * playerInnerBoostEndScale[(player[pn].playerBoostStopTime - 1) * 2 + 1];
				}
				else
					HideObject(player[pn].Player, i);
				break;
		}
	}
	if ((!player[pn].PlayerHeld) && 
		(player[pn].playerBoostButtonCount == 2 || player[pn].playerRevBoostButtonCount == 2)) {
		// Start sound
		if (!IsPlaying(player[pn].Player, 2)) 
		{
			PlaySound(player[pn].Player, 2, GENERIC_BOOST_SOUND, 
				Multiplayer ? 0.55f : 0.85f, 0, -2, 0, SBFLAG_CONTINUOUS|SBFLAG_FILTERENV|SBFLAG_NODOPPLER);
			SetPitch(player[pn].Player, 2, -0.1);
		} 
	} 
	else 
	{
		// Stop sound
		if (IsPlaying(player[pn].Player, 2)) 
			StopSound(player[pn].Player, 2, 0.f);
	}

	if (!Multiplayer) 
	{
		if (player[pn].playerFireButtonHold == (PLAYERFIREBUTHOLDTIME+5))
		{
			PlaySound(player[pn].Player, 10, GENERIC_WEAPON_OPEN, 0.75f, 0.f, 0.f, 0.f, SBFLAG_STOPPING|SBFLAG_NODOPPLER);
		}
		else if (!player[pn].playerFireButtonHold) 
		{
			StratSoundHeader *header = player[pn].Player->SoundBlock;

			if (header && header->block[10].flags & SBFLAG_STOPPING)
			{
				PlaySound(player[pn].Player, 10, GENERIC_WEAPON_CLOSE, 0.75f, 0.f, 0.f, 0.f, SBFLAG_NODOPPLER);
			}
		}
		sortOutTheGunWarez(pn);
	}
}


static void RedDogTiltProcess(int pn)
{
	Vector3	tempv;
	Point3	pos;
	
	if (PadPress[pn] & PADRIGHT)
		return;

	if (player[pn].PlayerOnHoldPlayer)
		return; 

	v3Sub(&tempv, &player[pn].Player->vel, &player[pn].PlayerOldVel);
	tempv.z = 0.0f;
	pos.x = 0.0f;
	pos.y = 0.0f;
	pos.z = -50.0f;
	ApplyForce(player[pn].Player, &tempv, &pos, 0.4f);
}

static void RedDogFrictionProcess(int pn)
{
	int atw=0, i;
	float SCALEFAC;
   	STRAT *strat;


  	if (PAL)
  		SCALEFAC = PAL_MOVESCALE;
  	else
  		SCALEFAC = 1.0f;
	//player->turn.z already up by PAL_MOVESCALE
   
	if (player[pn].onBoat == 1)
		strat = BoatStrat;
	else
		strat = player[pn].Player;

	if (strat == BoatStrat)
	{
		if (strat->objId[1]->collFlag & COLL_ON_WATER)
			atw = 4;
	}
	else
	{
		for (i=1; i<5; i++)
			if (player[pn].Player->objId[i]->collFlag & COLL_ON_FLOOR)
				atw++;
	}


	if (fabs(strat->relVel.y) < 0.001)
		strat->relVel.y = 0.0f;

	if (((player[pn].PlayerAcc == 0.0f) && (player[pn].playerSlide == 0) && (player[pn].playerSideStep == 0)) || (player[pn].PlayerAcc * strat->relVel.y < -0.02f))
	{
		if (strat->friction.x >= 0.02)
			strat->friction.x -= 0.02f;
		else
			strat->friction.x = 0.0f;
		if (strat->friction.y >= 0.02f)
			strat->friction.y -= 0.02f;
		else 
			strat->friction.y = 0.0f;
	}
	else// if (!player[pn].PlayerHeld)
	{
		strat->friction = player[pn].PlayerFriction;
		if (strat != BoatStrat)
		{
			player[pn].Player->objId[1]->crntRot.z += player[pn].Player->turn.z;
			player[pn].Player->objId[2]->crntRot.z += player[pn].Player->turn.z;
		}
	}
	
	//if ((player[pn].Player->flag & COLL_ON_FLOOR) && (!player[pn].playerSlide))
	if ((atw > 2) && (!player[pn].playerSlide))
	{
		if (PAL)
			strat->vel.z *= (0.928f);
		else
			strat->vel.z *= (0.95f);
					
		if ((GET_SURF_TYPE(player[pn].Player->polyType) != WALLOFDEATH) || (player[pn].playerBoostButtonCount != 2))
		{
			if (GET_SURF_TYPE(player[pn].Player->polyType) == LOOPTHELOOP)
			{
				if ((player[pn].Player->m.m[2][2] < 0.72f))
					player[pn].DoPlayerFriction = 0.0f;
			}
			else
			{
				if (player[pn].PlayerOnHoldPlayer)
				{
					if ((player[pn].Player->m.m[2][2] < 0.7f))
						player[pn].DoPlayerFriction = 0.0f;
				}
				else
				{
					if ((player[pn].Player->m.m[2][2] < 1.0f - PROCESS_ANG(MAX_ANG_NORMAL * SCALEFAC)))// && (Player->m.m[1][2] > 0.0f))
						player[pn].DoPlayerFriction = 0.0f;
				}
			}
		}
	}
	else
		player[pn].DoPlayerFriction = 0.0f;
}



void RedDogProcessNormal(int pn)
{
	if (player[pn].Player->health <= 0.0f)
		return;

	if (player[pn].Player->obj == NULL)
		return;

	playerControl(pn);
	playerBotControl(pn);
	debugPlayerControl(pn);
	RedDogFrictionProcess(pn);
	RedDogMoveProcess(pn);	
	RedDogObjectMoveProcess(pn);	
	polyTypeCheck(pn);

	if ((player[pn].PlayerState == PS_NORMAL) && (player[pn].PlayerHeld == 0))
		RedDogTiltProcess(pn);

  	if (PAL)
  		player[pn].Player->turn.z *= (1.0f - (0.1f * PAL_MOVESCALE));
  	else
		player[pn].Player->turn.z *= (1.0f - 0.1f);
	player[pn].Player->relAcc.y = 0.0f;
	if (BoatStrat)
		BoatStrat->turn.z *= 0.9f;
	
}

void RedDogProcessNoControl(int pn)
{
	player[pn].playerBoostButtonCount = 0;
	player[pn].playerRevBoostButtonCount = 0;
	player[pn].PlayerAcc = 0.0f;
  //	RedDogFrictionProcess(pn);
	//ENSURE THE SHIELD IS HIDDEN AND SPEED IS KILLED
	//KillMyVelocity(player[pn].Player);
	HideObject (player[pn].Player, 19);
//	RedDogMoveProcess(pn);	
	RedDogFrictionProcess(pn);
	RedDogMoveProcess(pn);	
	RedDogObjectMoveProcess(pn);	
	polyTypeCheck(pn);
 //	RedDogTiltProcess(pn);
	player[pn].Player->turn.z *= 0.99f;
	if (BoatStrat)
		BoatStrat->turn.z *= 0.99f;
	player[pn].Player->relAcc.y = 0.0f;
	player[pn].Player->rot_speed *= 0.9f;
}

#if 0
void RedDogProcessEntry(int pn)
{
	//Gravity = 0.000545f;
	RedDogFrictionProcess(pn);
	RedDogMoveProcess(pn);	
	polyTypeCheck(pn);

//	RedDogTiltProcess(pn);
	Gravity = 0.0109f;

}
#endif



void RedDogProcessWaterSlide(int pn)
{
	debugPlayerControl(pn);
	playerControl(pn);
	playerBotControl(pn);
	RedDogObjectMoveProcess(pn);
	player[0].PlayerOldVel.x = player[0].Player->pos.x - player[0].Player->oldPos.x;
	player[0].PlayerOldVel.y = player[0].Player->pos.y - player[0].Player->oldPos.y;
	player[0].PlayerOldVel.z = player[0].Player->pos.z - player[0].Player->oldPos.z;
	/*if (player[pn].PlayerEnergy < 1.0)
		player[pn].PlayerEnergy += ENERGY_RECHARGE_RATE;	*/
}


void RedDogProcessSideStep(int pn)
{
	if (player[pn].Player->obj == NULL)
		return;

	playerControl(pn);
	playerBotControl(pn);
	player[pn].playerBoostButtonCount = 0;
	player[pn].playerRevBoostButtonCount = 0;
	debugPlayerControl(pn);
	RedDogFrictionProcess(pn);
	RedDogMoveProcess(pn);	
	RedDogObjectMoveProcess(pn);	
	polyTypeCheck(pn);
	RedDogTiltProcess(pn);
	player[pn].Player->turn.z *= 0.9f;
	if (BoatStrat)
		BoatStrat->turn.z *= 0.9f;
	player[pn].Player->relAcc.y = 0.0f;
}


 

/* 
void RedDogProcessOnTrain(int pn)
{
	if (player[pn].Player->obj == NULL)
		return;

	playerControl(pn);
	playerBotControl(pn);
	debugPlayerControl(pn);
	RedDogFrictionProcess(pn);
	RedDogMoveProcess(pn);	
	RedDogObjectMoveProcess(pn);	
	polyTypeCheck(pn);
	player[pn].Player->turn.z = 0.0f;
	player[pn].Player->relAcc.y = 0.0f;
}
*/

void RedDogProcessTowerLift(int pn)
{

	if (player[pn].Player->obj == NULL)
		return;

	playerControl(pn);
	debugPlayerControl(pn);
	RedDogFrictionProcess(pn);
	v3Sub(&player[pn].PlayerOldVel, &player[pn].Player->pos, &player[pn].Player->oldPos);
	RedDogObjectMoveProcess(pn);	
	polyTypeCheck(pn);
	player[pn].Player->turn.z = 0.0f;
	player[pn].Player->relAcc.y = 0.0f;
}

#if 0
void LWaterSlide(STRAT *strat, float amount)
{
 
	int SegDiff;
	return;
	//SegDiff = BossStrat->route->splineData->lineSeg - WaterSlide->route->splineData->lineSeg; 

	//IF SEPARATE CAM CHAP MOVE HIM BACK
    /*if (BossStrat != WaterSlideCamLookStrat)
	{
		
		MoveAlongPath(WaterSlideCamLookStrat, -(amount + 10003.0f));
	 
		MoveAlongPath(WaterSlideCamLookStrat, 1.0f);
		MoveAlongPath(BossStrat, -(amount + 10003.0f));
	 
		MoveAlongPath(BossStrat, 1.0f);
	}
	else
	{
		MoveAlongPath(BossStrat, -(amount + 10003.0f));
	 
		MoveAlongPath(BossStrat, 1.0f);


	}*/
 
 //	MoveAlongPath(WaterSlideCamLookStrat, -(amount + 10003.0f));
	 
 //	MoveAlongPath(WaterSlideCamLookStrat, 1.0f);
	if (!BossStrat)
	{
		MoveAlongPath(WaterSlide, - (amount + 10003.0f));
		MoveAlongPath(WaterSlide, 1.0f);
	}
}
#endif