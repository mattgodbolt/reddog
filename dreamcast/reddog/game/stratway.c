#include "reddog.h"
#include "Dirs.h"


#include "strat.h"
#include "command.h"
#include "process.h"
#include "camera.h"
#include "coll.h"
#include "localdefs.h"
#include "draw.h"
#include "level.h"
#include <ctype.h>

#define BUSYCHECK 1
#define RESETPATH 0
//FOR MY IMPROVED NETWORK ONCE DONE
#define NETTEST 0
int SeeCheckPosLateral(STRAT *strat,float angle)
{	
 	Vector3 Forward, stratToWay, cross;
	
	Forward.y=strat->m.m[0][1];
	Forward.x=strat->m.m[0][0];
	Forward.z=0.0f;
	v3Normalise(&Forward);

	stratToWay.x = strat->CheckPos.x - strat->pos.x;
	stratToWay.y = strat->CheckPos.y - strat->pos.y;
	stratToWay.z = 0.0f;
	v3Normalise(&stratToWay);

	angle = (float) cos(angle);

	if ((v3Dot(&Forward, &stratToWay)) >= angle)
	{	
	  //	Forward.x = strat->m.m[1][0];
	  //	Forward.y = strat->m.m[1][1];
	  //	Forward.z = 0;
		
		v3Cross(&cross,&stratToWay,&Forward);
		if (cross.z >0)
		return 1;
		else
		return 2;
   	}
	else
		return 0;
	
 }



float	WayToPlayerDist(Vector3 *way)
{
	float x,y,z;

	x = way->x - player[currentPlayer].Player->pos.x;
	y = way->y - player[currentPlayer].Player->pos.y;
	z = way->z - player[currentPlayer].Player->pos.z;

	return ((float)sqrt((x*x)+(y*y)+(z*z)));

}

float	WayToPlayerDistXY(Vector3 *way)
{
	float x,y;

	x = way->x - player[currentPlayer].Player->pos.x;
	y = way->y - player[currentPlayer].Player->pos.y;
	x = ((float)sqrt((x*x)+(y*y)));
	return x;

}

float	MPWayToPlayerDistXY(Vector3 *way,int num)
{
	float x,y;

	x = way->x - player[num].Player->pos.x;
	y = way->y - player[num].Player->pos.y;
	return ((float)sqrt((x*x)+(y*y)));

}

float	MPWayToPlayerDist(Vector3 *way,int num)
{
	float x,y,z;

	x = way->x - player[num].Player->pos.x;
	y = way->y - player[num].Player->pos.y;
	z = way->z - player[num].Player->pos.z;
	return ((float)sqrt((x*x)+ (y*y) + (z*z)));

}




float	WayToPlayerDistNoRoot(Vector3 *way)
{
	float x,y,z;

	x = way->x - player[currentPlayer].Player->pos.x;
	y = way->y - player[currentPlayer].Player->pos.y;
	z = way->z - player[currentPlayer].Player->pos.z;

	return (((x*x)+(y*y)+(z*z)));

}

float	WayToPlayerDistXYNoRoot(Vector3 *way)
{
	float x,y,z;

	x = way->x - player[currentPlayer].Player->pos.x;
	y = way->y - player[currentPlayer].Player->pos.y;
	return (((x*x)+(y*y)));

}

float	StratToPlayerDistNoRoot(STRAT *strat)
{
	float x,y,z;

	x = strat->pos.x - player[currentPlayer].Player->pos.x;
	y = strat->pos.y - player[currentPlayer].Player->pos.y;
	z = strat->pos.z - player[currentPlayer].Player->pos.z;

	return (((x*x)+(y*y)+(z*z)));

}


float	WayToStratDist(Vector3 *way,STRAT *strat)
{
	float x,y,z;

	x = way->x - strat->pos.x;
	y = way->y - strat->pos.y;
	z = way->z - strat->pos.z;

	return ((float)sqrt((x*x)+(y*y)+(z*z)));

}

float	WayToStratDistXY(Vector3 *way,STRAT *strat)
{
	float x,y;

	x = way->x - strat->pos.x;
	y = way->y - strat->pos.y;
	return ((float)sqrt((x*x)+(y*y)));

}

float	WayToStratDistNoRoot(Vector3 *way,STRAT *strat)
{
	float x,y,z;

	x = way->x - strat->pos.x;
	y = way->y - strat->pos.y;
	if (strat->flag & STRAT_HOVER)
		return (((x*x)+(y*y)));
	else

	z = way->z - strat->pos.z;

	return (((x*x)+(y*y)+(z*z)));

}


/*PATH FOLLOWING */

static int NUMTOT;
static int NODESTACKPT;

 typedef struct nodeentry
{
	int popped;
	int visited;
	int node;
}	NODEENTRY;
	
static NODEENTRY NODESTACK[256];
/*NODEENTRY NODETEMP[256];*/
static int NODESTACKCLOSED[256];
static float ng[256];
static float nf[256];
static float nh[256];
static int	  nparent[256];


static void PushNode(int node)
{
	NODESTACK[NUMTOT].node=node;	
	NODESTACK[NUMTOT].popped = 0;
	NUMTOT++;
	NODESTACKPT++;
}


static int PopNode()
{
	int i,loop,node=0;
	float lowscore;
	//= 9000000000.0f;	   
	
	//get the first score
	for (loop=0;loop<NUMTOT;loop++)
	{
		if (!NODESTACK[loop].popped)
		{
			lowscore = nf[NODESTACK[loop].node];

			node = loop;
			break;
		}
	}

	
	for (i=loop;i<NUMTOT;i++)
	{
		if (NODESTACK[i].popped)
			continue;
		else
		{
			if (nf[NODESTACK[i].node] <= lowscore)
			{
				node = i;
				lowscore = nf[NODESTACK[i].node];
			}
		}
	}


	NODESTACKPT--;	


  	NODESTACK[node].popped=1; 
	
	return(NODESTACK[node].node);
	
}

static void InsertClosed(int node)
{
	NODESTACKCLOSED[node]=node;


}

static void RemoveClosed(int node)
{
	NODESTACKCLOSED[node]=-1;
   /*	nparent[node]=-1; */

}

static int SearchClose(int node)
{
	if (NODESTACKCLOSED[node] <0)
		return(0);
	else
		return(1);

}

static int SearchOpen(int node)
{	int i;
	for (i=0;i<NUMTOT;i++)
	{
		if (NODESTACK[i].node == node)
			return (1);
	}

	return(0);

}


static float Score(Point3 *src, Point3 *dest)
{
	float x = src->x - dest->x;
	float y = src->y - dest->y;

/*	return ((float) sqrt((x*x) +(y*y)));  */

	return (((x*x) +(y*y)));

}

static float Score2(Point2 *src, Point2 *dest)
{
	float x = src->x - dest->x;
	float y = src->y - dest->y;

/*	return ((float) sqrt((x*x) +(y*y)));  */

	return (((x*x) +(y*y)));

}

static void ClearPath(STRAT *strat,int WAYTOGO)
{
	int i;
	ROUTE *route = strat->route;
	NODESTACKPT = 0;
	NUMTOT=0;
#if 0
	for (i=0;i<route->path->numwaypoints;i++)
	{
		ng[i]=0;
		nf[i]=0;
		nh[i]=0;
	   /*	nparent[i]=-1;
		NODESTACK[i].popped=0;
		NODESTACK[i].node=-1;
		NODESTACKCLOSED[i]=-1;*/
	}
#endif

	for (i=0;i<route->path->numwaypoints + 1;i++)
	{
		ng[i]=0;
		nf[i]=0;
		nh[i]=0;
		nparent[i]=-1;
		NODESTACK[i].popped=0;
		NODESTACK[i].node=-1;
		NODESTACKCLOSED[i]=-1;
	}

}


static int SHIFTS[4] = {0,8,16,24};



	   
	 
static int GetNextPath(STRAT *strat,int startnode,int WAYTOGO)
{
	int node=startnode;
	int found=0,way1,parent;
	int connect;
	int i,child,inopen,inclosed,loop,nodes[4],numconnects=0;
	ROUTE *route = strat->route;
	float newg;
#if NETTEST
	int first=0;
#endif

	//handle the start node
	
	ng[node] = 0;


   	/*
	nh[node] = 
   				Score(&route->path->waypointlist[node],
   												&route->path->waypointlist[WAYTOGO])*

   				Score(&route->path->waypointlist[node],
   												&route->path->waypointlist[WAYTOGO]);
   	*/
	/*
 	nh[node] = 
				Score(&strat->pos,
												&route->path->waypointlist[startnode])*

				Score(&strat->pos,
												&route->path->waypointlist[WAYTOGO]);
   	*/

  	nh[node] = 9000000000.0f;

	nf[node] = ng[node] +	nh[node];
	PushNode(node);

	nparent[node]=node;

	/*if ((startnode == 7) && (WAYTOGO == 9) ||
	   (startnode == 9) && (WAYTOGO == 7))
	   test = 1;
	*/
	while (NODESTACKPT>=0)
	{								     
		node = PopNode();

		if (node == WAYTOGO)
		{
			found ++;
			break;
	
		}

		numconnects = 0;

#if NETTEST
		if (!first)
		{
			nodes[0] = strat->route->curway;
			nodes[1] = strat->route->lastway;

			numconnects = 2;
			first = 1;

		}
		else
#endif
		{

		for (i=0;i<4;i++)
		{
			
			connect = ((route->path->net->connectors[node] >> (SHIFTS[i]))&255);
						   
			/*
			connect = ((route->path->net->connectors[node] >> (8*i))&255);
			  */
			if (connect)
			{

			 	nodes[numconnects]=connect-1;
				numconnects ++;
			}
		}
		}

		for (loop=0;loop<numconnects;loop++)
		{
			child = nodes[loop];
		   	if (route->path->net->waybusy[child] && (route->curway != child))
				continue;
			newg = ng[node] + 
			   	Score(&route->path->waypointlist[child],
												&route->path->waypointlist[node]);

			 //	Score(&route->path->waypointlist[child],
			 //									&route->path->waypointlist[WAYTOGO]);


			inclosed = SearchClose(child);
			inopen = SearchOpen(child);

			if (((inclosed) || (inopen)) && (ng[child] <= newg))
				continue;

			nparent[child] = node;


			ng[child] = newg;
			nh[child] =  Score(&route->path->waypointlist[child],
						 	&route->path->waypointlist[node])*

						Score(&route->path->waypointlist[child],
								&route->path->waypointlist[startnode]);

			nf[child] = ng[child] +	nh[child];

			if (inclosed)
				RemoveClosed(child);
				
			
			if (!inopen)
				PushNode(child);
		}

		InsertClosed(node);
	}

	if (found)
	{
	   


		found=0;
		i = WAYTOGO;
		while (!found)
		{

			if (nparent[i] == startnode)
			{
				found++;
				way1 = i;
				break;

			}
		  i = nparent[i];
		}



		return(way1);

	}
	else
	{

	   //	dLog("****NO PATH FOUND FROM %d TO %d \n",startnode,WAYTOGO);
	   //	dAssert(found,"NO PATH FOUND IN NETWORK TRAVERSAL");
		return(-1);
	}	
}




int GetNextPathSpline(STRAT *strat,int startnode,int WAYTOGO)
{
	int node=startnode;
	int destnode;
	float scoreforward=0;
	float scorebackward=0;

	if (node > strat->route->Collide->numpoints-1)
		node = strat->route->Collide->numpoints-1;


	while (node != WAYTOGO)
	{

		if (node==strat->route->Collide->numpoints-1)
			destnode = 0;
		else
			destnode = node +1;

		scoreforward +=  
				Score2(&strat->route->Collide->pointlist[node],
												&strat->route->Collide->pointlist[destnode]);



		node = destnode;


	}

	node = startnode;

	while (node != WAYTOGO)
	{

		if (!node)
/*			destnode = strat->route->Collide->numpoints-1; */
			destnode = strat->route->Collide->numpoints-1;
		else
			destnode = node -1;

		scorebackward +=  
				Score2(&strat->route->Collide->pointlist[node],
												&strat->route->Collide->pointlist[destnode]);



		node = destnode;


	}


	if (scorebackward < scoreforward)
	{
		if (startnode == 0)
	   		return(strat->route->Collide->numpoints-1);
	   /*		return(strat->route->Collide->numpoints-2);	 */
		else
			return(startnode-1);


	}
	else
	{
		if (startnode == strat->route->Collide->numpoints-1)
		 	return(0);
		 /*	return(1);	*/
		else
			return(startnode+1);


	}
}




/*resets the path to the first node */
/*in network mode this shall be done automatically within getnextway */

void ResetPath(STRAT *strat)
{
	ROUTE *route = strat->route;

	route->curway = 0;
	strat->CheckPos.x = route->path->waypointlist[route->curway].x;  
	strat->CheckPos.y = route->path->waypointlist[route->curway].y;  
	strat->CheckPos.z = route->path->waypointlist[route->curway].z;  

}

void ClearBusy(STRAT *strat)
{
	ROUTE *route = strat->route;
	short box;
	MAPAREA *Area;


	if (!route)
		return;


	if (route->camped > -1)
	{
		box = strat->route->path->area;
		Area = &MapAreas[box];

		Area->camppoints[route->camped].busy = 0;
		route->camped = -1;
	}

	if (route->path->waytype == NETWORK)
	{
		if (strat->flag2 & STRAT2_ONPATROLROUTE)
		   	route->path->net->waybusy[route->path->net->patrol[route->patrolnumber].initialpath[route->curway]]=0;  
		else
		   	route->path->net->waybusy[route->curway]=0;  
	}
}


/*gets the next waypoint, returns (0) if it cannot move to that point */

int GetNextWay(STRAT *strat)
{
   	ROUTE *route = strat->route;
  	int oldway = route->curway;

//	strat->lastdist=100000.0f;
	route->curway++;

#if RESETPATH
	if (strat->curway > strat->path->numwaypoints-1)
	{
/*	dLog("curway %d\n",strat->curway); */
		dLog("wayp %d\n",strat->path->numwaypoints);
		InitPath(strat);
		return;
	}
	else
	{
		strat->CheckPos.x = strat->path->waypointlist[strat->curway].x;  
		strat->CheckPos.y = strat->path->waypointlist[strat->curway].y;  
		strat->CheckPos.z = strat->path->waypointlist[strat->curway].z;  
	}
#else

	/*check for network mode */
	/*if at the end of a network head toward the start */

	if (route->path->waytype == NETWORK)
	{
	 	if (route->path->net->waybusy[route->path->net->patrol[route->patrolnumber].initialpath[route->curway]])
		{
			route->curway --;
			return (0);

		}

		if (route->curway > route->path->net->patrol[route->patrolnumber].numpathpoints-1)
			route->curway = 0;

		/*CLEAR THE OLD BUSY WAY...SET THE NEW WAY TO BUSY */
		#if BUSYCHECK
			route->path->net->waybusy[route->path->net->patrol[route->patrolnumber].initialpath[oldway]]=0;  
			route->path->net->waybusy[route->path->net->patrol[route->patrolnumber].initialpath[route->curway]]=1;  
		#endif
		strat->CheckPos.x = route->path->waypointlist[route->path->net->patrol[route->patrolnumber].initialpath[route->curway]].x;  
		strat->CheckPos.y = route->path->waypointlist[route->path->net->patrol[route->patrolnumber].initialpath[route->curway]].y;  
		strat->CheckPos.z = route->path->waypointlist[route->path->net->patrol[route->patrolnumber].initialpath[route->curway]].z;  
		route->lastway = route->curway;
		return(1);

	}
	else
	{
	
		if (route->curway > route->path->numwaypoints-1)
			route->curway = route->path->numwaypoints-1;
		strat->CheckPos.x = route->path->waypointlist[route->curway].x;  
		strat->CheckPos.y = route->path->waypointlist[route->curway].y;  
		strat->CheckPos.z = route->path->waypointlist[route->curway].z;  
		return(1);
	}
#endif
}

void GetPrevWay(STRAT *strat)
{
	
	ROUTE *route = strat->route;
//	strat->lastdist=100000.0f;
#if RESETPATH

	dLog("Not implemented yet\n");
	return;
	if (strat->curway > strat->path->numwaypoints-1)
	{
	dLog("curway %d\n",strat->curway);
		dLog("wayp %d\n",strat->path->numwaypoints);
		InitPath(strat);
		return;
	}
	else
	{
		strat->CheckPos.x = strat->path->waypointlist[strat->curway].x;  
		strat->CheckPos.y = strat->path->waypointlist[strat->curway].y;  
		strat->CheckPos.z = strat->path->waypointlist[strat->curway].z;  
	}
#else

	if (route->path->waytype == NETWORK)
	{
		if (route->curway == 0)
			route->curway=route->path->net->patrol[route->patrolnumber].numpathpoints;
		else
			route->curway--;
		strat->CheckPos.x = route->path->waypointlist[route->path->net->patrol[route->patrolnumber].initialpath[route->curway]].x;  
		strat->CheckPos.y = route->path->waypointlist[route->path->net->patrol[route->patrolnumber].initialpath[route->curway]].y;  
		strat->CheckPos.z = route->path->waypointlist[route->path->net->patrol[route->patrolnumber].initialpath[route->curway]].z;  

	}
	else
	{
		if (route->curway == 0)
			return;
		else
			route->curway--;
		strat->CheckPos.x = route->path->waypointlist[route->curway].x;  
		strat->CheckPos.y = route->path->waypointlist[route->curway].y;  
		strat->CheckPos.z = route->path->waypointlist[route->curway].z;  
	}

#endif
}


/*GETS THE NEAREST PATROL PATH WAY TO THE STRAT, CONSIDERING A NETWORK */
/*THIS IS TO ALLOW THINGS ON A NETWORK TO RETURN TO THEIR ORIGINAL PATHS */
/*A MINIMUM DISTANCE IS PASSED IN TO STOP THE STRAT FROM PICKING A  */
/*WAYPOINT THAT IT IS ALREADY ON */
int GetNearestNetPatrolPoint(STRAT *strat,float distance)
{
	int way,way1;
	float lastdist,dist;
	ROUTE *route = strat->route;
	lastdist=1000000000.0f;
//	strat->lastdist=1000000.0f;
	distance = distance * distance;

	if (route->path->waytype != NETWORK)
		return(GetNearestWay(strat,distance));

	if ((strat->flag2 & STRAT2_ENTIRENET) )
	{
		if (!distance)
			way1 = route->pathstartnode;
		else
		{
			way1=route->curway;
			for (way=0;way<route->path->numwaypoints;way++)
			{
				#if BUSYCHECK
					if ((route->path->net->waybusy[way]) && (way != route->curway))
			  			continue;
			   	#endif
					dist = WayToStratDistNoRoot(&route->path->waypointlist[way],strat);
					if ((dist < lastdist) && (dist >= distance))
					{
						lastdist = dist;
						way1 = way;
					}
			}
		}
/*		dLog("chosen way %d current is %d\n",way1,strat->curway); */

		dLog("chosen way %d current is %d\n",way1,route->curway);


	}
	else
	{

		way1=route->curway;
		for (way=0;way<route->path->net->patrol[route->patrolnumber].numpathpoints;way++)
		{
		
/*			dLog("considering node %d \n",strat->path->net->patrol[strat->patrolnumber].initialpath[way]); */

			dLog("considering node %d \n",route->path->net->patrol[route->patrolnumber].initialpath[way]);


		  	dist = WayToStratDistNoRoot(&route->path->waypointlist[route->path->net->patrol[route->patrolnumber].initialpath[way]],strat);
			if ((dist < lastdist) && (dist >= distance))
			{
				lastdist = dist;
				way1 = route->path->net->patrol[route->patrolnumber].initialpath[way];
			}
		}

	}
/*	dLog("chosen way %d current is %d\n",way1,strat->curway); */

	dLog("chosen way %d current is %d\n",way1,route->curway);

	
/*	if (!(strat->flag2 & STRAT2_ENTIRENET)) */
/*	{ */
/*		if (way1 == strat->curway) */
		if (way1 == route->curway)
 			return(1);
  /*	} */

	ClearPath(strat,way1);
   /*	way1 = GetNextPath(strat,strat->curway,way1); */
/*   	if ((way1=GetNextPath(strat,strat->curway,way1)) < 0) */
   	if ((way1=GetNextPath(strat,route->curway,way1)) < 0)
   		return(1);
	#if BUSYCHECK
		ClearBusy(strat);
		/*strat->path->net->waybusy[strat->curway] = 0; */
/*		strat->curway = way1;	 */
  /*		strat->path->net->waybusy[strat->curway] = 1; */
		route->curway = way1;	
		route->path->net->waybusy[route->curway] = 1;
	#else	 
/*		strat->curway = way1;	 */
		route->curway = way1;	
	#endif
	dLog("chosen way %d\n",way1);
/*	strat->CheckPos.x = strat->path->waypointlist[strat->curway].x;   */
/*	strat->CheckPos.y = strat->path->waypointlist[strat->curway].y;   */
/*	strat->CheckPos.z = strat->path->waypointlist[strat->curway].z;   */
	strat->CheckPos.x = route->path->waypointlist[route->curway].x;  
	strat->CheckPos.y = route->path->waypointlist[route->curway].y;  
	strat->CheckPos.z = route->path->waypointlist[route->curway].z;  
		
	return(0);	



}



/*GETS THE NEAREST SPLINE WAYPOINT WITHIN distance UNITS FROM REDDOG */
int GetNearestCollideWay(STRAT *strat,float distance)
{
	float lastdist,playertowaydist,strattowaydist,dist,dist4,dist3;
	int	startway,endway,way;
	ROUTE *route = strat->route;
	Vector3 point;

  	lastdist=1000000000.0f;

	for (way=0;way<route->Collide->numpoints;way++)
	{
		point.x = route->Collide->pointlist[way].x;		  
		point.y = route->Collide->pointlist[way].y;		  
		point.z = 0;		  
		dist = WayToStratDistNoRoot(&point,strat);
	  
		if ((dist < lastdist))
		{
			/*ONLY CONSIDER POINTS WE CAN SEE ON THE SPLINE*/
			
		   /*	if (LineSightSpline(route->Collide,&point,&strat->oldPos))
			{*/
				lastdist = dist;
				startway = way;
		   /*	}  */
		}
	}

	/*DERIVE DESTINATION SPLINE POINT*/

  	lastdist=1000000000.0f;
	endway = startway;

	for (way=0;way<route->Collide->numpoints;way++)
	{
		point.x = route->Collide->pointlist[way].x;		  
		point.y = route->Collide->pointlist[way].y;		  
		point.z = 0;		  
		dist = WayToPlayerDistXYNoRoot(&point);
				
		if ((dist < lastdist) && (dist>distance))
		{
		   /*	if (!Occupied(strat,&point))
			{*/
				lastdist = dist;
				endway = way;
		   	/*} */ 	
		}
	}

	
	if (startway == endway)
	{
		route->curway = endway;
		strat->CheckPos.x = route->Collide->pointlist[endway].x;  
		strat->CheckPos.y = route->Collide->pointlist[endway].y;  
		return(1);
	} 
		/*
	if (startway == route->Collide->numpoints-1)
		endway = 1;
	else
		endway = startway+1;
				 */



		
  	ClearPath(strat,startway);
	if ((endway=GetNextPathSpline(strat,startway,endway)) < 0)
		return(1);

	route->curway = endway;

	if (startway == endway)
	{
		strat->CheckPos.x = route->Collide->pointlist[endway].x;  
		strat->CheckPos.y = route->Collide->pointlist[endway].y;  
		return(1);
	}   
		
	strat->CheckPos.x = route->Collide->pointlist[endway].x;  
	strat->CheckPos.y = route->Collide->pointlist[endway].y;  
		
	return(0);	
}


int GetNearestCollideWayFound(STRAT *strat,float distance)
{
	float lastdist,dist;
	int	startway,endway,way;
	ROUTE *route = strat->route;
	Vector3 point;

	/*DERIVE DESTINATION SPLINE POINT*/

  	lastdist=9000000000.0f;
	startway = route->curway;

	for (way=0;way<route->Collide->numpoints;way++)
	{
		point.x = route->Collide->pointlist[way].x;		  
		point.y = route->Collide->pointlist[way].y;		  
		point.z = 0;		  
		dist = WayToPlayerDistXYNoRoot(&point);
				
		if ((dist < lastdist) && (dist>distance))
		{
		   /*	if (!Occupied(strat,&point))
			{*/
				lastdist = dist;
				endway = way;
		   	/*} */ 	
		}
	}

	if (startway == endway)
	{
		strat->CheckPos.x = route->Collide->pointlist[endway].x;  
		strat->CheckPos.y = route->Collide->pointlist[endway].y;  
		return(1);
	}
		/*
	if (startway == route->Collide->numpoints-1)
		endway = 1;
	else
		endway = startway+1;
				 */



		
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


//neg distance passed in gives consider all points mode
/*GETS THE NEAREST WAY WITHIN distance UNITS FROM REDDOG */
int GetNearestWay(STRAT *strat,float distance)
{
	float lastdist,playertowaydist,strattowaydist,dist,dist4,dist3;
	int	way1,way,test;
	ROUTE *route = strat->route;
 // 	lastdist=10000000.0f;
  	lastdist=9000000000.0f;


//	strat->lastdist=10000.0f;
  

	way1 = route->curway;

	if (distance >= 0.0)
	distance = distance * distance;


	if ((distance < 0) || (route->path->waytype == NETWORK))
	{
		//first a consider all points check
		if (distance < 0)
		{

			for (way=0;way<route->path->numwaypoints;way++)
			{
				if (strat->flag & STRAT_HOVER)
					dist = WayToPlayerDistXYNoRoot(&route->path->waypointlist[way]);
				else
					dist = WayToPlayerDistNoRoot(&route->path->waypointlist[way]);
				
				if ((dist < lastdist))
				{
					lastdist = dist;
					way1 = way;
				}
			}

			if (route->path->waytype == NETWORK)
			{
			#if BUSYCHECK

				ClearBusy(strat);
				route->lastway = route->curway;
			
				route->curway = way1;	
			
				route->path->net->waybusy[route->curway] = 1;
			#else
				route->curway = way1;	
			#endif
			}
			route->curway = way1;	
			strat->CheckPos.x = route->path->waypointlist[route->curway].x;  
			strat->CheckPos.y = route->path->waypointlist[route->curway].y;  
			strat->CheckPos.z = route->path->waypointlist[route->curway].z;  
		
		}
		else
		{

			//normal proceedings

			for (way=0;way<route->path->numwaypoints;way++)
			{
					#if BUSYCHECK

						/*if busy and not my own node then don't check */
						if ((route->path->net->waybusy[way]) && (way != route->curway))
				  	  		continue;
				   	#endif

					if (!LineSightPos(strat,&player[currentPlayer].Player->pos,&route->path->waypointlist[way]))
						continue;
				  
					if (strat->flag & STRAT_HOVER)
						dist = WayToPlayerDistXYNoRoot(&route->path->waypointlist[way]);
					else
						dist = WayToPlayerDistNoRoot(&route->path->waypointlist[way]);
				
					if ((dist < lastdist) && (dist>distance))
					{
						lastdist = dist;
						way1 = way;
					}
			}

			if (way1==route->curway)
				return(1);
		
			ClearPath(strat,way1);

			if ((way1=GetNextPath(strat,route->curway,way1)) < 0)
				return(1);

			#if BUSYCHECK
   				//is the candidate destination busy ?
				if (route->path->net->waybusy[way1])
				 	return(2);

				ClearBusy(strat);
				route->lastway = route->curway;
			
				route->curway = way1;	
				route->path->net->waybusy[route->curway] = 1;
			#else
				route->curway = way1;	
			#endif
		
			strat->CheckPos.x = route->path->waypointlist[route->curway].x;  
			strat->CheckPos.y = route->path->waypointlist[route->curway].y;  
			strat->CheckPos.z = route->path->waypointlist[route->curway].z;  
		}
		return(0);	
	}
	else
	{
	   //	return(GetNearestWaySpline(strat));
		


		dist4  = WayToPlayerDist(&route->path->waypointlist[way1]);
		dist3  = WayToStratDist(&route->path->waypointlist[way1],strat);

		if (dist4 < dist3)
			return(0);

		for (way=0;way<route->path->numwaypoints;way++)
		{
			dist = WayToPlayerDistNoRoot(&route->path->waypointlist[way]);
			if ((dist < lastdist) && (dist > distance))
			{
				lastdist = dist;
				way1 = way;
			}
		}


	   
		/*IF CURRENT DISTANCE IS CLOSER NO MOVEMENT NEEDED */
		dist = StratToPlayerDistNoRoot(strat);
		if (dist<lastdist)	
			return(1);
	
		if (way1 != route->curway)
		{

			dLog("way here\n");
			if (way1 < route->curway)	
				GetPrevWay(strat);	
			else	
				GetNextWay(strat);
			return(0);
		}
		else
		{

			if (!route->curway)	
			{	/*at the first waypoint	 */
				/*check player distance to way 1 versus strat's */
				
		
				playertowaydist = WayToPlayerDist(&route->path->waypointlist[1]);			
				strattowaydist = WayToStratDist(&route->path->waypointlist[1],strat);
		
				if (playertowaydist < strattowaydist)		
				{
				
					GetNextWay(strat);	
					return(0);
				}			
				else	
					return(1);
			
			}
			else
			{
				if (route->curway == route->path->numwaypoints-1)	
				{
					/*at the last waypoint */
					/*check to see if player is closer to way-1 than strat */


					way1 = route->curway-1;

					playertowaydist = WayToPlayerDist(&route->path->waypointlist[way1]);
					strattowaydist = WayToStratDist(&route->path->waypointlist[way1],strat);

					if (playertowaydist < strattowaydist)
					{
						GetPrevWay(strat);
						return(0);
					}
					else
						return(1);

				}
				else
				{

					return(1);
				
				}
			}
		}
	}
}

/*GETS THE NEAREST WAY WITHIN distance UNITS FROM STRAT'S TARGET */
int GetNearestWayToTarget(STRAT *strat,float distance)
{
	float lastdist,targettowaydist,strattowaydist,dist,dist4,dist3;
	int	way1,way;
	ROUTE *route = strat->route;
	STRAT *target = strat->target;


  	lastdist=10000000.0f;


//	strat->lastdist=10000.0f;
  

	way1 = route->curway;

	distance = distance * distance;


	if (route->path->waytype == NETWORK)
	{
		for (way=0;way<route->path->numwaypoints;way++)
		{
				#if BUSYCHECK

					/*if busy and not my own node then don't check */
					if ((route->path->net->waybusy[way]) && (way != route->curway))
			  	  		continue;
			   	#endif

				if (!LineSightPos(strat,&target->pos,&route->path->waypointlist[way]))
					continue;
				  
				dist = WayToStratDistNoRoot(&route->path->waypointlist[way],target);
				
				if ((dist < lastdist) && (dist>distance))
				{
					lastdist = dist;
					way1 = way;
				}
		}

		if (way1==route->curway)
			return(1);
		
		ClearPath(strat,way1);
		if ((way1=GetNextPath(strat,route->curway,way1)) < 0)
			return(1);


		#if BUSYCHECK
   		  if (route->path->net->waybusy[way1])
			 	return(1);
		

			ClearBusy(strat);
			route->curway = way1;	
			route->path->net->waybusy[route->curway] = 1;
		#else
			route->curway = way1;	
		#endif
		
		strat->CheckPos.x = route->path->waypointlist[route->curway].x;  
		strat->CheckPos.y = route->path->waypointlist[route->curway].y;  
		strat->CheckPos.z = route->path->waypointlist[route->curway].z;  
		
		return(0);	
	}
	else
	{


		for (way=0;way<route->path->numwaypoints;way++)
		{
			dist = WayToStratDistNoRoot(&route->path->waypointlist[way],target);
			if ((dist < lastdist) && (dist > distance))
			{
				lastdist = dist;
				way1 = way;
			}
		}


	
		if (way1 != route->curway)
		{

			dLog("way here\n");
			if (way1 < route->curway)	
				GetPrevWay(strat);	
			else	
				GetNextWay(strat);
			return(0);
		}
	 
		return(1);
		//	else
		{

			if (!route->curway)	
			{	/*at the first waypoint	 */
				/*check player distance to way 1 versus strat's */
				
		
				targettowaydist = WayToStratDistNoRoot(&route->path->waypointlist[1],target);			
				strattowaydist = WayToStratDistNoRoot(&route->path->waypointlist[1],strat);
		
				if (targettowaydist < strattowaydist)		
				{
				
					GetNextWay(strat);	
					return(0);
				}			
				else	
					return(1);
			
			}
			else
			{
				if (route->curway == route->path->numwaypoints-1)	
				{
					/*at the last waypoint */
					/*check to see if player is closer to way-1 than strat */


					way1 = route->curway-1;

					targettowaydist = WayToStratDistNoRoot(&route->path->waypointlist[way1],target);
					strattowaydist = WayToStratDistNoRoot(&route->path->waypointlist[way1],strat);

					if (targettowaydist < strattowaydist)
					{
						GetPrevWay(strat);
						return(0);
					}
					else
						return(1);

				}
				else
				{

					return(1);
				
				}
			}
		}
	}
}




#define PLAYER 0
#define TARGET 1


/*GETS THE NEAREST CAMPING POINT TO A STRAT THAT A STRAT CAN SEE AND IS UNOCCUPIED*/
//MODES ARE 0 FOR NEAREST CAMP TO PLAYER AND 1 FOR NEAREST CAMP TO TARGET STRAT


int GetNearestCampPoint(STRAT *strat, int mode)
{
	float lastdist,dist;
	int	way1,way;
	short box;
	Point3 WayVec;
	Vector3 Pos;
	MAPAREA *Area;

	
	if (!strat->route)
		return(0);

	box = strat->route->path->area;
	

	if (box < 0)
		return(0);

	Area = &MapAreas[box];

	lastdist=10000000.0f;

	way1 = -1;

	//TO STOP TRYING TO MOVE TO A NON-EXISTANT TARGET
	if ((mode) && (!strat->target))
		return(0);

	ClearBusy(strat);

	for (way=0;way<Area->numcamppoints;way++)
	{
		#if BUSYCHECK
			/*if busy and not my own node then don't check */
	  		if (Area->camppoints[way].busy)
				continue;
			
			/*		if ((route->path->net->waybusy[way]) && (way != route->curway))
	   	  		continue; */
	   	#endif

			Pos.x = Area->camppoints[way].pos.x;
			Pos.y = Area->camppoints[way].pos.y;

		 	if (!PositionVisible(strat,&strat->pos,&Pos))
		 		continue;

	  
			
			WayVec.x = Area->camppoints[way].pos.x;
			WayVec.y = Area->camppoints[way].pos.y;
		
			if (!mode)
				dist = WayToPlayerDistNoRoot(&WayVec);
			else
				 dist = WayToStratDistNoRoot(&WayVec,strat->target);

		  	if ((dist < lastdist))
		  	{
		  		lastdist = dist;
		  		way1 = way;
		  	}
	}

	/*IF NO FREE CAMPPOINT FOUND THEN RETURN FALSE*/
	if (way1 <0)
		return(0);

	


	Area->camppoints[way1].busy = 1;
	strat->route->camped = way1;
		
	strat->CheckPos.x = Area->camppoints[way1].pos.x;  
	strat->CheckPos.y = Area->camppoints[way1].pos. y;  
	strat->CheckPos.z = strat->pos.z;  
		
	return(1);	
}

/*gets the nearest way to the strat on its path */

int GetNearestWayToMe(STRAT *strat)
{
	float lastdist,dist;
	int	way1,way;
	ROUTE *route = strat->route;
	lastdist=10000.0f;

	way1=route->curway;
	

	if (route->path->waytype == NETWORK)
	{
		for (way=0;way<route->path->numwaypoints;way++)
		{
	   
			#if BUSYCHECK
/*				if ((strat->path->net->waybusy[way]) && (way != strat->curway)) */
		   /*		if ((route->path->net->waybusy[way]) && (way != route->curway))
					continue;*/
			#endif

				dist = WayToStratDistNoRoot(&route->path->waypointlist[way],strat);
				if (dist < lastdist)
				{
					lastdist = dist;
					way1 = way;
				}
		}

		#if BUSYCHECK
			ClearBusy(strat);
			route->curway = way1;	
			route->path->net->waybusy[route->curway] = 1;
		#else
			route->curway = way1;	
		#endif
	}
	else
	{
		for (way=0;way<route->path->numwaypoints;way++)
		{
	   
			dist = WayToStratDistNoRoot(&route->path->waypointlist[way],strat);
			if (dist < lastdist)
			{
				lastdist = dist;
				way1 = way;
			}
		}
	}

	/*strat->curway=way1; */
/*	strat->CheckPos.x = strat->path->waypointlist[strat->curway].x;   */
/*	strat->CheckPos.y = strat->path->waypointlist[strat->curway].y;   */
 /*	strat->CheckPos.z = strat->path->waypointlist[strat->curway].z;   */
	strat->CheckPos.x = route->path->waypointlist[route->curway].x;  
	strat->CheckPos.y = route->path->waypointlist[route->curway].y;  
	strat->CheckPos.z = route->path->waypointlist[route->curway].z;  
	return(0);
}

/*GETS THE NEAREST WAY TO THE STRAT THAT IT CAN SEE */

int GetNearestVisibleWayToMe(STRAT *strat,float ang)
{
	float lastdist,dist;
	int	way1,way;
	ROUTE *route=strat->route;
	lastdist=100000.0f;

/*	way1=strat->curway; */
	way1=route->curway;
	

/*	for (way=0;way<strat->path->numwaypoints;way++) */
	for (way=0;way<route->path->numwaypoints;way++)
	{
	   
	  /*	dLog("x %f\n",strat->path->waypointlist[way].x); */
	   /*	if (strat->path->waypointlist[way].x == 176.539978) */
	   /*		way1 = way1; */
/*		if (SeePointZ(strat,&strat->path->waypointlist[way],ang)) */
/*		if (SeePointZ(strat,&route->path->waypointlist[way],ang)) */
	 	if (!LineSightPos(strat,&strat->pos,&route->path->waypointlist[way]))
	 			continue;
		/*if (SeePointZ(strat,&route->path->waypointlist[way],ang))
		{ */
	   
/*			dist = WayToStratDistNoRoot(&strat->path->waypointlist[way],strat); */
			dist = WayToStratDistNoRoot(&route->path->waypointlist[way],strat);
			if (dist < lastdist)
			{
				lastdist = dist;
				way1 = way;
			}
	   /*	} */
	}

	route->curway=way1;
	strat->CheckPos.x = route->path->waypointlist[route->curway].x;  
	strat->CheckPos.y = route->path->waypointlist[route->curway].y;  
	strat->CheckPos.z = route->path->waypointlist[route->curway].z;  
	return(0);
}

//ON SPLINE WILL ATTEMPT TO BACKUP TO FURTHEST POINT AWAY FROM THE PLAYER
//IF WE'RE NEAR THAT POINT ALREADY, THEN IT WILL RETURN 1
//ELSE 0
// 2D ONLY AT THE MOMENT

//MODE 0 - WILL CONSIDER POINT AND SEGMENTS WE ARE ON
//MODE 1 - PICKS FROM THE WHOLE PATH
//MODE 2 - GIVEN CURWAY WILL PICK CURWAY -1 OR + 1, WHICHEVER IS FURTHEST
//MODE 3 - RANDOM POINT PICK

int GetFurthestWay(STRAT *strat,int ConsiderAllPoints)
{
	float dist1,dist2,dist3;
	float lastdist = 100000.0f;
	float line1x, line1y, line2x, line2y, a,b,c;
	float distfromprevsegment,distfromnextsegment;
	
	int	way1,way2;

	ROUTE *route=strat->route;

	//IF NETWORK MODE THEN SET 'CONSIDER ALL POINTS' TRUE
	if ((route->path->waytype == NETWORK) && (ConsiderAllPoints != 3))
		ConsiderAllPoints = 1;

	if (ConsiderAllPoints)
	{
		lastdist = 0;

		switch (ConsiderAllPoints)
		{

			case (2):
				//picks the furthest from way -1 and way + 1
			
				//get the point previous to mine
				way1 = route->curway-1;

				//first way check
				if (way1 < 0)
					way1 = route->path->numwaypoints - 1;
				//get point following mine
				way2 = strat->route->curway + 1;

				//last way check
				if (way2 > route->path->numwaypoints - 1)
					way2 = route->path->numwaypoints - 1;

				dist1 = WayToPlayerDistXYNoRoot(&route->path->waypointlist[way1]);
				dist2 = WayToPlayerDistXYNoRoot(&route->path->waypointlist[way2]);

				if (dist2 > dist1)
					way1 = way2;
				break;
			case (1):
				lastdist = 0;
				//get the CURRENT point
				way1 = route->curway;

				for (way2 = 0; way2 < route->path->numwaypoints;way2++)
				{
				  	dist1 = WayToPlayerDistXYNoRoot(&route->path->waypointlist[way2]);
					
					if (dist1 > lastdist)
					{
					 	lastdist = dist1;
						way1 = way2;
					}
				}
				break;
			case (3):
				//pick a random point
				way1 = (int)(RandRange(0,(float)route->path->numwaypoints-1));
				break;
		}


	}
	else
	{
   	way1 = strat->route->curway;
	//between two segments ?

	if ((way1) && (way1 < route->path->numwaypoints - 1))
	{
		//if we are on the point, then consider routes way1 - 1 and way1 + 1
		if (NearCheckPosXY(strat,0.0))
		{
		  	dist1 = WayToPlayerDistXYNoRoot(&route->path->waypointlist[way1 + 1]);
		  	dist2 = WayToPlayerDistXYNoRoot(&route->path->waypointlist[way1 - 1]);
		  	dist3 = WayToPlayerDistXYNoRoot(&route->path->waypointlist[way1]);
		   	
			if ((dist3 < dist2) || (dist3 < dist1))
			{
				if (dist1 < dist2)
		  			way1 = way1 - 1;
				else
					way1 = way1 + 1;
			}
		}
		else
		{
			//find what segment of the curve way1 - 1, way1, way1 + 1
			//we are on

			line1x = strat->CheckPos.x;
			line1y = strat->CheckPos.y;
			line2x = route->path->waypointlist[way1-1].x;
			line2y = route->path->waypointlist[way1-1].y;
			a = line1x - line2x;
			b = line1y - line2y;
			c = -(a * line1x + b * line1y);
			distfromprevsegment = (a * strat->pos.x + b * strat->pos.y + c) /
									(sqrt)( a* a + b*b);

			line2x = route->path->waypointlist[way1+1].x;
			line2y = route->path->waypointlist[way1+1].y;
			a = line1x - line2x;
			b = line1y - line2y;
			c = -(a * line1x + b * line1y);
			distfromnextsegment = (a * strat->pos.x + b * strat->pos.y + c) /
									(sqrt)( a* a + b*b);

			if (distfromnextsegment < distfromprevsegment)
			{
				//consider way and way + 1
				//pick one furthest from player
				dist1 = WayToPlayerDistXYNoRoot(&route->path->waypointlist[way1]);
				dist2 = WayToPlayerDistXYNoRoot(&route->path->waypointlist[way1 + 1]);
				
				//SEE IF PLAYER IS NEARER TO WAY + 1 THAN STRAT	IS
				//IF SO, WE DON'T WANT TO HEAD DOWN THERE
							
				if (dist1 < dist2)
				{
				 	dist1 = WayToStratDistNoRoot(&route->path->waypointlist[way1 + 1],strat);

					if (dist1 < dist2)
						way1 = way1 + 1;
				}
			
			}
			else
			{
				//consider way and way - 1
				//pick one furthest from player
				dist1 = WayToPlayerDistXYNoRoot(&route->path->waypointlist[way1]);
				dist2 = WayToPlayerDistXYNoRoot(&route->path->waypointlist[way1 - 1]);
				//SEE IF PLAYER IS NEARER TO WAY - 1 THAN STRAT	IS
				//IF SO, WE DON'T WANT TO HEAD DOWN THERE
							
				if (dist1 < dist2)
				{
					
				 	dist1 = WayToStratDistNoRoot(&route->path->waypointlist[way1 - 1],strat);

					if (dist1 < dist2)
					way1 = way1 - 1;
		
				}
			}
		}
	}
	else
	{
		//THE LAST OR FIRST
		if (way1)
		{
			//the last waypoint
			dist1 = WayToPlayerDistXYNoRoot(&route->path->waypointlist[way1]);
			dist2 = WayToPlayerDistXYNoRoot(&route->path->waypointlist[way1 - 1]);
		   	//SEE IF PLAYER IS NEARER TO WAY - 1 THAN STRAT	IS
		   	//IF SO, WE DON'T WANT TO HEAD DOWN THERE
							
		  	if (dist1 < dist2)
		  	{
			   	dist1 = WayToStratDistNoRoot(&route->path->waypointlist[way1 - 1],strat);

			   	if (dist1 < dist2)
					way1 = way1 - 1;
		  	}
			//WE ARE GOING TO WAY1, UNLESS THE PLAYER IS NEARER TO IT THAN WE ARE
			else
			{
			   	dist2 = WayToStratDistNoRoot(&route->path->waypointlist[way1],strat);


				if (dist1 < dist2)
					way1 = way1 - 1;


			}

		}
		else
		{
		   	//the first point
			//consider way and way + 1
			//pick one furthest from player
			dist1 = WayToPlayerDistXYNoRoot(&route->path->waypointlist[way1]);
			dist2 = WayToPlayerDistXYNoRoot(&route->path->waypointlist[way1 + 1]);
		  	if (dist1 < dist2)
		  	{
				dist1 = WayToStratDistNoRoot(&route->path->waypointlist[way1 + 1],strat);

			   	if (dist1 < dist2)
					way1 = way1 + 1;
		  	}
			//ELSE WE ARE GOING TO FIRST WAY UNLESS THE PLAYER IS NEARER TO IT THAN WE ARE
			else
			{
				dist2 = WayToStratDistNoRoot(&route->path->waypointlist[way1],strat);

				if (dist1 < dist2)
					way1 = way1 + 1;

			}

		}


	}
	}

	if (route->path->waytype == NETWORK)
	{
		if (way1!=route->curway)
		{
			ClearPath(strat,way1);

			way1=GetNextPath(strat,route->curway,way1);
			if (way1 < 0)
				return(1);

		}

	}


	strat->route->curway = way1;
	strat->CheckPos.x = route->path->waypointlist[strat->route->curway].x;
	strat->CheckPos.y = route->path->waypointlist[strat->route->curway].y;

   if (NearCheckPosXY(strat,0.0))
		return(1);
	else
		return(0);




}

//ON SPLINE WILL ATTEMPT TO GO TO NEAREST POINT TO THE PLAYER
//IF WE'RE NEAR THAT POINT ALREADY, THEN IT WILL RETURN 1
//ELSE 0
// 2D ONLY AT THE MOMENT
int GetNearestWaySpline(STRAT *strat,float distance)
{
	float dist1,dist2;
	float lastdist = 100000.0f;
	float line1x, line1y, line2x, line2y, a,b,c;
	float distfromprevsegment,distfromnextsegment;
	int	way1,way2,oldway;
	ROUTE *route=strat->route;

	oldway = way1 = strat->route->curway;
	//between two segments ?

	if ((way1) && (way1 < route->path->numwaypoints - 1))
	{
		//if we are on the point, then consider routes way1 - 1 and way1 + 1
		if (NearCheckPosXY(strat,0.0))
		{
		  	dist1 = WayToPlayerDistXYNoRoot(&route->path->waypointlist[way1 + 1]);
		  	dist2 = WayToPlayerDistXYNoRoot(&route->path->waypointlist[way1 - 1]);
		  	if (dist1 > dist2)
		  		way1 = way1 - 1;
			else
				way1 = way1 + 1;

		}
		else
		{
			//find what segment of the curse way1 - 1, way1, way1 + 1
			//we are on

			line1x = strat->CheckPos.x;
			line1y = strat->CheckPos.y;
			line2x = route->path->waypointlist[way1-1].x;
			line2y = route->path->waypointlist[way1-1].y;
			a = line1x - line2x;
			b = line1y - line2y;
			c = -(a * line1x + b * line1y);
			distfromprevsegment = (a * strat->pos.x + b * strat->pos.y + c) /
									(sqrt)( a* a + b*b);

			line2x = route->path->waypointlist[way1+1].x;
			line2y = route->path->waypointlist[way1+1].y;
			a = line1x - line2x;
			b = line1y - line2y;
			c = -(a * line1x + b * line1y);
			distfromnextsegment = (a * strat->pos.x + b * strat->pos.y + c) /
									(sqrt)( a* a + b*b);

			if (distfromnextsegment < distfromprevsegment)
			{
				//consider way and way + 1
				//pick one furthest from player
				dist1 = WayToPlayerDistXYNoRoot(&route->path->waypointlist[way1]);
				dist2 = WayToPlayerDistXYNoRoot(&route->path->waypointlist[way1 + 1]);
				if (dist1 > dist2)
					way1 = way1 + 1;
			
			}
			else
			{
				//consider way and way -1
				//pick one furthest from player
				dist1 = WayToPlayerDistXYNoRoot(&route->path->waypointlist[way1]);
				dist2 = WayToPlayerDistXYNoRoot(&route->path->waypointlist[way1 - 1]);
				if (dist1 > dist2)
					way1 = way1 - 1;
		
			}
		}
	}
	else
	{
		//THE LAST OR FIRST
		if (way1)
		{
			//the last waypoint
			dist1 = WayToPlayerDistXYNoRoot(&route->path->waypointlist[way1]);
			dist2 = WayToPlayerDistXYNoRoot(&route->path->waypointlist[way1 - 1]);
			if (dist1 > dist2)
				way1 = way1 - 1;

		}
		else
		{
		   	//the first point
			//consider way and way + 1
			//pick one furthest from player
			dist1 = WayToPlayerDistXYNoRoot(&route->path->waypointlist[way1]);
			dist2 = WayToPlayerDistXYNoRoot(&route->path->waypointlist[way1 + 1]);
			if (dist1 > dist2)
				way1 = way1 + 1;


		}


	}

	strat->route->curway = way1;
	strat->CheckPos.x = route->path->waypointlist[strat->route->curway].x;
	strat->CheckPos.y = route->path->waypointlist[strat->route->curway].y;



	if (NearCheckPosXY(strat,0.0))
		return(1);
	else
		return(0);


/*	if (oldway == way1)
		return(1);
	else
		return(0);
*/

}













int	FirstWay(STRAT *strat)
{
	SplineData	*sd;
	float radius;
	ROUTE *route = strat->route;
	if (strat->route->splineData)
	{
		if (strat->pnode)
			radius = strat->pnode->radius;
		else
			radius = 1.0f;

		sd = strat->route->splineData;
	   
		//if stopped, and on the first segment, then we must have reached the first waypoint
		if ((sd->lineSeg <=0) && (pSqrDist(&route->path->waypointlist[0], &strat->pos) < radius))
			return 1;
		else
			return 0;
	}
	if (strat->route->curway == 0)
		return 1;
	else
		return 0;
}

int	LastWay(STRAT *strat)
{
	ROUTE *route = strat->route;
	int numpathpoints;
	int near;
	float radius;
	SplineData	*sd;

  
	
	if (strat->route->splineData)
	{
		if (route->path->waytype == NETWORK)
			numpathpoints = route->path->net->patrol[route->patrolnumber].numpathpoints;
		else  
			numpathpoints = route->path->numwaypoints;
		
		sd = strat->route->splineData;
	  
		if (!(strat->pnode))
			radius = 1.0;
		else
			radius = strat->pnode->radius;

		//if we are stopped and on the last line segment, then we must of hit the last waypoint
	  	if ((sd->lineSeg == numpathpoints -2) && (pSqrDist(&route->path->waypointlist[route->path->numwaypoints - 1], &strat->pos) < radius))
			return 1;
		else
			return 0;
	}
/*	if (strat->path->waytype == NETWORK) */
	if (route->path->waytype == NETWORK)
	{

/*		if (strat->curway == strat->path->net->patrol[strat->patrolnumber].numpathpoints-1) */
		if (route->curway == route->path->net->patrol[route->patrolnumber].numpathpoints-1)
			return 1;
		else
			return 0;

	}
	else
	{
	
/*		if (strat->curway == strat->path->numwaypoints-1) */
		if (route->curway == route->path->numwaypoints-1)
			return 1;
		else
			return 0;
	}
}




/* WAYPOINT CODE WAREZ */

//given a 'next' point to head to, this ensures we can reach it from the start point
//otherwise a candidate legally connected node is found

static int CheckNext(NET *net,int start, int next)
{
	int i;
	int found = 0;
	int connect,candidate;
	for (i=0;i<4;i++)
	{
		connect = ((net->connectors[start] >> (SHIFTS[i]))&255);
		if (connect)
		{

		 	connect = connect-1;
			if (connect == next)
				found++;
			else
				candidate = connect;
		}
	}

	if (found)
		return(next);
	else
		return(candidate);





}



/* Sets the initial path to follow */
/*IF CALLING STRAT HAS SET GLOBAL PARAM INT 0 TO BE = 99, THE STRAT WILL NOT BE PLACED
  ON THE CHECKPOINT AND ROUTE->PATH WILL JUST BE SET UP*/
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
   //	route->curway=0;
	route->curway=start;

	
	if (GetGlobalParamInt(0) != 99)
	{
	   	//SETS THE START NODE TO BE 0, FOR CONVEYOR TYPE STRATS
		if (GetGlobalParamInt(0) == 98)
		{
			route->pathstartnode = 0;
			start=route->pathstartnode;
			SetGlobalParamInt(0, 0);
		}

		//ARE WE ON A SIMPLE PATH, OR GIVEN FREE REIGN OVER A NETWORK ?
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
		   	//MUST BE ON A NETOWRK PATROL ROUTE
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

		//ORIENTATION, IF REQUESTED
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

/*set the current waypoint to be equal to the one we are referencing within */
/*our fixed path */
void LeaveFixedPath(STRAT *strat)
{
   	ROUTE *route = strat->route;
	if (route->path->waytype == NETWORK)
	{
		route->curway = route->path->net->patrol[route->patrolnumber].initialpath[route->curway];
		route->lastway = route->curway;
		ClearBusy(strat);
		strat->flag2 &= LNOT(STRAT2_ONPATROLROUTE);
	}
}


void MoveTowardCheckPosXY(STRAT *strat, float amount)
{
	Vector3 stratToCheckPos;

	v3Sub(&stratToCheckPos, &strat->CheckPos, &strat->pos);
  	stratToCheckPos.z = 0.0f;
	v3Normalise(&stratToCheckPos);
	v3Scale(&stratToCheckPos, &stratToCheckPos, amount);
	v3Inc(&strat->vel, &stratToCheckPos);
}

void MoveTowardCheckPos(STRAT *strat, float amount)
{
	Vector3 stratToCheckPos;

	v3Sub(&stratToCheckPos, &strat->CheckPos, &strat->pos);
	v3Normalise(&stratToCheckPos);
	v3Scale(&stratToCheckPos, &stratToCheckPos, amount);
	v3Inc(&strat->vel, &stratToCheckPos);
}

void MoveTowardCheckPosNow(STRAT *strat, float amount)
{
	Vector3 stratToCheckPos;

	v3Sub(&stratToCheckPos, &strat->CheckPos, &strat->pos);
	v3Normalise(&stratToCheckPos);
	v3Scale(&stratToCheckPos, &stratToCheckPos, amount);
	strat->vel = stratToCheckPos;
}



void MoveTowardPlayerXY(STRAT *strat, float amount)
{
	Vector3 stratToPlayer;

	v3Sub(&stratToPlayer, &player[currentPlayer].Player->pos, &strat->pos);
	stratToPlayer.z = 0.0f;
	v3Normalise(&stratToPlayer);
	v3Scale(&stratToPlayer, &stratToPlayer, amount);
	v3Inc(&strat->pos, &stratToPlayer);
}
