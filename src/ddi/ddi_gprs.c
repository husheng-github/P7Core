#include "devglobal.h"
#include "ddi_gprs.h"

#define DDI_GPRS_ENABLE
#ifdef DDI_GPRS_ENABLE
s32 ddi_gprs_open(void)
{
    return dev_wireless_open();
}
s32 ddi_gprs_close(void)
{
    return dev_wireless_close();
}
s32 ddi_gprs_get_signalquality(u8 *prssi, u16 *prxfull)
{
    return dev_wireless_get_csq(prssi, prxfull);
}
s32 ddi_gprs_telephony_dial(u8 *num)
{
    return dev_wireless_telephone_dial(num);
}
s32 ddi_gprs_telephony_hangup(void)
{
    return dev_wireless_telephone_hangup();
}
s32 ddi_gprs_telephony_answer(void)
{
    return DDI_ENODEV;
}
s32 ddi_gprs_get_pdpstatus(wireless_status_type_e wireless_status_type)
{
    return dev_wireless_get_status(wireless_status_type);
}
s32 ddi_gprs_set_apn(u8* apn, u8* user, u8* psw)
{
    return dev_wireless_set_apn(apn, user, psw);
}
s32 ddi_gprs_socket_create(u8 type, u8 mode, u8 *param, u16 port)
{
    return dev_wireless_socket_create(type, mode, param, port);
}
s32 ddi_gprs_socket_send(s32 socketid, u8 *wbuf, u32 wlen)
{
    return dev_wireless_socket_send(socketid, wbuf, wlen);
}
s32 ddi_gprs_socket_recv(s32 socketid, u8 *rbuf, u32 rlen)
{
    return dev_wireless_socket_recv(socketid, rbuf, rlen);
}
s32 ddi_gprs_socket_close(s32 socketid)
{
    return dev_wireless_socket_close(socketid);
}
s32 ddi_gprs_socket_get_status(s32 socketid)
{
    return dev_wireless_socket_get_status(socketid);
}
s32 ddi_gprs_get_siminfo(strSimInfo *sim_info)
{
    dev_wireless_get_ccid(sim_info->iccid, sizeof(sim_info->iccid));
    return dev_wireless_get_imsi(sim_info->imsi, sizeof(sim_info->imsi));
}
s32 ddi_gprs_get_imei(u8 *imei_buf, u32 buf_size)
{
    return dev_wireless_get_imei(imei_buf, buf_size);
}
s32 ddi_gprs_get_basestation_info(strBasestationInfo *basestation_info)
{
    return dev_wireless_get_basestation_info(basestation_info);
}

s32 ddi_gprs_ioctl(u32 nCmd, u32 lParam, u32 wParam)
{
    s32 ret = DDI_ERR;
    wireless_net_info_t *wireless_net_info;
    wireless_device_info_t *device_info = dev_wireless_get_device();

    switch(nCmd)
    {
        case DDI_GPRS_CTL_CHECKSIM:
            ret = dev_wireless_get_simcard_status();
            break;
            
        case DDI_GPRS_GET_NET_INFO:
            wireless_net_info = (wireless_net_info_t *)lParam;
            ret = dev_wireless_get_localip(wireless_net_info->m_localip, sizeof(wireless_net_info->m_localip));
            break;
            
        case DDI_GPRS_GET_MODULE_VER:
            ret = dev_wireless_get_modulever((u8 *)lParam, wParam);
            break;

        case DDI_GPRS_CTL_OPEN_AIRPLANE:
            ret = dev_wireless_open_airplane();
            break;

        case DDI_GPRS_CTL_CLOSE_AIRPLANE:
            ret = dev_wireless_close_airplane();
            break;

        case DDI_GPRS_CTL_CFG_NTP_SERVER:
            ret = dev_wireless_cfg_ntp_server((s8 *)lParam, wParam);
            break;

        case DDI_GPRS_CTL_QRY_NTP:
            ret = dev_wireless_qry_rtc((s8 *)lParam, wParam);
            break;
        case DDI_GPRS_DISABLE_SWITCH_CELL:
            ret = DDI_OK;
            break;

        case DDI_GPRS_GET_MODULE_APPVER:
            ret = dev_wireless_get_appver((u8 *)lParam, wParam);
            break;
        case DDI_GPRS_GET_SSLCERTFIRST:
            ret = DDI_OK;
            break;
        case DDI_GPRS_SET_SSLMOD:
            ret = dev_wireless_ssl_set_mode(lParam);
            break;
        case DDI_GPRS_SET_SSLVER:
            ret = dev_wireless_ssl_setver(lParam);
            break;
        case DDI_GPRS_SET_SSLFILTE:
            ret = dev_wireless_ssl_filte(lParam);
            break;

        case DDI_GPRS_GET_MODULE_ID:
            ret = dev_wireless_get_module_id((u8 *)lParam);
            break;

        case DDI_GPRS_SUPPORT_OTA:
            ret = dev_wireless_is_support_ota();
            break;

        case DDI_GPRS_EXC_OTA:
            ret = dev_wireless_setfota_upgrade_param((module_ota_param_t *)lParam);
            break;

        case DDI_GPRS_GET_OTA_PROCESS:
            ret = dev_wireless_getfota_status((OTA_STEP_E *)lParam, (ota_process_value_t *)wParam);
            break;

        case DDI_GPRS_CMD_TRANSPARENT:
            device_info->m_transparent_switch = lParam;
            ret = DDI_OK;
            break;
            
        case 0xfe:
            dev_wireless_start_dial_log_collect();
            ret = DDI_OK;
            break;
            
        case 0xff:
            dev_wireless_print_dial_log();
            ret = DDI_OK;
            break;
    }
    
    return ret;
}

#else
s32 ddi_gprs_open(void)
{
    return DDI_ENODEV;
}
s32 ddi_gprs_close(void)
{
    return DDI_ENODEV;
}
s32 ddi_gprs_get_signalquality(u8 *prssi, u16 *prxfull)
{
    return DDI_ENODEV;
}
s32 ddi_gprs_telephony_dial(u8 *num)
{
    return DDI_ENODEV;
}
s32 ddi_gprs_telephony_hangup(void)
{
    return DDI_ENODEV;
}
s32 ddi_gprs_telephony_answer(void)
{
    return DDI_ENODEV;
}
s32 ddi_gprs_get_pdpstatus(wireless_status_type_e wireless_status_type)
{
    return DDI_ENODEV;
}
s32 ddi_gprs_set_apn(u8* apn, u8* user, u8* psw)
{
    return DDI_ENODEV;
}
s32 ddi_gprs_socket_create(u8 type, u8 mode, u8 *param, u16 port)
{
    return DDI_ENODEV;
}
s32 ddi_gprs_socket_send(s32 socketid, u8 *wbuf, u32 wlen)
{
    return DDI_ENODEV;
}
s32 ddi_gprs_socket_recv(s32 socketid, u8 *rbuf, u32 rlen)
{
    return DDI_ENODEV;
}
s32 ddi_gprs_socket_close(s32 socketid)
{
    return DDI_ENODEV;
}
s32 ddi_gprs_socket_get_status(s32 socketid)
{
    return DDI_ENODEV;
}
s32 ddi_gprs_get_siminfo(strSimInfo *sim_info)
{
    return DDI_ENODEV;
}
s32 ddi_gprs_get_imei(u8 *imei_buf, u32 buf_size)
{
    return DDI_ENODEV;
}
s32 ddi_gprs_get_basestation_info(strBasestationInfo *basestation_info)
{
    return DDI_ENODEV;
}
s32 ddi_gprs_ioctl(u32 nCmd, u32 lParam, u32 wParam)
{
    return DDI_ENODEV;
}

#endif
