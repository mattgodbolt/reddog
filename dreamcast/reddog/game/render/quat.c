/*
 * Quat.c
 * Quaternion related functions
 */
#include "..\Reddog.h"
#include "Internal.h"
#include "Quat.h"

#define DELTA 0.01

void QuatSlerp(QUAT * from, QUAT *to, float t, QUAT *res)
{
	float to10,to11,to12,to13;
	double omega,cosom,sinom,scale0,scale1;
	float *fres = (float*)res;

	/*cosom = from->x * to->x + from->y * to->y + from->z * to->z + from->w * to->w;
	  */
	cosom = FIPR(from->x,from->y,from->z,from->w,to->x,to->y,to->z,to->w); 
	if ( cosom < 0.0)
	{
		cosom = -cosom;
		to10 = -to->x;
		to11 = -to->y;
		to12 = -to->z;
		to13 = -to->w;
	}
	else
	{
		to10 = to->x;
		to11 = to->y;
		to12 = to->z;
		to13 = to->w;
	}

	if ((1.0f -cosom) > DELTA)
	{
	
		omega = acos(cosom);
	   
		/*SinCos(omega,&val);
		sinom = 1.0f / val.sin;
	   
		SinCos((1.0 - t) * omega,&val);
		scale0 = val.sin * sinom;
	  
		SinCos(t * omega,&val);
		scale1 = val.sin * sinom;
		*/

		//was this
	   	sinom = 1.0 / sin(omega);
	   	scale0 = sin((1.0-t) * omega) * sinom;
	   	scale1 = sin(t * omega) * sinom;

		//..and was this
		//	scale0 = sin(omega-t) / sinom;
		//	scale1 = sin(t) / sinom;
		 		   
	}
	else
	{
		scale0 = 1.0f -t;
		scale1 = t;
	}
	
	//x
	*fres++ = scale0 * from->x + scale1 * to10;
  	//y
	*fres++ = scale0 * from->y + scale1 * to11;
	//z
	*fres++ = scale0 * from->z + scale1 * to12;
	//w
	*fres++ = scale0 * from->w + scale1 * to13;

   //	res->x = scale0 * from->x + scale1 * to10;

   //	res->y = scale0 * from->y + scale1 * to11;

   //	res->z = scale0 * from->z + scale1 * to12;

	//res->w = scale0 * from->w + scale1 * to13;


}

void QuatToMat(QUAT *quat, Matrix *m)
{
  /*	float wx,wy,wz,xx,yy,yz,xy,xz,zz,x2,y2,z2; */

	/*BRENDER METHOD*/

	float xs,ys,zs;
	float wx,wy,wz;
	float xx,xy,xz;
	float yy,yz,zz;
	float *mp = (float*)m;

	xs = quat->x * 2.0f;
	ys = quat->y * 2.0f;
	zs = quat->z * 2.0f;

	wx = (quat->w * xs);
	wy = (quat->w * ys);
	wz = (quat->w * zs);


	xx = (quat->x * xs);
	xy = (quat->x * ys);
	xz = (quat->x * zs);

	yy = (quat->y * ys);
	yz = (quat->y * zs);
	zz = (quat->z * zs);

	/*m->m[0][0] = 1.0f - (yy+zz);
	m->m[1][0] = xy - wz;
	m->m[2][0] = xz + wy;

  	m->m[0][1] = xy + wz;
	m->m[1][1] = 1.0f - (xx+zz);
	m->m[2][1] = yz-wx;

	m->m[0][2] = xz - wy;
	m->m[1][2] = yz + wx;
	m->m[2][2] = 1.0f - (xx+yy);
	*/


	*mp++ = 1.0f - (yy+zz);
	*mp++ = xy - wz;
	*mp++ = xz + wy;
    *mp++  = 0.0f;


  	*mp++ = xy + wz;
	*mp++ = 1.0f - (xx+zz);
	*mp++ = yz-wx;
    *mp++  = 0.0f;


	*mp++ = xz - wy;
	*mp++ = yz + wx;
	*mp++ = 1.0f - (xx+yy);
    *mp++  = 0.0f;
   
	*mp++  = 0.0f;
    *mp++  = 0.0f;
    *mp++  = 0.0f;
    *mp++  = 1.0f;

  //was this
  /*	

	m->m[0][0] = 1.0f - (yy+zz);
	m->m[0][1] = xy - wz;
	m->m[0][2] = xz + wy;

  	m->m[1][0] = xy + wz;
	m->m[1][1] = 1.0f - (xx+zz);
	m->m[1][2] = yz-wx;

	m->m[2][0] = xz - wy;
	m->m[2][1] = yz + wx;
	m->m[2][2] = 1.0f - (xx+yy);


   

	m->m[3][0] = 0.0f;

	m->m[3][1] = 0.0f;

	m->m[3][2] = 0.0f;

	m->m[0][3] = 0.0f;

	m->m[1][3] = 0.0f;

	m->m[2][3] = 0.0f;

	m->m[3][3] = 1.0f;
	*/

}

void QuatSlerpMat(QUAT * from, QUAT *to, float t, QUAT *res, Matrix *m)
{
	register float scale0,scale1;
	register float *fres, *fRead;
	register float xs,ys,zs;
	register float omega,cosom,sinom;



	float to10,to11,to12,to13;
	/*BRENDER METHOD*/

	float wx,wy,wz;
	float xx,xy,xz;
	float yy,yz,zz;
	float *mp = (float*)m;


	fres = (float*)res;
	prefetch (fres);

	fRead = &to->x;
	cosom = fipr((void *)from, (void *)to); 
	if ( cosom < 0.0)
	{
		cosom = -cosom;
		to10 = -*fRead++;
		to11 = -*fRead++;
		to12 = -*fRead++;
		to13 = -*fRead;
	}
	else
	{
		to10 = *fRead++;
		to11 = *fRead++;
		to12 = *fRead++;
		to13 = *fRead;
	}

	// Temporarily use sinom
	sinom = 1.f - cosom;
	if (sinom > DELTA)
	{
		/*
		omega = acos(cosom);
	   	sinom = 1.0 / sin(omega);
	   	scale0 = sin((1.0-t) * omega) * sinom;
	   	scale1 = sin(t * omega) * sinom;
		  */
	
	  //	omega = acos(cosom);
		// MattG again, multiplying this out:
/*

		if (cosom < 0)
			omega = PI/2.0 + (-cosom * PI/2.0);
		else
			omega = PI/2.0 - (cosom * PI/2.0);

		becomes

		if (cosom < 0)
			omega = PI/2 + (cosom * PI/2);
		else
			omega = PI/2 - (cosom * PI/2);
		pi2/2 * (1-cosom)
*/
	
	   
		if (cosom < 0)
			omega = PI/2.0f * (1+cosom);
		else
			omega = PI/2.0f * sinom;
		  
		sinom = 1.0 / sin(omega);
	   	scale0 = sin((1.0-t) * omega) * sinom;
	   	scale1 = sin(t * omega) * sinom;

		 		   
	}
	else
	{
		scale0 = 1.0f -t;
		scale1 = t;
	}

	fRead = &from->x;
	//x
	*fres++ = scale0 * *fRead++ /*from->x*/ + scale1 * to10;
  	//y
	*fres++ = scale0 * *fRead++ /*from->y*/ + scale1 * to11;
	//z
	*fres++ = scale0 * *fRead++ /*from->z*/ + scale1 * to12;
	//w
	*fres = scale0 * *fRead /*from->w*/ + scale1 * to13;
   
	prefetch (mp);
	fRead = &res->x;
	xs = *fRead++ /*res->x*/ * 2.0f;
	ys = *fRead++ /*res->x*/ * 2.0f;
	zs = *fRead   /*res->x*/ * 2.0f;

	wx = *fres * xs;
	wy = *fres * ys;
	wz = *fres-- * zs;
	//fres now back to z
	zz = *fres-- * zs;
	//fres now back to y
	yy = *fres * ys;
	yz = *fres-- * zs;
	//fres now back to x
	xx = *fres * xs;
	xy = *fres * ys;
	xz = *fres * zs;

	prefetch (mp+4);
	*mp++ = 1.0f - (yy+zz);
	*mp++ = xy - wz;
	*mp++ = xz + wy;
    *mp++  = 0.0f;


	prefetch (mp+4);
  	*mp++ = xy + wz;
	*mp++ = 1.0f - (xx+zz);
	*mp++ = yz-wx;
    *mp++  = 0.0f;


	prefetch (mp+4);
	*mp++ = xz - wy;
	*mp++ = yz + wx;
	*mp++ = 1.0f - (xx+yy);
    *mp++  = 0.0f;
   
	*mp++  = 0.0f;
    *mp++  = 0.0f;
    *mp++  = 0.0f;
    *mp++  = 1.0f;

}


#if 0
static void NormalizeQuat(QUAT *quat)
{
	float magnitude;
	magnitude = (quat->x * quat->x) + 
				(quat->y * quat->y) + 
				(quat->z * quat->z) + 
				(quat->w * quat->w);

	quat->x = quat->x / magnitude;
	quat->y = quat->y / magnitude;
	quat->z = quat->z / magnitude;
	quat->w = quat->w / magnitude;
}
#endif
#if 0
void MatToQuat(Matrix *m,QUAT *quat)
{
	float s;
	float q[4];
	int n[] = {1,2,0};
	int i=0;
	int j;
	int k;

 /*BRENDER*/

	float tr = m->m[0][0] + m->m[1][1] + m->m[2][2];

 
	if (tr >= 0.0)
	{
		s = (float)sqrt(tr + 1.0f);
		quat->w = s/2.0f;
		s = (0.5f/s);

		quat->x = (m->m[1][2] - m->m[2][1]) * s;

		quat->y = (m->m[2][0] - m->m[0][2]) * s;

		quat->z = (m->m[0][1] - m->m[1][0]) * s;



	}
	else
	{
	
		if (m->m[1][1] > m->m[0][0])
			i=1;
		if (m->m[2][2] > m->m[i][i])
			i=2;

		j=n[i];
		 k=n[j];

		s = (float)sqrt((m->m[i][i] - (m->m[j][j] + m->m[k][k])) + 1.0);
		q[i] = s / 2.0f;

		if (s)
			s=0.5f /s ;

		q[3] = (m->m[j][k] - m->m[k][j]) * s;

		q[j] = (m->m[i][j] + m->m[j][i]) * s;

		q[k] = (m->m[i][k] + m->m[k][i]) * s;

		quat->x = q[0];

		quat->y = q[1];

		quat->z = q[2];

		quat->w = q[3];

	}
}
#endif

/*FUNCTIONS NOT USED AT THE MOMENT BUT MAYBE LATER*/

#if 0

#define M_PI        3.14159

void QuatToAxisAngle(QUAT *quat,QUAT *axisAngle)
{
	float scale,tw;
	tw = (float)acos(quat->w) * 2;
	scale = (float)sin(tw / 2.0);
	axisAngle->x = quat->x / scale;
	axisAngle->y = quat->y / scale;
	axisAngle->z = quat->z / scale;

	/* NOW CONVERT THE ANGLE OF ROTATION BACK TO DEGREES */
	axisAngle->w = (tw * (360 / 2)) / (float)M_PI;
}


void EulerToQuat(float roll, float pitch, float yaw, QUAT *quat) 
{
	float cr,cp,cy,sr,sp,sy,cpcy,spsy;
	float tx,ty,tz,cx,cz,cc,cs,ss,sc,sx,sz;

	cr = cos(roll/2.0f);
	cp = cos(pitch/2.0f);
	cy = cos(yaw/2.0f);


	sr = sin(roll/2.0f);
	sp = sin(pitch/2.0f);
	sy = sin(yaw/2.0f);

	cpcy = cp * cy;
	spsy = sp * sy;

	quat->w =(cr * cpcy) + (sr * spsy);
	quat->x =(sr * cpcy) - (cr * spsy);
	quat->y =(cr * sp * cy) + (sr * cp * sy);
	quat->z =(cr * cp * sy) - (sr * sp * cy);
	
}

#endif

