#ifndef __DEV_WIFI_CONTROL_STRATEGY_
#define __DEV_WIFI_CONTROL_STRATEGY_

typedef enum{
    WF_NORMAL_RUN,        //����������
    WF_SCAN_AP,           //ɨ���ȵ㣬ֻ��Ҫ���е���ʼ�����̼���
    WF_AIR_KISS_CONFIG,   //΢��������ֻ��Ҫ���е���ʼ�����̼���
    WF_WEB_CONFIG         //��ҳ������ֻ��Ҫ���е���ʼ�����̼��ɣ��������Ϊ����·ģʽ
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

