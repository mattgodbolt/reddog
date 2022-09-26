#ifndef _BANDS_H
#define _BANDS_H

// The maximum number of bands
#define MAX_BANDS 64
// The number of matrices that can be stored in the buffer
// 1024 is about 48K
#define MAX_BAND_MATRICES 1024

// The maximum number of trails
#define MAX_TRAILS 16
// The maximum number of trail points
#define MAX_TRAIL_POINTS	256

// Shape enumeration
#define MAX_SHAPE_SIDES 8
typedef enum {
	SINGLEPOLY,
	TRIANGLE,
	TUBE,
	ST_MAX
} ShapeType;

// A handle to a band
typedef int BandHandle;

// Draws all the bands
void bDraw(void);
// Updates the bands
void bUpdate(void);
// Reset the band structure
void bReset (void);

//Clean Up
void bClean();
// Create a band
#ifndef MAX_INCLUDED
BandHandle bCreate (ShapeType sType, int mType, int maxSegs, LightingValue colour[2], Matrix43 *incMat);
#endif

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