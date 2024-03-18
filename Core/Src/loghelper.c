/**
 * @file loghelper.c
 * @author your name (you@domain.com)
 * @brief log��¼����
 * @version 0.1
 * @date 2023-08-02
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "loghelper.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
SubTimes_TypeDef subTimes={0};
LogHelper logHelper={0};



static void clearReceivedFlag(void);

/**
 * @brief  ����־������׷���ַ���
 * @note   �Զ����ַ���׷�Ӻ��������ܱ䳤���������ȳ���ʣ��ʱ�����޷�����ַ�������
 * @param  *format: ��ʽ���ַ���
 * @retval �����ַ�������
 */
static uint16_t appendStrings(const char *format, ...) {
  char log[1000]={0};
  int totalLength = 0;
  va_list args;
  va_start(args, format); 
    totalLength = vsprintf(log, format, args);
  va_end(args);  
  if ((totalLength>0)&&(logHelper.count + totalLength < logHelper.capacity )) {
    strcat(logHelper.buffer, log);
    logHelper.buffer[logHelper.count + totalLength] = '\0';
    logHelper.count += totalLength;
  }
  return totalLength;
}

/**
 * @brief  ����־������׷���ַ���
 * @note   �Զ����ַ���׷�Ӻ��������ܱ䳤����
 * @param  *format: ��ʽ���ַ���
 * @retval �����ַ�������
 */
static uint16_t appendStringln(const char *format, ...) {
  char log[1000]={0};
  int totalLength = 0;
  va_list args;
  va_start(args, format);
    totalLength = vsprintf(log, format, args);
  va_end(args);
  if ((totalLength>0)&&(logHelper.count + totalLength < logHelper.capacity )) {
    strcat(logHelper.buffer, log);
    strcat(logHelper.buffer, "\r\n");
    logHelper.count += totalLength + 2;
  }
  
  return totalLength;
}


static void print(const char* format, ...) {
  va_list args;
  uint32_t length;
  va_start(args, format);
    length = vsnprintf((char *)UserTxBufferFS, APP_TX_DATA_SIZE, (char *)format, args);
  va_end(args);
  CDC_Transmit_FS(UserTxBufferFS, length);
}

static void println(const char* format, ...) {
  va_list args;
  uint32_t length;
  va_start(args, format);
    length = vsnprintf((char *)UserTxBufferFS, APP_TX_DATA_SIZE, (char *)format, args);
  va_end(args);
  if (length < APP_TX_DATA_SIZE -2){
    strcat((char*)UserTxBufferFS, "\r\n");
    // UserTxBufferFS[length] = '\r';
    // UserTxBufferFS[length+1] = '\n';
  }
  CDC_Transmit_FS(UserTxBufferFS, length+2);
}

static void printtab(const char* format, ...) {
  va_list args;
  uint32_t length;
  va_start(args, format);
    length = vsnprintf((char *)UserTxBufferFS, APP_TX_DATA_SIZE, (char *)format, args);
  va_end(args);
  if (length < APP_TX_DATA_SIZE -1){
    UserTxBufferFS[length] = '\t';
  }
  CDC_Transmit_FS(UserTxBufferFS, length+1);
  
}

/**
 * @brief  ��ȡ len ���ȵ��ַ�����������Դ�ַ�
 * @note   
 * @param  s: ��ȡ���ַ���
 * @param  len: ��ȡ����
 * @retval 
 */
static void extractAndShift(char *s, uint16_t len) {
  char splitstr[INITIAL_BUFFER_SIZE-POP_STRING_SIZE] = {0};
  strncpy(s, logHelper.buffer, len);
  strncpy(splitstr, (logHelper.buffer + len), (logHelper.capacity - len));
  memset(logHelper.buffer, 0, logHelper.capacity);
  strcpy(logHelper.buffer, splitstr);
  if (logHelper.count > len)
  {
    logHelper.count -= len;
  }else{
    logHelper.count = 0;
  }  
}

static void checkReceive(){
  if(dataReceived){
    logHelper.logDataReceived = true;
  }else{
    logHelper.logDataReceived = false;
  }
}
static void changeMode(){
  if(logHelper.logDataReceived){
    if (strcmp((char*)UserRxBufferFS, "test") == 0) {
        logHelper.mode = Test;
    } else if (strcmp((char*)UserRxBufferFS, "check") == 0) {
        logHelper.mode = Check;
    } else {
        logHelper.mode = Normal;
    }
    clearReceivedFlag();
    memset(UserRxBufferFS,0,sizeof(UserRxBufferFS));

  }
}

/**
 * @brief  ��־���������ȷ���
 * @note   ����ö�ʱ���д����Ƽ�1ms��ʱ��
 * @retval None
 */
void Run(){
  char tempstr[POP_STRING_SIZE+1]={0};
  checkReceive();
  changeMode();
  if((logHelper.count > 0) && (dataTransmitted == USBD_OK) && usb_connect_status()){
    /* ��ȡ500�ַ����������������buffer */
    extractAndShift(tempstr,POP_STRING_SIZE);
    print("%s",tempstr);
  }
}

// �ڻص������м�鷢���Ƿ����
static void CDC_TransmitCplt_FS(uint8_t status) {
  if (status == USBD_OK) {
    // ���ݷ������
    dataTransmitted = 1;
  } else {
    // ���ݷ���ʧ��
    dataTransmitted = 0;
  }
}


// ���������ɱ�־
static void clearReceivedFlag() {
  if (dataReceived == 1) {
    // ���ݷ������
    dataReceived = 0;
  } 
}


/**
 * @brief  ʵ������־����������
 * @note   
 * @retval None
 */
void logHelperCreate()
{
  logHelper.capacity = INITIAL_BUFFER_SIZE;
  logHelper.Run = Run;
  logHelper.appendStrings = appendStrings;
  logHelper.appendStringln = appendStringln;
  logHelper.print = print;
  logHelper.println = println;
  logHelper.printtab = printtab;
  logHelper.clearReceivedFlag = clearReceivedFlag;
	logHelper.count = 0;
  logHelper.mode = Normal;
  if (&subTimes)
  {
    HAL_TIM_Base_Start_IT((subTimes.htim));
  }
  
} 

/**
 * @brief  ���̼����ֶ�
 * @note   
 * @retval None
 */
#if 1

void TestSubTimeStart(){
  subTimes.subtimeState = 1;
  subTimes.subtimeValue=0;
  subTimes.subtimeOut=0;
  __HAL_TIM_SET_COUNTER((subTimes.htim),0);
  // HAL_TIM_Base_Start_IT((subTimes.htim));
}

void TestSubTimeEnd(){
  subTimes.subtimeState = 0;
  subTimes.subtimeValue =  subTimes.subtimeOut * 65536 
                      + __HAL_TIM_GET_COUNTER((subTimes.htim));
  subTimes.subtimeOut=0;
  // HAL_TIM_Base_Stop_IT((subTimes.htim));
  // __HAL_TIM_SET_COUNTER((subTimes.htim),0);
}


/**
 * @brief  Getter Setter
 * @note   
 * @retval None
 */
void TimeOutOnce(){
  static int timer=0;
  if(subTimes.subtimeState){
    subTimes.subtimeOut++;
  }else{
    if(timer++>0){// 1ms����һ��������
      timer = 0;
    }
  }
}

void TimeOutValueClear(){
  subTimes.subtimeOut=0;
}

void TimeSubTimeValueClear(){
  subTimes.subtimeValue=0;
}

uint32_t GetSubTimeOutValue(){
  return subTimes.subtimeOut;
}

uint32_t GetSubTimeValue(){
  return subTimes.subtimeValue;
}

/**
 * @brief  
 * @note   
 * @param  *htim: us��ʱ��
 * @retval None
 */
void SubTimesCreate( TIM_HandleTypeDef *htim)
{
	subTimes.htim = htim;
	subTimes.TimeOutOnce            = TimeOutOnce;
	subTimes.TestSubTimeStart       = TestSubTimeStart;
	subTimes.TestSubTimeEnd         = TestSubTimeEnd;
	subTimes.TimeOutValueClear      = TimeOutValueClear;
	subTimes.TimeSubTimeValueClear  = TimeSubTimeValueClear;
	subTimes.GetSubTimeOutValue     = GetSubTimeOutValue;
	subTimes.GetSubTimeValue        = GetSubTimeValue;
  

} 

#endif/*<  ���̼����ֶ� >*/
