// SPDX-License-Identifier: GPL-3.0-only
/*
 * driver_passive_buzzer.c - 无源蜂鸣器驱动（标准库版）
 * TIM3_CH2（PA7，AF2），变频方式：固定 PSC=71，动态改 ARR。
 * 极性为低（硬件上低电平鸣叫）。
 */
#include "driver_passive_buzzer.h"
#include "driver_lcd.h"
#include "driver_timer.h"
#include "stm32f4xx.h"
#include "bsp_tim.h"
#include "bsp_gpio.h"

#define BUZZER_TIM      TIM3
#define BUZZER_CHANNEL  TIM_Channel_2

static void Buzzer_OC_Config(int pulse)
{
    TIM_OCInitTypeDef o = {0};
    o.TIM_OCMode      = TIM_OCMode_PWM1;
    o.TIM_OutputState = TIM_OutputState_Enable;
    o.TIM_Pulse       = pulse;
    o.TIM_OCPolarity  = TIM_OCPolarity_Low;
    o.TIM_OCIdleState = TIM_OCIdleState_Reset;
    TIM_OC2Init(BUZZER_TIM, &o);
    TIM_OC2PreloadConfig(BUZZER_TIM, TIM_OCPreload_Disable);
}

static void Buzzer_TIM_Config(int period)
{
    TIM_TimeBaseInitTypeDef t = {0};
    t.TIM_Prescaler         = 71;          /* 84M/72 = 1.1667MHz（与 HAL 版一致） */
    t.TIM_CounterMode       = TIM_CounterMode_Up;
    t.TIM_Period            = period;
    t.TIM_ClockDivision     = TIM_CKD_DIV1;
    t.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(BUZZER_TIM, &t);
    TIM_ARRPreloadConfig(BUZZER_TIM, ENABLE);
}

void PassiveBuzzer_Init(void)
{
    /* PA7 复用为 TIM3_CH2（覆盖 MX 阶段的同引脚配置） */
    BSP_GPIO_TIM3_Init();

    Buzzer_TIM_Config(999);          /* 初始约 1kHz */
    Buzzer_OC_Config(499);           /* 50% 占空比 */
    /* 注意：此处不使能计数器，与 HAL 版行为一致（等 Control(1) 再启动） */
}

void PassiveBuzzer_Control(int on)
{
    if (on)
    {
        TIM_Cmd(BUZZER_TIM, ENABLE);
        TIM_CCxCmd(BUZZER_TIM, BUZZER_CHANNEL, TIM_CCx_Enable);
    }
    else
    {
        TIM_CCxCmd(BUZZER_TIM, BUZZER_CHANNEL, TIM_CCx_Disable);
    }
}

void PassiveBuzzer_Set_Freq_Duty(int freq, int duty)
{
    int period = 1000000 / freq - 1;

    PassiveBuzzer_Control(0);
    Buzzer_TIM_Config(period);
    Buzzer_OC_Config(period * duty / 100);
    /* 立即装载新 ARR 并启动 */
    TIM_GenerateEvent(BUZZER_TIM, TIM_EventSource_Update);
    PassiveBuzzer_Control(1);
}

void PassiveBuzzer_Test(void)
{
    PassiveBuzzer_Init();
    LCD_Init();
    LCD_Clear();
    while (1)
    {
        LCD_PrintString(0, 0, "PassiveBuzzer");
        LCD_PrintString(0, 2, "ON ");
        PassiveBuzzer_Control(1);
        mdelay(1000);
        LCD_PrintString(0, 0, "PassiveBuzzer");
        LCD_PrintString(0, 2, "OFF");
        PassiveBuzzer_Control(0);
        mdelay(1000);
    }
}
