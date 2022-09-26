#ifndef _MACROS_H
#define _MACROS_H

#ifndef PI
#define PI 3.141592654f
#endif
#define PI2 (PI*2.0f)
#define HPI	(PI/2.0f)
#define asize(a)			(sizeof(a)/sizeof(a[0]))
#define SQR(a)				((a)*(a))
#define ANG(a)				(Angle)((a) * (65536.f / PI * 2.f))
#define RAD(a)				(Angle)((a) * (65536.f / 360.f))
#ifndef ABS
#define ABS(a)				((a) < 0 ? -(a) : (a))
#endif
#ifndef RANGE
#define RANGE(a,b,c)		((b) < (a) ? (a) : (b) > (c) ? (c) : (b))
#endif
#define LERP(a,b,alpha) ((a) + (((b)-(a))*(alpha)))

#define FLOAT_TO_UINT32(a)	(*(Uint32 *)&(a))
#define FLOAT_IS_NEG(a)		(FLOAT_TO_UINT32(a) & 0x80000000)
#define FLOAT_UNITY			(Uint32)(0x3f800000)
#define CLAMP(a)			((FLOAT_IS_NEG(a)) ? 0.f : (FLOAT_TO_UINT32(a) > FLOAT_UNITY) ? 1.f : (a))
#define CLAMP_NEG(a)		((FLOAT_IS_NEG(a)) ? 0.f : (a))

#endif
