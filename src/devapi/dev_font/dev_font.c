/********************************Copyright ( c ) ********************************
**                  DAQU 
**                         
**                        
**
**Created By:         
** Created Date:     
** Version:        
** Description:    该文件包含字库处理相关接口
  
****************************************************************************/
#include "devglobal.h"
//#define Test_Font
#include "dev_font.h"

#ifdef Test_Font
#define FontDebug dev_debug_printf
#else
#define FontDebug(a,...)
#endif


/*
m_ font: 字体类型值
               0：宋体
               1：黑体
               2：幼圆
m_ ascsize： ASCII字符字号
               0：6*12
               1：8*16
               2：12*24
               3：16*32
               4：24*48
m_ nativesize： 国语字符字号
               0：12*12
               1：16*16
               2：20*20
               3：24*24
               4：32*32
               5：48*48
*/


//G2默认2424字库
strFont CurrFont=
{
    0,//宋体
    FONT_TYPE_ASC6X12,
    FONT_TYPE_HZ1212,
};

const struct _strFontPara FontHzPara[1]=
{
    {FONT_TYPE_HZ1212, FONT_TYPE_HZ1212, "/mtd2/hz1212.bin", 0x480,   12, 12, 24,  FONT_VERTICAL,    0xFF},//用纵库，没有用P平台的横库
     #if 0
    {FONT_TYPE_HZ1616, FONT_TYPE_HZ1616, "/mtd1/hz1616.bin", 0,       16, 16, 32,  FONT_HORIZONTAL,  0xFF},
    {FONT_TYPE_HZ2020, 0XFF            , "/mtd1/hz2020.bin", 0,       20, 20, 60,  FONT_HORIZONTAL,  0xFF},
    {FONT_TYPE_HZ2424, FONT_TYPE_HZ1212, "/mtd1/hz2424.bin", 0,       24, 24, 72,  FONT_HORIZONTAL,  FONT_TYPE_HZ1212},
    {FONT_TYPE_HZ3232, FONT_TYPE_HZ1616, "/mtd1/hz3232.bin", 0,       32, 32, 128, FONT_HORIZONTAL,  FONT_TYPE_HZ1616},
    {FONT_TYPE_HZ4848, FONT_TYPE_HZ1212, "/mtd1/hz4848.bin", 0,       48, 48, 288, FONT_HORIZONTAL,  FONT_TYPE_HZ2424},
    {FONT_TYPE_NOHZ,   0xff            , "/mtd1/nohz",       0,       12, 12, 24,  FONT_HORIZONTAL,  0xff},
    #endif
};

//各种ASC在ASC库中的偏移
#define ASCII_6X8_OFFSET     0
#define ASCII_6X12_OFFSET    (ASCII_6X8_OFFSET+0x300)
#define ASCII_8X16_OFFSET    (ASCII_6X12_OFFSET+0x480)
#define ASCII_1224_OFFSET    (ASCII_8X16_OFFSET+0x600)
#define ASCII_1624_OFFSET    (ASCII_1224_OFFSET+0x1200)
#define ASCII_1632_OFFSET    (ASCII_1624_OFFSET+0x1200)
#define ASCII_2448_OFFSET    (0x1E780 )

const struct _strFontPara FontAscPara[1]=
{
    //{FONT_TYPE_ASC6X12, 0xff, "mtd1/ascii.bin", ASCII_6X12_OFFSET, 6,  12, 12,  FONT_HORIZONTAL, 0xFF},
    {FONT_TYPE_ASC6X12, 0xff, "/mtd2/hz1212.bin", ASCII_6X8_OFFSET, 6,  12, 12,  FONT_HORIZONTAL, 0xFF},
    #if 0
    {FONT_TYPE_ASC8X16, 0xff, "/mtd1/ascii.bin", ASCII_8X16_OFFSET, 8,  16, 16,  FONT_HORIZONTAL, 0XFF},
    {FONT_TYPE_ASC1224, 0xff, "/mtd1/ascii.bin", ASCII_1224_OFFSET, 12, 24, 48,  FONT_HORIZONTAL, 0XFF},
    {FONT_TYPE_ASC1632, 0xff, "/mtd1/ascii.bin", ASCII_1632_OFFSET, 16, 32, 64,  FONT_HORIZONTAL, 0XFF},
    {FONT_TYPE_ASC2448, 0xff, "/mtd1/ascii.bin", ASCII_2448_OFFSET, 24, 48, 144, FONT_HORIZONTAL, 0XFF},
    {FONT_TYPE_ASC6X8,  0xff, "/mtd1/ascii.bin", ASCII_6X8_OFFSET,  6,  8,  8,   FONT_HORIZONTAL, 0XFF},
    {FONT_TYPE_ASC1624, 0xff, "/mtd1/ascii.bin", ASCII_1624_OFFSET, 16, 24, 48,  FONT_HORIZONTAL, 0XFF},
    {FONT_TYPE_NOASC,   0xff, "/mtd1/noasc",     ASCII_1624_OFFSET, 6, 12, 12,  FONT_HORIZONTAL, 0XFF},
    #endif
};

const u8 LCD_CHANGE_WIDTHWAY12TO24[16]=
{
    0x00,0x03,0x0c,0x0f,
    0x30,0x33,0x3c,0x3f,
    0xc0,0xc3,0xcc,0xcf,
    0xf0,0xf3,0xfc,0xff,
};
const u8 LCD_CHANGE_WIDTHWAY12TO48[4]=
{
    0x00,0x0f,0xf0,0xff,
};
//2020不存在生僻字，遇到生僻字，填一个交叉
const u8 FONT_HZ2020_ERR_TABLE[]=
{
/*------------------------------------------------------------------------------
  源文件 / 文字 : 
  宽×高（像素）: 20×20
  字模格式/大小 : 单色点阵液晶字模，横向取模，字节正序/60字节
  数据转换日期  : 2014-6-21 14:59:07
------------------------------------------------------------------------------*/
0x00,0x00,0x00,
0x00,0x00,0x00,
0x3F,0xFF,0xC0,
0x38,0x00,0xC0,
0x38,0x01,0x40,
0x24,0x02,0x40,
0x22,0x04,0x40,
0x21,0x08,0x40,
0x20,0x90,0x40,
0x20,0x60,0x40,
0x20,0x60,0x40,
0x20,0x90,0x40,
0x21,0x08,0x40,
0x22,0x04,0x40,
0x24,0x02,0x40,
0x28,0x01,0x40,
0x30,0x00,0xC0,
0x3F,0xFF,0xC0,
0x00,0x00,0x00,
0x00,0x00,0x00
};

//////////////////////////////////////////////////////////////////
/****************************************************************
判断是否GB2312字库范围内
****************************************************************/
/*
区分字符在哪个区间
用于GB1318030字库

输入：
    str:输入的双字节的区位码(两个BYTE)

返回：
    0:找到--说明字库在存在字模,2312为双字节2区

    1:符号区，双字节5区 A840--A9A0
    2:汉字区，双字节3区 8140--A0FE
    3:汉字区，双字节4区 AA40--FEA0
    4:汉字区，4字节区

    
    5:符号区，双字节1区 A1A1--A9FE

创建：
    06－06－20
*/
static u32 ziku_DifferRange(u8 *str)
{
    u8 state_ret = 0;

    if ((str[0] >= 0xa8 && str[0] <= 0xa9) &&
            (str[1] >= 0x40 && str[1] <= 0xa0))
    {
        state_ret = 1;
    }
    else if ((str[0] >= 0x81 && str[0] <= 0xa0) &&
             (str[1] >= 0x40 && str[1] <= 0xfe))
    {
        state_ret = 2;
    }
    else if ((str[0] >= 0xaa && str[0] <= 0xfe) &&
             (str[1] >= 0x40 && str[1] <= 0xa0))
    {
        state_ret = 3;
    }
    else if((str[0] >= 0xa1 && str[0] <= 0xa9) &&//黄学佳 2013.11.07 增加对双字节二区的判断
            (str[1] >= 0xa1 && str[1] <= 0xfe))
    {
        state_ret = 5;
    }
    // hxj 2011.06.27 添加4字节的判断
    if(*(str +1) >= 0x30  && *(str+1) <= 0x39)
    {
        state_ret = 4;
    }

    return (state_ret);
}


/************************************************************
参数：
    width 字宽
    height 字高
    h2v  =1 H->V; 横库,左高右低，左上->右上->左下->右下==>纵库,下高上低，左上-->右上-->左下-->右下
         =0 V->H  纵库,下高上低，左上-->右上-->左下-->右下==>横库，左高右低，左上->右上->左下->右下
         =2 H->H;(横库,左高右低，左上左下，右上右下)==>横库，左高右低，左上->右上->左下->右下  宽度不足1Byte,补0
    dat 转换数据
RETURN:
    =0 FAIL
    =1 SUCCESS
算法：先求出点的坐标，再转换
****************************************************************/
static s32 FontHVChange(s32 width, s32 height, s32 h2v, s8 *dat)
{
    int old_widthbyte, old_heightbyte, old_mask = 0;
    int new_widthbyte, new_heightbyte;
    int i,j,k,l,m,n;
    int a;
    u8 data;

    s8 srcbuf[512];

    if(width > 48 || height > 48) return 0;    //限制只支持不高于48X48点阵
    memset(srcbuf, 0, sizeof(srcbuf));
    //dot = (char*)malloc(512);
    //横库，左高右低，左上->右上->左下->右下  宽度不足1Byte,补0
    //纵库,下高上低，左上-->右上-->左下-->右下
    if(1 == h2v)
    {
        //H->V;  横转纵
        //未经验证  ????
        //横库
        old_widthbyte  = (width+7)>>3;
        old_heightbyte = height;
        //纵库
        new_widthbyte = width;
        new_heightbyte = (height+7)>>3;
        //补齐
        n = (old_heightbyte+7)&0xf8;
        k = old_widthbyte*old_heightbyte;
        memcpy(srcbuf, dat, k);
        memset(&srcbuf[k], 0, (n-old_heightbyte)*old_widthbyte);
        a=n;
        n=0;
        //m=0;
        for(i=0; i<new_heightbyte; i++)
        {
            for(j=0; j<new_widthbyte; j++)
            {
                if(0 == (j&0x07))
                {
                    old_mask = 0x80;
                }
                else
                    old_mask >>= 1;
                data = 0;
                m = (i<<3)*old_widthbyte+(j>>3);
                for(k=0; k<8; k++)
                {
                    data >>= 1;
                    if(srcbuf[m]&old_mask)
                    {
                        data |= 0x80;
                    }
                    m += old_widthbyte;
                }
                dat[n++]=data;
            }
        }
    }
    else if(0 == h2v)
    {
        //V->H;  纵转横
        //纵库
        old_widthbyte = width;
        old_heightbyte = (height+7)>>3;
        //横库
        new_widthbyte = (width+7)>>3;
        new_heightbyte = height;
        //补齐
        n = (old_widthbyte+7)&0xf8;
        j=0;
        k=0;
        for(i=0; i<old_heightbyte; i++)
        {
            memcpy(&srcbuf[j], &dat[k], old_widthbyte);
            j += n;
            k += old_widthbyte;
        }
        a = n;
        n = 0;
        //m = 0;
        for(i=0; i<new_heightbyte; i++)
        {
            m = (i>>3) * a;

            if(0 == (i&0x07))
            {
                old_mask = 0x01;
            }
            else
                old_mask <<= 1;           //上低下高

            for(j=0; j<new_widthbyte; j++)
            {
                data = 0;
                l = (j<<3);

                for(k=0; k<8; k++)
                {
                    data <<= 1;
                    if(srcbuf[m+l+k]&old_mask)
                    {
                        data+=1;
                    }
                }
                dat[n++]=data;
            }
        }
    }
    else if(2 == h2v)
    {
        //横库转横库
        //横库
        old_widthbyte  = (width+7)>>3;
        old_heightbyte = height;
        //补齐
        //n = (old_heightbyte+7)&0xf8;
        k = old_widthbyte*old_heightbyte;
        memcpy(srcbuf, dat, k);
        n=0;
        for(i=0; i<height; i++)
        {
            for(j=0; j<old_widthbyte; j++)
            {
                dat[n++] = srcbuf[j*height+i];
            }
        }
    }
    else
        return -1;
    return 1;
}
/****************************************************************************
**Description:  字库线性放大扩展函数， 仅放大一倍，如需多倍放大，调用多次即可
**Input parameters:  Font_type: 表示横库还是纵库 横库纵库类型很多????

                    width:   宽
                    height:  高
                    Src: 源数据
                    Dst: 放大后数据
                    Type: 1 双向放大，2 横向放大，3 纵向放大(2跟3 暂时支持横库)
**Output parameters:
**
**Returned value:
**
**Created by:            hxj(2014-6-19)
**--------------------------------------------------------------------------
**Modified by:
**Modified by:
****************************************************************************/
s32 FontEx(u32 Font_HV, s32 width, s32 height, const u8* Src, u8* Dst, u8 Type)
{
    u32 hzsize;
    u8  data;
    u32 j,n,k,i, y, x, m;
    u8 buf_temp[300];//需要修改成通过申请方式
    u8 buf_temp1[300];


    if(Font_HV == FONT_VERTICAL)
    {
        FontDebug("纵库放大一倍 w:%d h:%d\r\n", width, height);
        j= 0;
        n = 0;
        for(k =0; k < (height+7)/8;)
        {
            j = k*2*2*width;
            n = k*width;

            //FontDebug("原数据第%d行\r\n", k);

            for(i=0; i < width; i++)
            {
                //uart_printf("元数据第%d列, 第%d个数据\r\n", i, n+i);
                data = LCD_CHANGE_WIDTHWAY12TO24[Src[n+i]&0x0f];
                //uart_printf("上半部放到,第%d行, 第%d列，共%d个\r\n", k*2, 2*i, j+i*2);
                buf_temp[j+i*2] = data;
                buf_temp[j+i*2+1] = data;

                if((k+1)*8 <= height)//当字体是12*12时，高度只是从2个字节变到3个字节
                {
                    data = LCD_CHANGE_WIDTHWAY12TO24[Src[n+i]>>4];
                    buf_temp[i*2 + j +2*width] = data;
                    buf_temp[1 +i*2+ j + 2*width] = data;
                }

            }

            k++;
        }

        hzsize = ((height*2+7)/8)*(width*2);
        FontDebug("拷贝放大后数据%d个\r\n", hzsize);
        memcpy(Dst, buf_temp, 72);//
        //sys_debug_format_u8(buf, 72);
    }

    else
    {
        FontDebug("横库放大一倍 w:%d h:%d\r\n", width, height);

        if(Type == 1 || Type == 2 )
        {
            FontDebug("横向放大\r\n");
            if((width&0x07)!=0)
                m=1; //说明半个字节
            else
                m=0;

            if(width <= 7)
                m=0;

            k = (width+7)>>3;
            x = 0;
            y = 0;

            for(i=0; i<height; i++)
            {

                for(j=0; j<(k-1); j++)
                {
                    buf_temp[x++] = LCD_CHANGE_WIDTHWAY12TO24[Src[y]>>4];
                    buf_temp[x++] = LCD_CHANGE_WIDTHWAY12TO24[Src[y]&0x0f];
                    y++;
                }
                
                buf_temp[x++] = LCD_CHANGE_WIDTHWAY12TO24[Src[y]>>4];
                if(m==0)
                    buf_temp[x++] = LCD_CHANGE_WIDTHWAY12TO24[Src[y]&0x0f];
                y++;
            }

            width <<=1;

            if(Type == 2)
            {
                memcpy(Dst, buf_temp ,x);  //拷贝数据   
                return 0;
            }
        }
        
        FontDebug("纵向放大\r\n");
        memcpy(buf_temp1,buf_temp ,x);  //拷贝数据

        //填点
        k = ((width) + 7)>>3;
        FontDebug("k=%d\r\n",k);
        FontDebug("high=%d\r\n", height);
        y=0;
        x=0;
        m=0;
        for(i=0; i<height; i++)
        {
            m=y;
            for(j=0; j < k; j++)
            {
                buf_temp[x++]= buf_temp1[y];
                y++;
            }

            for(j=0; j < k; j++)
            {
                buf_temp[x++]= buf_temp1[m];

                m++;
            }

        }
        height<<=1;
        memcpy(Dst,buf_temp ,x);  //拷贝数据
    }
    return 0;
}
/***************************************************************************************************
函数：unsigned long gt(unsigned char c1, unsigned char c2, unsigned char c3, unsigned char c4)
功能：计算汉字点阵在字库中的地址
参数：c1,c2,c3,c4：4字节汉字内码通过参数c1,c2,c3,c4传入，双字节内码通过参数c1,c2传入，c3=0,c4=0
返回：汉字点阵的字节地址(byte address)。如果用户是按word mode 读取点阵数据，则其地址(word
address)为字节地址除以2，即：word address = byte address / 2 .
例如：BaseAdd: 说明汉字点阵数据在字库中的起始地址，
“啊”字的内码为0xb0a1,则byte address = gt(0xb0,0xa1,0x00,0x00) *32

“ ”字的内码为0x8139ee39,则byte address = gt(0xee,0x39,0x81,0x39) *32
总数:27538+1038
****************************************************************************************************/
unsigned long gt (unsigned char c1, unsigned char c2, unsigned char c3, unsigned char c4)
{
    unsigned long h=0;

    if(c2 == 0x7f)return (h);

    if(c1 >= 0xA1 && c1 <= 0xA9 && c2 >= 0xa1) //Section 1        (yx:本区字符846 A1A1—A9FE)
    {
        h= (c1 - 0xA1) * 94 + (c2 - 0xA1);
    }

    else if(c1 >= 0xa8 && c1 <= 0xa9 && c2 < 0xa1) //Section 5    (yx:本区字符192 A840—A9A0)
    {
        if(c2 > 0x7f)
            c2--;
        h = (c1 - 0xa8)*96 + (c2 - 0x40) + 846;
    }

    if(c1 >= 0xb0 && c1 <= 0xf7 && c2 >= 0xa1) //Section 2         (yx:本区字符6768 B0A1—F7FE)
        h = (c1 - 0xB0) * 94 + (c2 - 0xA1) + 1038;
    else if(c1 < 0xa1 && c1 >= 0x81) //Section 3                  (yx:本区字符6080 8140—A0FE)
    {
        if(c2 > 0x7f)
            c2--;
        h = (c1 - 0x81)*190 + (c2 - 0x40) + 7806;//1038 + 6768;
    }
    else if(c1 >= 0xaa && c2 < 0xa1) //Section 4                  (yx:本区字符8160 AA40—FEA0)
    {
        if(c2 > 0x7f)
            c2--;
        h = (c1-0xaa)*96 + (c2-0x40) + 13886;//1038 +12848;
    }

    if(c2 >= 0x30 && c2 <= 0x39) //Extended Section (With 4 BYTES InCode)    (yx:本区字符6530 0x81308130--0x8439FE39)
    {
        if(c4 < 0x30 || c4 > 0x39)
            return(h = 0);

        h = (c3 - 0x81) * 12600 + (c4 - 0x39) * 1260 + (c1 - 0xee) * 10 + (c2 - 0x39) + 22046;
        if(h >= 22046 && h <= 28576) //22046+6530
            return(h);
        else
        {
            h=0;
            return(h);
        }
    }
    return(h);
}
unsigned long gt_2312(unsigned char c1, unsigned char c2, unsigned char c3, unsigned char c4)
{
    u32 t1, t2;
    
    if((c1<0xf8)&&(c1>=0xa1)&&(c2>=0xa1))
    {
        t1 = c1-0xa1;
        t2 = t1*94;
        t2 = t2+c2-0xa1;
    }
    return t2;
}
//------------字库点阵读取模块

//s32 FontHzFd = 0;
//s32 FontAscFd = 0;
//s32 FontHzFdLink = 0;//关联字库
u8 const *FontHzFdName = NULL;
u8 const *FontAscFdName = NULL;
u8 const *FontHzFdLinkName = NULL;


/****************************************************************************
**Description:      读取点阵数据,
                     如字库不存在汉字，则打开生僻字库
**Input parameters: Font 字体类型
                    Str  字符内码

**Output parameters:
                    Dot 返回点阵
**
**Returned value:   -1 失败
                    -2 无字库
                    >0 Str 偏移  1 说明是ASC，2 一般是汉字，4 也是汉字
**
**Created by:            hxj(2014-4-22) ---谭波2014 05 21因为打印需要修改
**--------------------------------------------------------------------------
**Modified by:      2014.06.21 将1212转为横库
**Modified by:
****************************************************************************/
s32 font_get_dot(strFont Font, const u8* Str, strFontPra *FontPra)
{
    u32 ziku_offset;
    u32 asciisize;
    s32 ret = -1;
    strFont use_font;
//    u32 fd_rare=0;//, cur_fd=0;
    u8 const *cur_fdname=NULL;
    u32 str_index = 0;
    
 //   u8  data;
    #ifdef Test_Font
     u8 temptest[128];  //sxl?
     #endif

    u32 hzsize;
    u32 offset;
//    u32 j,n,k,i;
    u32 k;

    if((Str == NULL) || (FontPra == NULL) || (FontPra->dot == NULL))
    {
        FontDebug("got font 参数错误\r\n");
        return -1;
    }

    if(FontAscFdName ==NULL
            || (FontHzFdName == NULL && FontHzFdLinkName == NULL))
    {
        FontDebug("字库不存在\r\n");
        return -2;
    }

    asciisize = FontAscPara[Font.m_ascsize].size;
    //从字库中读取点阵,  是否要处理小于0X20的情况
    if(Str[0] < 0x80)
    {
        #ifdef Test_Font
        memset(temptest,0,sizeof(temptest));
        temptest[0] = Str[str_index];

        FontDebug("读ASC... %s",temptest);
        #endif
        
        ziku_offset = (Str[str_index] - 0x20) * asciisize + FontAscPara[Font.m_ascsize].offset;
        //ddi_vfs_seek(FontAscFd, ziku_offset, 0);
        //ddi_vfs_read(FontPra->dot, asciisize, FontAscFd);
        fs_read_file(FontAscFdName, FontPra->dot, asciisize, ziku_offset);

        //uart_printf("读ASC ok\r\n");
        //sys_debug_format_u8(FontPra->dot, asciisize);

        FontPra->high = FontAscPara[Font.m_ascsize].high;
        FontPra->width= FontAscPara[Font.m_ascsize].width;
        str_index++;
        FontPra->size = FontAscPara[Font.m_ascsize].size;
        FontPra->hv = FONT_VERTICAL;// horizontal ，纵库
    }
    else
    {
        //sxl?
        #ifdef Test_Font
        memset(temptest,0,sizeof(temptest));
        memcpy(temptest,&Str[0],2);
            
        FontDebug("读汉字... %s\r\n",temptest);
        #endif
        
        ret = ziku_DifferRange((u8*)&Str[0]);
        if(ret == 0)
        {
            FontDebug("非生僻字...");

            if(FontHzFdName != NULL)
            {
                FontDebug("本字库...");
                use_font.m_nativesize = Font.m_nativesize;

                offset = FontHzPara[Font.m_nativesize].offset;
                hzsize = FontHzPara[Font.m_nativesize].size;
                cur_fdname = FontHzFdName;
                
                #ifdef Test_Font
                FontDebug("\r\noffset = %d %d\r\n",offset,hzsize);
                #endif
                
            }
            else if(FontHzFdLinkName != NULL)
            {
                FontDebug("关联字库...");

                use_font.m_nativesize = FontHzPara[Font.m_nativesize].font;

                offset = FontHzPara[use_font.m_nativesize].offset;
                hzsize = FontHzPara[use_font.m_nativesize].size;
                cur_fdname = FontHzFdLinkName;
            }

        }
        else
        {
            FontDebug("生僻字...");
            if(FontHzPara[Font.m_nativesize].rare == 0xFF)
            {
                FontDebug("本字体无生僻字库...");
                offset = FontHzPara[Font.m_nativesize].offset;
                hzsize = FontHzPara[Font.m_nativesize].size;
                cur_fdname = NULL;//0XFFFFFFFF;//无生僻字库
            }
            else
            {
                use_font.m_nativesize = FontHzPara[Font.m_nativesize].rare;//关联的生僻字库
                offset = FontHzPara[use_font.m_nativesize].offset;
                hzsize = FontHzPara[use_font.m_nativesize].size;

                /*fd_rare = ddi_vfs_open(&(FontHzPara[use_font.m_nativesize].name[0]), "rb");
                if(fd_rare == 0)
                {
                    FontDebug("无汉字生僻字显示字库...");
                    return -2;
                }

                cur_fd = fd_rare;
                */
                cur_fdname = &(FontHzPara[use_font.m_nativesize].name[0]);
            }
        }

        //读汉字点阵 不同的字库有不同的偏移算法
        if(FONT_TYPE_HZ1212 == use_font.m_nativesize)//G2 1212字库用纵库
        {
            FontDebug("ziku 1212...");
            #if 0  
            if(Str[str_index + 1] >= 0x30 && Str [str_index + 1] <= 0x39)
            {
                ziku_offset = gt(Str[str_index +2 ], Str[str_index + 3], Str[str_index ], Str[str_index + 1]);
                str_index += 4;
            }
            else
            {
                ziku_offset = gt(Str [str_index], Str [str_index + 1], 0, 0);
                str_index += 2;
            }
            #else
            //采用GB2312字库
            ziku_offset = gt_2312(Str [str_index], Str [str_index + 1], 0, 0);
            str_index += 2;
            #endif

            ziku_offset = ziku_offset*hzsize + offset;
            #ifdef Test_Font
            FontDebug("\r\nziku_offset = %d \r\n",ziku_offset);
            #endif
            
        }
        else if(FONT_TYPE_HZ1616 == use_font.m_nativesize)
        {

            FontDebug("ziku1616...SIZE %d ...%02x...%02x", hzsize, Str[str_index], Str[str_index + 1]);
            k = Str[str_index + 1];
            if(k >= 0x80)
            {
                k--;
            }

            ziku_offset = (Str[str_index]-0x81)*0xbe + (k-0x40);
            ziku_offset = ziku_offset*hzsize;
            FontDebug("ziku1616...SIZE %d ..offset :%08x.", hzsize, ziku_offset);

            str_index += 2;
        }
        else
        {
            FontDebug("ziku other...");
            ziku_offset = ((Str[str_index] - 0xa1) * 94 + (Str [str_index + 1] - 0xa1)) * hzsize
                          + offset;
            str_index += 2;
        }

        //if(cur_fd == 0XFFFFFFFF)//g2中暂时只有2020没有生僻字
        if(cur_fdname == NULL)
        {
            //字库不存在
            FontDebug("字库不存在...");
            //打一个交叉?
            if(Font.m_nativesize == FONT_TYPE_HZ2020)
            {
                FontDebug("填充2020交叉...");
                memcpy(FontPra->dot, FONT_HZ2020_ERR_TABLE, FontHzPara[Font.m_nativesize].size);
            }
            
            FontDebug("\r\n");
            FontPra->high = FontHzPara[Font.m_nativesize].high;
            FontPra->width= FontHzPara[Font.m_nativesize].width;
            FontPra->hv   = FontHzPara[Font.m_nativesize].hv;
            FontPra->size = FontHzPara[Font.m_nativesize].size;
            return str_index;
        }


        //ddi_vfs_seek(cur_fd, ziku_offset, 0);
        //ddi_vfs_read(FontPra->dot, hzsize, cur_fd, ziku_offset);
        #ifdef Test_Font
        FontDebug("\r\nziku_read = %s %d \r\n",cur_fdname,ziku_offset);
        #endif
        fs_read_file(cur_fdname, FontPra->dot, hzsize, ziku_offset);

        #ifdef Test_Font
        FontDebug("\r\nziku_read = %s %d \r\n",cur_fdname,ziku_offset);
        for(k = 0;k < hzsize;k++)
        {
            FontDebug("%02x ",FontPra->dot[k]);
        }
        FontDebug("\r\n");
        #endif
        
        //sys_debug_format_u8(FontPra->dot, hzsize);
        #if 0
        if(fd_rare != NULL)//生僻字很少用，开一次关一次，影响不大
        {
            ddi_vfs_close(fd_rare);
        }
        #endif 

        FontPra->high = FontHzPara[use_font.m_nativesize].high;
        FontPra->width= FontHzPara[use_font.m_nativesize].width;

        //判断字库是否需要转换
        if(FontHzPara[use_font.m_nativesize].hv != FontHzPara[Font.m_nativesize].hv)
        {

            //一个纵库，一个横库，要转换
            if(FontHzPara[use_font.m_nativesize].hv == FONT_HORIZONTAL)//横库转纵库
            {
                FontDebug("横库转纵库...");
                FontHVChange(FontPra->width, FontPra->high, 1, FontPra->dot);
            }
            else
            {
                FontDebug("纵库转横库...");
                FontHVChange(FontPra->width, FontPra->high, 0, FontPra->dot);
            }

        }

        while(use_font.m_nativesize != Font.m_nativesize)//要的跟读的不一样
        {
            FontDebug("扩...");
            //暂时只支持一倍一倍扩
            //要知道横库纵库? 再知道横是多少，纵是多少? 能不能支持指定扩展方式?
            FontEx(FontHzPara[Font.m_nativesize].hv, FontPra->width, FontPra->high, FontPra->dot, FontPra->dot, 1);

            FontPra->high *= 2;
            FontPra->width*= 2;

            if(FontPra->width == FontHzPara[Font.m_nativesize].width)//直到宽度相等才退出
            {
                break;
            }
        }

        FontPra->hv   = FontHzPara[Font.m_nativesize].hv;
        FontPra->size = FontHzPara[Font.m_nativesize].size;

        if(FontHzPara[Font.m_nativesize].hv == FONT_VERTICAL)//1212字库是纵库，把他转成横库，对外都是横库了
        {
            FontDebug("1212横转纵...");
            FontHVChange(FontPra->width, FontPra->high, 0, FontPra->dot);  
            FontPra->hv   = FONT_HORIZONTAL;//FontHzPara[Font.m_nativesize].hv;
        }
    }

    FontDebug("finish\r\n");
    return str_index;
}


/****************************************************************************
**Description:      根据字库类型打开字库

**Input parameters:
**Output parameters:
**
**Returned value:
**
**Created by:           
**--------------------------------------------------------------------------
**Modified by:
**Modified by:
****************************************************************************/
s32 font_open(strFont Font)
{
    u32 link_font;

    if(Font.m_nativesize < FONT_TYPE_NOHZ)
    {
        FontHzFdName = &(FontHzPara[Font.m_nativesize].name[0]);
        link_font = FontHzPara[Font.m_nativesize].font;
        FontHzFdLinkName = &(FontHzPara[link_font].name[0]);
    }
    else
    {
        FontDebug("无汉字字库\r\n");
        return -1;
    }
    if(Font.m_ascsize < FONT_TYPE_ASCMAX)
    {
        FontAscFdName = &(FontAscPara[Font.m_ascsize].name[0]);
    }
    else
    {
        FontDebug("无ASC显示字库\r\n");
        FontHzFdName = NULL;
        FontHzFdLinkName = NULL;
        return -2;
    }
    
    FontDebug("open font suc %s  %s!\r\n", &(FontHzPara[Font.m_nativesize].name[0]),&(FontAscPara[Font.m_ascsize].name[0]));
    return 0;
}


/****************************************************************************
**Description:      关闭当前打开的字库
**Input parameters:
**Output parameters:
**
**Returned value:
**
**Created by:            hxj(2014-4-22)
**--------------------------------------------------------------------------
**Modified by:
**Modified by:
****************************************************************************/
s32 font_close(strFont Font)
{
    FontHzFdName = NULL;
    FontAscFdName = NULL;
    FontHzFdLinkName = NULL;
        
    
    return 0;
}


/****************************************************************************
**Description:      解析输入的字符串，计算指定的字体的宽度与高度     
**Input parameters:  
**Output parameters: 
**                   
**Returned value:        
**                   
**Created by:            hxj(2014-7-1)
**--------------------------------------------------------------------------
**Modified by:      
**Modified by:      
****************************************************************************/
s32 font_get_str_wh(strFont Font, const u8* Str, u32* w, u32* h)
{
    u32 str_index = 0;

    if(Str[0] < 0x80)
    {
        *w = FontAscPara[Font.m_ascsize].width;
        *h = FontAscPara[Font.m_ascsize].high;
        str_index++;
    }
    else
    {
        if(Str[str_index + 1] >= 0x30 && Str [str_index + 1] <= 0x39)
        {
            str_index += 4;
            //uart_printf("4字节汉字\r\n");
        }
        else
        {
            str_index += 2;
        }
        
        *w = FontHzPara[Font.m_nativesize].width;
        *h = FontHzPara[Font.m_nativesize].high;
    }    

    return str_index;
}

void font_get_currtype(strFont *Font)
{
    Font->m_font = CurrFont.m_font;
    Font->m_ascsize = CurrFont.m_ascsize;
    Font->m_nativesize = CurrFont.m_nativesize;
}

void font_set_currtype(strFont *Font)
{
    CurrFont.m_font = Font->m_font;
    CurrFont.m_ascsize = Font->m_ascsize;
    CurrFont.m_nativesize = Font->m_nativesize;
}