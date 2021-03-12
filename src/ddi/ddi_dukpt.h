

#ifndef DDI_DUKPT_H_
#define DDI_DUKPT_H_


#include "ddi_common.h"








#ifdef DUKPTK_DEBUG
#define dukpt_printf dev_debug_printf
#else
#define dukpt_printf(fmt,arg...) 
#endif


typedef struct _strDukptInitInfo
{
    u8 m_groupindex;    //��Կ���0~1
    u8 m_keyindex;      //��Կ������0~9
    u8 m_initkey[24];   //��ʼ��Կ
    u8 m_keylen;        //��ʼ��Կ����,�̶�Ϊ16
    u8 m_ksnindex;      //ksn������,Ŀǰ����Կ������Ҫ����һ��
    u8 m_initksn[20];   //��ʼksn
    u8 m_ksnlen;        //��ʼksn����,�̶�Ϊ10
    
}strDukptInitInfo;


typedef struct _strDukptInInfo
{
    u8 m_groupindex;  //��Կ���0~1
    u8 m_ksnindex;   //ksn������
    u8 m_dukptmode;  //DUKPTģʽ: 1 -- ����PIN      2 --- ����MAC      3 --- ʹ��ͬһ��ksn����PIN��MAC
    u8 m_macmode;    //mac�㷨,Ŀǰ֧��3���㷨:  0 --- �㷨һ     1 --- �㷨��     2 --- �㷨��
    u8 *m_macdata;   //Ҫ���ܵ�MAC����
    u32 m_maclen;    //Ҫ���ܵ�MAC���ݳ���
    u8 *m_pindata;   //Ҫ���ܵ�PIN����
    u8 m_pinlen;     //Ҫ���ܵ�PIN���ݳ���

}strDukptInInfo;

typedef struct _strDukptOutInfo
{
    u8 *m_ksn;      //ksn
    u8 m_ksnlen;    //ksn����
    u8 *m_mac;      //���ܺ��MAC����
    u8 m_maclen;    //���ܺ��MAC���ݳ���
    u8 *m_pin;      //���ܺ��PIN����
    u8 m_pinlen;    //���ܺ��PIN���ݳ���

}strDukptOutInfo;

typedef struct _strDevDukpt
{
    s32 flag;           //��ʶ��˵�����ṹ���Ƿ������Կ
    u8 m_groupindex;    //��Կ���0~1
    u8 m_keyindex;      //��Կ������0~9
    u8 m_pinkey[24];    //��Կ
    u8 m_keylen;        //��Կ����
    u8 m_ksn[20];       //ksn
    u8 m_ksnlen;        //ksn����
}strDevDukpt;


typedef enum
{
	DDI_DUKPT_CTL_VER = 0,  // ��ȡDUKPT�豸�汾
    DDI_DUKPT_CTL_KSN_NEXT     //   DUKPT��Կ����
}DDI_DUKPT_CTL;


//=====================================================
//���⺯������
extern s32 ddi_dukpt_open(void);
extern s32 ddi_dukpt_close(void);
extern s32 ddi_dukpt_inject (strDukptInitInfo* lpstrDukptInitInfo);
extern s32 dev_dukpt_arithprocess(u32 nKeyGroup, u32 nKeyIndex, u32 nLen, const u8* lpIn, u8* lpOut, u32 nMode,u8 encryptemode);
extern s32 ddi_dukpt_encrypt(u32 nKeyGroup, u32 nKeyIndex, u32 nLen, const u8* lpIn, u8* lpOut, u32 nMode);
extern s32 ddi_dukpt_decrypt(u32 nKeyGroup, u32 nKeyIndex, u32 nLen, const u8* lpIn, u8* lpOut, u32 nMode);
extern s32 ddi_dukpt_getksn(u32 nKeyGroup, u32 nKeyIndex, u32 *lpLen, u8* lpOut);
extern s32 ddi_dukpt_ioctl(u32 nCmd, u32 lParam, u32 wParam);
//=====================================================
typedef s32 (*core_ddi_dukpt_open)(void);
typedef s32 (*core_ddi_dukpt_close)(void);
typedef s32 (*core_ddi_dukpt_inject) (strDukptInitInfo* lpstrDukptInitInfo);
typedef s32 (*core_ddi_dukpt_encrypt)(u32 nKeyGroup, u32 nKeyIndex, u32 nLen, const u8* lpIn, u8* lpOut, u32 nMode);
typedef s32 (*core_ddi_dukpt_decrypt)(u32 nKeyGroup, u32 nKsnIndex, u32 nLen, const u8* lpIn, u8* lpOut, u32 nMode);
typedef s32 (*core_ddi_dukpt_getksn)(u32 nKeyGroup, u32 nKeyIndex, u32 *lpLen, u8* lpOut);
typedef s32 (*core_ddi_dukpt_ioctl)(u32 nCmd, u32 lParam, u32 wParam);


#endif



