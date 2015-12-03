#include "my_malloc.h"

/* You *MUST* use this macro when calling my_sbrk to allocate the
 * appropriate size. Failure to do so may result in an incorrect
 * grading!
 */
#define SBRK_SIZE 2048

/* If you want to use debugging printouts, it is HIGHLY recommended
 * to use this macro or something similar. If you produce output from
 * your code then you may receive a 20 point deduction. You have been
 * warned.
 */
#ifdef DEBUG
#define DEBUG_PRINT(x) printf x
#else
#define DEBUG_PRINT(x)
#endif


/* our freelist structure - this is where the current freelist of
 * blocks will be maintained. failure to maintain the list inside
 * of this structure will result in no credit, as the grader will
 * expect it to be maintained here.
 * Technically this should be declared static for the same reasons
 * as above, but DO NOT CHANGE the way this structure is declared
 * or it will break the autograder.
 */
static void *split_block(void *location, short totalsize, size_t neededsize);

static void omit_block(metadata_t *location);

//static void *mergeAdjacent(void *ptr);

metadata_t *freelist;

void *my_malloc_size_order(size_t size) {
//    /* FIX ME */
//    //default to no error
//    ERRNO = NO_ERROR;
//
//    //Too big, complain to the user
//    if (size > SBRK_SIZE) {
//        ERRNO = SINGLE_REQUEST_TOO_LARGE;
//        return NULL;
//    }
//
//    //List does not exist, we need to create it
//    if (freelist == NULL) {
//        //start is the address of our fancy new chunk of memory!
//        void *start = my_sbrk(SBRK_SIZE);
//
//        //shouldn't happen because we just started the chain but...
//        if (start == NULL) {
//            //if we can't get memory from sbrk, throw error
//            ERRNO = OUT_OF_MEMORY;
//            return NULL;
//        }
//
//        //immediately break the chunk into what we need and put the rest on the freelist,
//        //split_block also sets size of start
//        freelist = split_block(start, SBRK_SIZE, size);
//        //change inUse to equal 1
//        ((metadata_t *) start)->in_use = 1;
//        //Give address + metadata for user to write data
//        return *(&start + sizeof(SBRK_SIZE));
//    } else { //freelist exists, find where we want to put our data
//
//    }

    return my_malloc_addr_order(size);
}

void *my_malloc_addr_order(size_t size) {
    /* FIX ME -- fixed?*/
    //default to no error
    ERRNO = NO_ERROR;

    //Too big, complain to the user
    if (size + sizeof(metadata_t) > SBRK_SIZE) {
        ERRNO = SINGLE_REQUEST_TOO_LARGE;
        return NULL;
    }

    //List does not exist, we need to create it
    if (freelist == NULL) {
        //start is the address of our fancy new chunk of memory!
        metadata_t *start = my_sbrk(SBRK_SIZE);

        //shouldn't happen because we just started the chain but...
        if (start == NULL) {
            //if we can't get memory from sbrk, throw error
            ERRNO = OUT_OF_MEMORY;
            return NULL;
        }
        //immediately break the chunk into what we need and put the rest on the freelist,
        //split_block also sets size of start
        freelist = split_block(start, SBRK_SIZE, size);
        //change inUse to equal 1
        (start)->in_use = 1;
        //Give address + metadata for user to write data
        return start + 1;
    } else { //freelist exists, find where we want to put our data
        metadata_t *probe = freelist;
        //search for a place on freelist until we find one, or reach the end of the list
        while (!(probe->in_use == 0 && probe->size >= size) && probe->next != NULL) {
            probe = probe->next;
        }
        //we found our spot
        if ((probe->in_use == 0 && probe->size >= size) /*&& probe->next != NULL*/) {
            //split the block to what we need, fix chain, and set sizes
            void *newblock = split_block(probe, probe->size, size);
            probe->in_use = 1;
            //location is first in freelist, so must update it!!!
            if (probe == freelist) {
                freelist = newblock;
            }
            //return address but with space for our metadata
            return probe + 1;
        } else { //we reached the end of the list, probe->next is NULL
            //get new memory from sbrk
            //TODO: check that address is correct for address-sorted list!!!
            void *start = my_sbrk(SBRK_SIZE);

            //if start == NULL, then
            if (start == NULL) {
                //we can't get any more memory from sbrk, throw error
                ERRNO = OUT_OF_MEMORY;
                return NULL;
            }
            if (start > (void *) freelist) {
                //set the relationship to the fresh block
                probe->next = start;
                //fresh block has previous to probe
                ((metadata_t *) start)->prev = probe;
                //fresh block is end of chain
                ((metadata_t *) start)->next = NULL;
            } else {
                //block should go BEFORE freelist
                ((metadata_t *) start)->prev = NULL;
                ((metadata_t *) start)->next = freelist;
            }
            //fresh block has the entire bucket for size until split
            ((metadata_t *) start)->size = SBRK_SIZE;
            //fresh block is used
            ((metadata_t *) start)->in_use = 1;
            if (start > (void *) freelist) {
                //split the block to what we need, fix chain, and set sizes
                split_block(start, SBRK_SIZE, size);
            } else {
                //new sub block should be at the front of the freelist by address
                freelist = split_block(start, SBRK_SIZE, size);
            }

            //return our memory location with space for our metadata
            return (metadata_t *) start + 1;
        }
    }
    //I don't think we can get here...
    return NULL;
}

/**
 * Important thing: neededsize is the size that the user asks for, NOT including sizeof(metadata_t)
 */
static void *split_block(void *location, short totalsize, size_t neededsize) {
    //If there is enough space to warrant a split, split it
    if (totalsize >= neededsize + 2 * sizeof(metadata_t) + 1) {
        //starting position of second block will be &previous block + total size
        void *second_block = (char *) location + neededsize + sizeof(metadata_t);

        //The new block will take the old block's prev and next pointers
        if (((metadata_t *) location)->prev != NULL) {
            //omit used block from forward chain
            ((metadata_t *) location)->prev->next = second_block;
            //and inherit prev
            ((metadata_t *) second_block)->prev = ((metadata_t *) location)->prev;
        } else {
            //explicitly declare null
            ((metadata_t *) second_block)->prev = NULL;
        }
        if (((metadata_t *) location)->next != NULL) {
            //omit used block from reverse chain
            ((metadata_t *) location)->next->prev = second_block;
            //and inherit next
            ((metadata_t *) second_block)->next = ((metadata_t *) location)->next;
        } else {
            //explicitly declare null
            ((metadata_t *) second_block)->next = NULL;
        }

        //The new block is unused, so set to 0 to avoid bad things
        ((metadata_t *) second_block)->in_use = 0;
        //The new block has size of total_size - needed_size.  *Should* always fit in short
        ((metadata_t *) second_block)->size = (short) (totalsize - (neededsize + sizeof(metadata_t)));
        //make sure the original block knows its new size.  *Should* always fit in short
        ((metadata_t *) location)->size = (short) (neededsize + sizeof(metadata_t));
        //return address of second block
        return second_block;
    } else {
        //remove the block from the chain using helper method
        omit_block(location);
        //and return NULL
        return NULL;
    }
}

static void omit_block(metadata_t *location) {
    if (location->prev != NULL) {
        location->prev->next = location->next;
    }
    if (location->next != NULL) {
        location->next->prev = location->prev;
    }
}

void my_free_size_order(void *ptr) {
    my_free_addr_order(ptr);
}

void my_free_addr_order(void *ptr) {
    /* FIX ME */

    //default to no error
    ERRNO = NO_ERROR;

    //First thing is to fix this address to access our metadata
    //This will work unless the user wrote over it, in which case it's their fault
    void *location = (char *) ptr - sizeof(metadata_t);
    //LOCATION IS A FREE BLOCK! LOCATION IS FREEEEE!
    ((metadata_t *) location)->in_use = 0;
    //Destroy previous pointers
//    omit_block(location);

    //navigate the list to re-insert this block!
    void *probe = (char *) freelist;
    if (probe < location) {
        //I hope to god these are never equal...
        while (probe < location && ((metadata_t *) probe)->next != NULL) {
            probe = ((metadata_t *) probe)->next;
        }
        //reached the end
        if (((metadata_t *) probe)->next == NULL) {
            //can we combine the blocks? are they adjacent
            if ((char *) probe + sizeof(metadata_t) == (char *) ptr + ((metadata_t *) location)->size) {
                //can combine, so let location eat the space that was in size
                ((metadata_t *) probe)->size = ((metadata_t *) probe)->size + ((metadata_t *) location)->size;
            } else {
                //no, just stick it on.
                ((metadata_t *) probe)->next = location;
                ((metadata_t *) location)->prev = probe;
            }
        } else {
            //need to insert this block in between two existing
            ((metadata_t *) location)->next = (metadata_t *) probe;
            if (((metadata_t *) probe)->prev != NULL) {
                ((metadata_t *) location)->prev = ((metadata_t *) probe)->prev;
                ((metadata_t *) location)->prev->next = (metadata_t *) location;
            }
            ((metadata_t *) probe)->prev = (metadata_t *) location;
        }
    } else {
        //location needs to at the beginning of the freelist
        if ((char *) ptr + ((metadata_t *) location)->size == probe) {
            ((metadata_t *) location)->size = ((metadata_t *) location)->size + ((metadata_t *) probe)->size;
        } else {
            //no, just stick it on.
            freelist->prev = location;
            ((metadata_t *) location)->next = freelist;
            freelist->prev = NULL;
        }
        freelist = location;
    }
}

////mergeAdjacent expects ptr to point to the start of the actual block not the start of the data
//static void *mergeAdjacent(void *ptr) {
//    if (((metadata_t *) ptr)->next != NULL) {
//        //if it is right after
//        int ptrsize = ((metadata_t *) ptr)->size;
//        metadata_t *ptrnext = ((metadata_t *) ptr)->next;
//        if (ptr + ((metadata_t *) ptr)->size == ((metadata_t *) ptr)->next) {
//            //Steal the next
//            ((metadata_t *) ptr)->next = ((metadata_t *) ptr)->next->next;
//            //nullify stuff to avoid bads
//            ((metadata_t *) ptr)->next->prev = NULL;
//            ((metadata_t *) ptr)->next->next = NULL;
//            //Finally increase our size to include next
//            ((metadata_t *) ptr)->size = ((metadata_t *) ptr)->size + ((metadata_t *) ptr)->next->size;
//        }
//    }
//    if (((metadata_t *) ptr)->prev != NULL) {
//        //if it is right before
//        if (((metadata_t *) ptr)->prev + ((metadata_t *) ptr)->prev->size == ptr) {
//            //omit us from list
//            if (((metadata_t *) ptr)->next != NULL) {
//                ((metadata_t *) ptr)->prev->next = ((metadata_t *) ptr)->next;
//                ((metadata_t *) ptr)->next->prev = ((metadata_t *) ptr)->prev;
//            }
//            //steal our size
//            ((metadata_t *) ptr)->prev->size = ((metadata_t *) ptr)->prev->size + ((metadata_t *) ptr)->size;
//            ((metadata_t *) ptr)->next = NULL;
//            ((metadata_t *) ptr)->prev = NULL;
//        }
//    }
//}

