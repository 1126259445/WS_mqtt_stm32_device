#ifndef __SPI_H
#define __SPI_H

#include "stm32f4xx.h"
void SPI1_Init(void);
u8 SPI1_ReadWriteByte(u8 TxData);//SPI总线读写一个字节
		 
#endif

