#ifndef __BSP_EXTI_H
#define __BSP_EXTI_H

#include "stm32f4xx.h"

/* EXTI 中断优先级与使能（引脚配置在 bsp_gpio.c 中完成）
 * EXTI3     : PC3 MPU6050 INT   -> 优先级 5,0
 * EXTI9_5   : PA6 红外接收 + PF8 编码器 S1 -> 优先级 5,0
 * EXTI15_10 : PG14 预留          -> 优先级 5,0
 * 注意：优先级数值 5 == configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY，
 *       这些 ISR 中可以调用 FreeRTOS 的 FromISR API。 */
void BSP_EXTI_NVIC_Init(void);

#endif
