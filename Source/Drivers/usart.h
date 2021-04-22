#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "stm32f4xx_conf.h"


//#define USART3_NCIC



#define USART_REC_LEN  			64  	//�����������ֽ��� 200
#define EN_USART1_RX 			1		//ʹ�ܣ�1��/��ֹ��0������1����
	  	
extern u8  USART_RX_BUF[USART_REC_LEN]; //���ջ���,���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з� 
extern u16 USART_RX_STA;         		//����״̬���	
//����봮���жϽ��գ��벻Ҫע�����º궨��



#define RECV_BUF_SIZE 2048
typedef struct
{
	uint16_t in;
	uint16_t out;
	uint8_t buf[RECV_BUF_SIZE];
}_uart_queue_t;
extern _uart_queue_t net_rx_queue ;
extern _uart_queue_t redio_rx_queue ;




#define SEND_BUF_SIZE 2048
/* ��ͨ������Ϣ֡ */
typedef struct
{
	uint32_t index;
	uint32_t len;
	uint8_t buf[SEND_BUF_SIZE];
} __attribute__((packed)) _uart_msg_buf_t;

/* ��ͨ������Ϣ�������Ͷ��� */
#define MSG_ARRAY_SIZE 3
typedef struct
{
	uint32_t in;
	uint32_t out;
	_uart_msg_buf_t msg_array[MSG_ARRAY_SIZE];
} _comm_msg_queue_t;


extern _comm_msg_queue_t net_tx_queue ;
extern _comm_msg_queue_t redio_tx_queue ;

void uart1_init(u32 bound);
void uart2_init(u32 bound);
void uart3_init(u32 bound);

void MYDMA_Config(DMA_Stream_TypeDef *DMA_Streamx,u32 chx,u32 par,u32 mar,u16 ndtr);
void MYDMA_Enable(DMA_Stream_TypeDef *DMA_Streamx,u16 ndtr);

extern uint8_t usart2_recv_idle;
extern uint8_t usart3_recv_idle ;

void MSG_USART_Send(DMA_Stream_TypeDef *DMA_Streamx , uint32_t DMA_FLAG_TCIFx, _comm_msg_queue_t *_pMsgQueue);
void dma_send_data_uart(_comm_msg_queue_t *p_msg,uint8_t *buf,uint32_t len);
#endif


