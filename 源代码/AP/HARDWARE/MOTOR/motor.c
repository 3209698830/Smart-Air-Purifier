#include "motor.h"

/*
驱动板A口
单片机引脚   接的模块   驱动板端
PB4 TIM3 CH1   JTRST     IN1
PB3 TIM2 CH2   JTDO      IN2
*/
void Motor_Init()
{
	GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;

/**************PB4 TIM3 CH1   JTRST     IN1****************/	
	//1.时钟使能 GPIO --定时器
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
 
	//2.GPIO初始化--复用功能
    GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_4;        //引脚4
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AF;        //复用模式
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_25MHz;
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP ;
    GPIO_Init(GPIOB, &GPIO_InitStructure); 
	
	//3.定时器初始化 
	TIM_TimeBaseStructure.TIM_Prescaler = (84-1); //定时器频率：1MHZ 1000 000 HZ
    TIM_TimeBaseStructure.TIM_Period     = (1000-1); //计1000个数，用时1ms  PWM周期
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //向上计数
    
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	//4.将GPIO 与 指定的硬件 连接在一起 
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_TIM3);
	
	//5.输出比较通道初始化
	TIM_OCInitStructure.TIM_OCMode         = TIM_OCMode_PWM1; //PWM1模式
    TIM_OCInitStructure.TIM_OCPolarity     = TIM_OCPolarity_High; //极性电平，这里设置为高电平
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;//输出通道使能
    TIM_OCInitStructure.TIM_Pulse = 0; //CCR的值

    TIM_OC1Init(TIM3, &TIM_OCInitStructure);

	//6.定时器的预装载使能
	TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable); 
		
	//7.定时器使能
	TIM_Cmd(TIM3, ENABLE);
	
/**************PB3 TIM2 CH2   JTDO      IN2****************/	
	//1.时钟使能 GPIO --定时器
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
 
	//2.GPIO初始化--复用功能
    GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_3;        //引脚3
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AF;        //复用模式
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_25MHz;
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP ;
    GPIO_Init(GPIOB, &GPIO_InitStructure); 
	
	//3.定时器初始化 
	TIM_TimeBaseStructure.TIM_Prescaler = (84-1); //定时器频率：1MHZ 1000 000 HZ
    TIM_TimeBaseStructure.TIM_Period     = (1000-1); //计1000个数，用时1ms  PWM周期
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //向上计数
    
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	//4.将GPIO 与 指定的硬件 连接在一起 
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_TIM2);
	
	//5.输出比较通道初始化
	TIM_OCInitStructure.TIM_OCMode         = TIM_OCMode_PWM1; //PWM1模式
    TIM_OCInitStructure.TIM_OCPolarity     = TIM_OCPolarity_High; //极性电平，这里设置为高电平
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;//输出通道使能
    TIM_OCInitStructure.TIM_Pulse = 0; //CCR的值

    TIM_OC2Init(TIM2, &TIM_OCInitStructure);

	//6.定时器的预装载使能
	TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable); 
		
	//7.定时器使能
	TIM_Cmd(TIM2, ENABLE);	
}

void Motor_Demo()
{
	uint32_t speed = (1000-1);
	while(1)
	{
		//正转
		TIM_SetCompare1(TIM3, speed);
		TIM_SetCompare2(TIM2, 0);
		delay_s(5);
		//调速
		speed-=300;
		TIM_SetCompare1(TIM3,speed );
		TIM_SetCompare2(TIM2, 0);
		delay_s(5);
		
		//反转
		speed = (1000-1);
		TIM_SetCompare1(TIM3, 0);
		TIM_SetCompare2(TIM2, speed);
		delay_s(5);
		
		//调速
		speed-=300;
		TIM_SetCompare1(TIM3, 0);
		TIM_SetCompare2(TIM2, speed);
		delay_s(5);
	}
}