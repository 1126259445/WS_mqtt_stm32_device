/**
  ******************************************************************************
  * @file    usbd_cdc_vcp.c
  * @author  MCD Application Team
  * @version V1.2.1
  * @date    17-March-2018
  * @brief   Generic media access Layer.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2015 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                      <http://www.st.com/SLA0044>
  *
  ******************************************************************************
  */

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
#pragma     data_alignment = 4
#endif                          /* USB_OTG_HS_INTERNAL_DMA_ENABLED */

/* Includes ------------------------------------------------------------------ */
#include "usbd_cdc_vcp.h"
#include "stdio.h"

/* Private typedef ----------------------------------------------------------- */
/* Private define ------------------------------------------------------------ */
/* Private macro ------------------------------------------------------------- */
/* Private variables --------------------------------------------------------- */
LINE_CODING linecoding = {
  115200,                       /* baud rate */
  0x00,                         /* stop bits-1 */
  0x00,                         /* parity - none */
  0x08                          /* nb. of bits 8 */
};


/* These are external variables imported from CDC core to be used for IN
 * transfer management. */
extern uint8_t APP_Rx_Buffer[]; /* Write CDC received data in this buffer.
                                 * These data will be sent over USB IN endpoint
                                 * in the CDC core functions. */
extern uint32_t APP_Rx_ptr_in;  /* Increment this pointer or roll it back to
                                 * start address when writing received data in
                                 * the buffer APP_Rx_Buffer. */

/* Private function prototypes ----------------------------------------------- */
static uint16_t VCP_Init(void);
static uint16_t VCP_DeInit(void);
static uint16_t VCP_Ctrl(uint32_t Cmd, uint8_t * Buf, uint32_t Len);
static uint16_t VCP_DataTx(void);
static uint16_t VCP_DataRx(uint8_t * Buf, uint32_t Len);


CDC_IF_Prop_TypeDef VCP_fops = {
  VCP_Init,
  VCP_DeInit,
  VCP_Ctrl,
  VCP_DataTx,
  VCP_DataRx
};

/* Private functions --------------------------------------------------------- */
/**
  * @brief  VCP_Init
  *         Initializes the Media on the STM32
  * @param  None
  * @retval Result of the operation (USBD_OK in all cases)
  */
static uint16_t VCP_Init(void)
{

  return USBD_OK;
}

/**
  * @brief  VCP_DeInit
  *         DeInitializes the Media on the STM32
  * @param  None
  * @retval Result of the operation (USBD_OK in all cases)
  */
static uint16_t VCP_DeInit(void)
{

  return USBD_OK;
}


/**
  * @brief  VCP_Ctrl
  *         Manage the CDC class requests
  * @param  Cmd: Command code            
  * @param  Buf: Buffer containing command data (request parameters)
  * @param  Len: Number of data to be sent (in bytes)
  * @retval Result of the operation (USBD_OK in all cases)
  */
static uint16_t VCP_Ctrl(uint32_t Cmd, uint8_t * Buf, uint32_t Len)
{
  switch (Cmd)
  {
  case SEND_ENCAPSULATED_COMMAND:
    /* Not needed for this driver */
    break;

  case GET_ENCAPSULATED_RESPONSE:
    /* Not needed for this driver */
    break;

  case SET_COMM_FEATURE:
    /* Not needed for this driver */
    break;

  case GET_COMM_FEATURE:
    /* Not needed for this driver */
    break;

  case CLEAR_COMM_FEATURE:
    /* Not needed for this driver */
    break;

  case SET_LINE_CODING:
    linecoding.bitrate =
      (uint32_t) (Buf[0] | (Buf[1] << 8) | (Buf[2] << 16) | (Buf[3] << 24));
    linecoding.format = Buf[4];
    linecoding.paritytype = Buf[5];
    linecoding.datatype = Buf[6];
    /* Set the new configuration */
    break;

  case GET_LINE_CODING:
    Buf[0] = (uint8_t) (linecoding.bitrate);
    Buf[1] = (uint8_t) (linecoding.bitrate >> 8);
    Buf[2] = (uint8_t) (linecoding.bitrate >> 16);
    Buf[3] = (uint8_t) (linecoding.bitrate >> 24);
    Buf[4] = linecoding.format;
    Buf[5] = linecoding.paritytype;
    Buf[6] = linecoding.datatype;
    break;

  case SET_CONTROL_LINE_STATE:
    /* Not needed for this driver */
    break;

  case SEND_BREAK:
    /* Not needed for this driver */
    break;

  default:
    break;
  }

  return USBD_OK;
}

/**
  * @brief  VCP_DataTx
  *         CDC received data to be send over USB IN endpoint are managed in 
  *         this function.
  * @param  Buf: Buffer of data to be sent
  * @param  Len: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else VCP_FAIL
  */
static uint16_t VCP_DataTx(void)
{
  if (linecoding.datatype == 7)
  {
  //  APP_Rx_Buffer[APP_Rx_ptr_in] = USART_ReceiveData(EVAL_COM1) & 0x7F;
  }
  else if (linecoding.datatype == 8)
  {
  //  APP_Rx_Buffer[APP_Rx_ptr_in] = USART_ReceiveData(EVAL_COM1);
  }

//  APP_Rx_ptr_in++;

  /* To avoid buffer overflow */
  if (APP_Rx_ptr_in == APP_RX_DATA_SIZE)
  {
    APP_Rx_ptr_in = 0;
  }

  return USBD_OK;
}

/**
  * @brief  VCP_DataRx
  *         Data received over USB OUT endpoint are sent over CDC interface 
  *         through this function.
  *           
  *         @note
  *         This function will block any OUT packet reception on USB endpoint 
  *         until exiting this function. If you exit this function before transfer
  *         is complete on CDC interface (ie. using DMA controller) it will result 
  *         in receiving more data while previous ones are still not sent.
  *                 
  * @param  Buf: Buffer of data received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else VCP_FAIL
  */
static uint16_t VCP_DataRx(uint8_t * Buf, uint32_t Len)
{
  uint32_t i;

  for (i = 0; i < Len; i++)
  {
	  printf("%s",Buf);
//    USART_SendData(EVAL_COM1, *(Buf + i));
//    while (USART_GetFlagStatus(EVAL_COM1, USART_FLAG_TXE) == RESET);
  }

  return USBD_OK;
}

/**
  * @brief  VCP_COMConfig
  *         Configure the COM Port with default values or values received from host.
  * @param  Conf: can be DEFAULT_CONFIG to set the default configuration or OTHER_CONFIG
  *         to set a configuration received from the host.
  * @retval None.
  */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
