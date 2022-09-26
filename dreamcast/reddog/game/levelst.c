#include "reddog.h"
#include "specfx.h"


#ifndef	LEVELST_H
#include "levelst.h"
#endif

#if ALLRESIDENT
	#include	"C:\STRATS\PCODE\LEVELSTRATS.C"
#else
	#if LEVEL1
		#include "C:\STRATS\PCODE\LEVEL1ST.C"
	#elif LEVEL2
		#include "C:\STRATS\PCODE\LEVEL2ST.C"
	#elif LEVEL3
		#include "C:\STRATS\PCODE\LEVEL3ST.C"
	#elif LEVEL4
		#include "C:\STRATS\PCODE\LEVEL4ST.C"
	#elif LEVEL5
		#include "C:\STRATS\PCODE\LEVEL5ST.C"
	#elif LEVEL6
		#include "C:\STRATS\PCODE\LEVEL6ST.C"
	#elif LEVEL7
		#include "C:\STRATS\PCODE\LEVEL7ST.C"
	#elif LEVEL8
		#include "C:\STRATS\PCODE\LEVEL8ST.C"
	#elif LEVEL9
		#include "C:\STRATS\PCODE\LEVEL9ST.C"
	#elif LEVEL10
		#include "C:\STRATS\PCODE\LEVEL10ST.C"
	#endif
#endif