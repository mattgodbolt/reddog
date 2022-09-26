#include "RedDog.h"
#include "view.h"
#include "strat.h"
#include "draw.h"
#include "hud.h"
#include "input.h"
#include "Player.h"
#include "command.h"
#include "stratphysic.h"
#include "camera.h"
#include "process.h"
#include "coll.h"
#include "MPlayer.h"
#include "render\internal.h"


extern	float smoothSideStepSum(int pn);

extern	Map		*Savmap;
extern	PLAYER player[4];
extern	PNode *spriteHolder;
extern	PNode	*spriteLock;
extern	LEVELSETTINGS   *LevelSettings;
extern	Material	textureShootMe;
extern int frameCount;

#define CAM_TILT_DIR 0.0f


PNode	*targetTLSpriteHolder;
PNode	*targetTRSpriteHolder;
PNode	*targetBRSpriteHolder;
PNode	*targetBLSpriteHolder;
PNode	*targetSpriteHolder;

float wideAmt = 0.f;
extern	int caIndex;
extern	Vector3 collVect;
extern	Point3 collArrayPos[32];
extern Point3 collArrayOrigin[32];
extern STRAT *collArrayStrat[32];
extern Object *collArrayObject[32];

extern int lineCollBBoxPolyColl(Point3 *ls, Point3 *le, CollBBox *cb, Point3 *p, Vector3 *n, int *polyType);
extern void lineCollBBoxColl(Point3 *ps, Point3 *pe, CollBBox *cb, Point3 *ls, Point3 *le);
extern void doHighVColl(STRAT *crntStrat, CollObject *co, Point3 *ocpt, Point3 *cpt, HDStrat *hds, int type);
//returns 0 if invalid screen space
//1 if valid

Bool lineLandscapeCollisionA(int pn, Point3 *ps, Point3 *pe, Point3 *p, STRAT *strat, Vector3 *n, int *polyType)
{
	CollBBox	cb;
	CollLeaf	*cl = NULL;
	float		dx,dy, cpl = 0.0f, caDist, tempf;
	int			col = 0, colhdc = 0, cax;
	Vector3		v, vn, from, to;
	Point3		cp, ls, le, collP;
	HDStrat		*hds;

	from = *ps;
	to = *pe;

	hds = firstHDStrat;

	caIndex = 0;
	while(hds)
	{
		if (hds->strat == strat)
		{
			hds = hds->next;
			continue;
		}
		if ((hds->strat->flag & STRAT_COLLSTRAT) && (hds->valid) && (!(hds->strat->flag & STRAT_HIDDEN)))
		{
			if (pointLineDistanceFix(&hds->strat->pos, &from, &to) < hds->strat->pnode->radius + 10)
			{			
				collVect.x = 0.0;
				collVect.y = 0.0;
				collVect.z = 0.0;
				/****** WHAT IS THIS MATT !!!!!!!****/
				

				doHighVColl(NULL, hds->collObj, &from, &to, hds, 1);
				if ((collVect.x != 0.0) || (collVect.y != 0.0) || (collVect.z != 0.0))
				{
					colhdc = 1;
					//v3Inc(&to, &collVect);
				}

				
				
			}
		}
		hds = hds->next;
	}

	if (caIndex > 0)
	{
		caDist = pSqrDist(&collArrayPos[caIndex - 1], &collArrayOrigin[caIndex - 1]);
		cax = caIndex;
		caIndex--;

		while (caIndex > 0)
		{
			tempf = pSqrDist(&collArrayPos[caIndex - 1], &collArrayOrigin[caIndex - 1]);
			if (tempf < caDist)
			{
				caDist = tempf;
				cax = caIndex;
			}
			caIndex--;
		}

		player[pn].aimStrat = collArrayStrat[cax - 1];
		player[pn].aimObj = collArrayObject[cax - 1];
	}

	caIndex = 0;

	if (colhdc)
		*p = to;

	cp = from;
	v3Sub(&v, &to, &from);
	vn = v;
	v3Normalise(&vn);
	v3Scale(&vn, &vn, 0.01f);
	if (fabs(from.x - to.x) < 0.001f)
		dx = 1.0f / 0.001f;
	else
		dx = 1.0f / fabs(from.x - to.x);

	if (fabs(from.y - to.y) < 0.001f)
		dy = 1.0f / 0.001f;
	else
		dy = 1.0f / fabs(from.y - to.y);

	while (cpl < 1.0f)
	{
		findCollBBox(&cp, &cb);
		if (cl == cb.leaf)
			break;
		cl = cb.leaf;
		lineCollBBoxColl(&from, &to, &cb, &ls, &le);
		col = lineCollBBoxPolyColl(&ls, &le, &cb, &collP, n, polyType);
		if (col)
		{
			*p = collP;
			return TRUE;
		}
		
		cp = le;
		v3Inc(&cp, &vn);
		if (dx < dy)
			cpl = fabs(cp.x - from.x) * dx;
		else
			cpl = fabs(cp.y - from.y) * dy;
	}

	return FALSE;
}


static int point3ToScreen(Point3 *pos, Point2 *p2)
{
	Point p;
	float rW;

	mLoadModelToScreen();

	*(Point3 *)&p = *pos;


	p.w = 1.f;
	mPoint (&p, &p);

   	if (p.z < 0)
		return(0);


	rW = 1.f / p.w; // == 1/p.2 * 0.5f
	p2->x = pViewMidPoint.x + pViewSize.x * p.x * rW;
	p2->y = pViewMidPoint.y + pViewSize.y * p.y * rW;
	return(1);
}


void resetCamera(int pn)
{
	int i;

	player[pn].cam->u.lookDir.rotX = player[pn].cam->u.lookDir.rotY = player[pn].cam->u.lookDir.rotZ = 0.0f;
	player[pn].cam->HoldTime = 0;
	player[pn].targlookDir.x = player[pn].targlookDir.y = player[pn].targlookDir.z = 0.0f;
  	player[pn].cam->pos.x = player[pn].cam->pos.y = player[pn].cam->pos.z = 0;
	player[pn].camLookCrntPos.x = player[pn].camLookCrntPos.y = player[pn].camLookCrntPos.z = 1.0f;
	player[pn].TargetList = NULL;
	player[pn].joyPos.x = player[pn].joyPos.y =0.0f;
	player[pn].cameraPos = THIRD_PERSON_CLOSE;
	player[pn].CamLookOff.x = player[pn].CamLookOff.y = player[pn].CamLookOff.z = 0.0f;
	player[pn].CamPosOff.x = player[pn].CamPosOff.y = player[pn].CamPosOff.z = 0.0f;
	player[pn].PlayerFriction.x = player[pn].PlayerFriction.y = player[pn].PlayerFriction.z = 0.0f;
	player[pn].PlayerOldVel.x = player[pn].PlayerOldVel.y = player[pn].PlayerOldVel.z = 0.0f;
	player[pn].currentTarget = 0;
	player[pn].PlayerState = PS_NORMAL;
	player[pn].PlayerHeld = 0;
	noJumpPoints = 0;
	player[pn].targetting = 0;
	player[pn].PlayerWeaponPower = 0;
	player[pn].gunDischarged = 1;
	if (!Multiplayer || GetCurrentGameType() != PREDATOR || !player[pn].special)
		player[pn].PlayerSecondaryWeapon.type = 0;
	player[pn].PlayerSecondaryWeapon.amount = 0;
}

static void findTarget(Point3 *p, int pn)
{
	Vector3	camDir, joyDir, rot;
	Matrix	m;
	float	ang;

	if (!player[pn].CameraStrat)
		return;

	
	if (joyState[pn].x > 0.0f)
		joyDir.x = ((joyState[pn].x * 0.4f) - player[pn].joyPos.x);
	else
		joyDir.x = ((joyState[pn].x * 0.4f) - player[pn].joyPos.x);

	if (joyState[pn].y > 0.0f)
		joyDir.y = (joyState[pn].y * 0.4f) - player[pn].joyPos.y;
	else
		joyDir.y = (joyState[pn].y * 0.2f) - player[pn].joyPos.y;
		
	if (player[pn].PlayerState & PS_SIDESTEP)
		player[pn].joyPos.x = 0.0f;
	else
		player[pn].joyPos.x += (joyDir.x * 0.2f);
	player[pn].joyPos.y += joyDir.y * 0.2f;


	rot.x = player[pn].targlookDir.x - (player[pn].joyPos.y * (0.8f + player[pn].zooming * 1.14f));
	rot.y = player[pn].targlookDir.z;
	rot.z = player[pn].targlookDir.y + (player[pn].joyPos.x * (0.9f + player[pn].zooming * 1.27f));
	mRotateXYZ(&m, rot.x, rot.y, -rot.z);

	camDir.x = m.m[1][0];
	camDir.y = m.m[1][1];
	camDir.z = m.m[1][2];
	player[pn].GlobalCamMatrix = m;

	if (Multiplayer)
	{
		p->x = player[pn].CameraStrat->pos.x + (camDir.x * 300.0f); 
		p->y = player[pn].CameraStrat->pos.y + (camDir.y * 300.0f); 
		p->z = player[pn].CameraStrat->pos.z + (camDir.z * 300.0f); 
	}
	else
	{
		p->x = player[pn].CameraStrat->pos.x + (camDir.x * 300.0f); 
		p->y = player[pn].CameraStrat->pos.y + (camDir.y * 300.0f); 
		p->z = player[pn].CameraStrat->pos.z + (camDir.z * 300.0f); 
	}
}



static void CamMoveTo(Point3 *p, int pn)
{
	Vector3	tempv;
	float SCALEFAC = 1.0f;
	Point3 camp, ps, pe;

	switch(player[pn].cameraPos)
	{
		case THIRD_PERSON_CLOSE:
			/*v3Sub(&tempv, &player[0].CameraStrat->pos, &player[0].camLookCrntPos);
			v3Normalise(&tempv);
			v3Scale(&tempv, &tempv, 2.0f);
			v3Add(&pe, &player[0].CameraStrat->pos, &tempv);
			ps = player[0].camLookCrntPos;

			if (lineLandscapeCollision(&ps, &pe, &camp, NULL, NULL))
			{
				v3Scale(&tempv, &tempv, 0.9f);
				v3Dec(&camp, &tempv);
				*p = camp;
			}*/

			if ((BossStrat) && (BossStrat->pnode))
			{
				if (BossStrat->pnode->typeId == 20)
				{
					player[0].CameraStrat->pos.x = player[0].Player->pos.x;
					player[0].CameraStrat->pos.y = player[0].Player->pos.y + 10.0f;
					player[0].CameraStrat->pos.z = player[0].Player->pos.z + 3.5f;
					v3Sub(&tempv, &player[pn].Player->pos, &player[pn].Player->oldPos);
					if (PAL)
						v3Scale(&tempv,&tempv,PAL_MOVESCALE);

					v3Inc(&player[pn].CameraStrat->pos, &tempv);
					//v3Inc(&player[pn].CameraStrat->pos, &tempv);
					break;
				}
			}
			dAssert(1, "");
			v3Sub(&tempv, p, &player[pn].CameraStrat->pos);
			dAssert(1, "");
		   
			//v3Scale(&tempv, &tempv, 0.2f * (0.7f + player[pn].zooming) * 1.4286f);
			if (player[pn].PlayerHeld)
				v3Scale(&tempv, &tempv, 0.1f);
			else
				v3Scale(&tempv, &tempv, 0.2f);
			dAssert(1, "");
			if (PAL)
				v3Scale(&tempv,&tempv,PAL_MOVESCALE);

			v3Inc(&player[pn].CameraStrat->pos, &tempv);
			dAssert(1, "");
			if (player[0].onBoat == 1)
			{
				v3Sub(&tempv, &player[0].Player->oldPos, &player[0].Player->oldOldPos);
				//v3Inc(&player[pn].CameraStrat->pos, &tempv);
			}
			else
			{
				//if (player[pn].PlayerOnHoldPlayer)
				//{
				//	v3Sub(&tempv, &player[pn].PlayerOnHoldPlayer->strat->oldPos, &player[pn].PlayerOnHoldPlayer->strat->oldOldPos);
				//	v3Inc(&player[pn].CameraStrat->pos, &tempv);
				//}
			}

			break;
		case FIRST_PERSON:
			player[pn].CameraStrat->pos = *p;
			v3Inc(&player[pn].CameraStrat->pos, &player[pn].Player->vel);
		  	if (PAL)
				SCALEFAC = PAL_MOVESCALE;
			player[pn].CameraStrat->pos.x += player[pn].Player->m.m[1][0] * 0.5f * SCALEFAC;
			player[pn].CameraStrat->pos.y += player[pn].Player->m.m[1][1] * 0.5f * SCALEFAC;
			player[pn].CameraStrat->pos.z += player[pn].Player->m.m[1][2] * 0.5f * SCALEFAC;

			break;
	}
}

static void TowerLiftCamMoveTo(Point3 *p, int pn)
{
	Vector3	tempv;

	switch(player[pn].cameraPos)
	{
		case THIRD_PERSON_CLOSE:
			player[pn].CameraStrat->pos = *p;
			break;
		case FIRST_PERSON:
			player[pn].CameraStrat->pos = *p;
			break;
	}
}

static void CamLookAt(Point3 *p, int pn)
{
	float	xAng, yAng, zAng, xdiff, ydiff, zdiff, camSpeed;
	Vector3	dir, tempv, tempv2, tempv3, camLookVel;

	if (!player[pn].CameraStrat)
		return;

	/*if (player[pn].PlayerHeld)
	{
		switch(player[pn].PlayerState)
		{
			case PS_TOWERLIFT:
				dir.x = p->x - player[pn].camLookCrntPos.x;						 
				dir.y = p->y - player[pn].camLookCrntPos.y;
				dir.z = p->z - player[pn].camLookCrntPos.z;
				dir.x *= 0.5f;
				dir.y *= 0.5f;
				dir.z *= 0.2f;
				if (PAL)
					v3Scale(&dir,&dir,PAL_MOVESCALE);
				v3Inc(&player[pn].camLookCrntPos, &dir);
				break;
			case PS_ONTRAIN:
				dir.x = p->x - player[pn].camLookCrntPos.x;						 
				dir.y = p->y - player[pn].camLookCrntPos.y;
				dir.z = p->z - player[pn].camLookCrntPos.z;
				dir.x *= 0.8f;
				dir.y *= 0.8f;
				dir.z *= 0.8f;
				if (PAL)
					v3Scale(&dir,&dir,PAL_MOVESCALE);
				v3Inc(&player[pn].camLookCrntPos, &dir);
				break;
		  /*	case PS_INDROPSHIP:
				dir.x = p->x - player[pn].camLookCrntPos.x;						 
				dir.y = p->y - player[pn].camLookCrntPos.y;
				dir.z = p->z - player[pn].camLookCrntPos.z;
				dir.x *= 1.8f;
				dir.y *= 1.8f;
				dir.z *= 1.8f;
				v3Inc(&player[pn].camLookCrntPos, &dir);
				break;		  */
	/*		default:
				if (BossStrat)
				{
					if (BossStrat->pnode->typeId == 20)
					{
						player[pn].camLookCrntPos.x = player[pn].Player->pos.x;
						player[pn].camLookCrntPos.y = player[pn].Player->pos.y;
						player[pn].camLookCrntPos.z = player[pn].Player->pos.z + 2.0f;
						v3Sub(&tempv, &player[pn].Player->pos, &player[pn].Player->oldPos);
						//v3Inc(&player[pn].camLookCrntPos, &tempv);
						break;
					}
				}
				player[pn].camLookCrntPos = *p;
			 	break;
		}
	}
	else*/
	if ((BossStrat) && (BossStrat->pnode))
	{
		if (BossStrat->pnode->typeId == 20)
		{
			player[0].camLookCrntPos.x = player[0].Player->pos.x;
			player[0].camLookCrntPos.y = player[0].Player->pos.y - 10.0f;
			player[0].camLookCrntPos.z = player[0].Player->pos.z + 1.0f;
		}
	}
	
	if (player[pn].cameraPos == FIRST_PERSON)
	{
		player[pn].camLookCrntPos.x += player[pn].Player->m.m[0][0] * player[pn].smooothJoyX;
		player[pn].camLookCrntPos.y += player[pn].Player->m.m[0][1] * player[pn].smooothJoyX;
		player[pn].camLookCrntPos.z += player[pn].Player->m.m[0][2] * player[pn].smooothJoyX;
	}
	dir.x = p->x - player[pn].camLookCrntPos.x;						 
	dir.y = p->y - player[pn].camLookCrntPos.y;
	dir.z = p->z - player[pn].camLookCrntPos.z;

	v3Scale(&dir, &dir, 0.2f * (0.8f + player[pn].zooming) * 1.25f);
	if (PAL)
	   	v3Scale(&dir,&dir,PAL_MOVESCALE);
	v3Inc(&player[pn].camLookCrntPos, &dir);
	
	
	xdiff = player[pn].CameraStrat->pos.x - player[pn].camLookCrntPos.x;
	ydiff = player[pn].CameraStrat->pos.y - player[pn].camLookCrntPos.y;
	zdiff = player[pn].CameraStrat->pos.z - player[pn].camLookCrntPos.z;
	tempv.x = -xdiff;
	tempv.y = -ydiff;
	tempv.z = -zdiff;
	

	tempv2.x = 0.0f;
	tempv2.y = 0.0f;
	tempv2.z = 1.0f;
	v3Normalise(&tempv);
	if (tempv.z <= -0.99f)
	{
		tempv.x = 0.01f;
		tempv.y = 0.01f;
		tempv.z = -9.9f;
		v3Normalise(&tempv);
	}
	player[pn].CameraStrat->m.m[1][0] = tempv.x;
	player[pn].CameraStrat->m.m[1][1] = tempv.y;
	player[pn].CameraStrat->m.m[1][2] = tempv.z;
	if (acos(v3Dot(&tempv, &tempv2)) < 0.01f)
	{
		tempv.x = 0.0;
		tempv.y = 1.0;
		tempv.z = 0.0;
	}
	v3Cross(&tempv3, &tempv2, &tempv);
	v3Normalise(&tempv3);
	player[pn].CameraStrat->m.m[0][0] = tempv3.x;
	player[pn].CameraStrat->m.m[0][1] = tempv3.y;
	player[pn].CameraStrat->m.m[0][2] = tempv3.z;
	v3Cross(&tempv2, &tempv, &tempv3);
	player[pn].CameraStrat->m.m[2][0] = tempv2.x;
	player[pn].CameraStrat->m.m[2][1] = tempv2.y;
	player[pn].CameraStrat->m.m[2][2] = tempv2.z;

	
#if 0
	xAng = (float)atan2( (double)zdiff, sqrt(xdiff * xdiff + ydiff * ydiff));
	yAng = (float)atan2( (double)player[pn].CameraStrat->pos.z-player[pn].camLookCrntPos.z, 
						 (double)player[pn].CameraStrat->pos.x-player[pn].camLookCrntPos.x);
	zAng = (float)atan2( (double)player[pn].CameraStrat->pos.x-player[pn].camLookCrntPos.x, 
						 (double)player[pn].CameraStrat->pos.y-player[pn].camLookCrntPos.y);
#endif
	xAng =fatan2( zdiff, sqrt(xdiff * xdiff + ydiff * ydiff));
	yAng =fatan2( player[pn].CameraStrat->pos.z-player[pn].camLookCrntPos.z, 
						 player[pn].CameraStrat->pos.x-player[pn].camLookCrntPos.x);
	zAng =fatan2( player[pn].CameraStrat->pos.x-player[pn].camLookCrntPos.x, 
						 player[pn].CameraStrat->pos.y-player[pn].camLookCrntPos.y);

//	xAng = xAng / (1.f / (2*PI));
//	yAng = yAng / (1.f / (2*PI));
//	zAng = zAng / (1.f / (2*PI));
	xAng = xAng * (2*PI);
	yAng = yAng * (2*PI);
	zAng = zAng * (2*PI);






	player[pn].cam->u.lookDir.rotX = (-PI + xAng);
	player[pn].cam->u.lookDir.rotY = zAng;
	player[pn].cam->u.lookDir.rotZ = -PI;

	player[pn].targlookDir.x = player[pn].cam->u.lookDir.rotX;
	player[pn].targlookDir.y = player[pn].cam->u.lookDir.rotY;
	player[pn].targlookDir.z = player[pn].cam->u.lookDir.rotZ;

	direction = -zAng;
}

static void TowerLiftCamLookAt(Point3 *p, int pn)
{
	float	xAng, yAng, zAng, xdiff, ydiff, zdiff, camSpeed;
	Vector3	dir, tempv, camLookVel;

	if (!player[pn].CameraStrat)
		return;

	player[pn].camLookCrntPos = *p;
	
	xdiff = player[pn].CameraStrat->pos.x - player[pn].camLookCrntPos.x;
	ydiff = player[pn].CameraStrat->pos.y - player[pn].camLookCrntPos.y;
	zdiff = player[pn].CameraStrat->pos.z - player[pn].camLookCrntPos.z;
#if 0
	xAng = (float)atan2( (double)zdiff, sqrt(xdiff * xdiff + ydiff * ydiff));
	yAng = (float)atan2( (double)player[pn].CameraStrat->pos.z-player[pn].camLookCrntPos.z, 
						 (double)player[pn].CameraStrat->pos.x-player[pn].camLookCrntPos.x);
	zAng = (float)atan2( (double)player[pn].CameraStrat->pos.x-player[pn].camLookCrntPos.x, 
						 (double)player[pn].CameraStrat->pos.y-player[pn].camLookCrntPos.y);
#endif
	xAng = PI2 * fatan2( zdiff, sqrt(xdiff * xdiff + ydiff * ydiff));
	yAng = PI2 * fatan2( player[pn].CameraStrat->pos.z-player[pn].camLookCrntPos.z, 
						player[pn].CameraStrat->pos.x-player[pn].camLookCrntPos.x);
	zAng = PI2 * fatan2( player[pn].CameraStrat->pos.x-player[pn].camLookCrntPos.x, 
						player[pn].CameraStrat->pos.y-player[pn].camLookCrntPos.y);

	player[pn].cam->u.lookDir.rotX = (-PI + xAng);
	player[pn].cam->u.lookDir.rotY = zAng;
	player[pn].cam->u.lookDir.rotZ = -PI;

	player[pn].targlookDir.x = player[pn].cam->u.lookDir.rotX;
	player[pn].targlookDir.y = player[pn].cam->u.lookDir.rotY;
	player[pn].targlookDir.z = player[pn].cam->u.lookDir.rotZ;

	direction = -zAng;
}

static void CamLookAtDirect(Point3 *p, int pn)
{
	float	xAng, yAng, zAng, xdiff, ydiff, zdiff;
	Vector3	dir;

	
	xdiff = player[pn].cam->pos.x - p->x;
	ydiff = player[pn].cam->pos.y - p->y;
	zdiff = player[pn].cam->pos.z - p->z;
	
	#if 0
		xAng = (float)atan2( (double)zdiff, sqrt(xdiff * xdiff + ydiff * ydiff));
		yAng = (float)atan2( (double)player[pn].cam->pos.z-p->z, (double)player[pn].cam->pos.x-p->x);
		zAng = (float)atan2( (double)player[pn].cam->pos.x-p->x, (double)player[pn].cam->pos.y-p->y);
	#endif
	
	xAng = PI2 * fatan2( zdiff, sqrt(xdiff * xdiff + ydiff * ydiff));
	yAng = PI2 * fatan2( player[pn].cam->pos.z-p->z, player[pn].cam->pos.x-p->x);
	zAng = PI2 * fatan2( player[pn].cam->pos.x-p->x, player[pn].cam->pos.y-p->y);




	player[pn].cam->u.lookDir.rotX = (-PI + xAng);
	player[pn].cam->u.lookDir.rotY = zAng;
	player[pn].cam->u.lookDir.rotZ = -PI;

	/* Update the HUD direction*/
	direction = -zAng;
}

static void TrackLookAt(Vector3 *pos, int pn)
{
	Vector3	dir,POSOFF;
	Point3 p;
	int	camPosId, camLookPosId;

 	/*FIRST FLOAT PARAM IS THE FOV*/
  	player[pn].cam->screenAngle = player[pn].CameraStrat->parent->FVar[0]; 
   /*	CamLookAt(pos);	*/

	// First IVar is widescreen or not
	if (pn != 3) { // special case for 3 player game
		if (player[pn].CameraStrat->parent->IVar[0]) {
			player[pn].cam->flags |= 1;
			if (player[pn].CameraStrat->parent->IVar[0] < 0) {
				wideAmt = -player[pn].CameraStrat->parent->IVar[0]/100.f;
			} else {
				wideAmt = 1.0;
			}
			player[pn].cam->screenY = PHYS_SCR_Y - (PHYS_SCR_Y - ((9 * PHYS_SCR_X)/16)) * Delta (wideAmt);
		} 
		else
		{
			player[pn].cam->flags &= ~1;
			if (!(player[pn].cam->flags & 2))
			  	player[pn].cam->screenY = PHYS_SCR_Y;
			else
			{
				player[pn].cam->flags |= 1;
			player[pn].cam->screenY = PHYS_SCR_Y - (PHYS_SCR_Y - ((9 * PHYS_SCR_X)/16)) * Delta (wideAmt);
				wideAmt = wideAmt * 0.98;
			}
		}
	}

	camPosId = GetSem(player[pn].CameraStrat->parent, 0);
	camLookPosId = GetSem(player[pn].CameraStrat->parent, 1);
	if ((camPosId > 0) && player[pn].CameraStrat->parent->parent->objId)
	{
		p.x = player[pn].CameraStrat->parent->parent->objId[camPosId]->wm.m[3][0];
		p.y = player[pn].CameraStrat->parent->parent->objId[camPosId]->wm.m[3][1];
		p.z = player[pn].CameraStrat->parent->parent->objId[camPosId]->wm.m[3][2];
	}
	else
		p = player[pn].CameraStrat->parent->pos;

   	player[pn].cam->pos = p;

	//HAS THE CAMERA GOT A NON-PLAYER TARGET ?
		//HAS THE CAMERA TRACK STRAT GOT A PARENT ?
		//IF SO, THEN LOOK AT IT
		if (player[pn].CameraStrat->parent->parent)
		{
			if ((camLookPosId > 0)&& player[pn].CameraStrat->parent->parent->objId)

			{
				p.x = player[pn].CameraStrat->parent->parent->objId[camLookPosId]->wm.m[3][0];
				p.y = player[pn].CameraStrat->parent->parent->objId[camLookPosId]->wm.m[3][1];
				p.z = player[pn].CameraStrat->parent->parent->objId[camLookPosId]->wm.m[3][2];
			}
			else
				p = player[pn].CameraStrat->parent->parent->pos;
			CamLookAtDirect(&p, pn);
			return;
		}


  	/*IS CAMERA SET TO TRACK THE PLAYER OR A POINT IN FRONT OF IT ?*/
   	if (player[pn].CameraStrat->parent->flag & STRAT_ENEMY) 
  	{

		//IF ENEMY TYPE, JUST POINT IN YOUR DIRECTION
  		dir.x = dir.z = 0;
  		dir.y = 100.0f;
  		mVector3Multiply (&POSOFF, &dir, &player[pn].CameraStrat->parent->m);
  		POSOFF.x += player[pn].CameraStrat->parent->pos.x;
  		POSOFF.y += player[pn].CameraStrat->parent->pos.y;
  		POSOFF.z += player[pn].CameraStrat->parent->pos.z;
		
		
		CamLookAtDirect(&POSOFF, pn);
  	}
	else  
   			CamLookAtDirect(&player[pn].Player->pos, pn);
}

static void findCamPos(Point3	*pos, Point3 *v, int pn)
{
	Vector3	dir, tempv;

	
	tempv.x = player[pn].Player->m.m[0][0];
	tempv.y = player[pn].Player->m.m[0][1];
	tempv.z = 0.0f;
	v3Normalise(&tempv);
	
	dir.x = v->x * tempv.x;
	dir.y = v->x * tempv.y;
	dir.z = v->x * tempv.z;

	tempv.x = player[pn].Player->m.m[1][0];
	tempv.y = player[pn].Player->m.m[1][1];
	tempv.z = 0.0f;
	v3Normalise(&tempv);

	dir.x += v->y * tempv.x;
	dir.y += v->y * tempv.y;
	dir.z += v->y * tempv.z;

	pos->x = player[pn].Player->pos.x + dir.x; 
	pos->y = player[pn].Player->pos.y + dir.y;
	pos->z = player[pn].Player->pos.z + dir.z;
}

static void findPosMXY(Point3 *pos, Point3 *v, Matrix *m)
{
	Vector3	dir, tempv;

	tempv.x = m->m[0][0];
	tempv.y = m->m[0][1];
	tempv.z = 0.0f;

	if (v3Dot(&tempv, &tempv) < 0.0001f)
		return;

	v3Normalise(&tempv);
	
	dir.x = v->x * tempv.x;
	dir.y = v->x * tempv.y;
	dir.z = v->x * tempv.z;

	tempv.x = m->m[1][0];
	tempv.y = m->m[1][1];
	tempv.z = 0.0f;
	v3Normalise(&tempv);

	dir.x += v->y * tempv.x;
	dir.y += v->y * tempv.y;
	dir.z += v->y * tempv.z;

	*pos = dir;
}

static void doCloseCam(int pn)
{
	Matrix m;
	Point3	tempPoint, tempv, lookAt;
	float f, zoomf;

	

	m = player[pn].Player->m;

	zoomf = (((player[pn].zooming + 0.6f) / 0.6f) * 0.8f) + 0.2f;
	if (player[pn].PlayerHeld)
	{
		mPreRotateZ(&m, -player[pn].smoothJoyX * player[pn].smoothJoyX * player[pn].smoothJoyX * 0.5f * zoomf);
	}

	if (player[pn].playerSkidDir)
		mPreRotateZ(&m, smoothSideStepSum(pn));
	else
		mPreRotateZ(&m, -smoothSideStepSum(pn));

	tempPoint.x = 0.0f;
	tempPoint.y = 0.0f;
	tempPoint.z = 0.0f;
	//if (player[0].PlayerSecondaryWeapon.type != LASER)
	{
		findPosMXY(&player[pn].camNewPos, &tempPoint, &m);
		v3Inc(&player[pn].camNewPos, &player[pn].Player->pos);
	}
	/*else
	{
		player[pn].camNewPos.x = player[pn].Player->objId[5]->wm.m[3][0];
		player[pn].camNewPos.y = player[pn].Player->objId[5]->wm.m[3][1];
		player[pn].camNewPos.z = player[pn].Player->objId[5]->wm.m[3][2];
		tempv.x = player[pn].Player->objId[5]->wm.m[1][0];
		tempv.y = player[pn].Player->objId[5]->wm.m[1][1];
		tempv.z = player[pn].Player->objId[5]->wm.m[1][2];
		v3Normalise(&tempv);
		v3Scale(&tempv, &tempv, 2.0f);
		v3Inc(&player[pn].camNewPos, &tempv);
	}*/

	f = player[pn].smoothJoyY;
	f *= f;
	f *= f;
	f *= zoomf;

	//if (player[0].PlayerSecondaryWeapon.type != LASER)
	{
		if (player[pn].smoothJoyY > 0.0f)
		{
			
			player[pn].camNewPos.z = player[pn].Player->pos.z + (LevelSettings->NormalCamlookHeight[THIRD_PERSON_CLOSE] + player[pn].smoothJoyY * zoomf);
		}
		else
		{
			f *= -1.0f;
			player[pn].camNewPos.z = player[pn].Player->pos.z + (LevelSettings->NormalCamlookHeight[THIRD_PERSON_CLOSE] - RANGE(-0.7f, player[pn].smoothJoyY * zoomf, 0.0f) * 1.5f);
		}
	}

	
	lookAt = player[pn].camNewPos;
	CamLookAt(&lookAt, pn);

	if (player[pn].PlayerAirTime)
	{
		player[pn].jumpCamXSpeed += player[pn].Player->m.m[0][0] * 0.04f;
		player[pn].jumpCamYSpeed += player[pn].Player->m.m[0][1] * 0.04f;
		player[pn].jumpCamZSpeed += 0.01f;
		player[pn].camNewPos.x = player[pn].CameraStrat->pos.x + player[pn].jumpCamXSpeed;
		player[pn].camNewPos.y = player[pn].CameraStrat->pos.y + player[pn].jumpCamYSpeed;
		player[pn].camNewPos.z = player[pn].CameraStrat->pos.z + player[pn].jumpCamZSpeed;
	}
	else
	{
		player[pn].jumpCamXSpeed = 0.0f;
		player[pn].jumpCamYSpeed = 0.0f;
		player[pn].jumpCamZSpeed = 0.0f;
		tempPoint.x = 0.0f;

		if (player[pn].smoothJoyY > 0.0f)
			tempPoint.y = LevelSettings->NormalCamDist[THIRD_PERSON_CLOSE] + (player[pn].smoothJoyY * 2.0f * zoomf);
		else
			tempPoint.y = LevelSettings->NormalCamDist[THIRD_PERSON_CLOSE] - RANGE(-0.7f, player[pn].smoothJoyY, 0.0f * zoomf) * 2.86f;
		tempPoint.z = 0.0f;

		if ((player[pn].Player->polyType & CAMERA) || 
			((player[pn].oldPolyType & CAMERA) && (GET_SURF_TYPE(player[pn].Player->polyType) == AIR)))
		{
			if (m.m[1][2] < 0.0f)
			{
				tempPoint.y -= player[pn].Player->m.m[1][2] * 3.0f;
			}
		}

		
		/*if (player[0].PlayerSecondaryWeapon.type == LASER)
		{
			player[pn].camNewPos.x = player[pn].Player->objId[5]->wm.m[3][0];
			player[pn].camNewPos.y = player[pn].Player->objId[5]->wm.m[3][1];
			player[pn].camNewPos.z = player[pn].Player->objId[5]->wm.m[3][2];
			tempv.x = player[pn].Player->objId[5]->wm.m[1][0];
			tempv.y = player[pn].Player->objId[5]->wm.m[1][1];
			tempv.z = player[pn].Player->objId[5]->wm.m[1][2];
			v3Normalise(&tempv);
			v3Scale(&tempv, &tempv, 2.0f);
			v3Dec(&player[pn].camNewPos, &tempv);
			player[pn].camNewPos.z += 1.0f;
		}
		else*/
		{
			findPosMXY(&player[pn].camNewPos, &tempPoint, &m);
			
			if ((player[pn].Player->polyType & CAMERA) || 
			((player[pn].oldPolyType & CAMERA) && (GET_SURF_TYPE(player[pn].Player->polyType) == AIR)))
			{
				player[pn].camNewPos.z = -m.m[1][2] * 8.0f * zoomf;
			}

			v3Inc(&player[pn].camNewPos, &player[pn].Player->pos);
			if (player[pn].smoothJoyY > 0.0f)
				player[pn].camNewPos.z += LevelSettings->NormalCamHeight[THIRD_PERSON_CLOSE] - (player[pn].smoothJoyY * 2.0f * zoomf);
			else
				player[pn].camNewPos.z += LevelSettings->NormalCamHeight[THIRD_PERSON_CLOSE] - (RANGE(-0.7f, player[pn].smoothJoyY * zoomf, 0.0f) * 3.6f);
		}


		/*if (PlayerState != PS_SIDESTEP)
		{
			tempv = Player->vel;
			v3Scale(&tempv, &tempv, 4.0f);
			v3Inc(&camNewPos, &tempv);
		}*/

		
		
		if (!((player[pn].CameraStrat) && (player[pn].CameraStrat->parent))) {
	
			player[pn].cam->screenAngle = GlobalFieldOfView + player[pn].zooming;

			if (wideAmt < 0.01f && !Multiplayer) {
				player[pn].cam->flags &= ~1;
				player[pn].cam->screenY = PHYS_SCR_Y;
			}
		} else
			TrackLookAt(&lookAt, pn);
			
		
	}

	findTarget(&lookAt, pn);
	player[pn].aimPos = lookAt;
//	if (!(player[pn].Player->flag & STRAT_UNDER_WATER))
		CamMoveTo(&player[pn].camNewPos, pn);
		
	player[pn].idleCamTilt = -(player[pn].Player->relVel.x * 0.3f) * CAM_TILT_DIR;
	player[pn].crntCamTilt += ((player[pn].idleCamTilt - player[pn].crntCamTilt) * 0.1f) * CAM_TILT_DIR;
	CameraSet (player[pn].cam, player[pn].crntCamTilt);
}


static void doCloseCamWaterSlide(int pn)
{
	Matrix m;
	Point3	tempPoint, tempv, lookAt;
	Vector3	v;
	float f;

	m = player[pn].Player->m;

	
	f = player[pn].smoothJoyY;
	f *= f;
	f *= f;

	//player[pn].camNewPos.z = player[pn].camLookCrntPos.z;
	
	
	
  	if (BossStrat)
  	{
  		lookAt.x = player[pn].camLookCrntPos.x;
  		lookAt.y = player[pn].camLookCrntPos.y;
  		lookAt.z = player[pn].camLookCrntPos.z;



  	}
  	else   
	{
	
		if (!WaterSlideCamLookStrat)
			return;
		
		tempPoint.x = 0.0f;
		tempPoint.y = 10.0f;
		tempPoint.z = 0.0f;


	  //	if (BossStrat)
	   //	{
	   //		tempPoint.z = (player[pn].Player->pos.z - BossStrat->pos.z);
	   //		tempPoint.y = (player[pn].Player->pos.y - BossStrat->pos.y);


	   //	}


		findPosMXY(&player[pn].camNewPos, &tempPoint, &m);
		v3Inc(&player[pn].camNewPos, &player[pn].Player->pos);

		v3Sub(&tempv, &WaterSlideCamLookStrat->pos, &WaterSlideCamLookStrat->parent->pos);
		tempv.z = 0.0f;
		v3Normalise(&tempv);
		v3Scale(&tempv, &tempv, 10.0f);
		v3Inc(&tempv, &player[pn].Player->pos);
		
		player[pn].camNewPos.x = tempv.x;
		player[pn].camNewPos.y = tempv.y;
		//player[pn].camNewPos.z += WaterSlideCamLookStrat->pos.z;

		if (player[pn].smooothJoyY > 0.0f)
		{
			player[pn].camNewPos.z += 0.5f + (player[pn].smooothJoyY) * 5.0f;
		}
		else
		{
			f *= -1.0f;
			player[pn].camNewPos.z += 0.5f + RANGE(-0.7f, player[pn].smooothJoyY, 0.0f) * 1.5f;
		}

		lookAt = player[pn].camNewPos;
		//lookAt.x = player[pn].camLookCrntPos.x + WaterSlideCamLookStrat->m.m[0][0] * 0.1f * player[0].playerOffsetX;
		//lookAt.y = player[pn].camLookCrntPos.y + WaterSlideCamLookStrat->m.m[0][1] * 0.1f * player[0].playerOffsetX;
		//lookAt.z = player[pn].camLookCrntPos.z + WaterSlideCamLookStrat->m.m[0][2] * 0.1f * player[0].playerOffsetX;
	}
	//lookAt.z = player[pn].camNewPos.z;
	CamLookAt(&lookAt, pn);

	player[pn].jumpCamXSpeed = 0.0f;
	player[pn].jumpCamYSpeed = 0.0f;
	player[pn].jumpCamZSpeed = 0.0f;
	tempPoint.x = 0.0f;

	if (player[pn].smooothJoyY > 0.0f)
		tempPoint.y = LevelSettings->NormalCamDist[THIRD_PERSON_CLOSE] + (player[pn].smooothJoyY * 2.0f);
	else
		tempPoint.y = LevelSettings->NormalCamDist[THIRD_PERSON_CLOSE] - RANGE(-0.7f, player[pn].smooothJoyY, 0.0f) * 2.86f;
	tempPoint.z = 0.0f;

	if ((player[pn].Player->polyType & CAMERA) || 
		((player[pn].oldPolyType & CAMERA) && (GET_SURF_TYPE(player[pn].Player->polyType) == AIR)))
	{
		if (m.m[1][2] < 0.0f)
		{
			tempPoint.y -= player[pn].Player->m.m[1][2] * 3.0f;
		}
	}

	mPoint3Multiply3(&player[pn].camNewPos, &tempPoint, &m);

	if ((player[pn].Player->polyType & CAMERA) || 
		((player[pn].oldPolyType & CAMERA) && (GET_SURF_TYPE(player[pn].Player->polyType) == AIR)))
	{
		player[pn].camNewPos.z += -m.m[1][2] * 8.0f;
	}

	v3Inc(&player[pn].camNewPos, &player[pn].Player->pos);

	
	if (player[pn].smooothJoyY > 0.0f)
		player[pn].camNewPos.z += 0.5f + LevelSettings->NormalCamHeight[THIRD_PERSON_CLOSE] - (player[pn].smooothJoyY * 2.0f);
	else
		player[pn].camNewPos.z += 0.5f + LevelSettings->NormalCamHeight[THIRD_PERSON_CLOSE] - (RANGE(-0.7f, player[pn].smooothJoyY, 0.0f) * 3.6f);
 
	
	if (!((player[pn].CameraStrat) && (player[pn].CameraStrat->parent))) {
		player[pn].cam->screenAngle = GlobalFieldOfView;
		if (wideAmt < 0.01f && !Multiplayer) {
			player[pn].cam->flags &= ~1;
			player[pn].cam->screenY = PHYS_SCR_Y;
		}
	} else
		TrackLookAt(&lookAt, pn);
		
	findTarget(&lookAt, pn);
	player[pn].aimPos = lookAt;

	v3Sub(&tempv, &player[pn].Player->pos, &player[pn].camLookCrntPos);
	v3Normalise(&tempv);
	v3Scale(&tempv, &tempv, 9.0f);
	v3Inc(&tempv, &player[pn].Player->pos);
	player[pn].camNewPos.x = tempv.x;
	player[pn].camNewPos.y = tempv.y;

	CamMoveTo(&player[pn].camNewPos, pn);
	player[pn].CameraStrat->pos = player[pn].camNewPos;
	CameraSet (player[pn].cam, 0.0f);
}


static void doTowerLiftCloseCam(int pn)
{
	Matrix m;
	Point3	tempPoint, tempv, lookAt;
	m = player[pn].Player->m;

	if (player[pn].playerSkidDir)
		mPreRotateZ(&m, smoothSideStepSum(pn));
	else
		mPreRotateZ(&m, -smoothSideStepSum(pn));

	tempPoint.x = 0.0f;
	tempPoint.y = 0.0f;
	tempPoint.z = 0.0f;
	findPosMXY(&player[pn].camNewPos, &tempPoint, &m);
	v3Inc(&player[pn].camNewPos, &player[pn].Player->pos);

	if (player[pn].smoothJoyY > 0.0f)
		player[pn].camNewPos.z = player[pn].Player->pos.z + LevelSettings->NormalCamlookHeight[THIRD_PERSON_CLOSE] + player[pn].smoothJoyY;
	else
		player[pn].camNewPos.z = player[pn].Player->pos.z + LevelSettings->NormalCamlookHeight[THIRD_PERSON_CLOSE] - RANGE(-0.7f, player[pn].smoothJoyY, 0.0f) * 1.5f;

	
	lookAt = player[pn].camNewPos;
	CamLookAt(&lookAt, pn);

	if (player[pn].PlayerAirTime)
	{
		player[pn].jumpCamXSpeed += player[pn].Player->m.m[0][0] * 0.04f;
		player[pn].jumpCamYSpeed += player[pn].Player->m.m[0][1] * 0.04f;
		player[pn].jumpCamZSpeed += 0.01f;
		player[pn].camNewPos.x = player[pn].CameraStrat->pos.x + player[pn].jumpCamXSpeed;
		player[pn].camNewPos.y = player[pn].CameraStrat->pos.y + player[pn].jumpCamYSpeed;
		player[pn].camNewPos.z = player[pn].CameraStrat->pos.z + player[pn].jumpCamZSpeed;
	}
	else
	{
		player[pn].jumpCamXSpeed = 0.0f;
		player[pn].jumpCamYSpeed = 0.0f;
		player[pn].jumpCamZSpeed = 0.0f;
		tempPoint.x = 0.0f;
		if (player[pn].smoothJoyY > 0.0f)
			tempPoint.y = LevelSettings->NormalCamDist[THIRD_PERSON_CLOSE] + (player[pn].smoothJoyY * 2.0f);
		else
			tempPoint.y = LevelSettings->NormalCamDist[THIRD_PERSON_CLOSE] - RANGE(-0.7f, player[pn].smoothJoyY, 0.0f) * 2.86f;
		tempPoint.z = 0.0f;

		if ((player[pn].Player->polyType & CAMERA) || 
			((player[pn].oldPolyType & CAMERA) && (GET_SURF_TYPE(player[pn].Player->polyType) == AIR)))
			if (player[pn].Player->m.m[1][2] < 0.0f)
				tempPoint.y -= player[pn].Player->m.m[1][2] * 3.0f;

		findPosMXY(&player[pn].camNewPos, &tempPoint, &m);
		if ((player[pn].Player->polyType & CAMERA) || 
			((player[pn].oldPolyType & CAMERA) && (GET_SURF_TYPE(player[pn].Player->polyType) == AIR)))
			player[pn].camNewPos.z = -player[pn].Player->m.m[1][2] * 8.0f;
		v3Inc(&player[pn].camNewPos, &player[pn].Player->pos);

		/*if (PlayerState != PS_SIDESTEP)
		{
			tempv = Player->vel;
			v3Scale(&tempv, &tempv, 4.0f);
			v3Inc(&camNewPos, &tempv);
		}*/

		if (player[pn].smoothJoyY > 0.0f)
			player[pn].camNewPos.z += LevelSettings->NormalCamHeight[THIRD_PERSON_CLOSE] - (player[pn].smoothJoyY * 2.0f);
		else
			player[pn].camNewPos.z += LevelSettings->NormalCamHeight[THIRD_PERSON_CLOSE] - (RANGE(-0.7f, player[pn].smoothJoyY, 0.0f) * 3.6f);
		
		if (!((player[pn].CameraStrat) && (player[pn].CameraStrat->parent)))
			player[pn].cam->screenAngle = GlobalFieldOfView;
		else
			TrackLookAt(&lookAt, pn);
			
		
	}
	findTarget(&lookAt, pn);
	player[pn].aimPos = lookAt;

	CamMoveTo(&player[pn].camNewPos, pn);
	CameraSet (player[pn].cam, 0.0f);
}

static void doTowerLiftCloseCam2(int pn)
{
	Matrix m;
	Point3	tempPoint, tempv, lookAt;
	m = player[pn].Player->m;

	if (player[pn].playerSkidDir)
		mPreRotateZ(&m, smoothSideStepSum(pn));
	else
		mPreRotateZ(&m, -smoothSideStepSum(pn));

	tempPoint.x = 0.0f;
	tempPoint.y = 0.0f;
	tempPoint.z = 0.0f;
	findPosMXY(&player[pn].camNewPos, &tempPoint, &m);
	v3Inc(&player[pn].camNewPos, &player[pn].Player->pos);

	if (player[pn].smoothJoyY > 0.0f)
		player[pn].camNewPos.z = player[pn].Player->pos.z + LevelSettings->NormalCamlookHeight[THIRD_PERSON_CLOSE] + player[pn].smoothJoyY;
	else
		player[pn].camNewPos.z = player[pn].Player->pos.z + LevelSettings->NormalCamlookHeight[THIRD_PERSON_CLOSE] - RANGE(-0.7f, player[pn].smoothJoyY, 0.0f) * 0.0f;

	
	lookAt = player[pn].camNewPos;
	TowerLiftCamLookAt(&lookAt, pn);

	if (player[pn].PlayerAirTime)
	{
		player[pn].jumpCamXSpeed += player[pn].Player->m.m[0][0] * 0.04f;
		player[pn].jumpCamYSpeed += player[pn].Player->m.m[0][1] * 0.04f;
		player[pn].jumpCamZSpeed += 0.01f;
		player[pn].camNewPos.x = player[pn].CameraStrat->pos.x + player[pn].jumpCamXSpeed;
		player[pn].camNewPos.y = player[pn].CameraStrat->pos.y + player[pn].jumpCamYSpeed;
		player[pn].camNewPos.z = player[pn].CameraStrat->pos.z + player[pn].jumpCamZSpeed;
	}
	else
	{
		player[pn].jumpCamXSpeed = 0.0f;
		player[pn].jumpCamYSpeed = 0.0f;
		player[pn].jumpCamZSpeed = 0.0f;
		tempPoint.x = 0.0f;
		if (player[pn].smoothJoyY > 0.0f)
			tempPoint.y = -3.0f + (player[pn].smoothJoyY * 1.0f);
		else
			tempPoint.y = -3.0f - RANGE(-0.7f, player[pn].smoothJoyY, 0.0f) * 1.5f;
		tempPoint.z = 0.0f;

		if ((player[pn].Player->polyType & CAMERA) || 
			((player[pn].oldPolyType & CAMERA) && (GET_SURF_TYPE(player[pn].Player->polyType) == AIR)))
			if (player[pn].Player->m.m[1][2] < 0.0f)
				tempPoint.y -= player[pn].Player->m.m[1][2] * 3.0f;

		findPosMXY(&player[pn].camNewPos, &tempPoint, &m);
		if ((player[pn].Player->polyType & CAMERA) || 
			((player[pn].oldPolyType & CAMERA) && (GET_SURF_TYPE(player[pn].Player->polyType) == AIR)))
			player[pn].camNewPos.z = -player[pn].Player->m.m[1][2] * 8.0f;
		v3Inc(&player[pn].camNewPos, &player[pn].Player->pos);

		/*if (PlayerState != PS_SIDESTEP)
		{
			tempv = Player->vel;
			v3Scale(&tempv, &tempv, 4.0f);
			v3Inc(&camNewPos, &tempv);
		}*/

		if (player[pn].smoothJoyY > 0.0f)
			player[pn].camNewPos.z += LevelSettings->NormalCamHeight[THIRD_PERSON_CLOSE];
		else
			player[pn].camNewPos.z += LevelSettings->NormalCamHeight[THIRD_PERSON_CLOSE] - (RANGE(-0.7f, player[pn].smoothJoyY, 0.0f) * 1.0f);
		
		if (!((player[pn].CameraStrat) && (player[pn].CameraStrat->parent)))
			player[pn].cam->screenAngle = GlobalFieldOfView;
		else
			TrackLookAt(&lookAt, pn);
			
		
	}
	findTarget(&lookAt, pn);
	player[pn].aimPos = lookAt;

	TowerLiftCamMoveTo(&player[pn].camNewPos, pn);
	CameraSet (player[pn].cam, 0.0f);
}

static void doCloseCamOnTrain(int pn)
{
	Matrix m;
	Point3	tempPoint, tempv, lookAt;
	float	smoothY;

	smoothY = player[pn].smoothJoyY;// * player[pn].smoothJoyY * player[pn].smoothJoyY;

	m = player[pn].Player->m;

	tempPoint.x = 0.0f;
	tempPoint.y = 0.0f;
	tempPoint.z = 0.0f;
	mPreRotateZ(&m, -player[pn].PlayerHeldCameraZAng*0.1f);
	findPosMXY(&player[pn].camNewPos, &tempPoint, &m);
	v3Inc(&player[pn].camNewPos, &player[pn].Player->pos);

	if (smoothY > 0.0f)
		player[pn].camNewPos.z = player[pn].Player->pos.z + LevelSettings->NormalCamlookHeight[THIRD_PERSON_CLOSE] + smoothY;
	else
		player[pn].camNewPos.z = player[pn].Player->pos.z + LevelSettings->NormalCamlookHeight[THIRD_PERSON_CLOSE] - RANGE(-0.7f, smoothY, 0.0f) * 1.5f;

	v3Inc(&player[pn].camNewPos, &player[pn].PlayerOldVel);
	lookAt = player[pn].camNewPos;

	tempPoint.x = 0.0f;
	if (smoothY > 0.0f)
		tempPoint.y = LevelSettings->NormalCamDist[THIRD_PERSON_CLOSE] + (smoothY * 2.0f);
	else
		tempPoint.y = LevelSettings->NormalCamDist[THIRD_PERSON_CLOSE] - RANGE(-0.7f, smoothY, 0.0f) * 2.86f;
	tempPoint.z = 0.0f;

	findPosMXY(&player[pn].camNewPos, &tempPoint, &m);
	player[pn].camNewPos.z = -player[pn].Player->m.m[1][2] * 8.0f;
	v3Inc(&player[pn].camNewPos, &player[pn].Player->pos);

	if (smoothY > 0.0f)
		player[pn].camNewPos.z += LevelSettings->NormalCamHeight[THIRD_PERSON_CLOSE] - (smoothY * 2.0f);
	else
		player[pn].camNewPos.z += LevelSettings->NormalCamHeight[THIRD_PERSON_CLOSE] - (RANGE(-0.7f, smoothY, 0.0f) * 3.6f);
	
	
  	CamLookAt(&lookAt, pn);
	findTarget(&lookAt, pn);
	player[pn].aimPos = lookAt;

	tempv = player[pn].PlayerOldVel;
	v3Scale(&tempv, &tempv, 2.0f);
	v3Inc(&player[pn].camNewPos, &tempv);

	v3Sub(&tempv, &player[pn].camNewPos, &player[pn].CameraStrat->pos);
	v3Scale(&tempv, &tempv, 0.8f);
	v3Inc(&player[pn].CameraStrat->pos, &tempv);
	CameraSet (player[pn].cam, 0.0f);
}

static void doFirstCam(int pn)
{
	Matrix m;
	Point3	tempPoint, tempv, lookAt;

	m = player[pn].Player->m;

	if (player[pn].playerSkidDir)
		mPreRotateZ(&m, smoothSideStepSum(pn));
	else
		mPreRotateZ(&m, -smoothSideStepSum(pn));

	tempPoint.x = 0.0f;
	tempPoint.y = 10.0f;
	tempPoint.z = 0.0f;
	findPosMXY(&player[pn].camNewPos, &tempPoint, &m);		
	v3Inc(&player[pn].camNewPos, &player[pn].Player->pos);
	player[pn].camNewPos.z = player[pn].Player->pos.z + player[pn].Player->m.m[1][2] * 10.0f + player[pn].smooothJoyY * 5.0f;
	lookAt = player[pn].camNewPos;
	player[pn].camNewPos = player[pn].Player->pos;
	player[pn].camNewPos.z += 0.5f;
	/*player[pn].camNewPos.x -= player[0].Player->m.m[1][0];
	player[pn].camNewPos.y -= player[0].Player->m.m[1][1];
	player[pn].camNewPos.z -= player[0].Player->m.m[1][2];*/
	CamLookAt(&lookAt, pn);
	
	if (!((player[pn].CameraStrat) && (player[pn].CameraStrat->parent)))
		player[pn].cam->screenAngle = GlobalFieldOfView + player[pn].zooming;
	else
		TrackLookAt(&lookAt, pn);
		
	findTarget(&lookAt, pn);
	player[pn].aimPos = lookAt;


	if (player[pn].camNewPos.z < player[pn].Player->pos.z)
  		player[pn].camNewPos.z = player[pn].Player->pos.z;
  	
	
	CamMoveTo(&player[pn].camNewPos, pn);

	player[pn].idleCamTilt = -(player[pn].Player->relVel.x * 0.3f) * CAM_TILT_DIR;
	player[pn].crntCamTilt += ((player[pn].idleCamTilt - player[pn].crntCamTilt) * 0.1f) * CAM_TILT_DIR;
	CameraSet (player[pn].cam, player[pn].rollAng + (player[pn].crntCamTilt));
}

static void doStaticCam(int pn)
{
	CamLookAt(&player[pn].Player->pos, pn);
	CameraSet (player[pn].cam, 0.0f);
}

static void doHeldCam(int pn)
{
   //	CameraSet (player[pn].cam, player[pn].crntCamTilt * CAM_TILT_DIR);
 	CameraSet (player[pn].cam, 0);
 	if (player[pn].cam->HoldTime)
		player[pn].cam->HoldTime--;
	else
	{
		player[pn].cameraPos = THIRD_PERSON_CLOSE; 
	  	player[pn].cam->screenAngle = GlobalFieldOfView;

	}	
}



static void doCamera(int pn)
{
	Point3	tempPoint, lookAt;
	Vector3	tempv;
	Matrix m;

	
	if (player[pn].CameraStrat != NULL)
		player[pn].cam->pos = player[pn].CameraStrat->pos;

	switch (player[pn].cameraPos)
	{
		case THIRD_PERSON_CLOSE:					
			doCloseCam(pn);
			break;
		case FIRST_PERSON:
			doFirstCam(pn);
			break;
		case STATIC_PERSON:
			doStaticCam(pn);
			break;
		case CAMERA_HELD:
			doHeldCam(pn);
			break;
	}
}
#if 0
void doCameraShield(int pn)
{
	Point3	tempPoint;
	
	if (player[pn].CameraStrat != NULL)
		player[pn].cam->pos = player[pn].CameraStrat->pos;

	
	tempPoint.x = 0.0f;
	tempPoint.y = 0.0f;
	tempPoint.z = 0.0f;
	findCamLookAt(&player[pn].camNewPos, &tempPoint, pn);		
	player[pn].camNewPos.z = player[pn].Player->pos.z;
  	CamLookAt(&player[pn].camNewPos, pn);
	
	if (!((player[pn].CameraStrat) && (player[pn].CameraStrat->parent)))
		player[pn].cam->screenAngle = GlobalFieldOfView;
	else
		TrackLookAt(&player[pn].camNewPos, pn);
		
	findTarget(&player[pn].camNewPos, pn);
	player[pn].aimPos = player[pn].camNewPos;


	tempPoint.x = 0.0f; 
	tempPoint.y = 5.0f;
	tempPoint.z = - 10.0f;
	findCamPos(&player[pn].camNewPos, &tempPoint, pn);

	if (player[pn].camNewPos.z < player[pn].Player->pos.z)
  		player[pn].camNewPos.z = player[pn].Player->pos.z;

  	player[pn].camNewPos.z += player[pn].CamPosOff.z;
	CamMoveTo(&player[pn].camNewPos, pn);
	CameraSet (player[pn].cam, 0.0f);
	player[pn].Player->objId[5]->idleRot.z *= 0.5f;
	player[pn].Player->objId[5]->crntRot.z = player[pn].Player->objId[5]->idleRot.z;
}
#endif





static void doCameraPlayerHeld(int pn)
{
	Point3	tempPoint;
	Vector3	tempv, gunV, playerV, crossV;
	Matrix	m;
	float tf;

	tf = 1.0f;

	player[pn].CameraStrat->flag &= LNOT(STRAT_COLLFLOOR);
	if (player[pn].PlayerState == PS_ONTRAIN)
	{
		gunV.x = player[pn].Player->objId[5]->wm.m[1][0];
		gunV.y = player[pn].Player->objId[5]->wm.m[1][1];
		gunV.z = 0.0f;
		v3Normalise(&gunV);
		playerV.x = player[pn].Player->m.m[1][0];
		playerV.y = player[pn].Player->m.m[1][1];
		playerV.z = 0.0f;
		v3Normalise(&playerV);
		if (v3Dot(&gunV, &playerV) < 0.707f)
		{
			v3Cross(&crossV, &gunV, &playerV);
			if (player[pn].smoothJoyX < 0.0f)
			{
				if (crossV.z < 0.0f)
					tf = 0.0f;
			}
			else
			{
				if (crossV.z > 0.0f)
					tf = 0.0f;
			}
		}

		player[pn].PlayerHeldCameraZAng += player[pn].smoothJoyX * player[pn].smoothJoyX * player[pn].smoothJoyX * tf;
	}

	if (player[pn].CameraStrat != NULL)
		player[pn].cam->pos = player[pn].CameraStrat->pos;

	

	if(player[pn].PlayerState & PS_TOWERLIFT)
		player[pn].PlayerHeldCameraZAng *= 0.9f;

	m = player[pn].Player->m;
	

	switch(player[pn].cameraPos)
	{
		case FIRST_PERSON:
			switch(player[pn].PlayerState)
			{
				case PS_TOWERLIFT:
					doFirstCam(pn);
					break;
				case PS_WATERSLIDE:
					doFirstCam(pn);
					break;
				case PS_ONTRAIN:
			   //	case PS_INDROPSHIP:
					doFirstCam(pn);
					break;
				case PS_NORMAL:
					doFirstCam(pn);
					break;
				default:
					doFirstCam(pn);
					break;
			}
			break;
		case THIRD_PERSON_CLOSE:
			switch(player[pn].PlayerState)
			{
				case PS_TOWERLIFT:
					doTowerLiftCloseCam(pn);
					break;
				case PS_WATERSLIDE:
					doCloseCamWaterSlide(pn);
					break;
				case PS_ONTRAIN:
					doCloseCamOnTrain(pn);
					break;
				case PS_INDROPSHIP:
				case PS_NORMAL:
				case PS_ONPATH:
					doCloseCam(pn);
					break;
				default:
					doCloseCam(pn);
					break;
			}
			break;
	}
	
	/*if (!((player[pn].CameraStrat) && (player[pn].CameraStrat->parent)))
		player[pn].cam->screenAngle = GlobalFieldOfView;
	else
		TrackLookAt(&player[pn].camNewPos, pn);*/
		
	findTarget(&player[pn].camNewPos, pn);
	player[pn].aimPos = player[pn].camNewPos;


	CameraSet (player[pn].cam, 0.0f);
}


void addToTargetArray(STRAT *strat, Object *obj, int pn)
{
	int	i;

	if (strat->flag == STRAT_DEAD)
		return;

	for (i=0; i<MAX_TARGETS; i++)
	{
		if ((player[pn].target[i].strat == strat) && (player[pn].target[i].obj == obj))
		{
			if (player[pn].target[i].locked)
				return;

			player[pn].target[i].n++;
			player[pn].target[i].locked = 1;
			return;
		}
	}

	/* Make a bleeping noise */
	PlaySound(player[pn].Player, 0, GENERIC_LOCKON_SOUND, 0.8, 0, 0, 0, 
		SBFLAG_NOREVERB|SBFLAG_NODOPPLER|SBFLAG_NODOPPLER);


	for (i=0; i<MAX_TARGETS; i++)
	{
		if (player[pn].target[i].strat == NULL)
		{
			player[pn].target[i].strat = strat;
			player[pn].target[i].obj = obj;
			player[pn].target[i].time = 0.0f;
			player[pn].target[i].n = 0;
			player[pn].target[i].locked = 1;
			return;
		}
	}
}


void clearTargetArray(int pn)
{
	int	i;

	
	for (i=0; i<MAX_TARGETS; i++)
	{
		player[pn].target[i].strat = NULL;
		player[pn].target[i].obj = NULL;
		player[pn].target[i].n = 0;
		player[pn].target[i].locked = 0;
	}
}


void DrawTarget(Point3 *p, float time)
{
	Point2	destP, currP, startP, tempP;
	int	i;
	
#define LOCKON_TIME 0.3f
#define INV_LOCKON_TIME (1.0f / 0.3f)
	
	mPush(&mCurMatrix);
	mUnit(&mCurMatrix);
	
	if (point3ToScreen(p, &tempP))
	{
		if (time >= LOCKON_TIME)
		{
			destP.x = tempP.x - 20.0f * X_SCALE;
			destP.y = tempP.y - 20.0f;
			rSprite2D (destP.x, destP.y, targetTLSpriteHolder->obj);
			destP.x = tempP.x + 20.0f * X_SCALE;
			destP.y = tempP.y - 20.0f;
			rSprite2D (destP.x, destP.y, targetTRSpriteHolder->obj);
			destP.x = tempP.x + 20.0f * X_SCALE;
			destP.y = tempP.y + 20.0f;
			rSprite2D (destP.x, destP.y, targetBRSpriteHolder->obj);
			destP.x = tempP.x - 20.0f * X_SCALE;
			destP.y = tempP.y + 20.0f;
			rSprite2D (destP.x, destP.y, targetBLSpriteHolder->obj);
		}
		else	
		{
			startP.x = 0.0f;
			startP.y = 0.0f;
			destP.x = tempP.x - 20.0f * X_SCALE;
			destP.y = tempP.y - 20.0f;
			currP.x = startP.x + (destP.x - startP.x) * (time * INV_LOCKON_TIME);
			currP.y = startP.y + (destP.y - startP.y) * (time * INV_LOCKON_TIME);
			rSprite2D (currP.x, currP.y, targetTLSpriteHolder->obj);	
			startP.x = PHYS_SCR_X;
			startP.y = 0.0f;
			destP.x = tempP.x + 20.0f * X_SCALE;
			destP.y = tempP.y - 20.0f;
			currP.x = startP.x + (destP.x - startP.x) * (time * INV_LOCKON_TIME);
			currP.y = startP.y + (destP.y - startP.y) * (time * INV_LOCKON_TIME);
			rSprite2D (currP.x, currP.y, targetTRSpriteHolder->obj);	
			startP.x = PHYS_SCR_X;
			startP.y = PHYS_SCR_Y;
			destP.x = tempP.x + 20.0f * X_SCALE;
			destP.y = tempP.y + 20.0f;
			currP.x = startP.x + (destP.x - startP.x) * (time * INV_LOCKON_TIME);
			currP.y = startP.y + (destP.y - startP.y) * (time * INV_LOCKON_TIME);
			rSprite2D (currP.x, currP.y, targetBRSpriteHolder->obj);	
			startP.x = 0.0f;
			startP.y = PHYS_SCR_Y;
			destP.x = tempP.x - 20.0f * X_SCALE;
			destP.y = tempP.y + 20.0f;
			currP.x = startP.x + (destP.x - startP.x) * (time * INV_LOCKON_TIME);
			currP.y = startP.y + (destP.y - startP.y) * (time * INV_LOCKON_TIME);
			rSprite2D (currP.x, currP.y, targetBLSpriteHolder->obj);	
		}
	}
	mPop(&mCurMatrix);
}

void DrawTargetSpin(Point3 *p, float time)
{
	Point2	destP, currP, startP, tempP;
	float	dist;
	int	i;
	
#define LOCKON_TIME 0.3f
#define INV_LOCKON_TIME (1.0f / 0.3f)
	
	mPush(&mCurMatrix);
	mUnit(&mCurMatrix);
	
#define ARROW_ROT	0.0f
#define ARROW_SPEED 15.0f

	if (point3ToScreen(p, &tempP))
	{
		dist = 30.0 * fabs(sin(time * ARROW_SPEED));
		destP.x = tempP.x - (20.0f + dist);
		destP.y = tempP.y - (20.0f + dist);
		rSprite2DMtl (destP.x, destP.y, &textureShootMe, 1.0f, ARROW_ROT * time * 10.0f);
		destP.x = tempP.x + (20.0f + dist);
		destP.y = tempP.y - (20.0f + dist);
		rSprite2DMtl (destP.x, destP.y, &textureShootMe, 1.0f, ARROW_ROT * time * 10.0f + HPI);
		destP.x = tempP.x + (20.0f + dist);
		destP.y = tempP.y + (20.0f + dist);
		rSprite2DMtl (destP.x, destP.y, &textureShootMe, 1.0f, ARROW_ROT * time * 10.0f + PI);
		destP.x = tempP.x - (20.0f + dist);
		destP.y = tempP.y + (20.0f + dist);
		rSprite2DMtl (destP.x, destP.y, &textureShootMe, 1.0f, ARROW_ROT * time * 10.0f + PI + HPI);
		//destP.x = tempP.x * X_SCALE;
		//destP.y = tempP.y;
		//rSprite2DMtl (destP.x, destP.y, &textureShootMe, 1.0f, ARROW_ROT * time * 10.0f);
	}
	mPop(&mCurMatrix);
}



static void DrawPrimaryTarget(STRAT *strat, float time)
{
	Point2	destP, currP, startP, tempP;
	Colour	col;
	Uint32  colour = 0x8fffffff;
	int	i;

	#define LOCKON_TIME 0.3f
	#define INV_LOCKON_TIME (1.0f / 0.3f)

	mPush(&mCurMatrix);
	mUnit(&mCurMatrix);


	if (point3ToScreen(&strat->pos, &tempP))
	{
		if (time >= LOCKON_TIME)
		{
	/*		destP.x = tempP.x - 20.0f * X_SCALE;
		destP.y = tempP.y - 20.0f;
		rSprite2D (destP.x, destP.y, targetTLSpriteHolder->obj);
		destP.x = tempP.x + 20.0f * X_SCALE;
		destP.y = tempP.y - 20.0f;
		rSprite2D (destP.x, destP.y, targetTRSpriteHolder->obj);
		destP.x = tempP.x + 20.0f * X_SCALE;
		destP.y = tempP.y + 20.0f;
		rSprite2D (destP.x, destP.y, targetBRSpriteHolder->obj);
		destP.x = tempP.x - 20.0f * X_SCALE;
		destP.y = tempP.y + 20.0f;
		rSprite2D (destP.x, destP.y, targetBLSpriteHolder->obj); */
		destP.x = tempP.x;
		destP.y = tempP.y;
	   	rSprite2D (destP.x, destP.y, strat->obj);

		}
		else	
		{
		/*	startP.x = 0.0f;
		startP.y = 0.0f;
		destP.x = tempP.x - 20.0f * X_SCALE;
		destP.y = tempP.y - 20.0f;
		currP.x = startP.x + (destP.x - startP.x) * (time * INV_LOCKON_TIME);
		currP.y = startP.y + (destP.y - startP.y) * (time * INV_LOCKON_TIME);
		rSprite2D (currP.x, currP.y, targetTLSpriteHolder->obj);	
		startP.x = PHYS_SCR_X;
		startP.y = 0.0f;
		destP.x = tempP.x + 20.0f * X_SCALE;
		destP.y = tempP.y - 20.0f;
		currP.x = startP.x + (destP.x - startP.x) * (time * INV_LOCKON_TIME);
		currP.y = startP.y + (destP.y - startP.y) * (time * INV_LOCKON_TIME);
		rSprite2D (currP.x, currP.y, targetTRSpriteHolder->obj);	
		startP.x = PHYS_SCR_X;
		startP.y = PHYS_SCR_Y;
		destP.x = tempP.x + 20.0f * X_SCALE;
		destP.y = tempP.y + 20.0f;
		currP.x = startP.x + (destP.x - startP.x) * (time * INV_LOCKON_TIME);
		currP.y = startP.y + (destP.y - startP.y) * (time * INV_LOCKON_TIME);
		rSprite2D (currP.x, currP.y, targetBRSpriteHolder->obj);	
		startP.x = 0.0f;
		startP.y = PHYS_SCR_Y;
		destP.x = tempP.x - 20.0f * X_SCALE;
		destP.y = tempP.y + 20.0f;
		currP.x = startP.x + (destP.x - startP.x) * (time * INV_LOCKON_TIME);
		currP.y = startP.y + (destP.y - startP.y) * (time * INV_LOCKON_TIME);
		rSprite2D (currP.x, currP.y, targetBLSpriteHolder->obj);   */	
			startP.x = 0.0f;
			startP.y = PHYS_SCR_Y;
			destP.x = tempP.x * X_SCALE;
			destP.y = tempP.y ;
			currP.x = startP.x + (destP.x - startP.x) * (time * INV_LOCKON_TIME);
			currP.y = startP.y + (destP.y - startP.y) * (time * INV_LOCKON_TIME);
			destP.x = tempP.x;
			destP.y = tempP.y;

		//	rSprite2D (currP.x, currP.y, strat->obj);
		 	rSprite2D (tempP.x, tempP.y, strat->obj);
		}
	}
	col.argb.a = (colour&0xff000000)>>24;
	col.argb.b = (colour&0x00ff0000)>>16;
	col.argb.g = (colour&0x0000ff00)>>8;
	col.argb.r = (colour&0x000000ff);
	mPush(&mCurMatrix);
	mUnit(&mCurMatrix);
  //	rLineOnTop(&strat->pos, &strat->parent->pos, col, col);
   //	DrawLine(&strat->pos,&strat->parent->pos,0xffffffff);
		mPop(&mCurMatrix);

	mPop(&mCurMatrix);
}


static int	findObjId(STRAT *strat, Object *obj)
{
	dAssert((int)obj > 0x80000000, "crap obj");
	return OBJECT_GET_ID(obj->collFlag);
}

static void addParentMatrix(Matrix *m, Object *obj)
{
	if (obj->parent)
		addParentMatrix(m, obj->parent);

	mPreTranslate(m, obj->crntPos.x, obj->crntPos.y, obj->crntPos.z);
	mPreRotateXYZ(m, obj->crntRot.x, obj->crntRot.y, obj->crntRot.z);
	mPreScale(m, obj->scale.x, obj->scale.y, obj->scale.z);
	
}

static void DrawTargets(int pn)
{
	int		i, objId;
	Point3	p,temp;

	for (i=0; i<MAX_TARGETS; i++)
	{
		if (player[pn].target[i].strat != NULL)
		{
		   			
			/* Make a bleeping noise */
			if (player[pn].target[i].time < LOCKON_TIME &&
				(player[pn].target[i].time + 0.01667f) > LOCKON_TIME)
				PlaySound(player[pn].Player, 0, GENERIC_LOCKON_SOUND, 0.8, 0, 0, 0, 
					SBFLAG_NOREVERB||SBFLAG_NODOPPLER|SBFLAG_NODOPPLER);
			player[pn].target[i].time += 0.01667f;
			
			//if ((!(player[pn].target[i].strat->flag & COLL_TARGETABLE)) && lineLandscapeCollision(&player[pn].target[i].strat->pos, &player[pn].Player->pos, &temp))
			//	continue;
			//CHECK TO SEE IF LINE INTERCEPTS LANDSCAPE FROM PLAYER TO TARGET
		  	if (frameCount % 30 == 0)
			{
				if (lineLandscapeCollision(&player[pn].CameraStrat->pos, &player[pn].target[i].strat->pos, &temp, player[pn].target[i].strat, NULL))
				{
					//INTERCEPTION SO REMOVE IF IT'S A WEAPON TARGETABLE WAREZ
					if (player[pn].target[i].strat->flag & COLL_TARGETABLE)
						RemoveTarget(player[pn].target[i].strat, pn, -1);
				}
		  
			}
			else 
			{
			//OTHERWISE WE DRAW THE CHAPPY
			//FIRST SEE WHETHER IT'S A LOCKED ON BY PLAYER TARGET
				if ((player[pn].target[i].strat->flag & COLL_TARGETABLE) || (player[pn].target[i].obj->collFlag & COLL_TARGETABLE))
				{
					if (player[pn].target[i].obj == NULL)
						DrawTarget(&player[pn].target[i].strat->pos, player[pn].target[i].time);
					else
					{
						objId = findObjId(player[pn].target[i].strat, player[pn].target[i].obj);
						if (objId)
						{
							//findObjectWorldPosition(&p, player[pn].target[i].strat, objId);				
							p.x = player[pn].target[i].strat->objId[objId]->wm.m[3][0];
							p.y = player[pn].target[i].strat->objId[objId]->wm.m[3][1];
							p.z = player[pn].target[i].strat->objId[objId]->wm.m[3][2];
							DrawTarget(&p, player[pn].target[i].time);
						}
						else
						{
							DrawTarget(&player[pn].target[i].strat->pos, player[pn].target[i].time);
						}

					}
				}
				else
				{

					//if not a primary target type, then let's remove
				  //	if (!(player[pn].target[i].strat->flag & STRAT_PRIMETARGET))
						RemoveTarget(player[pn].target[i].strat, pn, -1);
				   //	else
				   //	{
				   //		if (player[pn].target[i].strat->pnode->anim)
				   //			rUpdateAnims(player[pn].target[i].strat->pnode,&player[pn].target[i].strat->anim->anim,player[pn].target[i].strat->obj);

				   //		DrawPrimaryTarget(player[pn].target[i].strat, player[pn].target[i].time);
				   //	}
				}
			}
		}
	}
	/*for (i=0; i<noSwarmTargets; i++)
	{
		if (swarmStratTargetArray[i])
		{
			if (swarmObjectTargetArray[i])
			{
				findObjectWorldPosition(&p, swarmStratTargetArray[i], swarmObjectTargetArray[i]);		
					DrawTarget(&p, 0.3f);
			}
			else
			{
					DrawTarget(&swarmStratTargetArray[i]->pos, 0.3f);
			}
		}
	}*/
}

static void RemoveTarget(STRAT *strat, int pn, int targetNo)
{
	int	i;

	if (targetNo == -1)
	{
		for (i=0; i<MAX_TARGETS; i++)
		{
			if (player[pn].target[i].strat == strat)
			{
				player[pn].target[i].strat = NULL;
				player[pn].target[i].obj = NULL;
				player[pn].target[i].n = 0;
			}
		}
	}
	else
	{

		if (player[pn].target[targetNo].n)
			player[pn].target[targetNo].n--;
		else
		{
			player[pn].target[targetNo].strat = NULL;
			player[pn].target[targetNo].obj = NULL;
		}	
	}
}

static void RemoveTargetStrat(STRAT *strat, int pn, int sn, int id)
{
	int	i;
	STRAT *s;

	s = (STRAT *)sn;

	if (Multiplayer)
	{
		if (player[pn].playerTargetNumber == s->Player->n)
		{
			player[pn].playerTargetNumber = -1;
			player[pn].playerTargetTime = 0;
		}
	}

	for (i=0; i<MAX_TARGETS; i++)
	{
		if (id)
		{
			if ((s == player[pn].target[i].strat) && 
				(player[pn].target[i].obj == player[pn].target[i].strat->objId[id]))
			{
				player[pn].target[i].strat = NULL;
				player[pn].target[i].obj = NULL;
				player[pn].target[i].n = 0;
				break;
			}
		}
		else
		{
			if (s == player[pn].target[i].strat)
			{
				player[pn].target[i].strat = NULL;
				player[pn].target[i].obj = NULL;
				player[pn].target[i].n = 0;
				break;
			}
		}
	}
}



static void drawAimTarget(int pn)
{
	Point2 p;

	mPush(&mCurMatrix);
	mUnit(&mCurMatrix);


	point3ToScreen(&player[pn].aimPos, &p);

	rAim(p.x, p.y, player[pn].aimType, pn);
	mPop(&mCurMatrix);
}


/*post sh4 change*/
void doTarget(int pn)
{
	Object *obj = NULL;
	int		objId;
	Point3	p, ps, pe, from;
	Vector3	targetDirection, tempv;
	float lambda;

	if (!player[pn].CameraStrat || !player[pn].Player ||
		player[pn].Player->var == 1234.f)
		return;

	ps = player[pn].CameraStrat->pos;
	v3Sub(&targetDirection, &player[pn].aimPos, &player[pn].CameraStrat->pos);
	pe.x = ps.x + targetDirection.x;
	pe.y = ps.y + targetDirection.y;
	pe.z = ps.z + targetDirection.z;
	player[pn].aimedPoint = player[pn].aimPos;

	obj = NULL;
	player[pn].aimStrat = NULL;
	player[pn].aimObj = NULL;
	if (player[pn].Player->pnode)
	{
		player[pn].aimStrat = targetStratCollision(&obj, pn);

		if (obj)
			player[pn].aimObj = obj;
		
		if (player[pn].aimStrat)
		{

			if (!player[pn].TargetList)
			{
				player[pn].TargetList = player[pn].aimStrat;
			}


			if (player[pn].aimStrat->pnode)
			{
				if (!player[pn].aimStrat->hdBlock)
				{
					objId = findObjId(player[pn].aimStrat, player[pn].aimObj);
					if (objId)
					{
						p.x = player[pn].aimStrat->objId[objId]->wm.m[3][0];
						p.y = player[pn].aimStrat->objId[objId]->wm.m[3][1];
						p.z = player[pn].aimStrat->objId[objId]->wm.m[3][2];
						//findObjectWorldPosition(&p, player[pn].aimStrat, objId);
						v3Dec(&p, &player[pn].CameraStrat->pos);
						lambda = v3Length(&p) / v3Length(&targetDirection);
						player[pn].aimedPoint.x = ps.x + lambda * targetDirection.x;
						player[pn].aimedPoint.y = ps.y + lambda * targetDirection.y;
						player[pn].aimedPoint.z = ps.z + lambda * targetDirection.z;
					}
					else
					{
						v3Sub(&p, &player[pn].aimStrat->pos, &player[pn].CameraStrat->pos);
						lambda = v3Length(&p) / v3Length(&targetDirection);
						player[pn].aimedPoint.x = ps.x + lambda * targetDirection.x;
						player[pn].aimedPoint.y = ps.y + lambda * targetDirection.y;
						player[pn].aimedPoint.z = ps.z + lambda * targetDirection.z;
					}
				}
			}
		}

		v3Normalise(&targetDirection);
		v3Scale(&targetDirection, &targetDirection, 5.0f);
		v3Add(&from, &player[pn].CameraStrat->pos, &targetDirection);
		
	 	if (player[pn].aimStrat)
		{
			if ((fabs(player[pn].aimedPoint.x - from.x) > 0.01f) && 
				(fabs(player[pn].aimedPoint.y - from.y) > 0.01f))
				if (lineLandscapeCollisionA(pn, &from, &player[pn].aimedPoint, &player[pn].aimedPoint, player[pn].aimStrat, &player[pn].aimedNormal, &player[pn].aimedPolyType))
					player[pn].aimStrat = NULL;
		}
		else
		{
			if ((fabs(player[pn].aimPos.x - from.x) > 0.01f) && 
				(fabs(player[pn].aimPos.y - from.y) > 0.01f))
				if (lineLandscapeCollisionA(pn, &from, &player[pn].aimPos, &player[pn].aimedPoint, NULL, &player[pn].aimedNormal, &player[pn].aimedPolyType))
					player[pn].aimStrat = NULL;		
		}
 

		if (player[pn].Player)
			if (player[pn].rdhm)
				if (GetSem(player[pn].rdhm, 0) > 0)
					player[pn].targetting = 0;

		if (player[pn].aimStrat != player[pn].Player)
		{
			if (player[pn].aimStrat)
			{
				if ((player[pn].targetting == 1) || (player[pn].targetting == 3))
				{
					// Multiplayer change: can't target a cloaked player
					if (Multiplayer && player[pn].aimStrat->Player && player[pn].aimStrat->Player->cloaked != 0) {
						// Don't lock on
					} else {
						if ((player[pn].aimStrat->flag & COLL_TARGETABLE) && (player[pn].aimStrat->health > 0.0f))
							addToTargetArray(player[pn].aimStrat, NULL, pn);
						else if(player[pn].aimObj && (player[pn].aimObj->collFlag & COLL_TARGETABLE) && (player[pn].aimObj->health > 0.0f))
							addToTargetArray(player[pn].aimStrat, player[pn].aimObj, pn);
					}
				}
			}
		}
		
		mPush(&mCurMatrix);
		mUnit (&mCurMatrix);

		if (!(player[pn].PlayerState & PS_INDROPSHIP))
		{
			if (!player[pn].CameraStrat->parent)
			{
				DrawTargets(pn);
				drawAimTarget(pn);
			}
		}
		mPop(&mCurMatrix);
	}


	if (player[pn].Player->pnode)
	{
		if ((fabs(player[pn].aimPos.x - player[pn].CameraStrat->pos.x) > 0.1f) &&
			(fabs(player[pn].aimPos.y - player[pn].CameraStrat->pos.y) > 0.1f))
			TowardTargetPosXZ(player[pn].Player,5,1.0f,&player[pn].aimedPoint);
		else
			TowardTargetPosXZ(player[pn].Player,5,1.0f,&player[pn].aimPos);

		if (player[pn].currentMainGun != 5 && !Multiplayer)
		{
			if ((fabs(player[pn].aimPos.x - player[pn].CameraStrat->pos.x) > 0.1f) &&
				(fabs(player[pn].aimPos.y - player[pn].CameraStrat->pos.y) > 0.1f))
				TowardTargetPosXZ(player[pn].Player,player[pn].currentMainGun,1.0f,&player[pn].aimedPoint);
			else
				TowardTargetPosXZ(player[pn].Player,player[pn].currentMainGun,1.0f,&player[pn].aimPos);
		}

	}
}

void doCam(int pn)
{
	if (!player[pn].CameraStrat) {
		if (Multiplayer)
			CameraSet (player[pn].cam, 0);
		return;
	}
	
	if (!player[pn].cam->HoldTime)
   	{
		if ((pSqrDist(&player[pn].Player->pos, &player[pn].CameraStrat->pos) > 400.0f) || (fabs(player[pn].Player->pos.z - player[pn].CameraStrat->pos.z) > 7.0f))
		{
			player[pn].CameraStrat->flag &= ~STRAT_COLLFLOOR;
		}	
		else if (pSqrDist(&player[pn].Player->pos, &player[pn].CameraStrat->pos) < 100.0f)
		{
			if (player[pn].Player->flag)
			{
				if (!(player[pn].CameraStrat->flag & STRAT_COLLFLOOR))
				{
					player[pn].CameraStrat->flag |= STRAT_COLLFLOOR;
					ResetOCP(player[pn].CameraStrat);
				}
			}
		}


		/*if (player[pn].PlayerState == PS_ONPATH || player[pn].cameraPos == STATIC_PERSON) 
		{
			wideAmt += 0.05f;
		} 
		else 
		{
			wideAmt -= 0.05f;
		}
		*/


		if (!Multiplayer) 
		{
			if (wideAmt < 0.01f) 
			{
				wideAmt = 0;
				player[pn].cam->flags &= ~1;
				player[pn].cam->screenY = PHYS_SCR_Y;
			} 
			else 
			{
				player[pn].cam->flags |= 1;
				player[pn].cam->screenY = PHYS_SCR_Y - (PHYS_SCR_Y - ((9 * PHYS_SCR_Y)/16)) * Delta (wideAmt);
				if (wideAmt > 1)
					wideAmt = 1;
			}
		}
		//if (player[pn].PlayerHeld)
		//	doCameraPlayerHeld(pn);
		//else
			doCamera(pn);
	} 
	else
		doHeldCam(pn);
}

int	LastSplineSection(STRAT *strat)
{
	int numpathpoints;
	ROUTE *route = strat->route;

	dAssert(strat->route,"strat must have a path");
	dAssert(strat->route->splineData,"You forgot to do initSplineData");

	if (strat->route->splineData)
	{
		numpathpoints = route->path->numwaypoints;
		
		if (strat->route->splineData->lineSeg + 2 == numpathpoints)
			return 1;
		else
			return 0;
	}
}

int	FirstSplineSection(STRAT *strat)
{
	int numpathpoints;
	ROUTE *route = strat->route;

	dAssert(strat->route,"strat must have a path");
	dAssert(strat->route->splineData,"You forgot to do initSplineData");

	if (strat->route->splineData)
	{
		numpathpoints = route->path->numwaypoints;
		
		if (strat->route->splineData->lineSeg == 0)
			return 1;
		else
			return 0;
	}
}



float distToLastWay(STRAT *strat)
{
	ROUTE *route = strat->route;
	Point3	p;

	p = route->path->waypointlist[route->path->numwaypoints - 1];

	return pDist(&strat->pos, &p);
}

float DistToLastWayXY(STRAT *strat, int mode)
{
	ROUTE *route = strat->route;
	Point3	p1, p2;

	p1 = route->path->waypointlist[route->path->numwaypoints - 1];
	p1.z = 0.0f;
	p2 = strat->pos;
	p2.z = 0.0f;

	if (mode)
   		//return the root value
		return pSqrDist(&p1, &p2);
	else
		return pDist(&p1, &p2);
}


float distToFirstWay(STRAT *strat)
{
	ROUTE *route = strat->route;
	Point3	p;

	p = route->path->waypointlist[0];

	return pDist(&strat->pos, &p);
}

int	StratInPlayerCone(STRAT *strat, int pn, float ang)
{
	Vector3	playerV, playerToStratV;

	playerV.x = player[pn].Player->m.m[1][0];
	playerV.y = player[pn].Player->m.m[1][1];
	playerV.z = player[pn].Player->m.m[1][2];

	v3Sub(&playerToStratV, &strat->pos, &player[pn].Player->pos);
	v3Normalise(&playerToStratV);
	if (acos(v3Dot(&playerV, &playerToStratV)) < ang)
		return 1;
	else
		return 0;
}


