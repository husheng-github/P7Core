#ifndef __DEV_TIMER_H
#define __DEV_TIMER_H

#if 0
typedef enum _TIME_NumType
{
    TIM_NO0 = 0,
    TIM_NO1 = 1,
    TIM_NO2 = 2,
    TIM_NO3 = 3,
    TIM_NO4 = 4,
    TIM_NO5 = 5,
}time_numtype_t;
#endif

typedef struct _STR_TIMER_CFG
{
    u32 m_tus;          //定时值 
    u16  m_reload;      //自动重载
}str_timer_cfg_t;


void dev_timer_init(void);
s32 dev_timer_request(s32 timid, const str_timer_cfg_t t_cfg, void(*pFun)(void*), void *data);
s32 dev_timer_free(u32 timid);
void dev_timer_stop(u32 timid);
void dev_timer_run(u32 timid);
void dev_timer_setvalue(u32 timid, u32 tus);
void dev_timer_setvalue_clk(u32 timid, u32 clk);
void dev_timer_int_enable(u32 timid);
void dev_timer_int_disable(u32 timid);
void dev_timer_int_clear(u32 timid);

#endif

