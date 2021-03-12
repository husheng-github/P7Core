#ifndef __DDI_GPRS_H__
#define __DDI_GPRS_H__


#include "ddi_common.h"

#define DDI_GPRS_CTL_CHECKSIM                          0  //���SIM�Ƿ����
#define DDI_GPRS_GET_NET_INFO                          1  //��ȡ����������Ϣ
#define DDI_GPRS_INSER_HOME_CARD_CFG                   2  //����ǰmcc��mnc���������ļ�
#define DDI_GPRS_DEL_HOME_CARD_CFG                     3  //����ǰmcc��mnc�������ļ���ɾ��
#define DDI_GPRS_SET_SWITCH_CELL_THRESHOLD             4  //�����л���վ����ֵ
#define DDI_GPRS_HAPPENED_SWITCH_CELL                  5  //�Ƿ�����վ�л���ͨ��ѹ�����Գ����¼ʹ��
#define DDI_GPRS_CTL_CLOSE_PS                          6  //�ر�����ģʽ
#define DDI_GPRS_HAPPENED_CLOSE_PS                     7  //�Ƿ����ر�����ģʽ��ͨ��ѹ�����Գ����¼ʹ��
#define DDI_GPRS_CTL_PING_COMM                         8  //ping������
#define DDI_GPRS_CTL_LOCK_CELLS                        9 //�������л�վ
#define DDI_GPRS_CTL_UNLOCK_CELLS                      10 //�������л�վ
#define DDI_GPRS_CTL_GET_CELLS                         11 //��ȡ�����Ļ�վ��Ϣ
#define DDI_GPRS_CTL_SWITCH_CELL                       12 //�����л���վ
#define DDI_GPRS_CTL_GET_SERVER_CELL                   13 //��ȡ��ǰ��վ��Ϣ
#define DDI_GPRS_CTL_SET_OPTIMIZATION_SWITCH           14 //�������Ż����Կ���
#define DDI_GPRS_CTL_REG_NET                           15 //repower sim and register net
#define DDI_GPRS_DISABLE_SWITCH_CELL                   16

#define DDI_GPRS_GET_MODULE_VER                        20//��ȡ�̼��汾
#define DDI_GPRS_REPOWER                               21
#define DDI_GPRS_CTL_OPEN_AIRPLANE                     22
#define DDI_GPRS_CTL_CLOSE_AIRPLANE                    23
#define DDI_GPRS_CTL_CFG_NTP_SERVER                    24
#define DDI_GPRS_CTL_QRY_NTP                           25
#define DDI_GPRS_CTL_QRY_CGREG                         26   //��ѯ�Ƿ�ע��
#define DDI_GPRS_GET_MODULE_APPVER                     27   //��ȡģ��App�汾
#define DDI_GPRS_GET_SSLCERTFIRST                      28   //�ж�SSL�Ƿ�֤������
#define DDI_GPRS_SET_SSLMOD                            29   //����SSLģʽ
#define DDI_GPRS_SET_SSLVER                            30   //����SSL�汾

#define DDI_GPRS_GET_MODULE_ID                         31   //��ȡģ��id
#define DDI_GPRS_SUPPORT_OTA                           32   //��ȡģ���Ƿ�֧��OTA
#define DDI_GPRS_EXC_OTA                               33   //ִ��ota����
#define DDI_GPRS_GET_OTA_PROCESS                       34   //��ȡota��������
#define DDI_GPRS_SET_SSLFILTE                          35   //����SSL����Э��

#define DDI_GPRS_CMD_TRANSPARENT                       0xf0

typedef enum _SSL_AUTHEN_MOD
{
    SSL_AUTHEN_MOD_NONE=0,                  //������SSLģʽ
    SSL_AUTHEN_MOD_NOCERT,                  //SSL��֤��ģʽ
    SSL_AUTHEN_MOD_TRUST,                   //SSL TRUST��֤(����)
    SSL_AUTHEN_MOD_BIDIRECT,                //SSL ˫����֤

}SSL_AUTHEN_MOD_t;


typedef enum{
    GPRS_STATUS_NONE = 0,               //����
    GPRS_STATUS_CONNECTING = 1,         //��������
    GPRS_STATUS_CONNECTED = 2,          //���ӳɹ�
    GPRS_STATUS_DISCONNECTED = 3        //���ӶϿ�
}SOCKET_STATUS_E;

typedef enum{
    SOCKET_TYPE_TCP,
    SOCKET_TYPE_UDP
}SOCKET_TYPE;

typedef enum{
    HOST_TYPE_IPADDR,     //ip��ַ��ʽ
    HOST_TYPE_URL         //��ַ��ʽ
}HOST_TYPE;

typedef enum
{
    WS_NO_ERR = 0,                     // �޴�����Ϣ
    WS_REGISTRATION_DENIED = 1,        // ע�����类��
    WS_NO_INSERT_SIM = 2,              // δ���� SIM ��
    WS_UNKNOW_SIM = 3,                 // δ֪���� SIM ��
    WS_NEED_INPUT_SIM_PIN = 4,         // ��Ҫ���� PIN
    WS_NEED_INPUT_SIM_PUK = 5,         // ��Ҫ���� PUK
    WS_NEED_INPUT_SIM_PIN2 = 6,        // ��Ҫ���� PIN2
    WS_NEED_INPUT_SIM_PUK2 = 7,        // ��Ҫ���� PUK2
    WS_UNKNOW_WIRELESS_MODULE = 8,     // δ֪����ģ��
    WS_UNKNOW_MACHINE_TYPE = 9,        // δ֪����
    WS_NO_WIRELESS_MODULE = 10,        // ȱ������ģ��
    WS_NO_CERT_SIM_DATA_ILLEGAL = 11,  // ��֤ SIM ���ݲ��Ϸ�
    WS_NO_CERT_SIM_MODULE_NOT_SUPPORT= 12, // ����ģ�鲻֧����֤ SIM �ļ�
}WIRE_CONTROL_FAIL_REASON_E;

//������Ʋ��Բ���
typedef enum
{
    WS_MODULE_POWER_ON = 0,         //�ϵ�
    WS_MODULES_INIT = 1,            // ģ���ʼ��
    WS_SEARCHING_NETWORK = 2,       // Ѱ����
    WS_ACTIVE_PDP,                  // ��������ģʽ
    WS_ACTIVE_PDP_SUCCESS,          // ��������ģʽ�ɹ�
    WS_INACTIVE_PDP,                // ȥ��������ģʽ
    WS_MODULE_POWER_OFF,            //�µ�
    WS_CHECK_CMD                    //ָ���쳣�����ָ��ִ���Ƿ�ok
}WIRE_CONTROL_STRATEGY_STEP_E;

typedef enum
{
    WS_GET_WIRELESS_STATE = 0,          //��ȡ����״̬����
    WS_GET_REG_NETWORK_FAIL_REASON     //��ȡ����ע������ʧ��ԭ������
}wireless_status_type_e;

//GPRSͨѶ
typedef struct
{
	u8 iccid[32];   //- SIM��ID��
	u8 imsi[32];    //-SIM��IMSI
}strSimInfo;
typedef struct 
{
	u8 asLac[5+1];//localization area code or CDMA sid
	u8 asCi[9+1];	//cell identifier  or CDMA nid
	u8 asMcc[3+1];//country code
	u8 asMnc[2+1];//network operator code
	u8 asBsic[5+1];//CDMA bid
	//u8 asBaseStatinInfoGsm[512];//���ն˿ɻ�ȡ�����������߷ָÿ���ʽΪ��5λ��10���ƣ�LAC + 5λ��10���ƣ�cell
	u8 bIsBaseStationOK;
}strBasestationInfo;

typedef struct 
 {
    s8 m_localip[16];   //���� IP
    s8 m_gateway [16];  //��������
    s8 m_mask [16];     //��������
    s8 m_dns1 [16];     //Ĭ�� dns
    s8 m_dns2 [16];     //���� dns
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

//��ǰʹ�õ���������������
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
    OTA_DT_HTTP,                        //http��ʽ����
    OTA_DT_FTP                          //ftp��ʽ����
}OTA_DOWNLOAD_TYPE_E;

typedef enum{
    OTA_STEP_IDLE,                            //����
    OTA_STEP_DOWNLOAD,                        //����
    OTA_STEP_UPGRADE                          //����
}OTA_STEP_E;

typedef enum{
    HS_IDLE,                      //����
    HS_PROCESSING,                //����ִ��
    HS_EXEC_SUCC,                 //ִ�����---�ɹ�
    HS_EXEC_FAIL                  //ִ�����---ʧ��
}HANDLE_STEP_E;

typedef struct{
    HANDLE_STEP_E m_handle_step;
    s32 m_val;                     //stepΪOTA_STEP_DOWNLOAD�����ֵ��ʾ���صİٷֱ�
}ota_process_value_t;

typedef struct{
    OTA_DOWNLOAD_TYPE_E m_fota_type;
    s8 m_url[128];
    s8 m_filename[32];     //ftp��Ҫ
    s8 m_username[32];     //ftp��Ҫ
    s8 m_password[32];     //ftp��Ҫ
}module_ota_param_t;

//=====================================================
//���⺯������
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
