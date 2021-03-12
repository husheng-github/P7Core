#ifndef __DEV_WIRELESS_THREAD_H
#define __DEV_WIRELESS_THREAD_H

#define WIRE_SPECIAL_BIT0               0x01       //����ָ�����ݳ���
#define WIRE_SPECIAL_SEND_DATA_LEN      0x02       //��������ģʽ
#define WIRE_SPECIAL_READY              0x04       //�ȴ�READY��־
#define WIRE_SPECIAL_RCVCMD             0x08       //����ָ��������պ�������


typedef enum{
    WIRE_AT_RESULT_MODULE_READY = 0,    //�ϵ緵�ص�at command ready
    WIRE_AT_RESULT_OK,
    WIRE_AT_RESULT_ERROR,
    WIRE_AT_RESULT_TIMEROUT,
    WIRE_AT_RESULT_OTHER,
    WIRE_AT_RESULT_DAIL_NOANSWER,
    //��������
    WIRE_AT_RESULT_REPORT_MIPCALL,
    WIRE_AT_RESULT_REPORT_MIPOPEN,
    WIRE_AT_RESULT_REPORT_MIPSTAT,
    WIRE_AT_RESULT_CONTINUE,            //�����������ݣ����TCP�������ݡ�����֤��ȣ��ڷ��������
                                        //��Ҫ�ȴ�">"��������ַ�
    
    //---------------------------------
    WIRE_AT_RESULT_NONE = 0x7F,
}WIRE_AT_RESULT_E;

//����pdp��Ӧ���Ӳ���
typedef enum{
    WIRE_ACTIVE_NETWORK_STEP_GET_IP,            //��ȡip
    WIRE_ACTIVE_NETWORK_STEP_GET_APN,           //��ȡapn
    WIRE_ACTIVE_NETWORK_STEP_SET_APN,           //����apn
    WIRE_ACTIVE_NETWORK_STEP_CHECK_PDP          //������缤�����
}WIRE_ACTIVE_NETWORK_STEP_E;

//ȥ����pdp��Ӧ���Ӳ���
typedef enum{
    WIRE_INACTIVE_NETWORK_STEP_GET_IP,             //��ȡip
    WIRE_INACTIVE_NETWORK_STEP_CLOSE_PDP,          //�ر���������
    WIRE_INACTIVE_NETWORK_STEP_CHECK_RESULT        //�ر���������
}WIRE_INACTIVE_NETWORK_STEP_E;


//Ѱ��task��Ӧ���Ӳ���
typedef enum{
    WIRE_SEARCHING_NETWORK_STEP_CHECK_CARD,     //���sim��
    WIRE_SEARCHING_NETWORK_CHECK_RPLMN,         //ȷ��RPLMN�Ƿ���ʵ������
    WIRE_SEARCHING_NETWORK_CLEAR_RPLMN,         //���ʵ������
    WIRE_SEARCHING_NETWORK_STEP_GET_CCID,       //��ȡccid
    WIRE_SEARCHING_NETWORK_STEP_CHECK_CSQ,      //����ź�ǿ��
    WIRE_SEARCHING_NETWORK_STEP_SETCEREG,       //sxl20200902�������ź�ʱ�ϱ�״̬
    WIRE_SEARCHING_NETWORK_CHECK_REG,           //���ע��
    WIRE_SEARCHING_NETWORK_GET_IMSI,            //��ȡimsi
    WIRE_SEARCHING_NETWORK_SET_REG_PARAM,       //����creg����
    WIRE_SEARCHING_NETWORK_SET_4G_REG_PARAM,    //������CEREG=2������4G������2G����ע����
    WIRE_SEARCHING_NETWORK_GET_MAIL_CELL,       //��ȡ����վ��Ϣ
    WIRE_SEARCHING_NETWORK_STEP_SET_FMT        //�������ݸ�ʽ
}WIRE_SEARCHING_NETWORK_STEP_E;

//��ʼ��task��Ӧ���Ӳ���
typedef enum{
    WIRE_INIT_STEP_AT_TEST,
    WIRE_INIT_STEP_ATE,
    WIRE_INIT_GET_MODEL_VER,
    WIRE_INIT_OPEN_FLOW_CTL,          //������
    WIRE_INIT_GET_IMEI,
    WIRE_INIT_CHECK_WORK_MODE,       //ȷ�����е���ʽ
    WIRE_INIT_SET_WORK_MODE,         //�޸�������ʽ
    WIRE_INIT_GPIO_INIT,             //���ù�������ģ����豸��gpio����
    WIRE_INIT_GPIO_SET_VAL,          //���ù�������ģ����豸��gpioֵ
    WIRE_INIT_GET_APP_VER,            //��ȡapp�İ汾
    WIRE_INIT_SET_USE_EXTERANL_AMP,   //����ʹ���ⲿ����
    WIRE_INIT_CFG_AUDIO_PLAY_PATH,    //���ò��Ŵ�speekerͨ������
    WIRE_INIT_CLOSE_AUDIO_PLAY_SLEEP,  //�ر�rda����������ǰ������
    WIRE_INIT_CFG_AUDIO_MUT,           //������˷羲����speeker�Ǿ���
    WIRE_INIT_CFG_VOL,                  //����������С
    WIRE_INIT_SET_USE_UARTRING,       //����Ҫ��Ҫʹ��uart ring
    WIRE_INIT_SET_CSCLK,
    WIRE_INIT_SET_GTLPM_MODE           //AT+GTLPMMODE����
}WIRE_TASK_INIT_STEP_E;

//�ϵ翪��task��Ӧ���Ӳ���
typedef enum{
    WIRE_ONBOOT_STEP_BOOT,
    WIRE_ONBOOT_STEP_BOOT1,      //���T8����,�ϵ�ȴ�һ��ʱ���򿪴���
    WIRE_ONBOOT_STEP_POWER
}WIRE_TASK_ONBOOT_STEP_E;

//��ȡ��վ��Ϣ
typedef enum{
    WIRE_GET_CELL_STEP_MAIN_HEAD,
    WIRE_GET_CELL_STEP_MAIN_INFO,
    WIRE_GET_CELL_STEP_NEAR_HEAD,
    WIRE_GET_CELL_STEP_NEAR_INFO,
}WIRE_GET_CELL_STEP_E;

//������ģʽ�Ӳ���
typedef enum{
    WIRE_OA_STEP_OPEN_AIRPLANE,            //�򿪷���ģʽ
    WIRE_OA_STEP_CHECK_RESULT              //������ģʽ�Ƿ�򿪳ɹ�
}WIRE_OPEN_AIRPLANE_STEP_E;

//����task
typedef enum
{
    WIRE_TASK_FREE = 0,
    WS_TASK_MODULES_ONBOOT = 1,             //ģ�鿪��
    WS_TASK_MODULES_INIT = 2,               // ģ���ʼ��
    WS_TASK_SEARCHING_NETWORK = 3,          // Ѱ����
    WS_TASK_ACTIVE_NETWORK = 4,             // ��������ģʽ
    WS_TASK_INACTIVE_NETWORK,               // ȥ��������ģʽ
    WS_TASK_INACTIVE_NETWORK_APP,           // ȥ��������ģʽ---app�����
    WS_TASK_POWEROFF_MODULE,                //ģ���µ�--�ػ�ʱʹ��
    WS_TASK_RESET_MODULE,                   //ģ���µ�--�ڲ���λʱʹ��
    WS_TASK_GET_CSQ,                        //��ȡģ���ź�ǿ��
    WS_TASK_GET_IMEI,                       //��ȡģ��imei
    WS_TASK_GET_IMSI,                       //��ȡģ��imsi
    WS_TASK_GET_CCID,                       //��ȡģ��CCID
    WS_TASK_GET_MODULEVER,                  //��ȡģ��汾
    WS_TASK_GET_LOCAL_IP,                   //��ȡģ�鱾��ip
    WS_TASK_CREATE_SOCKET,
    WS_TASK_GET_SOCKET_LIST,                //��ȡ���õ�socketlist
    WS_TASK_SOCKET_SENDBUFF_LEN,            //�������ݳ���   L610������
    WS_TASK_SOCKET_SENDBUFF,
    WS_TASK_SOCKET_RECVDATA,
    WS_TASK_SOCKET_CLOSE,
    WS_TASK_TELEPHONE_DAIL,
    WS_TASK_TELEPHONE_HANGUP,
    WS_TASK_GET_CELL_INFO,
    WS_TASK_OPEN_AIRPLANE,
    WS_TASK_CLOSE_AIRPLANE,
    WS_TASK_CHECK_REG,
    WS_TASK_CHECK_CS_REG,
    WS_TASK_GET_AIRPLANE_FLAG,
    WS_TASK_CFG_NTP_SERVER,
    WS_TASK_QRY_RTC,
    WS_TASK_CHECK_CMD,                    //���ָ���Ƿ��쳣
    WS_TASK_GET_WORK_MODE,           //��ȡģʽ
    WS_TASK_SET_WORK_MODE,           //���ý�2Gģʽ
    WS_TASK_GET_HOSTBYNAME,          //��������
    
    WS_TASK_ENTER_SLEEP,             //��ģ�����͹���
    WS_TASK_WAKEUP,                  //����ģ��
    WS_TASK_SET_GPIOOUT,             //GPIO�ܽ����
    WS_TASK_PLAYAUDIO ,              //������Ƶ
    WS_TASK_CTLCAMERA ,              //��������ͷ
    WS_TASK_GET_APPVER,                //��ȡAPP�汾
    WS_TASK_SSL_SETMOD,                //����SSL MOD
    WS_TASK_SSL_SETVER,                //����SSL �汾
    WS_TASK_SSL_SETCERT,               //����SSL֤��
    WS_TASK_SSL_FILTE,                 //����SSL����Э��
    WS_TASK_GET_MODEMAUDOFILELIST,     //��ȡmodem audio�ļ��б�
    WS_TASK_DELETE_AUDIOFILE,          //ɾ��audio file
    WS_TASK_ADD_AUDIOFILE_LEN,         //����audio file����
    WS_TASK_ADD_AUDIOFILE_DATA,         //����audio file����
    WS_TASK_PLAY_AUDIOFILE,            //������Ƶ�ļ�
    WS_TASK_GETSET_AUDIOVOLUME,         //��ȡ���ò�������
    WS_TASK_GETSET_AUDIOPLAYPATH,
    WS_TASK_PLAY_AUDIO_STOP,
    WS_TASK_SET_EXTAMP_CONTROLPIN,      //�����ⲿ���ſ��ƽ�
    WS_TASK_SET_FOTA_PARAM,             //����FOTA����
    WS_TASK_WRITE_8960INFO_TO_RPLMN,   //���в��ǵ���Ϣд��RPLMN������8960ǰ����
    WS_TASK_CLEAR_RPLMN,               //���RPLMN
    
    WIRE_SLEEP_STEP = 0xff,               //������ִ��
    
}WIRE_TASK_ID;

//����socket���ݵ���step
typedef enum{
    WIRE_SOCKET_SEND_STEP_TO_BUFF,
    WIRE_SOCKET_SEND_STEP_PUSH
}WIRE_SOCKET_SEND_STEP_E;

typedef s32 (*wire_thread_handle_pfunc)(u8 result, u8 *src, u16 srclen);
typedef struct
{
    WIRE_TASK_ID  m_task_id;
    void* m_ap;                         //ָ�룬����ָ������ָ��
    u8 m_step;                          //��Ӧ�����µ�״̬
    WIRE_AT_RESULT_E m_result;          //����ִ�н��
    u8 m_lparam;                        //����m_task��m_step���ݵĲ���
    u8 m_wparam;                        //����m_task��m_step���ݵĲ�����(������¼ѭ������)
    u16 m_param1;                        //�������ݹ��̲���(����)
    u32 m_param2;                        //�������ݹ��̲���(����)
    u32 m_addr_param;                   //���ݵ�ַ
    wire_thread_handle_pfunc m_func;    //�������ִ�к���
    volatile u16 m_timeover;            //��ʱʱ��(��λ: WIRE_TIMER_BASE ms)
    u16 m_timeroutbak;                  //�����������ϴ�����ʱ�����ᵱǰָ�ʱʱ��
}wireless_thread_node_param_t;

void dev_wireless_thread_node_param_init(void);
wireless_thread_node_param_t *dev_wireless_get_thread_node_param(void);
void dev_wireless_set_thread_node_param(wireless_thread_node_param_t *wireless_thread_node_param);
void dev_wireless_thread_set_timeout(u32 ms_timeout);
void dev_wireless_thread_set_param(WIRE_TASK_ID task_id, wire_thread_handle_pfunc hand_func, u32 timeover);
void dev_wireless_thread_param_clean(void);
s32 dev_wireless_thread_task_is_change(WIRE_TASK_ID task_id);
void dev_wireless_thread_set_result(WIRE_AT_RESULT_E result);
s32 dev_wireless_thread_run_switch(s32 onoff);
s32 dev_wireless_thread_is_free(void);
void dev_wireless_thread(void);

#endif

