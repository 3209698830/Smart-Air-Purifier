#include "ro.h"

// 锟斤拷锟斤拷锟斤拷全锟街硷拷锟斤拷值
volatile int16_t encoder_cnt = 0;

/*锟斤拷转锟斤拷锟斤拷锟斤拷锟斤拷
锟斤拷锟斤拷
锟斤拷占锟斤拷锟饺硷拷一锟斤拷锟斤拷锟铰斤拷锟截达拷锟斤拷
	A锟洁：PC6		锟斤拷锟诫，锟斤拷锟斤拷锟斤拷锟解部锟叫讹拷, EXTI_Line6
	B锟洁：PG15   锟斤拷锟诫，锟斤拷锟斤拷锟斤拷锟解部锟叫讹拷 锟斤拷EXTI_Line15
*/
void PO_Init(void)
{
	//时锟斤拷使锟斤拷	GPIO,EXTI,NVIC
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;

	//1.时锟斤拷使锟斤拷
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG | RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);	

	//2.GPIO锟斤拷始锟斤拷
	// ---------- 锟斤拷始锟斤拷 GPIOG Pin15 ----------
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOG, &GPIO_InitStructure);
	
	// ---------- 锟斤拷始锟斤拷 GPIOC Pin6 ----------
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	//3.锟斤拷锟截硷拷锟斤拷锟饺帮拷GPIO锟斤拷锟斤拷锟斤拷锟接碉拷EXTI锟斤拷路选锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷EXTI_Init之前锟斤拷
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOG,EXTI_PinSource15);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC,EXTI_PinSource6);
	
	//4.锟解部锟叫断筹拷始锟斤拷锟斤拷锟斤拷锟斤拷锟叫讹拷锟竭★拷锟街匡拷锟斤拷始锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷 | 锟较诧拷锟斤拷
	//------ EXTI6 (PC6) ------
	EXTI_InitStruct.EXTI_Line	 = EXTI_Line6;
	EXTI_InitStruct.EXTI_Mode	 = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling; //锟铰斤拷锟截达拷锟斤拷
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStruct);

	//------ EXTI15 (PG15) ------
	EXTI_InitStruct.EXTI_Line	 = EXTI_Line15;
	EXTI_InitStruct.EXTI_Mode	 = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStruct);
	
	//5.NVIC锟斤拷始锟斤拷
	// ---- EXTI6 通锟斤拷 EXTI9_5_IRQn ----
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_Init(&NVIC_InitStruct);

	// ---- EXTI15 通锟斤拷 EXTI15_10_IRQn ----
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_Init(&NVIC_InitStruct);
}

// 锟斤拷锟斤拷 Pin5~Pin9 锟斤拷锟叫讹拷 锟斤拷 PC6(EXTI6)
void EXTI9_5_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line6) != RESET)
    {
		//A锟斤拷(PC6)锟铰斤拷锟斤拷锟斤拷锟剿ｏ拷锟斤拷取B锟斤拷(PG15)锟斤拷平锟叫讹拷锟斤拷转锟斤拷锟斤拷
		uint8_t B = GPIO_ReadInputDataBit(GPIOG, GPIO_Pin_15);
		if(B == 0)
		{
			if(++encoder_cnt>50)	encoder_cnt=50;
		}
		else
		{
			if(--encoder_cnt<0)	encoder_cnt=0;
		}
        EXTI_ClearITPendingBit(EXTI_Line6);  //锟斤拷锟斤拷卸媳锟街疚?
    }
}

// 锟斤拷锟斤拷 Pin10~Pin15 锟斤拷锟叫讹拷 锟斤拷 PG15(EXTI15)
void EXTI15_10_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line15) != RESET)
    {
		//B锟斤拷(PG15)锟铰斤拷锟斤拷锟斤拷锟剿ｏ拷锟斤拷取A锟斤拷(PC6)锟斤拷平锟叫讹拷锟斤拷转锟斤拷锟斤拷
		uint8_t A = GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_6);
		if(A == 1)
		{
			if(++encoder_cnt>50)	encoder_cnt=50;
		}
		else
		{
			if(--encoder_cnt<0)	encoder_cnt=0;
		}
		
        EXTI_ClearITPendingBit(EXTI_Line15);
    }
}


void PO_Demo(void){
	
	DWT_Init();
	Uart1_Init(9600);
	
	int16_t last_encoder_cnt = 0;
	
	while(1){
	if(last_encoder_cnt!=encoder_cnt){
		
		last_encoder_cnt=encoder_cnt;
		printf("encoder_cnt: %d\r\n",encoder_cnt);
	}

	//printf("---");
	DWT_Delay_ms(20);
	}

}

