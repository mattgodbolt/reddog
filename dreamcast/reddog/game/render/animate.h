/*
 * Animate.h
 * The animation / object heirarchy routines
 */

#ifndef _ANIMATE_H
#define _ANIMATE_H
#include "Render\Quat.h"  


/*
 * Load an object from the given stream with PNode information
 * node should point to an address of a PNode structure
 * in which pNode information is stored.  If NULL, then a PNode
 * is allocated first. The memory
 * for the name and SubObjectId of the pnode is allocated
 * from alloc(ptr).If you are not interested in such 
 * information, then pass NULL
 * It is the caller's responsibility to free up the object
 * and any PNode information
 */
/* ObjectLoad discarding pNode information (backwards compatibilty) */

#define MAX_SUB_OBJECTS 64


typedef union animfield
{
	Uint32 intfield;
	float floatfield;
}	ANIMFIELD;





typedef struct tagAnimation
{
	Uint32	type;							/* Animation type */
#define ANIM_TEXTURE		0	
#define ANIM_MATRIX			1
#define ANIM_MORPH			2
#define ANIM_WATEREFFECT	3
#define ANIM_MODELANIM		4
#define INTERPOLATE_ANIM	8
#define SCALE_ANIM			16


	void	*typeData;						/* Opaque pointer to type data */
	char	name[16];						/* 15 bytes max, + terminator */

	Uint32	firstFrame;
	Uint32	lastFrame;						/* The last frame of this animation */
	Uint32	frameDelay;						/* The delay between each frame in 1/60Hz units */
} Animation;

/*
 * An animation within a strat
 */

typedef struct tagStratAnim
{
	Animation	*anim;						/* Currently playing anim, or NULL */

	Uint32	frame;							/* The current frame of this animation */
	Uint32	curDelay;						/* Counter of wait states per frame */

	float	multiplier;						/* Speed adjustment */

#define ANIM_FLAG_REPEAT					(1<<1)
#define ANIM_FLAG_RUNNING					(1<<2)
#define ANIM_FLAG_BACKWARDS					(1<<3)
#define ANIM_FLAG_PINGPONG					(1<<4) /* Mutually exclusive with REPEAT */
#define ANIM_FLAG_TRIGGER					(1<<5) /* Generate trigger at end of animation */
#define ANIM_FLAG_TRIGGERED					(1<<6) /* Animation has been triggered */
#define ANIM_TWEENING						(1<<7)
#define ANIM_FLAG_PAUSED							(1<<8)
#define ANIM_FLAG_NOTWEEN					(1<<9)
#define ANIM_FLAG_ONCE_DONE					(1<<10)	/*WHEN ONE ITERATION OF THE ANIM HAS BEEN DONE*/


#define MODEL_ANIM							(1<<31)
#define TEXTURE_ANIM						(1<<30)
	Uint32	flags;							/* The flags for this animation */
} StratAnim;

typedef struct AnimInstance
{
	struct tagStratAnim  anim;		/*ANIM COPY	 */
	struct AnimInstance *next;
}ANIMINST;

typedef struct ModelAnimInstance
{
	struct tagAnimation  anim;		/*ANIM COPY	 */
	struct ModelAnimInstance *next;
}MODELANIMINST;

#define SLOWINTERP	0
#define	FASTINTERP	1


typedef struct keyjump
{
	char	name[32];	/*sub-anim name
						  DEPENDING ON SOVING AT LOAD TIME, OR STRAT COMPILE TIME
						  THIS MAY BE STORED INGAME OR SKIPPED AT .RDO LOAD TIME
						  AND REMOVED FORM THIS STRUCTURE*/
	short	offset;		/*offset into the models anim data for the above named sub-anim*/
	short   numframes;	/*number of total frames in this sub-anim*/
	//short	key[16];	/*Key frame array*/
   	short numkeys;
	short *key;
	/*EG : A KEYJUMP OF REDDOGWALK, OFFSET 0, NUMFRAMES 15 WITH KEYS OF 0,5,15, TELLS
		  US THAT REDDOGWALK STARTS AT 0 OFFSET AND RUNS FOR 15 FRAMES WITH THREE KEYFRAMES AT FRAMES 0, 5
		  AND 15*/
}KEYJUMP;


typedef struct interp
{
  /* 	RotVect  rot; */
  //	Point3 scale;  
	Vector3 trans; 
					/*DELTA VALUES TO NEXT KEY FRAME*/
   /*	RotVect drot;*/	/*IF PLAYBACK SPEED BECOMES AN ISSUE WE'LL PUT THE ROT STUFF BACK IN*/	
   //	Point3 dscale;	   
	Vector3 dtrans;	  
	QUAT quat;
   /*	float	x;
	float	y;
	float	z;
	float	w;*/
					/*CURRENTLY 40 BYTES PER KEY FRAME PER OBJECT NEEDED,
					  WITH ROTS AND DROTS, 88 IS NEEDED*/		 
} INTERP;
 
typedef struct sinterp
{
  /* 	RotVect  rot; */
	Point3 scale; 
	Vector3 trans; 
					/*DELTA VALUES TO NEXT KEY FRAME*/
   /*	RotVect drot;*/	/*IF PLAYBACK SPEED BECOMES AN ISSUE WE'LL PUT THE ROT STUFF BACK IN*/	
	Point3 dscale;
	Vector3 dtrans;	  
	QUAT quat;
   /*	float	x;
	float	y;
	float	z;
	float	w;*/
					/*CURRENTLY 64 BYTES PER KEY FRAME PER OBJECT NEEDED,
					  WITH ROTS AND DROTS, 88 IS NEEDED*/		 
} SINTERP;


  typedef union animpoints
{
	Matrix *matrices;
	INTERP *interpolate;
	SINTERP *sinterpolate;	
}	ANIMPOINT;

							   
typedef struct ModelAnim
{
	short	numsubmods;
	short	numkeyframes;
	ANIMPOINT    animdata;
	/*
	Matrix *matrices;
	INTERP *interpolate; */
	KEYJUMP *KeyFrames;

}MODELANIM;



/*
 * An animation block
 */
typedef struct tagAnimBlock
{
	Animation				*anim;
	Uint16					nAnims;
} AnimBlock;


/*
 * The % amount needed to move to go from 60 to 50Hz mode
 */
//#define PAL_SCALE (PAL ? (60.f / 50.f) : 1.f)
//#define PAL_MOVESCALE	1.2f
//#define PAL_ACCSCALE	PAL_MOVESCALE * PAL_MOVESCALE
#define PAL_SCALE (PAL ? (1.2f) : 1.0f)
#define PAL_MOVESCALE	1.2f
#define PAL_ACCSCALE	PAL_MOVESCALE * PAL_MOVESCALE


#endif
