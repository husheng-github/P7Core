#include "usbd_printer.h"

__ALIGN_BEGIN uint8_t RxBuffer[PRINTER_OUT_PACKET_SIZE] __ALIGN_END;

PrinterDataStruct PrinterData;

uint8_t Printer_Tx_State = 0;

/* USB CDC device Configuration Descriptor */
__ALIGN_BEGIN uint8_t usbd_printer_CfgDesc[USB_PRINTER_CONFIG_DESC_SIZ] __ALIGN_END = {
    /*Configuration Descriptor*/
    0x09,                                /* bLength: Configuration Descriptor size */
    USB_CONFIGURATION_DESCRIPTOR_TYPE,   /* bDescriptorType: Configuration */
    LOBYTE(USB_PRINTER_CONFIG_DESC_SIZ), /* wTotalLength:no of returned bytes */
    HIBYTE(USB_PRINTER_CONFIG_DESC_SIZ), /* */
    0x01,                                /* bNumInterfaces: 1 interface */
    0x01,                                /* bConfigurationValue: Configuration value */
    0x00,                                /* iConfiguration: Index of string descriptor describing the configuration */
    0xC0,                                /* bmAttributes: self powered */
    0x32,                                /* MaxPower 100 mA */

    /* Printer class interface descriptor*/
    0x09,                          /* bLength: Endpoint Descriptor size */
    USB_INTERFACE_DESCRIPTOR_TYPE, /* bDescriptorType: */
    0x00,                          /* bInterfaceNumber: Number of Interface */
    0x00,                          /* bAlternateSetting: Alternate setting */
    0x02,                          /* bNumEndpoints: Two endpoints used */
    0x07,                          /* bInterfaceClass: Printer */
    0x01,                          /* bInterfaceSubClass: Printer */
    0x02,                          /* bInterfaceProtocol: Bidirectional */
    0x00,                          /* iInterface: */

    /*Endpoint OUT Descriptor*/
    0x07,                             /* bLength: Endpoint Descriptor size */
    USB_ENDPOINT_DESCRIPTOR_TYPE,     /* bDescriptorType: Endpoint */
    PRINTER_OUT_EP,                   /* bEndpointAddress */
    0x02,                             /* bmAttributes: Bulk */
    LOBYTE(CDC_DATA_MAX_PACKET_SIZE), /* wMaxPacketSize: */
    HIBYTE(CDC_DATA_MAX_PACKET_SIZE), /* */
    0x00,                             /* bInterval: ignore for Bulk transfer */

    /*Endpoint IN Descriptor*/
    0x07,                             /* bLength: Endpoint Descriptor size */
    USB_ENDPOINT_DESCRIPTOR_TYPE,     /* bDescriptorType: Endpoint */
    PRINTER_IN_EP,                    /* bEndpointAddress */
    0x02,                             /* bmAttributes: Bulk */
    LOBYTE(CDC_DATA_MAX_PACKET_SIZE), /* wMaxPacketSize: */
    HIBYTE(CDC_DATA_MAX_PACKET_SIZE), /* */
    0x00                              /* bInterval: ignore for Bulk transfer */
};

int IsPrinterTxBufferFull() {
    int ret = 0;
    NVIC_DisableIRQ(USB_IRQn);
    ret = PrinterData.rx_structure.Rx_counter >= PRINTER_RX_DATA_SIZE;
    NVIC_EnableIRQ(USB_IRQn);
    return ret;
}

uint16_t PrinterTx(uint8_t* buf, uint32_t len) {
    uint32_t i = 0;

    for (i = 0; i < len; i++) {
        while (IsPrinterTxBufferFull()) {
            int t;
            for (t = 0; t < 1000; t++) {}
        }
        PrinterData.rx_structure.APP_Rx_Buffer[PrinterData.rx_structure.APP_Rx_ptr_in++] = (*buf++);
        NVIC_DisableIRQ(USB_IRQn);
        PrinterData.rx_structure.Rx_counter++;
        NVIC_EnableIRQ(USB_IRQn);
        /* To avoid buffer overflow */
        if (PrinterData.rx_structure.APP_Rx_ptr_in >= PRINTER_RX_DATA_SIZE) {
            PrinterData.rx_structure.APP_Rx_ptr_in = 0;
        }
    }

    return USBD_OK;
}

int32_t PrinterRxChar(void) {
    int ret = -1;
    if (!PrinterData.tx_structure.Tx_counter) {
        return -1;
    }
    else {
        NVIC_DisableIRQ(USB_IRQn);
        if (PrinterData.tx_structure.APP_Tx_ptr_out >= PRINTER_TX_DATA_SIZE) {
            PrinterData.tx_structure.APP_Tx_ptr_out = 0;
        }
        PrinterData.tx_structure.Tx_counter--;
        ret = (PrinterData.tx_structure.APP_Tx_Buffer[PrinterData.tx_structure.APP_Tx_ptr_out++]);
        NVIC_EnableIRQ(USB_IRQn);
        return ret;
    }
}

uint32_t PrinterRxBufLen(void) {
    return (PrinterData.tx_structure.Tx_counter);
}

uint8_t* PrinterGetRxBufAddr(void) {
    return (&(PrinterData.tx_structure.APP_Tx_Buffer[PrinterData.tx_structure.APP_Tx_ptr_out]));
}

static uint16_t PrinterRx(uint8_t* buf, uint32_t len) {
    for (uint32_t i = 0; i < len; i++) {
        PrinterData.tx_structure.APP_Tx_Buffer[PrinterData.tx_structure.APP_Tx_ptr_in] = *(buf + i);
        PrinterData.tx_structure.APP_Tx_ptr_in++;
        PrinterData.tx_structure.Tx_counter++;
        if (PrinterData.tx_structure.Tx_counter > PRINTER_TX_DATA_SIZE) {
            PrinterData.tx_structure.Tx_counter = PRINTER_TX_DATA_SIZE;
            PrinterData.tx_structure.APP_Tx_ptr_out++;
            PrinterData.tx_structure.APP_Tx_ptr_out %= PRINTER_TX_DATA_SIZE;
        }

        if (PrinterData.tx_structure.APP_Tx_ptr_in >= PRINTER_TX_DATA_SIZE) {
            PrinterData.tx_structure.APP_Tx_ptr_in = 0;
        }
    }
    return USBD_OK;
}

static uint8_t usbd_printer_Init(void* pdev, uint8_t cfgidx) {
    /* Open EP IN */
    DCD_EP_Open(pdev, PRINTER_IN_EP, PRINTER_IN_PACKET_SIZE, USB_OTG_EP_BULK);

    /* Open EP OUT */
    DCD_EP_Open(pdev, PRINTER_OUT_EP, PRINTER_OUT_PACKET_SIZE, USB_OTG_EP_BULK);

    memset(&PrinterData, 0, sizeof(PrinterData));

    /* Prepare Out endpoint to receive next packet */
    DCD_EP_PrepareRx(pdev, PRINTER_OUT_EP, (uint8_t*)(RxBuffer), PRINTER_OUT_PACKET_SIZE);
    return USBD_OK;
}

static uint8_t usbd_printer_DeInit(void* pdev, uint8_t cfgidx) {
    /* Open EP IN */
    DCD_EP_Close(pdev, PRINTER_IN_EP);

    /* Open EP OUT */
    DCD_EP_Close(pdev, PRINTER_OUT_EP);

    /* Restore default state of the Interface physical components */

    return USBD_OK;
}

uint8_t PrinterDeviceID[] = "  MANUFACTURER:MEGAHUNT;\
COMMAND SET:ESC/POS;\
MODEL:M_ONE_TYPE;\
COMMENT:Impact Printer;\
ACTIVE COMMAND:ESC/POS;";

static uint8_t usbd_printer_Setup(void* pdev, USB_SETUP_REQ* req) {
    uint16_t printerDeviceIDLength = sizeof(PrinterDeviceID) - 1;
    switch (req->bmRequest & USB_REQ_TYPE_MASK) {
        case USB_REQ_TYPE_CLASS:
            /* Check if the request is a data setup packet */
            if (req->wLength) {
                /* Check if the request is Device-to-Host */
                if (req->bmRequest & 0x80 && req->bRequest == 0) {
                    PrinterDeviceID[0] = printerDeviceIDLength >> 8;
                    PrinterDeviceID[1] = printerDeviceIDLength & 0xFF;
                    req->wLength       = printerDeviceIDLength;

                    /* Send the data to the host */
                    USBD_CtlSendData(pdev, PrinterDeviceID, req->wLength);
                    return USBD_OK;
                }
            }
            return USBD_OK;
        default:
            USBD_CtlError(pdev, req);
            return USBD_FAIL;
    }
}

static uint8_t usbd_printer_DataIn(void* pdev, uint8_t epnum) {
    uint32_t readyRxLength = PrinterData.rx_structure.Rx_counter;
    uint32_t readyRxOffset = PrinterData.rx_structure.APP_Rx_ptr_out;

    if (Printer_Tx_State == 0) {
        return USBD_OK;
    }

    // readyRxLength = MIN(readyRxLength, CDC_DATA_IN_PACKET_SIZE);
    readyRxLength = MIN(readyRxLength, PRINTER_RX_DATA_SIZE - readyRxOffset);

    if (readyRxLength == 0) {
        Printer_Tx_State = 0;
    }
    else {
        PrinterData.rx_structure.Rx_counter -= readyRxLength;
        PrinterData.rx_structure.APP_Rx_ptr_out += readyRxLength;
        PrinterData.rx_structure.APP_Rx_ptr_out %= PRINTER_RX_DATA_SIZE;
    }
    /* Prepare the available data buffer to be sent on IN endpoint */
    DCD_EP_Tx(pdev, PRINTER_IN_EP, (uint8_t*)&PrinterData.rx_structure.APP_Rx_Buffer[readyRxOffset], readyRxLength);
    return USBD_OK;
}

uint8_t print_data_end_flag = 0;
/**
 * @brief  usbd_cdc_DataOut
 *         Data received on non-control Out endpoint
 * @param  pdev: device instance
 * @param  epnum: endpoint number
 * @retval status
 */
static uint8_t usbd_printer_DataOut(void* pdev, uint8_t epnum) {
    uint16_t USB_Rx_Cnt;

    /* Get the received data buffer and update the counter */
    USB_Rx_Cnt = ((USB_OTG_CORE_HANDLE*)pdev)->dev.out_ep[epnum].xfer_count;
    /* USB data will be immediately processed, this allow next USB traffic being
       NAKed till the end of the application Xfer */
    PrinterRx(RxBuffer, USB_Rx_Cnt);

    if ((USB_Rx_Cnt == 2) && (RxBuffer[0] == 0x0D) && (RxBuffer[1] == 0x0A)) {
        print_data_end_flag = 1;
    }
    else {
        print_data_end_flag = 0;
    }

    /* Prepare Out endpoint to receive next packet */
    DCD_EP_PrepareRx(pdev, PRINTER_OUT_EP, (uint8_t*)(RxBuffer), PRINTER_OUT_PACKET_SIZE);

    return USBD_OK;
}

/**
 * @brief  usbd_audio_SOF
 *         Start Of Frame event management
 * @param  pdev: instance
 * @param  epnum: endpoint number
 * @retval status
 */
static uint8_t usbd_printer_SOF(void* pdev) {
    static uint32_t                      FrameCount = 0;
    USB_OTG_TXCSRL_IN_PERIPHERAL_TypeDef txcsrl;

    if (FrameCount++ < PRINTER_IN_FRAME_INTERVAL)
        return USBD_OK;

    txcsrl.d8 = ((USB_OTG_CORE_HANDLE*)pdev)->regs.CSRREGS[PRINTER_IN_EP & 0x7f]->TXCSRL;
    if (txcsrl.b.fifo_not_empty) {
        return USBD_OK;
    }
    /* Reset the frame counter */
    FrameCount = 0;

    /* Check the data to be sent through IN pipe */
    if (Printer_Tx_State != 1) {
        uint32_t readyRxLength = PrinterData.rx_structure.Rx_counter;
        uint32_t readyRxOffset = PrinterData.rx_structure.APP_Rx_ptr_out;

        readyRxLength = MIN(readyRxLength, PRINTER_RX_DATA_SIZE - readyRxOffset);

        if (readyRxLength == 0)
            return USBD_OK;
        PrinterData.rx_structure.Rx_counter -= readyRxLength;
        PrinterData.rx_structure.APP_Rx_ptr_out += readyRxLength;
        PrinterData.rx_structure.APP_Rx_ptr_out %= PRINTER_RX_DATA_SIZE;
        Printer_Tx_State = 1;

        DCD_EP_Tx(pdev, PRINTER_IN_EP, (uint8_t*)&PrinterData.rx_structure.APP_Rx_Buffer[readyRxOffset], readyRxLength);
    }

    return USBD_OK;
}

static uint8_t* USBD_printer_GetCfgDesc(uint8_t speed, uint16_t* length) {
    *length = sizeof(usbd_printer_CfgDesc);
    return usbd_printer_CfgDesc;
}

USBD_Class_cb_TypeDef USBD_Printer_cb = {
    usbd_printer_Init,
    usbd_printer_DeInit,

    usbd_printer_Setup,
    NULL,
    NULL,

    usbd_printer_DataIn,
    usbd_printer_DataOut,
    usbd_printer_SOF,

    NULL,
    NULL,
    USBD_printer_GetCfgDesc,
};
