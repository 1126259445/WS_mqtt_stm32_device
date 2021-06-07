#include "stm32f4xx_it.h"
#include "FreeRTOS.h"
#include "task.h"
#include "data_process.h"
#include "mqtt_client.h"
#include "enc28j60.h"	 


/*���ݴ�������*/
void data_task(void *pvParameter)
{
	while(1)
	{
		vTaskDelay(20);
		
		if(mqtt_cmd.Switch & (1 << 0))
		{
			ENC28J60_Reset();
			
			//__set_FAULTMASK(1); // �ر������ж�
			NVIC_SystemReset(); // ��λ
		}
	}
}


void data_process()
{
	xTaskCreate(data_task,"data_task",configMINIMAL_STACK_SIZE,NULL,tskIDLE_PRIORITY + 4,NULL);
}

