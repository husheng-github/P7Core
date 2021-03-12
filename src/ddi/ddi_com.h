

#ifndef _DDI_COM_H_
#define _DDI_COM_H_


#include "ddi_common.h"



typedef struct _strComAttr
{
    s32 m_baud;     //�����ʣ�9600��19200��38400��115200
    s32 m_databits;     //����λ��ȡֵ��5��6��7��8
    s32 m_parity;       //У�鷽ʽ��0-��У�飻1-��У�飻2-żУ��
    s32 m_stopbits;     //ֹͣλ��1��2
}strComAttr;


/*���ڿ�������*/
#define     DDI_COM_CTL_BAUD            (0) //���ò�����
#define     DDI_COM_CTL_FLWCTL          (1) //��������

#define CDC_PORT   5
#define DBG_PORT   0


//=====================================================
//���⺯������
extern s32 ddi_com_open (u32 nCom, const strComAttr *lpstrAttr);
extern s32 ddi_com_close (u32 nCom);
extern s32 ddi_com_clear (u32 nCom);
extern s32 ddi_com_tcdrain (u32 nCom);
extern s32 ddi_com_read (u32 nCom, u8 *lpOut, s32 nLe );
extern s32 ddi_com_write (u32 nCom, u8 *lpIn, s32 nLe );
extern s32 ddi_com_ioctl(u32 nCom, u32 nCmd, u32 lParam, u32 wParam);

//=======================================================
typedef s32 (*core_ddi_com_open) (u32 nCom, const strComAttr *lpstrAttr);
typedef s32 (*core_ddi_com_close) (u32 nCom);
typedef s32 (*core_ddi_com_clear) (u32 nCom);
typedef s32 (*core_ddi_com_read) (u32 nCom, u8 *lpOut, s32 nLe );
typedef s32 (*core_ddi_com_write) (u32 nCom, u8 *lpIn, s32 nLe );
typedef s32 (*core_ddi_com_ioctl)(u32 nCom, u32 nCmd, u32 lParam, u32 wParam);
typedef s32 (*core_ddi_com_tcdrain)(u32 nCom);




#endif

