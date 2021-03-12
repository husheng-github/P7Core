/*****************************************************************************
**                  DAQU 
**                         
**                        
**
**Created By:     pengxuebin,20170422      
** Created Date:     
** Version:        
** Description:    ADC驱动程序  
****************************************************************************/
#include "devglobal.h"
#include "sdk/mhscpu_sdk.h"
#include "drv_dac.h"

/****************************************************************************
**Description:       打开DAC
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170422
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_dac_open(void)
{
    return drv_dac_open();
}
/****************************************************************************
**Description:       关闭DAC设备
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170422
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_dac_close(void)
{
    return drv_dac_close();
}

/****************************************************************************
**Description:       DAC tts播放
**Input parameters:    
**Output parameters: 
**Returned value:
                    >=0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
                    DEVSTATUS_ERR_FAIL:  失败
                    
**Created by:        pengxuebin,20170422
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/

s32 dev_dac_tts_play(strttsData_t *ttsdata, s32 ttslen)
{
    return drv_dac_tts_play(ttsdata,ttslen);
}

/****************************************************************************
**Description:       DAC 播放
**Input parameters:    
**Output parameters: 
**Returned value:
                    >=0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
                    DEVSTATUS_ERR_FAIL:  失败
                    
**Created by:        pengxuebin,20170422
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_dac_play(u32 waveaddr)
{
    return drv_dac_play(waveaddr);
}

/****************************************************************************
**Description:       DAC 暂停
**Input parameters:    
**Output parameters: 
**Returned value:
                    >=0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
                    DEVSTATUS_ERR_FAIL:  失败
                    
**Created by:        pengxuebin,20170422
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/

s32 dev_dac_pause(void)
{
    return drv_dac_pause();
}

/****************************************************************************
**Description:       DAC 恢复
**Input parameters:    
**Output parameters: 
**Returned value:
                    >=0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
                    DEVSTATUS_ERR_FAIL:  失败
                    
**Created by:        pengxuebin,20170422
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/

s32 dev_dac_replay(void)
{
    return drv_dac_replay();
}

/****************************************************************************
**Description:       DAC 设置音量等级
**Input parameters:    
**Output parameters: 
**Returned value:
                    >=0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
                    DEVSTATUS_ERR_FAIL:  失败
                    
**Created by:        pengxuebin,20170422
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/

s32 dev_dac_set_volume_level(int level)
{
    return drv_dac_set_volume_level(level);
}

/****************************************************************************
**Description:       DAC 获取音量等级
**Input parameters:    
**Output parameters: 
**Returned value:
                    >=0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
                    DEVSTATUS_ERR_FAIL:  失败
                    
**Created by:        pengxuebin,20170422
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/

s32 dev_dac_get_volume_level(void)
{
    return drv_dac_get_volume_level();
}

/****************************************************************************
**Description:       DAC 获取播放状态
**Input parameters:    
**Output parameters: 
**Returned value:
                    >=0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
                    DEVSTATUS_ERR_FAIL:  失败
                    
**Created by:        pengxuebin,20170422
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/

s32 dev_dac_is_playing(void)
{
    return drv_dac_is_playing();
}


