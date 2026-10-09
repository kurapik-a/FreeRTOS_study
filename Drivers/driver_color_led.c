// SPDX-License-Identifier: GPL-3.0-only
/*
 * driver_color_led.c - 三色 LED 驱动（标准库版）
 * TIM2：CH1=PA5(绿) CH2=PA1(蓝) CH3=PA2(红)，极性低电平有效
 */
#include "driver_color_led.h"
#include "driver_lcd.h"
#include "driver_timer.h"
#include "stm32f4xx.h"
#include "bsp_tim.h"
#include "FreeRTOS.h"
#include "task.h"

#define CHANNEL_RED   TIM_Channel_3
#define CHANNEL_GREEN TIM_Channel_1
#define CHANNEL_BLUE  TIM_Channel_2

#define COLOR_TIM     TIM2

static void ColorLED_OC_Config(uint8_t ch, int pulse)
{
    TIM_OCInitTypeDef o = {0};
    o.TIM_OCMode      = TIM_OCMode_PWM1;
    o.TIM_OutputState = TIM_OutputState_Enable;
    o.TIM_Pulse       = pulse;
    o.TIM_OCPolarity  = TIM_OCPolarity_Low;
    switch (ch)
    {
    case TIM_Channel_1: TIM_OC1Init(COLOR_TIM, &o); break;
    case TIM_Channel_2: TIM_OC2Init(COLOR_TIM, &o); break;
    case TIM_Channel_3: TIM_OC3Init(COLOR_TIM, &o); break;
    default: break;
    }
}

static void ColorLED_CC_Cmd(uint8_t ch, FunctionalState st)
{
    TIM_CCxCmd(COLOR_TIM, ch, st);
}

void ColorLED_Init(void)
{
    /* TIM2/PWM 引脚已在 BSP_TIM2_PWM_Init 中初始化 */
}

void ColorLED_Set(uint32_t color)
{
    int r, g, b;
    r = (color >> 16) & 0xff;
    g = (color >> 8) & 0xff;
    b = (color >> 0) & 0xff;

    ColorLED_CC_Cmd(CHANNEL_RED, DISABLE);
    ColorLED_CC_Cmd(CHANNEL_GREEN, DISABLE);
    ColorLED_CC_Cmd(CHANNEL_BLUE, DISABLE);

    ColorLED_OC_Config(CHANNEL_RED,   r * 2000 / 255);
    ColorLED_OC_Config(CHANNEL_GREEN, g * 2000 / 255);
    ColorLED_OC_Config(CHANNEL_BLUE,  b * 2000 / 255);

    ColorLED_CC_Cmd(CHANNEL_RED, ENABLE);
    ColorLED_CC_Cmd(CHANNEL_GREEN, ENABLE);
    ColorLED_CC_Cmd(CHANNEL_BLUE, ENABLE);
}

void ColorLED_Test(void)
{
    uint32_t color = 0;
    ColorLED_Init();
    while (1)
    {
        ColorLED_Set(color);
        color += 200000;
        color &= 0x00ffffff;
        vTaskDelay(1000);
    }
}
