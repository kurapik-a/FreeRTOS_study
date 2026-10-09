#ifndef __BSP_GPIO_H
#define __BSP_GPIO_H

#include "stm32f4xx.h"

/* 板级 GPIO 初始化（对应 HAL 工程的 MX_GPIO_Init + 各 MspInit 中的 GPIO 部分）
 * 引脚映射（DShanMCU-F407）：
 *   PE2  输出推挽      - SR04 Trig
 *   PE3/PE4 输入       - 按键 / SR04 Echo
 *   PF6/PF7 输入       - 旋转编码器 S2/KEY
 *   PF8  EXTI上升沿    - 旋转编码器 S1
 *   PF9/PF10 输出推挽  - LED
 *   PC0/PC1/PC4 输出推挽 - 预留（PC1 为 W25Q64 片选）
 *   PC2  输出开漏上拉  - DHT11/DS18B20 单总线
 *   PC3  EXTI上升沿    - MPU6050 INT
 *   PA6  EXTI双边沿    - 红外接收
 *   PC5  输入          - 红外避障
 *   PG14 EXTI上升沿    - 预留
 */
void BSP_GPIO_Init(void);

/* 各外设 MspInit 中的 AF 引脚配置（被 bsp_i2c/bsp_spi/bsp_usart/bsp_tim 调用） */
void BSP_GPIO_I2C1_Init(void);
void BSP_GPIO_I2C2_Init(void);
void BSP_GPIO_SPI1_Init(void);
void BSP_GPIO_USART1_Init(void);
void BSP_GPIO_TIM1_Init(void);
void BSP_GPIO_TIM2_Init(void);
void BSP_GPIO_TIM3_Init(void);

#endif
