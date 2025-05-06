#include "stm32f4xx.h"
#include "ucos_ii.h"
#include "usart.h"
#include "adc.h"
#include "key.h"
#include "timer.h"
#include "exti.h"
#include "dac.h"
#include "includes.h"
#include "sys.h"
#include "delay.h"

// 在main.c的全局变量区域添加
static INT8U collect_enabled = 1;  // 1表示允许采集，0表示停止采集


// 定义任务堆栈大小
#define TASK_STK_SIZE  128

// 定义任务优先级
#define COLLECT_TASK_PRIO  5
#define PROCESS_TASK_PRIO  7
#define COMM_TASK_PRIO     8

// 定义信号量
OS_EVENT *TSem;  // 定时器中断信号量
OS_EVENT *PSem;  // 按键信号量

// 任务堆栈
OS_STK CollectTaskStk[TASK_STK_SIZE];
OS_STK ProcessTaskStk[TASK_STK_SIZE];
OS_STK CommTaskStk[TASK_STK_SIZE];

// ADC值缓冲区
INT16U adc_value;

// 发送单个字符
void UART_SendChar(USART_TypeDef *USARTx, char c)
{
    while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);
    USART_SendData(USARTx, (uint8_t)c);
}

// 发送数字的ASCII码（0-9）
void UART_SendNumber(USART_TypeDef *USARTx, uint16_t num)
{
    if (num >= 10)
        UART_SendNumber(USARTx, num / 10); // 递归发送高位
    
    UART_SendChar(USARTx, (num % 10) + '0'); // 发送当前位（ASCII转换）
}



// 采集任务
void CollectTask(void *pdata)
{
    INT8U err;
    
    while (1)
    {
        // 等待定时器信号量
        OSSemPend(TSem, 0, &err);
        
        // 只有在允许采集时才执行采集
        if(collect_enabled)
        {
            // 执行ADC采集
            adc_value = Get_Adc(ADC_Channel_5);
            
//            // 发送 "ADC: " 的ASCII码
//            UART_SendChar(USART1, 'A');
//            UART_SendChar(USART1, 'D');
//            UART_SendChar(USART1, 'C');
//            UART_SendChar(USART1, ':');
//            UART_SendChar(USART1, ' ');
            
            // 发送ADC值的ASCII码
            UART_SendNumber(USART1, adc_value);
            
            // 发送换行符（\r\n）
            UART_SendChar(USART1, '\r');
            UART_SendChar(USART1, '\n');
        }
    }
}

// 处理任务（响应按键中断）
// 处理任务（响应按键中断）
// 处理任务（响应按键中断）
void ProcessTask(void *pdata)
{
    INT8U err;
    const char *msg;
    INT32U last_key_time = 0;
    INT32U now;
    INT32U wait_start;
    #define KEY_DEBOUNCE_MS 200  // 去抖动时间
    
    while(1)
    {
        // 等待按键信号量
        OSSemPend(PSem, 0, &err);
        
        // 获取当前时间并检查是否在去抖动期内
        now = OSTimeGet();
        if((now - last_key_time) < KEY_DEBOUNCE_MS) {
            continue;  // 忽略抖动期间的触发
        }
        last_key_time = now;
        
        // 等待按键稳定释放（带超时）
        wait_start = now;
        while(KEY0 == 0) {
            if((OSTimeGet() - wait_start) > 1000) {  // 1秒超时
                break;
            }
            OSTimeDlyHMSM(0, 0, 0, 10);
        }
        
        // 切换状态
        collect_enabled = !collect_enabled;
        
        if(collect_enabled) {
            msg = "ADC Collection Started!\r\n";
            // 立即触发一次采集
            OSSemPost(TSem);
        } else {
            msg = "ADC Collection Stopped!\r\n";
        }
        
        // 发送状态信息
        while(*msg) {
            UART_SendChar(USART1, *msg++);
        }
        
        // 额外延迟确保按键完全处理完毕
        OSTimeDlyHMSM(0, 0, 0, 100);
    }
}

int main(void)
{
    INT8U err;
    OS_CPU_SR cpu_sr=0;
    // 初始化硬件
    delay_init(168);		  //初始化延时函数
    uart_init(115200);      // 初始化串口1，波特率115200
    Adc_Init();              // 初始化ADC1
    TIM3_Int_Init(799, 8399); // 250ms定时器中断（TIM3）
    EXTIX_Init();             // 初始化外部中断（按键）
    KEY_Init();
    
    // 初始化uC/OS-II
    OSInit();
    
    // 创建信号量
    PSem = OSSemCreate(0);  // 按键信号量初始为0
    TSem = OSSemCreate(0);  // 定时器信号量初始为0
    
    // 创建任务
    OS_ENTER_CRITICAL();
    OSTaskCreate(ProcessTask, NULL, 
                &ProcessTaskStk[TASK_STK_SIZE-1], PROCESS_TASK_PRIO);
    OSTaskCreate(CollectTask, NULL, 
                &CollectTaskStk[TASK_STK_SIZE-1], COLLECT_TASK_PRIO);
    OSTaskSuspend(10);	//挂起起始任务.            
    OS_EXIT_CRITICAL();
   
    
    
    
    // 启动多任务环境
    OSStart();
    
    return 0;
}

