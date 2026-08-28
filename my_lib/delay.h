#ifndef _DELAY_H_
#define _DELAY_H_

#include "stm32f10x.h"

    void Delay_Init(void); // 延迟函数初始化
    void Delay(uint32_t ms); // 延迟
uint32_t GetTick(void); // 获取系统的当前时间
uint64_t GetUs(void); // 获取当前的微秒级时间
    void DelayUs(uint32_t us); // 微秒级延迟

#endif
