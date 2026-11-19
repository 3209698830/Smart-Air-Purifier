#ifndef __DWT_H
#define __DWT_H
#include "stm32f4xx.h"

void DWT_Init(void);
void DWT_Delay_us(volatile uint32_t us);
void DWT_Delay_ms(volatile uint32_t ms);
void DWT_Delay_s(volatile uint32_t s);

static inline uint32_t DWT_GetCnt(void)
{
    return DWT->CYCCNT;
}

#endif
