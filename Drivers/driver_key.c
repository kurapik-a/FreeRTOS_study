// SPDX-License-Identifier: GPL-3.0-only
/*
 * driver_key.c - 按键驱动（标准库版）
 * PE3 输入，按下为低电平
 */
#include "driver_key.h"
#include "driver_led.h"
#include "driver_lcd.h"
#include "stm32f4xx.h"

#define KEY_GPIO_GROUP GPIOE
#define KEY_GPIO_PIN   GPIO_Pin_3

void Key_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
    GPIO_InitStruct.GPIO_Pin  = KEY_GPIO_PIN;
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(KEY_GPIO_GROUP, &GPIO_InitStruct);
}

int Key_Read(void)
{
    if (Bit_RESET == GPIO_ReadInputDataBit(KEY_GPIO_GROUP, KEY_GPIO_PIN))
        return 1;
    else
        return 0;
}

void Key_Test(void)
{
    int len;
    int val;
    Key_Init();
    Led_Init();
    while (1)
    {
        LCD_PrintString(0, 0, "Key Test: ");
        val = Key_Read();
        Led_Control(LED_Green, val);
        len = LCD_PrintString(0, 2, "Key : ");
        len += LCD_PrintString(len, 2, val ? "Pressed":"Released");
        LCD_ClearLine(len, 2);
    }
}
