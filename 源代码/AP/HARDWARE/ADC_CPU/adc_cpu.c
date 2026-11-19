#include "adc_cpu.h"

/*****************************************
引脚说明  ADC1 IN16
*******************************************/
void Adc_Cpu_Init(void)
{
    //定义结构体变量  
    ADC_CommonInitTypeDef   ADC_CommonInitStruct;
    ADC_InitTypeDef         ADC_InitStruct;
   
    //1.开启ADC1时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);  
       
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
    
    ADC_Init(ADC1, &ADC_InitStruct);
    //5.使能ADC。
    ADC_Cmd(ADC1, ENABLE);
	
	//将温度传感器从掉电模式中唤醒
	ADC_TempSensorVrefintCmd(ENABLE);
}

u16 Adc_Cpu_Value(uint8_t ADC_Channel)
{
    u16 value;
    //指定转换的通道 ADC_Channel_16     1代表你要转换的这个通道的顺序-作为第一个转换
    ADC_RegularChannelConfig(ADC1, ADC_Channel, 1, ADC_SampleTime_480Cycles);
   
    //开启软件转换：
    ADC_SoftwareStartConv(ADC1);

    //等待转换结束
    while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
    
    
    value = ADC_GetConversionValue(ADC1);
    
    return value;
}

void Adc_Cpu_Demo()
{
	while(1){
		
		u16 value = Adc_Cpu_Value(ADC_Channel_16);
		
		float v = value *3.3 / 4095 ;
		//通过公式转换为温度
		float temp = (v - 0.76) / 2.5 + 25;
		
		printf("value:%d temp:%.2f\r\n",value,temp);
		
		delay_s(1);
	}
}
