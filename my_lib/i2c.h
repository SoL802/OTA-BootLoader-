/**
  ******************************************************************************
  * @file    i2c.h
  */

#include "stm32f10x.h"
void My_I2C_Init(void);
int My_I2C_SendBytes(I2C_TypeDef *I2Cx, uint8_t Addr, const uint8_t *pData, uint16_t Size);
int My_I2C_ReceiveBytes(I2C_TypeDef *I2Cx, uint8_t Addr, uint8_t *pBuffer, uint16_t Size);
