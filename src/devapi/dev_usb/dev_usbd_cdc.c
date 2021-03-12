/********************************Copyright ( c ) ********************************
**                  DAQU 
**                         
**                        
**
**Created By:         
** Created Date:     
** Version:        
** Description:     USB Devices CDC接口程序 
****************************************************************************/
#include "USBBSP.h"

#include "hid_keyboard.h"
#include "usbd_hid_bsp.h"
#include "usbd_printer.h"
#include "usbd_cdc_core.h"

#include "usbd_usr.h"

#include "devglobal.h"


#if(USBD_CDC_EXIST==1)

static s32 g_usbd_cdc_fd=-1;
static volatile U32 g_usbd_connect_timerid = 0;
static volatile u8 g_usbd_connect = 0;


extern struct APP_DATA_STRUCT_DEF APP_Gdata_param; 
void dev_usbd_cdc_setconfig_flg(u8 flg)
{
    APP_Gdata_param.COM_config_cmp = flg;
}

/****************************************************************************
**Description:        打开USB Devices CDC设备
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
**Created by:        pengxuebin 20170811
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_usbd_cdc_open(u8 type)
{
    if(g_usbd_cdc_fd < 0)
    {
        USBSetup(type);
        g_usbd_cdc_fd = 0;
    }   
    return g_usbd_cdc_fd;
}
/****************************************************************************
**Description:	    关闭USB Devices CDC设备
**Input parameters:	
**Output parameters: 
**Returned value:
                    0:成功
**Created by:		pengxuebin 20170811
**--------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
s32 dev_usbd_cdc_close(void)
{
    USBPHY_CR1_TypeDef usbphy_cr1;

    if(g_usbd_cdc_fd >= 0)
    {
      #if 0
        usbphy_cr1.d32 = MHSCPU_READ_REG32(&SYSCTRL->USBPHY_CR1);
        usbphy_cr1.b.commononn = 0;
        usbphy_cr1.b.stop_ck_for_suspend = 0;
        MHSCPU_WRITE_REG32(&SYSCTRL->USBPHY_CR1,usbphy_cr1.d32);
      #endif
        SYSCTRL_AHBPeriphClockCmd(SYSCTRL_AHBPeriph_USB,DISABLE);
        SYSCTRL_AHBPeriphResetCmd(SYSCTRL_AHBPeriph_USB,DISABLE);
        dev_usbd_cdc_setconfig_flg(0);
        g_usbd_connect = 0;
        g_usbd_cdc_fd = -1;
    }   
    return 0;
}
void dev_usbd_cdc_settimerid(void)
{
    g_usbd_connect_timerid = dev_user_gettimeID();
}
s32 dev_usbd_cdc_get_status(void)
{
  #if 0  
    return USBD_USR_get_deivcestatus();
  #else
    u8 flg;
    if(g_usbd_connect == 0)
    {
        flg = APP_Gdata_param.COM_config_cmp;
        if(flg)
        {
//            if(1 == dev_user_querrytimer(g_usbd_connect_timerid, 50))
            {
                g_usbd_connect = 1;
//                VCP_UsbBufClean();
            }
        }
    }
    else
    {
        if(APP_Gdata_param.COM_config_cmp==0)
        {
            g_usbd_connect = 0;
//            VCP_UsbBufClean();
        }
    }
    return g_usbd_connect;
  #endif
}

s32 dev_usbd_cdc_write(u8 *wbuf, u16 wlen)
{
    if(dev_usbd_cdc_get_status())
    {
        if(VCP_DataTx(wbuf, wlen))
        {
            //发送失败
            APP_Gdata_param.COM_config_cmp = 0;
            g_usbd_connect = 0;
            return 0;
        }
        return wlen;     
    }   
    else
    {
        return 0;
    }
}    
s32 dev_usbd_cdc_read(u8 *rbuf, u16 rlen)
{
    s32 i;
    s32 ret;

    dev_usbd_cdc_get_status();
    for(i=0; i<rlen; i++)
    {
        ret = VCP_GetRxChar();
        if(ret < 0)
        {
            break;
        }
        rbuf[i] = ret&0xff;
    }
    return i;
}     
s32 dev_usbd_cdc_tcdrain(void)
{
    
    return 0;    
}     
s32 dev_usbd_cdc_flush(void)
{
    s32 i;
    s32 ret;
    
    for(i=0; i<100000; i++)
    {
        ret = VCP_GetRxChar();
        if(ret < 0)
        {
            break;
        }
    }
    return 0;    
}

s32 dev_usbd_print_write(u8 *wbuf, u16 wlen)
{
    if(PrinterTx(wbuf, wlen) != USBD_OK)
    {
        return 0;
    }
    return wlen;   
}

s32 dev_usbd_print_read(u8 *rbuf, u16 rlen)
{
    s32 i;
    s32 ret;

    for(i=0; i<rlen; i++)
    {
        ret = PrinterRxChar();
        if(ret < 0)
        {
            break;
        }
        rbuf[i] = ret&0xff;
    }
    return i;   
}
s32 dev_usbd_print_flush(void)
{
   
    s32 i;
    s32 ret;
    
    for(i=0; i<100000; i++)
    {
        ret = PrinterRxChar();
        if(ret < 0)
        {
            break;
        }
    }
    return 0;      
}



void usbd_cdc_test(void)
{
    u8 tmp[32];
    s32 i;
    
    dev_usbd_cdc_open(USB_DEV_PRINT_CDC);
    while(1)
    {
        i = VCP_GetRxChar();
        if(i >= 0)
        {
            dev_debug_printf("i=%02X\r\n", i);
            tmp[0] = i&0xff;
            VCP_DataTx((uint8_t *)tmp,1);
        }
    }
    
}
#else
#include "devglobal.h"
void dev_usbd_cdc_setconfig_flg(u8 flg)
{
}
s32 dev_usbd_cdc_open(void)
{
    return -1;    
}    
s32 dev_usbd_cdc_close(void)
{
    return -1;    
}   

void dev_usbd_cdc_settimerid(void)
{
}

s32 dev_usbd_cdc_get_status(void)
{
    return 0;
}

s32 dev_usbd_cdc_write(u8 *wbuf, u16 wlen)
{
    return -1;    
}    
s32 dev_usbd_cdc_read(u8 *rbuf, u16 rlen)
{
    return -1;    
}     
s32 dev_usbd_cdc_tcdrain(void)
{
    return -1;    
}     
s32 dev_usbd_cdc_flush(void)
{
    return -1;    
}    
#endif
