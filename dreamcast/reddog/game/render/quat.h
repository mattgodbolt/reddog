/*
 * Quat.h
 * Quaternion typedefs
 */

#ifndef _RDQUAT_H
#define _RDQUAT_H
  
	typedef struct Quat
	{
		float x;
		float y;
		float z;
		float w;
	} QUAT;

extern void QuatSlerp(QUAT * from, QUAT *to, float t, QUAT *res);
extern void QuatToMat(QUAT *quat, Matrix *m);

//extern void NormalizeQuat(QUAT *quat);
	
#endif
	 