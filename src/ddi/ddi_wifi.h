#ifndef __DDI_WIFI_H__
#define __DDI_WIFI_H__


#include "ddi_common.h"




#define WIFI_WORK_MODE_STATION (1)
#define WIFI_WORK_MODE_SOFTAP (2)
#define WIFI_WORK_MODE_SOFTAT_STATION (3)

#define WIFI_TRANSFER_MODE_NORMAL (0)
#define WIFI_TRANSFER_MODE_TRANSPARENT (1)

#define WIFI_CONNECT_MODE_SINGLE (0)
#define WIFI_CONNECT_MODE_MULTIPLE (1)

#define WIFI_SLEEP_MODE_DISABLE (0)
#define WIFI_SLEEP_MODE_LIGHT (1)
#define WIFI_SLEEP_MODE_MODEM (2)

#define WIFI_SOCKET_TYPE_TCP (0)
#define WIFI_SOCKET_TYPE_UDP (1)
#define WIFI_SOCKET_TYPE_SSL (2)

#define WIFI_STATUS_CONNECTING (0)
#define WIFI_STATUS_INITIALIZED (1)
#define WIFI_STATUS_CONNECTED 	(2)

#define WIFI_STATUS_CONNECT_AP_CONNECTING (0)
#define WIFI_STATUS_CONNECT_AP_DISCONNECTED (1)
#define WIFI_STATUS_CONNECT_AP_CONNECTED  (2)

#define WIFI_STATUS_SCAN_AP_PROCEEING  (0)
#define WIFI_STATUS_SCAN_AP_SUCCESSED  (1)

#define WIFI_SOCKET_STATUS_DISCONNECTED (0)
#define WIFI_SOCKET_STATUS_CONNECTING (1)
#define WIFI_SOCKET_STATUS_CONNECTED (2)


//kal events defines
#if 0
#define KAL_EVENT_AT_RESPONSE_SUCCESS (0x1 << 1)
#define KAL_EVENT_AT_READER_CLOSED (0x1 << 2)
#define KAL_EVENT_AT_READER_CLOSED_CONFIRM (0x1 << 3)
#define KAL_EVENT_AT_WRITE_SOCKET_DATA (0x1 << 4)
#define KAL_EVENT_DATA_CONSUMED (0x1 << 5)
#endif

#define WIFI_IOCTL_UPDATE    			1
#define WIFI_IOCTL_GET_FIRMWARE_VER 	2  //���ӻ�ȡWIFI�汾�ӿ�
#define WIFI_IOCTL_GET_SSID             3  //���ӻ�ȡWIFI SSID
#define WIFI_IOCTL_GET_NET_INFO         4
#define WIFI_IOCTL_CREAT_TCP_SERVER     5
#define WIFI_IOCTL_TCP_SERVER_ACCEPT    6
#define WIFI_IOCTL_TCP_SERVER_CLOSE     7
#define WIFI_IOCTL_CFG_NTP_SERVER       8
#define WIFI_IOCTL_GET_NTP              9
#define WIFI_IOCTL_REG_EVENT            10          //ע���¼��ϱ�
#define WIFI_IOCTL_CLEAR_RF_PARAM       0xf0          //���RF��Ƶ����
#define WIFI_IOCTL_CMD_TRANSPARENT      0xf1          //͸������     ��Ӧ�ò�ֱ�ӷ���wifiָ��
#define WIFI_IOCTL_CMD_REPOWER          0xf2          //͸������     ��Ӧ�ò�ֱ�ӷ���wifiָ��







#define WIFI_VER_AT    0
#define WIFI_VER_SDK   1

typedef enum{
    WIFI_AIRKISS_FREE = 0,       //����
    WIFI_AIRKISS_SEARCHING,      //����������
    WIFI_AIRKISS_GETCONFIGING,   //���ڻ�ȡwifi ssid��password
    WIFI_AIRKISS_GETCONFIG,      //�Ѿ���ȡ�� wifi ssid��password
    WIFI_AIRKISS_CONNECTING,     //��������������
    WIFI_AIRKISS_DHCP_TIMEOUT,   //�����ȡIP��ʱ
    WIFI_AIRKISS_CONNECT_OK,     //�������ӳɹ�
    WIFI_AIRKISS_CONNECT_ERR,    //��������ʧ��
}WIFI_AIRKISS_STEP_E;

typedef struct __WIFI_IOCTL_UPDATE
{
    u8 *m_filename;         //���ص��ļ�
    u32 m_start;            //���ص���ʼλ��
    u32 m_size;             //���صĴ�С
    u32 m_address;          //�����ļ�����
}wifi_ioctl_update_t;

typedef struct __WIFI_APINFO{
    u32 m_ecn;
    u8 m_ssid[64];
    s32 m_rssi;
    u8 m_mac[18];
    s32 m_channel;
    s32 m_freq_offset;
    s32 m_freq_cali;
    s32 m_pairwise_cipher;
    s32 m_group_cipher;
    s32 m_bgn;
    s32 m_wps;
} wifi_apinfo_t;

typedef struct 
 {
    s8 m_localip[16]; //���� IP
    s8 m_gateway [16]; //��������
    s8 m_mask [16]; //��������
    s8 m_dns1 [16]; //Ĭ�� dns
    s8 m_dns2 [16]; //���� dns
    s8 m_macid[32];
 }wifi_net_info_t;

typedef enum
{
    WF_NO_ERR = 0,                     // �޴�����Ϣ
    WF_AP_IS_NULL = 1,                 // �ȵ���δ����
    WF_NO_WIFI_MODULE = 2,             // ȱ��wifiģ��
    WF_WIFI_AP_PASSWORD_IS_INVALID,    // AP���벻��
}WIFI_CONTROL_FAIL_REASON_E;

//������Ʋ��Բ���
typedef enum
{
    WIFI_MODULE_POWER_ON = 0,         //�ϵ�
    WIFI_MODULES_INIT = 1,            // ģ���ʼ��
    WIFI_MODULES_OPEN_SUCCESS,        //wifiģ��򿪳ɹ�
    WIFI_AP_CONNECTTING,              // �����ȵ���
    WIFI_AP_CONNECTTED,               // �����ȵ�ɹ�
    WIFI_AP_DISCONNECT,               // �Ͽ��ȵ�����
    WIFI_MODULE_POWER_OFF,            //�µ�
    WIFI_CHECK_CMD                    //ָ���쳣�����ָ��ִ���Ƿ�ok
}WIFI_CONTROL_STRATEGY_STEP_E;

typedef enum
{
    WIFI_GET_REG_NETWORK_STATE = 0,      //��ȡwifi״̬����
    WIFI_GET_REG_NETWORK_FAIL_REASON     //��ȡwifi����ʧ��ԭ������
}wifi_status_type_e;

typedef enum{
    WN_EVENT_TCP_CONNECTED,              //tcp���ӳɹ�
    WN_EVENT_TCP_DISCONNECT,             //tcp���ӶϿ�
    WN_EVENT_TCP_DATA_IN                 //�յ���̨����
}WIFI_NOTIFY_EVENT_E;

typedef s32 (*wifi_notify_event_func)(WIFI_NOTIFY_EVENT_E event, u32 lparam, u32 wparam);


//=====================================================
//���⺯������
s32 ddi_wifi_open(void);
s32 ddi_wifi_close(void);
s32 ddi_wifi_scanap_start(wifi_apinfo_t *lp_scaninfo, u32 apmax);
s32 ddi_wifi_scanap_status(u32 *ap_num);
s32 ddi_wifi_connectap_start(u8 *ssid, u8 *psw, u8 *bssid);
s32 ddi_wifi_connectap_status (void);
s32 ddi_wifi_disconnectap(void);
s32 ddi_wifi_socket_create_start(u8 type, u8 mode, u8 *param, u16 port);
s32 ddi_wifi_socket_get_status(s32 socketid);
s32 ddi_wifi_socket_send(s32 socketid, u8 *wbuf, s32 wlen);
s32 ddi_wifi_socket_recv(s32 socketid, u8 *rbuf, s32 rlen);
s32 ddi_wifi_socket_close(s32 socketid);
s32 ddi_wifi_get_signal(s32 *rssi);
s32 ddi_wifi_ioctl(u32 nCmd, u32 lParam, u32 wParam);

s32 ddi_wifi_start_airkiss(void);
s32 ddi_wifi_get_airkiss_status(void);
s32 ddi_wifi_get_airkiss_config(    u8 *ssid, u8 *password);
s32 ddi_wifi_stop_airkiss(void);


//===============================================
typedef	s32 (*core_ddi_wifi_open)(void);
typedef	s32 (*core_ddi_wifi_close)(void);
typedef s32 (*core_ddi_wifi_scanap_start)(wifi_apinfo_t *lp_apinfo, u32 ap_num);
typedef s32 (*core_ddi_wifi_scanap_status) (u32 *ap_num);
typedef	s32 (*core_ddi_wifi_connectap_start)(u8 *ssid, u8 *psw, u8 *bssid);
typedef s32 (*core_ddi_wifi_connectap_status)(void);
typedef	s32 (*core_ddi_wifi_disconnectap)(void);
typedef	s32 (*core_ddi_wifi_socket_create_start)(u8 type, u8 mode, u8 *param, u16 port);
typedef s32 (*core_ddi_wifi_socket_get_status)(s32 socketid);
typedef	s32 (*core_ddi_wifi_socket_send)(s32 socketid, u8 *wbuf, s32 wlen);
typedef	s32 (*core_ddi_wifi_socket_recv)(s32 socketid, u8 *rbuf, s32 rlen);
typedef	s32 (*core_ddi_wifi_socket_close)(s32 socketid);
typedef	s32 (*core_ddi_wifi_get_signal)(s32 *rssi);
typedef s32 (*core_ddi_wifi_ioctl)(u32 nCmd, u32 lParam, u32 wParam);
typedef s32 (*core_ddi_wifi_start_airkiss)(void);
typedef s32 (*core_ddi_wifi_get_airkiss_status)(void);
typedef s32 (*core_ddi_wifi_get_airkiss_config)(    u8 *ssid, u8 *password);
typedef s32 (*core_ddi_wifi_stop_airkiss)(void);


#endif /* __DDI_WIFI_H__ */
