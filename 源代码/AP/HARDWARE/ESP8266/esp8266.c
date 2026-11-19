#include "esp8266.h"
#include "string.h"
#include "stdbool.h"
#include "ng.h"   // 负离子发生器控制(Control_NG)
#include "semphr.h"  // FreeRTOS 互斥量

//#include "dht11.h"

uint8_t MQTT_Connected = 0;

/* ESP8266 AT通信互斥量：多个任务(WifiCloud/NG)都会调用发布/订阅，
   必须串行化，否则共享的rx_buffer会被互相破坏。
   使用递归互斥量，因为 ProcessData→执行指令→发布 会嵌套调用发布函数 */
static SemaphoreHandle_t xEsp8266Mutex = NULL;

void ESP8266_Init()
{
    Usart3_Init(115200);
    if(xEsp8266Mutex == NULL)
    {
        xEsp8266Mutex = xSemaphoreCreateRecursiveMutex();
    }
}

void ESP8266_SendBytes(uint8_t *data, uint16_t len)
{
    Usart3_SendBytes(data,len);
}

ESP_Status ESP8266_SendCmd(char *cmd,const char*ack,int timeout)
{
    int cnt=0,ret=ESP_TIMEOUT;

    if(xSemaphoreTakeRecursive(xEsp8266Mutex, pdMS_TO_TICKS(100)) != pdTRUE) return ESP_TIMEOUT;

    //清空接收缓冲区（带中断保护）
    Usart3_RxClear();

    //发送命令
    Usart3_SendStr(cmd);
    
    //等待应答，超时检测
    while(1){
        if(strstr(rx_buffer,ack))
        {
            ret=ESP_OK;
            break;
        }
        else{
            vTaskDelay(pdMS_TO_TICKS(10));
            cnt++; //表示超时
            if(cnt>=timeout/10)
            {
                break;
            }
        }
    }
    
    xSemaphoreGiveRecursive(xEsp8266Mutex);
    return ret;
}
ESP_Status ESP8266_BytesAck(const char*ack,int timeout)
{
    ESP_Status ret = ESP_TIMEOUT;

    if(xSemaphoreTakeRecursive(xEsp8266Mutex, pdMS_TO_TICKS(100)) != pdTRUE) return ESP_TIMEOUT;

    //清空接收缓冲区（带中断保护）
    Usart3_RxClear();

    while(--timeout >=0){
        if(strstr(rx_buffer,ack))
        {
            ret = ESP_OK;
            vTaskDelay(pdMS_TO_TICKS(100));  // 短暂等待数据完整到达
            break;
        }
            
        
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    //printf("ESP8266_BytesAck:%s\r\n",rx_buffer);
    xSemaphoreGiveRecursive(xEsp8266Mutex);
    return ret;
}
//连接WIFI
ESP_Status ESP8266_ConnectWIFI()
{
    //printf("Esp8266_ConnectWIFI start\r\n");
	//DWT_Delay_s(2);
    vTaskDelay(pdMS_TO_TICKS(2000));
	//printf("TaskDelay\r\n");
    //1.退出透传模式
    ESP8266_SendCmd("+++","",1000);
    vTaskDelay(pdMS_TO_TICKS(1000));
	//printf("+++\r\n");

    //2.设置为非透传模式
    if(ESP8266_SendCmd("AT+CIPMODE=0\r\n","OK",3000) == ESP_TIMEOUT)//应答
    {
        //printf("AT+CIPMODE=0 failed\r\n");
        return ESP_ERROR;
    }
 
    //3.复位wifi
    if(ESP8266_SendCmd("AT+RST\r\n","OK",3000) == ESP_TIMEOUT)//应答
    {
        //printf("AT RST failed\r\n");
        return -1;
    }
    //printf("AT RST success\r\n");    
#if 0 
    //4.测试AT指令
    if(ESP8266_SendCmd("AT\r\n","OK",3000) == ESP_TIMEOUT)//应答
    {
        //printf("AT failed\r\n");
        return ESP_ERROR;
    }
    //printf("AT success\r\n");
#endif    
    //5.关闭回显功能，减少干扰
    if(ESP8266_SendCmd("ATE0\r\n","OK",2000) == ESP_TIMEOUT)//应答
    {
        //printf("ATE0 failed\r\n");
        return ESP_ERROR;
    }
    printf("ATE0 success\r\n");
    
    //6.查询和设置STA模式
    if(ESP8266_SendCmd("AT+CWMODE?\r\n","+CWMODE:1",3000) == ESP_TIMEOUT)//应答
    {
        //当前不在STA模式，则需要设置
        if(ESP8266_SendCmd("AT+CWMODE=1\r\n","OK",3000) == ESP_TIMEOUT)//应答
        {
            //printf("AT+CWMODE=1 ERROR\r\n");
            return ESP_ERROR;
        }
    }
    //printf("AT+CWMODE=1 success\r\n");
    
    //7.查询和设置当前WIFI的状态
    if(ESP8266_SendCmd("AT+CIPSTATUS\r\n","STATUS:5",2000) == ESP_OK)//应答
    {
        //printf("start connect WIFI...\r\n");
        //表明当前模块没有连接WIFI，需连接WIFI
        //设置模块WIFI名称，WIFI密码
        char cmd[100] = {0};
        sprintf(cmd,"AT+CWJAP=\"%s\",\"%s\"\r\n",WIFI_NAME,WIFI_PASSWD);
        if(ESP8266_SendCmd(cmd,"OK",8000) == ESP_TIMEOUT)//应答
        {
            //printf("WIFI CONNECT failed\r\n");    
            return ESP_ERROR;
        }    
    }
    
    //printf("WIFI_NAME:%s WIFI_PASSWD:%s\r\n",WIFI_NAME,WIFI_PASSWD); 
    //printf("WIFI connect success\r\n");
    
    return ESP_OK;
}
/*
 * 函数功能：建立TCP连接到巴法云
 * 分析：使用AT+CIPSTART建立TCP长连接，这是MQTT通信的基础
 */
ESP_Status ESP8266_ConnectTCP()
{
    //printf("ESP8266_ConnectTCP to MQTT Server...\r\n");
    
    //开启透明传输模式
//    ESP8266_SendCmd("AT+CIPMODE=1\r\n");
//    if(ESP8266_Recv_Ack("OK",3000) == -1)//应答
//    {
//        printf("AT+CIPMODE=1 failed\r\n");
//        return -4;
//    }
//    printf("AT+CIPMODE=1 success\r\n");
    //1.查询是已建立 TCP、UDP 或 SSL 传输连接  STATUS:3
    if(ESP8266_SendCmd("AT+CIPSTATUS\r\n","STATUS:3",2000) == ESP_OK)//应答
    {
        //断开连接
        if(ESP8266_SendCmd("AT+CIPCLOSE\r\n","OK",2000) == ESP_TIMEOUT)//应答
        {
            //printf("AT+CIPCLOSE failed\r\n");    
            return ESP_ERROR;
        }
    }
    //2.连接服务器和端口
    char cmd[100] = {0};
    sprintf(cmd,"AT+CIPSTART=\"TCP\",\"%s\",%d\r\n",BEMFA_SERVER,BEMFA_PORT);
    if(ESP8266_SendCmd(cmd,"OK",5000) == ESP_TIMEOUT)//应答
    {
        printf("connect server failed\r\n");
        return ESP_ERROR;
    }
    printf("connect server success\r\n");
    
    //进入透传模式
    //ESP8266_SendCmd("AT+CIPSEND\r\n");
    //vTaskDelay(pdMS_TO_TICKS(3000)); //必须要加

    //printf("ESP8266_ConnectTCP success\r\n"); 

    return ESP_OK;
}


// 从 +IPD 数据中提取 CONNACK 报文
static ESP_Status parse_connack_from_ipd(char *rx_buffer, uint16_t rx_len, 
                                uint8_t *connack_flags, uint8_t *return_code) {
    // 查找 "+IPD," 字符串
    char *ipd_start = strstr(rx_buffer, "+IPD,");
    if (!ipd_start) return 0;  // 未找到 +IPD
    
    // 跳过 "+IPD,"
    ipd_start += 5;

    // 读取长度字段（直到第一个冒号）
    uint16_t data_len = 0;
    while (*ipd_start != ':') {
        data_len = data_len * 10 + (*ipd_start - '0');
        ipd_start++;
    }
    ipd_start++;  // 跳过冒号
    
    // 检查长度是否为 4（CONNACK 固定长度）
    if (data_len < 4) return 0;
    
    // 提取 CONNACK 数据
    uint8_t *connack_data = (uint8_t *)ipd_start;
    
    // 验证固定报头
    if (connack_data[0] != 0x20) return ESP_ERROR;  // 非 CONNACK 报文
    if (connack_data[1] != 0x02) return ESP_ERROR;  // 剩余长度错误
    
    // 解析可变报头
    *connack_flags = connack_data[2];  // 连接确认标志
    *return_code = connack_data[3];    // 返回码
    
    return ESP_OK;  // 解析成功
}
// 处理 CONNACK 返回码
ESP_Status handle_connack_return_code(uint8_t return_code) {
    
    ESP_Status ret = ESP_ERROR;
    
    switch (return_code) {
        case 0x00:
            // 连接成功
            //printf("MQTT Connection Accepted\n");
            MQTT_Connected = 1;
            ret = ESP_OK;
            break;
        case 0x01:
            //printf("Connection Refused: Unacceptable Protocol Version\n");
            break;
        case 0x02:
            //printf("Connection Refused: Identifier Rejected\n");
            break;
        case 0x03:
            //printf("Connection Refused: Server Unavailable\n");
            break;
        case 0x04:
            //printf("Connection Refused: Bad User Name or Password\n");
            break;
        case 0x05:
            //printf("Connection Refused: Not Authorized\n");
            break;
        default:
            //printf("Connection Refused: Unknown Error (0x%02X)\n", return_code);
            break;
    }
    return ret;
}
/*
 * 函数功能：发送MQTT CONNECT报文进行认证
 * 分析：
 * 巴法云MQTT连接使用特殊格式：
 * - ClientID = UID
 * - 用户名 = 空
 * - 密码 = 空
 * 
 * MQTT CONNECT报文结构（固定头+可变头+载荷）：
 * 固定头：0x10 + 剩余长度
 * 可变头：协议名"MQTT"(4字节)、协议级别(0x04)、连接标志、保活时间
 * 载荷：ClientID
 */
ESP_Status ESP8266_ConnectMQTT(void)
{
    if(xSemaphoreTakeRecursive(xEsp8266Mutex, portMAX_DELAY) != pdTRUE) return ESP_ERROR;

    uint8_t packet[256] = {0};
    uint16_t index = 0;
    
    // ---- 构建MQTT CONNECT报文 ----
    
    // 1. 固定头 - 报文类型
    packet[index++] = 0x10;  // CONNECT报文
    
    // 2. 计算剩余长度（可变头+载荷）  有效载荷：数据长度+数据
    // 可变头10字节 + ClientID(2字节长度+内容)
    uint16_t remaining_len = 10 + 2 + strlen(BEMFA_UID);
    
    // 编码剩余长度
    packet[index++] = remaining_len;  // 假设长度小于128字节
    
    // 3. 可变头：协议名 "MQTT"
    packet[index++] = 0x00;  // 协议名长度高位
    packet[index++] = 0x04;  // 协议名长度低位（4字节）
    packet[index++] = 'M';
    packet[index++] = 'Q';
    packet[index++] = 'T';
    packet[index++] = 'T';
    
    // 4. 协议级别：MQTT 3.1.1
    packet[index++] = 0x04;
    
    // 5. 连接标志：Clean Session=1, 用户名和密码标志均为0
    // 位1:Clean Session=1, 其他标志位为0
    packet[index++] = 0x02;  // 0000 0010
    
    // 6. 保活时间：60秒
    packet[index++] = 0x00;  // 高位
    packet[index++] = 0x3C;  // 低位（60）
    
    // 7. 载荷：ClientID（使用用户私钥）
    uint8_t clen = strlen(BEMFA_UID);
    packet[index++] = 0x00;  // 长度高位
    packet[index++] = clen;  // 长度低位
    memcpy(&packet[index], BEMFA_UID, clen);
    index += clen;
    
    // 8. 通过AT+CIPSEND发送报文
    char cmd[32]={0};
    sprintf(cmd, "AT+CIPSEND=%d\r\n", index);
    
    // 发送长度指令
    ESP_Status conn_ret = ESP_ERROR;
    if(ESP8266_SendCmd(cmd, ">", 2000) == ESP_OK)
    {
        // 发送MQTT CONNECT报文
        ESP8266_SendBytes(packet,index);
        
        // 等待CONNACK响应
        if(ESP8266_BytesAck("+IPD",5000) == ESP_OK)
        {
            uint8_t connack_flags= -1;
            uint8_t return_code = -1;
            if(parse_connack_from_ipd(rx_buffer, rx_index1,&connack_flags,&return_code) ==ESP_OK)
            {
                conn_ret = handle_connack_return_code(return_code);
            }   
        }
    }
    
    if(conn_ret != ESP_OK) {
        MQTT_Connected = 0;
    }
    xSemaphoreGiveRecursive(xEsp8266Mutex);
    return conn_ret;
}
#if 1
/*
 * 函数功能：订阅MQTT主题
 * 参数：topic - 要订阅的主题
 * 分析：
 * SUBSCRIBE报文结构：
 * 固定头：0x82 + 剩余长度
 * 可变头：报文标识符（2字节）
 * 载荷：主题过滤器（2字节长度+内容+QoS）
 */
void ESP8266_MQTTSubscribe(char *topic)
{
    if(xSemaphoreTakeRecursive(xEsp8266Mutex, portMAX_DELAY) != pdTRUE) return;

    uint8_t packet[256] = {0};
    uint16_t index = 0;
    
    // 1. 固定头 - 报文类型
    packet[index++] = 0x82;  // SUBSCRIBE报文
    
    // 2. 剩余长度
    uint16_t remaining_len = 2 + 2 + strlen(topic) + 1; // 报文标识符(2) + 主题(2+len+QoS(1))
    packet[index++] = remaining_len;
    
    // 3. 可变头：报文标识符（任意值）
    packet[index++] = 0x00;
    packet[index++] = 0x01;  // 报文标识符=1
    
    // 4. 载荷：主题过滤器
    uint8_t tlen = strlen(topic);
    packet[index++] = 0x00;
    packet[index++] = tlen;
    memcpy(&packet[index], topic, tlen);
    index += tlen;
    packet[index++] = 0x00;  // QoS=0
    
    // 5. 发送
    char cmd[32];
    sprintf(cmd, "AT+CIPSEND=%d\r\n", index);
    
    if(ESP8266_SendCmd(cmd, ">", 2000) == ESP_OK)
    {
        ESP8266_SendBytes(packet, index);
        /* 必须等SEND OK！否则紧接着的下一条AT+CIPSEND会在ESP8266
           仍处于数据接收阶段时到达，被当作报文数据拼进本条SUBSCRIBE，
           导致订阅报文损坏、云端显示"未订阅" */
        if(ESP8266_BytesAck("SEND OK", 3000) != ESP_OK)
        {
            MQTT_Connected = 0;  // 发送失败，连接可能已断
        }
        //printf("Subscribe: %s\r\n", topic);
    }
    //订阅确认
    xSemaphoreGiveRecursive(xEsp8266Mutex);
}
#endif

/*
 * 函数功能：发布MQTT消息
 * 返回值：ESP_OK 发送成功(收到SEND OK)；ESP_ERROR 发送失败(连接可能已断开)
 */
ESP_Status ESP8266_MQTTPublish(char *topic, char *msg)
{
    ESP_Status ret = ESP_ERROR;
    if(xSemaphoreTakeRecursive(xEsp8266Mutex, portMAX_DELAY) != pdTRUE) return ESP_ERROR;

    uint8_t packet[512];
    uint16_t index = 0;

    // 1. 固定头 - 报文类型（QoS=0）
    packet[index++] = 0x30;  // PUBLISH报文，QoS=0

    // 2. 计算剩余长度
    uint16_t remaining_len = 2 + strlen(topic) + strlen(msg);
    if(remaining_len > 127) {
        // MQTT剩余长度>127需要变长编码，此处简单拒绝超长消息
        xSemaphoreGiveRecursive(xEsp8266Mutex);
        return ESP_ERROR;
    }
    packet[index++] = remaining_len;

    // 3. 可变头：主题
    uint8_t tlen = strlen(topic);
    packet[index++] = 0x00;
    packet[index++] = tlen;
    memcpy(&packet[index], topic, tlen);
    index += tlen;

    // 4. 载荷：消息内容
    uint8_t mlen = strlen(msg);
    memcpy(&packet[index], msg, mlen);
    index += mlen;

    // 5. 发送
    char cmd[32] = {0};
    sprintf(cmd, "AT+CIPSEND=%d\r\n", index);

    if(ESP8266_SendCmd(cmd, ">", 2000) == ESP_OK)
    {
        Usart3_SendBytes(packet, index);
        // 必须等待 SEND OK 确认，否则无法判断连接是否断开
        if(ESP8266_BytesAck("SEND OK", 3000) == ESP_OK)
        {
            ret = ESP_OK;
        }
        else
        {
            // 没收到 SEND OK，很可能 TCP/MQTT 连接已断
            MQTT_Connected = 0;
            ret = ESP_ERROR;
        }
    }
    else
    {
        // 没收到 ">" 提示符，连接异常
        MQTT_Connected = 0;
        ret = ESP_ERROR;
    }

    xSemaphoreGiveRecursive(xEsp8266Mutex);
    return ret;
}

/*
 * 函数功能：发送 MQTT PINGREQ 保活心跳
 */
ESP_Status ESP8266_MQTTPing(void)
{
    ESP_Status ret = ESP_ERROR;
    if(xSemaphoreTakeRecursive(xEsp8266Mutex, portMAX_DELAY) != pdTRUE) return ESP_ERROR;

    // PINGREQ 固定两字节: 0xC0 0x00
    uint8_t ping[2] = {0xC0, 0x00};
    char cmd[32] = {0};
    sprintf(cmd, "AT+CIPSEND=2\r\n");

    if(ESP8266_SendCmd(cmd, ">", 2000) == ESP_OK)
    {
        Usart3_SendBytes(ping, 2);
        // PINGRESP 报文为 0xD0 0x00，通过 +IPD,2 包裹
        if(ESP8266_BytesAck("+IPD", 3000) == ESP_OK)
        {
            ret = ESP_OK;
        }
        else
        {
            MQTT_Connected = 0;
            ret = ESP_ERROR;
        }
    }
    else
    {
        MQTT_Connected = 0;
        ret = ESP_ERROR;
    }

    xSemaphoreGiveRecursive(xEsp8266Mutex);
    return ret;
}
/**
 * 函数功能：执行风扇控制指令（on/off/auto）
 */
static void bemfa_exec_fan_cmd(const char *msg)
{
    if (strcmp(msg, "on") == 0) {
        PM_FAN_MODE=1;//设置手动模式标志位
        Bafa_Data_set_moode1();
    } else if (strcmp(msg, "off") == 0) {
        PM_FAN_MODE=1;//设置手动模式标志位
        Bafa_Data_set_moode2();
    } else if (strcmp(msg, "auto") == 0) {
        PM_FAN_MODE=0;//自动模式
        Bafa_Data_set_auto();
    }
}

/**
 * 函数功能：执行负离子(NG006)控制指令
 * 云端发on/off → 切入手动模式并执行开关（模式牵连风扇，共用PM_FAN_MODE）
 * 云端发auto  → 恢复自动模式（负离子随后由PM浓度自动控制）
 */
static void bemfa_exec_ng_cmd(const char *msg)
{
    if (strcmp(msg, "on") == 0) {
        PM_FAN_MODE = 1;      // 手动模式
        Control_NG(1);        // 开启负离子发生器
    } else if (strcmp(msg, "off") == 0) {
        PM_FAN_MODE = 1;      // 手动模式
        Control_NG(0);        // 关闭负离子发生器
    } else if (strcmp(msg, "auto") == 0) {
        PM_FAN_MODE = 0;      // 恢复自动模式
    }
}
/**
 * 解析巴法云二进制协议格式
 * 格式: [类型1字节][0x0B][主题长度2字节大端][主题名称][消息内容]
 */
static void parse_bemfa_binary_protocol(uint8_t *data, uint16_t len)
{
    uint16_t offset = 0;
    
    // 1. 跳过消息类型字节（'0' = 0x30）
    offset++;
    
    // 2. 跳过分隔符 
    offset++;
    
    // 3. 读取主题长度（2字节，大端模式）
    if (offset + 2 > len) {
        //printf("Error: Binary protocol too short for topic length\r\n");
        return;
    }
    uint16_t topic_len = (data[offset] << 8) | data[offset + 1];
    offset += 2;
    
    // 4. 提取主题名称
    if (offset + topic_len > len) {
        //printf("Error: Topic length exceeds data length\r\n");
        return;
    }
    char topic_name[64] = {0};
    if (topic_len > 0 && topic_len < 63) {
        memcpy(topic_name, &data[offset], topic_len);
        offset += topic_len;
    }
    
    // 5. 提取消息内容
    uint16_t msg_len = len - offset;
    char message[32] = {0};
    if (msg_len > 0 && msg_len < 31) {
        memcpy(message, &data[offset], msg_len);
    } else {
        msg_len = 0;
    }

    // 6. 修剪消息尾部干扰字符（\r\n、空格、巴法云控制字符0x01等）
    while (msg_len > 0 && ((uint8_t)message[msg_len-1] <= 0x20)) {
        message[--msg_len] = '\0';
    }
    // 修剪消息头部控制字符
    char *msg_p = message;
    while (msg_len > 0 && (uint8_t)(*msg_p) <= 0x20 && *msg_p != '\0') {
        msg_p++;
        msg_len--;
    }

    // 7. 打印调试信息
    //printf("Topic: %s, Message: %s\r\n", topic_name, message);

    // 8. 根据主题名称执行控制指令
    if (strcmp(topic_name,TOPIC_Fan) == 0) {
        bemfa_exec_fan_cmd(msg_p);
    }
    else if (strcmp(topic_name,TOPIC_NG) == 0) {
        bemfa_exec_ng_cmd(msg_p);
    }
	else {
        //printf("Unknown topic: %s\r\n", topic_name);
    }
}
/**
 * 函数功能：处理ESP8266接收到的数据（巴法云平台优化版）
 * 优化要点：
 * 1. 使用长度字段解析，避免strlen导致的长度错误
 * 2. 支持巴法云控制字符（0x01）的自动跳过
 * 3. 支持带主题标识的协议格式解析
 * 4. 增加缓冲区保护，防止溢出
 * 5. 增加数据校验和安全检查
 */
void ESP8266_ProcessData(void)
{
    if(xSemaphoreTakeRecursive(xEsp8266Mutex, pdMS_TO_TICKS(100)) != pdTRUE) return;

    // 检查缓冲区是否为空
    if (rx_buffer[0] == '\0') {
        xSemaphoreGiveRecursive(xEsp8266Mutex);
        return;
    }

    // 检查是否收到+IPD数据
    char *p = strstr((char *)rx_buffer, "+IPD,");
    if (p == NULL) {
        // 不是+IPD数据，可能是AT指令响应或其他，交给其他处理函数
        Usart3_RxClear();
        xSemaphoreGiveRecursive(xEsp8266Mutex);
        return;
    }

    // 解析数据长度和数据内容（使用长度字段，而非strlen）
    char *len_start = p + 5;  // 跳过"+IPD,"
    char *data_start = strchr(len_start, ':');

    if (data_start == NULL) {
        Usart3_RxClear();
        xSemaphoreGiveRecursive(xEsp8266Mutex);
        return;
    }

    // 解析长度字段（字符串转数字）
    uint16_t data_len = 0;
    char *temp = len_start;
    while (*temp != ':') {
        if (*temp < '0' || *temp > '9') {
            Usart3_RxClear();
            xSemaphoreGiveRecursive(xEsp8266Mutex);
            return;
        }
        data_len = data_len * 10 + (*temp - '0');
        temp++;
    }
    data_start++;  // 跳过':'

    // 安全检查：确保数据长度在合理范围内
    if (data_len == 0 || data_len > RX_BUF_SIZE - 1) {
        Usart3_RxClear();
        xSemaphoreGiveRecursive(xEsp8266Mutex);
        return;
    }

    // ====== 关键：等待+IPD数据完整再解析 ======
    uint16_t payload_off = (uint16_t)(data_start - rx_buffer);
    if (rx_index1 < payload_off + data_len) {
        if (rx_index1 >= RX_BUF_SIZE - 2) {
            Usart3_RxClear();
        }
        xSemaphoreGiveRecursive(xEsp8266Mutex);
        return;  // 等待下次循环继续接收
    }

    // 打印原始接收数据（调试用）
    //printf("Raw[%d]: ", data_len);
    for(uint16_t i = 0; i < data_len; i++) {
        //printf("%02X ", data_start[i]);
    }
    //printf("\r\n");

    // ====== 巴法云平台协议解析 ======
    // 协议格式：[类型1字节][0x0B][主题长度2字节大端][主题名称][消息内容]

    // 跳过巴法云控制字符（0x01），平台下发消息前可能带此字节
    while (data_len > 0 && (uint8_t)data_start[0] == 0x01) {
        data_start++;
        data_len--;
    }

    // 处理方式1：MQTT PUBLISH(QoS0=0x30) 或 巴法云二进制协议(0x50='P')
    // 类型字节低4位为PUBLISH标志位，所以用0xF0掩码判断
    if (data_len >= 4 && ((((uint8_t)data_start[0] & 0xF0) == 0x30) ||
                          ((uint8_t)data_start[0] == 0x50))) {
        // 解析巴法云二进制协议
        parse_bemfa_binary_protocol((uint8_t *)data_start, data_len);
    }
    // 处理方式2：纯文本指令兜底（部分设备类型下发格式为 +IPD,2:on）
    else if (data_len >= 2 && data_len < 16) {
        char plain[16] = {0};
        memcpy(plain, data_start, data_len);
        // 修剪首尾控制字符和空白
        char *s = plain;
        while (*s && (uint8_t)*s <= 0x20) s++;
        char *e = s + strlen(s);
        while (e > s && (uint8_t)*(e-1) <= 0x20) e--;
        *e = '\0';
        if (strcmp(s, "on") == 0 || strcmp(s, "off") == 0 || strcmp(s, "auto") == 0) {
            bemfa_exec_fan_cmd(s);
        }
    }
    // 其他格式（SUBACK等），直接丢弃

    // 清空缓冲区，准备下一次接收（注意：数据已被处理，可以安全清空）
    Usart3_RxClear();
    xSemaphoreGiveRecursive(xEsp8266Mutex);
}

static TaskHandle_t wifiTaskHandle = NULL;

/* WiFi云端通信任务函数：所有联网+上报逻辑放这里 */
static void Task_WifiDemo(void *pvParameters)
{
    //DWT_Init();

    // 逐级建立连接：WiFi → TCP → MQTT
	while(ESP8266_ConnectWIFI() != ESP_OK)
    {
        //printf("connect failed\r\n");
        //printf("retry connect.....\r\n");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
	while(ESP8266_ConnectTCP() != ESP_OK)
    {
        printf("ESP8266_ConnectTCP failed\r\n");
        printf("retry connect.....\r\n");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
	while(ESP8266_ConnectMQTT() != ESP_OK)
    {
        printf("ESP8266_ConnectMQTT failed\r\n");
        printf("retry connect.....\r\n");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
	// 仅订阅风扇主题（温湿度、PM2.5只上报不订阅，无需订阅）
	ESP8266_MQTTSubscribe(TOPIC_Fan);
	vTaskDelay(pdMS_TO_TICKS(2000));
	
	int timeout = 0;
	
    while(1)
    {
        // 1. 处理云端下发的风扇控制指令
        ESP8266_ProcessData();
        
        // 2. 定时上报所有设备数据
        if(++timeout % 5 == 0)
        {
            char msg[32] = {0};
            // ---------- 1. 上报风扇状态 → fan003 ----------
                ESP8266_MQTTPublish(TOPIC_Fan, "auto");
			
            // ---------- 2. 上报温度 → temp004 ----------
			snprintf(msg, sizeof(msg), "%d", timeout);
			ESP8266_MQTTPublish(TOPIC_Tem, msg);
            // ---------- 3. 上报湿度 → humidity004 ----------
            snprintf(msg, sizeof(msg), "%d", timeout);
			ESP8266_MQTTPublish(TOPIC_Hum, msg);
            // ---------- 4. 上报PM2.5 → pm004 ----------
            snprintf(msg, sizeof(msg), "%d", timeout);
			ESP8266_MQTTPublish(TOPIC_Pm, msg);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/* 原来的ESP8266_Demo：现在只负责创建wifi任务，不跑循环、不开调度器 */
void ESP8266_Demo(void)
{
    xTaskCreate(
        Task_WifiDemo,        //任务函数
        "WifiDemoTask",       //任务名
        512,                   //栈大小，不够就调到1024
        NULL,                  //参数
        2,                     //优先级，按需修改
        &wifiTaskHandle
    );
	
	//开启调度器
    vTaskStartScheduler();
}

