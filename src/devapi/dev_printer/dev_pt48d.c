
#include "devglobal.h"
#include "sdk/mhscpu_sdk.h"

#if(PRINTER_EXIST==1)

#include "dev_pt48d.h"
#include "dev_pt48d_hw.h"
#include "dev_pt48d_detect.h"
#include "dev_pt48d_dot.h"
#include "dev_pt48d_config.h"

#define TP_TEMPERATURE_MAX  80  
#define TpMinWaitTime   ((0.010))

uint8_t TP_MAX_HEAT_DOT = MAX_HEAT_ADJ;
uint8_t TP_HEAT_DOT_BAK = MAX_HEAT_ADJ;
uint8_t clr_all_dot = 0;
uint8_t choose_dot = 1;
uint8_t heat_dot_setting = 1;
uint8_t aera_speed = 0;

static TP_T tp;
static uint8_t TP_dot[TP_BUFLINE_MAX][LineDot/8+1];  // 增加一个控制位
static uint16_t battery_voltage = 380;
static uint32_t g_printer_state = 1; //1 正常， 0 异常 
static s32 g_print_overtime_flag = 0;
static int16_t temperature_bak = 25;

static uint32_t const TpAccelerationSteps[] =
{
  #if 1
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
  #endif
};  

uint8_t const Heat_Temperater_Ratio[]=
{
  #if 1
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
  #endif
};
    
uint8_t const Heat_TemperaterLow_Ratio[]=
{
  #if 1
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
  #endif
};

uint8_t const Battery_Voltage_Ratio[]=
{
  #if 1
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
  #endif
};

uint8_t const Density_Ratio[]=
{
    // 比例      浓度
    100,        ///5
    100,        ///7
    100,        ///9
    100,
};

struct __vol_speed_map vol_speed_map[] = 
{
    {660, 1200},
    {710, 1100},
    {760, 1000},
    {800, 850},
    {840, 750}
};

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
};

uint8_t const heat_cnt_to_ratio96[] =
{
    80,
    100,
    100,
    100,
    100,
};
#endif

extern s32 drv_com_write(s32 nCom, u8 *wbuf, u16 wlen);
extern void BatteryVoltageADJ(uint16_t bat_vol);

extern void PrtSetHeatDot(uint8_t val)
{
    TP_MAX_HEAT_DOT = val;
    TP_HEAT_DOT_BAK = val;
}

/**
 * @brief  设置打印机速度
 * @param [in]  speed:速度区间
 */
void TPSetSpeed(uint8_t speed)
{
    uint16_t const TPHeatTbl[] =
    {
       TIMER1_MS_TO_CNT(0.20),
       TIMER1_MS_TO_CNT(0.30),
       TIMER1_MS_TO_CNT(0.40), 
       TIMER1_MS_TO_CNT(0.50), 
       TIMER1_MS_TO_CNT(0.60), 
       TIMER1_MS_TO_CNT(0.70), 
       TIMER1_MS_TO_CNT(0.80), 
       TIMER1_MS_TO_CNT(0.90),
       TIMER1_MS_TO_CNT(1.10),
       TIMER1_MS_TO_CNT(1.30),
       TIMER1_MS_TO_CNT(1.60),
       TIMER1_MS_TO_CNT(1.90),
       TIMER1_MS_TO_CNT(2.10),
    };

    if (speed < ARRAY_SIZE(TPHeatTbl))
    {
        tp.heat_setting = TPHeatTbl[speed];
      #if defined(NEW_HEAT_TIME)
        tp.heat_adj = tp.heat_setting;
      #endif
    }
}

void SetDesity(void)
{
    tp.head = tp.tail = 0;
    tp.state = TPSTATE_IDLE;
}

/**
 * @brief  打印机设备初始化
 */
extern void printer_init(void)
{
    pt48d_dev_init();
    PRN_POWER_CHARGE();
    
    MOTOR_PWR_OFF();
    STROBE_0_OFF();
    STROBE_1_OFF();

    MOTOR_PHASE_1A_LOW();
    MOTOR_PHASE_1B_LOW();
    MOTOR_PHASE_2A_LOW();
    MOTOR_PHASE_2B_LOW();    

    tp.head = tp.tail = 0;
    tp.state = TPSTATE_IDLE;
   
    TPSetSpeed(2);
    pt_sleep();   
}

/**
 * @brief  启动热敏打印机
 */
void TPStart(void)
{
    if (!(TPPrinterReady() && (tp.state == TPSTATE_IDLE))) 
        return;

    tp.state = TPSTATE_START;
    tp.accel = 0;   

    pt_timer_set_periodUs(TpAccelerationSteps[0]);
    PRN_POWER_CHARGE();

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
    
    pt_timer_start();
}

/**
 * @brief  设置打印机状态
 * @param [in]  state:设置状态
 */
void dev_printer_setpaper_state(uint32_t state)
{
    g_printer_state = state;
}

/**
 * @brief  获取打印机状态
 * @retval 打印机状态
 */
s32 dev_printer_getpaper_state(void)
{
    return g_printer_state;
}

/**
 * @brief  判断打印是否超时
 * @note  供loop接口里调用
 * @retval  1-超时 0-正常 
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
    uint32_t head;
    u32 beg_ticks = 0;

  #if defined(HISTORY_HEAT_ENABLE)
    head = (tp.head+2);
  #else
    head = (tp.head+1);
  #endif
  
    head = (head%TP_BUFLINE_MAX);
    if(dev_printer_getpaper_state() == PT_STATUS_NOPAPER)
    {
        return;
    }

    beg_ticks = dev_user_gettimeID();
    while (head == tp.tail)
    {
        //因为打印中断处理程序有可能在异常的情况下进入Idle状态，所以需要不断检查这个状态       
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

    memcpy(TP_dot[tp.head], dot, len);
    TP_dot[tp.head][LineDot/8] = cmd;
    tp.head = head;

    TPStart();
}

void TPFeedLine(uint16_t line)
{    
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
    if(tm < TpMinWaitTime*1000)  
        tm = TpMinWaitTime*1000;

    pt_timer_set_periodUs(tm);
    pt_timer_start();
}

static void TPForwardStep(int direction)
{
    pt_printline_count(); 
 
    switch (tp.phase & 0x07)
    {  
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
    }

    tp.phase += direction;
    pt_check_paper();
}

static uint8_t TPFeedStep(void)
{
    uint16_t time;
    
    if(aera_speed) 
    {
        TPSetTimeCnt(TpAccelerationSteps[tp.accel]);
    } 
    else 
    {
        time  = TPGetStepTime();
        TPSetTimeCnt(time); 
    }

    if(TPPrinterReady() != TRUE )
    {
        aera_speed = 0;
        return 0;
    }

    if(tp.feedmax)
    {
        TPForwardStep(1);
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

    tp.feedmax = 10*1;      // 每1ms中断一次
    tp.state = TPSTATE_PRE_IDLE;
}

static void TPIntSetIdle(void)
{
    STROBE_0_OFF();     // stop heat
    STROBE_1_OFF();     // stop heat
    pt_timer_stop();
    MOTOR_PWR_OFF();

    tp.state = TPSTATE_IDLE;

  #if defined(NEW_HEAT_TIME)
    PrintOn_Flag = FALSE;
  #endif
}

uint16_t MaxHeatDotsAdj(uint16_t dots)
{
    uint16_t max_heat_dot;  //新的最大值

    max_heat_dot = dots/(dots/(TP_MAX_HEAT_DOT+1)+1);  //总加热点数加热次数    
    if ((max_heat_dot+1) <= TP_MAX_HEAT_DOT)         //8个点的误差，引起多一次的加热
    {        
        max_heat_dot += 1;
    }
    else
    {        
        max_heat_dot = TP_MAX_HEAT_DOT;
    }
    
    return max_heat_dot;
}

void TPSetHeatDots(uint8_t dot)
{
    if(dot > 2)
        return;

    if(dot == 0)  //取消指令选择加热点数
    {
        heat_dot_setting = 0;
        choose_dot = 1;  //底下2句没什么用，逻辑需要
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

    static uint8_t  dot_ratio_tbl96[96/4]=
    {
        78, 79, 80, 81, 82, 83,//0-24
        84, 85, 86, 87, 88, 89,
        100,101,102,103,114,115,
        116,117,118,119,120,120,
    };
        
    static uint8_t  dot_ratio_tbl80[80/4]=
    {
        78, 79, 80, 81, 82, 83,//0-24
        84, 85, 86, 87, 88, 89,
        100,101,102,103,114,115,
        116,117,
    };
        
    static uint8_t  dot_ratio_tbl72[72/4]=
    {
        78, 79, 80, 81, 82, 83,//0-24
        84, 85, 86, 87, 88, 89,
        100,101,102,103,114,115,
    };
        
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

void BatteryVoltageADJ(uint16_t bat_vol)
{
    //根据电压和浓度调整
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
}

static uint32_t TPHeatThermalAdj(uint32_t tm,int16_t temp)
{
    char cTemp[64];    
    
    if((temp < 25)&&((temp >= (-20))))
    {
        tm = tm *Heat_TemperaterLow_Ratio[(25 - temp)]/100;
    }
    else if ((temp >= 25)&&(temp <= 80))
    {
        tm = tm *Heat_Temperater_Ratio[(temp-25)]/100;
    }
    else if (temp>80)
    {
        tm = tm * 25 /100;   //超过65度，按50%加热
    }

    return tm;
}

uint32_t TPCheckBusy(void)
{
	if (tp.state == TPSTATE_IDLE)
	{
		return FALSE;
	}
	return TRUE;
}

void TPFeedToMark(uint16_t line)
{
	line <<= 1;		// 一个点行等于两步
	TPPrintCmdToBuf(TP_CMD_FEED_TO_MARK, (uint8_t *)(&line), sizeof(line));
}

uint8_t IsPrintBufEmpty(void)
{
	if(tp.head != tp.tail)	
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

void TPPrintTestPage(void)
{
    uint32_t len,i;
    char buf[64];

    return;
}

//加热前几行的加热时间调整
static uint32_t TPHeatPreLineAdj(uint32_t tm)
{   
    static const uint8_t TpPrelineAdjTbl[] = {
        //150, 148, 146, 144, 142, 
        140, 138, 136, 134, 132, 
        130, 128, 126, 124, 122, 
        120, 118, 116, 114, 112, 
        110, 108, 106, 104, 102, 100,
    };

    if (tp.accel < ARRAY_SIZE(TpPrelineAdjTbl)) 
    {
        tm = tm * TpPrelineAdjTbl[tp.accel] / 100;
    }    
    
    return tm;
}

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

#if defined(NEW_HEAT_TIME)
  #if !defined(ADJ_PRINT_BY_VOL)
    heat = tp.heat_adj;
  #else
    heat = tp.heat_adj;//去掉电压调整浓度
  #endif
#else
    heat = TPHeatVoltageAdj(tp.heat_setting);
#endif    
    
    heat = TPHeatDotsAdj(heat,max_heat_dots);

  #if defined(TEMP_SNS_ENABLE)   
    temp = TPHTemperature();
    set_temperature(temp);
    heat = TPHeatThermalAdj(heat, temp);
  #endif    

    heat = TPHeatPreLineAdj(heat);
    heat *= heat_cnt;
    heat += TpMinWaitTime;
    
    while(1)
    {
        time_sum = 0;
        for(i=0; i<TP_MAX_STROBE_STEP; i++)
        {
            if((tp.accel+i) < (ARRAY_SIZE(TpAccelerationSteps) - 1))  //加速表内继续加速
            {
                time = TpAccelerationSteps[tp.accel+i];             
            }
            else
            {
                time = TpAccelerationSteps[ARRAY_SIZE(TpAccelerationSteps)-1];  //加速表最后一步                
            }          
                        
            tp.feed_time[i] = time;
            time_sum += time;
        }
                
        if(time_sum < heat)
        {
            if(tp.accel)  //原来的变速速程序会直接跳变
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
    uint16_t i,j,pt;        // 行缓冲区指针
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

static void TPDataDMAShiftToPrn(void)
{
    TPDataShiftOut(tp.heat_buf[tp.heat_cnt],ARRAY_SIZE(tp.heat_buf[0]));
}

static uint8_t TPCheckBuf(void)
{
    uint8_t ret;
    uint8_t flg = 0;   

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

            tp.state = TPSTATE_HEAT_WITH_FEED;
            ret = 1;
            break;
        case TP_CMD_FEED:
            tp.feedmax = TP_dot[tp.tail][0] | (TP_dot[tp.tail][1] << 8);
            tp.tail = (tp.tail+1)%TP_BUFLINE_MAX;// & (ARRAY_SIZE(TP_dot)-1);

            tp.state = TPSTATE_FEED;
            ret = 2;
            break;
        default:    // 未知类型，属于严重错误
            tp.tail = (tp.tail+1)%TP_BUFLINE_MAX;// & (ARRAY_SIZE(TP_dot)-1);
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

void TPISRProc(void)
{
    char cTemp[64];    

    switch (tp.state)
    {
    case TPSTATE_START:     // start
        switch (TPCheckBuf())
        {
        case 1:     // 打印
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
      
    case TPSTATE_HEAT_WITHOUT_FEED:

        if(tp.heat_remain)          // 还要继续加热，每行刚开始加热或者一个步进内加热时间足够时此条件不成立
        {           
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
                {
                    tp.heat_cnt++;

                    STROBE_0_OFF();
                    STROBE_1_OFF();

                    LATCH_LOW();
                    LATCH_HIGH();                   

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
                }
            }
            else    // no any data need to print//本行加热次数完成
            {                
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
                        tp.tail = (tp.tail+1)%TP_BUFLINE_MAX;// & (ARRAY_SIZE(TP_dot)-1);

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

void TPFeedStop(void)
{
    tp.feedmax = 0;     // interrupt will stop feed automaticcly
}

void TPFeedStart(void)
{
    if (tp.state == TPSTATE_IDLE)
    {
        TPFeedLine(50*8);      // 5mm
    }
}

void TPPrintLine(uint8_t *dot)
{
    if((!TPPrinterReady()) == TRUE)
    {        
        PRN_POWER_CHARGE();
        dev_user_delay_ms(5);
        TPPaperSNSDetect(0);
        
        if((!TPPrinterReady()) == TRUE)
        {
            tp.head = tp.tail = 0;
            tp.state = TPSTATE_IDLE;
            memset(tp.heat_buf, 0, sizeof(tp.heat_buf));
            
            PRN_POWER_DISCHARGE();

            return;
        }
    }

    TPPrintCmdToBuf(TP_CMD_PRINT_DOTLINE, dot, LineDot/8); 
}

void TPPrintRasterLine(uint8_t *dot)
{
    TPPrintCmdToBuf(TP_CMD_PRINT_DOTLINE_RASTER, dot, LineDot/8);
}

void PrtFeedStart(uint8_t len)
{
    if (tp.state == TPSTATE_IDLE)
    {
        TPFeedLine(len*8);      
    }

    return;
}

/**
 * @brief  用于打印机的图片打印接口
 * @param [in]  buf:图片缓存地址
 * @param [in]  x:图片的宽度
 * @param [in]  y:图片的高度
 */
void PrtPrintStart(const uint8_t *buf, const uint32_t x, const uint32_t y)
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

/**
 * @brief  用于设置打印机的浓度
 * @param [in]  speed:浓度等级，从0~6
 */
void PrtSetSpeed(uint8_t speed)
{
    char cTemp[64];
    
    TPSetSpeed(speed);    
    return;
}

int16_t PrtGetTemperature(void)
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

void TPSelfTest_1(void)
{
    uint8_t dot[LineDot/8];         //定义一个行，行的字节数目是多少，这边一共是576/8=72
    uint32_t i,j,n;
    
    memset(dot, 0x00, sizeof(dot));
    pt_resume();    
  
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
    }
   
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
    }    
    
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
    }
    
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
    }  


    memset(dot, 0x00, sizeof(dot));
    for (i=0; i<64; i++)
    {
        TPPrintLine(dot);
    }
    dev_user_delay_ms(1000);
    pt_sleep();
}

void TPSelfTest_2(void)
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

void TPSelfTest_5(void)
{
    uint32_t i;
    uint32_t n;
    uint8_t buf[LineDot/8];
    uint8_t dot[LineDot/8];   
    
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

void TPSelfTest_4(void)
{
    uint32_t i;
    uint32_t n;
    uint8_t buf[LineDot/8];
    uint8_t dot[LineDot/8];  

    pt_resume();   
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
}

void TPSelfTest_3(void)
{
    uint32_t i,j,n;
    uint8_t buf[LineDot/8];
    uint8_t dot[LineDot/8];

    pt_resume();

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

/**
 * @brief  打印机设备初始化
 */
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

