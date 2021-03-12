#ifndef __DRV_TIMER_H
#define __DRV_TIMER_H

#ifdef DEBUG_TIMER_EN
#define TIMER_DEBUG(format,...)    dev_debug_printf("%s(%d):"format, __func__, __LINE__, ##__VA_ARGS__)
#define TIMER_DEBUGHEX             dev_debug_printformat
#else
#define TIMER_DEBUG(...) 
#define TIMER_DEBUGHEX(...)
#endif

#define PWM0_PIN    GPIO_PIN_PTA0   //GPIO_PIN_PTB0
#define PWM1_PIN    GPIO_PIN_PTA1   //GPIO_PIN_PTB1
#define PWM2_PIN    GPIO_PIN_PTA2   //GPIO_PIN_PTB2
#define PWM3_PIN    GPIO_PIN_PTA3   //GPIO_PIN_PTB3
#define PWM4_PIN    GPIO_PIN_PTA4   //GPIO_PIN_PTB4
#define PWM5_PIN    GPIO_PIN_PTA5   //GPIO_PIN_PTB5
#define PWM6_PIN    GPIO_PIN_PTA6   //GPIO_PIN_PTB6
#define PWM7_PIN    GPIO_PIN_PTA7   //GPIO_PIN_PTB7

#define DRVTIMNUM_MAX    8

typedef enum {
    TIMER_MODE_CNT = 0,         //count time
    TIMER_MODE_PWM,             // PWM
//    TIMER_MODE_CAPTURE          //CAPTURE
}TIMER_CONFIG_MODE;


typedef struct __DRVTIMER_CFG{
    TIMER_CONFIG_MODE m_mode;
    u16  m_reload;      //自动重载
    u16  m_toutflg;     //定时器到标志
    u32 m_tus;          //定时值
    u32 m_pwmhz;        //PWM时钟
    u16 m_LowLevelPeriod;  //低电平周期
    u16 m_HighLevelPeriod; //高电平周期
    
}DRVTIMER_CFG_t;


void drv_timer_init(void);
s32 drv_timer_request(s32 timid, const DRVTIMER_CFG_t t_cfg, void(*pFun)(void*), void *data);
s32 drv_timer_free(u32 timid);
void drv_timer_stop(u32 timid);
void drv_timer_run(u32 timid);
void drv_timer_setvalue(u32 timid, u32 tus);
void drv_timer_setvalue_clk(u32 timid, u32 tclk);
void drv_timer_int_enable(u32 timid);
void drv_timer_int_disable(u32 timid);
void drv_timer_int_clear(u32 timid);
void drv_timer_pwmset(s32 pwmid, const DRVTIMER_CFG_t t_cfg);
s32 drv_pwm_pincfg(u32 pwmid, u8 mod);

#endif
