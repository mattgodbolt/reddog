#if 0
extern void DeleteMyParentParent(STRAT *strat);
void DeleteMyParentParent(STRAT *strat)
{
	
	if (strat->parent)
    {
			if (strat->parent->parent)
			{
				DeleteValidAreaEntry(strat->parent->parent);
				Delete(strat->parent->parent);
				strat->parent->parent = NULL;
			}
	}
}




// Sibling array
extern int RegisterInSiblingArray (STRAT *s);
extern STRAT *GetSibling(int i);
extern void ClearSiblingArray(void);
extern void PollSiblingArray(void);
extern int GetNumSiblings(void);
#define MAX_SIBLINGS 40
STRAT *SiblingArray[MAX_SIBLINGS];
int nSiblings = 0;

int RegisterInSiblingArray (STRAT *s)
{
	int retVal = nSiblings;
	dAssert (nSiblings < MAX_SIBLINGS, "Out of siblings");
	SiblingArray[nSiblings++] = s;
	return retVal;
}
STRAT *GetSibling(int i)
{
	dAssert (i >= 0 && i < nSiblings, "Out of range sibling");
	return SiblingArray[i];
}
void ClearSiblingArray(void)
{
	nSiblings = 0;
}
void PollSiblingArray(void)
{
	int i;
	for (i = 0; i < nSiblings; ++i)
		if (SiblingArray[i] && !(SiblingArray[i]->flag & STRAT_ALIVE))
			SiblingArray[i] = NULL;
}
int GetNumSiblings(void)
{
	return nSiblings;
}



extern  int				SeePlayerZRel(STRAT *strat,float angle);
int SeePlayerZRel(STRAT *strat,float angle)
{
	Point3 p;
	Matrix im;

	p = player[0].Player->pos;

	mInvertRot(&strat->m, &im);
	v3Dec(&p, &strat->pos);
	mPoint3Apply3(&p, &im);
	p.z = 0.0f;
	v3Normalise(&p);
	if (acos(p.y) < angle)
		return 1;
	else
		return 0;
}


extern void findObjectWorldPosition(Point3 *p, STRAT *strat, int objId);
static void findObjectWorldPosition(Point3 *p, STRAT *strat, int objId)
{
	Matrix	m;


	m = strat->m;
	mPreScale(&m, strat->scale.x, strat->scale.y, strat->scale.z);
	mPostTranslate(&m, strat->pos.x, strat->pos.y, strat->pos.z);

	if (strat->objId[objId])
		addParentMatrix(&m, strat->objId[objId]);

	p->x = m.m[3][0];
	p->y = m.m[3][1];
	p->z = m.m[3][2];
}

extern void SetStrafePos(STRAT *strat,float bound, float x, float y, float z);
/*SET A POSITION TO STRAFE ALONG */
void SetStrafePos(STRAT *strat,float bound, float x, float y, float z)
{
	Vector3 delta;

	delta.x = (player[currentPlayer].Player->pos.x - strat->pos.x);
	delta.y = (player[currentPlayer].Player->pos.y - strat->pos.y);
	delta.z = 0;
	v3Normalise(&delta);
	bound *= 5.0f;
	delta.x *= bound;
	delta.y *= bound;



	SetCheckPos(strat,x+delta.y,y-delta.x,z);
/*	if (bound<0) */
/*		dLog("bound %f\n",bound); */
}
extern	void			DamagePlayer(float	amount);
void DamagePlayer(float amount)
{
	player[currentPlayer].Player->health -= amount;
}


extern void SnapToAnimPos(STRAT *strat, int objId, float vely);

void SnapToAnimPos(STRAT *strat, int objId, float vely)
{
   	Vector3 vec2,vec;
	Matrix rotMat;
	mUnit(&rotMat);
	vely *= 3.5;
	vely = 0;
   //	mPreTranslate(&rotMat, strat->pos.x, strat->pos.y, strat->pos.z);
	//mPreMultiply(&rotMat, &strat->m);
	findTotalMatrixInterp(&rotMat, strat->objId[objId]);

  //	strat->relAcc.x = rotMat.m[3][0] - strat->pos.x;
  //	strat->relAcc.y = rotMat.m[3][1] - strat->pos.y;
  //	strat->relAcc.z = rotMat.m[3][2] - strat->pos.z;
	vec.x = rotMat.m[3][0];
	vec.y = rotMat.m[3][1] + vely;
	vec.z = rotMat.m[3][2];
   //	strat->relAcc.y = strat->objId[objId]->crntPos.y;
   //	strat->relAcc.z = strat->objId[objId]->crntPos.z;
  //	strat->relAcc.x = strat->objId[objId]->crntPos.x;
  //	strat->relAcc.y = strat->objId[objId]->crntPos.y;
  //	strat->relAcc.z = strat->objId[objId]->crntPos.z;
  	mVector3Multiply (&vec2, &vec, &strat->m);
	strat->pos.x += vec2.x;
	strat->pos.y += vec2.y;
	strat->pos.z += vec2.z;
	strat->objId[objId]->crntPos.x -= vec.x;
	strat->objId[objId]->crntPos.y -= vec.y;
	strat->objId[objId]->crntPos.z -= vec.z;
  //	strat->pos.x =  rotMat.m[3][0];
  // 	strat->pos.y =  rotMat.m[3][1];
  // 	strat->pos.z =  rotMat.m[3][2];


	//strat->relAcc.x = strat->relAcc.y = strat->relAcc.z = 0;

}
extern  int				SeePointZ(STRAT *strat,Vector3 *pos,float angle);
extern	int				OnlyInsideArea(STRAT *strat, float x, float y);
int OnlyInsideArea(STRAT *strat, float x, float y)
{
/*	short box = strat->path->area; */
	short box = strat->route->path->area;
	MAPAREA *Area;
	Vector2 Probe;
	int subarea;
	float radius,radius2;

	Probe.x = x;
	Probe.y = y;

	/*IF NO DEFINED AREA FOR THIS STRAT THEN RETURN AS BEING INSIDE ALWAYS */
	if (box <0)
		return(INSIDE);

	Area = &MapAreas[box];
	radius=Area->Surround.radius;
	radius2=radius*radius;

	/*FIRST CHECK WE ARE INSIDE THE OUTERMOST CIRCLE */
	if (!(StratDistanceToPointXYNoRoot(&Probe,&Area->Surround.pos) < radius2))
		return (OUTSIDE);
	else
	{
		#if 0
		
		//not using circles or boxes
		/*CHECK INTERSECTION WITH CIRCULAR REGIONS */
		for (subarea=0;subarea<Area->numcircleareas;subarea++)
		{
			radius = Area->circlebbox[subarea].radius;
			radius2=radius*radius;
			/*if distance less than circle's radius we are outside the valid area */
			if (StratDistanceToPointXYNoRoot(&Probe,&Area->circlebbox[subarea].pos) < radius2)
				return (OUTSIDE);
		}
		/*CHECK INTERSECTION WITH BOX REGIONS */
		for (subarea=0;subarea<Area->numboxareas;subarea++)
			if (InsideBox(&Probe,&Area->boxbbox[subarea]))
				return (OUTSIDE);
		#endif

		/*CHECK INTERSECTION WITH SPLINE REGIONS */
		for (subarea=0;subarea<Area->numsplineareas;subarea++)
			if (InsideSpline(&Probe,&Area->splinebbox[subarea]))
				return (OUTSIDE);
		return(INSIDE);
	}
}

extern	int				FindValidCheckPosXY(STRAT *strat, float near, float far);
 int FindValidCheckPosXY(STRAT *strat, float near, float far)
{
	Point3	p, tempp, pos;
	Matrix	m;
	float	ry, rang, dist, rad;
	int	i=0;

	dAssert(strat->route && strat->route->path, "This strat needs a path");
	dAssert(strat->route->path->area != -1, "You have not given a valid area");
	rad = MapAreas[strat->route->path->area].Surround.radius;
	pos.x = MapAreas[strat->route->path->area].Surround.pos.x;
	pos.y = MapAreas[strat->route->path->area].Surround.pos.y;
	pos.z = 0.0f;

	if (pDistXY(&player[currentPlayer].Player->pos, &pos) > rad)
	{
		strat->CheckPos = pos;
		return 1;
	}

	while(1)
	{
		i++;
		if(i>10)
		{
			strat->CheckPos = pos;
			return 1;
		}
		ry = RandRange(0.0, rad);
		rang = RandRange(0.0, 6.2832);
		p.x = 0.0f;
		p.y = ry;
		p.z = 0.0f;
		mRotateXYZ(&m, 0.0f, 0.0f, rang);
		mPoint3Apply3(&p, &m);
		tempp = pos;
		tempp.z = 0.0f;
		v3Inc(&p, &tempp);
		dist = pDistXY(&p, &player[currentPlayer].Player->pos);
		if ((dist > near) && (dist < far))
			if (!OnlyInsideArea(strat, p.x, p.y))
				break;
	}
	strat->CheckPos.x = p.x;
	strat->CheckPos.y = p.y;
	strat->CheckPos.z = 0.0f;
	return 1;
}

/* Sets the initial path to follow */
/*IF CALLING STRAT HAS SET GLOBAL PARAM INT 0 TO BE = 99, THE STRAT WILL NOT BE PLACED
  ON THE CHECKPOINT AND ROUTE->PATH WILL BE SET UP*/
//calling strat has set global param int 0 to be 100, the orientation shall not be set
void InitPath(STRAT *strat)
{
	ROUTE *route;

	short start;
	short next;

	/*RETURN IF NO DEFINED STRAT PATH*/
	if (!strat->route)
		return;

	route = strat->route;

	start=route->pathstartnode;
	next=start+1;



	route->path = MapPaths+route->pathnum;
	route->curway=0;

	
	if (GetGlobalParamInt(0) != 99)
	{
		if (GetGlobalParamInt(0) == 98)
		{
			route->pathstartnode = 0;
			start=route->pathstartnode;
			SetGlobalParamInt(0, 0);
		}

		if ((route->path->waytype == PATH) || (strat->flag2 & STRAT2_ENTIRENET))
		{
		
			strat->pos.x = route->path->waypointlist[start].x;  
			strat->pos.y = route->path->waypointlist[start].y;  
			strat->pos.z = route->path->waypointlist[start].z;


			//ensure we can 'see' the next node
  			if ((route->path->waytype == NETWORK))
				next = CheckNext(route->path->net,start,next);
			else
			{
				if (start == route->path->numwaypoints-1)
   					next = start-1;
			}
			strat->CheckPos.x = route->path->waypointlist[next].x;  
			strat->CheckPos.y = route->path->waypointlist[next].y;  
			strat->CheckPos.z = route->path->waypointlist[next].z;  
  			if ((route->path->waytype == NETWORK) && (strat->flag2 & STRAT2_ENTIRENET))
				route->curway=start; //was start
			else
		 		route->curway=next;
			route->lastway = route->curway;
		}
		else
		{
	   	   //	start = 0;
		   	strat->flag2 |= STRAT2_ONPATROLROUTE;
			strat->pos.x = route->path->waypointlist[route->path->net->patrol[route->patrolnumber].initialpath[start]].x;  
			strat->pos.y = route->path->waypointlist[route->path->net->patrol[route->patrolnumber].initialpath[start]].y;  
			strat->pos.z = route->path->waypointlist[route->path->net->patrol[route->patrolnumber].initialpath[start]].z;  
		 	if (start == route->path->net->patrol[route->patrolnumber].numpathpoints-1)
				next=0;

			strat->CheckPos.x = route->path->waypointlist[route->path->net->patrol[route->patrolnumber].initialpath[next]].x;  
			strat->CheckPos.y = route->path->waypointlist[route->path->net->patrol[route->patrolnumber].initialpath[next]].y;  
			strat->CheckPos.z = route->path->waypointlist[route->path->net->patrol[route->patrolnumber].initialpath[next]].z;  

			route->curway = next;
		}
   #if RESETPATH
		strat->relVel.x =strat->relVel.y = strat->relVel.z = 0;
		strat->oldVel.x =strat->oldVel.y = strat->oldVel.z = 0;
		strat->vel.x =strat->vel.y = strat->vel.z = 0;
		strat->rot_vect.x = strat->rot_vect.y = strat->rot_vect.z = 0;
		strat->rot_speed=0;	
		strat->turn.x = strat->turn.y = strat->turn.z =0.0f;
	#endif
		if (GetGlobalParamInt(0) != 100)
	{

		if (!(strat->flag2 & STRAT2_INDEPENDENTROT))
		{	
	
			if (strat->flag & STRAT_HOVER)
		   		pointToXY(strat,&strat->CheckPos);
		   	else
				pointToXZ(strat,&strat->CheckPos);
		}
	}

		strat->oldOldPos = strat->oldPos = strat->pos;
	}
	else
	{	strat->CheckPos.x = route->path->waypointlist[strat->route->curway].x;  
	 	strat->CheckPos.y = route->path->waypointlist[strat->route->curway].y;  
	 	strat->CheckPos.z = route->path->waypointlist[strat->route->curway].z;  
   //		strat->route->curway = start;
	}
}
extern	void			WorldToStrat(STRAT *strat, float x, float y, float z);
void WorldToStrat(STRAT *strat, float x, float y, float z)
{
	Matrix im;
	Point3 p;

	p.x = x;
	p.y = y;
	p.z = z;

	v3Dec(&p, &strat->pos);
	mUnit(&im);
	mInvertRot(&strat->m, &im);
	mPoint3Apply3(&p, &im);
	strat->CheckPos = p;
}

extern	float SqrDistToLastWayXY(STRAT *strat);
float SqrDistToLastWayXY(STRAT *strat)
{
	ROUTE *route = strat->route;
	Point3	p1, p2;


	p1 = route->path->waypointlist[route->path->numwaypoints - 1];
	p1.z = 0.0f;
	p2 = strat->pos;
	p2.z = 0.0f;


	return pSqrDist(&p1, &p2);
}
extern	void			SetExpRot(STRAT *strat, float amount);
void	SetExpRot(STRAT *strat, float amount)
{
	strat->pnode->expRot = amount;
}

extern void makeRotMatrix(Matrix *mat, Vector3 *v, float speed);
extern	int				EnemyInActivation(STRAT *strat, int an);
int	EnemyInActivation(STRAT *me, int an)
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

		
		if ((strat == player[0].Player) || (strat == me))
		{
			coll = coll->next;
			continue;
		}

		if ((strat->flag & STRAT_COLLSTRAT) && !(strat->flag & STRAT_BULLET) && !(strat->flag2 & STRAT2_PICKUP) && (strat != me) && (strat->flag & STRAT_ENEMY))
		{
			c = MapActs[me->actindex[an]].pos;
			p = strat->pos;
			radius = MapActs[me->actindex[an]].radius + strat->pnode->radius;
			if (pSqrDist(&p, &c) < radius * radius)
				return 1;
		}
		coll = coll->next;
	}
	return 0;
}


extern void Line (float x1, float y1, float z1, float x2, float y2, float z2, Uint32 col1, Uint32 col2);
extern Uint32 PackColour (float r, float g, float b);
Uint32 PackColour (float r, float g, float b)
{
	return 0xff000000 | (Uint32) ( (((int)(r*255.f))<<16) | (((int)(g*255.f))<<8) | (((int)(b*255.f))<<0) );
}

#if 0
void TargLookAt(Point3 *p, int pn)
{
	float	xAng, yAng, zAng, xdiff, ydiff, zdiff;
	Vector3	dir;

	dir.x = p->x - player[pn].camLookCrntPos.x;						 
	dir.y = p->y - player[pn].camLookCrntPos.y;
	dir.z = p->z - player[pn].camLookCrntPos.z;
	
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

	player[pn].targlookDir.x = (-PI + xAng);
	player[pn].targlookDir.y = zAng;
	player[pn].targlookDir.z = -PI;

	/* Update the HUD direction*/
	direction = -zAng;
}
#endif

#if 0
float CamOff=0.0;
void CamModeChange()
{
	CamOff=5.0f;
}
#endif



#if 0
void findCamLookAt(Vector3 *pos, Vector3 *point, int pn)
{
	Matrix	m2;
	
	if (!player[pn].Player->obj)
		return;

	m2 = player[pn].Player->m;
	mPoint3Multiply3(pos, point, &m2);
	v3Inc(pos, &player[pn].Player->pos);
	pos->z += player[pn].CamLookOff.z;
}
#endif

extern	void			RelTurnAwayCheckPosXY(STRAT *strat, float ang);
void RelTurnAwayCheckPosXY(STRAT *strat, float ang)
{
	Point3	p;
	Matrix	im;
	Vector3	forward, v;
	float	actualAng;

	if (PAL)
		ang *= PAL_SCALE;

	v3Sub(&v, &strat->pos, &strat->CheckPos);
	v3Inc(&v, &strat->pos);
	forward.x = 0.0f;
	forward.y = 1.0f;
	forward.z = 0.0f;
	p = v;
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

extern	void			FlattenPlayer(float amount);
void FlattenPlayer(float amount)
{
	Vector3	force, stratUp, oldVel;
	Point3	p;

	stratUp.x = player[currentPlayer].Player->m.m[2][0];
	stratUp.y = player[currentPlayer].Player->m.m[2][1];
	stratUp.z = player[currentPlayer].Player->m.m[2][2];
	v3Normalise(&stratUp);
	p.x = stratUp.x * player[currentPlayer].Player->pnode->radius * 2.0f;
	p.y = stratUp.y * player[currentPlayer].Player->pnode->radius * 2.0f;
	p.z = stratUp.z * player[currentPlayer].Player->pnode->radius * 2.0f;
	force.x = force.y = 0.0f;
	force.z = player[currentPlayer].Player->pnode->mass * 0.01f * amount;
	oldVel = player[currentPlayer].Player->vel;
	ApplyForce(player[currentPlayer].Player, &force, &p, 0.1f);
	player[currentPlayer].Player->vel = oldVel;
}

extern  void			TowardWayZ (STRAT *strat, Point3 *p, float maxx);
#if 0
float PDist(float x, float y, float z, float x2, float y2, float z2)
{
	Point3	p, q;

	p.x = x;
	p.y = y;
	p.z = z;

	q.x = x2;
	q.y = y2;
	q.z = z2;

	return pDist(&p, &q);
}
#endif

void AccelerateTowardsCheckPos(STRAT *strat, float amount)
{
	Vector3 forceV;

	v3Sub(&forceV, &strat->CheckPos, &strat->pos);
	v3Normalise(&forceV);
  //	v3Scale(&forceV, &forceV, amount * PAL_SCALE);
	ApplyForce(strat, &forceV, NULL, 1.0f);
}


void morphVectorRotate(Vector3 *vMorphed, Vector3 *vFrom, Vector3 *vTo, float amount)
{
	Matrix m;
	Vector3	a, b, c, n;
	float	ang;

	mUnit(&m);
	b = *vFrom;
	c = *vTo;

	v3Cross(&n, &b, &c);
	v3Normalise(&n);
	v3Cross(&c, &n, &b);
	m.m[0][0] = c.x;
	m.m[0][1] = c.y;
	m.m[0][2] = c.z;

	m.m[1][0] = b.x;
	m.m[1][1] = b.y;
	m.m[1][2] = b.z;

	m.m[2][0] = n.x;
	m.m[2][1] = n.y;
	m.m[2][2] = n.z;

	ang = acos(v3Dot(vFrom, vTo));
	if (ang > amount)
		mPreRotateZ(&m, -amount);
	else
		mPreRotateZ(&m, -ang);

	vMorphed->x = m.m[1][0];
	vMorphed->y = m.m[1][1];
	vMorphed->z = m.m[1][2];
}

static float newDerive(STRAT *strat)
{
	Vector3 lastwp;
	Vector3 nextwp;
	Vector3 dist;
	float real;

	lastwp = strat->CheckPos;
	nextwp = strat->pos;
	dist.x = lastwp.x - nextwp.x;
	dist.y = lastwp.y - nextwp.y;
	dist.z = lastwp.z - nextwp.z;
	real =((float)sqrt(v3Dot(&dist,&dist)));	
	if (real < 1.0f)
		real =  1.0f;

	return(real);
}

extern	void			MoveToward(STRAT *strat,float x,float y,float z, float speed);
void MoveToward(STRAT *strat,float x,float y,float z, float speed)
{
	Vector3 vel;

	if (PAL)
		speed *= PAL_SCALE;

	vel.x = x - strat->pos.x;
	vel.y = y - strat->pos.y;
	vel.z = z - strat->pos.z;

	if (sqrt(vel.x*vel.x +vel.y*vel.y + vel.z*vel.z) < 1.0)
		return;

	v3Normalise(&vel);
	vel.x *= speed;
	vel.y *= speed;
	vel.z *= speed;

	strat->vel.x += vel.x;
	strat->vel.y += vel.y;
	strat->vel.z += vel.z;
}
#if 0
int	UnderWater(STRAT *strat)
{
	if (strat->flag & STRAT_UNDER_WATER)
		return 1;
	else
		return 0;
}
#endif

#if 0
void IAmWaterSlideCamLook(STRAT *strat)
{
	WaterSlideCamLookStrat = strat;
}
#endif

#if 0
void RelToAbsPosCheck(STRAT *strat, float x, float y, float z)
{
	Point3	p;

	p.x = x;
	p.y = y;
	p.z = z;
	mPoint3Apply3(&p, &strat->m);
	v3Inc(&p, &strat->pos);
	strat->CheckPos = p;
}
#endif


#if 0
/*
 * Should really be in draw.c
 */
void LineShot (STRAT *strat, int objId, float xpos, float ypos, float zpos, float r, float g, float b)
{
	Matrix mat, rotMat;
	Point3	vec, vec2;
	Colour c;

	mUnit(&rotMat);
	mUnit(&mat);
	mPreTranslate(&mat, strat->pos.x, strat->pos.y, strat->pos.z);
	mPreMultiply(&mat, &strat->m);
	mPreScale(&mat, strat->scale.x, strat->scale.y, strat->scale.z);
	findTotalMatrix(&mat, strat->objId[objId]);
	vec.x = mat.m[3][0];
	vec.y = mat.m[3][1];
	vec.z = mat.m[3][2];
	

	mPreMultiply(&rotMat, &strat->m);
	findTotalRotMatrix(&rotMat, strat->objId[objId]);

	vec.x += (rotMat.m[0][0] * xpos) + (rotMat.m[1][0] * ypos) + (rotMat.m[2][0] * zpos);
	vec.y += (rotMat.m[0][1] * xpos) + (rotMat.m[1][1] * ypos) + (rotMat.m[2][1] * zpos);
	vec.z += (rotMat.m[0][2] * xpos) + (rotMat.m[1][2] * ypos) + (rotMat.m[2][2] * zpos);

	vec2 = vec;
	vec2.x += 1000.f * rotMat.m[1][0];
	vec2.y += 1000.f * rotMat.m[1][1];
	vec2.z += 1000.f * rotMat.m[1][2];

	c.argb.a = 255;
	c.argb.r = 255 * r;
	c.argb.g = 255 * g;
	c.argb.b = 255 * b;

	rLine (&vec, &vec2, c, c);
}
#endif

#if 0
void RelFlatten(STRAT *strat, float amount)
{
	Vector3	a, f;
	Point3	p;
	
	a.x = strat->m.m[2][0];
	a.y = strat->m.m[2][1];
	a.z = strat->m.m[2][2];
	v3Scale(&f, &a, amount);
	v3Scale(&p, &a, 10.0f);
	ApplyForce(strat, &f, &p, 0.01f);
}
#endif

#if 0
void TurnTowardDirectionOfMotion(STRAT *strat, float rate)
{
	Matrix	stratIM;
	Vector3	motionV, forwardV;
	float	ang;

	motionV = strat->vel;
	forwardV.x = 0.0f;
	forwardV.y = 1.0f;
	forwardV.z = 0.0f;
	mInvertRot(&strat->m, &stratIM);
	mPoint3Apply3(&motionV, &stratIM);
	v3Normalise(&motionV);
	ang = acos(v3Dot(&forwardV, &motionV));
	if (motionV.x < 0.0f)
	{
		if (ang < rate)
			mPreRotateZ(&strat->m, ang);
		else
			mPreRotateZ(&strat->m, rate);
	}
	else
	{
		if (ang < rate)
			mPreRotateZ(&strat->m, -ang);
		else
			mPreRotateZ(&strat->m, -rate);
	}
}
#endif


#if 0
void TowardWayX(STRAT *strat, Point3 *p,float maxx)
{
	Vector3	stratAxis, stratToWay, newAxis;
	float	ang;

	stratAxis.x = 0;
	stratAxis.y = strat->m.m[1][1];
	stratAxis.z = strat->m.m[1][2];
	
	stratToWay.x = 0;
	stratToWay.y = (p->y - strat->pos.y);
	stratToWay.z = (p->z - strat->pos.z);

	v3Normalise(&stratAxis);
	v3Normalise(&stratToWay);


	ang = (float)acos(v3Dot(&stratAxis, &stratToWay));

	if (ang < 0.01f)
		return;



	if (ang > maxx)
		ang = maxx;
	
	v3Cross(&newAxis, &stratAxis, &stratToWay);

	if ((newAxis.x == 0.0f) &&
		(newAxis.y == 0.0f) &&
		(newAxis.z == 0.0f))
		return;

	if (newAxis.z > 0.0f)
		mPreRotateX(&strat->m, ang);
	else
		mPreRotateX(&strat->m, -ang);
}
#endif



#if 0
void OnCamera(STRAT *strat, float x, float y, float z)
{
	strat->pnode->typeId = 9;
}
#endif


#if 0
void TransObjectAbs(STRAT *strat, int objId, float x, float y, float z)
{
	strat->objId[objId]->crntPos.x = strat->pnode->objId[objId]->initPos.x + x;
	strat->objId[objId]->crntPos.y = strat->pnode->objId[objId]->initPos.y + y;
	strat->objId[objId]->crntPos.z = strat->pnode->objId[objId]->initPos.z + z;
}
#endif

#if 0
float WayPointX(STRAT *strat, int wp)
{
	return strat->route->path->waypointlist[wp].x;
}

float WayPointY(STRAT *strat, int wp)
{
	return strat->route->path->waypointlist[wp].y;
}
#endif

#if 0
int	IAmOnFloor(STRAT *strat)
{
	if (strat->flag & COLL_ON_FLOOR)
		return 1;
	else	
		return 0;
}
#endif
#if 0
void BossPlatTilt(STRAT *crntStrat)
{
	Vector3 BossCentre;
	Vector3 WorldUp;
	Vector3 Resultant1;
	Vector3 Resultant2;


	WorldUp.x = WorldUp.y = 0;
	WorldUp.z = 1.0;

	BossCentre.x = (BossStrat->pos.x - crntStrat->pos.x);
	BossCentre.y = (BossStrat->pos.y - crntStrat->pos.y);
	BossCentre.z = -(BossStrat->pos.z - (crntStrat->pos.z));
	BossCentre.z = -3.0f;

	
	/*ApplyAbsForce(crntStrat,BossCentre.x,BossCentre.y,BossCentre.z,
						 BossStrat->pos.x,BossStrat->pos.y,BossStrat->pos.z + 5.0,
						 0.001);

	return;
	*/
	
	
	v3Cross(&Resultant1,&BossCentre,&WorldUp);

   
  	v3Cross(&Resultant2,&Resultant1,&BossCentre);

	v3Normalise(&BossCentre);
	v3Normalise(&Resultant1);
	v3Normalise(&Resultant2);

 	crntStrat->m.m[1][0] = BossCentre.x;
 	crntStrat->m.m[1][1] = BossCentre.y;
 	crntStrat->m.m[1][2] = BossCentre.z;
	crntStrat->m.m[0][0] = Resultant1.x;
	crntStrat->m.m[0][1] = Resultant1.y;
	crntStrat->m.m[0][2] = Resultant1.z;
 	crntStrat->m.m[2][0] = Resultant2.x;
	crntStrat->m.m[2][1] = Resultant2.y;
	crntStrat->m.m[2][2] = Resultant2.z;
}
#endif
#if 0
void ObjectTowardTargetXZ(STRAT *strat, int	objId, float ang)
{

	TowardTargetPosXZ(strat,objId,ang,&strat->target->pos);

}
#endif

#if 0
void crntToIdle(Object *obj)
{
	int i;

	if (obj->no_child > 0)
		for (i=0; i<obj->no_child; i++)
			crntToIdle(&obj->child[i]);

	obj->crntPos.x += (obj->idlePos.x - obj->crntPos.x) * 0.5f; /*wobble */
	obj->crntPos.y += (obj->idlePos.y - obj->crntPos.y) * 0.5f;
	obj->crntPos.z += (obj->idlePos.z - obj->crntPos.z) * 0.5f;
	obj->crntRot.x += 0.15f*(obj->idleRot.x - obj->crntRot.x);
	obj->crntRot.y += 0.15f*(obj->idleRot.y - obj->crntRot.y);
	obj->crntRot.z += 0.15f*(obj->idleRot.z - obj->crntRot.z);
}
#endif


#if 0
float pointLineDist2D(float sx, float sy, float ex, float ey, float px, float py)
{
	Point3	s, e, p;

	s.x = sx; s.y = sy; s.z = 0.0f;
	e.x = ex; e.y = ey; e.z = 0.0f;
	p.x = px; p.y = py; p.z = 0.0f;

	return pointLineDistance(&p, &s, &e);
}
#endif

#if 0
void newDirectMove(STRAT *strat, float maxx)
{
	Vector3	stratAxis, stratToWay, newAxis;
	Point3	p1, p2;
	float	ang,amount;


	if (strat->route->curway)
		p1 = strat->route->path->waypointlist[strat->route->curway-1];
	else
		p1 = strat->route->path->waypointlist[strat->route->path->numwaypoints];

	p2 = strat->route->path->waypointlist[strat->route->curway];
	stratAxis.x = strat->m.m[1][0];
	stratAxis.y = strat->m.m[1][1];
	stratAxis.z = strat->m.m[1][2];
	v3Sub(&stratToWay, p, &strat->pos);
	v3Normalise(&stratToWay);
	ang = acos(v3Dot(&stratAxis, &stratToWay));

	if (ang < 0.001f)
		return;

 	if (ang > maxx)
   		ang = maxx;

	if (strat->route)	
		amount = ang * newDerive(strat);
		/*amount = ((strat->relVel.y) * ang * PI2)/Derive(strat);*/
	else
		amount = (ang * 0.1f);

	if (strat->target)	
	{
   		amount *= 5.0f;
		if (amount > PI2/8.0f)
			amount = PI2/8.0f;
	}	

	v3Cross(&newAxis, &stratAxis, &stratToWay);

	if ((newAxis.x != 0.0f) || (newAxis.y != 0.0f) || (newAxis.z != 0.0f))
	{
		v3Normalise(&newAxis);	
		combineRotation(strat, &newAxis, amount);
		strat->rot_speed *= 0.5f;
	}
}
#endif

#if 0
float myPow(float x, float y)
{
	if (x > 0.0f)
		return x / (0.2f - 0.2f*x + x);
	else
		return x / (0.2f + 0.2f*x - x);
}
#endif


#if 0
void DrawLine(Point3 *a, Point3 *b, Uint32 colour)
{
	Colour	col;

	col.argb.a = (colour&0xff000000)>>24;
	col.argb.b = (colour&0x00ff0000)>>16;
	col.argb.g = (colour&0x0000ff00)>>8;
	col.argb.r = (colour&0x000000ff);
	mPush(&mCurMatrix);
	mUnit(&mCurMatrix);
	rLineOnTop(a, b, col, col);
	mPop(&mCurMatrix);
}


void IAmWaterSlide(STRAT *strat)
{
	WaterSlide = strat;
}
#endif


#if 0
Uint32 GetParentStratId(STRAT *strat)
{
	if (strat->parent)
		return strat->parent->id;
	else
		return 0;
}

Uint32 GetStratId(STRAT *strat)
{
	if (strat)
		return strat->id;
	else
		return 0;
}

Uint32 GetPlayerId(STRAT *strat, int pn)
{
	if (player[pn].Player)
		return player[pn].Player->id;
	else
		return 0;
}
#endif

#if 0
void IAmEscortTanker(STRAT *strat)
{
	EscortTanker = strat;
}
#endif

#if 0
	void IAmBoatStrat(STRAT *strat)
	{
		BoatStrat = strat;
	}

	void IAmBossGuide(STRAT *strat)
	{
		BossGuide = strat;
	}
#endif


#if 0
void DrawBeam(STRAT *strat)
{

	DrawLine(&strat->pos, &player[0].aimedPoint, 0xff00ffff);
}
#endif


#if 0
	 	 
int CrntRotToIdleRotX(STRAT *strat, int objId, float ang)
{
	float *c, *i;
	int n;
	c = &strat->objId[objId]->crntRot.x;
	i = &strat->objId[objId]->idleRot.x;

	if (*c == *i)
		return 1;
	else if ((*c < *i))
	{
		if (fabs((*i) - (*c)) > ang)
			*c += ang;
		else
		{
			*c = *i;
			return 1;
		}
	}
	else
	{
		if (fabs((*c) - (*i)) > ang)
			*c -= ang;
		else
		{
			*c = *i;
			return 1;
		}
	}
	return 0;
}
#endif
#if 0

void stratAccForward(STRAT *crntStrat, float	amount)
{
	Vector3	force, cp;
	
	force.x = amount * crntStrat->pnode->mass * crntStrat->m.m[1][0];
	force.y = amount * crntStrat->pnode->mass * crntStrat->m.m[1][1];
	force.z = amount * crntStrat->pnode->mass * crntStrat->m.m[1][2];
	cp.x = cp.y = cp.z = 0.0f;

	if (crntStrat->flag & COLL_ON_FLOOR)
		ApplyForce(crntStrat, &force, &cp, 1.0f);
}



void setOldP(void)
{
	STRAT *strat;

	strat = FirstStrat;
	while(strat)
	{	
		
		if (strat->flag & STRAT_CALCRELVEL)
			calcRelVel(strat);	
		strat->flag &= ~STRAT2_MATRIX;
		if (strat != player[currentPlayer].Player) 
			strat->oldPos = strat->pos;
		else if (!PlayerHeld)
			strat->oldPos = strat->pos;

		strat=strat->next;

	}  
}
#endif
/*
int	findHoldPlayerNumber(STRAT *strat)
{
	int i;

	for (i=0; i<MAX_HOLD_PLAYER; i++)
		if (strat == HoldPlayerStrat[i])
			return i;

	dAssert(0, "oops");
}
*/
/*
int ClosedPath(STRAT *strat)
{
	ROUTE *route = strat->route;

	if (pDist(&route->path->waypointlist[0], &route->path->waypointlist[route->path->numwaypoints - 1]) < 0.1f)
		return 1;

	return 0;
}
*/

/*
void TestDraw(STRAT *strat)
{
	Vector3 targetDir,camDir,cross;
	
	camDir.x = player[0].CameraStrat->m.m[1][0];
	camDir.y = player[0].CameraStrat->m.m[1][1];
	camDir.z = 0.0f;
	v3Normalise(&camDir);
	v3Sub(&targetDir, &strat->pos, &player[0].CameraStrat->pos);
	targetDir.z = 0.0f;
	v3Normalise(&targetDir);
	strat->damage = acos(v3Dot(&targetDir, &camDir));
	v3Cross(&cross, &camDir, &targetDir);
	if (cross.z < 0.0f)
		strat->damage *= -1.0f;

	DrawArrowToTarget(strat,strat->damage);
}
*/

#if 0
void FlattenToVector(STRAT *strat, float x, float y, float z , float amount)
{
	Vector3	force;
	Point3	p;
	Vector3	oldv;

	oldv = strat->vel;
	force.x = x;
	force.y = y;
	force.z = z;
	v3Scale(&force, &force, 10.0f);
	p.x = strat->m.m[2][0] * 10.0f;
	p.y = strat->m.m[2][1] * 10.0f;
	p.z = strat->m.m[2][2] * 10.0f;
	ApplyForce(strat, &force, &p, amount);
	strat->vel = oldv;
}
#endif

/*MATT CHANGE
void initObj(Object *obj)
{
	int i;

	if (!obj)
		return ;

	obj->initRot = obj->idleRot;
	obj->initPos = obj->idlePos;

	for (i=0; i<obj->no_child; i++)
		initObj(&obj->child[i]);
}
*/
/*
void resetCrnt(Object *obj)
{
	int i;

	if (obj->no_child > 0)
		for (i=0; i<obj->no_child; i++)
			resetCrnt(&obj->child[i]);

 //MATT CHANGE
 //	obj->crntRot = obj->initRot = obj->idleRot;
 //	obj->crntPos = obj->initPos = obj->idlePos;
	obj->crntRot = obj->idleRot;
	obj->crntPos = obj->idlePos;


}
*/
#if 0
void pointObjectTo(STRAT *crntStrat, int objId, Point3 *p)
{
	Vector3	fromTo, stratV, rotVect;
	Matrix	im;
	float	angle;
	
	if (!crntStrat->pnode) 
		return;

	fromTo.x = p->x - crntStrat->pos.x;
	fromTo.y = p->y - crntStrat->pos.y;
	fromTo.z = p->z - crntStrat->pos.z;
	stratV.x = 0.0f;
	stratV.y = 1.0f;
	stratV.z = 0.0f;
	v3Normalise(&fromTo);
	angle = (float)acos(v3Dot(&fromTo, &stratV));
	/*mUnit(&crntStrat->m); */
	v3Cross(&rotVect, &stratV, &fromTo);
	v3Normalise(&rotVect);
	 
	mUnit(&im);
	mInvertRot(&crntStrat->m, &im);
	/*mInvert3d(&im, &crntStrat->m);*/
	makeRotMatrixBr(&crntStrat->objId[objId]->m, &rotVect, angle);
	crntStrat->objId[objId]->m.m[3][0] = 0.0f;
	crntStrat->objId[objId]->m.m[3][1] = 0.0f;
	crntStrat->objId[objId]->m.m[3][2] = 5.0f;

	mPostMultiply(&crntStrat->objId[objId]->m, &im);
}
#endif

#if 0
void stratMakeRotMatrix(STRAT *crntStrat)
{
	float	s, c, t, sx, sy, sz, txy, tyz, txz, speed;
	Vector3	*v;
	Matrix *mat;


	mat = &crntStrat->m;
	v = &crntStrat->rot_vect;

/*	speed *= -crntStrat->rot_speed; */
	speed = -crntStrat->rot_speed;

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
#endif


#if 0
void pointTo(STRAT *crntStrat, Point3 *p)
{
	Vector3	fromTo, stratV, rotVect;
	float	angle;
	
/*	dLog("%f, %f, %f\n, %f, %f, %f\n", crntStrat->pos.x, crntStrat->pos.y, crntStrat->pos.z,  */
/*										p->x, p->y, p->z); */
	
	fromTo.x = p->x - crntStrat->pos.x;
	fromTo.y = p->y - crntStrat->pos.y;
	fromTo.z = p->z - crntStrat->pos.z;
	stratV.x = 0.0f;
	stratV.y = 1.0f;
	stratV.z = 0.0f;
	v3Normalise(&fromTo);
	angle = (float)acos(v3Dot(&fromTo, &stratV));
	/*mUnit(&crntStrat->m); */
	v3Cross(&rotVect, &stratV, &fromTo);
	if ((rotVect.x != 0.0f) || (rotVect.y != 0.0f) || (rotVect.z != 0.0f))
		v3Normalise(&rotVect);
	else
	{
		rotVect.x = 0.0f;
		rotVect.y = 0.0f;
		rotVect.z = 1.0f;
	}
	
	makeRotMatrixBr(&crntStrat->m, &rotVect, angle);
	/*dLog("pos1: %f, %f, %f\npos2 : %f, %f, %f\nangle: %f\n", crntStrat->pos.x, crntStrat->pos.y, crntStrat->pos.z, p->x, p->y,p->z, angle); */
}
#endif

#if 0
void ApplyForce(STRAT *crntStrat, Vector3 *v, Point3 *p, float ratio)
{
	float nn, length, radius, mass;
	Point3	np, tempP;
	Vector3	new_rot_vect;

	
/*	if (v3Length(p) > 20.0f) */
/*		dLog("bollox\n"); */
	if (!p)
	{
		p = &tempP;
		p->x = p->y = p->z = 0.0f;
	}
	/*v3Sub(&transP, p, &crntStrat->pnode->com); */
	/*p = &transP; */

	radius = (crntStrat->pnode->radius * crntStrat->scale.x);
	mass = crntStrat->pnode->mass;
	v3Scale(&np, v, 1.0f / mass);
	v3Inc(&crntStrat->vel, &np);

	if (((v->x != 0.0f) || (v->y != 0.0f) || (v->z != 0.0f)) && 
		((p->x != 0.0f) || (p->y != 0.0f) || (p->z != 0.0f)))
	{		
		v3Scale(&np, v, -v3Dot(v, p) / v3Dot(v, v));
		v3Inc(&np, p);
		nn = -v3Dot(&np, &np) * v3Length(v) * 0.01f;
		v3Cross(&new_rot_vect, v, &np);
		length = v3Length(&new_rot_vect);
		if (length > 0.01f)
		{
			v3Scale(&np, &crntStrat->rotRest, 1.0f / length);
			v3Mul(&new_rot_vect, &new_rot_vect, &np);
			combineRotation( crntStrat, &new_rot_vect, nn / (crntStrat->pnode->mass * radius * radius * ratio * 0.3f));
		}
	}
}
#endif

#if 0
void makeRotMatrixBr(Matrix *mat, Vector3 *v, float ang)
{
	float	w, sinAng, x, y, z, xs, ys, zs, xx, xy, xz, yy, yz, zz, wx, wy, wz;

	w = (float)cos(ang*0.5f);
	sinAng = (float)sin(ang*0.5f);

	x = v->x * sinAng;
	y = v->y * sinAng;
	z = v->z * sinAng;

	xs = 2.0f * x;
	ys = 2.0f * y;
	zs = 2.0f * z;
	wx = w * xs;
	wy = w * ys;
	wz = w * zs;
	xx = x * xs;
	xy = x * ys;
	xz = x * zs;
	yy = y * ys;
	yz = y * zs;
	zz = z * zs;

	mat->m[0][0] = 1.0f-(yy+zz);	mat->m[1][0] = xy - wz;				mat->m[2][0] = xz + wy;
	mat->m[0][1] = xy + wz;			mat->m[1][1] = 1.0f - (xx + zz);	mat->m[2][1] = yz - wx;
	mat->m[0][2] = xz - wy;			mat->m[1][2] = yz + wx;				mat->m[2][2] = 1.0f - (xx + yy);
}
#endif


#if 0
void ObjectTowardTargetX(STRAT *strat, int	objId, float ang)
{
	Matrix	rotMat;
	Vector3	objectForward, objectToTarget;
	Point3	objPos;
	float	rotAng, dist, temp;

	rotMat = strat->m;
	findTotalRotMatrix(&rotMat, strat->objId[objId]);

	objectForward.x = rotMat.m[1][0];
	objectForward.y = rotMat.m[1][1];
	objectForward.z = rotMat.m[1][2];

	mUnit(&rotMat);
	mPreTranslate(&rotMat, strat->pos.x, strat->pos.y, strat->pos.z);
	mPreMultiply(&rotMat, &strat->m);
	mPreScale(&rotMat, strat->scale.x, strat->scale.y, strat->scale.z);
	findTotalMatrix(&rotMat, strat->objId[objId]);

	objPos.x = rotMat.m[3][0];
	objPos.y = rotMat.m[3][1];
	objPos.z = rotMat.m[3][2];
	objectToTarget.x = strat->target->pos.x - objPos.x;
	objectToTarget.y = strat->target->pos.y - objPos.y;
	objectToTarget.z = strat->target->pos.z - objPos.z;
	dist = (float)sqrt(v3Dot(&objectToTarget, &objectToTarget));
	objectToTarget.x = objectForward.x * dist;
	objectToTarget.y = objectForward.y * dist;
	temp			 = (objectForward.z * dist + objPos.z);

	v3Normalise(&objectToTarget);
	v3Normalise(&objectForward);
	rotAng = (float)acos(v3Dot(&objectForward, &objectToTarget));
	if (rotAng > ang)
		rotAng = ang;
	else if(rotAng < 0.0001f)
		return;

	if (temp > strat->target->pos.z)
		strat->objId[objId]->idleRot.x -= rotAng;
	else
		strat->objId[objId]->idleRot.x += rotAng;
}

void ObjectTowardTargetZ(STRAT *strat, int	objId, float ang)
{
	Point3	p;
	Matrix	im;
	Vector3	forward;
	float	rotAng;
	Matrix	rotMat;
   Vector3	cross,objectLateral,objectForward, objectToTarget;
	Point3	objPos;

	rotMat = strat->m;
	findTotalRotMatrix(&rotMat, strat->objId[objId]);

	objectForward.x = rotMat.m[1][0];
	objectForward.y = rotMat.m[1][1];
	objectForward.z = rotMat.m[1][2];


	mUnit(&rotMat);
	mPreTranslate(&rotMat, strat->pos.x, strat->pos.y, strat->pos.z);
	mPreMultiply(&rotMat, &strat->m);
	mPreScale(&rotMat, strat->scale.x, strat->scale.y, strat->scale.z);
	findTotalMatrix(&rotMat, strat->objId[objId]);

	objPos.x = rotMat.m[3][0];
	objPos.y = rotMat.m[3][1];
	objPos.z = rotMat.m[3][2];
	objectToTarget.x = strat->target->pos.x - objPos.x;
	objectToTarget.y = strat->target->pos.y - objPos.y;
	objectToTarget.z = strat->target->pos.z - objPos.z;
	//dist = (float)sqrt(v3Dot(&objectToTarget, &objectToTarget));
	//objectToTarget.x = objectForward.x * dist;
	//objectToTarget.y = objectForward.y * dist;
	//temp			 = (objectForward.z * dist + objPos.z);

	objPos.z = objectToTarget.z = objectForward.z = 0;

	v3Normalise(&objectToTarget);
	v3Normalise(&objectForward);
	v3Normalise(&objectLateral);


	
	
	rotAng = (float)acos(v3Dot(&objectForward, &objectToTarget));
	if (rotAng > ang)
		rotAng = ang;
	else if(rotAng < 0.0001f)
		return;

	v3Cross(&cross,&objectForward,&objectToTarget);
 
	if (cross.z < 0)
		strat->objId[objId]->idleRot.z -= rotAng;
	else
		strat->objId[objId]->idleRot.z += rotAng;

}
#endif


extern float GetScorpXOffset(float frame);
float GetScorpXOffset(float frame)
{
	return(SCORPXMOVES[(int) frame]);

}


extern void InitLightning(void);
extern	int				SeeParentZ(STRAT *strat,float angle);
extern  int				NearParent(STRAT *strat,float dist);
int SeeParentZ(STRAT *strat,float angle)
{	
	Vector3 Forward, stratToWay;
	
	Forward.x=strat->m.m[1][0];
	Forward.y=strat->m.m[1][1];
	Forward.z=0.0f;
	v3Normalise(&Forward);

	stratToWay.x = strat->parent->pos.x - strat->pos.x;
	stratToWay.y = strat->parent->pos.y - strat->pos.y;
	stratToWay.z = 0.0f;
	v3Normalise(&stratToWay);

	angle = (float) cos(angle);

	if ((v3Dot(&Forward, &stratToWay)) > angle)
		return 1;
	else
		return 0;
}




//MAKES A STRAT A TARGET WAREZY THING
//void AddMeToTarget(STRAT *strat)
//{
	//addToTargetArray(strat, NULL, currentPlayer);
//	
//}

extern	int				InPlayerZCone(STRAT *strat, float amount);
int	InPlayerZCone(STRAT *strat, float amount)
{
	Vector3 playerToStrat;

	v3Sub(&playerToStrat, &strat->pos, &player[0].Player->pos);
	v3Normalise(&playerToStrat);
	if (acos(playerToStrat.z) < amount)
		return 1;
	else
		return 0;
}

void IAmRedDogHud(STRAT *strat)
{
	strat->parent->Player->RedDogHudStrat = strat;
}

extern	void			IAmRedDogHud(STRAT *strat);
void IAmPlayerShield(STRAT *strat)
{
	strat->parent->Player->shieldStrat = strat;
}
void ObjectNoFade(STRAT *strat, int objId)
{
	ObjectNoFadeRec(strat->objId[objId]);
}

extern	void			ObjectNoFade(STRAT *strat, int objId);
extern	void			DrawTarget(Point3 *p, float time);
extern	void			CalcObjMat(STRAT *strat);
void CalcObjMat(STRAT *strat)
{
	if (strat)
	{
		objectToWorld(strat);
		mShortInvert(&strat->obj->im, &strat->obj->wm);
	}
}



int MyParentParentInvalid(STRAT *strat)
{
	unsigned int	id;

	//NO PARENT ATTACHED THEN RETURN VALID
	if (!strat->parent->parent)
		return(0);

	id = (int)strat->parent->parent;

	if (id < 1024)
		return (1);
	else
		return(0);

}


void IAmDaBoss(STRAT *strat)
{
	BossStrat = strat;

}

//extern void IAmDaBoss(STRAT *strat);
//extern	void			RemoveFromAvoidStratArray(STRAT *strat);
//extern	void			FreeASBlock(STRAT *strat);
//void RemoveFromAvoidStratArray(STRAT *strat)
//{
//	FreeASBlock(strat);
//}
//extern  void	CamSetPos(STRAT *strat);
//extern	int		HoldPlayerHitFloor(void);


// Strat glue code for bands systems
//#include "Render\Bands.h"
// Strat glue code for sparks systems
//#include "Render\Sparks.h"


void RollTowardPlayer(STRAT *strat, float ang)
{ 
	Point3	p;
	Matrix	im;
	Vector3	forward,up;
	float	actualAng;

	forward.x = 0.0f;
	forward.y = 0.0f;
	forward.z = 1.0f;
	p = player[currentPlayer].Player->pos;
 //	v3Dec(&p, &strat->pos);
 //	mInvertRot(&strat->m, &im);
 //	mPoint3Apply3(&p, &im);
 //	p.y = 0.0f;
 //	v3Normalise(&p);
	actualAng = PI2 * (p.x - strat->pos.x)/30.0;

	if (actualAng > PI2/9.0)
		actualAng = PI2/360.0;
	if (actualAng < -PI2/9.0)
		actualAng = -PI2/360.0;

  //	if (strat->pos.x < p.x)
  //		actualAng = 0.05;
  //	else
  //		actualAng = -0.05;




 	up.x = strat->m.m[2][0];
	up.y = strat->m.m[2][1];
	up.z = strat->m.m[2][2];
	ang = (float) acos(v3Dot(&up, &forward));

	if (fabs(ang) < ((PI2/360.0f) * 35.0f))
		mPreRotateY(&strat->m, actualAng);


  /*	if ((p.x < 0.0f) && (ang < actualAng))
	else if ((p.x > 0.0f) && (ang < actualAng))
		mPreRotateY(&strat->m, -ang);
	else if((p.x < 0.0f) && (ang >= actualAng))
		mPreRotateY(&strat->m, actualAng);
	else if ((p.x > 0.0f) && (ang <= actualAng))
		mPreRotateY(&strat->m, -actualAng);	 */


}
//extern void RollTowardPlayer(STRAT *strat, float ang);

/*PUT INTO PNODE LOADER */
//void initObj(Object *obj);

//extern STRAT	RedDog;
//extern STRAT	RedDog2;
//extern	Uint32	globalStratID;


//extern	Uint32			GetParentStratId(STRAT *strat);
//extern	Uint32			GetStratId(STRAT *strat);
//extern	Uint32			GetPlayerId(STRAT *strat, int pn);

//extern	void			IAmEscortTanker(STRAT *strat);
//extern	void			IAmWaterSlideCamLookStrat(STRAT *strat);
//extern	void			IAmWaterSlide(STRAT *strat);
//extern	void			IAmBoatStrat(STRAT *strat);
//extern	void			IAmBossGuide(STRAT *strat);
void FindAimPoint(STRAT *strat, float bulletSpeed, int pn)
{
	Point3	firePoint, playerPos, p;
	Vector3 redDogVelocity, line, tempv;
	float	lambda = 50.0f, playerDist, enemyDist, pt, et, playerSpeed, delta = 25.0f;

	firePoint = strat->CheckPos;
	v3Sub(&redDogVelocity, &player[pn].Player->pos, &player[pn].Player->oldPos);
	playerPos = player[pn].Player->pos;

	if (v3SqrLength(&redDogVelocity) < 0.01f)
	{
		strat->CheckPos = player[pn].Player->pos;
		return;
	}

	line = redDogVelocity;
	v3Normalise(&line);
	playerSpeed = v3Length(&redDogVelocity);

	while((fabs(et - pt) < 0.03f) || (delta > 0.1f))
	{
		v3Scale(&tempv, &line, lambda);
		v3Add(&p, &playerPos, &tempv);
		playerDist = pSqrDist(&p, &playerPos);
		enemyDist = pSqrDist(&p, &firePoint);
		pt = playerDist / playerSpeed;
		et = enemyDist / bulletSpeed;

		if (et > pt)
			lambda += delta;
		else
			lambda -= delta;

		delta *= 0.5f;
	}

	strat->CheckPos = p;
}

extern	void			FindAimPoint(STRAT *strat, float bulletSpeed, int pn);
#if 0
/*SPAWN FROM A POSITION WITH A LOOK AT VECTOR TO GO TO */

void MissileSpawn(STRAT *parent, int StratEntry, float xpos, float ypos, float zpos, float xoff, float yoff, float zoff, int model)
{
	STRAT *strat;
	Matrix mat, rotMat;
	Point3 pos,vec2;
/*	Vector source,dest; */


	strat = AddStrat(StratEntry);
	if (!(strat))
		return;
		
	pos.x = 0.0f;
	pos.y = 0.0f;
	pos.z = 0.0f;

	strat->m = parent->m;
	/*mat = parent->m; */
	mUnit(&mat);
	mUnit(&rotMat);
	/*IF SPAWN ANIM */
	if ((model) && (parent->objId))	
	{
		mPreTranslate(&mat, parent->pos.x, parent->pos.y, parent->pos.z);
		mPreMultiply(&mat, &parent->m);
		mPreScale(&mat, parent->scale.x, parent->scale.y, parent->scale.z);
		findTotalMatrix(&mat, parent->objId[model]);

		vec2.x = mat.m[3][0];
		vec2.y = mat.m[3][1];
		vec2.z = mat.m[3][2];

		mPreMultiply(&rotMat, &parent->m);
		findTotalRotMatrix(&rotMat, parent->objId[model]);

		strat->m=rotMat;

		vec2.x += (rotMat.m[0][0] * xpos) + (rotMat.m[1][0] * ypos) + (rotMat.m[2][0] * zpos);
		vec2.y += (rotMat.m[0][1] * xpos) + (rotMat.m[1][1] * ypos) + (rotMat.m[2][1] * zpos);
		vec2.z += (rotMat.m[0][2] * xpos) + (rotMat.m[1][2] * ypos) + (rotMat.m[2][2] * zpos);

		strat->pos = vec2;
	}
	else
	{
		strat->m = parent->m;
		strat->pos = parent->pos;
		strat->pos.x += (strat->m.m[0][0] * xpos) + (strat->m.m[1][0] * ypos) + (strat->m.m[2][0] * zpos);
		strat->pos.y += (strat->m.m[0][1] * xpos) + (strat->m.m[1][1] * ypos) + (strat->m.m[2][1] * zpos);
		strat->pos.z += (strat->m.m[0][2] * xpos) + (strat->m.m[1][2] * ypos) + (strat->m.m[2][2] * zpos);
	}
	
/*	mPreRotateXYZ(&strat->m, xang, yang, zang); */

	strat->parent = parent;
	strat->oldPos = strat->pos;

	strat->CheckPos.x = player[currentPlayer].Player->pos.x;
	strat->CheckPos.y = player[currentPlayer].Player->pos.y;
	strat->CheckPos.z = player[currentPlayer].Player->pos.z;


	if (player[currentPlayer].Player->relAcc.y > 1.0f) 
	{
		yoff *= (player[currentPlayer].Player->vel.y );
		xoff *= (player[currentPlayer].Player->vel.y );
	}

	strat->CheckPos.x += (player[currentPlayer].Player->m.m[0][0] * xoff) + 
						 (player[currentPlayer].Player->m.m[1][0] * yoff) + 
						 (player[currentPlayer].Player->m.m[2][0] * zoff);
	strat->CheckPos.y += (player[currentPlayer].Player->m.m[0][1] * xoff) + 
						 (player[currentPlayer].Player->m.m[1][1] * yoff) + 
						 (player[currentPlayer].Player->m.m[2][1] * zoff);
	strat->CheckPos.z += (player[currentPlayer].Player->m.m[0][2] * xoff) + 
						 (player[currentPlayer].Player->m.m[1][2] * yoff) + 
						 (player[currentPlayer].Player->m.m[2][2] * zoff);

	


	pointToXZ(strat,&strat->CheckPos);
/*	singularMatrix(&strat->m); */
	strat->func(strat);

}
#endif

#if 0
	//NOT CURRENTLY USED
	//extern void			MissileSpawn(STRAT *parent, int StratEntry, float xpos, float ypos, float zpos, float xang, float yang, float zang, int model);

	//extern	void			SetObjectIdleRotX(STRAT *strat, int objId, float amount);
	//extern	void			SetObjectIdleRotY(STRAT *strat, int objId, float amount);
	//extern	void			SetObjectIdleRotZ(STRAT *strat, int objId, float amount);
	//extern	void			SetObjectIdlePosX(STRAT *strat, int objId, float amount);
	//extern	void			SetObjectIdlePosY(STRAT *strat, int objId, float amount);
	//extern	void			SetObjectIdlePosZ(STRAT *strat, int objId, float amount);

	//extern	void			SetObjectCrntRotX(STRAT *strat, int objId, float amount);
	//extern	void			SetObjectCrntRotY(STRAT *strat, int objId, float amount);
	//extern	void			SetObjectCrntRotZ(STRAT *strat, int objId, float amount);
	//extern	void			SetObjectCrntPosX(STRAT *strat, int objId, float amount);
	//extern	void			SetObjectCrntPosY(STRAT *strat, int objId, float amount);
	//extern	void			SetObjectCrntPosZ(STRAT *strat, int objId, float amount);

	//extern	float			GetObjectIdlePosX(STRAT *strat, int objId);
	//extern	float			GetObjectIdlePosY(STRAT *strat, int objId);
	//extern	float			GetObjectIdlePosZ(STRAT *strat, int objId);
	//extern	float			GetObjectIdleRotX(STRAT *strat, int objId);
	//extern	float			GetObjectIdleRotY(STRAT *strat, int objId);
	//extern	float			GetObjectIdleRotZ(STRAT *strat, int objId);
	//extern	float			GetObjectCrntPosX(STRAT *strat, int objId);
	//extern	float			GetObjectCrntPosY(STRAT *strat, int objId);
	//extern	float			GetObjectCrntPosZ(STRAT *strat, int objId);
	//extern	float			GetObjectCrntRotX(STRAT *strat, int objId);
	//extern	float			GetObjectCrntRotY(STRAT *strat, int objId);
	//extern	float			GetObjectCrntRotZ(STRAT *strat, int objId);
  //WHAT ARE THESE FOR
	//extern	Point3	GlobalActivationPos;
	//extern	float	GlobalActivationRadius;
	//extern Point3	rRegionCentre;
	//extern float	rRegionSqrRadius;
	//extern	void			RelFlatten(STRAT *strat, float amount);
	//extern	int				GetSecondaryTarget(STRAT *strat, int pn);
	//extern	void			GetTargetPos(STRAT *strat, int pn, int i);
	//extern	void LineShot (STRAT *strat, int ObjID, float x, float y, float z, float r, float g, float b);
	//extern	int				UnderWater(STRAT *strat);
	//extern	void			OnCamera(STRAT *strat, float x, float y, float z);
	//extern	void			SetOnCamera(STRAT *strat, float x, float y, float z);
	//extern	void			IAmBBox(STRAT *strat);
	//extern	void			LoopWaterSlide(STRAT *strat, float amount);
	//extern	float			WayPointY(STRAT *strat, int wp);
	//extern	float			WayPointX(STRAT *strat, int wp);
	//extern	int				IAmOnFloor(STRAT *strat);
	//extern	void	DrawBeam(STRAT *strat);
	//extern  void			BossPlatTilt();
	//extern	float 	CheckDistXY(STRAT *strat);
	//extern	void  	TransObjectAbs(STRAT *strat, int objId, float x, float y, float z);
	//extern	int	 	LineOfSight(float ax, float ay, float az, float bx, float by, float bz, float dist);
	//extern	int		LineSightParent(STRAT *strat);
	//extern	int		FireSightPlayer(STRAT *strat);
	//extern	int		NextReverseWayPoint(STRAT *strat);
	//extern	int		PlayerObjectHitFloor(int objId);
	//extern	int		TargetNearPlayer(STRAT *strat,float amount);
	//extern	int		GetTargetProxim(STRAT *parent,float dist);
	//extern	int	   	PlayerInRadius(STRAT *strat);
	//extern	int	 	StratXCheckPos(STRAT *strat);
	//extern	float 	PDist(float x, float y, float z, float x2, float y2, float z2);
	//extern	void  	FlattenToVector(STRAT *strat, float x, float y, float z , float amount);
	//extern	void 	ObjectTowardPlayerOffsetXZ(STRAT *strat, int	objId, float angx, float angz, float angXOffset, float angZOffset);
	//extern	void  	ApplyForcePlayer(float x, float y, float z);
	//extern	void  	ApplyForcePlayerRel(float x, float y, float z);
	//extern	void   	ResetOCP(STRAT *strat);
	//extern	void	PushRedDogAlongPath(STRAT *strat, float amount);
	//extern	void	RelTurnTowardZ(STRAT *strat, float x, float y, float z, float ang);
	//extern	void   	IAmFloorStrat(STRAT *strat);
	//extern	void   	ApplyUpForce(STRAT *strat, float amount);
	//extern	void 	ApplyRotToPlayer(STRAT *strat);
	//extern	void	UnRegisterAsTarget(STRAT *strat);
	//extern	void 	RelToAbsPosCheck(STRAT *strat, float x, float y, float z);
	//extern	void 	SetRotFlag(STRAT *strat);
	//extern	void   	InitRotInit(STRAT *strat);
	//extern	void	PauseAllStrats(STRAT *strat,int mode);
	//extern	void	UnPauseAllStrats(STRAT *strat, int mode);
	//extern	void 	SetPlayerFriction(float x, float y, float z);
	//extern	void 	RelTurnTowardXY(STRAT *strat, float x, float y, float z, float ang);
	//extern	void  	Roll  (STRAT *strat, float amount);
	//extern	void  	Pitch (STRAT *strat, float amount);
	//extern	void 	ScaleX (STRAT *strat, float amount);
	//extern	void 	ScaleY (STRAT *strat, float amount);
	//extern	void 	ScaleZ (STRAT *strat, float amount);

#endif

#if 0
void SetObjectIdleRotX(STRAT *strat, int objId, float amount)
{
	strat->objId[objId]->collFlag |= OBJECT_STRATMOVE;
	strat->objId[objId]->idleRot.x = amount;
}

void SetObjectIdleRotY(STRAT *strat, int objId, float amount)
{
	strat->objId[objId]->collFlag |= OBJECT_STRATMOVE;
	strat->objId[objId]->idleRot.y = amount;
}

void SetObjectIdleRotZ(STRAT *strat, int objId, float amount)
{
	strat->objId[objId]->collFlag |= OBJECT_STRATMOVE;
	strat->objId[objId]->idleRot.z = amount;
}
void SetObjectIdlePosX(STRAT *strat, int objId, float amount)
{
	strat->objId[objId]->idlePos.x = amount;
}

void SetObjectIdlePosY(STRAT *strat, int objId, float amount)
{
	strat->objId[objId]->idlePos.y = amount;
}

void SetObjectIdlePosZ(STRAT *strat, int objId, float amount)
{
	strat->objId[objId]->idlePos.z = amount;
}
#endif

#if 0
float GetObjectCrntRotX(STRAT *strat, int objId)
{

	return strat->objId[objId]->crntRot.x;
}

float GetObjectCrntRotY(STRAT *strat, int objId)
{
	return strat->objId[objId]->crntRot.y;
}

float GetObjectCrntRotZ(STRAT *strat, int objId)
{
	return strat->objId[objId]->crntRot.z;
}
#endif

#if 0
void SetObjectCrntPosX(STRAT *strat, int objId, float amount)
{
	strat->objId[objId]->crntPos.x = amount;
}

void SetObjectCrntPosY(STRAT *strat, int objId, float amount)
{
	strat->objId[objId]->crntPos.y = amount;
}

void SetObjectCrntPosZ(STRAT *strat, int objId, float amount)
{
	strat->objId[objId]->crntPos.z = amount;
}
#endif

#if 0

void SetObjectCrntRotX(STRAT *strat, int objId, float amount)
{
	strat->objId[objId]->collFlag |= OBJECT_STRATMOVE;
	strat->objId[objId]->crntRot.x = amount;
}

void SetObjectCrntRotY(STRAT *strat, int objId, float amount)
{
	strat->objId[objId]->collFlag |= OBJECT_STRATMOVE;
	strat->objId[objId]->crntRot.y = amount;
}

void SetObjectCrntRotZ(STRAT *strat, int objId, float amount)
{
	strat->objId[objId]->collFlag |= OBJECT_STRATMOVE;
	strat->objId[objId]->crntRot.z = amount;
}
#endif
#if 0
#if 0
/////////////////////////////////////
#if 0

float GetObjectIdlePosX(STRAT *strat, int objId)
{
	return strat->objId[objId]->idlePos.x;
}

float GetObjectIdlePosY(STRAT *strat, int objId)
{
	return strat->objId[objId]->idlePos.y;
}

float GetObjectIdlePosZ(STRAT *strat, int objId)
{
	return strat->objId[objId]->idlePos.z;
}
#endif

#if 0
float GetObjectIdleRotX(STRAT *strat, int objId)
{

	return strat->objId[objId]->idleRot.x;
}

float GetObjectIdleRotY(STRAT *strat, int objId)
{
	return strat->objId[objId]->idleRot.y;
}

float GetObjectIdleRotZ(STRAT *strat, int objId)
{
	return strat->objId[objId]->idleRot.z;
}
#endif

float GetObjectCrntPosX(STRAT *strat, int objId)
{
	return strat->objId[objId]->crntPos.x;
}

float GetObjectCrntPosY(STRAT *strat, int objId)
{
	return strat->objId[objId]->crntPos.y;
}

float GetObjectCrntPosZ(STRAT *strat, int objId)
{
	return strat->objId[objId]->crntPos.z;
}
#endif

void RelTurnTowardXY(STRAT *strat, float x, float y, float z, float ang)
{
	Point3	p;
	Matrix	im;
	Vector3	forward;
	float	actualAng;

	forward.x = 0.0f;
	forward.y = 1.0f;
	forward.z = 0.0f;
	p.x = x;
	p.y = y;
	p.z = z;
	v3Dec(&p, &strat->pos);
	/*mInvert3d(&im, &strat->m);*/
	mInvertRot(&strat->m, &im);
	mPoint3Apply3(&p, &im);
	p.z = 0.0f;
	v3Normalise(&p);
	actualAng = (float) acos(v3Dot(&p, &forward));
	if (p.x < 0.0f)
		mPreRotateZ(&strat->m, ang * actualAng);
	else if (p.x > 0.0f)
		mPreRotateZ(&strat->m, -actualAng * ang);
}

void RelTurnTowardZ(STRAT *strat, float x, float y, float z, float ang)
{
	Point3	p;
	Matrix	im;
	Vector3	forward;
	float	actualAng;

	forward.x = 0.0f;
	forward.y = 1.0f;
	forward.z = 0.0f;
	p.x = x;
	p.y = y;
	p.z = z;
	v3Dec(&p, &strat->pos);
	/*mInvert3d(&im, &strat->m);*/
	mInvertRot(&strat->m, &im);
	mPoint3Apply3(&p, &im);
	p.x = 0.0f;
	v3Normalise(&p);
	actualAng = (float) acos(v3Dot(&p, &forward));
	if (p.z < 0.0f) 
		mPreRotateX(&strat->m, -actualAng * ang);
	else if (p.z > 0.0f)
		mPreRotateX(&strat->m, actualAng * ang);
}
#endif
#if 0
void ApplyUpForce(STRAT *strat, float	amount)
{
	Vector3 force;
	Point3	p;

	p.x = p.y = p.z = force.x = force.y = 0.0f;
	force.z = amount;
	ApplyForce(strat, &force, &p, 1.0f);
}
#endif

#if 0
// This routine taken out to be replaced with nice inline version
float	RandRange(float from , float to)
{
	float	val;

	val = ((float) (rand() & 1023)) * (0.000976562f * (to - from)) + from; 
	dAssert(((val >= from) && (val <= to)), "random range return wrong value\n");
	return val;
}
#endif
#if 0
void ApplyForceRelPos(STRAT *strat, int objId, float fx, float fy, float fz, float px, float py, float pz)
{
	Vector3	force;
	Point3	p, fromP;
	Matrix	m;

	
	if ((objId) && (px || py || pz))
	{
		fromP.x = px;
		fromP.y = py;
		fromP.z = pz;
		m = strat->objId[objId]->wm;
		mVector3Multiply(&p, &fromP, &m);

		p.x = strat->objId[objId]->wm.m[3][0] - strat->pos.x;
		p.y = strat->objId[objId]->wm.m[3][1] - strat->pos.y;
		p.z = strat->objId[objId]->wm.m[3][2] - strat->pos.z;
		
	}
	else
	{
		p.x = px;
		p.y = py;
		p.z = pz;
	}

	force.x = fx;
	force.y = fy;
	force.z = fz;

	ApplyForce(strat, &force, &p, 0.1f);
}
#endif

#if 0
void InitRotInit(STRAT *strat)
{  //MATT CHANGE
	return;
  //	if (strat->obj)
  //		initObj(strat->obj);
}
#endif

#if 0
float	CheckDistXY(STRAT *strat)
{
	float	xDif, yDif;

	xDif = strat->CheckPos.x - strat->pos.x;
	yDif = strat->CheckPos.y - strat->pos.y;

		return (float)sqrt((xDif * xDif) + (yDif * yDif));
}
#endif

#if 0
int LineOfSight(float ax, float ay, float az, float bx, float by, float bz, float dist)
{
	Point3	a, b, p, q;

	p.x = p.y = p.z = 0.0f;
	a.x = ax;
	a.y = ay;
	a.z = az;
	b.x = bx;
	b.y = by;
	b.z = bz;

	v3Sub(&q, &p, &a);
	if (v3Dot(&q, &q) < 50.0f * 50.0f)
		return 0;
	else
		return 1;
}
#endif

#if 0		
int	PlayerObjectHitFloor(int objId)
{
	if (player[currentPlayer].Player->objId[objId]->collFlag & COLL_ON_FLOOR)
		return 1;
	else
		return 0;
}
#endif

#if 0
void SetRotFlag(STRAT *strat)
{
	strat->obj->collFlag |= OBJECT_CALC_MATRIX;
}
#endif

#if 0
void Delay(int dLogs)
{
	int	i;

	for (i=0; i<dLogs; i++)
		dLog("time = %d\n", i);
	
}
#endif
#if 0
int	StratXCheckPos(STRAT *strat)
{
	STRAT *str;
	Vector3	stratForward;
	Vector3 TESTVEC,probe;
	float x,y,dist;

	stratForward.z = 0.0f;stratForward.x=0.0f;
	stratForward.y = (strat->relVel.y * strat->pnode->radius) + 1.0f;	
	mVector3Multiply(&TESTVEC,&stratForward,&strat->m);

	probe.x = strat->pos.x+TESTVEC.x;
	probe.y = strat->pos.y+TESTVEC.y;


	str = FirstStrat;
	while (str)
	{
		if ((str->pnode == NULL) || !(str->route))
		{
			str = str->next;
			continue;
		}
		if (strat->route->path->net != str->route->path->net)
		{
			str = str->next;
			continue;
		}
		if (str==strat)
		{

			str = str->next;
			continue;

		}

		
		x = (str->pos.x - probe.x);
		y = (str->pos.y - probe.y);


		dist = (float)sqrt((x*x)+(y*y));

		if (dist < (strat->pnode->radius * strat->scale.x))
			return(1);
		
		str = str->next;
	}

	return 0;
}
#endif
#if 0
int CollideStratPos(STRAT *strat,Vector3 *pos)
{
	STRAT *str;
	float x,y,dist;


	str = FirstStrat;
	while (str)
	{

		if ((!str->pnode) || (!(str->flag & STRAT_COLLSTRAT)))
		{
			str = str->next;
			continue;
		}




		if (str->flag & STRAT_BULLET)
		{
			str = str->next;
			continue;
		}

	   	if ((str==strat) || (str == strat->parent))
	   //	if ((str==strat) || (str == strat->parent) || (str == strat->target))
		{

			str = str->next;
			continue;

		}


	 	if (pointLineDistanceFix(&str->pos,&strat->pos,pos) <= (str->pnode->radius))
			return(1);

		str = str->next;
	}

	return 0;



}
#endif

#if 0
int CollideStratPos(STRAT *strat,Vector3 *pos)
{
	STRAT *str;
	float x,y,dist;


	str = FirstStrat;
	while (str)
	{

		if ((!str->pnode) || (!(str->flag & STRAT_COLLSTRAT)))
		{
			str = str->next;
			continue;
		}




		if (str->flag & STRAT_BULLET)
		{
			str = str->next;
			continue;
		}

	   	if ((str==strat) || (str == strat->parent))
	   //	if ((str==strat) || (str == strat->parent) || (str == strat->target))
		{

			str = str->next;
			continue;

		}

		x = (str->pos.x - pos->x);
		y = (str->pos.y - pos->y);


		dist = (float)sqrt((x*x)+(y*y)); 

		if (dist < (strat->pnode->radius ))
		{

			return(1);
		}

		str = str->next;
	}

	return 0;



}
#endif

#if 0
Matrix shipOldM = {1.0f, 0.0f, 0.0f, 0.0f, 
				   0.0f, 1.0f, 0.0f, 0.0f,
				   0.0f, 0.0f, 1.0f, 0.0f,
				   0.0f, 0.0f, 0.0f, 1.0f};

void recApplyRot(Object *obj, Point3 *p, Matrix *m)
{
	int	i;
	Point3	newP;

	for (i=0; i<obj->ncp; i++)
	{
		v3Sub(&newP, &obj->ocpt[i], p);
		mPoint3Apply3(&newP, m);
		v3Inc(&newP, p);
		obj->ocpt[i] = newP;
	}

	for (i=0; i<obj->no_child; i++)
		recApplyRot(&obj->child[i], p, m);
}


void ApplyRotToPlayer(STRAT *strat)
{
	Matrix a, b, c, invB;
	Point3	p;

	b = shipOldM;
	c = strat->m;
	mInvertRot(&shipOldM, &invB);
	mMultiply(&a, &invB, &c);
	shipOldM = strat->m;
	if (player[currentPlayer].Player->obj)
		recApplyRot(player[currentPlayer].Player->obj, &strat->pos, &a);

	mPreMultiply(&player[currentPlayer].Player->m, &a);
	v3Sub(&p, &player[currentPlayer].Player->pos, &strat->pos);
	mPoint3Apply(&p, &a);
	v3Inc(&p, &strat->pos);
	player[currentPlayer].Player->pos = p;
}
#endif

#if 0

enum {ERROR,NORMAL,MATRIX,INTERPOLATION};

void ObjectTowardPlayerOffsetXZ(STRAT *strat, int	objId, float angx, float angz, float angXOffset, float angZOffset)
{
	Matrix	rotMat, im;
	Point3	relP;
	float	cangx, cangz;
	Vector3	temp, temp2;
	int MODE;

	/*HAS THE STRAT GOT AN ANIM*/

	if (strat->anim)
	{
		/*IS IT A MODEL ANIM*/
		if (strat->anim->anim.anim)
		{

			/* A CHECK TO ENSURE IT IS VALID TO MOVE THE OBJECT*/
		   	if (strat->objId[objId]->collFlag & OBJECT_STRATMOVE)
			{	 
				if (strat->pnode->anim->anim->type&INTERPOLATE_ANIM)
					MODE = INTERPOLATION;
				else
					MODE = MATRIX;
			}
			else
				MODE = ERROR;	 

		}
		else
			MODE = NORMAL;

	
	}
	else
		MODE = NORMAL;



	if (!MODE)
		return;

	rotMat = strat->m;

	/*GRAB THE ROTATION MATRIX FOR THE STRAT*/
	switch (MODE)
	{
		case (NORMAL):
			findTotalRotMatrix(&rotMat, strat->objId[objId]);
 			break;

		case (INTERPOLATION):
			findTotalRotMatrixInterp(&rotMat, strat->objId[objId]);
 			break;

	}	
	mPreRotateX(&rotMat, angXOffset);
	mPreRotateZ(&rotMat, angZOffset);
	mInvertRot(&rotMat, &im);
	mUnit(&rotMat);
	mPreTranslate(&rotMat, strat->pos.x, strat->pos.y, strat->pos.z);
	mPreMultiply(&rotMat, &strat->m);
	mPreScale(&rotMat, strat->scale.x, strat->scale.y, strat->scale.z);

	switch (MODE)
	{
		case (NORMAL):
			findTotalMatrix(&rotMat, strat->objId[objId]);
 			break;

		case (INTERPOLATION):
			findTotalMatrixInterp(&rotMat, strat->objId[objId]);
 			break;

	}	
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
	temp = relP;
	temp2 = temp;
	temp2.x = 0.0f;
	v3Normalise(&temp);
	v3Normalise(&temp2);
	cangz = acos(v3Dot(&temp, &temp2));

	if (cangx > 0.001f)
	{
		if (relP.z > 0.0)
		{
			if (angx > cangx)
			{
				strat->objId[objId]->idleRot.x += cangx;
			}
			else
			{
				strat->objId[objId]->idleRot.x += angx;
			}
		}
		else
		{
			if (angx > cangx)
			{
				strat->objId[objId]->idleRot.x -= cangx;
			}
			else
			{
				strat->objId[objId]->idleRot.x -= angx;
			}
		}
	}

	if (cangz > 0.001f)
	{
		if (relP.x > 0.0f)
		{
			if (angz > cangz)
			{
				strat->objId[objId]->idleRot.z -= cangz;
			}
			else
			{
				strat->objId[objId]->idleRot.z -= angz;
			}
		}
		else
		{
			if (angz > cangz)
			{
				strat->objId[objId]->idleRot.z += cangz;
			}
			else
			{
				strat->objId[objId]->idleRot.z += angz;
			}
		}
	}
}
#endif

#if 0

void PlayerGunUp(float amount)
{
	if ((amount > 0.0f) && (player[currentPlayer].Player->objId[5]->idleRot.x > -0.2f))
		player[currentPlayer].Player->objId[5]->idleRot.x -= amount*0.5f;
	if ((amount < 0.0f) && (player[currentPlayer].Player->objId[5]->idleRot.x < 0.7f))
		player[currentPlayer].Player->objId[5]->idleRot.x -= amount*0.5f;
}


void PlayerGunRot(float yRot, float xRot)
{

	if (xRot < 0.0f)
	{
		xRot = (xRot * 0.5f) + 0.3f;
		
	}
	else
	{
		xRot = (0.2f * xRot) + 0.3f; 
	}
	player[currentPlayer].Player->objId[5]->idleRot.x = xRot;
	player[currentPlayer].Player->objId[5]->idleRot.z = -yRot;

}


void PlayerGunLeft(float amount)
{
		player[currentPlayer].Player->objId[5]->idleRot.z += amount;	
}
#endif
#if 0
void PushRedDogAlongPath(STRAT *strat, float amount)
{
	int	i, closestLine = 0;
	Point3	ps, pe, pos, closestPoint;
	Vector3	v, v2, playerForward;
	float	minDist = 1000.0f, dist;
	float	lambda;
	Vector3	lDir, temp;
	Point3	pOnL;

	/*if (Player->splineData == NULL)*/
	if (player[currentPlayer].Player->route == NULL)
	{

		player[currentPlayer].Player->route = GetNextRoute(player[currentPlayer].Player);
		if (player[currentPlayer].Player->route)
		{
			player[currentPlayer].Player->route->splineData = (SplineData *)MissionAlloc (sizeof(SplineData));
			player[currentPlayer].Player->route->splineData;
			player[currentPlayer].Player->route->splineData->p = strat->route->path->waypointlist[0];
			player[currentPlayer].Player->route->splineData->lineSeg = -1;
			player[currentPlayer].Player->route->splineData->t = 0.0f;
		}
	}

	for (i=0; i<strat->route->path->numwaypoints - 1; i++)
	{
		ps = strat->route->path->waypointlist[i];
		pe = strat->route->path->waypointlist[i+1];
		if (((ps.z > player[currentPlayer].Player->pos.z + 20.0f) && (pe.z > player[currentPlayer].Player->pos.z + 20.0f)) ||
			((ps.z < player[currentPlayer].Player->pos.z - 20.0f) && (pe.z < player[currentPlayer].Player->pos.z - 20.0f)))
			continue;

		v3Sub(&lDir, &pe, &ps);
		v3Sub(&temp, &player[currentPlayer].Player->pos, &ps);
		lambda = v3Dot(&lDir, &temp) / v3Dot(&lDir, &lDir);
		lambda = RANGE(0.0f, lambda, 1.0f);
		v3Scale(&lDir, &lDir, lambda);
		v3Add(&pOnL, &ps, &lDir);
		v3Sub(&temp, &pOnL, &player[currentPlayer].Player->pos);
		dist = (float) sqrt(v3Dot(&temp, &temp));

		if ( dist < minDist)
		{
			minDist = dist;
			closestLine = i;
			closestPoint = pOnL;
		}
	}
	if (minDist > 100.0f)
		return;

	ps = strat->route->path->waypointlist[closestLine];
	pe = strat->route->path->waypointlist[closestLine+1];
	v3Sub(&v, &pe, &ps);
	v3Sub(&v2, &closestPoint, &ps);

	SplinePathPos(player[currentPlayer].Player->route->splineData, strat->route, player[currentPlayer].Player->route->splineData->lineSeg, v3Length(&v2) / v3Length(&v));
	v = player[currentPlayer].Player->route->splineData->oldp_p;

	playerForward.x = player[currentPlayer].Player->m.m[1][0];
	playerForward.y = player[currentPlayer].Player->m.m[1][1];
	playerForward.z = player[currentPlayer].Player->m.m[1][2];
	v3Normalise(&playerForward);
	v3Normalise(&v);

	/*if ((currPushV.x != 0.0f) || (currPushV.y != 0.0f) || (currPushV.z != 0.0f))
		v3Normalise(&currPushV);
	else
		currPushV = v;
		

	if (acos(v3Dot(&currPushV, &v)) > 0.01f)
	{
		morphVectorRotate(&currPushV, &currPushV, &v, 0.02f);
		v = currPushV;
	}*/
	v3Scale(&v, &v, amount);
	pos.x = pos.y = pos.z = 0.0f;
	v3Scale(&playerForward, &playerForward, 5.0f);
	v3Inc(&pos, &playerForward);
	ApplyForce(player[currentPlayer].Player, &v, &pos, 0.01f);
}
#endif

#if 0
int	PlayerInRadius(STRAT *strat)
{
	Point3	playerToStrat;

	v3Sub(&playerToStrat, &player[currentPlayer].Player->pos, &strat->pos);
	if ((strat->pnode->radius * strat->pnode->radius) > v3Dot(&playerToStrat, &playerToStrat))
		return 1;
	else
		return 0;
}

void SetPlayerFriction(float x, float y, float z)
{
	if (player[currentPlayer].Player)
	{
		player[currentPlayer].Player->friction.x = 1.0f - x;
		player[currentPlayer].Player->friction.y = 1.0f - y;
		player[currentPlayer].Player->friction.z = 1.0f - z;
	}
}

void ApplyForcePlayer(float x, float y, float z)
{
	Vector3	force;
	Point3	p;

	p.x = p.y = p.z = 0.0f;
	force.x = x;
	force.y = y;
	force.z = z;

	ApplyForce(player[currentPlayer].Player, &force, &p, 0.1f);
}

void ApplyForcePlayerRel(float x, float y, float z)
{
	Vector3	xAxis, yAxis, zAxis, force;
	Point3	p;

	p.x = p.y = p.z = 0.0f;

	xAxis.x = x * player[currentPlayer].Player->m.m[0][0];
	xAxis.y = x * player[currentPlayer].Player->m.m[0][1];
	xAxis.z = x * player[currentPlayer].Player->m.m[0][2];

	yAxis.x = y * player[currentPlayer].Player->m.m[1][0];
	yAxis.y = y * player[currentPlayer].Player->m.m[1][1];
	yAxis.z = y * player[currentPlayer].Player->m.m[1][2];

	zAxis.x = z * player[currentPlayer].Player->m.m[2][0];
	zAxis.y = z * player[currentPlayer].Player->m.m[2][1];
	zAxis.z = z * player[currentPlayer].Player->m.m[2][2];

	force.x =(xAxis.x + yAxis.x + zAxis.x);
	force.y =(xAxis.y + yAxis.y + zAxis.y);
	force.z =(xAxis.z + yAxis.z + zAxis.z);

	ApplyForce(player[currentPlayer].Player, &force, &p, 0.1f);
}
#endif








void FindNearestWay(STRAT *strat)
{
	float lastdist,dist;
	int	way1,way;
	ROUTE *route=strat->route;
	lastdist=10000.0f;
	for (way=0;way<route->path->numwaypoints-1;way++)
	{
		dist = WayToStratDistNoRoot(&route->path->waypointlist[way],strat);
		if (dist < lastdist)
		{
			lastdist = dist;
			way1 = way;
		}
	}
	route->curway=way1;
	strat->CheckPos.x = route->path->waypointlist[route->curway].x;  
	strat->CheckPos.y = route->path->waypointlist[route->curway].y;  
	strat->CheckPos.z = route->path->waypointlist[route->curway].z;  
}


#if 0
void SplineIntersectionPoint(STRAT *strat,SPLINEBOX *splinearea)
{
	int   numpoints = splinearea->numpoints,i;
	Vector2 SegmentPointA,SegmentPointB,point,point2,probe;
	Vector3 point3;
	float	Lambda,x,y,s,L,dist;
	int CollideSegment=0,Line,Nearest=-1;
	float lastdist = 10000.0f;
    Vector3 Force,forward;
	Vector3 a,b;
   	float ang;
	Vector3 POSVEC;
	Colour col;

	forward.z = 0.0f;forward.x=0.0f;

	if (!strat->relVel.y)
		forward.y = -1.5f ;
	else
		forward.y = 1.5f * -(strat->relVel.y);
	
	mVector3Multiply(&TESTVEC,&forward,&strat->m);

	probe.x = strat->pos.x+TESTVEC.x;
	probe.y = strat->pos.y+TESTVEC.y;

	for (i=0;i<numpoints;i++)

	{

/*		Line=OutRegions[i]; */
		Line=i;
		SegmentPointA=splinearea->pointlist[Line];
		
		/*CHECK FOR LAST SEGMENT */
		if (Line==numpoints-1)
			SegmentPointB = splinearea->pointlist[0];
		else
			SegmentPointB = splinearea->pointlist[Line+1];

		x = SegmentPointB.x - SegmentPointA.x;
		y = SegmentPointB.y - SegmentPointA.y;
		L = ((x*x) + (y*y));
		Lambda = (float)sqrt(L);

		s = ((((SegmentPointB.y - strat->pos.y) * (-x) )
		-	((SegmentPointB.x - strat->pos.x) * (-y) )))
			/ L;

		dist = (float)fabs(s * Lambda);
		/*dist = (s * Lambda); */

		y = probe.y;
		x = probe.x;

		
		if (dist < lastdist)
		{
			lastdist = dist;
			Nearest = Line;
			
		}
	}



	SegmentPointA = splinearea->pointlist[Nearest];
	if (Nearest==numpoints-1)
		SegmentPointB = splinearea->pointlist[0];
	else
		SegmentPointB = splinearea->pointlist[Nearest+1];


	x = SegmentPointA.x - SegmentPointB.x;
	y = SegmentPointA.y - SegmentPointB.y;

	x /= 2.0f;
	y /= 2.0f;

	point3.x = strat->pos.x  - x;

	point3.y = strat->pos.y  - y;

	point3.z = 0;


	dist = (float)sqrt((x*x) + (y*y));


	ApplyForce(strat, &point3, NULL, dist);

	return;



	
	/*FIND THE NEAREST POINT TO THE PLAYER*/
	point.x = SegmentPointA.x;
	point.y = SegmentPointA.y;

	point2.x = SegmentPointB.x;
	point2.y = SegmentPointB.y;

	if (WayToPlayerDistXYNoRoot(&point) < WayToPlayerDistXYNoRoot(&point2))
	{
		strat->CheckPos.x = point.x;
		strat->CheckPos.y = point.y;
	}
	else
	{
		strat->CheckPos.x= point2.x;
		strat->CheckPos.y = point2.y;


	}


	
}
#endif

#if 0
static void DeflectMeSpline(Vector2 *Probe,STRAT *strat,SPLINEBOX *splinearea)
{
	Matrix M;
	int   numpoints = splinearea->numpoints,i;
	Vector2 SegmentPointA,SegmentPointB;
	float	Lambda,x,y,s,L,dist;
	int CollideSegment=0,Line,Nearest=-1;
	float lastdist = 10000.0f;
    Vector3 Force;
	Vector3 a,b;
   	float ang;
	Vector3 POSVEC;
	Colour col;

	
	col.argb.a = col.argb.b = col.argb.g = col.argb.r = 0xff;




  /*	return; */

	/*FIND THE NEAREST POINT ON THE INTERSECTING SPLINE REGION */
	
	for (i=0;i<numpoints;i++)

	{

/*		Line=OutRegions[i]; */
		Line=i;
		SegmentPointA=splinearea->pointlist[Line];
		
		/*CHECK FOR LAST SEGMENT */
		if (Line==numpoints-1)
			SegmentPointB = splinearea->pointlist[0];
		else
			SegmentPointB = splinearea->pointlist[Line+1];

		x = SegmentPointB.x - SegmentPointA.x;
		y = SegmentPointB.y - SegmentPointA.y;
		L = ((x*x) + (y*y));
		Lambda = (float)sqrt(L);

		s = ((((SegmentPointB.y - strat->pos.y) * (-x) )
		-	((SegmentPointB.x - strat->pos.x) * (-y) )))
			/ L;

		dist = (float)fabs(s * Lambda);
		/*dist = (s * Lambda); */

		y = Probe->y;
		x = Probe->x;

		
		if (dist < lastdist)
		{
			lastdist = dist;
			Nearest = Line;
			
		}
	}



	SegmentPointA = splinearea->pointlist[Nearest];
	if (Nearest==numpoints-1)
		SegmentPointB = splinearea->pointlist[0];
	else
		SegmentPointB = splinearea->pointlist[Nearest+1];

	
	x = (SegmentPointB.x - SegmentPointA.x);
	y = (SegmentPointB.y - SegmentPointA.y);

	a.x =x;
	a.y =y;
	a.z =1.0f;
	v3Normalise(&a);
	
	b.x = (Probe->x - strat->pos.x);
	b.y = (Probe->y - strat->pos.y);
	b.z =1.0f;
	v3Normalise(&b);

 	ang = (float)acos((double)v3Dot(&a,&b));

 	POSVEC.x = 0.0f;
	POSVEC.z = 0.0f;
	POSVEC.y = 0.0f;

	POSVEC.y = 2.0f * strat->pnode->radius;
	M = strat->m;

 	if (ang>=(PI2/4.0f))
		mPreRotateZ(&M,ang);
	else
		mPreRotateZ(&M,-ang);

	mVector3Multiply(&Force,&POSVEC,&M);

  	SetCheckPos(strat,strat->pos.x + Force.x,strat->pos.y + Force.y, strat->pos.z);	

}

static void DeflectMeBox(STRAT *strat,BOXBOX *boxarea)
{	
	Vector2 pos;
    Vector3 Force,AppForcePoint,POSVEC,Direction;
	float ang;
	Matrix M;


/*	dLog("deflecting from box area \n"); */

	pos.x = ((boxarea->maxx) - (boxarea->minx))/2.0f;
	pos.y = ((boxarea->maxy) - (boxarea->miny))/2.0f;

	AppForcePoint.x = TESTVEC.x;
	AppForcePoint.y = TESTVEC.y;
	AppForcePoint.z=0;

    Force.x = pos.x - (strat->pos.x+TESTVEC.x);
	Force.y = pos.y - (strat->pos.y+TESTVEC.y);
	Force.z=0;		


	
	
	/*CENTRE POINT TO PROBE	 */
	
    Force.x = -(pos.x - (strat->pos.x+TESTVEC.x));/* + 1.0f; */
	Force.y = -(pos.y - (strat->pos.y+TESTVEC.y));/* + 1.0f; */
	Force.z=1.0f;	
	
	
	/*PROBE TO STRAT */

	POSVEC.x = -TESTVEC.x;
	POSVEC.y = -TESTVEC.y;
	POSVEC.z = 1.0f;
	
	v3Cross(&Direction,&POSVEC,&Force);

 	Force.x = strat->pos.x - pos.x;

  	Force.y = strat->pos.y - pos.y;

   	ang = (PI2/4.0f)/(float)sqrt((Force.x * Force.x) + (Force.y * Force.y)) * 0.005f ;

	M = strat->m;

   	if (Direction.z < 0)
   	   	mPreRotateZ(&M,-ang);
   	else
   	   	mPreRotateZ(&M,ang);
  
	POSVEC.x = 5.5f; POSVEC.y = 0.0f;
	POSVEC.z = 0.0f;

	mVector3Multiply(&Force,&POSVEC,&M);

	SetCheckPos(strat,strat->pos.x + Force.x,strat->pos.y + Force.y, strat->pos.z);	

}





static void DeflectMe(STRAT *strat,Vector2 *pos,float def)
{	
    Vector3 Force,AppForcePoint,Direction,POSVEC;
	float amount,turn,ang;
	Matrix M;

	AppForcePoint.x = TESTVEC.x;
	AppForcePoint.y = TESTVEC.y;
	AppForcePoint.x = strat->pos.x;
	AppForcePoint.y = strat->pos.y;
	AppForcePoint.z=0;

    Force.x = pos->x - (strat->pos.x+TESTVEC.x);/* + 1.0f; */
	Force.y = pos->y - (strat->pos.y+TESTVEC.y);/* + 1.0f; */
	Force.z=1.0f;	
	
	


	amount = def-(float)sqrt((Force.x*Force.x)+(Force.y*Force.y));

	/*smaller the distance the greater the turn rate */
	turn=0.1f; 
	turn=9.1f; 
	amount = 3.0f * amount;
	/*dLog("amount %f\n",amount); */
	

	
	
	/*CENTRE POINT TO PROBE	 */
	
    Force.x = (pos->x - (strat->pos.x+TESTVEC.x));/* + 1.0f; */
	Force.y = (pos->y - (strat->pos.y+TESTVEC.y));/* + 1.0f; */
	Force.z=1.0f;	
 
	
	v3Normalise(&Force);
	
	/*PROBE TO STRAT */

	POSVEC.x = TESTVEC.x;
	POSVEC.y = TESTVEC.y;
	POSVEC.z = 1.0f;
	v3Normalise(&POSVEC);
	
	v3Cross(&Direction,&POSVEC,&Force);

   	ang = (float)acos((double)v3Dot(&POSVEC,&Force));
 	Force.x = strat->pos.x - pos->x;

  	Force.y = strat->pos.y - pos->y;

  /* 	ang = (PI2/4.0f)/(float)sqrt((Force.x * Force.x) + (Force.y * Force.y)) * 0.005f ; */
  /* 	ang = (PI2/4.0f); */
   /*	ang *= strat->relVel.y; */

	M = strat->m;

   	/*ang = PI2/4.0f; */
	ang = PI2/4.0f - ang;
   	if (Direction.z < 0)
/*   	if (ang >= PI2/4.0f) */
	{
	POSVEC.x = 0.0f * strat->pnode->radius; POSVEC.y = 0.0f;
	POSVEC.z = 0.0f;
	POSVEC.y = 4.0f * strat->pnode->radius;

   	   	mPreRotateZ(&M,ang);
  /* 	   	mPreRotateZ(&M,PI2/4.0f); */
	}
	else
	{
   /*	ang = PI2/4.0f; */
	POSVEC.x = 0.0f * strat->pnode->radius; POSVEC.y = 0.0f;
	POSVEC.y = 4.0f * strat->pnode->radius;

	POSVEC.z = 0.0f;

   	   	mPreRotateZ(&M,-ang);
/*   	   	mPreRotateZ(&M,-PI2/4.0f); */
  	 }

	mVector3Multiply(&Force,&POSVEC,&M);

	SetCheckPos(strat,strat->pos.x + Force.x,strat->pos.y + Force.y, strat->pos.z);	

}

#define TENDEG (PI2/360.0f)*10.0f

static void DeflectMe3(STRAT *strat,Vector2 *pos,float def)
{	
    Vector3 Force,AppForcePoint,Direction,POSVEC;
	float amount,turn,ang;
	Matrix M;

	AppForcePoint.x = TESTVEC.x;
	AppForcePoint.y = TESTVEC.y;
	AppForcePoint.x = strat->pos.x;
	AppForcePoint.y = strat->pos.y;
	AppForcePoint.z=0;

    Force.x = pos->x - (strat->pos.x+TESTVEC.x);/* + 1.0f; */
	Force.y = pos->y - (strat->pos.y+TESTVEC.y);/* + 1.0f; */
	Force.z=1.0f;	
	
	


	amount = def-(float)sqrt((Force.x*Force.x)+(Force.y*Force.y));

	/*smaller the distance the greater the turn rate */
	turn=0.1f; 
	turn=9.1f; 
	amount = 3.0f * amount;
	/*dLog("amount %f\n",amount); */
	

	
	
	/*CENTRE POINT TO PROBE	 */
	
    Force.x = (pos->x - (strat->pos.x+TESTVEC.x));/* + 1.0f; */
	Force.y = (pos->y - (strat->pos.y+TESTVEC.y));/* + 1.0f; */
	Force.z=1.0f;	
 
	
	v3Normalise(&Force);
	
	/*PROBE TO STRAT */

	POSVEC.x = TESTVEC.x;
	POSVEC.y = TESTVEC.y;
	POSVEC.z = 1.0f;
	v3Normalise(&POSVEC);
	
	v3Cross(&Direction,&POSVEC,&Force);

   	ang = (float)acos((double)v3Dot(&POSVEC,&Force));
 	Force.x = strat->pos.x - pos->x;

  	Force.y = strat->pos.y - pos->y;

	M = strat->m;

	if (ang < TENDEG)
		ang = TENDEG;

   	if (Direction.z < 0)
	{
		POSVEC.x = 0.0f * strat->pnode->radius; POSVEC.y = 0.0f;
		POSVEC.z = 0.0f;
		POSVEC.y = 4.0f * strat->pnode->radius;

   	   	mPreRotateZ(&M,ang);
	}
	else
	{
		POSVEC.x = 0.0f * strat->pnode->radius; POSVEC.y = 0.0f;
		POSVEC.y = 4.0f * strat->pnode->radius;
		POSVEC.z = 0.0f;

   	   	mPreRotateZ(&M,-ang);
  	 }

	mVector3Multiply(&Force,&POSVEC,&M);

	SetCheckPos(strat,strat->pos.x + Force.x,strat->pos.y + Force.y, strat->pos.z);	

}
#endif

#if 0
int IntersectCircle(STRAT *strat,Vector2 *C,Vector2 *A,Vector2 *B,float distance)
{
	Vector3 a;
	float strattopoint,strattoplay;
	float upper,lower,lambda,x,y,real;
	Vector2 P;


	a.x = B->x - A->x;
	a.y = B->y - A->y;
	/*a.z = 1.0f; */


	upper = -(a.x * A->x) + (a.x * C->x) - (a.y * A->y) +(a.y * C->y);
	lower = (a.x * a.x) + (a.y * a.y);

	lambda = upper/lower;


	P.x = A->x + (lambda * a.x);
	P.y = A->y + (lambda * a.y);
	
	x = strat->pos.x - P.x;
	y = strat->pos.y - P.y;
	strattopoint = (x*x)+(y*y);


	strattoplay = (a.x*a.x)+(a.y*a.y);


	if (strattoplay < strattopoint)
		return(OUTSIDE);

	
	x = C->x - P.x;
	y = C->y - P.y;
 	real = (float)((x*x) + (y*y));
	
	if (real < distance)
  		return(INSIDE);

	else
		return(OUTSIDE);
}
#endif





#if 0 
void Roll  (STRAT *strat, float amount)
{
	strat->turn.y = amount;

}
void Pitch (STRAT *strat, float amount)
{
	strat->turn.x = amount;
}
#endif

#if 0
void StratClean()
{
	STRAT *strat,*temp;

	strat = FirstStrat;

	while (strat)
	{
		temp=strat->next;
		RemoveStrat(strat);
		strat=temp;	
	}
}
#endif





#if 0
void ScaleX (STRAT *strat, float amount)
{
	strat->scale.x = amount;
}

void ScaleY (STRAT *strat, float amount)
{
	strat->scale.y = amount;
}

void ScaleZ (STRAT *strat, float amount)
{
	strat->scale.z = amount;
}
#endif



#if 0
void ResetObject(STRAT *strat, int objId)
{
	strat->objId[objId]->idlePos.x = 0.0f;
	strat->objId[objId]->idlePos.y = 0.0f;
	strat->objId[objId]->idlePos.z = 0.0f;
	strat->objId[objId]->idleRot.x = 0.0f;
	strat->objId[objId]->idleRot.y = 0.0f;
	strat->objId[objId]->idleRot.z = 0.0f;
}
#endif



//mode 0 keep camera as normal game camera
//mode 1 set playerheld and move camera accordingly
void PauseAllStrats(STRAT *me,int mode)
{
	STRAT *strat;
	
	
	strat = FirstStrat;

 /*	dLog("strats\n"); */

	if (!mode)
	{
		if (player[currentPlayer].CameraStrat)
			player[currentPlayer].CameraStrat->var = CAMERASCRIPTVAR;
	}
	else
		player[currentPlayer].PlayerHeld = 1;

	while (strat)
	{
		if ((strat != me) && (strat != player[currentPlayer].CameraStrat))
		{
			if (strat->trigger)
				PauseTriggers(strat,-1,0);
			strat->flag2 |= STRAT2_PROCESSSTOPPED;


		}
		
		strat=strat->next;
	}
}
//mode 0 keep camera as normal game camera
//mode 1 set playerheld and move camera accordingly


void UnPauseAllStrats(STRAT *me,int mode)
{
	STRAT *strat;
	
	strat = FirstStrat;

 /*	dLog("strats\n"); */
  if (!mode)
	{
		if (player[currentPlayer].CameraStrat)
			player[currentPlayer].CameraStrat->var = CAMERANORMALVAR;
	}
	else
		player[currentPlayer].PlayerHeld = 0;

//	PlayerHeld = 0;
	while (strat)
	{
		if (strat != me)
		{
			if (strat->trigger)
				UnPauseTriggers(strat);
			strat->flag2 = strat->flag2 & LNOT(STRAT2_PROCESSSTOPPED);


		}
		strat=strat->next;
	}
}








#if 0
int LineSightParent(STRAT *strat)
{
	Vector2 player;
/*	short box = strat->path->area; */
	short box = strat->route->path->area;
	MAPAREA *Area;
	int subarea;
	float radius,radius2;



	/*IF NO DEFINED AREA FOR THIS STRAT THEN RETURN AS BEING INSIDE ALWAYS */
	if (box <0)
	{
		/*TESTLINE = 1; */
		return(VISIBLE);
	}

	Area = &MapAreas[box];

	player.x = strat->parent->pos.x;
	player.y = strat->parent->pos.y;


	/*CHECK INTERSECTION WITH CIRCULAR REGIONS */
   	for (subarea=0;subarea<Area->numcircleareas;subarea++)
	{
		radius = Area->circlebbox[subarea].radius;
	  	 	radius2=radius*radius;
   
		if (!LineSightCircle(strat,&Area->circlebbox[subarea].pos,&player,radius2))
		{

			/*put something down at the centre point of the circle */

			DRAWVEC = Area->circlebbox[subarea].pos;
		  	/*TESTLINE = 0; */
			return (NONVISIBLE);
		}	
	} 		 

   /*	player.x = Player->pos.x;
	player.y = Player->pos.y;
	 */

   	for (subarea=0;subarea<Area->numlinesightareas;subarea++)
	{
		 	if (IntersectSplineGen(strat,&Area->linesightbbox[subarea],&player))
				return (NONVISIBLE);
	} 

	

	/*THESE TWO RECENTLY ADDED*/
   	for (subarea=0;subarea<Area->numsplineareas;subarea++)
	{
		 	if (IntersectSplineGen(strat,&Area->splinebbox[subarea],&player))
				return (NONVISIBLE);
	} 

   	for (subarea=0;subarea<Area->numextsplineareas;subarea++)
	{
		 	if (IntersectSplineGen(strat,&Area->extsplinebbox[subarea],&player))
				return (NONVISIBLE);
	} 

	return(VISIBLE);
}


int FireSightPlayer(STRAT *strat)
{
	Vector2 player;
/*	short box = strat->path->area; */
	short box = strat->route->path->area;
	MAPAREA *Area;
	int subarea;
	float radius,radius2;



	/*IF NO DEFINED AREA FOR THIS STRAT THEN RETURN AS BEING INSIDE ALWAYS */
	if (box <0)
	{
		/*TESTLINE = 1; */
		return(VISIBLE);
	}

	Area = &MapAreas[box];

	player.x = Player->pos.x;
	player.y = Player->pos.y;


	player.x = Player->pos.x;
	player.y = Player->pos.y;


   	for (subarea=0;subarea<Area->numlinesightareas;subarea++)
	{
		 	if (IntersectSplineGen(strat,&Area->linesightbbox[subarea],&player))
				return (NONVISIBLE);
	} 

	

	return(VISIBLE);
}
#endif




int GetTargetProxim(STRAT *parent,float dist)
{
	STRAT *strat;
	float x,y,z;
	dist *= dist;

	strat = FirstStrat;

	while (strat)
	{
		if ((strat != player[currentPlayer].Player) && (strat != player[currentPlayer].CameraStrat)
			&& (strat != parent)
			&& (strat != parent->target)
			&& ((strat->flag & STRAT_COLLSTRAT))
			&& (!(strat->flag & STRAT_FRIENDLY))
			&& (!(strat->flag & STRAT_BULLET)) 
			&& (!(strat->flag & STRAT_HIDDEN)) 
			&& (!(strat->flag & STRAT_EXPLODEPIECE)) 
			&& ((strat->flag > STRAT_ALIVE)))
			{
				x = strat->pos.x - parent->pos.x;
				y = strat->pos.y - parent->pos.y;
				z = strat->pos.z - parent->pos.z;

				if (((x*x)+(y*y)+(z*z)) <= dist)
				{
					if (strat->pnode)
					dLog("NAME %s\n",strat->pnode->name);
					parent->target = strat;
					parent->CheckPos.x = strat->pos.x;
					parent->CheckPos.y = strat->pos.y;
					parent->CheckPos.z = strat->pos.z;
					return(1);

				}

			}

		strat = strat->next;

	}

	return(0);

}

int TargetNearPlayer(STRAT *strat,float dist)
{
	float real;
	double x,y,z;
	if (abs(strat->relVel.y > 1.0f))
		dist *= (float)fabs(strat->relVel.y);		
	
	x = (strat->CheckPos.x-player[currentPlayer].Player->pos.x );
	y = (strat->CheckPos.y-player[currentPlayer].Player->pos.y );
	z = (strat->CheckPos.z-player[currentPlayer].Player->pos.z );


	real =(float)sqrt((x*x)+(y*y)+(z*z));


	if (real <=(NearestTarget->strat->pnode->radius*NearestTarget->strat->scale.x+dist))
		return(1);
	else
		return(0);
}

int NextReverseWayPoint(STRAT *strat)
{
	float playertowaydist,strattowaydist;
	int way1;
	ROUTE *route = strat->route;

	way1 = route->curway-1;
	if (way1 <0)
		way1= 0;


	playertowaydist= WayToPlayerDist(&route->path->waypointlist[way1]);
	strattowaydist = WayToStratDist(&route->path->waypointlist[way1],strat);


	if (playertowaydist < strattowaydist)
		GetNextWay(strat);
	else
		GetPrevWay(strat);

	return(0);


}









#if WAD

#if SH4
	void MapInit(Stream *fp)

#else
	void MapInit(char *file)

#endif
{	
	#if !SH4
		FILE *fp;
	#endif
	char MapFile[128];
	char objname[64];
	int	TempAnims[64];
	short NumPaths,NumAreas,NumWayPoints,patrol;
	int MapEntry,loop,i,entry,way,test,len;
	unsigned char NumPathPoints,NumPatrolRoutes, LevelFlags;
	MAPTRIGGER *StartTrig;
	MAPEVENT *StartEvent;
 	short NumCamMods;
	int light;
	int nBytes;
	float tempf;

	STRAT *strat;
	int num;
	float fnum;

	ResetRandom();
	num = rand();
	fnum = frand();
	
	#if !SH4
	  	dLog("STRAT SIZE %d\n",sizeof(STRAT));
	
		sprintf(MapFile,"%s%s%s",MAPDIR,file,".MAP");

		fp=fopen(MapFile, "rb");

		dAssert (fp, "COULD NOT OPEN MAP");
	#endif
  	fread(&NumStrats,sizeof(int),1,fp);
  	/*dLog("number of strats %d\n",NumStrats); */


  	GameMap=(STRATMAP*)SHeapAlloc (MissionHeap,sizeof(STRATMAP)*NumStrats);

		
  	for (i=0;i<NumStrats;i++)
  	{
		GameMap[i].strat = NULL;
		GameMap[i].invalid = 0;
  		fread(&GameMap[i].StratEnt,sizeof(short),1,fp);
		fread(&GameMap[i].pos,sizeof(Vector3),1,fp);
		fread(&GameMap[i].modelid,sizeof(short),1,fp);
		fread(&GameMap[i].rot,sizeof(Vector3),1,fp);
		fread(&GameMap[i].scale,sizeof(Vector3),1,fp);
		fread(&GameMap[i].parent,sizeof(short),1,fp);
		fread(&GameMap[i].flag,sizeof(int),1,fp);
		fread(&GameMap[i].flag2,sizeof(int),1,fp);

		fread(&GameMap[i].way,sizeof(short),1,fp);
		fread(&GameMap[i].startnode,sizeof(unsigned char),1,fp);
		fread(&GameMap[i].startpatrol,sizeof(unsigned char),1,fp);

		fread(&GameMap[i].trig,sizeof(short),1,fp);
		dLog("to be triggered %d\n",GameMap[i].trig);

		/*READ IN EVENT THE OBJECT IS ATTACHED */
		fread(&GameMap[i].attachedevent,sizeof(short),1,fp);

		fread(&GameMap[i].eventdepend,sizeof(short),1,fp);
		for (loop=0;loop<5;loop++)
			fread(&GameMap[i].activation[loop],sizeof(short),1,fp);

		fread(&GameMap[i].intparams,sizeof(unsigned char),1,fp);

		GameMap[i].iparamlist = (int *)SHeapAlloc (MissionHeap,sizeof(int) * GameMap[i].intparams);

		for (test=0;test<GameMap[i].intparams;test++)
			fread(&GameMap[i].iparamlist[test],sizeof(int),1,fp);

		fread(&GameMap[i].floatparams,sizeof(unsigned char),1,fp);

		GameMap[i].fparamlist = (float *)SHeapAlloc (MissionHeap,sizeof(float) * GameMap[i].floatparams);

		for (test=0;test<GameMap[i].floatparams;test++)
			fread(&GameMap[i].fparamlist[test],sizeof(float),1,fp);

		/*READ MAP STRAT DELAY IN */
		fread(&GameMap[i].delay,sizeof(short),1,fp);

	}

	/*READ IN THE LEVEL SPECIFIC FLAGS IF SET */
	fread(&LevelFlags,sizeof(unsigned char),1,fp);

	LevelSettings = NULL;
	if  (LevelFlags)
	{
		LevelSettings =  (LEVELSETTINGS *)SHeapAlloc (MissionHeap, sizeof (LEVELSETTINGS));

 	   /*	fread(LevelSettings, sizeof(LEVELSETTINGS),1,fp); */
   	  	fread(&LevelSettings->ambientred, sizeof(unsigned char),1,fp);
   		fread(&LevelSettings->ambientgreen, sizeof(unsigned char),1,fp);
   		fread(&LevelSettings->ambientblue, sizeof(unsigned char),1,fp);
   		fread(&LevelSettings->scapeintensity, sizeof(float),1,fp);
   		fread(&LevelSettings->stratintensity, sizeof(float),1,fp);
   		fread(&LevelSettings->fogred, sizeof(unsigned char),1,fp);
   		fread(&LevelSettings->foggreen, sizeof(unsigned char),1,fp);
   		fread(&LevelSettings->fogblue, sizeof(unsigned char),1,fp);
   		fread(&LevelSettings->fognear, sizeof(float),1,fp);
   		fread(&LevelSettings->fogfar, sizeof(float),1,fp);
   		fread(&LevelSettings->fov, sizeof(float),1,fp);
   		fread(&LevelSettings->farz, sizeof(float),1,fp);
   		fread(&tempf, sizeof(float),1,fp);
   		fread(&tempf, sizeof(float),1,fp);
   		fread(&tempf, sizeof(float),1,fp);
		LevelSettings->fov = 0.7f;
		
   		SetLevelVars(LevelSettings);
			
	}
	else
	/*else setup with default values */
		SetLevelVars(NULL);

	/*NOW READ IN THE TRIGGER POINT */
	fread(&NumTrigs,sizeof(short),1,fp);
	dLog("map contains %d triggers \n",NumTrigs);
	if (NumTrigs)
		MapTrigs =  (MAPTRIGGER *)SHeapAlloc (MissionHeap, sizeof (MAPTRIGGER) * NumTrigs);

	StartTrig = MapTrigs;
	for (i=0;i<NumTrigs;i++)
	{
		fread(&MapTrigs->nument,sizeof(short),1,fp);
		MapTrigs->entries = (short*)SHeapAlloc (MissionHeap, sizeof(short) * MapTrigs->nument);

		MapTrigs->gen = (unsigned char*)SHeapAlloc (MissionHeap, sizeof(unsigned char) * MapTrigs->nument);

		/*READ IN THE FLAG */
		fread(&MapTrigs->flag,sizeof(short),1,fp);

		dLog("****trig %d has a flag of %d\n",i,MapTrigs->flag);

		/*TYPE TIMER READ IN THE COUNTER VALUE */
		if (MapTrigs->flag & TIMERTRIG)
		{
			fread(&MapTrigs->counter,sizeof(short),1,fp);
			dLog("****trig %d has a counter of %d\n",i,MapTrigs->counter);
		}

		/*READ IN THE RADIUS			 */
		fread(&MapTrigs->radius,sizeof(float),1,fp);
		/*READ IN THE POSITION			 */
		fread(&MapTrigs->pos,sizeof(Vector3),1,fp);

		dLog("trig %d has %d attached \n",i,MapTrigs->nument);
		dLog("trig %d has radius of %f \n",i,MapTrigs->radius);
			
		for (loop=0;loop<MapTrigs->nument;loop++)
			MapTrigs->gen[loop]=0;

		fread(MapTrigs->entries,sizeof(short),MapTrigs->nument,fp);
		MapTrigs->triggered=0;
		MapTrigs++;
	}
	MapTrigs = StartTrig;


	/*NOW READ IN THE EVENTS */
	fread(&NumEvents,sizeof(short),1,fp);
	dLog("map contains %d events \n",NumEvents);

	if (NumEvents)
		MapEvents =  (MAPEVENT *)SHeapAlloc (MissionHeap, sizeof (MAPEVENT) * NumEvents);
	StartEvent = MapEvents;
	for (i=0;i<NumEvents;i++)
	{
		fread(&MapEvents->flag,sizeof(unsigned char),1,fp);
		fread(&MapEvents->timer,sizeof(short),1,fp);
		MapEvents->starttimer = MapEvents->timer;

		fread(&MapEvents->nument,sizeof(short),1,fp);
		MapEvents->startnument = MapEvents->nument;

		/*read number of trig children */
		fread(&MapEvents->numchild,sizeof(unsigned char),1,fp);
		MapEvents->childlist = (short*)SHeapAlloc (MissionHeap, sizeof(short) * MapEvents->numchild);
		fread(MapEvents->childlist,sizeof(short),MapEvents->numchild,fp);

  	    MapEvents->eventstatus = (unsigned char*)SHeapAlloc (MissionHeap, sizeof(unsigned char) * MapEvents->nument);
	  	for (loop=0;loop<MapEvents->nument;loop++)
	  		MapEvents->eventstatus[loop]=WAITING;

	  	MapEvents->entries = (short*)SHeapAlloc (MissionHeap, sizeof(short) * MapEvents->nument);
		fread(MapEvents->entries,sizeof(short),MapEvents->nument,fp);
		MapEvents->count =0;
		MapEvents->deleted =0;
		MapEvents->triggered =0;

	  //	MapEvents->timer = 300;

	  	MapEvents->eventstrat = (STRAT**)SHeapAlloc (MissionHeap, sizeof(STRAT *) * MapEvents->nument);

		MapEvents++;
	}	
	MapEvents = StartEvent;

	/*NOW READ IN THE ACTIVATION POINTS */
	fread(&NumActs,sizeof(short),1,fp);
	dLog("map contains %d strat activation points \n",NumActs);
	if (NumActs)
	{
		MapActs =  (MAPACTIVATION *)SHeapAlloc (MissionHeap, sizeof (MAPACTIVATION) * NumActs);
		fread(MapActs,sizeof(MAPACTIVATION),NumActs,fp);
	}

	/*NOW READ IN THE STRAT PATHS */

	fread(&NumPaths,sizeof(short),1,fp);
	dLog("map contains %d paths \n",NumPaths);
	if (NumPaths)
		MapPaths =  (WAYPATH *)SHeapAlloc (MissionHeap, sizeof (WAYPATH) * NumPaths);

	for (i=0;i<NumPaths;i++)
	{

		fread(&(MapPaths+i)->waytype,sizeof(short),1,fp);
		if ((MapPaths+i)->waytype == PATH)
		{
			(MapPaths+i)->net = NULL;
			fread(&NumWayPoints,sizeof(short),1,fp);

			(MapPaths+i)->numwaypoints = NumWayPoints;
			dLog("path %d paths has %d waypoints\n",i,NumWayPoints);
			(MapPaths+i)->waypointlist =  (Vector3 *)SHeapAlloc (MissionHeap, sizeof (Vector3) * NumWayPoints);

			fread((MapPaths+i)->waypointlist,sizeof(Vector3),NumWayPoints,fp);

			fread(&(MapPaths+i)->area,sizeof(short),1,fp);
			dLog("path %d is attached to %d area\n",i,(MapPaths+i)->area);
		}
		else
		{
			dLog("reading network info\n");

			fread(&NumWayPoints,sizeof(short),1,fp);
			(MapPaths+i)->numwaypoints = NumWayPoints;
			dLog("path %d paths has %d waypoints\n",i,NumWayPoints);
			(MapPaths+i)->waypointlist =  (Vector3 *)SHeapAlloc (MissionHeap, sizeof (Vector3) * NumWayPoints);
			fread((MapPaths+i)->waypointlist,sizeof(Vector3),NumWayPoints,fp);

			(MapPaths+i)->net =  (NET *)SHeapAlloc (MissionHeap, sizeof (NET));
			(MapPaths+i)->net->waybusy =  (char *)SHeapAlloc (MissionHeap, sizeof (char) * NumWayPoints);
			for (way=0;way<NumWayPoints;way++)
				(MapPaths+i)->net->waybusy[way]=0;

			(MapPaths+i)->net->connectors =  (int *)SHeapAlloc (MissionHeap, sizeof (int) * NumWayPoints);
			fread((MapPaths+i)->net->connectors,sizeof(int),NumWayPoints,fp);

			fread(&NumPatrolRoutes,sizeof(unsigned char), 1, fp);
			dLog("number of patrol routes %d\n",NumPatrolRoutes);

			(MapPaths+i)->net->patrol = (PATROL *)SHeapAlloc (MissionHeap, sizeof (PATROL) * NumPatrolRoutes);

			for (patrol=0;patrol<NumPatrolRoutes;patrol++)
			{
				fread(&NumPathPoints,sizeof(unsigned char),1,fp);
				dLog("number of path points %d\n",NumPathPoints);
				(MapPaths+i)->net->patrol[patrol].numpathpoints = NumPathPoints;
				(MapPaths+i)->net->patrol[patrol].initialpath = 
				(unsigned char *)SHeapAlloc (MissionHeap, sizeof (unsigned char) * NumPathPoints);
				fread((MapPaths+i)->net->patrol[patrol].initialpath,sizeof(unsigned char),NumPathPoints,fp);

			}
			fread(&(MapPaths+i)->area,sizeof(short),1,fp);
			dLog("path %d is attached to %d area\n",i,(MapPaths+i)->area);
		}
				
	  }

	 /*Now read in the MapAreas, if any */
	 fread(&NumAreas,sizeof(short),1,fp);
	 if (NumAreas)	
	 	MapAreas = (MAPAREA*)SHeapAlloc(MissionHeap, sizeof (MAPAREA) * NumAreas);

	 dLog("map has %d\n",NumAreas);
	 for (i=0;i<NumAreas;i++)
	 {
	 	/*READ IN THE SURROUNDING BBOX CIRCLE */
	 	fread(&(MapAreas+i)->Surround,sizeof(CIRCLEBOX),1,fp);
	 	dLog("surround rad %f\n",(MapAreas+i)->Surround.radius);

	 	/*READ THE EXT SPLINE SHAPE INFO */
	 	fread(&(MapAreas+i)->numextsplineareas,sizeof(short),1,fp);
	 	dLog("number of shape subs %d\n",(MapAreas+i)->numextsplineareas);
	 	if ((MapAreas +i)->numextsplineareas)
	 	{
	 		(MapAreas+i)->extsplinebbox =  (SPLINEBOX *)SHeapAlloc (MissionHeap, sizeof (SPLINEBOX) * (MapAreas+i)->numextsplineareas);
	 			
	 		for (entry=0;entry<(MapAreas+i)->numextsplineareas;entry++)
	 		{
	 			fread(&(MapAreas+i)->extsplinebbox[entry].numpoints,sizeof(short),1,fp);
	 			dLog("number of points %d\n",(MapAreas+i)->extsplinebbox[entry].numpoints);
	 			(MapAreas+i)->extsplinebbox[entry].pointlist =  (Vector2 *)SHeapAlloc (MissionHeap, sizeof (Vector2) * (MapAreas+i)->extsplinebbox[entry].numpoints);
	 			fread((MapAreas+i)->extsplinebbox[entry].pointlist,sizeof(Vector2),(MapAreas+i)->extsplinebbox[entry].numpoints,fp);
	 		}
						
	 	}

	 	/*READ THE APPROPRIATE SUB CIRCLE AREA INFO */
	 	fread(&(MapAreas+i)->numcircleareas,sizeof(short),1,fp);
	 	dLog("number of circle subs %d\n",(MapAreas+i)->numcircleareas);
	 	if ((MapAreas +i)->numcircleareas)
	 	{
	 		(MapAreas+i)->circlebbox =  (CIRCLEBOX *)SHeapAlloc (MissionHeap, sizeof (CIRCLEBOX) * (MapAreas+i)->numcircleareas);
	 		fread((MapAreas+i)->circlebbox,sizeof(CIRCLEBOX),(MapAreas+i)->numcircleareas,fp);
	 	}

	 	/*READ THE APPROPRIATE SUB BOX AREA INFO */
	 	fread(&(MapAreas+i)->numboxareas,sizeof(short),1,fp);
	 	dLog("number of box subs %d\n",(MapAreas+i)->numboxareas);
	 	if ((MapAreas +i)->numboxareas)
	 	{
	 		(MapAreas+i)->boxbbox =  (BOXBOX *)SHeapAlloc (MissionHeap, sizeof (BOXBOX) * (MapAreas+i)->numboxareas);
	 		fread((MapAreas+i)->boxbbox,sizeof(BOXBOX),(MapAreas+i)->numboxareas,fp);
	 	}

	 	/*READ THE SPLINE SHAPE INFO */
	 	fread(&(MapAreas+i)->numsplineareas,sizeof(short),1,fp);
	 	dLog("number of shape subs %d\n",(MapAreas+i)->numsplineareas);
	 	if ((MapAreas +i)->numsplineareas)
	 	{
	 		(MapAreas+i)->splinebbox =  (SPLINEBOX *)SHeapAlloc (MissionHeap, sizeof (SPLINEBOX) * (MapAreas+i)->numsplineareas);
	 			
	 		for (entry=0;entry<(MapAreas+i)->numsplineareas;entry++)
	 		{
	 			fread(&(MapAreas+i)->splinebbox[entry].numpoints,sizeof(short),1,fp);
	 			dLog("number of points %d\n",(MapAreas+i)->splinebbox[entry].numpoints);
	 			(MapAreas+i)->splinebbox[entry].pointlist =  (Vector2 *)SHeapAlloc (MissionHeap, sizeof (Vector2) * (MapAreas+i)->splinebbox[entry].numpoints);
	 			fread((MapAreas+i)->splinebbox[entry].pointlist,sizeof(Vector2),(MapAreas+i)->splinebbox[entry].numpoints,fp);
	 		}
						
	 	}

	 	/*READ THE LINE SHAPE INFO */
	 	fread(&(MapAreas+i)->numlinesightareas,sizeof(short),1,fp);
	 	dLog("number of shape subs %d\n",(MapAreas+i)->numlinesightareas);
	 	if ((MapAreas +i)->numlinesightareas)
	 	{
	 		(MapAreas+i)->linesightbbox =  (SPLINEBOX *)SHeapAlloc (MissionHeap, sizeof (SPLINEBOX) * (MapAreas+i)->numlinesightareas);
						
	 		for (entry=0;entry<(MapAreas+i)->numlinesightareas;entry++)
	 		{
	 			fread(&(MapAreas+i)->linesightbbox[entry].numpoints,sizeof(short),1,fp);
	 			dLog("number of points %d\n",(MapAreas+i)->linesightbbox[entry].numpoints);
	 			(MapAreas+i)->linesightbbox[entry].pointlist =  (Vector2 *)SHeapAlloc (MissionHeap, sizeof (Vector2) * (MapAreas+i)->linesightbbox[entry].numpoints);
	 			fread((MapAreas+i)->linesightbbox[entry].pointlist,sizeof(Vector2),(MapAreas+i)->linesightbbox[entry].numpoints,fp);
	 		}
						
	 	}

	 	/*READ THE CAMPPOINT INFO */
	 	fread(&(MapAreas+i)->numcamppoints,sizeof(short),1,fp);
	 	if (MapAreas[i].numcamppoints)
	 	{
	 		MapAreas[i].camppoints =  (CAMPPOINT *)SHeapAlloc (MissionHeap, sizeof (CAMPPOINT) * (MapAreas+i)->numcamppoints);
						
	 		for (entry=0;entry<(MapAreas+i)->numcamppoints;entry++)
			{
	 			fread(&(MapAreas+i)->camppoints[entry].pos,sizeof(Vector2),1,fp);
				(MapAreas+i)->camppoints[entry].busy = 0;			
			}
		}

	 }

	 /*Now read in the MapLights, if any */
	 fread(&NumLights,sizeof(short),1,fp);
	 dLog("map contains %d lights \n",NumLights);
	 for (light = 0; light < NumLights; ++light)
	 {
	 	char type;
	 	Point3 pos;
	 	Vector3 dir;
	 	unsigned char col[3];
	 	float Near, Far, min, max;
	 	Light *l;
	 	LightingValue lv;

	 	fread (&type, 1, 1, fp);
	 	fread (&pos, sizeof (float), 3, fp);
	 	fread (col, sizeof (char), 3, fp);
	 	fread (&Near, sizeof (float), 1, fp);
	 	fread (&Far, sizeof (float), 1, fp);

	 	lv.r = col[0] * (1.f / 255.f);
	 	lv.g = col[1] * (1.f / 255.f);
	 	lv.b = col[2] * (1.f / 255.f);

	 	switch (type)
	 	{
	 		case 0: /* Omnilight */
	 			/*l = newOmniLight (&pos, &lv, Near, Far, NULL);*/
	 			/*l->flags |= LIGHT_STATIC;*/
	 			break;
	 		case 1: /* Spotlight */
	 			fread (&min, sizeof (float), 1, fp);
	 			fread (&max, sizeof (float), 1, fp);
	 			fread (&dir, sizeof (float), 3, fp);
	 			/*l = newSpotLight (&pos, &lv, Near, Far, dir, min, max, NULL);*/
	 			/*l->flags |= LIGHT_STATIC;*/
	 			break;
	 		default:
	 			dAssert (FALSE, "Oh no!");
	 			break;
	 	}
	 }

	 /*SEE IF WE HAVE ANY CAMERA MODIFIERS TO LOAD */

 	fread(&NumCamMods,sizeof(short),1,fp);

	if (NumCamMods)
	{
		CamModArray	 = (CAMMODIFIER*)SHeapAlloc(MissionHeap, sizeof (CAMMODIFIER) * NumCamMods);


		
		for (i=0;i<NumCamMods;i++)
		{
			fread(&CamModArray[i].amount,sizeof(float),1,fp);

			fread(&CamModArray[i].speed,sizeof(float),1,fp);

/*			fread(&CamModArray[i].upoff,sizeof(Point3),1,fp);

			fread(&CamModArray[i].downoff,sizeof(Point3),1,fp);	   */

		}


	}



 	fread(&NumAnims,sizeof(int),1,fp);
	dLog("NUM ANIMS %d\n",NumAnims);

	#if ALLRESIDENT
		GameAnims =  (STRATANIMMATT *)SHeapAlloc (MissionHeap, sizeof (STRATANIMMATT) * MAXANIMS);
	#else
		GameAnims =  (STRATANIMMATT *)SHeapAlloc (MissionHeap, sizeof (STRATANIMMATT) * NumAnims);
	#endif

	for (i=0;i<NumAnims;i++)
	{
		fread(&MapEntry,sizeof(int),1,fp);
		fread(&len,sizeof(int),1,fp);
		fread(objname,sizeof(char),len,fp);
		
		#if ALLRESIDENT
			 
			fread(&entry,sizeof(int),1,fp);
			/*we need to save entry for later on */
			TempAnims[i]=entry;

		   	GameAnims[entry].objent = MapEntry;
		   	GameAnims[entry].name=(char*)SHeapAlloc (MissionHeap,sizeof(char)*len);
		   	strncpy(GameAnims[entry].name,objname,len);
			 /*	dLog("looking for anim %s in entry %d\n",GameAnims[entry].name,GameAnims[entry].objent); */
		   	GameAnims[entry].animent=0;
			/*
		   	GameAnims[entry].animent=rFindAnim(GameMods[GameAnims[entry].objent],GameAnims[entry].name); 
			*/
		#else
	
		 	GameAnims[i].objent = MapEntry;
			GameAnims[i].name=(char*)SHeapAlloc (MissionHeap,sizeof(char)*len);
			strncpy(GameAnims[i].name,objname,len);
			GameAnims[i].animent=0;
			/*
		   	GameAnims[i].animent=rFindAnim(&GameObjects[GameAnims[i].objent],GameAnims[i].name); 
			*/
		#endif

			dLog("anim entry %d\n",GameAnims[i].animent);
			dLog("model entry %d\n",GameAnims[i].objent);
	}
  
	 
	 /*OPEN UP THE LIST OF MODELS TO BE USED IN THE GAME */
	
	/* Ensure the end of the WAD chunk has been found */
	sSkip (fp);

	fread(&NumMod,sizeof(int),1,fp);

	dLog("NUM MODS %d\n",NumMod);

	#if ALLRESIDENT
		GameObjects =  (PNode *)SHeapAlloc (MissionHeap, sizeof (PNode) * NumMod);
		for (i=0;i<MAXMODS;i++)
			GameMods[i]=NULL;
		//GameHeads =  (PNode *)SHeapAlloc (MissionHeap, sizeof (PNode) * MAXCHARACTERS);
		//for (i=0;i<MAXCHARACTERS;i++)
		//	GameHeads[i]=NULL;
	#else
		GameObjects =  (PNode *)SHeapAlloc (MissionHeap, sizeof (PNode) * NumMod);
	#endif


	for (i=0;i<NumMod;i++)
	{
		#if ALLRESIDENT
			(void)PNodeLoad (&GameObjects[i], fp, (Allocator *)SHeapAlloc, MissionHeap);

			fread(&entry,sizeof(int),1,fp);
			GameMods[entry] = &GameObjects[i];

			dLog("entry %d name %s address %x\n",entry,GameObjects[i].name,GameMods[entry]);
			
		#else
			(void)PNodeLoad (&GameObjects[i], ModelFile, SHeapAlloc, MissionHeap);
		#endif

	}
	

	/*RESOLVE ANIM DETAILS */
	for (i=0;i<NumAnims;i++)
	{
		
		#if ALLRESIDENT
			entry = TempAnims[i];
		   	GameAnims[entry].animent=rFindAnim(GameMods[GameAnims[entry].objent],GameAnims[entry].name);
	   	#else
			GameAnims[i].animent=rFindAnim(&GameObjects[GameAnims[i].objent],GameAnims[i].name);
	  	#endif


	}

	/*
 	fread(&NumAnims,sizeof(int),1,fp);
	dLog("NUM ANIMS %d\n",NumAnims);

	#if ALLRESIDENT
		GameAnims =  (STRATANIMMATT *)SHeapAlloc (MissionHeap, sizeof (STRATANIMMATT) * MAXANIMS);
	#else
		GameAnims =  (STRATANIMMATT *)SHeapAlloc (MissionHeap, sizeof (STRATANIMMATT) * NumAnims);
	#endif

	for (i=0;i<NumAnims;i++)
	{
		fread(&MapEntry,sizeof(int),1,fp);
		fread(&len,sizeof(int),1,fp);
		fread(objname,sizeof(char),len,fp);
		
		#if ALLRESIDENT
				
			fread(&entry,sizeof(int),1,fp);
	
		   	GameAnims[entry].objent = MapEntry;
		   	GameAnims[entry].name=(char*)SHeapAlloc (MissionHeap,sizeof(char)*len);
		   	strncpy(GameAnims[entry].name,objname,len);
			 /*	dLog("looking for anim %s in entry %d\n",GameAnims[entry].name,GameAnims[entry].objent); 
		   	GameAnims[entry].animent=0;
			GameAnims[entry].animent=rFindAnim(GameMods[GameAnims[entry].objent],GameAnims[entry].name);
	   	#else
	
		 	GameAnims[entry].objent = MapEntry;
			GameAnims[i].name=(char*)SHeapAlloc (MissionHeap,sizeof(char)*len);
			strncpy(GameAnims[i].name,objname,len);
			GameAnims[i].animent=0;

			GameAnims[i].animent=rFindAnim(&GameObjects[GameAnims[i].objent],GameAnims[i].name);
	  	#endif

			dLog("anim entry %d\n",GameAnims[i].animent);
			dLog("model entry %d\n",GameAnims[i].objent);
	}
	*/

 
	/*DO WE HAVE A DOME TO LOAD ?*/
	sRead(&nBytes, 4, fp);
	if (nBytes)
	{
		LoadDome (fp);
	}
	sSkip (fp);



	/*	ARE WE IN DEMO MODE ? IF SO, WE'LL HAVE TO TRY AND READ IN THE CONTROL
		AND CAMERA SCRIPTS */

	if (InputMode == DEMOREAD)
	{
	
	
		/*IS THERE MORE STUFF AT THE END ? LIKE THE .LOG FILE FOR EXAMPLE ? 
		  EH ! EH ! ANSWER ME THAT MICROSOFT */
		/* Well, I'll answer that question...*/
		/*Cheers Bill*/

		sRead(&nBytes, 4, fp);
		if (nBytes)
		{
			InputBuffer = (INPUT*)SHeapAlloc (MissionHeap, nBytes);

			sRead (InputBuffer, nBytes, fp);
		}

		sSkip (fp);
		sRead (&nBytes, 4, fp);
		
		if (nBytes)
		{
				
				/*READ IN THE NUMBER OF COMMANDS*/
				fread(&CamCommands,sizeof(short),1,fp);
				
				CamBuffer=(CAMCOMMAND*)SHeapAlloc (MissionHeap,sizeof(CAMCOMMAND)*CamCommands);
				
				
				fread(CamBuffer,sizeof(CAMCOMMAND),CamCommands,fp);
				CamMode = CAMSCRIPT;
		}
		sSkip (fp);
	}
	else
	{
		sRead (&nBytes, 4, fp);
		if (nBytes)
			sIgnore(nBytes, fp);
		
		sSkip (fp);

		sRead (&nBytes, 4, fp);
		if (nBytes)
			sIgnore(nBytes, fp);
		
		sSkip (fp);
	}


	/*DO WE HAVE A SCRIPT TO LOAD ?*/
	sRead(&nBytes, 4, fp);
	if (nBytes)
	{
	
		
		LoadScript (fp);
	
	
	}
	sSkip (fp);


	/*DO WE HAVE CHARACTER MODELS TO LOAD ?*/
	LoadHeads(fp);

	/*
	sRead(&nBytes, 4, fp);
	if (nBytes)
	{
	
		
		LoadHeads (fp);
	
	
	}
	sSkip (fp);
	*/







	#if !SH4
		/*Ensure the wad is kept open on set 5*/
		fclose(fp);
	#endif


	/*ALL MODELS AND STUFF LOADED SO LET'S GEN THE MAP STRATS */
	for (i=0;i<NumStrats;i++)
	{
	 
		if (GameMap[i].StratEnt == 34)
			loop = 2;

		
		/*ONLY CREATE IF IT'S A NON TRIGGERED STRAT */
	   	/*THAT'S NOT DEPENDENT ON AN EVENT HAPPENING */
		if ((!GameMap[i].trig) && (GameMap[i].eventdepend < 0))
	   		MapStratGen(i,WARM);	
	   	else
	   		GameMap[i].strat = NULL;

	}


	/*BUILD OUR EVENTS LIST */

  /*	StartEvent = MapEvents;
	for (i=0;i<NumEvents;i++)
	{

		for (loop=0;loop<NumStrats;loop++)
		{
			if (GameMap[loop].strat && (GameMap[loop].attachedevent == i))
			{
				MapEvents->eventstrat[MapEvents->count] = GameMap[loop].strat;
				MapEvents->eventstatus[MapEvents->count] = ALIVE;
				MapEvents->count++;

			}

		}
		MapEvents++;
	}
	MapEvents = StartEvent;
	*/

	dLog("****NEXT FREE ADDR %x\n",NextFreeAnim);

	/*NOW SET THE DUMMY PLAYER */
	

	player[currentPlayer].Player = &DummStrat;
	StratReset(player[currentPlayer].Player);
	PlayerLives = 1;
//	red = 0x10;	
	GameOverFlag=0;
	CamNo = 0;
	player[currentPlayer].CameraStrat = NULL;

	for (i=0;i<MAXCAM;i++)
	   CamTracks[i]=NULL;

	PlayerLifeLost = 0;

}
#else



void MapInit(char *file)
{
	FILE *fp;
	FILE *ObjectFile;
	char testc[1];
	char MapFile[128];
	char ModelFile[128];
	char outstr[128];
	char entryn[8];
	char objname[64];
	char ascnum[8];
	short NumPaths,NumAreas,NumWayPoints,patrol;
	int loop,i,entry,way,test;
	int c;
	unsigned char NumPathPoints,NumPatrolRoutes, LevelFlags;
	MAPTRIGGER *StartTrig;
	MAPEVENT *StartEvent;

	ResetRandom();

	dLog("STRAT SIZE %d\n",sizeof(STRAT));
/*	strncpy(file,fileo,strlen(fileo)-1); */
	
	sprintf(ModelFile,"%s%s%s",MAPDIR,file,".MOD");
	
	
	
	/*OPEN UP THE LIST OF MODELS TO BE USED IN THE GAME */
	
	fp=fopen(ModelFile, "ra");
	fscanf(fp, "%s", ascnum);

	NumMod=atoi(ascnum);

	dLog("NUM MODS %d\n",NumMod);

	#if ALLRESIDENT
		GameObjects =  (PNode *)SHeapAlloc (MissionHeap, sizeof (PNode) * NumMod);
		for (i=0;i<MAXMODS;i++)
			GameMods[i]=NULL;
	#else
		GameObjects =  (PNode *)SHeapAlloc (MissionHeap, sizeof (PNode) * NumMod);
	#endif


	for (i=0;i<NumMod;i++)
	{

		while (isspace (c=fgetc(fp)));
		ungetc (c, fp);

		fscanf(fp, "%s\n", objname);
	


		if (!strnicmp (objname, "P:\\GAME\\OBJECTS\\", 16)) /*{ */
			sprintf (outstr, REDDOG_OBJECTS "%s.RDO", &objname[16]);
		/*} else// { */
		else
			sprintf(outstr,REDDOG_OBJECTS "%s.RDO",objname);
		/*} */
		dLog("loading file %s %d\n",outstr,i);

		ObjectFile = fopen(outstr, "rb");
		dAssert(ObjectFile != NULL, "ObjectFile not found\n");
		if (!ObjectFile)
			exit(1);

		#if ALLRESIDENT
			fscanf(fp, "%s\n", entryn);
			entry = atoi(entryn);
		/*	fread(&entry,sizeof(int),1,fp); */
			
			/*			fread(&entry,sizeof(int),1,fp); */

			(void)PNodeLoad (&GameObjects[i], ObjectFile, SHeapAlloc, MissionHeap);
			GameMods[entry] = &GameObjects[i];

			dLog("entry %d name %s address %x\n",entry,GameObjects[i].name,GameMods[entry]);
			
/*			(void)PNodeLoad (&GameObjects[entry], ObjectFile, SHeapAlloc, MissionHeap); */
		#else
			(void)PNodeLoad (&GameObjects[i], ObjectFile, SHeapAlloc, MissionHeap);
		#endif

		fclose (ObjectFile);

	}

	fclose(fp);

	/*OPEN UP THE LIST OF ANIMS TO BE USED IN THE GAME */
	sprintf(ModelFile,"%s%s%s",MAPDIR,file,".ANI");
	

	fp=fopen(ModelFile, "ra");
	if (fp)
	{
		dLog("opening anims \n");
		
		fscanf(fp, "%s", ascnum);

		NumAnims=atoi(ascnum);
		dLog("NUM ANIMS %d\n",NumAnims);

		#if ALLRESIDENT
			GameAnims =  (STRATANIMMATT *)SHeapAlloc (MissionHeap, sizeof (STRATANIMMATT) * MAXANIMS);
		#else
			GameAnims =  (STRATANIMMATT *)SHeapAlloc (MissionHeap, sizeof (STRATANIMMATT) * NumAnims);
		#endif

		for (i=0;i<NumAnims;i++)
		{
			int c;

			while (isspace (c=fgetc(fp)));
			ungetc (c, fp);

			fscanf(fp, "%s\n", ascnum);
			fscanf(fp, "%s\n", objname);
			#if ALLRESIDENT
				
				fscanf(fp, "%s\n", entryn);
				entry = atoi(entryn);
	
				GameAnims[entry].objent = atoi(ascnum);
				GameAnims[entry].name=(char*)SHeapAlloc (MissionHeap,sizeof(char)*(strlen(objname)+1));
				strcpy(GameAnims[entry].name,objname);
			 /*	dLog("looking for anim %s in entry %d\n",GameAnims[entry].name,GameAnims[entry].objent); */
				GameAnims[entry].animent=0;
				/*GameAnims[entry].animent=rFindAnim(GameMods[GameAnims[entry].objent],GameAnims[entry].name);*/
			#else
	
				GameAnims[i].objent = atoi(ascnum);
				GameAnims[i].name=(char*)SHeapAlloc (MissionHeap,sizeof(char)*(strlen(objname)+1));
				strcpy(GameAnims[i].name,objname);
				GameAnims[i].animent=0;

				/*GameAnims[i].animent=rFindAnim(&GameObjects[GameAnims[i].objent],GameAnims[i].name);*/
			#endif

			dLog("anim entry %d\n",GameAnims[i].animent);
			/*dLog("looking for anim %s\n",GameAnims[i].name); */
			dLog("model entry %d\n",GameAnims[i].objent);


		}
		fclose(fp);
	}
	
	
	/*NOW THE MAP ITSELF **** MUST MAKE THIS DYNAMIC ME BEAUTIES, BASED ON NUMSTRATS READ IN */
	
	/*for (i=0;i<32;i++) */
	 /*	GameMap[i].StratEnt=0; */
	
	sprintf(MapFile,"%s%s%s",MAPDIR,file,".MAP");
	if (fp=fopen(MapFile, "rb"))
	{

		fread(&NumStrats,sizeof(int),1,fp);
		/*dLog("number of strats %d\n",NumStrats); */

		GameMap=(STRATMAP*)SHeapAlloc (MissionHeap,sizeof(STRATMAP)*NumStrats);

		
		for (i=0;i<NumStrats;i++)
		{
			fread(&GameMap[i].StratEnt,sizeof(short),1,fp);
			/*dLog("entry %d\n",GameMap[i].StratEnt); */

			fread(&GameMap[i].pos,sizeof(Vector3),1,fp);
			fread(&GameMap[i].modelid,sizeof(short),1,fp);
			fread(&GameMap[i].rot,sizeof(Vector3),1,fp);
			fread(&GameMap[i].scale,sizeof(Vector3),1,fp);
			fread(&GameMap[i].flag,sizeof(int),1,fp);
			fread(&GameMap[i].flag2,sizeof(int),1,fp);

		/*   dLog("smart is %d\n",STRAT2_SMARTAMPPICKUP);		 */

		/*   dLog("power is %d\n",STRAT2_POWERAMPPICKUP);		 */

		/*   dLog("BLAST is %d\n",STRAT2_BLASTAMPPICKUP);		 */


			/*dLog("flag %d\n",GameMap[i].flag); */
			fread(&GameMap[i].way,sizeof(short),1,fp);
			fread(&GameMap[i].startnode,sizeof(unsigned char),1,fp);
			fread(&GameMap[i].startpatrol,sizeof(unsigned char),1,fp);
	
			
			
			/*dLog("way %d\n",GameMap[i].way); */
			fread(&GameMap[i].trig,sizeof(short),1,fp);
			dLog("to be triggered %d\n",GameMap[i].trig);

			/*READ IN EVENT THE OBJECT IS ATTACHED */
			fread(&GameMap[i].attachedevent,sizeof(short),1,fp);


			fread(&GameMap[i].eventdepend,sizeof(short),1,fp);
 			for (loop=0;loop<5;loop++)
				fread(&GameMap[i].activation[loop],sizeof(short),1,fp);

	/*		fread(&GameMap[i].armour,sizeof(float),1,fp); */
	/*		fread(&GameMap[i].health,sizeof(float),1,fp); */
			
			fread(&GameMap[i].intparams,sizeof(unsigned char),1,fp);

		  /*	dLog("NUM INTS %d\n",GameMap[i].intparams); */

			GameMap[i].iparamlist = (int *)SHeapAlloc (MissionHeap,sizeof(int) * GameMap[i].intparams);

			for (test=0;test<GameMap[i].intparams;test++)
				fread(&GameMap[i].iparamlist[test],sizeof(int),1,fp);

		  /*	for (test=0;test<GameMap[i].intparams;test++) */
		  /*		dLog("READ INTS %d\n",GameMap[i].iparamlist[test]); */

			fread(&GameMap[i].floatparams,sizeof(unsigned char),1,fp);

		  /*	dLog("NUM FLOATS %d\n",GameMap[i].floatparams); */

			GameMap[i].fparamlist = (float *)SHeapAlloc (MissionHeap,sizeof(float) * GameMap[i].floatparams);

			for (test=0;test<GameMap[i].floatparams;test++)
				fread(&GameMap[i].fparamlist[test],sizeof(float),1,fp);

		   /*	fread(&GameMap[i].fparamlist,sizeof(float),GameMap[i].floatparams,fp); */

		  /*	for (test=0;test<GameMap[i].floatparams;test++) */
		  /*		dLog("READ FLOATS %f\n",GameMap[i].fparamlist[test]); */
	

			/*READ MAP STRAT DELAY IN */
			fread(&GameMap[i].delay,sizeof(short),1,fp);

			
			/*ONLY CREATE IF IT'S A NON TRIGGERED STRAT */
			/*THAT'S NOT DEPENDENT ON AN EVENT HAPPENING */
			if ((!GameMap[i].trig) && (GameMap[i].eventdepend < 0))
				MapStratGen(i,COLD);	
			else
				GameMap[i].strat = NULL;
		}

		/*READ IN THE LEVEL SPECIFIC FLAGS IF SET */
		fread(&LevelFlags,sizeof(unsigned char),1,fp);

		LevelSettings = NULL;
		if (LevelFlags)
		{
			LevelSettings =  (LEVELSETTINGS *)SHeapAlloc (MissionHeap, sizeof (LEVELSETTINGS));


/*			fread(&LevelSettings, sizeof(LEVELSETTINGS),1,fp); */
			fread(&LevelSettings->ambientred, sizeof(unsigned char),1,fp);
			fread(&LevelSettings->ambientgreen, sizeof(unsigned char),1,fp);
			fread(&LevelSettings->ambientblue, sizeof(unsigned char),1,fp);
			fread(&LevelSettings->scapeintensity, sizeof(float),1,fp);
			fread(&LevelSettings->stratintensity, sizeof(float),1,fp);
			fread(&LevelSettings->fogred, sizeof(unsigned char),1,fp);
			fread(&LevelSettings->foggreen, sizeof(unsigned char),1,fp);
			fread(&LevelSettings->fogblue, sizeof(unsigned char),1,fp);
			fread(&LevelSettings->fognear, sizeof(float),1,fp);
			fread(&LevelSettings->fogfar, sizeof(float),1,fp);
			fread(&LevelSettings->fov, sizeof(float),1,fp);
			fread(&LevelSettings->farz, sizeof(float),1,fp);
			fread(&LevelSettings->camHeight, sizeof(float),1,fp);
			fread(&LevelSettings->camDist, sizeof(float),1,fp);
			fread(&LevelSettings->camlookHeight, sizeof(float),1,fp);
			LevelSettings->camlookHeight = 1.0f;
			LevelSettings->camHeight = 1.0f;
			LevelSettings->camDist =-6.0f;
			LevelSettings->fov = 0.7f;


			SetLevelVars(LevelSettings);

			
		}
		else
			SetLevelVars(NULL);

		/*else setup with default values */

		/*NOW READ IN THE TRIGGER POINT */
		fread(&NumTrigs,sizeof(short),1,fp);
		dLog("map contains %d triggers \n",NumTrigs);
		if (NumTrigs)
			MapTrigs =  (MAPTRIGGER *)SHeapAlloc (MissionHeap, sizeof (MAPTRIGGER) * NumTrigs);

		StartTrig = MapTrigs;
		for (i=0;i<NumTrigs;i++)
		{
			fread(&MapTrigs->nument,sizeof(short),1,fp);
			
			MapTrigs->entries = (short*)SHeapAlloc (MissionHeap, sizeof(short) * MapTrigs->nument);
			MapTrigs->gen = (unsigned char*)SHeapAlloc (MissionHeap, sizeof(unsigned char) * MapTrigs->nument);

			/*READ IN THE FLAG */
			fread(&MapTrigs->flag,sizeof(short),1,fp);

			dLog("****trig %d has a flag of %d\n",i,MapTrigs->flag);

			/*TYPE TIMER READ IN THE COUNTER VALUE */
			if (MapTrigs->flag & TIMERTRIG)
			{
				fread(&MapTrigs->counter,sizeof(short),1,fp);
				dLog("****trig %d has a counter of %d\n",i,MapTrigs->counter);
			}

			/*READ IN THE RADIUS			 */
			fread(&MapTrigs->radius,sizeof(float),1,fp);
			/*READ IN THE POSITION			 */
			fread(&MapTrigs->pos,sizeof(Vector3),1,fp);

			dLog("trig %d has %d attached \n",i,MapTrigs->nument);
			dLog("trig %d has radius of %f \n",i,MapTrigs->radius);
			
			fread(MapTrigs->entries,sizeof(short),MapTrigs->nument,fp);

			/*CLEAR THE 'HAVE CREATED' FLAG FOR EACH STRAT ATTACHED TO THE TRIGGER*/
			for (loop=0;loop<MapTrigs->nument;loop++)
				MapTrigs->gen[loop]=0;

			MapTrigs->triggered=0;
			MapTrigs++;
		}
		MapTrigs = StartTrig;


		/*NOW READ IN THE EVENTS */
		fread(&NumEvents,sizeof(short),1,fp);
		dLog("map contains %d events \n",NumEvents);

		if (NumEvents)
			MapEvents =  (MAPEVENT *)SHeapAlloc (MissionHeap, sizeof (MAPEVENT) * NumEvents);
		StartEvent = MapEvents;
		for (i=0;i<NumEvents;i++)
		{

			fread(&MapEvents->nument,sizeof(short),1,fp);
			/*read number of trig children */
			fread(&MapEvents->numchild,sizeof(unsigned char),1,fp);
  			MapEvents->childlist = (short*)SHeapAlloc (MissionHeap, sizeof(short) * MapEvents->numchild);
			fread(MapEvents->childlist,sizeof(short),MapEvents->numchild,fp);

	  	    MapEvents->eventstatus = (unsigned char*)SHeapAlloc (MissionHeap, sizeof(unsigned char) * MapEvents->nument);
		  	for (loop=0;loop<MapEvents->nument;loop++)
		  		MapEvents->eventstatus[loop]=WAITING;

		  	MapEvents->entries = (short*)SHeapAlloc (MissionHeap, sizeof(short) * MapEvents->nument);
			fread(MapEvents->entries,sizeof(short),MapEvents->nument,fp);
			MapEvents->count =0;
			MapEvents->deleted =0;
			MapEvents->triggered =0;

		  	MapEvents->eventstrat = (STRAT**)SHeapAlloc (MissionHeap, sizeof(STRAT *) * MapEvents->nument);

		   /*	for (loop=0;loop<MapEvents->nument;loop++) */
		   /*		MapEvents->eventstrat[loop]=-1; */

			for (loop=0;loop<NumStrats;loop++)
			{
				if (GameMap[loop].strat && (GameMap[loop].attachedevent == i))
				{
					MapEvents->eventstrat[MapEvents->count] = GameMap[loop].strat;
					MapEvents->eventstatus[MapEvents->count] = ALIVE;
					MapEvents->count++;

				}

			}


			MapEvents++;
		}	
		MapEvents = StartEvent;





		/*NOW READ IN THE ACTIVATION POINTS */
		fread(&NumActs,sizeof(short),1,fp);
		dLog("map contains %d strat activation points \n",NumActs);
		if (NumActs)
		{
			MapActs =  (MAPACTIVATION *)SHeapAlloc (MissionHeap, sizeof (MAPACTIVATION) * NumActs);
			fread(MapActs,sizeof(MAPACTIVATION),NumActs,fp);

		}

		/*NOW READ IN THE STRAT PATHS */

		fread(&NumPaths,sizeof(short),1,fp);
		dLog("map contains %d paths \n",NumPaths);
		if (NumPaths)
			MapPaths =  (WAYPATH *)SHeapAlloc (MissionHeap, sizeof (WAYPATH) * NumPaths);

		for (i=0;i<NumPaths;i++)
		{

			fread(&(MapPaths+i)->waytype,sizeof(short),1,fp);
			if ((MapPaths+i)->waytype == PATH)
			{
				(MapPaths+i)->net = NULL;
				fread(&NumWayPoints,sizeof(short),1,fp);

				(MapPaths+i)->numwaypoints = NumWayPoints;
				dLog("path %d paths has %d waypoints\n",i,NumWayPoints);
				(MapPaths+i)->waypointlist =  (Vector3 *)SHeapAlloc (MissionHeap, sizeof (Vector3) * NumWayPoints);

				fread((MapPaths+i)->waypointlist,sizeof(Vector3),NumWayPoints,fp);

				fread(&(MapPaths+i)->area,sizeof(short),1,fp);
				dLog("path %d is attached to %d area\n",i,(MapPaths+i)->area);
			}
			else
			{
				dLog("reading network info\n");

				fread(&NumWayPoints,sizeof(short),1,fp);
				(MapPaths+i)->numwaypoints = NumWayPoints;
				dLog("path %d paths has %d waypoints\n",i,NumWayPoints);
				(MapPaths+i)->waypointlist =  (Vector3 *)SHeapAlloc (MissionHeap, sizeof (Vector3) * NumWayPoints);
				fread((MapPaths+i)->waypointlist,sizeof(Vector3),NumWayPoints,fp);

				(MapPaths+i)->net =  (NET *)SHeapAlloc (MissionHeap, sizeof (NET));

					


				(MapPaths+i)->net->waybusy =  (char *)SHeapAlloc (MissionHeap, sizeof (char) * NumWayPoints);
				for (way=0;way<NumWayPoints;way++)
					(MapPaths+i)->net->waybusy[way]=0;
				
				
				(MapPaths+i)->net->connectors =  (int *)SHeapAlloc (MissionHeap, sizeof (int) * NumWayPoints);
				fread((MapPaths+i)->net->connectors,sizeof(int),NumWayPoints,fp);

		  /*		for (test=0;test<NumWayPoints;test++)
				{
					dLog("node %d connect 0 %d\n",test,(MapPaths+i)->net->connectors[test]&255);

					dLog("node %d connect 1 %d\n",test,((MapPaths+i)->net->connectors[test]>>8)&255);

					dLog("node %d connect 2 %d\n",test,((MapPaths+i)->net->connectors[test]>>16)&255);

					dLog("node %d connect 3 %d\n",test,((MapPaths+i)->net->connectors[test]>>24)&255);

				}	 */



/*				(MapPaths+i)->visited =  (char *)SHeapAlloc (MissionHeap, sizeof (char) * NumWayPoints); */

				fread(&NumPatrolRoutes,sizeof(unsigned char), 1, fp);
				dLog("number of patrol routes %d\n",NumPatrolRoutes);

				(MapPaths+i)->net->patrol = (PATROL *)SHeapAlloc (MissionHeap, sizeof (PATROL) * NumPatrolRoutes);


				for (patrol=0;patrol<NumPatrolRoutes;patrol++)
				{

					fread(&NumPathPoints,sizeof(unsigned char),1,fp);
					dLog("number of path points %d\n",NumPathPoints);
					(MapPaths+i)->net->patrol[patrol].numpathpoints = NumPathPoints;
					(MapPaths+i)->net->patrol[patrol].initialpath = 
					(unsigned char *)SHeapAlloc (MissionHeap, sizeof (unsigned char) * NumPathPoints);
					fread((MapPaths+i)->net->patrol[patrol].initialpath,sizeof(unsigned char),NumPathPoints,fp);

				}
				fread(&(MapPaths+i)->area,sizeof(short),1,fp);
				dLog("path %d is attached to %d area\n",i,(MapPaths+i)->area);
			}
				
		}

		/*Now read in the MapAreas, if any */
		if (!feof (fp))
		{

			

			fread(&NumAreas,sizeof(short),1,fp);
			if (NumAreas)	
				MapAreas = (MAPAREA*)SHeapAlloc(MissionHeap, sizeof (MAPAREA) * NumAreas);

			dLog("map has %d\n",NumAreas);
			for (i=0;i<NumAreas;i++)
			{
					/*READ IN THE SURROUNDING BBOX CIRCLE */
				fread(&(MapAreas+i)->Surround,sizeof(CIRCLEBOX),1,fp);
				dLog("surround rad %f\n",(MapAreas+i)->Surround.radius);

					/*READ THE APPROPRIATE SUB CIRCLE AREA INFO */
					fread(&(MapAreas+i)->numcircleareas,sizeof(short),1,fp);
					dLog("number of circle subs %d\n",(MapAreas+i)->numcircleareas);
					if ((MapAreas +i)->numcircleareas)
					{
						(MapAreas+i)->circlebbox =  (CIRCLEBOX *)SHeapAlloc (MissionHeap, sizeof (CIRCLEBOX) * (MapAreas+i)->numcircleareas);
						fread((MapAreas+i)->circlebbox,sizeof(CIRCLEBOX),(MapAreas+i)->numcircleareas,fp);
					}

					/*READ THE APPROPRIATE SUB BOX AREA INFO */
					fread(&(MapAreas+i)->numboxareas,sizeof(short),1,fp);
					dLog("number of box subs %d\n",(MapAreas+i)->numboxareas);
					if ((MapAreas +i)->numboxareas)
					{
						(MapAreas+i)->boxbbox =  (BOXBOX *)SHeapAlloc (MissionHeap, sizeof (BOXBOX) * (MapAreas+i)->numboxareas);
						fread((MapAreas+i)->boxbbox,sizeof(BOXBOX),(MapAreas+i)->numboxareas,fp);
					}

					#if 0
						/*READ THE APPROPRIATE SUB ELLIPSOID AREA INFO */
						fread(&(MapAreas+i)->numellipseareas,sizeof(short),1,fp);
						dLog("number of ellipse subs %d\n",(MapAreas+i)->numellipseareas);
						if ((MapAreas +i)->numellipseareas)
						{
							(MapAreas+i)->ellipsebbox =  (ELLIPSEBOX *)SHeapAlloc (MissionHeap, sizeof (ELLIPSEBOX) * (MapAreas+i)->numellipseareas);
							fread((MapAreas+i)->ellipsebbox,sizeof(ELLIPSEBOX),(MapAreas+i)->numellipseareas,fp);
						}
					#endif

					/*READ THE SPLINE SHAPE INFO */
					fread(&(MapAreas+i)->numsplineareas,sizeof(short),1,fp);
					dLog("number of shape subs %d\n",(MapAreas+i)->numsplineareas);
					if ((MapAreas +i)->numsplineareas)
					{
						(MapAreas+i)->splinebbox =  (SPLINEBOX *)SHeapAlloc (MissionHeap, sizeof (SPLINEBOX) * (MapAreas+i)->numsplineareas);
						
						for (entry=0;entry<(MapAreas+i)->numsplineareas;entry++)
						{
							fread(&(MapAreas+i)->splinebbox[entry].numpoints,sizeof(short),1,fp);
							dLog("number of points %d\n",(MapAreas+i)->splinebbox[entry].numpoints);
							(MapAreas+i)->splinebbox[entry].pointlist =  (Vector2 *)SHeapAlloc (MissionHeap, sizeof (Vector2) * (MapAreas+i)->splinebbox[entry].numpoints);
							fread((MapAreas+i)->splinebbox[entry].pointlist,sizeof(Vector2),(MapAreas+i)->splinebbox[entry].numpoints,fp);
						}
						
					}

					/*READ THE LINE SHAPE INFO */
					fread(&(MapAreas+i)->numlinesightareas,sizeof(short),1,fp);
					dLog("number of shape subs %d\n",(MapAreas+i)->numlinesightareas);
					if ((MapAreas +i)->numlinesightareas)
					{
						(MapAreas+i)->linesightbbox =  (SPLINEBOX *)SHeapAlloc (MissionHeap, sizeof (SPLINEBOX) * (MapAreas+i)->numlinesightareas);
						
						for (entry=0;entry<(MapAreas+i)->numlinesightareas;entry++)
						{
							fread(&(MapAreas+i)->linesightbbox[entry].numpoints,sizeof(short),1,fp);
							dLog("number of points %d\n",(MapAreas+i)->linesightbbox[entry].numpoints);
							(MapAreas+i)->linesightbbox[entry].pointlist =  (Vector2 *)SHeapAlloc (MissionHeap, sizeof (Vector2) * (MapAreas+i)->linesightbbox[entry].numpoints);
							fread((MapAreas+i)->linesightbbox[entry].pointlist,sizeof(Vector2),(MapAreas+i)->linesightbbox[entry].numpoints,fp);
						}
						
					}

			}


		}

		/*Now read in the MapLights, if any */
		if (!feof (fp)) {
			int light;
			fread(&NumLights,sizeof(short),1,fp);
			dLog("map contains %d lights \n",NumLights);
			for (light = 0; light < NumLights; ++light) {
				char type;
				Point3 pos;
				Vector3 dir;
				unsigned char col[3];
				float Near, Far, min, max;
				Light *l;
				LightingValue lv;

				fread (&type, 1, 1, fp);
				fread (&pos, sizeof (float), 3, fp);
				fread (col, sizeof (char), 3, fp);
				fread (&Near, sizeof (float), 1, fp);
				fread (&Far, sizeof (float), 1, fp);

				lv.r = col[0] * (1.f / 255.f);
				lv.g = col[1] * (1.f / 255.f);
				lv.b = col[2] * (1.f / 255.f);

				switch (type) {
				case 0: /* Omnilight */
					l = newOmniLight (&pos, &lv, Near, Far, NULL);
					l->flags |= LIGHT_STATIC;
					break;
				case 1: /* Spotlight */
					fread (&min, sizeof (float), 1, fp);
					fread (&max, sizeof (float), 1, fp);
					fread (&dir, sizeof (float), 3, fp);
					l = newSpotLight (&pos, &lv, Near, Far, dir, min, max, NULL);
					l->flags |= LIGHT_STATIC;
					break;
				default:
					dAssert (FALSE, "Oh no!");
					break;
				}
			}
		}

		fclose(fp);
	}
	else
	{
		sprintf(outstr,"HELP %s\n",MapFile);
		/*OutputDebugString(outstr); */
	}

	dLog("****NEXT FREE ADDR %x\n",NextFreeAnim);

   /*	StartTrig = MapTrigs; */
	/*NOW SET THE DUMMY PLAYER */
	Player = &DummStrat;
	StratReset(Player);
	PlayerLives = 3;
//	red = 0x10;	
	GameOverFlag=0;
	CamNo = 0;
	player[currentPlayer].CameraStrat = NULL;

	for (i=0;i<MAXCAM;i++)
	   CamTracks[i]=NULL;

}
#endif




void MakeTargets(float radius)
{

	STRAT *strat;
	Vector3 forward;
	TARGETS *Target,*LastTarget,*NextTarget2;
	float minradius,lastdist,cosangle;
	int i;
	
	strat = FirstStrat;

	radius = 200.0f;
	minradius = 0.0f;

	lastdist = 1000000.0f;

	/*CLEAR EXISTING TARGETS */
  

	player[pn].NearestTarget = player[pn].NextTarget = &player[pn].PrimaryTargets[0];
	for (i=0;i<MAX_PRIMTARGETS;i++)
	{
		player[pn].PrimaryTargets[i].prev = NULL;
		if (i==MAX_PRIMTARGETS-1)
			player[pn].PrimaryTargets[i].next = NULL;
		else
			player[pn].PrimaryTargets[i].next = &player[pn].PrimaryTargets[i+1];
		player[pn].PrimaryTargets[i].strat = NULL;
	}

	player[pn].NearestTarget->prev = NULL;

	LastTarget = NULL;
	while (strat)
	{
		if ((strat != player[currentPlayer].Player) && (strat != player[currentPlayer].CameraStrat)
			&& ((strat->flag & STRAT_COLLSTRAT))
			&& (!(strat->flag & STRAT_FRIENDLY))
			&& (!(strat->flag & STRAT_BULLET)) 
			&& (!(strat->flag & STRAT_HIDDEN)) 
			&& (!(strat->flag & STRAT_EXPLODEPIECE)) 
			&& ((strat->flag > STRAT_ALIVE)) 
		   	&& NextTarget)
				lastdist = TargetsNearPlayer(strat,radius,minradius,lastdist);
		strat = strat->next;

	}


	
/*		Target = NearestTarget; */
  /*		while (Target) */
	/*	{ */
			/*NextTarget2 = Target->prev; */
	  /*		if (Target->strat) */
		/* 	dLog("***strat log list2 %s add %x\n",Target->strat->pnode->name,Target->strat); */
		  /*	Target = Target->prev; */
	
			
	   /*	} */


	
	/*go through the list of targets removing those which are not within our viewing cone*/
	/* which at the moment is set at 45.0f */
	if (NearestTarget->strat)
	{
		radius = PI2/32.0f;

		DeriveForwardAim(&forward);
		cosangle = (float)cos(radius);
		Target = NearestTarget;
		LastTarget = Target;
		while (Target)
		{
		   /*	if (Target->strat) */
		   /*	 	dLog("***strat log list %s add %x\n",Target->strat->pnode->name,Target->strat); */
			NextTarget2 = Target->prev;
	  
			if (!WithinAim(Target->strat,cosangle,&forward))
			{	
				RemovePrimaryTarget(Target,LastTarget, pn);
				if (Target == NearestTarget)
					NearestTarget = NextTarget2;
			}
			else
				LastTarget = Target;
			Target = NextTarget2;
	
		}	
	 
		/*Target = NearestTarget; */
	   /*	Target = &PrimaryTargets[0]; */
	   /*	LastTarget = Target; */
	   /*	for (;;) */
	   /*	{ */
	   /*		Target->strat->flag2 |= STRAT2_TARGETTED; */
		 /*	NextTarget2 = Target->prev; */

		   /*	dLog("strat log %x\n",Target->strat); */
		   /*	Target = NextTarget2; */
	 	   /*	if ((!Target)) */
	   		 /*	break; */
	
		/*}	 */
  
	}


}







void DeriveProbePointVal(STRAT *strat,Vector2 *probe,float length)
{
	Vector3 forward;
	forward.z = 0.0f;forward.x=0.0f;
/*	forward.y = 1.5f * strat->pnode->radius * strat->relVel.y; */
	forward.y = length * strat->relVel.y;
  	if (forward.y<0)
		if (forward.y>-1.0f)
	  		forward.y-=length;
   	else
	 	if (forward.y<1.0f)
	   		forward.y+=length;



	if (strat->relVel.y >=0.0)
	   	forward.y = length;	
	else
   	   	forward.y = -length;	
	
	mVector3Multiply(&TESTVEC,&forward,&strat->m);

	probe->x = strat->pos.x+TESTVEC.x;
	probe->y = strat->pos.y+TESTVEC.y;

}

void DeriveCollPoint(STRAT *strat,Vector2 *probe,float ang)
{
	Vector3 forward;
	Matrix  m;

	m = strat->m;
	mPreRotateZ(&m,(float)ang);
	
	forward.z = 0.0f;forward.x=5.5f;/* * strat->relVel.y; */
	forward.y = 15.5f;/* * strat->relVel.y; */
	if (forward.y<0)
		if (forward.y>-1.0f)
			forward.y-=1.0f;
	else
		if (forward.y<1.0f)
			forward.y+=1.0f;

	mVector3Multiply(&TESTVEC,&forward,&m);

	probe->x = strat->pos.x+TESTVEC.x;
	probe->y = strat->pos.y+TESTVEC.y;

}



void DeriveProbeAbsolute(STRAT *strat,Vector2 *probe)
{
	Vector3 forward;
	forward.z = 0.0f;forward.x=0;
	forward.y = 13.0f * strat->vel.y;

	mVector3Multiply(&TESTVEC,&forward,&strat->m);

	probe->x = strat->pos.x+TESTVEC.x;
	probe->y = strat->pos.y+TESTVEC.y;


}




/* RETURNS 1 IF STRAT IS OUTSIDE AREA*/
int StratOutSide(STRAT *strat,float ProbeLength)
{
	Vector2 Probe;
	Vector3 TESTVEC,line,forward;

	if (!strat->route)
 		return(0);

	DeriveProbePointVal(strat,&Probe,ProbeLength);

 	if (!(StratInRegions(strat,Probe.x,Probe.y)))
	{  
		if (strat->route->CollisionType)
			return(1);
		else
			return(0);
		
	}
 	else
	{
		/* Probe is not outside the areas*/
		strat->CheckPos.x = Probe.x;

		strat->CheckPos.y = Probe.y;

 		return(0);
	}
}



#if 0
/*
 * Strat lights
 */
void AddOmniLight (STRAT *strat, float x, float y, float z, float min, float max)
{
	return;
}
void AddExplosionLight (STRAT *strat, float x, float y, float z, float min, float max)
{
	return;
}
void SetLightColour (STRAT *strat, int num, float r, float g, float b)
{
	return;
}
void SetLightDist (STRAT *strat, int num, float min, float max)
{
	return;
}
//long red=0x00;

void RegisterHit()
{
//	red = 0x1f;	
}
#endif


#if 0 /*LOCAL_COPY*/
#define MAPDIR "MAP\\"
#else
#define MAPDIR "C:\\STRATS\\MAP\\"
#endif
 void GetFurthestWay(STRAT *strat)
{
	float dist1,dist2;
	int	way1,way2;
	ROUTE *route=strat->route;


/*	dLog("cur further way %d\n",strat->curway); */

	/*IF ON FIRST POINT OF PATH, TEST THIS AND SECOND POINT */

/*	if (!strat->curway) */
	if (!route->curway)
	{
		way1 = 0;
		way2 = 1;
/*		dist1  = WayToPlayerDist(&strat->path->waypointlist[way1]); */
/*		dist2  = WayToPlayerDist(&strat->path->waypointlist[way2]); */
	
		dist1  = WayToPlayerDist(&route->path->waypointlist[way1]);
		dist2  = WayToPlayerDist(&route->path->waypointlist[way2]);
	
	}
	else
	{
		/*OTHERWISE ARE WE ON THE LAST POINT OF THE PATH ? */
/*		if (strat->curway == strat->path->numwaypoints-1) */
		if (route->curway == route->path->numwaypoints-1)
		{
		/*	way1 = strat->curway; */
		 /*	way2 = strat->curway-1; */
		  /*	dist1  = WayToPlayerDist(&strat->path->waypointlist[way1]); */
		   /*	dist2  = WayToPlayerDist(&strat->path->waypointlist[way2]); */
			way1 = route->curway;
			way2 = route->curway-1;
			dist1  = WayToPlayerDist(&route->path->waypointlist[way1]);
			dist2  = WayToPlayerDist(&route->path->waypointlist[way2]);
		}
		else
		{

/*			way1 = strat->curway-1;
			way2 = strat->curway+1;
			dist1  = WayToPlayerDist(&strat->path->waypointlist[way1]);
			dist2  = WayToPlayerDist(&strat->path->waypointlist[way2]);
  */

			way1 = route->curway-1;
			way2 = route->curway+1;
			dist1  = WayToPlayerDist(&route->path->waypointlist[way1]);
			dist2  = WayToPlayerDist(&route->path->waypointlist[way2]);

		}


	}


  /*	if (dist1 > dist2)
		strat->curway = way1;
	else
		strat->curway = way2;
		
	strat->CheckPos.x = strat->path->waypointlist[strat->curway].x;  
	strat->CheckPos.y = strat->path->waypointlist[strat->curway].y;  
	strat->CheckPos.z = strat->path->waypointlist[strat->curway].z;  

*/
	if (dist1 > dist2)
		route->curway = way1;
	else
		route->curway = way2;
		
	strat->CheckPos.x = route->path->waypointlist[route->curway].x;  
	strat->CheckPos.y = route->path->waypointlist[route->curway].y;  
	strat->CheckPos.z = route->path->waypointlist[route->curway].z;  

}




/*** FROM OLD DRAW.C
STRAT *BBoxStrat;
static void *foo (void *p, size_t size)
{
	(void)p;
	return malloc (size);
}



void rDrawObjectMatt(Object *Obj)
{
	int i;
#if DRAW_COLLISION_POINTS
	Point3	p;
#endif

	mPreTranslate(&mCurMatrix,Obj->crntPos.x,Obj->crntPos.y,Obj->crntPos.z);		
	mPreRotateXYZ(&mCurMatrix,Obj->crntRot.x,Obj->crntRot.y,Obj->crntRot.z);
	mPreScale(&mCurMatrix,Obj->scale.x,Obj->scale.y,Obj->scale.z);
	
	
	if (Obj->model)
		rDrawModel (Obj->model);

#if DRAW_COLLISION_POINTS
	for (i=0;i<Obj->ncp;i++)
	{
		mPoint3Multiply3(&p, &Obj->cp[i], &mCurMatrix);
		DrawMarker(p.x, p.y, p.z);
	}
#endif

	for (i=0;i<Obj->no_child;i++)
	{
		mPush(&mCurMatrix);
			rDrawObjectMatt(&Obj->child[i]);
		mPop(&mCurMatrix);
	}	
}




 void mCopy3(Matrix *from, Matrix *to)
{
	int	i, j;

	for (i=0; i<3; i++)
		for (j=0; j<3; j++)
			to->m[i][j] = from->m[i][j];
}

void mCopy4(Matrix *from, Matrix *to)
{
	int	i, j;

	for (i=0; i<4; i++)
		for (j=0; j<4; j++)
			to->m[i][j] = from->m[i][j];
}

//***END OF OLDDRAW.C ROUTINES

/*returns 1 if newline has been hit*/
int DisplayScript(int entry, float numchars, int line,int x, int y, int mode)
{
	int i;
	unsigned char nChar;

	if (!GameScript[entry].script)
		return;

#define EXTRA_CHARS 6

	for (i=0;i<GameScript[entry].numlines;i++)
	{
		nChar = *GameScript[entry].script[i];
		if (i<line)
			ScriptPrint(x, y + i, GameScript[entry].script[i]+1,nChar + EXTRA_CHARS);
		else
		{ 
			if (i == line)
			{
				if (numchars < (nChar + EXTRA_CHARS))
				{
				//	if ((lineflash) && (numchars))
				//	{
						ScriptPrint(x, y + i, GameScript[entry].script[i]+1, numchars);
					   
				//		if (ltime >= 3)
				//		{
				//		 	ltime = 0;
				//			lineflash = 0;
				//		}
				//		else
				//			ltime++;
				//	}
				//	else
				//	{
				//		ScriptPrint(x, y + i, GameScript[entry].script[i]+1, numchars - 1);
				//		lineflash = 1;
				//	}
				}
				else
				{
				 	ScriptPrint(x, y + i, GameScript[entry].script[i]+1, nChar + EXTRA_CHARS);
					line ++;
					return(line);
				}
			}
		}
	}
	return(0);
}




 
void TowardWayMax(STRAT *strat)
{
	TowardWayX(strat,&strat->CheckPos,PI2);
	TowardWayZ(strat,&strat->CheckPos,PI2);
	strat->rot_speed=0;

}


void TowardWaySav(STRAT *crntStrat, Point3	*to, float	amount)
{
	Vector3	a, b, cross;

	a.x = to->x - crntStrat->pos.x;
	a.y = to->y - crntStrat->pos.y;
	a.z = to->z - crntStrat->pos.z;

	v3Normalise(&a);
	b.x = crntStrat->m.m[1][0];
	b.y = crntStrat->m.m[1][1];
	b.z = crntStrat->m.m[1][2];
	mVector3Multiply3( &b, &yAxis, &crntStrat->m); 
	v3Cross(&cross, &a, &b);
	combineRotation(crntStrat, &cross, amount);
}

int SeeWayZ(STRAT *strat,float angle)
{	
	Vector3 Forward, stratToWay;
	
	Forward.x=strat->m.m[1][0];
	Forward.y=strat->m.m[1][1];
	Forward.z=0.0f;
	v3Normalise(&Forward);

	stratToWay.x = strat->CheckPos.x - strat->pos.x;
	stratToWay.y = strat->CheckPos.y - strat->pos.y;
	stratToWay.z = 0.0f;
	v3Normalise(&stratToWay);

	angle = (float) cos(angle);

/*	if (acos(v3Dot(&Forward, &stratToWay)) < angle) */
	if ((v3Dot(&Forward, &stratToWay)) > angle)
		return 1;
	else
		return 0;
}



 
void SetTargetPullOut(STRAT *strat,float x, float y, float z)
{

	Vector3 vec,vec2;

	vec.x = x;
	vec.y = y;
	vec.z = z;
	mVector3Multiply (&vec2, &vec, &strat->m);
	strat->CheckPos.z = vec2.z+strat->pos.z; 
	strat->CheckPos.y = vec2.y+strat->pos.y; 
	strat->CheckPos.x = vec2.x+strat->pos.x ; 

/*	strat->waytoward=0; */
/*	strat->lastdist = WayDist(strat); */

}



 void TowardPlayerAbs(STRAT *strat)
{
 	Vector3 tempv;
 	Point3	origin;

	origin.x = origin.y = origin.z  =0.0f;

	tempv.x = (player[currentPlayer].Player->pos.x - strat->pos.x)*5.0f;
	tempv.y = (player[currentPlayer].Player->pos.y - strat->pos.y)*5.0f;
	tempv.z = (player[currentPlayer].Player->pos.z - strat->pos.z)*5.0f;
	ApplyForce(strat, &tempv, &origin, 100.0f);

}


  int MoveAway(STRAT *collider, STRAT *collided)
{
	Vector3 colliderdir, collideddir;
	float nineT;
	int movValid,dirValid;

	movValid =  dirValid = 0;


	nineT = cos(PI/2.0f);


	/*forward vector*/
	colliderdir.x = collider->m.m[1][0];
	colliderdir.y = collider->m.m[1][1];
	colliderdir.z = collider->m.m[1][2];


	collideddir.x = collided->m.m[1][0];
	collideddir.y = collided->m.m[1][1];
	collideddir.z = collided->m.m[1][2];

	if (v3Dot(&colliderdir,&collideddir) > nineT)
		dirValid = 1;
	   /*	return(1);*/


	/*direction vector*/

	colliderdir.x = collider->pos.x - collider->oldPos.x;
	colliderdir.y = collider->pos.y - collider->oldPos.y;
	colliderdir.z = collider->pos.z - collider->oldPos.z;


	collideddir.x = collided->pos.x - collided->oldPos.x;
	collideddir.y = collided->pos.y - collided->oldPos.y;
	collideddir.z = collided->pos.z - collided->oldPos.z;

	v3Normalise(&colliderdir);
	v3Normalise(&collideddir);

	if (v3Dot(&colliderdir,&collideddir) > nineT)
		movValid = 1;
		
	if (movValid & dirValid)
	return(1);
	else

	return(0);

}





 /*MODE	0 - CHECKPOS = VALUE
		1 - CHECKPOS += VALUE*/


void DeriveDeflection(STRAT *collider, STRAT *collided,int mode)
{
	Vector3 colliderforward;
	Vector3 collidedforward;
	Vector3 crossres;
	Vector3 checkpos;
	Vector3 newpos,newpos2,fromTo;
  	/*float dist; */
	Matrix  m;
	float	ang,x,y,dist;


 /*	if (collider->vel.y >= 0.0)
	{ */
		colliderforward.x = collider->m.m[1][0];
		colliderforward.y = collider->m.m[1][1];
  /*	}
	else
	{
		colliderforward.x = -collider->m.m[1][0];
		colliderforward.y = -collider->m.m[1][1];


	}
	*/

	colliderforward.z = 0;

	collidedforward.x = collided->m.m[0][0];
	collidedforward.y = collided->m.m[0][1];
	collidedforward.z = 0;

	v3Cross(&crossres,&collidedforward,&colliderforward);


	ang = acos(v3Dot(&colliderforward,&collidedforward));

	m = collider->m;

   	if (crossres.z <0)
   		mPreRotateZ(&m,-ang);
   	else
   		mPreRotateZ(&m,ang);

  /*	collider->m = m;   */
	
	checkpos.x = checkpos.z =0;
	checkpos.y = collider->pnode->radius;

	mVector3Multiply (&newpos, &checkpos, &m);

  /*	m = collided->m; */

  /* 	if (crossres.z <0)
   		mPreRotateZ(&m,ang);
   	else
   		mPreRotateZ(&m,-ang);

	collided->m = m;
	
	checkpos.x = checkpos.z =0;
	checkpos.y = collided->pnode->radius;

	mVector3Multiply (&newpos2, &checkpos, &m);	 */

	if (!mode)
	{
		collider->CheckPos.x = collider->pos.x + newpos.x;

		collider->CheckPos.y = collider->pos.y + newpos.y;

		collider->CheckPos.z = collider->pos.z + newpos.z;

		/*
		collided->CheckPos.x = collided->pos.x + newpos2.x;

		collided->CheckPos.y = collided->pos.y + newpos2.y;

		collided->CheckPos.z = collided->pos.z + newpos2.z;
		*/
	 }
	else
	{

		if (mode ==1)
		{

			if (collider->vel.y < 0)
			{ 
				collider->CheckPos.x += newpos.x;

			collider->CheckPos.y += newpos.y;
							   
			collider->CheckPos.z += newpos.z;	 
		}
			else
			{
				collider->CheckPos.x += newpos.x;

			collider->CheckPos.y += newpos.y;
							   
			collider->CheckPos.z += newpos.z;	 
			}

		  /*	collided->CheckPos.x += newpos2.x;

			collided->CheckPos.y += newpos2.y;
							   
			collided->CheckPos.z += newpos2.z;	 
			*/


			/*
			collider->CheckPos.x += (collider->pos.x - collided->pos.x) * dist;

			collider->CheckPos.y += (collider->pos.y - collided->pos.y) * dist;

			collider->CheckPos.z += (collider->pos.z - collided->pos.z) * dist;
			  */
			  /*	if (collider->pos.x < collided->pos.x)
					collider->CheckPos.x -= 1.0f * dist;
				else
					collider->CheckPos.x += 1.0f * dist;

				if (collider->pos.y < collided->pos.y)
					collider->CheckPos.y -= 1.0f * dist;
				else
					collider->CheckPos.y += 1.0f * dist; 
				*/
		
	}
		else
		{
			/*	if (collider->pos.x < collided->pos.x)
					collider->parent->pos.x -= 1.0f;
				else
					collider->parent->pos.x += 1.0f;

				if (collider->pos.y < collided->pos.y)
					collider->parent->pos.y -= 1.0f;
				else
					collider->parent->pos.y += 1.0f;   */

		 		collider->parent->pos.x += newpos.x;
				collider->CheckPos.x += newpos.x;

		 		collider->parent->pos.y += newpos.y;
		collider->CheckPos.y += newpos.y;
							   
		 		collider->parent->pos.z += newpos.z;
		collider->CheckPos.z += newpos.z;


				

		  /*		collided->parent->pos.x += newpos2.x;
				collided->CheckPos.x += newpos2.x;

		 		collided->parent->pos.y += newpos2.y;
		collided->CheckPos.y += newpos2.y;
							   
		 		collided->parent->pos.z += newpos2.z;
		collided->CheckPos.z += newpos2.z;
		*/
		
		}

	}
}





 /*COLLIDE WITH STRATS AND SET UP A DEFLECTION POINT*/
int	CollideStrat(STRAT *strat,float scalefactor,int mode)
{
	STRAT *str;
	Vector3	stratForward;
	Vector3 TESTVEC,probe;
	float x,y,dist,sourcearea;

	return(0);


	stratForward.z = 0.0f;stratForward.x=0.0f;
 
   /*	if (fabs(strat->vel.y)> 0.0 )	 */
		stratForward.y = fabs((strat->pnode->radius) * strat->vel.y);	

	 	if (strat->vel.y < -1.0)
			stratForward.y -= strat->pnode->radius;
		else
			stratForward.y += strat->pnode->radius;	 
		/* 	else
		stratForward.y = 1.0f;			*/
	mVector3Multiply(&TESTVEC,&stratForward,&strat->m);

	probe.x = strat->pos.x+TESTVEC.x;
	probe.y = strat->pos.y+TESTVEC.y;

 	probe.x = strat->pos.x;

	probe.y = strat->pos.y;
   

  	scalefactor = 1.0f;	  	
	sourcearea = strat->pnode->radius * strat->scale.x * scalefactor;
	sourcearea *= sourcearea;

	str = FirstStrat;
	while (str)
	{

		if ((!str->pnode) || (!(str->flag & STRAT_COLLSTRAT)))
		{
			str = str->next;
			continue;
		}




		if (str->flag & STRAT_BULLET)
		{
			str = str->next;
			continue;
		}

		if ((str==strat) || (str == strat->parent))
		{

			str = str->next;
			continue;

		}

		
		x = (str->pos.x - probe.x);
		y = (str->pos.y - probe.y);


		/*dist = (float)sqrt((x*x)+(y*y)); */

		dist = ((x*x)+(y*y));

		if (dist < (sourcearea))
		{
   	  	  /*	if (StratVisible(strat,str,((PI2/360.0) * 75.0)))
			{*/ 	   

				if 	(!MoveAway(str,strat))
			    {
				if
					(mode != 2)
					DeriveDeflection(strat,str,mode);	
			   /*	strat->pos.x += strat->pos.x - str->pos.x;

				strat->pos.y += strat->pos.y - str->pos.y;
		  */		 
				KillMyVelocity(strat);
				return(1);
					   }
		  /* }   */	
		}
		str = str->next;
	}

	return 0;
}



 int VisibleOnCurrent(STRAT *strat,float ProbeLength)
{
	Vector2 Probe;
	Vector3 TESTVEC,line,forward;

	if (!strat->route)
 		return(0);

	DeriveProbePointVal(strat,&Probe,ProbeLength);

 	/*1 = outer */
	if (strat->route->CollisionType == 1)
		return(1);
	else
	{
		if (InsideSpline(&Probe,strat->route->Collide))
			return(0);
		else
			return(1);
	}
}





 void RotateOrder(STRAT *strat,int currentway)
{
	int order = strat->path->net->connectors[currentway];
	int swap1 = (order&32767);
	int swap2  = order>>16;

	strat->path->net->connectors[currentway]=(swap1<<16)|swap2;
	

}
#if 0
void DeriveDeflection(STRAT *collider, STRAT *collided,int mode)
{
	Vector3 colliderforward;
	Vector3 colliderleft;
	Vector3 collidedforward;
	Vector3 crossres;
	Vector3 checkpos,temp;
	Vector3 newpos,fromTo;
  	/*float dist; */
	Matrix  m,newmat;
	float	ang,x,y,dist;


	colliderleft.x = collider->m.m[0][0];
	colliderleft.y = collider->m.m[0][1];
	colliderleft.z = collider->m.m[0][2];

	mInvertRot(&collider->m,&newmat);

	v3Sub(&temp,&collided->pos,&collider->pos);

	mVector3Multiply (&checkpos, &temp, &newmat);



	if (!mode)
	{
		if (checkpos.x >0)
		{
			collider->CheckPos.x = collider->pos.x + colliderleft.x;
			collider->CheckPos.y = collider->pos.y + colliderleft.y;
		}
		else
		{
			collider->CheckPos.x = collider->pos.x - colliderleft.x;
			collider->CheckPos.y = collider->pos.y - colliderleft.y;
		}
	}
	else
	{

		if (mode ==1)
		{
			if (checkpos.x >0)
			  	v3Inc(&collider->CheckPos,&colliderleft);
			else
			  	v3Sub(&collider->CheckPos,&collider->CheckPos,&colliderleft);

	
		}
		else
		{
			if (checkpos.x >0)
			{
			  	v3Inc(&collider->CheckPos,&colliderleft);
			  	v3Inc(&collider->parent->pos,&colliderleft);
			}
			else
			{
			  	v3Sub(&collider->CheckPos,&collider->CheckPos,&colliderleft);
			  	v3Sub(&collider->parent->pos,&collider->parent->pos,&colliderleft);
			}
		}

	}
}
#endif

 int StratVisible(STRAT *strat,STRAT *lookat, float angle)
{
	Vector3 Forward, stratToWay;
	
	Forward.x=strat->m.m[1][0];
	Forward.y=strat->m.m[1][1];
	Forward.z=0.0f;
	v3Normalise(&Forward);

	if (strat->vel.y < 0.0)
	{
		stratToWay.x = -(lookat->pos.x - strat->pos.x);
		stratToWay.y = -(lookat->pos.y - strat->pos.y);
}
	else
	{
		stratToWay.x = lookat->pos.x - strat->pos.x;
		stratToWay.y = lookat->pos.y - strat->pos.y;
}
	stratToWay.z = 0.0f;
	v3Normalise(&stratToWay);

	angle = (float) cos(angle);

/*	if (acos(v3Dot(&Forward, &stratToWay)) < angle) */
	if ((v3Dot(&Forward, &stratToWay)) > angle)
		return 1;
	else
		return 0;

}





int GetNearestCollideWayFoundToCheckPos(STRAT *strat,float x, float y)
{
	float lastdist,dist;
	int	startway,endway,way;
	ROUTE *route = strat->route;
	Vector3 point,dest;

	dest.x = x;
	dest.y = y;
	/*DERIVE DESTINATION SPLINE POINT*/

  	lastdist=10000000.0f;
	startway = route->curway;

	for (way=0;way<route->Collide->numpoints;way++)
	{
		point.x = route->Collide->pointlist[way].x;		  
		point.y = route->Collide->pointlist[way].y;		  
		dist = Score(&point,&dest);
				
		if (dist < lastdist)
		{
			lastdist = dist;
			endway = way;
		}
	}

	if (startway == endway)
	{
		strat->CheckPos.x = route->Collide->pointlist[endway].x;  
		strat->CheckPos.y = route->Collide->pointlist[endway].y;  
		return(1);
	}


		
  	ClearPath(strat,startway);
	if ((endway=GetNextPathSpline(strat,startway,endway)) < 0)
		return(1);

	if (startway == endway)
	{
		strat->CheckPos.x = route->Collide->pointlist[endway].x;  
		strat->CheckPos.y = route->Collide->pointlist[endway].y;  
		return(1);
	}   
		
	strat->CheckPos.x = route->Collide->pointlist[endway].x;  
	strat->CheckPos.y = route->Collide->pointlist[endway].y;  
		
	route->curway = endway;
	return(0);	
}




   
#if 0 

int GetNextPathSpline(STRAT *strat,int startnode,int WAYTOGO)
{
	int node=startnode;
	int found=0,way1,parent;
	int connect;
	int i,child,inopen,inclosed,loop,nodes[4],numconnects=0;
	ROUTE *route = strat->route;
	float newg;

   	/*WAYTEST = WAYTOGO;	*/
 	WAYTEST = WAYTOGO;

	ng[node] = 0;


	nh[node] = 
				Score2(&route->Collide->pointlist[node],
												&route->Collide->pointlist[node])*

				Score2(&route->Collide->pointlist[node],
												&route->Collide->pointlist[startnode]);


	nf[node] = ng[node] +	nh[node];
	PushNode(node);

	nparent[node]=0;
	
	while (NODESTACKPT>=0)
	{								     
		node = PopNode();

		if (node == WAYTOGO)
		{
			found ++;
			break;
	
		}

	 
		if (node==route->Collide->numpoints)
			child =0;
		else
	  		child = node+1;

		newg = ng[node] + 
			Score2(&route->Collide->pointlist[child],
					&route->Collide->pointlist[node]);


		inclosed = SearchClose(child);
		inopen = SearchOpen(child);

		if (((inclosed) || (inopen)) && (ng[child] <= newg))
			continue;

		nparent[child] = node;


		ng[child] = newg;
		nh[child] =  Score2(&route->Collide->pointlist[child],
					 	&route->Collide->pointlist[node])*

						Score2(&route->Collide->pointlist[child],
								&route->Collide->pointlist[startnode]);

		nf[child] = ng[child] +	nh[child];

		if (inclosed)
			RemoveClosed(child);
				
			
		if (!inopen)
			PushNode(child);
		InsertClosed(node);
	}

	if (found)
	{
	   

		for (i=0;i<256;i++)
		{
			if (nparent[i] == WAYTOGO)
			{
			 	parent = i;
				break;
			}
		}

		while (nparent[i])
		{
			way1 = i;

			i = nparent[i];

		}

		return(way1);

	}
	else
	{

		dLog("****NO PATH FOUND FROM %d TO %d \n",startnode,WAYTOGO);
		return(-1);
	}	
}
#endif


#if TESTEDIT
	void PlayerStratColl()
	{
		STRAT *strat = FirstStrat;
		int PlayerCollide=0;
		float x,y,z,dist;

		while (strat)
		{
			if (strat != Player)
			{
				strat->flag &= (0xffff-STRAT_HITPLAYER);

				if ((!PlayerCollide) && (player[currentPlayer].Player->flag & STRAT_COLLSTRAT))
				{
					x = player[currentPlayer].Player->pos.x - strat->pos.x;
					y = player[currentPlayer].Player->pos.y - strat->pos.y;
					z = player[currentPlayer].Player->pos.z - strat->pos.z;

					dist = (float)sqrt((x*x)+(y*y)+(z*z));

					if (dist < 1.5)
					{
						strat->flag |= STRAT_HITPLAYER;

						CollideStrat=strat;
					
	
						PlayerCollide++;
					}
				}

			}
			strat=strat->next;
		}		
	}
#endif




 int	ClampPos(STRAT *strat)
{
	int subarea, i, dir;
	Point3	p1, p2, pr, sp;
	Vector3	v, vn, tempv, vx,force;
	Vector2	pr2;
	SPLINEBOX	*sb;
	float lambda;
	int coll=0;

	force.x = 0;
	force.y = 0;
	force.z = 0;

	sp = strat->CheckPos;
	sp.z = 0.0f;
	switch (strat->route->CollisionType)
	{
		case (1):
			sb = strat->route->Collide;
			dAssert(sb->numpoints > 2, "Invalid area\n");
			p1.x = sb->pointlist[0].x;
			p1.y = sb->pointlist[0].y;
			p1.z = 0.0f;
			p2.x = sb->pointlist[1].x;
			p2.y = sb->pointlist[1].y;
			p2.z = 0.0f;
			v3Sub(&v, &p2, &p1);
			v3Scale(&v, &v, 0.5f);
			v3Add(&pr, &p1, &v);
			vn.x = v.y;
			vn.y = -v.x;
			vn.z = 0.0f;
			v3Normalise(&vn);
			v3Scale(&vn, &vn, 0.1f);
			v3Inc(&pr, &vn);
		
			pr2.x = pr.x;
			pr2.y = pr.y;

			if (!InsideSpline(&pr2,strat->route->Collide))
				dir = 1;
			else
				dir = -1;

			for (i=0; i<sb->numpoints; i++)
			{
				if (dir == 1)
				{
					p1.x = sb->pointlist[i].x;
					p1.y = sb->pointlist[i].y;
			
					p2.x = sb->pointlist[(i+1)%sb->numpoints].x;
					p2.y = sb->pointlist[(i+1)%sb->numpoints].y;	
				}
				else
				{
					p1.x = sb->pointlist[(i+1)%sb->numpoints].x;
					p1.y = sb->pointlist[(i+1)%sb->numpoints].y;
			
					p2.x = sb->pointlist[i].x;
					p2.y = sb->pointlist[i].y;
				}
				p2.z = 0.0f;
				p1.z = 0.0f;

				v3Sub(&v, &p2, &p1);
				vn.x = v.y;
				vn.y = -v.x;
				vn.z = 0.0f;

				v3Normalise(&vn);
				lambda = pointLineDistance(&sp, &p1, &p2);
				if (lambda > strat->pnode->radius)
					continue;

		   
				v3Sub(&tempv, &sp, &p2);
				v3Cross(&vx, &v, &tempv);
				if (vx.z > 0.0f)
					lambda += strat->pnode->radius;
				else
					lambda = strat->pnode->radius - lambda;

				coll++;
				v3Scale(&vn, &vn, lambda);
			   	v3Inc(&force, &vn);  
			}
			if (coll)
			{

				force.x = force.x / coll;
				force.y = force.y / coll;
				force.z = force.z / coll;
			   	v3Inc(&strat->CheckPos, &force);  

			}


			break;
		case (2):
			sb = strat->route->Collide;
			dAssert(sb->numpoints > 2, "Invalid area\n");
			p1.x = sb->pointlist[0].x;
			p1.y = sb->pointlist[0].y;
			p1.z = 0.0f;
			p2.x = sb->pointlist[1].x;
			p2.y = sb->pointlist[1].y;
			p2.z = 0.0f;
			v3Sub(&v, &p2, &p1);
			v3Scale(&v, &v, 0.5f);
			v3Add(&pr, &p1, &v);
			vn.x = v.y;
			vn.y = -v.x;
			vn.z = 0.0f;
			v3Normalise(&vn);
			v3Scale(&vn, &vn, 0.1f);
			v3Inc(&pr, &vn);
		
			pr2.x = pr.x;
			pr2.y = pr.y;

			if (InsideSpline(&pr2,strat->route->Collide))
				dir = 1;
			else
				dir = -1;

			for (i=0; i<sb->numpoints; i++)
			{
				if (dir == 1)
				{
					p1.x = sb->pointlist[i].x;
					p1.y = sb->pointlist[i].y;
			
					p2.x = sb->pointlist[(i+1)%sb->numpoints].x;
					p2.y = sb->pointlist[(i+1)%sb->numpoints].y;	
				}
				else
				{
					p1.x = sb->pointlist[(i+1)%sb->numpoints].x;
					p1.y = sb->pointlist[(i+1)%sb->numpoints].y;
			   
					p2.x = sb->pointlist[i].x;
					p2.y = sb->pointlist[i].y;
				}
				p2.z = 0.0f;
				p1.z = 0.0f;

				v3Sub(&v, &p2, &p1);
				vn.x = v.y;
				vn.y = -v.x;
				vn.z = 0.0f;

				v3Normalise(&vn);
				lambda = pointLineDistance(&sp, &p1, &p2);
				if (lambda > strat->pnode->radius)
					continue;

				v3Sub(&tempv, &sp, &p2);
				v3Cross(&vx, &v, &tempv);
				if (vx.z > 0.0f)
					lambda += strat->pnode->radius;
				else
					lambda = strat->pnode->radius - lambda;

				v3Scale(&vn, &vn, lambda);
			  	v3Inc(&force, &vn);   		
				
			  coll++;	
		}
			if (coll)
			{

				force.x = force.x / coll;
				force.y = force.y / coll;
				force.z = force.z / coll;
				v3Inc(&strat->CheckPos, &force);


			}

		break;

		default:
			return(0);

	}

	return(1);
}


void PosRot(STRAT *strat)
{

	ClampPos(strat);

}


void SetOldPosAllStrats(void)
{
	STRAT *strat;

	strat = FirstStrat;
	while(strat)
	{
		strat->oldPos = strat->pos;
		strat = strat->next;
	}
}


int FindValidPoint(STRAT *strat, float angle, float scale)
{
	Vector3 Probe,ProbeTransformed;
	Matrix  rotmat;
	rotmat = strat->m;

	mUnit(&rotmat);
	/*DERIVE THE FORWARD VECTOR*/
	Probe.x = Probe.z =0;
	Probe.y = scale;

	/*ROTATE THROUGH THE VERTICAL ANGLE*/
	mRotateZ(&rotmat,angle);

	/*APPLY THIS MATRIX TO OUR FORWARD VECTOR*/
	mPoint3Multiply3(&ProbeTransformed, &Probe, &rotmat);

	ProbeTransformed.x += strat->pos.x;
	ProbeTransformed.y += strat->pos.y;
   /*	Probe.z += strat->pos.z;*/


	strat->CheckPos.x = ProbeTransformed.x;
	strat->CheckPos.y = ProbeTransformed.y;

	/*NOW CHECK THE NEW POINT FOR VALIDITY WITHIN THE STRAT'S ALLOTTED AREA*/
	if (!(PointInRegions(strat,&ProbeTransformed)))
		return(0);
	else
	{
		strat->CheckPos.x = ProbeTransformed.x;
		strat->CheckPos.y = ProbeTransformed.y;
	   /*	strat->CheckPos.z = Probe.z;*/

		return(1);
	}




}






int InSight(STRAT *strat)
{
    return (PositionVisible(strat,&strat->pos,&player[currentPlayer].Player->pos));
}


/*Works out the nearest intersection point between a strat and a spline line, given its
  line direction*/

int FindSpline(STRAT *strat,Vector3 *Line, SPLINEBOX *splinearea)
{
	int   numpoints = splinearea->numpoints,i;
	Vector2 C,D,P,A,B;
	float	x,y,upper,lower,r,s;
	int CollideSegment=0;
	float strattoplay,strattopoint;
	float lastdist = 10000.0f;
	
	A.x=strat->pos.x;
	A.y=strat->pos.y;

	B.x=Line->x;
	B.y=Line->y;

	for (i=0;i<numpoints;i++)
	{
		C=splinearea->pointlist[i];
		
		/*CHECK FOR LAST SEGMENT */
		if (i==numpoints-1)
			D = splinearea->pointlist[0];
		else
			D = splinearea->pointlist[i+1];


		upper = (A.y - C.y) * (D.x - C.x) -
				(A.x - C.x) * (D.y - C.y);
		lower = (B.x - A.x) * (D.y - C.y) -
				(B.y - A.y) * (D.x - C.x);

		r = upper/lower;

		if ((r<0) || (r>1.0))
			continue;


		upper = (A.y - C.y) * (B.x - A.x) -
				(A.x - C.x) * (B.y - A.y);

		s = upper/lower;

		if ((s<0) || (s>1.0))
			continue;



		P.x = A.x + r*(B.x-A.x);
		P.y = A.y + r*(B.y-A.y);

		x = strat->pos.x - P.x;
		y = strat->pos.y - P.y;
		strattopoint = (x*x)+(y*y);


		if (strattopoint > lastdist)
			continue;

		lastdist = strattopoint;

		   	TESTLINE = 1;
		 	DRAWVEC.x =P.x;
			DRAWVEC.y =P.y;	  
	}

  	strat->CheckPos.x = DRAWVEC.x;

	strat->CheckPos.y = DRAWVEC.y;	 

   	 TESTLINE = 0;
}





void FindIntersectionPoint(STRAT *strat,int Line)
{

}


/*GIVEN A SPLINE, DOES STARTPOS TO ENDPOS INTERSECT IT */
int LineSightSpline(SPLINEBOX *spline, Vector3 *startpos, Vector3 *endpos)
{

	Vector2 start,end;

	start.x = startpos->x;
	start.y = startpos->y;

	end.x = endpos->x;
	end.y = endpos->y;

 	if (LineIntersectSpline(spline,&start,&end))
		return (NONVISIBLE);

	return(VISIBLE);


}


/*DETECTS THE VISIBILITY OF THE PLAYER BELOW THE STRAT */

float	SeePlayerBelow(STRAT *strat,float angle)
{	
/*	Matrix rotmat; */
	Vector3 Forward,dst,RotForward;
	double Yaw,length, cosangle;

	/*GIVEN AN ANGLE OF CONE DERIVE WHETHER A SPECIFIC POINT */
	/*CAN SEE THE PLAYER WITHIN IT */
	

	Forward.x=0;Forward.z=-1.0f;

	Forward.y=0.0f;

/*	mRotateXYZ (&rotmat,strat->rot.x,strat->rot.y,strat->rot.z); */
/*	mVector3Multiply3 (&RotForward, &Forward, &rotmat); */
	mVector3Multiply (&RotForward, &Forward, &strat->m);

	cosangle=cos(angle);


	dst.x = -(strat->pos.x - Player->pos.x);
	dst.y = -(strat->pos.y - Player->pos.y);
	dst.z = -(strat->pos.z - Player->pos.z);
	
	
	length=sqrt((dst.x*dst.x) + (dst.y * dst.y) +(dst.z*dst.z));


	Yaw = ((dst.x * RotForward.x)+(dst.y * RotForward.y)  + (dst.z * RotForward.z)) / length;

	if (Yaw > cosangle)
		return(1.0);
	else
		return(0.0);
}


void SetNewParent(STRAT *strat)
{
/*	dLog("doing trigger %x\n",strat->parent); */
	
	if (strat->parent->parent)
		strat->parent=strat->parent->parent;
}


int Occupied(STRAT *strat, Vector3 *point)
{
	float x,y,dist;
	STRAT *str;

	str = FirstStrat;
	while (str)
	{
		if (!str->route)
		{
			str = str->next;
			continue;
		}

		if (strat->route->path == str->route->path)
		{
			if (strat == str)
			{
				str = str->next;
				continue;
			}
			x = strat->pos.x - point->x; 

			y = strat->pos.y - point->y;
			
			dist = ((x*x) + (y*y));

			if (dist < (strat->pnode->radius * strat->pnode->radius))
				return(1);	

		}

		str = str->next;
	}
	return(0);


}




float Score(Point3 *src, Point3 *dest)
{
	float x = src->x - dest->x;
	float y = src->y - dest->y;

/*	return ((float) sqrt((x*x) +(y*y)));  */

	return (((x*x) +(y*y)));

}

void GetNearestOuterWay(STRAT *strat,float distance)
{
	float lastdist,playertowaydist,strattowaydist,dist,dist4,dist3;
	int	startway,endway,way;
	ROUTE *route = strat->route;
	short box = strat->route->path->area;
	MAPAREA *Area;
	Vector3 point;

   	Area = &MapAreas[box];
	
	lastdist=10000000.0f;

	for (way=0;way<Area->splinebbox[0].numpoints;way++)
	{
		point.x = Area->splinebbox[0].pointlist[way].x;		  
		point.y = Area->splinebbox[0].pointlist[way].y;		  
		point.z = 0;		  
		dist = WayToStratDistNoRoot(&point,strat);
				
		if ((dist < lastdist))
		{
			lastdist = dist;
			startway = way;
		}
	}

	strat->CheckPos.x = Area->splinebbox[0].pointlist[startway].x;

	strat->CheckPos.y = Area->splinebbox[0].pointlist[startway].y;


}



void GetNearestOuterToPlayer(STRAT *strat,float distance)
{
	float lastdist,playertowaydist,strattowaydist,dist,dist4,dist3;
	int	startway,endway,way;
	ROUTE *route = strat->route;
	short box = strat->route->path->area;
	MAPAREA *Area;
	Vector3 point;

   	Area = &MapAreas[box];
	
	lastdist=10000000.0f;

	for (way=0;way<Area->splinebbox[0].numpoints;way++)
	{


		point.x = Area->splinebbox[0].pointlist[way].x;		  
		point.y = Area->splinebbox[0].pointlist[way].y;		  
		point.z = 0;		  
		
		if (!LineSightPos(strat,&point,&strat->pos))
			continue;
		
		dist = WayToPlayerDistNoRoot(&point);
				
		if ((dist < lastdist))
		{
			lastdist = dist;
			startway = way;
		}
	}

	strat->CheckPos.x = Area->splinebbox[0].pointlist[startway].x;

	strat->CheckPos.y = Area->splinebbox[0].pointlist[startway].y;


}






/* ENSURES A 2D POINT IS WITHIN A SET OF BOUNDING REGIONS*/

int PointInRegions(STRAT *strat, float x, float y)
{
	short box = strat->route->path->area;
	MAPAREA *Area;
	Vector2 Probe;
	int subarea;
	float radius,radius2;

	Probe.x = x;
	Probe.y = y;

	/*IF NO DEFINED AREA FOR THIS STRAT THEN RETURN AS BEING INSIDE ALWAYS */
	if (box <0)
		return(INSIDE);

	Area = &MapAreas[box];
	radius=Area->Surround.radius;
	radius2=radius*radius;

	/*FIRST CHECK WE ARE INSIDE THE OUTERMOST CIRCLE */
	if (!(StratDistanceToPointXYNoRoot(&Probe,&Area->Surround.pos) < radius2))
		return (OUTSIDE);
	else
	{
		/*CHECK INTERSECTION WITH CIRCULAR REGIONS */
		for (subarea=0;subarea<Area->numcircleareas;subarea++)
		{
			radius = Area->circlebbox[subarea].radius;
			radius2=radius*radius;
			/*if distance less than circle's radius we are outside the valid area */
			if (StratDistanceToPointXYNoRoot(&Probe,&Area->circlebbox[subarea].pos) < radius2)
			{
				return (INSIDE);
			}
		}

		/*CHECK INTERSECTION WITH SPLINE REGIONS */
		for (subarea=0;subarea<Area->numsplineareas;subarea++)
			if (InsideSpline(&Probe,&Area->splinebbox[subarea]))
			{
				strat->route->CollisionType = 1;
				strat->route->Collide = &Area->splinebbox[subarea];
				return (INSIDE);
			}
			return(OUTSIDE);
	}
}





PNode *FindModelPnode(char *modelname)
{
	int i;
	for (i=0;i<NumMod;i++)
	{
		if (!(strncmp(modelname,GameObjects[i].name,strlen(GameObjects[i].name)+1)))
			return (&GameObjects[i]);
	}
}
#endif




#if 0
int GetNextNetWay(STRAT *strat,int way)
{
	int loop,connect,destway=way,wayd;

	for (loop=0;loop<4;loop++)
	{
		if (!(connect = (strat->path->net->connectors[strat->curway]>>8*loop)&255))
			break;
		connect -- ;
		if (connect != way)
		{
			destway = connect;
			break;
		}
	}

	wayd = strat->curway;
	dLog("way now %d new way %d\n",way,destway);
	strat->curway=destway;

	strat->CheckPos.x = strat->path->waypointlist[strat->curway].x;  
	strat->CheckPos.y = strat->path->waypointlist[strat->curway].y;  
	strat->CheckPos.z = strat->path->waypointlist[strat->curway].z;  


	return (wayd);
	return (destway);

}
#endif

#if 0


void SetModelByName(STRAT *strat,char *modelname)
{
	// If there was a previous model, free that XXX is this right, MattP?  mg
	if (strat->pnode)
		CleanUp (strat);

	strat->pnode = FindModelPnode(modelname);
//	dLog("allocating %d object \n",sizeof(Object));


	// Only take a copy of the object if necessary
	if (strat->pnode->flag & OBJECT_SHARE_OBJECT_DATA) {
		strat->obj = strat->pnode->obj;
		strat->objId = strat->pnode->objId;
	} else {
		// Take a copy for the strat
		strat->obj = (Object *)CHeapAlloc(ObjHeap,sizeof(Object));
		TOTALALLOC+=sizeof(Object);
		dAssert(strat->obj,"OUT OF OBJ SPACE\n");
		
		newObject(strat->pnode->obj, strat->obj);
		setParent(strat->obj, NULL);
		setOCPT(strat, strat->obj);
		
		if (strat->pnode->noObjId)
		{
			//dLog("allocating %d object ids \n",sizeof(Object*) * (strat->pnode->noObjId));
			strat->objId = (Object **)CHeapAlloc(ObjHeap,sizeof(Object *) * (strat->pnode->noObjId + 1));
			TOTALALLOC+=sizeof(Object*) * (strat->pnode->noObjId+1);
			dAssert(strat->objId !=NULL,"OBJID Failed");
			setObjId(strat->objId, strat->pnode->objId, strat->obj, strat->pnode->obj, strat->pnode->noObjId);
		}
		else
			strat->objId=NULL;
	}
}



void calcRotMatFull(Matrix *m,float *xang,float *yang,float *zang);
#define PRECISE 1


void calcRotMatFull(Matrix *m,float *xang,float *yang,float *zang)
{
	Vector3	rotation;

#if PRECISE
		// Find the x rotation
/*	rotation.x = (float) asin (unscaled.GetRow(1).z);

	// Check cos(x)
	if (fabs(cos(rotation.x)) > 0.001f) {
		rotation.y	= (float) atan2 (-unscaled.GetRow(0).z, unscaled.GetRow(2).z);
		rotation.z	= (float) atan2 (-unscaled.GetRow(1).x, unscaled.GetRow(1).y);
	} else {
		rotation.y	= (float) atan2 (unscaled.GetRow(2).x, unscaled.GetRow(0).x);
		rotation.z	= 0.f;
	}
*/
	// Find the x rotation
	rotation.x = (float) asin (m->m[1][2]);

	// Check cos(x)
	if (fabs(cos(rotation.x)) > 0.001f) {
		rotation.y	= (float) atan2 (-m->m[0][2], m->m[2][2]);
		rotation.z	= (float) atan2 (-m->m[1][0], m->m[1][1]);
	} else {
		rotation.y	= (float) atan2 (m->m[2][0], m->m[0][0]);
		rotation.z	= 0.0f;
	}

	*xang = rotation.x;
	*zang = rotation.z;
	*yang = rotation.y;
#else
	float	cosY;

	*yang = (float)-asin(m->m[0][2]);
	cosY = (float)cos(*yang);
	*xang = (float)asin(m->m[1][2] / cosY);
	*zang = (float)acos(m->m[0][0] / cosY);
//	mVector3Multiply3(&forward, &yAxis, &crntStrat->m);

	//CHECK THE X COMPONENT OF THE Y ROTATION FOR DIRECTION
	//if (forward.x >= 0.0f)

	if (m->m[1][0] > 0.0f)
		*zang = PI2-*zang;
#endif	
}


void calcRotMat(Matrix *m,float *xang,float *zang)
{
//	Vector3	forward;

#if PRECISE	
	Vector3	rotation;

		// Find the x rotation
/*	rotation.x = (float) asin (unscaled.GetRow(1).z);

	// Check cos(x)
	if (fabs(cos(rotation.x)) > 0.001f) {
		rotation.y	= (float) atan2 (-unscaled.GetRow(0).z, unscaled.GetRow(2).z);
		rotation.z	= (float) atan2 (-unscaled.GetRow(1).x, unscaled.GetRow(1).y);
	} else {
		rotation.y	= (float) atan2 (unscaled.GetRow(2).x, unscaled.GetRow(0).x);
		rotation.z	= 0.f;
	}
*/
	// Find the x rotation
	rotation.x = (float) asin (m->m[1][2]);

	// Check cos(x)
	if (fabs(cos(rotation.x)) > 0.001f) {
		rotation.y	= (float) atan2 (-m->m[0][2], m->m[2][2]);
		rotation.z	= (float) atan2 (-m->m[1][0], m->m[1][1]);
	} else {
		rotation.y	= (float) atan2 (m->m[2][0], m->m[0][0]);
		rotation.z	= 0.f;
	}

	*xang = rotation.x;
	*zang = rotation.z;
#else
	float	cosY,yang;

	yang = (float)-asin(m->m[0][2]);
	cosY = (float)cos(yang);
	*xang = (float)asin(m->m[1][2] / cosY);
	*zang = (float)acos(m->m[0][0] / cosY);
//	mVector3Multiply3(&forward, &yAxis, &crntStrat->m);

	//CHECK THE X COMPONENT OF THE Y ROTATION FOR DIRECTION
	//if (forward.x >= 0.0f)

	if (m->m[1][0] > 0.0f)
		*zang = PI2-*zang;
#endif
}


void calcRotMatX(Matrix *m,float *xang)
{
	float	cosY,yang;

	yang = (float)-asin(m->m[0][2]);
	cosY = (float)cos(yang);
	*xang = (float)asin(m->m[1][2] / cosY);
	
}

void calcRotMatZ(Matrix *m,float *zang)
{
	float	cosY,yang;

	yang = (float)-asin(m->m[0][2]);
	cosY = (float)cos(yang);
	*zang = (float)acos(m->m[0][0] / cosY);

	if (m->m[1][0] > 0.0f)
		*zang = PI2-*zang;	
}


//TURN STRAT TOWARD POSTION VECTOR IN X and Y AT A ROTATION RATE OF MAXX and MAXY

void TowardPos (STRAT *strat, float maxx, float maxy)
{
	double result_x, result_y;
	double diff, diff2;
#if NEWSTRAT
	float zrot, xrot;

	calcRotMat(&strat->m,&xrot,&zrot);

	PosAngPitchYaw(&strat->pos, &strat->CheckPos, &result_x, &result_y);


	diff2 = result_y - zrot;

	if (diff2 > PI)
		diff2 -= PI2;
	if (diff2 < -PI)
		diff2 += PI2;


	diff = fabs(diff2);

	if (diff > maxy)
		diff = maxy ;

	if (diff2 > 0)
		mPreRotateZ(&strat->m,(float)diff);
	else
		mPreRotateZ(&strat->m,(float)-diff);
				
	//x angle

	diff2 = result_x - xrot;
   	if (diff2 > PI)
 		diff2 -= PI2;
	if (diff2 < - PI)
		diff2 += PI2;


	diff = fabs(diff2);
	if (diff > maxx)
	 	diff = maxx ;


   	if (diff2 > 0)
		mPreRotateX(&strat->m,(float)diff);
	else
		mPreRotateX(&strat->m,(float)-diff);
  	   
	mNormalize(&strat->m);
#else

	PosAngPitchYaw(&strat->pos, &strat->CheckPos, &result_x, &result_y);

	
	diff2 = (result_y) - strat->rot.z;

	if (diff2 > PI)
		diff2 -= PI2;
	if (diff2 < -PI)
		diff2 += PI2;


	diff = fabs(diff2);

	if (diff > maxy)
		diff = maxy ;

	if (diff2 > 0)
		strat->rot.z = (strat->rot.z + (float)diff);
	else
		strat->rot.z = (strat->rot.z - (float)diff);
				


	//x angle

	diff2 = result_x-strat->rot.x;

   	if (diff2 > PI)
 		diff2 -= PI2;
	if (diff2 < - PI)
		diff2 += PI2;

	diff = fabs(diff2);
	if (diff > maxx)
	 	diff = maxx ;

   	if (diff2 > 0)
	 	strat->rot.x = (strat->rot.x + (float)diff);
	else
	  	strat->rot.x = (strat->rot.x - (float)diff);	
  	   
	if (strat->rot.x > PI2)
		strat->rot.x = strat->rot.x-PI2;
	if (strat->rot.x < 0)
		strat->rot.x = PI2 + strat->rot.x;
	if (strat->rot.z > PI2)
		strat->rot.z = strat->rot.z-PI2;
	if (strat->rot.z < 0)
		strat->rot.z = PI2 + strat->rot.z;
	strat->flag |= STRAT_ROTATED;
#endif
}


void TowardPosYaw (STRAT *strat, float maxy)
{


	double	result_y;
	double	diff, diff2;

#if NEWSTRAT
	float zrot, xrot;


	calcRotMat(&strat->m,&xrot,&zrot);

	PosAngYaw(&strat->pos, &strat->CheckPos, &result_y);


	diff2 = (result_y) - zrot;

	if (diff2 > PI)
		diff2 -= PI2;
	if (diff2 < -PI)
		diff2 += PI2;


	diff = fabs(diff2);

	if (diff > maxy)
		diff = maxy ;

	if (diff2 > 0)
		mPreRotateZ(&strat->m,(float)diff);
	else
		mPreRotateZ(&strat->m,(float)-diff);
				
  	   
	mNormalize(&strat->m);
#else
	
	PosAngYaw(&strat->pos, &strat->CheckPos, &result_y);


	diff2 = result_y - strat->rot.z;


	if (diff2 > PI)
		diff2 -= PI2;
	if (diff2 < -PI)
		diff2 += PI2;
	
	
	diff = fabs(diff2);
	
	if (diff > maxy)
		diff = maxy;

	
	
	if (diff2 > 0)
		strat->rot.z = (strat->rot.z + (float)diff) ;
	else
		strat->rot.z = (strat->rot.z - (float)diff) ;

	if (strat->rot.z > PI2)
		strat->rot.z = strat->rot.z - PI2;
	if (strat->rot.z < 0)
		strat->rot.z = PI2 + strat->rot.z;
	strat->flag |= STRAT_ROTATED;
#endif
}

void TowardPosPitch (STRAT *strat, float maxx)
{

	double result_x;	
	double diff, diff2;
	
	
#if NEWSTRAT
	float zrot, xrot;

	calcRotMat(&strat->m,&xrot,&zrot);

	PosAngPitch(&strat->pos, &strat->CheckPos, &result_x);


	diff2 = (result_x) - xrot;

	if (diff2 > PI)
		diff2 -= PI2;
	if (diff2 < -PI)
		diff2 += PI2;


	diff = fabs(diff2);

	if (diff > maxx)
		diff = maxx ;

	if (diff2 > 0)
		mPreRotateX(&strat->m,(float)diff);
	else
		mPreRotateX(&strat->m,(float)-diff);
				
  	   
	mNormalize(&strat->m);
#else
	PosAngPitch(&strat->pos, &strat->CheckPos, &result_x);
	
	diff2 = result_x - strat->rot.x;

	if (diff2 > PI)
		diff2 -= PI2;
	if (diff2 < -PI)
		diff2 += PI2;

	diff = fabs(diff2);

	if (diff > maxx)
		diff = maxx;

	if (diff2 > 0)
		strat->rot.x = (strat->rot.x + (float)diff);
	else
		strat->rot.x = (strat->rot.x - (float)diff);

	if (strat->rot.x > PI2)
		strat->rot.x = strat->rot.x-PI2;
	if (strat->rot.x < 0)
		strat->rot.x = PI2 + strat->rot.x;
	strat->flag |= STRAT_ROTATED;
#endif
}

void TowardPlayerYaw (STRAT *strat, float maxy)
{
	double	result_y;
	double	diff, diff2;


#if NEWSTRAT
	float zrot;

	calcRotMatZ(&strat->m,&zrot);


	PosAngYaw(&strat->pos, &player[currentPlayer].Player->pos, &result_y);

	//y angle

	diff2 = (result_y)-zrot;
   	if (diff2 > PI)
 		diff2 -= PI2;
	if (diff2 < - PI)
		diff2 += PI2;


	diff = fabs(diff2);
	if (diff > maxy)
	 	diff = maxy ;


   	if (diff2 > 0)
		mPreRotateZ(&strat->m,(float)diff);
	else
		mPreRotateZ(&strat->m,(float)-diff);
  	   
	mNormalize(&strat->m);
#else

	PosAngYaw(&strat->pos, &player[currentPlayer].Player->pos, &result_y);
	diff2 = result_y - strat->rot.z;

	if (diff2 > PI)
		diff2 -= PI2;
	if (diff2 < -PI)
		diff2 += PI2;
	
	diff = fabs(diff2);


	if (diff > maxy)
		diff = maxy;

	if (diff2 > 0)
		strat->rot.z = (strat->rot.z + (float)diff) ;
	else
		strat->rot.z = (strat->rot.z - (float)diff) ;

	if (strat->rot.z > PI2)
		strat->rot.z = strat->rot.z-PI2;
	if (strat->rot.z < 0)
		strat->rot.z = PI2 + strat->rot.z;
	strat->flag |= STRAT_ROTATED;
#endif
}

void TowardPlayerPitch (STRAT *strat, float maxx)
{


#if NEWSTRAT
	stratTowardWay(strat,&player[currentPlayer].Player->pos,maxx);
	
#else
	double result_x;
	double diff, diff2;
	
	PosAngPitch(&strat->pos, &player[currentPlayer].Player->pos, &result_x);
	
	diff2 = (result_x) - strat->rot.x;

	if (diff2 > PI)
		diff2 -= PI2;
	if (diff2 < -PI)
		diff2 += PI2;

	diff = fabs(diff2);

	if (diff > maxx)
		diff = maxx;

	if (diff2 > 0)
		strat->rot.x = (strat->rot.x + (float)diff);
	else
		strat->rot.x = (strat->rot.x - (float)diff);

	if (strat->rot.x > PI2)
		strat->rot.x = strat->rot.x-PI2;
	if (strat->rot.x < 0)
		strat->rot.x = PI2 + strat->rot.x;
	strat->flag |= STRAT_ROTATED;
#endif
}


void TowardWay (STRAT *strat, float maxx, float maxy)
{

#if NEWSTRAT
	
	stratTowardWay(strat,&strat->CheckPos,maxx);


#else	
	
	double result_x, result_y;
	double diff, diff2;

	PosAngPitchYaw(&strat->pos, &strat->CheckPos, &result_x, &result_y);

	diff2 = (result_y) - strat->rot.z;

	if (diff2 > PI)
		diff2 -= PI2;
	if (diff2 < -PI)
		diff2 += PI2;

	diff = fabs(diff2);

	if (diff > maxy)
		diff = maxy ;

	if (diff2 > 0)
		strat->rot.z = (strat->rot.z + (float)diff);
	else
		strat->rot.z = (strat->rot.z - (float)diff);
				


	//x angle

	diff2 = (result_x)-strat->rot.x;
   	if (diff2 > PI)
 		diff2 -= PI2;
	if (diff2 < - PI)
		diff2 += PI2;


	diff = fabs(diff2);
	if (diff > maxx)
	 	diff = maxx ;

   	if (diff2 > 0)
	 	strat->rot.x = (strat->rot.x + (float)diff);
	else
	  	strat->rot.x = (strat->rot.x - (float)diff);	
  	   
	if (strat->rot.x > PI2)
		strat->rot.x = strat->rot.x-PI2;
	if (strat->rot.x < 0)
		strat->rot.x = PI2 + strat->rot.x;
	if (strat->rot.z > PI2)
		strat->rot.z = strat->rot.z-PI2;
	if (strat->rot.z < 0)
		strat->rot.z = PI2 + strat->rot.z;

	strat->flag |= STRAT_ROTATED;
#endif	

}

void TowardWay2 (STRAT *strat, float maxx, float maxy)
{

#if NEWSTRAT

	stratTowardWay(strat,&strat->CheckPos,maxx);
#else	
	
	double result_x, result_y;
	double diff, diff2;

	PosAngPitchYaw(&strat->pos, &strat->CheckPos, &result_x, &result_y);

	diff2 = (result_y) - strat->rot.z;

	if (diff2 > PI)
		diff2 -= PI2;
	if (diff2 < -PI)
		diff2 += PI2;

	diff = fabs(diff2);

	if (diff > maxy)
		diff = maxy ;

	if (diff2 > 0)
		strat->rot.z = (strat->rot.z + (float)diff);
	else
		strat->rot.z = (strat->rot.z - (float)diff);
				


	//x angle

	diff2 = (result_x)-strat->rot.x;
   	if (diff2 > PI)
 		diff2 -= PI2;
	if (diff2 < - PI)
		diff2 += PI2;


	diff = fabs(diff2);
	if (diff > maxx)
	 	diff = maxx ;

   	if (diff2 > 0)
	 	strat->rot.x = (strat->rot.x + (float)diff);
	else
	  	strat->rot.x = (strat->rot.x - (float)diff);	
  	   
	if (strat->rot.x > PI2)
		strat->rot.x = strat->rot.x-PI2;
	if (strat->rot.x < 0)
		strat->rot.x = PI2 + strat->rot.x;
	if (strat->rot.z > PI2)
		strat->rot.z = strat->rot.z-PI2;
	if (strat->rot.z < 0)
		strat->rot.z = PI2 + strat->rot.z;
	strat->flag |= STRAT_ROTATED;
#endif	

}




#endif

#if 0

void DeflectMe2(STRAT *strat,Vector2 *pos,float def)
{	
    Vector3 Force,AppForcePoint;
	float amount,turn;

	AppForcePoint.x = TESTVEC.x;
	AppForcePoint.y = TESTVEC.y;
	AppForcePoint.z=0;

    Force.x = pos->x - (strat->pos.x+TESTVEC.x) + 1.0f;
	Force.y = pos->y - (strat->pos.y+TESTVEC.y) + 1.0f;
	Force.z=0;	
	
	

	amount = def-(float)sqrt((Force.x*Force.x)+(Force.y*Force.y));

	//smaller the distance the greater the turn rate
	turn=0.001f; 
	amount = 3.0f * amount;
	//dLog("amount %f\n",amount);
	

	Force.x *=	-amount;
	Force.y *=	-amount;
   //	ApplyForce(strat,&Force,&AppForcePoint,turn);

	SetCheckPos(strat,strat->pos.x + (Force.x*10.0f),strat->pos.y + (Force.y*10.0f),strat->pos.z);

	
}

void		PickUpStrat()
{
	#if TESTEDIT
		dLog("picked up\n");
		if (CollideStrat)
			if (CollideStrat->flag & STRAT_HITPLAYER)
			{
				CollideStrat->flag |= STRAT_COLLECTED;
				Player->flag &= 0xffff-STRAT_COLLSTRAT;
			}
	#endif
} 

void		DropStrat()
{
	#if TESTEDIT
		if (CollideStrat)
			CollideStrat->flag &= (0xffff-STRAT_COLLECTED);
		CollideStrat=NULL;
		Player->flag |= STRAT_COLLSTRAT;
	#endif
}
	

void TowardPlayer (STRAT *strat, float maxx, float maxy)
{

#if NEWSTRAT
   //	dLog("toward player ****8\n");
	stratTowardWay(strat,&player[currentPlayer].Player->pos,maxx);
#else
	double result_x, result_y;
	double diff, diff2;

	
	PosAngPitchYaw(&strat->pos, &player[currentPlayer].Player->pos, &result_x, &result_y);

	diff2 = (result_y) - strat->rot.z;

	if (diff2 > PI)
		diff2 -= PI2;
	if (diff2 < -PI)
		diff2 += PI2;


	diff = fabs(diff2);

	if (diff > maxy)
		diff = maxy;

	if (diff2 > 0)
		strat->rot.z = (strat->rot.z + (float)diff);
	else
		strat->rot.z = (strat->rot.z - (float)diff);
				
	
	diff2 = (result_x)-strat->rot.x;


   	if (diff2 > PI)
   		diff2 -= PI2;
   	if (diff2 < - PI)
		diff2 += PI2;
  
	diff = fabs(diff2);

	if (diff > maxx)
	 	diff = maxx;

   	if (diff2 > 0)
	 	strat->rot.x = (strat->rot.x + (float)diff);
	else
	  	strat->rot.x = (strat->rot.x - (float)diff);	
   
	if (strat->rot.x > PI2)
		strat->rot.x = strat->rot.x-PI2;
	if (strat->rot.x < 0)
		strat->rot.x = PI2 + strat->rot.x;
	if (strat->rot.z > PI2)
		strat->rot.z = strat->rot.z-PI2;
	if (strat->rot.z < 0)
		strat->rot.z = PI2 + strat->rot.z;
	strat->flag |= STRAT_ROTATED;
#endif
}


void NearWay(STRAT *strat,float dist)
{
	double x,y,z;
	float real;

	x = (strat->CheckPos.x-strat->pos.x);
	y = (strat->CheckPos.y-strat->pos.y);
	z = (strat->CheckPos.z-strat->pos.z);

	real =(float)sqrt((x*x)+(y*y)+(z*z));
}



int NearParent(STRAT *strat,float dist)
{
	double x,y,z;
	float real;

	x = (strat->CheckPos.x-strat->pos.x);
	y = (strat->CheckPos.y-strat->pos.y);
	z = (strat->CheckPos.z-strat->pos.z);



	real =(float)sqrt((x*x)+(y*y)+(z*z));

	strat->relAcc.y = (0.005f)*(real);

	return(0);

}


int NearCheckPos2(STRAT *strat,float dist)
{
	double x,y,z;
	float real;

//	dist *= strat->vel.y;		
	
	x = (strat->CheckPos.x-strat->pos.x );
	y = (strat->CheckPos.y-strat->pos.y );
	z = (strat->CheckPos.z-strat->pos.z );

//	if (x > 10.0)
//		return (0);
//	if (y > 10.0)
//		return (0);
//	if (z > 10.0)
//		return (0);


	real =(float)sqrt((x*x)+(y*y)+(z*z));

	
	if (real <=dist)
		return(1);
	else
		return(0);


}


int SeePlayer(STRAT *strat,float angle)
{	
	Vector3 Forward, stratToWay;
	
	Forward.x=strat->m.m[1][0];
	Forward.y=strat->m.m[1][1];
	Forward.z=strat->m.m[1][2];
	v3Normalise(&Forward);

	stratToWay.x = Player->pos.x - strat->pos.x;
	stratToWay.y = Player->pos.y - strat->pos.y;
	stratToWay.z = Player->pos.z - strat->pos.z;
	v3Normalise(&stratToWay);

	if (acos(v3Dot(&Forward, &stratToWay)) < angle)
		return 1;
	else
		return 0;
}

// STRAT TO POSITION VECTOR

void PosAngPitchYaw(Vector3 *stratpos, Vector3 *pos, double *xang, double *yang)
{
	Vector3 dst;

	
	dst.x = (pos->x - stratpos->x );
	dst.z = (pos->z - stratpos->z );
	dst.y = (pos->y - stratpos->y );

	*xang = Satan2(dst.z, sqrt((dst.x*dst.x)+(dst.y*dst.y))) ;
	*yang = PI2-Satan2(dst.x,dst.y);

}

void PosAngPitch(Vector3 *stratpos, Vector3 *pos, double *xang)
{
	Vector3 dst;
	
	dst.x = (pos->x - stratpos->x );
	dst.z = (pos->z - stratpos->z );
	dst.y = (pos->y - stratpos->y );

	*xang = Satan2(dst.z, sqrt((dst.x*dst.x)+(dst.y*dst.y))) ;

}

void PosAngYaw(Vector3 *stratpos, Vector3 *pos, double *yawang)
{
	Vector3 dst;
	

	dst.x = (pos->x - stratpos->x);
	dst.y = (pos->y - stratpos->y);
	

	*yawang = PI2-Satan2(dst.x,dst.y);
}




int Over(STRAT *strat)
{

	Vector3 stratToWay;
	Vector3 Forward;

//	if (!strat)
//		return(0);

	//MAKE THIS A ONCE PER FRAME THING
	Forward.x=Player->m.m[1][0];
	Forward.y=Player->m.m[1][1];
	Forward.z=0.0f;
	v3Normalise(&Forward);

	stratToWay.x = -(player[currentPlayer].Player->objId[5]->wm.m[3][0] - strat->pos.x);
	stratToWay.y = -(player[currentPlayer].Player->objId[5]->wm.m[3][1] - strat->pos.y);
	stratToWay.z = 0.0f;
	v3Normalise(&stratToWay);

//	if (acos(v3Dot(&Forward, &stratToWay)) < angle)
	if ((v3Dot(&Forward, &stratToWay)) > (PI2/8.0f))
		return 1;
	else
		return 0;

}





#if TESTEDIT
	void ReOutMap()
	{
		FILE *fp;
		int i;
		char MapFile[128];
		dLog("opening file\n");

		sprintf(MapFile,"%s%s",MAPDIR,"LEVEL7.MAP");

		if (fp=fopen(MapFile, "wb"))
		{
			dLog("opened file\n");
			fwrite(&MapNum,sizeof(int),1,fp);

			for (i=0;i<MapNum;i++)
			{
				fwrite(&GameMap[i].StratEnt,sizeof(short),1,fp);

				fwrite(&GameMap[i].strat->pos,sizeof(Vector3),1,fp);
				fwrite(&GameMap[i].modelid,sizeof(short),1,fp);
			//	fwrite(&GameMap[i].relAcc,sizeof(Vector3),1,fp);
			//	fwrite(&GameMap[i].vel,sizeof(Vector3),1,fp);

				fwrite(&GameMap[i].rot,sizeof(Vector3),1,fp);
			}
			fclose(fp);
		}
	}
#endif


void ResetRot(STRAT *strat)
{
	mUnit(&strat->m);
}



void PlayerMech(STRAT *strat)
{
	GameMode=MECH;
	CamModeChange();
}

void PlayerTank(STRAT *strat)
{
	GameMode=TANK;
	CamModeChange();
}
#endif


#if 0
float BOIDPOS=0;


void BoidPosSet(STRAT *strat)
{
	Vector3 vec,vec2;

 	vec.x = strat->offset.x * 3.15f-(float)fabs((3.15f*(sin(strat->wobble))));
	vec.y = strat->offset.y;
	vec.z = strat->offset.z * 3.15f-(float)fabs((3.15f*(cos(strat->wobble))));	
   
	
	
	strat->wobble += 0.022f;

	mVector3Multiply (&vec2, &vec, &strat->parent->m);


	strat->CheckPos.z = vec2.z+strat->parent->pos.z; 
	strat->CheckPos.y = vec2.y+strat->parent->pos.y; 
	strat->CheckPos.x = vec2.x+strat->parent->pos.x ; 

}
#endif


#if 0
void BoidSpawn(STRAT *parent, int StratEntry, float xpos, float ypos, float zpos, float xang, float yang, float zang)
{
	STRAT *strat;
	strat = AddStrat(StratEntry);
	#if _DEBUG
		if (!(strat))
			return;
	#endif

	strat->pos.x = parent->pos.x + xpos;
	strat->pos.y = parent->pos.y + ypos;
	strat->pos.z = parent->pos.z + zpos;

 	strat->offset.x = xpos;
 	strat->offset.y = ypos ;
 	strat->offset.z = zpos;

#if !NEWSTRAT
	strat->rot.x = xang;
	strat->rot.y = yang;
	strat->rot.z = zang;
#endif
	strat->parent = parent;
	srand(rand());
	strat->wobble = 256.0f/((float)rand());
//	parent->child = strat;
}
#endif

#if 0
void TowardParent (STRAT *strat, float maxx, float maxy)
{
	
	BoidPosSet(strat);

	//strat->damp.x=1.0f;
	//strat->damp.y=1.0f;
	//strat->damp.z=1.0f;

	maxx = maxy = PI2/5.0f;

	maxx = maxy = PI2/BOIDPOS;


	//BOIDPOS+=35.0f;
	//BOIDPOS-=17.5f;
	//BOIDPOS-=7.5f;

	BOIDPOS-=0.75f;

	maxx = maxy = 0.052f;
	
	NearParent(strat,2.0f);
	TowardWay(strat,maxx,maxy);

}
#endif

#if 0
int Inside2(STRAT *strat,Vector2 *Probe);

void GetNextNonCollPoint(STRAT *strat)
{
	Vector2 probe;

		DeriveCollPoint(strat,&probe,PI2/4);
		if (Inside2(strat,&probe) == INSIDE)
		{
			dLog("RIGHT\n");
			strat->flag2 |= STRAT2_STRAFELEFT;
			SetCheckPos(strat,probe.x,probe.y,strat->pos.z);
			return;
		}


		DeriveCollPoint(strat,&probe,-PI2/4);
		if (Inside2(strat,&probe) == INSIDE)
		{
			dLog("LEFT\n");
			strat->flag2 |= STRAT2_STRAFERIGHT;
			SetCheckPos(strat,probe.x,probe.y,strat->pos.z);
			return;
		}


	dLog("BACK\n");
	strat->flag2 |= STRAT2_STRAFEBACK;
	DeriveCollPoint(strat,&probe,PI2/2);
	SetCheckPos(strat,probe.x,probe.y,strat->pos.z);



}

#endif


#if 0
int Inside2(STRAT *strat,Vector2 *Probe)
{

	short box = strat->path->area;
	MAPAREA *Area;
	int subarea;
	float radius,radius2,dist;

	//IF NO DEFINED AREA FOR THIS STRAT THEN RETURN AS BEING INSIDE ALWAYS
	if (box <0)
	{
		dLog("****** NO AREA LINKED WHEN CALLING INSIDE AREA ***\n");
		return(INSIDE);
	}
	Area = &MapAreas[box];


	radius=Area->Surround.radius;
	radius2=radius*radius;



	//FIRST CHECK WE ARE INSIDE THE OUTERMOST CIRCLE
	if (!(StratDistanceToPointXYNoRoot(Probe,&Area->Surround.pos) < radius2))
		return (OUTSIDE);
	else
	{

		//CHECK INTERSECTION WITH CIRCULAR REGIONS
		for (subarea=0;subarea<Area->numcircleareas;subarea++)
		{
			radius = Area->circlebbox[subarea].radius;
			radius2=radius*radius;
   
			
			//if distance less than circle's radius we are outside the valid area
			if (dist=StratDistanceToPointXYNoRoot(Probe,&Area->circlebbox[subarea].pos) < radius2)
				return (OUTSIDE);
		}


		//CHECK INTERSECTION WITH BOX REGIONS
		for (subarea=0;subarea<Area->numboxareas;subarea++)
		{
			if (InsideBox(Probe,&Area->boxbbox[subarea]))
				return (OUTSIDE);
		}

		
		//CHECK INTERSECTION WITH SPLINE REGIONS
		for (subarea=0;subarea<Area->numsplineareas;subarea++)
		{
			if (InsideSpline(Probe,&Area->splinebbox[subarea]))
				return (OUTSIDE);
		}

		


		return(INSIDE);
		

	}


}
#endif


#if 0
int LineSight(STRAT *strat,Vector2 *Probe)
{

	short box = strat->path->area;
	MAPAREA *Area;
	int subarea;
	float radius,radius2;

	//IF NO DEFINED AREA FOR THIS STRAT THEN RETURN AS BEING INSIDE ALWAYS
	if (box <0)
		return(INSIDE);

	Area = &MapAreas[box];


	radius=Area->Surround.radius;
	radius2=radius*radius;



	//FIRST CHECK WE ARE INSIDE THE OUTERMOST CIRCLE
	if (!(StratDistanceToPointXYNoRoot(Probe,&Area->Surround.pos) < radius2))
		return (OUTSIDE);
	else
	{

		//CHECK INTERSECTION WITH CIRCULAR REGIONS
		for (subarea=0;subarea<Area->numcircleareas;subarea++)
		{
			radius = Area->circlebbox[subarea].radius;
			radius2=radius*radius;
   
			if (LineSightCircle(&strat->pos,Probe,&Area->circlebbox[subarea].pos,radius2))
				return (OUTSIDE);
			
		}


		//CHECK INTERSECTION WITH BOX REGIONS
		for (subarea=0;subarea<Area->numboxareas;subarea++)
		{
			if (InsideBox(Probe,&Area->boxbbox[subarea]))
				return (OUTSIDE);
		}

		
		//CHECK INTERSECTION WITH SPLINE REGIONS
		for (subarea=0;subarea<Area->numsplineareas;subarea++)
		{
			if (InsideSpline(Probe,&Area->splinebbox[subarea]))
				return (OUTSIDE);
		}

		


		return(INSIDE);
		

	}

	   
}
#endif

#if 0
void combineRotationMat(STRAT *crntStrat, Vector3 *v2, float speed2)
{
	Vector3	av1, av2, fv, *v1;
	float	fv_length;
	float	*speed1;
	Matrix	*m;

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
	if (fv_length != 0.0f)
	{
		v1->x = fv.x / fv_length;
		v1->y = fv.y / fv_length;
		v1->z = fv.z / fv_length;
	}
	else
		dLog("zero length vector\n");

	*speed1 = fv_length;
}

Vector3 yAxis={0.0,1.0,0.0};

void newStratMatrixMat(STRAT *crntStrat)
{
	int	i,j;
	Matrix	tempm1, tempm2;

	makeRotMatrix(&tempm1, &crntStrat->rot_vect, crntStrat->rot_speed);
	mMultiply(&tempm2, &crntStrat->m, &tempm1);

	for (i=0; i<3; i++)
		for (j=0; j<3; j++)
			crntStrat->m.m[i][j] = tempm2.m[i][j];
}


void TowardWayMat(STRAT *crntStrat, Point3	*to, float	amount)
{
	Vector3	a, b, cross;

	a.x =   to->x-crntStrat->pos.x;
	a.y =   to->y-crntStrat->pos.y ;
	a.z =   to->z-crntStrat->pos.z ;

	
	v3Normalise(&a);
	mVector3Multiply( &b, &yAxis, &crntStrat->m);

	
	v3Cross(&cross, &a, &b);
	v3Normalise(&cross);

	
	amount *= (float)acos((double)v3Dot(&a,&b));
	if (amount > PI2)
		amount = PI2;
	if (amount < -PI2)
		amount = -PI2;

	
	combineRotationMat(crntStrat, &cross, -amount);
	newStratMatrixMat(crntStrat);  
	mNormalize(&crntStrat->m);
}

void UpdatePos(STRAT *strat)
{
	Vector3 vec2;

	//ROTATE FORCES BY OBJECT'S ROTATION	
	//AND UPDATE SPEED

	#if !TESTEDIT	
		if (!(strat->flag & SAVSTRAT))
	#endif
		{	

	#if TESTEDIT
			if (strat != Player)
				return;
	#endif


	#if	NEWSTRAT
			newStratMatrix(strat);
			mNormalize(&strat->m);	
	#else	
		//IF THE STRAT'S ROTATION IS CHANGED THEN MAKE NEW MATRIX
	
			if (strat->flag & STRAT_ROTATED)	
				mRotateXYZ (&strat->m,strat->rot.x,strat->rot.y,strat->rot.z);
			strat->flag &= 0xffff-STRAT_ROTATED;
	#endif

			mVector3Multiply (&vec2, &strat->relAcc, &strat->m);

			strat->vel.z += vec2.z; 
			strat->vel.y += vec2.y; 
			strat->vel.x += vec2.x; 

	
			//ADD TO WORLD POSITIONS


			strat->pos.x += strat->vel.x;
			strat->pos.y += strat->vel.y;
			strat->pos.z += strat->vel.z;
	
			//CLEAR FORCES
	
		   	strat->relAcc.x=0;
   			strat->relAcc.y=0;
   			strat->relAcc.z=0;

			//FRICTIONS			 
   	
			strat->vel.x *= strat->damp.x ;
   			strat->vel.z *= strat->damp.z;
   			strat->vel.y *= strat->damp.y;

			strat->rot_speed *= 0.5f;

		}	
}

/*TO TRIGGER ON STRAT PICKING UP */
void  WhenPickUp(STRAT *strat,TRIGGER *trig)
{	int me;
	if (!(trig->flag & TRIG_HELD))
	{
		if (trig->flag & TRIG_RUNNING)
		{
			TRIGSET		
			trig->address(strat);
		   	RestoreTrig(strat);

			TRIGRESTORE
		}
		else
		{		
			/*TODO */
			me = ((int)trig->trig_count)&(0xffffffff - STRAT2_PICKUP);
/*			me = (int)trig->trig_count; */
			if (Pickups[me])
			{
				TRIGSET		
				trig->address(strat);
		   	RestoreTrig(strat);

				TRIGRESTORE
			}
		}
	}
}

void  WhenPickUpUsed(STRAT *strat,TRIGGER *trig)
{	int me;
	if (!(trig->flag & TRIG_HELD))
	{
		if (trig->flag & TRIG_RUNNING)
		{
			TRIGSET		
			trig->address(strat);
		   	RestoreTrig(strat);

			TRIGRESTORE
		}
		else
		{		
			/*TODO */
			me = ((int)trig->trig_count)&(0xffffffff - STRAT2_PICKUP);
/*			me = (int)trig->trig_count; */
			if (!Pickups[me])
			{
				TRIGSET		
				trig->address(strat);
		   	RestoreTrig(strat);

				TRIGRESTORE
			}
		}
	}
}

/******* PICKUP FUNCTIONS ********/ 


/****** LOG THE PICKUP COLLECTED ***/ 

void RegisterPickup(STRAT *strat)
{
	int pickuptype = strat->flag2 & (0x0000ffff - STRAT2_PICKUP);

   /*	dLog("YOU PICKED UP %d\n",pickuptype); */

	switch (pickuptype)
	{
		case (STRAT2_POWERAMPPICKUP - STRAT2_PICKUP):
		case (STRAT2_SMARTAMPPICKUP - STRAT2_PICKUP):
		case (STRAT2_BLASTAMPPICKUP - STRAT2_PICKUP):
		case (STRAT2_BOOSTAMPPICKUP - STRAT2_PICKUP):
		   /*	dLog("YOU PICKED UP GOOD\n"); */
			Pickups[pickuptype] = 50;
			break;
		default:
			break;

	}
	
}


int Collected(int PickUpEntry)
{
	int pickuptype = PickUpEntry & (0xffffffff - STRAT2_PICKUP);

	return (Pickups[pickuptype]);
	
}

void DecPower(int PickUpEntry)
{
	int pickuptype = PickUpEntry & (0xffffffff - STRAT2_PICKUP);

	Pickups[pickuptype] -= 1;

	if (Pickups[pickuptype] < 0)
		Pickups[pickuptype] = 0;

/*	dLog("PICKVAL %d \n",Pickups[pickuptype]); */
/*	dLog("%f \n",0.35 * (float)Pickups[pickuptype]); */

}


void ClearPickups()
{
	int i;

	for (i=0;i<32;i++)
		Pickups[i] = 0;
	
}


void MakePickup(STRAT *parent,int MODELENTRY , int StratEntry, int flag)
{
	float xpos=0,ypos=0,zpos=0;
	STRAT *strat;
	strat = AddStrat(StratEntry);
	if (!(strat))
		return;
	strat->pos = parent->pos;
	strat->oldPos = strat->pos;

	strat->flag2 |= flag;
	SetModel(strat, MODELENTRY);
	strat->func(strat);
}

#endif

