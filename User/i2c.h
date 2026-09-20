/*!
    \file    i2c.h
    \brief   the header file of I2C

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

#ifndef I2C_H
#define I2C_H

#include "gd32f10x.h"

/* USER CODE BEGIN 0 */
#define Ina226DeviceAddr			0x41
#define Ina226DeviceDie_ID			0x2260

#define HDC_DeviceAddr 				0x40
#define HDC_DeviceID 				0x1050

#define SWAP_ENDIAN_16(x) (((x) >> 8) | ((x) << 8))

#define Configuration_Register_Init 0x41ff // 1次平均 8.244ms 连续监测

/****************************ina226*******************************/

#define Configuration_Register 			0x00
#define Shunt_Voltage_Register 			0x01
#define Bus_Voltage_Register				0x02
#define Power_Register							0x03
#define Current_Register 						0x04
#define	Calibration_Register				0x05
#define	Mask_or_Enable_Register			0x06
#define	Alert_Limit_Register				0x07
#define Manufacturer_ID_Register 		0xFE
#define	Die_ID_Register							0xFF


/***************Configuration_Register********************/

#define ResetBit																				(0x01 << 15) /*15bit*/
#define Default_bit																			(0x40 << 14) /*12-14bit*/
									
#define AveragingMode_1_bit															(0x00 << 9)/*9-11bit*/
#define AveragingMode_4_bit															(0x01 << 9)
#define AveragingMode_16_bit														(0x02 << 9)
#define AveragingMode_64_bit														(0x03 << 9)
#define AveragingMode_128_bit														(0x04 << 9)
#define AveragingMode_256_bit														(0x05 << 9)
#define AveragingMode_512_bit														(0x06 << 9)
#define AveragingMode_1024_bit													(0x07 << 9)



#define BusVoltageConversionTime_140us_bit      				(0x00 << 9)/*6-8bit*/
#define BusVoltageConversionTime_204us_bit      				(0x01 << 9)
#define BusVoltageConversionTime_332us_bit      				(0x02 << 9)
#define BusVoltageConversionTime_558us_bit      				(0x03 << 9)
#define BusVoltageConversionTime_1100us_bit     				(0x04 << 9)
#define BusVoltageConversionTime_2116us_bit     				(0x05 << 9)
#define BusVoltageConversionTime_4156us_bit     				(0x06 << 9)
#define BusVoltageConversionTime_8244us_bit     				(0x07 << 9)

#define ShuntVoltageConversionTime_140us_bit      			(0x00 << 9)/*3-5bit*/
#define ShuntVoltageConversionTime_204us_bit      			(0x01 << 9)
#define ShuntVoltageConversionTime_332us_bit      			(0x02 << 9)
#define ShuntVoltageConversionTime_558us_bit      			(0x03 << 9)
#define ShuntVoltageConversionTime_1100us_bit     			(0x04 << 9)
#define ShuntVoltageConversionTime_2116us_bit     			(0x05 << 9)
#define ShuntVoltageConversionTime_4156us_bit     			(0x06 << 9)
#define ShuntVoltageConversionTime_8244us_bit     			(0x07 << 9)

#define OperatingModePowerDown1_bit													(0x00)/*0-2bit*/
#define OperatingModeShuntVoltageTriggered_bit							(0x01)
#define OperatingModeBusVoltageTriggered_bit								(0x02)
#define OperatingModeShuntandBusTriggered_bit               (0x03)
#define OperatingModePowerDown2_bit                         (0x04)
#define OperatingModeShuntVoltageContinuous_bit             (0x05)
#define OperatingModeBusVoltageContinuous_bit               (0x06)
#define OperatingModeShuntandBusContinuous_bit              (0x07)

//定义配置数据
#define 	INA226_VAL_LSB	2.5f	//分流电压 LSB 2.5uV
#define     Voltage_LSB		1.25f   //总线电压 LSB 1.25mV
#define     CURRENT_LSB 	1.0f 	//电流LSB 1mA
#define     POWER_LSB       25*CURRENT_LSB
#define     CAL             2560     //0.00512*1000/(Current_LSB*R_SHUNT) = 512  //电流偏大改小


#define I2C_TIME_OUT           (uint16_t)(5000)

#define I2C_OK                 1
#define I2C_FAIL               0
#define I2C_END                1


#pragma pack(1)  // 设置为1字节对齐
typedef struct
{
    float voltageVal;			//mV
    float Shunt_voltage;		//uV
    float Shunt_Current;		//mA
    float Power_Val;			//功率
    float Power;				//功率mW
} INA226;



#define I2CREAD 0x1
#define I2CWRITE 0x0

/****************************end**********************************/


/****************************hdc1080*******************************/

#define         Configuration_register_add              0x02
#define         Temperature_register_add                0x00
#define         Humidity_register_add                   0x01
#define         DeviceID_register_add  									0xff
typedef enum
{
  Temperature_Resolution_14_bit = 0,
  Temperature_Resolution_11_bit = 1
}Temp_Reso;

typedef enum
{
  Humidity_Resolution_14_bit = 0,
  Humidity_Resolution_11_bit = 1,
  Humidity_Resolution_8_bit =2
}Humi_Reso;

typedef struct
{
	float Temp;
	float Humi;
}HDC1080;


typedef struct {

	float voltageVal;
	float Temp;
	float Humi;
	float currentVal;

}environment_t;


typedef enum {
    I2C_START = 0,
    I2C_SEND_ADDRESS,
    I2C_CLEAR_ADDRESS_FLAG,
    I2C_TRANSMIT_DATA,
    I2C_STOP,
} i2c_process_enum;

/****************************end**********************************/

#pragma pack()  // 恢复默认对齐

union registers 
{
	unsigned char a[2];
	unsigned short int b;
};

/* USER CODE END Private defines */
extern environment_t g_environment;


/* configure the GPIO ports */
void i2c_gpio_config(void);
/* configure the I2C0 interfaces */
void i2c_config(void);

void SernerConfig(void);
void ReadSernerData(void);

uint16_t INA226_ReadRegister(uint8_t reg_addr);



#endif  /* I2C_H */
