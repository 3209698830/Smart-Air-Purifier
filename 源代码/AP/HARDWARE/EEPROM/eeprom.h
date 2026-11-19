#ifndef __EEPROM_H
#define __EEPROM_H

#include "stm32f4xx.h"
#include "stdio.h"
#include "delay.h"

void EEPROM_GPIO_Config(void);
uint32_t EEPROM_PageWrite(u8 WriteAddr, u8* pBuffer, u8 NumByteToWrite);
uint32_t EEPROM_BufferRead(u8 ReadAddr, u8* pBuffer, u16 NumByteToRead);

void EEPROM_Demo();

#endif /* __I2C_EE_H */
