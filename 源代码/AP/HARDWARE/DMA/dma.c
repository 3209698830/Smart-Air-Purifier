#include "dma.h"

u16 adc_value[2]={0};

/*****************************************
引脚说明 
可调电阻连接在PA5 ADC1_IN5
温度传感器  ADC1_IN16
*******************************************/
void Dma_Init(void)
{
    //定义结构体变量
    GPIO_InitTypeDef        GPIO_InitStruct;
    ADC_CommonInitTypeDef   ADC_CommonInitStruct;
    ADC_InitTypeDef         ADC_InitStruct;
   
    //1.开启PA口时钟和ADC1时钟，设置PA5为模拟输入。
    RCC_AHB1PeriphClockCmd (RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);  
    
	//2.GPIO初始化 模拟模式
    GPIO_InitStruct.GPIO_Pin    	= GPIO_Pin_5;        //引脚5
    GPIO_InitStruct.GPIO_Mode    	= GPIO_Mode_AN;        //模拟模式
    GPIO_InitStruct.GPIO_PuPd   	= GPIO_PuPd_NOPULL;     //浮空模式

    GPIO_Init(GPIOA, &GPIO_InitStruct);       
	
	// 3. 配置ADC公共参数（必须！）
    ADC_CommonInitStruct.ADC_Mode = ADC_Mode_Independent;         // 独立模式
    ADC_CommonInitStruct.ADC_Prescaler = ADC_Prescaler_Div4;      // 分频系数，确保ADC时钟≤36MHz（F4系列）
    ADC_CommonInitStruct.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled; //使能DMA访问 在双重/三重模式下才有用
    ADC_CommonInitStruct.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles; // 采样间隔
    ADC_CommonInit(&ADC_CommonInitStruct);
	
	//4.ADC初始化
    ADC_InitStruct.ADC_Resolution        	= ADC_Resolution_12b; //12位分辨率
    ADC_InitStruct.ADC_DataAlign        	= ADC_DataAlign_Right;//数据右对齐
    ADC_InitStruct.ADC_ContinuousConvMode   = ENABLE; //连续转换模式
	//ADC_InitStruct.ADC_ExternalTrigConv   =  //不使用触发模式，则认为使用软件触发
    ADC_InitStruct.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None; //软件启动触发
    ADC_InitStruct.ADC_NbrOfConversion    	= 2; //转换通道数目
    ADC_InitStruct.ADC_ScanConvMode        	= ENABLE;//扫描选择
    
    ADC_Init(ADC1, &ADC_InitStruct);
    //5.使能ADC。
    ADC_Cmd(ADC1, ENABLE);
	
	/******温度传感器*****/
	//将温度传感器从掉电模式中唤醒
	ADC_TempSensorVrefintCmd(ENABLE);
	
	//ADC的DMA使能。调用该函数告诉 ADC 硬件：转换完成后，立即向 DMA 控制器发送一个请求信号。
	ADC_DMACmd(ADC1, ENABLE);
	//ADC采用连续转换模式，每次转换都发送DMA请求。每次转换完成都产生 DMA 请求，ADC 持续不断循环转换
	ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
	
    /****************DMA2 数据流0 通道0*******************/
    
    DMA_InitTypeDef DMA_InitStruct;
    //时钟使能
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
	
	//DMA配置初始化 
    DMA_StructInit(&DMA_InitStruct);
    
     /* Initialize the DMA_Channel member */
    DMA_InitStruct.DMA_Channel 					= 0;//DMA数据流的指定通道
    DMA_InitStruct.DMA_PeripheralBaseAddr 		= (u32)ADC1 + 0x4C; //外设数据寄存器地址--地址常量
    DMA_InitStruct.DMA_Memory0BaseAddr 			= (u32)adc_value; //存储器的地址   --地址常量
    DMA_InitStruct.DMA_DIR 						= DMA_DIR_PeripheralToMemory; //DMA方向 外设到存储器
    DMA_InitStruct.DMA_BufferSize 				= 2; //传输计数器的个数，一次使能 要搬运多少次
    DMA_InitStruct.DMA_PeripheralInc 			= DMA_PeripheralInc_Disable; //外设地址不需要自增
    DMA_InitStruct.DMA_MemoryInc 				= DMA_MemoryInc_Enable;//存储器地址需要自增
    DMA_InitStruct.DMA_PeripheralDataSize 		= DMA_PeripheralDataSize_HalfWord; //外设到DMA的数据宽度 半字
    DMA_InitStruct.DMA_MemoryDataSize 			= DMA_MemoryDataSize_HalfWord;//DMA到存储器的数据宽度 半字
    DMA_InitStruct.DMA_Mode 					= DMA_Mode_Circular; //DMA模式  单次模式  / 循环模式DMA_Mode_Circular
    DMA_InitStruct.DMA_Priority 				= DMA_Priority_High; //DMA优先级   当由多少通道数据要搬运时 哪个先搬运
    DMA_InitStruct.DMA_FIFOMode 				= DMA_FIFOMode_Disable;//直接模式 /FIFO模式
    /* Initialize the DMA_FIFOThreshold member */
    //DMA_InitStruct.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull; FIFO模式在用到
    DMA_InitStruct.DMA_MemoryBurst 				= DMA_MemoryBurst_Single;//每个DMA请求期间，只传输一个数据单元
    DMA_InitStruct.DMA_PeripheralBurst 			= DMA_PeripheralBurst_Single;//每个DMA请求期间，只传输一个数据单元
    
    DMA_Init(DMA2_Stream0,&DMA_InitStruct);
	
	DMA_Cmd(DMA2_Stream0,ENABLE);
	
	//让ADC 开始干活
	//指定转换的通道
    ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 1, ADC_SampleTime_56Cycles);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 2, ADC_SampleTime_480Cycles);
	
    //开启软件转换
    ADC_SoftwareStartConv(ADC1);
}

void Dma_Demo()
{
	while(1){
		printf("adc_value[0]:%d [1]:%d\r\n",adc_value[0],adc_value[1]);
		
		float v = adc_value[1] *3.3 / 4095 ;
		//通过公式转换为温度
		float temp = (v - 0.76) / 2.5 + 25;
		
		printf("temp:%.2f\r\n",temp);

		delay_s(1);
	}
}
