#ifndef __BOOTLOADER_H_
#define __BOOTLOADER_H_ 
#include "stm32f10x.h"   // Device header
#include "main.h"
typedef void (*Load_a)(void);//函数指针，调用指针相当于调用函数
void Bootloader_Branch(void);
uint8_t Bootloader_Enter(uint8_t timeout);
void Bootloader_Event(uint8_t *data,uint16_t datalen);
void Bootloader_Information(void);
void Load_A_Program(uint32_t addr);
void Bootloader_Clear(void);
uint16_t Xmdoem_CRC16(uint8_t *data,uint16_t datalen);
#endif 
//A区将 OTA_FLAG变成对勾
//A区下载完毕 OTA_FLAG 变成对勾
//OTA时，最新的程序文件下载在哪？ 
//分片下载，[256],下载到W25Q64中

//OTA时，最新版本的程序文件如何下载? 下载多少？
//服务区下发告诉我们最新版本的程序文件大小，字节数
//下载多少个变量需要保存

//发生OTA事件时，B区如何更新A区？
//根据AT24C02的下载量，拿数据(每次1024字节)，写到A区


