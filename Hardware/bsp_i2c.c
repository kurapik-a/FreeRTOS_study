/*
 * bsp_i2c.c - I2C1/I2C2 主机轮询驱动（标准库实现）
 * 等价于 HAL_I2C_Master_Transmit / HAL_I2C_Mem_Write / HAL_I2C_Mem_Read
 * （memAddrSize 固定 1 字节，与 OLED/MPU6050 的用法一致）
 */
#include "bsp_i2c.h"
#include "bsp_gpio.h"

/* 超时计数：168MHz 下约几百 ms 量级 */
#define I2C_TMO     2000000u

static int i2c_wait_event(I2C_TypeDef *i2c, uint32_t event)
{
    uint32_t tmo = I2C_TMO;
    while (!I2C_CheckEvent(i2c, event))
    {
        if (--tmo == 0) return -1;
    }
    return 0;
}

static int i2c_wait_flag(uint32_t flag, FlagStatus status, I2C_TypeDef *i2c)
{
    uint32_t tmo = I2C_TMO;
    while (I2C_GetFlagStatus(i2c, flag) != status)
    {
        if (--tmo == 0) return -1;
    }
    return 0;
}

/* 超时后的简单恢复：发 STOP、等总线释放 */
static int i2c_recover(I2C_TypeDef *i2c)
{
    uint32_t tmo = I2C_TMO;
    I2C_GenerateSTOP(i2c, ENABLE);
    while (I2C_GetFlagStatus(i2c, I2C_FLAG_BUSY) != RESET)
    {
        if (--tmo == 0) return -1;
    }
    return -1;   /* 告知调用方本次传输失败 */
}

int BSP_I2C_MasterWrite(I2C_TypeDef *i2c, uint8_t addr, const uint8_t *buf, uint16_t len)
{
    uint16_t i;

    if (i2c_wait_flag(I2C_FLAG_BUSY, RESET, i2c)) return i2c_recover(i2c);
    I2C_GenerateSTART(i2c, ENABLE);
    if (i2c_wait_event(i2c, I2C_EVENT_MASTER_MODE_SELECT)) return i2c_recover(i2c);

    I2C_Send7bitAddress(i2c, addr, I2C_Direction_Transmitter);
    if (i2c_wait_event(i2c, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) return i2c_recover(i2c);

    for (i = 0; i < len; i++)
    {
        I2C_SendData(i2c, buf[i]);
        if (i2c_wait_event(i2c, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) return i2c_recover(i2c);
    }

    I2C_GenerateSTOP(i2c, ENABLE);
    if (i2c_wait_flag(I2C_FLAG_BUSY, RESET, i2c)) return -1;
    return 0;
}

int BSP_I2C_MemWrite(I2C_TypeDef *i2c, uint8_t addr, uint8_t mem, const uint8_t *buf, uint16_t len)
{
    uint16_t i;

    if (i2c_wait_flag(I2C_FLAG_BUSY, RESET, i2c)) return i2c_recover(i2c);
    I2C_GenerateSTART(i2c, ENABLE);
    if (i2c_wait_event(i2c, I2C_EVENT_MASTER_MODE_SELECT)) return i2c_recover(i2c);

    I2C_Send7bitAddress(i2c, addr, I2C_Direction_Transmitter);
    if (i2c_wait_event(i2c, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) return i2c_recover(i2c);

    I2C_SendData(i2c, mem);                             /* 寄存器/内存地址 */
    if (i2c_wait_event(i2c, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) return i2c_recover(i2c);

    for (i = 0; i < len; i++)
    {
        I2C_SendData(i2c, buf[i]);
        if (i2c_wait_event(i2c, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) return i2c_recover(i2c);
    }

    I2C_GenerateSTOP(i2c, ENABLE);
    if (i2c_wait_flag(I2C_FLAG_BUSY, RESET, i2c)) return -1;
    return 0;
}

int BSP_I2C_MemRead(I2C_TypeDef *i2c, uint8_t addr, uint8_t mem, uint8_t *buf, uint16_t len)
{
    uint16_t i;

    if (len == 0) return 0;
    if (i2c_wait_flag(I2C_FLAG_BUSY, RESET, i2c)) return i2c_recover(i2c);

    /* 第一段：写出寄存器地址 */
    I2C_GenerateSTART(i2c, ENABLE);
    if (i2c_wait_event(i2c, I2C_EVENT_MASTER_MODE_SELECT)) return i2c_recover(i2c);
    I2C_Send7bitAddress(i2c, addr, I2C_Direction_Transmitter);
    if (i2c_wait_event(i2c, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) return i2c_recover(i2c);
    I2C_SendData(i2c, mem);
    if (i2c_wait_event(i2c, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) return i2c_recover(i2c);

    /* 第二段：重复起始 + 读 */
    I2C_GenerateSTART(i2c, ENABLE);
    if (i2c_wait_event(i2c, I2C_EVENT_MASTER_MODE_SELECT)) return i2c_recover(i2c);
    I2C_Send7bitAddress(i2c, addr, I2C_Direction_Receiver);
    if (i2c_wait_event(i2c, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED)) return i2c_recover(i2c);

    if (len == 1)
    {
        /* 单字节：先关 ACK + 预发 STOP，再收最后一个字节 */
        I2C_AcknowledgeConfig(i2c, DISABLE);
        I2C_GenerateSTOP(i2c, ENABLE);
        if (i2c_wait_flag(I2C_FLAG_RXNE, SET, i2c)) return i2c_recover(i2c);
        buf[0] = I2C_ReceiveData(i2c);
    }
    else
    {
        for (i = 0; i < len; i++)
        {
            if (i == len - 1)
            {
                /* 收最后一个字节前关闭 ACK 并准备 STOP */
                I2C_AcknowledgeConfig(i2c, DISABLE);
                I2C_GenerateSTOP(i2c, ENABLE);
            }
            if (i2c_wait_flag(I2C_FLAG_RXNE, SET, i2c)) return i2c_recover(i2c);
            buf[i] = I2C_ReceiveData(i2c);
        }
    }

    /* 等 STOP 完成，恢复 ACK */
    if (i2c_wait_flag(I2C_FLAG_STOPF, SET, i2c)) { /* STOPF 由读 SR1/SR2 自动清除 */ }
    I2C_AcknowledgeConfig(i2c, ENABLE);
    return 0;
}

void BSP_I2C1_Init(void)
{
    I2C_InitTypeDef i;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
    BSP_GPIO_I2C1_Init();

    I2C_DeInit(I2C1);
    i.I2C_ClockSpeed          = 100000;
    i.I2C_DutyCycle           = I2C_DutyCycle_2;
    i.I2C_OwnAddress1         = 0;
    i.I2C_Mode                = I2C_Mode_I2C;
    i.I2C_Ack                 = I2C_Ack_Enable;
    i.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init(I2C1, &i);
    I2C_Cmd(I2C1, ENABLE);
}

void BSP_I2C2_Init(void)
{
    I2C_InitTypeDef i;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
    BSP_GPIO_I2C2_Init();

    I2C_DeInit(I2C2);
    i.I2C_ClockSpeed          = 100000;
    i.I2C_DutyCycle           = I2C_DutyCycle_2;
    i.I2C_OwnAddress1         = 0;
    i.I2C_Mode                = I2C_Mode_I2C;
    i.I2C_Ack                 = I2C_Ack_Enable;
    i.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init(I2C2, &i);
    I2C_Cmd(I2C2, ENABLE);
}
