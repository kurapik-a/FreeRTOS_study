/*
 * bsp_usart.c - USART1 初始化（标准库实现）
 * 参数与 HAL 工程 MX_USART1_UART_Init 一致：115200-8-N-1，TX/RX，无流控。
 * 说明：HAL 工程里虽配置了 USART1 的 DMA（DMA2 Stream7/Stream2），
 *       但所有驱动实际都走轮询收发，故本移植不搬运 DMA 部分。
 */
#include "bsp_usart.h"
#include "bsp_gpio.h"

void BSP_USART1_Init(void)
{
    USART_InitTypeDef u;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    BSP_GPIO_USART1_Init();

    USART_DeInit(USART1);
    u.USART_BaudRate            = 115200;
    u.USART_WordLength          = USART_WordLength_8b;
    u.USART_StopBits            = USART_StopBits_1;
    u.USART_Parity              = USART_Parity_No;
    u.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
    u.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(USART1, &u);
    USART_Cmd(USART1, ENABLE);
}

void BSP_USART1_SendByte(uint8_t c)
{
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    USART_SendData(USART1, c);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);   /* 与 HAL 一致等发送完成 */
}

int BSP_USART1_RecvByte(uint8_t *c)
{
    /* 清溢出标志（对应 HAL 的 __HAL_UART_CLEAR_OREFLAG） */
    if (USART_GetFlagStatus(USART1, USART_FLAG_ORE) == SET)
    {
        (void)USART_ReceiveData(USART1);
    }
    while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);
    *c = (uint8_t)USART_ReceiveData(USART1);
    return 0;
}
