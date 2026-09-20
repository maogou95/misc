/*!
    \file    main.h
    \brief   the header file of main

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

#ifndef MAIN_H
#define MAIN_H


#include "gpio.h"
#include "usart.h"
#include "adc.h"
#include "lcd.h"
#include "lfs.h"
#include "sfud.h"
#include "bsp_timer.h"
#include "bw236.h"
#define DEVICETYPE 0x68

#define SOURCE_RS485   0
#define SOURCE_BLUETOOTH 1

#define DEV_INFO_PATH    	"dev_info"
#define SENSOR_INFO_PATH    "sensor_info"
#define HISTORY_DATA_PATH    "history_data"


typedef enum {
	CAN,
	RS485,
}can_or_rs485_e;

typedef enum {
    BT_EXIT_IDLE = 0,
    BT_EXIT_WAIT_A,      
    BT_EXIT_SENT_A,      
    BT_EXIT_SUCCESS,
    BT_EXIT_FAILED
} ExitTransparentState_t;

extern volatile ExitTransparentState_t ExitTransparentState;




#define MAX_COMMANDS 32
#define CMD_BUF_LEN  64

typedef enum {
    CMD_SET_DEVICE_INFO     = 0x01,
    CMD_GET_DEVICE_INFO     = 0x02,
    CMD_CALIBRATE_CHANNEL   = 0x03,
    CMD_RESET_CHANNEL       = 0x04,
    CMD_SYNC_TIME           = 0x05,
    CMD_SET_TIME            = 0x06,
    CMD_READ_REALTIME_DATA  = 0x07,
    CMD_READ_HISTORY_DATA   = 0x08,
    // 预留命令
		CMD_EXIT_TRANSPARENT_MODE = 0x09,
    CMD_GET_KAB       = 0x0A,
    CMD_SET_KAB          = 0x0B,
    CMD_RESERVED_12          = 0x0C,
    CMD_RESERVED_13          = 0x0D,
    CMD_RESERVED_14          = 0x0E,
    CMD_RESERVED_15          = 0x0F,
    CMD_RESERVED_16          = 0x10,
    CMD_RESERVED_17          = 0x11,
    CMD_RESERVED_18          = 0x12,
    CMD_RESERVED_19          = 0x13,
    CMD_RESERVED_20          = 0x14
} CommandCode_t;

typedef enum
{
    ACK = 0x33,
    NACK = 0x77,
    Continue = 0x88,
} ACK_Type;
#pragma pack(1) 

typedef struct {
	
	uint16_t dev_num;
	uint32_t rs485_baudrate;
	uint32_t can_baudrate;
	uint32_t can_type_num;//使用can扩展id，前面16位代表id，后面16位代表编号
	uint16_t save_interval;
	can_or_rs485_e can_or_rs485_switch;
	uint8_t software_version[3]; // 软件版本号，格式如 {1, 2, 3} 表示 1.2.3
  uint8_t hardware_version[3]; // 硬件版本号，格式如 {1, 2, 3} 表示 1.2.3
	
}devInfo_t;

typedef struct {
	
	uint16_t zero_point;
	float KA;
	float KB;

}channelInfo_t;

typedef struct {
    float voltage_upper_threshold; // 电压上阈值 (Float)，单位可以根据应用情况设定（比如伏特）
    float voltage_lower_threshold; // 电压下阈值 (Float)
    float temperature_upper_threshold; // 温度上阈值 (Float)，单位可以设为摄氏度
    float temperature_lower_threshold; // 温度下阈值 (Float)
    float humidity_upper_threshold;    // 湿度上阈值 (Float)，单位可以设为百分比
    float humidity_lower_threshold;    // 湿度下阈值 (Float)
} Thresholds_t;

typedef struct {
    uint8_t rs485_hs_flag;
    uint8_t bt_hs_flag;
} history_flag_t;

typedef struct
{
    uint8_t  Head[4];								// 帧头          0
    uint8_t  eDestInstrType;							// 目标分站类型  4
    uint32_t uDestInstrNum;							// 目标分站编号  5
    uint8_t  eSourInstrType;							// 源分站类型    9
    uint32_t uSourInstrNum;							// 源分站编号   10
    uint8_t  uCommand;								// 命令ID       14
    uint8_t  uACK;									// ACK码        15
    uint16_t uDataLen;								// 数据长度     16
} shortbasepack_t;



typedef struct
{
    uint8_t  Head[4];								// 帧头          0
    uint8_t  eDestInstrType;							// 目标分站类型  4
    uint32_t uDestInstrNum;							// 目标分站编号  5
    uint8_t  eSourInstrType;							// 源分站类型    9
    uint32_t uSourInstrNum;							// 源分站编号   10
    uint8_t  uCommand;								// 命令ID       14
    uint8_t  uACK;									// ACK码        15
    uint16_t uDataLen;								// 数据长度     16
    uint16_t uCrc;									// CRC校验
    uint8_t  Tail[4];
} basepack_t;
#pragma pack() 

// 命令处理函数类型：传入数据缓冲区及其长度
typedef void (*CommandHandler_t)(const uint8_t* data, uint16_t len, uint8_t source);

// 命令注册项
typedef struct {
    uint8_t cmd;
    CommandHandler_t handler;
} CommandRegistry_t;

/* led spark function */
void led_spark(void);

#endif /* MAIN_H */
