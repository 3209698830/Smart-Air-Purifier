#include "pm.h"

/*
pm2.5传感器
引脚PD7 推挽输出(传感器的led)
引脚PA4 模拟模式

*/

void PM_Init(void){

	GPIO_InitTypeDef GPIO_InitStruct;
	/**********PF9 PF10***************/
	//1.GPIOF组时钟使能
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	//2.GPIO配置
	GPIO_InitStruct.GPIO_Pin   = GPIO_Pin_7;//引脚
	GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_OUT;//输出模式
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;//推挽
	GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_DOWN;//下拉
	GPIO_InitStruct.GPIO_Speed = GPIO_Medium_Speed;//速度
	GPIO_Init(GPIOD,&GPIO_InitStruct);
	
	//ADC初始化
	ADC_PM_Init();
	
	DWT_Init();
	
	DWT_Delay_s(2);
	
}

void Led_Status(uint8_t x){//LED灯控制   x: 0灭，1亮

	if(x){
	GPIO_SetBits(GPIOD,GPIO_Pin_7);
	}else{
	GPIO_ResetBits(GPIOD,GPIO_Pin_7);
	}
}

//采集时序函数
uint16_t Acquisition_timing(void){
	
		uint16_t ADCVal;
		Led_Status(1);                //开启红外LED
		DWT_Delay_us(280);            //开灯等待280us

		ADCVal = Get_ADC_PM_Value();   //读取ADC，接口保持不变

		DWT_Delay_us(40);             //原厂样例改为40us，代替原先19us
		Led_Status(0);                //关闭红外LED
		DWT_Delay_us(9680);           //剩余时间凑够10ms周期
		return ADCVal;
}

void Get_PM_Value_demo(void)
{
	uint16_t ADCVal;
	uint16_t filter_out;
	int dustVal = 0;
	float Voltage;
	uint16_t voltage_mv;
	
	while(1)
	{
		Led_Status(1);                //开启红外LED
		DWT_Delay_us(280);            //开灯等待280us

		ADCVal = Get_ADC_PM_Value();   //读取ADC，接口保持不变
		
		DWT_Delay_us(40);             //原厂样例改为40us，代替原先19us
		Led_Status(0);                //关闭红外LED
		DWT_Delay_us(9680);           //剩余时间凑够10ms周期
		
		//----------去极值求平均滤波 ----------
		//filter_out = Filter_ExtremeAverage(ADCVal);
		filter_out =ADCVal;
		
		//---------- 电压换算 (mV) ----------
		// 注意：如果你转接板自带分压 *11 就不要*2，二选一！
		voltage_mv = (u16)((( (uint32_t)filter_out * 3300 ) / 4096 ));
		Voltage = voltage_mv / 1000.0f;
		
		//---------- 粉尘浓度计算，参考原厂逻辑 ----------
		if(voltage_mv > NO_DUST_MV)
		{
			dustVal = (int)( (voltage_mv - NO_DUST_MV) * COV_RATIO );
		}
		else
		{
			dustVal = 0;
		}

		printf("ADC:%d  Filter:%d  Voltage:%.2f V  PM2.5:%d ug/m3\r\n",ADCVal,filter_out,Voltage,dustVal);

		DWT_Delay_s(1); //打印间隔1秒
	}
}


/**
 * @brief   去极值平均滤波
 * @param   raw_val:单次读到的ADC原始值
 * @retval  滤波之后的ADC结果
 */
uint16_t Filter_ExtremeAverage(uint16_t raw_val)
{
    static uint16_t buf[PM_SAMPLE_CNT];   //采样缓冲区
    static uint8_t index = 0;
    uint32_t sum = 0;
    uint16_t max_val, min_val;
    uint8_t i;

    //填入新采样值
    buf[index++] = raw_val;
    if(index >= PM_SAMPLE_CNT)
    {
        index = 0;
    }

    max_val = buf[0];
    min_val = buf[0];
    sum = buf[0];

    //遍历求和，找出最大最小值
    for(i = 1; i < PM_SAMPLE_CNT; i++)
    {
        sum += buf[i];
        if(buf[i] > max_val)
        {
            max_val = buf[i];
        }
        if(buf[i] < min_val)
        {
            min_val = buf[i];
        }
    }

    //去掉1个最大值，1个最小值，剩余求平均
    sum = sum - max_val - min_val;
    return (uint16_t)( sum / (PM_SAMPLE_CNT - 2) );
}
