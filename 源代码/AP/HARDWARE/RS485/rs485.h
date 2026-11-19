#ifndef __RS485_H
#define __RS485_H

#include "stm32f4xx.h"
#include "sys.h"
#include "delay.h"
#include "stdio.h"
#include "string.h"
#define  RS485_RE  PGout(8)	

extern void RS485_Init(int baud);
extern void RS485_Demo();

#endif