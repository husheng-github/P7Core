/********************************Copyright ( c ) ********************************
**                  DAQU 
**                         
**                        
**
**Created By:         
** Created Date:     
** Version:        
** Description:     串行口接口程序 
****************************************************************************/
#include "devglobal.h"
#include "drv_uart.h"


static s32 g_uart_fd[PORT_NO_MAX];
/****************************************************************************
**Description:        初始化串口设备
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
**Created by:        pengxuebin 20170317
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void dev_com_init(void)
{
    s32 i;
    
    drv_com_init();
    for(i=0; i<PORT_NO_MAX; i++)
    {
        g_uart_fd[i] = -1;
    }
}
/****************************************************************************
**Description:        打开串口设备
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin 20170317
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_com_open(s32 nCom, s32 baud, s32 databits, u32 parity, s32 stopbits, s32 flowctl)
{
    s32 ret = DEVSTATUS_ERR_PARAM_ERR;
    
    if(nCom >= PORT_NO_MAX)
    {
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    if(g_uart_fd[nCom]>=0)
    {
        return 0;
    }
    if(nCom < PORT_UART_PHY_MAX)
    {
        ret = drv_com_open(nCom, baud, databits, parity, stopbits, flowctl);
    }
    else if(nCom == PORT_HID_NO)
    {
        //ret = 0;
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    else if(nCom == PORT_CDCD_NO)
    {
        ret = dev_usbd_cdc_open(USB_DEV_PRINT_CDC);
        g_uart_fd[PORT_PRINT_NO] = ret;
    }
    #if 0  //sxl20190917
    else if(nCom == PORT_SPI_NO)
    {
        ret = dev_spi_open(SPI_DEV_AP);
    }
    #endif
    else if(nCom == PORT_PRINT_NO)
    {
        ret = dev_usbd_cdc_open(USB_DEV_PRINT_ONLY);
        g_uart_fd[PORT_CDCD_NO] = ret;
    }
    else
    {   //参数错误
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    if(ret >= 0)
    {
        g_uart_fd[nCom] = ret;
    }
    return ret;
}
/****************************************************************************
**Description:        关闭串口设备
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
**Created by:        pengxuebin 20170317
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_com_close(s32 nCom)
{
    s32 ret = DEVSTATUS_ERR_PARAM_ERR;
    
    if(nCom >= PORT_NO_MAX)
    {
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    if(g_uart_fd[nCom]<0)
    {
        return 0;
    }
    g_uart_fd[nCom] = -1;
    if(nCom < PORT_UART_PHY_MAX)
    {
        ret = drv_com_close(nCom);
    }
    else if(nCom == PORT_HID_NO)
    {
        //ret = 0;
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    else if(nCom == PORT_CDCD_NO)
    {
        ret = dev_usbd_cdc_close();
        g_uart_fd[PORT_PRINT_NO] = -1;
    }
    #if 0  //sxl20190917
    else if(nCom == PORT_SPI_NO)
    {
        ret = dev_spi_close(SPI_DEV_AP);
    }
    #endif
    else if(nCom == PORT_PRINT_NO)
    {
        ret = dev_usbd_cdc_close();
        g_uart_fd[PORT_CDCD_NO] = -1;
    }
    return ret;
}
/****************************************************************************
**Description:        设置串口设备波特率
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
                    DEVSTATUS_ERR_DEVICE_NOTOPEN:串口没打开
**Created by:        pengxuebin 20170317
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_com_setbaud(s32 nCom,  s32 baud, s32 databits, u32 parity, s32 stopbits, s32 flowctl)
{
    s32 ret = DEVSTATUS_ERR_PARAM_ERR;
    
    if(nCom >= PORT_NO_MAX)
    {
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    if(g_uart_fd[nCom]<0)
    {
        return DEVSTATUS_ERR_DEVICE_NOTOPEN;
    }
    if(nCom < PORT_UART_PHY_MAX)
    {
        ret = drv_com_setbaud(nCom, baud, databits, parity, stopbits, flowctl);
    }
    else if(nCom == PORT_HID_NO)
    {
        ret = 0;
    }
    else if(nCom == PORT_CDCD_NO)
    {
        ret = 0;
    }
    else if(nCom == PORT_SPI_NO)
    {
        ret = 0;
    }
    else if(nCom == PORT_PRINT_NO)
    {
        ret = 0;
    }
    return ret;
}
/****************************************************************************
**Description:        向串口设备写数据
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
                    DEVSTATUS_ERR_DEVICE_NOTOPEN:串口没打开
**Created by:        pengxuebin 20170317
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_com_write(s32 nCom, u8 *wbuf, u16 wlen)
{
    s32 ret = DEVSTATUS_ERR_PARAM_ERR;
    u16 woffset = 0;
    
    if(nCom >= PORT_NO_MAX)
    {
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    if(g_uart_fd[nCom]<0)
    {
        return DEVSTATUS_ERR_DEVICE_NOTOPEN;
    }
    if(nCom < PORT_UART_PHY_MAX)
    {
      #if 1  
        while(woffset < wlen)
        {
            ret = drv_com_write(nCom, &wbuf[woffset], wlen-woffset); //sxl?2017不加限制的话会死机
            if(ret < 0)
            {
                break;
            }
            //drv_com_tcdrain(nCom);
            //dev_user_delay_us(100);
            woffset += ret;
        }
        ret = woffset;
      #else 
        ret = drv_com_write(nCom, &wbuf[woffset], wlen-woffset); //sxl?2017不加限制的话会死机
      #endif 
           
    }
    else if(nCom == PORT_HID_NO)
    {
        ret = 0;
    }
    else if(nCom == PORT_CDCD_NO)
    {
        ret = dev_usbd_cdc_write(wbuf, wlen);
    }
    #if 0 //sxl20190917
    else if(nCom == PORT_SPI_NO)
    {
        ret = dev_spi_slave_write(wbuf, wlen);
    }
    #endif
#ifndef TRENDIT_BOOT 
    else if(nCom == PORT_PRINT_NO)
    {
        drv_systick_set_run_switch(0);
        ret = dev_usbd_print_write(wbuf, wlen);
        drv_systick_set_run_switch(1);
    }
#endif
    return ret;
}
/****************************************************************************
**Description:        从串口设备读数据
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
                    DEVSTATUS_ERR_DEVICE_NOTOPEN:串口没打开
**Created by:        pengxuebin 20170317
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_com_read(s32 nCom, u8 *rbuf, u16 rlen)
{
    s32 ret = DEVSTATUS_ERR_PARAM_ERR;
    
    if(nCom >= PORT_NO_MAX)
    {
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    if(g_uart_fd[nCom]<0)
    {
        return DEVSTATUS_ERR_DEVICE_NOTOPEN;
    }
    if(nCom < PORT_UART_PHY_MAX)
    {
        ret = drv_com_read(nCom, rbuf, rlen);
    }
    else if(nCom == PORT_HID_NO)
    {
        ret = 0;
    }
    else if(nCom == PORT_CDCD_NO)
    {
        ret = dev_usbd_cdc_read(rbuf, rlen);
    }
    #if 0 //sxl20190917
    else if(nCom == PORT_SPI_NO)
    {
        ret = dev_spi_slave_read(rbuf, rlen);
    }
    #endif
#ifndef TRENDIT_BOOT     
    else if(nCom == PORT_PRINT_NO)
    {
        ret = dev_usbd_print_read(rbuf, rlen);
    }
#endif    
    return ret;
}
/****************************************************************************
**Description:        等待串口设备发送完
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
                    DEVSTATUS_ERR_DEVICE_NOTOPEN:串口没打开
**Created by:        pengxuebin 20170317
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_com_tcdrain(s32 nCom)
{
    s32 ret = DEVSTATUS_ERR_PARAM_ERR;
    
    if(nCom >= PORT_NO_MAX)
    {
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    if(g_uart_fd[nCom]<0)
    {
        return DEVSTATUS_ERR_DEVICE_NOTOPEN;
    }
    if(nCom < PORT_UART_PHY_MAX)
    {
        ret = drv_com_tcdrain(nCom);
    }
    else if(nCom == PORT_HID_NO)
    {
        ret = 0;
    }
    else if(nCom == PORT_CDCD_NO)
    {
        ret = dev_usbd_cdc_tcdrain();
    }
    #if 0  //sxl20190917
    else if(nCom == PORT_SPI_NO)
    {
        ret = dev_spi_slave_tcdrain();
    }
    #endif
    else if(nCom == PORT_PRINT_NO)
    {
        ret = 0;//dev_usbd_print_tcdrain();  
    }   
    return ret;
}
/****************************************************************************
**Description:        清FIFO
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
                    DEVSTATUS_ERR_DEVICE_NOTOPEN:串口没打开
**Created by:        pengxuebin 20170317
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_com_flush(s32 nCom)
{
    s32 ret = DEVSTATUS_ERR_PARAM_ERR;
    
    if(nCom >= PORT_NO_MAX)
    {
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    if(g_uart_fd[nCom]<0)
    {
        return DEVSTATUS_ERR_DEVICE_NOTOPEN;
    }
    if(nCom < PORT_UART_PHY_MAX)
    {
        ret = drv_com_flush(nCom);
    }
    else if(nCom == PORT_HID_NO)
    {
        ret = 0;
    }
    else if(nCom == PORT_CDCD_NO)
    {
        ret = dev_usbd_cdc_flush();
    }
    #if 0  //sxl20190917
    else if(nCom == PORT_SPI_NO)
    {
        ret = dev_spi_slave_tcdrain();
    }
    #endif
#ifndef TRENDIT_BOOT     
    else if(nCom == PORT_PRINT_NO)
    {
        ret = dev_usbd_print_flush();  
    }   
#endif
    return ret;
}
/**
 * @brief 获取波特率 
 * @param [in] nCom: 串口号 
 * @param [out] *bps:返回当前波特率
 * @retval =0 获取成功，
           <0 获取失败
 * @since pengxuebin,20190813 由于1902 96Mhz影响无线通讯，改72Mhz主频通讯
 */
s32 dev_com_get_baud(u32 nCom, u32 *baud)
{
    return drv_com_get_baud(nCom, baud);
}

/**
 * @brief 串口ioctl控制
 * @param [in] nCom: 串口号 
 * @param [out] *bps:返回当前波特率
 * @retval =0 获取成功，
           <0 获取失败
 * @since pengxuebin,20190813 由于1902 96Mhz影响无线通讯，改72Mhz主频通讯
 */
s32 dev_com_ioctl(u32 nCom, u32 nCmd, u32 lParam, u32 wParam)
{
    s32 ret=DEVSTATUS_ERR_PARAM_ERR;   
    
    if(nCom >= PORT_NO_MAX)
    {
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    if(g_uart_fd[nCom]<0)
    {
        return DEVSTATUS_ERR_DEVICE_NOTOPEN;
    }
#if 0 //20191016 pengxuebin,??   
    switch(nCmd)
    {
    case DDI_COM_CTL_BAUD:            //(0) //设置波特率
    case DDI_COM_CTL_FLWCTL:            //(1) //设置流控
        break;
    case DDI_COM_GET_BAUD:            //(2) //获取设备波特率
        ret = dev_com_get_baud(nCom, (u32*)wParam);
        if(ret < 0)
        {
            ret = DEVSTATUS_ERR_FAIL;
        }
        break;
    }
#endif    
    return ret;
}
void dev_com_writedirect(u8 data)
{
    drv_com_writedirect(data);
}

#define TEST_PORT   0//PORT_CDCD_NO
void dev_com_test(void)
{
  #if 1  
    u8 tmp[32];
    s32 i;

  #if 0  
    dev_com_close(TEST_PORT);
    dev_com_open(TEST_PORT, 115200, 8, 'n', 0, 0);
    while(1)
    {
        if(dev_usbd_cdc_get_status())
        {
            dev_com_write(TEST_PORT, "connect\r\n", 9);
            break;
        }
    }
  #endif  
    while(1)
    {
        i = dev_com_read(TEST_PORT, tmp, 32);
        if(i > 0)
        {
            //dev_debug_printf(tmp, i);
            //tmp[0] = i&0xff;
            //VCP_DataTx((uint8_t *)tmp,1);
            //dev_com_write(PORT_CDCD_NO, tmp, i);
            dev_com_write(0, tmp, i);
        }
        i = dev_com_read(0, tmp, 32);
        if(i > 0)
        {
            dev_com_write(TEST_PORT, tmp, i);
        } 
    }
 #endif
 #if 0
    u8 tmp[2048];
    s32 i;

    for(i=0; i<2048; i++)
    {
        tmp[i] = i&0xff;
    }
    dev_com_write(0, tmp, 2048);
  #endif  
}

