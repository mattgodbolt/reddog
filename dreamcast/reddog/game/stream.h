/*
 * Stream.h
 * Access to CD subsystem
 */

#ifndef _STREAM_H
#define _STREAM_H

/*
 * The maximum number of open streams
 */
#define MAX_STREAMS 1

/*
 * The maximum number of files in a directory
 */
#define MAX_STR_DIRENT	64

/*
 * The number of sectors' worth of pre-reading that is done
 * 16 sectors is a pre-read of 64k
 */
#define STREAM_PREREAD 32

/*
 * A stream encapsulates a file
 */
typedef struct tagStream
{
	GDFS		gdfs;					/* Handle to OS file */
	Bool		fin;					/* Activity complete flag */
	Bool		active;					/* Is this descriptor active? */
	Sint32		bytesLeft;				/* Bytes left for this file */
	Uint32		secAmt;					/* Amount of valid data in the secBuf */
	Uint32		secPtr;					/* Offset into the secBuf */
	char		*preReadBuf;			/* Preread buffer */
	Uint32		preReadSize;			/* Size of preread buffer */
	char		*secBuf;				/* Data for partial-sector reads */
	char		*secBase;				/* Base of secBuf, non-32 aligned */
} Stream;

/*
 * Initialise the streams
 */
void sInit(void);

/*
 * Open a stream
 * Dies on an assertion failure in case the file can't be opened
 */
Stream *sOpen (char *filename);

/*
 * Close a stream
 * NB you *must* close all streams you open, unless
 * you call sCloseAll
 */
void sClose (Stream *str);

/*
 * Close all open streams
 * Used, for example, before playing an audio track
 * or when resetting to a well-defined point in the game
 */
void sCloseAll (void);

/*
 * Read bytes from a stream
 * Reads nBytes bytes into memory
 * Returns the number of bytes successfully read
 */
int sRead (void *memory, int nBytes, Stream *stream);

/*
 * Read bytes asynchronously from a stream
 * Reads nBytes bytes into memory asynchronously
 * Use sFinished() to check if the current operation has finished
 * NB ignores any data in the stream cache, so use only on freshly
 * opened files, memory must be a multiple of 32 and nBytes a multiple
 * of GDD_FS_SCTSIZE
 */
void sReadAsynch (void *memory, int nBytes, Stream *stream);

/*
 * Check to see if an asynchronous read has finished
 */
Bool sFinished (Stream *str);

/*
 * Used in WADs between files
 */
void sSkip (Stream *str);

/*
 * Set an explicit preread buffer
 */
void sSetBuf (Stream *str, char *buf, Uint32 size);

/*
 * Ignore some sheet
 */
void sIgnore (int nBytes, Stream *str);

#endif
 