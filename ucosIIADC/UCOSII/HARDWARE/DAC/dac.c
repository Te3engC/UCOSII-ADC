#include "dac.h"

/**
  * @brief  ��ʼ�� DAC
  * @param  channel: DAC ͨ�� (DAC_CHANNEL_1 �� DAC_CHANNEL_2)
  * @retval None
  */
  
  
void DAC_init(DAC_Channel channel)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    DAC_InitTypeDef DAC_InitStruct;

    // 1. ʹ�� GPIOA �� DAC ʱ��
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);

    // 2. ���� DAC ���ţ�PA4 �� PA5��
    GPIO_InitStruct.GPIO_Pin = (channel == DAC_CHANNEL_1) ? GPIO_Pin_4 : GPIO_Pin_5;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AN;      // ģ��ģʽ
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;  // ��������
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    // 3. ���� DAC
    DAC_InitStruct.DAC_Trigger = DAC_Trigger_None;       // �������
    DAC_InitStruct.DAC_WaveGeneration = DAC_WaveGeneration_None;  // �޲�������
    DAC_InitStruct.DAC_OutputBuffer = DAC_OutputBuffer_Enable;    // ʹ���������
    DAC_InitStruct.DAC_LFSRUnmask_TriangleAmplitude = DAC_LFSRUnmask_Bit0; // δʹ��

    if (channel == DAC_CHANNEL_1)
        DAC_Init(DAC_Channel_1, &DAC_InitStruct);
    else
        DAC_Init(DAC_Channel_2, &DAC_InitStruct);

    // 4. ʹ�� DAC
    DAC_Cmd((channel == DAC_CHANNEL_1) ? DAC_Channel_1 : DAC_Channel_2, ENABLE);
}

/**
  * @brief  ���� DAC �����ѹ��12λ���ȣ�0~4095 �� 0~3.3V��
  * @param  channel: DAC ͨ��
  * @param  value: ������ (0~4095)
  * @retval None
  */
void DAC_SetVoltage(DAC_Channel channel, uint16_t value)
{
    // �������뷶Χ
    if (value > 4095) value = 4095;

    // ���� DAC ���
    if (channel == DAC_CHANNEL_1)
        DAC_SetChannel1Data(DAC_Align_12b_R, value);
    else
        DAC_SetChannel2Data(DAC_Align_12b_R, value);
}

/**
  * @brief  ���� DAC �����ѹ����������0.0~3.3V��
  * @param  channel: DAC ͨ��
  * @param  voltage: ��ѹֵ (0.0~3.3V)
  * @retval None
  */
void DAC_SetVoltageFloat(DAC_Channel channel, float voltage)
{
    uint16_t dac_value;

    // �������뷶Χ
    if (voltage < 0.0f) voltage = 0.0f;
    if (voltage > 3.3f) voltage = 3.3f;

    // ת��Ϊ 12 λ������
    dac_value = (uint16_t)(voltage * 4095 / 3.3f);

    // ���� DAC_SetVoltage
    DAC_SetVoltage(channel, dac_value);
}