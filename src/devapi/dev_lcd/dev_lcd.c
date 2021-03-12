/********************************Copyright ( c ) ********************************
**                  DAQU 
**                         
**                        
**
**Created By:         
** Created Date:     
** Version:        
** Description:    该文件包含LCD的驱动相关接口
  
****************************************************************************/
#include "devglobal.h"
#include "drv_lcd.h"

#if(LCD_EXIST==1)
static s32 g_lcd_fd = -1;
u8 g_lcd_dispram[LCD_MAX_ROW][LCD_MAX_COL];


#define ZIKU_BUFF_MAX   64//一个汉字点阵需要的最大缓冲, 1212汉字，最大是24字节

#define ZIKUHZ12ADD (0x480)//汉字点阵在字库中的偏移，前面放的是ASC
#define ZIKUHZ12_NAME "/mtd2/hz1212.bin"

struct _strLcdConf g_LcdConf;
//LCD的画笔颜色和背景色, 这两个不宜去掉，
//因为LCD接口保存的是RGB，如果每次都转16BIT，耗时
u16 POINT_COLOR = WHITE; //画笔颜色
u16 BACK_COLOR = BLUE;  //背景色
/****************************************************************************
**Description:        全屏显示
**Input parameters:    color LCD_COLOR_WHITE 白色 清屏
                          LCD_COLOR_BLACK 黑屏
**Output parameters: 
**Returned value:
**Created by:        pengxuebin,20170526
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_lcd_full(u8 color)
{
    u16 colIdx;
    u8 pageIdx;
    s32 i;
    
   
    if(color == 0){

        //屏控制器ST7567是132*65
        for (pageIdx = 0; pageIdx < 9; pageIdx++) {
            drv_lcd_set_ram_addr(pageIdx, 0x00);	
            for (colIdx = 0; colIdx < 132; colIdx++)
                drv_lcd_send_byte(0x00);
        }
    }else{
        for (pageIdx = 0; pageIdx < 8; pageIdx++){
            drv_lcd_set_ram_addr(pageIdx, 0x00+LCD_OFFSET);	
            //	for (colIdx = 0; colIdx < MAX_DOT_COL; colIdx++)	
            for (colIdx = 0; colIdx < LCD_MAX_COL-1; colIdx++) //600A新屏131同时对应左右8列 2014.12.22
                drv_lcd_send_byte(0xFF);
        }
    }
    

 #if 0   
  #ifdef MONO_LCD_2LINE 
    if(color == 0){

        //600A新屏控制器不是128*64，而是132*65
        for (pageIdx = 0; pageIdx < 9; pageIdx++) {
            drv_lcd_set_ram_addr(pageIdx, 0x00);	
            for (colIdx = 0; colIdx < 132; colIdx++)
                drv_lcd_send_byte(0x00);
        }
    }else{
        for (pageIdx = 0; pageIdx < LCD_MAX_ROW; pageIdx++){
            drv_lcd_set_ram_addr(pageIdx, 0x00);	
            //	for (colIdx = 0; colIdx < MAX_DOT_COL; colIdx++)	
            for (colIdx = 0; colIdx < LCD_MAX_COL-1; colIdx++) //600A新屏131同时对应左右8列 2014.12.22
                drv_lcd_send_byte(0xFF);
        }
    }
  #else
    if(color == 0){

        //屏控制器ST7567是132*65
        for (pageIdx = 0; pageIdx < 9; pageIdx++) {
            drv_lcd_set_ram_addr(pageIdx, 0x00);	
            for (colIdx = 0; colIdx < 132; colIdx++)
                drv_lcd_send_byte(0x00);
        }
    }else{
        for (pageIdx = 0; pageIdx < LCD_MAX_ROW; pageIdx++){
            drv_lcd_set_ram_addr(pageIdx, 0x00);	
            //	for (colIdx = 0; colIdx < MAX_DOT_COL; colIdx++)	
            for (colIdx = 0; colIdx < LCD_MAX_COL-1; colIdx++) //600A新屏131同时对应左右8列 2014.12.22
                drv_lcd_send_byte(0xFF);
        }
    }
  #endif
 #endif 
    if(color == 0)
    {
        memset((u8*)g_lcd_dispram, 0, LCD_MAX_ROW*LCD_MAX_COL);
    }
    else
    {
        memset((u8*)g_lcd_dispram, 0xFF, LCD_MAX_ROW*LCD_MAX_COL);
        
    }
    return 0;
}
/****************************************************************************
**Description:	    画点
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170527
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
static void dev_lcd_set_pixel(u8 color, s32 x, s32 y)
{
    if((x>=LCD_MAX_COL) || (y>=LCD_MAX_LINE))
    {
        return;
    }
    if(color)
    {
        g_lcd_dispram[y>>3][x] |= (1<<(y&0x07));
    }
    else
    {
        g_lcd_dispram[y>>3][x] &= (~(1<<(y&0x07)));
    }
}
/****************************************************************************
**Description:	    反转(inversion)
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170527
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
static void dev_lcd_inver_pixel(s32 x, s32 y)
{
    if((x>=LCD_MAX_COL) || (y>=LCD_MAX_LINE))
    {
        return;
    }
    g_lcd_dispram[y>>3][x] ^= (1<<(y&0x07));
}

/****************************************************************************
**Description:	    画横线
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170527
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
static void dev_lcd_line_draw_h_line(s32 x0, s32 y0, s32 x1)
{
    s32 i;
    
    if(x0<=x1)
    {
        for(i=x0; i<x1; i++)
        {
            dev_lcd_set_pixel(1, i, y0);
        }
    }
    else
    {
        for(i=x1; i<x0; i++)
        {
            dev_lcd_set_pixel(1, i, y0); 
        }
    }
}
/****************************************************************************
**Description:	    画竖线
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170527
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
static void dev_lcd_line_draw_v_line(s32 x0, s32 y0, s32 y1)
{
    s32 i;
    
    if(y0<=y1)
    {
        for(i=y0; i<y1; i++)
        {
            dev_lcd_set_pixel(1, x0, i);
        }
    }
    else
    {
        for(i=y1; i<y0; i++)
        {
            dev_lcd_set_pixel(1, x0, i); 
        }
    }
}
/****************************************************************************
**Description:	    画直线
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170527
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
//static 
void dev_lcd_draw_line(s32 x1, s32 y1, s32 x2, s32 y2)
{
 /*   s32 i, x, y;
    s32 ratio;
    s32 j;
    

    if(x0 == x1)
    {
        dev_lcd_line_draw_v_line(x0, y0, y1);
        return;
    }
    if(y0 == y1) 
    {
        dev_lcd_line_draw_h_line(x0, y0, x1);
        return;
    }
    
    ratio = ((y0-y1)<<10)/(x0-x1);
    if(x0 < x1)
    {
        for(i=x0; i<=x1; i++)
        {
            j = y0+((ratio*(i-x0)+512)>>10);
            dev_lcd_set_pixel(1, i, j); 
        }
    }
    else
    {
        for(i=x1; i<=x0; i++)
        {
            j = y1+((ratio*(i-x1)+512)>>10);
            dev_lcd_set_pixel(1, i, j); 
        }
    }
    */
        u16 t;
    int xerr=0,yerr=0,delta_x,delta_y,distance;
    int incx,incy,uRow,uCol;
    delta_x=x2-x1; //计算坐标增量
    delta_y=y2-y1;
    uRow=x1;
    uCol=y1;
    if(delta_x>0)
        incx=1; //设置单步方向
    else if(delta_x==0)
        incx=0;//垂直线
    else
    {
        incx=-1;
        delta_x=-delta_x;
    }
    
    if(delta_y>0)
        incy=1;
    else if(delta_y==0)
        incy=0;//水平线
    else
    {
        incy=-1;
        delta_y=-delta_y;
    }
    
    if( delta_x>delta_y)
        distance=delta_x; //选取基本增量坐标轴
    else 
        distance=delta_y;

    for(t=0; t<=distance+1; t++ ) //画线输出
    {
        dev_lcd_set_pixel(1,uRow,uCol);//画点
        xerr+=delta_x ;
        yerr+=delta_y ;

        if(xerr>distance)
        {
            xerr-=distance;
            uRow+=incx;
        }

        if(yerr>distance)
        {
            yerr-=distance;
            uCol+=incy;
        }
    }
}
u16 x_beg, x_end, y_beg, y_end;///记录本函数改动的XY范围
/**
 *@brief:      dev_lcd_show_str
 *@details:       指定左上角坐标显示字符串(仅仅改变显示缓冲，不改变LCD)
 *@param[in]   u32 nX            
               u32 nY            
               const u8* lpText  
 *@param[out]  无
 *@retval:     static
 */
static void dev_lcd_show_str(u32 nX, u32 nY, const u8* lpText)
{
    u32 str_len, str_index;
    u32 ret;
    u8 buf[ZIKU_BUFF_MAX];

    s32 h, v;
    s32 disp_x, disp_y;
    s32 disp_bx;
    u8 flg;

	u8 temptest[10];

//    strFont use_font;//读取点阵的字体，如不是生僻字，则跟CurrFont一样，如是生僻字，则不一样
    s32 res;
    strFontPra font_pra;
    strFont str_font;//设置字体

    if(lpText == NULL)
        return;
    if(nX > g_LcdConf.m_width)
        return;
    if(nY > g_LcdConf.m_height)
        return;

    str_len = strlen((const char *)lpText);
    if(str_len == 0)
        return;

    y_end = 0;
        
    font_pra.dot = buf;//赋值指针

    font_get_currtype(&str_font);
    font_open(str_font);
   
    disp_bx = nX;
    disp_x = nX;
    disp_y = nY;

    for(str_index = 0; str_index < str_len; )
    {
        if(lpText[str_index]<0x20)
        {
            str_index++;
            LCD_DEBUG("非可见字符\r\n");
            continue;
        }
        if(lpText[str_index] < 0x80)
        {   //ASCII
            flg = 0;
        }
        else
        {   //汉字
            flg = 1;
        }
        res = font_get_dot(str_font, &lpText[str_index], &font_pra);
        if(res < 0)
        {
            LCD_DEBUG("获取点阵失败\r\n");
            //uart_printf("失败\r\n");
            return;
        }
        else
        {
            str_index += res;//调整字符串偏移
        }

        if(font_pra.hv == FONT_VERTICAL)
        {
            //生僻字的显示方式 12*12字库
            //纵库 字节倒序，第一个字节第一个bit是左上角
            //BLcdDebug("纵库 %d, %d\r\n", font_pra.high, font_pra.width);
            for(h = 0; h < font_pra.width; h++)//宽
            {
                for(v = 0; v < font_pra.high; v++)//高
                {
                    if((font_pra.dot[h + (v/8)*font_pra.width] & (0x01 << (v%8))) != 0)
                    {
                        //点黑
                        if((disp_x + h) < g_LcdConf.m_width
                                && (disp_y + v) < g_LcdConf.m_height)
                        {
                            dev_lcd_set_pixel(1, disp_x + h,disp_y + v);
                        }
                    }
                    else
                    {
                        dev_lcd_set_pixel(0, disp_x + h, disp_y + v);
                    }
                }
            }
        }
        else if(font_pra.hv == FONT_HORIZONTAL)
        {
            //BLcdDebug("横库 %d, %d\r\n", font_pra.high, font_pra.width);
            for(v = 0; v < font_pra.high; v++)//高
            {
                for(h = 0; h < font_pra.width; h++)//宽
                {
                    if((font_pra.dot[h/8 + v* ((font_pra.width + 7)/8)] & (0x80 >> (h%8))) != 0)
                    {
                        //点黑
                        //uart_printf("v:%d,w:%d,b:%d,bit:%d\r\n", v, h, h/8 + v* ((cur_w + 7)/8), h%8);
                        if((disp_x + h) < g_LcdConf.m_width
                                && (disp_y + v) < g_LcdConf.m_height)
                        {
                            dev_lcd_set_pixel(1, disp_x + h,disp_y + v);
                        }

                    }
                    else
                    {
                        dev_lcd_set_pixel(0, disp_x + h, disp_y + v);  
                    }
                }
            }
        }

        disp_x += font_pra.width+flg;
        
        //记录y轴改变的最大位置
        if(y_end <  disp_y+v)
            y_end = disp_y+v;
    }
    
    //if(str_len)
    if(disp_x != disp_bx)// 未显示
    {
        //设置首字符的前一列为白点
        for(v=0; v<(font_pra.high+1); v++)
        {
             dev_lcd_set_pixel(0, disp_bx-1, disp_y + v);
        }
        //设置所有字符的底线为白点
        for(h=0; h<(disp_x - disp_bx); h++)
        {
            dev_lcd_set_pixel(0, disp_bx+h, disp_y+font_pra.high);
        }

        //---
        disp_x+=h;
        disp_y+=v;
    }

    //uart_printf("显示字符串结束\r\n");
//   k_free(buf);

    x_beg = nX;
    x_end = disp_x;
    y_beg = nY;
    //y_end = disp_y;
	
  #ifdef BLCD_TEST
    LCD_DEBUG("\r\nlcd show str: %d %d %d\r\n",x_beg,x_end,y_beg);
	for(disp_x = 0;disp_x < 8;disp_x++)
	{
	    for(disp_y = 0;disp_y < 128;disp_y++)
	    {
			LCD_DEBUG("%02x ",g_lcd_dispram[disp_x][disp_y]);
	    }
	}
	LCD_DEBUG("\r\n");
  #endif
	
    //关闭字库文件
    font_close(str_font);
}

/****************************************************************************
**Description:	    根据修改部分刷屏,
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170526
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
static void dev_lcd_brush_chg(void)
{
    u16 colIdx;
    u8 pageIdx;
 //   u8 m; 

    u16 page_beg, page_end;
    s32 i;

    
  #if 0
    if(y_end >= LCD_MAX_DOT_ROW )
        y_end = LCD_MAX_DOT_ROW-1;

    if(y_beg>=LCD_MAX_DOT_ROW)
        y_beg = LCD_MAX_DOT_ROW-1;

    if(x_beg >= MAXCOL)
        x_beg = MAXCOL - 1;
    if(x_end >= MAXCOL)
        x_end = MAXCOL - 1;
  #endif
    
    if(y_end >= 64 )
        y_end = 63;

    if(y_beg>=64)
        y_beg = 63;
        
    
    if(x_beg >= MAXCOL)
        x_beg = MAXCOL - 1;
    if(x_end >= MAXCOL)
        x_end = MAXCOL - 1;
    page_beg = y_beg/8;
    page_end = y_end/8;

    //uart_printf(" %d, %d, %d, %d\r\n", y_beg, y_end, page_beg, page_end);
    //uart_printf(" %d, %d\r\n", x_beg, x_end);
//    if(0 == dev_open_lcd())
    {                  
        for (pageIdx = page_beg; pageIdx < page_end+1; pageIdx++)
        {
            //uart_printf("pageIdx:%d\r\n", pageIdx);
            drv_lcd_set_ram_addr(pageIdx, x_beg+LCD_OFFSET);   

            for (colIdx = x_beg; colIdx <= x_end; colIdx++)
            {
                drv_lcd_send_byte(g_lcd_dispram[pageIdx][colIdx]);
            }
        }
    }
}
/****************************************************************************
**Description:	    画矩形边框
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170601
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
static void dev_lcd_draw_rectangle(s32 x1, s32 y1, s32 x2, s32 y2) 
{
    s32 tmp=0;
    //swap value:
    if(x1>x2)
    {
        tmp = x1;
        x1 = x2;
        x2=tmp;
    }
    if(y1>y2)
    {
        tmp = y1;
        y1 = y2;
        y2 = tmp;
    }
// horizontal
    dev_lcd_draw_line(x1, y1, x2 ,y1);
    dev_lcd_draw_line(x1, y2, x2 ,y2);

// vertical
    dev_lcd_draw_line(x1, y1+1, x1 ,y2);
    dev_lcd_draw_line(x2, y1+1, x2 ,y2);
}
/****************************************************************************
**Description:	    矩形填充
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170601
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
static void dev_lcd_fill_rectangle(s32 x1, s32 y1, s32 x2, s32 y2) 
{
    s32 tmp=0;
    s32 i,j;
    //swap value:
    if(x1>x2)
    {
        tmp = x1;
        x1 = x2;
        x2=tmp;
    }
    if(y1>y2)
    {
        tmp = y1;
        y1 = y2;
        y2 = tmp;
    }
    for(i=x1; i<=x2; i++)
    {
        for(j=y1; j<=y2; j++)
        {
            dev_lcd_set_pixel(1, i, j);
        }
    }
}
/****************************************************************************
**Description:	    矩形清空
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170601
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
static void dev_lcd_clear_rectangle(s32 x1, s32 y1, s32 x2, s32 y2) 
{
    s32 tmp=0;
    s32 i,j;
    //swap value:
    if(x1>x2)
    {
        tmp = x1;
        x1 = x2;
        x2=tmp;
    }
    if(y1>y2)
    {
        tmp = y1;
        y1 = y2;
        y2 = tmp;
    }
    for(i=x1; i<=x2; i++)
    {
        for(j=y1; j<=y2; j++)
        {
            dev_lcd_set_pixel(0, i, j);
        }
    }
}
/****************************************************************************
**Description:	    指定区域反显
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170601
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
static void dev_lcd_nodisp_area_rowraw(s32 x1, s32 y1, s32 x2, s32 y2) 
{
    s32 tmp=0;
    s32 i,j;
    //swap value:
    if(x1>x2)
    {
        tmp = x1;
        x1 = x2;
        x2=tmp;
    }
    if(y1>y2)
    {
        tmp = y1;
        y1 = y2;
        y2 = tmp;
    }
    for(i=x1; i<x2; i++)
    {
        for(j=y1; j<y2; j++)
        {
            dev_lcd_inver_pixel(i, j);
        }
    }
}
/****************************************************************************
**Description:	    指定矩形区域反显(立即显示)
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170601
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
static s32 dev_lcd_invert_rect(const strRect *lpstrRect)
{
    dev_lcd_nodisp_area_rowraw(lpstrRect->m_x0, lpstrRect->m_y0, lpstrRect->m_x1, lpstrRect->m_y1);
    dev_lcd_brush_screen();
    
    return DDI_OK;
}

/****************************************************************************
**Description:	    设置对比度
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170601
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
static void dev_lcd_set_contrast(s32 contrast)
{
    if(g_lcd_fd == 0)
    {
        if(contrast < 0x01)
        {
            contrast = 0x01;
        }
        else if(contrast > 0x3f)
        {
            contrast = 0x3f;
        }
        drv_lcd_set_contrast(contrast);
    }
}
/****************************************************************************
**Description:	    打开LCD设备
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170526
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
s32 dev_lcd_open(void)
{
    strFont l_strfont;
    
    if(g_lcd_fd < 0)
    {
//LCD_DEBUG("\r\n");        
        drv_lcd_init();
        memset((u8*)g_lcd_dispram, 0, LCD_MAX_ROW*LCD_MAX_COL);
        
        g_LcdConf.m_width = LCD_MAX_COL;
        g_LcdConf.m_height = LCD_MAX_LINE;
        g_LcdConf.m_linewidth = 1;//设置默认线宽为1
        
        l_strfont.m_font = 0;
        l_strfont.m_ascsize = FONT_TYPE_ASC6X12;
        l_strfont.m_nativesize = FONT_TYPE_HZ1212;
        font_set_currtype(&l_strfont);
        
        g_lcd_fd = 0;
        dev_lcd_brush_screen();
    }
    return DDI_OK;
}
/****************************************************************************
**Description:	    在指定位置显示指定大小的图片
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170526
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
static s32 dev_lcd_show_bmp(const strRect *lpstrRect, const strPicture * lpstrPic)
{
    u8 data; 
    u8 x,y;
    u8 i,j;
    u8 x_max, y_max;
    //u16 c;
    u8 siXLen, siYlen;
    u8 siX0 = lpstrRect->m_x0;
    u8 siY0 = lpstrRect->m_y0;
    u8 *pasBmpData = (u8 *)lpstrPic->m_pic;

    if((lpstrRect->m_x1 - lpstrRect->m_x0) > lpstrPic->m_width)
    {
        //BLcdDebug("区域比图片宽\r\n");
        x_max = lpstrPic->m_width;
    }
    else
    {
        //BLcdDebug("区域比图片窄\r\n");
        x_max = lpstrRect->m_x1 - lpstrRect->m_x0;
    }
        
    if((lpstrRect->m_y1 - lpstrRect->m_y0) > lpstrPic->m_height)
    {
        //BLcdDebug("区域比图片高\r\n");
        y_max = lpstrPic->m_height;
    }
    else
    {
        //BLcdDebug("区域比图片矮\r\n");
        y_max = lpstrRect->m_y1 - lpstrRect->m_y0;
    }

    if((y_max + lpstrRect->m_y0) > LCD_MAX_DOT_ROW)
    {   
        y_max = LCD_MAX_DOT_ROW - lpstrRect->m_y0;
    }

    if((x_max + lpstrRect->m_x0) > MAXCOL)
    {
        x_max = MAXCOL - lpstrRect->m_x0;
    }
    
    siXLen = lpstrPic->m_width;
    siYlen = lpstrPic->m_height; 

    x = 0;
    y = 0;

    for(i = siX0; i<siX0+siXLen; i++)
    {
        if(x>x_max)//超界
        {
            x++;
            continue;
        }
        
        for(j = siY0; j < siY0+siYlen; j++)
        {
            data = *(pasBmpData + (y/8)*siXLen + x);//取字节

            if(y>y_max)//超界
            {
                y++;
                continue;
            }
            
            if(0 == (data & (0x01<<(y%8))))
                g_lcd_dispram[j/8][i] &= ~(0x01<<(j%8));
            else
                g_lcd_dispram[j/8][i] |= (0x01<<(j%8));

            y++;
        }

        x++;
        y=0;
    }

    dev_lcd_brush_screen();

    return 0;//成功
}
/****************************************************************************
**Description:	    在指定位置显示指定大小的图片
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170526
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
static s32 dev_lcd_show_bmp_ram(u32 x, u32 y, u32 xlen, u32 ylen, const u8 *BmpFileName)
{
//    s32                f;
    BITMAPFILEHEADER    bf;
    BITMAPINFOHEADER    bi;
    unsigned long       LineBytes;
    unsigned long       ImgSize;
    unsigned long       i, j, k,m;
   unsigned long       sd256_bitmap_len;
    short               n;
    u8 buf[40];
    u8 *sd256_bitmap = NULL;    //包含前8个字节
//    u32 r,g,b,
    u32 c;
//    u16 color;
//    u8  *pal=NULL, *bmpd=NULL;
   u32  palatte=0;

//    u32 sd256_bitmap_index;
 //   u32 bmpd_index;
 //   u32 line_data_num;
//    u32 read_line_num;//每次读的行数

 //   u32 d1,a1,c1,b1;

  #if 0  
    f = ddi_vfs_open(BmpFileName,"rb");
    if (f == NULL)
    {
        return -6;
    }

    memset (buf, 0, sizeof (buf));
    n = ddi_vfs_read(buf, 14, f);
  #endif
    n = fs_read_file(BmpFileName, (u8*)buf, 14, 0);
    if(n != 14)
    {
        return -6;
    }
//dev_debug_printformat(NULL, (u8*)buf, 14);
    //bf.bfType      = (buf[0]<<8);
    bf.bfSize      = buf[2]+(((u32)buf[3])<<8)+(((u32)buf[4])<<16)+(((u32)buf[5])<<24);
    
    //bf.bfReserved1 = (((u32)buf[6]))+(((u32)buf[7])<<8);
    //bf.bfReserved2 = (((u32)buf[8]))+(((u32)buf[9])<<8);;
    bf.bfOffBits   = (((u32)buf[10]))+(((u32)buf[11])<<8)+(((u32)buf[12])<<16)+(((u32)buf[13])<<24);
    //bf.bfOffBits = (bf.bfOffBits<<16)+buf[5];


    memset (buf, 0, sizeof (buf));
    //ddi_vfs_seek (f, 14, SEEK_SET);
    //n = ddi_vfs_read(buf, 80, f);
    n = fs_read_file(BmpFileName, (u8*)buf, 40, 14);
//dev_debug_printf("%s(%d):read n=%d\r\n", __FUNCTION__, __LINE__, n);        
//if(n>0)dev_debug_printformat(NULL, (u8*)buf, n);
    bi.biSize           = (((u32)buf[0]))+(((u32)buf[1])<<8)+(((u32)buf[2])<<16)+(((u32)buf[3])<<24);
    bi.biWidth          = (((u32)buf[4]))+(((u32)buf[5])<<8)+(((u32)buf[6])<<16)+(((u32)buf[7])<<24);
    bi.biHeight         = (((u32)buf[8]))+(((u32)buf[9])<<8)+(((u32)buf[10])<<16)+(((u32)buf[11])<<24);
    bi.biPlanes         = (((u32)buf[12]))+(((u32)buf[13])<<8);
    bi.biBitCount       = (((u32)buf[14]))+(((u32)buf[15])<<8);
    bi.biCompression    = (((u32)buf[16]))+(((u32)buf[17])<<8)+(((u32)buf[18])<<16)+(((u32)buf[19])<<24);
    bi.biSizeImage      = (((u32)buf[20]))+(((u32)buf[21])<<8)+(((u32)buf[22])<<16)+(((u32)buf[23])<<24);
    bi.biXPelsPerMeter  = (((u32)buf[24]))+(((u32)buf[25])<<8)+(((u32)buf[26])<<16)+(((u32)buf[27])<<24);
    bi.biYPelsPerMeter  = (((u32)buf[28]))+(((u32)buf[29])<<8)+(((u32)buf[30])<<16)+(((u32)buf[21])<<24);
    bi.biClrUsed        = (((u32)buf[32]))+(((u32)buf[33])<<8)+(((u32)buf[34])<<16)+(((u32)buf[35])<<24);
    bi.biClrImportant   = (((u32)buf[36]))+(((u32)buf[37])<<8)+(((u32)buf[38])<<16)+(((u32)buf[39])<<24);


    LineBytes = (unsigned long) (((bi.biWidth>>5)+1)<<5) * bi.biBitCount;

    ImgSize   = (unsigned long) LineBytes  * bi.biHeight/8;

    if(bi.biBitCount != 1)  //如果不是单色图片，K200P不支持显示
        return -1;
    
    //ddi_vfs_seek(f, 54, SEEK_SET);
    //n = ddi_vfs_read(buf, 8, f);
    n = fs_read_file(BmpFileName, (u8*)buf, 8, 54);
//dev_debug_printf("%s(%d):read n=%d\r\n", __FUNCTION__, __LINE__, n);        
//if(n>0)dev_debug_printformat(NULL, (u8*)buf, n);
   
    palatte = (((u32)buf[0]))+(((u32)buf[1])<<8)+(((u32)buf[2])<<16)+(((u32)buf[3])<<24);;


    sd256_bitmap_len = ImgSize;

    if(xlen > bi.biWidth)
        xlen = bi.biWidth;

    //如果长度比图片小，
    if(ylen > bi.biHeight)
        ylen = bi.biHeight;

    k = bi.biWidth;
    if((k%8) != 0)
    {
        k=((k>>3)+1)<<3;
    }
    
    if((k%32) != 0) //注意:单色图片对齐方式(4字节对齐)
    {
        m = (k>>3);
        m %= 4;
        k = ((k>>3) + 4-m)<<3;
    }

    //ddi_vfs_seek (f, bf.bfOffBits, SEEK_SET);
    sd256_bitmap = (u8*)k_malloc(k>>3);

    for(j=0; j<bi.biHeight; j++) //图片取模:横向,左高右低
    {
        //n = ddi_vfs_read (sd256_bitmap, k>>3, f);
        n = fs_read_file(BmpFileName, sd256_bitmap, (k>>3), bf.bfOffBits+((k>>3)*j));
        
        if(j < (bi.biHeight - ylen))
        {
            continue;
        }
    
        for(i=0; i<xlen; i++)
        {           
            c = sd256_bitmap[(i>>3)]&(1 << (7-i%8));//320
            if(palatte>0)
            {
                if(c)
                {
                    dev_lcd_set_pixel(1, (x+i),(y+(bi.biHeight-1)-j)); //点黑从左上角开始显示
                }
                else
                {
                    dev_lcd_set_pixel(0, (x+i),(y+(bi.biHeight-1)-j));//点白,从左上角开始显示
                }
            }
            else
            {
                if(c)
                {
                    dev_lcd_set_pixel(0, (x+i),(y+(bi.biHeight-1)-j)); //点白,从左上角开始显示
                }
                else
                {
                    dev_lcd_set_pixel(1, (x+i),(y+(bi.biHeight-1)-j));//点黑从左上角开始显示
                }
                
            }
        }
    }

    k_free(sd256_bitmap);
 //   ddi_vfs_close (f);

    return 0;
}
/****************************************************************************
**Description:	    关闭LCD设备
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170526
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
s32 dev_lcd_close(void)
{
    g_lcd_fd = -1;
    drv_lcd_disp_off();
    drv_lcd_bl_ctl(0);
    return DDI_OK;
}
/****************************************************************************
**Description:	    刷屏
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170601
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
void dev_lcd_brush_screen(void)
{
	s32 colIdx;
	s32 pageIdx;
	s32 m;
	s32 i;
	
	
	i = 8;
    
	
    if(0 == dev_lcd_open())
    {
        for (pageIdx = 0; pageIdx < i; pageIdx++)
        {
            drv_lcd_set_ram_addr(pageIdx, 0x00+LCD_OFFSET);   

            for (colIdx = 0; colIdx < LCD_MAX_COL; colIdx++)
            {
                drv_lcd_send_byte(g_lcd_dispram[pageIdx][colIdx]);
            }
        }
		
    }
	
	
}
/****************************************************************************
**Description:	    用指定颜色填充矩形区域
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170601
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
s32 dev_lcd_fill_rect(const strRect *lpstrRect, u32 nRGB)
{
    u16 color;
    strRect *temp_lpstrRect=(strRect *)lpstrRect;

    if(temp_lpstrRect == NULL)
        return DDI_ERR;

    if(g_lcd_fd == -1)
        return DDI_EIO;


    dev_lcd_fill_rectangle(temp_lpstrRect->m_x0,temp_lpstrRect->m_y0,temp_lpstrRect->m_x1,temp_lpstrRect->m_y1);
    dev_lcd_brush_screen();
    return DDI_OK;
}
/****************************************************************************
**Description:	     用背景色填充一块区域
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170601
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
s32 dev_lcd_clear_rect (const strRect *lpstrRect)
{

    strRect *temp_lpstrRect=(strRect *)lpstrRect;


    if(temp_lpstrRect == NULL)
        return DDI_ERR;

    if(temp_lpstrRect->m_x1 >= g_LcdConf.m_width)
    {
        temp_lpstrRect->m_x1 = g_LcdConf.m_width;
    }
    
    if(temp_lpstrRect->m_y1 >= g_LcdConf.m_height)
    {
        temp_lpstrRect->m_y1= g_LcdConf.m_height;
    }

    
    if(g_lcd_fd == 0)
    {
            dev_lcd_clear_rectangle(temp_lpstrRect->m_x0,temp_lpstrRect->m_y0,temp_lpstrRect->m_x1,temp_lpstrRect->m_y1);

            #if 1
            x_beg = temp_lpstrRect->m_x0;
            x_end = temp_lpstrRect->m_x1;

            y_beg = temp_lpstrRect->m_y0;
            y_end = temp_lpstrRect->m_y1;
            
            dev_lcd_brush_chg();
            #else
            dev_lcd_brush_screen();
            #endif
            return DDI_OK;
    }
    else
    {
        return DDI_EIO;
    }

}
/****************************************************************************
**Description:	     //在指定起始位置按设定的字体属性显示文本
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170601
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
s32 dev_lcd_show_text(u32 nX, u32 nY, const u8* lpText)
{
    u8 x,y;


    if(lpText == NULL)
        return DDI_ERR;

    if(g_lcd_fd < 0)
        return DDI_EIO;

    x = (nX&0xff);
    y = (nY&0xff);

	//sxl?
//	dev_debug_printf("\r\ndev_lcd_show_text:%d %d %s",nX,nY,lpText);

    dev_lcd_show_str(x, y, (u8*)lpText);//左对齐
    dev_lcd_brush_chg();

    return DDI_OK;
}
/****************************************************************************
**Description:	     在指定矩形区域显示图形
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170601
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
s32 dev_lcd_show_picture(const strRect *lpstrRect, const strPicture * lpstrPic)
{
    
    if(g_lcd_fd < 0)
        return DDI_EIO;

    return dev_lcd_show_bmp(lpstrRect, lpstrPic);        
}
/****************************************************************************
**Description:	      使用前景色在指定位置画个点
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170601
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
s32 dev_lcd_show_pixel(u32 nX, u32 nY)
{
    u8 x,y,page;
    u16 color;

    if(g_lcd_fd < 0)
        return DDI_EIO;

    x = (nX&0xff);
    y = (nY&0xff);
    page = y/8;
    
    dev_lcd_set_pixel(1, x,y);//点黑缓冲指定点
    drv_lcd_set_ram_addr(page, x+LCD_OFFSET);
    drv_lcd_send_byte(g_lcd_dispram[page][x]);//把这个点发送的LCD
    
    return DDI_OK;
}
/****************************************************************************
**Description:	      画线
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170601
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
#if 0
//------线宽补偿方法--------------
struct _strLineWith
{
    s32 m_x0;
    s32 m_x1;
    s32 m_y0;
    s32 m_y1;
};

//竖线补偿
const struct _strLineWith line_with_v[11]=
{
    {0,  0, 0, 0},
    {0,  0, 0, 0},
    {1,  1, 0, 0},
    {-1,-1,0,0},
    {2,  2, 0, 0},
    {-2,-2,0,0},
    {3,  3, 0, 0},
    {-3,-3,0,0},
    {4,  4, 0, 0},
    {-4,-4,0,0}
};

//横线补偿
const struct _strLineWith line_with_h[11]=
{
    {0,0, 0, 0},
    {0,0, 0, 0},
    {0,0, 1, 1},
    {0,0,-1,-1},
    {0,0, 2, 2},
    {0,0,-2,-2},
    {0,0, 3, 3},
    {0,0,-3,-3},
    {0,0, 4, 4},
    {0,0,-4,-4}
};

//矩形补偿
const struct _strLineWith line_with_r[11]=
{
    { 0, 0, 0, 0},
    { 0, 0, 0, 0},
    { 1,-1, 1,-1},
    {-1, 1,-1, 1},
    { 2,-2, 2,-2},
    {-2, 2,-2, 2},
    { 3,-3, 3,-3},
    {-3, 3,-3, 3},
    { 4,-4, 4,-4},
    {-4, 4,-4, 4}
};
#endif

s32 dev_lcd_show_line(const strLine  * lpstrLine )
{
    strLine *temp_lpstrLine=(strLine  *)lpstrLine;
    u32 linewidth;
//    struct _strLineWith * line_width_fix;//线宽补偿

//    s32 x0, y0;//, x1, y1;
    
    linewidth = g_LcdConf.m_linewidth;

    if(lpstrLine == NULL)
        return DDI_ERR;
    
    if(g_lcd_fd < 0)
        return DDI_EIO;

    dev_lcd_draw_line(temp_lpstrLine->m_x0,temp_lpstrLine->m_y0,temp_lpstrLine->m_x1,temp_lpstrLine->m_y1);
    dev_lcd_brush_screen();
    return DDI_OK;
}
/****************************************************************************
**Description:	      画矩形
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170601
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
s32 dev_lcd_show_rect(const strRect  * lpstrRect)
{
    strRect *temp_lpstrRect=(strRect  *)lpstrRect;
    
   u32 linewidth;

//    s32 x0, y0, x1, y1;

    if(temp_lpstrRect == NULL)
        return DDI_ERR;


    if(g_lcd_fd < 0)
        return DDI_EIO;

    linewidth = g_LcdConf.m_linewidth;

    dev_lcd_draw_rectangle(temp_lpstrRect->m_x0,temp_lpstrRect->m_y0,temp_lpstrRect->m_x1,temp_lpstrRect->m_y1);
    dev_lcd_brush_screen();
    return DDI_OK;  
}
/****************************************************************************
**Description:	      截取指定矩形区域屏幕数据
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170601
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
s32 dev_lcd_extract_rect(const strRect *lpstrRect, strPicture * lpstrPic)
{
    u32 x, y;
    u32 x_start, x_end;
    u32 y_start, y_end;
    u32 i,j;

    u8 data;
    u8 *pasBmpData = (u8 *)lpstrPic->m_pic;
    
    if(g_lcd_fd < 0)
        return DDI_EIO;

    
    x_start = lpstrRect->m_x0;
    x_end = lpstrRect->m_x1;
    y_start = lpstrRect->m_y0;
    y_end = lpstrRect->m_y1;
    
    if (x_start > x_end)
    {
        x = x_end;
        x_end = x_start;
    }
    else
    {
        x = x_start;
    }

    if (y_start > y_end)
    {
        y = y_end;
        y_end = y_start;
    }
    else
    {
        y = y_start;
    }

    lpstrPic->m_width = x_end - x_start;
    lpstrPic->m_height = y_end - y_start;

    if((lpstrPic->m_height + lpstrRect->m_y0) > LCD_MAX_DOT_ROW)
    {   
        lpstrPic->m_height = LCD_MAX_DOT_ROW - lpstrRect->m_y0;
    }

    if((lpstrPic->m_width + lpstrRect->m_x0) > MAXCOL)
    {
        lpstrPic->m_width = MAXCOL - lpstrRect->m_x0;
    }

    for(i = x_start; i<x_start+lpstrPic->m_width; i++)
    {        
        if(x>lpstrPic->m_width)//超界
        {
            x++;
            continue;
        }
        
        for(j = y_start; j < y_start+lpstrPic->m_height; j++)
        {
            data = g_lcd_dispram[j/8][i];//取字节

            if(y>lpstrPic->m_height)//超界
            {
                y++;
                continue;
            }
        
            if(0 == (data & (0x01<<(y%8))))
                *(pasBmpData + (y/8)*lpstrPic->m_width + x) &= ~(0x01<<(j%8));
            else
                *(pasBmpData + (y/8)*lpstrPic->m_width + x) |= (0x01<<(j%8));

            y++;
        }

        x++;
        y=0;
    }
        
    return 0;
    
}



/****************************************************************************
**Description:	   ioctl
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170601
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
s32 dev_lcd_ioctl(u32 nCmd, u32 lParam, u32 wParam)
{
    s32 ret;

//    strFont *str_font;//设置字体
    strBackGround *str_background;
    

    if(g_lcd_fd < 0)
        return DDI_EIO;

    switch(nCmd)
    {
    case DDI_LCD_CTL_SET_BACKGROUND://设置指定区域背景色
        LCD_DEBUG("设置背景色\r\n");
        str_background = (strBackGround *)lParam;
        if(str_background->m_mode == 0)//纯色填充
        {
            LCD_DEBUG("纯色\r\n");
            g_LcdConf.m_backcolor = str_background->m_purecolor;
            BACK_COLOR = RGB216BIT(g_LcdConf.m_backcolor);
            ret = DDI_OK;
        }
        else if(str_background->m_mode == 1)//图片背景，G2暂时不支持
        {
            LCD_DEBUG("图片背景，G2暂不支持\r\n");
            ret = DDI_ERR;
        } 
        else
        {
            LCD_DEBUG("模式错误\r\n");
            ret = DDI_EINVAL;   
        }
        break;

    case DDI_LCD_CTL_SET_COLOR://设置前景色, 测试OK
        g_LcdConf.m_pencolor = lParam;
        POINT_COLOR = RGB216BIT(lParam);
        ret = DDI_OK;
        break;

    case DDI_LCD_CTL_SET_FONT://设置字体//已测试
        //判断参数合法性
       #if 0
        str_font = (strFont *)lParam;
        CurrFont.m_font = str_font->m_font;//字体，目前只有宋体字库
        CurrFont.m_ascsize = str_font->m_ascsize;
        CurrFont.m_nativesize = str_font->m_nativesize;
       #endif
        font_set_currtype((strFont *)lParam);
        ret = DDI_OK;
        break;

    case DDI_LCD_CTL_SET_LINEWIDTH://设置线宽
        if(lParam > 10)
        {
            ret = DDI_EINVAL;
        }
        else
        {
            LCD_DEBUG("设置线宽:%d\r\n", lParam);
            g_LcdConf.m_linewidth = lParam;
            ret = DDI_OK;
        }
        break;

    case DDI_LCD_CTL_RESOLUTION://读取屏幕分辨率
        *((u16*)lParam) = g_LcdConf.m_width;
        *((u16*)wParam) = g_LcdConf.m_height;
        ret = DDI_OK;
        break;

    case DDI_LCD_CTL_INVERT_RECT://反显指定区域

        ret = dev_lcd_invert_rect((strRect *)lParam);
        
        break;

    case DDI_LCD_CTL_BRIGHT://控制屏幕背光亮度
        if(lParam > 15)
        {
            ret = DDI_EINVAL;
        }
        else
        {
            //else 黑白屏 设置对比度
            dev_lcd_set_contrast((u8)(lParam*4));
            
            ret = DDI_OK;
        }
        break;

    case DDI_LCD_CTL_ICON://控制图标
        ret = DDI_ERR;
        break;

    case DDI_LCD_CTL_GET_DOTCOLOR://获取指定点颜色
        ret = DDI_ERR;
        break;

    case DDI_LCD_CTL_BKLIGHT://使能LCD背光 测试OK
            ret = DDI_OK;
            if(lParam == 0)//关
            {
//                dev_key_bl_dis();
//                 drv_lcd_bl_ctl(0);
            }
            else if(lParam == 1)//开
            {
//                dev_key_bl_en();
//                 drv_lcd_bl_ctl(1);
            }
            else
            {
                //参数错误
                ret = DDI_EINVAL;
            }
        break;

    case DDI_LCD_CTL_BKLIGHTTIME://设置LCD背光时间 测试OK
        if(lParam > 255)
        {
            ret = DDI_EINVAL;
        }
        else
        {
//            dev_key_bl_settime(lParam);
            ret = DDI_OK;
        }

        break;

    case DDI_LCD_CTL_GET_COLOR://获取当前前景色 // 测试ok
        *(u32*)lParam = g_LcdConf.m_pencolor;
        ret = DDI_OK;
        break;

    case DDI_LCD_CTL_BKLIGHT_CTRL://人工控制背光亮灭

            if(lParam == 0)//熄灭
            {
                drv_lcd_bl_ctl(0);
                ret = DDI_OK;
            }
            else if(lParam == 1)//点亮
            {
                drv_lcd_bl_ctl(1);
                ret = DDI_OK;
            }
            else
            {
                ret = DDI_EINVAL;   
            }

        
        break;
    case DDI_LCD_CTL_GET_FONT://获取当前字体
        font_get_currtype((strFont *)wParam);
        ret = DDI_OK;
        break;
    case DDI_LCD_CTL_GET_LCDTYPE://获取LCD类型
        *((u32*)lParam) = DDI_LCD_TYPE_MONO;
        ret = DDI_OK;
        break;

    default:
        ret = DDI_ERR;
        break;
    }

    return ret;
}
       
/****************************************************************************
**Description:	      显示图形文件
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170601
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
//蓝牙图标
#define BT_ICON_COL    6
#define BT_ICON_LINE   8
u8 const BT_ICON_tab[] = {0x44,0x6C,0x38,0xFE,0x92,0x6C};
//电池图标
#define BAT_ICON_COL   16//14
#define BAT_ICON_LINE   8
u8 const BAT_ICON_tab[] = {0x7E,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x7E,0x18,0x18};

s32 dev_lcd_show_bat_icon(const strRect *lpstrRect, u8 mode)
{
    u8 bat_icon_buf[BAT_ICON_COL];
    s32 i,j;
    u16 col_s,col_e;
    u16 line_s,line_e;
    u8 row;
    u8 n;
    u8 flg;

    if(mode>5)
    {
        return DDI_EINVAL;
    }
    if(lpstrRect!=NULL)
    {
        if((lpstrRect->m_x0>=LCD_MAX_COL)||(lpstrRect->m_y0>=LCD_MAX_LINE))
        {
            return DDI_EINVAL;
        }
    }
    memcpy(bat_icon_buf, BAT_ICON_tab, BAT_ICON_COL);
    //填充电池格数
    for(i=0; i<(mode*2); i++)
    {
        bat_icon_buf[i+2] |= 0x18;
    }
    //
    if(lpstrRect!=NULL)
    {
        col_s = lpstrRect->m_x0;
        line_s = lpstrRect->m_y0;
        col_e = col_s+BAT_ICON_COL-1;
        line_e = line_s+BAT_ICON_LINE-1;
        if(col_e>=LCD_MAX_COL)
        {
            col_e = LCD_MAX_COL-1;
        }
    }
    else
    { 
        col_s = LCD_MAX_COL-BAT_ICON_COL-2;		//电池图标的位置    5
        line_s = 0;
        col_e = col_s+BAT_ICON_COL-1;
        line_e = line_s+BAT_ICON_LINE-1;
    }
    row = line_s>>3;
    n = line_s&0x07;
    if((row==(LCD_MAX_ROW-1))||(n==0))
    {
        flg = 0;
    }
    else
    {
        flg = 1;
    }  
    for(i=col_s,j=0; i<=col_e; i++,j++)
    { 
        g_lcd_dispram[row][i] &= ~(((1<<(8-n))-1)<<n);
        g_lcd_dispram[row][i] |= (bat_icon_buf[j]<<n);
        if(flg)
        {
            g_lcd_dispram[row+1][i] &= ~((1<<n)-1);
            g_lcd_dispram[row+1][i] |= (bat_icon_buf[j]>>(8-n));
        }
    }
   
    dev_lcd_brush_screen();
    return DDI_OK;
}
s32 dev_lcd_show_bt_icon(const strRect *lpstrRect,u8 mode)
{
    u8 bt_icon_buf[BT_ICON_COL];
    s32 i,j;
    u16 col_s,col_e;
    u16 line_s,line_e;
    u8 row;
    u8 n;
    u8 flg;

    if(lpstrRect!=NULL)
    {
        if((lpstrRect->m_x0>=LCD_MAX_COL)||(lpstrRect->m_y0>=LCD_MAX_LINE))
        {
            return DDI_EINVAL;
        }
    }
    memcpy(bt_icon_buf, BT_ICON_tab, BT_ICON_COL);
    
    //
    if(lpstrRect!=NULL)
    {
        col_s = lpstrRect->m_x0;
        line_s = lpstrRect->m_y0;
        col_e = col_s+BT_ICON_COL-1;
        line_e = line_s+BT_ICON_LINE-1;
        if(col_e>=LCD_MAX_COL)
        {
            col_e = LCD_MAX_COL-1;
        }
    }
    else
    {
        col_s = 5;
        line_s = 0;
        col_e = col_s+BT_ICON_COL-1;
        line_e = line_s+BT_ICON_LINE-1;
    }
    row = line_s>>3;
    n = line_s&0x07;
    if((row==(LCD_MAX_ROW-1))||(n==0))
    {
        flg = 0;
    }
    else
    {
        flg = 1;
    }   
    for(i=col_s,j=0; i<=col_e; i++,j++)
    { 
        g_lcd_dispram[row][i] &= ~(((1<<(8-n))-1)<<n);
        if(mode)
        {
            g_lcd_dispram[row][i] |= (bt_icon_buf[j]<<n);
        }
        if(flg)
        {
            g_lcd_dispram[row+1][i] &= ~((1<<n)-1);
            if(mode)
            {
                g_lcd_dispram[row+1][i] |= (bt_icon_buf[j]>>(8-n));
            }
        }
    }
    dev_lcd_brush_screen();
    return DDI_OK;
}

s32 dev_lcd_show_picture_file(const strRect *lpstrRect, const u8 * lpBmpName)
{
    s32 ret;

    if(g_lcd_fd < 0)
        return DDI_EIO;
//dev_debug_printf("%s:%s\r\n", __FUNCTION__, lpBmpName);
                  
    if(0 == strcmp(lpBmpName, BT_ICON_FLNAME0))
    {
        return dev_lcd_show_bt_icon(lpstrRect, 0);
    }
    else if(0 == strcmp(lpBmpName, BT_ICON_FLNAME1))
    {
        return dev_lcd_show_bt_icon(lpstrRect, 1);
    }
    else if(0 == memcmp(lpBmpName, BAT_ICON_FLNAME, BAT_ICON_FLNAME_LEN))
    {
        if(strlen(lpBmpName) == (BAT_ICON_FLNAME_LEN+1))
        {
            if((lpBmpName[BAT_ICON_FLNAME_LEN]>=0x30)
            && (lpBmpName[BAT_ICON_FLNAME_LEN]<=0x35))
            {
                return dev_lcd_show_bat_icon(lpstrRect, lpBmpName[BAT_ICON_FLNAME_LEN]-0x30);
            }
        }
        
    }
    ret = dev_lcd_show_bmp_ram(lpstrRect->m_x0, lpstrRect->m_y0,
                        lpstrRect->m_x1 - lpstrRect->m_x0, lpstrRect->m_y1-lpstrRect->m_y0,
                        lpBmpName);
    if(ret < 0)
    {
        return ret;
    }
    dev_lcd_brush_screen();

    return DDI_OK;
}
//定义起始位置
#define LCD_BATTERY_OFFSET_X   38
#define LCD_BATTERY_OFFSET_Y   16
void dev_lcd_disp_battary(u8 batnum)
{
    u8 x,y;
    u8 i,j,k;

    if(batnum > 5)
    {
        //大于5格，不显示
        return;
    }
    //画框
    //画上、下边
    for(i=0; i<46; i++)
    {
        dev_lcd_set_pixel(1, LCD_BATTERY_OFFSET_X+i, LCD_BATTERY_OFFSET_Y);
        dev_lcd_set_pixel(1, LCD_BATTERY_OFFSET_X+i, LCD_BATTERY_OFFSET_Y+1);
        dev_lcd_set_pixel(1, LCD_BATTERY_OFFSET_X+i, LCD_BATTERY_OFFSET_Y+23);
        dev_lcd_set_pixel(1, LCD_BATTERY_OFFSET_X+i, LCD_BATTERY_OFFSET_Y+24);
    }
    //画左右边
    for(i=2; i<(25-2); i++)
    {
        dev_lcd_set_pixel(1, LCD_BATTERY_OFFSET_X, LCD_BATTERY_OFFSET_Y+i);
        dev_lcd_set_pixel(1, LCD_BATTERY_OFFSET_X+1, LCD_BATTERY_OFFSET_Y+i);
        dev_lcd_set_pixel(1, LCD_BATTERY_OFFSET_X+44, LCD_BATTERY_OFFSET_Y+i);
        dev_lcd_set_pixel(1, LCD_BATTERY_OFFSET_X+45, LCD_BATTERY_OFFSET_Y+i);
    }
    //画电池正极
    for(i=46; i<51; i++)
    {
        dev_lcd_set_pixel(1, LCD_BATTERY_OFFSET_X+i, LCD_BATTERY_OFFSET_Y+6);
        dev_lcd_set_pixel(1, LCD_BATTERY_OFFSET_X+i, LCD_BATTERY_OFFSET_Y+7);
        dev_lcd_set_pixel(1, LCD_BATTERY_OFFSET_X+i, LCD_BATTERY_OFFSET_Y+17);
        dev_lcd_set_pixel(1, LCD_BATTERY_OFFSET_X+i, LCD_BATTERY_OFFSET_Y+18);
    }
    for(i=8; i<17; i++)
    {
        dev_lcd_set_pixel(1, LCD_BATTERY_OFFSET_X+49, LCD_BATTERY_OFFSET_Y+i);
        dev_lcd_set_pixel(1, LCD_BATTERY_OFFSET_X+50, LCD_BATTERY_OFFSET_Y+i);
    }
    //去掉两个点
    dev_lcd_set_pixel(0, LCD_BATTERY_OFFSET_X+50, LCD_BATTERY_OFFSET_Y+6);
    dev_lcd_set_pixel(0, LCD_BATTERY_OFFSET_X+50, LCD_BATTERY_OFFSET_Y+18);
    //增加两个点
    dev_lcd_set_pixel(1, LCD_BATTERY_OFFSET_X+48, LCD_BATTERY_OFFSET_Y+8);
    dev_lcd_set_pixel(1, LCD_BATTERY_OFFSET_X+48, LCD_BATTERY_OFFSET_Y+16);
    
    //画电池格数
    x=LCD_BATTERY_OFFSET_X+4;
    y = LCD_BATTERY_OFFSET_Y+4;
    for(k=0; k<batnum; k++)
    {
        for(i=0; i<6; i++)
        {
            for(j=0; j<17; j++)
            {
                dev_lcd_set_pixel(1, x+i, y+j);
            }
        }
        x+=8;
    }
    
}
//=================================================
//以下是LCD旧接口
/****************************************************************************
**Description:	      画上\下划线
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170601
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
static void dev_lcd_write_line(u8 row, u8 head, u8 tail)
{
    s32 i;
    s32 page;
    u8 mask;
    
    switch(row)
    {
  #ifdef MONO_LCD_2LINE  
    case 0:
        page = 0;
        mask = 0x01;
        for(i=head; i<tail; i++)
        {
            g_lcd_dispram[0][i] |= 0x01;
        }
        break;
    case 1:
        page = 1;
        mask = 0x10;
        break;
  #else
    case 0:
        page = 0;
        mask = 0x01;
        break;
    case 1:
        page = 1;
        mask = 0x10;
        break;
    case 2:
        page = 3;
        mask = 0x02;
        break;
    case 3:
        page = 4;
        mask = 0x40;
        break;
    case 4:
        page = 6;
        mask = 0x08;
        break;
    case 5:
        page = 7;
        mask = 0x80;
        break;
    
  #endif 
    default:
        return;
//        break;
    }
    for(i=head; i<tail; i++)
    {
        g_lcd_dispram[page][i] |= mask;
    }
}
/****************************************************************************
**Description:	      填充显示内存(12点阵数据,需要插补),字库的存取采用读文件方式
**Input parameters:	
          rowid 页号(0-4)
          colid 列号(0-128)
          *str 要显示的字符串
          atr 显示属性
          {
            FDISP 正显
            NOFDISP 反显
            LDISP|CDISP|RDISP 显示的起始位置 左|中|右
            INCOL 汉字显示插入一列
          }
**Output parameters: 
**Returned value:  
**Created by:		pengxuebin,20170601
          注意，RDISP 时禁止使用INCOL
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
void dev_lcd_fill_rowram(u8 rowid, u8 colid, u8 *str, u8 atr)
{ 
    s32 i,digcount;
    u32 temp5;
    u8 tmpdisram[2][132],temp1;   //sxl?
    u32 zikuoffset;
    int fd;
    s32 ret;

	#ifdef BLCD_TEST
	u32 tempi;
	u32 tempj;
    u8 temp[128];
	#endif

    union ConverData
    {
        u8 s[24];
        u16 z[12];
    } dot;

    u8 no, t_atr;
    u8 Hzlen;
//dev_debug_printf("str=%s\r\n", str);    

  #if 0  
    fd = ddi_vfs_open(ZIKUHZ12_NAME, 0);
    if(fd == 0)
    {
        BLcdDebug("无显示字库\r\n");
    }
  #endif  
    if (colid >= MAXCOL) colid = 0;//纠错128
    if (rowid >= MAXROW)
    {
        //rowid = 0;//5
        return;
    }

    no = strlen((const char *)str);

    if (no > MAXCHAR)
        no = MAXCHAR;

    temp1 = colid;         //列数

    Hzlen = 0;             //汉字个数

    for (i = 0; i < no; )
    {
        if (str[i] < 0x80)   //ASCII码表12*6
        {
            temp1 += 6;
            if (temp1  > MAXCOL)
                break;
        }
        else                 //汉字，12*12
        {
            temp1 += 6;

            if (temp1 > MAXCOL) // 2011.09.06 hxj add
                break;

            i ++;

            if (atr & INCOL)
            {
                if (i < 16)
                    temp1 += 7;
                else
                    temp1 += 6;
            }
            else
                temp1 += 6;

            if (temp1 > MAXCOL)
                break;

            Hzlen ++;
        }
        i ++;
    }

    no = i;
    if (atr & INCOL)
    {
        if (colid + no * 6 + Hzlen - MAXCOL > 0 )
            colid = 0;//纠错
    }
    else
    {
        if (colid + no * 6 - MAXCOL > 0)
            colid = 0;//纠错
    }

    t_atr = atr & 0x1C;//(LDISP | CDISP | RDISP)
    switch(t_atr)
    {
    case CDISP://居中
        if (atr & INCOL)
        {
            if (colid + no * 6 + Hzlen - MAXCOL > 0 )
                colid = 0;
            else
                colid += (MAXCOL - (colid + no * 6 + Hzlen))/2;
        }
        else
            colid += (MAXCOL - (colid + no * 6))/2;
        break;
        
    case RDISP://右对齐
        if (atr & INCOL)
            colid = MAXCOL  - (no *6 + Hzlen);
        else
            colid = MAXCOL  - no *6;
        break;
        
    default://左对齐
        break;
    }
    
    colid += OFFSET;

  #if 0 
    switch(rowid)
    {
    case DISPAGE1://0
        for(i = 0; i < 2; i ++)
        {
            for(digcount = 0; digcount < 128; digcount ++)
            {
                tmpdisram[i][digcount] = g_lcd_dispram[i][digcount];
            }
        }
        break;
    case DISPAGE2://1
        for(digcount = 0; digcount < 128; digcount ++)
        {
            tmpdisram[0][digcount] = g_lcd_dispram[1][digcount]>>5;
            tmpdisram[0][digcount]|= g_lcd_dispram[2][digcount]<<3;
            tmpdisram[1][digcount] = g_lcd_dispram[2][digcount]>>5;
            tmpdisram[1][digcount]|= g_lcd_dispram[3][digcount]<<3;
        }
        break;   
  #ifndef MONO_LCD_2LINE       
    case DISPAGE3://2
        for(digcount = 0; digcount < 128; digcount ++)
        {
            tmpdisram[0][digcount] = g_lcd_dispram[3][digcount]>>2;
            tmpdisram[0][digcount]|= g_lcd_dispram[4][digcount]<<6;
            tmpdisram[1][digcount] = g_lcd_dispram[4][digcount]>>2;
        }
        break;
    case DISPAGE4://3
        for(digcount = 0; digcount < 128; digcount ++)
        {
            tmpdisram[0][digcount] = g_lcd_dispram[4][digcount]>>7;
            tmpdisram[0][digcount]|= g_lcd_dispram[5][digcount]<<1;
            tmpdisram[1][digcount] = g_lcd_dispram[5][digcount]>>7;
            tmpdisram[1][digcount]|= g_lcd_dispram[6][digcount]<<1;
        }
        break;
    case DISPAGE5://4
        for(digcount = 0; digcount < 128; digcount ++)
        {
            tmpdisram[0][digcount] = g_lcd_dispram[6][digcount]>>4;
            tmpdisram[0][digcount]|= g_lcd_dispram[7][digcount]<<4;
            tmpdisram[1][digcount] = g_lcd_dispram[7][digcount]>>4;
        }
        break;
  #endif      
    default:
        break;
    }

    temp1 = colid;
    for(digcount = 0; digcount < no; digcount ++)
    {
        if (str[digcount] < 0x80)
        {
            temp5 = (str[digcount] - 0x20) * 12;
            //ddi_vfs_seek(fd, temp5, 0);
            //ddi_vfs_read(dot.s, 12,fd);
            fs_read_file(ZIKUHZ12_NAME, dot.s, 12, temp5);
            if ((atr & NOFDISP) != NOFDISP)
            {
                for(i = 0; i < 6; i ++)
                {
                    tmpdisram[0][temp1 + i]= dot.s[i];//ASCII[temp5+i];
                    tmpdisram[1][temp1 + i] &= 0xf0;
                    tmpdisram[1][temp1 + i] |= dot.s[6 + i];//ASCII[temp5+i + 6];
                }
            }
            else
            {
                for(i = 0; i < 6; i ++)
                {
                    tmpdisram[0][temp1 + i]= 0xff - dot.s[i];//ASCII[temp5+i];
                    tmpdisram[1][temp1 + i] &= 0xf0;
                    tmpdisram[1][temp1 + i] |= 0xf - dot.s[6 + i];//ASCII[temp5+i + 6];
                }
            }
            temp1 += 6;
        }
        else
        {
            if (temp1 > OFFSET + MAXCOL - 12) //if (digcount > (MAXCHAR - 2))
            {
                ;
            }
            else
            {
              #if 0  
                if(str [digcount + 1] >= 0x30  && str [digcount + 1] <= 0x39)
                {
                    zikuoffset = gt(str [digcount +2 ], str [digcount + 3], str [digcount ], str [digcount + 1]);
                    digcount ++;
                    digcount ++;
                }
                else
                {
                    zikuoffset = gt(str [digcount], str [digcount + 1], 0, 0);
                }
              #else
                //ZIKUHZ12_NAME采用12*12的GB2312字库
                zikuoffset = gt_2312(str [digcount], str [digcount + 1], 0, 0);
                
              #endif  
                temp5 = zikuoffset*24 + ZIKUHZ12ADD;
                //ddi_vfs_seek(fd, temp5, 0);
                //ddi_vfs_read(dot.s, 24, fd);
                #ifdef BLCD_TEST
				
				memset(temp,0,sizeof(temp));
				memcpy(temp,&str [digcount],2);
				LCD_DEBUG("\r\nread character:%s\r\n",temp);
				
				#endif
				
                fs_read_file(ZIKUHZ12_NAME, dot.s, 24, temp5);

				#ifdef BLCD_TEST
				for(tempi = 0;tempi < 24;tempi++)
				{
					LCD_DEBUG("%02x ",dot.s[tempi]);
				}
				LCD_DEBUG("\r\n");
				#endif


                if ((atr & NOFDISP) != NOFDISP)//正显
                {
                    for(i = 0; i < 12; i ++)
                    {
                        tmpdisram[0][temp1 + i]= dot.s[i];
                        tmpdisram[1][temp1 + i] &= 0xf0;
                        tmpdisram[1][temp1 + i] |= dot.s[12 + i];
                    }
                }
                else
                {
                    for(i = 0; i < 12; i ++)
                    {
                        tmpdisram[0][temp1 + i]= 0xff - dot.s[i];
                        tmpdisram[1][temp1 + i] &= 0xf0;
                        tmpdisram[1][temp1 + i] |= 0xf - dot.s[12 + i];
                    }
                }
                if ((atr & INCOL) == INCOL)
                {
                    if (digcount < 16)
                    {
                        if ((atr & NOFDISP) == NOFDISP)         //反显
                        {
                            tmpdisram[0][temp1 + 12]= 0xff ;
                            tmpdisram[1][temp1 + 12] &= 0xf0;
                            tmpdisram[1][temp1 + 12] |= 0xf;
                        }
                        temp1 += 13;
                    }
                    else
                    {
                        temp1 += 12;
                    }
                }
                else
                {
                    temp1 += 12;
                }
            }
            digcount ++;
        } 
    }
    
    //这里添加边框的处理
    if(atr & SIDELINE)//有左右框
    {
        temp1 += 2;
        tmpdisram[0][temp1-1] = 0xff;
        tmpdisram[1][temp1-1] = 0x0f;
        for(i = temp1-2; i > colid; i --)
        {
            tmpdisram[0][i] = tmpdisram[0][i-1];
            tmpdisram[1][i] = tmpdisram[1][i-1];
        }
        tmpdisram[0][colid] = 0xff;
        tmpdisram[1][colid] = 0x0f;
    }

    switch(rowid)
    {
    case DISPAGE1:
        for(i = 0; i < 2; i ++)
            for(digcount = 0; digcount < 128; digcount ++)
                g_lcd_dispram[i][digcount] = tmpdisram[i][digcount];
        break;
    case DISPAGE2:
        for(digcount = 0; digcount < 128; digcount ++)
        {
            g_lcd_dispram[1][digcount] &= 0x1f;
            g_lcd_dispram[1][digcount] |= tmpdisram[0][digcount]<<5;
            g_lcd_dispram[2][digcount] = tmpdisram[0][digcount]>>3;
            g_lcd_dispram[2][digcount] |= tmpdisram[1][digcount]<<5;
            g_lcd_dispram[3][digcount] &= 0xfc;
            g_lcd_dispram[3][digcount] |= tmpdisram[1][digcount]>>3;
        }
        break;
  #ifndef MONO_LCD_2LINE  
    case DISPAGE3:
        for(digcount = 0; digcount < 128; digcount ++)
        {
            g_lcd_dispram[3][digcount] &= 0x03;   //0831
            g_lcd_dispram[3][digcount] |= tmpdisram[0][digcount]<<2;
            g_lcd_dispram[4][digcount] &= 0xc0;
            g_lcd_dispram[4][digcount] |= tmpdisram[0][digcount]>>6;
            g_lcd_dispram[4][digcount] |= tmpdisram[1][digcount]<<2;
        }
        break;
    case DISPAGE4:
        for(digcount = 0; digcount < 128; digcount ++)
        {
            g_lcd_dispram[4][digcount] &= 0x7f;    //0831
            g_lcd_dispram[4][digcount]|= tmpdisram[0][digcount]<<7;
            g_lcd_dispram[5][digcount] = tmpdisram[0][digcount]>>1;
            g_lcd_dispram[5][digcount]|= tmpdisram[1][digcount]<<7;
            g_lcd_dispram[6][digcount]&= 0xf8;
            g_lcd_dispram[6][digcount]|= tmpdisram[1][digcount]>>1;
        }
        break;
    case DISPAGE5:
        for(digcount = 0; digcount < 128; digcount ++)
        {
            g_lcd_dispram[6][digcount] &= 0x0f;  //0831
            g_lcd_dispram[6][digcount]|= tmpdisram[0][digcount]<<4;
            g_lcd_dispram[7][digcount] = tmpdisram[0][digcount]>>4;
            g_lcd_dispram[7][digcount]|= tmpdisram[1][digcount]<<4;
        }
        break;
  #endif      
    default:
        break;
    }


	#ifdef BLCD_TEST
	LCD_DEBUG("\r\ndispbuf:\r\n");
	for(tempi = 0;tempi < 8;tempi++)
	{
	    for(tempj = 0;tempj<128;tempj++)
	    {
			LCD_DEBUG("%02x ",gDispRam[tempi][tempj]);
	    }
	}
	LCD_DEBUG("\r\n");
	#endif

    //画上下线
    if(atr&OVERLINE)
    {
        dev_lcd_write_line(rowid, colid, temp1);
    }
    if(atr&DOWNLINE)
    {
        dev_lcd_write_line(rowid+1, colid, temp1);
    }
  #else
    dev_lcd_show_str(colid, rowid*13, str);
  #endif  
}
/****************************************************************************
**Description:	      清除显示内存中某一行起始列之间的内容
**Input parameters:	
                    row 行号(0-4)
                    startCol 起始列 (0-128)
                    endCol 终止列(0-128) (endCol > startCol)
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170601
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
static void dev_lcd_clear_rowram_at(u8 row, u8 startCol, u8 endCol)
{
    u8 i;
    if (startCol < endCol)
    {
        startCol += OFFSET;
        endCol += OFFSET;
        switch(row)
        {
        case DISPAGE1:
            for (i = startCol; i < endCol; i ++)
                g_lcd_dispram[0][i] = 0x0;
            for (i = startCol; i < endCol; i ++)
                g_lcd_dispram[1][i] &= 0xe0;
            break;
        case DISPAGE2:
            for (i = startCol; i < endCol; i ++)
                g_lcd_dispram[1][i] &= 0x1f;
            for (i = startCol; i < endCol; i ++)
                g_lcd_dispram[2][i] &= 0x0;
            for (i = startCol; i < endCol; i ++)
                g_lcd_dispram[3][i] &= 0xfc;
            break;
  #ifndef MONO_LCD_2LINE  
        case DISPAGE3:
            for (i = startCol; i < endCol; i ++)
                g_lcd_dispram[3][i] &= 0x03;
            for (i = startCol; i < endCol; i ++)
                g_lcd_dispram[4][i] &= 0x80;
            break;
        case DISPAGE4:
            for (i = startCol; i < endCol; i ++)
                g_lcd_dispram[4][i] &= 0x7f;
            for (i = startCol; i < endCol; i ++)
                g_lcd_dispram[5][i] &= 0x0;
            for (i = startCol; i < endCol; i ++)
                g_lcd_dispram[6][i] &= 0xf0;
            break;
        case DISPAGE5:
            for (i = startCol; i < endCol; i ++)
                g_lcd_dispram[6][i] &= 0x0f;
            for (i = startCol; i < endCol; i ++)
                g_lcd_dispram[7][i] &= 0x0;
            break;
     #endif       
        default:
            break;
        }
    }
}
/****************************************************************************
**Description:	      清除显示内存中某一行起始列之间的内容
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170601
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
static void dev_lcd_clear_disp_ram(void)
{
    u8 i, j;
    
    for(i = 0; i < LCD_MAX_ROW; i++)
    {
       for(j = 0; j < LCD_MAX_COL; j++)
       {
          g_lcd_dispram[i][j] = 0;   
       }    
    }   
}
/****************************************************************************
**Description:	     清除显示内存
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170601
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
void dev_lcd_clear_ram(void)
{
    dev_lcd_clear_disp_ram();
}
/****************************************************************************
**Description:	     清屏
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170601
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
void dev_lcd_clear_screen(void)
{
    if(g_lcd_fd == 0)
    {
        dev_lcd_clear_ram();
        dev_lcd_brush_screen();
    }
}
/****************************************************************************
**Description:	     清除显示内存中某一行的内容
**Input parameters:	 row 行号(0-4)
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170601
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
void dev_lcd_clear_rowram(u8 row)
{
    dev_lcd_clear_rowram_at(row, 0, LCD_MAX_COL);
}
/****************************************************************************
**Description:	     先擦除该行原先内容，在LCD上某一行,某一列(col以列为单位)显示数据
**Input parameters:	 
          row 页号
          col  列号
          *str 显示字符串
          atr  显示属性
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170601
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
void dev_lcd_disp_row(u8 row, u8 col, void *str,u8 atr)
{    
    if(g_lcd_fd == 0)
    {
        dev_lcd_clear_rowram(row);
        dev_lcd_fill_rowram(row, col, (u8 *)str, atr);
        dev_lcd_brush_screen();
    }
}
/****************************************************************************
**Description:	     清除显示内存中某一行的内容
**Input parameters:	 row 行号(0-4)
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170601
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
void dev_lcd_clear_row(u8 row)
{
    if(g_lcd_fd == 0)
    {
        dev_lcd_clear_rowram(row);
        dev_lcd_brush_screen();
    }
}

/****************************************************************************
**Description:	     LCD唤醒
**Input parameters:	 
**Output parameters: 
**Returned value:
**Created by:		cgj,20190417
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
void dev_lcd_resume(void)
{
    if(g_lcd_fd == 0)
    {
    	drv_lcd_bl_ctl(1);
		drv_lcd_disp_on();
		
    }
}


/****************************************************************************
**Description:	     LCD休眠
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		cgj,20190417
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
void dev_lcd_suspend(void)
{
    if(g_lcd_fd == 0)
    {
    	
		drv_lcd_disp_off();
		drv_lcd_bl_ctl(0);
    }
}
#if 0
void dev_lcd_test_1(void)
{
    s32 i,j;
    
    for(i=0; i<LCD_MAX_ROW; i++)
    {
        if(i&0x01)
        {
            memset(g_lcd_dispram[i], 0, LCD_MAX_COL);
        }
        else
        {
            memset(g_lcd_dispram[i], 0xFF, LCD_MAX_COL);
        }
        for(j=0; j<(LCD_MAX_COL-7); j+=16)
        {
            if(i&0x01)
            {
                memset(&g_lcd_dispram[i][j], 0xFF, 8);
            }
            else
            {
                memset(&g_lcd_dispram[i][j], 0x00, 8);
            }
        }
    }
    dev_lcd_brush_screen();
    
}
#endif

#else

s32 dev_lcd_open(void)
{
//    drv_lcd_pininit();
//    drv_lcd_A0_ctl(1);
    return DDI_ENODEV;
}
s32 dev_lcd_close(void)
{
    return DDI_ENODEV;
}

void dev_lcd_brush_screen(void)
{
}
s32 dev_lcd_ioctl(u32 nCmd, u32 lParam, u32 wParam)
{
    return DDI_ENODEV;
}
void dev_lcd_fill_rowram(u8 rowid, u8 colid, u8 *str, u8 atr)
{
}
void dev_lcd_clear_ram(void)
{
}
void dev_lcd_clear_screen(void)
{
}
void dev_lcd_clear_rowram(u8 row)
{
}
void dev_lcd_disp_row(u8 row, u8 col, void *str,u8 atr)
{
}
void dev_lcd_clear_row(u8 row)
{
}
#endif


