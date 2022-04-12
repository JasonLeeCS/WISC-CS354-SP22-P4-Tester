#include "mem.h"                      
extern BLOCK_HEADER* first_header;




// return a pointer to the payload
// if a large enough free block isn't available, return NULL
void* Mem_Alloc(int size)
{
    int payload = 0;
    int padding = 0;
    int blockSize = 0;

    BLOCK_HEADER *payloadPTR;
    BLOCK_HEADER *curr = first_header;

    // find a free block that's big enough
    while(!isLast(curr))
    {
        blockSize = getSize(curr); // Computes blockSize
        payloadPTR = getUserPointer(curr); // sets payloadPTR to a pointer to the first element of the payload
        payload = curr->payload; // Sets payload = size of the current block header

        // allocate the block
        /**
         * If the current block is free and the size input fits inside the payload
         * Set allocated
         **/
        if((!isFree(curr)) && (size <= (blockSize - 8))) 
        {
            setAllocated(curr);
            // split if necessary (if padding size is greater than or equal to 16 split the block)
            if(payload >= 16)
            {
                while((padding + size + (unsigned long) curr) % 16 != 0)
                {
                    padding = padding + 1;
                }

                curr->payload = size; // Sets payload in the current block header = to the size input

                curr->size_alloc = padding + size + 8; // Sets size_alloc of the current block header + padding + size + header size

                setAllocated(curr);
                blockSize = getSize(curr);
                curr = getNextHeader(curr, blockSize);

                if(!isLast(curr))
                {
                    curr->size_alloc = 8 - blockSize + payload; // Sets size_alloc to header size - block size + payload
                    curr->payload = curr->size_alloc - 8; // Sets payload to block size - header
                }
            }
            return payloadPTR;
        }
        curr = getNextHeader(curr, blockSize); // Continue to search for available blocks
    }
    return NULL; // return NULL if we didn't find a block

}


// return 0 on success
// return -1 if the input ptr was invalid
int Mem_Free(void *ptr)
{
    BLOCK_HEADER* curr = first_header;
    BLOCK_HEADER* next;
    BLOCK_HEADER* doubleNext;
    int diff = 1; 
    int found = 0; 
    int padding = 0;
    // traverse the list and check all pointers to find the correct block 
    // if you reach the end of the list without finding it return -1
    
    // free the block 
    while(!isLast(curr))
    {
        if(isFree(curr)) // If the current pointer is = to the one we need
        {
            diff = 8 + (unsigned long)curr - (unsigned long)found;
            if(diff == 0)
            {
                padding = getPadding(curr); // Stores padding 
                curr->payload += padding; // Updates current payload with padding size 
                setFree(curr); 
                found = 1; // Sets found to 1, which means we found the pointer we needed
            }
        }
        curr = getNextHeader(curr, getSize(curr)); // Continue to search
    }
    // coalesce adjacent free blocks
    curr = first_header;
    if(found)
    {
        while(!isLast(curr))
        {
            next = getNextHeader(curr, getSize(curr)); 
            doubleNext = getNextHeader(next, getSize(next));

            if(!isFree(curr))
            {
                if(!isFree(next))
                {
                    while(!isFree(doubleNext))
                    {
                        doubleNext = getNextHeader(doubleNext, getSize(doubleNext));
                    }
                }
                /**
                 * Sets the size_alloc of the current block to the difference between the
                 * current block and the last free block
                 **/
                curr->size_alloc = (unsigned long)doubleNext - (unsigned long)curr;
                // Sets payload of the current block to the size of the current block - header size
                curr->payload = curr->size_alloc - 8;
                curr = getNextHeader(curr, getSize(curr));
                continue;
            }
        }
        curr = getNextHeader(curr, getsize(curr));
    }
    if(found)
    {
        return 0;
    }
    return -1;
}

/**
 * This function takes in a block header and checks if it is the last block
 * Returns 1 if the block header is the last else it returns 0
 **/
int isLast(BLOCK_HEADER* bHeader)
{
    if(bHeader->size_alloc == 1)
    {
        return 1;
    }
    return 0;
}

/**
 * This function takes in a block header and checks if it is allocated
 * If the block header is free, return 0
 * else if the block header is allocated, return 1
 **/
int isFree(BLOCK_HEADER* bHeader)
{
    if(bHeader->size_alloc % 2 == 0)
    {
        return 0;
    }
    return 1;
}

// This function takes in a block header and returns the size of it
int getSize(BLOCK_HEADER* bHeader)
{
    if(isFree(bHeader))
    {
        return bHeader->size_alloc - 1;
    }
    return bHeader->size_alloc;
}

// This function takes in a block header and returns the padding size
int getPadding(BLOCK_HEADER* bHeader)
{
    return getSize(bHeader) - bHeader->payload - 8;
}

/**
 * This function takes in a block header and the sizeof it
 * and returns a pointer to the next block header
 **/
void* getNextHeader(BLOCK_HEADER* bHeader, int sizeOf)
{
    return (BLOCK_HEADER*)(sizeof + (unsigned long) bHeader);
}

/**
 * This function takes in a block header
 * and returns a pointer to the address of the payload
 **/
void* getUserPointer(BLOCK_HEADER* bHeader)
{
    return(void*)(8 + (unsigned long) bHeader);
}

/**
 * This function takes in a block header and if it is allocated, sets it to free
 * if the block header is not allocated then it does nothing
 **/
void setFree(BLOCK_HEADER* bHeader)
{
    if(isFree(bHeader))
    {
        bHeader->size_alloc -= 1;
    }
}


/**
 * This function takes in a block header and if it is currently free, sets it to allocated
 * if the block header is not free then it does nothing
 **/
void setAllocated(BLOCK_HEADER* bHeader)
{
    if(!isFree(bHeader))
    {
        bHeader->size_alloc += 1;
    }
}