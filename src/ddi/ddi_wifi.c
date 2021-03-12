#include "devglobal.h"
#include "ddi_wifi.h"

#ifdef DDI_WIFI_ENABLE
s32 ddi_wifi_open(void)
{
    return dev_wifi_open();
}
s32 ddi_wifi_close(void)
{
    return dev_wifi_close();
}
#if 0
s32 ddi_wifi_get_status(void)
{
	return dev_wifi_get_status();
}
s32 ddi_wifi_scanap(wifi_apinfo_t *lp_apinfo, u32 *ap_num)
{
    return dev_wifi_scanap_wait(lp_apinfo, ap_num);
}

s32 ddi_wifi_connetap(u8 *ssid, u8 *psw, u8 *bssid)
{
    return dev_wifi_connetap_wait(ssid, psw, bssid);
}
#endif
s32 ddi_wifi_scanap_start(wifi_apinfo_t *lp_scaninfo, u32 apmax)
{
    return dev_wifi_scanap_start(lp_scaninfo, apmax);
}
s32 ddi_wifi_scanap_status(u32 *ap_num)
{
    return dev_wifi_scanap_status(ap_num);
}
s32 ddi_wifi_connectap_start(u8 *ssid, u8 *psw, u8 *bssid)
{
    return dev_wifi_connetap(ssid, psw, bssid);
}
s32 ddi_wifi_connectap_status (void)
{
    return dev_wifi_get_connectap_status();
}

s32 ddi_wifi_disconnectap(void)
{
    return dev_wifi_disconnetap();
}
#if 0
s32 ddi_wifi_socket_create(u8 type, u8 mode, u8 *param, u16 port)
{
    return dev_wifi_socket_create_wait(type, mode, param, port);
}
#endif
s32 ddi_wifi_socket_create_start(u8 type, u8 mode, u8 *param, u16 port)
{
    return dev_wifi_socket_create(type, mode, param, port);
}

s32 ddi_wifi_socket_get_status(s32 socketid)
{
    s32 ret = dev_wifi_socket_get_status(socketid);

    if(3 == ret)
    {
        ret = WIFI_SOCKET_STATUS_DISCONNECTED;
    }
    
    return ret;
}
s32 ddi_wifi_socket_send(s32 socketid, u8 *wbuf, s32 wlen)
{
    return dev_wifi_socket_send(socketid, wbuf, wlen);
}
s32 ddi_wifi_socket_recv(s32 socketid, u8 *rbuf, s32 rlen)
{
    return dev_wifi_socket_recv(socketid, rbuf, rlen);
}
s32 ddi_wifi_socket_close(s32 socketid)
{
    return dev_wifi_socket_close(socketid);
}
s32 ddi_wifi_get_signal(s32 *rssi)
{
    return dev_wifi_get_signal(rssi);
}

s32 ddi_wifi_ioctl(u32 nCmd, u32 lParam, u32 wParam)
{
    return dev_wifi_ioctl(nCmd, lParam, wParam);
}

s32 ddi_wifi_start_airkiss(void)
{
    return dev_wifi_start_airkiss();
}

s32 ddi_wifi_get_airkiss_status(void)
{
    return dev_wifi_get_airkiss_status();
}

s32 ddi_wifi_get_airkiss_config(    u8 *ssid, u8 *password)
{
    return dev_wifi_get_airkiss_config(ssid, password);
}

s32 ddi_wifi_stop_airkiss(void)
{
    return dev_wifi_stop_airkiss();
}

#else
s32 ddi_wifi_open(void)
{
    return DDI_ENODEV;
}
s32 ddi_wifi_close(void)
{
    return DDI_ENODEV;
}
s32 ddi_wifi_scanap_start(wifi_apinfo_t *lp_scaninfo, u32 apmax)
{
    return DDI_ENODEV;
}
s32 ddi_wifi_scanap_status(u32 *ap_num)
{
    return DDI_ENODEV;
}
s32 ddi_wifi_connectap_start(u8 *ssid, u8 *psw, u8 *bssid)
{
    return DDI_ENODEV;
}
s32 ddi_wifi_connectap_status (void)
{
    return DDI_ENODEV;
}

s32 ddi_wifi_disconnectap(void)
{
    return DDI_ENODEV;
}
s32 ddi_wifi_socket_create_start(u8 type, u8 mode, u8 *param, u16 port)
{
    return DDI_ENODEV;
}
s32 ddi_wifi_socket_get_status(s32 socketid)
{
    return DDI_ENODEV;
}
s32 ddi_wifi_socket_send(s32 socketid, u8 *wbuf, s32 wlen)
{
    return DDI_ENODEV;
}
s32 ddi_wifi_socket_recv(s32 socketid, u8 *rbuf, s32 rlen)
{
    return DDI_ENODEV;
}
s32 ddi_wifi_socket_close(s32 socketid)
{
    return DDI_ENODEV;
}
s32 ddi_wifi_get_signal(s32 *rssi)
{
    return DDI_ENODEV;
}
s32 ddi_wifi_ioctl(u32 nCmd, u32 lParam, u32 wParam)
{
    return DDI_ENODEV;
}

s32 ddi_wifi_start_airkiss(void)
{
    return DDI_ENODEV;
}

s32 ddi_wifi_get_airkiss_status(void)
{
    return DDI_ENODEV;
}

s32 ddi_wifi_get_airkiss_config(    u8 *ssid, u8 *password)
{
    return DDI_ENODEV;
}

s32 ddi_wifi_stop_airkiss(void)
{
    return DDI_ENODEV;
}

#endif
