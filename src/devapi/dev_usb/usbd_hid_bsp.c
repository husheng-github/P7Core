/* Includes ------------------------------------------------------------------*/
#include <stdio.h>

#include "usbd_desc.h"
#include "usbd_hid_bsp.h"
#include "usbd_req.h"

/** @defgroup USBD_HID_Private_FunctionPrototypes
 * @{
 */
static uint8_t USBD_HID_Init(void* pdev, uint8_t cfgidx);

static uint8_t USBD_HID_DeInit(void* pdev, uint8_t cfgidx);

static uint8_t USBD_HID_Keyboard_Setup(void* pdev, USB_SETUP_REQ* req);

static uint8_t* USBD_HID_Keyboard_GetCfgDesc(uint8_t speed, uint16_t* length);

static uint8_t USBD_HID_DataIn(void* pdev, uint8_t epnum);

static uint8_t USBD_HID_EP0_RxReady(void* pdev);

/** @defgroup USBD_HID_Private_Variables
 * @{
 */
USBD_Class_cb_TypeDef USBD_HID_Keyboard_cb = {
    USBD_HID_Init,
    USBD_HID_DeInit,
    USBD_HID_Keyboard_Setup,
    NULL,                 /*EP0_TxSent*/
    USBD_HID_EP0_RxReady, /*EP0_RxReady*/
    USBD_HID_DataIn,      /*DataIn*/
    NULL,                 /*DataOut*/
    NULL,                 /*SOF */
    NULL,
    NULL,
    USBD_HID_Keyboard_GetCfgDesc
    //  USBD_HID_GetDeviceQualifierDesc,
};

__ALIGN_BEGIN static uint32_t USBD_HID_AltSet __ALIGN_END = 0;

__ALIGN_BEGIN static uint32_t USBD_HID_Protocol __ALIGN_END = 0;

__ALIGN_BEGIN static uint32_t USBD_HID_IdleState __ALIGN_END = 0;

/* USB HID device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_HID_CfgDesc_Keyboard[USB_HID_CONFIG_DESC_SIZ] __ALIGN_END = {
    0x09,                            /* bLength: Configuration Descriptor size */
    USB_DESC_TYPE_CONFIGURATION,     /* bDescriptorType: Configuration */
    LOBYTE(USB_HID_CONFIG_DESC_SIZ), /* wTotalLength: Bytes returned */
    HIBYTE(USB_HID_CONFIG_DESC_SIZ), /* */
    0x01,                            /*bNumInterfaces: 1 interface*/
    0x01,                            /*bConfigurationValue: Configuration value*/
    0x00,                            /*iConfiguration: Index of string descriptor describing the configuration*/
    0xE0,                            /*bmAttributes: bus powered and Support Remote Wake-up */
    0x32,                            /*MaxPower 100 mA: this current is used for detecting Vbus*/

    /************** Descriptor of Joystick Mouse interface ****************/
    /* 09 */
    0x09,                    /*bLength: Interface Descriptor size*/
    USB_DESC_TYPE_INTERFACE, /*bDescriptorType: Interface descriptor type*/
    0x00,                    /*bInterfaceNumber: Number of Interface*/
    0x00,                    /*bAlternateSetting: Alternate setting*/
    0x01,                    /*bNumEndpoints*/
    0x03,                    /*bInterfaceClass: HID*/
    0x01,                    /*bInterfaceSubClass : 1=BOOT, 0=no boot*/
    0x01,                    /*nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
    0x00,                    /*iInterface: Index of string descriptor*/
    /******************** Descriptor of Joystick Mouse HID ********************/
    /* 18 */
    0x09,                                                                             /*bLength: HID Descriptor size*/
    HID_DESCRIPTOR_TYPE,                                                              /*bDescriptorType: HID*/
    0x11,                                                                             /*bcdHID: HID Class Spec release number*/
    0x01, 0x00,                                                                       /*bCountryCode: Hardware target country*/
    0x01,                                                                             /*bNumDescriptors: Number of HID class descriptors to follow*/
    0x22,                                                                             /*bDescriptorType*/
    HID_KEYBOARD_REPORT_DESC_SIZE, /*wItemLength: Total length of Report descriptor*/ // modified for keyboard
    0x00,

    /******************** Descriptor of Mouse endpoint ********************/
    /* 27 */
    0x07,                   /*bLength: Endpoint Descriptor size*/
    USB_DESC_TYPE_ENDPOINT, /*bDescriptorType:*/

    HID_EPIN_ADDR, /*bEndpointAddress: Endpoint Address (IN)*/
    0x03,          /*bmAttributes: Interrupt endpoint*/
    HID_EPIN_SIZE, /*wMaxPacketSize*/
    0x00,          /*wMaxPacketSize*/
    HID_BINTERVAL, /*bInterval: Polling Interval (10 ms)*/
    /* 34 */
};

__ALIGN_BEGIN static uint8_t HID_KEYBOARD_ReportDesc[HID_KEYBOARD_REPORT_DESC_SIZE] __ALIGN_END = {
    0x05, 0x01, // USAGE_PAGE (Generic Desktop)
    0x09, 0x06, // USAGE (Keyboard)
    0xa1, 0x01, // COLLECTION (Application)

    0x05, 0x07, // USAGE_PAGE (Keyboard)
    0x19, 0xe0, // USAGE_MINIMUM (Keyboard LeftControl)
    0x29, 0xe7, // USAGE_MAXIMUM (Keyboard Right GUI)

    0x15, 0x00, // LOGICAL_MINIMUM (0)
    0x25, 0x01, // LOGICAL_MAXIMUM (1)
    0x75, 0x01, // REPORT_SIZE (1)
    0x95, 0x08, // REPORT_COUNT (8)

    0x81, 0x02, // INPUT (Data,Var,Abs)
    0x95, 0x01, // REPORT_COUNT (1)
    0x75, 0x08, // REPORT_SIZE (8)
    0x81, 0x03, // INPUT (Cnst,Var,Abs)

    0x95, 0x05, // REPORT_COUNT (5)
    0x75, 0x01, // REPORT_SIZE (1)
    0x05, 0x08, // USAGE_PAGE (LEDs)
    0x19, 0x01, // USAGE_MINIMUM (Num Lock)
    0x29, 0x05, // USAGE_MAXIMUM (Kana)
    0x91, 0x02, // OUTPUT (Data,Var,Abs)
    0x95, 0x01, // REPORT_COUNT (1)
    0x75, 0x03, // REPORT_SIZE (3)
    0x91, 0x03, // OUTPUT (Cnst,Var,Abs)
    0x95, 0x06, // REPORT_COUNT (6)
    0x75, 0x08, // REPORT_SIZE (8)
    0x15, 0x00, // LOGICAL_MINIMUM (0)
    0x25, 0xFF, // LOGICAL_MAXIMUM (255)
    0x05, 0x07, // USAGE_PAGE (Keyboard)
    0x19, 0x00, // USAGE_MINIMUM (Reserved (no event indicated))
    0x29, 0x65, // USAGE_MAXIMUM (Keyboard Application)
    0x81, 0x00, // INPUT (Data,Ary,Abs)
    0xc0        // END_COLLECTION
};

/* USB HID device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_HID_Keyboard_Desc[USB_HID_DESC_SIZ] __ALIGN_END = {
    /* 18 */
    0x09,                /*bLength: HID Descriptor size*/
    HID_DESCRIPTOR_TYPE, /*bDescriptorType: HID*/
    0x11,                /*bcdHID: HID Class Spec release number*/
    0x01,
    0x00,                          /*bCountryCode: Hardware target country*/
    0x01,                          /*bNumDescriptors: Number of HID class descriptors to follow*/
    0x22,                          /*bDescriptorType*/
    HID_KEYBOARD_REPORT_DESC_SIZE, /*wItemLength: Total length of Report descriptor*/
    0x00,
};

/**
 * @brief  USBD_HID_Init
 *         Initialize the HID interface
 * @param  pdev: device instance
 * @param  cfgidx: Configuration index
 * @retval status
 */
static uint8_t USBD_HID_Init(void* pdev, uint8_t cfgidx)
{
    /* Open EP IN */
    DCD_EP_Open(pdev, HID_EPIN_ADDR, HID_IN_PACKET, USB_OTG_EP_INT);

    return USBD_OK;
}

/**
 * @brief  USBD_HID_Init
 *         DeInitialize the HID layer
 * @param  pdev: device instance
 * @param  cfgidx: Configuration index
 * @retval status
 */
static uint8_t USBD_HID_DeInit(void* pdev, uint8_t cfgidx)
{
    /* Close HID EPs */
    DCD_EP_Close(pdev, HID_EPIN_ADDR);

    return USBD_OK;
}

uint8_t hid_rxbuffer[8];

/**
 * @brief  USBD_HID_Setup
 *         Handle the HID specific requests
 * @param  pdev: instance
 * @param  req: usb requests
 * @retval status
 */
static uint8_t USBD_HID_Keyboard_Setup(void* pdev, USB_SETUP_REQ* req)
{
    uint16_t len  = 0;
    uint8_t* pbuf = NULL;

    switch (req->bmRequest & USB_REQ_TYPE_MASK)
    {
        case USB_REQ_TYPE_CLASS:
            switch (req->bRequest)
            {
                case HID_REQ_SET_REPORT:
                    USBD_CtlPrepareRx(pdev, hid_rxbuffer, 1);
                    break;
                case HID_REQ_SET_PROTOCOL:
                    USBD_HID_Protocol = (uint8_t)(req->wValue);
                    break;

                case HID_REQ_GET_PROTOCOL:
                    USBD_CtlSendData(pdev, (uint8_t*)&USBD_HID_Protocol, 1);
                    break;

                case HID_REQ_SET_IDLE:
                    USBD_HID_IdleState = (uint8_t)(req->wValue >> 8);
                    break;

                case HID_REQ_GET_IDLE:
                    USBD_CtlSendData(pdev, (uint8_t*)&USBD_HID_IdleState, 1);
                    break;

                default:
                    USBD_CtlError(pdev, req);
                    return USBD_FAIL;
            }
            break;

        case USB_REQ_TYPE_STANDARD:
            switch (req->bRequest)
            {
                case USB_REQ_GET_DESCRIPTOR:
                    if (req->wValue >> 8 == HID_REPORT_DESC)
                    {
                        len  = MIN(HID_KEYBOARD_REPORT_DESC_SIZE, req->wLength);
                        pbuf = HID_KEYBOARD_ReportDesc;
                    }
                    else if (req->wValue >> 8 == HID_DESCRIPTOR_TYPE)
                    {
                        pbuf = USBD_HID_Keyboard_Desc;
                        len  = MIN(USB_HID_DESC_SIZ, req->wLength);
                    }
                    USBD_CtlSendData(pdev, pbuf, len);
                    break;

                case USB_REQ_GET_INTERFACE:
                    USBD_CtlSendData(pdev, (uint8_t*)&USBD_HID_AltSet, 1);
                    break;

                case USB_REQ_SET_INTERFACE:
                    USBD_HID_AltSet = (uint8_t)(req->wValue);
                    break;
            }
            break;
        default:
            USBD_CtlError(pdev, req);
            return USBD_FAIL;
    }
    return USBD_OK;
}

/**
 * @brief  USBD_HID_SendReport
 *         Send HID Report
 * @param  pdev: device instance
 * @param  buff: pointer to report
 * @retval status
 */
uint8_t USBD_HID_SendReport(USB_OTG_CORE_HANDLE* pdev, uint8_t* report, uint16_t len)
{
    if (pdev->dev.device_status == USB_OTG_CONFIGURED)
    {
        DCD_EP_Tx(pdev, HID_EPIN_ADDR, report, len);
        while (DCD_GetEPStatus(pdev, HID_EPIN_ADDR) != USB_OTG_EP_TX_VALID) {}
    }
    return USBD_OK;
}

/**
 * @brief  USBD_HID_GetCfgDesc
 *         return configuration descriptor
 * @param  speed : current device speed
 * @param  length : pointer data length
 * @retval pointer to descriptor buffer
 */
static uint8_t* USBD_HID_Keyboard_GetCfgDesc(uint8_t speed, uint16_t* length)
{
    *length = sizeof(USBD_HID_CfgDesc_Keyboard);
    return USBD_HID_CfgDesc_Keyboard;
}

/**
 * @brief  USBD_HID_DataIn
 *         handle data IN Stage
 * @param  pdev: device instance
 * @param  epnum: endpoint index
 * @retval status
 */
static uint8_t USBD_HID_DataIn(void* pdev, uint8_t epnum)
{
    /* Ensure that the FIFO is empty before a new transfer, this condition could
    be caused by  a new transfer before the end of the previous transfer */
    DCD_EP_Flush(pdev, HID_EPIN_ADDR);
    return USBD_OK;
}

static uint8_t USBD_HID_EP0_RxReady(void* pdev)
{
    USB_OTG_EP* ep = &((USB_OTG_CORE_HANDLE*)pdev)->dev.out_ep[0];
    if (ep->xfer_buff == hid_rxbuffer)
    {
        // Will fired when SET_REPORT request callback
        printf("setreport: %02X\n", hid_rxbuffer[0]);
    }
    return USBD_OK;
}
