/********************************Copyright ( c ) ********************************
**                  DAQU 
**                         
**                        
**
**Created By:         
** Created Date:     
** Version:        
** Description:    ���ļ�����IC��
  
****************************************************************************/
#include "devglobal.h"
#include "drv_icc.h"
static s32 g_icc_fd[ICC_SLOT_MAX]={-1,-1, -1};
static u8 g_psam_existflg=0;        //����PSAM�����ڱ�־
/****************************************************************************
**Description:       ��ICC�豸
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170425
**---------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void dev_icc_init(void)
{
    u8 machineid;

    machineid = dev_misc_getmachinetypeid();
    if(machineid == MACHINE_TYPE_ID_T1)
    {
        g_psam_existflg = 1;
    }
    else
    {
        g_psam_existflg = 0;
    }
    //
    drv_icc_init();
    //��ʼ��PSAM��
    if(g_psam_existflg)
    {
        drv_psam_init();
    }
}
/****************************************************************************
**Description:       ��ICC�豸
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170425
**---------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_icc_open(s32 nslot)
{
    s32 ret = DEVSTATUS_ERR_PARAM_ERR;
    
    if(nslot >= ICC_SLOT_MAX)
    {
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    if(g_icc_fd[nslot]<0)
    {
        switch(nslot)
        {
            case ICC_SLOT_ICCARD:  
                ret = drv_icc_open();
                break;
            case ICC_SLOT_PSAM1:
            case ICC_SLOT_PSAM2:
                if(g_psam_existflg)
                {
                    ret = drv_psam_open(nslot-1);
                }
                break;
        }
        if(ret==0)
        {
            g_icc_fd[nslot] = 0;
        }
    }
    else
    {
        return 0;
    }
    return ret;
}
/****************************************************************************
**Description:       �ر�ICC�豸
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
**Created by:        pengxuebin,20170425
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_icc_close(s32 nslot)
{
    s32 ret = DEVSTATUS_ERR_PARAM_ERR;
    
    if(nslot >= ICC_SLOT_MAX)
    {
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    if(g_icc_fd[nslot]==0)
    {
        switch(nslot)
        {
        case ICC_SLOT_ICCARD:  
            ret = drv_icc_close();
            break;
        case ICC_SLOT_PSAM1:
        case ICC_SLOT_PSAM2:
            if(g_psam_existflg)
            {
                ret = drv_psam_close(nslot-1);
            }
            break;
        }
        g_icc_fd[nslot] = -1;
    }
    return ret;
}
/****************************************************************************
**Description:       �µ�
**Input parameters:    
**Output parameters: 
**Returned value:
                    
**Created by:        pengxuebin,20170425
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_icc_poweroff(s32 nslot)
{
    s32 ret = DEVSTATUS_ERR_PARAM_ERR;
    
    if(nslot >= ICC_SLOT_MAX)
    {
        ICC_DEBUG("PARAM Err!(nslot=%d)\r\n", nslot);
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    if(g_icc_fd[nslot]<0)
    {
        ICC_DEBUG("DEVICE_NOTOPEN!\r\n");
        return DEVSTATUS_ERR_DEVICE_NOTOPEN;
    }
    switch(nslot)
    {
    case ICC_SLOT_ICCARD:  
        ret = drv_icc_poweroff();
        break;
    case ICC_SLOT_PSAM1:
    case ICC_SLOT_PSAM2:
        if(g_psam_existflg)
        {
            ret = drv_psam_poweroff(nslot-1);
        }
        break;
    }
    return ret;
}
/****************************************************************************
**Description:       ��ȡIC��״̬
**Input parameters:    
**Output parameters: 
**Returned value:
                    0: �޿�
                    1: �п�
**Created by:        pengxuebin,20170425
**---------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_icc_getstatus(s32 nslot)
{
    
    if(ICC_SLOT_ICCARD == nslot)
    {
        return drv_icc_getstatus();
    }
    else
    {
        return 1;
    }
}
/****************************************************************************
**Description:       ��λ
**Input parameters:    
**Output parameters: 
**Returned value:
                    >0:��λ�ɹ�,����ATR����
                    <0:ʧ��
**Created by:        pengxuebin,20170425
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_icc_reset(s32 nslot, u8 *lpAtr)
{
    s32 ret = DEVSTATUS_ERR_PARAM_ERR;
    
    if(nslot >= ICC_SLOT_MAX)
    {
        ICC_DEBUG("PARAM Err!(nslot=%d)\r\n", nslot);
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    if(g_icc_fd[nslot]<0)
    {
        ICC_DEBUG("DEVICE_NOTOPEN!\r\n");
        return DEVSTATUS_ERR_DEVICE_NOTOPEN;
    }
    switch(nslot)
    {
    case ICC_SLOT_ICCARD: 
        ret = drv_icc_reset(lpAtr);
        break;
    case ICC_SLOT_PSAM1:
    case ICC_SLOT_PSAM2:
        if(g_psam_existflg)
        {
            ret = drv_psam_reset(nslot-1, lpAtr);
        }
        break;
    }
    return ret;
}
/****************************************************************************
**Description:       APDU����
**Input parameters:    
**Output parameters: 
**Returned value:
                    >0:��λ�ɹ�,����ATR����
                    <0:ʧ��
**Created by:        pengxuebin,20170425
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_icc_exchange_apdu(s32 nslot, const u8* lpCApdu, u32 nCApduLen, u8*lpRApdu, u32* lpRApduLen, u32 nRApduSize)
{
    s32 ret = DEVSTATUS_ERR_PARAM_ERR;
    
    if(nslot >= ICC_SLOT_MAX)
    {
        ICC_DEBUG("PARAM Err!(nslot=%d)\r\n", nslot);
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    if(g_icc_fd[nslot]<0)
    {
        ICC_DEBUG("DEVICE_NOTOPEN!\r\n");
        return DEVSTATUS_ERR_DEVICE_NOTOPEN;
    }
    switch(nslot)
    {
    case ICC_SLOT_ICCARD:  
        ret = drv_icc_exchange_apdu((u8*)lpCApdu, nCApduLen, lpRApdu, lpRApduLen, nRApduSize);
        break;
    case ICC_SLOT_PSAM1:
    case ICC_SLOT_PSAM2:
        if(g_psam_existflg)
        {
            ret = drv_psam_exchange_apdu(nslot-1, (u8*)lpCApdu, nCApduLen, lpRApdu, lpRApduLen, nRApduSize);
        }
        break;
    }
    return ret;
}
/****************************************************************************
**Description:        ��ͣ
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170808
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_icc_suspend(void)
{
    //drv_icc_poweroff();
    return 0;
    
}
/****************************************************************************
**Description:        ����
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170808
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_icc_resume(void)
{
    return 0;
}
