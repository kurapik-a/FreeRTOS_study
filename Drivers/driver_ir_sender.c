// SPDX-License-Identifier: GPL-3.0-only
/*
 * driver_ir_sender.c - 红外发射驱动（标准库版）
 * TIM9_CH2 输出 38kHz 载波（PSC=0, ARR=999），NEC 协议发送。
 * 注意：与原 HAL 版一致，本驱动未配置 TIM9 的复用引脚（PA3），
 *       且与光敏 ADC（PA3）共用引脚，二者不可同时使用。
 */
#include "driver_ir_sender.h"
#include "driver_lcd.h"
#include "driver_timer.h"
#include "stm32f4xx.h"

#define IR_SENDER_CHANNEL TIM_Channel_2
#define IR_SENDER_TIM     TIM9

static void IRSender_TIM_Config(void)
{
    TIM_TimeBaseInitTypeDef t = {0};
    TIM_OCInitTypeDef o = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM9, ENABLE);

    t.TIM_Prescaler         = 0;
    t.TIM_CounterMode       = TIM_CounterMode_Up;
    t.TIM_Period            = 999;
    t.TIM_ClockDivision     = TIM_CKD_DIV1;
    t.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(IR_SENDER_TIM, &t);
    TIM_ARRPreloadConfig(IR_SENDER_TIM, ENABLE);

    o.TIM_OCMode      = TIM_OCMode_PWM1;
    o.TIM_OutputState = TIM_OutputState_Enable;
    o.TIM_Pulse       = 473;               /* 约 47% 占空比 */
    o.TIM_OCPolarity  = TIM_OCPolarity_Low;
    TIM_OC2Init(IR_SENDER_TIM, &o);
    TIM_OC2PreloadConfig(IR_SENDER_TIM, TIM_OCPreload_Disable);
}

static void IRSender_Carrier(int on)
{
    if (on)
    {
        TIM_Cmd(IR_SENDER_TIM, ENABLE);
        TIM_CCxCmd(IR_SENDER_TIM, IR_SENDER_CHANNEL, TIM_CCx_Enable);
    }
    else
    {
        TIM_CCxCmd(IR_SENDER_TIM, IR_SENDER_CHANNEL, TIM_CCx_Disable);
    }
}

void IRSender_Init(void)
{
    IRSender_TIM_Config();
    /* 不使能计数器，等发送时再开（与 HAL 版一致） */
}

void IRSender_Write(uint8_t device, uint8_t data)
{
    uint8_t datas[4];
    int i, j;
    datas[0] = device;
    datas[1] = ~device;
    datas[2] = data;
    datas[3] = ~data;

    IRSender_Carrier(1);            /* 9ms 引导 */
    udelay(9000);
    IRSender_Carrier(0);
    udelay(4500);                   /* 4.5ms 间隔 */
    for (i = 0; i < 4; i++)
    {
        for (j = 7; j >= 0; j--)
        {
            IRSender_Carrier(1);    /* 560us 载波 */
            udelay(560);
            IRSender_Carrier(0);
            if (datas[i] & (1<<j))
            {
                udelay(1600);       /* 逻辑 1: 1.6ms 空闲 */
            }
            else
            {
                udelay(560);        /* 逻辑 0: 0.56ms 空闲 */
            }
        }
    }
    IRSender_Carrier(1);            /* 结束的 560us 脉冲 */
    udelay(560);
    IRSender_Carrier(0);
}

void IRSender_Test(void)
{
    int i;
    int len;
    uint8_t device = 0x55;
    const uint8_t codes[]= {0xa2, 0xe2, 0x22, 0x02, 0xc2, 0xe0, 0xa8, 0x90, \
                            0x68, 0x98, 0xb0, 0x30, 0x18, 0x7a, 0x10, 0x38, \
                            0x5a, 0x42, 0x4a, 0x52, 0x00};
    const char *names[]= {"Power", "Menu", "Test", "+", "Return", "Left", "Play", "Right", \
                            "0", "-", "C", "1", "2", "3", "4", "5", \
                            "6", "7", "8", "9", "Repeat"};
    IRSender_Init();
    while (1)
    {
        for (i = 0; i < sizeof(codes)/sizeof(codes[0]); i++)
        {
            LCD_PrintString(0, 0, "IR Sender: ");
            LCD_PrintString(0, 2, "Device  Data");
            LCD_PrintString(0, 4, "                ");
            LCD_PrintHex(0, 4, device, 1);
            LCD_PrintHex(8, 4, codes[i], 1);
            LCD_PrintString(0, 6, "                ");
            len = LCD_PrintString(0, 6, "Key name: ");
            LCD_PrintString(len, 6, names[i]);
            IRSender_Write(device, codes[i]);
            mdelay(3000);
        }
    }
}
