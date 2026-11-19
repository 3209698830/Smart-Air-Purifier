#include "tim6.h"

void Tim6_Init()
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;
	
	//1.时钟使能  42M * 2  = 84M
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
	
	//2.定时器初始化
	TIM_TimeBaseStructInit(&TIM_TimeBaseInitStruct);
	
	TIM_TimeBaseInitStruct.TIM_Prescaler = 8400-1;//预分频值  10000HZ 
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;//向上计数
	TIM_TimeBaseInitStruct.TIM_Period = 10000-1;// 重装载值 
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;//时钟分频因子

	TIM_TimeBaseInit(TIM6, &TIM_TimeBaseInitStruct);
	
	//3.NVIC初始化
	NVIC_InitStruct.NVIC_IRQChannel = TIM6_DAC_IRQn;//中断源-中断的编号/通道号 stm32f4xx.h
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;//抢占优先级 0-3
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;//响应优先级 0-3
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;//中断源使能
	NVIC_Init(&NVIC_InitStruct);
	
	//4.定时器的更新中断使能
	TIM_ITConfig(TIM6, TIM_IT_Update,ENABLE);
	
	//5.定时器使能--计数器使能
	TIM_Cmd(TIM6,ENABLE);
	
	
}
//6.编写中断服务函数
void TIM6_DAC_IRQHandler()
{
	//判断中断标志位置1
	if(TIM_GetITStatus(TIM6,TIM_IT_Update) == SET)
	{
		//清空中断标志位
		TIM_ClearITPendingBit(TIM6,TIM_IT_Update);
		
		GPIO_ToggleBits(GPIOF, GPIO_Pin_9);
	}
}