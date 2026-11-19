#ifndef __DMA_PM_H
#define __DMA_PM_H

#include "stm32f4xx.h"
#include "stdio.h"
#include "dwt.h"

void Dma_PM_Init(void);
void Dma_PM_Demo(void);
void Led_PM_Status(uint8_t x);

#endif