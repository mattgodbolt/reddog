/*
 * Matrix handling functions
 */

#include "..\RedDog.h"

#define FLT_EPSILON	1.192092896e-07F	/* smallest such that 1.0+FLT_EPSILON != 1.0 */

#define NC(a) dAssert(a != NULL, "NULL pointer passed to a matrix routine")

/*
 * Global variables
 */
#pragma section Matrices
#pragma aligndata8(foobarbaz)
#pragma aligndata32(mCurMatrix, mStack, mTempMatrix, modelToScreen, worldToModel, modelToView, viewToModel)
#pragma aligndata32(mWorldToScreen, mWorldToView, mPersp)
float foobarbaz[4][4];

/* Various matrices */
Matrix	mStack[MAX_MATRIX_STACK];
Matrix	mCurMatrix;
Matrix	mTempMatrix;
Matrix	modelToScreen;
Matrix	worldToModel;
Matrix	modelToView;
Matrix	viewToModel;
Matrix	mWorldToScreen;
Matrix	mWorldToView;
Matrix	mPersp;
Point2	pViewMidPoint, pViewSize;

float mUnitMatrix[4][4] = { { 1.f, 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f, 0.f }, { 0.f, 0.f, 1.f, 0.f }, { 0.f, 0.f, 0.f, 1.f } };
Matrix X, Y, Z, acc;

#pragma section

Matrix *mStackPtr;

/* Pushes the given matrix */
void mPush (const Matrix *m)
{
	dAssert (mStackPtr < &mStack[MAX_MATRIX_STACK], "Matrix stack overflow");
	*mStackPtr++ = *m;
}

/* Pops the top of the matrix stack into the given matrix */
void mPop (Matrix *m)
{
	mStackPtr--;
	dAssert (mStackPtr >= &mStack[0], "Matrix stack underflow");
	*m = *mStackPtr;
}

/* Resets the matrix stack */
void mResetStack (void)
{
	mStackPtr = mStack;
}

/* Multiplies two matrices (res = m1 * m2) */
void mMultiply (Matrix *res, const Matrix *m1, const Matrix *m2)
{
	NC(res); NC(m1); NC(m2);
	mLoad (m2);
	mMul (res, m1);
}

/* Premultiplies a matrix by another (res = mat * res) */
void mPreMultiply (Matrix *res, const Matrix *mat)
{
	NC(res); NC(mat);
	mLoad (res);
	mMul (res, mat);
}

/* Postmultiplies a matrix by another (res = res * mat) */
void mPostMultiply (Matrix *res, const Matrix *mat)
{
	NC(res); NC(mat);
	mLoad (mat);
	mMul (res, res);
}

/*
 * Multiply a Point3 with a matrix and returns another Point3
 * res = poi * mat
 */
void mPoint3Multiply3 (Point3 *res, const Point3 *p, const Matrix *mat)
{
	Point pt, poo;

	NC(res);NC(p);NC(mat);

	mLoad (mat);
	*(Point3 *)&poo = *p;
	poo.w = 1.f;
	mPoint (&pt, &poo);
	*res = *(Point3 *)&pt;
}
/*
 * Multiply a Point3 with a matrix43 and returns another Point3
 * res = poi * mat
 */
void mShortPoint3Multiply3 (Point3 *res, const Point3 *p, const Matrix43 *mat)
{
	Point pt, poo;

	mLoad43 (mat);
	*(Point3 *)&poo = *p;
	poo.w = 1.f;
	mPoint (&pt, &poo);
	*res = *(Point3 *)&pt;
}


/* Multiplies a Vector3 with a Matrix : res = vec * mat */
void mVector3Multiply (Vector3 *res, const Vector3 *vec, const Matrix *mat)
{
	Point pt, poo;

	NC(res);NC(vec);NC(mat);
	mLoadForVector (mat);
	*(Point3 *)&poo = *vec;
	poo.w = 1.f;
	mPoint (&pt, &poo);
	*res = *(Vector3 *)&pt;
}


/* Unitize a matrix */
#if 0
void mUnit (Matrix *m)
{
	*m = *((Matrix *)&mUnitMatrix);
	/*
	ld_ext (mUnitMatrix);
	st_ext (m->m);
	*/
}
#endif
/* Sets a matrix for translation */
void mTranslate (Matrix *m, float x, float y, float z)
{
	mUnit (m);
	/*
	ld_ext (mUnitMatrix);
	st_ext (m->m);
	*/
	m->m[3][0] = x;
	m->m[3][1] = y;
	m->m[3][2] = z;
}

/*
 * Generate a perspectivization matrix
 */
void mPerspective (Matrix *m, float FOV, float aspectRatio, float nearPlane, float farPlane)
{
	SinCosVal val;
	float hOverD;
	float zScale;

	dAssert (m != NULL, "NULL matrix passed");
	dAssert (FOV > 0, "Silly FOV");
	dAssert (aspectRatio > 0, "Silly aspect ratio");
	dAssert (nearPlane > 0, "Silly nearplane");
	dAssert (farPlane > nearPlane, "Silly farplane");

	SinCos (FOV * 0.5f, &val);
	hOverD = (val.cos / val.sin);
	zScale = farPlane / (farPlane - nearPlane);

	mUnit (m);

	m->m[0][0] = hOverD / aspectRatio;				/* Scale in X direction */
	m->m[1][1] = hOverD;							/* Scale in Y direction */
	m->m[2][2] = zScale;							/* Scale in Z (scales to unit) */
	m->m[3][2] = -nearPlane * zScale;				/* Translation so near plane is on origin */

	m->m[2][3] = 1.f;								/* Makes W equal to Zv */
	m->m[3][3] = 0.f;								/* Prevents the '1' in W from propagating */
}

/* Rotation matrix setup */

void mRotateZ (Matrix *m, float angle)
{
	SinCosVal val;
	
	mUnit (m);

	SinCos (angle, &val);

	m->m[0][0] = val.cos;
	m->m[0][1] = val.sin;
	m->m[1][0] = -val.sin;
	m->m[1][1] = val.cos;
}
void mRotateX (Matrix *m, float angle)
{
	SinCosVal val;
	
	mUnit (m);

	SinCos (angle, &val);

	m->m[1][1] = val.cos;
	m->m[1][2] = val.sin;
	m->m[2][1] = -val.sin;
	m->m[2][2] = val.cos;
}

void mRotateY (Matrix *m, float angle)
{
	SinCosVal val;
	
	mUnit (m);

	SinCos (angle, &val);

	m->m[0][0] = val.cos;
	m->m[2][0] = val.sin;
	m->m[0][2] = -val.sin;
	m->m[2][2] = val.cos;
}

void mRotateXYZ (Matrix *m, float xr, float yr, float zr)
{
	register float *store = &m->m[0][0];
	SinCosVal	x, y, z;
	SinCos (xr, &x);
	SinCos (yr, &y);
	SinCos (zr, &z);

	// First row
	*store++ = y.cos * z.cos - x.sin * y.sin * z.sin;
	*store++ = y.cos * z.sin + x.sin * y.sin * z.cos;
	*store++ = - x.cos * y.sin;
	*store++ = 0.f;

	// Second row
	*store++ = - x.cos * z.sin;
	*store++ = x.cos * z.cos;
	*store++ = x.sin;
	*store++ = 0.f;

	// Third row
	*store++ = y.sin * z.cos + x.sin * y.cos * z.sin;
	*store++ = y.sin * z.sin - x.sin * y.cos * z.cos;
	*store++ = x.cos * y.cos;
	*store++ = 0.f;

	// Last row
	*store++ = 0.f;
	*store++ = 0.f;
	*store++ = 0.f;
	*store++ = 1.f;
}

/*
 * Creates a matrix which is the same as
 * scale() * rotateXYZ() * translate()
 */
void mObjectMatrix (Matrix *m, Point3 *scale, Point3 *rotate, Point3 *translate)
{
	register float *store = &m->m[0][0], *read;
	register float s;
	SinCosVal	x, y, z;
	read = &rotate->x;
	SinCos (*read++, &x);
	SinCos (*read++, &y);
	SinCos (*read, &z);

	// Begin scale reading
	read = &scale->x;

	// First row
	s = *read++;
	*store++ = s * (y.cos * z.cos - x.sin * y.sin * z.sin);
	*store++ = s * (y.cos * z.sin + x.sin * y.sin * z.cos);
	*store++ = s * (- x.cos * y.sin);
	*store++ = 0.f;

	// Second row
	s  = *read++;
	*store++ = s * (- x.cos * z.sin);
	*store++ = s * (x.cos * z.cos);
	*store++ = s * (x.sin);
	*store++ = 0.f;

	// Third row
	s  = *read;
	*store++ = s * (y.sin * z.cos + x.sin * y.cos * z.sin);
	*store++ = s * (y.sin * z.sin - x.sin * y.cos * z.cos);
	*store++ = s * (x.cos * y.cos);
	*store++ = 0.f;

	// Last row
	read = &translate->x;
	*store++ = *read++;
	*store++ = *read++;
	*store++ = *read;
	*store = 1.f;
}

/* Sets a matrix for scale */
void mScale (Matrix *m, float x, float y, float z)
{
	NC(m);
	m->m[0][0] = x; m->m[0][1] = 0; m->m[0][2] = 0; m->m[0][3] = 0;
	m->m[1][0] = 0; m->m[1][1] = y; m->m[1][2] = 0; m->m[1][3] = 0;
	m->m[2][0] = 0; m->m[2][1] = 0; m->m[2][2] = z; m->m[2][3] = 0;
	m->m[3][0] = 0; m->m[3][1] = 0; m->m[3][2] = 0; m->m[3][3] = 1;
}

/* Pre/post multiplies a matrix with a Z rotation matrix */
void mPreRotateZ (Matrix *m, float angle)
{
	mRotateZ (&mTempMatrix, angle);
	mPreMultiply (m, &mTempMatrix);
}
void mPostRotateZ (Matrix *m, float angle)
{
	mRotateZ (&mTempMatrix, angle);
	mPostMultiply (m, &mTempMatrix);
}

/* Pre/post multiplies a matrix with a Y rotation matrix */
void mPreRotateY (Matrix *m, float angle)
{
	mRotateY (&mTempMatrix, angle);
	mPreMultiply (m, &mTempMatrix);
}
void mPostRotateY (Matrix *m, float angle)
{
	mRotateY (&mTempMatrix, angle);
	mPostMultiply (m, &mTempMatrix);
}

/* Pre/post multiplies a matrix with a X rotation matrix */
void mPreRotateX (Matrix *m, float angle)
{
	mRotateX (&mTempMatrix, angle);
	mPreMultiply (m, &mTempMatrix);
}

void mPostRotateX (Matrix *m, float angle)
{
	mRotateX (&mTempMatrix, angle);
	mPostMultiply (m, &mTempMatrix);
}

/* Pre/post multiplies a matrix with a XYZ rotation matrix */
void mPreRotateXYZ (Matrix *m, float xr, float yr, float zr)
{
	mRotateXYZ (&mTempMatrix, xr, yr, zr);
	mPreMultiply (m, &mTempMatrix);
}
void mPostRotateXYZ (Matrix *m, float xr, float yr, float zr)
{
	mRotateXYZ (&mTempMatrix, xr, yr, zr);
	mPostMultiply (m, &mTempMatrix);
}

/* Pre/post translate */
void mPreTranslate (Matrix *m, float x, float y, float z)
{
	NC(m);
	m->m[3][0] += x * m->m[0][0] + y * m->m[1][0] + z * m->m[2][0];
	m->m[3][1] += x * m->m[0][1] + y * m->m[1][1] + z * m->m[2][1];
	m->m[3][2] += x * m->m[0][2] + y * m->m[1][2] + z * m->m[2][2];
}

/* Postmultiplies a matrix with a translation matrix */
void mPostTranslate (Matrix *m, float x, float y, float z)
{
	NC(m);
	m->m[3][0] += x;
	m->m[3][1] += y;
	m->m[3][2] += z;
}

/* Pre/post scale */
void mPreScale (Matrix *m, float x, float y, float z)
{
	NC(m);
	m->m[0][0] *= x; m->m[0][1] *= x; m->m[0][2] *= x;
	m->m[1][0] *= y; m->m[1][1] *= y; m->m[1][2] *= y;
	m->m[2][0] *= z; m->m[2][1] *= z; m->m[2][2] *= z;
}

/* Postmultiplies a matrix with a scale matrix */
void mPostScale (Matrix *m, float x, float y, float z)
{
	NC(m);
	m->m[0][0] *= x; m->m[1][0] *= x; m->m[2][0] *= x; m->m[3][0] *= x;
	m->m[0][1] *= y; m->m[1][1] *= y; m->m[2][1] *= y; m->m[3][1] *= y;
	m->m[0][2] *= z; m->m[1][2] *= z; m->m[2][2] *= z; m->m[3][2] *= z;
}

/*
 * Invert a matrix which has [0 0 0 1]T in its last column
 * dest = src^-1
 */
#define ACCUMULATE if (temp >= 0.0) pos += temp; else neg += temp
#define PRECISION_LIMIT 1.0e-15

void mInvert3d (Matrix *dest, const Matrix *src)
{
    register float	det;
    register float	pos, neg, temp;
	register float s00, s01, s02, s10, s11, s12; /* just about fits */
	register float *pop;
		
	dAssert (dest != NULL, "Destination matrix NULL");
	dAssert (src != NULL, "Source matrix NULL");
	dAssert (src != dest, "Source matrix == destination");
	
	pop = &src->m[0][0];
	s00 = *pop++;
	s01 = *pop++;
	s02 = *pop++;
	pop++;
	s10 = *pop++;
	s11 = *pop++;
	s12 = *pop;

    /*
	 * Calculate the determinant of submatrix A and determine if the
	 * the matrix is singular as limited by the single precision
	 * floating-point data representation.
	 */
    pos = neg = 0.f;
    temp =  s00 * s11 * src->m[2][2];
    ACCUMULATE;
	temp =  s01 * s12 * src->m[2][0];
    ACCUMULATE;
	temp =  s02 * s10 * src->m[2][1];
    ACCUMULATE;
	temp = -s02 * s11 * src->m[2][0];
    ACCUMULATE;
	temp = -s01 * s10 * src->m[2][2];
    ACCUMULATE;
	temp = -s00 * s12 * src->m[2][1];
    ACCUMULATE;
	det = pos + neg;
	
    /* Is the submatrix A singular? */
#ifdef _DEBUG
	if (ABS(det) <= FLT_EPSILON || (ABS(det/(pos - neg)) <= PRECISION_LIMIT)) {
//		dAssert (0, "Singular matrix");
		mUnit (dest);
		return;
	}
#endif

	/*
	 * Calculate inverse(A) = adj(A) / det(A)
	 */
    det = 1.0f / det;
	
    dest->m[0][0] =  (s11		   * src->m[2][2] - s12 * src->m[2][1]) * det;
    dest->m[1][0] = -(s10		   * src->m[2][2] - s12 * src->m[2][0]) * det;
    dest->m[2][0] =  (s10		   * src->m[2][1] - s11 * src->m[2][0]) * det;
    dest->m[0][1] = -(s01		   * src->m[2][2] - src->m[0][2] * src->m[2][1]) * det;
    dest->m[1][1] =  (s00		   * src->m[2][2] - src->m[0][2] * src->m[2][0]) * det;
    dest->m[2][1] = -(s00		   * src->m[2][1] - src->m[0][1] * src->m[2][0]) * det;
    dest->m[0][2] =  (s01		   * s12 - src->m[0][2] * s11) * det;
    dest->m[1][2] = -(s00		   * s12 - src->m[0][2] * s10) * det;
    dest->m[2][2] =  (s00		   * s11 - src->m[0][1] * s10) * det;
	
    /*
	 * Calculate -C * inverse(A)
	 */
    dest->m[3][0] = -(src->m[3][0] * dest->m[0][0] + src->m[3][1] * dest->m[1][0] + src->m[3][2] * dest->m[2][0]);
    dest->m[3][1] = -(src->m[3][0] * dest->m[0][1] + src->m[3][1] * dest->m[1][1] + src->m[3][2] * dest->m[2][1]);
    dest->m[3][2] = -(src->m[3][0] * dest->m[0][2] + src->m[3][1] * dest->m[1][2] + src->m[3][2] * dest->m[2][2]);
	
	dest->m[0][3] = dest->m[1][3] = dest->m[2][3] = 0.f;
	dest->m[3][3] = 1.f;
}

void mShortInvert3d (Matrix43 *dest, const Matrix *src)
{
    register float	det;
    register float	pos, neg, temp;
	register float s00, s01, s02, s10, s11, s12; /* just about fits */
	register float *pop;
		
	dAssert (dest != NULL, "Destination matrix NULL");
	dAssert (src != NULL, "Source matrix NULL");
   //	dAssert (src != dest, "Source matrix == destination");
	
	pop = &src->m[0][0];
	s00 = *pop++;
	s01 = *pop++;
	s02 = *pop++;
	pop++;
	s10 = *pop++;
	s11 = *pop++;
	s12 = *pop;

    /*
	 * Calculate the determinant of submatrix A and determine if the
	 * the matrix is singular as limited by the single precision
	 * floating-point data representation.
	 */
    pos = neg = 0.f;
    temp =  s00 * s11 * src->m[2][2];
    ACCUMULATE;
	temp =  s01 * s12 * src->m[2][0];
    ACCUMULATE;
	temp =  s02 * s10 * src->m[2][1];
    ACCUMULATE;
	temp = -s02 * s11 * src->m[2][0];
    ACCUMULATE;
	temp = -s01 * s10 * src->m[2][2];
    ACCUMULATE;
	temp = -s00 * s12 * src->m[2][1];
    ACCUMULATE;
	det = pos + neg;
	
    /* Is the submatrix A singular? */
#ifdef _DEBUG
	if (ABS(det) <= FLT_EPSILON || (ABS(det/(pos - neg)) <= PRECISION_LIMIT)) {
//		dAssert (0, "Singular matrix");
		mShortUnit (dest);
		return;
	}
#endif

	/*
	 * Calculate inverse(A) = adj(A) / det(A)
	 */
    det = 1.0f / det;
	
    dest->m[0][0] =  (s11		   * src->m[2][2] - s12 * src->m[2][1]) * det;
    dest->m[1][0] = -(s10		   * src->m[2][2] - s12 * src->m[2][0]) * det;
    dest->m[2][0] =  (s10		   * src->m[2][1] - s11 * src->m[2][0]) * det;
    dest->m[0][1] = -(s01		   * src->m[2][2] - src->m[0][2] * src->m[2][1]) * det;
    dest->m[1][1] =  (s00		   * src->m[2][2] - src->m[0][2] * src->m[2][0]) * det;
    dest->m[2][1] = -(s00		   * src->m[2][1] - src->m[0][1] * src->m[2][0]) * det;
    dest->m[0][2] =  (s01		   * s12 - src->m[0][2] * s11) * det;
    dest->m[1][2] = -(s00		   * s12 - src->m[0][2] * s10) * det;
    dest->m[2][2] =  (s00		   * s11 - src->m[0][1] * s10) * det;
	
    /*
	 * Calculate -C * inverse(A)
	 */
    dest->m[3][0] = -(src->m[3][0] * dest->m[0][0] + src->m[3][1] * dest->m[1][0] + src->m[3][2] * dest->m[2][0]);
    dest->m[3][1] = -(src->m[3][0] * dest->m[0][1] + src->m[3][1] * dest->m[1][1] + src->m[3][2] * dest->m[2][1]);
    dest->m[3][2] = -(src->m[3][0] * dest->m[0][2] + src->m[3][1] * dest->m[1][2] + src->m[3][2] * dest->m[2][2]);
	
//	dest->m[0][3] = dest->m[1][3] = dest->m[2][3] = 0.f;
//	dest->m[3][3] = 1.f;
}


/* Generate a random unit vector (slowish) */
void v3Rand (Vector3 *vec)
{
	vec->x = frand()*2.f - 1.f;
	vec->y = frand()*2.f - 1.f;
	vec->z = frand()*2.f - 1.f;
	v3Normalise (vec);
}

/* Calculate the cross product : res = a X b */
void v3Cross (Vector3 *res, const Vector3 *a, const Vector3 *b)
{
	NC(res); NC(a); NC(b);
	dAssert (res != a, "Must be different!");
	dAssert (res != b, "Must be different!");

	res->x = (a->y * b->z - a->z * b->y);
	res->y = (a->z * b->x - a->x * b->z);
	res->z = (a->x * b->y - a->y * b->x);
}

#define MAC4(a,b, c,d, e,f, g,h)	((a)*(b) + (c)*(d) + (e)*(f) + (g)*(h))
#define MAC3(a,b, c,d, e,f)			((a)*(b) + (c)*(d) + (e)*(f))
#define MAC2(a,b, c,d)				((a)*(b) + (c)*(d))

/* Calculate the determinant of a matrix (internal use only) */
#define DET2(a,b,c,d) MAC2(a,d,-b,c)
#pragma inline(DET3)
static float DET3 (float a1, float a2, float a3,
				   float b1, float b2, float b3,
				   float c1, float c2, float c3)
{
	return MAC3 (a1, DET2 (b2, b3, c2, c3),
				-b1, DET2 (a2, a3, c2, c3),
				 c1, DET2 (a2, a3, b2, b3));
}

/* Publicly exported version of DET3 */
float mDeterminant3x3 (const Matrix *matrix)
{
	return DET3 (matrix->m[0][0], matrix->m[0][1], matrix->m[0][2],
				 matrix->m[1][0], matrix->m[1][1], matrix->m[1][2],
				 matrix->m[2][0], matrix->m[2][1], matrix->m[1][2]);
}
				   
/*
 * Return the adjoint matrix (matrix of determinants)
 * dest = adj (src)
 */
void mAdjoint (Matrix *dest, const Matrix *src)
{
	float a1, a2, a3, a4, b1, b2, b3, b4;
	float c1, c2, c3, c4, d1, d2, d3, d4;

	dAssert (dest != NULL, "NULL destination matrix passed");
	dAssert (src != NULL, "NULL source matrix passed");

	a1 = src->m[0][0]; b1 = src->m[0][1];
	c1 = src->m[0][2]; d1 = src->m[0][3];

	a2 = src->m[1][0]; b2 = src->m[1][1];
	c2 = src->m[1][2]; d2 = src->m[1][3];

	a3 = src->m[2][0]; b3 = src->m[2][1];
	c3 = src->m[2][2]; d3 = src->m[2][3];

	a4 = src->m[3][0]; b4 = src->m[3][1];
	c4 = src->m[3][2]; d4 = src->m[3][3];

	/* Transpose as well */

    dest->m[0][0] =  DET3 (b2, b3, b4, c2, c3, c4, d2, d3, d4);
    dest->m[1][0] = -DET3 (a2, a3, a4, c2, c3, c4, d2, d3, d4);
    dest->m[2][0] =  DET3 (a2, a3, a4, b2, b3, b4, d2, d3, d4);
    dest->m[3][0] = -DET3 (a2, a3, a4, b2, b3, b4, c2, c3, c4);
         
    dest->m[0][1] = -DET3 (b1, b3, b4, c1, c3, c4, d1, d3, d4);
    dest->m[1][1] =  DET3 (a1, a3, a4, c1, c3, c4, d1, d3, d4);
    dest->m[2][1] = -DET3 (a1, a3, a4, b1, b3, b4, d1, d3, d4);
    dest->m[3][1] =  DET3 (a1, a3, a4, b1, b3, b4, c1, c3, c4);
        
    dest->m[0][2] =  DET3 (b1, b2, b4, c1, c2, c4, d1, d2, d4);
    dest->m[1][2] = -DET3 (a1, a2, a4, c1, c2, c4, d1, d2, d4);
    dest->m[2][2] =  DET3 (a1, a2, a4, b1, b2, b4, d1, d2, d4);
    dest->m[3][2] = -DET3 (a1, a2, a4, b1, b2, b4, c1, c2, c4);
         
    dest->m[0][3] = -DET3 (b1, b2, b3, c1, c2, c3, d1, d2, d3);
    dest->m[1][3] =  DET3 (a1, a2, a3, c1, c2, c3, d1, d2, d3);
    dest->m[2][3] = -DET3 (a1, a2, a3, b1, b2, b3, d1, d2, d3);
    dest->m[3][3] =  DET3 (a1, a2, a3, b1, b2, b3, c1, c2, c3);

}

/*
 * Get the determinant of a matrix
 */
float mDeterminant (const Matrix *matrix)
{
	float a1, a2, a3, a4, b1, b2, b3, b4;
	float c1, c2, c3, c4, d1, d2, d3, d4;

	dAssert (matrix != NULL, "NULL matrix passed");
	
	a1 = matrix->m[0][0]; b1 = matrix->m[0][1]; 
	c1 = matrix->m[0][2]; d1 = matrix->m[0][3];
	
	a2 = matrix->m[1][0]; b2 = matrix->m[1][1]; 
	c2 = matrix->m[1][2]; d2 = matrix->m[1][3];
	
	a3 = matrix->m[2][0]; b3 = matrix->m[2][1]; 
	c3 = matrix->m[2][2]; d3 = matrix->m[2][3];
	
	a4 = matrix->m[3][0]; b4 = matrix->m[3][1]; 
	c4 = matrix->m[3][2]; d4 = matrix->m[3][3];
	
    return MAC4 (a1, DET3( b2, b3, b4, c2, c3, c4, d2, d3, d4),
				-b1, DET3( a2, a3, a4, c2, c3, c4, d2, d3, d4),
				 c1, DET3( a2, a3, a4, b2, b3, b4, d2, d3, d4),
				-d1, DET3( a2, a3, a4, b2, b3, b4, c2, c3, c4));

}

/*
 * Invert a matrix into another matrix
 * dest = src^-1
 */
void mInvert (Matrix *dest, const Matrix *src)
{
	int i;
	float determinant, rDeterminant;

	dAssert (dest != NULL, "NULL destination matrix passed");
	dAssert (src != NULL, "NULL source matrix passed");

	/* Calculate the adjoint matrix */
	mAdjoint (dest, src);

	/* Calculate the determinant of that matrix */
	determinant = mDeterminant (dest);

	/* Check for singular matrices */
	dAssert (determinant >= 0.000001f, "Attempt to invert singular matrix");
	if (determinant < 0.000001f)
		determinant = 0.000001f;

	/* Compute the reciprocal, ready for scaling the adjoint matrix */
	rDeterminant = 1.f / determinant;

	/* Scale the matrix */
	for (i = 0 ; i < 3; ++i) {
		dest->m[i][0] *= rDeterminant; 
		dest->m[i][1] *= rDeterminant;
		dest->m[i][2] *= rDeterminant; 
		dest->m[i][3] *= rDeterminant;
	}

}
/*
 * Invert a matrix into another matrix43
 * dest = src^-1
 */
void mShortInvert (Matrix43 *dest, const Matrix *src)
{
	int i;
	Matrix temp;
	float determinant, rDeterminant;

	dAssert (dest != NULL, "NULL destination matrix passed");
	dAssert (src != NULL, "NULL source matrix passed");

	/* Calculate the adjoint matrix */
	mAdjoint (&temp, src);

	/* Calculate the determinant of that matrix */
	determinant = mDeterminant (&temp);

	/* Check for singular matrices */
	dAssert (determinant >= 0.000001f, "Attempt to invert singular matrix");
	if (determinant < 0.000001f)
		determinant = 0.000001f;

	/* Compute the reciprocal, ready for scaling the adjoint matrix */
	rDeterminant = 1.f / determinant;

	/* Scale the matrix */
	for (i = 0 ;i < 3; ++i) {
		dest->m[i][0] = temp.m[i][0] * rDeterminant; 
		dest->m[i][1] = temp.m[i][1] * rDeterminant;
		dest->m[i][2] = temp.m[i][2] * rDeterminant;
	}
}


/*
 * Delta function for the strats
 * Gives an 's' shaped curve in the interval 0-1
 */
float Delta(float x)
{
	if (x < 0)
		return 0.f;
	if (x > 1)
		return 1.f;
	if (x < 0.5) {
		return (x*x*2.f);
	} else {
		x = 1.f - x;
		return 1.f - (x*x*2.f);
	}
}

/* 43 matrices */
void m43to44 (Matrix *m, Matrix43 *m2)
{
	register float *src, *dest;
	src = &m2->m[0][0];
	dest = &m->m[0][0];
	*dest++ = *src++; *dest++ = *src++; *dest++ = *src++; *dest++ = 0.f;
	*dest++ = *src++; *dest++ = *src++; *dest++ = *src++; *dest++ = 0.f;
	*dest++ = *src++; *dest++ = *src++; *dest++ = *src++; *dest++ = 0.f;
	*dest++ = *src++; *dest++ = *src++; *dest++ = *src++; *dest++ = 1.f;
}

// Floating point random number generator
#define LFSR_LENGTH 250
#define XOR_TAP 147
/* generate a floating point number uniformly distributed in [0.0f, 1.0f] */
static long lfsr[LFSR_LENGTH];
static long lfsr_orig[LFSR_LENGTH]=
{
	0x0674A2B8L,0x57E0FC4CL,0x0255DF91L,0x60E32DFAL,0x129D9C8BL,
		0x17581662L,0x40B855EBL,0x1C824025L,0x5527CDAAL,0x2DFDA7EAL,
		0x73C340C8L,0x684ADBB3L,0x5CA27CC1L,0x78513509L,0x1CB912BFL,
		0x4FD9326FL,0x113380EEL,0x5C335601L,0x74E03A18L,0x3C63ECB2L,
		0x70B6B432L,0x485BB400L,0x4B10A825L,0x51E08252L,0x724F3914L,
		0x1AE9A893L,0x2408A9A0L,0x61C8C20FL,0x0C5C4917L,0x7731D0ABL,
		0x7240E687L,0x21B9C16AL,0x46850798L,0x4E9540A1L,0x403A3842L,
		0x32043C2EL,0x2096BE46L,0x55C25156L,0x6079DC72L,0x06EDF7F4L,
		0x1C4D2AB1L,0x6F0A0D70L,0x7740A9E3L,0x55E6A032L,0x06D7F83DL,
		0x0C22B73EL,0x0147A873L,0x5409E89CL,0x76558B36L,0x1074AC3CL,
		0x735BC5B3L,0x7DC2E3A4L,0x6467000EL,0x58056335L,0x4D693007L,
		0x5759F9CCL,0x1E594382L,0x6717062EL,0x2DB51A29L,0x438B0F63L,
		0x2B85D19AL,0x69F7C58DL,0x03AF7D4CL,0x1DC1977CL,0x4E412012L,
		0x6F7A8FF8L,0x5C84C3A2L,0x04BF4857L,0x05017CA2L,0x5A324354L,
		0x6B393D45L,0x3BCCC50FL,0x2E100A7BL,0x3879F114L,0x547EC448L,
		0x0E248609L,0x361A1939L,0x36B85F50L,0x79EA86F2L,0x3AEBCAACL,
		0x55629D3AL,0x4F949E19L,0x595F8469L,0x57C83CF6L,0x2B977B0EL,
		0x502B8CB6L,0x63BC66C1L,0x4A1B9CBAL,0x32AC1179L,0x7A2581E4L,
		0x4BB9BDCEL,0x2A630D17L,0x20F89324L,0x1ECB0B48L,0x45AC72ADL,
		0x4197AE60L,0x1F9271B7L,0x22578B52L,0x3800F1CAL,0x03F1A589L,
		0x402CBC6BL,0x154FE247L,0x042103A8L,0x6C21E5B5L,0x0529727FL,
		0x5B610F0FL,0x4BDB7463L,0x4D51CAA7L,0x0F9CF52DL,0x1250D989L,
		0x4CD7D6D2L,0x1ADA4A67L,0x76FFB497L,0x41C827C8L,0x7557A80BL,
		0x42C96984L,0x15E6B2C3L,0x63C061A1L,0x4038C2C1L,0x10E548ACL,
		0x69C778A8L,0x177EC5F5L,0x246AAB73L,0x7CDFF7FDL,0x3EE490F2L,
		0x032BF1E3L,0x56D75183L,0x36A6E38FL,0x16E1466CL,0x1FE379F4L,
		0x500DD660L,0x6FBEF813L,0x2E3C8E66L,0x01B61DBAL,0x5CB1E635L,
		0x1099253DL,0x22103684L,0x74ABC91DL,0x65520E32L,0x12234973L,
		0x127CC131L,0x44E9406EL,0x00B9E99EL,0x77000C3BL,0x134B4124L,
		0x241DBE25L,0x36477A24L,0x5C7DBDCEL,0x6DB179A2L,0x60CD21C3L,
		0x50A5434EL,0x619AA644L,0x46F16F61L,0x44B76D44L,0x5FAED361L,
		0x22127AC3L,0x04FBAF6EL,0x593978BFL,0x2A5C023EL,0x5A39C87EL,
		0x6116E999L,0x4C2828F5L,0x14D75A79L,0x36568505L,0x34642B91L,
		0x1BC831FBL,0x6B52CE5AL,0x2BEB75B7L,0x3D9194E2L,0x0B8A9DF3L,
		0x635328DCL,0x48265D85L,0x68628D4FL,0x516E0A9BL,0x6AB3BEF6L,
		0x1A9F28FFL,0x25E07AA0L,0x7CE5C879L,0x5CE1B7F5L,0x35CEF42DL,
		0x74222061L,0x2B254553L,0x30B29D13L,0x436D20E3L,0x3A5B44E3L,
		0x3DBA93F6L,0x69DD7923L,0x56976E27L,0x0EA32006L,0x55810410L,
		0x69BBAD57L,0x6ACD9BF0L,0x4AAB905FL,0x61807BEDL,0x390F0740L,
		0x3454A116L,0x462EC155L,0x0B7D7A96L,0x2E21D3C1L,0x2C71541DL,
		0x77D2F074L,0x151AAC63L,0x7BD2C20DL,0x36A088D4L,0x03DBF536L,
		0x43EA1AE3L,0x4D2D8D33L,0x66808CACL,0x39C3633FL,0x040C02F5L,
		0x09110977L,0x132CF5E7L,0x2F26A79EL,0x5FDA3310L,0x250DCE5DL,
		0x24317815L,0x78DC0304L,0x6000C518L,0x74490FF8L,0x3F7A515CL,
		0x7964987CL,0x47ADCA85L,0x38122775L,0x5F27AD32L,0x35B5CA0EL,
		0x568D077FL,0x4A75C275L,0x2DCA599CL,0x53FFF9BEL,0x72808407L,
		0x6CE74293L,0x4D91A3C4L,0x64D5615EL,0x5D276199L,0x54B567EFL,
		0x3931F8D2L,0x76284002L,0x36EBC88DL,0x03D342FAL,0x6F9BA251L,
		0x364A67CBL,0x40669977L,0x51643A64L,0x186433F5L,0x0A212DB6L,
		0x4310C60BL,0x536D69EBL,0x71E46FA2L,0x3B42CA2EL,0x18C99F64L
};

static int tap=XOR_TAP;
static int last_word=0;	
float frand(void) 
{
	float retVal;
	if (++last_word>=LFSR_LENGTH) last_word=0;
	if (++tap>=LFSR_LENGTH) tap=0;
	lfsr[last_word] = lfsr[last_word] ^ lfsr[tap];
	retVal = ((float)lfsr[last_word])* (1.f / 0x7FFFFFFFL);
	dAssert (retVal >=0 && retVal <= 1, "Erk in frand() - please let MattG know!");
	return retVal;
}

float RandRange(float a, float b)
{
	return (frand() * (b - a)) + a;
}
/*
 * Reset the random number generators
 */
void ResetRandom(void)
{
	srand(1);
	memcpy (lfsr, lfsr_orig, sizeof (lfsr));
	tap = XOR_TAP;
	last_word = 0;
}

/*
 * Reseed random number generator randomly
 */
void ReseedRandom(void)
{
	SYS_RTC_DATE dateAndTime;
	Uint32 mangler;
	int i;

	syRtcGetDate (&dateAndTime);
	mangler = (dateAndTime.second<<8) ^ dateAndTime.minute ^ (dateAndTime.month<<16) ^ syTmrGetCount() ^ (currentFrame<<24);
	srand (mangler);
	for (i = 0; i < asize (lfsr); ++i)
		lfsr[i] ^= (rand()&~(1<<31));
}

void mUnit32(Matrix32 *m)
{
	m->m[0][0] = 1.f;
	m->m[0][1] = 0.f;
	m->m[1][0] = 0.f;
	m->m[1][1] = 1.f;
	m->m[2][0] = 0.f;
	m->m[2][1] = 0.f;
}
void mRotate32(Matrix32 *m, float zRot)
{
	SinCosVal val;
	SinCos (zRot, &val);
	m->m[0][0] = val.cos;
	m->m[0][1] = val.sin;
	m->m[1][0] = -val.sin;
	m->m[1][1] = val.cos;
	m->m[2][0] = 0.f;
	m->m[2][1] = 0.f;
}
void mMultiply32(Matrix32 *result, const Matrix32 *a, const Matrix32 *b)
{
	float g, h, i, j, k, l, *p;
	g = a->m[0][0] * b->m[0][0] + a->m[0][1] * b->m[1][0];
	h = a->m[0][0] * b->m[0][1] + a->m[0][1] * b->m[1][1];
	i = a->m[1][0] * b->m[0][0] + a->m[1][1] * b->m[1][0];
	j = a->m[1][0] * b->m[0][1] + a->m[1][1] * b->m[1][1];
	k = a->m[2][0] * b->m[0][0] + a->m[2][1] * b->m[1][0] + b->m[2][0];
	l = a->m[2][0] * b->m[0][1] + a->m[2][1] * b->m[1][1] + b->m[2][1];
	p = &result->m[0][0];
	*p++ = g; *p++ = h;
	*p++ = i; *p++ = j;
	*p++ = k; *p++ = l;
}
void mPostTranslate32(Matrix32 *result, float x, float y)
{
	result->m[2][0] += x;
	result->m[2][1] += y;
}
void mPreTranslate32(Matrix32 *result, float x, float y)
{
	Matrix32 temp;
	mUnit32(&temp);
	temp.m[2][0] = x;
	temp.m[2][1] = y;
	mMultiply32 (result, &temp, result);
}
void mPostRotate32(Matrix32 *result, float zRot)
{
	Matrix32 temp;
	mRotate32 (&temp, zRot);
	mMultiply32 (result, result, &temp);
}
void mPostScale32(Matrix32 *result, float xScale, float yScale)
{
	Matrix32 temp;
	result->m[0][0] *= xScale; result->m[0][1] *= yScale;
	result->m[1][0] *= xScale; result->m[1][1] *= yScale;
	result->m[2][0] *= xScale; result->m[2][1] *= yScale;
}
void mPreScale32(Matrix32 *result, float xScale, float yScale)
{
	Matrix32 temp;
	result->m[0][0] *= xScale; result->m[1][0] *= yScale;
	result->m[0][1] *= xScale; result->m[1][1] *= yScale;
}

void mPoint2Multiply(Point2 *result, const Point2 *p, const Matrix32 *mat)
{
	float x; // Prevent problems when p == result
	x = p->x * mat->m[0][0] + p->y * mat->m[1][0] + mat->m[2][0];	
	result->y = p->x * mat->m[0][1] + p->y * mat->m[1][1] + mat->m[2][1];
	result->x = x;
}

void mUnit (Matrix *m)
{
	register float *mat = &m->m[0][0];
	*mat++ = 1.f; *mat++ = 0.f; *mat++ = 0.f; *mat++ = 0.f;
	*mat++ = 0.f; *mat++ = 1.f; *mat++ = 0.f; *mat++ = 0.f;
	*mat++ = 0.f; *mat++ = 0.f; *mat++ = 1.f; *mat++ = 0.f;
	*mat++ = 0.f; *mat++ = 0.f; *mat++ = 0.f; *mat   = 1.f;
}
