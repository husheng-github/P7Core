#ifndef _PT48D_CONFIG_H_
#define _PT48D_CONFIG_H_

//1.1.15
//陈明了低压48d支持完整一维码
//1.1.16
//增加版本查询命令1d 49 41
//1.1.17
//效果微调
//1.2.1
//添加条码、图片对齐
//1.2.2
//96点改成80点
//1.3
//1、当检测到 温度低于 10度的时候打印机自动切换到64个点；
//2、 当温度达到10度以上自动切换到96点；
//3、留出96、64、点的切换接口给我们调用，我们可以根据电压变化调节打印机的加热点数。
//指令:1b 1b 60 n
//n=0,加热点数由温度决定，开机默认
//n=1,设定加热点数96
//n=2,设定加热点数64
//1.3.2
//1b 1b 60 n n=1设定96或80改成72
//===================================
//#define PT486
//#define PT487
//#define PT487_100DPI

//#define PT488
//#define PT48D
#define PT48D_NEW_DENSITY//原先浓度太浓
//#define HIGH_SPEED

#define VERSION_MAJOR    1
#define VERSION_MINOR    3
#define VERSION_TEST    2
#define VERSION_BETA    0

//#define GT30L24M1Z  //深圳盒子支付字库ic

#define MAX_HEAT_ADJ      128//80 //96 //64//72//128//
//#define SPEED_60MM

//#define STAMP_BOARD
//#define CODEPAGE
#if defined(CODEPAGE)
//#define FISCAL_REQUIRE//需求的字符放到codepage 255做
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
//#define TPH_JING_CHI//高度是0.0625mm
//#define TPH_JING_TEST//用半步驱动
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
#define SUPPORT_ALL_BARCODE //支持所有条码
//#define SUPPORT_QRCode //没法用
#endif

#define NEW_HEAT_TIME
#define POWER_5V
//#define ADJ_PRINT_BY_VOL //根据电压调整浓度

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

