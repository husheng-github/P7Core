

#ifndef _DDI_MAG_H_
#define _DDI_MAG_H_


#include "ddi_common.h"



/*磁卡*/
#define     DDI_MAG_CTL_VER             (0) //获取磁卡阅读器版本     
#define     DDI_MAG_CTL_GETMAGSTATUS    (1)


//=====================================================
//对外函数声明
extern s32 ddi_mag_open (void);
extern s32 ddi_mag_close (void);
extern s32 ddi_mag_clear(void);
extern s32 ddi_mag_cfg(u32 cfg, void *p);
extern s32 ddi_mag_ioctl(u32 nCmd, u32 lParam, u32 wParam);
extern s32 ddi_mag_read (u8 *lpTrack1, u8 *lpTrack2, u8 *lpTrack3, u8 *trackstatus);
//===================================================
typedef s32 (*core_ddi_mag_open) (void);
typedef s32 (*core_ddi_mag_close) (void);
typedef s32 (*core_ddi_mag_clear)(void);
typedef s32 (*core_ddi_mag_read) (u8 *lpTrack1, u8 *lpTrack2, u8 *lpTrack3, u8 *trackstatus);
typedef s32 (*core_ddi_mag_ioctl)(u32 nCmd, u32 lParam, u32 wParam);

#endif

