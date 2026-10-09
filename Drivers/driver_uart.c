// SPDX-License-Identifier: GPL-3.0-only
/*
 * driver_uart.c - 调试串口驱动（标准库版）
 * USART1 115200-8-N-1；printf 经 fputc 重定向（Keil MicroLIB）
 */
#include "driver_uart.h"
#include "driver_lcd.h"
#include "driver_timer.h"
#include "bsp_usart.h"

#include <stdio.h>

void UART_Init(void)
{
    /* USART1 已在 BSP_USART1_Init 中初始化 */
}

int fputc(int c, FILE *f)
{
    (void)f;
    BSP_USART1_SendByte((uint8_t)c);
    return c;
}

int fgetc(FILE *f)
{
    uint8_t ch = 0;
    (void)f;

    BSP_USART1_RecvByte(&ch);
    BSP_USART1_SendByte(ch);   /* 回显 */
    return ch;
}

void UART_Test(void)
{
    char c;

    UART_Init();

    LCD_PrintString(0, 0, "Use Serial Tool on PC to test UART");

    printf("100ask UART test:\n\r");
    while (1)
    {
        c = fgetc(NULL);
        printf("\n\rget %c\n\r", c);
    }
}
