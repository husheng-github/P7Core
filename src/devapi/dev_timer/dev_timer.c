/********************************Copyright ( c ) ********************************
**                  DAQU 
**                         
**                        
**
**Created By:     pengxuebin,20170320      
** Created Date:     
** Version:        
** Description:    定时器对外接口函数
  
****************************************************************************/
#include "devglobal.h"
#include "drv_timer.h"


/****************************************************************************
**Description:        定时器设备初始化
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170320 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void dev_timer_init(void)
{
    drv_timer_init();
}
/****************************************************************************
**Description:        定时器设备初始化
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170320 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_timer_request(s32 timid, const str_timer_cfg_t t_cfg, void(*pFun)(void*), void *data)
{ 
    DRVTIMER_CFG_t l_drvt_cfg;

    l_drvt_cfg.m_mode = TIMER_MODE_CNT;
    l_drvt_cfg.m_reload = t_cfg.m_reload;    //
    l_drvt_cfg.m_tus = t_cfg.m_tus;
    return drv_timer_request(timid, l_drvt_cfg, pFun, data);
}
/****************************************************************************
**Description:        定时器释放
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170321 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_timer_free(u32 timid)
{
    return drv_timer_free(timid);
}
/****************************************************************************
**Description:        关闭定时器
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170321 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void dev_timer_stop(u32 timid)
{
    drv_timer_stop(timid);
}
/****************************************************************************
**Description:        启动定时器
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170321 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void dev_timer_run(u32 timid)
{
    drv_timer_run(timid);
}

/****************************************************************************
**Description:        设置定时器值
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170621
**---------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void dev_timer_setvalue(u32 timid, u32 tus)
{
    drv_timer_setvalue(timid, tus);
}
/****************************************************************************
**Description:        设置定时器值(以时钟CLK为单位)
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170621
**---------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void dev_timer_setvalue_clk(u32 timid, u32 tclk)
{
    drv_timer_setvalue_clk(timid, tclk);
}

/****************************************************************************
**Description:        使能中断
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170321 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void dev_timer_int_enable(u32 timid)
{
    drv_timer_int_enable(timid);
}
/****************************************************************************
**Description:        禁能中断
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170321 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void dev_timer_int_disable(u32 timid)
{
    drv_timer_int_disable(timid);
}
/****************************************************************************
**Description:        清除中断标志
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170711 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void dev_timer_int_clear(u32 timid)
{
    drv_timer_int_clear(timid);
}


