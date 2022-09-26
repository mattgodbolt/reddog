/*
 * RedDog.h
 * All source files in the Red Dog project should include
 * this header, as configuration parameters are stored within
 */

#ifndef _REDDOG_H
#define _REDDOG_H

////////////////////////////////////////////////////////////////////
// Version! Edit in FEBase.cpp!!
////////////////////////////////////////////////////////////////////
extern const char *REDDOG_VERSION;

/*
 * ANSI includes
 */
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <machine.h>

/*
 * Ensure strict unions in Kamui headers, and include them
 */
#define _STRICT_UNION_
#define fread (void)
#define fopen(a, b) (FILE *)NULL
#define fclose (void)
#include <shinobi.h>
#include <kamui.h>
#define	_KM_USE_VERTEX_MACRO_L4_
#include <kamuix.h>

/* A couple of fast memory copying routines */
extern void AlignedCopy8(void *dest, const void *src, int numBytesMultipleOf32);
extern void *AlignedCopy8SQ(void *dest, const void *src, int numBytesMultipleOf32); // returns dest+
extern void *AlignedCopy8SQWithPref(void *dest, const void *src, int numBytesMultipleOf32); // returns dest+

// Fix the bug in kamuix.h
#undef kmxxSetVertex_7
#define kmxxSetVertex_7(pcw, x, y, invw, fu, fv, bi, oi)						\
		*pkmCurrentPtr++ = pcw;													\
		*(PKMFLOAT)pkmCurrentPtr++ = x;													\
		*(PKMFLOAT)pkmCurrentPtr++ = y;													\
		*(PKMFLOAT)pkmCurrentPtr++ = invw;												\
		*(PKMFLOAT)pkmCurrentPtr++ = fu;													\
		*(PKMFLOAT)pkmCurrentPtr++ = fv;													\
		*(PKMFLOAT)pkmCurrentPtr++ = bi;												\
		*(PKMFLOAT)pkmCurrentPtr   = oi;												\
		prefetch((void *)pkmCurrentPtr);										\
		pkmCurrentPtr++;

// We know the addresses are aligned
#undef kmxxStartVertexStrip
#define kmxxStartVertexStrip(desc) KMSTATUS_SUCCESS;{										\
	register KMDWORD StructSize = ((PKMCURRENTLISTSTATE)(desc)->pCurrentListState)->GlobalParamSize; \
	AlignedCopy8(pkmCurrentPtr,	(PKMDWORD)((desc)->pGlobalParam), StructSize);		\
	prefetch((void *)pkmCurrentPtr);														\
	if(StructSize == 0x40) prefetch((void *)(pkmCurrentPtr + 8));							\
	pkmCurrentPtr += (StructSize >> 2);														\
}

// Debug polygon overflow checks
#ifdef _DEBUG
#undef kmxxReleaseCurrentPtr
#define kmxxReleaseCurrentPtr(desc)	\
	*pkmCurrentListPtr = ((KMDWORD)(*pkmCurrentListPtr) & 0xFC000000) + ((KMDWORD)pkmCurrentPtr & 0x0FFFFFFF); \
	if (((PKMCURRENTLISTSTATE)(desc)->pCurrentListState)->ListType != 4) \
		dAssert (*pkmCurrentListPtr < (KMDWORD)((&(desc)->ppBuffer[0]->pOpaquePolygonBuffer)[((PKMCURRENTLISTSTATE)(desc)->pCurrentListState)->ListType + 1]), "Poly overflow"); \
}

#endif

/*
 * This include is specific to the person building the executable
 * You must have your own local copy of this file - it is *not*
 * under source control
 */
#include "LocalDefs.h"

/* Defaults: */
#ifndef ANTIALIAS
#define ANTIALIAS 0
#endif
#ifndef COLLISION
#define COLLISION 1
#endif
#ifdef FRAMES_PER_SECOND
#undef FRAMES_PER_SECOND
#endif
#define FRAMES_PER_SECOND (PAL?25:30)
#ifndef PROFILE_BARS
#define PROFILE_BARS 0
#endif
#ifndef SHOW_FPS
#define SHOW_FPS 0
#endif
#ifndef NEWCOLL
#define NEWCOLL 1
#endif
#ifndef SHOWSPEED
#define SHOWSPEED 0
#endif
#ifndef SHOWPOLYTYPE
#define SHOWPOLYTYPE 0
#endif
#ifndef DRAWNET
#define DRAWNET 0
#endif
#ifndef DRAWSPLINE
#define DRAWSPLINE 0
#endif
#ifndef DLOG
#define DLOG 0
#endif
#ifndef COUNT_GEOMETRY
#define COUNT_GEOMETRY 0
#endif
#ifndef DRAW_DEBUG
#define DRAW_DEBUG 0
#endif
#ifndef BENCHMARK
#define BENCHMARK 0
#endif
#ifndef NEW_TITLE_PAGE
#define NEW_TITLE_PAGE 0
#endif
#ifndef MUSIC
#define MUSIC 0
#endif
#ifndef GINSU
#define GINSU 0
#endif
#ifndef AUDIO64
#define AUDIO64 1
#endif
#ifndef GDDA_MUSIC
#define GDDA_MUSIC 0
#endif
#ifndef GODMODE
#define GODMODE 0
#endif
#ifndef GODMODEMACHINE
#define GODMODEMACHINE 0
#endif
#ifndef NEW_FRONT_END
#define	NEW_FRONT_END 1
#endif

#define FE_TEXRAM (4*1024*1024+256*1024)

/*
 * Useful macros
 */
#include "Macros.h"

/*
 * Debugging macros
 */
#ifndef _RDDEBUG_H
#include "RDDebug.h"
#endif

/*
 * All the includes we need
 */
#include "RDTypes.h"
#include "Memory.h"
#include "Stream.h"
#include "Render\Animate.h"
#include "Render\Render.h"
#include "Render\lighting.h"
#include "sndutls.h"
#include "Strat.h"

/* Global variables */
extern Bool		initialised;
extern bool GinsuAutoDemo;
extern Uint32	currentFrame;
extern int		PauseMode;
extern Bool		PAL;
#ifdef _PAL
extern Uint32 PalMode;
#endif

// Some new magicness
void CrashOut (char *message, Uint32 col1, Uint32 col2);

// Number of frames we've been inactive; after 300 seconds we kick the screensaver in
extern int InactiveFrames;

// Mo' Debug madness
extern bool ValidAddress(const void *addr);
#ifdef _DEBUG
#define ASSERT_VALID(f) dAssert(ValidAddress(f), "Invalid memory address '" #f "'")
#else
#define ASSERT_VALID(f) (void)0
#endif

#endif
