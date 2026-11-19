#ifndef __SPI_H
#define __SPI_H

#include "stm32f4xx.h"
#include "sys.h"

#define F_CS	PBout(14)

void Spi1_Init();
u8 Spi1_Recv_Send_Byte(u8 txdata);

#endif