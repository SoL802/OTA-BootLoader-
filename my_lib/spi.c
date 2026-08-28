 /*
  * @file    spi.c
  */
#include "spi.h"
//spi1 初始化
//cs PA4 
//D0 PA6  MISO
//D1 PA7  MOSI
//SCLK PA5
void App_SPI1_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_4;//cs
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_2MHz;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	GPIO_WriteBit(GPIOA,GPIO_Pin_4,Bit_SET);//系统上电 / GPIO 初始化完成后，CS 必须是高电平

	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_5;//SCLK
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_2MHz;
	GPIO_Init(GPIOA,&GPIO_InitStruct);

	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_6;//MISO
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IN_FLOATING;//输入浮空模式
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_2MHz;
	GPIO_Init(GPIOA,&GPIO_InitStruct);

	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_7;//MOSI
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_2MHz;
	GPIO_Init(GPIOA,&GPIO_InitStruct);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);
	SPI_InitTypeDef SPI_InitStruct;
	SPI_InitStruct.SPI_Mode=SPI_Mode_Master;
	SPI_InitStruct.SPI_Direction=SPI_Direction_2Lines_FullDuplex;
	SPI_InitStruct.SPI_DataSize=SPI_DataSize_8b;
	SPI_InitStruct.SPI_CPOL=SPI_CPOL_Low;
	SPI_InitStruct.SPI_CPHA=SPI_CPHA_1Edge;
	SPI_InitStruct.SPI_FirstBit=SPI_FirstBit_MSB;
	SPI_InitStruct.SPI_BaudRatePrescaler=SPI_BaudRatePrescaler_64;
	SPI_InitStruct.SPI_NSS=SPI_NSS_Soft;
	SPI_InitStruct.SPI_CRCPolynomial=7;
	SPI_Init(SPI1,&SPI_InitStruct);

	SPI_NSSInternalSoftwareConfig(SPI1,SPI_NSSInternalSoft_Set);
}
/** 
 * @brief  SPI1 全双工读写一个字节
 * @param  txd: 要发送的字节
 * @retval 接收到的字节
 * @note   SPI 是全双工：发 1 字节的同时，必然收 1 字节
 */
uint8_t SPI1_ReadWriteByte(uint8_t txd)
{
        /*----------------------------------------------------------
     * 步骤 1：等待发送缓冲区空（TXE = 1）
     *----------------------------------------------------------
     * TXE (Transmit Buffer Empty)
     * - 0：发送缓冲区非空，不能写新数据
     * - 1：发送缓冲区空，可以写下一个数据
     *
     * 为什么必须等？
     * - SPI 发送是“移位”过程，不是瞬间完成
     * - 如果上一次数据还没发完就写 TDR，
     *   会覆盖数据或导致硬件异常
     *
     */

    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
   /*----------------------------------------------------------
     * 步骤 2：写入要发送的数据
     *----------------------------------------------------------
     * SPI_I2S_SendData() 实际是写 SPI->DR（数据寄存器）
     * - 写 DR 后，数据会被搬到发送移位寄存器
     * - 硬件自动产生 SCK 时钟
     *
     * 此时：
     * - MOSI 上开始发送 txd 的最高位（MSB）
     * - SCK 开始翻转
     */
    SPI_I2S_SendData(SPI1, txd);
/*----------------------------------------------------------
     * 步骤 3：等待接收缓冲区非空（RXNE = 1）
     *----------------------------------------------------------
     * RXNE (Receive Buffer Not Empty)
     * - 0：接收缓冲区空
     * - 1：接收完成，可以读数据
     *
     * 为什么必须等？
     * - SPI 是全双工的：
     *   发第 N 字节的同时，也在收第 N 字节
     * - 只有 8 位全部移入完成后，RXNE 才会置位
     *
     * 注意：
     * - 这个“收到”的字节，正是刚才发送 txd 期间
     *   从 MISO 线上移入的数据
     */
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
    /*
     * 步骤 4：读取接收到的数据
     *----------------------------------------------------------
     * 读 DR 会自动清除 RXNE 标志
     *
     * 在 W25Q64 场景下的典型用法：
     * - 发命令 / 地址时：返回值通常丢弃
     * - 读数据时：txd = 0xFF（产生时钟），返回值有效
     */
    return SPI_I2S_ReceiveData(SPI1);
}
void SPI1_Write(uint8_t *wdata, uint16_t datalen)
{
    uint16_t i;
    for(i = 0; i < datalen; i++)
    {
        SPI1_ReadWriteByte(wdata[i]);
    }
}
void SPI1_Read(uint8_t *rdata, uint16_t datalen)
{
    uint16_t i;
    for(i = 0; i < datalen; i++)
    {
        rdata[i] = SPI1_ReadWriteByte(0xFF);
    }
}

// @简介：使用SPI总线以主机的身份收发数据
//
// @参数：SPIx    - 所用的SPI接口的名称，可以填SPI1或SPI2
// @参数：pDataTx - 要发送的数据（数组）
// @参数：pDataRx - 接收数据缓冲区（数组），从从机接收到的数据被保存在这个参数当中
// @参数：Size    - 要收发数据的数量，以字节为单位
//
void My_SPI_MasterTransmitReceive(SPI_TypeDef *SPIx, const uint8_t *pDataTx, uint8_t *pDataRx, uint16_t Size)
{
	if(Size == 0) return;
	
	// #1. 闭合总开关
	SPI_Cmd(SPIx, ENABLE);
	
	// #2. 写入第一个字节
	SPI_I2S_SendData(SPIx, pDataTx[0]);
	
	// #3. 读写Size-1个字节
	for(uint16_t i=0; i<Size-1; i++)
	{
		// 向TDR写数据
		while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == RESET);
		
		SPI_I2S_SendData(SPIx, pDataTx[i+1]);
		
		// 从RDR读数据
		while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_RXNE) == RESET);
		
		pDataRx[i] = SPI_I2S_ReceiveData(SPIx);
	}
	
	// #4. 读取最后一个字节
	while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_RXNE) == RESET);
	
	pDataRx[Size-1] = SPI_I2S_ReceiveData(SPIx);
	
	// #5. 断开总开关
	SPI_Cmd(SPIx, DISABLE);
}
