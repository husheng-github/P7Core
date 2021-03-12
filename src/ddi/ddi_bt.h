

#ifndef DDI_BT_H_
#define DDI_BT_H_



#include "ddi_common.h"


#define     DDI_BT_CTL_VER                      (0)           //��ȡ�������Ƴ���汾
#define     DDI_BT_CTL_MVER                     (1)           //��ȡ����ģ��̼��汾
#define     DDI_BT_CTL_GET_ADDR                 (2)           //��ȡ������ַ
#define     DDI_BT_CTL_RPIN                     (3)           //ȡPIN CODE
#define     DDI_BT_CTL_RNAME                    (4)           //��ȡ�����豸����
#define     DDI_BT_CTL_PAIRED_LIST              (5)           //��ȡ������б�
#define     DDI_BT_CTL_PAIRED_DEL               (6)           //ɾ��������豸��Ϣ
#define     DDI_BT_CTL_REMOTE_INFO              (7)           //��ȡԶ���豸����Ϣ
#define     DDI_BT_CTL_WPIN                     (8)           //����PIN CODE
#define     DDI_BT_CTL_WNAME                    (9)           //���������豸����
#define     DDI_BT_CTL_DISCOVERABLE             (10)          //�����Ƿ�ɱ�ɨ��
#define     DDI_BT_CTL_PAIRED_MODE_SET	        (11)          //�������ģʽ
#define     DDI_BT_CTL_PAIRED_MODE_GET	        (12)          //��ȡ���ģʽ
#define     DDI_BT_CTL_BT_STACK_VER            (13)
#define     DDI_BT_CTL_LOCAL_INFO               (14)
#define     DDI_BT_CTL_FIRMWAREUPDATE           (15)          //�̼�����
#define     DDI_BT_CTL_SET_ADDR                 (16)        //����������ַ


//=====================================================
//���⺯������
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

