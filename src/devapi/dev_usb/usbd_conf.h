/************************ (C) COPYRIGHT Megahuntmicro *************************
 * File Name            : usbd_conf.h
 * Author               : Megahuntmicro
 * Version              : V1.0.0
 * Date                 : 21-October-2014
 * Description          : USB Device configuration file
 *****************************************************************************/

#ifndef __USBD_CONF__H__
#define __USBD_CONF__H__

/* Includes ------------------------------------------------------------------*/
#include "usb_conf.h"

#define USBD_CFG_MAX_NUM 1
#define USBD_ITF_MAX_NUM 4

#define USB_MAX_STR_DESC_SIZ 64

#define USBD_SELF_POWERED

#define USBD_DYNAMIC_DESCRIPTOR_CHANGE_ENABLED

/** @defgroup USB_String_Descriptors
 * @{
 */

/** @defgroup USB_HID_Class_Layer_Parameter
 * @{
 */

#define HID_IN_PACKET  8

#define CDC_IN_EP  0x82 /* EP2 for data IN */
#define CDC_OUT_EP 0x02 /* EP2 for data OUT */

/* CDC Endpoints parameters: you can fine tune these values depending on the needed baudrates and performance. */
#define CDC_DATA_MAX_PACKET_SIZE 64 /* Endpoint IN & OUT Packet size */
#define CDC_CMD_PACKET_SZE       8  /* Control Endpoint Packet size */

#define CDC_IN_FRAME_INTERVAL 5 /* Number of frames between IN transfers */
#define APP_TX_DATA_SIZE      4128
#define APP_RX_DATA_SIZE      4128 /* Total size of IN buffer: APP_RX_DATA_SIZE*8/MAX_BAUDARATE*1000 should be > CDC_IN_FRAME_INTERVAL */

#define APP_FOPS VCP_fops
/**
 * @}
 */

// Printer define

#define PRINTER_IN_EP  0x81
#define PRINTER_OUT_EP 0x01

#define PRINTER_MAX_PACKET_SIZE 64

#define PRINTER_IN_FRAME_INTERVAL 5 /* Number of frames between IN transfers */

#endif //__USBD_CONF__H__

/************************ (C) COPYRIGHT 2014 Megahuntmicro ****END OF FILE****/
