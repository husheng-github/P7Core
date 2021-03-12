/********************************Copyright ( c ) ********************************
**                  DAQU 
**                         
**                        
**
**Created By:     pengxuebin,20170320      
** Created Date:     
** Version:        
** Description:    watchdog�����ӿں���
  
****************************************************************************/
#include "devglobal.h"
#include "drv_watchdog.h"

static s32 g_watchdog_fd = -1;

/****************************************************************************
**Description:       ��WATCHDOG
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170630
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_watchdog_open(void)
{
    if(g_watchdog_fd < 0)
    {
      #if(WATCHDOG_SET_MOD==1)
        drv_watchdog_modconfig(1);   //�ж�ģʽ
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
**Description:       �ر�WATCHDOG
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
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
    return DEVSTATUS_ERR_PARAM_ERR;  //��֧�ֹر�
}

/****************************************************************************
**Description:       ι��
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
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
**Description:       ����watchdogʱ��
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
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
