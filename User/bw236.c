#include "bw236.h"
#include <stdarg.h>
#include <stdio.h>
AT_CommandContext at_cmd;
int tcp_id;
uint8_t is_transparent_mode;  // 是否处于透传模式：1 = 是，0 = 否

void AT_Ring_Init(AT_RingBuffer *rb) {
    rb->head = 0;
    rb->tail = 0;
}

uint8_t AT_Ring_Put(AT_RingBuffer *rb, char ch) {
    uint16_t next = (rb->head + 1) % AT_RINGBUF_SIZE;
    if (next == rb->tail) return 0;
    rb->buffer[rb->head] = ch;
    rb->head = next;
    return 1;
}

// 提取一帧数据（以 \r\n 结尾），返回长度
uint16_t AT_Ring_GetFrame(AT_RingBuffer *rb, char *frame, uint16_t max_len) {
    uint16_t len = 0;
    uint8_t at_count = 0;  // 用于统计连续的 0x40
    while (rb->tail != rb->head && len < max_len - 1) {
        char ch = rb->buffer[rb->tail];
        frame[len++] = ch;
        rb->tail = (rb->tail + 1) % AT_RINGBUF_SIZE;

        if ((uint8_t)ch == 0x40) {
            at_count++;
        } else {
            at_count = 0;
        }

        // 检查是否是 \r\n 结尾
        if (len >= 2 && frame[len - 2] == '\r' && frame[len - 1] == '\n') {
            frame[len] = '\0';
            return len;
        }

        // 检查是否是 4 个连续的 0x40
        if (at_count == 4) {
            return len;
        }
    }

    return 0;  // 没有找到完整帧
}



void AT_SendCommandEx(const char *cmd, const char *args_fmt, uint32_t timeout_ms, AT_Callback cb, ...) {
    if (at_cmd.active) return;

    va_list args;
    va_start(args, cb);

    // 构造 AT 指令
    if (args_fmt && strlen(args_fmt) > 0) {
        snprintf(at_cmd.tx_cmd, sizeof(at_cmd.tx_cmd), "%s=", cmd);
        size_t len = strlen(at_cmd.tx_cmd);
        vsnprintf(&at_cmd.tx_cmd[len], sizeof(at_cmd.tx_cmd) - len, args_fmt, args);
    } else {
        snprintf(at_cmd.tx_cmd, sizeof(at_cmd.tx_cmd), "%s", cmd);
    }

    va_end(args);

    // 添加回车换行
    strncat(at_cmd.tx_cmd, "\r\n", sizeof(at_cmd.tx_cmd) - strlen(at_cmd.tx_cmd) - 1);

    // 初始化命令状态
    at_cmd.tick_start = HAL_GetTick();
    at_cmd.state = AT_STATE_WAITING;
    at_cmd.callback = cb;

    if (timeout_ms != 0)
        at_cmd.active = 1;
    else
        at_cmd.active = 0;
    // 发送数据
    bt_sendbuff((uint8_t *)at_cmd.tx_cmd, strlen(at_cmd.tx_cmd));

    if (timeout_ms == 0) {
        // === 阻塞等待模式 ===
        uint32_t wait_start = HAL_GetTick();
        while ((HAL_GetTick() - wait_start) < 500) {  // 最多等待 500ms
            if (at_cmd.state == AT_STATE_OK || at_cmd.state == AT_STATE_ERROR) {
                if (cb) cb(at_cmd.state, at_cmd.reply_frames, at_cmd.frame_count);
                at_cmd.active = 0;
                return;
            }
        }

        // 超时
        at_cmd.state = AT_STATE_TIMEOUT;
        if (cb) cb(at_cmd.state, at_cmd.reply_frames, at_cmd.frame_count);
        at_cmd.active = 0;
    } else {
        // === 异步等待模式 ===
        at_cmd.timeout_ms = timeout_ms;
    }
}



void AT_SendU8Buff(const char *cmd, const char *args_fmt, uint8_t *buff, uint32_t length, ...) {
    uint16_t offset;
    // 构造 AT 指令

    va_list args;
    va_start(args, length);

    // 构造 AT 指令
    if (args_fmt && strlen(args_fmt) > 0) {
        snprintf(at_cmd.tx_cmd, sizeof(at_cmd.tx_cmd), "%s=", cmd);
        size_t len = strlen(at_cmd.tx_cmd);
        vsnprintf(&at_cmd.tx_cmd[len], sizeof(at_cmd.tx_cmd) - len, args_fmt, args);
    } else {
        return;
    }

    va_end(args);

    offset = strlen(at_cmd.tx_cmd);
    if (offset + length + 2 > sizeof(at_cmd.tx_cmd)) {
        // 超出缓冲区，返回错误
        return;
    }


    memcpy(&at_cmd.tx_cmd[offset], buff, length);
    offset = offset + length;
    //添加回车换行
    //memcpy(&at_cmd.tx_cmd[offset], "\r\n", strlen("\r\n"));
    //offset = offset + strlen("\r\n");

    // 初始化命令状态
//   at_cmd.tick_start = HAL_GetTick();
//   at_cmd.state = AT_STATE_WAITING;
// //  at_cmd.callback = NULL;
//   at_cmd.active = 1;


    // 关键初始化
    //at_cmd.frame_count = 0;
    //AT_Ring_Init(&at_cmd.rx_ring);
    //memset(at_cmd.reply_frames, 0, sizeof(at_cmd.reply_frames));
    // 发送数据
    bt_sendbuff((uint8_t *)at_cmd.tx_cmd, offset);


    // === 阻塞等待模式 ===
    // uint32_t wait_start = HAL_GetTick();
    // while ((HAL_GetTick() - wait_start) < 500) {  // 最多等待 500ms
    //     if (at_cmd.state == AT_STATE_OK || at_cmd.state == AT_STATE_ERROR) {
    //         at_cmd.active = 0;
    //         return;
    //     }
    // }

    // 超时
//  at_cmd.state = AT_STATE_OK;
//  at_cmd.active = 0;
    printf("send U8 data succese!\n");

}

void sendBTdata(uint8_t *buff, uint16_t length) {

    printf("is_transparent_mode:%d\r\n",is_transparent_mode);
    if (is_transparent_mode) {
        bt_sendbuff(buff, length);
    } else {
        UART_Recv_t *BT_buff = get_UART_Recv_t(usart1);
				printf("RS485_buff->source:%d\r\n", BT_buff->source);
        if (BT_buff->source) {
            AT_SendU8Buff(WFSEND, "0,%d,",buff, length, length);
            if (at_cmd.state != AT_STATE_OK) {
                printf("send BT U8 data timeout!\n");
            }
        } else {

            AT_SendU8Buff(GATTSEND, "%d,",buff, length, length);
            if (at_cmd.state != AT_STATE_OK) {
                printf("send BT U8 data timeout!\n");
            }
        }



    }

    // AT_SendU8Buff(GATTSEND, "%d,",buff, length, length);
    // if (at_cmd.state != AT_STATE_OK) {
    //     printf("send BT U8 data timeout!\n");
    // }

}
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


int hexstr_to_bytes(const char *hexstr, uint8_t *bytes, int max_len) {
    int len = strlen(hexstr);
    if (len % 2 != 0) return -1;

    int count = 0;
    for (int i = 0; i < len; i += 2) {
        if (count >= max_len) return -2;

        char byte_str[3] = { hexstr[i], hexstr[i + 1], '\0' };
        if (!isxdigit(byte_str[0]) || !isxdigit(byte_str[1])) return -3;

        bytes[count++] = (uint8_t)strtol(byte_str, NULL, 16);
    }
    return count;
}

void bytes_to_hexstr(const uint8_t *bytes, int length, char *hexstr) {
    for (int i = 0; i < length; ++i) {
        sprintf(&hexstr[i * 2], "%02x", bytes[i]);
    }
    hexstr[length * 2] = '\0';
}


void AT_UART_IdleHandler(AT_CommandContext *ctx, uint8_t *dma_buf, uint16_t dma_buf_size) {

    //  if (ctx->active == 0) return;

    for (uint16_t i = 0; i < dma_buf_size; i++) {
        AT_Ring_Put(&ctx->rx_ring, dma_buf[i]);
    }

    char frame_buf[256];
    uint16_t len;

    int buff_len;


    while ((len = AT_Ring_GetFrame(&ctx->rx_ring, frame_buf, sizeof(frame_buf))) > 0) {

        printf("frame_buf:%s\n",frame_buf);

        if (strstr(frame_buf, "GATTDATA") != NULL) {  //待定，到时候具体看数据包格式

            UART_Recv_t *BT_buff = get_UART_Recv_t(usart1);
            const char *equal_pos = strchr(frame_buf, '=');
            if (equal_pos) {
                // 3. 从 '=' 后面开始解析
                if (sscanf(equal_pos + 1, "%d,%512[^\r\n]", &buff_len, BT_buff->buffer) == 2) {
                    printf("Length: %d\n", buff_len);
                    BT_buff->complete = 1;
                    BT_buff->length = buff_len;
                    BT_buff->source = 0;
                    return;
                } else {
                    printf("格式解析失败\n");
                }
            } else {
                printf("未找到 '=' 符号\n");
            }
        } else if (strstr(frame_buf, "WFDATA") != NULL)  {

            UART_Recv_t *BT_buff = get_UART_Recv_t(usart1);
            const char *equal_pos = strchr(frame_buf, '=');
            if (equal_pos) {
                // 3. 从 '=' 后面开始解析
								printf("%s\r\n", equal_pos + 1);
                if (sscanf(equal_pos + 1, "%d,%d,%512[^\r\n]", &tcp_id, &buff_len, BT_buff->buffer) == 3) {
                    printf("Length: %d\n", buff_len);
                    BT_buff->complete = 1;
                    BT_buff->length = buff_len;

                    BT_buff->source = 1;


                    return;
                } else {
                    printf("格式解析失败\n");
                }
            } else {
                printf("未找到 '=' 符号\n");
            }

        } else if ((len >= 8) &&
                   memcmp(frame_buf, "\x25\x25\x25\x25", 4) == 0 &&
                   memcmp(&frame_buf[len - 4], "\x40\x40\x40\x40", 4) == 0) {

            printf("包含 25252525 -> 40404040 数据帧\n");

            UART_Recv_t *BT_buff = get_UART_Recv_t(usart1);

            int i = 0;
            while (i <= len - (4 + 4)) {
                // 查找帧头
                if (memcmp(&frame_buf[i], "\x25\x25\x25\x25", 4) == 0) {
                    // 找到帧头，开始向后查找帧尾
                    for (int j = i + 4; j <= len - 4; j++) {
                        if (memcmp(&frame_buf[j], "\x40\x40\x40\x40", 4) == 0) {
                            int frame_len = j + 4 - i;
                            memcpy(BT_buff->buffer, &frame_buf[i], frame_len);
                            BT_buff->length = frame_len;
                            BT_buff->complete = 1;
                            printf("success:%d\r\n",BT_buff->length);
                            return;  // 成功
                        }
                    }
                }
                i++;
            }
        } else {

            printf("不包含 来源关键字 字符串\n");
        }

        if (ctx->frame_count < AT_REPLY_FRAME_MAX && strlen(frame_buf) > 4 && strstr(frame_buf, "WLANSTAT") == NULL) {
            strncpy(ctx->reply_frames[ctx->frame_count].data, frame_buf, sizeof(ctx->reply_frames[0].data) - 1);
            ctx->reply_frames[ctx->frame_count].length = len;
            ctx->frame_count++;
            printf("frame_count:%d\n",ctx->frame_count);
        }

        if (strstr(frame_buf, "OK") || strstr(frame_buf, "ERROR")) {
            ctx->state = strstr(frame_buf, "OK") ? AT_STATE_OK : AT_STATE_ERROR;

            //if (ctx->callback) {
            //    ctx->callback(ctx->state, ctx->reply_frames, ctx->frame_count);
            //		ctx->frame_count--;
            //}
            //
            //ctx->active = 0;
            //return;
        }
    }
}


void AT_TickHandler(void) {
    if ((at_cmd.active && at_cmd.state == AT_STATE_WAITING) || (at_cmd.active && at_cmd.state == AT_STATE_OK) || (at_cmd.active && at_cmd.state == AT_STATE_ERROR)) {
        if ((HAL_GetTick() - at_cmd.tick_start > at_cmd.timeout_ms) || (at_cmd.state == AT_STATE_ERROR)) {
            printf("AT_STATE_TIMEOUT!\n\r");
            at_cmd.state = AT_STATE_TIMEOUT;
            while (at_cmd.frame_count) {
                if (at_cmd.callback) at_cmd.callback(AT_STATE_TIMEOUT, at_cmd.reply_frames, at_cmd.frame_count);
                else printf("callback NULL\n");
                at_cmd.frame_count--;
                printf("frame_count--:%d\n",at_cmd.frame_count);
            }

            at_cmd.active = 0;
        }
    }
}

// 超时回调实现
void AT_DefaultTimeoutCallback(AT_State state, const char *resp) {
    if (state == AT_STATE_TIMEOUT) {
        printf("[AT] Timeout occurred. No response received within expected time.\n");
    } else if (state == AT_STATE_ERROR) {
        printf("[AT] Error response: %s\n", resp ? resp : "NULL");
    } else if (state == AT_STATE_OK) {
        printf("[AT] Success response: %s\n", resp ? resp : "NULL");
    } else {
        printf("[AT] Unknown state: %d\n", state);
    }
}


uint8_t AT_SetTpmod(void) {

    AT_SendCommandEx(TPMODE, "%d", 0, NULL, 1);
    if (at_cmd.state == AT_STATE_OK) {
        printf("[AT] AT_SetTpmod succese\n");
        is_transparent_mode = 1;
        return 1;
    }
    return 0;
}


uint8_t AT_WTpmod(void) {

    AT_SendCommandEx(TPMODE, NULL, 0, NULL);
    if (at_cmd.state == AT_STATE_OK) {
        return 1;
    }
    return 0;
}

void AT_ExitTpmod(void) {

    AT_SendCommandEx(TPMODE, "%d", 0, NULL, 0);
    if (at_cmd.state == AT_STATE_OK) {
        printf("[AT] AT_SetTpmod succese\n");
        is_transparent_mode = 0;
    }
}

void AT_ResponseCallback(AT_State state, AT_ReplyFrame *frames, uint8_t count);



void extract_after_equal(const char *input, char *output, size_t max_len) {
    const char *equal_pos = strchr(input, '=');
    if (equal_pos && *(equal_pos + 1) != '\0') {
        strncpy(output, equal_pos + 1, max_len - 1);
        output[max_len - 1] = '\0';  // 确保以 \0 结尾
    } else {
        output[0] = '\0'; // 没有等号或等号后面为空
    }
}
void AT_ResponseCallback(AT_State state, AT_ReplyFrame *frames, uint8_t count) {
    char result[128];
    if (state == AT_STATE_OK && count >= 1) {

        printf("Data Frame: %s\n", frames[count-1].data);
        printf("[AT] Unknown state: %d\n", state);
        printf("准备发送回包\n");
        sendBTdata((uint8_t *)frames[count-1].data, strlen(frames[count-1].data));

    } else if (state == AT_STATE_TIMEOUT && count >= 1) {

        // frames[0] 是数据帧
        printf("Data Frame: %s\n", frames[count-1].data);
        printf("[AT] Unknown state: %d\n", state);
        // extract_after_equal(frames[count-1].data, result, sizeof (result));
        printf("准备发送回包\n");
        sendBTdata((uint8_t *)frames[count-1].data, strlen(frames[count-1].data));
        //AT_SendCommandEx(GATTSEND, "%d,%s", 0, NULL, strlen(frames[count-1].data), frames[count-1].data);
    }
}


void AT_Scan(void) {

    AT_SendCommandEx(SCAN, "%d", 9000, AT_ResponseCallback, 5);//扫描时间为5000ms

}


uint8_t AT_Reboot(void) {

    AT_SendCommandEx(REBOOT, NULL, 0, NULL);
    return at_cmd.state == AT_STATE_OK ? 1:0;

}


uint8_t AT_Restore(void) {

    AT_SendCommandEx(RESTORE, NULL, 0, NULL);
    return at_cmd.state == AT_STATE_OK ? 1:0;

}


void AT_Stat(void) {

    AT_SendCommandEx(STAT, NULL, 1000, AT_ResponseCallback);

}


uint8_t AT_Dsca(char *param) {

    AT_SendCommandEx(DSCA, "%s", 0, NULL, param);
    return at_cmd.state == AT_STATE_OK ? 1:0;

}


int is_valid_integer(const char *str)
{
    if (str == NULL || *str == '\0') return 0;

    char *endptr;
    long val = strtol(str, &endptr, 10);

    if (*endptr != '\0') return 0;
    if (val > INT32_MAX || val < INT32_MIN) return 0;

    return 1;
}


uint8_t AT_Name(char *param) {

    if (!is_valid_integer(param)) {

        AT_SendCommandEx(NAME, NULL, 1000, AT_ResponseCallback);
        return 3;

    } else {

        AT_SendCommandEx(NAME, "GUD600-%s,1", 0, NULL, param);

        return at_cmd.state == AT_STATE_OK ? 1:0;
    }

}


uint8_t AT_Role(uint8_t param) {

    if (!param) {

        AT_SendCommandEx(ROLE, NULL, 1000, AT_ResponseCallback);
        return 0;

    } else {

        AT_SendCommandEx(ROLE, "%d", 0, NULL, 1);
        return at_cmd.state == AT_STATE_OK ? 1:0;
    }

}



uint8_t AT_RAP(uint8_t param, char *val) {

    if (!param) {

        AT_SendCommandEx(RAP, NULL, 1000, AT_ResponseCallback);
        return 3;

    } else {

        AT_SendCommandEx(RAP, "%s", 0, NULL, val);




        return at_cmd.state == AT_STATE_OK ? 1:0;
    }

}


void AT_LIP(void) {

    AT_SendCommandEx(LIP, NULL, 1000, AT_ResponseCallback);

}

uint8_t AT_DHCP(char *param) {



    AT_SendCommandEx(DHCP, "%s", 0, NULL, param);
    return at_cmd.state == AT_STATE_OK ? 1:0;


}




uint8_t AT_SIP(uint8_t param, char *ip) {

    if (!param) {
        AT_SendCommandEx(SIP, NULL, 1000, AT_ResponseCallback);
        return 0;

    } else {

        AT_SendCommandEx(SIP, "%s", 0, NULL, ip);
        return at_cmd.state == AT_STATE_OK ? 1:0;
    }

}


uint8_t AT_GW(uint8_t param, char *gw) {

    if (!param) {
        AT_SendCommandEx(GW, NULL, 1000, AT_ResponseCallback);
        return 0;

    } else {

        AT_SendCommandEx(GW, "%s", 0, NULL, gw);
        return at_cmd.state == AT_STATE_OK ? 1:0;
    }

}


uint8_t AT_MASK(uint8_t param, char *mask) {

    if (!param) {
        AT_SendCommandEx(MASK, NULL, 1000, AT_ResponseCallback);
        return 0;

    } else {

        AT_SendCommandEx(MASK, "%s", 0, NULL, mask);
        return at_cmd.state == AT_STATE_OK ? 1:0;
    }

}


uint8_t AT_DNS(uint8_t param, char *dns) {

    if (!param) {
        AT_SendCommandEx(DNS, NULL, 1000, AT_ResponseCallback);
        return 0;

    } else {

        AT_SendCommandEx(DNS, "%s", 0, NULL, dns);
        return at_cmd.state == AT_STATE_OK ? 1:0;
    }

}

uint8_t AT_APAC(uint8_t param, char *apac) {

    if (!param) {
        AT_SendCommandEx(APAC, NULL, 1000, AT_ResponseCallback);
        return 0;

    } else {

        AT_SendCommandEx(APAC, "%s", 0, NULL, apac);
        return at_cmd.state == AT_STATE_OK ? 1:0;
    }

}


uint8_t AT_Apac(uint8_t param) {

    if (!param) {

        AT_SendCommandEx(APAC, NULL, 1000, AT_ResponseCallback);
        return 0;

    } else {

        AT_SendCommandEx(APAC, "%d", 0, NULL, param-1);
        return at_cmd.state == AT_STATE_OK ? 1:0;
    }

}


void AT_Rssi(void) {

    AT_SendCommandEx(RSSI, NULL, 1000, AT_ResponseCallback);
}


void sendWFdata(uint8_t *buff, uint16_t length, uint8_t id) {

    AT_SendU8Buff(GATTSEND, "%d,%d,",buff, length, id, length);
    if (at_cmd.state != AT_STATE_OK) {
        printf("send BT U8 data timeout!\n");
    }

}
#include <ctype.h>
#include <string.h>
void parse_input(const char *input, char *prefix, char *value) {
    int i = 0;

    while (input[i] != '\0' && input[i] != '=') {
        prefix[i] = input[i];
        i++;
    }
    prefix[i] = '\0';

    if (input[i] == '=') {
        i++;
    }

    int j = 0;
    while (input[i] != '\0') {
        value[j++] = input[i++];
    }
    value[j] = '\0';
}

// 将字符串映射为命令ID
AtCommandID get_command_id(const char *prefix) {
    if (strcmp(prefix, "TPMODE") == 0) return CMD_TPMODE;
		else if (strcmp(prefix, "WTPMODE") == 0) return CMD_WTPMODE;
    else if (strcmp(prefix, "SCAN") == 0) return CMD_SCAN;
    else if (strcmp(prefix, "REBOOT") == 0) return CMD_REBOOT;
    else if (strcmp(prefix, "RESTORE") == 0) return CMD_RESTORE;
    else if (strcmp(prefix, "STAT") == 0) return CMD_STAT;
    else if (strcmp(prefix, "DSCA") == 0) return CMD_DSCA;
    else if (strcmp(prefix, "NAME") == 0) return CMD_NAME;
    else if (strcmp(prefix, "LENAME") == 0) return CMD_LENAME;
    else if (strcmp(prefix, "GATTSEND") == 0) return CMD_GATTSEND;
    else if (strcmp(prefix, "MODE") == 0) return CMD_MODE;
    else if (strcmp(prefix, "ROLE") == 0) return CMD_ROLE;
    else if (strcmp(prefix, "RAP") == 0) return CMD_RAP;
    else if (strcmp(prefix, "WRAP") == 0) return CMD_WRAP;
    else if (strcmp(prefix, "LIP") == 0) return CMD_LIP;
    else if (strcmp(prefix, "DHCP") == 0) return CMD_DHCP;
    else if (strcmp(prefix, "SIP") == 0) return CMD_SIP;
    else if (strcmp(prefix, "WSIP") == 0) return CMD_WSIP;
    else if (strcmp(prefix, "GW") == 0) return CMD_GW;
    else if (strcmp(prefix, "WGW") == 0) return CMD_WGW;
    else if (strcmp(prefix, "MASK") == 0) return CMD_MASK;
    else if (strcmp(prefix, "WMASK") == 0) return CMD_WMASK;
    else if (strcmp(prefix, "DNS") == 0) return CMD_DNS;
    else if (strcmp(prefix, "WDNS") == 0) return CMD_WDNS;
    else if (strcmp(prefix, "APAC") == 0) return CMD_APAC;
		else if (strcmp(prefix, "WAPAC") == 0) return CMD_WAPAC;
    else if (strcmp(prefix, "RSSI") == 0) return CMD_RSSI;
    else if (strcmp(prefix, "SOCK") == 0) return CMD_SOCK;
    else if (strcmp(prefix, "WLANC") == 0) return CMD_WLANC;
    else if (strcmp(prefix, "WFSEND") == 0) return CMD_WFSEND;
    else if (strcmp(prefix, "CLOSE") == 0) return CMD_CLOSE;
    else return CMD_UNKNOWN;
}

// 用 switch-case 处理命令
void handle_command(const char *prefix, const char *value) {
    UART_Recv_t *BT_buff = get_UART_Recv_t(usart1);
    uint8_t res;
    switch (get_command_id(prefix)) {
    case CMD_TPMODE:
        printf("处理透传模式命令（AT+TPMODE）\n");
        if (AT_SetTpmod()) {
            sendBTdata("OK\r\n", strlen("OK\r\n"));
        } else {
            sendBTdata("ERROR\r\n", strlen("ERROR\r\n"));
        }

        break;
			case CMD_WTPMODE:
        printf("处理透传模式命令（AT+TPMODE）\n");
        AT_WTpmod();
        break;	
				
    case CMD_SCAN:
        printf("处理WiFi扫描命令（AT+SCAN）\n");
        AT_Scan();
        break;
    case CMD_REBOOT:				
				if (AT_Reboot()) {
            sendBTdata("OK\r\n", strlen("OK\r\n"));
        } else {
            sendBTdata("ERROR\r\n", strlen("ERROR\r\n"));
        }
        printf("处理软件复位命令（AT+REBOOT）\n");
        break;
    case CMD_RESTORE:
        printf("处理恢复出厂设置命令（AT+RESTORE）\n");
        break;
    case CMD_STAT:
        AT_Stat();
        printf("处理模块连接状态查询命令（AT+STAT）\n");
        break;
    case CMD_DSCA:
        AT_Dsca(value);
        printf("处理断开WiFi或蓝牙连接命令（AT+DSCA）\n");
        break;
    case CMD_NAME:

        res = AT_Name(value);
        if  (res) {
            if (res != 3) sendBTdata("OK\r\n", strlen("OK\r\n"));
        } else {
            sendBTdata("ERROR\r\n", strlen("ERROR\r\n"));
        }
        printf("处理BLE蓝牙名称设置命令（AT+NAME）\n");
        break;
    case CMD_LENAME:
        printf("处理BLE蓝牙名称带MAC后缀命令（AT+LENAME）\n");
        break;
    case CMD_GATTSEND:
        printf("处理BLE数据发送命令（AT+GATTSEND）\n");
        break;
    case CMD_MODE:
        printf("处理蓝牙模式设置命令（AT+MODE）\n");
        break;
    case CMD_ROLE:
        printf("处理STA/AP/STA+AP模式设置命令（AT+ROLE）\n");
        break;
    case CMD_RAP:
        res = AT_RAP(1, value) ;
        if  (res) {
            if (res != 3) sendBTdata("OK\r\n", strlen("OK\r\n"));

        } else {
            sendBTdata("ERROR\r\n", strlen("ERROR\r\n"));
        }
        printf("处理连接热点或查询热点命令（AT+RAP）\n");
        break;

    case CMD_WRAP:
        res = AT_RAP(0, value) ;

        printf("处理连接热点或查询热点命令（AT+RAP）\n");
        break;

    case CMD_LIP:
        AT_LIP();
        printf("处理查询模块当前IP地址命令（AT+LIP）\n");
        break;
    case CMD_DHCP:
        if (AT_DHCP(value)) {
            sendBTdata("OK\r\n", strlen("OK\r\n"));
        } else {
            sendBTdata("ERROR\r\n", strlen("ERROR\r\n"));
        }

        printf("处理DHCP模式设置命令（AT+DHCP）\n");
        break;
    case CMD_SIP:
        
		if (AT_SIP(1, value)) {
            sendBTdata("OK\r\n", strlen("OK\r\n"));
        } else {
            sendBTdata("ERROR\r\n", strlen("ERROR\r\n"));
        }
        printf("处理静态IP设置命令（AT+SIP）\n");
        break;
    case CMD_WSIP:
        AT_SIP(0, value);
        printf("处理静态IP设置命令（AT+SIP）\n");
        break;
    case CMD_GW:
			
				if (AT_GW(1, value)) {
            sendBTdata("OK\r\n", strlen("OK\r\n"));
        } else {
            sendBTdata("ERROR\r\n", strlen("ERROR\r\n"));
        }
        printf("处理网关设置命令（AT+GW）\n");
    case CMD_WGW:
        AT_GW(0, value);
        printf("处理网关设置命令（AT+GW）\n");
        break;
    case CMD_MASK:
		if (AT_MASK(1, value)) {
            sendBTdata("OK\r\n", strlen("OK\r\n"));
        } else {
            sendBTdata("ERROR\r\n", strlen("ERROR\r\n"));
        }
        printf("处理子网掩码设置命令（AT+MASK）\n");
        break;
    case CMD_WMASK:
        AT_MASK(0, value);
        printf("处理子网掩码设置命令（AT+MASK）\n");
        break;
    case CMD_DNS:
        if (AT_DNS(1, value)) {
            sendBTdata("OK\r\n", strlen("OK\r\n"));
        } else {
            sendBTdata("ERROR\r\n", strlen("ERROR\r\n"));
        }
        printf("处理DNS地址设置命令（AT+DNS）\n");
        break;
    case CMD_WDNS:
        AT_DNS(0, value);
        printf("处理DNS地址设置命令（AT+DNS）\n");
        break;
    case CMD_APAC:
			if (AT_APAC(1, value)) {
            sendBTdata("OK\r\n", strlen("OK\r\n"));
        } else {
            sendBTdata("ERROR\r\n", strlen("ERROR\r\n"));
        }
        printf("处理上电自动连接热点命令（AT+APAC）\n");
        break;
		case CMD_WAPAC:
        AT_APAC(0, value);
        printf("处理上电自动连接热点命令（AT+APAC）\n");
        break;
    case CMD_RSSI:
        printf("处理信号强度查询命令（AT+RSSI）\n");
        break;
    case CMD_SOCK:
        printf("处理SOCKET默认启动设置命令（AT+SOCK）\n");
        break;
    case CMD_WLANC:
        printf("处理启动SOCKET/MQTT连接命令（AT+WLANC）\n");
        break;
    case CMD_WFSEND:
        printf("处理通过SOCKET发送数据命令（AT+WFSEND）\n");
        break;
    case CMD_CLOSE:
        printf("处理断开TCP客户端连接命令（AT+CLOSE）\n");
        break;
    default:
        printf("无效或未知命令：%s\n", prefix);
        break;
    }
}

uint8_t AT_cmd_Dw(uint8_t *buff, uint16_t length, uint8_t source) {

    char symbol = buff[0];
    if (symbol != '+') {
        return 0;
    }

    char prefix[50] = {0};
    char value[50] = {0};
    parse_input((char *)&buff[1], prefix, value);

    handle_command(prefix, value);
    return 1;

}

