#ifndef __DEV_WIRELESS_CONTROL_STRATEGY_
#define __DEV_WIRELESS_CONTROL_STRATEGY_

static void wireless_at_timeout_handle(s8 *atcmd, u32 timeout);
void dev_wireless_set_control_strategy_step(WIRE_CONTROL_STRATEGY_STEP_E control_step);
void dev_wireless_set_control_strategy_fail(WIRE_CONTROL_FAIL_REASON_E control_fail_reason);
void dev_wireless_control_strategy(void);

#endif

