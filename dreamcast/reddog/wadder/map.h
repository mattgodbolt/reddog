/*
 * Map.h
 * The map format
 */

#ifndef _MAP_H
#define _MAP_H

/*
 * An entry in the vert/colour list
 */
typedef struct tagStripPos
{
	Point3		p;
	Uint32		colour;
} StripPos;

#ifndef _SCAPEMODEL_H
	#ifdef WADDER
		#include "ScapeModel.h"
	#else
		#include "Render\\ScapeModel.h"
	#endif
#endif

/* The header of the map */
typedef struct _MAPHEADER {
#define MAPHEADER_TAG		(*((Uint32 *)&"RDMP"))
#define MAPHEADER_VERSION	0x110
	Uint32		tag;			/* Tag used in sanity checking */
	Uint32		version;		/* Version of the map, 0x100 being version 1, 0x150 being 1.5 etc */
} MapHeader;

typedef struct tagCollNode
{
	struct tagCollNode *left;
	struct tagCollNode *right;
} CollNode;

typedef struct 
{
	Uint16	noPoly;
	Uint16	height;
	Uint16	*polyNumber;
} CollLeaf;

typedef struct collPoly
{
	Uint16	flag;
	Uint16	v[3];
	Vector3	n;
	float	D;
} CollPoly;

/* A shadow box */
typedef struct tagShadBox {
	float	rot[3*3]; /* 3x3 rotation matrix */
	float	trans[3]; /* 3 translation vectors */
} ShadBox;

typedef union tagBoxTree
{
	struct {
		void			*tag;	// == NULL to mark as a leaf
		ScapeModel		*model; // == model number on export/load
	} leaf;
	struct {
		union tagBoxTree	*child[4];
		BBox			bounds;
	} node;
} BoxTree;


/*
 * A visibility box
 * NB this is not axis aligned in world space, rather it is a
 * box bounded by (0,0,0) and (1,1,1) in its own space
 * So the box itself is stored as a matrix.  Neat, huh?
 * In fact, it is stored as its inverse matrix - so multiplying a world
 * space point by the matrix and testing to see if it is within (0,1)[x,y,z]
 * is all we have to do to get arbitrary world-space bounding box collision!!!
 */
typedef struct tagVisBox
{
	Matrix		worldToBoxSpace;	/* The world to box-space matrix */
	Uint32		ID;					/* The (0,31) identifier */
	Uint32		canAlsoSee;			/* The bitmask of other boxes that can be seen from this box,
									 * if 0, then none */
} VisBox;

/* The map as stored */
typedef struct _MAP {
	MapHeader		header;
	Uint32			numBoxes;		/* The number of bounding boxes */
	Uint32			numMaterials;	/* The number of materials */
	Material		*material;		/* The pointer to the material array */
	ScapeModel		*box;			/* The pointer to the box array */
	CollNode		*topNode;		/* Pointer to the top node */
	float			xScale, yScale;	/* Grid scale (width and height of each box) */
	float			xMax, yMax;		/* Number of bounding boxes in each direction */
	float			xMin, yMin;		/* Minimum x and y vertex value in landscape */
	CollPoly		*collPoly;		/* The collision polys */
	StripPos		**collLookup;	/* The collision lookup list */
	Uint32			numVisBoxes;	/* The number of visibility boxes */
	VisBox			*visBox;		/* The visibility box array */
	ShadBox			*shadBox;
	BoxTree			*hierarchy;		/* The box tree */
	/* Followed by numMaterials materials, then
	 * the v array, then
	 * numBoxes boxes */
} Map;


/*
 * Load a map in, using the MapHeap
 */
Map *MapLoad (Stream *s);

#endif