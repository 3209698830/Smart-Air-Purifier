#ifndef __ADC_H
#define __ADC_H

#include "stm32f4xx.h"
#include "stdio.h"
#include "delay.h"
void Adc_PA5_Init(void);
u16 Get_Adc_Value(uint8_t ADC_Channel);
void Adc_Demo();

#endif