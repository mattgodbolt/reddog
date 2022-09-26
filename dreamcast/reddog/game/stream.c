/*
 * Stream layer
 */
#include "RedDog.h"

static Stream sBuf[MAX_STREAMS];

float sGetPC(void)
{
	float ret;
	Sint32 fileSize;
	if (!sBuf[0].active)
		return 0.f;
	gdFsGetFileSize (sBuf[0].gdfs, &fileSize);
	ret = (100.f * (float) (fileSize - sBuf[0].bytesLeft)) / (float)fileSize;
	return ret;
}

/*
 * Initialise the stream library
 */
void sInit (void)
{
	Sint32 ret;

	memset (sBuf, 0, sizeof (sBuf));
}

/*
 * Open a stream
 */
Stream *sOpen (char *filename)
{
	int str;
	Stream *retVal;

	// Take the oppurtunity to check the drive
	gdFsReqDrvStat();

	/*
	 * First find a disused Stream
	 */
	for (str = 0; str < MAX_STREAMS; ++str)
		if (!sBuf[str].active)
			break;
	dAssert (str != MAX_STREAMS, "Out of streams!");

	retVal = &sBuf[str];

	// Invalidate the areas used
	if (retVal->secBuf)
		syCacheOCBI (retVal->secBuf, GDD_FS_SCTSIZE * STREAM_PREREAD);
	if (retVal->preReadBuf)
		syCacheOCBI (retVal->preReadBuf, retVal->preReadSize);

	/* Now open the GDFS file */
	retVal->gdfs = gdFsOpen (filename, NULL);
	if (retVal->gdfs == NULL)
		sbExitSystem ();

	dAssert (retVal->gdfs, "Unable to open file");

	/* Steal some scratch RAM for the preread buffer */
	retVal->secBase = ScratchAlloc (GDD_FS_SCTSIZE * STREAM_PREREAD + 32);
	retVal->secBuf = (char *)((((int)retVal->secBase + 31) & ~31));

	/* Initialise the remaining values */
	retVal->fin = retVal->active = TRUE;
	retVal->secAmt = retVal->secPtr = 0;
	retVal->preReadBuf = retVal->secBuf;
	retVal->preReadSize = STREAM_PREREAD;
	gdFsGetFileSize (retVal->gdfs, &retVal->bytesLeft);

	return retVal;
}

/*
 * Close a stream
 */
void sClose (Stream *str)
{
	dAssert (str != NULL, "Bogus stream passed to sClose");
	gdFsClose (str->gdfs);

	// Invalidate the areas used
	if (str->secBuf)
		syCacheOCBI (str->secBuf, GDD_FS_SCTSIZE * STREAM_PREREAD);
	if (str->preReadBuf)
		syCacheOCBI (str->preReadBuf, str->preReadSize);

	if (str->secBase) {
		ScratchRewind (str->secBase);
		str->secBase = str->secBuf = NULL;
	}

	str->gdfs = NULL;
	str->active = FALSE;
	str->fin = FALSE;

	// Take the oppurtunity to check the drive
	gdFsReqDrvStat();
}

/*
 * Read bytes from a stream
 */
int sRead (void *mem, int nBytes, Stream *str)
{
	char *memory = (char *)mem;
	int nRead = 0;
	int nSecs;

	dAssert (str != NULL, "Bogus stream passed to sRead");
	dAssert (str->secPtr <= str->preReadSize * GDD_FS_SCTSIZE, "It's not working");

	/* Clamp to the end of the file */
	if (nBytes > str->bytesLeft)
		nBytes = str->bytesLeft;
	
	/*
	 * First copy any remaining data out of the secBuf
	 */
	if (str->secAmt) {
		/* Is there more data than we need? */
		if (str->secAmt >= nBytes) {
			memcpy (memory, &str->preReadBuf[str->secPtr], nBytes);
			str->secAmt -= nBytes;
			str->secPtr += nBytes;
			str->bytesLeft -= nBytes;
			return nBytes;
		} else {
			/* There isn't enough data in the buffer */
			int i;
			memcpy (memory, &str->preReadBuf[str->secPtr], str->secAmt);
			memory += str->secAmt;
			nRead += str->secAmt;
			str->secAmt = 0;
		}
	}

	/* Keep trying to read in whole chunks and copying them */
	while (nRead < nBytes) {
		Uint32 nToGo = nBytes - nRead;
		Uint32 nToRead = (nToGo  > (GDD_FS_SCTSIZE * str->preReadSize)) ? 
			(GDD_FS_SCTSIZE * str->preReadSize) : nToGo;
		Uint32 nLeft;
		Sint32 read;

		dAssert (str->secAmt == 0, "Internal inconsistency");

		/*
		 * How many sectors can we preread?
		 */
		nLeft = gdFsCalcSctSize (str->bytesLeft - nRead);
		nLeft = (nLeft > str->preReadSize) ? str->preReadSize : nLeft;

		read = gdFsRead (str->gdfs, nLeft, str->preReadBuf);
		dAssert (read == GDD_ERR_OK, "Error reading file");
		if (read != GDD_ERR_OK)
			sbExitSystem ();

		memcpy (memory, str->preReadBuf, nToRead);
		str->secPtr = nToRead;
		str->secAmt = (GDD_FS_SCTSIZE * nLeft) - (nToRead);
		memory += nToRead;
		nRead += nToRead;
		LoadPoll();
	}

	str->bytesLeft -= nRead;
	return nRead;
}

/*
 * Used in WADs between files
 */
void sSkip (Stream *str)
{
	static Uint32 wadEnd = 0xdeadbabe;
	Uint32 read;
	sRead (&read, 4, str);
	dAssert (read == wadEnd, "Corrupted WAD file - are you reading enough data?");
}

/*
 * Jump past data in the stream
 */
void sIgnore (int nBytes, Stream *str)
{
	char c;
	dAssert (str != NULL, "No stream");
	/*
	 * Ignore data left in the cache first
	 */
	if (str->secAmt) {
		if (str->secAmt >= nBytes) {
			str->secAmt -= nBytes;
			str->secPtr += nBytes;
			str->bytesLeft -= nBytes;
			return;
		} else {
			/* Flush the cache */
			str->bytesLeft -= str->secAmt;
			nBytes -= str->secAmt;
			str->secAmt = 0;
			str->secPtr = 0;
		}
	} else {
		while (nBytes > GDD_FS_SCTSIZE) {
			nBytes -= GDD_FS_SCTSIZE;
			str->bytesLeft -= GDD_FS_SCTSIZE;
			dAssert (str->bytesLeft > 0, "Seek past EOF");
			gdFsSeek (str->gdfs, 1, GDD_SEEK_CUR); 
		}
	}
	/* Cheesey as FUCK XXX */
	while (nBytes) {
		sRead (&c, 1, str);
		nBytes--;
	}
}

/*
 * Set an explicit preread buffer
 */
void sSetBuf (Stream *str, char *buf, Uint32 size)
{
	char *buf2;

	dAssert (str->secAmt == 0, "Unable to setbuf on an active stream");

	/* Align to 32byte boundary */
	buf2 = (char *)((int)(buf + 31) & ~31);
	size -= (buf2-buf);

	/* Adjust size to be sector length */
	size = size / GDD_FS_SCTSIZE;
	/* Write in */
	str->preReadBuf = buf2;
	str->preReadSize = size;

	/* Free the ScratchAlloced buffer */
	if (str->secBase) {
		ScratchRewind (str->secBase);
		str->secBase = str->secBuf = NULL;
	}
}

 /*
 * Read bytes asynchronously from a stream
 * Reads nBytes bytes into memory asynchronously
 * Use sFinished() to check if the current operation has finished
 */
void sReadAsynch (void *memory, int nBytes, Stream *str)
{
	dAssert (((int)memory & 0x1f) == 0, "Unaligned asynch action");
	dAssert ((nBytes & (GDD_FS_SCTSIZE-1)) == 0, "Not a sector-aligned length");
	dAssert (str, "No stream");
	syCacheOCBI(memory, nBytes);
	gdFsReqRd32 (str->gdfs, nBytes / GDD_FS_SCTSIZE, memory);
}

/*
 * Check to see if an asynchronous read has finished
 */
Bool sFinished (Stream *str)
{
	return (gdFsGetStat (str->gdfs) != GDD_STAT_READ);
}

