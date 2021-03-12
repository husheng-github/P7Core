

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
    u8 m_groupindex;    //密钥组号0~1
    u8 m_keyindex;      //密钥索引号0~9
    u8 m_initkey[24];   //初始密钥
    u8 m_keylen;        //初始密钥长度,固定为16
    u8 m_ksnindex;      //ksn索引号,目前与密钥索引号要保持一致
    u8 m_initksn[20];   //初始ksn
    u8 m_ksnlen;        //初始ksn长度,固定为10
    
}strDukptInitInfo;


typedef struct _strDukptInInfo
{
    u8 m_groupindex;  //密钥组号0~1
    u8 m_ksnindex;   //ksn索引号
    u8 m_dukptmode;  //DUKPT模式: 1 -- 加密PIN      2 --- 加密MAC      3 --- 使用同一组ksn加密PIN和MAC
    u8 m_macmode;    //mac算法,目前支持3种算法:  0 --- 算法一     1 --- 算法二     2 --- 算法三
    u8 *m_macdata;   //要加密的MAC数据
    u32 m_maclen;    //要加密的MAC数据长度
    u8 *m_pindata;   //要加密的PIN数据
    u8 m_pinlen;     //要加密的PIN数据长度

}strDukptInInfo;

typedef struct _strDukptOutInfo
{
    u8 *m_ksn;      //ksn
    u8 m_ksnlen;    //ksn长度
    u8 *m_mac;      //加密后的MAC数据
    u8 m_maclen;    //加密后的MAC数据长度
    u8 *m_pin;      //加密后的PIN数据
    u8 m_pinlen;    //加密后的PIN数据长度

}strDukptOutInfo;

typedef struct _strDevDukpt
{
    s32 flag;           //标识，说明本结构体是否存在密钥
    u8 m_groupindex;    //密钥组号0~1
    u8 m_keyindex;      //密钥索引号0~9
    u8 m_pinkey[24];    //密钥
    u8 m_keylen;        //密钥长度
    u8 m_ksn[20];       //ksn
    u8 m_ksnlen;        //ksn长度
}strDevDukpt;


typedef enum
{
	DDI_DUKPT_CTL_VER = 0,  // 获取DUKPT设备版本
    DDI_DUKPT_CTL_KSN_NEXT     //   DUKPT密钥递增
}DDI_DUKPT_CTL;


//=====================================================
//对外函数声明
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



