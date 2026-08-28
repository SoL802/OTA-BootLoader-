/**
  ******************************************************************************
  * @file    spi.h
  * @brief   spi驱动头文件
  */
	
#include "stm32f10x.h"	
void App_SPI1_Init(void);
uint8_t SPI1_ReadWriteByte(uint8_t txd);
void SPI1_Write(uint8_t *wdata, uint16_t datalen);
void SPI1_Read(uint8_t *rdata, uint16_t datalen);
//@ 使用SPI总线收发数据
void My_SPI_MasterTransmitReceive(SPI_TypeDef *SPIx, const uint8_t *pDataTx, uint8_t *pDataRx, uint16_t Size);
