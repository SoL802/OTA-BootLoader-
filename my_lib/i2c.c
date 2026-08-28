/**
  ******************************************************************************
  * @file    i2c.c
  */
#include "i2c.h"
void My_I2C_Init(void)
{
 //初始化iic引脚
RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
GPIO_InitTypeDef GPIO_InitStructure;
GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;//开漏输出
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
GPIO_Init(GPIOB, &GPIO_InitStructure);
 //初始化i2c1
I2C_InitTypeDef I2C_InitStructure;
RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
/* 复位 I2C1 */
    I2C_DeInit(I2C1);

    /* 配置 I2C1 */
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = 0x00;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress =
        I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = 100000;    // 100kHz

    I2C_Init(I2C1, &I2C_InitStructure);

    /* 使能 I2C1 */
    I2C_Cmd(I2C1, ENABLE);


}	
//
// @简介：通过I2C向从机写入多个字节
// 
// @参数 I2Cx：填写要操作的I2C的名称，可以是I2C1或I2C2
// @参数 Addr：填写从机的地址，左对齐 - A6 A5 A4 A3 A2 A1 A0 0
// @参数 pData：要发送的数据（数组）
// @参数 Size：要发送的数据的数量，以字节为单位
//
// @返回值：0 - 发送成功， -1 - 寻址失败， -2 - 数据被拒收
//
__weak int My_I2C_SendBytes(I2C_TypeDef *I2Cx, uint8_t Addr, const uint8_t *pData, uint16_t Size)
{
	// #1. 等待总线空闲
	while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY) == SET);
	
	// #2. 发送起始位
	I2C_GenerateSTART(I2Cx, ENABLE);
	
	while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_SB) == RESET);
	
	// #3. 寻址阶段
	I2C_ClearFlag(I2Cx, I2C_FLAG_AF);
	
	I2C_SendData(I2Cx, Addr & 0xfe);
	
	while(1)
	{
		if(I2C_GetFlagStatus(I2Cx, I2C_FLAG_ADDR) == SET)
		{
			break;
		}
		if(I2C_GetFlagStatus(I2Cx, I2C_FLAG_AF) == SET)
		{
			I2C_GenerateSTOP(I2Cx, ENABLE);
			return -1; // 寻址失败
		}
	}
	
	// 清除ADDR
	I2C_ReadRegister(I2Cx, I2C_Register_SR1);
	I2C_ReadRegister(I2Cx, I2C_Register_SR2);
	
	// #4. 发送数据
	for(uint16_t i=0; i<Size; i++)
	{
		while(1)
		{
			if(I2C_GetFlagStatus(I2Cx, I2C_FLAG_AF) == SET)
			{
				I2C_GenerateSTOP(I2Cx, ENABLE);
				return -2; // 数据被拒收
			}
			if(I2C_GetFlagStatus(I2Cx, I2C_FLAG_TXE) == SET)
			{
				break;
			}
		}
		
		I2C_SendData(I2Cx, pData[i]);
	}
	
	while(1)
	{
		if(I2C_GetFlagStatus(I2Cx, I2C_FLAG_AF) == SET)
		{
				I2C_GenerateSTOP(I2Cx, ENABLE);
				return -2; // 数据被拒收			
		}
		
		if(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BTF) == SET)
		{
			break;
		}
	}
	
	// #5. 发送停止位
	I2C_GenerateSTOP(I2Cx, ENABLE);
	return 0; // 成功
}


//
// @简介：通过I2C从从机读多个字节
// 
// @参数 I2Cx：填写要操作的I2C的名称，可以是I2C1或I2C2
// @参数 Addr：填写从机的地址，左对齐 - A6 A5 A4 A3 A2 A1 A0 0
// @参数 pBuffer：接收缓冲区（数组）
// @参数 Size：要读取的数据的数量，以字节为单位
//
// @返回值：0 - 发送成功， -1 - 寻址失败
//
__weak int My_I2C_ReceiveBytes(I2C_TypeDef *I2Cx, uint8_t Addr, uint8_t *pBuffer, uint16_t Size)
{
	if(Size == 0) return 0;
	
	// #1. 等待总线空闲
	while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY) == SET);
	
	// #2. 发送起始位
	I2C_GenerateSTART(I2Cx, ENABLE);
	
	while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_SB) == RESET);
	
	// #3. 寻址阶段
	I2C_ClearFlag(I2Cx, I2C_FLAG_AF);
	
	I2C_SendData(I2Cx, Addr | 0x01);
	
	while(1)
	{
		if(I2C_GetFlagStatus(I2Cx, I2C_FLAG_ADDR) == SET)
		{
			break;
		}
		if(I2C_GetFlagStatus(I2Cx, I2C_FLAG_AF) == SET)
		{
			I2C_GenerateSTOP(I2Cx, ENABLE);
			return -1; // 寻址失败
		}
	}
	
	// #4. 数据读取
	if(Size == 1)
	{
		// 向ACK写0
		I2C_AcknowledgeConfig(I2Cx, DISABLE);
		
		// 清除ADDR
		__disable_irq();
		I2C_ReadRegister(I2Cx, I2C_Register_SR1);
		I2C_ReadRegister(I2Cx, I2C_Register_SR2);
		
		// 发送停止位
		I2C_GenerateSTOP(I2Cx, ENABLE);
		__enable_irq();
		// 等待RxNE置位
		while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_RXNE) == RESET);
		// 读取数据
		pBuffer[0] = I2C_ReceiveData(I2Cx);
	}
	else
	{
		// 向ACK写1
		I2C_AcknowledgeConfig(I2Cx, ENABLE);
		
		// 清除ADDR
		I2C_ReadRegister(I2Cx, I2C_Register_SR1);
		I2C_ReadRegister(I2Cx, I2C_Register_SR2);
		
		for(uint16_t i=0; i<Size-1; i++)
		{
			if(i==Size-2)
			{
				__disable_irq();
			}
			// 等待RxNE置位
			while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_RXNE) == RESET);
			// 读取
			pBuffer[i] = I2C_ReceiveData(I2Cx);
		}
		
		// 向ACK写0
		I2C_AcknowledgeConfig(I2Cx, DISABLE);
		// 发送停止位
		I2C_GenerateSTOP(I2Cx, ENABLE);
		
		__enable_irq();
		
		// 等待RxNE置位
		while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_RXNE) == RESET);
		pBuffer[Size-1] = I2C_ReceiveData(I2Cx);
	}
	
	return 0;
}
