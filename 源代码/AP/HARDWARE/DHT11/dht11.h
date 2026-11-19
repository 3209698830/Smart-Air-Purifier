#ifndef __DHT11_H_
#define __DHT11_H_

#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "dwt.h"

//#include "delay.h"
#include "sys.h"



extern void Dht11_Init(void);
extern void Dht11_Pin_Mode(GPIOMode_TypeDef mode);
extern int  Dht11_Start(void);
extern uint8_t Dht11_Read_Byte(void);
extern int  Dht11_Data(u8 *data);
extern void Dht11_Demo();
int DHT11_GetData(uint8_t *temp, uint8_t *humi);


#endif
