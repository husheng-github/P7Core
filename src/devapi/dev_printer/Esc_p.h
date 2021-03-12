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
    uint8_t mode;		// ��ǰģʽ
//	uint8_t paper_type;		// ֽ������
	uint8_t h_motionunit;	// �ƶ���Ԫ
	uint8_t v_motionunit;	// �ƶ���Ԫ
	//----chang
	uint8_t international_character_set;	// ѡ������ַ���
	uint8_t character_code_page;					// ����ҳ

	uint8_t prt_on;		// �Ƿ�������ӡ��
	uint8_t larger;		// �Ŵ���
	uint8_t font_en;		// ��������
	uint8_t font_cn;		// ��������
	uint8_t bold;			// ����
	uint8_t double_strike;//�ص���ӡ
	uint8_t underline;	// �»���
	uint8_t revert;		// ������ʾ
    uint8_t smoothing_mode; 	// ƽ��ģʽ

    uint8_t status4;//��ӡ��״̬

	uint8_t rotate;		// ��ת
	uint16_t start_dot;	// ���Դ�ӡλ��
	uint8_t tab[8];		// ˮƽ�Ʊ�λ
	uint8_t linespace;	// �м��
	uint8_t charspace;	// �ַ����
	uint8_t align;		// ���뷽ʽ
	uint16_t leftspace;	// ��߾�
	uint16_t print_width;	// ��ӡ���

#if defined(SUPPORT_PICTURE)
	ESC_P_PICTURE_T picture;	// ��ӡ�ڰ�λͼ
#endif
#if defined(SUPPORT_BITMAP)
	ESC_P_BITMAP_T bitmap;	// λͼ
#endif
#if defined(TM_T88II) || defined(SW40)
    uint8_t upside_down;    // ���µ���ģʽ
#endif


	uint8_t barcode_height;	// ����߶�
	uint8_t barcode_width;	// ������
	uint8_t barcode_leftspace;//��������
	uint8_t barcode_char_pos;	// �����ʶ���ַ��Ĵ�ӡλ��
	uint8_t barcode_font;		// ����ʶ���ַ�����
	uint8_t userdefine_char;	// �û��Զ����ַ���
    ESC_P_FONT_BUF_T font_buf;

#if defined(SUPPORT_USER_CHAR_AB)
	ESC_P_USER_CHAR_A_T user_char_a[0x7e-0x20+1];
	ESC_P_USER_CHAR_B_T user_char_b[0x7e-0x20+1];
#endif
 //  ESC_P_FONT_BUF_T font_buf;
	uint8_t dot_minrow;
	uint8_t dot[LineDot][FONT_CN_A_HEIGHT*FONT_ENLARGE_MAX/8];		// �Ŵ�8��
	uint8_t chinese_mode;		// ����ģʽ
#if defined(SUPPORT_CHINESE_USER_CHAR)
	ESC_P_CHINESE_USER_CHAR_T chinese_user_char[0xFE-0xA1+1];	// �û��Զ��庺���ַ�
#endif
   uint16_t page_H;
   uint16_t asb_mode;
   uint8_t bitmap_flag;

}ESC_P_STS_T;

extern  ESC_P_STS_T  esc_sts;




//======================================================================================================
// ��ӡģʽ
#define prt_mode_font				(1 << 0)		// ����
#define prt_mode_revert				(1 << 1)		// ����
#define prt_mode_upside_down		(1 << 2)		// ����


#define prt_mode_bold				(1 << 3)		// ����
#define prt_mode_double_height		(1 << 4)		// ����
#define prt_mode_double_weight		(1 << 5)		// ����
#define prt_mode_underline			(1 << 6)		// �»���

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
