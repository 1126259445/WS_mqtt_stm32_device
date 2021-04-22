#include "tick.h"

/* ����ֱ�Ӷ�ȡTIM5��ʱ��ֵ����Ϊ����ϵͳ������ʱ�� */

void TickInit(void)
{
	/* TIM5ʱ��Ϊ60MHz����Ҫ���ó�1ms�ļ������� */
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);

	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period = 0xffffffff; //t is the time between each Timer irq.
	TIM_TimeBaseInitStructure.TIM_Prescaler = (60000 - 1); //t = (1+TIM_Prescaler/SystemCoreClock)*(1+TIM_Period)
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseInitStructure);

	TIM_Cmd(TIM5, ENABLE);
}

uint8_t TimInterval(uint32_t *last_tick, uint32_t interval)
{
	if (*last_tick + interval < uwTick)
	{
		*last_tick = uwTick;
		return 1;
	}
	
	return 0;
}

uint8_t TimWait(uint32_t *last_tick, uint32_t wait)
{
	return (*last_tick + wait < uwTick) ? 1 : 0;
}


/* end of file */
