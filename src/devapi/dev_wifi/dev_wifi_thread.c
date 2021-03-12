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
 * @brief 无线任务运行开关
 */
wifi_thread_node_param_t *dev_wifi_get_thread_node_param(void)
{
    return &g_wifi_thread_node_param;
}

/**
* @brief 设置任务参数----主要是在发送某条指令等数据接收的时候，如果来了socket数据，可以先处理，收完后再接着处理之前的命令
* @param[in] wifi_thread_node_param 之前命令的线程参数
* @retval  无
 */
void dev_wifi_set_thread_node_param(wifi_thread_node_param_t *wifi_thread_node_param)
{
    memcpy(&g_wifi_thread_node_param, wifi_thread_node_param, sizeof(wifi_thread_node_param_t));
}

/**
* @brief 设置任务参数的超时时间
* @param[in] ms_timeout 任务超时时间
* @retval  无
 */
void dev_wifi_thread_set_timeout(u32 ms_timeout)
{
    g_wifi_thread_node_param.m_timeover = (ms_timeout+WIFI_TIMER_BASE-1)/WIFI_TIMER_BASE;
}

/**
* @brief 设置任务参数
* @param[in] task_id 任务id
* @param[in] hand_func 任务执行函数
* @param[in] timeover 任务超时时间
* @retval  无
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
* @brief 设置任务结果
* @param[in] result 任务结果
* @retval  无
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
 * @brief 无线任务运行开关
 */
s32 dev_wifi_thread_run_switch(s32 onoff)
{
    g_wifi_thread_run_flag = onoff;
}

/**
* @brief 判断任务ID是否变成了FREE以外的TASKID
* @param[in] 
* @retval  TRUE 任务变了
* @retval  FALSE 有执行任务
 */
s32 dev_wifi_thread_task_is_change(WIFI_TASK_ID task_id)
{
    ddi_watchdog_feed();
    if(g_wifi_thread_node_param.m_task_id != WIFI_TASK_FREE
        && g_wifi_thread_node_param.m_task_id != task_id)
    {
        //Add by xiaohonghui 2020.8.12 task_id变为接收数据，不算任务改变
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
* @brief 任务是否无其它执行任务
* @param[in] 
* @retval  TRUE 任务空闲
* @retval  FALSE 有执行任务
 */
s32 dev_wifi_thread_is_free(void)
{
    return (g_wifi_thread_node_param.m_task_id == WIFI_TASK_FREE || WIFI_TASK_SOCKET_RECVDATA == g_wifi_thread_node_param.m_task_id);
}

/**
 * @brief 无线任务执行函数，包括模块控制策略、数据接收和分发处理
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

    //模块控制策略
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

    //数据读取与分发处理
    dev_wifi_data_handle();
}

#endif
