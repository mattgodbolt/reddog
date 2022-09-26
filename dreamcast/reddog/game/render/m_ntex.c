/*
 * Model-type textured rasterisers
 */

#include "..\RedDog.h"
#include "Internal.h"
#include "Rasterisers.h"

#pragma section RASTERISERS

#if 0
#define REND_NAME nontexturedModelRasteriser
#include "ModelRend.h"
#else
void nontexturedModelRasteriser (ModelContext *context, Uint32 num)
{
	extern PKMDWORD nontexturedModelRasteriserASM (ModelContext *context, Uint32 num, PKMDWORD pkm);
	kmxxGetCurrentPtr (&vertBuf);
	kmxxStartVertexStrip (&vertBuf);
	pkmCurrentPtr = nontexturedModelRasteriserASM (context, num, pkmCurrentPtr);
	kmxxReleaseCurrentPtr (&vertBuf);
}
#endif

#define REND_NAME nontexturedModelRasteriserClipped
#define CLIPPED
#include "ModelRend.h"

/*
 * Generic, catch-all rasterisers
 */

#define GENERIC
#define REND_NAME nontexturedGenRend
#include "ModelRend.h"

#define GENERIC
#define CLIPPED
#define REND_NAME nontexturedGenRendClipped
#include "ModelRend.h"

