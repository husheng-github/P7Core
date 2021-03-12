#include "devglobal.h"
#include "ddi_com.h"
#include "ddi_gprs.h"
#include "drv_wireless_io.h"
#include "dev_wireless_thread.h"
#include "dev_wireless_thread_additional_device.h"
#include "module.h"
#include "dev_wireless.h"
#include "module_FIBOCOM.h"
#include "module_FIBOCOM_additional.h"
#include "wireless_control_strategy.h"
#include "wireless_data_handle.h"

/*****************************************************************************
*                                                                            *
*  @file     dev_wireless_thread_additional_device.C                                                *
*  @brief    此文件主要是对广和通无线模块上挂载的外围设备线程函数进行处理                                       *
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

#if(WIRELESS_WITH_BT == TRUE)
static s32 G500_bt_open_callback(u8 result, u8 *src, u16 srclen)
{
    s32 ret = DDI_ERR;
    u8 *pstr = NULL;
    wireless_device_info_t *device_info = dev_wireless_get_device();
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_additional_node_param();

    TRACE_DBG("result:%d, src:%s", result, src);
    do{
        pstr = strstr(src, "BTOK");
        if(pstr)                         ///<打开成功
        {
            device_info->m_bt_info.m_bt_flag = TRUE;
            device_info->m_bt_info.m_open_flag = TRUE;
            dev_wireless_thread_additional_set_result(WIRE_AT_RESULT_OK);
            ret = DDI_OK;
            break;
        }

        pstr = strstr(src, "BTERROR");
        if(pstr)
        {
            device_info->m_bt_info.m_open_flag = FALSE;
            dev_wireless_thread_additional_set_result(WIRE_AT_RESULT_ERROR);
            ret = DDI_ERR;
            break;
        }

        if(WIRE_AT_RESULT_TIMEROUT == result)
        {
            dev_wireless_thread_additional_set_result(WIRE_AT_RESULT_TIMEROUT);
            ret = DDI_ERR;
            break;
        }
        
        
    }while(0);

    return ret;
}

s32 G500_bt_open(u8 type, AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;
    s8 buff[128];
    wireless_device_info_t *device_info = dev_wireless_get_device();
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_additional_node_param();

    switch(at_api_get_type)
    {
        case AT_API_NO_BLOCK:
            break;
        case AT_API_BLOCK:
            snprintf(buff, sizeof(buff), "AT=AT+BTOPEN=%d", type);
            dev_wireless_send_at_cmd(buff, at_api_get_type, TRUE);
            dev_wireless_thread_run_switch(FALSE);
            dev_wireless_thread_additional_set_param(WS_TASK_OPENBT, G500_bt_open_callback, WIRE_AT_DEFAULT_TIMEOUT);
            dev_wireless_thread_run_switch(TRUE);
            while(1)
            {
                if(dev_wireless_thread_additional_task_is_change(WS_TASK_OPENBT))
                {
                    ret = DDI_EBUSY;
                    break;
                }

                if(dev_wireless_thread_additional_is_free())
                {
                    if(TRUE == device_info->m_bt_info.m_bt_flag)
                    {
                        device_info->m_bt_info.m_bt_flag = FALSE;
                        TRACE_INFO("open bt success!");
                        ret = DDI_OK;
                    }
                    else
                    {
                        TRACE_INFO("open bt fail!");
                        ret = DDI_ERR;
                    }
                    break;
                }

                ddi_misc_msleep(50);
            }
        break;
    }

    return ret;
}

static s32 G500_bt_close_callback(u8 result, u8 *src, u16 srclen)
{
    s32 ret = DDI_ERR;
    s8 *pstr = NULL;
    wireless_device_info_t *device_info = dev_wireless_get_device();
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_additional_node_param();

    TRACE_DBG("result:%d, src:%s", result, src);
    do{
        pstr = strstr(src, "BTOK");
        if(pstr)
        {
            device_info->m_bt_info.m_bt_flag = TRUE;
            device_info->m_bt_info.m_open_flag = FALSE;
            dev_wireless_thread_additional_set_result(WIRE_AT_RESULT_OK);
            ret = DDI_OK;
            break;
        }

        pstr = strstr(src, "BTERROR");
        if(pstr)
        {
            dev_wireless_thread_additional_set_result(WIRE_AT_RESULT_ERROR);
            ret = DDI_ERR;
            break;
        }

        if(WIRE_AT_RESULT_TIMEROUT == result)
        {
            dev_wireless_thread_additional_set_result(WIRE_AT_RESULT_TIMEROUT);
            ret = DDI_ERR;
            break;
        }
    }while(0);

    return ret;
}

s32 G500_bt_close(AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;
    wireless_device_info_t *device_info = dev_wireless_get_device();
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_additional_node_param();

    switch(at_api_get_type)
    {
        case AT_API_NO_BLOCK:
            break;

        case AT_API_BLOCK:
            dev_wireless_send_at_cmd("AT=AT+BTCLOSE=0", at_api_get_type, TRUE);
            dev_wireless_thread_run_switch(FALSE);
            dev_wireless_thread_additional_set_param(WS_TASK_CLOSEBT, G500_bt_close_callback, WIRE_AT_DEFAULT_TIMEOUT);
            dev_wireless_thread_run_switch(TRUE);
        while(1)
        {
            if(dev_wireless_thread_additional_task_is_change(WS_TASK_CLOSEBT))
            {
                ret = DDI_EBUSY;
                break;
            }

            if(dev_wireless_thread_additional_is_free())
            {
                if(TRUE == device_info->m_bt_info.m_bt_flag)
                {
                    device_info->m_bt_info.m_bt_flag = FALSE;
                    TRACE_INFO("close bt success!");
                    ret = DDI_OK;
                }
                else
                {
                    TRACE_INFO("close bt fail!");
                    ret = DDI_ERR;
                }
                break;
            }

            ddi_misc_msleep(50);
        }
        break;
    }

    return ret;
}

static s32 G500_bt_get_status_callback(u8 result, u8 *src, u16 srclen)
{
    s32 ret = DDI_ERR;
    s8 *pstr = NULL;
    wireless_device_info_t *device_info = dev_wireless_get_device();
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_additional_node_param();

    TRACE_DBG("result:%d, src:%s", result, src);
    do{
        pstr = strstr(src, "+BTSTATUS:");
        if(pstr)
        {
            pstr += strlen("+BTSTATUS:");
            device_info->m_bt_info.m_bt_status = dev_maths_asc_to_u32(pstr, strlen(pstr), MODULUS_DEC);
            thread_node_param->m_lparam = 1;
            break;
        }

        pstr = strstr(src, "BTOK");
        if(pstr)
        {
            if(1 == thread_node_param->m_lparam)
            {
                device_info->m_bt_info.m_bt_flag = TRUE;
                dev_wireless_thread_additional_set_result(WIRE_AT_RESULT_OK);
                ret = DDI_OK;
                break;
            }
            else
            {
                dev_wireless_thread_additional_set_result(WIRE_AT_RESULT_ERROR);
                ret = DDI_ERR;
                break;
            }
        }

        pstr = strstr(src, "BTERROR");
        if(pstr)
        {
            dev_wireless_thread_additional_set_result(WIRE_AT_RESULT_ERROR);
            ret = DDI_ERR;
            break;
        }

        if(WIRE_AT_RESULT_TIMEROUT == result)
        {
            dev_wireless_thread_additional_set_result(WIRE_AT_RESULT_TIMEROUT);
            ret = DDI_ERR;
            break;
        }
    }while(0);

    return ret;
}

s32 G500_bt_get_status(AT_API_GET_TYPE_E at_api_get_type)
{
    
    s32 ret = DDI_ERR;
    wireless_device_info_t *device_info = dev_wireless_get_device();
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_additional_node_param();
    
    switch(at_api_get_type)
    {
        case AT_API_NO_BLOCK:
            break;

        case AT_API_BLOCK:
            dev_wireless_send_at_cmd("AT=AT+BTSTATUS=?", at_api_get_type, TRUE);
            dev_wireless_thread_run_switch(FALSE);
            dev_wireless_thread_additional_set_param(WS_TASK_GET_BT_STATUS, G500_bt_get_status_callback, WIRE_AT_DEFAULT_TIMEOUT);
            dev_wireless_thread_run_switch(TRUE);
            while(1)
            {
                if(dev_wireless_thread_additional_task_is_change(WS_TASK_GET_BT_STATUS))
                {
                    ret = DDI_EBUSY;
                    break;
                }

                if(dev_wireless_thread_additional_is_free())
                {
                    if(TRUE == device_info->m_bt_info.m_bt_flag)   //sxl?20200927  m_bt_flag这个是做什么用的
                    {
                        TRACE_INFO("get bt status success!");
                        device_info->m_bt_info.m_bt_flag = FALSE;
                        ret = device_info->m_bt_info.m_bt_status;
                    }
                    else
                    {
                        TRACE_INFO("get bt status fail!");
                        ret = DDI_ERR;
                    }
                    break;
                }

                ddi_misc_msleep(50);
            }
            break;
            
    }

    return ret;
}

static s32 G500_bt_get_ver_callback(u8 result, u8 *src, u16 srclen)
{
    s32 ret = DDI_ERR;
    s8 *pstr = NULL;
    wireless_device_info_t *device_info = dev_wireless_get_device();
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_additional_node_param();

    TRACE_DBG("result:%d, src:%s", result, src);
    do{
        pstr = strstr(src, "+BTVER:");
        if(pstr)
        {
            pstr += strlen("+BTVER:");
            snprintf(device_info->m_bt_info.m_bt_ver, sizeof(device_info->m_bt_info.m_bt_ver), "%s", pstr);
            thread_node_param->m_lparam = 1;
            ret = DDI_OK;
            break;
        }

        pstr = strstr(src, "BTOK");
        if(pstr)
        {
            if(1 == thread_node_param->m_lparam)
            {
                device_info->m_bt_info.m_bt_flag = TRUE;
                dev_wireless_thread_additional_set_result(WIRE_AT_RESULT_OK);
                ret = DDI_OK;
                break;
            }
            else
            {
                dev_wireless_thread_additional_set_result(WIRE_AT_RESULT_ERROR);
                ret = DDI_ERR;
                break;
            }
        }

        pstr = strstr(src, "BTERROR");
        if(pstr)
        {
            dev_wireless_thread_additional_set_result(WIRE_AT_RESULT_ERROR);
            ret = DDI_ERR;
            break;
        }

        if(WIRE_AT_RESULT_TIMEROUT == result)
        {
            dev_wireless_thread_additional_set_result(WIRE_AT_RESULT_TIMEROUT);
            ret = DDI_ERR;
            break;
        }
    }while(0);

    return ret;
}

s32 G500_bt_get_ver(u8 *ver_buf, s32 buf_size, AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;
    wireless_device_info_t *device_info = dev_wireless_get_device();
    wireless_thread_node_param_t *thread_node_param = dev_wireless_get_thread_additional_node_param();

    switch(at_api_get_type)
    {
        case AT_API_NO_BLOCK:
            break;

        case AT_API_BLOCK:
            dev_wireless_send_at_cmd("AT=AT+BTVER=?", at_api_get_type, TRUE);
            dev_wireless_thread_run_switch(FALSE);
            dev_wireless_thread_additional_set_param(WS_TASK_GET_BT_VER, G500_bt_get_ver_callback, WIRE_AT_DEFAULT_TIMEOUT);
            dev_wireless_thread_run_switch(TRUE);
            while(1)
            {
                if(dev_wireless_thread_additional_task_is_change(WS_TASK_GET_BT_VER))
                {
                    ret = DDI_EBUSY;
                    break;
                }

                if(dev_wireless_thread_additional_is_free())
                {
                    if(TRUE == device_info->m_bt_info.m_bt_flag)
                    {
                        TRACE_INFO("get bt ver success!");
                        device_info->m_bt_info.m_bt_flag = FALSE;
                        snprintf(ver_buf, buf_size, "%s", device_info->m_bt_info.m_bt_ver);
                        ret = DDI_OK;
                    }
                    else
                    {
                        TRACE_INFO("get bt ver fail!");
                        ret = DDI_ERR;
                    }
                    break;
                }

                ddi_misc_msleep(50);
            }
            break;
        }

    return ret;
}

static s32 G500_bt_set_param_callback(u8 result, u8 *src, u16 srclen)
{
    s32 ret = DDI_ERR;
    s8 *pstr = NULL;
    wireless_device_info_t *device_info = dev_wireless_get_device();
    wireless_thread_node_param_t *thread_node_param = dev_wireless_get_thread_additional_node_param();

    TRACE_DBG("result:%d, src:%s", result, src);
    do{
        pstr = strstr(src, "BTOK");
        if(pstr)
        {
            TRACE_INFO("handle success");
            device_info->m_bt_info.m_bt_flag = TRUE;
            dev_wireless_thread_additional_set_result(WIRE_AT_RESULT_OK);
            ret = DDI_OK;
            break;
        }

        pstr = strstr(src, "BTERROR");
        if(pstr)
        {
            TRACE_INFO("handle fail");
            dev_wireless_thread_additional_set_result(WIRE_AT_RESULT_ERROR);
            ret = DDI_ERR;
            break;
        }

        if(WIRE_AT_RESULT_TIMEROUT == result)
        {
            dev_wireless_thread_additional_set_result(WIRE_AT_RESULT_TIMEROUT);
            ret = DDI_ERR;
            break;
        }
    }while(0);

    return ret;
}

s32 G500_bt_set_param(bt_param_t *param, AT_API_GET_TYPE_E at_api_get_type)
{
    s32 i = 0;
    s32 ret = DDI_ERR;
    s8 buff[128];
    s8 mac_addr[32];
    s8 *pstr = mac_addr;
    s32 maclen = 6;
    wireless_device_info_t *device_info = dev_wireless_get_device();
    wireless_thread_node_param_t *thread_node_param = dev_wireless_get_thread_additional_node_param();

    switch(at_api_get_type)
    {
        case AT_API_NO_BLOCK:
            break;

        case AT_API_BLOCK:
            if((0 != param->m_type & 0x02) && (param->m_addr[0] & 0x01))
            {
                TRACE_INFO("DDI_EINVAL");
                ret = DDI_EINVAL;
                break;
            }

            for(i = 0; i < maclen; i++)
            {
                sprintf(pstr, "%02x", param->m_addr[i]);
                pstr += 2;
                //snprintf(mac_addr+strlen(mac_addr), sizeof(mac_addr)-strlen(mac_addr), "%02x", param->m_addr[i]);
            }

            if(1 == param->m_type)
            {
                snprintf(buff, sizeof(buff), "AT=AT+BTPARAMSET=%d,%s", param->m_type, param->m_name);
            }
            else if(2 == param->m_type)
            {
                snprintf(buff, sizeof(buff), "AT=AT+BTPARAMSET=%d,%s", param->m_type, mac_addr);
            }
            else if(3 == param->m_type)
            {
                snprintf(buff, sizeof(buff), "AT=AT+BTPARAMSET=%d,%s,%s", param->m_type, param->m_name, mac_addr);
            }

            dev_wireless_send_at_cmd(buff, at_api_get_type, TRUE);
            dev_wireless_thread_run_switch(FALSE);
            dev_wireless_thread_additional_set_param(WS_TASK_SET_BTPARAM, G500_bt_set_param_callback, WIRE_AT_DEFAULT_TIMEOUT);
            dev_wireless_thread_run_switch(TRUE);
            while(1)
            {
                if(dev_wireless_thread_additional_task_is_change(WS_TASK_SET_BTPARAM))
                {
                    TRACE_INFO("DDI_EBUSY");
                    ret = DDI_EBUSY;
                    break;
                }

                if(dev_wireless_thread_additional_is_free())
                {
                    if(TRUE == device_info->m_bt_info.m_bt_flag)
                    {
                        device_info->m_bt_info.m_bt_flag = FALSE;
                        TRACE_INFO("set bt param success!");
                        ret = DDI_OK;
                    }
                    else
                    {
                        TRACE_INFO("set bt param fail!");
                        ret = DDI_ERR;
                    }
                    break;
                }

                ddi_misc_msleep(50);
            }
    }

    return ret;
}

static s32 G500_bt_get_param_callback(u8 result, u8 *src, u16 srclen)
{
    s32 ret = DDI_ERR;
    s8 *pstr = NULL;
    s8 dst[3][SPLIT_PER_MAX] = {0};
    wireless_device_info_t *device_info = dev_wireless_get_device();
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_additional_node_param();

    TRACE_DBG("result:%d, src:%s", result, src);
    do{
        pstr = strstr(src, "+BTPARAMGET:");
        if(pstr)
        {
            pstr += strlen("+BTPARAMGET:");
            memset(dst, 0, sizeof(dst));
            thread_node_param->m_lparam = 1;
            ret = dev_maths_split(dst, sizeof(dst)/sizeof(dst[0]), pstr, ",", SPLIT_IGNOR_LEN0);
            if(sizeof(dst)/sizeof(dst[0]) == ret)
            {
                dev_maths_str_trim(dst[0]);
                dev_maths_str_trim(dst[1]);
                dev_maths_str_trim(dst[2]);
                if(3 == dev_maths_asc_to_u32(dst[0], strlen(dst[0]), MODULUS_DEC))
                {
                    snprintf(device_info->m_bt_info.m_bt_name, sizeof(device_info->m_bt_info.m_bt_name), "%s", dst[1]);
                    snprintf(device_info->m_bt_info.m_bt_mac_addr, sizeof(device_info->m_bt_info.m_bt_mac_addr), "%s", dst[2]);
                }
                else if(2 == dev_maths_asc_to_u32(dst[0], strlen(dst[0]), MODULUS_DEC))
                {
                    snprintf(device_info->m_bt_info.m_bt_mac_addr, sizeof(device_info->m_bt_info.m_bt_mac_addr), "%s", dst[2]);
                }
                else if(1 == dev_maths_asc_to_u32(dst[0], strlen(dst[0]), MODULUS_DEC))
                {
                    snprintf(device_info->m_bt_info.m_bt_name, sizeof(device_info->m_bt_info.m_bt_name), "%s", dst[1]);
                }
            }
            break;
        }

        pstr = strstr(src, "BTOK");
        if(pstr)
        {
            if(1 == thread_node_param->m_lparam)
            {
                device_info->m_bt_info.m_bt_flag = TRUE;
                dev_wireless_thread_additional_set_result(WIRE_AT_RESULT_OK);
                ret = DDI_OK;
                break;
            }
            else
            {
                dev_wireless_thread_additional_set_result(WIRE_AT_RESULT_ERROR);
                ret = DDI_ERR;
                break;
            }
        }

        pstr = strstr(src, "BTERROR");
        if(pstr)
        {
            dev_wireless_thread_additional_set_result(WIRE_AT_RESULT_ERROR);
            ret = DDI_ERR;
            break;
        }

        if(WIRE_AT_RESULT_TIMEROUT == result)
        {
            dev_wireless_thread_additional_set_result(WIRE_AT_RESULT_TIMEROUT);
            ret = DDI_ERR;
            break;
        }
    }while(0);

    return ret;
}


s32 G500_bt_get_param(bt_param_t *param, AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;
    s8 buff[128];
    wireless_device_info_t *device_info = dev_wireless_get_device();
    wireless_thread_node_param_t *thread_node_param = dev_wireless_get_thread_additional_node_param();

    switch(at_api_get_type)
    {
        case AT_API_NO_BLOCK:
            break;

        case AT_API_BLOCK:
            snprintf(buff, sizeof(buff), "AT=AT+BTPARAMGET=%d", param->m_type);
            dev_wireless_send_at_cmd(buff, at_api_get_type, TRUE);
            dev_wireless_thread_run_switch(FALSE);
            dev_wireless_thread_additional_set_param(WS_TASK_GET_BTPARAM, G500_bt_get_param_callback, WIRE_AT_DEFAULT_TIMEOUT);
            dev_wireless_thread_run_switch(TRUE);
            while(1)
            {
                if(dev_wireless_thread_additional_task_is_change(WS_TASK_GET_BTPARAM))
                {
                    ret = DDI_EBUSY;
                    break;
                }

                if(dev_wireless_thread_additional_is_free())
                {
                    if(TRUE == device_info->m_bt_info.m_bt_flag)
                    {
                        device_info->m_bt_info.m_bt_flag = FALSE;
                        switch(param->m_type)
                        {
                            case 1:
                                snprintf(param->m_name, sizeof(param->m_name),"%s", device_info->m_bt_info.m_bt_name);
                                break;

                            case 2:
                                snprintf(param->m_addr, sizeof(param->m_addr),"%s", device_info->m_bt_info.m_bt_mac_addr);
                                break;

                            case 3:
                                snprintf(param->m_name, sizeof(param->m_name),"%s", device_info->m_bt_info.m_bt_name);
                                snprintf(param->m_addr, sizeof(param->m_addr),"%s", device_info->m_bt_info.m_bt_mac_addr);
                                break;
                        }
                        TRACE_INFO("btname:%s", param->m_name);
                        TRACE_INFO("btaddr:%s", param->m_addr);
                        ret = DDI_OK;
                    }
                    else
                    {
                        TRACE_INFO("get bt param fail!");
                        ret = DDI_ERR;
                    }
                    break;
                }

                ddi_misc_msleep(50);
            }
    }

    return ret;
}

static s32 G500_bt_socket_create_callback(u8 result, u8 *src, u16 srclen)
{
    s32 ret = DDI_ERR;
    s8 *pstr = NULL;
    s8 dst[2][SPLIT_PER_MAX] = {0};
    wireless_device_info_t *device_info = dev_wireless_get_device();
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_additional_node_param();

    TRACE_DBG("result:%d, src:%s", result, src);
    do{
        pstr = strstr(src, "BTOK");
        if(pstr)
        {
            TRACE_INFO("BTOK");
            dev_wireless_thread_additional_set_result(WIRE_AT_RESULT_OK);
            ret = DDI_OK;
            break;
        }

        pstr = strstr(src, "BTERROR");
        if(pstr)
        {
            TRACE_INFO("BTERROR");
            dev_wireless_thread_additional_set_result(WIRE_AT_RESULT_ERROR);
            ret = DDI_ERR;
            break;
        }

        /*pstr = strstr(src, "+BTSOCKETCREATE:");
        if(pstr)
        {
            dev_wireless_thread_additional_set_result(WIRE_AT_RESULT_ERROR);
            ret = DDI_ERR;
            break;
        }*/

        if(WIRE_AT_RESULT_TIMEROUT == result)
        {
            TRACE_INFO("TIMEOUT");
            dev_wireless_thread_additional_set_result(WIRE_AT_RESULT_TIMEROUT);
            ret = DDI_ERR;
            break;
        }
    }while(0);

    return ret;
}

s32 G500_bt_socket_create(s32 socket_id, SOCKET_TYPE type, u8 *param, u16 port, AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;
    s8 buff[128];
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_additional_node_param();
    wireless_device_info_t *device_info = dev_wireless_get_device();
    u8 typetmp;

    TRACE_INFO("enter");
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
            snprintf(buff, sizeof(buff), "AT=AT+BTSOCKETCREATE=%d,%d,%s,%d", socket_id, typetmp, param, port);
            dev_wireless_send_at_cmd(buff, at_api_get_type, TRUE);
            dev_wireless_thread_run_switch(FALSE);
            dev_wireless_thread_additional_set_param(WS_TASK_BT_CREAT_SOCKET, G500_bt_socket_create_callback, WIRE_AT_CMD_RESEND_TIMEOUT*3);
            dev_wireless_thread_run_switch(TRUE);
            while(1)
            {
                if(dev_wireless_thread_additional_task_is_change(WS_TASK_BT_CREAT_SOCKET))
                {
                    ret = DDI_EBUSY;
                    break;
                }

                if(dev_wireless_thread_additional_is_free())
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

    return ret;
}

s32 G500_bt_tcp_connect_report_handle(s8 *buff)
{
    s32 ret = DDI_ERR;
    s8 *pstr = NULL;
    wireless_device_info_t *wireless_device = dev_wireless_get_device();
    s8 dst[2][SPLIT_PER_MAX];
    s32 socket_sn = 0;

    do{
        pstr = strstr(buff, "+BTSOCKETSTATUSRP:");
        if(pstr)
        {
            pstr += strlen("+BTSOCKETSTATUSRP:");
            memset(dst, 0, sizeof(dst));
            ret = dev_maths_split(dst, sizeof(dst)/sizeof(dst[0]), pstr, ",", SPLIT_IGNOR_LEN0);
            if(sizeof(dst)/sizeof(dst[0]) == ret)
            {
                dev_maths_str_trim(dst[0]);
                dev_maths_str_trim(dst[1]);

                if(dev_maths_asc_to_u32(dst[0], strlen(dst[0]), MODULUS_DEC) > BT_SOCKET_MAX)
                {
                    TRACE_ERR("socket id is error:%d", dev_maths_asc_to_u32(dst[0], strlen(dst[0]), MODULUS_DEC));
                }
                else
                {
                    socket_sn = dev_maths_asc_to_u32(dst[0], strlen(dst[0]), MODULUS_DEC);
                    if(wireless_device->m_bt_info.m_socket_info[socket_sn])
                    {
                        ret = dev_maths_asc_to_u32(dst[1], strlen(dst[1]), MODULUS_DEC);
                        wireless_device->m_bt_info.m_socket_info[socket_sn]->m_status = (0 == ret)?GPRS_STATUS_CONNECTED:GPRS_STATUS_DISCONNECTED;
                        if(GPRS_STATUS_CONNECTED == wireless_device->m_bt_info.m_socket_info[socket_sn]->m_status)
                        {
                            TRACE_INFO("sn:%d connect success", socket_sn);
                        }
                        else
                        {
                            TRACE_INFO("sn:%d, status call back is %d",
                                                            socket_sn,
                                                            wireless_device->m_bt_info.m_socket_info[socket_sn]->m_status);
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

static s32 G500_bt_send_AT_MIPSEND(s32 id, u8 *wbuf, u32 wlen)
{
    u8 sendbuf[32];
    s32 i;
    s32 ret;
    s32 len = 0;

    if(wlen > WIRELESS_BT_SEND_DATA_BUFLEN)
    {
        TRACE_INFO("DDI_EDATA");
        return DDI_EDATA;
    }

    snprintf(sendbuf, sizeof(sendbuf), "AT=AT+BTSOCKETSEND=%d,%d,", id, wlen);
    ret = dev_wireless_send_at_cmd(sendbuf, AT_API_BLOCK, FALSE);

    for(i=0; i<wlen; i++)
    {
        memset(sendbuf, 0, sizeof(sendbuf));
        sprintf(sendbuf, "%02X", wbuf[i]);
        dev_wireless_send_at_cmd(sendbuf, AT_API_BLOCK, FALSE);
    }

    ret = dev_wireless_send_at_cmd(AT_CMD_RES_END, AT_API_BLOCK, FALSE);
    dev_wireless_thread_additional_set_timeout(WIRE_AT_TCP_SEND_TIMEOUT);
    return DDI_OK;
}

static s32 G500_bt_socket_send_onepacket_callback(u8 result, u8 *src, u16 srclen)
{
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_additional_node_param();
    wireless_device_info_t *device_info = dev_wireless_get_device();
    s32 ret = DDI_ERR;
    u8 sn;
    u8 *pstr = NULL;
    s8 dst[2][SPLIT_PER_MAX] = {0};

    TRACE_DBG("result=%d, src=%s", result, src);
    do{
        pstr = strstr(src, "+BTSOCKETSEND:");
        if(pstr && 1 == thread_node_param->m_lparam)
        {
            pstr += strlen("+BTSOCKETSEND:");
            memset(dst, 0, sizeof(dst));
            ret = dev_maths_split(dst, sizeof(dst)/sizeof(dst[0]), pstr, ",", SPLIT_IGNOR_LEN0);
            if(sizeof(dst)/sizeof(dst[0]) == ret)
            {
                dev_maths_str_trim(dst[0]);
                dev_maths_str_trim(dst[1]);
                ret = dev_maths_asc_to_u32(dst[1], strlen(dst[1]), MODULUS_DEC);
                if(0 == ret)
                {
                    device_info->m_bt_info.m_bt_flag = TRUE;
                    dev_wireless_thread_additional_set_result(WIRE_AT_RESULT_OK);
                    TRACE_INFO("Send buf success");
                    ret = DDI_OK;
                }
                else if(1 == ret)
                {
                    TRACE_INFO("bt socket not exist");
                    dev_wireless_thread_additional_set_result(WIRE_AT_RESULT_ERROR);
                    ret = DDI_ERR;
                }
                else
                {
                    TRACE_INFO("send socket fail");
                    dev_wireless_thread_additional_set_result(WIRE_AT_RESULT_ERROR);
                    ret = DDI_ERR;
                }
                break;
            }
        }

        pstr = strstr(src, "BTOK");
        if(pstr)
        {
            TRACE_INFO("BTOK");
            thread_node_param->m_lparam = 1;
            ret = DDI_OK;
            break;
        }

        pstr = strstr(src, "BTERROR");
        if(pstr)
        {
            TRACE_INFO("BTERROR");
            dev_wireless_thread_additional_set_result(WIRE_AT_RESULT_ERROR);
            ret = DDI_ERR;
            break;
        }

        if(WIRE_AT_RESULT_TIMEROUT == result)
        {
            TRACE_INFO("TIMEOUT");
            dev_wireless_thread_additional_set_result(WIRE_AT_RESULT_TIMEROUT);
            ret = DDI_ERR;
            break;
        }
    }while(0);

    return ret;
}

static s32 G500_bt_socket_send_onepacket(s32 socket_id, u8 *wbuf, s32 wlen)
{
    s32 ret = DDI_ERR;
    wireless_device_info_t *device_info = dev_wireless_get_device();
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_additional_node_param();

    dev_wireless_thread_run_switch(FALSE);
    dev_wireless_thread_additional_set_param(WS_TASK_BT_SOCKET_SENDBUFF, G500_bt_socket_send_onepacket_callback, WIRE_AT_TCP_SEND_TIMEOUT);
    dev_wireless_thread_run_switch(TRUE);
    G500_bt_send_AT_MIPSEND(socket_id, wbuf, wlen);

    while(1)
    {
        if(dev_wireless_thread_additional_task_is_change(WS_TASK_BT_SOCKET_SENDBUFF))
        {
            TRACE_INFO("DDI_EBUSY");
            ret = DDI_EBUSY;
            break;
        }

        if(dev_wireless_thread_additional_is_free())
        {
            if(TRUE == device_info->m_bt_info.m_bt_flag)
            {
                TRACE_INFO("send data success");
                device_info->m_bt_info.m_bt_flag = FALSE;
                ret = DDI_OK;
            }
            else
            {
                TRACE_INFO("send data fail");
                ret = DDI_ERR;
            }
            break;
        }

        dev_user_delay_ms(10);
    }

    return ret;
}

s32 G500_bt_socket_send(s32 socket_id, u8 *wbuf, s32 wlen, AT_API_GET_TYPE_E at_api_get_type)
{
    s32 i,j;
    s32 ret = 0;

    TRACE_INFO("send data");
    if(AT_API_BLOCK == at_api_get_type)
    {
        i = 0;
        while(i<wlen)
        {
            if((i+WIRELESS_BT_SEND_DATA_BUFLEN) > wlen)
            {
                j = wlen - i;
            }
            else
            {
                j = WIRELESS_BT_SEND_DATA_BUFLEN;
            }

            ret = G500_bt_socket_send_onepacket(socket_id, &wbuf[i], j);
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

static s32 G500_bt_socket_close_callback(u8 result, u8 *src, u16 srclen)
{
    s32 ret =DDI_ERR;
    s8 *pstr = NULL;
    s8 dst[2][SPLIT_PER_MAX] = {0};
    wireless_device_info_t *device_info = dev_wireless_get_device();
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_additional_node_param();

    TRACE_INFO("result:%d, src:%s", result, src);
    do{
        pstr = strstr(src, "BTOK");
        if(pstr)
        {
            TRACE_INFO("BTOK");
            thread_node_param->m_lparam = 1;
            ret = DDI_OK;
            break;
        }

        pstr = strstr(src, "BTERROR");
        if(pstr)
        {
            TRACE_INFO("BTERROR");
            dev_wireless_thread_additional_set_result(WIRE_AT_RESULT_ERROR);
            ret = DDI_ERR;
            break;
        }

        pstr = strstr(src, "+BTSOCKETCLOSE:");
        if(pstr)
        {
            pstr += strlen("+BTSOCKETCLOSE:");
            memset(dst, 0, sizeof(dst));
            ret = dev_maths_split(dst, sizeof(dst)/sizeof(dst[0]), pstr, ",", SPLIT_IGNOR_LEN0);
            if(sizeof(dst)/sizeof(dst[0]) == ret)
            {
                dev_maths_str_trim(dst[0]);
                dev_maths_str_trim(dst[1]);
                ret = dev_maths_asc_to_u32(dst[0], strlen(dst[0]), MODULUS_DEC);

                if(0 <= ret && ret < BT_SOCKET_MAX && 1 == thread_node_param->m_lparam)
                {
                    if(0 == dev_maths_asc_to_u32(dst[1], strlen(dst[1]), MODULUS_DEC))
                    {
                        device_info->m_bt_info.m_socket_info[ret]->m_status = dev_maths_asc_to_u32(dst[1], strlen(dst[1]), MODULUS_DEC);
                        device_info->m_bt_info.m_bt_flag = TRUE;
                        dev_wireless_thread_additional_set_result(WIRE_AT_RESULT_OK);
                        ret = DDI_OK;
                        break;
                    }
                    else if(1 == dev_maths_asc_to_u32(dst[1], strlen(dst[1]), MODULUS_DEC))
                    {
                        dev_wireless_thread_additional_set_result(WIRE_AT_RESULT_ERROR);
                        ret = DDI_ERR;
                        break;
                    }
                }
            }
        }

        if(WIRE_AT_RESULT_TIMEROUT == result)
        {
            TRACE_INFO("TIMEOUT");
            dev_wireless_thread_additional_set_result(WIRE_AT_RESULT_TIMEROUT);
            ret = DDI_ERR;
            break;
        }
    }while(0);

    return ret;
}

s32 G500_bt_socket_close(s32 socket_id, AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;
    s8 buff[128];
    wireless_device_info_t *device_info = dev_wireless_get_device();
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_additional_node_param();

    switch(at_api_get_type)
    {
        case AT_API_NO_BLOCK:
            break;

        case AT_API_BLOCK:
            memset(buff, 0, sizeof(buff));
            snprintf(buff, sizeof(buff), "AT=AT+BTSOCKETCLOSE=%d", socket_id);
            dev_wireless_send_at_cmd(buff, at_api_get_type, TRUE);
            dev_wireless_thread_run_switch(FALSE);
            dev_wireless_thread_additional_set_param(WS_TASK_BT_SOCKET_CLOSE, G500_bt_socket_close_callback, WIRE_BT_SOCKET_CLOSE_TIMEOUT);
            dev_wireless_thread_run_switch(TRUE);
            while(1)
            {
                if(dev_wireless_thread_additional_task_is_change(WS_TASK_BT_SOCKET_CLOSE))
                {
                    ret = DDI_EBUSY;
                    break;
                }

                if(dev_wireless_thread_additional_is_free())
                {
                    if(TRUE == device_info->m_bt_info.m_bt_flag)
                    {
                        device_info->m_bt_info.m_bt_flag = FALSE;
                        TRACE_INFO("bt socket close success!");
                        ret = DDI_OK;
                    }
                    else
                    {
                        TRACE_INFO("bt socket close fail!");
                        ret = DDI_ERR;
                    }
                    break;
                }

                ddi_misc_msleep(50);
            }
            break;
    }

    return ret;
}

#endif
