#ifndef     _ON_PT48D_H_
#define     _ON_PT48D_H_

#include "includes.h"
#include "devglobal.h"
#include "dev_pt48d_config.h"

#define Select_IDLE     (10)
#define LineDot         (384)
#define TP_BUFLINE_MAX  50

typedef enum
{
    TPSTATE_IDLE = 0,
    TPSTATE_PRE_IDLE,
    TPSTATE_START,    //
    TPSTATE_HEAT_WITH_FEED,       // 开始加热，走纸，时间最长为马达步进时间(下一状态为TPSTATE_HEAT1_FEED_B)，最短为加热时间(下一状态为TPSTATE_HEAT1_STOP_A)
    TPSTATE_HEAT_WITHOUT_FEED,   //加热不走纸
    TPSTATE_FEED,                   //走纸
    TPSTATE_FEED_TO_MARK,
    TPSTATE_FEED_FIND_MARK,
    TPSTATE_FEED_AFTERMARK,
    TPSTATE_CUT_PRE_FEED,       // 进刀到Home位置
    TPSTATE_CUT_FEED,           // 进刀到希望的位置(Partial/Full)
    TPSTATE_CUT_REVERSE,        // 先退刀到Home位置
    TPSTATE_CUT_REVERSE1,       // 再退一段距离
    TPSTATE_WAIT_TIME,          //唤醒等待时间
    TPSTATE_MAX
} TPSTATE_T;

typedef struct
{
    TPSTATE_T state;
    uint8_t phase;  
    uint16_t repeat;  
    uint32_t heat;
    uint32_t heat_setting;
    
  #if defined(NEW_HEAT_TIME)
    uint32_t heat_adj;
  #endif
  
    uint32_t heat_remain;
    uint32_t feed_time[TP_MAX_STROBE_STEP];
    uint8_t feed_step;      // 单个Strobe加热时马达步进计数器
    uint8_t strobe_step;    // 单个点行加热时需要分的Strobe数的计数器
    uint8_t accel;
    uint16_t head;
    volatile uint16_t tail;
    uint16_t feedmax;
    uint8_t heat_max_cnt;
    uint8_t heat_cnt;
    uint8_t heat_buf[((LineDot/TP_MAX_HEAT_STROBE)+(64-7)-1)/(64-7)][LineDot/8];
    int32_t markbefore;
} TP_T;

typedef enum _PRINTER_STATUS
{
    PT_STATUS_IDLE = 0,
    PT_STATUS_BUSYING = 1,
    PT_STATUS_NOPAPER = 2,
    PT_STATUS_FAULT   = 3,    
    PT_STATUS_HIGH_TEMP = 4,
}printer_status_t;

struct __vol_speed_map
{
    uint16_t vol;
    uint16_t speed;
};

extern uint8_t clr_all_dot;

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

#ifdef PT487_100DPI
//#define LineDot        (192)
#else
//#define LineDot        (384)
#endif

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



