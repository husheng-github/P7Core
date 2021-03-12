#ifndef _PT48D_CONFIG_H_
#define _PT48D_CONFIG_H_

//1.1.15
//�����˵�ѹ48d֧������һά��
//1.1.16
//���Ӱ汾��ѯ����1d 49 41
//1.1.17
//Ч��΢��
//1.2.1
//������롢ͼƬ����
//1.2.2
//96��ĳ�80��
//1.3
//1������⵽ �¶ȵ��� 10�ȵ�ʱ���ӡ���Զ��л���64���㣻
//2�� ���¶ȴﵽ10�������Զ��л���96�㣻
//3������96��64������л��ӿڸ����ǵ��ã����ǿ��Ը��ݵ�ѹ�仯���ڴ�ӡ���ļ��ȵ�����
//ָ��:1b 1b 60 n
//n=0,���ȵ������¶Ⱦ���������Ĭ��
//n=1,�趨���ȵ���96
//n=2,�趨���ȵ���64
//1.3.2
//1b 1b 60 n n=1�趨96��80�ĳ�72
//===================================
//#define PT486
//#define PT487
//#define PT487_100DPI

//#define PT488
//#define PT48D
#define PT48D_NEW_DENSITY//ԭ��Ũ��̫Ũ
//#define HIGH_SPEED

#define VERSION_MAJOR    1
#define VERSION_MINOR    3
#define VERSION_TEST    2
#define VERSION_BETA    0

//#define GT30L24M1Z  //���ں���֧���ֿ�ic

#define MAX_HEAT_ADJ      128//80 //96 //64//72//128//
//#define SPEED_60MM

//#define STAMP_BOARD
//#define CODEPAGE
#if defined(CODEPAGE)
//#define FISCAL_REQUIRE//������ַ��ŵ�codepage 255��
#endif

//#define SELF_TEST_BITMAP_SAMPLE
//#define POWER_SELFTEST
#define TP_VOLTAGE_SNS
#define TEMP_SNS_ENABLE
//#define LOW_5V_PRINT
//#define ESC_v_P_ENABLE
//#define TP_SELF_TEST
//#define Brazil_Require
//#define OldBoard
//#define TPH_JING_CHI//�߶���0.0625mm
//#define TPH_JING_TEST//�ð벽����
//#define BLACK_MARK_ENABLE

//#define RASTER
//#define BRIGHTEK
//#define ESC58ALL
#define TM_T88II
//#define CHINESE_FONT
//#define GBK
//#define SUPPORT_BITMAP
#define SUPPORT_PICTURE
//#define SUPPORT_NVBITMAP
#define FONT_DOWNLOAD
//#define SUPPORT_USER_CHAR_AB
//#if define SUPPORT_CHINESE_USER_CHAR
#define TP_SOFT_START
//#define DEBUG_TEST
//#define POWER_ON_PRINT
//#define BAUD_460800
#define LPC114
#if defined(LPC114)
#define SUPPORT_ALL_BARCODE //֧����������
//#define SUPPORT_QRCode //û����
#endif

#define NEW_HEAT_TIME
#define POWER_5V
//#define ADJ_PRINT_BY_VOL //���ݵ�ѹ����Ũ��

#if defined(GT30L24M1Z)
#if defined(FONT_DOWNLOAD)
#undef FONT_DOWNLOAD
#endif
#if defined(SUPPORT_NVBITMAP)
#undef SUPPORT_NVBITMAP
#endif
#if !defined(POWER_5V)
#define POWER_5V
#endif

#define SUPPORT_BITMAP
#define SUPPORT_PICTURE

#endif

#if !defined(CODEPAGE)
#define CHINESE_FONT

#if defined(GT30L24M1Z)
#define GB18030
#else
#define GBK
#endif

#endif
#endif

