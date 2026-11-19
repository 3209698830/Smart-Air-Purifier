#include "fire.h"
#include "beep.h"
/*
VCC <--> 3.3V   
GND <--> GND  
DO <--> PB7  
*/
void Fire_Init()
{
    GPIO_InitTypeDef GPIO_InitStruct;
    
    /*开启时钟*/
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    
#ifdef FIRE_BLUE    
    /*GPIO初始化*/
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;  
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;  
    GPIO_Init( GPIOB ,&GPIO_InitStruct);
    
     /*外部中断配置*/
    EXTI_InitTypeDef EXTI_InitStruct;
    EXTI_InitStruct.EXTI_Line = EXTI_Line7;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStruct);
#else
    /*GPIO初始化*/
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;  
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;  
    GPIO_Init( GPIOB ,&GPIO_InitStruct);
    
    /*外部中断配置*/
    EXTI_InitTypeDef EXTI_InitStruct;
    EXTI_InitStruct.EXTI_Line = EXTI_Line7;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStruct);
#endif
  
    /*连接 EXTI 中断源到 PB7引脚*/
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource7);
 
    /*配置NVIC*/
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = EXTI9_5_IRQn; //中断源的选择 stm32f4xx.h
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}
#if 0
void EXTI9_5_IRQHandler()
{
	//1.先判断中断标志位是否触发
    if(EXTI_GetITStatus(EXTI_Line7) == SET)
    {
        BEEP(1);
        GPIO_ToggleBits(GPIOF, GPIO_Pin_10);
        EXTI_ClearITPendingBit(EXTI_Line7);//清除中断标志位
    }
}
#endif