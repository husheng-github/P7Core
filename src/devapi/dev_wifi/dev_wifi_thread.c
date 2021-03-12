#include "devglobal.h"
#include "drv_wifi_io.h"
#include "dev_wifi_thread.h"
#include "dev_wifi.h"
#include "wifi_control_strategy.h"
#include "wifi_data_handle.h"
#include "dev_wifi_wrapper.h"

#if (WIFI_EXIST == 1)
static s32 g_wifi_thread_run_flag = FALSE;
static wifi_thread_node_param_t g_wifi_thread_node_param;

void dev_wifi_thread_node_param_init(void)
{
    memset(&g_wifi_thread_node_param, 0, sizeof(g_wifi_thread_node_param));
}

/**
 * @brief �����������п���
 */
wifi_thread_node_param_t *dev_wifi_get_thread_node_param(void)
{
    return &g_wifi_thread_node_param;
}

/**
* @brief �����������----��Ҫ���ڷ���ĳ��ָ������ݽ��յ�ʱ���������socket���ݣ������ȴ���������ٽ��Ŵ���֮ǰ������
* @param[in] wifi_thread_node_param ֮ǰ������̲߳���
* @retval  ��
 */
void dev_wifi_set_thread_node_param(wifi_thread_node_param_t *wifi_thread_node_param)
{
    memcpy(&g_wifi_thread_node_param, wifi_thread_node_param, sizeof(wifi_thread_node_param_t));
}

/**
* @brief ������������ĳ�ʱʱ��
* @param[in] ms_timeout ����ʱʱ��
* @retval  ��
 */
void dev_wifi_thread_set_timeout(u32 ms_timeout)
{
    g_wifi_thread_node_param.m_timeover = (ms_timeout+WIFI_TIMER_BASE-1)/WIFI_TIMER_BASE;
}

/**
* @brief �����������
* @param[in] task_id ����id
* @param[in] hand_func ����ִ�к���
* @param[in] timeover ����ʱʱ��
* @retval  ��
 */
void dev_wifi_thread_set_param(WIFI_TASK_ID task_id, wifi_thread_handle_pfunc hand_func, u32 timeover)
{
    g_wifi_thread_node_param.m_task_id = task_id;
    g_wifi_thread_node_param.m_func = hand_func;
    g_wifi_thread_node_param.m_step = 0;
    g_wifi_thread_node_param.m_result = 0;
    g_wifi_thread_node_param.m_lparam = 0;
    g_wifi_thread_node_param.m_wparam = 0;
    g_wifi_thread_node_param.m_param1 = 0;
    g_wifi_thread_node_param.m_param2 = 0;
    dev_wifi_thread_set_timeout(timeover);
}

void dev_wifi_thread_param_clean(void)
{
    g_wifi_thread_node_param.m_lparam = 0;
    g_wifi_thread_node_param.m_wparam = 0;
    g_wifi_thread_node_param.m_param1 = 0;
    g_wifi_thread_node_param.m_param2 = 0;
}

/**
* @brief ����������
* @param[in] result ������
* @retval  ��
 */
void dev_wifi_thread_set_result(WIFI_AT_RESULT_E result)
{
    g_wifi_thread_node_param.m_task_id = WIFI_TASK_FREE;
    g_wifi_thread_node_param.m_func = NULL;
    g_wifi_thread_node_param.m_step = 0;
    g_wifi_thread_node_param.m_result = result;
    g_wifi_thread_node_param.m_lparam = 0;
    g_wifi_thread_node_param.m_wparam = 0;
    dev_wifi_thread_set_timeout(0);
}


/**
 * @brief �����������п���
 */
s32 dev_wifi_thread_run_switch(s32 onoff)
{
    g_wifi_thread_run_flag = onoff;
}

/**
* @brief �ж�����ID�Ƿ�����FREE�����TASKID
* @param[in] 
* @retval  TRUE �������
* @retval  FALSE ��ִ������
 */
s32 dev_wifi_thread_task_is_change(WIFI_TASK_ID task_id)
{
    ddi_watchdog_feed();
    if(g_wifi_thread_node_param.m_task_id != WIFI_TASK_FREE
        && g_wifi_thread_node_param.m_task_id != task_id)
    {
        //Add by xiaohonghui 2020.8.12 task_id��Ϊ�������ݣ���������ı�
        if(WIFI_TASK_SOCKET_RECVDATA == g_wifi_thread_node_param.m_task_id)
        {
            return FALSE;
        }
        else
        {
            TRACE_ERR("task change to %d", g_wifi_thread_node_param.m_task_id);
            return TRUE;
        }
    }
    else
    {
        return FALSE;
    }
}

/**
* @brief �����Ƿ�������ִ������
* @param[in] 
* @retval  TRUE �������
* @retval  FALSE ��ִ������
 */
s32 dev_wifi_thread_is_free(void)
{
    return (g_wifi_thread_node_param.m_task_id == WIFI_TASK_FREE || WIFI_TASK_SOCKET_RECVDATA == g_wifi_thread_node_param.m_task_id);
}

/**
 * @brief ��������ִ�к���������ģ����Ʋ��ԡ����ݽ��պͷַ�����
 */
void dev_wifi_thread(void)
{
    wifi_thread_node_param_t *thread_node_param = (wifi_thread_node_param_t *)dev_wifi_get_thread_node_param();
    wifi_device_info_t *device_info = dev_wifi_get_device();

#ifdef WIFI_FIXED_FREQ_TEST
    if(1)
#else
    if(!g_wifi_thread_run_flag || device_info->m_transparent_switch)
#endif
    {
        return ;
    }

    //ģ����Ʋ���
    if(!device_info->m_use_tcp_server
        && dev_wifi_thread_is_free() 
        && (WF_NO_ERR == dev_wifi_get_status(WIFI_GET_REG_NETWORK_FAIL_REASON)
        || WF_AP_IS_NULL == dev_wifi_get_status(WIFI_GET_REG_NETWORK_FAIL_REASON)))
    {
        dev_wifi_control_strategy();
    }
    /*else
    {
        TRACE_DBG("task_id:%d, %d", thread_node_param->m_task_id, dev_wifi_get_status(WIFI_GET_REG_NETWORK_FAIL_REASON));
    }*/

    //���ݶ�ȡ��ַ�����
    dev_wifi_data_handle();
}

#endif
