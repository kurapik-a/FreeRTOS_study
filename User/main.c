/*
 * main.c - STM32F407 标准库 + FreeRTOS 游戏手表
 *
 * 时钟：HSE 25MHz -> PLL(M=25,N=336,P=2) -> 168MHz（启动文件调用
 *       Start/system_stm32f4xx.c 的 SystemInit() 完成配置，main 中无需重复配置）
 * FreeRTOS：tick 1kHz（SysTick 由移植层接管），heap_4，15KB 堆
 */
#include "FreeRTOS.h"
#include "task.h"
#include "bsp_gpio.h"
#include "bsp_i2c.h"
#include "bsp_spi.h"
#include "bsp_usart.h"
#include "bsp_tim.h"
#include "bsp_adc.h"
#include "bsp_exti.h"

extern void MX_FREERTOS_Init(void);

int main(void)
{
    /* 1. 外设初始化（标准库） */
    BSP_GPIO_Init();
    BSP_I2C1_Init();          /* I2C1: OLED + MPU6050（PB8/PB9） */
    BSP_I2C2_Init();          /* I2C2: 预留（PB10/PB11） */
    BSP_SPI1_Init();          /* SPI1: W25Q64（PB3/PB4/PB5） */
    BSP_USART1_Init();        /* USART1: 调试串口 115200（PA9/PA10） */
    BSP_TIM1_PWM_Init();      /* TIM1_CH1 PE9 */
    BSP_TIM2_PWM_Init();      /* TIM2_CH1/2/3 彩灯（PA5/PA1/PA2） */
    BSP_TIM3_PWM_Init();      /* TIM3_CH2 无源蜂鸣器（PA7） */
    BSP_TIM4_Timebase_Init(); /* TIM4 1MHz 时基：udelay / system_get_ns */
    BSP_ADC1_Init();          /* ADC1 通道3 光敏（PA3） */
    BSP_EXTI_NVIC_Init();     /* EXTI 中断：MPU6050/红外/编码器 */

    /* 2. 创建任务与内核对象（原 freertos.c 的 MX_FREERTOS_Init） */
    MX_FREERTOS_Init();

    /* 3. 启动调度器（SysTick/PendSV/SVC 由 FreeRTOS 接管） */
    vTaskStartScheduler();

    /* 正常不会到这里 */
    while (1)
    {
    }
}
