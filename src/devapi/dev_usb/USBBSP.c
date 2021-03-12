#include "USBBSP.h"

#include "hid_keyboard.h"
#include "usbd_hid_bsp.h"
#include "usbd_printer.h"
#include "usbd_cdc_core.h"

#include "usbd_usr.h"
#include "devglobal.h"

__ALIGN_BEGIN USB_OTG_CORE_HANDLE USBDev __ALIGN_END;

USBD_Class_cb_TypeDef* PrinterCallback;
USBD_Class_cb_TypeDef* HIDCallback;
USBD_Class_cb_TypeDef* VCPCallback;

static uint8_t  CompositeInit(void* pdev, uint8_t cfgidx);
static uint8_t  CompositeDeInit(void* pdev, uint8_t cfgidx);
static uint8_t  CompositeSetup(void* pdev, USB_SETUP_REQ* req);
static uint8_t  CompositeEP0_TxSent(void* pdev);
static uint8_t  CompositeEP0_RxReady(void* pdev);
static uint8_t  CompositeDataIn(void* pdev, uint8_t epnum);
static uint8_t  CompositeDataOut(void* pdev, uint8_t epnum);
static uint8_t  CompositeSOF(void* pdev);
static uint8_t* GetCompositeDescriptor(uint8_t speed, uint16_t* length);

USBD_Class_cb_TypeDef USBCompositeCB = {
    CompositeInit,
    CompositeDeInit,
    /* Control Endpoints*/
    CompositeSetup,
    CompositeEP0_TxSent,
    CompositeEP0_RxReady,
    /* Class Specific Endpoints*/
    CompositeDataIn,
    CompositeDataOut,
    CompositeSOF,

    NULL,
    NULL,

    GetCompositeDescriptor,
};


#define USB_Composite_CONFIG_DESC_SIZ (9 + (9 + 9 + 7) + (8 + 9 + 5 + 5 + 4 + 5 + 9 + 7 + 7) + (9 + 7 + 7))

__ALIGN_BEGIN static uint8_t CompositeDescriptor[USB_Composite_CONFIG_DESC_SIZ] __ALIGN_END = {
    0x09,                                  /* bLength: Configuration Descriptor size */
    USB_CONFIGURATION_DESCRIPTOR_TYPE,     /* bDescriptorType: Configuration */
    LOBYTE(USB_Composite_CONFIG_DESC_SIZ), /* wTotalLength:no of returned bytes */
    HIBYTE(USB_Composite_CONFIG_DESC_SIZ), /* */
    USBD_ITF_MAX_NUM,                      /*bNumInterfaces: 3 interface*/
    0x01,                                  /*bConfigurationValue: Configuration value*/
    0x00,                                  /*iConfiguration: Index of string descriptor describing the configuration*/
    0xE0,                                  /*bmAttributes: bus powered and Support Remote Wake-up */
    0xC8,                                  /*MaxPower 400 mA: this current is used for detecting Vbus*/
};

void USBSetup(uint8_t type) {
    NVIC_InitTypeDef   NVIC_InitStructure;
    USBPHY_CR1_TypeDef usbphy_cr1;

    if(type == USB_DEV_PRINT_ONLY)
    {
        CompositeDescriptor[4] = 0x01;   //usb只枚举出一个usb打印机设备
    }
    else if(type == USB_DEV_PRINT_CDC)
    {
        CompositeDescriptor[4] = 0x04;
    }

    SYSCTRL_AHBPeriphClockCmd(SYSCTRL_AHBPeriph_USB, ENABLE);
    SYSCTRL_AHBPeriphResetCmd(SYSCTRL_AHBPeriph_USB, ENABLE);

    /* Enable USB Interrupt */
    NVIC_SetPriorityGrouping(NVIC_PriorityGroup_1);
    NVIC_InitStructure.NVIC_IRQChannel                   = USB_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USB_OTG_BSP_mDelay(200);

    usbphy_cr1.d32                   = MHSCPU_READ_REG32(&SYSCTRL->USBPHY_CR1);
    usbphy_cr1.b.commononn           = 0;
    usbphy_cr1.b.stop_ck_for_suspend = 0;
    MHSCPU_WRITE_REG32(&SYSCTRL->USBPHY_CR1, usbphy_cr1.d32);

    memset(&USBDev, 0x00, sizeof(USBDev));

    HIDCallback     = &USBD_HID_Keyboard_cb;
    PrinterCallback = &USBD_Printer_cb;
    VCPCallback     = &USBD_CDC_cb;

    Keyboard_Configuration(&USBDev);
    USBD_Init(&USBDev, USB_OTG_FS_CORE_ID, &USR_desc, &USBCompositeCB, &USRD_cb);
}

void USB_IRQHandler(void) {
    USBD_OTG_ISR_Handler(&USBDev);
    NVIC_ClearPendingIRQ(USB_IRQn);
}

static uint8_t CompositeInit(void* pdev, uint8_t cfgidx) {
    HIDCallback->Init(pdev, cfgidx);
    PrinterCallback->Init(pdev, cfgidx);
    VCPCallback->Init(pdev, cfgidx);
    return USBD_OK;
}

static uint8_t CompositeDeInit(void* pdev, uint8_t cfgidx) {
    HIDCallback->DeInit(pdev, cfgidx);
    PrinterCallback->DeInit(pdev, cfgidx);
    VCPCallback->DeInit(pdev, cfgidx);
    return USBD_OK;
}

static uint8_t CompositeSetup(void* pdev, USB_SETUP_REQ* req) {
    switch (LOBYTE(req->wIndex)) {
        case 0:
            return PrinterCallback->Setup(pdev, req);
        case 1:
            return VCPCallback->Setup(pdev, req);
        case 3:
            return HIDCallback->Setup(pdev, req);
        default:
            return USBD_FAIL;
    }
}

static uint8_t CompositeEP0_TxSent(void* pdev) {
    return USBD_OK;
}

static uint8_t CompositeEP0_RxReady(void* pdev) {
    VCPCallback->EP0_RxReady(pdev);
    HIDCallback->EP0_RxReady(pdev);
    return USBD_OK;
}

static uint8_t CompositeDataIn(void* pdev, uint8_t epnum) {
    epnum = epnum | 0x80;
    switch (epnum) {
        case CDC_IN_EP:
            return VCPCallback->DataIn(pdev, epnum);
        case HID_EPIN_ADDR:
            return HIDCallback->DataIn(pdev, epnum);
        case PRINTER_IN_EP:
            return PrinterCallback->DataIn(pdev, epnum);
        default:
            return USBD_FAIL;
    }
}

static uint8_t CompositeDataOut(void* pdev, uint8_t epnum) {
    switch (epnum) {
        case CDC_OUT_EP:
            return VCPCallback->DataOut(pdev, epnum);
        case PRINTER_OUT_EP:
            return PrinterCallback->DataOut(pdev, epnum);
        default:
            return USBD_FAIL;
    }
}

static uint8_t CompositeSOF(void* pdev) {
    VCPCallback->SOF(pdev);
    return USBD_OK;
}

static uint8_t* GetCompositeDescriptor(uint8_t speed, uint16_t* length)
{
    uint16_t printerDescriptorSize, hidDescriptorSize, vcpDescriptorSize;

    uint8_t* printerDescriptor = PrinterCallback->GetConfigDescriptor(speed, &printerDescriptorSize);
    printerDescriptor[9 + 2]   = 0; // Printer Interface 3
    memcpy(CompositeDescriptor + 9, printerDescriptor + 9, printerDescriptorSize - 9);

    uint8_t* vcpDescriptor    = VCPCallback->GetConfigDescriptor(speed, &vcpDescriptorSize);
    vcpDescriptor[9 + 2]      = 1; // First Interface 1
    vcpDescriptor[9 + 8 + 2]  = 1; // CDC Interface 1
    vcpDescriptor[9 + 36 + 2] = 2; // Data Interface 2
    memcpy(CompositeDescriptor + printerDescriptorSize, vcpDescriptor + 9, vcpDescriptorSize - 9);

    uint8_t* hidDescriptor = HIDCallback->GetConfigDescriptor(speed, &hidDescriptorSize);
    hidDescriptor[9 + 2]   = 3; // HID Interface 0
    memcpy(CompositeDescriptor + printerDescriptorSize + vcpDescriptorSize - 9, hidDescriptor + 9, hidDescriptorSize - 9);

    *length = hidDescriptorSize + vcpDescriptorSize - 9 + printerDescriptorSize - 9;
    return CompositeDescriptor;
}
