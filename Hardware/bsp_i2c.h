#ifndef __BSP_I2C_H
#define __BSP_I2C_H

#include "stm32f4xx.h"

/* 标准库轮询方式 I2C 主机驱动（对应 HAL_I2C_Master_Transmit / Mem_Write / Mem_Read）
 * 返回值：0 成功，-1 超时/错误
 * 地址为 8bit 格式（7bit 地址左移 1 位，如 OLED 0x78、MPU6050 0xD0） */
int BSP_I2C_MasterWrite(I2C_TypeDef *i2c, uint8_t addr, const uint8_t *buf, uint16_t len);
int BSP_I2C_MemWrite(I2C_TypeDef *i2c, uint8_t addr, uint8_t mem, const uint8_t *buf, uint16_t len);
int BSP_I2C_MemRead(I2C_TypeDef *i2c, uint8_t addr, uint8_t mem, uint8_t *buf, uint16_t len);

void BSP_I2C1_Init(void);   /* PB8/PB9, 100kHz */
void BSP_I2C2_Init(void);   /* PB10/PB11, 100kHz */

#endif
