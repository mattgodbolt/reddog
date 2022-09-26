/*
 * Bumpmap.c
 * Bump mapping support routines
 */

#include "..\RedDog.h"
#include "Internal.h"

/*
 * A bumpmap vector (polar coordinate)
 */
typedef struct
{
	float s;
	float cosT, sinT;
} BMVect;

/*
 * Octant table : 
 *		 \3	|1 /
 *		  \	| /
 *		2  \|/	0
 *      ---------
 *		6  /|\  4
 *		  /	| \	 
 *		 /7 | 5\
 */
const float Octant[8] = 
{
	 0.00f,  0.25f,  0.50f, -0.25f,
	 1.00f, -0.75f, -0.50f,  0.75f
};


/*
 * Take a vector v, and return
 * the angle s in radians, and sin and cosine of t
 * where s and t are the polar coordinate representation
 * of the vector v
 */
#define BM_EPSILON (0.00001f)
static void bmVectorToBump (BMVect *out, float xx, float yy, float zz)
{
	register float x = xx, y = yy, z = zz, s;
	float fxyMin, fxyMax, rHyp, xySq, xyzSq, adj;
	int octant;
	float nonRegFP;

	/*
	 * First port of call - calculate 's'
	 */

	/* 
	 * Check for a near-unit-Z vector 
	 * ie where x and z are nearly nothing
	 * The least significant bit of the octant is set here, too
	 * (least sig bit == which of the diagonals x/y lies in)
	 */
	if ((fabs(x) + fabs(y)) < BM_EPSILON) {
		octant = 0;
		fxyMin = 0.f;
		fxyMax = 1.f;
	} else if (fabs(x) > fabs(y)) {
		octant = 0;
		fxyMin = y;
		fxyMax = x;
	} else {
		octant = 1;
		fxyMin = x;
		fxyMax = y;
	}

	/*
	 * Negative 'z' values can't be bumpmapped - I clamp them to a low value
	 */
	z = MAX (0.0001f, z);

	/*
	 * Actually do the divide to get tan(s)
	 */
	s = fxyMin / fxyMax;

	/*
	 * Calculate the rest of the octant
	 * x>y			= bit 0
	 * sign of X	= bit 1
	 * sign of Y	= bit 2
	 */
	nonRegFP = x;
	octant |= ((*(Uint32 *)&nonRegFP) & (1<<31)) >> 30;
	nonRegFP = y;
	octant |= ((*(Uint32 *)&nonRegFP) & (1<<31)) >> 29;

	/*
	 * Calculate sinT and cosT
	 * sin(T) = opp/hyp = y / sqrt(x*x+y*y+z*z)
	 * cos(T) = adj/hyp = sqrt(x*x+z*z) / sqrt(x*x+y*y+z*z)
	 */
	xySq = SQR(x) + SQR(y);
	xyzSq = xySq + SQR(z);
	rHyp = isqrt (xyzSq);
	adj = sqrt(xySq);

	/*
	 * Now to do one iteration of a quadratic approximation
	 * to atan(s)
	 */
	s = fabs(s);
#define MAGIC_CONSTANT_A ((-11.562812f) * (1.f/256.f))
#define MAGIC_CONSTANT_B (( 43.562812f) * (1.f/256.f))
	s = (MAGIC_CONSTANT_A * s + MAGIC_CONSTANT_B) * s;

	/*
	 * Write out the data
	 */
	out->sinT = z * rHyp;
	out->cosT = adj * rHyp;
	/*
	 * Account for the octant information
	 */
	out->s = fabs (s - Octant[octant]);
}

/*
 * An offshoot of the bumpmapper - a fast atan2 function
 * which returns values in the range [-0.5, 0.5]
 */
float __fatan2 (float xx, float yy)
{
	register float x = yy, y = xx, ocOffset;
	register float tan;
	register int octant;
	float nonRegFP;

	/* 
	 * Check for the case where x and y are nearly nothing
	 * The least significant bit of the octant is set here, too
	 * (least sig bit == which of the diagonals x/y lies in)
	 */
	if (fabs(y) > fabs(x)) {
		octant = 1;
		tan = x / y;
	} else {
		octant = 0;
		tan = y / x;
	}

	/*
	 * Calculate the rest of the octant
	 * x>y			= bit 0
	 * sign of X	= bit 1
	 * sign of Y	= bit 2
	 */
	nonRegFP = x;
	octant |= ((*(Uint32 *)&nonRegFP)>>30) & 2;
	nonRegFP = y;
	octant |= ((*(Uint32 *)&nonRegFP)>>29) & 4;
	ocOffset = Octant[octant];

	/*
	 * Now to do one iteration of a quadratic approximation
	 * to atan(tan)
	 */
	tan = fabs(tan);
	tan = (MAGIC_CONSTANT_A * tan + MAGIC_CONSTANT_B) * tan;

	// Take into acount the octant information
	return fabs (tan - ocOffset);
}

/*
 * Called to bump-map a buffer
 * Fills in the LightingValue array with the specular values required for bump-mapping
 */
void bmMapBuffer (Model *model, LightingValue *output, Vector3 *lightDirInModelSpace)
{
	int nVerts = model->nVerts;
	float *normals = (float *)model->vertexNormal;
	float *info = (float *)model->bump;
	float bumpX, bumpY, bumpZ;
	float x, y, z;
	float lX, lY, lZ;
	float angle;
	BMVect vect;

	dAssert (info != NULL, "No bump map info in model!");

	lX = lightDirInModelSpace->x;
	lY = lightDirInModelSpace->y;
	lZ = lightDirInModelSpace->z;

	do {
		/*
		 * Find the amount along the X axis
		 * info points at the X axis vector
		 */
		x = *info++;
		y = *info++;
		z = *info++;
		bumpX = FIPR (x, y, z, 0, lX, lY, lZ, 0);

		/*
		 * Y axis
		 */
		x = *info++;
		y = *info++;
		z = *info++;
		bumpY = FIPR (x, y, z, 0, lX, lY, lZ, 0);

		/*
		 * Z axis (normal)
		 */
		x = *normals++;
		y = *normals++;
		z = *normals++;
		bumpZ = FIPR (x, y, z, 0, lX, lY, lZ, 0);

		/*
		 * Now to get the polar coordinates
		 */
		bmVectorToBump (&vect, bumpX, bumpY, bumpZ);

		angle = vect.s + *info++;

		output->a = 0.5f;				/* K1 */
		if (angle > 1.f)
			angle -= 1.f;
		output->r = 0.0f;				/* K2 */
		output->g = vect.cosT;			/* K3 */
		output->b = angle;				/* S (+ ang offset) */

		output++;

	} while (--nVerts);
}
