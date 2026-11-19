#include "beep_audio.h"
//#include "led.h"
#include "delay.h"

 //PF8  TIM13_CH1
void BeepAudio_Init(void)
{
    /* 开启时钟*/
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM13,ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF,ENABLE);
    
    /* GPIO初始化*/
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init( GPIOF, &GPIO_InitStruct);
    
    /* GPIO复用*/
    GPIO_PinAFConfig(GPIOF, GPIO_PinSource8, GPIO_AF_TIM13);
    
    /* 配置定时器*/ //10khz
    //Fpwm = 主频 / (（ARR+1）*（PSC+1）)
    
    //523 =  84000000 / 84 / x
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_TimeBaseInitStruct.TIM_Period = 84-1;
    TIM_TimeBaseInitStruct.TIM_Prescaler = 1-1;  
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM13, &TIM_TimeBaseInitStruct);
    
    /* OC配置*/
    //占空比 = CCR/ARR
    TIM_OCInitTypeDef TIM_OCInitStruct;
    TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStruct.TIM_Pulse = 40; //CCR
    TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC1Init(TIM13, &TIM_OCInitStruct);
    
    TIM_Cmd(TIM13, ENABLE);
}

void Set_Prescaler(u16 FNote)
{
    
    // 523(FNote) = 84000000 / x / 84
    // 523(FNote) = 1000000 / x
    TIM_PrescalerConfig(TIM13, (1000000/FNote)-1, TIM_PSCReloadMode_Update);
}

/* 中音部分 1~7 */
u16 MusicNote_Mid[12] = {
  /*
     1   2   3   4   5   6   7
     do  re  mi  fa  so  la  si
  */
    523,587,659,698,784,880,988, 
};

//"12311231345-345-5.65.4315.65.431151-"
void Music_Play(char *buf)
{
    for(u16 i=0; buf[i]!='\0'; i++)
    {
        /* 遇到横杠，延续上一个音符，总共延时1000ms*/
        if(buf[i] == '-')
        {
            delay_ms(500);
            continue;
        }
        
        /* 遇到下一个字符为小数点，则当前音符播放150ms*/
        if(buf[i+1] == '.')
        {
            Set_Prescaler(MusicNote_Mid[buf[i]-'0'-1]);
            delay_ms(150);
            continue;
        }
        
        /* 遇到当前为小数点，则提前播放下一个音符播放350ms*/
        if(buf[i] == '.')
        {
            Set_Prescaler(MusicNote_Mid[buf[i+1]-'0'-1]);
            i++;
            delay_ms(350);
            continue;
        }
        /* 正常音符播放*/
        Set_Prescaler(MusicNote_Mid[buf[i]-'0'-1]);
        delay_ms(500);
    }
}

void BeepAudio_Demo()
{
	char TowTiger_buf[ ]= "12311231345-345-5.65.4315.65.431151-"; //两只老虎
	/* 方式二*/
	while(1){
		Music_Play(TowTiger_buf);
	}
	
}
