#include "RedDog.h"
#include "draw.h"
#include "coll.h"
#include "camera.h"
#include "view.h"
#include "command.h"
#include "stratphysic.h"

#if DRAWSPLINE
	extern	void DrawSplinePath(struct waypath *path);
#endif

 

HoldPlayerBlock *hpbFirst = NULL;
HoldPlayerBlock *hpbLast = NULL;
STRAT *WaterSlideCamLookStrat = NULL;

StratList *firstAvoidStrat, *lastAvoidStrat;

#define HIGH_SOLUTION	0
#define LOW_SOLUTION	1
extern void moveTrailsBack(float amount);
extern	float	moveTrailsDistance;
Vector3 HoldPlayerV;

void Flatten(STRAT *crntStrat, float amount)
{
	Vector3	force, stratUp, oldVel;
	Point3	p;

	if (PAL)
		amount *= PAL_MOVESCALE;

	stratUp.x = crntStrat->m.m[2][0];
	stratUp.y = crntStrat->m.m[2][1];
	stratUp.z = crntStrat->m.m[2][2];
	v3Normalise(&stratUp);
	p.x = stratUp.x * crntStrat->pnode->radius * 2.0f;
	p.y = stratUp.y * crntStrat->pnode->radius * 2.0f;
	p.z = stratUp.z * crntStrat->pnode->radius * 2.0f;
	force.x = force.y = 0.0f;
	force.z = crntStrat->pnode->mass * 0.01f * amount;
	oldVel = crntStrat->vel;
	ApplyForce(crntStrat, &force, &p, 0.1f);
	crntStrat->vel = oldVel;
}

void DirectionPitch(STRAT *crntStrat, float amount)
{
	Vector3	force, stratUp, vec;
	Point3	p;
	float ang;
						 
#if 0
	stratUp.x = crntStrat->m.m[2][0];
	stratUp.y = crntStrat->m.m[2][1];
	stratUp.z = crntStrat->m.m[2][2];
   //	stratUp.z = 0;
	v3Normalise(&stratUp);

 	p.x = crntStrat->pos.x - player[currentPlayer].Player->pos.x;
	p.y = crntStrat->pos.y - player[currentPlayer].Player->pos.y;
	p.z = crntStrat->pos.z - player[currentPlayer].Player->pos.z;


	v3Normalise(&p);
	
	ang = acos(v3Dot(&stratUp,&p));




	if (ang > (PI2 / 8.0f))
	{
	
		v3Cross(&vec,&stratUp,&p);
		if (vec.z < 0.0)

			mPreRotateX(&crntStrat->m,amount);
		else
  		  	mPreRotateX(&crntStrat->m,-amount);




	}
	return;
#endif							   


	crntStrat->pnode->mass = 1000.0f;
	crntStrat->pnode->com.x = 0;
	crntStrat->pnode->com.y = 0;
	crntStrat->pnode->com.z = 0;



	force = crntStrat->vel;
 //	v3Normalise(&force);
 	v3Scale(&force, &force, crntStrat->pnode->mass * amount * 0.1);
	stratUp.x = crntStrat->m.m[2][0];
	stratUp.y = crntStrat->m.m[2][1];
	stratUp.z = crntStrat->m.m[2][2];
	p.x = stratUp.x * crntStrat->pnode->radius * 2.0f;
	p.y = stratUp.y * crntStrat->pnode->radius * 2.0f;
	p.z = stratUp.z * crntStrat->pnode->radius * 2.0f;
	ApplyForce(crntStrat, &force, &p, 0.01f);
}


void RelRotate(STRAT *strat, float x, float y, float z)
{
	if (x)
		mPreRotateX(&strat->m, x);
	if (y)
		mPreRotateY(&strat->m, y);
	if (z)
		mPreRotateZ(&strat->m, z);
}

//brings the strat pos into player's space
void SetCheckPosPlayerRotate(STRAT *strat, float ang)
{
	Point3	p;
	Matrix	im;

	p = strat->pos;
	v3Dec(&p, &player[currentPlayer].Player->pos);
	mInvertRot(&player[currentPlayer].Player->m,&im);
	mPoint3Apply3(&p, &im);
	strat->CheckPos = p;
}

//bring the strat checkpos into strat's space
void SetCheckPosMyRotate(STRAT *strat, float ang)
{
	Point3	p;
	Matrix	im;

	p = strat->CheckPos;
	v3Dec(&p, &strat->pos);
	mInvertRot(&strat->m,&im);
	mPoint3Apply3(&p, &im);
	strat->CheckPos = p;
}


void RelTurnTowardCheckPosXY(STRAT *strat, float ang)
{
	Point3	p;
	Matrix	im;
	Vector3	forward;
	float	actualAng;

	if (PAL)
		ang *= PAL_ACCSCALE;

	forward.x = 0.0f;
	forward.y = 1.0f;
	forward.z = 0.0f;
	p = strat->CheckPos;
	v3Dec(&p, &strat->pos);
	mInvertRot(&strat->m, &im);
	mPoint3Apply3(&p, &im);
	p.z = 0.0f;
	v3Normalise(&p);
	actualAng = (float) acos(v3Dot(&p, &forward));
	if ((p.x < 0.0f) && (ang < actualAng))
		mPreRotateZ(&strat->m, ang);
	else if ((p.x > 0.0f) && (ang < actualAng))
		mPreRotateZ(&strat->m, -ang);
	else if((p.x < 0.0f) && (ang >= actualAng))
		mPreRotateZ(&strat->m, actualAng);
	else if ((p.x > 0.0f) && (ang >= actualAng))
		mPreRotateZ(&strat->m, -actualAng);
}

void RelTurnTowardPlayerXY(STRAT *strat, float ang)
{
	Point3	p;
	Matrix	im;
	Vector3	forward;
	float	actualAng;

	if (PAL)
		ang *= PAL_MOVESCALE;

	forward.x = 0.0f;
	forward.y = 1.0f;
	forward.z = 0.0f;
	p = player[currentPlayer].Player->pos;
	v3Dec(&p, &strat->pos);
	mInvertRot(&strat->m, &im);
	mPoint3Apply3(&p, &im);
 	p.z = 0.0f;
	v3Normalise(&p);
	actualAng = (float) acos(v3Dot(&p, &forward));
	if ((p.x < 0.0f) && (ang < actualAng))
		mPreRotateZ(&strat->m, ang);
	else if ((p.x > 0.0f) && (ang < actualAng))
		mPreRotateZ(&strat->m, -ang);
	else if((p.x < 0.0f) && (ang >= actualAng))
		mPreRotateZ(&strat->m, actualAng);
	else if ((p.x > 0.0f) && (ang >= actualAng))
		mPreRotateZ(&strat->m, -actualAng);
}

void ApplyRelForce(STRAT *strat, int objId, float fx, float fy, float fz, float px, float py, float pz)
{
	Vector3	xAxis, yAxis, zAxis, force;
	Point3	p, fromP;
	Matrix	m;

	
	if (px || py || pz)
	{
		fromP.x = px;
		fromP.y = py;
		fromP.z = pz;
		
		if (objId)
		{
			m = strat->objId[objId]->wm;
		}
		else
			m = strat->m;

		mVector3Multiply(&p, &fromP, &m);

		if (objId)
		{
			p.x = strat->objId[objId]->wm.m[3][0] - strat->pos.x;
			p.y = strat->objId[objId]->wm.m[3][1] - strat->pos.y;
			p.z = strat->objId[objId]->wm.m[3][2] - strat->pos.z;
		}

		
	}
	else
	{
		p.x = px;
		p.y = py;
		p.z = pz;
	}

	xAxis.x = fx * strat->m.m[0][0];
	xAxis.y = fx * strat->m.m[0][1];
	xAxis.z = fx * strat->m.m[0][2];

	yAxis.x = fy * strat->m.m[1][0];
	yAxis.y = fy * strat->m.m[1][1];
	yAxis.z = fy * strat->m.m[1][2];

	zAxis.x = fz * strat->m.m[2][0];
	zAxis.y = fz * strat->m.m[2][1];
	zAxis.z = fz * strat->m.m[2][2];

	force.x =(xAxis.x + yAxis.x + zAxis.x);
	force.y =(xAxis.y + yAxis.y + zAxis.y);
	force.z =(xAxis.z + yAxis.z + zAxis.z);

	ApplyForce(strat, &force, &p, 0.1f);
}

void AvoidStrats(STRAT *crntStrat, float amount)
{
	STRAT *strat;
	StratList *sl;
	Vector3	fromTo,v1,v2;
	float	dist;

	if (!(crntStrat->flag & STRAT_AVOIDSTRAT))
		AddToAvoidStratArray(crntStrat);

	if (pSqrDist(&player[0].Player->pos, &crntStrat->pos) < 100.0f)
		return;

	v1.z = 0;
	v2.z = 0;
	sl = firstAvoidStrat;
	while(sl)
	{
		strat = sl->strat;
		if ((strat == crntStrat) || 
			(!strat->pnode) || 
			(strat->flag & STRAT_BULLET) || 
			(strat == player[currentPlayer].Player) || 
			(strat == player[currentPlayer].CameraStrat))
		{
			sl = sl->next;
			continue;
		}
		if (pDist(&strat->pos, &crntStrat->pos) - (strat->pnode->radius + crntStrat->pnode->radius) < amount)
		{
			dist = pDist(&crntStrat->pos, &strat->pos) - crntStrat->pnode->radius - strat->pnode->radius; 

			if (dist < 1.0f)
				dist = 1.0f;

			v3Sub(&fromTo, &crntStrat->pos, &strat->pos);
			v3Normalise(&fromTo);
			v3Scale(&fromTo, &fromTo, crntStrat->pnode->mass * (0.03f*amount) / dist);
			fromTo.z = 0.0f;
			ApplyForce(crntStrat, &fromTo, NULL, crntStrat->pnode->collForceRatio);
		}
		sl = sl->next;
	}
}

void TransObjectRel(STRAT *strat, int objId, float x, float y, float z)
{
	Vector3 trans,src;
	Matrix	m;
	src.x = x;
	src.y = y;
	src.z = z;

	mUnit(&m);
	
	mPreRotateXYZ(&m, strat->objId[objId]->crntRot.x, strat->objId[objId]->crntRot.y, strat->objId[objId]->crntRot.z);
	mVector3Multiply(&trans,&src,&m);

	strat->objId[objId]->crntPos.x += trans.x;
	strat->objId[objId]->crntPos.y += trans.y;
	strat->objId[objId]->crntPos.z += trans.z;
}

float	CheckDistXYNoRoot(STRAT *strat)
{
	float	xDif, yDif;

	xDif = strat->CheckPos.x - strat->pos.x;
	yDif = strat->CheckPos.y - strat->pos.y;

	return ((xDif * xDif) + (yDif * yDif));
}


void MoveTowardXY(STRAT *strat,float x,float y, float speed)
{
	Vector3 vel;


	vel.x = x - strat->pos.x;
	vel.y = y - strat->pos.y;


  //	speed = 3.0f;
  //	if (sqrt(vel.x*vel.x +vel.y*vel.y + vel.z*vel.z) < 1.0)
  //		return;

	if ((vel.x * vel.x + vel.y * vel.y) < 1.0)
		return;

	v3Normalise(&vel);
	vel.x *= speed;
	vel.y *= speed;

	if (GetGlobalParamInt(0))
	{
		strat->relAcc.x += vel.x;
		strat->relAcc.y += vel.y;
	}
	else
	{
		strat->vel.x += vel.x;
		strat->vel.y += vel.y;
	}
	//	strat->vel.z *= 0.9f;
}


void StickToParent(STRAT *strat, int objId, float x, float y, float z, int	mat)
{
	STRAT *parent;
	Vector3	px, py, pz;
	Point3 p;
	Matrix m;

	parent = strat->parent;
	if (objId)
	{
		objectToWorld(parent);
		px.x = parent->objId[objId]->wm.m[0][0];
		px.y = parent->objId[objId]->wm.m[0][1];
		px.z = parent->objId[objId]->wm.m[0][2];

		py.x = parent->objId[objId]->wm.m[1][0];
		py.y = parent->objId[objId]->wm.m[1][1];
		py.z = parent->objId[objId]->wm.m[1][2];

		pz.x = parent->objId[objId]->wm.m[2][0];
		pz.y = parent->objId[objId]->wm.m[2][1];
		pz.z = parent->objId[objId]->wm.m[2][2];

		v3Normalise(&px);
		v3Normalise(&py);
		v3Normalise(&pz);

		if (mat)
		{
			strat->m.m[0][0] = px.x;
			strat->m.m[0][1] = px.y;
			strat->m.m[0][2] = px.z;

			strat->m.m[1][0] = py.x;
			strat->m.m[1][1] = py.y;
			strat->m.m[1][2] = py.z;

			strat->m.m[2][0] = pz.x;
			strat->m.m[2][1] = pz.y;
			strat->m.m[2][2] = pz.z;
		}

		p.x = parent->objId[objId]->wm.m[3][0];
		p.y = parent->objId[objId]->wm.m[3][1];
		p.z = parent->objId[objId]->wm.m[3][2];

		v3Scale(&px, &px, x);
		v3Scale(&py, &py, y);
		v3Scale(&pz, &pz, z);

		v3Inc(&p, &px);
		v3Inc(&p, &py);
		v3Inc(&p, &pz);

		strat->pos = p;
		
	}
	else
	{
		px.x = parent->m.m[0][0];
		px.y = parent->m.m[0][1];
		px.z = parent->m.m[0][2];

		py.x = parent->m.m[1][0];
		py.y = parent->m.m[1][1];
		py.z = parent->m.m[1][2];

		pz.x = parent->m.m[2][0];
		pz.y = parent->m.m[2][1];
		pz.z = parent->m.m[2][2];
		strat->pos.x = parent->pos.x + x * px.x + y * py.x + z * pz.x;
		strat->pos.y = parent->pos.y + x * px.y + y * py.y + z * pz.y;
		strat->pos.z = parent->pos.z + x * px.z + y * py.z + z * pz.z;
		if (mat)
			strat->m = strat->parent->m;
	}
}

void PointTo(STRAT *strat, float x, float y, float z)
{
	Point3	p;

	p.x = x;
	p.y = y;
	p.z = z;
	pointToXZ(strat,&p);
}

int	ObjectHitStrat(STRAT *strat, int objId)
{
	if (strat->objId[objId]->collFlag & COLL_HITSTRAT)
	{
		//strat->objId[objId]->collFlag &= ~COLL_HITSTRAT;
		return 1;
	}
	else
		return 0;
}

int	ObjectHitFloor(STRAT *strat, int objId)
{
	if (strat->objId[objId]->collFlag & COLL_ON_FLOOR)
		return 1;
	else
		return 0;
}

void FragAtObject(STRAT *strat, int objId)
{
	Point3 p;

	p.x = strat->objId[objId]->wm.m[3][0];
	p.y = strat->objId[objId]->wm.m[3][1];
	p.z = strat->objId[objId]->wm.m[3][2];
	//findObjectWorldPosition(&p, strat, objId);
	rCreateExpFragments(&p, 0.1f, 100);
}

void RemoveObject(STRAT *strat, int objId)
{
	dAssert((strat) && (strat->pnode) && (objId <= strat->pnode->noObjId) && (strat->objId[objId]) && (strat->obj), "noStrat");

	strat->objId[objId]->collFlag |= OBJECT_NOCOLLISION;
	strat->objId[objId]->collFlag |= OBJECT_INVISIBLE;
}


void NoCollObject(STRAT *strat, int objId)
{
	strat->objId[objId]->collFlag |= OBJECT_NOCOLLISION;
}


void HideObject(STRAT *strat, int objId)
{
	if ((!strat) || (!(strat->pnode)) || (objId > strat->pnode->noObjId))
		return;

	if (strat->objId[objId])
		strat->objId[objId]->collFlag |= OBJECT_INVISIBLE;
}

void UnhideObject(STRAT *strat, int objId)
{
	if ((!strat) || (!(strat->pnode)) || (objId > strat->pnode->noObjId))
		return;

	if (strat->objId[objId])
		strat->objId[objId]->collFlag &= (0xffffffff - OBJECT_INVISIBLE);
}

void ClearRotFlag(STRAT *strat)
{
	strat->obj->collFlag &= ~OBJECT_CALC_MATRIX;
}

void ClearLastStratCollision(STRAT *strat)
{
	if (strat->target)
	{
	//	strat->target->flag2 &= (0xffffffff - STRAT2_COLLISIONWITHSTRAT);
		strat->target = NULL;
	}
}


#define COS90 0

static int CollideStratPos(STRAT *strat,Vector3 *pos)
{
	STRAT *str;
	float x,y,z,dist;
	Vector3 stratdir,probedir;
  //	float dirangle;
	float SquareRad;

	stratdir.z = 0;
	probedir.z = 0;

   //	dirangle = cos(PI2/4.0f);


	str = FirstStrat;
	while (str)
	{

		if ((!str->pnode) || (!(str->flag & STRAT_COLLSTRAT)) || (str->flag & STRAT_NOSTRAT2STRAT))
		{
			str = str->next;
			continue;
		}




		if (str->flag & STRAT_BULLET)
		{
			str = str->next;
			continue;
		}

	   	if ((str==strat) || (str == strat->parent) || (str->parent ==  strat))
		  //	((str->parent == strat) && (str->flag2 & STRAT2_SHIELDTYPE)) )
		{

			str = str->next;
			continue;

		}

		x = (str->pos.x - strat->pos.x);
		y = (str->pos.y - strat->pos.y);
		z = (str->pos.z - strat->pos.z);


		SquareRad  = str->pnode->radius * str->pnode->radius;
		//dist = (float)sqrt((x*x)+(y*y)); 
		dist = ((x*x)+(y*y)+(z*z)); 


		//is strat within candidate's radius
		//if (dist <= (str->pnode->radius ))
		if (dist <= SquareRad)
		{
			//check probe direction against candidate's direction
			probedir.x = strat->pos.x - pos->x;
			probedir.y = strat->pos.y - pos->y;
   //			probedir.z = 0;

			stratdir.x = strat->pos.x - str->pos.x;
			stratdir.y = strat->pos.y - str->pos.y;
	 //		stratdir.z = 0;

			v3Normalise(&probedir);
			v3Normalise(&stratdir);

		  //	if ((v3Dot(&probedir, &stratdir)) >= dirangle)	 
			if ((v3Dot(&probedir, &stratdir)) >= COS90)	 
				return(1);
		}
		else
		{
			//check that probe point is not whithin a strat's radius

			x = str->pos.x - pos->x;
			y = str->pos.y - pos->y; 
			z = str->pos.z - pos->z; 
		  //	dist = (float)sqrt((x*x)+(y*y)); 
			dist = ((x*x)+(y*y)+(z*z)); 
		  //	if (dist <= str->pnode->radius)
			if (dist <= SquareRad)
				 return(1);

		}



		str = str->next;
	}

	return 0;



}


int GetFreeCollisionSpace(STRAT *strat, float amount,int direction)
{
	int i;
	Vector3 forward,checkpos;


	amount = strat->pnode->radius * 3.0f;
	amount = strat->pnode->radius;

	forward.z = 0;
	for (i=direction;i<4;i++)
	{

		forward.x = forward.y = 0;
		
		switch (i)
		{
			/*forward vector check*/
			case 0:
				forward.x = amount;

				break;
			/*reverse vector check*/
			case 1:
				forward.x = -amount;

				
				break;
			/*right vector check*/
			case 2:
				forward.y = amount;

				break;
			/*left vector check*/
			case 3:
				forward.y = -amount;

				break;

		}


		mVector3Multiply (&checkpos, &forward, &strat->m);
		checkpos.x += strat->pos.x;
		checkpos.y += strat->pos.y;
		checkpos.z = strat->pos.z;

		/*is strat inside str's radius ?*/


		if (!CollideStratPos(strat,&checkpos)) 
		{ 
			if (PointInRegions(strat,&checkpos))
			{
		forward.x = forward.y = 0;
		
		switch (i)
		{
			/*forward vector check*/
			case 0:
				forward.x = amount * 10.0;

				break;
			/*reverse vector check*/
			case 1:
				forward.x = -amount * 10.0;

				
				break;
			/*right vector check*/
			case 2:
				forward.y = amount * 10.0;

				break;
			/*left vector check*/
			case 3:
				forward.y = -amount * 10.0;

				break;

		}

		mVector3Multiply (&checkpos, &forward, &strat->m);
		checkpos.x += strat->pos.x;
		checkpos.y += strat->pos.y;
		checkpos.z = strat->pos.z;


				strat->CheckPos.x = checkpos.x;
	  			strat->CheckPos.y = checkpos.y;
				return(1);
			}
	   	}	


	}
 
	return(0);
}
//UPDATE 0,1,2,3 

#define MP 0 
int CollideStratNew(STRAT *strat, int update)
{
	STRAT *str;
	int invalid;
	float x,y,z,dist,radius;
	Vector3 probe,stratForward,TESTVEC,v;
	stratForward.z = 0.0f;stratForward.x=0.0f;
 
	

	probe.x = strat->pos.x;
	probe.y = strat->pos.y;
	probe.z = strat->pos.z;


	str = FirstStrat;
	while (str)
	{

		if ((!str->pnode) || (!(str->flag & STRAT_COLLSTRAT)) || (str->flag & STRAT_NOSTRAT2STRAT))
		{
			str = str->next;
			continue;
		}




		if (str->flag & STRAT_BULLET)
		{
			str = str->next;
			continue;
		}

		if ((str==strat) || (str == strat->parent) || (str->parent ==  strat))
	   	//if ((str==strat) || (str == strat->parent) || ((str->parent == strat) && (str->flag2 & STRAT2_SHIELDTYPE)) )
		{

			str = str->next;
			continue;

		}

		invalid = 0;
		switch (update)
		{
			case 1:
				if (str == strat->target)
					invalid++;
					break;
			case 2:
				if (str != strat->target)
					invalid++;
					break;
			case 3:
				if (str->target == strat)
					invalid++;
					break;
			default:
				break;

		}

		if (invalid)
		{
			str= str->next;
			continue;
		}
			
		if (!str)
			invalid++;
		
		x = (str->pos.x - probe.x);
		y = (str->pos.y - probe.y);
		z = (str->pos.z - probe.z);



		dist = (float)sqrt((x*x)+(y*y) + (z*z)); 

		if (dist < (strat->pnode->radius))
		{

			if (update != 2)
			{
				strat->target = str;
				str->flag2 |= STRAT2_STRATCOLLIDED;
			}
			return(1);
		}
		str = str->next;
	}

	return 0;



}



void RelTurnTowardCheckPosSmoothXY(STRAT *strat, float ang)
{
	Point3	p;
	Matrix	im;
	Vector3	forward;
	float	actualAng;

	if (PAL)
		ang *= PAL_MOVESCALE;


	forward.x = 0.0f;
	forward.y = 1.0f;
	forward.z = 0.0f;
	p = strat->CheckPos;
	v3Dec(&p, &strat->pos);
	mInvertRot(&strat->m, &im);
	/*mInvert3d(&im, &strat->m);*/
	mPoint3Apply3(&p, &im);
	p.z = 0.0f;
	v3Normalise(&p);
	actualAng = (float) acos(v3Dot(&p, &forward));
	
	ang =     actualAng;
	dLog("%f\n", -ang * strat->pnode->mass * 0.01f); 
	
	if (p.x < 0.0f)
		ApplyRelForce(strat, 0, -ang * strat->pnode->mass * 0.02f, 0.0f, 0.0f, 0.0f, strat->pnode->radius * 1.0f, 0.0f);
	else if (p.x > 0.0f)
		ApplyRelForce(strat, 0, ang * strat->pnode->mass * 0.02f, 0.0f, 0.0f, 0.0f, strat->pnode->radius * 1.0f, 0.0f);
}

int	GetTargetCount(int pn)
{
	int i, count = 0;

	for (i=0; i<MAX_TARGETS; i++)
		if ((player[pn].target[i].strat) && //(player[pn].target[i].locked) && 
		   ((player[pn].target[i].strat->flag & COLL_TARGETABLE) || 
		   ((player[pn].target[i].obj->collFlag & COLL_TARGETABLE))))
			count++;

	return count;
}

void GetTargetStratPos(STRAT *strat, int pn, int si, int id)
{
	STRAT *sstrat;
	int objId;

	sstrat = (STRAT *)si;

	if ((sstrat) && !(sstrat->flag & STRAT_HIDDEN))
	{
		if (id)
		{
			strat->CheckPos.x = sstrat->objId[id]->wm.m[3][0];
			strat->CheckPos.y = sstrat->objId[id]->wm.m[3][1];
			strat->CheckPos.z = sstrat->objId[id]->wm.m[3][2];
		}
		else
			strat->CheckPos = sstrat->pos;
	}
	else
	{
		strat->CheckPos.x = strat->CheckPos.y = strat->CheckPos.z = 0.0f;
	}
}
int GetSecondaryTargetStrat(STRAT *strat, int pn)
{	
	int	fired = 0, i, n;

	for (i = 0; i < MAX_TARGETS; i++)
	{
		n = (i+player[pn].currentTarget) % MAX_TARGETS;
		if (player[pn].target[n].strat && (player[pn].target[n].strat->flag & COLL_TARGETABLE))
		{
			player[pn].currentTarget++;
			SetSem(strat, 0, 0);
			return (int)player[pn].target[n].strat;
		}
		else if (player[pn].target[n].strat && (player[pn].target[n].obj->collFlag & COLL_TARGETABLE))
		{
			player[pn].currentTarget++;
			SetSem(strat, 0, OBJECT_GET_ID(player[pn].target[n].obj->collFlag));
			return (int)player[pn].target[n].strat;
		}
	}
	return 0;
}


void ObjectNotTargetable(STRAT *strat, int objId)
{
	strat->objId[objId]->collFlag &= ~COLL_TARGETABLE;
}

void ObjectTargetable(STRAT *strat, int objId)
{
	strat->objId[objId]->collFlag |= COLL_TARGETABLE;
}

void SetOnCamera(STRAT *strat, float x, float y, float z, float rx, float ry, float rz)
{
	Matrix m, im;
	Vector3 pf, cf;
	float ang;

	/*mRotateXYZ (&m, player[currentPlayer].cam->u.lookDir.rotX, 
					player[currentPlayer].cam->u.lookDir.rotZ, 
					-player[currentPlayer].cam->u.lookDir.rotY);*/
	
	cf.x = player[0].CameraStrat->m.m[1][0];
	cf.y = player[0].CameraStrat->m.m[1][1];
	cf.z = player[0].CameraStrat->m.m[1][2];
	v3Normalise(&cf);
	mInvertRot(&player[0].Player->m, &im);
	mPoint3Apply3(&cf, &im);
	
	ang = asin (cf.z);
	
	m = player[0].CameraStrat->m;
	mPreRotateXYZ(&m, rx, ry, rz);
	mPreRotateY(&m, player[0].rollAng);
	mPreRotateX(&m, -ang);
	strat->m = m;
	strat->pos.x = player[currentPlayer].cam->pos.x + x * m.m[0][0] + y * m.m[1][0] + z * m.m[2][0];
	strat->pos.y = player[currentPlayer].cam->pos.y + x * m.m[0][1] + y * m.m[1][1] + z * m.m[2][1];
	strat->pos.z = player[currentPlayer].cam->pos.z + x * m.m[0][2] + y * m.m[1][2] + z * m.m[2][2];
	v3Inc(&strat->pos, &player[0].Player->vel);
	
}

void AwayWayZ(STRAT *strat, float maxx)
{
	Vector3	stratAxis, stratToWay, newAxis;
	float	ang,lastdist;
	Point3	*p;

	if (PAL)
		maxx *= PAL_MOVESCALE;

	p = &strat->CheckPos;

	stratAxis.x = -strat->m.m[1][0];
	stratAxis.y = -strat->m.m[1][1];
	stratAxis.z = 0.0f;
	
	stratToWay.x = (p->x - strat->pos.x);
	stratToWay.y = (p->y - strat->pos.y);
	stratToWay.z = 0.0f;

	v3Normalise(&stratAxis);
	v3Normalise(&stratToWay);


	ang = (float)acos(v3Dot(&stratAxis, &stratToWay));

	if (ang < 0.001f)
		return;



	if (ang > maxx)
		ang = maxx;

	if (!(AwaySeeWayZ(strat,PI2/8.0f)))
	{

		lastdist = CheckDistXYNoRoot(strat);
		ang = ang + (ang * PI2/lastdist * 2.0f);

		if (ang < maxx)
			ang = maxx;
	}
	
	v3Cross(&newAxis, &stratAxis, &stratToWay);

	if ((newAxis.x == 0.0f) &&
		(newAxis.y == 0.0f) &&
		(newAxis.z == 0.0f))
		return;

	if (newAxis.z > 0.0f)
	{
		mPreRotateZ(&strat->m, ang);
		strat->turnangz = ang;
	}
	else
	{
		mPreRotateZ(&strat->m, -ang);
		strat->turnangz = -ang;
	}
}

void ReturnToStartTransform(STRAT *strat,int objId)
{
 	strat->objId[objId]->idleRot.x = strat->pnode->objId[objId]->initRot.x;
 	strat->objId[objId]->idleRot.y = strat->pnode->objId[objId]->initRot.y;
 	strat->objId[objId]->idleRot.z = strat->pnode->objId[objId]->initRot.z;
 	strat->objId[objId]->crntPos.x = strat->pnode->objId[objId]->initPos.x;
 	strat->objId[objId]->crntPos.y = strat->pnode->objId[objId]->initPos.y;
 	strat->objId[objId]->crntPos.z = strat->pnode->objId[objId]->initPos.z;



}


/*CAN PUT A CHECK ON HERE TO CATER FOR NON-SCALE ANIMS*/
static void findTotalRotMatrixInterp(Matrix *mat, Object *obj)
{
	Vector3 invscale;
	Matrix temp2;

	if (obj->parent != NULL)
		findTotalRotMatrixInterp(mat, obj->parent);

	invscale.x = 1.0f / obj->scale.x;

	invscale.y = 1.0f / obj->scale.y;

	invscale.z = 1.0f / obj->scale.z;

		
   	temp2 = obj->m;

	//scale down the obj->m
   	mPreScale(&temp2, invscale.x, invscale.y, invscale.z);
  
	//multiply by current
   	mPreMultiply(mat,&temp2);
	  
}

static void findTotalRotMatrix(Matrix *mat, Object *obj)
{
	if (obj->parent != NULL)
		findTotalRotMatrix(mat, obj->parent);
	mPreRotateXYZ(mat, obj->crntRot.x, obj->crntRot.y, obj->crntRot.z);
}



//MAXXANG / MAXZANG SET TO TWOPI IF NO RESTRICTION REQUIRED ON THEM
//RETURNS 0 IF TURN COULD BE MADE
//1 IF TURN REQUIRED IS GREATER THAN MAXZANG
//2 IF TURN REQUIRED IS GREATER THAN MAXXANG
int ObjectTowardPlayerXZ(STRAT *strat, int	objId, float angx, float angz, float MAXZANG, float MAXXANG)
{
	Matrix	rotMat, im;
	Point3	relP,relP2;
	float	cangx2,cangz2,cangx, cangz;
	Vector3	temp, temp2;
	int retval=0;

	
	rotMat = strat->m;
 	if (strat->pnode->anim)
 		findTotalRotMatrixInterp(&rotMat, strat->objId[objId]);
 	else
		findTotalRotMatrix(&rotMat, strat->objId[objId]);

	mInvertRot(&rotMat, &im);
	mUnit(&rotMat);
	mPreTranslate(&rotMat, strat->pos.x, strat->pos.y, strat->pos.z);
	mPreMultiply(&rotMat, &strat->m);
	mPreScale(&rotMat, strat->scale.x, strat->scale.y, strat->scale.z);
	if (!(strat->pnode->anim))
		findTotalMatrix(&rotMat, strat->objId[objId]);
	else
	 	findTotalMatrixInterp(&rotMat, strat->objId[objId]);

	relP.x = rotMat.m[3][0];
	relP.y = rotMat.m[3][1];
	relP.z = rotMat.m[3][2];
	v3Sub(&relP, &player[currentPlayer].Player->pos, &relP);
	mPoint3Apply3(&relP, &im);
	temp = relP;
	temp2 = temp;
	temp2.z = 0.0f;
	v3Normalise(&temp);
	v3Normalise(&temp2);
	cangx = acos(v3Dot(&temp, &temp2));

	//temp = relP;
	temp2 = relP;
	temp2.x = 0.0f;
	//v3Normalise(&temp);
	v3Normalise(&temp2);
	cangz = acos(v3Dot(&temp, &temp2));



	//test for maxxang
   //	MAXXANG = (PI2/360.0f) * 10.0f;


   

	cangx2 = strat->objId[objId]->idleRot.x / PI2;
	cangx2 = (int)(cangx2);
	cangx2 = strat->objId[objId]->idleRot.x - (cangx2 * PI2);

	cangz2 = strat->objId[objId]->idleRot.z / PI2;
	cangz2 = (int)(cangz2);
	cangz2 = strat->objId[objId]->idleRot.z - (cangz2 * PI2);


	if (!strat->pnode->anim)
	{
		cangx2 -= strat->pnode->objId[objId]->initRot.x;
		cangz2 -= strat->pnode->objId[objId]->initRot.z;



	}



	
	
	if (angx > cangx)
		angx = cangx;


	if (angz > cangz)
		angz = cangz;

	//only pitch if we're allowed to

	if (cangx > 0.001f)
	{
		if (relP.z > 0.0)
		{
			   
			 if (MAXXANG == PI2 || ( 
				((cangx2 + angx < MAXXANG)
	   			&& (cangx2 + angx > -MAXXANG)))
				)
   				strat->objId[objId]->idleRot.x += angx;
			else
				retval = 2;

		}
		else
		{

		     if (MAXXANG == PI2 || (
				((cangx2 - angx < MAXXANG)
		   		&& (cangx2 - angx > -MAXXANG)))
				)
			 		strat->objId[objId]->idleRot.x -= angx;
	   		   else
				retval = 2;

		}
	}

	if (cangz > 0.001f)
	{
		if (relP.x > 0.0)
		{
		     if ((MAXZANG == PI2) || (
				((cangz2 - angz < MAXZANG)
		   		&& (cangz2 - angz > -MAXZANG)
				)))
				strat->objId[objId]->idleRot.z -= angz;
	   	   	 else
		   		retval = 1;
		}
		else
		{
		  	if ((MAXZANG == PI2) || (
			   ((cangz2 + angz < MAXZANG)
		 	   && (cangz2 + angz > -MAXZANG)
			   )))
	   			strat->objId[objId]->idleRot.z += angz;
		 	else
		 		retval = 1;
		}
	}



	strat->objId[objId]->collFlag |= OBJECT_STRATMOVE;




	return(retval);
}

int ObjectTowardPlayerXZVF(STRAT *strat, int	objId, float angx, float angz, float MAXZANG, float MAXXANG, float mul)
{
	Matrix	rotMat, im;
	Point3	relP,relP2;
	float	cangx2,cangz2,cangx, cangz;
	Vector3	temp, temp2, tempv;
	int retval=0;

	
	rotMat = strat->m;
 	if (strat->pnode->anim)
 		findTotalRotMatrixInterp(&rotMat, strat->objId[objId]);
 	else
		findTotalRotMatrix(&rotMat, strat->objId[objId]);

	mInvertRot(&rotMat, &im);
	mUnit(&rotMat);
	mPreTranslate(&rotMat, strat->pos.x, strat->pos.y, strat->pos.z);
	mPreMultiply(&rotMat, &strat->m);
	mPreScale(&rotMat, strat->scale.x, strat->scale.y, strat->scale.z);
	if (!(strat->pnode->anim))
		findTotalMatrix(&rotMat, strat->objId[objId]);
	else
	 	findTotalMatrixInterp(&rotMat, strat->objId[objId]);

	relP.x = rotMat.m[3][0];
	relP.y = rotMat.m[3][1];
	relP.z = rotMat.m[3][2];
	tempv = player[currentPlayer].Player->vel;
	v3Scale(&tempv, &tempv, mul);
	v3Inc(&tempv, &player[currentPlayer].Player->pos); 
	v3Sub(&relP, &tempv, &relP);
	mPoint3Apply3(&relP, &im);
	temp = relP;
	temp2 = temp;
	temp2.z = 0.0f;
	v3Normalise(&temp);
	v3Normalise(&temp2);
	cangx = acos(v3Dot(&temp, &temp2));

	//temp = relP;
	temp2 = relP;
	temp2.x = 0.0f;
	//v3Normalise(&temp);
	v3Normalise(&temp2);
	cangz = acos(v3Dot(&temp, &temp2));



	//test for maxxang
   //	MAXXANG = (PI2/360.0f) * 10.0f;


   

	cangx2 = strat->objId[objId]->idleRot.x / PI2;
	cangx2 = (int)(cangx2);
	cangx2 = strat->objId[objId]->idleRot.x - (cangx2 * PI2);

	cangz2 = strat->objId[objId]->idleRot.z / PI2;
	cangz2 = (int)(cangz2);
	cangz2 = strat->objId[objId]->idleRot.z - (cangz2 * PI2);


	if (!strat->pnode->anim)
	{
		cangx2 -= strat->pnode->objId[objId]->initRot.x;
		cangz2 -= strat->pnode->objId[objId]->initRot.z;



	}



	
	
	if (angx > cangx)
		angx = cangx;


	if (angz > cangz)
		angz = cangz;

	//only pitch if we're allowed to

	if (cangx > 0.001f)
	{
		if (relP.z > 0.0)
		{
			   
			 if (MAXXANG == PI2 || ( 
				((cangx2 + angx < MAXXANG)
	   			&& (cangx2 + angx > -MAXXANG)))
				)
   				strat->objId[objId]->idleRot.x += angx;
			else
				retval = 2;

		}
		else
		{

		     if (MAXXANG == PI2 || (
				((cangx2 - angx < MAXXANG)
		   		&& (cangx2 - angx > -MAXXANG)))
				)
			 		strat->objId[objId]->idleRot.x -= angx;
	   		   else
				retval = 2;

		}
	}

	if (cangz > 0.001f)
	{
		if (relP.x > 0.0)
		{
		     if ((MAXZANG == PI2) || (
				((cangz2 - angz < MAXZANG)
		   		&& (cangz2 - angz > -MAXZANG)
				)))
				strat->objId[objId]->idleRot.z -= angz;
	   	   	 else
		   		retval = 1;
		}
		else
		{
		  	if ((MAXZANG == PI2) || (
			   ((cangz2 + angz < MAXZANG)
		 	   && (cangz2 + angz > -MAXZANG)
			   )))
	   			strat->objId[objId]->idleRot.z += angz;
		 	else
		 		retval = 1;
		}
	}



	strat->objId[objId]->collFlag |= OBJECT_STRATMOVE;




	return(retval);
}


//AS ABOVE BUT WITH ADDITONAL OFFSET PASSED
//MAXXANG / MAXZANG SET TO TWOPI IF NO RESTRICTION REQUIRED ON THEM
//RETURNS 0 IF TURN COULD BE MADE
//1 IF TURN REQUIRED IS GREATER THAN MAXZANG
//2 IF TURN REQUIRED IS GREATER THAN MAXXANG
int ObjectTowardPlayerXZOffset(STRAT *strat, int	objId, float angx, float angz, float MAXZANG, float MAXXANG, float offx, float offy, float offz)
{
	Matrix	rotMat, im;
	Point3	relP,relP2;
	float	cangx2,cangz2,cangx, cangz;
	Vector3	temp, temp2;
	int retval=0;


	
	rotMat = strat->m;
 	if (strat->pnode->anim)
 		findTotalRotMatrixInterp(&rotMat, strat->objId[objId]);
 	else
		findTotalRotMatrix(&rotMat, strat->objId[objId]);

	mInvertRot(&rotMat, &im);
	mUnit(&rotMat);
	mPreTranslate(&rotMat, strat->pos.x, strat->pos.y, strat->pos.z);
	mPreMultiply(&rotMat, &strat->m);
	mPreScale(&rotMat, strat->scale.x, strat->scale.y, strat->scale.z);
	if (!(strat->pnode->anim))
		findTotalMatrix(&rotMat, strat->objId[objId]);
	else
	 	findTotalMatrixInterp(&rotMat, strat->objId[objId]);

	mPreTranslate(&rotMat,offx,offy,offz);



	relP.x = rotMat.m[3][0];
	relP.y = rotMat.m[3][1];
	relP.z = rotMat.m[3][2];

	v3Sub(&relP, &player[currentPlayer].Player->pos, &relP);
	mPoint3Apply3(&relP, &im);
	temp = relP;
	temp2 = temp;
	temp2.z = 0.0f;
	v3Normalise(&temp);
	v3Normalise(&temp2);
	cangx = acos(v3Dot(&temp, &temp2));

	//temp = relP;
	temp2 = relP;
	temp2.x = 0.0f;
	//v3Normalise(&temp);
	v3Normalise(&temp2);
	cangz = acos(v3Dot(&temp, &temp2));



	//test for maxxang
   //	MAXXANG = (PI2/360.0f) * 10.0f;


   

	cangx2 = strat->objId[objId]->idleRot.x / PI2;
	cangx2 = (int)((cangx2));
	cangx2 = strat->objId[objId]->idleRot.x - (cangx2 * PI2);

	cangz2 = strat->objId[objId]->idleRot.z / PI2;
	cangz2 = (int)((cangz2));
	cangz2 = strat->objId[objId]->idleRot.z - (cangz2 * PI2);


	if (!strat->pnode->anim)
	{
		cangx2 -= strat->pnode->objId[objId]->initRot.x;
		cangz2 -= strat->pnode->objId[objId]->initRot.z;



	}


	
	
	if (angx > cangx)
		angx = cangx;


	if (angz > cangz)
		angz = cangz;

	//only pitch if we're allowed to

	if (cangx > 0.001f)
	{
		if (relP.z > 0.0)
		{
			   
			 if (MAXXANG == PI2 || ( 
				((cangx2 + angx < MAXXANG)
	   			&& (cangx2 + angx > -MAXXANG)))
				)
   				strat->objId[objId]->idleRot.x += angx;
			else
				retval = 2;

		}
		else
		{

		     if (MAXXANG == PI2 || (
				((cangx2 - angx < MAXXANG)
		   		&& (cangx2 - angx > -MAXXANG)))
				)
			 		strat->objId[objId]->idleRot.x -= angx;
	   		   else
				retval = 2;

		}
	}

	if (cangz > 0.001f)
	{
		if (relP.x > 0.0)
		{
		     if ((MAXZANG == PI2) || (
				((cangz2 - angz < MAXZANG)
		   		&& (cangz2 - angz > -MAXZANG)
				)))
				strat->objId[objId]->idleRot.z -= angz;
	   	   	 else
		   		retval = 1;
		}
		else
		{
		  	if ((MAXZANG == PI2) || (
			   ((cangz2 + angz < MAXZANG)
		 	   && (cangz2 + angz > -MAXZANG)
			   )))
	   			strat->objId[objId]->idleRot.z += angz;
		 	else
		 		retval = 1;
		}
	}



	strat->objId[objId]->collFlag |= OBJECT_STRATMOVE;




	return(retval);
}


void RotateObjectXYZ(STRAT *strat, int objId, float x, float y, float z)
{
   /*	return;	  */
	if (PAL)
	{
		x *= PAL_MOVESCALE;
		y *= PAL_MOVESCALE;
		z *= PAL_MOVESCALE;

	}


	if (objId == 0)
	{
		strat->obj->crntRot.x += x;
		strat->obj->crntRot.y += y;
		strat->obj->crntRot.z += z;
		//ENSURE ID IS SET TO MOVE
		strat->obj->collFlag |= OBJECT_STRATMOVE;
	}
	else
	{
		strat->objId[objId]->crntRot.x += x;
		strat->objId[objId]->crntRot.y += y;
		strat->objId[objId]->crntRot.z += z;
		//ENSURE ID IS SET TO MOVE
		strat->objId[objId]->collFlag |= OBJECT_STRATMOVE;
	}
 
	


}

void RotateStratObjectXYZ(STRAT *strat, float x, float y, float z)
{
	mPreRotateXYZ(&strat->obj->m, x, y, z);
}

void ObjectReturnToStart(STRAT *strat, int objId)
{
	
	strat->objId[objId]->idleRot = strat->pnode->objId[objId]->initRot;
}

void RotationRestriction(STRAT *strat, float x, float y, float z)
{
	strat->rotRest.x = x;
	strat->rotRest.y = y;
	strat->rotRest.z = z;
}


void TowardTargetPosXZ(STRAT *strat, int	objId, float ang, Point3 *TARGPOS)
{
	Point3	pos, newP;
	float	angle;

	if (!strat->pnode)
		return;

 
	pointFromWorldToObject(&pos, TARGPOS, strat, strat->objId[objId]);
	if (pos.y == 0.0f)
		return;

	if (strat == player[0].Player)
	{
		switch (player[0].PlayerWeaponType)
		{
			case 0:
				pos.z -= 0.25f;
				pos.x -= 0.55f;
				break;
			case 1:
				pos.z -= 0.25f;
				break;
			case 2:
				break;
			case 3:
				pos.z -= 0.25f;
				break;
		}
	}

	angle = (pos.x / pos.y);
	if (fabs(angle) > 0.001f)
		angle = (float)atan(angle);
	else
		angle = 0.0f;

	if (pos.y < 0.0f)
		strat->objId[objId]->idleRot.z += angle * ang;   
	else
		strat->objId[objId]->idleRot.z -= angle * ang;   

	angle = pos.z / pos.y;
	if (fabs(angle) > 0.001f)
		angle = (float)atan(angle);
	else
		angle = 0.0f;

	if (pos.y < 0.0f)
		strat->objId[objId]->idleRot.x -= angle * ang;   
	else
		strat->objId[objId]->idleRot.x += angle * ang;   

	if (strat->objId[objId]->idleRot.x > PI)
		strat->objId[objId]->idleRot.x = PI;
	else if (strat->objId[objId]->idleRot.x < -PI)
		strat->objId[objId]->idleRot.x = -PI;
}

void IAmHoldPlayer(STRAT *strat)
{
	if (hpbFirst)
	{
		hpbLast->next = (HoldPlayerBlock *)CHeapAlloc(ObjHeap,sizeof(HoldPlayerBlock));
	  	#ifdef MEMORY_CHECK
			HPBALLOC += sizeof(HoldPlayerBlock);
		  	HPBALLOCCHKSUM += (int)hpbLast->next;
		#endif
		hpbLast->next->prev = hpbLast;
		hpbLast = hpbLast->next;
		hpbLast->m = strat->m;
		hpbLast->strat = strat;
		strat->hpb = hpbLast;
		hpbLast->next = NULL;
	}
	else
	{
		hpbFirst = (HoldPlayerBlock *)CHeapAlloc(ObjHeap,sizeof(HoldPlayerBlock));
	  	#ifdef MEMORY_CHECK
			HPBALLOC += sizeof(HoldPlayerBlock);
		  	HPBALLOCCHKSUM += (int)hpbFirst;
		#endif
		hpbFirst->next = NULL;
		hpbFirst->prev = NULL;
		hpbFirst->m = strat->m;
		hpbFirst->strat = strat;
		hpbLast = hpbFirst;
		strat->hpb = hpbFirst;
	}
	strat->flag2 |= STRAT2_HOLDPLAYER;
}

void IAmHoldPlayerMain(STRAT *strat)
{
	IAmHoldPlayer(strat);
	HoldPlayerMain = strat;
}

static void DefeatOptimizer(void)
{
	int a;
	a = 4;
}

void HoldPlayer(STRAT *strat)
{
	Matrix	im, m, *hm;
	Vector3	v;
	Point3	op, p;
	int	pn;

	for (pn = 0; pn<4; pn++)
	{
		if (player[pn].Player)
		{
			if (player[pn].PlayerOnHoldPlayer)
			{
				if (player[pn].PlayerOnHoldPlayer->strat == strat)
				{
					mInvertRot(&strat->hpb->m, &im);
					mMultiply(&m, &im, &strat->hpb->strat->m);
					mMultiply(&player[pn].Player->m, &player[pn].Player->m, &m);
					op = player[pn].Player->pos;
					p = op;
					v3Dec(&p, &strat->hpb->strat->pos);
					mPoint3Apply3(&p, &im);
					mPoint3Apply3(&p, &strat->hpb->strat->m);
					v3Inc(&p, &strat->hpb->strat->pos);
					v3Sub(&v, &p, &op);
					// COMPILER BUG WORKAROUND
					DefeatOptimizer();
					// END COMPILER BUG WORKAROUND
					
					v3Inc(&player[pn].Player->pos, &v);
					v3Inc(&player[pn].Player->oldPos, &v);
					v3Inc(&player[pn].Player->oldOldPos, &v);
					v3Inc(&player[pn].CameraStrat->pos, &v);
					v3Inc(&player[pn].CameraStrat->oldPos, &v);
					v3Inc(&player[pn].CameraStrat->oldOldPos, &v);
					v3Inc(&player[pn].camLookCrntPos, &v);

					v3Sub(&v, &strat->hpb->strat->pos, &strat->hpb->strat->oldPos);
					v3Inc(&player[pn].Player->pos, &v);
					v3Inc(&player[pn].Player->oldPos, &v);
					v3Inc(&player[pn].Player->oldOldPos, &v);
					v3Inc(&player[pn].CameraStrat->pos, &v);
					v3Inc(&player[pn].CameraStrat->oldPos, &v);
					v3Inc(&player[pn].CameraStrat->oldOldPos, &v);
					v3Inc(&player[pn].camLookCrntPos, &v);
					v3Sub(&HoldPlayerV, &player[pn].PlayerOnHoldPlayer->strat->oldPos, &player[pn].PlayerOnHoldPlayer->strat->oldOldPos);
					v3Inc(&HoldPlayerV, &v);
				}
				
			}
			if (strat->hpb)
				strat->hpb->m = strat->hpb->strat->m; 
		}
	}
}

void HoldPlayerPos(STRAT *strat, int objId, float px, float py, float pz)
{
	Vector3	relX, relY, relZ, tempv;
	Matrix  m;
	Point3	p;


	
	if (objId == 0)
	{
		m = strat->m;
		m.m[3][0] = strat->pos.x;
		m.m[3][1] = strat->pos.y;
		m.m[3][2] = strat->pos.z;
	}
	else
		fromObjectToWorld(&m, strat, strat->objId[objId], MATRIX_RST);
		//m = strat->objId[objId]->wm;

	p.x = px;
	p.y = py;
	p.z = pz;

	mPoint3Apply(&p, &m);
	//v3Sub(&tempv, &strat->oldPos, &strat->oldOldPos);
	//v3Inc(&p, &tempv);
	player[currentPlayer].Player->pos = p;
	player[currentPlayer].Player->vel.x = player[currentPlayer].Player->vel.y = player[currentPlayer].Player->vel.z = 0.0f;
}

void HoldPlayerRot(STRAT *strat, int objId, float rx, float ry, float rz)
{
	Matrix m;

	if (objId == 0)
	{
		m = strat->m;
	}
	else
	{
		m = strat->objId[objId]->wm;
		m.m[3][0] = 0.0f;
		m.m[3][1] = 0.0f;
		m.m[3][2] = 0.0f;
		m.m[0][3] = 0.0f;
		m.m[1][3] = 0.0f;
		m.m[2][3] = 0.0f;
		m.m[3][3] = 1.0f;

	}

	
	player[currentPlayer].Player->m = m;
	mPreRotateX(&player[currentPlayer].Player->m, rx);
	mPreRotateZ(&player[currentPlayer].Player->m, rz);
	mPreRotateY(&player[currentPlayer].Player->m, ry);
}


//SPLINE ROUTINES
static Matrix bezier = {-1.0, 3.0f, -3.0f, 1.0f, 
				 3.0f, -6.0f, 3.0f, 0.0f,
				 -3.0f, 3.0f, 0.0f, 0.0f,
				 1.0f, 0.0f, 0.0f, 0.0f};


static void SplinePathPos(SplineData *sd, struct route *route, int i, float t)
{
	float	l1, l2;
	int numpathpoints;
	struct waypath *path=route->path; 
	Point	tp, ctp, octp;
	Point3	p, p2, a, b, c, d;
	Vector3	tempv, tempv2, tangent, av, n, v;
	Matrix	pm;


	
	if (route->path->waytype == NETWORK)
		numpathpoints = path->net->patrol[route->patrolnumber].numpathpoints;
	else
		numpathpoints = path->numwaypoints;

	if ((i == 0) || (i == -1))
	{
		if (i == -1)
			i = 0;

		if (route->path->waytype == NETWORK)
		{
			a = path->waypointlist[path->net->patrol[route->patrolnumber].initialpath[i]];
			d = path->waypointlist[path->net->patrol[route->patrolnumber].initialpath[i+1]];
		}
		else
		{
			a = path->waypointlist[i];
			d = path->waypointlist[i+1];
		
		}
		v3Sub(&tempv, &d, &a);
		tempv2 = tempv;
		v3Scale(&tempv, &tempv, SPLINE_TURN);
		v3Add(&b, &tempv, &a);
		
	   
			
	   	if (i+2 == numpathpoints)
		{
			v3Sub(&c, &d, &tempv);
		}
		else
		{
			if (path->waytype == NETWORK)
				v3Sub(&tempv, &d, &path->waypointlist[path->net->patrol[route->patrolnumber].initialpath[i+2]]);
			else
			{
				v3Sub(&tempv, &d, &path->waypointlist[i+2]);
			}
			l1 = sqrt(v3Dot(&tempv2, &tempv2));
			if (v3SqrLength(&tempv))
				v3Normalise(&tempv);
			if (v3SqrLength(&tempv2))
				v3Normalise(&tempv2);
			v3Add(&av, &tempv, &tempv2);
			v3Scale(&av, &av, 0.5f);
			v3Cross(&n, &tempv, &tempv2);
			v3Cross(&tangent, &n, &av);
			v3Normalise(&tangent);
			v = tangent;
			v3Scale(&v, &v, SPLINE_TURN * l1);
			v3Sub(&c, &d, &v);
		}
		pm.m[0][0] = a.x; pm.m[0][1] = a.y; pm.m[0][2] = a.z; pm.m[0][3] = 1.0f;
		pm.m[1][0] = b.x; pm.m[1][1] = b.y; pm.m[1][2] = b.z; pm.m[1][3] = 1.0f;
		pm.m[2][0] = c.x; pm.m[2][1] = c.y; pm.m[2][2] = c.z; pm.m[2][3] = 1.0f;
		pm.m[3][0] = d.x; pm.m[3][1] = d.y; pm.m[3][2] = d.z; pm.m[3][3] = 1.0f;
		mPreMultiply(&pm, &bezier);
		sd->m = pm;
	}
	else if (i == numpathpoints - 2)
	{
		if (path->waytype == NETWORK)
		{
			a = path->waypointlist[path->net->patrol[route->patrolnumber].initialpath[i]];
			d = path->waypointlist[path->net->patrol[route->patrolnumber].initialpath[i+1]];
		}
		else
		{
			a = path->waypointlist[i];
			d = path->waypointlist[i+1];
		}
		v3Sub(&tempv, &a, &d);
		tempv2 = tempv;
		v3Scale(&tempv, &tempv, SPLINE_TURN);
		v3Add(&c, &tempv, &d);
		if (path->waytype == NETWORK)
			v3Sub(&tempv, &d, &path->waypointlist[path->net->patrol[route->patrolnumber].initialpath[i-1]]);
		else
		{
			v3Sub(&tempv, &a, &path->waypointlist[i-1]);
		}
		l1 = sqrt(v3Dot(&tempv2, &tempv2));
		if (v3SqrLength(&tempv))
			v3Normalise(&tempv);
		if (v3SqrLength(&tempv2))
			v3Normalise(&tempv2);
		v3Add(&av, &tempv, &tempv2);
		v3Scale(&av, &av, 0.5f);
		v3Cross(&n, &tempv, &tempv2);
		v3Cross(&tangent, &n, &av);
		v3Normalise(&tangent);
		v = tangent;
		v3Scale(&v, &v, SPLINE_TURN * l1);
		v3Sub(&b, &a, &v);
		pm.m[0][0] = a.x; pm.m[0][1] = a.y; pm.m[0][2] = a.z; pm.m[0][3] = 1.0f;
		pm.m[1][0] = b.x; pm.m[1][1] = b.y; pm.m[1][2] = b.z; pm.m[1][3] = 1.0f;
		pm.m[2][0] = c.x; pm.m[2][1] = c.y; pm.m[2][2] = c.z; pm.m[2][3] = 1.0f;
		pm.m[3][0] = d.x; pm.m[3][1] = d.y; pm.m[3][2] = d.z; pm.m[3][3] = 1.0f;
		mPreMultiply(&pm, &bezier);
		sd->m = pm;
	}
	else
	{
		if (path->waytype == NETWORK)
		{
			a = path->waypointlist[path->net->patrol[route->patrolnumber].initialpath[i]];
			d = path->waypointlist[path->net->patrol[route->patrolnumber].initialpath[i+1]];
		}
		else
		{
			a = path->waypointlist[i];
			d = path->waypointlist[i+1];
		}
		v3Sub(&tempv2, &d, &a);
	   	if (path->waytype == NETWORK)
	   		v3Sub(&tempv, &d, &path->waypointlist[path->net->patrol[route->patrolnumber].initialpath[i+2]]);
	   	else
		{
			v3Sub(&tempv, &d, &path->waypointlist[i+2]);
		}
		l1 = sqrt(v3Dot(&tempv2, &tempv2));
		if (v3SqrLength(&tempv))
			v3Normalise(&tempv);
		if (v3SqrLength(&tempv2))
			v3Normalise(&tempv2);
		v3Add(&av, &tempv, &tempv2);
		v3Scale(&av, &av, 0.5f);
		v3Cross(&n, &tempv, &tempv2);
		v3Cross(&tangent, &n, &av);
		v3Normalise(&tangent);
		v = tangent;
		v3Scale(&v, &v, SPLINE_TURN * l1);
		v3Sub(&c, &d, &v);

		v3Sub(&tempv2, &a, &d);
		if (path->waytype == NETWORK)
			v3Sub(&tempv, &d, &path->waypointlist[path->net->patrol[route->patrolnumber].initialpath[i-1]]);
		else
		{
			v3Sub(&tempv, &a, &path->waypointlist[i-1]);
	   
		
		}
		if (v3SqrLength(&tempv))
			v3Normalise(&tempv);
		if (v3SqrLength(&tempv2))
			v3Normalise(&tempv2);
		v3Add(&av, &tempv, &tempv2);
		v3Scale(&av, &av, 0.5f);
		v3Cross(&n, &tempv, &tempv2);
		v3Cross(&tangent, &n, &av);
		v3Normalise(&tangent);
		v = tangent;
		v3Scale(&v, &v, SPLINE_TURN * l1);
		v3Sub(&b, &a, &v);

		pm.m[0][0] = a.x; pm.m[0][1] = a.y; pm.m[0][2] = a.z; pm.m[0][3] = 1.0f;
		pm.m[1][0] = b.x; pm.m[1][1] = b.y; pm.m[1][2] = b.z; pm.m[1][3] = 1.0f;
		pm.m[2][0] = c.x; pm.m[2][1] = c.y; pm.m[2][2] = c.z; pm.m[2][3] = 1.0f;
		pm.m[3][0] = d.x; pm.m[3][1] = d.y; pm.m[3][2] = d.z; pm.m[3][3] = 1.0f;
		mPreMultiply(&pm, &bezier);
		sd->m = pm;
	}
	tp.x = t*t*t;
	tp.y = t*t;
	tp.z = t;
	tp.w = 1.0f;
	mLoad (&sd->m);
	mPoint (&ctp, &tp);
	sd->lineSeg = i;
	
	sd->p.x = ctp.x;
	sd->p.y = ctp.y;
	sd->p.z = ctp.z;
	dAssert(sd->p.x < 1000000.0f, "Balls");
}

static float SqrDistToLastWay(STRAT *strat)
{
	ROUTE *route = strat->route;
	Point3	p;

	p = route->path->waypointlist[route->path->numwaypoints - 1];

	return pSqrDist(&strat->pos, &p);
}
 
static int moveAlongPathXY(STRAT *strat, struct route *route, float amount)
{
  	struct waypath *path = route->path;
	float		dist, t;
	SplineData	*sd;
	Point3	p;
	int numpathpoints;
	
	dAssert(strat->route->splineData, "Spline Data not initialised\n");
	
	if (route->path->waytype == NETWORK)
		numpathpoints = path->net->patrol[route->patrolnumber].numpathpoints;
	else
		numpathpoints = path->numwaypoints;

	if (fabs(amount) < 0.01f)
	{
		strat->pos.x = strat->route->splineData->p.x;
		strat->pos.y = strat->route->splineData->p.y;
		return 1;
	}
	
	sd = strat->route->splineData;
	sd->oldvel = sd->vel;
	sd->oldp = sd->p;

	if (((amount > 0.0) && (sd->lineSeg + 2 == numpathpoints) && (SqrDistToLastWay(strat) < amount * amount)) ||
		((amount > 0.0f) && (sd->lineSeg + 1 >= numpathpoints)) ||
		((amount < 0.0f) && (sd->lineSeg < 0)))
	{
		strat->pos.x = sd->p.x;
		strat->pos.y = sd->p.y;
		return 0;
	}

	p = sd->p;
	if ( amount > 0.0f)
	{
		SplinePathPos(sd, route, sd->lineSeg, 1.0f);
		dist = pSqrDist(&p, &sd->p);

		if ((dist < 0.001f) && (sd->lineSeg + 2 == strat->route->path->numwaypoints))
			return 0;

		if (dist < amount * amount)
		{
			t = 0.5f;
			sd->lineSeg++;
			if (sd->lineSeg + 1 == strat->route->path->numwaypoints)
			{
				sd->t = 1.0f;
				sd->p = strat->route->path->waypointlist[sd->lineSeg];
				sd->lineSeg--;
				strat->pos.x = sd->p.x;
				strat->pos.y = sd->p.y;
				v3Sub(&sd->oldp_p, &sd->p, &sd->oldp);
				v3Sub(&sd->vel, &sd->p, &sd->oldp);
				return 1;
			}
			else
				sd->t = 0.0f;
		}
		else
			t = (1.0f - sd->t) * 0.5f;
		
		sd->t += t;
		SplinePathPos(sd, route, sd->lineSeg, sd->t);
		dist = pDist(&p, &sd->p);
		while ((dist > amount + SPLINE_ACC) || (dist < amount - SPLINE_ACC))
		{
			t *= 0.5f;
			if (dist > amount + SPLINE_ACC)
				sd->t -= t;
			else
				sd->t += t;

			SplinePathPos(sd, route, sd->lineSeg, sd->t);
			dAssert((sd->t <= 1.0f) && (sd->t >= 0.0f), "spline position error !(0.0<=t<=1.0)\n");
			
			dist = pDist(&p, &sd->p);	
			
		}
	}
	else
	{
		SplinePathPos(sd, route, sd->lineSeg, 0.0f);
		dist = pDist(&p, &sd->p);
		if (dist < -(amount + SPLINE_ACC))
		{
			t = 0.5f;
			sd->lineSeg--;
			if (sd->lineSeg == -1)
			{
				if (path->waytype == NETWORK)
					sd->p = path->waypointlist[path->net->patrol[route->patrolnumber].initialpath[0]];
				else 
					sd->p = path->waypointlist[0];
				sd->oldp = sd->p;
				sd->t = 0.0f;
				sd->ang = 0.0f;
				return 1;
			}
			sd->t = 0.5f;
		}
		else
		{
			t = sd->t * 0.5f;
			sd->t -= t;
		}

		SplinePathPos(sd, route, sd->lineSeg, sd->t);
		dist = pDist(&p, &sd->p);
		while ((dist > -(amount - SPLINE_ACC)) || (dist < -(amount + SPLINE_ACC)))
		{
			t *= 0.5f;
			if (dist > -(amount - SPLINE_ACC))
				sd->t += t;
			else
				sd->t -= t;
			SplinePathPos(sd, route, sd->lineSeg, sd->t);
			dist = pDist(&p, &sd->p);	
		}
	}

	strat->pos.x = p.x;
	strat->pos.y = p.y;

	v3Sub(&sd->oldp_p, &sd->p, &sd->oldp);
	v3Sub(&sd->vel, &sd->p, &sd->oldp);
	return 1;
}

int MoveAlongPathXY(STRAT *strat, float amount)
{
		float a;
		int res;
#define MAX_SPLINE_JUMP 5.0f
#if DRAWSPLINE
	DrawSplinePath(strat->route->path);
#endif

	// Account for PAL/NTSC issues
   	if (PAL)
		amount *= PAL_MOVESCALE;



	a = amount;
	res = 0;
	if (fabs(a) > 10000.0)
	{
		if (a < 0.0f)
			a += 10000.0f;
		else
			a -= 10000.0f;

		res = moveAlongPathXY(strat, strat->route, a);

	}
	else if (a > 0.0f)
	{
		if (a < MAX_SPLINE_JUMP)
		{
			res = moveAlongPathXY(strat, strat->route, a);
		}
		else
		{
			while (a > 0.0f)
			{
				a -= MAX_SPLINE_JUMP;
				if (a > 0.0f)
					res = moveAlongPathXY(strat, strat->route, MAX_SPLINE_JUMP);
				else
					res = moveAlongPathXY(strat, strat->route, a + MAX_SPLINE_JUMP);
			}
		}
	}
	else
	{
		if (a > -MAX_SPLINE_JUMP)
		{
			res = moveAlongPathXY(strat, strat->route, a);
		}
		else
		{
			while (a < 0.0f)
			{
				a += MAX_SPLINE_JUMP;
				if (a < 0.0f)
					res = moveAlongPathXY(strat, strat->route, -MAX_SPLINE_JUMP);
				else
					res = moveAlongPathXY(strat, strat->route, a - MAX_SPLINE_JUMP);
			}
		}
	}
	return res;
}

void FaceAlongPath(STRAT *strat, int dir)
{
	Vector3			x, y, z, temp;
	SplineData		sd;

/* 1 : face along direction of travel along path
   2 : face along path but always flat
   3 : face along the path never mind direction of travel*/

	if (dir == 1)
	{
		y = strat->route->splineData->oldp_p;
		v3Normalise(&y);
		temp.x = 0.0f;
		temp.y = 0.0f;
		temp.z = 1.0f;
		v3Cross(&x, &y, &temp);
		v3Normalise(&x);
		v3Cross(&z, &x, &y);
		strat->m.m[0][0] = x.x;
		strat->m.m[0][1] = x.y;
		strat->m.m[0][2] = x.z;

		strat->m.m[1][0] = y.x;
		strat->m.m[1][1] = y.y;
		strat->m.m[1][2] = y.z;

		strat->m.m[2][0] = z.x;
		strat->m.m[2][1] = z.y;
		strat->m.m[2][2] = z.z;
	}
	else if (dir == 2)
	{
		sd.p = strat->route->splineData->p;
		sd.oldp = sd.p;
		sd.lineSeg = strat->route->splineData->lineSeg;
		sd.t = strat->route->splineData->t;
		sd.ang = 0.0f;
		if (sd.t + 0.1f > 1.0f)
		{
			if (sd.lineSeg + 2 == strat->route->path->numwaypoints)
				return;
			else
				SplinePathPos(&sd, strat->route, sd.lineSeg + 1, (sd.t + 0.1f) - 1.0f);
		}
		else
			SplinePathPos(&sd, strat->route, sd.lineSeg, sd.t + 0.1f);
		v3Sub(&sd.oldp_p, &sd.p, &sd.oldp);
		y = sd.oldp_p;
		v3Normalise(&y);
		z.x = 0.0f;
		z.y = 0.0f;
		z.z = 1.0f;
		v3Cross(&x, &y, &z);
		v3Normalise(&x);
		/*v3Neg(&x);*/
		v3Neg(&z);
		v3Cross(&y, &x, &z);
		v3Neg(&z);
		strat->m.m[0][0] = x.x;
		strat->m.m[0][1] = x.y;
		strat->m.m[0][2] = x.z;

		strat->m.m[1][0] = y.x;
		strat->m.m[1][1] = y.y;
		strat->m.m[1][2] = y.z;

		strat->m.m[2][0] = z.x;
		strat->m.m[2][1] = z.y;
		strat->m.m[2][2] = z.z;
	}
	else
	{
		sd.p = strat->route->splineData->p;
		sd.oldp = sd.p;
		sd.lineSeg = strat->route->splineData->lineSeg;
		sd.t = strat->route->splineData->t;
		sd.ang = 0.0f;
		if (sd.t + 0.1f > 1.0f)
		{
			if (sd.lineSeg + 2 == strat->route->path->numwaypoints)
				return;
			else
				SplinePathPos(&sd, strat->route, sd.lineSeg + 1, (sd.t + 0.1f) - 1.0f);
		}
		else
			SplinePathPos(&sd, strat->route, sd.lineSeg, sd.t + 0.1f);
		v3Sub(&sd.oldp_p, &sd.p, &sd.oldp);
		y = sd.oldp_p;
		v3Normalise(&y);
		temp.x = 0.0f;
		temp.y = 0.0f;
		temp.z = 1.0f;
		v3Cross(&x, &y, &temp);
		v3Normalise(&x);
		v3Cross(&z, &x, &y);
		strat->m.m[0][0] = x.x;
		strat->m.m[0][1] = x.y;
		strat->m.m[0][2] = x.z;

		strat->m.m[1][0] = y.x;
		strat->m.m[1][1] = y.y;
		strat->m.m[1][2] = y.z;

		strat->m.m[2][0] = z.x;
		strat->m.m[2][1] = z.y;
		strat->m.m[2][2] = z.z;
	}
}



static void moveAlongPath(STRAT *strat, struct route *route, float amount)
{
  	struct waypath *path = route->path;
	float		dist, t, tempf;
	SplineData	*sd;
	Point3	p;
	int numpathpoints;
	
	dAssert(strat->route->splineData, "Spline Data not initialised\n");
	
	if (route->path->waytype == NETWORK)
		numpathpoints = path->net->patrol[route->patrolnumber].numpathpoints;
	else
		numpathpoints = path->numwaypoints;

	
	if (fabs(amount) < 0.01f)
	{
		strat->pos = strat->route->splineData->p;
		return;
	}
	
	if (strat->route->splineData->lineSeg + 2 == numpathpoints)
	{
		tempf = pDist(&strat->pos, &strat->route->path->waypointlist[numpathpoints - 1]);
		if (tempf < amount)
		{
			amount -= tempf;
			SetGlobalParamInt(0, 98);
			InitPath(strat);
			InitSplineData(strat);
			moveAlongPath(strat, route, amount);
			moveAlongPath(strat, route, amount);
			return;
		}
	}

	sd = strat->route->splineData;
	sd->oldvel = sd->vel;
	sd->oldp = sd->p;
														   //+2
	
	
	// 0 ..x.. 1 ..xx...2    - numpathpoints = 3, lineseg x = 0, lineseg xx = 1
	//therefore we are on last spline if lineseg = numpathpoints - 2
   	//matt change ... (before) sd->lineseg + 2 == numpathpoints - 1
	if (((amount > 0.0f) && (sd->lineSeg + 1 >= numpathpoints)) ||
		((amount < 0.0f) && (sd->lineSeg < 0)))
	{
		strat->pos = sd->p;
		return;
	}

	p = sd->p;
	if ( amount > 0.0f)
	{
		SplinePathPos(sd, route, sd->lineSeg, 1.0f);
		dist = pDist(&p, &sd->p);
		if (dist < amount - SPLINE_ACC)
		{
			t = 0.5f;
			sd->lineSeg++;
			if (sd->lineSeg + 1 == strat->route->path->numwaypoints)
			{
				sd->t = 1.0f;
				sd->p = strat->route->path->waypointlist[sd->lineSeg];
				sd->lineSeg--;
				strat->pos = sd->p;
				v3Sub(&sd->oldp_p, &sd->p, &sd->oldp);
				v3Sub(&sd->vel, &sd->p, &sd->oldp);
				return;
			}
			else
				sd->t = 0.0f;
		}
		else
			t = (1.0f - sd->t) * 0.5f;
		
		sd->t += t;
		SplinePathPos(sd, route, sd->lineSeg, sd->t);
		dist = pDist(&p, &sd->p);
		while ((dist > amount + SPLINE_ACC) || (dist < amount - SPLINE_ACC))
		{
			t *= 0.5f;

			if (t < 0.000001f)
				break;

			if (dist > amount + SPLINE_ACC)
				sd->t -= t;
			else
				sd->t += t;

			SplinePathPos(sd, route, sd->lineSeg, sd->t);
			dAssert((sd->t <= 1.0f) && (sd->t >= 0.0f), "spline position error !(0.0<=t<=1.0)\n");
			
			dist = pDist(&p, &sd->p);	
			
		}
	}
	else
	{
		SplinePathPos(sd, route, sd->lineSeg, 0.0f);
		dist = pDist(&p, &sd->p);
		if (dist < -(amount + SPLINE_ACC))
		{
			t = 0.5f;
			sd->lineSeg--;
			if (sd->lineSeg == -1)
			{
				if (path->waytype == NETWORK)
					sd->p = path->waypointlist[path->net->patrol[route->patrolnumber].initialpath[0]];
				else 
					sd->p = path->waypointlist[0];
				sd->oldp = sd->p;
				sd->t = 0.0f;
				sd->ang = 0.0f;
				return;
			}
			sd->t = 0.5f;
		}
		else
		{
			t = sd->t * 0.5f;
			sd->t -= t;
		}
		SplinePathPos(sd, route, sd->lineSeg, sd->t);
		dist = pDist(&p, &sd->p);
		while ((dist > -(amount - SPLINE_ACC)) || (dist < -(amount + SPLINE_ACC)))
		{
			t *= 0.5f;
			if (t < 0.000001f)
				break;
			if (dist > -(amount - SPLINE_ACC))
				sd->t += t;
			else
				sd->t -= t;
			SplinePathPos(sd, route, sd->lineSeg, sd->t);
			dist = pDist(&p, &sd->p);	
		}
	}

	strat->pos = p;

	v3Sub(&sd->oldp_p, &sd->p, &sd->oldp);
	v3Sub(&sd->vel, &sd->p, &sd->oldp);
}

void MoveAlongPath(STRAT *strat, float amount)
{
	float a;
#define MAX_SPLINE_JUMP 5.0f
#if DRAWSPLINE
	DrawSplinePath(strat->route->path);
#endif

	

	#define MOVE_FACT 1.0f

	if (amount == 0.0f)
	{
		strat->pos = strat->route->splineData->p;
		return;
	}

#if 0
	if (fabs(amount) > 10000.0f)
	{
		/*if (amount < 0.0f)
			amount = ((amount + 10000.0f) * MOVE_FACT) - 10000.0f;
		else
			amount = ((amount - 10000.0f) * MOVE_FACT) + 10000.0f;*/
	}
	else
		amount *= MOVE_FACT;
#endif
	
	// Account for PAL/NTSC issues
   	if (PAL)
		amount *= PAL_MOVESCALE;

	a = amount;
	if (fabs(a) > 10000.0)
	{
		if (a < 0.0f)
			a += 10000.0f;
		else
			a -= 10000.0f;

		moveAlongPath(strat, strat->route, a);

	}
	else if (a > 0.0f)
	{
		if (a < MAX_SPLINE_JUMP)
		{
			moveAlongPath(strat, strat->route, a);
		}
		else
		{
			while (a > 0.0f)
			{
				a -= MAX_SPLINE_JUMP;
				if (a > 0.0f)
					moveAlongPath(strat, strat->route, MAX_SPLINE_JUMP);
				else
					moveAlongPath(strat, strat->route, a + MAX_SPLINE_JUMP);
			}
		}
	}
	else
	{
		if (a > -MAX_SPLINE_JUMP)
		{
			moveAlongPath(strat, strat->route, a);
		}
		else
		{
			while (a < 0.0f)
			{
				a += MAX_SPLINE_JUMP;
				if (a < 0.0f)
					moveAlongPath(strat, strat->route, -MAX_SPLINE_JUMP);
				else
					moveAlongPath(strat, strat->route, a - MAX_SPLINE_JUMP);
			}
		}
	}
}

static float	zAng(Vector3 *a, Vector3 *b)
{
	Vector3	c, d;

	c = *a;
	d = *b;

	c.z = 0.0f;
	d.z = 0.0f;
	v3Normalise(&c);
	v3Normalise(&d);
	return acos(v3Dot(&c, &d));
}

static float	xAng(Vector3 *a, Vector3 *b)
{
	Vector3	c, d, forward, up, right;
	Matrix im, m;

	forward = *a;
	v3Normalise(&forward);
	d = *b;
	v3Normalise(&d);
	up.x = 0.0f;
	up.y = 0.0f;
	up.z = 1.0f;
	v3Cross(&right, &forward, &up);
	v3Cross(&up, &forward, &right);
	m.m[0][0] = right.x;
	m.m[0][1] = right.y;
	m.m[0][2] = right.z;
	m.m[1][0] = forward.x;
	m.m[1][1] = forward.y;
	m.m[1][2] = forward.z;
	m.m[2][0] = -up.x;
	m.m[2][1] = -up.y;
	m.m[2][2] = -up.z;
	mInvertRot(&m, &im);
	mPoint3Apply3(&d, &im);
	return asin(fabs(d.z));
}

static void mFlatten(Matrix *m)
{
	Vector3 up, right, forward;

	up.x = 0.0f;
	up.y = 0.0f;
	up.z = 1.0f;

	forward.x = m->m[1][0];
	forward.y = m->m[1][1];
	forward.z = m->m[1][2];

	v3Cross(&right, &forward, &up);
	v3Cross(&up, &right, &forward);
	m->m[0][0] = right.x;
	m->m[0][1] = right.y;
	m->m[0][2] = right.z;
	m->m[1][0] = forward.x;
	m->m[1][1] = forward.y;
	m->m[1][2] = forward.z;
	m->m[2][0] = up.x;
	m->m[2][1] = up.y;
	m->m[2][2] = up.z;
}

void makeMatrixFromVector(Vector3 *v, Matrix *m)
{
	Vector3 dir;

	if (v3Dot(v, v) < 0.0001f)
	{
		mUnit(m);
		return;
	}

	dir = *v;

	v3Normalise(&dir);

	if (dir.z > 0.9999f)
	{
		mUnit(m);
		mRotateXYZ(m, -3.14159f * 0.5f, 0.0f, 0.0f);
		
		return;
	}

	if (dir.z < -0.9999f)
	{
		mUnit(m);
		mRotateXYZ(m, 3.14159f * 0.5f, 0.0f, 0.0f);
		
		return;
	}

	m->m[0][0] = dir.y;
	m->m[0][1] = -dir.x;
	m->m[0][2] = 0.0f;
	m->m[0][3] = 0.0f;

	m->m[1][0] = dir.x;
	m->m[1][1] = dir.y;
	m->m[1][2] = dir.z;
	m->m[1][3] = 0.0f;

	m->m[2][0] = dir.z * dir.x;
	m->m[2][1] = dir.z * dir.y;
	m->m[2][2] = -((dir.x * dir.x) + (dir.y * dir.y));
	m->m[2][3] = 0.0f;

	m->m[3][0] = 0.0f;
	m->m[3][1] = 0.0f;
	m->m[3][2] = 0.0f;
	m->m[3][3] = 1.0f;
}

#if 0
void relPositionOfPointToLine(Point3 *p, Point3 *s, Point3 *e, Point3 *rp)
{
	Matrix m, im;
	Vector3	dir, tempv, tempv2;
	Point3 mp;

	v3Sub(&dir, e, s);
	v3Sub(&mp, p, s);
	v3Normalise(&dir);
	makeMatrixFromVector(&dir, &m);
	mInvertRot(&m, &im);
	mPoint3Multiply3(rp, &mp, &im);
}
#endif


float FacePlayerWaterSlide(STRAT *strat, float angOffset, float rs)
{	
	float hdist;
	Vector3	dir, v;
	Point3	res;

	//v3Sub(&dir ,&WaterSlideCamLookStrat->pos, &WaterSlide->pos);
	//relPositionOfPointToLine(&player[0].Player->pos, &WaterSlide->pos, &WaterSlideCamLookStrat->pos, &res);
	v.x = WaterSlide->m.m[1][0];
	v.y = WaterSlide->m.m[1][1];
	v.z = WaterSlide->m.m[1][2];
	//v3Sub(&v, &WaterSlideCamLookStrat->pos, &WaterSlide->pos);
	//v3Normalise(&v);
	v3Scale(&v, &v, 100.0f);
	v3Sub(&dir, &player[0].Player->pos, &WaterSlide->pos);
	v3Add(&res, &WaterSlide->pos, &v);
	v3Inc(&res, &dir);
	//v3Add(&res, &WaterSlideCamLookStrat->pos, &dir);
	//HoldPlayerMain->CheckPos = res;
	HoldPlayerMain->CheckPos = WaterSlideCamLookStrat->pos;
	RelTurnTowardCheckPosXY(HoldPlayerMain, 0.05);
}

/*
float FacePlayerWaterSlide(STRAT *strat, float angOffset, float rs)
{
	Matrix im;
	Point3	ip, tempp;
	float ang;
	Vector3	a, b, tempv;


	//mUnit(&HoldPlayerMain->m);
	//return;
	a.x = HoldPlayerMain->m.m[1][0];
	a.y = HoldPlayerMain->m.m[1][1];
	a.z = HoldPlayerMain->m.m[1][2];
	
	tempv.x = WaterSlideCamLookStrat->m.m[0][0];
	tempv.y = WaterSlideCamLookStrat->m.m[0][1];
	tempv.z = WaterSlideCamLookStrat->m.m[0][2];

	//if (BossStrat == WaterSlideCamLookStrat)
	//	v3Scale(&tempv, &tempv, 1.0f * -player[0].playerOffsetX);
	//else
	v3Scale(&tempv, &tempv, 1.0f * player[0].playerOffsetX);
	v3Add(&tempp, &WaterSlideCamLookStrat->pos, &tempv);
	v3Sub(&b, &tempp, &HoldPlayerMain->pos);
	ip = b;

	mInvertRot(&HoldPlayerMain->m, &im);
	mPoint3Apply3(&ip, &im);
	ang = zAng(&a, &b);

	if (ang > rs)
		ang = rs;
  
	if (ip.x < 0.0f)
		mPreRotateZ(&HoldPlayerMain->m, ang);
	else
		mPreRotateZ(&HoldPlayerMain->m, -ang);

	a.x = HoldPlayerMain->m.m[1][0];
	a.y = HoldPlayerMain->m.m[1][1];
	a.z = HoldPlayerMain->m.m[1][2];
	
	b.x = WaterSlide->m.m[1][0];
	b.y = WaterSlide->m.m[1][1];
	b.z = WaterSlide->m.m[1][2];
	ip = b;

	mInvertRot(&HoldPlayerMain->m, &im);
	mPoint3Apply3(&ip, &im);
	ang = xAng(&a, &b);

	if (ang > rs)
		ang = rs;

	if (ip.z < 0.0f)
		mPreRotateX(&HoldPlayerMain->m, -ang);
	else
		mPreRotateX(&HoldPlayerMain->m, ang);

	mFlatten(&HoldPlayerMain->m);
}*/

/*
float FacePlayerWaterSlide(STRAT *strat, float angOffset, float rs)
{
	Matrix im;
	Point3	ip;
	float ang;
	Vector3	a, b;

//	int n;

//	n = findHoldPlayerNumber(strat);

//	a.x = HoldPlayerStrat[n]->m.m[1][0];
//	a.y = HoldPlayerStrat[n]->m.m[1][1];
//	a.z = HoldPlayerStrat[n]->m.m[1][2];
	a.x = player[0].Player->m.m[1][0];
	a.y = player[0].Player->m.m[1][1];
	a.z = player[0].Player->m.m[1][2];
	
	
	v3Sub(&b, &WaterSlideCamLookStrat->pos, &player[currentPlayer].Player->pos);
	ip = b;

 //	mInvertRot(&HoldPlayerStrat[n]->m, &im);
	mInvertRot(&player[0].Player->m, &im);



	mPoint3Apply3(&ip, &im);
	ang = zAng(&a, &b);

	if (ang > rs)
		ang = rs;
  
	//if (ip.x < 0.0f)
	//	mPreRotateZ(&HoldPlayerStrat[n]->m, ang);
   //	else
   //		mPreRotateZ(&HoldPlayerStrat[n]->m, -ang);
   
	if (ip.x < 0.0f)
		mPreRotateZ(&player[0].Player->m, ang);
	else
		mPreRotateZ(&player[0].Player->m, -ang);


  //	a.x = HoldPlayerStrat[n]->m.m[1][0];
  //	a.y = HoldPlayerStrat[n]->m.m[1][1];
  //	a.z = HoldPlayerStrat[n]->m.m[1][2];
	a.x = player[0].Player->m.m[1][0];
	a.y = player[0].Player->m.m[1][1];
	a.z = player[0].Player->m.m[1][2];
	
	b.x = WaterSlide->m.m[1][0];
	b.y = WaterSlide->m.m[1][1];
	b.z = WaterSlide->m.m[1][2];
	ip = b;

  //	mInvertRot(&HoldPlayerStrat[n]->m, &im);
	mInvertRot(&player[0].Player->m, &im);

	mPoint3Apply3(&ip, &im);
	ang = xAng(&a, &b);

	if (ang > rs)
		ang = rs;

   //	if (ip.z < 0.0f)
   //		mPreRotateX(&HoldPlayerStrat[n]->m, -ang);
   //	else
   //		mPreRotateX(&HoldPlayerStrat[n]->m, ang);
	if (ip.z < 0.0f)
		mPreRotateX(&player[0].Player->m, -ang);
	else
		mPreRotateX(&player[0].Player->m, ang);

	mFlatten(&player[0].Player->m);

 //	mFlatten(&HoldPlayerStrat[n]->m);
}
*/

void InitSplineData(STRAT *strat)
{
   	/*if (!strat->route)
		 return;
	  */
	int start;

   
	if (strat->route->splineData == NULL)
	{
		strat->route->splineData = (SplineData *)CHeapAlloc(ObjHeap,sizeof(SplineData)); 
		#ifdef MEMORY_CHECK
		    ALLOCSPLINES += sizeof(SplineData);
			ALLOCSPLINESCHKSUM += (int)strat->route->splineData;
		#endif
	}	
	//	strat->route->splineData = (SplineData *)MissionAlloc (sizeof(SplineData));

	dAssert(strat->route->splineData != NULL, "No more mission memory left to allocate spline data\n");

	start=strat->route->pathstartnode;
	/*IF THE STRAT IS ON A NETWORK THEN TAKE ITS PATROL AS SPLINE DATA*/
	if (strat->route->path->waytype == NETWORK)
	{
		strat->route->splineData->p = strat->route->path->waypointlist[strat->route->path->net->patrol[strat->route->patrolnumber].initialpath[0]];
		strat->pos = strat->route->path->waypointlist[strat->route->path->net->patrol[strat->route->patrolnumber].initialpath[0]]; 
	}
	else
	{
		strat->route->splineData->p = strat->route->path->waypointlist[start];
		strat->pos = strat->route->path->waypointlist[start];
	}

	strat->route->splineData->vel.x =0;
	strat->route->splineData->vel.y =0;
	strat->route->splineData->vel.z =0;
	strat->route->splineData->oldvel.x =0;
	strat->route->splineData->oldvel.y =0;
	strat->route->splineData->oldvel.z =0;
	mUnit(&strat->route->splineData->m);
	strat->route->splineData->oldp_p = strat->route->splineData->oldp = strat->route->splineData->p;
	strat->route->splineData->lineSeg = start;
	strat->route->splineData->t = 0.0f;
	strat->route->splineData->ang = 0.0f;
}

void TiltAlongPath(STRAT *strat, float amount)
{
	float	ang;
	SplineData *sd;
	Vector3	n1, n2, v1, v2, up;

	if (PAL)
		amount *= PAL_MOVESCALE;

	sd = strat->route->splineData;
	up.x = 0.0f;
	up.y = 0.0f;
	up.z = 1.0f;
	v1 = sd->oldvel;
	if ((v1.x == 0.0f) && (v1.y == 0.0f) && (v1.z == 0.0f))
		v1 = sd->vel;

	v2 = sd->vel;
	if (v3SqrLength(&v1) < 0.0001f)
	{
		sd->ang = 0.0f;
		return;
	}
	v3Normalise(&v1);
	if (v3SqrLength(&v2) < 0.0001f)
	{
		sd->ang = 0.0f;
		return;
	}
	v3Normalise(&v2);
	ang = acos(v3Dot(&v1, &v2));
	if (ang > 0.0001f)
	{
		v3Cross(&n1, &sd->vel, &up);
		v3Cross(&n2, &sd->oldvel, &up);
		v3Cross(&up, &n1, &n2);
		if (up.z < 0.0f)
			sd->ang -= ang;
		else
			sd->ang += ang;

		
	}

	mPreRotateY(&strat->m, sd->ang * amount);
	
}

void SplineRelFlatten(STRAT *strat, float amount)
{
   //	if (PAL)
   //		strat->route->splineData->ang *= (0.95f * PAL_SCALE);
   //	else
		strat->route->splineData->ang *= 0.95f;
}


void FlattenToPoly(STRAT *strat, float amount)
{
	Vector3	force;
	Point3	p;
	Vector3	oldv;

	if (PAL)
		amount *= PAL_MOVESCALE;

	oldv = strat->vel;
	force = strat->cpn;
	v3Scale(&force, &force, 10.0f);
	p.x = strat->m.m[2][0] * 10.0f;
	p.y = strat->m.m[2][1] * 10.0f;
	p.z = strat->m.m[2][2] * 10.0f;
	ApplyForce(strat, &force, &p, amount);
	strat->vel = oldv;
}

void PickCheckPosStratToPlayerAngleOffset(STRAT *strat, float ang)
{
	Vector3	stratToPlayer, tempv;
	Matrix	m, im;
	Point3 p;

	mInvertRot(&strat->m, &im);
	v3Sub(&p, &player[0].Player->pos, &strat->pos);
	tempv = p;
	mPoint3Apply3(&p, &im);
	mRotateZ(&m, ang);
	mPoint3Apply3(&p, &m);
	mPoint3Apply3(&p, &strat->m);
	v3Add(&strat->CheckPos, &p, &strat->pos);
}

float ProjectileAngle(STRAT *strat, int objId, float xpos, float ypos, float zpos, float u, float tx, float ty, float tz)
{
	float	angle = 0.7854f, dangle = 0.7854f, ux, uy, t1, t2, a = -9.81f, t;
	float	A, B, C;
	float	distx, disty, v;
	Matrix	*m;
	Vector3	x, y, z;
	Point3	p1, p2;
	int		solution = HIGH_SOLUTION;

	objectToWorld(strat);
	m = &strat->objId[objId]->wm;
	x.x = m->m[0][0];
	x.y = m->m[0][1];
	x.z = m->m[0][2];
	y.x = m->m[1][0];
	y.y = m->m[1][1];
	y.z = m->m[1][2];
	z.x = m->m[2][0];
	z.y = m->m[2][1];
	z.z = m->m[2][2];
	v3Normalise(&x);
	v3Normalise(&y);
	v3Normalise(&z);
	v3Scale(&x, &x, xpos);
	v3Scale(&y, &y, ypos);
	v3Scale(&z, &z, zpos);
	p1.x = m->m[3][0];
	p1.y = m->m[3][1];
	p1.z = m->m[3][2];
	v3Inc(&p1, &x);
	v3Inc(&p1, &y);
	v3Inc(&p1, &z);
	p2.x = tx;
	p2.y = ty;
	p2.z = tz;

	disty = p2.z - p1.z;
	p1.z = 0.0f;
	p2.z = 0.0f;
	distx = pDist(&p1, &p2);
	
	if (solution == LOW_SOLUTION)
	{
		if (disty < 0.0f)
		{
			while(1)
			{
				ux = u * cos(angle);
				uy = u * sin(angle);
				t1 = ((-uy) + sqrt((uy * uy) + (4.0 * a * disty))) / (2.0f * a);
				t2 = ((-uy) - sqrt((uy * uy) + (4.0 * a * disty))) / (2.0f * a);
				
				if (t1 > t2 ) t = t1; else t = t2;

				if (ux * t > distx + 1.0f)
				{
					angle -= dangle;
					dangle *= 0.5f;
				}
				else if (ux * t < distx - 1.0f)
				{
					if (angle == 0.7854)
						break;
					angle += dangle;
					dangle *= 0.5f;
				}
				else
					break;

				if (dangle < 0.0002f)
					break;
			}
		}
		else
		{
			while(1)
			{
				ux = u * cos(angle);
				uy = u * sin(angle);
				v = ((uy * uy) + (4.0f * a * disty));
				t1 = ((-uy) + sqrt(v)) / (2.0f * a);
				t2 = ((-uy) - sqrt(v)) / (2.0f * a);

				if (t1 < t2) t = t1; else t = t2;

				if ( v < 0.0f)
					t = 0.0f;
			
				if ((ux * t > distx + 1.0f) || (t == 0.0f))
				{
					if (angle == 0.7854)
						break;

					angle += dangle;
					dangle *= 0.5f;
				}
				else if (ux * t < distx - 1.0f)
				{
					angle -= dangle;
					dangle *= 0.5f;
				}
				else
					break;

				if ((fabs(t1 - t2) < 0.01f))
					break;
			}

			if ((dangle < 0.00001f) || (ux * t < distx - 1.0f))
			{	
				angle = 0.7854f;
				dangle = 0.7854f;
				while(1)
				{

					ux = u * cos(angle);
					uy = u * sin(angle);
					v = (uy * uy) + (4.0 * a * disty);
					t1 = ((-uy) + sqrt(v)) / (2.0f * a);
					t2 = ((-uy) - sqrt(v)) / (2.0f * a);
					
					if (t1 > t2 ) t = t1; else t = t2;

					if (ux * t > distx + 1.0f)
					{
						angle -= dangle;
						dangle *= 0.5f;
					}
					else if ((ux * t < distx - 1.0f) || (v < 0.0f))
					{
						if (angle == 0.7854)
							break;
						angle += dangle;
						dangle *= 0.5f;
					}
					else
						break;

					if (dangle < 0.0002f)
						break;
				}
			}
		}
	}
	else
	{
		angle = 0.7854f;
		dangle = 0.7854f;
		while(1)
		{
			ux = u * cos(angle);
			uy = u * sin(angle);
			A = 0.5f * a;
			B = uy;
			C = -disty;

			t1 = ((-B) + sqrt((B * B) - (4.0f * A * C))) / (2.0f * A);
			t2 = ((-B) - sqrt((B * B) - (4.0f * A * C))) / (2.0f * A);
			if (t1 > t2) t = t1; else t = t2;
			if (ux * t > distx + 1.0f)
			{
				angle += dangle;
				dangle *= 0.5f;
			}
			else if (ux * t < distx - 1.0f)
			{
				if (angle == 0.7854)
					break;
				angle -= dangle;
				dangle *= 0.5f;
			}
			else
				break;

			if (dangle < 0.0002f)
				break;
		}
	}
		
	return angle;
}

void FacePlayerXY(STRAT *strat, float xang)
{
	Vector3 stratForward, stratToPlayer, cross;
	float	ang;

	mUnit(&strat->m);
	stratForward.x = 0.0f;
	stratForward.y = 1.0f;
	stratForward.z = 0.0f;
	v3Sub(&stratToPlayer, &player[currentPlayer].Player->pos, &strat->pos);
	stratToPlayer.z = 0.0f;
	v3Normalise(&stratToPlayer);
	v3Cross(&cross, &stratForward, &stratToPlayer);
	ang = acos(v3Dot(&stratForward, &stratToPlayer));
	
	if(cross.z < 0.0f)
		mPreRotateZ(&strat->m, -ang);
	else
		mPreRotateZ(&strat->m, +ang);	
	mPreRotateX(&strat->m, xang);
	
}

void FacePosXY(STRAT *strat, float xang)
{
	Vector3 stratForward, stratToPlayer, cross;
	float	ang;

	mUnit(&strat->m);
	stratForward.x = 0.0f;
	stratForward.y = 1.0f;
	stratForward.z = 0.0f;
	v3Sub(&stratToPlayer, &strat->CheckPos, &strat->pos);
	stratToPlayer.z = 0.0f;
	v3Normalise(&stratToPlayer);
	v3Cross(&cross, &stratForward, &stratToPlayer);
	ang = acos(v3Dot(&stratForward, &stratToPlayer));
	
	if(cross.z < 0.0f)
		mPreRotateZ(&strat->m, -ang);
	else
		mPreRotateZ(&strat->m, +ang);	
	mPreRotateX(&strat->m, xang);
	
}

void MoveAllStratsBack (STRAT *strat, float distance)
{
	int i; 
	STRAT *s;
	SPARK *spark;

	player[0].camNewPos.y -= distance;
	player[0].cam->pos.y -= distance;
	player[0].camLookCrntPos.y -= distance;

	for (s = FirstStrat; s; s = s->next) {
		if (s == strat)
			continue;
		s->pos.y -= distance;
		s->oldPos.y -= distance;
		s->oldOldPos.y -= distance;
	}
	
	
	CameraSet (player[0].cam, 0.0);
	doTarget(0);
	playerBotControl(0);
	
	
	spark = firstSpark;

	while(spark)
	{
		spark->pos.y -= distance;
		spark->oldPos.y -= distance;
		spark = spark->next;
	}
	moveTrailsBack(distance);
	

}

void AddToAvoidStratArray(STRAT *strat)
{
	strat->flag |= STRAT_AVOIDSTRAT;
	if ((firstAvoidStrat == NULL) && (lastAvoidStrat == NULL))
	{
		firstAvoidStrat = (StratList *)CHeapAlloc(ObjHeap,sizeof(StratList));
		firstAvoidStrat->next = NULL;
		firstAvoidStrat->prev = NULL;
		firstAvoidStrat->strat = strat;
		lastAvoidStrat = firstAvoidStrat;
	}
	else
	{
		lastAvoidStrat->next = (StratList *)CHeapAlloc(ObjHeap,sizeof(StratList));
		lastAvoidStrat->next->prev = lastAvoidStrat;
		lastAvoidStrat = lastAvoidStrat->next;
		lastAvoidStrat->next = NULL;
		lastAvoidStrat->strat = strat;
	}
}

