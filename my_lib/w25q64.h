#ifndef __W25Q64_H__
#define __W25Q64_H__    
#include "stm32f10x.h"	

void App_W25Q64_Init(void);
void App_W25Q64_WaitBusy(void);
void App_W25Q64_Enable(void);
void App_W25Q64_PageWrite(uint8_t *wbuff, uint16_t pagenum);
void App_W25Q64_Erase64K(uint8_t blocknum);
void App_W25Q64_Read(uint8_t *rbuff, uint32_t addr,uint32_t datalen);

#endif
