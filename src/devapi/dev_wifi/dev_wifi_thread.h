#ifndef __DEV_WIFI_THREAD_H
#define __DEV_WIFI_THREAD_H

#define WIFI_SPECIAL_BIT0            0x01       //接收指定数据长度
#define WIFI_SPECIAL_SEND_DATA_LEN   0x02       //发送数据模式
#define WIFI_SPECIAL_READY     0x04       //等待READY标志
#define WIFI_SPECIAL_RCVCMD    0x08       //处理指定命令接收后续数据

typedef enum{
    WIFI_AT_RESULT_MODULE_READY = 0,    //上电返回的at command ready
    WIFI_AT_RESULT_OK,
    WIFI_AT_RESULT_ERROR,
    WIFI_AT_RESULT_TIMEROUT,
    WIFI_AT_RESULT_OTHER,
    WIFI_AT_RESULT_DAIL_NOANSWER,
    //主动上送
    WIFI_AT_RESULT_REPORT_MIPCALL,
    WIFI_AT_RESULT_REPORT_MIPOPEN,
    WIFI_AT_RESULT_REPORT_MIPSTAT,
    WIFI_AT_RESULT_CONTINUE,            //继续发送数据，针对TCP发送数据、设置证书等，在发送命令后，
                                        //需要等待">"后传输后续字符
    
    //---------------------------------
    WIFI_AT_RESULT_DNSFAIL,             //域名解析失败
    WIFI_AT_RESULT_SOCKET_ALREAYDY_EXIT,  //套接字存在
    WIFI_AT_RESULT_NONE = 0x7F,
}WIFI_AT_RESULT_E;

//寻网task对应的子步骤
typedef enum{
    WIFI_AP_CONNECTTING_STEP1,     
}WIFI_AP_CONNECTTING_E;

//初始化task对应的子步骤
typedef enum{
    WIFI_INIT_STEP_AT_TEST,
    WIFI_INIT_STEP_ATE,
    WIFI_INIT_SET_HARDFLOW,           //设置流控
    WIFI_INIT_GET_MODEL_VER,          //获取固件版本
    WIFI_INIT_SET_CUR_MODE,           //设置当前的工作模式
    WIFI_INIT_SET_TRANS_MODE,         //设置传输模式
    WIFI_INIT_SET_AUTO_CONN,          //设置上电时是否自动连接热点
    WIFI_INIT_SET_MUX,                //设置连接模式
    WIFI_INIT_SET_SCAN_RES,           //设置扫描热点的返回格式
    WIFI_INIT_SET_WAKEUP_GPIO,        //设置唤醒模式为高电平唤醒light sleep
    WIFI_INIT_DISCONNECT_AP,           //固件默认启动时会连trendit热点
    WIFI_INIT_SET_SLEEP_MODE,          //设置休眠模式
    WIFI_INIT_SET_MAX_CONN,            //设置最大连接    ---web配网才做这之后的流程
    WIFI_INIT_OPEN_TCP_SERVER,         //创建tcp server
    WIFI_INIT_STET_SOFTAP_NAME,        //设置发射的ap名字
    WIFI_INIT_GET_SOFTAP_NAME          //查询ap名字
}WIFI_TASK_INIT_STEP_E;

//上电开机task对应的子步骤
typedef enum{
    WIFI_ONBOOT_STEP_BOOT,
    WIFI_ONBOOT_STEP_POWER
}WIFI_TASK_ONBOOT_STEP_E;

//任务task
typedef enum
{
    WIFI_TASK_FREE = 0,
    WIFI_TASK_MODULES_ONBOOT = 1,             //模块开机
    WIFI_TASK_MODULES_INIT = 2,               // 模块初始化
    WIFI_TASK_AP_CONNETING = 3,               // 连接热点
    WIFI_TASK_AP_DISCONNECT,                  // 断开热点
    WIFI_TASK_POWEROFF_MODULE,                //模块下电--关机时使用
    WIFI_TASK_RESET_MODULE,                   //模块下电--内部复位时使用
    WIFI_TASK_CREATE_SOCKET,
    WIFI_TASK_GET_SOCKET_LIST,                //获取可用的socketlist
    WIFI_TASK_SOCKET_SEND_DATALEN,
    WIFI_TASK_SOCKET_SENDBUFF,
    WIFI_TASK_SOCKET_RECVDATA,
    WIFI_TASK_SOCKET_CLOSE,
    WIFI_TASK_CFG_NTP_SERVER,
    WIFI_TASK_QRY_RTC,
    WIFI_TASK_CHECK_CMD,                    //检查指令是否异常
    WIFI_TASK_GET_CUR_AP_INFO,              //获取当前连接ap的信息
    WIFI_TASK_SCAN_AP,                      //扫描热点
    
    WIFI_TASK_ENTER_SLEEP,             //让模块进入低功耗
    WIFI_TASK_WAKEUP,                  //唤醒模块

    WIFI_TASK_GET_NETINFO,              //获取网络信息

    WIFI_TASK_STOP_AIRKISS,                  //关闭airkiss
    WIFI_TASK_GET_NET_STATE,                 //获取网络连接状态

    WIFI_TASK_CLEAR_RF_PARAM,
    
    WIFI_SLEEP_STEP = 0xff,               //休眠再执行
}WIFI_TASK_ID;

//发送socket数据的子step
typedef enum{
    WIFI_SOCKET_SEND_STEP_TO_BUFF,
    WIFI_SOCKET_SEND_STEP_PUSH
}WIFI_SOCKET_SEND_STEP_E;

typedef s32 (*wifi_thread_handle_pfunc)(u8 result, u8 *src, u16 srclen);
typedef struct
{
    WIFI_TASK_ID  m_task_id;
    void* m_ap;                         //指针，用来指定参数指针
    u8 m_step;                          //对应任务下的状态
    WIFI_AT_RESULT_E m_result;          //任务执行结果
    u8 m_lparam;                        //跟着m_task和m_step传递的参数
    u8 m_wparam;                        //跟着m_task和m_step传递的参数，(用来记录循环次数)
    u16 m_param1;                        //用来传递过程参数(备用)
    u16 m_param2;                        //用来传递过程参数(备用)
    u32 m_addr_param;                    //用来传递过程参数(备用)
    wifi_thread_handle_pfunc m_func;    //任务结点的执行函数
    volatile u16 m_timeover;            //超时时间(单位: WIFI_TIMER_BASE ms)
    u16 m_timeroutbak;                  //当接收主动上传数据时，冻结当前指令超时时间
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

