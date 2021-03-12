/********************************Copyright ( c ) ********************************
**                  DAQU 
**                         
**                        
**
**Created By:     pengxuebin,20170428      
** Created Date:     
** Version:        
** Description:    AUDIO��������  
****************************************************************************/
#include "devglobal.h"
#include "sdk/mhscpu_sdk.h"
#include "drv_audio.h"

static s32 g_drv_audio_fd = -1;

typedef struct _strBeepCtl
{
    volatile u8 m_runflg;
    u8  m_status;   //��¼��ǰ״̬, 0 ������  1 ����
    u8  m_times;    //�صƴ���,��˸�ɷ�����ʼ,��������
    u16 m_ontime;   //����ʱ��
    u16 m_offtime;  //�ر�ʱ��
    u32 m_timeid;   //��¼ID
}strBeepCtl_t;
static strBeepCtl_t g_beepctl;
static u32 g_pwmhz;
/****************************************************************************
**Description:        ��audio�豸
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170429 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_audio_open(void)
{
    str_pwm_cfg_t pwm_cfg;
    s32 ret;
    
    if(g_drv_audio_fd < 0)
    {
        g_beepctl.m_runflg = 0;
        
        
      #if(MACHINETYPE == MACHINE_S1)
        g_pwmhz = BEEP_PWM_HZ_Default;
      #else //M3��M5
        
        g_pwmhz = BEEP_PWM_HZ_Default;
        
      #endif
        pwm_cfg.m_pwmhz = g_pwmhz;
        pwm_cfg.m_LowPeriod = BEEP_LowLevelPeriod;
        pwm_cfg.m_HighPeriod = BEEP_HighLevelPeriod;
        
        ret = dev_pwm_open(BEEP_PWM_CH, pwm_cfg);
        if(ret < 0)
        {
            AUDIO_DEBUG("ret=%d\r\n", ret);
            return DEVSTATUS_ERR_FAIL;
        }
        g_drv_audio_fd = 0;
    }
    return DEVSTATUS_SUCCESS;
}
/****************************************************************************
**Description:        �ر�audio�豸
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170429 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_audio_close(void)
{
    if(g_drv_audio_fd>=0)
    {
        dev_pwm_stop(BEEP_PWM_CH);
        dev_pwm_close(BEEP_PWM_CH);
        g_drv_audio_fd = -1;
        g_beepctl.m_runflg = 0;
    }
    return 0;
}
/****************************************************************************
**Description:        ��������������
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
**Created by:        pengxuebin,20170429 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
static s32 drv_audio_beep_ctl(u8 mode)
{
    if(mode)
    {
        dev_pwm_run(BEEP_PWM_CH);
    }
    else
    {
        dev_pwm_stop(BEEP_PWM_CH);
    }
    g_beepctl.m_status = mode;
    return DEVSTATUS_SUCCESS;
}
/****************************************************************************
**Description:        ������������
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
**Created by:        pengxuebin,20170429 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void drv_audio_task(void)
{
    u32 timeid;
    u32 tmp;

    if(g_drv_audio_fd < 0)
    {
        return;
    }
    
    if(g_beepctl.m_runflg)
    {
        timeid = dev_user_gettimeID();
        tmp = timeid-g_beepctl.m_timeid;
        if(g_beepctl.m_status == 0)
        {
            if(tmp >= g_beepctl.m_offtime)
            {
                //����ʱ�䵽����Ҫ����
                drv_audio_beep_ctl(1);
                g_beepctl.m_timeid = timeid;
            }
        }
        else
        {
            if(tmp >= g_beepctl.m_ontime)
            {
                //����ʱ�䵽���農��
                drv_audio_beep_ctl(0);
                g_beepctl.m_timeid = timeid;
                if(g_beepctl.m_times > 0)
                {
                    g_beepctl.m_times--;
                }
                if(g_beepctl.m_times == 0)
                {
                    g_beepctl.m_runflg = 0;
                }
            }
        }
    }
}
/****************************************************************************
**Description:        ����������
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170429 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_audio_beep(strbeepParam_t l_beepparam)
{ 
    str_pwm_cfg_t l_pwm_cfg;

    if(g_drv_audio_fd < 0)
    {
        AUDIO_DEBUG("DEVICE_NOTOPEN!\r\n");
        return DEVSTATUS_ERR_DEVICE_NOTOPEN;
    }
    g_beepctl.m_runflg = 0;
    drv_audio_beep_ctl(0);   
    if(l_beepparam.m_times != 0)
    {
        l_pwm_cfg.m_pwmhz = l_beepparam.m_pwmhz;
        l_pwm_cfg.m_LowPeriod = 1;
        l_pwm_cfg.m_HighPeriod = 1;
        dev_pwm_set(BEEP_PWM_CH, l_pwm_cfg);
        g_beepctl.m_times = l_beepparam.m_times;
        g_beepctl.m_ontime = l_beepparam.m_ontime;
        g_beepctl.m_offtime = l_beepparam.m_offtime;
        g_beepctl.m_timeid = dev_user_gettimeID();
        drv_audio_beep_ctl(1);
        g_beepctl.m_runflg = 1;
    }
    return 0;
}
/****************************************************************************
**Description:        ��ͣ
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170808
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_audio_suspend(void)
{
    
    if(g_drv_audio_fd < 0)
    {
        return DDI_EIO;
    }
    dev_pwm_stop(BEEP_PWM_CH);
    dev_pwm_close(BEEP_PWM_CH);
    return DDI_OK;
}
/****************************************************************************
**Description:        ����
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170808
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_audio_resume(void)
{    
    str_pwm_cfg_t pwm_cfg;
    
    if(g_drv_audio_fd < 0)
    {
        return DDI_EIO;
    }
    
    g_beepctl.m_runflg = 0;
    pwm_cfg.m_pwmhz = g_pwmhz;//BEEP_PWM_HZ_Default;
    pwm_cfg.m_LowPeriod = BEEP_LowLevelPeriod;
    pwm_cfg.m_HighPeriod = BEEP_HighLevelPeriod;
    dev_pwm_open(BEEP_PWM_CH, pwm_cfg);
    return DDI_OK;
}
void drv_audio_set_pwm(u32 pwmhz)
{
    g_pwmhz = pwmhz;
}
u32 drv_audio_get_pwm(void)
{
    return g_pwmhz;
}