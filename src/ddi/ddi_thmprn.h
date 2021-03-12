

#ifndef _DDI_THMPRN_H_
#define _DDI_THMPRN_H_


#include "ddi_common.h"
#include "../devapi/dev_printer/Esc_p.h"


typedef struct _strPrnTextCtrl
{
    u32  m_align;           //对齐方式
    u32  m_offset;           //列偏移
    u32  m_font;            //字体，关联ASCII字符
    u32  m_ascsize;         //ASCII字符字号
    u32  m_asczoom;        //ASCII字符放大方式
    u32  m_nativesize;       //国语字号
    u32  m_nativezoom;     //国语放大方式
} strPrnTextCtrl;

typedef struct _strPrnCombTextCtrl
{
    u32  m_x0;             //基于原点的横坐标 
    u32  m_y0;             //基于原点的纵坐标 
    u32  m_font;            //字体，关联ASCII字符
    u32  m_ascsize;         //ASCII字符字号
    u32  m_asczoom;        //ASCII字符放大方式
    u32  m_nativesize;       //国语字号
    u32  m_nativezoom;     //国语放大方式
    u8*  m_text;           //打印文本内容
} strPrnCombTextCtrl;

typedef enum eFont
{
    E_PRN_FONT_SONGTI   = 0,     //宋体
    E_PRN_FONT_HEITI    = 1,     //黑体
    E_PRN_FONT_YOUYUAN  = 2,     //幼圆
    E_PRN_FONT_YOUHEI   = 3,     //幼黑
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
#define     DDI_THMPRN_CTL_VER          (0) //获取热敏打印版本
#define     DDI_THMPRN_CTL_GRAY         (1) //设置打印灰度
#define     DDI_THMPRN_CTL_LINEGAP      (2) //设置打印文本行间距
#define     DDI_THMPRN_CTL_COLGAP       (3) //设置打印文本列间距
#define     DDI_THMPRN_CTL_SETBATVOL    (4) //设置电池电压
#define     DDI_THMPRN_GET_PRINT_LEN    (5) //获取打印机长度
#define     DDI_THMPRN_CTL_BLOCK_TEST   (6) //黑块打印机测试
#define     DDI_THMPRN_CTL_GET_PAPER_STATUS   (7) //通过逻辑电平获取纸张状态




//============================================
//对外函数声明
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



