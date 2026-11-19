#include "flash.h"



static uint32_t GetSector(uint32_t Address)
{
	uint32_t sector = 0;

	if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
	{
		sector = FLASH_Sector_0;  
	}
	else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
	{
		sector = FLASH_Sector_1;  
	}
	else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
	{
		sector = FLASH_Sector_2;  
	}
	else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
	{
		sector = FLASH_Sector_3;  
	}
	else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
	{
		sector = FLASH_Sector_4;  
	}
	else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
	{
		sector = FLASH_Sector_5;  
	}
	else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
	{
		sector = FLASH_Sector_6;  
	}
	else if((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7))
	{
		sector = FLASH_Sector_7;  
	}

	return sector;
}


void Flash_WriteData(u32 address,char*data,u32 len)
{
	uint32_t uwStartSector = 0;
	uint32_t uwEndSector = 0;
	uint32_t uwAddress = 0;
	uint32_t uwSectorCounter = 0;

	__IO uint32_t uwData32 = 0;
	__IO uint32_t uwMemoryProgramStatus = 0;

	//1.解除FLASH相关寄存器的写保护  -写使能
	FLASH_Unlock();
	
	//2.清除相关标志位
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR); 
	
	//3.获取你要写入的那个地址所在的扇区
	uint32_t FLASH_USER_START_ADDR = address;
	uint32_t FLASH_USER_END_ADDR   = address+len;
	
	uwStartSector = GetSector(FLASH_USER_START_ADDR);
	uwEndSector = GetSector(FLASH_USER_END_ADDR);
	
	uwSectorCounter = uwStartSector;
	while (uwSectorCounter <= uwEndSector) 
	{
		//4.扇区擦除
		if (FLASH_EraseSector(uwSectorCounter, VoltageRange_3) != FLASH_COMPLETE)
		{ 
			printf("erase flash error\r\n");
		}

		uwSectorCounter += 8;
	}
	//5.写入数据
	uwAddress = FLASH_USER_START_ADDR;

	while (uwAddress < FLASH_USER_END_ADDR)
	{
		if (FLASH_ProgramByte(uwAddress,*data) == FLASH_COMPLETE)
		{
			uwAddress++;
			data++;
		}
		else
		{ 
			printf("write flash data error\r\n");
		}
	}
	
	//6.恢复写保护
	FLASH_Lock();
}

void Flash_ReadData(u32 address,char*data,u32 len)
{
	char*addr = (char*)address;
	
	for(int i=0; i<len; i++)
	{
		data[i] = *addr++;
	}
}

void Flash_Demo()
{
//	char write_buf[] = "123456789";
//	Flash_WriteData(0x08040000,write_buf,strlen(write_buf));
	
	delay_s(2);
	
	char read_buf[10] = {0};
	Flash_ReadData(0x08040000,read_buf,9);
	
	printf("read_buf:%s\r\n",read_buf);
	
	while(1);
}
