#ifndef __BSP_SPI_H
#define __BSP_SPI_H

#include "stm32f4xx.h"

/* SPI1 主机（PB3/PB4/PB5，NSS 软件管理，APB2 168MHz/4 = 42MHz） */
void BSP_SPI1_Init(void);

/* 等价于 HAL_SPI_TransmitReceive / Transmit / Receive（返回 0 成功，-1 超时） */
int BSP_SPI_TransmitReceive(uint8_t *pTx, uint8_t *pRx, uint16_t len);
int BSP_SPI_Transmit(const uint8_t *pTx, uint16_t len);
int BSP_SPI_Receive(uint8_t *pRx, uint16_t len);

#endif
