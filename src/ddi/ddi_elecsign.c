#include "ddi_elecsign.h"
#include "devglobal.h"
#include "devapi/dev_font/dev_font.h"

//#define DDI_ELEC_SIGN_ENABLE 1
#ifdef DDI_ELEC_SIGN_ENABLE

#define LCD_WIDTH 128
#define LCD_HEIGHT 64
typedef struct ts_sample pen_point_struct;

typedef struct _SignatureBitmap
{
	uint32_t lcd_width;
	uint32_t lcd_height;
	uint32_t canvas_x;
	uint32_t canvas_y;
	uint32_t canvas_width;
	uint32_t canvas_height;
	uint32_t canvas_pixel_width;
	uint8_t* buffer;
	uint32_t buffer_size;
	int32_t line_width;
	int32_t cur_x;
	int32_t cur_y;
}SignatureBitmap;
static SignatureBitmap *g_signature_bitmap = NULL;

SignatureBitmap* signature_bitmap_create(uint32_t lcd_width, uint32_t lcd_height, 
		uint32_t canvas_x, uint32_t canvas_y, uint32_t canvas_width, uint32_t canvas_height)
{
	SignatureBitmap* thiz;

	if(lcd_width == 0 || lcd_height == 0 || canvas_width == 0 || canvas_height == 0)
	{
		return NULL;
	}
	if(canvas_x + canvas_width > lcd_width || canvas_y + canvas_height > lcd_height)
	{
		return NULL;
	}
	
	canvas_width >>= 3;
	canvas_width <<= 3;
	if(canvas_height >= lcd_height)
	{
		canvas_height = lcd_height;
	}

	thiz = k_malloc(sizeof(SignatureBitmap));
	if(thiz == NULL) 
	{
		return NULL;
	}
	thiz->lcd_width = lcd_width;
	thiz->lcd_height = lcd_height;
	thiz->canvas_x = canvas_x;
	thiz->canvas_y = canvas_y;
	thiz->canvas_width = canvas_width / 8;
	thiz->canvas_height = canvas_height;
	thiz->canvas_pixel_width = canvas_width;
	thiz->line_width = 3;
	thiz->buffer_size = thiz->canvas_width * thiz->canvas_height;
	thiz->buffer = (uint8_t *)k_malloc(thiz->buffer_size);
	if(thiz->buffer == NULL)
	{
		k_free(thiz);
		
		return NULL;
	}
	dev_debug_printf("%s %d %d %d %d\r\n", __func__, thiz->canvas_x, thiz->canvas_y, thiz->canvas_pixel_width, thiz->canvas_height);
	thiz->cur_x = 0xffffffff;
	thiz->cur_y = 0xffffffff;	
	memset(thiz->buffer, 0x0, thiz->buffer_size);
	
	return thiz;
}

void signature_bitmap_fill_pixel(SignatureBitmap* thiz, uint32_t x, uint32_t y)
{
	uint32_t index = 0;
	if(thiz == NULL)
	{
		return;
	}
	if(x / 8 >= thiz->canvas_width || y >= thiz->canvas_height)
	{
		dev_debug_printf("point out of range (%d, %d)\r\n", x / 8, y);
		return;
	}
	index = (uint32_t)((y * thiz->canvas_width * 8 + x) / 8);
	//printf("set point (%d, %d) => (%d, %d)\r\n", x, y, index, x%8);
	thiz->buffer[index] |= (0x80 >> (x % 8));
}

int32_t signature_bitmap_draw_text(
	SignatureBitmap* thiz, uint32_t nX, uint32_t nY, const uint8_t *lpText)
{
    uint32_t str_len, str_index;
    uint32_t ret;
    uint8_t *buf;
    int32_t h, v;
    int32_t disp_x, disp_y;
    int32_t native_h, asc_h;
    int32_t h_offset;
    strFont use_font;//读取点阵的字体，如不是生僻字，则跟CurrFont一样，如是生僻字，则不一样
    int32_t res;
    strFontPra font_pra;

	//ddi_lcd_show_text(thiz->canvas_x + nX, thiz->canvas_y + nY, lpText);

	//dev_debug_printf("\r\ndisp = %d %d %d %d\r\n",nX,nY,LcdConf.m_width,LcdConf.m_height);
    if(lpText == NULL) 
	{
		dev_debug_printf("%s cannot show empty text\r\n", __func__);
        return -1;
    }
    if(nX > thiz->canvas_width * 8)
    {
   		dev_debug_printf("%s cannot x is too large, cannot bigger than %d \r\n", __func__, thiz->canvas_width * 8);
        return -1;
    }
    if(nY > thiz->canvas_height)
    {
   		dev_debug_printf("%s cannot y is too large, cannot bigger than %d \r\n", __func__, thiz->canvas_height);
        return -1;
    }

    buf = (u8*)k_malloc(512);
    if(buf == NULL)
    {
  		dev_debug_printf("%s malloc font buffer failed \r\n", __func__);
        return -1;
    }

    font_pra.dot = buf;//赋值指针

    font_open(CurrFont);

    asc_h = FontAscPara[CurrFont.m_ascsize].high;
    native_h = FontHzPara[CurrFont.m_nativesize].high;
   
    str_len = strlen((const char *)lpText);
    disp_x = nX;
    disp_y = nY;

    for(str_index = 0; str_index < str_len; )
    {
        if(lpText[str_index]<0x20)
        {
            str_index++;
            dev_debug_printf("%s invisible text\r\n", __func__);
            continue;
        }
        
        res = font_get_dot(CurrFont, &lpText[str_index], &font_pra);
        if(res < 0)
        {
            dev_debug_printf("%s get dot failed\r\n", __func__);
            k_free(buf);
            return -1;
        }
        else
        {
            str_index += res;//调整字符串偏移
        }

        //add by hecaiwen, 2014.09.19,计算不同字体底部对齐的偏移量
        if(font_pra.high < native_h)
            h_offset = native_h - asc_h;
        else if(font_pra.high < asc_h)
            h_offset =  asc_h - native_h - ((asc_h - native_h)/5);
        else 
            h_offset =  0;

		//font_pra.hv = FONT_HORIZONTAL;//sxl?
		//dev_debug_printf("\r\noffset = %d %d %d %d\r\n",h_offset,font_pra.high,asc_h,native_h);

	    //dev_debug_printf("\r\nfont_pra.hv = %d %d %d %d\r\n",h_offset,font_pra.hv,font_pra.width,font_pra.high);
            
        if(font_pra.hv == FONT_VERTICAL)
        {
            //生僻字的显示方式 12*12字库
            //纵库 字节倒序，第一个字节第一个bit是左上角
            //dev_debug_printf("纵库 %d, %d\r\n", font_pra.high, font_pra.width);
            for(h = 0; h < font_pra.width; h++)//宽
            {
                for(v = 0; v < font_pra.high; v++)//高
                {
                    if((font_pra.dot[h + (v/8)*font_pra.width] & (0x01 << (v%8))) != 0)
                    {
                          signature_bitmap_fill_pixel(thiz, disp_x + h + h_offset, disp_y + v);
                    }
				}
            }

        }
        else if(font_pra.hv == FONT_HORIZONTAL)
        {
            //dev_debug_printf("横库 %d, %d\r\n", font_pra.high, font_pra.width);
            for(v = 0; v < font_pra.high; v++)//高
            {
                for(h = 0; h < font_pra.width; h++)//宽
                {
                    if((font_pra.dot[h/8 + v* ((font_pra.width + 7)/8)] & (0x80 >> (h%8))) != 0)
                    {
                        //点黑
                        //dev_debug_printf("v:%d,w:%d,b:%d,bit:%d\r\n", v, h, h/8 + v* ((cur_w + 7)/8), h%8);                            
                        signature_bitmap_fill_pixel(thiz, disp_x + h + h_offset, disp_y + v);
                    }
                }
            }
        }

        disp_x += font_pra.width;
    }

    k_free(buf);
    font_close(CurrFont);

	return 0;
}

void signature_bitmap_set_line_width(SignatureBitmap* thiz, int32_t line_width)
{
	if(thiz != NULL && line_width > 0) 
	{
		thiz->line_width = line_width;
	}
}

void swap(int32_t *x, int32_t *y)
{
	int32_t temp = *x;
	*x = *y;
	*y = temp;
}

void signature_bitmap_fill_pixel_with_line_width(SignatureBitmap* thiz, uint32_t x, uint32_t y, int32_t line_width)
{
	int32_t right, left, top, buttom;

	if(line_width <= 0)
	{
		line_width = 1;
	}
	right = line_width >> 1;
	left = -1 * right;
	top = line_width >> 1;;
	buttom = -top;
	for(; left <= right; left++)
	{
		for(top = line_width >> 1; top >= buttom; top--)
		{
			//dev_debug_printf("set point (%d, %d)\r\n", x + left, y + top);
			signature_bitmap_fill_pixel(thiz, x + left, y + top);
		}
	}
}

/**
布雷森汉姆直线算法（英语：Bresenham's line algorithm）画直线
参考wiki页面：https://zh.wikipedia.org/wiki/%E5%B8%83%E9%9B%B7%E6%A3%AE%E6%BC%A2%E5%A7%86%E7%9B%B4%E7%B7%9A%E6%BC%94%E7%AE%97%E6%B3%95
*/
void signature_bitmap_draw_line(SignatureBitmap *thiz, int32_t x0, int32_t y0, int32_t x1, int32_t y1){
    int32_t deltax, deltay, error, ystep, x, y;
	int32_t steep = abs(y1 - y0) > abs(x1 - x0);

	if(steep)
	{
		swap(&x0, &y0);
		swap(&x1, &y1);
	}
	if(x0 > x1)
	{
		swap(&x0, &x1);
		swap(&y0, &y1);
	}
	deltax = x1 - x0;
	deltay = abs(y1 - y0);
	error = deltax / 2;
	y = y0;
	if(y0 < y1)
	{
		ystep = 1;
	}
	else 
	{
		ystep = -1;
	}

	for(x = x0; x <= x1; x++) 
	{
		
		if(steep) 
		{
			signature_bitmap_fill_pixel_with_line_width(thiz, y, x, thiz->line_width);
		}
		else
		{	
			signature_bitmap_fill_pixel_with_line_width(thiz, x, y, thiz->line_width);
		}
		error = error - deltay;
		if(error < 0)
		{
			y = y + ystep;
			error = error + deltax;
		}
	} 
}

/*
void signature_bitmap_print(SignatureBitmap *thiz)
{
#if 0
	s32 ret = 0;
	u8 i;
	s32 keyvalue;
	u8 cnt = 0;

	
	ret = ddi_thmprn_open();
	dev_debug_printf("ddi_thmprn_open = %d\r\n",ret);

    ret = ddi_thmprn_get_status();
	dev_debug_printf("app get print status = %d\r\n",ret);

	ddi_thmprn_print_image(0, thiz->canvas_pixel_width, thiz->canvas_height, (uint8_t *)thiz->buffer);

	//等待打印完
	cnt = 0;
	while(ddi_thmprn_get_status() == PRN_PRINTING)
	{
		cnt++;
		if(cnt >= 100)
		{
			break;
		}
		else
		{
			ddi_misc_msleep(200);
		}
	}
	
	ddi_thmprn_close();
#endif
}
*/

void signature_bitmap_destroy(SignatureBitmap *thiz)
{
	if(thiz != NULL)
	{
		k_free(thiz->buffer);
		k_free(thiz);
	}
}

void signature_bitmap_show_sign_react(SignatureBitmap *thiz)
{
	strLine line;
	line.m_x0 = thiz->canvas_x;
	line.m_y0 = thiz->canvas_y;
	line.m_x1 = thiz->canvas_x + thiz->canvas_pixel_width;
	line.m_y1 = thiz->canvas_y;

	ddi_lcd_show_line(&line);

	line.m_x0 = line.m_x1;
	line.m_y0 = line.m_y1;
	line.m_x1 = thiz->canvas_x + thiz->canvas_pixel_width;
	line.m_y1 = thiz->canvas_y + thiz->canvas_height;
	
	ddi_lcd_show_line(&line);

	line.m_x0 = line.m_x1;
	line.m_y0 = line.m_y1;
	line.m_x1 = thiz->canvas_x;
	line.m_y1 = thiz->canvas_y + thiz->canvas_height;

	ddi_lcd_show_line(&line);
	
	line.m_x0 = line.m_x1;
	line.m_y0 = line.m_y1;
	line.m_x0 = thiz->canvas_x;
	line.m_y0 = thiz->canvas_y;
	
	ddi_lcd_show_line(&line);
}

static int32_t signature_bitmap_point_hit_canvas(SignatureBitmap* thiz, int32_t x, int32_t y)
{
	if(x < thiz->canvas_x || x > (thiz->canvas_x + thiz->canvas_pixel_width) )
	{
		return 0;
	}
	if(y < thiz->canvas_y || y > thiz->canvas_y + thiz->canvas_height)
	{
		return 0;
	}
	return 1;
}

static int32_t signature_bitmap_translate_to_canvas_point
	(SignatureBitmap* thiz, int32_t *x, int32_t *y, pen_point_struct *pos)
{
	int32_t px = pos->x;//LCD_WIDTH - pos->y;
	int32_t py = pos->y;
	if(!signature_bitmap_point_hit_canvas(thiz, px, py))
	{
		//指针没有命中canvas
		thiz->cur_x = 0xffffffff;
		thiz->cur_y = 0xffffffff;	
		return -1;
	}
	
	*x = px - thiz->canvas_x;
	*y = py - thiz->canvas_y;
	
	return 0;
}
	
void signature_bitmap_pointer_down_hdlr(pen_point_struct pos)
{
	SignatureBitmap* thiz = g_signature_bitmap;
	int32_t x, y, ret;
	
//	dev_debug_printf("---- %s ----- x = %d, y = %d \r\n",__func__, pos.x, pos.y);
	if(thiz == NULL)
	{
		dev_debug_printf("g_signature_bitmap not initialized\r\n");
		return;
	}
	ret = signature_bitmap_translate_to_canvas_point(thiz, &x, &y, &pos);
	if(ret != 0)
	{
		return;
	}
	
	thiz->cur_x = x;
	thiz->cur_y = y; 
}

void signature_bitmap_pointer_up_hdlr(pen_point_struct pos)
{
	SignatureBitmap* thiz = g_signature_bitmap;
//	dev_debug_printf("---- %s ----- x = %d, y = %d \r\n",__func__, pos.x, pos.y);

	if(thiz == NULL)
	{
		dev_debug_printf("g_signature_bitmap not initialized\r\n");
		return;
	}
	thiz->cur_x = 0xffffffff;
	thiz->cur_y = 0xffffffff;	
}


void signature_bitmap_pointer_move_hdlr(pen_point_struct pos)
{
	SignatureBitmap* thiz = g_signature_bitmap;
	int32_t x, y, ret;
//	dev_debug_printf("---- %s ----- x = %d, y = %d \r\n",__func__, pos.x, pos.y);
	if(thiz == NULL)
	{
		dev_debug_printf("g_signature_bitmap not initialized\r\n");
		return;
	}
	ret = signature_bitmap_translate_to_canvas_point(thiz, &x, &y, &pos);
	if(ret != 0)
	{
		return;
	}
	
	if(thiz->cur_x != 0xffffffff)
	{
	
		strLine line;
		line.m_x0 = thiz->cur_x;
		line.m_y0 = thiz->cur_y;
		line.m_x1 = x;
		line.m_y1 = y;
		signature_bitmap_draw_line(thiz, line.m_x0, line.m_y0, line.m_x1, line.m_y1);

		//屏幕上显示的签名线段，要加上canvas偏移
		line.m_x0 += thiz->canvas_x;
		line.m_y0 += thiz->canvas_y;
		line.m_x1 += thiz->canvas_x;
		line.m_y1 += thiz->canvas_y;
		ddi_lcd_show_line(&line);
	}

	thiz->cur_x = x;
	thiz->cur_y = y;
}

/////////////////////////
//end SignatureBitmap
/////////////////////////

s32 ddi_elec_sign_open(s32 canvas_x, s32 canvas_y, s32 canvas_width, s32 canvas_height)
{
	SignatureBitmap* thiz = NULL;

	if(g_signature_bitmap != NULL)
	{
		dev_debug_printf("signature_bitmap has been initialized\r\n");
		return DDI_EIO;
	}
	g_signature_bitmap = signature_bitmap_create(LCD_WIDTH, LCD_HEIGHT, canvas_x, canvas_y,
				canvas_width, canvas_height);
	if(g_signature_bitmap == NULL)
	{
	 	dev_debug_printf("signature_bitmap_create failed\r\n");
		return DDI_EOVERFLOW;
	}

	thiz = g_signature_bitmap;
	if(thiz == NULL)
	{
		dev_debug_printf("signature_bitmap not initialized\r\n");
		return DDI_EIO;
	}
	
//	ddi_tp_init();
//	mmi_pen_unblock();  //sxl?20181213

	memset(thiz->buffer, 0, thiz->buffer_size);
	thiz->cur_x = 0xffffffff;
	thiz->cur_y = 0xffffffff;	

	dev_tp_open();

	return DDI_OK;
}
 
s32 ddi_elec_sign_get_screen_size(s32 *width, s32 *height)
{
	*width = LCD_WIDTH;
	*height = LCD_HEIGHT;

	return DDI_OK;
}
 
s32 ddi_elec_sign_get_data_size(s32 with_header)
{
	SignatureBitmap* thiz = g_signature_bitmap;
	if(thiz == NULL)
	{
		dev_debug_printf("signature_bitmap not initialized\r\n");
		return DDI_EIO;
	}
	
	if(with_header) 
	{
		//TODO，包含bmp文件头的数据长度
		return DDI_EIO;
	} 
	else 
	{
		return thiz->buffer_size;
	}
}
 
s32 ddi_elec_sign_get_data(unsigned char* buffer, s32 *buffer_size, s32 with_header)
{
	SignatureBitmap* thiz = g_signature_bitmap;
	if(thiz == NULL)
	{
		dev_debug_printf("signature_bitmap not initialized\r\n");
		return DDI_EIO;
	}
	
	if(with_header) 
	{
		//TODO，包含bmp文件头的数据长度
		return DDI_EIO;
	} 
	else
	{
		if(*buffer_size < thiz->buffer_size) 
		{
			return DDI_EINVAL;
		}
		*buffer_size = thiz->buffer_size;
		memcpy(buffer, thiz->buffer, thiz->buffer_size);
		
		return DDI_OK;
	}
}

s32 ddi_elec_sign_start(void)
{
#if 0
	SignatureBitmap* thiz = g_signature_bitmap;
	if(thiz == NULL)
	{
		dev_debug_printf("signature_bitmap not initialized\r\n");
		return DDI_EIO;
	}
	memset(thiz->buffer, 0, thiz->buffer_size);
	thiz->cur_x = 0xffffffff;
	thiz->cur_y = 0xffffffff;	
	//显示签名区域	
	//signature_bitmap_show_sign_react(thiz); //sxl2018
	//注册tp回调
	ddi_tp_up_register(signature_bitmap_pointer_up_hdlr);
	ddi_tp_move_register(signature_bitmap_pointer_move_hdlr);
	ddi_tp_down_register(signature_bitmap_pointer_down_hdlr);
#endif
	return DDI_OK;
}

s32 ddi_elec_sign_stop(void)
{
/*
	ddi_tp_up_register(NULL);
	ddi_tp_move_register(NULL);
	ddi_tp_down_register(NULL);	
*/

	return DDI_OK;
}

/*
s32 ddi_elec_sign_print(void)
{
	SignatureBitmap* thiz = g_signature_bitmap;
	if(thiz == NULL)
	{
		dev_debug_printf("signature_bitmap not initialized\r\n");
		return DDI_EIO;
	}
	signature_bitmap_print(thiz);
	
	return DDI_OK;
}
*/
s32 ddi_elec_sign_draw_text(u32 x, u32 y, u8 *text)
{
	SignatureBitmap* thiz = g_signature_bitmap;
	if(thiz == NULL)
	{
		dev_debug_printf("signature_bitmap not initialized\r\n");
		return DDI_EIO;
	}

	return signature_bitmap_draw_text(thiz, x, y, text);
}
 
s32 ddi_elec_sign_close(void)
{
    if(g_signature_bitmap != NULL)
    {
		signature_bitmap_destroy(g_signature_bitmap);
		g_signature_bitmap = NULL;
    }
	
  	dev_tp_close();

	return DDI_OK;
}

s32 ddi_elec_loop(void)
{
  	struct ts_sample ts;
	s32 ret;
	s32 LcdX, LcdY;
	s32 width,height;
	static struct ts_sample old_ts;
	s32 absolute_X = 0, absolute_Y = 0;

	memset(&ts, 0, sizeof(ts));
	ret = dev_tp_read(&ts);
//	dev_debug_printf("type = %d,  (%d,  %d)\r\n", ts.type, ts.x, ts.y);

	if(ret < 0)
		return ret;
	
	if((ts.type == TP_TOUCH_BEGIN) || (ts.type == TP_TOUCH_HOLD))
	{
		TpChangeToLcd(&ts, &LcdX, &LcdY);
		ts.x = LcdX;
		ts.y = LcdY;
		if(ts.type == TP_TOUCH_BEGIN)
		{
		//	dev_debug_printf("type = %d,  (%d,  %d)\r\n", ts.type, ts.x, ts.y);
			//dev_lcd_draw_line(LcdX, LcdY, LcdX, LcdY);
			signature_bitmap_pointer_down_hdlr(ts);

			//signature_bitmap_pointer_down_hdlr(ts);
		} else if(ts.type == TP_TOUCH_HOLD){
			//dev_debug_printf("(%d,  %d), (%d,  %d)\r\n",old_ts.x, old_ts.y, LcdX, LcdY);
			if(LcdX > old_ts.x)
			{
				absolute_X = LcdX - old_ts.x;
			} else {
				absolute_X = old_ts.x - LcdX;
			}
			if(absolute_X > 40)   //判断是否飞点，如果是飞点直接过滤掉
			{
				LcdX = old_ts.x;
			}

			if(LcdY > old_ts.y)
			{
				absolute_Y = LcdY - old_ts.y;
			} else {
				absolute_Y = old_ts.y - LcdY;
			}
			if(absolute_Y > 30)   //判断是否飞点，如果是飞点直接过滤掉
			{
				LcdY = old_ts.y;
			}
			//dev_lcd_draw_line(old_ts.x, old_ts.y, LcdX, LcdY);
			ts.x = LcdX;
			ts.y = LcdY;
			signature_bitmap_pointer_move_hdlr(ts);
		}
		//dev_lcd_brush_screen();
		dev_debug_printf("old_ts.type = %d, old_ts.x = %d, old_ts.y = %d, ts.type = %d, ts.x = %d, ts.y = %d\r\n",
			old_ts.type, old_ts.x, old_ts.y, ts.type,ts.x, ts.y);
		old_ts.type = ts.type;
		old_ts.x	= LcdX;
		old_ts.y	= LcdY;
	} else if(ts.type == TP_TOUCH_END){
		//signature_bitmap_pointer_up_hdlr(ts);
		old_ts.type = TP_TOUCH_END;
		old_ts.x	= 0;
		old_ts.y	= 0;
	}
	
	return 0;	
}

s32 ddi_elec_sign_process(void)
{
	return DDI_ENODEV;
}                    

s32 ddi_elec_sign_ioctl(u32 nCmd, u32 lParam, u32 wParam)
{
	s32 ret = 0;
    
    switch(nCmd)
    {
        case DDI_ELEC_SIGN_LOOP:// 获取DUKPT设备版本
			ddi_elec_loop();
            break;

		case DDI_ELEC_SIGN_CALIBRATE_STATE:
			ret = dev_if_tp_calibrate();
			
			break;

		case DDI_ELEC_SIGN_CALIBRATE:
			ret = ts_calibrate();
			break;

			
        default:
            ret = DDI_EINVAL;
            break;
			
    }
    
    return ret;
}


#else /* DDI_ELEC_SIGN_ENABLE */

s32 ddi_elec_sign_open(s32 canvas_x, s32 canvas_y, s32 canvas_width, s32 canvas_height)
{
	return DDI_ENODEV;
}
 
s32 ddi_elec_sign_get_screen_size(s32 *width, s32 *height)
{
	return DDI_ENODEV;
}

s32 ddi_elec_sign_get_data_size(s32 with_header)
{
	return DDI_ENODEV;
}
 
s32 ddi_elec_sign_get_data(unsigned char* buffer, s32 *buffer_size, s32 with_header)

{
	return DDI_ENODEV;
}

s32 ddi_elec_sign_start(void)
{
	return DDI_ENODEV;
}

s32 ddi_elec_sign_stop(void)
{
	return DDI_ENODEV;
}
#if 0
s32 ddi_elec_sign_print(void)
{
	return DDI_ENODEV;
}
#endif
s32 ddi_elec_sign_draw_text(u32 x, u32 y, u8 *text)
{
	return DDI_ENODEV;
}

s32 ddi_elec_sign_close(void)
{
	return DDI_ENODEV;
}

#endif /* DDI_ELEC_SIGN_ENABLE */
