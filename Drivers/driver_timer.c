// SPDX-License-Identifier: GPL-3.0-only
/*
 * driver_timer.c - 延时与系统时间戳（标准库版）
 * udelay/mdelay：基于 TIM4（1MHz 自由计数）的忙等延时
 * system_get_ns：ms 部分 = FreeRTOS tick（1ms/tick），µs 部分 = TIM4 计数
 * getRunTimeCounterValue：FreeRTOS 运行时间统计使用
 */
#include "driver_timer.h"
#include "FreeRTOS.h"
#include "task.h"
#include "bsp_tim.h"

void udelay(int us)
{
    uint32_t ticks;
    uint32_t told, tnow, tcnt = 0;
    uint32_t reload = BSP_TIM4_GetReload();
    ticks = us * reload / (1000);  /* reload 对应 1ms */
    told = BSP_TIM4_GetCount();
    while (1)
    {
        tnow = BSP_TIM4_GetCount();
        if (tnow != told)
        {
            if (tnow > told)
            {
                tcnt += tnow - told;
            }
            else
            {
                tcnt += reload - told + tnow;
            }
            told = tnow;
            if (tcnt >= ticks)
            {
                break;
            }
        }
    }
}

void mdelay(int ms)
{
    for (int i = 0; i < ms; i++)
        udelay(1000);
}

/* ms 计数：FreeRTOS tick = 1kHz，1 tick = 1ms */
uint32_t BSP_GetTickMS(void)
{
    return (uint32_t)xTaskGetTickCount();
}

uint64_t system_get_ns(void)
{
    uint64_t ns = BSP_GetTickMS();
    uint32_t cnt = BSP_TIM4_GetCount();    /* 1 计数 = 1µs（TIM4 为 1MHz） */
    ns *= 1000000;
    ns += (uint64_t)cnt * 1000;
    return ns;
}

unsigned long getRunTimeCounterValue(void)
{
     return (unsigned long)(system_get_ns() / 1000);
}

/* FreeRTOS 运行时间统计钩子（FreeRTOSConfig.h 中引用） */
void configureTimerForRunTimeStats(void)
{
    /* TIM4 已在 BSP_TIM4_Timebase_Init 中配置为 1MHz，无需额外操作 */
}
