#ifndef __DEV_POWER_H
#define __DEV_POWER_H

#define WAKEUP_MOD_RTC          (1<<31)  //RTC唤醒
#define WAKEUP_MOD_REQ_GPIO     (1<<30)  //请求脚唤醒(S1和主CPU通讯有一个专门的唤醒脚)
#define WAKEUP_MOD_KEYPAD       (1<<29)  //按键唤醒(M2P用按键唤醒)

#define WAKEUP_MOD_UART3        (1<<3)  //串口3接收唤醒
#define WAKEUP_MOD_UART2        (1<<2)  //串口2接收唤醒
#define WAKEUP_MOD_UART1        (1<<1)  //串口1接收唤醒
#define WAKEUP_MOD_UART0        (1<<0)  //串口0接收唤醒

typedef struct _str_WAKEUP_INFO
{
    u32 m_mod;              //唤醒模式
    u32 m_rtc_ts;           //rts唤醒时间(单位:s)
    
}str_wakeup_info_t;

void dev_power_init(void);
void dev_power_switch_ctl(u8 flg);
s32 dev_power_sleep(u32 mod);

#endif

