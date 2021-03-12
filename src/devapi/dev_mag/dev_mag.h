#ifndef __DEV_MAG_H
#define __DEV_MAG_H


void dev_mag_init(void);
s32 dev_mag_open(void);
s32 dev_mag_close(void);
s32 dev_mag_read(u8 *tk1, u8 *tk2, u8 *tk3, u8 *trackstatus);
s32 dev_mag_clear(void);
s32 dev_mag_ioctl(u32 nCmd, u32 lParam, u32 wParam);
s32 dev_mag_ioctl_getmagswipestatus(void);
s32 dev_mag_get_ver(u8 *ver);


#endif

