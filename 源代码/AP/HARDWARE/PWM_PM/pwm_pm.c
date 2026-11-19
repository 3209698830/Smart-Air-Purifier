#include "pwm_pm.h"

/*
 引脚 PB7 复用,浮空  		  TIM4_CH2  默认高电平
      PD6 推挽 下拉  
*/

void Pwm_Pm_Init(void){

		GPIO_InitTypeDef GPIO_InitStructure;
		GPIO_InitTypeDef GPIO_PD6_InitStructure;
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;
	
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	//2.GPIO初始化--PD6 
    GPIO_PD6_InitStructure.GPIO_Pin		= GPIO_Pin_6;        //引脚6
    GPIO_PD6_InitStructure.GPIO_Mode    = GPIO_Mode_OUT;        //输出
    GPIO_PD6_InitStructure.GPIO_Speed   = GPIO_Speed_25MHz;
    GPIO_PD6_InitStructure.GPIO_OType   = GPIO_OType_PP;
    GPIO_PD6_InitStructure.GPIO_PuPd    = GPIO_PuPd_DOWN ;
    GPIO_Init(GPIOD, &GPIO_PD6_InitStructure); 
	
	

/**************PB7 TIM4 CH2 ****************/	
	//1.时钟使能 GPIO --定时器
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
 
	//2.GPIO初始化--复用功能
    GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_7;        //引脚4
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AF;        //复用模式
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_25MHz;
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_NOPULL ;
    GPIO_Init(GPIOB, &GPIO_InitStructure); 
	
	//3.定时器初始化 
		TIM_TimeBaseStructure.TIM_Prescaler = (84-1); //定时器频率：1MHZ 1000 000 HZ
    TIM_TimeBaseStructure.TIM_Period     = (10000-1); //计1000个数，用时1ms  PWM周期
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //向上计数
    
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

	//4.将GPIO 与 指定的硬件 连接在一起 
		GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_TIM4);
	
	//5.输出比较通道初始化
		TIM_OCInitStructure.TIM_OCMode         = TIM_OCMode_PWM1; //PWM1模式
    TIM_OCInitStructure.TIM_OCPolarity     = TIM_OCPolarity_High; //极性电平，这里设置为高电平
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;//输出通道使能
    TIM_OCInitStructure.TIM_Pulse = 0; //CCR的值

    TIM_OC2Init(TIM4, &TIM_OCInitStructure);

	//6.定时器的预装载使能
		TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable); 
		
	//7.定时器使能
		TIM_Cmd(TIM4, ENABLE);
		
		DWT_Init();//"延时功能初始化"
}

void Pwm_Pm_Demo(void){

		int speed = (10000-1);
		//GPIO_ResetBits(GPIOD,GPIO_Pin_6);
	
		while(1)
		{
		//调速
		speed-=1000;
		speed =(speed < 0) ? (10000-1):speed;
		
		TIM_SetCompare2(TIM4,speed);
		DWT_Delay_ms(500);
		}
	

}


