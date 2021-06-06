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


static ip_addr_t ipaddr, netmask, gw; //定义IP地址
struct netif enc28j60_netif;  //定义网络接口


/*本机网卡配置添加*/
void LwIP_Config (void)
{   
    IP4_ADDR(&ipaddr, 192, 168, 1, 168);  		//设置本地ip地址
    IP4_ADDR(&gw, 192, 168, 1, 1);			    //网关
    IP4_ADDR(&netmask, 255, 255, 255, 0);		//子网掩码	 
	
	// 初始化LWIP协议栈,执行检查用户所有可配置的值，初始化所有的模块
	lwip_init();
	// 添加网络接口
	while ((netif_add(&enc28j60_netif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &ethernet_input)==NULL))
	{
         vTaskDelay(2);
	}
	netif_set_default(&enc28j60_netif); // 注册默认的网络接口
 	netif_set_up(&enc28j60_netif);      // 建立网络接口用于处理通信
}


/*网络数据处理*/
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
		
		/*网卡输入*/
		ethernetif_input(&enc28j60_netif); 
		/* 无操作系统超时检测 */
		sys_check_timeouts();
		
		/*MQTT连接成功后的操作*/
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
			/*MQTT连接断开后 间隔1S尝试重连 */
			if(last_mqtt_time + 1000 < uwTick)
			{
				last_mqtt_time = uwTick;
		
				mqtt_disconnect(mqtt_client);
				mqtt_client_free(mqtt_client);
				mqtt_example_init();
			}	
		}
		
		/*网络连接状态监测*/
		if (netif_is_link_up(&enc28j60_netif))
		{
			//printf("netif is link up\r\n");
			/* When the netif is fully configured this function must be called 当netif完全配置时，必须调用此函数*/
			//netif_set_up(&enc28j60_netif);

		}
		else
		{
			/*网络断开连接后间隔500ms重新初始化*/
			if( last_netif_time + 500 < uwTick)
			{
				last_netif_time = uwTick;
				
				printf("netif set down\r\n");
				netif_set_up(&enc28j60_netif);
				/* When the netif link is down this function must be called 当netif链接断开时，必须调用这个函数*/
				netif_set_down(&enc28j60_netif);
				/*移除网卡重新配置*/
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

