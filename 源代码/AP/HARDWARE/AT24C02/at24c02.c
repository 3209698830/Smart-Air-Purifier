#include "at24c02.h"
//#include "iic.h"
#if 0
void AT24C02_Init()
{
	Iic_Init();
}

void AT24C02_PageWrite(u32 address,char*data,int len)
{
	//起始信号
	Iic_Start();
	
	//             固定A2A1A0读写位  0写  1读
	//发送设备地址 1010 0000
	Iic_Send_Byte(0xA0);
	//接收应答
	if(Iic_Recv_Ack() != 0){
		printf("write address error\r\n");
		return ;
	}
	
	//发送字地址，你要写入数据的这一块内存地址
	Iic_Send_Byte(address);
	if(Iic_Recv_Ack() != 0){
		printf("write word address error\r\n");
		return ;
	}
	for(int i=0; i<len; i++){
		//发送数据
		Iic_Send_Byte(data[i]);
		if(Iic_Recv_Ack() != 0){
			printf("write data error\r\n");
			return ;
		}
	}
	Iic_Stop();
}
void AT24C02_RandomRead(u32 address,char*buff,int len)
{
	/*第一步，先通过一个“伪写”命令（发送设备地址和字地址，但不发送数据）来设置内部地址计数器。***************/
	//起始信号
	Iic_Start();
	//发送设备地址 1010 0000
	Iic_Send_Byte(0xA0);
	//接收应答
	if(Iic_Recv_Ack() != 0){
		printf("write address error\r\n");
		return ;
	}
	//发送字地址，你要写入数据的这一块内存地址
	Iic_Send_Byte(address);
	if(Iic_Recv_Ack() != 0){
		printf("write word address error\r\n");
		return ;
	}
	/*第二步，执行当前地址读命令来读取目标地址的数据。*/
	//起始信号
	Iic_Start();
	//发送设备地址 1010 0001
	Iic_Send_Byte(0xA1);
	//接收应答
	if(Iic_Recv_Ack() != 0){
		printf("write address error\r\n");
		return ;
	}
	for(int i=0; i<len; i++)
	{
		buff[i] = Iic_Recv_Byte();
		if(i== len-1){
			//最后一个字节
			Iic_Send_Ack(1); //NO ACK
		}else{
			Iic_Send_Ack(0); //ACK
		}
	}
	Iic_Stop();
}
void AT24C02_Demo()
{
	//写数据
//	char data[] = "hello";
//	AT24C02_PageWrite(0x08,data,5);
	
//	delay_s(1);
//	
	char buff[6]= {0};
	AT24C02_RandomRead(0x08,buff,5);
	
	printf("buff:%s\r\n",buff);
	
	while(1);
}
#endif