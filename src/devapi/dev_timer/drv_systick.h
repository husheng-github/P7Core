#ifndef __DRV_SYSTICK_H
#define __DRV_SYSTICK_H


s32 drv_systick_int(void);
u32 drv_systick_get_usid(void);
u32 drv_systick_querry_us(u32 tus, u32 delayus);
u32 drv_systick_get_msid(void);
u32 drv_systick_querry_ms(u32 tms, u32 delayms);
void drv_systick_delay_us(u32 nus);
void drv_systick_delay_ms(u32 nms);
void drv_systick_ctl(u8 flg);
void drv_systick_set_run_switch(s32 flag);
s32 drv_systick_get_run_switch(void);

#endif

