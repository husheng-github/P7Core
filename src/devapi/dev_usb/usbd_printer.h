/************************ (C) COPYRIGHT Megahuntmicro *************************
 * File Name            : usbd_printer.h
 * Author               : Megahuntmicro
 * Version              : V1.0.0
 * Date                 : 14-October-2020
 * Description          : Header for usbd_printer.c file.
 *****************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USBD_PRINTER_H
#define __USBD_PRINTER_H

/* Includes ------------------------------------------------------------------*/
#include "usb_core.h"
#include "usbd_desc.h"
#include "usbd_conf.h"
#include "usbd_req.h"
#include "usbd_ioreq.h"

#define USB_PRINTER_CONFIG_DESC_SIZ (9 + 9 + 7 + 7)
#define USB_PRINTER_DESC_SIZ        (USB_PRINTER_CONFIG_DESC_SIZ - 9)

#define PRINTER_RX_DATA_SIZE 2048
#define PRINTER_TX_DATA_SIZE 40960

#define PRINTER_OUT_PACKET_SIZE PRINTER_MAX_PACKET_SIZE
#define PRINTER_IN_PACKET_SIZE  PRINTER_MAX_PACKET_SIZE

/* Exported typef ------------------------------------------------------------*/
typedef struct
{
    /* Host Send buffer */
    struct
    {
        uint8_t APP_Tx_Buffer[PRINTER_TX_DATA_SIZE]; // Buffer
        volatile uint32_t APP_Tx_ptr_in;  // Data input offset
        volatile uint32_t APP_Tx_ptr_out; // Data output offset
        volatile uint32_t Tx_counter;     // Data ready size
    } tx_structure;
    /* Host Receive buffer */
    struct
    {
        uint8_t APP_Rx_Buffer[PRINTER_RX_DATA_SIZE]; // Buffer
        volatile uint32_t APP_Rx_ptr_in;  // Data input offset
        volatile uint32_t APP_Rx_ptr_out; // Data output offset
        volatile uint32_t Rx_counter;     // Data ready size
    } rx_structure;
}PrinterDataStruct;

/* Exported constants --------------------------------------------------------*/
extern PrinterDataStruct PrinterData;
extern USBD_Class_cb_TypeDef USBD_Printer_cb;

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

extern uint16_t PrinterTx(uint8_t* buf, uint32_t len);
extern int32_t PrinterRxChar(void);

#endif

/************************ (C) COPYRIGHT 2020 Megahuntmicro ****END OF FILE****/
