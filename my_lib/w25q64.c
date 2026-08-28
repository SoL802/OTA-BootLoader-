#include "w25q64.h"
#include "spi.h"
void App_W25Q64_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_4;//cs
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_2MHz;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	GPIO_WriteBit(GPIOA,GPIO_Pin_4,Bit_SET);//系统上电 / GPIO 初始化完成后，CS 必须是高电平
  App_SPI1_Init();
}
void App_W25Q64_WaitBusy(void)
{
	uint8_t res;
  do
  {
    GPIO_WriteBit(GPIOA,GPIO_Pin_4,Bit_RESET);//cs 低电平，发送命令
    SPI1_ReadWriteByte(0X05);
    res=SPI1_ReadWriteByte(0xff);
    GPIO_WriteBit(GPIOA,GPIO_Pin_4,Bit_SET);//cs 高电平
  }while((res&0x01)!=0);
}
void App_W25Q64_Enable(void)
{
    App_W25Q64_WaitBusy();
	GPIO_WriteBit(GPIOA,GPIO_Pin_4,Bit_RESET);
    SPI1_ReadWriteByte(0X06);
    GPIO_WriteBit(GPIOA,GPIO_Pin_4,Bit_SET);

}
void App_W25Q64_Erase64K(uint8_t blocknum)
{
  uint8_t wdata[4];
  wdata[0]=0xD8;
  wdata[1]=(blocknum*64*1024)>>16;
  wdata[2]=(blocknum*64*1024)>>8;
  wdata[3]=(blocknum*64*1024)>>0;
  App_W25Q64_WaitBusy();
  App_W25Q64_Enable();
  GPIO_WriteBit(GPIOA,GPIO_Pin_4,Bit_RESET);
  SPI1_Write(wdata,4);
  GPIO_WriteBit(GPIOA,GPIO_Pin_4,Bit_SET);
  App_W25Q64_WaitBusy();
}
void App_W25Q64_PageWrite(uint8_t *wbuff, uint16_t pagenum)
{
  uint8_t wdata[4];
  wdata[0]=0x02;
  wdata[1]=(pagenum*256)>>16;
  wdata[2]=(pagenum*256)>>8;
  wdata[3]=(pagenum*256)>>0;
  App_W25Q64_WaitBusy();
  App_W25Q64_Enable();
  GPIO_WriteBit(GPIOA,GPIO_Pin_4,Bit_RESET);
  SPI1_Write(wdata,4);
  SPI1_Write(wbuff,256);
  GPIO_WriteBit(GPIOA,GPIO_Pin_4,Bit_SET);


}
void App_W25Q64_Read(uint8_t *rbuff, uint32_t addr,uint32_t datalen)//数据 偏移量 长度
{
  uint8_t rdata[4];
  rdata[0]=0x03;
  rdata[1]=(addr)>>16;
  rdata[2]=(addr)>>8;
  rdata[3]=(addr)>>0;
  App_W25Q64_WaitBusy();
  GPIO_WriteBit(GPIOA,GPIO_Pin_4,Bit_RESET);
  SPI1_Write(rdata,4);
  SPI1_Read(rbuff,datalen);
  GPIO_WriteBit(GPIOA,GPIO_Pin_4,Bit_SET);
}

