#ifndef __FLASH_H__
#define __FLASH_H__

#include <stdint.h>
#include "stm32f10x.h"

void App_Flash_Erase(uint16_t start, uint16_t num);
void App_WriteFlash(uint32_t startaddr, uint32_t *wdata, uint32_t write_num);

#endif
