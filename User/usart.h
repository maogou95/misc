#ifndef USART_H
#define USART_H
#include "gd32f10x.h"
#include <stdio.h>

#define UART_RX_BUF_LEN 512

typedef struct {
    uint8_t  buffer[UART_RX_BUF_LEN];  // 接收缓冲区
		uint8_t  dma_buffer[UART_RX_BUF_LEN];  // 接收缓冲区
    uint16_t length;                   // 当前接收到的字节数
		uint16_t dma_length;                   // 当前接收到的字节数
    uint8_t  complete;                 // 接收完成标志（如一帧接收完成）
		uint8_t source; //0:bt 1:wifi
} UART_Recv_t;

typedef enum {
	usart0,
	usart1,
	usart2,
	usart3,
}USART_Num_e;




/* led spark function */
void usart_nvic_config(void);
void usart_config(uint32_t rs485Baudrate, uint32_t btBaudrate, uint32_t dbBaudrate);
void dma_config(void);
UART_Recv_t *get_UART_Recv_t(USART_Num_e usart_num) ;

void rs485_sendbuff(uint8_t *buff, uint16_t length);
void bt_sendbuff(uint8_t *buff, uint16_t length);

#endif /* USART_H */

