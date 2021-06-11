#ifndef __digital_h
#define __digital_h
#include "stm32f4xx_it.h"

#define PERIPH2BB(addr, bit)        ((uint32_t *)(PERIPH_BB_BASE + ((addr) - PERIPH_BASE) * 32 + ((bit) * 4)))

typedef uint32_t * digitalPin_t;

#define digitalHigh(p)      (*p = 1)
#define digitalLow(p)       (*p = 0)
#define digitalSet(p, n)    (*p = n)
#define digitalGet(p)       (*p)
#define digitalToggle(p)    *p = !(*p)

extern digitalPin_t digitalInit(GPIO_TypeDef* port, const uint16_t pin, uint8_t initial);

#endif /* __digital_h */

/* end of file */
