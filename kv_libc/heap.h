#ifndef HEAP_4__H
#define HEAP_4__H

#include <stdint.h>
#include <stddef.h>
#include <plat_def.h>



#define portBYTE_ALIGNMENT                      16
#define portBYTE_ALIGNMENT_MASK                 15
typedef unsigned int BaseType_t;

// following is for porting ...
#define configSUPPORT_DYNAMIC_ALLOCATION        1
#define portMAX_DELAY                           0xffffffffffffffffull
#define PRIVILEGED_DATA
#define PRIVILEGED_FUNCTION

#define xTaskResumeAll() 0
#define configASSERT(x) 
#define mtCOVERAGE_TEST_MARKER()    
#define taskENTER_CRITICAL()    
#define taskEXIT_CRITICAL() 
#define vTaskSuspendAll()   
#define traceMALLOC(...)    
#define traceFREE(...)  



/* Define the linked list structure.  This is used to link free blocks in order
 * of their memory address. */
typedef struct A_BLOCK_LINK
{
    struct A_BLOCK_LINK * pxNextFreeBlock; /*<< The next free block in the list. */
    size_t xBlockSize;                     /*<< The size of the free block. */
} BlockLink_t;

typedef struct heap_ctx
{
    /* Create a couple of list links to mark the start and end of the list. */
    BlockLink_t xStart, * pxEnd ;

    /* Keeps track of the number of calls to allocate and free memory as well as the
    * number of free bytes remaining, but says nothing about fragmentation. */
    size_t xFreeBytesRemaining;
    size_t xMinimumEverFreeBytesRemaining;
    size_t xNumberOfSuccessfulAllocations;
    size_t xNumberOfSuccessfulFrees;
    uint8_t *heapMem;
    size_t heapSize;
}heap_ctx_t;


/* Used by heap_5.c to define the start address and size of each memory region
 * that together comprise the total FreeRTOS heap space. */
typedef struct HeapRegion
{
    uint8_t * pucStartAddress;
    size_t xSizeInBytes;
} HeapRegion_t;

/* Used to pass information about the heap out of vPortGetHeapStats(). */
typedef struct xHeapStats
{
    size_t xAvailableHeapSpaceInBytes;          /* The total heap size currently available - this is the sum of all the free blocks, not the largest block that can be allocated. */
    size_t xSizeOfLargestFreeBlockInBytes;      /* The maximum size, in bytes, of all the free blocks within the heap at the time vPortGetHeapStats() is called. */
    size_t xSizeOfSmallestFreeBlockInBytes;     /* The minimum size, in bytes, of all the free blocks within the heap at the time vPortGetHeapStats() is called. */
    size_t xNumberOfFreeBlocks;                 /* The number of free memory blocks within the heap at the time vPortGetHeapStats() is called. */
    size_t xMinimumEverFreeBytesRemaining;      /* The minimum amount of total free memory (sum of all free blocks) there has been in the heap since the system booted. */
    size_t xNumberOfSuccessfulAllocations;      /* The number of calls to pvPortMalloc() that have returned a valid memory block. */
    size_t xNumberOfSuccessfulFrees;            /* The number of calls to vPortFree() that has successfully freed a block of memory. */
} HeapStats_t;

/*
 * Used to define multiple heap regions for use by heap_5.c.  This function
 * must be called before any calls to pvPortMalloc() - not creating a task,
 * queue, semaphore, mutex, software timer, event group, etc. will result in
 * pvPortMalloc being called.
 *
 * pxHeapRegions passes in an array of HeapRegion_t structures - each of which
 * defines a region of memory that can be used as the heap.  The array is
 * terminated by a HeapRegions_t structure that has a size of 0.  The region
 * with the lowest start address must appear first in the array.
 */
void vPortDefineHeapRegions( const HeapRegion_t * const pxHeapRegions );

/*
 * Returns a HeapStats_t structure filled with information about the current
 * heap state.
 */
void vPortGetHeapStats( HeapStats_t * pxHeapStats );

/*
 * Map to the memory management routines required for the port.
 */
void * pvPortMalloc( size_t xSize );
void vPortFree( void * pv );
void vPortInitialiseBlocks( void );
size_t xPortGetFreeHeapSize( void );
size_t xPortGetMinimumEverFreeHeapSize( void );



#define kv_malloc(x) pvPortMalloc(x)
#define kv_free(x)  vPortFree(x)
#define kv_getFreeSize() xPortGetFreeHeapSize()

#endif
