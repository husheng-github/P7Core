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
    TPSTATE_HEAT_WITH_FEED,       // ��ʼ���ȣ���ֽ��ʱ���Ϊ��ﲽ��ʱ��(��һ״̬ΪTPSTATE_HEAT1_FEED_B)�����Ϊ����ʱ��(��һ״̬ΪTPSTATE_HEAT1_STOP_A)
    TPSTATE_HEAT_WITHOUT_FEED,   //���Ȳ���ֽ
    TPSTATE_FEED,                   //��ֽ
    TPSTATE_FEED_TO_MARK,
    TPSTATE_FEED_FIND_MARK,
    TPSTATE_FEED_AFTERMARK,
    TPSTATE_CUT_PRE_FEED,       // ������Homeλ��
    TPSTATE_CUT_FEED,           // ������ϣ����λ��(Partial/Full)
    TPSTATE_CUT_REVERSE,        // ���˵���Homeλ��
    TPSTATE_CUT_REVERSE1,       // ����һ�ξ���
    TPSTATE_WAIT_TIME,          //���ѵȴ�ʱ��
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
    uint8_t feed_step;      // ����Strobe����ʱ��ﲽ��������
    uint8_t strobe_step;    // �������м���ʱ��Ҫ�ֵ�Strobe���ļ�����
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

// ����ȷ��ÿ��������Ҫ�ּ��μ���(TP_MAX_HEAT_STROBE)��Ȼ��ȷ��ÿ�μ�����Ҫ��ֽ�Ĳ���(TP_MAX_STROBE_STEP)
// ���ձ��뱣֤(TP_MAX_HEAT_STROBE*TP_MAX_STROBE_STEP) = һ�����е��ܲ�����
#define Half_Step

#ifdef Half_Step
    #define TP_MAX_HEAT_STROBE  (1)         // �ּ��μ��ȣ����뱣֤(LineDot/8/TP_MAX_HEAT_STROBE)������
    #if defined(DOUBLE_HEAT_ENABLE)
        #define TP_MAX_STROBE_STEP  (2)
    #else
        #define TP_MAX_STROBE_STEP  (4)         // ÿ������Strobe�ϵĲ���
    #endif
#else
    #define TP_MAX_HEAT_STROBE  (1)         // �ּ��μ��ȣ����뱣֤(LineDot/8/TP_MAX_HEAT_STROBE)������
    #if defined(DOUBLE_HEAT_ENABLE)
        #define TP_MAX_STROBE_STEP  (1)
    #else
        #define TP_MAX_STROBE_STEP  (2)         // ÿ������Strobe�ϵĲ���
    #endif
#endif

#if defined(TPH_JING_CHI)
    #define TP_MAX_HEAT_DOT     (96)
#else
    #if defined(DEBUG_TEST)
        #define TP_MAX_HEAT_DOT     (64)        // ÿ������ܹ����ȵĵ�������������8
    #else
        //#define TP_MAX_HEAT_DOT       (96)        // ÿ������ܹ����ȵĵ�������������8
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



