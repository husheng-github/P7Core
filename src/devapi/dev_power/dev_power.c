/********************************Copyright ( c ) ********************************
**                  DAQU 
**                         
**                        
**
**Created By:         
** Created Date:     
** Version:        
** Description:    该文件包含电源管理的驱动相关接口
  
****************************************************************************/
#include "devglobal.h"
#include "drv_power.h"
//static s32 g_power_fd = -1;
/****************************************************************************
**Description:       电源初始化
**Input parameters:    
**Output parameters: 
**Returned value:
**Created by:        pengxuebin,20170606
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void dev_power_init(void)
{
    drv_power_init();
}
/****************************************************************************
**Description:       电源初始化
**Input parameters:    
**Output parameters: 
**Returned value:
**Created by:        pengxuebin,20170606
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void dev_power_switch_ctl(u8 flg)
{
    drv_power_switch_ctl(flg);
}
#if 0
/****************************************************************************
**Description:       SLEEP
**Input parameters:    
                   wakeupmod: 唤醒方式
                   
**Output parameters: 
**Returned value:
**Created by:        pengxuebin,20170629
**---------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void dev_power_wakeupmod(str_wakeup_info_t info)
{
    if((info.m_mod&WAKEUP_MOD_RTC)&&(info.m_rtc_ts!=0))
    {
        dev_rtc_set_alarm(1, info.m_rtc_ts);
    }
    
}
#endif
/****************************************************************************
**Description:       SLEEP
**Input parameters:    
                   mod:     sleep模式
                            1:      sleep模式
                            2:      deep sleep模式
**Output parameters: 
**Returned value:
**Created by:        pengxuebin,20170629
**---------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_power_sleep(u32 mod)
{
    return drv_power_sleep(mod);
}

