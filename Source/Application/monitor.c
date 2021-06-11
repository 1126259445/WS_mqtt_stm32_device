#include "FreeRTOS.h"
#include "task.h"
#include "watchdog.h"
#include "monitor.h"
#include "signaling.h"

/*Ñ­»·¼à¿ØÏµÍ³×´Ì¬*/
void monitor_loop(void *pvParameter)
{
	while(1)
	{
		vTaskDelay(100);
		
		IWDG_Feed();
		
		/*ÐÅºÅµÆ100ms */
		signalingEvent(0);
	}
}


void monitor()
{
	xTaskCreate(monitor_loop,"monitor_loop",configMINIMAL_STACK_SIZE,NULL,tskIDLE_PRIORITY + 4,NULL);
}

