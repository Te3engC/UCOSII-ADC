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

// ��main.c��ȫ�ֱ����������
static INT8U collect_enabled = 1;  // 1��ʾ����ɼ���0��ʾֹͣ�ɼ�


// ���������ջ��С
#define TASK_STK_SIZE  128

// �����������ȼ�
#define COLLECT_TASK_PRIO  5
#define PROCESS_TASK_PRIO  7
#define COMM_TASK_PRIO     8

// �����ź���
OS_EVENT *TSem;  // ��ʱ���ж��ź���
OS_EVENT *PSem;  // �����ź���

// �����ջ
OS_STK CollectTaskStk[TASK_STK_SIZE];
OS_STK ProcessTaskStk[TASK_STK_SIZE];
OS_STK CommTaskStk[TASK_STK_SIZE];

// ADCֵ������
INT16U adc_value;

// ���͵����ַ�
void UART_SendChar(USART_TypeDef *USARTx, char c)
{
    while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);
    USART_SendData(USARTx, (uint8_t)c);
}

// �������ֵ�ASCII�루0-9��
void UART_SendNumber(USART_TypeDef *USARTx, uint16_t num)
{
    if (num >= 10)
        UART_SendNumber(USARTx, num / 10); // �ݹ鷢�͸�λ
    
    UART_SendChar(USARTx, (num % 10) + '0'); // ���͵�ǰλ��ASCIIת����
}



// �ɼ�����
void CollectTask(void *pdata)
{
    INT8U err;
    
    while (1)
    {
        // �ȴ���ʱ���ź���
        OSSemPend(TSem, 0, &err);
        
        // ֻ��������ɼ�ʱ��ִ�вɼ�
        if(collect_enabled)
        {
            // ִ��ADC�ɼ�
            adc_value = Get_Adc(ADC_Channel_5);
            
//            // ���� "ADC: " ��ASCII��
//            UART_SendChar(USART1, 'A');
//            UART_SendChar(USART1, 'D');
//            UART_SendChar(USART1, 'C');
//            UART_SendChar(USART1, ':');
//            UART_SendChar(USART1, ' ');
            
            // ����ADCֵ��ASCII��
            UART_SendNumber(USART1, adc_value);
            
            // ���ͻ��з���\r\n��
            UART_SendChar(USART1, '\r');
            UART_SendChar(USART1, '\n');
        }
    }
}

// ����������Ӧ�����жϣ�
// ����������Ӧ�����жϣ�
// ����������Ӧ�����жϣ�
void ProcessTask(void *pdata)
{
    INT8U err;
    const char *msg;
    INT32U last_key_time = 0;
    INT32U now;
    INT32U wait_start;
    #define KEY_DEBOUNCE_MS 200  // ȥ����ʱ��
    
    while(1)
    {
        // �ȴ������ź���
        OSSemPend(PSem, 0, &err);
        
        // ��ȡ��ǰʱ�䲢����Ƿ���ȥ��������
        now = OSTimeGet();
        if((now - last_key_time) < KEY_DEBOUNCE_MS) {
            continue;  // ���Զ����ڼ�Ĵ���
        }
        last_key_time = now;
        
        // �ȴ������ȶ��ͷţ�����ʱ��
        wait_start = now;
        while(KEY0 == 0) {
            if((OSTimeGet() - wait_start) > 1000) {  // 1�볬ʱ
                break;
            }
            OSTimeDlyHMSM(0, 0, 0, 10);
        }
        
        // �л�״̬
        collect_enabled = !collect_enabled;
        
        if(collect_enabled) {
            msg = "ADC Collection Started!\r\n";
            // ��������һ�βɼ�
            OSSemPost(TSem);
        } else {
            msg = "ADC Collection Stopped!\r\n";
        }
        
        // ����״̬��Ϣ
        while(*msg) {
            UART_SendChar(USART1, *msg++);
        }
        
        // �����ӳ�ȷ��������ȫ�������
        OSTimeDlyHMSM(0, 0, 0, 100);
    }
}

int main(void)
{
    INT8U err;
    OS_CPU_SR cpu_sr=0;
    // ��ʼ��Ӳ��
    delay_init(168);		  //��ʼ����ʱ����
    uart_init(115200);      // ��ʼ������1��������115200
    Adc_Init();              // ��ʼ��ADC1
    TIM3_Int_Init(799, 8399); // 250ms��ʱ���жϣ�TIM3��
    EXTIX_Init();             // ��ʼ���ⲿ�жϣ�������
    KEY_Init();
    
    // ��ʼ��uC/OS-II
    OSInit();
    
    // �����ź���
    PSem = OSSemCreate(0);  // �����ź�����ʼΪ0
    TSem = OSSemCreate(0);  // ��ʱ���ź�����ʼΪ0
    
    // ��������
    OS_ENTER_CRITICAL();
    OSTaskCreate(ProcessTask, NULL, 
                &ProcessTaskStk[TASK_STK_SIZE-1], PROCESS_TASK_PRIO);
    OSTaskCreate(CollectTask, NULL, 
                &CollectTaskStk[TASK_STK_SIZE-1], COLLECT_TASK_PRIO);
    OSTaskSuspend(10);	//������ʼ����.            
    OS_EXIT_CRITICAL();
   
    
    
    
    // ���������񻷾�
    OSStart();
    
    return 0;
}

