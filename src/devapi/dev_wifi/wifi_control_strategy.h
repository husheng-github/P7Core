#ifndef __DEV_WIFI_CONTROL_STRATEGY_
#define __DEV_WIFI_CONTROL_STRATEGY_

typedef enum{
    WF_NORMAL_RUN,        //正常流程跑
    WF_SCAN_AP,           //扫描热点，只需要运行到初始化流程即可
    WF_AIR_KISS_CONFIG,   //微信配网，只需要运行到初始化流程即可
    WF_WEB_CONFIG         //网页配网，只需要运行到初始化流程即可，并且配成为多链路模式
}WIFI_STRATEGY_E;
static void wifi_power_on_process(void);
static void wifi_module_init(void);
static void wifi_connect_ap_process(void);
static void wifi_disconnect_ap_process(void);

static void wifi_at_timeout_handle(s8 *atcmd, u32 timeout);
void dev_wifi_set_control_strategy_step(WIFI_CONTROL_STRATEGY_STEP_E control_step);
void dev_wifi_set_control_strategy_fail(WIFI_CONTROL_FAIL_REASON_E control_fail_reason);
void dev_wifi_control_strategy(void);

#endif

