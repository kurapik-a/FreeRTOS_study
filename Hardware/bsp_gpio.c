/*
 * bsp_gpio.c - 板级 GPIO 初始化（标准库实现）
 * 对应 HAL 工程 Core/Src/gpio.c 的 MX_GPIO_Init，
 * 以及 i2c.c/spi.c/usart.c/tim.c 各 MspInit 中的 GPIO 配置。
 */
#include "bsp_gpio.h"

/* 与 HAL GPIO_SPEED_FREQ 对应的标准库速度 */
#define BSP_SPEED_LOW        GPIO_Speed_2MHz      /* HAL: GPIO_SPEED_FREQ_LOW */
#define BSP_SPEED_VERY_HIGH  GPIO_Speed_100MHz    /* HAL: GPIO_SPEED_FREQ_VERY_HIGH */

static void BSP_GPIO_OutPP(GPIO_TypeDef *port, uint16_t pins)
{
    GPIO_InitTypeDef g;
    g.GPIO_Pin  = pins;
    g.GPIO_Mode = GPIO_Mode_OUT;
    g.GPIO_OType = GPIO_OType_PP;
    g.GPIO_PuPd = GPIO_PuPd_NOPULL;
    g.GPIO_Speed = BSP_SPEED_LOW;
    GPIO_Init(port, &g);
}

static void BSP_GPIO_In(GPIO_TypeDef *port, uint16_t pins, GPIOPuPd_TypeDef pupd)
{
    GPIO_InitTypeDef g;
    g.GPIO_Pin  = pins;
    g.GPIO_Mode = GPIO_Mode_IN;
    g.GPIO_OType = GPIO_OType_PP;
    g.GPIO_PuPd = pupd;
    g.GPIO_Speed = BSP_SPEED_LOW;
    GPIO_Init(port, &g);
}

static void BSP_GPIO_Exti(GPIO_TypeDef *port, uint16_t pin, uint8_t pin_src,
                          uint8_t port_src, GPIOPuPd_TypeDef pupd,
                          EXTITrigger_TypeDef trig)
{
    GPIO_InitTypeDef g;
    g.GPIO_Pin  = pin;
    g.GPIO_Mode = GPIO_Mode_IN;            /* 标准库中 EXTI 输入即 GPIO_Mode_IN + EXTI 配置 */
    g.GPIO_OType = GPIO_OType_PP;
    g.GPIO_PuPd = pupd;
    g.GPIO_Speed = BSP_SPEED_LOW;
    GPIO_Init(port, &g);

    /* 连接到 EXTI 线 */
    SYSCFG_EXTILineConfig((uint8_t)port_src, pin_src);
    {
        EXTI_InitTypeDef e;
        e.EXTI_Line    = pin;              /* EXTI_Line_x == GPIO_Pin_x 掩码一致 */
        e.EXTI_Mode    = EXTI_Mode_Interrupt;
        e.EXTI_Trigger = trig;
        e.EXTI_LineCmd = ENABLE;
        EXTI_Init(&e);
    }
}

/* AF 复用引脚 */
static void BSP_GPIO_AF(GPIO_TypeDef *port, uint16_t pins, uint8_t afsrc, GPIOSpeed_TypeDef speed)
{
    GPIO_InitTypeDef g;
    uint8_t i;
    g.GPIO_Pin  = pins;
    g.GPIO_Mode = GPIO_Mode_AF;
    g.GPIO_OType = GPIO_OType_PP;
    g.GPIO_PuPd = GPIO_PuPd_NOPULL;
    g.GPIO_Speed = speed;
    GPIO_Init(port, &g);
    for (i = 0; i < 16; i++)
    {
        if (pins & (1u << i))
            GPIO_PinAFConfig(port, i, afsrc);
    }
}

void BSP_GPIO_Init(void)
{
    GPIO_InitTypeDef g;

    /* 时钟使能（对应 HAL 的 __HAL_RCC_GPIOx_CLK_ENABLE） */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB |
                           RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOE |
                           RCC_AHB1Periph_GPIOF | RCC_AHB1Periph_GPIOG, ENABLE);
    /* EXTI 多路复用在 SYSCFG 中，必须先开其时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    /* ---- 初始输出电平（对应 HAL_GPIO_WritePin） ---- */
    GPIO_ResetBits(GPIOE, GPIO_Pin_2);                              /* SR04 Trig 拉低 */
    GPIO_ResetBits(GPIOF, GPIO_Pin_9 | GPIO_Pin_10);                /* LED 熄灭 */
    GPIO_ResetBits(GPIOC, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_4);

    /* ---- 普通输入 ---- */
    BSP_GPIO_In(GPIOE, GPIO_Pin_3 | GPIO_Pin_4, GPIO_PuPd_NOPULL);  /* 按键 / SR04 Echo */
    BSP_GPIO_In(GPIOF, GPIO_Pin_6 | GPIO_Pin_7, GPIO_PuPd_NOPULL);  /* 编码器 S2/KEY */
    BSP_GPIO_In(GPIOC, GPIO_Pin_5, GPIO_PuPd_NOPULL);               /* 红外避障 */

    /* ---- EXTI 输入 ---- */
    BSP_GPIO_Exti(GPIOF, GPIO_Pin_8, 8, EXTI_PortSourceGPIOF, GPIO_PuPd_UP, EXTI_Trigger_Rising);          /* 编码器 S1 */
    BSP_GPIO_Exti(GPIOC, GPIO_Pin_3, 3, EXTI_PortSourceGPIOC, GPIO_PuPd_UP, EXTI_Trigger_Rising);          /* MPU6050 INT */
    BSP_GPIO_Exti(GPIOA, GPIO_Pin_6, 6, EXTI_PortSourceGPIOA, GPIO_PuPd_UP, EXTI_Trigger_Rising_Falling);  /* 红外接收 */
    BSP_GPIO_Exti(GPIOG, GPIO_Pin_14, 14, EXTI_PortSourceGPIOG, GPIO_PuPd_UP, EXTI_Trigger_Rising);        /* 预留 */

    /* ---- 输出 ---- */
    BSP_GPIO_OutPP(GPIOE, GPIO_Pin_2);                              /* SR04 Trig */
    BSP_GPIO_OutPP(GPIOF, GPIO_Pin_9 | GPIO_Pin_10);                /* LED */
    BSP_GPIO_OutPP(GPIOC, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4);    /* PC1=W25Q64 CS */

    /* PC2: 开漏 + 上拉（DHT11/DS18B20 单总线） */
    g.GPIO_Pin  = GPIO_Pin_2;
    g.GPIO_Mode = GPIO_Mode_OUT;
    g.GPIO_OType = GPIO_OType_OD;
    g.GPIO_PuPd = GPIO_PuPd_UP;
    g.GPIO_Speed = BSP_SPEED_LOW;
    GPIO_Init(GPIOC, &g);
}

/* ================= 以下为各外设 MspInit 中的 AF 引脚配置 ================= */

void BSP_GPIO_I2C1_Init(void)   /* PB8/PB9 -> AF4, 开漏 */
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    {
        GPIO_InitTypeDef g;
        g.GPIO_Pin  = GPIO_Pin_8 | GPIO_Pin_9;
        g.GPIO_Mode = GPIO_Mode_AF;
        g.GPIO_OType = GPIO_OType_OD;
        g.GPIO_PuPd = GPIO_PuPd_NOPULL;
        g.GPIO_Speed = BSP_SPEED_VERY_HIGH;
        GPIO_Init(GPIOB, &g);
        GPIO_PinAFConfig(GPIOB, 8, GPIO_AF_I2C1);
        GPIO_PinAFConfig(GPIOB, 9, GPIO_AF_I2C1);
    }
}

void BSP_GPIO_I2C2_Init(void)   /* PB10/PB11 -> AF4, 开漏 */
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    {
        GPIO_InitTypeDef g;
        g.GPIO_Pin  = GPIO_Pin_10 | GPIO_Pin_11;
        g.GPIO_Mode = GPIO_Mode_AF;
        g.GPIO_OType = GPIO_OType_OD;
        g.GPIO_PuPd = GPIO_PuPd_NOPULL;
        g.GPIO_Speed = BSP_SPEED_VERY_HIGH;
        GPIO_Init(GPIOB, &g);
        GPIO_PinAFConfig(GPIOB, 10, GPIO_AF_I2C2);
        GPIO_PinAFConfig(GPIOB, 11, GPIO_AF_I2C2);
    }
}

void BSP_GPIO_SPI1_Init(void)   /* PB3/PB4/PB5 -> AF5, 推挽 */
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    BSP_GPIO_AF(GPIOB, GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5, GPIO_AF_SPI1, BSP_SPEED_VERY_HIGH);
}

void BSP_GPIO_USART1_Init(void) /* PA9/PA10 -> AF7, 推挽 */
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    BSP_GPIO_AF(GPIOA, GPIO_Pin_9 | GPIO_Pin_10, GPIO_AF_USART1, BSP_SPEED_VERY_HIGH);
}

void BSP_GPIO_TIM1_Init(void)   /* PE9 -> AF1 TIM1_CH1 */
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
    BSP_GPIO_AF(GPIOE, GPIO_Pin_9, GPIO_AF_TIM1, BSP_SPEED_LOW);
}

void BSP_GPIO_TIM2_Init(void)   /* PA1/PA2/PA5 -> AF1 TIM2 CH2/CH3/CH1 */
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    BSP_GPIO_AF(GPIOA, GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_5, GPIO_AF_TIM2, BSP_SPEED_LOW);
}

void BSP_GPIO_TIM3_Init(void)   /* PA7 -> AF2 TIM3_CH2 */
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    BSP_GPIO_AF(GPIOA, GPIO_Pin_7, GPIO_AF_TIM3, BSP_SPEED_LOW);
}
