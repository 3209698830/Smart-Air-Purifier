#include "dht11.h"
#include "dwt.h"

/**********************************
引脚说明：
DHT11 --- PG9
***********************************/
void Dht11_Init(void)
{
		DWT_Init();
    GPIO_InitTypeDef      GPIO_InitStructure;
    
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
     
    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_9;                //第9号引脚
    GPIO_InitStructure.GPIO_Mode     = GPIO_Mode_OUT;            //输出模式
    GPIO_InitStructure.GPIO_OType     = GPIO_OType_PP;            //推挽输出，增强驱动能力，引脚的输出电流更大
    GPIO_InitStructure.GPIO_Speed     = GPIO_Speed_25MHz;            //引脚的速度最大为25MHz
    GPIO_InitStructure.GPIO_PuPd     = GPIO_PuPd_NOPULL;            //没有使用内部上拉电阻
    GPIO_Init(GPIOG, &GPIO_InitStructure);    
    
    //温湿度模块还没有工作，那么它的触发引脚是高电平
    PGout(9) = 1;
}
//引脚模式变更
void Dht11_Pin_Mode(GPIOMode_TypeDef mode)
{
    GPIO_InitTypeDef      GPIO_InitStructure;
    
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;                //第9号引脚
    GPIO_InitStructure.GPIO_Mode  = mode;                    //输入/输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;            //推挽输出，增强驱动能力，引脚的输出电流更大
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;        //引脚的速度最大为25MHz
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;        //没有使用内部上拉电阻
    GPIO_Init(GPIOG, &GPIO_InitStructure);    
}
//启动DHT11正常返回0
int  Dht11_Start(void)
{
    int t = 0;
    
    //启动信号
    //引脚做为输出
    Dht11_Pin_Mode(GPIO_Mode_OUT);
    PGout(9) = 1;
    DWT_Delay_ms(1);
    PGout(9) = 0;
    DWT_Delay_ms(20);
    PGout(9) = 1;
    DWT_Delay_us(30);
    
    //响应信号 
    //引脚做为输入
    Dht11_Pin_Mode(GPIO_Mode_IN);    
    
    //等待低电平到来
    while(PGin(9) == 1)
    {
        t++;
        DWT_Delay_us(2);
        
        if(t >= 20) //40us未监测到低电平，则返回
            return -1;
    }
    
    DWT_Delay_us(40);
    
    t = 0;
    //等待高电平到来，过滤低电平
    while(PGin(9) == 0)
    {
        t++;
        DWT_Delay_us(2);
        
        if(t >= 50) //100us未监测到高电平，则返回
            return -2;    
    }
    
    DWT_Delay_us(40);
    
    t = 0;
    //等待低电平到来，过滤高电平
    while(PGin(9) == 1)
    {
        t++;
        DWT_Delay_us(2);
        
        if(t >= 50) //100us未监测到低电平，则返回
            return -3;    
    }
    
    
    return 0;
}
uint8_t Dht11_Read_Byte(void)
{
    u8 i, t,data = 0x00; 
    
    for(i=0; i<8; i++)
    {
        t = 0;
        //等待高电平到来，过滤低电平
        while(PGin(9) == 0)
        {
            t++;
            DWT_Delay_us(2);
            
            if(t >= 50) //100us未监测到高电平，则返回
                return 0;    
        }    
    
        //延时一小段时间
        DWT_Delay_us(40);
        
        
        if(PGin(9) == 1)
        {
            data |= (0x01<<(7-i));
        
            t = 0;
            //等待低电平到来，过滤高电平
            while(PGin(9) == 1)
            {
                t++;
                DWT_Delay_us(2);
                
                if(t >= 50) //100us未监测到低电平，则返回
                    return -3;    
            }            
                
        }
    
    }
    return data;
}

int Dht11_Data(u8 *data)
{
    u8 i;
    
    for(i=0; i<5; i++)
    {
        data[i] = Dht11_Read_Byte();
    }
    if(data[4] == data[0]+data[1]+data[2]+data[3])
    {
        return 0;
    }
    else
    {
        return -1;
    }
}
#if 0
void Dht11_Demo()
{
	u8 data[5] = {0},ret;
	
	while(1){
		//MCU发起始信号，模块应答
		ret = Dht11_Start();
		if(ret == 0){
			//读取数据,并且校验
			ret = Dht11_Data(data);
			if(ret == 0){
				printf("H:%d.%d T:%d.%d\r\n",data[0],data[1],data[2],data[3]);
			}else{
				printf("parity error\r\n");
			}
		}
		
		DWT_Delay_s(2);
	}
}
#endif
int DHT11_GetData(uint8_t *temp, uint8_t *humi)
{
	uint8_t buf[5];
	int ret;
	
	taskENTER_CRITICAL(); 
	ret = Dht11_Start();
	if(ret == 0)
	{
		ret = Dht11_Data(buf);
	}
	taskEXIT_CRITICAL();
	
	if(ret != 0)
	{
		return -1;
	}
	
	*humi = buf[0];
	*temp = buf[2];
	
	return 0;
}

