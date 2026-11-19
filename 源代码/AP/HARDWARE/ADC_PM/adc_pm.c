#include "adc_pm.h"

/*
 * PM2.5 sensor ADC (sensor OUT -> PA4 / ADC1_CH4)
 *
 * RTOS-safe ADC timing rules:
 *  1. ADC1 is enabled ONCE here (ADC_Cmd). Never toggle ADON per read:
 *     after each re-enable the ADC needs t_STAB (~3us) and the first
 *     conversion after enable is unreliable -> low/wrong readings.
 *  2. Get_ADC_PM_Value() is called INSIDE the 0.32ms LED pulse window
 *     (sample point = 0.28ms after LED-on), so it must stay fast:
 *     NO printf / heavy code allowed inside this function!
 *     (At 9600 baud one printf char blocks ~104us and destroys the pulse.)
 
 */

void ADC_PM_Init(void)
{
    GPIO_InitTypeDef        GPIO_InitStruct;
    ADC_CommonInitTypeDef   ADC_CommonInitStruct;
    ADC_InitTypeDef         ADC_InitStruct;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

    /* PA4 analog mode */
    GPIO_InitStruct.GPIO_Pin     = GPIO_Pin_4;
    GPIO_InitStruct.GPIO_Mode    = GPIO_Mode_AN;
    GPIO_InitStruct.GPIO_PuPd    = GPIO_PuPd_NOPULL;
		//GPIO_InitStruct.GPIO_Speed = GPIO_Low_Speed;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* ADCCLK = PCLK2/4 = 84MHz/4 = 21MHz (max 36MHz) */
    ADC_CommonInitStruct.ADC_Mode              = ADC_Mode_Independent;
    ADC_CommonInitStruct.ADC_Prescaler         = ADC_Prescaler_Div4;
    ADC_CommonInitStruct.ADC_DMAAccessMode     = ADC_DMAAccessMode_Disabled;
    ADC_CommonInit(&ADC_CommonInitStruct);

    ADC_InitStruct.ADC_DataAlign               = ADC_DataAlign_Right;
    ADC_InitStruct.ADC_ContinuousConvMode      = DISABLE;
    ADC_InitStruct.ADC_ExternalTrigConvEdge    = ADC_ExternalTrigConvEdge_None;
    ADC_InitStruct.ADC_NbrOfConversion         = 1;
    ADC_InitStruct.ADC_ScanConvMode            = DISABLE;
    ADC_Init(ADC1, &ADC_InitStruct);

    /* fixed ch4 (PA4): sample 480 cyc -> total conv ~= (480+12)/21MHz ~= 23us */
    ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 1, ADC_SampleTime_480Cycles);

    /* EOC set at end of each regular conversion */
    ADC_EOCOnEachRegularChannelCmd(ADC1, ENABLE);

    /* enable ADC once here, and keep it on forever (do NOT toggle in tasks) */
    ADC_Cmd(ADC1, ENABLE);
}

/* blocking single conversion, timing-safe: no printf inside */
uint16_t Get_ADC_PM_Value(void)
{
    uint16_t value;
    uint32_t timeout = 0;

    ADC_ClearFlag(ADC1, ADC_FLAG_EOC);
    ADC_ClearFlag(ADC1, ADC_FLAG_OVR);

    ADC_SoftwareStartConv(ADC1);

    /* 等待转换完成，带超时保护，防止ADC异常时死循环 */
    while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET)
    {
        if(++timeout >= ADC_TIME_OUT)
        {
            /* 超时返回0，避免挂死 */
            return 0;
        }
    }

    value = ADC_GetConversionValue(ADC1);

    return value;
}

void ADC_PM_Demo(void)
{
    while(1){
        u16 value = Get_ADC_PM_Value();
        printf("value:%d\r\n",value);
        DWT_Delay_ms(1000);
    }
}
