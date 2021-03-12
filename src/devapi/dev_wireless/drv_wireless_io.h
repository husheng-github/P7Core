#ifndef __DRV_WIRELESS_IO_H
#define __DRV_WIRELESS_IO_H

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define WIRELESS_WITH_POWERKEY                 FALSE         //硬件是否有上电管脚 TRUE:有    ,FALSE:没有
#define WIRELESS_NEED_SLEEP                    FALSE         //无线模块是否需低功耗控制
#define WIRELESS_USE_GPIO                      FALSE         //是否有无线模块上的gpio接我们自己的设备
#define WIRELESS_USE_EXTERANL_AMP              TRUE          //是否控制外部功放  //sxl?2020测试用，要还原
#define WIRELESS_USE_UART_RING                 FALSE          //是否需要无线模块唤醒MCU
#define WIRELESS_WITH_BT                       FALSE          //无线模块上是否挂载蓝牙
#define WIRELESS_CONTROL_BATTERY_COLLECT       FALSE          //无线模块上电的时候是否延迟电池采样  目前T6需要
#define WIRELESS_USE_SAVEEDVOL                 TRUE          //声音保存到flash
#define WIRELESS_CLOSE_AUDIO_PLAY_SLEEP        FALSE          //去掉RDA在声音播放前的延时100ms

#ifdef MACHINE_T8_OLD
#define GPRS_WAKEUP_PIN        GPIO_PIN_PTA0    //WAKEUP
#define GPRS_POWEREN_PIN       GPIO_PIN_PTA1    //MCU端控制无线模块的电源使能脚，一般是先使能给无线模块供电，然后再置powerkey
#define GPRS_POWERON_PIN       GPIO_PIN_NONE                //powerkey，通过此管脚让无线模块开机、关机
#else
#define GPRS_WAKEUP_PIN        GPIO_PIN_NONE
#define GPRS_POWEREN_PIN       GPIO_PIN_PTC9   //MCU端电源使能脚
#define GPRS_POWERON_PIN       GPIO_PIN_NONE                //powerkey，通过此管脚让无线模块开机、关机
#endif

void drv_gprs_io_init(void);
void drv_wireless_set_ringanddtrpin_init(void);
void drv_wireless_ringanddtrpin_enter_sleep(void);
void drv_wireless_ringanddtrpin_exit_sleep(void);



#endif