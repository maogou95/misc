/*!
    \file    rtc.h
    \brief   headfile of RTC check and config,time_show and time_adjust function
    
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

#ifndef __RTC_H
#define __RTC_H

#include "gd32f10x.h"
#include <stdio.h>


typedef struct {
    uint8_t year;   // 年，例如 2025
    uint8_t  month;  // 月份 1-12
    uint8_t  day;    // 日期 1-31
    uint8_t  hour;   // 小时 0-23
    uint8_t  min;    // 分钟 0-59
    uint8_t  sec;    // 秒 0-59
} DateTime_t;

extern DateTime_t g_DateTime;
void nvic_configuration(void);
void rtc_configuration(void);
uint32_t time_regulate(void);
void time_adjust(void);
void time_display(uint32_t timevar);
void time_show(void);
uint8_t usart_scanf(uint32_t value);
void set_time(uint8_t yy, uint8_t mm, uint8_t dd, uint8_t hh, uint8_t mi, uint8_t ss);

void rtc_init(void);
void printf_time(void);
#endif /* __RTC_H */

