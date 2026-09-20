/*!
    \file    rtc.c
    \brief   RTC check and config,time_show and time_adjust function
    
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

#include "rtc.h"

/* enter the second interruption,set the second interrupt flag to 1 */
DateTime_t g_DateTime;
uint32_t datetime_to_counter(int year, int month, int day, int hour, int min, int sec);

/*!
    \brief      configure the nested vectored interrupt controller
    \param[in]  none
    \param[out] none
    \retval     none
*/
void nvic_configuration(void)
{
    nvic_priority_group_set(NVIC_PRIGROUP_PRE1_SUB3);
    nvic_irq_enable(RTC_IRQn,1,0);
}

/*!
    \brief      configure the RTC
    \param[in]  none
    \param[out] none
    \retval     none
*/
void rtc_configuration(void)
{
    /* enable PMU and BKPI clocks */
    rcu_periph_clock_enable(RCU_BKPI);
    rcu_periph_clock_enable(RCU_PMU);
    /* allow access to BKP domain */
    pmu_backup_write_enable();

    /* reset backup domain */
    bkp_deinit();

    /* enable LXTAL */
    rcu_osci_on(RCU_LXTAL);
    /* wait till LXTAL is ready */
    rcu_osci_stab_wait(RCU_LXTAL);
    
    /* select RCU_LXTAL as RTC clock source */
    rcu_rtc_clock_config(RCU_RTCSRC_LXTAL);

    /* enable RTC Clock */
    rcu_periph_clock_enable(RCU_RTC);

    /* wait for RTC registers synchronization */
    rtc_register_sync_wait();

    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();

    /* enable the RTC second interrupt*/
  //  rtc_interrupt_enable(RTC_INT_SECOND);

    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();

    /* set RTC prescaler: set RTC period to 1s */
    rtc_prescaler_set(32767);

    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();
}


uint32_t RTCSRC_FLAG = 0;
void rtc_init(void) {
		nvic_configuration();
		/* get RTC clock entry selection */
    RTCSRC_FLAG = GET_BITS(RCU_BDCTL, 8, 9);

    if ((bkp_data_read(BKP_DATA_0) != 0xA5A5) || (0x00 == RTCSRC_FLAG)){
        /* backup data register value is not correct or not yet programmed
        or RTC clock source is not configured (when the first time the program 
        is executed or data in RCU_BDCTL is lost due to Vbat feeding) */
        printf("\r\nThis is a RTC demo!\r\n");
        printf("\r\n\n RTC not yet configured....");

        /* RTC configuration */
        rtc_configuration();

        printf("\r\n RTC configured....");

        /* adjust time by values entred by the user on the hyperterminal */
        time_adjust();

        bkp_data_write(BKP_DATA_0, 0xA5A5);
    }else{
        /* check if the power on reset flag is set */
        if (rcu_flag_get(RCU_FLAG_PORRST) != RESET){
            printf("\r\n\n Power On Reset occurred....");
        }else if (rcu_flag_get(RCU_FLAG_SWRST) != RESET){
            /* check if the pin reset flag is set */
            printf("\r\n\n External Reset occurred....");
        }
        
        /* allow access to BKP domain */
        rcu_periph_clock_enable(RCU_PMU);
        pmu_backup_write_enable();
        
        printf("\r\n No need to configure RTC....");
        /* wait for RTC registers synchronization */
        rtc_register_sync_wait();

        /* enable the RTC second */
   //     rtc_interrupt_enable(RTC_INT_SECOND);
//
        /* wait until last write operation on RTC registers has finished */
        rtc_lwoff_wait();
    }

}



/*!
    \brief      adjust time 
    \param[in]  none
    \param[out] none
    \retval     none
*/
void time_adjust(void)
{
    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();
    /* change the current time */
    rtc_counter_set(datetime_to_counter(2025, 5, 16, 15, 35 ,30));
    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();
}

void set_time(uint8_t yy, uint8_t mm, uint8_t dd, uint8_t hh, uint8_t mi, uint8_t ss)
{
    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();
    /* change the current time */
    rtc_counter_set(datetime_to_counter(2000+yy,mm, dd, hh, mi, ss));
    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();
}

const uint8_t month_days[12] = {
    31,28,31,30,31,30,31,31,30,31,30,31
};

// 判断是否为闰年
uint8_t is_leap_year(int year) {
    return ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0));
}


uint32_t datetime_to_counter(int year, int month, int day, int hour, int min, int sec) {
    uint32_t days = 0;

    // 计算从1970年到当前年份之前的总天数
    for (int y = 1970; y < year; y++) {
        days += is_leap_year(y) ? 366 : 365;
    }

    // 当前年份内的月份天数（注意处理2月闰年）
    for (int m = 1; m < month; m++) {
        days += month_days[m - 1];
        if (m == 2 && is_leap_year(year)) {
            days += 1;
        }
    }

    // 加上当前月的天数（day-1，因为今天的秒还没算）
    days += (day - 1);

    // 总秒数 = 天 * 86400 + 时 * 3600 + 分 * 60 + 秒
    uint32_t seconds = days * 86400 + hour * 3600 + min * 60 + sec;
    return seconds;
}

void counter_to_datetime(uint32_t seconds,
                         uint8_t* year, uint8_t* month, uint8_t* day,
                         uint8_t* hour, uint8_t* min, uint8_t* sec) {
    uint32_t days = seconds / 86400;
    uint32_t remain = seconds % 86400;
													 
		uint16_t yy;

    *hour = remain / 3600;
    remain %= 3600;
    *min = remain / 60;
    *sec = remain % 60;

    yy = 1970;

    while (1) {
        int year_days = is_leap_year(yy) ? 366 : 365;
        if (days >= year_days) {
            days -= year_days;
            (yy)++;
        } else {
            break;
        }
    }

    *month = 1;
    while (1) {
        uint32_t mdays = month_days[*month - 1];
        if (*month == 2 && is_leap_year(yy)) {
            mdays++;
        }

        if (days >= mdays) {
            days -= mdays;
            (*month)++;
        } else {
            break;
        }
    }
		*year = yy-2000;
    *day = days + 1;
}


void printf_time(void) {
    counter_to_datetime(rtc_counter_get(), &g_DateTime.year, &g_DateTime.month, &g_DateTime.day, &g_DateTime.hour, &g_DateTime.min, &g_DateTime.sec);
    //printf("Parsed time = 20%02d/%02d/%02d %02d:%02d:%02d\n", g_DateTime.year, g_DateTime.month, g_DateTime.day, g_DateTime.hour, g_DateTime.min, g_DateTime.sec);

}
