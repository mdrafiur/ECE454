/*
 * This implementation replicates the implicit list implementation
 * provided in the textbook
 * "Computer Systems - A Programmer's Perspective"
 * Blocks are never coalesced or reused.
 * Realloc is implemented directly using mm_malloc and mm_free.
 *
 * For our implementation we used a segregated, explicit double linked free list
 * That separates through the power of two, and sorted by logarithm
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Eclipse */
    "Eclipse",
    /* Md Rafiur Rashid */
    "Md Rafiur Rashid",
    /* rafiur.rashid@mail.utoronto.ca */
    "rafiur.rashid@mail.utoronto.ca",
    /* Second member's full name (leave blank if none) */
    "Xuan Fang",
    /* Second member's email address (leave blank if none) */
    "benny.fang@mail.utoronto.ca"
};

/*************************************************************************
 * Basic Constants and Macros
 * You are not required to use these macros but may find them helpful.
 *************************************************************************/
#define WSIZE       sizeof(void *)            /* word size (bytes) */
#define DSIZE       (2 * WSIZE)            /* doubleword size (bytes) */
#define CHUNKSIZE   (1<<7)      /* initial heap size (bytes) */

#define MAX(x,y) ((x) > (y)?(x) :(y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p)          (*(uintptr_t *)(p))
#define PUT(p,val)      (*(uintptr_t *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)     (GET(p) & ~(DSIZE - 1))
#define GET_ALLOC(p)    (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)        ((char *)(bp) - WSIZE)
#define FTRP(bp)        ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

/* Define the previous location and successor location of free list */

#define PRED(bp) (*(void **)(bp))
#define SUCC(bp) (*(void **)(bp) + WSIZE)

/* Get an address of a pointer of pred of succ */
#define GET_PRED(bp) (*(uintptr_t *)(bp))
#define GET_SUCC(bp) (*(uintptr_t *)( (char *)(bp) + WSIZE) )

/* Put an address of a pointer into another as pred or succ pointer */
#define PUT_PRED(p1, p2) (*(uintptr_t *)p1 = (uintptr_t)(p2))
#define PUT_SUCC(p1, p2) (*(uintptr_t *)((char *)(p1) + WSIZE) = (uintptr_t)p2)

void* heap_listp = NULL;
/* Define prologue and epilogue pointer for testing */
void* prologue = NULL;
void* epilogue = NULL;

void add_front_of_free_list(void *bp);
void remove_from_free_list(void* bp);

/* Define constant such as seg list size, min block size and log of that */
#define SEG_LIST_SIZE 8
#define MIN_BLOCK_SIZE (2 * DSIZE)
#define LOG2_OF_MIN_BLOCK_SIZE 5

/* Declare seg_list as free list */
static void *SEGREGATED_LIST[SEG_LIST_SIZE] = {NULL};

const int tab64[64] = {
    63,  0, 58,  1, 59, 47, 53,  2,
    60, 39, 48, 27, 54, 33, 42,  3,
    61, 51, 37, 40, 49, 18, 28, 20,
    55, 30, 34, 11, 43, 14, 22,  4,
    62, 57, 46, 52, 38, 26, 32, 41,
    50, 36, 17, 19, 29, 10, 13, 21,
    56, 45, 25, 31, 35, 16,  9, 12,
    44, 24, 15,  8, 23,  7,  6,  5};

int logBase2 (uint64_t value)
{
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    value |= value >> 32;
    return tab64[((uint64_t)((value - (value >> 1))*0x07EDD5E59A4E28C2)) >> 58];
}

/**********************************************************
 * mm_init
 * Initialize the heap, including "allocation" of the
 * prologue and epilogue
 **********************************************************/
int mm_init(void)
{
    if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1)
        return -1;
    PUT(heap_listp, 0);                         // alignment padding
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1));   // prologue header
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1));   // prologue footer
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1));    // epilogue header
    prologue = (void*) ((char*)heap_listp + WSIZE);	//define prologue pointer
    epilogue = (void*) ((char*)heap_listp + 3 * WSIZE); //define epilogue pointer
    heap_listp += DSIZE;
    return 0;
}

/**********************************************************
 * coalesce
 * Covers the 4 cases discussed in the text:
 * - both neighbours are allocated
 * - the next block is available for coalescing
 * - the previous block is available for coalescing
 * - both neighbours are available for coalescing
 **********************************************************/
void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) {       /* Case 1 */
    	add_front_of_free_list(bp);
        return bp;
    }

    else if (prev_alloc && !next_alloc) { /* Case 2 */
    	remove_from_free_list(SUCC(bp));
    	size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
        add_front_of_free_list(bp);
        return (bp);
    }

    else if (!prev_alloc && next_alloc) { /* Case 3 */
    	remove_from_free_list(PREV_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        add_front_of_free_list(PREV_BLKP(bp));
        return (PREV_BLKP(bp));
    }

    else {            /* Case 4 */
    	remove_from_free_list(NEXT_BLKP(bp));
    	remove_from_free_list(PREV_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp)))  +
            GET_SIZE(FTRP(NEXT_BLKP(bp)))  ;
        PUT(HDRP(PREV_BLKP(bp)), PACK(size,0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size,0));
        add_front_of_free_list(PREV_BLKP(bp));
        return (PREV_BLKP(bp));
    }
}

/**********************************************************
 * extend_heap
 * Extend the heap by "words" words, maintaining alignment
 * requirements of course. Free the former epilogue block
 * and reallocate its new header
 **********************************************************/
void *extend_heap(size_t words)
{
    char *bp;
    size_t size;

    /* Allocate an even number of words to maintain alignments */
    size = (words % 2) ? (words+1) * WSIZE : words * WSIZE;
    if ( (bp = mem_sbrk(size)) == (void *)-1 )
        return NULL;

    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0));                // free block header
    PUT(FTRP(bp), PACK(size, 0));                // free block footer
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));        // new epilogue header

    epilogue = (void*) NEXT_BLKP(bp); //redefine epilogue pointer

    /* Coalesce if the previous block was free */
    return coalesce(bp);
}

int get_seg_list_index(size_t block_size)
{
    size_t size = --block_size;
    size |= size >> 1;
    size |= size >> 2;
    size |= size >> 4;
    size |= size >> 8;
    size |= size >> 16;
    size++;

    int exponent = logBase2(size);

    if( exponent < (SEG_LIST_SIZE + LOG2_OF_MIN_BLOCK_SIZE) )
        return (block_size < size) ? (exponent - LOG2_OF_MIN_BLOCK_SIZE - 1) : (exponent - LOG2_OF_MIN_BLOCK_SIZE);
    else
        return (SEG_LIST_SIZE - 1);
}

void add_front_of_free_list(void *bp)
{
    size_t block_size = GET_SIZE(HDRP(bp));
    int index = get_seg_list_index(block_size);

    if(SEGREGATED_LIST[index] != NULL)
    {
        void *current = SEGREGATED_LIST[index];
        PUT_SUCC(bp, current);
        PUT_PRED(current, bp);
        PUT_PRED(bp, 0);
        SEGREGATED_LIST[index] = bp;
    }
    else
    {
    	PUT_SUCC(bp, 0);
    	PUT_PRED(bp, 0);
        SEGREGATED_LIST[index] = bp;
    }
}

void remove_from_free_list(void* bp)
{
    size_t block_size = GET_SIZE(HDRP(bp));
    int index = get_seg_list_index(block_size);

    if(SEGREGATED_LIST[index] == NULL)
        return;

    if (SEGREGATED_LIST[index] == bp)
    {
    	if (GET_SUCC(bp) != 0) {
    		//printf ("Successor is %lx\n", GET_SUCC(bp));
			PUT_PRED(SUCC(bp), 0);
    		SEGREGATED_LIST[index] = SUCC(bp);
    	}
    	else SEGREGATED_LIST[index] = NULL;
        return;
    }

    else {
    	if (GET_PRED(bp) == 0) {
			PUT_SUCC(PRED(bp), SUCC(bp));
		}

		if (GET_SUCC(bp) == 0) {
			PUT_PRED(SUCC(bp), PRED(bp));
		}
	}
    return;
}

/**********************************************************
 * find_fit
 * Traverse the heap searching for a block to fit asize
 * Return NULL if no free blocks can handle that size
 * Assumed that asize is aligned
 **********************************************************/
void * find_fit(size_t asize)
{
    assert(asize >= MIN_BLOCK_SIZE);
    assert(asize % DSIZE == 0);

    int i;
    int seg_list_index = get_seg_list_index(asize);

    for(i = seg_list_index; i < SEG_LIST_SIZE; i++)
    {
        void *current = SEGREGATED_LIST[i];
        while(current && GET_SIZE(HDRP(current)) < asize) current = SUCC(current);

        if(current)
        {
        	//do not do remove here but instead in the place function
            //remove_from_free_list(current);
            return current;
        }
    }
    return NULL;
}

void split(void *bp, size_t asize)
{
    size_t remaining_size = GET_SIZE(HDRP(bp)) - asize;
    void *split_block = (void*)((char*)(bp) + asize);

    remove_from_free_list(bp);

    PUT(HDRP(bp), PACK(asize, 1));
    PUT(FTRP(bp), PACK(asize, 1));

    PUT(HDRP(split_block), PACK(remaining_size, 0));
    PUT(FTRP(split_block), PACK(remaining_size, 0));

    add_front_of_free_list(split_block);
}

/**********************************************************
 * place
 * Mark the block as allocated
 **********************************************************/
void place(void* bp, size_t asize)
{
    /* Get the current block size */
    size_t bsize = GET_SIZE(HDRP(bp));

    if(bsize >= (asize + MIN_BLOCK_SIZE))
        split(bp, asize);

    else
    {
        remove_from_free_list(bp);
        PUT(HDRP(bp), PACK(bsize, 1));
        PUT(FTRP(bp), PACK(bsize, 1));
    }
}

/**********************************************************
 * mm_free
 * Free the block and coalesce with neighbouring blocks
 **********************************************************/
void mm_free(void *bp)
{
    if(bp == NULL){
        return;
    }
    size_t size = GET_SIZE(HDRP(bp));
    PUT(HDRP(bp), PACK(size,0));
    PUT(FTRP(bp), PACK(size,0));
    coalesce(bp);
}


/**********************************************************
 * mm_malloc
 * Allocate a block of size bytes.
 * The type of search is determined by find_fit
 * The decision of splitting the block, or not is determined
 *   in place(..)
 * If no block satisfies the request, the heap is extended
 **********************************************************/
void *mm_malloc(size_t size)
{
    size_t asize; /* adjusted block size */
    size_t extendsize; /* amount to extend heap if no fit */
    char * bp;

    /* Ignore spurious requests */
    if (size == 0)
        return NULL;

    /* Adjust block size to include overhead and alignment reqs. */
    if (size <= DSIZE)
        asize = 2 * DSIZE;
    else
        asize = DSIZE * ((size + (DSIZE) + (DSIZE-1))/ DSIZE);

    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
        return bp;
    }

    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
        return NULL;
    place(bp, asize);
    return bp;

}

/**********************************************************
 * mm_realloc
 * Implemented simply in terms of mm_malloc and mm_free
 *********************************************************/
void *mm_realloc(void *ptr, size_t size)
{
    /* If size == 0 then this is just free, and we return NULL. */
    if(size == 0){
        mm_free(ptr);
        return NULL;
    }
    /* If oldptr is NULL, then this is just malloc. */
    if (ptr == NULL)
        return (mm_malloc(size));

    void *oldptr = ptr;
    void *newptr;
    size_t copySize;

    newptr = mm_malloc(size);
    if (newptr == NULL)
        return NULL;

    /* Copy the old data. */
    copySize = GET_SIZE(HDRP(oldptr));
    if (size < copySize)
        copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}

/**********************************************************
 * mm_check
 * Check the consistency of the memory heap
 * Return nonzero if the heap is consistent.
 * Checks:
 * -consistency of the header and footer
 * -validity of prologue and epilogue when traversing through the heap
 * -if there is non-coalesced blocks
 *********************************************************/
int mm_check(void){
	void* bp;
	for (bp = prologue; bp < epilogue; bp = NEXT_BLKP(bp)) {
		//check that the next block is not free
		if(!GET_ALLOC(bp) && !GET_ALLOC(NEXT_BLKP(bp) ) ){
			printf("There are non-coalesced blocks!\n");
			return -1;
		}
		//check if the size specified in header and footer are consistent
		if (GET_SIZE(HDRP(bp)) != GET_SIZE(FTRP(bp))){
			printf("Heap is not consistent.\n");
			return -1;
		}
	}
	//check if the heap ended in epilogue
	if(bp != epilogue){
		printf("The heap didn't end in epilogue.\n");
		return -1;
	}
	while(bp > prologue) bp = PREV_BLKP(bp);
	//check if the heap began in prologue
	if(bp != prologue){
		printf("The heap didn't begin in prologue.\n");
		return -1;
	}
    return 0;
}
