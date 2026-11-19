#ifndef __DELAY_H__
#define __DELAY_H__

#include "stm32f4xx.h"

extern void SysTick_st_Init( void);
extern void delay_st_us( uint32_t nus);
extern void delay_st_ms( uint32_t nms);
extern void delay_st_s( uint32_t ns);

#endif
