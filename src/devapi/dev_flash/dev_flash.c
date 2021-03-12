/********************************Copyright ( c ) ********************************
**                  DAQU 
**                         
**                        
**
**Created By:     pengxuebin,20170405     
** Created Date:     
** Version:        
** Description:    该文件处理内部、外部FLASH对外接口函数
  
****************************************************************************/
#include "devglobal.h"
#include "sdk/mhscpu_sdk.h"
#include "drv_spiflash.h"
#include "drv_internalflash.h"
#include "drv_qspiflash.h"

/****************************************************************************
**Description:        flash初始化
**Input parameters:    
**Output parameters: 
**Returned value:
                    <0:失败
                    0:成功
**Created by:        pengxuebin,20170405 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_flash_init(void)
{
    s32 ret = DEVSTATUS_SUCCESS;
    
    //drv_internalflash_open();
  #if(QSPIFLASH_EXIST==1)
    ret = drv_qspiflash_open();
  #endif
    
    #if(SPIFLASH_EXIST==1)
    ret = drv_spiflash_open();  
    #endif
  
    return ret;
}
/****************************************************************************
**Description:        读flash
**Input parameters:    
**Output parameters: 
**Returned value:
                    <0:失败
                    >=0:成功,读取数据个数
**Created by:        pengxuebin,20170405 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_flash_read(u32 addrsta, u8* rbuf, u32 rlen)
{
    s32 ret;

    if(addrsta >= SPIFLASH_ADDRSTA)
    {
        #if(SPIFLASH_EXIST==1)
        ret = drv_spiflash_read(addrsta-SPIFLASH_ADDRSTA, rbuf, rlen);
        #else
        ret = DEVSTATUS_ERR_PARAM_ERR;
        #endif
    }
    else
    {
        //ret = drv_internalflash_read(addrsta,rbuf,rlen);
      #if(QSPIFLASH_EXIST==1)
        ret = drv_qspiflash_read(addrsta,rbuf,rlen);
      #endif
    }

    return ret;
}


/****************************************************************************
**Description:        写flash
**Input parameters:    
**Output parameters: 
**Returned value:
                    <0:失败
                    >=0:成功,写入数据个数
**Created by:        pengxuebin,20170410 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_flash_write(u32 addrsta, u8 *wbuf, u32 wlen)
{
    s32 ret;

    if(addrsta >= SPIFLASH_ADDRSTA)
    {
        #if(SPIFLASH_EXIST==1)
        ret = drv_spiflash_writewitherase(addrsta-SPIFLASH_ADDRSTA, wbuf, wlen);
        #else
        ret = DEVSTATUS_ERR_PARAM_ERR;
        #endif
    }
    else
    {
        //ret = drv_internalflash_write(addrsta, wbuf, wlen);
      #if(QSPIFLASH_EXIST==1)
        ret = drv_qspiflash_write(addrsta, wbuf, wlen);
      #endif
    }
    
    return ret;
}
/****************************************************************************
**Description:        擦除FLASH块
**Input parameters:    
**Output parameters: 
**Returned value:
                    <0:失败
                    =0:成功
**Created by:        pengxuebin,20170410 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_flash_erasesector(u32 addrsta)
{
    s32 ret;

    if(addrsta >= SPIFLASH_ADDRSTA)
    {
        #if(SPIFLASH_EXIST==1)
        ret = drv_spiflash_erase(addrsta-SPIFLASH_ADDRSTA);
        #else
        ret = DEVSTATUS_ERR_PARAM_ERR;
        #endif
    }
    else
    {
        //ret = drv_internalflash_erasesector(addrsta);
      #if(QSPIFLASH_EXIST==1)
        ret = drv_qspiflash_erasesector(addrsta);
      #endif
    }
    return ret;
}
/****************************************************************************
**Description:        带擦除的写
**Input parameters:    
**Output parameters: 
**Returned value:
                    <0:失败
                    =0:成功
**Created by:        pengxuebin,20170411 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_flash_writewitherase(u32 addrsta, u8 *wbuf, u32 wlen)
{
    s32 ret;

    if(addrsta >= SPIFLASH_ADDRSTA)
    {
        #if(SPIFLASH_EXIST==1)
        ret = drv_spiflash_writewitherase(addrsta-SPIFLASH_ADDRSTA, wbuf, wlen);
        #else
        ret = DEVSTATUS_ERR_PARAM_ERR;
        #endif
    }
    else
    {
        //ret = drv_internalflash_writespec(addrsta, wbuf, wlen);
      #if(QSPIFLASH_EXIST==1)
        ret = drv_qspiflash_writespec(addrsta, wbuf, wlen);
      #endif
    }
    
    return ret;
}


#if 0//def TRENDIT_BOOT
#define SPIFLASH_TEST_MAX    1024
s32 drv_spiflash_check_busy(void);
void dev_flash_test(u32 addrsta)
{
    
    u8 flg;
    u32 key;
    s32 ret;
    u8 tmp[SPIFLASH_TEST_MAX];
    u32 rlen;
    s32 i;
    u32 timerid;

    flg = 1;
    while(1)
    {
        if(flg == 1)
        {
            flg = 0;
            lcd_interface_clear_ram();
            lcd_interface_fill_rowram(0, 0, "FLASH TEST", NOFDISP|CDISP);
            lcd_interface_fill_rowram(1, 0, "1.Read flash", FDISP);
            lcd_interface_fill_rowram(2, 0, "2.write1", FDISP);
            lcd_interface_fill_rowram(3, 0, "3.write2", FDISP);
            lcd_interface_fill_rowram(4, 0, "4.write3", FDISP);
            lcd_interface_brush_screen();
            keypad_interface_clear();
            timerid = dev_user_gettimeID();
        }
        if(dev_user_querrytimer(timerid, 1000))
        {
            timerid = dev_user_gettimeID();
            dev_debug_printf("%s(%d):Loop\r\n", __FUNCTION__, __LINE__);
        }
        if(keypad_interface_read_beep(&key)>0)
        {
            switch(key)
            {
            case DIGITAL1:
                memset(tmp, 0, SPIFLASH_TEST_MAX);
                ret = dev_flash_read(addrsta, tmp, SPIFLASH_TEST_MAX);
dev_debug_printf("%s(%d):ret=%d\r\n",__FUNCTION__, __LINE__, ret); 
                if(ret > 0)
                {
                    dev_debug_printformat(NULL, tmp, ret);
                }
                break;
                
            case DIGITAL2:
                memset(tmp, 0, SPIFLASH_TEST_MAX);
                for(i=0; i<SPIFLASH_TEST_MAX; i++)
                {
                    tmp[i] = i;
                }
                ret = dev_flash_write(addrsta, tmp, SPIFLASH_TEST_MAX);
dev_debug_printf("%s(%d):ret = %d\r\n",__FUNCTION__, __LINE__, ret);
            
                break;
                
            case DIGITAL3:
                memset(tmp, 0, SPIFLASH_TEST_MAX);
                for(i=0; i<SPIFLASH_TEST_MAX; i++)
                {
                    tmp[i] = (i&0x0f)+0x30;
                }
                ret = dev_flash_write(addrsta, tmp, SPIFLASH_TEST_MAX);
dev_debug_printf("%s(%d):ret = %d\r\n",__FUNCTION__, __LINE__, ret);
                break;
                
            case DIGITAL4: 
                memset(tmp, 0, SPIFLASH_TEST_MAX);
                for(i=0; i<SPIFLASH_TEST_MAX; i++)
                {
                    tmp[i] = (i&0x0f)+0x40;
                }
                ret = dev_flash_write(addrsta, tmp, SPIFLASH_TEST_MAX);
dev_debug_printf("%s(%d):ret = %d\r\n",__FUNCTION__, __LINE__, ret);
                break;
            case DIGITAL5: 
                ret = 0;//drv_spiflash_check_busy();
dev_debug_printf("%s(%d):ret=%d\r\n",__FUNCTION__, __LINE__, ret);
                break;
            case DIGITAL6: 
//dev_debug_printf("%s(%d):\r\n",__FUNCTION__, __LINE__);
                //dev_com_test();
 //               ret = ddi_misc_bat_status();
dev_debug_printf("%s(%d):ret=%08X\r\n",__FUNCTION__, __LINE__, ret);
                break;
            case ESC:
                flg = 1;
                return;
                break;
            }
        }
    }
}
#endif