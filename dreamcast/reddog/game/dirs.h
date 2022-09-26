/*
 * Dirs.h
 * Places on the network where various files reside
 */

#ifndef _DIRS_H
#define _DIRS_H

#define WAD 1			/*FOR WAD MAP OUTPUT */
/* The root directory on the network (with trailing backslash */
#if 0 /*LOCAL_COPY*/
/* Local stuff resides from the .EXE downwards */
#define REDDOG_ROOT
#else
#define REDDOG_ROOT				"P:\\GAME\\"
#endif

/* Where Red Dog Objects are kept */
#define	REDDOG_OBJECTS			REDDOG_ROOT "OBJECTS\\"
#define	NEW_REDDOG_OBJECTS		REDDOG_ROOT "NEWOBJECTS\\"
/* Where landscapes are kept */
#define REDDOG_LANDSCAPES		REDDOG_ROOT "SCAPES\\"
/* Where texture maps are kept */
#define REDDOG_TEXMAPS			REDDOG_ROOT "TEXMAPS\\"
/* Where the original textures are kept */
#define REDDOG_ORIG_TEXMAPS		"P:\\TEXTURES\\"
#define REDDOG_ORIG_TEXMAPS_2	"\\\\SANDY\\REDDOG\\TEXTURES\\"

#endif
