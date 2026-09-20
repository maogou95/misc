/*
    FreeRTOS V8.2.3 - Copyright (C) 2015 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>>> AND MODIFIED BY <<<< the FreeRTOS exception.

    ***************************************************************************
    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<
    ***************************************************************************

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available on the following
    link: http://www.freertos.org/a00114.html

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that is more than just the market leader, it     *
     *    is the industry's de facto standard.                               *
     *                                                                       *
     *    Help yourself get started quickly while simultaneously helping     *
     *    to support the FreeRTOS project by purchasing a FreeRTOS           *
     *    tutorial book, reference manual, or both:                          *
     *    http://www.FreeRTOS.org/Documentation                              *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
    the FAQ page "My application does not run, what could be wrong?".  Have you
    defined configASSERT()?

    http://www.FreeRTOS.org/support - In return for receiving this top quality
    embedded software for free we request you assist our global community by
    participating in the support forum.

    http://www.FreeRTOS.org/training - Investing in training allows your team to
    be as productive as possible as early as possible.  Now you can receive
    FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
    Ltd, and the world's leading authority on the world's leading RTOS.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
    Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

    http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
    Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and commercial middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/

/*
 * A sample implementation of pvPortMalloc() and vPortFree() that combines
 * (coalescences) adjacent memory blocks as they are freed, and in so doing
 * limits memory fragmentation.
 *
 * See heap_1.c, heap_2.c and heap_3.c for alternative implementations, and the
 * memory management pages of http://www.FreeRTOS.org for more information.
 */
#include <stdlib.h>
#include <stdint.h>
/* Defining MPU_WRAPPERS_INCLUDED_FROM_API_FILE prevents task.h from redefining
all the API functions to use the MPU wrappers.  That should only be done when
task.h is included from an application file. */
#define MPU_WRAPPERS_INCLUDED_FROM_API_FILE

//#include "FreeRTOS.h"
//#include "task.h"

#undef MPU_WRAPPERS_INCLUDED_FROM_API_FILE
#define configTOTAL_HEAP_SIZE (38 * 1024)
/* Block sizes must not get too small. */
#define heapMINIMUM_BLOCK_SIZE	( ( size_t ) ( xHeapStructSize << 1 ) )

/* Assumes 8bit bytes! */
#define heapBITS_PER_BYTE		( ( size_t ) 8 )

/* Allocate the memory for the heap. */
#if( configAPPLICATION_ALLOCATED_HEAP == 1 )
	/* The application writer has already defined the array used for the RTOS
	heap - probably so it can be placed in a special segment or address. */
	extern uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];
#else
	static uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];
#endif /* configAPPLICATION_ALLOCATED_HEAP */

/* Define the linked list structure.  This is used to link free blocks in order
of their memory address. */
typedef struct A_BLOCK_LINK
{
	struct A_BLOCK_LINK *pxNextFreeBlock;	/*<< The next free block in the list. */
	size_t xBlockSize;						/*<< The size of the free block. */
} BlockLink_t;

/*-----------------------------------------------------------*/

/*
 * Inserts a block of memory that is being freed into the correct position in
 * the list of free memory blocks.  The block being freed will be merged with
 * the block in front it and/or the block behind it if the memory blocks are
 * adjacent to each other.
 */
static void prvInsertBlockIntoFreeList( BlockLink_t *pxBlockToInsert );

/*
 * Called automatically to setup the required heap structures the first time
 * pvPortMalloc() is called.
 */
static void prvHeapInit( void );

/*-----------------------------------------------------------*/
#define portBYTE_ALIGNMENT 8

#if portBYTE_ALIGNMENT == 8
    #define portBYTE_ALIGNMENT_MASK (0x0007)
#endif
/* The size of the structure placed at the beginning of each allocated memory
block must by correctly byte aligned. */
static const size_t xHeapStructSize	= ( sizeof( BlockLink_t ) + ( ( size_t ) ( portBYTE_ALIGNMENT - 1 ) ) ) & ~( ( size_t ) portBYTE_ALIGNMENT_MASK );

/* Create a couple of list links to mark the start and end of the list. */
static BlockLink_t xStart, *pxEnd = NULL;

/* Keeps track of the number of free bytes remaining, but says nothing about
fragmentation. */
static size_t xFreeBytesRemaining = 0U;
static size_t xMinimumEverFreeBytesRemaining = 0U;
static size_t xNumberOfSuccessfulAllocations = 0;//成功分配
static size_t xNumberOfSuccessfulFrees       = 0;
/* Gets set to the top bit of an size_t type.  When this bit in the xBlockSize
member of an BlockLink_t structure is set then the block belongs to the
application.  When the bit is free the block is still part of the free heap
space. */
static size_t xBlockAllocatedBit = 0;

/*-----------------------------------------------------------*/

void *pvPortMalloc(size_t xWantedSize)
{
    BlockLink_t *pxBlock, *pxPreviousBlock, *pxNewBlockLink;
    void        *pvReturn = NULL;

    // vTaskSuspendAll();//在裸机上使用不需要关闭其他线程

    {
        if (pxEnd == NULL)//末尾节点为null代表未初始化，第一次分配内存需要先对堆空间初始化
        {
            prvHeapInit();
        }

       /* 检查内存块是否分配的bit，高一位代表分配标记位，所有管理的的内存有最大值，申请的不能大于最大值 */
        if ((xWantedSize & xBlockAllocatedBit) == 0)
        {
            /* The wanted size must be increased so it can contain a BlockLink_t
             * structure in addition to the requested amount of bytes. */
            if ((xWantedSize > 0) && ((xWantedSize + xHeapStructSize) > xWantedSize)) /* Overflow check */
            {
                xWantedSize += xHeapStructSize;//分配内存是以块的形式分配，每次增加一块

                /*控制字节对齐，保证分配内存字节对齐 */
                if ((xWantedSize & portBYTE_ALIGNMENT_MASK) != 0x00)
                {
                    /* 检查是否溢出 */
                    if ((xWantedSize + (portBYTE_ALIGNMENT - (xWantedSize & portBYTE_ALIGNMENT_MASK)))
                        > xWantedSize)
                    {
                        xWantedSize += (portBYTE_ALIGNMENT - (xWantedSize & portBYTE_ALIGNMENT_MASK));
                    }
                    else
                    {
                        xWantedSize = 0;
                    }
                }
            }
            else
            {
                xWantedSize = 0;
            }
			//剩余内存够才分配
            if ((xWantedSize > 0) && (xWantedSize <= xFreeBytesRemaining))
            {
                /* 遍历链表找到第一个满足要求的节点 */
                pxPreviousBlock = &xStart;
                pxBlock         = xStart.pxNextFreeBlock;

                while ((pxBlock->xBlockSize < xWantedSize) && (pxBlock->pxNextFreeBlock != NULL))
                {
                    pxPreviousBlock = pxBlock;
                    pxBlock         = pxBlock->pxNextFreeBlock;
                }

                /* 找到节点 */
                if (pxBlock != pxEnd)
                {
                    /* pxPreviousBlock->pxNextFreeBlock为空闲块地址，真正返回地址的是块节
					点信息之后的数组 */
                    pvReturn = (void *) (((uint8_t *) pxPreviousBlock->pxNextFreeBlock) + xHeapStructSize);

                    /* 节点从空闲表去除 */
                    pxPreviousBlock->pxNextFreeBlock = pxBlock->pxNextFreeBlock;

                    /* 拆分空闲节点 */
                    if ((pxBlock->xBlockSize - xWantedSize) > heapMINIMUM_BLOCK_SIZE)
                    {
                        //分裂内存块
                        pxNewBlockLink = (void *) (((uint8_t *) pxBlock) + xWantedSize);

                        //计算新节点
                        pxNewBlockLink->xBlockSize = pxBlock->xBlockSize - xWantedSize;
                        pxBlock->xBlockSize        = xWantedSize;

                        /* 将新节点插入空闲快链表 */
                        prvInsertBlockIntoFreeList(pxNewBlockLink);
                    }

                    xFreeBytesRemaining -= pxBlock->xBlockSize;

                    if (xFreeBytesRemaining < xMinimumEverFreeBytesRemaining)
                    {
                        xMinimumEverFreeBytesRemaining = xFreeBytesRemaining;
                    }

                   /*分配块高一位置为标记以分配*/
                    pxBlock->xBlockSize      |= xBlockAllocatedBit;
                    pxBlock->pxNextFreeBlock  = NULL;
                    xNumberOfSuccessfulAllocations++;
                }
            }
        }
    }
    // ( void ) xTaskResumeAll();

    return pvReturn;
}

/*-----------------------------------------------------------*/

void vPortFree(void *pv)
{
    uint8_t     *puc = (uint8_t *) pv;
    BlockLink_t *pxLink;

    if (pv != NULL)
    {
        //内存块释放一个heap结构体
        puc -= xHeapStructSize;
        
        pxLink = (void *) puc;
		//确保该块已经分配，才能进行释放操作
        if ((pxLink->xBlockSize & xBlockAllocatedBit) != 0)
        {
            if (pxLink->pxNextFreeBlock == NULL)
            {
                /*取消最高位的已用标志位*/
                pxLink->xBlockSize &= ~xBlockAllocatedBit;

                // vTaskSuspendAll();//裸机不需要暂停
                {
                    /*插入空闲内存块*/
                    xFreeBytesRemaining += pxLink->xBlockSize;

                    prvInsertBlockIntoFreeList(((BlockLink_t *) pxLink));
                    xNumberOfSuccessfulFrees++;
                }
                // ( void ) xTaskResumeAll();
            }
        }
    }
}

/*-----------------------------------------------------------*/

size_t xPortGetFreeHeapSize( void )
{
	return xFreeBytesRemaining;
}
/*-----------------------------------------------------------*/

size_t xPortGetMinimumEverFreeHeapSize( void )
{
	return xMinimumEverFreeBytesRemaining;
}
/*-----------------------------------------------------------*/

void vPortInitialiseBlocks( void )
{
	/* This just exists to keep the linker quiet. */
}
/*-----------------------------------------------------------*/

static void prvHeapInit( void )  //堆初始化，不需要用户调用，第一次pvPortMalloc()会执行该函数
{
BlockLink_t *pxFirstFreeBlock;
uint8_t *pucAlignedHeap;
size_t uxAddress;
size_t xTotalHeapSize = configTOTAL_HEAP_SIZE;

/* 8字节对齐，因为AAPCS规则要求堆栈8字节对齐 判断&（0111）是否为0*/
	uxAddress = ( size_t ) ucHeap;

	if( ( uxAddress & portBYTE_ALIGNMENT_MASK ) != 0 )  //
	{
		uxAddress += ( portBYTE_ALIGNMENT - 1 );
		uxAddress &= ~( ( size_t ) portBYTE_ALIGNMENT_MASK );
		xTotalHeapSize -= uxAddress - ( size_t ) ucHeap;
	}

	pucAlignedHeap = ( uint8_t * ) uxAddress;

	/* xStart 存储空闲节点连表首节点*/
	xStart.pxNextFreeBlock = ( void * ) pucAlignedHeap;
	xStart.xBlockSize = ( size_t ) 0;

	/* pxEnd 和xStart不同，pxEND是一个指针指向堆空闲末尾，标记空闲块链表的结束*/
	uxAddress = ( ( size_t ) pucAlignedHeap ) + xTotalHeapSize;
	uxAddress -= xHeapStructSize;
	uxAddress &= ~( ( size_t ) portBYTE_ALIGNMENT_MASK );
	pxEnd = ( void * ) uxAddress;
	pxEnd->xBlockSize = 0;
	pxEnd->pxNextFreeBlock = NULL;

	/* 第一个空闲块指针，指向堆空间字节对齐后的第一个节点，
	并且此空闲块的下一个节点是pxend，至此空闲块成链*/
	pxFirstFreeBlock = ( void * ) pucAlignedHeap;
	pxFirstFreeBlock->xBlockSize = uxAddress - ( size_t ) pxFirstFreeBlock;
	pxFirstFreeBlock->pxNextFreeBlock = pxEnd;

	/* 一个统计堆空闲历史最小值，一个显示当前堆空闲字节*/
	xMinimumEverFreeBytesRemaining = pxFirstFreeBlock->xBlockSize;
	xFreeBytesRemaining = pxFirstFreeBlock->xBlockSize;

	/* 代表内存快是否分配的bit */
	xBlockAllocatedBit = ( ( size_t ) 1 ) << ( ( sizeof( size_t ) * heapBITS_PER_BYTE ) - 1 );
}

/*-----------------------------------------------------------*/

static void prvInsertBlockIntoFreeList(BlockLink_t *pxBlockToInsert) /* PRIVILEGED_FUNCTION */
{
    BlockLink_t *pxIterator;
    uint8_t     *puc;

    /* 根据地址遍历，寻找插入点. */
    for (pxIterator = &xStart; pxIterator->pxNextFreeBlock < pxBlockToInsert; pxIterator = pxIterator->pxNextFreeBlock)
    {
        /* Nothing to do here, just iterate to the right position. */
    }

   /* 判找到插入点后判断是否和前一个空闲块连续，连续则合并 */
    puc = (uint8_t *) pxIterator;

    if ((puc + pxIterator->xBlockSize) == (uint8_t *) pxBlockToInsert)
    {
        pxIterator->xBlockSize += pxBlockToInsert->xBlockSize;
        pxBlockToInsert         = pxIterator;
    }

    /* 判找到插入点后判断是否和后一个空闲块连续，连续则合并 ，pxEnd标记堆空间结束是例外，不允许合并*/
    puc = (uint8_t *) pxBlockToInsert;

    if ((puc + pxBlockToInsert->xBlockSize) == (uint8_t *) pxIterator->pxNextFreeBlock)
    {
        if (pxIterator->pxNextFreeBlock != pxEnd)
        {
            /* Form one big block from the two blocks. */
            pxBlockToInsert->xBlockSize      += pxIterator->pxNextFreeBlock->xBlockSize;
            pxBlockToInsert->pxNextFreeBlock  = pxIterator->pxNextFreeBlock->pxNextFreeBlock;
        }
        else
        {
            pxBlockToInsert->pxNextFreeBlock = pxEnd;
        }
    }
    else
    {
        pxBlockToInsert->pxNextFreeBlock = pxIterator->pxNextFreeBlock;
    }

    /* 链表插入操作，当插入节点和前一个节点连续合并不需要执行操作*/
    if (pxIterator != pxBlockToInsert)
    {
        pxIterator->pxNextFreeBlock = pxBlockToInsert;
    }
}


