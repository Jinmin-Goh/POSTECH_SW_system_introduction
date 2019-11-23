/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

team_t team = {
    /* Team name */
    "20160074",
    /* First member's full name */
    "Goh Jinmin",
    /* First member's email address */
    "eric9709",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};


/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

// additional macro in textbook

// Basic constants
#define WSIZE       4           // word size
#define DSIZE       8           // double word size
#define CHUNKSIZE   (1 << 12)   // extend heap by this amount
#define MAX(x, y)   ((x) > (y) ? (x) : (y))
#define MIN(x, y)   ((x) < (y) ? (x) : (y)) 

// Pack a size and allocated bit into a word
#define PACK(size, alloc)   ((size) | (alloc))

// Read and write a word at address p 
#define GET(p)          (*(unsigned int *)(p))
#define PUT(p, val)     (*(unsigned int *)(p) = (val)) 

// Read the size and allocated fields from address p
#define GET_SIZE(p)     (GET(p) & ~0x7)
#define GET_ALLOC(p)    (GET(p) & 0x1)

// Given block ptr bp, compute address of its header and footer
#define HDRP(bp)    ((char *)(bp) - WSIZE)
#define FTRP(bp)    ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

// Given block ptr bp, compute address of next and previous blocks
#define NEXT_BLKP(bp)   ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp)   ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

// helper functions in textbook
static void *extend_heap(size_t words);
static void *coalesce(void *bp);
static void *find_fit(size_t asize);
static void place(void *bp, size_t asize);
static char *heap_listp = 0;

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    if((heap_listp = mem_sbrk(4 * WSIZE)) == (void *)-1)    // failed to allocate heap(out of memory)
        return -1;

    PUT(heap_listp, 0); // unuesd padding word
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1));  // prologue header
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1));  // prologue footer
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1));  // epilogue header
    heap_listp += (2 * WSIZE);

    // Extend the empty heap with a free block of CHUNKSIZE bytes
    if(extend_heap(CHUNKSIZE) == NULL)
        return -1;    
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t asize;       // adjusted block size
    size_t extendsize;  // amount to extend eap if no fit
    void *bp = NULL;
    
    // ignore spurious requests
    if(size == 0)
        return NULL;

    // adjust block size to include overhead and alignment reqs
    if(size <= DSIZE)
        asize = 2 * DSIZE;
    else
        asize = ALIGN(size + DSIZE);

    // search the free list for a fit
    if((bp = find_fit(asize)))
    {
        place(bp, asize);
        return bp;
    }

    // no fit found
    extendsize = MAX(asize, CHUNKSIZE);
    if((bp = extend_heap(extendsize / WSIZE)) == NULL)
        return NULL;    
    place(bp, asize);
    return bp;

}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HDRP(ptr));

    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));
    coalesce(ptr);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    size_t newsize = size;   // size of the new block
    int remain;     // the remain size after allocation
    int extend;     // size of heap extension 
    int blockbuf;

    // ptr is NULL -> malloc
    if(ptr == NULL)
        return mm_malloc(size);

    // size is 0 -> free
    if(size == 0)
    {
        mm_free(ptr);
        return NULL;
    }
    
    // align block size
    if(size <= DSIZE)
        newsize = 2 * DSIZE;
    else
        newsize = ALIGN(size + DSIZE);

    blockbuf = GET_SIZE(HDRP(ptr)) - newsize;
    
    if(blockbuf < 0)   // not enough block
    {
        // check the next block is free
        if(!GET_ALLOC(HDRP(NEXT_BLKP(ptr))))
        {
            // calculate the scarce space amount
            remain = GET_SIZE(HDRP(ptr)) + GET_SIZE(HDRP(NEXT_BLKP(ptr))) - newsize;
            
            if(remain < 0)  // not enough block
            {
                extend = MAX(-remain, CHUNKSIZE);
                if(extend_heap(extend) == NULL)
                    return NULL;
                remain += extend;
            }
            PUT(HDRP(ptr), PACK(newsize + remain, 1));
            PUT(FTRP(ptr), PACK(newsize + remain, 1));
        }
        else    // enough space
        {
            oldptr = mm_malloc(newsize - DSIZE);
            memcpy(oldptr, ptr, MIN(size, newsize));
            mm_free(ptr);
        }
    }
    return oldptr;
}

static void *extend_heap(size_t words)
{
    void *bp;
    size_t size;

    // Allocate an even number of words to maintain alignment
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    if((long)(bp = mem_sbrk(size)) == -1)
        return NULL;

    // Initialize free block header/footer and the epilogue header and list
    PUT(HDRP(bp), PACK(size, 0));   // free block header
    PUT(FTRP(bp), PACK(size, 0));   // free block footer
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));   // new epilogue header
    
    // coalesce if the previous block was free
    return coalesce(bp);
}

static void *coalesce (void *bp) 
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc)       // case 1
        return bp;

    else if (prev_alloc && !next_alloc) // case 2
    {
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }

    else if (!prev_alloc && next_alloc) // case 3
    {     
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }

    else    // case 4
    {
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    return bp;
}


static void *find_fit(size_t asize)
{
    // best-fit search
    void *bp, *tempp;

    tempp = NULL;

    for(bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
    {
        // exactly same size of empty block
        if(!GET_ALLOC(HDRP(bp)) && (asize == GET_SIZE(HDRP(bp))))
            return bp;
        else if(!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp))))
        {   
            if(tempp == NULL || (GET_SIZE(HDRP(bp)) < GET_SIZE(HDRP(tempp))))
                tempp = bp;
        }
    }
    return tempp;
}

static void place(void *bp, size_t asize)
{
    size_t csize = GET_SIZE(HDRP(bp));

    if((csize - asize) >= (2 * DSIZE))
    {
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(csize - asize, 0));
        PUT(FTRP(bp), PACK(csize - asize, 0));
    }
    else
    {
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
    }
}












