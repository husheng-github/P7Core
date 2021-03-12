#ifndef __DEV_USERTIMER_H
#define __DEV_USERTIMER_H


void dev_user_timer_init(void);
s32 dev_user_timer_open(u32 *timer);
s32 dev_user_timer_close(u32 *timer);
u32 dev_user_gettimeID(void);
u32 dev_user_querrytimer(u32 Time, u32 Delay);
u32 dev_user_querrytimer_us(u32 Time, u32 Delay);
void dev_user_delay_us(u32 nus);
void dev_user_delay_ms(u32 nms);
#endif

