#ifndef _DDI_LCD_H_
#define _DDI_LCD_H_


#include "ddi_common.h"


#if 0
typedef enum
{                                                                                                      
     SDK_WORD_ZOOM_N  = 0,           //基本字号                             
     SDK_WORD_ZOOM_H  = 1,           //纵向倍高                      
     SDK_WORD_ZOOM_W  = 2,             //横向倍宽                        
     SDK_WORD_ZOOM_WH = 3,            //整倍放大                  
}eZoom;


typedef enum eAscSize
{
    E_WORD_ASCII_6_12   = 0,     //6*12
    E_WORD_ASCII_8_16   = 1,     //8*16
    E_WORD_ASCII_12_24  = 2,     //12*24
    E_WORD_ASCII_16_32  = 3,     //16*32
    E_WORD_ASCII_24_48  = 4,     //24*48 打印不支持
    E_WORD_ASCII_ERR  =   5,     //检测错误
}eAscSize;

typedef enum eNativeSize
{
    E_WORD_NATIVE_12_12 = 0,     //12*12
    E_WORD_NATIVE_16_16 = 1,     //16*16
    E_WORD_NATIVE_20_20 = 2,     //20*20
    E_WORD_NATIVE_24_24 = 3,     //24*24
    E_WORD_NATIVE_32_32 = 4,     //32*32
    E_WORD_NATIVE_48_48 = 5,     //48*48  打印不支持
    E_WORD_NATIVE_ERR  =   6,     //检测错误
}eNativeSize;
#endif

typedef struct _strRect
{
	u32  m_x0;  //矩形起点X坐标
	u32  m_y0;  //矩形起点Y坐标
	u32  m_x1;  //矩形终点X坐标
	u32  m_y1;  //矩形终点Y坐标
}strRect;
typedef struct _strDot
{
    u32 m_x;//点 X坐标
    u32 m_y;//点 Y坐标
} strDot;


typedef struct _strLine
{
    u32  m_x0;  //线起点X坐标
    u32  m_y0;  //线起点Y坐标
    u32  m_x1;  //线终点X坐标
    u32  m_y1;  //线终点Y坐标
}strLine;


typedef struct _strPicture
{
    u32  m_width;   //图形像素点宽度
    u32  m_height;  //图形像素点高度
    u32  *m_pic;   //图形像素数据,m_pic：格式：aRGB, 排列规则：横向排列，从左到右、从上到下
}strPicture;

typedef struct _strBackGround
{
    u32 m_mode;//背景设置方式
    strRect m_rect;//背景窗口矩形区域
    u32		m_purecolor;//纯色，格式：aRGB 
    strPicture	m_pic;//背景图片
}strBackGround;

typedef struct _strFont
{
    u32 m_font;             //字体，关联ASCII字符
    u32 m_ascsize;          //ASCII字符字号
    u32 m_nativesize;       //国语字号
} strFont;

#define DDI_LCD_TYPE_MONO   0
#define DDI_LCD_TYPE_TFT    1

/*显示控制命令*/
#define     DDI_LCD_CTL_SET_BACKGROUND      (0) //设置指定区域背景色
#define     DDI_LCD_CTL_SET_COLOR           (1) //设置前景色
#define     DDI_LCD_CTL_SET_FONT            (2) //设置当前字体
#define     DDI_LCD_CTL_SET_LINEWIDTH       (3) //设置线宽
#define     DDI_LCD_CTL_RESOLUTION          (4) //读取屏幕分辨率
#define     DDI_LCD_CTL_INVERT_RECT         (5) //反显指定区域
#define     DDI_LCD_CTL_BRIGHT              (6) //控制屏幕背光亮度,黑白屏调节灰度
#define     DDI_LCD_CTL_ICON                (7) //控制图标
#define     DDI_LCD_CTL_GET_DOTCOLOR        (8) //获取指定点颜色
#define     DDI_LCD_CTL_BKLIGHT             (9) //使能LCD背光
#define     DDI_LCD_CTL_BKLIGHTTIME         (10) //设置LCD背光时间
#define     DDI_LCD_CTL_GET_COLOR           (11) //获取当前的前景颜色
#define     DDI_LCD_CTL_BKLIGHT_CTRL        (12) //控制LCD背光亮灭
#define     DDI_LCD_CTL_GET_FONT            (13)//获取当前字体
#define     DDI_LCD_CTL_GET_LCDTYPE         (14)//获取屏幕类型：0:   黑白屏，1:彩屏

//=====================================================
//对外函数声明
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
typedef s32 (*core_ddi_lcd_show_picture_file)(const strRect *lpstrRect,const u8 * lpBmpName);//显示图形文件
typedef s32 (*core_ddi_lcd_show_pixel) (u32 nX, u32 nY);
typedef s32 (*core_ddi_lcd_show_line)(const strLine  * lpstrLine );//划线
typedef s32 (*core_ddi_lcd_show_rect)(const strRect  * lpstrRect);
typedef s32 (*core_ddi_lcd_extract_rect)(const strRect *lpstrRect, strPicture * lpstrPic);
typedef s32 (*core_ddi_lcd_ioctl)(u32 nCmd, u32 lParam, u32 wParam);
typedef void (*core_ddi_lcd_brush_screen)(void);


#endif

