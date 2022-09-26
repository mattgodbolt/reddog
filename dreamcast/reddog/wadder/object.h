/*
 * Object.h
 * Object heirarchy
 */

#ifndef _OBJECT_H
#define _OBJECT_H


typedef struct tagCP1
{
	Point3	centre;
	float	radius;
} CP1;

typedef struct tagCP2
{
	Point3	centre;
	Point3	worldPos;
	float	radius;
} CP2;

typedef struct tagobject
{
	/*
	 * Exported/imported values
	 */
	Model				*model;				/* Object's model (if any) */
	struct tagobject	*child;				/* Pointer to child array */
	RotVect				idleRot;			/* Object's current rotation */
	Vector3				scale;				/* Object's current scale */
	Point3				idlePos;			/* Object's 'idle' position */
	Uint8 				ncp;				/* Number of collision points */
	Uint8 				no_child;			/* Number of children */
	CP2					*cp;				/* Array of collision points */
  //	Uint32 				cpType;				/* max of 32 cps, 1 = Rotatable, 0 = static */
  	Colour				specularCol;	   	/*SPECULAR COLOUR RANGE*/
	Matrix				m;					/* The current matrix (potentially calculated from crntPos and crntRot) */

	/*
	 * Values used during game
	 */
  	Matrix43			im;
	Vector3				animscale;			/*used in scale animations*/
	float				health;				//objects health
	Matrix				wm;
	struct tagobject	*parent;
	Point3				*ocpt;
	Uint32				collFlag;			/* Coll & object flags */
  	float				slerpDelta;
	float				slerpFrame;
	float               transparency;		 /*DEGREE OF TRANSLUCENCY*/
	Point3				crntPos;
	RotVect				crntRot;			/* Object's current rotation */
	Vector3				initPos;
	RotVect				initRot;
} Object;

typedef struct tagObjectHeader
{
	Uint32				tag;
#define OBJECTHEADER_TAG *(Uint32 *)&"OBJT"
/*#define OBJECTHEADER_VERSION	0x191*/			/* Change this if you change the object structure */
 #define OBJECTHEADER_VERSION	0x192
	Uint32				version;
	Uint32				nameLen;
} ObjectHeader;


typedef struct pnode
{
	Object					*obj;
	Object					**objId;
	int						noObjId;
	float					mass;
	float					radius;
	char					*name;
	float					collForceRatio;
	Uint32					flag;			/*STRAT Flags */
	Uint16					typeId;
	Point3					com;
	Vector3					groundFriction;
	Vector3					airFriction;
	float					expRot;
 /*	struct	tagAnimBlock	*GeomAnim;*/
	struct tagAnimBlock		*anim;
}	PNode;

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
PNode *PNodeLoad (PNode *node, Stream *s, Allocator *alloc, void *ptr);

#define MAX_SUB_OBJECTS 64

/*
 * Render an object
 */
void rDrawObject (Object *obj);

/*
 * Cleans up a PNode and frees the information in it
 * NB Does >NOT< free the memory for the PNode itself -
 * it is up to the caller to ensure this memory is released
 * in the appropriate manner
 */
void PNodeFree (PNode *node, Deallocator *dealloc, void *ptr);

/*
 * Fix up an object, after loading in the relevant textures
 */
void rFixUpObject (Object *obj);


extern void SetTweenKey(PNode *node,Object *obj,StratAnim *curAnim);
extern void CombineAnim(PNode *node, StratAnim *curAnim, Object *rootobj, Object *reqobj);
  

#endif
