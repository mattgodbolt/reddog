#ifndef _SHADOW_H
#define _SHADOW_H

/*
 * Real shadows for Red Dog
 */

#define MAX_SHADOW_VERTS	82

// A shadow-casting face
typedef struct tagShadFace
{
	Sint8				culled;			// Has this face been culled?
	Sint8				p[3];			// The three vertices
	struct tagShadEdge	*edges[3];		// The three edges surrounding this face
	Vector3				faceNormal;		// The face normal of this face
} ShadFace;

// An edge of a shadow-casting face
typedef struct tagShadEdge
{
	ShadFace	*face[2];				// The two faces on either side of this edge
	Sint8		v0;						// The vertex number in face 0 of the first in face 0
	Sint8		v1;						// (v0+1) % 3
} ShadEdge;

// A shadow hull - a convex hull of a model of shadow-casting triangles
typedef struct tagShadHull
{
	int			nFaces;					// Number of faces 
	int			nEdges;					// Number of edges - will be (3*nFaces / 2)
	ShadFace	*face;					// Face array
	ShadEdge	*edge;					// Edge array
} ShadHull;

// A shadow model is a set of hulls
typedef struct tagShadModel
{
	int			nHulls;					// The number of shadow hulls to load
	int			nVertices;				// The number of vertices in the shadow model
	ShadHull	*hull;					// The array of hulls themselves
	Point3		*vertex;				// The vertices
} ShadModel;

// Load a shadow model into the Mission heap
ShadModel *rLoadShadModel (Stream *s);

// Calculate a shadow model using mCurMatrix
void rCalcShadModel (ShadModel *model, int hiQuality);

// Draw the currently calculated shadow with mCurMatrix
void rDrawShadModel (ShadModel *model);

// Draw the special-cased tank shadows
void DoTankShads(int pn);

// Sigh
void rShadowWorkaround(void);

// Shadow models
extern ShadModel *wheelShadow, *backChassisShadow, *gunShadow, *frontChassisShadow, *cubeShadow, *sphereShadow;

#endif
