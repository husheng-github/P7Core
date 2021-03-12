

#ifndef DDI_BT_H_
#define DDI_BT_H_



#include "ddi_common.h"


#define     DDI_BT_CTL_VER                      (0)           //获取蓝牙控制程序版本
#define     DDI_BT_CTL_MVER                     (1)           //获取蓝牙模块固件版本
#define     DDI_BT_CTL_GET_ADDR                 (2)           //获取蓝牙地址
#define     DDI_BT_CTL_RPIN                     (3)           //取PIN CODE
#define     DDI_BT_CTL_RNAME                    (4)           //获取蓝牙设备名称
#define     DDI_BT_CTL_PAIRED_LIST              (5)           //获取已配对列表
#define     DDI_BT_CTL_PAIRED_DEL               (6)           //删除已配对设备信息
#define     DDI_BT_CTL_REMOTE_INFO              (7)           //获取远端设备的信息
#define     DDI_BT_CTL_WPIN                     (8)           //设置PIN CODE
#define     DDI_BT_CTL_WNAME                    (9)           //设置蓝牙设备名称
#define     DDI_BT_CTL_DISCOVERABLE             (10)          //设置是否可被扫描
#define     DDI_BT_CTL_PAIRED_MODE_SET	        (11)          //设置配对模式
#define     DDI_BT_CTL_PAIRED_MODE_GET	        (12)          //获取配对模式
#define     DDI_BT_CTL_BT_STACK_VER            (13)
#define     DDI_BT_CTL_LOCAL_INFO               (14)
#define     DDI_BT_CTL_FIRMWAREUPDATE           (15)          //固件升级
#define     DDI_BT_CTL_SET_ADDR                 (16)        //设置蓝牙地址


//=====================================================
//对外函数声明
extern s32 ddi_bt_open(void);
extern s32 ddi_bt_close(void);
extern s32 ddi_bt_write(u8 *lpIn, s32 nLe);
extern s32 ddi_bt_read (u8 *lpOut, s32 nLe);
extern s32 ddi_bt_get_status(void);
extern s32 ddi_bt_disconnect(const u8 *lpMac);
extern s32 ddi_bt_ioctl(u32 nCmd, u32 lParam, u32 wParam);
//====================================================
typedef s32 (*core_ddi_bt_open) (void);
typedef s32 (*core_ddi_bt_close)(void);
typedef s32 (*core_ddi_bt_disconnect)(const u8 *lpMac);
typedef s32 (*core_ddi_bt_write)(u8 *lpIn, s32 nLe);
typedef s32 (*core_ddi_bt_read)(u8 *lpOut, s32 nLe );
typedef s32 (*core_ddi_bt_get_status)(void);
typedef s32 (*core_ddi_bt_ioctl)(u32 nCmd, u32 lParam, u32 wParam);

#endif

