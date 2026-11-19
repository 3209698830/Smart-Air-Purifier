#ifndef __MY_KEY_H
#define __MY_KEY_H

#include "stm32f4xx.h"
#include "ng.h"
#include "ac.h"

// 按键引脚定义
#define KEY0_PIN        GPIO_Pin_0
#define KEY0_PORT       GPIOA
#define KEY0_CLK        RCC_AHB1Periph_GPIOA

#define KEY1_PIN        GPIO_Pin_2
#define KEY1_PORT       GPIOE
#define KEY1_CLK        RCC_AHB1Periph_GPIOE

#define KEY2_PIN        GPIO_Pin_3
#define KEY2_PORT       GPIOE
#define KEY2_CLK        RCC_AHB1Periph_GPIOE

#define KEY3_PIN        GPIO_Pin_4
#define KEY3_PORT       GPIOE
#define KEY3_CLK        RCC_AHB1Periph_GPIOE

extern uint8_t PM_FAN_MODE;		//风扇模式   0:自动(默认)    1:手动

// 按键数量
#define KEY_NUM         4

// 按键ID枚举
typedef enum {
    KEY_0 = 0,      // PA0
    KEY_1,          // PE2
    KEY_2,          // PE3
    KEY_3,          // PE4
    KEY_NONE
} KEY_ID;

// 按键状态枚举
typedef enum {
    KEY_STATE_IDLE = 0,         // 空闲/释放状态
    KEY_STATE_PRESSING,         // 确认按下中（消抖计时）
    KEY_STATE_PRESSED,          // 已确认按下
    KEY_STATE_HOLDING,          // 长按保持
    KEY_STATE_RELEASING,        // 确认释放中（消抖计时）
} KEY_STATE;

// 按键事件枚举
typedef enum {
    KEY_EVENT_NONE = 0,
    KEY_EVENT_SHORT_PRESS,      // 短按事件
    KEY_EVENT_LONG_PRESS,       // 长按事件
    KEY_EVENT_RELEASE,          // 释放事件
} KEY_EVENT;

// 按键配置参数
#define KEY_SCAN_INTERVAL   10      // 扫描间隔 10ms
#define KEY_DEBOUNCE_CNT    2       // 消抖计数阈值 20ms (2*10ms)
#define KEY_LONG_CNT        100     // 长按计数阈值 1s (100*10ms)

// 函数声明
void MY_KEY_Init(void);
void KEY_ScanTask(void);            // 需每10ms调用一次
KEY_EVENT KEY_GetEvent(uint8_t key_id);
uint8_t KEY_IsPressed(uint8_t key_id);
uint32_t KEY_GetHoldTime(uint8_t key_id);
void KEY_Demo();
void KEY_To_PMpj();//针对空气净化项目的功能函数
#endif