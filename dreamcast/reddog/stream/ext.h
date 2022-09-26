
#include "..\game\localdefs.h"
#include <machine.h>
#include <shinobi.h>
#include <kamui.h>
#include <kamuix.h>

//FROM MEMORY.H
typedef void *Allocator (void *, size_t);
typedef void *Deallocator (void *, void *);

#include "..\Game\DIRS.h"
#include "..\Game\RDTypes.h"
#include "..\Game\Stream.h"
#include "..\Game\Macros.h"
#include "..\Game\Render\Animate.h"
#include "..\Game\Render\Material.h"
#include "..\Game\Render\Model.h"
#include "..\Game\Render\Object.h"
#include "structs.h"

#include "..\game\input.h"
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <math.h>
#include <dos.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>
#include <time.h>
#include <sys\stat.h>

#ifndef	FILE_H
#include "file.h"
#endif

#ifndef	LEX_H
#include "lex.ext"
#endif

//#ifndef	STREAM_H
//#include "stream.h"
//#endif

//#ifndef	LIB3D_H
//#include "lib3d.h"
//#endif

#ifndef	KEYBOARD_H
#include "keyboard.h"
#endif

#ifndef	DEFS_H
#include "defs.h"
#endif

//#ifndef	TIMER_H
//#include "timer.h"
//#endif

//#ifndef	STRAT_H
//#include "strat.h"
//#endif

