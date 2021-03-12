

#ifndef _DEV_ICCPSAM_H_
#define _DEV_ICCPSAM_H_


#include "ddi_common.h"

/*ICC/PSAM控制命令*/
#define		DDI_ICCPSAM_CTL_VER					    0//获取ICC/PSAM设备版本
#define		DDI_ICCPSAM_CTL_ETUD					1//指定复位初始ETU
#define		DDI_MEMORY_CTL_POWEROFF				    2//MEMORY卡下电操作
#define		DDI_SLE4428_CTL_RESET					3//SLE4428卡复位操作
#define		DDI_SLE4428_CTL_READ					4//SLE4428卡读操作
#define		DDI_SLE4428_CTL_WRITE					5//SLE4428卡写操作
#define		DDI_SLE4428_CTL_READEC					6//SLE4428卡读剩余密码校验次数
#define		DDI_SLE4428_CTL_VERIFY					7//SLE4428卡密码校验
#define		DDI_SLE4428_CTL_UPDATESC				8//SLE4428卡密码修改
#define		DDI_SLE4442_CTL_RESET					9//SLE4442卡复位操作
#define		DDI_SLE4442_CTL_READ					10//SLE4442卡读操作
#define		DDI_SLE4442_CTL_WRITE					11//SLE4442卡写操作
#define		DDI_SLE4442_CTL_READEC					12//SLE4442卡读剩余密码校验次数
#define		DDI_SLE4442_CTL_VERIFY					13//SLE4442卡密码校验
#define		DDI_SLE4442_CTL_UPDATESC				14//SLE4442卡密码修改
#define		DDI_AT24CXX_CTL_READ					15//AT24C系列卡读操作
#define		DDI_AT24CXX_CTL_WRITE					16//AT24C系列卡写操作
#define		DDI_AT88SCXX_CTL_RESET					17//AT88SC系列卡复位操作
#define		DDI_AT88SCXX_CTL_READ					18//AT88SC系列卡读操作
#define		DDI_AT88SCXX_CTL_WRITE					19//AT88SC系列卡写操作
#define		DDI_AT88SCXX_CTL_READEC					20//AT88SC系列卡读剩余密码校验次数
#define		DDI_AT88SCXX_CTL_VERIFY					21//AT88SC系列卡密码校验
#define		DDI_AT88SCXX_CTL_UPDATEEC				22//AT88SC系列卡密码修改
#define		DDI_IS23SC1604_CTL_RESET				23//IS23SC1604卡复位操作
#define		DDI_IS23SC1604_CTL_READ					24//IS23SC1604卡读操作
#define		DDI_IS23SC1604_CTL_WRITE				25//IS23SC1604卡写操作
#define		DDI_IS23SC1604_CTL_ERASE				26//IS23SC1604卡擦除操作
#define		DDI_IS23SC1604_CTL_READEC				27//IS23SC1604卡读剩余密码校验次数
#define		DDI_IS23SC1604_CTL_VERIFY				28//IS23SC1604卡密码校验
#define		DDI_IS23SC1604_CTL_UPDATEEC				29//IS23SC1604卡修改密码


typedef enum _CARDSLOT
{
	CARDSLOT_ICC,
	CARDSLOT_PSAM1,
	CARDSLOT_PSAM2,
	CARDSLOT_PSAM3,
	CARDSLOT_RF
} CARDSLOT;

#define EMVICCCARDNO  0

/*ICC/PSAM控制命令*/


//对外函数声明
extern s32 ddi_iccpsam_open (u32 nSlot);
extern s32 ddi_iccpsam_close(u32 nSlot);
extern s32 ddi_iccpsam_poweroff (u32 nSlot);
extern s32 ddi_iccpsam_poweron (u32 nSlot, u8 *lpAtr);
extern s32 ddi_iccpsam_exchange_apdu (u32 nSlot, const u8* lpCApdu, u32 nCApduLen, u8*lpRApdu, u32* lpRApduLen, u32 nRApduSize);
extern s32 ddi_iccpsam_get_status (u32 nSlot);
extern s32 ddi_iccpsam_ioctl(u32 nCmd, u32 lParam, u32 wParam);
//==================================================
typedef s32 (*core_ddi_iccpsam_open) (u32 nSlot);
typedef s32 (*core_ddi_iccpsam_close)(u32 nSlot);
typedef s32 (*core_ddi_iccpsam_poweron) (u32 nSlot, u8 *lpAtr);
typedef s32 (*core_ddi_iccpsam_poweroff) (u32 nSlot);
typedef s32 (*core_ddi_iccpsam_get_status) (u32 nSlot);
typedef s32 (*core_ddi_iccpsam_exchange_apdu) (u32 nSlot, const u8* lpCApdu, u32 nCApduLen, u8*lpRApdu, u32* lpRApduLen, u32 nRApduSize);
typedef s32 (*core_ddi_iccpsam_ioctl)(u32 nCmd, u32 lParam, u32 wParam);

#endif

