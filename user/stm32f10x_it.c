/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.6.0
  * @date    20-September-2021
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2011 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include "usart.h"
/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

// @brief  USART1 中断处理函数
void USART1_IRQHandler(void)
{
  if(USART_GetITStatus(USART1, USART_IT_IDLE) == SET)//如果USART_IT_IDLE置位，表示空闲中断发生，进入if
  {
    USART_GetFlagStatus(USART1,USART_FLAG_IDLE);     //清除空闲中断标志位步骤1：读取状态标志寄存器
    USART_ReceiveData(USART1);                       //清除空闲中断标志位步骤2： 读取数据寄存器
    USART_Rx_Ptr.URXCount+=RX_BUF_SIZE-DMA_GetCurrDataCounter(DMA1_Channel5);//DMA总量-剩余量 DMA接受量     //getcurrent剩余数据量 //+=操作，将本次的数据接收量，累计到URXCount变量
    USART_Rx_Ptr.URxDataIn->end=&USART_Rx_Buffer[USART_Rx_Ptr.URXCount-1];//IN指针指向的结构体中的e指针记录本次接收的结束位置
    USART_Rx_Ptr.URxDataIn++;                        //IN指针向后移
    if(USART_Rx_Ptr.URxDataIn == USART_Rx_Ptr.URxDataEnd)//如果后移到END标记的位置，进入if
    {
      USART_Rx_Ptr.URxDataIn=&USART_Rx_Ptr.URxData[0];   //回卷，重新指向0号成员
    }

		USART_Rx_Ptr.URxDataIn->start = USART_Rx_Buffer;     //下次接收位置返回缓冲区的起始位置
    USART_Rx_Ptr.URXCount = 0;                           //累计值清零
    DMA_Cmd(DMA1_Channel5, DISABLE);                     //关闭DMA通道
    DMA_SetCurrDataCounter(DMA1_Channel5, RX_BUF_SIZE);  //重新设置接收量
    DMA1_Channel5->CMAR = (uint32_t)USART_Rx_Ptr.URxDataIn->start;//重新设置接收位置
    DMA_Cmd(DMA1_Channel5, ENABLE);                      //开启DMA通道  
  }
}











/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
#ifndef RTE_CMSIS_RTOS_RTX
void SVC_Handler(void)
{
}
#endif

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
#ifndef RTE_CMSIS_RTOS_RTX
void PendSV_Handler(void)
{
}
#endif

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
/*
#ifndef RTE_CMSIS_RTOS_RTX
void SysTick_Handler(void)
{
}
#endif
*/
/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

extern __IO uint64_t ulTicks;

void SysTick_Handler(void)
{
	ulTicks++;
}

/**
  * @}
  */ 
