#ifndef _LIGHTING_H
#define _LIGHTING_H

/*
 * *gulp* here we go...
 * Firstly model lighting:
 * All of the available light sources are traversed per top-level model
 * All those that apply are added into the general global picture
 * A weight is calculated per light based on its unique properties
 */

typedef struct tagLightingValue
{
	float a, r, g, b;
} LightingValue;

typedef struct tagLightingBuffer
{
	Point3		position;					// Current position of the light
	struct {
		float		r, g, b;				// The colour of the light
	} colour;
} LightingBuffer;

typedef float ModelLighter(LightingBuffer *buffer, void *lightData, Point3 *centroid); // Model lighting routine

typedef struct tagLight
{
	ModelLighter		*modelLighter;		// The routine which applies its information to the current model colour buffer
	void				*lightData;			// Data passed to the model lighter
} Light;

typedef struct tagScapeLight
{
	struct tagScapeLight *nextActive;		// The next active light
	struct tagScapeLight *nextMdl;			// The next active light for this model
	struct strat		*parent;			// Parent strat, if any
	Point3				position;			// Position of the light
	float				radius;				// Effective radius of the light
	float				rSqrRadius;			// 1/(radius^2)
	float				r, g, b;			// Not the colour, surely!
	float				radiusMul;			// Multiply by this every frame
	float				minRadius;			// Kill if less than this radius
} ScapeLight;

/* Global variables */
extern Point sunDir, StratSunDir;
extern Point3 lightPos;
extern LightingValue lAmbient;
extern float CurrentStratIntensity;
typedef struct {
	float r, g, b;
} _lColour;
extern _lColour lColour, lOffset;

#define MAX_LIGHTS			64
#define MAX_SCAPE_LIGHTS	64
#define MAX_LIT_VERTICES	768

void lUpdateScapeLights (void);
void lLightVertices (const ScapeModel *m);
void lNoLighting (void);

#endif
