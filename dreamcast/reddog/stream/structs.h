
/*- STRUCTS.H --------------------------------------------------------------*/


#ifndef _STRUCTS_H
#define _STRUCTS_H


#ifndef FALSE
	#define FALSE 0
	#define TRUE  1
#endif


#if 0
//copied from the game so we don't have to mess around with rdtypes.h includes
typedef union tagPoint2 {
	float v[2];
	float p[2];
	struct {
		float x, y;
	};
} Point2, Vector2;



typedef union tagPoint3 {
	float v[3];
	float p[3];
	struct {
		float x, y, z;
	};
} Point3, Vector3;
#endif


#ifndef Sint32 
	#define Sint32 int
	#define Uint32 unsigned int
	#define Sint16 short
	#define Uint16 unsigned short
	#define Uint8 unsigned char
	#define ubyte	unsigned char
	#define ulong	unsigned long
	#define uword	unsigned short
#endif


#define CLIP   9999

//hack
#define PDS_PERIPHERAL Uint16

typedef float	MATRIX[4][4];

typedef struct angvect
{
	double vx;
	double vy;
	double vz;
}	ANGLEVECT;


typedef struct stratobj
{
	float vx;	
	float vy;	
	float vz;
	float xpos;	
	float ypos;	
	float zpos;
	ANGLEVECT rot;
	MATRIX rotmat;	
	Uint16 internal;
	Uint16 *stackptr;
	Uint16 timer[4];
	Uint16 stack[4];
	struct strip *Mesh;
	void (*func)(struct stratobj *strat);
	void (*callback)(struct stratobj *strat);
}	STRATOBJ;




/****************************************************************************/
/*									3D	LIBRARY TYPES											 */
/****************************************************************************/


typedef struct vector
{
	float vx;
	float vy;
	float vz;
} VECTOR;

#define VERTEX VECTOR

typedef struct normal
{
	Sint16 vx;
	Sint16 vy;
	Sint16 vz;
} NORMAL;


typedef	struct face
{
	Uint16	numside;
	Uint16 	vertices[10];
}	FACE;




#if 0
typedef struct
{
	Sint8 vx;
	Sint8 vy;
	Sint8 vz;
//	Sint8 pad;

} SMALLVECTOR;

typedef struct nsvector
{
	Sint8 vx;
	Sint8 vy;
	Sint8 vz;
	Sint8 pad;

} NSVECTOR;

typedef struct s8vector
{
	Sint16 vx;
	Sint16 vy;
	Sint16 vz;
	Sint16 pad;

} S8VECTOR;

typedef struct stratvector
{
	Sint16 vx;
	Sint16 vy;
	Sint16 vz;
	Uint16 pad;

} STRATVECTOR;

typedef	VECTOR			SVECTOR;

/*typedef	struct  vector
{
	Fixed32	x;
	Fixed32	y;
	Fixed32	z;

}VECTOR;*/

#define	POINT	VECTOR

typedef	Fixed32			RotMatrix[3][3];
typedef	Fixed32			TransMatrix[3][4];
typedef	Sint16			STransMatrix[3][4];
typedef	TransMatrix		MATRIX;
typedef	STransMatrix	SMATRIX;


typedef	struct {
	Uint16	Address;			/*THE VRAM ADDRESS*/	
	Uint16	Size;				/*THE FORMAT OF THE X/Y RANGES*/
	Uint16	LookAddress;	/*THE VRAM LOOKUP ADDRESS*/	
 	Uint16	pad;	/*THE VRAM LOOKUP ADDRESS*/	
	Uint8		pad2;
}TEXTURE;



typedef	struct	polygon
{
//	VECTOR	norm;				/*POLYGON'S FACE NORMAL*/
	Uint16	vertices[4];	/*VERTEX LIST*/
//   Uint32	texno ;		/* テクスチャ番号 */
//   Uint16	colour ;		/* テクスチャ番号 */
//   Uint16	flags ;		/* テクスチャ番号 */
}POLYGON;

typedef	struct	polygon2
{
	VECTOR	norm;				/*POLYGON'S FACE NORMAL*/
   Uint32	texno ;		/* テクスチャ番号 */
	Uint16	vertices[4];	/*VERTEX LIST*/
   Uint16	colour ;		/* テクスチャ番号 */
   Uint16	flags ;		/* テクスチャ番号 */
}POLYGON2;

typedef	struct	vertnorm
{
	VECTOR	norm[4];			/*POLYGON'S 4 VERTEX NORMALS*/
}VERTNORM;

typedef struct {

/*    Uint16	 texno ;		
    Uint16	 colno ;		
    Uint16	 dir 	;		
    Uint8	 flag ;		
    Uint8	 pad 	;*/		
} ATTR ;


typedef struct {
	Sint32 xm;			/* 4.12 x multiplier */
	Sint32 zm;			/* 4.12 y multiplier */
	Sint32 c;			/* 20.12 constant */
} BOUND;

typedef struct {
	Uint8 flags;		/* COLL_FLAT, COLL_EQN, COLL_QUAD etc. */
	Uint8 surface;		/* collision flags for the surfaces*/
	Uint16 eflags;		/* face number in object of polygon */
	BOUND boundary[4];	/* for face boundary check */
	BOUND plane;		/* plane equation */
} FACE_COLL;

typedef struct packet
{
	Uint16  control;      /* control word                     */
   Uint8	 flag ;		
   Uint8	 pad 	;		
   Uint16  drawmode;     /* draw mode                        */
   Uint16  colour;        /* color info.                      */
   Uint16  charAddr;     /* character address                */
   Uint16  charSize;     /* character size                   */
//   Uint16  link;         /* command link                     */
} PACKET ;
					  


typedef struct {
	Fixed32		radius;		/* Model radius */
	Uint16		nPoint;		/* Number of points */
	POINT			*tPoint;		/* Point table */
	SMALLVECTOR		*tNormal;	/* Point normal table (for lighting) */
	Uint16		nPoly;		/* Number of polys */
	POLYGON		*tPoly;		/* Polygon table */
	PACKET		*tAttr;		/* Polygon attribute table (default) */
	Uint16 nfloor;		/* number of floor collision faces */
	Uint16 nceil;		/* number of ceiling collision faces */
	FACE_COLL *lcoll;			/* list of collision faces */
	VECTOR	bbox;			/* Bounding box */

	Uint16	*Shade;
	SMALLVECTOR	*fNormal;	/* Face normal table (for lighting) */
	Uint16		Double;
} OBJECT;


typedef struct model_struct {
							/* full model radius */
	Uint16 nobj;		/* number of objects in model */
//	Uint16 flags;		/* model / object flags */
	Uint16 index;		/* start index of objects within the object list*/
//	OBJECT *lobj;		/* list of objects */
} MODEL;

typedef struct
{
	VECTOR rot;
	VECTOR trn;
} POS;

typedef struct
{
	STRATVECTOR rot;
	VECTOR trn;
} STRATPOS;

typedef	struct transform
{
	VECTOR scale;		
	VECTOR translate;
	RotMatrix rotate;
	VECTOR rot;
}TRANSFORM;

typedef	struct strattransform
{
	VECTOR scale;		
	VECTOR translate;
	RotMatrix rotate;
	STRATVECTOR rot;
}STRATTRANSFORM;

typedef	struct PreSprite {                    /* Sprite Command Table             */
    PACKET  *packet;                    /* control word                     */
    Sint16  ax;                         /* point A x                        */
    Sint16  ay;                         /* point A y                        */
    Sint16  bx;                         /* point B x                        */
    Sint16  by;                         /* point B y                        */
    Sint16  cx;                         /* point C x                        */
    Sint16  cy;                         /* point C y                        */
    Sint16  dx;                         /* point D x                        */
    Sint16  dy;                         /* point D y                        */
    Uint16  grshAddr;                   /* gouraud shading table address    */
//    Uint16  dummy;                      /* dummy area                       */
	 struct PreSprite	*Link;
} PRESPRITE;

/*ANIM ENUMERATIONS*/

/*enum { ANIM_NULL, ANIM_START, ANIM_WALK, ANIM_STOP, ANIM_LEFT, ANIM_RIGHT, 
	   ANIM_WALK2RUN, ANIM_RUN, ANIM_SKID, ANIM_SWIM, ANIM_SWIMEND,
	   ANIM_SWIMWAIT, ANIM_JUMP, ANIM_PLAY };

enum { JUMP_NOT = 0, JUMP_TAKEOFF, JUMP_RISE, JUMP_APEX, JUMP_FALL, JUMP_LAND };
*/


/****************************************************************************/
/****************************************************************************/
/*									OBJECT LIBRARY TYPES											 */
/****************************************************************************/



typedef struct models
{
/*	PDATA	*model;*/		/*POINTER TO BASE MODEL*/
/*	short	maxallowed;*/	/*MAXIMUM ALLOWED NUMBER OF THIS MODEL'S INSTANCES*/
} MODELS;


typedef struct {
	Uint8	frame;
	Uint8	trigger;
} ATRIG;


typedef struct {
	Uint32	speed;		/* Value to add to the anim counters for speeds */
	Uint32	ntrigger;	/* Number of triggers */
	ATRIG	*ltrigger;	/* list of trigger bit patterns and frames */
	Uint32	nframes;	/* Number of frames */
	STRATPOS 	*loffset;	/* List of object offsets */
	Uint32	nvert;		/* Number of morphing vertices */
	NSVECTOR **llvert;	/* List of list of vertices (frames * verts) */
	Uint32	nobj;		/* Number of rotating objects */
	SMATRIX  **llmat;	/* List of list of matrices (frames * objs) */
} ANIM;


/* INTRO ANIM STUFF*/

typedef struct {
    Uint32 model;
	 Sint32 scale;
    MATRIX mat;
} TFORM;

typedef struct {
    Uint32 model;
	 Sint32 scale;
    MATRIX mat;
    Uint32 nvert;       /* number of vertices in morphed model */
    SVECTOR *lvert;     /* new list of vertices for morphed model */
} MORPH;


typedef struct {
    VECTOR cam_pos;
    STRATVECTOR cam_rot;
} CAMERA2;

typedef struct {
    Uint32 nframe;
    Uint32 blanks;      /* number of blank bytes at the end of a frame */
    CAMERA2 cam;
    Uint32 ntform;
    TFORM *ltform;
    Uint32 nmorph;
    MORPH *lmorph;
} FRAME;


typedef struct {
    FRAME *lframe;
} ANIM2;



/* DEFINITION OF A MODEL'S INSTANTIATION BLOCK*/

typedef	struct ModelInstance
{							
	Uint16			*Shade;
	OBJECT			*model;			/*OBJECT'S BASE MODEL*/
  	STRATTRANSFORM 		trans;			/*OBJECT'S SCALE,ROTATE AND TRANSLATION*/
	Uint8			draw;
	Sint8			z;
}MODELINSTANCE;

typedef	struct SpriteInstance
{							
	TRANSFORM 		trans;			/*OBJECT'S SCALE,ROTATE AND TRANSLATION*/
	ANIM				*anim;
	Uint32			offset;
	Uint32			frame;
	Uint16			texture;
}SPRITEINSTANCE;

typedef	struct Instance
{							
	ubyte			 	assigned;
	MODELINSTANCE	*Block;
}INSTANCE;


/* SOME SHIT LEFT AROUND FROM CSDX*/

/* BASIC STRUCTURE OF OBJECT ENGINE'S LINKED LIST*/

struct objnode
{							
	struct objnode *prev;
	struct objchunk *chunk;
	struct objnode *next;
};


struct spobjnode
{											/* struct for sprite objects */
/*	POLY_FT4 sprite;*/
	int	assigned;
	void (*function)(struct objnode *obj); 		/*Runtime address of object*/
};

//struct spobjnode	spritetable[2][NUM_SPRITE];	/*Active sprite table*/



/* OBJECT INSTANCE BLOCK */

typedef struct objchunk
{						
	int list;			/*INDEX WITHIN MODEL INSTANCE TABLE*/							
	short	objtype;		/*OBJECT TYPE, GIVES THE REF. NUMBER TO REQD MODEL TABLE*/
	TRANSFORM 		trans;	/*OBJECT'S SCALE,ROTATE AND TRANSLATION*/
/*	short	state;*/
/*	void (*function)(struct objnode *obj);*/ /*Runtime address of object*/
/*	unsigned long	**shapemod;*/
/*	struct objnode	*parent;*/
/*	char	hit_flag;*/
}OBJCHUNK;

#endif

#endif


