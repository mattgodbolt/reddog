/*
 * RedDog.h
 * All source files in the Red Dog project should include
 * this header, as configuration parameters are stored within
 */



#ifndef _REDDOG_H
#define _REDDOG_H


// FUCKING COMPILER BUGS!!!!!!!!
#define memcpy MGmemcpy
//#include <shinobi.h>

//bits needed from shinobi
//#include <machine.h>      /* SHC Builtin Functions                    */
//#include <sg_gd.h>        /* GD Library                               */
//#include <sg_xpt.h>

//#include <mathf.h>
//#include <math.h>
//BITS NEEDED FROM MATHF
#ifndef _MATHF
#define _MATHF

#ifdef __cplusplus
extern "C" {
#endif
extern float frexpf(float, int *);
extern float ldexpf(float, int);
extern float modff(float, float *);
extern float ceilf(float);
#if defined(_SH2E)|defined(_SH3E)|defined(_SH4)
#ifdef _FPD
#define  fabsf  _builtin_fabs
#else
#define  fabsf  _builtin_fabsf
#endif
#else
extern float fabsf(float);
#endif
extern float floorf(float);
extern float fmodf(float, float);
extern float acosf(float);
extern float asinf(float);
extern float atanf(float);
extern float atan2f(float,float);
extern float cosf(float);
extern float sinf(float);
extern float tanf(float);
extern float coshf(float);
extern float tanhf(float);
extern float sinhf(float);
extern float expf(float);
extern float logf(float);
extern float log10f(float);
extern float powf(float,float);
#if defined(_SH3E)|defined(_SH4)
#ifdef _FPD
#define  sqrtf  _builtin_sqrt
#else
#define  sqrtf  _builtin_sqrtf
#endif
#else
extern float sqrtf(float);
#endif
#ifdef __cplusplus
}
#endif

#endif
 
//ENDOF BITS NEEDED FROM MATHF

#define  NULL           ((void *)0)

//BITS NEEDED FROM MATH
#ifndef _MATH
#define _MATH

#ifndef ERANGE
#define ERANGE  1100
#endif

#ifndef EDOM
#define EDOM    1101
#endif

#ifndef ENUM
#define ENUM    1208
#endif

extern volatile int _errno;
extern const double _HUGE_VAL;
#define HUGE_VAL  _HUGE_VAL

#ifdef __cplusplus
extern "C" {
#endif
extern double frexp(double, int *);
extern double ldexp(double, int );
extern double modf(double, double *);
extern double ceil(double);

#ifdef _SH4
#ifdef _FPS
#define  fabs  _builtin_fabsf
#else
#define  fabs  _builtin_fabs
#endif
#else
#if defined(_SH2E)|defined(_SH3E)
#ifdef _FLT
#define  fabs  _builtin_fabsf
#else
extern double fabs(double);
#endif
#else
extern double fabs(double);
#endif
#endif

extern double floor(double);
extern double fmod(double, double);

extern double acos(double);
extern double asin(double);
extern double atan(double);
extern double atan2(double,double);
extern double cos(double);
extern double sin(double);
extern double tan(double);
extern double cosh(double);
extern double tanh(double);
extern double sinh(double);
extern double exp(double);
extern double log(double);
extern double log10(double);
extern double pow(double,double);

#ifdef _COMPLEX_
extern double sqrt(double);
#else
#ifdef _SH4
#ifdef _FPS
#define  sqrt  _builtin_sqrtf
#else
#define  sqrt  _builtin_sqrt
#endif
#else
#ifdef _SH3E
#ifdef _FLT
#define  sqrt  _builtin_sqrtf
#else
extern double sqrt(double);
#endif
#else
extern double sqrt(double);
#endif
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif

//ENDOF BITS NEEDED FROM MATH


//BITS NEEDED FROM SG_XPT.H

/****** GLOBAL DECLARATION **********************************************/
#ifndef _TYPEDEF_Uint8
#define _TYPEDEF_Uint8
typedef unsigned char     Uint8;        /*  unsigned 1 byte integer     */
#endif
#ifndef _TYPEDEF_Sint8
#define _TYPEDEF_Sint8
typedef signed char     Sint8;          /*  signed 1 byte integer       */
#endif
#ifndef _TYPEDEF_Uint16
#define _TYPEDEF_Uint16
typedef unsigned short  Uint16;         /*  unsigned 2 byte integer     */
#endif
#ifndef _TYPEDEF_Sint16
#define _TYPEDEF_Sint16
typedef signed short    Sint16;         /*  signed 2 byte integer       */
#endif
#ifndef _TYPEDEF_Uint32
#define _TYPEDEF_Uint32
typedef unsigned long   Uint32;         /*  unsigned 4 byte integer     */
#endif
#ifndef _TYPEDEF_Sint32
#define _TYPEDEF_Sint32
typedef signed long     Sint32;         /*  signed 4 byte integer       */
#endif
#ifndef _TYPEDEF_Float32
#define _TYPEDEF_Float32
typedef float           Float32;        /*  4 byte real number          */
#endif
#ifndef _TYPEDEF_Float64
#define _TYPEDEF_Float64
typedef double          Float64;        /*  8 byte real number          */
#endif
#ifndef _TYPEDEF_Float
#define _TYPEDEF_Float
typedef float           Float;          /*  4 byte real number          */
#endif
#ifndef _TYPEDEF_Double
#define _TYPEDEF_Double
typedef double           Double;        /*  8 byte real number          */
#endif
#ifndef _TYPEDEF_Void
#define _TYPEDEF_Void
typedef void            Void;           /*  void                        */
#endif
#ifdef __SHC__
#ifndef __cplusplus
enum bool
{
    false,
    true
};
typedef enum bool   bool;
#endif
#endif

#ifndef _TYPEDEF_Bool
#define _TYPEDEF_Bool
typedef Sint32 Bool ;					/*  Bool                        */
#endif

//END OF BITS NEEDED FROM SG_XPT.H



//BITS NEEDED FROM SG_GD
/* Sector size (Mode 1) */
#define GDD_FS_SCTSIZE  (2048)

typedef struct GDS_FS_WORK GDFS_WORK;

typedef struct GDS_FS_HANDLE {
	GDFS_WORK *wk;		/* pointer of lib work */
	Sint32 fid;		/* file id */
	Sint32 fad;		/* fad */
	Sint32 fsize;		/* file size */
	Sint32 fsctsize;		/* sctor size of the file */
	Sint32 ofs;		/* offset */
	Sint32 trnsed;		/* transfered size */
	Sint32 rsize;		/* reading size */
	Sint32 trsize;		/* transfering size */
	void (*rdendcb)(void *);		/* read_end callback */
	void *rdcb_1st;		/* read_end callback 1st arg. */
	void (*trendcb)(void *);		/* trans_end callback */
	void *trcb_1st;		/* trans_end callback 1st arg. */
	void (*errcb)(void *, Sint32);		/* error callback */
	void *errcb_1st;		/* error callback 1st arg. */
	Sint32 gdchn;			/* gdc handle */
	volatile Sint32 gdchn_wait;		/* gdc wait type */
	Sint32 ex_errcode;		/* extra error code */
	Sint16 act;		/* handle act */
	Sint16 trflag;		/* transfer flag */
	Sint16 used;		/* used flag */
	Sint16 tmode;		/* transfer mode */
	Sint16 stat;		/* handle status */
	Sint16 err;		/* error status */
} GDFS_HANDLE; /* 84 bytes */
typedef GDFS_HANDLE *GDFS;
//END OF BITS NEEDED FROM SG_GD

//BITS NEEDED FROM MACHINE.H
#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned long size_t;
#endif
//END OF BITS NEEDED FROM MACHINE.H

//#include <kamui.h>
//bits needed from kamui

//#include "kmbase.h"

//BITS FROM KM BASE
#define IN
#define OUT

typedef int 		 	KMINT32;				/* i	*/
typedef unsigned int 	KMUINT32;				/* u	*/
typedef unsigned long	KMDWORD;				/* dw	*/
typedef unsigned char	KMBYTE;					/* b	*/
typedef float			KMFLOAT;				/* f	*/

#ifndef DWORD
	typedef unsigned long	DWORD;					/* dw	*/
#endif

#ifndef VOID
	typedef void			VOID;
#endif

typedef DWORD			*PKMDWORD;				/* pdw	*/
typedef VOID 			KMVOID;					/* 		*/
typedef KMVOID 			*PKMVOID;				/* p	*/
//END OF BITS FROM KM BASE

#include "kmenum.h"

//#include "kmstruct.h"
/** BITS FROM KMSTRUCT
/*
 * FrameBuffer/TextureSurface
 */

typedef struct tagKMSURFACEDESC
{
	KMDWORD	SurfaceType;				/* 0:FrameBuffer    1:Texture	*/
	KMDWORD	BitDepth;					/* 16bpp...16   32bpp...32  etc	*/
 	KMDWORD	PixelFormat;				/* 1555,4444,etc				*/

	union{
		KMDWORD	USize;					/* USize 8-1024					*/
		KMDWORD	Width;					/* FB/Tex(Width)				*/
	}
#if defined(_STRICT_UNION_)
	u0
#endif
	;

	union{
		KMDWORD	VSize;					/* VSize 8-1024					*/
		KMDWORD	Height;					/* FB/Tex(Height)				*/
	}
#if defined(_STRICT_UNION_)
	u1
#endif
	;

	union{
		KMDWORD	dwTextureSize;			/* TextureSize					*/
		KMDWORD	dwFrameBufferSize;		/* FrameBufferSize				*/
	}
#if defined(_STRICT_UNION_)
	uSize
#endif
	;

	KMDWORD		fSurfaceFlags;			/* Surface Flags				*/
	PKMDWORD	pSurface;				/* Texture instance(Pointer on PVRII Core)	*/
	PKMDWORD	pVirtual;				/* Texture instance(Virtual  address on SH4)*/
	PKMDWORD	pPhysical;				/* Texture instance(physical address on SH4)*/

}KMSURFACEDESC;

//** END OF BITS FROM KMSTRUCT


//end of kamui include bits

#define	_KM_USE_VERTEX_MACRO_L4_

// Fix the bug in kamuix.h
#undef kmxxSetVertex_7
#define kmxxSetVertex_7(pcw, x, y, invw, fu, fv, bi, oi)						\
		*pkmCurrentPtr++ = pcw;													\
		*(PKMFLOAT)pkmCurrentPtr++ = x;													\
		*(PKMFLOAT)pkmCurrentPtr++ = y;													\
		*(PKMFLOAT)pkmCurrentPtr++ = invw;												\
		*(PKMFLOAT)pkmCurrentPtr++ = fu;													\
		*(PKMFLOAT)pkmCurrentPtr++ = fv;													\
		*(PKMFLOAT)pkmCurrentPtr++ = bi;												\
		*(PKMFLOAT)pkmCurrentPtr   = oi;												\
		prefetch((void *)pkmCurrentPtr);										\
		pkmCurrentPtr++;

/*
 * This include is specific to the person building the executable
 * You must have your own local copy of this file - it is *not*
 * under source control
 */
#include "LocalDefs.h"

/* Defaults: */
#ifndef ANTIALIAS
#define ANTIALIAS 0
#endif
#ifndef COLLISION
#define COLLISION 1
#endif
#ifdef FRAMES_PER_SECOND
#undef FRAMES_PER_SECOND
#endif
#define FRAMES_PER_SECOND (PAL?25:30)
#ifndef PROFILE_BARS
#define PROFILE_BARS 0
#endif
#ifndef SHOW_FPS
#define SHOW_FPS 0
#endif
#ifndef NEWCOLL
#define NEWCOLL 1
#endif
#ifndef SHOWSPEED
#define SHOWSPEED 0
#endif
#ifndef SHOWPOLYTYPE
#define SHOWPOLYTYPE 0
#endif
#ifndef DRAWNET
#define DRAWNET 0
#endif
#ifndef DRAWSPLINE
#define DRAWSPLINE 0
#endif
#ifndef DLOG
#define DLOG 0
#endif
#ifndef COUNT_GEOMETRY
#define COUNT_GEOMETRY 0
#endif
#ifndef DRAW_DEBUG
#define DRAW_DEBUG 0
#endif
#ifndef BENCHMARK
#define BENCHMARK 0
#endif
#ifndef NEW_TITLE_PAGE
#define NEW_TITLE_PAGE 0
#endif
#ifndef MUSIC
#define MUSIC 0
#endif

/*bits needed from macros*/
#ifndef PI
#define PI 3.141592654f
#endif
#define PI2 (PI*2.0f)
#define HPI	(PI/2.0f)


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
//END OF RDTYPES

//REMOVED COS STRATS DON'T NEED ACCESS TO IT
//#include "Memory.h"
/* Types used to pass allocators to functions */
//BITS NEEDED FROM MEMORY.H
typedef void *Allocator (void *, size_t);
typedef void *Deallocator (void *, void *);


/************  START OF STREAM.H INCLUDE *************/
/*
 * Stream.h
 * Access to CD subsystem
 */

#ifndef _STREAM_H
#define _STREAM_H

/*
 * The maximum number of open streams
 */
#define MAX_STREAMS 1

/*
 * The maximum number of files in a directory
 */
#define MAX_STR_DIRENT	64

/*
 * The number of sectors' worth of pre-reading that is done
 * 16 sectors is a pre-read of 32k
 */
#define STREAM_PREREAD 16


/*
 * A stream encapsulates a file
 */
typedef struct tagStream
{
	GDFS		gdfs;					/* Handle to OS file */
	Bool		fin;					/* Activity complete flag */
	Bool		active;					/* Is this descriptor active? */
	Sint32		bytesLeft;				/* Bytes left for this file */
	Uint32		secAmt;					/* Amount of valid data in the secBuf */
	Uint32		secPtr;					/* Offset into the secBuf */
	Uint32		pad[2];					/* Padding ensures secBuf is aligned */
	char		*preReadBuf;			/* Preread buffer */
	Uint32		preReadSize;			/* Size of preread buffer */
	char		secBuf[GDD_FS_SCTSIZE * STREAM_PREREAD];
										/* Data for partial-sector reads */
} Stream;
#endif
//END OF STREAM

#include "Render\Animate.h"
//#include "Render\Render.h"

/*
 * An entry in the vert/colour list
 */

//from map.h
typedef struct tagStripPos
{
	Point3		p;
	Uint32		colour;
} StripPos;



//#include "Render\Material.h"

/*** BITS FROM MATERIAL.H ******/

	/* Lowest level processed material */
	typedef struct tagMaterialInfo
	{
		Uint32					GLOBALPARAMBUFFER;		/*  Grobal (sic) Parameter Buffer	*/
		Uint32					ISPPARAMBUFFER;			/*  ISP Parameter Buffer			*/
		Uint32					TSPPARAMBUFFER;			/*	TSP Parameter Buffer			*/
		Uint32					TexturePARAMBUFFER;		/*	Texture Parameter Buffer		*/
		struct tagRasteriser	*renderer;				/*	The renderer for this material	*/
	} MaterialInfo;

	/* A material */
	typedef struct {
		Colour			diffuse;			/* The diffuse colour */
		Colour			specular;			/* The specular colour */
		float			exponent;			/* The exponent of the specular colour - ie higher number (eg 60) tighter spotlight */
		Uint32			hideID;				/* Bitmask of hidden stuff for landscape hiding */
		Uint32			flags;				/* flags */
		union {
			KMSURFACEDESC	*surfaceDesc;	/* Pointer to the texture surface of the texture for this material */
			Uint32			GBIX;			/* GBIX of the texture */
		} surf;
		MaterialInfo		info;			/* The lowest level material information */
		struct {
			Uint32				flags;			/* The flags for this material */
			/*
			 * Note that only some flags have an effect on a pasted texture :
			 * At this stage the UV values, and lighting values have already been worked out
			 * and so MF_ENVIRONMENT, MG_TWO_SIDED, MF_NO_LIGHTING and MF_TEXTURE_PASTE have no effect
			 */
			float				transparency;	/* How transparent this texture is (opacity values are *'d by this) */
			union {
				KMSURFACEDESC	*surfaceDesc;	/* Pointer to the texture surface of the texture for this material */
				Uint32			GBIX;			/* GBIX of the texture */
			} surf;
			MaterialInfo		info;			/* The lowest level material information */
		} pasted;								/* The pasted texture identification */
	} Material;

/*** END OF BITS FROM MATERIAL.H ***/

//#include "Render\Model.h"
/*** BITS FROM MODEL.H *****/
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
	 * Bump mapping information
	*/
	typedef struct tagBumpMapInfo
	{
		Point3		biNormal;
		Point3		tangent;
		float		angShift;
	} BumpMapInfo;

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

/*** END OF BITS FROM MODEL.H *****/


#include "Render\Object.h"
//#include "Render\lighting.h"
/*** BITS FROM LIGHTING.H ****/

	typedef struct tagLightingValue
	{
		float a, r, g, b;
	} LightingValue;

/*** END OF BITS FROM LIGHTING.H ****/

// Floating point random number in [0,1]
float frand(void);
//#define RandRange(a,b) ((frand() *((b)-(a))) + (a))


#include "Strat.h"


#include "sndutls.h"
//PUT BACK TO SNDUTLS WHEN DES IS DONE, AND REMOVE INCLUDES FROM THAT FILE
//#include "stratsound.h"

//spark includes for strats
#ifndef _SPARKS_H
#define _SPARKS_H


// Strat glue code
void MakeSpark (STRAT *strat, float x, float y, float z);

// Shield sparks
void MakeShieldSpark (void);
void MakeShieldSparkTable (Model *shieldModel);

#endif

extern Bool		PAL;

//band includes for strats
#ifndef _BANDS_H
#define _BANDS_H

// A handle to a band
typedef int BandHandle;
// Strat glue code
int CreateBand (STRAT *strat, int shapeType, int maxSegs, int mType,
				float a0, float r0, float g0, float b0,
				float a1, float r1, float g1, float b1);
void DeleteBand (STRAT *strat, BandHandle bandH);
void AddBandSegment (STRAT *strat, BandHandle bandH, float x, float y, float z, float scale);
void RubberBand (STRAT *strat, BandHandle bandH, float x, float y, float z, float scale);
int AnyFreeJonnies (int nBands, int nSegmentsPerBand);
void SetBandColour (BandHandle bandH,
					float a0, float r0, float g0, float b0,
					float a1, float r1, float g1, float b1);
void SetBandRotXYZandScaling (BandHandle bandH, float rX, float rY, float rZ,
							  float sX, float sY, float sZ);
void UpdateTrail (STRAT *strat, int trailNo, float x, float y, float z);
void DeleteTrail (STRAT *strat, int trailNo);
int CreateTrail (STRAT *strat, int type, int nSegs,
				 float a0, float r0, float g0, float b0,
				 float a1, float r1, float g1, float b1);

#endif



#endif
