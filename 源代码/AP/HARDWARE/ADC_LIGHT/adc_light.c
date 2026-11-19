#include "adc_light.h"

/*****************************************
引脚说明  光敏传感器- PF7  ADC3_IN5
*******************************************/
void Adc_Light_Init(void)
{
    //定义结构体变量
    GPIO_InitTypeDef        GPIO_InitStruct;
    ADC_CommonInitTypeDef   ADC_CommonInitStruct;
    ADC_InitTypeDef         ADC_InitStruct;

    
    //1.开启PA口时钟和ADC1时钟，设置为模拟输入。
    RCC_AHB1PeriphClockCmd (RCC_AHB1Periph_GPIOF, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);  
    
	//2.GPIO初始化 模拟模式
    GPIO_InitStruct.GPIO_Pin    	= GPIO_Pin_7;        //引脚
    GPIO_InitStruct.GPIO_Mode    	= GPIO_Mode_AN;        //模拟模式
    GPIO_InitStruct.GPIO_PuPd   	= GPIO_PuPd_NOPULL;     //浮空模式

    GPIO_Init(GPIOF, &GPIO_InitStruct);       
	
	// 3. 配置ADC公共参数（必须！）
    ADC_CommonInitStruct.ADC_Mode = ADC_Mode_Independent;         // 独立模式
    ADC_CommonInitStruct.ADC_Prescaler = ADC_Prescaler_Div4;      // 分频系数，确保ADC时钟≤36MHz（F4系列）
    ADC_CommonInitStruct.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled; // 禁止DMA访问
    ADC_CommonInitStruct.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles; // 采样间隔
    ADC_CommonInit(&ADC_CommonInitStruct);
	
	//4.ADC初始化
    ADC_InitStruct.ADC_Resolution        	= ADC_Resolution_12b; //12位分辨率
    ADC_InitStruct.ADC_DataAlign        	= ADC_DataAlign_Right;//数据右对齐
    ADC_InitStruct.ADC_ContinuousConvMode   = DISABLE; //关闭连续转换模式
	//ADC_InitStruct.ADC_ExternalTrigConv   =  //不使用触发模式，则认为使用软件触发
    ADC_InitStruct.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None; //软件启动触发
    ADC_InitStruct.ADC_NbrOfConversion    	= 1; //转换通道数目
    ADC_InitStruct.ADC_ScanConvMode        	= DISABLE;//关闭扫描选择
    
    ADC_Init(ADC3, &ADC_InitStruct);
    //5.使能ADC。
    ADC_Cmd(ADC3, ENABLE);
}

u16 Adc_Light_Get_Value(uint8_t ADC_Channel)
{
    u16 value;
    //指定转换的通道 ADC_Channel_5     1代表你要转换的这个通道的顺序-作为第一个转换
    ADC_RegularChannelConfig(ADC3, ADC_Channel, 1, ADC_SampleTime_56Cycles);
   
    //开启软件转换：
    ADC_SoftwareStartConv(ADC3);

    //等待转换结束
    while(ADC_GetFlagStatus(ADC3, ADC_FLAG_EOC) == RESET);
    
    
    value = ADC_GetConversionValue(ADC3);
    
    return value;
}

void Adc_Light_Demo()
{
	while(1){
		
		u16 value = Adc_Light_Get_Value(ADC_Channel_5);
		printf("value:%d vol:%.2fV\r\n",value,value*3.3 / 4095);
		
		delay_s(1);
	}
}
void Adc_Light_PWM_Demo()
{
	while(1){
		
		u16 value = Adc_Light_Get_Value(ADC_Channel_5);
		
		//0-999
		u16 light = value * 999 *1.0 / 4095;
		
		TIM_SetCompare1(TIM14, light);
		
		printf("value:%d vol:%.2fV light:%d\r\n",value,value*3.3 / 4095,light);
		
		delay_ms(200);
	}
}