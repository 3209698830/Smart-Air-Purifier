#ifndef __ICKEY_H
#define __ICKEY_H

#include "stm32f4xx.h"

extern void ICKey_TIM5_CH1_Init();
#define KEY0  GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0)

#endif
