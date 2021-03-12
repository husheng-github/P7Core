#ifndef __DEV_FONT_H
#define __DEV_FONT_H

//#include "ddi/ddi.h"
#include "ddi/ddi_lcd.h"

#define FONT_TYPE_HZMAX     7
#define FONT_TYPE_ASCMAX    8

#define FONT_TYPE_HZ1212    0
#define FONT_TYPE_HZ1616    1
#define FONT_TYPE_HZ2020    2
#define FONT_TYPE_HZ2424    3
#define FONT_TYPE_HZ3232    4  
#define FONT_TYPE_HZ4848    5
#define FONT_TYPE_NOHZ      6//�����ڵ��ֿ⣬���ڲ��Ժڿ�

#define FONT_TYPE_ASC6X12    0
#define FONT_TYPE_ASC8X16    1
#define FONT_TYPE_ASC1224    2
#define FONT_TYPE_ASC1632    3
#define FONT_TYPE_ASC2448    4
#define FONT_TYPE_ASC6X8     5
#define FONT_TYPE_ASC1624    6
#define FONT_TYPE_NOASC      7//�����ڵ��ֿ⣬���ڲ��Ժڿ�

#define FONT_HORIZONTAL 1//���
#define FONT_VERTICAL   2//�ݿ�



struct _strFontPara
{
    u8 type;//�ֿ�����
    u8 rare;//������Ƨ�ֿ�
    u8 name[16];
    u32 offset;
    u32 width;
    u32 high;
    u32 size;//һ���ַ�ռ�ռ� BYTE
    u8 hv;//˵���Ǻ�⻹���ݿ�
    //u32 ;//�Ƿ���Ҫָ��������GB2312����GB18030?
    u8 font;  //�������ֿ�
};

//���ص���ʹ�õĽṹ
typedef struct _strFontPra
{
    u32 hv;//��⻹���ݿ�
    u32 width;
    u32 high;
    u32 size;
    u8  *dot;
}strFontPra;

//============================================
typedef struct tagBITMAPFILEHEADER  //�ļ�ͷ  14B  
{ 
    u16  bfType;   
    u32  bfSize;   
    u16  bfReserved1;   
    u16  bfReserved2;   
    u32  bfOffBits;   
} BITMAPFILEHEADER; 

typedef struct tagBITMAPINFOHEADER  //ͷ�ļ���Ϣ
{ 
    u32 biSize;   
    s32 biWidth;     
    s32 biHeight;     
    u16 biPlanes;   
    u16 biBitCount;  
    u32 biCompression;   
    u32 biSizeImage;   
    s32 biXPelsPerMeter;   
    s32 biYPelsPerMeter;   
    u32 biClrUsed;  
    u32 biClrImportant;  
} BITMAPINFOHEADER;

typedef struct tagRGBQUAD
{ 
    u8    rgbBlue;    
    u8    rgbGreen;     
    u8    rgbRed;     
    u8    rgbReserved;    
} RGBQUAD;
#if 0
typedef struct _strFont
{
    u32 m_font;             //���壬����ASCII�ַ�
    u32 m_ascsize;          //ASCII�ַ��ֺ�
    u32 m_nativesize;       //�����ֺ�
} strFont;
#endif


typedef struct _CHINESE16X16_FONT_TAB
{
	signed char Index[2];
	char fontdot[32];
	
}CHINESE16X16_FONT_TAB;

//============================================
extern const u8 LCD_CHANGE_WIDTHWAY12TO24[16];
extern const u8 LCD_CHANGE_WIDTHWAY12TO48[4];
extern const struct _strFontPara FontHzPara[1];
extern const struct _strFontPara FontAscPara[1];
//extern strFont CurrFont;
#if 1

extern s32 font_get_str_wh(strFont Font, const u8* Str, u32* w, u32* h);
extern s32 font_get_dot(strFont Font, const u8* Str, strFontPra *FontPra);
extern s32 font_open(strFont Font);
extern s32 font_close(strFont Font);

extern unsigned long gt_2312(unsigned char c1, unsigned char c2, unsigned char c3, unsigned char c4);
void font_get_currtype(strFont *Font);
void font_set_currtype(strFont *Font);
s32 Font_HV_change(s32 width, s32 height, s32 h2v, s8 *dat);

#endif

#endif

