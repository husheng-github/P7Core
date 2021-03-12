#ifndef __DEV_LED_H
#define __DEV_LED_H



void dev_led_task(void);
s32 dev_led_open (void);
s32 dev_led_close (void);
s32 dev_led_sta_set(u32 nLed, u32 nSta);
s32 dev_led_ioctl(u32 nCmd, u32 lParam, u32 wParam);


#endif

