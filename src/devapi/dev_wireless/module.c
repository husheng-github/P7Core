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

/*****************************************************************************
*                                                                            *
*  @file     module.C                                                        *
*  @brief    模块接口层封装（module Interface layer）                                 *
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

#if(WIRELESS_EXIST == 1)
static wireless_module_t *gp_module = NULL;

/**
 * @brief 判断无线模块是否支持流控
 * @retval  TRUE  有流控
 * @retval  FALSE 没有流控
 * @retval  DDI_ERR 没有无线模块
 */
s32 MIL_get_flow_control_flag(void)
{
    s32 ret = DDI_ERR;

    do{
        if(!gp_module)
        {
            break;
        }

        ret = gp_module->m_with_flow_control;
    }while(0);

    return ret;
}

/**
 * @brief 设置硬件流控
 * @retval  DDI_OK  执行成功
 * @retval  DDI_ERR 执行失败
 */
s32 MIL_cfg_flow_control(AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;

    do{
        if(!gp_module)
        {
            break;
        }

        ret = gp_module->cfg_flow_control(at_api_get_type);
    }while(0);

    return ret;
}

/**
 * @brief 处理注网异常的主动上送
 * @param[in] buff：用来判断的字符串
 * @retval  DDI_OK  是此指令
 * @retval  DDI_ERR 不是此指令
 */
s32 MIL_net_reg_report_handle(s8 *buff)
{
    s32 ret = DDI_ERR;

    do{
        if(!gp_module)
        {
            break;
        }

        ret = gp_module->net_reg_report_handle(buff);
    }while(0);

    return ret;
}


/**
 * @brief 声音播报结束的上报
 * @param[in] buff：用来判断的字符串
 * @retval  DDI_OK  是此指令
 * @retval  DDI_ERR 不是此指令
 */
s32 MIL_audioend_report_handle(s8 *buff)
{
    s32 ret = DDI_ERR;

    do{
        if(!gp_module)
        {
            break;
        }

        ret = gp_module->audio_end_report_handle(buff);
    }while(0);
    
    return ret;
}


/**
 * @brief 处理pdp状态改变的主动上送
 * @param[in] buff：用来判断的字符串
 * @retval  DDI_OK  是此指令
 * @retval  DDI_ERR 不是此指令
 */
s32 MIL_pdp_report_handle(s8 *buff)
{
    s32 ret = DDI_ERR;

    do{
        if(!gp_module)
        {
            break;
        }

        ret = gp_module->pdp_report_handle(buff);
    }while(0);

    return ret;
}

/**
 * @brief 处理tcp三次握手的结果上送
 * @param[in] buff：用来判断的字符串
 * @retval  DDI_OK  是此指令
 * @retval  DDI_ERR 不是此指令
 */
s32 MIL_tcp_connect_report_handle(s8 *buff)
{
    s32 ret = DDI_ERR;

    do{
        if(!gp_module)
        {
            break;
        }

        ret = gp_module->tcp_connect_report_handle(buff);
    }while(0);

    return ret;
}

/**
 * @brief 处理tcp连接后状态改变的异常处理
 * @param[in] buff：用来判断的字符串
 * @retval  DDI_OK  是此指令
 * @retval  DDI_ERR 不是此指令
 */
s32 MIL_socket_report_handle(s8 *buff)
{
    s32 ret = DDI_ERR;

    do{
        if(!gp_module)
        {
            break;
        }

        ret = gp_module->socket_report_handle(buff);
    }while(0);

    return ret;
}

/**
 * @brief 获取模块imei的发送指令
 * @param[in] 无
 * @retval  指令
 */
s8 *MIL_get_imei_cmd(void)
{
    if(gp_module)
    {
        return gp_module->m_imei_cmd;
    }
    else
    {
        return NULL;
    }
}

/**
 * @brief 获取模块imei的返回处理函数，供模块控制策略里面调用
 * @param[in] result 结果
 * @param[in] src 结果对应的字符串
 * @retval  头部关键字
 */
s32 MIL_get_imei_callback(u8 result, u8 *src, u16 srclen)
{
    s32 ret = DDI_ERR;

    do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        ret = gp_module->get_imei_callback(result, src, srclen);
    }while(0);

    return ret;
}

/**
 * @brief 获取模块imei
 * @param[in] 无
 * @retval  头部关键字
 */
s32 MIL_get_imei(u8 *imei_buf, u32 buf_size, AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;

    do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        ret = gp_module->get_imei(imei_buf, buf_size, at_api_get_type);
    }while(0);

    return ret;
}

/**
 * @brief 获取模块网络制式的命令
 * @param[in] 无
 * @retval  指令
 */
s8 *MIL_get_check_work_mode_cmd(void)
{
    if(gp_module)
    {
        return gp_module->m_check_workmod_cmd;
    }
    else
    {
        return NULL;
    }
}

/**
 * @brief 检测网络制式
 * @param[in] at_api_get_type
 * @retval  TRUE:当前模块不支持，直接跳到下一条指令执行
 * @retval  DDI_OK:发送成功
 */
s32 MIL_check_work_mode(AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;

    do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        ret = gp_module->check_work_mode(at_api_get_type);
    }while(0);

    return ret;
}


/**
 * @brief 检测网络制式的回调处理函数
 * @param[in] result 结果
 * @param[in] src 结果对应的字符串
 * @retval  头部关键字
 */
s32 MIL_check_work_mode_callback(u8 result, u8 *src, u16 srclen)
{
    s32 ret = DDI_ERR;

    do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        ret = gp_module->check_work_mode_callback(result, src, srclen);
    }while(0);

    return ret;
}

/**
 * @brief 设置网络制式为LTE
 * @param[in] 无
 * @retval  指令
 */
s8 *MIL_get_set_work_mode_cmd(void)
{
    if(gp_module)
    {
        return gp_module->m_set_workmod_cmd;
    }
    else
    {
        return NULL;
    }
}

/**
 * @brief 设置网络制式
 * @param[in] 无
 * @retval  头部关键字
 */
s32 MIL_set_work_mode(AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;

    do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        ret = gp_module->set_work_mode(at_api_get_type);
    }while(0);

    return ret;
}

/**
 * @brief 获取rplmn的at指令
 * @param[in] 无
 * @retval  指令
 */
s8 *MIL_check_rplmn_cmd(void)
{
    if(gp_module)
    {
        return gp_module->m_check_rplmn_cmd;
    }
    else
    {
        return NULL;
    }
}

/**
 * @brief 确认rplmn是否是公网参数
 * @param[in] 无
 * @retval  TRUE 是
 * @retval  FALSE 否
 */
s32 MIL_check_rplmn_callback(u8 result, u8 *src, u16 srclen)
{
    s32 ret = DDI_ERR;

    do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        ret = gp_module->check_rplmn_callback(result, src, srclen);
    }while(0);

    return ret;
}

/**
 * @brief 确认rplmn是否是公网参数
 * @param[in] 无
 * @retval  DDI_OK 发送成功
 * @retval  <0:失败
 */
s32 MIL_check_rplmn(AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;

    do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        ret = gp_module->check_rplmn(at_api_get_type);
    }while(0);

    return ret;
}

/**
 * @brief 获取清除rplmn的at指令
 * @param[in] 无
 * @retval  指令
 */
s8 *MIL_clear_rplmn_cmd(void)
{
    if(gp_module)
    {
        return gp_module->m_clear_rplmn_cmd;
    }
    else
    {
        return NULL;
    }
}

/**
 * @brief 清除rplmn参数
 * @param[in] at_api_get_type
 * @retval  DDI_OK 发送成功
 * @retval  <0:失败
 */
s32 MIL_clear_rplmn(AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;

    do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        ret = gp_module->clear_rplmn(at_api_get_type);
    }while(0);

    return ret;
}


/**
 * @brief 获取模块sim卡状态的发送指令
 * @param[in] 无
 * @retval  指令
 */
s8 *MIL_simcard_state_cmd(void)
{
    if(gp_module)
    {
        return gp_module->m_simcard_state_cmd;
    }
    else
    {
        return NULL;
    }
}

/**
 * @brief 获取模块imei的返回处理函数，供模块控制策略里面调用
 * @param[in] result 结果
 * @param[in] src 结果对应的字符串
 * @retval  WITH_SIMCARD  有sim卡
 * @retval  NOWITH_SIMCARD  没有sim卡
 * @retval  < 0 执行失败
 */
s32 MIL_get_simcard_state_callback(u8 result, u8 *src, u16 srclen)
{
    s32 ret = DDI_ERR;

    do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        ret = gp_module->get_simcard_state_callback(result, src, srclen);
    }while(0);

    return ret;
}

s32 MIL_get_simcard_state(AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;

    do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        ret = gp_module->get_simcard_state(at_api_get_type);
    }while(0);

    return ret;
}

/**
 * @brief 获取模块ccid的发送指令
 * @param[in] 无
 * @retval  指令
 */
s8 *MIL_ccid_cmd(void)
{
    if(gp_module)
    {
        return gp_module->m_ccid_cmd;
    }
    else
    {
        return NULL;
    }
}

/**
 * @brief 获取模块ccid的返回处理函数，供模块控制策略里面调用
 * @param[in] result 结果
 * @param[in] src 结果对应的字符串
 * @retval  WITH_SIMCARD  有sim卡
 * @retval  NOWITH_SIMCARD  没有sim卡
 * @retval  < 0 执行失败
 */
s32 MIL_get_ccid_callback(u8 result, u8 *src, u16 srclen)
{
    s32 ret = DDI_ERR;

    do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        ret = gp_module->get_ccid_callback(result, src, srclen);
    }while(0);

    return ret;
}

s32 MIL_get_ccid(u8 *buf, u32 buf_size, AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;

    do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        ret = gp_module->get_ccid(buf, buf_size, at_api_get_type);
    }while(0);

    return ret;
}

/**
 * @brief 获取模块信号强度的发送指令
 * @param[in] 无
 * @retval  指令
 */
s8 *MIL_get_csq_cmd(void)
{
    if(gp_module)
    {
        return gp_module->m_get_csq_cmd;
    }
    else
    {
        return NULL;
    }
}

/**
 * @brief 获取模块ccid的返回处理函数，供模块控制策略里面调用
 * @param[in] result 结果
 * @param[in] src 结果对应的字符串
 * @retval  WITH_SIMCARD  有sim卡
 * @retval  NOWITH_SIMCARD  没有sim卡
 * @retval  < 0 执行失败
 */
s32 MIL_get_csq_callback(u8 result, u8 *src, u16 srclen)
{
    s32 ret = DDI_ERR;

    do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        ret = gp_module->get_csq_callback(result, src, srclen);
    }while(0);

    return ret;
}

/**
 * @brief 获取信号强度
 * @param[in] 无
 * @retval  头部关键字
 */
s32 MIL_get_csq(u8 *csq, u16 *bear_rate, AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;

    do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        ret = gp_module->get_csq(csq, bear_rate, at_api_get_type);
    }while(0);

    return ret;
}

/**
 * @brief 获取注网状态的发送指令
 * @param[in] 无
 * @retval  指令
 */
s8 *MIL_get_net_reg_cmd(void)
{
    if(gp_module)
    {
        return gp_module->get_reg_cmd();
    }
    else
    {
        return NULL;
    }
}

/**
 * @brief 获取模块ccid的返回处理函数，供模块控制策略里面调用
 * @param[in] result 结果
 * @param[in] src 结果对应的字符串
 * @retval  WITH_SIMCARD  有sim卡
 * @retval  NOWITH_SIMCARD  没有sim卡
 * @retval  < 0 执行失败
 */
s32 MIL_get_net_reg_callback(u8 result, u8 *src, u16 srclen)
{
    s32 ret = DDI_ERR;

    do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        ret = gp_module->get_net_reg_callback(result, src, srclen);
    }while(0);

    return ret;
}

/**
 * @brief 获取信号强度
 * @param[in] 无
 * @retval  头部关键字
 */
s32 MIL_get_net_reg(AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;

    do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        ret = gp_module->get_net_reg(at_api_get_type);
    }while(0);

    return ret;
}

/**
 * @brief 获取注网状态的发送指令
 * @param[in] 无
 * @retval  指令
 */
s8 *MIL_get_set_reg_param_cmd(void)
{
    if(gp_module)
    {
        return gp_module->get_set_reg_param_cmd();
    }
    else
    {
        return NULL;
    }
}

/**
 * @brief 获取信号强度
 * @param[in] 无
 * @retval  头部关键字
 */
s32 MIL_set_reg_param(AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;

    do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        ret = gp_module->set_reg_param(at_api_get_type);
    }while(0);

    return ret;
}

/**
 * @brief 获取imsi的发送指令
 * @param[in] 无
 * @retval  指令
 */
s8 *MIL_get_imsi_cmd(void)
{
    if(gp_module)
    {
        return gp_module->m_get_imsi_cmd;
    }
    else
    {
        return NULL;
    }
}

/**
 * @brief 获取模块imsi的返回处理函数，供模块控制策略里面调用
 * @param[in] result 结果
 * @param[in] src 结果对应的字符串
 * @retval  WITH_SIMCARD  有sim卡
 * @retval  NOWITH_SIMCARD  没有sim卡
 * @retval  < 0 执行失败
 */
s32 MIL_get_imsi_callback(u8 result, u8 *src, u16 srclen)
{
    s32 ret = DDI_ERR;

    do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        ret = gp_module->get_imsi_callback(result, src, srclen);
    }while(0);

    return ret;
}

/**
 * @brief 获取imsi
 * @param[in] 无
 * @retval  头部关键字
 */
s32 MIL_get_imsi(u8 *buf, u32 buf_size, AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;

    do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        ret = gp_module->get_imsi(buf, buf_size, at_api_get_type);
    }while(0);

    return ret;
}

/**
 * @brief 获取基站信息
 * @param[in] basestation_info 用来填充基站信息的缓存
 * @param[in] cell_num 缓存的buff数量， =1 获取主基站 >1获取周边基站
 * @param[in] at_api_get_type
 * @retval  DDI_OK 成功
 * @retval  DDI_ERR 出错
 * @retval  DDI_ETIMEOUT 超时
 */
s32 MIL_get_cellinfo(strBasestationInfo *basestation_info, s32 cell_num, AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;

    do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        ret = gp_module->get_cellinfo(basestation_info, cell_num, at_api_get_type);
    }while(0);

    return ret;
}

/**
 * @brief 打开飞行模式
 * @param[in] at_api_get_type
 * @retval  DDI_OK 成功
 * @retval  DDI_ERR 出错
 * @retval  DDI_ETIMEOUT 超时
 */
s32 MIL_open_airplane(AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;

    do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        ret = gp_module->open_airplane(at_api_get_type);
    }while(0);

    return ret;
}

/**
 * @brief 关闭飞行模式
 * @param[in] at_api_get_type
 * @retval  DDI_OK 成功
 * @retval  DDI_ERR 出错
 * @retval  DDI_ETIMEOUT 超时
 */
s32 MIL_close_airplane(AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;

    do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        ret = gp_module->close_airplane(at_api_get_type);
    }while(0);

    return ret;
}

/**
 * @brief 电话拨号
 * @param[in] num:电话号码
 * @param[in] at_api_get_type
 * @retval  DDI_OK 成功
 * @retval  DDI_ERR 出错
 * @retval  DDI_ETIMEOUT 超时
 */
s32 MIL_telephone_dial(u8 *num, AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;

    do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        ret = gp_module->telephone_dial(num, at_api_get_type);
    }while(0);

    return ret;
}

/**
 * @brief 电话挂断
 * @param[in] at_api_get_type
 * @retval  DDI_OK 成功
 * @retval  DDI_ERR 出错
 * @retval  DDI_ETIMEOUT 超时
 */
s32 MIL_telephone_hangup(AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;

    do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        ret = gp_module->telephone_hangup(at_api_get_type);
    }while(0);

    return ret;
}

/**
 * @brief 配置ntp服务器
 * @param[in] ntp_addr:ntp服务器
 * @param[in] ntp_port:ntp服务器端口
 * @param[in] at_api_get_type
 * @retval  DDI_OK 成功
 * @retval  DDI_ERR 出错
 * @retval  DDI_ETIMEOUT 超时
 */
s32 MIL_cfg_ntp_server(s8 *ntp_addr, s32 ntp_port, AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;

    do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        ret = gp_module->cfg_ntp_server(ntp_addr, ntp_port, at_api_get_type);
    }while(0);

    return ret;
}

/**
 * @brief 获取无线模块的rtc
 * @param[in] ntp_addr:ntp服务器
 * @param[in] ntp_port:ntp服务器端口
 * @param[in] at_api_get_type
 * @retval  DDI_OK 成功
 * @retval  DDI_ERR 出错
 * @retval  DDI_ETIMEOUT 超时
 */
s32 MIL_qry_rtc(s8 *rtc_buff, s32 rtc_buff_len, AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;

    do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        ret = gp_module->qry_rtc(rtc_buff, rtc_buff_len, at_api_get_type);
    }while(0);

    return ret;
}

/**
 * @brief 获取模块接收数据格式配置的指令
 * @param[in] 无
 * @retval  指令
 */
s8 *MIL_get_cfg_rec_data_format_cmd(void)
{
    if(gp_module)
    {
        return gp_module->m_cfg_rec_data_format_cmd;
    }
    else
    {
        return NULL;
    }
}

/**
 * @brief 配置接收数据的格式的返回处理函数，供模块控制策略里面调用
 * @param[in] result 结果
 * @param[in] src 结果对应的字符串
 * @retval  WITH_SIMCARD  有sim卡
 * @retval  NOWITH_SIMCARD  没有sim卡
 * @retval  < 0 执行失败
 */
s32 MIL_cfg_rec_data_format_callback(u8 result, u8 *src, u16 srclen)
{
    s32 ret = DDI_ERR;

    do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        ret = gp_module->cfg_rec_data_format_callback(result, src, srclen);
    }while(0);

    return ret;
}

/**
 * @brief 配置接收数据的格式
 * @param[in] 无
 * @retval  DDI_OK  执行成功
 */
s32 MIL_cfg_rec_data_format(AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;

    do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        ret = gp_module->cfg_rec_data_format(at_api_get_type);
    }while(0);

    return ret;
}

/**
 * @brief 获取本地ip命令
 * @param[in] 无
 * @retval  指令
 */
s8 *MIL_get_local_ip_cmd(void)
{
    if(gp_module)
    {
        return gp_module->m_get_local_ip_cmd;
    }
    else
    {
        return NULL;
    }
}

/**
 * @brief 获取本地ip返回执行函数
 * @param[in] result 结果
 * @param[in] src 结果对应的字符串
 * @retval  TRUE IP地址存在
 * @retval  FALSE IP地址不存在
 * @retval  < 0 执行失败
 */
s32 MIL_get_local_ip_callback(u8 result, u8 *src, u16 srclen)
{
    s32 ret = DDI_ERR;

    do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        ret = gp_module->get_local_ip_callback(result, src, srclen);
    }while(0);

    return ret;
}

/**
 * @brief 获取本地ip
 * @param[in] 无
 * @retval  DDI_OK  执行成功
 */
s32 MIL_get_local_ip(s8 *buf, s32 buf_size, AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;

    do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        ret = gp_module->get_local_ip(buf, buf_size, at_api_get_type);
    }while(0);

    return ret;
}

/**
 * @brief 获取设置apn指令
 * @param[in] 无
 * @retval  指令
 */
s8 *MIL_get_set_apn_cmd(void)
{
    if(gp_module)
    {
        return gp_module->get_set_apn_cmd();
    }
    else
    {
        return NULL;
    }
}

/**
 * @brief 设置apn返回执行函数
 * @param[in] result 结果
 * @param[in] src 结果对应的字符串
 * @retval  WITH_SIMCARD  有sim卡
 * @retval  NOWITH_SIMCARD  没有sim卡
 * @retval  < 0 执行失败
 */
s32 MIL_set_apn_callback(u8 result, u8 *src, u16 srclen)
{
    s32 ret = DDI_ERR;

    do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        ret = gp_module->set_apn_callback(result, src, srclen);
    }while(0);

    return ret;
}

/**
 * @brief 设置apn
 * @param[in] 无
 * @retval  指令
 */
s32 MIL_set_apn(s8 *apn, s8 *username, s8 *password, AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;

    do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        ret = gp_module->set_apn(apn, username, password, at_api_get_type);
    }while(0);

    return ret;
}

/**
 * @brief 去激活网络
 * @param[in] 无
 * @retval  指令
 */
s8 *MIL_get_inactive_pdp_cmd(void)
{
    s32 ret = DDI_ERR;

    if(!gp_module)
    {
        return NULL;
    }
    else
    {
        return gp_module->get_inactive_pdp_cmd;
    }
}

/**
 * @brief 去激活网络
 * @param[in] 无
 * @retval  指令
 */
s32 MIL_inactive_pdp(AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;

    do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        MODULE_FUNC_IS_NULL(gp_module->inactive_pdp);
        
        ret = gp_module->inactive_pdp(at_api_get_type);
    }while(0);

    return ret;
}

/**
 * @brief 创建socket
 * @param[in] socket_id:让模块使用的套接字id
 * @param[in] type
 * @param[in] param:服务器ip或域名
 * @param[in] port:服务器端口
 * @param[in] at_api_get_type:阻塞或非阻塞访问
 * @retval  DDI_OK:创建成功
 * @retval  DDI_ERR:创建失败
 * @retval  DDI_ETIMEOUT:创建超时
 */
s32 MIL_socket_create(s32 socket_id, SOCKET_TYPE type, u8 *param, u16 port, AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;
    
    do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        MODULE_FUNC_IS_NULL(gp_module->socket_create);
        
        ret = gp_module->socket_create(socket_id, type, param, port, at_api_get_type);
    }while(0);

    return ret;
}

/**
 * @brief 获取模块端可用的套接字id
 * @param[in] sock_list:用来填充的套接字buff
 * @param[in] socket_list_num:buff个数
 * @param[in] at_api_get_type:
 * @retval DDI_OK 获取成功
 * @retval DDI_ERR 获取失败
 */
s32 MIL_get_valid_socketid_list(s32 *sock_list, s32 socket_list_num, AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;
    
    do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        MODULE_FUNC_IS_NULL(gp_module->get_valid_socketid_list);

        ret = gp_module->get_valid_socketid_list(sock_list, socket_list_num, at_api_get_type);
    }while(0);

    return ret;
}

/**
 * @brief 发送socket数据
 * @param[in] socket_id:套接字id
 * @param[in] buff:要发送的数据
 * @param[in] len:要发送的数据长度
 * @param[in] at_api_get_type
 * @retval  发送的数据长度
 * @ <0  执行出错
 */
s32 MIL_socket_send(s32 socket_id, const u8 *buff, s32 len, AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;
    
    do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        MODULE_FUNC_IS_NULL(gp_module->socket_send);
        
        ret = gp_module->socket_send(socket_id, buff, len, at_api_get_type);
    }while(0);

    return ret;
}

/**
 * @brief 关闭socket
 * @param[in] socket_id:让模块关闭的套接字id
 * @retval  DDI_OK:创建成功
 * @retval  DDI_ERR:创建失败
 * @retval  DDI_ETIMEOUT:创建超时
 */
s32 MIL_socket_close(s32 socket_id, AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;
    
    do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        MODULE_FUNC_IS_NULL(gp_module->socket_close);
        
        ret = gp_module->socket_close(socket_id, at_api_get_type);
    }while(0);

    return ret;
}

/**
 * @brief 模块没找到使用的通用关机流程回调函数
 * @param[in] result:
 * @param[in] src:
 * @param[in] srclen:
 * @retval  DDI_OK:第一步操作成功
 * @retval  DDI_ETIMEOUT:第二步操作完成，可执行关机或复位操作
 */
static s32 general_power_off_callback(u8 result, u8 *src, u16 srclen)
{
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();
    s32 ret = DDI_ERR;
    s8 *pstr = NULL;

    TRACE_DBG("result:%d", result);
    switch(result)
    {
        case WIRE_AT_RESULT_TIMEROUT:
            if(0 == thread_node_param->m_lparam) //第一次超时
            {
                if(WIRELESS_WITH_POWERKEY)   //有电源控制引脚
                {
                    TRACE_ERR("[have power] finish power off gpio step 1");
                    dev_gpio_set_value(GPRS_POWERON_PIN, 1);
                    dev_wireless_thread_set_timeout(WIRE_AT_CMD_RESEND_TIMEOUT*3);
                    thread_node_param->m_lparam = 1;
                    ret = DDI_OK;
                }
                else  //没有电源控制脚，直接下电
                {
                    TRACE_ERR("finish power off gpio");
                    ret = DDI_ETIMEOUT;
                    dev_wireless_thread_set_result(result);
                }
            }
            else if(1 == thread_node_param->m_lparam) //第二次超时--有powerkey的第二次处理
            {
                TRACE_ERR("[have power] finish power off gpio step 2");
                ret = DDI_ETIMEOUT;
                dev_wireless_thread_set_result(result);
                dev_gpio_set_value(GPRS_POWEREN_PIN, 0);
                dev_wireless_set_power_flag(FALSE);
            }
            break;
    }

    return ret;
}

/**
 * @brief 模块没找到使用的通用关机函数
 * @param[in] result:
 * @param[in] src:
 * @param[in] srclen:
 * @retval  DDI_OK:第一步操作成功
 * @retval  DDI_ETIMEOUT:第二步操作完成，可执行关机或复位操作
 */
static s32 general_power_off(AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;
    s8 buff[128];
    wireless_thread_node_param_t *thread_node_param = (wireless_thread_node_param_t *)dev_wireless_get_thread_node_param();

    switch(at_api_get_type)
    {
        case AT_API_BLOCK:
            dev_wireless_thread_run_switch(TRUE);
            dev_wireless_thread_set_param(WS_TASK_POWEROFF_MODULE, general_power_off_callback, WIRE_AT_CMD_RESEND_TIMEOUT);
            
        case AT_API_NO_BLOCK:
            if(WIRELESS_WITH_POWERKEY)   //有电源控制引脚
            {
                dev_gpio_set_value(GPRS_POWERON_PIN, 0);
                dev_wireless_thread_set_timeout(WIRE_AT_CMD_RESEND_TIMEOUT);
            }
            else  //没有电源控制脚，直接下电
            {
                dev_gpio_set_value(GPRS_POWEREN_PIN, 0);
                dev_wireless_set_power_flag(FALSE);
                dev_wireless_thread_set_timeout(WIRE_AT_CMD_RESEND_TIMEOUT);
            }
            break;
    }

    if(AT_API_BLOCK == at_api_get_type)
    {
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
    }
    return ret;
}

/**
 * @brief 模快下电返回处理函数
 * @param[in] result
 * @param[in] src：结果字符串
 * @param[in] srclen：结果字符串长度
 * @retval DDI_OK 不能执行关机
 * @retval DDI_ETIMEOUT 能执行关机
 */
s32 MIL_power_off_callback(u8 result, u8 *src, u16 srclen)
{
    s32 ret = DDI_ERR;
    
    do{
        if(!gp_module)
        {
            ret = general_power_off_callback(result, src, srclen);
            break;
        }

        MODULE_FUNC_IS_NULL(gp_module->power_off_callback);
        
        ret = gp_module->power_off_callback(result, src, srclen);
    }while(0);

    TRACE_DBG("ret:%d", ret);
    return ret;
}

/**
 * @brief 模快下电
 * @param[in] at_api_get_type
 * @retval DDI_OK  (AT_API_BLOCK 返回此结果可关机)
 * @retval < 0执行失败
 */
s32 MIL_power_off(AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;
    
    do{
        if(!gp_module)
        {
            ret = general_power_off(at_api_get_type);
            break;
        }

        MODULE_FUNC_IS_NULL(gp_module->power_off);
        
        ret = gp_module->power_off(at_api_get_type);
    }while(0);

    return ret;
}
/**
 * @brief 配置GPIO
 * @param[in] certtype:0:TRUSTFILE    1:CERTFILE  2. KEYFILE
 * @param[in] 
 * @retval  DDI_OK 成功
 * @retval  DDI_ERR 失败
 */
s32 MIL_cfg_exGPIO(u8 exGPIO, u8 cfg, AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;

    do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        MODULE_FUNC_IS_NULL(gp_module->cfg_exGPIO);
        
        ret = gp_module->cfg_exGPIO(exGPIO, cfg, at_api_get_type);
    }while(0);

    return ret;

}


s32 MIL_set_exGPIO(u8 exGPIO, u8 value, AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;

    do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        MODULE_FUNC_IS_NULL(gp_module->set_exGPIO);
        
        ret = gp_module->set_exGPIO(exGPIO, value, at_api_get_type);
    }while(0);

    return ret;

}
s32 MIL_play_audio(u8 type, AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;

    do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        MODULE_FUNC_IS_NULL(gp_module->play_audio);
        
        ret = gp_module->play_audio(type, at_api_get_type);
    }while(0);

    return ret;

}

s32 MIL_get_appver_callback(u8 result, u8 *src, u16 srclen)
{
    s32 ret = DDI_ERR;

    do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        MODULE_FUNC_IS_NULL(gp_module->get_appver_callback);
        
        ret = gp_module->get_appver_callback(result, src, srclen);
    }while(0);

    return ret;
}

s32 MIL_get_appver(AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;

    do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        MODULE_FUNC_IS_NULL(gp_module->get_appver);
        
        ret = gp_module->get_appver(at_api_get_type);
    }while(0);

    return ret;

}
s32 MIL_suspend(AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;

    do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        MODULE_FUNC_IS_NULL(gp_module->suspend);
        
        ret = gp_module->suspend(at_api_get_type);
    }while(0);

    return ret;
}

s32 MIL_resume(AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;

    do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        MODULE_FUNC_IS_NULL(gp_module->resume);
        
        ret = gp_module->resume(at_api_get_type);
    }while(0);

    return ret;
}

s32 MIL_ssl_set_mod(SSL_AUTHEN_MOD_t mod, AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;

    do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        MODULE_FUNC_IS_NULL(gp_module->ssl_set_mod);
        
        ret = gp_module->ssl_set_mod(mod, at_api_get_type);
    }while(0);

    return ret;

}
s32 MIL_ssl_setver(u8 ver, AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;

    do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        MODULE_FUNC_IS_NULL(gp_module->ssl_setver);
        
        ret = gp_module->ssl_setver(ver, at_api_get_type);
    }while(0);

    return ret;

}

s32 MIL_ssl_filte(s32 type, AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;

    do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }
        MODULE_FUNC_IS_NULL(gp_module->ssl_filte);
        ret = gp_module->ssl_filte(type, at_api_get_type);
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
s32 MIL_ssl_setcert(u8 certtype, u8 *cert, u32 certlen, AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;

    do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        MODULE_FUNC_IS_NULL(gp_module->ssl_setcert);
        
        ret = gp_module->ssl_setcert(certtype, cert, certlen, at_api_get_type);
    }while(0);

    return ret;

}
#if 0
/**
 * @brief openSSL
 * @param[in] certtype:0:TRUSTFILE    1:CERTFILE  2. KEYFILE
 * @param[in] 
 * @retval  DDI_OK 成功
 * @retval  DDI_ERR 失败
 */
s32 MIL_ssl_open(s32 socketid)
{
    s32 ret = DDI_ERR;
    s32 sn;
    

    do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }
        ret = gp_module->ssl_setcert(certtype, cert, certlen, at_api_get_type);
    }while(0);

    return ret;

}
#endif

/**
 * @brief 获取tcp接收数据命令的头部关键字
 * @param[in] 无
 * @retval  头部关键字
 */
s8 *MIL_get_tcp_recv_cmd_head_kw(void)
{
    if(gp_module)
    {
        return gp_module->m_tcp_rec_cmd_head_kw;
    }
    else
    {
        return NULL;
    }
}


#if(WIRELESS_USE_EXTERANL_AMP == TRUE)
s32 MIL_ttsaudio_play(u8 ttsencode, u8 *ttstext, u32 ttstextlen)
{
   s32 ret = DDI_ERR;

   do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        MODULE_FUNC_IS_NULL(gp_module->ttsaudio_play);
        
        ret = gp_module->ttsaudio_play(ttsencode, ttstext, ttstextlen);
   }while(0);

   return ret;
}

s32 MIL_get_audio_file_list(s8 filelist[][WIRELESS_AUDIO_FILE_NAME_MAX], s32 filelist_num)
{
   s32 ret = DDI_ERR;

   do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        MODULE_FUNC_IS_NULL(gp_module->audio_get_audio_file_list);
        
        ret = gp_module->audio_get_audio_file_list(filelist, filelist_num);
   }while(0);

   return ret;
}

s32 MIL_delete_audio_file(s8 *deletefilename)
{
   s32 ret = DDI_ERR;

   do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        MODULE_FUNC_IS_NULL(gp_module->delete_audio_file);
        
        ret = gp_module->delete_audio_file(deletefilename);
   }while(0);

   return ret;
}

s32 MIL_add_audio_file(s32 file_index)
{
   s32 ret = DDI_ERR;

   do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        MODULE_FUNC_IS_NULL(gp_module->add_audio_file);
        
        ret = gp_module->add_audio_file(file_index);
   }while(0);

   return ret;
}

s32 MIL_get_audio_volume(u32 *audiovolume)
{
   s32 ret = DDI_ERR;

   do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        MODULE_FUNC_IS_NULL(gp_module->get_audio_volume);
        
        ret = gp_module->get_audio_volume(audiovolume);
   }while(0);

   return ret;
}


s32 MIL_set_audio_volume(u32 audiovolume, AT_API_GET_TYPE_E at_api_get_type)
{
   s32 ret = DDI_ERR;

   do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        MODULE_FUNC_IS_NULL(gp_module->set_audio_volume);
        
        ret = gp_module->set_audio_volume(audiovolume, at_api_get_type);
   }while(0);

   return ret;
}


s32 MIL_audiofile_play(s8 *audio_string)
{
   s32 ret = DDI_ERR;

   do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        MODULE_FUNC_IS_NULL(gp_module->audiofile_play);
        
        ret = gp_module->audiofile_play(audio_string);
   }while(0);

   return ret;
}

/**
 * @brief 获取配置外部功放控制GPIO
 * @param[in] exGPIO - 外部功放脚
 * @param[in] 
 * @retval  DDI_OK 成功
 * @retval  DDI_ERR 失败
 */
s8* MIL_get_set_useextamp_cmd(void)
{
    if(gp_module)
    {
       return gp_module->m_set_useextamp_cmd;
    }
    else
    {
        return NULL;
    }
}

/**
 * @brief 配置外部功放控制GPIO
 * @param[in] exGPIO - 外部功放脚
 * @param[in] 
 * @retval  DDI_OK 成功
 * @retval  DDI_ERR 失败
 */
s32 MIL_set_useextamp(AT_API_GET_TYPE_E at_api_get_type)
{
   s32 ret = DDI_ERR;

   do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        MODULE_FUNC_IS_NULL(gp_module->set_useextamp);
        
        ret = gp_module->set_useextamp(at_api_get_type);
   }while(0);

   return ret;
}

s8* MIL_get_cfg_audio_play_path_cmd(void)
{
    if(gp_module)
    {
        return gp_module->m_cfg_audio_play_path_cmd;
    }
    else
    {
        return NULL;
    }
}

/**
 * @brief 配置无线模块上喇叭播放的通道为speaker
 * @param[in] 
 * @retval  DDI_OK 成功
 * @retval  DDI_ERR 失败
 */
s32 MIL_cfg_audio_play_path(void)
{
   s32 ret = DDI_ERR;

   do{
        
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        MODULE_FUNC_IS_NULL(gp_module->cfg_audio_play_path);
        
        ret = gp_module->cfg_audio_play_path(AT_API_NO_BLOCK);
   }while(0);

   return ret;
}

s8 *MIL_get_cfg_audio_mut(void)
{
    if(gp_module)
    {
        return gp_module->m_cfg_audio_mut_cmd;
    }
    else
    {
        return NULL;
    }
}

/**
 * @brief 配置无线模块上喇叭通道麦克风静音、speaker非静音
 * @param[in] 
 * @retval  DDI_OK 成功
 * @retval  DDI_ERR 失败
 */
s32 MIL_cfg_audio_mut(void)
{
   s32 ret = DDI_ERR;

   do{
        
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        MODULE_FUNC_IS_NULL(gp_module->cfg_audio_mut);
        
        ret = gp_module->cfg_audio_mut(AT_API_NO_BLOCK);
   }while(0);

   return ret;
}

/**
 * @brief 关闭无线模块上的播放延时
 * @param[in] 
 * @retval  DDI_OK 成功
 * @retval  DDI_ERR 失败
 */
s32 MIL_close_audio_play_sleep(void)
{
   s32 ret = DDI_ERR;

   do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        MODULE_FUNC_IS_NULL(gp_module->close_audio_play_sleep);
        
        ret = gp_module->close_audio_play_sleep();
   }while(0);

   return ret;
}

/**
 * @brief 获取关闭无线模块上的播放延时指令
 * @param[in] 
 * @retval  
 */
u8* MIL_get_close_audio_play_sleep_cmd(void)
{
    if(gp_module)
    {
        return gp_module->m_close_audio_play_sleep_cmd;
    }
    else
    {
        return NULL;
    }
}
#endif


#if(WIRELESS_USE_UART_RING == TRUE)
s8* MIL_get_uartring_mode_cmd(void)
{
    if(gp_module)
    {
       return gp_module->m_uartring_mode_cmd;
    }
    else
    {
        return NULL;
    }
}

s32 MIL_set_useuartring(AT_API_GET_TYPE_E at_api_get_type)
{
    
   s32 ret = DDI_ERR;

   do{
        
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        MODULE_FUNC_IS_NULL(gp_module->uartring_mode);
        
        ret = gp_module->uartring_mode(at_api_get_type);
   }while(0);

   return ret;
    
}

s8* MIL_get_set_csclk_cmd(void)
{
    if(gp_module)
    {
       return gp_module->m_set_csclk_cmd;
    }
    else
    {
        return NULL;
    }
}

s32 MIL_set_csclk(AT_API_GET_TYPE_E at_api_get_type)
{
   s32 ret = DDI_ERR;

   do{
        
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        MODULE_FUNC_IS_NULL(gp_module->set_csclk);
        
        ret = gp_module->set_csclk(at_api_get_type);
       
   }while(0);

   return ret;
   
}

s8* MIL_get_gtlpm_mode_cmd(void)
{
    if(gp_module)
    {
       return gp_module->m_gtlpm_mode_cmd;
    }
    else
    {
        return NULL;
    }
}

s32 MIL_set_gtlpm_mode(AT_API_GET_TYPE_E at_api_get_type)
{
    s32 ret = DDI_ERR;

    do{
        if(!gp_module)
        {
            
            ret = DDI_ENODEV;
            break;
            
        }

        MODULE_FUNC_IS_NULL(gp_module->set_gtlpm_mode);
        
        ret = gp_module->set_gtlpm_mode(at_api_get_type);
    }while(0);

    return ret;
}
#endif

//设置FOTA升级参数
s32 MIL_setfota_upgrade_param(s8 fota_type, s8 *url, s8 *filename,s8 *username, s8 *password)
{
    
   s32 ret = DDI_ERR;

   do{
        
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        MODULE_FUNC_IS_NULL(gp_module->setfota_upgrade_param);
        
        ret = gp_module->setfota_upgrade_param(fota_type, url, filename, username, password);
   }while(0);

   return ret;
    
}

s32 MIL_fota_processing_report_handle(s8 *buff)
{
    s32 ret = DDI_ERR;

   do{
        
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        MODULE_FUNC_IS_NULL(gp_module->fota_processing_report_handle);
        
        ret = gp_module->fota_processing_report_handle(buff);
   }while(0);

   return ret;
}

s32 MIL_get_module_id(u8 *module_id)
{
   s32 ret = DDI_ERR;

   do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }

        if(!module_id)
        {
            ret = DDI_EINVAL;
            break;
        }
        
        *module_id = gp_module->m_id;
        ret = DDI_OK;
   }while(0);

   return ret;
}

s32 MIL_is_support_ota(void)
{
   s32 ret = DDI_ERR;

   do{
        if(!gp_module)
        {
            ret = DDI_ENODEV;
            break;
        }
        
        ret = gp_module->m_support_ota;
   }while(0);

   return ret;
}

wireless_module_t *dev_wireless_module_create(s8 *module_ver)
{
    if(strstr(module_ver, FIBOCOM_G500_MOULE_KEY_WORDS)
        || strstr(module_ver, FIBOCOM_G500_OPEN_CPU_MOULE_KEY_WORDS))
    {
        #if FIBOCOM_G500_SUPPORT
        gp_module = (wireless_module_t *)FIBOCOM_G500_module();
        #else
        gp_module = NULL;//(wireless_module_t *)FIBOCOM_G500_module();
        #endif
    }
    else if(strstr(module_ver, FIBOCOM_L610_MOULE_KEY_WORDS))
    {
        gp_module = (wireless_module_t *)FIBOCOM_L610_module();
    }

    return gp_module;
}
#endif
