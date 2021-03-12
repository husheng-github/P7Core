#ifndef __DEV_WIFI_THREAD_H
#define __DEV_WIFI_THREAD_H

#define WIFI_SPECIAL_BIT0            0x01       //����ָ�����ݳ���
#define WIFI_SPECIAL_SEND_DATA_LEN   0x02       //��������ģʽ
#define WIFI_SPECIAL_READY     0x04       //�ȴ�READY��־
#define WIFI_SPECIAL_RCVCMD    0x08       //����ָ��������պ�������

typedef enum{
    WIFI_AT_RESULT_MODULE_READY = 0,    //�ϵ緵�ص�at command ready
    WIFI_AT_RESULT_OK,
    WIFI_AT_RESULT_ERROR,
    WIFI_AT_RESULT_TIMEROUT,
    WIFI_AT_RESULT_OTHER,
    WIFI_AT_RESULT_DAIL_NOANSWER,
    //��������
    WIFI_AT_RESULT_REPORT_MIPCALL,
    WIFI_AT_RESULT_REPORT_MIPOPEN,
    WIFI_AT_RESULT_REPORT_MIPSTAT,
    WIFI_AT_RESULT_CONTINUE,            //�����������ݣ����TCP�������ݡ�����֤��ȣ��ڷ��������
                                        //��Ҫ�ȴ�">"��������ַ�
    
    //---------------------------------
    WIFI_AT_RESULT_DNSFAIL,             //��������ʧ��
    WIFI_AT_RESULT_SOCKET_ALREAYDY_EXIT,  //�׽��ִ���
    WIFI_AT_RESULT_NONE = 0x7F,
}WIFI_AT_RESULT_E;

//Ѱ��task��Ӧ���Ӳ���
typedef enum{
    WIFI_AP_CONNECTTING_STEP1,     
}WIFI_AP_CONNECTTING_E;

//��ʼ��task��Ӧ���Ӳ���
typedef enum{
    WIFI_INIT_STEP_AT_TEST,
    WIFI_INIT_STEP_ATE,
    WIFI_INIT_SET_HARDFLOW,           //��������
    WIFI_INIT_GET_MODEL_VER,          //��ȡ�̼��汾
    WIFI_INIT_SET_CUR_MODE,           //���õ�ǰ�Ĺ���ģʽ
    WIFI_INIT_SET_TRANS_MODE,         //���ô���ģʽ
    WIFI_INIT_SET_AUTO_CONN,          //�����ϵ�ʱ�Ƿ��Զ������ȵ�
    WIFI_INIT_SET_MUX,                //��������ģʽ
    WIFI_INIT_SET_SCAN_RES,           //����ɨ���ȵ�ķ��ظ�ʽ
    WIFI_INIT_SET_WAKEUP_GPIO,        //���û���ģʽΪ�ߵ�ƽ����light sleep
    WIFI_INIT_DISCONNECT_AP,           //�̼�Ĭ������ʱ����trendit�ȵ�
    WIFI_INIT_SET_SLEEP_MODE,          //��������ģʽ
    WIFI_INIT_SET_MAX_CONN,            //�����������    ---web����������֮�������
    WIFI_INIT_OPEN_TCP_SERVER,         //����tcp server
    WIFI_INIT_STET_SOFTAP_NAME,        //���÷����ap����
    WIFI_INIT_GET_SOFTAP_NAME          //��ѯap����
}WIFI_TASK_INIT_STEP_E;

//�ϵ翪��task��Ӧ���Ӳ���
typedef enum{
    WIFI_ONBOOT_STEP_BOOT,
    WIFI_ONBOOT_STEP_POWER
}WIFI_TASK_ONBOOT_STEP_E;

//����task
typedef enum
{
    WIFI_TASK_FREE = 0,
    WIFI_TASK_MODULES_ONBOOT = 1,             //ģ�鿪��
    WIFI_TASK_MODULES_INIT = 2,               // ģ���ʼ��
    WIFI_TASK_AP_CONNETING = 3,               // �����ȵ�
    WIFI_TASK_AP_DISCONNECT,                  // �Ͽ��ȵ�
    WIFI_TASK_POWEROFF_MODULE,                //ģ���µ�--�ػ�ʱʹ��
    WIFI_TASK_RESET_MODULE,                   //ģ���µ�--�ڲ���λʱʹ��
    WIFI_TASK_CREATE_SOCKET,
    WIFI_TASK_GET_SOCKET_LIST,                //��ȡ���õ�socketlist
    WIFI_TASK_SOCKET_SEND_DATALEN,
    WIFI_TASK_SOCKET_SENDBUFF,
    WIFI_TASK_SOCKET_RECVDATA,
    WIFI_TASK_SOCKET_CLOSE,
    WIFI_TASK_CFG_NTP_SERVER,
    WIFI_TASK_QRY_RTC,
    WIFI_TASK_CHECK_CMD,                    //���ָ���Ƿ��쳣
    WIFI_TASK_GET_CUR_AP_INFO,              //��ȡ��ǰ����ap����Ϣ
    WIFI_TASK_SCAN_AP,                      //ɨ���ȵ�
    
    WIFI_TASK_ENTER_SLEEP,             //��ģ�����͹���
    WIFI_TASK_WAKEUP,                  //����ģ��

    WIFI_TASK_GET_NETINFO,              //��ȡ������Ϣ

    WIFI_TASK_STOP_AIRKISS,                  //�ر�airkiss
    WIFI_TASK_GET_NET_STATE,                 //��ȡ��������״̬

    WIFI_TASK_CLEAR_RF_PARAM,
    
    WIFI_SLEEP_STEP = 0xff,               //������ִ��
}WIFI_TASK_ID;

//����socket���ݵ���step
typedef enum{
    WIFI_SOCKET_SEND_STEP_TO_BUFF,
    WIFI_SOCKET_SEND_STEP_PUSH
}WIFI_SOCKET_SEND_STEP_E;

typedef s32 (*wifi_thread_handle_pfunc)(u8 result, u8 *src, u16 srclen);
typedef struct
{
    WIFI_TASK_ID  m_task_id;
    void* m_ap;                         //ָ�룬����ָ������ָ��
    u8 m_step;                          //��Ӧ�����µ�״̬
    WIFI_AT_RESULT_E m_result;          //����ִ�н��
    u8 m_lparam;                        //����m_task��m_step���ݵĲ���
    u8 m_wparam;                        //����m_task��m_step���ݵĲ�����(������¼ѭ������)
    u16 m_param1;                        //�������ݹ��̲���(����)
    u16 m_param2;                        //�������ݹ��̲���(����)
    u32 m_addr_param;                    //�������ݹ��̲���(����)
    wifi_thread_handle_pfunc m_func;    //�������ִ�к���
    volatile u16 m_timeover;            //��ʱʱ��(��λ: WIFI_TIMER_BASE ms)
    u16 m_timeroutbak;                  //�����������ϴ�����ʱ�����ᵱǰָ�ʱʱ��
}wifi_thread_node_param_t;

void dev_wifi_thread_node_param_init(void);
wifi_thread_node_param_t *dev_wifi_get_thread_node_param(void);
void dev_wifi_set_thread_node_param(wifi_thread_node_param_t *wifi_thread_node_param);
void dev_wifi_thread_set_timeout(u32 ms_timeout);
void dev_wifi_thread_set_param(WIFI_TASK_ID task_id, wifi_thread_handle_pfunc hand_func, u32 timeover);
void dev_wifi_thread_param_clean(void);
s32 dev_wifi_thread_task_is_change(WIFI_TASK_ID task_id);
void dev_wifi_thread_set_result(WIFI_AT_RESULT_E result);
s32 dev_wifi_thread_run_switch(s32 onoff);
s32 dev_wifi_thread_is_free(void);
void dev_wifi_thread(void);

#endif

