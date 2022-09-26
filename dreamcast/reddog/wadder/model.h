/*
 * Render/Model.h
 * The model format
 */

#ifndef _MODEL_H
#define _MODEL_H

/*
 * The maximum number of vertices in a model
 */
#define MAX_VERTICES   (65536/sizeof(Point3))

/*
 * A vertex reference
 * This encodes the type of polygon being described, as well as the offset
 * in the model's vertex array to take point data from
 */
typedef Sint16 VertRef;

/*
 * An uncompressed UV value
 */
typedef struct tagUV
{
	float	u, v;
} UV;

/*
 * An axis-aligned bounding box
 */
typedef struct tagBBox
{
	Point3	low, hi;
} BBox;

/*
 * A compressed normal
 */
typedef struct tagCompNormal
{
	Uint8	latitude;
	Uint8	longitude;
} CompNormal;

/*
 * Data referencing each polygon individually
 * Quads are referenced as two triangles
 */
typedef struct tagPolygonData
{
	VertRef		v[3];					/* The vertices of this triangle */
} PolygonData;

/*
 * Bump mapping information
 */
typedef struct tagBumpMapInfo
{
	Point3		biNormal;
	Point3		tangent;
	float		angShift;
} BumpMapInfo;

/*
 * Model Flags
 */
#define MODELFLAG_SHADOW_BBOX		(1<<0)		/* Model throws a bbox shadow */
#define MODELFLAG_UNLIT				(1<<1)		/* Model shouldn't be lit at all */
#define MODELFLAG_SHADOW_ROUND		(1<<2)		/* Model throws a 'round' shadow (takes precedence over bbox shadow) */
#define MODELFLAG_FACEON			(1<<3)		/* Model should always be drawn face on to the camera */
#define MODELFLAG_SPECULAR_FLASH	(1<<4)		/* Model is flashed specularly */
#define MODELFLAG_WATER_EFFECT		(1<<5)		/* Model has the water effect applied to it */
#define MODELFLAG_LAVA_EFFECT		(1<<6)		/* Model has the lava effect applied to it */
#define MODELFLAG_FLAG_EFFECT		((1<<5)|(1<<6)) // Both for flag effect
#define MODELFLAG_ENVIRONMENTMAP	(1<<7)		/* Model requires environment mapping */
#define MODELFLAG_BUMPMAP			(1<<8)		/* Model requires bumpmapping */
#define MODELFLAG_FRONTEND			(1<<9)		/* Special case front end handling */
/* Flags used internally */
#define MODELFLAG_NOCLIPPING		(1<<31)		/* Model doesn't need clipping */
#define MODELFLAG_GLOBALTRANS		(1<<30)		/* Model should be drawn completely as trans polys */
#define MODELFLAG_GLOBALPASTEDOVERRIDE (1<<29)	/* Model's pasted amount should be adjusted */
#define MODELFLAG_GLOBALSUBTRACTIVE	(1<<28)		/* Model should be drawn with subtractive trans */
#define MODELFLAG_CHEAPDRAW			(1<<27)		/* Model has no cunning material and can be trawn trivially */
#define MODELFLAG_STATIC_LIGHT		(1<<26)		/* Special hackimous */
/*
 * A model
 */
typedef struct tagModel
{	
	BBox			bounds;				/* The axis-aligned bounding box of this model */
	Sint16			nVerts;				/* The number of vertices in the model */
	Sint16			nMats;				/* The number of materials in the model */
	Sint16			nTris;				/* The number of triangles */
	Sint16			nQuads;				/* The number of quads */
	Sint16			nStripPolys;		/* The number of polygons coded in the strip list at the end 
	                                     * Used mainly for benchmarking (and loading) */
	Sint16			nStrips;			/* The number of strips themselves */

	Material		*material;			/* Array of materials used in the model */
	Point3			*vertex;			/* Array of vertices */
	Vector3			*vertexNormal;		/* Array of vertex normals */
	Plane			*plane;				/* Array of polygon plane equations */
	UV				*uv;				/* Array of per-polygon-vertex UV values */
	VertRef			*strip;				/* The strip array - nTris triangles followed by nQuads quads 
										   New: Followed by polygon strips till 0:0 iff (nStrips != 0) */
	BumpMapInfo		*bump;				/* Bump map data */
	Uint32			modelFlags;			/* Model flags */
} Model;

/*
 * The header on a model
 */
typedef struct tagModelHeader
{
	Uint32			tag;
#define MODELHEADER_TAG *(Uint32 *)&"MODL"
#define MODELHEADER_VERSION 0x151
	Uint32			version;
} ModelHeader;


/*
 * Load a model from the given stream
 */
Model *ModelLoad (Stream *f, Allocator *alloc, void *ptr);

/*
 * Locks a model's data :
 * Fill in a model's poly field, and ensure the plane data is calculated
 * Memory for the poly data is taken from collCache
 * If the model's plane data has not already been calculated it is, and
 * memory for this taken from the Renderer's internal cache
 * Both caches are locked after this call - you *must* call ModelUnlock
 * before you call any other Renderer functions
 */
void ModelLock (Model *model);

/*
 * Make sure you call this function when you have finished with a model
 */
void ModelUnlock (Model *model);

/*
 * Release a model - discarding any cached information
 * and cleaning up any new topology information.
 * If the topology has changed, then the user is
 * responsible for freeing the old topology information
 * polyCache points to a polygon cache where the 'poly' 
 * information could have been stored
 * compCache points to a data cache where the uncompressed
 * model may have been stored.
 * A NULL deallocator means no deallocation is done
 */
void ModelRelease (Model *model, Deallocator *dealloc, void *ptr);

/*
 * Draw a model!
 */
void rDrawModel (Model *model);
void rDrawModelSimple (Model *model);

/*
 * Apply the water and lava effect to a model:
 * mCurMatrix must be up to date
 */
void rWaterModel (Model *model);
void rLavaModel (Model *model);
void rFlagModel (Model *model);

#endif
