/************************ (C) COPYRIGHT Megahuntmicro *************************
 * File Name            : usbd_cdc_vcp.h
 * Author               : Megahuntmicro
 * Version              : V1.0.0
 * Date                 : 21-October-2014
 * Description          : Header for usbd_cdc_vcp.c file.
 *****************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USBD_CDC_VCP_H
#define __USBD_CDC_VCP_H

/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc_core.h"
#include "usbd_conf.h"

/* Exported typef ------------------------------------------------------------*/
/* The following structures groups all needed parameters to be configured for the
    ComPort. These parameters can modified on the fly by the host through CDC class
    command class requests. */
typedef struct
{
    uint32_t bitrate;
    uint8_t  format;
    uint8_t  paritytype;
    uint8_t  datatype;
} LINE_CODING;

/* Exported constants --------------------------------------------------------*/
/* The following define is used to route the USART IRQ handler to be used.
   The IRQ handler function is implemented in the usbd_cdc_vcp.c file. */

#define DEFAULT_CONFIG 0
#define OTHER_CONFIG   1

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
uint16_t VCP_DataTx(uint8_t* Buf, uint32_t Len);
uint32_t VCP_GetTxBuflen(void);
uint32_t VCP_GetTxBufrsaddr(void);
int32_t  VCP_GetRxChar(void);
uint32_t VCP_GetRxBuflen(void);
uint8_t* VCP_GetRxBufrsaddr(void);

extern struct APP_DATA_STRUCT_DEF APP_Gdata_param;

#endif /* __USBD_CDC_VCP_H */

/************************ (C) COPYRIGHT 2014 Megahuntmicro ****END OF FILE****/
