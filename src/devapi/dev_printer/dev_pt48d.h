#ifndef     _ON_PT48D_H_
#define     _ON_PT48D_H_

#include "includes.h"
#include "devglobal.h"

#include "dev_pt48d_config.h"

//#define ARRAY_SIZE(arr)        (sizeof(arr) / sizeof(arr[0]))

extern uint8_t clr_all_dot;

#if defined(GT30L24M1Z)
    //#define DOUBLE_HEAT_ENABLE
   // #define HISTORY_HEAT_ENABLE
#else
    //#define DOUBLE_HEAT_ENABLE
  //  #define HISTORY_HEAT_ENABLE
#endif

#if defined(HISTORY_HEAT_ENABLE)
    #define HISTORY_HEAT_CNT    (1)
#endif

enum
{
    TP_CMD_PRINT_DOTLINE,
    TP_CMD_FEED_TO_MARK,
    TP_CMD_FEED,
    TP_CMD_CUT,
    TP_CMD_PRINT_DOTLINE_RASTER,
    TP_CMD_MAX
};

// 首先确定每个点行需要分几次加热(TP_MAX_HEAT_STROBE)，然后确定每次加热需要走纸的步数(TP_MAX_STROBE_STEP)
// 最终必须保证(TP_MAX_HEAT_STROBE*TP_MAX_STROBE_STEP) = 一个点行的总步进数
#define Half_Step
#ifdef Half_Step
    #define TP_MAX_HEAT_STROBE  (1)         // 分几次加热，必须保证(LineDot/8/TP_MAX_HEAT_STROBE)是整数
    #if defined(DOUBLE_HEAT_ENABLE)
        #define TP_MAX_STROBE_STEP  (2)
    #else
        #define TP_MAX_STROBE_STEP  (4)         // 每个加热Strobe上的步进
    #endif
#else
    #define TP_MAX_HEAT_STROBE  (1)         // 分几次加热，必须保证(LineDot/8/TP_MAX_HEAT_STROBE)是整数
    #if defined(DOUBLE_HEAT_ENABLE)
        #define TP_MAX_STROBE_STEP  (1)
    #else
        #define TP_MAX_STROBE_STEP  (2)         // 每个加热Strobe上的步进
    #endif
#endif

#if defined(TPH_JING_CHI)
    #define TP_MAX_HEAT_DOT     (96)
#else
    #if defined(DEBUG_TEST)
        #define TP_MAX_HEAT_DOT     (64)        // 每次最多能够加热的点数，必须大等于8
    #else
        //#define TP_MAX_HEAT_DOT       (96)        // 每次最多能够加热的点数，必须大等于8
    #endif
#endif


#define Select_IDLE (10)

typedef enum _PRINTER_STATUS
{
    PT_STATUS_IDLE = 0,
    PT_STATUS_BUSYING = 1,
    PT_STATUS_NOPAPER = 2,
    PT_STATUS_FAULT   = 3,    
    PT_STATUS_HIGH_TEMP = 4,
}printer_status_t;

#ifdef PT487_100DPI
//#define LineDot        (192)
#else
//#define LineDot        (384)
#endif
#define LineDot        (384)

#define TP_BUFLINE_MAX      50
//#define TP_LineDotByte_MAX  (LineDot>>3)

extern void pt48d_dev_init(void);


extern void SetDesity(void);

extern void TPFeedStart(void);
extern void TPFeedStop(void);

extern void TPISRProc(void);
extern void TPFeedLine(uint16_t line);
extern void printer_init(void);
extern void printer_test(void);
extern void PrtFeedStart(uint8_t len);
extern void PrtPrintStart(const uint8_t *buf, const uint32_t x, const uint32_t y);
extern void PrtSetSpeed(uint8_t speed);
extern int16_t PrtGetTemperature(void);
u16 dev_print_get_bufline(void);

extern void TPSelfTest_2(void);
extern void TPSelfTest_3(void);
extern void TPSelfTest_1(void);
extern void TPSelfTest_4(void);
s32 dev_printer_get_status(void);
s32 dev_printer_getpaper_state(void);

#endif



