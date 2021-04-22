/*
  Copyright (c) 2009-2017 Dave Gamble and cJSON contributors

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#include "usart.h"
#include "main.h"
#include <string.h>

//#include "lwip_demo.h"

void exit(int num/*status*/)
{
	if(num != 0)
	{
		printf("error exit num = %d\r\n",num);
	}
	return ;
}


/* Used by some code below as an example datatype. */
struct record
{
    const char *precision;
    double lat;
    double lon;
    const char *address;
    const char *city;
    const char *state;
    const char *zip;
    const char *country;
};




/* Create a bunch of objects as demonstration. */
static int print_preallocated(cJSON *root)
{
    /* declarations */
    char *out = NULL;
    char *buf = NULL;
    char *buf_fail = NULL;
    size_t len = 0;
    size_t len_fail = 0;

    /* formatted print */
    buf = cJSON_Print(root);
    /* create buffer to succeed */
    /* the extra 5 bytes are because of inaccuracies when reserving memory */
    len = strlen(buf);
//	printf("len = %d\r\n",len);
//	printf("%s",buf);
	
	//拷贝数据到TCP发送buf
//	memcpy(lwip_demo_buf,buf,len);  
	//拷贝json数据到串口3DMA发送
	dma_send_data_uart(&net_tx_queue,(uint8_t*)buf,len);
	
    free(buf);
    return 0;
}

/* Create a bunch of objects as demonstration. */
void create_objects(char *buf)
{
    /* declare a few. */
	cJSON *uav = NULL;
	cJSON *body = NULL;
		
    /* Here we construct some JSON standards, from the JSON site. */

    /* Our "Video" datatype: */
		
		static uint16_t num = 0;
		static float numf = 0;
		num++;
		char numstr[10] = {0};
		numf+=0.5;
		char numstrf[10] = {0};

		sprintf(numstrf,"%f",numf);
		sprintf(numstr,"%d",num);
		

		uav = cJSON_CreateObject();
		cJSON_AddItemToObject(uav,"body",body = cJSON_CreateObject());
			cJSON_AddStringToObject(body,"aircraftLocationLatitude","28.801666259765625");
			cJSON_AddStringToObject(body,"aircraftLocationLongitude","104.70318603515625");
			cJSON_AddStringToObject(body,"altitude",numstr);
			cJSON_AddStringToObject(body,"attitudef",numstrf);
			cJSON_AddStringToObject(body,"batteryWarning","0");
			cJSON_AddStringToObject(body,"createTime","1575618604208");
			cJSON_AddStringToObject(body,"current","3125.0");
			cJSON_AddStringToObject(body,"flightTimesec","2626");
			cJSON_AddStringToObject(body,"flyHeading","0.0");
			cJSON_AddStringToObject(body,"gpsLevel","0");
			cJSON_AddStringToObject(body,"gpsNum","32");
			cJSON_AddStringToObject(body,"homeLocationLatitude","28.801666259765625");
			cJSON_AddStringToObject(body,"homeLocationLongitude","104.70318603515625");
			cJSON_AddStringToObject(body,"isFlying","1");
			cJSON_AddStringToObject(body,"lifeTime","49.0");
			cJSON_AddStringToObject(body,"maxHeight","false");
			cJSON_AddStringToObject(body,"maxRadius","false");
			cJSON_AddStringToObject(body,"pitch","3.0");
			cJSON_AddStringToObject(body,"policeName","test");
			cJSON_AddStringToObject(body,"policeNumber","123456");
			cJSON_AddStringToObject(body,"registrationID","123456");
			cJSON_AddStringToObject(body,"remoteControlBattery","0");
			cJSON_AddStringToObject(body,"remoteControlSignal","90");
			cJSON_AddStringToObject(body,"roll","0");
			cJSON_AddStringToObject(body,"startTime","2134");
			cJSON_AddStringToObject(body,"temperature","26.6");
			cJSON_AddStringToObject(body,"uavModel","3VS-SECURITY");
			cJSON_AddStringToObject(body,"uavName","x70");
			cJSON_AddStringToObject(body,"uavSn","4MFCB11903035G");
			cJSON_AddStringToObject(body,"uavSw","19-11-11");
			cJSON_AddStringToObject(body,"velocity","70.0");
			cJSON_AddStringToObject(body,"verticalSpeed","1.0");
			cJSON_AddStringToObject(body,"videoSignal","100.0");
			cJSON_AddStringToObject(body,"videoTransportRate","0.0");
			cJSON_AddStringToObject(body,"voltage","22632.0");
		
		cJSON_AddStringToObject(uav,"cmdName","uavFlightdata");
		cJSON_AddStringToObject(uav,"cmdType","notify");
		cJSON_AddStringToObject(uav,"deviceID","request");
		cJSON_AddStringToObject(uav,"msgType","uavFlightdata");
		cJSON_AddNumberToObject(uav, "serialNo", 201912061725561660);
		
		
		char *temp;
		 temp = cJSON_Print(uav);
		 //memcpy(buf,temp,strlen(temp));
//		if (print_preallocated(uav) != 0)
//		{
//			cJSON_Delete(uav);
//			exit(EXIT_FAILURE);
//		}
    cJSON_Delete(uav);
}


