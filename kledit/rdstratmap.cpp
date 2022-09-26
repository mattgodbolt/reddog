/*
 * RDStratMap.cpp - the strat map file format
 */

#include "stdafx.h"
#include <process.h>
#include "modstack.h"
#include "camcontrol.h"
#include "camppoint.h"

//#define ARTIST 0
//#define SH4 0
#define COLLAPSED -1

int Valid=0;
int ALLMAPMODE=1;
IScene *GlobalScene;
FILE *godlog;

#if ARTIST
	int GDSRUN = 1;
	int	CODERUN = 1;
#else
	int GDSRUN = 0;
	int	CODERUN = 0;
#endif


enum {ACTIVATION,WAYPOINTS,TRIGGERS,ALL};
enum {PARSE1,PARSE2,PARSE3,PARSE3A, PARSE4, LOGBOX, LEVELVARS, CAMERAMODS,MAPCHECK,EXCLUSION};
//enum {PATH, NETWORK};
int Mode = PARSE1;		//export mode flag


#define MAXSTRATS   1024
INode *Strats[MAXSTRATS];
int  NumStrats;

#define MAXPATHS	512
INode *MapPaths[MAXPATHS];
INode *TRIGNODE=NULL;

#define MAXEVENTS	512

#define MAXTRIGS	512

#define MAXATTACHTRIGSTRAT 64

typedef struct eventstruct
{
	unsigned char flag;
   	short   timer;
	INode   *node;
	int		stratnum;
	short	assigned[MAXATTACHTRIGSTRAT];	//EACH TRIGGER MAY HAVE 32 STRATS ATTACHED TO IT
}EVENTSTRUCT;

EVENTSTRUCT MapEvents[MAXEVENTS];
int NumEvents;


INode *MapTrigs[MAXTRIGS];
int AttachedTrigs[MAXTRIGS]; //NUMBER OF STRATS ATTACHED TO TRIG MAPTRIGS[n]
//TRIGSTRUCT	AssignedStrats[MAXTRIGS];	//STRATS ASSIGNED TO EACH TRIGGER



#define MAXACTS	256
INode *MapActs[MAXACTS];	//ACTIVATION POINTS

#define MAXASSOCIATES	512
INode *LinkedAssociates[MAXASSOCIATES];
int LinkedAssociated;
INode *UnlinkedAssociates[MAXASSOCIATES];
int UnlinkedAssociated;
int FoundAssociates;
INode *NonConnects[MAXASSOCIATES];
int NonConnected;
int MapVarsSet;
int CamMods;


IScene *Scene;
//int MakeWay(Interface *maxInterface,INode *way,int startnode);

typedef struct maparea
{
	INode *area;
	INode *subarea;
} MAPAREA;

MAPAREA MapAreas[MAXPATHS];

typedef struct netnode
{
	Point3 pt;
	int connectednum;
	int connected[16];
	int collapsed;
	int mask;
	int selected  ;
} NETNODE;

int	NumArea = 0;
int NumPath = 0;
int NumTrig = 0;
int NumAct = 0;
int EntNum = 0;
int NumLights = 0;

#define OUTSIDE 0
#define INSIDE  1

#define MAX_EXCLUSIONS 16
INode *ExclusionZone[MAX_EXCLUSIONS];
int NumExclusionZones = 0;

 int NodePointCompare(Point3 *src, Point3 *src2)
{
	double x,y,z;

	if ((src->x == src2->x) && (src->y == src2->y) && (src->z == src2->z))
		int error = 1;

	//TO COVER ANY INACCURACY WHEN PLACING PATH OVER GRID
	x = fabs(src->x - src2->x);
	y = fabs(src->y - src2->y);
	z = fabs(src->z - src2->z);
										
	if ((x < 0.0001) && (y < 0.0001) && (z < 0.0001))
		return(1);
	else
		return(0);
}
	void ReplaceRef(NETNODE *nodes,int loop, int innerloop, int MAX)
	{
		for (int node = 0;node <MAX;node++)
	   	{

			for (int connect =0; connect < nodes[node].connectednum;connect++)
			{
				int trial = nodes[node].connected[connect];
				if (trial == innerloop)
					nodes[node].connected[connect] = loop;

			}

		}

	}

	int TrueDupFound(NETNODE *nodes,int innerloop,int MAX)
	{
		for (int loop = 0;loop<MAX;loop++)
		{
			if (loop != innerloop)
			{
				if (NodePointCompare(&nodes[loop].pt,&nodes[innerloop].pt))
				{
					//replace references to innerloop, with references to loop
					ReplaceRef(nodes,loop,innerloop,MAX);

				}


			}

			return(0);

		}


	}



int InsideSpline(Point3 *Pos, PolyShape *splinearea)
{
	int   numpoints = splinearea->lines->numPts,i;
	PolyPt SegmentPointA,SegmentPointB;
	float	x,y;
	int CollideSegment=0;

	x=Pos->x;
	y=Pos->y;
	
	for (i=0;i<numpoints;i++)
	{
		SegmentPointA=splinearea->lines->pts[i];
		
		/*CHECK FOR LAST SEGMENT */
		if (i==numpoints-1)
			SegmentPointB = splinearea->lines->pts[0];
		else
			SegmentPointB = splinearea->lines->pts[i+1];

		if ((((SegmentPointB.p.y <= y) && (y < SegmentPointA.p.y)) ||
			((SegmentPointA.p.y <= y) && (y < SegmentPointB.p.y))) &&
			(x < (SegmentPointA.p.x - SegmentPointB.p.x) * (y - SegmentPointB.p.y) /
			(SegmentPointA.p.y - SegmentPointB.p.y) + SegmentPointB.p.x))
			CollideSegment = !CollideSegment;	

	}
	
	/*AN EVEN AMOUNT OF COLLISIONS AND WE'RE OUTSIDE THE REGION */
	if (!CollideSegment)
		return(OUTSIDE);
	else
		return(INSIDE);
}




int InsideArea(INode *node)
{	int i;


	Matrix3 strattrans = node->GetNodeTM(GetCOREInterface()->GetTime());
	Point3	stratpos = strattrans.GetTrans();

	for (i=0;i<NumExclusionZones;i++)
	{

		//GET THE NEXT EXCLUSION NODE
		Object *obj = ExclusionZone[i]->GetObjectRef();
		//CAST IT INTO A SHAPE
		ShapeObject* shape = (ShapeObject*)obj;
		PolyShape pShape;
		shape->MakePolyShape(GetCOREInterface()->GetTime(), pShape);
		int numPts = pShape.lines->numPts;
		PolyPt *pt = pShape.lines->pts;
	  	//PULL OUT THE ROTATION MATRIX AND TRANSFORM OF THE EXCLUSION NODE
			
		Matrix3 transform = ExclusionZone[i]->GetNodeTM(GetCOREInterface()->GetTime());
		Point3	trans = transform.GetTrans();
   		Point3  rot;		
						
		for (int points=0;points<numPts;points++,pt++)
		 	//APPLY TRANFORMATION TO THE AREAPOINT AND OUTPUT		
			//NOTE 2D AT THE MOMENT SO NO HEIGHT "Z" COMPONENT
			pt->p = pt->p * transform;

		//CHECK FOR INTERSECTION BETWEEN THE GIVEN STRAT'S POINT AND THE BOX

		if (InsideSpline(&stratpos,&pShape))
		{

			TCHAR *name = node->GetName();
			int exclusionbox = i;
		}
		pShape.NewShape();

	}


	return(1);

}




class NullView: public View {
	public:
		Point2 ViewToScreen(Point3 p) { return Point2(p.x,p.y); }
		NullView() { worldToView.IdentityMatrix(); screenW=640.0f; screenH = 480.0f; }
	};


int GetEvent(INode *event)
{
	for (int i=0;i<NumEvents;i++)
	{
	 	if (MapEvents[i].node == event)
			return(i);
	}
	return (0);
}





//IF A STRAT IS LINKED TO AN EVENT, THIS WILL RETURN THE EVENT NUMBER


int	EventAttached(RDObject *ref)
{

	INode *node = ref->GetMyNode();

	INode *parent = node->GetParentNode();
	if (!parent)
		int fuck = 1;
	if (parent)
	{
		TCHAR array1[128];

		TCHAR *name = parent->GetName();

		TCHAR* ArrPt = name;

		//GET RID OF LEADING SPACES

		 while (*ArrPt == ' ')
		 	ArrPt++;

		 sscanf(ArrPt,"%s ",&array1);

		 if (!(strnicmp(array1,"EVENT",5)))
			return (GetEvent(parent));
	//		return(1);
	}
	return(-1);
   //	return (0);
}

//IF A STRAT IS LINKED TO A DELETION EVENT, THIS WILL RETURN THE EVENT NUMBER


int	DeletionEventAttached(RDObject *ref)
{

	INode *node = ref->GetMyNode();

	INode *parent = ref->deletioneventpoint;

	if (parent)
	{
		TCHAR array1[128];

		TCHAR *name = parent->GetName();

		TCHAR* ArrPt = name;

		//GET RID OF LEADING SPACES

		 while (*ArrPt == ' ')
		 	ArrPt++;

		 sscanf(ArrPt,"%s ",&array1);

		 if (!(strnicmp(array1,"EVENTDELETE",11)))
			return (GetEvent(parent));
	//		return(1);
	}
	return(-1);
   //	return (0);
}

	
float ComputeScaledRadius(float radius, Matrix3 *transform)
{

	Point3 row1 = transform->GetRow(0);

	Point3 row2 = transform->GetRow(1);

	Point3 row3 = transform->GetRow(2);

	//APPLY TRANFORMATION TO THE RADIUS		
				
	Point3 tvec,nradius;


	//APPLY TRANFORMATION TO THE WAYPOINT AND OUTPUT		
				
	nradius.x = radius;
	nradius.y = 0;
	nradius.z = 0;

	tvec.x =
		 	(nradius.x * row1.x) +
			(nradius.y * row1.y) +
			(nradius.z * row1.z);
	tvec.y = 
			(nradius.x * row2.x) +
			(nradius.y * row2.y) +
			(nradius.z * row2.z);
	tvec.z = 
		  	(nradius.x * row3.x) +
			(nradius.y * row3.y) +
			(nradius.z * row3.z);

	return( (float)sqrt((tvec.x * tvec.x) + (tvec.y * tvec.y) + (tvec.z*tvec.z)));

}



int	BuildInitialPath(Interface *maxInterface,INode *node,Point3 *path)
{

	Matrix3 transform = node->GetObjectTM(maxInterface->GetTime());
	Object *obj = node->EvalWorldState(maxInterface->GetTime()).obj;
	ShapeObject* shape = (ShapeObject*)obj;
	PolyShape pShape;
	shape->MakePolyShape(maxInterface->GetTime(), pShape);
	int numPts = pShape.lines->numPts;
	PolyPt *pt = pShape.lines->pts;

	for (int points=0;points<numPts;points++,pt++)
	{
		*(path+points) = pt->p * transform;

	}
	pShape.NewShape();

	return(points);
}


int AddStrat(INode *strat)
{
	int found=0;
	if (!strat)
		return(-1);

	for (int i=0;i<NumStrats;i++)
   	{
		if (Strats[i] == strat)
	 		return(i);
   	}

	//NEW STRAT

	InsideArea(strat);

	Strats[i] = strat;
	NumStrats++;
	return(-1);
}


void CheckForExclusion(INode *node)
{
	TCHAR *name = node->GetName();

	if (!strnicmp(name,"STRATEXCLUSION",14))
	{
		 ExclusionZone[NumExclusionZones] = node;	
		 NumExclusionZones++;	
	}


}

/*
 * The saving enumerator
 */
class RDStratExportEnumerator : public ITreeEnumProc {
private:
	// The interface to max
	Interface *maxInterface;
	FILE *fp;
	Point3					scale;
	Point3					rotation;
	Point3					translation;
public:

	RDStratExportEnumerator (IScene *scene, Interface *i, FILE *fp) 
	{
		assert (i != NULL);
		// Set up the variables
		maxInterface = i;
//		if (fp)  i need this to be nulled when fp is passed as null - mattG - any probs with this?
			this->fp = fp;
		scene->EnumTree (this);
	}
	
   
	void LookForName(INode *node,INode *Base,INode *SourceError,int Mode)
	{
		int i,numchild;
		INode *oldnode=node;
		
		numchild = node->NumberOfChildren();

		if (node == Base)
			return;
		
		TCHAR *name = node->GetName();
		TCHAR *BaseName = Base->GetName();

		if (strlen(BaseName) == strlen(name))
		{
			if (!strncmp(name,BaseName,strlen(BaseName)))
			{
				switch (Mode)
				{
					char error[128];
					case WAYPOINTS:
						if (ALLMAPMODE < GODMODEINTERMEDIATE)
						{	
							sprintf(error,"STRAT %s WARNING ** WAYPATH SPLINE USED THAT HAS A DUPLICATE MAX OBJECT NAME\n",SourceError->GetName());
	   						MessageBox (NULL, error,Base->GetName(),MB_OK);
						}
						else
							fprintf(godlog,"STRAT %s WARNING ** ACTIVATION CIRCLE USED THAT HAS A DUPLICATE MAX OBJECT NAME\n",SourceError->GetName());
						break;
					case TRIGGERS:
					  	if (ALLMAPMODE < GODMODEINTERMEDIATE)
						{	
							sprintf(error,"STRAT %s WARNING ** TRIGGER CIRCLE USED THAT HAS A DUPLICATE MAX OBJECT NAME\n",SourceError->GetName());
	   						MessageBox (NULL, error,Base->GetName(),MB_OK);
						}
						else
							fprintf(godlog,"STRAT %s WARNING ** ACTIVATION CIRCLE USED THAT HAS A DUPLICATE MAX OBJECT NAME\n",SourceError->GetName());
						break;
					case ACTIVATION:
					  	if (ALLMAPMODE < GODMODEINTERMEDIATE)
						{	
							sprintf(error,"STRAT %s WARNING ** ACTIVATION CIRCLE USED THAT HAS A DUPLICATE MAX OBJECT NAME\n",SourceError->GetName());
	   						MessageBox (NULL, error,Base->GetName(),MB_OK);
						}
						else
							fprintf(godlog,"STRAT %s WARNING ** ACTIVATION CIRCLE USED THAT HAS A DUPLICATE MAX OBJECT NAME\n",SourceError->GetName());
						break;
					case ALL:
	   					if (ALLMAPMODE < GODMODEINTERMEDIATE)
							MessageBox (NULL, "WARNING ** DUPLICATE MAX OBJECT NAME FOUND",Base->GetName(), MB_OK);
						else
							fprintf(godlog,"WARNING ** DUPLICATE MAX OBJECT NAME FOUND %s \n", Base->GetName());
						break;
						
				}
			}
		}

		if (!numchild)
			return;

		for (i=0;i<numchild;i++)
		{
			node = oldnode->GetChildNode(i);
			LookForName(node, Base, SourceError, Mode);
		}

	}


	
	
	// A function called once per object to save it
	int callback (INode *node)
	{
		Object *obj = node->GetObjectRef();
	  
		if (!obj)
			return TREE_CONTINUE;
	  
		obj = obj->FindBaseObject();
	   //		EvalWorldState(maxInterface->GetTime()).obj;

		//PARSE MODE WE JUST WANT TO BUILD UP THE WAYPOINT LISTS FOR THE STRATS TO INDEX


		switch (Mode)
		{
			case (EXCLUSION):
				CheckForExclusion(node);
				break;	
			
			//A PARSE TO CHECK FOR DUPLICATE MAX NODE NAMES
			case (MAPCHECK):
				if (ALLMAPMODE < GODMODEINTERMEDIATE) 	
					LookForName(GetCOREInterface()->GetRootNode(),node, NULL, ALL);
				break;


			case (PARSE1):

				//if (obj->ClassID() == RDOBJECT_CLASS_ID)
				 //	ParseWay((RDObject*) obj);
				//Check each strat to see if it's associated with a waypath, trigger
 				//and activation point
				if (obj->ClassID() == RDOBJECT_CLASS_ID)
				{
					RDObject *RDObj = (RDObject*)obj;
					AddStrat(node);
					ParseWay(RDObj,node);
					ParseTrig(RDObj,node);
					ParseEvent(RDObj,node);
				   	ParseDeletionEvent(RDObj,node);
					ParseActs(RDObj,node);
				}
				if (obj->ClassID() == camcontrols_CLASS_ID)
					CamMods++;
				break;
			case (PARSE2):
				// Is it a Red Dog Object?
				// if so output to the map
				if (obj->ClassID() == RDOBJECT_CLASS_ID)
				{
					RDObject *RDObj = (RDObject *)obj;
					StratMapOutput(maxInterface,fp,RDObj,node);
					Valid++;						
				}
				break;
			case (PARSE3):
				// Is it a Red Dog Object?
				// if so output to the map
				if (obj->ClassID() == RDOBJECT_CLASS_ID)
				{
					RDObject *RDobj = (RDObject *)obj;
					if (RDobj->trigpoint==TRIGNODE)
						OutPutEntry(fp,EntNum);
					/*else
					{
						if (RDobj->eventpoint==TRIGNODE)
							OutPutEntry(fp,EntNum);
					} */
					EntNum++;
					Valid++;						
				}
				break;
			case (PARSE3A):
				// Is it a Red Dog Object?
				// if so output to the map
				if (obj->ClassID() == RDOBJECT_CLASS_ID)
				{
					RDObject *RDobj = (RDObject *)obj;
					if ((RDobj->eventpoint==TRIGNODE))
						//|| (RDobj->deletioneventpoint==TRIGNODE))
						OutPutEntry(fp,EntNum);
				   
					if (RDobj->deletioneventpoint == TRIGNODE)
					{
						TCHAR *name = RDobj->stratSource;
						int b = 5;

					}
					
					EntNum++;
					Valid++;						
				}
				break;

			case PARSE4: // Lighting pass
				if (!fp) {
					if (obj->ClassID() == Class_ID(OMNI_LIGHT_CLASS_ID, 0) ||
						obj->ClassID() == Class_ID(SPOT_LIGHT_CLASS_ID, 0) ||
						obj->ClassID() == Class_ID(FSPOT_LIGHT_CLASS_ID, 0))
						NumLights++;
				} else {
					Matrix3 transform = node->GetObjTMAfterWSM(0);
					if (obj->ClassID() == Class_ID(OMNI_LIGHT_CLASS_ID, 0))
						OutputOmniLight (fp, (GenLight *)obj, transform);
					else if (obj->ClassID() == Class_ID(SPOT_LIGHT_CLASS_ID, 0) ||
						obj->ClassID() == Class_ID(FSPOT_LIGHT_CLASS_ID, 0))
						OutputSpotLight (fp, (GenLight *)obj, transform);
				}
				break;
			case LOGBOX: // LOOKING FOR NONLINKED, BBOXES WITHIN A STRAT AREA
				RegisterBox(node);
				break;
			case LEVELVARS: // OUTPUT THE LEVEL VARS ASSOCIATED WITH THE MAP

				if (!MapVarsSet)
				{
					OutPutLevelVars(fp);
					MapVarsSet = 1;
				}
				break;

			case CAMERAMODS: // OUTPUT THE CAMERAMODS
				if (obj->ClassID() == camcontrols_CLASS_ID)
				{
			 		OutputCameraMods(fp,(camcontrol*)obj);
				}
				break;

		}
		return TREE_CONTINUE;
	}

	void RegisterBox(INode *node)
	{

		int i,found=0;
		TCHAR* nodename = node->GetName();	
		if (!(strnicmp(nodename,"BBox",4)))
		{
			for (i=0;i<LinkedAssociated;i++)
			{
				if (LinkedAssociates[i] == node)
					found++;
			}

			if (!found)
			{
				UnlinkedAssociates[UnlinkedAssociated] = node;
				UnlinkedAssociated++;

			}

		}
	}



	void DoCommonLightStuff (FILE *fp, GenLight *maxLight, Matrix3 &transform)
	{
		LightState state;
		maxLight->EvalLightState (0, FOREVER, &state);
		Point3 position = transform.GetTrans();
		fprintf (fp, "\tPos : %f %f %f\n", position.x, position.y, position.z);

		Point3 colour = maxLight->GetRGBColor (0);
		colour.x *= maxLight->GetIntensity(0);
		colour.y *= maxLight->GetIntensity(0);
		colour.z *= maxLight->GetIntensity(0);
		if (colour.x > 1.f)
			colour.x = 1.f;
		if (colour.y > 1.f)
			colour.y = 1.f;
		if (colour.z > 1.f)
			colour.z = 1.f;
		fprintf (fp, "\tColour : %d %d %d\n", int(colour.x * 255.f), int(colour.y * 255.f), int(colour.z * 255.f));
		fprintf (fp, "\tNear : %f\n", state.attenStart);
		fprintf (fp, "\tFar : %f\n", state.attenEnd);
		
	}

	void OutPutLevelVars(FILE *fp)
	{
		// LEVEL FLAGS
		//fprintf(fp,"MAPVARSSET: %d\n",1);
		fprintf(fp,"%s %d\n", "LEVELFLAGSSET:",1);


 		fprintf (fp, "MAP LEVEL VARS\n");
		fprintf (fp, "\tAMBIENT LIGHTING\n");
		fprintf (fp, "\t\tAMBIENT RED :%d\n",levelflag::GetLightValue(RED));
		fprintf (fp, "\t\tAMBIENT GREEN :%d\n",levelflag::GetLightValue(GREEN));
		fprintf (fp, "\t\tAMBIENT BLUE :%d\n",levelflag::GetLightValue(BLUE));
		fprintf (fp, "\t\tAMBIENT SCAPE INTENSITY : %f\n",levelflag::GetFloatValue(SCAPEINTENSITY));
		fprintf (fp, "\t\tAMBIENT STRAT INTENSITY : %f\n",levelflag::GetFloatValue(STRATINTENSITY));
		fprintf (fp, "\tFOG VALUES\n");
		fprintf (fp, "\t\tFOG RED :%d\n",levelflag::GetLightValue(FOGRED));
		fprintf (fp, "\t\tFOG GREEN :%d\n",levelflag::GetLightValue(FOGGREEN));
		fprintf (fp, "\t\tFOG BLUE :%d\n",levelflag::GetLightValue(FOGBLUE));
		fprintf (fp, "\t\tFOG NEAR : %f\n",levelflag::GetFloatValue(FOGNEAR));
		fprintf (fp, "\t\tFOG FAR : %f\n",levelflag::GetFloatValue(FOGFAR));
		fprintf (fp, "\tVIEW VALUES\n");
		fprintf (fp, "\t\tFOV : %f\n",levelflag::GetFloatValue(FOV));
		fprintf (fp, "\t\tFARZ : %f\n",levelflag::GetFloatValue(FARZ));
		fprintf (fp, "\t\tCAMHEIGHT : %f\n",levelflag::GetFloatValue(CAMHEIGHT));
		fprintf (fp, "\t\tCAMDIST : %f\n",levelflag::GetFloatValue(CAMDIST));
		fprintf (fp, "\t\tCAMLOOKHEIGHT : %f\n",levelflag::GetFloatValue(CAMLOOKHEIGHT));

		if (!(strnicmp(levelflag::name,"CHOOSE DOME",11)))
			fprintf (fp, "\t\tLEVEL DOME : %s\n","NO_DOME");
		else
			fprintf (fp, "\t\tLEVEL DOME : %s\n",levelflag::name);
		fprintf (fp, "END MAP LEVEL VARS\n");
	}

	void OutputCameraMods(FILE *fp,camcontrol *obj)
	{
		fprintf(fp,"%s\n", "CAMERAMOD:");
		fprintf (fp, "\tCAMMOVEAMOUNT :%f\n",obj->amount);
		fprintf (fp, "\tCAMMOVESPEED :%f\n",obj->speed);
	}

	void OutputOmniLight (FILE *fp, GenLight *maxLight, Matrix3 &transform)
	{
		// An omni light
		fprintf (fp, "OMNILIGHT\n");
		DoCommonLightStuff (fp, maxLight, transform);
		fprintf (fp, "ENDOMNILIGHT\n");
	}
	void OutputSpotLight (FILE *fp, GenLight *maxLight, Matrix3 &transform)
	{
		// An omni light
		fprintf (fp, "SPOTLIGHT\n");
		DoCommonLightStuff (fp, maxLight, transform);
		fprintf (fp, "\tMin : %f\n", 2 * PI * maxLight->GetHotspot(0) / 360.f);
		fprintf (fp, "\tMax : %f\n", 2 * PI * maxLight->GetFallsize(0) / 360.f);
		Point3 dir = Normalize (transform.GetRow(2));
		fprintf (fp, "\tDir : %f %f %f\n", -dir.x, -dir.y, -dir.z);
		fprintf (fp, "ENDSPOTLIGHT\n");
	}
	
	void	BuildWayPath(INode *node)
	{
		//Object *obj = node->EvalWorldState(maxInterface->GetTime()).obj;
		//ShapeObject* shape = (ShapeObject*)obj;
		//PolyShape pShape;

		//shape->MakePolyShape(maxInterface->GetTime(), pShape);
		//int numPts = pShape.lines->numPts;
		//RECORD THE WAYPOINT NODE

		TCHAR *a = node->GetName();
		MapPaths[NumPath]=node;
		NumPath++;
	}

	void OutPutEntry(FILE *fp,int entry)
	{
			fprintf (fp,"\tMapEntry :%d\n",entry);

	}

	void	ParseTrig(RDObject *obj,INode *node)
	{
		int found=0,i;

		if (!(obj->trigpoint))
			return;

		for (i=0;i<NumTrig;i++)
		{
			if (MapTrigs[i] == obj->trigpoint) 
			{
			  //	AssignedStrats[i].stratid[AssignedStrats[i].stratnum] = AddStrat(node);
			  //	AssignedStrats[i].stratnum++;
				AttachedTrigs[i]++;
				found++;
				break;
			}
		}

		if (!found)
		{

			Object *objE = obj->trigpoint->EvalWorldState(maxInterface->GetTime()).obj;
		   	if ((objE->ClassID() != Class_ID(CIRCLE_CLASS_ID,0)))
			{
		 	   	TCHAR message[512];
				sprintf(message,"**NEEDS FIXING: STRAT %s REFERENCES INVALID CREATION/ACTIVATION TRIGGER %s ",node->GetName(),obj->trigpoint->GetName());
				MessageBox (NULL, message,NULL, MB_OK);
			   	//optional 'auto fix'
				//obj->DeleteReference(2);
			  	//obj->SetReference(2,NULL);
			   	//obj->trigpoint = NULL;
				
			}
			else
			{

				AttachedTrigs[i]++;
				MapTrigs[i]= obj->trigpoint;
				if (ALLMAPMODE < GODMODEINTERMEDIATE) 
					LookForName(GetCOREInterface()->GetRootNode(),MapTrigs[i],node,TRIGGERS);
		   	
				NumTrig++;
			}
		}
	}


	short EventGetTime(INode* node)
	{
		char eventname[128];
		char timerref[128];
		short time=0;

		TCHAR *name = node->GetName();

		//PARSE FOR NAME OF TYPE EVENT TIMER x WHERE x IS THE REQUIRED TIMER 
		if (sscanf(name,"%s %s %d",eventname, timerref, &time) == 3)
		{
			if (!strnicmp(timerref,"timer",5))
				return(time);
		}

	  return(0);
	}

	void	ParseEvent(RDObject *obj,INode *node)
	{
		int found=0,i,error=0;

		if ((obj->eventpoint == NULL))
			return;

		for (i=0;i<NumEvents;i++)
		{
			if ((MapEvents[i].node == obj->eventpoint)) 
			{
				int numchild = MapEvents[i].node->NumberOfChildren();

				for (int loop=0;loop<numchild;loop++)
				{
				  INode *child = MapEvents[i].node->GetChildNode(loop);
				  Object *childobj = child->EvalWorldState(GetCOREInterface()->GetTime()).obj;
				  
				  if (childobj == obj)
				  {

					if (ALLMAPMODE < GODMODEINTERMEDIATE)
		 			   	MessageBox (NULL, "THIS EVENT HAS A DUPLICATE DEPENDENT AND CREATOR",MapEvents[i].node->GetName(), MB_OK);
					else
						fprintf(godlog,"THIS EVENT HAS A DUPLICATE DEPENDENT AND CREATOR %s \n",MapEvents[i].node->GetName());
					error++;
					break;
						  
				  }


				}



				MapEvents[i].assigned[MapEvents[i].stratnum] = AddStrat(node);
				MapEvents[i].stratnum++;
				found++;
				break;
			}
		}

		if (error)
			return;

		if (!found)
		{
			//ensure a correct event has been selected
			if (strnicmp(obj->eventpoint->GetName(),"EVENT",5)) 
			{
		 	   	TCHAR message[512];
				sprintf(message,"**NEEDS FIXING: STRAT %s EVENT TRIGGER WITH INCORRECT NAME \n (SHOULD BE EVENT....)%s ",node->GetName(),obj->eventpoint->GetName());
				MessageBox (NULL, message,NULL, MB_OK);

			}
			else
			{
			  /*	if (obj->eventpoint)
				{
					TCHAR *name = obj->eventpoint->GetName();
		 			TCHAR message[512];
					sprintf(message,"EVENT %s IS A DELETION",name);
					MessageBox (NULL, message,NULL, MB_OK);
				}										  */

				if (!strnicmp(obj->eventpoint->GetName(),"EVENTDELETE",11))
					MapEvents[i].flag = 1;
				else
					MapEvents[i].flag = 0;
				MapEvents[i].timer = EventGetTime(obj->eventpoint);

				MapEvents[i].node = obj->eventpoint;
				MapEvents[i].assigned[MapEvents[i].stratnum] = AddStrat(node);
				MapEvents[i].stratnum++;
				NumEvents++;
			}
		}
	}
 
	void	ParseDeletionEvent(RDObject *obj,INode *node)
	{
		int found=0,i,error=0;

		if ((obj->deletioneventpoint == NULL))
			return;

		TCHAR *name = obj->stratSource;

		for (i=0;i<NumEvents;i++)
		{
			if ((MapEvents[i].node == obj->deletioneventpoint)) 
			{
				int numchild = MapEvents[i].node->NumberOfChildren();

				for (int loop=0;loop<numchild;loop++)
				{
				  INode *child = MapEvents[i].node->GetChildNode(loop);
				  Object *childobj = child->EvalWorldState(GetCOREInterface()->GetTime()).obj;
				  
				  if (childobj == obj)
				  {

					if (ALLMAPMODE < GODMODEINTERMEDIATE)
						MessageBox (NULL, "THIS EVENT HAS A DUPLICATE DEPENDENT AND CREATOR",MapEvents[i].node->GetName(), MB_OK);
					else
						fprintf(godlog,"THIS EVENT HAS A DUPLICATE DEPENDENT AND CREATOR %s \n",MapEvents[i].node->GetName());
					error++;
					break;
						  
				  }


				}



				MapEvents[i].assigned[MapEvents[i].stratnum] = AddStrat(node);
		  //		MapEvents[i].stratnum++;
				found++;
				break;
			}
		}

		if (error)
			return;

		if (!found)
		{

			//ensure a correct event has been selected
			if (strnicmp(obj->deletioneventpoint->GetName(),"EVENT",5)) 
			{
		 	   	TCHAR message[512];
				sprintf(message,"**NEEDS FIXING: STRAT %s DELETION EVENT TRIGGER WITH INCORRECT NAME \n (should be EVENT....)%s ",node->GetName(),obj->deletioneventpoint->GetName());
				MessageBox (NULL, message,NULL, MB_OK);

			}
			else
			{
			  	if (obj->deletioneventpoint)
				{
					TCHAR *name = obj->deletioneventpoint->GetName();
					
					if ((strnicmp(name,"EVENTDELETE",11)))
					{
						INode *node = obj->GetMyNode();
						TCHAR message[512];
						sprintf(message,"**EVENT %s FROM  %s ** IS AN INCORRECT NAMED DELETION EVENT",name,node->GetName());
						MessageBox (NULL, message,NULL, MB_OK);
						MapEvents[i].flag  = 0;
					}
					else
						MapEvents[i].flag  = 1;
				}
				
				MapEvents[i].timer = EventGetTime(obj->deletioneventpoint);

				MapEvents[i].node = obj->deletioneventpoint;
				MapEvents[i].assigned[MapEvents[i].stratnum] = AddStrat(node);
	//			MapEvents[i].stratnum++;
				NumEvents++;
			}
		}
	}


	void	ParseActs(RDObject *obj,INode *node)
	{
		int found=0,i,loop;


		for (loop=0;loop<5;loop++)
		{

			found = 0;
			if (!(obj->actpoint[loop]))
				continue;

			for (i=0;i<NumAct;i++)
			{
				if (MapActs[i] == obj->actpoint[loop]) 
				{
					found++;
					break;
				}
			}

			if (!found)
			{
				Object *objE = obj->actpoint[loop]->EvalWorldState(maxInterface->GetTime()).obj;
			   	if ((objE->ClassID() != Class_ID(CIRCLE_CLASS_ID,0)))
				{
			 	   	TCHAR message[128];
					sprintf(message,"**NEEDS FIXING: STRAT %s REFERENCES INVALID ACTIVATION TRIGGER %s ",node->GetName(),obj->trigpoint->GetName());
					MessageBox (NULL, message,NULL, MB_OK);
				
				}
				else
				{

					MapActs[i]= obj->actpoint[loop];
				 	if (ALLMAPMODE < GODMODEINTERMEDIATE) 
						LookForName(GetCOREInterface()->GetRootNode(),MapActs[i],node,ACTIVATION);
					NumAct++;
				}
			}
		}
	}

	void	ParseWay(RDObject *obj,INode *node)
	{
		int found=0,i;

		if (!(obj->waypoint))
			return;

		for (i=0;i<NumPath;i++)
		{
			if (MapPaths[i] == obj->waypoint) 
			{
				found++;
				break;
			}
		}

		if (!found)
		{
			MapPaths[i]= obj->waypoint;
			//*check for duplicate paths
			if (ALLMAPMODE < GODMODEINTERMEDIATE) 
				LookForName(GetCOREInterface()->GetRootNode(),MapPaths[i],node,WAYPOINTS);
			
			NumPath++;
		}
	}

	//HACK ROUTINE TO DERIVE THE CORRECT START NODE WITHIN THE FINAL NETWORK
	//AS AFTER THE COLLAPSE OF A NETWORK SPLINE COLLECTION, THE SPECIFIED
	//START NODE IS NOT NECESSARILY STILL VALID.
	//THIS ROUTINE PERFORMS THE COLLAPSE AND THEN REMAKES THE START NODE

	//***** TO DO
	//COULD CACHE UP GENERATED DATA AND REUSE FOR STRATS NEEDING THIS NETWORK INFO, AS
	//WELL AS THE NETWORK OUTPUT ITSELF
	
	int MakeWay(Interface *maxInterface,INode *way,int startnode, Point3 *spt, int ownpos)
	{

		TCHAR* nodename = way->GetName();	
 
		//ONLY DO IF IT'S A NETWORK
		if (!(strnicmp(nodename,"network",7)))
		{
			Object *obj = way->EvalWorldState(maxInterface->GetTime()).obj;
			ShapeObject* shape = (ShapeObject*)obj;
			PolyShape pShape;
			shape->MakePolyShape(maxInterface->GetTime(), pShape);
			int numPts = pShape.lines->numPts;
			PolyPt *pt = pShape.lines->pts;

			//PULL OUT THE ROTATION MATRIX AND TRANSFORM OF THE SPLINE'S NODE
			Matrix3 transform = way->GetObjectTM(0);
			int numchild = way->NumberOfChildren();
			Point3 PathPoint[256],StartPoint;
		   //	StartPoint.x = spt->x;
		   //	StartPoint.y = spt->y;
		   //	StartPoint.z = spt->z;



			if (numchild)
			{
				//				errorreporthere;

				INode *path = way->GetChildNode(0);			

				int pathpointnum = BuildInitialPath(maxInterface,path,&PathPoint[0]);
			}	
			int numLines = pShape.numLines;


			NETNODE Node[256];
			NETNODE RealNode[256];
				for (int i=0;i<256;i++)
				{
					Node[i].collapsed = 0;
					Node[i].connectednum = 0;
					for (int test =0;test<16;test++)
					{
						Node[i].connected[test] = -1;
						Node[i].mask = 0;

					}
			   
				}
			int NODENUM=0;
			int FIRSTNODEINLINE = 0;
			for (int line = 0;line < numLines; line++)
			{
				numPts = pShape.lines[line].numPts;
				pt = pShape.lines[line].pts;					
				int lastNODE = NODENUM;

				for (int points=0;points<numPts;points++,pt++)
				{
		
					//IF NOT THE FIRST NODE RECORDED AND NOT THE FIRST IN ANY
					//GIVEN NODE LINE RECORD IT AS BEING CONNECTED TO THE PREVIOUS

					if (points)
					{
						Node[NODENUM].connected[0]	= NODENUM-1;
						Node[NODENUM].connectednum = 1;
						Node[lastNODE].connected[Node[lastNODE].connectednum] = NODENUM;
						Node[lastNODE].connectednum ++;
						lastNODE = NODENUM;
						
					}
					else
					{
					 	FIRSTNODEINLINE = NODENUM;
			 			lastNODE = NODENUM;
						Node[NODENUM].connectednum = 0;
					} 
	  
					
					
				
					Node[NODENUM].pt = pt->p * transform;


			   //		if (NODENUM == startnode)
			   //			StartPoint = Node[NODENUM].pt;
						
					NODENUM++;

				}

				//connect last point to first if line is closed
				if ((points) && pShape.lines[line].IsClosed())
				{
					Node[NODENUM-1].connected[1] = FIRSTNODEINLINE;
					Node[NODENUM-1].connectednum = 2;
					Node[FIRSTNODEINLINE].connected[1] = NODENUM - 1;
					Node[FIRSTNODEINLINE].connectednum++;

					//NEW POINT MADE LINKING ENDPOINTS
					Node[NODENUM].connectednum = 2;
					Node[NODENUM].connected[0] = NODENUM - 1;
					Node[NODENUM].connected[1] = FIRSTNODEINLINE;
					Node[NODENUM].pt = Node[FIRSTNODEINLINE].pt;
					
					Node[NODENUM - 1].connected[2] = NODENUM;
					Node[NODENUM - 1].connectednum++;
					Node[FIRSTNODEINLINE].connected[2] = NODENUM;
					Node[FIRSTNODEINLINE].connectednum++;


					NODENUM++;

					

				}

			}

			if (ownpos)
			{
				StartPoint.x = Node[startnode].pt.x;
				StartPoint.y = Node[startnode].pt.y;
				StartPoint.z = Node[startnode].pt.z;
			}
			else
			{
				StartPoint.x = spt->x;
				StartPoint.y = spt->y;
				StartPoint.z = spt->z;

			}



#if 0
			int loop2,nodes;
			for (int node=0;node < NODENUM;node ++)
			{
			   	TrueDupFound(Node,node,NODENUM);
			  //	if (!Node[node].collapsed)
			  //	{
			  //		if (TrueDupFound(&Node,node))
			  //			Collapse()

			  //	}


			}
#endif

			//FIND DUPLICATE

			for (i=0;i<NODENUM;i++)
				Node[i].collapsed = 0;


			//SEARCH FOR INITIAL CONNECTIONS BETWEEN POINTS
			for (int nodes=0;nodes<NODENUM;nodes++)
			{
				for (int searchnodes=0;searchnodes<NODENUM;searchnodes++)
				{
					if ((searchnodes != nodes))
					{
							if (NodePointCompare(&Node[nodes].pt,&Node[searchnodes].pt))
							{
								int foundnode = 0;
								for (int test = 0; test < Node[nodes].connectednum;test++)
								{
									if (Node[nodes].connected[test] == searchnodes)
										foundnode++;
								}
								if (!foundnode)
								{

									Node[nodes].connected[Node[nodes].connectednum] = searchnodes;
									Node[nodes].connectednum++;
								}
							}
					}
				}

			}

			for (i=0;i<NODENUM;i++)
			{
				for (int search=0;search < NODENUM;search++)
				{
					//ANY NON COLLAPSED ONES NOT = TO MYSELF
					if ((search!=i)	&& (!Node[search].collapsed))
					{
						if ((NodePointCompare(&Node[i].pt,&Node[search].pt)) && (!Node[i].collapsed))
						{
						   //	if (i < search)
						   //	{
								Node[search].collapsed = 1;
		
								//NOW GO THROUGH THE NODES,
								//ANY NON=COLLAPSED ONES THAT REFERENCE
								//search REPLACE with i
								for (int searchnew = 0;searchnew < NODENUM; searchnew++)
								{
									if(searchnew != i)
									{
										for (int con=0;con<Node[searchnew].connectednum;con++)
										{
											if ((Node[searchnew].connected[con] == search))
											{
												
												Node[searchnew].connected[con] = i;
												
												//now go through all other nodes connect list any refs to search replace with i
												for (int loop=0;loop<NODENUM;loop++)
												{
													for (int con2=0;con2<Node[loop].connectednum;con2++)
													{
														if ((Node[loop].connected[con2] == search))
																	Node[loop].connected[con2] = i;
													}
												}
											}
										}	
									}		
								}
						  	//}								
						}
					}

				}

			}


			for (i=0;i<NODENUM;i++)
				Node[i].collapsed = 0;

			for (int loop2=0;loop2<NODENUM;loop2++)
			{
				for (int search=0;search<NODENUM;search++)
				{
					//added the second check sat need to test 
					if ((loop2!=search) && (!Node[loop2].collapsed))
					{
						if (NodePointCompare(&Node[loop2].pt,&Node[search].pt))
						{
							//if (loop2 < search)
							//{
								Node[search].collapsed = 1;
								
								for (int con = 0;con<Node[search].connectednum;con++)
								{
									int newnode = Node[search].connected[con];

									int found=0;
									for (int inner=0;inner<Node[loop2].connectednum;inner++)
									{
										if (Node[loop2].connected[inner] == newnode)
										{
											found++;
											break;
				
										}
									}

									if (!found)
									{
										Node[loop2].connected[Node[loop2].connectednum] = newnode;
										Node[loop2].connectednum++;

									}
								}
							//}
						}

					}

				}

			}

			//NOW GO THROUGH ALL NODES EXTRACTING A LIST OF UNIQUE NODE POSITIONS ONLY
			//FIRST RESOLVE ALL THE REFERENCES THEN COPY THE LIST


			int REALNODENUM = 0;

			for (i=0;i<NODENUM;i++)
			{
				if (!Node[i].collapsed)
				{

					for (loop2=0;loop2<NODENUM;loop2++)
					{
						if (!Node[loop2].collapsed)
						{
							for (int con=0;con<Node[loop2].connectednum;con++)
							{
								if (Node[loop2].connected[con] == i)
									Node[loop2].connected[con] = REALNODENUM;
							}
						}
					}

					REALNODENUM++;

				}

			}


			REALNODENUM = 0;



				int TESTARRAY[16];

				//remove duplicate node entries from each non-collapsed node
				//and make up a list of realnodes
				for (i=0;i<NODENUM;i++)
				{
					if (!Node[i].collapsed)
					{
						for (int loop3=0;loop3<16;loop3++)
							TESTARRAY[loop3] = -1;

						int current=0;
						for (loop3=0;loop3<Node[i].connectednum;loop3++)
						{
							int found = 0;
							for (loop2=0;loop2<current;loop2++)
							{
								if (TESTARRAY[loop2] == Node[i].connected[loop3])	
									found++;
							}

							if (!found)
							{
								TESTARRAY[current] = Node[i].connected[loop3];
								current++;


							}
						}


						for (int loop2=0;loop2<16;loop2++)
							Node[i].connected[loop2] = TESTARRAY[loop2];

						
						RealNode[REALNODENUM] = Node[i];							
					   	RealNode[REALNODENUM].connectednum = current;
						REALNODENUM++;

					}
				}



			//NOW CONVERT OUR INITIAL STARTING NODE TO THE CORRECT NODE WITHIN THE PATROL PATH

			for (nodes=0;nodes<REALNODENUM;nodes++)
			{
				if (NodePointCompare(&StartPoint,&RealNode[nodes].pt))
					return(nodes);
			}
			return (-1);
		}
		else
			return (-1);
	
	}

	//GETS THE REQUIRED ACTIVATION POINT INDEX

	int	GetActivation(INode *actpoint)
	{
		for (int i=0;i<NumAct;i++)
		{
			if (MapActs[i]==actpoint)
				return(i);

		}
		return (-1);
	}


	void	StratMapOutput(Interface *maxInterface,FILE *fp,RDObject *RDobj, INode *node)
	{
		char a;
		int i=0;

		//CHECK THE STRAT'S TRIGGER INDEX
		//IF IT HAS ONE WE'LL OUTPUT A LIST CONNECTED TO THE TRIGGER POINT AT THE END

		//if (!(RDobj->trigpoint))
		//{

			//OUTPUT THE STRAT NAME UPTO THE ".DST"
			fprintf (fp,"%s","STRAT ");
			while (RDobj->stratSource[i] != '.')
			{
				a=RDobj->stratSource[i];

			//	if (a == EOF)
			//		i++;
				
				
				fputc(a,fp);
				i++;
			} 
			fprintf(fp,"\n");

			//OUTPUT THE STRAT'S POSITION

			Matrix3 transform = node->GetNodeTM(maxInterface->GetTime());
			Point3	trans = transform.GetTrans();
			fprintf (fp,"%s %f %f %f\n","	Pos :", trans.x,trans.y,trans.z);

			//OUTPUT THE STRAT'S OBJECT FIELD
			//FIRST CHECK TO SEE IF ONE WAS ATTACHED

			if (!(strcmp("NONE",RDobj->objSource)))
			{		
				fprintf (fp,"\t%s\n","Object : NONE");
			}
			else
			{
				i=0;
				fprintf (fp,"\t%s %s%s","Object : ",STRAT_OBJECT_ROOT,"\\");
				while (RDobj->objSource[i] != '.')
				{
					a=toupper(RDobj->objSource[i]);
					fputc(a,fp);
					i++;
				} 
				fprintf(fp,"\n");
			}

//		fprintf (fp,"%s%s%s%s\n","	Object : ",STRAT_OBJECT_ROOT,"\\",RDobj->objSource);

			//OUTPUT THE STRAT'S REL VEL + ABS VEL
		
			trans.x = trans.y = trans.z = 0.0f;
			fprintf (fp,"%s %f %f %f\n","	Relative Velocity :", trans.x,trans.y,trans.z);

			fprintf (fp,"%s %f %f %f\n","	Absolute Velocity :", trans.x,trans.y,trans.z);

			//OUTPUT THE STRAT'S ROTATION
			GetRot(node);
			fprintf (fp,"%s %f %f %f\n","	Rot :", rotation.x,rotation.y,rotation.z);

			//OUTPUT THE STRAT'S SCALE
			fprintf (fp,"%s %f %f %f\n","	Scale :", scale.x,scale.y,scale.z);
			
			//CHECK THE PARENT STRAT IF ANY DEFINED
			int entry = AddStrat(RDobj->parent);
			fprintf (fp,"%s %d\n","	Parent :", entry);



			//OUTPUT THE STRAT'S FLAG
			fprintf (fp,"%s %d \n","	Flag :", RDobj->flag);

			//CHECK FOR STRAT BEING PICKUP AND, IF SO, SET THE CORRECT FLAG

			int flag2 = 0;

			TCHAR* nodename = node->GetName();	
		/*	if ((!(strnicmp(nodename,"PICKUP:",7))) || (!(strnicmp(nodename,"_PICKUP:",8))))
			{
				if ((!(strnicmp(nodename,"PICKUP:BLASTAMP",19))) || (!(strnicmp(nodename,"_PICKUP:BLASTAMP",20))) )
				{
					flag2 |= STRAT2_BLASTAMPPICKUP;
				}
			   	else if ((!(strnicmp(nodename,"PICKUP:SMARTAMP",19))) || (!(strnicmp(nodename,"_PICKUP:SMARTAMP",20))) )
				{
					flag2 |= STRAT2_SMARTAMPPICKUP;
				}
				else if ((!(strnicmp(nodename,"PICKUP:POWERAMP",19))) || (!(strnicmp(nodename,"_PICKUP:POWERAMP",20))) )
				{
					flag2 |= STRAT2_POWERAMPPICKUP;
				}
				else if ((!(strnicmp(nodename,"PICKUP:BOOSTAMP",19))) || (!(strnicmp(nodename,"_PICKUP:BOOSTAMP",20))) )
				{
					flag2 |= STRAT2_BOOSTAMPPICKUP;
				}
			}
*/
		   //	if (flag2 & STRAT2_POWERAMPPICKUP)
			 //		fprintf (fp,"%s %d \n"," power	Flag2 :", flag2);

			//if (flag2 & STRAT2_SMARTAMPPICKUP)
			  //		fprintf (fp,"%s %d \n"," smart	Flag2 :", flag2);

			//if (flag2 & STRAT2_BLASTAMPPICKUP)
			  //		fprintf (fp,"%s %d \n"," blast	Flag2 :", flag2);



			if (RDobj->entirenet)
				flag2 |= STRAT2_ENTIRENET;


			if (RDobj->ownrot)
				flag2 |= STRAT2_INDEPENDENTROT;

			//IF NOT ATTACHED TO A DELETION EVENT, THEN FLAG IT AS BEING INSTANTLY FULLY DELETABLE WHEN DELETED FROM WORLD

			if (!RDobj->deletioneventpoint)
				flag2 |= STRAT2_IMMEDIATEREMOVAL;

			fprintf (fp,"%s %d \n","	Flag2 :", flag2);


			//IF THE STRAT HAS A WAYPOINT THEN LOG ITS ENTRY
		
			int wayindex = -1;

			if (RDobj->waypoint)
			{
			
				int loop;
				for (loop=0;loop<NumPath;loop++)
				{
					if (RDobj->waypoint == MapPaths[loop])
						wayindex = loop;
				}
			
			}

			//OUTPUT THE STRAT'S WAYPOINT INDEX
			fprintf (fp,"%s %d \n","	WayPath :", wayindex);


			int old = RDobj->node;
			//OUTPUT THE STRAT'S START NODE
			if (RDobj->waypoint)
			{
				//JUNE 20TH CHANGE
				//ENSURE THE NODE CHOICE IS VALID
	   			SetUpNodeChoice(RDobj,0,NULL);

				transform = node->GetNodeTM(maxInterface->GetTime());
				trans = transform.GetTrans();

				int startnode = MakeWay(maxInterface,RDobj->waypoint,RDobj->node,&trans,RDobj->ownpos);
			   
				//if we're on patrol we leave the node value
				//else we get the compressed node value
				if ((RDobj->entirenet))
				{
					if (startnode >0)
						RDobj->node = startnode;
				}
			}


			fprintf (fp,"%s %d \n","	StartWay :", RDobj->node);


			RDobj->node = old;


			//OUTPUT THE STRAT'S START PATH NUMBER
			fprintf (fp,"%s %d \n","	StartPath :", RDobj->path);


			//OUTPUT WHETHER THE STRAT IS TRIGGERED
			//THIS SHALL ONLY BE TRUE IF THE MODE IS PARSE3, IE:CALL BY TRIGGER PROCESSING

			//if (Mode == PARSE3)	
			if (RDobj->trigpoint)
				wayindex = 1;
			else
				wayindex = 0;

			fprintf (fp,"%s %d \n","	TrigPath :", wayindex);


			//TO SEE IF THE STRAT HAS AN ATTACHED EVENT

			if (RDobj->eventpoint)
				wayindex = GetEvent(RDobj->eventpoint);
			else
				wayindex = -1;
			
			fprintf (fp,"%s %d \n","	AttachedEvent :", wayindex);

			//TO SEE IF THE STRAT HAS AN ATTACHED DELETION EVENT

		   /*	if (RDobj->deletioneventpoint)
				wayindex = GetEvent(RDobj->deletioneventpoint);
			else
				wayindex = -1;
			
			fprintf (fp,"%s %d \n","	AttachedDeletionEvent :", wayindex);  */

			//TO SEE IF THE STRAT IS EVENT DEPENDENT

			wayindex = EventAttached(RDobj);
			
			fprintf (fp,"%s %d \n","	Dependent :", wayindex);

			
			
			//HAS STRAT GOT AN ACTIVATION POINT ? IF SO OUTPUT ITS INDEX
			int loop;
			for (loop=0;loop<5;loop++)
			{
			
				if (!RDobj->actpoint[loop])
					fprintf (fp,"%s %d \n","	ActPoint :", -1);
				else
					fprintf (fp,"%s %d \n","	ActPoint :", GetActivation(RDobj->actpoint[loop]));
			}


			//NOW OUTPUT SOME TOP ARMOUR AND HEALTH WAREZ ACTION ME HEARTIES
			
			fprintf (fp,"%s %f \n","	Armour :", RDobj->armour);
			fprintf (fp,"%s %f \n","	Health :", RDobj->health);

			//IF THE STRAT HAS CHANGED IPARAMS, THEN OUTPUT THEM

			int changedparam = 0;
			int paramloop;
			if (RDobj->intparammax > 0)
			{
				for (loop=0;loop<RDobj->intparammax;loop++)
				{
					if (RDobj->intparams[loop].value.ival != RDobj->intparams[loop].def.ival)
						changedparam ++;
				}
				if (!changedparam)
					paramloop = 0;
				else
					paramloop = RDobj->intparammax;

				fprintf (fp,"\t%s %d \n","NUM INT PARAMS :", paramloop);
				for (loop=0;loop<paramloop;loop++)
					fprintf (fp,"\t\t%s %d \n","IPARAM:", RDobj->intparams[loop].value.ival);
																																  
			}
			else
				fprintf (fp,"\t%s %d \n","NUM INT PARAMS :", RDobj->intparammax);


			//IF THE STRAT HAS CHANGED FPARAMS, THEN OUTPUT THEM

			changedparam = 0;
			if (RDobj->floatparammax > 0)
			{
				for (loop=0;loop<RDobj->floatparammax;loop++)
				{
					if (RDobj->floatparams[loop].value.fval != RDobj->floatparams[loop].def.fval)
						changedparam ++;
				}

				if (!changedparam)
					paramloop = 0;
				else
					paramloop = RDobj->floatparammax;


				fprintf (fp,"\t%s %d \n","NUM FLOAT PARAMS :", paramloop);
				for (loop=0;loop<paramloop;loop++)
  					fprintf (fp,"\t\t%s %f \n","FPARAM:", RDobj->floatparams[loop].value.fval);

																			  
			}
			else
				fprintf (fp,"\t%s %d \n","NUM FLOAT PARAMS :", RDobj->floatparammax);


			//OUTPUT THE DELAY
			fprintf(fp,"\t%s %d \n","DELAY :",RDobj->delay);



			//END OF STRAT FIELD
			fprintf (fp,"%s\n","ENDSTRAT");
		//}
	}

	void GetRot(INode *node)
	{	
	
		// Calculate the translation, rotation and scaling factors
		Matrix3		parentTM;
		INode		*mum;
		Matrix3		nodeTM = node->GetNodeTM(maxInterface->GetTime());

		mum			= node->GetParentNode();
		parentTM	= mum->GetNodeTM(maxInterface->GetTime());
		Matrix3 localTM		= nodeTM * Inverse(parentTM);

		Quat quat;
		DecomposeMatrix (localTM, translation, quat, scale);

		// Pull out the rotation
		Matrix3 unscaled;

		// Unscale the matrix
		unscaled = localTM;
		unscaled.SetRow (0, Normalize(unscaled.GetRow(0)));
		unscaled.SetRow (1, Normalize(unscaled.GetRow(1)));
		unscaled.SetRow (2, Normalize(unscaled.GetRow(2)));

		if (localTM.Parity()) {
			unscaled.SetRow (0, unscaled.GetRow(0) * -1.f);
			unscaled.SetRow (1, unscaled.GetRow(1) * -1.f);
			unscaled.SetRow (2, unscaled.GetRow(2) * -1.f);
			scale	*= -1.f;
		}

		// Find the x rotation
		rotation.x = (float) asin (unscaled.GetRow(1).z);

		// Check cos(x)
		if (fabs(cos(rotation.x)) > 0.001f) {
			rotation.y	= (float) atan2 (-unscaled.GetRow(0).z, unscaled.GetRow(2).z);
			rotation.z	= (float) atan2 (-unscaled.GetRow(1).x, unscaled.GetRow(1).y);
		} else {
			rotation.y	= (float) atan2 (unscaled.GetRow(2).x, unscaled.GetRow(0).x);
			rotation.z	= 0.f;
		}
	}
};


class RDStratExport : public SceneExport {
public:
	/*
     * MAX API
	 */
	// Returns the number of supported extensions
	virtual int ExtCount() { return 1; }
	// Returns the file extension for a given number
	virtual const TCHAR *Ext(int i) { assert(i==0); return _T("SCR"); };
	// Returns the short and long descriptions, and other strings
	virtual const TCHAR *LongDesc() { return _T("Argonaut Software Red Dog Landscape"); }
	virtual const TCHAR *ShortDesc() { return _T("Red Dog strat map"); }
	virtual const TCHAR *AuthorName() { return _T("Argonaut Software"); }
	virtual const TCHAR *CopyrightMessage() { return _T("(c) 1998 Argonaut Software Ltd"); }
	virtual const TCHAR *OtherMessage1() { return _T(""); }
	virtual const TCHAR *OtherMessage2() { return _T(""); }
	// Returns the version of the plugin
	virtual unsigned int Version() { return RDM_VERSION; }
	// Show the About box
	virtual void ShowAbout (HWND parent) { return; }
	
	// Actually do the export
#if MAX3
	virtual int DoExport (const TCHAR *name, ExpInterface *ei, Interface *i, BOOL suppressPrompts /* = FALSE */, DWORD options)
#else
	virtual int DoExport (const TCHAR *name, ExpInterface *ei, Interface *i, BOOL suppressPrompts /* = FALSE */)
#endif
	{
		char convname[16];
		char convarg[16];
		int index=0;
		int loop;

		GlobalScene = ei->theScene;
		//CLEAR THE STRAT COUNTERS ATTACHED TO EACH POTENTIAL TRIGGER
		//AND A PARANOID CLEARANCE OF TRIG,PATH AND AREA STORES

		for (loop=0;loop<MAXTRIGS;loop++)
			AttachedTrigs[loop]=0;

		for (loop=0;loop<MAXTRIGS;loop++)
		{
			MapEvents[loop].timer=0;
			MapEvents[loop].node=NULL;
			MapEvents[loop].stratnum=0;
			for (int strat=0;strat<MAXATTACHTRIGSTRAT;strat++)
				MapEvents[loop].assigned[strat]=-1;

		}

		for (loop=0;loop<MAXTRIGS;loop++)
			MapTrigs[loop]=NULL;

		for (loop=0;loop<MAXACTS;loop++)
			MapActs[loop]=NULL;

		for (loop=0;loop<MAXPATHS;loop++)
			MapPaths[loop]=NULL;


		NumStrats = 0;
		for (loop=0;loop<MAXSTRATS;loop++)
			Strats[loop]=NULL;

		for (loop=0;loop<MAXPATHS;loop++)
		{
			MapAreas[loop].area=NULL;
			MapAreas[loop].subarea=NULL;
		}

		for (loop=0;loop<MAXASSOCIATES;loop++)
		{
			LinkedAssociates[loop]=NULL;
			UnlinkedAssociates[loop]=NULL;
			NonConnects[loop]=NULL;

		}

		for (loop=0;loop<MAX_EXCLUSIONS;loop++)
			ExclusionZone[loop]=NULL;

		NumExclusionZones = 0;


		EntNum=0;	
		NumArea = 0;	
		NumPath = 0;	
		NumTrig = 0;
		NumEvents = 0;
		NumAct = 0;
		Valid=0;
		MapVarsSet = 0;
		CamMods = 0;
		// CHECK FOR DUPLICATE MAX NODE NAMES
	   //	Mode = MAPCHECK;
	   //	RDStratExportEnumerator CheckScene(ei->theScene, i, NULL);

		//Enumerate through all scene objects extracting exclusion zones

		Mode = EXCLUSION;
		RDStratExportEnumerator ParseExclusions(ei->theScene, i, NULL);




		// Enumerate through all the objects in scene filtering out the waypoint paths
		Mode = PARSE1;
		RDStratExportEnumerator ParseScene(ei->theScene, i, NULL);


		
		FILE *f = fopen (name, "w");
		if (f==NULL)
			return FALSE;

		

		//OUTPUT ALL DEFAULT STRATS, (ONE'S NOT TRIGGERED)
		Mode = PARSE2;
		
		
		// Enumerate through all the objects in the scene, and add them to a map
		RDStratExportEnumerator saveEnumerator(ei->theScene, i, f);


		//ALL DEFAULT MAP STRATS DONE SO FLAG AS ENDED
		fprintf (f,"%s\n","STRATEND\n");
//		fprintf (f,"%s\n","ENDDEFAULT\n");

		// Enumerate through all the objects in scene to export the level flags
		Mode = LEVELVARS;
		RDStratExportEnumerator levelEnumerator(ei->theScene, i, f);
		if (!MapVarsSet)
			fprintf(f,"%s %d\n", "LEVELFLAGSSET:",0);


		//OUTPUT THE TRIGGER LIST
		Mode = PARSE3;
		fprintf (f,"%s %d\n","NUMTRIGS : ",NumTrig);

		for (index=0;index<NumTrig;index++)
		{

			EntNum = 0;
			fprintf (f,"TRIGGER %d\n",index);

			fprintf (f,"\tATTACHED STRATS : %d\n",AttachedTrigs[index]);
			
			TRIGNODE = MapTrigs[index];
			//EXPORT THE POSITION/RADIUS DETAILS
			ExportTrigger(index,i,f);
			
			if (index == 18)
				Mode = PARSE3;

			// Enumerate through all the objects in the scene, and add them to a map
			RDStratExportEnumerator saveEnumerator(ei->theScene, i, f);

			fprintf (f,"ENDTRIGGER\n");
		}	
	   
		fprintf (f,"TRIGEND :\n");
		
		//OUTPUT THE EVENT LIST
		Mode = PARSE3A;
		fprintf (f,"%s %d\n","NUMEVENTS : ",NumEvents);

		for (index=0;index<NumEvents;index++)
		{

			EntNum = 0;
			fprintf (f,"EVENT %d\n",index);
				
			fprintf (f, "\tEventType : %d\n",MapEvents[index].flag);

		   	   
			fprintf (f, "\tEventTimerValue : %d\n",MapEvents[index].timer);

			
			fprintf (f,"\tATTACHED EVENTS : %d\n",MapEvents[index].stratnum);
			
			TRIGNODE = MapEvents[index].node;
			//EXPORT THE POSITION/RADIUS DETAILS
			ExportEvent(index,i,f);
			
			// Enumerate through all the objects in the scene, and add them to a map
			RDStratExportEnumerator saveEnumerator(ei->theScene, i, f);

			fprintf (f,"ENDEVENT\n");
		}	
		fprintf (f,"EVENTEND :\n");
		
		//OUTPUT THE ACTIVATION POINTS LIST

		fprintf (f,"%s %d\n","NUMACTS : ",NumAct);

		for (index=0;index<NumAct;index++)
		{

			fprintf (f,"ACTIVATION %d\n",index);

						
			//SET THE NODE TO CHECK AGAINST
			TRIGNODE = MapActs[index];

			//EXPORT THE POSITION/RADIUS DETAILS
			ExportActivation(i,TRIGNODE,f);
			
			fprintf (f,"ENDACTIVATION\n");
		}	
		

		fprintf (f,"%s\n","ACTIVATIONEND :\n");
		
		

	  	Scene = ei->theScene;

		//CHUCK THE WAYPOINT LIST AT THE END OF THE MAP FILE
		ExportWay(i,f);	
		fprintf (f, "ENDOFPATHS :\n");

		//..only joking it wasn't really the end of the map, THIS is the real end
		
		ExportArea(i,f);		
		fprintf (f, "ENDOFAREAS :\n");

		
		
		// ..not quite at the end, as lights go now...
		Mode = PARSE4;
		NumLights = 0;
		RDStratExportEnumerator countLights (ei->theScene, i, NULL);
		fprintf (f,"NUMLIGHTS %d\n",NumLights);
		RDStratExportEnumerator outputLights (ei->theScene, i, f);
		fprintf (f, "ENDLIGHTS\n");
		

		// NOW FOR THE CAMERA MODIFIER RECORDS... IF ANY EXIST
		fprintf (f, "CAMMODS :%d\n",CamMods);

		if (CamMods)
		{
			Mode = CAMERAMODS;
			RDStratExportEnumerator levelEnumerator(ei->theScene, i, f);
		
		}
		
		
		//TERMINATE THE MAP
		fprintf (f,"%s\n","ENDMAP ");
		
		
		// Close the output file
		fclose (f);

		//WILL STRAT CONVERT IF THE SCENE CONTAINS A STRAT OBJECT ONLY
		if (Valid)
		{
			index=0;
			
			//SETUP THE CORRECT ARGUMENT NAME FOR THE STRAT CONVERTER

			int len = strlen(name)+1;

			while (name[len] != '\\')
				len--;

			len++;
			
			while (name[len] != '.')
			{
				convname[index]=name[len];
				len++;
				index++;
			}
			convname[index]=NULL;

			if (GetCOREInterface()->GetCancel())
				return FALSE;

//#if 0
#if ARTIST
#define CODESCAPE_PARMS "/nologo", "/noautoload", "-nogui", "-t1p1bnr+h:d:\\dreamcast\\reddog.elf"
#else
#if GODMODEMACHINE
#define CODESCAPE_PARMS "/nologo", "/noautoload", "-nogui", "-t1p1bnr+h:d:\\dreamcast\\reddog\\game\\reddog.elf"
#else
#define CODESCAPE_PARMS "/nologo", "/noautoload", "-nogui", "-t1p1bnr+h:c:\\dreamcast\\reddog\\game\\reddog.elf"
#endif
#endif

			int res;
			if (ALLMAPMODE)
			{
				if (ALLMAPMODE == 1)
					sprintf(convarg,"%s%s %s %s","@",convname,"ALLMAPS", "MAX");
		   		else
				{
					if (ALLMAPMODE == GODMODEINTERMEDIATE)
						sprintf(convarg,"%s%s %s %s","@",convname,"GODMODE", "MAX");
					else
						sprintf(convarg,"%s%s %s %s","@",convname,"GODMODEFINAL", "MAX");
				}
			}
			else
				sprintf(convarg,"%s%s %s %s","@",convname,"SINGLE", "MAX");
			#if ARTIST
			   res = 	_spawnl (_P_WAIT, "p:\\UTILS\\strat.exe","p:\\UTILS\\strat.exe", convarg, NULL);

			   if (!res)
			   {
					// Wad the data up
	  				_spawnl (_P_WAIT, "p:\\UTILS\\wadder.exe",NULL,NULL);
					// Update GDWorkshop
					if (GDSRUN) {
						SetCurrentDirectory ("D:\\DREAMCAST\\GDWORKSHOP");
						_spawnl (_P_WAIT, "d:\\DREAMCAST\\GDWORKSHOP\\gdworkshop.exe", "d:\\DREAMCAST\\GDWORKSHOP\\gdworkshop.exe", "d:\\DREAMCAST\\GDWORKSHOP\\Update.js", NULL);
					}
					// Run codescape
					if (CODERUN)
						_spawnl (_P_WAIT, "d:\\DREAMCAST\\CODESCAPE\\codescape.exe","d:\\DREAMCAST\\CODESCAPE\\codescape.exe", CODESCAPE_PARMS, NULL);
			   }
			#else

#if GODMODEMACHINE
				res = _spawnl (_P_WAIT, "d:\\DREAMCAST\\REDDOG\\STREAM\\strat.exe","d:\\DREAMCAST\\REDDOG\\STREAM\\strat.exe", convarg, NULL);
#else
				res = _spawnl (_P_WAIT, "c:\\DREAMCAST\\REDDOG\\STREAM\\strat.exe","c:\\DREAMCAST\\REDDOG\\STREAM\\strat.exe", convarg, NULL);
#endif
			
		  		if (!res)
		  		{
					// Run the WADder
					// GODMODE SETS ALLMAPMODE TO = 2 UNTIL ITS LAST FILE WHEN ALLMAPMODE = 3
					if (ALLMAPMODE != GODMODEINTERMEDIATE) 
#if GODMODEMACHINE
						_spawnl (_P_WAIT, "d:\\DREAMCAST\\REDDOG\\WADDER\\RELEASE\\wadder.exe",NULL, NULL);
#else
						_spawnl (_P_WAIT, "c:\\DREAMCAST\\REDDOG\\WADDER\\RELEASE\\wadder.exe",NULL, NULL);
#endif
				
					// Update GDWorkshop
					if (GDSRUN) {
#if GODMODEMACHINE
						SetCurrentDirectory ("c:\\DREAMCAST\\GDWORKSHOP");
						_spawnl (_P_WAIT, "c:\\DREAMCAST\\GDWORKSHOP\\gdworkshop.exe", "c:\\DREAMCAST\\GDWORKSHOP\\gdworkshop.exe", "c:\\DREAMCAST\\GDWORKSHOP\\Update.js", NULL);
#else
						SetCurrentDirectory ("d:\\DREAMCAST\\GDWORKSHOP");
						_spawnl (_P_WAIT, "d:\\DREAMCAST\\GDWORKSHOP\\gdworkshop.exe", "d:\\DREAMCAST\\GDWORKSHOP\\gdworkshop.exe", "d:\\DREAMCAST\\GDWORKSHOP\\Update.js", NULL);
#endif
					}
					// Run codescape
					if (CODERUN)
#if GODMODEMACHINE
						_spawnl (_P_WAIT, "d:\\DREAMCAST\\CODESCAPE\\codescape.exe","d:\\DREAMCAST\\CODESCAPE\\codescape.exe", CODESCAPE_PARMS, NULL);
#else
						_spawnl (_P_WAIT, "c:\\DREAMCAST\\CODESCAPE\\codescape.exe","c:\\DREAMCAST\\CODESCAPE\\codescape.exe", CODESCAPE_PARMS, NULL);
#endif
				}
			#endif
//#endif
		}
		return TRUE;
	}

	int	AddArea(INode *area,INode *path)
	{
		int found=0,i;

		for (i=0;i<NumArea;i++)
		{
			if (MapAreas[i].area == area)
			{
				found++;
				break;
			}
		}

		if (!found)
		{
			MapAreas[i].area=area;
	//		MapAreas[i].path=path;
			NumArea++;
			return(NumArea-1);
		}
		else
			return(i);
	}

#define DEBUG_NET 0

	int	CircleCollate(int entry,int numchild,FILE *fp,Interface *maxInterface,int write)
	{

		int Circlenum = 0;

		for (int children =0; children<numchild; children++)
		{
			INode *child=MapAreas[entry].area->GetChildNode(children);	
			TCHAR* nodename = child->GetName();	

			//MAKE SURE IT'S A VALID AREA NAME, BIT NODDY BUT BETTER THAN NOWT
			if (!(strnicmp(nodename,"BBox",4)))
			{
				Object *obj = child->EvalWorldState(maxInterface->GetTime()).obj;
				IParamArray *array = obj->GetParamBlock();
					
				if (obj->ClassID() == Class_ID(CIRCLE_CLASS_ID,0))
				{
					Circlenum++;
					if (write)
					{
						int radindex = obj->GetParamBlockIndex(CIRCLE_RADIUS);
						float radius;
						array->GetValue(radindex,0,radius,FOREVER);

						//PULL OUT THE ROTATION MATRIX AND TRANSFORM OF THE AREAS NODE
			
						Matrix3 transform = child->GetNodeTM(0);
						Point3	trans = transform.GetTrans();


						radius = ComputeScaledRadius(radius,&transform);


				
					//	Point3 xvec;
					 //	xvec.x = radius;
					 //	xvec.y = 0;
					 //	xvec.z = 0;
					  //	Point3 row1 = transform.GetRow(0);

						//APPLY TRANFORMATION TO THE WAYPOINT AND OUTPUT		
				
						//Point3 tvec;
					   //	tvec.x = (xvec.x * row1.x);

					   //	tvec = xvec * transform;

					   //	tvec.y = tvec.z = 0;
					   //	tvec = radius * trans;								 +
					   //	radius = (float)sqrt((tvec.x * tvec.x) + (tvec.y * tvec.y) + (tvec.z * tvec.z));

					  //	radius = tvec.x;

						//OUTPUT THE RADIUS VALUE
						fprintf (fp, "\t SubAreaRadius : %f\n",radius);

						fprintf (fp, "\t SubArVertex : %f %f \n",trans.x,trans.y);
					}					
				}
			}
		}				
		return(Circlenum);
	}


	//TAKES A RECTANGLE AND TRANSFORMS IT INTO SEPARATE BOX VERTICES ACCORDING TO THE NODE'S TRANSFORMATION

	void GetRectangleBounds(Object *obj, INode *node, Point3 *topright, Point3 *topleft, Point3 *bottomleft, Point3 *bottomright)
	{
		IParamArray *array = obj->GetParamBlock();
		Matrix3 transform = node->GetNodeTM(0);
		Point3	trans = transform.GetTrans();
							
		int lengthindex = obj->GetParamBlockIndex(RECTANGLE_LENGTH);
		int widthindex = obj->GetParamBlockIndex(RECTANGLE_WIDTH);
		float rectanglelength;
		float rectanglewidth;

		array->GetValue(lengthindex,0,rectanglelength,FOREVER);
		array->GetValue(widthindex,0,rectanglewidth,FOREVER);

		Point3 pos;

		//HALF DISTS NEEDED
		rectanglelength /= 2.0f;
		rectanglewidth /= 2.0f;
						
		pos.x = rectanglewidth;						
		pos.y = rectanglelength;						
		pos.z = 0;						
		*topright = pos * transform;

		pos.x = -rectanglewidth;						
		pos.y = rectanglelength;						
		pos.z = 0;						
		*topleft = pos * transform;

		pos.x = rectanglewidth;						
		pos.y = -rectanglelength;						
		pos.z = 0;						
		*bottomright = pos * transform;

		pos.x = -rectanglewidth;						
		pos.y = -rectanglelength;						
		pos.z = 0;						
		*bottomleft = pos * transform;
	}

	void OutputMaxMins(Point3 *topright, Point3 *topleft, Point3 *bottomleft, Point3 *bottomright,FILE *fp)
	{

		float maxx = bottomleft->x;
					
		if (bottomright->x > maxx)
			maxx = bottomright->x;
		if (topleft->x > maxx)
			maxx = topleft->x;
		if (topright->x > maxx)
			maxx = topright->x;

		float minx = bottomleft->x;

		if (bottomright->x < minx)
			minx = bottomright->x;
		if (topleft->x < minx)
			minx = topleft->x;
		if (topright->x < minx)
			minx = topright->x;

		float maxy = bottomleft->y;
					
		if (bottomright->y > maxy)
			maxy = bottomright->y;
		if (topleft->y > maxy)
			maxy = topright->y;
		if (topright->y > maxy)
			maxy = topright->y;	

		float miny = bottomleft->y;

		if (bottomright->y < miny)
			miny = bottomright->y;
		if (topleft->y < miny)
			miny = topleft->y;
		if (topright->y < miny)
			miny = topright->y;

		fprintf (fp, "\t SubArMaxx : %f\n",maxx);
		fprintf (fp, "\t SubArMinx : %f\n",minx);
		fprintf (fp, "\t SubArMaxy : %f\n",maxy);
		fprintf (fp, "\t SubArMiny : %f\n",miny);

	}

	int	BoxCollate(int entry,int numchild,FILE *fp,Interface *maxInterface,int write)
	{

		int Boxnum = 0;
		Point3 topright,topleft,bottomleft,bottomright;

		if (!write)
			LinkedAssociated = 0;

		for (int children =0; children<numchild; children++)
		{
			INode *child=MapAreas[entry].area->GetChildNode(children);	
			TCHAR* nodename = child->GetName();	

			//MAKE SURE IT'S A VALID AREA NAME, BIT NODDY BUT BETTER THAN NOWT
			if (!(strnicmp(nodename,"BBox",4)))
			{
				Object *obj = child->EvalWorldState(maxInterface->GetTime()).obj;
				IParamArray *array = obj->GetParamBlock();
					
				if (obj->ClassID() == Class_ID(RECTANGLE_CLASS_ID,0))
				{
					Boxnum++;
					if (!write)
					{
						LinkedAssociates[LinkedAssociated] = child;	//LOG THE NODE AS BEING ATTACHED
						LinkedAssociated++;
					}

					if (write)
					{
						//PULL OUT THE ROTATION MATRIX AND TRANSFORM OF THE AREAS NODE
			
						GetRectangleBounds(obj,child,&topright,&topleft,&bottomleft,&bottomright);
						OutputMaxMins(&topright,&topleft,&bottomleft,&bottomright,fp);


					}					
				}
			}
		}				
		if (write)
		{
			int i;
			for (i=0;i<NonConnected;i++)
			{

				//PULL OUT THE ROTATION MATRIX AND TRANSFORM OF THE AREAS NODE

				Object *obj = NonConnects[i]->EvalWorldState(maxInterface->GetTime()).obj;


				GetRectangleBounds(obj,NonConnects[i],&topright,&topleft,&bottomleft,&bottomright);
				OutputMaxMins(&topright,&topleft,&bottomleft,&bottomright,fp);

			}


		}
		
		
		
		
		return(Boxnum);
	}

	int within(Point3 centre, Point3 Pos, float radius)
	{
		float x = centre.x - Pos.x;
		float y = centre.y - Pos.y;
		float z = 0;
	//	centre.z - Pos.z;

		float dist = (float)sqrt((x*x)+(y*y)+(z*z));

		if (dist<=radius)
			return(1);
		else
			return(0);

	}



	int	GetNonLinkedAssociateBoxes(Point3 centre,float radius,Interface *maxInterface)
	{
		int i;
		FoundAssociates = 0;
		UnlinkedAssociated = 0;
		Mode = LOGBOX;
		RDStratExportEnumerator countLights (Scene, maxInterface, NULL);

		for (i=0;i<UnlinkedAssociated;i++)
		{	
			Object *obj = UnlinkedAssociates[i]->EvalWorldState(maxInterface->GetTime()).obj;
			IParamArray *array = obj->GetParamBlock();
					
			if (obj->ClassID() == Class_ID(RECTANGLE_CLASS_ID,0))
			{
				Matrix3 transform = UnlinkedAssociates[i]->GetNodeTM(0);
				Point3	trans = transform.GetTrans();
		
						
				int lengthindex = obj->GetParamBlockIndex(RECTANGLE_LENGTH);
				int widthindex = obj->GetParamBlockIndex(RECTANGLE_WIDTH);
				float rectanglelength;
				float rectanglewidth;

				array->GetValue(lengthindex,0,rectanglelength,FOREVER);
				array->GetValue(widthindex,0,rectanglewidth,FOREVER);

				Point3 pos,topright,topleft,bottomright,bottomleft;

				//HALF DISTS NEEDED
				rectanglelength /= 2.0f;
				rectanglewidth /= 2.0f;
						
				pos.x = rectanglewidth;						
				pos.y = rectanglelength;						
				pos.z = 0;						
				topright = pos * transform;

				if (within(centre,topright,radius))
				{
					NonConnects[FoundAssociates]=UnlinkedAssociates[i];
					FoundAssociates++;
					continue;
				}

				pos.x = -rectanglewidth;						
				pos.y = rectanglelength;						
				pos.z = 0;						
				topleft = pos * transform;

				if (within(centre,topleft,radius))
				{
					NonConnects[FoundAssociates]=UnlinkedAssociates[i];
					FoundAssociates++;
					continue;
				}

				pos.x = rectanglewidth;						
				pos.y = -rectanglelength;						
				pos.z = 0;						
				bottomright = pos * transform;

				if (within(centre,bottomright,radius))
				{
					NonConnects[FoundAssociates]=UnlinkedAssociates[i];
					FoundAssociates++;
					continue;
				}

				pos.x = -rectanglewidth;						
				pos.y = -rectanglelength;						
				pos.z = 0;						
				bottomleft = pos * transform;

				if (within(centre,bottomleft,radius))
				{
					NonConnects[FoundAssociates]=UnlinkedAssociates[i];
					FoundAssociates++;
					continue;
				}

			}

		}		
		
		NonConnected = FoundAssociates;
		return(FoundAssociates);

	}


	int	ShapeCollate(int entry,int numchild,FILE *fp,Interface *maxInterface,int write)
	{

		int Shapenum = 0;

		for (int children =0; children<numchild; children++)
		{
			INode *child=MapAreas[entry].area->GetChildNode(children);	
			TCHAR* nodename = child->GetName();	

			//MAKE SURE IT'S A VALID AREA NAME, BIT NODDY BUT BETTER THAN NOWT
			if (!(strnicmp(nodename,"BBox",4)))
			{
				Object *obj = child->EvalWorldState(maxInterface->GetTime()).obj;
				//IParamArray *array = obj->GetParamBlock();
				if (
					(obj->ClassID() != Class_ID(RECTANGLE_CLASS_ID,0))	&&
					(obj->ClassID() != Class_ID(CIRCLE_CLASS_ID,0))
				   //	(obj->ClassID() == Class_ID(SPLINESHAPE_CLASS_ID,0))	||
				   //	(obj->ClassID() == Class_ID(LINEARSHAPE_CLASS_ID,0))
					
					)
				{
					Shapenum++;
					if (write)
					{
						ShapeObject* shape = (ShapeObject*)obj;
						PolyShape pShape;
						shape->MakePolyShape(maxInterface->GetTime(), pShape);
						int numPts = pShape.lines->numPts;
						PolyPt *pt = pShape.lines->pts;

						//PULL OUT THE ROTATION MATRIX AND TRANSFORM OF THE AREAS NODE
			
						Matrix3 transform = child->GetNodeTM(0);
						Point3	trans = transform.GetTrans();
						Point3  rot;		
						
						fprintf (fp, "\t NumAreaPoints : %d\n",numPts);
						for (int points=0;points<numPts;points++,pt++)
						{

							//APPLY TRANFORMATION TO THE AREAPOINT AND OUTPUT		
							//NOTE 2D AT THE MOMENT SO NO HEIGHT "Z" COMPONENT
							rot = pt->p * transform;
							fprintf (fp, "\t ArVertex : %f %f \n",rot.x,rot.y);

						}
						pShape.NewShape();


						//now check for child escape of this bbox
						int escape = child->NumberOfChildren();
						if (!escape)
							fprintf (fp, "\t\t SubSubAreas : %d\n",0);
						else
						{
							fprintf (fp, "\t\t SubSubAreas : %d\n",1);
							child = child->GetChildNode(0);
							obj = child->EvalWorldState(maxInterface->GetTime()).obj;
							shape = (ShapeObject*)obj;
							shape->MakePolyShape(maxInterface->GetTime(), pShape);
							numPts = pShape.lines->numPts;
							pt = pShape.lines->pts;

							//PULL OUT THE ROTATION MATRIX AND TRANSFORM OF THE AREAS NODE
			
							transform = child->GetNodeTM(0);
							trans = transform.GetTrans();
						
							fprintf (fp, "\t\t NumAreaPoints : %d\n",numPts);
							for (int points=0;points<numPts;points++,pt++)
							{

								//APPLY TRANFORMATION TO THE AREAPOINT AND OUTPUT		
								//NOTE 2D AT THE MOMENT SO NO HEIGHT "Z" COMPONENT
								rot = pt->p * transform;
								fprintf (fp, "\t\t ArVertex : %f %f \n",rot.x,rot.y);

							}
							pShape.NewShape();
						
						}
					}					
				}
			}
		}				
		return(Shapenum);
	}

	int	ExtSplineCollate(int entry,int numchild,FILE *fp,Interface *maxInterface,int write)
	{

		int Shapenum = 0;

		for (int children =0; children<numchild; children++)
		{
			INode *child=MapAreas[entry].area->GetChildNode(children);	
			TCHAR* nodename = child->GetName();	

			//MAKE SURE IT'S A VALID AREA NAME, BIT NODDY BUT BETTER THAN NOWT
			if (!(strnicmp(nodename,"ExtBBox",4)))
			{
				Object *obj = child->EvalWorldState(maxInterface->GetTime()).obj;
				IParamArray *array = obj->GetParamBlock();
				if (
					(obj->ClassID() != Class_ID(RECTANGLE_CLASS_ID,0))	&&
					(obj->ClassID() != Class_ID(CIRCLE_CLASS_ID,0))
				   //	(obj->ClassID() == Class_ID(SPLINESHAPE_CLASS_ID,0))	||
				   //	(obj->ClassID() == Class_ID(LINEARSHAPE_CLASS_ID,0))
					
					)
				{
					Shapenum++;
					if (write)
					{
						ShapeObject* shape = (ShapeObject*)obj;
						PolyShape pShape;
						shape->MakePolyShape(maxInterface->GetTime(), pShape);
						int numPts = pShape.lines->numPts;
						PolyPt *pt = pShape.lines->pts;

						//PULL OUT THE ROTATION MATRIX AND TRANSFORM OF THE AREAS NODE
			
						Matrix3 transform = child->GetNodeTM(0);
						Point3	trans = transform.GetTrans();
						Point3  rot;		
						
						fprintf (fp, "\t NumAreaPoints : %d\n",numPts);
						for (int points=0;points<numPts;points++,pt++)
						{

							//APPLY TRANFORMATION TO THE AREAPOINT AND OUTPUT		
							//NOTE 2D AT THE MOMENT SO NO HEIGHT "Z" COMPONENT
							rot = pt->p * transform;
							fprintf (fp, "\t ArVertex : %f %f \n",rot.x,rot.y);

						}
						pShape.NewShape();

						//now check for child escape of this bbox
						int escape = child->NumberOfChildren();
						if (!escape)
							fprintf (fp, "\t\t SubSubAreas : %d\n",0);
						else
						{
							fprintf (fp, "\t\t SubSubAreas : %d\n",1);
							child = child->GetChildNode(0);
							obj = child->EvalWorldState(maxInterface->GetTime()).obj;
							shape = (ShapeObject*)obj;
							shape->MakePolyShape(maxInterface->GetTime(), pShape);
							numPts = pShape.lines->numPts;
							pt = pShape.lines->pts;

							//PULL OUT THE ROTATION MATRIX AND TRANSFORM OF THE AREAS NODE
			
							transform = child->GetNodeTM(0);
							trans = transform.GetTrans();
						
							fprintf (fp, "\t\t NumAreaPoints : %d\n",numPts);
							for (int points=0;points<numPts;points++,pt++)
							{

								//APPLY TRANFORMATION TO THE AREAPOINT AND OUTPUT		
								//NOTE 2D AT THE MOMENT SO NO HEIGHT "Z" COMPONENT
								rot = pt->p * transform;
								fprintf (fp, "\t\t ArVertex : %f %f \n",rot.x,rot.y);

							}
							pShape.NewShape();
						
						}
					}					
				}
			}
		}				
		return(Shapenum);
	}

	int	SightCollate(int entry,int numchild,FILE *fp,Interface *maxInterface,int write)
	{
		int Shapenum = 0;

		for (int children =0; children<numchild; children++)
		{
			INode *child=MapAreas[entry].area->GetChildNode(children);	
			TCHAR* nodename = child->GetName();	

			//MAKE SURE IT'S A VALID AREA NAME, BIT NODDY BUT BETTER THAN NOWT
			if (!(strnicmp(nodename,"Sight",5)))
			{
				Object *obj = child->EvalWorldState(maxInterface->GetTime()).obj;
				IParamArray *array = obj->GetParamBlock();
				if (
					(obj->ClassID() != Class_ID(RECTANGLE_CLASS_ID,0))	&&
					(obj->ClassID() != Class_ID(CIRCLE_CLASS_ID,0))
				   //	(obj->ClassID() == Class_ID(SPLINESHAPE_CLASS_ID,0))	||
				   //	(obj->ClassID() == Class_ID(LINEARSHAPE_CLASS_ID,0))
					
					)
				{
					Shapenum++;
					if (write)
					{
						Object *nodeobj = child->GetObjectRef();
						if (nodeobj->SuperClassID() == GEN_DERIVOB_CLASS_ID)
						{	
			   				IDerivedObject *derived = (IDerivedObject*)nodeobj;
							if (derived->NumModifiers())
							{
							    obj = derived->GetObjRef();
								if (ALLMAPMODE < GODMODEINTERMEDIATE)
								 	MessageBox (NULL, "A MODIFIER HAS BEEN APPLIED TO THIS SPLINE OBJECT",nodename, MB_OK);
								else
									fprintf(godlog,"A MODIFIER HAS BEEN APPLIED TO THIS SPLINE OBJECT %s \n",nodename);
							}
						}
						else
							obj=nodeobj;
						ShapeObject* shape = (ShapeObject*)obj;
						PolyShape pShape;
						shape->MakePolyShape(maxInterface->GetTime(), pShape);
						int numPts = pShape.lines->numPts;
						PolyPt *pt = pShape.lines->pts;

						//PULL OUT THE ROTATION MATRIX AND TRANSFORM OF THE AREAS NODE
			
						Matrix3 transform = child->GetNodeTM(0);
						Point3	trans = transform.GetTrans();
						Point3  rot;		
						
						fprintf (fp, "\t NumAreaPoints : %d\n",numPts);
						for (int points=0;points<numPts;points++,pt++)
						{

							//APPLY TRANFORMATION TO THE AREAPOINT AND OUTPUT		
							//NOTE 2D AT THE MOMENT SO NO HEIGHT "Z" COMPONENT
							rot = pt->p * transform;
							fprintf (fp, "\t ArVertex : %f %f \n",rot.x,rot.y);

						}
						pShape.NewShape();
					}					
				}
			}
		}				
		return(Shapenum);
	}

	int	CampSearchKennethWilliams(int entry,int numchild,FILE *fp,Interface *maxInterface,int write)
	{
		int Shapenum = 0;

		for (int children =0; children<numchild; children++)
		{
			INode *child=MapAreas[entry].area->GetChildNode(children);	

			Object *childobj = child->GetObjectRef();
			
			//ENSURE THE CHILD IS A CAMPPOINT TYPE
													
			if (childobj->ClassID() == CAMPPOINT_CLASS_ID)
			{
			   	Shapenum++;
			   	if (write)
			   	{
					Matrix3 transform = child->GetNodeTM(0);
					Point3	trans = transform.GetTrans();
					fprintf (fp, "\t CampPos : %f %f\n",trans.x,trans.y);
				}
			}
		}				
		return(Shapenum);
	}

	int	EllipseCollate(int entry,int numchild,FILE *fp,Interface *maxInterface,int write)
	{
		int Ellipsenum=0;
	
		for (int children =0; children<numchild; children++)
		{
			INode *child=MapAreas[entry].area->GetChildNode(children);	
			TCHAR* nodename = child->GetName();	

			//MAKE SURE IT'S A VALID AREA NAME, BIT NODDY BUT BETTER THAN NOWT
			if (!(strnicmp(nodename,"BBox",4)))
			{
				Object *obj = child->EvalWorldState(maxInterface->GetTime()).obj;
				IParamArray *array = obj->GetParamBlock();
							
				

				if (obj->ClassID() == Class_ID(ELLIPSE_CLASS_ID,0) )
				{
					Ellipsenum++;
					if (write)
					{
						float length,width;

						int lengthindex = obj->GetParamBlockIndex(ELLIPSE_LENGTH);
						int widthindex = obj->GetParamBlockIndex(ELLIPSE_WIDTH);
						array->GetValue(lengthindex,0,length,FOREVER);
						array->GetValue(widthindex,0,width,FOREVER);
						//OUTPUT THE LENGTH
						fprintf (fp, "\t SubAreaLength : %f\n",length);
						//OUTPUT THE WIDTH
						fprintf (fp, "\t SubAreaWidth : %f\n",width);

						//PULL OUT THE ROTATION MATRIX AND TRANSFORM OF THE AREAS NODE
			
						Matrix3 transform = child->GetNodeTM(0);
						Point3	trans = transform.GetTrans();
						fprintf (fp, "\t SubArVertex : %f %f \n",trans.x,trans.y);
					}
				}	
			}
		}				
		return(Ellipsenum);
	}

	void ExportArea(Interface *maxInterface, FILE *fp)
	{
		
		fprintf (fp,"%s %d\n","NUMAREAS : ",NumArea);

		for (int loop=0;loop<NumArea;loop++)
		{
			fprintf (fp,"%s\n","AREA ");

			Object *obj = MapAreas[loop].area->EvalWorldState(maxInterface->GetTime()).obj;
		
			//NEW STUFF FOR CIRCULAR BBOX REGIONS
			IParamArray *array = obj->GetParamBlock();

			int radindex = obj->GetParamBlockIndex(CIRCLE_RADIUS);
			float radius;
			array->GetValue(radindex,0,radius,FOREVER);


			//PULL OUT THE ROTATION MATRIX AND TRANSFORM OF THE AREAS NODE
			
			Matrix3 transform = MapAreas[loop].area->GetNodeTM(0);
			Point3	trans = transform.GetTrans();
			Point3 row1 = transform.GetRow(0);

			//APPLY TRANFORMATION TO THE WAYPOINT AND OUTPUT		

			radius = ComputeScaledRadius(radius,&transform);


				
			//OUTPUT THE RADIUS
			fprintf (fp, "\t AreaRadius : %f\n",radius);

			fprintf (fp, "\t ArVertex : %f %f \n",trans.x,trans.y);

			//NOW EXPORT ANY ATTACHED CHILDREN BBOX REGIONS
			int numchild = MapAreas[loop].area->NumberOfChildren();

			//WE KNOW THAT THE PATH ITSELF IS A CHILD OF THE BBOX SO DECREMENT NUMCHILD BY 1
			fprintf (fp, "\t SubAreas : %d \n",numchild-1);

			//EXPORT EXTERIOR SPLINE BOUNDING AREAS, SHOULD ONLY BE ONE PER AREA
			int exteriorspline = ExtSplineCollate(loop,numchild,fp,maxInterface,0);
		 	if (exteriorspline > 1)
			{
				if (ALLMAPMODE < GODMODEINTERMEDIATE)
				  	MessageBox (NULL, "MORE THAN ONE EXTERIOR SPLINE CONNECTED TO AREA",MapAreas[loop].area->GetName(), MB_OK);
				else
					fprintf(godlog,"MORE THAN ONE EXTERIOR SPLINE CONNECTED TO AREA %s \n",MapAreas[loop].area->GetName());
			}
			
			fprintf (fp, "\t NumExtSplineAreas : %d \n",exteriorspline);
			ExtSplineCollate(loop,numchild,fp,maxInterface,1);
			
			
			//EXPORT INTERIOR CIRCLE SPLINE BOUNDING AREAS
			int numcircles = CircleCollate(loop,numchild,fp,maxInterface,0);
			fprintf (fp, "\t NumCircleAreas : %d \n",numcircles);
			CircleCollate(loop,numchild,fp,maxInterface,1);
			
			//EXPORT INTERIOR BOX SPLINE BOUNDING AREAS
			int numboxes = BoxCollate(loop,numchild,fp,maxInterface,0);
			numboxes += GetNonLinkedAssociateBoxes(trans,radius,maxInterface);
			fprintf (fp, "\t NumBoxAreas : %d \n",numboxes);

			BoxCollate(loop,numchild,fp,maxInterface,1);

			
			#if 0
				int numellipses = EllipseCollate(loop,numchild,fp,maxInterface,0);
				fprintf (fp, "\t NumEllipseAreas : %d \n",numellipses);
				EllipseCollate(loop,numchild,fp,maxInterface,1);
			#endif

			//EXPORT INTERIOR ARBITARY SPLINE BOUNDING AREAS
			int numshapes = ShapeCollate(loop,numchild,fp,maxInterface,0);
			fprintf (fp, "\t NumShapeAreas : %d \n",numshapes);
			ShapeCollate(loop,numchild,fp,maxInterface,1);


			//EXPORT INTERIOR ARBITARY LINE OF SIGHT SPLINE BOUNDING AREAS
			numshapes = SightCollate(loop,numchild,fp,maxInterface,0);
			fprintf (fp, "\t NumSightAreas : %d \n",numshapes);
			SightCollate(loop,numchild,fp,maxInterface,1);

			//SEARCH FOR CAMPING POSITIONS ATTACHED TO THE AREA

			int numcamps = CampSearchKennethWilliams(loop,numchild,fp,maxInterface,0);
			fprintf (fp, "\t NumCampingPoints : %d \n",numcamps);
			CampSearchKennethWilliams(loop,numchild,fp,maxInterface,1);



			fprintf (fp,"%s\n","ENDAREA ");
		}

	}

 	int GlobChild;



	void ExportWay(Interface *maxInterface, FILE *fp)
	{
		fprintf (fp,"%s %d\n","NUMPATHS : ",NumPath);

		for (int loop=0;loop<NumPath;loop++)
		{
			fprintf (fp,"%s \n","WAYPATH ");

			int waytype = PATH;

			TCHAR* nodename = MapPaths[loop]->GetName();	
			if (!(strnicmp(nodename,"network",7)))
				waytype = NETWORK;
		   
			if (!(strnicmp(nodename,"entryspline",7)))
				waytype = ENTRYSPLINE;
			

			fprintf (fp,"%s %d\n","WAYTYPE:",waytype);
			Object *obj = MapPaths[loop]->EvalWorldState(maxInterface->GetTime()).obj;
			ShapeObject* shape = (ShapeObject*)obj;
			PolyShape pShape;
			shape->MakePolyShape(maxInterface->GetTime(), pShape);
			int numPts = pShape.lines->numPts;
			PolyPt *pt = pShape.lines->pts;


			//PULL OUT THE ROTATION MATRIX AND TRANSFORM OF THE SPLINE'S NODE
			
			
			Matrix3 transform = MapPaths[loop]->GetObjectTM(0);

			


			if (waytype != NETWORK)
			{
				Point3 rot;
				//NOW GET ITS ROWS FOR VERTEX TRANSFORMATION
				//Point3	trans = transform.GetTrans();
				//Point3 row1 = transform.GetRow(0);
				//Point3 row2 = transform.GetRow(1);
				//Point3 row3 = transform.GetRow(2);

				fprintf (fp, "\t NumWayPoints : %d\n",numPts);
				for (int points=0;points<numPts;points++,pt++)
				{

					//APPLY TRANFORMATION TO THE WAYPOINT AND OUTPUT		
				
				/*	rot.x =
						(pt->p.x * row1.x) +
						(pt->p.y * row1.y) +
						(pt->p.z * row1.z) + trans.x;
					rot.y = 
						(pt->p.x * row2.x) +
						(pt->p.y * row2.y) +
						(pt->p.z * row2.z) + trans.y;
					rot.z = 
						(pt->p.x * row3.x) +
						(pt->p.y * row3.y) +
						(pt->p.z * row3.z) + trans.z;*/
					//AKA 
					rot = pt->p * transform;
				  		fprintf (fp, "\t WayVertex : %f %f %f\n",rot.x,rot.y,rot.z);

				}
			}
			else
			{
				int numchild = MapPaths[loop]->NumberOfChildren();

	 			int numLines = pShape.numLines;


				NETNODE Node[256];
				NETNODE RealNode[256];
				for (int i=0;i<256;i++)
				{
					Node[i].collapsed = 0;
					Node[i].connectednum = 0;
					for (int test =0;test<16;test++)
					{
						Node[i].connected[test] = -1;
						Node[i].mask = 0;

					}
			   
				}
			int NODENUM=0;
			int FIRSTNODEINLINE = 0;
			for (int line = 0;line < numLines; line++)
			{
				numPts = pShape.lines[line].numPts;
				pt = pShape.lines[line].pts;
				int lastNODE = NODENUM;

				for (int points=0;points<numPts;points++,pt++)
				{
		
					//IF NOT THE FIRST NODE RECORDED AND NOT THE FIRST IN ANY
					//GIVEN NODE LINE RECORD IT AS BEING CONNECTED TO THE PREVIOUS

					if (points)
					{
						Node[NODENUM].connected[0]	= NODENUM-1;
						Node[NODENUM].connectednum = 1;
						Node[lastNODE].connected[Node[lastNODE].connectednum] = NODENUM;
						Node[lastNODE].connectednum ++;
						lastNODE = NODENUM;

						
					}
					else
					{
					 	FIRSTNODEINLINE = NODENUM;
					   	lastNODE = NODENUM;
						Node[NODENUM].connectednum = 0;
					} 
	  
					
					
				
					Node[NODENUM].pt = pt->p * transform;


						
					NODENUM++;

				}

				//connect last point to first if line is closed
				if ((points) && pShape.lines[line].IsClosed())
				{
					Node[NODENUM-1].connected[1] = FIRSTNODEINLINE;
					Node[NODENUM-1].connectednum = 2;
					Node[FIRSTNODEINLINE].connected[1] = NODENUM - 1;
					Node[FIRSTNODEINLINE].connectednum++;

					//NEW POINT MADE LINKING ENDPOINTS
					Node[NODENUM].connectednum = 2;
					Node[NODENUM].connected[0] = NODENUM - 1;
					Node[NODENUM].connected[1] = FIRSTNODEINLINE;
					Node[NODENUM].pt = Node[FIRSTNODEINLINE].pt;
					
					Node[NODENUM - 1].connected[2] = NODENUM;
					Node[NODENUM - 1].connectednum++;
					Node[FIRSTNODEINLINE].connected[2] = NODENUM;
					Node[FIRSTNODEINLINE].connectednum++;


					NODENUM++;

					

				}

			}

#if 0
			int loop2,nodes;
			for (int node=0;node < NODENUM;node ++)
			{
			   	TrueDupFound(Node,node,NODENUM);


			}
#endif

				//FIND DUPLICATE

				
				//MAKES THE SEARCH NODE ADD TO ITS CONNECTED LIST ANY NODE THAT HAS THE SAME VERTEX POS
				for (int nodes=0;nodes<NODENUM;nodes++)
				{
					for (int searchnodes=0;searchnodes<NODENUM;searchnodes++)
					{
						if ((searchnodes != nodes))
						{
							if (NodePointCompare(&Node[nodes].pt,&Node[searchnodes].pt))
							{
								int foundnode = 0;
								for (int test = 0; test < Node[nodes].connectednum;test++)
								{
									if (Node[nodes].connected[test] == searchnodes)
										foundnode++;
								}
								if (!foundnode)
								{

									Node[nodes].connected[Node[nodes].connectednum] = searchnodes;
									Node[nodes].connectednum++;
								}
							}
						}

					}

				}


				//search through node list merging common points and collapsing
				for (i=0;i<NODENUM;i++)
				{
					for (int search=0;search < NODENUM;search++)
					{
						//search node != this node, and not collapsed then consider the collapse
						if ((search!=i) && (!Node[search].collapsed))
						{
							if ((NodePointCompare(&Node[i].pt,&Node[search].pt)) && (!Node[i].collapsed))
							{
								//matt removed
								//if (i < search)
								//{
									Node[search].collapsed = 1;

									//NOW GO THROUGH THE NODES ANY NON=COLLAPSED ONES THAT REFERENCE
									//search REPLACE with i
									for (int searchnew = 0;searchnew < NODENUM; searchnew++)
									{
										if(searchnew != i)
										{
											for (int con=0;con<Node[searchnew].connectednum;con++)
											{
												if ((Node[searchnew].connected[con] == search))
												{
												
													Node[searchnew].connected[con] = i;
														
												//now go through all other nodes connect list any refs to search replace with i
											  	for (int loop=0;loop<NODENUM;loop++)
											  	{
											  		for (int con2=0;con2<Node[loop].connectednum;con2++)
											  		{
											  			if ((Node[loop].connected[con2] == search))
											   						Node[loop].connected[con2] = i;
											   		}
											   	}
												}
											}	
										}		
									}
								//}								
							}
						}

					}

				}


		  		for (i=0;i<NODENUM;i++)
		  			Node[i].collapsed = 0;

				for (int loop2=0;loop2<NODENUM;loop2++)
				{
					for (int search=0;search<NODENUM;search++)
					{
						if ((loop2!=search) && (!Node[loop2].collapsed))
						{
							if (NodePointCompare(&Node[loop2].pt,&Node[search].pt))
							{
							   //	if (loop2 < search)
							   //	{
									Node[search].collapsed = 1;
								
									for (int con = 0;con<Node[search].connectednum;con++)
									{
										int newnode = Node[search].connected[con];

										int found=0;
										for (int inner=0;inner<Node[loop2].connectednum;inner++)
										{
											if (Node[loop2].connected[inner] == newnode)
											{
												found++;
												break;
				
											}
	
										}

										if (!found)
										{
											Node[loop2].connected[Node[loop2].connectednum] = newnode;
											Node[loop2].connectednum++;
										}

									}
								//}
							}


						}

					}

				}

				

				#if DEBUG_NET
					//output initial connect list

					for (nodes=0;nodes<NODENUM;nodes++)
					{
						if (!Node[nodes].collapsed)
						{
							fprintf(fp,"\t INITNodeVertex: %d %f %f %f \n", nodes,Node[nodes].pt.x,Node[nodes].pt.y,Node[nodes].pt.z);	

							for (int i=0;i<Node[nodes].connectednum;i++)
								fprintf(fp,"\t CONNECTS: %d\n", Node[nodes].connected[i]);
						}		
					}
					fprintf(fp,"********END TEST******\n");
				#endif

				
				//COLLAPSE HANDLER
				//NOW GO THROUGH ALL NODES EXTRACTING A LIST OF UNIQUE NODE POSITIONS ONLY
				//FIRST RESOLVE ALL THE REFERENCES THEN COPY THE LIST
				int REALNODENUM = 0;

				for (i=0;i<NODENUM;i++)
				{
					if (!Node[i].collapsed)
					{

						for (loop2=0;loop2<NODENUM;loop2++)
						{
							if (!Node[loop2].collapsed)
							{
								for (int con=0;con<Node[loop2].connectednum;con++)
								{
									if (Node[loop2].connected[con] == i)
										Node[loop2].connected[con] = REALNODENUM;
	
								}
							}
						}

						REALNODENUM++;

					}


				}


				REALNODENUM = 0;


				int TESTARRAY[16];

				//remove duplicate node entries from each non-collapsed node
				//and make up a list of realnodes
				for (i=0;i<NODENUM;i++)
				{
					if (!Node[i].collapsed)
					{
						for (int loop3=0;loop3<16;loop3++)
							TESTARRAY[loop3] = -1;

						int current=0;
						for (loop3=0;loop3<Node[i].connectednum;loop3++)
						{
							int found = 0;
							for (loop2=0;loop2<current;loop2++)
							{
								if (TESTARRAY[loop2] == Node[i].connected[loop3])	
									found++;
							}

							if (!found)
							{
								TESTARRAY[current] = Node[i].connected[loop3];
								current++;


							}
						}


						for (int loop2=0;loop2<16;loop2++)
							Node[i].connected[loop2] = TESTARRAY[loop2];

						
						RealNode[REALNODENUM] = Node[i];							
					   	RealNode[REALNODENUM].connectednum = current;
						REALNODENUM++;

					}
				}


				
				#if DEBUG_NET
					for (nodes=0;nodes<REALNODENUM;nodes++)
					{
						fprintf(fp,"\t INITREALVertex: %d %f %f %f \n", nodes,RealNode[nodes].pt.x,RealNode[nodes].pt.y,RealNode[nodes].pt.z);

						for (int i=0;i<RealNode[nodes].connectednum;i++)
							fprintf(fp,"\t CONNECTS: %d\n", RealNode[nodes].connected[i]);

					}
					fprintf(fp,"********END REAL TEST******\n");
				#endif		
				
				
				
				fprintf (fp, "\t NumNetNodes : %d\n",REALNODENUM);

				//OUTPUT VERTEX LIST FOLLOWED BY CONNECTOR LIST
				int error = 0;
				for (nodes=0;nodes<REALNODENUM;nodes++)
				{
						//fprintf(fp,"\tnode %d num connected %d\n", nodes,RealNode[nodes].connectednum);
						fprintf(fp,"\t NodeVertex: %f %f %f \n", RealNode[nodes].pt.x,RealNode[nodes].pt.y,RealNode[nodes].pt.z);
						int connectedmask = 0;
						RealNode[nodes].mask = (NETTERMINATE<<24)|(NETTERMINATE<<16)|(NETTERMINATE<<8)|NETTERMINATE;
						RealNode[nodes].mask = 0;

						#if DEBUG_NET
							fprintf(fp,"**NODE %d has %d \n",nodes,RealNode[nodes].connectednum);
						#endif

						for (int i=0;i<RealNode[nodes].connectednum;i++)
						{

							if (RealNode[nodes].connected[i] != nodes)
							{
							//	fprintf(fp,"\t connected +1 %d\n",RealNode[nodes].connected[i]+1);
							//	fprintf(fp,"\t connectedmask %d\n",8*connectedmask);
								RealNode[nodes].mask |= ((RealNode[nodes].connected[i]+1)<<(8*connectedmask));
								connectedmask ++;
							}

						}

						if (connectedmask >4)
							error++;

					//fprintf(fp,"\t NodeMask: %d \n", RealNode[nodes].mask);	

					//	int err=0;
					//	dAssert(err != 0,"error\n");
					#if DEBUG_NET
						fprintf(fp,"\tmask1 %d \n", RealNode[nodes].mask&255);	
						fprintf(fp,"\tmask2 %d \n", (RealNode[nodes].mask>>8)&255);	
						fprintf(fp,"\tmask3 %d \n", (RealNode[nodes].mask>>16)&255);	
						fprintf(fp,"\tmask4 %d \n", (RealNode[nodes].mask>>24)&255);	
					#endif
				}		
				if (error)
				{
					if (ALLMAPMODE < GODMODEINTERMEDIATE)
						MessageBox (NULL, "TOO MANY CONNECTORS ON ONE NODE OF NAMED NETWORK, MAXIMUM ALLOWED 4, WILL CAUSE PROBLEMS",nodename, MB_OK);
					else
						fprintf(godlog,"TOO	MANY CONNECTORS ON ONE NODE OF NAMED NETWORK, MAXIMUM ALLOWED 4 %s \n",nodename);
				}
				for (nodes=0;nodes<REALNODENUM;nodes++)
						fprintf(fp,"\t NodeMask: %d \n", RealNode[nodes].mask);	


				//output info regarding attached patrol routes on this path

				fprintf(fp,"\t NumPatrolRoutes : %d \n", numchild);	

				

			 	for (int paths=0;paths<numchild;paths++)
			 //	for (int paths=0;paths<1;paths++)
				{

					Point3 PathPoint[256];

//					INode *path = MapPaths[loop]->GetChildNode(0);			

					INode *path = MapPaths[loop]->GetChildNode(paths);			

					int pathpointnum = BuildInitialPath(maxInterface,path,&PathPoint[0]);

					//NOW GO THROUGH AND CONVERT OUR INITIAL PATH TO NETWORK NODES
					fprintf(fp,"\t NumberNetWay: %d\n", pathpointnum);

					int bug = 0;
				
					for (int pathpt=0;pathpt<pathpointnum;pathpt++) 
					{
				
					//fprintf(fp,"\t NetPathVERT: X %f Y %f Z%f \n", PathPoint[pathpt].x,
					 //	PathPoint[pathpt].y,PathPoint[pathpt].z);		

						for (nodes=0;nodes<REALNODENUM;nodes++)
						{
					//	double x,y,z;

						//TO COVER ANY INACCURACY WHEN PLACING PATH OVER GRID
					  //	x = fabs(RealNode[nodes].pt.x - PathPoint[pathpt].x);
					  //	y = fabs(RealNode[nodes].pt.y - PathPoint[pathpt].y);
					  //	z = fabs(RealNode[nodes].pt.z - PathPoint[pathpt].z);
						
						
//						if ((x < 0.0001) && (y < 0.0001) && (z < 0.0001))

							if (NodePointCompare(&RealNode[nodes].pt,&PathPoint[pathpt]))
							{
								fprintf(fp,"\t NetPathNode: %d\n", nodes);		
							   	bug++;
								break;
							}
						//fprintf(fp,"\t CHECKING: X %f Y %f Z%f \n",
						//	RealNode[nodes].pt.x,
						//	RealNode[nodes].pt.y,
						//	RealNode[nodes].pt.z);

						//if (RealNode[nodes].pt == PathPoint[pathpt])
						//	fprintf(fp,"\t NetPathNode: %d\n", nodes);		
						}
					}

					if (bug < pathpointnum)
					{
						if (ALLMAPMODE < GODMODEINTERMEDIATE)
					 		MessageBox (NULL, "ENSURE ALL PATROL ROUTES ARE SNAPPED TO THE NETWORK VERTICES",nodename, MB_OK);
						else
							fprintf(godlog,"ENSURE ALL PATROL ROUTES ARE SNAPPED TO THE NETWORK VERTICES %s \n",nodename);
					}
					
			 }
			}

						


			//CHECK FOR A SURROUNDING AREA FOR THE PATH

			int Area = -1;


			//THE PARENT OF A SPLINE PATH, IF NAMED "BBox...", IS ITS BOUND AREA
				
			INode *parent = MapPaths[loop]->GetParentNode();			
			nodename = parent->GetName();	

			TCHAR *name = MapPaths[loop]->GetName();
			//MAKE SURE IT'S A VALID AREA NAME, BIT NODDY BUT BETTER THAN NOWT
			if (!(strnicmp(nodename,"BBox",4)))
			{
				Area = AddArea(parent,MapPaths[loop]);
			}	
					


			fprintf (fp,"\t PathArea : %d \n",Area);

			fprintf (fp,"%s\n","ENDWAYPATH ");
		}

	}


	//GETS THE INDEX FOR A GIVEN TRIGGER
	int	GetTrig(INode *trigger)
	{
		for (int i=0;i<NumTrig;i++)
		{
			if (MapTrigs[i] == trigger) 
				return(i);
		}
		return (-1);
	}


	void GetStratsAttachedToDeletionEvent(INode *node, INode *parent, int writemode, FILE *fp)
	{
		int i,numchild;
		INode *oldnode=node;
		
		numchild = node->NumberOfChildren();

	  	RDObject *obj = (RDObject*)node->GetObjectRef();
		if (obj && (obj->ClassID() == RDOBJECT_CLASS_ID))
		{
			if (obj->deletioneventpoint == parent)
			{
				GlobChild++;
			 	if (writemode)
				{
					int entry = AddStrat(node);
					//if (entry < 0)
					//	MessageBox (NULL, "TRIGGER REFERENCED WITH NO ATTACHED STRATS",child->GetName(), MB_OK);
					fprintf (fp, "\t\t\tDependentStrat : %d\n",entry);
				}
			}
		}


		for (i=0;i<numchild;i++)
		{
			node = oldnode->GetChildNode(i);
			GetStratsAttachedToDeletionEvent(node,parent,writemode,fp);
		}


		return;


	}



	//NEED TO EXPORT RADIUS AND POSITION OF CIRCLE

	void ExportEvent(int Index,Interface *maxInterface, FILE *fp)
	{

		short trigflag = 0;

		//SET THE NODE TO CHECK AGAINST
		INode *Trigger = MapEvents[Index].node;

		TCHAR* name = Trigger->GetName();


	 
		if (!strnicmp(Trigger->GetName(),"EVENTDELETE",11))
		{
			//need to check whether any strats has this deletion as an event
			//FRIIIDAAAYYYY

			//CHECK ALL STRATS FOR ONES POINTING TO THIS TRIGGER
			INode *newnode = GetCOREInterface()->GetRootNode();
		   	GlobChild = 0;
			GetStratsAttachedToDeletionEvent(newnode,Trigger,0,fp);
			fprintf (fp, "\t\tNumberOfTrigChildren : %d\n",GlobChild);
			GetStratsAttachedToDeletionEvent(newnode,Trigger,1,fp);



		}
		else
		{
			if (Trigger->NumberOfChildren()==0)
			{
				if (ALLMAPMODE < GODMODEINTERMEDIATE)
					MessageBox (NULL, "EVENT TRIGGER NEEDS ATTACHMENTS",name, MB_OK);
				else
					fprintf(godlog,"EVENT TRIGGER NEEDS ATTACHMENTS %s \n",name);

			}

			fprintf (fp, "\t\tNumberOfTrigChildren : %d\n",Trigger->NumberOfChildren());
			for (int i=0;i<Trigger->NumberOfChildren();i++)
			{
				INode *child = Trigger->GetChildNode(i);
				RDObject *obj = (RDObject*)child->GetObjectRef();

				if (obj->eventpoint == Trigger)
				{
					if (ALLMAPMODE < GODMODEINTERMEDIATE)
						MessageBox (NULL, "THIS EVENT HAS A DUPLICATE DEPENDENT AND CREATOR",Trigger->GetName(), MB_OK);
					else
						fprintf(godlog,"THIS EVENT HAS A DUPLICATE DEPENDENT AND CREATOR %s \n",Trigger->GetName());
				}
				else
				{


					int entry = AddStrat(child);
					if (entry < 0)
					{
						if (ALLMAPMODE < GODMODEINTERMEDIATE)
							MessageBox (NULL, "TRIGGER REFERENCED WITH NO ATTACHED STRATS",child->GetName(), MB_OK);
						else
							fprintf(godlog,"TRIGGER REFERENCED WITH NO ATTACHED STRATS %s \n",child->GetName());
					}
					fprintf (fp, "\t\t\tDependentStrat : %d\n",entry);
				 }
			}
		}
	}


	//NEED TO EXPORT RADIUS AND POSITION OF CIRCLE

	void ExportTrigger(int Index,Interface *maxInterface, FILE *fp)
	{

		short trigflag = 0;

		//SET THE NODE TO CHECK AGAINST
		INode *Trigger = MapTrigs[Index];


		//IF TRIGGER HAS A PARENT THEN DO NOT MAKE IT IMMEDIATELY ACTIVE
		//MAKE IT TRIGGER DEPENDENT
		if (!Trigger->GetParentNode())
			trigflag |= ACTIVETRIG;
		else
			trigflag |= DEPENDANTTRIG;

		TCHAR *name = Trigger->GetName();

		
		TCHAR array1[32];
		TCHAR array2[32];

		for (int i=0;i<32;i++)
			array1[i] = array2[i] = '\0';

		int timer=0,event=0;
		int count=0;

		TCHAR* ArrPt = name;

		//GET RID OF LEADING SPACES

	   //	while (*ArrPt == ' ')
		//	ArrPt++;

		ArrPt = WhiteSpaceRead(ArrPt);
		

		
		//sscanf(ArrPt,"%s ",&array1);

		//ArrPt += strlen(array1);

		//GET RID OF LEADING SPACES

	   //	while (*ArrPt == ' ')
	   //		ArrPt++;


		//sscanf(ArrPt," %s",&array2);

	  //	if (
	  //		( (!(strnicmp(ArrPt,"TIMER",5))) && (!(strnicmp(ArrPt,"TIMER",5)))) ||
	  //		( (!(strnicmp(ArrPt,"EVENT",5))) && (!(strnicmp(ArrPt,"EVENT",5)))))
	  //			MessageBox (NULL, "INVALID TRIGGER TYPE",ArrPt, MB_OK);
		
	  //	else
	  //	{
		
			timer = 
				(!(strnicmp(ArrPt,"TIMER",5))) |
				(!(strnicmp(ArrPt,"TIMER",5))) ;

			event = 
				(!(strnicmp(ArrPt,"EVENT",5))) |
				(!(strnicmp(ArrPt,"EVENT",5))) ;

			//AN EVENT TRIGGER SHOULD HAVE A CHILD TRIGGER CONNECTED, IF NOT ERROR
		/*	if (event)
				if (Trigger->NumberOfChildren()==0)
				{
					event = 0;
					MessageBox (NULL, "EVENT TRIGGER NEEDS ATTACHMENTS",name, MB_OK);
				}
				else
					trigflag |= EVENTTRIG;

		  */
			if (timer)
			{
				//if ((strnicmp(array1,"TIMER",5))) 
				//	ArrPt = array1;
				//else
				//	ArrPt = array2;
			 
			 //		MessageBox (NULL, "TIMER TRIGGER1 ",ArrPt, MB_OK);
				//ArrPt+=5;
			  //		MessageBox (NULL, "TIMER TRIGGER2 ",ArrPt, MB_OK);
				
				//IF TIMER, THEN PROCESS IT
				ArrPt = ArrPt + 5;
				
				ArrPt = WhiteSpaceRead(ArrPt);

				
				//while (*ArrPt == ' ')
				//	ArrPt++;

			//   	MessageBox (NULL, "TIMER TRIGGER ",ArrPt, MB_OK);

			  	//should really be parsing for a valid numeric value here
				// Altered by MattG to make it work even if someone's duff enough not to put a trigger in
				if ((!strlen(ArrPt)) || (!(sscanf(ArrPt,"%d ",&count))))
				{
					MessageBox (NULL, "TIMER TRIGGER NEEDS A VALUE",name, MB_OK);
					strcpy (array1, "2");
				}
			 	trigflag |= TIMERTRIG;


		
			}
		//}
		
		//output its flag
  		fprintf (fp, "\tTriggerFlag : %d\n",trigflag);

		if (timer)
		{
			itoa(count,array1,10);
			fprintf (fp, "\t\tTriggerActivateCount : %s\n",array1);
		}

	 
		event = 0;
		if (event)
		{
			fprintf (fp, "\t\tNumberOfTrigChildren : %d\n",Trigger->NumberOfChildren());
			for (int i=0;i<Trigger->NumberOfChildren();i++)
			{
				INode *child = Trigger->GetChildNode(i);

				RDObject *obj = (RDObject*)child->GetObjectRef();

				if (obj->eventpoint == Trigger)
				   	MessageBox (NULL, "THIS EVENT HAS A DUPLICATE DEPENDENT AND CREATOR",Trigger->GetName(), MB_OK);


				
				//			    	int entry = GetTrig(child);
				int entry = AddStrat(child);
				if (entry < 0)
					MessageBox (NULL, "TRIGGER REFERENCED WITH NO ATTACHED STRATS",child->GetName(), MB_OK);
				fprintf (fp, "\t\t\tDependentStrat : %d\n",entry);


			}
		   //	fprintf (fp, "\t\tNumberOfTrigMasters : %d\n",AssignedStrats[Index].stratnum);
		   //	for (i=0;i<AssignedStrats[Index].stratnum;i++)
			//	fprintf (fp, "\t\t\tMasterStrat : %d\n",AssignedStrats[Index].stratid[i]);






		}

	  





		Object *obj = Trigger->EvalWorldState(maxInterface->GetTime()).obj;
		ShapeObject* shape = (ShapeObject*)obj;

		IParamArray *array = shape->GetParamBlock();

		
		assert (array);
		float radius;
		array->GetValue (CIRCLE_RADIUS, TimeValue(0), radius, FOREVER);

	   //	Point3 xvec;
	  //	xvec.x = radius;
	  //	xvec.y = 0;
	  //	xvec.z = 0;
		Matrix3 transform = Trigger->GetNodeTM(0);
	   //	Point3 row1 = transform.GetRow(0);

		//APPLY TRANFORMATION TO THE WAYPOINT AND OUTPUT		
				
	  //	Point3 tvec;
	   //	tvec.x = (xvec.x * row1.x);

		//PULL OUT THE ROTATION MATRIX AND TRANSFORM OF THE AREAS NODE
			
		Point3	trans = transform.GetTrans();

	//	radius = tvec.x;

		radius = ComputeScaledRadius(radius,&transform);


		//OUTPUT THE RADIUS
		fprintf (fp, "\tTriggerRadius : %f\n",radius);


		//OUTPUT THE NODE POSITION
					
		fprintf (fp, "\tTriggerVertex : %f %f %f\n",trans.x,trans.y,trans.z);
	  

	}

	//NEED TO EXPORT RADIUS AND POSITION OF ACTIVATION CIRCLE

	void ExportActivation(Interface *maxInterface,INode *Activation, FILE *fp)
	{

		Object *obj = Activation->EvalWorldState(maxInterface->GetTime()).obj;
		ShapeObject* shape = (ShapeObject*)obj;

		IParamArray *array = shape->GetParamBlock();
		assert (array);
		float radius;
		array->GetValue (CIRCLE_RADIUS, TimeValue(0), radius, FOREVER);

		Matrix3 transform = Activation->GetObjectTM(0);
		Point3	trans = transform.GetTrans();

		radius = ComputeScaledRadius(radius,&transform);


		//OUTPUT THE RADIUS
		fprintf (fp, "\tActivationRadius : %f\n",radius);


		//OUTPUT THE NODE POSITION
					
		fprintf (fp, "\tActivationVertex : %f %f %f\n",trans.x,trans.y,trans.z);
	  

	}


};


/*
 * Red Dog Export Class description
 */

class RDStratExportClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create (BOOL loading = FALSE) {return new RDStratExport();}
	const TCHAR *	ClassName() {return GetString(IDS_STRAT_CLASS);}

	//SUPER CLASS
	SClass_ID		SuperClassID() {return SCENE_EXPORT_CLASS_ID;}

	Class_ID		ClassID() {return RDSTRATEXPORT_CLASS_ID;}
	const TCHAR* 	Category() {return GetString(IDS_CATEGORY);}
	void			ResetClassParams (BOOL fileReset);
};

static RDStratExportClassDesc RDStratExportDesc;
ClassDesc* GetRDStratExportDesc() {return &RDStratExportDesc;}

//TODO: Should implement this method to reset the plugin params when Max is reset
void RDStratExportClassDesc::ResetClassParams (BOOL fileReset) 
{

}
