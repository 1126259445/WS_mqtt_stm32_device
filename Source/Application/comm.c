#include "comm.h"
#include "FreeRTOS.h"
#include "task.h"
#include "usart.h"


void serial_common(void *pvParameter)
{
	while(1)
	{
		vTaskDelay(10);
		MSG_USART_Send(DMA1_Stream6 ,  DMA_FLAG_TCIF6, &redio_tx_queue);
		MSG_USART_Send(DMA1_Stream3 ,  DMA_FLAG_TCIF3, &net_tx_queue);
	}
}


void common()
{
	xTaskCreate(serial_common,"serial_common",configMINIMAL_STACK_SIZE,NULL,tskIDLE_PRIORITY + 4,NULL);
}

