#include "st_delay.h"

#include "FreeRTOS.h"
#include "task.h"

/* ============================================================
 * 延时函数库
 *
 * 提供微秒、毫秒、秒级延时
 * - delay_us: 基于 SysTick 忙等待，始终可用
 * - delay_ms: 调度器运行时用 vTaskDelay(让出CPU)，否则忙等待
 * - delay_s:  delay_ms 的简单封装
 *
 * 注意: DHT11 等需要精确微秒延时的场景建议用 DWT 计数器
 *       (见 dht11.c 中的 dht11_delay_us)，因为 SysTick 在
 *       FreeRTOS 启动后会被重新配置。
 * ============================================================ */

/* SysTick 初始化
 * 在调度器启动前配置 SysTick 用于基础计时
 * FreeRTOS 启动后会重新配置 SysTick 作为系统节拍定时器 */
void SysTick_st_Init(void)
{
    /* 停止 SysTick, 清除当前值 */
    SysTick->CTRL = 0;
    SysTick->VAL = 0;

    /* 设置重载值为最大值(24位), 以测量较长的时间间隔 */
    SysTick->LOAD = 0xFFFFFF;

    /* 启用 SysTick, 使用处理器时钟, 不产生中断(仅用于计时) */
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
}

/* 微秒级延时 - 基于 SysTick 忙等待
 * 始终可用(调度器启动前后均可)
 * 注意: 此函数为忙等待,不会让出CPU
 *       在 FreeRTOS 环境下,精度可能受调度影响 */
void delay_st_us(uint32_t nus)
{
    uint32_t ticks;
    uint32_t told, tnow, tcnt = 0;
    uint32_t reload = SysTick->LOAD;

    /* 计算需要的 SysTick 时钟周期数
     * SystemCoreClock 通常为 168MHz, 即 168 个周期/微秒 */
    ticks = nus * (SystemCoreClock / 1000000);
    told = SysTick->VAL;

    /* 忙等待, 通过 SysTick 递减计数器测量时间 */
    while(1)
    {
        tnow = SysTick->VAL;

        if(tnow != told)
        {
            /* SysTick 是递减计数器, 需要处理计数器回绕 */
            if(tnow < told)
                tcnt += told - tnow;  /* 正常递减 */
            else
                tcnt += reload - tnow + told;  /* 发生回绕 */

            told = tnow;

            if(tcnt >= ticks)
                break;  /* 达到目标时间 */
        }
    }
}

/* 毫秒级延时
 * - 调度器运行时: 使用 vTaskDelay 让出 CPU (推荐,不阻塞其他任务)
 * - 调度器未启动: 使用忙等待 (用于初始化阶段) */
void delay_st_ms(uint32_t nms)
{
    if(xTaskGetSchedulerState() == taskSCHEDULER_RUNNING)
    {
        /* 调度器运行: 使用 vTaskDelay 让出 CPU
         * 其他任务可以在此期间运行 */
        vTaskDelay(pdMS_TO_TICKS(nms));
    }
    else
    {
        /* 调度器未启动: 忙等待回退方案 (1ms 一次) */
        for(uint32_t i = 0; i < nms; i++)
        {
            delay_st_us(1000);
        }
    }
}

/* 秒级延时 - delay_ms 的简单封装 */
void delay_st_s(uint32_t ns)
{
    delay_st_ms(ns * 1000);
}
