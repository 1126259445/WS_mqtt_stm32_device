#ifndef __WITCHDOG_H__
#define __WITCHDOG_H__

#include "stm32f4xx_it.h"

void IWDG_Init(u8 prer,u16 rlr);//IWDG��ʼ��
void IWDG_Feed(void);  //ι������

#endif

