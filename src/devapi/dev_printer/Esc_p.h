#ifndef __ESC_P_H
#define __ESC_P_H
#include "includes.h"
#include "dev_pt48d.h"

//======================================================================================================
#if defined(BRIGHTEK)
#define FONT_A_WIDTH		(12)
#define FONT_A_HEIGHT		(24)
#define FONT_B_WIDTH		(8)
#define FONT_B_HEIGHT		(16)

//#define FONT_B_WIDTH		(9)
//#define FONT_B_HEIGHT		(24)

#define FONT_CN_A_WIDTH		(24)
#define FONT_CN_A_HEIGHT	(24)
#define FONT_CN_B_WIDTH		(16)
#define FONT_CN_B_HEIGHT	(16)
#define FONT_ENLARGE_MAX	(8)
#elif defined(ESC58ALL)
#define FONT_A_WIDTH		(12)
#define FONT_A_HEIGHT		(24)
#define FONT_B_WIDTH		(8)
#define FONT_B_HEIGHT		(16)

//#define FONT_B_WIDTH		(9)
//#define FONT_B_HEIGHT		(24)

#define FONT_CN_A_WIDTH		(24)
#define FONT_CN_A_HEIGHT	(24)
#define FONT_CN_B_WIDTH		(16)
#define FONT_CN_B_HEIGHT	(16)
#define FONT_ENLARGE_MAX	(1)


#elif defined(TM_T88II)
#define FONT_A_WIDTH		(12)
#define FONT_A_HEIGHT		(24)
#ifdef ASCII9X24
#define FONT_B_WIDTH		(9)
#define FONT_B_HEIGHT		(24)
#else
#define FONT_B_WIDTH		(8)
#define FONT_B_HEIGHT		(16)
#endif
#define FONT_CN_A_WIDTH		(24)
#define FONT_CN_A_HEIGHT	(24)
#define FONT_CN_B_WIDTH		(16)
#define FONT_CN_B_HEIGHT	(16)
#define FONT_ENLARGE_MAX	(8)  //(4)

//----chang
#define FONT_C_WIDTH        (9)
#define FONT_C_HEIGHT       (24)


#define h_motion_unit		(180)
#define v_motion_unit		(180)


#endif
#define LineDot        (384)


#if defined(SUPPORT_USER_CHAR_AB)
typedef struct
{
	uint8_t defined;
	uint8_t font[3][12];
}ESC_P_USER_CHAR_A_T;
#endif

typedef struct
{
	uint8_t defined;
	uint8_t font[2][8];
}ESC_P_USER_CHAR_B_T;

#if defined(SUPPORT_CHINESE_USER_CHAR)

typedef struct
{
	uint8_t defined;
	uint8_t font[72];
}ESC_P_CHINESE_USER_CHAR_T;
#endif

#if defined(SUPPORT_PICTURE)
typedef struct
{
	uint8_t mode;
	uint8_t height;
	uint16_t weight;
	uint8_t font[LineDot];
}ESC_P_PICTURE_T;
#endif

#if defined(SUPPORT_BITMAP)
typedef struct
{
	uint8_t defined;
	uint8_t mode;
	uint8_t height;
	uint16_t width;
	uint8_t font[48*4]; // 8
//    uint8_t font[48*1];
}ESC_P_BITMAP_T;
#endif

typedef union
{
	uint8_t font_a[FONT_A_WIDTH][FONT_A_HEIGHT/8];		// 12x24
	uint8_t font_b[FONT_B_WIDTH][FONT_B_HEIGHT/8];
    //----chang
    uint8_t font_c[FONT_C_WIDTH][FONT_C_HEIGHT/8];
	uint8_t font_cn_a[FONT_CN_A_WIDTH][FONT_CN_A_HEIGHT/8];
	uint8_t font_cn_b[FONT_CN_B_WIDTH][FONT_CN_B_HEIGHT/8];
    uint8_t picture[LineDot * 3];
}ESC_P_FONT_BUF_T;

typedef struct
{
    uint8_t mode;		// 当前模式
//	uint8_t paper_type;		// 纸张类型
	uint8_t h_motionunit;	// 移动单元
	uint8_t v_motionunit;	// 移动单元
	//----chang
	uint8_t international_character_set;	// 选择国际字符集
	uint8_t character_code_page;					// 代码页

	uint8_t prt_on;		// 是否启动打印机
	uint8_t larger;		// 放大倍数
	uint8_t font_en;		// 西文字体
	uint8_t font_cn;		// 中文字体
	uint8_t bold;			// 粗体
	uint8_t double_strike;//重叠打印
	uint8_t underline;	// 下划线
	uint8_t revert;		// 反白显示
    uint8_t smoothing_mode; 	// 平滑模式

    uint8_t status4;//打印机状态

	uint8_t rotate;		// 旋转
	uint16_t start_dot;	// 绝对打印位置
	uint8_t tab[8];		// 水平制表位
	uint8_t linespace;	// 行间距
	uint8_t charspace;	// 字符间距
	uint8_t align;		// 对齐方式
	uint16_t leftspace;	// 左边距
	uint16_t print_width;	// 打印宽度

#if defined(SUPPORT_PICTURE)
	ESC_P_PICTURE_T picture;	// 打印黑白位图
#endif
#if defined(SUPPORT_BITMAP)
	ESC_P_BITMAP_T bitmap;	// 位图
#endif
#if defined(TM_T88II) || defined(SW40)
    uint8_t upside_down;    // 上下倒置模式
#endif


	uint8_t barcode_height;	// 条码高度
	uint8_t barcode_width;	// 条码宽度
	uint8_t barcode_leftspace;//条码左间距
	uint8_t barcode_char_pos;	// 条码可识读字符的打印位置
	uint8_t barcode_font;		// 条码识读字符字体
	uint8_t userdefine_char;	// 用户自定义字符集
    ESC_P_FONT_BUF_T font_buf;

#if defined(SUPPORT_USER_CHAR_AB)
	ESC_P_USER_CHAR_A_T user_char_a[0x7e-0x20+1];
	ESC_P_USER_CHAR_B_T user_char_b[0x7e-0x20+1];
#endif
 //  ESC_P_FONT_BUF_T font_buf;
	uint8_t dot_minrow;
	uint8_t dot[LineDot][FONT_CN_A_HEIGHT*FONT_ENLARGE_MAX/8];		// 放大8倍
	uint8_t chinese_mode;		// 汉字模式
#if defined(SUPPORT_CHINESE_USER_CHAR)
	ESC_P_CHINESE_USER_CHAR_T chinese_user_char[0xFE-0xA1+1];	// 用户自定义汉字字符
#endif
   uint16_t page_H;
   uint16_t asb_mode;
   uint8_t bitmap_flag;

}ESC_P_STS_T;

extern  ESC_P_STS_T  esc_sts;




//======================================================================================================
// 打印模式
#define prt_mode_font				(1 << 0)		// 字体
#define prt_mode_revert				(1 << 1)		// 反白
#define prt_mode_upside_down		(1 << 2)		// 倒置


#define prt_mode_bold				(1 << 3)		// 粗体
#define prt_mode_double_height		(1 << 4)		// 倍高
#define prt_mode_double_weight		(1 << 5)		// 倍宽
#define prt_mode_underline			(1 << 6)		// 下划线

#define prt_mode_asb                (1 << 2)        //Enable/disable Automatic Status Back (ASB)
#define prt_mode_busy               (1 << 5)
//======================================================================================================





//======================================================================================================
extern void esc_p(void);
extern void esc_p_init(void);
extern void esc_init(void);
void Printf_Bitmap (uint8_t n,uint8_t mode);
extern uint8_t Getchar(void);
extern uint8_t Putchar(uint8_t c);


//======================================================================================================
#endif
