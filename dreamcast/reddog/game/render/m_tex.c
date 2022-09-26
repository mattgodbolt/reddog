/*
 * Model-type textured rasterisers
 */

#include "..\RedDog.h"
#include "Internal.h"
#include "Rasterisers.h"

#pragma section RASTERISERS

#if 0
#define REND_NAME texturedModelRasteriser
#define TEXTURED
#include "ModelRend.h"
#else
void texturedModelRasteriser (ModelContext *context, Uint32 num)
{
	extern PKMDWORD texturedModelRasteriserASM (ModelContext *context, Uint32 num, PKMDWORD pkm);
	kmxxGetCurrentPtr (&vertBuf);
	kmxxStartVertexStrip (&vertBuf);
	pkmCurrentPtr = texturedModelRasteriserASM (context, num, pkmCurrentPtr);
	kmxxReleaseCurrentPtr (&vertBuf);
}
#endif

#define REND_NAME texturedModelRasteriserClipped
#define TEXTURED
#define CLIPPED
#include "ModelRend.h"

/*
 * Generic, catch-all rasterisers
 */

#define TEXTURED
#define GENERIC
#define REND_NAME texturedGenRend
#include "ModelRend.h"

#define TEXTURED
#define GENERIC
#define CLIPPED
#define REND_NAME texturedGenRendClipped
#include "ModelRend.h"
