/********************************Copyright ( c ) ********************************
**                  DAQU 
**                         
**                        
**
**Created By:     pengxuebin,20170322      
** Created Date:     
** Version:        
** Description:    SPI�ӿڳ���
****************************************************************************/
#include "devglobal.h"
#include "sdk/mhscpu_sdk.h"
#include "drv_spi.h"

/****************************************************************************
**Description:        spi��ʼ��
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170324
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void dev_spi_init(void)
{
    drv_spi_init();
}
/****************************************************************************
**Description:            ��spi�豸
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170324
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_spi_open(SPI_DEVICE_t spidev)
{
    return drv_spi_open(spidev);
}
/****************************************************************************
**Description:        �ر�spi�豸
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170324
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_spi_close(SPI_DEVICE_t spidev)
{
    return drv_spi_close(spidev);
}
/****************************************************************************
**Description:         spi����ʽ����(��ѯ��ʽ)
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
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
**Description:         spi�ӷ�ʽ��������
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20180827
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_spi_slave_write(u8 *wbuf, u32 wlen)
{
    return drv_spi_slave_write(wbuf, wlen);
}
/****************************************************************************
**Description:         spi�ӷ�ʽ��������
**Input parameters:    
**Output parameters: 
**Returned value:
                    >=:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
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
            //��ʱ
            return -1;
        }
    }
}
#endif
