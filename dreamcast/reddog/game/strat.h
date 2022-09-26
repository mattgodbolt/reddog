#ifndef _STRAT_H
#define _STRAT_H

/*
 * Structures have moved to StratBlock.h
 */
//#include "StratBlock.h"
#define THIRD_PERSON_CLOSE 0
#define THIRD_PERSON_FAR 1
#define FIRST_PERSON 2
#define STATIC_PERSON 3
#define CAMERA_HELD	4 
#define MAX_SKID	128
#define MAX_TARGETS 32
#define MAX_PLAYER	4

#define CHEAT_PERMANENT_BOOST (1<<0)
#define CHEAT_INFINITE_SHIELD (1<<1)
#define CHEAT_INVUL (1<<6)
#define UPGRADE_ARMOUR_1 (1<<2)
#define UPGRADE_ARMOUR_2 (1<<3)
#define UPGRADE_MISSILE (1<<4)
#define UPGRADE_SHIELD (1<<5)
#define CHEAT_INF_CHARGE (1<<7)
#define CHEAT_BOT_1 (1<<8)
#define CHEAT_BOT_2 (1<<9)
#define SPECIAL_WEAP_SHELL			(1<<18)
#define SPECIAL_WEAP_CHARGE			(1<<19)
#define SPECIAL_WEAP_HOMING			(1<<20)
#define SPECIAL_WEAP_LOCKON			(1<<21)
#define SPECIAL_WEAP_SWARM			(1<<22)
#define SPECIAL_WEAP_ULTRALASER		(1<<23)
#define SPECIAL_WEAP_VULCAN			(1<<24)
#define SPECIAL_WEAP_ALL			(SPECIAL_WEAP_SHELL | SPECIAL_WEAP_CHARGE | SPECIAL_WEAP_HOMING | SPECIAL_WEAP_LOCKON | SPECIAL_WEAP_SWARM | SPECIAL_WEAP_ULTRALASER | SPECIAL_WEAP_VULCAN)
#define UPGRADE_ARMOUR_1_HEALTH 120.0
#define UPGRADE_ARMOUR_2_HEALTH 140.0
#define SHIELD_CUTOFF (Multiplayer?0.25f : ((player[0].reward & UPGRADE_SHIELD) ? 0.2f : 0.5f))

/*collision material flags 0 to 31 are usable*/
#define	ROCK			0
#define	GRASS			1
#define	METAL			2
#define	MUD				3
#define	SAND			4
#define	LOOPTHELOOP		5
#define	ICE				6
#define	WATER			7
#define SNOW			8
#define REALGRAVITY		9
#define WATERSLIDEWATER	10
#define SPIDERMAN		11
#define KOTH			12
#define DANGER			13
#define NODECAL			14
#define WALLOFDEATH		15
#define AIR				16


/*
 * Fade type
 */
typedef enum 
{	
	FADE_NONE, 
	FADE_TO_BLACK, 
	FADE_FROM_BLACK,
	FADE_TO_BLACK_FAST, 
	FADE_FROM_BLACK_FAST,
	FADE_TO_WHITE, 
	FADE_FROM_WHITE,
	FADE_TO_WHITE_FAST, 
	FADE_FROM_WHITE_FAST,
	FADE_SPIN_OUT,
	FADE_JET_BLACK,
	FADE_MAX
} FadeType;


typedef struct
{
	Matrix	m;
	Vector3	n, oldp_p, vel, oldvel;
	float	t, ang;  
	int		lineSeg;
	Point3	p, oldp;
}SplineData;

// The number of selectable tanks
#define MAX_TANK	4

typedef struct tagSecondaryWeapon
{
	int	type;
	int	amount;
}SecondaryWeapon;



typedef struct
{
	Point3		p;						//current collision Point
	Vector3		n;						//normal to current collision Poly
	StripPos	*v;						//use to be skidArray
	Sint8		stripStart[MAX_SKID];	//used to be S (start of strip or not
	Uint32		uv[MAX_SKID];			
	int			i;						//used to be currentSkid
	Uint8		coll;					//Used to be RedDogS
	int			mat;					//current poly Material type
	int			oldMat;					//old Material
}RedDogWheel;

typedef struct
{
	float		time;
	struct strat		*strat;
	Object		*obj;
	Uint16		n, locked;
}TARGET;


typedef	struct	targets
{
	struct  strat	*strat;
	struct	targets *next; 
	struct	targets *prev;
} TARGETS;

typedef struct 
{
	bool	active;	
	float	DoPlayerFriction;
	float	PlayerAcc;
	float	PlayerShieldEnergy;
	float	PlayerWeaponEnergy;
	float	PlayerWeaponCharge;
	float	smoothJoyY;
	float	smoothJoyX;
	float	smooothJoyY;
	float	smooothJoyX;
	float	PlayerHeldCameraZAng;
	float	jumpCamXSpeed;
	float	jumpCamYSpeed;
	float	jumpCamZSpeed;
	float	rollAng;
	float	crntCamTilt;
	float	idleCamTilt;
	float	ThonGravity;
	float	zooming;
	float	playerOffsetX;
	float	speedFactor;
	int		PlayerHeld;
	
	int		n;
	int		PlayerState;
	int		playerRevBoostButtonHold;
	int		playerRevBoostButtonRelease;
	int		playerRevBoostButtonCount;
	int		playerBoostButtonHold;
	int		playerBoostButtonRelease;
	int		playerBoostButtonCount;
	int		playerSideStepHold;
	int		playerSlide;
	int		playerSkidTurn;
	int		playerSkidDir;
	int		PlayerLoop;
	int		PlayerWeaponType;
	int		PlayerWeaponPower;
	int		playerSideStep;
	int		PlayerBreaking;
	int		PlayerWheelSpin;
	int		playerBoostStopTime;
	int		aimType;
	int		aimLine;
	int		cameraPos;
	int		PlayerAirTime;
	int		targetting;
	int		playerFireButtonHold;
	int		playerBButtonHold;
	int		fireSecondary;
	int		PlayerArmageddon;
	int		PlayerPuffTheMagicDragon;
	int		PlayerAfterJumpCountDown;
	int		shieldValid;
	int		ShieldHold;
	int		currentTarget;
	int		padDown;
	int		padLeft;
	int		lockedOnTime[4];
	int		playerTargetTime;
	int		playerTargetNumber;
	int		lostTargetTime;
	int		playerTargettedbyMissile;
	int		leftGunPickup;
	int		rightGunPickup;
	int		topGunPickup;
	int		currentMainGun;
	struct strat *rdhm;
	int		onBoat;
	int		gunDischarged;
	int		aimedPolyType;
	int		oldPolyType;
	int		mpChargeTime;
	float	engineHeat;
	Vector3	PlayerFriction;
	Vector3	PlayerOldVel;
	Vector3	HitDir;
	Vector3	aimedNormal;
	Point3	leftGunPos;
	Point3	rightGunPos;
	Point3	topGunPos;
	Point3	com;
	Point3	aimPos;
	Point3	camNewPos;
	Point3	CamPosOff;
	Point3	CamLookOff;
	Point3	targlookDir;
	Point3	joyPos;
	Point3	camLookCrntPos;
	Point3	aimedPoint;
	struct	tagCamera	*cam;
	PNode	*lod[3];						// The three levels of detail pnodes
	Object	*lodObject[3];					// The copies of the object per lod
	Object	**lodObjId[3];					// The objIds per lod
	Object  *normalGunObj;
	Object	*aimObj;
	struct strat *Player;					// The strat associated with this player
	struct strat *aimStrat;
	struct strat *CameraStrat;
	struct strat *TargetList;
	struct strat *shieldStrat;
	Matrix	GlobalCamMatrix;
	SecondaryWeapon PlayerSecondaryWeapon;
	RedDogWheel	rdWheel[4];
	TARGET	target[MAX_TARGETS];
	struct tag_HDStrat *PlayerOnHoldPlayer;
	// Multiplayer stuff :
	int		kills[MAX_PLAYER];	// Number of kills of other ppl
	int		lives;				// Number of lives remaining
	int		framesAlive;		// Number of frames spent alive
	int		maxFrames;			// The maximum number of frames spent in the 'special' state
	int		curFrames;			// The current number of frames in the 'special' state
	int		totFrames;			// The total time spent in 'special' state
	int		timer;				// Countdown timer for various uses
	bool	special;			// TRUE if you're the predator/bomb tank/suicide tank
	int		invuln;				// Seconds of invulnerability left
	int		team;				// Team 0 is no team, else 1 or 2
	int		shotsFired;			// Number of shots fired
	int		shotsHit;			// Number of shots that actually hit something
	int		cloaked;			// Whether the player is cloaked or not: 0 = no, -1 = forever, number = countdown timer
	int		reward;				// Rewards, like permanent boost and stronger weapons
	float	handicap;			// The max health this player can have due to handicap
	float	maxHealth;			// The current max health this player can have due to handicap and temporary effects
	struct strat	*RedDogHudStrat;
}PLAYER;


extern	bool Multiplayer;
extern int currentPlayer;
extern PNode	*MultiTank[MAX_TANK][3];	// The multiplayer tanks
extern PNode	*MPArrow;					// The arrow
extern int MultiTankType[4];				// The four choices of tank type
extern int PlayerLifeLost, PlayerCameraState[4];

#define		NONE					0
#define		BOMB					1
#define		VULCAN					2
#define		HOMING_BULLET			3
#define		ALL_ROUND_SHOCKWAVE		4
#define		ARMOUR					5
#define		ROCKET					6
#define		MINE					7
#define		HOVERON					8
#define		SWARM					9
#define		LASER					10
#define		ELECTRO					11
#define		CLOAKING				12
#define		STORMING_SHIELD			13
#define		FLAMER					14
#define		RDHM					15
#define		ENERGY					16

#define ANY_ACTIVATION			123

#define ENERGY_RECHARGE_RATE	(1.0 / 300.0)

#define LOCKON_WAIT				60.0f

#define MAX_HOLD_PLAYER	256

#define MAX_STRATCOLLS 128
typedef struct collstrat
{
	struct	strat	*strat;
	struct  collstrat *prev;
	struct	collstrat *next;
}COLLSTRAT;

typedef struct tag_spark
{
	Point3	pos;
	Point3	oldPos;
	Vector3	vel;
	short  	id;
	short  	time;
	int		type;
	Colour	col;
	struct	strat *parentstrat;
	Uint8	objid;
	Uint8   flag;
	struct tag_spark	*next;
	struct tag_spark	*prev;
}SPARK;

#define MAX_SP 512

extern	int		freeSparkPointer;
extern	int		freeSpark[MAX_SP];
extern	SPARK	*lastSpark;
extern	SPARK	sparkArray[MAX_SP];
extern	SPARK	*firstSpark;

#define SPIN_LEFT	1
#define SPIN_RIGHT	2
#define SPIN_TOP	4
#define SPIN_BOTTOM	8	

typedef struct strat
{
	PNode   *pnode;
	struct AnimInstance *anim;
	Object	*obj;
	Object	**objId;
	Vector3	relVel;
 	Vector3	vel;
	Point3	pos;		/*HAVEN'T GOT A CLUE ABOUT THIS */
	Point3	oldPos;
	Point3	oldOldPos;
	Vector3	rot_vect;
	float	rot_speed;
	float  	frame;		/*frame counter */
	Vector3 rotRest;
	Matrix	m;			/*CURRENT ROTATION MATRIX OF THE STRAT */
	Vector3	scale;		/*NOT SURE WHAT THIS IS */
 	Vector3	relAcc;
	Vector3	turn;

	Vector3 friction;	
	unsigned char	timeblock[6];
	unsigned char	stackblock[6];

	unsigned char   *timer;
	unsigned char	*stack;
	unsigned int   	flag;
	unsigned int    flag2;

	short   actindex[5];	/*activation point index */
	Vector3 CheckPos;	
 
	struct  route *route;
	float	*FVar; 	
	int		*IVar;

	struct  strat *parent;
	struct	strat *next;
	struct	strat *prev;
	struct trigger	*trigger;
	void (*func)(struct strat *strat);
	float	health;			/*Between 0.0 (Dead) and 1.0 (Full health) */
	float	damage;
	float  turnangz;		//GET RID OF THIS AND MOD A00WALKER
	struct strat *target;
	float	trans;			/*TRANSLUCENCY VALUE */
	float	var;   
	Uint16	polyType;
	Vector3	cpn;			//WOULD LIKE TO REMOVE THIS EVENTUALLY
	Vector3	wallN;
	struct tagStratSoundHeader *SoundBlock;
	COLLSTRAT *CollBlock;
	struct tag_HDStrat *hdBlock;
	struct tag_HoldPlayerBlock *hpb;
	PLAYER	*Player;
	struct  strat *CollWith;
	#ifdef MEMORY_CHECK
		 short  entry;   
	#endif

} STRAT;

typedef struct tag_StratList
{
	STRAT *strat;
	struct tag_StratList *next;
	struct tag_StratList *prev;
}StratList;

extern StratList *firstAvoidStrat, *lastAvoidStrat;
extern float	windX, windY;

#define FABS(a) (float)fabs(a)
#define FSQRT(a) (float)sqrt(a)
#define IABS(a) abs(a)


#define MAXMODS 512
#define MAXANIMS 64
#define MAXCHARACTERS 10 //MAXIMUM AMOUNT OF CHARACTERS


#define MAXCAM 10



/*****	GLOBAL DEFINITIONS				********/ 
/*****	SHARED BY STRATS AND MAIN GAME	********/ 
/*****	IF CHANGED UPDATE DEFS.C OF THE LEXER ********/ 
#define		EASY			0
#define		HARD			1


#define		MAX_GAMETARGETS 128
#define		MAX_STRATS		768
#define		MAX_TRIGGERS	1024
#define		MAX_BOID		64
#define		MAX_STRATANIM	1024
#define     MAX_MODELANIM	128

#define		MAX_STRATLIGHTS	128
#define		MAX_STRATROUTES	1024

#define	STRAT_DEAD					0
#define	STRAT_ALIVE		    			(1<<0)
#define STRAT_UNDER_WATER			(1<<1)
#define STRAT_HOLDING				(1<<2)		/*IF NON OF THESE SET ASSUME AGGRESSIVE PATH ONLY */
#define	STRAT_COLLFLOOR				(1<<3)
#define	STRAT_AVOIDSTRAT			(1<<4)   
#define	STRAT_GRAVITYON				(1<<5)
#define	STRAT_CALCRELVEL			(1<<6)
#define	STRAT_HIDDEN				(1<<7)
#define	STRAT_HITFLOOR				(1<<8)
#define	STRAT_COLLSTRAT				(1<<9)		/*ENABLES STRAT/STRAT COLLISION*/
//#define STRAT_PARENT				(1<<10)		/*STRAT_PARENT USED IN PARENT SPAWN*/												
#define STRAT_RUNONCE				(1<<10)		/*NEED TO FLAG AS A RUNONCE*/
#define	STRAT_FRIENDLY				(1<<11)		/*FRIENDLY */
#define	STRAT_ENEMY					(1<<12)		/*ENEMY */
#define	STRAT_SPRITE				(1<<13)		/*TYPE */
#define	STRAT_NOT_EXPLODE			(1<<14)		/*No Effect when explode */
#define STRAT_FPARAMSET				(1<<15)		/*Indicates the strat having attached FVARS */
#define	STRAT_DIRECTMOVE			(1<<16)		/*DIRECT ROTATION TO POINT */
#define	STRAT_PLANEMOVE				(1<<17)		/*Plane movement to way point */
#define	STRAT_HOVER					(1<<18)		/*Yaw only, roll maybe */
#define STRAT_NOMOVE				(1<<19)		/*LOW COLL STRATS, WHICH JUST AFFECT COLLIDING STRATS' MOVEMENT*/
//#define	STRAT_GROUND				(1<<19)		/*Ground objects */
#define	STRAT_BULLET				(1<<20)		/*Bullet type */
#define	STRAT_REALGRAVITYON	    	(1<<21)		/*Real Graqvity -9.81 */
#define STRAT_LOW_DETAIL_COLLISION	(1<<22)
#define STRAT_EXPLODEPIECE			(1<<23)
#define STRAT_IMPACTDIRECTION		(1<<24)		/* WHEN STRAT IS HIT BY STRAT THIS INFORMS COLLISION THAT DIRECTION OF HIT IS NEEDED*/
#define STRAT_AGGRESSIVE			(1<<25)		/*MODES FOR BEHAVIOUR */
#define STRAT_DEFENSIVE				(1<<26)
#define STRAT_NOSTRAT2STRAT  (1<<26)		/*TO ENSURE WE ARE NOT CONSIDERED WHEN STRATS PERFORM STRAT/STRAT COLLISION*/
//#define STRAT_PRIMETARGET			(1<<27)		/*THIS STRAT IS A PRIMARY TARGET INDICATOR*/
#define STRAT_DELETECALLBACK		(1<<27)		/*INDICATES A CALLBACK NEEDED ON DELETE*/

#define COLL_ON_WATER				(1<<31)
#define COLL_ON_FLOOR				(1<<30)
#define COLL_TARGETABLE				(1<<29)
#define	COLL_HITSTRAT				(1<<28)		/*INDICATES A COLLISION WITH STRAT */

//OBJ FLAG START
#define OBJECT_CALC_MATRIX				(1<<0)
#define OBJECT_SHARE_OBJECT_DATA		(1<<1)
#define OBJECT_HIT_PLAYER				(1<<2)
#define OBJECT_HIGH_DETAIL_COLLISION	(1<<3)
#define OBJECT_ID_MASK					((1<<4)|(1<<5)|(1<<6)|(1<<7)|(1<<8)|(1<<9))		/* bits 4 to 9 are the object ID */
#define OBJECT_RECORD_COLL				(1<<10)		// When a collision happens, record where
#define OBJECT_NOCOLLISION				(1<<11)
#define OBJECT_INVMATRIX				(1<<12)
#define OBJECT_STRATMOVE				(1<<13)	/*FLAG DEFINES WHETHER THE SUBOBJECT TAKES THE ANIM OR STRATMOVEMENT*/
#define OBJECT_INACTIVE_COLL			(1<<14)
#define OBJECT_IN_LAVA					(1<<15)
#define OBJECT_NOFADE					(1<<16) //Do not fade this object when passing farz
#define OBJECT_NOTVALIDTOHURT			(1<<17) //DO NOT HURT THE STRAT WHEN I'M HIT
#define OBJECT_NOCULL					(1<<18) //DO NOT Cull Faces when doing high detail collision
#define	OBJECT_NOBULLETCOLLIDE			(1<<19) //DO NOT ALLOW BULLETS TO HIT ME
#define OBJECT_NOCAMERACOLLIDE			(1<<20)	//DO NOT ALLOW CAMERAS TO HIT ME
#define OBJECT_NOPLAYERCOLLIDE			(1<<21)	//DO NOT ALLOW PLAYER TO HIT ME
#define OBJECT_SHOT_BY_SIBLING			(1<<22)
#define OBJECT_STRAT_SHADOW				(1<<23)	// Object has got a 'whole' shadow stretched over it
#define OBJECT_SPECULARFLASH			(1<<24)	//Specular flash this object
#define OBJECT_INVISIBLE				(1<<25) // This has changed to make room
#define OBJECT_NOTSOLID					(1<<26) //Detect Collision but no moving

#define OBJECT_GET_ID(a)			(((a)>>4) & 63)
#define OBJECT_MAKE_ID(a)			(((a) & 63)<<4) 
#define OBJECT_GET_COLL_ID(a)		(((a)>>15) & 127)// Bits 15 to 21 are the collision number
#define OBJECT_MAKE_COLL_ID(a)		(((a) & 127) << 15)
#define OBJECT_CLEAR_COLL_ID(a)		((a) & ~(127<<15))

/*FLAG 2 VARS */
#define STRAT2_PLAYER0				0
#define STRAT2_PLAYER1				1
#define STRAT2_PLAYER2				2
#define STRAT2_PLAYER3				3
#define STRAT2_MATRIX				(1<<2)
#define STRAT2_STRATTARGET			(1<<3)		//A STRAT WHO IS UP FOR TARGETTING BY OTHER STRATS
#define STRAT2_BIGDAMAGE			(1<<4)
#define STRAT2_ENEMY_CF				(1<<5)
#define STRAT2_CAMERA_CF			(1<<6)
#define STRAT2_BULLET_CF			(1<<7)
#define STRAT2_PLAYER_CF			(1<<8)
#define STRAT2_SPIN_LEFT	   		(1<<9)		//SPIN FLAG REPLACEMENTS
#define STRAT2_SPIN_RIGHT	   		(1<<10)
#define STRAT2_HOLDPLAYER			(1<<11)
#define STRAT2_COLLHDC				(1<<12)		
//#define STRAT2_SHIELDTYPE			(1<<12)	//FOR STRAT'S CHILD SHIELDS
#define STRAT2_MADEOFFMAP			(1<<13)		//THE STRAT WAS INITIALLY PLACED IN THE MAP
#define STRAT2_RESPAWN				(1<<14)
#define STRAT2_PICKUP				(1<<15)
#define STRAT2_TARGETTED			(1<<16)
#define	STRAT2_PROCESSSTOPPED		(1<<17)
#define STRAT2_HITHDC				(1<<18)
#define STRAT2_SUSPENDED   			(1<<19)		//A SUSPENDED STATE THAT MAP STRATS GO INTO WHEN THEY'RE DELETED..PRIOR TO BEING DELETED PROPERLY BY DELETION EVENTS
#define STRAT2_NOHDC				(1<<20)		//DISABLE HDC CHECKS ON A STRAT
#define STRAT2_STRATCOLLIDED   		(1<<21)		//FOR ENEMY AVOIDANCE ROUTINES
#define STRAT2_ENTIRENET			(1<<22)
#define STRAT2_COLL_MOVE			(1<<23)
#define STRAT2_NODAMAGE 			(1<<24)
#define STRAT2_INDEPENDENTROT 		(1<<25)
#define STRAT2_PLAYERHIT			(1<<26)
#define STRAT2_IMMEDIATEREMOVAL	  	(1<<27)		//INDICATES A MAP STRAT WITH NO DELETION TRIGGER, THAT SHOULD BE IMMEDIATELY REMOVED FROM THE STRAT LIST ON DELETION
#define STRAT2_IPARAMSET			(1<<28)
#define STRAT2_ONPATROLROUTE		(1<<29)
#define STRAT2_SPECULAR				(1<<30)
#define STRAT2_TRANSLUCENT			(1<<31)


#define XANGNEEDED 1
#define YANGNEEDED 2
#define ZANGNEEDED 4

#define PS_NORMAL		1
#define PS_WATERSLIDE	2
#define PS_SIDESTEP		4
#define PS_TOWERLIFT	8
#define PS_ONPATH		16
#define PS_ONTRAIN		32
#define PS_SCRIPTHELD	64
#define PS_INDROPSHIP	128
#define PS_READYTOENTER	512




#define PATH 0
#define NETWORK 1
#define ENTRYSPLINE 2
#define NETTERMINATE 254

/*USEFUL CONVERSIONS RIPPED FROM NINJA */

#define S_DEG_RAD(n)  ((n) * PI / 180.0)
															/*  deg ¨ rad  */
#define S_RAD_DEG(n)  ((n) * 180.0 / PI)
															/*  rad ¨ deg  */

/*THESE ONES DEAL WITH AN INTEGER REPRESENTATION OF 2PI -65535-65536 = -360 - 360 DEGS */

#define S_DEG_ANG(n)  (((Angle)(((n) * 65536.0) / 360.0)) & 0x0000ffff)
															/*  deg ¨ ang  */
#define S_ANG_DEG(n)  ((n) * 360.0 / 65536.0)
															/*  ang ¨ deg  */
/*#define S_RAD_ANG(n)  (((long)((n * 65536.0) / PI2)) & 0xffff) */
															/*  rad ¨ ang  */
/*#define S_ANG_RAD(n)  ((((float)n) * (PI2 / 65536.0))) */
															/*  ang ¨ rad  */
#define S_RAD_ANG(n)  (((long)((n * 32768.0) / PI)) & 0xffff)
														/*  rad ¨ ang  */
/*#define S_ANG_RAD(n)  ((((float)n) * (PI / 32768.0))) */
#define S_ANG_RAD(n)  ( (n) * (PI) / 32768.0 )
															/*  ang ¨ rad  */

#define SANG(n) S_RAD_ANG(n)

//#define Satan2(a,b) (atan2((double)a,(double)b))
#define Satan2(a,b) (PI2 * fatan2(a,b))
#define Scos(a) (float)cos((double)a)
#define Ssin(a) (float)sin((double)a)
#define Satan(a) (float)atan((double)a)

typedef struct levelsettings
{
	unsigned char ambientred;
	unsigned char ambientgreen;
	unsigned char ambientblue;
	float	scapeintensity;
	float	stratintensity;
	unsigned char fogred;
	unsigned char foggreen;
	unsigned char fogblue;
	float	fognear;
	float	fogfar;

	float	fov;
	float	farz;
	float	NormalCamHeight[3];
	float	NormalCamDist[3];
	float	NormalCamlookHeight[3];

	float	SideStepCamHeight[3];
	float	SideStepCamDist[3];
	float	SideStepCamlookHeight[3];

	float	JoyXDead;
	float	JoyYDead;
	
} LEVELSETTINGS;

typedef struct stratmap
{
	STRAT   *strat;		 /*POINTER TO THE LOCATION OF THE STRAT CREATED*/
	STRAT   *origstrat;	/*BACKUP OF THE ABOVE*/
	unsigned char	invalid;
	short	StratEnt;	/*ID OF STRAT IN THE STRAT LIST	 // COULD BE BYTE DEPENDING ON TOTAL NUM STRATS */
	short	modelid;									 /* COULD BE BYTE DEPENDING ON TOTAL NUM MODELS */
	Vector3 pos;		/*Start position of strat */
	Vector3	rot;		/*Start rotation of strat */
	Vector3	scale;		/*Start rotation of strat */
	int		flag;		/*Start flags of the object */
	int		flag2;		/*Start flags 2 of the object */
	short  	way;		/*WAYPOINT INDEX OF THIS STRAT	//DEFINATELY MAKE SHORT */
	unsigned char	startnode;	/*THE STARTING NODE ON THE SPECIFIED WAYPATH  //SHOULD BE AN UNSIGNED CHAR AS MAX OF 256 NODES ALLOWED */
	unsigned char	startpatrol;  /*THE STARTING PATROL ROUTE ON THE WAYPATH/NET  //SHOULD BE AN UNSIGNED CHAR AS MAX OF 256 NODES ALLOWED */
	short	trig;		/*IS IT A DEFAULT OR TRIGGERED STRAT	//MAKE A SIGNED CHAR IF TRIGS CAN BE KEPT BELOW 127 PER MAP */
	short	attachedevent;	/*HAS THE STRAT GOT AN ATTACHED EVENT ? */
	short	eventdepend;	/* WHAT EVENT, IF ANY, THE STRAT IS ATTACHED TO	*/
	short	activation[5];	/*HAS THE STRAT GOT AN ATTACHED 'ACTIVATION' TRIGGER ASSOCIATED ? */
	unsigned char intparams;	/*NUMBER OF INT PARAMS ATTACHED TO THE STRAT */
	unsigned char floatparams;	/*NUMBER OF FLOAT PARAMS ATTACHED TO THE STRAT */
	int		*iparamlist;
	float	*fparamlist;
	short	delay;		/*THE DELAY NEEDED BEFORE THE STRAT GOES ACTIVE */
	short	parent;		/*parent strat*/
} STRATMAP;


#define MAX_SCRIPTS 64
#define MAX_SCENES  16

typedef struct ScriptEntry
{
	short numlines;		//NUMBER OF LINES IN SCRIPT ENTRY
	unsigned char flag;
	signed char character; //ASSOCIATED CHARACTER
	short frametime;	//TIME ON SCREEN AFTER THE TEXT HAS COMPLETED PRINTING
	char **script;		//PTRS TO SCRIPT STRINGS

}SCRIPTENTRY;


typedef struct StratAnimMatt
{
	short objent;
	short animent;
	char *name;
}STRATANIMMATT;

/*EVENT TRIGGER FLAGS AND STRUCTURE */
#define	ACTIVETRIG		(1<<0)
#define	TIMERTRIG		(1<<1)
#define	EVENTTRIG		(1<<2)
#define	DEPENDANTTRIG	(1<<3)
#define	DEACTIVATED		(1<<15)



typedef struct maptrigger
{
	Vector3 pos;			/*POSITION */
	float	radius;			/*RADIUS */
	short   nument;			/*NUMBER OF ENTRIES ATTACHED TO THIS TRIGGER POINT */
	short	*entries;		/*LIST OF MAP ENTRIES TO CREATE */
	unsigned char *gen;		/* TO SAY WHETHER OR NOT EACH STRAT HAS BEEN CREATED */
	unsigned char	triggered;		/*TO SAY WHETHER OR NOT WE HAVE TRIGGERED THE TRIGGER !! */
	short	flag;			/*TYPE FLAG WAREZ */
	short	counter;		/*ATTACHED COUNTER START VALUE FOR TIMER TRIGGERS */
	short	currentcount;	/*CURRENT COUNTER VALUE FOR TIMER TRIGS */
}MAPTRIGGER;

#define MAX_EVENTS 64


#define WAITING 0
#define ALIVE 1
#define DEAD 2


/*MATT ADDED */
typedef struct mapevent
{
	unsigned char	flag;	/*FLAG TO SAY DELETION OR CREATION*/
	short   nument;			/*NUMBER OF ENTRIES ATTACHED TO THIS TRIGGER POINT */
 	short   startnument;	/*STATIC COPY OF THE ABOVE */
	short	*entries;		/*LIST OF MAP ENTRIES TO CREATE */
	unsigned char	triggered;		/*TO SAY WHETHER OR NOT WE HAVE TRIGGERED THE TRIGGER !! */
	unsigned char   numchild;
	short   *childlist;
	unsigned char *eventstatus;
	short	deleted;/*NUMBER OF DELETED MASTERS */
	short	count;	/*NUMBER OF CREATED MASTERS */
	STRAT **eventstrat;
	short   timer;	/*EVENT TIMER*/
	short   starttimer; /*COPY OF EVENT TIMER FOR MAP RESET WAREZ*/
}MAPEVENT;



typedef struct mapactivation
{
	float	radius;			/*RADIUS */
	Vector3 pos;			/*POSITION */
}MAPACTIVATION;



typedef struct patrol
{
	unsigned char numpathpoints;	/*number of points in the initial path through the network */
	unsigned char *initialpath;	/*reference to initial path vertex list */
}PATROL;



typedef struct network
{

	int  *connectors;		/*POINTER TO LIST OF CONNECTING NODES PER WAYPOINT */
/*	unsigned char *visited;		/*reference to visited index */ 
	unsigned char numpathpoints;	/*number of points in the initial path through the network */
	unsigned char *initialpath;	/*reference to initial path vertex list */
	char	*waybusy;		/*POINTER TO BUSY FIELDS */
	PATROL  *patrol;		/*POINTER TO PATROL ROUTES ARRAY */
}NET;


typedef struct waypath
{
	short	numwaypoints;	/*NUMBER OF POINTS IN THIS PATH */
	Vector3 *waypointlist;  /*POINTER TO THE LIST OF WAYPOINTS */
	short	area;			/*ATTACHED AREA TO WAYPATH	 */

	short  waytype;			

	NET *net;			/*POINTER TO NET STRUCTURE */
	
							/*IF NETWORKED	 */
/*	int  *connectors;		//POINTER TO LIST OF CONNECTING NODES PER WAYPOINT */
/*	unsigned char *visited;		//reference to visited index */
/*	unsigned char numpathpoints;	//number of points in the initial path through the network */
/*	unsigned char *initialpath;	//reference to initial path vertex list */

} WAYPATH;


typedef struct circlearea
{
	float	radius;			/*radius squared value */
	Vector2 pos;

}CIRCLEBOX;


typedef struct ellipsearea
{
	float	length;
	float	width;
	Vector2 pos;

}ELLIPSEBOX;

typedef struct splinearea
{
	short	numpoints;	/*NUMBER OF POINTS IN THIS PATH */
	Vector2 *pointlist;  /*POINTER TO THE LIST OF WAYPOINTS */
    struct 	splinearea *escape;	/*OPTIONAL ESCAPE ROUTE*/

}SPLINEBOX;


typedef struct boxarea
{
	float	maxx;
	float	minx;
	float	maxy;
	float	miny;

}BOXBOX;


typedef struct camppoint
{
	Vector2	pos;
	short  	busy;
	short	pad;	
}CAMPPOINT;


typedef struct maparea
{
	CIRCLEBOX Surround;	/*THE SURROUNDING ENCLOSURE */
	short	numextsplineareas;		/*NUMBER OF OUTER SPLINE COLLISION AREAS (SHOULD BE 1 ONLY) */
	SPLINEBOX *extsplinebbox; /*LIST OF OUTER SPLINE COLLISION AREAS (SHOULD BE 1 ONLY) */

	short	numcircleareas;		/*NUMBER OF INNER CIRCULAR COLLISION AREAS */
	CIRCLEBOX *circlebbox; /*LIST OF INNER COLLISION CIRCLES */
	short	numboxareas;	/*NUMBER OF INNER BOX COLLISION AREAS */
	BOXBOX *boxbbox;		/*LIST OF INNER BBOX COLLISION AREAS */
	short	numsplineareas;		/*NUMBER OF INNER SPLINE COLLISION AREAS */
	SPLINEBOX *splinebbox; /*LIST OF INNER SPLINE COLLISION AREAS */
	short	numlinesightareas;	/*NUMBER OF INNER SPLINE LINE SIGHT AREAS */
	SPLINEBOX *linesightbbox;		/*LIST OF INNER SPLINE LINE SIGHT AREAS */
	short	 numcamppoints;
	CAMPPOINT  *camppoints;
	/*REMOVED FOR NOW*/
	#if 0
		short	numellipseareas; /*NUMBER OF INNER ELLIPSE COLLISION AREAS */
		ELLIPSEBOX *ellipsebbox; /*LIST OF INNER ELLIPSE COLLISION AREAS */
	#endif
} MAPAREA;

typedef struct cammodifier
{
	float	amount;
	float   speed;

	/*Point3	upoff;
	Point3	downoff;*/
}	CAMMODIFIER;

typedef union var_tag
{
	float fvar;
	float ivar;
}	VAR;



typedef	struct	stratentry
{
	void *address;
	unsigned short numfloat;
	unsigned short numint;
} STRATENTRY;

typedef struct gametargets
{

	STRAT  *strat;
	short	occupied;
	struct gametargets *next;
	struct gametargets *prev;
}GAMETARGETS;



#define TANK	0
#define MECH	1

#define ABSOLUTE 1
#define REL 0


/*TRIGGER TYPE LOOK UPS INTO TRIGGER PROCESSING TABLE; */

#define	EVERY				0x00000000
#define	WHENHIT				0x00000001
#define	ONFLOOR				0x00000002
#define	PLAYERCOLLIDE		0x00000003
#define	NEARPLAYER			0x00000004
#define	NEARWAYPOINT		0x00000005
#define	PARENTDEAD			0x00000006
#define	PLAYERDEAD			0x00000007
#define	AHEADOFPLAYER		0x00000008
#define	NEARACTPOINT		0x00000009
#define	WHENANIMOVER		0x0000000a
#define	WHENDEAD			0x0000000b
#define	WHENOUTSIDE			0x0000000C
#define	WHENPARENTCREATED  	0x0000000d
#define	WHENHITSTRAT		0x0000000e
#define	WHENTARGETDEAD		0x0000000f
#define	WHENPAUSED			0x00000010
#define	WHENDELETED			0x00000011



/*NUMBER OF DIFFERENT TRIGGER TYPES */
#define TRIGTYPE	19



#define	TRIG_RUNNING		0x0001
#define	TRIG_DELETE			0x0002
#define	TRIG_HELD			0x0004
#define	TRIG_CHANGED		0x0008
#define TRIG_UNPAUSEABLE		0x0010


typedef	struct	trigger
{
	void	(*address)(struct strat *strat);
	int		trig_type;
	short   trig_count;
	short   trig_timer;
	unsigned char	internalloop[6];
	unsigned char	internalstack[6];
	short	flag;
	struct	trigger *next; 
	struct	trigger *next_trig; 
	struct	trigger *prev;
	struct	trigger *prev_trig; 
	struct  trigger *pausedby;
} TRIGGER;


#define MAX_PRIMTARGETS	4024


#define MAX_LOCKEDTARGETS	4024
typedef	struct	lockedtargets
{
	STRAT	*strat;
	int		lockedon;
} LOCKEDTARGETS;

typedef	struct	flock
{
	int		flocktype;
	Vector3	offset;
	float	wobble;
}	FLOCK;

typedef struct route
{
	short	pathstartnode; /*THESE BLOCKS WILL BE MOVED TO WAYPOINT STRUCT */
	short   patrolnumber;
	int		pathnum;	/*INDEX TO PATH ARRAY */
	struct waypath *path; /*POINTS TO CURRENT FOLLOW PATH */
	short	curway;		/*THE WAYPOINT IN THE CURRENT PATH */
	short   lastway;	/*last point hit*/
	short   CollisionType;
	SPLINEBOX *Collide;		/*THE AREA LAST COLLIDED WITH */
	SplineData *splineData;
	short   camped;					/*OCCUPIED CAMPING POINT WITH A GIVEN AREA*/


	struct  route *next;
} ROUTE;

#ifdef MEMORY_CHECK
	extern int	HDCALLOC,HDCALLOCCHKSUM,ALLOCSPLINES,ALLOCSPLINESCHKSUM,HPBALLOC,HPBALLOCCHKSUM;
#endif

extern  Vector3			CurrentRestart, HoldPlayerV;
extern	Vector3			CurrentRestartLook;
extern	int				Restart, boatPAC, NoMainGun;
extern	PLAYER			player[4];
extern	int				MissionFailure,RedDogFLS, RedDogFRS, RedDogRLS, RedDogRRS, GlobalParamInt[8], noSwarmTargets, swarmObjectTargetArray[20], noShocks, fadeN[4], fadeLength[4], fadeMode[4], bad_cheat;
extern	float			rFlareR, rFlareG, rFlareB, PlayerControlPlayerAcc, GlobalParamFloat[8], EscortTankerSpeed;
extern	STRAT			*oldtStrat, *OldPlayer, *WaterSlideCamLookStrat, *WaterSlide, *HoldPlayerMain, *swarmStratTargetArray[20], *BoatStrat, *BossGuide, *ArrowStrat, *EscortTanker, *EscortTankerHit[3], *SpecialStrat, *ScoobyDoo, *iceLiftGun;
extern	COLLSTRAT		*FirstColl;
extern	Point3			shockArray[64], destinationPoint, boatPA[4];

extern	int		noHits;
extern	STRAT	*hitStratArray[16];
extern	Object	*hitObjectArray[16];
extern	STRAT	*hitCollWithArray[16];

extern	void			CamSetNow(void);
extern	void			ObjectNoCull(STRAT *strat, int objId);
extern	float			GetGlobalParamFloat(int n);
extern	int				GetGlobalParamInt(int n);
extern	void			SetGlobalParamFloat(int n, float f);
extern	void			SetGlobalParamInt(int n, int i);
extern	void			ClearGlobalParams(void);
#define DrawShock(x1, y1, z1, x2, y2, z2, noise, ss, es) DrawShock2(x1, y1, z1, x2, y2, z2, noise, ss, es, 1)
extern	void			DrawShock2(float x1, float y1, float z1, float x2, float y2, float z2, float noise, float ss, float es, int madness);
extern	void			DrawStratBar(bool Enemy, float amount, float flash, float alpha);
extern	void			DrawProgressBar(STRAT *strat, int pn, int time);
extern	float			GetObjectHealth(STRAT *strat, int objId);
extern	void			SetObjectHealth(STRAT *strat, int objId, float amount);
extern	void			SetPosRelOffsetPlayerObject(STRAT *strat, int pn, int objId, float x, float y, float z);
extern	void			MapClean();
extern	Uint32			PadPress[4];
extern	Uint32			PadPush[4];
extern	float			PlayerControlJoyX;
extern	float			PlayerControlJoyY;
extern	Uint32			PlayerControlPadPress;
extern	void			StrtNoCamColl(STRAT *strat);
extern	int				GetPadPush(STRAT *strat, int pn);
extern	int				GetPadPress(STRAT *strat, int pn);
extern	void			findTotalMatrix(Matrix *mat, Object *obj);
extern	void			findTotalMatrixInterp(Matrix *mat, Object *obj);
extern	void			SetPlayer();
extern	void			AddStratToSwarmTargetArray(STRAT *strat, float range);
extern	void			ClearSwarmTargetArray(void);
extern	int				GetSwarmTarget(STRAT *strat, int i);
extern	int				ValidSwarmTarget(STRAT *strat, int st);
extern	void			GetSwarmTargetPos(STRAT *strat, int st);
extern	void			SetCamera(STRAT *strat);
extern  int				SeePlayer(STRAT *strat,float angle);
extern  int				SeePlayerZ(STRAT *strat,float angle);
extern	void			SetPlayerCamTilt(int pn, float amount);
extern	float			GetPlayerCamTilt(int pn);
extern	int				ObjectSeePlayerZ(STRAT *strat,float angle, int objId, float offsetx, float offsety, float offsetz);
extern	void			InitHDC(STRAT *strat);
extern	void			SetObjectIdleRot(STRAT *strat, int objId, float amount, int mode);
extern	void			CalculateStratPolyData(STRAT *strat, int i);
extern	void			ChangeState  (STRAT *strat, void *Addr);
extern	void			DoSpark(STRAT *strat, int n, float x, float y, float z, float speed, float spread, int time, int red, int green, int blue);
extern	float			RandRange(float a, float b);
extern  int				SeeWay(STRAT *strat,float angle);
extern	int				SeeWayZ(STRAT *strat,float angle);
extern	int				AwaySeeWayZ(STRAT *strat,float angle);
extern	void			GetBoatPA(STRAT *strat, int i);
extern	void			MoveX (STRAT *strat, float amount);
extern	void			MoveXNoZ (STRAT *strat, float amount);
extern	void			MoveY (STRAT *strat, float amount);
extern	void			MoveXYZNow(STRAT *strat, float x, float y, float z);
extern	void			MoveZ (STRAT *strat, float amount);
extern	void			Yaw   (STRAT *strat, float amount);
extern	void			Reset (STRAT *strat);
extern	void			Delete (STRAT *strat);
extern	void			StratInit();
extern	int				GetObjectFlag(STRAT *strat, int objId);
extern	void			SetObjectFlag(STRAT *strat, int objId, int flag);
extern	void			ThonGravity(int pn, float amount);
extern	float			WatNoise (float x, float y);
extern	int				GetFireSecondary(int pn);
extern	void			SetFireSecondary(int pn, int n);
extern	void			InitArrowArray(void);
extern	void			PlayerObjectRelRotateXYZ(int i, float x, float y, float z);
extern	void			DrawAimLine(int pn);
extern	int				ObjectTowardPlayerXZ(STRAT *strat, int	objId, float ang, float ang2, float MAXZANG, float MAXXANG);
extern	int				ObjectTowardPlayerXZVF(STRAT *strat, int	objId, float angx, float angz, float MAXZANG, float MAXXANG, float mul);
extern  int				ObjectTowardPlayerXZOffset(STRAT *strat, int	objId, float angx, float angz, float MAXZANG, float MAXXANG, float offx, float offy, float offz);
extern	void			SetPlayerArmageddon(int pn, int amount);
extern	void			UpdateArrowArray(STRAT *strat);
extern  void			UpdateObjectCollFlag(STRAT *strat,int id, int flag);
extern	int				EnemyInFrontXY(STRAT *strat, float m, float r, int type);
extern	int				PlayerInFrontXY(STRAT *strat, float m, float r);
extern	int 			GetPlayerArmageddon(int pn);
extern	void 			ObjectReturnToStart(STRAT *strat, int objId);
extern	void			RemoveParentFromArrowArray(STRAT *strat);
extern	void			TowardPlayer (STRAT *strat, float maxx, float maxy);
extern	void			IAmJumpPoint(STRAT *strat);
extern  void			TowardWay (STRAT *strat, float maxx, float maxy);
extern	void			DrawTargetOnObject(STRAT *strat, int objId);
extern  void			AwayWayZ (STRAT *strat, float maxx);
extern  void			TowardParent (STRAT *strat, float maxx, float maxy);
extern  void			Explosion(STRAT *crntStrat, float power, float radius);
extern	void			DrawTargetAt(float x, float y, float z);
extern	void			SetCamSimple(STRAT *strat);
extern	void			GetPlayerPos(STRAT *strat, int pn);
extern	void			ObjectNotSolid(STRAT *strat, int objId);
extern  int				NearPlayer(STRAT *strat,float dist);
extern  int				NearPlayerXY(STRAT *strat,float dist);
extern	void			StrtNoFade(STRAT *strat);
extern	void			AddParentToArrowArray(STRAT *strat);
extern	void			SetCheckPos(STRAT *strat,float x,float y,float z);
extern	void			SetCheckPosRel(STRAT *strat,int id,float x,float y,float z);
extern	void			SetCheckPosRelOffsetPlayerObject(STRAT *strat, int objId, float x, float y, float z);
extern	void			ClearTargetArray(int pn);
extern	int				GetSecondaryWeaponAmount(int pn);
extern	void			SetSecondaryWeaponAmount(int pn, int amount);
extern	int				GetSem(STRAT *strat, int n);
extern	void			AddScreenToTargetArray(int pn);
extern	void			clearTargetArray(int pn);
extern	void			SetSem(STRAT *strat, int n, int m);
extern	int				GetParentSem(STRAT *strat, int n);
extern	void			SetParentSem(STRAT *strat, int n, int m);
extern	void			AddSem(STRAT *strat, int n, int m);
extern	void			SubSem(STRAT *strat, int n, int m);
extern	void			AddParentSem(STRAT *strat, int n, int m);
extern	void			SubParentSem(STRAT *strat, int n, int m);
extern	void			SetPlayerEnergy(int pn, float amount);
extern	float			GetPlayerEnergy(int pn);
extern	void			SetHealthOfPlayer(int pn, float amount);
extern	void			MoveTowardXY(STRAT *strat,float x,float y,float speed);
extern  int				NearCheckPos(STRAT *strat,float dist);
extern  int				NearCheckPosXY(STRAT *strat,float dist);
extern  int				NearPosXY(STRAT *strat,float dist,float x, float y);
extern  int				NearCheckPosZ(STRAT *strat,float dist);
extern	void			IAmPlayerRDHM(STRAT *strat, int pn);
extern	int 			GetPlayerStrat(STRAT *strat,int pn);
extern	void			ReduceHealth(STRAT *strat, float amount);
extern  void			StratProc();
extern	void			SetObjScale(STRAT *strat, int objId, float x, float y, float z);
extern	void			Flatten(STRAT *crntStrat, float amount);
extern	int				LNOT(int value);
extern	void			GetParentObjectPos(STRAT *strat, int i);
extern	STRAT			*Spawn(STRAT *parent, int StratEntry, float xpos, float ypos, float zpos, float xang, float yang, float zang,int model);
extern	void			SpawnAbs(STRAT *parent,int StratEntry, float xpos, float ypos, float zpos, float xang, float yang, float zang);
extern	void			SetTarget(STRAT *strat,float x, float y, float z);
extern  int 			GetNextWay(STRAT *strat);
extern  void			GetPrevWay(STRAT *strat);
extern  int				FirstWay(STRAT *strat);
extern  int				LastWay(STRAT *strat);
extern	void			LeaveFixedPath(STRAT *strat);
extern	int				FirstSplineSection(STRAT *strat);
extern	int				LastSplineSection(STRAT *strat);
extern	void			SetPlayerCamera(STRAT *strat);
extern	void			DrawAllTags(void);
extern	void			MoveZRel(STRAT *strat, float amount);
extern	float			distToFirstWay(STRAT *strat);
extern	float			distToLastWay(STRAT *strat);
extern	int				ObjectHitStrat(STRAT *strat, int objId);
extern	int				ObjectHitFloor(STRAT *strat, int objId);
extern	STRAT			*PlayerShieldStrat(int pn);
extern	void			RadiusDamage(STRAT *strat, float radius, float amount);
extern	int				GetNearestNetPatrolPoint(STRAT *strat,float dist);
extern	int				PlayerReward(int pn, int bit);
extern  int				GetNearestWay(STRAT *strat,float distance);
extern	int				GetNearestWaySpline(STRAT *strat,float distance);
extern	int				GetFurthestWay(STRAT *strat,int ConsiderAllPoints);
extern	float			HealthOfPlayer(int pn);
extern	float			MaxHealthOfPlayer(int pn);
extern  int				GetNearestWayToTarget(STRAT *strat,float distance);
extern	float			PlayerDistPlayerCheckXY(void);
extern  int				GetNearestWayToMe(STRAT *strat);
extern  void			ExplodeMe(STRAT *strat, int StratEntry);
extern	void			SetCollisionDetail(STRAT *strat, int level);
extern  void			InitPath(STRAT *strat);
extern  void			MoveBackOneFrame(STRAT *strat);
extern	void			ApplyAbsForce(STRAT *strat, float fx, float fy, float fz, float px, float py, float pz, float r);
extern	void			RandomPointTo(STRAT *strat);
extern	void			RandomPointToCam(STRAT *strat);
extern  void			ClearObj(STRAT *strat);
extern	void			CreateTriggerParam(STRAT *strat,void *address,int TrigType,float TrigParam);
extern	void			CreateTrigger(STRAT *strat,void *address,int TrigType);
extern	void			KillTrigger(STRAT *strat,void *address,int TrigType);
extern	void			HoldTrig(STRAT *strat,void *address,int TrigType);
extern	void			ReleaseTrig(STRAT *strat,void *address,int TrigType);
extern	void			SetAnim(STRAT *strat,int anim);
extern	void			SetAnimSpeedPercent(STRAT *strat,float percent);
extern	void			RotateObjectXYZ(STRAT *strat, int objId, float x, float y, float z);
extern	void			ClearPlayerTargetArray(int pn);
extern	void			RemoveTarget(STRAT *strat, int pn, int	tagetNo);
extern	void			RemoveTargetStrat(STRAT *strat, int pn, int	sn, int id);
extern	void			SetFriction(STRAT *strat, float x, float y, float z);
extern	void			RotationRestriction(STRAT *strat, float x, float y, float z);
extern	void			IAmWater(STRAT *strat, float choppiness, float waveHeight);
extern	void			ApplyRelForce(STRAT *strat, int objId, float x, float y, float z, float x2, float y2, float z2);
extern	void			UnitiseMatrix(STRAT *strat);
extern	float			GetObjectCrntRot(STRAT *strat, int objId, int mode);
extern	float			GetObjectCrntScale(STRAT *strat, int objId, int mode);
extern	float			GetObjectAnimScale(STRAT *strat, int objId, int mode);
extern	float			GetSpecularColour(STRAT *strat, int objId, int mode);
extern	float			GetObjectCrntPos(STRAT *strat, int objId, int mode);
extern	float			GetObjectIdleRot(STRAT *strat, int objId, int mode);
extern	float			GetObjectIdlePos(STRAT *strat, int objId, int mode);
extern	int				CanPickup(STRAT *strat, int pn, int type, int amount);
extern	void			SetObjectCrntRot(STRAT *strat, int objId, float amount,int mode);
extern	void			SetObjectCrntPos(STRAT *strat, int objId, float amount,int mode);
extern	float			Range(float a, float b, float c);
extern	void			MyStratToWorld(STRAT *strat, float x, float y, float z);
extern	void			SetOnCamera(STRAT *strat, float x, float y, float z, float rx, float ry, float rz);
extern	void			TurnTowardPosXZ(STRAT *strat, float x, float y, float z, float xang, float zang);
extern	void			TargettingStrat(int stratNumber, int a);
extern	void			ActivateObjectCollision(STRAT *strat, int objId);
extern	void			InActivateObjectCollision(STRAT *strat, int objId);
extern	void			InActivateObjectDamageable(STRAT *strat, int objId);
extern	int				EnemyInActivation(STRAT *strat, int an, int mode);
extern	void 			SetCheckPosPlayerRotate(STRAT *strat, float ang);
extern	void 			SetCheckPosMyRotate(STRAT *strat, float ang);
extern	void			RelTurnTowardCheckPosXY(STRAT *strat, float ang);
extern	void			SetPlayerMatrix(STRAT *strat);
extern	void			RelTurnTowardCheckPosSmoothXY(STRAT *strat, float ang);
extern	int				NearActivation(STRAT *strat,int num);
extern	int				NearActivationXY(STRAT *strat,int num);
extern	int				PlayerNearActivation(STRAT *strat,int num);
extern	int				EscortTankerInActivationXY(STRAT *strat,int num);
extern	int				ParentNearActivationXY(STRAT *strat,int num);
extern	int				PlayerNearParentParentActivationXY(STRAT *strat,int num);
extern	int				ValidStrat(int s);
extern	int				PlayerNearParentActivationXY(STRAT *strat,int num);
extern	int				PlayerNearActivationXY(STRAT *strat,int num);
extern	int				MPPlayerNearActivationXY(STRAT *strat,int num);
extern  int				MPPlayerNearActivation(STRAT *strat,int num);
extern	int				ValidTargetStrat(STRAT *strat, int pn, int sn, int objId);
extern	int				GetSecondaryTargetStrat(STRAT *strat, int pn);
extern	int				GetParentPlayerNumber(STRAT *strat);
extern	int				GetPlayerNumber(STRAT *strat);
extern	void			PickCheckPosStratToPlayerAngleOffset(STRAT *strat, float ang);
extern	void			SetStratMatrixVector(STRAT *strat, float a, float b, float c);
extern	void			SetActivationPoint(STRAT *strat);
extern	void			PointTo(STRAT *strat, float x, float y, float z);
extern	void			ClearRotFlag(STRAT *strat);
extern	void			PickMeUp(STRAT *strat, int type, int amount);
extern	void			GetPlayerColour(STRAT *strat, int pn);
extern	int				GetHomingBulletTarget(int pn);
extern	int				SetCheckPosToTarget(STRAT *strat, int s);
extern	int				GetLockOnCount(void);
extern	void			GetTargetStratPos(STRAT *strat, int pn, int stratn, int id);
extern	void			MoveTowardCheckPosNow(STRAT *strat, float amount);
extern	void			ObjectNotTargetable(STRAT *strat, int	objId);
extern	void			ObjectTargetable(STRAT *strat, int	objId);
extern	void			TiltAlongPath(STRAT *strat, float amount);
extern	void			SmartBombExplode(STRAT *strat, int pn, float range, float damage);
extern	void			SplineRelFlatten(STRAT *strat, float amount);
extern	float			PlayerDistXY(STRAT *strat);
extern	float			ParentDistXY(STRAT *strat);
extern	int				uId(STRAT *strat);
extern	int				PlayerNearPlayerCheckPosXY(float a);
extern	void			PlayerTurnTowardPlayerCheckPosXY(void);
extern	int				PlayerSeePlayerCheckPosXY(float ang);
extern	int				NearParentXY(STRAT *strat,float dist);
extern	void			FacePlayerXY(STRAT *strat, float ang);
extern	float			SmoothFromTo(float r, float i, float n);
extern  float			SmoothFromToSum(float range, float t, float n);
extern	void			RedDogSkid(STRAT *strat);
extern  void			FlattenToPoly(STRAT *strat, float amount);
extern	void			GetActivationPos(STRAT *strat, int	actNo);
extern	float			GetActivationRadius(STRAT *strat, int actNo);
extern	void			CompleteTrigger(STRAT *strat);
extern	void			UnPauseTriggers(STRAT *strat);
extern	void			PauseTriggers(STRAT *strat,int Except,int Forget);
extern	void			EndTrigger(STRAT *strat);
extern	void			HoldTrigger(STRAT *strat);
extern	void			ReleaseTrigger(STRAT *strat);
extern	void			AvoidStrats(STRAT *strat, float	amount);
extern	void			AddToAvoidStratArray(STRAT *strat);
extern	void			TransObjectRel(STRAT *strat, int objId, float x, float y, float z);
extern	float			InfrontOfObject(STRAT *strat, int objId, float dist);
extern	void			RelRotate(STRAT *strat, float x, float y, float z);
extern	void			SetScreenARGB(float a, float r, float g, float b);
extern	void			BounceOffShield(STRAT *strat, int pn);
extern	void			StormingShieldHit(STRAT *strat);
extern	void			StickToParent(STRAT *strat, int objId, float x, float y, float z, int mat);
extern	void			StormingShield(STRAT *strat);
extern	void			RemoveObject(STRAT *strat, int objId);
extern	void			RotateStratObjectXYZ(STRAT *strat, float x, float y, float z);
extern	void			FragAtObject(STRAT *strat, int ObjId);
extern	void			HideObject(STRAT *strat, int objId);
extern	void			UnhideObject(STRAT *strat, int objId);
extern	void			MoveAlongPath(STRAT *strat, float amount);
extern	int				MoveAlongPathXY(STRAT *strat, float amount);
extern	void			FaceAlongPath(STRAT *strat, int dir);
extern	void			FaceAlongDir(STRAT *strat);
extern	void			FaceAlongDirXY(STRAT *strat, int type);
extern	void			InitSplineData(STRAT *strat);
extern	int				CrntRotToIdleRotX(STRAT *strat, int objId, float ang);
extern	int				CrntRotToIdleRotY(STRAT *strat, int objId, float ang);
extern	int				CrntRotToIdleRotZ(STRAT *strat, int objId, float ang);
extern	void			SetPlayerOCP(void);
extern	float			XRelPosX(STRAT *strat, float amount);
extern	void			GetGunDir(STRAT *strat, int pn);
extern	float			XRelPosY(STRAT *strat, float amount);
extern	float			FacePlayerWaterSlide(STRAT *strat, float ang, float rs);
extern	void			PlayerRelRotateXYZ(float x, float y, float z);
extern	void			RelPosToCheck(STRAT *strat, int objId, float x, float y, float z);
extern	void			SetModel(STRAT *strat,int ModelEntry);
extern	int				BattleTankInActivation(STRAT *strat, int n);
extern	float			GetWaterSlideRelRotZ(STRAT *strat);
extern	float			StratSpeed(STRAT *strat);
extern	void			RemoveMeFromAllTargets(STRAT *strat);
extern	void			AddWaterStrat(STRAT *strat);
extern	void			AddLavaStrat(STRAT *strat);
extern	void			FacePlayerAim(STRAT *strat, int pn);
extern	float			CheckPosDist(STRAT *strat);
extern	float			PlayerSpeed(int pn);
extern	void			SetPlayerAcc(int pn, float amount);
extern	float			WaterSlideCamLookDist(STRAT *strat);
extern	float			DistToAim(STRAT *strat, int pn);
extern	void			DamageAim(STRAT *strat, int pn, float amount);
#if SH4
	extern void			MapInit(Stream *fp);
#else
	extern	void			MapInit(char *file);
#endif
extern	void			MoveXFlat (STRAT *strat, float amount);
extern	void			MoveYFlat (STRAT *strat, float amount);
extern	void			DrawVLine(STRAT *strat, float dist);
extern  void			SetPlayerZoom(STRAT *strat, int pn, float amount);
extern  float			GetPlayerZoom(STRAT *strat, int pn);
extern	void			SetPlayerBoostButtonCount(int pn, int i);
extern	void			MakeFrags (STRAT *strat, float radius, int amount);
extern	void			MakeSplash (STRAT *strat, float radius, int amount);
extern	void			MakeExpFrags (STRAT *strat, float radius, int amount);
extern	void			MakeTrailFrags (STRAT *strat, float radius, int amount, float x, float y, float z);
extern	void			MakeConeFrags (STRAT *strat, float cosAngle, float velocityMax, int amount);
extern	void			GetNearestTagToPlayer(STRAT *strat);
extern	float			GetConeAmount (STRAT *strat);
extern	void			ResetMap();
extern	void			GameOver();
extern  short			GameOverFlag, LevelComplete;
extern	short			LevelReset;
extern	short			Skill;
extern	float			globalVar;
extern  float			globalVar2;
extern	Point2			joyState[4];
extern float		Delta(float x);
extern	void			SetSecondaryWeaponType(int pn, int type);
extern	int 			GetSecondaryWeaponType(int pn);
extern	void PointToXY(STRAT *crntStrat, float x, float y, float z);
extern void			PointPlayer(STRAT *strat);
extern	STRAT		 StratPool[MAX_STRATS];
extern	STRAT		*FirstStrat;
extern unsigned char PlayerLives;
extern void PointToTarget(STRAT *strat);
extern	void HoldPlayer(STRAT *strat);
extern	void IAmHoldPlayer(STRAT *strat);
extern	void IAmHoldPlayerMain(STRAT *strat);
extern int InsideArea(STRAT *strat);
extern	void RedDogElectroShock(STRAT *strat);
extern MAPAREA* MapAreas;
extern WAYPATH* MapPaths;
extern int GetLockOn(STRAT *strat);
extern	void	GetLastWay(STRAT *strat);
extern	void	GetFirstWay(STRAT *strat);
extern int FollowLockOn(STRAT *strat);
extern void MakeTargets(float radius);
extern void ResetPath(STRAT *strat);
extern int GetNearestVisibleWayToMe(STRAT *strat,float ang);
extern void ClearBusy();
extern void RelRotateX(STRAT *strat, float ang);
extern void RelRotateY(STRAT *strat, float ang);
extern void RelRotateZ(STRAT *strat, float ang);


extern float GlobalFogFar, GlobalFogDensity;
extern	Uint8  GlobalAmbientLightRed, GlobalAmbientLightGreen, GlobalAmbientLightBlue;
extern	float GlobalFieldOfView, GlobalAmbientLightIntensityScape, GlobalAmbientLightIntensityStrat;

extern  void	LogCam(STRAT *strat);
extern  void	ChangeCam(int number,int mode);

extern PNode *Dome;
extern CAMMODIFIER		*CamModArray;
extern void SetModelAnim(STRAT *strat,int animoffset,int flag);
extern void ThisModelAnimPlay(STRAT *strat, int flag);
extern int HitWall(STRAT *strat,int Update);
extern int LineSightPos(STRAT *strat,Vector3 *startpos,Vector3 *endpos);
extern int PlayerOutSide(STRAT *strat);
extern float ProjectileAngle(STRAT *strat, int objId,float xOffset,float yOffset, float zOffset, float speed, float tx, float ty, float tz);
extern void DeleteMyParent(STRAT *strat);

extern int HasActivation(STRAT *strat, int act);
extern int GetNearestCollideWay(STRAT *strat,float distance);
extern int GetNearestCollideWayFound(STRAT *strat,float distance);
extern void InheritMyParent(STRAT *strat);
extern void ResetTrig(STRAT *strat);
extern int PositionVisible(STRAT *strat,Vector3 *startpos,Vector3 *endpos);
  
extern int GetNearestCampPoint(STRAT *strat,int mode);
extern void KillMyVelocity(STRAT *strat);
extern int FireSightTarget(STRAT *strat);
extern int LineSightPlayer(STRAT *strat);
extern int	AvoidSurround(STRAT *strat,int SteerMode);
extern	float DistToLastWayXY(STRAT *strat, int mode);
extern int PlayerAvoidSurround(STRAT *strat);
extern int AnyFreeTargets(STRAT *strat);
extern void RegisterAsTarget(STRAT *strat,int MaxOccupants);
extern int SaveTarget(STRAT *strat);
extern void RestoreTarget(STRAT *strat,int target);
extern	void	HoldPlayerRelRotateXYZ(STRAT *strat, float x, float y, float z);
extern	void	FlattenHoldPlayer(STRAT *strat, float amount);
extern int DisplayScript(int entry,float numchars, int line, int x, int y, int mode);
extern int GetScriptTime(int entry);
extern int GetScriptLines(int entry);
extern int GetScriptFlag(int entry);

extern void UpdateTrigFlag(int flag);
extern void LinkToParentObject(STRAT *strat, int id,float xang, float yang, float zang, float offsetx, float offsety, float offsetz);
extern void ChangeCamStrat(STRAT *strat);
extern int MyParentInvalid(STRAT *strat);
extern int MyParentParentInvalid(STRAT *strat);
extern MAPACTIVATION  	*MapActs;

// Decal stuff :
// Mark a subobject as needing collisions being recorded
extern void RecordCollision (STRAT *strat, int subId); // if subId = 0 then mark first object
// Adds a decal 
extern void AddDecal (STRAT *strat, int subId, int dNum, float xSize, float ySize, float r, float g, float b);
// Scales a subobject
extern void SetObjectScale (STRAT *strat, int subObj, float x, float y, float z);
// Set object transparency <0 = invisible, but still drawn, >0.99 = opaque (cheaper) draw
extern void SetObjectTrans (STRAT *strat, int subObj, float trans);

// Shield craziness
extern int AccelerateTowardsShield (STRAT *strat, int vertex);
extern int GetShieldVertexNumber (int pn);

// Weather warez
extern void IAmTheSun (STRAT *strat);
extern void SetRainAmount (STRAT *strat, int amountIn, int amountOut, bool Snow);
extern	void	SetPlayerMaxHealth(int pn, float maxHealth);
// Visibility warez
extern Uint32 rScapeVisMask;
#define SetVisBit(n) rScapeVisMask |= (1<<(n))
#define ClearVisBit(n) rScapeVisMask &= ~(1<<(n))

// Lightning
extern int DrawLightning (STRAT *strat, int objId, float x, float y, float z, int vertID, int mode);
 
extern void StratProcPaused();
extern void DrawArrow(STRAT *strat, float x, float y, int id);
extern	void DrawArrowTo(STRAT *strat, float x, float y, float z, float a, float r, float g, float b);
extern int VisibleFromPlayer(STRAT *strat,float angle);
extern int GetScriptEntry(int entry,int val);
extern	void PNum(int x, int y, float val);
extern PNode *GameHeads[MAXCHARACTERS];
extern	int	ObjectShotBySibling(STRAT *strat);
extern void Overlay (int ch, float amt);
extern int GetScriptCharacter(int entry);

extern int InitTimer (int CountDown, int StartTime);
extern void AddTime(int AdditionalTime);
extern void HideTimer (void);

extern int score;
extern float WatNoise(float x, float y);
extern void SetPlayerToTarget(STRAT *strat,int valid);
extern void SetPlayerBack(STRAT *strat);
extern void StopTheStrats(STRAT *strat);

// Useful function: return the player number of a strat or -1
extern int CheckEndGameStatus(void);

extern void SignalSpawnPoint (Uint32 id);
extern int RegisterSpawnPoint (STRAT *strat, Uint32 id);
extern int FindDeadPlayer (void);
extern bool NearAnyPlayer (STRAT *strat, float dist);
extern int IAmAimStrat(STRAT *strat);
extern	void HoldPlayerPos(STRAT *strat, int objId, float px, float py, float pz);
extern void HoldPlayerRot(STRAT *strat, int objId, float rx, float ry, float rz);
extern STRAT *BossStrat;

extern int	StratInPlayerCone(STRAT *strat, int pn, float ang);
extern int Winner;
extern int NumberOfPlayers;

// Vibration warez
void Vibrate (STRAT *strat, int priority, float freq, float power, float seconds);

extern void SetObjectShadowState(STRAT *strat, int OnOrOff);
extern void MoveAllStratsBack (STRAT *strat, float distance);

extern void IncShotsFired (STRAT *strat);
extern int	PickupAllowed (int type);
extern void PlayerKillSelf (STRAT *strat);
extern void PNODESetInitToCrnt (STRAT *strat, int id);
extern void ClearObjCollFlag(STRAT *strat, int id, int flag);
extern int  FiredByAssassin(STRAT *strat);

extern void GiveFlagToPlayer (int pNum);
extern int	ShallISpawnAFlag (void);
extern void IAmFlag (STRAT *strat, int trueness);
extern int	ObjectHitPlayer(STRAT *strat, int objId);
extern void DrawLensFlare (STRAT *strat, int objId, float x, float y, float z, float r, float g, float b, float intensity);

extern void AdjustAmbient (float amt);
extern void AdjustRedDogAmbient (float amt);
extern void SetSpecularColour(STRAT *s, int objId, float r, float g, float b);
extern void ObjectSpecularFlash(STRAT *strat, int objId, int flashOrNot);

extern void DontWhiteMeOut (STRAT *s);
extern void SetWhiteOut (float red, float green, float blue);
extern void SetEnvironmentParms(float nearFog, float farFog, float farCam, int DomeOn,
						 float fogRed, float fogGreen, float fogBlue);

extern int BeamHit(STRAT *s,float length, float radius, float x, float y, float z);
extern int DisplayBriefing (int scriptNum, float amount, int debrief);
extern int GetScriptLine(int entry, int line, char **textPtrPtr);
extern void TimeFunction (int what);
extern int BossTime, GameTime, DamageTaken;
extern void SparkSuck	(STRAT *strat,int objid, int mode, int n, int icol, int time);
extern void SparkSuckPos(STRAT *strat,int objid, int mode, int n, int icol, int time, float x, float y, float z);
extern void BossDarkness (float den);
extern void RegisterDanger(STRAT *strat, float dangerVal);
extern float AnimMoveOffset(STRAT *strat, int mode);
extern int ProcessLightning(int mode);
extern int ValidToBeam(STRAT *strat, float x, float BeamLength, float z, int mode);
extern void RadarUpdate(STRAT *strat, int update);
extern void CopyParentAnim(STRAT *strat);
extern LEVELSETTINGS *LevelSettings;

extern void MakeWaterRipple (STRAT *strat, float momentum, float radius);
extern void TrigCheckMe (STRAT *strat);


// Line madness
extern void TexturedLine (STRAT *s, int objId, float dist, float thickness, 
					float x1, float y1, float z1, float x2, float y2, float z2);

// Crazy light action, take 2
extern void RemoveLight (int id);
extern int MakePointLight (STRAT *strat, float r, float g, float b, float dist, float multiplier, float x, float y, float z);
extern void SetPointLightRadius (int id, float r, float m);
extern void UpdateLight (STRAT *strat, int id, float x, float y, float z);
extern void ExplosionLight (STRAT *strat, float r, float g, float b, float radius, float mul, float min);
extern bool MissionBriefingDone;

// Flarey lights
extern int  GetUniqueID(void);
extern int  GetCurrentFrame(void);
extern int  OnScreen (STRAT *s);
extern int  Obscured (STRAT *s);
extern void DrawFlare (STRAT *s, float amt);

extern void UpdateGameMapEntry(STRAT *strat);

extern void RestartEnvParams(int getOrSet);
extern int PlayerIsAssassin(int);
extern bool BeingBriefed;
extern short LevelNum;
extern int MPStartupState;

extern	float sfxGetCDVolume(void);
extern void sfxSetCDVolume(float volume);

#define IsChallengeLevel() (LevelNum >= 6 && LevelNum < 13)
extern int GameDifficulty;

#endif
