#include "w25q128.h"

void W25q128_Init()
{
	Spi1_Init();
}

void W25q128_WriteEnable()
{
	F_CS = 0;
	
	Spi1_Recv_Send_Byte(0x06);
	
	F_CS = 1;
}

void W25q128_EraseSector(u32 addr)
{
	W25q128_WriteEnable();
	
	F_CS = 0;
	//发送擦除的指令
	Spi1_Recv_Send_Byte(0x20);
	//发送你要擦除的地址 
	Spi1_Recv_Send_Byte(addr>>16);
	Spi1_Recv_Send_Byte(addr>>8);
	Spi1_Recv_Send_Byte(addr);
	
	F_CS = 1;
	
	/**获取状态寄存器1的数据，判断BUSY（bit0）位  擦除/写入进展  
	BUSY（bit0）位 为1 代表正在擦除或者写入 
	BUSY（bit0）位 为0 代表擦除完成或者写入完成 
	*/
	while(1){
		F_CS = 0;
		Spi1_Recv_Send_Byte(0x05);

		u8 status = Spi1_Recv_Send_Byte(0x00);
		if((status & (0x01)) == 0)
		{
			F_CS = 1;
			break;
		}
		F_CS = 1;
	}
}

void W25q128_WriteSector(u32 addr,char*data,int len)
{
	W25q128_WriteEnable();
	
	F_CS = 0;
	
	//发送写操作指令
	Spi1_Recv_Send_Byte(0x02);
	
	//发送你要写入的地址
	Spi1_Recv_Send_Byte(addr>>16);
	Spi1_Recv_Send_Byte(addr>>8);
	Spi1_Recv_Send_Byte(addr);	
	
	for(int i=0; i<len; i++)
	{
		Spi1_Recv_Send_Byte(data[i]);
	}
	
	F_CS = 1;
	
	/**获取状态寄存器1的数据，判断BUSY（bit0）位  擦除/写入进展  
	BUSY（bit0）位 为1 代表正在擦除或者写入 
	BUSY（bit0）位 为0 代表擦除完成或者写入完成 
	*/
	while(1){
		F_CS = 0;
		Spi1_Recv_Send_Byte(0x05);

		u8 status = Spi1_Recv_Send_Byte(0x00);
		if((status & (0x01)) == 0)
		{
			F_CS = 1;
			break;
		}
		F_CS = 1;
	}
	
}

void W25q128_ReadData(u32 addr,char*data,int len)
{
	F_CS = 0;
	
	//发送读操作指令
	Spi1_Recv_Send_Byte(0x03);
	
	//发送你要读取的地址
	Spi1_Recv_Send_Byte(addr>>16);
	Spi1_Recv_Send_Byte(addr>>8);
	Spi1_Recv_Send_Byte(addr);
	
	for(int i=0; i<len; i++)
	{
		data[i] = Spi1_Recv_Send_Byte(0xFF);
	}
	
	F_CS = 1;
}

void W25q128_Demo()
{
	u32 addr = 0x010000;
	W25q128_EraseSector(addr);

	char data[] = "hello";
	W25q128_WriteSector(addr,data,5);
	
	char rxdata[6] = {0};
	W25q128_ReadData(addr,rxdata,5)	;
	
	printf("rxdata:%s\r\n",rxdata);
	
	while(1);
}