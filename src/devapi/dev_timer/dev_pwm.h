#ifndef __DEV_PWM_H
#define __DEV_PWM_H

typedef struct _STR_PWM_CFG
{
    u32 m_pwmhz;        //PWM时钟
    u16 m_LowPeriod;  //低电平周期
    u16 m_HighPeriod; //高电平周期
}str_pwm_cfg_t;

s32 dev_pwm_open(s32 pwmid, str_pwm_cfg_t pwm_cfg);
s32 dev_pwm_close(s32 pwmid);
s32 dev_pwm_set(s32 pwmid, const str_pwm_cfg_t pwm_cfg);
s32 dev_pwm_run(s32 pwmid);
s32 dev_pwm_stop(s32 pwmid);
#endif

