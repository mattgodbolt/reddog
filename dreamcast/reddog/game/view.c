/*
 * View.c
 * View setting routines
 */

#include "RedDog.h"
#include "Memory.h"
#include "View.h"
#include "Render\Internal.h"

/* Global variables */
Camera *currentCamera = NULL;

/* Static variables */
static float			fogNearZ = 0.f, fogFarZ = 0.f;
static Colour			fogColour = { 0xff00ff00 };
static float			nearZ = 0.f, farZ = 0.f, screenAngle = 0.f, fogDen = 0;
static Uint16			screenX = 0, screenY = 0, screenMidX = 0, screenMidY = 0;
#pragma section Matrices
static const Matrix		coordSystemChange = {
	{ { 1.0f, 0.f, 0.f, 0.f}, { 0.f, 0.f, 1.f, 0.f }, {0.f, -1.f, 0.f, 0.f}, {0.f, 0.f, 0.f, 1.f}}
};
static Matrix					roll, move, tempM;
#pragma section

/*
 * Create a camera (automatically deleted at mission end)
 */
Camera *CameraCreate (void)
{
	Camera *cam;

	cam = MissionAlloc (sizeof (Camera));

	dAssert (cam != NULL, "Unable to allocate memory for a camera");

	cam->nearZ				= 0.25f;
	cam->farZ				= 300.f;

	cam->fogNearZ			= 500.f;
	cam->fogColour.colour	= 0;
	
	SetWindow (cam, WINDOW_FULLSCREEN);
	cam->screenAngle		= 1.1f;

	cam->flags = 0;

	cam->pos.x = cam->pos.y = cam->pos.z = 0.f;
	return cam;
}

/*
 * Set up a camera to be a window of some sort
 */
void SetWindow (Camera *cam, int type)
{
	switch (type)
	{
	case WINDOW_FULLSCREEN:
	default:
		cam->screenX			= PHYS_SCR_X;
		cam->screenY			= PHYS_SCR_Y;
		cam->screenMidX			= PHYS_SCR_X / 2;
		cam->screenMidY			= PHYS_SCR_Y / 2 + rGlobalYAdjust;
		break;
	case WINDOW_TOP:
		cam->screenX			= PHYS_SCR_X;
		cam->screenY			= 256;
		cam->screenMidX			= PHYS_SCR_X / 2;
		cam->screenMidY			= 128 + rGlobalYAdjust;
		break;
	case WINDOW_BOTTOM:
		cam->screenX			= PHYS_SCR_X;
		cam->screenY			= 224;
		cam->screenMidX			= PHYS_SCR_X / 2;
		cam->screenMidY			= 256+112 /*+ rGlobalYAdjust */;
		break;
	case WINDOW_TL:
		cam->screenX			= PHYS_SCR_X / 2;
		cam->screenY			= 256;
		cam->screenMidX			= PHYS_SCR_X / 4;
		cam->screenMidY			= 128 + rGlobalYAdjust;
		break;
	case WINDOW_TR:
		cam->screenX			= PHYS_SCR_X / 2;
		cam->screenY			= 256;
		cam->screenMidX			= 3 * PHYS_SCR_X / 4;
		cam->screenMidY			= 128 + rGlobalYAdjust;
		break;
	case WINDOW_BL:
		cam->screenX			= PHYS_SCR_X / 2;
		cam->screenY			= 224;
		cam->screenMidX			= PHYS_SCR_X / 4;
		cam->screenMidY			= 256+112;
		break;
	case WINDOW_BR:
		cam->screenX			= PHYS_SCR_X / 2;
		cam->screenY			= 224;
		cam->screenMidX			= 3 * PHYS_SCR_X / 4;
		cam->screenMidY			= 256+112;
		break;
	}
}

/*
 * Set a camera as being current
 */
void CameraSet (Camera *cam, float rollAng)
{
	Point3					temp;
	int minX, maxX, minY, maxY;
	extern float GlobalFogFar, GlobalFogDensity;
				
	dAssert (cam != NULL, "No camera!");

	currentCamera = cam;
	
	// Right - none of this far Z past the fog far Z bollocks
	if (GlobalFogDensity == 1.f && cam->farZ > GlobalFogFar)
		cam->farZ = GlobalFogFar;

	//if (!cam->HoldTime)
	//{
	/* Recalculate the fog, if necessary */
	if ((fogNearZ != cam->fogNearZ) ||
		(fogFarZ != GlobalFogFar) ||
		(fogDen != GlobalFogDensity) ||
		(fogColour.colour != cam->fogColour.colour))
	{

		fogNearZ	= cam->fogNearZ;
		fogFarZ		= GlobalFogFar;
		fogColour	= cam->fogColour;
		fogDen		= GlobalFogDensity;

		rSetFog (fogColour.colour, fogNearZ, fogFarZ, GlobalFogDensity);
	}


	if ((screenX != cam->screenX) ||
		(screenY != cam->screenY) ||
		(screenMidX != cam->screenMidX) ||
		(screenMidY != cam->screenMidY) ||
		(screenAngle != cam->screenAngle) ||
		(farZ != cam->farZ) ||
		(nearZ != cam->nearZ))
	{

		dAssert (cam->screenX <= PHYS_SCR_X, "Camera setting too large");
		dAssert (cam->screenY <= PHYS_SCR_Y, "Camera setting too large");
		if (PAL)
			rSetPerspective (cam->screenAngle, (512.f / 480.f) * (float)cam->screenX / (float) cam->screenY, cam->nearZ, cam->farZ);
		else
			rSetPerspective (cam->screenAngle, (float)cam->screenX / (float) cam->screenY, cam->nearZ, cam->farZ);
		pViewMidPoint.x	= cam->screenMidX;
		pViewMidPoint.y	= cam->screenMidY;
		pViewSize.x		= cam->screenX * 0.5f;
		pViewSize.y		= cam->screenY * 0.5f;
		
		screenX		= cam->screenX;
		screenY		= cam->screenY;
		screenMidX	= cam->screenMidX;
		screenMidY	= cam->screenMidY;
		screenAngle = cam->screenAngle;
		farZ		= cam->farZ;
		nearZ		= cam->nearZ;

	}

	/* Set the clipping region */
	if (cam->flags & CAMERA_BLACKBARS) {
		float yTop, yBottom;
		extern Material fadeMat;
		kmSetUserClipping (&vertBuf, KM_OPAQUE_POLYGON, 0, 0, 255, 255);
		kmSetUserClipping (&vertBuf, KM_OPAQUE_MODIFIER, 0, 0, 255, 255);
		kmSetUserClipping (&vertBuf, KM_TRANS_POLYGON, 0, 0, 255, 255);
		kmSetUserClipping (&vertBuf, KM_TRANS_MODIFIER, 0, 0, 255, 255);
		// Only draw black bars at top and bottom at the moment
		yTop	= cam->screenMidY - (cam->screenY / 2);
		yBottom	= cam->screenMidY + (cam->screenY / 2);
		rSetMaterial (&fadeMat);
		kmxxGetCurrentPtr (&vertBuf);
		kmxxStartVertexStrip (&vertBuf);
		kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		0, 0, 10e6f, 0xff000000);
		kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		0, yTop, 10e6f, 0xff000000);
		kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		PHYS_SCR_X, 0, 10e6f, 0xff000000);
		kmxxSetVertex_0 (KM_VERTEXPARAM_ENDOFSTRIP, PHYS_SCR_X, yTop, 10e6f, 0xff000000);
		kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		0, yBottom, 10e6f, 0xff000000);
		kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		0, PHYS_SCR_Y, 10e6f, 0xff000000);
		kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		PHYS_SCR_X, yBottom, 10e6f, 0xff000000);
		kmxxSetVertex_0 (KM_VERTEXPARAM_ENDOFSTRIP, PHYS_SCR_X, PHYS_SCR_Y, 10e6f, 0xff000000);
		kmxxReleaseCurrentPtr (&vertBuf);
	} else {
		minX = (cam->screenMidX - (cam->screenX / 2)) / 32;
		maxX = (cam->screenMidX + (cam->screenX / 2)) / 32 - 1;
		minY = (cam->screenMidY - (cam->screenY / 2)) / 32;
		maxY = (cam->screenMidY + (cam->screenY / 2)) / 32 - 1;

#if 1
		kmSetUserClipping (&vertBuf, KM_OPAQUE_POLYGON, minX, minY, maxX, maxY);
		kmSetUserClipping (&vertBuf, KM_OPAQUE_MODIFIER, minX, minY, maxX, maxY);
		kmSetUserClipping (&vertBuf, KM_TRANS_POLYGON, minX, minY, maxX, maxY);
		kmSetUserClipping (&vertBuf, KM_TRANS_MODIFIER, minX, minY, maxX, maxY);
#else
		kmSetUserClipping (&vertBuf, KM_OPAQUE_POLYGON, 0, 0, 255, 255);
		kmSetUserClipping (&vertBuf, KM_OPAQUE_MODIFIER, 0, 0, 255, 255);
		kmSetUserClipping (&vertBuf, KM_TRANS_POLYGON, 0, 0, 255, 255);
		kmSetUserClipping (&vertBuf, KM_TRANS_MODIFIER, 0, 0, 255, 255);
#endif
	}
	//}
	/* Set up the view vector */
	switch (cam->type) {
	case CAMERA_LOOKATPOINT:
		temp.x = cam->u.lookAtPos.x - cam->pos.x;
		temp.y = cam->u.lookAtPos.y - cam->pos.y;
		temp.z = cam->u.lookAtPos.z - cam->pos.z;
		
		v3Normalise (&temp);
		dAssert (FALSE, "Not imped");

		break;

	case CAMERA_LOOKAROUND:
		mRotateXYZ (&tempM, -cam->u.lookDir.rotX, -cam->u.lookDir.rotY, -cam->u.lookDir.rotZ);
		mPostRotateZ(&tempM, rollAng);
		mMultiply (&roll, &coordSystemChange, &tempM);

		mTranslate (&move, -cam->pos.x, -cam->pos.y, -cam->pos.z);
		mMultiply (&mWorldToView, &move, &roll);
		break;

	default:
		dAssert (FALSE, "Unknown camera type");
		break;
	}

	mMultiply (&mWorldToScreen, &mWorldToView, &mPersp);

	// Set up stuff for the model renderer
	rSetModelContext();
	rSetBackgroundColour (cam->fogColour.colour);

	// Set the text window
	psSetWindow (-1, -1, -1, -1);
}
