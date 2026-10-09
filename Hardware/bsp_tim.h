#ifndef __BSP_TIM_H
#define __BSP_TIM_H

#include "stm32f4xx.h"

/* TIM1(CH1=PE9) / TIM2(CH1=PA5,CH2=PA1,CH3=PA2) / TIM3(CH2=PA7) PWM 初始化
 * 参数与 HAL 工程 MX_TIMx_Init 完全一致 */
void BSP_TIM1_PWM_Init(void);
void BSP_TIM2_PWM_Init(void);
void BSP_TIM3_PWM_Init(void);

/* TIM4 时基：1MHz 自由计数（PSC=83, ARR=999），不产生中断
 * 提供 µs 级延时基准与 ns 时间戳（配合 FreeRTOS tick 的 ms 部分） */
void BSP_TIM4_Timebase_Init(void);
uint32_t BSP_TIM4_GetCount(void);
uint32_t BSP_TIM4_GetReload(void);

#endif
