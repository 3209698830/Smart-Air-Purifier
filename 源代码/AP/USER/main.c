#include "stm32f4xx.h"

#include "beep.h"
//#include "key.h"
#include "exti.h"
#include "fire.h"
#include "infrared_avoid.h"
#include "matrixkey.h"
#include "tim6.h"
//#include "delay.h"
#include "ickey.h"
#include "pwm.h"
#include "servo.h"
#include "motor.h"
#include "beep_audio.h"
#include "uart1.h"
#include "string.h"
#include "protocol.h"
#include "sr04.h"
//#include "nec.h"
//#include "tim.h"
#include "saleae.h"
#include "dx24.h"
#include "dht11.h"
#include "iwdg.h"
#include "rs485.h"
#include "can.h"
#include "rtc.h"
#include "adc.h"
#include "adc_light.h"
#include "adc_cpu.h"
#include "dma.h"
#include "flash.h"
#include "at24c02.h"
#include "eeprom.h"
#include "w25q128.h"
#include "adc_pm.h"
#include "FreeRTOS.h"
#include "task.h"
#include "ac.h"
#include "pm.h"
#include "dwt.h"
#include "pwm_pm.h"
//#include "delay.h"
//#include "dma_pm.h"
//#include "st_delay.h"
#include "ro.h"
#include "ng.h"



uint8_t rx_buf[MAX_FRAME_LEN] = {0};
u16  rx_index = 0;
uint8_t  rx_flag = 0;

int main(void)
{
	/* FreeRTOS在Cortex‑M4上要求优先级分组设为Group4(全抢占无子优先级)，
	   否则BASEPRI掩码行为不正确，可能导致中断异常。
	   必须在任何NVIC_Init调用之前设置。 */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	Uart1_Init(9600);
	printf("action\r\n");
#if 0
	
	Pwm_Pm_Init();
	DWT_Init();
	PM_Init();
	Dht11_Init();
	ESP8266_Init();
	NG_Init();//负离子发生生器初始化
	PO_Init();//旋转编码器初始化
#endif
	//DWT_Delay_s(2);           //剩余时间凑够10ms周期
	air();




	while(1)
	{
		/* 此处不会执行，air()内部开启FreeRTOS调度器 */
	}
}
