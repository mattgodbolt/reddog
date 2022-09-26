/*#if UTD */
/*#include "C:\STRATS\HEADERS\LEVEL1ST.H" */
/*#endif */

/*#include "levdefs.h" */
#include "localdefs.h"
#if ALLRESIDENT

	#include	"C:\STRATS\HEADERS\LEVELSTRATS.H"
#else

	#if LEVEL1
		#include "C:\STRATS\HEADERS\LEVEL1ST.H"
	#elif LEVEL2
		#include "C:\STRATS\HEADERS\LEVEL2ST.H"
	#elif LEVEL3
		#include "C:\STRATS\HEADERS\LEVEL3ST.H"
	#elif LEVEL4
		#include "C:\STRATS\HEADERS\LEVEL4ST.H"
	#elif LEVEL5
		#include "C:\STRATS\HEADERS\LEVEL5ST.H"
	#elif LEVEL6
		#include "C:\STRATS\HEADERS\LEVEL6ST.H"
	#elif LEVEL7
		#include "C:\STRATS\HEADERS\LEVEL7ST.H"
	#elif LEVEL8
		#include "C:\STRATS\HEADERS\LEVEL8ST.H"
	#elif LEVEL9
		#include "C:\STRATS\HEADERS\LEVEL9ST.H"
	#elif LEVEL10
		#include "C:\STRATS\HEADERS\LEVEL10ST.H"
	#endif
#endif