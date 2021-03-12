#ifndef __DEV_POWER_H
#define __DEV_POWER_H

#define WAKEUP_MOD_RTC          (1<<31)  //RTC����
#define WAKEUP_MOD_REQ_GPIO     (1<<30)  //����Ż���(S1����CPUͨѶ��һ��ר�ŵĻ��ѽ�)
#define WAKEUP_MOD_KEYPAD       (1<<29)  //��������(M2P�ð�������)

#define WAKEUP_MOD_UART3        (1<<3)  //����3���ջ���
#define WAKEUP_MOD_UART2        (1<<2)  //����2���ջ���
#define WAKEUP_MOD_UART1        (1<<1)  //����1���ջ���
#define WAKEUP_MOD_UART0        (1<<0)  //����0���ջ���

typedef struct _str_WAKEUP_INFO
{
    u32 m_mod;              //����ģʽ
    u32 m_rtc_ts;           //rts����ʱ��(��λ:s)
    
}str_wakeup_info_t;

void dev_power_init(void);
void dev_power_switch_ctl(u8 flg);
s32 dev_power_sleep(u32 mod);

#endif

