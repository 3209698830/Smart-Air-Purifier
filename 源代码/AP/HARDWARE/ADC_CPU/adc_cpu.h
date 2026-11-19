#ifndef __ADC_CPU_H
#define __ADC_CPU_H

#include "stm32f4xx.h"
#include "stdio.h"
#include "delay.h"
void Adc_Cpu_Init(void);
u16 Adc_Cpu_Value(uint8_t ADC_Channel);
void Adc_Cpu_Demo();

#endif