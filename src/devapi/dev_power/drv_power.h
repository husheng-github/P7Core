#ifndef __DRV_POWER_H
#define __DRV_POWER_H


#ifdef DEBUG_POWER_EN
#define POWER_DEBUG(format,...)    dev_debug_printf("%s(%d):"format, __func__, __LINE__, ##__VA_ARGS__)
#define POWER_DEBUGHEX             dev_debug_printformat
#else
#define POWER_DEBUG(...) 
#define POWER_DEBUGHEX(...)
#endif

s32 drv_power_init(void);
void drv_power_switch_ctl(u8 flg);
s32 drv_power_get_wakuppin(void);
s32 drv_power_sleep(u32 mod);


#endif
