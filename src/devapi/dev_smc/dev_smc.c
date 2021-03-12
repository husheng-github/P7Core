
#include "devglobal.h"
#include "mhscpu_sysctrl.h"



/*******************************************************************
Author:   
Function Name:  void dev_smc_systemsoftreset(void)
Function Purpose:System software reset
Input Paramters: 
Output Paramters:N/A
Return Value:      N/A
Remark: N/A
Modify: N/A
********************************************************************/
void dev_smc_systemsoftreset(void)
{
    //先关所有中断
    __disable_irq();
    __disable_fiq();
    
    /* request software reset */
    //NVIC_SystemReset();
    __dsb(0xF);
    //*((u32 *)0xE000ED0C) = 0x05fa0004|(((*((u32 *)0xE000ED0C)) & (7L<<8))); 
    SYSCTRL_AHBPeriphResetCmd(SYSCTRL_GLB_RESET,ENABLE);
    __dsb(0xF);
    while (1);
    
}


