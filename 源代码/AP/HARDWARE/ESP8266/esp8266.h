#ifndef __ESP8266_H
#define __ESP8266_H

#include "stm32f4xx.h"
//#include "delay.h"
#include "FreeRTOS.h"
#include "task.h"
#include "ac.h"
#include "stdio.h"
#include "dwt.h"

#include "usart3.h"

// 巴法云配置参数
#define BEMFA_SERVER     "bemfa.com"     // 巴法云MQTT服务器
#define BEMFA_PORT       9501            // MQTT端口
#define BEMFA_UID        "2c3bd7abad0f4b06a6787574ecaf617f"  // 您的私钥

//主题名--设备
#define TOPIC_Fan	"fan003"
#define TOPIC_Hum   "humidity004"
#define TOPIC_Tem   "temp004"
#define TOPIC_Pm    "pm004"
#define TOPIC_NG    "NG006"
//#define TOPIC_TEMP        "TEMP004"

#define WIFI_NAME        "R_Note_11T_Pro"
#define WIFI_PASSWD      "00000000"



// ESP8266状态枚举
typedef enum {
    ESP_OK = 0,
    ESP_ERROR = 1,
    ESP_TIMEOUT = 2
} ESP_Status;


extern uint8_t MQTT_Connected;

extern void ESP8266_Init();
extern ESP_Status ESP8266_ConnectWIFI();
extern ESP_Status ESP8266_ConnectTCP();
extern ESP_Status ESP8266_SendCmd(char *cmd,const char*ack,int timeout);
extern void ESP8266_SendBytes(uint8_t *data, uint16_t len);
extern ESP_Status ESP8266_BytesAck(const char*ack,int timeout);

extern ESP_Status ESP8266_ConnectMQTT(void);
extern void ESP8266_MQTTSubscribe(char *topic);
extern ESP_Status ESP8266_MQTTPublish(char *topic, char *msg);
extern ESP_Status ESP8266_MQTTPing(void);   /* MQTT 保活心跳 PINGREQ */
extern void ESP8266_ProcessData(void);

extern void ESP8266_Demo();

#endif 
