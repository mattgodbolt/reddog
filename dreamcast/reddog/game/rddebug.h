/*
 * Red Dog assertion macros and other debugging routines
 */

#ifndef _RDDEBUG_H
#define _RDDEBUG_H

/*
 * Assertion macros
 * Under a debug build, all asserts are compiled in
 * Under a release build, only ANSI and fatal asserts are
 * compiled in - so only use these for really bad things
 * 
 * dAssert  = debug assert; fatal but not compiled into the release executables
 * fAssert  = fatal assert; fatal and present in release executable
 * wAssert  = non-fatal assert; violations are printed into the debug window
 * assert() = fatal assert - use of these is deprecated
 */
#if defined(_DEBUG)

extern void internalfAssert (char *, char *, char *, unsigned);
extern void internalwAssert (char *, char *, char *, unsigned);
extern int _ASSERT(int);

#ifdef _ARTIST
#define _ASSERT(a) (1)
#endif

/* Fatal assertions */
#define fAssert(expression, message) if (!(expression)) { internalfAssert (#expression, message, __FILE__, __LINE__); while (_ASSERT(0)) {;} } 
#define dAssert(expression, message) if (!(expression)) { internalfAssert (#expression, message, __FILE__, __LINE__); while (_ASSERT(0)) {;} }
/* Warning assertion */
#define wAssert(expression, message) if (!(expression)) internalwAssert (#expression, message, __FILE__, __LINE__)
/* Generic, ANSI-style assertion */
#define assert(expression) if (!(expression)) internalfAssert (#expression, "ANSI-style assertion failed", __FILE__, __LINE__)

#else

/* Non-debug build has fatal assertions */
extern void internalfAssert (char *, char *, char *, unsigned);

/* Fatal assertions */
#define fAssert(expression, message) if (!(expression)) internalfAssert (#expression, message, __FILE__, __LINE__) 
#define dAssert(expression, message) ((void) 0)
/* Warning assertion */
#define wAssert(expression, message) ((void) 0)
/* Generic, ANSI-style assertion */
#define assert(expression) if (!(expression)) internalfAssert (#expression, "ANSI-style assertion failed", __FILE__, __LINE__)

#endif

/*
 * log() function - printf's to the nwDebugWindow on a debug build, ignores otherwise
 */
void dPrintf (const char *fmt, ...);
#ifndef FINAL
	#if DLOG
		#define dLog dPrintf
	#else
		#define dLog (void)
	#endif
#else
#define dLog (void)
#endif


#ifndef _RELEASE
void TrapFPUExceptions(void);
void UnTrapFPUExceptions(void);
#else
#define TrapFPUExceptions() (void)0
#define UnTrapFPUExceptions() (void)0
#endif

#endif
