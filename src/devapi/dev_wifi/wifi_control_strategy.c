#include "devglobal.h"
#include "ddi_com.h"
#include "ddi_wifi.h"
#include "drv_wifi_io.h"
#include "dev_wifi_thread.h"
#include "dev_wifi.h"
#include "wifi_control_strategy.h"
#include "wifi_data_handle.h"

static u16 g_wifi_detect_times = 0;           //模块检测次数
static s32 g_wifi_strategy_run_flag = TRUE;
static WIFI_STRATEGY_E g_wifi_strategy = WF_NORMAL_RUN;

#if (WIFI_EXIST == 1)
static s32 wifi_check_cmd_process_callback(u8 result, u8 *src, u16 srclen)
{
    wifi_device_info_t *device_info = dev_wifi_get_device();

    TRACE_DBG("result:%d", result);
    if(WIFI_AT_RESULT_OK == result || WIFI_AT_RESULT_OTHER == result)
    {
        TRACE_INFO("AT cmd recv ok, restore step to %d", device_info->m_last_control_step);
        dev_wifi_set_control_strategy_step(device_info->m_last_control_step);
        dev_wifi_thread_set_result(result);
    }
    else if(WIFI_AT_RESULT_TIMEROUT == result)
    {
        wifi_at_timeout_handle(WIFI_AT_CMD, WIFI_AT_DEFAULT_TIMEOUT);
    }

    return 0;
}

/**
 * @brief 指令异常流程处理
 * @param[in] 无
 * @retval  无
 */
static void wifi_check_cmd_process(void)
{
    wifi_device_info_t *device_info = dev_wifi_get_device();

    TRACE_INFO("app send AT overtime, then check module(step:%d)", device_info->m_control_step);
    dev_wifi_send_at_cmd(WIFI_AT_CMD, AT_API_NO_BLOCK, TRUE);
    dev_wifi_thread_set_param(WIFI_TASK_CHECK_CMD, wifi_check_cmd_process_callback, WIFI_AT_DEFAULT_TIMEOUT);
}

static s32 wifi_power_off_callback(u8 result, u8 *src, u16 srclen)
{
    if(WIFI_AT_RESULT_TIMEROUT == result)
    {
        TRACE_INFO("power off finish, then repower on the module");
        wifi_power_on_process();
    }

    return 0;
}

/**
 * @brief wifi模块下电流程
 * @param[in] 无
 * @retval  无
 */
static void wifi_power_off_process(void)
{
    TRACE_INFO("enter");
    dev_gpio_direction_output(WIFI_EN_PIN, 0);
    dev_wifi_set_control_strategy_step(WIFI_MODULE_POWER_OFF);
    dev_wifi_thread_set_param(WIFI_TASK_RESET_MODULE, wifi_power_off_callback, WIFI_AT_BOOTON_TIMEOUT);
}

/**
 * @brief wifi模块初始化流程
 * @param[in] 无
 * @retval  无
 */
static void wifi_module_open_success_process(void)
{
    dev_wifi_set_control_strategy_step(WIFI_MODULES_OPEN_SUCCESS);
    switch(g_wifi_strategy)
    {
        case WF_NORMAL_RUN:
            //dev_wifi_thread_set_result(WIFI_AT_RESULT_OK);
            //wifi_connect_ap_process();   //Add by xiaohonghui  由应用层自己去决定连哪个热点
            break;

        case WF_AIR_KISS_CONFIG:
        case WF_WEB_CONFIG:
        case WF_SCAN_AP:
            //dev_wifi_thread_set_result(WIFI_AT_RESULT_OK);
            break;
    }
}

static s32 wifi_module_init_callback(u8 result, u8 *src, u16 srclen)
{
    wifi_thread_node_param_t *thread_node_param = (wifi_thread_node_param_t *)dev_wifi_get_thread_node_param();
    wifi_device_info_t *device_info = dev_wifi_get_device();
    s32 i = 0;
    s8 *pstr = NULL;
    s8 buff[128];

    TRACE_INFO("step:%d, result:%d", thread_node_param->m_step, result);
    switch(thread_node_param->m_step)
    {
        case WIFI_INIT_STEP_AT_TEST:
            if(WIFI_AT_RESULT_OK == result)
            {
                TRACE_INFO("wifi module power on success");
                g_wifi_detect_times = 0;
                thread_node_param->m_step = WIFI_INIT_STEP_ATE;
                dev_wifi_thread_param_clean();
                dev_wifi_send_at_cmd(WIFI_ATE_CMD, AT_API_NO_BLOCK, TRUE);
                dev_wifi_thread_set_timeout(WIFI_AT_DEFAULT_TIMEOUT);
            }
            else if(WIFI_AT_RESULT_TIMEROUT == result)
            {
                if(0 == thread_node_param->m_lparam)      //指令超时
                {
                    thread_node_param->m_lparam = 1;
                    g_wifi_detect_times++;
                    if(WIFI_MODULE_DETECT_TIMES <= g_wifi_detect_times)
                    {
                        TRACE_ERR("The machine don't containt wifi module");
                        dev_wifi_set_control_strategy_fail(WF_NO_WIFI_MODULE);
                        dev_wifi_thread_set_result(result);
                    }
                    else if(0 == g_wifi_detect_times % WIFI_RESET_THRESHOLD_TIMES)
                    {
                        TRACE_ERR("contine %d times failed, then power off module to retry", WIFI_RESET_THRESHOLD_TIMES);
                        wifi_power_off_process();
                    }
                    else
                    {
                        TRACE_DBG("send AT failed %d times", g_wifi_detect_times);
                        dev_wifi_thread_set_timeout(WIFI_AT_CMD_RESEND_TIMEOUT);
                    }
                }
                else   //休眠超时
                {
                    dev_wifi_send_at_cmd(WIFI_AT_CMD, AT_API_NO_BLOCK, TRUE);
                    dev_wifi_thread_set_timeout(WIFI_AT_DEFAULT_TIMEOUT);
                    thread_node_param->m_lparam = 0;
                }
            }
            break;

        case WIFI_INIT_STEP_ATE:
            if(WIFI_AT_RESULT_OK == result)
            {
#if(WIFI_SUPPORT_HARD_FLOW == TRUE)
                thread_node_param->m_step = WIFI_INIT_SET_HARDFLOW;
                dev_wifi_thread_param_clean();
                dev_wifi_send_at_cmd("AT+UART_CUR=115200,8,1,0,3", AT_API_NO_BLOCK, TRUE);
                dev_wifi_thread_set_timeout(WIFI_AT_DEFAULT_TIMEOUT);
#else
                thread_node_param->m_step = WIFI_INIT_GET_MODEL_VER;
                dev_wifi_thread_param_clean();
                dev_wifi_send_at_cmd(WIFI_AT_GMR_CMD, AT_API_NO_BLOCK, TRUE);
                dev_wifi_thread_set_timeout(WIFI_AT_DEFAULT_TIMEOUT);
                TRACE_DBG("m_func:%x,%x", thread_node_param->m_func, wifi_module_init_callback);
#endif
            }
            else if(WIFI_AT_RESULT_TIMEROUT == result)
            {
                wifi_at_timeout_handle(WIFI_ATE_CMD, WIFI_AT_DEFAULT_TIMEOUT);
            }
            break;

        case WIFI_INIT_SET_HARDFLOW:
            if(WIFI_AT_RESULT_OK == result)
            {
                dev_com_open(WIFI_PORT, 115200, 8, 'n', 0, 0);
#if(WIFI_RUN_OPEN_CPU == TRUE)
                send_command("AT+GTAFC=0,1\r\n");  //开启mcu端流控
#endif
                thread_node_param->m_step = WIFI_INIT_GET_MODEL_VER;
                dev_wifi_thread_param_clean();
                dev_wifi_send_at_cmd(WIFI_AT_GMR_CMD, AT_API_NO_BLOCK, TRUE);
                dev_wifi_thread_set_timeout(WIFI_AT_DEFAULT_TIMEOUT);
                TRACE_DBG("m_func:%x,%x", thread_node_param->m_func, wifi_module_init_callback);
            }
            else if(WIFI_AT_RESULT_TIMEROUT == result)
            {
                wifi_at_timeout_handle("AT+UART_CUR=115200,8,1,0,3", WIFI_AT_DEFAULT_TIMEOUT);
            }
            break;
            
        case WIFI_INIT_GET_MODEL_VER:
            if(WIFI_AT_RESULT_OTHER == result)
            {
                pstr = strstr(src, "AT version:");
                if(pstr)
                {
                    pstr += strlen("AT version:");
                    snprintf(device_info->m_at_ver, sizeof(device_info->m_at_ver), "%s", pstr);
                }

                pstr = strstr(src, "SDK version:");
                if(pstr)
                {
                    pstr += strlen("SDK version:");
                    snprintf(device_info->m_sdk_ver, sizeof(device_info->m_sdk_ver), "%s", pstr);
                    thread_node_param->m_lparam = 1;
                }
            }
            else if(WIFI_AT_RESULT_OK == result)
            {
                if(thread_node_param->m_lparam)      //收到版本前部分+ok，才认为指令结束
                {
                    dev_wifi_thread_param_clean();
                    if(WF_WEB_CONFIG == g_wifi_strategy)
                    {
                        dev_wifi_send_at_cmd(WIFI_AT_SET_CUR_MODE_3, AT_API_NO_BLOCK, TRUE);
                    }
                    else
                    {
                        dev_wifi_send_at_cmd(WIFI_AT_SET_CUR_MODE, AT_API_NO_BLOCK, TRUE);
                    }
                    dev_wifi_thread_set_timeout(WIFI_AT_DEFAULT_TIMEOUT);
                    thread_node_param->m_step = WIFI_INIT_SET_CUR_MODE;
                }
            }
            else if(WIFI_AT_RESULT_TIMEROUT == result)
            {
                wifi_at_timeout_handle(WIFI_AT_GMR_CMD, WIFI_AT_DEFAULT_TIMEOUT);
            }
            break;

        case WIFI_INIT_SET_CUR_MODE:
            if(WIFI_AT_RESULT_OK == result)
            {
                dev_wifi_thread_param_clean();
                dev_wifi_send_at_cmd(WIFI_AT_GET_TRANS_MODE, AT_API_NO_BLOCK, TRUE);
                dev_wifi_thread_set_timeout(WIFI_AT_DEFAULT_TIMEOUT);
                thread_node_param->m_step = WIFI_INIT_SET_TRANS_MODE;
            }
            else if(WIFI_AT_RESULT_TIMEROUT == result)
            {
                wifi_at_timeout_handle(WIFI_AT_SET_CUR_MODE, WIFI_AT_DEFAULT_TIMEOUT);
            }
            break;

        case WIFI_INIT_SET_TRANS_MODE:
            if(WIFI_AT_RESULT_OK == result)
            {
                dev_wifi_thread_param_clean();
                dev_wifi_send_at_cmd(WIFI_AT_SET_AUTO_CONN, AT_API_NO_BLOCK, TRUE);
                dev_wifi_thread_set_timeout(WIFI_AT_DEFAULT_TIMEOUT);
                thread_node_param->m_step = WIFI_INIT_SET_AUTO_CONN;
            }
            else if(WIFI_AT_RESULT_TIMEROUT == result)
            {
                wifi_at_timeout_handle(WIFI_AT_SET_TRANS_MODE, WIFI_AT_DEFAULT_TIMEOUT);
            }
            break;

        case WIFI_INIT_SET_AUTO_CONN:
            if(WIFI_AT_RESULT_OK == result)
            {
                dev_wifi_thread_param_clean();
                dev_wifi_send_at_cmd(WIFI_AT_SET_MUX, AT_API_NO_BLOCK, TRUE);
                dev_wifi_thread_set_timeout(WIFI_AT_DEFAULT_TIMEOUT);
                thread_node_param->m_step = WIFI_INIT_SET_MUX;
            }
            else if(WIFI_AT_RESULT_TIMEROUT == result)
            {
                wifi_at_timeout_handle(WIFI_AT_SET_AUTO_CONN, WIFI_AT_DEFAULT_TIMEOUT);
            }
            break;

        case WIFI_INIT_SET_MUX:
            if(WIFI_AT_RESULT_OK == result)
            {
                dev_wifi_thread_param_clean();
                dev_wifi_send_at_cmd(WIFI_AT_SCAN_RES_CFG, AT_API_NO_BLOCK, TRUE);
                dev_wifi_thread_set_timeout(WIFI_AT_DEFAULT_TIMEOUT);
                thread_node_param->m_step = WIFI_INIT_SET_SCAN_RES;
            }
            else if(WIFI_AT_RESULT_TIMEROUT == result)
            {
                wifi_at_timeout_handle(WIFI_AT_SET_MUX, WIFI_AT_DEFAULT_TIMEOUT);
            }
            break;

        case WIFI_INIT_SET_SCAN_RES:
            if(WIFI_AT_RESULT_OK == result)
            {
                dev_wifi_thread_param_clean();
                dev_wifi_send_at_cmd(WIFI_AT_WAKEUP_GPIO, AT_API_NO_BLOCK, TRUE);
                dev_wifi_thread_set_timeout(WIFI_AT_DEFAULT_TIMEOUT);
                thread_node_param->m_step = WIFI_INIT_SET_WAKEUP_GPIO;
            }
            else if(WIFI_AT_RESULT_TIMEROUT == result)
            {
                wifi_at_timeout_handle(WIFI_AT_SCAN_RES_CFG, WIFI_AT_DEFAULT_TIMEOUT);
            }
            break;

        case WIFI_INIT_SET_WAKEUP_GPIO:
            if(WIFI_AT_RESULT_OK == result)
            {
#if 0
                dev_wifi_thread_param_clean();
                dev_wifi_send_at_cmd(WIFI_AT_DISCONNECT_AP, AT_API_NO_BLOCK, TRUE);
                dev_wifi_thread_set_timeout(WIFI_AT_DEFAULT_TIMEOUT);
                thread_node_param->m_step = WIFI_INIT_DISCONNECT_AP;
#else
                dev_wifi_thread_param_clean();
                dev_wifi_send_at_cmd(WIFI_AT_SLEEP_MODE, AT_API_NO_BLOCK, TRUE);
                dev_wifi_thread_set_timeout(WIFI_AT_DEFAULT_TIMEOUT);
                thread_node_param->m_step = WIFI_INIT_SET_SLEEP_MODE;
#endif
            }
            else if(WIFI_AT_RESULT_TIMEROUT == result)
            {
                wifi_at_timeout_handle(WIFI_AT_WAKEUP_GPIO, WIFI_AT_DEFAULT_TIMEOUT);
            }
            break;

        case WIFI_INIT_DISCONNECT_AP:
            if(WIFI_AT_RESULT_OK == result || WIFI_AT_RESULT_TIMEROUT == result)
            {
                dev_wifi_thread_param_clean();
                dev_wifi_send_at_cmd(WIFI_AT_SLEEP_MODE, AT_API_NO_BLOCK, TRUE);
                dev_wifi_thread_set_timeout(WIFI_AT_DEFAULT_TIMEOUT);
                thread_node_param->m_step = WIFI_INIT_SET_SLEEP_MODE;
            }
            break;

        case WIFI_INIT_SET_SLEEP_MODE:
            if(WIFI_AT_RESULT_OK == result)
            {
                
                if(WF_WEB_CONFIG != g_wifi_strategy)
                {
                    dev_wifi_thread_set_result(WIFI_AT_RESULT_OK);
                    wifi_module_open_success_process();
                }
                else
                {
                    dev_wifi_thread_param_clean();
                    dev_wifi_send_at_cmd(WIFI_AT_SET_MAX_SERVERCONN, AT_API_NO_BLOCK, TRUE);
                    dev_wifi_thread_set_timeout(WIFI_AT_DEFAULT_TIMEOUT);
                    thread_node_param->m_step = WIFI_INIT_SET_MAX_CONN;
                }
            }
            else if(WIFI_AT_RESULT_TIMEROUT == result)
            {
                wifi_at_timeout_handle(WIFI_AT_SLEEP_MODE, WIFI_AT_DEFAULT_TIMEOUT);
            }
            break;

        case WIFI_INIT_SET_MAX_CONN:
            if(WIFI_AT_RESULT_OK == result)
            {
                dev_wifi_thread_param_clean();
                snprintf(buff, sizeof(buff), "AT+CIPSERVER=1,%d", device_info->m_tcp_server_port);
                dev_wifi_send_at_cmd(buff, AT_API_NO_BLOCK, TRUE);
                dev_wifi_thread_set_timeout(WIFI_AT_DEFAULT_TIMEOUT);
                thread_node_param->m_step = WIFI_INIT_OPEN_TCP_SERVER;
            }
            else if(WIFI_AT_RESULT_TIMEROUT == result)
            {
                wifi_at_timeout_handle(WIFI_AT_SET_MAX_SERVERCONN, WIFI_AT_DEFAULT_TIMEOUT);
            }
            break;

        case WIFI_INIT_OPEN_TCP_SERVER:
            if(WIFI_AT_RESULT_OK == result)
            {
                dev_wifi_thread_param_clean();
                snprintf(buff, sizeof(buff), "AT+CWSAP_CUR=\"%s\",\"\",11,0,3", device_info->m_ap_name);
                dev_wifi_send_at_cmd(buff, AT_API_NO_BLOCK, TRUE);
                dev_wifi_thread_set_timeout(WIFI_AT_DEFAULT_TIMEOUT);
                thread_node_param->m_step = WIFI_INIT_STET_SOFTAP_NAME;
            }
            else if(WIFI_AT_RESULT_TIMEROUT == result)
            {
                snprintf(buff, sizeof(buff), "AT+CIPSERVER=1,%d", device_info->m_tcp_server_port);
                wifi_at_timeout_handle(buff, WIFI_AT_DEFAULT_TIMEOUT);
            }
            break;

        case WIFI_INIT_STET_SOFTAP_NAME:
            if(WIFI_AT_RESULT_OK == result)
            {
                dev_wifi_thread_param_clean();
                dev_wifi_send_at_cmd(WIFI_AT_GET_AP_NAME, AT_API_NO_BLOCK, TRUE);
                dev_wifi_thread_set_timeout(WIFI_AT_DEFAULT_TIMEOUT);
                thread_node_param->m_step = WIFI_INIT_GET_SOFTAP_NAME;
            }
            else if(WIFI_AT_RESULT_TIMEROUT == result)
            {
                snprintf(buff, sizeof(buff), "AT+CWSAP_CUR=\"%s\",\"\",11,0,3", device_info->m_ap_name);
                wifi_at_timeout_handle(buff, WIFI_AT_DEFAULT_TIMEOUT);
            }
            break;

         case WIFI_INIT_GET_SOFTAP_NAME:
            dev_wifi_thread_set_result(WIFI_AT_RESULT_OK);
            wifi_module_open_success_process();
            break;
    }

    return 0;
}

/**
 * @brief wifi模块初始化流程
 * @param[in] 无
 * @retval  无
 */
static void wifi_module_init(void)
{
    TRACE_INFO("enter");
    dev_wifi_set_control_strategy_step(WIFI_MODULES_INIT);
    dev_wifi_send_at_cmd(WIFI_AT_CMD, AT_API_NO_BLOCK, TRUE);
    dev_wifi_thread_set_param(WIFI_TASK_MODULES_INIT, wifi_module_init_callback, WIFI_AT_DEFAULT_TIMEOUT);
}

static s32 wifi_power_on_process_callback(u8 result, u8 *src, u16 srclen)
{
    wifi_thread_node_param_t *thread_node_param = (wifi_thread_node_param_t *)dev_wifi_get_thread_node_param();
    
    TRACE_INFO("step:%d, result:%d", thread_node_param->m_step, result);
    switch(thread_node_param->m_step)
    {
        case WIFI_ONBOOT_STEP_BOOT:
            if(WIFI_AT_RESULT_TIMEROUT == result)
            {
                TRACE_DBG("timeout, ready to init step");
                dev_com_open(WIFI_PORT, 115200, 8, 'n', 0, 0);
                wifi_module_init();
            }
            break;
    }

    return 0;
}

/**
 * @brief wifi模块上电开机流程
 * @param[in] 无
 * @retval  无
 */
void wifi_power_on_process(void)
{
    TRACE_INFO("enter");
    dev_wifi_set_control_strategy_step(WIFI_MODULE_POWER_ON);
    dev_com_close(WIFI_PORT);
    dev_gpio_direction_output(WIFI_EN_PIN, 1);
    dev_wifi_thread_set_param(WIFI_TASK_MODULES_ONBOOT, 
                                    wifi_power_on_process_callback, 
                                    WIFI_AT_CMD_RESEND_TIMEOUT);
}

/**
 * @brief at指令接收超时的重发策略
 * @param[in] atcmd要执行重发的at命令
 * @retval  无
 */
static void wifi_at_timeout_handle(s8 *atcmd, u32 timeout)
{
    wifi_thread_node_param_t *thread_node_param = (wifi_thread_node_param_t *)dev_wifi_get_thread_node_param();

    if(0 == thread_node_param->m_param1)      //指令超时
    {
        thread_node_param->m_param1 = 1;
        thread_node_param->m_param2++;
        if(WIFI_RESET_THRESHOLD_TIMES == thread_node_param->m_param2)
        {
            TRACE_ERR("contine %d times failed, then power off module to retry", WIFI_RESET_THRESHOLD_TIMES);
            wifi_power_off_process();
        }
        else
        {
            TRACE_DBG("send %s failed %d times", atcmd, thread_node_param->m_param2);
            dev_wifi_thread_set_timeout(WIFI_AT_CMD_RESEND_TIMEOUT);
        }
    }
    else   //休眠超时
    {
        dev_wifi_send_at_cmd(atcmd, AT_API_NO_BLOCK, TRUE);
        dev_wifi_thread_set_timeout(timeout);
        thread_node_param->m_param1 = 0;
    }
}

/**
 * @brief 设置wifi模块控制策略的执行步骤
 * @param[in] control_step  流程步骤
 * @retval  无
 */
void dev_wifi_set_control_strategy_step(WIFI_CONTROL_STRATEGY_STEP_E control_step)
{
    wifi_device_info_t *wifi_device = dev_wifi_get_device();

    if(WIFI_CHECK_CMD == control_step && WIFI_CHECK_CMD != wifi_device->m_control_step)
    {
        wifi_device->m_last_control_step = wifi_device->m_control_step;
    }
    
    //加入异常处理，防止上次是check cmd，然后不能恢复
    if(WIFI_CHECK_CMD == control_step && WIFI_CHECK_CMD == wifi_device->m_last_control_step)
    {
        TRACE_ERR("last step is check cmd, then reset module");
        wifi_device->m_control_step = WIFI_MODULE_POWER_OFF;
    }
    else
    {
        if(wifi_device->m_control_step != control_step)
        {
            wifi_device->m_control_step = control_step;

            if(WIFI_CHECK_CMD != control_step)
            {
                wifi_device->m_control_fail_reason = WF_NO_ERR;
            }
        }
    }
	
}

/**
 * @brief 设置wifi模块控制策略执行步骤的失败原因
 * @param[in] control_fail_reason  流程步骤失败的原因
 * @retval  无
 */
void dev_wifi_set_control_strategy_fail(WIFI_CONTROL_FAIL_REASON_E control_fail_reason)
{
    wifi_device_info_t *wifi_device = dev_wifi_get_device();
    
    wifi_device->m_control_fail_reason = control_fail_reason;
}

/**
 * @brief wifi策略任务开关
 */
s32 dev_wifi_strategy_run_switch(s32 onoff)
{
    g_wifi_strategy_run_flag = onoff;
}

/**
 * @brief 设置策略的运行模式
 */
s32 dev_wifi_set_control_strategy(WIFI_STRATEGY_E wifi_strategy)
{
    g_wifi_strategy = wifi_strategy;
}


/**
 * @brief wifi模块控制策略，包括上电、初始化、注网条件检查、注网等流程控制
 */
void dev_wifi_control_strategy(void)
{
    wifi_device_info_t *device_info = dev_wifi_get_device();

    if(!g_wifi_strategy_run_flag)
    {
        return ;
    }
    
    switch(device_info->m_control_step)
    {
        case WIFI_MODULE_POWER_ON:
            wifi_power_on_process();
            break;

        case WIFI_MODULES_INIT:
            wifi_module_init();
            break;

        case WIFI_MODULES_OPEN_SUCCESS:
            wifi_module_open_success_process();
            break;

        case WIFI_MODULE_POWER_OFF:
            wifi_power_off_process();
            break;

        case WIFI_CHECK_CMD:
            wifi_check_cmd_process();
            break;
    }
}
#endif
