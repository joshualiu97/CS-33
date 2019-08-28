/*
 * This allocator is implemented with an explicit free list that is a doubly linked list.
 * The heap itself has an prologue header and footer (of total size 8), and a epilogue header of
 * size 4. We are using sizeof(void *) to determine if the program needs to deal with 8-byte pointers (64-bit)
 * or 4-byte pointers (32-bit). In the Makefile for this program, we are compiling with the 32-bit flag, which
 * means that the header and footers are 4 bytes long, and pointers take up 4 bytes. The explicit free list has
 * a head pointer called start_free_list that points to the start of the free list. If there are no items in the
 * free list, it is set to NULL. The free blocks all have two pointers in the payload. The first pointer is
 * a pointer to the previous free block, and the second pointer is a pointer to the next free block. The first
 * free block in the list has its previous pointer set to NULL, and the last free block in the list has its next
 * pointer set to NULL.
 *  
 * The functions are all explained below, before function is implemented.
 * The heap checker function mm_check() is implemented at the bottom of the file.
 *
 * Results:
 * Results for mm malloc:
 * trace  valid  util     ops      secs  Kops
 *  0       yes   99%    5694  0.000543 10492
 *  1       yes  100%    5848  0.000592  9887
 *  2       yes   99%    6648  0.000692  9607
 *  3       yes  100%    5380  0.000522 10299
 *  4       yes   99%   14400  0.000930 15489
 *  5       yes   96%    4800  0.004492  1069
 *  6       yes   95%    4800  0.004182  1148
 *  7       yes   55%   12000  0.036512   329
 *  8       yes   50%   24000  0.131931   182
 *  9       yes   54%   14401  0.002093  6880
 * 10       yes   45%   14401  0.002037  7071
 * Total          81%  112372  0.184525   609
 * 
 * Perf index = 49 (util) + 40 (thru) = 89/100
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
 * provide your information in the following struct.
 ********************************************************/
team_t team = {
    /* Your UID */
    "105136031",
    /* Your full name */
    "Joshua Liu",
    /* Your email address */
    "joshualiu97@ucla.edu",
    /* Leave blank */
    "",
    /* Leave blank */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* defines constants and values to be used throughout the program */
#define WSIZE (sizeof(void *))				/* Word and header/footer size (bytes) */ 
#define DSIZE (WSIZE * 2)                   /* Double word size (bytes) */ 
#define INITCHUNKSIZE (1<<4)				/* Initially extend heap by this amount (bytes) */
#define CHUNKSIZE (1<<4)                    /* Extend heap by this amount (bytes) */ 

/* determines the max or min, given two values */
#define MAX(x, y) ((x) > (y) ? (x) : (y))   /* Finds max of two numbers */
#define MIN(x, y) ((x) > (y) ? (y) : (x))   /* Finds min of two numbers */

/* Puts the block size and allocation bit together */
#define PACK(size, alloc) ((size) | (alloc))    /* Pack a size and allocated bit into a word */

/* Read and write a word at address p */	
#define GET(p) (*(unsigned int *)(p))					/* Gets value at p */
#define PUT(p, val) (*(unsigned int *)(p) = (val))		/* Puts value into p */

/* Read the size and allocated fields from address p */
#define GET_SIZE(p) (GET(p) & ~0x7)		/* Get size from header/footer */
#define GET_ALLOC(p) (GET(p) & 0x1)		/* Get allocation bit from header/footer */

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp) ((char *)(bp) - WSIZE)							/* Get the header block */
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)	/* Get the footer block */

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))		/* Get the next block pointer */
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE))) 	/* Get the previous block pointer */

/* Given block ptr bp, compute the address of the next and previous pointers */
#define GET_NEXT_PTR(bp) (*(char **)(bp + WSIZE))		/* Gets the next pointer in the free list */
#define GET_PREV_PTR(bp) (*(char **)(bp))				/* Get the previous pointer in the free list */

/* Set value of pointers in free list, op is other pointer */
#define SET_NEXT_PTR(bp, op) (GET_NEXT_PTR(bp) = op)		/* Sets the next pointer to other pointer (op) */
#define SET_PREV_PTR(bp, op) (GET_PREV_PTR(bp) = op)		/* Sets the previous pointer to other pointer (op) */

/* Global variable that points to first block(prologue block) */
static char *heap_listp;		/* Points to the start of the heap (specifically to the prologue block */
static char *start_free_list;	/* Points to the start of the explicit free list */

/* Function declarations */
static void *coalesce(void *bp);                /* Coalesces free blocks */
static void *extend_heap(size_t words);         /* Increases heap size */
static void *find_fit(size_t asize);            /* Finds a fit in the heap when allocating memory */
static void place(void *bp, size_t asize);		/* Marks a block of memory as allocated */
static void insert_free_block(void *bp);		/* Insert a free block from free list */
static void remove_free_block(void *bp);		/* Remove a free block from free list */
int mm_check(void);                             /* Checks consistency of heap */
void show_heap(void);							/* Prints out the heap, used for debugging */

/* 
 * mm_init - initialize the malloc package. Create a prologue and epilogue block 
 * and extend the size of the heap.
 */
int mm_init(void)
{
	//printf("init\n");
	
    /* Create the initial empty heap with alignment padding, prologue header, prologue footer, and epilogue header */
    if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1)
        return -1;
    PUT(heap_listp, 0);                             /* Alignment padding */
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1));    /* Prologue header */
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1));    /* Prologue footer */
    PUT(heap_listp + (3*WSIZE), PACK(0, 1));        /* Epilogue header */
    heap_listp += (2*WSIZE);						/* Points to the prologue block */
    start_free_list = NULL;							/* Set to NULL until a free block is inserted */

    /* Extend the empty heap with a free block of INITCHUNKSIZE bytes */
    if (extend_heap(INITCHUNKSIZE/WSIZE) == NULL)
        return -1;

    //mm_check();
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 * The size is adjusted so that it remains aligned to 8 bytes. The find_fit and place functions
 * are called to determine the best fit location and to place the block accordingly into the heap.
 */
void *mm_malloc(size_t size)
{
    //printf("malloc\n");

    size_t asize;          /* Adjusted block size */
    size_t extendsize;     /* Amount to extend heap if no fit */
    char *bp;

    /* Ignore size = 0 */
    if (size == 0)
	    return NULL;

    /* Adjust block size to include overhead and alignment requirements */
    if (size <= DSIZE)
        asize = 2*DSIZE;
    else
        asize = DSIZE * ((size + (DSIZE) + (DSIZE-1)) / DSIZE);

    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
		//show_heap();
		//mm_check();
        return bp;
    }

    /* Did not find fit, get more memory and place the block */
    extendsize = MAX(asize,CHUNKSIZE);
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
        return NULL;
    place(bp, asize);

    //mm_check();
    //show_heap();
    return bp;
}

/*
 * mm_free - Get the size of the block that ptr points to and set the headers and footers
 * to zero, which "frees" the block. Coalesce the free block after.
 */
void mm_free(void *ptr)
{
    //printf("free\n");

    if (ptr == NULL)
        return;
    
    size_t size = GET_SIZE(HDRP(ptr));
    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));
    coalesce(ptr);
    //show_heap();
    //mm_check();
    return;
}

/*
 * mm_realloc - Reallocates a block in the heap given a pointer. First, the inital cases are
 * taken into account. Then, we get all the sizes that we may check to see if the new block
 * fits within the vicinity of the old block.
 * 
 * 1. check if the new size is smaller
 * 2. check if the next block is available and if the new block fits in this place
 * 3. check if the previous block is available and if the new block fits in this place
 * 4. check if the previous and next block are available and if the new block fits in this place
 * 
 * If none of these conditions are met, then a new block is simply created, the data contents of the
 * old one are copied over, and the old block is freed.
 */
void *mm_realloc(void *ptr, size_t size)
{
	//printf("realloc\n");
	//mm_check();
	
	if (ptr == NULL) {              /* If ptr is NULL, call malloc(size) */
		//show_heap();
        return mm_malloc(size);
    }
    if (size == 0) {                /* If size == 0, return NULL */
        mm_free(ptr);
        //show_heap();
        return NULL;
    }
    
    /* Check for invalid ptr where it points to a free block */
    if (GET_ALLOC(HDRP(ptr)) == 0) {
    	//show_heap();
        return NULL;
    }

    /* Get information to check */
    size_t old_size = GET_SIZE(HDRP(ptr));                      /* Original size */
    size_t new_size = 0;                                        /* New size */
    if (size <= DSIZE)
        new_size = 2*DSIZE;
    else
        new_size = DSIZE * ((size + (DSIZE) + (DSIZE-1)) / DSIZE);
    size_t min_size = MIN(size, old_size);                  	/* Minimum of original and size given */
    size_t block_before = GET_SIZE(HDRP(PREV_BLKP(ptr)));       /* Size of block before */
    size_t valid_before = GET_ALLOC(HDRP(PREV_BLKP(ptr)));   	/* See if block before is free */
    size_t block_after = GET_SIZE(HDRP(NEXT_BLKP(ptr)));        /* Size of block after */
    size_t valid_after = GET_ALLOC(HDRP(NEXT_BLKP(ptr)));  	    /* See if block after is free */
    size_t sumAround = old_size + block_before + block_after;   /* Sum of all three blocks */
    char *data = (char *)(ptr);

    if (new_size == old_size) {		/* Same size passed in, do nothing */
    	//show_heap();
        return ptr;
    }
    else if ((new_size <= sumAround) && (!valid_before) && (!valid_after)) {  /* If block fits in vicinity (before and after), place it there */
        remove_free_block(NEXT_BLKP(ptr));          /* Remove free blocks from free list */
        remove_free_block(PREV_BLKP(ptr));
        ptr = PREV_BLKP(ptr);
        char *new_loc = (char *)(ptr);

        PUT(HDRP(ptr), PACK((sumAround), 0));       /* Create new header and footer that are free */
        PUT(FTRP(ptr), PACK((sumAround), 0));
        int iter = 0;                               /* Copy the contents of the data over */
        for (iter = 0; iter < min_size; iter++)
            new_loc[iter] = data[iter];

        if ((sumAround-new_size) >= (2*DSIZE)) {    /* Adjust the size so that every block is at least 16 bytes long */
            PUT(HDRP(ptr), PACK(new_size, 1));
            PUT(FTRP(ptr), PACK(new_size, 1));
            PUT(HDRP(NEXT_BLKP(ptr)), PACK((sumAround-new_size), 0));
            PUT(FTRP(NEXT_BLKP(ptr)), PACK((sumAround-new_size), 0));
            coalesce(NEXT_BLKP(ptr));               /* Coalesce free block */
        }
        else {
            PUT(HDRP(ptr), PACK(sumAround, 1));
            PUT(FTRP(ptr), PACK(sumAround, 1));
        }
        //mm_check();
        return ptr;
    }
    // else if ((GET_SIZE(HDRP(NEXT_BLKP(ptr))) == 0) && ((GET_ALLOC(HDRP(NEXT_BLKP(ptr)))) == 1)) {
    //     size_t extendsize = MIN(new_size,CHUNKSIZE);
    //     void* bp;
    //     if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
    //         return NULL;
    //     return mm_realloc(ptr, size);
    // }
    else if (new_size < old_size) {		/* If new size is smaller, make block smaller */
    	PUT(HDRP(ptr), PACK(old_size, 0));
    	PUT(FTRP(ptr), PACK(old_size, 0));
    	insert_free_block(ptr);
    	place(ptr, new_size);
    	//mm_check();
    	return ptr;
    }
    else if ((new_size <= (old_size + block_after)) && (!valid_after)) {     /* If block after is free, then utilize */
        remove_free_block(NEXT_BLKP(ptr));					/* Remove free blocks from free list */
        PUT(HDRP(ptr), PACK((old_size+block_after), 0));	/* Set the block to a free block */
    	PUT(FTRP(ptr), PACK((old_size+block_after), 0));
    	insert_free_block(ptr);								/* Insert the free block */
    	place(ptr, new_size);								/* Place the block */
    	//mm_check();
    	return ptr;
    }
    else if ((new_size <= (old_size + block_before)) && (!valid_before)) {   /* If block before is free, then utilize */
        ptr = PREV_BLKP(ptr);
        char *new_loc = (char *)(ptr);
        remove_free_block(ptr);								/* Remove free block */
        PUT(HDRP(ptr), PACK((old_size+block_before), 0));
    	PUT(FTRP(ptr), PACK((old_size+block_before), 0));
    	int iter = 0;										/* Copy data over */
        for (iter = 0; iter < min_size; iter++)
            new_loc[iter] = data[iter];

	    if (((old_size+block_before)-new_size) >= (2*DSIZE)) {		/* Place block and coalesce */
	        PUT(HDRP(ptr), PACK(new_size, 1));
	        PUT(FTRP(ptr), PACK(new_size, 1));
	        PUT(HDRP(NEXT_BLKP(ptr)), PACK(((old_size+block_before)-new_size), 0));
	        PUT(FTRP(NEXT_BLKP(ptr)), PACK(((old_size+block_before)-new_size), 0));
	        coalesce(NEXT_BLKP(ptr));
	    }
	    else {
	        PUT(HDRP(ptr), PACK((old_size+block_before), 1));
	        PUT(FTRP(ptr), PACK((old_size+block_before), 1));
	    }
    	//mm_check();
    	return ptr;
    }
    else {		/* If new block doesn't fit before or after, then create a new block, copy data over, and free the old block */ 
        /* Allocate block of new size */
        void* newptr = mm_malloc(new_size);
        memcpy(newptr, ptr, min_size);

        /* Free block that ptr points to */
        mm_free(ptr);

        //show_heap();
        return newptr;
    }
    //show_heap();
	return NULL;
}



/* Function implementations */
/*
 * Extends the heap given a size. Coalesce the new free block to combine the previous block
 * if the previous block was free.
 */
static void *extend_heap(size_t words)
{
    //printf("extend heap\n");

    char *bp;
    size_t size;

    /* Allocate an even number of words to maintain alignment */
    size = (words % 2) ? (words+1) * WSIZE : words * WSIZE;
    if ((long)(bp = mem_sbrk(size)) == -1)
        return NULL;

    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0));               /* Free block header */
    PUT(FTRP(bp), PACK(size, 0));               /* Free block footer */
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));       /* New epilogue header */

    /* Coalesce if the previous block was free */
    return coalesce(bp);
}

/*
 * Coalesce the block with 4 cases.
 * 1. The blocks before and after are allocated, so cannot coalesce.
 * 2. The block after is a free block.
 * 3. The block before is a free block.
 * 4. The blocks before and after are free.
 *
 * Through the coalescing processing, blocks that are removed (coalesced) and removed from
 * the free list, and blocks that are added are inserted to the free list.
 */
static void *coalesce(void *bp)
{
    //printf("coalesce\n");

    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) {                     /* Case 1 */
        insert_free_block(bp);
        return bp;
    }
    else if (prev_alloc && !next_alloc) {               /* Case 2 */
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        remove_free_block(NEXT_BLKP(bp));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }
    else if (!prev_alloc && next_alloc) {               /* Case 3 */
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        bp = PREV_BLKP(bp);
        remove_free_block(bp);
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }
    else {                                              /* Case 4 */
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
        remove_free_block(PREV_BLKP(bp));
        remove_free_block(NEXT_BLKP(bp));
        bp = PREV_BLKP(bp);
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }
    insert_free_block(bp);
    return bp;
}

/*
 * The find_fit function finds the best fit for the blocks, choosing the block that fits the smallest
 * block possible in order to minimize fragmentation. Other types of find_fit are tested but do not
 * yield great results.
 */
static void *find_fit(size_t asize)
{
	//printf("find fit\n");

    /* Best-fit search */
    void *bp;
    void *best_ptr = NULL;
    size_t smallest = 0;
    size_t curr_size;
    for (bp = start_free_list; bp != NULL; bp = GET_NEXT_PTR(bp)) {
        if (GET_ALLOC(HDRP(bp)) == 0) {				/* If bp points to free block */
            curr_size = GET_SIZE(HDRP(bp));
            if (asize <= curr_size) {				/* If block has enough space */
                if (smallest == 0) {				/* Base condition */
                    smallest = curr_size;
                    best_ptr = bp;
                }
                else if (curr_size < smallest) {	/* If smaller one, then set as best_ptr */
                    smallest = curr_size;
                    best_ptr = bp;
                }
            }
        }
    }
    return best_ptr;

    /* First-fit search */
    // void *bp;
    // for (bp = start_free_list; bp != NULL; bp = GET_NEXT_PTR(bp)) {
    //     if (GET_ALLOC(HDRP(bp)) == 0) {             /* If bp points to free block */
    //         size_t curr_size = GET_SIZE(HDRP(bp));
    //         if (asize <= curr_size) {               /* If block has enough space */
    //             return bp;
    //         }
    //     }
    // }
    // return NULL;

    /* Next-fit search */
    // void *bp;
    // size_t counter = 0;
    // for (bp = start_free_list; bp != NULL; bp = GET_NEXT_PTR(bp)) {
    //     if (GET_ALLOC(HDRP(bp)) == 0) {             /* If bp points to free block */
    //         size_t curr_size = GET_SIZE(HDRP(bp));
    //         if ((asize <= curr_size) && counter) {               /* If block has enough space */
    //             return bp;
    //         }
    //         counter += 1;
    //     }
    // }
    // return NULL;

    /* Finds last one in list */
    // void *bp;
    // void *myptr = NULL;
    // for (bp = start_free_list; bp != NULL; bp = GET_NEXT_PTR(bp)) {
    //     if (GET_ALLOC(HDRP(bp)) == 0) {             /* If bp points to free block */
    //         size_t curr_size = GET_SIZE(HDRP(bp));
    //         if (asize <= curr_size) {               /* If block has enough space */
    //             myptr = bp;
    //         }
    //     }
    // }
    // return myptr;
}
/*
 * Given a pointer and the size that we need to allocate, the function determines if the
 * difference between the block size at this position and the given size is greater than
 * the minimum block size (16 bytes). If it is, then make the rest a free block. If not,
 * then the whole block is allocated. This reduces fragmentation.
 */ 
static void place(void *bp, size_t asize)
{
    //printf("placing\n");

    size_t csize = GET_SIZE(HDRP(bp));		/* Get size of block */

    if ((csize-asize) >= (2*DSIZE)) {		/* Block size difference fits in a new (free) block */ 
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        remove_free_block(bp);
        PUT(HDRP(NEXT_BLKP(bp)), PACK(csize-asize, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(csize-asize, 0));
        coalesce(NEXT_BLKP(bp));
    }
    else {									/* Block size difference is too small and is placed in the whole block */
        remove_free_block(bp);
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
    }
    return;
}

/*
 * Inserts free block into free list in two cases. The block is inserted into the front.
 *
 * 1. The free list is free.
 * 2. The free list is not free.
 */
static void insert_free_block(void *bp)
{
	if (start_free_list == NULL) {		/* If there are no items in free list, set the block to first element */
        SET_NEXT_PTR(bp, NULL);
        SET_PREV_PTR(bp, NULL);
        start_free_list = bp;
        return;
    }
    else {		/* Inserting to front of list. start_free_list is beginning of current list. */
	    SET_NEXT_PTR(bp, start_free_list);
	    SET_PREV_PTR(start_free_list, bp);
	    SET_PREV_PTR(bp, NULL);
	    start_free_list = bp;
	    return;
	}
	
 //	/* Code to insert based on address. Does not perform as well as the previous method */
 //    if (start_free_list == NULL) {		/* If there are no items in free list, set the block to first element */
 //        SET_NEXT_PTR(bp, NULL);
 //        SET_PREV_PTR(bp, NULL);
 //        start_free_list = bp;
 //        return;
 //    }
    
 //    void *traverse = start_free_list;
 //    while ((GET_NEXT_PTR(traverse) != NULL) && (bp > traverse))		/* Go to memory addres in ascending order */
 //    	traverse = GET_NEXT_PTR(traverse);

 //    if ((GET_NEXT_PTR(traverse) == NULL) && (bp > traverse)) {		/* Insert at end */
 //    	SET_NEXT_PTR(traverse, bp);
 //    	SET_PREV_PTR(bp, traverse);
 //    	SET_NEXT_PTR(bp, NULL);
 //    	return;
 //    }

 //    if ((GET_NEXT_PTR(traverse) == NULL) && (bp < traverse)) {		/* Insert before */
 //    	if (GET_PREV_PTR(traverse) == NULL) {
 //    		SET_PREV_PTR(bp, NULL);
 //    		SET_NEXT_PTR(bp, traverse);
 //    		SET_PREV_PTR(traverse, bp);
 //    		start_free_list = bp;
 //    		return;
 //    	}
 //    	else {
 //    		SET_NEXT_PTR(GET_PREV_PTR(traverse), bp);
 //    		SET_PREV_PTR(bp, GET_PREV_PTR(traverse));
 //    		SET_NEXT_PTR(bp, traverse);
 //    		SET_PREV_PTR(traverse, bp);
 //    		return;
 //    	}
 //    }

 //    SET_NEXT_PTR(bp, GET_NEXT_PTR(traverse));
	// SET_PREV_PTR(GET_NEXT_PTR(traverse), bp);
	// SET_NEXT_PTR(traverse, bp);
	// SET_PREV_PTR(bp, traverse);
 //    return;
}

/*
 * Removes free block from free list in 4 cases.
 *
 * 1. Remove the last item.
 * 2. Remove the only item.
 * 3. Remove the first item.
 * 4. Remove the item for every other case (in between two blocks)
 */
static void remove_free_block(void *bp)
{
    /* If bp is last item in list */
    if ((GET_NEXT_PTR(bp) == NULL) && (GET_PREV_PTR(bp) != NULL)) {
        SET_NEXT_PTR(GET_PREV_PTR(bp), NULL);
        SET_PREV_PTR(bp, NULL);
        return;
    }
    /* If bp is only item in list */
    else if ((GET_NEXT_PTR(bp) == NULL) && (GET_NEXT_PTR(bp) == NULL)) {
        start_free_list = NULL;
        return;
    }
    /* If bp is first item in list */
    else if (bp == start_free_list) {
        SET_PREV_PTR(GET_NEXT_PTR(bp), NULL);
        start_free_list = GET_NEXT_PTR(bp);
        SET_NEXT_PTR(bp, NULL);
        return;
    }
	/* Remove item regularly */
    else {
	    SET_NEXT_PTR(GET_PREV_PTR(bp), GET_NEXT_PTR(bp));
	    SET_PREV_PTR(GET_NEXT_PTR(bp), GET_PREV_PTR(bp));
	    SET_NEXT_PTR(bp, NULL);
	    SET_PREV_PTR(bp, NULL);
	    return;
	}
}

/*
 * Prints the heap for debugging purposes. Shows block size and allocation bit.
 */
void show_heap(void)
{
	printf("\nShowing Whole Heap:\n");
	void *bp;
	for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
		printf("%d/%d\n", GET_SIZE(HDRP(bp)), GET_ALLOC(HDRP(bp)));
	printf("\nShowing Free List:\n");
	for (bp = start_free_list; bp != NULL; bp = GET_NEXT_PTR(bp))
		printf("%d/%d\n", GET_SIZE(HDRP(bp)), GET_ALLOC(HDRP(bp)));
	printf("\nEnd of Heap\n\n");
}

/*
 * Heap consistency checker tool. Performs several tasks to see if the heap
 * is in good shape after allocation functions are called. Returns a non-zero value
 * if heap is consistent.
 * 
 * 1. Checks to see that the sum given by the mem_heapsize() function matches the
 * 	  size of all the blocks added together.
 * 2. Checks if there are two blocks next to each other, which should not happen, since
 *    adjacent free blocks are always coalesced.
 * 3. Checks if any of the free blocks are smaller than the minimum block size (16).
 * 4. Checks if the number of free blocks in teh free list is correct.
 * 5. Checks if the free list has all the free blocks in the heap.
 *
 * Prints message if there is an error.
 */
int mm_check(void)      /* Returns non-zero value if heap is consistent */ 
{
    /* Get the sum of the blocks in the heap and see if it matches the value return from mem_heapsize() */
    size_t heap_size = mem_heapsize();
    void *bp;
    size_t valid_heap_size = 0;
    size_t sum_heap_size = DSIZE;       /* 8 bytes are not counted in sum loop */

    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
        sum_heap_size += GET_SIZE(HDRP(bp));
    if ((sum_heap_size) == heap_size)
        valid_heap_size = 1;

    /* Check if there are two free blocks next to each other */
    size_t valid_free_block_order = 1;
    size_t check_free_block = 0;
    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        if (check_free_block == 1 && GET_ALLOC(HDRP(bp)) == 0) {
            valid_free_block_order = 0;
            break;
        }
        if ((GET_ALLOC(HDRP(bp))) == 0)
            check_free_block = 1;
        else
            check_free_block = 0;
    }
    /* Check if any of the free blocks are less than 16; they should all be 16 or greater */
    size_t smaller_than_16 = 0;
    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
        if (GET_SIZE(HDRP(bp)) < 2*DSIZE && GET_ALLOC(HDRP(bp)) == 0)
            smaller_than_16 = 1;

    /* Check if number of free blocks in free list is correct */
    size_t num_free_block = 1;
    size_t sum_free = 0;
    size_t sum_free_from_list = 0;
    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        if (GET_ALLOC(HDRP(bp)) == 0)
            sum_free += 1;
    }
    for (bp = start_free_list; bp != NULL; bp = GET_NEXT_PTR(bp)) {
        sum_free_from_list += 1;
    }
    if (sum_free != sum_free_from_list)
        num_free_block = 0;

    /* Check if free list has all the free blocks in the heap */
    size_t found_in_free_list = 1;
    size_t found_current = 0;
    void *incr;
    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
    	if (GET_ALLOC(HDRP(bp)) == 0) {
    		for (incr = start_free_list; incr != NULL; incr = GET_NEXT_PTR(incr)) {
    			if (bp == incr)
    				found_current = 1;
    		}
    		if (found_current == 0) {
    			found_in_free_list = 0;
    			break;
    		}
    		found_current = 0;
    	}
    }

    if (!valid_heap_size) {
        printf("Heap size is wrong!\n");
        printf("calculated sum: %d\n", (int)sum_heap_size);
        printf("from mem_func: %d\n", (int)heap_size);
        return 0;
    }
    else if (!valid_free_block_order) {
        printf("Two or more free blocks in a row!\n");
        return 0;
    }
    else if (smaller_than_16) {
        printf("Invalid free block!\n");
        return 0;
    }
    else if (!num_free_block) {
        printf("Problem with free list!\n");
		printf("whole list: %d\n", (int)sum_free);
        printf("free list: %d\n", (int)sum_free_from_list);
        return 0;
    }
    else if (!found_in_free_list) {
    	printf("Pointer not inserted in free list!\n");
    	return 0;
    }
    else {
        //printf("Passed all memory tests\n");
        return 1;
    }
}
