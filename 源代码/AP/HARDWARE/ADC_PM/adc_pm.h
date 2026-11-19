#ifndef __ADC_PM_H
#define __ADC_PM_H

#define ADC_TIME_OUT 10000000    //等待EOC时，超时数值

#include "stm32f4xx.h"
#include "stdio.h"
#include "dwt.h"

void ADC_PM_Init(void);
uint16_t Get_ADC_PM_Value(void);
void ADC_PM_Demo(void);


#endif
