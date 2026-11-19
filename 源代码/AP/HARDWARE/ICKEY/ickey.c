#include "ickey.h"
//#include "led.h"


uint16_t Key_Value = 0;

static void delay(uint32_t ms)
{
    volatile uint32_t count, i;
    
    while(ms--) {
        // 内层循环产生约1ms延时
        for(count = 42000; count > 0; count--);
    }
}

//获取按键PA0低电平的持续时间  TIM5 CH1
void ICKey_TIM5_CH1_Init()
{
    /* 开启时钟 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
    
    /*GPIO初始化*/
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 ;  
    GPIO_Init( GPIOA, &GPIO_InitStruct);
    
    /* 将引脚连接上定时器5 */
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_TIM5);

    /* 定时器基础配置 */
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_TimeBaseStructure.TIM_Period = 0xFFFF;       
    TIM_TimeBaseStructure.TIM_Prescaler = 8400-1;    //84000 000 / 8400 = 10000hz 
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);
    
    /*通道一配置 input capture*/
    TIM_ICInitTypeDef  TIM_ICInitStruct;
    TIM_ICInitStruct.TIM_Channel = TIM_Channel_1; //通道1
    TIM_ICInitStruct.TIM_ICFilter = 0xF; //输入滤波器 设置为最大值 按键有抖动 简单滤波消抖
    TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Falling; //边沿检测器--下降沿触发
    TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1; //不分频
    TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;//直连输入 输入通道1 ---捕获通道1
    TIM_ICInit(TIM5, &TIM_ICInitStruct);
    
    /*通道二配置*/
    TIM_ICInitStruct.TIM_Channel = TIM_Channel_2; //通道2
    TIM_ICInitStruct.TIM_ICFilter = 0xF;
    TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Rising; //上升沿
    TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_IndirectTI; //次连--输入通道1 ---捕获通道2
    TIM_ICInit(TIM5, &TIM_ICInitStruct);
   
    /*使能通道*/
    TIM_CCxCmd(TIM5,TIM_Channel_1,TIM_CCx_Enable);
    TIM_CCxCmd(TIM5,TIM_Channel_2,TIM_CCx_Enable);
    
    /* 开启通道中断*/
    TIM_ITConfig(TIM5,TIM_IT_CC1, ENABLE);
    TIM_ITConfig(TIM5,TIM_IT_CC2, ENABLE);
    
    /* 配置NVIC */
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* 开启定时器 */
    TIM_Cmd(TIM5, ENABLE);
}

void TIM5_IRQHandler(void)
{
    //TIM_IT_CC1 按键按下时  下降沿触发  会触发通道1
    if(TIM_GetITStatus(TIM5, TIM_IT_CC1) == SET)
    {
        delay(50); //延时消抖
        if(KEY0 == 0)
        {
            /*计数值清0*/
            TIM5->CNT = 0;
                    
            /*按下灯点亮*/
            //LED1(0);                        
        }           
        /* 没有读取标志位，需手动清除*/
        TIM_ClearITPendingBit(TIM5, TIM_IT_CC1);
    }
    //TIM_IT_CC2 按键松开  上升沿触发  通道2
    if(TIM_GetITStatus(TIM5, TIM_IT_CC2) == SET)
    {
        /* 读取通道二寄存器的数值，读取同时会清除标志位*/
        Key_Value = TIM_GetCapture2(TIM5);
        
        /*抬手熄灭*/
        //LED1(1);
        
        //TIM_ClearITPendingBit(TIM5, TIM_IT_CC2);
    }
}
