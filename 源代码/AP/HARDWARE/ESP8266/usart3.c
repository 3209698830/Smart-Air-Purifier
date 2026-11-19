#include "usart3.h"

char rx_buffer[RX_BUF_SIZE] = {0};
int rx_index1 = 0;


/*
USART3_TX  PB10
USART3_RX  PB11
*/
void Usart3_Init(int BaudRate)
{
    GPIO_InitTypeDef         GPIO_InitStruct;
    USART_InitTypeDef         USART_InitStruct;
    NVIC_InitTypeDef         NVIC_InitStruct;
    
    //1.GPIO使能 串口使能
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    
    //2.初始化GPIO 复用功能
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10| GPIO_Pin_11;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;  /* TX推挽输出，不设的话值取决于栈残留，
                                                    可能变成开漏导致ESP8266收不到AT命令 */
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Medium_Speed;

    GPIO_Init(GPIOB,&GPIO_InitStruct);

    //3.UART初始化
    USART_InitStruct.USART_BaudRate            = BaudRate;         //波特率
    USART_InitStruct.USART_Mode                = USART_Mode_Tx|USART_Mode_Rx; //全双工
    USART_InitStruct.USART_Parity           = USART_Parity_No;    //无奇偶校验
    USART_InitStruct.USART_StopBits            = USART_StopBits_1; //1位停止位
    USART_InitStruct.USART_WordLength       = USART_WordLength_8b;//字长
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件控制流

    USART_Init( USART3,&USART_InitStruct);
    
    //4.UART 和GPIO映射 --连接
    GPIO_PinAFConfig(GPIOB,GPIO_PinSource10,GPIO_AF_USART3);
    GPIO_PinAFConfig(GPIOB,GPIO_PinSource11,GPIO_AF_USART3);
    
    //5.NVIC初始化
    NVIC_InitStruct.NVIC_IRQChannel= USART3_IRQn;//中断通道 
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;//中断通道使能
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x01;//抢占优先级
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x01;//响应优先级
    
    NVIC_Init(&NVIC_InitStruct);
    
    //6.串口接收中断使能
    USART_ITConfig(USART3,USART_IT_RXNE,ENABLE);
    
    /* 开启空闲中断使能*/
    USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);
    
    //7.串口使能
    USART_Cmd(USART3,ENABLE);
}

//8.串口中断服务函数
void USART3_IRQHandler(void)
{
    uint8_t  data;
    
    //判断中断标志位
    if(USART_GetITStatus(USART3,USART_IT_RXNE) == SET)
    {
        //接收数据
        data = USART_ReceiveData(USART3);
        
        if(rx_index1 < RX_BUF_SIZE - 1) 
        {
            rx_buffer[rx_index1++] = data;
            rx_buffer[rx_index1] = '\0'; // 保持字符串终止
        }
        
        //清除中断标志位
        USART_ClearITPendingBit(USART3,USART_IT_RXNE);
    }
    //2.判断空闲中断标志位是否触发
    if(USART_GetITStatus(USART3, USART_IT_IDLE) == SET)
    {
        //清除空闲中断标志位
        /* 手册26.6.1USART寄存器描述，先读SR再读DR，清除IDLE标志位*/
        USART3->SR;
        USART3->DR;
    }
}
void Usart3_SendStr(char *str)
{
    int len = strlen(str);

    for(int i=0; i<len; i++)
    {
        /* 必须先等TXE(发送寄存器空)再写入！
           原来先写后等，发完MQTT包立刻发"AT+CIPSEND"时，
           第一个字节'A'会覆盖DR里还没移出去的MQTT包最后一字节，
           导致"auto"->"autA"、"on"->"oA"的尾字符被替换成'A' */
        while(USART_GetFlagStatus(USART3,USART_FLAG_TXE)==RESET);
        USART_SendData(USART3, str[i]);
    }
    /* 等待最后一字节完全移出移位寄存器，保证函数返回时线上数据完整 */
    while(USART_GetFlagStatus(USART3,USART_FLAG_TC)==RESET);
}

/*
 * 函数功能：通过串口发送指定长度的字节数据
 */
void Usart3_SendBytes( uint8_t *data, uint16_t len)
{
    for(uint16_t i = 0; i < len; i++)
    {
        /* 先等TXE再写，防止覆盖上一字节 */
        while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
        USART_SendData(USART3, data[i]);
    }
    /* 等待最后一字节完全发出，防止后续AT命令的第一个'A'破坏包尾 */
    while(USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
}

/*
 * 函数功能：清空串口3接收缓冲区（关中断保护，防止ISR和任务同时操作rx_buffer）
 */
void Usart3_RxClear(void)
{
    NVIC_DisableIRQ(USART3_IRQn);
    rx_index1 = 0;
    memset(rx_buffer, 0, RX_BUF_SIZE);
    NVIC_EnableIRQ(USART3_IRQn);
}

/*
 * USART3回环诊断：不发FreeRTOS任务、不开调度器，直接裸机跑。
 * 用法：main里注释掉ESP8266_Demo()，放开Usart3_LoopbackTest()
 *
 * 流程：
 * 1. 初始化USART3(115200)
 * 2. 向ESP8266发送"AT\r\n"，等2秒
 * 3. 打印rx_buffer收到的内容到串口1(9600)供查看
 * 4. 再发"AT+RST\r\n"，看有没有复位消息
 *
 * 判读：
 * - rx_buffer有"OK"           -> 软件完全正常，问题在WiFi配置/流程
 * - rx_buffer有乱码           -> 波特率不对，试9600/74880
 * - rx_buffer完全空           -> 硬件问题：TX/RX接反、ESP8266没供电、
 *                                EN/CH_PD没拉高、杜邦线松动
 */
void Usart3_LoopbackTest(void)
{
    /* 此函数跑在裸机main里，没有FreeRTOS，用忙等延时 */
    int i;
    volatile uint32_t dly;

    Uart1_Init(9600);
    printf("\r\n===== USART3 Loopback Test =====\r\n");
    printf("PB10(TX) -> ESP8266 RXD\r\n");
    printf("PB11(RX) <- ESP8266 TXD\r\n");
    printf("Baud: 115200\r\n\r\n");

    Usart3_Init(115200);

    /* 清空接收缓冲 */
    memset(rx_buffer, 0, RX_BUF_SIZE);
    rx_index1 = 0;

    /* 发送AT测试 */
    printf("[1] Sending AT... \r\n");
    Usart3_SendStr("AT\r\n");

    /* 忙等2秒等回复(不用vTaskDelay,因为没开调度器) */
    for(dly = 0; dly < 2; dly++)
    {
        volatile uint32_t j;
        for(j = 0; j < 14000000; j++) {}  /* 约1秒 @168MHz */
    }

    printf("rx_buffer = [");
    for(i = 0; i < rx_index1; i++)
    {
        if(rx_buffer[i] >= 0x20 && rx_buffer[i] < 0x7F)
            printf("%c", rx_buffer[i]);
        else
            printf("\\x%02X", (uint8_t)rx_buffer[i]);
    }
    printf("]\r\n");
    printf("rx_index1 = %d\r\n\r\n", rx_index1);

    /* 第二轮：发AT+RST看复位消息 */
    memset(rx_buffer, 0, RX_BUF_SIZE);
    rx_index1 = 0;

    printf("[2] Sending AT+RST... \r\n");
    Usart3_SendStr("AT+RST\r\n");

    for(dly = 0; dly < 3; dly++)
    {
        volatile uint32_t j;
        for(j = 0; j < 14000000; j++) {}
    }

    printf("rx_buffer = [");
    for(i = 0; i < rx_index1; i++)
    {
        if(rx_buffer[i] >= 0x20 && rx_buffer[i] < 0x7F)
            printf("%c", rx_buffer[i]);
        else
            printf("\\x%02X", (uint8_t)rx_buffer[i]);
    }
    printf("]\r\n");
    printf("rx_index1 = %d\r\n\r\n", rx_index1);

    /* 判读提示 */
    if(rx_index1 == 0)
    {
        printf(">>> rx_buffer EMPTY: USART3 RX received NOTHING.\r\n");
        printf(">>> Check: TX/RX swapped? ESP8266 powered? EN/CH_PD pulled high?\r\n");
        printf(">>> Wiring: PB10->ESP_RXD, PB11<-ESP_TXD, common GND.\r\n");
    }
    else if(strstr(rx_buffer, "OK"))
    {
        printf(">>> Got 'OK': USART3 communication works! Problem is in WiFi config.\r\n");
    }
    else
    {
        printf(">>> Got garbled data: baud rate mismatch? Try 9600.\r\n");
    }

    printf("===== Test End =====\r\n");
    while(1);
}
