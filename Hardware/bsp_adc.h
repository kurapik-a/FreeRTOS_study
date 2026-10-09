#ifndef __BSP_ADC_H
#define __BSP_ADC_H

#include "stm32f4xx.h"

/* ADC1：PA3（通道 3），12 位，软件触发单次转换，3 周期采样（同 HAL 工程） */
void BSP_ADC1_Init(void);
int  BSP_ADC1_Read(uint16_t *val);   /* 0 成功，-1 超时 */

#endif
