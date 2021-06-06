/*
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Dirk Ziegelmeier <dziegel@gmx.de>
 *
 */

#include "lwip/apps/mqtt.h"
#include "mqtt_client.h"

#include "SEGGER_RTT.h"
#include "lwip/ip4_addr.h"
#include "string.h"
#include "cJSON.h"
#include "tick.h"

#ifndef DEVECE_ID
#define DEVECE_ID "DEV00001"
#endif

#ifndef MQ_PORT
#define MQ_PORT 53571 //1883
#endif 

#if LWIP_TCP

/** Define this to a compile-time IP address initialization
 * to connect anything else than IPv4 loopback
 */
#ifndef LWIP_MQTT_EXAMPLE_IPADDR_INIT
#if LWIP_IPV4
#define LWIP_MQTT_EXAMPLE_IPADDR_INIT = IPADDR4_INIT(IPADDR_ANY)
#else
#define LWIP_MQTT_EXAMPLE_IPADDR_INIT
#endif
#endif

//static ip_addr_t mqtt_ip LWIP_MQTT_EXAMPLE_IPADDR_INIT;
 
 
 
static ip_addr_t mqtt_ip ;
mqtt_client_t* mqtt_client;

mqtt_cmd_struct mqtt_cmd;

static const struct mqtt_connect_client_info_t mqtt_client_info =
{
  "STM32MQ_DEVICE",
  NULL, /* user */
  NULL, /* pass */
  10,  /* keep alive */
  NULL, /* will_topic */
  NULL, /* will_msg */
  0,    /* will_qos */
  0     /* will_retain */
#if LWIP_ALTCP && LWIP_ALTCP_TLS
  , NULL
#endif
};

static void 
mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags)
{
  const struct mqtt_connect_client_info_t* client_info = (const struct mqtt_connect_client_info_t*)arg;
	
  SEGGER_RTT_printf(0,"MQTT client \"%s\" mqtt_incoming_data_cb: len %d, flags %d\n", client_info->client_id,
          (int)len, (int)flags);
  SEGGER_RTT_printf(0,"%s",data);
	json_parse(data, len);
}

static void
mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len)
{
  const struct mqtt_connect_client_info_t* client_info = (const struct mqtt_connect_client_info_t*)arg;

  SEGGER_RTT_printf(0,"MQTT client \"%s\" mqtt_incoming_publish_cb: topic %s, len %d\n", client_info->client_id,
          topic, (int)tot_len);
}

static void
mqtt_request_cb(void *arg, err_t err)
{
  const struct mqtt_connect_client_info_t* client_info = (const struct mqtt_connect_client_info_t*)arg;

  SEGGER_RTT_printf(0,"MQTT client \"%s\" request cb: err %d\n", client_info->client_id, (int)err);
}

static void
mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status)
{
  const struct mqtt_connect_client_info_t* client_info = (const struct mqtt_connect_client_info_t*)arg;

  SEGGER_RTT_printf(0,"MQTT client \"%s\" connection cb: status %d\n", client_info->client_id, (int)status);

  if (status == MQTT_CONNECT_ACCEPTED) {
	  
	  mqtt_set_inpub_callback(mqtt_client,
          mqtt_incoming_publish_cb,
          mqtt_incoming_data_cb,
          LWIP_CONST_CAST(void*, &mqtt_client_info));
	  
	char topic[14] = {DEVECE_ID};
	strcat(topic,"_DOWN");
    mqtt_sub_unsub(client,
            topic, 0,
            mqtt_request_cb, LWIP_CONST_CAST(void*, client_info),
            1);		
  }
  else 
  {
//	  static uint32_t last_uwTick;
//	  
//	  if(last_uwTick + 1000 < uwTick)
//	  {
//			last_uwTick = uwTick;
//			/*断开连接后重新连接*/
//			mqtt_disconnect(mqtt_client);
//			//mqtt_client_free(mqtt_client);
//		  
//			mqtt_client_connect(mqtt_client,
//			  &mqtt_ip, MQ_PORT,
//			  mqtt_connection_cb, LWIP_CONST_CAST(void*, &mqtt_client_info),
//			  &mqtt_client_info);
//	  }
  }
}
#endif /* LWIP_TCP */


/* Called when publish is complete either with sucess or failure */
static void mqtt_pub_request_cb(void *arg, err_t result)
{
  if(result != ERR_OK) {
    SEGGER_RTT_printf(0,"Publish result: %d\n", result);
  }
}


/*init mqtt_client publish_data for mqtt*/
void mqtt_publish_data(char *pub_payload)
{
	err_t err;
	u8_t qos = 0; /* 0 1 or 2, see MQTT specification */
	u8_t retain = 0; /* No don't retain such crappy payload... */
	char pub_topic[12] = {DEVECE_ID};
	strcat(pub_topic,"_UP");
	err = mqtt_publish(mqtt_client, pub_topic, pub_payload, strlen(pub_payload), qos, retain, mqtt_pub_request_cb, 0);
	if(err != ERR_OK) {
	SEGGER_RTT_printf(0,"Publish err: %d\n", err);
	}
}


void mqtt_example_init(void)
{
 IP4_ADDR(&mqtt_ip, 103, 46, 128, 45);  		//设置连接的IP地址
//	IP4_ADDR(&mqtt_ip, 192, 168, 1, 100);  		//设置连接的IP地址
#if LWIP_TCP
  mqtt_client = mqtt_client_new();

  mqtt_client_connect(mqtt_client,
          &mqtt_ip, MQ_PORT,
          mqtt_connection_cb, LWIP_CONST_CAST(void*, &mqtt_client_info),
          &mqtt_client_info);
#endif /* LWIP_TCP */
}







/*------------------------------JSON data---------------------------------------*/
void joson_create_uav_data_send()
{
    /* declare a few. */
	cJSON *root = NULL;
	cJSON *head = NULL;
	cJSON *data = NULL;
		
    /* Here we construct some JSON standards, from the JSON site. */
		static uint16_t msg_num = 0;
		msg_num++;
		
		static uint64_t timestamp = 1492488028395;
		timestamp += 1000;
		
		int rand_num = rand()%30;

			/*模拟JSON数据*/
		root = cJSON_CreateObject();
		cJSON_AddItemToObject(root,"head",head = cJSON_CreateObject());
			cJSON_AddNumberToObject(head, "dev_id", 1);
			cJSON_AddNumberToObject(head, "msg_id", MSG_DATA_UP_ID);
			cJSON_AddNumberToObject(head, "msg_no", msg_num);
			cJSON_AddNumberToObject(head, "timestamp", timestamp);
		cJSON_AddItemToObject(root,"data",data = cJSON_CreateObject());
			cJSON_AddNumberToObject(data, "Battery", 55);
			cJSON_AddNumberToObject(data, "Longitude", 1131234567);
			cJSON_AddNumberToObject(data, "Latitude", 221234567);
			cJSON_AddNumberToObject(data, "Altitude", 900);
			cJSON_AddNumberToObject(data, "Env_Temperature", 250+rand_num);
			cJSON_AddNumberToObject(data, "Env_Humidity", 700+rand_num);
			cJSON_AddNumberToObject(data, "Env_Pressure", 1010+rand_num);
			cJSON_AddNumberToObject(data, "Rainfall", rand_num);
			cJSON_AddNumberToObject(data, "Wind_Speed", 10+rand_num);
			cJSON_AddNumberToObject(data, "Wind_Direction", 100+rand_num);
			cJSON_AddNumberToObject(data, "Switch", mqtt_cmd.Switch);
			cJSON_AddNumberToObject(data, "Variable_Val_0", mqtt_cmd.Variable_Val_0);
			cJSON_AddNumberToObject(data, "Variable_Val_1", mqtt_cmd.Variable_Val_1);
			cJSON_AddNumberToObject(data, "Variable_Val_2", mqtt_cmd.Variable_Val_2);
		
		char *pub_payload = NULL;
		pub_payload = cJSON_Print(root);
		mqtt_publish_data(pub_payload);
		
		if(pub_payload!=NULL)
		{
			free(pub_payload);
		}

    cJSON_Delete(root);
}




int json_parse(const uint8_t *buf, uint16_t length)
{
    cJSON *root, *head_item, *data_item;
    
    /* Head*/
    uint32_t msg_id = 0;
    uint32_t msg_no = 0;
    double timestamp = 0;
    
    /* 心跳 */
   
	
	
	SEGGER_RTT_printf(0, "\nbuf_LEN: %d\n", length);
    root = cJSON_ParseWithLength((const char *)buf, length);
    
    if (root)
    {
        head_item = cJSON_GetObjectItem(root, "head");
        
        if (head_item)
        {
            /* 获取消息ID确认消息类型 */
            msg_id = cJSON_GetObjectItem(head_item, "msg_id")->valueint;
            msg_no = cJSON_GetObjectItem(head_item, "msg_no")->valueint;
            timestamp = cJSON_GetObjectItem(head_item, "timestamp")->valuedouble;
            
            SEGGER_RTT_printf(0, "msg_id: %d\n", msg_id);
            SEGGER_RTT_printf(0, "msg_no: %d\n", msg_no);
            
//            sprintf(mqtt_msg_data.net_timestamp_str, "%.0f", timestamp);
//            SEGGER_RTT_printf(0, "timestamp: %s\n", mqtt_msg_data.net_timestamp_str);
//			/*根据时间戳判断 3秒内收到的数据才算有效 超时则不处理*/
//			if (abs(ardupoilt_ubox_data.unix_timestamp - timestamp) > 5000)
//            {
//				cJSON_Delete(root);
//				return 1;
//			}
			
            switch (msg_id)
            {
            case MSG_POWER_ON_ID:
                /* 开机应答 */
                data_item = cJSON_GetObjectItem(root, "data");                
                if (data_item)
                {
				
				}
                break;
            
            case MSG_HRAET_ID:
                /* 心跳应答 */
                data_item = cJSON_GetObjectItem(root, "data");
                
                if (data_item)
                {

                }           
                break;
				
			case MSG_DATA_DOWN_ID:
				/*下发的控制命令*/
				 data_item = cJSON_GetObjectItem(root, "data");
				if (data_item)
                {
                    mqtt_cmd.Switch = cJSON_GetObjectItem(data_item, "Switch")->valueint;
                    mqtt_cmd.Variable_Val_0 = cJSON_GetObjectItem(data_item, "Variable_Val_0")->valueint;
					mqtt_cmd.Variable_Val_1 = cJSON_GetObjectItem(data_item, "Variable_Val_1")->valueint;
					mqtt_cmd.Variable_Val_2 = cJSON_GetObjectItem(data_item, "Variable_Val_2")->valueint;
					SEGGER_RTT_printf(0,"Switch: %d\n Variable_Val_0: %d\n Variable_Val_1: %d\n Variable_Val_2: %d\n", mqtt_cmd.Switch,mqtt_cmd.Variable_Val_0,mqtt_cmd.Variable_Val_1,mqtt_cmd.Variable_Val_2);
				}
				break;
                   
            }
        }
    }
    
    cJSON_Delete(root);
    
    return 0;
}


