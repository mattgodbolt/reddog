//CRASH OUT BARS - WHITE AND WHITE TRIGGERS
//				 - BLACK OUT OF OBJ SPACE
//				 - BLUE, OVER THE MAXSOUND ALLOCATION
//				 - RED AND WHITE, NO ANIMS LEFT
//				 - BLUE AND  BLACK IVARS
//				 - GREEN AND BLACK FVARS
//				 - RED AND BLACK STRATS
//				 - RED AND GREEN: NO SOUND BLOCK MATEY
//				 - CYAN AND MAGENTA: OUT OF RAM IN FE


   
#include "reddog.h"
//#include "Dirs.h"

//#include "strat.h"
#include "command.h"
//#include "process.h"
#include "camera.h"
#include "coll.h"
//#include "localdefs.h"
#include "level.h"
#include "stratway.h"
#include "input.h"
#include "stratphysic.h"
#include "mplayer.h"
#include "view.h"
#include "Render\Sparks.h"
#include "Render\SpecialFX.h"
#include "Loading.h"
#include "Render\Internal.h"
//#include "GameState.h"

#ifndef	LEVELST_H
#include "levelst.h"
#endif
 


extern int suppressDebug;


#if SH4
	#undef fread
	#define fread(a,b,c,d) sRead(a,(b)*(c),d)
#endif

#define COLD 0 
#define WARM  1	  

#ifdef _DEBUG
	int deletedstrats;
#endif


static	void 	RemoveStrat(STRAT *strat);
static	void	TrigCheck(Point3 *pos);
static	void	EventCheck();
static	int		AheadOfPlayer(STRAT *strat,float dist);
void			UnRegisterAsTarget(STRAT *strat);
int				PlayerNearCheckPos(STRAT *strat,float dist);

//Uint32	globalStratID = 0;
PNode			*Dome;
static PNode	*GameObjects;
PNode	*GameHeads[MAXCHARACTERS];
static PNode	*GameMods[MAXMODS];

int				MultiTankType[4] = {0,1,2,3};

CAMMODIFIER		*CamModArray;
static STRATANIMMATT	*GameAnims;

/*Animation		*ModelAnims;*/


WAYPATH			*MapPaths;
MAPAREA			*MapAreas;
MAPTRIGGER		*MapTrigs;

/*MATT ADD */
MAPEVENT		*MapEvents;
MAPACTIVATION  	*MapActs;
LEVELSETTINGS   *LevelSettings;

short Skill=EASY;
bool MissionBriefingDone = FALSE;

#define TRIGSET	{strat->timer=trig->internalloop;strat->stack=trig->internalstack;}

#define TRIGRESTORE	{strat->timer=strat->timeblock;strat->stack=strat->stackblock;}

STRATMAP *GameMap;

unsigned char PlayerLives;

void newObject(Object *obj, Object *newObj, int clearcrnt);
void setObjId(Object **newObjId, Object **oldObjId, Object *newObj, Object *oldObj, int	noObjId);
void newObjectFree(Object *obj);

static int STRATNUM=0;
int NumSpawnPoints = 0;

STRAT DummStrat;

STRAT *OldPlayer = NULL;
STRAT *NextFreeStrat;
STRAT *FirstStrat;
STRAT *BossStrat;

GAMETARGETS *FirstTarget;
GAMETARGETS *NextFreeTarget;

TRIGGER		*NextFreeTrig,*LastTrig;
ANIMINST	*NextFreeAnim,*LastAnim;
MODELANIMINST	*NextFreeModelAnim,*LastModelAnim;

GAMETARGETS TargetPool[MAX_GAMETARGETS]; /*POOL OF TARGETTABLE-BY-ENEMY STRATS CAN MAKE DYNAMICALLY ALLOCATED*/
STRAT  StratPool[MAX_STRATS];		/*POOL OF STRATS CAN MAKE DYNAMICALLY ALLOCATED*/
ANIMINST  AnimPool[MAX_STRATANIM];	/*POOL OF ANIMS  */
MODELANIMINST ModelAnimPool[MAX_MODELANIM]; /*POOL OF MODEL ANIMS*/
TRIGGER	  Triggers[MAX_TRIGGERS];	/*POOL OF TRIGGERS   CAN MAKE DYNAMICALLY ALLOCATED*/
SCRIPTENTRY GameScript[MAX_SCRIPTS]; /*Game scripts CAN MAKE DYNAMICALLY ALLOCATED*/
static short	*GameScenes[MAX_SCENES]; /*SCENES or ptrs to lists of script entries, again may be dynamically done*/


ROUTE	StratRoutes[MAX_STRATROUTES]; /*POOL OF ROUTE FINDING INFO */
ROUTE   *NextFreeRoute,*LastRoute;


COLLSTRAT StratColls[MAX_STRATCOLLS];
struct collstrat *NextFreeColl,*FirstColl;

/*MATT ADD */
static short	NumTrigs;					/*THE NUMBER OF TRIGGER POINTS IN THE MAP */
static short	NumEvents;					/*THE NUMBER OF EVENTS IN THE MAP */
static short	NumActs;					/*THE NUMBER OF ACTIVATION POINTS IN THE MAP */
static int		NumStrats;					/*NUMBER OF STRATS IN THE MAP */
static int		NumLights;					/* Number of lights in the map */

static int NumMod=0;
static int NumAnims=0;

STRAT *CamTracks[MAXCAM];

Vector3 CurrentRestart;
Vector3 CurrentRestartLook;
int Restart;
static void QuickKill(STRAT *strat)
{
	Delete(strat);

}


void SetPlayerToTarget(STRAT *strat, int valid)
{
	if ((strat->target) && (valid))
		player[currentPlayer].Player = strat->target;

}


void SetPlayerBack(STRAT *strat)
{
	if (strat->target)
		player[currentPlayer].Player = OldPlayer;

}

/*TRIGGER PROCESSING GENERAL :- CHECK FOR CONDITION */
/*IF MET, CHANGE TO TRIGGER STACK, EXECUTE TRIGGER STRAT CODE, THEN CHANGE */
/*BACK TO STRAT STACK */

void	ResetTrig(STRAT *strat)
{
	/*MONDAY*/
	/*RESET THIS CURRENT TRIGGER'S TIMER AND STACK*/
	Reset(strat);
	/*CHANGE BACK TO STRAT STACK*/

	TRIGRESTORE
	/*END MONDAY*/
	Reset(strat);
	strat->trigger->flag |= TRIG_CHANGED;
}

void RestoreTrig(STRAT *strat)
{
  /*	int i;*/
  /*	if (strat->trigger->flag & TRIG_CHANGED)
	{
		for (i=0;i<6;i++)
		{
			strat->timer[i] = strat->trigger->internalloop[i];	
		
			strat->stack[i] = strat->trigger->internalstack[i];	

		}
		strat->trigger->flag = strat->trigger->flag & (0xffff - TRIG_CHANGED);
	}*/

	TRIGRESTORE


}

void setParent(Object *obj, Object *parent)
{
	int		i;

	obj->parent = parent;
	for (i=0; i<obj->no_child; i++)
		setParent(&obj->child[i], obj);
}

static void	Every(STRAT *strat,TRIGGER *trig)
{

	/*TRIGGER PROCESSING FOR EVERY X FRAMES */

/*	if (strat->pnode) */
/*	dLog("trig running %s\n",strat->pnode->name); */

	if (!(trig->flag & TRIG_HELD))
	{
		if (trig->flag & TRIG_RUNNING)
		{

		/*	dLog("still running\n"); */
			TRIGSET		
			trig->address(strat);
		   	RestoreTrig(strat);

		   /*TRIGRESTORE*/
		}
		else
		{	
			trig->trig_timer++;
			if (trig->trig_timer == trig->trig_count)
			{
				TRIGSET
				trig->flag |= TRIG_RUNNING;
				trig->address(strat);
		   		RestoreTrig(strat);

				//TRIGRESTORE
				trig->trig_timer = 0;	
			}
		   /*	else */
			 /*	trig->trig_timer++; */
		}
	}
}

/*STRAT TO STRAT COLLISION */

static void	WhenHit(STRAT *strat,TRIGGER *trig)
{

	/*dLog("come in here\n"); */
	if (!(trig->flag & TRIG_HELD))
	{
		if (trig->flag & TRIG_RUNNING)
		{
			/*if (strat->pnode) */
			/*	dLog("hit checks %s\n",strat->pnode->name); */

			TRIGSET		
			trig->address(strat);
		   	RestoreTrig(strat);

			//TRIGRESTORE
		}
		else
		{		
		/*	dLog("checking\n"); */
			/*dLog("%d\n", strat->flag); */
			if (strat->flag & COLL_HITSTRAT)
			{
			/*	if (strat->pnode) */
			/*		dLog("hit checks2 %s\n",strat->pnode->name); */

				/*strat->flag &= (0xffff - STRAT_HITSTRAT); */
				TRIGSET		
				trig->flag |= TRIG_RUNNING;
				trig->address(strat);
		   		RestoreTrig(strat);

			   //	TRIGRESTORE
			   /*	strat->flag &= (0xffffffff - COLL_HITSTRAT); */
			
			}
		}
	}

}

static void	OnFloor(STRAT *strat,TRIGGER *trig)
{
	if (!(trig->flag & TRIG_HELD))
	{
		if (trig->flag & TRIG_RUNNING)
		{
			TRIGSET		
			trig->address(strat);
		   	RestoreTrig(strat);

			//TRIGRESTORE
		}
		else
		{		
		 	if (strat->flag & STRAT_HITFLOOR)
		 //	if (strat->flag & COLL_ON_FLOOR)
			{
				TRIGSET		
				trig->flag |= TRIG_RUNNING;
				trig->address(strat);
		   		RestoreTrig(strat);

				//TRIGRESTORE
			}
		}
	}
}

static void	PlayerCollide(STRAT *strat)
{
}

static void  WhenNearWaypoint(STRAT *strat,TRIGGER *trig)
{
	
	/*WHEN WAYPOINT REACHED EXECUTE TRIGGER CODE	 */
	if (!(trig->flag & TRIG_HELD))
	{
		if (trig->flag & TRIG_RUNNING)
		{
			/*dLog("strat running %x\n",strat); */
			TRIGSET		
			trig->address(strat);
		   	RestoreTrig(strat);

			//TRIGRESTORE
		}
		else
		{		
			if (NearCheckPos(strat,trig->trig_count))
			{
				/*dLog("strat running %x\n",strat); */
				TRIGSET		
				trig->flag |= TRIG_RUNNING;
				trig->address(strat);
		   		RestoreTrig(strat);

				//TRIGRESTORE
			}
		}
	}
}


static void  WhenNearPlayer(STRAT *strat, TRIGGER *trig)
{
	/*WHEN NEAR THE PLAYER EXECUTE TRIGGER CODE */
	
	if (NearPlayer(strat,trig->trig_count))
	{
		TRIGSET
		trig->flag |= TRIG_RUNNING;
		trig->address(strat);
		RestoreTrig(strat);

		//TRIGRESTORE
	}

}

static void  WhenAheadPlayer(STRAT *strat, TRIGGER *trig)
{
	/*WHEN AHEAD OF THE PLAYER EXECUTE TRIGGER CODE */
/*	dLog("ahead player trig\n"); */
	if (AheadOfPlayer(strat,trig->trig_count))
	{
		TRIGSET
		trig->address(strat);
		RestoreTrig(strat);

		//TRIGRESTORE
	}

}

static void  WhenParentDead(STRAT *strat, TRIGGER *trig)
{
	/*WHEN PARENT IS KILLED OFF EXECUTE TRIGGER CODE */
	
	if (!(trig->flag & TRIG_HELD))
	{
		if (strat->parent->flag==STRAT_DEAD)
		{
			TRIGSET
			trig->address(strat);
		   	RestoreTrig(strat);

			//TRIGRESTORE
		}
	}
}

static void  WhenPlayerDead(STRAT *strat, TRIGGER *trig)
{

		
}

static void  WhenNearActive(STRAT *strat,TRIGGER *trig)
{
	
	/*WHEN WAYPOINT REACHED EXECUTE TRIGGER CODE	 */
	if (!(trig->flag & TRIG_HELD))
	{
		if (trig->flag & TRIG_RUNNING)
		{
			/*dLog("strat running %x\n",strat); */
			TRIGSET		
			trig->address(strat);
		   	RestoreTrig(strat);

			//TRIGRESTORE
		}
		else
		{		
			if (PlayerNearCheckPos(strat,trig->trig_count))
			{
				/*dLog("strat running %x\n",strat); */
				TRIGSET		
				trig->address(strat);
		   		RestoreTrig(strat);

				//TRIGRESTORE
			}
		}
	}
}

static void  WhenAnimationOver(STRAT *strat,TRIGGER *trig)
{
	
	/*WHEN END ANIMATION REACHED EXECUTE TRIGGER CODE	 */
	if (!(trig->flag & TRIG_HELD))
	{
		if (trig->flag & TRIG_RUNNING)
		{
			TRIGSET		
			trig->address(strat);
		   	RestoreTrig(strat);

			//TRIGRESTORE
		}
		else
		{		
			if ((!strat->anim) || (strat->flag & STRAT_HIDDEN) || (strat->anim->anim.flags & ANIM_FLAG_TRIGGERED))
			{
				TRIGSET		
				trig->address(strat);
		   		RestoreTrig(strat);

				//TRIGRESTORE
			}
		}
	}
}

/*TO TRIGGER ON HEALTH GOING BELOW 0 */
static void  WhenDead(STRAT *strat,TRIGGER *trig)
{
	
	/*WHEN END ANIMATION REACHED EXECUTE TRIGGER CODE	 */
	if (!(trig->flag & TRIG_HELD))
	{
		if (trig->flag & TRIG_RUNNING)
		{
			TRIGSET		
			trig->address(strat);
		   	RestoreTrig(strat);

			//TRIGRESTORE
		}
		else
		{		
			if (strat->health <= 0)
			{
				TRIGSET		
				trig->address(strat);
		   		RestoreTrig(strat);

				//TRIGRESTORE
			}
		}
	}
}

/*TO TRIGGER ON STRAT GOING OUTSIDE IT'S ALLOTTED PATH REGION */
static void  WhenOutside(STRAT *strat,TRIGGER *trig)
{	
	if (!(trig->flag & TRIG_HELD))
	{
		if (trig->flag & TRIG_RUNNING)
		{
			TRIGSET		
			trig->address(strat);
		   	RestoreTrig(strat);

			//TRIGRESTORE
		}
		else
		{		
			if (InsideArea(strat) <= 0)
			{
				TRIGSET		
				trig->address(strat);
			   	RestoreTrig(strat);

				//TRIGRESTORE
			}
		}
	}
}


static void  WhenParentCreated(STRAT *strat,TRIGGER *trig)
{
	
	/*WHEN STRAT'S PARENT IS CREATED */
	if (!(trig->flag & TRIG_HELD))
	{
		if (trig->flag & TRIG_RUNNING)
		{
			/*dLog("strat running %x\n",strat); */
			TRIGSET		
			trig->address(strat);
		   	RestoreTrig(strat);

			//TRIGRESTORE
		}
		else
		{		
			if ((MyParentInvalid(strat)) && (GameMap[(int)strat->parent].strat))
			{
				/*dLog("strat running %x\n",strat); */
				TRIGSET		
				trig->address(strat);
			   	RestoreTrig(strat);

				//TRIGRESTORE
			}
		}
	}
}

static void  WhenHitStrat(STRAT *strat,TRIGGER *trig)
{
	
	/*WHEN STRAT'S PARENT IS CREATED */
	if (!(trig->flag & TRIG_HELD))
	{
		if (trig->flag & TRIG_RUNNING)
		{
			/*dLog("strat running %x\n",strat); */
			TRIGSET		
			trig->address(strat);
		   	RestoreTrig(strat);

			//TRIGRESTORE
		}
		else
		{		
			if (strat->flag & COLL_HITSTRAT)
			{
				/*dLog("strat running %x\n",strat); */
				TRIGSET		
				trig->flag |= TRIG_RUNNING;
				trig->address(strat);
			   	RestoreTrig(strat);

				//TRIGRESTORE
			}
		}
	}
}
static void  WhenTargetDead(STRAT *strat,TRIGGER *trig)
{
	
	/*WHEN STRAT'S PARENT IS CREATED */
	if (!(trig->flag & TRIG_HELD))
	{
		if (trig->flag & TRIG_RUNNING)
		{
			/*dLog("strat running %x\n",strat); */
			TRIGSET		
			trig->address(strat);
		   	RestoreTrig(strat);

			//TRIGRESTORE
		}
		else
		{		
			if (strat->target->flag == STRAT_DEAD)
			{
				/*dLog("strat running %x\n",strat); */
				TRIGSET		
				trig->flag |= TRIG_RUNNING;
				trig->address(strat);
			   	RestoreTrig(strat);

				//TRIGRESTORE
			}
		}
	}
}

static void  WhenPaused(STRAT *strat,TRIGGER *trig)
{
	
	/*WHEN STRAT'S PARENT IS CREATED */
	if (!(trig->flag & TRIG_HELD))
	{
		if (trig->flag & TRIG_RUNNING)
		{
			/*dLog("strat running %x\n",strat); */
			TRIGSET		
			trig->address(strat);
		   	RestoreTrig(strat);

		   //	TRIGRESTORE
		}
		else
		{		
			if (PauseMode)
			{
				/*dLog("strat running %x\n",strat); */
				TRIGSET		
				trig->flag |= TRIG_RUNNING;
				trig->address(strat);
			   	RestoreTrig(strat);

				//TRIGRESTORE
			}
		}
	}
}

static void  WhenDeleted(STRAT *strat,TRIGGER *trig)
{
	
}







/* */
static void  *TriggerTable[TRIGTYPE]=
{
	&Every,
	&WhenHit,
	&OnFloor,
	&PlayerCollide,
	&WhenNearPlayer,
	&WhenNearWaypoint,
	&WhenParentDead,
	&WhenPlayerDead,
	&WhenAheadPlayer,
	&WhenNearActive,
	&WhenAnimationOver,
	&WhenDead,
	&WhenOutside,
	&WhenParentCreated,
	&WhenHitStrat,
	&WhenTargetDead,
	&WhenPaused,
	&WhenDeleted
};


static TRIGGER *CurrentTrig=NULL;
static TRIGGER *NextTrig=NULL;
static int trigs=0;

void ProcTrigger(STRAT *strat,TRIGGER *trig)
{
	TRIGGER *Next;
	void (*address)(STRAT *strat, TRIGGER *trig);
	while (trig)
	{
		/*PROCESS THE APPROPRIATE TRIGGER */

		address = TriggerTable[trig->trig_type];
		CurrentTrig = trig;
		Next = trig->next_trig;
		address(strat,trig);
	 	trig = Next;
	  //	trig=trig->next_trig;
	}

	CurrentTrig = NULL;
}

//immediately envoke a callback
void ProcTriggerDeleted(STRAT *strat,TRIGGER *trig)
{
   //	void (*address)(STRAT *strat, TRIGGER *trig);
	while (trig)
	{
		/*PROCESS THE APPROPRIATE TRIGGER */
		if (trig->trig_type == WHENDELETED)
		{
			//address = TriggerTable[trig->trig_type];
			
			CurrentTrig = trig;
		   	TRIGSET		
			//trig->flag |= TRIG_RUNNING;
			trig->address(strat);
			RestoreTrig(strat);

			TRIGRESTORE
			//address(strat,trig);
		}
		trig=trig->next_trig;
	}

	CurrentTrig = NULL;
}

void ProcTriggerPaused(STRAT *strat,TRIGGER *trig)
{
	void (*address)(STRAT *strat, TRIGGER *trig);
	while (trig)
	{
		/*PROCESS THE APPROPRIATE TRIGGER */
		if (trig->trig_type == WHENPAUSED)
		{
			address = TriggerTable[trig->trig_type];
			CurrentTrig = trig;
			address(strat,trig);
		}
		trig=trig->next_trig;
	}

	CurrentTrig = NULL;
}


void UpdateTrigFlag(int flag)
{
	CurrentTrig->flag |= flag;

}



/*WHEN CALLED FROM TRIGGER WILL PAUSE ALL BUT THE CALLING TRIGGER AND ANY SPECIFIED EXCEPTION TYPES
  WHEN CALLED OUTSIDE OF TRIGGER WILL PAUSE ALL BUT ANY SPECIFIED EXCEPTION TYPES*/
/*-2 big exception*/
void PauseTriggers(STRAT *strat,int Except, int Forget)
{
	int i;
	TRIGGER *Next;
	TRIGGER *trig = strat->trigger;
	void (*address)(STRAT *strat, TRIGGER *trig);

	while (trig)
	{
		if (Except == -2)
		{
			trig->flag |= TRIG_HELD;

		}
		else
		{

		if ((trig!=CurrentTrig))
		{
		  				
			//PAUSE THE TRIGGER IF IT DOES NOT MEET THE EXCEPTION CRITERIA AND IT'S NOT UNPAUSEABLE
			if ((trig->trig_type != Except) && (!(trig->flag & TRIG_UNPAUSEABLE)))
			{
				if ((!(trig->flag & TRIG_HELD)) || Forget)
				{
		   			trig->pausedby = CurrentTrig;
					trig->flag |= TRIG_HELD;
					if (Forget)
					{
						for (i=0;i<6;i++)
							trig->internalloop[i] = trig->internalstack[i] = 0;

					}
				}


			}

		}
		}
		Next = trig->next_trig;
		trig=trig->next_trig;
	}

  /*	CurrentTrig = NULL;	  */
}

/*REVERSE OF ABOVE*/

void UnPauseTriggers(STRAT *strat)
{
	TRIGGER *Next;
	TRIGGER *trig = strat->trigger;
	void (*address)(STRAT *strat, TRIGGER *trig);

	while (trig)
	{
		if ((trig!=CurrentTrig) && ((trig->pausedby == CurrentTrig) || (!trig->pausedby)))
			trig->flag &= SLNOT(TRIG_HELD);

		Next = trig->next_trig;
		trig=trig->next_trig;
	}

  /*	CurrentTrig = NULL;			*/
}



/*CREATE A TRIGGER OF TYPE AND PARAM SET, TO RUN CODE AT ADDRESS WHEN MET */

void CreateTriggerParam(STRAT *strat,void *address,int TrigType,float TrigParam)
{
	TRIGGER *trig;
	int i;

	trigs++;
	
/*	trig = NextFreeTrig; */
	

	if (LastTrig)
	{
		trig = LastTrig;
		LastTrig=LastTrig->next;
		
	}
	else
	{
		trig = NextFreeTrig;
	 	if (!trig)
	 	{
			CrashOut("OUT OF TRIGGERS", 0xffffffff, 0xffffffff);
	 		dAssert(trig,"OUT OF TRIGGERS\n");
	 	}
		NextFreeTrig=NextFreeTrig->next;
	}

	//POINT EXISTING TRIGGER TO NEW
	//NEW TRIG <- STRATTRIGGER
	if (strat->trigger)
		strat->trigger->prev_trig = trig;
	else
	{
	 //	trig->prev_trig = NULL;
		trig->next_trig = NULL;

	} 
	//else
		//else first trigger so ensure the next is nulled
	//	trig->next_trig = NULL;


  	trig->prev_trig = NULL;
	//POINT NEW TRIGGER TO EXISTING
	//NEW TRIG <-> STRATTRIGGER
	trig->next_trig = strat->trigger;

	//STRAT TRIGGER NOW THE NEW
	strat->trigger = trig;


	trig->address=address;
	trig->trig_type = TrigType;

	trig->trig_timer = 0;
	trig->trig_count = (short)TrigParam;
	trig->pausedby = NULL;
	trig->flag = 0;

	/*ENSURE STACK AND LOOP COUNTERS ARE CLEARED FOR THE TRIGGER */
	
	for (i=0;i<6;i++)
	{
		trig->internalloop[i]=0;
		trig->internalstack[i]=0;
	}
}

/*NON PARAM TRIGGERS AS THE MAJORITY WILL BE */

void CreateTrigger(STRAT *strat,void *address,int TrigType)
{
	TRIGGER *trig;
	int i;
	trigs++;	

	if (LastTrig)
	{
		trig = LastTrig;
		LastTrig=LastTrig->next;
		
	}
	else
	{
		trig = NextFreeTrig;
		dAssert(trig,"OUT OF TRIGGERS\n");
		NextFreeTrig=NextFreeTrig->next;
	}

	if (strat->trigger)
	{
		strat->trigger->prev_trig = trig;
	}
	else
	{
//		trig->prev_trig = NULL;
		trig->next_trig = NULL;

	} 
  
	
	trig->prev_trig = NULL;
	//else ensure the trigger is nulled
	  //	trig->next_trig = NULL;

	trig->next_trig = strat->trigger;
	strat->trigger = trig;


	
	trig->address=address;
	trig->trig_type = TrigType;
	trig->pausedby = NULL;
	trig->flag = 0;

	/*clear trigger stack and loop counters */
	for (i=0;i<6;i++)
	{
		trig->internalloop[i]=0;
		trig->internalstack[i]=0;
	}	
}


static void CheckForUnpaused(STRAT *strat,TRIGGER *trigger)
{
	TRIGGER *Next;
	TRIGGER *trig = strat->trigger;

	while (trig)
	{
		if ((trig!=trigger) && (trig->pausedby == trigger))
			trig->flag &= SLNOT(TRIG_HELD);

		Next = trig->next_trig;
		trig=trig->next_trig;
	}




}





/*SEARCH THROUGH STRAT TRIGGERS, (SHOULD BE MAX 4 ON GO AT ONCE) */
/*WHEN TRIGGER IS FOUND KILL IT */
/*USE WHEN NOT INSIDE THE TRIGGER'S PROCESSING BLOCK IF YOU WANT TO KILL A GIVEN */
/*TRIGGER OFF */

void KillTrigger(STRAT *strat,void *address,int TrigType)
{
 //	int notfirst;
	int starttrig=trigs;
	TRIGGER *trig=strat->trigger;

/*	dAssert(trig,"NO STRAT TRIGGER TO KILL\n"); */
		
	while (trig)
	{
		if ((trig->trig_type==TrigType) && ((int)(trig->address)==(int)address))
		{	
			//we have found the trigger to delete
			//do we need to unpause anything connected with it ?
			CheckForUnpaused(strat,trig);
		 
			trigs--;

			trig->flag=0;

		  //	notfirst=0;
			if (trig->prev_trig)
			{
				trig->prev_trig->next_trig = trig->next_trig;
		   //		notfirst++;
			}
			else
				strat->trigger = trig->next_trig;

			if (trig->next_trig)
			{
				trig->next_trig->prev_trig = trig->prev_trig;
			 	//notfirst++;
			}
			else
			{
				if (trig == strat->trigger)
					strat->trigger = NULL;

			}

		  //	if (!notfirst)
		   //		strat->trigger = NULL;

			//release the trigger to the pool
			trig->next = LastTrig;
			trig->prev_trig = trig->next_trig=NULL;
			/*LastTrig=trig->next; */
			LastTrig=trig;

			break;
		}
		trig=trig->next_trig;
	
	}

  	dAssert(trigs != starttrig,"ATTEMPT TO KILL NON-SET TRIGGER\n"); 

}


/*WHEN A STRAT IS DEAD THEN ALL ITS TRIGGERS MUST BE DESTROYED */
static void KillStratTriggers(STRAT *strat)
{
	int starttrig=trigs;
	TRIGGER *trig=strat->trigger,*oldtrig;

	
	while (trig)
	{
		trigs--;

		trig->flag=0;

		/*if (trig->prev_trig) */
		/*	trig->prev_trig->next_trig = trig->next_trig; */
		/*else */
		/*	strat->trigger = trig->next_trig; */

		/*if (trig->next_trig) */
		/*	trig->next_trig->prev_trig = trig->prev_trig; */

		trig->next = LastTrig;
		/*LastTrig=trig->next; */
		LastTrig=trig;

		oldtrig=trig;
		trig=trig->next_trig;
		oldtrig->prev_trig = oldtrig->next_trig=NULL;
	
	}
	CurrentTrig = strat->trigger = NULL;


}



/*CALLED AT THE END OF A TRIGGER'S INTERRUPT PROCESSING */
/*IF YOU WANT TO KILL OFF THE CURRENT TRIGGER */

void EndTrigger(STRAT *strat)
{
	TRIGGER *trig = CurrentTrig;
   //	int notfirst;

	if (!trig)
		return;
	
	trigs--;
	
	
/*	dAssert(trig,"NO STRAT TRIGGER TO END\n"); */

   //	notfirst=0;
	if (trig->prev_trig)
	{
		trig->prev_trig->next_trig = trig->next_trig;
   //		notfirst++;
	}
	else
		strat->trigger = trig->next_trig;

	if (trig->next_trig)
	{
		trig->next_trig->prev_trig = trig->prev_trig;
	 //	notfirst++;
	}
	else
	{
		if (trig == strat->trigger)
			strat->trigger = NULL;

	}


  //	if (!notfirst)
  //		strat->trigger = NULL;

	//RELEASE THE TRIG TO THE POOL
	trig->next = LastTrig;
	trig->prev_trig = trig->next_trig=NULL;
	/*LastTrig=trig->next; */
	LastTrig=trig;

	trig->flag=0;
	CurrentTrig = NULL;

 }


/*CALLED AT THE END OF TRIGGER PROCESSING IF YOU WANT TO JUST FLAG A TRIGGER AS */
/*BEING COMPLETE AND IT HAS NOT BEEN DELETED */

void CompleteTrigger(STRAT *strat)
{
   	if (CurrentTrig)
		CurrentTrig->flag &= SLNOT(TRIG_RUNNING);
}

/*ENABLES THE CURRENT TRIGGER TO BE SUSPENDED */

void HoldTrigger(STRAT *strat)
{
  	CurrentTrig->flag |= TRIG_HELD;
}

/*SUSPENDS A GIVEN TRIGGER */

void HoldTrig(STRAT *strat,void *address,int TrigType)
{
	int i=0;
	TRIGGER *trig=strat->trigger;

	dAssert(trig,"NO STRAT TRIGGER TO SUSPEND\n");
		
	while (trig)
	{
		if ((trig->trig_type==TrigType) && ((int)(trig->address)==(int)address))
		{	
			i++;
			trig->flag |= TRIG_HELD;
			break;
		}
		trig=trig->next_trig;
	
	}

	dAssert(i,"ATTEMPT TO SUSPEND NON-SET TRIGGER\n");

}


/*RELEASES THE CURRENT TRIGGER */

void ReleaseTrigger(STRAT *strat)
{
	CurrentTrig->flag &= SLNOT(TRIG_HELD);
}

/*RELEASES A GIVEN TRIGGER */

void ReleaseTrig(STRAT *strat,void *address,int TrigType)
{
	int i=0;
	TRIGGER *trig=strat->trigger;

	dAssert(trig,"NO STRAT TRIGGER TO RELEASE\n");
		
	while (trig)
	{
		if ((trig->trig_type==TrigType) && ((int)(trig->address)==(int)address))
		{	
			i++;
			trig->flag &= SLNOT(TRIG_HELD);
			break;
		}
		trig=trig->next_trig;
	
	}

	dAssert(i,"ATTEMPT TO RELEASE NON-SET TRIGGER\n");
}

void KillMyVelocity(STRAT *strat)
{
	if (!GetGlobalParamInt(0))
		strat->vel.z = strat->relVel.z = 0.0f;

	strat->vel.x = strat->vel.y = 0.0f;

	strat->relVel.x = strat->relVel.y = 0.0f;

}



int TOINT(float num)
{
	return ((int)num);
}


void ReduceHealth(STRAT *strat, float amount)
{
	strat->health -= amount;
}

void MoveBackOneFrame(STRAT *strat)
{
	strat->pos= strat->oldPos;
}

void ClearObj(STRAT *strat)
{
	strat->obj = NULL;
	strat->pnode = NULL;
}

void RandomPointTo(STRAT *strat)
{
	Vector3	direction;
	Point3	point;
	
	v3Rand(&direction);
	if (direction.z < 0.0f)
		direction.z *= -1.0f;

	point.x = strat->pos.x + direction.x;
	point.y = strat->pos.y + direction.y;
	point.z = strat->pos.z + direction.z;
	pointToXZ(strat, &point);
}

void PointToTarget(STRAT *strat)
{
	
	pointToXZ(strat, &strat->CheckPos);
}

void Explosion(STRAT *crntStrat, float power, float radius)
{
	STRAT	*strat;
	float	sd, savedDam;
	
	savedDam = crntStrat->damage;
	strat = FirstStrat;

	while (strat)
	{
		if (((!strat->pnode) || (strat->flag & STRAT_NOT_EXPLODE)) ||
			(strat == crntStrat))
		{
			strat = strat->next;
			continue;
		}

		if (crntStrat->flag & STRAT_ENEMY)
		{
			if (strat->flag & STRAT_ENEMY)
			{
				strat = strat->next;
				continue;
			}
		}
		else if (crntStrat->flag & STRAT_FRIENDLY)
		{
			if (!Multiplayer)
			{
				if (strat->flag & STRAT_FRIENDLY)
				{
					strat = strat->next;
					continue;
				}
			}
		}

		if ((strat->flag & STRAT_COLLSTRAT) && crntStrat->pnode)
		{
			sd = pDist(&crntStrat->pos, &strat->pos);
			if (sd > radius + (crntStrat->pnode->radius + strat->pnode->radius))
			{
				strat = strat->next;
				continue;
			}
			// Kludgey hack :)
			crntStrat->damage = power;
			HurtStrat (strat, crntStrat);
		}
		strat = strat->next;
	}
	crntStrat->damage = savedDam;
}


int	SeePlayer(STRAT *strat,float angle)
{	
	Vector3 dst,RotForward;

	RotForward.x = strat->m.m[1][0];
	RotForward.y = strat->m.m[1][1];
	RotForward.z = strat->m.m[1][2];

	v3Sub(&dst, &player[currentPlayer].Player->pos, &strat->pos);
	v3Normalise(&dst);

	if (acos(v3Dot(&dst, &RotForward)) < angle)
		return(1);
	else
		return(0);
}

int SeeWay(STRAT *strat,float angle)
{	
	Vector3 Forward, stratToWay;
	
	Forward.x=strat->m.m[1][0];
	Forward.y=strat->m.m[1][1];
	Forward.z=strat->m.m[1][2];
	v3Normalise(&Forward);

	stratToWay.x = strat->CheckPos.x - strat->pos.x;
	stratToWay.y = strat->CheckPos.y - strat->pos.y;
	stratToWay.z = strat->CheckPos.z - strat->pos.z;
	v3Normalise(&stratToWay);

	if (acos(v3Dot(&Forward, &stratToWay)) < angle)
		return 1;
	else
		return 0;
}

static int SeePointZ(STRAT *strat,Vector3 *pos,float angle)
{	
	Matrix im;
	Point3	p;
 	Vector3 forward;
	float actualAng;
	
	if ((pos->x == strat->pos.x) && (pos->y == strat->pos.y))
		return 0; 

	forward.x = 0.0f;
    forward.y = 1.0f;
    forward.z = 0.0f;
    p = *pos;
    v3Dec(&p, &strat->pos);
    mInvertRot(&strat->m, &im);
    mPoint3Apply3(&p, &im);
    p.z = 0.0f;
	
    v3Normalise(&p);

    actualAng = (float) acos(v3Dot(&p, &forward));	

	if (angle > actualAng)
		return 1;
	else
		return 0;
}


int SeeWayZ(STRAT *strat,float angle)
{	
	return (SeePointZ(strat,&strat->CheckPos,angle));

/*	Vector3 Forward, stratToWay;
	
	Forward.x=strat->m.m[1][0];
	Forward.y=strat->m.m[1][1];
	Forward.z=0.0f;
	v3Normalise(&Forward);

	stratToWay.x = strat->CheckPos.x - strat->pos.x;
	stratToWay.y = strat->CheckPos.y - strat->pos.y;
	stratToWay.z = 0.0f;
	v3Normalise(&stratToWay);

	angle = (float) cos(angle);
  */
/*	if (acos(v3Dot(&Forward, &stratToWay)) < angle) */
/*	if ((v3Dot(&Forward, &stratToWay)) > angle)
		return 1;
	else
		return 0;
*/
}
int SeeTargetZ(STRAT *strat,float angle)
{	

	return (SeePointZ(strat,&strat->target->pos,angle));
}

int ObjectSeePlayerZ(STRAT *strat,float angle, int objId, float offsetx, float offsety, float offsetz)
{	
	Matrix rotMat;
	Vector3 Forward, stratToWay;

	rotMat = strat->m;
	mPreScale(&rotMat, strat->scale.x, strat->scale.y, strat->scale.z);
	
	/*mUnit(&rotMat);
 
	mPreTranslate(&rotMat, strat->pos.x, strat->pos.y, strat->pos.z);
   	mPreMultiply(&rotMat, &strat->m);
   	mPreScale(&rotMat, strat->scale.x, strat->scale.y, strat->scale.z);
   */
	//mPostTranslate(&rotMat,strat->pos.x,strat->pos.y,strat->pos.z);
	if (!(strat->pnode->anim))
		findTotalMatrix(&rotMat, strat->objId[objId]);
  	else
		findTotalMatrixInterp(&rotMat, strat->objId[objId]);

	mPreTranslate(&rotMat,offsetx,offsety,offsetz);
	rotMat.m[3][0] += strat->pos.x;
	rotMat.m[3][1] += strat->pos.y;
	rotMat.m[3][2] += strat->pos.z;




	Forward.x=rotMat.m[1][0];
	Forward.y=rotMat.m[1][1];
	Forward.z=rotMat.m[1][2];
	v3Normalise(&Forward);

	stratToWay.x = player[currentPlayer].Player->pos.x - (rotMat.m[3][0]);
	stratToWay.y = player[currentPlayer].Player->pos.y - (rotMat.m[3][1]);
	stratToWay.z = player[currentPlayer].Player->pos.z - (rotMat.m[3][2]);
	
	v3Normalise(&stratToWay);





 	angle = (float) cos(angle);
										
	if ((v3Dot(&Forward, &stratToWay)) > angle)	 
		return 1;
		
   	else
		return 0;



}




int SeePlayerZ(STRAT *strat,float angle)
{	

	return (SeePointZ(strat,&player[currentPlayer].Player->pos,angle));
 /*	Vector3 Forward, stratToWay;
	
	Forward.x=strat->m.m[1][0];
	Forward.y=strat->m.m[1][1];
	Forward.z=0.0f;
	v3Normalise(&Forward);

	stratToWay.x = Player->pos.x - strat->pos.x;
	stratToWay.y = Player->pos.y - strat->pos.y;
	stratToWay.z = 0.0f;
	v3Normalise(&stratToWay);

	angle = (float) cos(angle);

/*	if (acos(v3Dot(&Forward, &stratToWay)) < angle) */
   /*	if ((v 3Dot(&Forward, &stratToWay)) > angle)
		return 1;
	else
		return 0;
	*/
}



int VisibleFromPlayer(STRAT *strat,float angle)
{	
 	Vector3 Forward, stratToWay;



	
	Forward.x=player[currentPlayer].Player->m.m[1][0];
	Forward.y=player[currentPlayer].Player->m.m[1][1];
	Forward.z=0.0f;
	v3Normalise(&Forward);

	stratToWay.x = strat->pos.x - player[currentPlayer].Player->pos.x;
	stratToWay.y = strat->pos.y - player[currentPlayer].Player->pos.y;
	stratToWay.z = 0.0f;
	v3Normalise(&stratToWay);

	angle = (float) cos(angle);

	if ((v3Dot(&Forward, &stratToWay)) > angle)	 
		return 1;
		
   	else
		return 0;
	
 }

int AwaySeeWayZ(STRAT *strat,float angle)
{	
	Vector3 Forward, stratToWay;
	
	Forward.x=-strat->m.m[1][0];
	Forward.y=-strat->m.m[1][1];
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


float WayDist(STRAT *strat)
{
	double x,y,z;
	float real;

	x = (strat->CheckPos.x-strat->pos.x );
	y = (strat->CheckPos.y-strat->pos.y );
	z = (strat->CheckPos.z-strat->pos.z );


	real =(float)sqrt((x*x)+(y*y)+(z*z));

	return (real);
}

void PointPlayer(STRAT *strat)
{
	pointToXZ(strat,&player[currentPlayer].Player->pos);

}

#define USEACOS 1

static void DeriveForwardAim(Vector3 *forward)
{
	forward->x = player[currentPlayer].Player->objId[5]->wm.m[1][0];
  	forward->y = player[currentPlayer].Player->objId[5]->wm.m[1][1];
	forward->z = 0;
	v3Normalise(forward);
}


static int WithinAim(STRAT *strat,float angle,Vector3 *Forward)
{
#if USEACOS
	Vector3 forw,stratToWay;

	forw.x = player[currentPlayer].CameraStrat->pos.x - strat->pos.x;
	forw.y = player[currentPlayer].CameraStrat->pos.y - strat->pos.y;
	forw.z = player[currentPlayer].CameraStrat->pos.z - strat->pos.z;
	v3Normalise(&forw);

	stratToWay.x = -(player[currentPlayer].Player->objId[5]->wm.m[3][0] - strat->pos.x);
	stratToWay.y = -(player[currentPlayer].Player->objId[5]->wm.m[3][1] - strat->pos.y);
	stratToWay.z = 0.0f;
	stratToWay.x = (player[currentPlayer].CameraStrat->pos.x - player[currentPlayer].aimedPoint.x);
	stratToWay.y = (player[currentPlayer].CameraStrat->pos.y - player[currentPlayer].aimedPoint.y);
	stratToWay.z = (player[currentPlayer].CameraStrat->pos.z - player[currentPlayer].aimedPoint.z);
	v3Normalise(&stratToWay);

/*	if (acos(v3Dot(&Forward, &stratToWay)) < angle) */
   	if ((v3Dot(&forw, &stratToWay)) > angle)
   	{
		dLog("IN CONE %s\n",strat->pnode->name);
		strat->flag2 |= STRAT2_TARGETTED;
		return 1;
   	}
   	else
   	{
	 	dLog("OUT CONE %s\n",strat->pnode->name);
	  	return 0;
   	}
#else

	Vector3 Forward,dst,RotForward;
	float Yaw,length;
	
	/*GIVEN AN ANGLE OF CONE DERIVE WHETHER A SPECIFIC POINT */
	/*CAN SEE THE PLAYER WITHIN IT */
	
	Forward.x=0;Forward.z=0.0f;

	Forward.y=1.0f;

	mVector3Multiply (&RotForward, &Forward, &player[currentPlayer].Player->m);


	dst.x = (strat->pos.x - player[currentPlayer].Player->pos.x);
	dst.y = (strat->pos.y - player[currentPlayer].Player->pos.y);
	dst.z = (strat->pos.z - player[currentPlayer].Player->pos.z);



	length=(float)sqrt((dst.x*dst.x) + (dst.y * dst.y) +(dst.z*dst.z));

	if (!length)
		return(0);

	Yaw = ((dst.x * RotForward.x)+(dst.y * RotForward.y)  + (dst.z * RotForward.z)) / length;

	if (Yaw > angle)
		return(1);
	else
		return(0);

#endif		
}


static TARGETS	  PrimaryTargets[MAX_PRIMTARGETS];	/*POOL OF PRIMARY GUN TARGETS */
static TARGETS *NearestTarget=NULL;
static TARGETS *NextTarget=NULL;




static void RemovePrimaryTarget (TARGETS *Target, TARGETS *Last)
{
	Last->prev = Target->prev;
	Target->strat = NULL;
}

/*ON DELETION OF TARGETTED STRAT, WILL REMOVE ITS LOG FROM THE TARGET LIST */
/*ENSURING THE NEAREST TARGET IS CORRECTLY SETUP */

static void	RemovePrimary(STRAT *strat)
{
	TARGETS *Target,*LastTarget;

	if (!NearestTarget)
		return;

	Target = NearestTarget;
	LastTarget = Target;
	for (;;)
	{

		NextTarget = Target->prev;
		if (Target->strat == strat)
		{	
			RemovePrimaryTarget(Target,LastTarget);
			if (Target == NearestTarget)
				NearestTarget = NextTarget;
		}
		else
			LastTarget = Target;

		Target = NextTarget;
		if (!Target)
			break;
	}	

}

static float TargetsNearPlayer(STRAT *strat,float dist,float mindist,float lastdist)
{
	double x,y,z;
	float real;

	
	x = (player[currentPlayer].Player->pos.x-strat->pos.x );
	y = (player[currentPlayer].Player->pos.y-strat->pos.y );
	z = (player[currentPlayer].Player->pos.z-strat->pos.z );



	real =(float)sqrt((x*x)+(y*y)+(z*z));

	/*dLog("considered near STRAT %x PLACED %f name %s flag %d\n",strat,lastdist,strat->pnode->name,strat->flag); */
	if ((real <= dist) && (real > mindist))
	{
  		if (real < lastdist)
		{

		   /*	dLog("near STRAT %x PLACED %f name %s flag %d\n",strat,lastdist,strat->pnode->name,strat->flag); */
			
		   	if (NearestTarget!=NextTarget)
				NextTarget->prev = NearestTarget;

			/*setup the new */
			NearestTarget = NextTarget;
			NearestTarget->strat = strat;
			/*increment the counter */
			NextTarget = NextTarget->next;
			lastdist = real;
			
	  	}
		else
		{
			dLog("not near STRAT  %x PLACED %f name %s flag %d\n",strat,lastdist,strat->pnode->name,strat->flag);
 			NextTarget->prev = NearestTarget->prev;
 	   	    NearestTarget->prev = NextTarget;
			NextTarget->strat = strat;
			 
			NextTarget = NextTarget->next;
			

		} 
		

	}
	return (lastdist);



}



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
  

	NearestTarget = NextTarget = &PrimaryTargets[0];
	for (i=0;i<MAX_PRIMTARGETS;i++)
	{
		PrimaryTargets[i].prev = NULL;
		if (i==MAX_PRIMTARGETS-1)
			PrimaryTargets[i].next = NULL;
		else
			PrimaryTargets[i].next = &PrimaryTargets[i+1];
		PrimaryTargets[i].strat = NULL;
	}

	NearestTarget->prev = NULL;

	LastTarget = NULL;
	while (strat)
	{
		if ((strat != player[currentPlayer].Player) && (strat != player[currentPlayer].CameraStrat)
			&& ((strat->flag & STRAT_COLLSTRAT))
			&& ((strat->flag & STRAT_ENEMY))
			&& (!(strat->flag & STRAT_BULLET)) 
			&& (!(strat->flag & STRAT_HIDDEN)) 
			&& (!(strat->flag & STRAT_EXPLODEPIECE)) 
			&& ((strat->flag > STRAT_ALIVE)) 
		   	&& NextTarget)
				lastdist = TargetsNearPlayer(strat,radius,minradius,lastdist);
		strat = strat->next;

	}

	
	/*go through the list of targets removing those which are not within our viewing cone*/
	/* which at the moment is set at 45.0f */
	if (NearestTarget->strat)
	{
		radius = PI2/16.0f;

		DeriveForwardAim(&forward);
		cosangle = (float)cos(radius);
		Target = NearestTarget;
		LastTarget = Target;
		while (Target)
		{
			NextTarget2 = Target->prev;
	  
			if (!WithinAim(Target->strat,cosangle,&forward))
			{	
				RemovePrimaryTarget(Target,LastTarget);
				if (Target == NearestTarget)
					NearestTarget = NextTarget2;
			}
			else
				LastTarget = Target;
			Target = NextTarget2;
	
		}	
	}


}






/*TRUE RETURN IF STRAT HAS NOT BEEN REGISTERED AS A MISSILE LOCK ON */
/*OR IF IT HAS LESS THAN 2 MISSILES ARE LOCKED ONTO IT */

static LOCKEDTARGETS Locks[MAX_LOCKEDTARGETS];

static int MaxLock(STRAT *strat)
{
	int i;

	for (i=0;i<MAX_LOCKEDTARGETS;i++)
	{
		if (Locks[i].strat == strat)
		{
		  	dLog("strat %s locks %d\n",Locks[i].strat->pnode->name,Locks[i].lockedon);
			if (Locks[i].lockedon < 2)
				return(1);
			else
				return (0);
		}
	}

	return(1);

}


static void IncLockOns(STRAT *strat)
{
	int i;

	for (i=0;i<MAX_LOCKEDTARGETS;i++)
	{
		if (!(Locks[i].strat))
			break;
		
		if (Locks[i].strat == strat)
		{
			Locks[i].lockedon++;
		 	return;
		}
	}

	Locks[i].strat = strat;

	Locks[i].lockedon = 1;

}


static void DecLockOns(STRAT *strat)
{
	int i;

	for (i=0;i<MAX_LOCKEDTARGETS;i++)
	{
		if (Locks[i].strat == strat)
		{
			Locks[i].lockedon--;
			if (!Locks[i].lockedon)
				Locks[i].strat = NULL;
			return;
		}
	}



}

int GetLockOn(STRAT *strat)
{
	Vector3 RotForward;
//	Vector3	Forward;
	STRAT *LastLock = NULL;
	TARGETS  *NextNear = NearestTarget;
	int found = 0;



	dLog("NEXT NEAR %x\n",NextNear);

	if (NextNear)
	{

		for (;;)


		{

			dLog("CONSIDERING STRAT %x END %x\n",NextNear,&PrimaryTargets[0]);

			if ((NextNear->strat) && 
				(!(NextNear->strat->flag & STRAT_HIDDEN)) &&
				(NextNear->strat->flag > STRAT_DEAD))
			{

				/*GOT A POTENTIAL TARGET IS IT ALREADY LOCKED ? */

				if ((MaxLock(NextNear->strat)))
				{
					dLog("FOUND\n");
					/*NO ? THEN BREAK */
					found++	;
					break;
				}
				else
					/*ELSE CONTINUE SEARCHING BUT MAKE SURE WE KEEP THIS JUST IN CASE THERE ARE NO OTHER */
					/*POSSIBLES */
				   LastLock = NextNear->strat;

			}

			NextNear = NextNear->prev;
			if (!NextNear)
			{
				NextNear = NearestTarget;
				break;
			}
		}

		if ((found) || (LastLock) )
		{

			dLog("NEAREST %x NEWTARGET %x\n",NearestTarget,NextNear);

			/*don't pick the same target as you had before */
		   /*	if (NextNear->strat == strat->target) */
		   /*		return(1); */

			strat->target = NextNear->strat;

			RotForward.x = 3.0f*NextNear->strat->m.m[1][0];
			RotForward.y = 3.0f*NextNear->strat->m.m[1][1];
			RotForward.z = 3.0f*NextNear->strat->m.m[1][2];


			strat->CheckPos.x = NextNear->strat->pos.x+RotForward.x;
			strat->CheckPos.y = NextNear->strat->pos.y+RotForward.y;
			strat->CheckPos.z = NextNear->strat->pos.z+RotForward.z;

			IncLockOns(NextNear->strat);
			return (1);
		 }
		else
		{
			strat->target = NULL;
			return(0);
		}


	}
	else
	{
		strat->target = NULL;
		return(0);
	}




	if ((NearestTarget) && (NearestTarget->strat) && 
	 (!(NearestTarget->strat->flag & STRAT_HIDDEN)) &&
		(NearestTarget->strat->flag > STRAT_DEAD))
	{
			strat->target = NearestTarget->strat;

			RotForward.x = 3.0f*NearestTarget->strat->m.m[1][0];
			RotForward.y = 3.0f*NearestTarget->strat->m.m[1][1];
			RotForward.z = 3.0f*NearestTarget->strat->m.m[1][2];


			strat->CheckPos.x = NearestTarget->strat->pos.x+RotForward.x;
			strat->CheckPos.y = NearestTarget->strat->pos.y+RotForward.y;
			strat->CheckPos.z = NearestTarget->strat->pos.z+RotForward.z;

	   /*		NearestTarget->targetnum++; */


			return (1);
	}
	else
	{	
		strat->target = NULL;
		return(0);

	}
}


int FollowLockOn(STRAT *strat)
{
	Vector3 RotForward;
	Vector3	Forward;
	/*FIRST ENSURE THE TARGET IS STILL VALID */
	int found=0;

 	if ((!NearestTarget) || (!strat->target))
		return(0);

	if ((!(strat->target->flag & STRAT_HIDDEN)) && (strat->target->flag > STRAT_DEAD) && (strat->target->health > 0.0))
	{
		Forward.z = 0.0f;Forward.x=0;
		Forward.y = 1.5f * strat->relVel.y;
		if (Forward.y<0)
		{
			if (Forward.y>-1.0f)
				Forward.y-=1.0f;
		}
		else
		{
			if (Forward.y<1.0f)
				Forward.y+=1.0f;
		}
		mVector3Multiply (&RotForward, &Forward, &strat->target->m);
		RotForward.x = strat->relVel.y*0.5f*strat->target->m.m[1][0];
		RotForward.y = strat->relVel.y*0.5f*strat->target->m.m[1][1];
		RotForward.z = strat->relVel.y*0.5f*strat->target->m.m[1][2];
		/*RotForward.x = strat->relVel.x*strat->target->m.m[1][0]; */
		/*RotForward.y = strat->relVel.y*strat->target->m.m[1][1]; */
		/*RotForward.z = strat->relVel.z*strat->target->m.m[1][2]; */
		strat->CheckPos.x = strat->target->pos.x+RotForward.x;
		strat->CheckPos.y = strat->target->pos.y+RotForward.y;
		strat->CheckPos.z = strat->target->pos.z+RotForward.z;

		return (1);
	}
	else
	  	return (0);
}


/* SETS THE CURRENT POSITION FOR CHECKING/MOVING TO */

void SetCheckPos(STRAT *strat,float x,float y,float z)
{
	strat->CheckPos.x = x;  
	strat->CheckPos.y = y;  
	strat->CheckPos.z = z;  

}


/*relative to the player */

void SetCheckPosRel(STRAT *strat,int id,float x,float y,float z)
{
	Vector3 vec,vec2;
	Matrix temp;
	Object *object;

#if 0
	vec.x = x;
	vec.y = y;
	vec.z = z;

/*	mVector3Multiply (&vec2, &vec, &Player->m); */

	mVector3Multiply (&vec2, &vec, &strat->m);


	strat->CheckPos.z = vec2.z+strat->pos.z; 
	strat->CheckPos.y = vec2.y+strat->pos.y; 
	strat->CheckPos.x = vec2.x+strat->pos.x ; 
#endif	   
		
	   //	id = (int)obj;
	 

		temp= strat->m;
		
		
		
	   	mPreScale(&temp,strat->scale.x,strat->scale.y,strat->scale.z);



	   	if (id)
		{
			object = strat->objId[id];
			findTotalMatrixInterp(&temp, object);
		}

		mPreTranslate(&temp,x,y,z);


	  //	mPreRotateXYZ(&strat->m, xang, yang, zang);

	   	strat->CheckPos = strat->pos;
	  	strat->CheckPos.x += temp.m[3][0];
	  	strat->CheckPos.y += temp.m[3][1];
	  	strat->CheckPos.z += temp.m[3][2];
		
		

}





void SetTarget(STRAT *strat,float x, float y, float z)
{
	Vector3 vec,vec2;
	float dx,dy,dz;

	vec.x = x;
	vec.y = y;
	vec.z = z;
	mVector3Multiply (&vec2, &vec, &player[currentPlayer].Player->m);
	strat->CheckPos.z = vec2.z+player[currentPlayer].Player->pos.z; 
	strat->CheckPos.y = vec2.y+player[currentPlayer].Player->pos.y; 
	strat->CheckPos.x = vec2.x+player[currentPlayer].Player->pos.x ; 

	dx = (strat->pos.x - strat->CheckPos.x);
	dy = (strat->pos.y - strat->CheckPos.y);
	dz = (strat->pos.z - strat->CheckPos.z);

/*	strat->waytoward=0; */
/*	strat->lastdist = WayDist(strat); */

}
int NearCheckPosXY(STRAT *strat,float dist)
{
	float real;
	float x, y;
	float RAD2,DIST2;

	if (abs(strat->relVel.y > 1.0f))
		dist *= (float)fabs(strat->relVel.y);

	x = (float)(strat->pos.x - strat->CheckPos.x);
	y = (float)(strat->pos.y - strat->CheckPos.y);


	RAD2 = strat->pnode->radius	* strat->scale.x;
	RAD2 *= RAD2;

   
	
	if (dist)
		DIST2 = dist * dist;
	else
		DIST2 = 0.0;

 //	real =(float)sqrt((x*x)+(y*y));	 

	real =(x*x)+(y*y);

	if (real < RAD2 + DIST2)
		return 1;
 	else
		return 0;
}

int NearPosXY(STRAT *strat,float dist, float xpos, float ypos)
{
	float real;
	float x, y;
	if (abs(strat->relVel.y > 1.0f))
		dist *= (float)fabs(strat->relVel.y);
	
	x = (float)(strat->pos.x - xpos);
	y = (float)(strat->pos.y - ypos);
	
	real =(float)sqrt((x*x)+(y*y));
	
	// 	strat->lastdist = real;
	if (real < (strat->pnode->radius*strat->scale.x+dist)){
		/*	if (real < (dist)){ */
		return 1;}			   
	else
		return 0;
}

int NearCheckPosZ(STRAT *strat,float dist)
{
	float z;
	
	z = (float)(strat->pos.z - strat->CheckPos.z);
	
	if (z < (strat->pnode->radius*strat->scale.x+dist)){
		return 1;}
	else
		return 0;
}

static int NearXYZ(STRAT *strat,float dist)
{
	float real;
	double x,y,z;
	if (abs(strat->relVel.y > 1.0f))
		dist *= (float)fabs(strat->relVel.y);		
	
	x = (strat->CheckPos.x-strat->pos.x );
	y = (strat->CheckPos.y-strat->pos.y );
	z = (strat->CheckPos.z-strat->pos.z );
	
	
	real =(float)sqrt((x*x)+(y*y)+(z*z));
	
	
	if (real <=(strat->pnode->radius*strat->scale.x+dist))
		return(1);
	else
		return(0);
}

void *processSpecNear[4] = 
{
	&NearXYZ,
		&NearXYZ,
		&NearXYZ,	/*HOVER */
		&NearCheckPosXY		/*GROUND */
};


int NearCheckPos(STRAT *strat,float dist)
{
	
	/*	float real; */
	int movetype;
	int	(*specProc)(STRAT *strat,float dist);
	
	movetype=(strat->flag>>17)&3;
	
	/*	dLog("movetype %d\n",movetype); */
	specProc = processSpecNear[movetype];
	return (specProc(strat,dist));
	
	
}

int PlayerNearCheckPos(STRAT *strat,float dist)
{
	double x,y,z;
	float real;
	
	dist = dist * dist;
	x = (strat->CheckPos.x-player[currentPlayer].Player->pos.x );
	y = (strat->CheckPos.y-player[currentPlayer].Player->pos.y );
	z = (strat->CheckPos.z-player[currentPlayer].Player->pos.z );
	
	/*	real =(float)sqrt((x*x)+(y*y)+(z*z)); */
	
	real =(float)((x*x)+(y*y)+(z*z));
	
	
	if (real <=dist)
		return(1);
	else
		return(0);
	
	
}
int NearPlayer(STRAT *strat,float dist)
{
	float x,y,z;
	float real;
	
	dist = dist * dist;
	
	/*	dist *= strat->vel.y;		 */
	
	x = (player[currentPlayer].Player->pos.x-strat->pos.x );
	y = (player[currentPlayer].Player->pos.y-strat->pos.y );
	z = (player[currentPlayer].Player->pos.z-strat->pos.z );
	
	/*	real =(float)sqrt((x*x)+(y*y)+(z*z)); */
	
	real =((x*x)+(y*y)+(z*z));
	
	if (real <=dist)
		return(1);
	else					
		return (0);
}

int NearPlayerXY(STRAT *strat,float dist)
{
	double x,y;
	float real;
	
	
	/*	dist *= strat->vel.y;		 */
	
	x = (player[currentPlayer].Player->pos.x-strat->pos.x );
	y = (player[currentPlayer].Player->pos.y-strat->pos.y );
	dist = dist * dist;
	
	
	/*	real =(float)sqrt((x*x)+(y*y)); */
	
	real =(float)((x*x)+(y*y));
	
   	if (real <=dist)
		return(1);
	else
		return (0);
}

int NearTargetXY(STRAT *strat,float dist)
{
	double x,y;
	float real;
	
	
	x = (strat->target->pos.x-strat->pos.x );
	y = (strat->target->pos.y-strat->pos.y );
	dist = dist * dist;
	
	
	real =(float)((x*x)+(y*y));
	
	if (real <=dist)
		return(1);
	else
		return (0);
}


float PlayerDist(STRAT *strat)
{
	float real;
	double x,y,z;
	
	
	x = (player[currentPlayer].Player->pos.x-strat->pos.x );
	y = (player[currentPlayer].Player->pos.y-strat->pos.y );
	z = (player[currentPlayer].Player->pos.z-strat->pos.z );
	
	
	real =(float)sqrt((x*x)+(y*y)+(z*z));
	
	return(real);
	
}

static int AheadOfPlayer(STRAT *strat,float dist)
{
	float y;
	
	y = (strat->pos.y-player[currentPlayer].Player->pos.y);
	
	if ((y<0) || (y>dist))
		return (0);
	else
		return(1);
}



void ChangeState  (STRAT *strat, void *Addr)
{
	/*ENSURE WE ARE POINTING AT THE STRAT'S OWN STACK */
	/*IN CASE OF INVOCATION FROM TRIGGER */
	
	strat->stack = strat->stackblock;
	strat->timer = strat->timeblock;
	
	Reset(strat);
	strat->func=Addr;	
	strat->func(strat);
}


void SetAnim(STRAT *strat, int anim)
{
	
	if (!strat->anim)
   	{
		if (LastAnim)
		{
			strat->anim=LastAnim;
			/*strat->anim->anim.anim =NULL;	*/
			LastAnim=LastAnim->next;
		}
		else
		{
			//			dLog("***GETTING ANIM %x\n",NextFreeAnim);
			strat->anim = NextFreeAnim;
			/*strat->anim->anim.anim =NULL;	  */
			
			dAssert(strat->anim,"OUT OF ANIMS\n");
			NextFreeAnim=NextFreeAnim->next;
		}
	}
	
	if (strat->anim)
	{
		PlayAnimation(strat->pnode,&strat->anim->anim,(int)GameAnims[anim].animent);
		strat->anim->anim.flags |= TEXTURE_ANIM;
	}
}
void SetAnimSpeedPercent(STRAT *strat,float percent)
{
	dAssert (strat && strat->anim, "No current animation");
	strat->anim->anim.multiplier = 100.f / percent;
}

void SetModelAnim(STRAT *strat, int animoffset,int flag)
{
	KEYJUMP *key;
	//	return;
	
	if (!strat->pnode->anim)
		return;
	
	
	if (!strat->anim)
	{
		if (LastAnim)
		{
			strat->anim=LastAnim;
			/*strat->anim->anim.anim = NULL;*/
			LastAnim=LastAnim->next;
		}
		else
		{
			//			dLog("***GETTING ANIM %x\n",NextFreeAnim);
			strat->anim = NextFreeAnim;
			/*strat->anim->anim.anim = NULL;	  */
			if (!strat->anim)
				CrashOut("ANIMS",(0x1f) << 16,0xffffffff);
			
			
			dAssert(strat->anim,"OUT OF ANIMS\n");
			NextFreeAnim=NextFreeAnim->next;
		}
	}
	
	/*GRAB A NEW MODEL ANIM BLOCK*/
	if (strat->anim)
	{
		/*HAS IT ALREADY BEEN ASSIGNED TO HAVE A MODEL ANIM BLOCK?*/
		if (!strat->anim->anim.anim)
		{
			if (LastModelAnim)
			{
				strat->anim->anim.anim=&LastModelAnim->anim;  
				strat->anim->anim.anim->typeData = (int*)0xffffffff;  
				LastModelAnim=LastModelAnim->next;
			}
			else
			{
				
				strat->anim->anim.anim = &NextFreeModelAnim->anim;
				strat->anim->anim.anim->typeData =(int*)0xffffffff;		
				
				NextFreeModelAnim=NextFreeModelAnim->next;
				
			}
		}
		
		if (strat->anim->anim.anim)
		{
			key = ((MODELANIM*)strat->pnode->anim->anim->typeData)->KeyFrames;
			
			
			/*THIS IS TO ENSURE WE DON'T RESTART ANIMS THAT ARE CURRENTLY RUNNING*/
			if (strat->pnode->anim->anim->type&INTERPOLATE_ANIM)
			{
				if ((KEYJUMP*)&key[animoffset] == (KEYJUMP*)strat->anim->anim.anim->typeData)
					
				{
					if ( strat->anim->anim.flags & ANIM_FLAG_RUNNING)
						return;
					else
						flag = flag | ANIM_FLAG_NOTWEEN;
				}
			} 
			else
				if ((
					((int*)key[animoffset].offset) == ((int*)strat->anim->anim.anim->typeData)
					)
					&&
					(
					strat->anim->anim.anim->lastFrame == key[animoffset].numframes
					)
					)
					return;
				
				//	flag|=ANIM_FLAG_NOTWEEN;
				/*THIS IS TO ENABLE A TWEEN TO OCCUR IF WE ARE GOING FROM ONE ANIM TO ANOTHER*/
				
				if ((strat->pnode->anim->anim->type&INTERPOLATE_ANIM)  && (!(flag & ANIM_FLAG_NOTWEEN)) && (((int*)strat->anim->anim.anim->typeData) != ((int*)0xffffffff)))
				{
					SetTweenKey(strat->pnode,strat->obj,&strat->anim->anim);
					flag += ANIM_TWEENING;
				}
				
				
				
				strat->anim->anim.anim->frameDelay = 1;
				
				strat->anim->anim.anim->type = ANIM_MODELANIM;
				
				strat->anim->anim.anim->firstFrame = 0;	
				strat->anim->anim.curDelay = 1;
				strat->anim->anim.frame = 0;
				
				
				/*SET THE RESET OFFSET FOR ADDING TO THE BASE MATRIX*/
				/*HACK TO MAKE THE TYPEDATA THE OFFSET VAL*/
				/*strat->anim->anim.anim->firstFrame = key[animoffset].offset;  */
				
				if (strat->pnode->anim->anim->type&INTERPOLATE_ANIM)
					strat->anim->anim.anim->typeData = (KEYJUMP*)&key[animoffset]; 
				else	
					strat->anim->anim.anim->typeData = (int*)key[animoffset].offset; 
				
				/*NUMBER OF FRAMES TO RUN BEFORE RESET*/
				strat->anim->anim.anim->lastFrame = key[animoffset].numframes + 1;
				
				
				/*STARTING FRAME NUMBER*/
				strat->anim->anim.frame = 0;
				
				strat->anim->anim.curDelay = 0;
				
				
				/* AND A FLAG IN THERE */
				
				strat->anim->anim.flags = (MODEL_ANIM + ANIM_FLAG_RUNNING + ANIM_FLAG_ONCE_DONE + flag);
				
				// A default speed multiplier of 1
				strat->anim->anim.multiplier = 1.f;
		}
		else
			/* COULD NOT ASSIGN AN ANIM MODEL BLOCK, SO INVALIDATE THE ANIM PROCESS*/
			strat->anim = NULL;
	}	
}
//JUST PLAYS THE FIRST ANIM IN A MODEL, FOR CUTSCENE USE
void ThisModelAnimPlay(STRAT *strat, int flag)
{
	SetModelAnim(strat, 0, flag);
	
}

static ROUTE *GetNextRoute(STRAT *strat)
{
	if (LastRoute)
	{
		strat->route=LastRoute;
		LastRoute=LastRoute->next;
	}
	else
	{
		dLog("***GETTING ROUTE %x\n",NextFreeRoute);
		strat->route = NextFreeRoute;
		dAssert(strat->route,"OUT OF ROUTE SPACE\n");
		NextFreeRoute=NextFreeRoute->next;
	}
	return(strat->route);
}

#ifdef MEMORY_CHECK
static int STARTNUMFREEBLOCKS;
static int MEMALLOCATLASTLIFE;
int ALLOCSPLINES=0;
int ALLOCSPLINESCHKSUM=0;
int	HPBALLOC = 0;
int HPBALLOCCHKSUM=0;
int	HDCALLOC = 0;
int HDCALLOCCHKSUM=0;
static int VARALLOC = 0;
static int VARALLOCCHKSUM = 0;
static int TOTALALLOC=0;
static int TOTALALLOCCHKSUM = 0;
#endif

void newObject(Object *obj, Object *newObj, int clearcrnt)
{
	int	i;
	
	
	newObj->model = obj->model;
	newObj->no_child = obj->no_child;
	newObj->idleRot = obj->idleRot;
	newObj->parent = NULL;
	if (clearcrnt == 2)
	{
		newObj->scale.x = newObj->scale.y = newObj->scale.z = 1.0f;
		newObj->animscale.x = newObj->animscale.y = newObj->animscale.z = 1.0f;
	}
	else
		newObj->scale = obj->scale;
	newObj->idlePos = obj->idlePos;
	newObj->ncp = obj->ncp;
	newObj->cp = obj->cp;
	//	newObj->cpType = obj->cpType;
   	newObj->specularCol = obj->specularCol;
	newObj->m = obj->m;
	newObj->transparency = 1.f;
	mShortUnit(&newObj->im);
	mUnit(&newObj->wm);
	if (newObj->ncp)
	{
		/*dLog("allocating %d ocpt \n",sizeof(Point3)*newObj->ncp); */
		newObj->ocpt = (Point3 *)CHeapAlloc (ObjHeap,sizeof(Point3)*newObj->ncp);
#ifdef MEMORY_CHECK
		TOTALALLOC += sizeof(Point3)*newObj->ncp;
		TOTALALLOCCHKSUM+=(int)newObj->ocpt;
#endif
		dAssert(newObj->ocpt !=NULL,"fucked ocpt alloc");
	}
	else
		newObj->ocpt=NULL;
	newObj->collFlag = obj->collFlag;
	newObj->crntPos = newObj->idlePos;
	
	//PNODES WITH ANIMATIONS USE CRNTROT IN RELATIVE SENSE FOR ADDING ONTO ANIMS
	//THEREFORE, THEY MUST BE CLEARED AT START
	//NON-ANIMS USE AS ABSOLUTE ROTATIONS, THEREFORE SET TO IDLE AT START
	//if (clearcrnt)
	//	newObj->crntRot.x = newObj->crntRot.y = newObj->crntRot.z = 0;
	//else
	
	if (clearcrnt)
	{ 
		newObj->idleRot.x = newObj->idleRot.y = newObj->idleRot.z = 0;
		obj->initRot.x = obj->initRot.y = obj->initRot.z = 0;
	}
	//else
	
	newObj->initRot = obj->initRot;
	
	
	
	newObj->crntRot = newObj->idleRot;
	
	
	newObj->initPos = obj->initPos;
	
	
	
	if (newObj->no_child)
	{		
		/*dLog("allocating %d child \n",sizeof(Object)*newObj->no_child); */
		newObj->child = (Object *)CHeapAlloc(ObjHeap,sizeof(Object) * newObj->no_child);	
#ifdef MEMORY_CHECK
		TOTALALLOC+=(sizeof(Object)*newObj->no_child);
		TOTALALLOCCHKSUM+=(int)newObj->child;
#endif
		dAssert(newObj->child !=NULL,"fucked child alloc");
		
		for (i=0; i<newObj->no_child; i++)
			newObject(&obj->child[i], &newObj->child[i],clearcrnt);
	}
	else
		newObj->child=NULL;
}

void newObjectFree(Object *obj)
{
	int i;
	
	if (obj->ncp)
	{
#ifdef MEMORY_CHECK
		TOTALALLOC-=(sizeof(Point3)*obj->ncp);
		TOTALALLOCCHKSUM-=(int)obj->ocpt;
#endif
		CHeapFree(ObjHeap,obj->ocpt);
		obj->ncp = 0;
		obj->ocpt = NULL;
		
	}
	
	if (obj->no_child)
	{
		for (i=0;i<obj->no_child;i++)
		{
			newObjectFree(&obj->child[i]);
			
		}
#ifdef MEMORY_CHECK
		TOTALALLOC-=(sizeof(Object)*obj->no_child);
		TOTALALLOCCHKSUM-=(int)obj->child;
#endif
		CHeapFree(ObjHeap,obj->child);
		obj->child = NULL;
		obj->no_child = 0;
		
	}
	
}

void setObjId(Object **newObjId, Object **oldObjId, Object *newObj, Object *oldObj, int	noObjId)
{
	int	i,loop;
	
	
	
	for (i=1; i<=noObjId; i++)
		if (oldObj == oldObjId[i])
			newObjId[i] = newObj;
		for (loop=0; loop<newObj->no_child; loop++)
			setObjId(newObjId, oldObjId, &newObj->child[loop], &oldObj->child[loop], noObjId);
		
}

static void CleanUp(STRAT *strat)
{
	
   	/* Firstly make sure this isn't a shared object */
	if (strat->pnode && !(strat->pnode->flag & OBJECT_SHARE_OBJECT_DATA)) 
	{
		dAssert (!(strat->flag & STRAT_EXPLODEPIECE), "Somehow an exploding piece got to cleanup!");
		if (strat->objId)
		{
#ifdef MEMORY_CHECK
			TOTALALLOC-=(sizeof(Object*) * (strat->pnode->noObjId+1));
			TOTALALLOCCHKSUM-=(int)strat->objId;
#endif
			CHeapFree(ObjHeap,strat->objId);
			strat->objId = NULL;
		}
		newObjectFree(strat->obj);
		
#ifdef MEMORY_CHECK
		TOTALALLOC-=sizeof(Object);
		TOTALALLOCCHKSUM -= (int)strat->obj;
#endif
		CHeapFree(ObjHeap,strat->obj);
		strat->obj = NULL;
	}
	
	strat->pnode = NULL;
}
static void freeCollObj(CollObject *co)
{
	int i;
	
	for (i=0; i<co->obj->no_child; i++)
	{
		freeCollObj(co->child[i]);
		//return;
	}
	
	if ((co->obj) && (co->obj->no_child))
	{	
#ifdef MEMORY_CHECK
		HDCALLOC -= sizeof(CollObject*) * co->obj->no_child;
		HDCALLOCCHKSUM -= (int)co->child;
#endif
		CHeapFree(ObjHeap,co->child);
		co->child = NULL;
	}
	
	if (((co->nPoly + co->corrupt) > 0) && (co->obj->model))
	{
		
#ifdef MEMORY_CHECK
		HDCALLOC -= sizeof(CollPoly) * (co->nPoly + co->corrupt);
		HDCALLOCCHKSUM -= (int)co->v;
#endif
		CHeapFree(ObjHeap,co->v);
		co->v = NULL;
		
#ifdef MEMORY_CHECK
		HDCALLOC -= sizeof(Point3) * co->obj->model->nVerts;
		HDCALLOCCHKSUM -= (int)co->poly;
#endif
		CHeapFree(ObjHeap,co->poly);
		co->poly = NULL;
		
	}
	co->nPoly = 0;
	co->corrupt = 0;
	co->obj = NULL;
	
	
#ifdef MEMORY_CHECK
	HDCALLOC -= sizeof(CollObject);
	HDCALLOCCHKSUM -= (int)co;
#endif
	CHeapFree(ObjHeap,co);
}



static void FreeHDBlock(STRAT *strat)
{
	/*Reset High Detail Collision Warez*/
	
	HDStrat *hds;
	
	
	if (!strat->hdBlock)
		return;
	
	hds = strat->hdBlock;
	
	if ((hds == firstHDStrat) && (hds == lastHDStrat))
	{
		hds->next = NULL;
		hds->prev = NULL;
		firstHDStrat = NULL;
		lastHDStrat = NULL;
	}
	else if (hds == firstHDStrat)
	{
		firstHDStrat = hds->next;
		hds->next->prev = NULL;
	}
	else if (hds == lastHDStrat)
	{
		lastHDStrat = hds->prev;
		hds->prev->next = NULL;
	}
	else
	{
		hds->prev->next = hds->next;
		hds->next->prev = hds->prev;
	}
	
	freeCollObj(hds->collObj);
	hds->collObj = NULL;
#ifdef MEMORY_CHECK
	HDCALLOC -= sizeof(HDStrat);
	HDCALLOCCHKSUM -= (int)hds;
#endif
	CHeapFree(ObjHeap,hds);
	strat->hdBlock = NULL;
}

void SetModel(STRAT *strat,int ModelEntry)
{
	/* If there was a previous model, free that XXX is this right, MattP?  mg */
	
	if (strat->pnode)
	{
		//clean up any parts of strat that may be using the original model
		//data
		if (strat->hdBlock)
			FreeHDBlock(strat);
		
		CleanUp (strat);
		strat->obj = NULL;
		strat->objId = NULL;
	}
	
	
#if ALLRESIDENT
	strat->pnode = GameMods[ModelEntry];
#else
	strat->pnode = &GameObjects[ModelEntry];
#endif	
	
	
	//ensure all anims are copies
   	/*
	if ((strat->pnode->flag & OBJECT_SHARE_OBJECT_DATA) &&
	(strat->pnode->anim))
	strat->pnode->flag &= LNOT(OBJECT_SHARE_OBJECT_DATA);
  	 */
	
	/* Only take a copy of the object if necessary */
	if (strat->pnode->flag & OBJECT_SHARE_OBJECT_DATA) {
		strat->obj = strat->pnode->obj;
		strat->objId = strat->pnode->objId;
	} else {
		
		
		/* Take a copy for the strat */
		strat->obj = (Object *)CHeapAlloc(ObjHeap,sizeof(Object));
		
#ifdef MEMORY_CHECK
		TOTALALLOC+=sizeof(Object);
		TOTALALLOCCHKSUM+=(int)strat->obj;
#endif
		if (!strat->obj)
		{
			CrashOut("OUT OF OBJECTS", 0x0, 0);
			dAssert(strat->obj,"OUT OF OBJ SPACE\n");
		}
		
#ifdef MEMORY_CHECK
		//	CheckCHeap (IntVarHeap);
		//	CheckCHeap (FloatVarHeap);
		CheckCHeap (ObjHeap);
#endif
		
		if (strat->pnode->anim)
		{
			//ENSURE SCALE ANIM OBJ COPIES HAVE A BASE SCALE OF 1.0
			if (strat->pnode->anim->anim->type & SCALE_ANIM)
				newObject(strat->pnode->obj, strat->obj,2);
			else
				newObject(strat->pnode->obj, strat->obj,1);
			
		}
		else
			newObject(strat->pnode->obj, strat->obj,0);
		
#ifdef MEMORY_CHECK
		//	CheckCHeap (IntVarHeap);
		//	CheckCHeap (FloatVarHeap);
		CheckCHeap (ObjHeap);
#endif
		
		setParent(strat->obj, NULL);
		ResetOCP(strat);
		
#ifdef MEMORY_CHECK
		//	CheckCHeap (IntVarHeap);
		//	CheckCHeap (FloatVarHeap);
		CheckCHeap (ObjHeap);
#endif
		
		if (strat->pnode->noObjId)
		{
			/*dLog("allocating %d object ids \n",sizeof(Object*) * (strat->pnode->noObjId)); */
			strat->objId = (Object **)CHeapAlloc(ObjHeap,sizeof(Object *) * (strat->pnode->noObjId + 1));
#ifdef MEMORY_CHECK
			TOTALALLOC+=sizeof(Object*) * (strat->pnode->noObjId+1);
			TOTALALLOCCHKSUM+=(int)strat->objId;
#endif
			dAssert(strat->objId !=NULL,"OBJID Failed");
			memset(strat->objId,0,sizeof(Object*) * (strat->pnode->noObjId + 1));
			
			setObjId(strat->objId, 
				strat->pnode->objId,
				strat->obj, 
				strat->pnode->obj,
				strat->pnode->noObjId);
			
#ifdef MEMORY_CHECK
			
			//	CheckCHeap (IntVarHeap);
			//	CheckCHeap (FloatVarHeap);
			CheckCHeap (ObjHeap);
#endif
			
		}
		else
			strat->objId=NULL;
	}
	
}

static void setOCPT(STRAT *crntStrat, Object *obj)
{
	Uint32	i;
	
	/*mPreTranslate(m, obj->idlePos.x, obj->idlePos.y, obj->idlePos.z); */
	/*mPreRotateXYZ(m, obj->idleRot.x, obj->idleRot.y, obj->idleRot.z); */
	/*mPreScale(m, obj->scale.x, obj->scale.y, obj->scale.z); */
	
	if (obj->ncp > 0)
		for (i=0; i<obj->ncp; i++)
		{
			obj->ocpt[i].x = crntStrat->pos.x;
			obj->ocpt[i].y = crntStrat->pos.y;
			obj->ocpt[i].z = crntStrat->pos.z;
		}
		/*mPoint3Multiply3(&obj->ocpt[i], &obj->cp[i], m); */
		
		if (obj->no_child > 0)
			for (i=0; i<obj->no_child; i++)
				setOCPT(crntStrat, &obj->child[i]);
}


// Useful function: return the player number of a strat or -1

void SetModelMPTank(STRAT *strat, PNode *node)
{
	
	if (strat->pnode)
		CleanUp (strat);
	
	strat->pnode = node;
	
	/* Take a copy for the strat */
	strat->obj = (Object *)CHeapAlloc(ObjHeap,sizeof(Object));
	
#ifdef MEMORY_CHECK
	TOTALALLOC+=sizeof(Object);
	  	TOTALALLOCCHKSUM+=(int)strat->obj;
#endif
		dAssert(strat->obj,"OUT OF OBJ SPACE\n");
		
		if (strat->pnode->anim)
			newObject(strat->pnode->obj, strat->obj,1);
		else
			newObject(strat->pnode->obj, strat->obj,0);
		
		setParent(strat->obj, NULL);
		setOCPT(strat, strat->obj);
		
		if (strat->pnode->noObjId)
		{
			/*dLog("allocating %d object ids \n",sizeof(Object*) * (strat->pnode->noObjId)); */
			strat->objId = (Object **)CHeapAlloc(ObjHeap,sizeof(Object *) * (strat->pnode->noObjId + 1));
#ifdef MEMORY_CHECK
			TOTALALLOC+=sizeof(Object*) * (strat->pnode->noObjId+1);
			TOTALALLOCCHKSUM += (int)strat->objId;
#endif
			dAssert(strat->objId !=NULL,"OBJID Failed");
			setObjId(strat->objId, 
				strat->pnode->objId,
				strat->obj, 
				strat->pnode->obj,
				strat->pnode->noObjId);
		}
		else
			strat->objId=NULL;
}

int GetPadPush(STRAT *strat, int pn)
{
	return (PadPush[pn]);
}

int GetPadPress(STRAT *strat, int pn)
{
	return PadPress[pn];
}

void Reset(STRAT *strat)
{
	strat->stack[0]=
		strat->stack[1]=
		strat->stack[2]=
		strat->stack[3]=
		strat->stack[4]=
		strat->stack[5]=0;
	strat->timer[0]=
		strat->timer[1]=
		strat->timer[2]=
		strat->timer[3]=
		strat->timer[4]=
		strat->timer[5]=0;
	
}

static void StratReset(STRAT *strat)
{
#if 1
	int i;
	
	/*WILL BE QUICKER TO BLAT A PREDEFINED BLOCK OF STRAT INIT MEM OVER THE STRAT BLOCK */
	/*IF THIS GETS MUCH BIGGER */
	
	strat->stack=strat->stackblock;
	strat->timer=strat->timeblock;
	
	for (i=0;i<6;i++)
		strat->stack[i]=strat->timer[i]=0;
	
	strat->SoundBlock = NULL;
	strat->vel.x=strat->vel.y=strat->vel.z=0.0f;
	strat->oldOldPos.x=strat->oldOldPos.y=strat->oldOldPos.z=0.0f;
	strat->oldPos.x=strat->oldPos.y=strat->oldPos.z=0.0f;
	strat->relAcc.x=strat->relAcc.y=strat->relAcc.z=0.0f;
	strat->relVel.x=strat->relVel.y=strat->relVel.z=0.0f;
	strat->rotRest.x=strat->rotRest.y=strat->rotRest.z=0.0f;
	strat->CheckPos.x=strat->CheckPos.y=strat->CheckPos.z=0.0f;
	strat->cpn.x=strat->cpn.y=strat->cpn.z=1.0f;
	
	strat->rot_vect.x = strat->rot_vect.y = 0.0f;
	strat->rot_vect.z = 1.0f;
	strat->rot_speed=0;
	
	strat->turn.x = strat->turn.y = strat->turn.z =0.0f;
	strat->scale.x = strat->scale.y = strat->scale.z =1.0f;
	
	mUnit(&strat->m);
	
	strat->IVar=NULL;
	strat->FVar=NULL;
	
	strat->trigger=NULL;
	strat->flag=STRAT_ALIVE;
	strat->flag2=0;
	strat->parent=NULL;
	
	strat->damage = 0;
	strat->target = NULL;
	strat->trans = 1.0;
	strat->frame = 0;
	strat->var = 0.0f;
	strat->turnangz=0.0f;
	//	strat->mapentry = -1;
	
	strat->wallN.x = strat->wallN.y = strat->wallN.z = 0;
	
	//	if (PAL)
	//	strat->friction.x = strat->friction.y = strat->friction.z = PAL_ACCSCALE;
	//else
	strat->friction.x = strat->friction.y = strat->friction.z = 1.f;
	strat->CollWith = NULL;
	
	
	
	/*	strat->CollWith = NULL;
	strat->pnode=NULL;
	strat->obj = NULL;
	strat->anim=NULL;
	
	  strat->SoundBlock = NULL;
	  strat->CollBlock = NULL;
	  strat->objId=NULL;
	  strat->route=NULL;
	*/	
#else
	int i;
	STRAT *next = strat->next;
	STRAT *prev = strat->prev;
	
	memset (strat, 0, sizeof (STRAT));
	strat->next = next;
	strat->prev = prev;
	/*WILL BE QUICKER TO BLAT A PREDEFINED BLOCK OF STRAT INIT MEM OVER THE STRAT BLOCK */
	/*IF THIS GETS MUCH BIGGER */
	
	strat->stack=strat->stackblock;
	strat->timer=strat->timeblock;
	
	//	for (i=0;i<6;i++)
	//		strat->stack[i]=strat->timer[i]=0;
	
	//	strat->vel.x=strat->vel.y=strat->vel.z=0;
	//	strat->oldVel.x=strat->oldVel.y=strat->oldVel.z=0;
	//	strat->oldPos.x=strat->oldPos.y=strat->oldPos.z=0;
	//	strat->relAcc.x=strat->relAcc.y=strat->relAcc.z=0;
	//	strat->relVel.x=strat->relVel.y=strat->relVel.z=0;
	
	//	strat->rot_vect.x = strat->rot_vect.y = 0.0f;
	strat->rot_vect.z = 1.0f;
	//	strat->rot_speed=0;
	
	//	strat->turn.x = strat->turn.y = strat->turn.z =0.0f;
	strat->scale.x = strat->scale.y = strat->scale.z =1.0f;
	
	mUnit(&strat->m);
	strat->flag=STRAT_ALIVE;
	strat->trans = 0.5;
#endif
}

static STRAT *AddStrat(int StratEntry)
{
	//STRAT *temp;
	STRAT *strat;
	int i;
	strat = NextFreeStrat;
	
   	if (!strat)
	{
		CrashOut("OUT OF STRATS", (0x1f) << 16, 0);
		dAssert(strat,"OUT OF STRATS\n");
	}
	
	if (strat)
	{
		
		
		
		STRATNUM++;
		NextFreeStrat = NextFreeStrat->next;
		
		/*fAssert ((strat == NULL) && (STRATNUM) , "OUT OF STRAT SPACE"); */
		
		//link strat to the firststrat
		//NEW STRAT -> FIRSTSTRAT
		strat->next = FirstStrat;  
		
		//point firststrat back to the new strat
		//NEW STRAT <-> FIRSTSTRAT
		if (FirstStrat)
			FirstStrat->prev = strat;
		
		FirstStrat = strat;	
		
		//RESET THE VARIABLES
		StratReset(strat);
		
		strat->func = STRATS[StratEntry].address;
		
		if (STRATS[StratEntry].numfloat)
		{
			strat->FVar = CHeapAlloc (FloatVarHeap,sizeof(float)*STRATS[StratEntry].numfloat);
#ifdef MEMORY_CHECK
			VARALLOC+=sizeof(float) * STRATS[StratEntry].numfloat;
			VARALLOCCHKSUM+=(int)strat->FVar;
			strat->entry = (short)StratEntry;   
#endif
			
			if (!strat->FVar)
			{
				CrashOut("OUT OF FVARS", (0x1f) << 8, 0);
				dAssert(strat->FVar,"out of FVARS \n");	
			}
			
			if (!(strat->FVar))
			{
				//	dLog("out of FVARS \n");	
				strat->flag = 0;
				RemoveStrat(strat);
				return(NULL);
			}
			// XXX
			for (i=0;i<STRATS[StratEntry].numfloat;i++)
				strat->FVar[i]=0.0f;
			// end XXX
		}		
		
		if (STRATS[StratEntry].numint)
		{
			strat->IVar = CHeapAlloc (IntVarHeap,sizeof(int)*STRATS[StratEntry].numint);
#ifdef MEMORY_CHECK
			VARALLOC+=sizeof(int) * STRATS[StratEntry].numint;
			VARALLOCCHKSUM+=(int)strat->IVar;
			strat->entry = (short)StratEntry;   
#endif
			
			if (!strat->IVar)
			{
				CrashOut("OUT OF IVARS", 31, 0);
				dAssert(strat->IVar,"out of IVARS \n");	
			}
			
			if (!(strat->IVar))
			{
				//	dLog("out of IVARS \n");	
				strat->flag = 0;
				RemoveStrat(strat);
				return(NULL);
			}
			// XXX
			for (i=0;i<STRATS[StratEntry].numint;i++)
				strat->IVar[i]=0;
			// end XXX
		}			
		
		
		
		
		/*		strat->waypoint=&wayvec; */
		
		/*if (globalStratID == 0)
		globalStratID = 1;
		strat->id = globalStratID;
		globalStratID++;  */
		return(strat);	
	}
	else
	{
		dLog("****** OUT OF STRAT SPACE *******\n");
	}
	return(NULL);
}

static void CleanExplodePiece (STRAT *strat)
{
	// We are sharing our parent's object
	strat->obj = NULL;
	strat->pnode = NULL;
}

static void DeleteMapStrat(STRAT *strat)
{
	
	//ENSURE AN IMMEDIATE REMOVAL OF THE STRAT
	strat->flag2 |= STRAT2_IMMEDIATEREMOVAL;
	strat->flag2 &= LNOT(STRAT2_SUSPENDED);
	
	Delete(strat);
	RemoveStrat(strat);
}

static void FreeASBlock(STRAT *strat)
{
	/*Reset High Detail Collision Warez*/
	
	StratList *sl;
	
	dAssert(firstAvoidStrat, "AvoidStrat has gone maaaaaaaad");
	
	sl = firstAvoidStrat;
	while (sl)
	{
		if (sl->strat == strat)
			break;
		
		sl = sl->next;
	}
	
	if (sl->strat != strat)
		dAssert(0, "AvoidStrat problem");
	
	if ((sl == firstAvoidStrat) && (sl == lastAvoidStrat))
	{
		sl->next = NULL;
		sl->prev = NULL;
		firstAvoidStrat = NULL;
		lastAvoidStrat = NULL;
	}
	else if (sl == firstAvoidStrat)
	{
		firstAvoidStrat = sl->next;
		sl->next->prev = NULL;
	}
	else if (sl == lastAvoidStrat)
	{
		lastAvoidStrat = sl->prev;
		sl->prev->next = NULL;
	}
	else
	{
		sl->prev->next = sl->next;
		sl->next->prev = sl->prev;
	}
	
	CHeapFree(ObjHeap,sl);
	strat->flag &= ~STRAT_AVOIDSTRAT;
}



/*ENSURE THE STRAT IS KILLED OFF SORT OF THING */

void Delete(STRAT *strat)
{
	
	int i;
	//invalidate our objects if we're exploding bits
	//if (strat->flag & STRAT_EXPLODEPIECE)
	//	CleanExplodePiece(strat);
	
	if (strat->flag & STRAT_DELETECALLBACK)
	{
		/* envoke any deletion callbacks*/
		ProcTriggerDeleted(strat,strat->trigger);
		strat->flag = strat->flag & LNOT(STRAT_DELETECALLBACK);
	}
	
	if (strat->flag & STRAT_AVOIDSTRAT)
	{
		FreeASBlock(strat);
	}
	
	
	
	
	if (Multiplayer)
	{
		for (i=0; i<4; i++)
			RemoveTarget(strat, i, -1);
	}
	else
		RemoveTarget(strat, 0, -1);
	
	/*REMOVES FROM PRIMARY TARGET LIST */
   	if (strat->flag2 & STRAT2_TARGETTED)
   	{
		RemovePrimary(strat);
		DecLockOns(strat);
   	}
	
	
	/*TAKE OUT FROM LOCKON LIST */
	/*SHOULD GO INTO MISSILE STRATS */
	
   	if (strat->target)
		DecLockOns(strat->target);
	
	
	/*FLAG THE STRAT TO BE KILLED, BY CLEARING ITS STATUS FLAG */
	strat->flag = STRAT_DEAD;
	
	/*	strat->pnode=NULL; */
	/*TO ENSURE NO TRIGGER KICKS IN AFTER THE DELETION IF CALLED FROM WITHIN A TRIGGER */
	if (strat->trigger) 
		PauseTriggers(strat,-2,0); 
	
	strat->flag2 |= STRAT2_PROCESSSTOPPED;
	
	/*strat->trigger=NULL; */
}

/*KILLS OFF THE STRAT'S PARENT WHEN REQUESTED*/
void DeleteMyParent(STRAT *strat)
{
	
	if (strat->parent)
    {
		//	DeleteMapStrat(strat->parent);
		//ensure the parent is not within the valid area array for strat avoidance
		DeleteValidAreaEntry(strat->parent);
		Delete(strat->parent);
		strat->parent = NULL;
	}
}



static void	SearchAnim(struct tagAnimation *anim)
{
	int i;
	
	for (i=0;i<MAX_MODELANIM;i++)
	{
		if (&ModelAnimPool[i].anim == anim)
		{
			ModelAnimPool[i].next = LastModelAnim;
			LastModelAnim = &ModelAnimPool[i];
			break;
			
		}
		
	}
	
}




static void freeHPBlock(STRAT *strat)
{
	struct tag_HoldPlayerBlock *temp_hpb;
	
	temp_hpb = hpbFirst;
	while(temp_hpb)
	{
		if (temp_hpb->strat == strat)
		{
			
			temp_hpb->strat = NULL;
			if ((temp_hpb == hpbFirst) && (temp_hpb == hpbLast))
			{
				hpbFirst = NULL;
				hpbLast = NULL;
#ifdef MEMORY_CHECK
				HPBALLOC -= sizeof(HoldPlayerBlock);
				HPBALLOCCHKSUM -= (int)temp_hpb;
#endif
				CHeapFree(ObjHeap,temp_hpb);
			}
			else if(temp_hpb == hpbFirst)
			{
				hpbFirst->next->prev = NULL;
				hpbFirst = hpbFirst->next;
				CHeapFree(ObjHeap,temp_hpb);
#ifdef MEMORY_CHECK
				HPBALLOC -= sizeof(HoldPlayerBlock);
				HPBALLOCCHKSUM -= (int)temp_hpb;
#endif
			}
			else if(temp_hpb == hpbLast)
			{
				hpbLast->prev->next = NULL;
				hpbLast = hpbLast->prev;
				CHeapFree(ObjHeap,temp_hpb);
#ifdef MEMORY_CHECK
				HPBALLOC -= sizeof(HoldPlayerBlock);
				HPBALLOCCHKSUM -= (int)temp_hpb;
#endif
			}
			else
			{
				temp_hpb->prev->next = temp_hpb->next;
				temp_hpb->next->prev = temp_hpb->prev;
				CHeapFree(ObjHeap,temp_hpb);
#ifdef MEMORY_CHECK
				HPBALLOC -= sizeof(HoldPlayerBlock);
				HPBALLOCCHKSUM -= (int)temp_hpb;
#endif
			}
			break;
		}
		
		temp_hpb = temp_hpb->next;
	}
	
	strat->hpb = NULL;
}

void Clean(STRAT *strat)
{
	
	// Band debugging
#ifdef _DEBUG
	BandCleanDebug (strat);
#endif
	
	if (strat->route)
	{
		ClearBusy(strat);
		/*	if (!strat->parent)
		{*/
		//FREE UP ALLOCATED SPLINE DATA INFO, IF NEEDED
		if (strat->route->splineData)
		{
#ifdef MEMORY_CHECK
			ALLOCSPLINES -= sizeof(SplineData);
			ALLOCSPLINESCHKSUM -= (int)strat->route->splineData;   
#endif
			CHeapFree(ObjHeap,strat->route->splineData);
			strat->route->splineData = NULL;
		}
		//RELEASE THE ROUTE BLOCK
		strat->route->next = LastRoute;
		LastRoute=strat->route;
		strat->route = NULL;
		/*}*/
	}
	strat->func = NULL;
	
	if (strat->hpb)
	{
		freeHPBlock(strat);
		strat->flag2 = strat->flag2 & LNOT(STRAT2_HOLDPLAYER);
	}
	
	if (strat->hdBlock)
	{
		FreeHDBlock(strat);
	}
	
	
	if (strat->flag2 & STRAT2_STRATTARGET)
	{
		UnRegisterAsTarget(strat);
		strat->flag2 = strat->flag2 & LNOT(STRAT2_STRATTARGET);
		
	}
	if (strat->IVar)
	{
#ifdef MEMORY_CHECK
		VARALLOC-=sizeof(int) * STRATS[strat->entry].numint;
		VARALLOCCHKSUM-=(int)strat->IVar;
#endif
		CHeapFree(IntVarHeap,strat->IVar);
		strat->IVar=NULL;
	}
	if (strat->FVar)
	{
#ifdef MEMORY_CHECK
		VARALLOC-=sizeof(float) * STRATS[strat->entry].numfloat;
		VARALLOCCHKSUM-=(int)strat->FVar;
#endif
		CHeapFree(FloatVarHeap,strat->FVar);
		strat->FVar=NULL;
	}
	
	if (strat->obj)
	{
		CleanUp(strat);
		strat->obj = NULL;
		strat->objId = NULL;
	}
	
	strat->pnode=NULL;
	
	/*Release any resident triggers */
	if (strat->trigger)
		KillStratTriggers(strat);
	
	/*FREE ITS ANIMS*/
	if (strat->anim)
	{
		strat->anim->next = LastAnim;
		LastAnim=strat->anim;			
		
		/*SEE IF IT WAS A MODEL ANIM AND FREE UP THE RELEVANT STORAGE BLOCK IF IT WAS*/
		
		if (strat->anim->anim.flags & MODEL_ANIM)
		{
			
			/*INVALIDATE THE INTERPOLATION DATA IF NEED BE*/
			/*	if (strat->pnode->anim->anim->type & INTERPOLATE_ANIM)
			((KEYJUMP*)strat->anim->anim.anim->typeData) = NULL;
			*/
			
			/*	((int*)strat->anim->anim.anim->typeData) = ((int*)0xffffffff); */
			strat->anim->anim.flags = MODEL_ANIM;
			
			/*FIND LAST MODEL ANIM*/
			SearchAnim(strat->anim->anim.anim);	
			
		}
		strat->anim->anim.anim = NULL;
		
		strat->anim = NULL;
		
	}
	
	if (strat->CollBlock)
	{
		if (strat->CollBlock->next)
			strat->CollBlock->next->prev = strat->CollBlock->prev;
		if (strat->CollBlock->prev)
			strat->CollBlock->prev->next = strat->CollBlock->next;
		else
			FirstColl = strat->CollBlock->next;		
		
		strat->CollBlock->prev = NULL;
		
		strat->CollBlock->next = NextFreeColl;
		NextFreeColl = strat->CollBlock;
		
		strat->CollBlock = NULL;
		
	}
	
	
	if (strat->SoundBlock)
	{
		FreeStratSoundBlock (strat);
		strat->SoundBlock = NULL;
	}

	// If it was a player, then set its activeness to 0, and kill its camera
	if (strat->Player) {
		dAssert (strat->Player->Player == strat, "Erk");
		strat->Player->active = FALSE;
		strat->Player->CameraStrat = NULL;
		strat->Player->Player = NULL;
		strat->Player = NULL;
	}
	
	
}


static void RemoveStrat(STRAT *strat)
{
	int i;
	
	dAssert (strat->flag == 0, "REMOVING A LIVING STRAT!!!!");	
	//update the map if a strat is a map created one
   	if (strat->flag2 & STRAT2_MADEOFFMAP)
	{
		if (!(strat->flag2 & STRAT2_SUSPENDED))
		{
			for (i=0;i<NumStrats;i++)
			{
				if (GameMap[i].strat == strat)
					GameMap[i].strat = NULL;
			}
			
			Clean(strat);
			
			if (strat->flag2 & STRAT2_IMMEDIATEREMOVAL)
			{
				//RELEASE THE BLOCK
				
				STRATNUM --;	
				if (strat->next)
					strat->next->prev = strat->prev;
				if (strat->prev)
					strat->prev->next = strat->next;
				
				//if strat was the firststrat then make firststrat
				//the next in the list
				if (strat == FirstStrat)
				{
					if (strat->next)
						FirstStrat = strat->next;
					else
						FirstStrat = NULL;
					
				}
				
				strat->prev = NULL;
				
				//POINT STRAT TO THE NEXTFREE ONE IN LIST
				strat->next = NextFreeStrat;
				
				//NEXT FREE STRAT BECOMES ME
				NextFreeStrat = strat;
			}
			strat->flag2 |= STRAT2_SUSPENDED;
		}
		
	}
   	else
	{
		//RELEASE THE BLOCK
		
		STRATNUM --;	
		if (strat->next)
			strat->next->prev = strat->prev;
		if (strat->prev)
			strat->prev->next = strat->next;
		
		if (strat == FirstStrat)
		{
			if (strat->next)
				FirstStrat = strat->next;
			else
				FirstStrat = NULL;
			
			
		}	
		strat->prev = NULL;
		
		strat->next = NextFreeStrat;
		NextFreeStrat = strat;
		Clean(strat);
		
	}
}

void findTotalMatrix(Matrix *mat, Object *obj)
{
	dAssert(obj, "no object in find total matrix");
	if (obj->parent != NULL)
		findTotalMatrix(mat, obj->parent);
	
	mPreTranslate(mat, obj->crntPos.x, obj->crntPos.y, obj->crntPos.z);	
	mPreRotateXYZ(mat, obj->crntRot.x, obj->crntRot.y, obj->crntRot.z);
	mPreScale(mat, obj->scale.x, obj->scale.y, obj->scale.z);
}

void findTotalMatrixInterp(Matrix *mat, Object *obj)
{
	if (obj->parent != NULL)
		findTotalMatrixInterp(mat, obj->parent);
	
   	mPreMultiply(mat,&obj->m);
	
}



//LINKS A STRAT TO PARENT'S OBJECT
//NOTE: STRAT MATRIX WILL BE IN THE SCALE OF THE LAST PIECE IN THE HIERARCHY TRAVERSED
//THIS IS REMOVED BY THE 'MNORMALISE' IN PROCESSSTRATS, EFFECTIVELY, RESETTING THE
//SCALE BACK TO UNIT, EVERY FRAME
//ENSURE ANY FUNCTIONS NEEDING THIS MATRIX, LINE SIGHT, COLLISION ETC, GET DONE, BEFORE
//THIS CALL IS MADE EACH FRAME
void LinkToParentObject(STRAT *strat, int id, float xang, float yang, float zang, float offsetx, float offsety, float offsetz)
{
	Object *object;
	//	int id;
	
	if ((strat->parent) && (!(strat->parent->flag == STRAT_DEAD)))
	{
		
		if ((!(strat->parent->pnode)) || (id > strat->parent->pnode->noObjId))
			return;
		
		//	id = (int)obj;
		
		
		strat->m= strat->parent->m;
		
		
		
		mPreScale(&strat->m,strat->parent->scale.x,strat->parent->scale.y,strat->parent->scale.z);
		
		
		
		if (id)
		{
			object = strat->parent->objId[id];
			findTotalMatrixInterp(&strat->m, object);
		}
		
		mPreTranslate(&strat->m,offsetx,offsety,offsetz);
		
		
		mPreRotateXYZ(&strat->m, xang, yang, zang);
		
		strat->pos = strat->parent->pos;
		strat->pos.x += strat->m.m[3][0];
		strat->pos.y += strat->m.m[3][1];
		strat->pos.z += strat->m.m[3][2];
		
		//takes out any scale nonsense left over
		mNormalize(&strat->m);
		
		strat->m.m[3][0] = strat->m.m[3][1] = strat->m.m[3][2] = 0;
		
		
	}
	
	
}




static int	objIdEqualsObj(Object *obj, Object *obj2)
{
	int	i;
	
	if (obj == obj2)
		return 1;
	
	for (i=0; i<obj->no_child; i++)
		if (objIdEqualsObj(&obj->child[i], obj2))
			return 1;
		
		return 0;
}

//static void loop(STRAT *strat)
//{
//}

static void DelayMe(STRAT *strat)
{
	int i = 0;
	
	dLog("TIMER %d strat %x\n",strat->timer[0],strat);
	if (strat->timer[0] < 70)
		strat->timer[0]++;
	else
	{
		
		/*REMOVES FROM SAV'S LIST */
		if (Multiplayer)
		{
			for (i=0; i<4; i++)
				RemoveTarget(strat, i, -1);
		}
		else
			RemoveTarget(strat, i, -1);
		
		
		//if (strat != player[currentPlayer].Player)
		//ensure the single level player is not destroyed
		//	if ((Multiplayer) || ((!Multiplayer) && (strat != player[0].Player)))
		Delete(strat);
		if (strat == BossStrat)
			BossStrat = NULL;
		//else
		//	strat->func = &loop;
	}
}

extern	STRAT	*waterStrat[32];
extern	int	noWaterStrats;
extern	STRAT	*lavaStrat[32];
extern	int	noLavaStrats;

void AddWaterStrat(STRAT *strat)
{
	Object *obj;
	
	dAssert(strat->pnode, "no pnode in water");
	waterStrat[noWaterStrats] = strat;
	
	
	if (strat->obj->model)
		obj = strat->obj;
	else if (strat->obj->child[0].model)
		obj = &strat->obj->child[0];
	else
		return;
	
	objectToWorld(strat);
	mShortInvert3d(&obj->im, &obj->wm);
	noWaterStrats++;
}

void IAmWater(STRAT *strat, float choppiness, float waveHeight)
{
	int ID;
	rSeaLevel = strat->pos.z;
	rSeaChoppiness = choppiness;
	rWaveHeight = waveHeight;
	
	ID = GetGlobalParamInt(0);
	if (ID)
	{
		rSeaEffectOn(strat->objId[ID]->model->material);
   	}
	else
	{
		dAssert (strat->pnode && strat->pnode->obj &&
			strat->pnode->obj->model &&
			strat->pnode->obj->model->material,
			"Knackered model on the strat");
		rSeaEffectOn(strat->pnode->obj->model->material);
	}
}

void ExplodeMe(STRAT *parent, int StratEntry)
{
	STRAT *strat;
	int i;
	int index = 1001;
	
	
	/*TURN OFF PARENT BY RUBBING HIM THE WRONG WAY */
	
	parent->flag |= STRAT_HIDDEN;
	parent->flag &= LNOT(STRAT_COLLSTRAT);
	if (parent->flag2 & STRAT2_TARGETTED)
	{
		RemovePrimary(parent);
		DecLockOns(parent);
		parent->flag2 &= LNOT(STRAT2_TARGETTED);
		
	}
	//ensure we are running parent code
	parent->flag2 = parent->flag2 & LNOT(STRAT2_PROCESSSTOPPED);
	
	//RUN ANY DELETION CALLBACKS
	if (parent->flag & STRAT_DELETECALLBACK)
	{
		/* envoke any deletion callbacks*/
		ProcTriggerDeleted(parent,parent->trigger);
		parent->flag = parent->flag & LNOT(STRAT_DELETECALLBACK);
	}
	
	
	if (!parent->pnode->noObjId)
	{
		if (parent->flag2 & STRAT2_RESPAWN)
		{
			return;
		}
		else
		{
			if (parent->trigger) 
				PauseTriggers(parent,-2,0); 
			
			parent->func = &DelayMe;
			parent->timeblock[0]=0;
			return;
		}
	}
	
	
	for (i=1;i<32;i++,index++)
		if (i <= parent->pnode->noObjId)
		{
			// Multiplayer warez - only explode every other objIds
			if (Multiplayer && (i&1))
				continue;
			
			strat = Spawn(parent,StratEntry,0,0,0,0,0,0,index);
			if (strat)
			{
				
				/*FEED IN THE SPEED AND ACCEL VALUES FROM THE PARENT */
				strat->vel = parent->vel;
				/*strat->relAcc = parent->relAcc; */
				/*MAKE SURE IT HAS FORWARD FORCE */
				/*strat->relAcc.y *= 0.5f; */
				/*strat->relAcc.y += 0.4f; */
				/*make sure it has an upward force */
				/*strat->vel.z += 0.8f; */
				
				/*PUT SOME SPINNING WAREZ ON THE PIECE RELATIVE TO ITS SPEED */
				/*Roll(strat,(strat->vel.x/PI2) * 0.07f); */
				
				/*Yaw(strat,(strat->vel.y/PI2) * 0.08f); */
				
				/*Pitch(strat,(strat->vel.z/PI2) * 0.09f); */
				
				
			}
			
		}
		
		/*Release any resident triggers */
		if (Multiplayer)
		{
			if (parent->flag & STRAT2_RESPAWN)
			{
				/*TO ENSURE NO TRIGGER KICKS IN AFTER THE DELETION IF CALLED FROM WITHIN A TRIGGER */
				if (parent->trigger) 
					PauseTriggers(parent,-2,0); 
				parent->func = &DelayMe;
				parent->timeblock[0]=0;
			}
		}
		else
		{
			/*TO ENSURE NO TRIGGER KICKS IN AFTER THE DELETION IF CALLED FROM WITHIN A TRIGGER */
			if (parent->trigger) 
				PauseTriggers(parent,-2,0); 
			parent->func = &DelayMe;
			parent->timeblock[0]=0;
		}
		
		
		
}

static float getScale(STRAT *strat, int objId)
{
	float scale=1.0f;
	Object *obj;
	
	obj = strat->objId[objId];
	/*scale = obj->scale.x; */
	
	while (obj->parent)
	{
		obj = obj->parent;
		scale *= obj->scale.x;
	}
	scale *= strat->scale.x;
	return scale;
}



#define INHERIT 1

STRAT *Spawn(STRAT *parent, int StratEntry, float xpos, float ypos, float zpos, float xang, float yang, float zang, int model)
{
	STRAT *strat;
	Matrix mat, rotMat;
	Point3 vec2;
	if (model > 1000)
	{
		if (!objIdEqualsObj(parent->obj, parent->objId[model - 1000]))
			return(NULL);
		else if (parent->objId[model - 1000]->no_child)
			return(NULL);
	}
	
	strat = AddStrat(StratEntry);
	//	dAssert(strat,"OUT OF STRAT SPACE DADDIO\n");
	
	
   	if (!(strat)) 
		return; 
	
	strat->parent = parent;
	/*IF SPAWN ANIM */
	if (model > 1000)
	{
		mUnit(&strat->m);
		strat->obj = parent->objId[model - 1000];
		
		strat->pnode = parent->pnode;
		strat->obj->idlePos.x = 0.0f;
		strat->obj->idlePos.y = 0.0f;
		strat->obj->idlePos.z = 0.0f;
		strat->obj->crntPos = strat->obj->idlePos;
		strat->pos.x = parent->objId[model-1000]->wm.m[3][0];
		strat->pos.y = parent->objId[model-1000]->wm.m[3][1];
		strat->pos.z = parent->objId[model-1000]->wm.m[3][2];
		strat->scale.x = getScale(parent, model-1000);
		strat->scale.y = strat->scale.z = strat->scale.x; 
		strat->flag |= STRAT_EXPLODEPIECE;
		mPreRotateXYZ(&strat->m, xang, yang, zang);
		
	}
	else if ((model) && (parent->objId) && (model <= parent->pnode->noObjId))	
	{
		//	LinkToParentObject(strat,(float)model,xang,yang,zang,xpos,ypos,zpos);
		LinkToParentObject(strat,model,xang,yang,zang,xpos,ypos,zpos);
		/*	if ((parent->anim) && (parent->anim->anim.anim))
		LinkToParentObject(strat,(float)model,xang,yang,zang,xpos,ypos,zpos);
		else
		{
		
		  mUnit(&mat);
		  mUnit(&rotMat);
		  
			
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
								mPreRotateXYZ(&strat->m, xang, yang, zang);
								
	}*/
	}
	else
	{
		strat->m = parent->m;
		strat->pos = parent->pos;
		strat->pos.x += (strat->m.m[0][0] * xpos) + (strat->m.m[1][0] * ypos) + (strat->m.m[2][0] * zpos);
		strat->pos.y += (strat->m.m[0][1] * xpos) + (strat->m.m[1][1] * ypos) + (strat->m.m[2][1] * zpos);
		strat->pos.z += (strat->m.m[0][2] * xpos) + (strat->m.m[1][2] * ypos) + (strat->m.m[2][2] * zpos);
		mPreRotateXYZ(&strat->m, xang, yang, zang);
		
	}
	dAssert(strat->m.m[0][0] == strat->m.m[0][0], "qnan");
	dAssert (strat->pos.x == strat->pos.x, "QNAN");
	
	strat->oldPos = strat->pos;
	strat->oldOldPos = strat->pos;
	
	strat->func(strat);	 
	return(strat);
}

void InheritMyParent(STRAT *strat)
{
	int i;
	if (strat->parent)
	{
		if (strat->parent->route)
		{
			if (!(strat->route))
				strat->route = GetNextRoute(strat);
			
			if (strat->route)
			{
				//IF THE STRAT ALREADY HAS A SPLINE BLOCK ALLOCATED
				//THEN FREE IT UP
				if (strat->route->splineData)
				{
#ifdef MEMORY_CHECK
					ALLOCSPLINES -= sizeof(SplineData);
					ALLOCSPLINESCHKSUM -= (int)strat->route->splineData;   
#endif
					
					CHeapFree(ObjHeap,strat->route->splineData);
				}
				*strat->route = *(strat->parent->route);
				strat->route->splineData = NULL;	
				dAssert(strat->route,"ROUTE ERROR \n");
				
			}
			/*INHERIT THE CONTROLLING VARIABLE*/
			strat->var = strat->parent->var;
			/*INHERIT PARENT NETWORK/WAYPATH CONTROL VARS*/
			if (strat->parent->flag2 & STRAT2_INDEPENDENTROT)
				strat->flag2 |= STRAT2_INDEPENDENTROT;
			if (strat->parent->flag2 & STRAT2_ENTIRENET)
				strat->flag2 |= STRAT2_ENTIRENET;
		}
#if 0
		//inherit the activation details from the parent
		for (i=0;i<5;i++)
		{
			strat->actindex[i] = strat->parent->actindex[i];
			
			
		}
#endif
		
	}
	
	
}




void SpawnAbs(STRAT *parent, int StratEntry, float xpos, float ypos, float zpos, float xang, float yang, float zang)
{
	STRAT *strat;
	/*	Vector3 vec2; */
	
	strat = AddStrat(StratEntry);
	
	if (!(strat))
		return;
	
	mPreRotateY(&strat->m,yang);
	mPreRotateZ(&strat->m,zang);
	mPreRotateX(&strat->m,xang);
	
	/*	stratRoll(strat,yang); */
	/*	stratYaw(strat,zang); */
	/*	stratPitch(strat,xang); */
	
	
	strat->parent = parent;		  
	strat->pos.x = xpos;
	strat->pos.y = ypos;
	strat->pos.z = zpos;
	strat->oldPos = strat->pos;
	strat->oldOldPos = strat->pos;
	strat->func(strat);	
}

static void resetPlayerVariables(int pn)
{
	int i, temp[4];
	
	for (i=0; i<4; i++)
		temp[i] = player[pn].kills[i];
	memset (&player[pn], 0, offsetof(PLAYER, cam));
	memset (&player[pn].PlayerSecondaryWeapon, 0, sizeof (player[pn].PlayerSecondaryWeapon));
	player[pn].aimStrat = NULL;
	player[pn].PlayerOnHoldPlayer = 0;
	player[pn].PlayerState = PS_NORMAL;
	player[pn].PlayerWeaponPower = 0;
	player[pn].gunDischarged = 1;
   	player[pn].onBoat = 0;
	player[pn].ThonGravity = 1.f;
	player[pn].n = pn;
	if (!Multiplayer)
	{
		SetupSpecials();
		if (player[0].reward & UPGRADE_ARMOUR_1)
			player[pn].maxHealth = UPGRADE_ARMOUR_1_HEALTH;
		if (player[0].reward & UPGRADE_ARMOUR_2)
			player[pn].maxHealth = UPGRADE_ARMOUR_2_HEALTH;
	} else
		player[pn].maxHealth = 100.f * GetHandicap(pn);
	
	for (i=0; i<4; i++)
		player[pn].kills[i] = temp[i];
	
}

void SetPlayer(STRAT *strat)
{
	// Fill in the structure for multiplayer, and set up the model
	int ph, ps;
	Bool checkLives;
	
	if (Multiplayer) {
		int i;
		if (GetCurrentGameType()==KNOCKOUT)
			checkLives = TRUE;
		else
			checkLives = FALSE;
		for (i = 0; i < NumberOfPlayers; ++i) {
			if (player[i].active == FALSE) {
				if (checkLives && player[i].lives == 0)
					continue;
				break;
			}
		}
		dAssert (i!=4, "Erm");
		// Reset after death variables go here
		
		resetPlayerVariables(i);

		player[i].rdhm = NULL;
		// MattG added
		player[i].ThonGravity = 1.f;
		// end MattG added
		player[i].active = TRUE;
		player[i].Player = strat;
		player[i].Player->Player = &player[i];
		player[i].PlayerSecondaryWeapon.amount = player[i].PlayerSecondaryWeapon.type = 0;
		//player[i].adrelanin = player[i].adrelaninGlow = 0;
		// Clean up any previous player data
		if (strat->pnode)
		{
			if (strat->hdBlock)
				FreeHDBlock(strat);
			
			CleanUp (strat);
			strat->obj = NULL;
			strat->objId = NULL;
		}
		strat->obj = (Object *)CHeapAlloc(ObjHeap,sizeof(Object)); //MultiTank[MultiTankType[i]][0]->obj;
		newObject(MultiTank[MultiTankType[i]][0]->obj, strat->obj,0);
		setParent (strat->obj, NULL);
		setOCPT(strat, strat->obj);
		strat->pnode = player[i].lod[0];
		strat->objId = (Object **)CHeapAlloc(ObjHeap,sizeof(Object *) * (strat->pnode->noObjId + 1));
		memset (strat->objId, 0, sizeof(Object *) * (strat->pnode->noObjId + 1));
		setObjId(strat->objId, 
			strat->pnode->objId,
			strat->obj, 
			strat->pnode->obj,
			strat->pnode->noObjId);
		// Make predator as necessary
		if (GetCurrentGameType() == PREDATOR && player[i].special) {
			MakePredator(i);
		}
	} else {
		
		ph = player[0].PlayerHeld;
		ps = player[0].PlayerState;
		resetPlayerVariables(0);
		player[0].ThonGravity = 1.f;
		player[0].PlayerState = ps;
		player[0].PlayerHeld = ph;
		player[0].special = 0;
	}
	
	OldPlayer=strat;
	// Crazy shield sparks only in single player mode
	if (!Multiplayer)
	{
		MakeShieldSparkTable (strat->objId[12]->model);
		currentPlayer = 0;
		player[currentPlayer].n = currentPlayer;
		player[currentPlayer].Player = strat;
		player[currentPlayer].Player->Player = &player[currentPlayer];
		player[currentPlayer].normalGunObj = player[currentPlayer].Player->objId[5];
		HideObject(strat, 23);
		
	}
	strat->Player->PlayerShieldEnergy = 1.0f;
	/*	Player->iArmour = 0.75f; */
	/*	Player->health = 0.5f; */
	
}

void SetCollisionDetail(STRAT *strat, int level)
{
	switch (level)
	{
	case 0:
		strat->flag &= ~STRAT_COLLSTRAT;
		break;
	case 1:
		strat->flag = STRAT_LOW_DETAIL_COLLISION;
		break;
	}
}

void SetCamera(STRAT *strat)
{
	PLAYER *p = strat->parent->Player;
	p->CameraStrat=strat;
	p->CameraStrat->parent = NULL;
	p->camLookCrntPos = p->CameraStrat->pos;
	
}

void MakeMeTheLast(STRAT *strat)
{
	STRAT *prevpoint = strat->prev;
	STRAT *nextpoint = strat->next;
	STRAT *CURRstrat,*laststrat;   	
	
	
	
	//CHECK TO GET OUT IF THE ONLY STRAT IN THE SCENE 
	
   	if (!strat->next)
		return;
	
	
	/*DELETE FROM WHERE WE WERE PLACED*/
	if (nextpoint)
		strat->next->prev = prevpoint;
	if (prevpoint)
		strat->prev->next = nextpoint;
	
   	CURRstrat = FirstStrat;
	
	//FIND THE LAST STRAT   
	laststrat = CURRstrat;
	
	
	while(CURRstrat)
	{	
		laststrat = CURRstrat;
		CURRstrat=CURRstrat->next;
	}  
	
	//laststrat now holds the last strat to be strat proc'd in the list
	//so point our existing strat to it
	
	strat->prev = laststrat;
	laststrat->next = strat;
	
	
	//laststrat->next = strat;
	//strat->prev = laststrat;
	
	//	strat->next = CURRstrat;
	strat->next = NULL;
	
	if (strat == FirstStrat)
		FirstStrat = nextpoint;
	
	
}


int MissionFailure;
void StratInit()
{
	STRAT *strat=StratPool;
	GAMETARGETS *target = TargetPool;
	TRIGGER	*trig=Triggers;
	ANIMINST *anim=AnimPool;
	MODELANIMINST *ModelAnim=ModelAnimPool;
	ROUTE    *route=StratRoutes;
	COLLSTRAT *coll = StratColls;
	
	int		i;
	int error;
	
	globalVar = globalVar2 = 0;
	
	ScoobyDoo=NULL;
	rResetFragments();
	clearTargetArray(0);
	clearTargetArray(1);
	clearTargetArray(2);
	clearTargetArray(3);
	InitArrowArray();
	resetSkid(0);
	resetSkid(1);
	resetSkid(2);
	resetSkid(3);
	SpecialStrat = NULL;
	firstAvoidStrat = NULL;
	lastAvoidStrat = NULL;
	currentCamera = NULL;
	ClearCollGlobals();
	initSpark();
	InitValidAreaArray();
	RadarUpdate(NULL,0);
	SoundReset();
	
	
	for (i=0;i<MAX_LOCKEDTARGETS;i++)
	{
		Locks[i].strat = NULL;
		Locks[i].lockedon =0;
		
	}
	
	FirstStrat = NextFreeStrat = NULL;
	
	for (i = 0; i < MAX_STRATS; i++, strat++)  
	{
		if (strat->func)
			error = 1;
		
		memset (strat, 0, sizeof (STRAT));
		
		/*		strat->SoundBlock = NULL;
		strat->objId = NULL;
		strat->obj = NULL;
		strat->IVar = NULL;
		strat->FVar = NULL;
		strat->route = NULL;
		strat->anim = NULL;
		strat->CollBlock = NULL;
		strat->hdBlock = NULL;
		strat->hpb = NULL;
		strat->pnode = NULL;
		strat->prev = NULL;
		strat->Player = NULL;  */
		strat->next = NextFreeStrat;
		NextFreeStrat = strat;
	}  
	
	//	FirstStrat = NextFreeStrat;
	
	
	strat = &DummStrat;
	memset (&DummStrat, 0, sizeof (STRAT));
	
	/*	strat->SoundBlock = NULL;
	strat->objId = NULL;
	strat->obj = NULL;
	strat->IVar = NULL;
	strat->FVar = NULL;
	strat->route = NULL;
	strat->hpb = NULL;
	strat->anim = NULL;
	strat->CollBlock = NULL;
	strat->pnode = NULL;
	strat->hdBlock=NULL;
	strat->Player = NULL;
	*/
	//LastStrat = NextFreeStrat;
	
	FirstTarget = NextFreeTarget = NULL;
	
	
	for (i = 0; i < MAX_GAMETARGETS; i++, target++)  
	{
		
		target->strat = NULL;
		target->occupied = 255;
		target->prev = NULL;
		target->next = NextFreeTarget;
		NextFreeTarget = target;
	}	
	
	//	LastTarget = NextFreeTarget;
	
	
	NextFreeTrig=NULL;
	LastTrig=NULL;
	
	for (i = 0; i < MAX_TRIGGERS; i++, trig++)  
	{
		trig->pausedby = NULL;
		trig->prev=NULL;
		trig->next = NextFreeTrig;
		NextFreeTrig = trig;
	}	
	
	NextFreeModelAnim=NULL;
	LastModelAnim=NULL;
	
	for (i = 0; i < MAX_MODELANIM; i++, ModelAnim++)  
	{
		ModelAnim->next = NextFreeModelAnim;
		ModelAnim->anim.lastFrame = 0;
		ModelAnim->anim.typeData = NULL;
		ModelAnim->anim.type = 0;
		NextFreeModelAnim = ModelAnim;
	}	
	
	
	
	NextFreeAnim=NULL;
	LastAnim=NULL;
	
	for (i = 0; i < MAX_STRATANIM; i++, anim++)  
	{
		anim->next = NextFreeAnim;
		anim->anim.flags = 0;
		//null the connection to model anims
		anim->anim.anim = NULL;
		NextFreeAnim = anim;
	}	
	
	NextFreeRoute=NULL;
	LastRoute=NULL;
	for (i = 0; i < MAX_STRATROUTES; i++, route++)  
	{
		route->camped = -1;
		route->splineData = NULL;
		route->next = NextFreeRoute;
		NextFreeRoute = route;
	}	
	
	FirstColl = NextFreeColl=NULL;
	for (i = 0; i < MAX_STRATCOLLS; i++, coll++)  
	{
		coll->strat = NULL;
		coll->next = NextFreeColl;
		NextFreeColl = coll;
	}	
	
	//ENSURE GLOBAL STRAT VARS ARE CLEAN
	OldPlayer = NULL;									 
	BossStrat = NULL;
	BoatStrat = NULL;
	ArrowStrat = NULL;
	WaterSlideCamLookStrat = NULL;
	WaterSlide = NULL;
	EscortTanker = NULL;
	//   	PresidentStrat = NULL;
	HoldPlayerMain = NULL;
	oldtStrat = NULL;
	hpbFirst = hpbLast = NULL;
	firstHDStrat = lastHDStrat = NULL;
	firstAvoidStrat = lastAvoidStrat = NULL;
	MissionFailure = 0;
	
#ifdef _DEBUG
	deletedstrats = 0;
#endif
	
	//ENSURE THE GLOBAL PARAMS ARE 0
	ClearGlobalParams();
	//	Dome=NULL;
}

int	LNOT(int value)
{
	//	return (((unsigned)(0xffffffff)) - value);
	return (4294967295 - value);
}

int	SLNOT(int value)
{
	//	return (((unsigned)(0xffffffff)) - value);
	return (65535 - value);
}



static void CountCollision()
{
	COLLSTRAT *coll=FirstColl;
   	STRAT *strat;
	int ENEMYCOUNT = 0;
	int FRIENDCOUNT = 0;
	int MAPENEMYCOUNT = 0;
	int MAPFRIENDCOUNT = 0;
	//	int SMAPENEMYCOUNT = 0;
	int totcoll=0;
	int x;
	//	CHECK_CHEAP(ObjHeap);
	
	//	while (coll)
	//	{
	//		totcoll ++;
	//		coll=coll->next;
	//	}
	strat = FirstStrat;
	while (strat)
	{
		if (strat->flag)
		{
			if (strat->flag & STRAT_ENEMY)
			{
				//if (strat->parent)
				//	if ((!MyParentInvalid(strat)) && strat->parent != player[0].Player)
				
				ENEMYCOUNT ++;
				//	else
				//	{
				//		if ((!MyParentInvalid(strat)) && strat->parent == player[0].Player)
				//		x = 2;
				//	}
			}
			else
				FRIENDCOUNT ++;
			
			if (strat->flag2 & STRAT2_MADEOFFMAP)
			{
				if (strat->flag & STRAT_ENEMY)
					MAPENEMYCOUNT ++;
				else 
					MAPFRIENDCOUNT ++;
			}
			
			//	if (strat->flag2 & STRAT2_SUSPENDED)
			//		SMAPENEMYCOUNT ++;
		}
		
		
		
		strat = strat->next;
	}
#if !GODMODE
#ifdef _DEBUG
	if (!suppressDebug) {
		tPrintf (20, 7, "%d ENEMIES", ENEMYCOUNT); 
		tPrintf (20, 8, "%d FRIENDS", FRIENDCOUNT); 
		tPrintf (20, 10, "%d MAPENEMIES", MAPENEMYCOUNT); 
		tPrintf (20, 11, "%d MAPFRIENDS", MAPFRIENDCOUNT); 
		tPrintf (20, 12, "%d DELETED", deletedstrats); 
		//	tPrintf (20, 12, "%d SMAPENEMIES", SMAPENEMYCOUNT); 
		//	tPrintf (20, 16, "%d STRATS", STRATNUM); 
#ifdef MEMORY_CHECK
		tPrintf (18, 12, "STARTNUMALLOC %d", STARTNUMFREEBLOCKS); 
		tPrintf (18, 14, "LASTNUMALLOC %d", MEMALLOCATLASTLIFE); 
#endif
	}
	
#endif
#endif
#if 0
	tPrintf (20, 8, "%d colls", totcoll); 
	tPrintf (20, 10, "%d STRATS", STRATNUM); 
	tPrintf (20, 12, "%d TRIGGERS", trigs); 
#endif
	
	
}


void StratProc()
{
	STRAT *strat,*temp, *next;
	int pn;
#ifdef _DEBUG
	int test;
#endif
	
	strat = FirstStrat;
	
	/*	dLog("strats\n"); */
	
#ifdef MEMORY_CHECK
	//	CheckCHeap (IntVarHeap);
	//	CheckCHeap (FloatVarHeap);
	//	CheckCHeap (ObjHeap);
#endif
	
	while (strat)
	{
		
		
		
#ifdef _DEBUG
		if ((strat > &StratPool[1024]) || (strat < StratPool))
			test = 1;
		if (strat->Player) {
			dAssert (strat->Player->Player == strat, "Unlinked player strat");
			dAssert (&player[strat->Player->n] == strat->Player, "Wrong numbered player");
			dAssert (strat->objId, "Lost objIds somehow");
		}
#endif
		next = strat->next;
		//if (strat->trigger)
		//	ProcTrigger(strat,strat->trigger);
		
		/*KILL THE STRAT OFF IF IT NEEDS IT, ELSE RUN ITS CODE BLOCK */
		
		dAssert (strat->pos.x == strat->pos.x, "QNAN");
		
		if (!strat->flag)
		{
			//	temp=strat->next;
			temp = next;
			RemoveStrat(strat);
			strat=temp;
			continue;
			/*if (!temp) */
			/*	return; */
			/*strat=temp; */
			/*strat->func(strat); */
		}
		else
		{
			
			if (!(strat->flag2 & STRAT2_PROCESSSTOPPED)) {
				dAssert (strat->func, "NULL strat->func");
				strat->func(strat);
			}	
			if (strat->SoundBlock)
				UpdateStratSound (strat);
			/*	ProcessStrat(strat); */
		}
		
		dAssert (strat->pos.x == strat->pos.x, "QNAN");
		
		if ((strat->flag) &&
			(strat->trigger))
			ProcTrigger(strat,strat->trigger);
		
		dAssert (strat->pos.x == strat->pos.x, "QNAN");
		
		/*		if (strat->trigger) */
		/*			ProcTrigger(strat,strat->trigger); */
		
		// strat=strat->next;
#ifdef MEMORY_CHECK
		//	CheckCHeap (IntVarHeap);
		//	CheckCHeap (FloatVarHeap);
		//	CheckCHeap (ObjHeap);
#endif
		
		strat=next;
	}
	
	strat = FirstStrat;
	
	for (pn=0; pn<4; pn++)
	{
		if (!player[pn].PlayerHeld)
			player[pn].PlayerOnHoldPlayer = NULL;
	}
	
	
	
	if (FirstStrat)
	{
		/*CHECK FOR TRIGGERS TO GO ON */
		TrigCheck(&player[currentPlayer].Player->pos);
		
		/*CHECK FOR EVENTS */
		
		EventCheck();
	}
	if ((!Multiplayer) && (player[0].CameraStrat) &&(player[0].CameraStrat->flag & STRAT_UNDER_WATER)) {
		rFlareR += 20.f/256.f;
		rFlareG += 39.f/256.f;
		rFlareB += 45.f/256.f;
	}
	
	while (strat)
	{
		if (strat->pnode)
		{
			if (strat->flag & (STRAT_COLLFLOOR | STRAT_COLLSTRAT))
				clearStratCollFlag(strat);
		}
		strat = strat->next;
	}
	
#ifdef _DEBUG
	CountCollision();
#endif
}

void StratProcPaused()
{
	STRAT *strat,*temp, *next;
	
	strat = FirstStrat;
	
	
	while (strat)
	{
		next = strat->next;
		if ((strat->flag) &&
			(strat->trigger))
			ProcTriggerPaused(strat,strat->trigger);
		
		
		strat=next;
	}
	
}


#define OUTSIDE 0
#define INSIDE 1
#define OUTSIDEEXTERNAL -1



Vector3 TESTVEC;

static void DeriveProbePoint(STRAT *strat,Vector2 *probe)
{
	Vector3 forward;
	forward.z = 0.0f;forward.x=0.0f;
	/*	forward.y = 1.5f * strat->pnode->radius * strat->relVel.y; */
	forward.y = 9.5f * strat->relVel.y;
	if (forward.y<0)
		if (forward.y>-1.0f)
			forward.y-=9.5f;
		else
			if (forward.y<1.0f)
				forward.y+=9.5f;
			
			
			
			if (strat->relVel.y >=0.0)
				forward.y = 9.5f;	
			else
				forward.y = -9.5f;
			
			mVector3Multiply(&TESTVEC,&forward,&strat->m);
			
			probe->x = strat->pos.x+TESTVEC.x;
			probe->y = strat->pos.y+TESTVEC.y;
			
}


float StratDistanceToPointXYNoRoot(Vector2 *Probe,Vector2 *pos)
{
	float x,y;
	
	x = ((Probe->x) - pos->x);	
	y = ((Probe->y) - pos->y);	
	
	/*	return ((float)sqrt((x*x)+(y*y))); */
	return ((float)((x*x)+(y*y)));
	
} 

static int InsideBox(Vector2 *Probe,BOXBOX *boxarea)
{
	/*	dLog("checking max - miny %f %f %f %f \n",boxarea->maxx,boxarea->minx,boxarea->maxy,boxarea->miny); */
	
	
	if ((Probe->x > boxarea->maxx) ||
		(Probe->x < boxarea->minx) ||
		(Probe->y > boxarea->maxy) ||
		(Probe->y < boxarea->miny))
		return(OUTSIDE);
	else
		return(INSIDE);
}


/*RETURNS INSIDE IF POINT IS INSIDE A SPLINE SHAPE */
/*OUTSIDE IF NOT */

static int InsideSpline(Vector2 *Probe, SPLINEBOX *splinearea)
{
	int   numpoints = splinearea->numpoints,i;
	Vector2 SegmentPointA,SegmentPointB;
	float	x,y;
	int CollideSegment=0;
	
	x=Probe->x;
	y=Probe->y;
	
	for (i=0;i<numpoints;i++)
	{
		SegmentPointA=splinearea->pointlist[i];
		
		/*CHECK FOR LAST SEGMENT */
		if (i==numpoints-1)
			SegmentPointB = splinearea->pointlist[0];
		else
			SegmentPointB = splinearea->pointlist[i+1];
		
		if ((((SegmentPointB.y <= y) && (y < SegmentPointA.y)) ||
			((SegmentPointA.y <= y) && (y < SegmentPointB.y))) &&
			(x < (SegmentPointA.x - SegmentPointB.x) * (y - SegmentPointB.y) /
			(SegmentPointA.y - SegmentPointB.y) + SegmentPointB.x))
			CollideSegment = !CollideSegment;	
		
	}
	
	/*AN EVEN AMOUNT OF COLLISIONS AND WE'RE OUTSIDE THE REGION */
	if (!CollideSegment)
		return(OUTSIDE);
	else
		return(INSIDE);
}



/*RETURNS INSIDE IF LINE INTERSECTS A GIVEN SPLINE REGION */

static int IntersectSpline(STRAT *strat, SPLINEBOX *splinearea)
{
	int   numpoints = splinearea->numpoints,i;
	Vector2 C,D,P,A,B;
	float	x,y,upper,lower,r,s;
	int CollideSegment=0;
	float strattoplay,strattopoint;
	
	A.x=strat->pos.x;
	A.y=strat->pos.y;
	
	B.x=player[currentPlayer].Player->pos.x;
	B.y=player[currentPlayer].Player->pos.y;
	
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
		
		
		x = strat->pos.x - player[currentPlayer].Player->pos.x;
		y = strat->pos.y - player[currentPlayer].Player->pos.y;
		strattoplay = (x*x)+(y*y);
		
		
		if (strattoplay < strattopoint)
			continue;
			/*	DRAWVEC.x =P.x;
		DRAWVEC.y =P.y;	*/
		return(INSIDE);
	}
	return (OUTSIDE);
}



/*SAME AS ABOVE BUT ARBITRARY POINT PASSED IN */
static int IntersectSplineGen(STRAT *strat, SPLINEBOX *splinearea,Vector2 *B)
{
	int   numpoints = splinearea->numpoints,i;
	Vector2 C,D,P,A;
	float	x,y,upper,lower,r,s;
	int CollideSegment=0;
	float strattoplay,strattopoint;
	
	A.x = strat->pos.x;
	A.y = strat->pos.y;
	
	
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
		lower = (B->x - A.x) * (D.y - C.y) -
			(B->y - A.y) * (D.x - C.x);
		
		r = upper/lower;
		
		if ((r<0) || (r>1.0))
			continue;
		
		
		upper = (A.y - C.y) * (B->x - A.x) -
			(A.x - C.x) * (B->y - A.y);
		
		s = upper/lower;
		
		if ((s<0) || (s>1.0))
			continue;
		
		
		
		P.x = A.x + r*(B->x-A.x);
		P.y = A.y + r*(B->y-A.y);
		
		x = A.x - P.x;
		y = A.y - P.y;
		strattopoint = (x*x)+(y*y);
		
		
		x = A.x - B->x;
		y = A.y - B->y;
		strattoplay = (x*x)+(y*y);
		
		
		/*	if (strattoplay < strattopoint)
		continue;
		*/
		return(INSIDE);
	}
   	return (OUTSIDE);
}


/*DOES THE LINE FROM A TO B INTERSECT THE GIVEN SPLINEAREA*/

static int LineIntersectSpline(SPLINEBOX *splinearea,Vector2 *START,Vector2 *B)
{
	int   numpoints = splinearea->numpoints,i;
	Vector2 C,D,P,A;
	float	x,y,upper,lower,r,s;
	int CollideSegment=0;
	float strattoplay,strattopoint;
	
	A.x = START->x;
	A.y = START->y;
	
	
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
		lower = (B->x - A.x) * (D.y - C.y) -
			(B->y - A.y) * (D.x - C.x);
		
		r = upper/lower;
		
		if ((r<0) || (r>1.0))
			continue;
		
		
		upper = (A.y - C.y) * (B->x - A.x) -
			(A.x - C.x) * (B->y - A.y);
		
		s = upper/lower;
		
		if ((s<0) || (s>1.0))
			continue;
		
		
		
		P.x = A.x + r*(B->x-A.x);
		P.y = A.y + r*(B->y-A.y);
		
		x = A.x - P.x;
		y = A.y - P.y;
		strattopoint = (x*x)+(y*y);
		
		
		x = A.x - B->x;
		y = A.y - B->y;
		strattoplay = (x*x)+(y*y);
		
		
		
		if (strattoplay < strattopoint)
			continue;
		
		return(INSIDE);
	}
   	return (OUTSIDE);
}

/* RETURNS 1 IF PLAYER IS OUTSIDE AREA*/
int PlayerOutSide(STRAT *strat)
{
	if (!strat->route)
		return(0);
	if (!(StratInRegions(strat,player[currentPlayer].Player->pos.x,player[currentPlayer].Player->pos.y)))
		return(1);
	else
		return(0);
}


int HitWall(STRAT *strat,int Update)
{
	Point3 rot,rot2,res,cross,pos;
	float ang;
	Matrix m;
	
	if ( (strat->wallN.x) ||
		(strat->wallN.y) ||
		(strat->wallN.z) )
	{
		v3Normalise(&strat->wallN);
		
		rot.x = -strat->m.m[1][0];
		
		rot.y = -strat->m.m[1][1];
		
		rot.z = -strat->m.m[1][2];
		
		rot.z = 0;
		
		
		cross.x = strat->wallN.x;
		
		cross.y = strat->wallN.y;
		
		cross.z	= 0;
		
		v3Cross(&res,&cross,&rot);
		
		ang = acos(v3Dot(&rot,&strat->wallN));
		
		if (ang > PI2/4.0f)
			return(0);
		
		/*	ang += 0.05f * (rand()&3);*/
		
		if (res.z > 0)
			mRotateZ(&m,-ang);
		else
			mRotateZ(&m,ang);
		
		rot.x = ((10.0f) * strat->wallN.x);
		rot.y = ((10.0f) * strat->wallN.y);
		rot.z = ((10.0f) * strat->wallN.z);
		rot.z = 0;
		
		mVector3Multiply (&rot2, &rot, &m);
		
		/*	v3Normalise(&rot2);				 */
		
		
		pos.x = strat->pos.x - strat->CheckPos.x;
		pos.y = strat->pos.y - strat->CheckPos.y;
		pos.z = strat->pos.z - strat->CheckPos.z;
		
		
		/*IF THE CHECKPOS IS FACING AWAY FROM THE WALL ALREADY THEN SKIP*/
		
		
		if (v3Dot(&pos,&strat->wallN) <0)
			/*		if (v3Dot(&pos,&rot2) <0)  */
			return(0);
		
		
		if (Update)
		{
			strat->CheckPos.x += rot2.x;
			strat->CheckPos.y += rot2.y;
			strat->CheckPos.z += rot2.z;
			
		}
		else
		{
			strat->CheckPos.x = strat->pos.x + rot2.x;
			strat->CheckPos.y = strat->pos.y + rot2.y;
			strat->CheckPos.z = strat->pos.z + rot2.z;
		}
		return(1);
	}
	else
		return(0);
	
	
}

#ifdef _DEBUG
static int VQNAN(Vector3 *v)
{
	if (v->x != v->x)
		return 1;
	if (v->y != v->y)
		return 1;
	if (v->z != v->z)
		return 1;
	
	return 0;
}
#endif


static int	stratAreaCollision(STRAT *strat, int SteerMode)
{
	short box;
	MAPAREA *Area;
	int subarea, i, dir;
	Point3	p1, p2, pr, sp;
	Vector3	v, vn, tempv, vx, force;
	Vector2	pr2;
	SPLINEBOX	*sb;
	float lambda;
	int coll,ocoll;
	
	if (!strat->route)
		return(0);
	
	box = strat->route->path->area;
	
	if (box < 0)
		return(0);
	
	coll = 0;
	force.x = 0;
	force.y = 0;
	force.z = 0;
	
	sp = strat->pos;
	sp.z = 0.0f;
	Area = &MapAreas[box];
	/*dAssert(Area->numsplineareas > 0, "Strat has no area\n");	*/
	
	for (subarea=0;subarea<Area->numsplineareas;subarea++)
	{
		sb = &Area->splinebbox[subarea];
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
		
#ifdef _DEBUG
		if (VQNAN(&vn))
			dAssert(0, "Oh dear, vector QNAN.");
#endif
		v3Scale(&vn, &vn, 0.1f);
		
#ifdef _DEBUG
		if (VQNAN(&vn))
			dAssert(0, "Oh dear, vector QNAN.");
#endif
		v3Inc(&pr, &vn);
		
		pr2.x = pr.x;
		pr2.y = pr.y;
		
		if (!InsideSpline(&pr2,&Area->splinebbox[subarea]))
			dir = 1;
		else
			dir = -1;
		
		
		for (i=0; i<sb->numpoints; i++)
		{		
			force.x = 0;
			force.y = 0;
			force.z = 0;
			
			sp = strat->pos;//changed
			sp.z = 0.0f;
			p1.x = sb->pointlist[i].x;
			p1.y = sb->pointlist[i].y;
			p1.z = 0.0f;
			
			v3Sub(&tempv, &sp, &p1);
			lambda = v3Length (&tempv);
			if (lambda > strat->pnode->radius)
				continue;
			
			v3Normalise(&tempv);
			v3Scale(&tempv, &tempv, strat->pnode->radius - lambda);
			coll++;
			//		if (!SteerMode)
			//		{
			if (Area->splinebbox[subarea].escape)
			{
				strat->route->CollisionType = 3;
				strat->route->Collide = Area->splinebbox[subarea].escape;
			}
			else
			{
				strat->route->CollisionType = 1;
				strat->route->Collide = &Area->splinebbox[subarea];
			}
			//	}
			
			v3Inc(&force, &tempv);  
			v3Inc(&strat->pos, &force);  //changed
#if 0
			if (v3Length(&force) > 3.0f)
				dAssert(0, "Hmmmmmmm, strange");
#endif
		}
		
		for (i=0; i<sb->numpoints; i++)
		{
			force.x = 0;
			force.y = 0;
			force.z = 0;
			
			sp = strat->pos;
			sp.z = 0.0f;
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
			
			/*v3Sub(&v, &sp, &p1);
			if (v3Length(&v) < strat->pnode->radius)
			{
			v3Inc(&strat->pos, &v);
			v3Inc(&sp, &v);
			strat->route->CollisionType = 1;
			strat->route->Collide = &Area->splinebbox[subarea];
		}*/
			
			v3Sub(&v, &p2, &p1);
			vn.x = v.y;
			vn.y = -v.x;
			vn.z = 0.0f;
			
			v3Normalise(&vn);
			
#ifdef _DEBUG
			if (VQNAN(&vn))
				dAssert(0, "Oh dear, vector QNAN.");
#endif
			lambda = pointLineDistanceFix(&sp, &p1, &p2);
			if (lambda > strat->pnode->radius)
				continue;
			
			//	if (!SteerMode)
			//	{
			if (Area->splinebbox[subarea].escape)
			{
				strat->route->CollisionType = 3;
				strat->route->Collide = Area->splinebbox[subarea].escape;
			}
			else
			{
				strat->route->CollisionType = 1;
				strat->route->Collide = &Area->splinebbox[subarea];
			}
			//	}
			
			v3Sub(&tempv, &sp, &p2);
			v3Cross(&vx, &v, &tempv);
			if (vx.z > 0.0f)
				lambda += strat->pnode->radius;
			else
				lambda = strat->pnode->radius - lambda;
			
			
			
			if (SteerMode != 3)
			{
				v3Scale(&vn, &vn, lambda);
				v3Inc(&force, &vn);
				v3Inc(&strat->pos, &force);
				if (SteerMode)
					v3Inc(&strat->CheckPos, &vn);
			}
			
			coll++;
			/*strat->vel .x = strat->vel.y = strat->vel.z = 0.0f;*/
		}
	}
	
	/*if ((coll) && (SteerMode !=3))
	{
	force.x  = force.x/coll;
	force.y  = force.y/coll;
	force.z  = force.z/coll;
	
	  v3Inc(&strat->pos, &force);
	  
	}*/
	
	ocoll = 0;
	force.x = 0;
	force.y = 0;
	force.z = 0;
	
	
	
	for (subarea=0;subarea<Area->numextsplineareas;subarea++)
	{
		force.x = 0;
		force.y = 0;
		force.z = 0;
		
		sb = &Area->extsplinebbox[subarea];
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
		
		if (InsideSpline(&pr2,&Area->extsplinebbox[subarea]))
			dir = 1;
		else
			dir = -1;
		
		
		for (i=0; i<sb->numpoints; i++)
		{
			
			force.x = 0;
			force.y = 0;
			force.z = 0;
			
			sp = strat->pos;//changed
			sp.z = 0.0f;
			p1.x = sb->pointlist[i].x;
			p1.y = sb->pointlist[i].y;
			p1.z = 0.0f;
			
			v3Sub(&tempv, &sp, &p1);
			lambda = v3Length (&tempv);
			if (lambda > strat->pnode->radius)
				continue;
			
			v3Normalise(&tempv);
			v3Scale(&tempv, &tempv, strat->pnode->radius - lambda);
			coll++;
			//	if (!SteerMode)
			//	{
			if (Area->extsplinebbox[subarea].escape)
			{
				strat->route->CollisionType = 3;
				strat->route->Collide = Area->extsplinebbox[subarea].escape;
			}
			else
			{
				strat->route->CollisionType = 2;
				strat->route->Collide = &Area->extsplinebbox[subarea];
			}
			//	}
			
			v3Inc(&force, &tempv);  
			v3Inc(&strat->pos, &force);  //changed
#if 0
			if (v3Length(&force) > 3.0f)
				dAssert(0, "Hmmmmmmm, strange");
#endif
		}
		
		for (i=0; i<sb->numpoints; i++)
		{
			sp = strat->pos;
			sp.z = 0.0f;
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
			
			/*v3Sub(&v, &sp, &p1);
			if (v3Length(&v) < strat->pnode->radius)
			{
			v3Inc(&strat->pos, &v);
			v3Inc(&sp, &v);
			strat->route->CollisionType = 2;
			strat->route->Collide = &Area->extsplinebbox[subarea];
		} */
			v3Sub(&v, &p2, &p1);
			vn.x = v.y;
			vn.y = -v.x;
			vn.z = 0.0f;
			
			v3Normalise(&vn);
			lambda = pointLineDistanceFix(&sp, &p1, &p2);
			if (lambda > strat->pnode->radius)
				continue;
			
			//if (!SteerMode)
			//{
			if (Area->extsplinebbox[subarea].escape)
			{
				strat->route->CollisionType = 3;
				strat->route->Collide = Area->extsplinebbox[subarea].escape;
			}
			else
			{
				strat->route->CollisionType = 2;
				strat->route->Collide = &Area->extsplinebbox[subarea];
			}
			//}
			
			v3Sub(&tempv, &sp, &p2);
			v3Cross(&vx, &v, &tempv);
			if (vx.z > 0.0f)
				lambda += strat->pnode->radius;
			else
				lambda = strat->pnode->radius - lambda;
			
			if (SteerMode != 3)
			{
				v3Scale(&vn, &vn, lambda); 
				v3Inc(&force, &vn);
				v3Inc(&strat->pos, &force);
				if (SteerMode)
					v3Inc(&strat->CheckPos, &vn);
			}
			ocoll++;
			/*strat->vel .x = strat->vel.y = strat->vel.z = 0.0f;*/
		}
	}
	
	/*if ((ocoll) && (SteerMode !=3))
	{
	force.x  = force.x/ocoll;
	force.y  = force.y/ocoll;
	force.z  = force.z/ocoll;
	
	  v3Inc(&strat->pos, &force);
	  
	}*/
	
	
	return(coll + ocoll);
}								       


static int	LastAreaCollision(STRAT *strat)
{
	int subarea, i, dir;
	Point3	p1, p2, pr, sp;
	Vector3	v, vn, tempv, vx,force;
	Vector2	pr2;
	SPLINEBOX	*sb;
	float lambda;
	int coll=0;
	
	sp = strat->pos;
	sp.z = 0.0f;
	switch (strat->route->CollisionType)
	{
		//INTERNAL COLLIDE
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
#ifdef _DEBUG
		if (VQNAN(&vn))
			dAssert(0, "Oh dear, vector QNAN.");
#endif
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
			
			force.x = 0;
			force.y = 0;
			force.z = 0;
			
			sp = strat->pos;//changed
			sp.z = 0.0f;
			p1.x = sb->pointlist[i].x;
			p1.y = sb->pointlist[i].y;
			p1.z = 0.0f;
			
			v3Sub(&tempv, &sp, &p1);
			lambda = v3Length (&tempv);
			if (lambda > strat->pnode->radius)
				continue;
			
			v3Normalise(&tempv);
			v3Scale(&tempv, &tempv, strat->pnode->radius - lambda);
			coll++;
			v3Inc(&force, &tempv);  
			v3Inc(&strat->pos, &force);  //changed
			v3Inc(&strat->CheckPos, &force);  //changed
#if 0
			if (v3Length(&force) > 3.0f)
				dAssert(0, "Hmmmmmmm, strange");
#endif
		}
		
		for (i=0; i<sb->numpoints; i++)
		{
			
			force.x = 0;
			force.y = 0;
			force.z = 0;
			
			sp = strat->pos;//changed
			sp.z = 0.0f;
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
#ifdef _DEBUG
			if (VQNAN(&vn))
				dAssert(0, "Oh dear, vector QNAN.");
#endif
			lambda = pointLineDistanceFix(&sp, &p1, &p2);
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
			v3Inc(&strat->pos, &force);  //changed
			v3Inc(&strat->CheckPos, &force);  //changed
#if 0
			if (v3Length(&force) > 3.0f)
				dAssert(0, "Hmmmmmmm, strange");
#endif
		}
		/*	if (coll)
		{
		
		  force.x = force.x / coll;
		  force.y = force.y / coll;
		  force.z = force.z / coll;
		  //v3Inc(&strat->pos, &force);  
		  v3Inc(&strat->CheckPos, &force);
		  
			
			}	*/
		
		
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
				
				force.x = 0;
				force.y = 0;
				force.z = 0;
				
				sp = strat->pos;//changed
				sp.z = 0.0f;
				p1.x = sb->pointlist[i].x;
				p1.y = sb->pointlist[i].y;
				p1.z = 0.0f;
				
				v3Sub(&tempv, &sp, &p1);
				lambda = v3Length (&tempv);
				if (lambda > strat->pnode->radius)
					continue;
				
				v3Normalise(&tempv);
				v3Scale(&tempv, &tempv, strat->pnode->radius - lambda);
				coll++;
				v3Inc(&force, &tempv);  
				v3Inc(&strat->pos, &force);  //changed
#if 0
				if (v3Length(&force) > 3.0f)
					dAssert(0, "Hmmmmmmm, strange");
#endif
			}
			
			for (i=0; i<sb->numpoints; i++)
			{
				force.x = 0;
				force.y = 0;
				force.z = 0;
				
				sp = strat->pos;//changed
				sp.z = 0.0f;
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
				lambda = pointLineDistanceFix(&sp, &p1, &p2);
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
				v3Inc(&strat->pos, &force);  //changed
				//v3Inc(&strat->CheckPos, &force);//changed
			}
			if (coll)
			{
				
				force.x = force.x / coll;
				force.y = force.y / coll;
				force.z = force.z / coll;
				//v3Inc(&strat->pos, &force);  
				v3Inc(&strat->CheckPos, &force);
				
				
			}
			
			break;
			
		default:
			return(0);
			
	}
	
	return(1);
}



/*STEERMODE 0 - JUST PUSH AWAY
1 - PUSH AWAY AND MODIFIY THE STEERING VECTOR
2 - PUSH AWAY, MODIFY THE STEERING VECTOR ON THE WALL YOU COLLIDED WITH
3 - DO NOT FORCE AWAY JUST CHECK FOR ILLEGAL AREA
4 - CHECK THE INTERSECTION OF THE STRAT'S CHECKPOS WITH VALID AREAS */

int AvoidSurround(STRAT *strat,int SteerMode)
{
	if (SteerMode == 2)
		return(LastAreaCollision(strat));
	else
		return(stratAreaCollision(strat,SteerMode));
	
}

int StratInRegions(STRAT *strat, float x, float y)
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
	{
		strat->route->CollisionType = 0;
		return (OUTSIDE);
   	}
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
				strat->route->CollisionType = 0;
				return (OUTSIDE);
			}
		}
		
		/*CHECK INTERSECTION WITH SPLINE REGIONS */
		for (subarea=0;subarea<Area->numsplineareas;subarea++)
			if (InsideSpline(&Probe,&Area->splinebbox[subarea]))
			{
				if (Area->splinebbox[subarea].escape)
					strat->route->CollisionType = 3;
				else
					strat->route->CollisionType = 1;
				//	strat->route->Collide = &Area->splinebbox[subarea];
				return (OUTSIDE);
			}
			
			for (subarea=0;subarea<Area->numextsplineareas;subarea++)
				if (!InsideSpline(&Probe,&Area->extsplinebbox[subarea]))
				{
					if (Area->extsplinebbox[subarea].escape)
						strat->route->CollisionType = 3;
					else
						strat->route->CollisionType = 2;
					//	strat->route->Collide = &Area->extsplinebbox[subarea];
					return (OUTSIDE);
				}
				
				return(INSIDE);
	}
}

int PointInRegions(STRAT *strat, Vector3 *point)
{
	short box;
	MAPAREA *Area;
	Vector2 Probe;
	int subarea;
	float radius,radius2;
	
	if (!strat->route)
		return(INSIDE);
	
	box = strat->route->path->area;
	
	Probe.x = point->x;
	Probe.y = point->y;		    
	
	/*IF NO DEFINED AREA FOR THIS STRAT THEN RETURN AS BEING INSIDE ALWAYS */
	if (box <0)
		return(INSIDE);
	
	Area = &MapAreas[box];
	radius=Area->Surround.radius;
	radius2=radius*radius;

	
	/*FIRST CHECK WE ARE INSIDE THE OUTERMOST CIRCLE */
	/*	if (!(StratDistanceToPointXYNoRoot(&Probe,&Area->Surround.pos) < radius2))
	{
	strat->route->CollisionType = 0;
	return (OUTSIDE);
   	}
	else
	{	*/
	/*CHECK INTERSECTION WITH SPLINE REGIONS */
	for (subarea=0;subarea<Area->numsplineareas;subarea++)
		if (InsideSpline(&Probe,&Area->splinebbox[subarea]))
		{
			if (Area->splinebbox[subarea].escape)
			{
				strat->route->CollisionType = 3;
				strat->route->Collide = Area->splinebbox[subarea].escape;
			}
			else
			{
				strat->route->CollisionType = 1;
				strat->route->Collide = &Area->splinebbox[subarea];
			}
			return (OUTSIDE);
		}
		
		for (subarea=0;subarea<Area->numextsplineareas;subarea++)
			if (!InsideSpline(&Probe,&Area->extsplinebbox[subarea]))
			{
				if (Area->extsplinebbox[subarea].escape)
				{
					strat->route->CollisionType = 3;
					strat->route->Collide = Area->extsplinebbox[subarea].escape;
				}
				else
				{
					strat->route->CollisionType = 2;
					strat->route->Collide = &Area->extsplinebbox[subarea];
				}
				return (OUTSIDE);
			}
			
			return(INSIDE);
			/*	} */
}

int PlayerAvoidSurround(STRAT *strat)
{
	return(!PointInRegions(strat,&player[currentPlayer].Player->pos));
	
	
}



/*DETECT WHETHER THE STRAT LIES INSIDE OR OUSIDE ITS BBOX REGIONS AS DEFINED IN THE EDITOR */

//int IntersectCircle(STRAT *strat,Vector2 *C,Vector2 *A,Vector2 *B,float distance);

//RETURNS OUTSIDE = 0, WHEN WE'VE GONE INTO INTERNAL BBOXES
//		  INSIDE = 1, WHEN WE'RE STILL INSIDE VALID REGIONS
//		  OUTSIDEEXTERNAL = -1, WHEN WE'VE GONE OUTSIDE THE EXTERNAL BBOX CIRCLE !!

int InsideArea(STRAT *strat)
{
	
	/*	short box = strat->path->area; */
	short box;
	MAPAREA *Area;
	Vector2 Probe;				
	int subarea;
	float radius,radius2;
	
	if (!strat->route)
		return (INSIDE);
	
	box = strat->route->path->area;
	
	
	
	
	/*IF NO DEFINED AREA FOR THIS STRAT THEN RETURN AS BEING INSIDE ALWAYS */
	if (box <0)
	{
		dLog("****** NO AREA LINKED WHEN CALLING INSIDE AREA ***\n");
		return(INSIDE);
	}
	Area = &MapAreas[box];
	
	
	radius=Area->Surround.radius;
	radius2=radius*radius;
	
	
	/*DERIVE A POINT TO USE FOR CHECKING */
	DeriveProbePoint(strat,&Probe);
	
	
	/*FIRST CHECK WE ARE INSIDE THE OUTERMOST CIRCLE */
	if (!(StratDistanceToPointXYNoRoot(&Probe,&Area->Surround.pos) < radius2))
	{
		//	strat->flag2 &= (0xffffffff - STRAT2_COLLIDECIRCLE);
		/*	strat->flag2 |= STRAT2_OUTERCIRCLE; */
		//no need to deflect		DeflectMe3(strat,&Area->Surround.pos,Area->Surround.radius);
		return (OUTSIDEEXTERNAL);
	}
	else
	{
		
#if 0
		//not using circulars or boxes
		/*CHECK INTERSECTION WITH CIRCULAR REGIONS */
		for (subarea=0;subarea<Area->numcircleareas;subarea++)
		{
			radius = Area->circlebbox[subarea].radius;
			radius2=radius*radius;
			/*if distance less than circle's radius we are outside the valid area */
			if (StratDistanceToPointXYNoRoot(&Probe,&Area->circlebbox[subarea].pos) < radius2)
				/*	if (IntersectCircle(strat,&Area->circlebbox[subarea].pos,&tpos,&Probe,radius2) ) */
			{
				//	strat->flag2 &= (0xffffffff - STRAT2_OUTERCIRCLE);
				//strat->flag2 |= STRAT2_COLLIDECIRCLE;
				//no need to deflect	DeflectMe(strat,&Area->circlebbox[subarea].pos,Area->circlebbox[subarea].radius);
				return (OUTSIDE);
			}
		}
		
		
		/*CHECK INTERSECTION WITH BOX REGIONS */
		for (subarea=0;subarea<Area->numboxareas;subarea++)
		{
			if (InsideBox(&Probe,&Area->boxbbox[subarea]))
			{
				//	strat->flag2 &= (0xffffffff - (STRAT2_OUTERCIRCLE+STRAT2_COLLIDECIRCLE));
				//	strat->flag2 &= (0xffffffff - (STRAT2_OUTERCIRCLE));
				//no need to deflect DeflectMeBox(strat,&Area->boxbbox[subarea]);
				return (OUTSIDE);
			}
		}
#endif
		
		/*CHECK INTERSECTION WITH SPLINE REGIONS */
		for (subarea=0;subarea<Area->numsplineareas;subarea++)
		{
			//	if (IntersectSplineGen(strat,&Area->splinebbox[subarea],&Probe))
			if (InsideSpline(&Probe,&Area->splinebbox[subarea]))
				/*if (InsideSpline(&Probe,&Area->splinebbox[subarea])) */
			{
				//no need to deflect	DeflectMeSpline(&Probe,strat,&Area->splinebbox[subarea]);
				return (OUTSIDE);
			}
		}
		
		
		
		
		return(INSIDE);
		
		
	}
	
	
}

static int LineSightCircle(STRAT *strat,Vector2 *C,Vector2 *B,float distance)
{
	Vector3 *A = &strat->pos;
	/**B = &Player->pos;*/
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
		return(INSIDE);
	
	
	x = C->x - P.x;
	y = C->y - P.y;
	
	real = (float)((x*x) + (y*y));
	
	if (real < distance)
		return(OUTSIDE);
	
	else
		return(INSIDE);
}

//Vector2 DRAWVEC;

#define VISIBLE 1
#define NONVISIBLE 0


/*RETURNS 0 IF INTERSECTING WITH A PLACED SIGHT AREA*/

int LineSightPos(STRAT *strat,Vector3 *startpos,Vector3 *endpos)
{
	Vector2 start,end;
	short box;
	MAPAREA *Area;
	int subarea;
	float radius,radius2;
	
	if (!strat->route)
		return(VISIBLE);
	
	box = strat->route->path->area;
	
	
	start.x = startpos->x;
	start.y = startpos->y;
	
	end.x = endpos->x;
	end.y = endpos->y;
	
	/*IF NO DEFINED AREA FOR THIS STRAT THEN RETURN AS BEING INSIDE ALWAYS */
	if (box <0)
	{
		/*TESTLINE = 1; */
		return(VISIBLE);
	}
	
	Area = &MapAreas[box];
	
#if 0
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
#endif
	
   	for (subarea=0;subarea<Area->numlinesightareas;subarea++)
	{
		if (LineIntersectSpline(&Area->linesightbbox[subarea],&start,&end))
			return (NONVISIBLE);
	} 
	
   	/*TESTLINE = 1; */
	return(VISIBLE);
}

int FireSightPlayer(STRAT *strat)
{
	return(LineSightPos(strat,&strat->pos,&player[currentPlayer].Player->pos));
}

int FireSightTarget(STRAT *strat)
{
	return(LineSightPos(strat,&strat->pos,&strat->target->pos));
}



/*RETURNS 0 IF INTERSECTING WITH A PLACED SIGHT AREA*/

int PositionVisible(STRAT *strat,Vector3 *startpos,Vector3 *endpos)
{
	Vector2 start,end;
	short box = strat->route->path->area;
	MAPAREA *Area;
	int subarea;
	float radius,radius2;
	
	
	start.x = startpos->x;
	start.y = startpos->y;
	
	end.x = endpos->x;
	end.y = endpos->y;
	
	/*IF NO DEFINED AREA FOR THIS STRAT THEN RETURN AS BEING INSIDE ALWAYS */
	if (box <0)
		return(VISIBLE);
	Area = &MapAreas[box];
	
	/*CHECK INTERSECTION WITH CIRCULAR REGIONS */
   	for (subarea=0;subarea<Area->numcircleareas;subarea++)
	{
		radius = Area->circlebbox[subarea].radius;
		radius2=radius*radius;
		
		if (!LineSightCircle(strat,&Area->circlebbox[subarea].pos,&start,radius2))
		{
			
			/*put something down at the centre point of the circle */
			
			//	DRAWVEC = Area->circlebbox[subarea].pos;
			/*TESTLINE = 0; */
			return (NONVISIBLE);
		}	
	} 		 
	
	
   	
	for (subarea=0;subarea<Area->numlinesightareas;subarea++)
	{
		if (LineIntersectSpline(&Area->linesightbbox[subarea],&start,&end))
			return (NONVISIBLE);
	} 
	for (subarea=0;subarea<Area->numextsplineareas;subarea++)
	{
		if (LineIntersectSpline(&Area->extsplinebbox[subarea],&start,&end))
			return (OUTSIDE);
	}
	
	for (subarea=0;subarea<Area->numsplineareas;subarea++)
	{
		if (LineIntersectSpline(&Area->splinebbox[subarea],&start,&end))
			return (OUTSIDE);
	}
	
	return(VISIBLE);
}


int LineSightPlayer(STRAT *strat)
{
	Vector2 p;
	/*	short box = strat->path->area; */
	short box;
	MAPAREA *Area;
	int subarea;
	float radius,radius2;
	
	
	if (!strat->route)
		return(VISIBLE);
	
	box = strat->route->path->area;
	/*IF NO DEFINED AREA FOR THIS STRAT THEN RETURN AS BEING INSIDE ALWAYS */
	if (box <0)
	{
		/*TESTLINE = 1; */
		return(VISIBLE);
	}
	
	Area = &MapAreas[box];
	
	p.x = player[currentPlayer].Player->pos.x;
	p.y = player[currentPlayer].Player->pos.y;
	
	
	/*CHECK INTERSECTION WITH CIRCULAR REGIONS */
   	for (subarea=0;subarea<Area->numcircleareas;subarea++)
	{
		radius = Area->circlebbox[subarea].radius;
		radius2=radius*radius;
		
		if (!LineSightCircle(strat,&Area->circlebbox[subarea].pos,&p,radius2))
		{
			
			/*put something down at the centre point of the circle */
			
			//			DRAWVEC = Area->circlebbox[subarea].pos;
			/*TESTLINE = 0; */
			return (NONVISIBLE);
		}	
	} 		 
	
	p.x = player[currentPlayer].Player->pos.x;
	p.y = player[currentPlayer].Player->pos.y;
	
	
   	for (subarea=0;subarea<Area->numlinesightareas;subarea++)
	{
		if (IntersectSplineGen(strat,&Area->linesightbbox[subarea],&p))
			return (NONVISIBLE);
	} 
	
	
	
	/*THESE TWO RECENTLY ADDED*/
   	for (subarea=0;subarea<Area->numsplineareas;subarea++)
	{
		if (IntersectSplineGen(strat,&Area->splinebbox[subarea],&p))
			return (NONVISIBLE);
	} 
	
   	for (subarea=0;subarea<Area->numextsplineareas;subarea++)
	{
		if (IntersectSplineGen(strat,&Area->extsplinebbox[subarea],&p))
			return (NONVISIBLE);
	} 
	
	return(VISIBLE);
}

int SaveTarget(STRAT *strat)
{
	return (int)(strat->target);
	
}

int GetPlayerStrat(STRAT *strat, int playnum)
{
	return (int)(player[playnum].Player);
}


void RestoreTarget(STRAT *strat,int target)
{
	strat->target = (STRAT*)target;
	
}

int AnyFreeTargets(STRAT *strat)
{	int found=0;
int targetval=0;


GAMETARGETS *FOUNDTARGET;
GAMETARGETS *target = FirstTarget;
//	return(found);
/*
while (target)
{
if (target->occupied)
{
target->occupied--;

  strat->target = target->strat;
  found++;
  break;
		}	
		
		  
			target = target->next;
			
			  }
			  
*/

while (target)
{
	if (target->occupied > targetval)
	{
		FOUNDTARGET = target;
		targetval = target->occupied;
		found++;
		
	}
	
	   target=target->next;
}


if (found)
{
	strat->target = FOUNDTARGET->strat;
	FOUNDTARGET->occupied--;
	
}
else
strat->target = NULL;


return(found);


}

GAMETARGETS *FindTarget(STRAT *strat)
{
	GAMETARGETS *target;
	
	target = FirstTarget;
	
	while (target)
	{
		if (target->strat == strat)
			break;
		
		target = target->next;
	}
	
	
	return(target);
}

static int FindTarget2(STRAT *strat)
{
	GAMETARGETS *target;
	
	target = FirstTarget;
	
	while (target)
	{
		if (target->strat == strat)
			return(1);
		
		target = target->next;
	}
	
	
	return(0);
	//	return(target);
}



int TargetStillValid(STRAT *strat)
{
	return(FindTarget2(strat->target));
	
}



static void UnRegisterAsTarget(STRAT *strat)
{
	
	GAMETARGETS *target;
	
	//FIRST FIND THE TARGET HOLDING THIS STRAT	
	
	target = FindTarget(strat);
	
	
	if (target)
	{
		if (target->next)
			target->next->prev = target->prev;
		if (target->prev)
			target->prev->next = target->next;
		else
			FirstTarget = target->next;		
		
		
		target->strat = NULL;
		target->occupied = 255;
		target->prev = NULL;
		
		target->next = NextFreeTarget;
		NextFreeTarget = target;
	}
}


void RegisterAsTarget(STRAT *strat,int MaxOccupants)
{	GAMETARGETS *Target;



Target = NextFreeTarget;

if (Target)
{
	NextFreeTarget = NextFreeTarget->next;
	
	Target->next = FirstTarget;
	
	
	Target->strat = strat;
	
	strat->flag2 |= STRAT2_STRATTARGET;
	
	Target->occupied = MaxOccupants;
	
	if (FirstTarget)
		FirstTarget->prev = Target;
	
	FirstTarget = Target;
}

}


void RegisterCollision(STRAT *strat)
{
	COLLSTRAT *coll;
	//ONLY REGISTER IF STRAT HAS NO COLLBLOCK
	if (strat->CollBlock)
		return;
#ifdef _DEBUG	
	if (!(strat->pnode))
		return;
#endif   
	coll = NextFreeColl;
	
	if (coll)
	{
		
		NextFreeColl = NextFreeColl->next;
		
		coll->next = FirstColl;					
		if (FirstColl)
			FirstColl->prev = coll;
		FirstColl = coll;
		
		strat->CollBlock = coll;
		coll->strat = strat;
	}
	
	
}


/*	GIVEN A STRAT TO CREATE, THIS CHECKS WHETHER OR NOT IT IS ATTACHED
TO AN EVENT, AND, IF SO, WHETHER THAT EVENT HAS TRIGGERED OR NOT */

static int	EventTriggered(int StratEntry)
{
	int event;
	
	event = GameMap[StratEntry].eventdepend;
	if (event >= 0)
	{
		if (!MapEvents[event].triggered)
			return(0);
			/*	else
			if (MapEvents[event].timer)
		return(0);		 */
		
	}
	return(1);
}


static void SetMyParent(STRAT *strat)
{
	//	strat->flag |= STRAT_PARENT;
	strat->parent = GameMap[(int)strat->parent].strat;
	/*KILL OFF THE TRIGGER*/
	EndTrigger(strat);
}




/*CREATES AN INSTANCE OF A STRAT FROM MAP ENTRY i */
static STRAT *MapStratGen(int i,int mode)
{
	STRAT *strat;
	short event;
	int loop;
	
	if (GameMap[i].invalid)
		return(NULL);
				
	strat=AddStrat(GameMap[i].StratEnt);
	dAssert(strat,"OUT OF STRAT OR VAR SPACE\n");
	
	
	if (GameMap[i].way >= 0)
	{
		if (GetNextRoute(strat))
		{
			strat->route->pathnum=GameMap[i].way;
			strat->route->pathstartnode=GameMap[i].startnode;
			strat->route->patrolnumber=GameMap[i].startpatrol;
			//FIX TO ENSURE STRAT->ROUTE->PATH IS SET UP CORRECTLY
			SetGlobalParamInt(0,99);
			InitPath(strat);
			ClearGlobalParams();
			
			
		}
	}
	/*	else */
	/*		strat->pathnum=-1; */
				
	
	mRotateXYZ (&strat->m,GameMap[i].rot.x,GameMap[i].rot.y,GameMap[i].rot.z);
	strat->scale = GameMap[i].scale;
	
	
	strat->pos.x = GameMap[i].pos.x;
	strat->pos.y = GameMap[i].pos.y;
	strat->pos.z = GameMap[i].pos.z;
	
   	if (GameMap[i].modelid != -1)
		SetModel(strat, GameMap[i].modelid);
	/*dLog("%s rot = %f, %f, %f\n", strat->pnode->name, GameMap[i].rot.x, GameMap[i].rot.y, GameMap[i].rot.z); */
	
	
	strat->oldOldPos = strat->oldPos = strat->pos;
	
	
	
	strat->flag = GameMap[i].flag | STRAT_ALIVE;
	strat->flag2 = GameMap[i].flag2;
	strat->flag2 |= STRAT2_MADEOFFMAP;
	
	for (loop=0;loop<5;loop++) 
		/*	if (GameMap[i].activation[loop] >= 0) */
		/*record the entry number for later use */
		strat->actindex[loop] = GameMap[i].activation[loop];
	
	/*IF THE STRAT NEEDS PARAMETERS THEN SET THEM UP */
	
	if (GameMap[i].intparams)
	{
		for (loop=0;loop<GameMap[i].intparams;loop++)
			strat->IVar[loop] = GameMap[i].iparamlist[loop];
		strat->flag2 |= STRAT2_IPARAMSET;
	}
	
	if (GameMap[i].floatparams)
	{
		for (loop=0;loop<GameMap[i].floatparams;loop++)
			strat->FVar[loop] = GameMap[i].fparamlist[loop];
		strat->flag |= STRAT_FPARAMSET;
	}
	/*SET THE ACTIVATION DELAY */
	strat->frame = GameMap[i].delay;
	
	event = GameMap[i].attachedevent;
	if (event >= 0)
	{
		/*RESERVE THE POINTER TO THE STRAT */
		GameMap[i].strat = strat;
		if (mode==WARM)
		{
			MapEvents[event].eventstrat[MapEvents[event].count]= strat;
			MapEvents[event].eventstatus[MapEvents[event].count]= ALIVE;
			MapEvents[event].count++;
		}
	}
	/*RESERVE THE POINTER TO THE STRAT */
	GameMap[i].strat = strat;
	GameMap[i].origstrat = strat;
	
	/*reserve pointer back to map, within the strat*/
	//strat->MapEntry = i;
	
	
	/*HAS THE THING GOT A PARENT*/
	if (GameMap[i].parent >= 0)
	{
		
		/*IF IT HAS THEN GREAT*/
		if (GameMap[GameMap[i].parent].strat)
		{
			strat->parent = GameMap[GameMap[i].parent].strat;
			//		strat->flag |= STRAT_PARENT;
			
		}
		else
		{
			/*ELSE SET UP A TRIGGER TO FIRE WHEN THE PARENT IS CREATED*/
			strat->parent = (STRAT*)GameMap[i].parent;
			CreateTrigger(strat,&SetMyParent,WHENPARENTCREATED);
			
		}
	}
	else
		/*ELSE NO REQUIRED PARENT*/
		strat->parent = NULL;
	
	
	// 	strat->func(strat);
	return(strat);
	
	
}

/*ENSURES THAT A GIVEN PARENT ID IS A TRUE STRAT ADDRESS*/

int MyParentInvalid(STRAT *strat)
{
	unsigned int	id;
	
	//NO PARENT ATTACHED THEN RETURN VALID
	if (!strat->parent)
		return(0);
	
	id = (int)strat->parent;
	
	if (id < 1024)
		return (1);
	else
		return(0);
	
}

int MyParentParentInvalid(STRAT *strat)
{
	unsigned int	id;
	
	//NO PARENT ATTACHED THEN RETURN VALID
	if (!strat->parent)
		return(0);
	
	if (!strat->parent->parent)
		return(0);
	
	id = (int)strat->parent->parent;
	
	if (id < 1024)
		return (1);
	else
		return(0);
	
}



/*PLAYER NEAR ACTIVATION */
void SetActivationPoint(STRAT *strat)
{
	/*	int index = (int)strat->lastdist; */
	/*	dLog("Getting index %d\n",index); */
	/*	strat->CheckPos = MapActs[index].pos; */
	/*	strat->lastdist = MapActs[index].radius; */
	
	
}
void GetActivationPos(STRAT *strat, int actNo)
{
	if (strat->actindex[actNo] == -1)
		return;
	
	strat->CheckPos = MapActs[strat->actindex[actNo]].pos;
	
	
	
}

float GetActivationRadius(STRAT *strat, int actNo)
{
	if (strat->actindex[actNo] == -1)
		return 0.0f;
	
	return MapActs[strat->actindex[actNo]].radius;
}


int PlayerNearActivation(STRAT *strat,int num)
{
	
	/*	dLog("****RADIUS %f\n",strat->lastdist); */
	if (!(player[currentPlayer].Player))
		return (0);
	
	
	if (strat->actindex[num] < 0)
		return 0;
	
	if (WayToPlayerDist(&MapActs[strat->actindex[num]].pos) <= MapActs[strat->actindex[num]].radius)
		/*	if (WayToPlayerDist(&strat->CheckPos) <= strat->lastdist) */
		return (1);
	
	return(0);
}

int NearActivation(STRAT *strat,int num)
{
	
	/*	dLog("****RADIUS %f\n",strat->lastdist); */
	
	
	if (WayToStratDist(&MapActs[strat->actindex[num]].pos,strat) <= MapActs[strat->actindex[num]].radius)
		/*	if (WayToPlayerDist(&strat->CheckPos) <= strat->lastdist) */
		return (1);
	
	return(0);
}


int NearActivationXY(STRAT *strat,int num)
{
	
	/*	dLog("****RADIUS %f\n",strat->lastdist); */
	
	
	if (WayToStratDistXY(&MapActs[strat->actindex[num]].pos,strat) <= MapActs[strat->actindex[num]].radius)
		/*	if (WayToPlayerDist(&strat->CheckPos) <= strat->lastdist) */
		return (1);
	
	return(0);
}





int PlayerNearActivationXY(STRAT *strat,int num)
{
	
	/*	dLog("****RADIUS %f\n",strat->lastdist); */
	if (!(player[currentPlayer].Player))
		return (0);
	
	if (strat->actindex[num] >= 0)
		if (WayToPlayerDistXY(&MapActs[strat->actindex[num]].pos) <= MapActs[strat->actindex[num]].radius)
			return (1);
		
		return(0);
}

int ParentNearActivationXY(STRAT *strat,int num)
{
	
	/*	dLog("****RADIUS %f\n",strat->lastdist); */
	
	if (strat->actindex[num] >= 0)
		if (WayToStratDistXY(&MapActs[strat->actindex[num]].pos,strat->parent) <= MapActs[strat->actindex[num]].radius)
			return (1);
		
		return(0);
}


int MPPlayerNearActivationXY(STRAT *strat,int num)
{
	int i;
	/*	dLog("****RADIUS %f\n",strat->lastdist); */
	
	
	for (i=0;i<NumberOfPlayers;i++)
	{
		if (player[i].Player && MPWayToPlayerDistXY(&MapActs[strat->actindex[num]].pos,i) <= MapActs[strat->actindex[num]].radius)
			return (1);
	}
	
	return(0);
}

int MPPlayerNearActivation(STRAT *strat,int num)
{
	int i;
	/*	dLog("****RADIUS %f\n",strat->lastdist); */
	
	
	for (i=0;i<NumberOfPlayers;i++)
	{
		if (player[i].Player && MPWayToPlayerDist(&MapActs[strat->actindex[num]].pos,i) <= MapActs[strat->actindex[num]].radius)
			return (1);
	}
	
	return(0);
}










/*PLAYER TO TRIG RADIUS AND THEN CREATE WAREZ */

static int NearTrig(MAPTRIGGER *trig, Point3 *pos)
{
	float real;
	float x, y;
	float radius = trig->radius * trig->radius;
	
	
	//	x = (float)(player[currentPlayer].Player->pos.x - trig->pos.x);
	//	y = (float)(player[currentPlayer].Player->pos.y - trig->pos.y);
	x = pos->x - trig->pos.x;
	y = pos->y - trig->pos.y;
	
	
	/*	real =(float)sqrt((x*x)+(y*y)); */
	
	real =(float)((x*x)+(y*y));
	
	/*	if (real < (Player->pnode->radius+trig->radius)) */
	/*	if (real < (trig->radius)) */
	if (real < (radius))
		return 1;
	else
		return 0;
	
	
}

/*	CHECKS THE TRIGGERS ON THE MAP AND CREATES ANY REQUIRED STRATS FROM THE MAP
VERY BASIC AT THE MOMENT */

static void TrigCheck(Point3 *pos)
{
	int i,loop, triggered, creatednow,createdprev;
	STRAT *strat;
	
	for (i=0;i<NumTrigs;i++)
	{
	/*	DEACTIVATED FLAG SETS THE TOP SIGN BIT ON THE FLAG
		SHOULD EQUATE TO A STRAIGHT LOAD..BRANCH LESS ZERO WAREZ */
		
		if (MapTrigs[i].flag < 0)
			continue;
		
		if (!(MapTrigs[i].triggered))
		{
			if (NearTrig(&MapTrigs[i],pos))
			{
				triggered = 1;
				
				if (MapTrigs[i].flag & TIMERTRIG)
					MapTrigs[i].currentcount = 0;
				else
				{
					for (loop=0;loop<MapTrigs[i].nument;loop++)
					{
					/*	CHECK THAT THE STRAT IN QUESTION IS NOT ATTACHED TO AN 
						UNTRIGGERED EVENT */
						
						if (EventTriggered(MapTrigs[i].entries[loop]))
						{
							
						/*ENSURE THAT THE STRATS HAVE NOT ALREADY BEEN TRIGGERED 
						THIS CATERS FOR TRIGGERS THAT HOLD STRATS DEPENDENT ON EVENTS
							OCCURRING, AS WELL AS INSTANT CREATION NON EVENT-DEPENDENT STRATS */
							
							if (!MapTrigs[i].gen[loop])
							{
								//AM I STILL VALID TO MAKE
								if (!(GameMap[MapTrigs[i].entries[loop]].invalid))
								{
									strat = MapStratGen(MapTrigs[i].entries[loop],WARM);
									if (strat)
									{
										//	strat->func(strat);
										
										//IF THIS IS A RUNONCE STRAT THAT HAS ALREADY DONE ITS THING THEN KILL IT
										if (GameMap[MapTrigs[i].entries[loop]].flag & STRAT_RUNONCE)
											strat->func = &QuickKill;
										else
										{
											strat->func(strat);
											if (strat->flag & STRAT_RUNONCE)
												GameMap[MapTrigs[i].entries[loop]].flag |= STRAT_RUNONCE;
										}
										
										MapTrigs[i].gen[loop]=1;
									}
								}
							}
						}
						else
							triggered = 0;
					}
				}
				MapTrigs[i].triggered = triggered;
			}
		}
		else
		{
			if (MapTrigs[i].flag & TIMERTRIG)
			{
				if (MapTrigs[i].currentcount == MapTrigs[i].counter)
				{
					
				/*	CREATE ANY NON-EVENT DEPENDENTS
					KEEP A COUNT OF HOW MANY WE CREATE */ 
					createdprev = 0;
					creatednow = 0;
					for (loop=0;loop<MapTrigs[i].nument;loop++)
					{
						if (EventTriggered(MapTrigs[i].entries[loop]))
						{
							
						/*ENSURE THAT THE STRATS HAVE NOT ALREADY BEEN TRIGGERED 
						THIS CATERS FOR TRIGGERS THAT HOLD STRATS DEPENDENT ON EVENTS
							OCCURRING, AS WELL AS INSTANT CREATION NON EVENT-DEPENDENT STRATS */
							
							if (!MapTrigs[i].gen[loop])
							{
								strat = MapStratGen(MapTrigs[i].entries[loop],WARM);
								if (strat)
								{
									//strat->func(strat);
									//IF THIS IS A RUNONCE STRAT THAT HAS ALREADY DONE ITS THING THEN KILL IT
									if (GameMap[MapTrigs[i].entries[loop]].flag & STRAT_RUNONCE)
										strat->func = &QuickKill;
									else
									{	
										strat->func(strat);
										if (strat->flag & STRAT_RUNONCE)
											GameMap[MapTrigs[i].entries[loop]].flag |= STRAT_RUNONCE;
									}
									MapTrigs[i].gen[loop]=1;
									creatednow ++;
								}
							}
							else
								createdprev++;
						}
					}
					
					/*DEACTIVATE IF ALL THE ATTACH STRATS HAVE BEEN TRIGGERED */
					
					if ((creatednow == MapTrigs[i].nument) || (createdprev == MapTrigs[i].nument))
						MapTrigs[i].flag |= DEACTIVATED;
					else
						/* SEND IT AROUND AGAIN*/
						MapTrigs[i].triggered = 0;
					
						/*	//for (loop=0;loop<MapTrigs[i].nument;loop++)
						//	MapStratGen(MapTrigs[i].entries[loop],WARM);
						
					//MapTrigs[i].flag |= DEACTIVATED;			   */
				}
				else
					MapTrigs[i].currentcount++;
				
			}
			else
				MapTrigs[i].flag |= DEACTIVATED;
			
		}
	}
	
	
}

int HasActivation(STRAT *strat, int act)
{
	int i;
	
	//ensure spawn strats are ignored
	if (!(strat->flag2 & STRAT2_MADEOFFMAP))
		return(0);
	
	if (act == 123)
	{
		for (i=0;i<5;i++)
		{
			if (strat->actindex[i] >= 0)
				return(1);
			
		}
	}
	else if ((act >= 0) && (act <= 4))
	{
		if (strat->actindex[act] >=0)
			return(1);
	}
	else
	{
		dAssert(0, "invalid entry");
	}
	
	return(0);
}

//TERMINOLOGY :- CREATION EVENT - ON THE DEATH OF STRAT OTHERS ARE CREATED
//			   - DELETION EVENT - ON THE DEATH OF STRAT OTHERS ARE DELETED

static void RecurseDelete(STRAT *strat)
{
	
	int i;
	
	
	
	//CHECK ENTRY IN WATER STRAT AND LAVA STRAT LIST
	//REMOVING IF NEEDED
	for (i=0;i<noWaterStrats;i++)
	{
		if (waterStrat[i] == strat)
			waterStrat[i] = NULL;
	}
	for (i=0;i<noLavaStrats;i++)
	{
		if (lavaStrat[i] == strat)
			lavaStrat[i] = NULL;
	}
#ifdef _DEBUG
	deletedstrats++;
#endif
	
	Delete(strat);
	
}


//DELETES ANY VALID SPAWNED STRATS
static void RecurseDeleteChildren(STRAT *strat, STRAT *BaseStrat, STRAT *OldBaseStrat)
{
	STRAT *child;
	child = FirstStrat;
	
	
	while (child)
	{
		OldBaseStrat = BaseStrat;
		if ((child->parent == BaseStrat) && (!child->Player) && (!(child->flag2 & STRAT2_MADEOFFMAP)))
		{
			
			Delete(child);
			
#ifdef _DEBUG
			deletedstrats++;
#endif
			BaseStrat = child;
			RecurseDeleteChildren(child,BaseStrat,OldBaseStrat);
		}
		BaseStrat = OldBaseStrat;
		child = child->next;
	}
}




static void EnvokeDeletionEvent(int event, int child)
{
	
	short stratnum,connectedcreations,i;
	
	stratnum = MapEvents[event].childlist[child];
	
	/*DELETE THE EVENT'S CHILD IF IT IS STILL ACTIVE, AND ANY ACTIVE CHILDREN*/
	if (GameMap[stratnum].strat)
	{
		//delete me and my children
		RecurseDelete(GameMap[stratnum].strat);
		RecurseDeleteChildren(GameMap[stratnum].origstrat,GameMap[stratnum].origstrat,GameMap[stratnum].origstrat);
		GameMap[stratnum].strat->flag2 &= LNOT(STRAT2_MADEOFFMAP + STRAT2_SUSPENDED);
		
	}
	else
   	{
		//HAS IT EVEN EVER BEEN CREATED ?
		//IF SO, ALLOW IT TO BE PROPERLY FREE'D UP, AND DELETE ANY CHILDREN
		if 	(GameMap[stratnum].origstrat)
		{
			//delete my children
			RecurseDeleteChildren(GameMap[stratnum].origstrat,GameMap[stratnum].origstrat,GameMap[stratnum].origstrat);
			GameMap[stratnum].origstrat->flag2 &= LNOT(STRAT2_MADEOFFMAP + STRAT2_SUSPENDED);
		}
		//ELSE IT WASN'T EVEN TRIGGERED
		//SO NOTHING TO DELETE
#ifdef _DEBUG
		else	   
		{
			
			deletedstrats++;
		}
#endif
	}
	
	//	else
	/*FLAG IT AS BEING INVALID TO MAKE*/
	GameMap[stratnum].invalid = 1;
	
	connectedcreations = GameMap[stratnum].attachedevent;
	
	/*OK, LET'S SEE WHAT CREATION EVENT IS CONNECTED TO IT..IF ANY*/
	
	if (connectedcreations >=0)
	{
		//was the strat alive ?
		if (!(GameMap[stratnum].strat))
		{
			for (i=0;i<MapEvents[connectedcreations].nument;i++)
			{
				//ensure the map entries hung off this event are invalid
				GameMap[MapEvents[connectedcreations].entries[i]].invalid = 1;
			}
		}
		else
		{
			for (i=0;i<MapEvents[connectedcreations].nument;i++)
			{
				//ensure the event registers for this strat, may need to be recursive
				if (MapEvents[connectedcreations].eventstrat[i] == GameMap[stratnum].strat)
				{
					MapEvents[connectedcreations].eventstatus[i] = ALIVE;
					MapEvents[connectedcreations].eventstrat[i] = &DummStrat;
					MapEvents[connectedcreations].eventstrat[i]->flag2 = STRAT2_PROCESSSTOPPED;
					DummStrat.flag = 0;
				}
			}
		}
		
		
	}
	
	
}

static void EventCheck()
{
	int i,loop,iloop;
	STRAT *strat;
	
	
	for (i=0;i<NumEvents;i++)
	{
		//HAS THE TRIGGER ALREADY FIRED OFF ?
		if (!MapEvents[i].triggered)
		{
			//NOPE, THEN LET' CHECK ITS ATTACHMENTS
			for (loop=0;loop<MapEvents[i].nument;loop++)
			{
				//IS ATTACHMENT 'LOOP' STILL VALID TO CHECK ?
				if (MapEvents[i].eventstatus[loop] == ALIVE)
				{
					//YEP, SO HAS THE STRAT ATTACHED TO THE TRIGGER BEEN DELETED ?
					if (!MapEvents[i].eventstrat[loop]->flag)
					{
						
						
						//YES, THEN FLAG ATTACHMENT AS DEAD
						MapEvents[i].eventstatus[loop]=DEAD;
						MapEvents[i].deleted++;
						
						
						//HAVE THE REQUIRED AMOUNT OF STRATS ATTACHED TO THE EVENT BEEN DELETED ?
						if (MapEvents[i].deleted == MapEvents[i].nument)
						{
							
							//YEP, THEN FIRE OFF THIS EVENT
							if (!MapEvents[i].timer)
								
								MapEvents[i].triggered = 2;
							else
								MapEvents[i].triggered = 1;
							
							// 	for (iloop=0;iloop<MapEvents[i].numchild;iloop++)
							//		if (!GameMap[MapEvents[i].childlist[iloop]].trig)
							//		MapStratGen(MapEvents[i].childlist[iloop],WARM);
							
						}
					}								
				}
			}
			
		}
		else
		{
			//EVENT MAY BE FIRED OFF IF IT'S A NON TIMER CHAPPY
			if (MapEvents[i].triggered == 2)
			{
				
				
				/*CREATE ALL STRATS ATTACHED TO THIS TRIGGER, THAT DON'T HAVE CREATION TRIGGERS*/
				for (iloop=0;iloop<MapEvents[i].numchild;iloop++)
				{
					/*NOW SEE IF IT'S A DELETION OR CREATION EVENT*/
					if (!MapEvents[i].flag)
					{
						/*CREATION EVENT*/
						if (!GameMap[MapEvents[i].childlist[iloop]].trig)
						{
							strat = MapStratGen(MapEvents[i].childlist[iloop],WARM);
							if (strat)
							{
								//	strat->func(strat);
								//IF THIS IS A RUNONCE STRAT THAT HAS ALREADY DONE ITS THING THEN KILL IT
								if (GameMap[MapEvents[i].childlist[iloop]].flag & STRAT_RUNONCE)
									strat->func = &QuickKill;
								else
								{
									strat->func(strat);
									if (strat->flag & STRAT_RUNONCE)
										GameMap[MapEvents[i].childlist[iloop]].flag |= STRAT_RUNONCE;
								}
							}
						}
					}
					else
					{
						/*DELETE STRAT IF IT HAS BEEN CREATED, IF IT HASN'T WE NEED TO INVALIDATE IT WAREZ*/
						EnvokeDeletionEvent(i,iloop);
						
						//	if (GameMap[MapEvents[i].childlist[iloop]].strat)
						//		Delete(GameMap[MapEvents[i].childlist[iloop]].strat);
						
					}
					
				}
				MapEvents[i].triggered = 3;
			}
			else
			{
				MapEvents[i].timer --;
				if (!MapEvents[i].timer)
					MapEvents[i].triggered ++;
				
			}
			
			
		}
		
		
		
	}
	
	
}

int	CamNo;

static void LoadHeads(Stream *fp)
{
	int		i,nBytes;
	short entry,numheads;
	
	//	fread(&numheads,sizeof(short),1,fp);
	
	for (i=0;i<MAXCHARACTERS;i++)
	{
#if 0
		fread(&entry,sizeof(short),1,fp);
		(void)PNodeLoad (&GameHeads[entry], fp, (Allocator *)SHeapAlloc, MissionHeap);
#else
		sRead(&nBytes, 4, fp);
		if (nBytes)
			GameHeads[i] = PNodeLoad (NULL, fp, (Allocator *)SHeapAlloc, MissionHeap);
		sSkip (fp);
		
#endif
		
	}
	
	
}

static void LoadSoundBank(Stream *fp)
{
	int		i,nBytes;
	int		KATSIZE;
	int   address,entry,numsounds;
	
	
	//clear the indirection table
   	for (i=0;i<MAXSOUNDS;i++)
		SFXIndirTab[i] = -1;
	
	
	sRead(&nBytes, 4, fp);
	if (nBytes)
	{
		//read the number of sounds
		fread(&numsounds,sizeof(int),1,fp);


		for (i=0;i<numsounds;i++)
		{
			
			//read the sound entry
			fread(&entry,sizeof(int),1,fp);
			
			//THIS CHECK SHOULD GO INTO STREAM REALLY
			if (entry > MAXSOUNDS)
				CrashOut("OVER SOUND LIMIT", 31, 31);

			//read the address map
			fread(&address,sizeof(int),1,fp);
			
			//resolve the link
			SFXIndirTab[entry] = address;// - 128;
			
			
		}
		
		//READ THE SIZE OF THE KAT FOR AUDIO TO READ
		fread(&KATSIZE,sizeof(int),1,fp);
		sSkip (fp);
		
		sRead(&nBytes, 4, fp);
		if (nBytes)
		{
			//kat file data is here
			SoundLoadBank(fp,KATSIZE);
		}
		sSkip (fp);
		
	}
}



static void LoadScript(Stream *fp)
{	short entry;
short numchars;
short numlines;
short numents,numscenes,scene,size,dummy,dummy2;
int loop,inner,loop2;
char *charpt;
int charloop;
float tempf;

//1 load all scene info in
//number of scenes
fread(&numscenes,sizeof(short),1,fp);

//2 load script entry numbers in
fread(&numents,sizeof(short),1,fp);

for (loop=0;loop<numscenes;loop++)
{
	//read the passage number
	fread(&scene,sizeof(short),1,fp);
	
	//read the number of script entries used in this passage
	fread(&size,sizeof(short),1,fp);
	
	GameScenes[scene] = 
		(short*)SHeapAlloc (MissionHeap, (sizeof(short) * size));
	
	for (loop2=0;loop2<size;loop2++)
	{
		fread(&dummy2,sizeof(short),1,fp);
		GameScenes[scene][loop2] = dummy2;
	}	
}


for (loop=0;loop<numents;loop++)
{
	
	fread(&entry,sizeof(short),1,fp);
	fread(&GameScript[entry].frametime,sizeof(short),1,fp);
	
	if (PAL)
	{
		tempf = (float)GameScript[entry].frametime;
		tempf = tempf / 1.2;
		GameScript[entry].frametime = (short)tempf;
	}
	
	fread(&GameScript[entry].numlines,sizeof(short),1,fp);
	
	fread(&GameScript[entry].flag,sizeof(unsigned char),1,fp);
	
	//read the associated character field (-1 if none)
	fread(&GameScript[entry].character,sizeof(signed char),1,fp);
	
	GameScript[entry].script = (char**)SHeapAlloc (MissionHeap,sizeof(char*) * GameScript[entry].numlines);
	
	for (inner=0;inner<GameScript[entry].numlines;inner++)
	{
		fread(&numchars,sizeof(short),1,fp);
		GameScript[entry].script[inner] = (char*)SHeapAlloc (MissionHeap, (sizeof(char) * numchars) + 1);
		
		//TACK THE NUMBER OF CHARS IN THE STRING LINE ONTO THE FRONT OF THE STRING
		//THIS WILL SAVE THE NEED TO USE STRLEN WITHIN THE TEXT PROCESSING
		*GameScript[entry].script[inner] = (unsigned char)numchars;
		
		fread(GameScript[entry].script[inner]+1,sizeof(char),numchars,fp);
		
		
		
	}
	
	
	//	GameScript[entry].script = 	(char *)SHeapAlloc (MissionHeap,sizeof(char) * numchars);
	
	
	//	fread(GameScript[entry].script,sizeof(char),numchars,fp);
	
}


}

static void CleanScript()
{  int i;
for (i=0;i<MAX_SCRIPTS;i++)
GameScript[i].script = NULL;

for (i=0;i<MAX_SCENES;i++)
GameScenes[i] = NULL;

}

int GetScriptEntry(int entry,int val)
{
	if (!GameScenes[entry])
		return(0);
	
	return(GameScenes[entry][val]);
	
	
}

int GetScriptTime(int entry)
{
	//	if (!GameScript[entry].script)
	//		return(0);

	return (GameScript[entry].frametime);
	
	
}

int GetScriptCharacter(int entry)
{
	if (!GameScript[entry].script)
		return(-1);
	
	
	return (GameScript[entry].character);
	
	
}


int GetScriptLines(int entry)
{
	
	return (GameScript[entry].numlines);
	
	
}

int GetScriptFlag(int entry)
{
	
	return	((int)GameScript[entry].flag);
}

int GetScriptLine(int entry, int line, char **textPtrPtr)
{
	if (!GameScript[entry].script)
		return 0;
	*textPtrPtr = GameScript[entry].script[line]+1;
	return *GameScript[entry].script[line];
}


/*returns 1 if newline has been hit*/
int DisplayScript(int entry, float numchars, int line,int x, int y, int mode)
{
	int i;
	unsigned char nChar;
	char buf[1024];
	
	if (!GameScript[entry].script)
		return;
	
	//mode = 1;
	if (mode)
		line = GameScript[entry].numlines;
	
	mUnit32 (psGetMatrix());
	
	// XXX: Fixme - ignores x and y for now
	x = 4;
	y = 4;
	
	strcpy (buf, GameScript[entry].script[0]+1);
	buf[GameScript[entry].script[0][0]-1] = '\0';
	
	ScriptPrint(x, y, buf, 1);
	return 0;
}


/*MATT ADD */
/*WAD LOADER HAS MAP,MODELS AND ANIM DATA IN ONE BIG BINARY FILE */



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
	short escape;
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
	
#ifdef MEMORY_CHECK
  		STARTNUMFREEBLOCKS = ObjHeap->nAlloced;
#endif
		
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
			GameMap[i].origstrat = NULL;
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
			LoadPoll();
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
				if (PAL)
				{
					tempf =  (float)MapTrigs->counter;
					tempf = tempf / 1.2f;
					MapTrigs->counter = (short)tempf;
					
					
				}
				
				
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
			LoadPoll();
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
			
			if ((PAL) && (MapEvents->timer))
			{
				tempf =  (float)MapEvents->timer;
				tempf = tempf / 1.2f;
				MapEvents->timer = (short)tempf;
			}
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
			LoadPoll();
		}	
		MapEvents = StartEvent;
		
		/*NOW READ IN THE ACTIVATION POINTS */
		fread(&NumActs,sizeof(short),1,fp);
		dLog("map contains %d strat activation points \n",NumActs);
		if (NumActs)
		{
			MapActs =  (MAPACTIVATION *)SHeapAlloc (MissionHeap, sizeof (MAPACTIVATION) * NumActs);
			fread(MapActs,sizeof(MAPACTIVATION),NumActs,fp);
			LoadPoll();
		}
		
		/*NOW READ IN THE STRAT PATHS */
		
		fread(&NumPaths,sizeof(short),1,fp);
		dLog("map contains %d paths \n",NumPaths);
		if (NumPaths)
			MapPaths =  (WAYPATH *)SHeapAlloc (MissionHeap, sizeof (WAYPATH) * NumPaths);
		
		for (i=0;i<NumPaths;i++)
		{
			
			fread(&(MapPaths+i)->waytype,sizeof(short),1,fp);
			if (((MapPaths+i)->waytype == PATH) || ((MapPaths+i)->waytype == ENTRYSPLINE))
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
			LoadPoll();
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
					(MapAreas + i)->extsplinebbox[entry].escape = NULL;
					fread(&escape,sizeof(short),1,fp);
					if (!escape)
						(MapAreas + i)->extsplinebbox[entry].escape = NULL;
					else
					{
						(MapAreas+i)->extsplinebbox[entry].escape =  (SPLINEBOX *)SHeapAlloc (MissionHeap, sizeof (SPLINEBOX));
						fread(&(MapAreas+i)->extsplinebbox[entry].escape->numpoints,sizeof(short),1,fp);
						(MapAreas+i)->extsplinebbox[entry].escape->pointlist =  (Vector2 *)SHeapAlloc (MissionHeap, sizeof (Vector2) * (MapAreas+i)->extsplinebbox[entry].escape->numpoints);
						fread((MapAreas+i)->extsplinebbox[entry].escape->pointlist,sizeof(Vector2),(MapAreas+i)->extsplinebbox[entry].escape->numpoints,fp);
						
					}	
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
					(MapAreas+i)->splinebbox[entry].pointlist =  (Vector2 *)SHeapAlloc (MissionHeap, sizeof (Vector2) * (MapAreas+i)->splinebbox[entry].numpoints);
					fread((MapAreas+i)->splinebbox[entry].pointlist,sizeof(Vector2),(MapAreas+i)->splinebbox[entry].numpoints,fp);
					fread(&escape,sizeof(short),1,fp);
					if (!escape)
						(MapAreas + i)->splinebbox[entry].escape = NULL;
					else
					{
						(MapAreas+i)->splinebbox[entry].escape =  (SPLINEBOX *)SHeapAlloc (MissionHeap, sizeof (SPLINEBOX));
						fread(&(MapAreas+i)->splinebbox[entry].escape->numpoints,sizeof(short),1,fp);
						(MapAreas+i)->splinebbox[entry].escape->pointlist =  (Vector2 *)SHeapAlloc (MissionHeap, sizeof (Vector2) * (MapAreas+i)->splinebbox[entry].escape->numpoints);
						fread((MapAreas+i)->splinebbox[entry].escape->pointlist,sizeof(Vector2),(MapAreas+i)->splinebbox[entry].escape->numpoints,fp);
						
					}	
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
			
			LoadPoll();
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
		 LoadPoll();
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
		 
		 
		 LoadPoll();
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
		 LoadPoll();
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
		 LoadPoll();
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
		 LoadPoll();
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
			 dAssert(InputBuffer,"OUT OF MISSION HEAP SPACE FOR DEMO\n");
			 
			 if (InputBuffer)
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
		 LoadPoll();
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
		 LoadPoll();
	 }
	 
	 
	 /*DO WE HAVE A SCRIPT TO LOAD ?*/
	 sRead(&nBytes, 4, fp);
	 if (nBytes)
	 {
		 
		 
		 LoadScript (fp);
		 LoadPoll();
		 
		 
	 }
	 sSkip (fp);
	 
	 
	 /*DO WE HAVE CHARACTER MODELS TO LOAD ?*/
	 LoadHeads(fp);
	 
	 /*DO WE HAVE SOUND BANK TO LOAD ?*/
	 LoadSoundBank(fp);
	 
#if !SH4
	 /*Ensure the wad is kept open on set 5*/
	 fclose(fp);
#endif
	 
	 
	 
	 
	 /*ALL MODELS AND STUFF LOADED SO LET'S GEN THE MAP STRATS */
	 for (i=0;i<NumStrats;i++)
	 {
		 
		 
		 
		 /*ONLY CREATE IF IT'S A NON TRIGGERED STRAT */
		 /*THAT'S NOT DEPENDENT ON AN EVENT HAPPENING */
		 if ((!GameMap[i].trig) && (GameMap[i].eventdepend < 0))
		 { 
			 strat = MapStratGen(i,WARM);	
			 
			 //need to see why this do not work
			 //does not work cos linit gets called after
			 //		strat->func(strat);
			 //	if (strat->flag & STRAT_RUNONCE)
			 //		GameMap[i].flag |= STRAT_RUNONCE;
			 
			 //RUNONCE CHECK NEEDED ON STRATFLAG AND AN UPDATE
			 //OF AN ENTRY IN GAMEMAPS, IF SUBSEQUENT CREATIONS FIND THIS
			 //ENTRY SET, THEN AN IMMEDIATE DELETION IS NEEDED
		 }
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
	 
	 //was
	 /*NOW SET THE DUMMY PLAYER */
	 player[currentPlayer].Player = &DummStrat;
	 StratReset(player[currentPlayer].Player);
	 //ensure the player variables are reset
	 resetPlayerVariables(0);
	 //   	resetCamera(0);
	 
	 
	 
#ifdef MEMORY_CHECK
	 PlayerLives = 8;
#else
	 if (LevelNum != 13) // Dont set this up on credits level; it will currently be the number of lives left after Alien baseness
		 PlayerLives = GetLevLives(LevelNum);
#endif
	 
	 GameOverFlag=LevelComplete = 0;
	 CamNo = 0;
	 player[currentPlayer].CameraStrat = NULL;
	 
	 for (i=0;i<MAXCAM;i++)
		 CamTracks[i]=NULL;
	 
	 PlayerLifeLost = 0;
	 Restart = 0;
	 
	 
	 
}


extern int TimerDirection, TimerTime, TimerShow;

int PlayerLifeLost;

/*RUNS WHEN PLAYER DIES AND LEVEL RESETS*/
void	ResetMap()
{	
	
	int i,loop;
	STRAT *strat;
	STRAT *temp;
	
	
#ifdef _DEBUG
	int tempcount = 0;
	int amount = STRATNUM;
#endif
	
	NoMainGun = 0;
	PlayerLifeLost = 1;
	
	/*clear out our targets*/
	clearTargetArray(0);
	clearTargetArray(1);
	clearTargetArray(2);
	clearTargetArray(3);
	
	/*FIRST DELETE THE STRATS*/	
	
	strat = FirstStrat;
	
	while (strat)
	{
		
		
		temp = strat->next;
		DeleteMapStrat(strat);
		strat=temp;
#ifdef _DEBUG
		tempcount ++;
#endif
#ifdef MEMORY_CHECK
		//	CheckCHeap (IntVarHeap);
		//	CheckCHeap (FloatVarHeap);
		CheckCHeap (ObjHeap);
#endif
		
	}
	
	
#ifdef _DEBUG
	if ((trigs) || (tempcount != amount))
		temp = strat;
	dAssert (!firstHDStrat, "FIRST HD SHOULD BE NULL");
	dAssert (!lastHDStrat, "LAST HD SHOULD BE NULL");
	dAssert (!hpbFirst, "FIRST HPB SHOULD BE NULL");
	dAssert (!hpbLast, "LAST HPB SHOULD BE NULL");
	
	
	
#endif
	
	
	StratInit();
	
#ifdef MEMORY_CHECK
	CheckCHeapIsFree(ObjHeap);
	CheckCHeapIsFree(SoundHeap);
	dAssert (!ALLOCSPLINES, "SPLINE MEM NOT FREED CORRECTLY");
	dAssert (!ALLOCSPLINESCHKSUM, "SPLINE MEM NOT FREED CORRECTLY");
	dAssert (!HPBALLOC, "HPB MEM NOT FREED CORRECTLY");
	dAssert (!HPBALLOCCHKSUM, "HPB MEM NOT FREED CORRECTLY");
	dAssert (!HDCALLOC, "HDC MEM NOT FREED CORRECTLY");
	dAssert (!HDCALLOCCHKSUM, "HDC MEM NOT FREED CORRECTLY");
	dAssert (!VARALLOC, "VAR MEM NOT FREED CORRECTLY");
	dAssert (!VARALLOCCHKSUM, "VAR MEM NOT FREED CORRECTLY");
	dAssert (!TOTALALLOC, "OBJ MEM NOT FREED CORRECTLY");
	dAssert (!TOTALALLOCCHKSUM, "OBJ MEM NOT FREED CORRECTLY");
	
	temp = (STRAT*)CHeapAlloc(ObjHeap,sizeof(STRAT));
	CHeapFree(ObjHeap,temp);
	MEMALLOCATLASTLIFE = ObjHeap->nAlloced;
#endif
	
	/*NOW CLEAR THE EVENT*/
	for (i=0;i<NumEvents;i++)
	{
		MapEvents[i].count=0;
		MapEvents[i].deleted =0;
		MapEvents[i].triggered =0;
		MapEvents[i].timer = MapEvents[i].starttimer;
		MapEvents[i].nument = MapEvents[i].startnument;
		
		
		for (loop=0;loop<MapEvents[i].nument;loop++)
		{
			MapEvents[i].eventstrat[loop] = NULL;
			MapEvents[i].eventstatus[loop] = WAITING;
		}
	}
	
	
	for (i=0;i<NumStrats;i++)
	{
		GameMap[i].strat = NULL;	
		GameMap[i].origstrat = NULL;	
		GameMap[i].invalid = 0;
	}
	
	/*CLEAR THE TIMER FLAG*/
	TimerShow = 0;
	
	//CLEAR BOSS
	BossStrat = NULL;
	
	
	player[0].Player = &DummStrat;
   	resetPlayerVariables(0);
	
	
	LevelReset = 0;
	player[currentPlayer].CameraStrat=NULL;
	ResetRandom();
	
	// Reset up fog et cetera
	SetLevelVars(LevelSettings);
	
	/*NOW DE-TRIGGER ALL OUR CREATION POINTS */
	for (i=0;i<NumTrigs;i++)
	{
		MapTrigs[i].flag &= SLNOT(DEACTIVATED);	
		MapTrigs[i].currentcount = 0;
		MapTrigs[i].triggered=0;
		/*CLEAR THE 'HAVE CREATED' FLAG FOR EACH STRAT ATTACHED TO THE TRIGGER*/
		for (loop=0;loop<MapTrigs[i].nument;loop++)
			MapTrigs[i].gen[loop]=0;
	}
	
	/*NOW CREATE ALL OUR STRATS*/
	
	for (i=0;i<NumStrats;i++)
	{
		/*ONLY CREATE IF IT'S A NON TRIGGERED STRAT */
		/*AND NON-DEPENDENT ON EVENTS OCCURRING*/
		//	if (!GameMap[i].trig)
		if ((!GameMap[i].trig) && (GameMap[i].eventdepend < 0))
		{
			strat = MapStratGen(i,WARM); 
			
			//IF THIS IS A RUNONCE STRAT THAT HAS ALREADY DONE ITS THING THEN KILL IT
			if (GameMap[i].flag & STRAT_RUNONCE)
				strat->func = &QuickKill;
			
			
		}
	}
	
	
	
	
	
#if 0
	/*CLEAR THE TIMER FLAG*/
	TimerShow = 0;
	
	//CLEAR BOSS
	BossStrat = NULL;
	
	
	player[0].Player = &DummStrat;
   	resetPlayerVariables(0);
	//	resetCamera(0);
	
	LevelReset = 0;
	
	player[currentPlayer].CameraStrat=NULL;
	ResetRandom();
	
	// Reset up fog et cetera
	SetLevelVars(LevelSettings);
#endif
}
void UpdateGameMapEntry(STRAT *strat)
{
	int i;
	for (i=0;i<NumStrats;i++)
	{
		if (GameMap[i].strat == strat)
			GameMap[i].flag |= STRAT_RUNONCE;
	}
}
//RUN WHEN GAMEOVER
void	MapClean() 
{
	int i;
	STRAT *strat,*temp;
	
	/*FIRST DELETE THE STRATS	 */
	
	strat = FirstStrat;
	
	while (strat)
	{
		temp = strat->next;
		/*		RemoveStrat(strat); */
		DeleteMapStrat(strat);
		
		strat=temp;
	}
	
	firstHDStrat = NULL;
	lastHDStrat = NULL;
	
	/*NOW DELETE THE PNODES */
	dLog("cleaning %d\n",NumMod);
	for (i=0;i<NumMod;i++)
	{
		PNode *pnode = &GameObjects[i];
		if (pnode->anim)
			dLog("cleaning %s with anim %s \n",pnode->name,pnode->anim->anim[0].name);
		(void)PNodeFree (&GameObjects[i], (Deallocator*)SHeapClear, MissionHeap);
	}
	
	/*	GameAnims=NULL; */
	
	CleanScript();
	
	/*NULLIFY OUR UNWANTED POINTERS AND CLEAR OUR COUNTERS */
	
	NumMod=0;
	NumTrigs=0;
	STRATNUM=0;
	
#ifdef MEMORY_CHECK
	TOTALALLOC=0;
#endif
	player[currentPlayer].CameraStrat=NULL;
}

void MakeFrags (STRAT *strat, float radius, int amount)
{
	dAssert (strat != NULL, "NULL strat passed to MakeFrags");
	rCreateFragments (&strat->pos, radius, amount);
}
float GetConeAmount (STRAT *strat)
{
	float x2, z2, cosAngle;
	/* Now to suss out the angle subtended */
	x2 = SQR(strat->m.m[0][0]) + SQR(strat->m.m[1][0]) + SQR(strat->m.m[2][0]) * SQR(0.5f);
	z2 = SQR(strat->m.m[0][2]) + SQR(strat->m.m[1][2]) + SQR(strat->m.m[2][2]);
	cosAngle = sqrt(z2) / sqrt(z2 + x2);
	
	return cosAngle;
}
void MakeConeFrags (STRAT *strat, float cosAngle, float velocityMax, int amount)
{
	Point3 pos;
	
	/* Get a unit vector in the Z direction */
	pos.x = strat->m.m[0][2];
	pos.y = strat->m.m[1][2];
	pos.z = strat->m.m[2][2];
	v3Normalise (&pos);
	
	rCreateConeFrags (&strat->pos, &pos, cosAngle, velocityMax, amount);
}


void MakeTrailFrags (STRAT *strat, float radius, int amount, float x, float y, float z)
{
	Point3 pos;
	dAssert (strat != NULL, "NULL strat passed to MakeFrags");
	pos.x = x; pos.y = y; pos.z = z;
	mPoint3Apply (&pos, &strat->m);
	v3Inc (&pos, &strat->pos);
	rCreateTrailFragments (&pos, radius, amount);
}

void MakeSplash (STRAT *strat, float radius, int amount)
{
	dAssert (strat != NULL, "NULL strat passed to MakeSplash");
	rCreateSplash (&strat->pos, radius, amount);
}

void MakeExpFrags (STRAT *strat, float radius, int amount)
{
	dAssert (strat != NULL, "NULL strat passed to MakeExpFrags");
	rCreateExpFragments (&strat->pos, radius, amount);
}


short GameOverFlag=0, LevelComplete = 0;

void GameOver(void)
{
	GameOverFlag++;	
}

void CompletedLevel(void)
{
	GameOverFlag++;	
	if (!MissionFailure)
		LevelComplete++;
}
short LevelReset=0;


/* POST SH4 */


void   LogCam(STRAT *strat)
{
	
	if (CamNo > MAXCAM)
		return;
	
	CamTracks[CamNo] = strat;
	CamNo ++;
	
}




/* CHANGES THE CAMERA TO A SPECIFIED INDEXED CAM*/
//MODE 1 WILL HOLD THE CAMERA IN LIEU OF A NEW ONE KICKING IN
//MODE -1 WILL NOT CHANGE CAMERA, BUT WILL ENSURE THAT EXISTING CAM IS NOT WIDESCREEN
void ChangeCam(int number,int mode)
{
	/* 0 will return to the game cam */
	Point3 tempp;


	if (mode < 0)
	{
		//ENSURE THE CAMTRACK IS NOT SET TO WIDESCREEN
	   	if (player[currentPlayer].CameraStrat->parent)
		   	player[currentPlayer].CameraStrat->parent->IVar[0] = 0;
		   	player[currentPlayer].cam->flags |= 2;
		   //	player[currentPlayer].cam->screenY = PHYS_SCR_Y;
		   	
		   //	wideAmt = wideAmt * 0.98;
		 
		   //	player[currentPlayer].zooming = 0;
		   

	   //	}
		return;

	}

	
	if (number)
	{
		if (number > CamNo)
			return;
		
		number--;
		
		player[currentPlayer].CameraStrat->parent = CamTracks[number];
		
		player[currentPlayer].CameraStrat->parent->flag |= STRAT_HIDDEN;
		
		if (player[currentPlayer].CameraStrat->parent->route)
			InitPath(player[currentPlayer].CameraStrat->parent);
		
			/*	CameraStrat->parent->func(CameraStrat->parent);
		*/
	}
	else
	{
		wideAmt = 0;
		player[currentPlayer].cam->flags = 0;
		player[currentPlayer].zooming = 0;
		player[currentPlayer].cam->screenY = PHYS_SCR_Y;
		
		tempp = player[0].Player->CheckPos;
		MyStratToWorld(player[0].Player, 0.0, -5.0f, 3.0f);
		player[currentPlayer].CameraStrat->pos = player[0].Player->CheckPos;
		player[currentPlayer].CameraStrat->oldPos = player[0].Player->CheckPos;
		player[currentPlayer].CameraStrat->oldOldPos = player[0].Player->CheckPos;
		player[currentPlayer].cam->pos = player[0].Player->CheckPos;
		ResetOCP(player[currentPlayer].CameraStrat);
		player[currentPlayer].CameraStrat->parent = NULL;
		MyStratToWorld(player[0].Player, 0.0, 3.0f, 1.0f);
		player[currentPlayer].camLookCrntPos = player[0].Player->CheckPos;
		player[0].Player->CheckPos = tempp;
		if (mode)
		{
			player[currentPlayer].cameraPos = CAMERA_HELD;
			player[currentPlayer].cam->HoldTime = 2;
		}
		else
			player[currentPlayer].cameraPos = PlayerCameraState[currentPlayer];
		
	}
}



/*CHANGES THE CAMERA TO A LINKED STRAT CAM*/

void ChangeCamStrat(STRAT *strat)
{
	/* 0 will return to the game cam */
	
	if (strat->parent)
	{
		
	   //	wideAmt = 1;
		player[currentPlayer].cameraPos = THIRD_PERSON_CLOSE;
		
		player[currentPlayer].CameraStrat->parent = strat->parent;
		
		player[currentPlayer].CameraStrat->parent->flag |= STRAT_HIDDEN;
		
		//	player[currentPlayer].CameraStrat->flag &= LNOT(STRAT_COLLFLOOR);
		
		if (player[currentPlayer].CameraStrat->parent->route)
			InitPath(player[currentPlayer].CameraStrat->parent);
		
			/*	CameraStrat->parent->func(CameraStrat->parent);
		*/
	}
	else
		player[currentPlayer].CameraStrat->parent = NULL;
}


void SetObjectScale(STRAT *strat, int objId, float x, float y, float z)
{
	if (x == 0.0f)
		x = 0.01f;
	if (y == 0.0f)
		y = 0.01f;
	if (z == 0.0f)
		z = 0.01f;
	
	if (objId == 0)
	{
		strat->obj->scale.x = x;
		strat->obj->scale.y = y;
		strat->obj->scale.z = z;
	}
	else
	{
		strat->objId[objId]->scale.x = x;
		strat->objId[objId]->scale.y = y;
		strat->objId[objId]->scale.z = z;
	}
}

void SetObjectAnimScale(STRAT *strat, int objId, float x, float y, float z)
{
	if (objId == 0)
	{
		strat->obj->animscale.x = x;
		strat->obj->animscale.y = y;
		strat->obj->animscale.z = z;
	}
	else
	{
		strat->objId[objId]->animscale.x = x;
		strat->objId[objId]->animscale.y = y;
		strat->objId[objId]->animscale.z = z;
	}
}


void SetObjectTrans(STRAT *strat, int objId, float trans)
{
	if (objId == 0)
	{
		strat->trans = trans;
	}
	else
	{
		strat->objId[objId]->transparency = trans;
	}
}




void DrawArrow(STRAT *strat, float x, float y, int id)
{
	rSprite2D(x,y,strat->objId[id]);
	
}

int InitTimer (int CountDown, int StartTime)
{
	if (StartTime > 0) {
		TimerDirection = CountDown;
		TimerTime = StartTime;
		TimerShow = 1;
	}
	return TimerTime;
}

void AddTime(int AdditionalTime)
{
	TimerTime = TimerTime + AdditionalTime;
}

void HideTimer (void)
{
	TimerShow = 0;
}



// Spawnpoints
typedef struct {
	STRAT	*strat;
	Uint32	SpawnID;
	Bool	active;
} SpawnPoint;

static SpawnPoint sPts[64];

int RegisterSpawnPoint (STRAT *strat, Uint32 id)
{
	sPts[NumSpawnPoints].strat = strat;
	sPts[NumSpawnPoints].SpawnID = id;
	sPts[NumSpawnPoints].active = FALSE;
	return NumSpawnPoints++;
}

void SetSpawnActivity (Uint32 number, Bool active)
{
	sPts[number].active = active;
}

void SignalSpawnPoint (Uint32 id)
{
	int i, nApplicable, nChosen;
	nApplicable = 0;
	for (i = 0; i < NumSpawnPoints; ++i) {
		if (sPts[i].active &&
			(sPts[i].SpawnID & id))
			nApplicable ++;
	}
	
	if (nApplicable == 0)
		return;
	
	do {
		nChosen = rand() & 63;
	} while (nChosen >= nApplicable);
	for (i = 0; i < NumSpawnPoints; ++i) {
		if (sPts[i].active &&
			(sPts[i].SpawnID & id)) {
			if (nChosen-- == 0)
				break;
		}
	}
	sPts[i].strat->var = (float) id;
	sPts[i].active = 0;
}

bool NearAnyPlayer (STRAT *strat, float dist)
{
	int i;
	dist = dist*dist;
	for (i = 0; i < 4; ++i) {
		if (player[i].active &&
			player[i].Player) {
			if (pSqrDist (&player[i].Player->pos, &strat->pos) < dist)
				return TRUE;
		}
	}
	return FALSE;
}

int GetParentPlayerNumber (STRAT *strat)
{
	if (strat)
	{
		if (strat->parent)
		{
			return GetPlayerNumber (strat->parent);
		}
		else
			return -1;
	}
}

int IAmAimStrat(STRAT *strat)
{
	if (strat == player[0].aimStrat)
		return(1);
	else
		return(0);
}

void SetObjectShadowState(STRAT *strat, int OnOrOff)
{
	dAssert (strat && strat->obj, "No model or strat");
	if (!strat || !strat->obj)
		return;
	if (OnOrOff) {
		strat->obj->collFlag |= OBJECT_STRAT_SHADOW;
	} else {
		strat->obj->collFlag &= ~OBJECT_STRAT_SHADOW;
	}
}


void LogPrint(int x, int y, float val)
{
	tPrintf (x, y, "%f value", val); 
	
	
}

//***** ONLY TO BE USED WITH NON-SHARED MODELS SUCH AS BOSSES *******/

void PNODESetInitToCrnt(STRAT *strat, int id)
{
	strat->objId[id]->idlePos =	strat->pnode->objId[id]->initPos = strat->objId[id]->crntPos;
	strat->objId[id]->idleRot =	strat->pnode->objId[id]->initRot = strat->objId[id]->crntRot;
}

int PickupAllowed (int type)
{
	if (Multiplayer) {
		switch (type) {
		case BOMB:
			if (!(mpSettings.weapons & WEAPON_BOMB))
				return FALSE;
			break;
		case ROCKET:
			if (!(mpSettings.weapons & WEAPON_HOMING_MISSILE))
				return FALSE;
			break;
		case MINE:
			if (!(mpSettings.weapons & WEAPON_MINE))
				return FALSE;
			break;
		case VULCAN:
			if (!(mpSettings.weapons & WEAPON_VULCAN))
				return FALSE;
			break;
		case ARMOUR:
			if (!(mpSettings.weapons & WEAPON_HEALTH))
				return FALSE;
			break;
		case HOMING_BULLET:
			if (!(mpSettings.weapons & WEAPON_HOMING_CANNON))
				return FALSE;
			break;
		case ELECTRO:
			if (!(mpSettings.weapons & WEAPON_ELECTRO))
				return FALSE;
			break;
		case LASER:
			if (GetCurrentGameType() == PREDATOR || !(mpSettings.weapons & WEAPON_ULTRA_LASER))
				return FALSE;
			break;
		case CLOAKING:
			if (GetCurrentGameType() != DEATHMATCH && GetCurrentGameType() != KNOCKOUT)
				return FALSE;
			if (!(mpSettings.weapons & WEAPON_STEALTH))
				return FALSE;
			break;
		case STORMING_SHIELD:
			if ((GetCurrentGameType() == DEATHMATCH || GetCurrentGameType() == KNOCKOUT) && (mpSettings.weapons & WEAPON_STORMING_SHIELD))
				return TRUE;
			else
				return FALSE;
			break;
		}
	}
	return TRUE;
}

void UpdateObjectCollFlag(STRAT *strat, int id, int flag)
{
	strat->objId[id]->collFlag |= flag;
	
}

void ClearObjCollFlag(STRAT *strat, int id, int flag)
{
	strat->objId[id]->collFlag =
		strat->objId[id]->collFlag & LNOT(flag);
	
}

void ObjectSpecularFlash(STRAT *strat, int objId, int flashOrNot)
{
   	dAssert (strat && objId && objId <= strat->pnode->noObjId && strat->objId[objId] && strat->objId[objId]->model,
		"Invalid parms");
	if (flashOrNot)
		strat->objId[objId]->collFlag |= OBJECT_SPECULARFLASH;
	else
	{
		strat->objId[objId]->specularCol.colour = 0;
		strat->objId[objId]->collFlag &= ~OBJECT_SPECULARFLASH;
		SetSpecularColour(strat,objId,0.0,0.0,0.0);
	}
}

//true if beam is hitting the player
int BeamHit(STRAT *strat,float length, float Radius,float x, float y, float z)
{
	float SaveX, SaveY, SaveZ;
	float dist;
	int  res; 
	Point3 end;
	
	//Matrix temp;
	//Point3 pos,end;
	
	end.x = x;
	end.y = y;
	end.z = z;
	
	SaveX = strat->CheckPos.x;
	SaveY = strat->CheckPos.y;
	SaveZ = strat->CheckPos.z;
	
	/*	temp= strat->m;
	
	  mPreScale(&temp,strat->scale.x,strat->scale.y,strat->scale.z);
	  
		mPreTranslate(&temp,0,length,0);
		
		  pos = strat->pos;
		  pos.x += temp.m[3][0];
		  pos.y += temp.m[3][1];
	pos.z += temp.m[3][2]; */
	
	////get the beam's end point
	//	SetCheckPosRel(strat,0,0, length * strat->scale.y, 0); 
	SetCheckPosRel(strat,0,0, length, 0); 
	
	
	//get dist from RD 
	//	dist = pointLineDistance(&player[0].Player->pos,&strat->pos,&strat->CheckPos);
	dist = pointLineDistance(&end,&strat->pos,&strat->CheckPos);
	//	dist = pointLineDistance(&strat->CheckPos,&strat->pos,&end);
	
	
	
	if (dist < (player[0].Player->pnode->radius + Radius))
		res = 1;
	else
		res = 0;
	
	strat->CheckPos.x = SaveX;
	strat->CheckPos.y = SaveY;
	strat->CheckPos.z = SaveZ;
	
	
	
	return(res);
	
	
}


#define MAXRAD 4
STRAT *RadarArray[MAXRAD];

//mode 0 = clean
//mode 1 = update
//mode 2 = unregister

static Uint32 GetBlendColour (float dist)
{
	float blend;
	if (dist < 50.0) {
		blend = RANGE(0, (dist - 30.f)/20.f, 1);
		return 0xff000000 | ColLerp (0xff0000, 0xffff00, blend);
	} else {
		if (dist < 110.0) {
			blend = RANGE(0, (dist - 90.f)/20.f, 1);
			return 0xff000000 | ColLerp (0xffff00, 0x00ff00, blend);
		} else
			return 0xff00ff00;
	}
}

void RadarUpdate(STRAT *strat, int update)
{
	int i;
	int closest;
	Uint32 colour;
	float lastdist;
	float dist;

	if (GetGlobalParamInt(0) == 1)
		return;

	switch (update)
	{
		case (0):
			//CLEAN
			for (i=0;i<MAXRAD;i++)
				RadarArray[i] = NULL;
			break;
		case (1):
			//REGISTER
			for (i=0;i<MAXRAD;i++)
			{
				if (!RadarArray[i])
				{
					RadarArray[i] = strat;
					break;
				}
			}
			break;
		case (2):
			//UNREGISTER
			for (i=0;i<MAXRAD;i++)
			{
				if (RadarArray[i] == strat)
				{
					RadarArray[i] = NULL;
					break;
			
				}
			}
			break;
		case (3):
			//DRAW
			//FIRST FIND RADAR ENTRY CLOSEST
			dist = 1000000.0f;
		   	closest = -1;
			
			for (i=0;i<MAXRAD;i++)
			{
				if (RadarArray[i])
				{
					lastdist = pDist(&strat->pos,&RadarArray[i]->pos);
					if (lastdist < dist)
					{
						dist = lastdist;
						closest = i;

					}
				}
			}
			//NOW DRAW ARROW

			if (closest >= 0)
			{
				colour = GetBlendColour (dist);
				DrawArrowToPoint(&RadarArray[closest]->pos,colour);
			}
			break;

		case 4:
			dist = pDist(&player[0].Player->pos,&RadarArray[0]->pos);
			colour = GetBlendColour (dist);
			DrawArrowToPoint(&RadarArray[0]->pos,colour);
			break;

		default:
			break;



	}

}


static void CopyAnim(Object *parentobj, Object *obj)
{
	int i;
   //	int OBJID = OBJECT_GET_ID(obj->collFlag);
	
	
   //	if (OBJID)	  
   //	{
	/*	obj->m.m[0][0] = parentobj->m.m[0][0];	 
		obj->m.m[0][1] = parentobj->m.m[0][1];	 
		obj->m.m[0][2] = parentobj->m.m[0][2];	 
		obj->m.m[0][3] = parentobj->m.m[0][3];	 
		obj->m.m[1][0] = parentobj->m.m[1][0];	 
		obj->m.m[1][1] = parentobj->m.m[1][1];	 
		obj->m.m[1][2] = parentobj->m.m[1][2];	 
		obj->m.m[1][3] = parentobj->m.m[1][3];	 
		obj->m.m[2][0] = parentobj->m.m[2][0];	 
		obj->m.m[2][1] = parentobj->m.m[2][1];	 
		obj->m.m[2][2] = parentobj->m.m[2][2];	 
		obj->m.m[2][3] = parentobj->m.m[2][3];	 
	  	obj->m.m[3][0] = parentobj->m.m[3][0];
		obj->m.m[3][1] = parentobj->m.m[3][1];	 
		obj->m.m[3][2] = parentobj->m.m[3][2];	 
  		obj->m.m[3][3] = parentobj->m.m[3][3];*/
  //	}	

	obj->m = parentobj->m;
   	obj->collFlag &=  LNOT(OBJECT_CALC_MATRIX);


	for (i = 0; i < obj->no_child; ++i)
	{
		CopyAnim (&parentobj->child[i],&obj->child[i]);	
	}

}

void CopyParentAnim(STRAT *strat)
{

	CopyAnim(strat->parent->obj,strat->obj);

}

void TrigCheckMe (STRAT *strat)
{
   //	Point3 p;
   //	p = player[currentPlayer].Player->pos;
   //	player[currentPlayer].Player->pos = strat->pos;
	TrigCheck(&strat->pos);
   //	player[currentPlayer].Player->pos = p;
}

void FadeOut (void)
{
	StartFade (FADE_TO_BLACK_FAST);
}

//stops n hides all ENEMY strats 'cept the caller
void StopTheStrats(STRAT *strat)
{	STRAT *curstrat;

	curstrat = FirstStrat;
	while (curstrat)
	{
		if ((curstrat->flag & STRAT_ENEMY) && (curstrat != strat))
		{
			//DOES IT HAVE A CALLBACK CHECK ?
			if (curstrat->flag & STRAT_DELETECALLBACK)
			{
				/* envoke any deletion callbacks*/
				ProcTriggerDeleted(curstrat,curstrat->trigger);
				curstrat->flag = curstrat->flag & LNOT(STRAT_DELETECALLBACK);
			}

			//stop all existing strat triggers
			PauseTriggers(curstrat,-2,1);
		
		  			
			if (curstrat->IVar)
			{
				CHeapFree(IntVarHeap,curstrat->IVar);
				curstrat->IVar=NULL;
			}
			if (curstrat->FVar)
			{
				CHeapFree(FloatVarHeap,curstrat->FVar);
			   	curstrat->FVar=NULL;
			}



		 	curstrat->flag |= STRAT_HIDDEN;
			curstrat->flag2 |= STRAT2_PROCESSSTOPPED;
		}

		curstrat = curstrat->next;

	}


}

int PlayerIsAssassin(int num)
{
	if (!Multiplayer || GetCurrentGameType() != PREDATOR)
		return false;
	else
		return player[num].special;
}

//*******************************************************//
//movement routines
//******************************************************
void SetFriction(STRAT *strat, float x, float y, float z)
{
	strat->friction.x = 1.0f - x;
	strat->friction.y = 1.0f - y;
	strat->friction.z = 1.0f - z;

   	if (PAL)
	{
		strat->friction.x = pow(strat->friction.x, 1.44f); 
		strat->friction.y = pow(strat->friction.y, 1.44f);
		strat->friction.z = pow(strat->friction.z, 1.44f);
	}

	if (strat->Player)
		player[strat->Player->n].PlayerFriction = strat->friction;
}

void stratPitch(STRAT *crntStrat, float amount)
{
	Vector3	axis;
	if (PAL)
		amount *= PAL_MOVESCALE;

	axis.x = crntStrat->m.m[0][0];
	axis.y = crntStrat->m.m[0][1];
	axis.z = crntStrat->m.m[0][2];
	if (amount != 0.0f)
		combineRotation(crntStrat, &axis, amount);
}

void stratYaw(STRAT *crntStrat, float amount)
{
	Vector3	axis;
	if (PAL)
		amount *= PAL_MOVESCALE;

	axis.x = crntStrat->m.m[2][0];
	axis.y = crntStrat->m.m[2][1];
	axis.z = crntStrat->m.m[2][2];
	if (amount != 0.0f)
		combineRotation(crntStrat, &axis, amount);
}

void stratRoll(STRAT *crntStrat, float amount)
{
	Vector3	axis;
	if (PAL)
		amount *= PAL_MOVESCALE;


	axis.x = crntStrat->m.m[1][0];
	axis.y = crntStrat->m.m[1][1];
	axis.z = crntStrat->m.m[1][2];
	if (amount != 0.0f)
		combineRotation(crntStrat, &axis, amount);
}

static float Derive(STRAT *strat)
{
	Vector3 dist;
	float real;

	v3Sub(&dist, &strat->CheckPos, &strat->pos);
	real =((float)sqrt(v3Dot(&dist,&dist)));	
	if (real < 1.0f)
		real =  1.0f;

	return(real);
}

static void RollRot(Vector3 *stratWay,Vector3 *forward,STRAT *strat)
{
	Vector3 newAxis;
	float ang;

	newAxis.x = forward->x;
	newAxis.y = forward->y;
	newAxis.z = 0;
	ang = v3Dot(&newAxis, stratWay);
	ang = (float)acos(ang);

	if (ang < 0.001f)
		return;
	v3Cross(&newAxis, forward, stratWay);

 	/*dLog("banking by ang %f\n",ang * (360/PI2)); */
	
	if ((newAxis.z < 0) )
		mPreRotateY(&strat->m,ang/12.0f);
	else
		mPreRotateY(&strat->m,-ang/12.0f);
}


/*Point directly at the point */

static void DirectMove(STRAT *strat,Point3 *p, float maxx)
{
	Vector3	stratAxis, stratToWay, newAxis;
	float	ang,amount;

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
		amount = ((strat->relVel.y) * ang * PI2)/Derive(strat);
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

/*PITCH, ROLL and a bit of banking action */

static void PlaneMove(STRAT *strat,Point3 *p,float maxx)
{
	Vector3	stratAxis, stratToWay, newAxis;
	float	ang,amount;

	stratAxis.x = strat->m.m[1][0];
	stratAxis.y = strat->m.m[1][1];
	stratAxis.z = strat->m.m[1][2];
	
	stratToWay.x = (p->x - strat->pos.x);
	stratToWay.y = (p->y - strat->pos.y);
	stratToWay.z = (p->z - strat->pos.z);



	v3Normalise(&stratAxis);
	v3Normalise(&stratToWay);


	ang = v3Dot(&stratAxis, &stratToWay);

	ang = (float)acos(ang);

	if (ang < 0.01f)
		return;

	if (ang > maxx)
		ang = maxx;

   	if (strat->route)	
   		amount = ((strat->relVel.y) * ang * PI2)/Derive(strat);
   	else	
		amount = (ang * 0.1f);
	
	
	v3Cross(&newAxis, &stratAxis, &stratToWay);

	if ((newAxis.x == 0.0f) &&
		(newAxis.y == 0.0f) &&
		(newAxis.z == 0.0f))
		return;
	v3Normalise(&newAxis);


	stratToWay.z = 0;
   	RollRot(&stratToWay,&stratAxis,strat);	
		
	combineRotation(strat, &newAxis, amount);

	strat->rot_speed *= 0.5f;


}

static void TowardWayZ(STRAT *strat, Point3 *p,float maxx)
{
	Vector3	stratAxis, stratToWay, newAxis;
	float	ang,lastdist;


	stratAxis.x = strat->m.m[1][0];
	stratAxis.y = strat->m.m[1][1];
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
  

  	if (!(SeePointZ(strat,p,PI2/8.0f)))
	{

		lastdist = CheckDistXYNoRoot(strat);
		if (lastdist)
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
		strat->turnangz =ang;
		mPreRotateZ(&strat->m, ang);
	}
	else
	{
		strat->turnangz =-ang;
		mPreRotateZ(&strat->m, -ang);
	}
}



//point in lateral and vertical vectors only. X/Z plane in max (X/Y plane)
void TurnTowardPosXZ(STRAT *strat, float x, float y, float z, float xamount, float zamount)
{
	Matrix im;
	Point3	p, ip;
	Vector3	stratToP, stratForward, q1, q2;
	float	ang;

	if (PAL)
	{
		xamount *= PAL_MOVESCALE;
		zamount *= PAL_MOVESCALE;

	}

	p.x = x;
	p.y = y;
	p.z = z;
	stratForward.x = strat->m.m[1][0];
	stratForward.y = strat->m.m[1][1];
	stratForward.z = strat->m.m[1][2];
	v3Sub(&stratToP, &p, &strat->pos);
	v3Normalise(&stratToP);

	q1 = stratToP;
	q2 = stratForward;
	q1.z = 0.0f;
	q2.z = 0.0f;

	ang = acos(v3Dot(&q1, &q2));
	if (ang > zamount)
		ang = zamount;

	v3Sub(&ip, &p, &strat->pos);
	mInvertRot(&strat->m, &im);
	mPoint3Apply3(&ip, &im);

	if (ip.x > 0.0f)
		mPreRotateZ(&strat->m, -ang);
	else
		mPreRotateZ(&strat->m, ang);


	q1 = stratToP;
	q2 = stratForward;
	q1.x = 0.0f;
	q2.x = 0.0f;

	ang = acos(v3Dot(&q1, &q2));
	if (ang > xamount)
		ang = xamount;

	if (ip.z < 0.0f)
		mPreRotateX(&strat->m, -ang);
	else
		mPreRotateX(&strat->m, ang);



}



/*YAW TOWARD ONLY */



static void *processSpecWay[3] = 
{
	&DirectMove,
	&PlaneMove,
	&TowardWayZ	/*HOVER */
 //	&TowardWayZ		/*GROUND */
};

static void stratTowardWay(STRAT *crntStrat, Point3 *p, float amount)
{
	int movetype;
	void	(*specProc)(STRAT *crntStrat, Point3 *p,float amount);

	if (PAL)
		amount *= PAL_MOVESCALE;
   	

	movetype=(crntStrat->flag>>17)&3;
   //	movetype=(crntStrat->flag>>16)&8;


/*	dLog("movetype %d\n",movetype); */
	specProc = processSpecWay[movetype];
	specProc(crntStrat,p,amount);
	return;

}

void TowardPlayer (STRAT *strat, float maxx, float maxy)
{
	stratTowardWay(strat,&player[currentPlayer].Player->pos,maxx);
}


void TowardWay (STRAT *strat, float maxx, float maxy)
{
	stratTowardWay(strat,&strat->CheckPos,maxx);
}


void TowardTarget(STRAT *strat,float maxx, float maxy)
{
	stratTowardWay(strat,&strat->target->pos,maxx);
}

void TowardParent (STRAT *strat, float maxx, float maxy)
{
	stratTowardWay(strat,&strat->parent->pos,maxx);
}



void MoveX (STRAT *strat, float amount)
{
	if (PAL)
		amount *= PAL_ACCSCALE;
	strat->relAcc.x += amount;
}

void MoveY (STRAT *strat, float amount)
{
	if (PAL)
		amount *= PAL_ACCSCALE;

	strat->relAcc.y += amount;
}


void MoveZ (STRAT *strat, float amount)
{
	if (PAL)
		amount *= PAL_ACCSCALE;
	strat->relAcc.z += amount;
}

void	RelRotateY(STRAT *strat, float ang)
{
	if (PAL)
		ang *= PAL_MOVESCALE;
	mPreRotateY(&strat->m,ang);


}

void MoveXFlat (STRAT *strat, float amount)
{
	Vector3 v;
   	if (PAL)
		amount *= PAL_MOVESCALE;

  	v.x = strat->m.m[0][0];
	v.y = strat->m.m[0][1];
	v.z = 0.0f;
	v3Normalise(&v);
	v3Scale(&v, &v, amount);
	v3Inc(&strat->vel, &v);
}

void MoveYFlat (STRAT *strat, float amount)
{
	Vector3 v;
	if (PAL)
		amount *= PAL_MOVESCALE;

	v.x = strat->m.m[1][0];
	v.y = strat->m.m[1][1];
	v.z = 0.0f;
	v3Normalise(&v);
	v3Scale(&v, &v, amount);
	v3Inc(&strat->vel, &v);
}

void MoveXNoZ (STRAT *strat, float amount)
{
	Vector3 vec, vec2;

	if (PAL)
		amount *= PAL_MOVESCALE;

	vec2.x = amount;
	vec2.y = 0.0f;
	vec2.z = 0.0f;
	mVector3Multiply (&vec, &vec2, &strat->m);
	strat->vel.x += vec.x; 
	strat->vel.y += vec.y; 
}

float	XRelPosX(STRAT *strat, float amount)
{
	Vector3	dir;

	if (PAL)
		amount *= PAL_MOVESCALE;

	dir.x = strat->m.m[0][0];
	dir.y = strat->m.m[0][1];
	dir.z = strat->m.m[0][2];

	v3Scale(&dir, &dir, amount);
	return strat->pos.x + dir.x;
}


float	XRelPosY(STRAT *strat, float amount)
{
	Vector3	dir;

	if (PAL)
		amount *= PAL_MOVESCALE;

	dir.x = strat->m.m[0][0];
	dir.y = strat->m.m[0][1];
	dir.z = strat->m.m[0][2];

	v3Scale(&dir, &dir, amount);
	return strat->pos.y + dir.y;
}



void	RelRotateX(STRAT *strat, float ang)
{
	if (PAL)
		ang *= PAL_MOVESCALE;
	mPreRotateX(&strat->m,ang);


}

void	RelRotateZ(STRAT *strat, float ang)
{
	
	if (PAL)
	   ang *= PAL_MOVESCALE;
	mPreRotateZ(&strat->m,ang);


}

void Yaw   (STRAT *strat, float amount)
{
	if (PAL)
		amount *= PAL_MOVESCALE;
	strat->turn.z = amount;
}
