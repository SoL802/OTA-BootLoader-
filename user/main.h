#ifndef __MAIN_H_
#define __MAIN_H_
#include "stm32f10x.h"
#define FLASH_SADDR 0x08000000         //flash起始地址
#define PAGE_SIZE 1024                 //flash扇区大小
#define PAGE_NUM 64                    //flash扇区总数量
#define B_PAGE_NUM 20                  //B区扇区数量
#define A_PAGE_NUM  PAGE_NUM-B_PAGE_NUM//A区扇区数量
#define A_PAGE_Start B_PAGE_NUM        //A区起始扇区编号
#define A_PAGE_StartAddr (FLASH_SADDR+(A_PAGE_Start*PAGE_SIZE))//A区起始地址
#define OTA_SET_FLAG ((uint32_t)  0xAABB1122)//OTA_flag对应值，如果相等，说明需要OTA更新
#define UPDATEA_A_FLAG   0X00000001//A区更新标志位
#define IAP_XMDOEMC_FLAG 0x00000002//
#define IAP_XMDOEMData_FLAG 0x00000004
#define SET_VERSION_FLAG 0x00000008
#define CMD_5_FLAG 0x00000010
#define CMD5_XMDOEMD_FLAG 0x00000020
#define CMD_6_FLAG 0x00000040
//擦除只擦a区
typedef struct{
uint32_t OTA_flag;     //A区设置标志位
	uint32_t Filelen[11];//0号成员固定对应OTA大小 W25Q64中不同块中程序固件的长度，0号成员固定对应W25Q64中编码的0块，用于OTA
	uint8_t OTA_version[32];//版本号
}OTA_INFOCB;           //OTA相关的信息结构体，需要保存到24C02
#define OTA_INFOCB_SIZE sizeof(OTA_INFOCB) //OTA相关的信息结构体占用的字节长度
 
typedef struct{
	uint8_t Updatabuff[PAGE_SIZE];//更新A区时，用于保存从W25Q64中读取的数据
  uint32_t W25Q64_BlockNumber;  //用于记录从哪个W25Q64的块中读取数据
	uint32_t XmdoemTimer;         //延时变量
	uint32_t XmdoemNumber;
	uint32_t XmdoemCRC;
	}UpData_A;                    //更新A区用的结构体

extern OTA_INFOCB OTA_info;// 外部变量声明
extern UpData_A UpData_a;
extern uint32_t BootStatusFlag;
#endif 
