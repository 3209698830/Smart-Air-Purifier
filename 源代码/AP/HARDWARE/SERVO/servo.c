#include "servo.h"

/*
舵机 TXD2 USART2_TX  PA2  TIM2_CH3 
*/
void Servo_Init()
{
	GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;
    	
	//1.时钟使能 GPIO --定时器
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
 
	//2.GPIO初始化--复用功能
    GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_2;        //引脚
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AF;        //复用模式
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_25MHz;
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP ;
    GPIO_Init(GPIOA, &GPIO_InitStructure); 
	
	//3.定时器初始化 
	TIM_TimeBaseStructure.TIM_Prescaler = (84-1); //定时器频率：1MHZ 1000 000 HZ
    TIM_TimeBaseStructure.TIM_Period     = (20000-1); //计20000个数，用时20ms  PWM周期
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //向上计数
    
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	//4.将GPIO 与 指定的硬件 连接在一起 
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_TIM2);
	
	//5.输出比较通道初始化
	TIM_OCInitStructure.TIM_OCMode         = TIM_OCMode_PWM1; //PWM1模式
    TIM_OCInitStructure.TIM_OCPolarity     = TIM_OCPolarity_High; //极性电平，这里设置为高电平
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;//输出通道使能
    TIM_OCInitStructure.TIM_Pulse = 0; //CCR的值

    TIM_OC3Init(TIM2, &TIM_OCInitStructure);

	//6.定时器的预装载使能
	TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable); 
		
	//7.定时器使能
	TIM_Cmd(TIM2, ENABLE);
}
/*
时间    角度      计数
20ms             20000
0.5ms   0度      500 
1.0ms   45度     1000 
1.5ms   90度     1500
2.0ms   135度    2000
2.5ms   180度    2500 
*/
void Servo_Angle(ServoAngle angle) //angle角度参数
{
	switch(angle)
	{
		case Angle_0:
					TIM_SetCompare3(TIM2, 500);
					break;
		case Angle_45:
					TIM_SetCompare3(TIM2, 1000);
					break;
		case Angle_90:
					TIM_SetCompare3(TIM2, 1500);
					break;
		case Angle_135:
					TIM_SetCompare3(TIM2, 2000);
					break;
		case Angle_180:	
					TIM_SetCompare3(TIM2, 2500);
					break;
	}
}

void Servo_Demo()
{
	while(1)
	{		
		Servo_Angle(Angle_0);
		delay_s(2);
		Servo_Angle(Angle_45);
		delay_s(2);
		Servo_Angle(Angle_90);
		delay_s(2);
		Servo_Angle(Angle_135);
		delay_s(2);	
		Servo_Angle(Angle_180);
		delay_s(2);			
	}
}