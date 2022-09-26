#include "RedDog.h"
#include "strat.h"
#include "stratphysic.h"
#include "collspec.h"
#include "specfx.h"
#include "camera.h"
#include "view.h"
#include "coll.h"
#include "command.h"
#include "process.h"
#include "debugdraw.h"
#include "MPlayer.h"

#define	COLL_VECTOR_ADD 0.0f
#define MAX_CP_BUFFER 64


#include "insidepoly.inc"

//extern	void drawBox(Point3 *a, Point3 *b, Matrix *m);
//extern	float invMapXScale;
//extern	Vector3	HoldPlayerV;
//Vector3 oldCollVect, collVectM;
 //int     stratCollFlag=0;
//int			vcCount = 0;
//int			vccrntBoxNo;
//Point3		vccrntCentre,vcppn;

extern	Map		*Savmap;

static int			vccrntPolyNo,vci;
static Point3		vcpp, *vcvs, *vcve;
static Vector3		vcfromTo, vcve_vs, vcvs_pp, vctempV;
Vector3 lineLandscapeNormal;
static CollPoly	*vccrntPoly;
int noWaterStrats = 0;
STRAT	*waterStrat[32];
int		noLavaStrats = 0;
STRAT	*lavaStrat[32];
static int		collide = 0,cpBufferObjCount=0, cpBufferCpCount=0; 
Point3 boatPA[4];
int		boatPAC = 0;
float	EscortTankerSpeed = 0.0f;
int GlobalPolyType;
Vector3	gcpn, wallN, GlobalCollPolyN, collVect;
static Point3	cpBuffer[MAX_CP_BUFFER * 2];
Point3 GlobalCollP;
static void	*cpBufferPointer[MAX_CP_BUFFER];
STRAT *iceLiftGun;
STRAT *collArrayStrat[32];
Point3 collArrayPos[32];
Point3 collArrayOrigin[32];
Object *collArrayObject[32];
int	 caIndex = 0;


// Collision array
Collision collArray[MAX_ACTIVE_COLLISIONS];
int nActiveCollisions = 0;

HDStrat		*firstHDStrat = NULL;
HDStrat		*lastHDStrat = NULL;

static int polysInModel(Model *model)
{
	int n = 0, nVerts, j;
	VertRef	*vRef;

	vRef = model->strip;
	for (;;)
	{	
		vRef++;					//skip material		
		nVerts = *vRef++;		//read then skip nVerts

		if (!nVerts)			//last strip then exit
			break;

		for (j=0; j<nVerts - 2; j++)
		{
			if ( (*vRef) == (*(vRef+2)) )
			{
				vRef++;						//if double back on itself then skip 
				continue;
			}
			n++;							//increment poly count
			vRef++;
		}
		vRef += 2;
	}

	return n;
}

static int setUpPolyData(CollPoly *coPoly, Model *model)
{
	int i = 0, nVerts, j, dir, pn = 0, ep = 0;
	VertRef	*vRef;
	Point3	p[3], *pa, *pb;
	Vector3	v[2];

	vRef = model->strip;
	for (;;)
	{	
		vRef++;					//skip material		
		nVerts = *vRef++;		//read then skip nVerts

		if (!nVerts)			//last strip then exit
			break;

		dir = 1;				

		for (j=0; j<nVerts - 2; j++)
		{
		
			if ( (*vRef) == (*(vRef+2)) )
			{
				vRef++;						//if double back on itself then skip 
				dir *= -1;
				continue;
			}

			if (dir == 1)
			{
				coPoly[pn].v[0] = *(vRef + 0);
				coPoly[pn].v[1] = *(vRef + 1);
				coPoly[pn].v[2] = *(vRef + 2);
			}
			else
			{
				coPoly[pn].v[0] = *(vRef + 0);
				coPoly[pn].v[1] = *(vRef + 2);
				coPoly[pn].v[2] = *(vRef + 1);
			}

			pa = (Point3 *)((char *)model->vertex + coPoly[pn].v[1]);
			pb = (Point3 *)((char *)model->vertex + coPoly[pn].v[0]);
			if (POINT3EQUAL(pa, pb))
			{
				ep++;
			}
			else
			{
				v3Sub(&v[0], pa, pb);
				pa = (Point3 *)((char *)model->vertex + coPoly[pn].v[2]);
				pb = (Point3 *)((char *)model->vertex + coPoly[pn].v[1]);
				if (POINT3EQUAL(pa, pb))
				{
					ep++;
				}
				else
				{
					v3Sub(&v[1], pa, pb);
					v3Cross(&coPoly[pn].n, &v[0], &v[1]);
					if (v3Dot(&coPoly[pn].n, &coPoly[pn].n) > 0.0001f)
					{
						v3Normalise(&coPoly[pn].n);
						pn++;
					}
					else
						ep++;		
				}
				
			}
			dir *= -1;
			vRef++;
		}
		vRef++;
		vRef++;
	}
	return ep;
}

static CollObject *initHDCObject(Object *obj)
{

	CollObject *co;
	int i;

	co = (CollObject *)CHeapAlloc(ObjHeap,sizeof(CollObject));
	dAssert(co, "out of ram for hdc");
	#ifdef MEMORY_CHECK
		HDCALLOC += sizeof(CollObject);
		HDCALLOCCHKSUM += (int)co;
	#endif
	
	if ((obj->model) && (!(obj->collFlag & OBJECT_NOCOLLISION)))
	{
		co->obj = obj;
		co->nPoly = polysInModel(obj->model);
		co->poly = (CollPoly *)CHeapAlloc(ObjHeap,sizeof(CollPoly) * co->nPoly);
		dAssert(co->poly, "Out of Ram");
	
		#ifdef MEMORY_CHECK
			HDCALLOC += sizeof(CollPoly) * co->nPoly;
			HDCALLOCCHKSUM += (int)co->poly;
		#endif
		co->corrupt = setUpPolyData(co->poly, obj->model);
		co->nPoly -= co->corrupt;
		co->v = (Point3 *)CHeapAlloc(ObjHeap,sizeof(Point3) * obj->model->nVerts);
		dAssert(co->v, "Out of Ram");

		#ifdef MEMORY_CHECK
			HDCALLOC += sizeof(Point3) * obj->model->nVerts;
			HDCALLOCCHKSUM += (int)co->v;
		#endif
	}
	else
	{
		co->obj = obj;
		co->nPoly = 0;
		co->corrupt = 0;
		co->poly = NULL;
		co->v = NULL;
	}

	if (obj->no_child)
	{
		co->child = (CollObject **)CHeapAlloc(ObjHeap,sizeof(CollObject *) * obj->no_child);
		#ifdef MEMORY_CHECK
			HDCALLOC += sizeof(CollObject *) * obj->no_child;
			HDCALLOCCHKSUM += (int)co->child;
		#endif
		dAssert(co->child, "Out of Ram");
	}
	else
		co->child = NULL;
	for (i=0; i<obj->no_child; i++)
		co->child[i] = initHDCObject(&obj->child[i]);

	return co;
}

void InitHDC(STRAT *strat)
{
	HDStrat *hds;
	int temp;

	if (!strat->obj)
		return;

	if (strat->hdBlock)
		return;

	strat->flag &= ~COLL_TARGETABLE;

	if (lastHDStrat)
	{
		lastHDStrat->next = (HDStrat *)CHeapAlloc(ObjHeap,sizeof(HDStrat));
		dAssert(lastHDStrat->next, "Out of Ram");
		#ifdef MEMORY_CHECK
			HDCALLOC += sizeof(HDStrat);
			HDCALLOCCHKSUM += (int)lastHDStrat->next;
		#endif
		hds = lastHDStrat->next;
		hds->prev = lastHDStrat;
		hds->next = NULL;
		lastHDStrat = hds;
		hds->strat = strat;
		hds->valid = 1;
		strat->hdBlock = hds;
		hds->collObj = initHDCObject(strat->obj);
	}
	else
	{
		firstHDStrat = (HDStrat *)CHeapAlloc(ObjHeap,sizeof(HDStrat));
		#ifdef MEMORY_CHECK
			HDCALLOC += sizeof(HDStrat);
			HDCALLOCCHKSUM += (int)firstHDStrat;
		#endif
		firstHDStrat->next = NULL;
		firstHDStrat->prev = NULL;
		lastHDStrat = firstHDStrat;
		firstHDStrat->strat = strat;
		strat->hdBlock = firstHDStrat;
		strat->hdBlock->valid = 1;
		firstHDStrat->collObj = initHDCObject(strat->obj);
	}
	dAssert(strat->hdBlock, "Out of Ram");

}

static void calculateObjectPolyData(CollObject *collObj)
{
	int i, k;
	Vector3 v[2], *vert, *oldVert;
	Point3	*p;
	CollPoly *cp;
	register float x, y, z, temp, *write;
	
	if ((collObj->obj->model) && (!(collObj->obj->collFlag & OBJECT_NOCOLLISION)))
	{
		k = collObj->obj->model->nVerts;
		vert = &collObj->v[0];
		oldVert = &collObj->obj->model->vertex[0];
		// MattG changed: load the matrix in first
		mLoad (&collObj->obj->wm);

		for (i=0; i<k; i++)
		{
			mPoint3 (vert, oldVert);
			vert++;
			oldVert++;
		}

		
		cp = &collObj->poly[0];
		for (i=0; i<collObj->nPoly; i++, cp++)
		{
			v3Sub(&v[0], (Point3 *)((char *)collObj->v + cp->v[1]), (Point3 *)((char *)collObj->v + cp->v[0]));
			v3Sub(&v[1], (Point3 *)((char *)collObj->v + cp->v[2]), (Point3 *)((char *)collObj->v + cp->v[1]));
			/* was :
			v3Cross(&cp->n, &v[0], &v[1]);
			v3Normalise(&cp->n);
			p = (Point3 *)((char *)collObj->v + cp->v[0]);
			cp->D = - v3Dot(p, &cp->n);
			*/
			// now expanded out
			x = (v[0].y * v[1].z - v[0].z * v[1].y);
			temp = x * x;
			y = (v[0].z * v[1].x - v[0].x * v[1].z);
			temp += y * y;
			z = (v[0].x * v[1].y - v[0].y * v[1].x);
			temp = isqrt (temp + z*z);
			write = &cp->n.x;
			p = (Point3 *)((char *)collObj->v + cp->v[0]);
			x *= temp;
			*write++ = x;
			y *= temp;
			*write++ = y;
			z *= temp;
			*write++ = z;
			*write = - (p->x * x + p->y * y + p->z * z);
		}
	}

	for (i=0; i<collObj->obj->no_child; i++)
		calculateObjectPolyData(collObj->child[i]);
}

int ValidForHDC(STRAT *thisStrat)
{
	COLLSTRAT		*coll;
	STRAT			*strat;
	float			r, rr;
	Point3			p1, p2;
	Vector3 v;

	if (!Multiplayer)
	{
		if (!player[0].Player)
			return 0;
		if (!player[0].Player->flag)
			return 0;
		if (!player[0].CameraStrat)
			return 0;
		p1 = player[0].CameraStrat->pos;
		v.x = player[0].Player->objId[5]->wm.m[1][0];
		v.y = player[0].Player->objId[5]->wm.m[1][1];
		v.z = player[0].Player->objId[5]->wm.m[1][2];
		v3Normalise(&v);
		v3Scale(&v, &v, 500.0f);
		v3Add(&p2, &v, &p1);
		r = thisStrat->pnode->radius + 10.0;
		
		if (pointLineDistance(&thisStrat->pos, &p1, &p2) < r)
			return 1;
	}
	coll = FirstColl;

	while (coll)
	{
		strat = coll->strat;

		if (!(strat->flag & STRAT_COLLFLOOR))
		{
			coll = coll->next;
			continue;
		}

		if (thisStrat == strat->parent)
		{
			coll = coll->next;
			continue;
		}

		rr = strat->pnode->radius;
		rr += r;
		rr *= rr;

		if (pSqrDist(&strat->pos, &thisStrat->pos) < rr)
			return 1;

		coll = coll->next;
	}

	return 0;
}

void CalculateStratPolyData(STRAT *strat, int i)
{
	if (strat->hdBlock == NULL)
		return;

	if (i == 0)
	{
		if (!ValidForHDC(strat))
		{
			strat->hdBlock->valid = 0;
			return;
		}
	}
	strat->hdBlock->valid = 1;
	objectToWorld(strat);
	calculateObjectPolyData(strat->hdBlock->collObj);
}

void ClearCollGlobals()
{
	int i;
	noWaterStrats = 0;
	noLavaStrats = 0;
	for (i=0;i<32;i++)
	{
		waterStrat[i]=NULL;
		lavaStrat[i] = NULL;
	}

	for (i=0;i<MAX_CP_BUFFER;i++)
		cpBufferPointer[i] = NULL;
	for (i=0;i<MAX_CP_BUFFER * 2;i++)
		cpBuffer[i].x = cpBuffer[i].y = cpBuffer[i].z = 0.0f;
	
	
   	GlobalCollP.x = GlobalCollP.y = GlobalCollP.z = 0.0f;
	GlobalPolyType = AIR;
	nActiveCollisions = 0;
	if (PAL)
		Gravity = PAL_GRAVITY;
	else
		Gravity = NTSC_GRAVITY;
  
	
	for (i=0;i<MAX_ACTIVE_COLLISIONS;i++)
	{
		collArray[i].normal.x = collArray[i].normal.y = collArray[i].normal.z =0.0f;
		collArray[i].pos.x = collArray[i].pos.y = collArray[i].pos.z = 0.0f;
		collArray[i].type = 0;
	}
}


void HurtStrat (STRAT *hurtee, STRAT *hurter)
{
	int otherPlayer;
	float prevHealth = hurtee->health, dam;

	// Check for damage being zero
	if ((dam = hurter->damage) == 0.f)
		return;

	if ((hurtee->Player) && (hurtee->Player->PlayerSecondaryWeapon.type == STORMING_SHIELD))
		return;

	/*BIG DAMAGE THEN FLAG THE HURTEE STRAT*/
	if (hurter->damage > 100.0f)
		hurtee->flag2 |= STRAT2_BIGDAMAGE;

	if (!Multiplayer)
	{
		if (hurtee->Player) 
		{
			extern int GameDifficulty;
			if (!(hurtee->Player->reward & CHEAT_INVUL)) {
				float newDamage = (hurter->damage + (hurter->damage * hurtee->Player->engineHeat * hurtee->Player->engineHeat));
				hurtee->health -=  newDamage;
				if (GameDifficulty) // Twice as much damage - tricky innit
					hurtee->health -=  newDamage;

			}
		}
		else
			hurtee->health -= hurter->damage;
	}
	else
		hurtee->health -= hurter->damage;
//MATT TEST
	if ((hurter->flag & STRAT_BULLET) && (GetSem(hurter, 0) != 1))
		hurter->damage = 0.f;

	otherPlayer = GetPlayerNumber (hurtee);
	if (otherPlayer >= 0) {
		// Make a noise
	   	//?????EL DODGO ?
	  //	int sound = sfxPlay (GENERIC_HITMETALFX, RandRange (0.7,0.9), -fabs(frand() * 0.05f));
		// Wiggle the pad about a bit
		if (hurtee->health > 0)
			Vibrate (hurtee, 5, 1.f, RANGE(0.2, dam / 40.f, 1.f), 0.1f + RANGE(0, dam / 15.f, 2) * 0.3f);
		else
			Vibrate (hurtee, 10, 1.f, 1, 0.5f);

		
		// Check the multiplayer game type
		if (Multiplayer) {
			// Are we playing a co-operative game and are the two strats involved on the
			// same team?
			if (player[(int)hurter->var].team == player[otherPlayer].team && player[otherPlayer].team &&
				!mpSettings.game.team.friendlyFire) {
				hurtee->health += dam;
				return;
			}

			// Increase 'number of things hit'
			player[(int)hurter->var].shotsHit++;

			switch (GetCurrentGameType()) {
			case KING_OF_THE_HILL:
				if (player[(int)hurter->var].special && player[(int)hurter->var].timer == 1) {
					hurtee->health = prevHealth - ((prevHealth - hurtee->health) / (float)NumberOfPlayers);
				}
				// falls to:
			default:
normalHurt:
				if ((hurtee->health <= 0.f) &&
					(prevHealth > 0.f)) {
					int pNum = (int)hurter->var;
					if (pNum >= 0 && pNum < 4) {
						if (pNum == otherPlayer)
							KillSelf (pNum);
						else
							KillPlayer (pNum, otherPlayer);
					}
				}
				break;
			case TAG:
			case SUICIDE_TAG:
				if (player[otherPlayer].invuln || player[(int)hurter->var].special == player[otherPlayer].special) {
					hurtee->health = prevHealth;
				} else {
					hurtee->health = prevHealth - ((prevHealth - hurtee->health) * 2);
				}
				hurtee->health = MAX (hurtee->health, 10.f); // Make sure the player isn't killed
				break;
			case PREDATOR:
			case KEEP_THE_FLAG:
				// Normal hurting warez
				if (player[(int)hurter->var].special || hurtee->Player->special) 
					goto normalHurt;
				// No hurting here
				hurtee->health = prevHealth;
				break;
			}
		}
	}
}

static void storeCPRec(Object *obj)
{
	int	i;

	if (obj->ncp)
	{
		cpBufferPointer[cpBufferObjCount * 2] = (void *)obj;
		cpBufferPointer[cpBufferObjCount * 2 + 1] = (void *)&cpBuffer[cpBufferCpCount];
		cpBufferObjCount++;
		dAssert(cpBufferObjCount < MAX_CP_BUFFER / 2,"storeCP : objCount > MAX_CP_BUFFER / 2");
	}

	for (i=0; i<obj->ncp; i++)
	{
		cpBuffer[cpBufferCpCount] = obj->ocpt[i];
		cpBufferCpCount++;
		dAssert(cpBufferCpCount < MAX_CP_BUFFER * 2,"storeCP : cpCount > MAX_CP_BUFFER * 2");
	}

	for (i=0; i<obj->no_child; i++)
		storeCPRec(&obj->child[i]);
}

static void storeCP(STRAT *strat)
{
	if (!strat)
		return;
	if (!strat->obj)
		return;
	cpBufferObjCount = 0;
	cpBufferCpCount = 0;
	storeCPRec(strat->obj);
}

static Point3 *getCP(Object *obj, int cpNo)
{
	int	i;
	Point3 *cpArray;

	for (i=0; i<MAX_CP_BUFFER; i+=2)
		if ((Object *)cpBufferPointer[i] == obj)
			break;

	dAssert(i<MAX_CP_BUFFER, "getCP : i > MAX_CP_BUFFER");
	cpArray = cpBufferPointer[i+1];
	return &cpArray[cpNo];
}

float pointLineDistance(Point3 *point, Point3 *lStart, Point3 *lEnd)
{
	float	lambda;
	Vector3	lDir, temp;
	Point3	pOnL;

	v3Sub(&lDir, lEnd, lStart);
	v3Sub(&temp, point, lStart);
	lambda = v3Dot(&lDir, &temp) / v3Dot(&lDir, &lDir);
	lambda = RANGE(0.0f, lambda, 1.0f);
	v3Scale(&lDir, &lDir, lambda);
	return pDist(&temp, &lDir);
}


float pointLineDistanceFix(Point3 *point, Point3 *lStart, Point3 *lEnd)
{
	float	lambda;
	Vector3	lDir, temp;
	Point3	pOnL;

	v3Sub(&lDir, lEnd, lStart);
	v3Sub(&temp, point, lStart);
	lambda = v3Dot(&lDir, &temp) / v3Dot(&lDir, &lDir);
	if ((lambda < 0.0f) || (lambda > 1.0f))
		return 1000.0f;

	v3Scale(&lDir, &lDir, lambda);
	return pDist(&temp, &lDir);
}

static float pointLineDistancePL(Point3 *point, Point3 *lStart, Point3 *lEnd, Point3 *pl)
{
	float	lambda;
	Vector3	lDir, temp;
	Point3	pOnL;

	v3Sub(&lDir, lEnd, lStart);
	v3Sub(&temp, point, lStart);
	lambda = v3Dot(&lDir, &temp) / v3Dot(&lDir, &lDir);
	lambda = RANGE(0.0f, lambda, 1.0f);
	v3Scale(&lDir, &lDir, lambda);
	v3Add(&pOnL, lStart, &lDir);
	*pl = pOnL;
	v3Sub(&temp, &pOnL, point);
	return (float) sqrt(v3Dot(&temp, &temp));
}

static float pointLineSqrDistance(Point3 *point, Point3 *lStart, Point3 *lEnd)
{
	Vector3	lDir, temp;
	Point3	pOnL;
	float	lambda;

	v3Sub(&lDir, lEnd, lStart);
	v3Sub(&temp, point, lStart);
	lambda = v3Dot(&lDir, &temp) / v3Dot(&lDir, &lDir);
	lambda = RANGE(0.0f, lambda, 1.0f);
	v3Scale(&lDir, &lDir, lambda);
	v3Add(&pOnL, lStart, &lDir);
	v3Sub(&temp, &pOnL, point);
	return (v3Dot(&temp, &temp));
}

static void vCollide(CollLeaf *leaf, Vector3 *vcfrom, Vector3 *vcto, int collFlag)
{
	register float	vctempL;
	register float	vcA, vcB, vcC, vcD;
	float leafHeight;
	register int			vcno_poly, vcpoly_no;
	Vector3	*n;

	dAssert(vcto->x < 10000.0, "INF");
	if ((vcfrom->x != vcto->x) || (vcfrom->y != vcto->y) || (vcfrom->z != vcto->z))
	{
		leafHeight = Uint16ToFloat (leaf->height);
		if (vcfrom->z > leafHeight && vcto->z > leafHeight)
			return;

		vcno_poly = leaf->noPoly;

		for (vcpoly_no=0; vcpoly_no < vcno_poly; vcpoly_no++)
		{
			vccrntPolyNo = leaf->polyNumber[vcpoly_no];
			vccrntPoly = &Savmap->collPoly[vccrntPolyNo];
		
			if ((vccrntPoly->flag & ALLCOLLIDE) != ALLCOLLIDE)
			{
				if (collFlag)
				{
					if (!(vccrntPoly->flag & collFlag))
						continue;
				}
			}
			
			n = &vccrntPoly->n;
			vcD = vccrntPoly->D;
			
			if (v3Dot(vcto, n) + vcD > 0.01f)
				continue;

			vctempL = v3Dot(vcfrom, n) + vcD;

			if (vctempL < -0.01f)
				continue;

			v3Sub(&vcfromTo, vcto, vcfrom);
			if (v3Dot(&vcfromTo, &vcfromTo) < 0.0001f)
				continue;

			if (v3Dot(&vcfromTo, n) >= 0.0f)
				continue;

			v3Scale(&vcfromTo, &vcfromTo, -vctempL / v3Dot(n, &vcfromTo));
			v3Add(&vcpp, vcfrom, &vcfromTo);

			vcA = n->x;
			vcB = n->y;
			vcC = n->z;

			for (vci=0; vci<3; vci++)
			{
				vcvs = &Savmap->collLookup[(vccrntPoly->v[vci])]->p;
				if (vci == 2)
					vcve = &Savmap->collLookup[(vccrntPoly->v[0])]->p;
				else
					vcve = &Savmap->collLookup[(vccrntPoly->v[vci + 1])]->p;
				v3Sub(&vcve_vs, vcvs, vcve);
				v3Sub(&vcvs_pp, &vcpp, vcvs);
				INSIDE_POLY;
			}	

			if (vci < 3)
				continue;

			v3Sub(&vctempV, &vcpp, vcto);
			v3Inc(&collVect, &vctempV);
			dAssert(collVect.x == collVect.x, "qnan");
			//v3Inc(&gcpn, &vccrntPoly->n); not sure xxx
			gcpn = vccrntPoly->n;
	   		collide++;
			

			GlobalCollP = vcpp;

			if (vccrntPoly->flag)
				GlobalPolyType = vccrntPoly->flag;

			GlobalCollPolyN.x = vcA;
			GlobalCollPolyN.y = vcB;
			GlobalCollPolyN.z = vcC;
			v3Inc(vcto, &vctempV);
			continue;

		}
		v3Scale(&collVect, &collVect, 1.005f);
	}
}

static void vCollideS(CollObject *leaf, Vector3 *vcfrom, Vector3 *vcto, int type, Uint32 flag)
{
	register float	vctempL;
	register float	vcA, vcB, vcC, vcD;
	float leafHeight, tempf;
	register int			vcno_poly, vcpoly_no;
	Vector3	*n;


	if ((vcfrom->x != vcto->x) || (vcfrom->y != vcto->y) || (vcfrom->z != vcto->z))
	{
		vcno_poly = leaf->nPoly;

		for (vcpoly_no=0; vcpoly_no < vcno_poly; vcpoly_no++)
		{
			vccrntPoly = &leaf->poly[vcpoly_no];
		
			n = &vccrntPoly->n;
			if (!(n->x < 1.1f))
				continue;

			vcD = vccrntPoly->D;
			
			if (v3Dot(vcto, n) + vcD > 0.01f)
				continue;

			vctempL = v3Dot(vcfrom, n) + vcD;

			if (vctempL < -0.01f)
				continue;

			v3Sub(&vcfromTo, vcto, vcfrom);
			if (v3Dot(&vcfromTo, &vcfromTo) < 0.0001f)
				continue;
			if (v3Dot(&vcfromTo, n) > 0.0f)
				continue;


			tempf = v3Dot(n, &vcfromTo);
			if (tempf == 0.0f)
				continue;

			v3Scale(&vcfromTo, &vcfromTo, -vctempL / tempf);
			v3Add(&vcpp, vcfrom, &vcfromTo);

			vcA = n->x;
			vcB = n->y;
			vcC = n->z;

			
			for (vci=0; vci<3; vci++)
			{
				vcvs = (Point3 *)((char *)leaf->v + (vccrntPoly->v[vci]));
				if (vci == 2)
					vcve = (Point3 *)((char *)leaf->v + (vccrntPoly->v[0]));
				else
					vcve = (Point3 *)((char *)leaf->v + (vccrntPoly->v[vci+1]));
				v3Sub(&vcve_vs, vcvs, vcve);
				v3Sub(&vcvs_pp, &vcpp, vcvs);
				INSIDE_POLY;
			}	

			if (vci < 3)
				continue;

			v3Sub(&vctempV, &vcpp, vcto);
			if (!(flag & OBJECT_NOTSOLID))
			{
				v3Inc(&collVect, &vctempV);
				dAssert(collVect.x == collVect.x, "qnan");
				v3Inc(vcto, &vctempV);
			}
	   		collide++;
			

			GlobalCollP = vcpp;

//			if (vccrntPoly->flag)
//				GlobalPolyType = GET_SURF_TYPE(vccrntPoly->flag);

			GlobalCollPolyN.x = vcA;
			GlobalCollPolyN.y = vcB;
			GlobalCollPolyN.z = vcC;
			continue;

		}
		v3Scale(&collVect, &collVect, 1.005f);
	}
}

//int NumCollisions = 0, CollTime = 0;
static void vSCollide(CollLeaf *leaf, Vector3 *from,  Vector3 *to, float	radius, int vsc, int collFlag)
{
	register int		poly_no;
	register CollPoly	*crntPoly, *polyArray;
	register float		A, B, C, *suck;
	float				D;
	register float		toX, toY, toZ, fromX, fromY, fromZ;
	float				leafHeight;

	unsigned short		*polyNumberArray;
	int					crntBoxNo, i;
	Point3				pp, ppn, *vs, *ve, crntCentre, pOnL;
	Vector3				ve_vs, vs_pp, tempV, lDir, ft, *n;
	float				lambda, tempL, tempL2;


	if ((from->x != to->x) || (from->y != to->y) || (from->z != to->z))
	{

		if (vsc > 5)
			return;

		leafHeight = Uint16ToFloat (leaf->height) + radius;
		if (from->z > leafHeight && to->z > leafHeight)
			return;

		// Read the 'to' in
		suck = &to->x;
		toX = *suck++;
		toY = *suck++;
		toZ = *suck++;

		polyArray = Savmap->collPoly;
		polyNumberArray = leaf->polyNumber;
		prefetch (polyNumberArray);

		// Read the 'from' in
		suck = &from->x;
		fromX = *suck++;
		fromY = *suck++;
		fromZ = *suck++;

		for (poly_no=leaf->noPoly; poly_no != 0; poly_no--)
		{
			// MattG changed: now reads sequentially
			crntPoly = &polyArray[*polyNumberArray];
			// Prefetch and increment polyNumberArray
			prefetch (++polyNumberArray); // Pre-increment to prefetch *next* poly number

			if ((crntPoly->flag & ALLCOLLIDE) != ALLCOLLIDE)
			{
				if (collFlag)
				{
					if (!(crntPoly->flag & collFlag))
						continue;	
				}
			}

			n = &crntPoly->n;

			// Read the normal into A, B, and C
			suck = &n->x;
			A = *suck++;
			B = *suck++;
			C = *suck++;

			D = *suck++;/*crntPoly->D;*/
			
			// Distance from 'to' to plane check
			tempL2 = toX*A + toY*B + toZ*C + D;
	// was: tempL2 = v3Dot(n, to) + D;
			if (tempL2 > radius)
				continue;

			// Distance from 'from' to plane check
			if ((A*fromX + B*fromY + C*fromZ + D) < -radius)
				continue;
	//was:	if (v3Dot(from, n) + D < -radius)
	//			continue;


			// Backface cull the polygon w.r.t. the vector from 'from' to 'to'
			// This is an expanded dot-product with (to - from)
			// It gives the compiler a chance to optimize a bit better - the inline
			// function handling is wazz
			if ((A*(toX - fromX) + B*(toY - fromY) + C*(toZ - fromZ)) > 0.f)
				continue;


	/*Was:	v3Sub(&ft, to, from);
			if (v3Dot(&ft, &crntPoly->n) > 0.0f)
				continue;
	*/
			// v3 functions expanded here
			pp.x = A * -tempL2 + toX;
			pp.y = B * -tempL2 + toY;
			pp.z = C * -tempL2 + toZ;
	//was:	v3Scale(&pp, n, -tempL2);
	//		v3Inc(&pp, to);

	//NumCollisions++;

			for (i=0; i<3; i++)
			{
				vs = &Savmap->collLookup[(crntPoly->v[i])]->p;
				prefetch (vs);
				if (i == 2)
					ve = &Savmap->collLookup[(crntPoly->v[0])]->p;
				else
					ve = &Savmap->collLookup[(crntPoly->v[i + 1])]->p;
				v3Sub(&ve_vs, vs, ve);
				v3Sub(&vs_pp, &pp, vs);
				INSIDE_POLY2;
			}	


			if (i < 3)
			{
				for (i=0; i<3; i++)
				{
					vs = &Savmap->collLookup[(crntPoly->v[i])]->p;
					if (i == 2)
						ve = &Savmap->collLookup[(crntPoly->v[0])]->p;
					else
						ve = &Savmap->collLookup[(crntPoly->v[i + 1])]->p;

					v3Sub(&lDir, ve, vs);
					v3Sub(&tempV, to, vs);
					lambda = v3Dot(&lDir, &tempV) / v3Dot(&lDir, &lDir);
					lambda = RANGE(0.0f, lambda, 1.0f);
					v3Scale(&lDir, &lDir, lambda);
					v3Add(&pOnL, vs, &lDir);
					v3Sub(&tempV, to, &pOnL);
					tempL = v3Dot(&tempV, &tempV);
					if (tempL > radius * radius)
						continue;
					else
					{
						tempL = radius - (sqrt(tempL));
						v3Normalise(&tempV);
						v3Scale(&tempV, &tempV, tempL);
						v3Inc(to, &tempV);
						toX += tempV.x;
						toY += tempV.y;
						toZ += tempV.z;
						v3Inc(&collVect, &tempV);
						dAssert(collVect.x == collVect.x, "qnan");
						if ((GET_SURF_TYPE(GlobalPolyType) == ROCK) || (GET_SURF_TYPE(GlobalPolyType) == AIR))
							GlobalPolyType = crntPoly->flag;
						if (!collide)
						{
							GlobalCollP = pp;
							
							GlobalCollPolyN.x = A;
							GlobalCollPolyN.y = B;
							GlobalCollPolyN.z = C;
						}	
						collide++;
						vccrntPoly = crntPoly;
						v3Inc(&gcpn, &vccrntPoly->n);
						if (!((crntPoly->n.z > 0.1f) || (crntPoly->n.z < -0.1f)))
							v3Inc(&wallN, &vccrntPoly->n);
					}
				}	
			}
			else
			{
				tempL2 -= radius;
				collVect.x -= tempL2 * A;
				collVect.y -= tempL2 * B;
				collVect.z -= tempL2 * C;
				dAssert(collVect.x == collVect.x, "qnan");
				to->x -= tempL2 * A;
				to->y -= tempL2 * B;
				to->z -= tempL2 * C;
				toX -= tempL2 * A;
				toY -= tempL2 * B;
				toZ -= tempL2 * C;
				if ((GET_SURF_TYPE(GlobalPolyType) == ROCK) || (GET_SURF_TYPE(GlobalPolyType) == AIR))
							GlobalPolyType = crntPoly->flag;
				if (!collide)
				{
					
  					GlobalCollP.x = pp.x;
					GlobalCollP.y = pp.y;
					GlobalCollP.z = pp.z;
					GlobalCollPolyN.x = A;
					GlobalCollPolyN.y = B;
					GlobalCollPolyN.z = C;
				}
				collide++;	
				vccrntPoly = crntPoly;
				v3Inc(&gcpn, &vccrntPoly->n);
				if (!((crntPoly->n.z > 0.1f) || (crntPoly->n.z < -0.1f)))
					v3Inc(&wallN, &vccrntPoly->n);
			}
		}
	}
}

static void vSCollideS(CollObject *leaf, Vector3 *from,  Vector3 *to, float	radius, Uint32 flag)
{
	register int		poly_no;
	register CollPoly	*crntPoly;
	register float		A, B, C, D, *suck;
	register float		toX, toY, toZ, fromX, fromY, fromZ;
	float				leafHeight;

	int					crntBoxNo, i;
	Point3				pp, ppn, *vs, *ve, crntCentre, pOnL;
	Vector3				ve_vs, vs_pp, tempV, lDir, ft, *n;
	float				lambda, tempL, tempL2;

	if ((from->x != to->x) || (from->y != to->y) || (from->z != to->z))
	{
		// Read the 'to' in
		suck = &to->x;
		toX = *suck++;
		toY = *suck++;
		toZ = *suck++;

		// Set up the loop counter
		crntPoly = leaf->poly;

		// Read the 'from' in
		suck = &from->x;
		fromX = *suck++;
		fromY = *suck++;
		fromZ = *suck++;

		// MattG: standard count down on poly_no, and crntPoly increments along the list
		for (poly_no=leaf->nPoly; poly_no != 0; poly_no--, crntPoly++)
		{
			n = &crntPoly->n;
			prefetch(n);

			//dAssert((n->x < 1.1f), "Some mad QNan happened!");
			if (!(n->x < 1.1f))
				continue;

			// Read the normal into A, B, and C
			suck = &n->x;
			A = *suck++;
			B = *suck++;
			C = *suck++;

			D = *suck++; /*crntPoly->D*/;	// And get D too
			
			// Distance from 'to' to plane check
			tempL2 = toX*A + toY*B + toZ*C + D;
			if (tempL2 > radius)
				continue;

	/*	it was:tempL2 = v3Dot(n, to) + D;
			if (tempL2 > radius)
				continue;
			*/

			// Distance from 'from' to plane check
			if ((A*fromX + B*fromY + C*fromZ + D) < -radius)
				continue;

	/* it was:
			if (v3Dot(from, n) + D < -radius)
				continue;
	*/
			// Backface cull the polygon w.r.t. the vector from 'from' to 'to'
			// This is an expanded dot-product with (to - from)
			// It gives the compiler a chance to optimize a bit better - the inline
			// function handling is wazz
			if (!(leaf->obj->collFlag & OBJECT_NOCULL))
				if ((A*(toX - fromX) + B*(toY - fromY) + C*(toZ - fromZ)) > 0.f)
					continue;

	/* it was:
			v3Sub(&ft, to, from);
			if (v3Dot(&ft, &crntPoly->n) > 0.0f)
				continue;
	*/
			crntPoly->flag = 0;

			// v3 functions expanded here
			pp.x = A * -tempL2 + toX;
			pp.y = B * -tempL2 + toY;
			pp.z = C * -tempL2 + toZ;

	/*was:	v3Scale(&pp, n, -tempL2);
			v3Inc(&pp, to);
	*/
			for (i=0; i<3; i++)
			{
				vs = (Point3 *)((char *)leaf->v + (crntPoly->v[i]));
				prefetch (vs);
				if (i == 2)
					ve = (Point3 *)((char *)leaf->v + (crntPoly->v[0]));
				else
					ve = (Point3 *)((char *)leaf->v + (crntPoly->v[i+1]));
				v3Sub(&ve_vs, vs, ve);
				v3Sub(&vs_pp, &pp, vs);
				INSIDE_POLY2;
			}	

			if (i < 3)
			{
				for (i=0; i<3; i++)
				{
					vs = (Point3 *)((char *)leaf->v + (crntPoly->v[i]));
					if (i == 2)
						ve =(Point3 *)((char *)leaf->v + (crntPoly->v[0]));
					else
						ve = (Point3 *)((char *)leaf->v + (crntPoly->v[i+1]));

					v3Sub(&lDir, ve, vs);
					v3Sub(&tempV, to, vs);
					lambda = v3Dot(&lDir, &tempV) / v3Dot(&lDir, &lDir);
					lambda = RANGE(0.0f, lambda, 1.0f);
					v3Scale(&lDir, &lDir, lambda);
					v3Add(&pOnL, vs, &lDir);
					v3Sub(&tempV, to, &pOnL);
					tempL = v3Dot(&tempV, &tempV);
					if (tempL > radius * radius)
						continue;
					else
					{
						tempL = radius - (sqrt(tempL));
						v3Normalise(&tempV);
						v3Scale(&tempV, &tempV, tempL);
						if (flag & OBJECT_NOTSOLID)
						{
						}
						else
						{
							v3Inc(to, &tempV);
							toX += tempV.x;
							toY += tempV.y;
							toZ += tempV.z;
							v3Inc(&collVect, &tempV);
							dAssert(collVect.x == collVect.x, "qnan");
						}
						if (!collide)
						{
							GlobalCollP = pp;
							//GlobalPolyType = crntPoly->flag;
							GlobalCollPolyN.x = A;
							GlobalCollPolyN.y = B;
							GlobalCollPolyN.z = C;
						}	
						collide++;
						vccrntPoly = crntPoly;
						v3Inc(&gcpn, &vccrntPoly->n);
						if (!((crntPoly->n.z > 0.1f) || (crntPoly->n.z < -0.1f)))
							v3Inc(&wallN, &vccrntPoly->n);
					}
				}	
			}
			else
			{
				tempL2 -= radius;
				if (flag & OBJECT_NOTSOLID)
				{
				}
				else
				{
					collVect.x -= tempL2 * A;
					collVect.y -= tempL2 * B;
					collVect.z -= tempL2 * C;
					dAssert(collVect.x == collVect.x, "qnan");
					to->x -= tempL2 * A;
					to->y -= tempL2 * B;
					to->z -= tempL2 * C;
					toX -= tempL2 * A;
					toY -= tempL2 * B;
					toZ -= tempL2 * C;
				}
				if (!collide)
				{
					//GlobalPolyType = crntPoly->flag;
  					GlobalCollP.x = pp.x;
					GlobalCollP.y = pp.y;
					GlobalCollP.z = pp.z;
					GlobalCollPolyN.x = A;
					GlobalCollPolyN.y = B;
					GlobalCollPolyN.z = C;
				}
				collide++;	
				vccrntPoly = crntPoly;
				v3Inc(&gcpn, &vccrntPoly->n);
				if (!((crntPoly->n.z > 0.1f) || (crntPoly->n.z < -0.1f)))
					v3Inc(&wallN, &vccrntPoly->n);
			}
		}
	}
}

static void mInvertRot(Matrix *a, Matrix *b)
{
	mUnit(b);
	b->m[0][0] = a->m[0][0];
	b->m[0][1] = a->m[1][0];
	b->m[0][2] = a->m[2][0];
	b->m[0][3] = 0.0f;

	b->m[1][0] = a->m[0][1];
	b->m[1][1] = a->m[1][1];
	b->m[1][2] = a->m[2][1];
	b->m[1][3] = 0.0f;

	b->m[2][0] = a->m[0][2];
	b->m[2][1] = a->m[1][2];
	b->m[2][2] = a->m[2][2];
	b->m[2][3] = 0.0f;

	b->m[3][0] = 0.0f;
	b->m[3][1] = 0.0f;
	b->m[3][2] = 0.0f;
	b->m[3][3] = 1.0f;
}

static void doStratSpecificColl(STRAT *crntStrat, Object *obj, Vector3 *relForce, Vector3*force, Point3 *forceP)
{
	void	(*specProc)(STRAT *crntStrat, Object *obj, Vector3 *relForce, Vector3*force, Point3 *forceP);

	// Record the collision point first, if necessary
	if (obj->collFlag & OBJECT_RECORD_COLL) {
		obj->collFlag = OBJECT_CLEAR_COLL_ID(obj->collFlag);
		if (nActiveCollisions < (MAX_ACTIVE_COLLISIONS-1)) {
			++nActiveCollisions; // One ahead so collision 0 means 'no collision'
			collArray[nActiveCollisions].pos = GlobalCollP;
			collArray[nActiveCollisions].normal = GlobalCollPolyN;
			collArray[nActiveCollisions].type = GlobalPolyType;
			obj->collFlag |= OBJECT_MAKE_COLL_ID(nActiveCollisions);
		}
	}

	dAssert (crntStrat->vel.x == crntStrat->vel.x, "QNAN");

	if (crntStrat->flag & STRAT_EXPLODEPIECE)
	{
		specProc = collSpecProc[0];
		specProc(crntStrat, obj, relForce, force, forceP);
	}
	else if (crntStrat->pnode)
	{
		specProc = collSpecProc[crntStrat->pnode->typeId];
		specProc(crntStrat, obj, relForce, force, forceP);
	}
	dAssert (crntStrat->vel.x == crntStrat->vel.x, "QNAN");
}
 
static void doHighVSColl(STRAT *crntStrat, CollObject *co, Point3 *ocpt, Point3 *cpt, float radius, HDStrat *hds)
{
	int rc = 0, i;
	Vector3	move;
	Point3	op;

	//MATTP CHANGE SO CHILDREN ARE CHECKED BEFORE ROOT OBJS 9 TIMES OUT OF 10, THIS
	//IS WHAT YOU'LL WANT CHECKED FIRST
	for (i=0; i<co->obj->no_child; i++)
		doHighVSColl(crntStrat, co->child[i], ocpt, cpt, radius, hds);
  
	rc = collide;

	v3Sub(&move, &hds->strat->pos, &hds->strat->oldPos);
	op = *ocpt;
	v3Inc(&op, &move);

	if ((co->obj->model) && !(co->obj->collFlag & OBJECT_NOCOLLISION))
	{
	 	if (!((co->obj->collFlag & OBJECT_NOCAMERACOLLIDE) && (crntStrat == player[currentPlayer].CameraStrat)))
		{
			if (!((crntStrat->flag & STRAT_BULLET) && (co->obj->collFlag & OBJECT_NOBULLETCOLLIDE)))
			{
				if (!((co->obj->collFlag & OBJECT_NOPLAYERCOLLIDE) && (crntStrat->Player)))
					vSCollideS(co, &op, cpt, radius, co->obj->collFlag);
			}
		}
	}

	if (rc != collide)
	{	
		hds->strat->CollWith = crntStrat;
		crntStrat->CollWith = hds->strat;
		if (crntStrat->parent)
			if (!MyParentInvalid(crntStrat))
				if (crntStrat->parent->parent == hds->strat->parent)
					co->obj->collFlag |= OBJECT_SHOT_BY_SIBLING;

		if (crntStrat->Player)
		{
			co->obj->collFlag |= OBJECT_HIT_PLAYER;
			//crntStrat->objId[1]->collFlag |= COLL_ON_FLOOR;
			//crntStrat->objId[2]->collFlag |= COLL_ON_FLOOR;
			//crntStrat->objId[3]->collFlag |= COLL_ON_FLOOR;
			//crntStrat->objId[4]->collFlag |= COLL_ON_FLOOR;
			if (!(co->obj->collFlag & OBJECT_NOTSOLID))
				crntStrat->Player->PlayerOnHoldPlayer = hds;

			hds->strat->flag2 |= STRAT2_PLAYERHIT;
		}
	   	
		crntStrat->flag2 |= STRAT2_HITHDC;
		if (!(crntStrat->flag & STRAT_BULLET))
		{
			crntStrat->flag |= STRAT_HITFLOOR;
		}
		
		if (hds->strat == EscortTanker)
		{
			if (!EscortTankerHit[0])
			{
				EscortTankerHit[0] = crntStrat;
			}
			else if (!EscortTankerHit[1])
			{
				EscortTankerHit[1] = crntStrat;
			}
			else if (!EscortTankerHit[2])
			{
				EscortTankerHit[2] = crntStrat;
			}
		}
		else if (crntStrat == EscortTanker)
		{
			if (!EscortTankerHit[0])
			{
				EscortTankerHit[0] = hds->strat;
			}
			else if (!EscortTankerHit[1])
			{
				EscortTankerHit[1] = hds->strat;
			}
			else if (!EscortTankerHit[2])
			{
				EscortTankerHit[2] = hds->strat;
			}
		}

		//hds->strat->flag |= COLL_HITSTRAT;
		
		if (((hds->strat->flag & STRAT_FRIENDLY) && (crntStrat->flag & STRAT_ENEMY)) ||
			((hds->strat->flag & STRAT_ENEMY) && (crntStrat->flag & STRAT_FRIENDLY)))
		{
			crntStrat->flag |= COLL_HITSTRAT;
			hds->strat->flag |= COLL_HITSTRAT;
			co->obj->collFlag |= COLL_HITSTRAT;
			co->obj->health -= crntStrat->damage;
			
			//ONLY HURT THE STRAT IF VALID OBJECT TO HIT
			if (!(co->obj->collFlag & OBJECT_NOTVALIDTOHURT))
			{
				HurtStrat (hds->strat, crntStrat);
				HurtStrat (crntStrat, hds->strat);
			}
		}
	}

	//for (i=0; i<co->obj->no_child; i++)
   //		doHighVSColl(crntStrat, co->child[i], ocpt, cpt, radius, hds);
}

void doHighVColl(STRAT *crntStrat, CollObject *co, Point3 *ocpt, Point3 *cpt, HDStrat *hds, int type)
{
	int i, x;
	int rc = 0;

	//MATTP CHANGE SO CHILDREN ARE CHECKED BEFORE ROOT OBJS 9 TIMES OUT OF 10, THIS
	//IS WHAT YOU'LL WANT CHECKED FIRST
	for (i=0; i<co->obj->no_child; i++)
		doHighVColl(crntStrat, co->child[i], ocpt, cpt, hds, 0);
 
	rc = collide;

	if ((co->obj->model)  && !(co->obj->collFlag & OBJECT_NOCOLLISION))
	{
		if (crntStrat)
		{
			if (!((co->obj->collFlag & OBJECT_NOCAMERACOLLIDE) && (crntStrat == player[currentPlayer].CameraStrat)))
			{
				if (!((crntStrat->flag & STRAT_BULLET) && (co->obj->collFlag & OBJECT_NOBULLETCOLLIDE)))
				{
					if (!((co->obj->collFlag & OBJECT_NOPLAYERCOLLIDE) && (crntStrat->Player)))
						vCollideS(co, ocpt, cpt, type, co->obj->collFlag);
				}
			}
		}
		else
			vCollideS(co, ocpt, cpt, type, co->obj->collFlag);
	}

	if (rc != collide)
	{
		if (caIndex < 32)
		{
			collArrayOrigin[caIndex] = *ocpt;
			collArrayStrat[caIndex] = hds->strat;
			collArrayPos[caIndex] = GlobalCollP;
			collArrayObject[caIndex] = co->obj;
			caIndex++;
		}
	}
	/*if (crntStrat)
	{
		if (rc != collide)
		{	
			hds->strat->CollWith = crntStrat;
			crntStrat->CollWith = hds->strat;
			if (crntStrat->parent)
				if (crntStrat->parent->parent == hds->strat->parent)
					co->obj->collFlag |= OBJECT_SHOT_BY_SIBLING;
		

			if (crntStrat->Player)
			{
				crntStrat->objId[1]->collFlag |= COLL_ON_FLOOR;
				crntStrat->objId[2]->collFlag |= COLL_ON_FLOOR;
				crntStrat->objId[3]->collFlag |= COLL_ON_FLOOR;
				crntStrat->objId[4]->collFlag |= COLL_ON_FLOOR;
				crntStrat->Player->PlayerOnHoldPlayer = hds;
				hds->strat->flag2 |= STRAT2_PLAYERHIT;
			}
			crntStrat->flag2 |= STRAT2_HITHDC;
	   		if (!(crntStrat->flag & STRAT_BULLET))
			{
				crntStrat->flag |= STRAT_HITFLOOR;
			}
		
			if (((hds->strat->flag & STRAT_FRIENDLY) && (crntStrat->flag & STRAT_ENEMY)) ||
				((hds->strat->flag & STRAT_ENEMY) && (crntStrat->flag & STRAT_FRIENDLY)))
			{
				crntStrat->flag |= COLL_HITSTRAT ;
				hds->strat->flag |= COLL_HITSTRAT;
				co->obj->collFlag |= COLL_HITSTRAT;
				co->obj->health -= crntStrat->damage;

				//ONLY HURT THE STRAT IF VALID OBJECT TO HIT
				if (!(co->obj->collFlag & OBJECT_NOTVALIDTOHURT))
				{
					HurtStrat (hds->strat, crntStrat);
					HurtStrat (crntStrat, hds->strat);
				}
			}
		}
	}*/

   //	for (i=0; i<co->obj->no_child; i++)
   //		doHighVColl(crntStrat, co->child[i], ocpt, cpt, hds, 0);
}

static void AddToObjectWMRec(Object *obj, Vector3 *v)
{
	int i;

	for (i=0; i<obj->no_child; i++)
		AddToObjectWMRec(&obj->child[i], v);

	obj->wm.m[3][0] += v->x;
	obj->wm.m[3][1] += v->y;
	obj->wm.m[3][2] += v->z;
}

void AddToObjectWM(STRAT *strat, Vector3 *v)
{
	AddToObjectWMRec(strat->obj, v);
}

static void ObjectCollisionFloor(STRAT *crntStrat, Object *obj, Matrix *rotM, CollData *cd)
{
	Uint32	child_no, cp_no, gridX, gridY;
	Point3	*cpt, *ocpt, waterP, tempP, *tp;
	Vector3	iCollVect, to, waterV, tempv, tempv2, lavaV, lavaP;
	Matrix	im;
	Model *model;
	int		subX, subY, wi, rc = 0, i, cax;
	CollNode *cpNode;
	CollLeaf *oldNode;
	HDStrat	*hds;
	float	dl, dw, cx, cy, cpx, cpy, tempf, caDist;
	STRAT *hdcStrat = NULL;
	CP2 *collPoint;


	for(child_no=0; child_no<obj->no_child; child_no++)
	{
		mPush(rotM);
		mPreRotateXYZ(rotM, obj->crntRot.x, obj->crntRot.y, obj->crntRot.z);
		ObjectCollisionFloor( crntStrat, &obj->child[child_no], rotM, cd);
		mPop(rotM);
	}

	if (obj->collFlag & OBJECT_NOCOLLISION)
		return;

	// MattG added:
	// collPoint points to the current collision point, 
	// ocpt points to old collsion point and cp_no is just a counter
	// which now counts down
	collPoint = obj->cp;
	ocpt = obj->ocpt;
	for(cp_no=obj->ncp; cp_no != 0; cp_no--, collPoint++, ocpt++)
	{
		
		mPoint3Multiply3(&collPoint->worldPos, &collPoint->centre, &obj->wm);
		cpt = &collPoint->worldPos;
	  //	DrawMarker(cpt->x, cpt->y, cpt->z);
		
		if ((cpt->x < Savmap->xMin - 20.0f) || (cpt->x > Savmap->xMax + 20.0f) || (cpt->y < Savmap->yMin - 20.0f) || (cpt->y > Savmap->yMax + 20.0f))
			return;



		collide = 0;
		collVect.x = 0.0f;
		collVect.y = 0.0f;
		collVect.z = 0.0f;

		if (collPoint->radius > 0.f)
		{
			if (!(crntStrat->flag2 & STRAT2_NOHDC))
			{
				hds = firstHDStrat;
				while(hds)
				{
					if ((hds->strat->flag & STRAT_COLLSTRAT)
						&& (hds->strat != crntStrat->parent)
						&& (hds->strat->parent != crntStrat)
						&& (hds->strat != crntStrat)
						&& (hds->valid))
					{
						if ((hds->strat == iceLiftGun) && (crntStrat->flag & STRAT_ENEMY))
						{
							hds = hds->next;
							continue;
						}

						if (pSqrDist(&hds->strat->pos, &crntStrat->pos) < SQR(hds->strat->pnode->radius + crntStrat->pnode->radius + 10.0))
						{
							if (!POINT3EQUAL(ocpt, cpt))
								doHighVSColl(crntStrat, hds->collObj, ocpt, cpt, collPoint->radius, hds);
						}
					}
					hds = hds->next;
				}
			}
		}
		else
		{
			if (!(crntStrat->flag2 & STRAT2_NOHDC))
			{
				caIndex = 0;
				hds = firstHDStrat;
				while(hds)
				{
					if ((hds->strat->flag & STRAT_COLLSTRAT)
						&& (hds->strat != crntStrat->parent)
						&& (hds->strat->parent != crntStrat)
						&& (hds->strat != crntStrat)
						&& (hds->valid))
					{
						if (pSqrDist(&hds->strat->pos, &crntStrat->pos) < SQR(hds->strat->pnode->radius + crntStrat->pnode->radius + 15.0))
						{
							if (!POINT3EQUAL(ocpt, cpt))
								doHighVColl(crntStrat, hds->collObj, ocpt, cpt, hds, 0);
						}
					}
					hds = hds->next;
				}
			}
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

			collArrayStrat[cax - 1]->CollWith = crntStrat;
			crntStrat->CollWith = collArrayStrat[cax - 1];

			if (crntStrat->parent)
				if (crntStrat->parent->parent == collArrayStrat[cax - 1]->parent)
					collArrayObject[cax - 1]->collFlag |= OBJECT_SHOT_BY_SIBLING;
		
			crntStrat->flag2 |= STRAT2_HITHDC;
	   		if (!(crntStrat->flag & STRAT_BULLET))
			{
				crntStrat->flag |= STRAT_HITFLOOR;
			}
		
			if (((collArrayStrat[cax - 1]->flag & STRAT_FRIENDLY) && (crntStrat->flag & STRAT_ENEMY)) ||
				((collArrayStrat[cax - 1]->flag & STRAT_ENEMY) && (crntStrat->flag & STRAT_FRIENDLY)))
			{
				crntStrat->flag |= COLL_HITSTRAT ;
				collArrayStrat[cax - 1]->flag |= COLL_HITSTRAT;
				collArrayObject[cax - 1]->collFlag |= COLL_HITSTRAT;
				collArrayObject[cax - 1]->health -= crntStrat->damage;

				//ONLY HURT THE STRAT IF VALID OBJECT TO HIT
				if (!(collArrayObject[cax - 1]->collFlag & OBJECT_NOTVALIDTOHURT))
				{
					HurtStrat (collArrayStrat[cax - 1], crntStrat);
					HurtStrat (crntStrat, collArrayStrat[cax - 1]);
				}
			}
		}

		cpx = cpt->x;
		cpy = cpt->y;
		cx = cd->cx;
		cy = cd->cy;
		dl = cd->dl;
		dw = cd->dw;
		cpNode = cd->node;
		v3Sub(&tempv, ocpt, cpt);

		//if (v3SqrLength(&tempv) > 0.000001f)
		if(crntStrat->flag & STRAT_COLLFLOOR)
		{
			
			
			while(cpNode->left)
			{
				if (dl > dw)
				{
					dl *= 0.5f;
					if (cpx > cx + dl)
					{
						cx += dl;
						cpNode = cpNode->right;
					}
					else
						cpNode = cpNode->left;
				}
				else
				{
					dw *= 0.5f;
					if (cpy > cy + dw)
					{
						cy += dw;
						cpNode = cpNode->left;
					}
					else
						cpNode = cpNode->right;
				}
			}
			vccrntPoly = NULL;
		
			oldNode = (CollLeaf *)cpNode->right;
			// MattG added: prefetch the polygon information in while we fiddle around down here
			if (oldNode->polyNumber)
				prefetch (oldNode->polyNumber);
			if (collPoint->radius > 0.f) 
			{
				if (((CollLeaf *)cpNode->right)->noPoly)
					vSCollide((CollLeaf *)cpNode->right, ocpt, cpt, collPoint->radius, 0 , crntStrat->flag2 & ALLCOLLIDE);

				//sav change
				if (collide)
					if ((GET_SURF_TYPE(crntStrat->polyType) == AIR) || (GET_SURF_TYPE(crntStrat->polyType) == ROCK))
						crntStrat->polyType = GlobalPolyType;
				
				
			} 
			else 
			{
				dAssert(cpt->x == cpt->x, "qnan");
				dAssert(cpt->x < 10000.0, "INF");
				vCollide((CollLeaf *)cpNode->right, ocpt, cpt, crntStrat->flag2 & ALLCOLLIDE);
				
			}
		}

		
		if (collide)
		{   
			//crntStrat->polyType = vccrntPoly->flag;
			v3Inc(cpt, &collVect);
			mInvertRot(rotM, &im);
			mVector3Multiply(&iCollVect, &collVect, &im);
			v3Mul(&iCollVect, &iCollVect, &crntStrat->scale);

			if (crntStrat->flag & STRAT_BULLET)
			{
				if (!(crntStrat->flag & COLL_HITSTRAT))
					obj->collFlag |= (COLL_ON_FLOOR | OBJECT_CALC_MATRIX);
			}
			else
			{
				obj->collFlag |= (COLL_ON_FLOOR | OBJECT_CALC_MATRIX);
			}

			dAssert(collVect.x == collVect.x, "qnan");
			doStratSpecificColl(crntStrat, obj, &iCollVect, &collVect, cpt);
			if (crntStrat->flag & STRAT_BULLET) 
			{
				if (crntStrat->flag2 & STRAT2_HITHDC) 
					addSpark(20, &crntStrat->pos, &crntStrat->vel, 0.8, 5.0, 20, 1, 0); 
				else
					addSpark(10, &crntStrat->pos, &crntStrat->vel, 0.8, 5.0, 3, 1, 0); 
			}
		}
		//obj->ocpt[cp_no] = *cpt;
		if ((crntStrat->Player) || (crntStrat == HoldPlayerMain) || (crntStrat == player[0].CameraStrat) ||
			(crntStrat == BoatStrat) || ((crntStrat->parent) && (!(MyParentInvalid(crntStrat))) && (crntStrat->parent->Player)))
		{
			for (wi = 0; wi<noWaterStrats; wi++)
			{
				if (waterStrat[wi])
				{
					if (!waterStrat[wi]->pnode)
						continue;
					if (!waterStrat[wi]->pnode->obj)
						continue;
				 
					model = waterStrat[wi]->obj->model;
					if (model)
					{
						//v3Sub(&tempv, cpt, &waterStrat[wi]->pos);
						tempv = *cpt;
						mShortPoint3Multiply3(&tempP, &tempv, &waterStrat[wi]->obj->im); 
					}
					else
					{
						model = waterStrat[wi]->obj->child[0].model;
						if (model)
						{
							//v3Sub(&tempv, cpt, &waterStrat[wi]->pos);
							tempv = *cpt;
							mShortPoint3Multiply3(&tempP, &tempv, &waterStrat[wi]->obj->child[0].im); 
						}
						else
							continue;
					}
				
					if ((tempP.x > model->bounds.low.x) && 
						(tempP.x < model->bounds.hi.x) && 
						(tempP.y > model->bounds.low.y) && 
						(tempP.y < model->bounds.hi.y))
					{
						/*if ((crntStrat == player[0].CameraStrat) && (cpt->z < waterStrat[wi]->pos.z + 1.0))
						{
							crntStrat->pos.z = waterStrat[wi]->pos.z + 1.0f;
						}
						else */if (crntStrat == BoatStrat)
						{
							tempf = waterStrat[wi]->pos.z + WatNoise(cpt->x, cpt->y) + obj->cp[0].radius + 1.0f;
							if (cpt->z < tempf)
							{
								//crntStrat->pos.z = waterStrat[wi]->pos.z;
								obj->collFlag |= COLL_ON_WATER;
							
								waterV.x = 0.0f;
								waterV.y = 0.0f;
								waterV.z = (tempf - cpt->z) * crntStrat->pnode->mass * 0.005f;
								v3Sub(&waterP, cpt, &crntStrat->pos);
								ApplyForce(crntStrat, &waterV, &waterP, crntStrat->pnode->collForceRatio * 2.0f);
								crntStrat->vel.z *= 0.98f;
								if (boatPAC < 4)
								{
									boatPA[boatPAC] = *cpt;
									boatPAC++;
								}
							}
						}
						else
						{
							if ((cpt->z < waterStrat[wi]->pos.z) ||
								((cpt->z - obj->cp[0].radius < waterStrat[wi]->pos.z) && (OBJECT_GET_ID(obj->collFlag) < 5)))
							{
								if (crntStrat->Player)
								{							
									if (OBJECT_GET_ID(obj->collFlag) < 5) 
									{
										v3Scale(&tempv, &crntStrat->vel, -1.0f);
										if (tempv.z < 0.0f)
											tempv.z *= -1.0;

										tempv.z += 0.1f;
										tempf = v3Length(&crntStrat->vel) * 0.1f;
										v3Rand(&tempv2);
										v3Scale(&tempv2, &tempv2, 0.4f);
										v3Inc(&tempv2, cpt);
										tempv2.z = waterStrat[wi]->pos.z;
										if (tempf > 0.01)
											addSpark(5, &tempv2, &tempv, tempf, 5.0, 10, 5, 0);
									}
									else
									{
										if (crntStrat->pos.z + 1.0 < waterStrat[wi]->pos.z)
										{
											crntStrat->vel.x *= 0.95f;
											crntStrat->vel.y *= 0.95f;
											crntStrat->vel.z *= 0.95f;
										}
										waterV.x = 0.0f;
										waterV.y = 0.0f;
										waterV.z = (waterStrat[wi]->pos.z - cpt->z) * crntStrat->pnode->mass * 0.003f;
										v3Sub(&waterP, cpt, &crntStrat->pos);
			 							ApplyForce(crntStrat, &waterV, &waterP, crntStrat->pnode->collForceRatio);
									}
								}
							
		  						obj->collFlag |= COLL_ON_WATER;

								if (crntStrat->pos.z < waterStrat[wi]->pos.z)
								{
									crntStrat->flag |= STRAT_UNDER_WATER;
									if (waterStrat[wi]->IVar[2])
										crntStrat->health -= 3.0f;

									if (crntStrat == player[0].CameraStrat)
									{
										if (waterStrat[wi]->IVar[1] == 1)
										{
											SetScreenARGB(0.5f, 0.0f, 0.5f, 0.0);
										}
										else
										{
											SetScreenARGB(0.5f, 0.2f, 0.2f, 0.8f);
										}
									}
								}


								if ((ocpt->z > waterStrat[wi]->pos.z) && (noWaterCP < 63))
								{
									noWaterCP++;
				  					water[noWaterCP-1].pos.x = cpt->x;
				  					water[noWaterCP-1].pos.y = cpt->y;
				  					water[noWaterCP-1].pos.z = waterStrat[wi]->pos.z;	
								}
							}
							else if ((ocpt->z < waterStrat[wi]->pos.z) && (noWaterCP < 63))
							{
								noWaterCP++;
			   					water[noWaterCP-1].pos.x = cpt->x;
			   					water[noWaterCP-1].pos.y = cpt->y;
			   					water[noWaterCP-1].pos.z = waterStrat[wi]->pos.z;	
							}
						}
					}
				}
			}

			for (wi = 0; wi<noLavaStrats; wi++)
			{
				if (!lavaStrat[wi])
					continue;

				dAssert(lavaStrat[wi]->pnode, "Water Strat has no pnode???\n");
				dAssert(lavaStrat[wi]->pnode->obj, "Water Strat has no Parent Object???\n");
				 
				model = lavaStrat[wi]->obj->model;
				if (!model)
				{
					model = lavaStrat[wi]->obj->child[0].model;
				}

				if (!model)
					continue;

				v3Sub(&tempv, cpt, &lavaStrat[wi]->pos);
				mShortPoint3Multiply3(&tempP, &tempv, &lavaStrat[wi]->obj->im); 
				
				if ((tempP.x > model->bounds.low.x) && 
					(tempP.x < model->bounds.hi.x) && 
					(tempP.y > model->bounds.low.y) && 
					(tempP.y < model->bounds.hi.y))
				{
					
					if (cpt->z - obj->cp[0].radius < lavaStrat[wi]->pos.z)
					{
						
						/*if (OBJECT_GET_ID(obj->collFlag) < 5) 
						{
							v3Scale(&tempv, &crntStrat->vel, -1.0f);
							if (tempv.z < 0.0f)
								tempv.z *= -1.0;

							tempv.z += 0.1f;
							tempf = v3Length(&crntStrat->vel) * 0.1f;
							v3Rand(&tempv2);
							v3Scale(&tempv2, &tempv2, 0.4f);
							v3Inc(&tempv2, cpt);
							tempv2.z = lavaStrat[wi]->pos.z;
							if (tempf > 0.01)
								addSpark(5, &tempv2, &tempv, tempf, 5.0, 10, 5, 0);
						}*/
						
		  				obj->collFlag |= OBJECT_IN_LAVA;	
					}
				}
			}
		}
	}
}

static void clearFloorCollFlag(STRAT *crntStrat, Object *obj)
{
	int	i;

	for (i=0; i<obj->no_child; i++)
		clearFloorCollFlag(crntStrat, &obj->child[i]);

	obj->collFlag &= ~COLL_ON_FLOOR;
}

static void setFloorCollFlag(STRAT *crntStrat, Object *obj)
{
	int	i;
	for (i=0; i<obj->no_child; i++)
		setFloorCollFlag(crntStrat, &obj->child[i]);

	if (obj->collFlag & COLL_ON_FLOOR)
		crntStrat->flag |= (STRAT_HITFLOOR | COLL_ON_FLOOR); 

	if (obj->collFlag & COLL_ON_WATER)
		crntStrat->flag |= COLL_ON_WATER;

/*	for (i=0; i<noWaterStrats; i++)
		if (crntStrat->pos.z < waterStrat[i]->pos.z)
			crntStrat->flag |= STRAT_UNDER_WATER;*/
}

static void clearStratCollFlagRec(Object *obj)
{
	int	i;

	for (i=0; i<obj->no_child; i++)
		clearStratCollFlagRec(&obj->child[i]);

	obj->collFlag &= ~(COLL_HITSTRAT | COLL_ON_WATER | OBJECT_INVMATRIX | OBJECT_SHOT_BY_SIBLING | COLL_ON_FLOOR | OBJECT_HIT_PLAYER);

	
}

void clearStratCollFlag(STRAT *crntStrat)
{
	crntStrat->flag &= ~(COLL_HITSTRAT | COLL_ON_FLOOR | STRAT_HITFLOOR | STRAT_UNDER_WATER | COLL_ON_WATER);
	crntStrat->flag2 &= ~(STRAT2_SPIN_LEFT | STRAT2_SPIN_RIGHT | STRAT2_PLAYERHIT | STRAT2_HITHDC);
//	crntStrat->spinFlag = 0;
	clearStratCollFlagRec(crntStrat->obj);
}
 
static void StratCollisionFloor(STRAT *crntStrat)
{
	Matrix	m;
	
	CollData cd;
	CollNode	*node;
	float	cx, cy, dl, dw, sx, sy, radius;

	m = crntStrat->m;
	mPreScale(&m, crntStrat->scale.x, crntStrat->scale.y, crntStrat->scale.z);
	mPostTranslate(&m, crntStrat->pos.x, crntStrat->pos.y, crntStrat->pos.z);
	
	if (crntStrat->pnode)
	{
		if (crntStrat->flag & STRAT_COLLFLOOR)
		{
			radius = crntStrat->pnode->radius + 7.0f;
			node = Savmap->topNode;
			cx = Savmap->xMin;
			cy = Savmap->yMin;
			sx = crntStrat->pos.x;
			sy = crntStrat->pos.y;
			dl = Savmap->xMax - Savmap->xMin;
			dw = Savmap->yMax - Savmap->yMin;
			while(node->left)
			{
				if (dl > dw)
				{
					dl *= 0.5f;
					if ((sx - radius) > (cx + dl))
					{
						cx += dl;
						node = node->right;
					}
					else if ((sx + radius) < (cx + dl))
						node = node->left;
					else
					{
						dl *= 2.0f;
						break;
					}
				}
				else
				{
					dw *= 0.5f;
					if ((sy - radius) > (cy + dw))
					{
						cy += dw;
						node = node->left;
					}
					else if ((sy + radius) < (cy + dw))
						node = node->right;
					else
					{
						dw *= 2.0f;
						break;
					}
				}
			}
			cd.dl = dl;
			cd.dw = dw;
			cd.cx = cx;
			cd.cy = cy;
			cd.node = node;
		}
		GlobalPolyType = AIR;
		if (crntStrat->Player)
			if (GET_SURF_TYPE(crntStrat->polyType) != AIR)
				crntStrat->Player->oldPolyType = crntStrat->polyType;
		crntStrat->polyType = AIR;
		ObjectCollisionFloor( crntStrat, crntStrat->obj, &m, &cd);
	}
}

static float lineLineDistance(Point3 *l1s, Point3 *l1e, Point3 *l2s, Point3 *l2e)
{
	float A, B, C, D, K, lambda, lambda2;
	Vector3	l1d, l2d, dv, temp;
	Point3	pl1, pl2;

	lambda = pSqrDist(l2s, l2e);

	if (pSqrDist(l1s, l1e) < 0.01f)
	{	
		if (lambda < 0.01f)
		{
			v3Sub(&pl1, l1s, l2s);
			return sqrt(v3Dot(&pl1, &pl1));
		}
		return pointLineDistance(l1s, l2s, l2e);
	}
	else if (lambda < 0.01f)
		return pointLineDistance(l2s, l1s, l1e);

	v3Sub(&l1d, l1e, l1s);
	v3Sub(&l2d, l2e, l2s);
	A = v3Dot(&l1d, &l2d);
//	if (!A)
//		return 1.0f;

	B = v3Dot(&l2d, &l2d);
	K = A / v3Dot(&l1d, &l1d);
	if ((A * K - B) == 0.0f)
		return 1.0f;
	
	v3Sub(&temp, l1s, l2s);
	C = K * v3Dot(&l1d, &temp);
	D = v3Dot(&l2d, &temp);
	lambda2 = (C - D) / (A * K - B);
 
	if (!A)
		lambda = 1.0f;
	else
	{
		lambda = (lambda2 * B - D) / A;
		lambda = RANGE(0.0f, lambda, 1.0f);
	}
	lambda2 = RANGE(0.0f, lambda2, 1.0f);
	v3Scale(&l1d, &l1d, lambda);
	v3Scale(&l2d, &l2d, lambda2);
	v3Add(&pl1, l1s, &l1d);
	v3Add(&pl2, l2s, &l2d);
	v3Sub(&dv, &pl2, &pl1);

	return (float) sqrt(v3Dot(&dv, &dv));

}

static float lineLineDistancePL(Point3 *l1s, Point3 *l1e, Point3 *l2s, Point3 *l2e, Point3 *mpl1, Point3 *mpl2)
{
	float A, B, C, D, K, lambda, lambda2;
	Vector3	l1d, l2d, dv, temp;
	Point3	pl1, pl2;

	lambda = pSqrDist(l2s, l2e);

	if (pSqrDist(l1s, l1e) < 0.01f)
	{	
		if (lambda < 0.01f)
		{
			*mpl1 = *l1s;
			*mpl2 = *l2s;
			v3Sub(&pl1, l1s, l2s);
			return sqrt(v3Dot(&pl1, &pl1));

		}
		*mpl1 = *l1s;
		return pointLineDistancePL(l1s, l2s, l2e, mpl2);
	}
	else if (lambda < 0.01f)
	{
		*mpl2 = *l2s;
		return pointLineDistancePL(l2s, l1s, l1e, mpl1);
	}

	v3Sub(&l1d, l1e, l1s);
	v3Sub(&l2d, l2e, l2s);
	A = v3Dot(&l1d, &l2d);
	B = v3Dot(&l2d, &l2d);
	K = A / v3Dot(&l1d, &l1d);
	v3Sub(&temp, l1s, l2s);
	C = K * v3Dot(&l1d, &temp);
	D = v3Dot(&l2d, &temp);
	lambda2 = (C - D) / (A * K - B);
	lambda = (lambda2 * B - D) / A;
	lambda = RANGE(0.0f, lambda, 1.0f);
	lambda2 = RANGE(0.0f, lambda2, 1.0f);
	v3Scale(&l1d, &l1d, lambda);
	v3Scale(&l2d, &l2d, lambda2);
	v3Add(&pl1, l1s, &l1d);
	v3Add(&pl2, l2s, &l2d);
	v3Sub(&dv, &pl2, &pl1);
	*mpl1 = pl1;
	*mpl2 = pl2;

	return (float) sqrt(v3Dot(&dv, &dv));

}

static int lineModelCollision(Point3 *ps, Point3 *pe, Model *model, Vector3 *move, float radius)
{	
	int		i, collFace = 0;
	Point3	pp, s, e, pl1, pl2;
	Vector3	ld;
	float	lambda, lx, ly, lz, hx, hy, hz, dist, tempf;

	
	lx = model->bounds.low.x;
	ly = model->bounds.low.y;
	lz = model->bounds.low.z;
	hx = model->bounds.hi.x;
	hy = model->bounds.hi.y;
	hz = model->bounds.hi.z;

	if (((ps->x - radius > hx ) && (pe->x - radius > hx)) ||
		((ps->y - radius > hy ) && (pe->y - radius > hy)) ||
		((ps->z - radius > hz ) && (pe->z - radius > hz)) ||
		((ps->x + radius < lx) && (pe->x + radius < lx)) ||
		((ps->y + radius < ly) && (pe->y + radius < ly)) ||
		((ps->z + radius < lz) && (pe->z + radius < lz)))
		return 0;

	for (i=0; i<6; i++)
	{
		switch(i)
		{
			case 0:
				if ((ps->y - radius > ly) || (pe->y < ps->y))
					continue;

				tempf = (pe->y - ps->y);
				lambda = (ly - ps->y) / tempf;
				v3Sub(&ld, pe, ps);
				v3Scale(&ld, &ld, lambda);
				v3Inc(&ld, ps);
				if ((ld.x > lx) &&
					(ld.x < hx) &&
					(ld.z > lz) &&
					(ld.z < hz))
				{
					move->x = move->z = 0.0f;
					move->y = (ly - (pe->y + radius)) - COLL_VECTOR_ADD;
					collFace = 1;
				}
				break;
			case 1:
				if ((ps->x + radius< hx) || (pe->x > ps->x))
					continue;

				tempf = (pe->x - ps->x);
				lambda = (hx - ps->x) / tempf;
				v3Sub(&ld, pe, ps);
				v3Scale(&ld, &ld, lambda);
				v3Inc(&ld, ps);
				if ((ld.y > ly) &&
					(ld.y < hy) &&
					(ld.z > lz) &&
					(ld.z < hz))
				{
					move->y = move->z = 0.0f;
					move->x = (hx - (pe->x - radius)) + COLL_VECTOR_ADD;
					collFace = 2;
				}
				break;
			case 2:
				if ((ps->y + radius < hy) || (pe->y > ps->y))
					continue;

				tempf = (pe->y - ps->y);
				lambda = (hy - ps->y) / tempf;
				v3Sub(&ld, pe, ps);
				v3Scale(&ld, &ld, lambda);
				v3Inc(&ld, ps);
				if ((ld.x > lx) &&
					(ld.x < hx) &&
					(ld.z > lz) &&
					(ld.z < hz))
				{
					move->x = move->z = 0.0f;
					move->y = (hy - (pe->y - radius)) + COLL_VECTOR_ADD;
					collFace = 3;
				}
				break;
			case 3:
				if ((ps->x - radius > lx) || (pe->x < ps->x))
					continue;
				tempf = (pe->x - ps->x);
				lambda = (lx - ps->x) / tempf;
				v3Sub(&ld, pe, ps);
				v3Scale(&ld, &ld, lambda);
				v3Inc(&ld, ps);
				if ((ld.y > ly) &&
					(ld.y < hy) &&
					(ld.z > lz) &&
					(ld.z < hz))
				{
					move->y = move->z = 0.0f;
					move->x = (lx - (pe->x + radius)) - COLL_VECTOR_ADD;
					collFace = 4;
				}
				break;
			case 4:
				if ((ps->z + radius < hz) || (pe->z > ps->z))
					continue;
				tempf = (pe->z - ps->z);
				lambda = (hz - ps->z) / tempf;
				v3Sub(&ld, pe, ps);
				v3Scale(&ld, &ld, lambda);
				v3Inc(&ld, ps);
				if ((ld.x > lx) &&
					(ld.x < hx) &&
					(ld.y > ly) &&
					(ld.y < hy))
				{
					move->x = move->y = 0.0f;
					move->z = (hz - (pe->z - radius)) + COLL_VECTOR_ADD;
					collFace = 5;
				}
				break;
			case 5:
				if ((ps->z - radius > lz) || (pe->z < ps->z))
					continue;
				tempf = (pe->z - ps->z);
				lambda = (lz - ps->z) / tempf;
				v3Sub(&ld, pe, ps);
				v3Scale(&ld, &ld, lambda);
				v3Inc(&ld, ps);
				if ((ld.x > lx) &&
					(ld.x < hx) &&
					(ld.y > ly) &&
					(ld.y < hy))
				{
					move->x = move->y = 0.0f;
					move->z = (lz - (pe->z + radius)) - COLL_VECTOR_ADD;
					collFace = 6;
				}
				break;
		}
		if (collFace)
			break;
	}

	if (1)
	{
		for (i=0; i<12; i++)
		{
			switch(i)
			{
				case 0:
					s.x = lx; s.y = ly; s.z = lz;
					e.x = hx; e.y = ly; e.z = lz;
					break;
				case 1:
					s.x = hx; s.y = ly; s.z = lz;
					e.x = hx; e.y = ly; e.z = hz;
					break;
				case 2:
					s.x = hx; s.y = ly; s.z = hz;
					e.x = lx; e.y = ly; e.z = hz;
					break;
				case 3:
					s.x = lx; s.y = ly; s.z = hz;
					e.x = lx; e.y = ly; e.z = lz;
					break;
				case 4:
					s.x = hx; s.y = ly; s.z = lz;
					e.x = hx; e.y = hy; e.z = lz;
					break;
				case 5:
					s.x = lx; s.y = ly; s.z = lz;
					e.x = lx; e.y = hy; e.z = lz;
					break;
				case 6:
					s.x = lx; s.y = ly; s.z = hz;
					e.x = lx; e.y = hy; e.z = hz;
					break;
				case 7:
					s.x = hx; s.y = ly; s.z = hz;
					e.x = hx; e.y = hy; e.z = hz;
					break;
				case 8:
					s.x = lx; s.y = hy; s.z = lz;
					e.x = hx; e.y = hy; e.z = lz;
					break;
				case 9:
					s.x = hx; s.y = hy; s.z = lz;
					e.x = hx; e.y = hy; e.z = hz;
					break;
				case 10:
					s.x = hx; s.y = hy; s.z = lz;
					e.x = lx; e.y = hy; e.z = lz;
					break;
				case 11:
					s.x = lx; s.y = hy; s.z = hz;
					e.x = lx; e.y = hy; e.z = lz;
					break;
			}
			
			dist = lineLineDistancePL(ps, pe, &s, &e, &pl1, &pl2);
			if (dist < radius)
			{
				v3Sub(&ld, &pl1, &pl2);
				lambda = radius - dist;

				v3Normalise(&ld);
				v3Scale(&ld, &ld, lambda);
				v3Inc(move, &ld);
				collFace = 1;
				break;
			}
		}
	}

	return collFace;
}

static int lineModelCollisionFast(Point3 *ps, Point3 *pe, Model *model, Vector3 *move, float radius)
{	
	int		i, collFace = 0;
	Point3	pp, s, e, pl1, pl2;
	Vector3	ld;
	float	lambda, lx, ly, lz, hx, hy, hz, dist;

	
	lx = model->bounds.low.x;
	ly = model->bounds.low.y;
	lz = model->bounds.low.z;
	hx = model->bounds.hi.x;
	hy = model->bounds.hi.y;
	hz = model->bounds.hi.z;

	if (((ps->x > hx ) && (pe->x > hx)) ||
		((ps->y > hy ) && (pe->y > hy)) ||
		((ps->z > hz ) && (pe->z > hz)) ||
		((ps->x < lx ) && (pe->x < lx)) ||
		((ps->y < ly ) && (pe->y < ly)) ||
		((ps->z < lz ) && (pe->z < lz)))
		return 0;

	v3Sub(&ld, pe, ps);

	for (i=0; i<6; i++)
	{
		switch(i)
		{
			case 0:
				if ((ps->y > ly) || (pe->y < ps->y))
					continue;
				lambda = (ly - ps->y) / ld.y;
				if ((lambda < 0.0f) || (lambda > 1.0f))
					continue;
				pp.x = ps->x + ld.x * lambda;
				pp.z = ps->z + ld.z * lambda;
				if ((pp.x > lx) &&
					(pp.x < hx) &&
					(pp.z > lz) &&
					(pp.z < hz))
					collFace = 1;
				break;
			case 1:
				if ((ps->x < hx) || (pe->x > ps->x))
					continue;
				lambda = (hx - ps->x) / ld.x;
				if ((lambda < 0.0f) || (lambda > 1.0f))
					continue;
				pp.y = ps->y + ld.y * lambda;
				pp.z = ps->z + ld.z * lambda;
				if ((pp.y > ly) &&
					(pp.y < hy) &&
					(pp.z > lz) &&
					(pp.z < hz))
					collFace = 2;
				break;
			case 2:
				if ((ps->y < hy) || (pe->y > ps->y))
					continue;
				lambda = (hy - ps->y) / ld.y;
				if ((lambda < 0.0f) || (lambda > 1.0f))
					continue;
				pp.x = ps->x + ld.x * lambda;
				pp.z = ps->z + ld.z * lambda;
				if ((pp.x > lx) &&
					(pp.x < hx) &&
					(pp.z > lz) &&
					(pp.z < hz))
					collFace = 3;
				break;
			case 3:
				if ((ps->x > lx) || (pe->x < ps->x))
					continue;
				lambda = (lx - ps->x) / ld.x;
				if ((lambda < 0.0f) || (lambda > 1.0f))
					continue;
				pp.y = ps->y + ld.y * lambda;
				pp.z = ps->z + ld.z * lambda;
				if ((pp.y > ly) &&
					(pp.y < hy) &&
					(pp.z > lz) &&
					(pp.z < hz))
					collFace = 4;
				break;
			case 4:
				if ((ps->z < hz) || (pe->z > ps->z))
					continue;
				lambda = (hz - ps->z) / ld.z;
				if ((lambda < 0.0f) || (lambda > 1.0f))
					continue;
				pp.x = ps->x + ld.x * lambda;
				pp.y = ps->y + ld.y * lambda;
				if ((pp.x > lx) &&
					(pp.x < hx) &&
					(pp.y > ly) &&
					(pp.y < hy))
					collFace = 5;
				break;
			case 5:
				if ((ps->z > lz) || (pe->z < ps->z))
					continue;
				lambda = (lz - ps->z) / ld.z;
				if ((lambda < 0.0f) || (lambda > 1.0f))
					continue;
				pp.x = ps->x + ld.x * lambda;
				pp.y = ps->y + ld.y * lambda;
				if ((pp.x > lx) &&
					(pp.x < hx) &&
					(pp.y > ly) &&
					(pp.y < hy))
					collFace = 6;
				break;
		}
		if (collFace)
			break;
	}

	return collFace;
}

int lineHDModelCollisionFast(Point3 *ps, Point3 *pe, CollObject *co, Vector3 *move, float radius)
{	
	int		i;
	Point3	pp, s, e, pl1, pl2;
	Vector3	ld;
	float	lambda, lx, ly, lz, hx, hy, hz, dist;

	collide = 0;
	vCollideS(co, ps, pe, 0, 0);

	if (collide > 0)
		return 1;
	else
		return 0;
}

static void fromOTWRec(Matrix *m, Object *obj, int a)
{
	if (obj->parent)
		fromOTWRec(m, obj->parent, a);

	if(a == 0)
	{
		mPreTranslate(m, obj->crntPos.x, obj->crntPos.y, obj->crntPos.z);
		mPreRotateXYZ(m, obj->crntRot.x, obj->crntRot.y, obj->crntRot.z);
		mPreScale(m, obj->scale.x, obj->scale.y, obj->scale.z);
		obj->wm = *m;
	}
	else if (a == 1)
		mPreRotateXYZ(m, obj->crntRot.x, obj->crntRot.y, obj->crntRot.z);
	else if (a == 2)
	{
		mPreRotateXYZ(m, obj->crntRot.x, obj->crntRot.y, obj->crntRot.z);
		mPreScale(m, obj->scale.x, obj->scale.y, obj->scale.z);
	}

}

void fromObjectToWorld(Matrix *m, STRAT *strat, Object *obj, int a)
{
	Matrix	m2;
	
	m2 = strat->m;
	if (a == 0)
	{
		mPostTranslate(&m2, strat->pos.x, strat->pos.y, strat->pos.z);
		mPreScale(&m2, strat->scale.x, strat->scale.y, strat->scale.z);
	}
	else if (a == 2)
		mPreScale(&m2, strat->scale.x, strat->scale.y, strat->scale.z);

	if (obj)
		fromOTWRec(&m2, obj, a);

	*m = m2;
}

static int cpObjectCollision(STRAT *lineStr, Object *lineStrObj, int cpNo, Object *obj, STRAT *strat)
{
	int		coll=0, i, lineMC = 0, hit = 0;
	Point3	transPS, transPE, collP, *ps, *pe, tempp, myps, mype;
	Vector3	cpMoveObj, cpMoveWorld, force, sCollVect, iSCollVect, vel, tempv;
	Matrix	m, im;
	

	if (!(obj->collFlag & OBJECT_NOCOLLISION))
	{
		if (obj->model)
		{
			v3Sub(&vel, &strat->pos, &strat->oldPos);
			cpMoveObj.x = cpMoveObj.y = cpMoveObj.z = 0.0f;
			ps = &lineStrObj->ocpt[cpNo];
			pe = &lineStrObj->cp[cpNo].worldPos;
			v3Add(&myps, ps, &vel);
			//v3Add(&mype, pe, &vel);
			//v3Inc(&myps, &HoldPlayerV);
			mype = *pe;
			//v3Add(&mype, ps, &vel);

			if (!(obj->collFlag & OBJECT_INVMATRIX))
			{
				obj->collFlag |= OBJECT_INVMATRIX;
				mShortInvert3d(&obj->im, &obj->wm);
			}
			mShortPoint3Multiply3(&transPS, &myps, &obj->im);
			mShortPoint3Multiply3(&transPE, &mype, &obj->im);

			lineMC = lineModelCollision(&transPS, &transPE, obj->model, &cpMoveObj, lineStrObj->cp[cpNo].radius);
			if (lineMC && (cpMoveObj.x || cpMoveObj.y || cpMoveObj.z))
			{
				v3Inc(&strat->wallN, &cpMoveObj);
				obj->collFlag |= COLL_HITSTRAT;
				hit = 1;
				
				if (Multiplayer)
				{
					for (i=0; i<4; i++)
					{
						if (lineStr == player[i].Player)
						{	
							//v3Scale(&tempv, &strat->vel, strat->pnode->mass); 
							if (strat->flag & STRAT_BULLET)
								v3Inc(&lineStr->Player->HitDir, &strat->vel);
							strat->flag2 |= STRAT2_PLAYERHIT;
						   //	strat->hitPlayerN = lineStr->Player->n;
							strat->flag2 &= ~3;
							strat->flag2 |= lineStr->Player->n;
						}
						else if (strat == player[i].Player)
						{
							//v3Scale(&tempv, &lineStr->vel, lineStr->pnode->mass); 
							if (lineStr->flag & STRAT_BULLET)
								v3Inc(&strat->Player->HitDir, &lineStr->vel);
							lineStr->flag2 |= STRAT2_PLAYERHIT;
						  //	lineStr->hitPlayerN = strat->Player->n;
							lineStr->flag2 &= ~3;
							lineStr->flag2 |= strat->Player->n;
						}
					}
				}
				else
				{
					if (lineStr == player[0].Player)
					{
						if (strat->flag & STRAT_BULLET)
							v3Inc(&lineStr->Player->HitDir, &strat->vel);
						strat->flag2 |= STRAT2_PLAYERHIT;
						strat->flag2 &= ~3;
						strat->flag2 |= lineStr->Player->n;
					}
					else if (strat == player[0].Player) 
					{
						if (lineStr->flag & STRAT_BULLET)
							v3Inc(&strat->Player->HitDir, &lineStr->vel);
						lineStr->flag2 |= STRAT2_PLAYERHIT;
					 
						lineStr->flag2 &= ~3;
						lineStr->flag2 |= strat->Player->n;
					}
					lineStrObj->health -= strat->damage;
					obj->health -= lineStr->damage;
				}
				
				if((strat->flag2 & STRAT2_COLL_MOVE) && (lineStr->flag2 & STRAT2_COLL_MOVE))
				{
					fromObjectToWorld(&m, strat, obj, MATRIX_RS);
					mVector3Multiply(&tempp, &cpMoveObj, &m);
					v3Inc(pe, &tempp);
					mVector3Multiply(&cpMoveWorld, &cpMoveObj, &m);
					sCollVect = cpMoveWorld;
					fromObjectToWorld(&m, lineStr, lineStrObj->parent, MATRIX_RS);
					mInvertRot(&m, &im);
					iSCollVect = cpMoveObj;
					//mVector3Multiply(&iSCollVect, &sCollVect, &im);
					dAssert(sCollVect.x == sCollVect.x, "qnan");
					doStratSpecificColl(lineStr, lineStrObj, &iSCollVect, &sCollVect, pe);
					lineStrObj->collFlag |= COLL_ON_FLOOR;
				}
			}
		}
	}

	for (i=0; i<obj->no_child; i++)
	{
		if (cpObjectCollision(lineStr, lineStrObj, cpNo, &obj->child[i], strat))
			hit = 1;
	}

	if (lineMC)
		hit = 1;

	return hit;
}	

static int objectCollisionStrat(STRAT *objStr, Object *obj, STRAT *strat, Vector3 *vel)
{
	int	i, hit = 0;
	
	for (i=0; i<obj->no_child; i++)
	{
		if (objectCollisionStrat(objStr, &obj->child[i], strat, vel))
		{
			hit = 1;
		}
	}

	for (i=0; i<obj->ncp; i++)
	{
		if (cpObjectCollision(objStr, obj, i, strat->obj, strat))
		{
			hit = 1;
		}
	}

	return hit;
}

static int detailedStratCollisionStrat(STRAT *crntStrat, STRAT *strat)
{
	Vector3	dist;

	if(!(crntStrat->flag2 && STRAT2_MATRIX))
	{
		objectToWorld(crntStrat);
		crntStrat->flag2 |= STRAT2_MATRIX;
	}
	if(!(strat->flag2 && STRAT2_MATRIX))
	{
		objectToWorld(strat);
		strat->flag2 |= STRAT2_MATRIX;
	}

	if (crntStrat->obj && strat->obj)
	{
		dist = crntStrat->vel;		
		return objectCollisionStrat(crntStrat, crntStrat->obj, strat, &dist);
	}
	
	return 0;
}
#if 0
float maxScale(STRAT *crntStrat)
{
	if (crntStrat->scale.x > crntStrat->scale.y)
	{
		if (crntStrat->scale.x > crntStrat->scale.z)
			return crntStrat->scale.x;
		else
			return crntStrat->scale.z;
	}
	else if (crntStrat->scale.y > crntStrat->scale.z)
		return crntStrat->scale.y;
	else
		return crntStrat->scale.z;
}

void singularMatrix(Matrix *m)
{
	Matrix	tempm;
	mInvert3d(&tempm, m);
}
#endif

int scsCheck(STRAT *strat, STRAT *crntStrat, COLLSTRAT *coll)
{
	
	if (!(strat->flag & STRAT_COLLSTRAT))
		return 1;
	
	if ((strat->hdBlock) || (!strat->pnode) || (strat == crntStrat))
		return 1;

	if (Multiplayer)
	{
		if ((strat->Player) && (crntStrat->Player))
			return 0;

		if ((strat == crntStrat->parent) || (crntStrat == strat->parent) || 
			(strat->parent == crntStrat->parent))
			return 1;

		if (((strat->parent) && (crntStrat == strat->parent->parent)) ||
			((crntStrat->parent) && (strat == crntStrat->parent->parent)))
			return 1;
	}
	else
	{
  		if(strat->flag & STRAT_FRIENDLY)
			return 1;
	}

	if (((strat->flag & STRAT_BULLET) && (crntStrat->flag2 & STRAT2_PICKUP)) ||
		((crntStrat->flag & STRAT_BULLET) && (strat->flag2 & STRAT2_PICKUP)))  
		return 1;

	if (((strat->flag & STRAT_BULLET) && (crntStrat->flag & STRAT_BULLET)))  
		return 1;

	/*WE COULD RECORD WHAT STRAT WAS COLLIDED WITH SO AS TO DISABLE THE COLLISION
	  CHECKS, AT THE MOMENT PLAYER-LAUNCHER,ETC WILL GET DONE TWICE PER FRAME*/

	return 0;
}

static void stratCollisionStrat(STRAT *crntStrat)
{
	register STRAT	*strat;
	float			distance, radius, tempf, caDist;
	Vector3			v, sToC, tv, c, tempv, tempv2, a;
	COLLSTRAT		*coll;
	int i, cax;

	coll = FirstColl;

	caIndex = 0;
	while (coll)
	{
		strat = coll->strat;
		
		if (scsCheck(strat, crntStrat, coll))
		{
			coll = coll->next;
			continue;
		}

		radius = (strat->pnode->radius * strat->scale.x) + (crntStrat->pnode->radius * crntStrat->scale.x); 	
		v3Sub(&v, &strat->pos, &crntStrat->pos);
		if (v3Length(&v) > radius + 20.0f)
		{
			coll = coll->next;
			continue;
		}

		if ((!Multiplayer) && (player[0].PlayerState == PS_ONTRAIN))
			distance = lineLineDistance(&strat->oldOldPos, &strat->oldPos, &crntStrat->oldOldPos, &crntStrat->oldPos);
		else
			distance = lineLineDistance(&strat->oldPos, &strat->pos, &crntStrat->oldPos, &crntStrat->pos);

		distance -= radius;

		if (distance < 10.0f)
		{
			if ((strat->flag & STRAT_LOW_DETAIL_COLLISION) || (crntStrat->flag & STRAT_LOW_DETAIL_COLLISION) || (strat->Player && crntStrat->Player))
			{
				if (distance < 0.0f)
				{
					if (caIndex < 32)
					{
						collArrayStrat[caIndex] = strat;	
						collArrayObject[caIndex] = NULL;
						collArrayOrigin[caIndex] = strat->oldPos;
						collArrayPos[caIndex] = strat->pos;
						caIndex++;
					}
					/********************************************************************/
					for (i = 0; i<4; i++)
					{
						if (strat == player[i].Player)
						{
							if (crntStrat->flag & STRAT_BULLET)
								v3Inc(&strat->Player->HitDir, &crntStrat->vel);
							crntStrat->flag2 |= STRAT2_PLAYERHIT;
						
							crntStrat->flag2 &= ~3;
							crntStrat->flag2 |= strat->Player->n;
						}
						else if (crntStrat == player[i].Player)
						{
						
							if (strat->flag & STRAT_BULLET)
								v3Inc(&crntStrat->Player->HitDir, &strat->vel);

							strat->flag2 |= STRAT2_PLAYERHIT;
						
							strat->flag2 &= ~3;
							strat->flag2 |= crntStrat->Player->n;
						}
					}

					if (strat->flag & STRAT_BULLET)
						addSpark(20, &strat->pos, &strat->vel, 0.8, 5.0, 20, 1, 0); 

					if (crntStrat->flag & STRAT_BULLET)
						addSpark(20, &crntStrat->pos, &crntStrat->vel, 0.8, 5.0, 20, 1, 0);

					if (crntStrat->flag & STRAT_IMPACTDIRECTION)
					{
						tv = strat->vel;
						
						tv.z = 0.0f;
						v3Sub(&sToC, &crntStrat->pos, &strat->pos);
						sToC.z = 0.0f;
						v3Cross(&c, &sToC, &tv);

						if (c.z < 0.0f)
							crntStrat->flag2 |= STRAT2_SPIN_LEFT;
						else
							crntStrat->flag2 |= STRAT2_SPIN_RIGHT;
					}

					if (strat->flag & STRAT_IMPACTDIRECTION)
					{
						tv = crntStrat->vel;
					
						tv.z = 0.0f;
						v3Sub(&sToC, &strat->pos, &crntStrat->pos);
						sToC.z = 0.0f;

						v3Cross(&c, &sToC, &tv);

						if (c.z < 0.0f)
							strat->flag2 |= STRAT2_SPIN_LEFT;
						else
							strat->flag2 |= STRAT2_SPIN_RIGHT;
					}				


					crntStrat->flag |= COLL_HITSTRAT;
					strat->flag |= COLL_HITSTRAT;
					crntStrat->CollWith = strat;
					strat->CollWith = crntStrat;

					if (!((strat->flag & STRAT_BULLET) || (crntStrat->flag & STRAT_BULLET)))
					{
						v3Normalise(&v);
						v3Scale(&v, &v, distance * 0.5f);
						if ((strat->flag2 & STRAT2_COLL_MOVE) && (crntStrat->flag2 & STRAT2_COLL_MOVE))
						{
							if (!(crntStrat->flag & STRAT_NOMOVE))
							{
								v3Inc(&crntStrat->pos, &v);
								if (v3SqrLength(&crntStrat->vel) < 0.0001f)
								{
									crntStrat->vel.x = crntStrat->vel.y = crntStrat->vel.z = 0.0;
									a.x = a.y = a.z = 0.0f;
								}
								else
								{
									if (strat->pnode->mass == 4001.0f)
									{
										tempv = crntStrat->vel;
										v3Normalise(&tempv);
										v3Sub(&tempv2, &strat->pos, &crntStrat->pos);
										v3Normalise(&tempv2);
										tempf = v3Length(&crntStrat->vel) * (v3Dot(&tempv, &tempv2));
										v3Scale(&tempv2, &tempv2, tempf * 0.6f);
										v3Dec(&crntStrat->vel, &tempv2);
										v3Scale(&tempv2, &tempv2, tempf * 1.6667f);
										a = tempv2;
									}
									else
									{
										tempv = crntStrat->vel;
										v3Normalise(&tempv);
										v3Sub(&tempv2, &strat->pos, &crntStrat->pos);
										v3Normalise(&tempv2);
										tempf = v3Length(&crntStrat->vel) * (v3Dot(&tempv, &tempv2));
										v3Scale(&tempv2, &tempv2, tempf);
										v3Dec(&crntStrat->vel, &tempv2);
										a = tempv2;
									}
								}
							}

							if (!(strat->flag & STRAT_NOMOVE))
							{
								v3Dec(&strat->pos, &v);
								if (v3SqrLength(&strat->vel) < 0.0001f)
								{
									strat->vel.x = strat->vel.y = strat->vel.z = 0.0f;
								}
								else
								{		
									if (crntStrat->pnode->mass == 4001.0f)
									{
										tempv = strat->vel;
										v3Normalise(&tempv);
										v3Sub(&tempv2, &crntStrat->pos, &strat->pos);
										v3Normalise(&tempv2);
										tempf = v3Length(&strat->vel) * (v3Dot(&tempv, &tempv2));
										v3Scale(&tempv2, &tempv2, tempf);
										v3Scale(&tempv2, &tempv2, 0.6f);
										v3Dec(&strat->vel, &tempv2);
										v3Inc(&strat->vel, &a);
										if (!(crntStrat->flag & STRAT_NOMOVE))
										{
											if (strat->pnode->mass == 4001.0f)
											{
												v3Inc(&crntStrat->vel, &tempv2);
											}
											else
											{
												v3Scale(&tempv2, &tempv2, 1.6667f);
												v3Inc(&crntStrat->vel, &tempv2);
											}
										}
									}
									else
									{

										tempv = strat->vel;
										v3Normalise(&tempv);
										v3Sub(&tempv2, &crntStrat->pos, &strat->pos);
										v3Normalise(&tempv2);
										tempf = v3Length(&strat->vel) * (v3Dot(&tempv, &tempv2));
										v3Scale(&tempv2, &tempv2, tempf);
										v3Dec(&strat->vel, &tempv2);
										v3Inc(&strat->vel, &a);
										if (!(crntStrat->flag & STRAT_NOMOVE))
										{
											if (strat->pnode->mass == 4001.0f)
											{
												v3Scale(&tempv2, &tempv2, 0.6f);
												v3Inc(&crntStrat->vel, &tempv2);
											}
											else
											{
												v3Inc(&crntStrat->vel, &tempv2);
											}
										}
									}
								}
							}	
						}
					}
					/*******************************************************************/
				}
			}
		}
		coll = coll->next;
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

		strat = collArrayStrat[cax - 1];

		
		if ((strat->damage) && (!(crntStrat->flag2 & STRAT2_NODAMAGE)))
		{
			HurtStrat (crntStrat, strat);
		}
		if ((crntStrat->damage) && (!(strat->flag2 & STRAT2_NODAMAGE)))
		{
			HurtStrat (strat, crntStrat);
		}
	}
}

static Object *lineObjectCollision(Point3 *from, Point3 *to, Object *obj, float dist, int pn)
{
	Point3	tFrom, tTo, objPos;
	Vector3	move;
	int		i;
	Object	*collObj, *closestObj;
	float	tempDist;

	closestObj = collObj = NULL;

	for (i=0; i<obj->no_child; i++)
	{
		collObj = lineObjectCollision(from, to, &obj->child[i], dist, pn);
		if (collObj)
		{
			objPos.x = collObj->wm.m[3][0];
			objPos.y = collObj->wm.m[3][1];
			objPos.z = collObj->wm.m[3][2];
			v3Dec(&objPos, from);
			tempDist = v3Length(&objPos);
			if (tempDist < dist)
			{
				dist = tempDist;
				closestObj = collObj;
			}
		}
	}

	
	if ((obj->model) && (!(obj->collFlag & OBJECT_INVISIBLE)) && (obj->collFlag & COLL_TARGETABLE))
	//if ((obj->model) && (!(obj->collFlag & OBJECT_INVISIBLE)))
	{
		if (!(obj->collFlag & OBJECT_INVMATRIX))
		{
			obj->collFlag |= OBJECT_INVMATRIX;
			mShortInvert3d(&obj->im, &obj->wm);
		}

		mShortPoint3Multiply3(&tFrom, from, &obj->im);
		mShortPoint3Multiply3(&tTo, to, &obj->im);

		if (lineModelCollisionFast(&tFrom, &tTo, obj->model, &move, 0.0f))
		{
			objPos.x = obj->wm.m[3][0];
			objPos.y = obj->wm.m[3][1];
			objPos.z = obj->wm.m[3][2];
			v3Dec(&objPos, from);
			tempDist = v3Length(&objPos);
			if (tempDist < dist)
			{
				dist = tempDist;
				closestObj = obj;
			}
		}
	}
	
	return closestObj;
}

static Object *lineHDObjectCollision(Point3 *from, Point3 *to, Object *obj, CollObject *co, float dist, int pn)
{
	Point3	tFrom, tTo, objPos;
	Vector3	move;
	int		i;
	Object	*collObj, *closestObj;
	float	tempDist;

	closestObj = collObj = NULL;

	for (i=0; i<obj->no_child; i++)
	{
		collObj = lineHDObjectCollision(from, to, &obj->child[i], co->child[i], dist, pn);
		if (collObj)
		{
			objPos.x = collObj->wm.m[3][0];
			objPos.y = collObj->wm.m[3][1];
			objPos.z = collObj->wm.m[3][2];
			v3Dec(&objPos, from);
			tempDist = v3Length(&objPos);
			if (tempDist < dist)
			{
				dist = tempDist;
				closestObj = collObj;
			}
		}
	}

	
	if ((obj->model) && (!(obj->collFlag & OBJECT_NOCOLLISION)) && (!(obj->collFlag & OBJECT_NOTSOLID)))
	{
		if (lineHDModelCollisionFast(from, to, co, &move, 0.0f))
		{
			objPos.x = obj->wm.m[3][0];
			objPos.y = obj->wm.m[3][1];
			objPos.z = obj->wm.m[3][2];
			v3Dec(&objPos, from);
			tempDist = v3Length(&objPos);
			if (tempDist < dist)
			{
				dist = tempDist;
				closestObj = obj;
			}
		}
	}
	
	return closestObj;
}

Object *lineStratCollision(Point3 *from, Point3 *to, STRAT *strat, int pn)
{
//	int test;
	if (pn >= 0)
	{
		if ((strat == player[pn].Player) || (strat == player[pn].CameraStrat))
			return NULL;
	}
//	if (!strat)
//		test = 1;

	if (!strat->pnode)
		return NULL;

	if(pointLineSqrDistance(&strat->pos, from, to) <  SQR(strat->pnode->radius * strat->scale.x)) 
	{
		if ((strat->flag & COLL_TARGETABLE) && (strat->obj))
			return strat->obj;
		else
		{
			if ((strat->obj) && (!(strat->flag & STRAT_HIDDEN)))
			{
				if (strat->hdBlock)
					return lineHDObjectCollision(from, to, strat->obj, strat->hdBlock->collObj, 1000000.0f, pn);
			}
			else
				return NULL;
		}
	}
	else
		return NULL;
	return NULL;
}


STRAT *targetStratCollision(Object **obj, int pn)
{
	STRAT	*strat, *closestStrat;
	float	shortestDist = 1500.0f, dist;
	Point3	ps, pe;
	Point3	targetDirection;
	Object	*objNear;
	COLLSTRAT	*coll;
	Vector3 tempv;

	ps = player[pn].CameraStrat->pos;
	v3Sub(&targetDirection, &player[pn].aimPos, &player[pn].CameraStrat->pos);
	v3Normalise(&targetDirection);
	v3Scale(&tempv, &targetDirection, 5.0f);
	v3Inc(&ps, &tempv);
	pe.x = ps.x + targetDirection.x * 500.0f;
	pe.y = ps.y + targetDirection.y * 500.0f;
	pe.z = ps.z + targetDirection.z * 500.0f;

	coll = FirstColl;

	while (coll)
	{
		strat = coll->strat;
		if ((strat == player[pn].Player) || (strat->flag2 & STRAT2_PICKUP))
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
					if ( dist < shortestDist)
					{
						closestStrat = strat;
						shortestDist = dist;
						*obj = objNear;
					}
				}
			}
		}
		coll = coll->next;
	}

	if (shortestDist == 1500.0f)
		return NULL;
	else
	{
		player[pn].aimedPoint = pe;
		return closestStrat;
	}
}


static void findCollBBox(Point3 *cp, CollBBox *cb)
{
	float		dx, dy, cx, cy;
	CollNode	*cn = Savmap->topNode;

	cx = Savmap->xMin;
	cy = Savmap->yMin;
	dx = Savmap->xMax - cx;
	dy = Savmap->yMax - cy;

	while(cn->left)
	{
		if (dx > dy)
		{
			dx *= 0.5f;
			if (cp->x > cx + dx)
			{
				cx += dx;
				cn = cn->right;
			}
			else
			{
				cn = cn->left;
			}
		}
		else
		{
			dy *= 0.5f;
			if (cp->y > cy + dy)
			{
				cy += dy;
				cn = cn->left;
			}
			else
			{
				cn = cn->right;
			}
		}
	}

	cb->leaf = (CollLeaf *)cn->right;
	cb->l = cx;
	cb->t = cy;
	cb->r = cx + dx;
	cb->b = cy + dy;
}

static void findZ(Point3 *ps, Point3 *pe, Point3 *p)
{
	float	dx, dy, lambda;
	Vector3 v;

	v3Sub(&v, pe, ps);
	dx = pe->x - ps->x;
	dy = pe->y - ps->y;

	if (fabs(dx) > fabs(dy))
		lambda = (p->x - ps->x) / dx;
	else
		lambda = (p->y - ps->y) / dy;

	dAssert((lambda <= 1.0f) && (lambda >= 0.0f), "findZ error");
	p->z = ps->z + lambda * v.z;
}

static int	IsBetween(float a, float b, float c)
{
	if (c > a)
	{
		if ((b <= c) && (b >= a))
			return 1;
		else
			return 0;
	}
	else
	{
		if ((b >= c) && (b <= a))
			return 1;
		else
			return 0;
	}
}

#define IS_VALID_POINT	\
						if (IsBetween(ps->x, x, pe->x) && IsBetween(ps->y, y, pe->y))\
						{\
							p[i].x = x;\
							p[i].y = y;\
							i++;\
						}\

void lineCollBBoxColl(Point3 *ps, Point3 *pe, CollBBox *cb, Point3 *ls, Point3 *le)
{
	float	dx, dy, y, m, x, c;
	Point3	p[3];
	int		i = 0, done_ls = 0, done_le = 0;
	
	dAssert((ps->x != pe->x) || (ps->x != pe->y), "error in lineCollBBoxColl");
	dx = pe->x - ps->x;
	dy = pe->y - ps->y;
	if ((ps->x >= cb->l) && (ps->x <= cb->r) && (ps->y >= cb->t) && (ps->y <= cb->b))
	{
		done_ls = 1;
		*ls = *ps;
	}
	if ((pe->x >= cb->l) && (pe->x <= cb->r) && (pe->y >= cb->t) && (pe->y <= cb->b))
	{
		done_le = 1;
		*le = *pe;
	}
	if (fabs(dy) > fabs(dx))
	{
		/*	using	y = mx + c   
			and		x = (y - c) / m
		*/
		m = dy / dx;
		if (fabs(m) < 0.0001f)
		{
			m = 0.0001f;
			c = ps->y;
		}
		else
			c = ps->y - m * ps->x;

		x = cb->l;
		y = m * x + c;
		if ((y >= cb->t) && (y <= cb->b))
			IS_VALID_POINT;

		x = cb->r;
		y = m * x + c;
		if ((y >= cb->t) && (y <= cb->b))
			IS_VALID_POINT;

		y = cb->t;
		x = (y - c) / m;
		if ((x >= cb->l) && (x <= cb->r))
			IS_VALID_POINT;

		y = cb->b;
		x = (y - c) / m;
		if ((x >= cb->l) && (x <= cb->r))
			IS_VALID_POINT;

		if (i==3)
		{
			i = 2;
			if (pSqrDist(&p[0], &p[1]) < 0.02f)
			{
				p[1] = p[2];
			}
		}

		if (i == 1)
		{
			
			
			if ((done_ls) && (!done_le))
			{
				findZ(ps, pe, &p[0]);
				*le = p[0];
			}
			else if ((done_le) && (!done_ls))
			{
				findZ(ps, pe, &p[0]);
				*ls = p[0];
			}
		}
		else if (i == 2)
		{
			/*if (done_ls || done_le)
				dAssert(0, "error in lineCBBC");*/

			if (fabs(ps->x - p[0].x) < fabs(ps->x - p[1].x))
			{
				if (!done_ls)
					*ls = p[0];	
				if (!done_le)
					*le = p[1];
			}
			else
			{
				if (!done_ls)
					*ls = p[1];
				if (!done_le)
					*le = p[0];
			}
			if (!done_le)
				findZ(ps, pe, le);
			if (!done_ls)
				findZ(ps, pe, ls);
		}
	}
	else
	{
		/*	using	x = my + c   
			and		y = (x - c) / m
		*/
		m = dx / dy;
		if (fabs(m) < 0.001f)
		{
			m = 0.001f;
			c = ps->x;
		}
		else
			c = ps->x - m * ps->y;

		x = cb->l;
		y = (x - c) / m;
		if ((y >= cb->t) && (y <= cb->b))
			IS_VALID_POINT;

		x = cb->r;
		y = (x - c) / m;
		if ((y >= cb->t) && (y <= cb->b))
			IS_VALID_POINT;

		y = cb->t;
		x = m * y + c;
		if ((x >= cb->l) && (x <= cb->r))
			IS_VALID_POINT;

		y = cb->b;
		x = m * y + c;
		if ((x >= cb->l) && (x <= cb->r))
			IS_VALID_POINT;


		if (i==3)
		{
			i = 2;
			if (pSqrDist(&p[0], &p[1]) < 0.02f)
			{
				p[1] = p[2];
			}
		}

		if (i == 1)
		{
			if ((done_ls) && (!done_le))
			{
				findZ(ps, pe, &p[0]);
				*le = p[0];
			}
			else if ((done_le) && (!done_ls))
			{
				findZ(ps, pe, &p[0]);
				*ls = p[0];
			}
		}
		else if (i == 2)
		{
			/*if (done_ls || done_le)
				dAssert(0, "error in lineCBBC");*/

			if (fabs(ps->y - p[0].y) < fabs(ps->y - p[1].y))
			{
				if (!done_ls)
					*ls = p[0];
				if (!done_le)
					*le = p[1];
			}
			else
			{
				if (!done_ls)
					*ls = p[1];
				if (!done_le)
					*le = p[0];
			}
			if (!done_le)
				findZ(ps, pe, le);
			if (!done_ls)
				findZ(ps, pe, ls);
		}
	}
	/*DrawMarker(ls->x, ls->y, Player->pos.z);
	DrawMarker(le->x, le->y, Player->pos.z);*/
}

static void linePlaneColl(Point3 *ls, Point3 *le, float A, float B, float C, float D, Point3 *p)
{
	float	lambda;
	Vector3	v;

	v3Sub(&v, le, ls);
	lambda = -(D + A * ls->x + B * ls->y + C * ls->z) / (A * v.x + B * v.y + C * v.z);
	p->x = ls->x + lambda * v.x;
	p->y = ls->y + lambda * v.y;
	p->z = ls->z + lambda * v.z;
}

static int pointInTriangle(Point3 *p, Point3 **v)
{
	Vector3 vect, cross[3], vp;

	v3Sub(&vp, p, v[1]);
	v3Sub(&vect, v[1], v[0]);
	v3Cross(&cross[0], &vect, &vp);
	v3Normalise(&cross[0]);
	v3Sub(&vp, p, v[2]);
	v3Sub(&vect, v[2], v[1]);
	v3Cross(&cross[1], &vect, &vp);
	v3Normalise(&cross[1]);
	v3Sub(&vp, p, v[0]);
	v3Sub(&vect, v[0], v[2]);
	v3Cross(&cross[2], &vect, &vp);
	if (v3SqrLength(&cross[2]) == 0.0f)
		return 0;

	v3Normalise(&cross[2]);

	if (v3Dot(&cross[0], &cross[1]) >= 0.0f)
		if (v3Dot(&cross[1], &cross[2]) >= 0.0f)
			return 1;

	return 0;
}

int lineCollBBoxPolyColl(Point3 *ls, Point3 *le, CollBBox *cb, Point3 *p, Vector3 *n, int *polyType)
{
	int	i, pn, vci, ct;
	float	vcA, vcB, vcC, D, lambda;
	Point3	*vert[3];
	Vector3	v, vcve_vs, *vcvs, vcvs_pp, *vcve;
	CollPoly *cp;

	/*DrawLine(ls, le, 0xff00ffff);*/
	ct = 0;
	for (i=0; i<cb->leaf->noPoly; i++)
	{
		pn = cb->leaf->polyNumber[i];
		cp = &Savmap->collPoly[pn];

		if ((cp->flag & ALLCOLLIDE) != ALLCOLLIDE)
			if (!(cp->flag & STRAT2_BULLET_CF))
				continue;
		
		vcA = cp->n.x;
		vcB = cp->n.y;
		vcC = cp->n.z;
		D = cp->D;

		if (vcA * ls->x + vcB * ls->y + vcC * ls->z + D < 0.0f)
			continue;

		if (vcA * le->x + vcB * le->y + vcC * le->z + D > 0.0f)
			continue;

		v3Sub(&v, le, ls);
		lambda = -(D + vcA * ls->x + vcB * ls->y + vcC * ls->z) / (vcA * v.x + vcB * v.y + vcC * v.z);
		p->x = ls->x + lambda * v.x;
		p->y = ls->y + lambda * v.y;
		p->z = ls->z + lambda * v.z;
		
		for (vci=0; vci<3; vci++)
		{
			vcvs = &Savmap->collLookup[cp->v[vci]]->p;
			if (vci == 2)
				vcve = &Savmap->collLookup[cp->v[0]]->p;
			else
				vcve = &Savmap->collLookup[cp->v[vci + 1]]->p;
			v3Sub(&vcve_vs, vcvs, vcve);
			v3Sub(&vcvs_pp, p, vcvs);
			INSIDE_POLY;
		}	

		if (vci < 3)
			continue;

		if (n)
			*n = cp->n;

		if (polyType)
			*polyType = cp->flag;

		*le = *p;
		ct =  1;
	}
	if (ct)
		*p = *le;
	return ct;
}

Bool lineLandscapeCollision(Point3 *ps, Point3 *pe, Point3 *p, STRAT *strat, Vector3 *n)
{
	CollBBox	cb;
	CollLeaf	*cl = NULL;
	float		dx,dy, cpl = 0.0f;
	int			col = 0, colhdc = 0;
	Vector3		v, vn, from, to;
	Point3		cp, ls, le, collP;
	HDStrat		*hds;

	from = *ps;
	to = *pe;

	hds = firstHDStrat;

	while(hds)
	{
		if (hds->strat == strat)
		{
			hds = hds->next;
			continue;
		}
		if ((hds->strat->flag & STRAT_COLLSTRAT) && (hds->valid))
		{
			if (pointLineDistanceFix(&hds->strat->pos, ps, pe) < hds->strat->pnode->radius + 10)
			{			
				collVect.x = 0.0;
				collVect.y = 0.0;
				collVect.z = 0.0;
				/****** WHAT IS THIS MATT !!!!!!!****/
				caIndex = 0;

				doHighVColl(NULL, hds->collObj, &from, &to, hds, 1);
				if ((collVect.x != 0.0) || (collVect.y != 0.0) || (collVect.z != 0.0))
					colhdc = 1;

				caIndex = 0;
				
			}
		}
		hds = hds->next;
	}

	

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
		col = lineCollBBoxPolyColl(&ls, &le, &cb, &collP, n, NULL);
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

	if (colhdc)
		return TRUE;
	else
		return FALSE;
}




void clearCollFlagRec(Object *obj)
{
	int i;

	obj->collFlag &= ~OBJECT_SHOT_BY_SIBLING;
	for (i=0; i<obj->no_child; i++)
		clearCollFlagRec(&obj->child[i]);
}

void clearCollFlag(STRAT *strat)
{
	clearCollFlagRec(strat->obj);
}

#if 0
static void calculateCPRec(Object *obj)
{
	int i;

	for (i=0; i<obj->no_child; i++)
	{
		calculateCPRec(&obj->child[i]);
	}

	for (i=0; i<obj->ncp; i++)
	{
		mPoint3Multiply3(&obj->cp[i].worldPos, &obj->cp[i].centre, &obj->wm);
	}
}

void calculateCP(STRAT *strat)
{
	calculateCPRec(strat->obj);
}
#endif


static void cpToOCPRec(Object *obj)
{
	int i;

	for (i=0; i<obj->no_child; i++)
		cpToOCPRec(&obj->child[i]);

	for (i=0; i<obj->ncp; i++)
		obj->ocpt[i] = obj->cp[i].worldPos;
}

static void cpToOCP(STRAT *strat)
{
	cpToOCPRec(strat->obj);
}


void collision(STRAT *crntStrat)
{ 
	if (!crntStrat->pnode)
		return;
#if !COLLISION
	return;
#endif
	GlobalPolyType = AIR;
	gcpn.x = gcpn.y = gcpn.z = 0.0f;
	wallN.x = wallN.y = wallN.z = 0.0f;
	if (crntStrat->flag)
	{
		//clearCollFlag(crntStrat);
		
		if (!(crntStrat->flag2 && STRAT2_MATRIX))
		{
			objectToWorld(crntStrat);
			crntStrat->flag2 |= STRAT2_MATRIX;
		}

		//TO ENSURE COLLISION WITH THE PLAYER IS CLEARED
		//SEE MATT IF THIS NEEDS TO CHANGE
	   	//if (!Multiplayer)
	   	//	crntStrat->flag2 &= ~STRAT2_PLAYERHIT;
	

		if ((crntStrat->flag & STRAT_COLLFLOOR) || (crntStrat->flag2 && STRAT2_COLLHDC))
		{
			StratCollisionFloor(crntStrat);
			setFloorCollFlag(crntStrat, crntStrat->obj);
		}	

		if (crntStrat->flag & STRAT_HITFLOOR)
		{
			crntStrat->cpn = gcpn;
			crntStrat->wallN = wallN;
			if (v3SqrLength(&crntStrat->cpn) > 0.01f)
				v3Normalise(&crntStrat->cpn);
		}
		
		if ((crntStrat->flag & STRAT_COLLSTRAT) && (crntStrat->flag & STRAT_FRIENDLY) && (!(crntStrat->hdBlock)))
			stratCollisionStrat(crntStrat);
		
		if ((crntStrat->flag & STRAT_COLLFLOOR) || (crntStrat->flag & STRAT_COLLSTRAT))
			cpToOCP(crntStrat);

	}
}

Bool lineBBocColl(CollLeaf *leaf, Vector3 *from, Vector3 *to, float radius)
{
	collide = 0;

	if (leaf->noPoly == 0)
		return FALSE;

	if (radius > 0.0f)
		vSCollide(leaf, from, to, radius, 0, 0);
	else
	{
		dAssert(to->x == to->x, "qnan");
		dAssert(to->x < 10000.0, "INF");
		vCollide(leaf, from, to, 0);
	}

	if (collide)
		return TRUE;

	return FALSE;
}

// Mark a subobject as needing collisions recorded
void RecordCollision (STRAT *strat, int subId)
{
	// if subId = 0 then mark first object
	if (subId == 0) {
		strat->obj->collFlag |= OBJECT_RECORD_COLL;
	} else {
		strat->objId[subId]->collFlag |= OBJECT_RECORD_COLL;
	}
}
