/********************************Copyright ( c ) ********************************
**                  DAQU 
**                         
**                        
**
**Created By:     pengxuebin,20170320      
** Created Date:     
** Version:        
** Description:    ��ʱ������ӿں���
  
****************************************************************************/
#include "devglobal.h"
#include "drv_timer.h"


/****************************************************************************
**Description:        ��ʱ���豸��ʼ��
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170320 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void dev_timer_init(void)
{
    drv_timer_init();
}
/****************************************************************************
**Description:        ��ʱ���豸��ʼ��
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
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
**Description:        ��ʱ���ͷ�
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170321 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_timer_free(u32 timid)
{
    return drv_timer_free(timid);
}
/****************************************************************************
**Description:        �رն�ʱ��
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170321 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void dev_timer_stop(u32 timid)
{
    drv_timer_stop(timid);
}
/****************************************************************************
**Description:        ������ʱ��
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170321 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void dev_timer_run(u32 timid)
{
    drv_timer_run(timid);
}

/****************************************************************************
**Description:        ���ö�ʱ��ֵ
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170621
**---------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void dev_timer_setvalue(u32 timid, u32 tus)
{
    drv_timer_setvalue(timid, tus);
}
/****************************************************************************
**Description:        ���ö�ʱ��ֵ(��ʱ��CLKΪ��λ)
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170621
**---------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void dev_timer_setvalue_clk(u32 timid, u32 tclk)
{
    drv_timer_setvalue_clk(timid, tclk);
}

/****************************************************************************
**Description:        ʹ���ж�
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170321 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void dev_timer_int_enable(u32 timid)
{
    drv_timer_int_enable(timid);
}
/****************************************************************************
**Description:        �����ж�
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170321 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void dev_timer_int_disable(u32 timid)
{
    drv_timer_int_disable(timid);
}
/****************************************************************************
**Description:        ����жϱ�־
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170711 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void dev_timer_int_clear(u32 timid)
{
    drv_timer_int_clear(timid);
}


