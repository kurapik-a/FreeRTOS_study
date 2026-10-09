/*
 * bsp_adc.c - ADC1 轮询采集（标准库实现）
 * 与 HAL 工程一致：独立模式、12bit、单次软件触发、通道 3（PA3）、3 周期采样。
 */
#include "bsp_adc.h"

#define ADC_TMO  500000u

void BSP_ADC1_Init(void)
{
    ADC_InitTypeDef a;
    ADC_CommonInitTypeDef c;
    GPIO_InitTypeDef g;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

    /* PA3 模拟输入 */
    g.GPIO_Pin  = GPIO_Pin_3;
    g.GPIO_Mode = GPIO_Mode_AN;
    g.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &g);

    /* ADC 公共配置（HAL 默认等价：独立模式、分频 2、无 DMA、无延迟） */
    c.ADC_Mode             = ADC_Mode_Independent;
    c.ADC_Prescaler        = ADC_Prescaler_Div2;
    c.ADC_DMAAccessMode    = ADC_DMAAccessMode_Disabled;
    c.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
    ADC_CommonInit(&c);

    a.ADC_Resolution           = ADC_Resolution_12b;
    a.ADC_ScanConvMode         = DISABLE;
    a.ADC_ContinuousConvMode   = DISABLE;
    a.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
    a.ADC_ExternalTrigConv     = ADC_ExternalTrigConv_T1_CC1;
    a.ADC_DataAlign            = ADC_DataAlign_Right;
    a.ADC_NbrOfConversion      = 1;
    ADC_Init(ADC1, &a);

    ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 1, ADC_SampleTime_3Cycles);
    ADC_Cmd(ADC1, ENABLE);
}

int BSP_ADC1_Read(uint16_t *val)
{
    uint32_t tmo = ADC_TMO;

    ADC_SoftwareStartConv(ADC1);
    while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET)
    {
        if (--tmo == 0) return -1;
    }
    *val = (uint16_t)ADC_GetConversionValue(ADC1);
    return 0;
}
