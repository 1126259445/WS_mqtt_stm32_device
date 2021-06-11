#include "digital.h"

digitalPin_t digitalInit(GPIO_TypeDef *port, const uint16_t pin, uint8_t initial)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    uint32_t *bbAddr;
    uint16_t myPin = pin;
    uint8_t bit;
    
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);

    bit = 0;
    while (myPin >>= 1)
        bit++;

    if (bit < 8)
    {
        bbAddr = PERIPH2BB((uint32_t)port + 0x14, bit);
    }
    else
    {
        bbAddr = PERIPH2BB((uint32_t)port + 0x15, bit-8);
    }

    *bbAddr = initial;

    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = pin;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(port, &GPIO_InitStructure);

    return bbAddr;
}
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/************************** (C) COPYRIGHT AEE Technology ******END OF FILE*****/
