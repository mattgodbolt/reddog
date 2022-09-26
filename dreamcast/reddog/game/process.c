#include "RedDog.h"
#include "strat.h"
#include "draw.h"
#include "coll.h"
#include "command.h"
#include "procspec.h"
#include "process.h"
#include "camera.h"
#include "player.h"



#define RDMK				1
#define GROUND_VEL_DAMP		0.85f
#define GROUND_ROT_DAMP		0.9f
#define AIR_VEL_DAMP		0.85f
#define AIR_ROT_DAMP		0.95f
#define WATER_ROT_DAMP		0.95f
#define WATER_VEL_DAMP		0.98f
#define FRICTION_COEFF_X	0.2f 
#define FRICTION_COEFF_Y	0.292f
#define FRICTION_COEFF_Z	0.292f

extern void collision(STRAT *strat);

extern PLAYER player[4];


//float Gravity = 0.0109f;
float Gravity = NTSC_GRAVITY;
float TGravity = 1.0f;
//int	wasOnBoat = 0;

void mNormalize(Matrix *m)
{
	register float one_over_dist, *read;
	register float x1, y1, z1, x2, y2, z2, x3, y3, z3;

	// Righty-ho!  MattG gets in here and does his usual annoying trick of rewriting bits...sorry!
	read = &m->m[0][0];
	x1 = *read++; 
	y1 = *read++; 
	z1 = *read; 
	read += 2;

	x2 = *read++; 
	y2 = *read++; 
	z2 = *read; 
	read += 2;

	x3 = y1*z2 - z1*y2;
	y3 = z1*x2 - x1*z2;
	z3 = x1*y2 - y1*x2;

	x2 = y3*z1 - z3*y1;
	y2 = z3*x1 - x3*z1;
	z2 = x3*y1 - y3*x1;

	*read++ = x3;
	*read++ = y3;
	*read = z3;
	read = &m->m[1][0];
	*read++ = x2;
	*read++ = y2;
	*read = z2;

	if ((x1 != 0.0f) || (y1 != 0.0f) || (z1 != 0.0f))
	{
		
		one_over_dist = isqrt (x1*x1 + y1*y1 + z1*z1);

		read = &m->m[0][0];
		*read++ = x1 * one_over_dist;
		*read++ = y1 * one_over_dist;
		*read   = z1 * one_over_dist;
		read += 2;
		one_over_dist = isqrt (x2*x2 + y2*y2 + z2*z2);

		*read++ = x2 * one_over_dist;
		*read++ = y2 * one_over_dist;
		*read   = z2 * one_over_dist;
		read += 2;

		one_over_dist = isqrt (x3*x3 + y3*y3 + z3*z3);
		
		*read++ = x3 * one_over_dist;
		*read++ = y3 * one_over_dist;
		*read   = z3 * one_over_dist;
	}
	else
	{
		mUnit(m);
		return;
	}
}

static void Friction(STRAT *strat)
{
	Vector3	xdir, ydir, zdir, relVel;
	Matrix m, im;
	int pn;
	float temp;
	//if ((strat->relVel.x == 0.0f) && (strat->relVel.y == 0.0f) && (strat->relVel.z == 0.0f))
		//return;

	if (strat == NULL)
		return;

	if (!((strat->friction.x != 1.0f) ||
		(strat->friction.y != 1.0f) ||
		(strat->friction.z != 1.0f)))
		return;

	m = strat->m;
	mInvertRot(&m, &im);


	dAssert(im.m[0][0] == im.m[0][0], "qnan");
	dAssert(m.m[0][0] == m.m[0][0], "qnan");

	
	mVector3Multiply(&relVel, &strat->vel, &im);
	if (strat->Player)
	{
		pn = GetPlayerNumber(strat);
		if (player[pn].Player->flag & COLL_ON_FLOOR)
		{	
			if (PAL)
			{
				temp = strat->friction.x + (1.0f - strat->friction.x) * (1.0f - player[pn].DoPlayerFriction);
				temp = 1.0f - ((1.0f - temp) * PAL_MOVESCALE);
				if (temp > 0.0f)
					relVel.x *= temp;
				else
					relVel.x = 0.0f;
				temp = strat->friction.y + (1.0f - strat->friction.y) * (1.0f - player[pn].DoPlayerFriction);
				temp = 1.0f - ((1.0f - temp) * PAL_MOVESCALE);
				if (temp > 0.0f)
					relVel.y *= temp;
				else
					relVel.y = 0.0f;
				temp = strat->friction.z + (1.0f - strat->friction.z) * (1.0f - player[pn].DoPlayerFriction);
				temp = 1.0f - ((1.0f - temp) * PAL_MOVESCALE);
				if (temp > 0.0f)
					relVel.z *= temp;
				else
					relVel.z = 0.0f;

				/*temp = 1.0 - ((1.0 - (strat->friction.x + (1.0f - strat->friction.x) * (1.0f - player[pn].DoPlayerFriction))) * PAL_ACCSCALE);
		   		relVel.x *= temp;
				temp = 1.0 - ((1.0 - (strat->friction.y + (1.0f - strat->friction.y) * (1.0f - player[pn].DoPlayerFriction))) * PAL_ACCSCALE);
				relVel.y *= temp;
				temp = 1.0 - ((1.0 - (strat->friction.z + (1.0f - strat->friction.z) * (1.0f - player[pn].DoPlayerFriction))) * PAL_ACCSCALE);
				relVel.z *= temp;*/
			}
			else
			{
				relVel.x *= strat->friction.x + (1.0f - strat->friction.x) * (1.0f - player[pn].DoPlayerFriction);
				relVel.y *= strat->friction.y + (1.0f - strat->friction.y) * (1.0f - player[pn].DoPlayerFriction);
				relVel.z *= strat->friction.z + (1.0f - strat->friction.z) * (1.0f - player[pn].DoPlayerFriction);
			}
			mVector3Multiply(&strat->vel, &relVel, &m);
		}
	}
	else if(strat == BoatStrat)
	{
		if (BoatStrat->objId[1]->collFlag & COLL_ON_WATER)
		{	
			if (PAL)
			{
		   		relVel.x *= (strat->friction.x + (1.0 - strat->friction.x) * (1.0 - (player[0].DoPlayerFriction * PAL_ACCSCALE))) ;
				relVel.y *= (strat->friction.y + (1.0 - strat->friction.y) * (1.0 - (player[0].DoPlayerFriction * PAL_ACCSCALE))) ;
				relVel.z *= (strat->friction.z + (1.0 - strat->friction.z) * (1.0 - (player[0].DoPlayerFriction * PAL_ACCSCALE))) ;
			}
			else
			{
				relVel.x *= strat->friction.x + (1.0f - strat->friction.x) * (1.0f - player[0].DoPlayerFriction);
				relVel.y *= strat->friction.y + (1.0f - strat->friction.y) * (1.0f - player[0].DoPlayerFriction);
				relVel.z *= strat->friction.z + (1.0f - strat->friction.z) * (1.0f - player[0].DoPlayerFriction);
			}
			mVector3Multiply(&strat->vel, &relVel, &m);
		}
	}
	else
	{
	    relVel.x *= (strat->friction.x);
		relVel.y *= (strat->friction.y);
		relVel.z *= (strat->friction.z);
		mVector3Multiply(&strat->vel, &relVel, &m);
	}

}


static void moveStrat(STRAT *strat)
{
	Vector3	temp;
	int pn;

	Friction(strat);

	dAssert(strat->vel.x == strat->vel.x, "qnan");
	v3Inc(&strat->pos, &strat->vel);
 
	if (strat->Player)
	{
		pn = GetPlayerNumber(strat);
		if (strat->flag & COLL_ON_FLOOR)
		{
			if (GET_SURF_TYPE(strat->polyType) != ICE)
			{
				if (PAL)
				{
					strat->vel.x *= 0.98f * (1 / PAL_ACCSCALE);
					strat->vel.y *= 0.98f * (1 / PAL_ACCSCALE);
					strat->vel.z *= 0.95f * (1 / PAL_ACCSCALE);
				}
				else
				{ 
					strat->vel.x *= 0.98f;
					strat->vel.y *= 0.98f;
					strat->vel.z *= 0.95f;
				}
			}
			else
			{
				temp.x = 0.99f;
				temp.y = 0.99f;
				temp.z = 1.0f;
				v3Mul(&strat->vel, &strat->vel, &temp);
			}

			if (PAL)
				strat->rot_speed *= 0.88f;
			else
				strat->rot_speed *= 0.9f;

			if (strat->m.m[2][2] < 0.6f)
			{
				if (strat->rot_speed >= 0.0f)
				{
					if (strat->rot_speed < 0.01f)
						strat->rot_speed = 0.01f;
				}
				else
				{
					if (strat->rot_speed > -0.01f)
						strat->rot_speed = -0.01f;
				}
			}
		}
		else
		{
				if (PAL)
					strat->rot_speed *= 0.952f;
				else
					strat->rot_speed *= 0.96f;

		}
	}
	else if (strat == BoatStrat)
	{
		if (PAL)
		{
			strat->vel.x *= 0.98f * (1 / PAL_ACCSCALE);
			strat->vel.y *= 0.98f * (1 / PAL_ACCSCALE) ;
			strat->vel.z *= 0.95f * (1 / PAL_ACCSCALE);
		}
		else
		{
			strat->vel.x *= 0.98f;
			strat->vel.y *= 0.98f;
			strat->vel.z *= 0.95f;
		}
	  	strat->rot_speed *= 0.9f;
		if (strat->m.m[2][2] < 0.6f)
		{
			if (strat->rot_speed >= 0.0f)
			{
				if (strat->rot_speed < 0.01f)
					strat->rot_speed = 0.01f;
			}
			else
			{
				if (strat->rot_speed > -0.01f)
					strat->rot_speed = -0.01f;
			}
		}
	}
	else
	{
			strat->rot_speed *= 0.9f;
	}

	strat->flag2 &= ~STRAT2_MATRIX;
}


void calcRelVel(STRAT *crntStrat)
{
	float		rdForwardSpeed, rdLeftSpeed, rdUpSpeed;
	Vector3	    rdForward, rdLeft, rdUp, rdVel, v, ov;

	v3Sub(&rdVel, &crntStrat->pos, &crntStrat->oldPos);
	/*if (crntStrat->Player)
	{
		if (crntStrat->Player->PlayerOnHoldPlayer)
		{
			v3Sub(&v, &crntStrat->Player->PlayerOnHoldPlayer->strat->oldPos, &crntStrat->Player->PlayerOnHoldPlayer->strat->oldOldPos);
			v3Dec(&rdVel, &v);
		}
	}*/
	dAssert(crntStrat->relVel.x == crntStrat->relVel.x, "qnan");
	v = rdVel;
	//rdVel = crntStrat->oldVel;

	if ((rdVel.x == 0.0f) && (rdVel.y == 0.0f) && (rdVel.z == 0.0f))
	{
		crntStrat->relVel.x = 0.0f;
		crntStrat->relVel.y = 0.0f;
		crntStrat->relVel.z = 0.0f;
		return;
	}

	rdLeft.x = crntStrat->m.m[0][0];
	rdLeft.y = crntStrat->m.m[0][1];
	rdLeft.z = crntStrat->m.m[0][2];
	rdForward.x = crntStrat->m.m[1][0];
	rdForward.y = crntStrat->m.m[1][1];
	rdForward.z = crntStrat->m.m[1][2];
	rdUp.x = crntStrat->m.m[2][0];
	rdUp.y = crntStrat->m.m[2][1];
	rdUp.z = crntStrat->m.m[2][2];
	v3Normalise( &rdVel);
	rdLeftSpeed = v3Length( &v) * v3Dot( &rdLeft, &rdVel);
	rdForwardSpeed = v3Length( &v) * v3Dot( &rdForward, &rdVel);
	rdUpSpeed = v3Length( &v) * v3Dot( &rdUp, &rdVel);
	crntStrat->relVel.x = rdLeftSpeed;
	crntStrat->relVel.y = rdForwardSpeed;
	crntStrat->relVel.z = rdUpSpeed;
	dAssert(crntStrat->relVel.x == crntStrat->relVel.x, "qnan");
}

static void makeRotMatrix(Matrix *mat, Vector3 *v, float speed)
{
	float	s, c, t, sx, sy, sz, txy, tyz, txz;

	if (PAL)
		speed *= PAL_MOVESCALE;

	if (((float)sqrt(v3Dot(v, v)) < 0.01f) && (speed < 0.0001f))
	{
		mUnit(mat);
		return;
	}
	speed *= -1.0f;

	s = (float)sin(speed);
	c = (float)cos(speed);
	t = 1.0f - c;

	txy = t * v->x;
	txz = txy * v->z;
	txy = txy * v->y;
	tyz = t * (v->y * v->z);

	sx = s * v->x;
	sy = s * v->y;
	sz = s * v->z;

	mUnit(mat);
	mat->m[0][0] =  t * (float)sqrt(fabs(v->x)) + c;
	mat->m[1][0] =  txy + sz;
	mat->m[2][0] =  txz - sy;

	mat->m[0][1] =  txy - sz;
	mat->m[1][1] =  t * (float)sqrt(fabs(v->y)) + c;
	mat->m[2][1] =  tyz + sx;

	mat->m[0][2] =  txz + sy;
	mat->m[1][2] =  tyz - sx;
	mat->m[2][2] =  t * (float)sqrt(fabs(v->z)) + c;
}

static void newStratMatrix(STRAT *crntStrat)
{
	Matrix	tempm1;
	
	makeRotMatrix(&tempm1, &crntStrat->rot_vect, crntStrat->rot_speed);
	mPostMultiply(&crntStrat->m, &tempm1);	
}

#if 0
//THE VALUES OF AIRFRICTION/GROUNDFRICTION ARE MODIFIED FOR PAL, IN PNODELOAD (OBJECT.C)
void applyFriction(STRAT *crntStrat)
{
	Vector3	force, centre;
	float	mass;

	if (!crntStrat->pnode) 
		return;

	centre.x = centre.y = centre.z = 0.0f;
	mass = crntStrat->pnode->mass;
	/*centre.z = -5.0f; */
	
	if ((crntStrat->flag & STRAT_COLLFLOOR) && (crntStrat->flag & COLL_ON_FLOOR))
	{
		if (crntStrat->relVel.x != 0.0f)
		{				     
			force.x = crntStrat->m.m[0][0];
			force.y = crntStrat->m.m[0][1];
			force.z = crntStrat->m.m[0][2];
			force.x *= -mass * crntStrat->relVel.x * crntStrat->pnode->groundFriction.x * FRICTION_COEFF_X;
			force.y *= -mass * crntStrat->relVel.x * crntStrat->pnode->groundFriction.x * FRICTION_COEFF_X;
			force.z *= -mass * crntStrat->relVel.x * crntStrat->pnode->groundFriction.x * FRICTION_COEFF_X;
			
				ApplyForce(crntStrat, &force, &centre, 1.0f);
		}
		if (crntStrat->relVel.y != 0.0f)
		{	
			force.x = crntStrat->m.m[1][0];
			force.y = crntStrat->m.m[1][1];
			force.z = crntStrat->m.m[1][2];	     
			force.x *= -mass * crntStrat->relVel.y * crntStrat->pnode->groundFriction.y * FRICTION_COEFF_Y;
			force.y *= -mass * crntStrat->relVel.y * crntStrat->pnode->groundFriction.y * FRICTION_COEFF_Y;
			force.z *= -mass * crntStrat->relVel.y * crntStrat->pnode->groundFriction.y * FRICTION_COEFF_Y;
			/*if (obj->collFlag & FLAG_ON_FLOOR) */
				ApplyForce(crntStrat, &force, &centre, 1.0f);
		}
		if (crntStrat->relVel.z != 0.0f)
		{				     
			force.x = crntStrat->m.m[2][0];
			force.y = crntStrat->m.m[2][1];
			force.z = crntStrat->m.m[2][2];
			force.x *= -mass * crntStrat->relVel.z * crntStrat->pnode->groundFriction.z * FRICTION_COEFF_Z;
			force.y *= -mass * crntStrat->relVel.z * crntStrat->pnode->groundFriction.z * FRICTION_COEFF_Z;
			force.z *= -mass * crntStrat->relVel.z * crntStrat->pnode->groundFriction.z * FRICTION_COEFF_Z;
			/*if (obj->collFlag & FLAG_ON_FLOOR) */
				ApplyForce(crntStrat, &force, &centre, 1.0f);
		}
	}
	else if (!(crntStrat->flag & COLL_ON_FLOOR))
	{
		if (crntStrat->relVel.x != 0.0f)
		{				     
			force.x = crntStrat->m.m[0][0];
			force.y = crntStrat->m.m[0][1];
			force.z = crntStrat->m.m[0][2];
			force.x *= -mass * crntStrat->relVel.x * crntStrat->pnode->airFriction.x * FRICTION_COEFF_X;
			force.y *= -mass * crntStrat->relVel.x * crntStrat->pnode->airFriction.x * FRICTION_COEFF_X;
			force.z *= -mass * crntStrat->relVel.x * crntStrat->pnode->airFriction.x * FRICTION_COEFF_X;
			
				ApplyForce(crntStrat, &force, &centre, 1.0f);
		}
		if (crntStrat->relVel.y != 0.0f)
		{	
			force.x = crntStrat->m.m[1][0];
			force.y = crntStrat->m.m[1][1];
			force.z = crntStrat->m.m[1][2];	     
			force.x *= -mass * crntStrat->relVel.y * crntStrat->pnode->airFriction.y * FRICTION_COEFF_Y;
			force.y *= -mass * crntStrat->relVel.y * crntStrat->pnode->airFriction.y * FRICTION_COEFF_Y;
			force.z *= -mass * crntStrat->relVel.y * crntStrat->pnode->airFriction.y * FRICTION_COEFF_Y;
			/*if (obj->collFlag & FLAG_ON_FLOOR) */
				ApplyForce(crntStrat, &force, &centre, 1.0f);
		}
		if (crntStrat->relVel.z != 0.0f)
		{				     
			force.x = crntStrat->m.m[2][0];
			force.y = crntStrat->m.m[2][1];
			force.z = crntStrat->m.m[2][2];
			force.x *= -mass * crntStrat->relVel.z * crntStrat->pnode->airFriction.z * FRICTION_COEFF_Z;
			force.y *= -mass * crntStrat->relVel.z * crntStrat->pnode->airFriction.z * FRICTION_COEFF_Z;
			force.z *= -mass * crntStrat->relVel.z * crntStrat->pnode->airFriction.z * FRICTION_COEFF_Z;
			/*if (obj->collFlag & FLAG_ON_FLOOR) */
				ApplyForce(crntStrat, &force, &centre, 1.0f);
		}
	}
}
#endif

void applyGravity(STRAT *crntStrat)
{
	Vector3	gravity, origin, p, tempp;
	float mass;
 
	if (!crntStrat->pnode)
		return;
	p = crntStrat->pnode->com;

	mass = crntStrat->pnode->mass;
	p.x *= crntStrat->scale.x;
	p.y *= crntStrat->scale.y;
	p.z *= crntStrat->scale.z;
	
	if (crntStrat->pnode)
	{
		gravity.x = 0.0f;
		gravity.y = 0.0f;
		mPoint3Multiply3(&origin, &p, &crntStrat->m);
		/*DrawMarker(origin.x + Player->pos.x, origin.y + Player->pos.y , origin.z + Player->pos.z);*/
		//dAssert((crntStrat->flag & STRAT_GRAVITYON) && 
		if (crntStrat->flag & STRAT_GRAVITYON)
		{
			if (crntStrat->Player)
			{
				if (GET_SURF_TYPE(crntStrat->polyType) == SPIDERMAN)
				{
					if (PAL)
						tempp.z = -2.0f * Gravity * mass * (crntStrat->Player->ThonGravity * PAL_ACCSCALE) * TGravity;
				   	else
						tempp.z = -2.0f * Gravity * mass * crntStrat->Player->ThonGravity * TGravity;
					
					
					tempp.x = tempp.y = 0.0f;
					mPoint3Multiply3(&gravity, &tempp, &crntStrat->m);
				}
				else
					gravity.z = -2.0f * Gravity * mass * crntStrat->Player->ThonGravity * TGravity;
			}
			else
			{
				gravity.z = -2.0f * Gravity * mass * TGravity;
			}
		}
		else if (crntStrat->flag & STRAT_REALGRAVITYON)
		{
			if (crntStrat->Player)
				gravity.z = -Gravity * mass * crntStrat->Player->ThonGravity * TGravity;
			else
				gravity.z = -Gravity * mass * TGravity;
		}
		else
			return;

		//if (PAL)
		//	v3Scale(&gravity,&gravity,PAL_SCALE);
		ApplyForce(crntStrat, &gravity, &origin, crntStrat->pnode->collForceRatio);
	}
}

static void doStratSpecificProcess(STRAT *crntStrat)
{
	void	(*specProc)(STRAT *crntStrat);

	if (crntStrat->flag & STRAT_EXPLODEPIECE)
	{
		specProc = processSpecProc[0];
		specProc(crntStrat);
	}
	else if (crntStrat->pnode)
	{
		specProc = processSpecProc[crntStrat->pnode->typeId];
		specProc(crntStrat);
	}
}


void ProcessStrat(STRAT *crntStrat)
{
	/*PROCESS ANY NON-STOPPED/NON-SPRITE STRATS*/
//	if ((!(crntStrat->flag & STRAT_SPRITE)) && (!(crntStrat->flag2 & STRAT2_PROCESSSTOPPED)))
//	if ((!(crntStrat->flag & STRAT_SPRITE)))
	{



#if COLLISION
		if ((crntStrat->flag & STRAT_GRAVITYON)  || (crntStrat->flag & STRAT_REALGRAVITYON))
			applyGravity(crntStrat);
#endif
		if (!(crntStrat->flag & STRAT_SPRITE))
			newStratMatrix(crntStrat);
	   
		//takes out any scale nonsense left over
	   	mNormalize(&crntStrat->m);

		dAssert (crntStrat->vel.x == crntStrat->vel.x, "QNAN");
		
		//if (crntStrat->flag & (STRAT_COLLFLOOR | STRAT_COLLSTRAT | STRAT_HITFLOOR))
		if (crntStrat->flag & (STRAT_COLLFLOOR | STRAT_COLLSTRAT))
			collision(crntStrat);

		dAssert (crntStrat->vel.x == crntStrat->vel.x, "QNAN");
		doStratSpecificProcess(crntStrat);
		dAssert (crntStrat->vel.x == crntStrat->vel.x, "QNAN");
	} 
}

void MoveAllStrats(void)
{
	STRAT *strat = FirstStrat;

	while (strat)
	{
		if (strat->flag)
		{
			if (strat->flag & STRAT_CALCRELVEL)
				calcRelVel(strat);	

			strat->flag2 &= ~STRAT2_MATRIX;	
			strat->oldOldPos = strat->oldPos;
			strat->oldPos = strat->pos;

			if ((strat->flag & STRAT_BULLET) && (strat->flag & COLL_HITSTRAT) && (GetSem(strat, 0) != 1))
			{
			}
			else
			{
				
				moveStrat(strat);

				if (strat == BoatStrat)
				{
					if (player[0].onBoat == 1)
					{
//						if (wasOnBoat == 0)
//						{
//							wasOnBoat = 1;
//						}
						
						HoldPlayerPos(strat, 0, 0.0f, 0.0f, 0.8f);
						HoldPlayerRot(strat, 0, 0.0f, 0.0f, 0.0f);
						player[0].Player->rot_speed = 0.0f;
						player[0].Player->vel.x = 0.0f;
						player[0].Player->vel.y = 0.0f;
						player[0].Player->vel.z = 0.0f;
					}
				}
				
			}
	   	}
		strat = strat->next;
	}
}

float PlayerDistPlayerCheckXY(void)
{
	Point3	p1, p2;

	p1 = player[currentPlayer].Player->pos;
	p2 = player[currentPlayer].Player->CheckPos;
	p1.z = 0.0f;
	p2.z = 0.0f;

	return pDist(&p1, &p2);
}

float GetObjectCrntPos(STRAT *strat, int objId, int mode)
{
	float res;
	switch (mode)
	{
		case 0:
			res =  strat->objId[objId]->crntPos.x;
			break;
	   	case 1:
			res =  strat->objId[objId]->crntPos.y;
			break;
		case 2:
			res =  strat->objId[objId]->crntPos.z;
		default:
			break;
	}
	return(res);
}

float GetObjectCrntScale(STRAT *strat, int objId, int mode)
{
	float res;
	switch (mode)
	{
		case 0:
			res =  strat->objId[objId]->scale.x;
			break;
	   	case 1:
			res =  strat->objId[objId]->scale.y;
			break;
		case 2:
			res =  strat->objId[objId]->scale.z;
		default:
			break;
	}
	return(res);
}


//gets the modifier to apply to an object's scale
float GetObjectAnimScale(STRAT *strat, int objId, int mode)
{
	float res;
	switch (mode)
	{
		case 0:
			res =  strat->objId[objId]->animscale.x;
			break;
	   	case 1:
			res =  strat->objId[objId]->animscale.y;
			break;
		case 2:
			res =  strat->objId[objId]->animscale.z;
		default:
			break;
	}
	return(res);
}




void SetObjectCrntPos(STRAT *strat, int objId, float amount, int mode)
{
	switch (mode)
	{
		case 0:
			strat->objId[objId]->crntPos.x = amount;
			break;
	   	case 1:
			strat->objId[objId]->crntPos.y = amount;
			break;
		case 2:
			strat->objId[objId]->crntPos.z = amount;
		default:
			break;
	}
}

void SetObjectCrntRot(STRAT *strat, int objId, float amount, int mode)
{
	switch (mode)
	{
		case 0:
			strat->objId[objId]->crntRot.x = amount;
			break;
	   	case 1:
			strat->objId[objId]->crntRot.y = amount;
			break;
		case 2:
			strat->objId[objId]->crntRot.z = amount;
		default:
			break;
	}
	strat->objId[objId]->collFlag |= OBJECT_STRATMOVE;
}

void SetObjectIdleRot(STRAT *strat, int objId, float amount, int mode)
{
	switch (mode)
	{
		case 0:
			strat->objId[objId]->idleRot.x = amount;
			break;
	   	case 1:
			strat->objId[objId]->idleRot.y = amount;
			break;
		case 2:
			strat->objId[objId]->idleRot.z = amount;
		default:
			break;
	}
	strat->objId[objId]->collFlag |= OBJECT_STRATMOVE;
}






float GetObjectIdlePos(STRAT *strat, int objId, int mode)
{
	float res;
	switch (mode)
	{
		case 0:
			res =  strat->objId[objId]->idlePos.x;
			break;
	   	case 1:
			res =  strat->objId[objId]->idlePos.y;
			break;
		case 2:
			res =  strat->objId[objId]->idlePos.z;
		default:
			break;
	}
	return(res);
}

float GetObjectCrntRot(STRAT *strat, int objId, int mode)
{
	float res;
	switch (mode)
	{
		case 0:
			res =  strat->objId[objId]->crntRot.x;
			break;
	   	case 1:
			res =  strat->objId[objId]->crntRot.y;
			break;
		case 2:
			res =  strat->objId[objId]->crntRot.z;
		default:
			break;
	}
	return(res);
}

float GetObjectIdleRot(STRAT *strat, int objId, int mode)
{
	float res;
	switch (mode)
	{
		case 0:
			res =  strat->objId[objId]->idleRot.x;
			break;
	   	case 1:
			res =  strat->objId[objId]->idleRot.y;
			break;
		case 2:
			res =  strat->objId[objId]->idleRot.z;
		default:
			break;
	}
	return(res);
}

static void ResetObjOCP(Object *obj)
{
	int i;

	for (i=0; i<obj->ncp; i++)
		mPoint3Multiply3(&obj->ocpt[i], &obj->cp[i].centre, &obj->wm);

	for (i=0; i<obj->no_child; i++)
		ResetObjOCP(&obj->child[i]);
}

void ResetOCP(STRAT *strat)
{
	objectToWorld(strat);
	ResetObjOCP(strat->obj);
	strat->oldPos = strat->pos;
	strat->oldOldPos = strat->oldPos;
}
