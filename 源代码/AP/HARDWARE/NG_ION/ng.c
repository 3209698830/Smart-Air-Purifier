#include "ng.h"

/*
负离子发生器
引脚  
	PC7
	PC8
    推挽输出，默认下拉电平（输出低电平）
*/
void NG_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // 开启GPIOC时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_7 | GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;        //普通输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;        //推挽输出
    GPIO_InitStructure.GPIO_Speed  = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd   = GPIO_PuPd_DOWN;       //下拉

    GPIO_Init(GPIOC, &GPIO_InitStructure);

    //初始化默认输出低电平
    GPIO_ResetBits(GPIOC, GPIO_Pin_7);
    GPIO_ResetBits(GPIOC, GPIO_Pin_8);
}
//控制开关
void Control_NG(uint8_t x)//开：x=1   关：x=0
{
	if(x)
    GPIO_SetBits(GPIOC, GPIO_Pin_7);
	else
	GPIO_ResetBits(GPIOC, GPIO_Pin_7);
}

//切换引脚电平
void Switch_status_NG(void){

	GPIO_ToggleBits(GPIOC, GPIO_Pin_7);
}

//读取工作状态  正在运行：返回1   关闭状态：返回0
uint8_t Read_stat_NG(void){

	if(GPIO_ReadOutputDataBit(GPIOC, GPIO_Pin_7) == 1)
	{
		return 1;
	}
	else
	{
		return 0;
	}

}
