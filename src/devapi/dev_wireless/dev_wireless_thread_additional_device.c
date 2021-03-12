#include "devglobal.h"
#include "ddi_com.h"
#include "ddi_gprs.h"
#include "drv_wireless_io.h"
#include "dev_wireless_thread.h"
#include "dev_wireless_thread_additional_device.h"
#include "module.h"
#include "dev_wireless.h"
#include "module_FIBOCOM.h"
#include "wireless_control_strategy.h"
#include "wireless_data_handle.h"
#include "dev_wireless_bt.h"

/*****************************************************************************
*                                                                            *
*  @file     dev_wireless_thread_additional_device.C                                                *
*  @brief    此文件主要是对无线模块上挂载的外围设备线程函数进行处理                                       *
*  Details.                                                                  *
*                                                                            *
*  @author   xiaohonghui                                                     *
*  Remark         : Description                                              *
*----------------------------------------------------------------------------*
*  Change History :                                                          *
*  <Date>     | <Version> | <Author>       | <Description>                   *
*----------------------------------------------------------------------------*
*  2020/010/17 | 1.0.0.1   | xiaohonghui    | Create file                     *
*----------------------------------------------------------------------------*
*                                                                            *
*****************************************************************************/


static wireless_thread_node_param_t g_wireless_thread_additional_node_param;     //无线上挂载的设备处理的线程参数

void dev_wireless_thread_addional_node_param_init(void)
{
    memset(&g_wireless_thread_additional_node_param, 0, sizeof(g_wireless_thread_additional_node_param));
}

/**
 * @brief 无线任务运行开关
 */
wireless_thread_node_param_t *dev_wireless_get_thread_additional_node_param(void)
{
    return &g_wireless_thread_additional_node_param;
}

/**
* @brief 设置任务参数----主要是在发送某条指令等数据接收的时候，如果来了socket数据，可以先处理，收完后再接着处理之前的命令
* @param[in] wifi_thread_node_param 之前命令的线程参数
* @retval  无
 */
void dev_wireless_set_thread_additional_node_param(wireless_thread_node_param_t *wireless_thread_node_param)
{
    memcpy(&g_wireless_thread_additional_node_param, wireless_thread_node_param, sizeof(wireless_thread_node_param_t));
}

/**
* @brief 设置任务参数的超时时间
* @param[in] ms_timeout 任务超时时间
* @retval  无
 */
void dev_wireless_thread_additional_set_timeout(u32 ms_timeout)
{
    g_wireless_thread_additional_node_param.m_timeover = (ms_timeout+WIRELESS_TIMER_BASE-1)/WIRELESS_TIMER_BASE;
}

/**
* @brief 设置任务参数
* @param[in] task_id 任务id
* @param[in] hand_func 任务执行函数
* @param[in] timeover 任务超时时间
* @retval  无
 */
void dev_wireless_thread_additional_set_param(WIRE_TASK_ID task_id, wire_thread_handle_pfunc hand_func, u32 timeover)
{
    g_wireless_thread_additional_node_param.m_task_id = task_id;
    g_wireless_thread_additional_node_param.m_func = hand_func;
    g_wireless_thread_additional_node_param.m_step = 0;
    g_wireless_thread_additional_node_param.m_result = 0;
    g_wireless_thread_additional_node_param.m_lparam = 0;
    g_wireless_thread_additional_node_param.m_wparam = 0;
    g_wireless_thread_additional_node_param.m_param1 = 0;
    g_wireless_thread_additional_node_param.m_param2 = 0;
    dev_wireless_thread_additional_set_timeout(timeover);
}

void dev_wireless_thread_additional_param_clean(void)
{
    g_wireless_thread_additional_node_param.m_lparam = 0;
    g_wireless_thread_additional_node_param.m_wparam = 0;
    g_wireless_thread_additional_node_param.m_param1 = 0;
    g_wireless_thread_additional_node_param.m_param2 = 0;
}

/**
* @brief 设置任务结果
* @param[in] result 任务结果
* @retval  无
 */
void dev_wireless_thread_additional_set_result(WIRE_AT_RESULT_E result)
{
    g_wireless_thread_additional_node_param.m_task_id = WIRE_TASK_FREE;
    g_wireless_thread_additional_node_param.m_func = NULL;
    g_wireless_thread_additional_node_param.m_step = 0;
    g_wireless_thread_additional_node_param.m_result = result;
    g_wireless_thread_additional_node_param.m_lparam = 0;
    g_wireless_thread_additional_node_param.m_wparam = 0;
    dev_wireless_thread_additional_set_timeout(0);
}

/**
* @brief 判断任务ID是否变成了FREE以外的TASKID
* @param[in] 
* @retval  TRUE 任务变了
* @retval  FALSE 有执行任务
 */
s32 dev_wireless_thread_additional_task_is_change(WIRE_TASK_ID task_id)
{
    if(g_wireless_thread_additional_node_param.m_task_id != WIRE_TASK_FREE
        && g_wireless_thread_additional_node_param.m_task_id != task_id)
    {
        if(WS_TASK_BT_SOCKET_RECVDATA == g_wireless_thread_additional_node_param.m_task_id)
        {
            return FALSE;
        }
        else
        {
            TRACE_ERR("task change to %d", g_wireless_thread_additional_node_param.m_task_id);
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
s32 dev_wireless_thread_additional_is_free(void)
{
    return g_wireless_thread_additional_node_param.m_task_id == WIRE_TASK_FREE;
}


