

#ifndef _DDI_THMPRN_H_
#define _DDI_THMPRN_H_


#include "ddi_common.h"
#include "../devapi/dev_printer/Esc_p.h"


typedef struct _strPrnTextCtrl
{
    u32  m_align;           //���뷽ʽ
    u32  m_offset;           //��ƫ��
    u32  m_font;            //���壬����ASCII�ַ�
    u32  m_ascsize;         //ASCII�ַ��ֺ�
    u32  m_asczoom;        //ASCII�ַ��Ŵ�ʽ
    u32  m_nativesize;       //�����ֺ�
    u32  m_nativezoom;     //����Ŵ�ʽ
} strPrnTextCtrl;

typedef struct _strPrnCombTextCtrl
{
    u32  m_x0;             //����ԭ��ĺ����� 
    u32  m_y0;             //����ԭ��������� 
    u32  m_font;            //���壬����ASCII�ַ�
    u32  m_ascsize;         //ASCII�ַ��ֺ�
    u32  m_asczoom;        //ASCII�ַ��Ŵ�ʽ
    u32  m_nativesize;       //�����ֺ�
    u32  m_nativezoom;     //����Ŵ�ʽ
    u8*  m_text;           //��ӡ�ı�����
} strPrnCombTextCtrl;

typedef enum eFont
{
    E_PRN_FONT_SONGTI   = 0,     //����
    E_PRN_FONT_HEITI    = 1,     //����
    E_PRN_FONT_YOUYUAN  = 2,     //��Բ
    E_PRN_FONT_YOUHEI   = 3,     //�׺�
}eFont;

typedef enum _PRINTERSTATUS
{
	PRN_FREE,
	PRN_PRINTING,
	PRN_NO_PAPER,
	PRN_OVER_TEMP,
	PRN_PRINTERERR
} PRINTERSTATUS;


typedef enum _PRN_CMD_SET
{
	PRN_CMD_SET_SPEED,
	PRN_CMD_SET_VOLTAGE
} PRN_CMD_SET;

union read_bmp
{
	u16 bufu16[20];
	u8	bufu8[40];
};
#define     DDI_THMPRN_CTL_VER          (0) //��ȡ������ӡ�汾
#define     DDI_THMPRN_CTL_GRAY         (1) //���ô�ӡ�Ҷ�
#define     DDI_THMPRN_CTL_LINEGAP      (2) //���ô�ӡ�ı��м��
#define     DDI_THMPRN_CTL_COLGAP       (3) //���ô�ӡ�ı��м��
#define     DDI_THMPRN_CTL_SETBATVOL    (4) //���õ�ص�ѹ
#define     DDI_THMPRN_GET_PRINT_LEN    (5) //��ȡ��ӡ������
#define     DDI_THMPRN_CTL_BLOCK_TEST   (6) //�ڿ��ӡ������
#define     DDI_THMPRN_CTL_GET_PAPER_STATUS   (7) //ͨ���߼���ƽ��ȡֽ��״̬




//============================================
//���⺯������
extern s32 ddi_thmprn_open (void);
extern s32 ddi_thmprn_close(void);
extern s32 ddi_thmprn_feed_paper(s32 nPixels);
extern s32 ddi_thmprn_print_image_file(u32 nOrgLeft, u32 nImageWidth, u32 nImageHeight, const u8 *lpImageName);
extern s32 ddi_thmprn_print_image (u32 nOrgLeft, u32 nImageWidth, u32 nImageHeight, const u8 *lpImage);
extern s32 ddi_thmprn_print_comb_text (u32 nNum, const strPrnCombTextCtrl* lpPrnCombTextCtrl[]);
extern s32 ddi_thmprn_print_text (strPrnTextCtrl *lpPrnCtrl, const u8 *lpText );
extern s32 ddi_thmprn_get_status (void);
extern s32 ddi_thmprn_ioctl(u32 nCmd, u32 lParam, u32 wParam);
extern s32 ddi_thmprn_esc_p(u8 *inEsc, u32 inLen);
extern s32 ddi_thmprn_esc_loop(void);
extern s32 ddi_thmprn_esc_init(void);

//==============================================
typedef s32 (*core_ddi_thmprn_open) (void);
typedef s32 (*core_ddi_thmprn_close)(void);
typedef s32 (*core_ddi_thmprn_feed_paper)(s32 nPixels);
typedef s32 (*core_ddi_thmprn_print_image) (u32 nOrgLeft, u32 nImageWidth, u32 nImageHeight, const u8 *lpImage);
typedef s32 (*core_ddi_thmprn_print_image_file)(u32 nOrgLeft, u32 nImageWidth, u32 nImageHeight, const u8 *lpImageName);
typedef s32 (*core_ddi_thmprn_print_text) (strPrnTextCtrl *lpPrnCtrl, const u8 *lpText );
typedef s32 (*core_ddi_thmprn_print_comb_text) (u32 nNum, const strPrnCombTextCtrl* lpPrnCombTextCtrl[] );
typedef s32 (*core_ddi_thmprn_get_status) (void);
typedef s32 (*core_ddi_thmprn_ioctl)(u32 nCmd, u32 lParam, u32 wParam);
typedef s32 (*core_ddi_thmprn_esc_p)(u8 *inEsc, u32 inLen);
typedef s32 (*core_ddi_thmprn_esc_loop)(void);
typedef s32 (*core_ddi_thmprn_esc_init)(void);


#endif



