#include "rs485.h"

/*
USART2  TX  PA2
USART2  RX  PA3
*/
void RS485_Init(int baud)
{
	GPIO_InitTypeDef GPIO_InitStruct;
    USART_InitTypeDef USART_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;
    
    //1.时钟使能
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    
    //2.GPIO初始化
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3;//引脚
    GPIO_InitStruct.GPIO_Mode= GPIO_Mode_AF;//模式  复用
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;//推挽
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;//上拉
    GPIO_InitStruct.GPIO_Speed = GPIO_Medium_Speed;//速度
    
    GPIO_Init(GPIOA,&GPIO_InitStruct);
    
    //3.UART初始化
    USART_InitStruct.USART_BaudRate = baud; //波特率
    USART_InitStruct.USART_WordLength = USART_WordLength_8b; //字长
    USART_InitStruct.USART_StopBits = USART_StopBits_1; //停止位
    USART_InitStruct.USART_Parity = USART_Parity_No ; // 无校验位
    USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; //能发送能接收
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;   //无硬件流
    USART_Init(USART2,&USART_InitStruct);
    
    //4.UART与GPIO连接在一起--映射
    GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_USART2);
    GPIO_PinAFConfig(GPIOA,GPIO_PinSource3,GPIO_AF_USART2);
    
    //5.配置NVIC--接收中断
    NVIC_InitStruct.NVIC_IRQChannel = USART2_IRQn ;//中断源 中断通道 stm32f4xx.h
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;//使能
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;//抢占优先级
    NVIC_InitStruct.NVIC_IRQChannelSubPriority  = 1;//响应优先级
    NVIC_Init(&NVIC_InitStruct);
    
    //6.接收中断使能
    USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);
	//空闲中断使能IDLE
	//USART_ITConfig(USART2,USART_IT_IDLE,ENABLE);
	
    //7.UART使能
    USART_Cmd(USART2,ENABLE);
	
	//PG8初始化

	//1.GPIOF组时钟使能
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
	//2.GPIO配置
	GPIO_InitStruct.GPIO_Pin   = GPIO_Pin_8;//引脚
	GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_OUT;//输出模式
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;//推挽
	GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_UP;//上拉
	GPIO_InitStruct.GPIO_Speed = GPIO_Medium_Speed;//速度
	GPIO_Init(GPIOG,&GPIO_InitStruct);
	
	RS485_RE = 0;
}
//8.编写串口接收中断服务函数
void USART2_IRQHandler()
{
    //1.判断接收中断标志位是否触发
    if(USART_GetITStatus(USART2, USART_IT_RXNE) == SET)
    {
        //接收数据
        uint16_t data = USART_ReceiveData(USART2);
		
        //打印到串口1
        printf("%c",data);
        
        USART_ClearITPendingBit(USART2,USART_IT_RXNE);//清空接收中断标志位
    } 
	
}

void RS485_SendStr(char *str,u16 len)
{
	RS485_RE = 1;
	
	for(int i=0; i<len; i++)
	{
		//发送缓冲区为空
		while(!USART_GetFlagStatus(USART2, USART_FLAG_TXE));
		USART_SendData(USART2,str[i]);
	}
	
	delay_s(1); //等待 RS485 将TTL电平数据全部转成485电平 发出去
	
	RS485_RE = 0;
}

void RS485_Demo(){
	
	
	while(1){
				
		delay_s(5);
	}
	
}