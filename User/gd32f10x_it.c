/*!
    \file    gd32f10x_it.c
    \brief   interrupt service routines

    \version 2024-12-20, V2.5.0, firmware for GD32F10x
*/

/*
    Copyright (c) 2024, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors
       may be used to endorse or promote products derived from this software without
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
OF SUCH DAMAGE.
*/
#include "gd32f10x_it.h"
#include "main.h"
#include "systick.h"
#include "usart.h"
#include <string.h>
#include "adc.h"
#include "bw236.h"
/*!
    \brief      this function handles NMI exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void NMI_Handler(void)
{
    /* if NMI exception occurs, go to infinite loop */
    while(1) {
    }
}

/*!
    \brief      this function handles HardFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void HardFault_Handler(void)
{
    /* if Hard Fault exception occurs, go to infinite loop */
    while(1) {
    }
}

/*!
    \brief      this function handles MemManage exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void MemManage_Handler(void)
{
    /* if Memory Manage exception occurs, go to infinite loop */
    while(1) {
    }
}

/*!
    \brief      this function handles BusFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void BusFault_Handler(void)
{
    /* if Bus Fault exception occurs, go to infinite loop */
    while(1) {
    }
}

/*!
    \brief      this function handles UsageFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void UsageFault_Handler(void)
{
    /* if Usage Fault exception occurs, go to infinite loop */
    while(1) {
    }
}

/*!
    \brief      this function handles SVC exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void SVC_Handler(void)
{
    /* if SVC exception occurs, go to infinite loop */
    while(1) {
    }
}

/*!
    \brief      this function handles DebugMon exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void DebugMon_Handler(void)
{
    /* if DebugMon exception occurs, go to infinite loop */
    while(1) {
    }
}

/*!
    \brief      this function handles PendSV exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void PendSV_Handler(void)
{
    /* if PendSV exception occurs, go to infinite loop */
    while(1) {
    }
}

/*!
    \brief      this function handles SysTick exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void SysTick_Handler(void)
{
    delay_decrement();
    HAL_IncTick();
    BSP_Timer_Handler(1);

}


void USART0_IRQHandler(void)
{
    UART_Recv_t *p;
    if(RESET != usart_interrupt_flag_get(USART0, USART_INT_FLAG_IDLE)) {
        /* clear IDLE flag */
        usart_data_receive(USART0);
        /* number of data received */

        p = get_UART_Recv_t(usart0);
        if (p == NULL) return;
        p->length = UART_RX_BUF_LEN - (uint16_t)(dma_transfer_number_get(DMA0, DMA_CH4));
				memcpy(p->buffer, p->dma_buffer, p->length);
        p->complete = 1;



        /* disable DMA and reconfigure */
        dma_channel_disable(DMA0, DMA_CH4);
        dma_transfer_number_config(DMA0, DMA_CH4, UART_RX_BUF_LEN);
        dma_channel_enable(DMA0, DMA_CH4);
    }
}
#include <string.h> //for use memmem

void USART1_IRQHandler(void)
{
    UART_Recv_t *p;
    uint8_t length;
    if(RESET != usart_interrupt_flag_get(USART1, USART_INT_FLAG_IDLE)) {
        /* clear IDLE flag */
        usart_data_receive(USART1);
        /* number of data received */

        p = get_UART_Recv_t(usart1);
        if (p == NULL) return;
        length = UART_RX_BUF_LEN - (uint16_t)(dma_transfer_number_get(DMA0, DMA_CH5));


        if (ExitTransparentState == BT_EXIT_WAIT_A) {
            for (int i = 0; i < length; i++) {
                if (p->dma_buffer[i] == 'a') {
                    bt_sendbuff("a", 1);
                    ExitTransparentState = BT_EXIT_SENT_A;
										memset(p->dma_buffer, 0, UART_RX_BUF_LEN);
                    break;
                }

            }
            return;
        } else if (ExitTransparentState == BT_EXIT_SENT_A) {
            p->dma_buffer[length] = '\0';
            if (strstr((char *)p->dma_buffer, "+ok")) {
                ExitTransparentState = BT_EXIT_SUCCESS;
								memset(p->dma_buffer, 0, UART_RX_BUF_LEN);
            }
            return;
        }
        p->dma_length = length;
        AT_UART_IdleHandler(&at_cmd, p->dma_buffer, p->dma_length);

        /* disable DMA and reconfigure */
        dma_channel_disable(DMA0, DMA_CH5);
        dma_transfer_number_config(DMA0, DMA_CH5, UART_RX_BUF_LEN);
        dma_channel_enable(DMA0, DMA_CH5);
    }
}


void USBD_LP_CAN0_RX0_IRQHandler(void)
{
    can_receive_message_struct rx_message;

    // 清空接收结构体
    memset(&rx_message, 0, sizeof(rx_message));

    // 直接读取 FIFO0 中的数据（中断触发即说明有数据）
    can_message_receive(CAN0, CAN_FIFO0, &rx_message);

    // 根据 ID 或者其他条件分类处理
    //if (rx_message.rx_sfid == 0x123) {
    //    // 处理逻辑
    //}
}



#define BUFFER_SIZE 10 // 缓冲区大小为10，用于保存最近的十个数值
// 转换通道个数


// 用于保存每个通道最近十个转换的值
static uint16_t adc_buffer[NOFCHANEL][BUFFER_SIZE] = {0};

// 缓冲区的索引
static uint8_t buffer_index = 0;

// 用于存储每个通道的中间四个数值的临时数组
static uint16_t middle_values[NOFCHANEL][4];

// 排序函数：从小到大
void sort(uint16_t *array, uint8_t size) {
    uint8_t i, j;
    uint16_t temp;
    for (i = 0; i < size - 1; i++) {
        for (j = i + 1; j < size; j++) {
            if (array[i] > array[j]) {
                temp = array[i];
                array[i] = array[j];
                array[j] = temp;
            }
        }
    }
}

// 计算平均值
uint16_t calculate_average(uint16_t *array, uint8_t size) {
    uint32_t sum = 0;
    for (uint8_t i = 0; i < size; i++) {
        sum += array[i];
    }
    return (uint16_t)(sum / size);
}



// DMA1 通道 1 中断服务程序
void DMA0_Channel0_IRQHandler(void) {

    if (dma_flag_get(DMA0, DMA_CH0, DMA_FLAG_FTF)) {
        // 清除中断标志
        dma_flag_clear(DMA0, DMA_CH0, DMA_FLAG_FTF);

        // 将最新的 ADC 值存入缓冲区
        for (uint8_t channel = 0; channel < NOFCHANEL; channel++) {
            adc_buffer[channel][buffer_index] = adc_value[channel];
        }

        // 更新缓冲区索引，保证其始终在 0 到 BUFFER_SIZE-1 之间
        buffer_index = (buffer_index + 1) % BUFFER_SIZE;

        // 如果缓冲区已经满了，进行排序和计算平均值
        if (buffer_index == 0) {
            // 对每个通道的最近十个数值进行排序和计算平均值
            for (uint8_t channel = 0; channel < NOFCHANEL; channel++) {
                // 排序最近的十个数值
                sort(adc_buffer[channel], BUFFER_SIZE);

                // 取中间的四个数值（第 3 至第 6 个）
                for (uint8_t i = 3; i < 7; i++) {
                    middle_values[channel][i - 3] = adc_buffer[channel][i];
                }

                // 计算这四个数值的平均值
                filtered_adc_values[channel]= calculate_average(middle_values[channel], 4);
								
            }
        }
    }
}

