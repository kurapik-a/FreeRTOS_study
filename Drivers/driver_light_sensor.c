// SPDX-License-Identifier: GPL-3.0-only
/*
 * driver_light_sensor.c - 光敏电阻驱动（标准库版）
 * PA3 -> ADC1 通道 3，软件触发单次转换
 * 注意：与原 HAL 版行为保持一致——Read 只启动转换并等待完成，
 *       原版并未把转换结果写入 *pData（保留原样，避免行为变化）。
 */
#include "driver_light_sensor.h"
#include "driver_lcd.h"
#include "driver_timer.h"
#include "stm32f4xx.h"
#include "bsp_adc.h"

#define LIGHT_SENSOR_ADC_TIMEOUT 500

void LightSensor_Init(void)
{
    /* ADC1 在 BSP_ADC1_Init 中统一初始化 */
}

int LightSensor_Read(uint32_t *pData)
{
    uint16_t val;
    if (0 == BSP_ADC1_Read(&val))
    {
        /* 原版未回填 pData，此处同样只返回状态 */
        return 0;
    }
    return -1;
}

void LightSensor_Test(void)
{
    uint32_t val;
    LightSensor_Init();
    while (1)
    {
        LCD_PrintString(0, 0, "Light Sensor: ");
        if (!LightSensor_Read(&val))
        {
            LCD_PrintSignedVal(0, 2, val);
        }
        mdelay(500);
    }
}
