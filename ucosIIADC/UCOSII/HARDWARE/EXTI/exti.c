#include "exti.h"
#include "delay.h" 
#include "led.h" 
#include "key.h"
#include "beep.h"
#include "key.h" 
#include "timer.h"



void EXTI4_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line4) != RESET)  // 确保是EXTI_Line4中断
    {
                
        OSSemPost(PSem);
            
        
        EXTI_ClearITPendingBit(EXTI_Line4);  // 清除中断标志
        
    }
}

//外部中断初始化程序
//初始化PE2~4,PA0为中断输入.
void EXTIX_Init(void)
{
    NVIC_InitTypeDef   NVIC_InitStructure;
    EXTI_InitTypeDef   EXTI_InitStructure;
    
    KEY_Init(); //按键对应的IO口初始化
 
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);//使能SYSCFG时钟
    
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource4);//PE4 连接到中断线4
    
    /* 配置EXTI_Line4 */
    EXTI_InitStructure.EXTI_Line = EXTI_Line4;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//中断事件
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //下降沿触发
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;//中断线使能
    EXTI_Init(&EXTI_InitStructure);//配置
 
    NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;//外部中断4
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;//抢占优先级1
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;//子优先级2
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//使能外部中断通道
    NVIC_Init(&NVIC_InitStructure);//配置

    PSem = OSSemCreate(0);  
}












