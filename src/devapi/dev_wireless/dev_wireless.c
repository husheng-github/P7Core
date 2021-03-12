#include "devglobal.h"
#include "ddi.h"
#include "ddi_com.h"
#include "ddi_gprs.h"
#include "drv_wireless_io.h"
#include "dev_wireless_thread.h"
#include "module.h"
#include "dev_wireless.h"
#include "module_FIBOCOM.h"
#include "wireless_control_strategy.h"
#include "wireless_data_handle.h"

static wireless_device_info_t g_wireless_device_info;
static s32 g_wireless_init_flag = FALSE;
static u32 g_get_csq_last_ticks = 0;

s32 g_wire_exgpio_flg = 0;   //初始化为0
const u8 wire_exgpio_loginid_tab[WIRE_EXGPIO_MAX] = {3, 1, 2, 0};

#if(WIRELESS_EXIST == 1)

wireless_device_info_t *dev_wireless_get_device(void)
{
    return &g_wireless_device_info;
}

/**
 * @brief 从文件获取拨号参数
 * @param[in] 无
 * @retval  DDI_OK 从文件获取成功
 * @retval  DDI_ERR 使用默认apn
 */
static s32 wireless_get_apn_from_file(void)
{
    s32 fd = vfs_open(WIRELESS_APN_FILE, "r");
    s32 ret = DDI_ERR;
    wireless_device_info_t *device_info = dev_wireless_get_device();
    s8 buff[128] = {0};
    s8 dst[3][SPLIT_PER_MAX] = {0};

    do{
        if(fd < 0)
        {
            break;
        }

        memset(buff, 0, sizeof(buff));
        ret = vfs_read(fd, buff, sizeof(buff));
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

        snprintf(device_info->m_apn, sizeof(device_info->m_apn), "%s", dst[0]);
        snprintf(device_info->m_username, sizeof(device_info->m_username), "%s", dst[1]);
        snprintf(device_info->m_password, sizeof(device_info->m_password), "%s", dst[2]);
        ret = DDI_OK;
    }while(0);

    if(fd >= 0)
    {
        vfs_close(fd);
    }

    if(DDI_ERR == ret)
    {
        snprintf(device_info->m_apn, sizeof(device_info->m_apn), "cmnet");
    }

    return ret;
}

/**
 * @brief 查询socket套接字在模块内部是否是创建中
 * @retval TRUE 存在
 * @retval FALSE 不存在
 * @retval < 0 执行出错
 */
static s32 wireless_socket_check_is_exist(s32 socket_id, AT_API_GET_TYPE_E at_api_get_type)
{
    s32 j = 0;
    s32 avlid_sockid_list[WIRELESS_SOCKET_MAX] = {0};   //模块端可用的socket id
    s32 ret = DDI_ERR;
    s32 exist_flag = FALSE;    //socketid是否在模块端可用清单里  TRUE 存在   ,FALSE不存在

    ret = MIL_get_valid_socketid_list(avlid_sockid_list, WIRELESS_SOCKET_MAX, AT_API_BLOCK);
    if(ret != DDI_OK)
    {
        TRACE_ERR("can't get avalid socket id list from module:%d", ret);
        return DDI_ERR;
    }

    exist_flag = TRUE;
    //判断此序号socket_id是否在模块端可用清单里
    for(j=0; j<WIRELESS_SOCKET_MAX; j++)
    {
        if(avlid_sockid_list[j] > 0 && avlid_sockid_list[j] == socket_id)
        {
            TRACE_INFO("socket id:%d exist in module avaliable list", socket_id);
            exist_flag = FALSE;
            break;
        }
    }
    
    return exist_flag;  
}

/**
 * @brief 通过应用层的socket sn找到socket资源
 * @retval 对应socket sn的资源
 */
static wireless_socket_info_t *wireless_get_socket_info_by_index(s32 index)
{
    if(index < 0 || index >= WIRELESS_SOCKET_MAX)
    {
        return NULL;
    }
    
    return g_wireless_device_info.m_socket_info[index];
}

/**
 * @brief 释放socket套接字资源
 * @retval  无
 */
static void wireless_socket_info_destroy(wireless_socket_info_t* thiz)
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
s32 dev_wireless_set_socket_status(s32 socket_id, SOCKET_STATUS_E socket_status)
{
    s32 i = 0;
    s32 ret = DDI_ERR;
    
    for(i = 0; i < WIRELESS_SOCKET_MAX; i++)
    {
        if(0xff == socket_id)     //将所有套接字设置为断开，在收到协议栈关闭时调用
        {
            if(g_wireless_device_info.m_socket_info[i])
            {
                if(GPRS_STATUS_CONNECTING == g_wireless_device_info.m_socket_info[i]->m_status
                    || GPRS_STATUS_CONNECTED == g_wireless_device_info.m_socket_info[i]->m_status)
                {
                    TRACE_INFO("socketid:%d, change status to:%d", 
                                                g_wireless_device_info.m_socket_info[i]->m_socket_id, 
                                                socket_status);
                    g_wireless_device_info.m_socket_info[i]->m_status = socket_status;
                    ret = DDI_OK;
                }
            }
        }
        else
        {
            if(g_wireless_device_info.m_socket_info[i] 
                && g_wireless_device_info.m_socket_info[i]->m_socket_id == socket_id)
            {
                TRACE_INFO("socketid:%d, change status to:%d", socket_id, socket_status);
                g_wireless_device_info.m_socket_info[i]->m_status = socket_status;
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
s32 dev_wireless_socket_info_destroy_by_id(s32 socket_id)
{
    s32 i = 0;
    s32 ret = DDI_ERR;
    
    for(i = 0; i < WIRELESS_SOCKET_MAX; i++)
    {
        if(g_wireless_device_info.m_socket_info[i] 
            && g_wireless_device_info.m_socket_info[i]->m_socket_id == socket_id)
        {
            wireless_socket_info_destroy(g_wireless_device_info.m_socket_info[i]);
            g_wireless_device_info.m_socket_info[i] = NULL;
            ret = DDI_OK;
            break;
        }
    }
    
    return ret;
}

/**
 * @brief 创建socket套接字资源
 * @retval  socket套接字资源
 */
static wireless_socket_info_t *wireless_socket_info_create(void)
{
    wireless_socket_info_t* thiz = (wireless_socket_info_t*)k_malloc(sizeof(wireless_socket_info_t));
    s32 ret = DDI_ERR;
    
    if(thiz == NULL)
    {
        TRACE_ERR("gprs_socket_info_create failed");
        return NULL;
    }

    memset(thiz, 0, sizeof(wireless_socket_info_t));
    ret = dev_circlequeue_init(&(thiz->m_read_buffer), WIRELESS_SOCKET_RCV_BUFLEN);
    if(ret < 0)
    {
        TRACE_ERR("gprs_socket_info_create failed for read buffer");
        wireless_socket_info_destroy(thiz);
        return NULL;
    }

    memset(thiz->m_serveraddr, 0, sizeof(thiz->m_serveraddr));
    thiz->m_serverport = 0;
    thiz->total_receive = 0;
    thiz->m_status = GPRS_STATUS_NONE;
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
static s32 wireless_get_available_socket_info_index(u8 *param, u16 port)
{
    s32 i = 0, j = 0;
    s32 avlid_sockid_list[WIRELESS_SOCKET_MAX] = {0};   //模块端可用的socket id
    s32 ret = DDI_ERR;
    s32 exist_flag = FALSE;    //socketid是否在模块端可用清单里  TRUE 存在   ,FALSE不存在
    s32 flag = 0;              //1:仅释放socket资源   2:关闭模块端socket和socket资源

    ret = MIL_get_valid_socketid_list(avlid_sockid_list, WIRELESS_SOCKET_MAX, AT_API_BLOCK);
    if(ret != DDI_OK)
    {
        TRACE_ERR("can't get avalid socket id list from module:%d", ret);
        return -1;
    }

    //优先使用空闲的socket资源
    for(i = 0; i < WIRELESS_SOCKET_MAX; i++)
    {
        if(!g_wireless_device_info.m_socket_info[i])
        {
            //判断此序号socket_id是否在模块端可用清单里
            for(j=0; j<WIRELESS_SOCKET_MAX; j++)
            {
                TRACE_DBG("j[%d]:%d,%d", j, avlid_sockid_list[j], i+1);
                if(avlid_sockid_list[j] > 0 && avlid_sockid_list[j] == i+1)
                {
                    TRACE_ERR("socketinfo and id:%d is free", i+1);
                    flag = 1;
                    break;
                }
            }

            if(1 == flag)
            {
                break;
            }
        }
    }

    if(0 == flag)
    {
        for(i = 0; i < WIRELESS_SOCKET_MAX; i++)
        {
            exist_flag = FALSE;
            //判断此序号socket_id是否在模块端可用清单里
            for(j=0; j<WIRELESS_SOCKET_MAX; j++)
            {
                TRACE_DBG("j[%d]:%d,%d", j, avlid_sockid_list[j], i+1);
                if(avlid_sockid_list[j] > 0 && avlid_sockid_list[j] == i+1)
                {
                    exist_flag = TRUE;
                    break;
                }
            }

            if(TRUE == exist_flag)   //在模块端可用id里，说明本地的socketid可释放
            {
                //socket buff还有数据应用未读走，不能直接用此socketid
                if(g_wireless_device_info.m_socket_info[i] && !dev_circlequeue_isempty(&(g_wireless_device_info.m_socket_info[i]->m_read_buffer)))
                {
                    TRACE_INFO("socket %d have data, then try next", i+1);
                    continue;
                }
                else
                {
                    TRACE_ERR("socket id:%d don't exit in module, then destory", i+1);
                    flag = 1;
                    break;
                }
            }
            else
            {
                if(!g_wireless_device_info.m_socket_info[i])
                {
                    TRACE_ERR("socketinfo:%d is NULL in local, create it", i+1);
                    flag = 2;
                    break;
                }
                else
                {
                    if(g_wireless_device_info.m_socket_info[i]->m_socket_id <= 0)  //本地socketid无效
                    {
                        TRACE_ERR("socketid:%d is not exist in local, destory it", i+1);
                        flag = 2;
                        break;
                    }
                    else
                    {
                        if(0 == g_wireless_device_info.m_socket_info[i]->m_status) //状态空闲
                        {
                            TRACE_ERR("socketid:%d state is free, use it", i+1);
                            flag = 2;
                            break;
                        }

                        if(0 == strcmp(param, g_wireless_device_info.m_socket_info[i]->m_serveraddr) 
                            && port == g_wireless_device_info.m_socket_info[i]->m_serverport)
                        {
                            TRACE_ERR("System exist the same socket, close before:%s, %d", 
                                            g_wireless_device_info.m_socket_info[i]->m_serveraddr,
                                            g_wireless_device_info.m_socket_info[i]->m_serverport);
                            flag = 2;
                            break;
                        }
                    }
                }
            }
        }
    }
    
    if(i < WIRELESS_SOCKET_MAX)
    {
        if(2 == flag)
        {
            if(DDI_OK != MIL_socket_close(i+1, AT_API_BLOCK))  //关闭socket
            {
                TRACE_ERR("close socketid:%d failed", i+1);
                return -1;
            }
        }
        
        if(1 == flag || 2 == flag)
        {
            if(!g_wireless_device_info.m_socket_info[i])
            {
                g_wireless_device_info.m_socket_info[i] = wireless_socket_info_create();
                if(g_wireless_device_info.m_socket_info[i])
                {
                    g_wireless_device_info.m_socket_info[i]->m_socket_id = i + 1;
                    return i;
                }
            }
            else
            {
                dev_circlequeue_clear(&(g_wireless_device_info.m_socket_info[i]->m_read_buffer));
                g_wireless_device_info.m_socket_info[i]->total_receive = 0;
                g_wireless_device_info.m_socket_info[i]->m_status = GPRS_STATUS_NONE;
                g_wireless_device_info.m_socket_info[i]->m_socket_id = i+1;
                memset(g_wireless_device_info.m_socket_info[i]->m_serveraddr, 0, sizeof(g_wireless_device_info.m_socket_info[i]->m_serveraddr));
                g_wireless_device_info.m_socket_info[i]->m_serverport = 0;
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
s32 dev_wireless_send_at_cmd(s8 *cmd, AT_API_GET_TYPE_E at_api_get_type, s32 end_flag)
{
    s32 ret = DDI_ERR;
    wireless_device_info_t *device_info = dev_wireless_get_device();
    s32 len = 0;

    do{
        //外部接口主动获取时，线程任务不执行，防止发送部分数据时被中断打断，然后在中断里也发命令了
        if(AT_API_BLOCK == at_api_get_type)
        {
            dev_wireless_thread_run_switch(FALSE);
        }

        if(strlen(cmd) > 4)
        {
            TRACE_INFO(">> %s", cmd);
            if(device_info->m_dial_log_buff)
            {
                len = strlen(device_info->m_dial_log_buff);
                snprintf(device_info->m_dial_log_buff+len, WIRELESS_DIAL_LOG_SIZE-len, ">> %s-%d\r\n", cmd, dev_user_gettimeID());
            }
        }
        else if(2 == strlen(cmd) && 0 == strcmp(cmd, WIRE_AT_CMD))
        {
            TRACE_DBG(">> %s", cmd);
        }
        
        ret = dev_com_write(WIRELESS_PORT, cmd, strlen(cmd));
        if(ret < 0)
        {
            TRACE_ERR("ret:%d", ret);
            ret = DDI_ETRANSPORT;
            break;
        }

        if(end_flag)
        {
            ret = dev_com_write(WIRELESS_PORT, AT_CMD_RES_END, strlen(AT_CMD_RES_END));
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
        dev_wireless_thread_run_switch(TRUE);
    }
    return ret;
}

s32 dev_wireless_init(void)
{
    s32 i = 0;

    if(!g_wireless_init_flag)
    {
        memset(&g_wireless_device_info, 0, sizeof(g_wireless_device_info));

        g_wireless_device_info.m_dial_log_buff = NULL;
        //初始化基站内存
        for(i=0; i<WIRELESS_CELL_NUM; i++)
        {
            g_wireless_device_info.m_cell_info[i] = (wireless_cell_info_t *)k_malloc(sizeof(wireless_cell_info_t));
            if(g_wireless_device_info.m_cell_info[i])
            {
                memset(g_wireless_device_info.m_cell_info[i], 0, sizeof(wireless_cell_info_t));
            }
        }
        
        //dev_com_open(WIRELESS_PORT, 115200, 8, 'n', 0, 0);
		//dev_com_close(WIRELESS_PORT);
        wireless_get_apn_from_file();    //从文件读取apn
        dev_wireless_set_control_strategy_step(WS_MODULE_POWER_ON);
        dev_wireless_thread_node_param_init();
        dev_wireless_init_parse_data();
        dev_wireless_thread_run_switch(TRUE);

#if(WIRELESS_USE_EXTERANL_AMP == TRUE)
        #ifdef WIRELESS_USE_SAVEEDVOL
        wireless_audio_get_vol_from_file();
        #endif
        
        g_wireless_device_info.m_audio_info.m_mp3_play_done = TRUE;
        g_wireless_device_info.m_audio_info.m_tts_play_done = TRUE;
#endif
        g_wireless_init_flag = TRUE;
    }
    
    return DDI_OK;
}

s32 dev_wireless_open(void)
{
    WIRE_CONTROL_FAIL_REASON_E control_fail_reason = dev_wireless_get_status(WS_GET_REG_NETWORK_FAIL_REASON);
    WIRE_CONTROL_STRATEGY_STEP_E control_step = dev_wireless_get_status(WS_GET_WIRELESS_STATE);
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();
    s32 ret = DDI_ERR;
    s8 local_ip[32] = {0};
    static u32 last_ticks = 0;

    //检查错误原因
    switch(control_fail_reason)
    {
        case WS_NO_INSERT_SIM:
            ret = DDI_ESIM;
            goto _out;
            break;

        case WS_UNKNOW_WIRELESS_MODULE:
        case WS_NO_WIRELESS_MODULE:
            ret = DDI_ENODEV;
            goto _out;
            break;
    }

    switch(control_step)
    {
        case WS_ACTIVE_PDP_SUCCESS:  //激活pdp成功
            if(dev_user_querrytimer(last_ticks, 1*1000*60))      //三分钟查一次ip
            {
                last_ticks = dev_user_gettimeID();
                memset(local_ip, 0, sizeof(local_ip));

                //查询一次ip在不在, task繁忙时直接返回正在注网
                if(!dev_wireless_thread_is_free())
                {
                    TRACE_ERR("now is hanle task:%d, step:%d", thread_node_param->m_task_id, thread_node_param->m_step);
                    ret = DDI_EOPERATION;
                    break;
                }
                
                dev_wireless_get_localip(local_ip, sizeof(local_ip));
                if(strlen(local_ip))
                {
                    ret = DDI_OK;
                }
                else
                {
                    TRACE_ERR("ip lost");
                    ret = DDI_EOPERATION;
                }
            }
            else
            {
                ret = DDI_OK;
            }
            break;

        default:   //正在注网
            ret = DDI_EOPERATION;
            break;
    }

_out:    
    return ret;
}

s32 dev_wireless_close(void)
{
    s32 sn = 0;
    
    for(sn=0; sn<WIRELESS_SOCKET_MAX; sn++)
    {
        if(g_wireless_device_info.m_socket_info[sn])
        {
            dev_wireless_socket_close(sn|WIRELESS_ID_MASK);
        }
    }
}

void dev_wireless_poweroff(void)
{
    TRACE_DBG("enter");

    if(dev_wireless_module_is_upgrading())
    {
        TRACE_INFO("upgrading now");
        return ;
    }
    
    dev_wireless_close();      //释放所有socket资源
    
    //关闭中断任务，并清除串口任务
    dev_wireless_thread_run_switch(FALSE);
    dev_wireless_thread_set_result(WIRE_AT_RESULT_OK);
    dev_com_flush(WIRELESS_PORT);
    
    MIL_power_off(AT_API_BLOCK);
    TRACE_DBG("out");
}

s32 dev_wireless_set_apn(u8* apn, u8* user, u8* psw)
{
    s32 ret = DDI_ERR;
    
    do{
        if(NULL == apn)
        {
            ret = DDI_EINVAL;
            break;
        }

        ret = MIL_set_apn(apn, user, psw, AT_API_BLOCK);
    }while(0);
        
    return ret;
}

/**
 * @brief 释放ip地址
 * @param[in] 无
 * @retval  DDI_OK
 * @retval  DDI_ERR
 * @retval  DDI_ETIMEOUT
 */
s32 dev_wireless_close_pdp(void)
{
    TRACE_INFO("app want to inactive network");
    
    return MIL_inactive_pdp(AT_API_BLOCK);
}

/**
 * @brief 获取网络状态或者注册网络失败的原因
 * @param[in] wireless_status_type 0获取网络状态  1获取注册网络失败的原因 
 * @retval  注册失败原因，详见WIRE_REG_NET_FAIL_REASON_E
 * @retval  网络状态，详见WIRELESS_STATUS_E
 */
s32 dev_wireless_get_status(wireless_status_type_e wireless_status_type)
{
    s32 ret = DDI_ERR;

    switch(wireless_status_type)
    {
        case WS_GET_WIRELESS_STATE:
            ret = g_wireless_device_info.m_control_step;
            break;

        case WS_GET_REG_NETWORK_FAIL_REASON:
            ret = g_wireless_device_info.m_control_fail_reason;
            break;
    }

    return ret;
}

s32 dev_wireless_get_simcard_status(void)
{
    s32 ret = DDI_ERR;

    switch(g_wireless_device_info.m_simcard_status)
    {
        case WITH_SIMCARD:
            ret = DDI_OK;
            break;
        
        case DETECTING_SIMCARD:
            ret = DDI_EBUSY;
            break;

        case NOWITH_SIMCARD:
            break;
    }
    
    return ret;
}

s32 dev_wireless_get_localip(u8 *ip_buf, u32 buf_size)
{
    s32 ret = DDI_ERR;

    do{
        if(!dev_wireless_thread_is_free())
        {
            if(strlen(g_wireless_device_info.m_local_ip))
            {
                snprintf(ip_buf, buf_size, "%s", g_wireless_device_info.m_local_ip);
                ret = DDI_OK;
                break;
            }
            else
            {
                ret = DDI_EBUSY;
                break;
            }
        }

        ret = MIL_get_local_ip(ip_buf, buf_size, AT_API_BLOCK);
    }while(0);
   
    return ret;
}

s32 dev_wireless_socket_create(u8 type, u8 mode, u8 *param, u16 port)
{
    s32 ret = DDI_ERR;
    s32 sn = 0;
    s8 buff[128] = {0};
    wireless_socket_info_t *socket_info = NULL;
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();
    
    do{
        if(SOCKET_TYPE_UDP == type || NULL == param || 0 == port)
        {
            ret = DDI_EINVAL;
            break;
        }

        if(!dev_wireless_thread_is_free())
        {
            TRACE_ERR("now is hanle task:%d, step:%d", thread_node_param->m_task_id, thread_node_param->m_step);
            ret = DDI_EBUSY;
            break;
        }
        
        sn = wireless_get_available_socket_info_index(param, port);
        if(sn < 0)
        {
            TRACE_ERR("dev_gprs_socket_request failed:%d", ret);
            ret = DDI_EBUSY;
            break;
        }

        socket_info = g_wireless_device_info.m_socket_info[sn];
        snprintf(socket_info->m_serveraddr, sizeof(socket_info->m_serveraddr), "%s", param);
        socket_info->m_serverport = port;

        socket_info->m_status = GPRS_STATUS_CONNECTING;
        ret = MIL_socket_create(socket_info->m_socket_id, type, param, port, AT_API_BLOCK);
        if(DDI_OK == ret)
        {
            ret = sn|WIRELESS_ID_MASK;
        }

        TRACE_DBG("socket_info->m_id:%d, socket_info->m_status:%d, ret:%08x", socket_info->m_socket_id, socket_info->m_status, ret);
        if(ret < 0)
        {
            wireless_socket_info_destroy(socket_info);
            g_wireless_device_info.m_socket_info[sn] = NULL;
        }
    }while(0);

    return ret;
}


s32 dev_wireless_socket_get_status(s32 socketid)
{
    s32 ret;
    u32 sn;
    wireless_socket_info_t* socket_info;

    sn = socketid&(~WIRELESS_ID_MASK); 
    socket_info = wireless_get_socket_info_by_index(sn);
    if(socket_info == NULL)
    {
        TRACE_ERR("cannot find socket info for socketid(%d,%d) Err", socketid, sn);
        return DDI_EINVAL;
    }

    ret = socket_info->m_status;
    return ret;
}

s32 dev_wireless_socket_send(s32 socketid, const u8 *wbuf, u32 wlen)
{
    s32 ret;
    u32 sn;
    wireless_socket_info_t *socket_info;
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();
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

        sn = socketid&(~WIRELESS_ID_MASK);
        socket_info = wireless_get_socket_info_by_index(sn);
        if(socket_info == NULL)
        {
            TRACE_ERR("cannot find socket info for socketid(%d) Err", socketid);
            ret = DDI_EINVAL;
            break;
        }

        if(WS_ACTIVE_PDP_SUCCESS != dev_wireless_get_status(WS_GET_WIRELESS_STATE))
        {
            TRACE_ERR("network is not active");
            ret = DDI_EOPERATION;
            break;
        }

        if(socket_info->m_status != GPRS_STATUS_CONNECTED)
        {
            TRACE_ERR("socket has disconnected");
            ret = DDI_ERR;
            break;
        }
        
        if(!dev_wireless_thread_is_free())
        {
            TRACE_ERR("now is hanle task:%d, step:%d", thread_node_param->m_task_id, thread_node_param->m_step);
            ret = DDI_EBUSY;
            break;
        }

        ret = MIL_socket_send(socket_info->m_socket_id, wbuf, wlen, AT_API_BLOCK);
    }while(0);
        
    return ret;
}


s32 dev_wireless_socket_recv(s32 socketid, u8 *rbuf, u32 rlen)
{
    s32 ret = DEVSTATUS_ERR_PARAM_ERR;
    u32 sn;
    s32 recv_ret = 0;
    wireless_socket_info_t *socket_info;

    do{
        
        ddi_watchdog_feed();  //ssl 接收的时候
        sn = socketid&(~WIRELESS_ID_MASK); 
        socket_info = wireless_get_socket_info_by_index(sn);
        if(socket_info == NULL)
        {
            TRACE_DBG("cannot find socket info for socketid(%d)", socketid);
            ret = DDI_EINVAL;
            break;
        }

        ret = dev_circlequeue_read(&(socket_info->m_read_buffer), rbuf, rlen);

        //Add by xiaohonghui 2020.6.19   链接断开并且没有数据时，给应用层返回已断开
        if(socket_info->m_status != GPRS_STATUS_CONNECTED  && 0 == ret)
        {
            TRACE_ERR("socket has disconnected");
            ret = DDI_ERR;
            break;
        }
    }while(0);
        
    return ret;
}

s32 dev_wireless_socket_close(s32 socketid)
{
    s32 ret = DDI_ERR;
    u32 i = 0;
    u32 sn;
    wireless_socket_info_t *socket_info;
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();

    TRACE_DBG("enter");
    do{
        sn = socketid&(~WIRELESS_ID_MASK); 
        socket_info = wireless_get_socket_info_by_index(sn);
        if(socket_info == NULL)
        {
            TRACE_DBG("cannot find socket info for socketid(%d)", socketid);
            ret = DDI_EINVAL;
            break;
        }

        //如果要关闭套接字正在接收数据，直接将接收数据任务取消
        if(dev_wireless_get_current_socket_id() == socket_info->m_socket_id && WS_TASK_SOCKET_RECVDATA == thread_node_param->m_task_id)
        {
            TRACE_DBG("release recv task when there is socket close task insert");
            dev_wireless_thread_set_result(WIRE_AT_RESULT_OK);
        }
        
        if(!dev_wireless_thread_is_free())
        {
            ret = DDI_EBUSY;
            break;
        }

        //记录要关闭的套接字，可避免在关闭套接字过程中收到数据
        g_wireless_device_info.m_cur_close_socket_id = socket_info->m_socket_id;
        
        ret = wireless_socket_check_is_exist(socket_info->m_socket_id, AT_API_BLOCK);
        if(TRUE == ret)
        {
            ret = MIL_socket_close(socket_info->m_socket_id, AT_API_BLOCK);
            if(DDI_OK == ret)
            {
                TRACE_INFO("close socketid:%d success", socket_info->m_socket_id);
            }
            else
            {
                TRACE_INFO("close socketid:%d fail ret:%d", socket_info->m_socket_id, ret);
            }
        }
        else if(FALSE == ret)
        {
            ret = DDI_OK;
        }

        if(DDI_OK == ret)
        {
            TRACE_INFO("destory socketid:%d info", socket_info->m_socket_id);
            dev_wireless_socket_info_destroy_by_id(socket_info->m_socket_id);
        }
    }while(0);

    g_wireless_device_info.m_cur_close_socket_id = -1;
    TRACE_DBG("socket_close = %d",ret);
    return ret;
}

/**
 * @brief 无线模块挂掉自己重启的处理函数
 * @param[in] 无
 * @retval  无
 */
void dev_wireless_module_reboot_handle(void)
{
    s32 i = 0;
    
    for(i = 0; i < WIRELESS_SOCKET_MAX; i++)
    {
        if(g_wireless_device_info.m_socket_info[i])
        {
            TRACE_INFO("destroy socket info");
            wireless_socket_info_destroy(g_wireless_device_info.m_socket_info[i]);
            g_wireless_device_info.m_socket_info[i] = NULL;
        }
    }

    memset(&g_wireless_device_info.m_audio_info, 0, sizeof(g_wireless_device_info.m_audio_info));
    dev_wireless_set_control_strategy_step(WS_MODULES_INIT);
}

s32 dev_wireless_get_csq(u8 *prssi, u16 *prxfull)
{
    s32 ret = DDI_ERR;
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();
    s32 step = 0;

    do{
        if(NULL == prssi || NULL == prxfull)
        {
            ret = DDI_EINVAL;
            break;
        }

        step = dev_wireless_get_status(WS_GET_WIRELESS_STATE);
        if(step == WS_MODULE_POWER_ON || step == WS_MODULE_POWER_OFF)   //add by xiaohonghui  上电或下电流程不发命令
        {
            
        }
        else
        {
            if(dev_wireless_thread_is_free())
            {
                if(dev_user_querrytimer(g_get_csq_last_ticks, WIRELESS_GET_CSQ_INTERVAL))
                {
                    ret = MIL_get_csq(prssi, prxfull, AT_API_BLOCK);
                    g_get_csq_last_ticks = dev_user_gettimeID();
                }
            }
            else
            {
                TRACE_DBG("task:%d, step:%d", thread_node_param->m_task_id, thread_node_param->m_step);
            }
        }

        *prssi = g_wireless_device_info.m_csq;
        *prxfull = g_wireless_device_info.m_bear_rate;
        ret = DDI_OK;
    }while(0);
    
    return ret;
}

s32 dev_wireless_get_ccid(u8 *ccid_buf, u32 buf_size)
{
    s32 ret = DDI_ERR;
    
    do{
        if(NULL == ccid_buf || 0 == buf_size)
        {
            ret = DDI_EINVAL;
            break;
        }

        ret = MIL_get_ccid(ccid_buf, buf_size, AT_API_BLOCK);
    }while(0);
    
    return ret;
}

s32 dev_wireless_get_imsi(u8 *imsi_buf, u32 buf_size)
{
    s32 ret = DDI_ERR;
    
    do{
        if(NULL == imsi_buf || 0 == buf_size)
        {
            ret = DDI_EINVAL;
            break;
        }

        ret = MIL_get_imsi(imsi_buf, buf_size, AT_API_BLOCK);
    }while(0);
    
    return ret;
}

s32 dev_wireless_get_imei(u8 *imei_buf, u32 buf_size)
{
    s32 ret = DDI_ERR;
    
    do{
        if(NULL == imei_buf || 0 == buf_size)
        {
            ret = DDI_EINVAL;
            break;
        }

        ret = MIL_get_imei(imei_buf, buf_size, AT_API_BLOCK);
    }while(0);
    
    return ret;
}

s32 dev_wireless_get_basestation_info(strBasestationInfo *basestation_info)
{
    s32 ret = 0;
    s32 step = 0;

    do{
        if(!basestation_info)
        {
            ret = DDI_EINVAL;
            break;
        }

        if(0 != g_wireless_device_info.m_main_cell_info.lac)
        {
            snprintf(basestation_info->asMcc, sizeof(basestation_info->asMcc), "%d", g_wireless_device_info.m_main_cell_info.mcc);
            snprintf(basestation_info->asMnc, sizeof(basestation_info->asMnc), "%d", g_wireless_device_info.m_main_cell_info.mnc);
            snprintf(basestation_info->asLac, sizeof(basestation_info->asLac), "%d", g_wireless_device_info.m_main_cell_info.lac);
            snprintf(basestation_info->asCi, sizeof(basestation_info->asCi), "%x", g_wireless_device_info.m_main_cell_info.cid);
            ret = DDI_OK;
        }
    }while(0);

    return ret;
}

s32 dev_wireless_cfg_ntp_server(s8 *ntp_addr, s32 ntp_port)
{
    s32 ret = DDI_ERR;
    s32 step = 0;
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();

    do{
        if(NULL == ntp_addr)
        {
            ret = DDI_EINVAL;
            break;
        }

        step = dev_wireless_get_status(WS_GET_WIRELESS_STATE);
        if(step == WS_MODULE_POWER_ON || step == WS_MODULE_POWER_OFF)
        {
            ret = DDI_ERR;
            break;
        }
        
        if(!dev_wireless_thread_is_free())
        {
            TRACE_ERR("task:%d, step:%d", thread_node_param->m_task_id, thread_node_param->m_step);
            ret = DDI_EBUSY;
            break;
        }

        ret = MIL_cfg_ntp_server(ntp_addr, ntp_port, AT_API_BLOCK);
    }while(0);
    
    return ret;
}

s32 dev_wireless_qry_rtc(s8 *rtc_buff, s32 rtc_buff_len)
{
    s32 ret = DDI_ERR;
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();

    do{
        if(NULL == rtc_buff || 0 == rtc_buff_len)
        {
            ret = DDI_EINVAL;
            break;
        }
        
        if(!dev_wireless_thread_is_free())
        {
            TRACE_DBG("task:%d, step:%d", thread_node_param->m_task_id, thread_node_param->m_step);
            ret = DDI_EBUSY;
            break;
        }

        ret = MIL_qry_rtc(rtc_buff, rtc_buff_len, AT_API_BLOCK);
    }while(0);
    
    return ret;
}

s32 dev_wireless_close_airplane(void)
{
    //关闭中断任务，并清除串口任务
    dev_wireless_thread_run_switch(FALSE);
    dev_wireless_thread_set_result(WIRE_AT_RESULT_OK);
    dev_com_flush(WIRELESS_PORT);
    
    return MIL_close_airplane(AT_API_BLOCK);
}


s32 dev_wireless_open_airplane(void)
{
    dev_wireless_close();      //释放所有socket资源
    
    //关闭中断任务，并清除串口任务
    dev_wireless_thread_run_switch(FALSE);
    dev_wireless_thread_set_result(WIRE_AT_RESULT_OK);
    dev_com_flush(WIRELESS_PORT);
    
    return MIL_open_airplane(AT_API_BLOCK);
}

s32 dev_wireless_get_modulever(u8 *modulever_buf, u32 buf_size)
{
    s32 ret = DDI_ERR;
    
    do{
        if(NULL == modulever_buf || 0 == buf_size)
        {
            ret = DDI_EINVAL;
            break;
        }

        snprintf(modulever_buf, buf_size, "%s", g_wireless_device_info.m_module_ver);
        ret = DDI_OK;
    }while(0);
    
    return ret;
}

/**
 * @brief 获取拨号结果  拨号指令是先返回ok，如果失败，后面会返回NO ANSWER
 * @param[in] 
 * @retval DDI_OK 需通过dev_wireless_get_telephone_dial_cmd_result查询命令是否成功
 * @retval DDI_EOPERATION 拨号失败---正在注网
 * @retval DDI_ERR 
 * @retval DDI_ETIMEOUT
 */
s32 dev_wireless_telephone_dial(u8 *num)
{
    s32 ret = DDI_ERR;
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();

    do{
        if(NULL == num)
        {
            ret = DDI_EINVAL;
            break;
        }
        
        //关闭中断任务，并清除串口任务
        dev_wireless_thread_run_switch(FALSE);
        dev_wireless_strategy_run_switch(FALSE);
        dev_wireless_thread_set_result(WIRE_AT_RESULT_OK);
        dev_com_flush(WIRELESS_PORT);

        ret = MIL_telephone_dial(num, AT_API_BLOCK);
    }while(0);
    
    return ret;
}

/**
 * @brief 获取拨号结果  注意：一定要dev_wireless_telephone_dial返回DDI_OK后再调这个接口查询
 * @param[in] 
 * @retval DDI_OK  返回ok后无其它数据返回，认为拨号成功
 * @retval DDI_ETIMEOUT  返回NO ANSWER
 * @retval DDI_ERR 拨号结果还未返回
 */
s32 dev_wireless_get_telephone_dial_cmd_result(void)
{
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();
    s32 ret = DDI_ERR;
    
    if(dev_wireless_thread_is_free())
    {
        switch(thread_node_param->m_result)
        {
            case WIRE_AT_RESULT_DAIL_NOANSWER:
                ret = DDI_ETIMEOUT;
                break;

            case WIRE_AT_RESULT_TIMEROUT:
                ret = DDI_OK;
                break;
        }
    }

    return ret;
}

s32 dev_wireless_telephone_hangup(void)
{
    s32 ret = DDI_ERR;
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();

    do{
        //关闭中断任务，并清除串口任务
        dev_wireless_thread_run_switch(FALSE);
        dev_wireless_strategy_run_switch(FALSE);
        dev_wireless_thread_set_result(WIRE_AT_RESULT_OK);
        dev_com_flush(WIRELESS_PORT);

        ret = MIL_telephone_hangup(AT_API_BLOCK);
    }while(0);

    dev_wireless_strategy_run_switch(TRUE);
    
    return ret;
}
s32 dev_wireless_telephone_answer(void)
{
    return DDI_ENODEV;
}

/**
 * @brief  获取模块APP版本
 * @param[in]
 * @param[in] 
 * @retval  DDI_OK 成功
 * @retval  DDI_ERR 失败
 */
s32 dev_wireless_get_appver(u8 *ver_buf, u32 buf_size)
{
    s32 ret = DDI_ERR;
    wireless_device_info_t *device_info = dev_wireless_get_device();

    do{
        if(NULL == ver_buf || 0 == buf_size)
        {
            ret = DDI_EINVAL;
            break;
        }
        
        snprintf(ver_buf, buf_size, "%s", device_info->m_module_app_ver);
        ret = DDI_OK;
    }while(0);

    return ret;
}
/**
 * @brief 设置SSL的模式
 * @param[in]
 * @param[in] 
 * @retval  DDI_OK 成功
 * @retval  DDI_ERR 失败
 */
s32 dev_wireless_ssl_set_mode(SSL_AUTHEN_MOD_t mod)
{
    s32 ret = DDI_ERR;
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();

    do{
            
        if(!dev_wireless_thread_is_free())
        {
            TRACE_DBG("task:%d, step:%d", thread_node_param->m_task_id, thread_node_param->m_step);
            ret = DDI_EBUSY;
            break;
        }

        ret = MIL_ssl_set_mod(mod, AT_API_BLOCK);
    }while(0);

    return ret;
}
/**
 * @brief 设置SSL的版本
 * @param[in]
 * @param[in] 
 * @retval  DDI_OK 成功
 * @retval  DDI_ERR 失败
 */
s32 dev_wireless_ssl_setver(u8 ver)
{
    s32 ret = DDI_ERR;
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();

    do{
            
        if(!dev_wireless_thread_is_free())
        {
            TRACE_DBG("task:%d, step:%d", thread_node_param->m_task_id, thread_node_param->m_step);
            ret = DDI_EBUSY;
            break;
        }

        ret = MIL_ssl_setver(ver, AT_API_BLOCK);
    }while(0);

    return ret;
}
/**
 * @brief 设置SSL过滤协议
 * @param[in]
 * @param[in]
 * @retval  DDI_OK 成功
 * @retval  DDI_ERR 失败
 */
s32 dev_wireless_ssl_filte(s32 type)
{
    s32 ret;
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();

    do{
        if((type != 1) && (type != 2))
        {
            TRACE_ERR("type=%d", type);
            return DDI_EINVAL;
        }

        if(!dev_wireless_thread_is_free())
        {
            TRACE_DBG("task:%d, step:%d", thread_node_param->m_task_id, thread_node_param->m_step);
            ret = DDI_EBUSY;
            break;
        }

        ret = MIL_ssl_filte(type, AT_API_BLOCK);
    }while(0);

    return ret;
}

/**
 * @brief 设置SSL的证书
 * @param[in] certtype:0:TRUSTFILE    1:CERTFILE  2. KEYFILE
 * @param[in] 
 * @retval  DDI_OK 成功
 * @retval  DDI_ERR 失败
 */
s32 dev_wireless_ssl_setcert(u8 certtype, u8 *cert, u32 certlen)
{
    
    s32 ret = DDI_ERR;
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();

    do{
            
        if(!dev_wireless_thread_is_free())
        {
            TRACE_DBG("task:%d, step:%d", thread_node_param->m_task_id, thread_node_param->m_step);
            ret = DDI_EBUSY;
            break;
        }

        ret = MIL_ssl_setcert(certtype, cert, certlen, AT_API_BLOCK);
    }while(0);

    return ret;
}
/**
 * @brief openSSL
 * @param[in] certtype:0:TRUSTFILE    1:CERTFILE  2. KEYFILE
 * @param[in] 
 * @retval  DDI_OK 成功
 * @retval  DDI_ERR 失败
 */
s32 dev_wireless_ssl_open(s32 socketid)
{
    s32 ret;
    u32 sn;
    wireless_socket_info_t *socket_info;
//    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();

    do{
        sn = socketid&(~WIRELESS_ID_MASK);
        socket_info = wireless_get_socket_info_by_index(sn);
        if(socket_info == NULL)
        {
            TRACE_ERR("cannot find socket info for socketid(%d) Err", socketid);
            ret = DDI_EINVAL;
            break;
        }
      #if 0
        if(WS_ACTIVE_PDP_SUCCESS != dev_wireless_get_status(WS_GET_WIRELESS_STATE))
        {
            TRACE_ERR("network is not active");
            ret = DDI_EOPERATION;
            break;
        }
      #endif

        if(socket_info->m_status != GPRS_STATUS_CONNECTED)
        {
            TRACE_ERR("socket has disconnected");
            ret = DDI_ERR;
            break;
        }
    
        g_wireless_device_info.m_sslsocket = sn;
        ret = DDI_OK;
    }while(0);
    
    return ret;
}

s32 dev_wireless_ssl_close(void)
{
    s32 ret;

    ret = dev_wireless_socket_close(g_wireless_device_info.m_sslsocket);
    if(DDI_OK==ret)
    {
        g_wireless_device_info.m_sslsocket = 0xFF;
        g_wireless_device_info.m_ssltype = 0;
    }
    return DDI_ENODEV;
}

s32 dev_wireless_ssl_send(const u8 *wbuf, s32 wlen)
{
    return dev_wireless_socket_send(g_wireless_device_info.m_sslsocket, wbuf, wlen);
}

s32 dev_wireless_ssl_recv(u8 *rbuf, s32 rlen)
{
    return dev_wireless_socket_recv(g_wireless_device_info.m_sslsocket, rbuf, rlen);
}

/**
 * @brief 
 * @param[in]
 * @param[in] 
 * @retval  DDI_OK 成功
 * @retval  DDI_ERR 失败
 */
s32 dev_wireless_set_exGPIO(u8 exGPIO, u8 value)
{
    s32 ret = DDI_ERR;
    s32 step = 0;
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();

    do{
        step = dev_wireless_get_status(WS_GET_WIRELESS_STATE);
        if(step == WS_MODULE_POWER_ON || step == WS_MODULE_POWER_OFF)   //add by xiaohonghui  上电或下电流程不发命令
        {
            ret = DDI_ERR;
            break;
        }
            
        if(exGPIO>=WIRE_EXGPIO_MAX)
        {
            ret = DDI_EDATA;
            break;
        }

        if(value)
        {
            g_wire_exgpio_flg |= (1<<exGPIO);
        }
        else
        {
            g_wire_exgpio_flg &= (~(1<<exGPIO));
        }

        ret = MIL_set_exGPIO(wire_exgpio_loginid_tab[exGPIO], (g_wire_exgpio_flg>>exGPIO)&0x01, AT_API_NO_BLOCK);
        ddi_misc_msleep(20);
    }while(0);

    return ret;
}

/**
 * @brief 控制蜂鸣器发声
 * @param[in]
 * @param[in] 
 * @retval  DDI_OK 成功
 * @retval  DDI_ERR 失败
 */
s32 dev_wireless_play_audio(u8 type)
{
    s32 ret;
    s32 step = 0;

    if(dev_wireless_module_is_upgrading())
    {
        TRACE_INFO("upgrading now");
        return DDI_EBUSY;
    }
    
    step = dev_wireless_get_status(WS_GET_WIRELESS_STATE);
    if(step == WS_MODULE_POWER_ON || step == WS_MODULE_POWER_OFF)   //add by xiaohonghui  上电或下电流程不发命令
    {
        return DDI_ERR;
    }
    
    ret = MIL_play_audio(type, AT_API_NO_BLOCK);
    ddi_misc_msleep(20);
    return ret;
}

#ifdef __WIRELESS_SLEEP_LOG
/**
 * @brief 记录低功耗进入和唤醒接口的耗费时间
 * @retval  无
 */
static s32 write_log(s8 *buff)
{
    s32 fd = vfs_open("/mtd0/sleep_log", "a+");

    if(fd > 0)
    {
        vfs_write(fd, buff, strlen(buff));
        vfs_close(fd);
    }

    return 0;
}
#endif

s32 dev_wireless_resume(void)
{
    s32 ret = DDI_ERR;
    s8 buff[128] = {0};
    s32 times = 0;
#ifdef __WIRELESS_SLEEP_LOG
    u32 curr_ticks = dev_user_gettimeID();
#endif

    if(dev_wireless_module_is_upgrading())
    {
        TRACE_INFO("upgrading now");
        return DDI_EBUSY;
    }
    
    TRACE_DBG("enter");
    dev_wireless_strategy_run_switch(TRUE);

    if(WS_MODULE_POWER_ON == dev_wireless_get_status(WS_GET_WIRELESS_STATE))
    {
        dev_wireless_thread_run_switch(TRUE);
        TRACE_DBG("power on state, direct return");
        return DDI_OK;
    }
    
    while(1)
    {
        ret = MIL_resume(AT_API_BLOCK);
        if(ret == DDI_OK)
        {
            memset(buff, 0, sizeof(buff));
            ret = MIL_get_local_ip(buff, sizeof(buff), AT_API_BLOCK);
            break;
        }

        times++;
        if(times > 5)
        {
            TRACE_ERR("resume fail, then power off the module");
            dev_wireless_set_control_strategy_step(WS_MODULE_POWER_OFF);
            break;
        }
        ddi_misc_msleep(100);
    }

    dev_wireless_thread_run_switch(TRUE);   //退出时才让策略任务执行
#ifdef __WIRELESS_SLEEP_LOG
    memset(buff, 0, sizeof(buff));
    snprintf(buff, sizeof(buff), "resume:%d,times:%d\r\n", dev_user_gettimeID()-curr_ticks, times);
    write_log(buff);
#endif
    return ret;
}

/**
 * @brief 模块低功耗接口
 * @retval  DDI_OK 成功
 * @retval  DDI_ERR 有按键干预
 */
s32 dev_wireless_suspend(void)
{
    s32 ret = DDI_ERR;
    u32 cur_ticks = dev_user_gettimeID();
    s32 pdp_state = 0, fail_reason = 0;
    s32 times = 0;
    u32 key = 0;
    s32 flag = 0;
    static s32 check_creg_flag = 0;
    u32 timeout = WIRE_AT_ACTIVE_PDP_TIMEOUT+10*1000;
#ifdef __WIRELESS_SLEEP_LOG
    u32 curr_ticks = dev_user_gettimeID();
    s8 buff[128] = {0};
#endif

    if(dev_wireless_module_is_upgrading())
    {
        TRACE_INFO("upgrading now");
        return DDI_EBUSY;
    }

    //dev_wireless_close();   //释放所有socket资源  //休眠之前不管 socket连接状态
    TRACE_DBG("enter");
    while(1)
    {
        ddi_watchdog_feed();
        if(0 == drv_keypad_get_powerkey()) //电源键松开
        {
            flag = 1;
        }

        //有按键、或者电源键松开再按下
        if(dev_keypad_read(&key) || (drv_keypad_get_powerkey() && flag))
        {
            ret = DDI_ERR;
            dev_wireless_thread_run_switch(TRUE);
            dev_wireless_strategy_run_switch(TRUE);
            break;
        }
        
        fail_reason = dev_wireless_get_status(WS_GET_REG_NETWORK_FAIL_REASON);
        //这两种不做低功耗操作
        if(WS_NO_WIRELESS_MODULE == fail_reason
            || WS_UNKNOW_WIRELESS_MODULE == fail_reason)
        {
            ret = DDI_OK;
            break;
        }

        pdp_state = dev_wireless_get_status(WS_GET_WIRELESS_STATE);
        if(WS_NO_INSERT_SIM == fail_reason || 
            (WS_ACTIVE_PDP_SUCCESS == pdp_state && dev_wireless_thread_is_free()))
        {
            ret = MIL_suspend(AT_API_BLOCK);
            if(ret == DDI_OK)
            {
                TRACE_INFO("let module into sleep success");
                break;
            }
        }

        if(WS_SEARCHING_NETWORK == pdp_state)
        {
            if(1 == check_creg_flag)     //一直是驻网状态第二次进入时只等3秒
            {
                timeout = 3*1000;
            }
        }
        else
        {
            check_creg_flag = 0;
            timeout = WIRE_AT_ACTIVE_PDP_TIMEOUT+10*1000;
        }

        //一直获取ip失败
        if(dev_user_querrytimer(cur_ticks, timeout))
        {
            if(0 == check_creg_flag && timeout > 3*1000)
            {
                check_creg_flag = 1;
            }
            
            times++;
            TRACE_INFO("let module into sleep overtime");
            //关闭中断任务，并清除串口任务
            dev_wireless_thread_run_switch(FALSE);
            dev_wireless_thread_set_result(WIRE_AT_RESULT_OK);
            dev_com_flush(WIRELESS_PORT);
            dev_wireless_strategy_run_switch(FALSE);
            ret = MIL_suspend(AT_API_BLOCK);
            if(ret == DDI_OK)
            {
                TRACE_INFO("let module into sleep success");
                break;
            }
            else if(times >= 3)
            {
                TRACE_INFO("let module into sleep fail, then poweroff module");
                dev_wireless_poweroff();
                dev_wireless_set_control_strategy_step(WS_MODULE_POWER_ON);
                ret = DDI_OK;
                break;
            }
        }

        dev_user_delay_ms(500);
    }

_out:
#ifdef __WIRELESS_SLEEP_LOG
    memset(buff, 0, sizeof(buff));
    snprintf(buff, sizeof(buff), "suspend:%d,times:%d\r\n", dev_user_gettimeID()-curr_ticks, times);
    write_log(buff);
#endif
    return ret;
}

/**
 * @brief 开始记录拨号收发的日志
 * @retval  无
 */
void dev_wireless_start_dial_log_collect(void)
{
    if(!g_wireless_device_info.m_dial_log_buff)
    {
        g_wireless_device_info.m_dial_log_buff = (s8 *)k_malloc(WIRELESS_DIAL_LOG_SIZE);
        if(g_wireless_device_info.m_dial_log_buff)
        {
            memset(g_wireless_device_info.m_dial_log_buff, 0, WIRELESS_DIAL_LOG_SIZE);
        }
    }
}

/**
 * @brief 输出拨号收发的日志
 * @retval  无
 */
void dev_wireless_print_dial_log(void)
{
    if(g_wireless_device_info.m_dial_log_buff)
    {
        dev_com_write(DEBUG_PORT_NO, g_wireless_device_info.m_dial_log_buff, strlen(g_wireless_device_info.m_dial_log_buff));
        dev_com_flush(DEBUG_PORT_NO);
        k_free(g_wireless_device_info.m_dial_log_buff);
        g_wireless_device_info.m_dial_log_buff = NULL;
    }
}

/**
 * @brief 设置无线模块当前的电源状态
 * @param[in] power_flag
 * @retval  无
 */
void dev_wireless_set_power_flag(s32 power_flag)
{
#if(WIRELESS_CONTROL_BATTERY_COLLECT == TRUE)
    if(power_flag)
    {
        dev_misc_batresumegetvoltimer_open(0);  //延时采集电量    sxl20200922
    }
#endif
    g_wireless_device_info.m_power_flag = power_flag;
}

/**
 * @brief 获取无线模块当前的电源状态
 * @retval  TRUE  上电
 * @retval  FALSE  下电
 */
s32 dev_wireless_get_power_flag(void)
{
    return g_wireless_device_info.m_power_flag;
}

#if(WIRELESS_USE_EXTERANL_AMP == TRUE)
/**
 * @brief 获取声音文件大小
 * @param[in] 无
 * @retval  DDI_OK 从文件获取成功
 * @retval  DDI_ERR 使用默认apn
 */
static s32 wireless_audio_get_vol_from_file(void)
{
    s32 fd = vfs_open(WIRELESS_AUDIO_FILE, "r");
    s32 ret = DDI_ERR;
    wireless_device_info_t *device_info = dev_wireless_get_device();
    s8 buff[128] = {0};
    s8 dst[1][SPLIT_PER_MAX] = {0};

    do{
        if(fd < 0)
        {
            device_info->m_audio_info.m_audio_vol = 5;   //默认声音
            break;
        }

        memset(buff, 0, sizeof(buff));
        ret = vfs_read(fd, buff, sizeof(buff));
        if(ret <= 0)
        {
            ret = DDI_ERR;
            break;
        }

        memset(dst, 0, sizeof(dst));
        ret = dev_maths_split(dst, sizeof(dst)/sizeof(dst[0]), buff, ",", SPLIT_NOIGNOR);
        if(ret > 0)
        {
            dev_maths_str_trim(dst[0]);
            device_info->m_audio_info.m_audio_vol = dev_maths_asc_to_u32(dst[0], strlen(dst[0]), MODULUS_DEC);
            ret = DDI_OK;
        }
        else
        {
            ret = DDI_ERR;
        }
    }while(0);

    if(fd >= 0)
    {
        vfs_close(fd);
    }

    return ret;
}

/**
 * @brief 将声音文件保存到文件
 * @param[in] 无
 * @retval  DDI_OK 从文件获取成功
 * @retval  DDI_ERR 使用默认apn
 */
static s32 wireless_audio_save_vol_to_file(void)
{
    s32 fd = vfs_open(WIRELESS_AUDIO_FILE, "w");
    s32 ret = DDI_ERR;
    wireless_device_info_t *device_info = dev_wireless_get_device();
    s8 buff[128] = {0};

    do{
        if(fd < 0)
        {
            break;
        }

        memset(buff, 0, sizeof(buff));
        snprintf(buff, sizeof(buff), "%d", device_info->m_audio_info.m_audio_vol);
        ret = vfs_write(fd, buff, strlen(buff));
        if(ret <= 0)
        {
            TRACE_ERR("write fail");
            ret = DDI_ERR;
            break;
        }

        ret = DDI_OK;
    }while(0);

    if(fd >= 0)
    {
        vfs_close(fd);
    }

    return ret;
}


/**
 * @brief tts播放
 * @retval  无
 */
s32 dev_wireless_ttsaudio_play(TTS_ENCODE_TYPE ttsencode, u8 *ttstext, u32 ttstextlen)
{
    s32 ret;

    TRACE_DBG("enter:%s", ttstext);
    do{
        if(NULL == ttstext || 0 == ttstextlen)
        {
            ret = DDI_EINVAL;
            break;
        }

        if(dev_wireless_module_is_upgrading())
        {
            TRACE_INFO("upgrading now");
            ret = DDI_EBUSY;
            break;
        }

        if(0 == g_wireless_device_info.m_audio_info.m_audio_vol)
        {
            TRACE_ERR("vol is 0, they ignore play");
            ret = DDI_OK;
            break;
        }

        ret = MIL_ttsaudio_play(ttsencode, ttstext, ttstextlen);
        if(DDI_OK == ret)
        {
            g_wireless_device_info.m_audio_info.m_tts_play_done = FALSE;
            g_wireless_device_info.m_audio_info.m_mp3_play_done = TRUE;
        }
    }while(0);
        
    return ret;
}

/**
 * @brief tts播放MP3文件
 * @param[in]   audiofiletab - 播放文件索引
 *              audiofilenum - 播放文件个数
 * @retval  无
 */
s32 dev_wireless_audiofile_play(s8 *audio_string)
{
    s32 ret;
    
    TRACE_DBG("enter:%d", g_wireless_device_info.m_audio_info.m_audio_vol);
    do{
        if(NULL == audio_string)
        {
            ret = DDI_EINVAL;
            break;
        }

        if(dev_wireless_module_is_upgrading())
        {
            TRACE_INFO("upgrading now");
            ret = DDI_EBUSY;
            break;
        }
        
        if(0 == g_wireless_device_info.m_audio_info.m_audio_vol)
        {
            ret = DDI_OK;
            break;
        }

        ret = MIL_audiofile_play(audio_string);
        if(ret == DDI_OK)
        {
            g_wireless_device_info.m_audio_info.m_tts_play_done = TRUE;
            g_wireless_device_info.m_audio_info.m_mp3_play_done = FALSE;
        }
    }while(0);
    
    return ret;
}

/**
 * @brief mp3或者tts是否已播放完
 * @retval  TRUE 正在播放
 * @retval  FALSE 播放完
 */
s32 dev_wireless_audio_isplaying(void)
{
    return !(g_wireless_device_info.m_audio_info.m_mp3_play_done == TRUE
                && g_wireless_device_info.m_audio_info.m_tts_play_done == TRUE);
}

/**
 * @brief 获取模块端mp3文件的清单
 * @retval  无
 */
s32 dev_wireless_get_audio_file_list(s8 filelist[][WIRELESS_AUDIO_FILE_NAME_MAX], s32 filelist_num)
{
    s32 ret;
    
    TRACE_DBG("enter");
    do{
        if(dev_wireless_module_is_upgrading())
        {
            TRACE_INFO("upgrading now");
            ret = DDI_EBUSY;
            break;
        }
        
        ret = MIL_get_audio_file_list(filelist, filelist_num);
    }while(0);
        
    return ret;
}

/**
 * @brief 删除播报文件
 * @retval  无
 */
s32 dev_wireless_delete_audio_file(s8 *deletefilename)
{
    s32 ret;
    
    TRACE_DBG("enter");
    do{
        if(dev_wireless_module_is_upgrading())
        {
            TRACE_INFO("upgrading now");
            ret = DDI_EBUSY;
            break;
        }
        
        ret = MIL_delete_audio_file(deletefilename);
    }while(0);
        
    return ret;
}


/**
 * @brief 增加播报文件
 * @param[in]   file_index - 文件在tts.bin里的序号
 * @retval  无
 */
s32 dev_wireless_add_audio_file(s32 file_index)
{
    s32 ret;
    
    TRACE_DBG("enter");
    do{
        if(dev_wireless_module_is_upgrading())
        {
            TRACE_INFO("upgrading now");
            ret = DDI_EBUSY;
            break;
        }
        
        ret = MIL_add_audio_file(file_index);
    }while(0);
    
    return ret;
}

u8 dev_get_machine_code(void)
{
    u16 val = ddi_misc_ioctl(MISC_IOCTL_GETTMSMACHINETYPEID, 0, 0);

    return (val & 0xff00)>>8;
}

/**
 * @brief 设置模块播报音量
 * @retval  无
 */
s32 dev_wireless_setaudiovolume(u32 audiovolume)
{
    s32 ret;
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();
    s32 max_vol = 0;
    u8 machine_code = dev_get_machine_code();

    if(dev_wireless_module_is_upgrading())
    {
        TRACE_INFO("upgrading now");
        return DDI_EBUSY;
    }
    
    TRACE_DBG("enter:%d", audiovolume);
    if(0x57 == machine_code)   //P7 声音为0 3 4 5，依次对应静音、小、中、大
    {
        max_vol = 5;
    }
    else
    {
        max_vol = 6;
    }
    
    do{
        if(audiovolume > max_vol)  //sxl20201019
        {
            ret = DDI_EINVAL;
            break;
        }

        //模块不支持声音大小为0
        if(0 == audiovolume && g_wireless_device_info.m_audio_info.m_audio_vol > 0)
        {
            g_wireless_device_info.m_audio_info.m_audio_vol = 0;
#ifdef WIRELESS_USE_SAVEEDVOL
            wireless_audio_save_vol_to_file();
#endif
            ret = DDI_OK;
            break;
        }

        if(0x57 == machine_code && audiovolume > 0)
        {
            audiovolume += 2;
        }

        if(audiovolume == g_wireless_device_info.m_audio_info.m_audio_vol)
        {
            TRACE_DBG("volume is same, then ignore this set:%d", audiovolume);
            ret = DDI_OK;
            break;
        }
        
        ret = MIL_set_audio_volume(audiovolume, AT_API_BLOCK);
        if(DDI_OK == ret)
        {
            g_wireless_device_info.m_audio_info.m_audio_vol = audiovolume;
#ifdef WIRELESS_USE_SAVEEDVOL
            wireless_audio_save_vol_to_file(); //sxl2020不保存到文件里,这里写文件会很频繁
#endif
        }
    }while(0);
    
    return ret;
}

/**
 * @brief 获取模块播报音量
 * @retval  无
 */
s32 dev_wireless_getaudiovolume(u32 *audiovolume)
{
    s32 ret;
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();
    u8 machine_code = dev_get_machine_code();
    
    TRACE_DBG("enter");
    do{
        if(dev_wireless_module_is_upgrading())
        {
            TRACE_INFO("upgrading now");
            ret = DDI_EBUSY;
            break;
        }
        
        if(0 == g_wireless_device_info.m_audio_info.m_audio_vol)
        {
            *audiovolume = 0;
            ret = DDI_OK;
            break;
        }
        
        if(!dev_wireless_thread_is_free())
        {
            TRACE_ERR("now is hanle task:%d, step:%d", thread_node_param->m_task_id, thread_node_param->m_step);
            ret = DDI_EBUSY;
            break;
        }
        
        ret = MIL_get_audio_volume(audiovolume);
        if(DDI_OK == ret)
        {
            if(0x57 == machine_code)
            {
                //应用层定义的是0-5范围
                if(*audiovolume < 3)
                {
                    *audiovolume = 3;
                }
                
                *audiovolume -= 2;
            }
        }
    }while(0);

    if(DDI_OK != ret)
    {
        TRACE_DBG("use memory data:%d", ret);
        *audiovolume = g_wireless_device_info.m_audio_info.m_audio_vol;
        ret = DDI_OK;
    }
    
    return ret;
}
#endif

/**
 * @brief   设置FOTA升级参数
 * @retval  无
 */
s32 dev_wireless_setfota_upgrade_param(module_ota_param_t *module_ota_param)
{
    s32 ret = DDI_ERR;
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();

    do{
        if(!module_ota_param)
        {
            ret = DDI_EINVAL;
            break;
        }

        if(0 == strlen(module_ota_param->m_url))
        {
            TRACE_ERR("url is null");
            ret = DDI_EINVAL;
            break;
        }

        if(!dev_wireless_thread_is_free())
        {
            TRACE_DBG("task:%d, step:%d", thread_node_param->m_task_id, thread_node_param->m_step);
            ret = DDI_EBUSY;
            break;
        }
        
        //ftp文件名不能为空
        if(OTA_DT_FTP == module_ota_param->m_fota_type
            && 0 == strlen(module_ota_param->m_filename))
        {
            TRACE_ERR("file not permit null");
            ret = DDI_EINVAL;
            break;
        }
            
        ret = MIL_setfota_upgrade_param(module_ota_param->m_fota_type, 
                                        module_ota_param->m_url, 
                                        module_ota_param->m_filename,
                                        module_ota_param->m_username, 
                                        module_ota_param->m_password);
    }while(0);
    
    return ret;
}

s32 dev_wireless_getfota_status(OTA_STEP_E *ota_step, ota_process_value_t *ota_process_value)
{
    s32 ret = DDI_ERR;
    wireless_device_info_t *wireless_device = dev_wireless_get_device();
    s32 flag = 0;
    
    do{
        if(!ota_step || !ota_process_value)
        {
            ret = DDI_EINVAL;
            break;
        }

        *ota_step = wireless_device->m_ota_info.m_step;
        memcpy(ota_process_value, &(wireless_device->m_ota_info.m_process_value), sizeof(ota_process_value_t));
        ret = DDI_OK;

        if(OTA_STEP_UPGRADE == wireless_device->m_ota_info.m_step)
        {
            switch(wireless_device->m_ota_info.m_process_value.m_handle_step)
            {
                case HS_EXEC_SUCC:         //升级完成，需要让无线模块重新走初始化流程
                    dev_wireless_module_reboot_handle();
                    dev_wireless_set_control_strategy_step(WS_MODULES_INIT);
                    flag = 1;
                    break;

                case HS_EXEC_FAIL:
                    flag = 1;
                    break;
            }

            if(flag)
            {
                //清除升级记录
                memset(&(wireless_device->m_ota_info), 0, sizeof(wireless_device->m_ota_info));
            }
        }
    }while(0);

    return ret;
}


/**
 * @brief ring pin和dtr管脚控制
 * @retval 
 * @retval 
 */
void dev_wireless_ringanddtrpin_init(void)
{
    #if WIRELESS_USE_UART_RING
    
    drv_wireless_set_ringanddtrpin_init();
    #endif
}


/**
 * @brief 进入休眠
 * @retval 
 * @retval 
 */
void dev_wireless_ringanddtrpin_enter_sleep(void)
{
    #if WIRELESS_USE_UART_RING
    drv_wireless_ringanddtrpin_enter_sleep();
    #endif
}


/**
 * @brief 退出休眠
 * @retval 
 * @retval 
 */
void dev_wireless_ringanddtrpin_exit_sleep(void)
{
    #if WIRELESS_USE_UART_RING
    drv_wireless_ringanddtrpin_exit_sleep();
    #endif
}

/**
 * @brief 获取无线模块id
 * @retval 
 * @retval 
 */
s32 dev_wireless_get_module_id(u8 *module_id)
{
    return MIL_get_module_id(module_id);
}

/**
 * @brief 是否支持ota升级
 * @retval 
 * @retval 
 */
s32 dev_wireless_is_support_ota(void)
{
    if(TRUE == MIL_is_support_ota())
    {
        return DDI_OK;
    }
    else
    {
        return DDI_ERR;
    }
}

/**
 * @brief 无线模块是否正在更新固件
 * @retval TRUE 正在更新，不允许操作无线模块
 * @retval FASE 没有更新
 */
s32 dev_wireless_module_is_upgrading(void)
{
    return (OTA_STEP_UPGRADE == g_wireless_device_info.m_ota_info.m_step
                && HS_PROCESSING == g_wireless_device_info.m_ota_info.m_process_value.m_handle_step);
}
#else
s32 dev_wireless_init(void)
{
    return 0;
}

s32 dev_wireless_suspend(void)
{
    return 0;
}

s32 dev_wireless_resume(void)
{
    return 0;
}

void dev_wireless_poweroff(void)
{
    
}

u8 dev_get_machine_code(void)
{
    return 0;
}
#endif