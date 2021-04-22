#ifndef __TICK_H
#define __TICK_H
#include "stm32f4xx_it.h"

#define uwTick	(TIM5->CNT)

void TickInit(void);
uint8_t TimInterval(uint32_t *a, uint32_t interval);	/* ʱ���� */
uint8_t TimWait(uint32_t *last_tick, uint32_t wait);	/* ʱ��ȴ� */

#endif /* __TICK_H */

/* end of file */
