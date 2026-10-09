/*
 * bsp_tim.c - 定时器初始化（标准库实现）
 * 时钟说明（HSE 25MHz -> 168MHz）：
 *   TIM1 挂 APB2，定时器时钟 168MHz
 *   TIM2/3/4 挂 APB1（42MHz，4 分频），定时器时钟 ×2 = 84MHz
 * 各定时器 PSC/ARR 与 HAL 工程 MX_TIMx_Init 完全一致。
 * 注：TIM9（红外发射用 38kHz 载波）在 driver_ir_sender.c 内部初始化。
 */
#include "bsp_tim.h"
#include "bsp_gpio.h"

void BSP_TIM1_PWM_Init(void)
{
    TIM_TimeBaseInitTypeDef t;
    TIM_OCInitTypeDef o;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
    BSP_GPIO_TIM1_Init();

    t.TIM_Prescaler         = 71;       /* 168M/72 = 2.333MHz */
    t.TIM_CounterMode       = TIM_CounterMode_Up;
    t.TIM_Period            = 999;      /* 2333Hz */
    t.TIM_ClockDivision     = TIM_CKD_DIV1;
    t.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM1, &t);

    o.TIM_OCMode      = TIM_OCMode_PWM1;
    o.TIM_OutputState = TIM_OutputState_Disable;   /* 等价 HAL：未 Start 前无输出 */
    o.TIM_OutputNState = TIM_OutputNState_Disable;
    o.TIM_Pulse       = 499;
    o.TIM_OCPolarity  = TIM_OCPolarity_High;
    o.TIM_OCNPolarity = TIM_OCNPolarity_High;
    o.TIM_OCIdleState = TIM_OCIdleState_Reset;
    o.TIM_OCNIdleState = TIM_OCNIdleState_Reset;
    TIM_OC1Init(TIM1, &o);
    TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Disable);
    TIM_ARRPreloadConfig(TIM1, DISABLE);

    TIM_Cmd(TIM1, ENABLE);
    TIM_CtrlPWMOutputs(TIM1, ENABLE);   /* 高级定时器必须使能 MOE */
}

void BSP_TIM2_PWM_Init(void)
{
    TIM_TimeBaseInitTypeDef t;
    TIM_OCInitTypeDef o;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    BSP_GPIO_TIM2_Init();

    t.TIM_Prescaler         = 359;      /* 84M/360 = 233.3kHz */
    t.TIM_CounterMode       = TIM_CounterMode_Up;
    t.TIM_Period            = 1999;     /* 116.7Hz，彩灯 PWM */
    t.TIM_ClockDivision     = TIM_CKD_DIV1;
    t.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM2, &t);
    TIM_ARRPreloadConfig(TIM2, ENABLE); /* 对应 AutoReloadPreload ENABLE */

    o.TIM_OCMode      = TIM_OCMode_PWM1;
    o.TIM_OutputState = TIM_OutputState_Disable;   /* 等价 HAL：由 ColorLED_Set 使能 */
    o.TIM_Pulse       = 0;
    o.TIM_OCPolarity  = TIM_OCPolarity_High;
    TIM_OC1Init(TIM2, &o);
    TIM_OC2Init(TIM2, &o);
    TIM_OC3Init(TIM2, &o);
    TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Disable);
    TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Disable);
    TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Disable);

    TIM_Cmd(TIM2, ENABLE);
}

void BSP_TIM3_PWM_Init(void)
{
    TIM_TimeBaseInitTypeDef t;
    TIM_OCInitTypeDef o;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    BSP_GPIO_TIM3_Init();

    t.TIM_Prescaler         = 71;       /* 84M/72 = 1.1667MHz（与 HAL 工程一致） */
    t.TIM_CounterMode       = TIM_CounterMode_Up;
    t.TIM_Period            = 999;
    t.TIM_ClockDivision     = TIM_CKD_DIV1;
    t.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM3, &t);

    o.TIM_OCMode      = TIM_OCMode_PWM1;
    o.TIM_OutputState = TIM_OutputState_Disable;   /* 等价 HAL：由 PassiveBuzzer 使能 */
    o.TIM_Pulse       = 499;
    o.TIM_OCPolarity  = TIM_OCPolarity_High;   /* 极性LOW由 PassiveBuzzer_Init 运行时再配置 */
    TIM_OC2Init(TIM3, &o);
    TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Disable);
    TIM_ARRPreloadConfig(TIM3, DISABLE);

    TIM_Cmd(TIM3, ENABLE);
}

void BSP_TIM4_Timebase_Init(void)
{
    TIM_TimeBaseInitTypeDef t;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

    t.TIM_Prescaler         = 83;       /* 84M/84 = 1MHz，1 个计数 = 1µs */
    t.TIM_CounterMode       = TIM_CounterMode_Up;
    t.TIM_Period            = 999;      /* 1000 计数 = 1ms */
    t.TIM_ClockDivision     = TIM_CKD_DIV1;
    t.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM4, &t);
    TIM_ARRPreloadConfig(TIM4, ENABLE);
    TIM_Cmd(TIM4, ENABLE);
    /* 不使能更新中断：ms 部分由 FreeRTOS tick（SysTick）提供 */
}

uint32_t BSP_TIM4_GetCount(void)
{
    return TIM_GetCounter(TIM4);
}

uint32_t BSP_TIM4_GetReload(void)
{
    return TIM4->ARR;   /* F4 标准库无 TIM_GetAutoreload，直接读寄存器 */
}
