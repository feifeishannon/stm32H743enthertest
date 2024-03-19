/**
 * @file loghelper.h
 * @author your name (you@domain.com)
 * @brief log记录工具
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
 * @brief  日志缓存结构
 * @note   缓存长度[APP_TX_DATA_SIZE]
 * @retval None
 */
typedef struct {
    char buffer[INITIAL_BUFFER_SIZE];    // 存储日志信息的数组
    HelperMode mode;   // 运行模式
    int capacity;   // 容量
    int count;      // 当前队列中的元素个数
    uint8_t logDataReceived;      // 当前队列中的元素个数
    void (*Run)();
    uint16_t (*appendStrings)(const char *format, ...);
    uint16_t (*appendStringln)(const char *format, ...);
    void (*print)(const char *format, ...);
    void (*println)(const char *format, ...);
    void (*printtab)(const char *format, ...);
    void (*clearReceivedFlag)();
} LogHelper;

/**
 * @brief  过程监视器对象
 * @note   
 *          1、使用此监视器需配置定时器
 *          2、过程开始的位置调用 subTimes.TestSubTimeStart()
 *          3、过程结束的位置调用 subTimes.TestSubTimeEnd()
 *          4、使用时需在定时器溢出回调中调用 subTimes.TimeOutOnce()
 *  
 */
typedef struct
{
    
    uint8_t  subtimeState;      /* 过程执行状态 */ 
    uint32_t subtimeValue;      /* 过程执行时间 */ 
    uint32_t subtimeOut;        /* 过程计时器溢出次数 */ 
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
 * @brief  对于响应时间小于100us的过程使用宏检测，尽可能缩短调用周期
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
