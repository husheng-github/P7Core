#ifndef __DRV_WATCHDOG_H
#define __DRV_WATCHDOG_H

#define WATCHDOG_TIME_DEFAULT   15    //pclk=48000000,最大可以定时89S，中断方式翻倍
#define WATCHDOG_SET_MOD        1     //1是中断方式，0是POLLING方式
#define WATCHDOG_PCLK           (SYSCTRL->PCLK_1MS_VAL*1000)
#define WATCHDOG_TIME_MAX       89    //pclk=48000000,最大可以定时89S，中断方式翻倍

void drv_watchdog_modconfig(s32 mod);
void drv_watchdog_set_reload(u32 ts);
void drv_watchdog_enable(void);
void drv_watchdog_clr_pending(void);
void drv_watchdog_feed(void);


#endif
