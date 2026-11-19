#include "can.h"
#include "delay.h"

/* 接收缓冲区和标志 */
uint8_t CAN1_RxData[8];
uint8_t CAN1_RxFlag = 0;

/**
 * @brief CAN1初始化函数
 * @param tsjw: 重新同步跳跃时间单元 (1~4)
 * @param tbs2: 时间段2 (1~8)
 * @param tbs1: 时间段1 (1~16)
 * @param brp: 波特率分频器 (1~1024)
 * @param mode: 工作模式 (0=正常, 1=回环)
 * @retval 0=成功, 其他=失败
 */
u8 CAN1_Mode_Init(u8 tsjw, u8 tbs2, u8 tbs1, u16 brp, u8 mode)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    CAN_InitTypeDef CAN_InitStructure;
    CAN_FilterInitTypeDef CAN_FilterInitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    // 1. 使能相关时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);   // GPIOA时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);   // CAN1时钟
    
    // 2. 配置GPIO引脚复用功能
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_CAN1); // PD0 -> CAN1_RX
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource1, GPIO_AF_CAN1); // PA1 -> CAN1_TX
    
    // 3. 初始化GPIO
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;            // 复用功能
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;         // 推挽输出
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;           // 上拉
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;      // 50MHz
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    
    // 4. 复位CAN1
    CAN_DeInit(CAN1);
    
    // 5. 配置CAN工作模式
    CAN_InitStructure.CAN_TTCM = DISABLE;     // 关闭时间触发通信模式
    CAN_InitStructure.CAN_ABOM = DISABLE;     // 关闭自动离线管理
    CAN_InitStructure.CAN_AWUM = DISABLE;     // 关闭自动唤醒
    CAN_InitStructure.CAN_NART = ENABLE;      // 禁止自动重传（回环测试需要）
    CAN_InitStructure.CAN_RFLM = DISABLE;     // 接收FIFO不锁定，新报文覆盖旧报文
    CAN_InitStructure.CAN_TXFP = DISABLE;     // 优先级由报文标识符决定
    
    // 设置工作模式
    if(mode == 0)
        CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;       // 正常模式
    else if(mode == 1)
        CAN_InitStructure.CAN_Mode = CAN_Mode_LoopBack;     // 回环模式[10](@ref)
    
    // 6. 配置波特率 (500Kbps)
    // 计算公式: 波特率 = Fpclk1 / ((tbs1+1+tbs2+1+1)*brp) 
    // 42MHz / ((7+6+1)*6) = 42M / 84 = 500Kbps [11](@ref)
    CAN_InitStructure.CAN_SJW = tsjw;         // 同步跳转宽度
    CAN_InitStructure.CAN_BS1 = tbs1;         // 时间段1
    CAN_InitStructure.CAN_BS2 = tbs2;         // 时间段2
    CAN_InitStructure.CAN_Prescaler = brp;    // 分频系数 (brp = 实际分频值)
    
    CAN_Init(CAN1, &CAN_InitStructure);
    
    // 7. 配置过滤器（回环模式下设置为全通过滤器）
    CAN_FilterInitStructure.CAN_FilterNumber = 0;            // 使用过滤器组0
    CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;  // 屏蔽位模式  掩码模式
    CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit; // 32位宽  尺度
    
    CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;      // ID高16位
    CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;       // ID低16位
    CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;  // 掩码高16位（全0，接收所有ID）
    CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;   // 掩码低16位（全0，接收所有ID）
    
    CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0; // 关联到FIFO0
    CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;  // 激活过滤器
    CAN_FilterInit(&CAN_FilterInitStructure);
    
    // 8. 配置中断（接收中断）
    NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX0_IRQn;     // CAN1 RX0中断
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    // 9. 使能接收中断
    CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);   // FIFO0消息挂号中断
    
    return 0;
}

/**
 * @brief CAN1发送函数
 * @param msg: 待发送数据缓冲区指针
 * @param len: 数据长度 (最大8字节)
 * @retval 0=成功, 其他=失败
 */
u8 CAN1_Send_Msg(u8 *msg, u8 len)
{
    CanTxMsg TxMessage;
    u8 mbox;
    u16 i = 0;
    u32 timeout = 0xFFFF;
    
    // 配置发送报文
    TxMessage.StdId = 0x12;          // 标准ID
    TxMessage.ExtId = 0x12;          // 扩展ID
    TxMessage.IDE = CAN_Id_Standard; // 标准帧
    TxMessage.RTR = CAN_RTR_DATA;    // 数据帧
    TxMessage.DLC = len;             // 数据长度
    
    // 复制数据
    for(i = 0; i < len; i++)
    {
        TxMessage.Data[i] = msg[i];
    }
    
    // 发送报文
    mbox = CAN_Transmit(CAN1, &TxMessage);
    
    // 等待发送完成
    while((CAN_TransmitStatus(CAN1, mbox) == CAN_TxStatus_Failed) && (timeout > 0))
    {
        timeout--;
    }
    
    if(timeout == 0)
        return 1;  // 发送超时
    
    return 0;  // 发送成功
}

/**
 * @brief CAN1接收函数（查询方式）
 * @param buf: 接收数据缓冲区（至少8字节）
 * @retval 0=无数据, 其他=接收到的数据长度
 */
u8 CAN1_Receive_Msg(u8 *buf)
{
    CanRxMsg RxMessage;
    u8 i;
    
    // 检查是否有数据在FIFO0中
    if(CAN_MessagePending(CAN1, CAN_FIFO0) == 0)
        return 0;  // 无数据
    
    // 接收报文
    CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);
    
    // 复制接收数据
    for(i = 0; i < 8; i++)
    {
        buf[i] = RxMessage.Data[i];
    }
    
    return RxMessage.DLC;  // 返回实际接收的数据长度
}

/**
 * @brief CAN1 RX0中断服务函数
 */
void CAN1_RX0_IRQHandler(void)
{
    CanRxMsg RxMessage;
    u8 i;
    
    if(CAN_GetITStatus(CAN1, CAN_IT_FMP0) != RESET)
    {
        // 接收报文
        CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);
        
        // 保存接收数据
        for(i = 0; i < 8; i++)
        {
            CAN1_RxData[i] = RxMessage.Data[i];
        }
        
        // 设置接收标志
        CAN1_RxFlag = 1;
        
        // 清除中断标志
        CAN_ClearITPendingBit(CAN1, CAN_IT_FMP0);
    }
}

void CAN1_Demo()
{
	printf("CAN1回环模式测试开始...\r\n");
    printf("波特率: 500Kbps\r\n");
    printf("发送ID: 0x12\r\n\r\n");
	
    u8 i;
    u8 send_data[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    u8 receive_data[8];
	
	while(1){
		// 每秒发送一次数据
        delay_s(1);
        
        // 准备发送数据
        send_data[0]++;
		
        printf("发送数据: ");
        for(i = 0; i < 8; i++)
        {
            printf("0x%02X ", send_data[i]);
        }
        printf("\r\n");
        
        // 发送数据
        CAN1_Send_Msg(send_data, 8);
		
		// 等待接收（回环模式下，发送的数据会自己接收）
        delay_ms(10);
        
        // 检查中断接收标志
        if(CAN1_RxFlag == 1)
        {
            CAN1_RxFlag = 0;
            printf("中断接收成功! 数据: ");
            for(i = 0; i < 8; i++)
            {
                printf("0x%02X ", CAN1_RxData[i]);
            }
            printf("\r\n\r\n");
        }
	}
}