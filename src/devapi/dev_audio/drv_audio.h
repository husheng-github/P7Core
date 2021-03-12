#ifndef __DRV_AUDIO_H
#define __DRV_AUDIO_H

#ifdef DEBUG_AUDIO_EN
#define AUDIO_DEBUG(format,...)    dev_debug_printf("%s(%d):"format, __func__, __LINE__, ##__VA_ARGS__)
#define AUDIO_DEBUGHEX             dev_debug_printformat
#else
#define AUDIO_DEBUG(...) 
#define AUDIO_DEBUGHEX(...)
#endif

#if(MACHINETYPE==MACHINE_S1)
#define BEEP_PWM_CH     1  //PWM1  PB1
#define BEEP_PWM_HZ_Default     4800 
#else
#define BEEP_PWM_CH     2  //PWM2
#define BEEP_PWM_HZ_Default     7000//6500//2700 //4000//
//#define BEEP_PWM_HZ_M5_Default  4000
#endif   
#define BEEP_LowLevelPeriod   1
#define BEEP_HighLevelPeriod   1

s32 drv_audio_open(void);
s32 drv_audio_close(void);
void drv_audio_task(void);
s32 drv_audio_beep(strbeepParam_t l_beepparam);
s32 drv_audio_suspend(void);
s32 drv_audio_resume(void);
void drv_audio_set_pwm(u32 pwmhz);
u32 drv_audio_get_pwm(void);

#endif

