#include "stm32f10x.h"
#include "bootloader.h"
#include "main.h"
#include "usart.h"
#include "delay.h" 
#include "i2c.h"
#include "at24c02.h"
#include "spi.h"
#include "w25q64.h"
#include "flash.h"
#include <string.h>
//分区跳转两大任务
//1.设置sp,A区起始地址为0x08005000 ,给到sp
//2.设置pc,A区起始地址为0x08005000+4 ,改到pc 
//把B区用到的外设，寄存器reset
//32单片机一个寄存器占4字节，所以pc需要跳转到A区起始地址+4字节
//sp和pc指针是每4字节读取flash地址
//RAM地址是0x20000000 -0x20004fff
Load_a Load_A;// 声明一个函数指针变量 Load_A



//BootLoader分支判断
void Bootloader_Branch(void)
{
	if(Bootloader_Enter(50)==0)
{
	if(OTA_info.OTA_flag==OTA_SET_FLAG)  //判断OTA_flag是不是OTA_SET_FLAG定义的值，是的话进入if
 {
    //A区设置标志位存在，A区为新程序
  My_USART_Printf(USART1,"OTA更新\r\n");//串口1输出信息
	BootStatusFlag|=UPDATEA_A_FLAG;       //置位标志位，表明需要更新A区
	UpData_a.W25Q64_BlockNumber=0;        //W25Q64_BlockNumber是不是OTA_SET_FLAG定义的值，不是的话进入else
 }
 else
 {
    //A区设置标志位不存在，A区为旧程序
   My_USART_Printf(USART1,"跳转A区\r\n");//
   Load_A_Program(A_PAGE_StartAddr);     //跳转到A区
 }
}

else
 {
	My_USART_Printf(USART1,"进入BootLoader命令行\r\n");
	Bootloader_Information(); 
 }

}


//enter
uint8_t Bootloader_Enter(uint8_t timeout)
{
	My_USART_Printf(USART1,"%dms内，输入小写字母 w ，进入bootloader命令行\r\n",timeout*100);
	while(timeout--)
	{
		Delay(100);
		if(USART_Rx_Buffer[0]=='w')
		{
			return 1; //进入命令行
		}
	}
	return 0;     //不进入命令行
}


void Bootloader_Information(void)
{
	My_USART_Printf(USART1,"\r\n");
	My_USART_Printf(USART1,"[1]擦除A区\r\n");
	My_USART_Printf(USART1,"[2]串口IAP下载\r\n");
	My_USART_Printf(USART1,"[3]设置OTA版本号\r\n");
	My_USART_Printf(USART1,"[4]查询OTA版本号\r\n");
	My_USART_Printf(USART1,"[5]向外部FLASH下载程序\r\n");
	My_USART_Printf(USART1,"[6]使用外部FLASH内程序\r\n");
	My_USART_Printf(USART1,"[7]重启\r\n");
//	My_USART_Printf(USART1,"[8]\r\n");
//	My_USART_Printf(USART1,"[9]\r\n");
}


//状态机编程
void Bootloader_Event(uint8_t *data,uint16_t datalen)
{
	int temp;
	uint8_t i=0;
	if(BootStatusFlag==0)
	{
	if((datalen==1)&&(data[0]=='1'))
	{
		My_USART_Printf(USART1,"擦除A区\r\n");
		App_Flash_Erase(A_PAGE_Start,A_PAGE_NUM);
	}
		else if((datalen==1)&&(data[0]=='2'))
	{
		My_USART_Printf(USART1,"通过Xmdoem协议，串口IAP下载A区程序，请使用bin格式文件\r\n");
    App_Flash_Erase(A_PAGE_Start,A_PAGE_NUM);
		BootStatusFlag|=(IAP_XMDOEMC_FLAG|IAP_XMDOEMData_FLAG);
		UpData_a.XmdoemTimer=0;
		UpData_a.XmdoemNumber=0;
		
	}
			else if((datalen==1)&&(data[0]=='3'))
	{
		My_USART_Printf(USART1,"设置版本号\r\n");
		BootStatusFlag |= SET_VERSION_FLAG;		
	}
			else if((datalen==1)&&(data[0]=='4'))
	{
		My_USART_Printf(USART1,"查询版本号\r\n");
    AT24C02_ReadOTAInfo();
		My_USART_Printf(USART1,"版本号:%s\r\n",OTA_info.OTA_version);
		Bootloader_Information();
	}
			else if((datalen==1)&&(data[0]=='5'))
	{
		My_USART_Printf(USART1,"向外部FLASH下载程序，输入需要使用的块编号（1-9）\r\n");
    BootStatusFlag |= CMD_5_FLAG;	
	}
			else if((datalen==1)&&(data[0]=='6'))
	{
	 My_USART_Printf(USART1,"使用外部FLASH内的程序，输入需要使用的块编号（1-9）\r\n");	
   BootStatusFlag |= CMD_6_FLAG;
	}
	    else if((datalen==1)&&(data[0]=='7'))
	{
		My_USART_Printf(USART1,"重启\r\n");
		Delay(100);
		NVIC_SystemReset();                //重启
	}
 }
	 else if(BootStatusFlag&IAP_XMDOEMC_FLAG)
		{
			if((datalen==133)&&(data[0]==0x01))
			{
				BootStatusFlag &= ~IAP_XMDOEMC_FLAG;
				UpData_a.XmdoemCRC=Xmdoem_CRC16(&data[3],128);
				if(UpData_a.XmdoemCRC==data[131]*256+data[132])
				{
					UpData_a.XmdoemNumber++;
					memcpy(&UpData_a.Updatabuff[((UpData_a.XmdoemNumber-1)%(PAGE_SIZE/128))*128],&data[3],128);
					if((UpData_a.XmdoemNumber%(PAGE_SIZE/128))==0)
					{
						if(BootStatusFlag&CMD5_XMDOEMD_FLAG){
							for(i=0;i<4;i++){
							App_W25Q64_PageWrite(&UpData_a.Updatabuff[i*256],(UpData_a.W25Q64_BlockNumber/8-1)*4+i +UpData_a.W25Q64_BlockNumber*64*4);//1024/256=4
							}
						}
						else{
						App_WriteFlash((((UpData_a.XmdoemNumber/(PAGE_SIZE/128))-1)*PAGE_SIZE+A_PAGE_StartAddr),(uint32_t *)UpData_a.Updatabuff,PAGE_SIZE);//写入到单片机A区相应的扇区
						}
					}
					My_USART_Printf(USART1,"\x06");
				}
				else{
					My_USART_Printf(USART1,"\x15");
				}
			}
		if((datalen==1)&&(data[0]==0x04))
		{
        My_USART_Printf(USART1,"\x06");
								if((UpData_a.XmdoemNumber%(PAGE_SIZE/128))!=0)
					{
						if(BootStatusFlag&CMD5_XMDOEMD_FLAG){
								for(i=0;i<4;i++){
							App_W25Q64_PageWrite(&UpData_a.Updatabuff[i*256],(UpData_a.W25Q64_BlockNumber/8)*4+i +UpData_a.W25Q64_BlockNumber*64*4);
							}
						}
						else{
						App_WriteFlash((((UpData_a.XmdoemNumber/(PAGE_SIZE/128)))*PAGE_SIZE+A_PAGE_StartAddr),(uint32_t *)UpData_a.Updatabuff,(UpData_a.XmdoemNumber%(PAGE_SIZE/128))*128);//写入到单片机A区相应的扇区
						}
					}
					BootStatusFlag &= ~IAP_XMDOEMData_FLAG;
					if(BootStatusFlag&CMD5_XMDOEMD_FLAG){
						BootStatusFlag&= ~CMD5_XMDOEMD_FLAG;
						OTA_info.Filelen[UpData_a.W25Q64_BlockNumber]=UpData_a.XmdoemNumber*128;
						AT24C02_WriteOTAInfo();
						Delay(100);
						Bootloader_Information();
					}
					else{
				  Delay(100);
		      NVIC_SystemReset();                //重启
					}
		 }			
	}
	else	if(BootStatusFlag&SET_VERSION_FLAG)
		{
			if((datalen==26))
			{
				if(sscanf((char *)data,"VER-%d.%d.%d-%d/%d/%d-%d:%d",&temp,&temp,&temp,&temp,&temp,&temp,&temp,&temp)==8){
					memset(OTA_info.OTA_version,0,32);
					memcpy(OTA_info.OTA_version,data,26);
					AT24C02_WriteOTAInfo();
					My_USART_Printf(USART1,"版本正确\r\n");
					BootStatusFlag&= ~SET_VERSION_FLAG;
					Bootloader_Information();
				}
				else{
					My_USART_Printf(USART1,"版本号格式错误\r\n");
				}
			}
			else My_USART_Printf(USART1,"版本号长度错误\r\n");
		}
		else	if(BootStatusFlag&CMD_5_FLAG) 
		{
			if(datalen==1){
				if((data[0]>=0x31)&&(data[0]<=0x39)){
					UpData_a.W25Q64_BlockNumber= data[0]-0x30;
					BootStatusFlag|=(IAP_XMDOEMC_FLAG|IAP_XMDOEMData_FLAG|CMD5_XMDOEMD_FLAG);
					UpData_a.XmdoemTimer=0;
		      UpData_a.XmdoemNumber=0;
					OTA_info.Filelen[UpData_a.W25Q64_BlockNumber]=0;
					App_W25Q64_Erase64K(UpData_a.W25Q64_BlockNumber);
					My_USART_Printf(USART1,"通过Xmdoem协议，向外部FLASH第%d个块下载程序，请使用bin格式文件\r\n",UpData_a.W25Q64_BlockNumber);
					BootStatusFlag&= ~CMD_5_FLAG;
					
				}else My_USART_Printf(USART1,"编号错误\r\n");
			}
			else My_USART_Printf(USART1,"数据长度错误\r\n");
		}
				else	if(BootStatusFlag&CMD_6_FLAG) 
		{
			if(datalen==1){
				if((data[0]>=0x31)&&(data[0]<=0x39)){
					UpData_a.W25Q64_BlockNumber= data[0]-0x30;
					BootStatusFlag|=UPDATEA_A_FLAG;       //置位标志位，表明需要更新A区
					BootStatusFlag&= ~CMD_6_FLAG;
					
				}else My_USART_Printf(USART1,"编号错误\r\n");
			}
			else My_USART_Printf(USART1,"数据长度错误\r\n");
		}
}
//函数名 设置sp
//参数    addr ：栈顶指针初始值
//返回值 无
__asm void MSR_SP(uint32_t addr)
{
    MSR MSP,r0   //addr的值加载到了r0通用寄存器，然后通过MSR指令，将通用寄存器r0的值写入到MSP主堆栈指针
    BX r14       //返回调用MSR_SP函数的主函数
}
//跳转A区
//addr:A区的起始地址
void Load_A_Program(uint32_t addr)
{
  //加载A区程序
if((*(uint32_t *)addr>=0x20000000)&&((*(uint32_t *)addr<=0x20004fff)))  //判断sp栈顶指针的范围是否合法，在对应型号的RAM控件范围内
{ 
    MSR_SP(*(uint32_t *)addr);                                          //设置sp

  Load_A=(Load_a)*(uint32_t *)(addr+4);                                 //将函数指针Load_a指向A区的复位向量// 从地址 (addr + 4) 处读取一个 32 位数据，
  // 强制转换为函数指针，并赋值给 Load_A
  Bootloader_Clear();                                                   //清除B区使用的外设
  Load_A(); // 通过函数指针调用函数                                     //调用函数指针Load_A，改变PC指针，从而转向A区的复位向量位置，完成跳转
}
else My_USART_Printf(USART1,"跳转A分区失败\r\n");
}
//清除B区使用的外设
void Bootloader_Clear(void){
	USART_DeInit(USART1);//复位串口
  GPIO_DeInit(GPIOA);  //复位GPIO
  GPIO_DeInit(GPIOB);
 

}
uint16_t Xmdoem_CRC16(uint8_t *data,uint16_t datalen)
{
	uint8_t i;
	uint16_t Crc_init =0x0000;//初始值
	uint16_t Crc_poly=0x1021;  //多项式
	while(datalen--)
	{
		Crc_init=(*data<<8)^Crc_init;
		for(i=0;i<8;i++)
		{
			if(Crc_init&0x8000)
				Crc_init=(Crc_init<<1)^Crc_poly;
			else
				Crc_init=(Crc_init<<1);
		}
		data++;
	}
	return Crc_init;
}


