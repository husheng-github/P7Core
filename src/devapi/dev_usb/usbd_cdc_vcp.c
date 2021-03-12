/************************ (C) COPYRIGHT Megahuntmicro *************************
 * File Name            : usbd_cdc_vcp.c
 * Author               : Megahuntmicro
 * Version              : V1.0.0
 * Date                 : 21-October-2014
 * Description          : Generic media access Layer.
 *****************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc_vcp.h"
#include "usb_conf.h"
#include "usbd_cdc_core.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
LINE_CODING linecoding = {
    115200, /* baud rate*/
    0x00,   /* stop bits-1*/
    0x00,   /* parity - none*/
    0x08    /* nb. of bits 8*/
};

UART_InitTypeDef USART_InitStructure;

/* These are external variables imported from CDC core to be used for IN
   transfer management. */

extern struct APP_DATA_STRUCT_DEF APP_Gdata_param;

/* Private function prototypes -----------------------------------------------*/
static uint16_t VCP_Init(void);
static uint16_t VCP_DeInit(void);
static uint16_t VCP_Ctrl(uint32_t Cmd, uint8_t* Buf, uint32_t Len);
// static uint16_t VCP_DataTx(uint8_t* Buf, uint32_t Len);
static uint16_t VCP_DataRx(uint8_t* Buf, uint32_t Len);

static uint16_t VCP_COMConfig(uint8_t Conf);

CDC_IF_Prop_TypeDef VCP_fops = {VCP_Init, VCP_DeInit, VCP_Ctrl, VCP_DataTx, VCP_DataRx};

/* Private functions ---------------------------------------------------------*/
/**
 * @brief  VCP_Init
 *         Initializes the Media on the STM32
 * @param  None
 * @retval Result of the opeartion (USBD_OK in all cases)
 */
static uint16_t VCP_Init(void)
{
    APP_Gdata_param.COM_config_cmp = 0;
    memset(&APP_Gdata_param, 0, sizeof(APP_Gdata_param));
    return USBD_OK;
}

/**
 * @brief  VCP_DeInit
 *         DeInitializes the Media on the STM32
 * @param  None
 * @retval Result of the opeartion (USBD_OK in all cases)
 */
static uint16_t VCP_DeInit(void)
{
    return USBD_OK;
}

/**
 * @brief  VCP_Ctrl
 *         Manage the CDC class requests
 * @param  Cmd: Command code
 * @param  Buf: Buffer containing command data (request parameters)
 * @param  Len: Number of data to be sent (in bytes)
 * @retval Result of the opeartion (USBD_OK in all cases)
 */
static uint16_t VCP_Ctrl(uint32_t Cmd, uint8_t* Buf, uint32_t Len)
{
    switch (Cmd)
    {
        case SEND_ENCAPSULATED_COMMAND:
            /* Not  needed for this driver */
            break;

        case GET_ENCAPSULATED_RESPONSE:
            /* Not  needed for this driver */
            break;

        case SET_COMM_FEATURE:
            /* Not  needed for this driver */
            break;

        case GET_COMM_FEATURE:
            /* Not  needed for this driver */
            break;

        case CLEAR_COMM_FEATURE:
            /* Not  needed for this driver */
            break;

        case SET_LINE_CODING:
            linecoding.bitrate    = (uint32_t)(Buf[0] | (Buf[1] << 8) | (Buf[2] << 16) | (Buf[3] << 24));
            linecoding.format     = Buf[4];
            linecoding.paritytype = Buf[5];
            linecoding.datatype   = Buf[6];

            APP_Gdata_param.COM_config_cmp = 1;
            /* Set the new configuration */
            VCP_COMConfig(OTHER_CONFIG);
            break;

        case GET_LINE_CODING:
            Buf[0] = (uint8_t)(linecoding.bitrate);
            Buf[1] = (uint8_t)(linecoding.bitrate >> 8);
            Buf[2] = (uint8_t)(linecoding.bitrate >> 16);
            Buf[3] = (uint8_t)(linecoding.bitrate >> 24);
            Buf[4] = linecoding.format;
            Buf[5] = linecoding.paritytype;
            Buf[6] = linecoding.datatype;
            break;

        case SET_CONTROL_LINE_STATE:
            /* Not  needed for this driver */
            break;

        case SEND_BREAK:
            /* Not  needed for this driver */
            break;

        default:
            break;
    }

    return USBD_OK;
}

int isUsbBufFull()
{
    int ret = 0;
    NVIC_DisableIRQ(USB_IRQn);
    ret = APP_Gdata_param.rx_structure.Rx_counter >= CDC_APP_RX_DATA_SIZE;
    NVIC_EnableIRQ(USB_IRQn);
    return ret;
}
/**
 * @brief  VCP_DataTx
 *         CDC received data to be send over USB IN endpoint are managed in
 *         this function.
 * @param  Buf: Buffer of data to be sent
 * @param  Len: Number of data to be sent (in bytes)
 * @retval Result of the opeartion: USBD_OK if all operations are OK else USBD_FAIL
 */
uint16_t VCP_DataTx(uint8_t* buf, uint32_t len)
{
    uint32_t i       = 0;
    uint8_t  dataMsk = linecoding.datatype == 7 ? 0x7F : 0xFF;
	uint32_t timeid;
    
#ifdef USER_SPECIFIED_DATA_SOURCE
    for (i = 0; i < len; i++)
    {
        *(APP_Rx_Buffer + APP_Rx_ptr_in) = *buf++;
        APP_Rx_ptr_in++;
    }
#else
   
    for (i = 0; i < len; i++)
    {
        timeid = dev_user_gettimeID();			
              
        do
        {
            /*判断CDCD的发送BUF是否填满了数据*/
            if(isUsbBufFull())  //填满了数据
            {          
                dev_user_delay_us(10);

                /*判断是否超时*/
                if(dev_user_querrytimer_us(timeid, 100))  //100us是测试得到的数据，插拔USB不会死机 
                {
                    return USBD_BUSY;
                }            
            }
            else  //没有填满数据
            {
                break;  
            }
        } while (1);
      
				
        APP_Gdata_param.rx_structure.APP_Rx_Buffer[APP_Gdata_param.rx_structure.APP_Rx_ptr_in++] = (*buf++) & dataMsk;
        NVIC_DisableIRQ(USB_IRQn);
        APP_Gdata_param.rx_structure.Rx_counter++;
        NVIC_EnableIRQ(USB_IRQn);
        /* To avoid buffer overflow */
        if (APP_Gdata_param.rx_structure.APP_Rx_ptr_in >= CDC_APP_RX_DATA_SIZE)
        {
            APP_Gdata_param.rx_structure.APP_Rx_ptr_in = 0;
        }
    }
#endif

    return USBD_OK;
}

/**
 * @brief  Get_TxBuf_length
 *         Get the length of the remaining data to be transmitted
 * @param  NONE
 * @retval Result receive data length
 */
uint32_t VCP_GetTxBuflen(void)
{
    return APP_Gdata_param.rx_structure.Rx_counter;
}

/**
 * @brief  Get_RxBuf_rsaddr
 *         Get reading receive data starting position.
 * @param  NONE
 * @retval Result received data is read starting position
 */
uint32_t VCP_GetTxBufrsaddr(void)
{
    return (APP_Gdata_param.rx_structure.APP_Rx_ptr_out);
}

/**
 * @brief  Get_RxData
 *         Get receive data by byte
 * @param  NONE
 * @retval Result receive data
 */
int32_t VCP_GetRxChar(void)
{
    /* Nothing received */
    int ret = -1;
    if (!APP_Gdata_param.tx_structure.Tx_counter)
    {
        return -1;
    }
    else
    {
        NVIC_DisableIRQ(USB_IRQn);
        if (APP_Gdata_param.tx_structure.APP_Tx_ptr_out >= CDC_APP_TX_DATA_SIZE)
        {
            APP_Gdata_param.tx_structure.APP_Tx_ptr_out = 0;
        }
        APP_Gdata_param.tx_structure.Tx_counter--;
        ret = (APP_Gdata_param.tx_structure.APP_Tx_Buffer[APP_Gdata_param.tx_structure.APP_Tx_ptr_out++]);
        NVIC_EnableIRQ(USB_IRQn);
        return ret;
    }
}

/**
 * @brief  Get_RxBuf_length
 *         Get receive data length
 * @param  NONE
 * @retval Result receive data length
 */
uint32_t VCP_GetRxBuflen(void)
{
    return (APP_Gdata_param.tx_structure.Tx_counter);
}

/**
 * @brief  Get_RxBuf_rsaddr
 *         Get reading receive data starting position.
 * @param  NONE
 * @retval Result received data is read starting position
 */
uint8_t* VCP_GetRxBufrsaddr(void)
{
    return (&(APP_Gdata_param.tx_structure.APP_Tx_Buffer[APP_Gdata_param.tx_structure.APP_Tx_ptr_out]));
}

/**
 * @brief  VCP_DataRx
 *         Data received over USB OUT endpoint are sent over CDC interface
 *         through this function.
 *
 *         @note
 *         This function will block any OUT packet reception on USB endpoint
 *         untill exiting this function. If you exit this function before transfer
 *         is complete on CDC interface (ie. using DMA controller) it will result
 *         in receiving more data while previous ones are still not sent.
 *
 * @param  Buf: Buffer of data to be received
 * @param  Len: Number of data received (in bytes)
 * @retval Result of the opeartion: USBD_OK if all operations are OK else VCP_FAIL
 */
static uint16_t VCP_DataRx(uint8_t* Buf, uint32_t Len)
{
    uint32_t i;
    for (i = 0; i < Len; i++)
    {
        APP_Gdata_param.tx_structure.APP_Tx_Buffer[APP_Gdata_param.tx_structure.APP_Tx_ptr_in] = *(Buf + i);
        APP_Gdata_param.tx_structure.APP_Tx_ptr_in++;
        APP_Gdata_param.tx_structure.Tx_counter++;
        if (APP_Gdata_param.tx_structure.Tx_counter > CDC_APP_TX_DATA_SIZE)
        {
            APP_Gdata_param.tx_structure.Tx_counter = CDC_APP_TX_DATA_SIZE;
            APP_Gdata_param.tx_structure.APP_Tx_ptr_out++;
            APP_Gdata_param.tx_structure.APP_Tx_ptr_out %= CDC_APP_TX_DATA_SIZE;
        }

        if (APP_Gdata_param.tx_structure.APP_Tx_ptr_in >= CDC_APP_TX_DATA_SIZE)
        {
            APP_Gdata_param.tx_structure.APP_Tx_ptr_in = 0;
        }
    }
    return USBD_OK;
}

/**
 * @brief  VCP_COMConfig
 *         Configure the COM Port with default values or values received from host.
 * @param  Conf: can be DEFAULT_CONFIG to set the default configuration or OTHER_CONFIG
 *         to set a configuration received from the host.
 * @retval None.
 */
static uint16_t VCP_COMConfig(uint8_t Conf)
{
    if (Conf == DEFAULT_CONFIG) {}
    else
    {
        /* set the Stop bit*/
        switch (linecoding.format)
        {
            case 0:
                USART_InitStructure.UART_StopBits = UART_StopBits_1;
                break;
            case 1:
                USART_InitStructure.UART_StopBits = UART_StopBits_1_5;
                break;
            case 2:
                USART_InitStructure.UART_StopBits = UART_StopBits_2;
                break;
            default:
                VCP_COMConfig(DEFAULT_CONFIG);
                return (USBD_FAIL);
        }

        /* set the parity bit*/
        switch (linecoding.paritytype)
        {
            case 0:
                USART_InitStructure.UART_Parity = UART_Parity_No;
                break;
            case 1:
                USART_InitStructure.UART_Parity = UART_Parity_Even;
                break;
            case 2:
                USART_InitStructure.UART_Parity = UART_Parity_Odd;
                break;
            default:
                VCP_COMConfig(DEFAULT_CONFIG);
                return (USBD_FAIL);
        }

        /*set the data type : only 8bits and 9bits is supported */
        switch (linecoding.datatype)
        {
            case 0x07:
                /* With this configuration a parity (Even or Odd) should be set */
                USART_InitStructure.UART_WordLength = UART_WordLength_8b;
                break;
            case 0x08:
                if (USART_InitStructure.UART_Parity == UART_Parity_No)
                {
                    USART_InitStructure.UART_WordLength = UART_WordLength_8b;
                }
                else
                {
                    // USART_InitStructure.UART_WordLength = UART_WordLength_9b;
                }

                break;
            default:
                // VCP_COMConfig(DEFAULT_CONFIG);
                return (USBD_FAIL);
        }

        USART_InitStructure.UART_BaudRate = linecoding.bitrate;
    }
    return USBD_OK;
}

/************************ (C) COPYRIGHT 2014 Megahuntmicro ****END OF FILE****/
