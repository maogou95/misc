#ifndef BW236_H_
#define BW236_H_
//==========================================
//#include "_TYPE.h"
//==========================================
#include "main.h"
#include "usart.h"
typedef enum {
    AT_STATE_IDLE,
    AT_STATE_WAITING,
    AT_STATE_OK,
    AT_STATE_ERROR,
    AT_STATE_TIMEOUT
} AT_State;



#define AT_RINGBUF_SIZE 1024  // 根据你的应用场景调整大小

typedef struct {
    char buffer[AT_RINGBUF_SIZE];
    uint16_t head;
    uint16_t tail;
		uint16_t count;  // 新增：当前有效数据长度
} AT_RingBuffer;

typedef struct {
    char data[256];
    uint16_t length;
} AT_ReplyFrame;


#define AT_REPLY_FRAME_MAX 10
typedef void (*AT_Callback)(AT_State state, AT_ReplyFrame *frames, uint8_t count);
typedef struct {
    char tx_cmd[512];                      // 发送命令缓存
    uint32_t tick_start;
    uint32_t timeout_ms;

    AT_State state;
    AT_Callback callback;
    uint8_t active;

    AT_ReplyFrame reply_frames[AT_REPLY_FRAME_MAX];  // 存放多帧回复
    uint8_t frame_count;

    AT_RingBuffer rx_ring;                // 接收环形缓存
} AT_CommandContext;



// 自定义 AT 命令枚举
typedef enum {
    CMD_UNKNOWN = 0,
		CMD_WTPMODE,
    CMD_TPMODE,
    CMD_SCAN,
    CMD_REBOOT,
    CMD_RESTORE,
    CMD_STAT,
    CMD_DSCA,
    CMD_NAME,
    CMD_LENAME,
    CMD_GATTSEND,
    CMD_MODE,
    CMD_ROLE,
    CMD_RAP,
		CMD_WRAP,
    CMD_LIP,
    CMD_DHCP,
    CMD_SIP,
		CMD_WSIP,
    CMD_GW,
		CMD_WGW,
    CMD_MASK,
		CMD_WMASK,
    CMD_DNS,
		CMD_WDNS,
    CMD_APAC,
		CMD_WAPAC,
    CMD_RSSI,
    CMD_SOCK,
    CMD_WLANC,
    CMD_WFSEND,
    CMD_CLOSE
} AtCommandID;


#define TPMODE "AT+TPMODE" //透传模式 0关闭 1开启 查询或者设置
#define SCAN "AT+SCAN" //wifi扫描周围设备 参数是5
#define REBOOT "AT+REBOOT" //软件复位
#define RESTORE "AT+RESTORE" //恢复出厂设置
#define STAT "AT+STAT" //查询模块连接状态
#define DSCA "AT+DSCA" //断开 Wi-Fi 或蓝牙连接1: 断开模块与热点的连接 2: 断开模块与蓝牙的连接
#define NAME "AT+LENAME" //BLE 蓝牙名称 (1~25 Bytes ASCII),0: 关闭后缀 1: 开启后缀“-XXXX”(MAC 地址后 4Byte)
#define GATTSEND "AT+GATTSEND" //从模式发送 BLE 数据 需要发送的数据长度+需要发送的数据内容   数据长度应小于 1000
#define MODE "AT+MODE" //查询/设置蓝牙模式 0: 蓝牙从模式 1: 蓝牙主模式

#define ROLE "AT+ROLE" //1:STA 模式2:AP 模式3:STA+AP 共存模式
#define RAP "AT+RAP" //查询已设置热点/连接热点
#define LIP "AT+LIP" //查询模块当前 IP 地址
#define DHCP "AT+DHCP" //0: 使用静态 IP 1: 使用动态 IP(defaul)
#define SIP "AT+SIP"//查询/设置静态 IP
#define GW "AT+GW"  //查询/设置网关
#define MASK "AT+MASK" //查询/设置子网掩码
#define DNS "AT+DNS"//查询/设置 DNS 地址
#define APAC "AT+APAC"//查询/设置上电自动连接热点
#define RSSI "AT+RSSI" //查询与热点之间的信号强度

#define SOCK "AT+SOCK" //查询和设置 SOCKET 模块上电后默认开启 TCP SERVER, 默认端口 9100
#define WLANC "AT+WLANC" //启动 SOCKET/MQTT 需要在 SOCKET 连接或者 MQTT 连接参数都正确设置后再发此条指令
#define WFSEND "AT+WFSEND" //发送 SOCKET 数据给远端设备
#define CLOSE "AT+CLOSE" //断开 TCP Client 与服务器的连接
uint8_t AT_WTpmod(void);
uint8_t AT_Reboot(void) ;
uint8_t AT_Restore(void);
extern AT_CommandContext at_cmd;
void AT_ExitTpmod(void);
uint8_t AT_Name(char *param);
void sendBTdata(uint8_t *buff, uint16_t length);
void AT_UART_IdleHandler(AT_CommandContext *ctx, uint8_t *dma_buf, uint16_t dma_buf_size) ;
void AT_Scan(void);
uint8_t AT_SetTpmod(void) ;
void AT_TickHandler(void) ;
uint8_t AT_cmd_Dw(uint8_t *buff, uint16_t length, uint8_t source) ;
uint8_t AT_Role(uint8_t param);
void AT_ExitTpmod(void);
void bytes_to_hexstr(const uint8_t *bytes, int length, char *hexstr);
#endif 

