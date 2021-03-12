#ifndef __DEV_WIFI_H
#define __DEV_WIFI_H

#define WIFI_FIXED_FREQ_TEST
#undef WIFI_FIXED_FREQ_TEST


#define WIFI_AP_INFO_FILE                  "ap_info"
#define WIFI_PORT                          PORT_UART3_NO
#define WIFI_RUN_OPEN_CPU                  FALSE     //是否是在opencpu上运行，串口接收不由我们控制
#define WIFI_SUPPORT_HARD_FLOW             FALSE
#define WIFI_RUN_OPEN_CPU_RX_BUFF          (9*1024)
#define WIFI_GET_CSQ_INTERVAL              10000     //模块是5秒更新一次csq,应用获取csq的间隔控制 10秒
#define WIFI_TIMER_BASE                    10        //10ms
#define WIFI_RCV_AT_MAX                    512
#define WIFI_SOCKET_RCV_BUFLEN             2048      //P3上是一包数据收全就发消息，应用才会读，之前定义的是2K，如果返回的数据超过2K，就无法收全
#define WIFI_SOCKET_SEND_BUFLEN            1024
#define WIFI_RCV_IPD_MAX                   0         //动态申请
#define WIFIAT_RXBUF_MAX                   (WIFI_RCV_AT_MAX+WIFI_RCV_IPD_MAX)
#define WIFI_SOCKET_MAX                    3     //允许开辟的SOCKET个数
#define WIFI_ID_MASK                       0x0D000000
#define WIFI_RESET_THRESHOLD_TIMES        5       //AT指令失败次数的触发复位模块的阈值
#define WIFI_SIMCARD_DETECT_TIMES         5       //sim卡检测不在时重复次数
#define WIFI_MODULE_DETECT_TIMES          10       //认为无线模块不存在的阈值
#define WIFI_CHECK_NET_REG_TIMES          300      //检测驻网是否成功的尝试次数
#define WIFI_AT_DEFAULT_TIMEOUT           1000
#define WIFI_AT_CMD_RESEND_TIMEOUT        1000    //指令重发的间隔时间
#define WIFI_AT_BOOTON_TIMEOUT            1000    //上电高电平持续时间
#define WIFI_AT_POWERON_TIMEOUT           500     //开机低电平持续时间
#define WIFI_AT_POWEROFF_WAIT_TIMEOUT     500
#define WIFI_AT_RECV_DEFAULT_WAIT_TIMEOUT 3000
#define WIFI_AT_REG_NET_TIMEOUT           30000
#define WIFI_AT_TCP_CONNECT_TIMEOUT       30000
#define WIFI_AT_TCP_SEND_TIMEOUT          10000
#define WIFI_AT_CONNECT_AP_TIMEOUT        40000    //设置ap后，等待获取ip的超时时间
#define WIFI_AT_SET_SLEEPTIME_TIMEOUT    300
#define WIFI_GET_CSQ_INTERVAL            3000


#define AT_CMD_RES_END                    "\r\n"
#define WIFI_AT_CMD                       "AT"
#define WIFI_ATE_CMD                      "ATE0"
#define WIFI_AT_GMR_CMD                   "AT+GMR"
#define WIFI_AT_SET_CUR_MODE              "AT+CWMODE_CUR=1"      //设置当前wifi模式---softAp
#define WIFI_AT_SET_CUR_MODE_3            "AT+CWMODE_CUR=3"      //设置当前wifi模式---softAp+station
#define WIFI_AT_SET_TRANS_MODE            "AT+CIPMODE=0"         //设置为普通传输模式
#define WIFI_AT_GET_TRANS_MODE            "AT+CIPMODE?"         //设置为普通传输模式
#define WIFI_AT_SET_MUX                   "AT+CIPMUX=1"          //设置tcp多连接
#define WIFI_AT_SET_AUTO_CONN             "AT+CWAUTOCONN=0"      //设置上电自动连接热点
#define WIFI_AT_SCAN_RES_CFG              "AT+CWLAPOPT=1,2047"   //设置扫描热点的返回格式
#define WIFI_AT_WAKEUP_GPIO               "AT+WAKEUPGPIO=1,0,1"  //设置高电平唤醒light-sleep
#define WIFI_AT_SLEEP_MODE                "AT+SLEEP=0"           //设置Light-sleep 模式
#define WIFI_AT_SET_MAX_SERVERCONN        "AT+CIPSERVERMAXCONN=2" 
#define WIFI_AT_OPEN_TCP_SERVER           "AT+CIPSERVER=1,80" 
#define WIFI_AT_SET_AP_NAME               "AT+CWSAP_CUR=\"Cloud-Printer\",\"\",1,0,4,0" 
#define WIFI_AT_GET_AP_NAME               "AT+CWSAP_CUR?"
#define WIFI_AT_READY_SEND_DATA           "> "



#define WIFI_AT_DISCONNECT_AP             "AT+CWQAP"             //断开热点


#define WIFI_AT_RES_AP_CONNECTED                     "WIFI GOT IP"          //热点连接成功
#define WIFI_AT_RES_AP_DISCONNECT                    "WIFI DISCONNECT"      //热点断开
#define WIFI_AT_RES_AP_GETIP_TIMEOUT                 "DHCP TIMEOUT"
#define WIFI_AT_RES_AP_CONNECT_AP_TIMEOUT            "+CWJAP:1"             //连接ap超时
#define WIFI_AT_RES_AP_CONNECT_AP_PASSWORD_ERR       "+CWJAP:2"             //ap密码错误
#define WIFI_AT_RES_AP_CONNECT_AP_NO_FOUND           "+CWJAP:3"             //找不到目标ap
#define WIFI_AT_RES_AP_CONNECT_AP_FAIL               "+CWJAP:4"             //连接ap失败
#define WIFI_AT_RES_BUSY                             "busy p..."

#define WIFI_AT_RES_TCP_CONNECT            ",CONNECT"            //TCP连接成功
#define WIFI_AT_RES_TCP_CLOSED             ",CLOSED"             //TCP连接断开

typedef enum{
    SOCKET_STATUS_NONE = 0,               //空闲
    SOCKET_STATUS_CONNECTING = 1,         //正在连接
    SOCKET_STATUS_CONNECTED = 2,          //连接成功
    SOCKET_STATUS_DISCONNECTED = 3        //连接断开
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
    WIFI_CONTROL_STRATEGY_STEP_E  m_last_control_step;     //设置指令异常时，备份之前的步骤
    WIFI_CONTROL_STRATEGY_STEP_E  m_control_step;
    WIFI_CONTROL_FAIL_REASON_E    m_control_fail_reason;
    wifi_socket_info_t *m_socket_info[WIFI_SOCKET_MAX];
    s8 m_rtc[32];                                          //模块端rtc时间
    s32 m_airkiss_change;                                  //微信配网修改了ap
    s8 m_bssid[64];
    s8 m_ssid[64];
    s8 m_password[64];
    s8 m_sdk_ver[32];
    s8 m_at_ver[32];
    s8 m_local_ip[32];
    u32 m_beg_connect_ap_time;
    s32 m_rssi;                               //信号
    WIFI_AIRKISS_STEP_E m_airkiss_step;
    s32 m_use_tcp_server;                     //是否用tcp server 1是  0否
    str_CircleQueue m_wifi_uart_rx_fifo;
    wifi_notify_event_func m_notify_func;     //事件主动上送接口
    s32 m_power_flag;                         //1 已上电  0没上电
    wifi_apinfo_t *m_scaninfo;                //存放应用层存放ap信息的内存
    s32 m_max_ap_num;                         //应用层存放ap信息的内存数量
    s32 m_scan_ap_no;                         //填充的热点个数
    s32 m_connect_ap_res;
    s32 m_idle_flag;                          //1刚从低功耗唤醒，由dev_wifi_get_connectap_status调用
    s32 m_transparent_switch;                 //1使用透明传输    0不使用
    s8 *m_ap_name;                            //softap模式的热点名
    u16 m_tcp_server_port;                    //tcp server的监听端口，用于网页配网
    s32 m_need_connect_ap;                    //FALSE不需要 TRUE需要  此变量主要是从低功耗唤醒时，如果wifi ap没连接成功，需要重新open，但是应用不会再调open，所以用此变量在查询连接状态的时候使用
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

