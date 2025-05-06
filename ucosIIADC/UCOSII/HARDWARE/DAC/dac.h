#ifndef __DAC_H
#define __DAC_H

#include "stm32f4xx.h"

// DAC ͨ������
typedef enum {
    DAC_CHANNEL_1 = 0,  // PA4
    DAC_CHANNEL_2 = 1      // PA5
} DAC_Channel;

// ��ʼ�� DAC
void DAC_init(DAC_Channel channel);

// ���� DAC �����ѹ��0~4095 �� 0~3.3V��
void DAC_SetVoltage(DAC_Channel channel, uint16_t value);

// ���� DAC �����ѹ��0.0~3.3V��
void DAC_SetVoltageFloat(DAC_Channel channel, float voltage);

#endif /* __DAC_H */