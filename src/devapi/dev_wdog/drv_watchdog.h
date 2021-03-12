#ifndef __DRV_WATCHDOG_H
#define __DRV_WATCHDOG_H

#define WATCHDOG_TIME_DEFAULT   15    //pclk=48000000,�����Զ�ʱ89S���жϷ�ʽ����
#define WATCHDOG_SET_MOD        1     //1���жϷ�ʽ��0��POLLING��ʽ
#define WATCHDOG_PCLK           (SYSCTRL->PCLK_1MS_VAL*1000)
#define WATCHDOG_TIME_MAX       89    //pclk=48000000,�����Զ�ʱ89S���жϷ�ʽ����

void drv_watchdog_modconfig(s32 mod);
void drv_watchdog_set_reload(u32 ts);
void drv_watchdog_enable(void);
void drv_watchdog_clr_pending(void);
void drv_watchdog_feed(void);


#endif
