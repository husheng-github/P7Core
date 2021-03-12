#ifndef __DEV_LCD_H
#define __DEV_LCD_H
#include "ddi/ddi_lcd.h"

#define MONO_LCD_2LINE   
#undef  MONO_LCD_2LINE

#ifdef MONO_LCD_2LINE
#define LCD_MAX_ROW     4//8
#define LCD_MAX_LINE    32//64
#define LCD_MAX_COL     128//132//
#define LCD_OFFSET       0
#else
//#define LCD_OVERTURN_EN   //定义是否翻转
#define LCD_MAX_ROW     8
#define LCD_MAX_LINE    64
#define LCD_MAX_COL     128//132//
#ifdef LCD_OVERTURN_EN
#define LCD_OFFSET       4
#else
#define LCD_OFFSET       0//4
#endif
#endif
#define BAT_ICON_FLNAME     "battery"
#define BAT_ICON_FLNAME_LEN  7
#define BT_ICON_FLNAME0     "disconnect"
#define BT_ICON_FLNAME1     "btconnect"


#if 0
#define LCD_Write               1
#define LCD_Scroll              2
//#define LCD_Fill              3
#define LCD_PutPixel            4
#define LCD_Line                5
#define LCD_Text                6
#define LCD_Cmd                 7
#define LCD_Disp                8
#define LCD_Cls                 9
#define LCD_ClearLine           10
#define LCD_Screen              11
#define LCD_FillRam             12
#define LCD_SetPara             13
#define LCD_Contrast            14

#define LCD_COMMAND             20
#define LCD_DATA                21
#endif

#define LINE1   0x00
#define LINE2   0x01
#define LINE3   0x02
#define LINE4   0x03
#define LINE5   0x04

#define DISPAGE1 0
#define DISPAGE2 1
#define DISPAGE3 2
#define DISPAGE4 3
#define DISPAGE5 4

#define BARLINE1 0
#define BARLINE2 16
#define BARLINE3 40
#define BARLINE4 56

#define FDISP    0x00   //正显(默认正显)//yhz 0905
#define NOFDISP  0x01   //反显
#define INCOL    0x02   //插入一列
#define LDISP    0x04   //左对齐
#define CDISP    0x08   //居中
#define RDISP    0x10   //右对齐

#define OVERLINE 0x20   //上面有标记横线
#define DOWNLINE 0x40   //下面有标记横线
#define SIDELINE 0x80   //左右有框线

#define CTRLA    0x80   //当本行字符满一行时，后跟有换行'\xa'或回车'\xd'字符,CTRLA取作用,当该位为0时不起作用

//---lcd 参数
#define MAXCOL          LCD_MAX_COL//128 //最大列数
#ifdef MONO_LCD_2LINE
#define MAXROW          2   //最大行数
#else
#define MAXROW          5   //最大行数
#endif
#define MAXCHAR         21  //每行最大字符数
#define LCDWORDWIDTH    12  //显示字列数
#define LCDCHARWIDTH    6   //显示字符列数

#define OFFSET          0   //偏移列数
#define LCD_COL_OFFSET  OFFSET
#define LCD_MAX_DOT_ROW LCD_MAX_LINE//64

//画笔颜色
#define WHITE            0xFFFF
#define BLACK            0x0000   
#define BLUE             0x001F  
#define BRED             0XF81F
#define GRED             0XFFE0
#define GBLUE            0X07FF
#define RED              0xF800
#define MAGENTA          0xF81F
#define GREEN            0x07E0
#define CYAN             0x7FFF
#define YELLOW           0xFFE0
#define BROWN            0XBC40 //棕色
#define BRRED            0XFC07 //棕红色
#define GRAY             0X8430 //灰色


#define GetRValue(ColorRGB) ((ColorRGB&0x00ff0000)>>16)
#define GetGValue(ColorRGB) ((ColorRGB&0x0000ff00)>>8)
#define GetBValue(ColorRGB) ((ColorRGB&0x000000ff)>>0)
#define RGB216BIT(ColorRGB) (GetRValue(ColorRGB)>>3<<11 | GetGValue(ColorRGB)>>2<<5 | GetBValue(ColorRGB)>>3)



struct _strLcdConf
{
    //器件参数
    u32 m_driver_id;//LCD 控制器型号
    u32 m_lcd_id;//lcd 型号
    u16 m_width;          //LCD 宽度
    u16 m_height;         //LCD 高度
    
    //配置参数
    u32 m_pencolor;//前景色，画笔颜色
    u32 m_backcolor;//背景色
    u32 m_linewidth;//线宽, 1到10倍

    //控制
    u8  m_dir;            //横屏还是竖屏控制：0，竖屏；1，横屏。  
    u8  m_wramcmd;        //开始写gram指令
    u8  m_setxcmd;        //设置x坐标指令
    u8  m_setycmd;        //设置y坐标指令  
    u8  m_gd;             //LCD ON/OFF 状态 0 OFF, 1 开   

    u16 m_lastx;    //上一次写的X Y 坐标
    u16 m_lasty;
};



//======================================================
s32 dev_lcd_open(void);
s32 dev_lcd_close(void);
void dev_lcd_brush_screen(void);
s32 dev_lcd_fill_rect(const strRect *lpstrRect, u32 nRGB);
s32 dev_lcd_clear_rect(const strRect *lpstrRect);
s32 dev_lcd_show_text(u32 nX, u32 nY, const u8* lpText);
s32 dev_lcd_show_picture(const strRect *lpstrRect, const strPicture * lpstrPic);
s32 dev_lcd_show_pixel (u32 nX, u32 nY);
s32 dev_lcd_show_line(const strLine  * lpstrLine );
s32 dev_lcd_show_rect(const strRect  * lpstrRect);
s32 dev_lcd_extract_rect(const strRect *lpstrRect, strPicture * lpstrPic);
s32 dev_lcd_ioctl(u32 nCmd, u32 lParam, u32 wParam);
s32 dev_lcd_show_picture_file(const strRect *lpstrRect, const u8 * lpBmpName);



void dev_lcd_fill_rowram(u8 rowid, u8 colid, u8 *str, u8 atr);
void dev_lcd_clear_ram(void);
void dev_lcd_clear_screen(void);
void dev_lcd_clear_rowram(u8 row);
void dev_lcd_disp_row(u8 row, u8 col, void *str,u8 atr);
void dev_lcd_clear_row(u8 row);
void dev_lcd_draw_line(s32 x0, s32 y0, s32 x1, s32 y1);
void dev_lcd_resume(void);
void dev_lcd_suspend(void);
void dev_lcd_disp_battary(u8 batnum);


#endif
