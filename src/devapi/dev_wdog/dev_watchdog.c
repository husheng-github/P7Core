/********************************Copyright ( c ) ********************************
**                  DAQU 
**                         
**                        
**
**Created By:     pengxuebin,20170320      
** Created Date:     
** Version:        
** Description:    watchdog驱动接口函数
  
****************************************************************************/
#include "devglobal.h"
#include "drv_watchdog.h"

static s32 g_watchdog_fd = -1;

/****************************************************************************
**Description:       打开WATCHDOG
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170630
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_watchdog_open(void)
{
    if(g_watchdog_fd < 0)
    {
      #if(WATCHDOG_SET_MOD==1)
        drv_watchdog_modconfig(1);   //中断模式
      #else
        drv_watchdog_modconfig(0);
      #endif
        drv_watchdog_set_reload(WATCHDOG_TIME_DEFAULT);
        drv_watchdog_feed();
        drv_watchdog_enable();
        g_watchdog_fd = 0;
    }
    return g_watchdog_fd;
}

/****************************************************************************
**Description:       关闭WATCHDOG
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170630
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_watchdog_close(void)
{
   #if 0 
    if(g_watchdog_fd == 0)
    {
        drv_watchdog_enable(0);
        g_watchdog_fd = -1;
    }
  #endif  
    return DEVSTATUS_ERR_PARAM_ERR;  //不支持关闭
}

/****************************************************************************
**Description:       喂狗
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170630
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_watchdog_feed(void)
{
    drv_watchdog_feed();
    return 0;
}
/****************************************************************************
**Description:       设置watchdog时间
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170630
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_watchdog_set_time(u32 ts)
{
    if(ts > WATCHDOG_TIME_MAX)
    {
        ts = WATCHDOG_TIME_MAX;
    }
    drv_watchdog_set_reload(ts);
}
