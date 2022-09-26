#ifndef FILE_H
#define FILE_H
#define MESHDIR "C:\\STREAM\\OBJECTS\\"

#if SH4
	#if ARTIST
		#define STRATDIR 	"P:\\DREAMCAST\\REDDOG\\STRATS\\"
		#define STRATMAPDIR 	"D:\\DREAMCAST\\"
		#define	GAMEDIR		"P:\\DREAMCAST\\REDDOG\\"
	#else
		#if GODMODEMACHINE
			#define STRATDIR 	"D:\\DREAMCAST\\REDDOG\\GAME\\STRATS\\"
			#define	GAMEDIR		"D:\\DREAMCAST\\REDDOG\\GAME\\"
		#else   
			#define STRATDIR 	"C:\\DREAMCAST\\REDDOG\\GAME\\STRATS\\"
			#define	GAMEDIR		"C:\\DREAMCAST\\REDDOG\\GAME\\"
		#endif	
	#endif
#else
	#define STRATDIR 	"C:\\REDDOG\\GAME\\STRAT\\"
	#define	GAMEDIR		"C:\\REDDOG\\GAME\\"
#endif
#define LOGDIR 	"C:\\STRATS\\ERR\\"
#define SCRIPTDIR "C:\\STRATS\\PCODE\\"
#define INTDIR "C:\\STRATS\\INTERNAL\\"
#define HEADERDIR "C:\\STRATS\\HEADERS\\"
#define MAPDIR "C:\\STRATS\\MAP\\"

#if ARTIST
	#define MAKEFILEDIR "C:\\STRATS\\"
#else
	#if GODMODEMACHINE
		#define MAKEFILEDIR "D:\\DREAMCAST\\REDDOG\\GAME\\"
	#else
		#define MAKEFILEDIR "C:\\DREAMCAST\\REDDOG\\GAME\\"
	#endif
#endif

#define CPATH  "C:\\STRATS\\PCODE\\"
#if ARTIST
	#define WADDIR  "D:\\DREAMCAST\\WADS\\"
#else
	#if GODMODEMACHINE
		#define WADDIR  "D:\\DREAMCAST\\WADS\\"
	#else
		#define WADDIR  "C:\\DREAMCAST\\WADS\\"
	#endif
#endif
#define SYSWADDIR  "P:\\WADS\\"
#define DOMEDIR  "P:\\GAME\\NEWOBJECTS\\DOMES\\"
#define SFXDIR  "P:\\SOUND\\SFX\\"



//#define NEW_REDDOG_OBJECTS "P:\\GAME\\NEWOBJECTS\\"
//#define REDDOG_OBJECTS "P:\\GAME\\OBJECTS\\"
#endif
