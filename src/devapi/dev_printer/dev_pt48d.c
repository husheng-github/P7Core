

#include "devglobal.h"
#include "sdk/mhscpu_sdk.h"

#if(PRINTER_EXIST==1)
#include "dev_pt48d.h"
#include "dev_pt48d_hw.h"
#include "dev_pt48d_detect.h"
#include "dev_pt48d_dot.h"
#include "dev_pt48d_config.h"

#define TP_TEMPERATURE_MAX  80     //80° 过温

//#define BIT_CAL_ENABLE
uint8_t TP_MAX_HEAT_DOT = MAX_HEAT_ADJ;
uint8_t TP_HEAT_DOT_BAK = MAX_HEAT_ADJ;
uint8_t clr_all_dot = 0;

uint8_t choose_dot = 1;
uint8_t heat_dot_setting = 1;

uint8_t aera_speed = 0;

#define TpMinWaitTime   ((0.010))

static uint16_t battery_voltage = 380;


extern s32 drv_com_write(s32 nCom, u8 *wbuf, u16 wlen);


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
    uint8_t phase;  //阶段
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
#if defined(HISTORY_HEAT_ENABLE)
    uint8_t history_cnt;
    uint8_t last_max_cnt;
    uint8_t first;
    uint32_t heat_percent;
#endif

    int32_t markbefore;
#if defined(DOUBLE_HEAT_ENABLE)
    uint8_t heat_line_cnt;
#endif
} TP_T;


//////////////////////////////////////////////////////////////////

#if 0
static uint16_t const TpAccelerationSteps[] =
{
    TIMER1_MS_TO_CNT(9.459),
    TIMER1_MS_TO_CNT(7.459),
    TIMER1_MS_TO_CNT(5.459),
    TIMER1_MS_TO_CNT(3.459),
    TIMER1_MS_TO_CNT(2.762),
    TIMER1_MS_TO_CNT(2.314),
    TIMER1_MS_TO_CNT(2.028),
    TIMER1_MS_TO_CNT(1.828),
    TIMER1_MS_TO_CNT(1.675),
    TIMER1_MS_TO_CNT(1.553),
    TIMER1_MS_TO_CNT(1.456),
    TIMER1_MS_TO_CNT(1.374),
    TIMER1_MS_TO_CNT(1.302),
    TIMER1_MS_TO_CNT(1.242),
    TIMER1_MS_TO_CNT(1.191),
    TIMER1_MS_TO_CNT(1.144),
    TIMER1_MS_TO_CNT(1.103),
    TIMER1_MS_TO_CNT(1.065),
    TIMER1_MS_TO_CNT(1.031),
    TIMER1_MS_TO_CNT(1.000),
    TIMER1_MS_TO_CNT(0.970),
    TIMER1_MS_TO_CNT(0.940),
    TIMER1_MS_TO_CNT(0.910),
    TIMER1_MS_TO_CNT(0.880),
    TIMER1_MS_TO_CNT(0.850),
#if defined(TPH_JING_TEST)
    TIMER1_MS_TO_CNT(0.820),
    TIMER1_MS_TO_CNT(0.790),
    TIMER1_MS_TO_CNT(0.760),
    TIMER1_MS_TO_CNT(0.730),
    TIMER1_MS_TO_CNT(0.700),
    /*TIMER1_MS_TO_CNT(0.670),
      TIMER1_MS_TO_CNT(0.640),
      TIMER1_MS_TO_CNT(0.610),
      TIMER1_MS_TO_CNT(0.580),
      TIMER1_MS_TO_CNT(0.550),
      TIMER1_MS_TO_CNT(0.520),
      TIMER1_MS_TO_CNT(0.500),*/
#endif
};
#else
//static uint16_t const TpAccelerationSteps[] =
static uint32_t const TpAccelerationSteps[] =
{
#if 0
/*
    TIMER1_MS_TO_CNT(9.250),
    TIMER1_MS_TO_CNT(8.250),
    TIMER1_MS_TO_CNT(7.760),
    TIMER1_MS_TO_CNT(6.250),
    TIMER1_MS_TO_CNT(5.760),
    
    TIMER1_MS_TO_CNT(4.250),///0
    TIMER1_MS_TO_CNT(3.760),//1/
    TIMER1_MS_TO_CNT(3.459),///2
*/
    TIMER1_MS_TO_CNT(9.027),
    TIMER1_MS_TO_CNT(5.579),
    TIMER1_MS_TO_CNT(4.308),
    TIMER1_MS_TO_CNT(2.614),
    TIMER1_MS_TO_CNT(2.500),
    TIMER1_MS_TO_CNT(2.426),
    TIMER1_MS_TO_CNT(2.272),
    TIMER1_MS_TO_CNT(2.144),
    TIMER1_MS_TO_CNT(2.035),
    TIMER1_MS_TO_CNT(1.941),
    TIMER1_MS_TO_CNT(1.859),
    TIMER1_MS_TO_CNT(1.786),
    TIMER1_MS_TO_CNT(1.721),
    TIMER1_MS_TO_CNT(1.663),
    TIMER1_MS_TO_CNT(1.610),
    TIMER1_MS_TO_CNT(1.561),
    TIMER1_MS_TO_CNT(1.517),
    TIMER1_MS_TO_CNT(1.477),
    TIMER1_MS_TO_CNT(1.439),
    TIMER1_MS_TO_CNT(1.404),
    TIMER1_MS_TO_CNT(1.372),
    TIMER1_MS_TO_CNT(1.342),
    TIMER1_MS_TO_CNT(1.313),
    TIMER1_MS_TO_CNT(1.287),
    TIMER1_MS_TO_CNT(1.261),
    TIMER1_MS_TO_CNT(1.238),
    TIMER1_MS_TO_CNT(1.215),
    TIMER1_MS_TO_CNT(1.194),
    TIMER1_MS_TO_CNT(1.174),
    TIMER1_MS_TO_CNT(1.155),
    TIMER1_MS_TO_CNT(1.136),
    TIMER1_MS_TO_CNT(1.119),
    TIMER1_MS_TO_CNT(1.102),
    TIMER1_MS_TO_CNT(1.086),
    TIMER1_MS_TO_CNT(1.071),
    TIMER1_MS_TO_CNT(1.056),
    TIMER1_MS_TO_CNT(1.042),
    TIMER1_MS_TO_CNT(1.029),
    TIMER1_MS_TO_CNT(1.016),
    TIMER1_MS_TO_CNT(1.003),
    
    TIMER1_MS_TO_CNT(0.991),
    TIMER1_MS_TO_CNT(0.979),
    TIMER1_MS_TO_CNT(0.968),
    TIMER1_MS_TO_CNT(0.957),
    TIMER1_MS_TO_CNT(0.947),
    TIMER1_MS_TO_CNT(0.936),
    TIMER1_MS_TO_CNT(0.927),
    TIMER1_MS_TO_CNT(0.917),
    TIMER1_MS_TO_CNT(0.908),
    TIMER1_MS_TO_CNT(0.899),
    TIMER1_MS_TO_CNT(0.890),
    TIMER1_MS_TO_CNT(0.882),
    TIMER1_MS_TO_CNT(0.873),
    TIMER1_MS_TO_CNT(0.865),
    TIMER1_MS_TO_CNT(0.857),
    TIMER1_MS_TO_CNT(0.850),
    TIMER1_MS_TO_CNT(0.842),
    TIMER1_MS_TO_CNT(0.835),
    TIMER1_MS_TO_CNT(0.828),
    TIMER1_MS_TO_CNT(0.821),
    TIMER1_MS_TO_CNT(0.815),
    TIMER1_MS_TO_CNT(0.808),
    TIMER1_MS_TO_CNT(0.802),

      TIMER1_MS_TO_CNT(0.796),
      TIMER1_MS_TO_CNT(0.789),
      TIMER1_MS_TO_CNT(0.784),
      TIMER1_MS_TO_CNT(0.778),
      TIMER1_MS_TO_CNT(0.772),
      TIMER1_MS_TO_CNT(0.766),
      TIMER1_MS_TO_CNT(0.761),
      TIMER1_MS_TO_CNT(0.756),
      TIMER1_MS_TO_CNT(0.750),
      TIMER1_MS_TO_CNT(0.745),
      TIMER1_MS_TO_CNT(0.740),
      TIMER1_MS_TO_CNT(0.735),
      TIMER1_MS_TO_CNT(0.731),
      TIMER1_MS_TO_CNT(0.726),
      TIMER1_MS_TO_CNT(0.721),
      TIMER1_MS_TO_CNT(0.717),
      TIMER1_MS_TO_CNT(0.712),
      TIMER1_MS_TO_CNT(0.708),
      TIMER1_MS_TO_CNT(0.704),
      TIMER1_MS_TO_CNT(0.699),
      TIMER1_MS_TO_CNT(0.695),
      TIMER1_MS_TO_CNT(0.691),
      TIMER1_MS_TO_CNT(0.687),
      TIMER1_MS_TO_CNT(0.683),
      TIMER1_MS_TO_CNT(0.679),
      TIMER1_MS_TO_CNT(0.675),
      TIMER1_MS_TO_CNT(0.672),
      TIMER1_MS_TO_CNT(0.668),
      TIMER1_MS_TO_CNT(0.664),
      TIMER1_MS_TO_CNT(0.661),
      TIMER1_MS_TO_CNT(0.657),
      TIMER1_MS_TO_CNT(0.654),
      TIMER1_MS_TO_CNT(0.651),
      TIMER1_MS_TO_CNT(0.647),
      TIMER1_MS_TO_CNT(0.644),
      TIMER1_MS_TO_CNT(0.641),
      TIMER1_MS_TO_CNT(0.637),
      TIMER1_MS_TO_CNT(0.634),
      TIMER1_MS_TO_CNT(0.631),
      TIMER1_MS_TO_CNT(0.628),
      TIMER1_MS_TO_CNT(0.625),
      TIMER1_MS_TO_CNT(0.622),
      TIMER1_MS_TO_CNT(0.619),
      TIMER1_MS_TO_CNT(0.616),
      TIMER1_MS_TO_CNT(0.614),
      TIMER1_MS_TO_CNT(0.611),
      TIMER1_MS_TO_CNT(0.608),
#else
    #ifdef MACHINE_P7 //P7
    5579,
    4308,
    2614,
    2500,
    2426,
    2272,
    2144,
    2035,
    1941,
    1859,
    1786,
    1721,
    1663,
    1610,
    1561,
    1517,
    1477,
    1439,
    1404,
    1372,
    1342,
    1313,
    1287,
    1261,
    1238,
    1215,
    1194,
    1174,
    1155,
    1136,
    1119,
    1102,
    1086,
    1071,
    1056,
    1042,
    1029,
    1016,
    1003,
    991,
    979,
    968,
    957,
    947,
    936,
    927,
    917,
    908,
    899,
    890,
    882,
    873,
    865,
    857,
    850,
    842,
    835,
    828,
    821,
    815,
    808,
    802,
    796,
    789,
    784,
    778,
    772,
    766,
    761,
    756,
    750,
    745,
    740,
    735,
    731,
    726,
    721,
    717,
    712,
    708,
    704,
    699,
    695,
    691,
    687,
    683,
    679,
    675,
    672,
    668,
    664,
    661,
    657,
    654,
    651,
    647,
    644,
    #if 0
    641,
    637,
    634,
    631,
    628,
    625,
    622,
    619,
    616,
    614,
    611,
    608,
    605,
    603,
    600,
    597,
    595,
    592,
    590,
    587,
    585,
    582,
    580,
    577,
    575,
    573,
    570,
    568,
    558,
    548,
    538,
    529,
    520,
    512,
    504,
    497,
    489,
    482,
    476,
    469,
    463,
    457,
    452,
    446,
    441,
    426,
    422,
    417,
    413,
    409,
    405,
    401,
    #endif
    #else
    //7450,
    //5379,
    4328,
    3711,
    3308,
    2915,
    2654,
    2426,
    2272,
    2144,
    2035,
    1941,
    1859,
    1786,
    1721,
    1663,
    1610,
    1561,
    1517,
    1477,
    1439,
    1404,
    1372,
    1342,
    1313,
    1287,
    1261,
    1238,
    1215,
    1194,
    1174,
    1155,
    1136,
    1119,
    1102,
    1086,
    1071,
    1056,
    1042,
    1029,
    1016,
    1003,
    991,
    979,
    968,
    957,
    947,
    937,
    927,
    917,
    908,
    899,
    890,
    882,
    873,
    865,
    857,
    850,
    842,
    835,
    828,
    821,
    815,
    808,
    802,
    796,
    789,
    784,
    778,
    772,
    766,
    761,
    756,
    750,
    745,
    740,
    735,
    731,
    726,
    721,
    717,
    712,
    708,
    704,
  //Mask by xiaohonghui 2020.4.1      //77mm/s
    699,
    695,
    691,
    687,
    683,
    679,
    675,
//#if 1                                     //Mask by xiaohonghui 2020.4.20 82mm/s
    672,
    668,
    664,
    661,
    657,
    654,
    651,


    647,
    644,
    #endif
#if 0                                     //Mask by xiaohonghui 2020.4.20 46mm/s
    641,
    637,
    634,
    631,
    628,
    625,
    622,
    619,
    616,
    614,
    611,
    608,

    600,    //半步驱动 50mm/s
    #if 0
    590,
    580,
    570,
    560,
    550,
    540,
    530,
    520,
    510,
    500,
    #endif
#endif
#if 0
    490,
    480,
    470,
    460,
    450,
    440,
    430,
    420,
    410,
    400
#endif
#endif

};
#endif


static TP_T tp;
static uint8_t TP_dot[TP_BUFLINE_MAX][LineDot/8+1];     // 增加一个控制位
//static uint8_t TP_dot[400][LineDot/8+1];     // 增加一个控制位
//extern uint8_t clr_all_dot=0;




#if defined(NEW_HEAT_TIME)
uint8_t const heat_cnt_to_ratio[] =
{

    100,    
    120,    
    120,    
    110,    
    100,    
    100,    
    110,
  /*
    100,

    160,//65
    120,//85
    125,
    95,//100,//100,
    80,//100,
    120,
    */
};

uint8_t const heat_cnt_to_ratio96[] =
{
    80,

    100,//65
    100,//85
    100,//95
    100,
};
#endif


uint8_t const Heat_Temperater_Ratio[]=
{
    // 比例        温度
    100,        //25
    99,
    98,
    97,
    96,
    95,         //30
    94,
    93,
    92,
    91,
    90,         //35
    89,
    88,
    87,
    86,
    85,         //40
    84,
    83,
    82,
    81,
    80,         //45
    79,
    78,
    77,
    76,
    75,         //50
    74,
    73,
    72,
    71,
    70,         //55
    69,
    68,
    67,
    66,
    65,         //60
    64,
    63,
    62,
    61,
    60,         //65
    59,
    58,
    57,
    56,
    55,         //70
    54,
    53,
    52,
    51,
    50,         //75
    48,//50,
    46,//50,
    44,//50,
    42,//50,
    40,//50,         //80
    38,//50,
    36,//50,
    34,//50,
    32,//50,
    30,//50,
};
uint8_t const Heat_TemperaterLow_Ratio[]=
{
    // 比例        温度
    100,        //25
    101,
    102,
    103,
    104,
    105,         //20
    106,
    107,
    108,
    109,
    110,         //15
    111,
    112,
    113,
    114,
    116,         //10
    118,
    120,
    122,
    124,
    128,         //5
    132,
    136,
    140,
    144,
    148,         //0
    152,
    156,
    160,
    164,
    168,         //-5
    172,
    178,
    186,
    194,
    202,         //-10
    210,
    216,
    220,
    224,
    228,         //-15
    230,         //
    238,         //
    242,         //
    248,         //
    250,         //-20
    255,         //
};



extern void PrtSetHeatDot(uint8_t val)
{
    TP_MAX_HEAT_DOT = val;
    TP_HEAT_DOT_BAK = val;
}
//static uint8_t desity;
extern void TPSetSpeed(uint8_t speed)
{
    uint16_t const TPHeatTbl[] =
    {
   /*     TIMER1_MS_TO_CNT(0.80), 
        TIMER1_MS_TO_CNT(0.90), 
        TIMER1_MS_TO_CNT(1.10), 
        TIMER1_MS_TO_CNT(1.30), 
        TIMER1_MS_TO_CNT(1.50),
        TIMER1_MS_TO_CNT(1.60),
        TIMER1_MS_TO_CNT(1.80),
        TIMER1_MS_TO_CNT(2.00),
        TIMER1_MS_TO_CNT(2.20),
        TIMER1_MS_TO_CNT(2.40),
        TIMER1_MS_TO_CNT(2.60),
        TIMER1_MS_TO_CNT(2.80),
*/
#ifdef MACHINE_P7
       TIMER1_MS_TO_CNT(0.20),
       TIMER1_MS_TO_CNT(0.30),
       TIMER1_MS_TO_CNT(0.40), 
       TIMER1_MS_TO_CNT(0.50), 
       TIMER1_MS_TO_CNT(0.60), 
       TIMER1_MS_TO_CNT(0.70), 
       TIMER1_MS_TO_CNT(0.80), //6
       TIMER1_MS_TO_CNT(0.90),
       TIMER1_MS_TO_CNT(1.10),
       TIMER1_MS_TO_CNT(1.30),
       TIMER1_MS_TO_CNT(1.60),
       TIMER1_MS_TO_CNT(1.90),
       TIMER1_MS_TO_CNT(2.10),
#else

       TIMER1_MS_TO_CNT(0.50), 
       TIMER1_MS_TO_CNT(0.60), 
       TIMER1_MS_TO_CNT(0.70), 
       TIMER1_MS_TO_CNT(0.75), 
       TIMER1_MS_TO_CNT(0.80), 
       TIMER1_MS_TO_CNT(0.90),
       TIMER1_MS_TO_CNT(1.10),
       TIMER1_MS_TO_CNT(1.30),
       TIMER1_MS_TO_CNT(1.60),
       TIMER1_MS_TO_CNT(1.90),
       TIMER1_MS_TO_CNT(2.10),
       TIMER1_MS_TO_CNT(2.40),
       TIMER1_MS_TO_CNT(2.50),
#endif
    };

    //desity = speed;
    if (speed < ARRAY_SIZE(TPHeatTbl))
    {
        tp.heat_setting = TPHeatTbl[speed];
#if defined(NEW_HEAT_TIME)
        tp.heat_adj = tp.heat_setting;
#endif
    }
}
extern void SetDesity(void)
{
    tp.head = tp.tail = 0;
    tp.state = TPSTATE_IDLE;
#if 0    
#if defined(TPH_JING_TEST)
    TPSetSpeed(2);
#else
#ifdef SPEED_30MM
    TPSetSpeed(5);
#elif defined(SPEED_50MM)
    TPSetSpeed(3);
#elif defined(SPEED_60MM)
    TPSetSpeed(2);
#elif defined(SPEED_70MM)
    TPSetSpeed(1);
#else
    TPSetSpeed(1);// 4
#endif
#endif
#endif

}


extern void BatteryVoltageADJ(uint16_t bat_vol);

extern void printer_init(void)
{
    pt48d_dev_init();

//    TPSetSpeed(4);
    PRN_POWER_CHARGE();
    MOTOR_PWR_OFF();
    STROBE_0_OFF();
    STROBE_1_OFF();

    MOTOR_PHASE_1A_LOW();
    MOTOR_PHASE_1B_LOW();
    MOTOR_PHASE_2A_LOW();
    MOTOR_PHASE_2B_LOW();
    //===========================

    //PRN_POWER_DISCHARGE();

    //LATCH_LOW();

    tp.head = tp.tail = 0;
    tp.state = TPSTATE_IDLE;

    //pt_resume();
//    pt48d_dev_init();
    TPSetSpeed(2);//1);
    pt_sleep();
    //BatteryVoltageADJ(830);
    

}

extern void TPStart(void)
{
    uint8_t i;
    uint32_t delay;
    //static uint8_t io_set = 0;


    if (!(TPPrinterReady() && (tp.state == TPSTATE_IDLE))) return;

    //DBG_STR("####  TPStart\r\n");

    tp.state = TPSTATE_START;
    tp.accel = 0;

 #if 0
    for(i=0; i<3; i++)
    {
        PRN_POWER_CHARGE();
        delay = 300;
        while(delay--);
        PRN_POWER_DISCHARGE();
        delay = 100;
        while(delay--);
    }
 #endif   

    //LPC_TMR32B1->MR0 = TpAccelerationSteps[0];
    pt_timer_set_periodUs(TpAccelerationSteps[0]);

    PRN_POWER_CHARGE();
    //delay = 3000;while(delay--);

#if defined(HISTORY_HEAT_ENABLE)
    tp.history_cnt = 0;
    tp.first = 0;
    tp.last_max_cnt = 0;
#endif

#if defined(DOUBLE_HEAT_ENABLE)
    tp.heat_line_cnt = 0;
#endif

#if defined(NEW_HEAT_TIME)
    PrintOn_Flag = TRUE;
#endif

    //reset_timer32(TIMER_32_1);
    //enable_timer32(TIMER_32_1);
    //NVIC_EnableIRQ(TIMER_32_1_IRQn);//enable timer32_1
    //pt_timer_set_periodUs(TpAccelerationSteps[0]);
    pt_timer_start();

}
static uint32_t g_printer_state = 1; //1 正常， 0 异常 

/****************************************************************************
**Description:       设置打印机状态 
**Input parameters:    PT_STATUS_IDLE 正常， PT_STATUS_NOPAPER 异常 
**Created by:        caishaojiang,20200523
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void dev_printer_setpaper_state(uint32_t state)
{
    g_printer_state = state;
}

/****************************************************************************
**Description:       设置打印机状态 
**return:    PT_STATUS_IDLE 正常， PT_STATUS_NOPAPER 异常 
**Created by:        caishaojiang,20200523
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_printer_getpaper_state(void)
{
    return g_printer_state;
}
#ifdef TEST_GPIO
extern iomux_pin_name_t  g_prt_while_test_pin; 
#endif

static s32 g_print_overtime_flag = 0;

/**
 * @brief 供loop接口里调用，判断打印是否超时
 * @retval  1 超时
 * @retval  0 正常
 */
s32 TPPrintIsOvertime(void)
{
    return g_print_overtime_flag;
}

/**
 * @brief 清除超时标志
 */
void TPPrintClearOvertimeFlag(void)
{
    g_print_overtime_flag = 0;
}


static void TPPrintCmdToBuf(uint8_t cmd, uint8_t *dot, uint8_t len)
{
#if 1
    uint32_t head;
    u32 beg_ticks = 0;

#if defined(HISTORY_HEAT_ENABLE)
    head = (tp.head+2);// & (ARRAY_SIZE(TP_dot)-1);
#else
    head = (tp.head+1);// & (ARRAY_SIZE(TP_dot)-1);
#endif
    head = (head%TP_BUFLINE_MAX);

    //DBG_STR("####  TPPrintCmdToBuf()\r\n");
    if(dev_printer_getpaper_state() == PT_STATUS_NOPAPER)
    {
        return;
    }
#ifdef TEST_GPIO
    dev_gpio_direction_output(g_prt_while_test_pin, 1);
#endif

    beg_ticks = dev_user_gettimeID();
    while (head == tp.tail)
    {
        // 因为打印中断处理程序有可能在异常的情况下进入Idle状态，所以需要不断检查这个状态
        //event_proc();
        if ((tp.state == TPSTATE_IDLE)&&TPPrinterReady())
        {
             TPStart();
        }
        
        if((!TPPrinterReady()) == TRUE)
        {
            tp.head = tp.tail = 0;
            tp.state = TPSTATE_IDLE;
            memset(tp.heat_buf, 0, sizeof(tp.heat_buf));
            return;
        }

        //Add by xiaohonghui 2020.6.9 超时，防止卡住
        if(dev_user_querrytimer(beg_ticks, 2*1000))
        {
            g_print_overtime_flag = 1;
            tp.head = tp.tail = 0;
            tp.state = TPSTATE_IDLE;
            memset(tp.heat_buf, 0, sizeof(tp.heat_buf));
            return;
        }
    }
#ifdef TEST_GPIO
    dev_gpio_direction_output(g_prt_while_test_pin, 0);
#endif
#endif
    //memcpy(TP_dot[tp.head & (ARRAY_SIZE(TP_dot)-1)], dot, len);
    memcpy(TP_dot[tp.head], dot, len);

    TP_dot[tp.head][LineDot/8] = cmd;

//    tp.head = (tp.head+1) & (ARRAY_SIZE(TP_dot)-1);
    tp.head = head;

    TPStart();

}


extern void TPFeedLine(uint16_t line)
{
    //DBG_STR("####  TPFeedLine()\r\n");
    
#ifdef Half_Step
    line <<= 2;     // 一个点行等于4步
#else
    line <<= 1;     // 一个点行等于2步
#endif

#if defined(TPH_JING_CHI)
#if defined(TPH_JING_TEST)
    TPPrintCmdToBuf(TP_CMD_FEED, (uint8_t *)(&line), sizeof(line));
#endif
#endif

    TPPrintCmdToBuf(TP_CMD_FEED, (uint8_t *)(&line), sizeof(line));
}




static uint16_t TPGetStepTime(void)
{
    uint32_t time;

    time = TpAccelerationSteps[tp.accel];
    if(tp.accel < (ARRAY_SIZE(TpAccelerationSteps)-1))
    {
        tp.accel++;
    }
    return time;
}


static void TPSetTimeCnt(uint32_t tm)
{
    if(tm < TpMinWaitTime*1000)  tm = TpMinWaitTime*1000;

    pt_timer_set_periodUs(tm);
    pt_timer_start();
}

static void TPForwardStep(int direction)
{

//    PRN_POWER_DISCHARGE();
    //sys_delay(10000);
    pt_printline_count();
#if defined(PT486) || defined(PT487)||defined(PT48D) || defined(PT48D_NEW_DENSITY)
#ifdef Half_Step
//dev_debug_printf("tp.phase=%d\r\n", tp.phase&0x07);
    switch (tp.phase & 0x07)
    {
#if defined(STAMP_BOARD)
    case 0:
        MOTOR_PHASE_1A_HIGH();
        MOTOR_PHASE_1B_LOW();
        MOTOR_PHASE_2A_HIGH();
        MOTOR_PHASE_2B_LOW();
        break;
    case 7:
        MOTOR_PHASE_1A_HIGH();
        MOTOR_PHASE_1B_LOW();
        MOTOR_PHASE_2A_LOW();
        MOTOR_PHASE_2B_LOW();
        break;
    case 6:
        MOTOR_PHASE_1A_HIGH();
        MOTOR_PHASE_1B_LOW();
        MOTOR_PHASE_2A_LOW();
        MOTOR_PHASE_2B_HIGH();
        break;
    case 5:
        MOTOR_PHASE_1A_LOW();
        MOTOR_PHASE_1B_LOW();
        MOTOR_PHASE_2A_LOW();
        MOTOR_PHASE_2B_HIGH();
        break;
    case 4:
        MOTOR_PHASE_1A_LOW();
        MOTOR_PHASE_1B_HIGH();
        MOTOR_PHASE_2A_LOW();
        MOTOR_PHASE_2B_HIGH();
        break;
    case 3:
        MOTOR_PHASE_1A_LOW();
        MOTOR_PHASE_1B_HIGH();
        MOTOR_PHASE_2A_LOW();
        MOTOR_PHASE_2B_LOW();
        break;
    case 2:
        MOTOR_PHASE_1A_LOW();
        MOTOR_PHASE_1B_HIGH();
        MOTOR_PHASE_2A_HIGH();
        MOTOR_PHASE_2B_LOW();
        break;
    case 1:
        MOTOR_PHASE_1A_LOW();
        MOTOR_PHASE_1B_LOW();
        MOTOR_PHASE_2A_HIGH();
        MOTOR_PHASE_2B_LOW();
        break;
#else
#if 0
    case 0:
        MOTOR_PHASE_1A_HIGH();
        MOTOR_PHASE_1B_LOW();
        MOTOR_PHASE_2A_HIGH();
        MOTOR_PHASE_2B_LOW();
        break;
    case 1:
        MOTOR_PHASE_1A_HIGH();
        MOTOR_PHASE_1B_LOW();
        MOTOR_PHASE_2A_LOW();
        MOTOR_PHASE_2B_LOW();
        break;
    case 2:
        MOTOR_PHASE_1A_HIGH();
        MOTOR_PHASE_1B_LOW();
        MOTOR_PHASE_2A_LOW();
        MOTOR_PHASE_2B_HIGH();
        break;
    case 3:
        MOTOR_PHASE_1A_LOW();
        MOTOR_PHASE_1B_LOW();
        MOTOR_PHASE_2A_LOW();
        MOTOR_PHASE_2B_HIGH();
        break;
    case 4:
        MOTOR_PHASE_1A_LOW();
        MOTOR_PHASE_1B_HIGH();
        MOTOR_PHASE_2A_LOW();
        MOTOR_PHASE_2B_HIGH();
        break;
    case 5:
        MOTOR_PHASE_1A_LOW();
        MOTOR_PHASE_1B_HIGH();
        MOTOR_PHASE_2A_LOW();
        MOTOR_PHASE_2B_LOW();
        break;
    case 6:
        MOTOR_PHASE_1A_LOW();
        MOTOR_PHASE_1B_HIGH();
        MOTOR_PHASE_2A_HIGH();
        MOTOR_PHASE_2B_LOW();
        break;
    case 7:
        MOTOR_PHASE_1A_LOW();
        MOTOR_PHASE_1B_LOW();
        MOTOR_PHASE_2A_HIGH();
        MOTOR_PHASE_2B_LOW();
        break;
#endif

case 0:
    MOTOR_PHASE_1A_HIGH();
    MOTOR_PHASE_1B_LOW();
    MOTOR_PHASE_2A_LOW();
    MOTOR_PHASE_2B_HIGH();
    break;
case 1:
    MOTOR_PHASE_1A_LOW();
    MOTOR_PHASE_1B_LOW();
    MOTOR_PHASE_2A_LOW();
    MOTOR_PHASE_2B_HIGH();
    break;
case 2:
    MOTOR_PHASE_1A_LOW();
    MOTOR_PHASE_1B_LOW();
    MOTOR_PHASE_2A_HIGH();
    MOTOR_PHASE_2B_HIGH();
    break;
case 3:
    MOTOR_PHASE_1A_LOW();
    MOTOR_PHASE_1B_LOW();
    MOTOR_PHASE_2A_HIGH();
    MOTOR_PHASE_2B_LOW();
    break;
case 4:
    MOTOR_PHASE_1A_LOW();
    MOTOR_PHASE_1B_HIGH();
    MOTOR_PHASE_2A_HIGH();
    MOTOR_PHASE_2B_LOW();
    break;
case 5:
    MOTOR_PHASE_1A_LOW();
    MOTOR_PHASE_1B_HIGH();
    MOTOR_PHASE_2A_LOW();
    MOTOR_PHASE_2B_LOW();
    break;
case 6:
    MOTOR_PHASE_1A_HIGH();
    MOTOR_PHASE_1B_HIGH();
    MOTOR_PHASE_2A_LOW();
    MOTOR_PHASE_2B_LOW();
    break;
case 7:
    MOTOR_PHASE_1A_HIGH();
    MOTOR_PHASE_1B_LOW();
    MOTOR_PHASE_2A_LOW();
    MOTOR_PHASE_2B_LOW();
    break;

#endif
    }

#else  // Half_Step

    switch (tp.phase & 0x03)
    {
#if defined(STAMP_BOARD)
    case 0:
        MOTOR_PHASE_1A_HIGH();
        MOTOR_PHASE_1B_LOW();
        MOTOR_PHASE_2A_HIGH();
        MOTOR_PHASE_2B_LOW();
        break;
    case 3:
        MOTOR_PHASE_1A_HIGH();
        MOTOR_PHASE_1B_LOW();
        MOTOR_PHASE_2A_LOW();
        MOTOR_PHASE_2B_HIGH();
        break;
    case 2:
        MOTOR_PHASE_1A_LOW();
        MOTOR_PHASE_1B_HIGH();
        MOTOR_PHASE_2A_LOW();
        MOTOR_PHASE_2B_HIGH();
        break;
    case 1:
        MOTOR_PHASE_1A_LOW();
        MOTOR_PHASE_1B_HIGH();
        MOTOR_PHASE_2A_HIGH();
        MOTOR_PHASE_2B_LOW();
        break;
#else       //#if defined(STAMP_BOARD)
#if 0
    case 0:
        MOTOR_PHASE_1A_HIGH();
        MOTOR_PHASE_1B_LOW();
        MOTOR_PHASE_2A_HIGH();
        MOTOR_PHASE_2B_LOW();
        break;
    case 1:
        MOTOR_PHASE_1A_HIGH();
        MOTOR_PHASE_1B_LOW();
        MOTOR_PHASE_2A_LOW();
        MOTOR_PHASE_2B_HIGH();
        break;
    case 2:
        MOTOR_PHASE_1A_LOW();
        MOTOR_PHASE_1B_HIGH();
        MOTOR_PHASE_2A_LOW();
        MOTOR_PHASE_2B_HIGH();
        break;
    case 3:
        MOTOR_PHASE_1A_LOW();
        MOTOR_PHASE_1B_HIGH();
        MOTOR_PHASE_2A_HIGH();
        MOTOR_PHASE_2B_LOW();
        break;
#endif

#if 0
    case 0:
        MOTOR_PHASE_1A_HIGH();
        MOTOR_PHASE_2A_LOW();
        MOTOR_PHASE_1B_LOW();
        MOTOR_PHASE_2B_HIGH();
        break;
    case 1:
        MOTOR_PHASE_1A_HIGH();
        MOTOR_PHASE_2A_LOW();
        MOTOR_PHASE_1B_HIGH();
        MOTOR_PHASE_2B_LOW();
        break;
    case 2:
        MOTOR_PHASE_1A_LOW();
        MOTOR_PHASE_2A_HIGH();
        MOTOR_PHASE_1B_HIGH();
        MOTOR_PHASE_2B_LOW();
        break;
    case 3:
        MOTOR_PHASE_1A_LOW();
        MOTOR_PHASE_2A_HIGH();
        MOTOR_PHASE_1B_LOW();
        MOTOR_PHASE_2B_HIGH();
        break;

#endif

    case 0:
        MOTOR_PHASE_1A_HIGH();
        MOTOR_PHASE_2A_LOW();
        MOTOR_PHASE_1B_LOW();
        MOTOR_PHASE_2B_HIGH();
        break;
    case 3:
        MOTOR_PHASE_1A_HIGH();
        MOTOR_PHASE_2A_LOW();
        MOTOR_PHASE_1B_HIGH();
        MOTOR_PHASE_2B_LOW();
        break;
    case 2:
        MOTOR_PHASE_1A_LOW();
        MOTOR_PHASE_2A_HIGH();
        MOTOR_PHASE_1B_HIGH();
        MOTOR_PHASE_2B_LOW();
        break;
    case 1:
        MOTOR_PHASE_1A_LOW();
        MOTOR_PHASE_2A_HIGH();
        MOTOR_PHASE_1B_LOW();
        MOTOR_PHASE_2B_HIGH();
        break;

                
#endif//#if defined(STAMP_BOARD)
    }
#endif  //#ifdef Half_Step

#else
#if defined(Half_Step) || (defined(TPH_JING_CHI)&&defined(TPH_JING_TEST))
    switch (tp.phase & 0x07)
    {
#if defined(STAMP_BOARD)
    case 0:
        MOTOR_PHASE_1A_HIGH();
        MOTOR_PHASE_1B_LOW();
        MOTOR_PHASE_2A_HIGH();
        MOTOR_PHASE_2B_LOW();
        break;
    case 1:
        MOTOR_PHASE_1A_HIGH();
        MOTOR_PHASE_1B_LOW();
        MOTOR_PHASE_2A_LOW();
        MOTOR_PHASE_2B_LOW();
        break;
    case 2:
        MOTOR_PHASE_1A_HIGH();
        MOTOR_PHASE_1B_LOW();
        MOTOR_PHASE_2A_LOW();
        MOTOR_PHASE_2B_HIGH();
        break;
    case 3:
        MOTOR_PHASE_1A_LOW();
        MOTOR_PHASE_1B_LOW();
        MOTOR_PHASE_2A_LOW();
        MOTOR_PHASE_2B_HIGH();
        break;
    case 4:
        MOTOR_PHASE_1A_LOW();
        MOTOR_PHASE_1B_HIGH();
        MOTOR_PHASE_2A_LOW();
        MOTOR_PHASE_2B_HIGH();
        break;
    case 5:
        MOTOR_PHASE_1A_LOW();
        MOTOR_PHASE_1B_HIGH();
        MOTOR_PHASE_2A_LOW();
        MOTOR_PHASE_2B_LOW();
        break;
    case 6:
        MOTOR_PHASE_1A_LOW();
        MOTOR_PHASE_1B_HIGH();
        MOTOR_PHASE_2A_HIGH();
        MOTOR_PHASE_2B_LOW();
        break;
    case 7:
        MOTOR_PHASE_1A_LOW();
        MOTOR_PHASE_1B_LOW();
        MOTOR_PHASE_2A_HIGH();
        MOTOR_PHASE_2B_LOW();
        break;
#else
    case 7:
        MOTOR_PHASE_1A_HIGH();
        MOTOR_PHASE_1B_LOW();
        MOTOR_PHASE_2A_HIGH();
        MOTOR_PHASE_2B_LOW();
        break;
    case 6:
        MOTOR_PHASE_1A_HIGH();
        MOTOR_PHASE_1B_LOW();
        MOTOR_PHASE_2A_LOW();
        MOTOR_PHASE_2B_LOW();
        break;
    case 5:
        MOTOR_PHASE_1A_HIGH();
        MOTOR_PHASE_1B_LOW();
        MOTOR_PHASE_2A_LOW();
        MOTOR_PHASE_2B_HIGH();
        break;
    case 4:
        MOTOR_PHASE_1A_LOW();
        MOTOR_PHASE_1B_LOW();
        MOTOR_PHASE_2A_LOW();
        MOTOR_PHASE_2B_HIGH();
        break;
    case 3:
        MOTOR_PHASE_1A_LOW();
        MOTOR_PHASE_1B_HIGH();
        MOTOR_PHASE_2A_LOW();
        MOTOR_PHASE_2B_HIGH();
        break;
    case 2:
        MOTOR_PHASE_1A_LOW();
        MOTOR_PHASE_1B_HIGH();
        MOTOR_PHASE_2A_LOW();
        MOTOR_PHASE_2B_LOW();
        break;
    case 1:
        MOTOR_PHASE_1A_LOW();
        MOTOR_PHASE_1B_HIGH();
        MOTOR_PHASE_2A_HIGH();
        MOTOR_PHASE_2B_LOW();
        break;
    case 0:
        MOTOR_PHASE_1A_LOW();
        MOTOR_PHASE_1B_LOW();
        MOTOR_PHASE_2A_HIGH();
        MOTOR_PHASE_2B_LOW();
        break;
#endif
    }

#else
    switch (tp.phase & 0x03)
    {
#if defined(STAMP_BOARD)
    case 0:
        MOTOR_PHASE_1A_HIGH();
        MOTOR_PHASE_1B_LOW();
        MOTOR_PHASE_2A_HIGH();
        MOTOR_PHASE_2B_LOW();
        break;
    case 1:
        MOTOR_PHASE_1A_HIGH();
        MOTOR_PHASE_1B_LOW();
        MOTOR_PHASE_2A_LOW();
        MOTOR_PHASE_2B_HIGH();
        break;
    case 2:
        MOTOR_PHASE_1A_LOW();
        MOTOR_PHASE_1B_HIGH();
        MOTOR_PHASE_2A_LOW();
        MOTOR_PHASE_2B_HIGH();
        break;
    case 3:
        MOTOR_PHASE_1A_LOW();
        MOTOR_PHASE_1B_HIGH();
        MOTOR_PHASE_2A_HIGH();
        MOTOR_PHASE_2B_LOW();
        break;
#else
    case 3:
        MOTOR_PHASE_1A_HIGH();
        MOTOR_PHASE_1B_LOW();
        MOTOR_PHASE_2A_HIGH();
        MOTOR_PHASE_2B_LOW();
        break;
    case 2:
        MOTOR_PHASE_1A_HIGH();
        MOTOR_PHASE_1B_LOW();
        MOTOR_PHASE_2A_LOW();
        MOTOR_PHASE_2B_HIGH();
        break;
    case 1:
        MOTOR_PHASE_1A_LOW();
        MOTOR_PHASE_1B_HIGH();
        MOTOR_PHASE_2A_LOW();
        MOTOR_PHASE_2B_HIGH();
        break;
    case 0:
        MOTOR_PHASE_1A_LOW();
        MOTOR_PHASE_1B_HIGH();
        MOTOR_PHASE_2A_HIGH();
        MOTOR_PHASE_2B_LOW();
        break;
#endif
    }
#endif
#endif
#if defined(BLACK_MARK_ENABLE)
    if(TPPrinterMark() == FALSE)        // find mark
    {
        tp.markbefore = 0;
    }
    else if(tp.markbefore >= 0)
    {
        tp.markbefore++;
    }
#endif

//    PRN_POWER_CHARGE();
    //sys_delay(30000);

    tp.phase += direction;
    pt_check_paper();


}

static uint8_t TPFeedStep(void)
{
    uint16_t time;
    if(aera_speed) {
        TPSetTimeCnt(TpAccelerationSteps[tp.accel]);
    } else {
        time  = TPGetStepTime();

        TPSetTimeCnt(time);  // set timer
    }


    if(TPPrinterReady() != TRUE )
    {
        aera_speed = 0;
        return 0;
    }

    if(tp.feedmax)
    {
        TPForwardStep(1);
        //DBG_STR("####  TPForwardStep(1) \r\n");
        tp.feedmax--;
    }
    if(tp.feedmax)
    {
        return 1;
    }
    else
    {
        aera_speed = 0;
        return 0;
    }
}

static void TPIntSetPreIdle(void)
{
    STROBE_0_OFF();     // stop heat
    STROBE_1_OFF();     // stop heat

    //tp.feedmax = 60*1;      // 每1ms中断一次
    tp.feedmax = 10*1;      // 每1ms中断一次

#if defined(HISTORY_HEAT_ENABLE)
    tp.history_cnt = 0;
    tp.first = 0;
    tp.last_max_cnt = 0;
#endif

    tp.state = TPSTATE_PRE_IDLE;
}

static void TPIntSetIdle(void)
{
    STROBE_0_OFF();     // stop heat
    STROBE_1_OFF();     // stop heat
    //DISABLE_TIMER_INTERRUPT();              // disable interrupt
    pt_timer_stop();
    MOTOR_PWR_OFF();

#if defined(HISTORY_HEAT_ENABLE)
    tp.history_cnt = 0;
    tp.first = 0;
    tp.last_max_cnt = 0;
#endif

    tp.state = TPSTATE_IDLE;

#if defined(NEW_HEAT_TIME)
    PrintOn_Flag = FALSE;
#endif
}

#if !defined(NEW_HEAT_TIME)
// 加热时间根据电压调整
static uint32_t TPHeatVoltageAdj(uint32_t tm)
{

#if defined(TP_VOLTAGE_SNS) && (!defined(NEW_HEAT_TIME))
    int8_t Voltage_Addr;
    //int16_t Temp_V;
    static uint32_t const Voltage_Ratio[]=
    {
#if 0//defined(DOUBLE_HEAT_ENABLE)
        TIMER1_MS_TO_CNT(5.30),//3.3V
        TIMER1_MS_TO_CNT(4.40),//3.4V
        TIMER1_MS_TO_CNT(4.00),//3.5V
        TIMER1_MS_TO_CNT(3.60),//3.6V
        TIMER1_MS_TO_CNT(3.30),//3.7V
        TIMER1_MS_TO_CNT(3.00),//3.8V
        TIMER1_MS_TO_CNT(2.80),//3.9V
        TIMER1_MS_TO_CNT(2.50),//4.0V
        TIMER1_MS_TO_CNT(2.20),//4.1V
        TIMER1_MS_TO_CNT(1.90),//4.2V
#else

#ifdef PT48D
        TIMER1_MS_TO_CNT(5.80),//3.3V  0
        TIMER1_MS_TO_CNT(5.20),//3.4V  1
        TIMER1_MS_TO_CNT(4.80),//3.5V  2
        TIMER1_MS_TO_CNT(4.00),//3.6V  3  4.00
        TIMER1_MS_TO_CNT(3.80),//3.7V  4  3.8
        TIMER1_MS_TO_CNT(3.70),//3.8V  5  3.7
        TIMER1_MS_TO_CNT(3.40),//3.9V  6  3.7
        TIMER1_MS_TO_CNT(2.50),//4.0V  7  2.6
        TIMER1_MS_TO_CNT(2.30),//4.1V  8
        TIMER1_MS_TO_CNT(2.10),//4.2V  9

        TIMER1_MS_TO_CNT(2.00),
        TIMER1_MS_TO_CNT(1.90),

#elif defined(PT487_100DPI)
        TIMER1_MS_TO_CNT(10.80),//3.3V  0
        TIMER1_MS_TO_CNT(10.20),//3.4V  1
        TIMER1_MS_TO_CNT(9.80),//3.5V  2
        TIMER1_MS_TO_CNT(9.00),//3.6V  3  4.00
        TIMER1_MS_TO_CNT(8.80),//3.7V  4  3.8
        TIMER1_MS_TO_CNT(8.70),//3.8V  5  3.7
        TIMER1_MS_TO_CNT(8.40),//3.9V  6  3.7
        TIMER1_MS_TO_CNT(8.50),//4.0V  7  2.6
        TIMER1_MS_TO_CNT(8.30),//4.1V  8
        TIMER1_MS_TO_CNT(8.10),//4.2V  9

#elif defined(PT48D_NEW_DENSITY)
#if defined(HIGH_SPEED)
#if defined(DOUBLE_HEAT_ENABLE)
        TIMER1_MS_TO_CNT(4.80/2),//3.3V
        TIMER1_MS_TO_CNT(4.50/2),//3.4V
        TIMER1_MS_TO_CNT(4.10/2),//3.5V
        TIMER1_MS_TO_CNT(3.50/2),//3.6V
        TIMER1_MS_TO_CNT(2.70/2),//3.7V
        TIMER1_MS_TO_CNT(2.50/2),//3.8V
        TIMER1_MS_TO_CNT(2.20/2),//3.9V
        TIMER1_MS_TO_CNT(1.90/2),//4.0V
        TIMER1_MS_TO_CNT(1.70/2),//4.1V
        TIMER1_MS_TO_CNT(1.50/2),//4.2V
#else
        TIMER1_MS_TO_CNT(4.80),//3.3V
        TIMER1_MS_TO_CNT(4.50),//3.4V
        TIMER1_MS_TO_CNT(4.10),//3.5V
        TIMER1_MS_TO_CNT(3.50),//3.6V
        TIMER1_MS_TO_CNT(2.70),//3.7V
        TIMER1_MS_TO_CNT(2.50),//3.8V
        TIMER1_MS_TO_CNT(2.20),//3.9V
        TIMER1_MS_TO_CNT(1.90),//4.0V
        TIMER1_MS_TO_CNT(1.70),//4.1V
        TIMER1_MS_TO_CNT(1.50),//4.2V
#endif
#else
        TIMER1_MS_TO_CNT(5.30),//3.3V
        TIMER1_MS_TO_CNT(5.00),//3.4V
        TIMER1_MS_TO_CNT(4.60),//3.5V
        TIMER1_MS_TO_CNT(4.20),//3.6V
        TIMER1_MS_TO_CNT(3.20),//3.7V
        TIMER1_MS_TO_CNT(3.00),//3.8V
        TIMER1_MS_TO_CNT(2.70),//3.9V
        TIMER1_MS_TO_CNT(2.40),//4.0V
        TIMER1_MS_TO_CNT(2.20),//4.1V
        TIMER1_MS_TO_CNT(2.00),//4.2V
#endif
#else
        TIMER1_MS_TO_CNT(5.80),//3.3V
        TIMER1_MS_TO_CNT(5.20),//3.4V
        TIMER1_MS_TO_CNT(4.80),//3.5V
        TIMER1_MS_TO_CNT(4.20),//3.6V
        TIMER1_MS_TO_CNT(3.60),//3.7V
        TIMER1_MS_TO_CNT(3.30),//3.8V
        TIMER1_MS_TO_CNT(3.10),//3.9V
        TIMER1_MS_TO_CNT(2.80),//4.0V
        TIMER1_MS_TO_CNT(2.60),//4.1V
        TIMER1_MS_TO_CNT(2.40),//4.2V
#endif

#endif
    };
#if defined(PT487_100DPI)
    Voltage_Addr = ((Power_AD*380*33)/(1024*180) - 33 -31+3);//3为修正值
#elif defined(PT48D_NEW_DENSITY)
    Voltage_Addr = ((Power_AD*49*33)/(1024*10) - 33+2);//3为修正值14  //30
#else
    Voltage_Addr = ((Power_AD*380*33)/(1024*180) - 33+3);//3为修正值14
#endif
    if(Voltage_Addr >= 0)
    {
#ifdef PT48D
        if(Voltage_Addr <= 11)
            tm = Voltage_Ratio[Voltage_Addr];
        else
            tm = Voltage_Ratio[11];
#else
        if(Voltage_Addr <= 9)
            tm = Voltage_Ratio[Voltage_Addr];
        else
            tm = Voltage_Ratio[9];
#endif
    }
    else
    {
        tm = Voltage_Ratio[0];
    }
#endif
    //Putchar(Voltage_Addr);

    return tm;
}
#endif

/*
static uint32_t getVoltageAdj(uint32_t mV)
{
    int index = -1, i = 0;
    uint32 voltage_list[10] = {3300,3400,3500,3600,3700,3800,3900,4000,4100,4200};
#if defined(TP_VOLTAGE_SNS) && (!defined(NEW_HEAT_TIME))
    int8_t Voltage_Addr;
    //int16_t Temp_V;
    static uint32_t const Voltage_Ratio[]=
    {
#if 0//defined(DOUBLE_HEAT_ENABLE)
        TIMER1_MS_TO_CNT(5.30),//3.3V
        TIMER1_MS_TO_CNT(4.40),//3.4V
        TIMER1_MS_TO_CNT(4.00),//3.5V
        TIMER1_MS_TO_CNT(3.60),//3.6V
        TIMER1_MS_TO_CNT(3.30),//3.7V
        TIMER1_MS_TO_CNT(3.00),//3.8V
        TIMER1_MS_TO_CNT(2.80),//3.9V
        TIMER1_MS_TO_CNT(2.50),//4.0V
        TIMER1_MS_TO_CNT(2.20),//4.1V
        TIMER1_MS_TO_CNT(1.90),//4.2V
#else

#ifdef PT48D
        TIMER1_MS_TO_CNT(5.80),//3.3V  0
        TIMER1_MS_TO_CNT(5.20),//3.4V  1
        TIMER1_MS_TO_CNT(4.80),//3.5V  2
        TIMER1_MS_TO_CNT(4.00),//3.6V  3  4.00
        TIMER1_MS_TO_CNT(3.80),//3.7V  4  3.8
        TIMER1_MS_TO_CNT(3.70),//3.8V  5  3.7
        TIMER1_MS_TO_CNT(3.40),//3.9V  6  3.7
        TIMER1_MS_TO_CNT(2.50),//4.0V  7  2.6
        TIMER1_MS_TO_CNT(2.30),//4.1V  8
        TIMER1_MS_TO_CNT(2.10),//4.2V  9

        TIMER1_MS_TO_CNT(2.00),
        TIMER1_MS_TO_CNT(1.90),

#elif defined(PT487_100DPI)
        TIMER1_MS_TO_CNT(10.80),//3.3V  0
        TIMER1_MS_TO_CNT(10.20),//3.4V  1
        TIMER1_MS_TO_CNT(9.80),//3.5V  2
        TIMER1_MS_TO_CNT(9.00),//3.6V  3  4.00
        TIMER1_MS_TO_CNT(8.80),//3.7V  4  3.8
        TIMER1_MS_TO_CNT(8.70),//3.8V  5  3.7
        TIMER1_MS_TO_CNT(8.40),//3.9V  6  3.7
        TIMER1_MS_TO_CNT(8.50),//4.0V  7  2.6
        TIMER1_MS_TO_CNT(8.30),//4.1V  8
        TIMER1_MS_TO_CNT(8.10),//4.2V  9

#elif defined(PT48D_NEW_DENSITY)
#if defined(HIGH_SPEED)
#if defined(DOUBLE_HEAT_ENABLE)
        TIMER1_MS_TO_CNT(4.80/2),//3.3V
        TIMER1_MS_TO_CNT(4.50/2),//3.4V
        TIMER1_MS_TO_CNT(4.10/2),//3.5V
        TIMER1_MS_TO_CNT(3.50/2),//3.6V
        TIMER1_MS_TO_CNT(2.70/2),//3.7V
        TIMER1_MS_TO_CNT(2.50/2),//3.8V
        TIMER1_MS_TO_CNT(2.20/2),//3.9V
        TIMER1_MS_TO_CNT(1.90/2),//4.0V
        TIMER1_MS_TO_CNT(1.70/2),//4.1V
        TIMER1_MS_TO_CNT(1.50/2),//4.2V
#else
        TIMER1_MS_TO_CNT(4.80),//3.3V
        TIMER1_MS_TO_CNT(4.50),//3.4V
        TIMER1_MS_TO_CNT(4.10),//3.5V
        TIMER1_MS_TO_CNT(3.50),//3.6V
        TIMER1_MS_TO_CNT(2.70),//3.7V
        TIMER1_MS_TO_CNT(2.50),//3.8V
        TIMER1_MS_TO_CNT(2.20),//3.9V
        TIMER1_MS_TO_CNT(1.90),//4.0V
        TIMER1_MS_TO_CNT(1.70),//4.1V
        TIMER1_MS_TO_CNT(1.50),//4.2V
#endif
#else
        TIMER1_MS_TO_CNT(5.30),//3.3V
        TIMER1_MS_TO_CNT(5.00),//3.4V
        TIMER1_MS_TO_CNT(4.60),//3.5V
        TIMER1_MS_TO_CNT(4.20),//3.6V
        TIMER1_MS_TO_CNT(3.20),//3.7V
        TIMER1_MS_TO_CNT(3.00),//3.8V
        TIMER1_MS_TO_CNT(2.70),//3.9V
        TIMER1_MS_TO_CNT(2.40),//4.0V
        TIMER1_MS_TO_CNT(2.20),//4.1V
        TIMER1_MS_TO_CNT(2.00),//4.2V
#endif
#else
        TIMER1_MS_TO_CNT(5.80),//3.3V
        TIMER1_MS_TO_CNT(5.20),//3.4V
        TIMER1_MS_TO_CNT(4.80),//3.5V
        TIMER1_MS_TO_CNT(4.20),//3.6V
        TIMER1_MS_TO_CNT(3.60),//3.7V
        TIMER1_MS_TO_CNT(3.30),//3.8V
        TIMER1_MS_TO_CNT(3.10),//3.9V
        TIMER1_MS_TO_CNT(2.80),//4.0V
        TIMER1_MS_TO_CNT(2.60),//4.1V
        TIMER1_MS_TO_CNT(2.40),//4.2V
#endif

#endif
    };
    //Putchar(Voltage_Addr);

    for(i = 1; i < 10; i++)
    {
        if(mV < voltage_list[i])
        {
            index = i - 1;
            break;
        }
    }
    if(mV > voltage_list[9])
    {
        index = 9;
    }
    
    return Voltage_Ratio[index];
}
*/


/*
   extern void WakeUpTP_MODE1(void)
   {
   volatile uint16_t len;

   PrintBufToZero();
   esc_sts.bitmap_flag = 0;
   memset(esc_sts.dot, 0, sizeof(esc_sts.dot));
   esc_sts.start_dot = 0;
   max_start_col =0;
   esc_sts.dot_minrow = ARRAY_SIZE(esc_sts.dot[0]);

   len =  LineDot/8;
   while (len--)
   {
   SSP0_SendData(0);
   }
   LATCH_LOW();
   len = 100;
   while (len--);
   LATCH_HIGH();
   tp.tail=tp.head;
   TPIntSetIdle();
   clr_all_dot=1;
   }
 */



uint16_t MaxHeatDotsAdj(uint16_t dots)
{

    uint16_t max_heat_dot;//新的最大值

    max_heat_dot = dots/(dots/(TP_MAX_HEAT_DOT+1)+1);//总加热点数加热次数
    if ((max_heat_dot+1) <= TP_MAX_HEAT_DOT)         //8个点的误差，引起多一次的加热
        max_heat_dot += 1;
    else
        max_heat_dot = TP_MAX_HEAT_DOT;
    return max_heat_dot;
}

extern void TPSetHeatDots(uint8_t dot)
{
    if(dot > 2)return;

    if(dot == 0)//取消指令选择加热点数
    {
        heat_dot_setting = 0;
        choose_dot = 1;//底下2句没什么用，逻辑需要
        TP_MAX_HEAT_DOT = MAX_HEAT_ADJ;
    }
    else
    {
        heat_dot_setting = 1;
        if(dot == 1)
        {
            choose_dot = 1;
            TP_MAX_HEAT_DOT = MAX_HEAT_ADJ;
        }
        else
        {
            choose_dot = 0;
            TP_MAX_HEAT_DOT = 64;
        }
    }
}


static uint32_t TPHeatDotsAdj(uint32_t tm,uint16_t dots)
{
    static uint8_t dot_ratio_tbl[64/4]=
    {
        80, 85, 88, 90, 93, 94,//0-24
        96, 98, 99, 99, 99, 99,
        110,130,130,130,
    };

//#if (MAX_HEAT_ADJ == 96)
    static uint8_t  dot_ratio_tbl96[96/4]=
    {
        78, 79, 80, 81, 82, 83,//0-24
        84, 85, 86, 87, 88, 89,
        100,101,102,103,114,115,
        116,117,118,119,120,120,
    };
//#elif (MAX_HEAT_ADJ == 80)
    static uint8_t  dot_ratio_tbl80[80/4]=
    {
        78, 79, 80, 81, 82, 83,//0-24
        84, 85, 86, 87, 88, 89,
        100,101,102,103,114,115,
        116,117,
    };
//#elif (MAX_HEAT_ADJ == 72)
    static uint8_t  dot_ratio_tbl72[72/4]=
    {
        78, 79, 80, 81, 82, 83,//0-24
        84, 85, 86, 87, 88, 89,
        100,101,102,103,114,115,
    };
//#elif (MAX_HEAT_ADJ == 128)
    const uint8_t dot_ratio_tbl128[128/4]=
    {
        #ifdef MACHINE_P7
        80, 81, 82, 83, 84, 85,//0-24
        86, 87, 88, 89, 90, 91,
        #else
        78, 79, 80, 81, 82, 83,//0-24
        84, 85, 86, 87, 88, 89,
        #endif
        100,101,102,103,114,115,
        116,117,118,119,120,120,
        120,120,120,120,120,120,
        120,120,
    };
//#endif


#if defined(PT48D)
    if(dots<TP_MAX_HEAT_DOT)
    {
        tm = tm * (dot_ratio_tbl[dots/4]+5)/100;
    }
    else
    {
        tm = tm * (dot_ratio_tbl[TP_MAX_HEAT_DOT/4-1]+34)/100;//+18
    }
#elif defined(PT48D_NEW_DENSITY) && (!defined(NEW_HEAT_TIME))

#if defined(HIGH_SPEED)
    if(dots<TP_MAX_HEAT_DOT)
    {
        tm = tm * (dot_ratio_tbl[dots/4]+5)/100;
    }
    else
    {
        tm = tm * (dot_ratio_tbl[TP_MAX_HEAT_DOT/4-1]+34)/100;//+18
    }
#else
    if(dots<TP_MAX_HEAT_DOT)
    {
        tm = tm * (dot_ratio_tbl[dots/4]-5)/100;
    }
    else
    {
        tm = tm * (dot_ratio_tbl[TP_MAX_HEAT_DOT/4-1]+34)/100;//+18
    }
#endif
#elif defined(NEW_HEAT_TIME)
    if(choose_dot)
    {
    if (TP_MAX_HEAT_DOT == 128) {
        if(dots<128)
        {
            tm = tm * dot_ratio_tbl128[dots/4] * heat_cnt_to_ratio96[tp.heat_max_cnt]/10000;
        }
        else
        {
            tm = tm * dot_ratio_tbl128[128/4-1]* heat_cnt_to_ratio96[tp.heat_max_cnt]/10000;
        }
    } else if (TP_MAX_HEAT_DOT == 96){
        if(dots<96)
        {
            tm = tm * dot_ratio_tbl96[dots/4] * heat_cnt_to_ratio96[tp.heat_max_cnt]/10000;
        }
        else
        {
            tm = tm * dot_ratio_tbl96[96/4-1]* heat_cnt_to_ratio96[tp.heat_max_cnt]/10000;
        }
    } else if (TP_MAX_HEAT_DOT == 80) {
        if(dots<80)
        {
            tm = tm * dot_ratio_tbl80[dots/4] * heat_cnt_to_ratio96[tp.heat_max_cnt]/10000;
        }
        else
        {
            tm = tm * dot_ratio_tbl80[80/4-1]* heat_cnt_to_ratio96[tp.heat_max_cnt]/10000;
        }
    } else if (TP_MAX_HEAT_DOT == 72) {
        if(dots<72)
        {
            tm = tm * dot_ratio_tbl72[dots/4] * heat_cnt_to_ratio96[tp.heat_max_cnt]/10000;
        }
        else
        {
            tm = tm * dot_ratio_tbl72[72/4-1]* heat_cnt_to_ratio96[tp.heat_max_cnt]/10000;
        }
    } else {
        if(dots<TP_MAX_HEAT_DOT)
        {
            //char cTemp[64];
            
            //sprintf(cTemp,"tp.heat_max_cnt = %d \r\n",tp.heat_max_cnt);
            //drv_com_write(DEBUG_PORT_NO,cTemp,strlen(cTemp));

            //sprintf(cTemp,"dots/4 = %d \r\n",dots/4);
            //drv_com_write(DEBUG_PORT_NO,cTemp,strlen(cTemp));
                
            tm = tm * dot_ratio_tbl[dots/4] * heat_cnt_to_ratio[tp.heat_max_cnt]/10000;
        }
        else
        {
            tm = tm * dot_ratio_tbl[64/4-1]* heat_cnt_to_ratio[tp.heat_max_cnt]/10000;
        }
    }
        }
#else
    if(dots<TP_MAX_HEAT_DOT)
    {
        tm = tm * (dot_ratio_tbl[dots/4])/100;
    }
    else
    {
        tm = tm * (dot_ratio_tbl[TP_MAX_HEAT_DOT/4-1])/100;//+18
    }

#endif
    return tm;
}


uint8_t const Battery_Voltage_Ratio[]=
{

    80,//435
    80,
    80,
    80,
    80,   
    80,//430
    80,
    80,
    80,
    80,
    81,//425
    82,
    83,
    84,
    85,
    86,//420
    86,
    86,
    86,
    88,
    90,//415
    92,
    94,
    96,
    98,
    // 比例      电压
    100,        //410
    101,
    102,
    103,
    104,
    105,         //405
    106,
    107,
    108,
    109,
    110,         //400
    111,
    112,
    113,
    114,
    115,         //395
    116,
    117,
    118,
    119,
    120,         //390
    121,
    122,
    123,
    124,
    125,         //385
    126,
    127,
    128,
    129,
    130,         //380
    131,
    132,
    133,
    134,
    135,         //375
    136,
    137,
    138,
    139,
    140,         //370
    140,         //
    140,         //
    140,         //
    140,         //
    140,         //365
    140,         //
    140,         //
    140,         //
    140,         //
    140,         //360
    140,         //
    140,         //
    140,         //
    140,         //
    140,         //355
    140,         //
    140,         //
    140,         //
    140,         //
    140,         //350
    140,         //
};
uint8_t const Density_Ratio[]=
{
    // 比例      浓度
    100,        ///5
    100,        ///7
    100,        ///9
    100,
};

struct __vol_speed_map{
    uint16_t vol;
    uint16_t speed;
};

struct __vol_speed_map vol_speed_map[] = {
        {660, 1200},
        {710, 1100},
        {760, 1000},
        {800, 850},
        {840, 750}
};

extern void BatteryVoltageADJ(uint16_t bat_vol)
{//根据电压和浓度调整
    int i;
    uint16_t speed_tmp = 1000;
    char cTemp[64];

    if(bat_vol < vol_speed_map[0].vol) {
            speed_tmp = vol_speed_map[0].speed;
    } else {
        for(i = 1; i < sizeof(vol_speed_map)/sizeof(vol_speed_map[0]); i++)
        {
            if(bat_vol <= vol_speed_map[i].vol)
            {
                speed_tmp =  vol_speed_map[i-1].speed - (bat_vol - vol_speed_map[i-1].vol)
                    *((vol_speed_map[i-1].speed-vol_speed_map[i].speed)/ (vol_speed_map[i].vol - vol_speed_map[i-1].vol));
                break;
            }
        }
    }
    tp.heat_adj = tp.heat_adj * speed_tmp / 1000;
    //sprintf(cTemp,"11111111111111111111111 tp.heat_adj = %d  bat_vol = %d \r\n",tp.heat_adj, bat_vol);
    //drv_com_write(DEBUG_PORT_NO,cTemp,strlen(cTemp));
}

static uint32_t TPHeatThermalAdj(uint32_t tm,int16_t temp)
{
    char cTemp[64];
    
    //sprintf(cTemp,"temp = %d \r\n",temp);
    //drv_com_write(DEBUG_PORT_NO,cTemp,strlen(cTemp));

    if((temp < 25)&&((temp >= (-20))))
    {
        tm = tm *Heat_TemperaterLow_Ratio[(25 - temp)]/100;
    }
    else if ((temp >= 25)&&(temp <= 80))//75))
    {
        tm = tm *Heat_Temperater_Ratio[(temp-25)]/100;
    }
    else if (temp>80)//75)
    {
        tm = tm * 25 /100;   //超过65度，按50%加热
    }

    return tm;
}

extern uint32_t TPCheckBusy(void)
{
	if (tp.state == TPSTATE_IDLE)
	{
		return FALSE;
	}
	return TRUE;
}

extern void TPFeedToMark(uint16_t line)
{
	line <<= 1;		// 一个点行等于两步
	TPPrintCmdToBuf(TP_CMD_FEED_TO_MARK, (uint8_t *)(&line), sizeof(line));
}

//======================================================================================================
extern uint8_t IsPrintBufEmpty(void)
{
	if(tp.head != tp.tail)	// have data
	{
		return 0;
	}
	else
	{
		return 1;
	}
}


extern void TPPrintTestPage(void)
{
        uint32_t len,i;
        char buf[64];

        return;

#if 0
        char const BC_SET_HRI[]={0x1d, 'H', 0x02};
        char const BC_EAN13[]={0x1d, 0x6b, 0x43, 0x0d, '6', '9', '0', '1', '0', '2', '8', '0', '7', '5', '8', '3', '1'};
        #if defined(SUPPORT_ALL_BARCODE)
        char const BC_CODE39[]={0x1d, 0x6b, 0x45, 0x04, '1', '2', '3', '4'};
        char const BC_UPCA[]={0x1d, 0x6b, 0x41, 0x0c, '0', '7', '5', '6', '7', '8', '1', '6', '4', '1', '2', '5'};
        char const BC_UPCE[]={0x1d, 0x6b, 0x42, 0x0c, '0', '4', '2', '1', '0', '0', '0', '0', '5', '2', '6', '4'};
        char const BC_EAN8[]={0x1d, 0x6b, 0x44, 0x08, '0', '4', '2', '1', '0', '0', '0', '9'};
        char const BC_ITF[]={0x1d, 0x6b, 0x46, 0x0a, '9', '8', '7', '6', '5', '4', '3', '2', '1', '0'};
        char const BC_CODABAR[]={0x1d, 0x6b, 0x47, 0x07, 'A', '4', '0', '1', '5', '6', 'B'};
        char const BC_CODE93[]={0x1d, 0x6b, 0x48, 0x06, 'T', 'E', 'S', 'T', '9', '3'};
        char const BC_CODE128[]={0x1d, 0x6b, 0x49, 0x0a, '{', 'A', 'H', 'I', '{', 'C', '3', '4', '5', '6'};
        #else
        char const BC_CODE39[]={0x1d, 0x6b, 0x45, 0x09, '1', '2', '3', '4', '5', '6', '7', '8', '9'};
        #endif
#if defined(SELF_TEST_BITMAP_SAMPLE)
		char const BITMAP[]={0x1c, 0x70, 0x01,0x00};
#endif
        char const Init[]={0x1b,'@'};



        PrintBufToZero();
        memcpy(buf, Init, sizeof(Init));
        TPPrintAsciiLine(buf, sizeof(Init));

        len = snprintf(buf, sizeof(buf),  "\n");
        TPPrintAsciiLine(buf,len);
    #if defined(PT486)
        len = snprintf(buf, sizeof(buf), "System: PT486LV\n");
    #elif defined(PT488)
        len = snprintf(buf, sizeof(buf), "System: PT488LV\n");
    #elif defined(PT487)
        len = snprintf(buf, sizeof(buf), "System: PT487LV\n");
    #elif defined(PT48D) || defined(PT48D_NEW_DENSITY)
	    len = snprintf(buf, sizeof(buf), "System: PT48DLV\n");
    #endif
        TPPrintAsciiLine(buf,len);

#if (VERSION_BETA==0)
    #ifdef CODEPAGE
            len = snprintf(buf, sizeof(buf), "Firmware:%d.%02d.%02dC LV\n", VERSION_MAJOR, VERSION_MINOR,VERSION_TEST);
    #else
            len = snprintf(buf, sizeof(buf), "Firmware:%d.%02d.%02d LV\n", VERSION_MAJOR, VERSION_MINOR,VERSION_TEST);
    #endif
#else
    #ifdef CODEPAGE
            len = snprintf(buf, sizeof(buf), "Firmware:%d.%02d.%02d beta%d C LV\n", VERSION_MAJOR, VERSION_MINOR,VERSION_TEST,VERSION_BETA);
    #else
            len = snprintf(buf, sizeof(buf), "Firmware:%d.%02d.%02d beta%d LV\n", VERSION_MAJOR, VERSION_MINOR,VERSION_TEST,VERSION_BETA);
    #endif
#endif
        TPPrintAsciiLine(buf,len);

        len = snprintf(buf, sizeof(buf), "Build date: %s\n", __DATE__);
        TPPrintAsciiLine(buf,len);

        len = snprintf(buf, sizeof(buf), "Build time: %s\n", __TIME__);
        TPPrintAsciiLine(buf,len);
#if 0
		len = snprintf(buf, sizeof(buf), "Voltage: %d.%dV\n", ((Power_AD*380*33)/(1024*180))/10,((Power_AD*380*33)/(1024*180))%10);
        TPPrintAsciiLine(buf,len);

	    len = snprintf(buf, sizeof(buf), "Temperature: %d℃\n",TPHTemperature());
        TPPrintAsciiLine(buf,len);
#endif
        len = snprintf(buf, sizeof(buf),  "\n");
        TPPrintAsciiLine(buf,len);

   	    len = snprintf(buf, sizeof(buf),  "[Uart Configure]\n");
        TPPrintAsciiLine(buf,len);

	    len = snprintf(buf, sizeof(buf),  "baudrate : %ld\n", config_idx2baud(para.com_baud));
        TPPrintAsciiLine(buf,len);

        len = snprintf(buf, sizeof(buf),  "flow ctrl : HW Flow Control\n");
        TPPrintAsciiLine(buf,len);

        len = snprintf(buf, sizeof(buf),  "\n");
        TPPrintAsciiLine(buf,len);

        len = snprintf(buf, sizeof(buf),  "[Install Fonts]\n");
        TPPrintAsciiLine(buf,len);

        len = snprintf(buf, sizeof(buf),  "ID  Font Name\n");
        TPPrintAsciiLine(buf,len);


        if(esc_sts.font_en == FONT_A_WIDTH)
        {
        len = snprintf(buf, sizeof(buf),  " 0  SYSTEM 12x24\n");
        TPPrintAsciiLine(buf,len);
        }
        else
        {
        len = snprintf(buf, sizeof(buf),  " 1  SYSTEM 8x16\n");
        TPPrintAsciiLine(buf,len);
        }
#ifndef CODEPAGE
        len = snprintf(buf, sizeof(buf),  "10  24x24GBK汉字字库\n");
        TPPrintAsciiLine(buf,len);
#endif
        len = snprintf(buf, sizeof(buf),  "\n");
        TPPrintAsciiLine(buf,len);

        len = snprintf(buf, sizeof(buf),  "[Barcode Samples]\n");
        TPPrintAsciiLine(buf, len);

        memcpy(buf, BC_SET_HRI, sizeof(BC_SET_HRI));
        TPPrintAsciiLine(buf, sizeof(BC_SET_HRI));

        #if defined(SUPPORT_ALL_BARCODE)
        len = snprintf(buf, sizeof(buf),  "UPCA:\n");
        TPPrintAsciiLine(buf, len);
        memcpy(buf, BC_UPCA, sizeof(BC_UPCA));
        TPPrintAsciiLine(buf, sizeof(BC_UPCA));

        len = snprintf(buf, sizeof(buf),  "UPCE:\n");
        TPPrintAsciiLine(buf, len);
        memcpy(buf, BC_UPCE, sizeof(BC_UPCE));
        TPPrintAsciiLine(buf, sizeof(BC_UPCE));

        len = snprintf(buf, sizeof(buf),  "EAN8:\n");
        TPPrintAsciiLine(buf, len);
        memcpy(buf, BC_EAN8, sizeof(BC_EAN8));
        TPPrintAsciiLine(buf, sizeof(BC_EAN8));

        len = snprintf(buf, sizeof(buf),  "EAN13:\n");
        TPPrintAsciiLine(buf, len);
        memcpy(buf, BC_EAN13, sizeof(BC_EAN13));
        TPPrintAsciiLine(buf, sizeof(BC_EAN13));

        len = snprintf(buf, sizeof(buf),  "CODE39:\n");
        TPPrintAsciiLine(buf, len);
        memcpy(buf, BC_CODE39, sizeof(BC_CODE39));
        TPPrintAsciiLine(buf, sizeof(BC_CODE39));

        len = snprintf(buf, sizeof(buf),  "ITF:\n");
        TPPrintAsciiLine(buf, len);
        memcpy(buf, BC_ITF, sizeof(BC_ITF));
        TPPrintAsciiLine(buf, sizeof(BC_ITF));

        len = snprintf(buf, sizeof(buf),  "CODABAR:\n");
        TPPrintAsciiLine(buf, len);
        memcpy(buf, BC_CODABAR, sizeof(BC_CODABAR));
        TPPrintAsciiLine(buf, sizeof(BC_CODABAR));

        len = snprintf(buf, sizeof(buf),  "CODE93:\n");
        TPPrintAsciiLine(buf, len);
        memcpy(buf, BC_CODE93, sizeof(BC_CODE93));
        TPPrintAsciiLine(buf, sizeof(BC_CODE93));

        len = snprintf(buf, sizeof(buf),  "CODE128:\n");
        TPPrintAsciiLine(buf, len);
        memcpy(buf, BC_CODE128, sizeof(BC_CODE128));
        TPPrintAsciiLine(buf, sizeof(BC_CODE128));
        #else
        len = snprintf(buf, sizeof(buf),  "EAN13:\n");
        TPPrintAsciiLine(buf,len);
        memcpy(buf, BC_EAN13, sizeof(BC_EAN13));
        TPPrintAsciiLine(buf, sizeof(BC_EAN13));

        len = snprintf(buf, sizeof(buf),  "CODE39:\n");
        TPPrintAsciiLine(buf,len);
        memcpy(buf, BC_CODE39, sizeof(BC_CODE39));
        TPPrintAsciiLine(buf, sizeof(BC_CODE39));
        #endif

        len = snprintf(buf, sizeof(buf),  "\n");
        TPPrintAsciiLine(buf,len);

        len = snprintf(buf, sizeof(buf),  "[ASCII Samples]\n");
        TPPrintAsciiLine(buf,len);
        for(i=0x20; i<0x80; i++)
        {
     //       PrintBufPushBytes(i);
        }

        len = snprintf(buf, sizeof(buf),  "\n\n");
        TPPrintAsciiLine(buf,len);

#if defined(SELF_TEST_BITMAP_SAMPLE)
		len = snprintf(buf, sizeof(buf),  "[Bitmap Samples]\n\n");
        TPPrintAsciiLine(buf,len);

        memcpy(buf, BITMAP, sizeof(BITMAP));
        TPPrintAsciiLine(buf, sizeof(BITMAP));

        len = snprintf(buf, sizeof(buf),  "\n\n");
        TPPrintAsciiLine(buf,len);
#endif
        len = snprintf(buf, sizeof(buf),  "Selftest Finished.\n");
        TPPrintAsciiLine(buf,len);

        len = snprintf(buf, sizeof(buf),  "\n\n\n\n\n");
        TPPrintAsciiLine(buf,len);

#endif
}


// 加热前几行的加热时间调整
static uint32_t TPHeatPreLineAdj(uint32_t tm)
{
    //  TODO:
    static const uint8_t TpPrelineAdjTbl[] = {
        //150, 148, 146, 144, 142, 
        140, 138, 136, 134, 132, 
        130, 128, 126, 124, 122, 
        120, 118, 116, 114, 112, 
        110, 108, 106, 104, 102, 100,
    };

    if (tp.accel < ARRAY_SIZE(TpPrelineAdjTbl)) {
        tm = tm * TpPrelineAdjTbl[tp.accel] / 100;
    }
    /*
    if(tp.accel < 10){
        tm += tm * 15 / 100;
    }else if(tp.accel < 15){
        tm += tm * 13 / 100;
    }*/
    
    return tm;
}

static int16_t temperature_bak = 25;
void set_temperature(int16_t temp)
{
    temperature_bak = temp;
}

int16_t get_temperature(void)
{
    return temperature_bak;
}

static void TPAdjustStepTime(uint8_t heat_cnt,uint16_t max_heat_dots)
{
    uint32_t heat;
    uint32_t time, time_sum;
    uint16_t i;
    int16_t temp;
    char cTemp[64];

    //drv_com_write(DEBUG_PORT_NO,"TPAdjustStepTime \r\n",strlen("TPAdjustStepTime \r\n"));

#if defined(NEW_HEAT_TIME)

 #if !defined(ADJ_PRINT_BY_VOL)
     //BatteryVoltageADJ(830);
    heat = tp.heat_adj;
 //    sprintf(cTemp,"heat 1 = %d \r\n",heat);
//    drv_com_write(DEBUG_PORT_NO,cTemp,strlen(cTemp));
 #else
    heat = tp.heat_adj;//去掉电压调整浓度
 #endif

#else
    heat = TPHeatVoltageAdj(tp.heat_setting);
#endif

    

    //sprintf(cTemp,"heat 1 = %d \r\n",heat);
    //drv_com_write(DEBUG_PORT_NO,cTemp,strlen(cTemp));

    heat = TPHeatDotsAdj(heat,max_heat_dots);

#if defined(TEMP_SNS_ENABLE)
    //sprintf(cTemp,"heat 2= %d \r\n",heat);
    //drv_com_write(DEBUG_PORT_NO,cTemp,strlen(cTemp));
    temp = TPHTemperature();
    set_temperature(temp);
    heat = TPHeatThermalAdj(heat, temp);//TPHTemperature());
  //      sprintf(cTemp,"heat 2= %d  temp = %d \r\n",heat, temp);
//    drv_com_write(DEBUG_PORT_NO,cTemp,strlen(cTemp));
#endif

    //sprintf(cTemp,"heat 3= %d \r\n",heat);
    //drv_com_write(DEBUG_PORT_NO,cTemp,strlen(cTemp));

    heat = TPHeatPreLineAdj(heat);

    //sprintf(cTemp,"heat 4= %d \r\n",heat);
    //drv_com_write(DEBUG_PORT_NO,cTemp,strlen(cTemp));

#if defined(HISTORY_HEAT_ENABLE)
    switch(tp.last_max_cnt )
    {
        //    #if (TP_MAX_HEAT_DOT == 128)
        //   case 1:
        //      tp.heat_percent = heat*65/100;// 55
        //      break;
        //   case 2:
        //        tp.heat_percent = heat*95/100;// 65
        //        break;
        //   case 3:
        //        tp.heat_percent = heat*100/100;//90
        //        break;
        //    #elif (TP_MAX_HEAT_DOT == 96)
    case 1:
        tp.heat_percent = heat*80/100;// 40 70
        break;
    case 2:
        tp.heat_percent = heat*50/100;//*80/100;// 80
        break;
    case 3:
        tp.heat_percent = heat*100/100;//90
        break;
        //    #else
        //    case 1:
        //      tp.heat_percent = heat*55/100;// 55
        //      break;
        //   case 2:
        //        tp.heat_percent = heat*65/100;// 65
        //        break;
        //   case 3:
        //        tp.heat_percent = heat*90/100;//90
        //        break;
        //    #endif
    case 4:
        //      tp.heat_percent = heat*75/100;//75
        //      break;
    case 5:
        //        tp.heat_percent = heat*80/100;//80
        //        break;
    case 6:
        //        tp.heat_percent = heat*90/100;//90
        //        break;

    case 0:
    default:
        tp.heat_percent = heat;
        break;
    }
    tp.heat = heat - tp.heat_percent;
    tp.last_max_cnt = heat_cnt;

    //sprintf(cTemp,"heat 5= %d \r\n",heat);
    //drv_com_write(DEBUG_PORT_NO,cTemp,strlen(cTemp));

    //sprintf(cTemp,"tp.heat_percent= %d \r\n",tp.heat_percent);
    //drv_com_write(DEBUG_PORT_NO,cTemp,strlen(cTemp));

    //sprintf(cTemp,"heat_cnt= %d \r\n",heat_cnt);
    //drv_com_write(DEBUG_PORT_NO,cTemp,strlen(cTemp));
    
    //printf("%d\n", tp.last_max_cnt);
#else
    tp.heat = heat;
#endif

    heat *= heat_cnt;
    heat += TpMinWaitTime;

    //sprintf(cTemp,"heat 6= %d \r\n",heat);
    //drv_com_write(DEBUG_PORT_NO,cTemp,strlen(cTemp));

    //if(heat_cnt == 1)
    //   heat += TpMinWaitTime*2;
    while(1)
    {
        time_sum = 0;
        for(i=0; i<TP_MAX_STROBE_STEP; i++)
        {

            if((tp.accel+i) < (ARRAY_SIZE(TpAccelerationSteps) - 1))//加速表内继续加速
            {
                time = TpAccelerationSteps[tp.accel+i];
            /*    if((battery_voltage > 375) && (time < 62500/55)) //限速55mm/s
                {
                    time = 62500/55;
                } else if((battery_voltage <= 375) && (time < 62500/50)) //限速50mm/s
                {
                    time = 62500/50;
                }*/    
            }
            else
            {
                time = TpAccelerationSteps[ARRAY_SIZE(TpAccelerationSteps)-1];//加速表最后一步
                /*if((battery_voltage > 375) && (time < 62500/55)) //限速55mm/s
                {
                    time = 62500/55;
                } else if((battery_voltage <= 375) && (time < 62500/50)) //限速50mm/s
                {
                    time = 62500/50;
                }
                */
            }

            //sprintf(cTemp,"time = %d \r\n",time);
            //drv_com_write(DEBUG_PORT_NO,cTemp,strlen(cTemp));
                        
            tp.feed_time[i] = time;
            time_sum += time;
        }
                
        if(time_sum < heat)
        {
            if(tp.accel)//原来的变速速程序会直接跳变
            {
                tp.accel--;
            }
            else    // bug
            {
                for(i=0; i<TP_MAX_STROBE_STEP; i++)
                {
                    tp.feed_time[i] = heat/TP_MAX_STROBE_STEP;      // 加速时间均分
                }
                break;
            }
        }
        else
        {
            if((tp.accel+TP_MAX_STROBE_STEP) < (ARRAY_SIZE(TpAccelerationSteps)-1))
            {
                tp.accel += TP_MAX_STROBE_STEP;
            }
            else
            {
                tp.accel = (ARRAY_SIZE(TpAccelerationSteps)-1);
            }
            break;
        }
    }

    //sprintf(cTemp,"tp.accel = %d \r\n",tp.accel);
    //drv_com_write(DEBUG_PORT_NO,cTemp,strlen(cTemp));

}

//#define BYTE_BIT_CAL_ENABLE 1
#if defined(BYTE_BIT_CAL_ENABLE)
static void TPDataShiftCntProc(uint8_t strobe_cnt)
{
    static uint8_t const Byte2DotNumTbl[] =
    {
        0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4,
        1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
        1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
        2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
        1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
        2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
        2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
        3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
        1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
        2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
        2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
        3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
        2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
        3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
        3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
        4,5,5,6,5,6,6,7,5,6,6,7,6,7,7,8
    };
    uint8_t c;          // 当前字节
    uint8_t dot;        // 当前字节的点数
    uint8_t heat_cnt;   // 分开几次加热
    uint16_t max_dot;   // 加热点累加和
    uint16_t i,j, pt;       // 行缓冲区指针
    uint16_t max_heat_dots=0;

    for(i=0, pt=((LineDot/8/TP_MAX_HEAT_STROBE)*strobe_cnt); i<LineDot/8/TP_MAX_HEAT_STROBE; i++, pt++)
    {
       c = TP_dot[tp.tail][pt];
       dot = Byte2DotNumTbl[c];
       max_heat_dots += dot;
    }
    max_heat_dots = MaxHeatDotsAdj(max_heat_dots);

    memset(tp.heat_buf[0], 0, sizeof(tp.heat_buf[0]));

    for(i=0, pt=((LineDot/8/TP_MAX_HEAT_STROBE)*strobe_cnt), heat_cnt=0, max_dot=0; i<LineDot/8/TP_MAX_HEAT_STROBE; i++, pt++)
    {
        c = TP_dot[tp.tail][pt];
        dot = Byte2DotNumTbl[c];
        if((max_dot+dot)<=max_heat_dots)
        {
            max_dot += dot;
            tp.heat_buf[heat_cnt][pt] = c;
        }
        else
        {
              for (j=0; j<8; j++)
              {
                 c = TP_dot[tp.tail][pt] & (1<<(7-(j&0x07)));
                 if(c)
                 {
                    if((max_dot+1) <= max_heat_dots)
                    {
                        max_dot++;
                    }
                    else
                    {
                        heat_cnt++;
                        max_dot = 1;
                        memset(tp.heat_buf[heat_cnt], 0, sizeof(tp.heat_buf[0]));
                    }
                    tp.heat_buf[heat_cnt][pt+j/8] |= c;
                 }
              }
        }

    }
    if(max_dot)
    {
       heat_cnt++;
    }

    tp.heat_max_cnt = heat_cnt;//每行最多加热的次数
    tp.heat_cnt = 0;
    // 计算具体的加速表来满足时间要求
    TPAdjustStepTime(heat_cnt,max_heat_dots);

}
#elif defined(BIT_CAL_ENABLE)
static uint8_t StartCal(uint8_t i)
{
//#if ((LineDot%TP_MAX_HEAT_DOT) == 0)
    if(i < ((LineDot/TP_MAX_HEAT_DOT+1)/2))
        return (2*i);
    else
        return (2*(i-((LineDot/TP_MAX_HEAT_DOT+1)/2))+1);
//#else
//    #error "StartCal Define ERROR"
//    return i;
//#endif
}
static void TPDataShiftCntProc(uint8_t strobe_cnt)
{
    static uint8_t const Byte2DotNumTbl[] =
    {
        0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4,
        1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
        1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
        2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
        1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
        2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
        2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
        3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
        1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
        2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
        2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
        3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
        2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
        3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
        3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
        4,5,5,6,5,6,6,7,5,6,6,7,6,7,7,8
    };
    uint8_t c;          // 当前字节
    uint8_t dot;        // 当前字节的点数
    uint8_t heat_cnt;   // 分开几次加热
    uint16_t max_dot;   // 加热点累加和
    uint16_t i,j, pt;       // 行缓冲区指针
    uint16_t max_heat_dots=0;

    for(i=0, pt=((LineDot/8/TP_MAX_HEAT_STROBE)*strobe_cnt); i<LineDot/8/TP_MAX_HEAT_STROBE; i++, pt++)
    {
       c = TP_dot[tp.tail][pt];
       dot = Byte2DotNumTbl[c];
       max_heat_dots += dot;
    }
    max_heat_dots = MaxHeatDotsAdj(max_heat_dots);

    memset(tp.heat_buf[0], 0, sizeof(tp.heat_buf[0]));
    for(i=0,heat_cnt=0,max_dot=0;i<LineDot/TP_MAX_HEAT_DOT/TP_MAX_HEAT_STROBE;i++)
    {
        for(j=0,pt=StartCal(i)+(LineDot/TP_MAX_HEAT_STROBE)*strobe_cnt;  \
            j<TP_MAX_HEAT_DOT;                                 \
            j++,pt+=(LineDot/TP_MAX_HEAT_DOT/TP_MAX_HEAT_STROBE))
        {
            c = TP_dot[tp.tail][pt/8] & (1<<(7-(pt&0x07)));
            if(c)
            {
                if((max_dot+1) <= max_heat_dots)
                {
                    max_dot++;
                }
                else
                {
                    heat_cnt++;
                    max_dot = 1;
                    memset(tp.heat_buf[heat_cnt], 0, sizeof(tp.heat_buf[0]));
                }
                tp.heat_buf[heat_cnt][pt/8] |= c;
            }
        }
    }
    if(max_dot)
    {
       heat_cnt++;
    }

    tp.heat_max_cnt = heat_cnt;//每行最多加热的次数
    tp.heat_cnt = 0;
    // 计算具体的加速表来满足时间要求
    TPAdjustStepTime(heat_cnt,max_heat_dots);

}
#else

static void TPDataShiftCntProc(uint8_t strobe_cnt)
{
    static uint8_t const Byte2DotNumTbl[] =
    {
        0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4,
        1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
        1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
        2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
        1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
        2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
        2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
        3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
        1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
        2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
        2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
        3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
        2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
        3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
        3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
        4,5,5,6,5,6,6,7,5,6,6,7,6,7,7,8
    };
    uint8_t c;          // 当前字节
    uint8_t dot;        // 当前字节的点数
    uint8_t heat_cnt;   // 分开几次加热
    uint16_t max_dot;   // 加热点累加和
    uint16_t i,j,pt;        // 行缓冲区指针
    uint16_t max_heat_dots=0;

    //drv_com_write(DEBUG_PORT_NO,"TPDataShiftCntProc\r\n",sizeof("TPDataShiftCntProc\r\n"));

    for(i=0, pt=((LineDot/8/TP_MAX_HEAT_STROBE)*strobe_cnt); i<LineDot/8/TP_MAX_HEAT_STROBE; i++, pt++)
    {
        c = TP_dot[tp.tail][pt];
        dot = Byte2DotNumTbl[c];
        max_heat_dots += dot;
    }
    max_heat_dots = MaxHeatDotsAdj(max_heat_dots);

    memset(tp.heat_buf[0], 0, sizeof(tp.heat_buf[0]));

    for(i=0, pt=((LineDot/8/TP_MAX_HEAT_STROBE)*strobe_cnt), heat_cnt=0, max_dot=0; i<LineDot/8/TP_MAX_HEAT_STROBE; i++, pt++)
    {
        c = TP_dot[tp.tail][pt];
        dot = Byte2DotNumTbl[c];
        if((max_dot+dot)<=max_heat_dots)
        {
            max_dot += dot;
            tp.heat_buf[heat_cnt][pt] = c;
        }
        else
        {
            for (j=0; j<8; j++)
            {
                c = TP_dot[tp.tail][pt] & (1<<(7-(j&0x07)));
                if(c)
                {
                    if((max_dot+1) <= max_heat_dots)
                    {
                        max_dot++;
                    }
                    else
                    {
                        heat_cnt++;
                        max_dot = 1;
                        memset((void *)tp.heat_buf[heat_cnt], 0, sizeof(tp.heat_buf[0]));
                    }
                    tp.heat_buf[heat_cnt][pt+j/8] |= c;
                }
            }
        }

    }

    if(max_dot)
    {
        heat_cnt++;
    }

    tp.heat_max_cnt = heat_cnt;//每行最多加热的次数
    tp.heat_cnt = 0;

    // 计算具体的加速表来满足时间要求
    TPAdjustStepTime(heat_cnt,max_heat_dots);

}

#endif

#if defined(HISTORY_HEAT_ENABLE)
static void TPDataDMAShiftToPrnHistory(void)
{
    uint16_t tail = (tp.tail-1)%TP_BUFLINE_MAX;// & (ARRAY_SIZE(TP_dot)-1);
    uint8_t i,data;
    //lpc111x无法按32位16位操作
    for(i=0;i<LineDot/8;i++)
    {
        data = ~TP_dot[tail][i];
        tp.heat_buf[tp.heat_cnt][i] &= data;
    }

    TPDataShiftOut(tp.heat_buf[tp.heat_cnt],ARRAY_SIZE(tp.heat_buf[0]));
}
#endif

static void TPDataDMAShiftToPrn(void)
{
    TPDataShiftOut(tp.heat_buf[tp.heat_cnt],ARRAY_SIZE(tp.heat_buf[0]));
}

static uint8_t TPCheckBuf(void)
{
    uint8_t ret;
    uint8_t flg = 0;

    //uint16_t feedmax;

    //drv_com_write(DEBUG_PORT_NO,"TPCheckBuf \r\n",strlen("TPCheckBuf \r\n"));

    if(TPPrinterReady() != TRUE)
    {
        TPIntSetIdle();

        tp.head = tp.tail = 0;
        memset(tp.heat_buf, 0, sizeof(tp.heat_buf));
        ret = Select_IDLE;

        return ret;
    }


    if (tp.head != tp.tail)
    {
        switch (TP_dot[tp.tail][LineDot/8])
        {
        case TP_CMD_PRINT_DOTLINE_RASTER:
            flg = 1;
        case TP_CMD_PRINT_DOTLINE:
            //printf("%d\n", flg);
            
            if(flg)
            {
                flg = 0;
                choose_dot = 0;
                TP_MAX_HEAT_DOT = MAX_HEAT_ADJ;//64;
            }
            else
            {
                if(heat_dot_setting == 0)
                {
                    if(TPHTemperature() > (int16_t)10)
                    {
                        
                        choose_dot = 1;
                        TP_MAX_HEAT_DOT = TP_HEAT_DOT_BAK;//MAX_HEAT_ADJ;
                    }
                    else
                    {
                        choose_dot = 0;
                        TP_MAX_HEAT_DOT = MAX_HEAT_ADJ;//64;
                    }
                }
            }

            TPDataShiftCntProc(0);      // 计算第一个加热行
            
            MOTOR_PWR_ON();
            if(tp.heat_cnt < tp.heat_max_cnt)//本行还有数据需要加热
            {
                TPDataDMAShiftToPrn();      // 开始送数据到打印机
            }

            tp.heat_remain = 0;
            tp.feed_step = 0;
            tp.strobe_step = 0;
#if defined(HISTORY_HEAT_ENABLE)
            tp.history_cnt = 0;
#endif

            tp.state = TPSTATE_HEAT_WITH_FEED;
            ret = 1;
            break;
        case TP_CMD_FEED:
            tp.feedmax = TP_dot[tp.tail][0] | (TP_dot[tp.tail][1] << 8);
            tp.tail = (tp.tail+1)%TP_BUFLINE_MAX;// & (ARRAY_SIZE(TP_dot)-1);
#if defined(HISTORY_HEAT_ENABLE)
            tp.last_max_cnt = 0;
            tp.history_cnt = 0;
            tp.first = 0;
#endif
            tp.state = TPSTATE_FEED;
            ret = 2;
            break;
        default:    // 未知类型，属于严重错误
            tp.tail = (tp.tail+1)%TP_BUFLINE_MAX;// & (ARRAY_SIZE(TP_dot)-1);
#if defined(HISTORY_HEAT_ENABLE)
            tp.last_max_cnt = 0;
            tp.history_cnt = 0;
            tp.first = 0;
#endif

            TPIntSetIdle();
            ret = 0;
            break;
        }
    }
    else
    {
        //TPIntSetPreIdle();
        ret = 0;
    }
    return ret;
}


extern void TPISRProc(void)
{
    char cTemp[64];
    //sprintf(cTemp,"tp.state = %d \r\n",tp.state);
    //drv_com_write(DEBUG_PORT_NO,cTemp,strlen(cTemp));
//dev_debug_printf("tp.state=%d\r\n", tp.state);

    switch (tp.state)
    {
    case TPSTATE_START:     // start
        switch (TPCheckBuf())
        {
        case 1:     // 打印

//            MOTOR_PWR_ON();
            TPSetTimeCnt(TPGetStepTime());  // set timer

            break;
        case 2:     // 走纸

            MOTOR_PWR_ON();
            TPSetTimeCnt(TPGetStepTime());  // set timer
            break;
        default:

            TPIntSetIdle();
            break;
        }
        break;
    case TPSTATE_HEAT_WITH_FEED:       // 开始马达步进

        TPForwardStep(1);
        // break;
    case TPSTATE_HEAT_WITHOUT_FEED:

        if(tp.heat_remain)          // 还要继续加热，每行刚开始加热或者一个步进内加热时间足够时此条件不成立
        {
            //sprintf(cTemp,"tp.heat_remain = %d \r\n",tp.heat_remain);
            //drv_com_write(DEBUG_PORT_NO,cTemp,strlen(cTemp));
            
            if(tp.feed_time[tp.feed_step] >= tp.heat_remain) // 当前步进的时间足够加热
            {
                TPSetTimeCnt(tp.heat_remain);   // 加热
                tp.feed_time[tp.feed_step] -= tp.heat_remain;
                tp.heat_remain = 0;
                tp.state = TPSTATE_HEAT_WITHOUT_FEED;
            }
            else            // 时间不够或者刚好，先加热剩余时间//如果时间不够则走一步后继续跳入上一个if,刚好则跳入else
            {
                TPSetTimeCnt(tp.feed_time[tp.feed_step]); // 加热剩余时间
                tp.heat_remain -= tp.feed_time[tp.feed_step];
                tp.state = TPSTATE_HEAT_WITH_FEED;
                tp.feed_step++;
            }
        }
        else 
           {
            if(tp.heat_cnt < tp.heat_max_cnt)   // 已经有数据被送给打印机
            {
#if 1   //pengxuebin,20180918             
#if defined(HISTORY_HEAT_ENABLE)
                
                if(tp.history_cnt< HISTORY_HEAT_CNT)
                {
                    //sprintf(cTemp,"tp.heat = %d \r\n",tp.heat);
                    //drv_com_write(DEBUG_PORT_NO,cTemp,strlen(cTemp));
                
                    if(tp.heat==0)
                        tp.heat_cnt++;
                    else
                        tp.history_cnt++;

                    STROBE_0_OFF();
                    STROBE_1_OFF();
                    LATCH_LOW();

                    LATCH_HIGH();

                    if(tp.heat==0)
                    {
                        if(tp.heat_cnt < tp.heat_max_cnt)
                        {
                            TPDataDMAShiftToPrn();
                        }
                    }
                    else
                    {
                        if(tp.first == 0)
                        {
                            TPDataDMAShiftToPrn();
                        }
                        else
                        {
                            TPDataDMAShiftToPrnHistory();
                        }
                    }
                    if (tp.feed_time[tp.feed_step] >= tp.heat_percent)
                    {
                        TPSetTimeCnt(tp.heat_percent);  // 加热
                        

                        sprintf(cTemp,"tp.feed_time = %d \r\n",tp.feed_time[tp.feed_step] );

                        tp.feed_time[tp.feed_step] -= tp.heat_percent;
                        
                        //drv_com_write(DEBUG_PORT_NO,cTemp,strlen(cTemp));
                                                
                        tp.state = TPSTATE_HEAT_WITHOUT_FEED;
                    }
                    else
                    {
                        //sprintf(cTemp,"tp.feed_time = %d \r\n",tp.feed_time[tp.feed_step] );
                        //drv_com_write(DEBUG_PORT_NO,cTemp,strlen(cTemp));

                        //sprintf(cTemp,"tp.heat_percent = %d \r\n",tp.heat_percent);
                        //drv_com_write(DEBUG_PORT_NO,cTemp,strlen(cTemp));
                                        
                        TPSetTimeCnt(tp.feed_time[tp.feed_step]);
                        tp.heat_remain = tp.heat_percent - tp.feed_time[tp.feed_step];
                        tp.state = TPSTATE_HEAT_WITH_FEED;
                        tp.feed_step++;
                    }
                    STROBE_1_ON();
                    STROBE_0_ON();


                }
                else
#endif
#endif
                {
#if defined(HISTORY_HEAT_ENABLE)
                    tp.history_cnt=0;
#endif

                    tp.heat_cnt++;

                    STROBE_0_OFF();
                    STROBE_1_OFF();

                    LATCH_LOW();

                    LATCH_HIGH();

                    //sprintf(cTemp,"tp.heat_cnt = %d \r\n",tp.heat_cnt);
                    //drv_com_write(DEBUG_PORT_NO,cTemp,strlen(cTemp));

                    if(tp.heat_cnt < tp.heat_max_cnt)   // 还有数据需要送到打印机
                    {
                        TPDataDMAShiftToPrn();      // shift next heat data to printer
                    }
                    if(tp.feed_time[tp.feed_step] >= tp.heat)    // 当前步进的时间足够加热
                    {
                        TPSetTimeCnt(tp.heat);  // 加热
                        tp.feed_time[tp.feed_step] -= tp.heat;
                        tp.state = TPSTATE_HEAT_WITHOUT_FEED;
                    }
                    else            // 时间不够或者刚好，先加热剩余时间
                    {
                        TPSetTimeCnt(tp.feed_time[tp.feed_step]);   // 加热剩余时间
                        tp.heat_remain = tp.heat - tp.feed_time[tp.feed_step];
                        tp.state = TPSTATE_HEAT_WITH_FEED;
                        tp.feed_step++;
                    }
                    STROBE_0_ON();
                    STROBE_1_ON();
//dev_debug_printf("DST\r\n");                    

                }
            }
            else    // no any data need to print//本行加热次数完成
            {
                //sprintf(cTemp,"finish \r\n");
                //drv_com_write(DEBUG_PORT_NO,cTemp,strlen(cTemp));
                
                STROBE_0_OFF();     // stop heat
                STROBE_1_OFF();     // stop heat
                TPSetTimeCnt(tp.feed_time[tp.feed_step]);   // 停止加热时间//最后一次加热会出现这种情况，加热次数完成还有剩余步进时间
#if TP_MAX_STROBE_STEP>1
                if(tp.feed_step < (TP_MAX_STROBE_STEP-1))       // 每个加热Strobe中包含的步进数
                {
                    tp.feed_step++;
                    tp.state = TPSTATE_HEAT_WITH_FEED;
                }
                else    // 完成一个加热Strobe的控制
#endif
                {
                    tp.feed_step = 0;
#if TP_MAX_HEAT_STROBE>1
                    if(tp.strobe_step < (TP_MAX_HEAT_STROBE-1))
                    {
                        tp.strobe_step++;
                        TPDataShiftCntProc(tp.strobe_step);     // 计算下一个加热行
                        if(tp.heat_cnt < tp.heat_max_cnt)
                        {
                            TPDataDMAShiftToPrn();      // 开始送数据到打印机
                        }
                        tp.state = TPSTATE_HEAT_WITH_FEED;
                    }
                    else    // 当前点行打印完成
#endif
                    {
#if defined(HISTORY_HEAT_ENABLE)
                        tp.first = 1;
#endif
#if defined(DOUBLE_HEAT_ENABLE)
                        {
                            if(++tp.heat_line_cnt == 2)
                            {
                                tp.heat_line_cnt = 0;
                                tp.tail = (tp.tail+1)%TP_BUFLINE_MAX;// & (ARRAY_SIZE(TP_dot)-1);
                            }
                        }
#else
                        tp.tail = (tp.tail+1)%TP_BUFLINE_MAX;// & (ARRAY_SIZE(TP_dot)-1);
#endif
                        switch(TPCheckBuf())
                        {
                        case 0:     // no data
                            TPIntSetPreIdle();
                            break;
                        case 1:
                        case 2:
                        case 3:
                            break;
                        default:    // bug
                            TPIntSetIdle();
                            break;
                        }
                    }
                }
            }
        }
        break;
    case TPSTATE_FEED:
        if (TPFeedStep() == 0)
        {
            switch(TPCheckBuf())
            {
            case 0:     // no data
                TPIntSetPreIdle();
                break;
            case 1:
            case 2:
            case 3:
                break;
            default:    // bug
                TPIntSetIdle();
                break;
            }
        }

        break;
    case TPSTATE_WAIT_TIME:
        if(--tp.repeat == 0 )
        {
            switch(TPCheckBuf())
            {
                //            case 0:   // no data
                //              TPIntSetPreIdle();
                //              break;
            case 1:
            case 2:
            case 3:
                break;
            default:    // bug
                TPIntSetIdle();
                break;
            }
        }
        break;
    case TPSTATE_PRE_IDLE:

        TPSetTimeCnt(TIMER1_MS_TO_CNT(1.0));

        if(tp.feedmax)
        {
            tp.feedmax--;
            if(tp.feedmax & 0x01)
            {
                PRN_POWER_CHARGE();
            }
            else
            {
//                PRN_POWER_DISCHARGE();
            }
        }
        else
        {
            tp.accel = 0;               // 下次需要退纸，重新开始缓启动
            switch(TPCheckBuf())
            {
            case 0:     // no data
            case Select_IDLE:
            default:    // bug
                TPIntSetIdle();
                break;
            case 1:
            case 2:
            case 3:
                break;
            }
        }
        break;
    default:
        tp.state = TPSTATE_IDLE;
        //break;
    case TPSTATE_IDLE:
        TPIntSetIdle();
        break;
    }

}


extern void TPFeedStop(void)
{
    tp.feedmax = 0;     // interrupt will stop feed automaticcly
}

extern void TPFeedStart(void)
{
    if (tp.state == TPSTATE_IDLE)
    {
        //TPFeedLine(500*8);      // 500mm
        TPFeedLine(50*8);      // 5mm
    }
}

extern iomux_pin_name_t  g_prt_line_test_pin;
extern void TPPrintLine(uint8_t *dot)
{
    //DBG_STR("####  TPPrintLine()\r\n");
#ifdef TEST_GPIO
    dev_gpio_direction_output(g_prt_line_test_pin, 1);
#endif

#if defined(TPH_JING_CHI)
#if defined(TPH_JING_TEST)
    TPPrintCmdToBuf(TP_CMD_PRINT_DOTLINE, dot, LineDot/8);
#endif
#endif

#if defined(DOUBLE_HEAT_ENABLE)
    //TPPrintCmdToBuf(TP_CMD_PRINT_DOTLINE, dot, LineDot/8);
#endif
    if((!TPPrinterReady()) == TRUE)
    {
        
        PRN_POWER_CHARGE();
        dev_user_delay_ms(5);//delay_ms(5);
        TPPaperSNSDetect(0);
        if((!TPPrinterReady()) == TRUE)
        {
            tp.head = tp.tail = 0;
            tp.state = TPSTATE_IDLE;
            memset(tp.heat_buf, 0, sizeof(tp.heat_buf));
            
            PRN_POWER_DISCHARGE();
  //          dev_debug_printf("----- %s ---- %d ---- onpaper ---\r\n",__func__,__LINE__);
#ifdef TEST_GPIO
            dev_gpio_direction_output(g_prt_line_test_pin, 0);
#endif
            return;
        }
    }

    TPPrintCmdToBuf(TP_CMD_PRINT_DOTLINE, dot, LineDot/8);    

#ifdef TEST_GPIO
    dev_gpio_direction_output(g_prt_line_test_pin, 0);
#endif
}
extern void TPPrintRasterLine(uint8_t *dot)
{
#if defined(DOUBLE_HEAT_ENABLE) || defined(PT487_100DPI)
    //TPPrintCmdToBuf(TP_CMD_PRINT_DOTLINE, dot, LineDot/8);
#endif
    TPPrintCmdToBuf(TP_CMD_PRINT_DOTLINE_RASTER, dot, LineDot/8);//TP_CMD_PRINT_DOTLINE_RASTER
}

/*
**********************************************************************************************************
** 名    称 PrtFeedStart
** 功    能 ：用于打印机走纸
** 入口参数 ：len 为走纸长度，定位mm
** 出口参数 ：
** 备    注 :
**********************************************************************************************************
*/
extern void PrtFeedStart(uint8_t len)
{
    if (tp.state == TPSTATE_IDLE)
    {
        TPFeedLine(len*8);      
    }

    return;
}

/*
**********************************************************************************************************
** 名    称 PrtPrintStart
** 功    能 ：用于打印机的图片打印接口
** 入口参数 ：buf 图片缓存地址，x 为图片的宽度， y 为图片的高度
** 出口参数 ：
** 备    注 :
**********************************************************************************************************
*/
extern void PrtPrintStart(const uint8_t *buf, const uint32_t x, const uint32_t y)
{
    uint32_t i;
    uint8_t line_buf[LineDot/8];

    for(i = 0; i < y; i++)
    {
        memset(line_buf,0,sizeof(line_buf));
        
        memcpy(line_buf, buf + ( i * x), x);

        TPPrintLine(line_buf);
    }
    
    return;
}

/*
**********************************************************************************************************
** 名    称 PrtSetSpeed
** 功    能 ：用于设置打印机的浓度
** 入口参数 ：speed浓度等级，从0~6
** 出口参数 ：
** 备    注 :
**********************************************************************************************************
*/
extern void PrtSetSpeed(uint8_t speed)
{
    char cTemp[64];
    TPSetSpeed(speed);
    
    return;
}

extern int16_t PrtGetTemperature(void)
{
    return TPHTemperature();
}
//获取当前Buf空余行数
u16 dev_print_get_bufline(void)
{
    u16 i,j;

    i = tp.tail;

    if(tp.head >= i)
    {
        j = (i+TP_BUFLINE_MAX-tp.head-1);
    }
    else
    {
        j = (i-tp.head-1);
    }
    return j;
}
#if 1//defined(DEBUG_TEST)
extern void TPSelfTest_1(void)
{
    uint8_t dot[LineDot/8];         //定义一个行，行的字节数目是多少，这边一共是576/8=72

    uint32_t i,j,n;
    memset(dot, 0x00, sizeof(dot));
    pt_resume();
    #if 1

    for (i=0; i<32; i++)
    {
        TPPrintLine(dot);
    }


    for(i=0;i<80;i++)
    {
        for(j=0;j<1;j++)
        {
            memset(dot,0xaa,sizeof(dot));
            TPPrintLine(dot);
        }
        for(j=0;j<1;j++)
        {
            memset(dot,0x55,sizeof(dot));
            TPPrintLine(dot);
        }
    }
    //第二段
    //10x16=160 dotline
    for(i=0;i<10;i++)
    {
        for(j=0;j<8;j++)
        {
            memset(dot,0xaa,sizeof(dot));
            TPPrintLine(dot);
        }
        for(j=0;j<8;j++)
        {
            memset(dot,0x55,sizeof(dot));
            TPPrintLine(dot);
        }
        //TPPrintCmdToBuf(TP_CMD_PRINT_DOTLINE,dot,TotalBit/8);
    }
    //第三段
    //(4+4)x20=160 dotline
    for(i=0;i<20;i++)
    {
        for(j=0;j<4;j++)
        {
            memset(dot,0xcc,sizeof(dot));
            TPPrintLine(dot);
        }
        for(j=0;j<4;j++)
        {
            memset(dot,0x33,sizeof(dot));
            TPPrintLine(dot);
        }
        //TPPrintCmdToBuf(TP_CMD_PRINT_DOTLINE,dot,TotalBit/8);
    }
    //第四段
    //(7+7)x10=140 dotline
    for(i=0;i<10;i++)
    {
        uint8_t buf1[3]={0xe3,0x8e,0x38};
        uint8_t buf2[3]={0x1c,0x71,0xc7};
        for(j=0;j<7;j++)
        {
            for(n=0;n<sizeof(dot)/sizeof(buf1);n++)
            {
                memcpy(dot+3*n,buf1,sizeof(buf1));
            }
            TPPrintLine(dot);
        }
        for(j=0;j<7;j++)
        {
            for(n=0;n<sizeof(dot)/sizeof(buf2);n++)
            {
                memcpy(dot+3*n,buf2,sizeof(buf2));
            }
            TPPrintLine(dot);
        }
        //TPPrintCmdToBuf(TP_CMD_PRINT_DOTLINE,dot,TotalBit/8);
    }

    //第五段
    //(4+4)x3x3=72 dotline
    for(i=0;i<3;i++)
    {

        for(j=0;j<4;j++)
        {
            for(n=0;n<sizeof(dot)/4;n++)
            {
                memset(dot+4*n,0xaa,2);
                memset(dot+4*n+2,0x00,2);
            }

        }
        for(j=0;j<4;j++)
        {
            for(n=0;n<sizeof(dot)/4;n++)
            {
                memset(dot+4*n,0x55,2);
                memset(dot+4*n+2,0x00,2);
            }
        }

        //TPPrintCmdToBuf(TP_CMD_PRINT_DOTLINE,dot,TotalBit/8);
    }
    //
    for(i=0;i<3;i++)
    {

        for(j=0;j<4;j++)
        {
            for(n=0;n<sizeof(dot)/4;n++)
            {
                memset(dot+4*n,0x00,2);
                memset(dot+4*n+2,0xaa,2);
            }

            TPPrintLine(dot);

        }
        for(j=0;j<4;j++)
        {
            for(n=0;n<sizeof(dot)/4;n++)
            {
                memset(dot+4*n,0x00,2);
                memset(dot+4*n+2,0x55,2);
            }
            TPPrintLine(dot);

        }

        //TPPrintCmdToBuf(TP_CMD_PRINT_DOTLINE,dot,TotalBit/8);
    }
    //
    for(i=0;i<3;i++)
    {

        for(j=0;j<4;j++)
        {
            for(n=0;n<sizeof(dot)/4;n++)
            {
                memset(dot+4*n,0xaa,2);
                memset(dot+4*n+2,0x00,2);
            }

            TPPrintLine(dot);

        }
        for(j=0;j<4;j++)
        {
            for(n=0;n<sizeof(dot)/4;n++)
            {
                memset(dot+4*n,0x55,2);
                memset(dot+4*n+2,0x00,2);
            }
            TPPrintLine(dot);

        }

        //TPPrintCmdToBuf(TP_CMD_PRINT_DOTLINE,dot,TotalBit/8);
    }
    #endif

    //第六段
    //8 dotline

    //6x4x5=120
    for(i=0;i<4;i++)
    {

        for(j=0;j<4;j++)
        {
            for(n=0;n<sizeof(dot)/4;n++)
            {
                memset(dot+4*n,0x00,2);
                memset(dot+4*n+2,0xff,2);
            }

            TPPrintLine(dot);

        }


        //TPPrintCmdToBuf(TP_CMD_PRINT_DOTLINE,dot,TotalBit/8);
    }
    //
    for(i=0;i<4;i++)
    {

        for(j=0;j<4;j++)
        {
            for(n=0;n<sizeof(dot)/4;n++)
            {
                memset(dot+4*n,0xff,2);
                memset(dot+4*n+2,0x00,2);
            }
            TPPrintLine(dot);

        }
        //TPPrintCmdToBuf(TP_CMD_PRINT_DOTLINE,dot,TotalBit/8);
    }
    //
    for(i=0;i<4;i++)
    {

        for(j=0;j<4;j++)
        {
            for(n=0;n<sizeof(dot)/4;n++)
            {
                memset(dot+4*n,0x00,2);
                memset(dot+4*n+2,0xff,2);
            }
            TPPrintLine(dot);

        }

        //TPPrintCmdToBuf(TP_CMD_PRINT_DOTLINE,dot,TotalBit/8);
    }
    //
    for(i=0;i<4;i++)
    {

        for(j=0;j<4;j++)
        {
            for(n=0;n<sizeof(dot)/4;n++)
            {
                memset(dot+4*n,0xff,2);
                memset(dot+4*n+2,0x00,2);
            }
            TPPrintLine(dot);

        }


        //TPPrintCmdToBuf(TP_CMD_PRINT_DOTLINE,dot,TotalBit/8);
    }

    //
    for(i=0;i<4;i++)
    {

        for(j=0;j<4;j++)
        {
            for(n=0;n<sizeof(dot)/4;n++)
            {
                memset(dot+4*n,0x00,2);
                memset(dot+4*n+2,0xff,2);
            }
            TPPrintLine(dot);

        }

        //TPPrintCmdToBuf(TP_CMD_PRINT_DOTLINE,dot,TotalBit/8);
    }

    //第七段，打印全黑

    /*
       for(j=0;j<75;j++)
       {
       memset(dot,0xff,sizeof(dot));
       TPPrintLine(dot);

       }

       memset(dot,0xff,sizeof(dot)/4);
       memset(dot+sizeof(dot)/4,0x00,sizeof(dot)/2);
       memset(dot+sizeof(dot)/4+sizeof(dot)/2,0xff,sizeof(dot)/4);
       for(j=0;j<75;j++)
       {
       TPPrintLine(dot);
       }

       memset(dot,0x00,sizeof(dot)/4);
       memset(dot+sizeof(dot)/4,0xff,sizeof(dot)/2);
       memset(dot+sizeof(dot)/4+sizeof(dot)/2,0x00,sizeof(dot)/4);

       for(j=0;j<75;j++)
       {
       TPPrintLine(dot);
       }*/


    memset(dot, 0x00, sizeof(dot));
    for (i=0; i<64; i++)
    {
        TPPrintLine(dot);
    }
    dev_user_delay_ms(1000);
    pt_sleep();

}
#endif
extern void TPSelfTest_2(void)
{
    uint32_t i;
    uint8_t buf[LineDot/8];

    dev_debug_printf("#### TPSelfTest_2()\r\n");

    pt_resume();
    dev_debug_printf("2#### TPSelfTest_2()\r\n");

    //for(i=0;i<250/*400*/;i++)
    for(i=0;i<16;i++)
    {
        ddi_watchdog_feed();
        memset(buf,0x01<<0,sizeof(buf));
        TPPrintLine(buf);
        memset(buf,0x01<<1,sizeof(buf));
        TPPrintLine(buf);
        memset(buf,0x01<<2,sizeof(buf));
        TPPrintLine(buf);
        memset(buf,0x01<<3,sizeof(buf));
        TPPrintLine(buf);
        memset(buf,0x01<<4,sizeof(buf));
        TPPrintLine(buf);
        memset(buf,0x01<<5,sizeof(buf));
        TPPrintLine(buf);
        memset(buf,0x01<<6,sizeof(buf));
        TPPrintLine(buf);
        memset(buf,0x01<<7,sizeof(buf));
        TPPrintLine(buf);

    }

    dev_user_delay_ms(1000);
    pt_sleep();
}


extern void TPSelfTest_5(void)
{
    uint32_t i;
    uint32_t n;
    uint8_t buf[LineDot/8];
    uint8_t dot[LineDot/8];

    //DBG_STR("#### TPSelfTest_2()\r\n");
    
    //memset(buf,0x01<<0,sizeof(buf));
    //TPPrintLine(buf);
    
    //memset(buf,0xff,sizeof(buf));
    //TPPrintLine(buf);

    pt_resume();

#if 1
    memset(buf,0xff,sizeof(buf));
    
    for(i=0;i<3000;i++)
    {
        ddi_watchdog_feed();
        TPPrintLine(buf);
    }
#endif
    dev_user_delay_ms(1000);
    pt_sleep();

    
}


extern void TPSelfTest_4(void)
{
    uint32_t i;
    uint32_t n;
    uint8_t buf[LineDot/8];
    uint8_t dot[LineDot/8];

    //DBG_STR("#### TPSelfTest_2()\r\n");
    
    //memset(buf,0x01<<0,sizeof(buf));
    //TPPrintLine(buf);
    
    //memset(buf,0xff,sizeof(buf));
    //TPPrintLine(buf);

    pt_resume();

    #if 1
    memset(buf,0xff,sizeof(buf));
    
    for(i=0;i<16;i++)
    {
        TPPrintLine(buf);
    }

    for(n=0;n<sizeof(buf)/4;n++)
    {
        memset(&buf[4*n],0xff,2);
        memset(&buf[4*n+2],0x00,2);
    }

    for(n=0;n<sizeof(dot)/4;n++)
    {
        memset(dot+4*n,0x00,2);
        memset(dot+4*n+2,0xff,2);
    }
    
    for(i=0;i<16;i++)
    {
        for(n=0;n<16;n++)
        {
            TPPrintLine(buf);
        }
        
        for(n=0;n<16;n++)
        {
            TPPrintLine(dot);
        }
    }
    
    for(i=0;i<16;i++)
    {
        memset(buf,0x01<<0,sizeof(buf));
        TPPrintLine(buf);
        memset(buf,0x01<<1,sizeof(buf));
        TPPrintLine(buf);
        memset(buf,0x01<<2,sizeof(buf));
        TPPrintLine(buf);
        memset(buf,0x01<<3,sizeof(buf));
        TPPrintLine(buf);
        memset(buf,0x01<<4,sizeof(buf));
        TPPrintLine(buf);
        memset(buf,0x01<<5,sizeof(buf));
        TPPrintLine(buf);
        memset(buf,0x01<<6,sizeof(buf));
        TPPrintLine(buf);
        memset(buf,0x01<<7,sizeof(buf));
        TPPrintLine(buf);
    }
    dev_user_delay_ms(1000);
    pt_sleep();
        #endif
}

extern void TPSelfTest_3(void)
{
    uint32_t i,j,n;
    uint8_t buf[LineDot/8];
    uint8_t dot[LineDot/8];

    //DBG_STR("#### TPSelfTest_3()\r\n");

    pt_resume();

#if 0
    for(i=0;i<60*8/*400*/;i++)
    {
        memset(buf,0xff,sizeof(buf));

        TPPrintLine(buf);
    }
#endif
#if 1
    //第六段
    //8 dotline

    //6x4x5=120
    //memset(buf,0xff,sizeof(buf));
    memset(dot, 0x00, sizeof(dot));
    for(i=0;i<4;i++)
    {

        for(j=0;j<4;j++)
        {
            for(n=0;n<sizeof(dot)/4;n++)
            {
                memset(dot+4*n,0x00,2);
                memset(dot+4*n+2,0xff,2);
            }

            TPPrintLine(dot);

        }


        //TPPrintCmdToBuf(TP_CMD_PRINT_DOTLINE,dot,TotalBit/8);
    }
    //
    for(i=0;i<4;i++)
    {

        for(j=0;j<4;j++)
        {
            for(n=0;n<sizeof(dot)/4;n++)
            {
                memset(dot+4*n,0xff,2);
                memset(dot+4*n+2,0x00,2);
            }
            TPPrintLine(dot);

        }
        //TPPrintCmdToBuf(TP_CMD_PRINT_DOTLINE,dot,TotalBit/8);
    }
    //
    for(i=0;i<4;i++)
    {

        for(j=0;j<4;j++)
        {
            for(n=0;n<sizeof(dot)/4;n++)
            {
                memset(dot+4*n,0x00,2);
                memset(dot+4*n+2,0xff,2);
            }
            TPPrintLine(dot);

        }

        //TPPrintCmdToBuf(TP_CMD_PRINT_DOTLINE,dot,TotalBit/8);
    }
    //
    for(i=0;i<4;i++)
    {

        for(j=0;j<4;j++)
        {
            for(n=0;n<sizeof(dot)/4;n++)
            {
                memset(dot+4*n,0xff,2);
                memset(dot+4*n+2,0x00,2);
            }
            TPPrintLine(dot);

        }


        //TPPrintCmdToBuf(TP_CMD_PRINT_DOTLINE,dot,TotalBit/8);
    }

    //
    for(i=0;i<4;i++)
    {

        for(j=0;j<4;j++)
        {
            for(n=0;n<sizeof(dot)/4;n++)
            {
                memset(dot+4*n,0x00,2);
                memset(dot+4*n+2,0xff,2);
            }
            TPPrintLine(dot);

        }

        //TPPrintCmdToBuf(TP_CMD_PRINT_DOTLINE,dot,TotalBit/8);
    }
#endif

    dev_user_delay_ms(1000);
    pt_sleep();

}
#if 0
extern void TPPrintTestBarcode(void)
{
    uint32_t i,maxsize;
    //uint8_t buf[LineDot/8];
    uint8_t buf[LineDot/8];

    uint8_t len =  (LineDot/8) / 2;


    maxsize = sizeof(BcodeData) / len;

    //DBG_STR("TPPrintTestBarcode into! maxsize:%ld\r\n",maxsize);

    for(i = 0; i < maxsize ; i++)
    {
        //DBG_PRINTF("i:%ld ###tp.state:%ld\r\n",i,tp.state );
        //DBG_PRINTF("tp.tail:%ld ###tp.head:%ld\r\n",i,tp.tail,tp.head );
        memset(buf,0,sizeof(buf));
        memcpy(buf,BcodeData +( i *len),len);

        TPPrintLine(buf);
    }
}


extern void TPPrintWordTest(void)
{
    uint32_t i;//maxsize;
    uint8_t buf[LineDot/8];

    uint8_t len =  0x2E;

   //maxsize = sizeof(Wordtest) / len;

    //DBG_STR("TPPrintTestBarcode into! maxsize:%ld\r\n",maxsize);

    for(i = 0; i < 0x88*2; i++)
    {
        //DBG_PRINTF("i:%ld ###tp.state:%ld\r\n",i,tp.state );
        //DBG_PRINTF("tp.tail:%ld ###tp.head:%ld\r\n",i,tp.tail,tp.head );
        memset(buf,0,sizeof(buf));
        memcpy(buf, Wordtest + ( i * len), len);
    //  uart_send_str("func\r\n");
        TPPrintLine(buf);
        
    // TPPrintLine(buf);
    }
}

extern void TPPrintTest(void)
{
    uint32_t i;
    uint8_t buf[LineDot/8];

    uint8_t len =  0x2E;

 //   maxsize = sizeof(Wordtest) / len;
 
    int k = 0, j = 0;
    uint8_t cmd = 0;
    uint8_t test_buf[64] = {0};

    for(j = 0; j < 64; j++)
    {
        if(uart_getchar(S302_PC_LPUART_ID,&cmd ) == 0)
        {
            test_buf[j] = cmd;
//          uart_putchar(cmd);
        }
    }
    
    for(k = 0; k < 64; k++)
    {
        uart_putchar(S302_PC_LPUART_ID,test_buf[k]);
    }

    //DBG_STR("TPPrintTestBarcode into! maxsize:%ld\r\n",maxsize);

    for(i = 0; i < 0x88*2; i++)
    {
        //DBG_PRINTF("i:%ld ###tp.state:%ld\r\n",i,tp.state );
        //DBG_PRINTF("tp.tail:%ld ###tp.head:%ld\r\n",i,tp.tail,tp.head );
        memset(buf,0,sizeof(buf));
        memcpy(buf, test_buf + ( i * len), len);


    
        TPPrintLine(buf);
        
    // TPPrintLine(buf);
    }
}
#endif

////////////////////////////////////////////////////////////////////

void pt48d_dev_init(void)
{
    pt_gpio_init();
    pt_spi_init();
    pt_timer_init();
}

/**********************************************
return :   0  :  IDLE
           1  :  BUSY
           2  :  缺纸
           3  :  打印头故障
**********************************************/
s32 dev_printer_get_status(void)
{
    s32 i;
    int16_t temp;
    
    if (TPGetPaperDetect() != 0) //无纸
    {
#if 0
        PRN_POWER_CHARGE();
        for(i=0; i<5; i++)
        {
            dev_user_delay_ms(1);//delay_ms(1);
            if(TPGetPaperDetect() == 0)
            {
                //有纸
                break;
            }
        }
        if(i>=5)
        {
            //缺纸
        //TPPaperSNSDetect(0);
            PRN_POWER_DISCHARGE();
            return PT_STATUS_NOPAPER;  //无纸返回
        }
#endif
        return PT_STATUS_NOPAPER;  //无纸返回
    }

    if (tp.state != TPSTATE_IDLE)
    {
        return PT_STATUS_BUSYING;
    }

    if(get_temperature() > TP_TEMPERATURE_MAX)
    {
        for(i = 0; i < 10; i++)  /*连续判断10次温度都超过最大限制温度才认为是温度超过了*/
        {
            temp = PrtGetTemperature();
            if(temp < TP_TEMPERATURE_MAX)
            {
                break;
            }
            dev_user_delay_ms(5);
        }
        //temp = PrtGetTemperature();
        set_temperature(temp);
        if(temp > TP_TEMPERATURE_MAX)
        {
            return PT_STATUS_HIGH_TEMP;
        }
    }
    
    return PT_STATUS_IDLE;
}


/**********************************************
return :   0  :  IDLE
           1  :  BUSY
           2  :  缺纸
           3  :  打印头故障
**********************************************/
s32 dev_esc_printer_get_status(void)
{
    s32 i;
    int16_t temp;
    
    if (TPGetPaperDetect() != 0) //无纸
    {
#if 0
        PRN_POWER_CHARGE();
        for(i=0; i<5; i++)
        {
            dev_user_delay_ms(1);//delay_ms(1);
            if(TPGetPaperDetect() == 0)
            {
                //有纸
                break;
            }
        }
        if(i>=5)
        {
            //缺纸
        //TPPaperSNSDetect(0);
            PRN_POWER_DISCHARGE();
            return PT_STATUS_NOPAPER;  //无纸返回
        }
#endif
        return PT_STATUS_NOPAPER;  //无纸返回
    }

    if (tp.state != TPSTATE_IDLE)
    {
        return PT_STATUS_BUSYING;
    }

    if(PrtGetTemperature() > TP_TEMPERATURE_MAX)
    {
        for(i = 0; i < 10; i++)  /*连续判断10次温度都超过最大限制温度才认为是温度超过了*/
        {
            temp = PrtGetTemperature();
            if(temp < TP_TEMPERATURE_MAX)
            {
                break;
            }
            dev_user_delay_ms(5);
        }
        //temp = PrtGetTemperature();
        set_temperature(temp);
        if(temp > TP_TEMPERATURE_MAX)
        {
            return PT_STATUS_HIGH_TEMP;
        }
    }
    
    return PT_STATUS_IDLE;
}


#if 0
void printer_word_test()
{
        PrtPrintStart(Wordtest, 0x2E, 0x88*2 ); 
        PrtPrintStart(Wordtest, 0x2E, 0x88*2 ); 
        PrtPrintStart(Wordtest, 0x2E, 0x88*2 ); 
        PrtPrintStart(Wordtest, 0x2E, 0x88*2 ); 
        PrtPrintStart(Wordtest, 0x2E, 0x88*2 );     

}


void printer_test(void)
{
    unsigned char cmd = 0;

    uart_send_str(S302_PC_LPUART_ID, "-----------  9.打印机芯测试                      -----------\r\n");
    uart_send_str(S302_PC_LPUART_ID, "-----------  0.返回上层界面  9.打印本界面        -----------\r\n");
    uart_send_str(S302_PC_LPUART_ID, "-----------  1.test1    2.test2     3.test3      -----------\r\n");
    uart_send_str(S302_PC_LPUART_ID, "-----------  f.进纸     b:打印二维码             -----------\r\n");
    uart_send_str(S302_PC_LPUART_ID, "请输入0-9 a-z 进行测试: \r\n");

    while(1)
    {
        if(uart_getchar(S302_PC_LPUART_ID,&cmd ) == 0)
        {
            switch(cmd)
            {
            case  '0':
                show_test_interface();
                return ;
                break;

            case  '1':
                TPSelfTest_1();
                break;

            case  '2':
                TPSelfTest_2();
                break;

            case  '3':
                TPSelfTest_3();
                break;

            case  'b':
                TPPrintTestBarcode();
                break;
                
        case 'c':
            uart_putchar(S302_PC_LPUART_ID,cmd);
        //  TPPrintWordTest();
            PrtPrintStart(Wordtest, 0x2E, 0x88*2 );     
            break;
            
        case 'd':
            TPPrintTest();
            break;

        case 'e':
             pt_timer_start();
            break;
            case  '9':
                uart_send_str(S302_PC_LPUART_ID, "-----------  9.打印机芯测试                      -----------\r\n");
                uart_send_str(S302_PC_LPUART_ID, "-----------  0.返回上层界面  9.打印本界面        -----------\r\n");
                uart_send_str(S302_PC_LPUART_ID, "-----------  1.test1    2.test2     3.test3      -----------\r\n");
                uart_send_str(S302_PC_LPUART_ID, "-----------  f.进纸     4.test1 + feed           -----------\r\n");
                uart_send_str(S302_PC_LPUART_ID, "请输入0-9 a-z 进行测试: \r\n");
        
                break;


            case  'f':
                //TPFeedStart();
                PrtFeedStart(200);
                break;
            case 's':
                PrtSetSpeed(3);
                break;
                

        case 'g':
            //SPI1_test();
            break;

            default:break;
            }
        }
    }
}

#endif
#endif

