/********************************Copyright ( c ) ********************************
**                  DAQU 
**                         
**                        
**
**Created By:     pengxuebin,20170320      
** Created Date:     
** Version:        
** Description:    watchdog驱动接口函数
  
****************************************************************************/
#include "devglobal.h"
#include "sdk/mhscpu_sdk.h"
#include "drv_watchdog.h"

u8 g_watchdog_mod = WATCHDOG_SET_MOD;
u8 g_watchdog_rstflg = 0;
/****************************************************************************
**Description:       WATCHDOG中断服务程序
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
        /*  喂狗操作和清中断均可以清除看门狗中断
            1、喂狗操作：同时完成清中断和重置看门狗计数器
            2、清中断操作：只清除看门狗中断，计数器由系统自动重载
        */
        /*  注释掉喂狗操作可测试看门狗系统复位功能 */

        /* 喂狗 */
        //  WDT_ReloadCounter();    //清中断相当于喂狗，故该语句可省略	
//        WDT_ClearITPendingBit();
//        printf("WDT Interrupt But Not Reset\n");

        dev_misc_reset_set(2);       //设置为WATCHDOG复位
        g_watchdog_rstflg = 1;
        dev_smc_systemsoftreset();
    }

}
/****************************************************************************
**Description:       使能WATCHDOG
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170630
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void drv_watchdog_modconfig(s32 mod)
{
    if(mod)
    {
        //中断模式
        g_watchdog_mod = 1;
        WDT->WDT_CR |= WDT_CR_RMOD;
    }
    else
    {   //CPU复位模式
        g_watchdog_mod = 0;
        WDT->WDT_CR &= ~WDT_CR_RMOD;
    }
}
/****************************************************************************
**Description:       设置WATCHDOG时间值
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
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
**Description:       使能WATCHDOG
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170630
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void drv_watchdog_enable(void)
{
    WDT->WDT_CR |= WDT_CR_WDT_EN;
}
/****************************************************************************
**Description:       清WATCHDOG中断标志
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
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
**Description:       清WATCHDOG中断标志
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170630
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
#define COUNTER_RELOAD_KEY                ((u32)0x76)
void drv_watchdog_feed(void)
{
    if((WDT->WDT_STAT & WDT_STAT_INT))
    {
        //看门狗中断产生,清中断
        WDT_ClearITPendingBit();
//        drv_watchdog_clr_pending();
    }
    //WDT_ReloadCounter();    //清中断相当于喂狗
    WDT->WDT_CRR = COUNTER_RELOAD_KEY;  //看门狗计数器重置
}
