#include "devglobal.h"
#include "drv_wifi_io.h"
#include "dev_wifi_thread.h"
#include "dev_wifi.h"
#include "wifi_control_strategy.h"
#include "wifi_data_handle.h"
#include "dev_wifi_wrapper.h"

#if (WIFI_EXIST == 1)
static wifi_device_info_t g_wifi_device_info;
static s32 g_wifi_init_flag = FALSE;
static u32 g_get_csq_last_ticks = 0;
static wifi_net_info_t g_wifi_net_info;
static s32 g_client_socket_id[WIFI_SOCKET_MAX];     //记录tcp 客户端socketid
static s32 g_wifi_exist = FALSE;

wifi_device_info_t *dev_wifi_get_device(void)
{
    return &g_wifi_device_info;
}

static s32 wifi_comom_response_callback(u8 result, u8 *src, u16 srclen)
{
    s8 *pstr = NULL;
    
    if(result == WIFI_AT_RESULT_TIMEROUT || result == WIFI_AT_RESULT_OK || result == WIFI_AT_RESULT_ERROR)
    {
        dev_wifi_thread_set_result(result);
    }
    
    return 0;
}

/**
 * @brief 从文件获取拨号参数
 * @param[in] 无
 * @retval  DDI_OK 从文件获取成功
 * @retval  DDI_ERR 使用默认apn
 */
static s32 wifi_get_ap_from_file(void)
{
    s32 ret = DDI_ERR;
    wifi_device_info_t *device_info = dev_wifi_get_device();
    s8 buff[256] = {0};
    s8 dst[3][SPLIT_PER_MAX] = {0};
    s32 fd = 0;

    do{
        fd = vfs_open(WIFI_AP_INFO_FILE, "r");
        if(fd < 0)
        {
            TRACE_ERR("open %s fail", WIFI_AP_INFO_FILE);
            ret = DDI_EIO;
            break;
        }
        
        memset(buff, 0, sizeof(buff));
        ret = vfs_read(fd, buff, sizeof(buff));
        vfs_close(fd);
        if(ret <= 0)
        {
            ret = DDI_ERR;
            break;
        }

        memset(dst, 0, sizeof(dst));
        ret = dev_maths_split(dst, sizeof(dst)/sizeof(dst[0]), buff, ",", SPLIT_NOIGNOR);
        if(ret != sizeof(dst)/sizeof(dst[0]))
        {
            ret = DDI_ERR;
            break;
        }

        snprintf(device_info->m_ssid, sizeof(device_info->m_ssid), "%s", dst[0]);
        snprintf(device_info->m_password, sizeof(device_info->m_password), "%s", dst[1]);
        snprintf(device_info->m_bssid, sizeof(device_info->m_bssid), "%s", dst[2]);
        ret = DDI_OK;
    }while(0);

    if(ret != DDI_OK)
    {
        snprintf(device_info->m_ssid, sizeof(device_info->m_ssid), "Trendit");
        snprintf(device_info->m_password, sizeof(device_info->m_password), "trendit123");
    }

    return ret;
}

/**
 * @brief 保存wifi参数
 * @param[in] 无
 * @retval  DDI_OK 从文件获取成功
 * @retval  DDI_ERR 使用默认apn
 */
static s32 wifi_save_ap_to_file(void)
{
    s32 ret = DDI_ERR;
    wifi_device_info_t *device_info = dev_wifi_get_device();
    s8 buff[256] = {0};
    s32 fd = 0;

    do{
        fd = vfs_open(WIFI_AP_INFO_FILE, "w");
        if(fd < 0)
        {
            TRACE_ERR("open %s fail", WIFI_AP_INFO_FILE);
            ret = DDI_EIO;
            break;
        }
        
        memset(buff, 0, sizeof(buff));
        snprintf(buff, sizeof(buff), "%s,%s,%s", device_info->m_ssid,device_info->m_password,device_info->m_bssid);
        ret = vfs_write(fd, buff, strlen(buff));
        TRACE_DBG("ret:%d", ret);
        vfs_close(fd);
        ret = DDI_OK;
    }while(0);

    return ret;
}

/**
 * @brief 通过应用层的socket sn找到socket资源
 * @retval 对应socket sn的资源
 */
static wifi_socket_info_t *wifi_get_socket_info_by_index(s32 index)
{
    if(index < 0 || index >= WIFI_SOCKET_MAX)
    {
        return NULL;
    }
    
    return g_wifi_device_info.m_socket_info[index];
}

/**
 * @brief 释放socket套接字资源
 * @retval  无
 */
static void wifi_socket_info_destroy(wifi_socket_info_t* thiz)
{
    if(thiz->m_read_buffer.m_buf != NULL) 
    {
        k_free(thiz->m_read_buffer.m_buf);
        thiz->m_read_buffer.m_buf = NULL;
    }

    memset(thiz->m_serveraddr, 0, sizeof(thiz->m_serveraddr));
    thiz->m_serverport = 0;
    k_free(thiz);
    thiz = NULL;
}


/**
 * @brief 修改套接字状态
 * @param[in] socket_id:套接字id
 * @param[in] socket_status:套接字状态
 * @retval ddi_ok 修改成功
 * @retval ddi_err 没找到此套接字
 */
s32 dev_wifi_set_socket_status(s32 socket_id, TCP_SOCKET_STATUS_E socket_status)
{
    s32 i = 0;
    s32 ret = DDI_ERR;
    
    for(i = 0; i < WIFI_SOCKET_MAX; i++)
    {
        if(0xff == socket_id)     //将所有套接字设置为断开，在收到协议栈关闭时调用
        {
            if(g_wifi_device_info.m_socket_info[i])
            {
                if(SOCKET_STATUS_CONNECTING == g_wifi_device_info.m_socket_info[i]->m_status
                    || SOCKET_STATUS_CONNECTED == g_wifi_device_info.m_socket_info[i]->m_status)
                {
                    TRACE_INFO("socketid:%d, change status to:%d", 
                                                g_wifi_device_info.m_socket_info[i]->m_socket_id, 
                                                socket_status);
                    g_wifi_device_info.m_socket_info[i]->m_status = socket_status;
                    ret = DDI_OK;
                }
            }
        }
        else
        {
            if(g_wifi_device_info.m_socket_info[i] 
                && g_wifi_device_info.m_socket_info[i]->m_socket_id == socket_id)
            {
                TRACE_INFO("socketid:%d, change status to:%d", socket_id, socket_status);
                g_wifi_device_info.m_socket_info[i]->m_status = socket_status;
                ret = DDI_OK;
                break;
            }
        }
    }
    
    return ret;
}

/**
 * @brief 通过socketid释放socket套接字资源
 * @retval ddi_ok 找到并释放成功
 * @retval ddi_err 没有此socketid
 */
s32 dev_wifi_socket_info_destroy_by_id(s32 socket_id)
{
    s32 i = 0;
    s32 ret = DDI_ERR;
    
    for(i = 0; i < WIFI_SOCKET_MAX; i++)
    {
        if(g_wifi_device_info.m_socket_info[i] 
            && g_wifi_device_info.m_socket_info[i]->m_socket_id == socket_id)
        {
            wifi_socket_info_destroy(g_wifi_device_info.m_socket_info[i]);
            g_wifi_device_info.m_socket_info[i] = NULL;
            ret = DDI_OK;
            break;
        }
    }
    
    return ret;
}

/**
 * @brief 通过socketid获取socket info的序号
 * @retval >=0 序号
 * @retval ddi_err 没有此socketid
 */
s32 dev_wifi_socket_info_sn_by_id(s32 socket_id)
{
    s32 i = 0;
    s32 ret = DDI_ERR;
    
    for(i = 0; i < WIFI_SOCKET_MAX; i++)
    {
        if(g_wifi_device_info.m_socket_info[i] 
            && g_wifi_device_info.m_socket_info[i]->m_socket_id == socket_id)
        {
            ret = i;
            break;
        }
    }
    
    return ret;
}


/**
 * @brief 创建socket套接字资源
 * @retval  socket套接字资源
 */
static wifi_socket_info_t *wifi_socket_info_create(u8 *param, u16 port)
{
    wifi_socket_info_t* thiz = (wifi_socket_info_t*)k_malloc(sizeof(wifi_socket_info_t));
    s32 ret = DDI_ERR;
    
    if(thiz == NULL)
    {
        TRACE_ERR("gprs_socket_info_create failed");
        return NULL;
    }

    memset(thiz, 0, sizeof(wifi_socket_info_t));
    if(0 == strcmp(param, "tms.trenditadmin.com")) //tms buff改为8K
    {
        ret = dev_circlequeue_init(&(thiz->m_read_buffer), WIFI_SOCKET_RCV_BUFLEN*4);
    }
    else
    {
        ret = dev_circlequeue_init(&(thiz->m_read_buffer), WIFI_SOCKET_RCV_BUFLEN);
    }
    if(ret < 0)
    {
        TRACE_ERR("gprs_socket_info_create failed for read buffer");
        wifi_socket_info_destroy(thiz);
        return NULL;
    }

    memset(thiz->m_serveraddr, 0, sizeof(thiz->m_serveraddr));
    thiz->m_serverport = 0;
    thiz->total_receive = 0;
    thiz->m_status = SOCKET_STATUS_NONE;
    thiz->m_socket_id = -1;
    return thiz;
}

/**
 * @brief 获取可以用套接字索引
 * @param[in] param:服务器ip或域名
 * @param[in] port:服务器端口
 * @retval  >=0 可以用索引
 * @retval  <0 无可以用索引
 */
static s32 wifi_get_available_socket_info_index(u8 *param, u16 port)
{
    s32 i = 0, j = 0;
    s32 avlid_sockid_list[WIFI_SOCKET_MAX] = {0};   //模块端可用的socket id
    s32 ret = DDI_ERR;
    s32 exist_flag = FALSE;    //socketid是否在模块端可用清单里  TRUE 存在   ,FALSE不存在
    s32 flag = 0;              //1:仅释放socket资源   2:关闭模块端socket和socket资源

    for(i = 0; i < WIFI_SOCKET_MAX; i++)
    {
        if(!g_wifi_device_info.m_socket_info[i])
        {
            TRACE_ERR("socketinfo:%d is NULL in local, create it", i+1);
            flag = 2;
            break;
        }
        else
        {
            if(g_wifi_device_info.m_socket_info[i]->m_socket_id < 0)  //本地socketid无效
            {
                TRACE_ERR("socketid:%d is not exist in local, destory it", i+1);
                flag = 2;
                break;
            }
            else
            {
                if(0 == g_wifi_device_info.m_socket_info[i]->m_status) //状态空闲
                {
                    TRACE_ERR("socketid:%d state is free, use it", i+1);
                    flag = 2;
                    break;
                }

                if(0 == strcmp(param, g_wifi_device_info.m_socket_info[i]->m_serveraddr) 
                    && port == g_wifi_device_info.m_socket_info[i]->m_serverport)
                {
                    TRACE_ERR("System exist the same socket, close before:%s, %d", 
                                    g_wifi_device_info.m_socket_info[i]->m_serveraddr,
                                    g_wifi_device_info.m_socket_info[i]->m_serverport);
                    flag = 2;
                    break;
                }
            }
        }
    }
    
    if(i < WIFI_SOCKET_MAX)
    {
        if(2 == flag)
        {
            ret = dev_wifi_socket_close(i);
            if(DDI_OK != ret && DDI_EINVAL != ret)  //关闭socket
            {
                TRACE_ERR("close socketid:%d failed", i);
                return -1;
            }
        }
        
        if(2 == flag)
        {
            TRACE_DBG("i:%d", i);
            if(!g_wifi_device_info.m_socket_info[i])
            {
                g_wifi_device_info.m_socket_info[i] = wifi_socket_info_create(param, port);
                if(g_wifi_device_info.m_socket_info[i])
                {
                    g_wifi_device_info.m_socket_info[i]->m_socket_id = i;
                    return i;
                }
            }
            else
            {
                dev_circlequeue_clear(&(g_wifi_device_info.m_socket_info[i]->m_read_buffer));
                g_wifi_device_info.m_socket_info[i]->total_receive = 0;
                g_wifi_device_info.m_socket_info[i]->m_status = SOCKET_STATUS_NONE;
                g_wifi_device_info.m_socket_info[i]->m_socket_id = i;
                memset(g_wifi_device_info.m_socket_info[i]->m_serveraddr, 0, sizeof(g_wifi_device_info.m_socket_info[i]->m_serveraddr));
                g_wifi_device_info.m_socket_info[i]->m_serverport = 0;
                return i;
            }
        }
    }
    
    return -1;  
}

/**
 * @brief 获取可以用套接字索引
 * @param[in] cmd:要发送的命令
 * @param[in] at_api_get_type:阻塞还是非阻塞发送
 * @param[in] end_flag:是否发送换行符   TRUE:发送换行符号  FALSE:不发送换行符
 * @retval  >=0 发送的数据长度
 * @retval  DDI_ETRANSPORT 发送数据失败
 */
s32 dev_wifi_send_at_cmd(s8 *cmd, AT_API_GET_TYPE_E at_api_get_type, s32 end_flag)
{
    s32 ret = DDI_ERR;
    wifi_device_info_t *device_info = dev_wifi_get_device();
    s32 len = 0;

    do{
        //外部接口主动获取时，线程任务不执行，防止发送部分数据时被中断打断，然后在中断里也发命令了
        if(AT_API_BLOCK == at_api_get_type)
        {
            dev_wifi_thread_run_switch(FALSE);
        }

        if(strlen(cmd) > 4)
        {
            TRACE_INFO(">> %s", cmd);
        }
        else if(2 == strlen(cmd) && 0 == strcmp(cmd, WIFI_AT_CMD))
        {
            TRACE_DBG(">> %s", cmd);
        }
        
        ret = dev_com_write(WIFI_PORT, cmd, strlen(cmd));
        if(ret < 0)
        {
            TRACE_ERR("ret:%d", ret);
            ret = DDI_ETRANSPORT;
            break;
        }

        if(end_flag)
        {
            ret = dev_com_write(WIFI_PORT, AT_CMD_RES_END, strlen(AT_CMD_RES_END));
            if(ret < 0)
            {
                TRACE_ERR("ret:%d", ret);
                ret = DDI_ETRANSPORT;
                break;
            }
        }
    }while(0);

    if(AT_API_BLOCK == at_api_get_type)
    {
        dev_wifi_thread_run_switch(TRUE);
    }
    return ret;
}

iomux_pin_name_t wifi_get_en_pin(void)
{
    if(dev_misc_get_hwver() > MACHINE_HWVER_M3V1)
    {
        TRACE_DBG("over hard 0, use PTA3");
        return GPIO_PIN_PTA3;
    }
    else
    {
        TRACE_DBG("hard 0, use PTA0");
        return GPIO_PIN_PTA0;
    }
}

iomux_pin_name_t wifi_get_wakeup_pin(void)
{
    if(dev_misc_get_hwver() > MACHINE_HWVER_M3V1)
    {
        return GPIO_PIN_PTB0;
    }
    else
    {
        return GPIO_PIN_PTA1;
    }
}

static void wifi_detect_exist(void)
{
    dev_gpio_config_mux(WIFI_EN_PIN, MUX_CONFIG_GPIO);
    dev_gpio_set_pad(WIFI_EN_PIN, PAD_CTL_PULL_NONE);
    dev_gpio_direction_input(WIFI_EN_PIN); 

    if(dev_gpio_get_value(WIFI_EN_PIN) > 0)
    {
        g_wifi_exist = TRUE;
    }
    else
    {
        g_wifi_exist = FALSE;
    }
}

s32 dev_wifi_init(void)
{
    s32 i = 0;

    TRACE_DBG("enter");
    if(!g_wifi_init_flag)
    {
        wifi_detect_exist();

        if(g_wifi_exist)
        {
            TRACE_DBG("enter1");
            drv_wifi_io_init();

#ifdef WIFI_FIXED_FREQ_TEST
            return 0;
#endif
            memset(&g_wifi_device_info, 0, sizeof(g_wifi_device_info));

            //创建wifi串口接收的环形buff
#if(WIFI_RUN_OPEN_CPU == TRUE)
            dev_circlequeue_init(&(g_wifi_device_info.m_wifi_uart_rx_fifo), WIFI_RUN_OPEN_CPU_RX_BUFF);
#endif

            dev_com_open(WIFI_PORT, 115200, 8, 'n', 0, 0);
            //wifi_get_ap_from_file();
            dev_wifi_set_control_strategy_step(WIFI_MODULE_POWER_ON);
            g_wifi_device_info.m_power_flag = 1;
            g_wifi_device_info.m_connect_ap_res = WIFI_STATUS_CONNECT_AP_DISCONNECTED;
            dev_wifi_thread_node_param_init();
            dev_wifi_init_parse_data();
            dev_wifi_thread_run_switch(TRUE);
            g_wifi_init_flag = TRUE;
        }
    }
    
    return DDI_OK;
}

static s32 wifi_open(AT_API_GET_TYPE_E at_api_get_type)
{
    WIFI_CONTROL_FAIL_REASON_E control_fail_reason = dev_wifi_get_status(WIFI_GET_REG_NETWORK_FAIL_REASON);
    s32 ret = DDI_ERR;
    u32 cur_ticks = 0;

    do{
        if(!g_wifi_exist)
        {
            TRACE_ERR("wifi don't exist");
            ret = DDI_ENODEV;
            break;
        }

        if(WF_NO_WIFI_MODULE == control_fail_reason)
        {
            ret = DDI_ENODEV;
            break;
        }
        else
        {
            dev_wifi_thread_run_switch(TRUE);
            dev_wifi_strategy_run_switch(TRUE);
            if(0 == g_wifi_device_info.m_power_flag)
            {
                g_wifi_device_info.m_power_flag = 1;
                dev_wifi_set_control_strategy_step(WIFI_MODULE_POWER_ON);
            }
            ret = DDI_OK;
        }

        if(AT_API_BLOCK == at_api_get_type)
        {
            cur_ticks = dev_user_gettimeID();
            while(1)
            {
                ddi_watchdog_feed();
                
                if(WIFI_MODULES_OPEN_SUCCESS == dev_wifi_get_status(WIFI_GET_REG_NETWORK_STATE))
                {
                    TRACE_INFO("open wifi success");
                    ret = DDI_OK;
                    break;
                }

                if(dev_user_querrytimer(cur_ticks, WIFI_AT_RECV_DEFAULT_WAIT_TIMEOUT*5))
                {
                    TRACE_ERR("open wifi failed");
                    ret = DDI_ETIMEOUT;
                    break;
                }

                dev_user_delay_ms(100);
            }
        }
    }while(0);
    
    return ret;
}

s32 dev_wifi_open(void)
{
    return wifi_open(AT_API_BLOCK);
}

/**
 * @brief 所有wifi套接字断开时的处理(物理网络没掉网)
 * @param [in/out] 
 * @param [in/out] 
 * @retval 
 * @retval 
 * @since 
 */
static void wifi_all_tcp_disconnect_hanle(void)
{
    s32 sn = 0;

    //释放本地的所有socket资源
    for(sn=0; sn<WIFI_SOCKET_MAX; sn++)
    {
        if(g_wifi_device_info.m_socket_info[sn])
        {
            wifi_socket_info_destroy(g_wifi_device_info.m_socket_info[sn]);
            g_wifi_device_info.m_socket_info[sn] = NULL;
        }
    }
}

s32 dev_wifi_close(void)
{
    
    s32 ret = DDI_ERR;

    do{
        TRACE_DBG("enter");

        if(!g_wifi_exist)
        {
            TRACE_ERR("wifi don't exist");
            ret = DDI_ENODEV;
            break;
        }

        wifi_all_tcp_disconnect_hanle();
        dev_gpio_direction_output(WIFI_EN_PIN, 0);
        dev_wifi_thread_set_result(WIFI_AT_RESULT_OTHER);
        dev_wifi_thread_run_switch(FALSE);
        g_wifi_device_info.m_power_flag = 0;
        g_wifi_device_info.m_connect_ap_res = WIFI_STATUS_CONNECT_AP_DISCONNECTED;
        ret = DDI_OK;
    }while(0);

    return ret;
}

static void get_connect_ap_cmd(s8 *cmd_buff, s32 cmd_buff_len)
{
    wifi_device_info_t *device_info = dev_wifi_get_device();

    if(strlen(device_info->m_bssid) > 0)
    {
        snprintf(cmd_buff, cmd_buff_len, "AT+CWJAP_CUR=\"%s\",\"%s\",\"%s\"", 
                                                device_info->m_ssid, 
                                                device_info->m_password, 
                                                device_info->m_bssid);
    }
    else
    {
        snprintf(cmd_buff, cmd_buff_len, "AT+CWJAP_CUR=\"%s\",\"%s\"", 
                                                device_info->m_ssid, 
                                                device_info->m_password);
    }
}

static s32 wifi_connect_ap_process_callback(u8 result, u8 *src, u16 srclen)
{
    TRACE_INFO("result:%d, src:%s", result, src);
    if(WIFI_AT_RESULT_OK == result || WIFI_AT_RESULT_ERROR == result || strstr(src, "FAIL") || WIFI_AT_RESULT_TIMEROUT == result)
    {
        dev_wifi_thread_set_result(result);
    }

    return 0;
}

s32 dev_wifi_connetap(u8 *ssid, u8 *psw, u8 *bssid)
{
    wifi_device_info_t *device_info = dev_wifi_get_device();
    wifi_thread_node_param_t *thread_node_param = (wifi_thread_node_param_t *)dev_wifi_get_thread_node_param();
    s32 ret = DDI_ERR;
    s8 tmp_buff[256] = {0};

    do{
        if(!dev_wifi_thread_is_free())
        {
            TRACE_ERR("now is hanle task:%d, step:%d", thread_node_param->m_task_id, thread_node_param->m_step);
            ret = DDI_EBUSY;
            break;
        }

        TRACE_DBG("enter");
        //应用从低功耗唤醒后，又会重新去连wifi热点，所以这里直接判断，避免重新连接
        if(WIFI_STATUS_CONNECT_AP_CONNECTED == device_info->m_connect_ap_res
            && 0 == strncmp(ssid, device_info->m_ssid, strlen(ssid))
            && 0 == strncmp(psw, device_info->m_password, strlen(psw)))
        {
            ret = DDI_OK;
            break;
        }

        TRACE_DBG("ready to connect ap:%d", device_info->m_connect_ap_res);
        device_info->m_connect_ap_res = WIFI_STATUS_CONNECT_AP_CONNECTING;
        snprintf(device_info->m_ssid, sizeof(device_info->m_ssid), "%s", ssid);
        snprintf(device_info->m_password, sizeof(device_info->m_password), "%s", psw);
        if(bssid && strlen(bssid))
        {
            snprintf(device_info->m_bssid, sizeof(device_info->m_bssid), "%s", bssid);
        }

        if(WIFI_MODULES_OPEN_SUCCESS != dev_wifi_get_status(WIFI_GET_REG_NETWORK_STATE))
        {
            device_info->m_need_connect_ap = TRUE;
            break;
        }
        else if(WIFI_MODULES_OPEN_SUCCESS == dev_wifi_get_status(WIFI_GET_REG_NETWORK_STATE))
        {
            device_info->m_need_connect_ap = FALSE;
        }
        
        memset(tmp_buff, 0, sizeof(tmp_buff));
        get_connect_ap_cmd(tmp_buff, sizeof(tmp_buff));
        dev_wifi_send_at_cmd(tmp_buff, AT_API_NO_BLOCK, TRUE);
        dev_wifi_thread_set_param(WIFI_TASK_AP_CONNETING, wifi_connect_ap_process_callback, WIFI_AT_DEFAULT_TIMEOUT*15);
        ret = DDI_OK;
    }while(0);
    
    return ret;
}

inline s32 dev_wifi_get_connectap_status(void)
{
    wifi_device_info_t *device_info = dev_wifi_get_device();
    wifi_thread_node_param_t *thread_node_param = (wifi_thread_node_param_t *)dev_wifi_get_thread_node_param();

    if(TRUE == device_info->m_need_connect_ap)
    {
        if(WIFI_MODULES_OPEN_SUCCESS != dev_wifi_get_status(WIFI_GET_REG_NETWORK_STATE))
        {
            return WIFI_STATUS_CONNECT_AP_CONNECTING;
        }
        else if(WIFI_MODULES_OPEN_SUCCESS == dev_wifi_get_status(WIFI_GET_REG_NETWORK_STATE))
        {
            TRACE_DBG("connect ap");
            dev_wifi_connetap(device_info->m_ssid, device_info->m_password, device_info->m_bssid);
        }
    }
    
    //wifi模块目前用的是nonos-at，指令最后都会返回ok或error，异步处理report-handle里判断gotip将状态改为已连接还不能直接返回给应用，必须等ok返回后，才能返回连接成功
    if(WIFI_TASK_AP_CONNETING == thread_node_param->m_task_id)
    {
        return WIFI_STATUS_CONNECT_AP_CONNECTING;
    }
    
    //连接热点成功时查询下网络状态，防止低功耗唤醒，wifi热点或套接字改变了
    if(WIFI_STATUS_CONNECT_AP_CONNECTED == device_info->m_connect_ap_res
        && TRUE == device_info->m_idle_flag)
    {
        device_info->m_idle_flag = FALSE;
        wifi_get_net_connect_state();
    }
    else
    {
        device_info->m_idle_flag = FALSE;
    }

    return device_info->m_connect_ap_res;
}

void dev_wifi_ctl_downloadmod(void)
{
}

static s32 wifi_get_signal_callback(u8 result, u8 *src, u16 srclen)
{
    s8 dst[4][SPLIT_PER_MAX];
    s8 *pstr = NULL;
    s32 ret = DDI_ERR;
    s32 i = 0;
    
    TRACE_DBG("result:%d, src:%s", result, src);
    if(WIFI_AT_RESULT_OTHER == result)
    {
        pstr = strstr(src, "+CWJAP:");
        if(pstr)
        {
            pstr += strlen("+CWJAP:");
            memset(dst, 0, sizeof(dst));
            ret = dev_maths_split(dst, sizeof(dst)/sizeof(dst[0]), pstr, ",", SPLIT_NOIGNOR);
            if(sizeof(dst)/sizeof(dst[0]) == ret)
            {
                for(i=0; i<sizeof(dst)/sizeof(dst[0]); i++)
                {
                    dev_maths_str_trim(dst[i]);
                }

                g_wifi_device_info.m_rssi = dev_maths_atoi(dst[3]);
                TRACE_DBG("rssi:%d", g_wifi_device_info.m_rssi);
            }
        }
    }
    else if(WIFI_AT_RESULT_OK == result | WIFI_AT_RESULT_ERROR == result || WIFI_AT_RESULT_TIMEROUT == result)
    {
        dev_wifi_thread_set_result(result);

        if(WIFI_AT_RESULT_TIMEROUT == result)
        {
            dev_wifi_set_control_strategy_step(WIFI_CHECK_CMD);
        }
    }

    return result;
}

static s32 wifi_get_signal(void)
{
    s8 buff[128] = {0};
    s32 ret = DDI_ERR;
    wifi_thread_node_param_t *thread_node_param = (wifi_thread_node_param_t *)dev_wifi_get_thread_node_param(); 

    dev_wifi_send_at_cmd("AT+CWJAP?", AT_API_BLOCK, TRUE);
    dev_wifi_thread_set_param(WIFI_TASK_GET_CUR_AP_INFO, wifi_get_signal_callback, WIFI_AT_CMD_RESEND_TIMEOUT);
    while(1)
    {
        dev_wifi_thread_run_switch(FALSE);
        if(dev_wifi_thread_task_is_change(WIFI_TASK_GET_CUR_AP_INFO))
        {
            ret = DDI_EBUSY;
            break;
        }
        
        if(dev_wifi_thread_is_free())
        {
            switch(thread_node_param->m_result)
            {
                case WIFI_AT_RESULT_OK:
                    ret = DDI_OK;
                    break;

                case WIFI_AT_RESULT_ERROR:
                    ret = DDI_ERR;
                    break;

                case WIFI_AT_RESULT_TIMEROUT:
                    ret = DDI_ETIMEOUT;
                    break;
            }

            break;
        }
        
        dev_wifi_thread_run_switch(TRUE);

        ddi_misc_msleep(50);
    }

    dev_wifi_thread_run_switch(TRUE);

    return ret;
}


s32 dev_wifi_get_signal(s32 *rssi)
{
    s32 ret = DDI_ERR;
    wifi_thread_node_param_t *thread_node_param = (wifi_thread_node_param_t *)dev_wifi_get_thread_node_param();
    s32 step = 0;

    do{
        if(NULL == rssi)
        {
            ret = DDI_EINVAL;
            break;
        }

        step = dev_wifi_get_status(WIFI_GET_REG_NETWORK_STATE);
        if(step == WIFI_MODULE_POWER_ON || step == WIFI_MODULE_POWER_OFF)   //add by xiaohonghui  上电或下电流程不发命令
        {
            
        }
        else
        {
            if(dev_wifi_thread_is_free())
            {
                if(dev_user_querrytimer(g_get_csq_last_ticks, WIFI_GET_CSQ_INTERVAL))
                {
                    ret = wifi_get_signal();
                    g_get_csq_last_ticks = dev_user_gettimeID();
                }
            }
            else
            {
                TRACE_DBG("task:%d, step:%d", thread_node_param->m_task_id, thread_node_param->m_step);
            }
        }

        *rssi = g_wifi_device_info.m_rssi;
        ret = DDI_OK;
    }while(0);
    
    return ret;
}

static s32 wifi_disconnetap_callback(u8 result, u8 *src, u16 srclen)
{
    TRACE_INFO("result:%d, src:%s", result, src);
    dev_wifi_thread_set_result(result);

    return 0;
}

s32 dev_wifi_disconnetap(void)
{
    wifi_device_info_t *device_info = dev_wifi_get_device();
    wifi_thread_node_param_t *thread_node_param = (wifi_thread_node_param_t *)dev_wifi_get_thread_node_param();
    s32 ret = DDI_ERR;
    s8 tmp_buff[256] = {0};

    do{
        TRACE_DBG("enter");
        
        if(!dev_wifi_thread_is_free())
        {
            TRACE_ERR("now is hanle task:%d, step:%d", thread_node_param->m_task_id, thread_node_param->m_step);
            ret = DDI_EBUSY;
            break;
        }

        dev_wifi_send_at_cmd(WIFI_AT_DISCONNECT_AP, AT_API_NO_BLOCK, TRUE);
        dev_wifi_thread_set_param(WIFI_TASK_AP_DISCONNECT, wifi_disconnetap_callback, WIFI_AT_DEFAULT_TIMEOUT);
        while(1)
        {
            ddi_watchdog_feed();
            dev_wifi_thread_run_switch(FALSE);
            
            if(dev_wifi_thread_task_is_change(WIFI_TASK_AP_DISCONNECT))
            {
                ret = DDI_EBUSY;
                break;
            }
            
            if(dev_wifi_thread_is_free())
            {
                switch(thread_node_param->m_result)
                {
                    case WIFI_AT_RESULT_OK:
                        ret = DDI_OK;
                        break;

                    case WIFI_AT_RESULT_ERROR:
                        ret = DDI_ERR;
                        break;

                    case WIFI_AT_RESULT_TIMEROUT:
                        ret = DDI_ETIMEOUT;
                        break;
                }

                break;
            }

            dev_wifi_thread_run_switch(TRUE);
            ddi_misc_msleep(50);
        }

        dev_wifi_thread_run_switch(TRUE);
    }while(0);

    device_info->m_connect_ap_res = WIFI_STATUS_CONNECT_AP_DISCONNECTED;
    return ret;
}

/**
 * @brief wifi模块下电，关机时调用
 * @retval  无
 */
void dev_wifi_poweroff(void)
{
    s32 sn = 0;
    
    TRACE_DBG("enter");

    if(WF_NO_WIFI_MODULE == dev_wifi_get_status(WIFI_GET_REG_NETWORK_FAIL_REASON))
    {
        return ;
    }
    
    //释放本地的所有socket资源
    for(sn=0; sn<WIFI_SOCKET_MAX; sn++)
    {
        if(g_wifi_device_info.m_socket_info[sn])
        {
            wifi_socket_info_destroy(g_wifi_device_info.m_socket_info[sn]);
            g_wifi_device_info.m_socket_info[sn] = NULL;
        }
    }
    
    dev_gpio_direction_output(WIFI_EN_PIN, 0);
}

/**
 * @brief 获取网络状态或者注册网络失败的原因
 * @param[in] wifi_status_type 0获取网络状态  1获取注册网络失败的原因 
 * @retval  注册失败原因，详见WIFI_REG_NET_FAIL_REASON_E
 * @retval  网络状态，详见WIFI_STATUS_E
 */
inline s32 dev_wifi_get_status(wifi_status_type_e wifi_status_type)
{
    s32 ret = DDI_ERR;

    switch(wifi_status_type)
    {
        case WIFI_GET_REG_NETWORK_STATE:
            ret = g_wifi_device_info.m_control_step;
            break;

        case WIFI_GET_REG_NETWORK_FAIL_REASON:
            ret = g_wifi_device_info.m_control_fail_reason;
            break;
    }

    return ret;
}

static s32 wifi_get_netinfo_callback(u8 result, u8 *src, u16 srclen)
{
    s8 dst[3][SPLIT_PER_MAX];
    s8 *pstr = NULL;
    s32 ret = DDI_ERR;
    s32 i = 0;
    
    TRACE_DBG("result:%d, src:%s", result, src);
    if(WIFI_AT_RESULT_OTHER == result)
    {
        pstr = strstr(src, "+CIFSR:STAIP");
        if(pstr)
        {
            pstr += strlen("+CIFSR:STAIP");
            memset(dst, 0, sizeof(dst));
            ret = dev_maths_split(dst, sizeof(dst)/sizeof(dst[0]), src, "\"", SPLIT_NOIGNOR);
            if(sizeof(dst)/sizeof(dst[0]) == ret)
            {
                dev_maths_str_trim(dst[1]);
                snprintf(g_wifi_net_info.m_localip, sizeof(g_wifi_net_info.m_localip), "%s", dst[1]);
            }
        }

        pstr = strstr(src, "+CIFSR:STAMAC");
        if(pstr)
        {
            pstr += strlen("+CIFSR:STAMAC");
            memset(dst, 0, sizeof(dst));
            ret = dev_maths_split(dst, sizeof(dst)/sizeof(dst[0]), src, "\"", SPLIT_NOIGNOR);
            if(sizeof(dst)/sizeof(dst[0]) == ret)
            {
                dev_maths_str_trim(dst[1]);
                snprintf(g_wifi_net_info.m_macid, sizeof(g_wifi_net_info.m_macid), "%s", dst[1]);
            }
        }

        pstr = strstr(src, "+CIFSR:APMAC");
        if(pstr)
        {
            pstr += strlen("+CIFSR:APMAC");
            memset(dst, 0, sizeof(dst));
            ret = dev_maths_split(dst, sizeof(dst)/sizeof(dst[0]), src, "\"", SPLIT_NOIGNOR);
            if(sizeof(dst)/sizeof(dst[0]) == ret)
            {
                dev_maths_str_trim(dst[1]);
                snprintf(g_wifi_net_info.m_macid, sizeof(g_wifi_net_info.m_macid), "%s", dst[1]);
            }
        }
    }
    else if(WIFI_AT_RESULT_OK == result | WIFI_AT_RESULT_ERROR == result || WIFI_AT_RESULT_TIMEROUT == result)
    {
        dev_wifi_thread_set_result(result);

        if(WIFI_AT_RESULT_TIMEROUT == result)
        {
            dev_wifi_set_control_strategy_step(WIFI_CHECK_CMD);
        }
    }

    return result;
}

/**
 * @brief 获取本地网络信息
 * @param[in] wifi_net_info 用来填充信息的缓存
 * @retval  DDI_OK   获取成功
 * @retval  DDI_EBUSY  设备繁忙
 * @retval  DDI_ERR    获取失败
 */
s32 dev_wifi_get_netinfo(wifi_net_info_t *wifi_net_info)
{
    s8 buff[128] = {0};
    s32 ret = DDI_ERR;
    s32 socket_sn  = 0;
    wifi_thread_node_param_t *thread_node_param = (wifi_thread_node_param_t *)dev_wifi_get_thread_node_param();

    if(NULL == wifi_net_info)
    {
        return DDI_EINVAL;
    }

    if(WF_NO_WIFI_MODULE == dev_wifi_get_status(WIFI_GET_REG_NETWORK_FAIL_REASON))
    {
        return DDI_ENODEV;
    }
    
    memset(&g_wifi_net_info, 0, sizeof(g_wifi_net_info));
    dev_wifi_send_at_cmd("AT+CIFSR", AT_API_BLOCK, TRUE);
    dev_wifi_thread_set_param(WIFI_TASK_GET_NETINFO, wifi_get_netinfo_callback, WIFI_AT_CMD_RESEND_TIMEOUT*2);
    while(1)
    {
        if(dev_wifi_thread_task_is_change(WIFI_TASK_GET_NETINFO))
        {
            ret = DDI_EBUSY;
            break;
        }
        
        if(dev_wifi_thread_is_free())
        {
            switch(thread_node_param->m_result)
            {
                case WIFI_AT_RESULT_OK:
                    memcpy(wifi_net_info, &g_wifi_net_info, sizeof(g_wifi_net_info));
                    ret = DDI_OK;
                    break;

                case WIFI_AT_RESULT_ERROR:
                    ret = DDI_ERR;
                    break;

                case WIFI_AT_RESULT_TIMEROUT:
                    ret = DDI_ETIMEOUT;
                    break;
            }

            break;
        }

        ddi_misc_msleep(50);
    }

    return ret;
}

static s32 wifi_socket_create_callback(u8 result, u8 *src, u16 srclen)
{
    wifi_thread_node_param_t *thread_node_param = (wifi_thread_node_param_t *)dev_wifi_get_thread_node_param();
    s32 tmp = 0;
    s32 socket_id = 0;
    s32 socket_sn  = 0;
    s32 fail_flag = FALSE;

    socket_id = thread_node_param->m_lparam;
    socket_sn = dev_wifi_socket_info_sn_by_id(socket_id);
    if(socket_sn < 0 || socket_sn >= WIFI_SOCKET_MAX)
    {
        TRACE_ERR("socket id don't exist:%d", socket_id);
        return DDI_ERR;
    }
    
    TRACE_DBG("result:%d, src:%s", result, src);
    if(WIFI_AT_RESULT_OK == result)
    {
        tmp = thread_node_param->m_lparam;
        dev_wifi_thread_set_result(result);
        thread_node_param->m_lparam = tmp;
    }
    else if(WIFI_AT_RESULT_OTHER == result)
    {
        if(strstr(src, "busy p..."))
        {
            thread_node_param->m_lparam++;
            if(thread_node_param->m_lparam >= 2)
            {
                TRACE_INFO("busy reach %d times", thread_node_param->m_lparam);
                fail_flag = TRUE;
            }
        }
        else if(strstr(src, "DNS Fail"))
        {
            TRACE_INFO("dns fail for hostname");
            fail_flag = TRUE;
            dev_wifi_thread_set_result(WIFI_AT_RESULT_DNSFAIL);
        }
        else if(strstr(src, "ALREADY CONNECTED"))   //我们这边没socket信息，但模块上有
        {
            if(g_wifi_device_info.m_socket_info[socket_sn])
            {
                if(!g_wifi_device_info.m_use_tcp_server && g_wifi_device_info.m_notify_func)
                {
                    g_wifi_device_info.m_notify_func(WN_EVENT_TCP_CONNECTED, socket_sn|WIFI_ID_MASK, 0);
                }
                g_wifi_device_info.m_socket_info[socket_sn]->m_status = SOCKET_STATUS_CONNECTED;
                TRACE_INFO("sn:%d connect success", socket_sn);
            }
            dev_wifi_thread_set_result(WIFI_AT_RESULT_SOCKET_ALREAYDY_EXIT);
        }
        else if(strstr(src, "no ip"))
        {
            TRACE_INFO("network drop");
            fail_flag = TRUE;
            wifi_all_tcp_disconnect_hanle();
            dev_wifi_thread_set_result(WIFI_AT_RESULT_ERROR);
            g_wifi_device_info.m_connect_ap_res = WIFI_STATUS_CONNECT_AP_DISCONNECTED;
        }
    }
    else if(WIFI_AT_RESULT_ERROR == result || WIFI_AT_RESULT_TIMEROUT == result)
    {
        TRACE_DBG("connect error or timeout");
        dev_wifi_thread_set_result(result);
        fail_flag = TRUE;

        if(WIFI_AT_RESULT_TIMEROUT == result)
        {
            dev_wifi_set_control_strategy_step(WIFI_CHECK_CMD);
        }
    }

    if(fail_flag)
    {
        if(g_wifi_device_info.m_socket_info[socket_sn])
        {
            TRACE_INFO("connect fail");
            if(!g_wifi_device_info.m_use_tcp_server && g_wifi_device_info.m_notify_func)
            {
                g_wifi_device_info.m_notify_func(WN_EVENT_TCP_DISCONNECT, socket_sn|WIFI_ID_MASK, 0);
            }
            g_wifi_device_info.m_socket_info[socket_sn]->m_status = SOCKET_STATUS_DISCONNECTED;
        }
    }
    return result;
}

static s32 wifi_socket_create(s32 socket_id, s8 *host_info, s32 port, AT_API_GET_TYPE_E at_api_get_type)
{
    s8 buff[128] = {0};
    s32 ret = DDI_ERR;
    wifi_thread_node_param_t *thread_node_param = (wifi_thread_node_param_t *)dev_wifi_get_thread_node_param();
    
    memset(buff, 0, sizeof(buff));
    snprintf(buff, sizeof(buff), "AT+CIPSTART=%d,\"TCP\",\"%s\",%d", socket_id, host_info, port);
    dev_wifi_send_at_cmd(buff, at_api_get_type, TRUE);
    dev_wifi_thread_set_param(WIFI_TASK_CREATE_SOCKET, wifi_socket_create_callback, WIFI_AT_CMD_RESEND_TIMEOUT*10);
    thread_node_param->m_lparam = socket_id;
    ret = DDI_OK;
    return ret;
}

s32 dev_wifi_socket_create(u8 type, u8 mode, u8 *param, u16 port)
{
    s32 ret = DDI_ERR;
    s32 sn = 0;
    s8 buff[128] = {0};
    wifi_socket_info_t *socket_info = NULL;
    wifi_thread_node_param_t *thread_node_param = (wifi_thread_node_param_t *)dev_wifi_get_thread_node_param();
    
    do{
        if(1 == type || NULL == param || 0 == port)
        {
            ret = DDI_EINVAL;
            break;
        }

        if(WF_NO_WIFI_MODULE == dev_wifi_get_status(WIFI_GET_REG_NETWORK_FAIL_REASON))
        {
            ret = DDI_ENODEV;
            break;
        }
        
        if(!dev_wifi_thread_is_free())
        {
            TRACE_ERR("now is hanle task:%d, step:%d", thread_node_param->m_task_id, thread_node_param->m_step);
            ret = DDI_EBUSY;
            break;
        }

        if(g_wifi_device_info.m_use_tcp_server)
        {
            TRACE_ERR("Now is web config, don't support tcp client now");
            ret = DDI_EBUSY;
            break;
        }
        
        sn = wifi_get_available_socket_info_index(param, port);
        if(sn < 0)
        {
            TRACE_ERR("dev_gprs_socket_request failed:%d", ret);
            ret = DDI_EBUSY;
            break;
        }

        socket_info = g_wifi_device_info.m_socket_info[sn];
        snprintf(socket_info->m_serveraddr, sizeof(socket_info->m_serveraddr), "%s", param);
        socket_info->m_serverport = port;

        socket_info->m_status = SOCKET_STATUS_CONNECTING;
        ret = wifi_socket_create(socket_info->m_socket_id, param, port, AT_API_BLOCK);
        if(DDI_OK == ret)
        {
            ret = sn|WIFI_ID_MASK;
        }

        TRACE_DBG("socket_info->m_id:%d, socket_info->m_status:%d, ret:%x", socket_info->m_socket_id, socket_info->m_status, ret);
        if(ret < 0)
        {
            wifi_socket_info_destroy(socket_info);
            g_wifi_device_info.m_socket_info[sn] = NULL;
        }
    }while(0);

    return ret;
}


s32 dev_wifi_socket_get_status(s32 socketid)
{
    s32 ret;
    u32 sn;
    wifi_socket_info_t* socket_info;

    if(g_wifi_device_info.m_use_tcp_server)
    {
        TRACE_ERR("Now is web config, don't support tcp client now");
        return DDI_EBUSY;
    }
    
    sn = socketid&(~WIFI_ID_MASK); 
    socket_info = wifi_get_socket_info_by_index(sn);
    if(socket_info == NULL)
    {
        TRACE_ERR("cannot find socket info for socketid(%d,%d) Err", socketid, sn);
        return DDI_EINVAL;
    }

    ret = socket_info->m_status;
    return ret;
}

static s32 wifi_send_data_callback(u8 result, u8 *src, u16 srclen)
{
    TRACE_DBG("result:%d, src:%s", result, src);
    if(WIFI_AT_RESULT_OTHER == result)
    {
        if(0 == strcmp(src, "SEND OK"))
        {
            dev_wifi_thread_set_result(WIFI_AT_RESULT_OK);
            dev_wifi_thread_run_switch(FALSE);         //避免send ok和wifi_send_data判断send 结果中间立即收到了后台数据，导致最终判发送结果就失败了
        }
        else if(0 == strcmp(src, "SEND FAIL"))
        {
            TRACE_ERR("send this pack data fail");
            dev_wifi_thread_set_result(WIFI_AT_RESULT_ERROR);
        }
    }
    else if(WIFI_AT_RESULT_ERROR == result || WIFI_AT_RESULT_TIMEROUT == result)
    {
        dev_wifi_thread_set_result(result);

        if(WIFI_AT_RESULT_TIMEROUT == result)
        {
            dev_wifi_set_control_strategy_step(WIFI_CHECK_CMD);
        }
    }

    return result;
}

static s32 wifi_send_data(s32 socket_id, u8 *wbuf, s32 wlen)
{
    s32 ret = DDI_ERR;
    wifi_thread_node_param_t *thread_node_param = (wifi_thread_node_param_t *)dev_wifi_get_thread_node_param();
    
    dev_com_write(WIFI_PORT, wbuf, wlen);
    dev_wifi_thread_set_param(WIFI_TASK_SOCKET_SENDBUFF, wifi_send_data_callback, WIFI_AT_CMD_RESEND_TIMEOUT*10);
    while(1)
    {
        ddi_watchdog_feed();
        dev_wifi_thread_run_switch(FALSE);
        if(dev_wifi_thread_task_is_change(WIFI_TASK_SOCKET_SENDBUFF))
        {
            ret = DDI_EBUSY;
            break;
        }
        
        if(dev_wifi_thread_is_free())
        {
            switch(thread_node_param->m_result)
            {
                case WIFI_AT_RESULT_OK:
                    TRACE_INFO("send this pack data ok");
                    dev_wifi_thread_run_switch(TRUE);
                    ret = wlen;
                    break;

                case WIFI_AT_RESULT_ERROR:
                    ret = DDI_ERR;
                    break;

                case WIFI_AT_RESULT_TIMEROUT:
                    ret = DDI_ETIMEOUT;
                    break;
            }

            break;
        }

        dev_wifi_thread_run_switch(TRUE);
        ddi_misc_msleep(50);
    }

    dev_wifi_thread_run_switch(TRUE);
    TRACE_DBG("out");
    return ret;
}

static s32 wifi_send_datalen_callback(u8 result, u8 *src, u16 srclen)
{
    TRACE_DBG("result:%d, src:%s", result, src);
    if(WIFI_AT_RESULT_OTHER == result)
    {
        if('>' == src[0])
        {
            dev_wifi_thread_set_result(WIFI_AT_RESULT_OK);
        }
    }
    else if(WIFI_AT_RESULT_OK == result)
    {
        TRACE_DBG("ready to recv >");
    }
    else if(WIFI_AT_RESULT_ERROR == result || WIFI_AT_RESULT_TIMEROUT == result)
    {
        dev_wifi_thread_set_result(result);

        if(WIFI_AT_RESULT_TIMEROUT == result)
        {
            dev_wifi_set_control_strategy_step(WIFI_CHECK_CMD);
        }
    }

    return result;
}

static s32 wifi_send_datalen(s32 socket_id, s32 wlen)
{
    s8 buff[128] = {0};
    s32 ret = DDI_ERR;
    wifi_thread_node_param_t *thread_node_param = (wifi_thread_node_param_t *)dev_wifi_get_thread_node_param();
    
    memset(buff, 0, sizeof(buff));
    snprintf(buff, sizeof(buff), "AT+CIPSEND=%d,%d", socket_id, wlen);
    dev_wifi_send_at_cmd(buff, AT_API_BLOCK, TRUE);
    dev_wifi_thread_set_param(WIFI_TASK_SOCKET_SEND_DATALEN, wifi_send_datalen_callback, WIFI_AT_CMD_RESEND_TIMEOUT*5);
    while(1)
    {
        ddi_watchdog_feed();
        dev_wifi_thread_run_switch(FALSE);
        if(dev_wifi_thread_task_is_change(WIFI_TASK_SOCKET_SEND_DATALEN))
        {
            ret = DDI_EBUSY;
            break;
        }
        
        if(dev_wifi_thread_is_free())
        {
            switch(thread_node_param->m_result)
            {
                case WIFI_AT_RESULT_OK:
                    ret = DDI_OK;
                    break;

                case WIFI_AT_RESULT_ERROR:
                    ret = DDI_ERR;
                    break;

                case WIFI_AT_RESULT_TIMEROUT:
                    ret = DDI_ETIMEOUT;
                    break;
            }

            break;
        }
        
        dev_wifi_thread_run_switch(TRUE);

        ddi_misc_msleep(50);
    }

    dev_wifi_thread_run_switch(TRUE);

    return ret;
}

static s32 wifi_socket_send_onepacket(s32 socket_id, u8 *wbuf, s32 wlen)
{
    s32 ret;
    u32 timerid;
    u8 sendflg;
    u8 step;
    wifi_thread_node_param_t *thread_node_param = (wifi_thread_node_param_t *)dev_wifi_get_thread_node_param();
    s8 tmp_buff[64];

    do{
        ret = wifi_send_datalen(socket_id, wlen);
        if(DDI_OK != ret)
        {
            TRACE_ERR("send datalen fail:%d", ret);
            break;
        }

        TRACE_DBG("ret:%d", ret);

        ret = wifi_send_data(socket_id, wbuf, wlen);
        if(ret < DDI_OK)
        {
            TRACE_ERR("send this pack data fail:%d", ret);
            break;
        }
    }while(0);

    TRACE_DBG("out");
    return ret;
}

static s32 wifi_socket_send(s32 socket_id, u8 *wbuf, s32 wlen, AT_API_GET_TYPE_E at_api_get_type)
{
    s32 i,j;
    s32 ret = 0;
    wifi_thread_node_param_t *thread_node_param = (wifi_thread_node_param_t *)dev_wifi_get_thread_node_param();

    if(AT_API_BLOCK == at_api_get_type)
    {
        i = 0;
        while(i<wlen)
        {
            if((i+WIFI_SOCKET_SEND_BUFLEN) > wlen)
            {
                j = wlen - i;
            }
            else
            {
                j = WIFI_SOCKET_SEND_BUFLEN;
            }
            
            ret = wifi_socket_send_onepacket(socket_id, &wbuf[i], j);
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

s32 dev_wifi_socket_send(s32 socketid, u8 *wbuf, u32 wlen)
{
    s32 ret;
    u32 sn;
    wifi_socket_info_t *socket_info;
    wifi_thread_node_param_t *thread_node_param = (wifi_thread_node_param_t *)dev_wifi_get_thread_node_param();
    s8 *send_buff = NULL;
    s32 send_len = 0;
    u32 i = 0, j = 0, tmp_len = 0, have_sendlen = 0;

    TRACE_DBG("enter");
    do{
        if(NULL == wbuf || 0 == wlen)
        {
            ret = DDI_EINVAL;
            break;
        }

        if(WF_NO_WIFI_MODULE == dev_wifi_get_status(WIFI_GET_REG_NETWORK_FAIL_REASON))
        {
            ret = DDI_ENODEV;
            break;
        }
        
        sn = socketid&(~WIFI_ID_MASK);
        socket_info = wifi_get_socket_info_by_index(sn);
        if(socket_info == NULL)
        {
            TRACE_ERR("cannot find socket info for socketid(%d) Err", socketid);
            ret = DDI_EINVAL;
            break;
        }

        if(!dev_wifi_thread_is_free() && WIFI_TASK_SOCKET_RECVDATA != thread_node_param->m_task_id)
        {
            TRACE_ERR("now is hanle task:%d, step:%d", thread_node_param->m_task_id, thread_node_param->m_step);
            ret = DDI_EBUSY;
            break;
        }

        if(!g_wifi_device_info.m_use_tcp_server)   //tcp server的时候是配网，不判断物理网络状态
        {
            if(WIFI_STATUS_CONNECT_AP_CONNECTED != dev_wifi_get_device()->m_connect_ap_res)
            {
                TRACE_ERR("network is not active");
                ret = DDI_EOPERATION;
                break;
            }
        }

        if(socket_info->m_status != SOCKET_STATUS_CONNECTED)
        {
            TRACE_ERR("socket has disconnected");
            ret = DDI_ERR;
            break;
        }

        ret = wifi_socket_send(socket_info->m_socket_id, wbuf, wlen, AT_API_BLOCK);
    }while(0);
        
    return ret;
}


s32 dev_wifi_socket_recv(s32 socketid, u8 *rbuf, u32 rlen)
{
    s32 ret = DEVSTATUS_ERR_PARAM_ERR;
    u32 sn;
    s32 recv_ret = 0;
    wifi_socket_info_t *socket_info;

    do{
        sn = socketid&(~WIFI_ID_MASK); 
        socket_info = wifi_get_socket_info_by_index(sn);
        if(socket_info == NULL)
        {
            TRACE_DBG("cannot find socket info for socketid(%d)", socketid);
            ret = DDI_EINVAL;
            break;
        }

        ret = dev_circlequeue_read(&(socket_info->m_read_buffer), rbuf, rlen);
    }while(0);
        
    return ret;
}

static s32 wifi_socket_close_callback(u8 result, u8 *src, u16 srclen)
{
    wifi_thread_node_param_t *thread_node_param = (wifi_thread_node_param_t *)dev_wifi_get_thread_node_param();
    s8 *pstr = NULL;

    TRACE_INFO("result:%d, src:%s", result, src);
    if(WIFI_AT_RESULT_OK == result)
    {
        dev_wifi_thread_set_result(result);
    }
    else if(WIFI_AT_RESULT_TIMEROUT == result || WIFI_AT_RESULT_ERROR == result)
    {
        if(WIFI_AT_RESULT_ERROR == result && 1 == thread_node_param->m_lparam)
        {
            TRACE_INFO("link is not exist, so close success");
            dev_wifi_thread_set_result(WIFI_AT_RESULT_OK);
        }
        else
        {
            TRACE_ERR("close socket failed:%d", result);
            dev_wifi_thread_set_result(result);
        }

        if(WIFI_AT_RESULT_TIMEROUT == result)
        {
            dev_wifi_set_control_strategy_step(WIFI_CHECK_CMD);
        }
    }
    else if(WIFI_AT_RESULT_OTHER == result)
    {
        if(strstr(src, "UNLINK"))
        {
            thread_node_param->m_lparam = 1;
        }
    }

    return result;
}

static s32 wifi_close_socket(s32 module_socket_id)
{
    s8 buff[64] = {0};
    wifi_thread_node_param_t *thread_node_param = (wifi_thread_node_param_t *)dev_wifi_get_thread_node_param();
    s32 ret = DDI_ERR;
    
    memset(buff, 0, sizeof(buff));
    snprintf(buff, sizeof(buff), "AT+CIPCLOSE=%d", module_socket_id);
    dev_wifi_send_at_cmd(buff, AT_API_BLOCK, TRUE);
    dev_wifi_thread_set_param(WIFI_TASK_SOCKET_CLOSE, wifi_socket_close_callback, WIFI_AT_DEFAULT_TIMEOUT);
    while(1)
    {
        ddi_watchdog_feed();
        dev_wifi_thread_run_switch(FALSE);
        
        if(dev_wifi_thread_task_is_change(WIFI_TASK_SOCKET_CLOSE))
        {
            ret = DDI_EBUSY;
            break;
        }
        
        if(dev_wifi_thread_is_free())
        {
            switch(thread_node_param->m_result)
            {
                case WIFI_AT_RESULT_OK:
                    ret = DDI_OK;
                    break;

                case WIFI_AT_RESULT_ERROR:
                    ret = DDI_ERR;
                    break;

                case WIFI_AT_RESULT_TIMEROUT:
                    ret = DDI_ETIMEOUT;
                    break;
            }

            break;
        }

        dev_wifi_thread_run_switch(TRUE);
        ddi_misc_msleep(50);
    }

    dev_wifi_thread_run_switch(TRUE);

    return ret;
}

s32 dev_wifi_socket_close(s32 socketid)
{
    s32 ret = DDI_ERR;
    u32 sn;
    wifi_socket_info_t *socket_info;
    wifi_thread_node_param_t *thread_node_param = (wifi_thread_node_param_t *)dev_wifi_get_thread_node_param();

    TRACE_DBG("enter");
    do{
        if(WF_NO_WIFI_MODULE == dev_wifi_get_status(WIFI_GET_REG_NETWORK_FAIL_REASON))
        {
            ret = DDI_ENODEV;
            break;
        }
        
        sn = socketid&(~WIFI_ID_MASK); 
        socket_info = wifi_get_socket_info_by_index(sn);
        if(socket_info == NULL)
        {
            TRACE_DBG("cannot find socket info for socketid(%d)", socketid);
            ret = DDI_EINVAL;
            break;
        }
        
        if(!dev_wifi_thread_is_free())
        {
            ret = DDI_EBUSY;
            break;
        }

        ret = wifi_close_socket(socket_info->m_socket_id);
        if(DDI_OK == ret)
        {
            TRACE_INFO("destory socketid:%d info", socket_info->m_socket_id);
            dev_wifi_socket_info_destroy_by_id(socket_info->m_socket_id);
        }
    }while(0);
    
    return ret;
}

s32 dev_wifi_resume(void)
{
    s32 ret = DDI_ERR;

    if(WF_NO_WIFI_MODULE == dev_wifi_get_status(WIFI_GET_REG_NETWORK_FAIL_REASON))
    {
        return DDI_ENODEV;
    }
    
    dev_wifi_thread_run_switch(TRUE);

    if(WIFI_STATUS_CONNECT_AP_CONNECTED == g_wifi_device_info.m_connect_ap_res)
    {
        dev_gpio_config_mux(WIFI_WAKE_PIN, MUX_CONFIG_ALT2);  //wakeup和串口TX脚复用，配置成串口模式,默认拉高,唤醒休眠
    }
    else
    {
        wifi_open(AT_API_NO_BLOCK);
        dev_wifi_set_control_strategy(WF_NORMAL_RUN);
    }

    g_wifi_device_info.m_idle_flag = TRUE;
    return DDI_OK;
}

/**
 * @brief 模块低功耗接口
 * @retval  DDI_OK 成功
 * @retval  DDI_ERR 有按键干预
 */
s32 dev_wifi_suspend(void)
{
    s32 ret = DDI_ERR;

    if(WF_NO_WIFI_MODULE == dev_wifi_get_status(WIFI_GET_REG_NETWORK_FAIL_REASON))
    {
        return DDI_ENODEV;
    }
    
    dev_wifi_thread_run_switch(FALSE);

    if(WIFI_STATUS_CONNECT_AP_CONNECTED == g_wifi_device_info.m_connect_ap_res)
    {
        dev_gpio_direction_output(WIFI_WAKE_PIN, 0);
    }
    else
    {
        dev_wifi_close();
    }
    
    return DDI_OK;
}

s32 dev_wifi_start_airkiss(void)
{
    s32 ret = DDI_ERR;
    u32 cur_ticks = 0;

    if(WF_NO_WIFI_MODULE == dev_wifi_get_status(WIFI_GET_REG_NETWORK_FAIL_REASON))
    {
        return DDI_ENODEV;
    }
    
    TRACE_DBG("enter");
    g_wifi_device_info.m_airkiss_step = WIFI_AIRKISS_FREE;
    g_wifi_device_info.m_airkiss_change = FALSE;
    dev_wifi_close();
    dev_wifi_set_control_strategy(WF_AIR_KISS_CONFIG);
    ret = dev_wifi_open();

    if(DDI_OK == ret)
    {
        dev_wifi_send_at_cmd("AT+CWSTARTSMART=2", AT_API_BLOCK, TRUE);
        dev_wifi_strategy_run_switch(FALSE);
    }
    
    return ret;
}

static s32 wifi_stop_airkiss_callback(u8 result, u8 *src, u16 srclen)
{
    if(WIFI_AT_RESULT_OK == result)
    {
        dev_wifi_thread_set_result(result);
    }
    else if(WIFI_AT_RESULT_ERROR == result || WIFI_AT_RESULT_TIMEROUT == result)
    {
        dev_wifi_thread_set_result(result);

        if(WIFI_AT_RESULT_TIMEROUT == result)
        {
            dev_wifi_set_control_strategy_step(WIFI_CHECK_CMD);
        }
    }

    return result;
}

s32 dev_wifi_stop_airkiss(void)
{
    s8 buff[128] = {0};
    s32 ret = DDI_ERR;
    wifi_thread_node_param_t *thread_node_param = (wifi_thread_node_param_t *)dev_wifi_get_thread_node_param();

    TRACE_DBG("enter");

    if(WF_NO_WIFI_MODULE == dev_wifi_get_status(WIFI_GET_REG_NETWORK_FAIL_REASON))
    {
        return DDI_ENODEV;
    }

    g_wifi_device_info.m_airkiss_step = WIFI_AIRKISS_FREE;
    dev_wifi_close();
    dev_wifi_open();
    dev_wifi_set_control_strategy(WF_NORMAL_RUN);
    ret = DDI_OK;
    
    return ret;
}

s32 dev_wifi_get_airkiss_config(    u8 *ssid, u8 *password)
{
    TRACE_DBG("enter:%d", g_wifi_device_info.m_airkiss_step);
    
    if(WIFI_AIRKISS_CONNECT_OK == g_wifi_device_info.m_airkiss_step 
        || WIFI_AIRKISS_GETCONFIG == g_wifi_device_info.m_airkiss_step
        || WIFI_AIRKISS_CONNECTING == g_wifi_device_info.m_airkiss_step)
    {
        sprintf(ssid, "%s", g_wifi_device_info.m_ssid);
        sprintf(password, "%s", g_wifi_device_info.m_password);
    }
    return DDI_OK;
}

s32 dev_wifi_get_airkiss_status(void)
{
    if(g_wifi_device_info.m_airkiss_change)
    {
        g_wifi_device_info.m_airkiss_change = FALSE;
        wifi_save_ap_to_file();
    }
    
    return g_wifi_device_info.m_airkiss_step;
}

static s32 wifi_scanap_start_callback(u8 result, u8 *src, u16 srclen)
{
    s8 *pstr = NULL;
    s8 dst[11][SPLIT_PER_MAX];
    s32 ret = 0, i = 0;
    
    TRACE_DBG("result:%d, src:%s", result, src);
    if(WIFI_AT_RESULT_OTHER == result)
    {
        pstr = strstr(src, "+CWLAP:(");
        if(pstr)
        {
            pstr += strlen("+CWLAP:(");
            memset(dst, 0, sizeof(dst));
            ret = dev_maths_split(dst, sizeof(dst)/sizeof(dst[0]), pstr, ",", SPLIT_NOIGNOR);
            if(ret == sizeof(dst)/sizeof(dst[0]))
            {
                for(i=0; i<ret; i++)
                {
                    dev_maths_str_trim(dst[i]);
                }
            }

            if(g_wifi_device_info.m_scan_ap_no < g_wifi_device_info.m_max_ap_num)
            {
                i = g_wifi_device_info.m_scan_ap_no;
                g_wifi_device_info.m_scaninfo[i].m_ecn = dev_maths_atoi(dst[0]);
                snprintf(g_wifi_device_info.m_scaninfo[i].m_ssid, 
                            sizeof(g_wifi_device_info.m_scaninfo[i].m_ssid), "%s", dst[1]);
                g_wifi_device_info.m_scaninfo[i].m_rssi = dev_maths_atoi(dst[2]);
                snprintf(g_wifi_device_info.m_scaninfo[i].m_mac, 
                            sizeof(g_wifi_device_info.m_scaninfo[i].m_mac), "%s", dst[3]);
                g_wifi_device_info.m_scaninfo[i].m_channel = dev_maths_atoi(dst[4]);
                g_wifi_device_info.m_scaninfo[i].m_freq_offset = dev_maths_atoi(dst[5]);
                g_wifi_device_info.m_scaninfo[i].m_freq_cali = dev_maths_atoi(dst[6]);
                g_wifi_device_info.m_scaninfo[i].m_pairwise_cipher = dev_maths_atoi(dst[7]);
                g_wifi_device_info.m_scaninfo[i].m_group_cipher = dev_maths_atoi(dst[8]);
                g_wifi_device_info.m_scaninfo[i].m_bgn = dev_maths_atoi(dst[9]);
                g_wifi_device_info.m_scaninfo[i].m_wps = dev_maths_atoi(dst[10]);
                g_wifi_device_info.m_scan_ap_no++;
            }
        }
    }
    else if(WIFI_AT_RESULT_ERROR == result || WIFI_AT_RESULT_TIMEROUT == result || WIFI_AT_RESULT_OK == result)
    {
        dev_wifi_thread_set_result(result);
    }

    return result;
}

s32 dev_wifi_scanap_start(wifi_apinfo_t *lp_scaninfo, u32 apmax)
{
    u32 cur_ticks = 0;
    s32 ret = DDI_ERR;
    wifi_thread_node_param_t *thread_node_param = (wifi_thread_node_param_t *)dev_wifi_get_thread_node_param();

    do{
        if(WF_NO_WIFI_MODULE == dev_wifi_get_status(WIFI_GET_REG_NETWORK_FAIL_REASON))
        {
            ret = DDI_ENODEV;
            break;
        }
        
        if(!lp_scaninfo || 0 == apmax)
        {
            ret = DDI_EINVAL;
            break;
        }

        if(!dev_wifi_thread_is_free())
        {
            TRACE_ERR("now is hanle task:%d, step:%d", thread_node_param->m_task_id, thread_node_param->m_step);
            ret = DDI_EBUSY;
            break;
        }

        g_wifi_device_info.m_scaninfo = lp_scaninfo;
        g_wifi_device_info.m_max_ap_num = apmax;
        g_wifi_device_info.m_scan_ap_no = 0;
        dev_wifi_send_at_cmd("AT+CWLAP", AT_API_BLOCK, TRUE);
        dev_wifi_thread_set_param(WIFI_TASK_SCAN_AP, wifi_scanap_start_callback, WIFI_AT_CMD_RESEND_TIMEOUT*5);
        ret = DDI_OK;
    }while(0);
    
    TRACE_DBG("out:%d", ret);
    return ret;
}

/**
 * @brief 获取扫描热点结果
 * @param[in] ap_num:扫描的热点个数
 * @retval  TRUE 扫描完成
 * @retval  FALSE 正在扫描
 * @retval  DDI_ERR
 * @retval  DDI_ETIMEOUT
 */
s32 dev_wifi_scanap_status(u32 *ap_num)
{
    s32 ret = DDI_ERR;
    wifi_thread_node_param_t *thread_node_param = (wifi_thread_node_param_t *)dev_wifi_get_thread_node_param();

    dev_wifi_thread_run_switch(FALSE);
    do{
        if(dev_wifi_thread_is_free())
        {
            if(g_wifi_device_info.m_scan_ap_no > 0)
            {
                *ap_num = g_wifi_device_info.m_scan_ap_no;
                ret = TRUE;
                break;
            }
            else
            {
                ret = DDI_ERR;
                break;
            }
        }
        else
        {
            ret = FALSE;
            break;
        }
    }while(0);

    dev_wifi_thread_run_switch(TRUE);
    return ret;
}

/**
 * @brief 创建tcp server
 * @retval  DDI_OK 成功
 * @retval  DDI_ERR 失败
 */
s32 dev_wifi_creat_tcp_server(s8 *ap_name, u16 tcp_server_port)
{
    s32 ret = DDI_ERR;
    u32 cur_ticks = 0;
    s32 i = 0;

    if(!ap_name || 0 == tcp_server_port)
    {
        TRACE_ERR("param is error");
        return DDI_EINVAL;
    }

    if(WF_NO_WIFI_MODULE == dev_wifi_get_status(WIFI_GET_REG_NETWORK_FAIL_REASON))
    {
        return DDI_ENODEV;
    }
    
    TRACE_DBG("enter:%d", g_wifi_device_info.m_use_tcp_server);
    if(!g_wifi_device_info.m_use_tcp_server)
    {
        for(i=0; i<WIFI_SOCKET_MAX; i++)
        {
            g_client_socket_id[i] = -1;
        }

        g_wifi_device_info.m_ap_name = ap_name;
        g_wifi_device_info.m_tcp_server_port = tcp_server_port;
        dev_wifi_close();
        dev_wifi_set_control_strategy(WF_WEB_CONFIG);
        ret = dev_wifi_open();
        if(DDI_OK == ret)
        {
            g_wifi_device_info.m_use_tcp_server = TRUE;
        }
    }

    return ret;
}

/**
 * @brief 监听是否有新客户端接入，如果有，返回新socketid
 * @retval  socketid
 * @retval  DDI_ERR 失败
 */
s32 dev_wifi_tcp_server_accept(void)
{
    s32 ret = DDI_ERR;
    s32 i = 0;
    
    do{
        if(WF_NO_WIFI_MODULE == dev_wifi_get_status(WIFI_GET_REG_NETWORK_FAIL_REASON))
        {
            ret = DDI_ENODEV;
            break;
        }
        
        if(!g_wifi_device_info.m_use_tcp_server)
        {
            TRACE_ERR("don't creat tcp server");
            ret = DDI_EIO;
            break;
        }

        for(i=0; i<WIFI_SOCKET_MAX; i++)
        {
            if(g_wifi_device_info.m_socket_info[i])
            {
                if(SOCKET_STATUS_CONNECTED == g_wifi_device_info.m_socket_info[i]->m_status)
                {
                    if(-1 == g_client_socket_id[i])
                    {
                        g_client_socket_id[i] = i;
                        ret = i|WIFI_ID_MASK;
                        break;
                    }
                }
            }
        }
    }while(0);

    return ret;
}

/**
 * @brief 创建tcp客户端的socketinfo，给wifi_loop里读到connect客户端连接进入创建
 * @param[in] socket_id 模块端的socket序号
 * @retval  DDI_OK  成功
 * @retval  DDI_ERR 失败
 */
s32 dev_wifi_tcp_client_connect(s32 socket_id)
{
    s32 ret = DDI_ERR;
    s32 sn = 0;

    do{
        TRACE_DBG("enter");
        if(g_wifi_device_info.m_use_tcp_server)
        {
            sn = wifi_get_available_socket_info_index("null", 9999);
            if(sn < 0)
            {
                TRACE_ERR("dev_gprs_socket_request failed:%d", ret);
                ret = DDI_EBUSY;
                break;
            }
            
            TRACE_INFO("have client:%d connect, %d", socket_id, sn);
            if(g_wifi_device_info.m_socket_info[sn])
            {
                g_wifi_device_info.m_socket_info[sn]->m_socket_id = socket_id;
                g_wifi_device_info.m_socket_info[sn]->m_status = SOCKET_STATUS_CONNECTED;
            }
        }
    }while(0);

    return DDI_OK;
}

/**
 * @brief 关闭tcp server监听
 * @param[in] socket_id 模块端的socket序号
 * @retval  DDI_OK  成功
 * @retval  DDI_ERR 失败
 */
s32 dev_wifi_close_tcp_server(void)
{
    s32 ret;

    do{
        if(WF_NO_WIFI_MODULE == dev_wifi_get_status(WIFI_GET_REG_NETWORK_FAIL_REASON))
        {
            ret = DDI_ENODEV;
            break;
        }
        
        if(!g_wifi_device_info.m_use_tcp_server)
        {
            TRACE_INFO("don't creat tcp server, direct return ok");
            return DDI_OK;
            break;
        }

        g_wifi_device_info.m_use_tcp_server = 0;
        TRACE_DBG("enter");
        dev_wifi_close();
        dev_wifi_open();
        dev_wifi_set_control_strategy(WF_NORMAL_RUN);
        ret = DDI_OK;
    }while(0);

    return ret;
}

static s32 wifi_cfg_ntp_server_callback(u8 result, u8 *src, u16 srclen)
{
    TRACE_DBG("result:%d, src:%s", result, src);
    if(WIFI_AT_RESULT_ERROR == result || WIFI_AT_RESULT_TIMEROUT == result || WIFI_AT_RESULT_OK == result)
    {
        dev_wifi_thread_set_result(result);

        if(WIFI_AT_RESULT_TIMEROUT == result)
        {
            dev_wifi_set_control_strategy_step(WIFI_CHECK_CMD);
        }
    }

    return result;
}

/**
 * @brief 配置ntp服务器
 * @param[in] ntp服务器地址
 * @param[in] ntp服务器的个数
 * @retval  DDI_OK  成功
 * @retval  DDI_ERR 失败
 */
s32 dev_wifi_cfg_ntp_server(s8 server_info[][64], s32 server_num)
{
    s32 ret, i;
    s8 tmpbuff[256] = {0};
    wifi_thread_node_param_t *thread_node_param = (wifi_thread_node_param_t *)dev_wifi_get_thread_node_param();

    do{
        if(WF_NO_WIFI_MODULE == dev_wifi_get_status(WIFI_GET_REG_NETWORK_FAIL_REASON))
        {
            ret = DDI_ENODEV;
            break;
        }
        
        if(!dev_wifi_thread_is_free())
        {
            TRACE_ERR("now is hanle task:%d, step:%d", thread_node_param->m_task_id, thread_node_param->m_step);
            ret = DDI_EBUSY;
            break;
        }
        
        memset(tmpbuff, 0, sizeof(tmpbuff));
        snprintf(tmpbuff, sizeof(tmpbuff), "%s", "AT+CIPSNTPCFG=1,8");
        if(server_num > 0)
        {
            for(i=0; i<server_num; i++)
            {
                snprintf(tmpbuff+strlen(tmpbuff), sizeof(tmpbuff)-strlen(tmpbuff), 
                       ",\"%s\"", server_info[i]);
            }
        }

        TRACE_DBG("%s", tmpbuff);
        dev_wifi_send_at_cmd(tmpbuff, AT_API_BLOCK, TRUE);
        dev_wifi_thread_set_param(WIFI_TASK_CFG_NTP_SERVER, wifi_cfg_ntp_server_callback, WIFI_AT_CMD_RESEND_TIMEOUT);
        while(1)
        {
            ddi_watchdog_feed();
            dev_wifi_thread_run_switch(FALSE);
            if(dev_wifi_thread_task_is_change(WIFI_TASK_CFG_NTP_SERVER))
            {
                ret = DDI_EBUSY;
                break;
            }
            
            if(dev_wifi_thread_is_free())
            {
                switch(thread_node_param->m_result)
                {
                    case WIFI_AT_RESULT_OK:
                        ret = DDI_OK;
                        break;

                    case WIFI_AT_RESULT_ERROR:
                        ret = DDI_ERR;
                        break;

                    case WIFI_AT_RESULT_TIMEROUT:
                        ret = DDI_ETIMEOUT;
                        break;
                }

                break;
            }
            
            dev_wifi_thread_run_switch(TRUE);

            ddi_misc_msleep(50);
        }
    }while(0);

    dev_wifi_thread_run_switch(TRUE);

    return ret;
}

static s8 g_wifi_ntp[64] = {0};
static s32 wifi_at_get_ntp_callback(u8 result, u8 *src, u16 srclen)
{
    s8 *pstr = NULL;
    
    if(result == WIFI_AT_RESULT_TIMEROUT || result == WIFI_AT_RESULT_OK)
    {
        dev_wifi_thread_set_result(result);
    }
    else if(result == WIFI_AT_RESULT_OTHER)
    {
        pstr = strstr(src, "+CIPSNTPTIME:");
        if(pstr)
        {
            memset(g_wifi_ntp, 0, sizeof(g_wifi_ntp));
            snprintf(g_wifi_ntp, sizeof(g_wifi_ntp), "%s", pstr+strlen("+CIPSNTPTIME:"));
        }
    }
    
    return 0;
}

/**
 * @brief 获取ntp时间
 * @param[in] ntp_info:用于填充时间的buff
 * @param[in] ntp_info_len:用于填充时间的buff长度
 * @retval  DDI_OK  成功
 * @retval  DDI_ERR 失败
 */
s32 dev_wifi_get_ntp(s8 *ntp_info, s32 ntp_info_len)
{
    s8 tmpbuff[256] = {0};
    s32 ret, i;
    wifi_thread_node_param_t *thread_node_param = (wifi_thread_node_param_t *)dev_wifi_get_thread_node_param();

    do{
        if(WF_NO_WIFI_MODULE == dev_wifi_get_status(WIFI_GET_REG_NETWORK_FAIL_REASON))
        {
            ret = DDI_ENODEV;
            break;
        }
        
        if(!dev_wifi_thread_is_free())
        {
            TRACE_ERR("now is hanle task:%d, step:%d", thread_node_param->m_task_id, thread_node_param->m_step);
            ret = DDI_EBUSY;
            break;
        }
        
        dev_wifi_send_at_cmd("AT+CIPSNTPTIME?", AT_API_BLOCK, TRUE);
        dev_wifi_thread_set_param(WIFI_TASK_QRY_RTC, wifi_at_get_ntp_callback, WIFI_AT_CMD_RESEND_TIMEOUT);
        while(1)
        {
            ddi_watchdog_feed();
            dev_wifi_thread_run_switch(FALSE);
            if(dev_wifi_thread_task_is_change(WIFI_TASK_QRY_RTC))
            {
                ret = DDI_EBUSY;
                break;
            }
            
            if(dev_wifi_thread_is_free())
            {
                switch(thread_node_param->m_result)
                {
                    case WIFI_AT_RESULT_OK:
                        snprintf(ntp_info, ntp_info_len, "%s", g_wifi_ntp);
                        ret = DDI_OK;
                        break;

                    case WIFI_AT_RESULT_ERROR:
                        ret = DDI_ERR;
                        break;

                    case WIFI_AT_RESULT_TIMEROUT:
                        ret = DDI_ETIMEOUT;
                        break;
                }

                break;
            }
            
            dev_wifi_thread_run_switch(TRUE);

            ddi_misc_msleep(50);
        }
    }while(0);

    dev_wifi_thread_run_switch(TRUE);

    return ret;
}

static s32 wifi_get_net_connect_state_callback(u8 result, u8 *src, u16 srclen)
{
    s8 *pstr = NULL;
    s32 status = 0;
    wifi_thread_node_param_t *thread_node_param = (wifi_thread_node_param_t *)dev_wifi_get_thread_node_param();
    u8 *socket_list = (u8 *)thread_node_param->m_addr_param;

    TRACE_DBG("result:%d, src:%s", result, src);
    if(result == WIFI_AT_RESULT_TIMEROUT || result == WIFI_AT_RESULT_OK)
    {
        status = thread_node_param->m_lparam;
        dev_wifi_thread_set_result(result);
        thread_node_param->m_lparam = status;
    }
    else if(result == WIFI_AT_RESULT_OTHER)
    {
        //2：ESP8266 Station 已连接 AP，获得 IP 地址
        //3：ESP8266 Station 已建立TCP 或 UDP 传输
        //4：ESP8266 Station 断开网络连接
        //5：ESP8266 Station 未连接 AP
        pstr = strstr(src, "STATUS:");
        if(pstr)
        {
            pstr += strlen("STATUS:");
            dev_maths_str_trim(pstr);
            status = dev_maths_asc_to_u32(pstr, strlen(pstr), MODULUS_DEC);
            TRACE_DBG("status:%d,%s", status, pstr);
            if(5 == status)
            {
                TRACE_INFO("ap disconnect, then clear local info");
                dev_wifi_close();   //清除本地缓存数据
            }
            else if(4 == status)   //所有套接字已关闭
            {
                TRACE_INFO("all socket is close, then clear tcp local info");
                //释放本地的所有socket资源
                wifi_all_tcp_disconnect_hanle();
            }
            else if(2 == status)
            {
                g_wifi_device_info.m_connect_ap_res = WIFI_STATUS_CONNECT_AP_CONNECTED;
            }
        }

        //+CIPSTATUS:<link	ID>,<type>,<remote	IP>,<remote	port>,<local	port>,<tetype>
        pstr = strstr(src, "+CIPSTATUS:");
        if(pstr)
        {
            pstr += strlen("+CIPSTATUS:");
            dev_maths_str_trim(pstr);
            if(thread_node_param->m_lparam < WIFI_SOCKET_MAX)
            {
                socket_list[thread_node_param->m_lparam] = dev_maths_asc_to_u32(pstr, strlen(pstr), MODULUS_DEC);
                TRACE_DBG("valid socket id:%d", socket_list[thread_node_param->m_lparam]);
                thread_node_param->m_lparam++;
            }
        }
    }
    
    return 0;
}

static void wifi_sync_local_socketlist(u8 *socket_list, s32 socket_num)
{
    s32 i = 0, j = 0;
    s32 flag = 0;
    
    do{
        if(0 == socket_num)
        {
            TRACE_INFO("have handled, exit");
            break;
        }

        TRACE_DBG("module socket num:%d", socket_num);
        //判断本地socket清单是否在模块清单里，不存在释放本地socket资源
        for(i = 0; i < WIFI_SOCKET_MAX; i++)
        {
            if(g_wifi_device_info.m_socket_info[i])
            {
                flag = 0;
                for(j=0; j<socket_num; j++)
                {
                    TRACE_DBG("j[%d]:%d,%d", j, socket_list[j], g_wifi_device_info.m_socket_info[i]->m_socket_id);
                    if(socket_list[j] == g_wifi_device_info.m_socket_info[i]->m_socket_id)
                    {
                        TRACE_ERR("socketinfo exit:%d", g_wifi_device_info.m_socket_info[i]->m_socket_id);
                        socket_list[j] = 255;
                        flag = 1;
                        break;
                    }
                }

                //模块端不存在， 只释放本地资源
                if(0 == flag)
                {
                    TRACE_DBG("can't find this socket in module:[%d]%d", i, g_wifi_device_info.m_socket_info[i]->m_socket_id);
                    wifi_socket_info_destroy(g_wifi_device_info.m_socket_info[i]);
                    g_wifi_device_info.m_socket_info[i] = NULL;
                }
            }
        }

        //如果模块端存在但是本地不存在，则关闭模块端套接字
        for(j=0; j<socket_num; j++)
        {
            if(socket_list[j] != 255)
            {
                TRACE_INFO("close socket");
                wifi_close_socket(socket_list[j]);
            }
        }
    }while(0);
}

/**
 * @brief 获取网络连接状态
 * @param[in] ntp_info:用于填充时间的buff
 * @param[in] ntp_info_len:用于填充时间的buff长度
 * @retval  DDI_OK  成功
 * @retval  DDI_ERR 失败
 */
static s32 wifi_get_net_connect_state(void)
{
    s8 tmpbuff[256] = {0};
    s32 ret, i;
    wifi_thread_node_param_t *thread_node_param = (wifi_thread_node_param_t *)dev_wifi_get_thread_node_param();
    u8 socket_list[WIFI_SOCKET_MAX];

    do{
        if(!dev_wifi_thread_is_free())
        {
            TRACE_ERR("now is hanle task:%d, step:%d", thread_node_param->m_task_id, thread_node_param->m_step);
            ret = DDI_EBUSY;
            break;
        }

        memset(socket_list, 0, sizeof(socket_list));
        dev_wifi_send_at_cmd("AT+CIPSTATUS", AT_API_BLOCK, TRUE);
        dev_wifi_thread_set_param(WIFI_TASK_GET_NET_STATE, wifi_get_net_connect_state_callback, WIFI_AT_CMD_RESEND_TIMEOUT);
        thread_node_param->m_addr_param = (u32)socket_list;
        while(1)
        {
            ddi_watchdog_feed();
            dev_wifi_thread_run_switch(FALSE);
            if(dev_wifi_thread_task_is_change(WIFI_TASK_GET_NET_STATE))
            {
                ret = DDI_EBUSY;
                break;
            }
            
            if(dev_wifi_thread_is_free())
            {
                switch(thread_node_param->m_result)
                {
                    case WIFI_AT_RESULT_OK:
                        wifi_sync_local_socketlist(socket_list, thread_node_param->m_lparam);
                        ret = DDI_OK;
                        break;

                    case WIFI_AT_RESULT_ERROR:
                        ret = DDI_ERR;
                        break;

                    case WIFI_AT_RESULT_TIMEROUT:
                        ret = DDI_ETIMEOUT;
                        break;
                }

                break;
            }
            
            dev_wifi_thread_run_switch(TRUE);
            ddi_misc_msleep(50);
        }
    }while(0);

    dev_wifi_thread_run_switch(TRUE);
    return ret;
}

s32 dev_wifi_ioctl(u32 nCmd, u32 lParam, u32 wParam)
{
    s32 ret = DDI_EINVAL;

    switch(nCmd)
    {
        case WIFI_IOCTL_GET_FIRMWARE_VER:
            if(wParam == NULL)
            {
                ret = DDI_EINVAL;
                break;
            }

            if(WIFI_VER_AT == lParam)
            {
                strcpy((u8 *)wParam, g_wifi_device_info.m_at_ver);
            }
            else if(WIFI_VER_SDK == lParam)
            {
                strcpy((u8 *)wParam, g_wifi_device_info.m_sdk_ver);
            }
            break;
            
        case WIFI_IOCTL_CREAT_TCP_SERVER:
            ret = dev_wifi_creat_tcp_server((s8 *)lParam, (u16)wParam);
            break;

        case WIFI_IOCTL_TCP_SERVER_ACCEPT:
            ret = dev_wifi_tcp_server_accept();
            break;

        case WIFI_IOCTL_TCP_SERVER_CLOSE:
            ret = dev_wifi_close_tcp_server();
            break;

        case WIFI_IOCTL_REG_EVENT:
            TRACE_DBG("enter");
            g_wifi_device_info.m_notify_func = (wifi_notify_event_func)lParam;
            ret = DDI_OK;
            break;

        case WIFI_IOCTL_CFG_NTP_SERVER:
            ret = dev_wifi_cfg_ntp_server((s8 (*)[64])lParam, (s32)wParam);
            break;

        case WIFI_IOCTL_GET_NTP:
            ret = dev_wifi_get_ntp((s8 *)lParam, (s32)wParam);
            break;

        case WIFI_IOCTL_GET_NET_INFO:
            ret = dev_wifi_get_netinfo((wifi_net_info_t *)lParam);
            break;

        case WIFI_IOCTL_CLEAR_RF_PARAM:
            ret = dev_wifi_clear_rf_param();
            break;

        case WIFI_IOCTL_CMD_TRANSPARENT:
            g_wifi_device_info.m_transparent_switch = lParam;
            break;

        case WIFI_IOCTL_CMD_REPOWER:
            dev_gpio_direction_output(WIFI_EN_PIN, 0);
            ddi_misc_msleep(500);
            dev_gpio_direction_output(WIFI_EN_PIN, 1);
            ret = DDI_OK;
            break;
    }
    
    return ret;
}

/**
 * @brief 清除wifi的射频参数，让wifi模块重新校准射频参数
 * 背景：P7和T6生产时，出现一批机器wifi扫描不到热点，乐鑫后面分析说是贴片后第一次开机有干扰，导致第一次校准射频参数出问题
 * @retval  DDI_OK 成功
 * @retval  DDI_ERR 有按键干预
 */
s32 dev_wifi_clear_rf_param(void)
{
    s8 tmpbuff[256] = {0};
    s32 ret, i;
    wifi_thread_node_param_t *thread_node_param = (wifi_thread_node_param_t *)dev_wifi_get_thread_node_param();

    do{
        if(WF_NO_WIFI_MODULE == dev_wifi_get_status(WIFI_GET_REG_NETWORK_FAIL_REASON))
        {
            ret = DDI_ENODEV;
            break;
        }
        
        if(!dev_wifi_thread_is_free())
        {
            TRACE_ERR("now is hanle task:%d, step:%d", thread_node_param->m_task_id, thread_node_param->m_step);
            ret = DDI_EBUSY;
            break;
        }
        
        dev_wifi_send_at_cmd("AT+RFCALRST", AT_API_BLOCK, TRUE);
        dev_wifi_thread_set_param(WIFI_TASK_CLEAR_RF_PARAM, wifi_comom_response_callback, WIFI_AT_CMD_RESEND_TIMEOUT*5);
        while(1)
        {
            ddi_watchdog_feed();
            dev_wifi_thread_run_switch(FALSE);
            if(dev_wifi_thread_task_is_change(WIFI_TASK_CLEAR_RF_PARAM))
            {
                ret = DDI_EBUSY;
                break;
            }
            
            if(dev_wifi_thread_is_free())
            {
                switch(thread_node_param->m_result)
                {
                    case WIFI_AT_RESULT_OK:
                        ret = DDI_OK;
                        break;

                    case WIFI_AT_RESULT_ERROR:
                        ret = DDI_ERR;
                        break;

                    case WIFI_AT_RESULT_TIMEROUT:
                        ret = DDI_ETIMEOUT;
                        break;
                }

                break;
            }
            
            dev_wifi_thread_run_switch(TRUE);

            ddi_misc_msleep(50);
        }
    }while(0);

    dev_wifi_thread_run_switch(TRUE);

    if(DDI_OK == ret)
    {
        dev_wifi_close();
        dev_wifi_open();
    }
    return ret;
}
#else
s32 dev_wifi_init(void)
{
    return DDI_ERR;
}
#endif