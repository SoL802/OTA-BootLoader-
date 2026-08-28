#ifndef __AT24C02_H__
#define __AT24C02_H__
#include "stm32f10x.h"
#define AT24C02_WADDR 0xA0
#define AT24C02_RADDR 0xA1
#define AT24C02_I2C_PORT  I2C1
#define AT24C02_PAGE_SIZE 8//页大小
#define OTA_INFO_EEPROM_ADDR 0x20  // OTA信息从EEPROM第32字节开始存
uint8_t AT24C02_WriteByte(uint8_t Addr, uint8_t Data);
uint8_t AT24C02_WritePage(uint8_t Addr, uint8_t *writeData);
uint8_t AT24C02_ReadData(uint8_t Addr, uint8_t *pData, uint16_t datalen);
//void AT24C02_ReadOTAInfo(void);
void AT24C02_WriteOTAInfo(void);
int AT24C02_ReadOTAInfo(void);
#endif
