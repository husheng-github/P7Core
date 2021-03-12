/*****************************************************************************
**                  DAQU 
**                         
**                        
**
**Created By:     pengxuebin,20170422      
** Created Date:     
** Version:        
** Description:    ADC��������  
****************************************************************************/
#include "devglobal.h"
#include "sdk/mhscpu_sdk.h"
#include "drv_dac.h"

/****************************************************************************
**Description:       ��DAC
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170422
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_dac_open(void)
{
    return drv_dac_open();
}
/****************************************************************************
**Description:       �ر�DAC�豸
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170422
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_dac_close(void)
{
    return drv_dac_close();
}

/****************************************************************************
**Description:       DAC tts����
**Input parameters:    
**Output parameters: 
**Returned value:
                    >=0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
                    DEVSTATUS_ERR_FAIL:  ʧ��
                    
**Created by:        pengxuebin,20170422
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/

s32 dev_dac_tts_play(strttsData_t *ttsdata, s32 ttslen)
{
    return drv_dac_tts_play(ttsdata,ttslen);
}

/****************************************************************************
**Description:       DAC ����
**Input parameters:    
**Output parameters: 
**Returned value:
                    >=0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
                    DEVSTATUS_ERR_FAIL:  ʧ��
                    
**Created by:        pengxuebin,20170422
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_dac_play(u32 waveaddr)
{
    return drv_dac_play(waveaddr);
}

/****************************************************************************
**Description:       DAC ��ͣ
**Input parameters:    
**Output parameters: 
**Returned value:
                    >=0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
                    DEVSTATUS_ERR_FAIL:  ʧ��
                    
**Created by:        pengxuebin,20170422
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/

s32 dev_dac_pause(void)
{
    return drv_dac_pause();
}

/****************************************************************************
**Description:       DAC �ָ�
**Input parameters:    
**Output parameters: 
**Returned value:
                    >=0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
                    DEVSTATUS_ERR_FAIL:  ʧ��
                    
**Created by:        pengxuebin,20170422
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/

s32 dev_dac_replay(void)
{
    return drv_dac_replay();
}

/****************************************************************************
**Description:       DAC ���������ȼ�
**Input parameters:    
**Output parameters: 
**Returned value:
                    >=0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
                    DEVSTATUS_ERR_FAIL:  ʧ��
                    
**Created by:        pengxuebin,20170422
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/

s32 dev_dac_set_volume_level(int level)
{
    return drv_dac_set_volume_level(level);
}

/****************************************************************************
**Description:       DAC ��ȡ�����ȼ�
**Input parameters:    
**Output parameters: 
**Returned value:
                    >=0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
                    DEVSTATUS_ERR_FAIL:  ʧ��
                    
**Created by:        pengxuebin,20170422
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/

s32 dev_dac_get_volume_level(void)
{
    return drv_dac_get_volume_level();
}

/****************************************************************************
**Description:       DAC ��ȡ����״̬
**Input parameters:    
**Output parameters: 
**Returned value:
                    >=0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
                    DEVSTATUS_ERR_FAIL:  ʧ��
                    
**Created by:        pengxuebin,20170422
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/

s32 dev_dac_is_playing(void)
{
    return drv_dac_is_playing();
}


