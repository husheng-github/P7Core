#ifndef __DEV_WIRELESS_H
#define __DEV_WIRELESS_H

#define IPV6_TEST                              //ipv6测试
#undef IPV6_TEST

#define WIRELESS_PORT                          PORT_UART1_NO
#define WIRELESS_APN_FILE                      "/mtd0/apn_info"
#define WIRELESS_AUDIO_FILE                    "/mtd0/audio_info"
#define WIRELESS_GET_CSQ_INTERVAL              10000     //模块是5秒更新一次csq,应用获取csq的间隔控制 10秒
#define WIRELESS_TIMER_BASE                    10        //10ms
#define WIRELESS_RCV_AT_MAX                    (512)
#define WIRELESS_SOCKET_RCV_BUFLEN             2048      //sxl?2020
#define WIRELESS_SOCKET_SEND_BUFLEN            1024
#define WIRELESS_RCV_IPD_MAX                   0         //动态申请
#define WIRELESSAT_RXBUF_MAX                   (WIRELESS_RCV_AT_MAX+WIRELESS_RCV_IPD_MAX)
#define WIRELESS_SOCKET_MAX                    3     //允许开辟的SOCKET个数
#define WIRELESS_ID_MASK                       0x08000000
#define WIRELESS_CELL_NUM                      7
#define WIRE_EXGPIO_MAX                        4       //无线模块端外挂设备的gpio个数
#define WIRELESS_DIAL_LOG_SIZE                 2048
#define WIRELESS_AUDIO_FILE_NAME_MAX           68               //LC610无线模块内部定义的最大长度为64字节
#define WIRELESS_AUDIO_FILE_UPLOAD_PER_SIZE    (2048)


typedef struct{
    u32 m_mcc;
    u32 m_mnc;
    u32 m_lac;
    u32 m_cid;
    u32 m_arfcn;
    u32 m_rxlen;
    u32 m_rssi;
}wireless_cell_info_t;

typedef struct
{
    s32 m_socket_id;
    volatile s32 m_status;
    str_CircleQueue m_read_buffer;
    s32 total_receive;
    s8 m_serveraddr[64];
    s32 m_serverport;
}wireless_socket_info_t;

typedef struct{
    s32 m_audio_vol;                                      //喇叭声音大小
    s32 m_tts_play_done;                                //是否播报完成 TRUE播报完成   FALSE 未播报
    s32 m_mp3_play_done;                                //是否播报完成 TRUE播报完成   FALSE 未播报
}wire_audio_info_t;

typedef struct{
    OTA_STEP_E          m_step;
    ota_process_value_t m_process_value;
}wire_ota_info_t;

typedef struct{
    WIRE_CONTROL_STRATEGY_STEP_E  m_last_control_step;     //设置指令异常时，备份之前的步骤
    WIRE_CONTROL_STRATEGY_STEP_E  m_control_step;
    WIRE_CONTROL_FAIL_REASON_E    m_control_fail_reason;
    wireless_socket_info_t *m_socket_info[WIRELESS_SOCKET_MAX];
    str_cell_info m_main_cell_info;
    wireless_cell_info_t *m_cell_info[WIRELESS_CELL_NUM];
    s32 m_cell_num;                                        //基站个数
    s8 m_rtc[32];                                          //模块端rtc时间
    s8 m_apn[64];
    s8 m_username[64];
    s8 m_password[64];
    s8 m_imei[32];
    s8 m_imsi[32];
    s8 m_ccid[32];
    s8 m_module_ver[32];
    s8 m_module_app_ver[32];
    s8 m_local_ip[32];
    u8 m_simcard_status;                                   //1没有sim卡 2有sim卡
    u8 m_csq;                                              //信号强度
    u16 m_bear_rate;                                       //误码率
    u32 m_beg_active_pdp_time;                             //激活pdp的开始时间
    u8 m_ssltype;                                           //0:不用SSL   1:无证书SSL，2:单项验证SSL           3:双向验证SSL
    u8 m_sslsocket;                                         //记录当前SSL socket
    s8 *m_dial_log_buff;                                    //用来记录拨号日志的缓存
    s32 m_power_flag;                                       //True上电   FALSE下电
    s32 m_transparent_switch;                 //1使用透明传输    0不使用
    wire_audio_info_t m_audio_info;
    wire_ota_info_t m_ota_info;
    s32 m_cur_close_socket_id;
}wireless_device_info_t;

typedef enum{
    WIRELESS_GPIO_OUT,                                      //输出
    WIRELESS_GPIO_IN,                                      //输入
}WIRELESS_GPIO_DIRECTION_E;

wireless_device_info_t *dev_wireless_get_device(void);
s32 dev_wireless_init(void);
s32 dev_wireless_open(void);
s32 dev_wireless_close(void);
void dev_wireless_poweroff(void);
s32 dev_wireless_set_apn(u8* apn, u8* user, u8* psw);
s32 dev_wireless_get_status(wireless_status_type_e wireless_status_type);
s32 dev_wireless_get_simcard_status(void);
s32 dev_wireless_get_localip(u8 *ip_buf, u32 buf_size);
s32 dev_wireless_socket_create(u8 type, u8 mode, u8 *param, u16 port);
s32 dev_wireless_socket_get_status(s32 socketid);
s32 dev_wireless_set_socket_status(s32 socket_id, SOCKET_STATUS_E socket_status);
s32 dev_wireless_socket_send(s32 socketid, const u8 *wbuf, u32 wlen);
s32 dev_wireless_socket_recv(s32 socketid, u8 *rbuf, u32 rlen);
s32 dev_wireless_socket_close(s32 socketid);
s32 dev_wireless_get_csq(u8 *prssi, u16 *prxfull);
s32 dev_wireless_get_ccid(u8 *ccid_buf, u32 buf_size);
s32 dev_wireless_get_imsi(u8 *imsi_buf, u32 buf_size);
s32 dev_wireless_get_imei(u8 *imei_buf, u32 buf_size);
s32 dev_wireless_get_basestation_info(strBasestationInfo *basestation_info);
s32 dev_wireless_cfg_ntp_server(s8 *ntp_addr, s32 ntp_port);
s32 dev_wireless_qry_rtc(s8 *rtc_buff, s32 rtc_buff_len);
s32 dev_wireless_close_airplane(void);
s32 dev_wireless_open_airplane(void);
s32 dev_wireless_get_modulever(u8 *modulever_buf, u32 buf_size);
s32 dev_wireless_telephone_dial(u8 *num);
s32 dev_wireless_get_telephone_dial_cmd_result(void);
s32 dev_wireless_telephone_hangup(void);
s32 dev_wireless_telephone_answer(void);

s32 dev_wireless_ssl_setcert(u8 certtype, u8 *cert, u32 certlen);
s32 dev_wireless_ssl_open(s32 socketid);
s32 dev_wireless_ssl_close(void);
s32 dev_wireless_ssl_send(const u8 *wbuf, s32 wlen);
s32 dev_wireless_ssl_recv(u8 *rbuf, s32 rlen);
s32 dev_wireless_set_exGPIO(u8 exGPIO, u8 value);
s32 dev_wireless_play_audio(u8 type);
s32 dev_wireless_resume(void);
s32 dev_wireless_suspend(void); 
void dev_wireless_start_dial_log_collect(void);
void dev_wireless_print_dial_log(void);

void dev_wireless_set_power_flag(s32 power_flag);
s32 dev_wireless_get_power_flag(void);
s32 dev_wireless_ttsaudio_play(TTS_ENCODE_TYPE ttsencode, u8 *ttstext, u32 ttstextlen);
s32 dev_wireless_audio_isplaying(void);
s32 dev_wireless_audiofile_play(s8 *audio_string);
s32 dev_wireless_delete_audio_file(s8 *deletefilename);
s32 dev_wireless_get_audio_file_list(s8 filelist[][WIRELESS_AUDIO_FILE_NAME_MAX], s32 filelist_num);
s32 dev_wireless_add_audio_file(s32 file_index);
s32 dev_wireless_getaudiovolume(u32 *audiovolume);
s32 dev_wireless_setaudiovolume(u32 audiovolume);
s32 dev_wireless_setfota_upgrade_param(module_ota_param_t *module_ota_param);
s32 dev_wireless_getfota_status(OTA_STEP_E *ota_step, ota_process_value_t *ota_process_value);
void dev_wireless_ringanddtrpin_init(void);
void dev_wireless_ringanddtrpin_enter_sleep(void);
void dev_wireless_ringanddtrpin_exit_sleep(void);
static s32 wireless_audio_save_vol_to_file(void);
static s32 wireless_audio_get_vol_from_file(void);
s32 dev_wireless_is_support_ota(void);
void dev_wireless_module_reboot_handle(void);
s32 dev_wireless_module_is_upgrading(void);
u8 dev_get_machine_code(void);



#endif

