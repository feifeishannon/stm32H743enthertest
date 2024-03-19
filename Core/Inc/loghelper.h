/**
 * @file loghelper.h
 * @author your name (you@domain.com)
 * @brief log��¼����
 * @version 0.1
 * @date 2023-08-02
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "usbd_cdc_if.h"
#include "stdio.h"
#include <stdarg.h>
#include <stdint.h>
#include "stm32h7xx_hal.h"

#define INITIAL_BUFFER_SIZE 10000
#define POP_STRING_SIZE     500


typedef enum{
    Normal,
    Test,
    Check
}HelperMode;

/**
 * @brief  ��־����ṹ
 * @note   ���泤��[APP_TX_DATA_SIZE]
 * @retval None
 */
typedef struct {
    char buffer[INITIAL_BUFFER_SIZE];    // �洢��־��Ϣ������
    HelperMode mode;   // ����ģʽ
    int capacity;   // ����
    int count;      // ��ǰ�����е�Ԫ�ظ���
    uint8_t logDataReceived;      // ��ǰ�����е�Ԫ�ظ���
    void (*Run)();
    uint16_t (*appendStrings)(const char *format, ...);
    uint16_t (*appendStringln)(const char *format, ...);
    void (*print)(const char *format, ...);
    void (*println)(const char *format, ...);
    void (*printtab)(const char *format, ...);
    void (*clearReceivedFlag)();
} LogHelper;

/**
 * @brief  ���̼���������
 * @note   
 *          1��ʹ�ô˼����������ö�ʱ��
 *          2�����̿�ʼ��λ�õ��� subTimes.TestSubTimeStart()
 *          3�����̽�����λ�õ��� subTimes.TestSubTimeEnd()
 *          4��ʹ��ʱ���ڶ�ʱ������ص��е��� subTimes.TimeOutOnce()
 *  
 */
typedef struct
{
    
    uint8_t  subtimeState;      /* ����ִ��״̬ */ 
    uint32_t subtimeValue;      /* ����ִ��ʱ�� */ 
    uint32_t subtimeOut;        /* ���̼�ʱ��������� */ 
    TIM_HandleTypeDef *htim;
    void (*TimeOutOnce)();
    void (*TestSubTimeStart)();
    void (*TestSubTimeEnd)();
    void (*TimeOutValueClear)();
    void (*TimeSubTimeValueClear)();
    uint32_t (*GetSubTimeOutValue)();
    uint32_t (*GetSubTimeValue)();
    
} SubTimes_TypeDef;

/***************************************************************/
/**
 * @brief  ������Ӧʱ��С��100us�Ĺ���ʹ�ú��⣬���������̵�������
 * @note   
 */
#define TEST_SUB_TIME(htim)        subTimes.subtime=0;\
                                    subTimes.subtimeOut=0;\
                                    __HAL_TIM_SET_COUNTER((htim),0);\
                                    HAL_TIM_Base_Start_IT((htim));

#define TEST_SUB_TIME_END(htim)    subTimes.subtime = subTimes.subtimeOut * 65536 + __HAL_TIM_GET_COUNTER((htim));\
                                    subTimes.subtimeOut=0;\
                                    HAL_TIM_Base_Stop_IT((htim));\
                                    __HAL_TIM_SET_COUNTER((htim),0);
/***************************************************************/

extern uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];

/** Data to send over USB CDC are stored in this buffer   */
extern uint8_t UserTxBufferFS[APP_TX_DATA_SIZE];

extern SubTimes_TypeDef subTimes;
extern LogHelper logHelper;

void SubTimesCreate( TIM_HandleTypeDef *htim);
void logHelperCreate(void);
