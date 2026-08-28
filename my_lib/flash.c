#include "stm32f10x.h" 
#include "main.h"
#include "flash.h"//闪存控制器
//以页为单位擦除
//按字写入一次写四个字节
void App_Flash_Erase(uint16_t start,uint16_t num)//A区擦除 A起始扇区 A区个数
{
    uint16_t i;
    FLASH_Unlock();
       FLASH_ClearFlag(
        FLASH_FLAG_EOP
        | FLASH_FLAG_PGERR
        | FLASH_FLAG_WRPRTERR
    );
    for(i=start;i<num;i++)
    {
        FLASH_ErasePage(FLASH_SADDR + ((uint32_t)start + i) * PAGE_SIZE);
    }
    FLASH_Lock();
}
void App_WriteFlash(uint32_t startaddr,uint32_t *wdata,uint32_t write_num)
{ 
    //检查数据指针
    if (wdata == 0) return;
    //没有数据需要写入
    if (write_num == 0) return;
     // 写入字节数必须是 4 的倍数，因为 FLASH_ProgramWord 一次写入 4 字节。
    if ((write_num & 0x03) != 0)return;

     //Flash 字地址必须 4 字节对齐
    if ((startaddr & 0x03) != 0)return;
    //检查写入大小是否为4的倍数
    if(write_num%4!=0)return;
    //解锁
    FLASH_Unlock();
   // 清除之前可能残留的 Flash 状态标志
    FLASH_ClearFlag(
        FLASH_FLAG_EOP
        | FLASH_FLAG_PGERR
        | FLASH_FLAG_WRPRTERR);
 //循环写入
 //按字写入
 //每次写入4字节
while(write_num)
{
    FLASH_ProgramWord(startaddr,*wdata);
    write_num-=4;
    startaddr+=4;
    wdata++;
}
//上锁
FLASH_Lock();
}
