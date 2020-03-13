///////////////////////////////////////////////////////////////////////////////
//
// Copyright 2019 Jim Skrentny
// Posting or sharing this file is prohibited, including any changes/additions.
//
///////////////////////////////////////////////////////////////////////////////
// Main File:        heapAlloc.c
// This File:        heapAlloc.c
// Other Files:      (name of all other files if any)
// Semester:         CS 354 Fall 2019
//
// Author:           Jason Sutanto
// Email:            jsutanto2@wisc.edu
// CS Login:         Sutanto
//
/////////////////////////// OTHER SOURCES OF HELP /////////////////////////////
//                   fully acknowledge and credit all sources of help,
//                   other than Instructors and TAs.
//
// Persons:          Identify persons by name, relationship to you, and email.
//                   Describe in detail the the ideas and help they provided.
//
// Online sources:   avoid web searches to solve your problems, but if you do
//                   search, be sure to include Web URLs and description of
//                   of any information you find.
///////////////////////////////////////////////////////////////////////////////

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <stdio.h>
//#include "heapAlloc.h"

/*
 * This structure serves as the header for each allocated and free block.
 * It also serves as the footer for each free block but only containing size.
 */
typedef struct blockHeader {
    int size_status;
    /*
     * Size of the block is always a multiple of 8.
     * Size is stored in all block headers and free block footers.
     *
     * Status is stored only in headers using the two least significant bits.
     *   Bit0 => least significant bit, last bit
     *   Bit0 == 0 => free block
     *   Bit0 == 1 => allocated block
     *
     *   Bit1 => second last bit
     *   Bit1 == 0 => previous block is free
     *   Bit1 == 1 => previous block is allocated
     *
     * End Mark:
     *  The end of the available memory is indicated using a size_status of 1.
     *
     * Examples:
     *
     * 1. Allocated block of size 24 bytes:
     *    Header:
     *      If the previous block is allocated, size_status should be 27
     *      If the previous block is free, size_status should be 25
     *
     * 2. Free block of size 24 bytes:
     *    Header:
     *      If the previous block is allocated, size_status should be 26
     *      If the previous block is free, size_status should be 24
     *    Footer:
     *      size_status should be 24
     */
} blockHeader;

/* Global variable - DO NOT CHANGE. It should always point to the first block,
 * i.e., the block at the lowest address.
 */

blockHeader *heapStart = NULL;
blockHeader *nextLoc = NULL;
static int heapSize = 0;

/* 
 * Function for allocating 'size' bytes of heap memory.
 * Argument size: requested size for the payload
 * Returns address of allocated block on success.
 * Returns NULL on failure.
 * This function should:
 * - Check size - Return NULL if not positive or if larger than heap space.
 * - Determine block size rounding up to a multiple of 8 and possibly adding padding as a result.
 * - Use NEXT-FIT PLACEMENT POLICY to chose a free block
 * - Use SPLITTING to divide the chosen free block into two if it is too large.
 * - Update header(s) and footer as needed.
 * Tips: Be careful with pointer arithmetic and scale factors.
 */
void* allocHeap(int size) {
    //if nextLoc is null save size of heap since heapStart points to the start of heap
      if(nextLoc == NULL){
         heapSize = (heapStart->size_status) - (heapStart->size_status & 3) ;
      }
    
    //ensure size is greater than zero
    if(size <= 0){
        fprintf(stderr,
                "Error size user wants to allocate is too large or \n");
        return NULL;
    }
    
    //pointer to current blockHeader 
    blockHeader* current;
    //if nextLocation pointer is null set current pointer to start of heap
    if(nextLoc == NULL){
     current = heapStart;
    } else {
    //else set current to nextLoc
    current = nextLoc;
    }
    //save size in variable and add header 
    int allocSize = size + 4;
    //pad memory if it is not a multiple of 8
    if(allocSize % 8 != 0){
        //variable to store padding
	allocSize = (allocSize+8) - (allocSize+8) % 8;
    }
    //nextLoc is not null so set pointer to point at block after last allocated block
    if(nextLoc != NULL){
        current = nextLoc;
    }
    
    //checks if search reaches 1 cycle
    blockHeader* prev = current;
    
    //move pointer with the size of the current block
    int currentSize;
    
    //saves the current size of block
    int blockSize;
    
    //check if allocSize is within the heap
    if(allocSize > heapSize){
     return NULL;
    }
    
    //used for splitting
    int nextSize;

    while(1){
        //Check if current is allocated 
        if((current -> size_status & 1) == 1){
        //go to next header by incrementing with size of current block
            currentSize = current->size_status >> 2 << 2;
            current = (blockHeader*)((char*)current + currentSize);
	//if current's memory is not allocated 
        }else{
            //Check if current block has enough space
            if(allocSize <= (currentSize = current->size_status >> 2 << 2)){
                //case where you cannot split 
                if(allocSize == currentSize){	        
                    current-> size_status = allocSize + 1;
		    //save address of payload
                    blockHeader* allocatedAddress = (blockHeader*)current + 1;
                    //go to next header to activate p bit
                    current = (blockHeader*)((char*)current + allocSize);
                    current -> size_status += 2;
		    //point nextLoc to 
                    nextLoc = current;
                    //return address of header of alloced mem
                    return allocatedAddress;
                }else{   //you can split block

                    //get size of current block 
		    blockSize = current -> size_status >> 2 << 2;
		    //change size of header and active both p and a bit
                    current->size_status = allocSize + 1 + 2;
		    //save allocated size without p and a bit
                    int allocMem = current -> size_status - 3;
		    //get size of remaining free memory
		    nextSize = blockSize - (allocMem);
		    blockSize = nextSize;
                    //save the address of header of alloced mem
                    blockHeader* allocatedAddress = (blockHeader*) current + 1;
                    current = (blockHeader*)((char*) current + allocSize);
		    //set p bit of next header and change its size to remaining free memory
                    current -> size_status += nextSize;
		    current -> size_status += 2;
                    //go to footer of next header and change the size
                    current = (blockHeader*)((char*) current + nextSize);
                    current = (blockHeader*)((char*) current - 4);
                    current -> size_status = nextSize;
		    current = (blockHeader*)((char*) current - (nextSize-4));
                    //set next to current which points to nexthader
                    nextLoc = current;
                    //return address of header memory
                    return allocatedAddress;
                    
                }
            }else{
                //go to next block
		currentSize = current->size_status >> 2 << 2;
                current = (blockHeader*)((char*)current + currentSize);
            }
        }
        //if you reach end mark go to heap start
        if(current->size_status == 1){
            //wrap around
            current = heapStart;
        }
        //after search if current is == prev then fail
        if(current == prev){

            return NULL;
        }
    }
    //alloc fails so return NULL
    return NULL;
} 


    /*
     * Function for freeing up a previously allocated block.
     * Argument ptr: address of the block to be freed up.
     * Returns 0 on success.
     * Returns -1 on failure.
     * This function should:
     * - Return -1 if ptr is NULL.
     * - Return -1 if ptr is not a multiple of 8.
     * - Return -1 if ptr is outside of the heap space.
     * - Return -1 if ptr block is already freed.
     * - USE IMMEDIATE COALESCING if one or both of the adjacent neighbors are free.
     * - Update header(s) and footer as needed.
     */
int freeHeap(void *ptr) {
        //pointer cannot be null
        if(ptr == NULL) return -1;
        //Check if address of ptr is a multiple of 8
        if(((int) ptr) % 8 != 0) return -1;

        //check if ptr's address is within the heap space
        blockHeader *endMark = heapStart;
        int i = endMark -> size_status;
        //traverse entire heap
        while(i != 1) {
            int a_bit = i % 2;
            int p_bit = (i >> 1) % 2;
            endMark = (blockHeader*) ((void*)(endMark) + i - 2 * p_bit - a_bit);
            i = endMark -> size_status;
        }
        //ptr address is not within the bounds of heap
        if((int)(ptr) <= (int)(heapStart) || (int)(ptr) >= (int)(endMark)) return -1;

        //block header of the block user wants to free
        blockHeader *specifiedFreeBlock = (blockHeader*)(ptr - sizeof(blockHeader));
        //size of block to free plus p-bits and a-bits
        int sizeStatus = specifiedFreeBlock -> size_status;

        //calculates the a-bit
        int a_bit = sizeStatus % 2;
        //return -1 if the a-bit is already freed
        //sets the block to be freed a-bit to 0
        if(a_bit != 1) return -1;
        else {
            specifiedFreeBlock -> size_status -= 1;
            //reset the size status
            sizeStatus = specifiedFreeBlock -> size_status;
            a_bit = 0;
        }

        //calculate p bit of block you want to free and get the size
        int p_bit = (sizeStatus >> 1) % 2;
        int size = sizeStatus - 2 * p_bit - a_bit;
        //check next block
        blockHeader *next = (blockHeader*) (((void*) (specifiedFreeBlock)) + size);
        //check if next block is allocated and is not end mark
        if(next -> size_status != 1 && (next -> size_status % 2) == 0) {
            //coalesce the block you want to free and next block
            specifiedFreeBlock -> size_status += (next -> size_status - 2);
            //save new size of free block
            sizeStatus = specifiedFreeBlock -> size_status;
            size = sizeStatus - 2 * p_bit - a_bit;
        }
        //check block to the left of specified free block
        if(p_bit == 0) {
            //pointer to the footer of the previous block
            //pointer to the footer of the previous block
            blockHeader *previousFooter = (blockHeader*) (((void*)(specifiedFreeBlock)) - sizeof(blockHeader));
            //calculate size of previous block
            int previousSize = previousFooter -> size_status;
            //go to header of previous block
            blockHeader *prevHeader = (blockHeader*) ((void*)(specifiedFreeBlock) - previousSize);
            //coalesce previous and specified block
            specifiedFreeBlock = prevHeader;
            //update size of coalesced block
            specifiedFreeBlock -> size_status += size;
            sizeStatus = specifiedFreeBlock -> size_status;
            a_bit = sizeStatus % 2;
            p_bit = (sizeStatus >> 1) % 2;
            size = sizeStatus - 2 * p_bit - a_bit;
        }
        //set the footer of coalsced block
        blockHeader* footer = (blockHeader*) ((void*)(specifiedFreeBlock) + size - sizeof(blockHeader));
        footer -> size_status = size;

        //activate p bit
        blockHeader* nextHeader = (blockHeader*) ((void*)(specifiedFreeBlock) + size);
        if(!(nextHeader -> size_status == 1)) {
            //get next header's p-bit
            int next_p_bit = (nextHeader -> size_status >> 1) % 2;
            //set p bit to zero since previous block is freed
            if(next_p_bit != 0) {
                nextHeader -> size_status -= 2;
            }
        }
        //coalsce does not fail so return 0
        return 0;

    }

/*
 * Function used to initialize the memory allocator.
 * Intended to be called ONLY once by a program.
 * Argument sizeOfRegion: the size of the heap space to be allocated.
 * Returns 0 on success.
 * Returns -1 on failure.
 */
int initHeap(int sizeOfRegion) {
    
    static int allocated_once = 0; //prevent multiple initHeap calls
    
    int pagesize;  // page size
    int padsize;   // size of padding when heap size not a multiple of page size
    int allocsize; // size of requested allocation including padding
    void* mmap_ptr; // pointer to memory mapped area
    int fd;
    
    blockHeader* endMark;
    
    if (0 != allocated_once) {
        fprintf(stderr,
                "Error:mem.c: InitHeap has allocated space during a previous call\n");
        return -1;
    }
    if (sizeOfRegion <= 0) {
        fprintf(stderr, "Error:mem.c: Requested block size is not positive\n");
        return -1;
    }
    
    // Get the pagesize
    pagesize = getpagesize();
    
    // Calculate padsize as the padding required to round up sizeOfRegion
    // to a multiple of pagesize
    padsize = sizeOfRegion % pagesize;
    padsize = (pagesize - padsize) % pagesize;
    
    allocsize = sizeOfRegion + padsize;
    
    // Using mmap to allocate memory
    fd = open("/dev/zero", O_RDWR);
    if (-1 == fd) {
        fprintf(stderr, "Error:mem.c: Cannot open /dev/zero\n");
        return -1;
    }
    mmap_ptr = mmap(NULL, allocsize, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (MAP_FAILED == mmap_ptr) {
        fprintf(stderr, "Error:mem.c: mmap cannot allocate space\n");
        allocated_once = 0;
        return -1;
    }
    
    allocated_once = 1;
    
    // for double word alignment and end mark
    allocsize -= 8;
    // Initially there is only one big free block in the heap.
    // Skip first 4 bytes for double word alignment requirement.
    heapStart = (blockHeader*) mmap_ptr + 1;
    
    // Set the end mark
    endMark = (blockHeader*)((void*)heapStart + allocsize);
    endMark->size_status = 1;
    
    // Set size in header
    heapStart->size_status = allocsize;
    
    // Set p-bit as allocated in header
    // note a-bit left at 0 for free
    heapStart->size_status += 2;
    
    // Set the footer
    blockHeader *footer = (blockHeader*) ((char*)heapStart + allocsize - 4);
    footer->size_status = allocsize;
    
    return 0;
}
    
// Initially there is only one big free block in the heap.

/*
 * Function to be used for DEBUGGING to help you visualize your heap structure.
 * Prints out a list of all the blocks including this information:
 * No.      : serial number of the block
 * Status   : free/used (allocated)
 * Prev     : status of previous block free/used (allocated)
 * t_Begin  : address of the first byte in the block (where the header starts)
 * t_End    : address of the last byte in the block
 * t_Size   : size of the block as stored in the block header
 */
 void DumpMem() {
           
           int counter;
           char status[5];
           char p_status[5];
           char *t_begin = NULL;
           char *t_end   = NULL;
           int t_size;
           
           blockHeader *current = heapStart;
           counter = 1;
           
           int used_size = 0;
           int free_size = 0;
           int is_used   = -1;
           
           fprintf(stdout, "************************************Block list***\
                   ********************************\n");
           fprintf(stdout, "No.\tStatus\tPrev\tt_Begin\t\tt_End\t\tt_Size\n");
           fprintf(stdout, "-------------------------------------------------\
                   --------------------------------\n");
           
           while (current->size_status != 1) {
               t_begin = (char*)current;
               t_size = current->size_status;
               
               if (t_size & 1) {
                   // LSB = 1 => used block
                   strcpy(status, "used");
                   is_used = 1;
                   t_size = t_size - 1;
               } else {
                   strcpy(status, "Free");
                   is_used = 0;
               }
               
               if (t_size & 2) {
                   strcpy(p_status, "used");
                   t_size = t_size - 2;
               } else {
                   strcpy(p_status, "Free");
               }
               
               if (is_used){
                   used_size += t_size;
               }
               else{
                   free_size += t_size;
               }
               
               t_end = t_begin + t_size - 1;
               
               fprintf(stdout, "%d\t%s\t%s\t0x%08lx\t0x%08lx\t%d\n", counter, status,
                       p_status, (unsigned long int)t_begin, (unsigned long int)t_end, t_size);
               
               current = (blockHeader*)((char*)current + t_size);
               counter = counter + 1;
           }
           
           fprintf(stdout, "---------------------------------------------------\
                   ------------------------------\n");
           fprintf(stdout, "***************************************************\
                   ******************************\n");
           fprintf(stdout, "Total used size = %d\n", used_size);
           fprintf(stdout, "Total free size = %d\n", free_size);
           fprintf(stdout, "Total size = %d\n", used_size + free_size);
           fprintf(stdout, "***************************************************\
                   ******************************\n");
           fflush(stdout);
           
           return;
     
 }




