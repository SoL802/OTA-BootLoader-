#include "stm32f10x.h"
#include "main.h"
#include "usart.h"
#include "delay.h" 
#include "i2c.h"
#include "at24c02.h"
#include "spi.h"
#include "w25q64.h"
#include "flash.h"
#include "bootloader.h"
//spi1 初始化
//cs PA4
//D0 PA6
//D1 PA7
//SCLK PA5
//硬件iic 
//PB6 SCL
//PB7 SDA
//flash w25q64
//usart TXD->PA10
//usart RXD->PA9
//w25q64 按页写入 16bytes
OTA_INFOCB OTA_info;//保存在24c02内的OTA相关的结构体
UpData_A UpData_a;//A区更新结构体
uint32_t BootStatusFlag;//记录全局状态标志位
//uint8_t a[5]={1,2,3,4,5};
int main(void)
{
	uint8_t i;//用于for循环
	
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//中断优先级分组2
  usart_init();//串口初始化
  My_I2C_Init();//I2C初始化
  Delay_Init();//延时初始化
  App_SPI1_Init();//SPI初始化
  App_W25Q64_Init();//25Q64初始化
	AT24C02_ReadOTAInfo();//从24C02读取数据到OTA_Info结构体


//	My_USART_Printf(USART1,"%x\r\n",Xmdoem_CRC16(a,5));  例子 8208

	
	
  Bootloader_Branch();//分支判断
	//主循环
  while(1)
	{
		Delay(10);
 	 if(USART_Rx_Ptr.URxDataOut!=USART_Rx_Ptr.URxDataIn)	
{
	Bootloader_Event(USART_Rx_Ptr.URxDataOut->start,USART_Rx_Ptr.URxDataOut->end-USART_Rx_Ptr.URxDataOut->start+1);
  USART_Rx_Ptr.URxDataOut++;
   if(USART_Rx_Ptr.URxDataOut == USART_Rx_Ptr.URxDataEnd)
  {
    USART_Rx_Ptr.URxDataOut=&USART_Rx_Ptr.URxData[0];
  }
}
		 if(BootStatusFlag&IAP_XMDOEMC_FLAG)
		 {
			 if(UpData_a.XmdoemTimer>=100)
			 {
				My_USART_Printf(USART1,"C");
        UpData_a.XmdoemTimer=0;  				 
			 }
			UpData_a.XmdoemTimer++;
		 }

		 
		 
		 
		 		//UPDATEA_A_FLAG复位，表明需要更新A区
		if((BootStatusFlag&UPDATEA_A_FLAG))
		{
			//A区更新
			My_USART_Printf(USART1,"长度%d字节\r\n",OTA_info.Filelen[UpData_a.W25Q64_BlockNumber]);//USART1输出信息
			if(OTA_info.Filelen[UpData_a.W25Q64_BlockNumber]%4==0)                //判断长度是否是4的整数倍，是的话进入if
		 {
			 //这个地方目前有问题，，，，！
			 App_Flash_Erase(A_PAGE_Start,A_PAGE_NUM);                            //擦除A区FLASH
				
				//4字节对齐
			 for(i=0;i<OTA_info.Filelen[UpData_a.W25Q64_BlockNumber]/PAGE_SIZE;i++) //每次读写一个数据，使用for循环，写入整数个扇区
				{
					App_W25Q64_Read( UpData_a.Updatabuff,i*1024+UpData_a.W25Q64_BlockNumber*64*1024,PAGE_SIZE);//先从W25Q64读取一个单片机扇区的数据
					App_WriteFlash(i*PAGE_SIZE+A_PAGE_StartAddr,(uint32_t *)UpData_a.Updatabuff,PAGE_SIZE);//写入到单片机A区相应的扇区
				}
				if(OTA_info.Filelen[UpData_a.W25Q64_BlockNumber]%1024!=0)//判断是否还有一个不足一个完整扇区的数据，有的话进入if
			{
				App_W25Q64_Read( UpData_a.Updatabuff,i*1024+UpData_a.W25Q64_BlockNumber*64*1024,OTA_info.Filelen[UpData_a.W25Q64_BlockNumber]%1024);//从W25Q64读取不足一个完整扇区的数据
				App_WriteFlash(i*PAGE_SIZE+A_PAGE_StartAddr,(uint32_t *)UpData_a.Updatabuff,OTA_info.Filelen[UpData_a.W25Q64_BlockNumber]%1024);         //然后写入单片机A区相应的扇区
			}
			if(UpData_a.W25Q64_BlockNumber==0)//如果W25Q64_BlockNumber是0，表示是OTA更新A区，进入if
	{
		OTA_info.OTA_flag=0;                //设置OTA_flag，只要不是OTA_SET_FLAG的值即可     
		AT24C02_WriteOTAInfo();             //写入24C02内保存
	}		
	   My_USART_Printf(USART1,"A区更新完毕\r\n");
     NVIC_SystemReset();                //重启
		 }
		 else                               //判断长度是否是4的整数倍，不是的话进入else
			{
			My_USART_Printf(USART1,"长度错误\r\n"); 
				BootStatusFlag&=~ UPDATEA_A_FLAG;//清除UPDATEA_A_FLAG标志位
			}

		}
		 
		 
	}
}

