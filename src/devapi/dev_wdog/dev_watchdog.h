#ifndef __DEV_WATCHDOG_H
#define __DEV_WATCHDOG_H

s32 dev_watchdog_open(void);
s32 dev_watchdog_close(void);
s32 dev_watchdog_feed(void);
s32 dev_watchdog_set_time(u32 ts);

#endif
