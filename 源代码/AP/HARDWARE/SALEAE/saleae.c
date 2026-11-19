#include "saleae.h"
 
/*
CAMERA 17号引脚 产生PWM波形   PA8  TIM1_CH1
*/
void Saleae_Pwm_Init()
{
	GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;
    	
	//1.时钟使能 GPIO --定时器
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
 
	//2.GPIO初始化--复用功能
    GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_8;        //引脚
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AF;        //复用模式
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_25MHz;
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP ;
    GPIO_Init(GPIOA, &GPIO_InitStructure); 
	
	//3.定时器初始化 
	TIM_TimeBaseStructure.TIM_Prescaler = (168-1); //定时器频率：1MHZ 1000 000 HZ
    TIM_TimeBaseStructure.TIM_Period     = (1000-1); //计1000个数，用时1ms  PWM周期
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //向上计数
	
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

	//4.将GPIO 与 指定的硬件 连接在一起 
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_TIM1);
	
	//5.输出比较通道初始化
	TIM_OCStructInit(&TIM_OCInitStructure);
	
	TIM_OCInitStructure.TIM_OCMode         = TIM_OCMode_PWM1; //PWM1模式
    TIM_OCInitStructure.TIM_OCPolarity     = TIM_OCPolarity_High; //极性电平，这里设置为高电平
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;//输出通道使能
    TIM_OCInitStructure.TIM_Pulse = 0; //CCR的值
	// 在 TIM_OCInitStructure 中添加（示例值）
//	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;  // 非空闲时输出有效电平
//	TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;
    TIM_OC1Init(TIM1, &TIM_OCInitStructure);

	//6.定时器的预装载使能
	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable); 
		
	//高级定时器的PWM使能	
	TIM_CtrlPWMOutputs(TIM1, ENABLE);	
	//7.定时器使能
	TIM_Cmd(TIM1, ENABLE);
}

void Saleae_Pwm_Demo()
{
	uint32_t Compare1 = 800;
	TIM_SetCompare1(TIM1, Compare1);
	
	while(1)
	{
		//Compare1 = (Compare1 >= 1000 )? 0 : (Compare1+100);
		//TIM_SetCompare1(TIM1, Compare1);
		//delay_ms(100);
	}
}