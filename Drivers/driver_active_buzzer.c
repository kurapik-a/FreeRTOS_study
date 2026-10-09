// SPDX-License-Identifier: GPL-3.0-only
/*
 * driver_active_buzzer.c - 有源蜂鸣器驱动（标准库版）
 * PA7 推挽输出，低电平响（与 TIM3_CH2 的无源蜂鸣器共用引脚，二选一使用）
 */
#include "driver_active_buzzer.h"
#include "driver_lcd.h"
#include "driver_timer.h"
#include "stm32f4xx.h"

void ActiveBuzzer_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_7;
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void ActiveBuzzer_Control(int on)
{
    if (on)
        GPIO_ResetBits(GPIOA, GPIO_Pin_7);
    else
        GPIO_SetBits(GPIOA, GPIO_Pin_7);
}

void ActiveBuzzer_Test(void)
{
    ActiveBuzzer_Init();
    while (1)
    {
        LCD_PrintString(0, 0, "ActiveBuzzer ON ");
        ActiveBuzzer_Control(1);
        mdelay(1000);
        LCD_PrintString(0, 0, "ActiveBuzzer OFF");
        ActiveBuzzer_Control(0);
        mdelay(1000);
    }
}
