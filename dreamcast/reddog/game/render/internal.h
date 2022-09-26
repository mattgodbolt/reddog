/*
 * Render/Internal.h
 * Internal functions etc
 */

#ifndef _RENDER_INTERNAL_H
#define _RENDER_INTERNAL_H

#include "Render\Lighting.h"

/* Initialise a context */
void InitContext (KMVERTEXCONTEXT *context);

/* Initialise the vertex render states */
void InitContexts (void);

/*
 * Prepare a context for rendering
 */
void rPrepareContext (KMVERTEXCONTEXT *context, Material *mat, Bool Pasted);

/*
 * Sets the current context, if necessary
 */
void rSetContext (KMVERTEXCONTEXT *context);

extern KMVERTEXCONTEXT		bkContext;				/* Used for plotting the background */
extern KMVERTEXCONTEXT		objContext;				/* Was used for drawing objects */
extern KMVERTEXCONTEXT		newObjContext;				/* Now used for drawing objects */
extern KMVERTEXCONTEXT		scapeContext;			/* Used for drawing landscapes */
extern KMVERTEXCONTEXT		sfxContext;				/* Used for special effects */
extern KMVERTEXCONTEXT		spriteContext;			/* Used for sprites */
extern KMVERTEXCONTEXT		shadowContext[3];		/* Used for shadows */
extern KMVERTEXCONTEXT		dummyContext;			/* Used for fast setting */

extern KMVERTEXBUFFDESC		vertBuf;				/* The vertex buffer */

extern Matrix				mPersp;					/* Perspectivization matrix */
extern Matrix				mWorldToView;			/* The view to world matrix */
extern Matrix				mWorldToScreen;			/* The world to screen matrix */

/*
 * Colour clamping values
 */
extern Uint32 rColClampMin;
extern Uint32 rColClampMax;

/*
 * Load a material from the given stream
 */
void rLoadMaterial (Material *mat, Stream *s);

/*
 * Initialise the text subsystem
 */
void tInit (void);

/*
 * Internal texture doobries
 */

/*
 * Initialise the texture subsystem
 */
void texInit (void);

/*
 * A texture animation
 */

/* As it is stored in the file */
typedef struct {
	Uint32		nFrames;			/* The number of frames in this animation */
#define TEXANIM_GLOBAL		1
#define TEXANIM_STRAT		2
#define TEXANIM_OFFSET		3		/* Offset texture animation - 'speed' is frame offset from last loaded texture */
	char		type;
	Uint32		speed;
	struct {
		char			name[16];	/* 15 bytes max, + terminator */
	} stratAnim;
} SerialisedTexAnim;

/* As it is active in the game */
typedef struct tagTexAnim
{
	SerialisedTexAnim		anim;
	Uint32					curFrame;
	Sint32					curCount;
	KMSURFACEDESC			*descArray;			// Used in the case of strat animations
	void					*textureArray;		// Used in the case of global animations, an array of all the frames
	int						nBytesPerTex;		// Number of bytes/texture
} TexAnim;

typedef struct tagTexPaletteEntry
{
	KMSURFACEDESC		desc;
	Uint32				GBIX;
	Uint32				type;
	TexAnim				*animation;
#ifdef _DEBUG
	Uint32				useCount;
#endif
} TexPaletteEntry;

extern TexPaletteEntry texPalette[MAX_TEXTURES];
extern int nTextures;
extern KMVERTEXCONTEXT *rLoadContext;

/*
 * Renderer-specific stuff
 */

/*
 * Unpacks a 32-bit colour into four floats
 */
#ifndef __cplusplus
void rColToLV (LightingValue *colour, Uint32 colour);
void rColToLVMul (LightingValue *colour, Uint32 colour, float *mulBy);
#else
// Can't for the life of me work out what's up with the previous two lines...
#endif
/*
 * Set this material as current
 */
void rSetMaterial (const Material *mat);
void rSetMaterialPasted (const Material *mat);

/*
 * All-new rendering stuff
 */
extern float WnearVal;	/* The near W value */
extern float rWnearVal;	/* 1.f / the near W value */
extern float farDist;	/* The far plane distance in worldspace */

/* A list of rasterisers for a type of material */
enum {
	RASTER_STRIP,						/* Strip-style rasteriser	*/
	RASTER_CLIPPEDSTRIP,				/* Clipped rasteriser		*/
	RASTER_MODELSTRIP,					/* Model-style strip rasteriser */
	RASTER_MODELSTRIP_NEARCLIPPED,		/* Model-style strip rasteriser */
	RASTER_NUM
};

#define RASTER_TRI_TO_STRIP (RASTER_MODELSTRIP - RASTER_MODEL)

/* 
 * Definition of a Strip Rasteriser 
 */
typedef struct {
	PKMDWORD pkmCurrentPtr;
	Uint32	mat;
	float	midX, midY;
	Uint32	*lightingBuffer;
	float	scaleX, scaleY;
	float	rWnearVal;
} StripRasterContext;
extern StripRasterContext theStripRasterContext;
typedef StripHeader *StripRasteriser(StripPos *v, StripEntry *ent, Uint32 num, StripRasterContext *context);

#define OC_OFF_FAR		1
#define OC_OFF_NEAR		2
#define OC_OFF_BOTTOM	4
#define OC_OFF_TOP		8
#define OC_OFF_RIGHT	16
#define OC_OFF_LEFT		32
#define OC_BITS			6
#define OC_N_FAR		64
#define OC_N_NEAR		128
#define OC_N_BOTTOM		256
#define OC_N_TOP		512
#define OC_N_RIGHT		1024
#define OC_N_LEFT		2048
#define OC_NOT			(OC_N_TOP | OC_N_BOTTOM | OC_N_LEFT | OC_N_RIGHT | OC_N_NEAR | OC_N_FAR)
#define OC_OFF			(OC_OFF_TOP | OC_OFF_BOTTOM | OC_OFF_LEFT | OC_OFF_RIGHT | OC_OFF_NEAR | OC_OFF_FAR)
typedef Sint16 Outcode;

typedef struct tagModelContext
{
	VertRef			*strip;					// The strip data (updated)
	UV				*uvArray;				// The uvArray (update)
	Vector3			*vertexNormal;			// Model's vertex normals
	Point3			*vertex;				// Model's vertices
	float			midX, midY;				// Middle point of the screen in the X and Y
	float			intensityOffset;		// The biased intensity offset (1 + ambient)
} ModelContext;

/*
 * Definition of a Model Rasteriser
 */

typedef struct tagRasteriser
{
	void *r[RASTER_NUM];
} Rasteriser;

/*
 * Environment maps a set of points and normals
 * XMTRX must be loaded with model to view
 */
void rPrepEMapVertices (Point3 *points, Vector3 *normals, UV *output, int nVerts,
						float eX, float eY, float eZ);

/*
 * Initialise the noise function
 */
void rInitNoise (void);

/*
 * Read a noise value
 */
float rNoise (float vec[3]);

/*
 * DoGlobalTexAnims - updates all global texture animations
 */
void DoGlobalTexAnims (void);

/* Initialises the lighting subsystem */
void lInit (void);

/*
 * Called at End Of Render
 */
void EORCallback(void *);

/* 
 * Called to mark EOR in the profile bar system
 */
void EORMark (void);

/*
 * Visibility system - is the point x,y,z inside the (0,0,0)/(1,1,1) in the current region
 */
Bool VisIsInside (float x, float y, float z);

/*
 * Initialises the shadow system
 */
void shInit (void);

/*
 * Draws a shadow for a bbox
 */
void shBBox (BBox *bounds);
void shBBoxRound (BBox *bounds);

/*
 * Fire stuff
 */
void rFireCallback(void);
void rProcessFire(void);
void rInitFire(void);

/*
 * View sizes
 */
extern Point2 pViewSize, pViewMidPoint;

#if COUNT_GEOMETRY
extern int nDrawn, nScape, nTris, nQuads, nLit, nLights;
#endif

/*
 * Weather warez
 */
void rResetWeather (void);
void rUpdateWeatherEffects(void);
void rDrawWeatherEffects(void);

/*
 * Special FX decal warez
 */
void rDrawDecals (void);
void rInitDecals (void);

/*
 * Global adjustment of Y values
 */
extern int rGlobalYAdjust;

/*
 * Is the fire texture animation being used?
 */
extern Bool fireUsed;
extern float rFlareR, rFlareG, rFlareB;

/* Set the DMA buffer, default is scratch RAM */
void texSetDMABuffer (char *buffer, int size);

/* Material change support routines */
PKMDWORD CopyMaterialData32 (PKMDWORD pkmCurrentPtr, void *GLOBALPARAMBUFFER);
PKMDWORD CopyMaterialData64 (PKMDWORD pkmCurrentPtr, void *GLOBALPARAMBUFFER, float *col);

#endif
