#ifndef __ADC_LIGHT_H
#define __ADC_LIGHT_H

#include "stm32f4xx.h"
#include "stdio.h"
#include "delay.h"

void Adc_Light_Init(void);
u16 Adc_Light_Get_Value(uint8_t ADC_Channel);
void Adc_Light_Demo();
void Adc_Light_PWM_Demo();

#endif