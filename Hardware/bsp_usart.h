#ifndef __BSP_USART_H
#define __BSP_USART_H

#include "stm32f4xx.h"

/* USART1：PA9-TX / PA10-RX，115200-8-N-1（printf 重定向在 driver_uart.c 的 fputc） */
void BSP_USART1_Init(void);

/* 轮询收发原语（供 driver_uart.c 的 fputc/fgetc 使用） */
void BSP_USART1_SendByte(uint8_t c);
int  BSP_USART1_RecvByte(uint8_t *c);   /* 阻塞等待 RXNE，返回 0 */

#endif
