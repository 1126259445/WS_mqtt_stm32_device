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

#ifndef MQTT_EXAMPLE_H
#define MQTT_EXAMPLE_H

#include "lwip/apps/mqtt.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MSG_POWER_ON_ID 5001
#define MSG_HRAET_ID 5002
#define MSG_DATA_UP_ID 5003
#define MSG_DATA_DOWN_ID 5004

#define MSG_POWER_ON_ACK_ID 6001
#define MSG_HRAET_ACK_ID 6002
#define MSG_DATA_UP_ACK_ID 6003
#define MSG_DATA_DOWN_ACK_ID 6004


typedef struct 
{
    uint32_t  Switch ;
	int32_t Variable_Val_0 ;
	int32_t Variable_Val_1 ;
	int32_t Variable_Val_2 ;
}mqtt_cmd_struct;
extern mqtt_cmd_struct mqtt_cmd;

void mqtt_example_init(void);
void joson_create_uav_data_send(void);
int json_parse(const uint8_t *buf, uint16_t length);

extern mqtt_client_t* mqtt_client;
#ifdef __cplusplus
}
#endif

#endif /* MQTT_EXAMPLE_H */
