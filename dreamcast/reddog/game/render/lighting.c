/*
 * Lighting.c
 * C lighting code
 */

// must remember to apply this to the fast model draw routines too

#include "..\Reddog.h"
#include "..\View.h"
#include "Internal.h"

#define SCAPELIGHT_TOLERANCE 2.5f

LightingValue lAmbient = { 0.f, 0.4f, 0.4f, 0.4f };
_lColour lColour;
_lColour lOffset;

Point StratSunDir;

// The lights - hooray!
Light gLight[MAX_LIGHTS];
ScapeLight gScapeLight[MAX_SCAPE_LIGHTS];
ScapeLight *lFirstScapeLight;
BBox lLitArea;

static bool LightOffscreen(const Point3 *midPoint, float radius);
Point3 *LastSpawnedScapeLightPos = NULL;

// Low-level light creation routine
static Light *lCreate (ModelLighter *routine, int nBytes)
{
	Light *retVal;
	void *vo;
	for (retVal = gLight; retVal < (gLight + MAX_LIGHTS); ++retVal) {
		if (retVal->modelLighter == NULL)
			break;
	}
	if (retVal == (gLight + MAX_LIGHTS))
		return NULL;

	vo = ObjAlloc (nBytes);
	if (vo == NULL)
		return NULL;

	retVal->lightData = vo;
	retVal->modelLighter = routine;

	return retVal;
}

static void lDelete (Light *l)
{
	if (l == NULL)
		return;

	ObjFree (l->lightData);
	l->modelLighter = NULL;
}

// Calculates the lighting madness for an object's centre position
// updates the global variables sunDir and lColour
void lCalculate (Point3 *centroid) 
{
	Light *l;
	LightingBuffer buffer;
	float totalWeight;

	// Start with the sun direction
	buffer.position = *(Point3*)&StratSunDir;
	totalWeight = (1+CurrentStratIntensity) * 2.f;
	buffer.colour.r = totalWeight;
	buffer.colour.g = totalWeight;
	buffer.colour.b = totalWeight; // xx optimise

	// Let each light do its worst
	for (l = gLight; l < (gLight + MAX_LIGHTS); ++l) {
		if (l->modelLighter)
			totalWeight += l->modelLighter (&buffer, l->lightData, centroid);
	}
	// Divide out the madness and clamp
	totalWeight = 1.f / totalWeight;

	lColour.r = buffer.colour.r		* totalWeight;
	lColour.g = buffer.colour.g		* totalWeight;
	lColour.b = buffer.colour.b		* totalWeight;
	sunDir.x  = (buffer.position.x	* totalWeight);
	sunDir.y  = (buffer.position.y	* totalWeight);
	sunDir.z  = (buffer.position.z	* totalWeight);

	lOffset.r = lColour.r - 1;
	lOffset.r = MAX(0,lOffset.r);
	lOffset.g = lColour.g - 1;
	lOffset.g = MAX(0,lOffset.g);
	lOffset.b = lColour.b - 1;
	lOffset.b = MAX(0,lOffset.b);

	lColour.r = RANGE(0,lColour.r, 1);
	lColour.g = RANGE(0,lColour.g, 1);
	lColour.b = RANGE(0,lColour.b, 1);
	v3Normalise ((Point3 *)&sunDir);
}

void lNoLighting (void)
{
	lOffset.r = lOffset.g = lOffset.b = 0.f;
	lColour.r = lColour.g = lColour.b = 1.f;
	sunDir.x = 0.4f;
	sunDir.y = 0.4f;
	sunDir.z = 0.2f;
	v3Normalise ((Point3 *)&sunDir);
}

typedef struct {
	Point3 position;
	float r, g, b;
	float multiplier;
	float rDist;
} PointLightData;

float PointLight(LightingBuffer *buffer, void *lightData, Point3 *centroid)
{
	PointLightData *l = (PointLightData *)lightData;
	Point3 pos;
	float dist;
	v3Sub (&pos, &l->position, centroid);
	if ((pos.x != 0.0) || (pos.y != 0.0) || (pos.z != 0.0))
	{
		dist = 1.f / v3Normalise (&pos);
		dist = dist * l->rDist;
		if (dist > 1)
			return 0;
		dist = (1 - dist)*l->multiplier;
		buffer->colour.r += l->r * dist;
		buffer->colour.g += l->g * dist;
		buffer->colour.b += l->b * dist;
		buffer->position.x += dist * pos.x;
		buffer->position.y += dist * pos.y;
		buffer->position.z += dist * pos.z;
		return dist;
	}
	else
		return 0.0f;
}

/* Initialises the lighting subsystem */
void lInit (void)
{
	Light *l;
	int i;

	for (l = gLight; l < (gLight + MAX_LIGHTS); ++l)
		l->modelLighter = NULL;

	for (i = 0; i < MAX_SCAPE_LIGHTS; ++i)
		gScapeLight[i].radius = 0;

	StratSunDir.x = 1;
	StratSunDir.y = -1;
	StratSunDir.z = 1;
	StratSunDir.w = 0;
	v3Normalise ((Point3 *)&StratSunDir);
}

void ExplosionLight (STRAT *strat, float r, float g, float b, float radius, float mul, float min)
{
	ScapeLight *l;
	bool found = FALSE;

	if (LastSpawnedScapeLightPos && pSqrDist (&strat->pos, LastSpawnedScapeLightPos) < SQR(SCAPELIGHT_TOLERANCE))
		return;

	LastSpawnedScapeLightPos = &strat->pos;

	for (l = gScapeLight; l < (gScapeLight + MAX_SCAPE_LIGHTS); ++l)
		if (l->radius == 0) {
			found = TRUE;
			break;
		}
	if (!found)
		return;
	l->parent = strat;
	l->position = strat->pos;
	l->minRadius = min;
	l->r		= r;
	l->g		= g;
	l->b		= b;
	l->radius	= radius;
	l->radiusMul= mul;
}

// Strat glue routines
int MakePointLight (STRAT *strat, float r, float g, float b, float dist, float multiplier, float x, float y, float z)
{
	Matrix matrix;
	Point3 lightPos;
	Light *l = lCreate (PointLight, sizeof (PointLightData));
	PointLightData *d;

	if (l == NULL)
		return -1; // No light

	lightPos.x = x;
	lightPos.y = y;
	lightPos.z = z;

	d = (PointLightData *)l->lightData;

	matrix = strat->m;
	mPreScale(&matrix, strat->scale.x, strat->scale.y, strat->scale.z);
	mPostTranslate(&matrix, strat->pos.x, strat->pos.y, strat->pos.z);
	mPoint3Multiply3(&d->position, &lightPos, &matrix);

	dAssert (d->position.x == d->position.x, "QNAN!!!");


	d->multiplier = multiplier;
	d->r = r;
	d->g = g;
	d->b = b;
	d->rDist = 1.f / dist;

	return l - gLight;
}

void RemoveLight (int id)
{
	if (id == -1)
		return;
   //	if (!gLight[id].modelLighter)
   //		return;
	
	dAssert (id >= 0 && id < MAX_LIGHTS && gLight[id].modelLighter, "Invalid light ID");
	lDelete (gLight + id);
}


void SetPointLightRadius (int id, float r, float m)
{
	PointLightData *d;
	if (id == -1)
		return;
	dAssert (id >= 0 && id < MAX_LIGHTS && gLight[id].modelLighter, "Invalid light ID");

	d = (PointLightData *)gLight[id].lightData;
	if (r == 0.0f)
		d->rDist = 0.1f;
	else
		d->rDist = 1.f / r;
	d->multiplier = m;
}

void UpdateLight (STRAT *strat, int id, float x, float y, float z)
{
	Matrix matrix;
	Point3 lightPos;
	Light *l;
	PointLightData *d;

	if (id == -1)
		return; // No light

	lightPos.x = x;
	lightPos.y = y;
	lightPos.z = z;

	d = (PointLightData *)gLight[id].lightData;

	matrix = strat->m;
	mPreScale(&matrix, strat->scale.x, strat->scale.y, strat->scale.z);
	mPostTranslate(&matrix, strat->pos.x, strat->pos.y, strat->pos.z);
	mPoint3Multiply3(&d->position, &lightPos, &matrix);
}

/*
 * Scape lighting madness
 * Find the lights that have any effect on the scape
 * Update the lights
 * Update the lighting box
 */
void lUpdateScapeLights (void)
{
	int i;
	ScapeLight *last, *light;
	Point p;
	LastSpawnedScapeLightPos = NULL;

	lLitArea.low.x = lLitArea.low.y = lLitArea.low.z =
		lLitArea.hi.x = lLitArea.hi.y = lLitArea.hi.z =	0.f;

	lFirstScapeLight = last = NULL;

	light = gScapeLight;
	for (i = 0; i < MAX_SCAPE_LIGHTS; ++i, ++light) {
		if (light->radius <= 0)
			continue;
		// Update the light
		if (light->parent) {
			light->position = light->parent->pos;
			if (light->parent->flag == 0) {
				if (light->radiusMul >= 0.98f) {
					light->radius = 0.f;
					continue;
				} else {
					light->parent = NULL; // Parent has died
				}
			}
		}
		light->radius *= light->radiusMul;
		light->rSqrRadius = 1.f / SQR(light->radius);
		// Kill off if too small, or if the parent has died
		if (light->radius < light->minRadius) {
			light->radius = 0;
			continue;
		}
		// Is this light applicable to the scape?
		if (!LightOffscreen(&light->position, light->radius)) {
#if COUNT_GEOMETRY
			nLights++;
#endif
			if (lFirstScapeLight == NULL) {
				register float *read, *write;
				lFirstScapeLight = last = light;
				write = &lLitArea.low.x;
				read = &light->position.x;
				*write++ = *read++ - light->radius;
				*write++ = *read++ - light->radius;
				*write++ = *read++ - light->radius;
				read = &light->position.x;
				*write++ = *read++ + light->radius;
				*write++ = *read++ + light->radius;
				*write++ = *read++ + light->radius;
			} else {
				register float *read;
				last->nextActive = light;
				last = light;
				read = &lLitArea.low.x;
				*read = MIN (*read, light->position.x - light->radius);
				read++;
				*read = MIN (*read, light->position.y - light->radius);
				read++;
				*read = MIN (*read, light->position.z - light->radius);
				read++;
				*read = MAX (*read, light->position.x + light->radius);
				read++;
				*read = MAX (*read, light->position.y + light->radius);
				read++;
				*read = MAX (*read, light->position.z + light->radius);
			}
			light->nextActive = NULL;
		} 
	}
}

#pragma aligndata32(lLightBuffer,lLightBufferZerod)
Uint32 lLightBuffer[MAX_LIT_VERTICES];
Uint32 lLightBufferZerod[MAX_LIT_VERTICES];

void lLightVertices (const ScapeModel *m)
{
	register ScapeLight *s, *first, *sNext;
	Uint32 *output = lLightBuffer;
	register float r, g, b, dist, *read, x, y, z;
//	register float r2, g2, b2, dist, dist2, *read;
	register int i;
	int nLightsActive;
	register StripPos *v = m->vertex;

	while(m->nVerts > MAX_LIT_VERTICES);
	dAssert (m->nVerts < MAX_LIT_VERTICES, "Ack");

	// Does it need lighting at all?
	if (m->bounds.low.x > lLitArea.hi.x ||
		m->bounds.low.y > lLitArea.hi.y ||
		m->bounds.low.z > lLitArea.hi.z ||
		m->bounds.hi.x < lLitArea.low.x ||
		m->bounds.hi.y < lLitArea.low.y ||
		m->bounds.hi.z < lLitArea.low.z) {
		theStripRasterContext.lightingBuffer = lLightBufferZerod;
		return;
	}

	// Find the applicable lights
	nLightsActive = 0;
	first = NULL;
	for (s = lFirstScapeLight; s; s = s->nextActive) {
		dist = s->radius;
		// Check X plane
		if ((s->position.x - dist) > m->bounds.hi.x ||
			(s->position.x + dist) < m->bounds.low.x)
			continue;
		// Check Y plane
		if ((s->position.y - dist) > m->bounds.hi.y ||
			(s->position.y + dist) < m->bounds.low.y)
			continue;
		// Check Z plane
		if ((s->position.z - dist) > m->bounds.hi.z ||
			(s->position.z + dist) < m->bounds.low.z)
			continue;
		s->nextMdl = first;
		first = s;
		nLightsActive++;
	}

	// Are any lights applicable?
	if (nLightsActive == 0) {
		theStripRasterContext.lightingBuffer = lLightBufferZerod;
		return;
	}
	theStripRasterContext.lightingBuffer = lLightBuffer;


#if COUNT_GEOMETRY
	nLit += m->nVerts * nLightsActive;
#endif

#if 1
	if (nLightsActive == 1) {
		register float distMul;
		s = first;
		distMul = s->rSqrRadius;
		read = &s->position.x;
		x = *read++;
		y = *read++;
		z = *read;
		for (i=0; i < m->nVerts; ++i) {
			read = &v->p.x;
			dist = SQR(*read - x); read++;
			dist +=SQR(*read - y); read++;
			dist +=SQR(*read - z);
			dist *= distMul;
			if (dist > 1) {// Outside sphere of influence
				*output++ = 0;
			} else {
				dist = 1 - dist;
				read = &s->r;
				r = *read++ * dist;
				g = *read++ * dist;
				b = *read * dist;
				if (r>1) r=1;
				if (g>1) g=1;
				if (b>1) b=1;
				*output++ = (((int)(r*255.f))<<16) | (((int)(g*255.f))<<8) | (((int)(b*255.f))<<0);
			}
			v++;
		}
	} else {
		for (i=0; i < m->nVerts; ++i) {
			read = (float *)v;
			r = g = b = 0;
			s = first;
			x = *read++;
			y = *read++;
			z = *read;
			do {
				read = &s->position.x;
				dist = SQR(*read - x); read++;
				dist +=SQR(*read - y); read++;
				dist +=SQR(*read - z);
				dist *= s->rSqrRadius;
				if (dist > 1) // Outside sphere of influence
					continue;
				dist = 1 - dist;
				read = &s->r;
				r = r + *read++ * dist;
				g = g + *read++ * dist;
				b = b + *read * dist;
			} while ((s = s->nextMdl) != NULL);
			if (r>1) r=1;
			if (g>1) g=1;
			if (b>1) b=1;
			*output++ = (((int)(r*255.f))<<16) | (((int)(g*255.f))<<8) | (((int)(b*255.f))<<0);
			v++;
		}
	}
#else
	i = m->nVerts;
	if (i & 1) {
		r = g = b = 0;
		for (s = first; s; s = s->nextMdl) {
			dist = pSqrDist ((Point3 *)v, &s->position) * s->rSqrRadius;
			if (dist > 1) // Outside sphere of influence
				continue;
			dist = 1 - dist;
			read = &s->r;
			r = r + *read++ * dist;
			g = g + *read++ * dist;
			b = b + *read * dist;
		}
		if (r>1) r=1;
		if (g>1) g=1;
		if (b>1) b=1;
		*output++ = (((int)(r*255.f))<<16) | (((int)(g*255.f))<<8) | (((int)(b*255.f))<<0);
		v++;
		i--;
	}
	for (; i > 0; i-=2) {
		r = g = b = r2 = g2 = b2 = 0;
		for (s = first; s; s = s->nextMdl) {
			dist  = pSqrDist ((Point3 *)v, &s->position) * s->rSqrRadius;
			dist2 = pSqrDist ((Point3 *)(v+1), &s->position) * s->rSqrRadius;
			if (dist > 1) // Outside sphere of influence
				dist = 1;
			dist = 1 - dist;
			if (dist2 > 1) // Outside sphere of influence
				dist2 = 1;
			dist2 = 1 - dist2;
			read = &s->r;
			r = r + *read * dist;
			r2 = r2 + *read * dist2;
			read++;
			g = g + *read * dist;
			g2 = g2 + *read * dist2;
			read++;
			b = b + *read * dist;
			b2 = b2 + *read * dist2;
			read++;
		}
		if (r>1) r=1;
		if (g>1) g=1;
		if (b>1) b=1;
		if (r2>1) r2=1;
		if (g2>1) g2=1;
		if (b2>1) b2=1;
		*output++ = (((int)(r*255.f))<<16) | (((int)(g*255.f))<<8) | (((int)(b*255.f))<<0);
		*output++ = (((int)(r2*255.f))<<16) | (((int)(g2*255.f))<<8) | (((int)(b2*255.f))<<0);
		v+=2;
	}
#endif
}

/*
 * Generates a plane for the purpose of onscreen checking
 */
#define rPreparePlaneAdd(modelToScreen, row) \
	x	= mWorldToScreen.m[0][3]	+ mWorldToScreen.m[0][row];\
	y	= mWorldToScreen.m[1][3]	+ mWorldToScreen.m[1][row];\
	z	= mWorldToScreen.m[2][3]	+ mWorldToScreen.m[2][row];\
	w	= -(mWorldToScreen.m[3][3]  + mWorldToScreen.m[3][row])

#define rPreparePlaneSub(modelToScreen, row) \
	x	= mWorldToScreen.m[0][3] - mWorldToScreen.m[0][row];\
	y	= mWorldToScreen.m[1][3] - mWorldToScreen.m[1][row];\
	z	= mWorldToScreen.m[2][3] - mWorldToScreen.m[2][row];\
	w	= -(mWorldToScreen.m[3][3] - mWorldToScreen.m[3][row])

#define rOffScreen(lowPoint, hiPoint) \
	(   (x * ((x>0) ? hiPoint##X : lowPoint##X) + \
		 y * ((y>0) ? hiPoint##Y : lowPoint##Y) + \
		 z * ((z>0) ? hiPoint##Z : lowPoint##Z)) < w)

/*
 * Checks to see if a model is on-screen
 */
static bool LightOffscreen(const Point3 *midPoint, float radius)
{
	register float x;
	register float y;
	register float z;
	register float w;
	register float lowX = midPoint->x - radius;
	register float lowY = midPoint->y - radius;
	register float lowZ = midPoint->z - radius;
	register float hiX = midPoint->x + radius;
	register float hiY = midPoint->y + radius;
	register float hiZ = midPoint->z + radius;

	Bool	accept = TRUE;
	Bool	nearClipped = FALSE;

	/* Check the near plane (Z==0 PLANE) */
	x = mWorldToScreen.m[0][2];
	y = mWorldToScreen.m[1][2];
	z = mWorldToScreen.m[2][2];
	w = -mWorldToScreen.m[3][2];
	/* Cull off the near plane */
	if (rOffScreen (low, hi))
		return TRUE;

	/* Check the far plane */
	rPreparePlaneSub (modelToScreen, 2);
	/* Cull off the far plane */
	if (rOffScreen (low, hi))
		return TRUE;

	/* Check the left plane */
	rPreparePlaneAdd (modelToScreen, 0);
	/* Cull off the left plane */
	if (rOffScreen (low, hi))
		return TRUE;

	/* Check the right plane */
	rPreparePlaneSub (modelToScreen, 0);
	/* Cull off the right plane */
	if (rOffScreen (low, hi))
		return TRUE;

	/* Check the top plane */
	rPreparePlaneAdd (modelToScreen, 1);
	/* Cull off the top plane */
	if (rOffScreen (low, hi))
		return TRUE;

	/* Check the bottom plane */
	rPreparePlaneSub (modelToScreen, 1);
	/* Cull off the bottom plane */
	if (rOffScreen (low, hi))
		return TRUE;

	return FALSE;
}
