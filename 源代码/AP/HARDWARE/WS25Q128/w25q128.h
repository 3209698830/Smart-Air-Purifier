#ifndef __W25Q128_H
#define __W25Q128_H

#include "stm32f4xx.h"
#include "spi.h"
#include "delay.h"
#include "stdio.h"

void W25q128_Init();
void W25q128_WriteEnable();
void W25q128_EraseSector(u32 addr);
void W25q128_WriteSector(u32 addr,char*data,int len);
void W25q128_ReadData(u32 addr,char*data,int len);
void W25q128_Demo();

#endif