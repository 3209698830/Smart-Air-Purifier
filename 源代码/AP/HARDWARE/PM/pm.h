#ifndef __PM_H
#define __PM_H
#include "stm32f4xx.h"
#include "adc_pm.h"
#include "dwt.h"
//#include "delay.h"
//#include "st_delay.h"

#include <stdio.h>
#include <stdint.h>

//==================== 宏定义滤波参数，可随时修改采样次数 ====================
#define PM_SAMPLE_CNT       10      //滤波采样次数：去极值平均，采10次
#define NO_DUST_MV          600     //无尘基准电压 600mV = 0.6V
#define COV_RATIO           0.17f  //换算系数 (1000/5.88 ≈ 170)

void PM_Init(void);
void Led_Status(uint8_t x);
void Get_PM_Value_demo(void);
uint16_t Filter_ExtremeAverage(uint16_t raw_val);

uint16_t Acquisition_timing(void);//采集时序函数

#endif
