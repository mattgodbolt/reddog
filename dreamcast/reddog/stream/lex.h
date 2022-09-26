#ifndef LEX_H
#define LEX_H

//DEBUG DEFINES
#define DEBUG 0
#define	DEBUGMOD 0		//TO CHECK MODELS PROCESSED
#define	DEBUGANIM 0		//TO CHECK ANIMS PROCESSED
#define	DEBUG_WAY 0		//TO CHECK ANIMS PROCESSED
#define	DEBUGTRIG 0
#define	DEBUG_TIMESTAMP 0 //TO CHECK THE TIMESTAMP PROCESSING
#define DEP_DEBUG	0	//TO CHECK THE DEPENDENCY PROCESSING
#define DEBUG_INT	0	//TO CHECK INTERNAL LISTS
#define DEBUG_OUT	0 //TO CHECK OUTPUT MODEL LISTS
#define DEBUG_STRATDUPS	0 //TO CHECK DUPLICATE FOUND STRATS
#define DEBUG_MAPFLAGS	0 //TO CHECK THE LIST OF FLAGS FOR EACH MAP STRAT
#define DEBUG_SOLVE 0 //TO CHECK WHAT STRATS ARE SOLVED

//MAXIMUM NUMBER OF CHARACTERS USED IN GAME
// 0 - INTELLIGENCE OFFICER
// 1 - HERO
// 2 - COMMANDER
// 3 - DROPSHIP PILOT
#define MAXGAMECHARS 10


//CODE DEFINES
#define MAX_LINE 512
#define MAX_STATE 45
#define MAX_DEFS 80
#define MAX_STRATS 1024
#define MAX_MODELS 512
#define MAX_NESTS 15

#define FIRSTPASS		0
#define SECONDPASS	1 

#define NOP				0
#define STRAT			1
#define STATEDEFINE		2
#define STATECHANGE		3
#define END				4
#define LOOP			5
#define ENDLOOP			6
#define WHILE			7
#define ENDWHILE		8
#define DEFINE			9
#define IF				10
#define ENDIF			11
#define ENDSTATE		12
#define ELSE			13
#define PRINT			14
#define LOCALINT		15
#define LOCALFLOAT		16
#define	EVALUATION		17
#define	CREATE			18
#define	BOIDCREATE		19
#define	TRIGON			20
#define	TRIGOFF			21
#define	TRIGEND			22
#define	TRIGFIN			23
#define	TRIGHOLD		24
#define	TRIGFREE		25
#define	SETMODEL		26
#define	CREATEIND		27
#define	LOOPIMM			28
#define	ENDIMMLOOP		29
#define	SETANIM			30
#define	FIRE			31
#define	DESTROYME		32
#define	PICKUP			33
#define	TRIGRESET		34
#define	PARAMINT		35
#define	PARAMFLOAT		36
#define SETMODELANIM    37
#define CREATEFAST		38
#define SETSOUND		39

//MISC DEFINES
#define CHILD		 0
#define BOID		 1
#define IND			 2
#define MFIRE		 3
#define CHILDFAST	 4
#define INT 1
#define	FLOAT 2


#define STRAT2_PICKUP (1<<15)

/******* STRAT KEYWORDS *******/

static struct {
	char *word;
	short length;
	short token;

#define K(word,token) {word,sizeof(word)-1,token,},
} Keywords[] = {
	K("STATE",	  		STATEDEFINE)
	K("TRIGGER",   		STATEDEFINE)
	K("STAT>",	   		STATECHANGE)
	K("STRAT",	   		STRAT)
	K("ENDSTRAT",		END)
	K("LOOP",   		LOOP)
	K("ENDLOOP",   		ENDLOOP)
	K("WHILE",   		WHILE)
	K("ENDWHILE",   	ENDWHILE)
	K("DEFINE",   		DEFINE)
	K("IF",   	   		IF)
	K("ENDIF",   		ENDIF)
	K("ENDTRIGGER",   	ENDSTATE)

	K("ENDSTATE",   	ENDSTATE)
	K("ELSE",   		ELSE)
	K("PRINT",   		PRINT)
	K("LOCALINT",   	LOCALINT)
	K("LOCALFLOAT",   	LOCALFLOAT)
	K("PARAMINT",   	PARAMINT)
	K("PARAMFLOAT",   	PARAMFLOAT)
	K("CREATE",		   	CREATE)
	K("FASTCREATE",		CREATEFAST)
	K("BOIDCREATE",		BOIDCREATE)
	K("TRIGSET>",		TRIGON)
	K("TRIGKILL>",		TRIGOFF)
	K("TRIGSTOP",		TRIGEND)
	K("TRIGFIN",		TRIGFIN)
	K("TRIGRESET",		TRIGRESET)
	K("TRIGHOLD>",		TRIGHOLD)
	K("TRIGFREE>",		TRIGFREE)
	K("OBJECT>",	    SETMODEL)
	K("PLAYSOUND>",		SETSOUND)
	K("ANIM>",		    SETANIM)
	K("CREAIND",		CREATEIND)
	K("MISSFIRE",		FIRE)
	K("IMMLOOP",		LOOPIMM)
	K("IMMENDLOOP",		ENDIMMLOOP)
	K("DESTROYME",		DESTROYME)
	K("PICKUP>",		PICKUP)
	K("MODELANIM>",		SETMODELANIM)
};
#undef K

/************** CAMERA SCRIPT KEYWORDS *********/
#define CAMERASCR	1
#define FRAMES		2

static struct {
	char *word;
	short length;
	short token;


#define K(word,token) {word,sizeof(word)-1,token,},
} CamFields[] = {
		K("CAMERA",	CAMERASCR)
		K("FRAMES",	FRAMES)
};
#undef K

/************** IN-GAME SCRIPT KEYWORDS *********/
#define TEXTSTART				1
#define TEXTEND					2
#define TEXTENTRY				3
#define SCRIPTENTRY				4
#define SCRIPTTIME				5
#define SCRIPT_SKIPPABLE		6	
#define SCRIPT_STOPSGAME		7
#define SCENESTART				8
#define SCENEEND				9
#define SCENESCRIPTENTRY		10
#define SCENEENTRYEND			11

#define	INTELLIGENCE_OFFICER	12
#define	HERO					13
#define	COMMANDER				14
#define	DROPSHIP_PILOT			15
#define	NOTEXT					16





static struct {
	char *word;
	short length;
	short token;


#define K(word,token) {word,sizeof(word)-1,token,},
} ScriptFields[] = {
		K("INTELLIGENCE OFFICER",	INTELLIGENCE_OFFICER)
		K("HERO", HERO)
		K("COMMANDER",	COMMANDER)
		K("DROPSHIP PILOT",	DROPSHIP_PILOT)
		K("NOTEXT",	NOTEXT)





		K("ENTRY ",	SCRIPTENTRY)
		K("TIME ",  SCRIPTTIME)
		K("START",	TEXTSTART)
		K("END",	TEXTEND)
		K("TEXT ",	TEXTENTRY)
		K("SCENEEND",	SCENEEND)
		K("SCENESCRIPTENTRY",	SCENESCRIPTENTRY)
		K("SCENEENTRYEND",	SCENEENTRYEND)
		K("SCENE",	SCENESTART)
	   	K("SCRIPT CAN BE SKIPPED", SCRIPT_SKIPPABLE)
		K("SCRIPT STOPS THE GAME", SCRIPT_STOPSGAME)
};
#undef K








/************** MAP FIELD KEYWORDS *************/

#define ENDSTRAT	1
#define POS			2
#define MODEL		3
#define RELVEL		4
#define ABSVEL		5
#define ROT			6
#define FLAG		7
#define WAYPATH		8
#define NUMPATH		9
#define NUMWAYS		10
#define WAYPOS		11
#define ENDMAP		12
#define ENDDEFAULT	13
#define NUMTRIGS	14
#define TRIGPATH	15
#define ENDTRIG		16
#define NUMTRIG		17
#define ATTACHED	18
#define TRIGVERTEX	19
#define TRIGRADIUS	20
#define MAPENTRY	21
#define ARMOUR		22
#define HEALTH		23
#define SCALE		24
#define NUMLIGHTS		25
#define OMNILIGHT		26
#define SPOTLIGHT		27
#define ENDOMNILIGHT	28
#define ENDSPOTLIGHT	29
#define COLOUR			30
#define NEAR			31
#define FAR				32
#define MINVAL				33
#define MAXVAL				34
#define DIR				35
#define ENDLIGHTS		36
#define PATHAREA		37

#define AREA				38
#define ENDAREA				39
#define NUMAREAPOINTS		40
#define AREAVERTEX			41
#define NUMAREAS			42
#define SUBAREAS			43
#define SUBAREAVERTEX		44
#define SUBAREARADIUS		45
#define AREARADIUS			46
#define NUMCIRCLEAREAS		47
#define NUMELLIPSEAREAS		48
#define ELLIPSELENGTH		49
#define ELLIPSEWIDTH		50

#define NUMBOXES			51
#define BOXMAXX				52
#define BOXMINX				53
#define BOXMAXY				54
#define BOXMINY				55
#define NUMSHAPEAREAS		56
#define WAYTYPE				57
#define NODEVERTEX			58
#define NODEMASK			59
#define NETNODENUM			60
#define NUMBERNETWAY		61
#define NETPATHNODE			62
#define STARTWAY			63
#define TRIGGERFLAG			64
#define TRIGGERACTIVATECOUNT		65
#define ACTPOINT		66
#define NUMACT		67
#define ACTRAD		68
#define ACTVERT		69
#define ENDACT		70
#define NUMSIGHTAREAS		71
#define NUMPATROLROUTES		72
#define STARTPATH		73
#define MAPVARS		74
#define LIGHTCOL	75
#define FOGVAL	76
#define FLOATVAL	77
#define BYTEVAL	78
#define INTVAL	79
#define SHORTVAL	80
#define ENDEVENT	81
#define DOMEVAL		82
#define POINT3VAL	83
#define NUMEXTSPLINEAREAS		84
#define ENDPATHS  85
#define SUBSUBAREAS	86
#define		STRAT_FRIENDLY		2048	//FRIENDLY
#define		STRAT_ENEMY			4096	//ENEMY


static struct {
	char *word;
	short length;
	short token;


#define K(word,token) {word,sizeof(word)-1,token,},
} MapFields[] = {
		K("ENDSTRAT",  						ENDSTRAT)
		K("ENDMAP",  						ENDMAP)
		K("Pos :",	  						POS)
		K("Object :",	  					MODEL)
		K("Relative Velocity :",	  		RELVEL)
		K("Absolute Velocity :",	  		ABSVEL)
		K("Rot :",	  						ROT)
		K("Flag :",	  						FLAG)
//		K("WayPath :",	  					WAYPATH)
		K("WayPath :",	  					SHORTVAL)
		K("NUMPATHS :",	  					NUMPATH)
		K("NumWayPoints :",					NUMWAYS)
		K("WayVertex :",					WAYPOS)
		K("ENDDEFAULT :",					ENDDEFAULT)
		K("TrigPath :",						TRIGPATH)
		K("TRIGEND :",						ENDTRIG)
		K("NUMTRIGS :",						NUMTRIG)
		K("ATTACHED STRATS :",				ATTACHED)
		K("TriggerVertex :",				TRIGVERTEX)
		K("TriggerRadius :",				TRIGRADIUS)
		K("MapEntry :",						MAPENTRY)
		K("Armour :",						ARMOUR)
		K("Health :",						HEALTH)
		K("Scale :",						SCALE)
		K("PathArea :",						PATHAREA)

		K("NUMLIGHTS", 		NUMLIGHTS)
		K("OMNILIGHT",		OMNILIGHT)
		K("SPOTLIGHT",		SPOTLIGHT)
		K("ENDOMNILIGHT",	ENDOMNILIGHT)
		K("ENDSPOTLIGHT",	ENDSPOTLIGHT)
		K("Colour :",		COLOUR)
		K("Near :",			NEAR)
		K("Far :",			FAR)
		K("Min :",			MINVAL)
		K("Max :",			MAXVAL)
		K("Dir :",			DIR)
		K("ENDLIGHTS",		ENDLIGHTS)
		K("AREA",			AREA)
	  //	K("ENDAREA",		ENDAREA)
		K("ENDOFAREAS :",		ENDAREA)
		K("ENDOFPATHS :",		ENDPATHS)
		K("NUMAREAS :",		NUMAREAS)
		K("NumAreaPoints :",NUMAREAPOINTS)
		K("ArVertex :",		AREAVERTEX)
		K("SubArVertex :",	SUBAREAVERTEX)
		K("AreaRadius :",	AREARADIUS)
		K("SubAreas :",		SUBAREAS)
		K("SubSubAreas :",		SUBSUBAREAS)
		K("NumCircleAreas :",	NUMCIRCLEAREAS)
		K("NumEllipseAreas :",	NUMELLIPSEAREAS)
		K("SubAreaRadius :",SUBAREARADIUS)
		K("SubAreaLength :",ELLIPSELENGTH)
		K("SubAreaWidth :",ELLIPSEWIDTH)
		K("NumBoxAreas :",NUMBOXES)
		K("SubArMaxx :", BOXMAXX)
		K("SubArMinx :", BOXMINX)
		K("SubArMaxy :", BOXMAXY)
		K("SubArMiny :", BOXMINY)
		K("NumShapeAreas :",	NUMSHAPEAREAS)
		K("NumExtSplineAreas :",	NUMEXTSPLINEAREAS)
		K("NumSightAreas :",	NUMSIGHTAREAS)
		K("WAYTYPE:",		WAYTYPE)
		K("NodeVertex:",	NODEVERTEX)
		K("NodeMask:",		NODEMASK)
		K("NumNetNodes :",	NETNODENUM)
		K("NumPatrolRoutes :",	NUMPATROLROUTES)
		K("NumberNetWay:",	NUMBERNETWAY)
		K("NetPathNode:",	NETPATHNODE)
		K("Flag2 :",	FLAG)
//		K("StartWay :",	STARTWAY)
//		K("StartPath :", STARTPATH)
		K("StartWay :",	BYTEVAL)
		K("StartPath :", BYTEVAL)
		K("TriggerFlag :",	TRIGGERFLAG)
		K("TriggerActivateCount :",	TRIGGERACTIVATECOUNT)
		K("ActPoint :",	ACTPOINT)
		K("NUMACTS :",	NUMACT)
		K("ActivationRadius :",	ACTRAD)
		K("ActivationVertex :",	ACTVERT)
	   //	K("ENDACTIVATION :",	ENDACT)
		K("ACTIVATIONEND :",	ENDACT)
		K("LEVELFLAGSSET:",	MAPVARS)
		K("AMBIENT RED :",	LIGHTCOL)
		K("AMBIENT GREEN :",	LIGHTCOL)
		K("AMBIENT BLUE :",	LIGHTCOL)
		K("FOG RED :",	LIGHTCOL)
		K("FOG GREEN :",	LIGHTCOL)
		K("FOG BLUE :",	LIGHTCOL)
		K("FOG NEAR :",	FOGVAL)
		K("FOG FAR :",	FOGVAL)
		K("AMBIENT SCAPE INTENSITY :",	FLOATVAL)
		K("AMBIENT STRAT INTENSITY :",	FLOATVAL)
		K("FOV :",	FLOATVAL)
		K("FARZ :",	FLOATVAL)
		K("CAMHEIGHT :",	FLOATVAL)
		K("CAMDIST :",	FLOATVAL)
		K("CAMLOOKHEIGHT :",	FLOATVAL)

		K("NUM INT PARAMS :",	BYTEVAL)
	   	K("IPARAM:",	INTVAL)
		K("NUM FLOAT PARAMS :",	BYTEVAL)
		K("FPARAM:",	FLOATVAL)
		K("DELAY :",	SHORTVAL)
   		K("DependentStrat :",	SHORTVAL)
   		K("ATTACHED EVENTS :",				ATTACHED)
		K("NUMEVENTS :",				SHORTVAL)
		K("CAMMODS :",				SHORTVAL)
		K("CAMMOVEAMOUNT :",   FLOATVAL)
		K("CAMMOVESPEED :",	   FLOATVAL)
		K("NumberOfTrigChildren :",	BYTEVAL)
	  //	K("ENDEVENT :",	ENDEVENT)
		K("EVENTEND :",	ENDEVENT)
	   //	K("EVENT :",	ENDEVENT)
		K("Dependent :",	SHORTVAL)
	 	K("EventType :",	BYTEVAL)
		K("AttachedEvent :",	SHORTVAL)
		K("LEVEL DOME :",	DOMEVAL)
		K("Parent :",	SHORTVAL)
		K("NumCampingPoints :",	SHORTVAL)
		K("CampPos :",	AREAVERTEX)
		K("EventTimerValue :",	SHORTVAL)
	//	K("NumberOfTrigMasters :",	SHORTVAL)

};
#undef K








char IncludeFile[]={"#include EXT\n"};

/*************************************************************************/
/*************************************************************************/
/*************************************************************************/
/*************************************************************************/

//ERROR TABLE


char errortable[20][100]={
	"Duplicate or Invalid Label at",
	"possible label space error at",
	"Undefined label referenced at ",
	"MULTIPLE SWITCHES TO STATE AT ",
	"UNCONDITIONAL LOOP AT ",
	"LOOP NESTING ERROR ",
	"UNOPENED LOOP AT ",
	"STUPID NESTING OF LOOPS AT ",
	"Invalid Loop Counter at ",
	"WHILE NESTING ERROR AT",
	"UNOPENED WHILE AT ",
	"DEFINITION ERROR /UNDEFINED/WRONG/OR DUPLICATE ",
	"Error in while expression at ",
	"UNOPENED IF STATEMENT AT ",
	"IF NESTING ERROR AT ",
	"Error in if expression at ",
	"UNBALANCED ELSE ",
	"	Error in expression at ",
	"PLEASE ENSURE INT PARAMS ARE DECLARED BEFORE ANY OTHER VAR TYPE",
	"PLEASE ENSURE FLOAT PARAMS ARE DECLARED AFTER INT PARAMS AND BEFORE ANY OTHER VAR TYPE"
};
/*************************************************************************/
/*************************************************************************/
/*************************************************************************/
/*************************************************************************/


//RESERVED WORD VARIABLES

typedef struct reserved
{
	char *symbol;
	char *word;

} RESERVED;

RESERVED ReservedWords[] = {
						"Hard", "GameDifficulty",
						"MPStartupState", "MPStartupState",
						"RestartX", "CurrentRestart.x",					
						"RestartY", "CurrentRestart.y",					
						"RestartZ", "CurrentRestart.z",
						"RestartLookX", "CurrentRestartLook.x",					
						"RestartLookY", "CurrentRestartLook.y",					
						"RestartLookZ", "CurrentRestartLook.z",
						"RestartSet", "Restart",
						"x",		"strat->pos.x",
						"y",		"strat->pos.y",
						"z",		"strat->pos.z",
						"parentx",		"strat->parent->pos.x",
						"parenty",		"strat->parent->pos.y",
						"parentz",		"strat->parent->pos.z",
						"parentcheckX",		"strat->parent->CheckPos.x",
						"parentcheckY",		"strat->parent->CheckPos.y",
						"parentcheckZ",		"strat->parent->CheckPos.z",
					 	"MyParentScaleX", "strat->parent->scale.x",
					 	"MyParentScaleY", "strat->parent->scale.y",
						"MyPatrol", "strat->route->path->net->patrol && (!(strat->flag2 & STRAT2_ENTIRENET))",
						"MyTarget", "strat->target",
						"MyTargetParent","strat->target->parent",
						"MyTargetVar","strat->target->var",
						"oldoldx",		"strat->oldOldPos.x",
						"oldoldy",		"strat->oldOldPos.y",
						"oldoldz",		"strat->oldOldPos.z",
						"oldx",		"strat->oldPos.x",
						"oldy",		"strat->oldPos.y",
						"oldz",		"strat->oldPos.z",
						"EscortTankerX", "EscortTanker->pos.x",
						"EscortTankerY", "EscortTanker->pos.y",
						"EscortTankerZ", "EscortTanker->pos.z",
						"parentoldx",		"strat->parent->oldPos.x",
						"parentoldy",		"strat->parent->oldPos.y",
						"parentoldz",		"strat->parent->oldPos.z",
						"MySemaphore",		"strat->var",
					   	"MyParentRadius","strat->parent->pnode->radius",
						"ExpRot", "strat->pnode->expRot",
					   	"PlayerRadius","player[currentPlayer].Player->pnode->radius",
						"ParentSemaphore",		"strat->parent->var",
						"speedx",	"strat->relVel.x",
						"speedy",	"strat->relVel.y",
						"speedz",	"strat->relVel.z",
						"rotx",	"strat->turn.x",
						"roty",	"strat->turn.y",
						"rotz",	"strat->turn.z",
						"ParentRoty", "strat->parent->turn.y",
						"absspeedx",	"strat->vel.x",
						"absspeedy",	"strat->vel.y",
						"absspeedz",	"strat->vel.z",
						"parentabsspeedx",	"strat->parent->vel.x",
						"parentabsspeedy",	"strat->parent->vel.y",
						"parentabsspeedz",	"strat->parent->vel.z",
						"parentspeedz", "strat->parent->relVel.z",
						"accx",		"strat->relAcc.x",
						"accy",		"strat->relAcc.y",
						"accz",		"strat->relAcc.z",
						"Status",	"strat->state ",
						"Camera",   "player[currentPlayer].CameraStrat",
						"CameraTarget",   "player[currentPlayer].CameraStrat->parent",
						"PlayerFlag",	"player[currentPlayer].Player->flag ",
						"PlayerFlag2",	"player[currentPlayer].Player->flag2 ",
						"PlayerHeld",	"player[currentPlayer].PlayerHeld ",
						"MyFlag",	"strat->flag ",
						"MyParentFlag",	"strat->parent->flag ",
						"MyParentParentFlag",	"strat->parent->parent->flag ",
		 				"MyTargetFlag","strat->target->flag",
						"ScoobyDoo", "ScoobyDoo",
						"MyParentFlag2",	"strat->parent->flag2 ",
						"MyFlag2",	"strat->flag2 ",
						"MyParent",	"strat->parent ",
					   	"MyPathNode","strat->route->pathstartnode",
						"MyParentTarget",	"strat->parent->target ",
					 	"MyParentsParent","strat->parent->parent",
						"TargetX",	"strat->target->pos.x ",
					 	"TargetY",	"strat->target->pos.y ",
					 	"TargetZ",	"strat->target->pos.z ",
						"MyPath",	"strat->route ",
						"OnNet",	"strat->route->path->waytype&NETWORK ",
						"OnEntrySpline", "strat->route->path->waytype&ENTRYSPLINE",
						"offx",		 "strat->offset.x",
						"offy",		"strat->offset.y",
						"offz",		"strat->offset.z",
						"scalex",	 "strat->scale.x",
						"scaley",	"strat->scale.y",
						"scalez",	"strat->scale.z",
						"DogX",		"player[currentPlayer].Player->pos.x ",
						"DogZ",		"player[currentPlayer].Player->pos.z ",
						"DogY",		"player[currentPlayer].Player->pos.y ",
						"OLDDogX",		"player[currentPlayer].Player->oldPos.x ",
						"OLDDogY",		"player[currentPlayer].Player->oldPos.y ",
						"OLDOLDDogX",		"player[currentPlayer].Player->oldOldPos.x ",
						"OLDOLDDogY",		"player[currentPlayer].Player->oldOldPos.y ",
						"HoldDogX",		"HoldPlayerMain->pos.x ",
						"HoldDogZ",		"HoldPlayerMain->pos.z ",
						"HoldDogY",		"HoldPlayerMain->pos.y ",
						"RelVelX",		"strat->relVel.x",
						"RelVelY",		"strat->relVel.y",
						"RelVelZ",		"strat->relVel.z",
						"VelX",		"strat->vel.x",
						"VelY",		"strat->vel.y",
						"VelZ",		"strat->vel.z",
						"Radius",	"strat->pnode->radius",
						"CamX",		"player[currentPlayer].CameraStrat->pos.x ",
						"CamZ",		"player[currentPlayer].CameraStrat->pos.z ",
						"CamY",		"player[currentPlayer].CameraStrat->pos.y ",
						"ParentCamX", "strat->parent->Player->CameraStrat->pos.x ",
						"ParentCamY", "strat->parent->Player->CameraStrat->pos.y ",
						"ParentCamZ", "strat->parent->Player->CameraStrat->pos.z ",
						"CheckX",	"strat->CheckPos.x ",
						"CheckY",	"strat->CheckPos.y ",
						"CheckZ",	"strat->CheckPos.z ",
					  	"PlayerTarget", "player[currentPlayer].Player->target",
						"PlayerCheckX",	"player[currentPlayer].Player->CheckPos.x ",
						"PlayerCheckY",	"player[currentPlayer].Player->CheckPos.y ",
						"PlayerCheckZ",	"player[currentPlayer].Player->CheckPos.z ",
						"health",	"strat->health",
						"damage",	"strat->damage",
						"ParentDamage", "strat->parent->damage",
						"Lives",	"PlayerLives",
						"Skill",	"Skill",
						"LevelReset",	"LevelReset",
						"myRot",	"strat->m",
						"PlayerRot",	"player[currentPlayer].Player->m",
						"curway",	"strat->route->curway",
						"trans",	"strat->trans",
						"targetspeedy",	"strat->target->relVel.y",
						"ResetMyAnim","strat->anim->anim.frame=0",
						"MyAnimFrame","strat->anim->anim.frame",
						"MyAnimFlag",	"strat->anim->anim.flags",
						"GlobalVar", "globalVar",
					  	"GlobalVar2", "globalVar2",
						"ParentVar", "strat->parent->var",
						"ParentParentVar", "strat->parent->parent->var",
						"MyVar", "strat->var",
						"RRegionCentreX", "rRegionCentre.x",
						"RRegionCentreY", "rRegionCentre.y",
						"RRegionCentreZ", "rRegionCentre.z",
						"RRegionSqrRadius", "rRegionSqrRadius",
						"ParentPNode", "strat->parent->pnode",
						"ParentObjId", "strat->parent->objId",
					   	"ObjId", "strat->objId",
					  	"Obj", "strat->obj",
						"ParentObj", "strat->parent->obj",
						"PNode",	"strat->pnode",
						"MyPNODEIDS", "strat->pnode->noObjId",
						"mass",		"strat->pnode->mass",
						"ActX",		"GlobalActivationPos.x",
						"ActY",		"GlobalActivationPos.y",
						"ActZ",		"GlobalActivationPos.z",
						"ActR",		"GlobalActivationRadius",
						"ExplosionOn",		"ExplosionOn",
						"frame",		"strat->frame",
						"LastWayX",		"strat->route->path->waypointlist[strat->route->path->numwaypoints-1].x",
						"LastWayY",		"strat->route->path->waypointlist[strat->route->path->numwaypoints-1].y",
						"LastWayZ",		"strat->route->path->waypointlist[strat->route->path->numwaypoints-1].z",
						"PlayerState",	"player[currentPlayer].PlayerState",
						"JoyX",			"joyState[0].x",
						"JoyY",			"joyState[0].y",
						"PlayerM00",	"player[currentPlayer].Player->m.m[0][0]",
						"PlayerM01",	"player[currentPlayer].Player->m.m[0][1]",
						"PlayerM02",	"player[currentPlayer].Player->m.m[0][2]",
						"PlayerM10",	"player[currentPlayer].Player->m.m[1][0]",
						"PlayerM11",	"player[currentPlayer].Player->m.m[1][1]",
						"PlayerM12",	"player[currentPlayer].Player->m.m[1][2]",
						"PlayerM20",	"player[currentPlayer].Player->m.m[2][0]",
						"PlayerM21",	"player[currentPlayer].Player->m.m[2][1]",
						"PlayerM22",	"player[currentPlayer].Player->m.m[2][2]",
						"StrM00",	"strat->m.m[0][0]",
						"StrM01",	"strat->m.m[0][1]",
						"StrM02",	"strat->m.m[0][2]",
						"StrM10",	"strat->m.m[1][0]",
						"StrM11",	"strat->m.m[1][1]",
						"StrM12",	"strat->m.m[1][2]",
						"StrM20",	"strat->m.m[2][0]",
						"StrM21",	"strat->m.m[2][1]",
						"StrM22",	"strat->m.m[2][2]",
						"PlayerArmageddon", "strat->Player->PlayerArmageddon",
						"ParentPlayerArmageddon", "strat->parent->Player->PlayerArmageddon",
						"PlayerPuffTheMagicDragon", "strat->Player->PlayerPuffTheMagicDragon",
						"PlayerObj",	"PlayerObj",
						"CameraPos",	"cameraPos",
						"SmoothJoyX",	"player[currentPlayer].smoothJoyX",
						"SmoothJoyY",	"player[currentPlayer].smoothJoyY",
						"rotSpeed",		"strat->rot_speed",
						"PlayerRotSpeed",		"player[currentPlayer].Player->rot_speed",
						"PlayerAbsSpeedX", "player[0].Player->vel.x",
						"PlayerAbsSpeedY", "player[0].Player->vel.y",
						"PlayerAbsSpeedZ", "player[0].Player->vel.z",
						"PlayerSpeedY", "player[0].Player->relVel.y",
						"WallNX",		"strat->wallN.x",
						"WallNY",		"strat->wallN.y",
						"WallNZ",		"strat->wallN.z",
						"PlayerControlJoyX", "PlayerControlJoyX",
						"PlayerControlJoyY", "PlayerControlJoyY",
						"PlayerControlPadPress", "PlayerControlPadPress",
						"PlayerControlPlayerAcc", "PlayerControlPlayerAcc",
						"PlayerActVelX",	"player[0].PlayerOldVel.x",
						"PlayerActVelY",	"player[0].PlayerOldVel.y",
						"PlayerActVelZ",	"player[0].PlayerOldVel.z",
						"PlayerVelX",	"player[currentPlayer].Player->vel.x",
						"PlayerVelY",	"player[currentPlayer].Player->vel.y",
						"PlayerRelVelY",	"player[currentPlayer].Player->relVel.y",
						"PlayerVelZ",	"player[currentPlayer].Player->vel.z",
						"PlayerShieldEnergy", "player[0].PlayerShieldEnergy",
						"ParentShieldHold", "strat->parent->Player->ShieldHold",
						"ShieldHold", "strat->Player->ShieldHold",
						"HoldPlayerStrat", "HoldPlayerStrat",
						"windX", "windX",
						"windY", "windY",
						"RDFLWX", "strat->Player->rdWheel[0].p.x",
						"RDFLWY", "strat->Player->rdWheel[0].p.y",
						"RDFLWZ", "strat->Player->rdWheel[0].p.z",
						"RDFRWX", "strat->Player->rdWheel[1].p.x",
						"RDFRWY", "strat->Player->rdWheel[1].p.y",
						"RDFRWZ", "strat->Player->rdWheel[1].p.z",
						"RDRLWX", "strat->Player->rdWheel[2].p.x",
						"RDRLWY", "strat->Player->rdWheel[2].p.y",
						"RDRLWZ", "strat->Player->rdWheel[2].p.z",
						"RDRRWX", "strat->Player->rdWheel[3].p.x",
						"RDRRWY", "strat->Player->rdWheel[3].p.y",
						"RDRRWZ", "strat->Player->rdWheel[3].p.z",
						"RedDogFLS", "strat->Player->rdWheel[0].coll",
						"RedDogFRS", "strat->Player->rdWheel[1].coll",
						"RedDogRLS", "strat->Player->rdWheel[2].coll",
						"RedDogRRS", "strat->Player->rdWheel[3].coll",
						"spinflag","strat->spinFlag",
						"MyParentHealth","strat->parent->health",
						"PlayerHealth", "player[currentPlayer].Player->health",
						"CamLookX", "player[0].camLookCrntPos.x",
						"CamLookY", "player[0].camLookCrntPos.y",
						"CamLookZ", "player[0].camLookCrntPos.z",
						"adrelanin", "player[currentPlayer].PlayerEnergy",
						"Me", "strat",
						"PlayerWeaponPower", "player[0].PlayerWeaponPower",
						"PlayerWeaponType", "player[0].PlayerWeaponType",
						"Player","player[currentPlayer].Player",
						"OldPlayer","OldPlayer",
						"Score", "score",
						"SecondaryWeaponAmount", "strat->Player->PlayerSecondaryWeapon.amount",
						"ParentSecondaryWeaponAmount", "strat->parent->Player->PlayerSecondaryWeapon.amount",
						"SecondaryWeaponType", "strat->Player->PlayerSecondaryWeapon.type",
					  	"ParentSecondaryWeaponType","strat->parent->Player->PlayerSecondaryWeapon.type",
						"PlayerOnHoldPlayer",	"player[0].PlayerOnHoldPlayer ",
						"Multiplayer", "Multiplayer",
						"ParentPlayerNumber", "strat->parent->Player->n",
						"ParentPlayer", "strat->parent->Player",
						"LifeLost","PlayerLifeLost",
					  	"HitPlayerNumber", "strat->flag2 & 3",
						"Targetting", "strat->Player->targetting",
						"PlayerTargetTime", "strat->Player->playerTargetTime",
						"PlayerTargetNumber", "strat->Player->playerTargetNumber",
						"parentLeftGunX", "strat->parent->Player->leftGunPos.x",
						"parentLeftGunY", "strat->parent->Player->leftGunPos.y",
						"parentLeftGunZ", "strat->parent->Player->leftGunPos.z",
						"parentRightGunX", "strat->parent->Player->rightGunPos.x",
						"parentRightGunY", "strat->parent->Player->rightGunPos.y",
						"parentRightGunZ", "strat->parent->Player->rightGunPos.z",
						"parentTopGunX", "strat->parent->Player->topGunPos.x",
						"parentTopGunY", "strat->parent->Player->topGunPos.y",
						"parentTopGunZ", "strat->parent->Player->topGunPos.z",
						"parent", "strat->parent",
						"LeftGunPickUp", "player[0].leftGunPickup",
						"RightGunPickUp", "player[0].rightGunPickup",
						"TopGunPickUp", "player[0].topGunPickup",
						"GlobalArmageddon", "player[0].PlayerArmageddon",
						"BossStratFrame", "BossStrat->frame",
						"BossStrat","BossStrat",
						"BossVar","BossStrat->var",
						"BossFrame","BossStrat->frame",
						"BossRadius", "BossStrat->pnode->radius",
						"BossX","BossStrat->pos.x",
						"BossY","BossStrat->pos.y",
						"BossZ","BossStrat->pos.z",
						"PI2", "PI2",
						"PI", "PI",
						"PAL", "PAL",
						"NTSC", "(!PAL)",
						"DEATHMATCH", "DEATHMATCH",
						"KNOCKOUT", "KNOCKOUT",
						"TAG", "TAG",
						"SUICIDE_TAG", "SUICIDE_TAG",
						"PREDATOR", "PREDATOR",
						"KEEP_THE_FLAG", "KEEP_THE_FLAG",
						"KING_OF_THE_HILL", "KING_OF_THE_HILL",
						"NoSwarmTargets", "noSwarmTargets",
						"playerOffsetX", "player[0].playerOffsetX",
						"Cloaked", "strat->Player->cloaked",
					   	"IAMRedDogHud", "strat->parent->Player->RedDogHudStrat = strat",
						"IAmPlayerShield", "strat->parent->Player->shieldStrat = strat",
		  				"IAmOnFloor","(strat->flag & COLL_ON_FLOOR) || (strat->flag & STRAT_HITFLOOR)",
						"IAmDaBoss","BossStrat = strat",
						"BoatStrat", "BoatStrat",
						"IAmBossGuide", "BossGuide = strat",
					   	"IAmWaterSlide", "WaterSlide = strat",
					   	"IAmWaterSlideCamLook", "WaterSlideCamLookStrat = strat",
						"EscortTanker", "EscortTanker",
						"HITHDC", "STRAT2_HITHDC",
						"THIRD_PERSON_CLOSE", "THIRD_PERSON_CLOSE",
						"THIRD_PERSON_FAR", "THIRD_PERSON_FAR",
						"FIRST_PERSON", "FIRST_PERSON",
				   		"UnderWater","strat->flag & STRAT_UNDER_WATER",
						"CameraState", "player[0].cameraPos",
						"PlayerOnBoat", "player[0].onBoat",
						"HoldPlayerMain", "HoldPlayerMain",
						"BossGuideX", "BossGuide->pos.x",
						"BossGuideY", "BossGuide->pos.y",
						"BossGuideZ", "BossGuide->pos.z",
						"speedFactor", "strat->Player->speedFactor",
						"HoldDogAbsspeedx", "HoldPlayerMain->vel.x",
						"HoldDogAbsspeedy", "HoldPlayerMain->vel.y",
						"OBJECT_IN_LAVA" , "OBJECT_IN_LAVA",
						"OBJECT_NOFADE", "OBJECT_NOFADE",
						"ParentFrame",  "strat->parent->frame",
						"WaterSlideCamLookStrat", "WaterSlideCamLookStrat",
						"ArrowStrat", "ArrowStrat",
						"EscortTanker", "EscortTanker",
						"MyParentParent", "strat->parent->parent",
						"MyParentParentHealth", "strat->parent->parent->health",
						"MyParentParentTarget", "strat->parent->parent->target",
						"GlobalFloat0", "GlobalParamFloat[0]",
						"GlobalFloat1", "GlobalParamFloat[1]",
						"GlobalFloat2", "GlobalParamFloat[2]",
						"SpecialStrat", "SpecialStrat",
						"SpecialStratX", "SpecialStrat->pos.x",
						"SpecialStratY", "SpecialStrat->pos.y",
						"SpecialStratZ", "SpecialStrat->pos.z",
						"CollWith", "strat->CollWith",
						"MissionBriefingDone", "MissionBriefingDone", 
						"CollWithFlag", "strat->CollWith->flag",
						"CollWithFlag2", "strat->CollWith->flag2",
						"CollWithDamage", "strat->CollWith->damage",
						"rFlareB", "rFlareB",
						"CamTilt", "player[0].crntCamTilt",
						"EscortTankerSpeed", "EscortTankerSpeed",
						"DestX", "destinationPoint.x",
						"DestY", "destinationPoint.y",
						"DestZ", "destinationPoint.z",
						"fadeN", "fadeN[0]",
						"fadeLength", "fadeLength[0]",
						"fadeMode", "fadeMode[0]",
						"IceLiftGun", "iceLiftGun",
						"MissionFailure", "MissionFailure",
						"CollWithParent", "strat->CollWith->parent",
						"EngineHeat", "player[0].engineHeat",
						"BoatPAC", "boatPAC",
						"AimPointX", "player[0].aimedPoint.x",
						"AimPointY", "player[0].aimedPoint.y",
						"AimPointZ", "player[0].aimedPoint.z",
						"AimNormalX", "player[0].aimedNormal.x",
						"AimNormalY", "player[0].aimedNormal.y",
						"AimNormalZ", "player[0].aimedNormal.z",
						"AimStrt", "player[0].aimStrat",
						"PolyNX", "strat->cpn.x",
						"PolyNY", "strat->cpn.y",
						"PolyNZ", "strat->cpn.z",
						"NULL","NULL",
						"BeingBriefed", "BeingBriefed",
						"NoMainGun", "NoMainGun",
						"HoldPlayerVX", "HoldPlayerV.x",
						"HoldPlayerVY", "HoldPlayerV.y",
						"HoldPlayerVZ", "HoldPlayerV.z",
						"PlayerWeaponCharge", "player[0].PlayerWeaponCharge",
						"PlayerWeaponEnergy", "player[0].PlayerWeaponEnergy",
						"PolyType", "strat->polyType",
						"bad_cheat", "bad_cheat",
						"GameOverFlag", "GameOverFlag",
						"MyParentNum", "((int)(strat->parent))",
						"ParentCamIVar", "strat->parent->IVar[0]",
						"PlayerCameraState0", "PlayerCameraState[0]",
						"PlayerCameraState1", "PlayerCameraState[1]",
						"PlayerCameraState2", "PlayerCameraState[2]",
						"PlayerCameraState3", "PlayerCameraState[3]",
						"CameraState0", "player[0].cameraPos",
						"CameraState1", "player[1].cameraPos",
						"CameraState2", "player[2].cameraPos",
						"CameraState3", "player[3].cameraPos",
						"COLL_ON_WATER", "COLL_ON_WATER",
						"LevelNum", "LevelNum"

					};

#define NUMRES 380


/*************************************************************************/
/*************************************************************************/
/*************************************************************************/
/*************************************************************************/

//RESERVED PROCEDURE VARIABLES

typedef struct procedures
{
	char 	*symbol;			//KEY WORD TO CHECK FOR
	int 	params;			//NUMBER OF EXPECTED PARAMETERS IN THE ROUTINE
								//ADDITIONAL TO THE STRAT *
	int 	strat;			//FLAG TO SAY WHETHER THE STRAT * NEEDS TO BE PASSED

} PROCEDURES;


PROCEDURES ProcedureWords[] = {
						"MoveZ",				1, 1,
						"MoveX",				1, 1,
						"MoveY",				1, 1,
						"MoveXYZNow",			3, 1,
						"MoveXNoZ",				1, 1,
						"ScaleX",				1, 1,
						"ScaleY",				1, 1,
						"ScaleZ",				1, 1,
						"Roll",					1, 1,
						"Pitch",				1, 1,
						"Yaw",					1, 1,
						"TowardPlayer",			2, 1,
						"TowardTarget",			2, 1,
						"TowardPlayerPitch",	1, 1,
						"ObjectTowardPlayerXZ", 5, 1,
						"ObjectTowardPlayerXZVF", 6, 1,
						"ObjectTowardPlayerXZOffset", 8, 1,
						"ObjectTowardTargetXZ", 2, 1,
					 	"ObjectTowardTargetX",	2, 1,
						"ObjectTowardTargetZ",	2, 1,
						"TowardPlayerYaw",		1, 1,
						"TowardPos",			2, 1,
						"TowardPosPitch",		1, 1,
						"TowardPosYaw",			1, 1,
						"TowardWay",			2, 1,
						"TowardWayZ",			1, 1,
						"AwayWayZ",				1, 1,
						"TowardParent",			2, 1,
						"SetCheckPos",			3, 1,
						"SetCheckPosRel",		4, 1,
						"MoveToward",			4, 1,
						"NearCheckPos",			1, 1,
						"NearCheckPosZ",		1, 1,
						"NearPlayer",			1, 1,
						"NearPlayerXY",			1, 1,
						"NearTargetXY",			1, 1,
						"NearCheckPosXY",		1, 1,
						"NearPosXY",			3, 1,
						"Delete",				0, 1,
						"Flatten",				1, 1,
						"FlattenPlayer",		1, 0,
						"SetTarget",			3, 1,
						"SetPlayer",			0, 1,
						"SetCamera",			0, 1,
						"InitPath",				0, 1,
						"GetNextWay",			0, 1,
						"GetPrevWay",			0, 1,
						"FirstWay",				0, 1,
						"LastWay",				0, 1,
						"SeePlayer",			1, 1,
						"SeeTargetZ",			1, 1,
						"SeeWay",				1, 1,
						"SeeWayZ",				1, 1,
						"AwaySeeWayZ",			1, 1,
						"SeePlayerBelow",		1, 1,
						"SetNewParent",			0, 1,
						"Delta",				1, 0,
						"ANG",					1, 0,
						"Scos",					1, 0,
						"Ssin",					1, 0,
						"Satan",				1, 0,
						"rand",					0, 0,
						"CameraDrop",			0, 0,
						"CameraCollect",		0, 0,
						"GetPadPush",			1, 1,
						"GetPadPress",			1, 1,
						"Explosion",			2, 1,
//						"SetExpRot",			1, 1,
						"LNOT",					1, 0,
						"ResetRot",				0, 1,
						"MakeFrags",			2, 1,
						"AddOmniLight",			5, 1,
						"SetLightColour",		4, 1, 
						"SetLightDist",			3, 1,
						"ClearSplash",			0, 0,
						"SetNextSplash",		0, 0,
						"Splash",				0, 0,
						"GetSplashPos",			0, 1,
						"LastSplash",			0, 0,
						"Dead",					0, 1,
						"ResetMap",				0, 1,
						"PointPlayer",			0, 1,
						"RegisterHit",			0, 0,
						"GameOver",				0, 0,
						"CompletedLevel",		0, 0,
						"ReduceHealth",			1, 1,
						"AddExplosionLight",	5, 1,
						"MakeSplash",			2, 1,
						"MakeExpFrags",			2, 1,
						"PlayerDistXYBound",	1, 1,
						"SetCollisionDetail",	1, 1,
						"MoveBackOneFrame",		0, 1,
						"RandomPointTo",		0, 1,
						"RandomPointToCam",		0, 1,
						"GetNearestWay",		1, 1,
						"GetNearestWaySpline",	1, 1,
						"GetNearestWayToTarget",1, 1,
						"GetNearestCollideWay", 1, 1,
						"GetNearestCollideWayFound", 1, 1,
						"GetNearestCollideWayFoundToCheckPos", 2, 1,
						"GetNearestVisibleWay", 1, 1,
						"GetNearestWayToMe",	0, 1,
						"GetNearestVisibleWayToMe", 1, 1,
						"GetNearestNetPatrolPoint", 1, 1,
						"TargetStillValid",		0, 1,
						"GetFurthestWay",		1, 1,
						"ClearObj",				0, 1,
						"PointToTarget",		0, 1,
						"RemoveTarget",			2, 1,
						"RemoveTargetStrat",	3, 1,
						"ResetObject",			1, 1,
						"RotateObjectXYZ",		4, 1,
						"RotateStratObjectXYZ", 3, 1,
						"PlayerInRadius",		0, 1,
						"ApplyForcePlayerRel",	3, 0,
						"SetPlayerFriction",	3, 0,
						"SetFriction",			3, 1,
						"ApplyForcePlayer",		3, 0,
						"DeleteValidAreaEntry", 0, 1,
						"InsideArea",			0, 1,
						"HitWall",				1, 1,
					  //	"SetStrafePos",			4, 1,
						"IAmFloorStrat",		0, 1,
						"StratApplyForce",		0, 1,
						"RotationRestriction",	3, 1,
						"ObjectReturnToStart",	1, 1,
						"IAmBBox",				0, 1,
						"IAmWater",				2, 1,
						"GetNextNetWay",		1, 1,
						"ApplyRelForce",		7, 1,
						"LeaveFixedPath",		0, 1,
						"InitRotInit",			0, 1,
						"CollideStrat",			2, 1,
						"GetLockOn",			0, 1,
						"MakeTargets",			1, 0,
						"TargetNearPlayer",		1, 1,
						"FollowLockOn",			0, 1,
						"ClearBusy",			0, 1,
						"SetActivationPoint",	0, 1,
						"PlayerNearActivation", 1, 1,
						"NearActivation",		1, 1,
						"NearActivationXY",		1, 1,
						"PlayerNearActivationXY", 1, 1,
						"PlayerNearParentParentActivationXY", 1, 1,
						"PlayerNearParentActivationXY", 1, 1,
						"MPPlayerNearActivationXY", 1, 1,
						"MPPlayerNearActivation", 1, 1,
						"RandRange",			2, 0,
						"FABS",					1, 0,
						"IABS",					1, 0,
						"FSQRT",				1, 0,
						"SetCheckPosPlayerRotate", 1, 1,
						"SetCheckPosMyRotate", 1, 1,
						"RelTurnTowardXY",		5, 1,
						"RelTurnTowardZ",		4, 1,
						"ResetPath",			0, 1,
					   //	"DamagePlayer",			1, 0,
						"AvoidStrats",			1, 1,
						"TransObjectRel",		4, 1,
						"EnemyInFrontXY",		3, 1,
						"PlayerInFrontXY",		2, 1,
						"TransObjectAbs",		4, 1,
						"ApplyUpForce",			1, 1,
						"LineSightPlayer",		0, 1,
						"LineSightParent",		0, 1,
						"FireSightPlayer",		0, 1,
						"FireSightTarget",		0, 1,
						"SeePlayerZ",			1, 1,
						"SeePlayerZRel",		1, 1,
						"RelTurnTowardCheckPosXY", 1, 1,
						"RelTurnAwayCheckPosXY", 1, 1,
						"RelTurnTowardPlayerXY", 1, 1,
						"RollTowardPlayer", 1, 1,
						"RelTurnTowardCheckPosSmoothXY", 1, 1,
						"RelRotate",			3, 1,
						"RelRotateX",			1, 1,
						"RelRotateY",			1, 1,
						"RelRotateZ",			1, 1,
						"WayPointX",			1, 1,
						"WayPointY",			1, 1,
				 //		"CheckDistXY",			0, 1,
						"PlayerDistPlayerCheckXY", 0, 0,
						"InheritMyParent",		0, 1,
						"MoveTowardXY",			3, 1,
						"StickToParent",		5, 1,
						"PointTo",				3, 1,
						"PointToXY",			3, 1,
						"OnlyInsideArea",		2, 1,
						"LineOfSight",			7, 0,
						"ObjectHitStrat",		1, 1,
						"ObjectHitFloor",		1, 1,
						"PlayerObjectHitFloor", 1, 0,
						"ClearRotFlag",			0, 1,
						"SetRotFlag",			0, 1,
						"RemoveObject",			1, 1,
						"NoCollObject",			1, 1,
						"StratXCheckPos",		0, 1,
						"Delay",				1, 0,
						"GetTargetProxim",		1, 1,
						"ClearBusy",			0, 1,
						"GetActivationPos",		1, 1,
						"GetActivationRadius",	1, 1,
						"GetTargetCount",		1, 0,
						"GetSecondaryTargetStrat", 1, 1,
						"GetTargetStratPos",	3, 1,
						"RemoveTarget",			1, 0,
						"ObjectNotTargetable",	1, 1,
						"ObjectTargetable",		1, 1,
						"FragAtObject",			1, 1,
						"HideObject",			1, 1,
						"UnhideObject",			1, 1,
						"MakeTrailFrags",		5, 1,
						"OnCamera",				3, 1,
						"CamSetPos",			0, 1,			//NEW POST SH4
						"LogCam",				0, 1,			//NEW POST SH4
						"ApplyVelToPlayer",		0, 1,
						"ApplyRotToPlayer",		0, 1,
						"InViewAngle",			1, 1,
						"PushRedDogAlongPath",	1, 1,
						"AccelerateTowardsCheckPos", 1, 1,
						"TurnTowardDirectionOfMotion", 1, 1,
						"HoldPlayerPos",		4, 1,
						"HoldPlayerRot",		4, 1,
						"MoveAlongPath",		1, 1, 
						"MoveAlongPathXY",		1, 1, 
						"FaceAlongPath",		1, 1, 
						"InitSplineData",		0, 1,
						"TiltAlongPath",		1, 1,
						"RelFlatten",			1, 1,
						"SplineRelFlatten",		1, 1,
						"FlattenToPoly",		1, 1,
						"FlattenToVector",		4, 1, 
						"PickCheckPosStratToPlayerAngleOffset", 1, 1,
						"MoveTowardCheckPosXY", 1, 1,
						"MoveTowardCheckPos",	1, 1,
						"MoveTowardPlayerXY",	1, 1,
						"ValidatePosition",		1, 1,
						"PlayerOutSide",		0, 1,
						"StratOutSide",			1, 1,
						"DeleteMyParent",		0, 1,
					 //	"DeleteMyParentParent",		0, 1,
						"PlayerDistXY",			0, 1,
						"ParentDistXY",			0, 1,
						"AvoidPlayer",			1, 1,
						"NearParentXY",			1, 1,
						"ProjectileAngle",		8, 1,
						"CrntRotToIdleRotX",	2, 1,
						"CrntRotToIdleRotY",	2, 1,
						"CrntRotToIdleRotZ",	2, 1,
						"ReturnToStartTransform",1,1,
						"AllocStratSoundBlock", 1, 1,
						"PlaySound",			7, 1,
						"StopSound",			2, 1,
						"SetVolume",			2, 1,
						"SetPitch",				2, 1,
						"FacePlayerXY",			1, 1,
						"FacePosXY",			1, 1,
						"HasActivation",		1, 1,
						"PauseTriggers",		2, 1,
						"UnPauseTriggers",		0, 1,
						"LineShot",				7, 1,
						"ObjectTowardPlayerOffsetXZ", 5, 1,
					   //	"FindValidCheckPosXY", 2, 1,
						"IAmHoldPlayer",		0, 1,
						"IAmHoldPlayerMain",	0, 1,
						"HoldPlayer",			0, 1,
						"PullPlayer",			1, 1,
						"XRelPosX",				1, 1,
						"XRelPosY",				1, 1,
						"FindValidPoint",		2, 1,
						"FacePlayerWaterSlide", 2, 1,
						"KillMyVelocity",		0, 1,
						"UpdateGameMapEntry",   0,1,
						"AvoidSurround",		1, 1,
						"PlayerAvoidSurround",	0, 1,
						"InSight",				0, 1,
						"GetNearestCampPoint",	1, 1,
						"MakeConeFrags",		3, 1,
						"GetConeAmount",		0, 1, 
						"GetFreeCollisionSpace", 2, 1, 
						"ClearLastStratCollision", 0, 1,
						"MakeMeTheLast",		0,1,
						"CollideStratNew",		1, 1,
						"ApplyAbsForce",		7, 1,
						"FaceAlongDirXY",		1, 1,
						"FaceAlongDir",			0, 1,
						"PlayerNearPlayerCheckPosXY", 1, 0,
						"AnyFreeTargets",		0, 1,
						"RegisterAsTarget",		1, 1,
						"UnRegisterAsTarget",	0, 1,
						"SaveTarget",			0, 1,
						"RestoreTarget",		1, 1,
						"PlayerTurnTowardPlayerCheckPosXY", 0, 0,
						"PlayerSeePlayerCheckPosXY", 1, 0,
						"CreateBand",			11, 1,
						"DeleteBand",			1, 1,
						"RubberBand",			5, 1,
						"AddBandSegment",		5, 1,
						"AnyFreeJonnies",		2, 0,
						"PlayerObjectRelRotateXYZ", 4, 0,
						"SetBandColour",		9, 0,
						"SetBandRotXYZandScaling", 7, 0,
						"PlayerRelRotateXYZ",	3, 0,
						"HoldPlayerHitFloor",	0, 0,
						"DisplayScript",		6, 0,
						"GetScriptEntry",		2, 0,
						"GetScriptFlag",		1, 0,
						"GetScriptTime",		1, 0,
						"GetScriptLines",		1, 0,
						"FlattenHoldPlayer",	1, 1,
						"HoldPlayerRelRotateXYZ", 3, 1,
						"MakeSpark",			3, 1,
						"CreateTrail",			10, 1,
						"UpdateTrail",			4, 1, 
						"GetParentObjectPos",	1, 1,
						"DeleteTrail",			1, 1,
						"ChangingState",		0, 0,
						"MyParentInvalid",		0, 1,
						"MyParentParentInvalid", 0, 1,
						"ChangeCamStrat",		0, 1,
						"ChangeCam",			2, 0,
						"PauseAllStrats",		1, 1,
						"UnPauseAllStrats",		1, 1,
						"SetPosRelOffsetPlayerObject", 5, 1,
						"SetCheckPosRelOffsetPlayerObject", 4, 1,
						"RelPosToCheck",		4, 1,
						"WorldPosToCheck",		4, 1,
						"RelToAbsPosCheck",		3, 1,
						"UpdateTrigFlag",		1, 0,
						"LinkToParentObject",	7, 1,
						"IAmJumpPoint",			0, 1,
						"DirectionPitch",		1, 1,
						"TOINT",				1, 0,
						"RecordCollision",		1, 1,
						"AddDecal",				7, 1,
						"SetObjectScale",		4, 1,
						"SetObjectAnimScale",		4, 1,
						"SeeCheckPosLateral",	1, 1,
						"UnitiseMatrix",		0, 1,
						"GetShieldVertexNumber", 0, 0,
						"AccelerateTowardsShield", 1, 2,
						"SetObjectTrans",		2, 1,
						"RedDogSkid",			0, 1,
						"IAmTheSun",			0, 1,
						"SetRainAmount",		3, 1,
						"FirstSplineSection",	0, 1,
						"LastSplineSection",	0, 1,
						"distToFirstWay",		0, 1,
						"distToLastWay",		0, 1,
					   	"StopTheStrats",        0, 1,
						"SetVisBit",			1, 0,
						"ClearVisBit",			1, 0,
						"SetVisBit",			1, 0,
					   	"GetObjectCrntRot",	2, 1,
					   	"GetSpecularColour",	2, 1,
					   	"GetObjectCrntPos",	2, 1,
						"GetObjectAnimScale",2,1,
						"GetObjectCrntScale",2,1,
					   	"SetObjectCrntRot",	3, 1,
						"SetObjectIdleRot",	3, 1,
					   	"SetObjectCrntPos",	3, 1,
					   	"GetObjectIdleRot",	2, 1,
					   	"GetObjectIdlePos",	2, 1,
						"DrawLightning",		6, 1,
						"SetAnimSpeedPercent",	1, 1,
						"SmoothFromTo",			3, 0,
						"SmoothFromToSum",		3, 0,
						"BattleTankInActivation", 1, 1,
						"DrawArrow",			3, 1,
						"VisibleFromPlayer",	1, 1,
					//	"WorldToStrat",			3, 1,
						"MyStratToWorld",			3, 1,
						"GetWaterSlideRelRotZ", 0, 1,
						"PDist",				6, 0,
						"StratSpeed",			0, 1,
						"TurnTowardPosXZ",		5, 1,
						"Overlay",				2, 0,
						"GetScriptCharacter",	1, 0,
						"InitTimer",			2, 0,
						"HideTimer",			0, 0,
						"DrawStratBar",			4, 0,
						"DrawProgressBar",		2, 1,
						"SetPlayerToTarget",	1, 1,
						"SetPlayerBack",		0, 1,
						"WatNoise",				2, 0,
						"RegisterCollision",	0, 1,
						"PickMeUp",				2, 1,
						"GetFireSecondary",		1, 0,
						"SetFireSecondary",		2, 0,
						"RegisterSpawnPoint",	1, 1,
						"SetSpawnActivity",		2, 0,
						"FindDeadPlayer",		0, 0,
						"SignalSpawnPoint",		1, 0,
						"NearAnyPlayer",		1, 1,
						"GetParentPlayerNumber", 0, 1,
						"GetPlayerNumber", 0, 1,
						"ClearPlayerTargetArray",	1, 0,
						"CheckEndGameStatus",	0, 0,
						"ValidTargetStrat",		3, 1,
						"RemoveMeFromAllTargets", 0, 1,
						"HealthOfPlayer",		1, 0,
						"TargettingStrat",		2, 0,
						"AddWaterStrat",		0, 1,
						"AddLavaStrat",			0, 1,
						"FacePlayerAim",		1, 1,
						"GetPlayerStrat",		1, 1,
						"ObjectSeePlayerZ",		5, 1,
						"ResetOCP",				0, 1,
						"SetObjScale",			4, 1,
						"Vibrate",				4, 1,
						"IAmAimStrat",			0, 1,
						"ActivateObjectCollision", 1, 1,
						"InActivateObjectCollision", 1, 1,
						"InActivateObjectDamageable", 1, 1,
						"InitHDC",				0, 1,
						"CalculateStratPolyData", 1, 1,
						"CheckPosDist",			0, 1,
						"DrawShock",			9, 0,
						"DrawShock2",			10, 0,
						"ObjectShotBySibling",	0, 1,
						"SetObjectShadowState", 1, 1,
						"SetGlobalParamFloat", 2, 0,
						"SetGlobalParamInt", 2, 0,
						"GetGlobalParamFloat", 1, 0,
						"GetGlobalParamInt", 1, 0,
						"ClearGlobalParams", 0, 0,
						"MoveAllStratsBack", 1, 1,
						"LWaterSlide", 1, 1,
						"SetSem", 2, 1,
						"GetSem", 1, 1,
						"SetParentSem", 2, 1,
						"GetParentSem", 1, 1,
						"AddSem", 2, 1,
						"SubSem", 2, 1,
						"AddParentSem", 2, 1,
						"SubParentSem", 2, 1,
						"PNum", 3, 0,
						"LogPrint",3,0,
						"GetObjectHealth", 1, 1,
						"SetObjectHealth", 2, 1,
						"PNODESetInitToCrnt",1,1,
						"MPGameType", 0, 0,
						"PickupAllowed", 1, 0,
						"IncShotsFired", 0, 1,
						"ThonGravity", 2, 0,
						"PlayerKillSelf", 0, 1,
						"RedDogElectroShock", 0, 1,
						"RedDogElectroShockMultiPlayer", 0, 1,
						"DrawBeam",0, 1,
						"GetPlayerPos", 1, 1,
						"SetHealthOfPlayer", 2, 0,
						"SetPlayerZoom", 2, 1,
						"GetPlayerZoom", 1, 1,
						"SetPlayerBoostButtonCount", 2, 0,
						"PlayerSpeed", 1, 0,
						"ClearObjCollFlag",2,1,
						"SetPlayerAcc", 2, 0,
						"DistToAim", 1, 1,
						"DamageAim", 2, 1,
						"SetPlayerArmageddon", 2, 0,
						"GetPlayerArmageddon", 1, 0,
						"GetSecondaryWeaponAmount", 1, 0,
						"FreeHDBlock", 0, 1,
						"SetSecondaryWeaponAmount", 2, 0,
						"SetSecondaryWeaponType", 2, 0,
						"GetSecondaryWeaponType", 1, 0,
						"FindAimPoint", 2, 1,
						"SmartBombExplode", 3, 1,
						"AddStratToSwarmTargetArray",1 , 1,
						"ClearSwarmTargetArray",0 ,0,
						"GetSwarmTarget", 1, 1,
						"ValidSwarmTarget", 1, 1,
						"GetSwarmTargetPos", 1, 1,
						"FiredByAssassin", 0, 1,
						"ShallISpawnAFlag", 0, 0,
						"GiveFlagToPlayer", 1, 0,
						"IAmFlag", 1, 1,
						"AddScreenToTargetArray", 1, 0,
						"DrawTargetAt", 3, 0,
						"ClearTargetArray", 1, 0,
						"StormingShield", 0, 1,
						"StormingShieldHit", 0, 1,
						"ThisModelAnimPlay",1,1,
						"GetHomingBulletTarget", 1, 0,
						"SetCheckPosToTarget", 1, 1,
						"BossPlatTilt",0,1,
						"SetPlayerOCP", 0, 0,
						"GetLastWay", 0, 1,
						"GetFirstWay", 0, 1,
						"DrawTargetOnObject", 1, 1,
						"BeamPlayerShieldColl", 3, 1,
						"BeamPlayerColl", 3, 1,
						"ObjectPlayerShieldDist", 2, 1,
						"DrawVLine", 1, 1,
						"ObjectPlayerDist", 2, 1,
						"DrawLensFlare", 8, 1,
						"uId", 1, 0,
						"SetPlayerMaxHealth", 2, 0,
						"Infron tOfObject", 2, 1,
						"AdjustAmbient", 1, 0,
						"AdjustRedDogAmbient", 1, 0,
						"GetPlayerColour", 1, 1,
						"Range", 3, 0,
						"DrawAimLine", 1, 0,
						"WaterSlideCamLookDist", 0, 1,
						"SetSpecularColour", 4, 1,
						"ObjectSpecularFlash", 2, 1,
						"GetObjectFlag", 1, 1,
						"SetObjectFlag", 2, 1,
						"StratInPlayerCone", 2, 1,
						"DontWhiteMeOut", 0, 1,
						"DontFogMeOut", 0, 1,
						"SetWhiteOut", 3, 0,
						"BeamHit",5,1,
						"SetEnvironmentParms", 7, 0,
						"DisplayBriefing", 3, 0,
						"TimeFunction", 1, 0,
						"DrawArrowTo", 7, 1,
						"AddParentToArrowArray", 0, 1,
						"InitArrowArray", 0, 0,
						"UpdateArrowArray", 0, 1,
						"SparkSuck",5,1,
						"RemoveParentFromArrowArray", 0, 1,
						"BossDarkness", 1, 0,
						"StrtNoFade", 0, 1,
						"ValidToBeam", 4, 1,
						"ProcessLightning", 1, 0,
						"InfrontOfObject", 2, 1,
						"UpdateObjectCollFlag", 2,1,
						"ObjectNoCull", 1, 1,
					   //	"EnemyInActivationXY", 1, 1,
						"RadarUpdate",1,1,
						"MoveXFlat", 1, 1,
						"EnemyInActivation", 2, 1,
						"MoveYFlat", 1, 1,
				  		"RegisterDanger",1,1,
						"CopyParentAnim",0,1,
						"GetParentStratId", 0, 1,
						"GetStratId", 0, 1,
						"GetPlayerId", 1, 1,
						"AnimMoveOffset", 1, 1,
						"SparkSuckPos",8,1,
						"TrigCheckMe",0, 1,
						"ParentNearActivationXY",		1, 1,
						"SetOnCamera", 6, 1,
						"AddToAvoidStratArray", 0, 1,
						"RemoveFromAvoidStratArray", 0, 1,
						"ObjectNotSolid", 1, 1,
						"MakeWaterRipple", 2, 1,
					   //	"RegisterInSiblingArray", 0, 1,
					   //	"ClearSiblingArray", 0, 0,
					   //	"GetNumSiblings", 0, 0,
					   //	"GetSibling", 1, 0,
					  //	"PollSiblingArray", 0, 0,
						"PackColour", 3, 0, 
						"Line", 8, 0,
						"ObjectHitPlayer", 1, 1,
						"TexturedLine", 9, 1,
						"PlayerShieldStrat", 1, 0,
						"BounceOffShield", 1, 1,
						"MakePointLight", 8, 1,
						"RemoveLight", 1, 0,
						"SetPointLightRadius", 3, 0,
						"UpdateLight", 4, 1,
						"GetGunDir", 1, 1,
						"DoSpark", 10, 1,
						"ExplosionLight", 6, 1,
						"SetPlayerMatrix", 0, 1,
						"SetPlayerCamera", 0, 1,
						"RadiusDamage", 2, 1,
						"hasFadeFinished", 0, 0,
						"FadeOut", 0, 0,
						"SetPlayerEnergy", 2, 0,
						"GetPlayerEnergy", 1, 0,
						"GetUniqueID", 0, 0,
						"GetCurrentFrame", 0, 0,
						"OnScreen", 0, 1,
						"Obscured", 0, 1,
						"DrawFlare", 1, 1,
						"SetPlayerCamTilt", 2, 0,
						"GetPlayerCamTilt", 1, 0,
						"SetScreenARGB", 4, 0,
						"SoundPlayTrack", 1, 0,
						"SoundStopTrack", 0, 0,
						"MusicFade", 1, 0,
						"SoundFade", 2, 0,
						"RestartEnvParams", 1, 0,
						"PlayerIsAssassin", 1, 0,
						"EscortTankerInActivationXY", 1, 1,
						"StrtNoCamColl", 0, 1,
						"DrawAllTags", 0, 0,
					//	"SnapToAnimPos",2,1,
						"SetCamSimple", 0, 1,
						"DistToLastWayXY", 1, 1,
				   //		"SqrDistToLastWayXY", 0, 1,
						"MoveZRel", 1, 1,
						"GetNearestTagToPlayer", 0, 1,
						"CamSetNow", 0, 0,
						"PlayerReward", 2, 0,
						"MoveTowardCheckPosNow", 1, 1,
						"IsPlaying", 1, 1,
						"SetAudibleRange", 3, 1,
						"GetBoatPA", 1, 1,
						"SetStratMatrixVector", 3, 1,
						"IsChallengeLevel", 0, 0,
						"MusicSetIntensity", 1, 0,
						"DontDieJustSitAround", 0, 1,
						"CanPickup", 3, 1,
						"DoCred", 1, 0,
						"InitCred", 0, 0,
						"FinaliseGameScore", 0, 0,
						"sfxGetCDVolume", 0, 0,
						"sfxSetCDVolume",1,0,
						"KingOfTheHillGame", 0, 0,
						"MaxHealthOfPlayer",	1, 0,
						"IAmPlayerRDHM", 1, 1,
						"ValidStrat", 1, 0,
						"SoundReset", 0, 0
					};
#define NUMPROC 603


/*************************************************************************/
/*************************************************************************/
/*************************************************************************/
/*************************************************************************/

//		VALID OPERATIONS 
//	(SET 2 IS FOR REPLACING ASSIGNMENT SYMBOLS, (WHERE = BECOMES =)


RESERVED Operations[] = {
						"+","+ ",
						"-","- ",
						"*","* ",
						"/","/ ",
						"or","| ",
						"OR","|| ",
						"and","& ",
						"not","! ",
						"is","& ",
						"AND","&& ",
						"<","< ",
						"=","==",
						">","> ",
						">=",">= ",
						"<=","<= ",
						"<>","!= ",
						"!=","!= ",
						"||","|| ",
						"&&","&& ",
						"&","& ",
						",",", ",
						"|","| ",
						"%","% ",
									};


RESERVED Operations2[] = {
						"+","+ ",
						"-","- ",
						"*","* ",
						"/","/ ",
						"or","| ",
						"OR","|| ",
						"and","& ",
						"not","! ",
						"is","& ",
						"AND","&& ",
						"<","< ",
						"=","=",
						">","> ",
						">=",">= ",
						"<=","<= ",
						"<>","!= ",
						"!=","!= ",
						"||","|| ",
						"&&","&& ",
						"&","& ",
						",",", ",
						"|","| ",
						"%","% ",
									};


#define NUMOPER 23

// STRUCTURE DEFS

typedef struct scriptline
{
	struct scriptline *prev;
	char 	 *text;
	struct scriptline *next;
}	SCRIPTLINE;

typedef struct state
{
  	SCRIPTLINE 	*line;			//NEXT LINE OF THE PCODE TO ACCESS	
  	char 			*state;	
	int			Internal;
	int			stack[30];
	int			indent;
} STATE;

typedef struct definition
{
 	char *definition;
	char *value;
}	DEFINITION;

typedef struct modrec
{
	char *Name;
	int	 used;
}	MODREC;

typedef struct soundrec
{
	char *Name;
	int	 used;
}	SOUNDREC;

typedef struct animrec
{
	char *Name;
	char *ModelName;
	int	 used;
}	ANIMREC;


typedef struct stratfield
{
	char	*name;
	char	*addr;
	int		floatnum;
	int		intnum;
	int		currentlevel;
}STRATFIELD;

//internal procedures

void 		PushLine();
void		PushLine2();
void		WriteHeader(FILE *scriptfile);
void		HeaderEnd(FILE *fp);
void		error(int type,char *line,int number);
void 		ReadToEnd(char *dest,char*src);
void 		ReadToEndUpper(char *dest,char*src);
void 		AddLabel(char *cursor);
void 		CopyVal(char *dest,char *cursor);
void		IncludeStart(FILE *fp,char *Level);
void		WriteScript(FILE *scriptfile,SCRIPTLINE *start);
void		Clear(char *input,int amount);
void		HeaderStart(FILE *fp);
void		CleanScript(SCRIPTLINE *start);
void		StateDefine();
void		StatementBlockStructure(char *scriptstring, char *tab);
void		SetState (char *cursor);
void		InsertCondition(int RefState);
void		InsertTrigger(int RefState,char *cursor);
void		RemoveTrigger(int RefState,char *cursor);
void		HoldTrigger(int RefState,char *cursor);
void		ReleaseTrigger(int RefState,char *cursor);
void		EndTrigger();
void		FinishTrigger();
void		ResetTrigger();

void		InsertLoop(char *cursor);
void		InsertLoopImmediate(char *cursor);
void		InsertModelChange(char *cursor);
void		CloseLoop(char *cursor);
void		CloseLoopImm(char *cursor);
void		CloseWhile(char *cursor);
void		CloseIf(char *cursor);
void		CloseElse(char *cursor);
void		Indentation(char *tab);
void		AddLine(char *linetext);
void		ResetSwitches();
void		output(int type,char *line);
void		GetLine();
void		GetLine2();
void		IfLabelCheck();
void		ParseSpawn(char *cursor, int mode);
void		ParseExplode(char *cursor);
void		CleanModel();
void		CleanAnim();
void		CleanSound();
int			GetAnimIndex(char *model);
int			GetModelIndex(char *model);
int			GetSoundIndex(char *sfx);
int			ParseExpression(char *out, char *in,int Mode);
int			ProcessModel(char *line);
int			NumericCheck(char *symbol);
int			NumericIntCheck(char *symbol);
int			NumericFloatCheck(char *symbol);
int			InsertEvaluation(char *cursor);
int			InsertPrint(char *cursor);
int			Interpret(int ParseMode,int token,char *line_tail,char*line,int linenum);
int			CheckLabel(char *cursor);
int 		AddDefinition(char *symbol,char *cursor);
int			AddVar(char *cursor, int mode);
int 		PopLine();
int			InsertWhile(char *cursor);
int			InsertIf(char *cursor);
int			StateUnBalanced(char *line_tail,int linenum);
int 		AddDefine(char *cursor);
char 		*SymbolGet(char *symbol,char *in);
char 		*Numeric(char *cursor, char *line);
char  		*WhiteSpaceRead(char *cursor);
char  		*LeadingSpace(char *cursor, char *line);
char 		*StateLabelParse(char *cursor);
char		*IntVarInsert(char *out,int varnum);
char		*FloatVarInsert(char *out,int varnum);
char		*Copy (char *out, char *in, int size);
FILE 		*OpenLog(char *filename);
FILE 		*OpenScript(char *filename);
FILE 		*OpenHeader(char *filename);
SCRIPTLINE 	*InitScript();
void	ParsePickup(char *cursor,int mode);


#endif
