#include "iwdg.h"

void Iwdg_Init(void)
{
    //1.使能 看门狗预分频寄存器和重装载值寄存器的写访问
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);   
    //2.设置独立看门狗的预分频系数，确定时钟:250HZ
    IWDG_SetPrescaler(IWDG_Prescaler_128);
    //3.设置看门狗重装载值，确定溢出时间:2S
    IWDG_SetReload(500);
    //4.使能看门狗
    IWDG_Enable();
    //5.应用程序喂狗
    IWDG_ReloadCounter();
}

void Iwdg_Demo()
{
	printf("start\r\n");
	int cnt=0;
	
	while(1){
		printf("cnt:%d\r\n",cnt++);
		IWDG_ReloadCounter();//喂狗
		delay_s(1);
	}	
}