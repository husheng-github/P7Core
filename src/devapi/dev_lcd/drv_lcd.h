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
    iomux_pin_name_t m_gpiopin;     //GPIO�ܽ���Ƭѡ
    s32              m_rstval;      //GPIO��ʼ��ֵ
//    u32 m_pinmask;                  //pin����
}str_lcd_pin_t;


/******************LCD�����***********************************/
#define __DISPLAY_OFF        0xAE                     //�ر�LCD��ʾ
#define __DISPLAY_ON         0xAF                     //��LCD��ʾ
#define __DISPLAY_STR_L      0x40                     //���ÿ�ʼ��ʾ���С������������ʵ�ֹ�����ʾ
#define __PAGE_ADDR          0xB0                     //����λ��ʾҳ��
#define __COL_ADDR_HI        0x10                     //����λ��ʾ�е�ַ����λ
#define __COL_ADDR_LO        0x00                     //����λ��ʾ�е�ַ����λ
#define __SEG_DIR_NOR        0xA0                     //����SEG����
#define __SEG_DIR_REV        0xA1                     //����SEG��������
#define __DISPLAY_INV        0xA7                     //����Ҳ������ʾ�ĵ�����ĵ���ʾ
#define __DISPLAY_NOR        0xA6                     //������ʾ
#define __DISPLAY_ALL_PIXEL  0xA5                     //��ʾ���еĵ�
#define __DISPLAY_NOR_PIXEL  0xA4                     //������ʾ�����__DISPLAY_ALL_PIXEL��˵
#define __DISPLAY_RESET      0xE2                     //��λLCD
#define __COM_DIR_REV        0xC8                     //���з�ת��0-->64 1-->63 ...63-->1 64-->0
#define __COM_DIR_NOR        0xC0                     //���лָ�����
#define __PWR_ON             0x2F                     //VB VF VRȫ����
#define __PWR_OFF            0x28                     //VB VF VRȫ���ر�
#define __REG_RATIO          0x20                     //����LCD��Ӱ����
#define __NEGATIVE_LEVEL    0x2F    //VB VF VR all open

#define __CONT_CTRL          0x81                     //���õ�������,˫����ȷ�0x81,�ٷ�һ��0x00-0x3f������

#define __BIAS9              0xa2
#define __BIAS7              0xa3

#define __BIAS_CONTROL       0xa8                     //��������
#define __RATIO              0x3f                     //�Աȶ�

//=================================================
void drv_lcd_send_byte(u8 data);
void drv_lcd_set_ram_addr(u8 page, u8 col);
void drv_lcd_set_contrast(u8 contrast);
void drv_lcd_init(void);
void drv_lcd_disp_on(void);
extern void drv_lcd_A0_ctl(u8 flg);

#endif
