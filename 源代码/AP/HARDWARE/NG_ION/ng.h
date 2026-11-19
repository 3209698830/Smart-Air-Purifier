#ifndef __NG_H
#define __NG_H
#include "stm32f4xx.h"

void NG_Init(void);
void Control_NG(uint8_t x);//发生器 开：x=1   关：x=0
uint8_t Read_stat_NG(void);//切换读取工作状态  正在运行：返回1   关闭状态：返回0

#endif
