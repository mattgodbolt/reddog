#include "RedDog.h"
#include "strat.h"
#include "debugdraw.h"
#include "coll.h"
#include "camera.h"
#include "view.h"
#include "command.h"
#include "Player.h"
#include "input.h"
#include "stratphysic.h"
#include "stratway.h"

      

STRAT *BossGuide = NULL, *EscortTanker = NULL, *oldtStrat = NULL;
STRAT *EscortTankerHit[3];
STRAT *ScoobyDoo=NULL;
float globalVar, globalVar2, tStratTime = 0.0f;
Point3	shockArray[64], destinationPoint;
int		noShocks;
STRAT *BoatStrat, *ArrowStrat, *SpecialStrat;

#define N_ARROW_ARRAY	32
static STRAT *arrowArray[N_ARROW_ARRAY];
static char arrowArrayVis[N_ARROW_ARRAY];

float	GlobalParamFloat[8];
int		GlobalParamInt[8];
int	PlayerCameraState[4];
int fadeLength[4];
int fadeN[4];
int fadeMode[4];
int NoMainGun = 0;
int bad_cheat = 0;

Point3	drawTargetArray[64];
int		currentDrawTarget = 0;
int		freeSparkPointer = 0;

int		freeSpark[MAX_SP];
SPARK	*lastSpark = NULL;
SPARK	sparkArray[MAX_SP];
SPARK	*firstSpark = NULL;
extern	STRAT	*lavaStrat[32];
extern	int	noLavaStrats;
static float ang = 0;

static GroupCheckPos validArea[32];
STRAT *WaterSlide;

void TestCPlusPlus(void)
{}

  
void DrawLine(Point3 *a, Point3 *b, Uint32 colour)
{
	Colour	col;

	col.argb.a = (colour&0xff000000)>>24;
	col.argb.b = (colour&0x00ff0000)>>16;
	col.argb.g = (colour&0x0000ff00)>>8;
	col.argb.r = (colour&0x000000ff);
	mPush(&mCurMatrix);
	mUnit(&mCurMatrix);
	rLine(a, b, col, col);
	mPop(&mCurMatrix);
}

static void vectorToMatrix(Vector3 *v, Matrix *m)
{
	Vector3	x, y, z;

	if (v->z == 1.0)
	{
		z.x = 1.0f;
		z.y = z.z = 0.0f;
	}
	else
	{
		z.x = z.y = 0.0f;
		z.z = 1.0f;
	}

	y.x = v->x;
	y.y = v->y;
	y.z = v->z;
	v3Normalise(&y);
	v3Cross(&x, &y, &z);
	mUnit(m);
	m->m[0][0] = x.x;
	m->m[0][1] = x.y;
	m->m[0][2] = x.z;
	m->m[1][0] = y.x;
	m->m[1][1] = y.y;
	m->m[1][2] = y.z;
	m->m[2][0] = z.x;
	m->m[2][1] = z.y;
	m->m[2][2] = z.z;
}


void initSpark(void)
{
	int i;

	firstSpark = NULL;
	lastSpark = NULL;
	freeSparkPointer = 0;
	ang = 0;
	for (i=0; i<MAX_SP; i++)
	{
		freeSpark[i] = i;
		sparkArray[i].id = i;
		sparkArray[i].time = 0;
	}
}


static void v3RandDir(Vector3 *v, Vector3 *dir, float sx, float sz)
{
	Matrix	m;
	float ax, az;

	ax = RandRange(-sx * 0.5f, sx * 0.5f);
	az = RandRange(-sz * 0.5f, sz * 0.5f);
	vectorToMatrix(dir, &m);
	mPreRotateXYZ(&m, ax, 0.0f, az);
	v->x = m.m[1][0];
	v->y = m.m[1][1];
	v->z = m.m[1][2];
}

SPARK *addSpark(int n, Point3	*pos, Vector3 *dir, float speed, float spread, int time, int type, int icol)
{
	int	i, id;
	Vector3	v, tempv;

//	if (PAL)
//		speed *= PAL_SCALE;



	if (spread < 5.0f)
	{
		v = *dir;
		v3Normalise(&v);
	}

	for (i=0; i<n; i++)
	{
		if (freeSparkPointer >= MAX_SP - 1)
			break;
		id = freeSpark[freeSparkPointer];
		if (lastSpark == NULL)
		{
			sparkArray[id].prev = NULL;
			sparkArray[id].next = NULL;

			if (time > 3)
				time *= RandRange(0.7, 1.3);
			
			sparkArray[id].time = time;
			lastSpark = &sparkArray[id];
			firstSpark = lastSpark;
			lastSpark->pos = *pos;
			lastSpark->type = type;
			lastSpark->col.colour = icol;
			
		}
		else
		{
			sparkArray[id].prev = lastSpark;
			lastSpark->next = &sparkArray[id];
			lastSpark = lastSpark->next;
			lastSpark->next = NULL;
			lastSpark->pos = *pos;

			if (time > 3)
				time *= RandRange(0.7, 1.3);

			lastSpark->time = time;
			lastSpark->type = type;
			lastSpark->col.colour = icol;
		}
		if (spread > 0.f) {
			if (spread < 5.0)
			{
				v3RandDir(&tempv, &v, spread, spread);
			}
			else
			{
				v3Rand(&tempv);
			}
			v3Scale(&lastSpark->vel, &tempv, speed +  (speed * 0.5f * (frand()*2.f - 1.f)));
		} else {
			lastSpark->vel = *dir;
		}
		if (lastSpark->type == 5)
		{
			v3Inc(&lastSpark->vel, &player[0].Player->vel);
			if (lastSpark->vel.z < 0.0f)
				lastSpark->vel.z *= -1.0f;
		}

		/*if (icol)
		{
			lastSpark->col.argb.a = (icol >> 24) & 0xff;
			lastSpark->col.argb.r = (icol >> 16) & 0xff;
			lastSpark->col.argb.g = (icol >> 8) & 0xff;
			lastSpark->col.argb.b = (icol >> 0) & 0xff;
		}
		else
		{*/
	   	//suck type
		if (type==6)
		{

			lastSpark->col.argb.a = 180;
			lastSpark->col.argb.r = 170;
			lastSpark->col.argb.g = 170;
			lastSpark->col.argb.b = 170;
		}
		//whirlwind
		if (type==7)
		{

			lastSpark->col.argb.a = 180;
			lastSpark->col.argb.r = 170;
			lastSpark->col.argb.g = 185;
			lastSpark->col.argb.b = 190;
		}
		//mult colour whirlwind
		if (type==8)
		{

			lastSpark->col.argb.a = 240;
			lastSpark->col.argb.r = (int)RandRange(128,255.0);
			lastSpark->col.argb.g = (int)RandRange(128,255.0);
			lastSpark->col.argb.b = (int)RandRange(128,255.0);
		}
		//}
		freeSparkPointer++;
	}
	return(lastSpark);
}

void SparkSuck(STRAT *strat,int objid, int mode, int n, int icol, int time)
{
	SparkSuckPos(strat, objid, mode, n, icol, time, 0, 0, 0);
}

void SparkSuckPos(STRAT *strat, int objid, int mode, int n, int icol, int time, float x, float y, float z)
{
	Vector3 pos;
	Vector3 vel;
	SPARK *spark;
	Vector3 savepos;
	int i,max;
	float chargedist;

	/*
	if (mode == 6)
		max = 5;
	else
		max = 10;
	*/

	if (time == 0)
		time = 120;

	

	if ((mode == 11) || (mode == 12))
	{
		if (!(chargedist =  GetGlobalParamFloat(8)))
			chargedist = 40.0f;


		// ChargeupSpark
		savepos = strat->CheckPos;
		for (i = 0; i < n; ++i)
		{
			Vector3 v;
			float scalar;
			//set the position of the spark
			SetCheckPosRel(strat,objid,x,y,z);
			pos = strat->CheckPos;
		  //	vel.x = (strat->CheckPos.x - pos.x)*scalar;
		  //	vel.y = (strat->CheckPos.y - pos.y)*scalar;
		   //	vel.z = (strat->CheckPos.z - pos.z)*scalar;
			if (mode == 12)
			{
				//white explosion puff 	
				vel.x = 0;
				vel.y = 0;
				vel.z = 1.0f;
				icol = 255<<16 | 255 << 8 | 255;
				addSpark(1, &pos, &vel, RandRange(0.1,0.5), 1.3, time, 1, icol);
			}
			else
			{
				v3Rand (&v);
				scalar = 1.f / (float)(time-3);
				pos.x += v.x * chargedist;
				pos.y += v.y * chargedist;
				pos.z += v.z * chargedist;
				vel.x = (strat->CheckPos.x - pos.x)*scalar;
				vel.y = (strat->CheckPos.y - pos.y)*scalar;
				vel.z = (strat->CheckPos.z - pos.z)*scalar;
				addSpark(1, &pos, &vel, 1.f, 0, time, ABS(mode), icol); 
			}
		}
		strat->CheckPos = savepos;
	} else {
		savepos = strat->CheckPos;
		if (mode == 6)
			SetCheckPosRel(strat,objid,0,10.0,10.0);
		else
			SetCheckPosRel(strat,objid,x,y,z);
		
		pos = strat->CheckPos;
		strat->CheckPos = savepos;
		vel.x = strat->pos.x - pos.x;
		vel.y = strat->pos.y - pos.y;
		vel.z = strat->pos.z - pos.z;
		
		
		for (i=0;i<n;i++)
		{
			spark = addSpark(1, &pos, &vel, 0.8, 5.0, time, ABS(mode), icol); 
			spark->parentstrat = strat;
			spark->objid = objid;
		}
		if (mode == 6)
			SetCheckPosRel(strat,objid,0,10.0,-10.0);
		else
			SetCheckPosRel(strat,objid,0,0.0,0.0);
		
		pos = strat->CheckPos;
		strat->CheckPos = savepos;
		vel.x = strat->pos.x - pos.x;
		vel.y = strat->pos.y - pos.y;
		vel.z = strat->pos.z - pos.z;
		
		for (i=0;i<n;i++)
		{
			spark = addSpark(1, &pos, &vel, 0.8, 5.0, time, ABS(mode), icol); 
			spark->parentstrat = strat;
			spark->objid = objid;
		}
	}
}


static void removeSpark(SPARK *spark)
{
	if ((spark->next == NULL) && (spark->prev == NULL))
	{
		firstSpark = NULL;
		lastSpark = NULL;
	}
	else if (spark->prev == NULL)
	{
		firstSpark = spark->next;
		firstSpark->prev = NULL;
	}
	else if (spark->next == NULL)
	{
		lastSpark = spark->prev;
		lastSpark->next = NULL;
	}
	else
	{
		spark->prev->next = spark->next;
		spark->next->prev = spark->prev;
	}

	freeSparkPointer--;
	dAssert(freeSparkPointer >= 0, "balls to the max");
	freeSpark[freeSparkPointer] = spark->id;
}

void processSparks(void)
{
	SPARK *spark, *ns;
	Vector3 pos,diff;
	float mag;
	int deathtime =0, doneonce = 0;
	float offsetx,offsety;
	float fallspeed;

	fallspeed =  0.0218f;

	
  //	if (PAL)
  //		fallspeed *= PAL_SCALE;

	spark = firstSpark;
	ang = ang + PI2/360.0;

	while(spark)
	{
		ns = spark->next;
		spark->oldPos = spark->pos;
	 	if (spark->type >= 6 && spark->type < 11)
		{
			//parent dead ? then convert to drop frags (mg checks spark->parentstrat too)
			if ((spark->type != 9) && (!(spark->parentstrat) || !(spark->parentstrat->flag)))
			{
			   	spark->parentstrat = NULL;
				spark->vel.x += RandRange(-3.0,3.0);
				spark->vel.y += RandRange(-3.0,3.0);
				if (RandRange(0,10) < 6.0)
			   		spark->type = 10;
				else
					spark->type = 9;
			}
			else
			{
				if (spark->parentstrat)
				{
					if ((spark->objid))
					{
						pos.x =  spark->parentstrat->objId[spark->objid]->wm.m[3][0];
						pos.y =  spark->parentstrat->objId[spark->objid]->wm.m[3][1];
						pos.z =  spark->parentstrat->objId[spark->objid]->wm.m[3][2];
					}
					else
						pos = spark->parentstrat->pos;
				}
			}

			if (spark->type == 7)
			{
				if (!doneonce)
				{
					offsetx = 15.0 * sin(ang);
					offsety = 15.0 * cos(ang);
					doneonce = 1;

				}
				pos.x = pos.x  + offsety - offsetx;
				pos.y = pos.y + offsetx + offsety;
			}

		   	
			diff.x = spark->pos.x - pos.x;
			diff.y = spark->pos.y - pos.y;
			diff.z = spark->pos.z - pos.z;

			if (spark->type == 6)
			{
				if (spark->parentstrat) { // Vague paranoia courtesy of mg
					v3Normalise(&diff);
					spark->vel.x  -=(diff.x )/spark->time * 3;
					spark->vel.y  -=(diff.y )/spark->time * 3;
					spark->vel.z  -=(diff.z )/spark->time * 3;

					spark->pos.x += spark->parentstrat->vel.x;	
					spark->pos.y += spark->parentstrat->vel.y;	
					spark->pos.z += spark->parentstrat->vel.z;
				} else
					spark->type = 10;
				deathtime = 25;
			}
			else
			{
				if ((spark->type == 7) || (spark->type == 8))
				{
					//whirlwind affect just need this only, so make a type 7
					spark->vel.x  -=(spark->pos.x - pos.x )/spark->time;
					spark->vel.y  -=(spark->pos.y - pos.y )/spark->time;
					spark->vel.z  -=(spark->pos.z - pos.z )/spark->time;
				}
				else
				{
					if (spark->type == 10)
					{
						spark->vel.x += RandRange(-3.0,3.0);
						spark->vel.y += RandRange(-3.0,3.0);

					}
					spark->vel.x *= 0.85f;
					spark->vel.y *= 0.85f;
				 	spark->vel.z -= 0.0109f;

				}
			}
		}

		
		v3Inc(&spark->pos, &spark->vel);

		if (spark->type <= 6)
			spark->vel.z -= fallspeed;
		else
		{
			if (spark->col.argb.a < 255)
				spark->col.argb.a ++;

			if (spark->col.argb.r < 255)
				spark->col.argb.r ++;

			if (spark->col.argb.g < 255)
				spark->col.argb.g ++;

			if (spark->col.argb.b < 255)
				spark->col.argb.b ++;


		}


		spark->time--;

	//	if (PAL)
	//		v3Scale(&spark->vel, &spark->vel, 0.95f * PAL_SCALE);
	//   	else
			v3Scale(&spark->vel, &spark->vel, 0.95f);


		if (spark->time <= deathtime)
			removeSpark(spark);


		spark = ns;
	}
}

static void addShockToShockArray(Point3 *a, Point3 *b)
{
	if (noShocks > 31)
		return;
	shockArray[noShocks*2] = *a;
	shockArray[noShocks*2+1] = *b;
	noShocks ++;
}


void InitValidAreaArray()
{
   	int i;
	for (i=0;i<32;i++)
	{
		validArea[i].strat = NULL;
		validArea[i].radius = 0;

	}

}

void DeleteValidAreaEntry(STRAT *strat)
{
	int i;
	for (i=0;i<32;i++)
	{
		if (validArea[i].strat == strat)
			validArea[i].strat = NULL;

	}


}





void combineRotation(STRAT *crntStrat, Vector3 *v2, float speed2)
{
	Vector3	av1, av2, fv, *v1;
	float	fv_length;
	float	*speed1;
	Matrix	*m;

#if 0
   	if (PAL)
	{
	 	v3Scale(v2,v2,PAL_SCALE);
		speed2 *= PAL_SCALE;
	}
#endif

	m = &crntStrat->m;
	v1 = &crntStrat->rot_vect;
	speed1 = &crntStrat->rot_speed;
	av1.x = v1->x * (float)*speed1;
	av1.y = v1->y * (float)*speed1;
	av1.z = v1->z * (float)*speed1;
	av2.x = v2->x * (float)speed2;
	av2.y = v2->y * (float)speed2;
	av2.z = v2->z * (float)speed2;
	fv.x = av1.x + av2.x;
	fv.y = av1.y + av2.y;
	fv.z = av1.z + av2.z;

	
	fv_length = (float)sqrt(fv.x*fv.x + fv.y*fv.y + fv.z*fv.z);

	if (fv_length)
	{
		v1->x = fv.x / fv_length;
		v1->y = fv.y / fv_length;
		v1->z = fv.z / fv_length;
	}
	else
	{
		/*dLog("zero length vector (In combineRotation)\n"); */
		return;
	}

	*speed1 = fv_length;
}

void pointToXY(STRAT *crntStrat, Point3 *p)
{
	Vector3 stratToPos, stratForward;
	float	ang;

	if ((p->x == crntStrat->pos.x) && (p->y == crntStrat->pos.y))
		return;

	mUnit(&crntStrat->m);
 	stratForward.x = 0.0f;
	stratForward.y = 1.0f;
	stratForward.z = 0.0f;
	v3Sub(&stratToPos, p, &crntStrat->pos);
	stratToPos.z = 0.0f;
	v3Normalise(&stratToPos);
	ang = (float)acos(v3Dot(&stratToPos, &stratForward));

	if (crntStrat->pos.x < p->x)
		mPreRotateZ(&crntStrat->m, -ang);
	else
		mPreRotateZ(&crntStrat->m, ang); 
}

void ApplyForce(STRAT *crntStrat, Vector3 *v, Point3 *p, float ratio)
{
	float lambda, rot_speed, vv, nn, length, rLength, radius, mass;
	Point3	np, ap,fp,cm;
	Vector3	new_rot_vect, origin;
	float one_over_mass;

	if (v3SqrLength(v) < 0.0001f)
		return;

	if (PAL)
		ratio *= PAL_ACCSCALE;

	if (!crntStrat->pnode)
		return;

	origin.x = 0.0f;
	origin.y = 0.0f;
	origin.z = 0.0f;
	if (crntStrat->Player)
	{
		if (p == NULL)
			fp = crntStrat->Player->com;
		else
			fp = *p;
	}
	else
	{
		if (p == NULL)
			fp = crntStrat->pnode->com;
		else
			fp = *p;
	}


	if (crntStrat->Player)
		mPoint3Multiply3(&cm, &crntStrat->Player->com, &crntStrat->m);
	else
		mPoint3Multiply3(&cm, &crntStrat->pnode->com, &crntStrat->m);

	v3Dec(&fp, &cm);
	ap = fp;
	radius = (crntStrat->pnode->radius * crntStrat->scale.x);
	mass = crntStrat->pnode->mass;
	one_over_mass = 1.0f / mass;

	crntStrat->vel.x += v->x * one_over_mass;
	crntStrat->vel.y += v->y * one_over_mass;
	crntStrat->vel.z += v->z * one_over_mass;
	if ((v->x != 0.0f) ||
		(v->y != 0.0f) ||
		(v->z != 0.0f))
	{
		if ((fp.x != crntStrat->pnode->com.x) ||
			(fp.y != crntStrat->pnode->com.y) ||
			(fp.z != crntStrat->pnode->com.z))
		{
			vv = (v->x * v->x + v->y * v->y + v->z * v->z);
			lambda = -(v->x * ap.x + 
					  v->y * ap.y + 
					  v->z * ap.z) / vv;
					 
			np.x = ap.x + lambda * v->x;
			np.y = ap.y + lambda * v->y;
			np.z = ap.z + lambda * v->z;
			nn = -(float)((np.x*np.x + np.y*np.y + np.z*np.z) * sqrt(vv))*0.01f;
			new_rot_vect.x = v->y * np.z - v->z * np.y;
			new_rot_vect.y = v->z * np.x - v->x * np.z;
			new_rot_vect.z = v->x * np.y - v->y * np.x;

			length = v3Length(&new_rot_vect);
			if (fabs(length) > 0.01f)
			{
				rLength = (1.0f / length);
				new_rot_vect.x *= rLength * crntStrat->rotRest.x;
				new_rot_vect.y *= rLength * crntStrat->rotRest.y;
				new_rot_vect.z *= rLength * crntStrat->rotRest.z;
				rot_speed = nn / (crntStrat->pnode->mass * radius * radius * ratio * 0.3f);
				combineRotation( crntStrat, &new_rot_vect, rot_speed);
			}
		}
	}
}

void pointToXZ(STRAT *crntStrat, Point3 *p)
{
	Matrix im;
	Point3	ip, mp;
	Vector3	tempv;

	v3Sub(&mp, p, &crntStrat->pos);
	v3Normalise(&mp);
	mInvertRot(&crntStrat->m, &im);
	mPoint3Multiply3(&ip, &mp, &im);
	tempv = ip;
	v3Normalise(&tempv);
	if (tempv.y > 0.0f)
	{
		mPreRotateZ(&crntStrat->m, -asin(tempv.x));
	}
	else
	{
		mPreRotateZ(&crntStrat->m,  asin(tempv.x) - PI);
	}

	mInvertRot(&crntStrat->m, &im);
	mPoint3Multiply3(&ip, &mp, &im);
	tempv = ip;
	v3Normalise(&tempv);
	
	if (tempv.y > 0.0f)
	{
		mPreRotateX(&crntStrat->m, asin(tempv.z));
	}
	else
	{
		mPreRotateX(&crntStrat->m, PI - asin(tempv.z));
	}

}

static void findObjectWorldMatrix(Matrix *m, Object *obj, Matrix *fm)
{
	Matrix	newM;

	mTranslate(&newM, obj->crntPos.x, obj->crntPos.y, obj->crntPos.z);
	mPreRotateXYZ(&newM, obj->idleRot.x, obj->idleRot.y, obj->idleRot.z);
	mPreScale(&newM, obj->scale.x, obj->scale.y, obj->scale.z);

	mPreMultiply(&newM, m);
	if (obj->parent)
		findObjectWorldMatrix(&newM, obj->parent, fm);
	else
	{
		*fm = newM;
	}
}
	
void pointFromWorldToObject(Point3	*objP, Point3	*worldP, STRAT *strat, Object *obj)
{
	Matrix	im, m, sm, fm;


	sm = strat->m;
	mPreScale(&sm, strat->scale.x, strat->scale.y, strat->scale.z);
	mPostTranslate(&sm, strat->pos.x, strat->pos.y, strat->pos.z);

	if (obj->parent != NULL)
	{
		mUnit(&m);
		findObjectWorldMatrix(&m, obj, &fm);
		mPreMultiply(&sm, &fm);
		
	}

	mInvert3d(&im, &sm);
	mPoint3Multiply3(objP, worldP, &im);
}



static void OTWRec(Matrix *m, Object *obj)
{
	int	i;
	
	mPreTranslate(m, obj->crntPos.x, obj->crntPos.y, obj->crntPos.z);
	mPreRotateXYZ(m, obj->crntRot.x, obj->crntRot.y, obj->crntRot.z);
	mPreScale(m, obj->scale.x, obj->scale.y, obj->scale.z);

	obj->wm = *m;

	for (i=0; i<obj->no_child; i++)
	{
		mPush(m);
		OTWRec(m, &obj->child[i]);
		mPop(m);
	}
}

static void AnimOTWRec(Matrix *m, Object *obj)
{
	int	i;
	float x,y,z;   
	
	
   	mPreMultiply(m,&obj->m);

	obj->wm = *m;

#if DRAW_DEBUG
	if ((obj->ncp) && (!(obj->collFlag & OBJECT_NOCOLLISION)))
	{
		x = m->m[3][0];
		y = m->m[3][1];
		z = m->m[3][2];
		DrawMarker (x,y,z);
	}
#endif
	for (i=0; i<obj->no_child; i++)
	{
		mPush(m);
		AnimOTWRec(m, &obj->child[i]);
		mPop(m);
	}
}






void objectToWorld(STRAT *strat)
{
	Matrix	m;
	Object  *obj;

//	if (strat->pnode->flag & OBJECT_SHARE_OBJECT_DATA)
//		return;

	m = strat->m;
	mPostTranslate(&m, strat->pos.x, strat->pos.y, strat->pos.z);
	mPreScale(&m, strat->scale.x, strat->scale.y, strat->scale.z);
  //	if ((strat->pnode->anim) && (!(strat->pnode->anim->anim->type & SCALE_ANIM)))
	if ((strat->pnode->anim))
  		AnimOTWRec(&m, strat->obj);
	else
		OTWRec(&m, strat->obj);
}

void ValidatePosition(STRAT *strat, float dist)
{
	int	i;
	Point3	s1p, s2p;
	float	l, ad;
	Vector3	force;

	s1p = strat->pos;
	s1p.z = 0.0f;
	for (i=0; i<32; i++)
	{
		if (validArea[i].strat)
		{
			if (validArea[i].strat == strat)
				continue;
			else
			{
				s2p = validArea[i].strat->pos;
				s2p.z = 0.0f;
				l = (dist + validArea[i].radius) - pDist(&s1p, &s2p);
				if (l > 0.0f)
				{
					if ((s1p.x != s2p.x) || (s1p.y != s2p.y))
					{
						v3Sub(&force, &s1p, &s2p);
						v3Normalise(&force);
					}
					else
					{
						force.x = 0.0f;
						force.y = 1.0f;
						force.z = 0.0f;
					}
					v3Scale(&force, &force, RANGE(0.0, 30.0f * l, 1.0f));
					ApplyForce(strat, &force, NULL, 1.0f);
					v3Neg(&force);
					ApplyForce(validArea[i].strat, &force, NULL, 1.0f);
				}
			}
		}
	}

	for (i=0; i<32; i++)
	{
		if (validArea[i].strat == strat)
		{
			validArea[i].radius = dist;
			break;
		}
	}
	
	if (i == 32)
	{
		for (i=0; i<32; i++)
		{
			if (!validArea[i].strat)
			{
				validArea[i].strat = strat;
				validArea[i].radius = dist;
				break;
			}
		}
	}
}

float PlayerDistXY(STRAT *strat)
{
	Point3	a, b;

	a = strat->pos;
	b = player[currentPlayer].Player->pos;
	a.z = 0.0f;
	b.z = 0.0f;
	return pDist(&a, &b);
}

float ParentDistXY(STRAT *strat)
{
	Point3	a, b;

	a = strat->pos;
	b = strat->parent->pos;
	a.z = 0.0f;
	b.z = 0.0f;
	return pDist(&a, &b);
}

void AvoidPlayer(STRAT *strat, float radius)
{
	float	dist, amount;
	Vector3	playerToStrat;

	dist = PlayerDistXY(strat);
	if (dist < radius)
	{
		amount = radius - dist;
		v3Sub(&playerToStrat, &strat->pos, &player[currentPlayer].Player->pos);
		if ((!playerToStrat.x) && (!playerToStrat.y))
		{
			playerToStrat.x = playerToStrat.y = 1.0f;
		}
		
		playerToStrat.z = 0.0f;
		v3Normalise(&playerToStrat);
		v3Scale(&playerToStrat, &playerToStrat, amount * 10.0f);
		ApplyForce(strat, &playerToStrat, NULL, 1.0f);
	}
}

int NearParentXY(STRAT *strat,float dist)
{
	double x,y;
	float real;

	x = (strat->parent->pos.x-strat->pos.x );
	y = (strat->parent->pos.y-strat->pos.y );
	dist = dist * dist;
	
	real =(float)((x*x)+(y*y));
	if (real < ((strat->pnode ? (strat->pnode->radius*strat->scale.x) : 0) + dist))//{

  //	if (real <=dist)
		return(1);
	else
		return (0);
}



int CrntRotToIdleRotZ(STRAT *strat, int objId, float ang)
{
	Vector3 stratforward;
	Vector3 targetforward;
	Vector3 crossvec;
	float turnang;
	Matrix forwardmat,targetmat;



	if (strat->objId[objId]->crntRot.z == strat->objId[objId]->idleRot.z)
		return (1);

	if (PAL)
		ang *= PAL_MOVESCALE;

	mRotateZ(&forwardmat,strat->objId[objId]->crntRot.z);
	mRotateZ(&targetmat,strat->objId[objId]->idleRot.z);

	stratforward.y = forwardmat.m[1][1];
	targetforward.y = targetmat.m[1][1];

	stratforward.z = 0;
	stratforward.x = forwardmat.m[1][0];
	targetforward.z = 0;
	targetforward.x = targetmat.m[1][0];

	v3Normalise(&stratforward);
	v3Normalise(&targetforward);


	turnang = acos(v3Dot(&stratforward,&targetforward));

	if (turnang > ang)
		turnang = ang;



	v3Cross(&crossvec,&stratforward,&targetforward);


	if (crossvec.z >0)
		strat->objId[objId]->crntRot.z += turnang;
	else
		strat->objId[objId]->crntRot.z -= turnang;


	 return(1);

}

int CrntRotToIdleRotX(STRAT *strat, int objId, float ang)
{
	Vector3 stratforward,forward;
	Vector3 vec,vec2;
	Vector3 crossvec;
	float turnang;
	Matrix crntmat,idleinv,idlemat;

	if (strat->objId[objId]->crntRot.x == strat->objId[objId]->idleRot.x)
		return (1);

	if (PAL)
		ang *= PAL_MOVESCALE;

	//derive idle rot matrix

	mRotateXYZ(&idlemat,strat->objId[objId]->idleRot.x,strat->objId[objId]->idleRot.y,strat->objId[objId]->idleRot.z);

	//invert

	mInvert(&idleinv,&idlemat);

	//apply to currentrot vector to bring it into idle's space
	forward.x  = 0;
	forward.y = 1;
	forward.z = 0.0;
	mRotateXYZ(&crntmat,strat->objId[objId]->crntRot.x,strat->objId[objId]->crntRot.y,strat->objId[objId]->crntRot.z);
   	//make forward vector of crnt rot
	mVector3Multiply (&vec2, &forward, &crntmat);
	

	//bring it into idle's space

	mVector3Multiply (&vec, &vec2, &idleinv);
	

	//zero x

	vec.x = 0;
	stratforward.x = 0;

	stratforward.y = 1;
	stratforward.z = 0;

	v3Normalise(&vec);




	//dot and cross
	turnang = acos(v3Dot(&vec,&stratforward));


	if (turnang > ang)
		turnang = ang;

	
	
   	v3Cross(&crossvec,&vec,&stratforward);

   	if (crossvec.x >0)
		strat->objId[objId]->crntRot.x += turnang;
	else
		strat->objId[objId]->crntRot.x -= turnang;


	 return(1);

}


int CrntRotToIdleRotY(STRAT *strat, int objId, float ang)
{
	float *c, *i;
	int n;

	if (PAL)
		ang *= PAL_MOVESCALE;

	c = &strat->objId[objId]->crntRot.y;
	i = &strat->objId[objId]->idleRot.y;
	if (*c == *i)
		return 1;
	else if (*c < *i)
	{
		if (*i - *c > ang)
			*c += ang;
		else
		{
			*c = *i;
			return 1;
		}
	}
	else
	{
		if (*c - *i > ang)
			*c -= ang;
		else
		{
			*c = *i;
			return 1;
		}
	}
	return 0;
}



void ApplyAbsForce(STRAT *strat, float fx, float fy, float fz, float px, float py, float pz, float r)
{
	Vector3 f;
	Point3 p;

	if (PAL)
	{
		f.x = fx * PAL_ACCSCALE;
		f.y = fy * PAL_ACCSCALE;
		f.z = fz * PAL_ACCSCALE;
	}
	else
	{
		f.x = fx;
		f.y = fy;
		f.z = fz;
	}

	p.x = px;
	p.y = py;
	p.z = pz;

	ApplyForce(strat, &f, &p, r);
}

void FaceAlongDirXY(STRAT *strat, int type)
{
	Vector3 v, n;
	Matrix im;
	float ang;

	n.x = n.z = 0.0f;
	n.y = 1.0f;
	v3Sub(&v, &strat->pos, &strat->oldPos);
	if (type == -1)
		v3Neg(&v);

	if ((v.x == 0.0f) && (v.y == 0.0f))
		return;
	/*dAssert(!(v.x == v.y == 0.0f) , "FaceAlongDir");*/
	mInvertRot(&strat->m, &im);
	mPoint3Apply(&v, &im);
	v.z = 0.0f;
	v3Normalise(&v);
	ang = acos(v3Dot(&n, &v));
	if (v.x > 0.0f)
		ang *= -1.0f;
	mPreRotateZ(&strat->m, ang);
}

int PlayerNearPlayerCheckPosXY(float dist)
{
	Vector3	v;
	float	radius = player[currentPlayer].Player->pnode->radius + dist;

	v3Sub(&v, &player[currentPlayer].Player->pos, &player[currentPlayer].Player->CheckPos);
	v.z = 0.0f;
	if (v3Dot(&v, &v) < radius * radius)
		return 1;

	return 0;
}

void PlayerTurnTowardPlayerCheckPosXY(void)
{
	Matrix im;
	Point3	ip, a, b;
	Vector3	pf, pc;
	float	ang;

	a = player[currentPlayer].Player->CheckPos;
	b = player[currentPlayer].Player->pos;
	a.z = b.z = 0.0f;
	v3Sub(&ip, &a, &b);
	#if DRAW_DEBUG
		//DrawMarker(a.x, a.y, Player->pos.z); 
	#endif
	mInvertRot(&player[currentPlayer].Player->m, &im);
	mPoint3Apply(&ip, &im);
	v3Normalise(&ip);
	pf.x = player[currentPlayer].Player->m.m[1][0];
	pf.y = player[currentPlayer].Player->m.m[1][1];
	pf.z = 0.0f;
	v3Sub(&pc, &player[currentPlayer].Player->CheckPos, &player[currentPlayer].Player->pos);
	pc.z = 0.0f;
	v3Normalise(&pc);
	v3Normalise(&pf);
	ang = acos(v3Dot(&pc, &pf));
	if (ang < HPI)
	{
		PlayerControlJoyX = RANGE(-1.0, (ip.x / ip.y) * 3.0f, 1.0);
	}
	else
	{
		if (ip.x < 0.0f)
			PlayerControlJoyX = -1.0f;
		else
			PlayerControlJoyX = 1.0f;
	}
}

int	PlayerSeePlayerCheckPosXY(float ang)
{
	Vector3	a, b;

	a.x = player[currentPlayer].Player->m.m[1][0];
	a.y = player[currentPlayer].Player->m.m[1][1];
	a.z = 0.0f;

	v3Sub(&b, &player[currentPlayer].Player->CheckPos, &player[currentPlayer].Player->pos);
	b.z = 0.0f;
	v3Normalise(&a);
	v3Normalise(&b);
	if (acos(v3Dot(&a, &b)) <= ang)
		return 1;
	else
		return 0;
}

void PlayerObjectRelRotateXYZ(int i, float x, float y, float z)
{
	if ((!player[currentPlayer].Player) || (!player[currentPlayer].Player->pnode))
		return;
	player[currentPlayer].Player->objId[i]->idleRot.x += x;
	player[currentPlayer].Player->objId[i]->idleRot.y += y;
	player[currentPlayer].Player->objId[i]->idleRot.z += z;
}

void PlayerRelRotateXYZ(float x, float y, float z)
{
	mPreRotateXYZ(&player[currentPlayer].Player->m, x, y, z);
}

void HoldPlayerRelRotateXYZ(STRAT *strat, float x, float y, float z)
{
	int n;

	mPreRotateXYZ(&strat->m, x, y, z);
}

void FlattenHoldPlayer(STRAT *strat, float amount)
{
	Vector3	force, stratUp, oldVel;
	Point3	p;
	int	n;

	if (!strat->hpb)
		return;

	if (PAL)
		amount *= PAL_MOVESCALE;

	stratUp.x = strat->m.m[2][0];
	stratUp.y = strat->m.m[2][1];
	stratUp.z = strat->m.m[2][2];
	v3Normalise(&stratUp);
	p.x = stratUp.x * strat->pnode->radius * 2.0f;
	p.y = stratUp.y * strat->pnode->radius * 2.0f;
	p.z = stratUp.z * strat->pnode->radius * 2.0f;
	force.x = force.y = 0.0f;
	force.z = strat->pnode->mass * 0.01f * amount;
	oldVel = strat->vel;
	ApplyForce(strat, &force, &p, 0.1f);
	strat->vel = oldVel;
}

//SETS POSITION RELATIVE TO A PLAYER OBJECT
//IF OBJID IS 0, WILL SET RELATIVE TO THE PLAYER

void SetPosRelOffsetPlayerObject(STRAT *strat, int pn, int objId, float x, float y, float z)
{
	Matrix m;
	Object *obj;
	Point3	p,p2;

	p.x = x;
	p.y = y;
	p.z = z;

	mUnit(&m);

	if (objId)
	{

		obj = player[pn].Player->objId[objId];
		while(obj)
		{
			mPostRotateXYZ(&m, obj->crntRot.x, obj->crntRot.y, obj->crntRot.z);
			obj = obj->parent;
		}
   	
		
		mPostMultiply(&m, &player[pn].Player->m);
		mPoint3Apply3(&p, &m);
		strat->pos = p;
		v3Inc(&strat->pos, &player[pn].Player->pos);

	}
	else
	{
		mVector3Multiply(&p2,&p,&player[pn].Player->m);
		p2.x += player[pn].Player->pos.x;
		p2.y += player[pn].Player->pos.y;
		p2.z += player[pn].Player->pos.z;
		strat->pos = p2;


	}




}
//SETS CHECK POSITION RELATIVE TO A PLAYER OBJECT
//IF OBJID IS 0, WILL SET RELATIVE TO THE PLAYER

void SetCheckPosRelOffsetPlayerObject(STRAT *strat, int objId, float x, float y, float z)
{
	Matrix m;
	Object *obj;
	Point3	p,p2;

	p.x = x;
	p.y = y;
	p.z = z;

	mUnit(&m);

	if (objId)
	{

		obj = player[currentPlayer].Player->objId[objId];
		while(obj)
		{
			mPostRotateXYZ(&m, obj->crntRot.x, obj->crntRot.y, obj->crntRot.z);
			obj = obj->parent;
		}
		
		mPostMultiply(&m, &player[currentPlayer].Player->m);
		mPoint3Apply3(&p, &m);
		strat->CheckPos = p;
		v3Inc(&strat->CheckPos, &player[currentPlayer].Player->pos);
	}
	else
	{
		mVector3Multiply(&p2,&p,&player[currentPlayer].Player->m);
		p2.x += player[currentPlayer].Player->pos.x;
		p2.y += player[currentPlayer].Player->pos.y;
		p2.z += player[currentPlayer].Player->pos.z;
		strat->CheckPos = p2;


	}
}
void RelPosToCheck(STRAT *strat, int objId, float x, float y, float z)
{
	Matrix m;
	Matrix im;
	Point3	p;

	p.x = x;
	p.y = y;
	p.z = z;

	if (objId == 0)
	{	
		v3Dec(&p, &strat->pos);

		mInvertRot(&strat->m, &im);
		mPoint3Apply3(&p, &im);
		strat->CheckPos = p;
	}
	else
	{
		fromObjectToWorld(&m, strat, strat->objId[objId], MATRIX_RST);
		mInvert3d(&im, &m);
		mPoint3Apply(&p, &im);
		strat->CheckPos = p;
	}
}

//brings a point into strat's space
void WorldPosToCheck(STRAT *strat, int objId, float x, float y, float z)
{
	Matrix m;
	Matrix im;
	Point3	p;
	Point3  sp;

	p.x = x;
	p.y = y;
	p.z = z;

	sp = strat->pos;


	if (objId == 0)
	{	
		v3Dec(&sp, &p);

		sp.x = x - strat->pos.x;
		sp.y = y - strat->pos.y;
		sp.z = z - strat->pos.z;
					 
		mInvertRot(&strat->m, &im);
		mPoint3Apply3(&sp, &im);
		strat->CheckPos = sp;
	}
}




void IAmJumpPoint(STRAT *strat)
{
	int i;

	for (i=0; i<5; i++)
	{
		if (strat->actindex[i] >= 0)
		{
			jumpPoint[noJumpPoints].pos = MapActs[strat->actindex[i]].pos;
			jumpPoint[noJumpPoints].radius = MapActs[strat->actindex[i]].radius;
			noJumpPoints++;
			dAssert(noJumpPoints < 32, "Too many jump Points");
		}
	}
}

void UnitiseMatrix(STRAT *strat)
{
	Vector3	x, y, z;

	if (fabs(strat->m.m[1][2]) > 0.95f)
		pointToXZ(strat, &player[currentPlayer].CameraStrat->pos);
	z.x = z.y = 0.0f;
	z.z = 1.0f;
	y.x = strat->m.m[1][0];
	y.y = strat->m.m[1][1];
	y.z = 0.0f;
	v3Normalise(&y);
	v3Cross(&x, &y, &z);
	mUnit(&strat->m);
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




float smoothSideStep(int pn)
{
	float	f = ((float)(player[pn].playerSideStepHold) / SKID_FRAMES);
	
	f = ((sin(2*PI*f-HPI)+1) / SKID_FRAMES) * HPI;
	return f;
}

float smoothSideStepSum(int pn)
{
	float	f, r = 0.0f;
	int	i;
	
	for (i=0; i<player[pn].playerSideStepHold; i++)
	{
		f = ((float)(i) / SKID_FRAMES);
		r += ((sin(2*PI*f-HPI)+1) / SKID_FRAMES) * HPI;
	}

	return (r);
}

float SmoothFromTo(float r, float i, float n)
{
	float	f = i/n;

	if (i > n)
		return 0.0f;

	f = ((sin(PI2*f-HPI)+1.0) / n) * r;
	return f;
}

float SmoothFromToSum(float range, float t, float n)
{
	float	f, r = 0.0f;
	float	i=0.0;

	for (i=0.0; i<t; i++)
	{
		f = i / n;
		r += ((sin(2*PI*f-HPI)+1.0) / n) * range;
	}

	return (r);
}

int	BattleTankInActivation(STRAT *strat, int n)
{
	STRAT *s;
	Point3	p1, p2;
	float dist;

	dist = MapActs[strat->actindex[n]].radius;
	p1 = MapActs[strat->actindex[n]].pos;
	p1.z = 0.0f;
	s = FirstStrat;

	while(s)
	{
		if (s->pnode)
		{
			if (s->pnode->typeId == 16)
			{
				p2 = s->pos;
				p2.z = 0.0f;
				if (pDist(&p1, &p2) <= dist)
					return 1;
			}
		}

		s = s->next;
	}
	return 0;
}



void MyStratToWorld(STRAT *strat, float x, float y, float z)
{
	Point3 p;

	p.x = x;
	p.y = y;
	p.z = z;

	mPoint3Apply3(&p, &strat->m);
	v3Inc(&p, &strat->pos);
	strat->CheckPos = p;
}


static Matrix	WaterSlideMatrix;

float GetWaterSlideRelRotZ(STRAT *strat)
{

	Vector3	v, w, vXw;

	v.x = strat->m.m[1][0];
	v.y = strat->m.m[1][1];
	v.z = 0.0f;

	w.x = WaterSlideMatrix.m[1][0];
	w.y = WaterSlideMatrix.m[1][1];
	w.z = 0.0f;

	v3Normalise(&v);
	v3Normalise(&w);

	v3Cross(&vXw, &v, &w);
	WaterSlideMatrix = strat->m;

	if (vXw.z < 0.0f)
		return acos(v3Dot(&v, &w));
	else
		return -acos(v3Dot(&v, &w));
}

void FaceAlongDir(STRAT *strat)
{
	Vector3	Y, up, X, Z;

	if ((strat->pos.x == strat->oldPos.x) &&
		(strat->pos.y == strat->oldPos.y) &&
		(strat->pos.z == strat->oldPos.z))
		return;

	v3Sub(&Y, &strat->pos, &strat->oldPos);
	v3Normalise(&Y);
	if (fabs(Y.z) > 0.999f)
	{
		if (Y.z > 0.999f)
		{
			mUnit(&strat->m);
			RelRotateX(strat, HPI);
		}
		else
		{
			mUnit(&strat->m);
			RelRotateX(strat, -HPI);
		}
		return;
	}
	up.x = 0.0f;
	up.y = 0.0f;
	up.z = 1.0f;

	v3Cross(&X, &Y, &up);
	v3Cross(&Z, &X, &Y);

	mUnit(&strat->m);
	strat->m.m[0][0] = X.x;
	strat->m.m[0][1] = X.y;
	strat->m.m[0][2] = X.z;

	strat->m.m[1][0] = Y.x;
	strat->m.m[1][1] = Y.y;
	strat->m.m[1][2] = Y.z;

	strat->m.m[2][0] = Z.x;
	strat->m.m[2][1] = Z.y;
	strat->m.m[2][2] = Z.z;
}

float StratSpeed(STRAT *strat)
{
	return pDist(&strat->pos, &strat->oldPos);
}

#if 0
int stratQNAN(STRAT *strat)
{
	if ((strat->vel.x < 100.0f) && (strat->relVel.x < 100.0f) && (strat->pos.x < 1000000.0f) && (strat->m.m[0][0] < 100.0f))
		return 0;
	else
	{
		dAssert(0, "oops strat QNAN");
		return 1;
	}
}

void anyStratQNAN(void)
{

	STRAT *strat;

	strat = FirstStrat;

	while(strat)
	{
		if ((strat->vel.x < 100.0f) && (strat->relVel.x < 100.0f) && (strat->pos.x < 1000000.0f) && (strat->m.m[0][0] < 100.0f))
		{
		}
		else
		{
			dAssert(0, "oops strat QNAN");
			return;
		}
		strat = strat->next;
	}
}
#endif

int	GetFireSecondary(int pn)
{
	return player[pn].fireSecondary;
}

void SetFireSecondary(int pn, int n)
{
	player[pn].fireSecondary = n;
}

void ClearPlayerTargetArray(int pn)
{
	int	i;

	for (i=0; i<MAX_TARGETS; i++)
	{
		player[pn].target[i].strat = NULL;
		player[pn].target[i].obj = NULL;
		player[pn].target[i].n = 0;
		player[pn].target[i].locked = 0;
	}
	player[pn].playerTargetTime = 0;
	player[pn].playerTargetNumber = -1;
}

int	ValidTargetStrat(STRAT *strat, int pn, int sn, int objId)
{
	int i;
	STRAT *s;

	s = (STRAT *)sn;

	if (objId == 0)
	{
		if (!(s->flag))
			return 0;

		if (s->health > 0.0f)
			return 1;
		else
			return 0;
	}
	else
	{
		if (!(s->flag))
			return 0;

		//sav I've had to modify this, pleae let me know if you need to change it back MP
		if ((s->objId) && (s->objId[objId]->health > 0.0)  && (!(s->objId[objId]->collFlag & OBJECT_INVISIBLE)))
		  //	&& (s->objId[objId]->collFlag & COLL_TARGETABLE))
			return 1;
		else
			return 0;
	}
}


void RemoveMeFromAllTargets(STRAT *strat)
{
	int i, pn;

	for (pn=0; pn<4; pn++)
	{
		if (strat->Player->n == pn)
			continue;

		if (player[pn].playerTargetNumber == strat->Player->n)
		{
			player[pn].playerTargetNumber = -1;
			player[pn].playerTargetTime = 0;
		}


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
	strat->Player->playerTargettedbyMissile = 0;
	strat->Player->lockedOnTime[0] = 0;
	strat->Player->lockedOnTime[1] = 0;
	strat->Player->lockedOnTime[2] = 0;
	strat->Player->lockedOnTime[3] = 0;
}

static int lineAllStratColl(Point3 *p1, Point3 *p2, Point3 *p, int pn)
{
	STRAT	*strat, *closestStrat;
	float	shortestDist = 1500.0f, dist;
	Point3	ps, pe;
	Object	*objNear;
	COLLSTRAT	*coll;

	ps = *p1;
	pe = *p2;

	coll = FirstColl;

	while (coll)
	{
		strat = coll->strat;

		if ((pn >= 0) && (strat->Player))
		{
			coll = coll->next;
			continue;
		}
		if (!(strat->flag & STRAT_BULLET))
		{		
			if (strat->flag & STRAT_COLLSTRAT)
			{
				objNear = lineStratCollision(&ps, &pe, strat, pn);
				if (objNear)
				{
					dist = pDist(&strat->pos, &ps);
					if (dist < shortestDist)
					{
						*p = strat->pos;
						shortestDist = dist;
					}
				}
			}
		}
		coll = coll->next;
	}

	if (shortestDist == 1500.0f)
		return 0;
	else
		return 1;
}

static int	obscuredByStrat(Point3 *p1, Point3 *p2, Point3 *p, int pn)
{
	return (lineAllStratColl(p1, p2, p, pn));
}

static int	obscuredByScape(Point3 *p1, Point3 *p2, Point3 *p, STRAT *strat)
{
	return lineLandscapeCollision(p1, p2, p, strat, NULL);
}

//DETECTS THE INTERSECTION OF A STRAT OF POS AND LENGTH, WITH THE LANDSCAPE
//MODE 1 - SCALES THE BEAM TO THE POINT OF INTERSECTION ALONG THE FORWARD AXIS
//		 - WORKS ASSUMING A UNIT MODEL SCALE IN THE FORWARD AXIS 
int ValidToBeam(STRAT *strat, float x, float BeamLength, float z, int mode)
{
	Point3 collPt;
	Point3 pos,oldpos;
	Matrix temp;
	float dist;
	//static int testbed=0;

									  
//	TestDraw(strat);
	temp= strat->m;
		
   	mPreScale(&temp,strat->scale.x,strat->scale.y,strat->scale.z);


   	mPreTranslate(&temp,0,BeamLength,0);


   	pos = strat->pos;
  	pos.x += temp.m[3][0];
  	pos.y += temp.m[3][1];
  	pos.z += temp.m[3][2];

	
  /*	return(!(obscured(&strat->pos,&pos,&collPt,0,strat)));
	
  	  */
	
	if (obscuredByStrat(&strat->pos,&pos,&collPt,0))
	{

		//ensure we are not hitting any strats we are spawned from
	 	if (strat->parent)
		{
			if ((collPt.x != strat->parent->pos.x) ||
				(collPt.y != strat->parent->pos.y) ||
				(collPt.z != strat->parent->pos.z) )
			{
				if (mode)
				{
					dist = pDist(&strat->pos,&collPt);
					strat->scale.y = dist;
				}
				return(0);
			}
		}
		else
		{
			if (mode)
			{
				dist = pDist(&strat->pos,&collPt);
				strat->scale.y = dist;
			}
			return(0);
	
		}	
	}
	//just scape checks less beam is inside its targetable parent
	if (obscuredByScape (&strat->pos, &pos, &collPt, strat)) 
	{
		if (mode)
		{
			dist = pDist(&strat->pos,&collPt);
			strat->scale.y = dist;
		}
		return(0);
	}
	else
		return(1);
  	
}



int obscured(Point3 *p1, Point3 *p2, Point3 *p, int pn, STRAT *strat)
{
   	if (obscuredByStrat(p1, p2, p, pn))
	{
		if (obscuredByScape(p1, p, p, strat))
			return 1;
		else
			return 1;
	}
	else if (obscuredByScape(p1, p2, p, strat))
		return 1;

	return 0;
}

float HealthOfPlayer(int pn)
{
	dAssert (pn >=0 && pn < 4, "Player out of range");
	return player[pn].Player->health;
}
float MaxHealthOfPlayer(int pn)
{
	dAssert (pn >=0 && pn < 4, "Player out of range");
	return player[pn].maxHealth;
}


void TargettingStrat(int stratNumber, int a)
{
	STRAT *strat;

	strat = (STRAT *)stratNumber;
	if (strat->Player)
		strat->Player->playerTargettedbyMissile = a;
}

static void stratPointToP(STRAT *strat, Point3 *p)
{
	Vector3	up, stratForward, stratRight, stratUp;

	up.x = 0.0f;
	up.y = 0.0f;
	up.z = 1.0f;

	v3Sub(&stratForward, p, &strat->pos);
	v3Normalise(&stratForward);
	v3Cross(&stratRight, &stratForward, &up);
	v3Cross(&stratUp, &stratRight, &stratForward);

	mUnit(&strat->m);
	strat->m.m[0][0] = stratRight.x;
	strat->m.m[0][1] = stratRight.y;
	strat->m.m[0][2] = stratRight.z;
	strat->m.m[1][0] = stratForward.x;
	strat->m.m[1][1] = stratForward.y;
	strat->m.m[1][2] = stratForward.z;
	strat->m.m[2][0] = stratUp.x;
	strat->m.m[2][1] = stratUp.y;
	strat->m.m[2][2] = stratUp.z;
}

void ActivateObjectCollision(STRAT *strat, int objId)
{
  //	dAssert(strat->objId[objId], "oops");
	if (!objId)
	{

		strat->obj->collFlag &= ~OBJECT_NOCOLLISION;
		strat->obj->collFlag &= ~(COLL_ON_FLOOR + COLL_HITSTRAT);
	}
	else
	{
		if (strat->objId[objId])
		{
			strat->objId[objId]->collFlag &= ~OBJECT_NOCOLLISION;
			strat->objId[objId]->collFlag &= ~(COLL_ON_FLOOR + COLL_HITSTRAT);
	   
		
		}
	}
	//ensure our coll point info is updated
	ResetOCP(strat);
}

void InActivateObjectCollision(STRAT *strat, int objId)
{
 //	dAssert(strat->objId[objId], "oops");
	if ((!strat) || (!(strat->pnode)) || (objId > strat->pnode->noObjId))
		return;

	if (!objId)
	{
		if (strat->obj)
			strat->obj->collFlag |= OBJECT_NOCOLLISION;
	}
	else
	{
		if (strat->objId[objId])
			strat->objId[objId]->collFlag |= OBJECT_NOCOLLISION;
	}
}
void InActivateObjectDamageable(STRAT *strat, int objId)
{
 //	dAssert(strat->objId[objId], "oops");
	if (!objId)
		strat->obj->collFlag |= OBJECT_NOTVALIDTOHURT;
	else
		strat->objId[objId]->collFlag |= OBJECT_NOTVALIDTOHURT;
}


void FacePlayerAim(STRAT *strat, int pn)
{
	Point3	p;

	p = player[pn].aimedPoint;
	stratPointToP(strat, &p);
}

STRAT *PlayerStrat(int pn)
{
	return player[pn].Player;
}

void RandomPointToCam(STRAT *strat)
{
	Point3	p;
	Vector3	v;

	p = player[0].CameraStrat->pos;

	v3Rand(&v);
	v3Normalise(&v);
	v3Inc(&p, &v);
	pointToXZ(strat, &p);
}


void SetObjScale(STRAT *strat, int objId, float x, float y, float z)
{
	strat->objId[objId]->scale.x = x;
	strat->objId[objId]->scale.y = y;
	strat->objId[objId]->scale.z = z;
}



float CheckPosDist(STRAT *strat)
{
	return pDist(&strat->pos, &strat->CheckPos);
}

void DrawShock2(float x1, float y1, float z1, float x2, float y2, float z2, float noise, float ss, float es, int madness)
{
	Point3 p1, p2;
	Colour startcol, endcol;
	Colour ostartcol, oendcol;
	float	alpha, red, green, blue;
	int		a, r, g, b;
	int		ai, ri, gi, bi;

	if (GetGlobalParamInt(0))
	{
		a	= ((int)(GetGlobalParamFloat(0) * 255.0f))<<24;
		r	= ((int)(GetGlobalParamFloat(1) * 255.0f))<<16;
		g	= ((int)(GetGlobalParamFloat(2) * 255.0f))<<8;
		b	= ((int)(GetGlobalParamFloat(3) * 255.0f));

		ai	= ((int)(GetGlobalParamFloat(4) * 255.0f))<<24;
		ri	= ((int)(GetGlobalParamFloat(5) * 255.0f))<<16;
		gi	= ((int)(GetGlobalParamFloat(6) * 255.0f))<<8;
		bi	= ((int)(GetGlobalParamFloat(7) * 255.0f));

	   	startcol.colour = ((0xcf << 24)|r|g|b);
		endcol.colour = ((0xaf << 24)|r|g|b);
		ostartcol.colour = ((0x40 << 24)|ri|gi|bi);
		oendcol.colour = ((0x3f << 24)|ri|gi|bi);
	}
	else
	{
		startcol.colour = 0xcf9090ff;
		endcol.colour = 0xaf3030b0;
		ostartcol.colour = 0x409090ff;
		oendcol.colour = 0x3f3030b0;
	}

	p1.x = x1;
	p1.y = y1;
	p1.z = z1;

	p2.x = x2;
	p2.y = y2;
	p2.z = z2;

	rDrawLightning (&p1, &p2, startcol, endcol, ostartcol, oendcol, 3, noise, ss, es, madness);
}

int	ObjectShotBySibling(STRAT *strat)
{
	int	i;

	
   //	if (strat->pnode->noObjId)
   //	{
	   //	for (i=1; i<strat->pnode->noObjId + 1; i++)
		for (i=strat->pnode->noObjId; i > 0; i--)
		{
		 	if (strat->objId[i])
			{
				if (strat->objId[i]->collFlag & OBJECT_SHOT_BY_SIBLING)
					return i;
			}
	 //	else
	 //		return 0;
		}
	//}
	return 0;
}

float GetGlobalParamFloat(int n)
{
	return GlobalParamFloat[n];
}

int GetGlobalParamInt(int n)
{
	return GlobalParamInt[n];
}

void SetGlobalParamFloat(int n, float f)
{
	GlobalParamFloat[n] = f;
}

void SetGlobalParamInt(int n, int i)
{
	GlobalParamInt[n] = i;
}

void ClearGlobalParams(void)
{
	int i;

	for (i=0; i<8; i++)
	{
		GlobalParamInt[i] = 0;
		GlobalParamFloat[i] = 0.0f;
	}
}

void IAmPlayerRDHM(STRAT *strat, int pn)
{
	player[pn].rdhm = strat;
}

int	GetSem(STRAT *strat, int n)
{
	Uint32	flag;
	int *x;

	if (!Multiplayer)
	{
		if (player[0].Player->health <= 0.0f)
			return 0;
	}

	if (!strat)
		return 0;

	flag = 63<<(n*6);
	x = (int *)(&strat->var);
	return ((*x) & flag)>>(n*6);
}

void SetSem(STRAT *strat, int n, int m)
{
	int *x;

	if (!strat)
		return;

	if (!Multiplayer)
	{
		if ((strat == player[0].Player) && (player[0].Player->health <= 0.0f))
			return;
	}


	x = (int *)(&strat->var);

	dAssert((m>=0) && (m < 64), "Oops");

	*x &= ~(63<<(n*6));
	*x |= (m<<(n*6));
}

int	GetParentSem(STRAT *strat, int n)
{
	Uint32	flag;
	int	*val;

	
	if (!strat)
		return 0;

	if (!(strat->parent))
		return 0;

	if (Multiplayer)
	{
		if ((strat->parent->Player) && (strat->parent->health <= 0.0f))
			return 0;
	}
	else
	{
		if (strat->parent && strat->parent->Player && (strat->parent->health <= 0.0))
			return 0;
	}


	flag = 63<<(n*6);

	val = (int *)(&strat->parent->var);
	return ((*val) & flag)>>(n*6);
}

void SetParentSem(STRAT *strat, int n, int m)
{
	int *x;

	if (!strat)
		return;

	if (!(strat->parent))
		return;

	if (Multiplayer)
	{
		if ((strat->parent->Player) && (strat->parent->health <= 0.0f))
			return;
	}
	else
	{
		if ((strat->parent->Player) && (player[0].Player->health <= 0.0f))
			return;
	}


	x = (int *)(&strat->parent->var);

	dAssert((m>=0) && (m < 64), "Oops");
	*x &= ~(63<<(n*6));
	*x |= (m<<(n*6));
}

void AddSem(STRAT *strat, int n, int m)
{
	int a;

	a = GetSem(strat, n);
	a += m;

	dAssert((a>=0) && (a<64), "oops");

	SetSem(strat, n, a);
}

void SubSem(STRAT *strat, int n, int m)
{
	int a;

	a = GetSem(strat, n);
	a -= m;

	dAssert((a>=0) && (a<64), "oops");

	SetSem(strat, n, a);
}

void AddParentSem(STRAT *strat, int n, int m)
{
	int a;

	if (Multiplayer)
	{
		if ((strat->parent) && (strat->parent->Player) && (strat->parent->health <= 0.0f))
			return;
	}
	else
	{
		if ((strat->parent) && (strat->parent->Player) && (player[0].Player->health <= 0.0f))
			return;
	}
 
	a = GetParentSem(strat, n);
	a += m;

	dAssert((a>=0) && (a<64), "oops");

	SetParentSem(strat, n, a);
}

void SubParentSem(STRAT *strat, int n, int m)
{
	int a;

	if (Multiplayer)
	{
		if ((strat->parent) && (strat->parent->Player) && (strat->parent->health <= 0.0f))
			return;
	}
	else
	{
		if ((strat->parent) && (strat->parent->Player) && (player[0].Player->health <= 0.0f))
			return;
	}

	a = GetParentSem(strat, n);
	if (a-m >= 0)
	{
		a -= m;
		SetParentSem(strat, n, a);
	}
}

void PNum(int x, int y, float val)
{
	tPrintf (x, y, "%f", val);
}

float GetObjectHealth(STRAT *strat, int objId)
{
	return strat->objId[objId]->health;
}

void SetObjectHealth(STRAT *strat, int objId, float amount)
{
	strat->objId[objId]->health = amount;
}

void PointToXY(STRAT *crntStrat, float x, float y, float z)
{
	Point3 p;

	p.x = x;
	p.y = y;
	p.z = z;

	pointToXY(crntStrat, &p);
}

void ThonGravity(int pn, float amount)
{
	player[pn].ThonGravity = amount;
}

static int cameraSeePos(Point3 *p, float ang, int pn)
{
	Vector3 camToP, camDir;

	camDir.x = player[pn].CameraStrat->m.m[1][0];
	camDir.y = player[pn].CameraStrat->m.m[1][1];
	camDir.z = player[pn].CameraStrat->m.m[1][2];
	v3Sub(&camToP, p, &player[pn].CameraStrat->pos);
	v3Normalise(&camToP);
	if (acos(v3Dot(&camToP, &camDir)) < ang)
		return 1;

	return 0;
}

void RedDogElectroShock(STRAT *strat)
{
	COLLSTRAT		*coll;
	STRAT			*s;
	Point3			p, tempp;
	Point3			esArray[64];
	STRAT			*sArray[64];
	int				oArray[64];
	int				nes, i;

	coll = FirstColl;
	nes = 0;

	MyStratToWorld(strat, 0.0f, 1.5f, 0.0f);
	while ((coll) && (nes < 64))
	{
		s = coll->strat;
		if ((s->flag & STRAT_COLLSTRAT) && (s->flag & STRAT_ENEMY) && !(s->flag & STRAT_BULLET) && !(s->flag2 & STRAT2_PICKUP))
		{
			if (s->health <= 0.0f)
			{
				coll = coll->next;
				continue;
			}

			if (pSqrDist(&strat->pos, &s->pos) > SQR(100.0f))
			{
				coll = coll->next;
				continue;
			}

			p = s->pos;
			if (cameraSeePos(&p, 0.4, 0))
			{
				if (obscuredByScape(&player[0].CameraStrat->pos, &p, &tempp, s))
				{
					coll = coll->next;
					continue;
				}

				if (obscuredByScape(&strat->pos, &p, &tempp, s))
				{
					coll = coll->next;
					continue;
				}
				

				if (s->flag & COLL_TARGETABLE)
				{
					if (nes < 64)
					{
						esArray[nes] = s->pos;
						sArray[nes] = s;
						oArray[nes] = 0;
						nes++;
					}
					else
						break;
				}
				else
				{
					for (i = 1; i <= s->pnode->noObjId; i++)
					{
						if ((s->objId[i]->collFlag & COLL_TARGETABLE) && (nes < 64))
						{
							esArray[nes].x = s->objId[i]->wm.m[3][0];
							esArray[nes].y = s->objId[i]->wm.m[3][1];
							esArray[nes].z = s->objId[i]->wm.m[3][2];
							sArray[nes] = s;
							oArray[nes] = i;
							nes++;
						}
						if (nes >= 64)
							break;
					}
				}
				
			}
		}

	   //	if (nes > 64)
	   //		CrashOut("OUT OF TRIGGERS", 0xffffffff, 0);


		coll = coll->next;
	}

	if (nes == 0)
		return;

	i = (int)RandRange(0.0, (float)(nes));

	if (i >= 63)
		i = 63;

	DrawShock2(strat->CheckPos.x, strat->CheckPos.y, strat->CheckPos.z, esArray[i].x, esArray[i].y, esArray[i].z, 0.2, 2.0, 2.0, 4);
	if (oArray[i])
	{
		sArray[i]->objId[oArray[i]]->health -= 3;
		sArray[i]->health -= 3.0f;
	}
	else
	{
		sArray[i]->health -= 3;
	}
	player[0].PlayerSecondaryWeapon.amount--;
}

void RedDogElectroShockMultiPlayer(STRAT *strat)
{
	Point3	p, tempp;
	float	tempf;
	int		i, n;


	MyStratToWorld(strat, 0.0f, 1.5f, 0.0f);
	p = strat->CheckPos;
	n = strat->Player->n;
	for (i=0; i<NumberOfPlayers; i++)
	{
		if (player[i].Player == NULL || (i == strat->Player->n) || (player[i].Player->health <= 0.0))
			continue;
		
		if (cameraSeePos(&player[i].Player->pos, 0.4, strat->Player->n))
		{
			if (pSqrDist(&p, &player[i].Player->pos) > 2500.0f)
				continue;

			if (obscured(&p, &player[i].Player->pos, &tempp, n, NULL))
				continue;

			n = i;
			addShockToShockArray(&player[i].Player->pos, &p);
			strat->damage = 2.0f;
			tempf = strat->var;
			strat->var = strat->Player->n;
			HurtStrat(player[i].Player, strat);
			strat->var = tempf;
			strat->damage = 0.0f;
			p = player[i].Player->pos;
		}
	}

	if (n == strat->Player->n)
	{
		tempp.x = p.x;
		tempp.y = p.y;
		tempp.z = p.z + 1.0;
		addShockToShockArray(&tempp, &p);
	}
	strat->Player->PlayerSecondaryWeapon.amount--;
}

void GetPlayerPos(STRAT *strat, int pn)
{
	if (player[pn].Player)
	{
		strat->CheckPos = player[pn].Player->pos;
	}
	else
	{
		strat->CheckPos.x = 0.0f;
		strat->CheckPos.y = 0.0f;
		strat->CheckPos.z = 0.0f;
	}
}

void SetHealthOfPlayer(int pn, float amount)
{
	if (player[pn].Player)
		player[pn].Player->health = amount;
}

void SetPlayerZoom(STRAT *strat, int pn, float amount)
{
	player[pn].zooming = amount;
}

float GetPlayerZoom(STRAT *strat, int pn)
{
	return player[pn].zooming;
}


void SetPlayerBoostButtonCount(int pn, int i)
{
	player[pn].playerBoostButtonCount = i;
}

float PlayerSpeed(int pn)
{
	if (player[pn].Player)
		return (sqrt(v3Dot(&player[pn].Player->vel, &player[pn].Player->vel)));
	else
		return 0.0f;
}

void SetPlayerAcc(int pn, float amount)
{
	player[pn].PlayerAcc = amount;
}

float DistToAim(STRAT *strat, int pn)
{
	return pDist(&strat->pos, &player[pn].aimedPoint);
}

int		noHits = 0;
STRAT	*hitStratArray[16];
Object	*hitObjectArray[16];
STRAT	*hitCollWithArray[16];

void DamageAim(STRAT *strat, int pn, float amount)
{
	float	tempf, tempf2, length;
	int id = 0;


	length = GetGlobalParamFloat(0);
	if (player[pn].aimStrat)
	{
		if (!Multiplayer)
			if ((player[pn].aimStrat == EscortTanker) || (player[pn].aimStrat->flag & STRAT_FRIENDLY))
				return;

		tempf = strat->damage;
		tempf2 = strat->var;
		strat->damage = amount;
		strat->var = pn;
		HurtStrat(player[pn].aimStrat, strat);
		if (player[pn].aimObj)
			player[pn].aimObj->health -= amount;

		hitStratArray[noHits] = player[pn].aimStrat;
		hitCollWithArray[noHits] = strat;

		if (player[pn].aimObj)
			hitObjectArray[noHits] = player[pn].aimObj;
		else
			hitObjectArray[noHits] = NULL;


		strat->var = tempf2;
		strat->damage = tempf;
		noHits++;
	} else {
		Vector3 v;
		extern void rMakeDecal (Point3 *cPoint, Vector3 *faceNormal, Vector3 *vVector, float xSize, float ySize, Sint16 mat, float r, float g, float b);
		v3Sub (&v, &player[pn].aimedPoint, &strat->pos);
		v3Normalise (&v);
		if (length < 250.0f)
		{
			if (GET_SURF_TYPE(player[pn].aimedPolyType) != NODECAL)
			{
				if (GlobalParamInt[0])
					rMakeDecal (&player[pn].aimedPoint, &player[pn].aimedNormal, &v, 0.25, 0.25, 0, 0.8, 0.8, 0);
				else
					rMakeDecal (&player[pn].aimedPoint, &player[pn].aimedNormal, &v, 1, 1, 0, 0.8, 0.8, 0);
			}
		}
	}
}

void SetPlayerArmageddon(int pn, int amount)
{
	player[pn].PlayerArmageddon = amount;
}

int GetPlayerArmageddon(int pn)
{
	return player[pn].PlayerArmageddon;
}

int GetSecondaryWeaponAmount(int pn)
{
	return player[pn].PlayerSecondaryWeapon.amount;
}

void SetSecondaryWeaponAmount(int pn, int amount)
{
	player[pn].PlayerSecondaryWeapon.amount = amount;
}

void SetSecondaryWeaponType(int pn, int type)
{
	player[pn].PlayerSecondaryWeapon.type = type;
}

int GetSecondaryWeaponType(int pn)
{
	return player[pn].PlayerSecondaryWeapon.type;
}

int	GetPlayerNumber(STRAT *strat)
{
	if (strat)
	{
		if (strat->Player)
			return strat->Player->n;
		else
			return -1;
	}
	else
		return -1;
}

void SmartBombExplode(STRAT *strat, int pn, float range, float damage)
{
	COLLSTRAT		*coll;
	STRAT *s;

	coll = FirstColl;

	while (coll)
	{
		s = coll->strat;
		if ((s->flag & STRAT_COLLSTRAT) && (s->flag & STRAT_ENEMY) && !(s->flag & STRAT_BULLET))
		{
			if (pSqrDist(&strat->pos, &s->pos) < range * range)
				s->health -= damage;
		}
		coll = coll->next;
	}
}

void ClearSwarmTargetArray(void)
{
	int i;

	for (i=0; i<20; i++)
	{
		swarmStratTargetArray[i] = NULL;
		swarmObjectTargetArray[i] = 0;
	}

	noSwarmTargets = 0;
}

void AddStratToSwarmTargetArray(STRAT *strat, float range)
{
	int i, nid;
	COLLSTRAT		*coll;
	STRAT *s;

	ClearSwarmTargetArray();
	coll = FirstColl;

	while (coll)
	{
		s = coll->strat;
		if ((s->flag & STRAT_COLLSTRAT) && (s->flag & STRAT_ENEMY) && !(s->flag & STRAT_BULLET))
		{
			if (pSqrDist(&strat->pos, &s->pos) > range * range)
			{
				coll = coll->next;
				continue;
			}

			if (s->flag & COLL_TARGETABLE)
			{
				swarmStratTargetArray[noSwarmTargets] = s;
				swarmObjectTargetArray[noSwarmTargets] = 0;
				noSwarmTargets++;
			}
			else
			{
				if (s->pnode)
				{
					for (i=1; i<s->pnode->noObjId + 1; i++)
					{
						if (s->objId[i])
						{
							if (s->objId[i]->collFlag & COLL_TARGETABLE)
							{
								if (s->objId[i]->health > 0.0f)
								{
									swarmStratTargetArray[noSwarmTargets] = s;
									swarmObjectTargetArray[noSwarmTargets] = i;
									noSwarmTargets++;
								}
							}
						}

						if (noSwarmTargets == 20)
							break;
					}
				}
			}
		}
		coll = coll->next;
		if (noSwarmTargets == 20)
			break;
	}
}

int	GetSwarmTarget(STRAT *strat, int i)
{
	STRAT	*s;
	int		obj = 0;	

	s = swarmStratTargetArray[i % noSwarmTargets];
	obj = swarmObjectTargetArray[i % noSwarmTargets];

	SetSem(strat, 0, obj);

	

	return (int)s;
}

int	ValidSwarmTarget(STRAT *strat, int st)
{
	STRAT *s;
	int obj = 0;

	obj = GetSem(strat, 0);
	s = (STRAT *)st;

	if (obj)
	{
		if (s->objId[obj]->health > 0.0f)
		{
			if (s->flag)
				return 1;
			else
				return 0;
		}
		else
			return 0;
	}
	else if ((s->flag) && !(s->flag & STRAT_HIDDEN))
		return 1;
	
	return 0;
}

void GetSwarmTargetPos(STRAT *strat, int st)
{
	int objId;
	STRAT *s;

	s = (STRAT *)st;

	objId = GetSem(strat, 0);

	
	if (objId == 0)
	{
		strat->CheckPos = s->pos;
	}
	else
	{
		strat->CheckPos.x = s->objId[objId]->wm.m[3][0];
		strat->CheckPos.y = s->objId[objId]->wm.m[3][1];
		strat->CheckPos.z = s->objId[objId]->wm.m[3][2];
	//	findObjectWorldPosition(&strat->CheckPos, s, objId);				
	}
}

void AddScreenToTargetArray(int pn)
{
	Vector3	dir, camToStrat;
	COLLSTRAT		*coll;
	STRAT *s;
	Point3	tempp, p;
	int i, n = 0;

	dir.x = player[pn].CameraStrat->m.m[1][0];
	dir.y = player[pn].CameraStrat->m.m[1][1];
	dir.z = player[pn].CameraStrat->m.m[1][2];
	v3Normalise(&dir);
	
	clearTargetArray(pn);
	coll = FirstColl;

	while (coll)
	{
		s = coll->strat;
		if (n == GetSecondaryWeaponAmount(0))
			break;

		if (s->Player && s->Player->cloaked)
			continue;

		if ((s->flag & STRAT_COLLSTRAT) && (s->flag & STRAT_ENEMY) && (!(s->flag & STRAT_BULLET)) && (s->health > 0.0f))
		{
			v3Sub(&camToStrat, &s->pos, &player[pn].CameraStrat->pos);
			v3Normalise(&camToStrat);

			if (acos(v3Dot(&camToStrat, &dir)) > 0.5f)
			{
				coll = coll->next;
				continue;
			}

			if (s->flag & COLL_TARGETABLE)
			{
				if (!obscuredByScape(&player[pn].CameraStrat->pos, &s->pos, &tempp, s))
				{
					if ((s->health > 0.0f) && (s->flag & COLL_TARGETABLE))
					{
						addToTargetArray(s, NULL, pn);
						n++;
					}
				}
			}
			else
			{
				if (s->pnode)
				{
					for (i=1; i<s->pnode->noObjId + 1; i++)
					{
						if (s->objId[i])
						{
							if (s->objId[i]->collFlag & COLL_TARGETABLE)
							{
								if (s->objId[i]->health > 0.0f)
								{
									p.x = s->objId[i]->wm.m[3][0];
									p.y = s->objId[i]->wm.m[3][1];
									p.z = s->objId[i]->wm.m[3][2];
									if (!obscuredByScape(&player[pn].CameraStrat->pos, &p, &tempp, s))
									{
										if ((s->objId[i]->collFlag & COLL_TARGETABLE) && (s->objId[i]->health > 0.0f))
										{
											addToTargetArray(s, s->objId[i], pn);
											n++;
										}
									}
								}
							}
						}
					}
				}
			}
		}
		coll = coll->next;
	}

}

void DrawTargetAt(float x, float y, float z)
{
	Point3 p;

	p.x = x;
	p.y = y;
	p.z = z;

	dAssert(currentDrawTarget < 64, "OVERFLOW ON DRAW TARGETS");


	drawTargetArray[currentDrawTarget] = p;
	currentDrawTarget++;
}

void ClearTargetArray(int pn)
{
	clearTargetArray(pn);
}

void StormingShield(STRAT *strat)
{
	int i;
	Point3	p1, p2;

	for (i=0; i<5; i++)
	{
		v3Rand(&p1);
		v3Rand(&p2);
		v3Scale(&p1, &p1, 2.0f);
		v3Scale(&p2, &p2, 2.0f);
		v3Inc(&p1, &strat->pos);
		v3Inc(&p2, &strat->pos);
		addShockToShockArray(&p1, &p2);
	}
}

void StormingShieldHit(STRAT *strat)
{
	int pn;
	float	tempf;

	pn = strat->flag2 & 3;
	tempf = strat->var;
	strat->var = strat->Player->n;
	strat->damage = 101.0f;
	HurtStrat(player[pn].Player, strat);
	strat->damage = 0.0f;
	strat->var = tempf;
}

int	GetHomingBulletTarget(int pn)
{
	int				noStrats = 0;
	Vector3			dir, gunToStrat;
	COLLSTRAT		*coll;
	STRAT			*strat;
	int				s[64];

	dir.x = player[pn].Player->objId[5]->wm.m[1][0];
	dir.y = player[pn].Player->objId[5]->wm.m[1][1];
	dir.z = player[pn].Player->objId[5]->wm.m[1][2];

	v3Normalise(&dir);

	

	coll = FirstColl;

	while (coll)
	{
		strat = coll->strat;

		if ((strat->flag & STRAT_COLLSTRAT) && !(strat->flag & STRAT_BULLET) && !(strat->flag2 & STRAT2_PICKUP))
		{
			v3Sub(&gunToStrat, &strat->pos, &player[pn].Player->pos);
			v3Normalise(&gunToStrat);
			if ((strat == player[pn].Player) || (acos(v3Dot(&dir, &gunToStrat)) > 0.4f))
			{
				coll = coll->next;
				continue;
			}
			s[noStrats] = (int)strat;
			noStrats++;
		}
		coll = coll->next;
	}
	
	if (noStrats > 0)
		return s[rand() % noStrats];
	else
		return 0;
}

int SetCheckPosToTarget(STRAT *thisStrat, int s)
{
	STRAT *strat;
	
	strat = (STRAT *)s;
	if (strat->pnode)
	{
		if (strat->health > 0.0f)
		{
			if (strat->flag)
			{
				thisStrat->CheckPos = strat->pos;
				return 1;
			}
		}
	}
	
	return 0;
}

static void SetObjectOCPRec(Object *obj)
{
	int i;

	for (i=0; i<obj->ncp; i++)
	{
		mPoint3Multiply3(&obj->cp[i].worldPos, &obj->cp[i].centre, &obj->wm);
		obj->ocpt[i] = obj->cp[i].worldPos;
	}

	for (i=0; i<obj->no_child; i++)
		SetObjectOCPRec(&obj->child[i]);
}

void SetPlayerOCP(void)
{
	objectToWorld(player[0].Player);
	SetObjectOCPRec(player[0].Player->obj);
	player[0].Player->oldPos = player[0].Player->pos;
	player[0].Player->oldOldPos = player[0].Player->oldPos;

   	if (player[0].CameraStrat)
	{
		objectToWorld(player[0].CameraStrat);
		SetObjectOCPRec(player[0].CameraStrat->obj);
		player[0].CameraStrat->oldPos = player[0].CameraStrat->pos;
		player[0].CameraStrat->oldOldPos = player[0].CameraStrat->oldPos;
	}
}

void GetLastWay(STRAT *strat)
{
	if (LastWay(strat))
	{
		GetPrevWay(strat);
		GetNextWay(strat);
	}
	else 
	{
		while (!LastWay(strat))
		{
			GetNextWay(strat);
		}
	}
}

void GetFirstWay(STRAT *strat)
{
	if (FirstWay(strat))
	{
		GetNextWay(strat);
		GetPrevWay(strat);
	}
	else 
	{
		while (!FirstWay(strat))
		{
			GetPrevWay(strat);
		}
	}
}

void DrawTargetOnObject(STRAT *strat, int objId)
{
	DrawTargetAt(strat->objId[objId]->wm.m[3][0], strat->objId[objId]->wm.m[3][1], strat->objId[objId]->wm.m[3][2]);
}

int	BeamPlayerShieldColl(STRAT *strat, int pn, int objId, float radius)
{
	Point3	p1, p2;
	Vector3	dir, v;
	float ang, dist;

	if (!player[pn].ShieldHold)
		return 0;

	p1.x = strat->objId[objId]->wm.m[3][0];
	p1.y = strat->objId[objId]->wm.m[3][1];
	p1.z = strat->objId[objId]->wm.m[3][2];

	dir.x = strat->objId[objId]->wm.m[1][0];
	dir.y = strat->objId[objId]->wm.m[1][1];
	dir.z = strat->objId[objId]->wm.m[1][2];

	v3Normalise(&dir);

	p2 = player[pn].shieldStrat->pos;

	v3Sub(&v, &p2, &p1);
	v3Normalise(&v);

	ang = acos(v3Dot(&v, &dir));
	dist = pDist(&p1, &p2);

	if (dist * sin(ang) < radius + 1.0f)
		return 1;
	else
		return 0;

}

int	BeamPlayerColl(STRAT *strat, int pn, int objId, float radius)
{
	Point3	p1, p2;
	Vector3	dir, v;
	float ang, dist;

	p1.x = strat->objId[objId]->wm.m[3][0];
	p1.y = strat->objId[objId]->wm.m[3][1];
	p1.z = strat->objId[objId]->wm.m[3][2];

	dir.x = strat->objId[objId]->wm.m[1][0];
	dir.y = strat->objId[objId]->wm.m[1][1];
	dir.z = strat->objId[objId]->wm.m[1][2];

	v3Normalise(&dir);

	p2 = player[pn].Player->pos;

	v3Sub(&v, &p2, &p1);
	v3Normalise(&v);

	ang = acos(v3Dot(&v, &dir));
	dist = pDist(&p1, &p2);

	if (dist * sin(ang) < radius + 1.0f)
		return 1;
	else
		return 0;

}


float ObjectPlayerShieldDist(STRAT *strat, int pn, int objId)
{
	Point3	p1, p2;

	p1.x = strat->objId[objId]->wm.m[3][0];
	p1.y = strat->objId[objId]->wm.m[3][1];
	p1.z = strat->objId[objId]->wm.m[3][2];

	p2 = player[pn].shieldStrat->pos;

	return pDist(&p1, &p2);
}

float ObjectPlayerDist(STRAT *strat, int pn, int objId)
{
	Point3	p1, p2;

	p1.x = strat->objId[objId]->wm.m[3][0];
	p1.y = strat->objId[objId]->wm.m[3][1];
	p1.z = strat->objId[objId]->wm.m[3][2];

	p2 = player[pn].Player->pos;

	return pDist(&p1, &p2);
}


int uId(STRAT *strat)
{
	if (strat->pnode)
		return strat->pnode->typeId;
	else
		return -1;
}

void SetPlayerMaxHealth(int pn, float maxHealth)
{
	player[pn].maxHealth = maxHealth;
	if (!Multiplayer)
	{
		if (player[0].reward & UPGRADE_ARMOUR_1)
			player[0].maxHealth = UPGRADE_ARMOUR_1_HEALTH;
		if (player[0].reward & UPGRADE_ARMOUR_2)
			player[0].maxHealth = UPGRADE_ARMOUR_2_HEALTH;
	}	
}

float InfrontOfObject(STRAT *strat, int objId, float dist)
{
	Point3 p, p1, p2;
	Vector3	v;

	p = player[0].Player->pos;
	objectToWorld(strat);

	v.x = strat->objId[objId]->wm.m[1][0];
	v.y = strat->objId[objId]->wm.m[1][1];
	v.z = strat->objId[objId]->wm.m[1][2];
	v3Normalise(&v);
	v3Scale(&v, &v, dist);
	p1.x = strat->objId[objId]->wm.m[3][0];
	p1.y = strat->objId[objId]->wm.m[3][1];
	p1.z = strat->objId[objId]->wm.m[3][2];
	v3Add(&p2, &p1, &v);

	return pointLineDistance(&p, &p1, &p2);
}

float Range(float a, float b, float c)
{
	return RANGE(a, b, c);
}

void DrawAimLine(int pn)
{
	player[pn].aimLine = 1;
}

float WaterSlideCamLookDist(STRAT *strat)
{
	return pDist(&strat->pos, &WaterSlideCamLookStrat->pos);
}

void AddLavaStrat(STRAT *strat)
{
	dAssert(strat->pnode, "no pnode in water");
	lavaStrat[noLavaStrats] = strat;
	objectToWorld(strat);
	mShortInvert(&strat->obj->im, &strat->obj->wm);
	noLavaStrats++;
}

int	GetObjectFlag(STRAT *strat, int objId)
{
	return strat->objId[objId]->collFlag;
}

void SetObjectFlag(STRAT *strat, int objId, int flag)
{
	strat->objId[objId]->collFlag = flag;
}


void MoveXYZNow(STRAT *strat, float x, float y, float z)
{
	Matrix m;
	Vector3 v;
	if (PAL)
	{
		x *= PAL_MOVESCALE;
		y *= PAL_MOVESCALE;
		z *= PAL_MOVESCALE;
	}

	m = strat->m;
	if (x != 0.0f)
	{
		v.x = strat->m.m[0][0];
		v.y = strat->m.m[0][1];
		v.z = strat->m.m[0][2];
		v3Scale(&v, &v, x);
		v3Inc(&strat->pos, &v);
	}
	if(y != 0.0)
	{
		v.x = strat->m.m[1][0];
		v.y = strat->m.m[1][1];
		v.z = strat->m.m[1][2];
		v3Scale(&v, &v, y);
		v3Inc(&strat->pos, &v);
	}
	if(z != 0.0)
	{
		v.x = strat->m.m[2][0];
		v.y = strat->m.m[2][1];
		v.z = strat->m.m[2][2];
		v3Scale(&v, &v, z);
		v3Inc(&strat->pos, &v);
	}
}

void InitArrowArray(void)
{
	int i;

	EscortTankerHit[0] = NULL;
	EscortTankerHit[1] = NULL;
	EscortTankerHit[2] = NULL;

	for (i=0; i<N_ARROW_ARRAY; i++)
	{
		arrowArray[i] = NULL;
		arrowArrayVis[i] = 0;
	}
}

void AddParentToArrowArray(STRAT *strat)
{
	int i;

	for (i=0; i<N_ARROW_ARRAY; i++)
	{
		if (arrowArray[i] == NULL)
		{
			arrowArray[i] = strat->parent;
			return;
		}
	}
	dAssert(0, "Too many array Enemies");
}

void UpdateArrowArray(STRAT *strat)
{
	int i;
	float dist = 100000000.0f, tempf = 0;
	STRAT	*tStrat = NULL;
	Vector3 camDir, targetDir, cross, tempv;

	for (i=0; i<N_ARROW_ARRAY; i++)
	{
		if ((EscortTankerHit[0]) && (EscortTankerHit[0] == arrowArray[i]))
			EscortTanker->health = -1.0f;

		if ((EscortTankerHit[1]) && (EscortTankerHit[1] == arrowArray[i]))
			EscortTanker->health = -1.0f;

		if ((EscortTankerHit[2]) && (EscortTankerHit[2] == arrowArray[i]))
			EscortTanker->health = -1.0f;

		if ((arrowArray[i]) && (arrowArray[i]->health <= 0.0f))
		{
			arrowArray[i] = NULL;
		}
		else if (arrowArray[i] && EscortTanker)
		{
			tempf = pSqrDist(&arrowArray[i]->pos, &EscortTanker->pos);
			if ((tempf < dist) && (arrowArray[i]->pos.y + 100.0 > EscortTanker->pos.y))
			{
				dist = tempf;
				tStrat = arrowArray[i];
			}
		}
	}

	if (tStrat)
	{
		
		camDir.x = player[0].CameraStrat->m.m[1][0];
		camDir.y = player[0].CameraStrat->m.m[1][1];
		camDir.z = 0.0f;
		v3Normalise(&camDir);
		v3Sub(&targetDir, &tStrat->pos, &player[0].CameraStrat->pos);
		targetDir.z = 0.0f;
		v3Normalise(&targetDir);
		strat->damage = acos(v3Dot(&targetDir, &camDir));
		v3Cross(&cross, &camDir, &targetDir);
		if (cross.z < 0.0f)
			strat->damage *= -1.0f;

		if (obscuredByScape(&player[0].CameraStrat->pos, &tStrat->pos, &tempv, tStrat))
		{
			tStrat = NULL;
			tStratTime += 0.01f;
		}
		else
		{
			if (tStrat == oldtStrat)
				tStratTime += 0.01f;
			else
			{
				oldtStrat = tStrat;
				tStratTime = 0.0f;
			}

			DrawTargetSpin(&tStrat->pos, tStratTime); 
		}
	}
	else
		strat->damage = 0.0f;

	EscortTankerHit[0] = NULL;
	EscortTankerHit[1] = NULL;
	EscortTankerHit[2] = NULL;

}

void RemoveParentFromArrowArray(STRAT *strat)
{
	int i;

	for (i=0; i<N_ARROW_ARRAY; i++)
	{
		if (strat->parent == arrowArray[i])
		{
			arrowArray[i] = NULL;
			arrowArrayVis[i] = 0;
		}
	}
}

static void ObjectNoFadeRec(Object *obj)
{
	int i;

	obj->collFlag |= OBJECT_NOFADE;
	for (i=0; i<obj->no_child; i++)
	{
		ObjectNoFadeRec(&obj->child[i]);
	}
}



static void ObjectNoCamCollRec(Object *obj)
{
	int i;

	obj->collFlag |= OBJECT_NOCAMERACOLLIDE;
	for (i=0; i<obj->no_child; i++)
	{
		ObjectNoCamCollRec(&obj->child[i]);
	}
}



void StrtNoFade(STRAT *strat)
{

   

	ObjectNoFadeRec(strat->obj);

}

void StrtNoCamColl(STRAT *strat)
{


	ObjectNoCamCollRec(strat->obj);

}


void ObjectNoCull(STRAT *strat, int objId)
{
	strat->objId[objId]->collFlag |= OBJECT_NOCULL;
}

//mode 0 for 2d
//mode 1 for 3d
int	EnemyInActivation(STRAT *me, int an, int mode)
{
	STRAT		*strat;
	COLLSTRAT	*coll;
	Point3	p, c;
	float	radius;

	if (!HasActivation(me, an))
		return 0;

	coll = FirstColl;

	while (coll)
	{
		strat = coll->strat;

		if ((strat->pnode->radius > 100.0f) || (strat == me) || (strat == player[0].Player) || (strat->parent == me))
		{
			coll = coll->next;
			continue;
		}
		if ((strat->flag & STRAT_COLLSTRAT) && !(strat->flag & STRAT_BULLET) && !(strat->flag2 & STRAT2_PICKUP) && (strat != me) && (strat->flag & STRAT_ENEMY) && (strat->pnode->radius < 30.0f))
		{
			c = MapActs[me->actindex[an]].pos;
			if (!mode)
				c.z = 0.0;
			p = strat->pos;
			if (!mode)
				p.z = 0.0f;
			radius = MapActs[me->actindex[an]].radius + strat->pnode->radius;
			if (pSqrDist(&p, &c) < radius * radius)
				return 1;
		}
		coll = coll->next;
	}
	return 0;
}

void ObjectNotSolid(STRAT *strat, int objId)
{
	strat->objId[objId]->collFlag |= OBJECT_NOTSOLID;
}

int	ObjectHitPlayer(STRAT *strat, int objId)
{
	if (strat->objId[objId]->collFlag & OBJECT_HIT_PLAYER)
		return 1;
	else
		return 0;
}

STRAT *PlayerShieldStrat(int pn)
{
	
	if (player[pn].Player)
	{
		if (player[pn].Player->health > 0.0f)
		{
			if (player[pn].shieldStrat)
			{
				return player[pn].shieldStrat;
			}
		}
	}
	
	return NULL;
}

static float LineSphereIntersection(Point3 *p1, Point3 *p2, Point3 *s, float r)
{
	float a, b, c, i, j, k, x1, y1, z1, x2, y2, z2, l, m, n, res1, res2;

	l = s->x;
	m = s->y;
	n = s->z;

	x1 = p1->x;
	y1 = p1->y;
	z1 = p1->z;

	x2 = p2->x;
	y2 = p2->y;
	z2 = p2->z;

	i = x2 - x1;
	j = y2 - y1;
	k = z2 - z1;

	a = (i * i) + (j * j) + (k * k);
	b = (2.0f * i * (x1 - l)) + (2.0f * j * (y1 - m)) + (2.0f * k * (z1 - n));
	c = (l * l) + (m * m) + (n * n) + (x1 * x1) + (y1 * y1) + (z1 * z1) - (2.0f * (l * x1 + m * y1 + n * z1)) - (r * r);

	if (((b * b) - (4.0f * a * c)) < 0.0f)
	{
		return 0.0001f;
	}
	else
	{
		res1 = (-b + sqrt((b * b) - (4.0f * a * c))) / (2.0f * a);
		res2 = (-b - sqrt((b * b) - (4.0f * a * c))) / (2.0f * a);
		if (res1 < res2)
			return res1;
		else
			return res2;
	}
}


void BounceOffShield(STRAT *strat, int pn)
{
	Point3 p1, p2, s, p;
	Vector3	v, sc_p, ls_p, n1, n2;
	float rad, lambda, strength;
	
	rad = 3.579f;

	v = strat->vel;
	v3Scale(&v, &v, 10.0f);

	v3Sub(&p1, &strat->pos, &v);
	v3Add(&p2, &strat->pos, &v);

	strat->parent = player[pn].Player;
	/*s.x = player[pn].Player->objId[5]->wm.m[3][0];
	s.y = player[pn].Player->objId[5]->wm.m[3][1];
	s.z = player[pn].Player->objId[5]->wm.m[3][2];*/

	s = player[pn].Player->pos;
	/*s.x += 0.5f * player[pn].Player->m.m[1][0];
	s.y += 0.5f * player[pn].Player->m.m[1][1];
	s.z += 0.5f * player[pn].Player->m.m[1][2];*/

	lambda = LineSphereIntersection(&p1, &p2, &s, rad);

  //	if (lambda == 0.0f)
  //		lambda = 0.0001f;
	//dAssert(fabs(lambda) > 0.0001f, "oops");

	v3Sub(&v, &p2, &p1);
	v3Scale(&v, &v, lambda);
	v3Add(&p, &p1, &v);
	strat->pos = p;
	/*
	v3Sub(&sc_p, &p, &s);
	v3Sub(&ls_p, &p, &p1);
	n1 = sc_p;
	*/
	v3Sub(&ls_p, &p, &p1);
	n1.x = player[pn].Player->objId[5]->wm.m[1][0];
	n1.y = player[pn].Player->objId[5]->wm.m[1][1];
	n1.z = player[pn].Player->objId[5]->wm.m[1][2];
	n2 = ls_p;
	v3Normalise(&n1);
	v3Normalise(&n2);
	v3Neg(&n2);
	strength = v3Dot(&n1, &n2);
	if (acos(strength) < 0.1f)
	{
		v3Neg(&strat->vel);
	}
	else
	{
		v3Scale(&n1, &n1, strength * 2.0f * v3Length(&strat->vel));
		v3Inc(&strat->vel, &n1);
	}
	if (!Multiplayer)
		strat->damage *= 10.0f;

	if (BossStrat && player[0].onBoat)
	{
		strat->vel.z = 0.0f;
		strat->rot_speed = 0.0f;
	}
	  	dAssert (strat->pos.x == strat->pos.x, "QNAN");
	  	dAssert (strat->vel.x == strat->vel.x, "QNAN");


	makeMatrixFromVector(&strat->vel, &strat->m);
}

void GetGunDir(STRAT *strat, int pn)
{
	strat->CheckPos.x = player[pn].Player->objId[5]->wm.m[1][0];
	strat->CheckPos.y = player[pn].Player->objId[5]->wm.m[1][1];
	strat->CheckPos.z = player[pn].Player->objId[5]->wm.m[1][2];
	v3Normalise(&strat->CheckPos);
}

void DoSpark(STRAT *strat, int n, float x, float y, float z, float speed, float spread, int time, int red, int green, int blue)
{
	Vector3 v;
	int col;

	col = (red << 16) | (green << 8) | (blue << 0);

	/*v.x = strat->m.m[1][0];
	v.y = strat->m.m[1][1];
	v.z = strat->m.m[1][2];*/
	v.x = x;
	v.y = y;
	v.z = z;

	addSpark(n, &strat->pos, &v, speed, spread, time, 1, col);
}

void SetPlayerMatrix(STRAT *strat)
{
	player[0].Player->m = strat->m;
}

void SetPlayerCamera(STRAT *strat)
{
	MyStratToWorld(strat, 0.0, -6.0f, 1.0f);
	player[0].CameraStrat->pos = strat->CheckPos;
	MyStratToWorld(strat, 0.0, 0.0f, 1.0f);
	player[0].camLookCrntPos = strat->CheckPos;
}

void RadiusDamage(STRAT *cs, float radius, float amount)
{
	COLLSTRAT	*coll;
	STRAT *strat;

	coll = FirstColl;

	while (coll)
	{
		strat = coll->strat;

		if (strat->Player)
		{
			coll = coll->next;
			continue;
		}
		if (!(strat->flag & STRAT_BULLET))
		{		
			if (strat->flag & STRAT_COLLSTRAT)
			{
				if (pSqrDist(&strat->pos, &cs->pos) - (strat->pnode->radius * strat->pnode->radius) < radius * radius)
					strat->health -= amount;
			}
		}
		coll = coll->next;
	}

}

void SetPlayerEnergy(int pn, float amount)
{
	player[pn].PlayerWeaponEnergy = amount;
}

float GetPlayerEnergy(int pn)
{
	return player[pn].PlayerWeaponEnergy;
}

void SetPlayerCamTilt(int pn, float amount)
{
	player[pn].crntCamTilt = amount;
}

float GetPlayerCamTilt(int pn)
{
	return player[pn].crntCamTilt;
}

void SetScreenARGB(float a, float r, float g, float b)
{
	Colour col;

	col.argb.a = 0;
	col.argb.r = r * 255.0;
	col.argb.g = g * 255.0;
	col.argb.b = b * 255.0;

	rFade(col, a);
}

static int inArrowArray(STRAT *strat)
{
	int i;

	for (i=0; i<N_ARROW_ARRAY; i++)
		if (strat == arrowArray[i])
			return 1;

	return 0;
}

int EnemyInFrontXY(STRAT *strat, float m, float r, int type)
{
	Vector3		v;
	Point3		p, ep;
	COLLSTRAT	*coll;
	STRAT		*collStrat;
	float		dist;


	v.x = strat->m.m[1][0];
	v.y = strat->m.m[1][1];
	v.z = 0.0f;

	v3Normalise(&v);

	p = strat->pos;
	p.z = 0.0f;
	v3Scale(&v, &v, m);
	v3Inc(&p, &v);

	coll = FirstColl;

	while (coll)
	{
		collStrat = coll->strat;

		switch (type)
		{
			case 0:
				if (collStrat->flag & (STRAT_FRIENDLY | STRAT_BULLET))
				{
					coll = coll->next;
					continue;
				}
				break;
			case 1:
				if ((collStrat->flag & (STRAT_FRIENDLY | STRAT_BULLET)) || (!inArrowArray(collStrat)))
				{
					if (collStrat != player[0].Player)
					{
						coll = coll->next;
						continue;
					}
				}
				break;
			case 2:
				if ((collStrat->flag & (STRAT_FRIENDLY | STRAT_BULLET)) || (!inArrowArray(collStrat)))
				{
					coll = coll->next;
					continue;
				}
				break;
		}

		ep = collStrat->pos;
		ep.z = 0.0f;
		dist = r + collStrat->pnode->radius;

		if (pSqrDist(&p, &ep) < dist * dist)
			return 1;

		coll = coll->next;
	}
	return 0;
}

int PlayerInFrontXY(STRAT *strat, float m, float r)
{
	Vector3		v;
	Point3		p, ep;
	float		dist;


	if (!player[0].Player)
		return 0;

	v.x = strat->m.m[1][0];
	v.y = strat->m.m[1][1];
	v.z = 0.0f;

	if (v3SqrLength(&v) < 0.0001f)
		return 0;

	v3Normalise(&v);

	p = strat->pos;
	p.z = 0.0f;
	v3Scale(&v, &v, m);
	v3Inc(&p, &v);

	ep = player[0].Player->pos;
	ep.z = 0.0f;
	dist = r + player[0].Player->pnode->radius;

	if (pSqrDist(&p, &ep) < dist * dist)
		return 1;
	else
		return 0;
}


int EscortTankerInActivationXY(STRAT *strat, int num)
{
	Point3 p1, p2;

	if (!EscortTanker)
		return 0;

	p1 = EscortTanker->pos;
	p2 = MapActs[strat->actindex[num]].pos;
	p1.z = 0.0f;
	p2.z = 0.0f;


	if (strat->actindex[num] >= 0)
		if (pDist(&p1, &p2) <= MapActs[strat->actindex[num]].radius)
			return (1);

	return(0);
}

void DrawAllTags(void)
{

	int i;
	Point3 p, p2;
	Vector3 tempv;

	for (i=0; i<N_ARROW_ARRAY; i++)
	{
		if ((arrowArray[i]) && ((currentFrame & 0x1f) == i))
		{
			v3Sub(&tempv, &player[0].CameraStrat->pos, &arrowArray[i]->pos);
			v3Normalise(&tempv);
			v3Scale(&tempv, &tempv, arrowArray[i]->pnode->radius + 1.0f);
			v3Add(&p2, &arrowArray[i]->pos, &tempv);
			if (obscured(&player[0].CameraStrat->pos, &p2, &p, 0, 0))
				arrowArrayVis[i] = 0;
			else
				arrowArrayVis[i] = 1;
		}

		if ((arrowArray[i]) && (arrowArrayVis[i]))
			DrawTargetSpin(&arrowArray[i]->pos, ((float)(currentFrame)) * 0.01f); 
	}
}

void DrawArrowTo(STRAT *strat, float x, float y, float z, float a, float r, float g, float b)
{
	Point3 p;
	Uint32 col;

	if (!Multiplayer)
		if (player[0].CameraStrat)
			if (player[0].CameraStrat->parent)
				return;
	p.x = x;
	p.y = y;
	p.z = z;
	col = (((int)(a * 255.0f))<<24)|(((int)(r * 255.0f))<<16)|(((int)(g * 255.0f))<<8)|((int)(b));

	DrawArrowToPoint(&p, col);
}
void SetCamSimple(STRAT *strat)
{
   //	return;
	player[0].cameraPos = THIRD_PERSON_CLOSE;
	player[0].CameraStrat->parent = strat;
	player[0].CameraStrat->parent->flag |= STRAT_HIDDEN;
}

void MoveZRel(STRAT *strat, float amount)
{
	Vector3 v;

	v.x = strat->m.m[2][0];
	v.y = strat->m.m[2][1];
	v.z = strat->m.m[2][2];

	v3Normalise(&v);
	v3Scale(&v, &v, amount);
	v3Inc(&strat->pos, &v);
}

void GetNearestTagToPlayer(STRAT *strat)
{
	int i;
	Point3 p, p2;
	Vector3 tempv;
	float closestDist = 100000000.0f, dist;
	STRAT *closestStrat;


	closestStrat = NULL;
	for (i=0; i<N_ARROW_ARRAY; i++)
	{
		if (arrowArray[i])
		{
			dist = pSqrDist(&arrowArray[i]->pos, &player[0].Player->pos);
			if (dist < closestDist)
			{
				closestStrat = arrowArray[i];
				closestDist = dist;
			}
		}
	}
	if (closestStrat)
		strat->CheckPos = closestStrat->pos;
	else
	{
		strat->CheckPos.x = 0.0f;
		strat->CheckPos.y = 0.0f;
		strat->CheckPos.z = 0.0f;
	}
}


void doControlFade(int pn)
{
	float a, r, g, b;

	if (fadeN[pn] < fadeLength[pn])
	{
		switch (fadeMode[pn])
		{
			case FADE_TO_BLACK:
				a = (((float)(fadeN[pn])) / ((float)(fadeLength[pn])));
				SetScreenARGB(a, 0.0f, 0.0f, 0.0f);
				break;
			case FADE_FROM_BLACK:
				a = 1.0f - (((float)(fadeN[pn])) / ((float)(fadeLength[pn])));
				SetScreenARGB(a, 0.0f, 0.0f, 0.0f);
				break;
		}
	}
	else if ((fadeN[pn] >= fadeLength[pn]) && (fadeMode[pn] != FADE_NONE))
	{
		switch (fadeMode[pn])
		{
			case FADE_TO_BLACK:
				SetScreenARGB(1, 0.0f, 0.0f, 0.0f);
				break;
			case FADE_FROM_BLACK:
				fadeMode[pn] = FADE_NONE;
				fadeN[pn] = 0;
				fadeLength[pn] = 0;
				break;
		}
		
	}
	fadeN[pn]++;
}

void CamSetNow(void)
{
	CameraSet(player[0].cam, 0.0f);
}

void GetParentObjectPos(STRAT *strat, int i)
{
	strat->CheckPos.x = strat->parent->objId[i]->wm.m[3][0];
	strat->CheckPos.y = strat->parent->objId[i]->wm.m[3][1];
	strat->CheckPos.z = strat->parent->objId[i]->wm.m[3][2];
}

int	PlayerReward(int pn, int bit)
{
	return (player[pn].reward & bit);
}

void DrawVLine(STRAT *strat, float dist)
{
	Point3 p1, p2;
	Vector3 v, vn;
	float lambda;

	if (dist > 40.0f)
		dist = 40.0f;

	lambda = RandRange(0.0, dist);

	p1 = strat->pos;
	vn.x = strat->m.m[1][0];
	vn.y = strat->m.m[1][1];
	vn.z = strat->m.m[1][2];
	v3Scale(&v, &vn, lambda);
	v3Inc(&p1, &v);
	v3Scale(&v, &vn, 60.0);
	v3Add(&p2, &p1, &v);
	DrawLine(&p1, &p2, 0xff00ffff);
}

void GetBoatPA(STRAT *strat, int i)
{
	strat->CheckPos = boatPA[i];
}

int	PlayerNearParentParentActivationXY(STRAT *strat,int num)
{
	STRAT *ms;
	float a, b;

	if (!(player[currentPlayer].Player))
	   return (0);

	if (!strat->parent)
		return 0;

	if (!strat->parent->parent)
		return 0;

	ms = strat->parent->parent;

	if (ms->actindex[num] >= 0)
	{
		a = WayToPlayerDistXY(&MapActs[ms->actindex[num]].pos);
		b = MapActs[ms->actindex[num]].radius;
		if (a <= b)
			return 1;
	}

	return 0;
}


int	PlayerNearParentActivationXY(STRAT *strat,int num)
{
	STRAT *ms;
	float a, b;

	if (!(player[currentPlayer].Player))
	   return (0);

	if (!strat->parent)
		return 0;

	ms = strat->parent;

	if (ms->actindex[num] >= 0)
	{
		a = WayToPlayerDistXY(&MapActs[ms->actindex[num]].pos);
		b = MapActs[ms->actindex[num]].radius;
		if (a <= b)
			return 1;
	}

	return 0;
}

void SetStratMatrixVector(STRAT *strat, float a, float b, float c)
{
	Vector3 v;

	v.x = a;
	v.y = b;
	v.z = c;

	makeMatrixFromVector(&v, &strat->m);
	if (PAL)
		RelRotateX(strat, 1.570795 * 0.83333f);
	else
		RelRotateX(strat, 1.570795);
}

int CanPickup(STRAT *strat, int pn, int type, int amount)
{

	if (type == 5)
	{
		if (Multiplayer)
		{
			if (player[pn].Player->health >= 100.0)
				return 0;
			else
				return 1;
		}
		else
		{
			if (player[pn].Player->health >= player[pn].maxHealth)
				return 0;
			else
				return 1;
		}

	}
	if (type == 16)
	{
		if (player[0].PlayerWeaponEnergy >= 2.0f)
			return 0;
		else
			return 1;
	}
	if ((player[pn].PlayerSecondaryWeapon.type == type) && (player[pn].PlayerSecondaryWeapon.amount == amount))
		return 0;
	else
		return 1;
}

#define MAX_CREDITS 29 
const int credInfo[] =
{
//	offset	lines	x	y	center	double title

			//SEGA

	0,		1,		0,	0,	1,		0,	//Sega
	1,		3,		0,	0,	1,		1,	//Director of Product Development
	4,		2,		0,	0,	1,		0,	//Producer
	6,		2,		0,	0,	1,		0,	//Senior Test Manager
	8,		2,		0,	0,	1,		0,	//Senior Lead Tester
	10,		2,		0,	0,	1,		0,	//Lead Tester
	12,		9,		0,	0,	1,		0,	//Testers
	21,		3,		0,	0,	1,		1,	//Localisation Co-ordinator
	24,		3,		0,	0,	1,		0,	//Localisation Team
	27,		2,		0,	0,	1,		0,	//Senior Design
	29,		3,		0,	0,	1,		1,	//Manual Localisation
	32,		2,		0,	0,	1,		0,	//Marketing Director
	34,		3,		0,	0,	1,		1,	//Product Marketing
	37,		3,		0,	0,	1,		0,	//Special Thanks
										
			//ARGONAUT					
										
	40,		1,		0,	0,	1,		0,	//Argonaut Software
	41,		9,		0,	0,	1,		0,	//Art team
	50,		2,		0,	0,	1,		0,	//Animation
	52,		4,		0,	0,	1,		0,	//Programming team
	56,		2,		0,	0,	1,		0,	//Audio managers
	58,		3,		0,	0,	1,		0,	//Audio Engineers
	61,		2,		0,	0,	1,		0,
	63,		2,		0,	0,	1,		0,	//Testing Lead
	65,		2,		0,	0,	1,		0,	//Additional Test
	67,		2,		0,	0,	1,		0,	//Designer
	69,		2,		0,	0,	1,		0,	//Lead Designer
	71,		2,		0,	0,	1,		0,	//Producer
	73,		3,		0,	0,	1,		0,	//Executive producer
	76,		3,		0,	0,	1,		0,	//Special Thanks
	79,		1,		0,	0,	1,		0	//The End


};

const char *creditList[] =
{
	"SEGA EUROPE",

	"Director of",
	"Product Development",
	"Naohiko Hoshino",

	"Producer",
	"Matt O\'Driscoll",

	"Senior Test Manager",
	"Jason Cumberbatch",

	"Senior Lead Tester",
	"Darren Lloyd",

	"Lead Tester",
	"Daniel Slater",

	"Testers",
	"Nick Bennett",
	"Peter O'Brien",
	"Chris Geiles",
	"Justyn McLean",
	"Daley Salami",
	"Andrew Clements",
	"Shaun Leach",
	"Michael Lawsiriphat",

	"Localisation",
	"Co-ordinator",
	"Roberto Párraga-Sánchez",

	"Localisation Team",
	"Angelika Michitsch",
	"Dave Thompson",

	"Senior Design",
	"Paul Jerem",

	"Manual Localisation",
	"Co-ordinator",
	"Sarah Ward",

	"Marketing Director",
	"Giles Thomas",

	"Product Marketing",
	"Manager",
	"Jim Pride",

	"Special Thanks:",
	"Kazutoshi Miyake",
	"Jean-Francois Cecillon",

	"ARGONAUT SOFTWARE",

	"Art Team", 
	"David Hego",
	"David Taylor",
	"Jeff Vanelle",
	"Juan Garcia",
	"Leon Brazil",
	"Melanie Amadi",
	"Nick Lee",
	"Suzanne Cole",

	"Animation",
	"Dave Lowry",

	"Programming Team",
	"Matt Godbolt",
	"Matt Porter",
	"Saviz Izadpanah",

	"Lead Musician",
	"Justin Scharvona",

	"Musicians",
	"Adam Fothergill",
	"Karin Griffin",

	"Audio Programmer",
	"Richard Griffiths",

	"Testing Lead",
	"Jake Fearnside",

	"Additional Test",
	"Carl Ross",

	"Designer",
	"Mark Stephenson",

	"Lead Designer",
	"Sefton Hill",

	"Producer",
	"Nick Clarke",

	"Executive Producer",
	"Jez San",
	"Keith Robinson",

	"Special Thanks:",
	"Christophe Moyne",
	"Rob Lever",

	"THE END"

};


int credTime = 0; 
int credN = 0;

void InitCred(void)
{
	credTime = 0; 
	credN = 0;
				
}
 

#define CREDIT_NCR 6

void DoCred(int x)
{
	int i, sf, n, title;
	float alpha = 1.0f;

	if (x == 100)
	{
		psSetColour (0xffffff);
		mUnit32 (psGetMatrix());
		mPostTranslate32(psGetMatrix(), -320, 0);
		mPostScale32(psGetMatrix(), 1.5, 1.5);
		mPostTranslate32(psGetMatrix(), 320, 0);
		psSetPriority(0);
		psSetAlpha (1.f);
		psDrawCentred("CONGRATULATIONS!", 10);
		return;
	}

	credTime = credTime + 1;
	n = credN % MAX_CREDITS;

	if (n == 28)
	{
		sf = 210;
		if (credTime > 200)
			GameOverFlag = 1;
	}
	else
		sf = (credInfo[(n * CREDIT_NCR) + 1] * 30) + 90;



	if (credTime == sf)
	{
		alpha = 0.0;
		credTime = 0;
		credN++;
		n = credN % MAX_CREDITS;
		sf = (credInfo[(n * CREDIT_NCR) + 1] * 30) + 90;
	}

	if (credTime > sf - 10)
	{
		alpha = (sf - credTime) * 0.1f;
	}
	else if (credTime < 10)
	{
		alpha = credTime * 0.1f;
	}
	else
	{
		alpha = 1.0f;
	}

	n = credN % MAX_CREDITS;

	psSetColour (0xffffff);
	
	psSetPriority(0);
	psSetAlpha (1.f);
	
	for (i=0; i<credInfo[(n * CREDIT_NCR) + 1]; i++)
	{
		if (i == 0)
			title = 1;
		else if ((i == 1) && (credInfo[n * CREDIT_NCR + 5]))
			title = 1;
		else
			title = 0;

		if (title)
		{
			mUnit32 (psGetMatrix());
			mPostTranslate32(psGetMatrix(), -320, 0);
			mPostScale32(psGetMatrix(), 1.5, 1.5);
			mPostTranslate32(psGetMatrix(), 320, 0);
			
			psSetAlpha (alpha);
			if (credInfo[(n * CREDIT_NCR) + 4])
				psDrawCentred(creditList[credInfo[n * CREDIT_NCR] + i], (credInfo[(n * CREDIT_NCR) + 3] + i) * 32);
			else
				psDrawString(creditList[credInfo[n * CREDIT_NCR] + i], credInfo[(n * CREDIT_NCR) + 2], (credInfo[(n * CREDIT_NCR) + 3] + i) * 32);
		}
		else
		{
			mUnit32 (psGetMatrix());
			psSetAlpha (alpha * 0.5f);
			if (credInfo[(n * CREDIT_NCR) + 4])
				psDrawCentred(creditList[credInfo[n * CREDIT_NCR] + i], (credInfo[(n * CREDIT_NCR) + 3] + i) * 32 + 32);
			else
				psDrawString(creditList[credInfo[n * CREDIT_NCR] + i], credInfo[(n * CREDIT_NCR) + 2], (credInfo[(n * CREDIT_NCR) + 3] + i) * 32 + 32);
		}
	}
}

void DrawProgressBar(STRAT *strat, int pn, int time)
{
	player[pn].mpChargeTime = time;
}

int	ValidStrat(int s)
{
	STRAT *strat;

	if (s == 0)
		return 0;

	strat = (STRAT *)s;
	if (strat->flag)
		return 1;
	else
		return 0;
}
