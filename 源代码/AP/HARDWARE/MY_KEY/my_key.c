#include "my_key.h"

#include "FreeRTOS.h"
#include "task.h"



// 按键数据结构
typedef struct {
    GPIO_TypeDef*   GPIOx;          // 端口
    uint16_t        GPIO_Pin;       // 引脚
    uint8_t         active_level;   // 有效电平 0=低有效, 1=高有效
    
    KEY_STATE       state;          // 当前状态
    uint8_t         debounce_cnt;   // 消抖计数器
    uint16_t        hold_cnt;       // 长按计数器
    KEY_EVENT       event;          // 触发事件
} KEY_TYPE;

// 按键表（低电平有效）
static KEY_TYPE key_table[KEY_NUM] = {
    {KEY0_PORT, KEY0_PIN, 0, KEY_STATE_IDLE, 0, 0, KEY_EVENT_NONE},  // PA0
		
    {KEY1_PORT, KEY1_PIN, 0, KEY_STATE_IDLE, 0, 0, KEY_EVENT_NONE},  // PE2

    {KEY2_PORT, KEY2_PIN, 0, KEY_STATE_IDLE, 0, 0, KEY_EVENT_NONE},  // PE3
		
    {KEY3_PORT, KEY3_PIN, 0, KEY_STATE_IDLE, 0, 0, KEY_EVENT_NONE},  // PE4
};

/**
 * @brief  按键GPIO初始化
 */
void MY_KEY_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 使能时钟
    RCC_AHB1PeriphClockCmd(KEY0_CLK | KEY1_CLK | KEY2_CLK | KEY3_CLK, ENABLE);
    
    // PA0配置：输入模式，上拉（按下为低电平）
    GPIO_InitStructure.GPIO_Pin   = KEY0_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;  // 补全：输入模式下任意值均可
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;     // 补全：输入模式下无效
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_Init(KEY0_PORT, &GPIO_InitStructure);
    
    // PE2-4配置：输入模式，上拉（按下为低电平）
    GPIO_InitStructure.GPIO_Pin = KEY1_PIN | KEY2_PIN | KEY3_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;        // 上拉
    GPIO_Init(GPIOE, &GPIO_InitStructure);
}

/**
 * @brief  读取单个按键原始状态
 * @param  key: 按键索引
 * @retval 1=按下，0=释放
 */
static uint8_t KEY_ReadPin(uint8_t key)
{
    uint8_t pin_state;
    
    pin_state = GPIO_ReadInputDataBit(key_table[key].GPIOx, key_table[key].GPIO_Pin);
    
    // 根据有效电平判断是否为按下状态
    if (key_table[key].active_level == 1) {
        return pin_state;           // 高有效：高电平=按下
    } else {
        return (pin_state == 0) ? 1 : 0;  // 低有效：低电平=按下
    }
}

/**
 * @brief  按键扫描任务（计数法消抖核心）
 * @note   必须在定时中断中每10ms调用一次，或在主循环中通过延时保证周期
 */
void KEY_ScanTask(void)
{
    uint8_t i;
    uint8_t key_current;    // 当前物理状态
    
    for (i = 0; i < KEY_NUM; i++) {
        key_current = KEY_ReadPin(i);
        
        switch (key_table[i].state) {
            
            //================ 空闲状态 ================
            case KEY_STATE_IDLE:
                if (key_current == 1) {                 // 检测到按下
                    key_table[i].debounce_cnt = 0;      // 清零计数器
                    key_table[i].state = KEY_STATE_PRESSING;
                }
                key_table[i].event = KEY_EVENT_NONE;
                break;
            
            //================ 消抖确认按下 ================
            case KEY_STATE_PRESSING:
                if (key_current == 1) {
                    key_table[i].debounce_cnt++;        // 持续按下，计数++
                    
                    // 达到消抖阈值，确认按下
                    if (key_table[i].debounce_cnt >= KEY_DEBOUNCE_CNT) {
                        key_table[i].state = KEY_STATE_PRESSED;
                        key_table[i].hold_cnt = 0;      // 准备计时长按
                        key_table[i].event = KEY_EVENT_SHORT_PRESS; // 触发短按事件
                    }
                } else {
                    // 抖动/误触，回到空闲
                    key_table[i].state = KEY_STATE_IDLE;
                }
                break;
            
            //================ 已确认按下 ================
            case KEY_STATE_PRESSED:
            case KEY_STATE_HOLDING:
                if (key_current == 1) {
                    key_table[i].hold_cnt++;            // 持续计时
                    
                    // 达到长按阈值
                    if (key_table[i].hold_cnt >= KEY_LONG_CNT) {
                        if (key_table[i].state != KEY_STATE_HOLDING) {
                            key_table[i].state = KEY_STATE_HOLDING;
                            key_table[i].event = KEY_EVENT_LONG_PRESS;
                        }
                        key_table[i].hold_cnt = KEY_LONG_CNT; // 保持最大值，防止溢出
                    } else {
                        key_table[i].event = KEY_EVENT_NONE;
                    }
                } else {
                    // 开始释放消抖
                    key_table[i].debounce_cnt = 0;
                    key_table[i].state = KEY_STATE_RELEASING;
                }
                break;
            
            //================ 消抖确认释放 ================
            case KEY_STATE_RELEASING:
                if (key_current == 0) {
                    key_table[i].debounce_cnt++;
                    
                    // 达到消抖阈值，确认释放
                    if (key_table[i].debounce_cnt >= KEY_DEBOUNCE_CNT) {
                        key_table[i].state = KEY_STATE_IDLE;
                        key_table[i].event = KEY_EVENT_RELEASE;
                        key_table[i].hold_cnt = 0;
                    }
                } else {
                    // 抖动，回到按住状态
                    key_table[i].state = (key_table[i].hold_cnt >= KEY_LONG_CNT) ? 
                                          KEY_STATE_HOLDING : KEY_STATE_PRESSED;
                }
                break;
            
            default:
                key_table[i].state = KEY_STATE_IDLE;
                break;
        }
    }
}

/**
 * @brief  获取按键事件（读取后自动清除）
 * @param  key_id: 按键ID 0-3
 * @retval 按键事件类型
 */
KEY_EVENT KEY_GetEvent(uint8_t key_id)
{
    KEY_EVENT event;
    
    if (key_id >= KEY_NUM) return KEY_EVENT_NONE;
    
    event = key_table[key_id].event;
    key_table[key_id].event = KEY_EVENT_NONE;   // 清除事件
    
    return event;
}

/**
 * @brief  查询按键是否处于按下状态
 * @param  key_id: 按键ID
 * @retval 1=按下，0=释放
 */
uint8_t KEY_IsPressed(uint8_t key_id)
{
    if (key_id >= KEY_NUM) return 0;
    
    return (key_table[key_id].state == KEY_STATE_PRESSED || 
            key_table[key_id].state == KEY_STATE_HOLDING) ? 1 : 0;
}

/**
 * @brief  获取按键按住时间（单位：10ms）
 */
uint32_t KEY_GetHoldTime(uint8_t key_id)
{
    if (key_id >= KEY_NUM) return 0;
    return key_table[key_id].hold_cnt * KEY_SCAN_INTERVAL;
}

void KEY_Demo()
{
	while (1) {
        KEY_ScanTask();
        
        // 方式1：查询事件（推荐）
        for (uint8_t i = 0; i < KEY_NUM; i++) {
            KEY_EVENT evt = KEY_GetEvent(i);
            
            switch (evt) {
                case KEY_EVENT_SHORT_PRESS:
                    
                    //printf("KEY%d 短按\r\n", i);
                    // 执行短按动作
					switch(i)
					{
						case KEY_0:
									GPIO_ToggleBits(GPIOF, GPIO_Pin_9);
									break;
						case KEY_1:
									GPIO_ToggleBits(GPIOF, GPIO_Pin_10);
									break;
						case KEY_2:
									GPIO_ToggleBits(GPIOE, GPIO_Pin_13);
									break;
						case KEY_3:
									GPIO_ToggleBits(GPIOE, GPIO_Pin_14);
									break;						
					}
                    break;
                    
          case KEY_EVENT_LONG_PRESS:
          switch(i)
					{
						case KEY_0:
									GPIO_ToggleBits(GPIOF, GPIO_Pin_9);
									break;
						case KEY_1:
									GPIO_ToggleBits(GPIOF, GPIO_Pin_10);
									break;
						case KEY_2:
									GPIO_ToggleBits(GPIOE, GPIO_Pin_13);
									break;
						case KEY_3:
									GPIO_ToggleBits(GPIOE, GPIO_Pin_14);
									break;						
					}
                    //printf("KEY%d 长按1秒\r\n", i);
                    // 执行长按动作
                    break;
                    
                case KEY_EVENT_RELEASE:
                    //printf("KEY%d 释放，按住时间：%dms\r\n", 
                    //       i, KEY_GetHoldTime(i));
                    break;
                    
                default:
                    break;
            }
        }
        
        // 方式2：查询持续状态
        if (KEY_IsPressed(KEY_0)) {
            // KEY0正在按住
        }
        
		// 在主循环中添加一个短延迟
        vTaskDelay(10);
    }
}

void KEY_To_PMpj()
{
        KEY_ScanTask();
        
        // 方式1：查询事件（推荐）
        for (uint8_t i = 0; i < KEY_NUM; i++) {
            KEY_EVENT evt = KEY_GetEvent(i);
            
            switch (evt) {
                case KEY_EVENT_SHORT_PRESS:
                    
                    //printf("KEY%d 短按\r\n", i);
                    // 执行短按动作
					switch(i)
					{
						case KEY_0:
									PM_FAN_MODE=(PM_FAN_MODE==0)?1:0;
									break;
						case KEY_1:
							if(PM_FAN_MODE)
									Switch_status_NG();//改变负离子发生器 启停状态
									break;
						case KEY_2:
									Set_user_act(1);
									break;
						case KEY_3:

									break;						
					}
                    break;
                    
          case KEY_EVENT_LONG_PRESS:
          switch(i)
					{
						case KEY_0:
									
									break;
						case KEY_1:
									
									break;
						case KEY_2:

									break;
						case KEY_3:

									break;						
					}
                    //printf("KEY%d 长按1秒\r\n", i);
                    // 执行长按动作
                    break;
                    
                case KEY_EVENT_RELEASE:
                    //printf("KEY%d 释放，按住时间：%dms\r\n", 
                    //       i, KEY_GetHoldTime(i));
                    break;
                    
                default:
                    break;
            }
        }
        
//        // 方式2：查询持续状态
//        if (KEY_IsPressed(KEY_0)) {
//            // KEY0正在按住
//					}

}
