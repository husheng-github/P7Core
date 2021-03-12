/********************************Copyright ( c ) ********************************
**                  DAQU 
**                         
**                        
**
**Created By:     pengxuebin,20170322      
** Created Date:     
** Version:        
** Description:    SPI接口程序
****************************************************************************/
#include "devglobal.h"
#include "sdk/mhscpu_sdk.h"
#include "drv_spi.h"

/****************************************************************************
**Description:        spi初始化
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170324
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void dev_spi_init(void)
{
    drv_spi_init();
}
/****************************************************************************
**Description:            打开spi设备
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170324
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_spi_open(SPI_DEVICE_t spidev)
{
    return drv_spi_open(spidev);
}
/****************************************************************************
**Description:        关闭spi设备
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170324
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_spi_close(SPI_DEVICE_t spidev)
{
    return drv_spi_close(spidev);
}
/****************************************************************************
**Description:         spi主方式传输(查询方式)
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170324
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_spi_master_transceive_polling(SPI_DEVICE_t spidev, void* txbuf, void* rxbuf, s32 le)
{
    return drv_spi_master_transceive_polling(spidev, txbuf, rxbuf, le);
}
#if(SPI_SLAVE_EN==1)
/****************************************************************************
**Description:         spi从方式发送数据
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20180827
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_spi_slave_write(u8 *wbuf, u32 wlen)
{
    return drv_spi_slave_write(wbuf, wlen);
}
/****************************************************************************
**Description:         spi从方式接收数据
**Input parameters:    
**Output parameters: 
**Returned value:
                    >=:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20180827
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_spi_slave_read(u8 *rbuf, u32 rlen)
{
    return drv_spi_slave_read(rbuf, rlen);
}

s32 dev_spi_slave_tcdrain(void)
{
    u32 timeid;

    timeid = dev_user_gettimeID();
    while(1)
    {
        if(0 == drv_spi_slave_check_txbusying())
        {
            return 0;
        }
        if(dev_user_querrytimer(timeid, 500))
        {
            //超时
            return -1;
        }
    }
}
#endif
