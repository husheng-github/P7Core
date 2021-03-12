#ifndef _DDI_LCD_H_
#define _DDI_LCD_H_


#include "ddi_common.h"


#if 0
typedef enum
{                                                                                                      
     SDK_WORD_ZOOM_N  = 0,           //�����ֺ�                             
     SDK_WORD_ZOOM_H  = 1,           //���򱶸�                      
     SDK_WORD_ZOOM_W  = 2,             //���򱶿�                        
     SDK_WORD_ZOOM_WH = 3,            //�����Ŵ�                  
}eZoom;


typedef enum eAscSize
{
    E_WORD_ASCII_6_12   = 0,     //6*12
    E_WORD_ASCII_8_16   = 1,     //8*16
    E_WORD_ASCII_12_24  = 2,     //12*24
    E_WORD_ASCII_16_32  = 3,     //16*32
    E_WORD_ASCII_24_48  = 4,     //24*48 ��ӡ��֧��
    E_WORD_ASCII_ERR  =   5,     //������
}eAscSize;

typedef enum eNativeSize
{
    E_WORD_NATIVE_12_12 = 0,     //12*12
    E_WORD_NATIVE_16_16 = 1,     //16*16
    E_WORD_NATIVE_20_20 = 2,     //20*20
    E_WORD_NATIVE_24_24 = 3,     //24*24
    E_WORD_NATIVE_32_32 = 4,     //32*32
    E_WORD_NATIVE_48_48 = 5,     //48*48  ��ӡ��֧��
    E_WORD_NATIVE_ERR  =   6,     //������
}eNativeSize;
#endif

typedef struct _strRect
{
	u32  m_x0;  //�������X����
	u32  m_y0;  //�������Y����
	u32  m_x1;  //�����յ�X����
	u32  m_y1;  //�����յ�Y����
}strRect;
typedef struct _strDot
{
    u32 m_x;//�� X����
    u32 m_y;//�� Y����
} strDot;


typedef struct _strLine
{
    u32  m_x0;  //�����X����
    u32  m_y0;  //�����Y����
    u32  m_x1;  //���յ�X����
    u32  m_y1;  //���յ�Y����
}strLine;


typedef struct _strPicture
{
    u32  m_width;   //ͼ�����ص���
    u32  m_height;  //ͼ�����ص�߶�
    u32  *m_pic;   //ͼ����������,m_pic����ʽ��aRGB, ���й��򣺺������У������ҡ����ϵ���
}strPicture;

typedef struct _strBackGround
{
    u32 m_mode;//�������÷�ʽ
    strRect m_rect;//�������ھ�������
    u32		m_purecolor;//��ɫ����ʽ��aRGB 
    strPicture	m_pic;//����ͼƬ
}strBackGround;

typedef struct _strFont
{
    u32 m_font;             //���壬����ASCII�ַ�
    u32 m_ascsize;          //ASCII�ַ��ֺ�
    u32 m_nativesize;       //�����ֺ�
} strFont;

#define DDI_LCD_TYPE_MONO   0
#define DDI_LCD_TYPE_TFT    1

/*��ʾ��������*/
#define     DDI_LCD_CTL_SET_BACKGROUND      (0) //����ָ�����򱳾�ɫ
#define     DDI_LCD_CTL_SET_COLOR           (1) //����ǰ��ɫ
#define     DDI_LCD_CTL_SET_FONT            (2) //���õ�ǰ����
#define     DDI_LCD_CTL_SET_LINEWIDTH       (3) //�����߿�
#define     DDI_LCD_CTL_RESOLUTION          (4) //��ȡ��Ļ�ֱ���
#define     DDI_LCD_CTL_INVERT_RECT         (5) //����ָ������
#define     DDI_LCD_CTL_BRIGHT              (6) //������Ļ��������,�ڰ������ڻҶ�
#define     DDI_LCD_CTL_ICON                (7) //����ͼ��
#define     DDI_LCD_CTL_GET_DOTCOLOR        (8) //��ȡָ������ɫ
#define     DDI_LCD_CTL_BKLIGHT             (9) //ʹ��LCD����
#define     DDI_LCD_CTL_BKLIGHTTIME         (10) //����LCD����ʱ��
#define     DDI_LCD_CTL_GET_COLOR           (11) //��ȡ��ǰ��ǰ����ɫ
#define     DDI_LCD_CTL_BKLIGHT_CTRL        (12) //����LCD��������
#define     DDI_LCD_CTL_GET_FONT            (13)//��ȡ��ǰ����
#define     DDI_LCD_CTL_GET_LCDTYPE         (14)//��ȡ��Ļ���ͣ�0:   �ڰ�����1:����

//=====================================================
//���⺯������
extern s32 ddi_lcd_open (void);
extern s32 ddi_lcd_close (void);
extern s32 ddi_lcd_fill_rect (const strRect *lpstrRect, u32 nRGB);
extern s32 ddi_lcd_clear_rect (const strRect *lpstrRect);
extern s32 ddi_lcd_show_text(u32 nX, u32 nY, const u8* lpText);
extern s32 ddi_lcd_show_picture(const strRect *lpstrRect, const strPicture * lpstrPic);
extern s32 ddi_lcd_monochrome (u16 x, u16 y, u16 width, u16 height, u8 *p);
extern s32 ddi_lcd_show_pixel (u32 nX, u32 nY);
extern s32 ddi_lcd_show_line(const strLine  * lpstrLine );
extern s32 ddi_lcd_show_rect(const strRect  * lpstrRect);
extern s32 ddi_lcd_extract_rect(const strRect *lpstrRect, strPicture * lpstrPic);
extern s32 ddi_lcd_ioctl(u32 nCmd, u32 lParam, u32 wParam);
extern s32 ddi_lcd_show_picture_file(const strRect *lpstrRect, const u8 * lpBmpName);
extern void ddi_lcd_brush_screen(void);
//==================================================
typedef s32 (*core_ddi_lcd_open) (void);
typedef s32 (*core_ddi_lcd_close) (void);
typedef s32 (*core_ddi_lcd_fill_rect) (const strRect *lpstrRect, u32 nRGB);
typedef s32 (*core_ddi_lcd_clear_rect) (const strRect *lpstrRect);
typedef s32 (*core_ddi_lcd_show_text) (u32 nX, u32 nY, const u8* lpText);
typedef s32 (*core_ddi_lcd_show_picture)(const strRect *lpstrRect, const strPicture * lpstrPic);
typedef s32 (*core_ddi_lcd_monochrome)(u16 x, u16 y, u16 width, u16 height, u8 *p);
typedef s32 (*core_ddi_lcd_show_picture_file)(const strRect *lpstrRect,const u8 * lpBmpName);//��ʾͼ���ļ�
typedef s32 (*core_ddi_lcd_show_pixel) (u32 nX, u32 nY);
typedef s32 (*core_ddi_lcd_show_line)(const strLine  * lpstrLine );//����
typedef s32 (*core_ddi_lcd_show_rect)(const strRect  * lpstrRect);
typedef s32 (*core_ddi_lcd_extract_rect)(const strRect *lpstrRect, strPicture * lpstrPic);
typedef s32 (*core_ddi_lcd_ioctl)(u32 nCmd, u32 lParam, u32 wParam);
typedef void (*core_ddi_lcd_brush_screen)(void);


#endif

