#include "usart.h"
#include "gpio.h"
#include "systick.h"
#include <stdio.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define USART0_RDATA_ADDRESS      ((uint32_t)&USART_DATA(USART0))
#define USART1_RDATA_ADDRESS      ((uint32_t)&USART_DATA(USART1))

UART_Recv_t uart0_rx;
UART_Recv_t uart1_rx;


UART_Recv_t *get_UART_Recv_t(USART_Num_e usart_num)
{
    UART_Recv_t *p;
    if (usart_num == usart0) {

        p = &uart0_rx;

    } else if (usart_num == usart1) {

        p = &uart1_rx;

    } else {

        p = NULL;
    }
    return p;
}



void usart_nvic_config(void)
{
    nvic_irq_enable(USART0_IRQn, 0, 0);
    nvic_irq_enable(USART1_IRQn, 0, 0);
}

void dma_config(void)
{
    dma_parameter_struct dma_init_struct;

    rcu_periph_clock_enable(RCU_DMA0);

    /* deinitialize DMA channel4 (USART0 rx) */
    dma_deinit(DMA0, DMA_CH4);
    dma_init_struct.direction = DMA_PERIPHERAL_TO_MEMORY;
    dma_init_struct.memory_addr = (uint32_t)uart0_rx.dma_buffer;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.number = UART_RX_BUF_LEN;
    dma_init_struct.periph_addr = USART0_RDATA_ADDRESS;
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
    dma_init(DMA0, DMA_CH4, &dma_init_struct);
    /* configure DMA mode */
    dma_circulation_disable(DMA0, DMA_CH4);
    /* enable DMA channel4 */
    dma_channel_enable(DMA0, DMA_CH4);



    dma_deinit(DMA0, DMA_CH5);
    dma_init_struct.direction = DMA_PERIPHERAL_TO_MEMORY;
    dma_init_struct.memory_addr = (uint32_t)uart1_rx.dma_buffer;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.number = UART_RX_BUF_LEN;
    dma_init_struct.periph_addr = USART1_RDATA_ADDRESS;
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
    dma_init(DMA0, DMA_CH5, &dma_init_struct);
    /* configure DMA mode */
    dma_circulation_disable(DMA0, DMA_CH5);
    /* enable DMA channel4 */
    dma_channel_enable(DMA0, DMA_CH5);
}


void usart_config(uint32_t rs485Baudrate, uint32_t btBaudrate, uint32_t dbBaudrate)
{
    /**RS485 ´®¿Ú**/
    /* enable GPIO clock */
    rcu_periph_clock_enable(RCU_GPIOA);

    /* enable USART clock */
    rcu_periph_clock_enable(RCU_USART0);
	
		//rcu_periph_clock_enable(RCU_AF);
	

    /* connect port to USARTx_Tx */
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);

    /* connect port to USARTx_Rx */
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_10);

			
		usart_deinit(USART0);
    usart_baudrate_set(USART0, rs485Baudrate);
		usart_word_length_set(USART0, USART_WL_8BIT);
    usart_stop_bit_set(USART0, USART_STB_1BIT);
    usart_parity_config(USART0, USART_PM_NONE);
    usart_hardware_flow_rts_config(USART0, USART_RTS_DISABLE);
    usart_hardware_flow_cts_config(USART0, USART_CTS_DISABLE);
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
    usart_dma_receive_config(USART0, USART_RECEIVE_DMA_ENABLE);
    usart_enable(USART0);
    usart_interrupt_enable(USART0, USART_INT_IDLE);


    /**À¶ÑÀ ´®¿Ú**/
    /* enable GPIO clock */
    rcu_periph_clock_enable(RCU_GPIOA);

    /* enable USART clock */
    rcu_periph_clock_enable(RCU_USART1);

    /* connect port to USARTx_Tx */
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_2);

    /* connect port to USARTx_Rx */
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_3);

    /* USART configure */
    usart_deinit(USART1);
    usart_baudrate_set(USART1, btBaudrate);
		usart_word_length_set(USART1, USART_WL_8BIT);
    usart_stop_bit_set(USART1, USART_STB_1BIT);
    usart_parity_config(USART1, USART_PM_NONE);
    usart_hardware_flow_rts_config(USART1, USART_RTS_DISABLE);
    usart_hardware_flow_cts_config(USART1, USART_CTS_DISABLE);
    usart_receive_config(USART1, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART1, USART_TRANSMIT_ENABLE);
    usart_dma_receive_config(USART1, USART_RECEIVE_DMA_ENABLE);
    usart_enable(USART1);
    usart_interrupt_enable(USART1, USART_INT_IDLE);

    /**debug ´®¿Ú**/
    /* enable GPIO clock */
    rcu_periph_clock_enable(RCU_GPIOC);
//GPIO_USART0_REMAP
    /* enable USART clock */
    rcu_periph_clock_enable(RCU_UART3);

    /* connect port to USARTx_Tx */
    gpio_init(GPIOC, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10);

    /* connect port to USARTx_Rx */
    gpio_init(GPIOC, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_11);

    /* USART configure */
   // usart_deinit(UART4);
   // usart_baudrate_set(UART4, dbBaudrate);
   // usart_receive_config(UART4, USART_RECEIVE_DISABLE);
   // usart_transmit_config(UART4, USART_TRANSMIT_ENABLE);
   // usart_enable(UART4);
		
		
		
		usart_deinit(UART3);
    usart_baudrate_set(UART3, dbBaudrate);
    usart_word_length_set(UART3, USART_WL_8BIT);
    usart_stop_bit_set(UART3, USART_STB_1BIT);
    usart_parity_config(UART3, USART_PM_NONE);
    usart_hardware_flow_rts_config(UART3, USART_RTS_DISABLE);
    usart_hardware_flow_cts_config(UART3, USART_CTS_DISABLE);
    usart_receive_config(UART3, USART_RECEIVE_ENABLE);
    usart_transmit_config(UART3, USART_TRANSMIT_ENABLE);
    usart_enable(UART3);
}


void rs485_sendbuff(uint8_t *buff, uint16_t length)
{
    uint16_t tx_count = 0;
    RS485_DE_HIGH();
    for(tx_count = 0; tx_count < length; tx_count++) {
        while(RESET == usart_flag_get(USART0, USART_FLAG_TBE));
        usart_data_transmit(USART0, buff[tx_count]);
    }
    delay_1ms(10);
    RS485_DE_LOW();
}


void bt_sendbuff(uint8_t *buff, uint16_t length)
{
    uint16_t tx_count = 0;
		printf("bt:%s\n", (char *)buff);
	
		for (uint16_t i = 0; i < length; i++) {
        printf("%02X ", buff[i]);
    }
    printf("\n");
	
    for(tx_count = 0; tx_count < length; tx_count++) {
        while(RESET == usart_flag_get(USART1, USART_FLAG_TBE));
        usart_data_transmit(USART1, buff[tx_count]);
    }
}

/* retarget the C library printf function to the USART */
int fputc(int ch, FILE *f)
{
    usart_data_transmit(UART3, (uint8_t)ch);
    while(RESET == usart_flag_get(UART3, USART_FLAG_TBE));
    return ch;
}