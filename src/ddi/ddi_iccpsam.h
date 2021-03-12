

#ifndef _DEV_ICCPSAM_H_
#define _DEV_ICCPSAM_H_


#include "ddi_common.h"

/*ICC/PSAM��������*/
#define		DDI_ICCPSAM_CTL_VER					    0//��ȡICC/PSAM�豸�汾
#define		DDI_ICCPSAM_CTL_ETUD					1//ָ����λ��ʼETU
#define		DDI_MEMORY_CTL_POWEROFF				    2//MEMORY���µ����
#define		DDI_SLE4428_CTL_RESET					3//SLE4428����λ����
#define		DDI_SLE4428_CTL_READ					4//SLE4428��������
#define		DDI_SLE4428_CTL_WRITE					5//SLE4428��д����
#define		DDI_SLE4428_CTL_READEC					6//SLE4428����ʣ������У�����
#define		DDI_SLE4428_CTL_VERIFY					7//SLE4428������У��
#define		DDI_SLE4428_CTL_UPDATESC				8//SLE4428�������޸�
#define		DDI_SLE4442_CTL_RESET					9//SLE4442����λ����
#define		DDI_SLE4442_CTL_READ					10//SLE4442��������
#define		DDI_SLE4442_CTL_WRITE					11//SLE4442��д����
#define		DDI_SLE4442_CTL_READEC					12//SLE4442����ʣ������У�����
#define		DDI_SLE4442_CTL_VERIFY					13//SLE4442������У��
#define		DDI_SLE4442_CTL_UPDATESC				14//SLE4442�������޸�
#define		DDI_AT24CXX_CTL_READ					15//AT24Cϵ�п�������
#define		DDI_AT24CXX_CTL_WRITE					16//AT24Cϵ�п�д����
#define		DDI_AT88SCXX_CTL_RESET					17//AT88SCϵ�п���λ����
#define		DDI_AT88SCXX_CTL_READ					18//AT88SCϵ�п�������
#define		DDI_AT88SCXX_CTL_WRITE					19//AT88SCϵ�п�д����
#define		DDI_AT88SCXX_CTL_READEC					20//AT88SCϵ�п���ʣ������У�����
#define		DDI_AT88SCXX_CTL_VERIFY					21//AT88SCϵ�п�����У��
#define		DDI_AT88SCXX_CTL_UPDATEEC				22//AT88SCϵ�п������޸�
#define		DDI_IS23SC1604_CTL_RESET				23//IS23SC1604����λ����
#define		DDI_IS23SC1604_CTL_READ					24//IS23SC1604��������
#define		DDI_IS23SC1604_CTL_WRITE				25//IS23SC1604��д����
#define		DDI_IS23SC1604_CTL_ERASE				26//IS23SC1604����������
#define		DDI_IS23SC1604_CTL_READEC				27//IS23SC1604����ʣ������У�����
#define		DDI_IS23SC1604_CTL_VERIFY				28//IS23SC1604������У��
#define		DDI_IS23SC1604_CTL_UPDATEEC				29//IS23SC1604���޸�����


typedef enum _CARDSLOT
{
	CARDSLOT_ICC,
	CARDSLOT_PSAM1,
	CARDSLOT_PSAM2,
	CARDSLOT_PSAM3,
	CARDSLOT_RF
} CARDSLOT;

#define EMVICCCARDNO  0

/*ICC/PSAM��������*/


//���⺯������
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

