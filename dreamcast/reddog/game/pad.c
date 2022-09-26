#include "reddog.h"
#include "input.h"
#include "view.h"
#include "command.h"
#include "GameState.h"
#include "process.h"
#include "collspec.h"
#include "strtcoll.h"
#include "stratPhysic.h"
#include "camera.h"
#include "player.h"
#include "VMSStruct.h"
#include "Render\Sparks.h"


#define SHIELD_SCALE_SPEED_X	0.2f
#define SHIELD_SCALE_SPEED_Y	0.7f
#define SHIELD_SCALE_SPEED_Z	0.7f
#define SHIELD_SCALE_SPEED_X_D	0.8f
#define SHIELD_SCALE_SPEED_Y_D	0.9f
#define SHIELD_SCALE_SPEED_Z_D	0.9f
#define TEXT 1

extern float smoothSideStep(int pn);
extern	int obscured(Point3 *p1, Point3 *p2, Point3 *p, int pn, STRAT *strat);

extern	PLAYER player[4];
extern	float	ambientL;
extern	Uint8	hLightState;
extern	Bool	debounce;

//extern	void setOCPT(STRAT *crntStrat, Object *obj);
static float ShieldPerturb = 0.f, ShieldSpeed = 1.f;

void PlayerControlClean(int pn)
{

	PlayerControlJoyX = 0.0f;
	PlayerControlJoyY = 0.0f;
	PlayerControlPlayerAcc = 0.0f;
	PlayerControlPadPress = 0;
	PlayerControlPadPush = 0;

	player[pn].playerBoostButtonHold = player[pn].playerBoostButtonRelease = player[pn].playerBoostButtonCount = 0;
	player[pn].playerRevBoostButtonHold = player[pn].playerRevBoostButtonRelease = player[pn].playerRevBoostButtonCount = 0;
	player[pn].playerFireButtonHold = 0;
	player[pn].fireSecondary = player[pn].targetting = 0;
	globalVar = 0.0f;
   	/*FloorStrat = NULL;*/ 
	player[pn].CameraStrat = player[pn].Player = NULL;
	ShieldPerturb = 0.f;
	ShieldSpeed = 1.f;
}

//int	oldPlayerObj;

void debugPlayerControl(int pn)
{
	float rotA;
	Vector3	dir;

	#if (!GINSU && !GODMODE)

		if (PadPush[pn] & PADUP)	
		{
			player[pn].cameraPos = THIRD_PERSON_CLOSE;
			PlayerCameraState[pn] = player[pn].cameraPos;
		}	
		else if (PadPush[pn] & PADDOWN)
		{
			player[pn].cameraPos = FIRST_PERSON;
			PlayerCameraState[pn] = player[pn].cameraPos;
		}
	
	

	/*if(PadPush[pn] & PADY)
	{
		player[pn].PlayerSecondaryWeapon.type = (player[pn].PlayerSecondaryWeapon.type + 1) % 6;
		player[pn].PlayerSecondaryWeapon.amount = 1000;
	}*/

/*	if ((player[pn].Player) && (player[pn].Player->objId[11]))
	{
		if(PadPress[pn] & PADY)
			player[pn].Player->objId[11]->collFlag &= ~OBJECT_INVISIBLE;
		else
			player[pn].Player->objId[11]->collFlag |= OBJECT_INVISIBLE;
	}
*/

		if (player[pn].PlayerState != PS_SIDESTEP)
		{
			//if (!Multiplayer)
			{
			
				#ifndef _RELEASE
					if (PadPush[pn] & PADDOWN)
					{
						extern int suppressDebug;
						suppressDebug = !suppressDebug;
					}
				#endif
			}

			if (PadPush[pn] & PADUP)
			{
				player[pn].cameraPos = THIRD_PERSON_CLOSE;
				PlayerCameraState[pn] = player[pn].cameraPos;
			}
			else if (PadPush[pn] & PADDOWN)
			{
				player[pn].cameraPos = FIRST_PERSON;
				PlayerCameraState[pn] = player[pn].cameraPos;
			}

		}

		if (!Multiplayer)
		{
			if (PadPress[1])
			{
				if (PadPress[1] & PAD_CAMERA0)
					ChangeCam(0,0);

				if (PadPress[1] & PAD_CAMERA1)
					ChangeCam(1,0);

				if (PadPress[1] & PAD_CAMERA2)
					ChangeCam(2,0);

				if (PadPress[1] & PAD_CAMERA3)
					ChangeCam(3,0);

				if (PadPress[1] & PAD_CAMERA4)
					ChangeCam(4,0);

				if (PadPress[1] & PAD_CAMERA5)
					ChangeCam(5,0);

				if (PadPress[1] & PAD_CAMERA6)
					ChangeCam(6,0);

				if (PadPress[1] & PAD_CAMERA7)
					ChangeCam(7,0);

				if (PadPress[1] & PAD_CAMERA8)
					ChangeCam(8,0);

				if (PadPress[1] & PAD_CAMERA9)
					ChangeCam(9,0);

			}
		}
	#else
		if (PadPush[pn] & PADUP)
		{
			player[pn].cameraPos = THIRD_PERSON_CLOSE;
			PlayerCameraState[pn] = player[pn].cameraPos;
		}
		else if (PadPush[pn] & PADDOWN)
		{
			player[pn].cameraPos = FIRST_PERSON;
			PlayerCameraState[pn] = player[pn].cameraPos;
		}

	
	#endif
}

static void MakeSparks(void)
{
	int i, j, l, m, max, nSparks;
	Point3 *vertex;

	if (Multiplayer)
		return; // No thanks
	
	// Create some spark warez
	// Sparks are created from the midpoint of two random points
	// They then progress to the midpoint of two other random points
	max = player[currentPlayer].Player->objId[12]->model->nVerts;
	vertex = player[currentPlayer].Player->objId[12]->model->vertex;
	nSparks = RandRange(1,3);
	do {
		MakeShieldSpark ();
	} while (--nSparks);
}

#if 0
// Shield craziness
int AccelerateTowardsShield (STRAT *strat, int vertex)
{
	Point3 p, *vert;

	vert = &player[currentPlayer].Player->objId[12]->model->vertex[vertex];
	mPoint3Multiply3 (&p, vert, &player[currentPlayer].Player->objId[12]->wm);
	
	strat->pos.x = (strat->pos.x + p.x) * 0.5f;
	strat->pos.y = (strat->pos.y + p.y) * 0.5f;
	strat->pos.z = (strat->pos.z + p.z) * 0.5f;

	return (pSqrDist (&strat->pos, &p) < SQR(1.f));
}
#endif

int GetShieldVertexNumber (int pn)
{
	return (int) RandRange(0, player[pn].Player->objId[12]->model->nVerts);
}

void shieldControl(int pn)
{	
	ShieldSparkMatrix = player[pn].Player->objId[12]->wm;
	ShieldPerturb -= (1.f / 12.f);
	player[pn].Player->objId[19]->scale = player[pn].Player->objId[12]->scale;
	if(PadPress[pn] & PADX) 
	{
		if ((player[pn].PlayerShieldEnergy > 0.0f) && (player[pn].shieldValid == 1))
		{
			if (!(player[0].reward & CHEAT_INFINITE_SHIELD))
				player[pn].PlayerShieldEnergy -= 0.003f;

			MakeSparks();

			if (player[pn].ShieldHold == 0) {
				PlaySound (player[pn].Player, 3, GENERIC_SHIELD_SOUND, 1, 0, 0, 0, SBFLAG_CONTINUOUS|SBFLAG_NODOPPLER);
				UnhideObject(player[pn].Player, 12);
				UnhideObject(player[pn].Player, 18);
				UnhideObject (player[pn].Player, 19);
				player[pn].Player->objId[12]->scale.x = 0.1f;
				player[pn].Player->objId[12]->scale.y = 0.1f;
				player[pn].Player->objId[12]->scale.z = 0.1f;
				player[pn].Player->objId[12]->transparency = 0.f;
				player[pn].Player->objId[18]->transparency = 0.f;
				player[pn].Player->objId[19]->transparency = 0.6f;
			}
			player[pn].ShieldHold ++;
			if (player[pn].Player->objId[12]->scale.x < 0.99f &&
				player[pn].Player->objId[12]->scale.y < 0.99f &&
				player[pn].Player->objId[12]->scale.z < 0.99f) {
				player[pn].Player->objId[12]->scale.x += (1.f - player[pn].Player->objId[12]->scale.x) * SHIELD_SCALE_SPEED_X;
				player[pn].Player->objId[12]->scale.y += (1.f - player[pn].Player->objId[12]->scale.y) * SHIELD_SCALE_SPEED_Y;
				player[pn].Player->objId[12]->scale.z += (1.f - player[pn].Player->objId[12]->scale.z) * SHIELD_SCALE_SPEED_Z;
				player[pn].Player->objId[12]->transparency += 0.1f;
				player[pn].Player->objId[18]->transparency = 0.6f * Delta(SQR(player[pn].Player->objId[12]->transparency));
				if ((rand() & 3))
					player[pn].Player->objId[12]->transparency = 1.0f;
				else
					player[pn].Player->objId[12]->transparency = 0.4f;
			} else {
				SinCosVal val;
				float clamp;
				SinCos ((currentFrame & 0xfffff) * ShieldSpeed * ((2 * PI) / 16.f), &val);
				clamp = MAX(0, ShieldPerturb);
				val.sin *= 0.08f * clamp;
				val.cos *= 0.07f * clamp;
				player[pn].Player->objId[12]->scale.x = 1.f + val.sin;
				player[pn].Player->objId[12]->scale.z = 1.f - val.cos;
				player[pn].Player->objId[18]->scale.x = 1.f + val.sin;
				player[pn].Player->objId[18]->scale.z = 1.f - val.cos;
				player[pn].Player->objId[12]->transparency = 1.0f;
				player[pn].Player->objId[18]->transparency = 0.6f;
				if ((rand() & 31) > 2)
					player[pn].Player->objId[12]->transparency = 1.0f;
				else {
					player[pn].Player->objId[12]->transparency = 0.4f;
					if ((ShieldPerturb <= 0) && (rand() & 1)) {
						ShieldPerturb = RandRange(0.5, 0.95);
						ShieldSpeed = RandRange (0.8, 1.2);
					}
				}
				if (player[pn].ShieldHold >= 20) {
					if (player[pn].ShieldHold > 40)
						HideObject (player[pn].Player, 19);
					else {
						if ((rand() & 3))
							UnhideObject (player[pn].Player, 19);
						else
							HideObject (player[pn].Player, 19);

						player[pn].Player->objId[19]->transparency -= (1.f / 20.f);
					}
				}
			}
		}
		else
		{
			if (player[pn].PlayerShieldEnergy < 1.0f)
				player[pn].PlayerShieldEnergy += 0.001f;

			player[pn].shieldValid = 0;
		}
			
	}
	else
	{
		//if (Multiplayer)
		{
			if (player[pn].PlayerShieldEnergy < 1.0f)
				player[pn].PlayerShieldEnergy += 0.001f;
		}

		if (player[pn].PlayerShieldEnergy > SHIELD_CUTOFF)
			player[pn].shieldValid = 1;
		else
			player[pn].shieldValid = 0;
		
	}


	if ((player[pn].shieldValid == 0) || !(PadPress[pn] & PADX))
	{
			
		if (IsPlaying(player[pn].Player, 3)) 
			StopSound(player[pn].Player, 3, 0.f);
		player[pn].ShieldHold = 0;
		if (player[pn].Player->objId[12]->scale.x > 0.1f ||
			player[pn].Player->objId[12]->scale.y > 0.1f ||
			player[pn].Player->objId[12]->scale.z > 0.1f)
		{
			MakeSparks();		
			player[pn].Player->objId[12]->scale.x *= SHIELD_SCALE_SPEED_X_D;
			player[pn].Player->objId[12]->scale.y *= SHIELD_SCALE_SPEED_Y_D;
			player[pn].Player->objId[12]->scale.z *= SHIELD_SCALE_SPEED_Z_D;
			player[pn].Player->objId[12]->transparency -= 0.14f;
			player[pn].Player->objId[18]->transparency -= 0.2f;
			player[pn].Player->objId[19]->transparency -= (1.f / 20.f);
		} else {
			HideObject(player[pn].Player, 12);
			HideObject(player[pn].Player, 18);
			HideObject(player[pn].Player, 19);
		}
	}
}

static void boostControl(int pn)
{
	if (PadPress[pn] & PADFORWARD)
	{
		if (player[pn].playerBoostButtonHold == 0)
			player[pn].playerBoostButtonCount++;
		else if ((player[pn].playerBoostButtonHold > 10) && (player[pn].playerBoostButtonCount != 2))
			player[pn].playerBoostButtonCount = 0;

		player[pn].playerBoostButtonHold++;
		player[pn].playerBoostButtonRelease = 0;
		

		if (((player[pn].PlayerLoop == 1) || (player[pn].playerBoostButtonCount != 2)) && (fabs(joyState[pn].x) > 0.8f))
		{
			player[pn].PlayerLoop = 1;
		}
		else
		{
			player[pn].PlayerLoop = 0;
		}

		// Are we ready to boost, or are we in a multiplayer game and have ALWAYS BOOST on
		if (Multiplayer && (player[pn].zooming == 0.0) && !(CurProfile[LogToPhys(pn)].controller & CTRL_NOTDBL))
				player[pn].playerBoostButtonCount = 2;
	}
	else
	{
		if ((player[pn].playerBoostButtonRelease > 10) || (player[pn].playerBoostButtonCount == 2))
			player[pn].playerBoostButtonCount = 0;

		player[pn].playerBoostButtonHold = 0;
		player[pn].playerBoostButtonRelease++;
	}

	if (PadPress[pn] & PADBACKWARD)
	{
		if (player[pn].playerRevBoostButtonHold == 0)
			player[pn].playerRevBoostButtonCount++;
		else if ((player[pn].playerRevBoostButtonHold > 10) && (player[pn].playerRevBoostButtonCount != 2))
			player[pn].playerRevBoostButtonCount = 0;

		player[pn].playerRevBoostButtonHold++;
		player[pn].playerRevBoostButtonRelease = 0;

		if (((player[pn].PlayerLoop == 1) || (player[pn].playerRevBoostButtonCount != 2)) && (fabs(joyState[pn].x) > 0.8f))
		{
			player[pn].PlayerLoop = 1;
		}
		else
		{
			player[pn].PlayerLoop = 0;
		}

		if (Multiplayer && (player[pn].zooming == 0.0))
				player[pn].playerRevBoostButtonCount = 2;
	}
	else
	{
		if ((player[pn].playerRevBoostButtonRelease > 10) || (player[pn].playerRevBoostButtonCount == 2))
			player[pn].playerRevBoostButtonCount = 0;

		player[pn].playerRevBoostButtonHold = 0;
		player[pn].playerRevBoostButtonRelease++;

		
	}

	if (!Multiplayer)
	{
		if (((player[0].playerBoostButtonCount == 2) || (player[0].playerRevBoostButtonCount == 2)) && 
			((player[pn].playerBoostButtonHold > 180) || (player[pn].playerRevBoostButtonHold > 180)))
		{
			if (!(player[0].reward & CHEAT_PERMANENT_BOOST)) {
				if (player[0].engineHeat <= 0.995f)
					player[0].engineHeat += 0.005f;
				else
					player[0].engineHeat = 1.0f;
			}
		}
		else if ((player[0].playerBoostButtonCount != 2) && (player[0].playerRevBoostButtonCount != 2))
		{
			if (player[0].engineHeat >= 0.005f)
				player[0].engineHeat -= 0.01f;
			else
				player[0].engineHeat = 0.0f;
		}
	}
}

static int inTargetArray(STRAT *strat, Object *obj, int pn)
{
	int i;

	for (i=0; i<MAX_TARGETS; i++)
	{
		if ((player[pn].target[i].strat == strat) && (player[pn].target[i].obj == obj))
			return 1;
	}
	return 0;
}

static int inPlayerCone(int n, int pn, float ang)
{
	Point3	p1, p2;
	Vector3	v1, v2;

	p1 = player[pn].CameraStrat->pos;
	p2 = player[n].Player->pos;
	v3Sub(&v1, &p2, &p1);
	v3Normalise(&v1);
	v2.x = player[pn].CameraStrat->m.m[1][0];
	v2.y = player[pn].CameraStrat->m.m[1][1];
	v2.z = player[pn].CameraStrat->m.m[1][2];
	v3Normalise(&v2);
	if (acos(v3Dot(&v1, &v2)) < ang)
		return 1;
	else
		return 0;
}

#define ENERGY_REDUCE 0.008f
#define ADRENALIN_INCREASE 0.04f
#define ADRENALIN_DECREASE 0.03333f
#define ADRELANIN_FADE 0.001f

void fireControl(int pn)
{
	Point3	tempp;
	int i;

	if (!Multiplayer)
	{
		
		if (!PauseMode)
		{
			if ((player[pn].PlayerWeaponEnergy >= 0.03 || (player[pn].reward & CHEAT_INF_CHARGE)) && (PadPress[pn] & PADY))
			{
				if ((player[pn].PlayerWeaponCharge <= 1.97)) 
				{
					player[pn].PlayerWeaponCharge += 0.03f;
					
					SetPitch (player[pn].Player, 13, (player[pn].PlayerWeaponCharge/2.f));
					SetPitch (player[pn].Player, 14, (player[pn].PlayerWeaponCharge/4.f));

					if (!IsPlaying(player[pn].Player, 13)) 
						PlaySound(player[pn].Player, 13, 45, 0.75f, 0.f, 0.f, 0.f, SBFLAG_NOREVERB|SBFLAG_NODOPPLER);

					if (!IsPlaying(player[pn].Player, 14)) 
						PlaySound(player[pn].Player, 14, 46, 0.75f, 0.f, 0.f, 0.f, SBFLAG_NOREVERB|SBFLAG_NODOPPLER);
				} else {
					if (IsPlaying(player[pn].Player, 13))
						StopSound(player[pn].Player, 13, 0.f);
					if (IsPlaying(player[pn].Player, 14))
						StopSound(player[pn].Player, 14, 0.f);
				}

				if (!(player[pn].reward & CHEAT_INF_CHARGE))
					player[pn].PlayerWeaponEnergy -= 0.03f;
			}
			else
			{
				if (player[pn].PlayerWeaponCharge >= ADRELANIN_FADE)
					player[pn].PlayerWeaponCharge -= ADRELANIN_FADE;
				else
				{
					player[pn].PlayerWeaponCharge = 0.0f;
					player[pn].PlayerWeaponPower = 0;
				}

				if (IsPlaying(player[pn].Player, 13))
					StopSound(player[pn].Player, 13, 0.f);
				if (IsPlaying(player[pn].Player, 14))
					StopSound(player[pn].Player, 14, 0.f);
			}
		}
		
		if (player[pn].PlayerWeaponCharge >= 1.0)
			player[pn].PlayerWeaponPower = 2;
		else if(player[pn].PlayerWeaponCharge > 0.0)
			player[pn].PlayerWeaponPower = 1;
	}

	if (PadPress[pn] & PADFIRE)
	{
		player[pn].playerFireButtonHold++;
					
		if (!player[pn].playerFireButtonHold)
		{
			player[pn].aimType = 1;
		}

	
		if (player[pn].playerFireButtonHold > 12)
		{
			player[pn].targetting = 1;
			player[pn].aimType = 4;
		}
		else if(player[pn].playerFireButtonHold > 8)
			player[pn].aimType = 3;
		else if (player[pn].playerFireButtonHold > 4)
			player[pn].aimType = 2;
	}
	else
	{
		player[pn].playerFireButtonHold = 0;
		
		player[pn].aimType = 0;
		if (player[pn].targetting == 1)
			player[pn].fireSecondary = 1;
		if (player[pn].targetting == 1)
		{
			player[pn].currentTarget = 0;
			player[pn].targetting = 0;
		}
	}
	
	

	if (PadPress[pn] & PADB)
	{
		player[pn].playerBButtonHold++;

		switch(player[pn].PlayerSecondaryWeapon.type)
		{
			case 0:
				player[pn].PlayerArmageddon = 0;
				player[pn].PlayerPuffTheMagicDragon = 0;
				break;
			case ROCKET:
				if (Multiplayer)
				{
					player[pn].targetting = 3;
					if (player[pn].playerTargetNumber >= 0)
					{
						if (player[player[pn].playerTargetNumber].Player)
						{
							//if (!inTargetArray(player[player[pn].playerTargetNumber].Player, NULL, pn))
								//addToTargetArray(player[player[pn].playerTargetNumber].Player, NULL, pn);

							if (inPlayerCone(player[pn].playerTargetNumber, pn, 0.4f) && 
								!obscured(&player[pn].CameraStrat->pos, &player[player[pn].playerTargetNumber].Player->pos, &tempp, pn, NULL))
							{
								player[pn].lostTargetTime = 0;
								player[pn].playerTargetTime++;
								if (player[pn].playerTargetTime > LOCKON_WAIT)
									player[pn].fireSecondary = 2;
									
							}
							else
							{
								player[pn].lostTargetTime++;
								if (player[pn].lostTargetTime > 15)
								{
									RemoveTarget(player[player[pn].playerTargetNumber].Player, pn, -1);
									if (player[pn].targetting == 3)
									{
										player[pn].targetting = 0;
										player[pn].fireSecondary = 0;
										player[pn].playerTargetTime = 0;
										player[pn].playerTargetNumber = -1;
									}
								}
							}
						}
					}
					else
					{
						if (player[pn].targetting == 3)
						{
							player[pn].lostTargetTime++;
						}
						if ((player[pn].lostTargetTime > 15) || (player[pn].playerTargetNumber == -1))
						{
							player[pn].playerTargetTime = 0;
							player[pn].playerTargetNumber = -1;	
							for (i=0; i<NumberOfPlayers; i++)
							{
								if ((i == pn) || player[i].Player == NULL || (player[i].Player->var == 1234))
									continue;

								if ((player[i].Player->flag & COLL_TARGETABLE) && 
									inPlayerCone(i, pn, 0.4f)  && 
									!obscured(&player[pn].CameraStrat->pos, &player[i].Player->pos, &tempp, pn, NULL))
								{
									player[pn].lostTargetTime = 0;
									player[pn].playerTargetNumber = i;
									if (!inTargetArray(player[player[pn].playerTargetNumber].Player, NULL, pn))
									{
										addToTargetArray(player[player[pn].playerTargetNumber].Player, NULL, pn);
										break;
									}
								}
							}
						}
					}
				}
				else
				{
				
				}
				break;
			case BOMB:
			case VULCAN:
			case HOMING_BULLET:
				break;
		}
	}
	else
	{
		player[pn].playerBButtonHold = 0;
		
		switch(player[pn].PlayerSecondaryWeapon.type)
		{
			case VULCAN:
				player[pn].PlayerPuffTheMagicDragon = 0;
				break;
			case ROCKET:
				if (Multiplayer)
				{
					if (player[pn].targetting == 3)
					{
						player[pn].targetting = 0;
						player[pn].playerTargetTime = 0;
						player[pn].playerTargetNumber = -1;
						ClearPlayerTargetArray(pn);
					}
				}
				break;
		}
	}
}

static void sideStepPlayerControl(int pn)
{
	STRAT *strat;

	if (player[pn].PlayerState == PS_ONPATH)
		return;

	if (player[pn].onBoat == 1)
		strat = BoatStrat;
	else
		strat = player[pn].Player;

	player[pn].playerSkidTurn = 0;
	if ((PadPress[pn] & PADFORWARD) && (PadPress[pn] & PADBACKWARD) && (player[pn].zooming == 0.0f) && (player[pn].PlayerState != PS_ONPATH))
	{
		if (!player[pn].playerSideStepHold)
		{
			if (joyState[pn].x > 0.0f)
				player[pn].playerSkidDir = 1;
			else
				player[pn].playerSkidDir = 0;
		}
		player[pn].PlayerState = PS_SIDESTEP;
		
		
		if (player[pn].playerSideStepHold < SKID_FRAMES)
		{
			if (player[pn].playerSkidDir)
			{
				if (strat != BoatStrat)
				{
					player[pn].Player->objId[1]->idleRot.x -= 0.2f;
					player[pn].Player->objId[2]->idleRot.x += 0.2f;
					player[pn].Player->objId[3]->idleRot.x += 0.2f;
					player[pn].Player->objId[4]->idleRot.x -= 0.2f;
				}
				mPreRotateZ(&strat->m, -smoothSideStep(pn));
				player[pn].Player->objId[5]->idleRot.z += smoothSideStep(pn);
				player[pn].Player->objId[5]->crntRot.z += smoothSideStep(pn);
				
			}
			else
			{
				if (strat != BoatStrat)
				{
					player[pn].Player->objId[1]->idleRot.x += 0.2f;
					player[pn].Player->objId[2]->idleRot.x -= 0.2f;
					player[pn].Player->objId[3]->idleRot.x -= 0.2f;
					player[pn].Player->objId[4]->idleRot.x += 0.2f;
				}
				mPreRotateZ(&strat->m, smoothSideStep(pn));
				player[pn].Player->objId[5]->idleRot.z -= smoothSideStep(pn);
				player[pn].Player->objId[5]->crntRot.z -= smoothSideStep(pn);

				//mPreRotateZ(&Player->m, 0.10472f);
			}
			player[pn].playerSkidTurn = 1;
		}

		if (player[pn].playerSideStepHold < SKID_FRAMES)
			player[pn].playerSideStepHold++;

		if (player[pn].playerSideStepHold < 15)
			player[pn].playerSlide = 1;
	}
	else
	{
		if (player[pn].playerSideStepHold)
		{
			player[pn].playerSideStepHold--;
			if (player[pn].playerSkidDir)
			{
				if (strat != BoatStrat)
				{
					player[pn].Player->objId[1]->idleRot.x += 0.2f;
					player[pn].Player->objId[2]->idleRot.x -= 0.2f;
					player[pn].Player->objId[3]->idleRot.x -= 0.2f;
					player[pn].Player->objId[4]->idleRot.x += 0.2f;
				}
				mPreRotateZ(&strat->m, smoothSideStep(pn));
				player[pn].Player->objId[5]->idleRot.z -= smoothSideStep(pn);
				player[pn].Player->objId[5]->crntRot.z -= smoothSideStep(pn);
			}
			else
			{
				if (strat != BoatStrat)
				{
					player[pn].Player->objId[1]->idleRot.x -= 0.2f;
					player[pn].Player->objId[2]->idleRot.x += 0.2f;
					player[pn].Player->objId[3]->idleRot.x += 0.2f;
					player[pn].Player->objId[4]->idleRot.x -= 0.2f;
				}
				mPreRotateZ(&strat->m, -smoothSideStep(pn));
				player[pn].Player->objId[5]->idleRot.z += smoothSideStep(pn);
				player[pn].Player->objId[5]->crntRot.z += smoothSideStep(pn);
			}
			player[pn].playerSkidTurn = 1;
		
			
		}
		else
		{
			if (player[pn].PlayerState == PS_SIDESTEP)
				player[pn].PlayerState = PS_NORMAL;
		}
	}
}

void playerControl(int pn)
{
	float rotA;
	player[pn].playerSlide = 0;
	
	shieldControl(pn);
	fireControl(pn);
	if (!player[pn].PlayerHeld)
	{
		boostControl(pn);
		sideStepPlayerControl(pn);
	}

	if (bad_cheat == 8)
	{
		if ((PadPress[pn] & PADUP))
		{
			
			PlayerLives = 2;
			player[pn].PlayerWeaponEnergy = 2;
			player[pn].PlayerWeaponType++;
			player[pn].PlayerWeaponType = player[pn].PlayerWeaponType % 4;
			player[pn].Player->vel.x *= 0.9f;
			player[pn].Player->vel.y *= 0.9f;
			player[pn].Player->vel.z = 0.0f;
			player[pn].Player->health = 100.0f;
			if (player[pn].onBoat == 1)
			{
				BoatStrat->pos.z += 0.5;
				BoatStrat->vel.z = 0.0f;
			}
			else
				player[pn].Player->pos.z += 0.3f;
		}


		if (PadPress[pn] & PADRIGHT)
		{
			if (player[pn].onBoat == 1)
			{
				BoatStrat->pos.x += 4.0f * player[pn].Player->m.m[1][0];
				BoatStrat->pos.y += 4.0f * player[pn].Player->m.m[1][1];
				BoatStrat->pos.z += 4.0f * player[pn].Player->m.m[1][2];
			}
			else
			{
				player[pn].Player->pos.x += 4.0f * player[pn].Player->m.m[1][0];
				player[pn].Player->pos.y += 4.0f * player[pn].Player->m.m[1][1];
				player[pn].Player->pos.z += 4.0f * player[pn].Player->m.m[1][2];
			}
		}

		if ((PadPress[pn] & PADUP) || (PadPress[pn] & PADRIGHT))
		{
			if (HoldPlayerMain)
			{
				
			}
			else
			{
				Flatten(player[pn].Player, 3.0f);
				player[pn].Player->rot_speed *= 0.8f;
				rotA = joyState[pn].x * joyState[pn].x * joyState[pn].x;
				Yaw(player[pn].Player, -rotA * 0.1f);
			}
		

		}

		if (PadPush[0] & PADDOWN)
		{
			player[0].PlayerSecondaryWeapon.amount = 10000;
			if (!Multiplayer)
			{
				switch(player[0].PlayerSecondaryWeapon.type)
				{
					case NONE:
						player[0].PlayerSecondaryWeapon.type = BOMB;
						break;
					case BOMB:
						player[0].PlayerSecondaryWeapon.type = VULCAN;
						break;
					case VULCAN:
						player[0].PlayerSecondaryWeapon.type = HOMING_BULLET;
						break;
					case HOMING_BULLET:
						player[0].PlayerSecondaryWeapon.type = ALL_ROUND_SHOCKWAVE;
						break;
					case ALL_ROUND_SHOCKWAVE:
						player[0].PlayerSecondaryWeapon.type = ROCKET;
						break;
					case ROCKET:
						player[0].PlayerSecondaryWeapon.type = SWARM;
						break;
					case SWARM:
						player[0].PlayerSecondaryWeapon.type = LASER;
						break;
					case LASER:
						player[0].PlayerSecondaryWeapon.type = ELECTRO;
						break;
					case ELECTRO:
						player[0].PlayerSecondaryWeapon.type = HOVERON;
						break;
					case HOVERON:
						player[0].PlayerSecondaryWeapon.type = BOMB;
						break;
					default:
						player[0].PlayerSecondaryWeapon.type = NONE;
						break;
				}
			}
		}
	}
}

