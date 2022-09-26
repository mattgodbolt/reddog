/*
 * Red Dog Types
 */

#ifndef _RDTYPES_H
#define _RDTYPES_H

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

typedef Uint32	TimeStamp;

typedef struct tagPoint2 {
	float x, y;
} Point2, Vector2;

typedef struct tagPoint3 {
	float x, y, z;
} Point3, Vector3;

typedef union {
	Uint32  colour;
	struct {
		Uint8   b;
		Uint8   g;
		Uint8   r;
		Uint8   a;
	} argb;
} Colour;

typedef struct tagRotVect {
	float x, y, z;
} RotVect;

// A matrix is a four by four array of floating point numbers
typedef struct tagMatrix
{
	float	m[4][4];
} Matrix;

// A Matrix43 is a matrix with the last column missing - assumed to be [0001]T
typedef struct tagMatrix43
{
	float	m[4][3];
} Matrix43;

/*
 * Matrix32 for text routines
 */
typedef struct tagMatrix32 {
	float m[3][2];
} Matrix32;

/*
 * A vector (or point) is an array of four floats
 * It is considered a row vector
 */
typedef struct tagVector
{
	float	x, y, z, w;
} Vector, Point, Plane;


typedef struct tagTexName {
	void            *filename;  /* texture filename strings     */
	Uint32               attr;  /* texture attribute            */
	Uint32            texaddr;  /* texture memory list address  */
} TexName;

typedef struct tagTexList {
	TexName			*textures;  /* texture array                */
	Uint32			nbTexture;  /* texture count                */
} TexList;

// A UV area
typedef struct {
	float minX, minY;
	float maxX, maxY;
} Box2D;

#endif
