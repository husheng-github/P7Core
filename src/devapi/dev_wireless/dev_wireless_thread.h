#ifndef __DEV_WIRELESS_THREAD_H
#define __DEV_WIRELESS_THREAD_H

#define WIRE_SPECIAL_BIT0               0x01       //接收指定数据长度
#define WIRE_SPECIAL_SEND_DATA_LEN      0x02       //发送数据模式
#define WIRE_SPECIAL_READY              0x04       //等待READY标志
#define WIRE_SPECIAL_RCVCMD             0x08       //处理指定命令接收后续数据


typedef enum{
    WIRE_AT_RESULT_MODULE_READY = 0,    //上电返回的at command ready
    WIRE_AT_RESULT_OK,
    WIRE_AT_RESULT_ERROR,
    WIRE_AT_RESULT_TIMEROUT,
    WIRE_AT_RESULT_OTHER,
    WIRE_AT_RESULT_DAIL_NOANSWER,
    //主动上送
    WIRE_AT_RESULT_REPORT_MIPCALL,
    WIRE_AT_RESULT_REPORT_MIPOPEN,
    WIRE_AT_RESULT_REPORT_MIPSTAT,
    WIRE_AT_RESULT_CONTINUE,            //继续发送数据，针对TCP发送数据、设置证书等，在发送命令后，
                                        //需要等待">"后传输后续字符
    
    //---------------------------------
    WIRE_AT_RESULT_NONE = 0x7F,
}WIRE_AT_RESULT_E;

//激活pdp对应的子步骤
typedef enum{
    WIRE_ACTIVE_NETWORK_STEP_GET_IP,            //获取ip
    WIRE_ACTIVE_NETWORK_STEP_GET_APN,           //获取apn
    WIRE_ACTIVE_NETWORK_STEP_SET_APN,           //设置apn
    WIRE_ACTIVE_NETWORK_STEP_CHECK_PDP          //检查网络激活情况
}WIRE_ACTIVE_NETWORK_STEP_E;

//去激活pdp对应的子步骤
typedef enum{
    WIRE_INACTIVE_NETWORK_STEP_GET_IP,             //获取ip
    WIRE_INACTIVE_NETWORK_STEP_CLOSE_PDP,          //关闭数据网络
    WIRE_INACTIVE_NETWORK_STEP_CHECK_RESULT        //关闭数据网络
}WIRE_INACTIVE_NETWORK_STEP_E;


//寻网task对应的子步骤
typedef enum{
    WIRE_SEARCHING_NETWORK_STEP_CHECK_CARD,     //检查sim卡
    WIRE_SEARCHING_NETWORK_CHECK_RPLMN,         //确认RPLMN是否是实网配置
    WIRE_SEARCHING_NETWORK_CLEAR_RPLMN,         //清除实网参数
    WIRE_SEARCHING_NETWORK_STEP_GET_CCID,       //获取ccid
    WIRE_SEARCHING_NETWORK_STEP_CHECK_CSQ,      //检查信号强度
    WIRE_SEARCHING_NETWORK_STEP_SETCEREG,       //sxl20200902设置无信号时上报状态
    WIRE_SEARCHING_NETWORK_CHECK_REG,           //检查注网
    WIRE_SEARCHING_NETWORK_GET_IMSI,            //获取imsi
    WIRE_SEARCHING_NETWORK_SET_REG_PARAM,       //设置creg参数
    WIRE_SEARCHING_NETWORK_SET_4G_REG_PARAM,    //先设置CEREG=2，看是4G卡还是2G卡先注网上
    WIRE_SEARCHING_NETWORK_GET_MAIL_CELL,       //获取主基站信息
    WIRE_SEARCHING_NETWORK_STEP_SET_FMT        //设置数据格式
}WIRE_SEARCHING_NETWORK_STEP_E;

//初始化task对应的子步骤
typedef enum{
    WIRE_INIT_STEP_AT_TEST,
    WIRE_INIT_STEP_ATE,
    WIRE_INIT_GET_MODEL_VER,
    WIRE_INIT_OPEN_FLOW_CTL,          //打开流控
    WIRE_INIT_GET_IMEI,
    WIRE_INIT_CHECK_WORK_MODE,       //确认运行的制式
    WIRE_INIT_SET_WORK_MODE,         //修改运行制式
    WIRE_INIT_GPIO_INIT,             //配置挂在无线模块端设备的gpio属性
    WIRE_INIT_GPIO_SET_VAL,          //配置挂在无线模块端设备的gpio值
    WIRE_INIT_GET_APP_VER,            //获取app的版本
    WIRE_INIT_SET_USE_EXTERANL_AMP,   //设置使用外部功放
    WIRE_INIT_CFG_AUDIO_PLAY_PATH,    //设置播放从speeker通道发出
    WIRE_INIT_CLOSE_AUDIO_PLAY_SLEEP,  //关闭rda在声音播放前的休眠
    WIRE_INIT_CFG_AUDIO_MUT,           //设置麦克风静音，speeker非静音
    WIRE_INIT_CFG_VOL,                  //配置声音大小
    WIRE_INIT_SET_USE_UARTRING,       //配置要不要使用uart ring
    WIRE_INIT_SET_CSCLK,
    WIRE_INIT_SET_GTLPM_MODE           //AT+GTLPMMODE设置
}WIRE_TASK_INIT_STEP_E;

//上电开机task对应的子步骤
typedef enum{
    WIRE_ONBOOT_STEP_BOOT,
    WIRE_ONBOOT_STEP_BOOT1,      //针对T8机型,上电等待一段时间后打开串口
    WIRE_ONBOOT_STEP_POWER
}WIRE_TASK_ONBOOT_STEP_E;

//获取基站信息
typedef enum{
    WIRE_GET_CELL_STEP_MAIN_HEAD,
    WIRE_GET_CELL_STEP_MAIN_INFO,
    WIRE_GET_CELL_STEP_NEAR_HEAD,
    WIRE_GET_CELL_STEP_NEAR_INFO,
}WIRE_GET_CELL_STEP_E;

//开飞行模式子步骤
typedef enum{
    WIRE_OA_STEP_OPEN_AIRPLANE,            //打开飞行模式
    WIRE_OA_STEP_CHECK_RESULT              //检查飞行模式是否打开成功
}WIRE_OPEN_AIRPLANE_STEP_E;

//任务task
typedef enum
{
    WIRE_TASK_FREE = 0,
    WS_TASK_MODULES_ONBOOT = 1,             //模块开机
    WS_TASK_MODULES_INIT = 2,               // 模块初始化
    WS_TASK_SEARCHING_NETWORK = 3,          // 寻网中
    WS_TASK_ACTIVE_NETWORK = 4,             // 激活数据模式
    WS_TASK_INACTIVE_NETWORK,               // 去激活数据模式
    WS_TASK_INACTIVE_NETWORK_APP,           // 去激活数据模式---app层调用
    WS_TASK_POWEROFF_MODULE,                //模块下电--关机时使用
    WS_TASK_RESET_MODULE,                   //模块下电--内部复位时使用
    WS_TASK_GET_CSQ,                        //获取模块信号强度
    WS_TASK_GET_IMEI,                       //获取模块imei
    WS_TASK_GET_IMSI,                       //获取模块imsi
    WS_TASK_GET_CCID,                       //获取模块CCID
    WS_TASK_GET_MODULEVER,                  //获取模块版本
    WS_TASK_GET_LOCAL_IP,                   //获取模块本地ip
    WS_TASK_CREATE_SOCKET,
    WS_TASK_GET_SOCKET_LIST,                //获取可用的socketlist
    WS_TASK_SOCKET_SENDBUFF_LEN,            //发送数据长度   L610可适用
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
    WS_TASK_CHECK_CMD,                    //检查指令是否异常
    WS_TASK_GET_WORK_MODE,           //获取模式
    WS_TASK_SET_WORK_MODE,           //设置仅2G模式
    WS_TASK_GET_HOSTBYNAME,          //域名解析
    
    WS_TASK_ENTER_SLEEP,             //让模块进入低功耗
    WS_TASK_WAKEUP,                  //唤醒模块
    WS_TASK_SET_GPIOOUT,             //GPIO管脚输出
    WS_TASK_PLAYAUDIO ,              //播放音频
    WS_TASK_CTLCAMERA ,              //控制摄像头
    WS_TASK_GET_APPVER,                //获取APP版本
    WS_TASK_SSL_SETMOD,                //设置SSL MOD
    WS_TASK_SSL_SETVER,                //设置SSL 版本
    WS_TASK_SSL_SETCERT,               //设置SSL证书
    WS_TASK_SSL_FILTE,                 //设置SSL过滤协议
    WS_TASK_GET_MODEMAUDOFILELIST,     //获取modem audio文件列表
    WS_TASK_DELETE_AUDIOFILE,          //删除audio file
    WS_TASK_ADD_AUDIOFILE_LEN,         //增加audio file长度
    WS_TASK_ADD_AUDIOFILE_DATA,         //增加audio file数据
    WS_TASK_PLAY_AUDIOFILE,            //播放音频文件
    WS_TASK_GETSET_AUDIOVOLUME,         //获取设置播报音量
    WS_TASK_GETSET_AUDIOPLAYPATH,
    WS_TASK_PLAY_AUDIO_STOP,
    WS_TASK_SET_EXTAMP_CONTROLPIN,      //设置外部功放控制脚
    WS_TASK_SET_FOTA_PARAM,             //设置FOTA参数
    WS_TASK_WRITE_8960INFO_TO_RPLMN,   //将中测仪的信息写入RPLMN，测试8960前调用
    WS_TASK_CLEAR_RPLMN,               //清除RPLMN
    
    WIRE_SLEEP_STEP = 0xff,               //休眠再执行
    
}WIRE_TASK_ID;

//发送socket数据的子step
typedef enum{
    WIRE_SOCKET_SEND_STEP_TO_BUFF,
    WIRE_SOCKET_SEND_STEP_PUSH
}WIRE_SOCKET_SEND_STEP_E;

typedef s32 (*wire_thread_handle_pfunc)(u8 result, u8 *src, u16 srclen);
typedef struct
{
    WIRE_TASK_ID  m_task_id;
    void* m_ap;                         //指针，用来指定参数指针
    u8 m_step;                          //对应任务下的状态
    WIRE_AT_RESULT_E m_result;          //任务执行结果
    u8 m_lparam;                        //跟着m_task和m_step传递的参数
    u8 m_wparam;                        //跟着m_task和m_step传递的参数，(用来记录循环次数)
    u16 m_param1;                        //用来传递过程参数(备用)
    u32 m_param2;                        //用来传递过程参数(备用)
    u32 m_addr_param;                   //传递地址
    wire_thread_handle_pfunc m_func;    //任务结点的执行函数
    volatile u16 m_timeover;            //超时时间(单位: WIRE_TIMER_BASE ms)
    u16 m_timeroutbak;                  //当接收主动上传数据时，冻结当前指令超时时间
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

