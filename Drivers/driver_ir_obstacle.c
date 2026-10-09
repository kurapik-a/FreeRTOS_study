// SPDX-License-Identifier: GPL-3.0-only
/*
 * driver_ir_obstacle.c - 红外避障驱动（标准库版）
 * PC5 输入，检测到障碍输出低电平
 */
#include "driver_ir_obstacle.h"
#include "driver_lcd.h"
#include "driver_timer.h"
#include "stm32f4xx.h"

#define IROBSTACLE_GPIO_GROUP GPIOC
#define IROBSTACLE_GPIO_PIN   GPIO_Pin_5

void IRObstacle_Init(void)
{
    /* PC5 已在 BSP_GPIO_Init 中配置为输入 */
}

int IRObstacle_Read(void)
{
    if (Bit_RESET == GPIO_ReadInputDataBit(IROBSTACLE_GPIO_GROUP, IROBSTACLE_GPIO_PIN))
        return 0;
    else
        return 1;
}

void IRObstacle_Test(void)
{
    IRObstacle_Init();
    while (1)
    {
        LCD_PrintString(0, 0, "IRObstacle: ");
        if (!IRObstacle_Read())
        {
            LCD_PrintString(0, 2, "Crashing");
        }
        else
        {
            LCD_PrintString(0, 2, "Safe      ");
        }
    }
}
