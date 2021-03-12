#ifndef _TEST_INTERFACE_H
#define _TEST_INTERFACE_H

#if 0
#define POWER           (0x01) //系统关机
#define F1              (0x02)  // 0x02
#define F2              (0x03)        // 0x03
#define F3              (0x04)       // 0x04
#define FUNCTION        (0x05)       // 0x05
#define DIGITAL1        (0x06)       // 0x06
#define DIGITAL2        (0x07)  // 0x07
#define DIGITAL3        (0x08)  // 0x08
#define ALPHA           (0x09)  // 0x09
#define DIGITAL4        (0x0A)       // 0x0A
#define DIGITAL5        (0x0B)  // 0x0B
#define DIGITAL6        (0x0C)  // 0x0C
#define ESC             (0x0D)  // 0x0D
#define DIGITAL7        (0x0E)          // 0x0E
#define DIGITAL8        (0x0F)  // 0x0F
#define DIGITAL9        (0x10)  // 0x10
#define CLEAR           (0x11)  // 0x11
#define UP_OR_10        (0x12) //向上，星号  // 0x12
#define DIGITAL0        (0x13)             // 0x13
#define DOWN_OR_11      (0x14) //向下，井号 // 0x14
#define ENTER           (0x15)         // 0x15

//--------------------------------------------
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

#define LCD_MAX_ROW     8
#define LCD_MAX_LINE    64
#define LCD_MAX_COL     128//132//
//--------------------------------------------

#endif



void lcd_interface_clear_ram(void);
void lcd_interface_clear_rowram(u8 row);
void lcd_interface_fill_rowram(u8 rowid, u8 colid, u8 *str, u8 atr);
void lcd_interface_brush_screen(void);
void lcd_interface_fill_all(u32 nRGB);
void lcd_interface_bl_ctl(u8 mod);
s32 keypad_interface_read_beep(u32 *rkey);
s32 keypad_interface_clear(void);


//#define dev_lcd_clear_ram    lcd_interface_clear_ram
//#define dev_lcd_clear_rowram lcd_interface_clear_rowram
//#define dev_lcd_fill_rowram  lcd_interface_fill_rowram
//#define dev_lcd_brush_screen lcd_interface_brush_screen
//#define drv_lcd_bl_ctl    lcd_interface_bl_ctl
//#define posdev_keypad_read_beep keypad_interface_read_beep
//#define posdev_keypad_clear keypad_interface_clear

#endif

