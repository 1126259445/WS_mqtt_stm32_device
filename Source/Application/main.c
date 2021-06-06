#include "stm32f4xx_it.h"
#include "usart.h"
#include "main.h"
#include "watchdog.h"
#include "tick.h"
#include "FreeRTOS.h"
#include "task.h"
#include "comm.h"
#include "net_task.h"
#include "monitor.h"

#include "usbd_conf.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "usbd_cdc_core.h"
#include "usbd_cdc_vcp.h"

__ALIGN_BEGIN USB_OTG_CORE_HANDLE     USB_OTG_dev __ALIGN_END ;


void InitTask(void *pvParameter)
{
	net_task();
	common();
	monitor();
	vTaskDelete(NULL);		/* 删除初始化任务 */
}

int main()
{
	uart1_init(115200);
	uart2_init(115200);
	uart3_init(115200);
	TickInit();
	IWDG_Init(4,500); //与分频数为64,重载值为500,溢出时间为1s	
	
//	USBD_Init(&USB_OTG_dev,
//#ifdef USE_USB_OTG_HS 
//	USB_OTG_HS_CORE_ID,
//#else            
//	USB_OTG_FS_CORE_ID,
//#endif            
//	&USR_desc,
//	&USBD_CDC_cb, 
//	&USR_cb);
	
	printf("sys start\r\n");
	/* 新建系统初始化任务 */
	xTaskCreate(InitTask, "InitTask", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
	
	/* FreeRTOS调度器开始工作 */
	vTaskStartScheduler();
	
	/* 此处不会执行到 */
	while (1);
}

