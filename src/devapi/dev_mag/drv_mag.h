#ifndef __DRV_MAG_H
#define __DRV_MAG_H


#ifdef DEBUG_MAG_EN
#define MAG_DEBUG(format,...)    dev_debug_printf("%s(%d):"format, __func__, __LINE__, ##__VA_ARGS__)
#define MAG_DEBUGHEX             dev_debug_printformat
#else
#define MAG_DEBUG(...) 
#define MAG_DEBUGHEX(...)
#endif


s32 drv_mag_open(void);
s32 drv_mag_close(void);
s32 drv_mag_read(u8 *tk1, u8 *tk2, u8 *tk3);
s32 drv_mag_clear(void);
s32 drv_mag_ioctl_getmagswipestatus(void);
s32 dev_mag_getmsrstatus(void);
s32 drv_mag_get_ver(u8 *ver);


#endif

