#include "usart.h"	
#include "stm32f4xx_it.h"
#include "main.h"
#include <stdlib.h>
#include <string.h>
 
uint8_t usart2_recv_idle = 0;
uint8_t usart3_recv_idle = 0;

int fputc(int ch, FILE *f)
{ 	
	while((USART1->SR&0X40)==0);//ѭ������,ֱ���������   
	USART1->DR = (u8) ch;      
	return ch;
}
 
#if EN_USART1_RX   //���ʹ���˽���
//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
u8 USART_RX_BUF[USART_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
//����״̬
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���Ŀ
u16 USART_RX_STA=0;       //����״̬���	

//��ʼ��IO ����1 
//bound:������
void uart1_init(u32 bound){
   //GPIO�˿�����
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); //ʹ��GPIOAʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);//ʹ��USART1ʱ��
 
	//����1��Ӧ���Ÿ���ӳ��
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1); //GPIOA9����ΪUSART1
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1); //GPIOA10����ΪUSART1
	
	//USART1�˿�����
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10; //GPIOA9��GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//�ٶ�50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //����
	GPIO_Init(GPIOA,&GPIO_InitStructure); //��ʼ��PA9��PA10

   //USART1 ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;//����������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
	USART_Init(USART1, &USART_InitStructure); //��ʼ������1
	
	USART_Cmd(USART1, ENABLE);  //ʹ�ܴ���1 
	
	//USART_ClearFlag(USART1, USART_FLAG_TC);
	

	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//��������ж�

	//Usart1 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;//����1�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ�����

}


void USART1_IRQHandler(void)                	//����1�жϷ������
{
	u8 Res;
//#if SYSTEM_SUPPORT_OS 		//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
//	OSIntEnter();    
//#endif
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
	{
		Res =USART_ReceiveData(USART1);//(USART1->DR);	//��ȡ���յ�������
		
		if((USART_RX_STA&0x8000)==0)//����δ���
		{
			if(USART_RX_STA&0x4000)//���յ���0x0d
			{
				if(Res!=0x0a)USART_RX_STA=0;//���մ���,���¿�ʼ
				else USART_RX_STA|=0x8000;	//��������� 
			}
			else //��û�յ�0X0D
			{	
				if(Res==0x0d)USART_RX_STA|=0x4000;
				else
				{
					USART_RX_BUF[USART_RX_STA&0X3FFF]=Res ;
					USART_RX_STA++;
					if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//�������ݴ���,���¿�ʼ����	  
				}		 
			}
		}   		 
  } 
//#if SYSTEM_SUPPORT_OS 	//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
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
   //GPIO�˿�����
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	DMA_InitTypeDef DMA_InitStructureureure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); //ʹ��GPIOAʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);//ʹ��USART2ʱ��
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1,ENABLE);//ʹ��DMA1ʱ��
 
	//����1��Ӧ���Ÿ���ӳ��
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_USART2); //GPIOA2����ΪUSART2
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource3,GPIO_AF_USART2); //GPIOA3����ΪUSART2
	
	//USART1�˿�����
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3; //GPIOA2��GPIOA3
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//�ٶ�50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //����
	GPIO_Init(GPIOA,&GPIO_InitStructure); //��ʼ��PA2��PA3

   //USART1 ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;//����������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
	USART_Init(USART2, &USART_InitStructure); //��ʼ������2
	
	USART_Cmd(USART2, ENABLE);  //ʹ�ܴ���2 
	

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
	 
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;//����1�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ�����
	
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
   //GPIO�˿�����
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	DMA_InitTypeDef DMA_InitStructureure;

	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE); //ʹ��GPIOBʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);//ʹ��USART2ʱ��
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1,ENABLE);//ʹ��DMA1ʱ��
 
	//����1��Ӧ���Ÿ���ӳ��
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource10,GPIO_AF_USART3); //GPIOB10����ΪUSART3
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource11,GPIO_AF_USART3); //GPIOB11����ΪUSART3
	
	//USART1�˿�����
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11; //GPIOB10��GPIOB11
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//�ٶ�50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //����
	GPIO_Init(GPIOB,&GPIO_InitStructure); 

   //USART1 ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;//����������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
	USART_Init(USART3, &USART_InitStructure); //��ʼ������3
	
	USART_Cmd(USART3, ENABLE);  //ʹ�ܴ���3
	
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
	 
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;//����1�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ�����
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
 

/*���ݷ��� ѭ��*/
void MSG_USART_Send(DMA_Stream_TypeDef *DMA_Streamx , uint32_t DMA_FLAG_TCIFx, _comm_msg_queue_t *_pMsgQueue)
{
	if (_pMsgQueue->in != _pMsgQueue->out)
	{
		if(DMA_GetFlagStatus(DMA_Streamx, DMA_FLAG_TCIFx) != RESET)
		{
			if (_pMsgQueue->msg_array[_pMsgQueue->out].len > 0)
			{	
				DMA_ClearFlag(DMA_Streamx, DMA_FLAG_TCIFx);
				DMA_Cmd(DMA_Streamx, DISABLE);                      //�ر�DMA���� 
				//while (DMA_GetCmdStatus(DMA_Streamx) != DISABLE){}	//ȷ��DMA���Ա����� 
				DMA_Streamx->M0AR	= (uint32_t)_pMsgQueue->msg_array[_pMsgQueue->out].buf;
			//	printf("net_tx_queue.msg_array[net_tx_queue.in].buf = %s\r\n",net_tx_queue.msg_array[net_tx_queue.in].buf);
				DMA_Streamx->NDTR	= _pMsgQueue->msg_array[_pMsgQueue->out].len;	
			//	DMA_SetCurrDataCounter(DMA_Streamx,_pMsgQueue->msg_array[_pMsgQueue->out].len);          //���ݴ�����  
				DMA_Cmd(DMA_Streamx, ENABLE);                      //����DMA���� 
			}
			/* ��Ϣ�����С��MSG_ARRAY_SIZE����ֹԽ�� */
			_pMsgQueue->out = (_pMsgQueue->out + 1) & MSG_ARRAY_SIZE;
		}
	}
}

/*����һ��DMA���ڷ���*/
void dma_send_data_uart(_comm_msg_queue_t *p_msg,uint8_t *buf,uint32_t len)
{
	memcpy(p_msg->msg_array[p_msg->in].buf,buf,len); 
//	printf("net_tx_queue.msg_array[net_tx_queue.in].buf = %s\r\n",p_msg->msg_array[p_msg->in].buf);
	p_msg->msg_array[p_msg->in].len = len;
	p_msg->in = (p_msg->in +1 ) % MSG_ARRAY_SIZE;
}



















//DMAx�ĸ�ͨ������
//����Ĵ�����ʽ�ǹ̶���,���Ҫ���ݲ�ͬ��������޸�
//�Ӵ洢��->����ģʽ/8λ���ݿ��/�洢������ģʽ
//DMA_Streamx:DMA������,DMA1_Stream0~7/DMA2_Stream0~7
//chx:DMAͨ��ѡ��,@ref DMA_channel DMA_Channel_0~DMA_Channel_7
//par:�����ַ
//mar:�洢����ַ
//ndtr:���ݴ�����  
void MYDMA_Config(DMA_Stream_TypeDef *DMA_Streamx,u32 chx,u32 par,u32 mar,u16 ndtr)
{ 
 
	DMA_InitTypeDef  DMA_InitStructureureure;
	
	if((u32)DMA_Streamx>(u32)DMA2)//�õ���ǰstream������DMA2����DMA1
	{
	  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2,ENABLE);//DMA2ʱ��ʹ�� 
		
	}else 
	{
	  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1,ENABLE);//DMA1ʱ��ʹ�� 
	}
  DMA_DeInit(DMA_Streamx);
	
	while (DMA_GetCmdStatus(DMA_Streamx) != DISABLE){}//�ȴ�DMA������ 
	
  /* ���� DMA Stream */
  DMA_InitStructureureure.DMA_Channel = chx;  //ͨ��ѡ��
  DMA_InitStructureureure.DMA_PeripheralBaseAddr = par;//DMA�����ַ
  DMA_InitStructureureure.DMA_Memory0BaseAddr = mar;//DMA �洢��0��ַ
  DMA_InitStructureureure.DMA_DIR = DMA_DIR_MemoryToPeripheral;//�洢��������ģʽ
  DMA_InitStructureureure.DMA_BufferSize = ndtr;//���ݴ����� 
  DMA_InitStructureureure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//���������ģʽ
  DMA_InitStructureureure.DMA_MemoryInc = DMA_MemoryInc_Enable;//�洢������ģʽ
  DMA_InitStructureureure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;//�������ݳ���:8λ
  DMA_InitStructureureure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;//�洢�����ݳ���:8λ
  DMA_InitStructureureure.DMA_Mode = DMA_Mode_Normal;// ʹ����ͨģʽ 
  DMA_InitStructureureure.DMA_Priority = DMA_Priority_Medium;//�е����ȼ�
  DMA_InitStructureureure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
  DMA_InitStructureureure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
  DMA_InitStructureureure.DMA_MemoryBurst = DMA_MemoryBurst_Single;//�洢��ͻ�����δ���
  DMA_InitStructureureure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;//����ͻ�����δ���
  DMA_Init(DMA_Streamx, &DMA_InitStructureureure);//��ʼ��DMA Stream
	

} 
//����һ��DMA����
//DMA_Streamx:DMA������,DMA1_Stream0~7/DMA2_Stream0~7 
//ndtr:���ݴ�����  
void MYDMA_Enable(DMA_Stream_TypeDef *DMA_Streamx,u16 ndtr)
{
 
	DMA_Cmd(DMA_Streamx, DISABLE);                      //�ر�DMA���� 
	
	while (DMA_GetCmdStatus(DMA_Streamx) != DISABLE){}	//ȷ��DMA���Ա�����  
		
	DMA_SetCurrDataCounter(DMA_Streamx,ndtr);          //���ݴ�����  
 
	DMA_Cmd(DMA_Streamx, ENABLE);                      //����DMA���� 
}	  

 

