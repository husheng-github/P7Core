

#ifndef _DDI_LED_H_
#define _DDI_LED_H_


#include "ddi_common.h"

//定义LED序号
#define   LED_NUM_MAX         4   //LED个数

#if 1
#define   LED_STATUS_B        0
#define   LED_STATUS_R         1

#define   LED_SIGNAL_Y      2
#define   LED_SIGNAL_B       3


#else
#define   LED_BLUE_NO        0
#define   LED_YELLOW_NO      1
#define   LED_GREEN_NO       2
#define   LED_RED_NO         3
#endif
#define   LED_ALL           0xff


#define   LED_OFF          0
#define   LED_ON           1

typedef struct _strLedGleamPara
{
    u32 m_led;          //LED编号
    u32 m_ontime;       //周期内亮保持时间，单位毫秒
    u32 m_offtime;      //周期内灭保持时间，单位毫秒
    u32 m_duration;     //闪烁持续时间，单位毫秒
}strLedGleamPara;


/********************************宏的定义***********************************/
/*Led等控制指令*/
#define     DDI_LED_CTL_VER                     (0) //获取led模块版本
#define     DDI_LED_CTL_GLEAM                   (1) //控制led灯闪烁
#define     DDI_LED_CTL_CLOSE_YELLOW            (2) //硬件未加黄灯，软件是将信号灯的黄灯闪烁作为开机状态，注网后，就将此状态关闭
#define     DDI_LED_CTL_LOOP                    (3) //灯循环亮,1s一次


//=====================================================
//对外函数声明
extern s32 ddi_led_open (void);
extern s32 ddi_led_close (void);
extern s32 ddi_led_sta_set(u32 nLed, u32 nSta);
extern s32 ddi_led_ioctl(u32 nCmd, u32 lParam, u32 wParam);

typedef s32 (*core_ddi_led_open) (void); 
typedef s32 (*core_ddi_led_close) (void); 
typedef s32 (*core_ddi_led_sta_set) (u32 nLed, u32 nSta); 
typedef s32 (*core_ddi_led_ioctl)(u32 nCmd, u32 lParam, u32 wParam);

#endif

