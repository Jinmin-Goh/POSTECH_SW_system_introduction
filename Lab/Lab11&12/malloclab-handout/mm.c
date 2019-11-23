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

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
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
#define CHUNKSIZE   (1 << 6)   // extend heap by this amount
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

// Address of foreward node and backward node entry
#define FWDP(ptr) ((char *)(ptr) + WSIZE)
#define BWDP(ptr) ((char *)(ptr))

// Address of foreward node and backward node on the list 
#define FWDN(ptr) (*(char **)(FWDP(ptr)))
#define BWDN(ptr) (*(char **)(ptr))

// helper functions in textbook
static void *extend_heap(size_t words);
static void *coalesce(void *bp);
static void *place(void *bp, size_t asize);
static void insert_node(void *bp, size_t size);
static void delete_node(void *bp);
void *free_list[15];

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    char *heap_listp;
    int list;
    if((heap_listp = mem_sbrk(4 * WSIZE)) == (void *)-1)    // failed to allocate heap(out of memory)
        return -1;

    PUT(heap_listp, 0); // unuesd padding word
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1));  // prologue header
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1));  // prologue footer
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1));  // epilogue header
    heap_listp += (2 * WSIZE);
    
    for (list = 0; list < 15; list++)
        free_list[list] = NULL;

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
    size_t ssize;
    void *bp = NULL;
    int list = 0;

    // ignore spurious requests
    if(size == 0)
        return NULL;

    // adjust block size to include overhead and alignment reqs
    if(size <= DSIZE)
        asize = 2 * DSIZE;
    else
        asize = ALIGN(size + DSIZE);

    ssize = asize;
    // search the free list for a fit
    while(list < 15)
    {
        if((list == 14) || ((ssize <= 1) && free_list[list]))
        {
            bp = free_list[list];
            while (bp && (asize > GET_SIZE(HDRP(bp))))
                bp = BWDN(bp);
            
            if (bp)
                break;
        }
        
        ssize = ssize >> 1;
        list++;
    }

    // no free block found
    if(bp == NULL)
    {
        extendsize = MAX(asize, CHUNKSIZE);
        if((bp = extend_heap(extendsize)) == NULL)
            return NULL;
    }
    bp = place(bp, asize);
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
    insert_node(ptr, size);
    coalesce(ptr);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *newptr = ptr;
    size_t nsize;   // size of the new block
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
    
    // adjust block size to include overhead and alignment reqs
    if(size <= DSIZE)
        nsize = 2 * DSIZE;
    else
        nsize = ALIGN(size + DSIZE);

    blockbuf = GET_SIZE(HDRP(ptr)) - nsize;
    
    if(blockbuf < 0)   // not enough block
    {
        // check the next block is free
        if(!GET_ALLOC(HDRP(NEXT_BLKP(ptr))))
        {
            // calculate the scarce space amount
            remain = GET_SIZE(HDRP(ptr)) + GET_SIZE(HDRP(NEXT_BLKP(ptr))) - nsize;
            
            if(remain < 0)  // not enough block
            {
                extend = MAX(-remain, CHUNKSIZE);
                if(extend_heap(extend) == NULL)
                    return NULL;
                remain += extend;
            }
            delete_node(NEXT_BLKP(ptr));
            PUT(HDRP(ptr), PACK(nsize + remain, 1));
            PUT(FTRP(ptr), PACK(nsize + remain, 1));
        }
        else
        {
            newptr = mm_malloc(nsize - DSIZE);
            memcpy(newptr, ptr, MIN(size, nsize));
            mm_free(ptr);
        }
    }
    return newptr;
}

static void *extend_heap(size_t words)
{
    void *bp;
    size_t size;

    // Allocate size to maintain alignment
    size = ALIGN(words);
    if((bp = mem_sbrk(size)) == (void *)-1)
        return NULL;

    // Initialize free block header/footer and the epilogue header and list
    PUT(HDRP(bp), PACK(size, 0));   // free block header
    PUT(FTRP(bp), PACK(size, 0));   // free block footer
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));   // new epilogue header
    
    insert_node(bp, size);  // add node into free list

    // coalesce if the previous block was free
    return coalesce(bp);
}

static void *coalesce (void *bp) 
{
    size_t prev_alloc = GET_ALLOC(HDRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc)       // case 1 - both adjacent blocks are allocated
        return bp;

    else if (prev_alloc && !next_alloc) // case 2 - current block is merged with next block
    {
        delete_node(bp);   
        delete_node(NEXT_BLKP(bp));     
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }

    else if (!prev_alloc && next_alloc) // case 3 - previous block is merged with current block
    {     
        delete_node(bp);
        delete_node(PREV_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }

    else    // case 4 - current and both adjacent blocks are merged
    {
        delete_node(bp);
        delete_node(PREV_BLKP(bp));
        delete_node(NEXT_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    insert_node(bp, size); // insert new free block to list
    return bp;
}

static void *place(void *bp, size_t asize)
{
    size_t csize = GET_SIZE(HDRP(bp));

    delete_node(bp);

    if (csize - asize < DSIZE * 2)
    {
        PUT(HDRP(bp), PACK(csize, 1)); 
        PUT(FTRP(bp), PACK(csize, 1)); 
    }
    
    else
    {
        PUT(HDRP(bp), PACK(csize - asize, 0));
        PUT(FTRP(bp), PACK(csize - asize, 0));
        PUT(HDRP(NEXT_BLKP(bp)), PACK(asize, 1));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(asize, 1));
        insert_node(bp, csize - asize);
        return NEXT_BLKP(bp);        
    }
    return bp;
}


static void insert_node(void *ptr, size_t size)
{
    int list = 0;
    void *searchp, *insertp;
    insertp = NULL;
    

    while((list < 14) && (size > 1))
    {
        size = size >> 1;
        list++;
    }
    searchp = free_list[list];
    // finding appropriate position
    while(searchp && (size > GET_SIZE(HDRP(searchp))))
    {
        insertp = searchp;
        searchp = BWDN(searchp);
    }

    // inserting node
    if(searchp)     // in case of searchp is not NULL
    {
        if(insertp) // in case of insertp is not NULL -> middle of list
        {
            *(unsigned int *)BWDP(ptr) = (unsigned int)searchp;
            *(unsigned int *)FWDP(searchp) = (unsigned int)ptr;
            *(unsigned int *)FWDP(ptr) = (unsigned int)insertp;
            *(unsigned int *)BWDP(insertp) = (unsigned int)ptr;
        } 
        else    // in case of insertp is NULL -> first of list
        {
            *(unsigned int *)BWDP(ptr) = (unsigned int)searchp;
            *(unsigned int *)FWDP(searchp) = (unsigned int)ptr;
            *(unsigned int *)FWDP(ptr) = (unsigned int)NULL;
            free_list[list] = ptr;
        }
    }
    else    // in case of searchp is NULL
    {
        if (insertp) // in case of insertp is not NULL -> end of list
        {
            *(unsigned int *)BWDP(ptr) = (unsigned int)NULL;
            *(unsigned int *)FWDP(ptr) = (unsigned int)insertp;
            *(unsigned int *)BWDP(insertp) = (unsigned int)ptr;
        }
        else    // in case of insertp is NULL -> list was empty
        {
            *(unsigned int *)BWDP(ptr) = (unsigned int)NULL;
            *(unsigned int *)FWDP(ptr) = (unsigned int)NULL;
            free_list[list] = ptr;
        }
    }
    return;
}

static void delete_node(void *ptr)
{
    int list = 0;
    size_t size;
    
    size = GET_SIZE(HDRP(ptr));

    while ((list < 14) && (size > 1))
    {
        size = size >> 1;
        list++;
    }
    
    if (BWDN(ptr))  // in case of backward node is not NULL
    {
        if (FWDN(ptr))    // in case of backward node is not NULL -> delete middle node
        {
            *(unsigned int *)(FWDP(BWDN(ptr))) = (unsigned int)FWDN(ptr); // change backward node's foreward node address to current's foreward node
            *(unsigned int *)(BWDP(FWDN(ptr))) = (unsigned int)BWDN(ptr); // change foreward node's backward node address to current's backward node
        }
        else    // in case of foreward node is NULL -> delete first node
        {
            *(unsigned int *)(FWDP(BWDN(ptr))) = (unsigned int)NULL;
            free_list[list] = BWDN(ptr);
        }
    }
    else    // in case of backward node is NULL
    {
        if (FWDN(ptr))  // in case of foreward node is not NULL -> delete last node
            *(unsigned int *)(BWDP(FWDN(ptr))) = (unsigned int)NULL;
        else
            free_list[list] = NULL;
    }
    
    return;
}




