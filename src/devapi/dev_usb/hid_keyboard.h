
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HID_KEYBOARD__H__
#define __HID_KEYBOARD__H__

#include "mhscpu.h"
#include "usb_bsp.h"
#include "usb_dcd_int.h"
#include "usb_otg.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_hid_bsp.h"
#include "usbd_usr.h"

extern void Keyboard_Configuration(USB_OTG_CORE_HANDLE* pdev);
extern void Keyboard_SendKey(char key);

#endif
