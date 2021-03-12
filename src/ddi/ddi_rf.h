

#ifndef _DDI_RF_H_
#define _DDI_RF_H_


#include "ddi_common.h"


/*�ǽӿ��Ķ�����������*/
#define     DDI_RF_CTL_VER              (0) //��ȡ�ǽӶ������豸�汾
#define     DDI_RF_CTL_SAK              (1) //��ȡѡ��Ӧ��SAKֵ
#define     DDI_RF_CTL_UID              (2) //��ƬID
#define     DDI_RF_CTL_MF_AUTH          (3) //Mifare����֤
#define     DDI_RF_CTL_MF_READ_RAW      (4) //��ȡԭʼ����������
#define     DDI_RF_CTL_MF_WRITE_RAW     (5) //дԭʼ����������
#define     DDI_RF_CTL_MF_READ_VALUE    (6) //��ȡ��ֵ
#define     DDI_RF_CTL_MF_WRITE_VALUE   (7) //д��ֵ
#define     DDI_RF_CTL_MF_INC_VALUE     (8) //��ֵ����
#define     DDI_RF_CTL_MF_DEC_VALUE     (9) //��ֵ����
#define     DDI_RF_CTL_MF_BACKUP_VALUE  (10) //��ֵ����
#define     DDI_RF_CTL_SET_PARAM        (11) //���÷ǽӿ���оƬ����
#define     DDI_RF_CTL_GET_PARAM        (12) //���ǽӿ���оƬ����

#if 0
//�����������ڲ��ԣ�����ΪDDI�ӿ�ʹ��
#define     DDI_RF_CTL_SET_LPMOD        (0x80) //����ѭ��ģʽ(��Ҫ���ڲ���)
#define     DDI_RF_CTL_TEST_RTSA        (0x81) //Requirements for Transaction Send Application
#endif
//============================================
//���⺯������
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




