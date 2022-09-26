/*
 * View.h
 * Viewing routines
 */

#ifndef _VIEW_H
#define _VIEW_H

/*
 * Definition of a camera as used to set up
 * Ninja view
 */
typedef struct tagCamera
{
	float			nearZ, farZ;	/* Near and far Z (positive values) */
	float			fogNearZ;		/* Point at which fogging begins */
	Colour			fogColour;		/* Colour of the fog */
	Point3			pos;			/* Current position (in world space) */
	Uint16			screenX;		/* The X size of the screen */
	Uint16			screenY;		/* The Y size of the screen */
	Uint16			screenMidX;		/* The middle X position of the screen */
	Uint16			screenMidY;		/* The middle Y position of the screen */
	float			screenAngle;	/* The angle in radians of the screen's FOV */
	Uint32			flags;
#define CAMERA_BLACKBARS		1	// Don't use hardware clipping, draw black bars instead
	enum
	{
		CAMERA_LOOKATPOINT,
		CAMERA_LOOKAROUND
	}				type;			/* Which type of camera this is */
	union {
		Point3			lookAtPos;	/* Point to look at */
		struct {
			float		rotX, rotY, rotZ;
		}				lookDir;	/* Direction to look in */
	} u;
	Uint16			HoldTime;		/* TIME TO HOLD LAST FRAME POSITION BEFORE A CAMERA SWITCH*/

} Camera;

/* Global 'current' camera */
extern Camera *currentCamera;

/*
 * Create a camera with 'Mission' lifetime
 */
extern Camera *CameraCreate (void);

/*
 * Set a camera as being current
 */
extern void CameraSet (Camera *cam, float rollAng);

/*
 * Dome functions
 */
//extern void LoadDome (Stream *s);
//extern void FreeDome (void);
//extern void DrawDome (Camera *cam);

/*
 * Set up a camera to be a window of some sort
 */
#define WINDOW_FULLSCREEN	0
#define WINDOW_TOP			1
#define WINDOW_BOTTOM		2
#define WINDOW_TL			3
#define WINDOW_TR			4
#define WINDOW_BL			5
#define WINDOW_BR			6
void SetWindow (Camera *cam, int type);

/*
 * Global variable which lets us know if we're splitscreening or not
 */

#endif