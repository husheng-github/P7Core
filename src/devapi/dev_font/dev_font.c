/********************************Copyright ( c ) ********************************
**                  DAQU 
**                         
**                        
**
**Created By:         
** Created Date:     
** Version:        
** Description:    ���ļ������ֿ⴦����ؽӿ�
  
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
m_ font: ��������ֵ
               0������
               1������
               2����Բ
m_ ascsize�� ASCII�ַ��ֺ�
               0��6*12
               1��8*16
               2��12*24
               3��16*32
               4��24*48
m_ nativesize�� �����ַ��ֺ�
               0��12*12
               1��16*16
               2��20*20
               3��24*24
               4��32*32
               5��48*48
*/


//G2Ĭ��2424�ֿ�
strFont CurrFont=
{
    0,//����
    FONT_TYPE_ASC6X12,
    FONT_TYPE_HZ1212,
};

const struct _strFontPara FontHzPara[1]=
{
    {FONT_TYPE_HZ1212, FONT_TYPE_HZ1212, "/mtd2/hz1212.bin", 0x480,   12, 12, 24,  FONT_VERTICAL,    0xFF},//���ݿ⣬û����Pƽ̨�ĺ��
     #if 0
    {FONT_TYPE_HZ1616, FONT_TYPE_HZ1616, "/mtd1/hz1616.bin", 0,       16, 16, 32,  FONT_HORIZONTAL,  0xFF},
    {FONT_TYPE_HZ2020, 0XFF            , "/mtd1/hz2020.bin", 0,       20, 20, 60,  FONT_HORIZONTAL,  0xFF},
    {FONT_TYPE_HZ2424, FONT_TYPE_HZ1212, "/mtd1/hz2424.bin", 0,       24, 24, 72,  FONT_HORIZONTAL,  FONT_TYPE_HZ1212},
    {FONT_TYPE_HZ3232, FONT_TYPE_HZ1616, "/mtd1/hz3232.bin", 0,       32, 32, 128, FONT_HORIZONTAL,  FONT_TYPE_HZ1616},
    {FONT_TYPE_HZ4848, FONT_TYPE_HZ1212, "/mtd1/hz4848.bin", 0,       48, 48, 288, FONT_HORIZONTAL,  FONT_TYPE_HZ2424},
    {FONT_TYPE_NOHZ,   0xff            , "/mtd1/nohz",       0,       12, 12, 24,  FONT_HORIZONTAL,  0xff},
    #endif
};

//����ASC��ASC���е�ƫ��
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
//2020��������Ƨ�֣�������Ƨ�֣���һ������
const u8 FONT_HZ2020_ERR_TABLE[]=
{
/*------------------------------------------------------------------------------
  Դ�ļ� / ���� : 
  ����ߣ����أ�: 20��20
  ��ģ��ʽ/��С : ��ɫ����Һ����ģ������ȡģ���ֽ�����/60�ֽ�
  ����ת������  : 2014-6-21 14:59:07
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
�ж��Ƿ�GB2312�ֿⷶΧ��
****************************************************************/
/*
�����ַ����ĸ�����
����GB1318030�ֿ�

���룺
    str:�����˫�ֽڵ���λ��(����BYTE)

���أ�
    0:�ҵ�--˵���ֿ��ڴ�����ģ,2312Ϊ˫�ֽ�2��

    1:��������˫�ֽ�5�� A840--A9A0
    2:��������˫�ֽ�3�� 8140--A0FE
    3:��������˫�ֽ�4�� AA40--FEA0
    4:��������4�ֽ���

    
    5:��������˫�ֽ�1�� A1A1--A9FE

������
    06��06��20
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
    else if((str[0] >= 0xa1 && str[0] <= 0xa9) &&//��ѧ�� 2013.11.07 ���Ӷ�˫�ֽڶ������ж�
            (str[1] >= 0xa1 && str[1] <= 0xfe))
    {
        state_ret = 5;
    }
    // hxj 2011.06.27 ���4�ֽڵ��ж�
    if(*(str +1) >= 0x30  && *(str+1) <= 0x39)
    {
        state_ret = 4;
    }

    return (state_ret);
}


/************************************************************
������
    width �ֿ�
    height �ָ�
    h2v  =1 H->V; ���,����ҵͣ�����->����->����->����==>�ݿ�,�¸��ϵͣ�����-->����-->����-->����
         =0 V->H  �ݿ�,�¸��ϵͣ�����-->����-->����-->����==>��⣬����ҵͣ�����->����->����->����
         =2 H->H;(���,����ҵͣ��������£���������)==>��⣬����ҵͣ�����->����->����->����  ��Ȳ���1Byte,��0
    dat ת������
RETURN:
    =0 FAIL
    =1 SUCCESS
�㷨�������������꣬��ת��
****************************************************************/
static s32 FontHVChange(s32 width, s32 height, s32 h2v, s8 *dat)
{
    int old_widthbyte, old_heightbyte, old_mask = 0;
    int new_widthbyte, new_heightbyte;
    int i,j,k,l,m,n;
    int a;
    u8 data;

    s8 srcbuf[512];

    if(width > 48 || height > 48) return 0;    //����ֻ֧�ֲ�����48X48����
    memset(srcbuf, 0, sizeof(srcbuf));
    //dot = (char*)malloc(512);
    //��⣬����ҵͣ�����->����->����->����  ��Ȳ���1Byte,��0
    //�ݿ�,�¸��ϵͣ�����-->����-->����-->����
    if(1 == h2v)
    {
        //H->V;  ��ת��
        //δ����֤  ????
        //���
        old_widthbyte  = (width+7)>>3;
        old_heightbyte = height;
        //�ݿ�
        new_widthbyte = width;
        new_heightbyte = (height+7)>>3;
        //����
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
        //V->H;  ��ת��
        //�ݿ�
        old_widthbyte = width;
        old_heightbyte = (height+7)>>3;
        //���
        new_widthbyte = (width+7)>>3;
        new_heightbyte = height;
        //����
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
                old_mask <<= 1;           //�ϵ��¸�

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
        //���ת���
        //���
        old_widthbyte  = (width+7)>>3;
        old_heightbyte = height;
        //����
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
**Description:  �ֿ����ԷŴ���չ������ ���Ŵ�һ��������౶�Ŵ󣬵��ö�μ���
**Input parameters:  Font_type: ��ʾ��⻹���ݿ� ����ݿ����ͺܶ�????

                    width:   ��
                    height:  ��
                    Src: Դ����
                    Dst: �Ŵ������
                    Type: 1 ˫��Ŵ�2 ����Ŵ�3 ����Ŵ�(2��3 ��ʱ֧�ֺ��)
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
    u8 buf_temp[300];//��Ҫ�޸ĳ�ͨ�����뷽ʽ
    u8 buf_temp1[300];


    if(Font_HV == FONT_VERTICAL)
    {
        FontDebug("�ݿ�Ŵ�һ�� w:%d h:%d\r\n", width, height);
        j= 0;
        n = 0;
        for(k =0; k < (height+7)/8;)
        {
            j = k*2*2*width;
            n = k*width;

            //FontDebug("ԭ���ݵ�%d��\r\n", k);

            for(i=0; i < width; i++)
            {
                //uart_printf("Ԫ���ݵ�%d��, ��%d������\r\n", i, n+i);
                data = LCD_CHANGE_WIDTHWAY12TO24[Src[n+i]&0x0f];
                //uart_printf("�ϰ벿�ŵ�,��%d��, ��%d�У���%d��\r\n", k*2, 2*i, j+i*2);
                buf_temp[j+i*2] = data;
                buf_temp[j+i*2+1] = data;

                if((k+1)*8 <= height)//��������12*12ʱ���߶�ֻ�Ǵ�2���ֽڱ䵽3���ֽ�
                {
                    data = LCD_CHANGE_WIDTHWAY12TO24[Src[n+i]>>4];
                    buf_temp[i*2 + j +2*width] = data;
                    buf_temp[1 +i*2+ j + 2*width] = data;
                }

            }

            k++;
        }

        hzsize = ((height*2+7)/8)*(width*2);
        FontDebug("�����Ŵ������%d��\r\n", hzsize);
        memcpy(Dst, buf_temp, 72);//
        //sys_debug_format_u8(buf, 72);
    }

    else
    {
        FontDebug("���Ŵ�һ�� w:%d h:%d\r\n", width, height);

        if(Type == 1 || Type == 2 )
        {
            FontDebug("����Ŵ�\r\n");
            if((width&0x07)!=0)
                m=1; //˵������ֽ�
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
                memcpy(Dst, buf_temp ,x);  //��������   
                return 0;
            }
        }
        
        FontDebug("����Ŵ�\r\n");
        memcpy(buf_temp1,buf_temp ,x);  //��������

        //���
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
        memcpy(Dst,buf_temp ,x);  //��������
    }
    return 0;
}
/***************************************************************************************************
������unsigned long gt(unsigned char c1, unsigned char c2, unsigned char c3, unsigned char c4)
���ܣ����㺺�ֵ������ֿ��еĵ�ַ
������c1,c2,c3,c4��4�ֽں�������ͨ������c1,c2,c3,c4���룬˫�ֽ�����ͨ������c1,c2���룬c3=0,c4=0
���أ����ֵ�����ֽڵ�ַ(byte address)������û��ǰ�word mode ��ȡ�������ݣ������ַ(word
address)Ϊ�ֽڵ�ַ����2������word address = byte address / 2 .
���磺BaseAdd: ˵�����ֵ����������ֿ��е���ʼ��ַ��
�������ֵ�����Ϊ0xb0a1,��byte address = gt(0xb0,0xa1,0x00,0x00) *32

�� ���ֵ�����Ϊ0x8139ee39,��byte address = gt(0xee,0x39,0x81,0x39) *32
����:27538+1038
****************************************************************************************************/
unsigned long gt (unsigned char c1, unsigned char c2, unsigned char c3, unsigned char c4)
{
    unsigned long h=0;

    if(c2 == 0x7f)return (h);

    if(c1 >= 0xA1 && c1 <= 0xA9 && c2 >= 0xa1) //Section 1        (yx:�����ַ�846 A1A1��A9FE)
    {
        h= (c1 - 0xA1) * 94 + (c2 - 0xA1);
    }

    else if(c1 >= 0xa8 && c1 <= 0xa9 && c2 < 0xa1) //Section 5    (yx:�����ַ�192 A840��A9A0)
    {
        if(c2 > 0x7f)
            c2--;
        h = (c1 - 0xa8)*96 + (c2 - 0x40) + 846;
    }

    if(c1 >= 0xb0 && c1 <= 0xf7 && c2 >= 0xa1) //Section 2         (yx:�����ַ�6768 B0A1��F7FE)
        h = (c1 - 0xB0) * 94 + (c2 - 0xA1) + 1038;
    else if(c1 < 0xa1 && c1 >= 0x81) //Section 3                  (yx:�����ַ�6080 8140��A0FE)
    {
        if(c2 > 0x7f)
            c2--;
        h = (c1 - 0x81)*190 + (c2 - 0x40) + 7806;//1038 + 6768;
    }
    else if(c1 >= 0xaa && c2 < 0xa1) //Section 4                  (yx:�����ַ�8160 AA40��FEA0)
    {
        if(c2 > 0x7f)
            c2--;
        h = (c1-0xaa)*96 + (c2-0x40) + 13886;//1038 +12848;
    }

    if(c2 >= 0x30 && c2 <= 0x39) //Extended Section (With 4 BYTES InCode)    (yx:�����ַ�6530 0x81308130--0x8439FE39)
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
//------------�ֿ�����ȡģ��

//s32 FontHzFd = 0;
//s32 FontAscFd = 0;
//s32 FontHzFdLink = 0;//�����ֿ�
u8 const *FontHzFdName = NULL;
u8 const *FontAscFdName = NULL;
u8 const *FontHzFdLinkName = NULL;


/****************************************************************************
**Description:      ��ȡ��������,
                     ���ֿⲻ���ں��֣������Ƨ�ֿ�
**Input parameters: Font ��������
                    Str  �ַ�����

**Output parameters:
                    Dot ���ص���
**
**Returned value:   -1 ʧ��
                    -2 ���ֿ�
                    >0 Str ƫ��  1 ˵����ASC��2 һ���Ǻ��֣�4 Ҳ�Ǻ���
**
**Created by:            hxj(2014-4-22) ---̷��2014 05 21��Ϊ��ӡ��Ҫ�޸�
**--------------------------------------------------------------------------
**Modified by:      2014.06.21 ��1212תΪ���
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
        FontDebug("got font ��������\r\n");
        return -1;
    }

    if(FontAscFdName ==NULL
            || (FontHzFdName == NULL && FontHzFdLinkName == NULL))
    {
        FontDebug("�ֿⲻ����\r\n");
        return -2;
    }

    asciisize = FontAscPara[Font.m_ascsize].size;
    //���ֿ��ж�ȡ����,  �Ƿ�Ҫ����С��0X20�����
    if(Str[0] < 0x80)
    {
        #ifdef Test_Font
        memset(temptest,0,sizeof(temptest));
        temptest[0] = Str[str_index];

        FontDebug("��ASC... %s",temptest);
        #endif
        
        ziku_offset = (Str[str_index] - 0x20) * asciisize + FontAscPara[Font.m_ascsize].offset;
        //ddi_vfs_seek(FontAscFd, ziku_offset, 0);
        //ddi_vfs_read(FontPra->dot, asciisize, FontAscFd);
        fs_read_file(FontAscFdName, FontPra->dot, asciisize, ziku_offset);

        //uart_printf("��ASC ok\r\n");
        //sys_debug_format_u8(FontPra->dot, asciisize);

        FontPra->high = FontAscPara[Font.m_ascsize].high;
        FontPra->width= FontAscPara[Font.m_ascsize].width;
        str_index++;
        FontPra->size = FontAscPara[Font.m_ascsize].size;
        FontPra->hv = FONT_VERTICAL;// horizontal ���ݿ�
    }
    else
    {
        //sxl?
        #ifdef Test_Font
        memset(temptest,0,sizeof(temptest));
        memcpy(temptest,&Str[0],2);
            
        FontDebug("������... %s\r\n",temptest);
        #endif
        
        ret = ziku_DifferRange((u8*)&Str[0]);
        if(ret == 0)
        {
            FontDebug("����Ƨ��...");

            if(FontHzFdName != NULL)
            {
                FontDebug("���ֿ�...");
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
                FontDebug("�����ֿ�...");

                use_font.m_nativesize = FontHzPara[Font.m_nativesize].font;

                offset = FontHzPara[use_font.m_nativesize].offset;
                hzsize = FontHzPara[use_font.m_nativesize].size;
                cur_fdname = FontHzFdLinkName;
            }

        }
        else
        {
            FontDebug("��Ƨ��...");
            if(FontHzPara[Font.m_nativesize].rare == 0xFF)
            {
                FontDebug("����������Ƨ�ֿ�...");
                offset = FontHzPara[Font.m_nativesize].offset;
                hzsize = FontHzPara[Font.m_nativesize].size;
                cur_fdname = NULL;//0XFFFFFFFF;//����Ƨ�ֿ�
            }
            else
            {
                use_font.m_nativesize = FontHzPara[Font.m_nativesize].rare;//��������Ƨ�ֿ�
                offset = FontHzPara[use_font.m_nativesize].offset;
                hzsize = FontHzPara[use_font.m_nativesize].size;

                /*fd_rare = ddi_vfs_open(&(FontHzPara[use_font.m_nativesize].name[0]), "rb");
                if(fd_rare == 0)
                {
                    FontDebug("�޺�����Ƨ����ʾ�ֿ�...");
                    return -2;
                }

                cur_fd = fd_rare;
                */
                cur_fdname = &(FontHzPara[use_font.m_nativesize].name[0]);
            }
        }

        //�����ֵ��� ��ͬ���ֿ��в�ͬ��ƫ���㷨
        if(FONT_TYPE_HZ1212 == use_font.m_nativesize)//G2 1212�ֿ����ݿ�
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
            //����GB2312�ֿ�
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

        //if(cur_fd == 0XFFFFFFFF)//g2����ʱֻ��2020û����Ƨ��
        if(cur_fdname == NULL)
        {
            //�ֿⲻ����
            FontDebug("�ֿⲻ����...");
            //��һ������?
            if(Font.m_nativesize == FONT_TYPE_HZ2020)
            {
                FontDebug("���2020����...");
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
        if(fd_rare != NULL)//��Ƨ�ֺ����ã���һ�ι�һ�Σ�Ӱ�첻��
        {
            ddi_vfs_close(fd_rare);
        }
        #endif 

        FontPra->high = FontHzPara[use_font.m_nativesize].high;
        FontPra->width= FontHzPara[use_font.m_nativesize].width;

        //�ж��ֿ��Ƿ���Ҫת��
        if(FontHzPara[use_font.m_nativesize].hv != FontHzPara[Font.m_nativesize].hv)
        {

            //һ���ݿ⣬һ����⣬Ҫת��
            if(FontHzPara[use_font.m_nativesize].hv == FONT_HORIZONTAL)//���ת�ݿ�
            {
                FontDebug("���ת�ݿ�...");
                FontHVChange(FontPra->width, FontPra->high, 1, FontPra->dot);
            }
            else
            {
                FontDebug("�ݿ�ת���...");
                FontHVChange(FontPra->width, FontPra->high, 0, FontPra->dot);
            }

        }

        while(use_font.m_nativesize != Font.m_nativesize)//Ҫ�ĸ����Ĳ�һ��
        {
            FontDebug("��...");
            //��ʱֻ֧��һ��һ����
            //Ҫ֪������ݿ�? ��֪�����Ƕ��٣����Ƕ���? �ܲ���֧��ָ����չ��ʽ?
            FontEx(FontHzPara[Font.m_nativesize].hv, FontPra->width, FontPra->high, FontPra->dot, FontPra->dot, 1);

            FontPra->high *= 2;
            FontPra->width*= 2;

            if(FontPra->width == FontHzPara[Font.m_nativesize].width)//ֱ�������Ȳ��˳�
            {
                break;
            }
        }

        FontPra->hv   = FontHzPara[Font.m_nativesize].hv;
        FontPra->size = FontHzPara[Font.m_nativesize].size;

        if(FontHzPara[Font.m_nativesize].hv == FONT_VERTICAL)//1212�ֿ����ݿ⣬����ת�ɺ�⣬���ⶼ�Ǻ����
        {
            FontDebug("1212��ת��...");
            FontHVChange(FontPra->width, FontPra->high, 0, FontPra->dot);  
            FontPra->hv   = FONT_HORIZONTAL;//FontHzPara[Font.m_nativesize].hv;
        }
    }

    FontDebug("finish\r\n");
    return str_index;
}


/****************************************************************************
**Description:      �����ֿ����ʹ��ֿ�

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
        FontDebug("�޺����ֿ�\r\n");
        return -1;
    }
    if(Font.m_ascsize < FONT_TYPE_ASCMAX)
    {
        FontAscFdName = &(FontAscPara[Font.m_ascsize].name[0]);
    }
    else
    {
        FontDebug("��ASC��ʾ�ֿ�\r\n");
        FontHzFdName = NULL;
        FontHzFdLinkName = NULL;
        return -2;
    }
    
    FontDebug("open font suc %s  %s!\r\n", &(FontHzPara[Font.m_nativesize].name[0]),&(FontAscPara[Font.m_ascsize].name[0]));
    return 0;
}


/****************************************************************************
**Description:      �رյ�ǰ�򿪵��ֿ�
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
**Description:      ����������ַ���������ָ��������Ŀ����߶�     
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
            //uart_printf("4�ֽں���\r\n");
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