#include "exti.h"
#include "rs485.h"
void Exti_Init()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;
	/*******************PA0********************************/	
	//1.时钟使能
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);	
	//2.GPIO初始化
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	//3.外部中断事件线 初始化EXTI
	EXTI_InitStruct.EXTI_Line	 = EXTI_Line0;  //中断线
	EXTI_InitStruct.EXTI_Mode	 = EXTI_Mode_Interrupt; //中断模式
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling; //下降沿
	EXTI_InitStruct.EXTI_LineCmd = ENABLE; //中断线使能
	EXTI_Init(&EXTI_InitStruct);
	
	//4.GPIO与EXTI连接在一起
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA,EXTI_PinSource0);
	
	//5.NVIC初始化
	NVIC_InitStruct.NVIC_IRQChannel = EXTI0_IRQn;//中断源-中断的编号/通道号 stm32f4xx.h
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;//抢占优先级 0-3
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;//响应优先级 0-3
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;//中断源使能
	NVIC_Init(&NVIC_InitStruct);
	
	/*******************PE234********************************/
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	
	//2.GPIO初始化
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	//3.外部中断事件线 初始化EXTI
	EXTI_InitStruct.EXTI_Line	 = EXTI_Line2|EXTI_Line3|EXTI_Line4;  //中断线
	EXTI_InitStruct.EXTI_Mode	 = EXTI_Mode_Interrupt; //中断模式
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling; //下降沿
	EXTI_InitStruct.EXTI_LineCmd = ENABLE; //中断线使能
	EXTI_Init(&EXTI_InitStruct);
	
	//4.GPIO与EXTI连接在一起
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE,EXTI_PinSource2);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE,EXTI_PinSource3);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE,EXTI_PinSource4);
	
	//5.NVIC初始化
	NVIC_InitStruct.NVIC_IRQChannel = EXTI2_IRQn;//中断源-中断的编号/通道号 stm32f4xx.h
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;//抢占优先级 0-3
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;//响应优先级 0-3
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;//中断源使能
	NVIC_Init(&NVIC_InitStruct);
	
	NVIC_InitStruct.NVIC_IRQChannel = EXTI2_IRQn;
	NVIC_Init(&NVIC_InitStruct);
	
	NVIC_InitStruct.NVIC_IRQChannel = EXTI3_IRQn;
	NVIC_Init(&NVIC_InitStruct);
	
	NVIC_InitStruct.NVIC_IRQChannel = EXTI4_IRQn;
	NVIC_Init(&NVIC_InitStruct);
}

//6.编写中断服务函数  --xxx.s启动文件中寻找
void EXTI0_IRQHandler()
{
	static int cnt=0;
	char data[10] = {0};
	
	//先判断中断标志位是否为1
	if(EXTI_GetITStatus(EXTI_Line0) == SET)
	{
		delay_ms(15);
		//清除中断标志位
		EXTI_ClearITPendingBit(EXTI_Line0);
		//业务处理
		GPIO_ToggleBits(GPIOF, GPIO_Pin_9);
				
		memset(data,0,sizeof(data));
		sprintf(data,"cnt:%d",cnt++);
		printf("send data:%s\r\n",data);
		RS485_SendStr(data,strlen(data));
	}
}
//6.编写中断服务函数  --xxx.s启动文件中寻找
void EXTI2_IRQHandler()
{
	//先判断中断标志位是否为1
	if(EXTI_GetITStatus(EXTI_Line2) == SET)
	{
		delay_ms(15);
		
		//清除中断标志位
		EXTI_ClearITPendingBit(EXTI_Line2);
		//业务处理
		GPIO_ToggleBits(GPIOF, GPIO_Pin_10);
	}
}

void EXTI3_IRQHandler()
{
	//先判断中断标志位是否为1
	if(EXTI_GetITStatus(EXTI_Line3) == SET)
	{
		delay_ms(15);
		
		//清除中断标志位
		EXTI_ClearITPendingBit(EXTI_Line3);
		//业务处理
		GPIO_ToggleBits(GPIOE, GPIO_Pin_13);
	}
}

void EXTI4_IRQHandler()
{
	//先判断中断标志位是否为1
	if(EXTI_GetITStatus(EXTI_Line4) == SET)
	{
		delay_ms(15);
		
		//清除中断标志位
		EXTI_ClearITPendingBit(EXTI_Line4);
		//业务处理
		GPIO_ToggleBits(GPIOE, GPIO_Pin_14);
	}
}