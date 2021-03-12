#include "devglobal.h"
#include "test_interface.h"
#include "ddi/ddi_audio.h"
#include "ddi/ddi_key.h"

#define KEYPAD_COM      0
#define KEYPAD_PHYSIC   1
#if(KEYPAD_EXIST == 1)
#define KEYPAD_TYPE     KEYPAD_COM//KEYPAD_PHYSIC//
#else
#define KEYPAD_TYPE     KEYPAD_COM//
#endif
#define DISP_COM        0
#define DISP_PHYSIC     1
#if(LCD_EXIST == 1)
#define DISP_TYPE       DISP_PHYSIC//DISP_COM//
#else
#define DISP_TYPE       DISP_COM//
#endif
#define KEYPAD_COM_PORT    PORT_CDCD_NO//PORT_UART0_NO//
//void test_interface_lcd_
void lcd_interface_clear_ram(void)
{
  #if(DISP_TYPE == DISP_COM)  
  #else
    dev_lcd_clear_ram();
  #endif
    
}
void lcd_interface_clear_rowram(u8 row)
{
  #if(DISP_TYPE == DISP_COM)  
  #else
    dev_lcd_clear_rowram(row);
  #endif
    
    
}

void lcd_interface_fill_rowram(u8 rowid, u8 colid, u8 *str, u8 atr)
{
  #if(DISP_TYPE == DISP_COM)  
    dev_debug_printf("%s\r\n", str);
  #else
    dev_lcd_fill_rowram(rowid, colid, str, atr);
  #endif
}
void lcd_interface_brush_screen(void)
{
  #if(DISP_TYPE == DISP_COM)  
  #else
    dev_lcd_brush_screen();
  #endif
    
}
void lcd_interface_fill_all(u32 nRGB)
{
  #if(DISP_TYPE == DISP_COM)  
  #else
    strRect l_strRect;
  
    l_strRect.m_x0 = 0;
    l_strRect.m_y0 = 0;
    l_strRect.m_x1 = LCD_MAX_COL;
    l_strRect.m_y1 = LCD_MAX_LINE;
    if(nRGB)
    {
        dev_lcd_fill_rect(&l_strRect, 1);
    }
    else
    {
        dev_lcd_clear_rect(&l_strRect);
    }
  #endif
    
}
void lcd_interface_bl_ctl(u8 mod)
{
  #if(DISP_TYPE == DISP_COM)  
  #else
    dev_lcd_ioctl(DDI_LCD_CTL_BKLIGHT_CTRL, mod, 0);
                
  #endif
    
}
s32 keypad_interface_read_beep(u32 *rkey)
{
  #if(KEYPAD_TYPE == KEYPAD_COM)  
    u8 ch;
    
    if(1 == dev_com_read(KEYPAD_COM_PORT, &ch, 1))
    {
        switch(ch)
        {
        case '1': 
            *rkey = DIGITAL1;
            break;
        case '2': 
            *rkey = DIGITAL2;
            break;
        case '3':
            *rkey = DIGITAL3;
            break; 
        case '4': 
            *rkey = DIGITAL4;
            break;
        case '5': 
            *rkey = DIGITAL5;
            break;
        case '6': 
            *rkey = DIGITAL6;
            break;
        case '7': 
            *rkey = DIGITAL7;
            break;
        case '8': 
            *rkey = DIGITAL8;
            break;
        case '9': 
            *rkey = DIGITAL9;
            break;
        case '0': 
            *rkey = DIGITAL0;
            break; 
        case 'e': 
        case 'E': 
            *rkey = ESC;
            break; 
        case 0x0d: 
        case 0x0a: 
            *rkey = ENTER;
            break; 
        case 'p':         
        case 'P':
            *rkey = POWER;
            break;
        default:
            *rkey = 0xFF;
            break;
        }
        return 1;
    }
    else
    {
        return 0;
    }
  #else
    
    s32 ret;
    u32 key;
    ret = dev_keypad_read(&key);
    if(ret > 0)
    {
        *rkey = key;
        dev_audio_ioctl(DDI_AUDIO_CTL_BUZZER, 0, 50);
    }
    return ret;
  #endif
}

s32 keypad_interface_clear(void)
{
  #if(KEYPAD_TYPE == KEYPAD_COM) 
    dev_com_flush(KEYPAD_COM_PORT);
  #else
    dev_keypad_clear();
  #endif
    return 0;
}

