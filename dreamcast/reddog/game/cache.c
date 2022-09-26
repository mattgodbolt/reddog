/*
 * Cache.c
 * Cache handling functions
 */

#include "RedDog.h"
#include "Cache.h"

#ifndef CACHE_LOG
#define CACHE_LOG 0
#endif

/*
 * Initialise a cache
 * No memory is allocated by this call - the caller must have allocated memory
 * The CHeap is used to hold cache information
 */
void CacheInit (Cache *cache, CHeap *heap, size_t heapSize)
{
	dAssert (cache != NULL, "NULL cache");
	dAssert (heap != NULL, "NULL heap");

	cache->isLocked	= FALSE;
	cache->nowTime	= 0xffffffff;
	cache->heap		= heap;
	cache->lowMem	= &heap->data[0];
	cache->hiMem	= cache->lowMem + heapSize;
}

/*
 * Frees an entry from the cache
 * This will automatically NULL the reference to the item
 */
void CacheFree (Cache *cache, char *memory)
{
	CacheEntry *entry;

	dAssert (cache != NULL, "NULL cache");
	dAssert (memory != NULL, "NULL memory");

	CacheLock (cache);

	/* Get the cache entry from the data pointer */
	entry = (CacheEntry *)(memory - CACHE_DATA_OFFSET);

	/* Ensure it is vaguely valid */
	dAssert (*(entry->reference) == memory, "Reference of cache entry != cache entry");

	/* Set the reference to NULL */
	*(entry->reference) = NULL;

	/* And free the cache entry */
	CHeapFree (cache->heap, entry);

	CacheUnlock (cache);
}

/*
 * Allocate some memory from a cache
 * To annoy you, and to prevent mishaps, the return value
 * is placed directly in reference, which MUST be the only
 * reference to the cache line
 * The cache must not be locked when you call this routine
 */
void CacheAlloc (Cache *cache, size_t bytes, char **reference)
{
	CacheEntry	*entry;
	Bool		isFree;
	CacheEntry	*oldestEntry;
	TimeStamp	oldestTime;

	CacheLock (cache);

	dAssert (cache != NULL, "NULL cache");
	dAssert (reference != NULL, "NULL reference");
	dAssert (bytes > 0, "Silly size");

	/* Make bytes a more manageable value, and add on the overhead of the cache */
	bytes = (bytes + 15 + sizeof (CacheEntry)) & ~15;

	/*
	 * Iterate through the whole heap finding either
	 * the first free block big enough, or the least
	 * recently used entry
	 */

	do {
		entry = (CacheEntry *)CHeapFirstBlock (cache->heap, &isFree);
		oldestEntry = NULL;
		oldestTime = 0x00000000;
		
		for ( ; entry != NULL; entry = (CacheEntry *)CHeapNextBlock (cache->heap, (char *)entry, &isFree)) {
			if (isFree) {
				if (CHEAP_SIZE_ALLOCED ((CHeapFreeBlock *)((char *)entry-4)) > bytes) {
					entry = (CacheEntry *) CHeapAllocNamed (cache->heap, (void *)entry, bytes);
					break;
				}
			} else {
				if ((cache->nowTime - entry->lastAccess) >= oldestTime) {
					oldestTime = (cache->nowTime - entry->lastAccess);
					oldestEntry = entry;
				}
			}
		}
		
		/* Could we find a big enough block? */
		if (entry == NULL) {
			/* No - so try freeing the oldest entry */
			dAssert (oldestEntry != NULL, "Cache too small!");
			CacheUnlock (cache);
			CacheFree (cache, (char *)&oldestEntry[1]);
			CacheLock (cache);
#if CACHE_LOG
			dLog ("Cache overflow (Cache@0x%x)\n", cache);
#endif
		}

	} while (entry == NULL);

	/* Mark in the reference */
	entry->lastAccess	= cache->nowTime;
	entry->reference	= reference;
	*reference			= ((char *)entry) + CACHE_DATA_OFFSET;

	/* Update the timestamp of the cache */
	cache->nowTime++;

	CacheUnlock (cache);
}

/*
 * Marks an entry in the cache as having being hit - this is
 * used by the LRU replacement strategy
 * This version can deal with CacheHits outside the cache
 */
void CacheHit (const Cache *cache, char *memory)
{
	CacheEntry *entry;

	dAssert (cache != NULL, "NULL cache");
	dAssert (memory != NULL, "NULL memory");

	/* Ignore memory outside of the cache */
	if (memory < cache->lowMem || memory >= cache->hiMem)
		return;

	/* Get the cache entry from the data pointer */
	entry = (CacheEntry *)(memory - CACHE_DATA_OFFSET);

	entry->lastAccess = cache->nowTime;
}

/*
 * Empties a cache
 * Note this function does not NULL references to the objects,
 * in case you haven't freed them yet
 */
void CacheEmpty (Cache *cache)
{
	CacheEntry	*entry;
	Bool		isFree;

	dAssert (cache != NULL, "NULL cache");
	CacheLock (cache);

	isFree = TRUE;
	do {
		entry = (CacheEntry *)CHeapFirstBlock (cache->heap, &isFree);
		for ( ; entry != NULL; entry = (CacheEntry *)CHeapNextBlock (cache->heap, (char *)entry, &isFree)) {
			if (!isFree) {
				CHeapFree (cache->heap, (void *)entry);
				break;
			}
		}
	} while (isFree == FALSE);

	cache->nowTime = 0xffffffff;
	CacheUnlock (cache);
}

/*
 * Compacts a cache by moving the first allocated block after
 * the first free block down
 */
void CacheCompact (Cache *cache) {}
/* XXX tricky to do in the case
   AABBBAAA where A is free */