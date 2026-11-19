#include "pwm.h"
 
/*
PF9  TIM14_CH1
*/
void Pwm_Init()
{
	GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;
    	
	//1.时钟使能 GPIO --定时器
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
 
	//2.GPIO初始化--复用功能
    GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_9;        //引脚9
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AF;        //复用模式
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_25MHz;
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP ;
    GPIO_Init(GPIOF, &GPIO_InitStructure); 
	
	//3.定时器初始化 
	TIM_TimeBaseStructure.TIM_Prescaler = (84-1); //定时器频率：1MHZ 1000 000 HZ
    TIM_TimeBaseStructure.TIM_Period     = (1000-1); //计1000个数，用时1ms  PWM周期
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //向上计数
    
    TIM_TimeBaseInit(TIM14, &TIM_TimeBaseStructure);

	//4.将GPIO 与 指定的硬件 连接在一起 
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource9, GPIO_AF_TIM14);
	
	//5.输出比较通道初始化
	TIM_OCInitStructure.TIM_OCMode         = TIM_OCMode_PWM1; //PWM1模式
    TIM_OCInitStructure.TIM_OCPolarity     = TIM_OCPolarity_Low; //极性电平，这里设置为低电平
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;//输出通道使能
    TIM_OCInitStructure.TIM_Pulse = 0; //CCR的值

    TIM_OC1Init(TIM14, &TIM_OCInitStructure);

	//6.定时器的预装载使能
	TIM_OC1PreloadConfig(TIM14, TIM_OCPreload_Enable); 
		
	//7.定时器使能
	TIM_Cmd(TIM14, ENABLE);
}

void Pwm_Demo()
{
	uint32_t Compare1 = 0;
	while(1)
	{
		Compare1 = (Compare1 >= 1000 )? 0 : (Compare1+100);
		TIM_SetCompare1(TIM14, Compare1);
		delay_ms(100);
	}
}