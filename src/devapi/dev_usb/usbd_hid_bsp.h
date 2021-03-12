/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USBD_HID_BSP_H__
#define __USBD_HID_BSP_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "usbd_ioreq.h"

#define HID_EPIN_ADDR 0x83
#define HID_EPIN_SIZE 0x08

#define USB_HID_CONFIG_DESC_SIZ    34
#define USB_HID_DESC_SIZ           9
#define HID_MOUSE_REPORT_DESC_SIZE 74

#define HID_DESCRIPTOR_TYPE 0x21
#define HID_REPORT_DESC     0x22

#define HID_BINTERVAL        0x02
#define HID_POLLING_INTERVAL 0x0A

#define HID_REQ_SET_PROTOCOL 0x0B
#define HID_REQ_GET_PROTOCOL 0x03

#define HID_REQ_SET_IDLE 0x0A
#define HID_REQ_GET_IDLE 0x02

#define HID_REQ_SET_REPORT 0x09
#define HID_REQ_GET_REPORT 0x01

#define HID_KEYBOARD_REPORT_DESC_SIZE 63

extern USBD_Class_cb_TypeDef USBD_HID_Keyboard_cb;

/** @defgroup USB_CORE_Exported_Functions
 * @{
 */
uint8_t USBD_HID_SendReport(USB_OTG_CORE_HANDLE* pdev, uint8_t* report, uint16_t len);
/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __USBD_HID_KEYBOARD_H__ */
