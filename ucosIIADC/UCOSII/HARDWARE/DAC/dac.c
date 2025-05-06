#include "dac.h"

/**
  * @brief  初始化 DAC
  * @param  channel: DAC 通道 (DAC_CHANNEL_1 或 DAC_CHANNEL_2)
  * @retval None
  */
  
  
void DAC_init(DAC_Channel channel)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    DAC_InitTypeDef DAC_InitStruct;

    // 1. 使能 GPIOA 和 DAC 时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);

    // 2. 配置 DAC 引脚（PA4 或 PA5）
    GPIO_InitStruct.GPIO_Pin = (channel == DAC_CHANNEL_1) ? GPIO_Pin_4 : GPIO_Pin_5;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AN;      // 模拟模式
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;  // 无上下拉
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    // 3. 配置 DAC
    DAC_InitStruct.DAC_Trigger = DAC_Trigger_None;       // 软件触发
    DAC_InitStruct.DAC_WaveGeneration = DAC_WaveGeneration_None;  // 无波形生成
    DAC_InitStruct.DAC_OutputBuffer = DAC_OutputBuffer_Enable;    // 使能输出缓冲
    DAC_InitStruct.DAC_LFSRUnmask_TriangleAmplitude = DAC_LFSRUnmask_Bit0; // 未使用

    if (channel == DAC_CHANNEL_1)
        DAC_Init(DAC_Channel_1, &DAC_InitStruct);
    else
        DAC_Init(DAC_Channel_2, &DAC_InitStruct);

    // 4. 使能 DAC
    DAC_Cmd((channel == DAC_CHANNEL_1) ? DAC_Channel_1 : DAC_Channel_2, ENABLE);
}

/**
  * @brief  设置 DAC 输出电压（12位精度，0~4095 → 0~3.3V）
  * @param  channel: DAC 通道
  * @param  value: 数字量 (0~4095)
  * @retval None
  */
void DAC_SetVoltage(DAC_Channel channel, uint16_t value)
{
    // 限制输入范围
    if (value > 4095) value = 4095;

    // 设置 DAC 输出
    if (channel == DAC_CHANNEL_1)
        DAC_SetChannel1Data(DAC_Align_12b_R, value);
    else
        DAC_SetChannel2Data(DAC_Align_12b_R, value);
}

/**
  * @brief  设置 DAC 输出电压（浮点数，0.0~3.3V）
  * @param  channel: DAC 通道
  * @param  voltage: 电压值 (0.0~3.3V)
  * @retval None
  */
void DAC_SetVoltageFloat(DAC_Channel channel, float voltage)
{
    uint16_t dac_value;

    // 限制输入范围
    if (voltage < 0.0f) voltage = 0.0f;
    if (voltage > 3.3f) voltage = 3.3f;

    // 转换为 12 位数字量
    dac_value = (uint16_t)(voltage * 4095 / 3.3f);

    // 调用 DAC_SetVoltage
    DAC_SetVoltage(channel, dac_value);
}