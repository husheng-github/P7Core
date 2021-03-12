#include "devglobal.h"
#include "ddi_lcd.h"

#ifdef DDI_LCD_ENABLE

s32 ddi_lcd_open (void)
{
	return dev_lcd_open();
}


s32 ddi_lcd_close (void)
{
	return dev_lcd_close();
}

s32 ddi_lcd_fill_rect(const strRect *lpstrRect, u32 nRGB)
{
	return dev_lcd_fill_rect(lpstrRect, nRGB);
}


s32 ddi_lcd_clear_rect(const strRect *lpstrRect)
{
	return dev_lcd_clear_rect(lpstrRect);
}


s32 ddi_lcd_show_text(u32 nX, u32 nY, const u8* lpText)
{
	return dev_lcd_show_text(nX, nY, lpText);
}


s32 ddi_lcd_show_picture(const strRect *lpstrRect, const strPicture * lpstrPic)
{
	return dev_lcd_show_picture(lpstrRect, lpstrPic);
}

s32 ddi_lcd_monochrome (u16 x, u16 y, u16 width, u16 height, u8 *p)
{
    strRect l_strrect;
    strPicture l_strPicture;
    u8 *tmp;
    s32 ret;
    u32 len;

    len = (width*height)>>3;
    tmp = k_malloc(len);
    if(tmp == NULL)
    {
        return DDI_ERR;
    }
    memcpy(tmp, p, len);
    Font_HV_change(width, height, 1, tmp);
    l_strrect.m_x0 = x;
    l_strrect.m_y0 = y;
    l_strrect.m_x1 = x+width-1;
    l_strrect.m_y1 = y+height-1;
    l_strPicture.m_height = height;
    l_strPicture.m_width = width;
    l_strPicture.m_pic = (u32*)tmp;
    ret = dev_lcd_show_picture(&l_strrect, &l_strPicture);
    k_free(tmp);
    return ret;
}


s32 ddi_lcd_show_pixel (u32 nX, u32 nY)
{
	return dev_lcd_show_pixel(nX, nY);
}


s32 ddi_lcd_show_line(const strLine  *lpstrLine )
{
	return dev_lcd_show_line(lpstrLine );
}

s32 ddi_lcd_show_rect(const strRect  *lpstrRect)
{
	return dev_lcd_show_rect(lpstrRect);
}



s32 ddi_lcd_extract_rect(const strRect *lpstrRect, strPicture * lpstrPic)
{
	return dev_lcd_extract_rect(lpstrRect, lpstrPic);
}

s32 ddi_lcd_ioctl(u32 nCmd, u32 lParam, u32 wParam)
{
	return dev_lcd_ioctl(nCmd, lParam, wParam);
}


s32 ddi_lcd_show_picture_file(const strRect *lpstrRect, const u8 *lpBmpName)
{
	return dev_lcd_show_picture_file(lpstrRect, lpBmpName);
}


void ddi_lcd_brush_screen(void)
{
	dev_lcd_brush_screen();
}



#else

s32 ddi_lcd_open (void)
{
	return DDI_ENODEV;
}


s32 ddi_lcd_close (void)
{
	return DDI_ENODEV;
}

s32 ddi_lcd_fill_rect (const strRect *lpstrRect, u32 nRGB)
{
	return DDI_ENODEV;
}


s32 ddi_lcd_clear_rect (const strRect *lpstrRect)
{
	return DDI_ENODEV;
}


s32 ddi_lcd_show_text(u32 nX, u32 nY, const u8* lpText)
{
	return DDI_ENODEV;
}


s32 ddi_lcd_show_picture(const strRect *lpstrRect, const strPicture * lpstrPic)
{
	return DDI_ENODEV;
}
s32 ddi_lcd_monochrome (u16 x, u16 y, u16 width, u16 height, u8 *p)
{
	return DDI_ENODEV;
}


s32 ddi_lcd_show_pixel (u32 nX, u32 nY)
{
	return DDI_ENODEV;
}


s32 ddi_lcd_show_line(const strLine  * lpstrLine )
{
	return DDI_ENODEV;
}

s32 ddi_lcd_show_rect(const strRect  * lpstrRect)
{
	return DDI_ENODEV;
}



s32 ddi_lcd_extract_rect(const strRect *lpstrRect, strPicture * lpstrPic)
{
	return DDI_ENODEV;
}

s32 ddi_lcd_ioctl(u32 nCmd, u32 lParam, u32 wParam)
{
	return DDI_ENODEV;
}


s32 ddi_lcd_show_picture_file(const strRect *lpstrRect, const u8 * lpBmpName)
{
	return DDI_ENODEV;
}


void ddi_lcd_brush_screen(void)
{
	
}


#endif


