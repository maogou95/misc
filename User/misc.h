#ifndef MISC_H_
#define MISC_H_
//==========================================
//#include "_TYPE.h"
//==========================================
#include "main.h"

 #define uint uint16_t
 #define uchar uint8_t
//------------------------------------------
//#ifdef _MODBUS_CRC_C_
//#define  global	   idata
//#else
//#define  global extern	 idata
//#endif
//------------------------------------------

 //==========================================
//#undef 	global
//==========================================
 uint crc16( uchar *puchMsg, uint usDataLen );

 uint16_t RS485_CRC(void *buf,uint16_t bsize);

uint8_t CRC8_Tab(uint8_t *ucPtr,uint8_t ucLen);
//==========================================
#endif

