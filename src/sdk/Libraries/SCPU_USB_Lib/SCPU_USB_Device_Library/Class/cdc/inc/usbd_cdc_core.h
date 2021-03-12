/**
 ******************************************************************************
 * @file    usbd_cdc_core.h
 * @author  MCD Application Team
 * @version V1.1.0
 * @date    19-March-2012
 * @brief   header file for the usbd_cdc_core.c file.
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/

#ifndef __USB_CDC_CORE_H_
#define __USB_CDC_CORE_H_

#include "usbd_ioreq.h"

/** @defgroup usbd_cdc
 * @brief This file is the Header file for USBD_cdc.c
 * @{
 */

/** @defgroup usbd_cdc_Exported_Defines
 * @{
 */
#define USB_CDC_CONFIG_DESC_SIZ (68)
#define USB_CDC_DESC_SIZ        (USB_CDC_CONFIG_DESC_SIZ - 9)

#define CDC_DESCRIPTOR_TYPE 0x21

#define DEVICE_CLASS_CDC    0x02
#define DEVICE_SUBCLASS_CDC 0x00

#define USB_DEVICE_DESCRIPTOR_TYPE        0x01
#define USB_CONFIGURATION_DESCRIPTOR_TYPE 0x02
#define USB_STRING_DESCRIPTOR_TYPE        0x03
#define USB_INTERFACE_DESCRIPTOR_TYPE     0x04
#define USB_ENDPOINT_DESCRIPTOR_TYPE      0x05

#define STANDARD_ENDPOINT_DESC_SIZE 0x09

#define CDC_DATA_IN_PACKET_SIZE CDC_DATA_MAX_PACKET_SIZE

#define CDC_DATA_OUT_PACKET_SIZE CDC_DATA_MAX_PACKET_SIZE

#define CDC_APP_RX_DATA_SIZE APP_RX_DATA_SIZE

#define CDC_APP_TX_DATA_SIZE APP_TX_DATA_SIZE

/*---------------------------------------------------------------------*/
/*  CDC definitions                                                    */
/*---------------------------------------------------------------------*/

/**************************************************/
/* CDC Requests                                   */
/**************************************************/
#define SEND_ENCAPSULATED_COMMAND 0x00
#define GET_ENCAPSULATED_RESPONSE 0x01
#define SET_COMM_FEATURE          0x02
#define GET_COMM_FEATURE          0x03
#define CLEAR_COMM_FEATURE        0x04
#define SET_LINE_CODING           0x20
#define GET_LINE_CODING           0x21
#define SET_CONTROL_LINE_STATE    0x22
#define SEND_BREAK                0x23
#define NO_CMD                    0xFF

/**
 * @}
 */

/** @defgroup USBD_CORE_Exported_TypesDefinitions
 * @{
 */
typedef struct _CDC_IF_PROP
{
    uint16_t (*pIf_Init)(void);
    uint16_t (*pIf_DeInit)(void);
    uint16_t (*pIf_Ctrl)(uint32_t Cmd, uint8_t* Buf, uint32_t Len);
    uint16_t (*pIf_DataTx)(uint8_t* Buf, uint32_t Len);
    uint16_t (*pIf_DataRx)(uint8_t* Buf, uint32_t Len);
} CDC_IF_Prop_TypeDef;
/**
 * @}
 */
struct APP_DATA_STRUCT_DEF
{
    /* Host Send buffer */
    struct
    {
        uint8_t APP_Tx_Buffer[CDC_APP_TX_DATA_SIZE]; // Buffer

        volatile uint32_t APP_Tx_ptr_in;  // Data input offset
        volatile uint32_t APP_Tx_ptr_out; // Data output offset
        volatile uint32_t Tx_counter;     // Data ready size
    } tx_structure;
    /* Host Receive buffer */
    struct
    {
        uint8_t APP_Rx_Buffer[CDC_APP_RX_DATA_SIZE]; // Buffer

        volatile uint32_t APP_Rx_ptr_in;  // Data input offset
        volatile uint32_t APP_Rx_ptr_out; // Data output offset
        volatile uint32_t Rx_counter;     // Data ready size
    } rx_structure;
    uint8_t COM_config_cmp; // Serial Staus(1:Connected; 0:DisConnected)
};

/** @defgroup USBD_CORE_Exported_Macros
 * @{
 */

/**
 * @}
 */

/** @defgroup USBD_CORE_Exported_Variables
 * @{
 */

extern USBD_Class_cb_TypeDef USBD_CDC_cb;
/**
 * @}
 */

/** @defgroup USB_CORE_Exported_Functions
 * @{
 */
/**
 * @}
 */

#endif // __USB_CDC_CORE_H_
/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT Megahuntmicro *****END OF FILE****/
