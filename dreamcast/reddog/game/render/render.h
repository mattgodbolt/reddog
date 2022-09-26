/*
 * Render.h
 * The rendering subsystem
 */
#ifndef _RENDER_H
#define _RENDER_H

#include "Render\Material.h"
#include "Render\Matrix.h"
#include "Render\Model.h"
#include "Render\Object.h"
#include "Render\Map.h"
#include "Render\ScapeModel.h"
#include "Render\Quat.h"

/*
 * Definitions :
 */
#if ANTIALIAS
#define PHYS_SCR_X					1280
#define X_SCALE						2
#else
#define PHYS_SCR_X					640
#define X_SCALE						1
#endif
#define PHYS_SCR_Y					480

extern Point2 pViewMidPoint;
#define X_MID						(pViewMidPoint.x)
#define Y_MID						(pViewMidPoint.y)
#define OPAQUE_POLYGON_SIZE			700000
#define OPAQUE_MODIFIER_SIZE		0
#define TRANSP_POLYGON_SIZE			300000
#define TRANSP_MODIFIER_SIZE		0

#define NEAR_W_CLIP_VALUE			(1.0f / 100.f)
#define W_IN_FRONT					(1.0f / 0.1f)
#define W_BEHIND					(-1.0f)

/*
 * Initialise the renderer - called once from main()
 * Now called when swapping between single/multi player
 * Now called also all the time to allocate textures!
 */
void rInitRenderer (Bool Multiplayer, Uint32 texAmt);

/*
 * Shuts down the renderer
 */
void rShutdownRenderer();

/*
 * Set the perspectivization matrix
 */
void rSetPerspective (float FOV, float aspectRatio, float nearPlane, float farPlane);

/*
 * Set the background colour
 */
void rSetBackgroundColour (Uint32 colour);

/*
 * Load a texture palette
 */
void texLoad (Stream *s);
Uint32 texLoadSingleTexture (Stream *s);

/*
 * Set up the fogging table
 */
void rSetFog (Uint32 colour, float fogNearZ, float fogFarZ, float fogAmt);

/*
 * Find a texture in the palette
 */
KMSURFACEDESC *texFind (Uint32 GBIX);

/*
 * Fix up a material after loading all textures
 * The base context is used for COLOURTYPEs, UVTYPES etc
 */
void rFixUpMaterial (Material *mat, KMVERTEXCONTEXT *baseContext);

/* Draw a string */
void tPrintf (float x, float y, char *fmt, ...);
void tScore (float x, float y, char *score, Uint32 colour);
void tScoreTime (float x, float y, char *score, Uint32 colour);

// Prints a string for the script processor
// Numchars is a float to allow smooth printing of all the
// fade-in effects
void ScriptPrint (float x, float y, char *fmt, float numchars);

// Text callback
void tCallback (void);

/*
 * Fades the screen to colour Col
 */
void rFade (Colour colour, float amt);

/*
 * End of frame function
 */
void rEndFrame (void);

/* Find an animation */
int rFindAnim (const PNode *node, const char *name);

/*
 * Draw a line in 3d
 */
void rLine (Point3 *start, Point3 *end, Colour stCol, Colour stEnd);

/*
 * Draw a line 'on top'
 */
void rLineOnTop (Point3 *start, Point3 *end, Colour stCol, Colour stEnd);

// 2d line
void rLine2D (float x0, float y0, float x1, float y1, float pri, Colour sCol, Colour eCol);


/*
 * Starts an animation off
 */
void PlayAnimation (const PNode *node, StratAnim *anim, int offset);

/*
 * Draw a sprite
 */
void rSprite (Point3 *pos, float xScale, float yScale, float rotationZ, Object *obj);
void rSpriteMtl (Point3 *pos, float xScale, float yScale, float rotationZ, Material *mat);
void rSpriteNoZScale (Point3 *p, float xSize, float ySize, Object *obj);
void rSprite2D (float x, float y, Object *obj);
void rSprite2DMtl (float x, float y, Material *mat, float scale, float rZ);

/*
 * Loads the modelToScreen matrix into XMTRX
 */
void mLoadModelToScreen (void);
void mLoadModelToScreenScaled (void);

/*
 * Profile bar warez
 */
void pbReset (void);
void pbDraw (void);
typedef enum {
	PB_CAM_AND_LIGHT,
	PB_STRAT,
	PB_COLLISION,
	PB_SFX,
	PB_SCAPE_DRAW,
	PB_STRAT_DRAW,
	PB_SHADOW,
	PB_POSTAMBLE,
	PB_SOUND,
	PB_MAX
} ProfileType;
void pbMark (Uint32 colour, Bool isRendering, ProfileType type);

/*
 * The fragments
 */
typedef enum {
	FT_FALL = 0,				/* A fragment which just falls under gravity */
	FT_NOGRAV = 1,				/* A fragment which just flows along its path */
	FT_LAVA = 2,				/* A lava fragment */
	FT_WATER = 3,				/* A water fragment */
	FT_MAX = 4
} FragType;

typedef struct tagFragment
{
	Uint32			lifeTimer;	/* 0 if dead, decremented otherwise */
	Point3			pos;		/* Position of fragment */
	Point3			posV;		/* Position vector */
	Point3			rot;		/* Rotation of fragment */
	Point3			rotV;		/* Rotation vector */
	Colour			col;		/* Colour of fragment */
	FragType		type;		/* The type of the fragment */
} Fragment;

#define MAX_FRAGMENTS 512

/*
 * rResetFragments - reset the fragment buffer
 */
void rResetFragments (void);

/*
 * Create fragments (give centre, radius and amount)
 */
void rCreateFragments (const Point3 *centre, float radius, Uint32 amount);
void rCreateSplash (const Point3 *centre, float radius, Uint32 amount);
void rCreateExpFragments (const Point3 *centre, float radius, Uint32 amount);
void rCreateExpFragmentsBright (const Point3 *centre, float radius, Uint32 amount);
void rCreateTrailFragments (const Point3 *centre, float radius, Uint32 amount);
void rCreateLavaBits (const Point3 *centre, Uint32 amount);
void rCreateConeFrags (const Point3 *centre, const Vector3 *dir, float cosAngle, float velMax, int amount);

/*
 * Updates the fragments and draws them
 */
void rUpdateFragments (void);
void rDrawFragments (void);

/*
 * Sea level parameters
 */
extern float rSeaLevel;
extern float rWaveHeight;
extern float rSeaChoppiness;
extern Bool  rSeaOn;

/*
 * Model Flag access
 */
extern Uint32 rGlobalModelFlagMask;
extern Uint32 rGlobalModelFlags;
extern float  rGlobalTransparency;
extern float  rGlobalPastedOverride;
/*
 * WaterCollision
 * Returns 0 if above water, else number of metres under water
 */
float rWaterCollision (Point3 *p);

void rSeaEffectOn(Material *m);
void rSeaEffectOff(void);

/*
 * Skidding warez
 */

// Start a skid - call once a frame directly before calls to DrawSkid
void rStartSkid (int pn);
// Draws a skid strip of numPoints points
void rDrawSkid (int numPoints, StripEntry *stripEntBuffer, StripPos *stripPosBuffer);

// Overlays
void rInitOverlays(void);
void rOverlay (int charNum, float amount);

/*
 * Set the text colour and alpha amount
 */
void psSetColour (Uint32 colour);
void psSetAlpha (float amount);

/*
 * Set the text window
 * -1 as a paramater means camera render extent (includes safe frame)
 * Text is not clipped to this region, it merely set the extent for center/left/right justify
 * and the origin is set to the top left
 */
void psSetWindow (float top, float left, float right, float bottom);
// Get the window
void psGetWindow (float window[4]);

/*
 * Set/get the text matrix
 */
void psSetMatrix (Matrix32 *m);
Matrix32 *psGetMatrix(void);

/*
 * Sets the text priority
 */
void psSetPriority (float priority);

/*
 * Draw a string, no printf() action
 * x and y in window space
 */
void psDrawString (const char *string, float x, float y);
float psDrawStringFE (const char *string, float x, float y, float nChars);

/*
 * Draw a string, centred within the current window at a given y position
 */
void psDrawCentred (const char *string, float y);
float psDrawCentredFE (const char *string, float y, float nChars);

/*
 * Draw a string, right aligned within the current window at a given y position
 */
void psDrawRJustified (const char *string, float y);

/*
 * Return a statically sprintf'ed buffer
 */
char *psFormat (const char *fmt, ...);
char *psFormatNum (int num);
char *psFormatTime(Uint32 min, Uint32 sec);
char *psFormatTimeMilli(Uint32 min, Uint32 sec, Uint32 ms);
extern int psLastLength;

/*
 * Return the length of a string in pixels
 */
float psStrWidth (const char *string);

/*
 * Get the number of characters in the alphabent
 */
int psNumChars(void);

/*
 * Get the character for a given number
 */
char psGetChar(int);

/*
 * Draw a title, centred within the current window at a given y position
 * Has a nice background too blending from col1 to col2 and back to col1
 */
void psDrawTitle (const char *string, float y, Uint32 col1, Uint32 col2);

/*
 * Centrally wordwraps some text
 */
float psCentreWordWrap (const char *string, float y, float width);

/*
 * FrontEnd staple
 */
float psCentreWordWrapFE (const char *string, float x0, float y0, float x1, float y1, float scale, float nChars);
float psDrawWordWrap (const char *string, float x, float y, float width);

/*
 * Hack for centring madness
 */
void psSetMultiplier (float scale);
float psGetMultiplier (void);

// Lightning
void rDrawLightning (Point3 *start, Point3 *end, Colour startCol, Colour endCol,
					 Colour oStartCol, Colour oEndCol,
					 int depth, float noise,
					 float startScale, float endScale, int madness);

#endif
