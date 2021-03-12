#include "includes.h"

uint16_t max_start_col=0;
//======================================================================================================
extern void DotBufFillToPrn(uint8_t *buf, uint16_t max_col, uint16_t max_rowbyte, uint16_t min_row, uint16_t max_row, uint8_t ratio_width, uint8_t ratio_height)
{
 //  DotBufFillToPrn(&esc_sts.dot[0][0], esc_sts.start_dot, ARRAY_SIZE(esc_sts.dot[0]), esc_sts.dot_minrow, ARRAY_SIZE(esc_sts.dot[0]), 1, 1);
	uint16_t row, col, start_col, bit;
	uint8_t bits, mask;
	uint8_t dot[LineDot/8];
	uint8_t ratio;

	if (max_col*ratio_width > LineDot) return;	// 参数错误

	// 对齐方式调整
	switch (esc_sts.align)
	{
	case 0:		// left align,左对齐
	default:
		start_col = 0;
		break;
	case 1:		// middle align，居中对齐
		start_col = (LineDot - max_col*ratio_width) >> 1;
		break;
	case 2:		// right align，右对齐
		start_col = LineDot - max_col*ratio_width;
		break;
	}
	// 上下倒置调整
//#if defined(TM_T88II) || defined(SW40)
	if (esc_sts.upside_down)
//#else
//	if (0)
//#endif
	{       //倒置
		for (row=max_row-1; row >= min_row; row--)
		{
			bits = 0;
			do
			{
				mask = (1 << bits);
				memset(dot, 0, sizeof(dot));
				for (col=0; col<max_col; col++)
				{
					if (buf[col*max_rowbyte+row] & mask)
					{
						bit = (LineDot - 1) - (start_col + col*ratio_width);
						for (ratio=0; ratio<ratio_width; ratio++, bit--)
						{
							dot[bit >> 3] |= 1<<(7-(bit & 0x07));
						}
					}
				}
				for (ratio=0; ratio<ratio_height; ratio++)
				{
                   if (clr_all_dot==1)
                       break;
					TPPrintLine(dot);
				}

                if (clr_all_dot==1)
                     break;
			}
			while ((++bits)<8);
		}
	}
	else      //正向打印
	{

		for (row=min_row; row<max_row; row++)        //将竖行转变为横行
		{
			bits = 7;
			do
			{
				mask = (1 << bits);
				memset(dot, 0, sizeof(dot));
				for (col=0; col<max_col; col++)
				{
					if (buf[col*max_rowbyte+row] & mask)
					{
						bit = start_col + col*ratio_width;       //起始纵向位置+纵行数*宽放大倍数
						for (ratio=0; ratio<ratio_width; ratio++, bit++)    //实现横向放大
						{
							dot[bit >> 3] |= 1<<(7-(bit & 0x07));
						}
					}
				}
				for (ratio=0; ratio<ratio_height; ratio++)          //实现纵向放大
				{
                  if (clr_all_dot==1)
                      break;
					TPPrintLine(dot);
				}
                if (clr_all_dot==1)
                     break;
			}
			while (bits--);
		}
	}
}
//======================================================================================================
extern uint8_t aera_speed;
static uint8_t start_aera_speed = 0;
extern void BufFillToPrn(uint16_t n)
{
	uint16_t row;

    if (clr_all_dot == 1)
    {
        clr_all_dot = 0;
    }

	if (esc_sts.start_dot)
	{
        start_aera_speed = 0;//暂时改为0
		DotBufFillToPrn(&esc_sts.dot[0][0], max_start_col, ARRAY_SIZE(esc_sts.dot[0]), esc_sts.dot_minrow, ARRAY_SIZE(esc_sts.dot[0]), 1, 1);
	}
	// 行间距调整
	row = (ARRAY_SIZE(esc_sts.dot[0]) - esc_sts.dot_minrow) << 3;
	if (row < esc_sts.linespace)//如果高度小于行间距，则打印出剩余的空白行
	{
		n += esc_sts.linespace-row;
	}
	if (n)
	{
        if(start_aera_speed == 1)
            aera_speed = 1;
        start_aera_speed = 0;
		TPFeedLine(n);
	}
}

extern void BufFillToPrn_0(uint16_t n)
{

    if (clr_all_dot == 1)
    {
        clr_all_dot = 0;
    }

	if (esc_sts.start_dot)
	{
		DotBufFillToPrn(&esc_sts.dot[0][0], max_start_col, ARRAY_SIZE(esc_sts.dot[0]), esc_sts.dot_minrow, ARRAY_SIZE(esc_sts.dot[0]), 1, 1);
	}

	if (n)
	{
		TPFeedLine(n);
	}
}

//======================================================================================================
extern void PrintCurrentBuffer(uint16_t n)
{
	// 先将原先的内容打印出来
	BufFillToPrn(n);
	// 然后清空缓冲区
	esc_sts.bitmap_flag = 0;
	memset(esc_sts.dot, 0, sizeof(esc_sts.dot));
	esc_sts.start_dot = 0;
    max_start_col =0;
	esc_sts.dot_minrow = ARRAY_SIZE(esc_sts.dot[0]);
}

extern void PrintCurrentBuffer_0(uint16_t n)
{
	// 先将原先的内容打印出来
	BufFillToPrn_0(n);
	// 然后清空缓冲区
	esc_sts.bitmap_flag = 0;
	memset(esc_sts.dot, 0, sizeof(esc_sts.dot));
	esc_sts.start_dot = 0;
    max_start_col =0;
	esc_sts.dot_minrow = ARRAY_SIZE(esc_sts.dot[0]);
}

//======================================================================================================
extern void DotFillToBuf(uint8_t *buf, uint16_t col, uint8_t row, uint8_t underline)
{
	uint16_t x, width, maxwidth;
	uint8_t nrow, ncol;
	uint16_t start_dot;
	uint8_t start_row;

    uint8_t dot[FONT_CN_A_HEIGHT*8/8]; 	// 放大2倍////////////////////////////dot[24*2/8]=dot[6]

	esc_sts.bitmap_flag = 1;
	row >>= 3;		// 高度由位转换为字节,传入的是位。

	nrow = row * ((esc_sts.larger & 0x0f)+1);                    //放大后的高度，larger前4位放高放大倍数，后4位放宽放大倍数
	if (nrow > ARRAY_SIZE(esc_sts.dot[0]))                     //放大后的高度>最大高度(数组高度)
	{
		return;
	}
	start_row = ARRAY_SIZE(esc_sts.dot[0])-nrow;            //
	// 当前的缓冲区没有空间再容纳新的打印字符
	ncol = col*(((esc_sts.larger >> 4)& 0x0f) + 1);                         //esc_sts.larger >> 4 取出宽放大倍数,放大后的宽度
	maxwidth = esc_sts.leftspace + esc_sts.print_width;
	if (maxwidth > LineDot)
	{
		maxwidth = LineDot - esc_sts.leftspace;
	}
	if (esc_sts.start_dot && ((esc_sts.start_dot + ncol) > (esc_sts.leftspace + maxwidth)))////当前空间不足，打印出该行
	{
		PrintCurrentBuffer(0);
	}
	if (esc_sts.start_dot == 0)
	{
		if ((ncol > maxwidth) && (ncol > (LineDot - esc_sts.leftspace)))
		{                                             //当宽度不够拿出位置来放左边距的话
			esc_sts.start_dot = LineDot - ncol;
		}
		else//否则从左间距开始打印
		{
			esc_sts.start_dot = esc_sts.leftspace;
		}
	}

	start_dot = esc_sts.start_dot;//记录要加下划线的起始位置

	for (x=0; x<col; x++)                 //横向放大
	{
		FontEnlarge(dot,buf, row);//纵向放大，变高
		buf+= row;                        //指针跳三个字节大小，即跑到了下一个纵行
		width = 0;
		do
		{
			if (esc_sts.start_dot < ARRAY_SIZE(esc_sts.dot))
			{
				memcpy(&esc_sts.dot[esc_sts.start_dot][start_row], dot, nrow);
				esc_sts.start_dot++;
			}
			else
			{
				break;
			}
		}
		while (width++ < (esc_sts.larger >> 4));//esc_sts.larger后四位存储放大高度
	}
	if (underline) FontUnderline(start_dot, esc_sts.start_dot);

	if (start_row < esc_sts.dot_minrow)
	{
		esc_sts.dot_minrow = start_row;
	}

	// 如果当前的缓冲区可以容纳字间距，则加上字间距，如果不能，则打印该行
	if ((esc_sts.start_dot + esc_sts.charspace) > (esc_sts.leftspace + maxwidth))
	{

		PrintCurrentBuffer(0);
	}
	else
	{

		// 字符间距调整
		esc_sts.start_dot += esc_sts.charspace;

	}
	if (esc_sts.revert)//间距反白
    {
        for (x=(start_dot+ncol); x<esc_sts.start_dot; x++)       //col 为width
        {
            memset(&esc_sts.dot[x][start_row], 0xff, nrow);
        }
    }

	if (underline) FontUnderline(start_dot, esc_sts.start_dot);

    if(esc_sts.start_dot > max_start_col)
	{
		max_start_col = esc_sts.start_dot;
	}
}
//======================================================================================================
extern void  PictureDotFillToBuf(uint8_t *buf, uint16_t col, uint16_t row)
{
    uint16_t x,start_row;


    row >>=3;
    if(row > ARRAY_SIZE(esc_sts.dot[0]))
	{
		row = ARRAY_SIZE(esc_sts.dot[0]);
	}

	start_row = ARRAY_SIZE(esc_sts.dot[0])-row;

    if(start_row < esc_sts.dot_minrow)
	{
		esc_sts.dot_minrow = start_row;
	}
	if (esc_sts.start_dot == 0)
	{
		esc_sts.start_dot = esc_sts.leftspace;
	}
    for(x=0; x<col; x++)
	{
		buf += row;
		if(esc_sts.start_dot < ARRAY_SIZE(esc_sts.dot))
		{
			memcpy(&esc_sts.dot[esc_sts.start_dot][start_row], buf, row);
			esc_sts.start_dot++;
		}
		else
		{
			break;
		}

	}

    if(esc_sts.start_dot)
	{
		DotBufFillToPrn(&esc_sts.dot[0][0], esc_sts.start_dot, ARRAY_SIZE(esc_sts.dot[0]), esc_sts.dot_minrow, ARRAY_SIZE(esc_sts.dot[0]), 1, 1);
  //       printf("DOT\n");

	}
	// 然后清空缓冲区
	memset(esc_sts.dot, 0, sizeof(esc_sts.dot));
	esc_sts.start_dot = 0;
	esc_sts.dot_minrow = ARRAY_SIZE(esc_sts.dot[0]);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


