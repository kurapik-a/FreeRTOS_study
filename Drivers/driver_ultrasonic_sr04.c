// SPDX-License-Identifier: GPL-3.0-only
/*
 * driver_ultrasonic_sr04.c - 超声波测距驱动（标准库版）
 * Trig = PE2 推挽输出，Echo = PE4 输入，测回波脉宽（ns 级时间戳）
 * 注：原版未把回波时间换算成距离写入 *pData，此处保持一致。
 */
#include "driver_ultrasonic_sr04.h"
#include "driver_lcd.h"
#include "driver_timer.h"
#include "stm32f4xx.h"

#define SR04_TRIG_GPIO_GROUP GPIOE
#define SR04_TRIG_GPIO_PIN   GPIO_Pin_2
#define SR04_ECHO_GPIO_GROUP GPIOE
#define SR04_ECHO_GPIO_PIN   GPIO_Pin_4
#define SR04_TIMEOUT   20000 // us

void SR04_Init(void)
{
    /* PE2/PE4 已在 BSP_GPIO_Init 中配置 */
}

int SR04_Read(uint32_t *pData)
{
    uint32_t timeout = SR04_TIMEOUT;
    uint64_t time;
    GPIO_WriteBit(SR04_TRIG_GPIO_GROUP, SR04_TRIG_GPIO_PIN, Bit_SET);
    udelay(20);
    GPIO_WriteBit(SR04_TRIG_GPIO_GROUP, SR04_TRIG_GPIO_PIN, Bit_RESET);
    while (timeout-- && (GPIO_ReadInputDataBit(SR04_ECHO_GPIO_GROUP, SR04_ECHO_GPIO_PIN) == Bit_RESET))
    {
        udelay(1);
    }
    if (!timeout)
    {
        return -1;
    }
    time = system_get_ns();
    timeout = SR04_TIMEOUT;
    while (timeout-- && (GPIO_ReadInputDataBit(SR04_ECHO_GPIO_GROUP, SR04_ECHO_GPIO_PIN) == Bit_SET))
    {
        udelay(1);
    }
    if (!timeout)
    {
        return -1;
    }
    time  = system_get_ns() - time;
    (void)time;   /* 原版未做距离换算，保持一致 */
    return 0;
}

void SR04_Test(void)
{
    uint32_t distance;
    int len;
    SR04_Init();
    while (1)
    {
        LCD_PrintString(0, 0, "SR04 Distance: ");
        if (!SR04_Read(&distance))
        {
            LCD_ClearLine(0, 2);
            len = LCD_PrintSignedVal(0, 2, distance);
            LCD_PrintString(len, 2, "cm");
        }
        else
        {
            LCD_ClearLine(0, 2);
            LCD_PrintString(0, 2, "err");
         }
    }
}
