#ifndef __RO_H
#define __RO_H
#include "stm32f4xx.h"
#include "dwt.h"
#include "uart1.h"

extern volatile int16_t encoder_cnt;
void PO_Init(void);
void EXTI9_5_IRQHandler(void);
void EXTI15_10_IRQHandler(void);
void PO_Demo(void);

#endif
