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
#include "sdk/mhscpu_sdk.h"
#include "drv_watchdog.h"

u8 g_watchdog_mod = WATCHDOG_SET_MOD;
u8 g_watchdog_rstflg = 0;
/****************************************************************************
**Description:       WATCHDOG�жϷ������
**Input parameters:    
**Output parameters: 
**Returned value:
**Created by:        pengxuebin,20170630
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void NMI_Handler(void)
{
    if (FALSE != WDT_GetITStatus())
    {
        /*  ι�����������жϾ�����������Ź��ж�
            1��ι��������ͬʱ������жϺ����ÿ��Ź�������
            2�����жϲ�����ֻ������Ź��жϣ���������ϵͳ�Զ�����
        */
        /*  ע�͵�ι�������ɲ��Կ��Ź�ϵͳ��λ���� */

        /* ι�� */
        //  WDT_ReloadCounter();    //���ж��൱��ι�����ʸ�����ʡ��	
//        WDT_ClearITPendingBit();
//        printf("WDT Interrupt But Not Reset\n");

        dev_misc_reset_set(2);       //����ΪWATCHDOG��λ
        g_watchdog_rstflg = 1;
        dev_smc_systemsoftreset();
    }

}
/****************************************************************************
**Description:       ʹ��WATCHDOG
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170630
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void drv_watchdog_modconfig(s32 mod)
{
    if(mod)
    {
        //�ж�ģʽ
        g_watchdog_mod = 1;
        WDT->WDT_CR |= WDT_CR_RMOD;
    }
    else
    {   //CPU��λģʽ
        g_watchdog_mod = 0;
        WDT->WDT_CR &= ~WDT_CR_RMOD;
    }
}
/****************************************************************************
**Description:       ����WATCHDOGʱ��ֵ
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170630
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void drv_watchdog_set_reload(u32 ts)
{
    u32 i;

//    i = ((WATCHDOG_PCLK>>g_watchdog_mod)*ts);
    i = ((WATCHDOG_PCLK)*ts);
    WDT->WDT_RLD = i;
}
/****************************************************************************
**Description:       ʹ��WATCHDOG
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170630
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void drv_watchdog_enable(void)
{
    WDT->WDT_CR |= WDT_CR_WDT_EN;
}
/****************************************************************************
**Description:       ��WATCHDOG�жϱ�־
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170630
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void drv_watchdog_clr_pending(void)
{
    u32 i;
    i = WDT->WDT_EOI;
}
/****************************************************************************
**Description:       ��WATCHDOG�жϱ�־
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170630
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
#define COUNTER_RELOAD_KEY                ((u32)0x76)
void drv_watchdog_feed(void)
{
    if((WDT->WDT_STAT & WDT_STAT_INT))
    {
        //���Ź��жϲ���,���ж�
        WDT_ClearITPendingBit();
//        drv_watchdog_clr_pending();
    }
    //WDT_ReloadCounter();    //���ж��൱��ι��
    WDT->WDT_CRR = COUNTER_RELOAD_KEY;  //���Ź�����������
}
