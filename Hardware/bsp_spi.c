/*
 * bsp_spi.c - SPI1 主机轮询驱动（标准库实现）
 * 参数与 HAL 工程 MX_SPI1_Init 一致：8bit、CPOL=0、CPHA=1EDGE、MSB 先行、
 * 波特率 4 分频（168/4 = 42MHz）、NSS 软件管理。
 * 片选由各驱动（W25Q64: PC1）自行控制。
 */
#include "bsp_spi.h"
#include "bsp_gpio.h"

#define SPI_TMO  200000u

static int spi_wait_tx(void)
{
    uint32_t tmo = SPI_TMO;
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET)
        if (--tmo == 0) return -1;
    return 0;
}

static int spi_wait_rx(void)
{
    uint32_t tmo = SPI_TMO;
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET)
        if (--tmo == 0) return -1;
    return 0;
}

static int spi_wait_notbusy(void)
{
    uint32_t tmo = SPI_TMO;
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET)
        if (--tmo == 0) return -1;
    return 0;
}

int BSP_SPI_TransmitReceive(uint8_t *pTx, uint8_t *pRx, uint16_t len)
{
    uint16_t i;
    for (i = 0; i < len; i++)
    {
        if (spi_wait_tx()) return -1;
        SPI_I2S_SendData(SPI1, pTx[i]);
        if (spi_wait_rx()) return -1;
        pRx[i] = (uint8_t)SPI_I2S_ReceiveData(SPI1);
    }
    return spi_wait_notbusy();
}

int BSP_SPI_Transmit(const uint8_t *pTx, uint16_t len)
{
    uint16_t i;
    uint8_t dummy;
    for (i = 0; i < len; i++)
    {
        if (spi_wait_tx()) return -1;
        SPI_I2S_SendData(SPI1, pTx[i]);
        if (spi_wait_rx()) return -1;
        dummy = (uint8_t)SPI_I2S_ReceiveData(SPI1);   /* 清 RXNE，防止溢出 */
        (void)dummy;
    }
    return spi_wait_notbusy();
}

int BSP_SPI_Receive(uint8_t *pRx, uint16_t len)
{
    uint16_t i;
    for (i = 0; i < len; i++)
    {
        if (spi_wait_tx()) return -1;
        SPI_I2S_SendData(SPI1, 0xFF);                 /* 全双工：读的同时发 dummy */
        if (spi_wait_rx()) return -1;
        pRx[i] = (uint8_t)SPI_I2S_ReceiveData(SPI1);
    }
    return spi_wait_notbusy();
}

void BSP_SPI1_Init(void)
{
    SPI_InitTypeDef s;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
    BSP_GPIO_SPI1_Init();

    SPI_I2S_DeInit(SPI1);
    s.SPI_Direction         = SPI_Direction_2Lines_FullDuplex;
    s.SPI_Mode              = SPI_Mode_Master;
    s.SPI_DataSize          = SPI_DataSize_8b;
    s.SPI_CPOL              = SPI_CPOL_Low;
    s.SPI_CPHA              = SPI_CPHA_1Edge;
    s.SPI_NSS               = SPI_NSS_Soft;
    s.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
    s.SPI_FirstBit          = SPI_FirstBit_MSB;
    s.SPI_CRCPolynomial     = 10;
    SPI_Init(SPI1, &s);
    SPI_Cmd(SPI1, ENABLE);
}
