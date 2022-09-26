/*
 * Memory.h
 * API for freeing and releasing memory
 */

#ifndef _MEMORY_H
#define _MEMORY_H

/* Define this to 1 to rigorously check the heaps on allocation and freeing of CHeap blocks */
#ifdef _DEBUG
#define CHECK_CHEAP_BLOCKS 1
#endif

#define MissionHeap				GameHeap
#define CollisionHeap			GameHeap
#define MapHeap					GameHeap
#define ScratchAlloc(size)		(SHeapAlloc (ScratchHeap, size))
#define MissionAlloc(size)		(SHeapAlloc (GameHeap, size))
#define MapAlloc(size)			(SHeapAlloc (GameHeap, size))
#define CollisionAlloc(size)	(SHeapAlloc (GameHeap, size))
#define ScratchRewind(block)	(SHeapRewind (ScratchHeap, block))
#define FloatVarAlloc(size)		(CHeapAlloc (FloatVarHeap, size))
#define IntVarAlloc(size)		(CHeapAlloc (IntVarHeap, size))
#define ObjAlloc(size)			(CHeapAlloc (ObjHeap, size))
#define ObjFree(addr)			CHeapFree(ObjHeap, addr)
#define SoundAlloc(size)		(CHeapAlloc (SoundHeap, size))
#define SoundFree(addr)			CHeapFree(SoundHeap, addr)

#define GAMEHEAP_TYPE			SHeap
#define SCRATCHHEAP_TYPE		SHeap
#define FLOATVARHEAP_TYPE		CHeap
#define INTVARHEAP_TYPE			CHeap
#define OBJHEAP_TYPE			CHeap
#define SOUNDHEAP_TYPE			CHeap

/* Initialise the memory subsystem */
void InitMemory (void);

/*************************************/

#define CHECK_HEAP(heap) dAssert (heap->sentinel==HEAP_SENTINEL, "Not a regular heap")

/*
 * Definition of a iteratable heap
 * Data can be allocated from the heap, but not freed
 * Iteration can take place in the heap
 * There is a (sizeof(size_t)) overhead on each entry
 * Any blocks are aligned to sizoef(size_t) boundarys
 * Failure to allocate causes a run-time error on debug, and a crash on release
 */
			    
typedef struct tagHeap
{
#define HEAP_SENTINEL (*((Uint32 *)&"HEAP"))
	Uint32		sentinel;	/* Sentinel to guard against calling wrong heap routines */
	char		*end;		/* Pointer to byte after last in the heap */
	char		*curPtr;	/* Current pointer */
	char		data[4];	/* Data resides hereafter */
} Heap, *pHeap;

/*
 * Allocate a new heap of the given size
 */
Heap *HeapCreate (size_t size);

/*
 * Allocate memory from a heap
 */
void *HeapAlloc (Heap *heap, size_t size);

/*
 * Iterate through a heap, calling the given function
 * with each entry in the heap, and the given private word
 */
typedef void HeapIterateFunction (void *heapData, void *privateWord);
static void HeapIterate (Heap *heap, HeapIterateFunction *heapFunction, void *privateWord);

/*
 * Clear a heap
 */
void HeapClear (Heap *heap);

/**************************************/

#define CHECK_SHEAP(heap) dAssert (heap->sentinel==SHEAP_SENTINEL, "Not a simple heap")

/*
 * Definition of a simple heap
 * Data can be allocated from the heap, but not freed
 * No iteration can take place in the heap
 * No overhead per entry
 * Returned blocks are on any alignment
 * Failure to allocate causes a run-time error on debug, and a crash on release
 */

typedef struct tagSHeap
{
#define SHEAP_SENTINEL (*((Uint32 *)&"SIHP"))
	Uint32		sentinel;		/* Sentinel */
	char		*end;		/* Pointer to byte after last in the heap */
	char		*curPtr;	/* Current pointer */
	char		data[4];	/* Data resides hereafter */
} SHeap, *pSHeap;

/*
 * Allocate a new heap of the given size
 */
static SHeap *SHeapCreate (size_t size);

/*
 * Allocate memory from a heap (any alignment)
 */
#ifndef MAX_INCLUDED
#pragma inline (SHeapAlloc)
static void *SHeapAlloc (SHeap *heap, size_t size)
{
	void *retVal;
	/*FIX ME*/
   /*	int i;*/
	//TO CATER FOR 0 SIZE REQUESTS
	if (!size)
		return(NULL);

	CHECK_SHEAP (heap);


	retVal = (void *)heap->curPtr;

   /*	dAssert (retVal != NULL, "Out of memory!!!"); */



	/*FIX ME*/
   /*	for (i=0;i<size;i++)
  		heap->curPtr[i]=0;
	 */


	heap->curPtr += (size+3) & 0xfffffffc;

	dAssert (heap->curPtr <= heap->end, "Simple heap size exceeded");
	dAssert (heap->curPtr > &heap->data[0], "Regular heap size exceeded by a hooj amount");

	return retVal;
}
#endif

/*
 * Rewind a heap pointer such that a new allocation request
 * will return block
 */
#ifndef MAX_INCLUDED
#pragma inline (SHeapRewind)
static void SHeapRewind (SHeap *heap, void *block)
{
	CHECK_SHEAP (heap);

	dAssert ((void *)heap->curPtr >= block, "Attempt to rewind SHeap forwards");
	dAssert ((void *)&heap->data[0] <= block, "Attempt to rewind SHeap behind start");

	heap->curPtr = (char *)block;
}
#endif

/*
 * Clear a heap
 */
void SHeapClear (SHeap *heap);

/**************************************/

#ifdef MEMORY_CHECK
#define CHECK_CHEAP(heap) dAssert (heap->sentinel==CHEAP_SENTINEL, "Not a simple heap"); CheckCHeap(heap)
#else
#define CHECK_CHEAP(heap) dAssert (heap->sentinel==CHEAP_SENTINEL, "Not a simple heap")
#endif
#define CHEAP_TAG (1<<31)
#define CHEAP_MASK ~CHEAP_TAG
#define CHEAP_IS_FREE(block) ((((block)->tag1) & CHEAP_TAG) == 0)
#define CHEAP_IS_ALLOCED(block) ((((block)->tag1) & CHEAP_TAG) != 0)
#define CHEAP_SIZE(block) ((block)->tag1 & CHEAP_MASK)
#define CHEAP_SIZE_ALLOCED(block) (((block)->tag1 & CHEAP_MASK) - sizeof (CHeapFreeBlock))
#define CHEAP_FIND_ENDPTR(newFreeBlock) ((Uint32 *)(((char *)(newFreeBlock)) + CHEAP_SIZE(newFreeBlock) - 4));

/*
 * Definition of a complex heap
 * Data can be allocated and freed from the heap
 * Iteration can take place in the heap
 * 8 byte overhead per block
 * Returned blocks are on 4-byte boundaries
 * Failure to allocate returns NULL on both debug and release
 */

typedef struct tagCHeapFreeBlock
{
	Uint32						tag1;
	struct tagCHeapFreeBlock	*next;
	struct tagCHeapFreeBlock	*prev;
	char						data[4];		/* not used.. but ensures sizeof (CHFreeBlock) is correct */
	/* Uint32			tag2; */
} CHeapFreeBlock;

typedef struct tagCHeapAllocedBlock
{
	Uint32					tag1;
#ifdef MEMORY_CHECK
	// Let's trace this muther
	void					*whence;
#endif
	char					data[4];		/* data is stored here */
	/* Uint32			tag2; */
} CHeapAllocedBlock;

typedef struct tagCHeap
{
#define CHEAP_SENTINEL (*((Uint32 *)&"CMHP"))
	Uint32			sentinel;		/* Sentinel */
	CHeapFreeBlock	*firstFree;		/* First free block */
	Uint32			nAlloced;		/* The number of bytes allocated from this heap */
	Uint32			keeper;			/* The first keeper */
	char			data[4];		/* Data resides hereafter */
	/* Uint32 secondKeeper		// and at the end, the second keeper */
} CHeap, *pCHeap;

/*
 * Allocate a new heap of the given size
 */
static CHeap *CHeapCreate (size_t size);

/*
 * Allocate memory from a heap
 */
void *CHeapAlloc (CHeap *heap, size_t size);

/*
 * Free memory from a heap
 */
void CHeapFree (CHeap *heap, void *memory);

/*
 * Return the first block in a heap, filling in isFree accordingly
 * Free blocks return a pointer into their 'next' field
 */
static void *CHeapFirstBlock (CHeap *heap, Bool *isFree);

/*
 * Return the next block in a heap, filling in isFree accordingly
 */
static void *CHeapNextBlock (CHeap *heap, char *prev, Bool *isFree);

/*
 * Get the status of a CHeap
 */
void CHeapGetStats (CHeap *heap, int *FreeMem, int *LargestBlock);

/*
 * Global variables
 */
extern	GAMEHEAP_TYPE		*GameHeap;
extern	char				*GameHeapBase;
extern	FLOATVARHEAP_TYPE   *FloatVarHeap;
extern	INTVARHEAP_TYPE		*IntVarHeap;
extern	OBJHEAP_TYPE		*ObjHeap;
extern	SCRATCHHEAP_TYPE	*ScratchHeap;
extern	SOUNDHEAP_TYPE		*SoundHeap;
 
/* Types used to pass allocators to functions */
typedef void *Allocator (void *, size_t);
typedef void *Deallocator (void *, void *);

/* Not the best place for this, and its slow */
int strnicmp (const char *a, const char *b, int num);
int stricmp (const char *a, const char *b);

#endif
