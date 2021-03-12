/********************************Copyright ( c ) ********************************
**                  DAQU 
**                         
**                        
**
**Created By:         
** Created Date:     
** Version:        
** Description:    ���ļ���չGPIO���ƺ���
  
****************************************************************************/
#include "devglobal.h"
#include "sdk/mhscpu_sdk.h"
#if 0
static s32 g_gpioex_fd = -1;
static u32 g_gpioex_status = 0;     
/****************************************************************************
**Description:        595��������
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:         
**---------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
static s32 dev_gpioex_senddata(u8 data)
{
    u8 snd[2];
    s32 ret;    
    
    snd[0] = data;
    
    ret = dev_spi_master_transceive_polling(SPI_DEV_595, &snd, NULL, 1);
    //MISC_DEBUG("status=%02X, ret=%d\r\n", data, ret);
    if(ret < 0)
    {
        return DEVSTATUS_ERR_FAIL;
    }
    else
    {
        return DEVSTATUS_SUCCESS;
    }
}
/****************************************************************************
**Description:        ��չGPIO��ʼ��
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:         
**---------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_gpioex_init(void)
{
    s32 ret;
    u8 machineid; 

    machineid = dev_misc_getmachinetypeid();
    if(machineid == MACHINE_TYPE_ID_T1)
    {
        if(g_gpioex_fd<0)
        {
            ret = dev_spi_open(SPI_DEV_595);
            if(ret >= 0)
            {
                g_gpioex_status = 0x02; //EX1 = 1;
                ret = dev_gpioex_senddata(g_gpioex_status);
                //MISC_DEBUG("ret=%d\r\n", ret);            
                if(ret>=0)
                { 
                    g_gpioex_fd = 0;
                }
            }
        }
        return g_gpioex_fd;
    }
    else
    {
        return DEVSTATUS_ERR_FAIL;
    }  
}
/****************************************************************************
**Description:        ��չGPIO��ʼ��
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_FAIL:  ��������
**Created by:         
**---------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_gpioex_set_value(u32 pin, u32 value)
{
    if(dev_gpioex_init()<0)
    {
        return DEVSTATUS_ERR_FAIL;
    }
    if(value)
    {
        g_gpioex_status |= (1<<pin);
    }
    else
    {
        g_gpioex_status &= (~(1<<pin));
    }    
    return dev_gpioex_senddata(g_gpioex_status&0xFF);
}
#endif
