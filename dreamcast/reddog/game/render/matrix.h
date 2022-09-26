/*
 * Matrix.h
 * Matt's matrices and vectors
 * This is a high-level API to the matrix math routines
 * and as such is nowhere near as optimal as hand coding the
 * matrix stuff yourself
 * A faster API is available under the mLoad(), mStore(), mMul() ...
 * >>>NEW<<<
 * All matrices need to be aligned to 8 byte boundaries - the best way
 * is to add them to the section 'Matrices' as in Matrix.c
 */

#ifndef _MATRIX_H
#define _MATRIX_H

#include <string.h>
#include <math.h>

/* Vector3 and Point3s are defined in RDTypes.h */

/*
 * The current matrix
 */
extern Matrix mCurMatrix;

/* Temporary matrix used for pre/post operations */
extern Matrix mTempMatrix;

/* Various other matrices */
extern Matrix	modelToScreen;
extern Matrix	modelToView;
extern Matrix	worldToModel;
extern Matrix	viewToModel;

/* Temporary point */
extern Point3 pTempPoint;

/************************
 * New, lower-level API *
 ************************/

#if 0
/*
 * Load a matrix into XMTRX
 */
#define mLoad(mat) { ld_ext((void *)(mat)); }

/* 
 * Load a matrix for vectors ie with no translation
 * XXX needs to be inline asm
 */
void mLoadForVector (Matrix *m);

/*
 * Load a matrix XY scaled
 */
void mLoadWithXYScale (Matrix *m, float xScale, float yScale);

/*
 * Store a matrix from XMTRX
 */
#define mStore(mat) { st_ext((void *)(mat)); }

/*
 * Multiply a matrix with XMTRX, the result is stored out
 * Both matrix, and resultant must be 8byte aligned, and may be the same
 * out = in * XMTRX
 */
/*#define mMul(out, in) { mtrx4mul ((void *)(in), (void *)(out)); }*/
void mMul (Matrix *out, Matrix *in);
#else

void mLoad (Matrix *m);
void mLoad43 (Matrix43 *m);
void mLoadWithXYScale (Matrix *m, float xScale, float yScale);
void mLoadWithXYScaleMirrored (Matrix *m, float xScale, float yScale);
void mStore (Matrix *m);
void mLoadForVector (Matrix *m);
void mMul (Matrix *out, Matrix *in);
#endif

/*
 * Multiply a point with XMTRX, the result is stored out
 * All must be aligned
 * out = in * XMTRX
 */
#define mPoint(out,in) ftrv((float *)(in), (float *)(out))
void mPointMirrored (Point *out, Point *in);

// Multiply three floats with an XMTRX, result is a Point
void mPointXYZ (Point *out, float x, float y, float z);

/*
 * Inverse square-root
 */
#define isqrt(i) _builtin_fsrra(i)

/*
 * Unitize a matrix
 */
void mUnit (Matrix *m);

/*
 * Unitize a matrix43
 */
#pragma inline(mShortUnit)
static void mShortUnit (Matrix43 *m)
{
	register float *mat = &m->m[0][0];
	*mat++ = 1.f; *mat++ = 0.f; *mat++ = 0.f; 
	*mat++ = 0.f; *mat++ = 1.f; *mat++ = 0.f; 
	*mat++ = 0.f; *mat++ = 0.f; *mat++ = 1.f; 
	*mat++ = 0.f; *mat++ = 0.f; *mat++ = 0.f; 
}


/*
 * Floating point inner product
 */
#ifdef INLINEASM_TEST
#pragma inline_asm(FIPR)
static float FIPR (float a, float b, float c, float d, float e, float f, float g, float h)
{
	FIPR	FV4, FV8
	FMOV	FR11, FR0
}
#else
extern float FIPR (float a, float b, float c, float d, float e, float f, float g, float h);
#endif

#define PolyCheck(x,y,z,A,B,C,D) FIPR(x,y,z,1.f,A,B,C,D)

/*
 * Multiply a point3 with XMTRX, the result is stored out
 */
void mPoint3 (Point3 *out, Point3 *in);

/*
 * Return the adjoint matrix (matrix of determinants)
 * dest = adj (src)
 */
void mAdjoint (Matrix *dest, const Matrix *src);

/*
 * Get the determinant of a matrix
 */
float mDeterminant (const Matrix *matrix);
/*
 * Get the determinant of the 3x3 upper left submatrix of a matrix
 */
float mDeterminant3x3 (const Matrix *matrix);

/*
 * Invert a matrix into another matrix
 * dest = src^-1
 */
void mInvert (Matrix *dest, const Matrix *src);
/*
 * Invert a matrix into another matrix 43
 * dest = src^-1
 */
void mShortInvert (Matrix43 *dest, const Matrix *src);
/*
 * Invert a matrix which has [0 0 0 1]T in its last column
 * dest = src^-1
 */
void mInvert3d (Matrix *dest, const Matrix *src);

/* As above but result is Matrix43 */
void mShortInvert3d (Matrix43 *dest, const Matrix *src);


/*
 * Inverts the 3x3 upper left submatrix of a matrix
 * dest = src[0-3][0-3]^-1
 */
void mInvert3x3 (Matrix *dest, const Matrix *src);

/* Generates a perspective matrix */
void mPerspective (Matrix *m, float FOV, float aspectRatio, float nearPlane, float farPlane);

/* 43 matrices */
void m43to44 (Matrix *m, Matrix43 *m2);

/*
 * The matrix stack
 * The stack is an empty, ascending stack
 */
#define MAX_MATRIX_STACK	32
extern Matrix mStack[MAX_MATRIX_STACK];
extern Matrix *mStackPtr;

/* Pushes the given matrix */
void mPush (const Matrix *m);

/* Pops the top of the matrix stack into the given matrix */
void mPop (Matrix *m);

/* Resets the matrix stack */
void mResetStack (void);

/*
 * Arithmetic operations
 */

/* Multiplies two matrices (res = m1 * m2) */
void mMultiply (Matrix *res, const Matrix *m1, const Matrix *m2);
/* Multiplies two matrices alignedly (res = m1 * m2) */
void mMultiplyAligned (Matrix *res, const Matrix *m1, const Matrix *m2);

/* Premultiplies a matrix by another (res = mat * res) */
void mPreMultiply (Matrix *res, const Matrix *mat);

/* Postmultiplies a matrix by another (res = res * mat) */
void mPostMultiply (Matrix *res, const Matrix *mat);

/* Multiplies a Vector3 with a Matrix : res = vec * mat */
void mVector3Multiply (Vector3 *res, const Vector3 *vec, const Matrix *mat);

/*
 * Multiply a Point3 with a matrix and returns another Point3
 * res = poi * mat
 */
void mPoint3Multiply3 (Point3 *res, const Point3 *p, const Matrix *mat);
/*
 * Multiply a Point3 with a matrix43 and returns another Point3
 * res = poi * mat
 */
void mShortPoint3Multiply3 (Point3 *res, const Point3 *p, const Matrix43 *mat);


/*
 * Get the sin and cos together
 */
typedef struct { float sin, cos; } SinCosVal;
void SinCos (float angle, SinCosVal *val);

/*
 * Matrix initialisation
 */


/* Sets a matrix for translation */
void mTranslate (Matrix *m, float x, float y, float z);

/* Sets a matrix for XYZ rotation (actually Y X Z rotation) */
void mRotateXYZ (Matrix *m, float xr, float yr, float zr);

/* Sets a matrix for Z rotation */
void mRotateZ (Matrix *m, float angle);

/* Sets a matrix for Y rotation */
void mRotateY (Matrix *m, float angle);

/* Sets a matrix for X rotation */
void mRotateX (Matrix *m, float angle);

/* Normalise a vector 
 * New: Returns the inverse squared length of the vector 
 */
float v3Normalise (Vector3 *normal);

/* Negate a vector */
#pragma inline(v3Neg)
static void v3Neg (Vector3 *v)
{
	v->x = -v->x;
	v->y = -v->y;
	v->z = -v->z;
}

/* Returns length of vector */
#define v3Length(v) ((float) sqrt ((v)->x*(v)->x + (v)->y*(v)->y + (v)->z * (v)->z))

/* Returns length squared of vector */
#define v3SqrLength(v) ((v)->x*(v)->x + (v)->y*(v)->y + (v)->z * (v)->z)

/* Return the square distance between two point3s */
#define pSqrDist(a,b) ((SQR((a)->x - (b)->x) + SQR((a)->y - (b)->y) + SQR((a)->z - (b)->z)))

/* Return the 2D distance between two points */
#define pDistXY(a,b) ((float) (sqrt(SQR((a)->x - (b)->x) + SQR((a)->y - (b)->y))))

/* Return the distance between two points */
#define pDist(a,b) (float) ((sqrt(SQR((a)->x - (b)->x) + SQR((a)->y - (b)->y) + SQR((a)->z - (b)->z))))

/* Sets a matrix for scale */
void mScale (Matrix *m, float x, float y, float z);

/* Pre/post multiplies a matrix with a Z rotation matrix */
void mPreRotateZ (Matrix *m, float angle);
void mPostRotateZ (Matrix *m, float angle);

/* Pre/post multiplies a matrix with a Y rotation matrix */
void mPreRotateY (Matrix *m, float angle);
void mPostRotateY (Matrix *m, float angle);

/* Pre/post multiplies a matrix with a X rotation matrix */
void mPreRotateX (Matrix *m, float angle);
void mPostRotateX (Matrix *m, float angle);

/* Pre/post multiplies a matrix with a XYZ rotation matrix */
void mPreRotateXYZ (Matrix *m, float xr, float yr, float zr);
void mPostRotateXYZ (Matrix *m, float xr, float yr, float zr);

/* Premultiplies a matrix with a translation matrix */
void mPreTranslate (Matrix *m, float x, float y, float z);

/* Postmultiplies a matrix with a translation matrix */
void mPostTranslate (Matrix *m, float x, float y, float z);

/* Premultiplies a matrix with a scale matrix */
void mPreScale (Matrix *m, float x, float y, float z);

/* Postmultiplies a matrix with a scale matrix */
void mPostScale (Matrix *m, float x, float y, float z);

/* Calculate the dot product between two vectors */
float v3Dot (const Vector3 *a, const Vector3 *b);

/* Subtract two vectors : res = a - b */
#pragma inline(v3Sub)
static void v3Sub (Vector3 *Res, const Vector3 *A, const Vector3 *B)
{
	register float *res = (float *)Res;
	register const float *a = (const float *)A, *b = (const float *)B;
	*res++ = *a++ - *b++;
	*res++ = *a++ - *b++;
	*res++ = *a++ - *b++;
}
/* a = a - b */
#pragma inline (v3Dec)
static void v3Dec (Vector3 *A, const Vector3 *B)
{
	register float *a = (float *)A;
	register const float *b = (const float *)B;
	*a++ -= *b++;
	*a++ -= *b++;
	*a++ -= *b++;
}

/* Add two vectors : res = a + b */
#pragma inline(v3Add)
static void v3Add (Vector3 *Res, const Vector3 *A, const Vector3 *B)
{
	register float *res = (float *)Res;
	register const float *a = (const float *)A, *b = (const float *)B;
	*res++ = *a++ + *b++;
	*res++ = *a++ + *b++;
	*res++ = *a++ + *b++;
}
/* a = a + b */
#pragma inline (v3Inc)
static void v3Inc (Vector3 *A, const Vector3 *B)
{
	register float *a = (float *)A;
	register const float *b = (const float *)B;
	*a++ += *b++;
	*a++ += *b++;
	*a++ += *b++;
}

/* Scale a vector by another res = a * K */
#pragma inline (v3Scale)
static void v3Scale (Vector3 *Res, const Vector3 *A, float k)
{
	register float *res = (float *)Res;
	register const float *a = (const float *)A;
	*res++ = *a++ * k;
	*res++ = *a++ * k;
	*res++ = *a++ * k;
}

/* Scale a vector by another vector res = a * b */
#pragma inline (v3Mul)
static void v3Mul (Vector3 *Res, const Vector3 *A, const Vector3 *B)
{
	register float *res = (float *)Res;
	register const float *a = (const float *)A, *b = (const float *)B;
	*res++ = *a++ * *b++;
	*res++ = *a++ * *b++;
	*res++ = *a++ * *b++;
}

/*
 * Multiply a Point3 with a matrix3
 */
void mPoint3Apply3(Point3 *p, const Matrix *mat);
#define mVector3Apply(p, mat) mPoint3Apply3 (p, mat)
void mPoint3Apply(Point3 *p, const Matrix *mat);

/* Generate a random unit vector (slowish) */
void v3Rand (Vector3 *vec);

/* Calculate the cross product : res = a X b */
void v3Cross (Vector3 *res, const Vector3 *a, const Vector3 *b);

// Convert a packed 16bit float to a float
#pragma inline (Uint16ToFloat)
static float Uint16ToFloat (Uint16 i)
{
	union {
		Uint32 integer;
		float floatingPt;

	} a;
	a.integer = i<<16;
	return a.floatingPt;
}

// Floating point random number in [0,1]
float frand(void);
//#define RandRange(a,b) ((frand() *((b)-(a))) + (a))

/*
 * An offshoot of the bumpmapper - a fast atan2 function
 * which returns values in the range [-1, 1]
 */
float fatan2 (float xx, float yy);

/*
 * Creates a matrix which is the same as
 * scale() * rotateXYZ() * translate()
 */
void mObjectMatrix (Matrix *m, Point3 *scale, Point3 *rotate, Point3 *translate);

/*
 * Reseed random number generator randomly
 */
void ReseedRandom(void);

/*
 * 2d matrix maths routines
 */
void mUnit32(Matrix32 *m);
void mRotate32(Matrix32 *m, float zRot);
void mMultiply32(Matrix32 *result, const Matrix32 *a, const Matrix32 *b);
void mPostTranslate32(Matrix32 *result, float x, float y);
void mPostRotate32(Matrix32 *result, float zRot);
void mPostScale32(Matrix32 *result, float xScale, float yScale);
void mPreScale32(Matrix32 *result, float xScale, float yScale);
void mPreTranslate32(Matrix32 *result, float x, float y);
void mPoint2Multiply(Point2 *result, const Point2 *p, const Matrix32 *mat);

#endif
