/*
 * bsp_exti.c - EXTI 中断服务与分发（标准库实现）
 * 保持与 HAL 工程相同的分发结构：EXTI IRQHandler -> HAL_GPIO_EXTI_Callback(pin)
 * （HAL_GPIO_EXTI_Callback 在 Drivers/driver_irq.c 中按引脚分发给各驱动）
 */
#include "bsp_exti.h"

/* 引脚分发回调：实现在 Drivers/driver_irq.c */
extern void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

/* 优先级分组 4：4 位全部作为抢占优先级（与 HAL 默认一致） */
void BSP_EXTI_NVIC_Init(void)
{
    NVIC_InitTypeDef n;

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    n.NVIC_IRQChannel                   = EXTI3_IRQn;
    n.NVIC_IRQChannelPreemptionPriority = 5;
    n.NVIC_IRQChannelSubPriority        = 0;
    n.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&n);

    n.NVIC_IRQChannel                   = EXTI9_5_IRQn;
    NVIC_Init(&n);

    n.NVIC_IRQChannel                   = EXTI15_10_IRQn;
    NVIC_Init(&n);
}

/* EXTI 线 3：MPU6050 INT（PC3） */
void EXTI3_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line3) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line3);
        HAL_GPIO_EXTI_Callback(GPIO_Pin_3);
    }
}

/* EXTI 线 [9:5]：PA6 红外接收（线 6） + PF8 编码器（线 8） */
void EXTI9_5_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line6) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line6);
        HAL_GPIO_EXTI_Callback(GPIO_Pin_6);
    }
    if (EXTI_GetITStatus(EXTI_Line8) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line8);
        HAL_GPIO_EXTI_Callback(GPIO_Pin_8);
    }
}

/* EXTI 线 [15:10]：PG14 预留 */
void EXTI15_10_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line14) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line14);
        HAL_GPIO_EXTI_Callback(GPIO_Pin_14);
    }
}
