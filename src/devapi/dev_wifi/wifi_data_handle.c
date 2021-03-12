#include "devglobal.h"
#include "drv_wifi_io.h"
#include "dev_wifi_thread.h"
#include "dev_wifi.h"
#include "wifi_control_strategy.h"
#include "wifi_data_handle.h"

#if (WIFI_EXIST == 1)
static wifi_parse_t  g_wifi_parse;
static wifi_at_result_info_t g_wifi_at_result_info_tab[] = {
    {WIFI_AT_RESULT_MODULE_READY, "ready"},
    {WIFI_AT_RESULT_OK, "OK"},
    {WIFI_AT_RESULT_ERROR, "ERROR"}
};

extern UINT32 MQTT_TASK_FOUR;
static s32 g_wifi_is_need_restore_thread_param = FALSE;
static wifi_thread_node_param_t g_last_thread_node_param;

/**
 * @brief 接收完tcp数据，将线程参数恢复成之前的参数
 * @param[in] 无
 * @retval  无
 */
static void wifi_recovery_thread_param(void)
{
    if(TRUE == g_wifi_is_need_restore_thread_param)
    {
        TRACE_INFO("recovery thread to %d", g_last_thread_node_param.m_task_id);
        g_wifi_is_need_restore_thread_param = FALSE;
        dev_wifi_set_thread_node_param(&g_last_thread_node_param);
    }
    else
    {
        dev_wifi_thread_set_result(WIFI_AT_RESULT_OK);
    }
}

/**
 * @brief P3上使用，数据由系统一次发过来
 * @param[in] 无
 * @retval  无
 */
void dev_wifi_set_parse_data(u8 *data, u16 len)
{
    wifi_device_info_t *device_info = dev_wifi_get_device();
    
    dev_circlequeue_write(&(device_info->m_wifi_uart_rx_fifo), data, len);
}

/**
 * @brief P3上使用，数据由系统一次发过来
 * @param[in] 无
 * @retval  无
 */
static s32 wifi_data_recv(u8 *buff, u16 len)
{
    wifi_device_info_t *device_info = dev_wifi_get_device();

    return dev_circlequeue_read(&(device_info->m_wifi_uart_rx_fifo), buff, len);
}

/**
 * @brief 无线模块已知返回的命令处理
 * @param[in] 无
 * @retval  1 有已知返回数据
 * @retval  0 无已知返回数据
 */
static s32 wifi_data_know_response(void)
{
    u32 i;
    wifi_thread_node_param_t *thread_node_param = (wifi_thread_node_param_t *)dev_wifi_get_thread_node_param();
    s32 ret = 0;

    if(g_wifi_parse.m_rcnt<WIFIAT_RXBUF_MAX)
    {
        g_wifi_parse.m_rbuf[g_wifi_parse.m_rcnt] = 0;
    }
    
    i = 0;
    for(i=0; i<sizeof(g_wifi_at_result_info_tab)/sizeof(g_wifi_at_result_info_tab[0]); i++)
    {
        if(0 == strcmp(g_wifi_parse.m_rbuf, g_wifi_at_result_info_tab[i].m_resp_words))
        {
            thread_node_param->m_result = g_wifi_at_result_info_tab[i].m_wifi_at_result;
            ret = 1;           //已知返回内容
            break;
        }
    }

    return ret;
}

/**
 * @brief 无线模块主动上送指令处理
 * @param[in] 无
 * @retval  1 有已知返回数据
 * @retval  0 无已知返回数据
 */
static s32 wifi_data_report(void)
{
    wifi_thread_node_param_t *thread_node_param = (wifi_thread_node_param_t *)dev_wifi_get_thread_node_param();
    wifi_device_info_t *device_info = dev_wifi_get_device();
    s8 dst[3][SPLIT_PER_MAX] = {0};
    s32 ret = 0;
    s32 socket_sn = 0;
    s32 socket_id = 0;
    s32 len = 0;
    s8 buff[128] = {0};

    do{
        if(strstr(g_wifi_parse.m_rbuf, WIFI_AT_RES_AP_CONNECT_AP_TIMEOUT))
        {
            TRACE_INFO("connect ap timeout");
            device_info->m_connect_ap_res = WIFI_STATUS_CONNECT_AP_DISCONNECTED;
        }
        else if(strstr(g_wifi_parse.m_rbuf, WIFI_AT_RES_AP_CONNECT_AP_PASSWORD_ERR))
        {
            TRACE_INFO("ap password is error");
            device_info->m_connect_ap_res = WIFI_STATUS_CONNECT_AP_DISCONNECTED;
        }
        else if(strstr(g_wifi_parse.m_rbuf, WIFI_AT_RES_AP_CONNECT_AP_NO_FOUND))
        {
            TRACE_INFO("can't find ap:%s, then retry", device_info->m_ssid);
            device_info->m_connect_ap_res = WIFI_STATUS_CONNECT_AP_DISCONNECTED;
        }
        else if(strstr(g_wifi_parse.m_rbuf, WIFI_AT_RES_AP_CONNECT_AP_FAIL))
        {
            TRACE_INFO("connect ap:%s fail, then retry", device_info->m_ssid);
            device_info->m_connect_ap_res = WIFI_STATUS_CONNECT_AP_DISCONNECTED;
        }
        
        if(strstr(g_wifi_parse.m_rbuf, WIFI_AT_RES_AP_GETIP_TIMEOUT))
        {
            TRACE_INFO("get dhcp timeout");
            
            if(WIFI_AIRKISS_FREE != device_info->m_airkiss_step)
            {
                TRACE_INFO("get dhcp timeout");
                device_info->m_airkiss_step = WIFI_AIRKISS_DHCP_TIMEOUT;
                ret = 1;
            }
            else
            {
                TRACE_INFO("get ip overtime, then retry");
                device_info->m_connect_ap_res = WIFI_STATUS_CONNECT_AP_DISCONNECTED;
            }
        }
        
        //热点连接成功
        if(0 == strcmp(g_wifi_parse.m_rbuf, WIFI_AT_RES_AP_CONNECTED))
        {
            if(WIFI_AIRKISS_FREE == device_info->m_airkiss_step)
            {
                TRACE_INFO("connect ap success");
                device_info->m_connect_ap_res = WIFI_STATUS_CONNECT_AP_CONNECTED;
                //snprintf(buff, sizeof(buff), "热点%s连接成功", device_info->m_ssid);
                //ddi_gb2312_tts_play_text(buff);
                ret = 1;
            }
            break;
        }

        //热点断开
        if(0 == strcmp(g_wifi_parse.m_rbuf, WIFI_AT_RES_AP_DISCONNECT))
        {
            TRACE_INFO("ap disconnect, retry to connect");
            device_info->m_connect_ap_res = WIFI_STATUS_CONNECT_AP_DISCONNECTED;
            memset(device_info->m_local_ip, 0, sizeof(device_info->m_local_ip));
            dev_wifi_set_socket_status(0xff, SOCKET_STATUS_DISCONNECTED);
            ret = 1;
            break;
        }

        //tcp连接成功
        if(strstr(g_wifi_parse.m_rbuf, WIFI_AT_RES_TCP_CONNECT))
        {
            memset(dst, 0, sizeof(dst));
            ret = dev_maths_split(dst, sizeof(dst)/sizeof(dst[0]), g_wifi_parse.m_rbuf, ",", SPLIT_NOIGNOR);
            if(ret > 1)
            {
                dev_maths_str_trim(dst[0]);
                dev_maths_str_trim(dst[1]);
                socket_id = dev_maths_asc_to_u32(dst[0], strlen(dst[0]), MODULUS_DEC);
                socket_sn = dev_wifi_socket_info_sn_by_id(socket_id);

                if(socket_sn < 0)
                {
                    TRACE_INFO("can't find sn by socket_id:%d", socket_id);
                    //tcp server需要创建socket infor,是否是tcp server，由接口内部判断
                    dev_wifi_tcp_client_connect(socket_id);
                }
                else
                {
                    if(device_info->m_socket_info[socket_sn])
                    {
                        if(!device_info->m_use_tcp_server && device_info->m_notify_func)
                        {
                            device_info->m_notify_func(WN_EVENT_TCP_CONNECTED, socket_sn|WIFI_ID_MASK, 0);
                        }
                        device_info->m_socket_info[socket_sn]->m_status = SOCKET_STATUS_CONNECTED;
                        TRACE_INFO("sn:%d connect success", socket_sn);
                    }
                }
                ret = 1;
                break;
            }
        }

        //tcp连接断开
        if(strstr(g_wifi_parse.m_rbuf, WIFI_AT_RES_TCP_CLOSED))
        {
            memset(dst, 0, sizeof(dst));
            ret = dev_maths_split(dst, sizeof(dst)/sizeof(dst[0]), g_wifi_parse.m_rbuf, ",", SPLIT_NOIGNOR);
            if(ret > 1)
            {
                dev_maths_str_trim(dst[0]);
                dev_maths_str_trim(dst[1]);
                socket_id = dev_maths_asc_to_u32(dst[0], strlen(dst[0]), MODULUS_DEC);
                socket_sn = dev_wifi_socket_info_sn_by_id(socket_id);
                if(!device_info->m_use_tcp_server && device_info->m_notify_func)
                {
                    device_info->m_notify_func(WN_EVENT_TCP_DISCONNECT, socket_sn|WIFI_ID_MASK, 0);
                }
                TRACE_INFO("socket close by peer, infor:%s, close socketid:%d", g_wifi_parse.m_rbuf, socket_id);
                dev_wifi_set_socket_status(socket_id, SOCKET_STATUS_DISCONNECTED);
                ret = 1;
                break;
            }
        }

        //开启airkiss配网模式
        if(0 == memcmp(g_wifi_parse.m_rbuf, "smartconfig type:AIRKISS", strlen("smartconfig type:AIRKISS")))
        {
            TRACE_INFO("start to airkiss conpair");
            device_info->m_airkiss_step = WIFI_AIRKISS_SEARCHING;
            ret = 1;
            break;
        } 
        else if(strstr(g_wifi_parse.m_rbuf, "get wifi info"))
        {
            TRACE_INFO("ready to get config infor");
            device_info->m_airkiss_step = WIFI_AIRKISS_GETCONFIGING;
            ret = 1;
            break;
        }
        else if(strstr(g_wifi_parse.m_rbuf, "ssid:"))
        {
            len = strlen("ssid:");
            //TRACE_DBG_HEX(g_wifi_parse.m_rbuf+len, strlen(g_wifi_parse.m_rbuf)-len);
            snprintf(device_info->m_ssid, sizeof(device_info->m_ssid), "%s", g_wifi_parse.m_rbuf+len);
            TRACE_DBG("%s", device_info->m_ssid);
            ret = 1;
            break;
        }
        else if(strstr(g_wifi_parse.m_rbuf, "password:"))
        {
            len = strlen("password:");
            //TRACE_DBG_HEX(g_wifi_parse.m_rbuf+len, strlen(g_wifi_parse.m_rbuf)-len);
            snprintf(device_info->m_password, sizeof(device_info->m_password), "%s", g_wifi_parse.m_rbuf+len);
            TRACE_DBG("%s", device_info->m_password);
            device_info->m_airkiss_step = WIFI_AIRKISS_GETCONFIG;
            device_info->m_airkiss_change = TRUE;
            ret = 1;
            break;
        }
        else if(strstr(g_wifi_parse.m_rbuf, "smartconfig connected wifi"))
        {
            TRACE_INFO("airkiss connect ap success");
            device_info->m_airkiss_step = WIFI_AIRKISS_CONNECT_OK;
            dev_wifi_thread_set_result(WIFI_AT_RESULT_OK);
            ret = 1;
            break;
        }
        else if(0 == memcmp(g_wifi_parse.m_rbuf, "smartconfig connect fail", strlen("smartconfig connect fail")))
        {
            TRACE_INFO("airkiss connect ap fail");
            device_info->m_airkiss_step = WIFI_AIRKISS_CONNECT_ERR;
            dev_wifi_thread_set_result(WIFI_AT_RESULT_ERROR);
            ret = 1;
            break;
        }
    }while(0);

    return ret;
}


/**
 * @brief 挂在无线模块端设备的控制指令返回
 * @param[in] 无
 * @retval  1 无线模块指令应答数据
 * @retval  3 无线模块指令应答数据
 * @retval  2 无线模块主动上送的数据，已处理，外面不需要再处理
 */
static s32 wifi_data_handle(void)
{
    u32 i;
    u32 j;
    wifi_thread_node_param_t *thread_node_param = (wifi_thread_node_param_t *)dev_wifi_get_thread_node_param();
    s32 ret = 0;

    if(g_wifi_parse.m_rcnt<WIFIAT_RXBUF_MAX)
    {
        g_wifi_parse.m_rbuf[g_wifi_parse.m_rcnt] = 0;
    }

    do{
        //任务函数不为空的时候处理，一条指令应答如close socket，先回ok,后面再回复socket关闭情况，如果判断数据返回那里有处理，且中间有休眠，获取的状态result就被这里修改了。
        if(NULL != thread_node_param->m_func)
        {
            if(wifi_data_know_response())
            {
                ret = 1;
                goto _out;
                break;
            }
        }

        if(wifi_data_report())
        {
            ret = 2;
            goto _out;
            break;
        }
    }while(0);

    
    if(NULL != thread_node_param->m_func)
    {
        thread_node_param->m_result = WIFI_AT_RESULT_OTHER;
        ret = 3;
    }

_out:    
    return ret;
}

/**
 * @brief 接收返回指令内容短的数据
 * @param[in] 无
 * @retval  > 0 有数据
 * @retval  0 读到数据，但是是无效数据或是tcp接收数据的头部关键字
 * @retval  -1 没有数据
 */
static s32 wifi_data_recv_normal(void)
{
    s32 i = 0;
    u8 data;
    wifi_thread_node_param_t *thread_node_param = (wifi_thread_node_param_t *)dev_wifi_get_thread_node_param();
    s8 dst[4][SPLIT_PER_MAX];
    s32 ret = 0;
    s8 *pstr = NULL;
    u32 timeover = 0;
    wifi_device_info_t *device_info = dev_wifi_get_device();
    s32 socket_id = 0;
    s32 socket_sn = 0;

#if(WIFI_RUN_OPEN_CPU == FALSE)
    i = dev_com_read(WIFI_PORT, &data, 1);
#else
    i = wifi_data_recv(&data, 1);
#endif
    if(i <= 0)
    {
        return -1;
    }

    g_wifi_parse.m_rbuf[g_wifi_parse.m_rcnt++] = data;
    if(g_wifi_parse.m_rcnt == 2)
    {
        if(0 == memcmp(&g_wifi_parse.m_rbuf[0], AT_CMD_RES_END, 2))
        {
            //接收完成,仅收到0d0a,扔掉
            g_wifi_parse.m_rcnt = 0;
            return 0;
        }

        //add by xiaohonghui 2020.8.11 判断是否是"> "
        if(0 == memcmp(&g_wifi_parse.m_rbuf[0], WIFI_AT_READY_SEND_DATA, 2))
        {
            g_wifi_parse.m_rbuf[1] = 0;
            return 2;
        }
    }
    else if(g_wifi_parse.m_rcnt>2)
    {
        if(g_wifi_parse.m_rcnt > strlen("+IPD")
            && 0 == memcmp(g_wifi_parse.m_rbuf, "+IPD", strlen("+IPD"))
            && ':' == data)
        {
            g_wifi_parse.m_rbuf[g_wifi_parse.m_rcnt-1] = 0;
            TRACE_DBG("m_rbuf:%s", g_wifi_parse.m_rbuf);
            ret = dev_maths_split(dst, sizeof(dst)/sizeof(dst[0]), g_wifi_parse.m_rbuf, ",", SPLIT_NOIGNOR);
            if(ret == 3)
            {
                dev_maths_str_trim(dst[1]);
                dev_maths_str_trim(dst[2]);
                TRACE_DBG("socketid:%d, recvlen:%d", dev_maths_asc_to_u32(dst[1], strlen(dst[1]), MODULUS_DEC),
                                                     dev_maths_asc_to_u32(dst[2], strlen(dst[2]), MODULUS_DEC));

                socket_id = dev_maths_asc_to_u32(dst[1], strlen(dst[1]), MODULUS_DEC);
                socket_sn = dev_wifi_socket_info_sn_by_id(socket_id);
                if(socket_sn < 0 || socket_sn >= WIFI_SOCKET_MAX)
                {
                    TRACE_ERR("socket id don't exist:%d", socket_id);
                    return 0;
                }
                else
                {
                    g_wifi_parse.m_socketid = socket_id;
                    g_wifi_parse.m_rlencnt = dev_maths_asc_to_u32(dst[2], strlen(dst[2]), MODULUS_DEC);
                    g_wifi_parse.m_special |= WIFI_SPECIAL_BIT0;
                    g_wifi_parse.m_rcnt = 0;

                    //串口1秒12K左右
                    timeover = (g_wifi_parse.m_rlencnt*1000*10)/(12*1024);
                    if(timeover < 3000)
                    {
                        timeover = WIFI_AT_RECV_DEFAULT_WAIT_TIMEOUT;
                    }

                    //当前有其它任务在执行
                    if(!dev_wifi_thread_is_free())
                    {
                        TRACE_DBG("ready to save thread:%d", thread_node_param->m_task_id);
                        g_wifi_is_need_restore_thread_param = TRUE;
                        memcpy(&g_last_thread_node_param, thread_node_param, sizeof(wifi_thread_node_param_t));
                    }
                    
                    dev_wifi_thread_set_param(WIFI_TASK_SOCKET_RECVDATA, NULL, timeover);
                    memset(g_wifi_parse.m_rbuf, 0, sizeof(g_wifi_parse.m_rbuf));
                    TRACE_INFO("timeover:%d, %02x", timeover, g_wifi_parse.m_special);
                    return 0;
                }
            }
        }
        
        if(0 == memcmp(&g_wifi_parse.m_rbuf[g_wifi_parse.m_rcnt-2], AT_CMD_RES_END, 2))
        {
            g_wifi_parse.m_rbuf[g_wifi_parse.m_rcnt-2] = 0;
            g_wifi_parse.m_rcnt -= 2;
            //接收完成
            return 2;
        }

        if(g_wifi_parse.m_rcnt >= WIFIAT_RXBUF_MAX)
        {
            TRACE_ERR("recv overflow");
            g_wifi_parse.m_rcnt = 0;
            return 0;
        }
    }
    else if(1 == g_wifi_parse.m_rcnt && g_wifi_parse.m_rbuf[0] == 0)
    {
        //接收完成,仅收到0d0a,扔掉
        g_wifi_parse.m_rcnt = 0;
        return 0;
    }

    return 0;
}

/**
 * @brief 接收tcp套接字的后部分数据
 * @param[in] 无
 * @retval  -1:不需要外部去处理数据，所以直接返回-1
 */
static s32 wifi_data_recv_miprtcp(void)
{
    s32 socket_sn = 0;
    wifi_device_info_t *device_info = dev_wifi_get_device();
    s32 i = 0, j = 0;
    s32 ret = 0;
    s32 clean_recv_flag = FALSE;

    do{
        socket_sn = dev_wifi_socket_info_sn_by_id(g_wifi_parse.m_socketid);
        
        if(socket_sn < 0 || socket_sn >= WIFI_SOCKET_MAX)
        {
            TRACE_ERR("socket id don't exist:%d", g_wifi_parse.m_socketid);
            clean_recv_flag = TRUE;
            break;
        }

        if(NULL == device_info->m_socket_info[socket_sn])
        {
            TRACE_ERR("socket infor is NULL");
            clean_recv_flag = TRUE;
            break;
        }
        
        i = dev_circlequeue_getspace(&(device_info->m_socket_info[socket_sn]->m_read_buffer));
        j = i > WIFIAT_RXBUF_MAX ? WIFIAT_RXBUF_MAX : i;
        j = g_wifi_parse.m_rlencnt > j ? j : g_wifi_parse.m_rlencnt;

#if(WIFI_RUN_OPEN_CPU == FALSE)
        ret = dev_com_read(WIFI_PORT, g_wifi_parse.m_rbuf, j);
#else
        ret = wifi_data_recv(g_wifi_parse.m_rbuf, j);
#endif
        
        if(ret <= 0)
        {
            TRACE_ERR("j:%d, i:%d, ret:%d", j, i, ret);
            break;
        }
        else
        {
           TRACE_DBG("ret len:%d,%d", ret, g_wifi_parse.m_rlencnt);
        }

        dev_circlequeue_write(&(device_info->m_socket_info[socket_sn]->m_read_buffer), g_wifi_parse.m_rbuf, ret);
        g_wifi_parse.m_rlencnt -= ret;

        TRACE_DBG("11ret len:%d,%d", ret, g_wifi_parse.m_rlencnt);
        if(g_wifi_parse.m_rlencnt <= 0)
        {
            TRACE_INFO("this packet recv ok");
            clean_recv_flag = TRUE;
            
            ret = dev_circlequeue_getdatanum(&(device_info->m_socket_info[socket_sn]->m_read_buffer));
            TRACE_DBG("datalen:%d", ret);

            if(!device_info->m_use_tcp_server && device_info->m_notify_func)
            {
                device_info->m_notify_func(WN_EVENT_TCP_DATA_IN, socket_sn|WIFI_ID_MASK, ret);
            }
        }
    }while(0);

    if(clean_recv_flag)
    {
        //dev_wifi_thread_set_result(WIFI_AT_RESULT_OK);    //Mask by xiaohonghui 2020.8.13 会导致其它task在执行的时候得到了结果
        //恢复线程参数
        wifi_recovery_thread_param();
        dev_wifi_init_parse_data();
    }
    return -1;
}

/**
 * @brief 数据接收与解析
 * @param[in] 无
 * @retval  > 0 有数据
 * @retval  0 没有数据
 */
static s32 wifi_data_recv_and_parse(void)
{
    s32 ret;
    u8 data = 0;

    while(1)
    {
        if(g_wifi_parse.m_special&WIFI_SPECIAL_BIT0)  //收取tcp数据
        {
            ret = wifi_data_recv_miprtcp();
        } 
        else
        {
            ret = wifi_data_recv_normal();               //收取短的指令返回数据
        }
        
        if(ret < 0)
        {
            ret = 0;
            break;
        }
        else if(ret != 0)
        {
            break;
        }
    }

    return ret;
}

void dev_wifi_set_parse_flag(s32 bit)
{
    g_wifi_parse.m_special |= bit;
    TRACE_DBG("g_wifi_parse.m_special:%x", g_wifi_parse.m_special);
}

/**
 * @brief 初始化全局变量
 * @param[in] 无
 * @retval  无
 */
void dev_wifi_init_parse_data(void)
{
    memset(&g_wifi_parse, 0, sizeof(g_wifi_parse));
}

/**
 * @brief 将str字符以spl分割,存于dst中，并返回子字符串数量
 * @param[in] dst 目标存储字符串
 * @param[in] dst_num 缓存个数
 * @param[in] str 需要分离的字符串
 * @param[in] spl 分隔符
 * @param[in] split_flag 长度为0的是否需忽略
 * @retval  1 时间到
 * @retval  0 时间未到
 */
s32 dev_wifi_data_handle(void)
{
    s32 ret = DDI_ERR;
    wifi_thread_node_param_t *thread_node_param = (wifi_thread_node_param_t *)dev_wifi_get_thread_node_param();
    wifi_device_info_t *device_info = dev_wifi_get_device();
    s32 len = 0;
    
    ret = wifi_data_recv_and_parse();
    if(0 == ret)
    {
        if(thread_node_param->m_timeover != 0)
        {
            thread_node_param->m_timeover--;
            if(0 == thread_node_param->m_timeover)
            {
                //超时处理
                if(g_wifi_parse.m_special&WIFI_SPECIAL_BIT0)
                {
                    TRACE_INFO("recv timerout");
                    g_wifi_parse.m_special &= (~WIFI_SPECIAL_BIT0);
                    dev_wifi_thread_set_result(WIFI_AT_RESULT_TIMEROUT);

                    //恢复线程参数
                    wifi_recovery_thread_param();
                }
                else if(thread_node_param->m_func != NULL)
                {
                    TRACE_INFO("timerout:%d,%d", thread_node_param->m_task_id,g_wifi_parse.m_special);
                    thread_node_param->m_result = WIFI_AT_RESULT_TIMEROUT;
                    thread_node_param->m_func(thread_node_param->m_result, g_wifi_parse.m_rbuf, g_wifi_parse.m_rcnt);
                }

                dev_wifi_init_parse_data();
            }
        }
    }
    else
    {
        TRACE_INFO("<< %s", g_wifi_parse.m_rbuf);
        
        ret = wifi_data_handle();
        
        TRACE_DBG("m_result=%d, ret:%d", thread_node_param->m_result, ret);
        switch(ret)
        {
            case 1:
            case 3:
                if(thread_node_param->m_func)
                {
                    thread_node_param->m_func(thread_node_param->m_result, g_wifi_parse.m_rbuf, g_wifi_parse.m_rcnt);
                }
                else
                {
                    TRACE_ERR("have nothing to do func");
                }
                break;
            case 2: //主动上报，已在wifi_data_handle里处理
                break;
        }

        if(0 == g_wifi_parse.m_special)
        {
            dev_wifi_init_parse_data();
        }
        else
        {
            ret = g_wifi_parse.m_special;
            dev_wifi_init_parse_data();
            g_wifi_parse.m_special = ret;
        }
    }
}
#endif
