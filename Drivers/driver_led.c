// SPDX-License-Identifier: GPL-3.0-only
/*
 * driver_led.c - LED 驱动（标准库版）
 * PF9/PF10 推挽输出，低电平点亮
 */
#include "FreeRTOS.h"
#include "task.h"
#include "driver_led.h"
#include "driver_timer.h"
#include "stdio.h"
#include "stm32f4xx.h"

int Led_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
    GPIO_ResetBits(GPIOF, GPIO_Pin_9 | GPIO_Pin_10);
    GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_9 | GPIO_Pin_10;
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOF, &GPIO_InitStruct);
    return 0;
}

int Led_Control(int which, int on)
{
    if (on)
    {
        //GPIO_ResetBits(GPIOF, GPIO_Pin_9);
        GPIO_ResetBits(GPIOF, GPIO_Pin_10);
    }
    else
        //GPIO_SetBits(GPIOF, GPIO_Pin_9);
        GPIO_SetBits(GPIOF, GPIO_Pin_10);
    return 0;
}

void Led_Test(void *params)
{
    (void)params;   /* FreeRTOS 任务函数统一签名，此处不使用参数 */
    UBaseType_t freeNum;
    TaskHandle_t xTaskHandle;
    Led_Init();
    while (1)
    {
        xTaskHandle = xTaskGetCurrentTaskHandle();
        freeNum = uxTaskGetStackHighWaterMark(xTaskHandle);
        printf("FreeStack of Task %s : %d\n\r", pcTaskGetName(xTaskHandle), freeNum);
        Led_Control(LED_Green, 1);
        vTaskDelay(500);
        Led_Control(LED_Green, 0);
        vTaskDelay(500);
    }
}
