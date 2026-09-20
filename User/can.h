#ifndef CAN_H
#define CAN_H
#include "gd32f10x.h"
#include <stdio.h>
#include "systick.h"
//
// === 单位类型定义 ===
//
typedef enum
{
    UNIT_NONE       = 0x00,   // 无单位
    UNIT_CELSIUS    = 0x01,   // 摄氏度 ℃
    UNIT_PERCENT    = 0x02,   // 百分比 %
    UNIT_PA         = 0x03,   // 帕 Pa
    UNIT_KPA        = 0x04,   // 千帕 kPa
    UNIT_MPA        = 0x05,   // 兆帕 MPa
    UNIT_VOLT       = 0x06,   // 伏特 V
    UNIT_MM         = 0x07,   // 毫米 mm
    UNIT_CM         = 0x08,   // 厘米 cm
    UNIT_M          = 0x09,   // 米 m
    UNIT_MA         = 0x0A,   // 毫安 mA
    UNIT_A          = 0x0B,   // 安 A
    UNIT_PPM        = 0x0C,   // ppm
    UNIT_DEGREE     = 0x0D,   // 角度 °
    UNIT_WATT       = 0x0E,   // 功率 W
    UNIT_LPM        = 0x0F,   // 流量 L/min
    UNIT_RESERVED   = 0x10    // 保留
} CAN_UnitType_t;

//
// === 传感器类型定义 ===
//
typedef enum
{
    SENSOR_TYPE_TEMP          = 0x01,   // 温度传感器（℃）
    SENSOR_TYPE_HUMI          = 0x02,   // 湿度传感器（%）
    SENSOR_TYPE_PRESSURE      = 0x03,   // 压力传感器（kPa / MPa）
    SENSOR_TYPE_VOLTAGE       = 0x04,   // 电压传感器（V）
    SENSOR_TYPE_CURRENT       = 0x05,   // 电流传感器（A）
    SENSOR_TYPE_WATER_LEVEL   = 0x06,   // 水位传感器（m / cm / mm）
    SENSOR_TYPE_DISPLACEMENT4 = 0x07,   // 四通道离层位移传感器（mm）
    SENSOR_TYPE_GAS           = 0x08,   // 气体浓度传感器（ppm）
    SENSOR_TYPE_VIBRATION     = 0x09,   // 振动传感器（mm/s）
    SENSOR_TYPE_TILT          = 0x0A,   // 倾角传感器（°）
    SENSOR_TYPE_STRESS        = 0x0B,   // 应力/应变传感器（με）
    SENSOR_TYPE_FLOW          = 0x0C,   // 流量传感器（L/min）
    SENSOR_TYPE_POWER         = 0x0D,   // 功率传感器（W）
    SENSOR_TYPE_RESERVED      = 0x0E    // 保留
} CAN_SensorType_t;


#define 		MSG_EMERGENCY 	0x00
#define    	MSG_CFG_REPLY   0x01   
#define    	MSG_CFG_INFO    0x02   
#define    	MSG_REALTIME    0x03   
#define    	MSG_RESERVED    0x04  
#define    	MSG_HEARTBEAT   0x07  

#pragma pack(push, 1)
typedef struct {
    uint8_t  channel_type;    
    uint8_t  channel_no;      
    float    value;           
    uint8_t  unit_index;      
    uint8_t  alarm_flag;     
} CANRealtimeData_t;

typedef struct {
    uint8_t reserved[8];
} CANHeartbeat_t;

#pragma pack(pop)


#define CAN_ID(msg_type, sensor_id)   (((msg_type & 0x7) << 8) | (sensor_id & 0xFF))

void CAN_BuildRealtimeFrame(uint8_t *payload, uint8_t type, uint8_t ch, float value, uint8_t unit, uint8_t alarm);
void CAN_BuildHeartbeatFrame(uint8_t *payload);
void can_gpio_config(void);
void can_nvic_config(void);
void can_config(uint32_t can_baudrate);

ErrStatus can0_send_message(uint32_t id, const uint8_t *data, uint8_t len, uint8_t ext);
#endif /* CAN_H */

