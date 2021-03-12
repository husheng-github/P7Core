

#ifndef _DDI_RF_H_
#define _DDI_RF_H_


#include "ddi_common.h"


/*非接卡阅读器控制命令*/
#define     DDI_RF_CTL_VER              (0) //获取非接读卡器设备版本
#define     DDI_RF_CTL_SAK              (1) //获取选择应答SAK值
#define     DDI_RF_CTL_UID              (2) //卡片ID
#define     DDI_RF_CTL_MF_AUTH          (3) //Mifare卡认证
#define     DDI_RF_CTL_MF_READ_RAW      (4) //读取原始二进制数据
#define     DDI_RF_CTL_MF_WRITE_RAW     (5) //写原始二进制数据
#define     DDI_RF_CTL_MF_READ_VALUE    (6) //读取块值
#define     DDI_RF_CTL_MF_WRITE_VALUE   (7) //写块值
#define     DDI_RF_CTL_MF_INC_VALUE     (8) //增值操作
#define     DDI_RF_CTL_MF_DEC_VALUE     (9) //减值操作
#define     DDI_RF_CTL_MF_BACKUP_VALUE  (10) //块值备份
#define     DDI_RF_CTL_SET_PARAM        (11) //设置非接控制芯片参数
#define     DDI_RF_CTL_GET_PARAM        (12) //读非接控制芯片参数

#if 0
//下面命令用于测试，不作为DDI接口使用
#define     DDI_RF_CTL_SET_LPMOD        (0x80) //设置循环模式(主要用于测试)
#define     DDI_RF_CTL_TEST_RTSA        (0x81) //Requirements for Transaction Send Application
#endif
//============================================
//对外函数声明
extern s32 ddi_rf_open (void);
extern s32 ddi_rf_close(void);
extern s32 ddi_rf_poweron (u32 nType);
extern s32 ddi_rf_poweroff (void);
extern s32 ddi_rf_get_status (void);
extern s32 ddi_rf_activate (void);
extern s32 ddi_rf_exchange_apdu (const u8*lpCApdu, u32 nCApduLen, u8*lpRApdu, u32 *lpRApduLen, u32 nRApduSize);
extern s32 ddi_rf_remove (void);
extern s32 ddi_rf_ioctl(u32 nCmd, u32 lParam, u32 wParam);
//============================================
typedef s32 (*core_ddi_rf_open) (void);
typedef s32 (*core_ddi_rf_close)(void);
typedef s32 (*core_ddi_rf_poweron) (u32 nType);
typedef s32 (*core_ddi_rf_poweroff) (void);
typedef s32 (*core_ddi_rf_get_status) (void);
typedef s32 (*core_ddi_rf_activate) (void);
typedef s32 (*core_ddi_rf_exchange_apdu) (const u8*lpCApdu, u32 nCApduLen, u8*lpRApdu, u32 *lpRApduLen, u32 nRApduSize);
typedef s32 (*core_ddi_rf_remove) (void);
typedef s32 (*core_ddi_rf_ioctl)(u32 nCmd, u32 lParam, u32 wParam);

#endif




