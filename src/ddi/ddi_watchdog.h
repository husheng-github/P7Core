

#ifndef _DDI_WATCHDOG_H_
#define _DDI_WATCHDOG_H_


#include "ddi_common.h"


//=====================================================
//对外函数声明
extern void ddi_watchdog_open(void);
extern void ddi_watchdog_close (void);
extern void ddi_watchdog_feed(void);
//====================================================
typedef void (*core_ddi_watchdog_open) (void);
typedef void (*core_ddi_watchdog_close) (void);
typedef void (*core_ddi_watchdog_feed) (void);

#endif

