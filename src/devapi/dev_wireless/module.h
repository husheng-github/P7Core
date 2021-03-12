#ifndef __MODULE_H_
#define __MODULE_H_

#define AT_CMD_RES_END                    "\r\n"
#define WIRE_AT_CMD                       "AT"
#define WIRE_ATE_CMD                      "ATE"
#define WIRE_AT_GMR_CMD                   "AT+GMR"

#define WIRE_RESET_THRESHOLD_TIMES        5       //ATָ��ʧ�ܴ����Ĵ�����λģ�����ֵ
#define WIRE_SIMCARD_DETECT_TIMES         5       //sim����ⲻ��ʱ�ظ�����
#define WIRE_MODULE_DETECT_TIMES          10       //��Ϊ����ģ�鲻���ڵ���ֵ
#define WIRE_CHECK_NET_REG_TIMES          300      //���פ���Ƿ�ɹ��ĳ��Դ���
#define WIRE_AT_DEFAULT_TIMEOUT           1000
#define WIRE_AT_CMD_RESEND_TIMEOUT        1000    //ָ���ط��ļ��ʱ��
#define WIRE_AT_BOOTON_TIMEOUT            1000    //�ϵ�ߵ�ƽ����ʱ��
#define WIRE_AT_POWERON_TIMEOUT           800     //�����͵�ƽ����ʱ��
#define WIRE_AT_POWEROFF_WAIT_TIMEOUT     3000
#define WIRE_AT_REG_NET_TIMEOUT           30000
#define WIRE_AT_TCP_CONNECT_TIMEOUT       30000
#define WIRE_AT_TCP_SEND_TIMEOUT          10000
#define WIRE_AT_ACTIVE_PDP_TIMEOUT        90000   //����apn�󣬵ȴ���ȡip�ĳ�ʱʱ��
#define WIRE_AT_CTL_TIMEOUT               500      //����GPIO��BEEP�Ƚӿڣ�����Ҫ�ȷ���
#define WIRE_AT_SET_SLEEPTIME_TIMEOUT    300
#define WIRE_AT_GETAUDIOFILELIST_TIMEOUT  2000   //2S��ʱ
#define WIRE_AT_PLAYAUDIOFILE_TIMEOUT     1000   //����play file��ʱʱ��1S
#define WIRE_AT_AUDIOPROCESSING_DEFAULT_TIMEOUT     1000
#define WIRE_AT_SET_FOTAPARAM_TIMEOUT     3000


#define FIBOCOM_G500_SUPPORT   0


typedef enum{
    DETECTING_SIMCARD=0,
    NOWITH_SIMCARD=1,
    WITH_SIMCARD=2
}SIM_CARD_STATE_E;

#define MODULE_FUNC_IS_NULL(func) do{ \
    if(!func) \
    { \
        TRACE_ERR("func is null"); \
        ret = DDI_EDATA; \
        break; \
    } \
}while(0);
    
typedef struct{
    WIRE_MODULE_ID_E m_id;                      //ģ����
    s32 m_with_flow_control;               //true������  flaseû����
    s32 m_support_ota;
    s32 (*socket_get_list)(void);          
    s8 *get_inactive_pdp_cmd;              //��ȡȥ������������
    s8 *m_get_cellinfo_cmd;                //��ȡ��վ��Ϣ��cmd
    s8 *m_power_off_cmd;                   //����ģ��ػ�ָ��
    s8 *m_cfg_net_reg_format_cmd;          //����creg����ظ�ʽ��ָ��
    s8 *m_get_local_ip_cmd;                //��ȡ����ip��������
    s8 *m_get_local_ip_res_kw;             //��ȡ����ip�������ַ��ص�ͷ����ʶ
    s8 *m_cfg_rec_data_format_cmd;         //���ý������ݷ��ص�������
    s8 *m_get_imsi_cmd;                    //��ȡimsi
    s8 *m_get_imsi_reg_kw;                 //��ȡimsi���عؼ���
    s8 *m_get_2G_net_reg_cmd;              //��ȡ2Gע��״̬
    s8 *m_get_4G_net_reg_cmd;              //��ȡ4Gע��״̬
    s8 *m_set_2G_reg_param_cmd;            //����creg=2����
    s8 *m_set_4G_reg_param_cmd;            //����creg=2����
    s8 *m_get_net_reg_res_kw;              //��ȡע��״̬���ص�ͷ����ʶ
    s8 *m_get_csq_cmd;                     //��ȡ�ź�ǿ��
    s8 *m_get_csq_res_kw;                  //��ȡ�ź�ǿ�ȷ��ص�ͷ����ʶ
    s8 *m_check_rplmn_cmd;                 //��ȡrplmn����ָ��
    s8 *m_clear_rplmn_cmd;                 //��ȡ���rplmn����ָ��
    s8 *m_imei_cmd;                        //��ȡimei��atָ��
    s8 *m_check_workmod_cmd;               //��ȡ����ģʽ
    s8 *m_set_workmod_cmd;                 //���ù���ģʽ
    s8 *m_ccid_cmd;                        //��ȡccid��atָ��
    s8 *m_ccid_cmd_res_kw;                 //ccid���ص�ͷ����ʶ
    s8 *m_simcard_state_cmd;               //��ȡsim��״ָ̬��
    s8 *m_simcard_state_cmd_res_kw;        //sim��״̬�п�
    s8 *m_tcp_rec_cmd_head_kw;             //tcp�������ݵ�ǰ׺
    s8 *m_pdp_report_head_kw;              //pdp״̬�������ص�ǰ׺
    s8 *m_connect_report_head_kw;          //���̨�������ֽ���������ص�ǰ׺
    s8 *m_socket_report_head_kw;           //socket״̬�ı��������ص�ǰ׺
    s8 *m_set_useextamp_cmd;               //���ù��Źܽ�����
    s8 *m_cfg_audio_play_path_cmd;     //���ò���ͨ��
    s8 *m_cfg_audio_mut_cmd;               //���þ�������
    s8 *m_close_audio_play_sleep_cmd;
    //sxl20200803 add
    s8 *m_uartring_mode_cmd;
    s8 *m_gtlpm_mode_cmd;
    s8 *m_set_csclk_cmd;
    
    s32 (*net_reg_report_handle)(s8 *buff);
    s32 (*pdp_report_handle)(s8 *buff);
    s32 (*tcp_connect_report_handle)(s8 *buff);
    s32 (*socket_report_handle)(s8 *buff);

    s32 (*cfg_flow_control)(AT_API_GET_TYPE_E at_api_get_type);
    
    s32 (*get_imei)(u8 *imei_buf, u32 buf_size, AT_API_GET_TYPE_E at_api_get_type);
    wire_thread_handle_pfunc get_imei_callback;

    s32 (*check_work_mode)(AT_API_GET_TYPE_E at_api_get_type);
    wire_thread_handle_pfunc check_work_mode_callback;

    s32 (*set_work_mode)(AT_API_GET_TYPE_E at_api_get_type);
    
    s32 (*get_simcard_state)(AT_API_GET_TYPE_E at_api_get_type);
    wire_thread_handle_pfunc get_simcard_state_callback;

    s32 (*check_rplmn)(AT_API_GET_TYPE_E at_api_get_type);
    wire_thread_handle_pfunc check_rplmn_callback;

    s32 (*clear_rplmn)(AT_API_GET_TYPE_E at_api_get_type);
        
    s32 (*get_ccid)(u8 *ccid_buf, u32 buf_size, AT_API_GET_TYPE_E at_api_get_type);
    wire_thread_handle_pfunc get_ccid_callback;

    s32 (*get_csq)(u8 *csq, u16 *bear_rate, AT_API_GET_TYPE_E at_api_get_type);
    wire_thread_handle_pfunc get_csq_callback;

    s8 *(*get_reg_cmd)(void);
    s32 (*get_net_reg)(AT_API_GET_TYPE_E at_api_get_type);
    wire_thread_handle_pfunc get_net_reg_callback;

    s8 *(*get_set_reg_param_cmd)(void);
    s32 (*set_reg_param)(AT_API_GET_TYPE_E at_api_get_type);

    s32 (*get_imsi)(u8 *buf, u32 buf_size, AT_API_GET_TYPE_E at_api_get_type);
    wire_thread_handle_pfunc get_imsi_callback;

    s32 (*cfg_rec_data_format)(AT_API_GET_TYPE_E at_api_get_type);
    wire_thread_handle_pfunc cfg_rec_data_format_callback;

    s32 (*get_local_ip)(s8 *buf, s32 buf_size, AT_API_GET_TYPE_E at_api_get_type);
    wire_thread_handle_pfunc get_local_ip_callback;

    s8* (*get_set_apn_cmd)(void);
    s32 (*set_apn)(s8 *apn, s8 *username, s8 *password, AT_API_GET_TYPE_E at_api_get_type);
    wire_thread_handle_pfunc set_apn_callback;

    s32 (*inactive_pdp)(AT_API_GET_TYPE_E at_api_get_type);

    s32 (*get_cellinfo)(strBasestationInfo *basestation_info, s32 cell_num, AT_API_GET_TYPE_E at_api_get_type);
    wire_thread_handle_pfunc get_cellinfo_callback;

    s32 (*open_airplane)(AT_API_GET_TYPE_E at_api_get_type);
    s32 (*close_airplane)(AT_API_GET_TYPE_E at_api_get_type);
    s32 (*telephone_dial)(u8 *num, AT_API_GET_TYPE_E at_api_get_type);
    s32 (*telephone_hangup)(AT_API_GET_TYPE_E at_api_get_type);
    s32 (*cfg_ntp_server)(s8 *ntp_addr, s32 ntp_port, AT_API_GET_TYPE_E at_api_get_type);
    s32 (*qry_rtc)(s8 *rtc_buff, s32 rtc_buff_len, AT_API_GET_TYPE_E at_api_get_type);

    s32 (*socket_create)(s32 socket_id, SOCKET_TYPE type, u8 *param, u16 port, AT_API_GET_TYPE_E at_api_get_type);
    s32 (*get_valid_socketid_list)(s32 *sock_list, s32 socket_list_num, AT_API_GET_TYPE_E at_api_get_type);
    s32 (*socket_send)(s32 socket_id, const u8 *buff, s32 len, AT_API_GET_TYPE_E at_api_get_type);
    s32 (*socket_close)(s32 socketid, AT_API_GET_TYPE_E at_api_get_type);

    s32 (*cfg_exGPIO)(u8 exGPIO, u8 cfg, AT_API_GET_TYPE_E at_api_get_type);
    s32 (*set_exGPIO)(u8 exGPIO, u8 value, AT_API_GET_TYPE_E at_api_get_type);
    s32 (*play_audio)(u8 type, AT_API_GET_TYPE_E at_api_get_type);
    s32 (*get_appver)(AT_API_GET_TYPE_E at_api_get_type);
    wire_thread_handle_pfunc get_appver_callback;
    
    s32 (*suspend)(AT_API_GET_TYPE_E at_api_get_type);
    s32 (*resume)(AT_API_GET_TYPE_E at_api_get_type);
    s32 (*ssl_set_mod)(SSL_AUTHEN_MOD_t mod, AT_API_GET_TYPE_E at_api_get_type);
    s32 (*ssl_setver)(u8 ver, AT_API_GET_TYPE_E at_api_get_type);
    s32 (*ssl_setcert)(u8 certtype, u8 *cert, u32 certlen, AT_API_GET_TYPE_E at_api_get_type);
    s32 (*ssl_filte)(s32 type, AT_API_GET_TYPE_E at_api_get_type);

    //tts audio play
    s32 (*audiofile_play)(s8 *audio_string);
    s32 (*ttsaudio_play)(u8 ttsencode, u8 *ttstext, u32 ttstextlen);
    s32 (*audio_end_report_handle)(s8 *buf);
    s32 (*audio_get_audio_file_list)(s8 filelist[][WIRELESS_AUDIO_FILE_NAME_MAX], s32 filelist_num);
    s32 (*delete_audio_file)(s8 *deletefilename);
    s32 (*add_audio_file)(s32 file_index);
    s32 (*get_audio_volume)(u32 *audiovolume);
    s32 (*set_audio_volume)(u32 audiovolume, AT_API_GET_TYPE_E at_api_get_type);
    s32 (*set_useextamp)(AT_API_GET_TYPE_E at_api_get_type);
    s32 (*cfg_audio_play_path)(AT_API_GET_TYPE_E at_api_get_type);
    s32 (*cfg_audio_mut)(AT_API_GET_TYPE_E at_api_get_type);
    s32 (*close_audio_play_sleep)(void);
    s32 (*setfota_upgrade_param)(s8 fota_type, s8 *url, s8 *filename,s8 *username, s8 *password);
    
    s32 (*fota_processing_report_handle)(s8 *buf);
    s32 (*power_off)(AT_API_GET_TYPE_E at_api_get_type);
    s32 (*uartring_mode)(AT_API_GET_TYPE_E at_api_get_type);
    s32 (*set_gtlpm_mode)(AT_API_GET_TYPE_E at_api_get_type);
    s32 (*set_csclk)(AT_API_GET_TYPE_E at_api_get_type);
    wire_thread_handle_pfunc power_off_callback;
}wireless_module_t;

s32 MIL_net_reg_report_handle(s8 *buff);
s32 MIL_pdp_report_handle(s8 *buff);
s32 MIL_tcp_connect_report_handle(s8 *buff);
s32 MIL_socket_report_handle(s8 *buff);
s8 *MIL_get_imei_cmd(void);
s32 MIL_get_imei_callback(u8 result, u8 *src, u16 srclen);
s32 MIL_get_imei(u8 *imei_buf, u32 buf_size, AT_API_GET_TYPE_E at_api_get_type);
s8 *MIL_get_check_work_mode_cmd(void);
s32 MIL_check_work_mode(AT_API_GET_TYPE_E at_api_get_type);
s32 MIL_check_work_mode_callback(u8 result, u8 *src, u16 srclen);
s8 *MIL_get_set_work_mode_cmd(void);
s32 MIL_set_work_mode(AT_API_GET_TYPE_E at_api_get_type);
s8 *MIL_check_rplmn_cmd(void);
s32 MIL_check_rplmn_callback(u8 result, u8 *src, u16 srclen);
s32 MIL_check_rplmn(AT_API_GET_TYPE_E at_api_get_type);
s8 *MIL_clear_rplmn_cmd(void);
s32 MIL_clear_rplmn(AT_API_GET_TYPE_E at_api_get_type);
s8 *MIL_simcard_state_cmd(void);
s32 MIL_get_simcard_state_callback(u8 result, u8 *src, u16 srclen);
s32 MIL_get_simcard_state(AT_API_GET_TYPE_E at_api_get_type);
s8 *MIL_ccid_cmd(void);
s32 MIL_get_ccid_callback(u8 result, u8 *src, u16 srclen);
s32 MIL_get_ccid(u8 *buf, u32 buf_size, AT_API_GET_TYPE_E at_api_get_type);
s8 *MIL_get_csq_cmd(void);
s32 MIL_get_csq_callback(u8 result, u8 *src, u16 srclen);
s32 MIL_get_csq(u8 *csq, u16 *bear_rate, AT_API_GET_TYPE_E at_api_get_type);
s8 *MIL_get_net_reg_cmd(void);
s32 MIL_get_net_reg_callback(u8 result, u8 *src, u16 srclen);
s32 MIL_get_net_reg(AT_API_GET_TYPE_E at_api_get_type);
s8 *MIL_get_imsi_cmd(void);
s32 MIL_get_imsi_callback(u8 result, u8 *src, u16 srclen);
s32 MIL_get_imsi(u8 *buf, u32 buf_size, AT_API_GET_TYPE_E at_api_get_type);
s32 MIL_get_cellinfo(strBasestationInfo *basestation_info, s32 cell_num, AT_API_GET_TYPE_E at_api_get_type);
s32 MIL_open_airplane(AT_API_GET_TYPE_E at_api_get_type);
s32 MIL_close_airplane(AT_API_GET_TYPE_E at_api_get_type);
s32 MIL_telephone_dial(u8 *num, AT_API_GET_TYPE_E at_api_get_type);
s32 MIL_telephone_hangup(AT_API_GET_TYPE_E at_api_get_type);
s32 MIL_cfg_ntp_server(s8 *ntp_addr, s32 ntp_port, AT_API_GET_TYPE_E at_api_get_type);
s32 MIL_qry_rtc(s8 *rtc_buff, s32 rtc_buff_len, AT_API_GET_TYPE_E at_api_get_type);
s8 *MIL_get_cfg_rec_data_format_cmd(void);
s32 MIL_cfg_rec_data_format_callback(u8 result, u8 *src, u16 srclen);
s32 MIL_cfg_rec_data_format(AT_API_GET_TYPE_E at_api_get_type);
s8 *MIL_get_local_ip_cmd(void);
s32 MIL_get_local_ip_callback(u8 result, u8 *src, u16 srclen);
s32 MIL_get_local_ip(s8 *buf, s32 buf_size, AT_API_GET_TYPE_E at_api_get_type);
s8 *MIL_get_set_apn_cmd(void);
s32 MIL_set_apn_callback(u8 result, u8 *src, u16 srclen);
s32 MIL_set_apn(s8 *apn, s8 *username, s8 *password, AT_API_GET_TYPE_E at_api_get_type);
s8 *MIL_get_inactive_pdp_cmd(void);
s32 MIL_inactive_pdp(AT_API_GET_TYPE_E at_api_get_type);
s32 MIL_socket_create(s32 socket_id, SOCKET_TYPE type, u8 *param, u16 port, AT_API_GET_TYPE_E at_api_get_type);
s32 MIL_get_valid_socketid_list(s32 *sock_list, s32 socket_list_num, AT_API_GET_TYPE_E at_api_get_type);
s32 MIL_socket_send(s32 socket_id, const u8 *buff, s32 len, AT_API_GET_TYPE_E at_api_get_type);
s32 MIL_socket_close(s32 socket_id, AT_API_GET_TYPE_E at_api_get_type);
s32 MIL_power_off_callback(u8 result, u8 *src, u16 srclen);
s32 MIL_power_off(AT_API_GET_TYPE_E at_api_get_type);
s32 MIL_cfg_exGPIO(u8 exGPIO, u8 cfg, AT_API_GET_TYPE_E at_api_get_type);
s32 MIL_set_exGPIO(u8 exGPIO, u8 value, AT_API_GET_TYPE_E at_api_get_type);
s32 MIL_play_audio(u8 type, AT_API_GET_TYPE_E at_api_get_type);
s32 MIL_get_appver_callback(u8 result, u8 *src, u16 srclen);
s32 MIL_get_appver(AT_API_GET_TYPE_E at_api_get_type);
s32 MIL_suspend(AT_API_GET_TYPE_E at_api_get_type);
s32 MIL_resume(AT_API_GET_TYPE_E at_api_get_type);
s32 MIL_ssl_set_mod(SSL_AUTHEN_MOD_t mod, AT_API_GET_TYPE_E at_api_get_type);
s32 MIL_ssl_setver(u8 ver, AT_API_GET_TYPE_E at_api_get_type);
s32 MIL_ssl_setcert(u8 certtype, u8 *cert, u32 certlen, AT_API_GET_TYPE_E at_api_get_type);
s32 MIL_ssl_open(s32 socketid);
s8 *MIL_get_tcp_recv_cmd_head_kw(void);
s32 MIL_ttsaudio_play(u8 ttsencode, u8 *ttstext, u32 ttstextlen);
wireless_module_t *dev_wireless_module_create(s8 *module_ver);
s32 MIL_audioend_report_handle(s8 *buff);
s32 MIL_get_audio_file_list(s8 filelist[][WIRELESS_AUDIO_FILE_NAME_MAX], s32 filelist_num);
s32 MIL_add_audio_file(s32 file_index);
s32 MIL_audiofile_play(s8 *audio_string);
s32 MIL_get_audio_volume(u32 *audiovolume);
s32 MIL_set_audio_volume(u32 audiovolume, AT_API_GET_TYPE_E at_api_get_type);
s8* MIL_get_set_useextamp_cmd(void);
s32 MIL_set_useextamp(AT_API_GET_TYPE_E at_api_get_type);
s8* MIL_get_cfg_audio_play_path_cmd(void);
s32 MIL_cfg_audio_play_path(void);
s8 *MIL_get_cfg_audio_mut(void);
s32 MIL_cfg_audio_mut(void);
s32 MIL_close_audio_play_sleep(void);
u8* MIL_get_close_audio_play_sleep_cmd(void);
s32 MIL_setfota_upgrade_param(s8 fota_type, s8 *url, s8 *filename,s8 *username, s8 *password);
s32 MIL_set_gtlpm_mode(AT_API_GET_TYPE_E at_api_get_type);
s32 MIL_set_csclk(AT_API_GET_TYPE_E at_api_get_type);
s32 MIL_set_net_cereg(AT_API_GET_TYPE_E at_api_get_type);
s8 *MIL_setcereg_cmd(void);
s8* MIL_get_uartring_mode_cmd(void);
s8* MIL_get_set_csclk_cmd(void);
s8* MIL_get_gtlpm_mode_cmd(void);
s32 MIL_get_module_id(u8 *module_id);
s32 MIL_is_support_ota(void);
s32 MIL_fota_processing_report_handle(s8 *buff);


#endif
