#include "spi.h"

/*
PB5		SPI1_MOSI
PB4		SPI1_MISO
PB3		SPI1_SCK
PB14	F_CS    
*/
void Spi1_Init()
{
	GPIO_InitTypeDef     GPIO_InitStructure;
    SPI_InitTypeDef 	SPI_InitStruct;
	
	//1.时钟使能
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE); 
	
	//2.GPIO初始化
    GPIO_InitStructure.GPIO_Pin     	= GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5;        //引脚3 4 5
    GPIO_InitStructure.GPIO_Mode     	= GPIO_Mode_AF;        //复用模式
    GPIO_InitStructure.GPIO_Speed     	= GPIO_Speed_25MHz;
    GPIO_InitStructure.GPIO_OType     	= GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd     	= GPIO_PuPd_UP ;
    GPIO_Init(GPIOB, &GPIO_InitStructure); 
    
    GPIO_InitStructure.GPIO_Pin    		= GPIO_Pin_14;        //引脚14
    GPIO_InitStructure.GPIO_Mode    	= GPIO_Mode_OUT;    //输出模式
    GPIO_InitStructure.GPIO_OType      	= GPIO_OType_PP;     //推挽输出
    GPIO_InitStructure.GPIO_PuPd       	= GPIO_PuPd_UP;     //上拉
    GPIO_InitStructure.GPIO_Speed      	= GPIO_Speed_25MHz; //25MHZ
    GPIO_Init(GPIOB, &GPIO_InitStructure);    
	
	//3.SPI初始化  84000 000 
	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;//时钟分频因子
	SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;//时钟极性
	SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;//时钟相位  模式0
	SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;//传输数据量 8
	SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;//双线全双工
	SPI_InitStruct.SPI_Mode = SPI_Mode_Master;//主机模式
	SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;//MSB
	SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;//片选引脚 软件触发
	SPI_InitStruct.SPI_CRCPolynomial = 7;//CRC校验
	SPI_Init(SPI1,&SPI_InitStruct);
	
	//4.GPIO与SPI连接在一起
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_SPI1);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_SPI1);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI1);
    
    	
	//5.SPI的中断使能
	
	//6.SPI使能
    SPI_Cmd(SPI1, ENABLE);
	
	//7.编写SPI的中断服务函数
	
	F_CS = 1;
}

u8 Spi1_Recv_Send_Byte(u8 txdata)
{
    u8 rxdata = 0x00;
    //查看发送缓冲区是否 为空
    while( SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE)  == RESET);
    //发送数据
    SPI_I2S_SendData(SPI1, txdata);
    
    while( SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
    //接受数据
    rxdata = SPI_I2S_ReceiveData(SPI1);
    
    return rxdata;
}
