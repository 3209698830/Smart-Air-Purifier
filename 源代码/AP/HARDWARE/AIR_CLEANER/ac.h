#ifndef __AC_H
#define __AC_H

#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stdio.h"
#include "uart1.h"
#include "pm.h"
#include "ro.h"
#include "esp8266.h"//wifi  +   串口

#include "lcd.h"
#include "touch.h"
#include "key.h"
#include "led.h"
#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "lv_demo_widgets.h"
#include "tim3.h"
#include "pwm_pm.h"
#include "dwt.h"

#define AG_TARGET_VALUE 500 //自动模式下，负离子发生器启动的目标值(pm2.5浓度值)

extern uint8_t PM_FAN_MODE;

// 所有底层硬件驱动头文件
#include "dht11.h"              // 温湿度传感器驱动
#include "my_key.h"                // 板载独立按键驱动
//#include "encoder.h"            // 旋转编码器模块驱动
//#include "tim_pwm.h"            // 定时器PWM输出，控制风扇转速
//#include "lcd.h"                // 2.8寸ILI9341 LCD彩屏底层驱动
//#include "relay.h"              // 负离子继电器IO驱动

//净化器全局状态结构体，唯一数据源，所有任务共享这份状态
typedef struct
{
    uint16_t pm25_value;        // PM2.5粉尘浓度数值，单位μg/m3
    uint8_t  temp;               // 环境温度
    uint8_t  humi;               // 环境湿度
    uint16_t  fan_speed;         // 风机转速 PWM计数范围：0~10000，0代表风扇关闭
    uint8_t  anion_switch;      // 负离子开关标志：0关闭  1开启
    uint8_t  work_mode;         // 工作模式：0自动模式 1手动模式 2睡眠模式
    uint8_t  alarm_flag;        // 超标报警标志：0无报警  1PM2.5超标报警
    uint8_t  lcd_backlight;     // LCD屏幕背光开关 0背光关闭 1背光点亮
    uint8_t  user_action_flag;  // 用户操作标记，按键/触屏/云端操作后置1，低功耗任务用来重置空闲计时
}Purifier_State_t;

int air();
void Get_PM_Value_ToFreeRtos(Purifier_State_t *purifier);
static void taskBegin(void* pvParameters);
//extern TaskHandle_t user_task1_handle;

extern Purifier_State_t purifier;  /* 净化器全局状态实例，定义在ac.c */

void Bafa_Data_set_moode1(void);
void Bafa_Data_set_moode2(void);
void Bafa_Data_set_auto(void);
void Set_user_act(uint8_t u);//设置用户是否正在使用标记


#endif
