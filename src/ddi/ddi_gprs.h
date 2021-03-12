#ifndef __DDI_GPRS_H__
#define __DDI_GPRS_H__


#include "ddi_common.h"

#define DDI_GPRS_CTL_CHECKSIM                          0  //检测SIM是否存在
#define DDI_GPRS_GET_NET_INFO                          1  //获取无线网络信息
#define DDI_GPRS_INSER_HOME_CARD_CFG                   2  //将当前mcc和mnc插入配置文件
#define DDI_GPRS_DEL_HOME_CARD_CFG                     3  //将当前mcc和mnc从配置文件中删除
#define DDI_GPRS_SET_SWITCH_CELL_THRESHOLD             4  //设置切换基站的阈值
#define DDI_GPRS_HAPPENED_SWITCH_CELL                  5  //是否发生基站切换，通信压力测试程序记录使用
#define DDI_GPRS_CTL_CLOSE_PS                          6  //关闭数据模式
#define DDI_GPRS_HAPPENED_CLOSE_PS                     7  //是否发生关闭数据模式，通信压力测试程序记录使用
#define DDI_GPRS_CTL_PING_COMM                         8  //ping服务器
#define DDI_GPRS_CTL_LOCK_CELLS                        9 //锁定可切基站
#define DDI_GPRS_CTL_UNLOCK_CELLS                      10 //解锁可切基站
#define DDI_GPRS_CTL_GET_CELLS                         11 //获取附近的基站信息
#define DDI_GPRS_CTL_SWITCH_CELL                       12 //请求切换基站
#define DDI_GPRS_CTL_GET_SERVER_CELL                   13 //获取当前基站信息
#define DDI_GPRS_CTL_SET_OPTIMIZATION_SWITCH           14 //设置置优化策略开关
#define DDI_GPRS_CTL_REG_NET                           15 //repower sim and register net
#define DDI_GPRS_DISABLE_SWITCH_CELL                   16

#define DDI_GPRS_GET_MODULE_VER                        20//获取固件版本
#define DDI_GPRS_REPOWER                               21
#define DDI_GPRS_CTL_OPEN_AIRPLANE                     22
#define DDI_GPRS_CTL_CLOSE_AIRPLANE                    23
#define DDI_GPRS_CTL_CFG_NTP_SERVER                    24
#define DDI_GPRS_CTL_QRY_NTP                           25
#define DDI_GPRS_CTL_QRY_CGREG                         26   //查询是否注网
#define DDI_GPRS_GET_MODULE_APPVER                     27   //获取模块App版本
#define DDI_GPRS_GET_SSLCERTFIRST                      28   //判断SSL是否证书优先
#define DDI_GPRS_SET_SSLMOD                            29   //设置SSL模式
#define DDI_GPRS_SET_SSLVER                            30   //设置SSL版本

#define DDI_GPRS_GET_MODULE_ID                         31   //获取模块id
#define DDI_GPRS_SUPPORT_OTA                           32   //获取模块是否支持OTA
#define DDI_GPRS_EXC_OTA                               33   //执行ota升级
#define DDI_GPRS_GET_OTA_PROCESS                       34   //获取ota升级过程
#define DDI_GPRS_SET_SSLFILTE                          35   //设置SSL过滤协议

#define DDI_GPRS_CMD_TRANSPARENT                       0xf0

typedef enum _SSL_AUTHEN_MOD
{
    SSL_AUTHEN_MOD_NONE=0,                  //不是用SSL模式
    SSL_AUTHEN_MOD_NOCERT,                  //SSL无证书模式
    SSL_AUTHEN_MOD_TRUST,                   //SSL TRUST认证(单向)
    SSL_AUTHEN_MOD_BIDIRECT,                //SSL 双向认证

}SSL_AUTHEN_MOD_t;


typedef enum{
    GPRS_STATUS_NONE = 0,               //空闲
    GPRS_STATUS_CONNECTING = 1,         //正在连接
    GPRS_STATUS_CONNECTED = 2,          //连接成功
    GPRS_STATUS_DISCONNECTED = 3        //连接断开
}SOCKET_STATUS_E;

typedef enum{
    SOCKET_TYPE_TCP,
    SOCKET_TYPE_UDP
}SOCKET_TYPE;

typedef enum{
    HOST_TYPE_IPADDR,     //ip地址形式
    HOST_TYPE_URL         //网址形式
}HOST_TYPE;

typedef enum
{
    WS_NO_ERR = 0,                     // 无错误信息
    WS_REGISTRATION_DENIED = 1,        // 注册网络被拒
    WS_NO_INSERT_SIM = 2,              // 未插入 SIM 卡
    WS_UNKNOW_SIM = 3,                 // 未知类型 SIM 卡
    WS_NEED_INPUT_SIM_PIN = 4,         // 需要输入 PIN
    WS_NEED_INPUT_SIM_PUK = 5,         // 需要输入 PUK
    WS_NEED_INPUT_SIM_PIN2 = 6,        // 需要输入 PIN2
    WS_NEED_INPUT_SIM_PUK2 = 7,        // 需要输入 PUK2
    WS_UNKNOW_WIRELESS_MODULE = 8,     // 未知无线模块
    WS_UNKNOW_MACHINE_TYPE = 9,        // 未知机型
    WS_NO_WIRELESS_MODULE = 10,        // 缺少无线模块
    WS_NO_CERT_SIM_DATA_ILLEGAL = 11,  // 认证 SIM 数据不合法
    WS_NO_CERT_SIM_MODULE_NOT_SUPPORT= 12, // 无线模块不支持认证 SIM 文件
}WIRE_CONTROL_FAIL_REASON_E;

//网络控制策略步骤
typedef enum
{
    WS_MODULE_POWER_ON = 0,         //上电
    WS_MODULES_INIT = 1,            // 模块初始化
    WS_SEARCHING_NETWORK = 2,       // 寻网中
    WS_ACTIVE_PDP,                  // 激活数据模式
    WS_ACTIVE_PDP_SUCCESS,          // 激活数据模式成功
    WS_INACTIVE_PDP,                // 去激活数据模式
    WS_MODULE_POWER_OFF,            //下电
    WS_CHECK_CMD                    //指令异常，检查指令执行是否ok
}WIRE_CONTROL_STRATEGY_STEP_E;

typedef enum
{
    WS_GET_WIRELESS_STATE = 0,          //获取无线状态类型
    WS_GET_REG_NETWORK_FAIL_REASON     //获取无线注册网络失败原因类型
}wireless_status_type_e;

//GPRS通讯
typedef struct
{
	u8 iccid[32];   //- SIM卡ID号
	u8 imsi[32];    //-SIM卡IMSI
}strSimInfo;
typedef struct 
{
	u8 asLac[5+1];//localization area code or CDMA sid
	u8 asCi[9+1];	//cell identifier  or CDMA nid
	u8 asMcc[3+1];//country code
	u8 asMnc[2+1];//network operator code
	u8 asBsic[5+1];//CDMA bid
	//u8 asBaseStatinInfoGsm[512];//若终端可获取多组则以竖线分割。每组格式为：5位（10进制）LAC + 5位（10进制）cell
	u8 bIsBaseStationOK;
}strBasestationInfo;

typedef struct 
 {
    s8 m_localip[16];   //本地 IP
    s8 m_gateway [16];  //本地网关
    s8 m_mask [16];     //子网掩码
    s8 m_dns1 [16];     //默认 dns
    s8 m_dns2 [16];     //备用 dns
    s8 m_default[32];
 }wireless_net_info_t;

typedef struct
{
    u16       mcc;
    u16       mnc;
    u32       lac;
    u32       cid;
    u8        bsic;
    u16       arfcn;
    s16       rxlev;
}str_cell_info;

//当前使用的无线网络是哪种
typedef enum
{
    WIRE_NET_2G,
    WIRE_NET_3G,
    WIRE_NET_4G
}WIRE_CUR_NET_MODE_E;

typedef enum{
    WIRE_VENDER_MTK             = 0x00,
    WIRE_VENDER_FIBOCOM         = 0x10
}WIRE_MODULE_VENDER_ID_E;

typedef enum{
    FIBOCOM_G500                    = 0x01 | WIRE_VENDER_FIBOCOM,
    FIBOCOM_L610                    = 0x02 | WIRE_VENDER_FIBOCOM
}WIRE_MODULE_ID_E;

typedef enum{
    OTA_DT_HTTP,                        //http方式升级
    OTA_DT_FTP                          //ftp方式升级
}OTA_DOWNLOAD_TYPE_E;

typedef enum{
    OTA_STEP_IDLE,                            //空闲
    OTA_STEP_DOWNLOAD,                        //下载
    OTA_STEP_UPGRADE                          //升级
}OTA_STEP_E;

typedef enum{
    HS_IDLE,                      //空闲
    HS_PROCESSING,                //正在执行
    HS_EXEC_SUCC,                 //执行完成---成功
    HS_EXEC_FAIL                  //执行完成---失败
}HANDLE_STEP_E;

typedef struct{
    HANDLE_STEP_E m_handle_step;
    s32 m_val;                     //step为OTA_STEP_DOWNLOAD，这个值表示下载的百分比
}ota_process_value_t;

typedef struct{
    OTA_DOWNLOAD_TYPE_E m_fota_type;
    s8 m_url[128];
    s8 m_filename[32];     //ftp需要
    s8 m_username[32];     //ftp需要
    s8 m_password[32];     //ftp需要
}module_ota_param_t;

//=====================================================
//对外函数声明
s32 ddi_gprs_open(void);
s32 ddi_gprs_close(void);
s32 ddi_gprs_get_signalquality(u8 *prssi, u16 *prxfull);
s32 ddi_gprs_telephony_dial(u8 *num);
s32 ddi_gprs_telephony_hangup(void);
s32 ddi_gprs_telephony_answer(void);
s32 ddi_gprs_get_pdpstatus(wireless_status_type_e wireless_status_type);
s32 ddi_gprs_set_apn(u8* apn, u8* user, u8* psw);
s32 ddi_gprs_socket_create(u8 type, u8 mode, u8 *param, u16 port);
s32 ddi_gprs_socket_send(s32 socketid, u8 *wbuf, u32 wlen);
s32 ddi_gprs_socket_recv(s32 socketid, u8 *rbuf, u32 rlen);
s32 ddi_gprs_socket_close(s32 socketid);
s32 ddi_gprs_socket_get_status(s32 socketid);
s32 ddi_gprs_get_siminfo(strSimInfo *sim_info);
s32 ddi_gprs_get_imei(u8 *imei_buf, u32 buf_size);
s32 ddi_gprs_get_basestation_info(strBasestationInfo *basestation_info);
s32 ddi_gprs_ioctl(u32 nCmd, u32 lParam, u32 wParam);
//=================================================
typedef s32 (*core_ddi_gprs_open)(void);
typedef s32 (*core_ddi_gprs_close)(void);
typedef s32 (*core_ddi_gprs_get_signalquality)(u8 *prssi, u16 *prxfull);
typedef s32 (*core_ddi_gprs_telephony_dial)(u8 *num);
typedef s32 (*core_ddi_gprs_telephony_hangup)(void);
typedef s32 (*core_ddi_gprs_telephony_answer)(void);
typedef s32 (*core_ddi_gprs_get_pdpstatus)(wireless_status_type_e wireless_status_type);
typedef s32 (*core_ddi_gprs_set_apn)(u8* apn, u8* user, u8* psw);
typedef s32 (*core_ddi_gprs_socket_create)(u8 type, u8 mode, u8 *param, u16 port);
typedef s32 (*core_ddi_gprs_socket_send)(s32 socketid, u8 *wbuf, u32 wlen);
typedef s32 (*core_ddi_gprs_socket_recv)(s32 socketid, u8 *rbuf, u32 rlen);
typedef s32 (*core_ddi_gprs_socket_close)(s32 socketid);
typedef s32 (*core_ddi_gprs_socket_get_status)(s32 socketid);
typedef s32 (*core_ddi_gprs_get_siminfo)(strSimInfo *sim_info);
typedef s32 (*core_ddi_gprs_get_imei)(u8 *imei_buf, u32 buf_size);
typedef s32 (*core_ddi_gprs_get_basestation_info)(strBasestationInfo *basestation_info);
typedef s32 (*core_ddi_gprs_ioctl)(u32 nCmd, u32 lParam, u32 wParam);

#endif /* __DDI_WIFI_H__ */
