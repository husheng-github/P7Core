#ifndef __DDI_CONFIG_H
#define __DDI_CONFIG_H

#include "devglobal.h"

//���巵��ֵ
#define DDI_OK          (0)//�ɹ�
#define DDI_ERR         (-1)//����
#define DDI_ETIMEOUT    (-2)//��ʱ
#define DDI_EBUSY       (-3)//�豸��æ
#define DDI_ENODEV      (-4)//�豸������
#define DDI_EACCES      (-5)//��Ȩ��
#define DDI_EINVAL      (-6)//������Ч
#define DDI_EIO         (-7)//�豸δ�򿪻��豸��������
#define DDI_EDATA       (-8)//���ݴ���
#define DDI_EPROTOCOL   (-9)//Э�����
#define DDI_ETRANSPORT  (-10)//�������
#define DDI_ESIM		(-11)//SIM��������
#define DDI_EOPERATION	(-12)//ע���������
#define DDI_ESET_APN 	(-13)//����APNʧ��
#define DDI_EWOULD_BLOCK (-14)//������
#define DDI_EOVERFLOW	(-15)//�ڴ����
#define DDI_EUNKOWN_CMD  (-16)//δ֪����
#define DDI_EINVALID_IP  (-17)//IP��ַ�Ƿ�
#define DDI_ELOW_CSQ     (-18)//�źŵ�
#define DDI_EWIRE_PDP_ACTIVING  (-19)//���ڼ�����������
#define DDI_EUN_SUPPORT         (-20)//��֧��
#define DDI_EREG_DENIED         (-21)//ע�����类��
#define DDI_EHIGHTEMP           (-22)//�¶ȹ���
#define DDI_ENOFILE             (-23)//�ļ�������
#define DDI_EHAVE_DONE          (-24)//�����ظ�����ִ�й�
#define DDI_EDNS_FAIL           (-30)//��������ʧ��




#define DDI_COM_ENABLE
//#define DDI_ICCPSAM_ENABLE
//#define DDI_MAG_ENABLE               //sxl?
//#define DDI_RF_ENABLE
//#define DDI_MIFARE_ENABLE  
#define DDI_AUDIO_ENABLE
#define DDI_LED_ENABLE             //sxl?
//#define DDI_WATCHDOG_ENABLE
//#define DDI_DUKPT_ENABLE

#if(KEYPAD_EXIST==1)
#define DDI_KEY_ENABLE
#endif

#if(LCD_EXIST==1)
#define DDI_LCD_ENABLE
#endif

#if(PRINTER_EXIST==1)
#define DDI_THMPRN_ENABLE
#endif

#if(BT_EXIST==1)  
#define DDI_BT_ENABLE
#endif

#if(WIFI_EXIST == 1)
#define DDI_WIFI_ENABLE
#endif

//�����ļ�
#define WATCHDOG_EN     1

#ifndef TRUE_TRENDIT
#define TRUE_TRENDIT 1
#endif

#ifndef FALSE_TRENDIT
#define FALSE_TRENDIT 0
#endif

typedef enum{
    AT_API_NO_BLOCK,     //��������ȡ����Ҫ����ģ����Ʋ��Ե���
    AT_API_BLOCK         //������ȡ����Ҫ���ɸ�Ӧ�ò��api����
}AT_API_GET_TYPE_E;


#endif
