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
static wireless_parse_t  g_wireless_parse;
static wire_at_result_info_t g_wire_at_result_info_tab[] = {
    {WIRE_AT_RESULT_MODULE_READY, "AT command ready"},
    {WIRE_AT_RESULT_OK, "OK"},
    {WIRE_AT_RESULT_ERROR, "ERROR"}
};
static s32 g_wireless_is_need_restore_thread_param = FALSE;
static wireless_thread_node_param_t g_last_thread_node_param;

/**
 * @brief 确认是否有备份任务执行
 * @param[in] 无
 * @retval  TRUE 有备份任务
 * @retval  FALSE 无备份任务
 */
s32 dev_wireless_is_have_bak_node_param(void)
{
    return WIRE_TASK_FREE != g_last_thread_node_param.m_task_id;
}

void dev_wireless_clear_bak_node_param(void)
{
    memset(&g_last_thread_node_param, 0, sizeof(g_last_thread_node_param));
}

/**
 * @brief 接收完tcp数据，将线程参数恢复成之前的参数
 * @param[in] 无
 * @retval  无
 */
static void wireless_recovery_thread_param(void)
{
    if(TRUE == g_wireless_is_need_restore_thread_param)
    {
        TRACE_INFO("recovery thread to %d", g_last_thread_node_param.m_task_id);
        g_wireless_is_need_restore_thread_param = FALSE;
        dev_wireless_set_thread_node_param(&g_last_thread_node_param);
        dev_wireless_clear_bak_node_param();
    }
    else
    {
        dev_wireless_thread_set_result(WIRE_AT_RESULT_OK);
    }
}

/**
 * @brief 无线模块已知返回的命令处理
 * @param[in] 无
 * @retval  1 有已知返回数据
 * @retval  0 无已知返回数据
 */
static s32 wireless_data_know_response(void)
{
    u32 i;
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();
    s32 ret = 0;

    if(g_wireless_parse.m_rcnt<WIRELESSAT_RXBUF_MAX)
    {
        g_wireless_parse.m_rbuf[g_wireless_parse.m_rcnt] = 0;
    }
    
    i = 0;
    for(i=0; i<sizeof(g_wire_at_result_info_tab)/sizeof(g_wire_at_result_info_tab[0]); i++)
    {
        if(0 == strcmp(g_wireless_parse.m_rbuf, g_wire_at_result_info_tab[i].m_resp_words))
        {
            thread_node_param->m_result = g_wire_at_result_info_tab[i].m_wire_at_result;
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
static s32 wireless_data_report(void)
{
    s32 ret = 0;
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();

    do{
       //关机或复位模块、开关飞行模式、拨号、挂断时不处理上报
       if(WS_TASK_POWEROFF_MODULE == thread_node_param->m_task_id
        || WS_TASK_RESET_MODULE == thread_node_param->m_task_id
        || WS_TASK_OPEN_AIRPLANE == thread_node_param->m_task_id
        || WS_TASK_CLOSE_AIRPLANE == thread_node_param->m_task_id
        || WS_TASK_TELEPHONE_DAIL == thread_node_param->m_task_id
        || WS_TASK_TELEPHONE_HANGUP == thread_node_param->m_task_id)
       {
           break;
       }
       
       //寻网流程不捕获creg数据
       //主动查询creg不捕获creg数据---拨号时
       if(WS_TASK_SEARCHING_NETWORK != thread_node_param->m_task_id
          && WS_TASK_CHECK_REG != thread_node_param->m_task_id)
       {
           if(DDI_OK == MIL_net_reg_report_handle(g_wireless_parse.m_rbuf))
           {
                ret = 1;
                break;
           }
       }

       //激活pdp状态下的pdp返回不捕获
       //主动查询ip不捕获
       if((WS_TASK_ACTIVE_NETWORK != thread_node_param->m_task_id 
            && WS_TASK_GET_LOCAL_IP != thread_node_param->m_task_id
            && WS_TASK_INACTIVE_NETWORK != thread_node_param->m_task_id)
                || (WS_TASK_ACTIVE_NETWORK == thread_node_param->m_task_id && WIRE_ACTIVE_NETWORK_STEP_GET_IP != thread_node_param->m_step))
       {
           if(DDI_OK == MIL_pdp_report_handle(g_wireless_parse.m_rbuf))
           {
                ret = 1;
                break;
           }
       }

       //查询套接字清单时不捕获返回内容
       if(WS_TASK_GET_SOCKET_LIST != thread_node_param->m_task_id)
       {
           if(DDI_OK == MIL_tcp_connect_report_handle(g_wireless_parse.m_rbuf))
           {
                ret = 1;
                break;
           }
       }

       if(DDI_OK == MIL_socket_report_handle(g_wireless_parse.m_rbuf))
       {
            ret = 1;
            break;
       }

       if(DDI_OK == MIL_fota_processing_report_handle(g_wireless_parse.m_rbuf))
       {
            ret = 1;
            break;
       }
    }while(0);

    return ret;
}

/**
 * @brief 挂在无线模块端设备的控制指令返回
 * @param[in] 无
 * @retval  1 有数据
 * @retval  0 没有数据
 */
static s32 wireless_data_from_additional_device(void)
{
    s32 ret = 0;

    do{
#if(WIRELESS_USE_EXTERANL_AMP == TRUE)
        if(DDI_OK == MIL_audioend_report_handle(g_wireless_parse.m_rbuf))
        {
            ret = 1;
            break;
        }
#endif
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
static s32 wireless_data_handle(void)
{
    u32 i;
    u32 j;
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();
    s32 ret = 0;

    if(g_wireless_parse.m_rcnt<WIRELESSAT_RXBUF_MAX)
    {
        g_wireless_parse.m_rbuf[g_wireless_parse.m_rcnt] = 0;
    }

    do{
        //任务函数不为空的时候处理，一条指令应答如close socket，先回ok,后面再回复socket关闭情况，如果判断数据返回那里有处理，且中间有休眠，获取的状态result就被这里修改了。
        if(NULL != thread_node_param->m_func)
        {
            if(wireless_data_know_response())
            {
#ifndef IPV6_TEST
                //处理无线模块的异常重启
                if(thread_node_param->m_result == WIRE_AT_RESULT_MODULE_READY
                    && WS_MODULE_POWER_ON != dev_wireless_get_status(WS_GET_WIRELESS_STATE)
                    && WS_MODULES_INIT != dev_wireless_get_status(WS_GET_WIRELESS_STATE)
                    && !dev_wireless_module_is_upgrading())  //升级时返回的at command ready不能处理
                {
                    dev_wireless_module_reboot_handle();
                    ret = 2;
                }
                else
                {
                    ret = 1;
                }
#else
                ret = 1;
#endif
                goto _out;
                break;
            }
        }

        if(wireless_data_report())
        {
            ret = 2;
            goto _out;
            break;
        }

        if(wireless_data_from_additional_device())
        {
            ret = 2;
            goto _out;
            break;
        }
    }while(0);

    
    if(NULL != thread_node_param->m_func)
    {
        thread_node_param->m_result = WIRE_AT_RESULT_OTHER;
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
static s32 wireless_data_recv_normal(void)
{
    s32 i = 0;
    u8 data;
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();
    s8 dst[3][SPLIT_PER_MAX];
    s32 ret = 0;
    s8 *pstr = NULL;
    u32 timeover = 0;
    s32 socket_id = 0;
    wireless_device_info_t *device_info = dev_wireless_get_device();
    
    i = dev_com_read(WIRELESS_PORT, &data, 1);
    if(i <= 0)
    {
        return -1;
    }

    g_wireless_parse.m_rbuf[g_wireless_parse.m_rcnt++] = data;
    if(g_wireless_parse.m_rcnt == 2)
    {
        if(0 == memcmp(&g_wireless_parse.m_rbuf[0], AT_CMD_RES_END, 2))
        {
            //接收完成,仅收到0d0a,扔掉
            g_wireless_parse.m_rcnt = 0;
            return 0;
        }
    }
    else if(g_wireless_parse.m_rcnt>2)
    {
        pstr = MIL_get_tcp_recv_cmd_head_kw();
        if(pstr && g_wireless_parse.m_rcnt > strlen(pstr)+3
            && 0 == memcmp(g_wireless_parse.m_rbuf, pstr, strlen(pstr)))
        {
            ret = dev_maths_split(dst, sizeof(dst)/sizeof(dst[0]), g_wireless_parse.m_rbuf+strlen(pstr), ",", SPLIT_NOIGNOR);
            if(ret == 3)
            {
                dev_maths_str_trim(dst[0]);
                dev_maths_str_trim(dst[1]);
                socket_id = dev_maths_asc_to_u32(dst[0], strlen(dst[0]), MODULUS_DEC);
                TRACE_DBG("socketid:%d, recvlen:%d", socket_id,
                                                     dev_maths_asc_to_u32(dst[1], strlen(dst[1]), MODULUS_DEC));

                //要关闭的套接字有接收数据时，直接忽略
                if(socket_id == device_info->m_cur_close_socket_id)
                {
                    TRACE_DBG("ignore tcp recv when there is socket close task handling");
                    g_wireless_parse.m_rcnt = 0;
                    return 0;
                }
                
                g_wireless_parse.m_socketid = socket_id;
                g_wireless_parse.m_rlencnt = dev_maths_asc_to_u32(dst[1], strlen(dst[1]), MODULUS_DEC);
                g_wireless_parse.m_special |= WIRE_SPECIAL_BIT0;
                g_wireless_parse.m_rcnt = 0;

                //串口1秒12K左右
                timeover = (g_wireless_parse.m_rlencnt*1000*10)/(12*1024);
                if(timeover < 1000)
                {
                    timeover = WIRE_AT_POWEROFF_WAIT_TIMEOUT;
                }

                //当前有其它任务在执行
                if(!dev_wireless_thread_is_free())
                {
                    TRACE_DBG("ready to save thread:%d", thread_node_param->m_task_id);
                    g_wireless_is_need_restore_thread_param = TRUE;
                    memcpy(&g_last_thread_node_param, thread_node_param, sizeof(wireless_thread_node_param_t));
                }
                
                dev_wireless_thread_set_param(WS_TASK_SOCKET_RECVDATA, NULL, timeover);
                memset(g_wireless_parse.m_rbuf, 0, sizeof(g_wireless_parse.m_rbuf));
                TRACE_INFO("timeover:%d, %02x", timeover, g_wireless_parse.m_special);
                return 0;
            }
        }
        
        if(0 == memcmp(&g_wireless_parse.m_rbuf[g_wireless_parse.m_rcnt-2], AT_CMD_RES_END, 2))
        {
            g_wireless_parse.m_rbuf[g_wireless_parse.m_rcnt-2] = 0;
            g_wireless_parse.m_rcnt -= 2;
            //接收完成
            return 2;
        }

        if(g_wireless_parse.m_rcnt >= WIRELESSAT_RXBUF_MAX)
        {
            TRACE_ERR("recv overflow");
            g_wireless_parse.m_rcnt = 0;
            return 0;
        }
    }
    else if(1 == g_wireless_parse.m_rcnt && g_wireless_parse.m_rbuf[0] == 0)
    {
        //接收完成,仅收到0d0a,扔掉
        g_wireless_parse.m_rcnt = 0;
        return 0;
    }

    return 0;
}

/**
 * @brief 接收tcp套接字的后部分数据
 * @param[in] 无
 * @retval  -1:不需要外部去处理数据，所以直接返回-1
 */
static s32 wireless_data_recv_miprtcp(void)
{
    s32 socket_sn = 0;
    wireless_device_info_t *device_info = dev_wireless_get_device();
    s32 i = 0, j = 0;
    s32 ret = 0;
    s32 clean_recv_flag = FALSE;

    do{
        socket_sn = g_wireless_parse.m_socketid - 1;
        if(socket_sn < 0 || socket_sn >= WIRELESS_SOCKET_MAX)
        {
            TRACE_ERR("socket id don't exist");
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
        j = i > WIRELESSAT_RXBUF_MAX ? WIRELESSAT_RXBUF_MAX : i;
        j = g_wireless_parse.m_rlencnt > j ? j : g_wireless_parse.m_rlencnt;

        ret = dev_com_read(WIRELESS_PORT, g_wireless_parse.m_rbuf, j);
        if(ret <= 0)
        {
            TRACE_ERR("j:%d, i:%d, ret:%d", j, i, ret);
            break;
        }

        dev_circlequeue_write(&(device_info->m_socket_info[socket_sn]->m_read_buffer), g_wireless_parse.m_rbuf, ret);
        g_wireless_parse.m_rlencnt -= ret;

        if(g_wireless_parse.m_rlencnt <= 0)
        {
            TRACE_INFO("this packet recv ok");
            clean_recv_flag = TRUE;
        }

        //要关闭的套接字有接收数据时，直接忽略
        if(g_wireless_parse.m_socketid == device_info->m_cur_close_socket_id)
        {
            TRACE_DBG("ignore tcp recv when there is socket close task handling");
            clean_recv_flag = TRUE;
        }
    }while(0);

    if(clean_recv_flag)
    {
        //dev_wifi_thread_set_result(WIFI_AT_RESULT_OK);    //Mask by xiaohonghui 2020.8.13 会导致其它task在执行的时候得到了结果
        //恢复线程参数
        wireless_recovery_thread_param();
        dev_wireless_init_parse_data();
    }
    return -1;
}

/**
 * @brief 数据接收与解析
 * @param[in] 无
 * @retval  > 0 有数据
 * @retval  0 没有数据
 */
static s32 wireless_data_recv_and_parse(void)
{
    s32 ret;

    while(1)
    {
        if(g_wireless_parse.m_special&WIRE_SPECIAL_BIT0)  //收取tcp数据
        {
            ret = wireless_data_recv_miprtcp();
        }
      #if 0  
        else if(thread_node_param->m_special&WIRE_SPECIAL_RCVCMD)
        {   //接收指定命令的数据内容
            ret = dev_wire_at_rev_specialcmd();
            if(ret>0)
            {
                dev_wire_at_ctl_specialcmd_flg(0);
            }
        }
      #endif
        else
        {
            ret = wireless_data_recv_normal();               //收取短的指令返回数据
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

/**
 * @brief 初始化全局变量
 * @param[in] 无
 * @retval  无
 */
void dev_wireless_init_parse_data(void)
{
    memset(&g_wireless_parse, 0, sizeof(g_wireless_parse));
}

/**
 * @brief 获取当前正在接收数据的socketid
 * @param[in] 无
 * @retval  socket_id
 */
s32 dev_wireless_get_current_socket_id(void)
{
    return g_wireless_parse.m_socketid;
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
s32 dev_wireless_data_handle(void)
{
    s32 ret = DDI_ERR;
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();
    wireless_device_info_t *device_info = dev_wireless_get_device();
    s32 len = 0;
    
    ret = wireless_data_recv_and_parse();
    if(0 == ret)
    {
        if(thread_node_param->m_timeover != 0)
        {
            thread_node_param->m_timeover--;
            if(0 == thread_node_param->m_timeover)
            {
                //超时处理
                if(g_wireless_parse.m_special&WIRE_SPECIAL_BIT0)
                {
                    TRACE_INFO("recv timerout");
                    dev_wireless_thread_set_result(WIRE_AT_RESULT_TIMEROUT);
                    wireless_recovery_thread_param();
                }
                else if(thread_node_param->m_func != NULL)
                {
                    TRACE_INFO("timerout:%d", thread_node_param->m_task_id);
                    thread_node_param->m_result = WIRE_AT_RESULT_TIMEROUT;
                    thread_node_param->m_func(thread_node_param->m_result, g_wireless_parse.m_rbuf, g_wireless_parse.m_rcnt);
                }

                dev_wireless_init_parse_data();
            }
        }
    }
    else
    {
        TRACE_INFO("<< %s", g_wireless_parse.m_rbuf);

        if(device_info->m_dial_log_buff)
        {
            len = strlen(device_info->m_dial_log_buff);
            snprintf(device_info->m_dial_log_buff+len, WIRELESS_DIAL_LOG_SIZE-len, "<< %s-%d\r\n", g_wireless_parse.m_rbuf, dev_user_gettimeID());
        }
        
        ret = wireless_data_handle();
        
        TRACE_DBG("m_result=%d, ret:%d", thread_node_param->m_result, ret);
        switch(ret)
        {
            case 1:
            case 3:
                if(thread_node_param->m_func)
                {
                    thread_node_param->m_func(thread_node_param->m_result, g_wireless_parse.m_rbuf, g_wireless_parse.m_rcnt);
                }
                else
                {
                    TRACE_ERR("have nothing to do func");
                }
                break;
            case 2: //主动上报，已在wireless_data_handle里处理
                break;
        }

        dev_wireless_init_parse_data();
    }
}
#endif
