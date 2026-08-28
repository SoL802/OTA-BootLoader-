/**
  * @brief   串口头文件
  */
	
#ifndef _USART_H_
#define _USART_H_
#include "stm32f10x.h"
#include <stdarg.h>
#include <stdio.h>
#define USART_RX_QUEUE_SIZE 10 //se指针对结构体数组长度
#define RX_MAX 2048            //单次接收最大量
#define RX_BUF_SIZE 2048 // 接收数组缓冲区长度
#define TX_BUF_SIZE 2048 // 发送数组缓冲区长度
#define LINE_SEPERATOR_CR   0x00 // 回车 \r
#define LINE_SEPERATOR_LF   0x01 // 换行 \n
#define LINE_SEPERATOR_CRLF 0x02 // 回车+换行 \r\n

typedef struct
{
 uint8_t *start;  //s用于标记起始位置
 uint8_t *end;    //e用于标记结束位置

} UCB_RxBuffptr;  //se指针对结构体

/* 接收管理句柄 */
typedef struct
{
 uint16_t URXCount;        // 累计接收数据量
 UCB_RxBuffptr URxData[USART_RX_QUEUE_SIZE];//se指针对结构体数组
 UCB_RxBuffptr *URxDataIn; // 指针用于标记接收数据                     接收数据入队指针
 UCB_RxBuffptr *URxDataOut;// OUT指针 用于提取接收的数据               接收数据出队指针  
	UCB_RxBuffptr *URxDataEnd;// IN和OUT指针的结尾标志                   接收数据结束指针
} UCB_RXPtr;               //串口控制结构体
extern UCB_RXPtr USART_Rx_Ptr;
extern uint8_t USART_Rx_Buffer[RX_BUF_SIZE];
void usart_init(void);
void DMA_init(void);
void Usart1Rx_PtrInit(void);
void My_USART_SendByte(USART_TypeDef *USARTx, const uint8_t Data);
void My_USART_SendBytes(USART_TypeDef *USARTx, const uint8_t *pData, uint16_t Size);
void My_USART_SendChar(USART_TypeDef *USARTx, const char C);
void My_USART_SendString(USART_TypeDef *USARTx, const char *Str);
void My_USART_Printf(USART_TypeDef *USARTx, const char *Format, ...);

uint8_t My_USART_ReceiveByte(USART_TypeDef *USARTx);
uint16_t My_USART_ReceiveBytes(USART_TypeDef *USARTx, uint8_t *pDataOut, uint16_t Size, int Timeout);
int My_USART_ReceiveLine(USART_TypeDef *USARTx, char *pStrOut, uint16_t MaxLength, uint16_t LineSeperator, int Timeout);

#endif

