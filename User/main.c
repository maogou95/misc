/*!
    \file    main.c
    \brief   led spark with systick, USART print and key example

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

#include "gd32f10x.h"
#include "systick.h"
#include <stdio.h>
#include "main.h"
#include "spi_flash.h"
#include "i2c.h"
#include "misc.h"
#include "can.h"
#include "rtc.h"


/*!
    \brief      toggle the led every 500ms
    \param[in]  none
    \param[out] none
    \retval     none
*/
extern int lfs_spi_flash_init(struct lfs_config *cfg);
extern struct lfs_config cfg;
lfs_t lfs;
channelInfo_t g_channelInfo[4];

#define MAKE_CAN_EXT_ID(dev_type, dev_id)   (((uint32_t)(dev_type) << 16) | ((uint32_t)(dev_id) & 0xFFFF))


devInfo_t g_devInfo = {
    .can_type_num = MAKE_CAN_EXT_ID(0x65,1),
    .dev_num = 1,
    .rs485_baudrate = 9600,
    .can_baudrate = 500,
    .can_or_rs485_switch = CAN,
    .software_version = {1,3,0},
    .hardware_version = {1,0,0},
};

Thresholds_t Thresholds = {
    .voltage_upper_threshold = 24.0f,
    .voltage_lower_threshold = 8.0f,
    .temperature_upper_threshold = 50.0f,
    .temperature_lower_threshold = 10.0f,
    .humidity_upper_threshold = 150.0f,
    .humidity_lower_threshold = 30.0f,
};

history_flag_t history_flag = {
    .rs485_hs_flag = 0,
    .bt_hs_flag = 0,

};
uint16_t g_actual_length[4];
struct BSP_TIMER ProcessSaveHsTimer;
struct BSP_TIMER CollectTimer;

lfs_file_t file_dev;
lfs_file_t file_sensor;
lfs_file_t file_hs;

void ProcessSaveHsCallback(void *user_data);
uint32_t fill_realdata_buff(uint8_t **ppdata);
/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/

void gud600_FixDisplay(void) {

    LCD_ShowChinese(5,6+182,"华虹科技",GREEN,BLACK,16,0);
    LCD_ShowChinese(35,15,"四通道离层位移传感器",GREEN,BLACK,24,0);
    LCD_ShowChinese(50,15+40,"通道一：",GREEN,BLACK,24,0);
    LCD_ShowChinese(50,15+70,"通道二：",GREEN,BLACK,24,0);
    LCD_ShowChinese(50,15+100,"通道三：",GREEN,BLACK,24,0);
    LCD_ShowChinese(50,15+130,"通道四：",GREEN,BLACK,24,0);

    LCD_ShowChinese(110,6+182,"电压：",GREEN,BLACK,16,0);
    LCD_ShowChinese(110,6+182+20,"温度：",GREEN,BLACK,16,0);
    LCD_ShowChinese(210,6+182,"湿度：",GREEN,BLACK,16,0);
    LCD_ShowChinese(210,6+182+20,"编号：",GREEN,BLACK,16,0);

}


void gud600_VarDisplay(void) {
    char str[30];
    int len = 0;

    for (uint8_t n=0; n<(sizeof (g_actual_length)/sizeof (uint16_t)); ++n) {
        len = sprintf(str, "%dmm", g_actual_length[n]);  // len = 4
        while (len < 10) {
            str[len++] = ' ';
        }
        str[len] = '\0';
        LCD_ShowString(150, 15+40+n*30, str, RED, GREEN, 24, 0);
    }

    float environment[4] = {g_environment.voltageVal, g_environment.Humi, g_environment.Temp, (float)g_devInfo.dev_num};

    for (uint8_t i = 0; i < 2; ++i) {
        for (uint8_t j = 0; j < 2; ++j) {
            uint8_t idx = i * 2 + j;
						if (idx == 3) {
							len = sprintf(str, "%d", (uint16_t)environment[idx]);
						} else {
							len = sprintf(str, "%.1f", environment[idx]);
						}
            
            while (len < 6) {
                str[len++] = ' ';
            }
            str[len] = '\0';
            LCD_ShowString(120 + j * 100 + 38, 6 + 182 + i * 20, str, RED, GREEN, 16, 0);
        }
    }

    len = sprintf(str, "%02d:%02d:%02d", g_DateTime.hour, g_DateTime.min, g_DateTime.sec);  // len = 4
    while (len < 9) {
        str[len++] = ' ';
    }
    LCD_ShowString(5, 6+182+20, str, RED, GREEN, 16, 0);

}





void gud600_GetSample(void *user_data) {
    uint16_t val1, val2, val3, val4;
    uint16_t *values[4] = { &val1, &val2, &val3, &val4 };
    get_adc_convert_voltage_value(&val1, &val2, &val3, &val4);
    ReadSernerData();
    for (unsigned char n = 0; n < 4; ++n) {
        if (g_channelInfo[n].zero_point == 0) {
            g_actual_length[n] = 0;
        } else {
            int16_t delta = (int16_t)(*values[n]) - (int16_t)(g_channelInfo[n].zero_point);
            if (delta < 0) delta = 0;

            float temp = ((float)delta)*g_channelInfo[n].KA + g_channelInfo[n].KB; ///// 2.65f;
            if (temp > 300.0f) temp = 300.0f;

            g_actual_length[n] = (uint16_t)temp;
						printf("GetSample%dchannel:	value=%d,offset=%d\n",n,*values[n],g_channelInfo[n].zero_point);
						
        }
    }
		
		
    if (g_devInfo.can_or_rs485_switch == CAN) {
			uint8_t buff[8];
			for (uint8_t n=0; n<sizeof (g_actual_length)/sizeof (g_actual_length[0]); ++n) {
				CAN_BuildRealtimeFrame(buff, 0x07, n, n, 0x07, 0);
				uint16_t id = CAN_ID(MSG_REALTIME, g_devInfo.can_type_num);
				if (can0_send_message(id, buff, sizeof (buff), 0) != SUCCESS) {
            printf("can message not send!\r\n");
        }
			} 
    }
}



unsigned short int fill_pack_to_buff(uint8_t Dtype, uint32_t Dnum, uint8_t cmd, uint8_t *buff, uint8_t *data, uint16_t lenth)
{
    basepack_t *basepack;
    basepack = (basepack_t *)pvPortMalloc(sizeof (basepack_t));
    memset(basepack->Head, 0x25, 4);
    memset(basepack->Tail, 0x40, 4);
    basepack->eSourInstrType = DEVICETYPE;
    basepack->uSourInstrNum = g_devInfo.dev_num;
    basepack->eDestInstrType = Dtype;
    basepack->uDestInstrNum = Dnum;
    basepack->uACK = NACK;
    basepack->uCommand = cmd;
    basepack->uDataLen = lenth;
    memcpy(buff, basepack, sizeof (shortbasepack_t));
    if (data != NULL) {
        memcpy(buff + sizeof (shortbasepack_t), data, lenth);
    }

    basepack->uCrc = RS485_CRC(buff+4,sizeof (shortbasepack_t) - 4 + lenth);
    memcpy(buff + sizeof (shortbasepack_t) + lenth, (uint8_t *) &basepack->uCrc, 6);

    vPortFree(basepack);
    return 24+lenth;
}

void write_dev_info(void)
{
    if (LFS_ERR_OK != lfs_file_open(&lfs, &file_dev, DEV_INFO_PATH, LFS_O_RDWR | LFS_O_CREAT | LFS_O_TRUNC)) {
        return;
    }
    lfs_file_rewind(&lfs, &file_dev);
    lfs_file_write(&lfs, &file_dev, &g_devInfo, sizeof(devInfo_t));
    lfs_file_write(&lfs, &file_dev, &Thresholds, sizeof(Thresholds_t));
    lfs_file_sync(&lfs, &file_dev);
    lfs_file_close(&lfs, &file_dev);

}
void write_channel_info(void)
{
    if (LFS_ERR_OK != lfs_file_open(&lfs, &file_sensor, SENSOR_INFO_PATH, LFS_O_RDWR | LFS_O_CREAT | LFS_O_TRUNC)) {
        return;
    }
    lfs_file_rewind(&lfs, &file_sensor);
    lfs_file_write(&lfs, &file_sensor, g_channelInfo, sizeof(channelInfo_t)*4);
    lfs_file_sync(&lfs, &file_sensor);
    lfs_file_close(&lfs, &file_sensor);

}

void read_dev_info(void) {
    devInfo_t old_info;
    write_dev_info();  // 写入当前 dev_info
    // 尝试打开文件
    if (LFS_ERR_OK != lfs_file_open(&lfs, &file_dev, DEV_INFO_PATH, LFS_O_RDWR | LFS_O_CREAT)) {
        return;
    }

    // 如果文件为空，说明未写入过，初始化
    if (lfs_file_size(&lfs, &file_dev) == 0) {
        lfs_file_close(&lfs, &file_dev);
        write_dev_info();  // 写入当前 dev_info
        return;
    }

    // 读取旧数据
    lfs_file_rewind(&lfs, &file_dev);
    lfs_file_read(&lfs, &file_dev, &old_info, sizeof(devInfo_t));
    lfs_file_read(&lfs, &file_dev, &Thresholds, sizeof(Thresholds_t));
    lfs_file_close(&lfs, &file_dev);

    // 比较版本号，如果当前版本号更高则写入
    if ((g_devInfo.software_version[0] > old_info.software_version[0]) ||
            (g_devInfo.software_version[0] == old_info.software_version[0] &&
             g_devInfo.software_version[1] > old_info.software_version[1]) ||
            (g_devInfo.software_version[0] == old_info.software_version[0] &&
             g_devInfo.software_version[1] == old_info.software_version[1] &&
             g_devInfo.software_version[2] > old_info.software_version[2])) {

        write_dev_info();  // 写入新数据
    } else {
        // 如果旧版本 >= 当前版本，可以选择保留旧的，也可以加载到 dev_info 中
        g_devInfo = old_info;
    }
}


void read_channel_info(void) {
    // 尝试打开文件
    if (LFS_ERR_OK != lfs_file_open(&lfs, &file_sensor, SENSOR_INFO_PATH, LFS_O_RDWR | LFS_O_CREAT)) {
        return;
    }

    // 如果文件为空，说明未写入过，初始化
    if (lfs_file_size(&lfs, &file_sensor) == 0) {
        lfs_file_close(&lfs, &file_sensor);
        write_channel_info();  // 写入当前 dev_info
        return;
    }

    // 读取旧数据
    lfs_file_rewind(&lfs, &file_sensor);
    lfs_file_read(&lfs, &file_sensor, g_channelInfo, sizeof(channelInfo_t)*4);
    lfs_file_close(&lfs, &file_sensor);
}

// 示例处理函数：设置设备信息
void handle_set_device_info(const uint8_t* data, uint16_t len, uint8_t source) {
    printf("[CMD 0x01] 设置设备信息: ");
    for (uint16_t i = 0; i < len; ++i) {
        printf("%02X ", data[i]);
    }
    printf("\n");

    // 设置设备信息的逻辑
    uint8_t *pbuff;
    uint16_t pbuff_lenth;
    uint16_t temp, retemp;
    uint32_t up_baudrate,downbaudrate,id;
    uint8_t reset_flag;
    shortbasepack_t *shortpack;
    shortpack = (shortbasepack_t *)data;
    pbuff = (uint8_t *)pvPortMalloc(24);
    temp = g_devInfo.save_interval;
    up_baudrate = g_devInfo.rs485_baudrate;
    id = g_devInfo.dev_num;

    memcpy(&g_devInfo, data + sizeof (shortbasepack_t), sizeof (g_devInfo));

    if (g_devInfo.rs485_baudrate != up_baudrate) {

        usart_disable(USART0);
        usart_baudrate_set(USART0, g_devInfo.rs485_baudrate);
        usart_enable(USART0);

    }


    if(g_devInfo.save_interval != 0 && g_devInfo.save_interval != temp) {

        BSP_Timer_Init(&ProcessSaveHsTimer,
                       ProcessSaveHsCallback,
                       g_devInfo.save_interval*60*1000,
                       0,
                       TIMER_TYPE_SOFTWARE);
        BSP_Timer_Restart(&ProcessSaveHsTimer);

    }
    if (g_devInfo.save_interval == 0) {

        BSP_Timer_Pause(&ProcessSaveHsTimer);

    }

    reset_flag = (id != g_devInfo.dev_num);

    write_dev_info();
    pbuff_lenth = fill_pack_to_buff(shortpack->eSourInstrType, shortpack->uSourInstrNum, CMD_SET_DEVICE_INFO, pbuff, NULL, 0);
    if (source == SOURCE_RS485) {
        rs485_sendbuff(pbuff, pbuff_lenth);

    } else {
        sendBTdata(pbuff, pbuff_lenth);
    }
    vPortFree(pbuff);
    //if (reset_flag) {
    //    //修改蓝牙名称
    //		AT_Name(g_devInfo.dev_num);
    //}


}


volatile ExitTransparentState_t ExitTransparentState = BT_EXIT_IDLE;
extern uint8_t is_transparent_mode;
int Exit_TransparentMode_Blocking(uint32_t timeout_ms)
{
    uint32_t exit_timer = 0;
    ExitTransparentState = BT_EXIT_WAIT_A;
    bt_sendbuff("+++", 3);
    exit_timer = HAL_GetTick();

    while ((HAL_GetTick() - exit_timer) < timeout_ms) {
        if (ExitTransparentState == BT_EXIT_SUCCESS) {
            is_transparent_mode = 0;
            printf("Exit_TransparentMode_Blocking SUCCESS\r\n");
            return 0;
        }
    }
    ExitTransparentState = BT_EXIT_IDLE;
    printf("Exit_TransparentMode_Blocking FAILED\r\n");
    return -1;
}


// 示例处理函数：同步时间（从主机获取时间同步）
void handle_exit_transparentmode(const uint8_t* data, uint16_t len, uint8_t source) {
    printf("[CMD 0x09]");
    for (uint16_t i = 0; i < len; ++i) {
        printf("%02X ", data[i]);
    }
    printf("\n");
    uint8_t *pbuff;
    uint16_t pbuff_lenth;
    shortbasepack_t *shortpack;
    shortpack = (shortbasepack_t *)data;
    pbuff = (uint8_t *)pvPortMalloc(24);

    Exit_TransparentMode_Blocking(1000);
    AT_ExitTpmod();
    pbuff_lenth = fill_pack_to_buff(shortpack->eSourInstrType, shortpack->uSourInstrNum, CMD_EXIT_TRANSPARENT_MODE, pbuff, NULL, 0);


    if (source == SOURCE_RS485) {
        rs485_sendbuff(pbuff, pbuff_lenth);


    } else {

        char *hexbuf = (char *)pvPortMalloc(50);
        bytes_to_hexstr(pbuff, 24, hexbuf);
        hexbuf[48] = '\r';
        hexbuf[49] = '\n';

        sendBTdata(hexbuf, 50);
        vPortFree(hexbuf);
    }
    vPortFree(pbuff);

}


// 示例处理函数：查询设备信息
void handle_get_device_info(const uint8_t* data, uint16_t len, uint8_t source) {
    (void)data; // 未使用
    printf("[CMD 0x02] 查询设备信息\n");
    // 查询设备信息的逻辑
    uint8_t *pbuff;
    uint16_t pbuff_lenth;
    shortbasepack_t *shortpack;
    shortpack = (shortbasepack_t *)data;
    pbuff = (uint8_t *)pvPortMalloc(24 + sizeof (devInfo_t));
    pbuff_lenth = fill_pack_to_buff(shortpack->eSourInstrType, shortpack->uSourInstrNum, CMD_GET_DEVICE_INFO, pbuff, (unsigned char *)&g_devInfo, sizeof (devInfo_t));

    if (source == SOURCE_RS485) {
        rs485_sendbuff(pbuff, pbuff_lenth);

    } else {
        sendBTdata(pbuff, pbuff_lenth);
    }

    vPortFree(pbuff);
}

// 示例处理函数：校准通道
void handle_calibrate_channel(const uint8_t* data, uint16_t len, uint8_t source) {
    printf("[CMD 0x03] 校准通道参数: ");
    for (uint16_t i = 0; i < len; ++i) {
        printf("%02X ", data[i]);
    }
    printf("\n");
    uint8_t *pbuff;
    uint16_t pbuff_lenth;
    shortbasepack_t *shortpack;
    shortpack = (shortbasepack_t *)data;
    pbuff = (uint8_t *)pvPortMalloc(24);
    uint8_t channel_num = shortpack->uDataLen;
    uint8_t channel_calibrate[4];
    memcpy(channel_calibrate, data + sizeof (shortbasepack_t), sizeof (uint8_t)*channel_num);
    uint16_t val1, val2, val3, val4;
    uint16_t *values[4] = { &val1, &val2, &val3, &val4 };
    get_adc_convert_voltage_value(&val1, &val2, &val3, &val4);

    if (channel_num<=4) {
        for (uint8_t i=0; i<channel_num; ++i) {
            g_channelInfo[channel_calibrate[i]].zero_point = *values[channel_calibrate[i]];
						printf("校准%d通道: offset=%d\n",channel_calibrate[i],*values[channel_calibrate[i]]);
        }
    }

    write_channel_info();
    pbuff_lenth = fill_pack_to_buff(shortpack->eSourInstrType, shortpack->uSourInstrNum, CMD_CALIBRATE_CHANNEL, pbuff, NULL, 0);
    if (source == SOURCE_RS485) {
        rs485_sendbuff(pbuff, pbuff_lenth);

    } else {
        sendBTdata(pbuff, pbuff_lenth);
    }
    vPortFree(pbuff);

}

// 示例处理函数：复位通道
void handle_reset_channel(const uint8_t* data, uint16_t len, uint8_t source) {


    uint8_t *pbuff;
    uint16_t pbuff_lenth;
    shortbasepack_t *shortpack;
    shortpack = (shortbasepack_t *)data;
    pbuff = (uint8_t *)pvPortMalloc(24);
    uint8_t channel_num = shortpack->uDataLen;
    uint8_t channel_calibrate[4];
    memcpy(channel_calibrate, data + sizeof (shortbasepack_t), sizeof (uint8_t)*channel_num);
    printf("[CMD 0x04] 复位通道: 通道号 = %d\n", channel_num);
    if (channel_num<=4) {
        for (uint8_t i=0; i<channel_num; ++i) {
            g_channelInfo[channel_calibrate[i]].zero_point = 0;
        }
    }

    write_channel_info();
    pbuff_lenth = fill_pack_to_buff(shortpack->eSourInstrType, shortpack->uSourInstrNum, CMD_RESET_CHANNEL, pbuff, NULL, 0);
    if (source == SOURCE_RS485) {
        rs485_sendbuff(pbuff, pbuff_lenth);

    } else {
        sendBTdata(pbuff, pbuff_lenth);
    }
    vPortFree(pbuff);
}

// 示例处理函数：同步时间（从主机获取时间同步）
void handle_sync_time(const uint8_t* data, uint16_t len, uint8_t source) {
    printf("[CMD 0x05] 同步时间: ");
    for (uint16_t i = 0; i < len; ++i) {
        printf("%02X ", data[i]);
    }
    printf("\n");
    uint8_t *pbuff;
    uint16_t pbuff_lenth;
    shortbasepack_t *shortpack;
    shortpack = (shortbasepack_t *)data;
    pbuff = (uint8_t *)pvPortMalloc(24);
    DateTime_t datetime;

    memcpy(&datetime, data + sizeof (shortbasepack_t), sizeof (DateTime_t));
    set_time(datetime.year, datetime.month, datetime.day, datetime.hour, datetime.min, datetime.sec);

    pbuff_lenth = fill_pack_to_buff(shortpack->eSourInstrType, shortpack->uSourInstrNum, CMD_SYNC_TIME, pbuff, NULL, 0);
    if (source == SOURCE_RS485) {
        rs485_sendbuff(pbuff, pbuff_lenth);

    } else {
        sendBTdata(pbuff, pbuff_lenth);
    }
    vPortFree(pbuff);
}



// 示例处理函数：读取实时数据
void handle_read_realtime_data(const uint8_t* data, uint16_t len, uint8_t source) {
    (void)data;
    (void)len;
    printf("[CMD 0x07] 读取实时数据\n");
    // 这里可以模拟返回实时数据
    uint8_t *pbuff;
    uint16_t pbuff_lenth;
    shortbasepack_t *shortpack;
    shortpack = (shortbasepack_t *)data;

    uint8_t *prData;
    uint8_t **rData = &prData;
    uint16_t rData_Length;
    rData_Length = fill_realdata_buff(rData);
    if (rData_Length <= 0) {
        return;
    }

    pbuff = (uint8_t *)pvPortMalloc(24 + rData_Length);

    pbuff_lenth = fill_pack_to_buff(shortpack->eSourInstrType, shortpack->uSourInstrNum, CMD_READ_REALTIME_DATA, pbuff, prData, rData_Length);

    if (source == SOURCE_RS485) {
        rs485_sendbuff(pbuff, pbuff_lenth);

    } else {
        sendBTdata(pbuff, pbuff_lenth);
    }
    vPortFree(prData);
    vPortFree(pbuff);
}

// 示例处理函数：读取历史数据
void handle_read_history_data(const uint8_t* data, uint16_t len, uint8_t source) {
    printf("[CMD 0x08] 读取历史数据\n");

    // 读取历史数据的逻辑
    if (source == SOURCE_RS485) {
        history_flag.rs485_hs_flag = 1;

    } else {
        history_flag.bt_hs_flag = 1;
    }
}


void handle_getKAKB(const uint8_t* data, uint16_t len, uint8_t source) {
    (void)data; // 未使用
    printf("[CMD 0x0A] 获取KAKB: ");
    // 查询设备信息的逻辑
    uint8_t *pbuff;
    uint16_t pbuff_lenth;
		float KAB[4*2*4];
		uint8_t offset = 0;
    shortbasepack_t *shortpack;
    shortpack = (shortbasepack_t *)data;
    pbuff = (uint8_t *)pvPortMalloc(24 + sizeof (g_channelInfo[4]) - sizeof (uint16_t)*4);
	
		for (int n=0; n<4; ++n) {
				memcpy(KAB+offset, &g_channelInfo[n].KA, sizeof (float));
				offset += 4;
				memcpy(KAB+offset, &g_channelInfo[n].KB, sizeof (float));
				offset += 4;
		}
	
    pbuff_lenth = fill_pack_to_buff(shortpack->eSourInstrType, shortpack->uSourInstrNum, CMD_GET_KAB, pbuff, KAB, 4*2*4);

    if (source == SOURCE_RS485) {
        rs485_sendbuff(pbuff, pbuff_lenth);

    } else {
        sendBTdata(pbuff, pbuff_lenth);
    }

    vPortFree(pbuff);
}



void handle_setKAKB(const uint8_t* data, uint16_t len, uint8_t source) {
    printf("[CMD 0x0B] 设置KAKB: ");
    for (uint16_t i = 0; i < len; ++i) {
        printf("%02X ", data[i]);
    }
    printf("\n");

    // 设置设备信息的逻辑
    uint8_t *pbuff;
    uint16_t pbuff_lenth;
		uint8_t offset = 0;
    shortbasepack_t *shortpack;
    shortpack = (shortbasepack_t *)data;
    pbuff = (uint8_t *)pvPortMalloc(24);

		for (int n=0; n<4; ++n) {
				memcpy(&g_channelInfo[n].KA, data + sizeof (shortbasepack_t) + offset, sizeof (float));
				offset += 4;
				memcpy(&g_channelInfo[n].KB, data + sizeof (shortbasepack_t) + offset, sizeof (float));
				offset += 4;
		}
    write_channel_info();
    pbuff_lenth = fill_pack_to_buff(shortpack->eSourInstrType, shortpack->uSourInstrNum, CMD_SET_KAB, pbuff, NULL, 0);
    if (source == SOURCE_RS485) {
        rs485_sendbuff(pbuff, pbuff_lenth);

    } else {
        sendBTdata(pbuff, pbuff_lenth);
    }
    vPortFree(pbuff);
}


static CommandRegistry_t commandTable[MAX_COMMANDS];
static uint8_t commandCount = 0;

int register_command(uint8_t cmd, CommandHandler_t handler) {
    if (commandCount >= MAX_COMMANDS) return -1;
    commandTable[commandCount++] = (CommandRegistry_t) {
        cmd, handler
    };
    return 0;
}

void init_commands() {
    register_command(CMD_SET_DEVICE_INFO,    handle_set_device_info);
    register_command(CMD_GET_DEVICE_INFO,    handle_get_device_info);
    register_command(CMD_CALIBRATE_CHANNEL,  handle_calibrate_channel);
    register_command(CMD_RESET_CHANNEL,      handle_reset_channel);

    register_command(CMD_SYNC_TIME,      handle_sync_time);
    register_command(CMD_SET_TIME,      handle_sync_time);
    register_command(CMD_READ_REALTIME_DATA,      handle_read_realtime_data);
    register_command(CMD_READ_HISTORY_DATA,      handle_read_history_data);
    register_command(CMD_EXIT_TRANSPARENT_MODE,      handle_exit_transparentmode);
	
		register_command(CMD_GET_KAB,      handle_getKAKB);
    register_command(CMD_SET_KAB,      handle_setKAKB);
	
	
	
}


void dispatch_command(uint8_t cmd, const uint8_t* buf, uint16_t len, uint8_t source) {


    for (int i = 0; i < commandCount; ++i) {
        if (commandTable[i].cmd == cmd) {
            commandTable[i].handler(buf, len, source);  // cmd 后面是参数
            return;
        }
    }
    printf("Unknown command: 0x%02X\n", cmd);
}


void process_data(void)
{

    unsigned char *data = NULL;
    unsigned short int lenth = 0;
    uint8_t sou;
    uint8_t *complete;
    UART_Recv_t *BT_buff = get_UART_Recv_t(usart1);
    UART_Recv_t *RS485_buff = get_UART_Recv_t(usart0);

    if (BT_buff->complete) {
        if (AT_cmd_Dw(BT_buff->buffer, BT_buff->length, BT_buff->source))
            BT_buff->complete = 0;
    }


    if (BT_buff->complete) {

        data = BT_buff->buffer;
        lenth = BT_buff->length;
        sou = SOURCE_BLUETOOTH;
        complete = &BT_buff->complete;

    } else if (RS485_buff->complete) {

        data = RS485_buff->buffer;
        lenth = RS485_buff->length;
        sou = SOURCE_RS485;
        complete = &RS485_buff->complete;
    }

    if (data == NULL || lenth == 0) return;

    printf(" rcv lenth: %d\n", lenth);
    for (uint16_t i=0; i<lenth; ++i) {
        printf("0x%02x ",data[i]);
    }
    printf("\n");
    shortbasepack_t *shortpack;
    uint16_t ucrc;


    if (lenth < sizeof (basepack_t)) {
        printf("not enough packet length \r\n");
        *complete = 0;
        return;
    }

    if (shortpack->uDestInstrNum != g_devInfo.dev_num) {
        printf("device num error\r\n");
        *complete = 0;
        return;
    }

    shortpack = (shortbasepack_t *)data;
    if (shortpack->eDestInstrType != 0x65 || shortpack->Head[0] != 0x25 || shortpack->Head[1] != 0x25 ||\
            (shortpack->uDataLen+24) != lenth) {   //
        *complete = 0;
        printf("packet error\r\n");
        return;
    }
    memcpy(&ucrc, data+lenth-6, sizeof(uint16_t));
    ucrc = SWAP_ENDIAN_16(ucrc);
    //if(ucrc != RS485_CRC(data+4,lenth-10)) {
    //		*complete = 0;
    //    return;
    //}

    if (shortpack->eSourInstrType == 0x00) {
        BT_buff->source = 0;
    } else if (shortpack->eSourInstrType == 0x01) {
        BT_buff->source = 1;
    } //if eSourInstrType and 0 is equal ,data from bt ,else data from wifi
    printf("RS485_buff->source:%d\r\n", BT_buff->source);
    dispatch_command(shortpack->uCommand, data, lenth, sou);
    *complete = 0;
}




// 假设你有定义以下的调试宏
#define DEBUG_DUMP 1

uint32_t fill_realdata_buff(uint8_t **ppdata)
{
    if (ppdata == NULL) {
        printf("错误：ppdata 为空指针\n");
        return 0;
    }

    uint32_t data_size = sizeof(DateTime_t) + sizeof(environment_t) - sizeof(g_environment.currentVal) + sizeof(g_actual_length);
    *ppdata = pvPortMalloc(data_size);
    if (*ppdata == NULL) {
        printf("内存分配失败\n");
        return 0;
    }

    uint32_t offset = 0;
    memcpy(*ppdata + offset, &g_DateTime, sizeof(DateTime_t));
    offset += sizeof(DateTime_t);

    memcpy(*ppdata + offset, &g_environment, sizeof(environment_t) - sizeof(g_environment.currentVal));
    offset += sizeof(environment_t) - sizeof(g_environment.currentVal);

    memcpy(*ppdata + offset, g_actual_length, sizeof(g_actual_length));
    offset += sizeof(g_actual_length);

    if (data_size != offset) {
        printf("错误：填充数据大小与计算的大小不匹配 (data_size=%u, offset=%u)\n", data_size, offset);
        vPortFree(*ppdata);
        *ppdata = NULL;
        return 0;
    }

#if DEBUG_DUMP
    // 打印数据长度
    printf("填充数据长度: %u 字节\n", data_size);

    // 打印每字节内容（16进制）
    printf("填充数据内容（16进制）:\n");
    for (uint32_t i = 0; i < data_size; ++i) {
        printf("%02X ", (*ppdata)[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
    if (data_size % 16 != 0) {
        printf("\n");
    }
#endif

    return data_size;
}



int write_history_data(unsigned char *pbuff, unsigned short int lenth)
{
    unsigned short int data_len;
    data_len = lenth;
    if (LFS_ERR_OK != lfs_file_open(&lfs, &file_hs, HISTORY_DATA_PATH, LFS_O_RDWR | LFS_O_CREAT | LFS_O_APPEND)) {
        return -1;
    }

    if (lfs_file_write(&lfs, &file_hs, &data_len, sizeof (uint16_t)) < 0) {
        lfs_file_close(&lfs, &file_hs);
        lfs_remove(&lfs, HISTORY_DATA_PATH);
        return -1;
    }

    if (lfs_file_write(&lfs, &file_hs, pbuff, lenth) < 0) {
        lfs_file_close(&lfs, &file_hs);
        lfs_remove(&lfs, HISTORY_DATA_PATH);
        return -1;
    }
    lfs_file_sync(&lfs, &file_hs);
    lfs_file_close(&lfs, &file_hs);
    return 0;
}

void ProcessSaveHsCallback(void *user_data)
{
    uint32_t lenth;
    uint8_t *pdata = NULL;  // 定义实际的数据指针
    uint8_t **ppdata = &pdata;  // ppdata 指向 pdata
    printf("保存历史数据\n");
    lenth = fill_realdata_buff(ppdata);
    if (lenth > 0 && *ppdata != NULL) {
        write_history_data(*ppdata, lenth);  // 写入历史数据存
    } else {
        printf("历史数据填充失败或无效数据长度\n");
    }
    if (*ppdata != NULL) {
        vPortFree(*ppdata);  // 释放分配的内cun
    }
}

int read_history_lenth(void)
{
    unsigned short int data_len;
    static unsigned int lfs_seek=0;
    if (LFS_ERR_OK != lfs_file_open(&lfs, &file_hs, HISTORY_DATA_PATH, LFS_O_RDONLY)) {
        lfs_seek=0;
        return -1;
    }
    printf("历史数据长度偏移lfs_seek:%d\n", lfs_seek);
    lfs_file_seek(&lfs, &file_hs, lfs_seek, LFS_SEEK_SET);
    int res = lfs_file_read(&lfs, &file_hs, &data_len, sizeof(data_len));
    if (res != sizeof(data_len)) {
        lfs_seek=0;
        lfs_file_close(&lfs, &file_hs);  // 确保文件关闭
        lfs_remove(&lfs, HISTORY_DATA_PATH);
        return res;
    }
    lfs_seek = lfs_seek+2+data_len;
    lfs_file_close(&lfs, &file_hs);
    return (int)data_len;
}

int read_history_data(unsigned char *pbuff, unsigned short int length, uint8_t flag)
{

    unsigned short int data_len;
    static unsigned int lfs_seek=0;

    data_len = length;

    if (flag) {
        lfs_seek=0;
        return 0;
    }

    if (LFS_ERR_OK != lfs_file_open(&lfs, &file_hs, HISTORY_DATA_PATH, LFS_O_RDONLY)) {
        lfs_seek=0;
        return -1;
    }
    lfs_seek = lfs_seek + sizeof (unsigned short int);
    printf("历史数据读取偏移lfs_seek:%d\n", lfs_seek);
    lfs_file_seek(&lfs, &file_hs, lfs_seek, LFS_SEEK_SET);
    int res = lfs_file_read(&lfs, &file_hs, pbuff, data_len);
    if (res < 0) {

        lfs_file_close(&lfs, &file_hs);  // 确保文件关闭
        lfs_remove(&lfs, HISTORY_DATA_PATH);
        lfs_seek = 0;
        return res;
    }
    if (res != data_len) {
        lfs_file_close(&lfs, &file_hs);
        lfs_remove(&lfs, HISTORY_DATA_PATH);
        lfs_seek = 0;
        return -2;  // 自定义错误码，表示读取数据不足
    }
    lfs_seek = lfs_seek + data_len;

    lfs_file_close(&lfs, &file_hs);

    return res;

}

void bt_send_hs_process(void)
{
    int res;
    unsigned short int data_len, send_len;
    uint8_t *pbuff;
    uint8_t *data;
    if (history_flag.bt_hs_flag) {

        res = read_history_lenth();
        if (res <= 0) {
            history_flag.bt_hs_flag = 0;
            pbuff = (uint8_t *)pvPortMalloc(24);
            send_len = fill_pack_to_buff(0, 0, CMD_READ_HISTORY_DATA, pbuff, NULL, 0);
            sendBTdata(pbuff, send_len);
            vPortFree(pbuff);
            read_history_data(NULL, 0, 1);
            return;
        }
        data_len = (unsigned short int)res;
        printf("读取到的历史数据的字节:%d\n", data_len);
        data = (uint8_t *)pvPortMalloc(data_len);
        res = read_history_data(data, data_len, 0);
        if (res < 0) {
            history_flag.bt_hs_flag = 0;
            vPortFree(data);
            pbuff = (uint8_t *)pvPortMalloc(24);
            send_len = fill_pack_to_buff(0, 0, CMD_READ_HISTORY_DATA, pbuff, NULL, 0);
            sendBTdata(pbuff, send_len);
            vPortFree(pbuff);
            return;
        }
        pbuff = (uint8_t *)pvPortMalloc(data_len + 24);
        send_len = fill_pack_to_buff(0, 0, CMD_READ_HISTORY_DATA, pbuff, data, data_len);

        sendBTdata(pbuff, send_len);
        vPortFree(data);
        vPortFree(pbuff);
    }

}

void rs485_send_hs_process(void)
{
    int res;
    unsigned short int data_len, send_len;
    uint8_t *pbuff;
    uint8_t *data;
    if (history_flag.rs485_hs_flag) {

        res = read_history_lenth();
        if (res <= 0) {
            history_flag.rs485_hs_flag = 0;
            pbuff = (uint8_t *)pvPortMalloc(24);
            send_len = fill_pack_to_buff(0, 0, CMD_READ_HISTORY_DATA, pbuff, NULL, 0);
            rs485_sendbuff(pbuff, send_len);
            vPortFree(pbuff);
            read_history_data(NULL, 0,1);
            return;
        }
        data_len = (unsigned short int)res;
        data = (uint8_t *)pvPortMalloc(data_len);
        res = read_history_data(data, data_len,0);
        if (res < 0) {
            history_flag.rs485_hs_flag = 0;
            pbuff = (uint8_t *)pvPortMalloc(24);
            send_len = fill_pack_to_buff(0, 0, CMD_READ_HISTORY_DATA, pbuff, NULL, 0);
            rs485_sendbuff(pbuff, send_len);
            vPortFree(pbuff);
            vPortFree(data);
            return;
        }
        pbuff = (uint8_t *)pvPortMalloc(data_len + 24);
        send_len = fill_pack_to_buff(0, 0, CMD_READ_HISTORY_DATA, pbuff, data, data_len);

        rs485_sendbuff(pbuff, send_len);
        vPortFree(data);
        vPortFree(pbuff);
    }

}

void set_BTname(void) {

    char name_str[10];
    sprintf(name_str, "%d", g_devInfo.dev_num);
    AT_Name(name_str);

}

int main(void)
{
#ifdef __FIRMWARE_VERSION_DEFINE
    uint32_t fw_ver = 0;
#endif
    /* configure systick */
    systick_config();
    user_gpio_init();
    
    /* initialize the LEDs, USART and key */

#ifdef __FIRMWARE_VERSION_DEFINE
    fw_ver = gd32f10x_firmware_version_get();
    /* print firmware version */
    printf("\r\nGD32F10x series firmware version: V%d.%d.%d", (uint8_t)(fw_ver >> 24), (uint8_t)(fw_ver >> 16), (uint8_t)(fw_ver >> 8));
#endif /* __FIRMWARE_VERSION_DEFINE */

    /* print out the clock frequency of system, AHB, APB1 and APB2 */
    printf("\r\nCK_SYS is %d", rcu_clock_freq_get(CK_SYS));
    printf("\r\nCK_AHB is %d", rcu_clock_freq_get(CK_AHB));
    printf("\r\nCK_APB1 is %d", rcu_clock_freq_get(CK_APB1));
    printf("\r\nCK_APB2 is %d", rcu_clock_freq_get(CK_APB2));

    adc_config();
    adc_gpio_config();
    adc_dma_config();

    spi0_gpio_config();
    spi0_config();


    spi2_gpio_config();
    spi2_config();


    LCD_Init();

    if (sfud_init() == SFUD_SUCCESS) {

    }
    lfs_spi_flash_init(&cfg);
    /* USER CODE END 2 */
    int err = lfs_mount(&lfs, &cfg);
    if (err) {
        printf("lfs_mount failed!\n");
        lfs_format(&lfs, &cfg);
        lfs_mount(&lfs, &cfg);
    }


    read_dev_info();
    read_channel_info();

    can_gpio_config();
    can_nvic_config();
    can_config(g_devInfo.can_baudrate);
		
		usart_nvic_config();
    usart_config(g_devInfo.rs485_baudrate, 115200, 115200);
    dma_config();

    /* configure the GPIO ports */
    i2c_gpio_config();
    /* configure the I2C0 interfaces */
    i2c_config();

    gud600_FixDisplay();
    rtc_init();
    INA226_ReadRegister(0xff);
    SernerConfig();


    init_commands();

    if(g_devInfo.save_interval != 0) {

        BSP_Timer_Init(&ProcessSaveHsTimer,
                       ProcessSaveHsCallback,
                       g_devInfo.save_interval*60*1000,
                       0,
                       TIMER_TYPE_SOFTWARE);
        BSP_Timer_Start(&ProcessSaveHsTimer);

    }

    BSP_Timer_Init(&CollectTimer,
                   gud600_GetSample,
                   3*1000,
                   0,
                   TIMER_TYPE_SOFTWARE);
    BSP_Timer_Start(&CollectTimer);



    //AT_ExitTpmod();
    //delay_1ms(500);
    //set_BTname();
    //delay_1ms(500);



    AT_ExitTpmod();
		delay_1ms(500);
		AT_Restore();
    delay_1ms(500);
    AT_Role(1);
		delay_1ms(500);
		set_BTname();
    delay_1ms(500);
    AT_SetTpmod();
		delay_1ms(500);
		AT_Reboot() ;
 


    while(1) {
        //gud600_GetSample();
        gud600_VarDisplay();
        AT_TickHandler();
        printf_time();
        BSP_Timer_SoftTimerTask();
        process_data();
        bt_send_hs_process();
        rs485_send_hs_process();
        //delay_1ms(1000);
    }

}

#ifdef GD_ECLIPSE_GCC
/* retarget the C library printf function to the USART, in Eclipse GCC environment */
int __io_putchar(int ch)
{
    usart_data_transmit(EVAL_COM0, (uint8_t) ch );
    while(RESET == usart_flag_get(EVAL_COM0, USART_FLAG_TBE));
    return ch;
}
#else
/* retarget the C library printf function to the USART */
//int fputc(int ch, FILE *f)
//{
//    usart_data_transmit(EVAL_COM0, (uint8_t)ch);
//    while(RESET == usart_flag_get(EVAL_COM0, USART_FLAG_TBE));
//    return ch;
//}
#endif /* GD_ECLIPSE_GCC */
