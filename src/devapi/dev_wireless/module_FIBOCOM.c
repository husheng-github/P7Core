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
static wireless_module_t *g_current_module = NULL;
static u32 g_check_reg_try_time = 0;
static s32 g_wire_net_reg_success = 0;             //1CGREG指令注网成功       2:CEREG指令注网成功
#if FIBOCOM_G500_SUPPORT
static wireless_module_t g_G500;
#endif

/*****************************************************************************
*                                                                            *
*  @file     module_FIBOCOM.C                                                *
*  @brief    此文件主要是设计广和通模块的接口                                                *
*  Details.                                                                  *
*                                                                            *
*  @author   xiaohonghui                                                     *
*  Remark         : Description                                              *
*----------------------------------------------------------------------------*
*  Change History :                                                          *
*  <Date>     | <Version> | <Author>       | <Description>                   *
*----------------------------------------------------------------------------*
*  2020/04/11 | 1.0.0.1   | xiaohonghui    | Create file                     *
*----------------------------------------------------------------------------*
*                                                                            *
*****************************************************************************/
static s8 g_apn_buff[128] = {0};

static s32 common_callback(u8 result, u8 *src, u16 srclen)
{
    s32 ret = DDI_ERR;
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();

    TRACE_DBG("result:%d, src:%s", result, src);
    switch(result)
    {
        case WIRE_AT_RESULT_OTHER:
            if(WS_TASK_GETSET_AUDIOVOLUME == thread_node_param->m_task_id)
            {
                if(strstr(src,"CLVLOK"))
                {
                    dev_wireless_thread_set_result(WIRE_AT_RESULT_OK);
                }
                else if(strstr(src,"CLVLERROR"))
                {
                    dev_wireless_thread_set_result(WIRE_AT_RESULT_ERROR);
                }
            }
            break;
            
        case WIRE_AT_RESULT_OK:
        case WIRE_AT_RESULT_TIMEROUT:
        case WIRE_AT_RESULT_ERROR:
            dev_wireless_thread_set_result(result);
            break;
    }
    
    return ret;
}

static s32 common_respone_handle(WIRE_TASK_ID task_id)
{
    s32 ret = DDI_ERR;
    wireless_thread_node_param_t *thread_node_param = dev_wireless_get_thread_node_param();
    
    while(1)
    {
        dev_wireless_thread_run_switch(FALSE);
        if(dev_wireless_thread_task_is_change(task_id))
        {
            ret = DDI_EBUSY;
            break;
        }

        if(dev_wireless_thread_is_free())
        {
            switch(thread_node_param->m_result)
            {
                case WIRE_AT_RESULT_OK:
                    ret = DDI_OK;
                    break;

                case WIRE_AT_RESULT_TIMEROUT:
                    ret = DDI_ETIMEOUT;
                    break;
            }

            break;
        }

        dev_wireless_thread_run_switch(TRUE);
        ddi_misc_msleep(10);
    }

    dev_wireless_thread_run_switch(TRUE);
    return ret;
}

/**
 * @brief 修改套接字状态
 * @param[in] socket_id:套接字id
 * @param[in] socket_status:套接字状态
 * @retval 协议栈的status
 * @       0 Disconnected
 * @       1 Connected
 * @       2 Busy(disconnecting or connecting)
 * 数据有以下三种返回:
 * +MIPCALL: <local IP address> (2G) | (+MIPCALL: 10.20.83.175,2409:8954:3298:9ED0:95DA:7E09:A700:D58B 4G)
 * +MIPCALL: 0
 * +MIPCALL: <status>[,<Local IP address>]
 */
s32 G500_handle_ip_at_res_data(s8 *src, s32 src_len)
{
    s8 *kw = g_current_module->m_get_local_ip_res_kw;
    s8 *pstr = NULL;
    wireless_device_info_t *device_info = dev_wireless_get_device();
    s8 dst[2][SPLIT_PER_MAX];
    s32 tmp_val = 0;
    s32 ret = -1;

    pstr = strstr(src, kw);
    if(pstr)
    {
        pstr += strlen(kw);

        memset(dst, 0, sizeof(dst));
        tmp_val = dev_maths_split(dst, sizeof(dst)/sizeof(dst[0]), pstr, ",", SPLIT_NOIGNOR);

        if(tmp_val == sizeof(dst)/sizeof(dst[0]))   
        {
            dev_maths_str_trim(dst[0]);

            if(1 == strlen(dst[0]))     //+MIPCALL: <status>[,<Local IP address>]
            {
                dev_maths_str_trim(dst[1]);
                snprintf(device_info->m_local_ip, sizeof(device_info->m_local_ip), "%s", dst[1]);
                TRACE_INFO("ip addr:%s", device_info->m_local_ip);
                ret = dev_maths_asc_to_u32(dst[0], strlen(dst[0]), MODULUS_DEC);
            }
            else  //+MIPCALL: 10.20.83.175,2409:8954:3298:9ED0:95DA:7E09:A700:D58B 4G
            {
                snprintf(device_info->m_local_ip, sizeof(device_info->m_local_ip), "%s", dst[0]);
                TRACE_INFO("ip addr:%s", device_info->m_local_ip);
                ret = 1;
            }
        }
        else
        {
            // * +MIPCALL: <local IP address>
            // * +MIPCALL: 0
            if(1 == tmp_val)  
            {
                dev_maths_str_trim(dst[0]);
                if(strlen(dst[0]) > 1)
                {
                    snprintf(device_info->m_local_ip, sizeof(device_info->m_local_ip), "%s", dst[0]);
                    TRACE_INFO("ip addr:%s", device_info->m_local_ip);
                    ret = 1;
                }
                else
                {
                    ret = dev_maths_asc_to_u32(dst[0], strlen(dst[0]), MODULUS_DEC);
                    if(2 == ret)
                    {
                        TRACE_INFO("mip status is 2");
                    }
                }
            }
        }
    }

    return ret;
}

static s32 G500_open_airplane_callback(u8 result, u8 *src, u16 srclen)
{
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();
    s8 *pstr = NULL;
    s32 val = 0;
        
    switch(thread_node_param->m_step)
    {
        case WIRE_OA_STEP_OPEN_AIRPLANE:
            if(WIRE_AT_RESULT_OK == result)
            {
                dev_wireless_send_at_cmd("AT+CFUN?", AT_API_BLOCK, TRUE);
                dev_wireless_thread_set_timeout(WIRE_AT_DEFAULT_TIMEOUT);
                thread_node_param->m_step = WIRE_OA_STEP_CHECK_RESULT;
            }
            else if(WIRE_AT_RESULT_TIMEROUT == result || WIRE_AT_RESULT_ERROR == result)
            {
                dev_wireless_thread_set_result(result);
            }
            break;

        case WIRE_OA_STEP_CHECK_RESULT:
            if(WIRE_AT_RESULT_OK == result || WIRE_AT_RESULT_TIMEROUT == result)
            {
                val = thread_node_param->m_lparam;
                dev_wireless_thread_set_result(result);
                thread_node_param->m_lparam = val;
            }
            else if(WIRE_AT_RESULT_OTHER == result)
            {
                pstr = strstr(src, "+CFUN:");
                if(pstr)
                {
                    pstr += strlen("+CFUN:");
                    dev_maths_str_trim(pstr);
                    thread_node_param->m_lparam = dev_maths_asc_to_u32(pstr, strlen(pstr), MODULUS_DEC);
                }
            }
            break;
    }

    return 0;
}

static s32 G500_clear_rplmn_callback(u8 result, u8 *src, u16 srclen)
{
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();

    if(WS_TASK_CLEAR_RPLMN == thread_node_param->m_task_id)
    {
        if(WIRE_AT_RESULT_OK == result)
        {
            dev_wireless_thread_set_result(result);
        }
        else if(WIRE_AT_RESULT_TIMEROUT == result || WIRE_AT_RESULT_ERROR == result)
        {
            dev_wireless_thread_set_result(result);
        }
    }

    return 0;
}

/**
 * @brief 清除RPLMN参数，清除后第一次sim卡当做新卡识别，第一次通讯会慢些
 * @param[in] 无
 * @retval DDI_OK 成功
 * @retval DDI_ERR 
 * @retval DDI_ETIMEOUT
 */
s32 G500_clear_rplmn(AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;
    static int flag = 0;
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();

    if(FIBOCOM_L610 == g_current_module->m_id)
    {
        return DDI_OK;
    }
    
    switch(at_api_get_type)
    {
        case AT_API_NO_BLOCK:
            dev_wireless_send_at_cmd(g_current_module->m_clear_rplmn_cmd, at_api_get_type, TRUE);
            if(ret >= 0)
            {
                dev_wireless_thread_set_timeout(WIRE_AT_DEFAULT_TIMEOUT);
                ret = DDI_OK;
            }
            break;

        case AT_API_BLOCK:
            if(0 == flag)
            {
                dev_wireless_send_at_cmd(g_current_module->m_clear_rplmn_cmd, at_api_get_type, TRUE);
                dev_wireless_thread_set_param(WS_TASK_CLEAR_RPLMN, G500_clear_rplmn_callback, WIRE_AT_DEFAULT_TIMEOUT);
            }
            break;
    }

    if(at_api_get_type == AT_API_BLOCK && 0 == flag)
    {
        while(1)
        {
            ddi_watchdog_feed();
            dev_wireless_thread_run_switch(FALSE);
            if(dev_wireless_thread_task_is_change(WS_TASK_CLEAR_RPLMN))
            {
                ret = DDI_EBUSY;
                break;
            }
            
            if(dev_wireless_thread_is_free())
            {
                switch(thread_node_param->m_result)
                {
                    case WIRE_AT_RESULT_OK:
                        flag = 1;
                        ret = DDI_OK;
                        break;

                    case WIRE_AT_RESULT_ERROR:
                        ret = DDI_ERR;
                        break;
                        
                    case WIRE_AT_RESULT_TIMEROUT:
                        ret = DDI_ETIMEOUT;
                        break;
                }

                break;
            }

            dev_wireless_thread_run_switch(TRUE);
            ddi_misc_msleep(50);
        }
        dev_wireless_thread_run_switch(TRUE);
    }

    return ret;
}

static s32 G500_set_8960_info_data_callback(u8 result, u8 *src, u16 srclen)
{
    if(WIRE_AT_RESULT_OK == result)
    {
        dev_wireless_thread_set_result(result);
    }
    else if(WIRE_AT_RESULT_TIMEROUT == result || WIRE_AT_RESULT_ERROR == result)
    {
        dev_wireless_thread_set_result(result);
    }

    return 0;
}

/**
 * @brief 写入中测仪的仪器的参数00101写进RPLMN
 * @param[in] 无
 * @retval DDI_OK 成功
 * @retval DDI_ERR 
 * @retval DDI_ETIMEOUT
 */
s32 G500_set_8960_info_data(void)
{
    s32 ret = DDI_ERR;
    static int flag = 0;
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();

    if(FIBOCOM_L610 == g_current_module->m_id)
    {
        return DDI_OK;
    }
    
    if(0 == flag)
    {
        dev_wireless_send_at_cmd("AT+CRSM=214,28542,0,4,3,\"00F110\"", AT_API_BLOCK, TRUE);
        dev_wireless_thread_set_param(WS_TASK_WRITE_8960INFO_TO_RPLMN, G500_set_8960_info_data_callback, WIRE_AT_DEFAULT_TIMEOUT);

        while(1)
        {
            ddi_watchdog_feed();
            dev_wireless_thread_run_switch(FALSE);
            if(dev_wireless_thread_task_is_change(WS_TASK_WRITE_8960INFO_TO_RPLMN))
            {
                ret = DDI_EBUSY;
                break;
            }
            
            if(dev_wireless_thread_is_free())
            {
                switch(thread_node_param->m_result)
                {
                    case WIRE_AT_RESULT_OK:
                        flag = 1;
                        ret = DDI_OK;
                        break;

                    case WIRE_AT_RESULT_ERROR:
                        ret = DDI_ERR;
                        break;
                        
                    case WIRE_AT_RESULT_TIMEROUT:
                        ret = DDI_ETIMEOUT;
                        break;
                }

                break;
            }

            dev_wireless_thread_run_switch(TRUE);
            ddi_misc_msleep(50);
        }

        dev_wireless_thread_run_switch(TRUE);
    }

    return 0;
}

s32 G500_open_airplane(AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();

    switch(at_api_get_type)
    {
        case AT_API_NO_BLOCK:
            break;

        case AT_API_BLOCK:
            G500_set_8960_info_data();

            //4G用8960测试，需要把制式设置成2G
            if(DDI_OK != L610_set_2G_mode())
            {
                ret = DDI_EOPERATION;
                break;
            }
            
            dev_wireless_send_at_cmd("AT+CFUN=4", at_api_get_type, TRUE);
            dev_wireless_thread_set_param(WS_TASK_OPEN_AIRPLANE, G500_open_airplane_callback, WIRE_AT_DEFAULT_TIMEOUT*3);
            while(1)
            {
                ddi_watchdog_feed();
                dev_wireless_thread_run_switch(FALSE);
                if(dev_wireless_thread_task_is_change(WS_TASK_OPEN_AIRPLANE))
                {
                    ret = DDI_EBUSY;
                    break;
                }
                
                if(dev_wireless_thread_is_free())
                {
                    switch(thread_node_param->m_result)
                    {
                        case WIRE_AT_RESULT_OK:
                            if(4 != thread_node_param->m_lparam)
                            {
                                TRACE_ERR("open airplane fail:%d", thread_node_param->m_lparam);
                                break;
                            }
                            ret = DDI_OK;
                            break;

                        case WIRE_AT_RESULT_ERROR:
                            ret = DDI_ERR;
                            break;
                            
                        case WIRE_AT_RESULT_TIMEROUT:
                            ret = DDI_ETIMEOUT;
                            break;
                    }

                    break;
                }

                dev_wireless_thread_run_switch(TRUE);
                ddi_misc_msleep(50);
            }
            dev_wireless_thread_run_switch(TRUE);
            break;
    }

    return ret;
}

static s32 G500_close_airplane_callback(u8 result, u8 *src, u16 srclen)
{
    if(WIRE_AT_RESULT_OK == result || WIRE_AT_RESULT_TIMEROUT == result)
    {
        dev_wireless_thread_set_result(result);
    }

    return 0;
}

static s32 L610_set_work_mode_callback(u8 result, u8 *src, u16 srclen)
{
    TRACE_DBG("result:%d, src:%s", result, src);
    
    if(WIRE_AT_RESULT_OK == result || WIRE_AT_RESULT_TIMEROUT == result)
    {
        dev_wireless_thread_set_result(result);
    }

    return 0;
}

static s32 L610_set_work_mode(WIRE_CUR_NET_MODE_E net_work_type)
{
    s32 ret = DDI_ERR;
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();

    if(WIRE_NET_2G == net_work_type)
    {
        dev_wireless_send_at_cmd("AT+GTRAT=0", AT_API_BLOCK, TRUE);
    }
    else if(WIRE_NET_4G == net_work_type)
    {
        dev_wireless_send_at_cmd("AT+GTRAT=10,3,0", AT_API_BLOCK, TRUE);
    }
    
    dev_wireless_thread_set_param(WS_TASK_SET_WORK_MODE, L610_set_work_mode_callback, WIRE_AT_DEFAULT_TIMEOUT);
    while(1)
    {
        ddi_watchdog_feed();
        dev_wireless_thread_run_switch(FALSE);
        if(dev_wireless_thread_task_is_change(WS_TASK_SET_WORK_MODE))
        {
            ret = DDI_EBUSY;
            break;
        }
        
        if(dev_wireless_thread_is_free())
        {
            switch(thread_node_param->m_result)
            {
                case WIRE_AT_RESULT_OK:
                    ret = DDI_OK;
                    break;
                    
                case WIRE_AT_RESULT_TIMEROUT:
                    ret = DDI_ETIMEOUT;
                    break;
            }

            break;
        }

        dev_wireless_thread_run_switch(TRUE);
        ddi_misc_msleep(50);
    }

    dev_wireless_thread_run_switch(TRUE);
    return ret;
}


static s32 L610_get_work_mode_callback(u8 result, u8 *src, u16 srclen)
{
    s8 *pstr = NULL;
    s8 dst[3][SPLIT_PER_MAX];
    s32 ret = DDI_ERR;
    s32 i = 0;
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();

    TRACE_DBG("result:%d, src:%s", result, src);
    if(WIRE_AT_RESULT_OK == result)
    {
        if(TRUE == thread_node_param->m_wparam)
        {
            ret = thread_node_param->m_lparam;
            if(thread_node_param->m_task_id != WS_TASK_MODULES_INIT)
            {
                dev_wireless_thread_set_result(result);
            }
            
            thread_node_param->m_lparam = ret;
            if(3 == thread_node_param->m_lparam)   //lte优先
            {
                ret = DDI_OK;
            }
            else
            {
                ret = DDI_ERR;
            }
        }
    }
    else if(WIRE_AT_RESULT_TIMEROUT == result)
    {
        if(thread_node_param->m_task_id != WS_TASK_MODULES_INIT)
        {
            dev_wireless_thread_set_result(result);
        }
        ret = DDI_ETIMEOUT;
    }
    else if(WIRE_AT_RESULT_OTHER == result)
    {
        pstr = strstr(src, "+GTRAT:");
        if(pstr)
        {
            pstr += strlen("+GTRAT:");
            memset(dst, 0, sizeof(dst));
            ret = dev_maths_split(dst, sizeof(dst)/sizeof(dst[0]), pstr, ",", SPLIT_NOIGNOR);
            TRACE_DBG("ret:%d", ret);
            if(ret >= sizeof(dst)/sizeof(dst[0])-1)
            {
                for(i=0; i<ret; i++)
                {
                    dev_maths_str_trim(dst[i]);
                }

                thread_node_param->m_lparam = dev_maths_asc_to_u32(dst[1], strlen(dst[1]), MODULUS_DEC);
                thread_node_param->m_wparam = TRUE;
            }
        }
        ret = DDI_ERR;
    }

    return ret;
}

static s32 L610_get_work_mode(void)
{
    s32 ret = DDI_ERR;
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();

    dev_wireless_send_at_cmd("AT+GTRAT?", AT_API_BLOCK, TRUE);
    dev_wireless_thread_set_param(WS_TASK_GET_WORK_MODE, L610_get_work_mode_callback, WIRE_AT_DEFAULT_TIMEOUT);
    while(1)
    {
        ddi_watchdog_feed();
        dev_wireless_thread_run_switch(FALSE);
        if(dev_wireless_thread_task_is_change(WS_TASK_GET_WORK_MODE))
        {
            ret = DDI_EBUSY;
            break;
        }
        
        if(dev_wireless_thread_is_free())
        {
            switch(thread_node_param->m_result)
            {
                case WIRE_AT_RESULT_OK:
                    ret = thread_node_param->m_lparam;
                    break;
                    
                case WIRE_AT_RESULT_TIMEROUT:
                    ret = DDI_ETIMEOUT;
                    break;
            }

            break;
        }

        dev_wireless_thread_run_switch(TRUE);
        ddi_misc_msleep(50);
    }

    dev_wireless_thread_run_switch(TRUE);
    return ret;
}

/**
 * @brief 设置为仅2G模式
 * @param[in] 无
 * @retval  DDI_OK  设置成功
 * @retval  <0  设置失败
 */
static s32 g_gsm_flag = FALSE;
static s32 L610_set_2G_mode(void)
{
    s32 ret = DDI_ERR;

    do{
        if(FIBOCOM_L610 != g_current_module->m_id)
        {
            ret = DDI_OK;
            break;
        }

        if(!g_gsm_flag)
        {
            //判断当前是仅2G
            ret = L610_get_work_mode();
            TRACE_INFO("ret:%d", ret);
            if(0 != ret)
            {
                ret = L610_set_work_mode(WIRE_NET_2G);
            }

            if(ret == DDI_OK)
            {
                g_gsm_flag = TRUE;
            }
        }
        ret = DDI_OK;
    }while(0);

    return ret;
}

/**
 * @brief 设置为制式自动识别
 * @param[in] 无
 * @retval  DDI_OK  设置成功
 * @retval  <0  设置失败
 */
static s32 L610_set_auto_mode(void)
{
    s32 ret = DDI_ERR;

    do{
        if(FIBOCOM_L610 != g_current_module->m_id)
        {
            ret = DDI_OK;
            break;
        }
        
        ret = L610_set_work_mode(WIRE_NET_4G);
        g_gsm_flag = FALSE;
    }while(0);

    return ret;
}


s32 G500_close_airplane(AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();

    switch(at_api_get_type)
    {
        case AT_API_NO_BLOCK:
            break;

        case AT_API_BLOCK:
            dev_wireless_send_at_cmd("AT+CFUN=1", at_api_get_type, TRUE);
            dev_wireless_thread_set_param(WS_TASK_CLOSE_AIRPLANE, G500_close_airplane_callback, WIRE_AT_DEFAULT_TIMEOUT);
            while(1)
            {
                ddi_watchdog_feed();
                dev_wireless_thread_run_switch(FALSE);
                if(dev_wireless_thread_task_is_change(WS_TASK_CLOSE_AIRPLANE))
                {
                    ret = DDI_EBUSY;
                    break;
                }
                
                if(dev_wireless_thread_is_free())
                {
                    switch(thread_node_param->m_result)
                    {
                        case WIRE_AT_RESULT_OK:
                            ret = DDI_OK;
                            break;
                            
                        case WIRE_AT_RESULT_TIMEROUT:
                            ret = DDI_ETIMEOUT;
                            break;
                    }

                    break;
                }

                dev_wireless_thread_run_switch(TRUE);
                ddi_misc_msleep(50);
            }
            dev_wireless_thread_run_switch(TRUE);
            break;
    }

    return ret;
}

static s32 G500_telephone_dial_callback(u8 result, u8 *src, u16 srclen)
{
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();
    
    if(WIRE_AT_RESULT_OK == result)
    {
        thread_node_param->m_lparam = 1;
        thread_node_param->m_wparam = result;
    }
    else if(WIRE_AT_RESULT_OTHER == result)
    {
        if(strstr(src, "NO ANSWER"))
        {
            TRACE_ERR("dail no answer");
            dev_wireless_thread_set_result(WIRE_AT_RESULT_DAIL_NOANSWER);
        }
    }
    else if(WIRE_AT_RESULT_TIMEROUT == result)
    {
        TRACE_ERR("dail no answer:%d, result:%d", thread_node_param->m_lparam, thread_node_param->m_result);
        if(1 == thread_node_param->m_lparam)
        {
            dev_wireless_thread_set_result(thread_node_param->m_wparam);
        }
        else
        {
            dev_wireless_thread_set_result(WIRE_AT_RESULT_TIMEROUT);
        }
    }

    return 0;
}

s32 G500_telephone_dial(u8 *num, AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;
    s8 buff[64] = {0};
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();

    switch(at_api_get_type)
    {
        case AT_API_NO_BLOCK:
            break;

        case AT_API_BLOCK:
            if(!G500_get_net_cs_reg(AT_API_BLOCK))   //检查是否注网成功
            {
                ret = DDI_EOPERATION;
                break;
            }

            dev_user_delay_ms(1000);
            snprintf(buff, sizeof(buff), "ATD%s;", num);
            dev_wireless_send_at_cmd(buff, at_api_get_type, TRUE);
            dev_wireless_thread_set_param(WS_TASK_TELEPHONE_DAIL, G500_telephone_dial_callback, WIRE_AT_DEFAULT_TIMEOUT*10);
            while(1)
            {
                ddi_watchdog_feed();
                if(dev_wireless_thread_task_is_change(WS_TASK_TELEPHONE_DAIL))
                {
                    ret = DDI_EBUSY;
                    break;
                }
                
                if(dev_wireless_thread_is_free())
                {
                    switch(thread_node_param->m_result)
                    {
                        case WIRE_AT_RESULT_OK:
                            ret = DDI_OK;
                            break;

                        case WIRE_AT_RESULT_ERROR:
                        case WIRE_AT_RESULT_DAIL_NOANSWER:
                            ret = DDI_ERR;
                            break;

                        case WIRE_AT_RESULT_TIMEROUT:
                            ret = DDI_ETIMEOUT;
                            break;
                    }
                    break;
                }

                ddi_misc_msleep(50);
            }
            break;
    }

    return ret;
}

static s32 G500_telephone_hangup_callback(u8 result, u8 *src, u16 srclen)
{
    if(WIRE_AT_RESULT_OK == result || WIRE_AT_RESULT_TIMEROUT == result || WIRE_AT_RESULT_ERROR == result)
    {
        dev_wireless_thread_set_result(result);
    }

    return 0;
}

s32 G500_telephone_hangup(AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();

    switch(at_api_get_type)
    {
        case AT_API_NO_BLOCK:
            break;

        case AT_API_BLOCK:
            dev_wireless_send_at_cmd("ATH", at_api_get_type, TRUE);
            dev_wireless_thread_set_param(WS_TASK_TELEPHONE_HANGUP, G500_telephone_hangup_callback, WIRE_AT_DEFAULT_TIMEOUT*3);
            while(1)
            {
                ddi_watchdog_feed();
                if(dev_wireless_thread_task_is_change(WS_TASK_TELEPHONE_HANGUP))
                {
                    ret = DDI_EBUSY;
                    break;
                }
                
                if(dev_wireless_thread_is_free())
                {
                    switch(thread_node_param->m_result)
                    {
                        case WIRE_AT_RESULT_OK:
                            ret = DDI_OK;
                            break;

                        case WIRE_AT_RESULT_ERROR:
                            ret = DDI_ERR;
                            break;

                        case WIRE_AT_RESULT_TIMEROUT:
                            ret = DDI_ETIMEOUT;
                            break;
                    }
                    break;
                }

                ddi_misc_msleep(50);
            }
            break;
    }

    G500_clear_rplmn(AT_API_BLOCK);
    L610_set_auto_mode();
    return ret;
}

static s32 G500_cfg_ntp_server_callback(u8 result, u8 *src, u16 srclen)
{
    s8 *pstr = NULL;
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();

    TRACE_DBG("result:%d,%s", result, src);
    if(WIRE_AT_RESULT_TIMEROUT == result)
    {
        dev_wireless_thread_set_result(result);
    }
    else if(WIRE_AT_RESULT_OK == result)
    {
        thread_node_param->m_lparam = 1;
    }
    else if(WIRE_AT_RESULT_OTHER == result)
    {
        if(1 == thread_node_param->m_lparam)
        {
            pstr = strstr(src, "+MIPNTP:");
            if(pstr)
            {
                pstr += strlen("+MIPNTP:");
                dev_maths_str_trim(pstr);
                dev_wireless_thread_set_result(WIRE_AT_RESULT_OK);
                thread_node_param->m_wparam = dev_maths_asc_to_u32(pstr, strlen(pstr), MODULUS_DEC);
            }
        }
    }
}

s32 G500_cfg_ntp_server(s8 *ntp_addr, s32 ntp_port, AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;
    s8 tmp_buff[128] = {0};
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();
    s8 ipaddr[32] = {0};

    switch(at_api_get_type)
    {
        case AT_API_NO_BLOCK:
            break;

        case AT_API_BLOCK:
            memset(ipaddr, 0, sizeof(ipaddr));
            ret = G500_gethostbyname(ntp_addr, ipaddr, AT_API_BLOCK);
            if(DDI_OK != ret)
            {
                TRACE_ERR("get ip fail:%s,%d", ntp_addr, ret);
                ret = DDI_ERR;
                break;
            }
            
            memset(tmp_buff, 0, sizeof(tmp_buff));
            snprintf(tmp_buff, sizeof(tmp_buff), "AT+MIPNTP=\"%s\",%d", ipaddr, ntp_port);
            dev_wireless_send_at_cmd(tmp_buff, at_api_get_type, TRUE);
            dev_wireless_thread_set_param(WS_TASK_CFG_NTP_SERVER, G500_cfg_ntp_server_callback, WIRE_AT_DEFAULT_TIMEOUT*3);
            while(1)
            {
                ddi_watchdog_feed();
                dev_wireless_thread_run_switch(FALSE);
                if(dev_wireless_thread_task_is_change(WS_TASK_CFG_NTP_SERVER))
                {
                    ret = DDI_EBUSY;
                    break;
                }
                
                if(dev_wireless_thread_is_free())
                {
                    switch(thread_node_param->m_result)
                    {
                        case WIRE_AT_RESULT_OK:
                            if(1 == thread_node_param->m_wparam)
                            {
                                ret = DDI_OK;
                            }
                            break;

                        case WIRE_AT_RESULT_TIMEROUT:
                            ret = DDI_ETIMEOUT;
                            break;
                    }
                    break;
                }

                dev_wireless_thread_run_switch(TRUE);
                ddi_misc_msleep(50);
            }
            dev_wireless_thread_run_switch(TRUE);
            break;
    }

    return ret;
}

static s32 G500_qry_rtc_callback(u8 result, u8 *src, u16 srclen)
{
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();
    wireless_device_info_t *device_info = dev_wireless_get_device();
    s8 dst[3][SPLIT_PER_MAX] = {0};
    s8 *pstr = NULL;
    s32 ret = 0;
    
    if(WIRE_AT_RESULT_OK == result || WIRE_AT_RESULT_TIMEROUT == result)
    {
        dev_wireless_thread_set_result(result);

        if(WIRE_AT_RESULT_TIMEROUT == result)
        {
            dev_wireless_set_control_strategy_step(WS_CHECK_CMD);
        }
    }
    else if(WIRE_AT_RESULT_OTHER == result)
    {
        memset(dst, 0, sizeof(dst));
        ret = dev_maths_split(dst, sizeof(dst)/sizeof(dst[0]), src, "\"", SPLIT_NOIGNOR);
        pstr = strstr(src, "+CCLK");
        if(pstr)
        {
            TRACE_DBG("ret:%d, %s", ret, src);
            snprintf(device_info->m_rtc, sizeof(device_info->m_rtc), "%s", dst[1]);
        }
    }
}

s32 G500_qry_rtc(s8 *rtc_buff, s32 rtc_buff_len, AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();
    wireless_device_info_t *device_info = dev_wireless_get_device();

    switch(at_api_get_type)
    {
        case AT_API_NO_BLOCK:
            break;

        case AT_API_BLOCK:
            dev_wireless_send_at_cmd("AT+CCLK?", at_api_get_type, TRUE);
            dev_wireless_thread_set_param(WS_TASK_QRY_RTC, G500_qry_rtc_callback, WIRE_AT_DEFAULT_TIMEOUT);
            while(1)
            {
                ddi_watchdog_feed();
                dev_wireless_thread_run_switch(FALSE);
                if(dev_wireless_thread_task_is_change(WS_TASK_QRY_RTC))
                {
                    ret = DDI_EBUSY;
                    break;
                }
                
                if(dev_wireless_thread_is_free())
                {
                    switch(thread_node_param->m_result)
                    {
                        case WIRE_AT_RESULT_OK:
                            snprintf(rtc_buff, rtc_buff_len, "%s", device_info->m_rtc);
                            ret = DDI_OK;
                            break;

                        case WIRE_AT_RESULT_TIMEROUT:
                            ret = DDI_ETIMEOUT;
                            break;
                    }
                    break;
                }

                dev_wireless_thread_run_switch(TRUE);
                ddi_misc_msleep(50);
            }
            dev_wireless_thread_run_switch(TRUE);
            break;
    }

    return ret;
}

static s32 G500_get_cellinfo_callback(u8 result, u8 *src, u16 srclen)
{
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();
    wireless_device_info_t *device_info = dev_wireless_get_device();
    s8 dst[23][SPLIT_PER_MAX];
    s32 ret = 0;
    s32 i = 0;
    u16 value = 0;
    s32 cur_cell_no = 0;
    u32 lac = 0;
    u32 cid = 0;

    TRACE_DBG("step:%d, result:%d, src:%s", thread_node_param->m_step, result, src);
    switch(thread_node_param->m_step)
    {
        case WIRE_GET_CELL_STEP_MAIN_HEAD:   //主基站头
            if(WIRE_AT_RESULT_OTHER == result)
            {
                if(0 == strcmp(src, "GSM service cell:"))
                {
                    thread_node_param->m_lparam = WIRE_NET_2G;
                    thread_node_param->m_step = WIRE_GET_CELL_STEP_MAIN_INFO;
                    break;
                }
                else if(0 == strcmp(src, "LTE service cell:"))
                {
                    thread_node_param->m_lparam = WIRE_NET_4G;
                    thread_node_param->m_step = WIRE_GET_CELL_STEP_MAIN_INFO;
                    TRACE_DBG("ready to get lte main cell content");
                    break;
                }
                else if(0 == strcmp(src, "GSM neighbor cell:"))  //不插sim卡时只有周边基站信息
                {
                    thread_node_param->m_lparam = WIRE_NET_2G;
                    thread_node_param->m_step = WIRE_GET_CELL_STEP_NEAR_INFO;
                    TRACE_DBG("ready to get near cell content");
                    break;
                }
                else if(0 == strcmp(src, "LTE neighbor cell:"))  //不插sim卡时只有周边基站信息
                {
                    thread_node_param->m_lparam = WIRE_NET_4G;
                    thread_node_param->m_step = WIRE_GET_CELL_STEP_NEAR_INFO;
                    TRACE_DBG("ready to get near cell content");
                    break;
                }
            }
            else if(WIRE_AT_RESULT_TIMEROUT == result || WIRE_AT_RESULT_ERROR == result)
            {
                TRACE_DBG("get cell info timeout");
                dev_wireless_thread_set_result(result);

                if(WIRE_AT_RESULT_TIMEROUT == result)
                {
                    dev_wireless_set_control_strategy_step(WS_CHECK_CMD);
                }
            }
            break;

        case WIRE_GET_CELL_STEP_MAIN_INFO:   //主基站内容
            if(WIRE_AT_RESULT_OTHER == result)
            {
                memset(dst, 0, sizeof(dst));
                ret = dev_maths_split(dst, sizeof(dst)/sizeof(dst[0]), src, ",", SPLIT_IGNOR_LEN0);
                if(ret > 0)
                {
                    for(i=0; i<ret; i++)
                    {
                        dev_maths_str_trim(dst[i]);
                    }

                    device_info->m_cell_num = 0;
                    cur_cell_no = device_info->m_cell_num;
                    if(WIRE_NET_2G == thread_node_param->m_lparam)
                    {
                        device_info->m_cell_info[cur_cell_no]->m_mcc = dev_maths_asc_to_u32(dst[2], strlen(dst[2]), MODULUS_DEC);
                        device_info->m_cell_info[cur_cell_no]->m_mnc = dev_maths_asc_to_u32(dst[3], strlen(dst[3]), MODULUS_DEC);
                        device_info->m_cell_info[cur_cell_no]->m_lac = dev_maths_asc_to_u32(dst[4], strlen(dst[4]), MODULUS_HEX);
                        device_info->m_cell_info[cur_cell_no]->m_cid = dev_maths_asc_to_u32(dst[5], strlen(dst[5]), MODULUS_HEX);
                        device_info->m_cell_info[cur_cell_no]->m_arfcn = dev_maths_asc_to_u32(dst[6], strlen(dst[6]), MODULUS_DEC);
                        device_info->m_cell_info[cur_cell_no]->m_rxlen = dev_maths_asc_to_u32(dst[21], strlen(dst[21]), MODULUS_DEC);
                        device_info->m_cell_info[cur_cell_no]->m_rssi = dev_maths_asc_to_u32(dst[22], strlen(dst[22]), MODULUS_DEC);
                        thread_node_param->m_step = WIRE_GET_CELL_STEP_NEAR_HEAD;
                        device_info->m_cell_num++;
                    }
                    else if(WIRE_NET_4G == thread_node_param->m_lparam)
                    {
                        device_info->m_cell_info[cur_cell_no]->m_mcc = dev_maths_asc_to_u32(dst[2], strlen(dst[2]), MODULUS_DEC);
                        device_info->m_cell_info[cur_cell_no]->m_mnc = dev_maths_asc_to_u32(dst[3], strlen(dst[3]), MODULUS_DEC);
                        device_info->m_cell_info[cur_cell_no]->m_lac = dev_maths_asc_to_u32(dst[4], strlen(dst[4]), MODULUS_HEX);
                        device_info->m_cell_info[cur_cell_no]->m_cid = dev_maths_asc_to_u32(dst[5], strlen(dst[5]), MODULUS_HEX);
                        device_info->m_cell_info[cur_cell_no]->m_arfcn = dev_maths_asc_to_u32(dst[6], strlen(dst[6]), MODULUS_DEC);
                        device_info->m_cell_info[cur_cell_no]->m_rxlen = dev_maths_asc_to_u32(dst[13], strlen(dst[21]), MODULUS_DEC);
                        device_info->m_cell_info[cur_cell_no]->m_rssi = dev_maths_asc_to_u32(dst[11], strlen(dst[22]), MODULUS_DEC);
                        thread_node_param->m_step = WIRE_GET_CELL_STEP_NEAR_HEAD;
                        device_info->m_cell_num++;
                    }
                }
            }
            else if(WIRE_AT_RESULT_TIMEROUT == result)
            {
                TRACE_DBG("get cell info timeout");
                dev_wireless_thread_set_result(result);
            }
            break;

        case WIRE_GET_CELL_STEP_NEAR_HEAD:    //周边基站头
            if(WIRE_AT_RESULT_OTHER == result)
            {
                if(0 == strcmp(src, "GSM neighbor cell:") || 0 == strcmp(src, "LTE neighbor cell:"))
                {
                    thread_node_param->m_step = WIRE_GET_CELL_STEP_NEAR_INFO;
                    TRACE_DBG("ready to get near cell content");
                    break;
                }
            }
            else if(WIRE_AT_RESULT_TIMEROUT == result)
            {
                TRACE_DBG("get cell info timeout");
                dev_wireless_thread_set_result(result);
            }
            else if(WIRE_AT_RESULT_OK == result)
            {
                TRACE_DBG("get cell info success:%d", device_info->m_cell_num);
                dev_wireless_thread_set_result(result);
            }
            break;

        case WIRE_GET_CELL_STEP_NEAR_INFO:   //周边基站内容
            if(WIRE_AT_RESULT_OTHER == result)
            {
                ret = dev_maths_split(dst, sizeof(dst)/sizeof(dst[0]), src, ",", SPLIT_IGNOR_LEN0);
                if(ret > 0)
                {
                    for(i=0; i<ret; i++)
                    {
                        dev_maths_str_trim(dst[i]);
                    }

                    cur_cell_no = device_info->m_cell_num;
                    lac = dev_maths_asc_to_u32(dst[4], strlen(dst[4]), MODULUS_HEX);
                    cid = dev_maths_asc_to_u32(dst[5], strlen(dst[5]), MODULUS_HEX);
                    if(lac ==0 || lac>0xffff || cid > 0xffff)
                    {
                        TRACE_INFO("lac or cid is invalid, ignore this cellinfo:%s", src);
                        thread_node_param->m_step = WIRE_GET_CELL_STEP_NEAR_HEAD;
                    }
                    else
                    {
                        if(WIRE_NET_2G == thread_node_param->m_lparam)
                        {
                            device_info->m_cell_info[cur_cell_no]->m_mcc = dev_maths_asc_to_u32(dst[2], strlen(dst[2]), MODULUS_DEC);
                            device_info->m_cell_info[cur_cell_no]->m_mnc = dev_maths_asc_to_u32(dst[3], strlen(dst[3]), MODULUS_DEC);
                            device_info->m_cell_info[cur_cell_no]->m_lac = dev_maths_asc_to_u32(dst[4], strlen(dst[4]), MODULUS_HEX);
                            device_info->m_cell_info[cur_cell_no]->m_cid = dev_maths_asc_to_u32(dst[5], strlen(dst[5]), MODULUS_HEX);
                            device_info->m_cell_info[cur_cell_no]->m_arfcn = dev_maths_asc_to_u32(dst[6], strlen(dst[6]), MODULUS_DEC);
                            device_info->m_cell_info[cur_cell_no]->m_rxlen = dev_maths_asc_to_u32(dst[12], strlen(dst[12]), MODULUS_DEC);
                            device_info->m_cell_info[cur_cell_no]->m_rssi = dev_maths_asc_to_u32(dst[13], strlen(dst[13]), MODULUS_DEC);
                            thread_node_param->m_step = WIRE_GET_CELL_STEP_NEAR_HEAD;
                            device_info->m_cell_num++;
                        }
                        else if(WIRE_NET_4G == thread_node_param->m_lparam)
                        {
                            device_info->m_cell_info[cur_cell_no]->m_mcc = dev_maths_asc_to_u32(dst[2], strlen(dst[2]), MODULUS_DEC);
                            device_info->m_cell_info[cur_cell_no]->m_mnc = dev_maths_asc_to_u32(dst[3], strlen(dst[3]), MODULUS_DEC);
                            device_info->m_cell_info[cur_cell_no]->m_lac = dev_maths_asc_to_u32(dst[4], strlen(dst[4]), MODULUS_HEX);
                            device_info->m_cell_info[cur_cell_no]->m_cid = dev_maths_asc_to_u32(dst[5], strlen(dst[5]), MODULUS_HEX);
                            device_info->m_cell_info[cur_cell_no]->m_arfcn = dev_maths_asc_to_u32(dst[6], strlen(dst[6]), MODULUS_DEC);
                            device_info->m_cell_info[cur_cell_no]->m_rxlen = dev_maths_asc_to_u32(dst[11], strlen(dst[12]), MODULUS_DEC);
                            device_info->m_cell_info[cur_cell_no]->m_rssi = dev_maths_asc_to_u32(dst[9], strlen(dst[13]), MODULUS_DEC);
                            thread_node_param->m_step = WIRE_GET_CELL_STEP_NEAR_HEAD;
                            device_info->m_cell_num++;
                        }
                    }
                }
            }
            else if(WIRE_AT_RESULT_TIMEROUT == result )
            {
                TRACE_DBG("get cell info timeout");
                dev_wireless_thread_set_result(result);
            }
            else if(WIRE_AT_RESULT_OK == result)
            {
                TRACE_DBG("get cell info success:%d", device_info->m_cell_num);
                dev_wireless_thread_set_result(result);
            }
            break;
    }

    return 0;
}

s32 G500_get_cellinfo(strBasestationInfo *basestation_info, s32 cell_num, AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();
    wireless_device_info_t *device_info = dev_wireless_get_device();
    
    if(g_current_module->m_id == FIBOCOM_L610)
    {
        if(device_info->m_cell_num > 0)
        {
            return DDI_OK;
        }
    }
    
    switch(at_api_get_type)
    {
        case AT_API_NO_BLOCK:
            break;

        case AT_API_BLOCK:
            dev_wireless_send_at_cmd(g_current_module->m_get_cellinfo_cmd, at_api_get_type, TRUE);
            if(FIBOCOM_L610 == g_current_module->m_id)
            {
                dev_wireless_thread_set_param(WS_TASK_GET_CELL_INFO, G500_get_cellinfo_callback, WIRE_AT_CMD_RESEND_TIMEOUT*10);
            }
            else
            {
                dev_wireless_thread_set_param(WS_TASK_GET_CELL_INFO, G500_get_cellinfo_callback, WIRE_AT_CMD_RESEND_TIMEOUT*3);
            }
            while(1)
            {
                ddi_watchdog_feed();
                dev_wireless_thread_run_switch(FALSE);
                if(dev_wireless_thread_task_is_change(WS_TASK_GET_CELL_INFO))
                {
                    ret = DDI_EBUSY;
                    break;
                }
                
                if(dev_wireless_thread_is_free())
                {
                    switch(thread_node_param->m_result)
                    {
                        case WIRE_AT_RESULT_OK:
                            ret = DDI_OK;
                            break;

                        case WIRE_AT_RESULT_ERROR:
                            ret = DDI_ERR;
                            break;
                            
                        case WIRE_AT_RESULT_TIMEROUT:
                            ret = DDI_ETIMEOUT;
                            break;
                    }

                    break;
                }

                dev_wireless_thread_run_switch(TRUE);
                ddi_misc_msleep(50);
            }
            dev_wireless_thread_run_switch(TRUE);
            break;
    }

    return ret;
}

/**
 * @brief 模块关机或复位的处理回到
 * @param[in] result:
 * @param[in] src:
 * @param[in] srclen:
 * @retval  DDI_OK:第一步操作成功
 * @retval  DDI_ETIMEOUT:第二步操作完成，可执行关机或复位操作
 */
static s32 G500_power_off_callback(u8 result, u8 *src, u16 srclen)
{
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();
    s32 ret = DDI_ERR;
    s8 *pstr = NULL;

    switch(result)
    {
        case WIRE_AT_RESULT_OK:
            thread_node_param->m_lparam = 1;
            ret = DDI_OK;

            if(WS_TASK_POWEROFF_MODULE == thread_node_param->m_task_id)  //关机收到ok后等2秒关机
            {
                dev_wireless_thread_set_timeout(WIRE_AT_CMD_RESEND_TIMEOUT*2);
            }
            else if(WS_TASK_RESET_MODULE == thread_node_param->m_task_id)  //复位模块按手册建议等12秒---后和袁工沟通，还是用2秒
            {
                dev_wireless_thread_set_timeout(WIRE_AT_CMD_RESEND_TIMEOUT*2);
            }
            break;

        case WIRE_AT_RESULT_TIMEROUT:
            if(0 == thread_node_param->m_lparam)   //cfun=0指令没返回的第一次超时处理，通过拉管脚控制
            {
                TRACE_ERR("cfun=0 exec timeout, then power off use gpio");
                
                if(WIRELESS_WITH_POWERKEY)   //有电源控制引脚
                {
                    dev_gpio_set_value(GPRS_POWERON_PIN, 0);
                    dev_wireless_thread_set_timeout(WIRE_AT_CMD_RESEND_TIMEOUT);
                    thread_node_param->m_lparam = 2;
                }
                else  //没有电源控制脚，直接下电
                {
                    dev_gpio_set_value(GPRS_POWEREN_PIN, 0);
                    dev_wireless_set_power_flag(FALSE);
                    dev_wireless_thread_set_timeout(WIRE_AT_CMD_RESEND_TIMEOUT);
                    thread_node_param->m_lparam = 2;
                }
                ret = DDI_OK;
            }
            else if(1 == thread_node_param->m_lparam)  //cfun=0指令有返回，超时后可执行复位或重启
            {
                TRACE_ERR("finish power off use cfun=0");
                ret = DDI_ETIMEOUT;
                if(WIRELESS_WITH_POWERKEY)   //有电源控制引脚
                {
                    dev_gpio_set_value(GPRS_POWERON_PIN, 1);
                    dev_gpio_set_value(GPRS_POWEREN_PIN, 0);
                    dev_wireless_set_power_flag(FALSE);
                }
                else
                {
                    dev_gpio_set_value(GPRS_POWEREN_PIN, 0);
                    dev_wireless_set_power_flag(FALSE);
                }
            }
            else  if(2 == thread_node_param->m_lparam) //cfun=0指令没返回后的第二次超时处理
            {
                if(WIRELESS_WITH_POWERKEY)   //有电源控制引脚
                {
                    TRACE_ERR("[have power] finish power off gpio step 1");
                    dev_gpio_set_value(GPRS_POWERON_PIN, 1);
                    dev_wireless_thread_set_timeout(WIRE_AT_CMD_RESEND_TIMEOUT*3);
                    thread_node_param->m_lparam = 3;
                    ret = DDI_OK;
                }
                else  //没有电源控制脚，直接下电
                {
                    TRACE_ERR("finish power off gpio");
                    ret = DDI_ETIMEOUT;
                }
            }
            else if(3 == thread_node_param->m_lparam) //cfun=0指令没返回后的第三次超时处理
            {
                TRACE_ERR("[have power] finish power off gpio step 2");
                ret = DDI_ETIMEOUT;
                dev_gpio_set_value(GPRS_POWEREN_PIN, 0);
                dev_wireless_set_power_flag(FALSE);
            }
            break;
    }

    if(DDI_ETIMEOUT == ret)
    {
        dev_wireless_thread_set_result(result);

        //关机成功后停止线程运行
        if(WS_TASK_POWEROFF_MODULE == thread_node_param->m_task_id)
        {
            dev_wireless_thread_run_switch(FALSE);
        }
    }

    return ret;
}

s32 G500_power_off(AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;
    s8 buff[128];
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();

    switch(at_api_get_type)
    {
        case AT_API_NO_BLOCK:
            ret = dev_wireless_send_at_cmd(g_current_module->m_power_off_cmd, AT_API_NO_BLOCK, TRUE);
            if(ret >= 0)
            {
                dev_wireless_thread_set_timeout(WIRE_AT_CMD_RESEND_TIMEOUT);
                ret = DDI_OK;
            }
            break;

        case AT_API_BLOCK:
            dev_wireless_send_at_cmd(g_current_module->m_power_off_cmd, at_api_get_type, TRUE);
            dev_wireless_thread_set_param(WS_TASK_POWEROFF_MODULE, G500_power_off_callback, WIRE_AT_CMD_RESEND_TIMEOUT);
            while(1)
            {
                ddi_watchdog_feed();
                if(dev_wireless_thread_task_is_change(WS_TASK_POWEROFF_MODULE))
                {
                    ret = DDI_EBUSY;
                    break;
                }
                
                if(dev_wireless_thread_is_free())
                {
                    switch(thread_node_param->m_result)
                    {
                        case WIRE_AT_RESULT_TIMEROUT:
                            ret = DDI_OK;
                            break;
                    }

                    break;
                }

                ddi_misc_msleep(50);
            }
            break;
    }

    return ret;
}

static s32 G500_socket_create_callback(u8 result, u8 *src, u16 srclen)
{
    if(WIRE_AT_RESULT_OK == result)
    {
        dev_wireless_thread_set_result(result);
    }
    else if(WIRE_AT_RESULT_ERROR == result || WIRE_AT_RESULT_TIMEROUT == result)
    {
        dev_wireless_thread_set_result(result);

        if(WIRE_AT_RESULT_TIMEROUT == result)
        {
            dev_wireless_set_control_strategy_step(WS_CHECK_CMD);
        }
    }

    return result;
}

static s32 G500_socket_create(s32 socket_id, SOCKET_TYPE type, u8 *param, u16 port, AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;
    s8 buff[128];
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();
    wireless_device_info_t *device_info = dev_wireless_get_device();
    u8 typetmp;

    switch(at_api_get_type)
    {
        case AT_API_NO_BLOCK:
            break;

        case AT_API_BLOCK:
            if(SOCKET_TYPE_TCP == type)
            {
                if((device_info->m_ssltype>=1)&&(device_info->m_ssltype<=3))
                {
                    typetmp = 2;        //采用SSL模式打开
                }
                else
                {
                    typetmp = type;     //采用TCP模式
                }
            }
            memset(buff, 0, sizeof(buff));
            snprintf(buff, sizeof(buff), "AT+MIPOPEN=%d,,\"%s\",%d,%d", socket_id, param, port, typetmp);
            dev_wireless_send_at_cmd(buff, at_api_get_type, TRUE);
            dev_wireless_thread_set_param(WS_TASK_CREATE_SOCKET, G500_socket_create_callback, WIRE_AT_CMD_RESEND_TIMEOUT*3);
            while(1)
            {
                ddi_watchdog_feed();
                dev_wireless_thread_run_switch(FALSE);
                if(dev_wireless_thread_task_is_change(WS_TASK_CREATE_SOCKET))
                {
                    ret = DDI_EBUSY;
                    break;
                }
                
                if(dev_wireless_thread_is_free())
                {
                    switch(thread_node_param->m_result)
                    {
                        case WIRE_AT_RESULT_OK:
                            ret = DDI_OK;
                            break;

                        case WIRE_AT_RESULT_ERROR:
                            ret = DDI_ERR;
                            break;

                        case WIRE_AT_RESULT_TIMEROUT:
                            ret = DDI_ETIMEOUT;
                            break;
                    }

                    break;
                }

                dev_wireless_thread_run_switch(TRUE);
                ddi_misc_msleep(50);
            }
            dev_wireless_thread_run_switch(TRUE);
            break;
    }

    return ret;
}

static s32 G500_get_valid_socketid_list_callback(u8 result, u8 *src, u16 srclen)
{
    s8 dst[WIRELESS_SOCKET_MAX][SPLIT_PER_MAX];
    s32 ret = 0;
    s8 *kw = g_current_module->m_connect_report_head_kw;
    s8 *pstr = NULL;
    s32 i = 0;
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();
    s32 socket_id = thread_node_param->m_lparam;
    s32 tmp_val = 0;
    s32 *socklist = (s32 *)thread_node_param->m_ap;
    s32 socklist_num = thread_node_param->m_lparam;

    TRACE_DBG("result:%d, src:%s:%x", result, src, socklist);
    if(WIRE_AT_RESULT_OTHER == result)
    {
        pstr = strstr(src, kw);
        if(pstr)
        {
            thread_node_param->m_wparam = 1;
            pstr += strlen(kw);

            memset(dst, 0, sizeof(dst));
            ret = dev_maths_split(dst, sizeof(dst)/sizeof(dst[0]), pstr, ",", SPLIT_NOIGNOR);

            for(i=0; i<ret && i<socklist_num; i++)
            {
                dev_maths_str_trim(dst[i]);
                if(socklist)
                {
                    socklist[i] = dev_maths_asc_to_u32(dst[i], strlen(dst[i]), MODULUS_DEC);
                }
            }
        }
    }
    else if(WIRE_AT_RESULT_OK == result)
    {
        if(1 == thread_node_param->m_wparam)
        {
            dev_wireless_thread_set_result(result);
        }
    }
    else if(WIRE_AT_RESULT_ERROR == result || WIRE_AT_RESULT_TIMEROUT == result)
    {
        dev_wireless_thread_set_result(result);

        if(WIRE_AT_RESULT_TIMEROUT == result)
        {
            dev_wireless_set_control_strategy_step(WS_CHECK_CMD);
        }
    }

    return result;
}

static s32 G500_get_valid_socketid_list(s32 *sock_list, s32 socket_list_num, AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;
    s8 buff[128];
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();
    
    switch(at_api_get_type)
    {
        case AT_API_NO_BLOCK:
            break;

        case AT_API_BLOCK:
            memset(buff, 0, sizeof(buff));
            snprintf(buff, sizeof(buff), "AT+MIPOPEN?");
            dev_wireless_send_at_cmd(buff, at_api_get_type, TRUE);
            dev_wireless_thread_set_param(WS_TASK_GET_SOCKET_LIST, G500_get_valid_socketid_list_callback, WIRE_AT_CMD_RESEND_TIMEOUT);
            thread_node_param->m_ap = sock_list;
            thread_node_param->m_lparam = socket_list_num;
            
            while(1)
            {
                ddi_watchdog_feed();
                dev_wireless_thread_run_switch(FALSE);
                if(dev_wireless_thread_task_is_change(WS_TASK_GET_SOCKET_LIST))
                {
                    ret = DDI_EBUSY;
                    break;
                }
                
                if(dev_wireless_thread_is_free())
                {
                    switch(thread_node_param->m_result)
                    {
                        case WIRE_AT_RESULT_OK:
                            ret = DDI_OK;
                            break;

                        case WIRE_AT_RESULT_ERROR:
                            ret = DDI_ERR;
                            break;

                        case WIRE_AT_RESULT_TIMEROUT:
                            ret = DDI_ETIMEOUT;
                            break;
                    }

                    break;
                }

                dev_wireless_thread_run_switch(TRUE);
                ddi_misc_msleep(50);
            }
            dev_wireless_thread_run_switch(TRUE);
            break;
    }

    return ret;
}


/**
* @brief 发送数据到模块缓冲区
* @param[in] task_id 任务id
* @param[in] hand_func 任务执行函数
* @param[in] timeover 任务超时时间
* @retval  无
 */
static s32 G500_send_AT_MIPSEND(s32 id, const u8 *wbuf, u32 wlen)
{
    u8 sendbuf[32];
    s32 i;
    s32 ret;
    s32 len = 0;

    if(wlen > WIRELESS_SOCKET_SEND_BUFLEN)
    {
        return DDI_EDATA;
    }
    
    //发送头部
    snprintf(sendbuf, sizeof(sendbuf), "AT+MIPSEND=%d,\"", id);
    ret = dev_wireless_send_at_cmd(sendbuf, AT_API_BLOCK, FALSE);
    
    //发送数据
    for(i=0; i<wlen; i++)
    {
        memset(sendbuf, 0, sizeof(sendbuf));
        sprintf(sendbuf, "%02X", wbuf[i]);
        dev_wireless_send_at_cmd(sendbuf, AT_API_BLOCK, FALSE);
    }
    
    //发送尾部
    ret = dev_wireless_send_at_cmd("\"", AT_API_BLOCK, FALSE);
    ret = dev_wireless_send_at_cmd(AT_CMD_RES_END, AT_API_BLOCK, FALSE);
    dev_wireless_thread_set_timeout(WIRE_AT_TCP_SEND_TIMEOUT);
    return DDI_OK;
}

static s32 G500_socket_send_onepacket_callback(u8 result, u8 *src, u16 srclen)
{
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();
    s32 ret = DDI_ERR;
    u8 sn;
    u8 *pstr = NULL;
    s8 dst[3][SPLIT_PER_MAX] = {0};

    TRACE_INFO("step=%d, result=%d", thread_node_param->m_step, result);
    switch(thread_node_param->m_step)
    {
        case WIRE_SOCKET_SEND_STEP_TO_BUFF:
            if((WIRE_AT_RESULT_ERROR == result) ||(WIRE_AT_RESULT_TIMEROUT == result))
            {
                dev_wireless_thread_set_result(result);   //出错，结束发送
            }
            else if(WIRE_AT_RESULT_OK == result)
            {
                if(thread_node_param->m_lparam)
                {
                    dev_wireless_thread_set_result(WIRE_AT_RESULT_OK);
                    thread_node_param->m_param1 = 2;//发送PUSH
                }
            }
            else if(WIRE_AT_RESULT_OTHER == result)
            {
                pstr = strstr(src, "+MIPSEND:");
                if(pstr)
                {
                    memset(dst, 0, sizeof(dst));
                    ret = dev_maths_split(dst, sizeof(dst)/sizeof(dst[0]), pstr+strlen("+MIPSEND:"), ",", SPLIT_IGNOR_LEN0);
                    if(sizeof(dst)/sizeof(dst[0]) == ret)
                    {
                        dev_maths_str_trim(dst[0]);
                        dev_maths_str_trim(dst[1]);
                        dev_maths_str_trim(dst[2]);
                        ret = dev_maths_asc_to_u32(dst[1], strlen(dst[1]), MODULUS_DEC);
                        if(ret)
                        {
                            TRACE_INFO("Send buf is overflow, need to wait ok");
                        }
                        else
                        {
                            thread_node_param->m_lparam = 1;
                        }
                    }
                }
                
                if(strstr(src, "+MIPXOFF"))         //缓存满
                {
                    TRACE_INFO("Send buf is overflow, need to wait OK");
                }
                else if(strstr(src, "+MIPXON"))     //可以发送 
                {
                    dev_wireless_thread_set_result(WIRE_AT_RESULT_OK);
                    thread_node_param->m_param1  = 1;//重发buf数据
                }
            }
            break;
            
        case WIRE_SOCKET_SEND_STEP_PUSH:
            if(WIRE_AT_RESULT_TIMEROUT == result)
            {
                dev_wireless_thread_set_result(result);
            }
            else if(WIRE_AT_RESULT_OTHER == result)
            {                      
                pstr = strstr(src, "+MIPPUSH:");
                if(pstr)
                {
                    memset(dst, 0, sizeof(dst));
                    ret = dev_maths_split(dst, sizeof(dst)/sizeof(dst[0]), pstr+strlen("+MIPPUSH:"), ",", SPLIT_IGNOR_LEN0);
                    if(2 == ret)
                    {
                        dev_maths_str_trim(dst[1]);
                        ret = dev_maths_asc_to_u32(dst[1], strlen(dst[1]), MODULUS_DEC);
                        if(0 == ret)
                        {
                            thread_node_param->m_lparam  = 1;
                        }
                        else
                        {
                            TRACE_INFO("push response invalid:%s", dst[1]);
                        }
                    }
                }
            }
            else if(WIRE_AT_RESULT_OK == result)
            {
                if(thread_node_param->m_lparam)
                {
                    dev_wireless_thread_set_result(result);
                }
            }
            break;
    }
    
    return 0;
}

static s32 G500_socket_send_onepacket(s32 socket_id, const u8 *wbuf, s32 wlen)
{
    s32 ret;
    u32 timerid;
    u8 sendflg;
    u8 step;
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();
    s8 tmp_buff[64];

    sendflg = 1;
    while(1)
    {
        ddi_watchdog_feed();
        dev_wireless_thread_run_switch(FALSE);
        if(dev_wireless_thread_task_is_change(WS_TASK_SOCKET_SENDBUFF))
        {
            ret = DDI_EBUSY;
            break;
        }
        
        if(1 == sendflg)   //发送数据到缓冲
        {
            sendflg = 0;
            dev_wireless_thread_set_param(WS_TASK_SOCKET_SENDBUFF, G500_socket_send_onepacket_callback, WIRE_AT_TCP_SEND_TIMEOUT);
            thread_node_param->m_param1 = 0;
            thread_node_param->m_step = WIRE_SOCKET_SEND_STEP_TO_BUFF;
            step = WIRE_SOCKET_SEND_STEP_TO_BUFF;
            G500_send_AT_MIPSEND(socket_id, wbuf, wlen);
        }
        else if(2 == sendflg) //将缓冲数据发到网络
        {
            sendflg = 0;
            dev_wireless_thread_set_param(WS_TASK_SOCKET_SENDBUFF, G500_socket_send_onepacket_callback, WIRE_AT_TCP_SEND_TIMEOUT);
            thread_node_param->m_step = WIRE_SOCKET_SEND_STEP_PUSH;
            thread_node_param->m_param1 = 0;
            step = WIRE_SOCKET_SEND_STEP_PUSH;

            snprintf(tmp_buff, sizeof(tmp_buff), "AT+MIPPUSH=%d", socket_id);
            dev_wireless_send_at_cmd(tmp_buff, AT_API_BLOCK, TRUE);
        }
        
        if(WIRE_TASK_FREE == thread_node_param->m_task_id)
        {
            if(WIRE_SOCKET_SEND_STEP_TO_BUFF == step)
            {
                //发送BUFF数据
                if(WIRE_AT_RESULT_OK == thread_node_param->m_result)
                {
                    if(1 == thread_node_param->m_param1)
                    {
                        //重发数据
                        sendflg = 1;
                        continue;
                    }
                    else if(2 == thread_node_param->m_param1)
                    {
                        //PUSHU数据
                        sendflg = 2;
                        continue;
                    }
                }
                
                if(WIRE_AT_RESULT_TIMEROUT == thread_node_param->m_result)
                {
                    ret = DDI_ETIMEOUT;
                }
                else //if(WIRE_AT_RESULT_ERROR == g_wire_flow_info.m_result)
                {
                    ret = DDI_ERR;
                }
            }
            else
            {
                //发送PUSH
                if(WIRE_AT_RESULT_OK == thread_node_param->m_result)
                {
                    ret = DDI_OK;
                }
                else if(WIRE_AT_RESULT_TIMEROUT == thread_node_param->m_result)
                {
                    ret = DDI_ETIMEOUT;
                }
                else //if(WIRE_AT_RESULT_ERROR == g_wire_flow_info.m_result)
                {
                    ret = DDI_ERR;
                }
            }
            break;
        }
        
        dev_wireless_thread_run_switch(TRUE);
        dev_user_delay_ms(10);
    }

    dev_wireless_thread_run_switch(TRUE);
    return ret;
}

static s32 G500_socket_send(s32 socket_id, const u8 *wbuf, s32 wlen, AT_API_GET_TYPE_E at_api_get_type)
{
    s32 i,j;
    s32 ret = 0;

    if(AT_API_BLOCK == at_api_get_type)
    {
        i = 0;
        while(i<wlen)
        {
            if((i+WIRELESS_SOCKET_SEND_BUFLEN) > wlen)
            {
                j = wlen - i;
            }
            else
            {
                j = WIRELESS_SOCKET_SEND_BUFLEN;
            }
            
            ret = G500_socket_send_onepacket(socket_id, &wbuf[i], j);
            if(ret < 0)
            {
                break;
            }
            else
            {
                i += j;
            }
        }
    }

    if(ret < 0)
    {
        return ret;
    }
    return i;
}

static s32 G500_socket_close_callback(u8 result, u8 *src, u16 srclen)
{
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();
    s8 *pstr = NULL;

    TRACE_INFO("result:%d, src:%s", result, src);
    if(WIRE_AT_RESULT_OK == result)
    {
        dev_wireless_thread_set_result(result);
    }
    else if(WIRE_AT_RESULT_TIMEROUT == result || WIRE_AT_RESULT_ERROR == result)
    {
        TRACE_ERR("close socket failed:%d", result);
        dev_wireless_thread_set_result(result);

        if(WIRE_AT_RESULT_TIMEROUT == result)
        {
            dev_wireless_set_control_strategy_step(WS_CHECK_CMD);
        }
    }

    return result;
}

static s32 G500_socket_close(s32 socket_id, AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;
    s8 buff[128];
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();
    
    switch(at_api_get_type)
    {
        case AT_API_NO_BLOCK:
            break;

        case AT_API_BLOCK:
            memset(buff, 0, sizeof(buff));
            snprintf(buff, sizeof(buff), "AT+MIPCLOSE=%d", socket_id);
            dev_wireless_send_at_cmd(buff, at_api_get_type, TRUE);
            dev_wireless_thread_set_param(WS_TASK_SOCKET_CLOSE, G500_socket_close_callback, WIRE_AT_DEFAULT_TIMEOUT);
            while(1)
            {
                ddi_watchdog_feed();
                dev_wireless_thread_run_switch(FALSE);
                if(dev_wireless_thread_task_is_change(WS_TASK_SOCKET_CLOSE))
                {
                    ret = DDI_EBUSY;
                    break;
                }
                
                if(dev_wireless_thread_is_free())
                {
                    switch(thread_node_param->m_result)
                    {
                        case WIRE_AT_RESULT_OK:
                            ret = DDI_OK;
                            break;

                        case WIRE_AT_RESULT_ERROR:
                            ret = DDI_ERR;
                            break;

                        case WIRE_AT_RESULT_TIMEROUT:
                            ret = DDI_ETIMEOUT;
                            break;
                    }

                    break;
                }

                dev_wireless_thread_run_switch(TRUE);
                ddi_misc_msleep(50);
            }
            dev_wireless_thread_run_switch(TRUE);
            break;
    }

    return ret;
}

static s8 *G500_get_set_apn_cmd(void)
{
    wireless_device_info_t *device_info = dev_wireless_get_device();
    
    if(strlen(device_info->m_username))
    {
        snprintf(g_apn_buff, sizeof(g_apn_buff), "AT+MIPCALL=1,\"%s\",\"%s\",\"%s\"", device_info->m_apn,
                                                                          device_info->m_username,
                                                                          device_info->m_password);
    }
    else
    {
        if(0 == strcmp(device_info->m_apn, "cmnet") && FIBOCOM_L610 == g_current_module->m_id)
        {
            snprintf(g_apn_buff, sizeof(g_apn_buff), "AT+MIPCALL=1");
        }
        else
        {
            snprintf(g_apn_buff, sizeof(g_apn_buff), "AT+MIPCALL=1,\"%s\"", device_info->m_apn);
        }
    }
    
    return g_apn_buff;
}

static s32 G500_set_apn_callback(u8 result, u8 *src, u16 srclen)
{
    TRACE_INFO("result:%d, src:%s", result, src);
    return DDI_ERR;
}

static s32 G500_set_apn(s8 *apn, s8 *username, s8 *password, AT_API_GET_TYPE_E at_api_get_type)
{
    wireless_device_info_t *device_info = dev_wireless_get_device();
    s32 ret = DDI_ERR;

    switch(at_api_get_type)
    {
        case AT_API_NO_BLOCK:
            ret = dev_wireless_send_at_cmd(G500_get_set_apn_cmd(), at_api_get_type, TRUE);
            if(ret >= 0)
            {
                dev_wireless_thread_set_timeout(WIRE_AT_DEFAULT_TIMEOUT);
                ret = DDI_OK;
            }
            break;

        case AT_API_BLOCK:
            if(0 != strcmp(apn, device_info->m_apn)
                || 0 != strcmp(username, device_info->m_username)
                || 0 != strcmp(password, device_info->m_password))
            {
                TRACE_INFO("apn change, then inactive pdp");
                snprintf(device_info->m_apn, sizeof(device_info->m_apn), "%s", apn);
                snprintf(device_info->m_username, sizeof(device_info->m_username), "%s", username);
                snprintf(device_info->m_password, sizeof(device_info->m_password), "%s", password);
                dev_wireless_set_control_strategy_step(WS_INACTIVE_PDP);
            }
            else
            {
                TRACE_INFO("apn is same, then ignore");
            }
            ret = DDI_OK;
            break;
    }

    return ret;
}

static s32 G500_inactive_pdp_callback(u8 result, u8 *src, u16 srclen)
{
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();

    switch(result)
    {
        case WIRE_AT_RESULT_OTHER:  //+MIPCALL: 0  数据分发任务里会处理
            break;

        case WIRE_AT_RESULT_OK:
        case WIRE_AT_RESULT_TIMEROUT:
            if(WS_TASK_INACTIVE_NETWORK_APP == thread_node_param->m_task_id)
            {
                dev_wireless_thread_set_result(result);
            }
            break;
    }

    return 0;
}

static s32 G500_inactive_pdp(AT_API_GET_TYPE_E at_api_get_type)
{
    wireless_device_info_t *device_info = dev_wireless_get_device();
    s32 ret = DDI_ERR;
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();

    switch(at_api_get_type)
    {
        case AT_API_BLOCK:
            dev_wireless_thread_set_param(WS_TASK_INACTIVE_NETWORK_APP, G500_inactive_pdp_callback, WIRE_AT_DEFAULT_TIMEOUT);
            
        case AT_API_NO_BLOCK:
            ret = dev_wireless_send_at_cmd(g_current_module->get_inactive_pdp_cmd, at_api_get_type, TRUE);
            if(ret >= 0)
            {
                dev_wireless_thread_set_timeout(WIRE_AT_DEFAULT_TIMEOUT);
                ret = DDI_OK;
            }
            break;
    }

    if(DDI_OK == ret && AT_API_BLOCK == at_api_get_type)
    {
        while(1)
        {
            ddi_watchdog_feed();
            dev_wireless_thread_run_switch(FALSE);
            if(dev_wireless_thread_task_is_change(WS_TASK_INACTIVE_NETWORK_APP))
            {
                ret = DDI_EBUSY;
                break;
            }
            
            if(dev_wireless_thread_is_free())
            {
                switch(thread_node_param->m_result)
                {
                    case WIRE_AT_RESULT_OK:
                        ret = DDI_OK;
                        break;

                    case WIRE_AT_RESULT_TIMEROUT:
                        ret = DDI_ETIMEOUT;
                        break;
                }
                break;
            }

            dev_wireless_thread_run_switch(TRUE);
            ddi_misc_msleep(50);
        }
        dev_wireless_thread_run_switch(TRUE);
    }

    return ret;
}

static s32 G500_get_local_ip_callback(u8 result, u8 *src, u16 srclen)
{
    wireless_device_info_t *device_info = dev_wireless_get_device();
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();
    s32 ret = DDI_ERR;
    s8 *kw = g_current_module->m_get_local_ip_res_kw;
    s8 *pstr = NULL;
    s32 tmp_val = 0;

    TRACE_DBG("result:%d, src:%s", result, src);
    switch(result)
    {
        case WIRE_AT_RESULT_OTHER:
            tmp_val = G500_handle_ip_at_res_data(src, srclen);
            if(1 == tmp_val)
            {
                ret = TRUE;
            }
            else if(0 == tmp_val)
            {
                TRACE_INFO("mip is close");
                ret = FALSE;
                if(WS_TASK_GET_LOCAL_IP == thread_node_param->m_task_id)
                {
                    memset(device_info->m_local_ip, 0, sizeof(device_info->m_local_ip));
                    TRACE_ERR("detect ip lost, then retry to active net");
                    dev_wireless_set_control_strategy_step(WS_SEARCHING_NETWORK);
                    dev_wireless_thread_set_result(result);
                }
            }
            else if(2 == tmp_val)
            {
                TRACE_INFO("mip is Busy(disconnecting or connecting)");
                ret = DDI_EBUSY;
            }
            break;

        case WIRE_AT_RESULT_OK:
            if(WS_TASK_GET_LOCAL_IP == thread_node_param->m_task_id)
            {
                dev_wireless_thread_set_result(result);
                ret = DDI_OK;
            }
            break;

        case WIRE_AT_RESULT_TIMEROUT:
            if(WS_TASK_GET_LOCAL_IP == thread_node_param->m_task_id)
            {
                TRACE_ERR("get timeout");
                dev_wireless_set_control_strategy_step(WS_CHECK_CMD);
                dev_wireless_thread_set_result(result);
                ret = DDI_ETIMEOUT;
            }
            break;
    }

    return ret;
}

static s32 G500_get_local_ip(u8 *buf, s32 buf_size, AT_API_GET_TYPE_E at_api_get_type)
{
    wireless_device_info_t *device_info = dev_wireless_get_device();
    s32 ret = DDI_ERR;

    switch(at_api_get_type)
    {
        case AT_API_BLOCK:
            dev_wireless_thread_set_param(WS_TASK_GET_LOCAL_IP, G500_get_local_ip_callback, WIRE_AT_DEFAULT_TIMEOUT);
            
        case AT_API_NO_BLOCK:
            ret = dev_wireless_send_at_cmd(g_current_module->m_get_local_ip_cmd, at_api_get_type, TRUE);
            if(ret >= 0)
            {
                dev_wireless_thread_set_timeout(WIRE_AT_DEFAULT_TIMEOUT);
                ret = DDI_OK;
            }
            break;
    }

    if(DDI_OK == ret && AT_API_BLOCK == at_api_get_type)
    {
        while(1)
        {
            ddi_watchdog_feed();
            dev_wireless_thread_run_switch(FALSE);
            if(dev_wireless_thread_task_is_change(WS_TASK_GET_LOCAL_IP))
            {
                ret = DDI_EBUSY;
                break;
            }
            
            if(dev_wireless_thread_is_free())
            {
                snprintf(buf, buf_size, "%s", device_info->m_local_ip);
                ret = DDI_OK;
                break;
            }

            dev_wireless_thread_run_switch(TRUE);
            ddi_misc_msleep(50);
        }
        dev_wireless_thread_run_switch(TRUE);
    }
    
    return ret;
}

static s8 g_ipaddr[32];
static s32 G500_gethostbyname_callback(u8 result, u8 *src, u16 srclen)
{
    s8 *pstr = NULL;
    s8 dst[2][SPLIT_PER_MAX];
    s32 ret = DDI_ERR;
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();
    
    TRACE_INFO("result:%d, src:%s", result, src);
    if(WIRE_AT_RESULT_OTHER == result)
    {
        pstr = strstr(src, "+MIPDNS:");
        if(pstr)
        {
            thread_node_param->m_lparam = TRUE;
            pstr += strlen("+MIPDNS:");
            memset(dst, 0, sizeof(dst));
            ret = dev_maths_split(dst, sizeof(dst)/sizeof(dst[0]), src, ",", SPLIT_NOIGNOR);
            if(sizeof(dst)/sizeof(dst[0]) == ret)
            {
                snprintf(g_ipaddr, sizeof(g_ipaddr), "%s", dst[1]);
            }
        }
    }
    else if(WIRE_AT_RESULT_OK == result || WIRE_AT_RESULT_TIMEROUT == result)
    {
        dev_wireless_thread_set_result(result);
    }
    
    return ret;
}

static s32 G500_gethostbyname(s8 *domain_name, s8 *ip_addr, AT_API_GET_TYPE_E at_api_get_type)
{
    wireless_device_info_t *device_info = dev_wireless_get_device();
    s32 ret = DDI_ERR;
    s8 tmp_buff[128] = {0};
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();

    switch(at_api_get_type)
    {
        case AT_API_NO_BLOCK:
            break;
            
        case AT_API_BLOCK:
            memset(g_ipaddr, 0, sizeof(g_ipaddr));
            memset(tmp_buff, 0, sizeof(tmp_buff));
            snprintf(tmp_buff, sizeof(tmp_buff), "AT+MIPDNS =\"%s\"", domain_name);
            dev_wireless_send_at_cmd(tmp_buff, at_api_get_type, TRUE);
            dev_wireless_thread_set_param(WS_TASK_GET_HOSTBYNAME, G500_gethostbyname_callback, WIRE_AT_DEFAULT_TIMEOUT*2);
            ret = DDI_OK;
            break;
    }

    if(DDI_OK == ret && AT_API_BLOCK == at_api_get_type)
    {
        while(1)
        {
            ddi_watchdog_feed();
            dev_wireless_thread_run_switch(FALSE);
            if(dev_wireless_thread_task_is_change(WS_TASK_GET_HOSTBYNAME))
            {
                ret = DDI_EBUSY;
                break;
            }
            
            if(dev_wireless_thread_is_free())
            {
                switch(thread_node_param->m_result)
                {
                    case WIRE_AT_RESULT_OK:
                        memcpy(ip_addr, g_ipaddr, strlen(g_ipaddr));
                        ret = DDI_OK;
                        break;

                    case WIRE_AT_RESULT_TIMEROUT:
                        ret = DDI_ETIMEOUT;
                        break;
                }
                break;
            }

            dev_wireless_thread_run_switch(TRUE);
            ddi_misc_msleep(50);
        }

        dev_wireless_thread_run_switch(TRUE);
    }
    
    return ret;
}

static s32 G500_cfg_rec_data_format_callback(u8 result, u8 *src, u16 srclen)
{
    return DDI_OK;
}

static s32 G500_cfg_rec_data_format(AT_API_GET_TYPE_E at_api_get_type)
{
    wireless_device_info_t *device_info = dev_wireless_get_device();
    s32 ret = DDI_ERR;

    switch(at_api_get_type)
    {
        case AT_API_NO_BLOCK:
            ret = dev_wireless_send_at_cmd(g_current_module->m_cfg_rec_data_format_cmd, at_api_get_type, TRUE);
            if(ret >= 0)
            {
                dev_wireless_thread_set_timeout(WIRE_AT_DEFAULT_TIMEOUT);
                ret = DDI_OK;
            }
            break;

        case AT_API_BLOCK:
            break;
    }

    return ret;
}

static s32 G500_get_imsi_callback(u8 result, u8 *src, u16 srclen)
{
    wireless_device_info_t *device_info = dev_wireless_get_device();
    s32 ret = DDI_ERR;
    s8 mcc[4], mnc[4];

    do{
        if(!device_info)
        {
            ret = DDI_ERR;
            break;
        }

        if(!strstr(src, "+CGREG:") && !strstr(src, "+CEREG:"))
        {
            memset(mcc, 0, sizeof(mcc));
            memset(mnc, 0, sizeof(mnc));
            memcpy(mcc, src, 3);
            memcpy(mnc, src+3, 2);
            device_info->m_main_cell_info.mcc = dev_maths_asc_to_u32(mcc, strlen(mcc), MODULUS_DEC);
            device_info->m_main_cell_info.mnc = dev_maths_asc_to_u32(mnc, strlen(mnc), MODULUS_DEC);
            snprintf(device_info->m_imsi, sizeof(device_info->m_imsi), "%s", src);
            TRACE_INFO("imsi:%s", device_info->m_imsi);
            ret = DDI_OK;
        }
    }while(0);

    return ret;
}

static s32 G500_get_imsi(u8 *buf, u32 buf_size, AT_API_GET_TYPE_E at_api_get_type)
{
    wireless_device_info_t *device_info = dev_wireless_get_device();
    s32 ret = DDI_ERR;

    switch(at_api_get_type)
    {
        case AT_API_NO_BLOCK:
            ret = dev_wireless_send_at_cmd(g_current_module->m_get_imsi_cmd, at_api_get_type, TRUE);
            if(ret >= 0)
            {
                dev_wireless_thread_set_timeout(WIRE_AT_DEFAULT_TIMEOUT);
                ret = DDI_OK;
            }
            break;

        case AT_API_BLOCK:
            if(device_info)
            {
                snprintf(buf, buf_size, "%s", device_info->m_imsi);
                ret = DDI_OK;
            }
            break;
    }

    return ret;
}

static s32 G500_get_net_cs_reg_callback(u8 result, u8 *src, u16 srclen)
{
    s32 ret = DDI_ERR;
    s8 *pstr = NULL;
    s8 dst[2][SPLIT_PER_MAX] = {0};
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();

    TRACE_DBG("result:%d", result);
    switch(result)
    {
        case WIRE_AT_RESULT_OTHER:
            pstr = strstr(src, "+CREG:");
            if(pstr)
            {
                pstr += strlen("+CREG:");
                memset(dst, 0, sizeof(dst));
                ret = dev_maths_split(dst, sizeof(dst)/sizeof(dst[0]), pstr, ",", SPLIT_IGNOR_LEN0);
                if(sizeof(dst)/sizeof(dst[0]) == ret)
                {
                    dev_maths_str_trim(dst[1]);
                    ret = dev_maths_asc_to_u32(dst[1], strlen(dst[1]), MODULUS_DEC);
                    if(1 == ret || 5 == ret)
                    {
                        ret = TRUE;
                    }
                    else
                    {
                        ret = FALSE;
                    }
                }
                else
                {
                    if(1 == ret)   //只返回cgreg:0
                    {
                        dev_maths_str_trim(dst[0]);
                        ret = dev_maths_asc_to_u32(dst[0], strlen(dst[0]), MODULUS_DEC);
                        if(0 == ret)
                        {
                            ret = FALSE;
                        }
                    }
                }
            }

            if(WS_TASK_CHECK_CS_REG == thread_node_param->m_task_id)
            {
                thread_node_param->m_lparam = ret;
            }
            break;

       case WIRE_AT_RESULT_OK:
            if(WS_TASK_CHECK_CS_REG == thread_node_param->m_task_id)
            {
                ret = thread_node_param->m_lparam;
                dev_wireless_thread_set_result(result);
                thread_node_param->m_lparam = ret;
            }
            break;

       case WIRE_AT_RESULT_TIMEROUT:
            dev_wireless_thread_set_result(result);
            break;
    }

    return ret;
}

static s32 G500_get_net_cs_reg(AT_API_GET_TYPE_E at_api_get_type)
{
    wireless_device_info_t *device_info = dev_wireless_get_device();
    s32 ret = DDI_ERR;
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();

    switch(at_api_get_type)
    {
        case AT_API_NO_BLOCK:
            break;
            
        case AT_API_BLOCK:
            ret = dev_wireless_send_at_cmd("AT+CREG?", at_api_get_type, TRUE);
            if(ret >= 0)
            {
                dev_wireless_thread_set_timeout(WIRE_AT_DEFAULT_TIMEOUT);
                ret = DDI_OK;
            }
            break;
    }

    if(AT_API_BLOCK == at_api_get_type)
    {
        dev_wireless_thread_set_param(WS_TASK_CHECK_CS_REG, G500_get_net_cs_reg_callback, WIRE_AT_DEFAULT_TIMEOUT);
        while(1)
        {
            ddi_watchdog_feed();
            dev_wireless_thread_run_switch(FALSE);
            if(dev_wireless_thread_task_is_change(WS_TASK_CHECK_CS_REG))
            {
                ret = DDI_EBUSY;
                break;
            }
            
            if(dev_wireless_thread_is_free())
            {
                switch(thread_node_param->m_result)
                {
                    case WIRE_AT_RESULT_OK:
                        ret = thread_node_param->m_lparam;
                        break;

                    case WIRE_AT_RESULT_TIMEROUT:
                        ret = FALSE;
                        break;
                }

                break;
            }
            dev_wireless_thread_run_switch(TRUE);
            ddi_misc_msleep(100);
        }
        dev_wireless_thread_run_switch(TRUE);
    }

    if(ret < DDI_OK)
    {
        ret = FALSE;
    }

    return ret;
}

static s32 G500_get_net_reg_callback(u8 result, u8 *src, u16 srclen)
{
    s32 ret = DDI_ERR;
    s8 *pstr = NULL;
    s8 dst[5][SPLIT_PER_MAX] = {0};
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();
    s32 i = 0;
    wireless_device_info_t *wireless_device = dev_wireless_get_device();

    TRACE_DBG("result:%d", result);
    switch(result)
    {
        case WIRE_AT_RESULT_OTHER:
            pstr = strstr(src, g_current_module->m_get_net_reg_res_kw);
            if(pstr)
            {
                pstr += strlen(g_current_module->m_get_net_reg_res_kw);
                memset(dst, 0, sizeof(dst));
                ret = dev_maths_split(dst, sizeof(dst)/sizeof(dst[0]), pstr, ",", SPLIT_IGNOR_LEN0);

                //去掉空格
                for(i=0; i<ret; i++)
                {
                    dev_maths_str_trim(dst[i]);
                }

                //+CGREG: n,status
                //+CGREG: 2,1,"2616","00000F15",0
                if(ret >= 2)
                {
                    if(ret > 2)  //取主基站信息
                    {
                        dev_maths_str_trim_special(dst[2], '\"');
                        dev_maths_str_trim_special(dst[3], '\"');
                        wireless_device->m_main_cell_info.lac = dev_maths_asc_to_u32(dst[2], strlen(dst[2]), MODULUS_HEX);
                        wireless_device->m_main_cell_info.cid = dev_maths_asc_to_u32(dst[3], strlen(dst[3]), MODULUS_HEX);
                        TRACE_INFO("lac:%x, cid:%x", wireless_device->m_main_cell_info.lac, wireless_device->m_main_cell_info.cid);
                    }
                    
                    dev_maths_str_trim(dst[1]);
                    ret = dev_maths_asc_to_u32(dst[1], strlen(dst[1]), MODULUS_DEC);
                    if(1 == ret || 5 == ret)
                    {
                        TRACE_INFO("reg success:%d", ret);
                        ret = TRUE;
                    }
                    else
                    {
                        ret = FALSE;
                    }
                }
                else if(1 == ret)   //只返回cgreg:0
                {
                    dev_maths_str_trim(dst[0]);
                    ret = dev_maths_asc_to_u32(dst[0], strlen(dst[0]), MODULUS_DEC);
                    if(0 == ret)
                    {
                        ret = FALSE;
                    }
                }
                else
                {
                    ret = FALSE;
                }
            }

            if(WS_TASK_CHECK_REG == thread_node_param->m_task_id)
            {
                thread_node_param->m_lparam = ret;
            }
            break;

       case WIRE_AT_RESULT_OK:
            if(WS_TASK_CHECK_REG == thread_node_param->m_task_id)
            {
                ret = thread_node_param->m_lparam;
                dev_wireless_thread_set_result(result);
                thread_node_param->m_lparam = ret;
            }
            break;

       case WIRE_AT_RESULT_TIMEROUT:
            dev_wireless_thread_set_result(result);
            break;
    }

    //注网时决定发AT+CGREG=2还是AT+CEREG=2
    if(WS_TASK_CHECK_REG != thread_node_param->m_task_id)
    {
        if(ret == TRUE)
        {
            if(strstr(src, "+CGREG"))
            {
                g_wire_net_reg_success = 1;
            }
            else
            {
                g_wire_net_reg_success = 2;
            }
        }
        else
        {
            //说明当前这种网络没有注网上
            if(g_wire_net_reg_success > 0)
            {
                TRACE_DBG("direct use last reg info:%d", g_wire_net_reg_success);
                ret = TRUE;
            }
        }
    }
    return ret;
}

static s32 G500_get_net_reg(AT_API_GET_TYPE_E at_api_get_type)
{
    wireless_device_info_t *device_info = dev_wireless_get_device();
    s32 ret = DDI_ERR;
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();
    
    switch(at_api_get_type)
    {
        case AT_API_NO_BLOCK:
            if(g_current_module->m_id == FIBOCOM_G500)
            {
                ret = dev_wireless_send_at_cmd(g_current_module->m_get_2G_net_reg_cmd, at_api_get_type, TRUE);
            }
            else
            {
                if(thread_node_param->m_step == WIRE_SEARCHING_NETWORK_CHECK_REG)   //查询CEREG或CGREG是否注网成功
                {
                    //注网成功后，查两次，避免4G卡查CGREG时先返回成功
                    if(g_wire_net_reg_success > 0)
                    {
                        ret = dev_wireless_send_at_cmd(g_current_module->m_get_4G_net_reg_cmd, at_api_get_type, TRUE);
                    }
                    else
                    {
                        //偶数次查CGREG是否注网成功  ，奇数次查CEREG是否成功
                        if(g_check_reg_try_time%2 == 0)
                        {
                            ret = dev_wireless_send_at_cmd(g_current_module->m_get_2G_net_reg_cmd, at_api_get_type, TRUE);
                        }
                        else if(g_check_reg_try_time%2 == 1)
                        {
                            ret = dev_wireless_send_at_cmd(g_current_module->m_get_4G_net_reg_cmd, at_api_get_type, TRUE);
                        }
                    }
                }
                else if(thread_node_param->m_step == WIRE_SEARCHING_NETWORK_GET_MAIL_CELL)    //获取主基站
                {
                    if(1 == g_wire_net_reg_success)
                    {
                        ret = dev_wireless_send_at_cmd(g_current_module->m_get_2G_net_reg_cmd, at_api_get_type, TRUE);
                    }
                    else
                    {
                        ret = dev_wireless_send_at_cmd(g_current_module->m_get_4G_net_reg_cmd, at_api_get_type, TRUE);
                    }
                }
            }
            
            if(ret >= 0)
            {
                dev_wireless_thread_set_timeout(WIRE_AT_DEFAULT_TIMEOUT);
                ret = DDI_OK;
            }
            g_check_reg_try_time++;
            break;
            
        case AT_API_BLOCK:
            if(1 == g_wire_net_reg_success)
            {
                ret = dev_wireless_send_at_cmd(g_current_module->m_get_2G_net_reg_cmd, at_api_get_type, TRUE);
            }
            else
            {
                ret = dev_wireless_send_at_cmd(g_current_module->m_get_4G_net_reg_cmd, at_api_get_type, TRUE);
            }
            if(ret >= 0)
            {
                dev_wireless_thread_set_timeout(WIRE_AT_DEFAULT_TIMEOUT);
                ret = DDI_OK;
            }
            break;
    }

    if(AT_API_BLOCK == at_api_get_type)
    {
        dev_wireless_thread_set_param(WS_TASK_CHECK_REG, G500_get_net_reg_callback, WIRE_AT_DEFAULT_TIMEOUT);
        while(1)
        {
            ddi_watchdog_feed();
            dev_wireless_thread_run_switch(FALSE);
            if(dev_wireless_thread_task_is_change(WS_TASK_CHECK_REG))
            {
                ret = DDI_EBUSY;
                break;
            }
            
            if(dev_wireless_thread_is_free())
            {
                switch(thread_node_param->m_result)
                {
                    case WIRE_AT_RESULT_OK:
                        ret = thread_node_param->m_lparam;
                        break;

                    case WIRE_AT_RESULT_TIMEROUT:
                        ret = FALSE;
                        break;
                }

                break;
            }
            
            dev_wireless_thread_run_switch(TRUE);
            ddi_misc_msleep(100);
        }

        dev_wireless_thread_run_switch(TRUE);
    }

    return ret;
}

static s8* G500_get_reg_cmd(void)
{
    if((g_check_reg_try_time-1)%2 == 0 || g_current_module->m_id == FIBOCOM_G500)
    {
        return g_current_module->m_get_2G_net_reg_cmd;
    }
    else if((g_check_reg_try_time-1)%2 == 1)
    {
        return g_current_module->m_get_4G_net_reg_cmd;
    }
}

static s8* G500_get_set_reg_param_cmd(void)
{
    if(1 == g_wire_net_reg_success || g_current_module->m_id == FIBOCOM_G500)
    {
        return g_current_module->m_set_2G_reg_param_cmd;
    }
    else
    {
        return g_current_module->m_set_4G_reg_param_cmd;
    }
}

static s32 G500_set_reg_param(AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();
    
    switch(at_api_get_type)
    {
        case AT_API_NO_BLOCK:
            if(1 == g_wire_net_reg_success || g_current_module->m_id == FIBOCOM_G500)
            {
                ret = dev_wireless_send_at_cmd(g_current_module->m_set_2G_reg_param_cmd, at_api_get_type, TRUE);
            }
            else
            {
                ret = dev_wireless_send_at_cmd(g_current_module->m_set_4G_reg_param_cmd, at_api_get_type, TRUE);
            }
            
            if(ret >= 0)
            {
                dev_wireless_thread_set_timeout(WIRE_AT_DEFAULT_TIMEOUT);
                ret = DDI_OK;
            }
            break;
            
        case AT_API_BLOCK:
            break;
    }

    return ret;
}

static s32 G500_get_csq_callback(u8 result, u8 *src, u16 srclen)
{
    wireless_device_info_t *device_info = dev_wireless_get_device();
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();
    s32 ret = DDI_ERR;
    s8 *pstr = NULL;
    s8 dst[2][SPLIT_PER_MAX] = {0};

    switch(result)
    {
        case WIRE_AT_RESULT_OTHER:
            pstr = strstr(src, g_current_module->m_get_csq_res_kw);
            if(pstr)
            {
                pstr += strlen(g_current_module->m_get_csq_res_kw);
                memset(dst, 0, sizeof(dst));
                ret = dev_maths_split(dst, sizeof(dst)/sizeof(dst[0]), pstr, ",", SPLIT_IGNOR_LEN0);
                if(sizeof(dst)/sizeof(dst[0]) == ret)
                {
                    dev_maths_str_trim(dst[0]);
                    dev_maths_str_trim(dst[1]);
                    device_info->m_csq = dev_maths_asc_to_u32(dst[0], strlen(dst[0]), MODULUS_DEC);
                    device_info->m_bear_rate = dev_maths_asc_to_u32(dst[1], strlen(dst[1]), MODULUS_DEC);
                    ret = DDI_OK;
                }
            }
            break;

        case WIRE_AT_RESULT_OK:
            if(WS_TASK_GET_CSQ == thread_node_param->m_task_id)
            {
                dev_wireless_thread_set_result(result);
                ret = DDI_OK;
            }
            break;

        case WIRE_AT_RESULT_TIMEROUT:
            if(WS_TASK_GET_CSQ == thread_node_param->m_task_id)
            {
                TRACE_ERR("get csq timeout");
                dev_wireless_set_control_strategy_step(WS_CHECK_CMD);
                dev_wireless_thread_set_result(result);
                ret = DDI_ETIMEOUT;
            }
            break;
    }

    return ret;
}

static s32 G500_get_csq(u8 *csq, u16 *bear_rate, AT_API_GET_TYPE_E at_api_get_type)
{
    wireless_device_info_t *device_info = dev_wireless_get_device();
    s32 ret = DDI_ERR;

    switch(at_api_get_type)
    {
        case AT_API_BLOCK:
            dev_wireless_thread_set_param(WS_TASK_GET_CSQ, G500_get_csq_callback, WIRE_AT_DEFAULT_TIMEOUT);

        case AT_API_NO_BLOCK:
            ret = dev_wireless_send_at_cmd(g_current_module->m_get_csq_cmd, at_api_get_type, TRUE);
            if(ret >= 0)
            {
                dev_wireless_thread_set_timeout(WIRE_AT_DEFAULT_TIMEOUT);
                ret = DDI_OK;
            }
            break;
    }

    if(DDI_OK == ret && AT_API_BLOCK == at_api_get_type)
    {
        while(1)
        {
            ddi_watchdog_feed();
            dev_wireless_thread_run_switch(FALSE);
            if(dev_wireless_thread_task_is_change(WS_TASK_GET_CSQ))
            {
                ret = DDI_EBUSY;
                break;
            }
            
            if(dev_wireless_thread_is_free())
            {
                break;
            }

            dev_wireless_thread_run_switch(TRUE);
            ddi_misc_msleep(50);
        }

        dev_wireless_thread_run_switch(TRUE);
    }

    return ret;
}

static s32 G500_get_ccid_callback(u8 result, u8 *src, u16 srclen)
{
    wireless_device_info_t *device_info = dev_wireless_get_device();
    s32 ret = DDI_ERR;
    s8 *pstr = NULL;

    do{
        if(!device_info)
        {
            ret = DDI_ERR;
            break;
        }

        pstr = strstr(src, g_current_module->m_ccid_cmd_res_kw);
        if(pstr)
        {
            pstr += strlen(g_current_module->m_ccid_cmd_res_kw);
            dev_maths_str_trim(pstr);
            snprintf(device_info->m_ccid, sizeof(device_info->m_imei), "%s", pstr);
            TRACE_INFO("ccid:%s", device_info->m_ccid);
            ret = DDI_OK;
        }
        break;
    }while(0);

    return ret;
}

static s32 G500_get_ccid(u8 *buf, u32 buf_size, AT_API_GET_TYPE_E at_api_get_type)
{
    wireless_device_info_t *device_info = dev_wireless_get_device();
    s32 ret = DDI_ERR;

    switch(at_api_get_type)
    {
        case AT_API_NO_BLOCK:
            ret = dev_wireless_send_at_cmd(g_current_module->m_ccid_cmd, at_api_get_type, TRUE);
            if(ret >= 0)
            {
                dev_wireless_thread_set_timeout(WIRE_AT_DEFAULT_TIMEOUT);
                ret = DDI_OK;
            }
            break;

        case AT_API_BLOCK:
            if(device_info)
            {
                snprintf(buf, buf_size, "%s", device_info->m_ccid);
                ret = DDI_OK;
            }
            break;
    }

    return ret;
}

/**
 * @brief 判断返回结果是公网模式还是仪器模式
 * @param[in] 
 * @retval TRUE 公网模式
 * @retval FALSE 仪器模式
 * @retval DDI_ERR 失败
 */
static s32 G500_check_rplmn_callback(u8 result, u8 *src, u16 srclen)
{
    s32 ret = DDI_ERR;
    
    TRACE_DBG("result:%d, src:%s", result, src);
    if(WIRE_AT_RESULT_OTHER == result)
    {
        if(strstr(src, "64F000") || strstr(src, "FFFFFF"))
        {
            ret = TRUE;
        }
        else if(strstr(src, "00F110"))
        {
            ret = FALSE;
        }
    }

    //只有广和通G500 2G模块才校验此参数
    if(FIBOCOM_G500 != g_current_module->m_id)
    {
        ret = TRUE;
    }
    
    return ret;
}

static s32 G500_check_rplmn(AT_API_GET_TYPE_E at_api_get_type)
{
    wireless_device_info_t *device_info = dev_wireless_get_device();
    s32 ret = DDI_ERR;

    switch(at_api_get_type)
    {
        case AT_API_NO_BLOCK:
            ret = dev_wireless_send_at_cmd(g_current_module->m_check_rplmn_cmd, at_api_get_type, TRUE);
            if(ret >= 0)
            {
                dev_wireless_thread_set_timeout(WIRE_AT_DEFAULT_TIMEOUT);
                ret = DDI_OK;
            }
            break;

        case AT_API_BLOCK:
            break;
    }

    return ret;
}

static s32 G500_get_imei_callback(u8 result, u8 *src, u16 srclen)
{
    wireless_device_info_t *device_info = dev_wireless_get_device();
    s32 ret = DDI_ERR;

    do{
        if(!device_info)
        {
            ret = DDI_ERR;
            break;
        }
        
        snprintf(device_info->m_imei, sizeof(device_info->m_imei), "%s", src);
        ret = DDI_OK;
        break;
    }while(0);

    return ret;
}

static s32 G500_get_imei(u8 *imei_buf, u32 buf_size, AT_API_GET_TYPE_E at_api_get_type)
{
    wireless_device_info_t *device_info = dev_wireless_get_device();
    s32 ret = DDI_ERR;

    switch(at_api_get_type)
    {
        case AT_API_NO_BLOCK:
            ret = dev_wireless_send_at_cmd(g_current_module->m_imei_cmd, at_api_get_type, TRUE);
            if(ret >= 0)
            {
                dev_wireless_thread_set_timeout(WIRE_AT_DEFAULT_TIMEOUT);
                ret = DDI_OK;
            }
            break;

        case AT_API_BLOCK:
            if(device_info)
            {
                snprintf(imei_buf, buf_size, "%s", device_info->m_imei);
                ret = DDI_OK;
            }
            break;
    }

    return ret;
}

static s32 G500_check_work_mode(AT_API_GET_TYPE_E at_api_get_type)
{
    wireless_device_info_t *device_info = dev_wireless_get_device();
    s32 ret = DDI_ERR;

    switch(at_api_get_type)
    {
        case AT_API_NO_BLOCK:
            if(FIBOCOM_L610 == g_current_module->m_id)
            {
                ret = dev_wireless_send_at_cmd(g_current_module->m_check_workmod_cmd, at_api_get_type, TRUE);
                if(ret >= 0)
                {
                    dev_wireless_thread_set_timeout(WIRE_AT_DEFAULT_TIMEOUT);
                    ret = DDI_OK;
                }
            }
            else
            {
                ret = TRUE;
            }
            break;

        case AT_API_BLOCK:
            break;
    }

    return ret;
}


static s32 G500_cfg_flow_control(AT_API_GET_TYPE_E at_api_get_type)
{
    wireless_device_info_t *device_info = dev_wireless_get_device();
    s32 ret = DDI_ERR;

    switch(at_api_get_type)
    {
        case AT_API_NO_BLOCK:
            ret = dev_wireless_send_at_cmd("AT&K3", at_api_get_type, TRUE);
            if(ret >= 0)
            {
                dev_wireless_thread_set_timeout(WIRE_AT_DEFAULT_TIMEOUT);
                ret = DDI_OK;
            }
            break;

        case AT_API_BLOCK:
            break;
    }

    return ret;
}

static s32 G500_get_simcard_state_callback(u8 result, u8 *src, u16 srclen)
{
    wireless_device_info_t *device_info = dev_wireless_get_device();
    s32 ret = DDI_ERR;

    do{
        if(!device_info)
        {
            ret = DDI_ERR;
            break;
        }

        if(0 == strcmp(src, g_current_module->m_simcard_state_cmd_res_kw))
        {
            ret = WITH_SIMCARD;
        }
        break;
    }while(0);

    return ret;
}

static s32 G500_get_simcard_state(AT_API_GET_TYPE_E at_api_get_type)
{
    wireless_device_info_t *device_info = dev_wireless_get_device();
    s32 ret = DDI_ERR;

    switch(at_api_get_type)
    {
        case AT_API_NO_BLOCK:
            ret = dev_wireless_send_at_cmd(g_current_module->m_simcard_state_cmd, at_api_get_type, TRUE);
            if(ret >= 0)
            {
                dev_wireless_thread_set_timeout(WIRE_AT_DEFAULT_TIMEOUT);
                ret = DDI_OK;
            }
            break;

        case AT_API_BLOCK:
            if(device_info)
            {
                ret = device_info->m_simcard_status;
            }
            break;
    }

    return ret;
}
//配置GPIO口-----------------------------------------------
static s32 G500_cfg_exGPIO_callback(u8 result, u8 *src, u16 srclen)
{
//    wireless_thread_node_param_t *thread_node_param = dev_wireless_get_thread_node_param();
    s32 ret = DDI_ERR;

    switch(result)
    {
    case WIRE_AT_RESULT_OTHER:
        if(0 == strcmp(src, "+GPIOCFG:OK"))
        {
            ret = DDI_OK;
            dev_wireless_thread_set_result(WIRE_AT_RESULT_OK);
        }
        else 
        {
            ret = DDI_ERR;
            dev_wireless_thread_set_result(WIRE_AT_RESULT_ERROR);
        }
        break;
    case WIRE_AT_RESULT_OK:
        ret = DDI_OK;
        dev_wireless_thread_set_result(result);
        break;
        
    case WIRE_AT_RESULT_TIMEROUT:
        ret = DDI_ETIMEOUT;
    case WIRE_AT_RESULT_ERROR:
        dev_wireless_thread_set_result(result);
        break;
    }
    return ret;
}
/**
 * @brief  设置GPIO
 * @param[in]
 * @param[in] 
 * @retval  DDI_OK 成功
 * @retval  DDI_ERR 失败
 */
static s32 G500_cfg_exGPIO(u8 exGPIO, u8 cfg, AT_API_GET_TYPE_E at_api_get_type)
{
//    wireless_device_info_t *device_info = dev_wireless_get_device();
    wireless_thread_node_param_t *thread_node_param = dev_wireless_get_thread_node_param();
    s32 ret = DDI_ERR;
    s8 buff[128];
    s32 i;
    u8 l_cfg;

    
    if(cfg)
    {
        l_cfg = 1;
    }
    else
    {
        l_cfg = 0;
    }
    snprintf(buff, sizeof(buff), "AT=AT+GPIOCFG=%d,%d", exGPIO, l_cfg);
    switch(at_api_get_type)
    {
    case AT_API_NO_BLOCK:
        //非阻塞
        dev_wireless_send_at_cmd(buff, at_api_get_type, TRUE);
        dev_wireless_thread_set_timeout(WIRE_AT_CTL_TIMEOUT);
        ret = DDI_OK;    
        break;
    case AT_API_BLOCK:
        dev_wireless_thread_set_param(WS_TASK_SET_GPIOOUT, G500_cfg_exGPIO_callback, WIRE_AT_CTL_TIMEOUT);
        dev_wireless_send_at_cmd(buff, at_api_get_type, TRUE);
        ret = DDI_OK;
        break;
    }

    if(DDI_OK == ret && AT_API_BLOCK == at_api_get_type)
    {
        while(1)
        {
            ddi_watchdog_feed();
            if(dev_wireless_thread_task_is_change(WS_TASK_SET_GPIOOUT))
            {
                ret = DDI_EBUSY;
                break;
            }
            if(dev_wireless_thread_is_free())
            {
                if(WIRE_AT_RESULT_OK == thread_node_param->m_result)
                {
                    ret = DDI_OK;
                }
                else
                {
                    ret = DDI_ERR;
                }
                break;
            }
            ddi_misc_msleep(50);
        }
    }
    return ret;
}
//设置GPIO口-----------------------------------------------
static s32 G500_set_exGPIO_callback(u8 result, u8 *src, u16 srclen)
{
    wireless_thread_node_param_t *thread_node_param = dev_wireless_get_thread_node_param();
    s32 ret = DDI_ERR;

    switch(result)
    {
    case WIRE_AT_RESULT_OTHER:
        if(0 == strcmp(src, "+GPIOSET:OK"))
        {
            ret = DDI_OK;
            dev_wireless_thread_set_result(WIRE_AT_RESULT_OK);
        }
        else 
        {
            ret = DDI_ERR;
            dev_wireless_thread_set_result(WIRE_AT_RESULT_ERROR);
        }
        break;
    case WIRE_AT_RESULT_OK:
        ret = DDI_OK;
        dev_wireless_thread_set_result(result);
        break;
        
    case WIRE_AT_RESULT_TIMEROUT:
        ret = DDI_ETIMEOUT;
    case WIRE_AT_RESULT_ERROR:
        dev_wireless_thread_set_result(result);
        break;
    }
    return ret;
}
/**
 * @brief  设置GPIO
 * @param[in]
 * @param[in] 
 * @retval  DDI_OK 成功
 * @retval  DDI_ERR 失败
 */
static s32 G500_set_exGPIO(u8 exGPIO, u8 value, AT_API_GET_TYPE_E at_api_get_type)
{
    wireless_device_info_t *device_info = dev_wireless_get_device();
    wireless_thread_node_param_t *thread_node_param = dev_wireless_get_thread_node_param();
    s32 ret = DDI_ERR;
    s8 buff[128];
    s32 i;
    u8 l_value;

    
    if(value)
    {
        l_value = 1;
    }
    else
    {
        l_value = 0;
    }
    switch(at_api_get_type)
    {
    case AT_API_NO_BLOCK:
        //非阻塞
        snprintf(buff, sizeof(buff), "AT=AT+GPIOSET=%d,%d", exGPIO, l_value);
        dev_wireless_send_at_cmd(buff, at_api_get_type, TRUE);
        dev_wireless_thread_set_timeout(WIRE_AT_CTL_TIMEOUT);
        ret = DDI_OK;    
        break;
    case AT_API_BLOCK:
        dev_wireless_thread_set_param(WS_TASK_SET_GPIOOUT, G500_set_exGPIO_callback, WIRE_AT_CTL_TIMEOUT);
        snprintf(buff, sizeof(buff), "AT=AT+GPIOSET=%d,%d", exGPIO, l_value);
        dev_wireless_send_at_cmd(buff, at_api_get_type, TRUE);
        ret = DDI_OK;
        break;
    }

    if(DDI_OK == ret && AT_API_BLOCK == at_api_get_type)
    {
        while(1)
        {
            ddi_watchdog_feed();
            if(dev_wireless_thread_task_is_change(WS_TASK_SET_GPIOOUT))
            {
                ret = DDI_EBUSY;
                break;
            }
            if(dev_wireless_thread_is_free())
            {
                if(WIRE_AT_RESULT_OK == thread_node_param->m_result)
                {
                    ret = DDI_OK;
                }
                else
                {
                    ret = DDI_ERR;
                }
                break;
            }
            ddi_misc_msleep(50);
        }
    }
    return ret;
}
//控制BEEP-----------------------------------------------
static s32 G500_play_audio_callback(u8 result, u8 *src, u16 srclen)
{
    wireless_thread_node_param_t *thread_node_param = dev_wireless_get_thread_node_param();
    s32 ret = DDI_ERR;

    switch(result)
    {
    case WIRE_AT_RESULT_OTHER:
        if(0 == strcmp(src, "+BEEP:OK"))
        {
            ret = DDI_OK;
            dev_wireless_thread_set_result(WIRE_AT_RESULT_OK);
        }
        else 
        {
            ret = DDI_ERR;
            dev_wireless_thread_set_result(WIRE_AT_RESULT_ERROR);
        }
        break;
    case WIRE_AT_RESULT_OK:
        ret = DDI_OK;
        dev_wireless_thread_set_result(result);
        break;
        
    case WIRE_AT_RESULT_TIMEROUT:
        ret = DDI_ETIMEOUT;
    case WIRE_AT_RESULT_ERROR:
        dev_wireless_thread_set_result(result);
        break;
    }
    return ret;
}
/**
 * @brief  设置GPIO
 * @param[in]
 * @param[in] 
 * @retval  DDI_OK 成功
 * @retval  DDI_ERR 失败
 */
#define ATCMD_BEEP_OK          "AT=AT+PLAYAUDIO=1,beepok.mp3,13,2,6\r\n"
#define ATCMD_BEEP_ERR         "AT=AT+PLAYAUDIO=1,beeperr.mp3,13,2,6\r\n"
static s32 G500_play_audio(u8 type, AT_API_GET_TYPE_E at_api_get_type)
{
    wireless_device_info_t *device_info = dev_wireless_get_device();
    wireless_thread_node_param_t *thread_node_param = dev_wireless_get_thread_node_param();
    s32 ret = DDI_ERR;
    s8 buff[128];
    s32 i;

    if(type == 0)
    {
        snprintf(buff, sizeof(buff), "%s", ATCMD_BEEP_OK);
    }
    else
    {
        snprintf(buff, sizeof(buff), "%s", ATCMD_BEEP_ERR);
    }
    switch(at_api_get_type)
    {
    case AT_API_NO_BLOCK:
        dev_wireless_send_at_cmd(buff, at_api_get_type, TRUE);
        dev_wireless_thread_set_timeout(WIRE_AT_CTL_TIMEOUT);
        ret = DDI_OK;
        break;
    case AT_API_BLOCK:
        dev_wireless_thread_set_param(WS_TASK_PLAYAUDIO, G500_play_audio_callback, WIRE_AT_CTL_TIMEOUT);
        dev_wireless_send_at_cmd(buff, at_api_get_type, TRUE);
        break;
    }

    if(DDI_OK == ret && AT_API_BLOCK == at_api_get_type)
    {
        while(1)
        {
            ddi_watchdog_feed();
            dev_wireless_thread_run_switch(FALSE);
            if(dev_wireless_thread_task_is_change(WS_TASK_PLAYAUDIO))
            {
                ret = DDI_EBUSY;
                break;
            }
            if(dev_wireless_thread_is_free())
            {
                if(WIRE_AT_RESULT_OK == thread_node_param->m_result)
                {
                    ret = DDI_OK;
                }
                else
                {
                    ret = DDI_ERR;
                }
                break;
            }
            dev_wireless_thread_run_switch(TRUE);
            ddi_misc_msleep(50);
        }
        dev_wireless_thread_run_switch(TRUE);
    }

    return ret;
}

//获取模块APP版本-----------------------------------------------
static s32 G500_get_appver_callback(u8 result, u8 *src, u16 srclen)
{
    wireless_thread_node_param_t *thread_node_param = dev_wireless_get_thread_node_param();
    wireless_device_info_t *device_info = dev_wireless_get_device();
    s32 ret = DDI_ERR;

    if(result == WIRE_AT_RESULT_OTHER)
    {
        if(0 == memcmp(src, "G500_", 5))
        {
            snprintf(device_info->m_module_app_ver, sizeof(device_info->m_module_app_ver), "%s", src);
            ret = DDI_OK;
        }
    }
    
    return ret;
}
/**
 * @brief  获取模块版本
 * @param[in]
 * @param[in] 
 * @retval  DDI_OK 成功
 * @retval  DDI_ERR 失败
 */
static s32 G500_get_appver(AT_API_GET_TYPE_E at_api_get_type)
{
    wireless_device_info_t *device_info = dev_wireless_get_device();
    wireless_thread_node_param_t *thread_node_param = dev_wireless_get_thread_node_param();
    s32 ret = DDI_ERR;
    s8 buff[128];
    s32 i;

    switch(at_api_get_type)
    {
        case AT_API_NO_BLOCK:
            snprintf(buff, sizeof(buff), "AT=AT+VER");
            dev_wireless_send_at_cmd(buff, at_api_get_type, TRUE);
            dev_wireless_thread_set_timeout(WIRE_AT_DEFAULT_TIMEOUT);
            break;
            
        case AT_API_BLOCK:
            ret = DDI_EINVAL;
            break;
    }

    return ret;
}
//设置休眠时间-----------------------------------------------
static s32 G500_set_sleep_time_callback(u8 result, u8 *src, u16 srclen)
{
    wireless_thread_node_param_t *thread_node_param = dev_wireless_get_thread_node_param();
    s32 ret = DDI_ERR;
    u8 buff[32];

    switch(result)
    {
        case WIRE_AT_RESULT_OK:
            ret = DDI_OK;

            if(thread_node_param->m_task_id == WS_TASK_ENTER_SLEEP)
            {
                dev_wireless_thread_set_result(result);
            }
            break;
            
        case WIRE_AT_RESULT_TIMEROUT:
            ret = DDI_ETIMEOUT;
            if(thread_node_param->m_task_id == WS_TASK_ENTER_SLEEP)
            {
                dev_wireless_thread_set_result(result);  //设置状态就好了，其他的不管
            }
            break;
    }
    return ret;
}
/**
 * @brief  模块低功耗接口
 * @param[in]
 * @param[in] 
 * @retval  DDI_OK 成功
 * @retval  DDI_ERR 失败
 */
static s32 G500_suspend(AT_API_GET_TYPE_E at_api_get_type)
{
    wireless_device_info_t *device_info = dev_wireless_get_device();
    wireless_thread_node_param_t *thread_node_param = dev_wireless_get_thread_node_param();
    s32 ret = DDI_ERR;
    
    switch(at_api_get_type)
    {
        case AT_API_NO_BLOCK:
            ret = DDI_EINVAL;
            break;
        
        case AT_API_BLOCK: //sxl?20200428这个超时时间为什么定的那么短，不用等free状态吗？
            dev_wireless_thread_set_param(WS_TASK_ENTER_SLEEP, G500_set_sleep_time_callback, WIRE_AT_SET_SLEEPTIME_TIMEOUT);
            dev_wireless_send_at_cmd("ATS24=2", at_api_get_type, TRUE);
            thread_node_param->m_lparam = 2;
            ret = DDI_OK;
            break;
    }

    if(DDI_OK == ret && AT_API_BLOCK == at_api_get_type)
    {
        while(1)
        {
            ddi_watchdog_feed();
            dev_wireless_thread_run_switch(FALSE);
            if(dev_wireless_thread_task_is_change(WS_TASK_ENTER_SLEEP))
            {
                ret = DDI_EBUSY;
                break;
            }
            if(dev_wireless_thread_is_free())
            {
                if(WIRE_AT_RESULT_OK == thread_node_param->m_result)
                {
                    ret = DDI_OK;
                }
                else
                {
                    ret = DDI_ERR;
                }
                break;
            }

            dev_wireless_thread_run_switch(TRUE);
            ddi_misc_msleep(10);
        }
		
        dev_wireless_thread_run_switch(TRUE);
    }

    return ret;
}

static s32 G500_resume_callback(u8 result, u8 *src, u16 srclen)
{
    wireless_thread_node_param_t *thread_node_param = dev_wireless_get_thread_node_param();
    s32 ret = DDI_ERR;
    s32 tmp_val = 0;

    TRACE_DBG("result:%d, step:%d", result, thread_node_param->m_step);
    switch(thread_node_param->m_step)
    {
        case 0:
            ret = G500_set_sleep_time_callback(result, src, srclen);
            if(ret == DDI_OK)
            {
                thread_node_param->m_step = 1;
                dev_wireless_send_at_cmd("ATS24?", AT_API_BLOCK, TRUE);
                dev_wireless_thread_set_timeout(WIRE_AT_SET_SLEEPTIME_TIMEOUT);
                thread_node_param->m_lparam = 2;
            }
            else if(ret == DDI_ETIMEOUT)
            {
                dev_wireless_thread_set_result(result);
            }
            break;
            
        case 1:
            if(WIRE_AT_RESULT_OTHER == result)
            {
                dev_maths_str_trim(src);
                thread_node_param->m_lparam = dev_maths_asc_to_u32(src, strlen(src), MODULUS_DEC);
            }
            else if(WIRE_AT_RESULT_OK == result)
            {
                tmp_val = thread_node_param->m_lparam;
                dev_wireless_thread_set_result(result);
                thread_node_param->m_lparam = tmp_val;
            }
            else if(WIRE_AT_RESULT_TIMEROUT == result)
            {
                dev_wireless_thread_set_result(result);
            }
            break;
    }
    return ret;
}

/**
 * @brief  模块低功耗接口
 * @param[in]
 * @param[in] 
 * @retval  DDI_OK 成功
 * @retval  DDI_ERR 失败
 */
static s32 G500_resume(AT_API_GET_TYPE_E at_api_get_type)
{
    wireless_device_info_t *device_info = dev_wireless_get_device();
    wireless_thread_node_param_t *thread_node_param = dev_wireless_get_thread_node_param();
    s32 ret = DDI_ERR;

    switch(at_api_get_type)
    {
        case AT_API_NO_BLOCK:
            ret = DDI_EINVAL;
            break;
        
        case AT_API_BLOCK:
            dev_wireless_thread_set_param(WS_TASK_WAKEUP, G500_resume_callback, WIRE_AT_SET_SLEEPTIME_TIMEOUT);
            dev_wireless_send_at_cmd("ATS24=0", at_api_get_type, TRUE);
            thread_node_param->m_lparam = 0;
            ret = DDI_OK;
            break;
    }

    if(DDI_OK == ret && AT_API_BLOCK == at_api_get_type)
    {
        while(1)
        {
            ddi_watchdog_feed();
            dev_wireless_thread_run_switch(FALSE);
            if(dev_wireless_thread_task_is_change(WS_TASK_WAKEUP))
            {
                ret = DDI_EBUSY;
                break;
            }
            if(dev_wireless_thread_is_free())
            {
                if(WIRE_AT_RESULT_OK == thread_node_param->m_result)
                {
                    TRACE_DBG("resume val:%d", thread_node_param->m_lparam);
                    if(0 == thread_node_param->m_lparam)
                    {
                        ret = DDI_OK;
                    }
                    else
                    {
                        ret = DDI_ERR;
                    }
                }
                else
                {
                    ret = DDI_ERR;
                }
                break;
            }

            dev_wireless_thread_run_switch(TRUE);
            ddi_misc_msleep(10);
        }
        
        dev_wireless_thread_run_switch(TRUE);
    }

    return ret;
}

//设置SSL模式-----------------------------------------------
static s32 G500_ssl_set_mod_callback(u8 result, u8 *src, u16 srclen)
{
    wireless_device_info_t *device_info = dev_wireless_get_device();
    wireless_thread_node_param_t *thread_node_param = dev_wireless_get_thread_node_param();
    s32 ret = DDI_ERR;

    switch(result)
    {
        case WIRE_AT_RESULT_OK:
            ret = DDI_OK;
            dev_wireless_thread_set_result(result);
            break;
            
        case WIRE_AT_RESULT_TIMEROUT:
            ret = DDI_ETIMEOUT;
        case WIRE_AT_RESULT_ERROR:
            dev_wireless_thread_set_result(result);
            break;
    }
    return ret;
}
/**
 * @brief 设置SSL的模式
 * @param[in]
 * @param[in] 
 * @retval  DDI_OK 成功
 * @retval  DDI_ERR 失败
 */
static s32 G500_ssl_set_mod(SSL_AUTHEN_MOD_t mod, AT_API_GET_TYPE_E at_api_get_type)
{
    wireless_device_info_t *device_info = dev_wireless_get_device();
    wireless_thread_node_param_t *thread_node_param = dev_wireless_get_thread_node_param();
    s32 ret = DDI_ERR;
    s8 buff[128];
    s32 i;

    switch(at_api_get_type)
    {
        case AT_API_NO_BLOCK:
            ret = DDI_EINVAL;
            break;
        case AT_API_BLOCK:
            if(mod > SSL_AUTHEN_MOD_BIDIRECT)
            {
                //参数错
                ret = DDI_EINVAL;
                break;
            }
            if((mod == SSL_AUTHEN_MOD_TRUST)
             ||(mod == SSL_AUTHEN_MOD_BIDIRECT))
            {
                i = 1;      //需要认证
             }
             else
             {
                i = 0;
             }
            snprintf(buff, sizeof(buff), "AT+GTSSLMODE=%d\r\n", i);
            dev_wireless_send_at_cmd(buff, at_api_get_type, TRUE);
            dev_wireless_thread_set_param(WS_TASK_SSL_SETMOD, G500_ssl_set_mod_callback, WIRE_AT_DEFAULT_TIMEOUT);
            ret = DDI_OK;
            break;
    }

    if(DDI_OK == ret && AT_API_BLOCK == at_api_get_type)
    {
        while(1)
        {
            ddi_watchdog_feed();
            dev_wireless_thread_run_switch(FALSE);
            if(dev_wireless_thread_task_is_change(WS_TASK_SSL_SETMOD))
            {
                ret = DDI_EBUSY;
                break;
            }
            if(dev_wireless_thread_is_free())
            {
                if(WIRE_AT_RESULT_OK == thread_node_param->m_result)
                {
                    ret = DDI_OK;
                    device_info->m_ssltype = mod;
                }
                else
                {
                    ret = DDI_ERR;
                }
                break;
            }
            dev_wireless_thread_run_switch(TRUE);
            ddi_misc_msleep(50);
        }
		
        dev_wireless_thread_run_switch(TRUE);
    }

    return ret;
}
//设置SSL版本-----------------------------------------------
static s32 G500_ssl_setver_callback(u8 result, u8 *src, u16 srclen)
{
    wireless_device_info_t *device_info = dev_wireless_get_device();
    wireless_thread_node_param_t *thread_node_param = dev_wireless_get_thread_node_param();
    s32 ret = DDI_ERR;

    switch(result)
    {
        case WIRE_AT_RESULT_OK:
            ret = DDI_OK;
            dev_wireless_thread_set_result(result);
            break;
            
        case WIRE_AT_RESULT_TIMEROUT:
            ret = DDI_ETIMEOUT;
        case WIRE_AT_RESULT_ERROR:
            dev_wireless_thread_set_result(result);
            break;
    }
    return ret;
}
/**
 * @brief 设置SSL的版本
 * @param[in]
 * @param[in] 
 * @retval  DDI_OK 成功
 * @retval  DDI_ERR 失败
 */
static s32 G500_ssl_setver(u8 ver, AT_API_GET_TYPE_E at_api_get_type)
{
    wireless_device_info_t *device_info = dev_wireless_get_device();
    wireless_thread_node_param_t *thread_node_param = dev_wireless_get_thread_node_param();
    s32 ret = DDI_ERR;
    s8 buff[128];
    s32 i;

    switch(at_api_get_type)
    {
        case AT_API_NO_BLOCK:
            ret = DDI_EINVAL;
            break;
        case AT_API_BLOCK:
            snprintf(buff, sizeof(buff), "AT+GTSSLVER=%d\r\n", ver);
            dev_wireless_send_at_cmd(buff, at_api_get_type, TRUE);
            dev_wireless_thread_set_param(WS_TASK_SSL_SETVER, G500_ssl_setver_callback, WIRE_AT_DEFAULT_TIMEOUT);
            ret = DDI_OK;
            break;
    }

    if(DDI_OK == ret && AT_API_BLOCK == at_api_get_type)
    {
        while(1)
        {
            ddi_watchdog_feed();
            dev_wireless_thread_run_switch(FALSE);
            if(dev_wireless_thread_task_is_change(WS_TASK_SSL_SETVER))
            {
                ret = DDI_EBUSY;
                break;
            }
            if(dev_wireless_thread_is_free())
            {
                if(WIRE_AT_RESULT_OK == thread_node_param->m_result)
                {
                    ret = DDI_OK;
                }
                else
                {
                    ret = DDI_ERR;
                }
                break;
            }

            dev_wireless_thread_run_switch(TRUE);
            ddi_misc_msleep(50);
        }
        dev_wireless_thread_run_switch(TRUE);
    }

    return ret;
}

static s32 G500_ssl_filte_callback(u8 result, u8 *src, u16 srclen)
{
    wireless_device_info_t *device_info = dev_wireless_get_device();
    wireless_thread_node_param_t *thread_node_param = dev_wireless_get_thread_node_param();
    s32 ret = DDI_ERR;

    switch(result)
    {
        case WIRE_AT_RESULT_OK:
            ret = DDI_OK;
            dev_wireless_thread_set_result(result);
            break;

        case WIRE_AT_RESULT_TIMEROUT:
            ret = DDI_ETIMEOUT;
            dev_wireless_thread_set_result(result);
            break;

        case WIRE_AT_RESULT_ERROR:
            ret = DDI_ERR;
            dev_wireless_thread_set_result(result);
            break;
    }

    return ret;
}

static s32 G500_ssl_filte(s32 type, AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;
    u8 buff[32];
    wireless_thread_node_param_t *thread_node_param = dev_wireless_get_thread_node_param();

    switch(at_api_get_type)
    {
        case AT_API_NO_BLOCK:
            break;

        case AT_API_BLOCK:
            memset(buff, 0, sizeof(buff));
            snprintf(buff, sizeof(buff), "AT+GTSSLCIPHER=%d,0", type);
            dev_wireless_send_at_cmd(buff, at_api_get_type, TRUE);
            dev_wireless_thread_set_param(WS_TASK_SSL_FILTE, G500_ssl_filte_callback, WIRE_AT_DEFAULT_TIMEOUT);
            ret = DDI_OK;
            break;
    }

    if(DDI_OK == ret && AT_API_BLOCK == at_api_get_type)
    {
        while(1)
        {
            if(dev_wireless_thread_task_is_change(WS_TASK_SSL_FILTE))
            {
                ret = DDI_EBUSY;
                break;
            }
            if(dev_wireless_thread_is_free())
            {
                if(WIRE_AT_RESULT_OK == thread_node_param->m_result)
                {
                    ret = DDI_OK;
                }
                else
                {
                    ret = DDI_ERR;
                }
                break;
            }
            ddi_misc_msleep(50);
        }
    }

    return ret;
}

//设置SSL证书-----------------------------------------------
static s32 G500_ssl_setcert_callback(u8 result, u8 *src, u16 srclen)
{
    wireless_device_info_t *device_info = dev_wireless_get_device();
    wireless_thread_node_param_t *thread_node_param = dev_wireless_get_thread_node_param();
    s32 ret = DDI_ERR;

    switch(result)
    {
        case WIRE_AT_RESULT_OTHER:
            if(0 == thread_node_param->m_step)
            {
                if(0 == strcmp(src, ">"))
                {
                    thread_node_param->m_param1 = 1;
                    dev_wireless_thread_set_result(WIRE_AT_RESULT_OK);
                    ret = DDI_OK;
                }
            }
            break;
        case WIRE_AT_RESULT_OK:
            if(1 == thread_node_param->m_step)
            {
                thread_node_param->m_param1 = 0;
                dev_wireless_thread_set_result(WIRE_AT_RESULT_OK);
                ret = DDI_OK;
            }
            break;
        case WIRE_AT_RESULT_ERROR:
            dev_wireless_thread_set_result(result);
            ret = DDI_ERR;
            break;
        case WIRE_AT_RESULT_TIMEROUT:
            dev_wireless_thread_set_result(result);
            ret = DDI_ETIMEOUT;
            break;
    }
    return ret;
}
#define SSL_CERTFILE_TYPE_MAX   3
const u8 AT_GTSSLFILE_STR[SSL_CERTFILE_TYPE_MAX][11] = 
                                {"TRUSTFILE",
                                   "CERTFILE",
                                   "KEYFILE",
                                };
static s32 G500_ssl_setcert(u8 certtype, u8 *cert, u32 certlen, AT_API_GET_TYPE_E at_api_get_type)
{
    wireless_device_info_t *device_info = dev_wireless_get_device();
    wireless_thread_node_param_t *thread_node_param = dev_wireless_get_thread_node_param();
    s32 ret = DDI_ERR;
    s8 buff[128];

    
    switch(at_api_get_type)
    {
        case AT_API_NO_BLOCK:
            ret = DDI_EINVAL;
            break;
        case AT_API_BLOCK:
            if(certtype>=SSL_CERTFILE_TYPE_MAX)
            {
                ret = DDI_EINVAL;
                break;
            }
            snprintf(buff, sizeof(buff), "AT+GTSSLFILE=\"%s\",%d", AT_GTSSLFILE_STR[certtype], strlen(cert));
            dev_wireless_send_at_cmd(buff, at_api_get_type, TRUE);
            dev_wireless_thread_set_param(WS_TASK_SSL_SETCERT, G500_ssl_setcert_callback, WIRE_AT_DEFAULT_TIMEOUT);
            ret = DDI_OK;
            break;
    }

    if(DDI_OK == ret && AT_API_BLOCK == at_api_get_type)
    {
        while(1)
        {
            ddi_watchdog_feed();
            dev_wireless_thread_run_switch(FALSE);
            if(dev_wireless_thread_task_is_change(WS_TASK_SSL_SETCERT))
            {
                ret = DDI_EBUSY;
                break;
            }
            if(dev_wireless_thread_is_free())
            {
                if(WIRE_AT_RESULT_OK == thread_node_param->m_result)
                {
                    ret = DDI_OK;
                }
                else
                {
                    ret = DDI_ERR;
                }
                break;
            }
            dev_wireless_thread_run_switch(TRUE);
            ddi_misc_msleep(50);
        }

        dev_wireless_thread_run_switch(TRUE);
        if(ret == DDI_OK)
        {
            //发送证书内容
            dev_wireless_thread_set_param(WS_TASK_SSL_SETCERT, G500_ssl_setcert_callback, WIRE_AT_DEFAULT_TIMEOUT);
            thread_node_param->m_step = 1;
            dev_wireless_send_at_cmd(cert, at_api_get_type, FALSE);
            while(1)
            {
                if(dev_wireless_thread_task_is_change(WS_TASK_SSL_SETCERT))
                {
                    ret = DDI_EBUSY;
                    break;
                }
                if(dev_wireless_thread_is_free())
                {
                    if(WIRE_AT_RESULT_OK == thread_node_param->m_result)
                    {
                        ret = DDI_OK;
                    }
                    else
                    {
                        ret = DDI_ERR;
                    }
                    break;
                }
                ddi_misc_msleep(50);
            }
        }
    }
    return ret;
}
#if 0
static s32 G500_ssl_open(s32 socketid)
{
    wireless_device_info_t *device_info = dev_wireless_get_device();
    
    if(socketid >= WIRELESS_SOCKET_MAX)
    {
        return DDI_EDATA;
    }
    if(device_info->m_socket_info[socketid] == NULL)
    {
         return DDI_ERR;
    }
    if(device_info->m_socket_info[socketid]->m_status != GPRS_STATUS_CONNECTED)
    {
       return DDI_ERR;
    }
    //记录socketid;
    device_info->m_sslsocket = socketid;
    return DDI_OK;
}
static s32 G500_ssl_close(s32 socketid)
{
    wireless_device_info_t *device_info = dev_wireless_get_device();
    
    device_info->m_sslsocket = 0xFF;
    device_info->m_ssltype = 0;
    return DDI_OK;
}
#endif
//-----------------------------------------------------
static s32 G500_net_reg_report_handle(s8 *buff)
{
    s8 *kw = g_current_module->m_get_net_reg_res_kw;
    s32 ret = DDI_ERR;
    wireless_device_info_t *wireless_device = dev_wireless_get_device();

    if(kw)
    {
        if(strstr(buff, kw))
        {
            if(!G500_get_net_reg_callback(WIRE_AT_RESULT_OTHER, buff, strlen(buff)))
            {
                TRACE_INFO("net drop:%s, retry to active pdp", buff);
                dev_wireless_set_control_strategy_step(WS_SEARCHING_NETWORK);
                memset(wireless_device->m_local_ip, 0, sizeof(wireless_device->m_local_ip));
                dev_wireless_set_socket_status(0xff, GPRS_STATUS_DISCONNECTED);
                ret = DDI_OK;
            }
        }
    }
    
    return ret;
}

static s32 G500_pdp_report_handle(s8 *buff)
{
    s32 ret = DDI_ERR;
    s8 *pstr = NULL;
    wireless_device_info_t *wireless_device = dev_wireless_get_device();
    s8 *key_words = g_current_module->m_pdp_report_head_kw;
    s32 tmp_val = 0;

    do{
        if(!key_words)
        {
            break;
        }

        tmp_val = G500_handle_ip_at_res_data(buff, strlen(buff));
        switch(tmp_val)
        {
            case 0:           //断开
                TRACE_INFO("net drop, retry to active pdp");
                dev_wireless_set_control_strategy_step(WS_INACTIVE_PDP);
                memset(wireless_device->m_local_ip, 0, sizeof(wireless_device->m_local_ip));
                dev_wireless_set_socket_status(0xff, GPRS_STATUS_DISCONNECTED);
                ret = DDI_OK;
                break;

            case 1:           //激活成功
                TRACE_INFO("active net success, local ip:%s", wireless_device->m_local_ip);
                dev_wireless_set_control_strategy_step(WS_ACTIVE_PDP_SUCCESS);
                //dev_wireless_thread_set_result(0);
                ret = DDI_OK;
                break;

            case 2:           //正在激活或正在关闭pdp
                break;
        }
    }while(0);

    return ret;
}

static s32 G500_tcp_connect_report_handle(s8 *buff)
{
    s32 ret = DDI_ERR;
    s8 *pstr = NULL;
    wireless_device_info_t *wireless_device = dev_wireless_get_device();
    s8 *key_words = g_current_module->m_connect_report_head_kw;
    s8 dst[2][SPLIT_PER_MAX];
    s32 socket_sn = 0;

    do{
        if(!key_words)
        {
            break;
        }

        pstr = strstr(buff, key_words);
        if(pstr)
        {
            pstr += strlen(key_words);
            memset(dst, 0, sizeof(dst));
            ret = dev_maths_split(dst, sizeof(dst)/sizeof(dst[0]), pstr, ",", SPLIT_IGNOR_LEN0);
            if(sizeof(dst)/sizeof(dst[0]) == ret)
            {
                dev_maths_str_trim(dst[0]);
                dev_maths_str_trim(dst[1]);

                if(dev_maths_asc_to_u32(dst[0], strlen(dst[0]), MODULUS_DEC) > WIRELESS_SOCKET_MAX)
                {
                    TRACE_ERR("socket id is error:%d", dev_maths_asc_to_u32(dst[0], strlen(dst[0]), MODULUS_DEC));
                }
                else
                {
                    socket_sn = dev_maths_asc_to_u32(dst[0], strlen(dst[0]), MODULUS_DEC)-1;
                    if(wireless_device->m_socket_info[socket_sn])
                    {
                        wireless_device->m_socket_info[socket_sn]->m_status = dev_maths_asc_to_u32(dst[1], strlen(dst[1]), MODULUS_DEC)?GPRS_STATUS_CONNECTED:GPRS_STATUS_CONNECTING;
                        if(GPRS_STATUS_CONNECTED == wireless_device->m_socket_info[socket_sn]->m_status)
                        {
                            TRACE_INFO("sn:%d connect success", socket_sn);
                        }
                        else
                        {
                            TRACE_INFO("sn:%d, status call back is %d", 
                                                            socket_sn,
                                                            wireless_device->m_socket_info[socket_sn]->m_status);
                        }
                    }
                }
            }
            ret = DDI_OK;
            break;
        }
    }while(0);

    return ret;
}

static s32 G500_socket_report_handle(s8 *buff)
{
    s32 ret = DDI_ERR;
    s8 *pstr = NULL;
    wireless_device_info_t *wireless_device = dev_wireless_get_device();
    s8 *key_words = g_current_module->m_socket_report_head_kw;
    s8 dst[2][SPLIT_PER_MAX];
    s32 socket_id = 0;

    do{
        if(!key_words)
        {
            break;
        }

        pstr = strstr(buff, key_words);
        if(pstr)
        {
            pstr += strlen(key_words);
            ret = dev_maths_split(dst, sizeof(dst)/sizeof(dst[0]), pstr, ",", SPLIT_IGNOR_LEN0);
            if(ret > 0)
            {
                dev_maths_str_trim(dst[0]);
                dev_maths_str_trim(dst[1]);

                socket_id = dev_maths_asc_to_u32(dst[0], strlen(dst[0]), MODULUS_DEC);
                if(dev_maths_asc_to_u32(dst[1], strlen(dst[1]), MODULUS_DEC))
                {
                    TRACE_INFO("socket close by peer, infor:%s, close socketid:%d", buff, socket_id);
                    dev_wireless_set_socket_status(socket_id, GPRS_STATUS_DISCONNECTED);
                }
            }
            
            ret = DDI_OK;
            break;
        }
    }while(0);

    return ret;
}

#if FIBOCOM_G500_SUPPORT
wireless_module_t* FIBOCOM_G500_module(void)
{
    g_current_module = &g_G500;
    g_G500.m_id = FIBOCOM_G500;

    g_G500.m_support_ota = FALSE;
    g_G500.m_with_flow_control = TRUE;
    g_G500.cfg_flow_control = G500_cfg_flow_control;
    
    g_G500.open_airplane = G500_open_airplane;

    g_G500.close_airplane = G500_close_airplane;
    g_G500.telephone_dial = G500_telephone_dial;
    g_G500.telephone_hangup = G500_telephone_hangup;
    g_G500.cfg_ntp_server = G500_cfg_ntp_server;
    g_G500.qry_rtc = G500_qry_rtc;
    
    g_G500.m_get_cellinfo_cmd = "AT+GTCCINFO?";
    g_G500.get_cellinfo = G500_get_cellinfo;
    g_G500.get_cellinfo_callback = G500_get_cellinfo_callback;
    
    g_G500.m_power_off_cmd = "AT+CFUN=0";
    g_G500.power_off = G500_power_off;
    g_G500.power_off_callback = G500_power_off_callback;
    
    g_G500.socket_create = G500_socket_create;
    g_G500.get_valid_socketid_list = G500_get_valid_socketid_list;
    g_G500.socket_send = G500_socket_send;
    g_G500.socket_close = G500_socket_close;
    
    g_G500.get_set_apn_cmd = G500_get_set_apn_cmd;
    g_G500.set_apn = G500_set_apn;
    g_G500.set_apn_callback = G500_set_apn_callback;

    g_G500.get_inactive_pdp_cmd = "AT+MIPCALL=0";
    g_G500.inactive_pdp = G500_inactive_pdp;
    
    g_G500.m_get_local_ip_res_kw = "+MIPCALL:";
    g_G500.m_get_local_ip_cmd = "AT+MIPCALL?";
    g_G500.get_local_ip = G500_get_local_ip;
    g_G500.get_local_ip_callback = G500_get_local_ip_callback;
    
    g_G500.m_cfg_rec_data_format_cmd = "AT+GTSET=\"IPRFMT\",2";
    g_G500.cfg_rec_data_format = G500_cfg_rec_data_format;
    g_G500.cfg_rec_data_format_callback = G500_cfg_rec_data_format_callback;

    g_G500.m_check_rplmn_cmd = "AT+CRSM=176,28542,0,0,11";
    g_G500.check_rplmn = G500_check_rplmn;
    g_G500.check_rplmn_callback = G500_check_rplmn_callback;

    g_G500.clear_rplmn = G500_clear_rplmn;
    g_G500.m_clear_rplmn_cmd = "AT+CRSM=214,28542,0,4,3,\"FFFFFF\"";
    
    g_G500.m_imei_cmd = "AT+CGSN";
    g_G500.get_imei = G500_get_imei;
    g_G500.get_imei_callback = G500_get_imei_callback;

    g_G500.m_check_workmod_cmd = "AT+GTRAT?";
    g_G500.check_work_mode = G500_check_work_mode;
    g_G500.check_work_mode_callback = L610_get_work_mode_callback;

    g_G500.m_ccid_cmd = "AT+CCID";
    g_G500.m_ccid_cmd_res_kw = "+CCID:";
    g_G500.get_ccid = G500_get_ccid;
    g_G500.get_ccid_callback = G500_get_ccid_callback;

    g_G500.m_simcard_state_cmd = "AT+CPIN?";
    g_G500.m_simcard_state_cmd_res_kw = "+CPIN: READY";
    g_G500.get_simcard_state = G500_get_simcard_state;
    g_G500.get_simcard_state_callback = G500_get_simcard_state_callback;

    g_G500.m_get_csq_cmd = "AT+CSQ?";
    g_G500.m_get_csq_res_kw = "+CSQ:";
    g_G500.get_csq = G500_get_csq;
    g_G500.get_csq_callback = G500_get_csq_callback;

    g_G500.m_get_2G_net_reg_cmd = "AT+CGREG?";
    g_G500.m_get_4G_net_reg_cmd = "AT+CEREG?";
    g_G500.m_get_net_reg_res_kw = "REG:";
    g_G500.get_net_reg = G500_get_net_reg;
    g_G500.get_net_reg_callback = G500_get_net_reg_callback;

    g_G500.m_set_2G_reg_param_cmd = "AT+CGREG=2";
    g_G500.m_set_4G_reg_param_cmd = "AT+CEREG=2";
    g_G500.get_reg_cmd = G500_get_reg_cmd;
    g_G500.set_reg_param = G500_set_reg_param;
    g_G500.get_set_reg_param_cmd = G500_get_set_reg_param_cmd;

    g_G500.m_get_imsi_cmd = "AT+CIMI";
    g_G500.get_imsi = G500_get_imsi;
    g_G500.get_imsi_callback = G500_get_imsi_callback;
    
    g_G500.m_tcp_rec_cmd_head_kw = "+MIPRTCP:";

    g_G500.m_pdp_report_head_kw = "+MIPCALL:";
    g_G500.m_connect_report_head_kw = "+MIPOPEN:";
    g_G500.m_socket_report_head_kw = "+MIPSTAT:";

    g_G500.net_reg_report_handle = G500_net_reg_report_handle;
    g_G500.pdp_report_handle = G500_pdp_report_handle;
    g_G500.tcp_connect_report_handle = G500_tcp_connect_report_handle;
    g_G500.socket_report_handle = G500_socket_report_handle;

    g_G500.cfg_exGPIO  =  G500_cfg_exGPIO; 
    g_G500.set_exGPIO  =  G500_set_exGPIO; 
    g_G500.play_audio   =  G500_play_audio;  
    g_G500.get_appver   =  G500_get_appver;  
    g_G500.get_appver_callback = G500_get_appver_callback;
    g_G500.suspend = G500_suspend;
    g_G500.resume = G500_resume;
    g_G500.ssl_set_mod  =  G500_ssl_set_mod; 
    g_G500.ssl_setver  =  G500_ssl_setver; 
    g_G500.ssl_setcert =  G500_ssl_setcert;
    
    return &g_G500;
}
#endif
static wireless_module_t g_L610;

/*****************************************************************************
*                                                                            *
*  @file     module_FIBOCOM.C                                                *
*  @brief    此文件主要是设计广和通模块的接口                                                *
*  Details.                                                                  *
*                                                                            *
*  @author   xiaohonghui                                                     *
*  Remark         : Description                                              *
*----------------------------------------------------------------------------*
*  Change History :                                                          *
*  <Date>     | <Version> | <Author>       | <Description>                   *
*----------------------------------------------------------------------------*
*  2020/04/11 | 1.0.0.1   | xiaohonghui    | Create file                     *
*----------------------------------------------------------------------------*
*                                                                            *
*****************************************************************************/

/**
 * @brief 模块关机或复位的处理回到
 * @param[in] result:
 * @param[in] src:
 * @param[in] srclen:
 * @retval  DDI_OK:第一步操作成功
 * @retval  DDI_ETIMEOUT:第二步操作完成，可执行关机或复位操作
 */
static s32 L610_power_off_callback(u8 result, u8 *src, u16 srclen)
{
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();
    s32 ret = DDI_ERR;
    s8 *pstr = NULL;

    switch(result)
    {
        case WIRE_AT_RESULT_OK:
            thread_node_param->m_lparam = 1;
            ret = DDI_OK;

            if(WS_TASK_POWEROFF_MODULE == thread_node_param->m_task_id)  //关机收到ok后等2秒关机
            {
                dev_wireless_thread_set_timeout(WIRE_AT_CMD_RESEND_TIMEOUT*2);
            }
            else if(WS_TASK_RESET_MODULE == thread_node_param->m_task_id)  //复位模块按手册建议等12秒---后和袁工沟通，还是用2秒
            {
                dev_wireless_thread_set_timeout(WIRE_AT_CMD_RESEND_TIMEOUT*2);
            }
            break;

        case WIRE_AT_RESULT_TIMEROUT:
            if(0 == thread_node_param->m_lparam)   //cfun=0指令没返回的第一次超时处理，通过拉管脚控制
            {
                TRACE_ERR("cfun=0 exec timeout, then power off use gpio");
                
                if(WIRELESS_WITH_POWERKEY)   //有电源控制引脚
                {
                    dev_gpio_set_value(GPRS_POWERON_PIN, 0);
                    dev_wireless_thread_set_timeout(WIRE_AT_POWEROFF_WAIT_TIMEOUT+WIRE_AT_CTL_TIMEOUT);
                    thread_node_param->m_lparam = 2;
                }
                else  //没有电源控制脚，直接下电
                {
                    dev_gpio_set_value(GPRS_POWEREN_PIN, 0);
                    dev_wireless_set_power_flag(FALSE);
                    dev_wireless_thread_set_timeout(WIRE_AT_POWEROFF_WAIT_TIMEOUT+WIRE_AT_CTL_TIMEOUT);
                    thread_node_param->m_lparam = 2;
                }
                ret = DDI_OK;
            }
            else if(1 == thread_node_param->m_lparam)  //cfun=0指令有返回，超时后可执行复位或重启
            {
                TRACE_ERR("finish power off use cfun=0");
                ret = DDI_ETIMEOUT;
                if(WIRELESS_WITH_POWERKEY)   //有电源控制引脚
                {
                    dev_gpio_set_value(GPRS_POWERON_PIN, 1);
                    dev_gpio_set_value(GPRS_POWEREN_PIN, 0);
                    dev_wireless_set_power_flag(FALSE);
                }
                else
                {
                    dev_gpio_set_value(GPRS_POWEREN_PIN, 0);
                    dev_wireless_set_power_flag(FALSE);
                }
            }
            else  if(2 == thread_node_param->m_lparam) //cfun=0指令没返回后的第二次超时处理
            {
                if(WIRELESS_WITH_POWERKEY)   //有电源控制引脚
                {
                    TRACE_ERR("[have power] finish power off gpio step 1");
                    dev_gpio_set_value(GPRS_POWERON_PIN, 1);
                    dev_gpio_set_value(GPRS_POWEREN_PIN, 0);
                    dev_wireless_set_power_flag(FALSE);
                    ret = DDI_ETIMEOUT;
                }
                else  //没有电源控制脚，直接下电
                {
                    TRACE_ERR("finish power off gpio");
                    ret = DDI_ETIMEOUT;
                }
            }
            break;
    }

    if(DDI_ETIMEOUT == ret)
    {
        dev_wireless_thread_set_result(result);

        //关机成功后停止线程运行
        if(WS_TASK_POWEROFF_MODULE == thread_node_param->m_task_id)
        {
            dev_wireless_thread_run_switch(FALSE);
        }
    }

    return ret;
}

s32 L610_power_off(AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;
    s8 buff[128];
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();

    switch(at_api_get_type)
    {
        case AT_API_NO_BLOCK:
            ret = dev_wireless_send_at_cmd(g_L610.m_power_off_cmd, AT_API_NO_BLOCK, TRUE);
            if(ret >= 0)
            {
                dev_wireless_thread_set_timeout(WIRE_AT_CMD_RESEND_TIMEOUT);
                ret = DDI_OK;
            }
            break;

        case AT_API_BLOCK:
            dev_wireless_send_at_cmd(g_L610.m_power_off_cmd, at_api_get_type, TRUE);
            dev_wireless_thread_set_param(WS_TASK_POWEROFF_MODULE, L610_power_off_callback, WIRE_AT_CMD_RESEND_TIMEOUT);
            while(1)
            {
                if(dev_wireless_thread_task_is_change(WS_TASK_POWEROFF_MODULE))
                {
                    ret = DDI_EBUSY;
                    break;
                }
                
                if(dev_wireless_thread_is_free())
                {
                    switch(thread_node_param->m_result)
                    {
                        case WIRE_AT_RESULT_TIMEROUT:
                            ret = DDI_OK;
                            break;
                    }

                    break;
                }

                ddi_misc_msleep(50);
            }
            break;
    }

    return ret;
}

static s32 L610_cfg_flow_control(AT_API_GET_TYPE_E at_api_get_type)
{
    wireless_device_info_t *device_info = dev_wireless_get_device();
    s32 ret = DDI_ERR;

    switch(at_api_get_type)
    {
        case AT_API_NO_BLOCK:
            ret = dev_wireless_send_at_cmd("AT&K3", at_api_get_type, TRUE);
            if(ret >= 0)
            {
                dev_wireless_thread_set_timeout(WIRE_AT_DEFAULT_TIMEOUT);
                ret = DDI_OK;
            }
            break;

        case AT_API_BLOCK:
            break;
    }

    return ret;
}

static s32 L610_send_data_callback(u8 result, u8 *src, u16 srclen)
{
    TRACE_DBG("result:%d, src:%s", result, src);
    if(WIRE_AT_RESULT_OTHER == result)
    {
        
    }
    else if(WIRE_AT_RESULT_ERROR == result || WIRE_AT_RESULT_TIMEROUT == result || WIRE_AT_RESULT_OK == result)
    {
        dev_wireless_thread_set_result(result);

        if(WIRE_AT_RESULT_TIMEROUT == result)
        {
            dev_wireless_set_control_strategy_step(WS_CHECK_CMD);
        }
    }

    return result;
}

static s32 L610_send_data(s32 socket_id, const u8 *wbuf, s32 wlen)
{
    s32 ret = DDI_ERR;
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();

    dev_com_write(WIRELESS_PORT, (u8 *)wbuf, wlen);
    dev_wireless_thread_set_param(WS_TASK_SOCKET_SENDBUFF, L610_send_data_callback, WIRE_AT_CMD_RESEND_TIMEOUT*5);
    while(1)
    {
        ddi_watchdog_feed();
        dev_wireless_thread_run_switch(FALSE);
        if(dev_wireless_thread_task_is_change(WS_TASK_SOCKET_SENDBUFF))
        {
            ret = DDI_EBUSY;
            break;
        }
        
        if(dev_wireless_thread_is_free())
        {
            switch(thread_node_param->m_result)
            {
                case WIRE_AT_RESULT_OK:
                    TRACE_INFO("send this pack data ok");
                    ret = wlen;
                    break;

                case WIRE_AT_RESULT_ERROR:
                    ret = DDI_ERR;
                    break;

                case WIRE_AT_RESULT_TIMEROUT:
                    ret = DDI_ETIMEOUT;
                    break;
            }

            break;
        }

        dev_wireless_thread_run_switch(TRUE);
        ddi_misc_msleep(50);
    }

    dev_wireless_thread_run_switch(TRUE);
    TRACE_DBG("out");
    return ret;
}

static s32 L610_send_datalen_callback(u8 result, u8 *src, u16 srclen)
{
    TRACE_DBG("result:%d, src:%s", result, src);
    if(WIRE_AT_RESULT_OTHER == result)
    {
        if('>' == src[0])
        {
            dev_wireless_thread_set_result(WIRE_AT_RESULT_OK);
        }
    }
    else if(WIRE_AT_RESULT_ERROR == result || WIRE_AT_RESULT_TIMEROUT == result)
    {
        dev_wireless_thread_set_result(result);

        if(WIRE_AT_RESULT_TIMEROUT == result)
        {
            dev_wireless_set_control_strategy_step(WS_CHECK_CMD);
        }
    }

    return result;
}

static s32 L610_send_datalen(s32 socket_id, s32 wlen)
{
    s8 buff[128] = {0};
    s32 ret = DDI_ERR;
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();
    
    memset(buff, 0, sizeof(buff));
    snprintf(buff, sizeof(buff), "AT+MIPSEND=%d,%d", socket_id, wlen);
    dev_wireless_send_at_cmd(buff, AT_API_BLOCK, TRUE);
    dev_wireless_thread_set_param(WS_TASK_SOCKET_SENDBUFF_LEN, L610_send_datalen_callback, WIRE_AT_CMD_RESEND_TIMEOUT*5);
    while(1)
    {
        ddi_watchdog_feed();
        dev_wireless_thread_run_switch(FALSE);
        if(dev_wireless_thread_task_is_change(WS_TASK_SOCKET_SENDBUFF_LEN))
        {
            ret = DDI_EBUSY;
            break;
        }
        
        if(dev_wireless_thread_is_free())
        {
            switch(thread_node_param->m_result)
            {
                case WIRE_AT_RESULT_OK:
                    ret = DDI_OK;
                    break;

                case WIRE_AT_RESULT_ERROR:
                    ret = DDI_ERR;
                    break;

                case WIRE_AT_RESULT_TIMEROUT:
                    ret = DDI_ETIMEOUT;
                    break;
            }

            break;
        }
        
        dev_wireless_thread_run_switch(TRUE);

        ddi_misc_msleep(50);
    }

    dev_wireless_thread_run_switch(TRUE);

    return ret;
}

static s32 L610_socket_send_onepacket(s32 socket_id, const u8 *wbuf, s32 wlen)
{
    s32 ret;
    u32 timerid;
    u8 sendflg;
    u8 step;
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();
    s8 tmp_buff[64];

    do{
        ret = L610_send_datalen(socket_id, wlen);
        if(DDI_OK != ret)
        {
            TRACE_ERR("send datalen fail:%d", ret);
            break;
        }

        ret = L610_send_data(socket_id, wbuf, wlen);
        if(ret < DDI_OK)
        {
            TRACE_ERR("send this pack data fail:%d", ret);
            break;
        }
    }while(0);

    TRACE_DBG("out");
    return ret;
}

static s32 L610_socket_send(s32 socket_id, const u8 *wbuf, s32 wlen, AT_API_GET_TYPE_E at_api_get_type)
{
    s32 i,j;
    s32 ret = 0;
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();

    if(AT_API_BLOCK == at_api_get_type)
    {
        i = 0;
        while(i<wlen)
        {
            if((i+WIRELESS_SOCKET_SEND_BUFLEN) > wlen)
            {
                j = wlen - i;
            }
            else
            {
                j = WIRELESS_SOCKET_SEND_BUFLEN;
            }
            
            ret = L610_socket_send_onepacket(socket_id, &wbuf[i], j);
            if(ret < 0)
            {
                break;
            }
            else
            {
                i += j;
            }
        }
    }

    if(ret < 0)
    {
        return ret;
    }
    TRACE_DBG("out");
    return i;
}

static s32 L610_set_work_mode_nb(AT_API_GET_TYPE_E at_api_get_type)
{
    wireless_device_info_t *device_info = dev_wireless_get_device();
    s32 ret = DDI_ERR;

    switch(at_api_get_type)
    {
        case AT_API_NO_BLOCK:
            if(FIBOCOM_L610 == g_current_module->m_id)
            {
                ret = dev_wireless_send_at_cmd(g_current_module->m_set_workmod_cmd, at_api_get_type, TRUE);
                if(ret >= 0)
                {
                    dev_wireless_thread_set_timeout(WIRE_AT_DEFAULT_TIMEOUT);
                    ret = DDI_OK;
                }
            }
            else
            {
                ret = TRUE;
            }
            break;

        case AT_API_BLOCK:
            break;
    }

    return ret;
}

#if(WIRELESS_USE_EXTERANL_AMP == TRUE)
static s32 L610_ttsaudio_play_callback(u8 result, u8 *src, u16 srclen)
{
    wireless_thread_node_param_t *thread_node_param = dev_wireless_get_thread_node_param();
    s32 ret = DDI_ERR;

    TRACE_DBG("result:%d, src:%s", result, src);
    switch(result)
    {
        case WIRE_AT_RESULT_OTHER:
            if(strstr(src, "GTTOK"))
            {
                dev_wireless_thread_set_result(WIRE_AT_RESULT_OK);
            }
            else if(strstr(src, "GTTERROR"))
            {
                dev_wireless_thread_set_result(WIRE_AT_RESULT_ERROR);
            }
            break;
            
        case WIRE_AT_RESULT_TIMEROUT:
            dev_wireless_thread_set_result(result);
            break;
    }
}

static s32 L610_ttsaudio_play(TTS_ENCODE_TYPE ttsencode, u8 *ttstext, u32 ttstextlen)
{
    wireless_thread_node_param_t *thread_node_param = dev_wireless_get_thread_node_param();
    s32 ret = DDI_ERR;
    s8 buff[256];

    //再播报新的内容
    memset(buff,0,sizeof(buff));
    snprintf(buff, sizeof(buff), "AT+GTTSINT=1,\"%s\",%d",ttstext, ttsencode);
    dev_wireless_send_at_cmd(buff, AT_API_BLOCK, TRUE);
    ret = DDI_OK;
    return ret;
    
}

static s32 L610_audiofile_play_callback(u8 result, u8 *src, u16 srclen)
{
    wireless_thread_node_param_t *thread_node_param = dev_wireless_get_thread_node_param();
    s32 ret = DDI_ERR;

    TRACE_DBG("result:%d, src:%s", result, src);
    switch(result)
    {
        case WIRE_AT_RESULT_OTHER:
            if(strstr(src, "AUDPLAYOK"))
            {
                dev_wireless_thread_set_result(WIRE_AT_RESULT_OK);
            }
            else if(strstr(src, "AUDPLAYERROR"))
            {
                dev_wireless_thread_set_result(WIRE_AT_RESULT_ERROR);
            }
            break;
            
        case WIRE_AT_RESULT_TIMEROUT:
            dev_wireless_thread_set_result(result);
            break;
    }
    return ret;
}

s32 L610_audiofile_play(s8 *audio_string)
{
    wireless_device_info_t *device_info = dev_wireless_get_device();
    wireless_thread_node_param_t *thread_node_param = dev_wireless_get_thread_node_param();
    s32 ret = DDI_ERR;
    s8 buff[1024];
    s32 i;
    
    memset(buff, 0, sizeof(buff));
    snprintf(buff, sizeof(buff), "AT+GTAUDPLAYINT=1,%s", audio_string);
    dev_wireless_send_at_cmd(buff, AT_API_BLOCK, TRUE);
    ret = DDI_OK;
    return ret;
}

/**
 * @brief 检查无线模块的上报信息是否是声音播报结束
 * @param[in] buf:上报信息buf
 * @retval  DDI_OK:检查到是声音播放结束的上报
 * @retval  other :不是声音播报的上报
 */
static s32 L610_audio_end_report_handle(s8 *buf)
{
    s32 ret = DDI_ERR;
    wireless_device_info_t *wireless_device = dev_wireless_get_device();

    //播报结束
    if(strstr(buf, "+AUDPLAY: END"))
    {
        TRACE_DBG("audio play mp3 end");
        wireless_device->m_audio_info.m_mp3_play_done = TRUE;
        ret = DDI_OK;
    }

    if(strstr(buf, "+TTS: END"))
    {
        TRACE_DBG("audio play tts end");
        wireless_device->m_audio_info.m_tts_play_done = TRUE;
        ret = DDI_OK;
    }
    
    return ret;
}

static s32 L610_delete_audio_file(s8 *deletefilename)
{
    wireless_device_info_t *device_info = dev_wireless_get_device();
    wireless_thread_node_param_t *thread_node_param = dev_wireless_get_thread_node_param();
    s32 ret = DDI_ERR;
    s8 buff[128];
    s32 i;
    
    memset(buff, 0, sizeof(buff));
    snprintf(buff, sizeof(buff), "AT+GTFDELETE=%s", deletefilename);
    dev_wireless_thread_set_param(WS_TASK_DELETE_AUDIOFILE, common_callback, WIRE_AT_DEFAULT_TIMEOUT*10);
    dev_wireless_send_at_cmd(buff, AT_API_BLOCK, TRUE);

    ret = common_respone_handle(WS_TASK_DELETE_AUDIOFILE);
    return ret;
    
}

static s32 L610_add_audio_file_sendlen_callback(u8 result, u8 *src, u16 srclen)
{
    s32 ret = DDI_ERR;
    
    TRACE_DBG("result:%d, src:%s", result, src);
    switch(result)
    {
        case WIRE_AT_RESULT_OTHER:
            if(0 == strcmp(src, ">"))
            {
                dev_wireless_thread_set_result(WIRE_AT_RESULT_OK);
            }
            break;
            
        case WIRE_AT_RESULT_TIMEROUT:
        case WIRE_AT_RESULT_ERROR:
            dev_wireless_thread_set_result(result);
            break;
    }
    
    return ret;
}

static s32 L610_add_audio_file_sendlen(s8 *addfilename, u32 addfiledatalen)
{
    wireless_thread_node_param_t *thread_node_param = dev_wireless_get_thread_node_param();
    s32 ret = DDI_ERR;
    s8 buff[128];
    s32 i;
    
    memset(buff, 0, sizeof(buff));
    snprintf(buff, sizeof(buff), "At+AUDWR=%s,%d",addfilename, addfiledatalen);
    dev_wireless_thread_set_param(WS_TASK_ADD_AUDIOFILE_LEN, L610_add_audio_file_sendlen_callback, WIRE_AT_DEFAULT_TIMEOUT); //先默认1S的超时时间
    dev_wireless_send_at_cmd(buff, AT_API_BLOCK, TRUE);
    ret = common_respone_handle(WS_TASK_ADD_AUDIOFILE_LEN);
    return ret;
}

static s32 L610_add_audio_file_senddata(u8 *add_filedata, u32 addfiledatalen)
{
    s32 ret = DDI_ERR;
    s32 i = 0, j = 0;
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();

    ret = dev_com_write(WIRELESS_PORT, add_filedata, addfiledatalen);
    dev_com_tcdrain(WIRELESS_PORT);
    
    return ret;
}

static s32 L610_add_audio_file(s32 file_index)
{
    s32 ret;
    s8 buff[WIRELESS_AUDIO_FILE_NAME_MAX] = {0};
    tts_bin_instance_t *tts_bin = ttsbin_instance();
    s32 filelen = 0;
    s8 *data_buff = NULL;
    s32 i = 0;
    s32 j = 0;

    do{
        if(!tts_bin)
        {
            TRACE_ERR("tts.bin load failed");
            ret = DDI_ERR;
            break;
        }

        data_buff = (s8 *)k_malloc(WIRELESS_AUDIO_FILE_UPLOAD_PER_SIZE);
        if(!data_buff)
        {
            ret = DDI_EOVERFLOW;
            break;
        }
        
        filelen = tts_bin->get_audio_filelen(file_index);
        memset(buff, 0, sizeof(buff));
        snprintf(buff, sizeof(buff), "\"%s.mp3\"", tts_bin->get_audio_filetext(file_index));
        ret = L610_add_audio_file_sendlen(buff, filelen);
        if(DDI_OK != ret)
        {
            TRACE_ERR("send datalen fail:%d", ret);
            break;
        }
        TRACE_DBG("ret:%d", ret);

        i = 0;
        dev_wireless_thread_set_param(WS_TASK_ADD_AUDIOFILE_DATA, common_callback, WIRE_AT_CMD_RESEND_TIMEOUT*5);
        while(i<filelen)
        {
            ddi_watchdog_feed();
            if((i+WIRELESS_AUDIO_FILE_UPLOAD_PER_SIZE) > filelen)
            {
                j = filelen - i;
            }
            else
            {
                j = WIRELESS_AUDIO_FILE_UPLOAD_PER_SIZE;
            }

            memset(data_buff, 0, WIRELESS_AUDIO_FILE_UPLOAD_PER_SIZE);
            ret = tts_bin->get_audio_filedata(file_index, data_buff, j, i);
            ret = L610_add_audio_file_senddata(data_buff, j);
            if(ret < DDI_OK)
            {
                TRACE_ERR("send this pack data fail:%d", ret);
                break;
            }
            else
            {
                i += j;
            }
        }

        if(i == filelen)
        {
            ret = common_respone_handle(WS_TASK_ADD_AUDIOFILE_DATA);
        }
        k_free(data_buff);
        data_buff = NULL;
    }while(0);

    TRACE_DBG("out");
    return ret;
}

static s32 L610_get_audio_volume_callback(u8 result, u8 *src, u16 srclen)
{
    wireless_thread_node_param_t *thread_node_param = dev_wireless_get_thread_node_param();
    s32 ret = DDI_ERR;
    s8 *pstr = NULL;
    s32 vol = 0;
    
    switch(result)
    {
        case WIRE_AT_RESULT_OTHER:
            pstr = strstr(src, "+CLVL:");
            if(pstr)
            {
                thread_node_param->m_lparam = TRUE;
                pstr += strlen("+CLVL:");
                dev_maths_str_trim(pstr);
                thread_node_param->m_wparam = dev_maths_asc_to_u32(pstr, strlen(pstr), MODULUS_DEC);
                TRACE_DBG("at get volume = %d", thread_node_param->m_wparam);
            }

            if(strstr(src,"CLVLOK"))
            {
                vol = thread_node_param->m_wparam;
                dev_wireless_thread_set_result(WIRE_AT_RESULT_OK);
                thread_node_param->m_wparam = vol;
            }
            else if(strstr(src,"CLVLERROR"))
            {
                dev_wireless_thread_set_result(WIRE_AT_RESULT_ERROR);
            }
            break;
            
        case WIRE_AT_RESULT_OK:
            if(thread_node_param->m_lparam)
            {
                vol = thread_node_param->m_wparam;
                dev_wireless_thread_set_result(result);
                thread_node_param->m_wparam = vol;
            }
            break;
            
        case WIRE_AT_RESULT_TIMEROUT:
            dev_wireless_thread_set_result(result);
            break;
    }
    return ret;
    
}

s32 L610_get_audio_volume(u32 *audiovolume)
{
    wireless_device_info_t *device_info = dev_wireless_get_device();
    wireless_thread_node_param_t *thread_node_param = dev_wireless_get_thread_node_param();
    s32 ret = DDI_ERR;
    s8 buff[32];
    s32 i;
    
    memset(buff, 0, sizeof(buff));
    strcpy(buff, "AT+CLVL?");

    dev_wireless_thread_set_param(WS_TASK_GETSET_AUDIOVOLUME, L610_get_audio_volume_callback, WIRE_AT_DEFAULT_TIMEOUT); //先默认1S的超时时间
    dev_wireless_send_at_cmd(buff, AT_API_BLOCK, TRUE);
    while(1)
    {
        ddi_watchdog_feed();
        dev_wireless_thread_run_switch(FALSE);
        
        if(dev_wireless_thread_task_is_change(WS_TASK_GETSET_AUDIOVOLUME))
        {
            ret = DDI_EBUSY;
            break;
        }
        
        if(dev_wireless_thread_is_free())
        {
            if(WIRE_AT_RESULT_OK == thread_node_param->m_result)
            {
                *audiovolume = thread_node_param->m_wparam;
                ret = DDI_OK;
            }
            else
            {
                ret = DDI_ERR;
            }
            break;
        }

        dev_wireless_thread_run_switch(TRUE);
        ddi_misc_msleep(20);
    }

    dev_wireless_thread_run_switch(TRUE);
    return ret;
}

s32 L610_set_audio_volume(u32 audiovolume, AT_API_GET_TYPE_E at_api_get_type)
{
    wireless_device_info_t *device_info = dev_wireless_get_device();
    wireless_thread_node_param_t *thread_node_param = dev_wireless_get_thread_node_param();
    s32 ret = DDI_ERR;
    s8 buff[32];
    s32 i;
    
    memset(buff, 0, sizeof(buff));
    sprintf(buff, "AT+CLVL=%d",audiovolume);
    dev_wireless_send_at_cmd(buff, AT_API_BLOCK, TRUE);
    dev_wireless_thread_set_timeout(WIRE_AT_DEFAULT_TIMEOUT);
    ret = DDI_OK;

    if(AT_API_BLOCK == at_api_get_type)
    {
        dev_wireless_thread_set_param(WS_TASK_GETSET_AUDIOVOLUME, common_callback, WIRE_AT_DEFAULT_TIMEOUT); //先默认1S的超时时间
        ret = common_respone_handle(WS_TASK_GETSET_AUDIOVOLUME);
    }
    return ret;
    
}

//设置外部功放
static s32 L610_set_useextamp(AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;

    switch(at_api_get_type)
    {
        case AT_API_NO_BLOCK:
            //非阻塞
            dev_wireless_send_at_cmd(g_current_module->m_set_useextamp_cmd, at_api_get_type, TRUE);
            dev_wireless_thread_set_timeout(WIRE_AT_CTL_TIMEOUT);
            ret = DDI_OK;    
            break;
            
        case AT_API_BLOCK:
            ret = DDI_ERR;
            break;
    }
    
    return ret;
}

//设置外部喇叭的通道为speaker
static s32 L610_cfg_audio_play_path(AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;

    switch(at_api_get_type)
    {
        case AT_API_NO_BLOCK:
            //非阻塞
            dev_wireless_send_at_cmd(g_current_module->m_cfg_audio_play_path_cmd, at_api_get_type, TRUE);
            dev_wireless_thread_set_timeout(WIRE_AT_DEFAULT_TIMEOUT);
            ret = DDI_OK;    
            break;
            
        case AT_API_BLOCK:
            ret = DDI_ERR;
            break;
    }
    
    return ret;
}

//设置外部喇叭 麦克风为静音，speaker为非静音
static s32 L610_cfg_audio_mut(AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;

    switch(at_api_get_type)
    {
        case AT_API_NO_BLOCK:
            //非阻塞
            dev_wireless_send_at_cmd(g_current_module->m_cfg_audio_mut_cmd, at_api_get_type, TRUE);
            dev_wireless_thread_set_timeout(WIRE_AT_DEFAULT_TIMEOUT);
            ret = DDI_OK;    
            break;
            
        case AT_API_BLOCK:
            ret = DDI_ERR;
            break;
    }
    
    return ret;
}

static s32 L610_get_audio_file_list_callback(u8 result, u8 *src, u16 srclen)
{
    wireless_thread_node_param_t *thread_node_param = dev_wireless_get_thread_node_param();
    s32 ret = DDI_ERR;
    s8 dst[2][SPLIT_PER_MAX] = {0};
    s8 (*filelist)[WIRELESS_AUDIO_FILE_NAME_MAX] = (s8 (*)[WIRELESS_AUDIO_FILE_NAME_MAX])thread_node_param->m_addr_param;
    s32 filelist_num = thread_node_param->m_param1;
    s32 cur_no = thread_node_param->m_param2;
    s8 *pstr = NULL;
    s8 buff[32];
    s32 tmp_val = 0;

    TRACE_DBG("result:%d, src:%s", result, src);
    switch(result)
    {
        case WIRE_AT_RESULT_OTHER:
            pstr = strstr(src, "+GTFLIST:");
            if(pstr)
            {
                if(cur_no < filelist_num)
                {
                    pstr += strlen("+GTFLIST:");
                    memset(dst, 0, sizeof(dst));
                    ret = dev_maths_split(dst, sizeof(dst)/sizeof(dst[0]), pstr, ",", SPLIT_NOIGNOR);
                    if(sizeof(dst)/sizeof(dst[0]) == ret)
                    {
                        dev_maths_str_trim(dst[0]);
                        snprintf(filelist[cur_no], WIRELESS_AUDIO_FILE_NAME_MAX, "%s", dst[0]);
                        thread_node_param->m_param2++;
                    }
                }
            }
            break;
            
        case WIRE_AT_RESULT_OK:
            tmp_val = thread_node_param->m_param2;
            dev_wireless_thread_set_result(result);
            thread_node_param->m_param2 = tmp_val;
            break;
            
        case WIRE_AT_RESULT_TIMEROUT:
            ret = DDI_ETIMEOUT;
            break;
    }

    return ret;
    
}

/**
 * @brief 获取modem端的audio file list
 * 
 * @retval  DDI_OK:检查到是声音播放结束的上报
 * @retval  other :不是声音播报的上报
 */
static s32 L610_audio_get_audio_file_list(s8 filelist[][WIRELESS_AUDIO_FILE_NAME_MAX], s32 filelist_num)
{
    wireless_device_info_t *device_info = dev_wireless_get_device();
    wireless_thread_node_param_t *thread_node_param = dev_wireless_get_thread_node_param();
    s32 ret = DDI_ERR;
    
    dev_wireless_send_at_cmd("AT+GTFLIST", AT_API_BLOCK, TRUE);
    dev_wireless_thread_set_param(WS_TASK_GET_MODEMAUDOFILELIST, L610_get_audio_file_list_callback, WIRE_AT_GETAUDIOFILELIST_TIMEOUT);
    thread_node_param->m_param1 = filelist_num;
    thread_node_param->m_addr_param = (u32)filelist;
    ret = common_respone_handle(WS_TASK_GET_MODEMAUDOFILELIST);
    if(DDI_OK == ret)
    {
        ret = thread_node_param->m_param2;
    }
    return ret;
}

/**
 * @brief 关闭声音延时
 * 
 * @retval  TRUE:此指令不需要发送，可直接执行下一条
 * @retval  DDI_OK：执行成功
 */
static s32 L610_close_audio_play_sleep(void)
{
    wireless_device_info_t *device_info = dev_wireless_get_device();
    
#if(WIRELESS_CLOSE_AUDIO_PLAY_SLEEP == FALSE)
    return TRUE;
#else
    //广和通目前提供的这三个固件不支持此指令
    if(0 == strcmp(device_info->m_module_ver, "16000.1028.00.06.12.01")
        || 0 == strcmp(device_info->m_module_ver, "16000.1028.00.06.12.02")
        || 0 == strcmp(device_info->m_module_ver, "16000.1000.00.06.12.01"))
    {
        TRACE_DBG("this version %s don't support this cmd", device_info->m_module_ver);
        return TRUE;
    }
    else
    {
        dev_wireless_send_at_cmd(g_L610.m_close_audio_play_sleep_cmd, AT_API_BLOCK, TRUE);
        dev_wireless_thread_set_timeout(WIRE_AT_DEFAULT_TIMEOUT);
    }
    
    return DDI_OK;
#endif
}

#endif

static s32 L610_setfota_upgrade_param_callback(u8 result, u8 *src, u16 srclen)
{
//    wireless_thread_node_param_t *thread_node_param = dev_wireless_get_thread_node_param();
    s32 ret = DDI_ERR;

    switch(result)
    {
    case WIRE_AT_RESULT_OTHER:
        ret = DDI_ERR;
        dev_wireless_thread_set_result(WIRE_AT_RESULT_ERROR);
        break;
    case WIRE_AT_RESULT_OK:
        ret = DDI_OK;
        dev_wireless_thread_set_result(result);
        break;
        
    case WIRE_AT_RESULT_TIMEROUT:
        ret = DDI_ETIMEOUT;
    case WIRE_AT_RESULT_ERROR:
        dev_wireless_thread_set_result(result);
        break;
    }
    return ret;
    
}

//设置FOTA升级参数
s32 L610_setfota_upgrade_param(s8 fota_type, s8 *url, s8 *filename,s8 *username, s8 *password)
{
    wireless_device_info_t *device_info = dev_wireless_get_device();
    wireless_thread_node_param_t *thread_node_param = dev_wireless_get_thread_node_param();
    s32 ret = DDI_ERR;
    s8 *buff;
    s32 i;
    s8 trytimes = 0;
    s32 buff_len = 512+32;
    
    do{
        buff = (s8 *)k_malloc(buff_len);
        if(!buff)
        {
            TRACE_ERR("malloc fail");
            ret = DDI_EOVERFLOW;
            break;
        }

        //先关闭通话打断
        dev_wireless_send_at_cmd("AT+GTSET=\"CALLBREAK\",1", AT_API_BLOCK, TRUE);
        dev_wireless_thread_set_param(WS_TASK_SET_FOTA_PARAM, common_callback, WIRE_AT_DEFAULT_TIMEOUT); //先默认1S的超时时间
        ret = common_respone_handle(WS_TASK_SET_FOTA_PARAM);
        if(DDI_OK != ret)
        {
            TRACE_ERR("exce error:%d", ret);
            break;
        }

        memset(buff, 0, buff_len);
        if(OTA_DT_HTTP == fota_type)
        {
            sprintf(buff, "AT+GTOTA=0,\"%s\"", url);
        }
        else
        {
            sprintf(buff, "AT+GTOTA=1,\"%s\",\"%s\"", url,filename);
            if(username && strlen(username))
            {
                if(password && strlen(password))
                {
                    sprintf(buff+strlen(buff), ",\"%s\",\"%s\"", username, password);
                }
                else
                {
                    sprintf(buff+strlen(buff), ",\"%s\"", username);
                }
            }
        }
        dev_wireless_send_at_cmd(buff, AT_API_NO_BLOCK, TRUE);
        dev_wireless_thread_set_param(WS_TASK_SET_FOTA_PARAM, common_callback, WIRE_AT_SET_FOTAPARAM_TIMEOUT); //先默认1S的超时时间
        ret = common_respone_handle(WS_TASK_SET_FOTA_PARAM);
        if(DDI_OK == ret)
        {
            device_info->m_ota_info.m_step = OTA_STEP_DOWNLOAD;
            device_info->m_ota_info.m_process_value.m_handle_step = HS_IDLE;
        }
    }while(0);

    if(buff)
    {
        k_free(buff);
        buff = NULL;
    }
    return ret;
}



/**
 * @brief FOTA升级上报信息
 * @param[in] buf:
 * @retval  DDI_OK:检查到是FOTA升级处理上报
 * @retval  other :不是FOTA升级处理的上报
 */
static s32 L610_fota_processing_report_handle(s8 *buf)
{
    s32 ret = DDI_ERR;
    s8 *pstr, *pstr_tmp;
    s8 percent[32];
    wireless_device_info_t *wireless_device = dev_wireless_get_device();

    do{
        if(strstr(buf, "GTOTA DOWNLOAD START"))
        {
            TRACE_INFO("begine download");
            wireless_device->m_ota_info.m_step = OTA_STEP_DOWNLOAD;
            wireless_device->m_ota_info.m_process_value.m_handle_step = HS_PROCESSING;
            wireless_device->m_ota_info.m_process_value.m_val = 0;
            ret = DDI_OK;
        }
        else if(strstr(buf, "GTOTA DOWNLOAD COMPLETE"))
        {
            TRACE_INFO("download complete");
            wireless_device->m_ota_info.m_step = OTA_STEP_UPGRADE;
            wireless_device->m_ota_info.m_process_value.m_handle_step = HS_PROCESSING;
            dev_wireless_strategy_run_switch(FALSE);       //关闭网络策略，因这时无线模块在更新固件中
            ret = DDI_OK;
        }
        else if(strstr(buf, "GTOTA DOWNLOAD FAILED"))
        {
            TRACE_INFO("download fail");
            wireless_device->m_ota_info.m_step = OTA_STEP_DOWNLOAD;
            wireless_device->m_ota_info.m_process_value.m_handle_step = HS_EXEC_FAIL;
            ret = DDI_OK;
        }
        else if(strstr(buf, "FOTA_UPDATE_SUCCESS"))
        {
            TRACE_INFO("upgrade success");
            wireless_device->m_ota_info.m_step = OTA_STEP_UPGRADE;
            wireless_device->m_ota_info.m_process_value.m_handle_step = HS_EXEC_SUCC;
            dev_wireless_strategy_run_switch(TRUE);
            ret = DDI_OK;
        }
        else if(strstr(buf, "FOTA_UPDATE_FAILED"))
        {
            TRACE_INFO("upgrade fail");
            wireless_device->m_ota_info.m_step = OTA_STEP_UPGRADE;
            wireless_device->m_ota_info.m_process_value.m_handle_step = HS_EXEC_FAIL;
            dev_wireless_strategy_run_switch(TRUE);
            ret = DDI_OK;
        }
        else
        {
            pstr = strstr(buf, "GTOTA DOWNLOADING:");
            if(pstr)
            {
                pstr += strlen("GTOTA DOWNLOADING:");
                dev_maths_str_trim(pstr);
                wireless_device->m_ota_info.m_process_value.m_val = dev_maths_asc_to_u32(pstr, strlen(pstr), MODULUS_DEC);
                TRACE_DBG("percent:%d", wireless_device->m_ota_info.m_process_value.m_val);
                ret = DDI_OK;
            }
        }
    }while(0);
    
    
    return ret;
    
}

#if(WIRELESS_USE_UART_RING == TRUE)
/**
 * @brief 设置DTR休眠唤醒
 * @param[in]
 * @retval
 * 
 */
static s32 L610_set_gtlpm_mode(AT_API_GET_TYPE_E at_api_get_type)
{
    
    wireless_device_info_t *device_info = dev_wireless_get_device();
    s32 ret = DDI_ERR;

    switch(at_api_get_type)
    {
        case AT_API_NO_BLOCK:
            
            ret = dev_wireless_send_at_cmd("AT+GTLPMMODE=2,0", at_api_get_type, TRUE);
            if(ret >= 0)
            {
                dev_wireless_thread_set_timeout(WIRE_AT_DEFAULT_TIMEOUT);
                ret = DDI_OK;
            }
            
            break;

        case AT_API_BLOCK:
            break;
    }

    return ret;
    
}


/**
 * @brief 设置ring mode
 * @param[in]
 * @retval
 * 
 */
static s32 L610_uartring_mode(AT_API_GET_TYPE_E at_api_get_type)
{
    
    wireless_device_info_t *device_info = dev_wireless_get_device();
    s32 ret = DDI_ERR;

    switch(at_api_get_type)
    {
        case AT_API_NO_BLOCK:
            
            ret = dev_wireless_send_at_cmd("AT+GTWAKE=2,1", at_api_get_type, TRUE);
            if(ret >= 0)
            {
                dev_wireless_thread_set_timeout(WIRE_AT_DEFAULT_TIMEOUT);
                ret = DDI_OK;
            }
            
            break;

        case AT_API_BLOCK:
            break;
    }
    
    return ret;
    
}



/**
 * @brief 设置休眠的CSCLK
 * @param[in]
 * @retval
 * 
 */
s32 L610_set_csclk(AT_API_GET_TYPE_E at_api_get_type)
{
    
    wireless_device_info_t *device_info = dev_wireless_get_device();
    s32 ret = DDI_ERR;

    switch(at_api_get_type)
    {
        case AT_API_NO_BLOCK:
            
            ret = dev_wireless_send_at_cmd("AT+CSCLK=1", at_api_get_type, TRUE);
            if(ret >= 0)
            {
                dev_wireless_thread_set_timeout(WIRE_AT_DEFAULT_TIMEOUT);
                ret = DDI_OK;
            }
            
            break;

        case AT_API_BLOCK:
            break;
    }
    
    return ret;
    
}
#endif

wireless_module_t* FIBOCOM_L610_module(void)
{
    g_current_module = &g_L610;
    g_L610.m_id = FIBOCOM_L610;

    g_L610.m_support_ota = TRUE;
    g_L610.m_with_flow_control = FALSE;
    g_L610.cfg_flow_control = L610_cfg_flow_control;
    
    g_L610.open_airplane = G500_open_airplane;

    g_L610.close_airplane = G500_close_airplane;
    g_L610.telephone_dial = G500_telephone_dial;
    g_L610.telephone_hangup = G500_telephone_hangup;
    g_L610.cfg_ntp_server = G500_cfg_ntp_server;
    g_L610.qry_rtc = G500_qry_rtc;
    
    g_L610.m_get_cellinfo_cmd = "AT+GTCCINFO?";
    g_L610.get_cellinfo = G500_get_cellinfo;
    g_L610.get_cellinfo_callback = G500_get_cellinfo_callback;
    
    g_L610.m_power_off_cmd = "AT+CPWROFF";
    g_L610.power_off = L610_power_off;
    g_L610.power_off_callback = L610_power_off_callback;
    
    g_L610.socket_create = G500_socket_create;
    g_L610.get_valid_socketid_list = G500_get_valid_socketid_list;
    g_L610.socket_send = L610_socket_send;
    g_L610.socket_close = G500_socket_close;
    
    g_L610.get_set_apn_cmd = G500_get_set_apn_cmd;
    g_L610.set_apn = G500_set_apn;
    g_L610.set_apn_callback = G500_set_apn_callback;

    g_L610.get_inactive_pdp_cmd = "AT+MIPCALL=0";
    g_L610.inactive_pdp = G500_inactive_pdp;
    
    g_L610.m_get_local_ip_res_kw = "+MIPCALL:";
    g_L610.m_get_local_ip_cmd = "AT+MIPCALL?";
    g_L610.get_local_ip = G500_get_local_ip;
    g_L610.get_local_ip_callback = G500_get_local_ip_callback;
    
    g_L610.m_cfg_rec_data_format_cmd = "AT+GTSET=\"IPRFMT\",2";
    g_L610.cfg_rec_data_format = G500_cfg_rec_data_format;
    g_L610.cfg_rec_data_format_callback = G500_cfg_rec_data_format_callback;

    g_L610.m_check_rplmn_cmd = "AT+CRSM=176,28542,0,0,11";
    g_L610.check_rplmn = G500_check_rplmn;
    g_L610.check_rplmn_callback = G500_check_rplmn_callback;

    g_L610.clear_rplmn = G500_clear_rplmn;
    g_L610.m_clear_rplmn_cmd = "AT+CRSM=214,28542,0,4,3,\"FFFFFF\"";
    
    g_L610.m_imei_cmd = "AT+CGSN";
    g_L610.get_imei = G500_get_imei;
    g_L610.get_imei_callback = G500_get_imei_callback;

    g_L610.m_check_workmod_cmd = "AT+GTRAT?";
    g_L610.check_work_mode = G500_check_work_mode;
    g_L610.check_work_mode_callback = L610_get_work_mode_callback;

    g_L610.m_set_workmod_cmd = "AT+GTRAT=10,3,0";
    g_L610.set_work_mode = L610_set_work_mode_nb;

    g_L610.m_ccid_cmd = "AT+CCID";
    g_L610.m_ccid_cmd_res_kw = "+CCID:";
    g_L610.get_ccid = G500_get_ccid;
    g_L610.get_ccid_callback = G500_get_ccid_callback;

    g_L610.m_simcard_state_cmd = "AT+CPIN?";
    g_L610.m_simcard_state_cmd_res_kw = "+CPIN: READY";
    g_L610.get_simcard_state = G500_get_simcard_state;
    g_L610.get_simcard_state_callback = G500_get_simcard_state_callback;

    g_L610.m_get_csq_cmd = "AT+CSQ?";
    g_L610.m_get_csq_res_kw = "+CSQ:";
    g_L610.get_csq = G500_get_csq;
    g_L610.get_csq_callback = G500_get_csq_callback;

    g_L610.m_get_2G_net_reg_cmd = "AT+CGREG?";
    g_L610.m_get_4G_net_reg_cmd = "AT+CEREG?";
    g_L610.get_reg_cmd = G500_get_reg_cmd;
    g_L610.m_get_net_reg_res_kw = "REG:";
    g_L610.get_net_reg = G500_get_net_reg;
    g_L610.get_net_reg_callback = G500_get_net_reg_callback;

    g_L610.m_set_2G_reg_param_cmd = "AT+CGREG=2";
    g_L610.m_set_4G_reg_param_cmd = "AT+CEREG=2";
    g_L610.set_reg_param = G500_set_reg_param;
    g_L610.get_set_reg_param_cmd = G500_get_set_reg_param_cmd;

    g_L610.m_get_imsi_cmd = "AT+CIMI";
    g_L610.get_imsi = G500_get_imsi;
    g_L610.get_imsi_callback = G500_get_imsi_callback;
    
    g_L610.m_tcp_rec_cmd_head_kw = "+MIPRTCP:";

    g_L610.m_pdp_report_head_kw = "+MIPCALL:";
    g_L610.m_connect_report_head_kw = "+MIPOPEN:";
    g_L610.m_socket_report_head_kw = "+MIPSTAT:";

    g_L610.net_reg_report_handle = G500_net_reg_report_handle;
    g_L610.pdp_report_handle = G500_pdp_report_handle;
    g_L610.tcp_connect_report_handle = G500_tcp_connect_report_handle;
    g_L610.socket_report_handle = G500_socket_report_handle;

    g_L610.cfg_exGPIO  =  G500_cfg_exGPIO; 
    g_L610.set_exGPIO  =  G500_set_exGPIO; 
    g_L610.play_audio   =  G500_play_audio;  
    g_L610.get_appver   =  G500_get_appver;  
    g_L610.get_appver_callback = G500_get_appver_callback;
    g_L610.suspend = G500_suspend;
    g_L610.resume = G500_resume;
    g_L610.ssl_set_mod  =  G500_ssl_set_mod; 
    g_L610.ssl_setver  =  G500_ssl_setver; 
    g_L610.ssl_setcert =  G500_ssl_setcert;
    g_L610.ssl_filte =  G500_ssl_filte;

#if(WIRELESS_USE_EXTERANL_AMP == TRUE)
    g_L610.ttsaudio_play = L610_ttsaudio_play;
    g_L610.audio_end_report_handle = L610_audio_end_report_handle;
    g_L610.delete_audio_file = L610_delete_audio_file;
    g_L610.add_audio_file = L610_add_audio_file;
    g_L610.audiofile_play = L610_audiofile_play;
    g_L610.get_audio_volume = L610_get_audio_volume;
    g_L610.set_audio_volume = L610_set_audio_volume;
    g_L610.audio_get_audio_file_list = L610_audio_get_audio_file_list;

    g_L610.m_set_useextamp_cmd = "AT+GTCOFPIN=35";
    g_L610.set_useextamp = L610_set_useextamp;
    g_L610.m_cfg_audio_play_path_cmd = "AT+MAPATH=2,0";
    g_L610.cfg_audio_play_path = L610_cfg_audio_play_path;
    g_L610.m_cfg_audio_mut_cmd = "AT+CMUT=1";
    g_L610.cfg_audio_mut = L610_cfg_audio_mut;
    g_L610.m_close_audio_play_sleep_cmd = "AT+GTSPKDLY=0";
    g_L610.close_audio_play_sleep = L610_close_audio_play_sleep;
#endif
    g_L610.setfota_upgrade_param = L610_setfota_upgrade_param;
    g_L610.fota_processing_report_handle = L610_fota_processing_report_handle;
#if(WIRELESS_USE_UART_RING == TRUE)
    g_L610.m_uartring_mode_cmd = "AT+GTWAKE=2,1";
    g_L610.uartring_mode = L610_uartring_mode;
    g_L610.m_gtlpm_mode_cmd = "AT+GTLPMMODE=2,0";
    g_L610.set_gtlpm_mode = L610_set_gtlpm_mode;
    g_L610.m_set_csclk_cmd = "AT+CSCLK=1";
    g_L610.set_csclk = L610_set_csclk;
#endif
    
    return &g_L610;
}
#endif
