#include "FreeRTOS.h"
#include "task.h"
#include "spi.h"
#include "net_task.h"
#include "lwip/udp.h"
#include "lwip/init.h"
#include "lwip/ip4_addr.h"
#include "lwip/timeouts.h"
#include "lwip/apps/mqtt.h"
#include "lwip/apps/mqtt_priv.h"

#include "mqtt_client.h"
#include "SEGGER_RTT.h"
#include "tick.h"

extern err_t ethernetif_init(struct netif *netif);
extern err_t ethernet_input(struct pbuf *p, struct netif *netif);
extern void ethernetif_input(struct netif *netif);


static ip_addr_t ipaddr, netmask, gw; //����IP��ַ
struct netif enc28j60_netif;  //��������ӿ�


/*���������������*/
void LwIP_Config (void)
{   
    IP4_ADDR(&ipaddr, 192, 168, 1, 168);  		//���ñ���ip��ַ
    IP4_ADDR(&gw, 192, 168, 1, 1);			    //����
    IP4_ADDR(&netmask, 255, 255, 255, 0);		//��������	 
	
	// ��ʼ��LWIPЭ��ջ,ִ�м���û����п����õ�ֵ����ʼ�����е�ģ��
	lwip_init();
	// �������ӿ�
	while ((netif_add(&enc28j60_netif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &ethernet_input)==NULL))
	{
         vTaskDelay(2);
	}
	netif_set_default(&enc28j60_netif); // ע��Ĭ�ϵ�����ӿ�
 	netif_set_up(&enc28j60_netif);      // ��������ӿ����ڴ���ͨ��
}


/*�������ݴ���*/
void net_process(void *pvParameter)
{
	uint32_t last_mqtt_time = 0;
	LwIP_Config();
	mqtt_example_init();
	
	while(1)
	{
		vTaskDelay(5);
		
		/*��������*/
		ethernetif_input(&enc28j60_netif); 
		/* �޲���ϵͳ��ʱ��� */
		sys_check_timeouts();
		
		if(last_mqtt_time + 500 < uwTick)
		{
			last_mqtt_time = uwTick;

			if(mqtt_client_is_connected(mqtt_client))
			{
				joson_create_uav_data_send();
			}
			else
			{
				mqtt_disconnect(mqtt_client);
				mqtt_client_free(mqtt_client);
				mqtt_example_init();
			}
		}
	}
}


void net_task()
{
	xTaskCreate(net_process,"net_process",configMINIMAL_STACK_SIZE*30,NULL,tskIDLE_PRIORITY + 4,NULL);
}

