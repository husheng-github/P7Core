#include "devglobal.h"
#include "ddi_com.h"
#include "ddi_gprs.h"
#include "drv_wireless_io.h"
#include "dev_wireless_thread.h"
#include "module.h"
#include "dev_wireless.h"
#include "module_FIBOCOM.h"
#include "wireless_control_strategy.h"
#include "wireless_data_handle.h"

#if(WIRELESS_EXIST == 1)
static s32 g_wireless_thread_run_flag = FALSE;
static wireless_thread_node_param_t g_wireless_thread_node_param;

void dev_wireless_thread_node_param_init(void)
{
    memset(&g_wireless_thread_node_param, 0, sizeof(g_wireless_thread_node_param));
}

/**
 * @brief 无线任务运行开关
 */
wireless_thread_node_param_t *dev_wireless_get_thread_node_param(void)
{
    return &g_wireless_thread_node_param;
}

/**
* @brief 设置任务参数----主要是在发送某条指令等数据接收的时候，如果来了socket数据，可以先处理，收完后再接着处理之前的命令
* @param[in] wifi_thread_node_param 之前命令的线程参数
* @retval  无
 */
void dev_wireless_set_thread_node_param(wireless_thread_node_param_t *wireless_thread_node_param)
{
    memcpy(&g_wireless_thread_node_param, wireless_thread_node_param, sizeof(wireless_thread_node_param_t));
}

/**
* @brief 设置任务参数的超时时间
* @param[in] ms_timeout 任务超时时间
* @retval  无
 */
void dev_wireless_thread_set_timeout(u32 ms_timeout)
{
    g_wireless_thread_node_param.m_timeover = (ms_timeout+WIRELESS_TIMER_BASE-1)/WIRELESS_TIMER_BASE;
}

/**
* @brief 设置任务参数
* @param[in] task_id 任务id
* @param[in] hand_func 任务执行函数
* @param[in] timeover 任务超时时间
* @retval  无
 */
void dev_wireless_thread_set_param(WIRE_TASK_ID task_id, wire_thread_handle_pfunc hand_func, u32 timeover)
{
    g_wireless_thread_node_param.m_task_id = task_id;
    g_wireless_thread_node_param.m_func = hand_func;
    g_wireless_thread_node_param.m_step = 0;
    g_wireless_thread_node_param.m_result = 0;
    g_wireless_thread_node_param.m_lparam = 0;
    g_wireless_thread_node_param.m_wparam = 0;
    g_wireless_thread_node_param.m_param1 = 0;
    g_wireless_thread_node_param.m_param2 = 0;
    dev_wireless_thread_set_timeout(timeover);

    if(WS_TASK_SOCKET_RECVDATA != g_wireless_thread_node_param.m_task_id)
    {
        dev_wireless_clear_bak_node_param();
    }
}

void dev_wireless_thread_param_clean(void)
{
    g_wireless_thread_node_param.m_lparam = 0;
    g_wireless_thread_node_param.m_wparam = 0;
    g_wireless_thread_node_param.m_param1 = 0;
    g_wireless_thread_node_param.m_param2 = 0;
}

/**
* @brief 设置任务结果
* @param[in] result 任务结果
* @retval  无
 */
void dev_wireless_thread_set_result(WIRE_AT_RESULT_E result)
{
    g_wireless_thread_node_param.m_task_id = WIRE_TASK_FREE;
    g_wireless_thread_node_param.m_func = NULL;
    g_wireless_thread_node_param.m_step = 0;
    g_wireless_thread_node_param.m_result = result;
    g_wireless_thread_node_param.m_lparam = 0;
    g_wireless_thread_node_param.m_wparam = 0;
    dev_wireless_thread_set_timeout(0);
}


/**
 * @brief 无线任务运行开关
 */
s32 dev_wireless_thread_run_switch(s32 onoff)
{
    g_wireless_thread_run_flag = onoff;
}

/**
* @brief 判断任务ID是否变成了FREE以外的TASKID
* @param[in] 
* @retval  TRUE 任务变了
* @retval  FALSE 有执行任务
 */
s32 dev_wireless_thread_task_is_change(WIRE_TASK_ID task_id)
{
    //ddi_watchdog_feed();   //sxl这里不要喂狗，由应用和有需要的地方去喂狗，控制在中断可以，其他程序死机的情况
    if(g_wireless_thread_node_param.m_task_id != WIRE_TASK_FREE
        && g_wireless_thread_node_param.m_task_id != task_id)
    {
        if(WS_TASK_SOCKET_RECVDATA == g_wireless_thread_node_param.m_task_id)
        {
            return FALSE;
        }
        else
        {
            TRACE_ERR("task change to %d", g_wireless_thread_node_param.m_task_id);
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
s32 dev_wireless_thread_is_free(void)
{
    if(dev_wireless_module_is_upgrading())
    {
        //TRACE_INFO("module is upgrading now:%d", g_wireless_thread_node_param.m_task_id);
        return FALSE;
    }
    else
    {
        return (g_wireless_thread_node_param.m_task_id == WIRE_TASK_FREE 
            || (WS_TASK_SOCKET_RECVDATA == g_wireless_thread_node_param.m_task_id && !dev_wireless_is_have_bak_node_param()));
    }
}

/**
 * @brief 无线任务执行函数，包括模块控制策略、数据接收和分发处理
 */
void dev_wireless_thread(void)
{
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();
    wireless_device_info_t *device_info = dev_wireless_get_device();
    
    if(!g_wireless_thread_run_flag || device_info->m_transparent_switch)
    {
        return ;
    }

    //模块控制策略
    if(WS_TASK_OPEN_AIRPLANE == thread_node_param->m_task_id
        || WS_TASK_CLOSE_AIRPLANE == thread_node_param->m_task_id
        || WS_TASK_TELEPHONE_DAIL == thread_node_param->m_task_id
        || WS_TASK_TELEPHONE_HANGUP == thread_node_param->m_task_id)
    {
        
    }
    else if(NULL == device_info->m_dial_log_buff   //拨号时不进入无线处理策略
                && dev_wireless_thread_is_free() 
                && WS_NO_ERR == dev_wireless_get_status(WS_GET_REG_NETWORK_FAIL_REASON))
    {
        dev_wireless_control_strategy();
    }
    
    //数据读取与分发处理
    dev_wireless_data_handle();
}
#endif
