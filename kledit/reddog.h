// Include the red dog includes
#ifndef _RD_MAX_H
#define _RD_MAX_H

#define MAX_INCLUDED

namespace RedDog
{
	extern "C" {
typedef void Stream, StratLight;
typedef void *Allocator, *Deallocator;
typedef unsigned char Uint8;
typedef unsigned short Uint16;
typedef unsigned long Uint32;
typedef void KMSURFACEDESC;
#define MIN(a,b) ((a) < (b)?(a):(b))
#define MAX(a,b) ((a) > (b)?(a):(b))

#include "..\Game\Localdefs.h"
#include "..\Game\RDTypes.h"
#include "..\Game\Macros.h"
#include "..\Game\Render\Material.h"
#include "..\Game\Render\Model.h"
#include "..\Game\Render\Animate.h"
#include "..\Game\Render\Object.h"
#include "..\Game\Render\Map.h"
#include "..\Game\Dirs.h"
#include "..\Game\Coll.h"
#include "..\Game\Render\Shadow.h"
	}
};

#endif