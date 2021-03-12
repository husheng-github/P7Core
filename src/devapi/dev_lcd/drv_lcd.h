#ifndef __DRV_LCD_H
#define __DRV_LCD_H


#ifdef DEBUG_LCD_EN
#define LCD_DEBUG(format,...)    dev_debug_printf("%s(%d):"format, __func__, __LINE__, ##__VA_ARGS__)
#define LCD_DEBUGHEX             dev_debug_printformat
#else
#define LCD_DEBUG(...) 
#define LCD_DEBUGHEX(...)
#endif


typedef struct _str_LCD_PIN
{
    iomux_pin_name_t m_gpiopin;     //GPIO管脚做片选
    s32              m_rstval;      //GPIO初始化值
//    u32 m_pinmask;                  //pin掩码
}str_lcd_pin_t;


/******************LCD命令定义***********************************/
#define __DISPLAY_OFF        0xAE                     //关闭LCD显示
#define __DISPLAY_ON         0xAF                     //打开LCD显示
#define __DISPLAY_STR_L      0x40                     //设置开始显示的行。此命令可用来实现滚动显示
#define __PAGE_ADDR          0xB0                     //后四位表示页号
#define __COL_ADDR_HI        0x10                     //后四位表示列地址高四位
#define __COL_ADDR_LO        0x00                     //后四位表示列地址低四位
#define __SEG_DIR_NOR        0xA0                     //设置SEG反向
#define __SEG_DIR_REV        0xA1                     //设置SEG正常方向
#define __DISPLAY_INV        0xA7                     //反显也就是显示的点灭，灭的点显示
#define __DISPLAY_NOR        0xA6                     //正常显示
#define __DISPLAY_ALL_PIXEL  0xA5                     //显示所有的点
#define __DISPLAY_NOR_PIXEL  0xA4                     //正常显示，针对__DISPLAY_ALL_PIXEL来说
#define __DISPLAY_RESET      0xE2                     //软复位LCD
#define __COM_DIR_REV        0xC8                     //将行反转，0-->64 1-->63 ...63-->1 64-->0
#define __COM_DIR_NOR        0xC0                     //将行恢复正常
#define __PWR_ON             0x2F                     //VB VF VR全部打开
#define __PWR_OFF            0x28                     //VB VF VR全部关闭
#define __REG_RATIO          0x20                     //整个LCD背影调整
#define __NEGATIVE_LEVEL    0x2F    //VB VF VR all open

#define __CONT_CTRL          0x81                     //设置电子音量,双命令，先发0x81,再发一个0x00-0x3f的命令

#define __BIAS9              0xa2
#define __BIAS7              0xa3

#define __BIAS_CONTROL       0xa8                     //电流控制
#define __RATIO              0x3f                     //对比度

//=================================================
void drv_lcd_send_byte(u8 data);
void drv_lcd_set_ram_addr(u8 page, u8 col);
void drv_lcd_set_contrast(u8 contrast);
void drv_lcd_init(void);
void drv_lcd_disp_on(void);
extern void drv_lcd_A0_ctl(u8 flg);

#endif
