#ifndef __CAN_H
#define __CAN_H

#include "stm32f4xx.h"

/* CAN接收数据缓冲区 */
extern uint8_t CAN1_RxData[8];
extern uint8_t CAN1_RxFlag;      // 接收标志

/* CAN初始化函数 */
u8 CAN1_Mode_Init(u8 tsjw, u8 tbs2, u8 tbs1, u16 brp, u8 mode);

/* CAN发送函数 */
u8 CAN1_Send_Msg(u8 *msg, u8 len);

/* CAN接收函数 */
u8 CAN1_Receive_Msg(u8 *buf);

void CAN1_Demo();
#endif
