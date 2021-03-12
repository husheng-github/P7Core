#ifndef __DEVGLOBAL_H
#define __DEVGLOBAL_H
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include "typedefine.h"
#include "dev_errno.h"



#ifndef SXL_DEBUG   //sxl?2017
//#define SXL_DEBUG
#endif



#include "devapi/dev_debug/dev_debug.h"
#include "devapi/dev_memoryalloc/dev_memoryalloc.h"
#include "devapi/dev_circlequeue/dev_circlequeue.h"
#include "devapi/dev_gpio/dev_gpio.h"
#include "devapi/dev_irq/dev_irq.h"
#include "devapi/dev_maths/dev_maths.h"
#include "devapi/dev_misc/dev_misc.h"
#include "devapi/dev_spi/dev_spi.h"
#include "devapi/dev_flash/drv_internalflash.h"
#include "devapi/dev_flash/dev_flash.h"
#include "devapi/dev_timer/dev_timer.h"
#include "devapi/dev_timer/dev_usertimer.h"
#include "devapi/dev_timer/dev_pwm.h"
#include "devapi/dev_uart/dev_uart.h"
#include "devapi/dev_rf/dev_rf_api.h"
#include "devapi/dev_rf/dev_rf_flow.h"
#include "devapi/dev_rf/dev_mifare.h"
#include "devapi/dev_adc/dev_adc.h"
#include "devapi/dev_dac/dev_dac.h"
#include "devapi/dev_mag/dev_mag.h"
#include "devapi/dev_icc/dev_icc.h"

#include "devapi/dev_crypt/dev_des.h"
#include "devapi/dev_crypt/dev_trng.h"
#include "devapi/dev_crypt/dev_sm.h"
#include "devapi/dev_crypt/dev_hash.h"
#include "devapi/dev_crypt/dev_rsa.h"

#include "devapi/dev_smc/dev_smc.h"
#include "devapi/dev_dryice/dev_dryice.h"
#include "devapi/dev_rtc/dev_rtc.h"
#include "devapi/dev_led/dev_led.h"
#include "devapi/dev_audio/dev_audio.h"
#include "devapi/dev_keypad/dev_keypad.h"
#include "devapi/dev_font/dev_font.h"
#include "devapi/dev_lcd/dev_lcd.h"
#include "devapi/dev_power/dev_power.h"
#include "devapi/dev_bt/dev_bt.h"
#include "devapi/dev_printer/dev_pt48d.h"
#include "devapi/dev_wdog/dev_watchdog.h"
#include "devapi/dev_usb/dev_usbd_cdc.h"

#include "ddi_common.h"
#include "ddi_gprs.h"
#include "ddi_audio.h"
#include "devapi/dev_audio/ttsbin_handle.h"
#include "devapi/dev_audio/dev_audio.h"
#include "devapi/dev_wireless/dev_wireless.h"

#include "ddi_com.h"
#include "ddi_wifi.h"
#include "devapi/dev_wifi/dev_wifi.h"
#include "devapi/common/common.h"


#include "lfs.h"
#include "lfs_util.h"

//#include "ddi/ddi.h"


#endif
