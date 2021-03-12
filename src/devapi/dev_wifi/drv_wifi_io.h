#ifndef __DRV_WIFI_IO_H
#define __DRV_WIFI_IO_H

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define WIFI_EN_PIN     wifi_get_en_pin()
#define WIFI_WAKE_PIN   wifi_get_wakeup_pin()

void drv_wifi_io_init(void);



#endif