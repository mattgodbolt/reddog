#include "RedDog.h"
#include "command.h"
#include "process.h"
#include "input.h"
/*#include "pad.h"*/
#include "view.h"
#include "camera.h"
#include "player.h"

extern	float Gravity;
#define NORTH		0
#define NORTH_EAST	1
#define EAST		2
#define SOUTH_EAST	3
#define SOUTH		4
#define SOUTH_WEST	5
#define WEST		6
#define NORTH_WEST	7


static void NonAnimProcess(STRAT *crntStrat)
{
	Vector3 vec2;
	dAssert(crntStrat->m.m[0][0] == crntStrat->m.m[0][0], "qnan");
	if (crntStrat == BoatStrat)
		return;
	if (crntStrat->turn.x != 0.0f)
		stratPitch(crntStrat, crntStrat->turn.x);
	if (crntStrat->turn.y != 0.0f)
		stratRoll(crntStrat, crntStrat->turn.y);
	if (crntStrat->turn.z != 0.0f)
		stratYaw(crntStrat, crntStrat->turn.z);
	mVector3Multiply (&vec2, &crntStrat->relAcc, &crntStrat->m);
	dAssert(crntStrat->m.m[0][0] == crntStrat->m.m[0][0], "qnan");

	crntStrat->vel.z += vec2.z; 
	crntStrat->vel.y += vec2.y; 
	crntStrat->vel.x += vec2.x; 
 
	crntStrat->relAcc.z = 0.0; 
	crntStrat->relAcc.y = 0.0; 
	crntStrat->relAcc.x = 0.0; 

}

static void NonAnimBounceProcess(STRAT *crntStrat)
{
	Vector3 vec2;

	if (crntStrat->turn.x != 0.0f)
		stratPitch(crntStrat, crntStrat->turn.x);
	if (crntStrat->turn.y != 0.0f)
		stratRoll(crntStrat, crntStrat->turn.y);
	if (crntStrat->turn.z != 0.0f)
		stratYaw(crntStrat, crntStrat->turn.z);
	mVector3Multiply (&vec2, &crntStrat->relAcc, &crntStrat->m);

	crntStrat->vel.z += vec2.z; 
	crntStrat->vel.y += vec2.y; 
	crntStrat->vel.x += vec2.x; 

	crntStrat->relAcc.z = 0.0; 
	crntStrat->relAcc.y = 0.0; 
	crntStrat->relAcc.x = 0.0; 

}


static void BattleTank1_2Process(STRAT *strat)
{
	Vector3 vec2;
   //THIS IS NOW DONE UNDER STRAT CONTROL
   //	strat->objId[5]->crntRot = strat->objId[5]->idleRot;
	strat->objId[1]->crntPos.z += (strat->objId[1]->idlePos.z - strat->objId[1]->crntPos.z) * 0.1f;
	strat->objId[2]->crntPos.z += (strat->objId[2]->idlePos.z - strat->objId[2]->crntPos.z) * 0.1f;
	strat->objId[3]->crntPos.z += (strat->objId[3]->idlePos.z - strat->objId[3]->crntPos.z) * 0.1f;
	strat->objId[4]->crntPos.z += (strat->objId[4]->idlePos.z - strat->objId[4]->crntPos.z) * 0.1f;


	mVector3Multiply(&vec2, &strat->relAcc, &strat->m);
	v3Inc(&strat->vel, &vec2);
	strat->relAcc.z = 0.0; 
	strat->relAcc.y = 0.0; 
	strat->relAcc.x = 0.0; 
}

static void MorterTankProcess(STRAT *strat)
{
	int i;
	Vector3 vec2;

	for (i=1; i<=6; i++)
	{
		if ((i==1) || (i==2) || (i==3))
			strat->objId[i]->crntRot.x -= (strat->relVel.y / 2.5f) - (strat->turnangz * 2.0f);
		else
			strat->objId[i]->crntRot.x -= (strat->relVel.y / 2.5f) + (strat->turnangz * 2.0f);
		strat->objId[i]->crntPos.z += (strat->objId[i]->idlePos.z - strat->objId[i]->crntPos.z) * 0.1f;
	}
	
	strat->objId[7]->crntPos.x += (strat->objId[7]->idlePos.x - strat->objId[7]->crntPos.x) * 0.1f;
	strat->objId[7]->crntPos.y += (strat->objId[7]->idlePos.y - strat->objId[7]->crntPos.y) * 0.1f;
	strat->objId[7]->crntPos.z += (strat->objId[7]->idlePos.z - strat->objId[7]->crntPos.z) * 0.1f;
	strat->turnangz = 0.0f;
	mVector3Multiply(&vec2, &strat->relAcc, &strat->m);
	v3Inc(&strat->vel, &vec2);
	strat->relAcc.z = 0.0; 
	strat->relAcc.y = 0.0; 
	strat->relAcc.x = 0.0; 
}

#define THIRTYFIVEDEG ((PI2/360.0f) * 35.0f)
/*
static void MobileHDHomingLauncherProcess(STRAT *crntStrat)
{
	Vector3 vec2;
	float	rotSpeed, rotAmount;
	int		i;

	if (crntStrat->pnode == NULL)
		return;

	if (crntStrat->turn.z != 0.0f){
		stratYaw(crntStrat, crntStrat->turn.z);
		crntStrat->turn.z=0;}
	mVector3Multiply(&vec2, &crntStrat->relAcc, &crntStrat->m);

	crntStrat->vel.z += vec2.z; 
	crntStrat->vel.y += vec2.y; 
	crntStrat->vel.x += vec2.x; 

	crntStrat->relAcc.z = 0.0; 
	crntStrat->relAcc.y = 0.0; 
	crntStrat->relAcc.x = 0.0; 

	if (crntStrat->relVel.y != 0.0f)
	{
		rotSpeed = -crntStrat->relVel.y / 2.5f;

		crntStrat->objId[1]->idleRot.x += rotSpeed;
		crntStrat->objId[3]->idleRot.x += rotSpeed;
		crntStrat->objId[5]->idleRot.x += rotSpeed;
		crntStrat->objId[7]->idleRot.x += rotSpeed;
		crntStrat->objId[9]->idleRot.x += rotSpeed;
		crntStrat->objId[11]->idleRot.x += rotSpeed;


		if (crntStrat->objId[1]->crntRot.z > (crntStrat->objId[1]->idleRot.z + THIRTYFIVEDEG))
		{
			crntStrat->objId[1]->crntRot.z = crntStrat->objId[1]->idleRot.z+THIRTYFIVEDEG ;
			crntStrat->objId[7]->crntRot.z = crntStrat->objId[7]->idleRot.z+THIRTYFIVEDEG ;
			crntStrat->objId[3]->crntRot.z = crntStrat->objId[3]->idleRot.z+(THIRTYFIVEDEG/2.0f) ;
			crntStrat->objId[9]->crntRot.z = crntStrat->objId[9]->idleRot.z+(THIRTYFIVEDEG/2.0f) ;
			crntStrat->objId[5]->crntRot.z = crntStrat->objId[5]->idleRot.z+(THIRTYFIVEDEG/4.0f) ;
			crntStrat->objId[11]->crntRot.z = crntStrat->objId[11]->idleRot.z+(THIRTYFIVEDEG/4.0f) ;

			
			
		}

		if (crntStrat->objId[1]->crntRot.z < (crntStrat->objId[1]->idleRot.z - THIRTYFIVEDEG))
		{
			crntStrat->objId[1]->crntRot.z = crntStrat->objId[1]->idleRot.z-THIRTYFIVEDEG ;
			crntStrat->objId[7]->crntRot.z = crntStrat->objId[7]->idleRot.z-THIRTYFIVEDEG ;
			crntStrat->objId[3]->crntRot.z = crntStrat->objId[3]->idleRot.z-(THIRTYFIVEDEG/2.0f) ;
			crntStrat->objId[9]->crntRot.z = crntStrat->objId[9]->idleRot.z-(THIRTYFIVEDEG/2.0f) ;
			crntStrat->objId[5]->crntRot.z = crntStrat->objId[5]->idleRot.z-(THIRTYFIVEDEG/4.0f) ;
			crntStrat->objId[11]->crntRot.z = crntStrat->objId[11]->idleRot.z-(THIRTYFIVEDEG/4.0f) ;
		}
		else
		{

			crntStrat->objId[1]->crntRot.z += crntStrat->turnangz ;
			crntStrat->objId[7]->crntRot.z += crntStrat->turnangz ;
			crntStrat->objId[3]->crntRot.z += crntStrat->turnangz/2.0f ;
			crntStrat->objId[9]->crntRot.z += crntStrat->turnangz/2.0f ;
			crntStrat->objId[5]->crntRot.z += crntStrat->turnangz/4.0f ;
			crntStrat->objId[11]->crntRot.z += crntStrat->turnangz/4.0f ;
		}
		
		crntStrat->objId[1]->crntRot.z *= 0.96f ;
		crntStrat->objId[7]->crntRot.z *= 0.96f;
		crntStrat->objId[3]->crntRot.z *= 0.96f;
		crntStrat->objId[9]->crntRot.z *= 0.96f;
		crntStrat->objId[5]->crntRot.z *= 0.96f;
		crntStrat->objId[11]->crntRot.z *= 0.96f;
		crntStrat->turnangz =0;
	}
	
	for (i=1; i<7; i++)
	{
		crntStrat->objId[i*2-1]->crntRot.x = crntStrat->objId[i*2-1]->idleRot.x;
		crntStrat->objId[i*2-1]->crntRot.y = crntStrat->objId[i*2-1]->idleRot.y;
		rotAmount = (crntStrat->objId[i*2]->idleRot.y - crntStrat->objId[i*2]->crntRot.y);
		crntStrat->objId[i*2]->crntRot.y += 0.05f * rotAmount;
	}

	crntStrat->objId[13]->crntRot.x += (crntStrat->objId[13]->idleRot.x - crntStrat->objId[13]->crntRot.x) * 0.5f;
	crntStrat->objId[13]->crntRot.z += (crntStrat->objId[13]->idleRot.z - crntStrat->objId[13]->crntRot.z) * 0.5f;
}
*/
static void waterProcess(STRAT *strat)
{
}

static void RapierProcess(STRAT *strat)
{
#if 0
	int	i, dir;
	float	Xang, Zang, zAngMul1, zAngMul2, zAngMul3, zAngMul4;
	float	xAngMul5, xAngMul6, xAngMul7, xAngMul8;
	float	localRotSpeedX	= 0.4f;
	float	localRotSpeedZ	= 0.8f;
	float	localRotSpeed	= 0.1f;

	strat->frame += 1.0f * localRotSpeed;
	dir = NORTH;

	switch (dir)
	{
		case NORTH:
			zAngMul1 = 1.0f;
			zAngMul2 = -1.0f;
			zAngMul3 = -1.0f;
			zAngMul4 = 1.0f;
			xAngMul5 = xAngMul6 = xAngMul7 = xAngMul8 = 0.0f;
			break;
		case NORTH_EAST:
			zAngMul1 = 1.0f;
			zAngMul2 = zAngMul4 = xAngMul5 = xAngMul7 = 0.0f;
			zAngMul3 = -1.0f;
			xAngMul6 = -1.0f;
			xAngMul8 = -1.0f;
			break;
		case EAST:
			zAngMul1 = 1.0f;
			zAngMul2 = 1.0f;
			zAngMul3 = -1.0f;
			zAngMul4 = -1.0f;
			xAngMul5 = xAngMul6 = xAngMul7 = xAngMul8 = 0.0f;
			break;
		case SOUTH_EAST:
			zAngMul2 = 1.0f;
			zAngMul1 = zAngMul3 = xAngMul6 = xAngMul8 = 0.0f;
			zAngMul4 = -1.0f;
			xAngMul5 = -1.0f;
			xAngMul7 = 1.0f;
			break;
		case SOUTH:
			zAngMul1 = -1.0f;
			zAngMul2 = 1.0f;
			zAngMul3 = 1.0f;
			zAngMul4 = -1.0f;
			xAngMul5 = xAngMul6 = xAngMul7 = xAngMul8 = 0.0f;
			break;
		case SOUTH_WEST:
			zAngMul1 = -1.0f;
			zAngMul2 = zAngMul4 = xAngMul5 = xAngMul7 = 0.0f;
			zAngMul3 = 1.0f;
			xAngMul6 = 1.0f;
			xAngMul8 = -1.0f;
			break;
		case WEST:
			zAngMul1 = -1.0f;
			zAngMul2 = -1.0f;
			zAngMul3 = 1.0f;
			zAngMul4 = 1.0f;
			xAngMul5 = xAngMul6 = xAngMul7 = xAngMul8 = 0.0f;
			break;
		case NORTH_WEST:
			zAngMul2 = -1.0f;
			zAngMul1 = zAngMul3 = xAngMul6 = xAngMul8 = 0.0f;
			zAngMul4 = 1.0f;
			xAngMul5 = 1.0f;
			xAngMul7 = 1.0f;
			break;
	}

	for (i=1; i<9; i++)
	{
		switch (i)
		{
			case 1:
				Xang = (float)sin(strat->frame) * localRotSpeedX;
				strat->objId[i]->idleRot.x = strat->pnode->objId[i]->initRot.x + Xang;
				if (zAngMul1)
				{
					Zang = (float)sin(strat->frame + HPI) * localRotSpeedZ;
					strat->objId[i]->idleRot.z = strat->pnode->objId[i]->initRot.z + Zang * zAngMul1;
				}
				break;
			case 2:
				Xang = (float)sin(strat->frame + PI) * localRotSpeedX;
				strat->objId[i]->idleRot.x = strat->pnode->objId[i]->initRot.x + Xang;
				if (zAngMul2)
				{
					Zang = (float)sin(strat->frame + HPI + PI) * localRotSpeedZ;
					strat->objId[i]->idleRot.z = strat->pnode->objId[i]->initRot.z + Zang * zAngMul2;
				}
				break;
			case 3:
				Xang = (float)sin(strat->frame) * localRotSpeedX;
				strat->objId[i]->idleRot.x = strat->pnode->objId[i]->initRot.x + Xang;
				if (zAngMul3)
				{
					Zang = (float)sin(strat->frame + HPI) * localRotSpeedZ;
					strat->objId[i]->idleRot.z = strat->pnode->objId[i]->initRot.z + Zang * zAngMul3;
				}
				break;
			case 4:
				Xang = (float)sin(strat->frame + PI) * localRotSpeedX;
				strat->objId[i]->idleRot.x = strat->pnode->objId[i]->initRot.x + Xang;
				if (zAngMul4)
				{
					Zang = (float)sin(strat->frame + PI + HPI) * localRotSpeedZ;
					strat->objId[i]->idleRot.z = strat->pnode->objId[i]->initRot.z + Zang * zAngMul4;
				}
				break;
			case 5:
				if (xAngMul5)
				{
					Xang = (float)sin(strat->frame + HPI + PI) * localRotSpeedX;
					strat->objId[i]->idleRot.x = strat->pnode->objId[i]->initRot.x + Xang * xAngMul5;
				}
				break;
			case 6:
				if (xAngMul6)
				{
					Xang = (float)sin(strat->frame + HPI + PI) * localRotSpeedX;
					strat->objId[i]->idleRot.x = strat->pnode->objId[i]->initRot.x + Xang * xAngMul6;
				}
				break;
			case 7:
				if (xAngMul7)
				{
					Xang = (float)sin(strat->frame + HPI + PI) * localRotSpeedX;
					strat->objId[i]->idleRot.x = strat->pnode->objId[i]->initRot.x + Xang * xAngMul7;
				}
				break;
			case 8:
				if (xAngMul8)
				{
					Xang = (float)sin(strat->frame + HPI + PI) * localRotSpeedX;
					strat->objId[i]->idleRot.x = strat->pnode->objId[i]->initRot.x + Xang * xAngMul8;
   			}
				break;
		}
	}

	for (i=1; i<9; i++)
	{
		strat->objId[i]->crntRot.x += (strat->objId[i]->idleRot.x - strat->objId[i]->crntRot.x) * 0.2f;
		strat->objId[i]->crntRot.z += (strat->objId[i]->idleRot.z - strat->objId[i]->crntRot.z) * 0.2f;
	}
#endif
}

#define TENDEG ((PI2/360.0) * 20.0)

static void A00Walker01Process(STRAT *strat)
{
#if 0
	float mass =  strat->pnode->mass * 0.05f;
	float TEST;
	NonAnimProcess(strat);
	if ((strat->flag2 & STRAT2_RECOIL))
		return;
		
		TEST = (float) fabs(strat->relVel.y);
	if (!TEST)
	 {
		strat->objId[3]->idleRot = strat->pnode->objId[3]->initRot;
		strat->objId[4]->idleRot = strat->pnode->objId[4]->initRot;
		strat->objId[5]->idleRot = strat->pnode->objId[5]->initRot;
		strat->objId[6]->idleRot = strat->pnode->objId[6]->initRot;
	 	 TEST = 0.5f;

	 }	

	ApplyRelForce(strat,0,strat->relAcc.x * mass,strat->relAcc.y * mass,strat->relAcc.z * mass,0,0,0);
 

	
   	strat->objId[1]->crntPos.y += (strat->objId[1]->idlePos.y - strat->objId[1]->crntPos.y) * 0.05f;
   	strat->objId[2]->crntPos.y += (strat->objId[2]->idlePos.y - strat->objId[2]->crntPos.y) * 0.05f;
   	strat->objId[1]->crntPos.x += (strat->objId[1]->idlePos.x - strat->objId[1]->crntPos.x) * 0.05f;
   	strat->objId[2]->crntPos.x += (strat->objId[2]->idlePos.x - strat->objId[2]->crntPos.x) * 0.05f;
   	strat->objId[1]->crntPos.z += (strat->objId[1]->idlePos.z - strat->objId[1]->crntPos.z) * 0.05f;
   	strat->objId[2]->crntPos.z += (strat->objId[2]->idlePos.z - strat->objId[2]->crntPos.z) * 0.05f;

		
	strat->objId[1]->crntRot.x += (strat->objId[1]->idleRot.x - strat->objId[1]->crntRot.x) *0.6f;
	strat->objId[2]->crntRot.x += (strat->objId[2]->idleRot.x - strat->objId[2]->crntRot.x) *0.6f;

	strat->objId[3]->crntRot.x += (strat->objId[3]->idleRot.x - strat->objId[3]->crntRot.x) * TEST *0.6f;
 	strat->objId[4]->crntRot.x += (strat->objId[4]->idleRot.x - strat->objId[4]->crntRot.x) * TEST *0.6f;
  	strat->objId[5]->crntRot.x += (strat->objId[5]->idleRot.x - strat->objId[5]->crntRot.x) * TEST *0.6f;
  	strat->objId[6]->crntRot.x += (strat->objId[6]->idleRot.x - strat->objId[6]->crntRot.x) * TEST * 0.6f;

  /*	strat->objId[1]->crntRot.x *= 0.86f;
  	strat->objId[2]->crntRot.x *= 0.86f;
	strat->turnang.z =0;
  */

	if (strat->relVel.y)
	{
		strat->frame += 0.1f;
		strat->objId[3]->idleRot.x = strat->pnode->objId[3]->initRot.x - (float)sin(strat->frame) * 0.5f;
		strat->objId[5]->idleRot.x = strat->pnode->objId[5]->initRot.x - (float)sin(strat->frame + PI) * 0.5f;
		strat->objId[4]->idleRot.x = strat->pnode->objId[4]->initRot.x - (float)sin(strat->frame + HPI) * 0.8f;
		strat->objId[6]->idleRot.x = strat->pnode->objId[6]->initRot.x - (float)sin(strat->frame + PI + HPI) * 0.8f;
	}	
#endif
}

static void MiniRapierProcess(STRAT *strat)
{
#if 0
	int	i;
  /*	static	float	frame = 0; */
	float	Xang, Zang, zAngMul1, zAngMul2, zAngMul3, zAngMul4;
	float	xAngMul5, xAngMul6, xAngMul7, xAngMul8;

	float	localRotSpeedZ = 1.0f;
	float	localRotSpeedX = 0.5f;
	float	localRotSpeed  = 0.2f;
	float	localForce1 = 0.12f;
	float	localForce2;
	float	minX = -0.1f;

	
	localForce2 = (float)sqrt(localForce1);
	strat->frame += localRotSpeed;/* * strat->relVel.y * 5.0f; */

	/*ApplyRelForce(strat, 0, 0.0f, strat->pnode->mass * localForce1, 0.0f); */
	zAngMul1 = 1.0f;
	zAngMul2 = -1.0f;
	zAngMul3 = -1.0f;
	zAngMul4 = 1.0f;
	xAngMul5 = xAngMul6 = xAngMul7 = xAngMul8 = 0.0f;

	for (i=1; i<5; i++)
	{
		switch (i)
		{

			case 1:
				Xang = (float)sin(strat->frame) * localRotSpeedX;
				if (Xang < minX)
					Xang = minX;
				strat->objId[i]->idleRot.x = strat->pnode->objId[i]->initRot.x + Xang;
				if (zAngMul1)
				{
					Zang = (float)sin(strat->frame +HPI) * localRotSpeedZ;
					strat->objId[i]->idleRot.z = strat->pnode->objId[i]->initRot.z + Zang * zAngMul1;
				}
				break;
			case 2:
				Xang = (float)sin(strat->frame + PI) * localRotSpeedX;
				if (Xang < minX)
					Xang = minX;
				strat->objId[i]->idleRot.x = strat->pnode->objId[i]->initRot.x + Xang;
				if (zAngMul2)
				{
					Zang = (float)sin(strat->frame + PI + HPI) * localRotSpeedZ;
					strat->objId[i]->idleRot.z = strat->pnode->objId[i]->initRot.z + Zang * zAngMul2;
				}
				break;
			case 3:
				Xang = -(float)sin(strat->frame + HPI) * localRotSpeedX;
				if (Xang < minX)
					Xang = minX;
				strat->objId[i]->idleRot.x = strat->pnode->objId[i]->initRot.x + Xang;
				if (zAngMul3)
				{
					Zang = -(float)sin(strat->frame + PI) * localRotSpeedZ;
					strat->objId[i]->idleRot.z = strat->pnode->objId[i]->initRot.z + Zang * zAngMul3;
				}
				break;
			case 4:
				Xang = -(float)sin(strat->frame + PI + HPI) * localRotSpeedX;
				if (Xang < minX)
					Xang = minX;
				strat->objId[i]->idleRot.x = strat->pnode->objId[i]->initRot.x + Xang;
				if (zAngMul4)
				{
					Zang = -(float)sin(strat->frame) * localRotSpeedZ;
					strat->objId[i]->idleRot.z = strat->pnode->objId[i]->initRot.z + Zang * zAngMul4;
				}
				break;
		}
	}

	for (i=1; i<5; i++)
	{
		strat->objId[i]->crntRot.x += (strat->objId[i]->idleRot.x - strat->objId[i]->crntRot.x) * 0.2f;
		strat->objId[i]->crntRot.z += (strat->objId[i]->idleRot.z - strat->objId[i]->crntRot.z) * 0.2f;
		/*strat->objId[i]->crntRot.x = strat->objId[i]->idleRot.x; */
		/*strat->objId[i]->crntRot.z = strat->objId[i]->idleRot.z; */
	}
	if (strat->vel.x * strat->vel.x + strat->vel.y * strat->vel.y > 0.05f)
	{
		strat->vel.x *= 0.5f;
		strat->vel.y *= 0.5f;
	}
#endif
}

static void EscortTankerProcess(STRAT *crntStrat)
{
	Vector3 vec2;

	mVector3Multiply (&vec2, &crntStrat->relAcc, &crntStrat->m);

	crntStrat->vel.z += vec2.z; 
	crntStrat->vel.y += vec2.y; 
	crntStrat->vel.x += vec2.x; 

	crntStrat->vel.z *= 0.95f;
	crntStrat->rot_speed *= 0.95f;

	crntStrat->relAcc.z = 0.0; 
	crntStrat->relAcc.y = 0.0; 
	crntStrat->relAcc.x = 0.0; 
}


static void CameraProcess(STRAT *crntStrat)
{
	Vector3 vec2;

	if (crntStrat->turn.x != 0.0f)
		stratPitch(crntStrat, crntStrat->turn.x);
	if (crntStrat->turn.y != 0.0f)
		stratRoll(crntStrat, crntStrat->turn.y);
	if (crntStrat->turn.z != 0.0f)
		stratYaw(crntStrat, crntStrat->turn.z);
	mVector3Multiply (&vec2, &crntStrat->relAcc, &crntStrat->m);

	crntStrat->vel.z += vec2.z; 
	crntStrat->vel.y += vec2.y; 
	crntStrat->vel.x += vec2.x; 

	crntStrat->relAcc.z = 0.0; 
	crntStrat->relAcc.y = 0.0; 
	crntStrat->relAcc.x = 0.0; 

}

/*static void GroundSpeeder1Process(STRAT *crntStrat)
{
	Vector3 vec2;

	mVector3Multiply (&vec2, &crntStrat->relAcc, &crntStrat->m);

	crntStrat->vel.z += vec2.z; 
	crntStrat->vel.y += vec2.y; 
	crntStrat->vel.x += vec2.x; 

	crntStrat->vel.z *= 0.9f;
	crntStrat->rot_speed *= 0.5f;

	crntStrat->relAcc.z = 0.0; 
	crntStrat->relAcc.y = 0.0; 
	crntStrat->relAcc.x = 0.0; 
	crntStrat->objId[4]->crntRot = crntStrat->objId[4]->idleRot;
}

static void GroundSpeeder2Process(STRAT *crntStrat)
{
	Vector3 vec2;
	int		i;

	mVector3Multiply (&vec2, &crntStrat->relAcc, &crntStrat->m);

	crntStrat->vel.z += vec2.z; 
	crntStrat->vel.y += vec2.y; 
	crntStrat->vel.x += vec2.x; 

	crntStrat->vel.z *= 0.95f;
	crntStrat->rot_speed *= 0.8f;
	for (i=1; i<4; i++)
	{
		crntStrat->objId[i]->crntRot.x -= crntStrat->var * 0.5f;
		crntStrat->objId[i]->crntPos.z += (crntStrat->objId[i]->idlePos.z - crntStrat->objId[i]->crntPos.z) * 0.1f;
	}

	crntStrat->relAcc.z = 0.0; 
	crntStrat->relAcc.y = 0.0; 
	crntStrat->relAcc.x = 0.0; 
	crntStrat->objId[4]->crntRot.x = (crntStrat->objId[3]->idlePos.z - crntStrat->objId[3]->crntPos.z) * 0.5f;
}

static void GroundSpeeder3Process(STRAT *crntStrat)
{
	Vector3 vec2;
	int		i;

	mVector3Multiply (&vec2, &crntStrat->relAcc, &crntStrat->m);

	crntStrat->vel.z += vec2.z; 
	crntStrat->vel.y += vec2.y; 
	crntStrat->vel.x += vec2.x; 

	crntStrat->vel.z *= 0.95f;
	crntStrat->rot_speed *= 0.8f;
	for (i=1; i<4; i++)
	{
		crntStrat->objId[i]->crntRot.x += 0.5f;
		crntStrat->objId[i]->crntPos.z += (crntStrat->objId[i]->idlePos.z - crntStrat->objId[i]->crntPos.z) * 0.1f;
	}

	crntStrat->relAcc.z = 0.0; 
	crntStrat->relAcc.y = 0.0; 
	crntStrat->relAcc.x = 0.0; 
	crntStrat->objId[4]->crntRot.x = (crntStrat->objId[1]->idlePos.z - crntStrat->objId[1]->crntPos.z) * -0.5f;
	crntStrat->objId[5]->crntRot.x = (crntStrat->objId[2]->idlePos.z - crntStrat->objId[2]->crntPos.z) * -0.5f;
	crntStrat->objId[6]->crntRot.x = (crntStrat->objId[3]->idlePos.z - crntStrat->objId[3]->crntPos.z) * 0.5f;
}

*/
static void RedDogProcess(STRAT *strat)
{
	int i, pn;
	Point3	p1, p2;

	if (strat->var == 1234.f)
		return;

	pn = strat->Player->n;
	p1 = strat->pos;
	p1.z = 0.0f;

	for (i=0; i < noJumpPoints; i++)
	{
		p2 = jumpPoint[i].pos;
		p2.z = 0.0f;
		if (pSqrDist(&p1, &p2) < jumpPoint[i].radius * jumpPoint[i].radius)
			break;
	}

	if (!(strat->flag & COLL_ON_FLOOR) && (i < noJumpPoints))
	{
		player[pn].PlayerAirTime ++;
		strat->flag |= STRAT_REALGRAVITYON;
		strat->flag &= ~STRAT_GRAVITYON;
	}
	else if (strat->flag & COLL_ON_FLOOR)
	{
		if (player[pn].PlayerAirTime)
			player[pn].PlayerAfterJumpCountDown = 10;
		player[pn].PlayerAirTime = 0;
		strat->flag &= ~STRAT_REALGRAVITYON;
		strat->flag |= STRAT_GRAVITYON;
	}

	switch(player[pn].cameraPos)
	{
		case FIRST_PERSON:
			/*strat->obj->collFlag |= OBJECT_INVISIBLE;
			HideObject(strat, 1);
			HideObject(strat, 2);
			HideObject(strat, 3);
			HideObject(strat, 4);
			HideObject(strat, 5);
			HideObject(strat, 6);
			HideObject(strat, 7);
			HideObject(strat, 8);
			HideObject(strat, 9);
			HideObject(strat, 10);
			HideObject(strat, 14);
			HideObject(strat, 15);
			HideObject(strat, 16);
			HideObject(strat, 17);*/

			break;
		case THIRD_PERSON_CLOSE:
			strat->obj->collFlag &= 0xffffffff - OBJECT_INVISIBLE;
			UnhideObject(strat, 1);
			UnhideObject(strat, 2);
			UnhideObject(strat, 3);
			UnhideObject(strat, 4);
			UnhideObject(strat, 5);
			UnhideObject(strat, 6);
			UnhideObject(strat, 7);
			UnhideObject(strat, 8);
			UnhideObject(strat, 9);
			UnhideObject(strat, 10);
			if (!Multiplayer)
			{
				UnhideObject(strat, 14);
				UnhideObject(strat, 15);
				UnhideObject(strat, 16);
			}
			UnhideObject(strat, 17);
			break;
	}
	switch(player[pn].PlayerState)
	{
		//case PS_SCRIPTHELD:
	   //		RedDogProcessEntry(pn);
	   //		break;
		case PS_NORMAL:
		case PS_ONPATH:
			RedDogProcessNormal(pn);
			break;
		case PS_WATERSLIDE:
			RedDogProcessWaterSlide(pn);
			break;
		case PS_SIDESTEP:
			RedDogProcessSideStep(pn);
			break;
		case PS_TOWERLIFT:
			RedDogProcessTowerLift(pn);
			break;
		case PS_SCRIPTHELD:
		case PS_INDROPSHIP:
			RedDogProcessNoControl(pn);
			break;
		case PS_ONTRAIN:
			//RedDogProcessOnTrain(pn);
		 	break;
		default:
			break;
	}
}

static void MultipleMissileLauncherProcess(STRAT *strat)
{
	int i;
	Vector3 vec2;
	Object *obj;

	for (i=1; i<=6; i++)
	{
		obj = strat->objId[i];

		if ((i==1) || (i==2) || (i==3))
			obj->crntRot.x -= (strat->relVel.y / obj->cp[0].radius) - (strat->turnangz * 2.0f);
		else
			obj->crntRot.x -= (strat->relVel.y / obj->cp[0].radius) + (strat->turnangz * 2.0f);

		obj->crntPos.z += (obj->idlePos.z - obj->crntPos.z) * 0.1f;
	}
	
	strat->turnangz = 0.0f;
	mVector3Multiply(&vec2, &strat->relAcc, &strat->m);
	v3Inc(&strat->vel, &vec2);
	strat->relAcc.z = 0.0; 
	strat->relAcc.y = 0.0; 
	strat->relAcc.x = 0.0; 
}

static void BattleTank2Process(STRAT *strat)
{
	int i;
	Vector3 vec2;

	if (strat->health <= 0.0)
		return;

	for (i=1; i<=6; i++)
	{
		if ((i==1) || (i==2) || (i==3))
			strat->objId[i]->crntRot.x -= (strat->relVel.y / 2.5f) + (strat->turnangz * 2.0f);
		else
			strat->objId[i]->crntRot.x -= (strat->relVel.y / 2.5f) - (strat->turnangz * 2.0f);

		strat->objId[i]->crntPos.z += (strat->objId[i]->idlePos.z - strat->objId[i]->crntPos.z) * 0.1f;
	}
	
	strat->turnangz = 0.0f;
	mVector3Multiply(&vec2, &strat->relAcc, &strat->m);
	v3Inc(&strat->vel, &vec2);
	strat->relAcc.z = 0.0; 
	strat->relAcc.y = 0.0; 
	strat->relAcc.x = 0.0; 
}

static void SecondaryGunProcess(STRAT *crntStrat)
{
	Vector3 vec2;

	if (crntStrat->turn.x != 0.0f)
		stratPitch(crntStrat, crntStrat->turn.x);
	if (crntStrat->turn.y != 0.0f)
		stratRoll(crntStrat, crntStrat->turn.y);
	if (crntStrat->turn.z != 0.0f)
		stratYaw(crntStrat, crntStrat->turn.z);
	mVector3Multiply (&vec2, &crntStrat->relAcc, &crntStrat->m);

	crntStrat->vel.z += vec2.z; 
	crntStrat->vel.y += vec2.y; 
	crntStrat->vel.x += vec2.x; 

	crntStrat->relAcc.z = 0.0; 
	crntStrat->relAcc.y = 0.0; 
	crntStrat->relAcc.x = 0.0; 

}

/*static void OutPostBossProcess(STRAT *strat)
{
	Vector3 vec2;

	if (strat->turn.x != 0.0f)
		stratPitch(strat, strat->turn.x);
	if (strat->turn.y != 0.0f)
		stratRoll(strat, strat->turn.y);
	if (strat->turn.z != 0.0f)
		stratYaw(strat, strat->turn.z);
	mVector3Multiply (&vec2, &strat->relAcc, &strat->m);

	strat->vel.z += vec2.z; 
	strat->vel.y += vec2.y; 
	strat->vel.x += vec2.x; 

	strat->relAcc.z = 0.0; 
	strat->relAcc.y = 0.0; 
	strat->relAcc.x = 0.0; 
	calcRelVel(strat);
	RelRotateY(strat, 0.01f * strat->relVel.x);
	RelRotateX(strat, -0.01f * strat->relVel.y);
	Flatten(strat, 1.0f);
	strat->objId[2]->crntRot.y = strat->relVel.x * 2.0f;
	strat->objId[5]->crntRot.y = strat->relVel.x * 2.0f;
	strat->objId[6]->crntRot.z -= 0.5f;
	strat->objId[1]->crntRot.z -= 0.5f;
	strat->objId[4]->crntRot.z -= 0.5f;
	strat->objId[20]->crntRot.x = -strat->relVel.y;

}
*/

static void CanyonBossProcess(STRAT *strat)
{
	Vector3 vec2;
	int i;
	float max = 0.0f;

	if (strat->turn.x != 0.0f)
		stratPitch(strat, strat->turn.x);
	if (strat->turn.y != 0.0f)
		stratRoll(strat, strat->turn.y);
	if (strat->turn.z != 0.0f)
		stratYaw(strat, strat->turn.z);
	mVector3Multiply (&vec2, &strat->relAcc, &strat->m);

	strat->vel.z += vec2.z; 
	strat->vel.y += vec2.y; 
	strat->vel.x += vec2.x; 

	strat->relAcc.z = 0.0; 
	strat->relAcc.y = 0.0; 
	strat->relAcc.x = 0.0; 
	calcRelVel(strat);
	//RelRotateY(strat, -0.04f * strat->relVel.x);

	Flatten(strat, 1.0f);

	
}

/*static void RaftProcess(STRAT *crntStrat)
{
	Vector3 vec2;

	if (crntStrat->turn.x != 0.0f)
		stratPitch(crntStrat, crntStrat->turn.x);
	if (crntStrat->turn.y != 0.0f)
		stratRoll(crntStrat, crntStrat->turn.y);
	if (crntStrat->turn.z != 0.0f)
		stratYaw(crntStrat, crntStrat->turn.z);
	mVector3Multiply (&vec2, &crntStrat->relAcc, &crntStrat->m);

	crntStrat->vel.x += vec2.x; 
	crntStrat->vel.y += vec2.y; 
	crntStrat->vel.z += vec2.z; 


	crntStrat->relAcc.x = 0.0; 
	crntStrat->relAcc.y = 0.0; 
	crntStrat->relAcc.z = 0.0;
	crntStrat->vel.x *= 0.85f;
	crntStrat->vel.y *= 0.85f;
}
*/
static void PresidentsCarProcess(STRAT *crntStrat)
{
	Vector3 vec2, tempv;
	int i;
	float tempf;
	float SCALEFAC = 1.0f;

	if (PAL)
		SCALEFAC = 1.2f;


	if (crntStrat == BoatStrat)
		return;
	if (crntStrat->turn.x != 0.0f)
		stratPitch(crntStrat, crntStrat->turn.x);
	if (crntStrat->turn.y != 0.0f)
		stratRoll(crntStrat, crntStrat->turn.y);
	if (crntStrat->turn.z != 0.0f)
		stratYaw(crntStrat, crntStrat->turn.z);
	mVector3Multiply (&vec2, &crntStrat->relAcc, &crntStrat->m);

	v3Sub(&tempv, &crntStrat->oldPos, &crntStrat->oldOldPos);
	tempf = v3Length(&tempv);
	for (i=1; i<5; i++)
	{
		crntStrat->objId[i]->crntRot.x -= tempf * 1.538f * SCALEFAC;
		crntStrat->objId[i]->crntPos.z += (crntStrat->objId[i]->idlePos.z - crntStrat->objId[i]->crntPos.z) * 0.05f * SCALEFAC;
	}

	crntStrat->rot_speed *= 0.9f;
	crntStrat->vel.z += vec2.z; 
	crntStrat->vel.y += vec2.y; 
	crntStrat->vel.x += vec2.x; 

	crntStrat->relAcc.z = 0.0; 
	crntStrat->relAcc.y = 0.0; 
	crntStrat->relAcc.x = 0.0; 
	
}

void *processSpecProc[] = 
{
	&NonAnimProcess,					//0
	&RedDogProcess,						//1
	&NonAnimProcess,					//2
	&NonAnimProcess,					//3
	&waterProcess,						//4
	&BattleTank1_2Process,				//5
	&MiniRapierProcess,					//6
	&A00Walker01Process,				//7
	&MorterTankProcess,					//8
	&EscortTankerProcess,				//9
	&CameraProcess,						//10
	&NonAnimProcess,					//11
	&RedDogProcess,						//12
	&NonAnimProcess,					//13
	&NonAnimProcess,					//14
	&MultipleMissileLauncherProcess,	//15
	&BattleTank2Process,				//16
	&NonAnimBounceProcess,				//17
	&SecondaryGunProcess,				//18
	&NonAnimProcess,					//19
	&CanyonBossProcess,					//20
	&NonAnimProcess,					//21
   	&NonAnimProcess,					//FSLEVEL2DROP 22
	&PresidentsCarProcess				//23

};