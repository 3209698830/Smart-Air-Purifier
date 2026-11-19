#include "dwt.h"

void DWT_Init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

// 微秒延时：忙等待
void DWT_Delay_us(volatile uint32_t us)
{
    volatile uint32_t start  = DWT->CYCCNT;
    volatile uint32_t cycles = us * (SystemCoreClock / 1000000U);

    while ((DWT->CYCCNT - start) < cycles);
}

// 毫秒延时：忙等待
void DWT_Delay_ms(volatile uint32_t ms)
{
    DWT_Delay_us(ms * 1000U);
}

// 秒延时：忙等待
void DWT_Delay_s(volatile uint32_t s)
{
    DWT_Delay_ms(s * 1000U);
}
