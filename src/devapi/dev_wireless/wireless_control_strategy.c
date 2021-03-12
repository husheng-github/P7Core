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
static u16 g_wireless_detect_times = 0;           //模块检测次数
static u16 g_wireless_check_reg_times = 0;        //检测驻网的次数
extern const u8 wire_exgpio_loginid_tab[WIRE_EXGPIO_MAX];
extern s32 g_wire_exgpio_flg;
static s32 g_wireless_strategy_run_flag = TRUE;

static s32 wireless_check_cmd_process_callback(u8 result, u8 *src, u16 srclen)
{
    wireless_device_info_t *device_info = dev_wireless_get_device();

    TRACE_DBG("result:%d", result);
    if(WIRE_AT_RESULT_OK == result)
    {
        TRACE_INFO("AT cmd recv ok, restore step to %d", device_info->m_last_control_step);
        dev_wireless_set_control_strategy_step(device_info->m_last_control_step);
        dev_wireless_thread_set_result(result);
    }
    else if(WIRE_AT_RESULT_TIMEROUT == result)
    {
        wireless_at_timeout_handle(WIRE_AT_CMD, WIRE_AT_DEFAULT_TIMEOUT);
    }

    return 0;
}

/**
 * @brief 指令异常流程处理
 * @param[in] 无
 * @retval  无
 */
static void wireless_check_cmd_process(void)
{
    wireless_device_info_t *device_info = dev_wireless_get_device();

    TRACE_INFO("app send AT overtime, then check module(step:%d)", device_info->m_control_step);
    //dev_wireless_set_control_strategy_step(WS_CHECK_CMD);
    dev_wireless_send_at_cmd(WIRE_AT_CMD, AT_API_NO_BLOCK, TRUE);
    dev_wireless_thread_set_param(WS_TASK_CHECK_CMD, wireless_check_cmd_process_callback, WIRE_AT_DEFAULT_TIMEOUT);
}

static s32 wireless_power_off_callback(u8 result, u8 *src, u16 srclen)
{
    if(DDI_ETIMEOUT == MIL_power_off_callback(result, src, srclen))
    {
        TRACE_INFO("power off success");
        dev_wireless_set_control_strategy_step(WS_MODULE_POWER_ON);
    }

    return 0;
}

/**
 * @brief 无线模块下电流程
 * @param[in] 无
 * @retval  无
 */
static void wireless_power_off_process(void)
{
    TRACE_INFO("enter");
    dev_wireless_set_control_strategy_step(WS_MODULE_POWER_OFF);
    dev_wireless_thread_set_param(WS_TASK_RESET_MODULE, wireless_power_off_callback, WIRE_AT_POWEROFF_WAIT_TIMEOUT);
    MIL_power_off(AT_API_NO_BLOCK);
}

/**
 * @brief PDP激活成功后的流程
 * @param[in] 无
 * @retval  无
 */
static s32 wireless_active_pdp_success_process(void)
{
    return 0;
}

static s32 wireless_inactive_pdp_process_callback(u8 result, u8 *src, u16 srclen)
{
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();
    wireless_device_info_t *device_info = dev_wireless_get_device();
    s32 tmp_val = 0;

    TRACE_INFO("step:%d, result:%d", thread_node_param->m_step, result);
    switch(thread_node_param->m_step)
    {
        case WIRE_INACTIVE_NETWORK_STEP_GET_IP:
            if(WIRE_AT_RESULT_OTHER == result)
            {
                if(TRUE == MIL_get_local_ip_callback(result, src, srclen))  //IP存在
                {
                    thread_node_param->m_lparam = 1;
                }
                else   //IP不存在
                {
                    thread_node_param->m_lparam = 2;
                }
            }
            else if(WIRE_AT_RESULT_OK == result)
            {
                if(2 == thread_node_param->m_lparam)
                {
                    TRACE_INFO("ip not exist, retry to active pdp");
                    dev_wireless_set_control_strategy_step(WS_SEARCHING_NETWORK);
                    dev_wireless_thread_set_result(result);
                }
                else if(1 == thread_node_param->m_lparam)
                {
                    TRACE_INFO("ip exist, try to close pdp");
                    MIL_inactive_pdp(AT_API_NO_BLOCK);
                    thread_node_param->m_step = WIRE_INACTIVE_NETWORK_STEP_CLOSE_PDP;
                }
            }
            else if(WIRE_AT_RESULT_TIMEROUT == result)
            {
                wireless_at_timeout_handle((s8 *)MIL_get_local_ip_cmd(), WIRE_AT_DEFAULT_TIMEOUT);
            }
            break;

        case WIRE_INACTIVE_NETWORK_STEP_CLOSE_PDP:
            if(WIRE_AT_RESULT_OK == result)
            {
                TRACE_INFO("close pdp cmd return ok, wait result in report handle");
                dev_wireless_thread_set_result(result);
            }
            else if(WIRE_AT_RESULT_TIMEROUT == result)
            {
                wireless_at_timeout_handle((s8 *)MIL_get_inactive_pdp_cmd(), WIRE_AT_DEFAULT_TIMEOUT);
            }
            break;
    }

    return 0;
}

static void wireless_inactive_pdp_process(void)
{
    TRACE_INFO("inactive pdp");
    MIL_get_local_ip(NULL, 0, AT_API_NO_BLOCK);
    dev_wireless_set_control_strategy_step(WS_INACTIVE_PDP);
    dev_wireless_thread_set_param(WS_TASK_INACTIVE_NETWORK, wireless_inactive_pdp_process_callback, WIRE_AT_DEFAULT_TIMEOUT);
}

static s32 wireless_active_pdp_callback(u8 result, u8 *src, u16 srclen)
{
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();
    wireless_device_info_t *device_info = dev_wireless_get_device();
    s32 tmp_val = 0;

    TRACE_INFO("step:%d, result:%d", thread_node_param->m_step, result);
    switch(thread_node_param->m_step)
    {
        case WIRE_ACTIVE_NETWORK_STEP_GET_IP:
            if(WIRE_AT_RESULT_OTHER == result)
            {
                tmp_val = MIL_get_local_ip_callback(result, src, srclen);
                if(TRUE == tmp_val)   //IP存在
                {
                    thread_node_param->m_lparam = 1;
                } 
                else if(FALSE == tmp_val)    //IP不存在
                {
                    thread_node_param->m_lparam = 2;
                }
                else if(DDI_EBUSY == tmp_val)    //Busy(disconnecting or connecting)  等超时进行重发
                {
                    
                }
            }
            else if(WIRE_AT_RESULT_OK == result)
            {
                if(1 == thread_node_param->m_lparam)
                {
                    TRACE_INFO("ip exist, pdp active success");
                    dev_wireless_set_control_strategy_step(WS_ACTIVE_PDP_SUCCESS);
                    dev_wireless_thread_set_result(result);
                }
                else if(2 == thread_node_param->m_lparam)
                {
                    TRACE_INFO("apn:%s, username:%s, password:%s", device_info->m_apn, device_info->m_username, device_info->m_password);
                    thread_node_param->m_step = WIRE_ACTIVE_NETWORK_STEP_SET_APN;
                    dev_wireless_thread_param_clean();
                    MIL_set_apn(device_info->m_apn, device_info->m_username, device_info->m_password, AT_API_NO_BLOCK);
                }
            }
            else if(WIRE_AT_RESULT_TIMEROUT == result)
            {
                wireless_at_timeout_handle((s8 *)MIL_get_local_ip_cmd(), WIRE_AT_DEFAULT_TIMEOUT);
            }
            break;

#if 0  //Mask by xiaohonghui 4.20         中断里不去读flash
        case WIRE_ACTIVE_NETWORK_STEP_GET_APN:
            wireless_get_apn_from_file();
            TRACE_INFO("apn:%s, username:%s, password:%s", device_info->m_apn, device_info->m_username, device_info->m_password);
            thread_node_param->m_step = WIRE_ACTIVE_NETWORK_STEP_SET_APN;
            dev_wireless_thread_param_clean();
            MIL_set_apn(device_info->m_apn, device_info->m_username, device_info->m_password, AT_API_NO_BLOCK);
            break;
#endif
            
        case WIRE_ACTIVE_NETWORK_STEP_SET_APN:
            if(WIRE_AT_RESULT_OK == result)
            {
                TRACE_INFO("set apn success");
                device_info->m_beg_active_pdp_time = dev_user_gettimeID();   //异步等待返回的ip
                dev_wireless_thread_set_result(result);
            }
            else if(WIRE_AT_RESULT_ERROR == result)
            {
                TRACE_INFO("set apn error, then retry to get ip");
                MIL_get_local_ip(NULL, 0, AT_API_NO_BLOCK);
                thread_node_param->m_step = WIRE_ACTIVE_NETWORK_STEP_GET_IP;
                dev_wireless_thread_set_timeout(WIRE_AT_CMD_RESEND_TIMEOUT);
                dev_wireless_thread_param_clean();
            }
            else if(WIRE_AT_RESULT_TIMEROUT == result)
            {
                wireless_at_timeout_handle((s8 *)MIL_get_set_apn_cmd(), WIRE_AT_DEFAULT_TIMEOUT);
            }
            break;
    }

    return 0;
}

/**
 * @brief 激活pdp流程处理
 * @param[in] 无
 * @retval  无
 */
static void wireless_active_pdp_process(void)
{
    TRACE_INFO("enter");
    dev_wireless_set_control_strategy_step(WS_ACTIVE_PDP);
    MIL_get_local_ip(NULL, 0, AT_API_NO_BLOCK);
    dev_wireless_thread_set_param(WS_TASK_ACTIVE_NETWORK, wireless_active_pdp_callback, WIRE_AT_DEFAULT_TIMEOUT);
}

static s32 wireless_searching_network_callback(u8 result, u8 *src, u16 srclen)
{
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();
    wireless_device_info_t *device_info = dev_wireless_get_device();
    s32 tmp_val = 0;

    TRACE_INFO("step:%d, result:%d", thread_node_param->m_step, result);
    switch(thread_node_param->m_step)
    {
        case WIRE_SEARCHING_NETWORK_STEP_CHECK_CARD:
            if(WIRE_AT_RESULT_OTHER == result || WIRE_AT_RESULT_ERROR == result)
            {
                if(0 == thread_node_param->m_wparam)    //指令前部分
                {
                    tmp_val = MIL_get_simcard_state_callback(result, src, srclen);
                    if(WITH_SIMCARD == tmp_val)
                    {
                        device_info->m_simcard_status = WITH_SIMCARD;
                        thread_node_param->m_wparam = 1;
                        break;
                    }
                    else if(WIRE_AT_RESULT_ERROR == result || NOWITH_SIMCARD == tmp_val)
                    {
                        thread_node_param->m_lparam++;
                        if(thread_node_param->m_lparam >= WIRE_SIMCARD_DETECT_TIMES)
                        {
                            TRACE_ERR("Don't detect simcard, then exit");
                            device_info->m_simcard_status = NOWITH_SIMCARD;
                            dev_wireless_set_control_strategy_fail(WS_NO_INSERT_SIM);
                            dev_wireless_thread_set_result(result);
                        }
                    }
                }
            }
            else if(WIRE_AT_RESULT_OK == result)
            {
                if(1 == thread_node_param->m_wparam)    //指令前部分
                {
                    TRACE_DBG("check rplmn");
                    thread_node_param->m_step = WIRE_SEARCHING_NETWORK_CHECK_RPLMN;
                    dev_wireless_thread_param_clean();
                    MIL_check_rplmn(AT_API_NO_BLOCK);
                }
            }
            else if(WIRE_AT_RESULT_TIMEROUT == result)
            {
                if(thread_node_param->m_lparam > 0)    //指令有返回但是无sim卡
                {
                    dev_wireless_send_at_cmd((s8 *)MIL_simcard_state_cmd(), AT_API_NO_BLOCK, TRUE);
                    dev_wireless_thread_set_timeout(WIRE_AT_CMD_RESEND_TIMEOUT);
                }
                else
                {
                    wireless_at_timeout_handle((s8 *)MIL_simcard_state_cmd(), WIRE_AT_DEFAULT_TIMEOUT);
                }
            }
            break;

        case WIRE_SEARCHING_NETWORK_CHECK_RPLMN:
            if(WIRE_AT_RESULT_OTHER == result)
            {
                thread_node_param->m_lparam = MIL_check_rplmn_callback(result, src, srclen);
            }
            else if(WIRE_AT_RESULT_OK == result)
            {
                if(TRUE == thread_node_param->m_lparam)
                {
                    TRACE_DBG("get ccid");
                    thread_node_param->m_step = WIRE_SEARCHING_NETWORK_STEP_GET_CCID;
                    dev_wireless_thread_param_clean();
                    MIL_get_ccid(NULL, 0, AT_API_NO_BLOCK);
                }
                else if(FALSE == thread_node_param->m_lparam)
                {
                    TRACE_INFO("info is 8960, then clear this info");
                    thread_node_param->m_step = WIRE_SEARCHING_NETWORK_CLEAR_RPLMN;
                    dev_wireless_thread_param_clean();
                    MIL_clear_rplmn(AT_API_NO_BLOCK);
                }
            }
            else if(WIRE_AT_RESULT_TIMEROUT == result)
            {
                wireless_at_timeout_handle(MIL_check_rplmn_cmd(), WIRE_AT_DEFAULT_TIMEOUT);
            }
            break;

        case WIRE_SEARCHING_NETWORK_CLEAR_RPLMN:
            if(WIRE_AT_RESULT_OK == result)
            {
                TRACE_DBG("get ccid");
                thread_node_param->m_step = WIRE_SEARCHING_NETWORK_STEP_GET_CCID;
                dev_wireless_thread_param_clean();
                MIL_get_ccid(NULL, 0, AT_API_NO_BLOCK);
            }
            else if(WIRE_AT_RESULT_TIMEROUT == result)
            {
                wireless_at_timeout_handle(MIL_clear_rplmn_cmd(), WIRE_AT_DEFAULT_TIMEOUT);
            }
            break;
            
        case WIRE_SEARCHING_NETWORK_STEP_GET_CCID:
            if(WIRE_AT_RESULT_OTHER == result)
            {
                if(DDI_OK == MIL_get_ccid_callback(result, src, srclen))
                {
                    thread_node_param->m_wparam = 1;
                }
            }
            else if(WIRE_AT_RESULT_OK == result)
            {
                if(thread_node_param->m_wparam)
                {
                    TRACE_DBG("get csq");
                    thread_node_param->m_step = WIRE_SEARCHING_NETWORK_STEP_CHECK_CSQ;
                    dev_wireless_thread_param_clean();
                    MIL_get_csq(NULL, NULL, AT_API_NO_BLOCK);
                }
            }
            else if(WIRE_AT_RESULT_TIMEROUT == result)
            {
                wireless_at_timeout_handle((s8 *)MIL_ccid_cmd(), WIRE_AT_DEFAULT_TIMEOUT);
            }
            break;

        case WIRE_SEARCHING_NETWORK_STEP_CHECK_CSQ:
            if(WIRE_AT_RESULT_OTHER == result)
            {
                if(DDI_OK == MIL_get_csq_callback(result, src, srclen))
                {
                    if(device_info->m_csq > 0 && device_info->m_csq != 99)
                    {
                        thread_node_param->m_wparam = 1;
                    }
                    else
                    {
                        thread_node_param->m_wparam = 2;
                        TRACE_ERR("csq is invalid:%d, retry to get", device_info->m_csq);
                    }
                }
            }
            else if(WIRE_AT_RESULT_OK == result)
            {
                if(1 == thread_node_param->m_wparam)
                {
                    thread_node_param->m_step = WIRE_SEARCHING_NETWORK_SET_4G_REG_PARAM;
                    dev_wireless_thread_param_clean();
                    MIL_set_reg_param(AT_API_NO_BLOCK);
                }
            }
            else if(WIRE_AT_RESULT_TIMEROUT == result)
            {
                if(thread_node_param->m_wparam > 0) //指令有返回
                {
                    MIL_get_csq(NULL, NULL, AT_API_NO_BLOCK);
                }
                else //指令超时
                {
                    wireless_at_timeout_handle((s8 *)MIL_get_csq_cmd(), WIRE_AT_DEFAULT_TIMEOUT);
                }
            }
            break;

        case WIRE_SEARCHING_NETWORK_SET_4G_REG_PARAM:   //4G模块先设置CEREG=2(这样2G卡只有CGREG能注网成功)，2G直接跳过
            if(WIRE_AT_RESULT_OK == result)
            {
                thread_node_param->m_step = WIRE_SEARCHING_NETWORK_CHECK_REG;
                dev_wireless_thread_param_clean();
                MIL_get_net_reg(AT_API_NO_BLOCK);
            }
            else if(WIRE_AT_RESULT_TIMEROUT == result)
            {
                wireless_at_timeout_handle((s8 *)MIL_get_set_reg_param_cmd(), WIRE_AT_DEFAULT_TIMEOUT);
            }
            break;

        case WIRE_SEARCHING_NETWORK_CHECK_REG:
            if(WIRE_AT_RESULT_OTHER == result)
            {
                tmp_val = MIL_get_net_reg_callback(result, src, srclen);
                TRACE_DBG("tmp_val:%d", tmp_val);
                if(TRUE == tmp_val)
                {
                    thread_node_param->m_wparam = 1;
                    thread_node_param->m_lparam++;
                }
                else if(FALSE == tmp_val)
                {
                    thread_node_param->m_wparam = 2;
                    dev_wireless_thread_set_timeout(WIRE_AT_CMD_RESEND_TIMEOUT);
                    g_wireless_check_reg_times++;
                    if(g_wireless_check_reg_times >= WIRE_CHECK_NET_REG_TIMES)
                    {
                        TRACE_ERR("check reg over %s times, then reset module", g_wireless_check_reg_times);
                        wireless_power_off_process();
                    }
                }
            }
            else if(WIRE_AT_RESULT_OK == result)
            {
                //连续2次注网成功，才认为是成功，避免4G卡先查cgreg成功了
                if(1 == thread_node_param->m_wparam && thread_node_param->m_lparam >= 2)
                {
                    thread_node_param->m_step = WIRE_SEARCHING_NETWORK_SET_REG_PARAM;
                    dev_wireless_thread_param_clean();
                    MIL_set_reg_param(AT_API_NO_BLOCK);
                }
            }
            else if(WIRE_AT_RESULT_TIMEROUT == result)
            {
                if(thread_node_param->m_wparam > 0)   //注网不成功后重发注网查询命令
                {
                    MIL_get_net_reg(AT_API_NO_BLOCK);
                }
                else    //未收到指令返回，重发注网查询命令
                {
                    wireless_at_timeout_handle((s8 *)MIL_get_net_reg_cmd(), WIRE_AT_DEFAULT_TIMEOUT);
                }
            }
            break;

        case WIRE_SEARCHING_NETWORK_SET_REG_PARAM:
            if(WIRE_AT_RESULT_OK == result)
            {
                thread_node_param->m_step = WIRE_SEARCHING_NETWORK_GET_MAIL_CELL;
                dev_wireless_thread_param_clean();
                MIL_get_net_reg(AT_API_NO_BLOCK);
            }
            else if(WIRE_AT_RESULT_TIMEROUT == result)
            {
                wireless_at_timeout_handle((s8 *)MIL_get_set_reg_param_cmd(), WIRE_AT_DEFAULT_TIMEOUT);
            }
            break;

        case WIRE_SEARCHING_NETWORK_GET_MAIL_CELL:
            if(WIRE_AT_RESULT_OTHER == result)
            {
                MIL_get_net_reg_callback(result, src, srclen);
            }
            else if(WIRE_AT_RESULT_OK == result)
            {
                thread_node_param->m_step = WIRE_SEARCHING_NETWORK_GET_IMSI;
                dev_wireless_thread_param_clean();
                MIL_get_imsi(NULL, 0, AT_API_NO_BLOCK);
            }
            else if(WIRE_AT_RESULT_TIMEROUT == result)
            {
                wireless_at_timeout_handle((s8 *)MIL_get_net_reg_cmd(), WIRE_AT_DEFAULT_TIMEOUT);
            }
            break;
            
        case WIRE_SEARCHING_NETWORK_GET_IMSI:
            if(WIRE_AT_RESULT_OTHER == result)
            {
                if(DDI_OK == MIL_get_imsi_callback(result, src, srclen))
                {
                    thread_node_param->m_wparam = 1;
                }
            }
            else if(WIRE_AT_RESULT_OK == result)
            {
                if(thread_node_param->m_wparam)
                {
                    thread_node_param->m_step = WIRE_SEARCHING_NETWORK_STEP_SET_FMT;
                    dev_wireless_thread_param_clean();
                    MIL_cfg_rec_data_format(AT_API_NO_BLOCK);
                }
            }
            else if(WIRE_AT_RESULT_TIMEROUT == result)
            {
                wireless_at_timeout_handle((s8 *)MIL_get_imsi_cmd(), WIRE_AT_DEFAULT_TIMEOUT);
            }
            break;


        case WIRE_SEARCHING_NETWORK_STEP_SET_FMT:
            if(WIRE_AT_RESULT_OK == result)
            {
                TRACE_INFO("check net reg success");
                wireless_active_pdp_process();
            }
            else if(WIRE_AT_RESULT_TIMEROUT == result)
            {
                wireless_at_timeout_handle((s8 *)MIL_get_cfg_rec_data_format_cmd(), WIRE_AT_DEFAULT_TIMEOUT);
            }
            break;
    }

    return 0;
}

/**
 * @brief 无线模块驻网条件检查
 * @param[in] 无
 * @retval  无
 */
static void wireless_searching_network_process(void)
{
    TRACE_INFO("enter");
    g_wireless_check_reg_times = 0;
    dev_wireless_set_control_strategy_step(WS_SEARCHING_NETWORK);
    dev_wireless_send_at_cmd((s8 *)MIL_simcard_state_cmd(), AT_API_NO_BLOCK, TRUE);
    dev_wireless_thread_set_param(WS_TASK_SEARCHING_NETWORK, wireless_searching_network_callback, WIRE_AT_DEFAULT_TIMEOUT);
}

static void wireless_cfg_extern_gpio(void)
{
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();

    dev_wireless_thread_param_clean();
    thread_node_param->m_step = WIRE_INIT_GPIO_INIT;
    thread_node_param->m_lparam = 0;     //记录操作的gpio脚
    MIL_cfg_exGPIO(wire_exgpio_loginid_tab[thread_node_param->m_lparam], WIRELESS_GPIO_OUT, AT_API_NO_BLOCK);
}

static s32 wireless_module_init_callback(u8 result, u8 *src, u16 srclen)
{
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();
    wireless_device_info_t *device_info = dev_wireless_get_device();
    s32 i = 0;
    s32 ret = DDI_ERR;
    s8 buff[128] = {0};

    TRACE_INFO("step:%d, result:%d", thread_node_param->m_step, result);
    switch(thread_node_param->m_step)
    {
        case WIRE_INIT_STEP_AT_TEST:
            if(WIRE_AT_RESULT_OK == result)
            {
                TRACE_INFO("wireless module power on success");
                g_wireless_detect_times = 0;
                thread_node_param->m_step = WIRE_INIT_STEP_ATE;
                dev_wireless_thread_param_clean();
                dev_wireless_send_at_cmd(WIRE_ATE_CMD, AT_API_NO_BLOCK, TRUE);
                dev_wireless_thread_set_timeout(WIRE_AT_DEFAULT_TIMEOUT);
            }
            else if(WIRE_AT_RESULT_TIMEROUT == result)
            {
                if(0 == thread_node_param->m_lparam)      //指令超时
                {
                    thread_node_param->m_lparam = 1;
                    g_wireless_detect_times++;
                    if(WIRE_MODULE_DETECT_TIMES <= g_wireless_detect_times)
                    {
                        TRACE_ERR("The machine don't containt wireless module");
                        dev_wireless_set_control_strategy_fail(WS_NO_WIRELESS_MODULE);
                        dev_wireless_thread_set_result(result);
                    }
                    else if(0 == g_wireless_detect_times % WIRE_RESET_THRESHOLD_TIMES)
                    {
                        TRACE_ERR("contine %d times failed, then power off module to retry", WIRE_RESET_THRESHOLD_TIMES);
                        wireless_power_off_process();
                    }
                    else
                    {
                        TRACE_DBG("send AT failed %d times", g_wireless_detect_times);
                        dev_wireless_thread_set_timeout(WIRE_AT_CMD_RESEND_TIMEOUT);
                    }
                }
                else   //休眠超时
                {
                    dev_wireless_send_at_cmd(WIRE_AT_CMD, AT_API_NO_BLOCK, TRUE);
                    dev_wireless_thread_set_timeout(WIRE_AT_DEFAULT_TIMEOUT);
                    thread_node_param->m_lparam = 0;
                }
            }
            break;

        case WIRE_INIT_STEP_ATE:
            if(WIRE_AT_RESULT_OK == result)
            {
                thread_node_param->m_step = WIRE_INIT_GET_MODEL_VER;
                dev_wireless_thread_param_clean();
                dev_wireless_send_at_cmd(WIRE_AT_GMR_CMD, AT_API_NO_BLOCK, TRUE);
                dev_wireless_thread_set_timeout(WIRE_AT_DEFAULT_TIMEOUT);
            }
            else if(WIRE_AT_RESULT_TIMEROUT == result)
            {
                wireless_at_timeout_handle(WIRE_ATE_CMD, WIRE_AT_DEFAULT_TIMEOUT);
            }
            break;

        case WIRE_INIT_GET_MODEL_VER:
            if(WIRE_AT_RESULT_OTHER == result)
            {
                snprintf(device_info->m_module_ver, sizeof(device_info->m_module_ver), "%s", src);
                if(!dev_wireless_module_create(src))
                {
                    TRACE_ERR("unknow module:%s", src);
                    dev_wireless_set_control_strategy_fail(WS_UNKNOW_WIRELESS_MODULE);
                    dev_wireless_thread_set_result(result);
                }
                else
                {
                    thread_node_param->m_lparam = 1;
                }
            }
            else if(WIRE_AT_RESULT_OK == result)
            {
                if(thread_node_param->m_lparam)      //收到版本前部分+ok，才认为指令结束
                {
                    TRACE_DBG("get imei");
                    if(TRUE == MIL_get_flow_control_flag())
                    {
                        thread_node_param->m_step = WIRE_INIT_OPEN_FLOW_CTL;
                        dev_wireless_thread_param_clean();
                        MIL_cfg_flow_control(AT_API_NO_BLOCK);
                    }
                    else
                    {
                        dev_wireless_thread_param_clean();
                        ret = MIL_check_work_mode(AT_API_NO_BLOCK);
                        if(TRUE == ret)  //模块不支持模式检测
                        {
                            TRACE_INFO("module don't need to check work mode");
                            thread_node_param->m_step = WIRE_INIT_GET_IMEI;
                            MIL_get_imei(NULL, 0, AT_API_NO_BLOCK);
                        }
                        else
                        {
                            thread_node_param->m_step = WIRE_INIT_CHECK_WORK_MODE;
                        }
                    }
                }
            }
            else if(WIRE_AT_RESULT_TIMEROUT == result)
            {
                wireless_at_timeout_handle(WIRE_AT_GMR_CMD, WIRE_AT_DEFAULT_TIMEOUT);
            }
            break;

        case WIRE_INIT_OPEN_FLOW_CTL:
            if(WIRE_AT_RESULT_OK == result)
            {
                dev_com_close(WIRELESS_PORT);
                dev_com_open(WIRELESS_PORT, 115200, 8, 'n', 0, 1);
                dev_wireless_thread_param_clean();
                ret = MIL_check_work_mode(AT_API_NO_BLOCK);
                if(TRUE == ret)  //模块不支持模式检测
                {
                    TRACE_INFO("module don't need to check work mode");
                    thread_node_param->m_step = WIRE_INIT_GET_IMEI;
                    MIL_get_imei(NULL, 0, AT_API_NO_BLOCK);
                }
                else
                {
                    thread_node_param->m_step = WIRE_INIT_CHECK_WORK_MODE;
                }
            }
            else if(WIRE_AT_RESULT_TIMEROUT == result)
            {
                wireless_at_timeout_handle("AT&K3", WIRE_AT_DEFAULT_TIMEOUT);
            }
            break;

        case WIRE_INIT_CHECK_WORK_MODE:
            ret = MIL_check_work_mode_callback(result, src, srclen);
            if(WIRE_AT_RESULT_OK == result)
            {
                if(DDI_OK == ret)  //网络制式正确
                {
                    TRACE_INFO("work mode is ok, then get imei");
                    dev_wireless_thread_param_clean();
                    thread_node_param->m_step = WIRE_INIT_GET_IMEI;
                    MIL_get_imei(NULL, 0, AT_API_NO_BLOCK);
                }
                else
                {
                    TRACE_INFO("work mode is invalid, then change it");
                    dev_wireless_thread_param_clean();
                    thread_node_param->m_step = WIRE_INIT_SET_WORK_MODE;
                    MIL_set_work_mode(AT_API_NO_BLOCK);
                }
            }
            else if(WIRE_AT_RESULT_TIMEROUT == result)
            {
                wireless_at_timeout_handle(MIL_get_check_work_mode_cmd(), WIRE_AT_DEFAULT_TIMEOUT);
            }
            break;

        case WIRE_INIT_SET_WORK_MODE:
            if(WIRE_AT_RESULT_OK == result)
            {
                TRACE_INFO("work mode set ok, then get imei");
                dev_wireless_thread_param_clean();
                thread_node_param->m_step = WIRE_INIT_GET_IMEI;
                MIL_get_imei(NULL, 0, AT_API_NO_BLOCK);
            }
            else if(WIRE_AT_RESULT_TIMEROUT == result)
            {
                wireless_at_timeout_handle(MIL_get_set_work_mode_cmd(), WIRE_AT_DEFAULT_TIMEOUT);
            }
            break;
            
        case WIRE_INIT_GET_IMEI:
            if(WIRE_AT_RESULT_OTHER == result)
            {
                MIL_get_imei_callback(result, src, srclen);
            }
            else if(WIRE_AT_RESULT_OK == result)
            {
#if(WIRELESS_USE_GPIO == TRUE)
               wireless_cfg_extern_gpio();
#elif(WIRELESS_USE_EXTERANL_AMP == TRUE)
                TRACE_INFO("set exteranl amp");
                dev_wireless_thread_param_clean();
                thread_node_param->m_step = WIRE_INIT_SET_USE_EXTERANL_AMP;
                MIL_set_useextamp(AT_API_NO_BLOCK);
#else
                wireless_searching_network_process();
#endif
            }
            else if(WIRE_AT_RESULT_TIMEROUT == result)
            {
                wireless_at_timeout_handle(MIL_get_imei_cmd(), WIRE_AT_DEFAULT_TIMEOUT);
            }
            break;

#if(WIRELESS_USE_GPIO == TRUE)
        case WIRE_INIT_GPIO_INIT:
            if((WIRE_AT_RESULT_OK == result)
             ||(WIRE_AT_RESULT_OTHER == result))
            {
                i = thread_node_param->m_lparam;
                thread_node_param->m_step = WIRE_INIT_GPIO_SET_VAL;
                thread_node_param->m_wparam = 0;
                MIL_set_exGPIO(wire_exgpio_loginid_tab[i], (g_wire_exgpio_flg>>i)&0x01, AT_API_NO_BLOCK);
            }
            else if(WIRE_AT_RESULT_ERROR == result)
            {
                //不支持GPIO控制指令
                //检查和注册网络 
                TRACE_ERR("module don't support gpio cfg");
                wireless_searching_network_process();
            }
            else if(WIRE_AT_RESULT_TIMEROUT == result)
            {
                thread_node_param->m_wparam++;
                if(thread_node_param->m_wparam < WIRE_RESET_THRESHOLD_TIMES)
                {
                    MIL_cfg_exGPIO(wire_exgpio_loginid_tab[thread_node_param->m_lparam], WIRELESS_GPIO_OUT, AT_API_NO_BLOCK);
                }
                else
                {
                    TRACE_ERR("contine %d times failed, then power off module to retry", WIRE_RESET_THRESHOLD_TIMES);
                    wireless_power_off_process();
                }
            }
            break;

        case WIRE_INIT_GPIO_SET_VAL:
            if((WIRE_AT_RESULT_OK == result)
             ||(WIRE_AT_RESULT_OTHER == result))
            {
                thread_node_param->m_lparam++;
                if(thread_node_param->m_lparam >= WIRE_EXGPIO_MAX)
                {
                    TRACE_INFO("finish module gpio cfg");
                    dev_wireless_thread_param_clean();
                    thread_node_param->m_step = WIRE_INIT_GET_APP_VER;
                    MIL_get_appver(AT_API_NO_BLOCK);
                }
                else
                {
                    thread_node_param->m_step = WIRE_INIT_GPIO_INIT;
                    thread_node_param->m_wparam = 0;
                    MIL_cfg_exGPIO(wire_exgpio_loginid_tab[thread_node_param->m_lparam], WIRELESS_GPIO_OUT, AT_API_NO_BLOCK);
                }
            }
            else if(WIRE_AT_RESULT_ERROR == result)
            {
                //不支持GPIO控制指令
                //检查和注册网络 
                TRACE_ERR("module don't support gpio cfg");
                wireless_searching_network_process();
            }
            else if(WIRE_AT_RESULT_TIMEROUT == result)
            {
                thread_node_param->m_wparam++;
                if(thread_node_param->m_wparam < WIRE_RESET_THRESHOLD_TIMES)
                {
                    MIL_set_exGPIO(wire_exgpio_loginid_tab[i], (g_wire_exgpio_flg>>i)&0x01, AT_API_NO_BLOCK);
                }
                else
                {
                    TRACE_ERR("contine %d times failed, then power off module to retry", WIRE_RESET_THRESHOLD_TIMES);
                    wireless_power_off_process();
                }
            }
            break;

        case WIRE_INIT_GET_APP_VER:
            if(WIRE_AT_RESULT_OTHER == result)
            {
                if(DDI_OK == MIL_get_appver_callback(result, src, srclen))
                {
                    thread_node_param->m_lparam = 1;
                    wireless_searching_network_process();
                }
            }
            else if(WIRE_AT_RESULT_TIMEROUT == result)
            {
                thread_node_param->m_wparam++;
                if(thread_node_param->m_wparam < WIRE_RESET_THRESHOLD_TIMES)
                {
                    MIL_get_appver(AT_API_NO_BLOCK);
                }
                else
                {
                    TRACE_ERR("contine %d times failed, then power off module to retry", WIRE_RESET_THRESHOLD_TIMES);
                    wireless_power_off_process();
                }
            }
            break;
#endif

#if(WIRELESS_USE_EXTERANL_AMP == TRUE)
        case WIRE_INIT_SET_USE_EXTERANL_AMP:
            if(WIRE_AT_RESULT_OK == result)
            {
                dev_wireless_thread_param_clean();
                thread_node_param->m_step = WIRE_INIT_CFG_AUDIO_PLAY_PATH;
                MIL_cfg_audio_play_path();
            }
            else if(WIRE_AT_RESULT_TIMEROUT == result)
            {
                wireless_at_timeout_handle(MIL_get_set_useextamp_cmd(), WIRE_AT_DEFAULT_TIMEOUT);
            }
            break;

        case WIRE_INIT_CFG_AUDIO_PLAY_PATH:
            if(WIRE_AT_RESULT_OK == result)
            {
                dev_wireless_thread_param_clean();
                thread_node_param->m_step = WIRE_INIT_CLOSE_AUDIO_PLAY_SLEEP;
                if(TRUE == MIL_close_audio_play_sleep())
                {
                    dev_wireless_thread_param_clean();
                    thread_node_param->m_step = WIRE_INIT_CFG_AUDIO_MUT;
                    MIL_cfg_audio_mut();
                }
            }
            else if(WIRE_AT_RESULT_TIMEROUT == result)
            {
                wireless_at_timeout_handle(MIL_get_cfg_audio_play_path_cmd(), WIRE_AT_DEFAULT_TIMEOUT);
            }
            break;

        case WIRE_INIT_CLOSE_AUDIO_PLAY_SLEEP:
            if(WIRE_AT_RESULT_OK == result)
            {
                dev_wireless_thread_param_clean();
                thread_node_param->m_step = WIRE_INIT_CFG_AUDIO_MUT;
                MIL_cfg_audio_mut();
            }
            else if(WIRE_AT_RESULT_TIMEROUT == result)
            {
                wireless_at_timeout_handle(MIL_get_close_audio_play_sleep_cmd(), WIRE_AT_DEFAULT_TIMEOUT);
            }
            break;

        case WIRE_INIT_CFG_AUDIO_MUT:
            if(WIRE_AT_RESULT_OK == result)
            {
                dev_wireless_thread_param_clean();
                thread_node_param->m_step = WIRE_INIT_CFG_VOL;
                MIL_set_audio_volume(device_info->m_audio_info.m_audio_vol, AT_API_NO_BLOCK);
            }
            else if(WIRE_AT_RESULT_TIMEROUT == result)
            {
                wireless_at_timeout_handle(MIL_get_cfg_audio_mut(), WIRE_AT_DEFAULT_TIMEOUT);
            }
            break;

       case WIRE_INIT_CFG_VOL:
            if(WIRE_AT_RESULT_OK == result || strstr(src, "CLVLOK"))
            {
                TRACE_DBG("vol is %d", device_info->m_audio_info.m_audio_vol);
            #if(WIRELESS_USE_UART_RING == TRUE)
                dev_wireless_thread_param_clean();
                thread_node_param->m_step = WIRE_INIT_SET_GTLPM_MODE;
                MIL_set_gtlpm_mode(AT_API_NO_BLOCK);
            #else
                wireless_searching_network_process();
            #endif
            }
            else if(WIRE_AT_RESULT_TIMEROUT == result)
            {
                memset(buff, 0, sizeof(buff));
                snprintf(buff, sizeof(buff), "AT+CLVL=%d", device_info->m_audio_info.m_audio_vol);
                wireless_at_timeout_handle(buff, WIRE_AT_DEFAULT_TIMEOUT);
            }
            break;
#endif

#if(WIRELESS_USE_UART_RING == TRUE)
         case WIRE_INIT_SET_GTLPM_MODE:
            if(WIRE_AT_RESULT_OK == result)
            {
                dev_wireless_thread_param_clean();
                thread_node_param->m_step = WIRE_INIT_SET_CSCLK;
                MIL_set_csclk(AT_API_NO_BLOCK);
            }
            else if(WIRE_AT_RESULT_TIMEROUT == result)
            {
                wireless_at_timeout_handle(MIL_get_gtlpm_mode_cmd(), WIRE_AT_DEFAULT_TIMEOUT);
            }
            break;
        case WIRE_INIT_SET_CSCLK:
            if(WIRE_AT_RESULT_OK == result)
            {
                dev_wireless_thread_param_clean();
                thread_node_param->m_step = WIRE_INIT_SET_USE_UARTRING;
                MIL_set_useuartring(AT_API_NO_BLOCK);
            }
            else if(WIRE_AT_RESULT_TIMEROUT == result)
            {
                wireless_at_timeout_handle(MIL_get_set_csclk_cmd(), WIRE_AT_DEFAULT_TIMEOUT);
            }
            break;
            
        case WIRE_INIT_SET_USE_UARTRING:
            if(WIRE_AT_RESULT_OK == result)
            {
                wireless_searching_network_process();
            }
            else if(WIRE_AT_RESULT_TIMEROUT == result)
            {
                wireless_at_timeout_handle(MIL_get_uartring_mode_cmd(), WIRE_AT_DEFAULT_TIMEOUT);
            }
            break;
#endif
    }

    return 0;
}

/**
 * @brief 无线模块初始化流程
 * @param[in] 无
 * @retval  无
 */
static void wireless_module_init(void)
{
    TRACE_INFO("enter");
    dev_wireless_set_control_strategy_step(WS_MODULES_INIT);
    dev_wireless_send_at_cmd(WIRE_AT_CMD, AT_API_NO_BLOCK, TRUE);
    dev_wireless_thread_set_param(WS_TASK_MODULES_INIT, wireless_module_init_callback, WIRE_AT_DEFAULT_TIMEOUT);
}

static s32 wireless_power_on_process_callback(u8 result, u8 *src, u16 srclen)
{
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();
    
    TRACE_INFO("step:%d, result:%d", thread_node_param->m_step, result);
    switch(thread_node_param->m_step)
    {
        case WIRE_ONBOOT_STEP_BOOT:
            if(WIRE_AT_RESULT_TIMEROUT == result)
            {
                if(!WIRELESS_WITH_POWERKEY)   //沒有电源脚，上电等3秒，再发AT
                {
                    
                    dev_com_close(WIRELESS_PORT);
                    dev_gpio_set_value(GPRS_POWEREN_PIN, 1);             //上电
                    dev_wireless_set_power_flag(TRUE);
                    dev_wireless_thread_set_timeout(WIRE_AT_POWERON_TIMEOUT);   //1s
                    //thread_node_param->m_step++;
                    thread_node_param->m_step = WIRE_ONBOOT_STEP_BOOT1;
                }
                else
                {
                    dev_gpio_set_value(GPRS_POWERON_PIN, 0);
                    dev_wireless_thread_set_timeout(WIRE_AT_POWERON_TIMEOUT);
                    //thread_node_param->m_step++;
                    thread_node_param->m_step = WIRE_ONBOOT_STEP_POWER;
                }
            }
          #if 0
            else if(WIRE_AT_RESULT_MODULE_READY == result)
            {
                if(!WIRELESS_WITH_POWERKEY)   //沒有电源脚，直接去到init流程
                {
                    TRACE_INFO("recv at command ready");
                    wireless_module_init();
                }
            }
          #endif
            break;
       case WIRE_ONBOOT_STEP_BOOT1:
            if(WIRE_AT_RESULT_TIMEROUT == result)
            {
                //只有T8这种更没有电源脚才会执行到这里
                dev_com_open(WIRELESS_PORT, 115200, 8, 'n', 0, 0);   //sxl20200814开流控
                dev_wireless_thread_set_timeout(WIRE_AT_POWERON_TIMEOUT*6);   //接收READY
                thread_node_param->m_step = WIRE_ONBOOT_STEP_POWER;
            }
            break;

        case WIRE_ONBOOT_STEP_POWER:
            if(WIRE_AT_RESULT_TIMEROUT == result)
            {
                if(WIRELESS_WITH_POWERKEY)
                {
                    dev_gpio_set_value(GPRS_POWERON_PIN, 1);
                }
                
                wireless_module_init();
            }
            else if(WIRE_AT_RESULT_MODULE_READY == result)
            {
                if(!WIRELESS_WITH_POWERKEY)   //沒有电源脚，收到command ready，说明模块启动正常
                {
                    TRACE_INFO("recv at command ready");
                    wireless_module_init();
                }
            }
            break;
    }

    return 0;
}

/**
 * @brief 无线模块上电开机流程
 * @param[in] 无
 * @retval  无
 */
void wireless_power_on_process(void)
{
    TRACE_INFO("enter");

    if(0/*2 == dev_misc_reset_get_from_flash()*/)
    {
        TRACE_INFO("watch dog reset, jump to init step");
        dev_wireless_set_control_strategy_step(WS_MODULES_INIT);
        return ;
    }
    else
    {
        TRACE_INFO("normal power on module");
    }
    
    dev_wireless_set_control_strategy_step(WS_MODULE_POWER_ON);
    if(WIRELESS_WITH_POWERKEY)
    {
        dev_gpio_direction_output(GPRS_POWEREN_PIN, 1); 
        dev_gpio_direction_output(GPRS_POWERON_PIN, 1);
        dev_wireless_thread_set_param(WS_TASK_MODULES_ONBOOT, 
                                        wireless_power_on_process_callback, 
                                        WIRE_AT_CMD_RESEND_TIMEOUT);
    }
    else      //模块通过硬件上电的，等1秒
    {
        dev_wireless_thread_set_param(WS_TASK_MODULES_ONBOOT, 
                                        wireless_power_on_process_callback, 
                                        WIRE_AT_CMD_RESEND_TIMEOUT*2);
    }
}

/**
 * @brief at指令接收超时的重发策略
 * @param[in] atcmd要执行重发的at命令
 * @retval  无
 */
static void wireless_at_timeout_handle(s8 *atcmd, u32 timeout)
{
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();

    if(0 == thread_node_param->m_param1)      //指令超时
    {
        thread_node_param->m_param1 = 1;
        thread_node_param->m_param2++;
#ifdef IPV6_TEST
        if(0)
#else
        if(WIRE_RESET_THRESHOLD_TIMES == thread_node_param->m_param2)
#endif
        {
            TRACE_ERR("contine %d times failed, then power off module to retry", WIRE_RESET_THRESHOLD_TIMES);
            wireless_power_off_process();
        }
        else
        {
            TRACE_DBG("send %s failed %d times", atcmd, thread_node_param->m_param2);
            dev_wireless_thread_set_timeout(WIRE_AT_CMD_RESEND_TIMEOUT);
        }
    }
    else   //休眠超时
    {
        dev_wireless_send_at_cmd(atcmd, AT_API_NO_BLOCK, TRUE);
        dev_wireless_thread_set_timeout(timeout);
        thread_node_param->m_param1 = 0;
    }
}

/**
 * @brief 设置无线模块控制策略的执行步骤
 * @param[in] control_step  流程步骤
 * @retval  无
 */
void dev_wireless_set_control_strategy_step(WIRE_CONTROL_STRATEGY_STEP_E control_step)
{
    wireless_device_info_t *wireless_device = dev_wireless_get_device();

    if(WS_CHECK_CMD == control_step && WS_CHECK_CMD != wireless_device->m_control_step)
    {
        wireless_device->m_last_control_step = wireless_device->m_control_step;
    }
    
    //加入异常处理，防止上次是check cmd，然后不能恢复
    if(WS_CHECK_CMD == control_step && WS_CHECK_CMD == wireless_device->m_last_control_step)
    {
        TRACE_ERR("last step is check cmd, then reset module");
        wireless_device->m_control_step = WS_MODULE_POWER_OFF;
    }
    else
    {
        wireless_device->m_control_step = control_step;

        if(WS_CHECK_CMD != control_step)
        {
            wireless_device->m_control_fail_reason = WS_NO_ERR;
        }
    }
	
}

/**
 * @brief 设置无线模块控制策略执行步骤的失败原因
 * @param[in] control_fail_reason  流程步骤失败的原因
 * @retval  无
 */
void dev_wireless_set_control_strategy_fail(WIRE_CONTROL_FAIL_REASON_E control_fail_reason)
{
    wireless_device_info_t *wireless_device = dev_wireless_get_device();
    
    wireless_device->m_control_fail_reason = control_fail_reason;
}

/**
 * @brief 无线策略任务开关
 */
s32 dev_wireless_strategy_run_switch(s32 onoff)
{
    g_wireless_strategy_run_flag = onoff;
}

/**
 * @brief 无线模块控制策略，包括上电、初始化、注网条件检查、注网等流程控制
 */
void dev_wireless_control_strategy(void)
{
    wireless_device_info_t *device_info = dev_wireless_get_device();

    if(!g_wireless_strategy_run_flag)
    {
        return ;
    }
    
    switch(device_info->m_control_step)
    {
        case WS_MODULE_POWER_ON:
            wireless_power_on_process();
            break;

        case WS_MODULES_INIT:
            wireless_module_init();
            break;

        case WS_SEARCHING_NETWORK:
            device_info->m_beg_active_pdp_time = 0;
            wireless_searching_network_process();
            break;

        case WS_ACTIVE_PDP:
            if(device_info->m_beg_active_pdp_time > 0)
            {
                if(dev_user_querrytimer(device_info->m_beg_active_pdp_time, WIRE_AT_ACTIVE_PDP_TIMEOUT))
                {
                    TRACE_INFO("active pdp overtime:%d, then reset module");
                    wireless_power_off_process();
                }
            }
            else
            {
                wireless_active_pdp_process();
            }
            break;

        case WS_ACTIVE_PDP_SUCCESS:
            device_info->m_beg_active_pdp_time = 0;
            wireless_active_pdp_success_process();
            break;

        case WS_INACTIVE_PDP:
            wireless_inactive_pdp_process();
            break;

        case WS_MODULE_POWER_OFF:
            wireless_power_off_process();
            break;

        case WS_CHECK_CMD:
            wireless_check_cmd_process();
            break;
    }
}

#endif
