#ifndef __USART3_H
#define __USART3_H

#include "stm32f4xx.h"
#include "string.h"

#define RX_BUF_SIZE  512
extern char rx_buffer[RX_BUF_SIZE];
extern int rx_index1 ;


extern void Usart3_Init(int BaudRate);
extern void Usart3_SendStr(char *str);
extern void Usart3_SendBytes( uint8_t *data, uint16_t len);
extern void Usart3_RxClear(void);   /* clear rx buffer (irq-safe) */
extern void Usart3_LoopbackTest(void);
#endif