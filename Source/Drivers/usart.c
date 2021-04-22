#include "usart.h"	
#include "stm32f4xx_it.h"
#include "main.h"
#include <stdlib.h>
#include <string.h>
 
uint8_t usart2_recv_idle = 0;
uint8_t usart3_recv_idle = 0;

int fputc(int ch, FILE *f)
{ 	
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
	USART1->DR = (u8) ch;      
	return ch;
}
 
#if EN_USART1_RX   //如果使能了接收
//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	
u8 USART_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 USART_RX_STA=0;       //接收状态标记	

//初始化IO 串口1 
//bound:波特率
void uart1_init(u32 bound){
   //GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); //使能GPIOA时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);//使能USART1时钟
 
	//串口1对应引脚复用映射
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1); //GPIOA9复用为USART1
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1); //GPIOA10复用为USART1
	
	//USART1端口配置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10; //GPIOA9与GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOA,&GPIO_InitStructure); //初始化PA9，PA10

   //USART1 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	USART_Init(USART1, &USART_InitStructure); //初始化串口1
	
	USART_Cmd(USART1, ENABLE);  //使能串口1 
	
	//USART_ClearFlag(USART1, USART_FLAG_TC);
	

	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启相关中断

	//Usart1 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;//串口1中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器、

}


void USART1_IRQHandler(void)                	//串口1中断服务程序
{
	u8 Res;
//#if SYSTEM_SUPPORT_OS 		//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
//	OSIntEnter();    
//#endif
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
		Res =USART_ReceiveData(USART1);//(USART1->DR);	//读取接收到的数据
		
		if((USART_RX_STA&0x8000)==0)//接收未完成
		{
			if(USART_RX_STA&0x4000)//接收到了0x0d
			{
				if(Res!=0x0a)USART_RX_STA=0;//接收错误,重新开始
				else USART_RX_STA|=0x8000;	//接收完成了 
			}
			else //还没收到0X0D
			{	
				if(Res==0x0d)USART_RX_STA|=0x4000;
				else
				{
					USART_RX_BUF[USART_RX_STA&0X3FFF]=Res ;
					USART_RX_STA++;
					if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//接收数据错误,重新开始接收	  
				}		 
			}
		}   		 
  } 
//#if SYSTEM_SUPPORT_OS 	//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
//	OSIntExit();  											 
//#endif
} 
#endif	


_uart_queue_t net_rx_queue = {.in = 0, .out = 0};
_uart_queue_t redio_rx_queue = {.in = 0, .out = 0};


_comm_msg_queue_t net_tx_queue = {.in = 0, .out = 0};
_comm_msg_queue_t redio_tx_queue = {.in = 0, .out = 0};



void uart2_init(u32 bound)
	{
   //GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	DMA_InitTypeDef DMA_InitStructureureure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); //使能GPIOA时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);//使能USART2时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1,ENABLE);//使能DMA1时钟
 
	//串口1对应引脚复用映射
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_USART2); //GPIOA2复用为USART2
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource3,GPIO_AF_USART2); //GPIOA3复用为USART2
	
	//USART1端口配置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3; //GPIOA2与GPIOA3
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOA,&GPIO_InitStructure); //初始化PA2，PA3

   //USART1 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	USART_Init(USART2, &USART_InitStructure); //初始化串口2
	
	USART_Cmd(USART2, ENABLE);  //使能串口2 
	

	//usart DMA tx
	DMA_DeInit(DMA1_Stream6);
	while(DMA_GetCmdStatus(DMA1_Stream6) != DISABLE);
	DMA_InitStructureureure.DMA_Channel = DMA_Channel_4;
	DMA_InitStructureureure.DMA_PeripheralBaseAddr = (u32)&USART2->DR;
	DMA_InitStructureureure.DMA_Memory0BaseAddr = (u32)redio_tx_queue.msg_array[redio_tx_queue.out].buf;;
	DMA_InitStructureureure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructureureure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructureureure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStructureureure.DMA_BufferSize = SEND_BUF_SIZE;
	DMA_InitStructureureure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructureureure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructureureure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructureureure.DMA_Priority = DMA_Priority_Medium;
	DMA_InitStructureureure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructureureure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	DMA_InitStructureureure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructureureure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA1_Stream6, &DMA_InitStructureureure);
	
	USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);
	USART_ClearFlag(USART2, USART_FLAG_TC);
	DMA_Cmd(DMA1_Stream6, ENABLE);

	//usart DMA Rx
	DMA_DeInit(DMA1_Stream5);
	while(DMA_GetCmdStatus(DMA1_Stream5) != DISABLE);
	DMA_InitStructureureure.DMA_Channel = DMA_Channel_4;
	DMA_InitStructureureure.DMA_PeripheralBaseAddr = (u32)&USART2->DR;
	DMA_InitStructureureure.DMA_Memory0BaseAddr = (u32)redio_rx_queue.buf;
	DMA_InitStructureureure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructureureure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructureureure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructureureure.DMA_BufferSize = RECV_BUF_SIZE;
	DMA_InitStructureureure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructureureure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructureureure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructureureure.DMA_Priority = DMA_Priority_Medium;
	DMA_InitStructureureure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructureureure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	DMA_InitStructureureure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructureureure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA1_Stream5, &DMA_InitStructureureure);
	USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);
	USART_ClearFlag(USART2, USART_FLAG_RXNE);
	DMA_Cmd(DMA1_Stream5, ENABLE);

#ifdef USART2_NVIC
	NVIC_InitTypeDef NVIC_InitStructure;
	USART_ITConfig( USART2, USART_IT_IDLE, ENABLE);
	 
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;//串口1中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器、
	
#endif
}
	
#ifdef USART2_NVIC
void USART2_IRQHandler()
{
	if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)
	{
		usart2_recv_idle = 1;
		USART2->DR;
	//	USART_ClearITPendingBit( USART1, USART_IT_IDLE );
		printf("usart2_recv_finish\r\n");
	}
}
#endif









void uart3_init(u32 bound)
{
   //GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	DMA_InitTypeDef DMA_InitStructureure;

	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE); //使能GPIOB时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);//使能USART2时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1,ENABLE);//使能DMA1时钟
 
	//串口1对应引脚复用映射
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource10,GPIO_AF_USART3); //GPIOB10复用为USART3
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource11,GPIO_AF_USART3); //GPIOB11复用为USART3
	
	//USART1端口配置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11; //GPIOB10与GPIOB11
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOB,&GPIO_InitStructure); 

   //USART1 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	USART_Init(USART3, &USART_InitStructure); //初始化串口3
	
	USART_Cmd(USART3, ENABLE);  //使能串口3
	
	//usart DMA tx
	DMA_DeInit(DMA1_Stream3);
	while(DMA_GetCmdStatus(DMA1_Stream3) != DISABLE);
	DMA_InitStructureure.DMA_Channel = DMA_Channel_4;
	DMA_InitStructureure.DMA_PeripheralBaseAddr = (u32)&USART3->DR;
	DMA_InitStructureure.DMA_Memory0BaseAddr = (u32)net_tx_queue.msg_array[net_tx_queue.out].buf;
	DMA_InitStructureure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructureure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructureure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStructureure.DMA_BufferSize = SEND_BUF_SIZE;
	DMA_InitStructureure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructureure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructureure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructureure.DMA_Priority = DMA_Priority_Medium;
	DMA_InitStructureure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructureure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	DMA_InitStructureure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructureure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA1_Stream3, &DMA_InitStructureure);

	USART_DMACmd(USART3, USART_DMAReq_Tx, ENABLE);
	USART_ClearFlag(USART3, USART_FLAG_TC);
	DMA_Cmd(DMA1_Stream3, ENABLE);


	//usart DMA Rx
	DMA_DeInit(DMA1_Stream1);
	while(DMA_GetCmdStatus(DMA1_Stream1) != DISABLE);
	DMA_InitStructureure.DMA_Channel = DMA_Channel_4;
	DMA_InitStructureure.DMA_PeripheralBaseAddr = (u32)&USART3->DR;
	DMA_InitStructureure.DMA_Memory0BaseAddr = (u32)net_rx_queue.buf;
	DMA_InitStructureure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructureure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructureure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructureure.DMA_BufferSize = RECV_BUF_SIZE;
	DMA_InitStructureure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructureure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructureure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructureure.DMA_Priority = DMA_Priority_Medium;
	DMA_InitStructureure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructureure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	DMA_InitStructureure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructureure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA1_Stream1, &DMA_InitStructureure);
	USART_DMACmd(USART3, USART_DMAReq_Rx, ENABLE);
	USART_ClearFlag(USART3, USART_FLAG_RXNE);
	DMA_Cmd(DMA1_Stream1, ENABLE);
	
	#ifdef USART3_NCIC
	NVIC_InitTypeDef NVIC_InitStructure;
	USART_ITConfig( USART3, USART_IT_IDLE, ENABLE);
	 
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;//串口1中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器、
	#endif
}


#ifdef USART3_NCIC
void USART3_IRQHandler()
{
	if(USART_GetITStatus(USART3, USART_IT_IDLE) != RESET)
	{
		usart3_recv_idle = 1;
		USART3->DR;
		printf("usart3_recv_finish\r\n");
	}
}
#endif
 

/*数据发送 循环*/
void MSG_USART_Send(DMA_Stream_TypeDef *DMA_Streamx , uint32_t DMA_FLAG_TCIFx, _comm_msg_queue_t *_pMsgQueue)
{
	if (_pMsgQueue->in != _pMsgQueue->out)
	{
		if(DMA_GetFlagStatus(DMA_Streamx, DMA_FLAG_TCIFx) != RESET)
		{
			if (_pMsgQueue->msg_array[_pMsgQueue->out].len > 0)
			{	
				DMA_ClearFlag(DMA_Streamx, DMA_FLAG_TCIFx);
				DMA_Cmd(DMA_Streamx, DISABLE);                      //关闭DMA传输 
				//while (DMA_GetCmdStatus(DMA_Streamx) != DISABLE){}	//确保DMA可以被设置 
				DMA_Streamx->M0AR	= (uint32_t)_pMsgQueue->msg_array[_pMsgQueue->out].buf;
			//	printf("net_tx_queue.msg_array[net_tx_queue.in].buf = %s\r\n",net_tx_queue.msg_array[net_tx_queue.in].buf);
				DMA_Streamx->NDTR	= _pMsgQueue->msg_array[_pMsgQueue->out].len;	
			//	DMA_SetCurrDataCounter(DMA_Streamx,_pMsgQueue->msg_array[_pMsgQueue->out].len);          //数据传输量  
				DMA_Cmd(DMA_Streamx, ENABLE);                      //开启DMA传输 
			}
			/* 消息缓存大小：MSG_ARRAY_SIZE，防止越界 */
			_pMsgQueue->out = (_pMsgQueue->out + 1) & MSG_ARRAY_SIZE;
		}
	}
}

/*进行一次DMA串口发送*/
void dma_send_data_uart(_comm_msg_queue_t *p_msg,uint8_t *buf,uint32_t len)
{
	memcpy(p_msg->msg_array[p_msg->in].buf,buf,len); 
//	printf("net_tx_queue.msg_array[net_tx_queue.in].buf = %s\r\n",p_msg->msg_array[p_msg->in].buf);
	p_msg->msg_array[p_msg->in].len = len;
	p_msg->in = (p_msg->in +1 ) % MSG_ARRAY_SIZE;
}



















//DMAx的各通道配置
//这里的传输形式是固定的,这点要根据不同的情况来修改
//从存储器->外设模式/8位数据宽度/存储器增量模式
//DMA_Streamx:DMA数据流,DMA1_Stream0~7/DMA2_Stream0~7
//chx:DMA通道选择,@ref DMA_channel DMA_Channel_0~DMA_Channel_7
//par:外设地址
//mar:存储器地址
//ndtr:数据传输量  
void MYDMA_Config(DMA_Stream_TypeDef *DMA_Streamx,u32 chx,u32 par,u32 mar,u16 ndtr)
{ 
 
	DMA_InitTypeDef  DMA_InitStructureureure;
	
	if((u32)DMA_Streamx>(u32)DMA2)//得到当前stream是属于DMA2还是DMA1
	{
	  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2,ENABLE);//DMA2时钟使能 
		
	}else 
	{
	  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1,ENABLE);//DMA1时钟使能 
	}
  DMA_DeInit(DMA_Streamx);
	
	while (DMA_GetCmdStatus(DMA_Streamx) != DISABLE){}//等待DMA可配置 
	
  /* 配置 DMA Stream */
  DMA_InitStructureureure.DMA_Channel = chx;  //通道选择
  DMA_InitStructureureure.DMA_PeripheralBaseAddr = par;//DMA外设地址
  DMA_InitStructureureure.DMA_Memory0BaseAddr = mar;//DMA 存储器0地址
  DMA_InitStructureureure.DMA_DIR = DMA_DIR_MemoryToPeripheral;//存储器到外设模式
  DMA_InitStructureureure.DMA_BufferSize = ndtr;//数据传输量 
  DMA_InitStructureureure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//外设非增量模式
  DMA_InitStructureureure.DMA_MemoryInc = DMA_MemoryInc_Enable;//存储器增量模式
  DMA_InitStructureureure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;//外设数据长度:8位
  DMA_InitStructureureure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;//存储器数据长度:8位
  DMA_InitStructureureure.DMA_Mode = DMA_Mode_Normal;// 使用普通模式 
  DMA_InitStructureureure.DMA_Priority = DMA_Priority_Medium;//中等优先级
  DMA_InitStructureureure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
  DMA_InitStructureureure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
  DMA_InitStructureureure.DMA_MemoryBurst = DMA_MemoryBurst_Single;//存储器突发单次传输
  DMA_InitStructureureure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;//外设突发单次传输
  DMA_Init(DMA_Streamx, &DMA_InitStructureureure);//初始化DMA Stream
	

} 
//开启一次DMA传输
//DMA_Streamx:DMA数据流,DMA1_Stream0~7/DMA2_Stream0~7 
//ndtr:数据传输量  
void MYDMA_Enable(DMA_Stream_TypeDef *DMA_Streamx,u16 ndtr)
{
 
	DMA_Cmd(DMA_Streamx, DISABLE);                      //关闭DMA传输 
	
	while (DMA_GetCmdStatus(DMA_Streamx) != DISABLE){}	//确保DMA可以被设置  
		
	DMA_SetCurrDataCounter(DMA_Streamx,ndtr);          //数据传输量  
 
	DMA_Cmd(DMA_Streamx, ENABLE);                      //开启DMA传输 
}	  

 

