#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "stm32f4xx_conf.h"


//#define USART3_NCIC



#define USART_REC_LEN  			64  	//定义最大接收字节数 200
#define EN_USART1_RX 			1		//使能（1）/禁止（0）串口1接收
	  	
extern u8  USART_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern u16 USART_RX_STA;         		//接收状态标记	
//如果想串口中断接收，请不要注释以下宏定义



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
/* 普通串口消息帧 */
typedef struct
{
	uint32_t index;
	uint32_t len;
	uint8_t buf[SEND_BUF_SIZE];
} __attribute__((packed)) _uart_msg_buf_t;

/* 普通串口消息队列类型定义 */
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


