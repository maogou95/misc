#ifndef __MALLOC_H
#define __MALLOC_H
typedef unsigned          char u8;
typedef unsigned short     int u16;
typedef unsigned           int u32;


#define SRAMIN 0
#include <stdlib.h>
#include <stdint.h>
void vPortFree(void *pv);
void *pvPortMalloc(size_t xWantedSize);
#endif













