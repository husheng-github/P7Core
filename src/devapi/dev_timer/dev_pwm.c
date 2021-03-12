/********************************Copyright ( c ) ********************************
**                  DAQU 
**                         
**                        
**
**Created By:     pengxuebin,20170322      
** Created Date:     
** Version:        
** Description:    定时器对外接口函数
  
****************************************************************************/
#include "devglobal.h"
#include "drv_timer.h"

#define PWMNUM_MAX      6
static s32 g_dev_pwm_fd[PWMNUM_MAX];
static void dev_pwm_init(void)
{
    static u8 flg=1;
    s32 i;
    
    if(flg)
    {
        flg = 0;
        for(i=0; i<PWMNUM_MAX; i++)
        {
            g_dev_pwm_fd[i] = -1;
        }
    }
}
/****************************************************************************
**Description:        打开PWM设备
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170322 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_pwm_open(s32 pwmid, str_pwm_cfg_t pwm_cfg)
{
    s32 ret;
    DRVTIMER_CFG_t l_drvt_cfg;

    dev_pwm_init();
    if(pwmid >= PWMNUM_MAX)
    {
        TIMER_DEBUG("pwmid=%d\r\n", pwmid);
        return -1;
    }   
    if(g_dev_pwm_fd[pwmid]<0)
    {
        drv_pwm_pincfg(pwmid, 0);
        
        l_drvt_cfg.m_mode = TIMER_MODE_PWM;
        l_drvt_cfg.m_pwmhz = pwm_cfg.m_pwmhz;
        l_drvt_cfg.m_LowLevelPeriod = pwm_cfg.m_LowPeriod;
        l_drvt_cfg.m_HighLevelPeriod = pwm_cfg.m_HighPeriod;
        ret = drv_timer_request(pwmid, l_drvt_cfg, NULL, NULL);
        if(ret>=0)
        {
            //drv_timer_run(pwmid);
            g_dev_pwm_fd[pwmid] = 0;
        }
        return ret;
    }
    return g_dev_pwm_fd[pwmid];
    
}
/****************************************************************************
**Description:        关闭PWM设备
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170322 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_pwm_close(s32 pwmid)
{
    if(pwmid >= PWMNUM_MAX)
    {
        return -1;
    }

    if(g_dev_pwm_fd[pwmid]>=0)
    {
        drv_pwm_pincfg(pwmid, 0);
        drv_timer_free(pwmid);
        g_dev_pwm_fd[pwmid] = -1;
    }
    return 0;
}
/****************************************************************************
**Description:        PWM参数设置
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170322 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_pwm_set(s32 pwmid, const str_pwm_cfg_t pwm_cfg)
{
    DRVTIMER_CFG_t l_drvt_cfg;
    
    if(pwmid >= PWMNUM_MAX)
    {
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    if(g_dev_pwm_fd[pwmid] < 0)
    {
        return DEVSTATUS_ERR_DEVICE_NOTOPEN;
    }

    drv_timer_stop(pwmid);
    l_drvt_cfg.m_mode = TIMER_MODE_PWM;
    l_drvt_cfg.m_pwmhz = pwm_cfg.m_pwmhz;
    l_drvt_cfg.m_LowLevelPeriod = pwm_cfg.m_LowPeriod;
    l_drvt_cfg.m_HighLevelPeriod = pwm_cfg.m_HighPeriod;
        
    drv_timer_pwmset(pwmid, l_drvt_cfg);
    return 0;
}
/****************************************************************************
**Description:        PWM运行
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170322 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_pwm_run(s32 pwmid)
{
    if(pwmid >= PWMNUM_MAX)
    {
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    if(g_dev_pwm_fd[pwmid] < 0)
    {
        return DEVSTATUS_ERR_DEVICE_NOTOPEN;
    }

    drv_pwm_pincfg(pwmid, 1);
    drv_timer_run(pwmid);
    return 0;
}
/****************************************************************************
**Description:        PWM停止
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170322 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_pwm_stop(s32 pwmid)
{
    if(pwmid >= PWMNUM_MAX)
    {
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    if(g_dev_pwm_fd[pwmid] < 0)
    {
        return DEVSTATUS_ERR_DEVICE_NOTOPEN;
    }
    
    drv_timer_stop(pwmid);
    drv_pwm_pincfg(pwmid, 0);
    return 0;
}
