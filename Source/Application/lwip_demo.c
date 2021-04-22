//#include "lwip_demo.h"
#include "tcp_client.h"
#include "stdio.h"
#include "stdlib.h"

 uint8_t lwip_flag;
 uint8_t lwip_test_mode;
 uint8_t lwip_demo_buf[2048];
 
 void lwip_demo_client(void)
 {
	 static uint32_t t_client_cnt = 0;
	 
	if((lwip_flag&LWIP_CONNECTED)==LWIP_CONNECTED)
	{
	//t_client_cnt++;
	//	sprintf((char*)lwip_demo_buf,"tcp_client send %d\r\n",t_client_cnt);    
		printf("%s ",lwip_demo_buf);
		lwip_flag |= LWIP_SEND_DATA;
	}
	else
	{
		tcp_client_connect_remotehost();//û�������ϣ���ʱ����TCP�ͻ���ģʽ��������������
	}
	
	if((lwip_flag&LWIP_NEW_DATA)==LWIP_NEW_DATA)
	{
		lwip_flag &=~LWIP_NEW_DATA;        //����������ݵı�־
		printf("tcprecv_data ��%s \r\n",lwip_demo_buf);
	}
 }
 
