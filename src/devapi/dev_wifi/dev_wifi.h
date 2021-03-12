#ifndef __DEV_WIFI_H
#define __DEV_WIFI_H

#define WIFI_FIXED_FREQ_TEST
#undef WIFI_FIXED_FREQ_TEST


#define WIFI_AP_INFO_FILE                  "ap_info"
#define WIFI_PORT                          PORT_UART3_NO
#define WIFI_RUN_OPEN_CPU                  FALSE     //�Ƿ�����opencpu�����У����ڽ��ղ������ǿ���
#define WIFI_SUPPORT_HARD_FLOW             FALSE
#define WIFI_RUN_OPEN_CPU_RX_BUFF          (9*1024)
#define WIFI_GET_CSQ_INTERVAL              10000     //ģ����5�����һ��csq,Ӧ�û�ȡcsq�ļ������ 10��
#define WIFI_TIMER_BASE                    10        //10ms
#define WIFI_RCV_AT_MAX                    512
#define WIFI_SOCKET_RCV_BUFLEN             2048      //P3����һ��������ȫ�ͷ���Ϣ��Ӧ�òŻ����֮ǰ�������2K��������ص����ݳ���2K�����޷���ȫ
#define WIFI_SOCKET_SEND_BUFLEN            1024
#define WIFI_RCV_IPD_MAX                   0         //��̬����
#define WIFIAT_RXBUF_MAX                   (WIFI_RCV_AT_MAX+WIFI_RCV_IPD_MAX)
#define WIFI_SOCKET_MAX                    3     //�����ٵ�SOCKET����
#define WIFI_ID_MASK                       0x0D000000
#define WIFI_RESET_THRESHOLD_TIMES        5       //ATָ��ʧ�ܴ����Ĵ�����λģ�����ֵ
#define WIFI_SIMCARD_DETECT_TIMES         5       //sim����ⲻ��ʱ�ظ�����
#define WIFI_MODULE_DETECT_TIMES          10       //��Ϊ����ģ�鲻���ڵ���ֵ
#define WIFI_CHECK_NET_REG_TIMES          300      //���פ���Ƿ�ɹ��ĳ��Դ���
#define WIFI_AT_DEFAULT_TIMEOUT           1000
#define WIFI_AT_CMD_RESEND_TIMEOUT        1000    //ָ���ط��ļ��ʱ��
#define WIFI_AT_BOOTON_TIMEOUT            1000    //�ϵ�ߵ�ƽ����ʱ��
#define WIFI_AT_POWERON_TIMEOUT           500     //�����͵�ƽ����ʱ��
#define WIFI_AT_POWEROFF_WAIT_TIMEOUT     500
#define WIFI_AT_RECV_DEFAULT_WAIT_TIMEOUT 3000
#define WIFI_AT_REG_NET_TIMEOUT           30000
#define WIFI_AT_TCP_CONNECT_TIMEOUT       30000
#define WIFI_AT_TCP_SEND_TIMEOUT          10000
#define WIFI_AT_CONNECT_AP_TIMEOUT        40000    //����ap�󣬵ȴ���ȡip�ĳ�ʱʱ��
#define WIFI_AT_SET_SLEEPTIME_TIMEOUT    300
#define WIFI_GET_CSQ_INTERVAL            3000


#define AT_CMD_RES_END                    "\r\n"
#define WIFI_AT_CMD                       "AT"
#define WIFI_ATE_CMD                      "ATE0"
#define WIFI_AT_GMR_CMD                   "AT+GMR"
#define WIFI_AT_SET_CUR_MODE              "AT+CWMODE_CUR=1"      //���õ�ǰwifiģʽ---softAp
#define WIFI_AT_SET_CUR_MODE_3            "AT+CWMODE_CUR=3"      //���õ�ǰwifiģʽ---softAp+station
#define WIFI_AT_SET_TRANS_MODE            "AT+CIPMODE=0"         //����Ϊ��ͨ����ģʽ
#define WIFI_AT_GET_TRANS_MODE            "AT+CIPMODE?"         //����Ϊ��ͨ����ģʽ
#define WIFI_AT_SET_MUX                   "AT+CIPMUX=1"          //����tcp������
#define WIFI_AT_SET_AUTO_CONN             "AT+CWAUTOCONN=0"      //�����ϵ��Զ������ȵ�
#define WIFI_AT_SCAN_RES_CFG              "AT+CWLAPOPT=1,2047"   //����ɨ���ȵ�ķ��ظ�ʽ
#define WIFI_AT_WAKEUP_GPIO               "AT+WAKEUPGPIO=1,0,1"  //���øߵ�ƽ����light-sleep
#define WIFI_AT_SLEEP_MODE                "AT+SLEEP=0"           //����Light-sleep ģʽ
#define WIFI_AT_SET_MAX_SERVERCONN        "AT+CIPSERVERMAXCONN=2" 
#define WIFI_AT_OPEN_TCP_SERVER           "AT+CIPSERVER=1,80" 
#define WIFI_AT_SET_AP_NAME               "AT+CWSAP_CUR=\"Cloud-Printer\",\"\",1,0,4,0" 
#define WIFI_AT_GET_AP_NAME               "AT+CWSAP_CUR?"
#define WIFI_AT_READY_SEND_DATA           "> "



#define WIFI_AT_DISCONNECT_AP             "AT+CWQAP"             //�Ͽ��ȵ�


#define WIFI_AT_RES_AP_CONNECTED                     "WIFI GOT IP"          //�ȵ����ӳɹ�
#define WIFI_AT_RES_AP_DISCONNECT                    "WIFI DISCONNECT"      //�ȵ�Ͽ�
#define WIFI_AT_RES_AP_GETIP_TIMEOUT                 "DHCP TIMEOUT"
#define WIFI_AT_RES_AP_CONNECT_AP_TIMEOUT            "+CWJAP:1"             //����ap��ʱ
#define WIFI_AT_RES_AP_CONNECT_AP_PASSWORD_ERR       "+CWJAP:2"             //ap�������
#define WIFI_AT_RES_AP_CONNECT_AP_NO_FOUND           "+CWJAP:3"             //�Ҳ���Ŀ��ap
#define WIFI_AT_RES_AP_CONNECT_AP_FAIL               "+CWJAP:4"             //����apʧ��
#define WIFI_AT_RES_BUSY                             "busy p..."

#define WIFI_AT_RES_TCP_CONNECT            ",CONNECT"            //TCP���ӳɹ�
#define WIFI_AT_RES_TCP_CLOSED             ",CLOSED"             //TCP���ӶϿ�

typedef enum{
    SOCKET_STATUS_NONE = 0,               //����
    SOCKET_STATUS_CONNECTING = 1,         //��������
    SOCKET_STATUS_CONNECTED = 2,          //���ӳɹ�
    SOCKET_STATUS_DISCONNECTED = 3        //���ӶϿ�
}TCP_SOCKET_STATUS_E;

typedef struct
{
    s32 m_socket_id;
    volatile s32 m_status;
    str_CircleQueue m_read_buffer;
    s32 total_receive;
    s8 m_serveraddr[64];
    s32 m_serverport;
}wifi_socket_info_t;

typedef struct{
    WIFI_CONTROL_STRATEGY_STEP_E  m_last_control_step;     //����ָ���쳣ʱ������֮ǰ�Ĳ���
    WIFI_CONTROL_STRATEGY_STEP_E  m_control_step;
    WIFI_CONTROL_FAIL_REASON_E    m_control_fail_reason;
    wifi_socket_info_t *m_socket_info[WIFI_SOCKET_MAX];
    s8 m_rtc[32];                                          //ģ���rtcʱ��
    s32 m_airkiss_change;                                  //΢�������޸���ap
    s8 m_bssid[64];
    s8 m_ssid[64];
    s8 m_password[64];
    s8 m_sdk_ver[32];
    s8 m_at_ver[32];
    s8 m_local_ip[32];
    u32 m_beg_connect_ap_time;
    s32 m_rssi;                               //�ź�
    WIFI_AIRKISS_STEP_E m_airkiss_step;
    s32 m_use_tcp_server;                     //�Ƿ���tcp server 1��  0��
    str_CircleQueue m_wifi_uart_rx_fifo;
    wifi_notify_event_func m_notify_func;     //�¼��������ͽӿ�
    s32 m_power_flag;                         //1 ���ϵ�  0û�ϵ�
    wifi_apinfo_t *m_scaninfo;                //���Ӧ�ò���ap��Ϣ���ڴ�
    s32 m_max_ap_num;                         //Ӧ�ò���ap��Ϣ���ڴ�����
    s32 m_scan_ap_no;                         //�����ȵ����
    s32 m_connect_ap_res;
    s32 m_idle_flag;                          //1�մӵ͹��Ļ��ѣ���dev_wifi_get_connectap_status����
    s32 m_transparent_switch;                 //1ʹ��͸������    0��ʹ��
    s8 *m_ap_name;                            //softapģʽ���ȵ���
    u16 m_tcp_server_port;                    //tcp server�ļ����˿ڣ�������ҳ����
    s32 m_need_connect_ap;                    //FALSE����Ҫ TRUE��Ҫ  �˱�����Ҫ�Ǵӵ͹��Ļ���ʱ�����wifi apû���ӳɹ�����Ҫ����open������Ӧ�ò����ٵ�open�������ô˱����ڲ�ѯ����״̬��ʱ��ʹ��
}wifi_device_info_t;

iomux_pin_name_t wifi_get_en_pin(void);
iomux_pin_name_t wifi_get_wakeup_pin(void);

wifi_device_info_t *dev_wifi_get_device(void);
s32 dev_wifi_init(void);
s32 dev_wifi_open(void);
s32 dev_wifi_close(void);
void dev_wifi_poweroff(void);
s32 dev_wifi_scanap_start(wifi_apinfo_t *lp_scaninfo, u32 apmax);
s32 dev_wifi_scanap_status(u32 *ap_num);
s32 dev_wifi_connetap(u8 *ssid, u8 *psw, u8 *bssid);
s32 dev_wifi_get_signal(s32 *rssi);
s32 dev_wifi_disconnetap(void);
s32 dev_wifi_get_connectap_status(void);
s32 dev_wifi_get_localip(u8 *ip_buf, u32 buf_size);
s32 dev_wifi_socket_create(u8 type, u8 mode, u8 *param, u16 port);
s32 dev_wifi_socket_get_status(s32 socketid);
s32 dev_wifi_socket_send(s32 socketid, u8 *wbuf, u32 wlen);
s32 dev_wifi_socket_recv(s32 socketid, u8 *rbuf, u32 rlen);
s32 dev_wifi_socket_close(s32 socketid);
s32 dev_wifi_resume(void);
s32 dev_wifi_suspend(void); 
s32 ddi_wifi_get_airkiss_config(u8 *ssid, u8 *password);
s32 ddi_wifi_get_airkiss_status(void);
s32 ddi_wifi_start_airkiss(void);
s32 ddi_wifi_stop_airkiss(void);
s32 dev_wifi_get_status(wifi_status_type_e wifi_status_type);
s32 dev_wifi_clear_rf_param(void);


#endif

