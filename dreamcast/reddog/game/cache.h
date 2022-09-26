/*
 * Cache.h
 * Cache handling functions
 */

#ifndef _CACHE_H
#define _CACHE_H

/*
 * A cache is used to store information that is essentially
 * transient, but which is not prudent to calculate every
 * frame.  For example, the landscape polygons do not have
 * a plane equation stored within them - instead when such
 * information is required, this data is calculated into
 * an area of memory allocated in a polygon cache.  The
 * mesh's normals pointer is then set to be the address in the
 * cache, and the cache line's reference pointer set to be the
 * address of the mesh's normals pointer.  The cache is then locked
 * during the accesses to the normals, and is unlocked when finished.
 * If the cache becomes full, entries can be discard - and the pointers
 * to them are set to null.
 *
 * Points to note:
 *		There must be one, and only one pointer to the information 
 *		in the cache.
 *		The cache must be locked during accesses to prevent stale
 *		pointers from being used
 *
 * Example
 *	if (model->normals == NULL) {
 *		model->normals = CacheAlloc (NormalsCache, model->blah * sizeof(), &model->normals);
 *		CacheLock (NormalsCache);
 *		CalcNormals (model->normals);
 *	} else {
 *		CacheLock (NormalsCache);  prevent any further calls from disrupting the cache
 *	}
 *	...draw the model / collide the model
 *	CacheUnlock (NormalsCache);
 *
 */

/* Get the definitions of the CHeap */
#include "Memory.h"

/*
 * Types and structures
 */

/* An entry in the cache */
typedef struct tagCacheEntry
{
	char		**reference;			/* The reference to this cache entry */
	TimeStamp	lastAccess;				/* The timestamp of the last access */
/*	char		data[0];				   The data in this cache */
} CacheEntry;

#define CACHE_DATA_OFFSET sizeof (CacheEntry)

/* A cache in itself */
typedef struct tagCache
{
	Bool		isLocked;				/* Is the cache locked? */
	TimeStamp	nowTime;				/* The time now */
	CHeap		*heap;					/* The heap of memory for this cache */
	char		*lowMem;				/* Lowest extent of cache in memory */
	char		*hiMem;					/* Highest extent of cache in memory */
} Cache;

/*
 * Functions
 */

/* Locks a cache */
#pragma inline (CacheLock)
static void CacheLock (Cache *cache)
{
	dAssert (cache != NULL, "Null cache locked");
	fAssert (cache->isLocked == FALSE, "Cache is already locked");
	cache->isLocked = TRUE;
}

/* Unlocks a cache */
#pragma inline (CacheUnlock)
static void CacheUnlock (Cache *cache)
{
	dAssert (cache != NULL, "Null cache unlocked");
	fAssert (cache->isLocked == TRUE, "Cache is already unlocked");
	cache->isLocked = FALSE;
}

/*
 * Initialise a cache
 * No memory is allocated by this call - the caller must have allocated memory
 * The CHeap is used to hold cache information
 */
void CacheInit (Cache *cache, CHeap *heap, size_t heapSize);

/*
 * Frees an entry from the cache
 * This will automatically NULL the reference to the item
 */
void CacheFree (Cache *cache, char *memory);

/*
 * Allocate some memory from a cache
 * To annoy you, and to prevent mishaps, the return value
 * is placed directly in reference, which MUST be the only
 * reference to the cache line
 * The cache must not be locked when you call this routine
 */
void CacheAlloc (Cache *cache, size_t bytes, char **reference);

/*
 * Marks an entry in the cache as having being hit - this is
 * used by the LRU replacement strategy
 * This version can deal with CacheHits outside the cache
 */
void CacheHit (const Cache *cache, char *memory);

/*
 * Fast, inline cacheHit routine
 * Data >must< be in the cache
 */
#pragma inline (CacheHitInternal)
static void CacheHitInternal (const Cache *cache, char *memory)
{
	dAssert (memory >= cache->lowMem && memory < cache->hiMem, "External cache hit");
	((CacheEntry *)(memory - (CACHE_DATA_OFFSET)))->lastAccess = cache->nowTime;
}

/*
 * Empties a cache
 * Note this function does not NULL references to the objects,
 * in case you haven't freed them yet
 */
void CacheEmpty (Cache *cache);

#endif
