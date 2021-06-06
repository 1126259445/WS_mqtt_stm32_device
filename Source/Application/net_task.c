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
	uint32_t last_send_time = 0;
	uint32_t last_mqtt_time = 0;
	uint32_t last_netif_time = 0;
	
	LwIP_Config();
	mqtt_example_init();
	
	while(1)
	{
		vTaskDelay(5);
		
		/*��������*/
		ethernetif_input(&enc28j60_netif); 
		/* �޲���ϵͳ��ʱ��� */
		sys_check_timeouts();
		
		/*MQTT���ӳɹ���Ĳ���*/
		if(mqtt_client_is_connected(mqtt_client))
		{
			if(last_send_time + 500 < uwTick)
			{
				last_send_time = uwTick;
				joson_create_uav_data_send();
			}
		}
		else
		{
			/*MQTT���ӶϿ��� ���1S�������� */
			if(last_mqtt_time + 1000 < uwTick)
			{
				last_mqtt_time = uwTick;
		
				mqtt_disconnect(mqtt_client);
				mqtt_client_free(mqtt_client);
				mqtt_example_init();
			}	
		}
		
		/*��������״̬���*/
		if (netif_is_link_up(&enc28j60_netif))
		{
			//printf("netif is link up\r\n");
			/* When the netif is fully configured this function must be called ��netif��ȫ����ʱ��������ô˺���*/
			//netif_set_up(&enc28j60_netif);

		}
		else
		{
			/*����Ͽ����Ӻ���500ms���³�ʼ��*/
			if( last_netif_time + 500 < uwTick)
			{
				last_netif_time = uwTick;
				
				printf("netif set down\r\n");
				netif_set_up(&enc28j60_netif);
				/* When the netif link is down this function must be called ��netif���ӶϿ�ʱ����������������*/
				netif_set_down(&enc28j60_netif);
				/*�Ƴ�������������*/
				netif_remove(&enc28j60_netif);
				LwIP_Config();
				
			}
		}		

	}
}


void net_task()
{
	xTaskCreate(net_process,"net_process",configMINIMAL_STACK_SIZE*30,NULL,tskIDLE_PRIORITY + 4,NULL);
}

