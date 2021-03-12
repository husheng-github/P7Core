#ifndef __DRV_WIRELESS_IO_H
#define __DRV_WIRELESS_IO_H

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define WIRELESS_WITH_POWERKEY                 FALSE         //Ӳ���Ƿ����ϵ�ܽ� TRUE:��    ,FALSE:û��
#define WIRELESS_NEED_SLEEP                    FALSE         //����ģ���Ƿ���͹��Ŀ���
#define WIRELESS_USE_GPIO                      FALSE         //�Ƿ�������ģ���ϵ�gpio�������Լ����豸
#define WIRELESS_USE_EXTERANL_AMP              TRUE          //�Ƿ�����ⲿ����  //sxl?2020�����ã�Ҫ��ԭ
#define WIRELESS_USE_UART_RING                 FALSE          //�Ƿ���Ҫ����ģ�黽��MCU
#define WIRELESS_WITH_BT                       FALSE          //����ģ�����Ƿ��������
#define WIRELESS_CONTROL_BATTERY_COLLECT       FALSE          //����ģ���ϵ��ʱ���Ƿ��ӳٵ�ز���  ĿǰT6��Ҫ
#define WIRELESS_USE_SAVEEDVOL                 TRUE          //�������浽flash
#define WIRELESS_CLOSE_AUDIO_PLAY_SLEEP        FALSE          //ȥ��RDA����������ǰ����ʱ100ms

#ifdef MACHINE_T8_OLD
#define GPRS_WAKEUP_PIN        GPIO_PIN_PTA0    //WAKEUP
#define GPRS_POWEREN_PIN       GPIO_PIN_PTA1    //MCU�˿�������ģ��ĵ�Դʹ�ܽţ�һ������ʹ�ܸ�����ģ�鹩�磬Ȼ������powerkey
#define GPRS_POWERON_PIN       GPIO_PIN_NONE                //powerkey��ͨ���˹ܽ�������ģ�鿪�����ػ�
#else
#define GPRS_WAKEUP_PIN        GPIO_PIN_NONE
#define GPRS_POWEREN_PIN       GPIO_PIN_PTC9   //MCU�˵�Դʹ�ܽ�
#define GPRS_POWERON_PIN       GPIO_PIN_NONE                //powerkey��ͨ���˹ܽ�������ģ�鿪�����ػ�
#endif

void drv_gprs_io_init(void);
void drv_wireless_set_ringanddtrpin_init(void);
void drv_wireless_ringanddtrpin_enter_sleep(void);
void drv_wireless_ringanddtrpin_exit_sleep(void);



#endif