#include "..\RedDog.h"
#include "Internal.h"

/*
 * Draw a line 'on top'
 */
Material	lineMat = { 0, 0, 0.f, 0.f, MF_ALPHA };

static void rLineInternal (float x0, float y0, float rW0, float x1, float y1, float rW1, Colour stCol, Colour stEnd, float thickness)
{
	float		length, scale, delX, delY, shiftX, shiftY;
	/* Begin the triangle strip */
	rSetMaterial (&lineMat);

	delX = x1 - x0;
	delY = y0 - y1;
	length = SQR(delY) + SQR(delX);
	if (length < 0.00001f)
		return;
	scale = thickness * isqrt (length);
	shiftX = scale * delY;
	shiftY = scale * delX;

	kmxxGetCurrentPtr (&vertBuf);
	kmxxStartVertexStrip (&vertBuf);

	kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		x0 + shiftX, y0 + shiftY, rW0, stCol.colour);
	kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		x1 + shiftX, y1 + shiftY, rW1, stEnd.colour);
	kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		x0 - shiftX, y0 - shiftY, rW0, stCol.colour);
	kmxxSetVertex_0 (KM_VERTEXPARAM_ENDOFSTRIP,	x1 - shiftX, y1 - shiftY, rW1, stEnd.colour);

	kmxxReleaseCurrentPtr (&vertBuf);
}

void rLineOnTop (Point3 *start, Point3 *end, Colour stCol, Colour stEnd)
{
	Point		stCam, endCam;
	float		rW1, rW2;
	float		x0, y0, x1, y1;

	/* Calculate modelToScreen */
	mLoadModelToScreenScaled();

	*(Point3 *)&stCam = *start;
	*(Point3 *)&endCam = *end;
	stCam.w = 1.f;
	endCam.w = 1.f;
	mPoint (&stCam, &stCam);
	rW1 = 1.f / stCam.w;
	mPoint (&endCam, &endCam);
	rW2 = 1.f / endCam.w;

	/* Perform clipping */
	if (stCam.z < 0) {
		float alpha;
		if (endCam.z < 0)
			return;	/* Completely offscreen */
		alpha = endCam.z / (endCam.z - stCam.z);
		stCam.x = LERP(endCam.x, stCam.x, alpha);
		stCam.y = LERP(endCam.y, stCam.y, alpha);
		stCol.colour = ColLerp (stEnd.colour, stCol.colour, alpha);
		rW1 = rWnearVal;
	} else if (endCam.z < 0) {
		float alpha;
		alpha = endCam.z / (endCam.z - stCam.z);
		endCam.x = LERP(endCam.x, stCam.x, alpha);
		endCam.y = LERP(endCam.y, stCam.y, alpha);
		stEnd.colour = ColLerp (stEnd.colour, stCol.colour, alpha);
		rW2 = rWnearVal;
	}

	x0 = X_MID + rW1 * stCam.x;
	x1 = X_MID + rW2 * endCam.x;
	y0 = Y_MID + rW1 * stCam.y;
	y1 = Y_MID + rW2 * endCam.y;

	rLineInternal (x0, y0, 1000.f, x1, y1, 1000.f, stCol, stEnd, 2);
}

void rLine(Point3 *start, Point3 *end, Colour stCol, Colour stEnd)
{
	Point		stCam, endCam;
	float		rW1, rW2;
	float		x0, y0, x1, y1;

	/* Calculate modelToScreen */
	mLoadModelToScreenScaled();

	*(Point3 *)&stCam = *start;
	*(Point3 *)&endCam = *end;
	stCam.w = 1.f;
	endCam.w = 1.f;
	mPoint (&stCam, &stCam);
	rW1 = 1.f / stCam.w;
	mPoint (&endCam, &endCam);
	rW2 = 1.f / endCam.w;

	/* Perform clipping */
	if (stCam.z < 0) {
		float alpha;
		if (endCam.z < 0)
			return;	/* Completely offscreen */
		alpha = endCam.z / (endCam.z - stCam.z);
		stCam.x = LERP(endCam.x, stCam.x, alpha);
		stCam.y = LERP(endCam.y, stCam.y, alpha);
		stCol.colour = ColLerp (stEnd.colour, stCol.colour, alpha);
		rW1 = rWnearVal;
	} else if (endCam.z < 0) {
		float alpha;
		alpha = endCam.z / (endCam.z - stCam.z);
		endCam.x = LERP(endCam.x, stCam.x, alpha);
		endCam.y = LERP(endCam.y, stCam.y, alpha);
		stEnd.colour = ColLerp (stEnd.colour, stCol.colour, alpha);
		rW2 = rWnearVal;
	}

	x0 = X_MID + rW1 * stCam.x;
	x1 = X_MID + rW2 * endCam.x;
	y0 = Y_MID + rW1 * stCam.y;
	y1 = Y_MID + rW2 * endCam.y;

	rLineInternal (x0, y0, rW1, x1, y1, rW2, stCol, stEnd, 1);
}

void rWeldLine(Point3 *start, Point3 *end, Colour stCol, Colour stEnd)
{
	Point		stCam, endCam;
	float		rW1, rW2;
	float		x0, y0, x1, y1;

	/* Calculate modelToScreen */
	mLoadModelToScreenScaled();

	*(Point3 *)&stCam = *start;
	*(Point3 *)&endCam = *end;
	stCam.w = 1.f;
	endCam.w = 1.f;
	mPoint (&stCam, &stCam);
	rW1 = 1.f / stCam.w;
	mPoint (&endCam, &endCam);
	rW2 = 1.f / endCam.w;

	/* Perform clipping */
	if (stCam.z < 0) {
		float alpha;
		if (endCam.z < 0)
			return;	/* Completely offscreen */
		alpha = endCam.z / (endCam.z - stCam.z);
		stCam.x = LERP(endCam.x, stCam.x, alpha);
		stCam.y = LERP(endCam.y, stCam.y, alpha);
		stCol.colour = ColLerp (stEnd.colour, stCol.colour, alpha);
		rW1 = rWnearVal;
	} else if (endCam.z < 0) {
		float alpha;
		alpha = endCam.z / (endCam.z - stCam.z);
		endCam.x = LERP(endCam.x, stCam.x, alpha);
		endCam.y = LERP(endCam.y, stCam.y, alpha);
		stEnd.colour = ColLerp (stEnd.colour, stCol.colour, alpha);
		rW2 = rWnearVal;
	}

	x0 = X_MID + rW1 * stCam.x;
	x1 = X_MID + rW2 * endCam.x;
	y0 = Y_MID + rW1 * stCam.y;
	y1 = Y_MID + rW2 * endCam.y;

	rLineInternal (x0, y0, rW1+0.0001f, x1, y1, rW2+0.0001f, stCol, stEnd, 1.1f);
	stCol.argb.a = stEnd.argb.a = rand() & 0x3f + 0x10;
	rLineInternal (x0, y0, rW1, x1, y1, rW2, stCol, stEnd, 4.f + frand() * 2.f);
}

void Line (float x1, float y1, float z1, float x2, float y2, float z2, Uint32 col1, Uint32 col2)
{
	Point		stCam, endCam;
	Colour c1, c2;
	float		rW1, rW2;
	float		x0, y0;

	c1.colour = col1;
	c2.colour = col2;

	mUnit (&mCurMatrix);

	/* Calculate modelToScreen */
	mLoadModelToScreenScaled();

	stCam.x = x1;
	stCam.y = y1;
	stCam.z = z1;
	endCam.x = x2;
	endCam.y = y2;
	endCam.z = z2;
	stCam.w = 1.f;
	endCam.w = 1.f;
	mPoint (&stCam, &stCam);
	rW1 = 1.f / stCam.w;
	mPoint (&endCam, &endCam);
	rW2 = 1.f / endCam.w;

	/* Perform clipping */
	if (stCam.z < 0) {
		float alpha;
		if (endCam.z < 0)
			return;	/* Completely offscreen */
		alpha = endCam.z / (endCam.z - stCam.z);
		stCam.x = LERP(endCam.x, stCam.x, alpha);
		stCam.y = LERP(endCam.y, stCam.y, alpha);
		rW1 = rWnearVal;
	} else if (endCam.z < 0) {
		float alpha;
		alpha = endCam.z / (endCam.z - stCam.z);
		endCam.x = LERP(endCam.x, stCam.x, alpha);
		endCam.y = LERP(endCam.y, stCam.y, alpha);
		rW2 = rWnearVal;
	}

	x0 = X_MID + rW1 * stCam.x;
	x1 = X_MID + rW2 * endCam.x;
	y0 = Y_MID + rW1 * stCam.y;
	y1 = Y_MID + rW2 * endCam.y;

	rLineInternal (x0, y0, rW1+0.0001f, x1, y1, rW2+0.0001f, c1, c1, 1.1f);
	rLineInternal (x0, y0, rW1, x1, y1, rW2, c2, c2, 4.f);

}
void rLine2D (float x0, float y0, float x1, float y1, float pri, Colour sCol, Colour eCol)
{
	rLineInternal (x0, y0, pri, x1, y1, pri, sCol, eCol, 0.503f);
}

void TexturedLine (STRAT *s, int objId, float dist, float thickness, 
					float x1, float y1, float z1, float x2, float y2, float z2)
{
	float uStart, uEnd;
	Point		stCam, endCam;
	Colour c1, c2;
	float		rW1, rW2;
	float		x0, y0;
	float		length, scale, delX, delY, shiftX, shiftY;
	Object *o;

	mUnit (&mCurMatrix);

	/* Calculate modelToScreen */
	mLoadModelToScreenScaled();

	stCam.x = x1;
	stCam.y = y1;
	stCam.z = z1;
	endCam.x = x2;
	endCam.y = y2;
	endCam.z = z2;
	stCam.w = 1.f;
	endCam.w = 1.f;

	// Calculate the starting and ending U values
	uStart = 0;
	if (dist) {
		uEnd = pDist ((Point3 *)&stCam, (Point3 *)&endCam) / dist;
	} else {
		uEnd = 1.f;
	}

	// Calculate screen positions
	mPoint (&stCam, &stCam);
	rW1 = 1.f / stCam.w;
	mPoint (&endCam, &endCam);
	rW2 = 1.f / endCam.w;

	thickness *= (rW1+rW2) * 0.5f;

	/* Perform clipping */
	if (stCam.z < 0) {
		float alpha;
		if (endCam.z < 0)
			return;	/* Completely offscreen */
		alpha = endCam.z / (endCam.z - stCam.z);
		stCam.x = LERP(endCam.x, stCam.x, alpha);
		stCam.y = LERP(endCam.y, stCam.y, alpha);
		uStart = LERP(uEnd, uStart, alpha);
		rW1 = rWnearVal;
	} else if (endCam.z < 0) {
		float alpha;
		alpha = endCam.z / (endCam.z - stCam.z);
		endCam.x = LERP(endCam.x, stCam.x, alpha);
		endCam.y = LERP(endCam.y, stCam.y, alpha);
		uEnd = LERP(uEnd, uStart, alpha);
		rW2 = rWnearVal;
	}

	x0 = X_MID + rW1 * stCam.x;
	x1 = X_MID + rW2 * endCam.x;
	y0 = Y_MID + rW1 * stCam.y;
	y1 = Y_MID + rW2 * endCam.y;

	/* Begin the triangle strip */
	o = s->objId[objId];
	while (!o->model) {
		dAssert (o->no_child, "No material");
		o = &o->child[0];
	}
	rSetMaterial (&o->model->material[0]);

	delX = x1 - x0;
	delY = y0 - y1;
	length = SQR(delY) + SQR(delX);
	scale = thickness * isqrt (length);
	shiftX = scale * delY;
	shiftY = scale * delX;

	kmxxGetCurrentPtr (&vertBuf);
	kmxxStartVertexStrip (&vertBuf);

	kmxxSetVertex_7 (KM_VERTEXPARAM_NORMAL,		x0 + shiftX, y0 + shiftY, rW1, uStart, 1, 1, 0);
	kmxxSetVertex_7 (KM_VERTEXPARAM_NORMAL,		x1 + shiftX, y1 + shiftY, rW2, uEnd, 1, 1, 0);
	kmxxSetVertex_7 (KM_VERTEXPARAM_NORMAL,		x0 - shiftX, y0 - shiftY, rW1, uStart, 0, 1, 0);
	kmxxSetVertex_7 (KM_VERTEXPARAM_ENDOFSTRIP,	x1 - shiftX, y1 - shiftY, rW2, uEnd, 0, 1, 0);

	kmxxReleaseCurrentPtr (&vertBuf);
}
