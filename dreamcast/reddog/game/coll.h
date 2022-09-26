#ifndef _COLL_H
#define _COLL_H
#include "strat.h"
#include "render\material.h"

#ifndef MAX_INCLUDED
/* Things MAX doesn't need to worry about */
#define MATRIX_RST	0
#define MATRIX_RS	2
#define MATRIX_R	1

#define POINT3EQUAL(a, b) ((a->x == b->x) && (a->y == b->y) && (a->z == b->z))

typedef struct 
{
	float l, t, r, b;
	CollLeaf	*leaf;
}CollBBox;

typedef struct COLLDATA 
{
	CollNode	*node;
	float		cx, cy, dl, dw;
}CollData;

// A recorded collision
typedef struct tagCollision
{
	Point3		pos;
	Vector3		normal;
	int			type;
} Collision;



typedef struct tag_CollObject
{
	Uint16			nPoly;
	Uint16			corrupt;
	Object			*obj;
	CollPoly		*poly;
	Point3			*v;
	struct tag_CollObject	**child;
}CollObject;

typedef struct tag_HDStrat
{
	STRAT		*strat;
	CollObject	*collObj;
	struct tag_HDStrat *next;
	struct tag_HDStrat *prev;
	int			valid;
}HDStrat;

#define MAX_ACTIVE_COLLISIONS	128	// Don't change this number wihtout altering the coll flags
extern Collision collArray[MAX_ACTIVE_COLLISIONS];
extern int nActiveCollisions;

extern HDStrat		*firstHDStrat;
extern HDStrat		*lastHDStrat;
extern	Point3 GlobalCollP;
extern 	Vector3 GlobalCollPolyN;
extern	CollPoly	*vccrntPoly;
extern void StratCollisionFloor(STRAT *crntStrat);
extern void stratCollisionStrat(STRAT *crntStrat);
extern void mInvertRot(Matrix *a, Matrix *b);
extern void clearFloorCollFlag(STRAT *crntStrat, Object *obj);
extern void clearStratCollFlag(STRAT *crntStrat);
extern void singularMatrix(Matrix *m);
extern STRAT *targetStratCollision(Object **obj, int pn);
extern Bool lineLandscapeCollision(Point3 *ps, Point3 *pe, Point3 *p, STRAT *strat, Vector3 *n);
extern	void findCollBBox(Point3 *cp, CollBBox *cb);
extern	Bool lineBBocColl(CollLeaf *leaf, Vector3 *from, Vector3 *to, float radius);
extern int GlobalPolyType;
extern void HurtStrat (STRAT *hurtee, STRAT *hurter);
extern void ClearCollGlobals();
extern float pointLineDistance(Point3 *point, Point3 *lStart, Point3 *lEnd);
extern float pointLineDistanceFix(Point3 *point, Point3 *lStart, Point3 *lEnd);
extern Object *lineStratCollision(Point3 *from, Point3 *to, STRAT *strat, int pn);
extern void fromObjectToWorld(Matrix *m, STRAT *strat, Object *obj, int a);
extern	void AddToObjectWM(STRAT *strat, Vector3 *v);
//extern int HDCSIZE,HDMODELSALLOC;
#endif


#endif

