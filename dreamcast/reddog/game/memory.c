/*
 * Memory.c
 * Implementation of a memory system
 */

#include "RedDog.h"
#include "Memory.h"

#define MEG *1024*1024

 /*
 * We now have one unified game heap
 */
#define GAMEHEAP_SIZE		(10 MEG)
#define GAMEHEAP_CREATE		SHeapCreate

/*
 * Scratch memory management
 * Scratch memory should be used for transient memory
 * needs - eg it is used in the material loader to
 * store textures in main RAM prior to uploading them
 * to texture RAM
 */
#define SCRATCHHEAP_SIZE		(128 * 1024)
#define SCRATCHHEAP_CREATE		SHeapCreate

/*
 * StratVar memory management
 * Anything allocated on this heap is destroyed next time
 * a map is loaded in (usually beginning of a mission)
 */
#define FLOATVARHEAP_SIZE	(16 * 1024)
#define FLOATVARHEAP_CREATE	CHeapCreate

#define INTVARHEAP_SIZE		(16 * 1024)
#define INTVARHEAP_CREATE	CHeapCreate

/* 
 * PNode collision heap space warez 
 */
#define OBJHEAP_SIZE		(512 * 1024)
#define OBJHEAP_CREATE		CHeapCreate

#define SOUNDHEAP_SIZE		(32 * 1024)
#define SOUNDHEAP_CREATE	CHeapCreate

////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * Global variables
 */
GAMEHEAP_TYPE			*GameHeap;
char					*GameHeapBase;

/* on complex heap */
FLOATVARHEAP_TYPE		*FloatVarHeap;
INTVARHEAP_TYPE			*IntVarHeap; 
OBJHEAP_TYPE			*ObjHeap; 
SCRATCHHEAP_TYPE		*ScratchHeap;
SOUNDHEAP_TYPE			*SoundHeap;

/*
 * The Katana memory pointer
 */

/*
 * Internally used permanent memory allocation
 */
static char *PermAlloc (size_t size)
{
	int i;
	char *retVal;
	retVal = syMalloc (size);

	dAssert (retVal != NULL, "Out of memory!!!");

	return retVal;
}

/*
 * Create a heap
 */

Heap *HeapCreate (size_t size)
{
	Heap *retVal;
	size_t totalSize;
	
	/* Calculate the total size of the heap, including the heap header */
	totalSize = size + sizeof (Heap) - sizeof (char[4]);

	/* Allocate the heap */
	retVal = (Heap *)PermAlloc (totalSize);

	/* Fill in the details */
	retVal->end			= (((char *)retVal) + totalSize);
	retVal->curPtr		= (char *)&retVal->data[0];
	retVal->sentinel	= HEAP_SENTINEL;

	return retVal;
}

/*
 * Allocate off the heap
 */
void *HeapAlloc (Heap *heap, size_t size)
{
	int i;
	void *retVal;
	
	CHECK_HEAP (heap);

	/* Align to a multiple of size_t and add an extra size_t in */
	/* size = ((size + sizeof(size_t) - 1) & ~sizeof(size_t)) + sizeof (size_t); optimizes to... */
	size = ((size + sizeof(size_t)*2 - 1) & ~(sizeof(size_t)-1));

	/* Calculate the return value */
	retVal = (void *)(heap->curPtr + sizeof (size_t));


	/*FIX ME*/
  	/*for (i=0;i<sizeof(size_t);i++)
  		heap->curPtr[i]=0;
  	  */

	/* Place the size of this block in the heap */
	*((size_t *)heap->curPtr) = size;

	/* Move current pointer on  */
	heap->curPtr += size;

	dAssert (heap->curPtr < heap->end, "Regular heap size exceeded");
	dAssert (heap->curPtr > &heap->data[0], "Regular heap size exceeded by a hooj amount");

	return retVal;
}


/*
 * Iterate through the heap
 */
static void HeapIterate (Heap *heap, HeapIterateFunction *heapFunction, void *privateWord)
{
	char *curBlock;
	size_t blockSize;
	char *element;

	CHECK_HEAP (heap);

	for (curBlock = (char *)&heap->data[0]; curBlock < heap->end; curBlock += blockSize)
	{
		/* Get the size of this block */
		blockSize = *((size_t *)curBlock);

		/* Calculate the position of the element */
		element = curBlock + sizeof (size_t);

		/* Call the iterated function */
		heapFunction ((void *)element, privateWord);
	}
}

/*
 * Clear a heap
 */
void HeapClear (Heap *heap)
{
	CHECK_HEAP (heap);

	heap->curPtr = (char *)&heap->data[0];
}


/************************************/


/*
 * Simple heap routines
 */

/*
 * Allocate a new heap of the given size
 */
static SHeap *SHeapCreate (size_t size)
{
	SHeap *retVal;
	size_t totalSize;
	
	/* Calculate the total size of the heap, including the heap header */
	totalSize = size + sizeof (SHeap) - sizeof (char[4]);

	/* Allocate the heap */
	retVal = (SHeap *)PermAlloc (totalSize);

	/* Fill in the details */
	retVal->end			= (((char *)retVal) + totalSize);
	retVal->curPtr		= (char *)&retVal->data[0];
	retVal->sentinel	= SHEAP_SENTINEL;

	return retVal;
}

/*
 * Clear a heap
 */
void SHeapClear (SHeap *heap)
{
	heap->curPtr		= (char *)&heap->data[0];
}

/*************************************/

/*
 * Complex heap routines
 */

/* Mark a block as a free block of the given size */
static void MarkBlockAsFree (CHeapFreeBlock *freeBlock, size_t size, CHeap *heap)
{
	Uint32 *endPtr;

	fAssert (size >= sizeof (CHeapFreeBlock), "Free block too small in Complex heap");
	fAssert (freeBlock != NULL, "NULL block passed");

	freeBlock->tag1		= size;
	
	if (heap->firstFree)
		heap->firstFree->prev = freeBlock;

	freeBlock->next		= heap->firstFree;
	freeBlock->prev		= NULL;

	heap->firstFree		= freeBlock;

	endPtr = (Uint32 *)(((char *)freeBlock) + size - 4);
	*endPtr = size;
}

/*
 * Allocate a new heap of the given size
 */
static CHeap *CHeapCreate (size_t size)
{
	CHeap *retVal;
	Uint32 *endKeeper;
	size_t totalSize;

	fAssert (size >= sizeof (CHeapFreeBlock), "Initial Complex heap size too small");
	
	/* Calculate the total size of the heap, including the heap header, and the end keeper */
	totalSize = size + sizeof (CHeap) - sizeof (char[4]) + 4;

	/* Allocate the heap */
	retVal = (CHeap *)PermAlloc (totalSize);

	/* Fill in the details */
	retVal->sentinel	= CHEAP_SENTINEL;
	retVal->nAlloced	= 0;
	retVal->keeper		= 0xffffffff;		/* Fill in the first keeper */
	retVal->firstFree	= NULL;

	/* Calculate the position of the second keeper and fill it in */
	endKeeper = (Uint32 *)((char *)retVal + totalSize - 4);
	*endKeeper			= 0xffffffff;

	/* Set up the initial free block */
	MarkBlockAsFree ((CHeapFreeBlock *)&retVal->data[0], totalSize - sizeof (CHeap), retVal);

	/* On a memory check build, set the data */
#ifdef MEMORY_CHECK
	{ 
		Uint32 *start, *end;
		start = (Uint32 *)&retVal->firstFree->data[0];
		end = CHEAP_FIND_ENDPTR(retVal->firstFree);
		while (start < end)
			*start++ = 0xfeedb00b;
	}
#endif
	return retVal;
}

#ifdef CHECK_CHEAP_BLOCKS
static Bool CHeapCheckBlock (void *mem)
{
	CHeapAllocedBlock *block = (CHeapAllocedBlock *)mem;
	Uint32 *endPtr = CHEAP_FIND_ENDPTR (block);
	return (block->tag1 == *endPtr);
}
#endif

#ifdef MEMORY_CHECK
/*
 * CheckCHeap
 */
void CheckCHeap (CHeap *heap)
{
	CHeapAllocedBlock *block;

	dAssert (heap != NULL, "Heap is NULL");

	for (block = (CHeapAllocedBlock *)&heap->data[0]; ;) {
		if (block->tag1 == 0xffffffff) {	/* Last block? */
			return;
		}
		dAssert (CHeapCheckBlock (block), "CHeap block mismatch");
		// Check free blocks haven't been munged
		if (CHEAP_IS_FREE(block)) {
			Uint32 *start, *end;
			start = (Uint32 *)&((CHeapFreeBlock*)block)->data[0];
			end = CHEAP_FIND_ENDPTR(block);
			while (start < end) {
				dAssert (*start == 0xfeedb00b, "A freed block has been written to!");
				start++;
			}
		}
		block = (CHeapAllocedBlock *)(((char *)(block)) + CHEAP_SIZE(block));
	}
}

void CheckCHeapIsFree (CHeap *heap)
{
	CHeapAllocedBlock *block;

	dAssert (heap != NULL, "Heap is NULL");

	for (block = (CHeapAllocedBlock *)&heap->data[0]; ;) {
		if (block->tag1 == 0xffffffff) {	/* Last block? */
			return;
		}
		dAssert (CHeapCheckBlock (block), "CHeap block mismatch");
		if (CHEAP_IS_ALLOCED(block)) {
			void *alloc = block->whence;
			dAssert (FALSE, "Allocated block found in alledgedly free heap");
			// If you get here, then go into your code window, hit ctrl-G, and type 'alloc'
			// You should then be at the line which allocated the memory that was left allocated
		}
		block = (CHeapAllocedBlock *)(((char *)(block)) + CHEAP_SIZE(block));
	}
}
#endif

/*
 * Allocate memory from a heap
 */
#ifdef MEMORY_CHECK
static void *CHeapAllocInternal (CHeap *heap, CHeapFreeBlock *block, size_t size, void *whence)
#else
static void *CHeapAllocInternal (CHeap *heap, CHeapFreeBlock *block, size_t size)
#endif
{
	/* Is there enough room to allocate a free block in the left-over space? */
	if (CHEAP_SIZE(block) - size >= sizeof (CHeapFreeBlock)) {
		CHeapFreeBlock		*newFreeBlock;
		Uint32				*endPtr;
		size_t				freeSize;

		/* Calculate the position and size of the new free block */
		newFreeBlock = (CHeapFreeBlock *)(((char *)block) + size);
		freeSize = CHEAP_SIZE(block) - size;

		/* Update the linked lists on either side of the block, and fill in the new free block */
		newFreeBlock->next = block->next;
		if (newFreeBlock->next)
			newFreeBlock->next->prev = newFreeBlock;

		newFreeBlock->prev = block->prev;
		if (newFreeBlock->prev)
			newFreeBlock->prev->next = newFreeBlock;
		else
			heap->firstFree = newFreeBlock;

		/* Be careful to update tag1 first as CHEAP_FIND_ENDPTR uses it */
		newFreeBlock->tag1 = freeSize;
		endPtr = CHEAP_FIND_ENDPTR (newFreeBlock);
		*endPtr = freeSize;

		/* Update the block to be allocated */
		block->tag1 = size | CHEAP_TAG;
		endPtr = CHEAP_FIND_ENDPTR (block);
		*endPtr = size | CHEAP_TAG;

	} else {
		Uint32	*endPtr;

		/*
		 * The block actually allocated is the whole block, so unchain it
		 * from the free list
		 */
		if (block->prev)
			block->prev->next = block->next;
		else
			heap->firstFree = block->next;

		if (block->next)
			block->next->prev = block->prev;

		/* Mark the block as allocated */
		endPtr = CHEAP_FIND_ENDPTR (block);
		*endPtr = block->tag1 = (block->tag1 | CHEAP_TAG);

	}

	/* On a memory check build, set the data */
#ifdef MEMORY_CHECK
	{ 
		Uint32 *start, *end;
		start = (Uint32 *)&block->data[0];
		end = CHEAP_FIND_ENDPTR(block);
		while (start < end)
			*start++ = 0xfbadc0de;
	}
	/* Also, update the nAlloced */
	heap->nAlloced += CHEAP_SIZE(block);
	((CHeapAllocedBlock *)block)->whence = whence;
#endif

	/* Return the address of the data within */
	return (void *)(&((CHeapAllocedBlock *)block)->data[0]);
}
/*int ALLOCS = 0; */

#ifdef MEMORY_CHECK
	size_t CHAPALLOC=0;
#endif

void *CHeapAlloc (CHeap *heap, size_t size)
{
	CHeapFreeBlock *block;


/*	ALLOCS ++; */
/*	dLog("alloc %d\n",ALLOCS); */

#ifdef MEMORY_CHECK
	CHAPALLOC += size;
#endif
	
	CHECK_CHEAP (heap);

	/* Make sure we haven't been asked anything too silly */
	wAssert (size != 0, "Zero length allocation request");
	if (size==0)
		return NULL;

	/* Round size to at least a 4-byte boundary, and add the 8 byte overhead */
	size = (size + 3 + sizeof (CHeapAllocedBlock)) & ~3;

	/* Make sure the size is at least enough to hold an empty free block */
	if (size < sizeof (CHeapFreeBlock))
		size = sizeof (CHeapFreeBlock);

	for (block = heap->firstFree; block; block = block->next) {

#ifdef CHECK_CHEAP_BLOCKS
		dAssert (CHEAP_IS_FREE(block), "Allocated block in free block chain");

		dAssert (CHeapCheckBlock (block), "CHeap free block tag mismatch");		
#endif
		/* Is the block big enough? */
		if (CHEAP_SIZE(block) >= size)
		{
		/*	if (heap==ObjHeap) */
		/*		 ("allocated\n"); */
#ifdef MEMORY_CHECK
			extern Uint32 *getSP(void);
			Uint32 *stack = getSP();
			return CHeapAllocInternal (heap, block, size, (void*)(stack[4]-4));
#else
			return CHeapAllocInternal (heap, block, size);
#endif

		}
	}

	return NULL;
}

/*
 * Free memory from a heap
 */
void CHeapFree (CHeap *heap, void *memory)
{
	CHeapAllocedBlock *block;
	CHeapFreeBlock *freeBlock, *nextFreeBlock;
	Uint32 *endPtr, *prevBlockTag, *nextBlockTag;
	size_t totSize;
	Uint32 selector;

/*	ALLOCS --; */
/*	dLog("alloc %d\n",ALLOCS); */

	CHECK_CHEAP (heap);

	dAssert (memory != NULL, "NULL pointer passed to CHeapFree");
	if (memory == NULL)
		return;

	/* Get the block, and the end pointer */
	block	= (CHeapAllocedBlock *)(((char *)memory)-offsetof(CHeapAllocedBlock, data));
	endPtr	= CHEAP_FIND_ENDPTR (block);

	dAssert (block->tag1 == *endPtr, "Non-MAlloced or corrupt block passed to CHeapFree");
	dAssert (CHEAP_IS_ALLOCED (block), "Free block passed to CHeapFree");

#ifdef MEMORY_CHECK
	heap->nAlloced -= CHEAP_SIZE(block);
#endif

	/* Check the blocks at either end of the allocated block */
	prevBlockTag = (Uint32 *)(((char *)block) - 4);
	nextBlockTag = (Uint32 *)(((char *)block) + CHEAP_SIZE(block));

	/* Convert the free/allocated status into one of four possible states */
	selector = ((*prevBlockTag & CHEAP_TAG) >> 30) | (*nextBlockTag >> 31);
	switch (selector) {
	case 0: /* Case when both the previous and next blocks are free */
		/*
		 * Firstly, we need to remove the next free block from the free
		 * chain, as all three blocks (two currently on the chain)
		 * will become a single block
		 */
		nextFreeBlock = (CHeapFreeBlock *)nextBlockTag;
		freeBlock = (CHeapFreeBlock *)(((char *)block) - *prevBlockTag);
		totSize = *prevBlockTag + *nextBlockTag + CHEAP_SIZE(block);

#ifdef CHECK_CHEAP_BLOCKS
		dAssert (CHeapCheckBlock (nextFreeBlock), "Next block corrupt");
		dAssert (CHeapCheckBlock (freeBlock), "Previous block corrupt");
#endif

		if (nextFreeBlock->prev)
			nextFreeBlock->prev->next = nextFreeBlock->next;
		else
			heap->firstFree = nextFreeBlock->next;
		if (nextFreeBlock->next)
			nextFreeBlock->next->prev = nextFreeBlock->prev;

		/*
		 * Next, invalidate the tag of the current block to prevent it being mistaken
		 * for an allocated block and being freed twice
		 */
		block->tag1 = 0xdeadbeef;

		/*
		 * Now simply update the free block to be the total size
		 */
		freeBlock->tag1 = totSize;
		endPtr = CHEAP_FIND_ENDPTR (freeBlock);
		*endPtr = totSize;

		/* For debugging purposes set as free */
#ifdef MEMORY_CHECK
		{
			Uint32 *start;
			start = (Uint32 *)&freeBlock->data[0];
			while (start < endPtr)
				*start++ = 0xfeedb00b;
		}
#endif

		break;

	case 1:	/* Case when the previous block only is free */
		/*
		 * Simply extend the previous block to be totSize big
		 * and invalidate the tag for the current block
		 */
		freeBlock = (CHeapFreeBlock *)(((char *)block) - *prevBlockTag);
		totSize = *prevBlockTag + CHEAP_SIZE(block);

#ifdef CHECK_CHEAP_BLOCKS
		dAssert (CHeapCheckBlock (freeBlock), "Previous block corrupt");
#endif

		freeBlock->tag1 = totSize;
		endPtr = CHEAP_FIND_ENDPTR (freeBlock);
		*endPtr = totSize;

		block->tag1 = 0xdeadbeef;

		/* For debugging purposes set as free */
#ifdef MEMORY_CHECK
		{
			Uint32 *start;
			start = (Uint32 *)&freeBlock->data[0];
			while (start < endPtr)
				*start++ = 0xfeedb00b;
		}
#endif

		break;

	case 2: /* Case when the next block only is free */
		/* Remove the next block from the freelist */
		nextFreeBlock = (CHeapFreeBlock *)nextBlockTag;
		totSize = *nextBlockTag + CHEAP_SIZE(block);

#ifdef CHECK_CHEAP_BLOCKS
		dAssert (CHeapCheckBlock (nextFreeBlock), "Next block corrupt");
#endif
		if (nextFreeBlock->prev)
			nextFreeBlock->prev->next = nextFreeBlock->next;
		else
			heap->firstFree = nextFreeBlock->next;
		if (nextFreeBlock->next)
			nextFreeBlock->next->prev = nextFreeBlock->prev;

		/* Convert the block into a free block of size totSize */
		freeBlock = (CHeapFreeBlock *)block;
		freeBlock->tag1 = totSize;
		endPtr = CHEAP_FIND_ENDPTR(freeBlock);
		*endPtr = totSize;

		/* Chain it onto the linked list */
		freeBlock->prev = NULL;
		freeBlock->next = heap->firstFree;

		if (heap->firstFree)
			heap->firstFree->prev = freeBlock;

		heap->firstFree = freeBlock;

		/* For debugging purposes set as free */
#ifdef MEMORY_CHECK
		{
			Uint32 *start;
			start = (Uint32 *)&freeBlock->data[0];
			while (start < endPtr)
				*start++ = 0xfeedb00b;
		}
#endif

		break;

	case 3: /* Case when neither block around this block is free */
		/*
		 * Simply chain this block onto the freeList and we're done
		 */
		freeBlock = (CHeapFreeBlock *)block;
		freeBlock->tag1 = *endPtr = CHEAP_SIZE(freeBlock);

		freeBlock->prev = NULL;
		freeBlock->next = heap->firstFree;

		if (heap->firstFree)
			heap->firstFree->prev = freeBlock;

		heap->firstFree = freeBlock;

		/* For debugging purposes set as free */
#ifdef MEMORY_CHECK
		{
			Uint32 *start;
			start = (Uint32 *)&freeBlock->data[0];
			while (start < endPtr)
				*start++ = 0xfeedb00b;
		}
#endif
		break;
	}
}

/*
 * Return the first block in a heap, filling in isFree accordingly
 */
static void *CHeapFirstBlock (CHeap *heap, Bool *isFree)
{
	CHeapAllocedBlock *retVal;

	dAssert (heap != NULL, "NULL heap");
	dAssert (isFree != NULL, "NULL isFree ptr");

	CHECK_CHEAP (heap);

	retVal = (CHeapAllocedBlock *)&heap->data[0];
	if (CHEAP_IS_FREE(retVal))
		*isFree = TRUE;
	else
		*isFree = FALSE;

	return &retVal->data[0];
}

/*
 * Return the next block in a heap, filling in isFree accordingly
 */
static void *CHeapNextBlock (CHeap *heap, char *prev, Bool *isFree)
{
	CHeapAllocedBlock	*retVal;
	size_t				size;

	dAssert (heap != NULL, "NULL heap");
	dAssert (prev != NULL, "NULL previous pointer");
	dAssert (isFree != NULL, "NULL isFree ptr");

	CHECK_CHEAP (heap);

	size = CHEAP_SIZE((CHeapAllocedBlock *)(prev - offsetof(CHeapAllocedBlock, data)));
	
	retVal = (CHeapAllocedBlock *)(prev - offsetof(CHeapAllocedBlock, data) + size);

	/* End of the heap? */
	if (retVal->tag1 == 0xffffffff)
		return NULL;

	if (CHEAP_IS_FREE(retVal))
		*isFree = TRUE;
	else
		*isFree = FALSE;

	return &retVal->data[0];
}

/*
 * Get the CHeap stats
 */
void CHeapGetStats (CHeap *heap, int *FreeMem, int *LargestBlock)
{
	void *vo;
	int free, large;
	Bool isFree;
	CHECK_CHEAP (heap);

	free = large = 0;

	vo = CHeapFirstBlock (heap, &isFree);

	do {
		if (isFree) {
			int size;
			size = CHEAP_SIZE((CHeapAllocedBlock *)((char *)vo - offsetof(CHeapAllocedBlock, data)));
			free += size;
			if (size > large)
				large = size;
		}
		vo = CHeapNextBlock (heap, vo, &isFree);
	} while (vo);

	if (FreeMem)
		*FreeMem = free;
	if (LargestBlock)
		*LargestBlock = large;
}

/*
 * Initialise the memory subsystem
 */
void InitMemory (void)
{
	
	/*dLog("Initialising memory\n"); */
	
	GameHeap = GAMEHEAP_CREATE (GAMEHEAP_SIZE);
	dAssert (GameHeap != NULL, "Unable to allocate memory for GameHeap");
		
	ScratchHeap = SCRATCHHEAP_CREATE (SCRATCHHEAP_SIZE);
	dAssert (ScratchHeap != NULL, "Unable to allocate memory for ScratchHeap");
	
	/* STRAT VAR HEAPS */
	FloatVarHeap = FLOATVARHEAP_CREATE (FLOATVARHEAP_SIZE);
	dAssert (FloatVarHeap != NULL, "Unable to allocate memory for FloatVarHeap");
	IntVarHeap = INTVARHEAP_CREATE (INTVARHEAP_SIZE);
	dAssert (IntVarHeap != NULL, "Unable to allocate memory for IntVarHeap");
	ObjHeap = OBJHEAP_CREATE (OBJHEAP_SIZE);
	dAssert (ObjHeap != NULL, "Unable to allocate memory for ObjHeap");

	/* Sound heap */
	SoundHeap = SOUNDHEAP_CREATE (SOUNDHEAP_SIZE);
}
