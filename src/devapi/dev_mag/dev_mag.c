/********************************Copyright ( c ) ********************************
**                  DAQU 
**                         
**                        
**
**Created By:         
** Created Date:     
** Version:        
** Description:    ���ļ������ſ���������ؽӿ�
  
****************************************************************************/
#include "devglobal.h"
#include "sdk/mhscpu_sdk.h"
#include "drv_mag.h"

#if(MAG_EXIST==1)
/**
 * @brief �ſ���ʼ��
 * @param [in/out] ��
 * @retval ��
 * @since pengxuebin 20190808
 */
void dev_mag_init(void)
{
    drv_mag_init();
}
/****************************************************************************
**Description:       ��MAG�豸
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170424
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_mag_open(void)
{
    return drv_mag_open();
}
/****************************************************************************
**Description:       �ر�MAG�豸
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170424
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_mag_close(void)
{
    return drv_mag_close();
}

/****************************************************************************
**Description:       ���ſ�����
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_DEVICE_NOTOPEN:
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
                    DEVSTATUS_ERR_FAIL: ʧ��
**Created by:        pengxuebin,20170424
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_mag_read(u8 *tk1, u8 *tk2, u8 *tk3, u8 *trackstatus)
{
    s32 ret;
    s32 status;
    
    ret = drv_mag_read(tk1, tk2, tk3);
    status = drv_mag_ioctl_getmagswipestatus();
    if(trackstatus != NULL)
    {
        trackstatus[0] = status &0xff;
        trackstatus[1] = (status>>8) &0xff;
        trackstatus[2] = (status>>16) &0xff;
    }
    return ret;    
}
/****************************************************************************
**Description:       ����ſ�����
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_DEVICE_NOTOPEN:
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
                    DEVSTATUS_ERR_FAIL: ʧ��
**Created by:        pengxuebin,20170424
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_mag_clear(void)
{
    return drv_mag_clear();
}



s32 dev_mag_ioctl_getmagswipestatus(void)
{
    return drv_mag_ioctl_getmagswipestatus();
}
s32 dev_mag_get_ver(u8 *ver)
{
    return drv_mag_get_ver(ver);
}

#else
s32 dev_mag_open(void)
{
    return DEVSTATUS_ERR_FAIL;
}
s32 dev_mag_close(void)
{
    return DEVSTATUS_ERR_FAIL;
}
s32 dev_mag_read(u8 *tk1, u8 *tk2, u8 *tk3, u8 *trackstatus)
{
    return DEVSTATUS_ERR_FAIL;
}
s32 dev_mag_clear(void)
{
    return DEVSTATUS_ERR_FAIL;
}
s32 dev_mag_ioctl_getmagswipestatus(void)
{
	return DEVSTATUS_ERR_FAIL;
}
s32 dev_mag_get_ver(u8 *ver)
{
    return DEVSTATUS_ERR_FAIL;
}
#endif

