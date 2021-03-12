/************************ (C) COPYRIGHT Megahuntmicro *************************
 * File Name            : usbd_desc.c
 * Author               : Megahuntmicro
 * Version              : V1.0.0
 * Date                 : 21-October-2014
 * Description          : This file provides the USBD descriptors and string formating method.
 *****************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "usbd_desc.h"
#include "usb_regs.h"
#include "usbd_conf.h"
#include "usbd_core.h"
#include "usbd_req.h"

/** @defgroup USBD_DESC_Private_Defines
 * @{
 */
#define USBD_VID 0x0D28
#define USBD_PID 0x0204

#define USBD_LANGID_STRING       0x0409
#define USBD_MANUFACTURER_STRING "Megahunt"

#define USBD_PRODUCT_STRING      "Megahunt Composite Device"
#define USBD_SERIALNUMBER_STRING "00000000011C"

#define USBD_CONFIGURATION_STRING "USB Config"
#define USBD_INTERFACE_STRING     "USB Interface"

/**
 * @}
 */

/** @defgroup USBD_DESC_Private_Variables
 * @{
 */
USBD_DEVICE USR_desc = {
    USBD_USR_DeviceDescriptor,          /* DeviceDescriptor */
    USBD_USR_LangIDStrDescriptor,       /* LangID:0409 */
    USBD_USR_ManufacturerStrDescriptor, /* Manufacturer:Megahunt */
    USBD_USR_ProductStrDescriptor,      /* Product:Megahunt Composite Device */
    USBD_USR_SerialStrDescriptor,       /* SerialNumber */
    USBD_USR_ConfigStrDescriptor,       /* Config */
    USBD_USR_InterfaceStrDescriptor,    /* Interface */
};

/* USB Standard Device Descriptor */
__ALIGN_BEGIN uint8_t USBD_DeviceDesc[USB_SIZ_DEVICE_DESC] __ALIGN_END = {
    0x12,                       /*bLength */
    USB_DEVICE_DESCRIPTOR_TYPE, /*bDescriptorType*/
    0x00,                       /*bcdUSB */
    0x02,
    0x00,                 /*bDeviceClass*/
    0x00,                 /*bDeviceSubClass*/
    0x00,                 /*bDeviceProtocol*/
    USB_OTG_MAX_EP0_SIZE, /*bMaxPacketSize*/
    LOBYTE(USBD_VID),     /*idVendor*/
    HIBYTE(USBD_VID),     /*idVendor*/
    LOBYTE(USBD_PID),     /*idVendor*/
    HIBYTE(USBD_PID),     /*idVendor*/
    0x00,                 /*bcdDevice rel. 2.00*/
    0x02,
    USBD_IDX_MFC_STR,     /*Index of manufacturer  string*/
    USBD_IDX_PRODUCT_STR, /*Index of product string*/
    USBD_IDX_SERIAL_STR,  /*Index of serial number string*/
    USBD_CFG_MAX_NUM,     /*bNumConfigurations*/
};                        /* USB_DeviceDescriptor */

/* USB Standard Device Descriptor */
__ALIGN_BEGIN uint8_t USBD_LangIDDesc[USB_SIZ_STRING_LANGID] __ALIGN_END = {
    USB_SIZ_STRING_LANGID,
    USB_DESC_TYPE_STRING,
    LOBYTE(USBD_LANGID_STRING),
    HIBYTE(USBD_LANGID_STRING),
};
/**
 * @}
 */

/** @defgroup USBD_DESC_Private_Functions
 * @{
 */
/**
 * @brief  USBD_USR_DeviceDescriptor
 *         return the device descriptor
 * @param  speed : current device speed
 * @param  length : pointer to data length variable
 * @retval pointer to descriptor buffer
 */
uint8_t* USBD_USR_DeviceDescriptor(uint8_t speed, uint16_t* length)
{
    *length = sizeof(USBD_DeviceDesc);
    return USBD_DeviceDesc;
}

/**
 * @brief  USBD_USR_LangIDStrDescriptor
 *         return the LangID string descriptor
 * @param  speed : current device speed
 * @param  length : pointer to data length variable
 * @retval pointer to descriptor buffer
 */
uint8_t* USBD_USR_LangIDStrDescriptor(uint8_t speed, uint16_t* length)
{
    *length = sizeof(USBD_LangIDDesc);
    return USBD_LangIDDesc;
}

/**
 * @brief  USBD_USR_ProductStrDescriptor
 *         return the product string descriptor
 * @param  speed : current device speed
 * @param  length : pointer to data length variable
 * @retval pointer to descriptor buffer
 */
uint8_t* USBD_USR_ProductStrDescriptor(uint8_t speed, uint16_t* length)
{
    USBD_GetString(USBD_PRODUCT_STRING, USBD_StrDesc, length);
    return USBD_StrDesc;
}

/**
 * @brief  USBD_USR_ManufacturerStrDescriptor
 *         return the manufacturer string descriptor
 * @param  speed : current device speed
 * @param  length : pointer to data length variable
 * @retval pointer to descriptor buffer
 */
uint8_t* USBD_USR_ManufacturerStrDescriptor(uint8_t speed, uint16_t* length)
{
    USBD_GetString(USBD_MANUFACTURER_STRING, USBD_StrDesc, length);
    return USBD_StrDesc;
}

/**
 * @brief  USBD_USR_SerialStrDescriptor
 *         return the serial number string descriptor
 * @param  speed : current device speed
 * @param  length : pointer to data length variable
 * @retval pointer to descriptor buffer
 */
uint8_t* USBD_USR_SerialStrDescriptor(uint8_t speed, uint16_t* length)
{
    USBD_GetString(USBD_SERIALNUMBER_STRING, USBD_StrDesc, length);
    return USBD_StrDesc;
}

/**
 * @brief  USBD_USR_ConfigStrDescriptor
 *         return the configuration string descriptor
 * @param  speed : current device speed
 * @param  length : pointer to data length variable
 * @retval pointer to descriptor buffer
 */
uint8_t* USBD_USR_ConfigStrDescriptor(uint8_t speed, uint16_t* length)
{
    USBD_GetString(USBD_CONFIGURATION_STRING, USBD_StrDesc, length);
    return USBD_StrDesc;
}

/**
 * @brief  USBD_USR_InterfaceStrDescriptor
 *         return the interface string descriptor
 * @param  speed : current device speed
 * @param  length : pointer to data length variable
 * @retval pointer to descriptor buffer
 */
uint8_t* USBD_USR_InterfaceStrDescriptor(uint8_t speed, uint16_t* length)
{
    USBD_GetString(USBD_INTERFACE_STRING, USBD_StrDesc, length);
    return USBD_StrDesc;
}
/**
 * @}
 */

/************************ (C) COPYRIGHT 2014 Megahuntmicro ****END OF FILE****/
