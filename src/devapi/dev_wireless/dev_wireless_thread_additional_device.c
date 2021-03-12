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
*  @brief    ���ļ���Ҫ�Ƕ�����ģ���Ϲ��ص���Χ�豸�̺߳������д���                                       *
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


static wireless_thread_node_param_t g_wireless_thread_additional_node_param;     //�����Ϲ��ص��豸������̲߳���

void dev_wireless_thread_addional_node_param_init(void)
{
    memset(&g_wireless_thread_additional_node_param, 0, sizeof(g_wireless_thread_additional_node_param));
}

/**
 * @brief �����������п���
 */
wireless_thread_node_param_t *dev_wireless_get_thread_additional_node_param(void)
{
    return &g_wireless_thread_additional_node_param;
}

/**
* @brief �����������----��Ҫ���ڷ���ĳ��ָ������ݽ��յ�ʱ���������socket���ݣ������ȴ���������ٽ��Ŵ���֮ǰ������
* @param[in] wifi_thread_node_param ֮ǰ������̲߳���
* @retval  ��
 */
void dev_wireless_set_thread_additional_node_param(wireless_thread_node_param_t *wireless_thread_node_param)
{
    memcpy(&g_wireless_thread_additional_node_param, wireless_thread_node_param, sizeof(wireless_thread_node_param_t));
}

/**
* @brief ������������ĳ�ʱʱ��
* @param[in] ms_timeout ����ʱʱ��
* @retval  ��
 */
void dev_wireless_thread_additional_set_timeout(u32 ms_timeout)
{
    g_wireless_thread_additional_node_param.m_timeover = (ms_timeout+WIRELESS_TIMER_BASE-1)/WIRELESS_TIMER_BASE;
}

/**
* @brief �����������
* @param[in] task_id ����id
* @param[in] hand_func ����ִ�к���
* @param[in] timeover ����ʱʱ��
* @retval  ��
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
* @brief ����������
* @param[in] result ������
* @retval  ��
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
* @brief �ж�����ID�Ƿ�����FREE�����TASKID
* @param[in] 
* @retval  TRUE �������
* @retval  FALSE ��ִ������
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
* @brief �����Ƿ�������ִ������
* @param[in] 
* @retval  TRUE �������
* @retval  FALSE ��ִ������
 */
s32 dev_wireless_thread_additional_is_free(void)
{
    return g_wireless_thread_additional_node_param.m_task_id == WIRE_TASK_FREE;
}


