

#ifndef _DDI_MISC_H_
#define _DDI_MISC_H_

//#include "ddi.h"
//#include "pciglobal.h"

#include "ddi_common.h"

#define RTC_ENABLE
//-------------------------
//�����
typedef enum _SNType
{
     SNTYPE_POSSN,
     SNTYPE_APCUPID,
     SNTYPE_SPCUPID,
}SNType;
//------------------------------
//�ײ�̼��汾
typedef enum _FIRMWARETYPE
{
    FIRMWARETYPE_HW = 0,
    FIRMWARETYPE_APBOOT,
    FIRMWARETYPE_APCORE,
    FIRMWARETYPE_APVIVA,
    FIRMWARETYPE_APMANAGE,
    FIRMWARETYPE_APAPP,
    FIRMWARETYPE_SPBOOT,
    FIRMWARETYPE_SPCORE,
    FIRMWARETYPE_SPAPP,
    FIRMWARETYPE_ROMTYPE,
}FIRMWARETYPE;
//-----------------------------------------
//���
#define     DDI_POWER_NULL               0//��ص������㣬�ػ�
#define     DDI_POWER_ZERO               1//���0��
#define     DDI_POWER_ONE                2//���1��
#define     DDI_POWER_TWO                3//���2��
#define     DDI_POWER_THREE              4//���3��
#define     DDI_POWER_FOUR               5//���4��
#define     DDI_POWER_FIVE               6//���5��

#define     DDI_POWER_NO_AD              0 //�����
#define     DDI_POWER_AD_CHARGE          1//����磬�����
#define     DDI_POWER_AD_NO_CHARGE       2//����磬δ���
#define     DDI_POWER_AD_CHARGE_FULL     3//����磬�����
#define     DDI_POWER_AD_BATTERY_ERR     4//����磬��ع���

typedef enum
{
   //DDI_BATTERY_LOW_POWEROFF = 0,
   //DDI_BATTERY_LOW_TX_PROHIBIT,
   DDI_BATTERY_LOW_WARNING = 0,
   DDI_BATTERY_LEVEL_0,
   DDI_BATTERY_LEVEL_1,
   DDI_BATTERY_LEVEL_2,
   DDI_BATTERY_LEVEL_3, 
   DDI_BATTERY_LEVEL_4, /* BATTERY_LEVEL_4 */
   DDI_BATTERY_LEVEL_5, /* BATTERY_LEVEL_5 */
   DDI_BATTERY_LEVEL_6 = DDI_BATTERY_LEVEL_5, /* BATTERY_LEVEL_6 */
   DDI_BATTERY_LEVEL_LAST = 9,
}DDI_battery_level_enum;

typedef struct __POWER_STATUS
{
    u8 m_aclinestate;       //USB���״̬,  0:USB�Ѱγ� 1:USB���ڳ��
    u8 m_batteryflg;        //��صĴӵ�״̬,0:δ���(�����Ҳ�����״̬) 
                            //               4:�����
    DDI_battery_level_enum m_batterylevel;
    u8 m_batterylifepercent;    //��ص����ٷֱ�
    u8 m_batteryisexist;    //�����λ�ж�, 0: �����ڣ� 1: ����
    u32 m_batteryvol;       //��ص�ѹ����λ:mV
    u32 m_batterytemper;    //����¶ȣ���λΪ0.01C,��ȡ�����¶�Ϊ0
}power_status_t; 

#if 0
struct _strBatSta
{
    u16 vol;//��ѹֵ��mv
    u8 per;//���� 0-100;
    u8 exp;//�Ƿ�����磬1���У�0 ��
    u8 chg;//�Ƿ��ڳ�磬
    u8 stag;//��������
    u16 bottonvol;//Ŧ�۵�ص�ѹ����Ҫ�򿪼�⣬���ݲ�����Ч��
/*    
#define PWR_BAT_CHG_NO      0//δ���
#define PWR_BAT_CHG_ING     1//���ڳ��(���)
#define PWR_BAT_CHG_SLOW    2//����(�������ٳ��Сʱ)
#define PWR_BAT_CHG_OK      3//������   
*/    
};
#endif

#if 0
typedef struct _PCI_CORE_VERINFO
{
	u8 ver[20];
}PCI_CORE_VERINFO;
#endif


//SP security status
#define SPSTATUSMACHINE_BOOTSTART     0
#define SPSTATUSMACHINE_BOOTDOWNLOAD  1
#define SPSTATUSMACHINE_CORESTART     2
#define SPSTATUSMACHINE_APPRUN        3



typedef struct{
    
	u8 spstatusmachine;
	
	u8 spsecurestatus_secureenableflag:1;  //bit 0
	u8 spsecurestatus_tamperstatus:1;      //bit 1
	u8 spsecurestatus_bootstatus:1;        //bit 2
	u8 spsecurestatus_corestatus:1;        //bit 3
	u8 spsecurestatus_appstatus:1;         //bit 4
	u8 spsecurestatus_seucerkeystatus:1;   //bit 5
	u8 spsecurestatus_lock:1;              //bit6
	u8 spsecurestatus_resettype:1;         //bit7
    
	u8 spsecurestatus_trenditpkstatus:1;   //bit 0
	u8 spsecurestatus_acquirerpkstatus:1;  //bit 1
	u8 spsecurestatus_fanqieenableflag:1;  //bit 2
	u8 spsecurestatus_fanqiestatus:1;      //bit 3
	u8 spsecurestatus_fanqiejiexist:1;    //bit 4
	u8 spsecurestatus_authenskexist:1;    //bit 5
	u8 spsecurestatus_updateflag:1;       //bit 6
	u8 spsecurestatus_reserved:1;         // bit 7 
	u8 reserved[5];                      // reserved for future use

	u8 tamperdetectedreg[4];            //����ʱ�����Ĵ���ֵ
	u8 currenttamperreg[4];           //��ǰ�����Ĵ���ֵ
	u8 tampercount[4];
	u8 unlockmode; 
    
	
}SPSTATUS;

//--------------------------------------------
typedef struct _strMCUPARAM_IOCTL
{
    u8* m_param;
    u16 m_sn;
    u16 m_paramlen;
    u16 m_offset;
    u16 m_ruf;
}strMCUparam_ioctl_t;

typedef struct _mem_info{
    u32 m_remaining_memory;   //
    u32 m_remaining_block;    
    s8 reserve[12];
}mem_info_t;

typedef enum _strLANGUAGE_PARAM
{
    LANGUAGE_SIMPLIFIED_CHINESE=0,    //��������
    LANGUAGE_ENGLISH             ,    //Ӣ��
    LANGUAGE_TYPE_MAX            ,
}strLanguageparam_t;


//#define MISC_IOCTL_MCUCALIBRATE     0       //MCU�ڲ���У׼
#define MISC_IOCTL_SET_UPDATEFLG    1       //����UPDATE��־
#define MISC_IOCTL_CLR_UPDATEFLG    2       //���UPDATE��־
#define MISC_IOCTL_GET_WAKEUPPIN    3       //��ȡWAKEUP�ܽ�״̬
//#define MISC_IOCTL_SETHWSNANDSNKEY  4       //����Ӳ�����кż����к���Կ
#define MISC_IOCTL_SETMCUPARAM      4       //���ò���
#define MISC_IOCTL_GETMCUPARAM      5       //��ȡ����
#define MISC_IOCTL_MCUPARAMINITIAL  6       //������ʼ��
#define MISC_IOCTL_GETHWVER         7       //��ȡӲ���汾��
#define MISC_IOCTL_GETGPRSMODULEVERSION 8    //��ȡGPRSģ��汾��
#define MISC_IOCTL_GETTMSMACHINETYPEID 9   //��ȡTMS����������������
#define MISC_IOCTL_GET_DEBUGFLAG      11       //��ȡDEBUG��־
#define MISC_IOCTL_SPCANCEL        12     //ȡ��sp����ָ��
#define MISC_IOCTL_SETCHARGESTATUS 13     //���ó��״̬��wParamΪ1��ʼ��� 0ֹͣ���

#define MISC_IOCTL_OPENCASHBOX     14     //��Ǯ��

#define MISC_IOCTL_GET_SYSTEM_MEM       15     //��ȡϵͳʣ���ڴ�
#define MISC_IOCTL_GET_LANGUAGETYPE     16     //��ȡ����ģʽ
#define MISC_IOCTL_SET_LANGUAGETYPE     17     //��������ģʽ
#define MISC_IOCTL_GET_RESET_FLAG       18     //��ȡ��λ��־
#define MISC_IOCTL_CLEAR_RESET_FLAG     19     //�����λ��־
#define MISC_IOCTL_CDC_OUTPUT_SWITCH     0XFF    //�ر�cdc�������������������ʱ��



#define MISC_IOCTL_READHWSN         0x80       //��SN
#define MISC_IOCTL_CLEARHWSN        0x81      //��Ӳ�����к�
#define MISC_IOCTL_SETHWSNANDSNKEY  0x82      //����Ӳ�����кż����к���Կ
#define MISC_IOCTL_GETHWSNANDSNKEY  0x83      //��ȡӲ�����кż����к���Կ


typedef enum _SysDevProbe
{
	DEV_IC_CARD = 0,
	DEV_MAG_CARD,
	DEV_RF_CARD,
	DEV_TOUCH_SCREEN,
	DEV_BLUETOOTH,
	DEV_WIRELESS_MODULE,
	DEV_MODEM,
	DEV_WIFI,
	DEV_ETHERNET,
	DEV_GPS,
	DEV_SD_CARD,
	DEV_BARCODE_GUN,
	DEV_FINGERPRINT_ID,
	DEV_CAMERA,
	DEV_PRINT,
    DEV_BEEP,                   //������
    DEV_AUDIO,                  //����
	DEV_SPMCU,      //��CPU�ж�
}SysDevProbe;

//=====================================================
//���⺯������
void ddi_misc_appprinf(const char *fmt,...);
s32 ddi_misc_msleep(u32 nMs);
s32 ddi_misc_set_time(const u8 *lpTime);
s32 ddi_misc_get_time(u8 *lpTime);
s32 ddi_misc_get_tick(u32 *nTick);  //ϵͳ�δ�
s32 ddi_misc_read_dsn(SNType sntype, u8 *lpOut);
s32 ddi_misc_sleep(s32 status);
s32 ddi_misc_get_firmwareversion(FIRMWARETYPE firmwaretype, u8 *lpOut);
s32 ddi_misc_bat_status(power_status_t *lp_power_status );
s32 ddi_misc_poweroff(void);
void *k_mallocapp(unsigned nbytes);
void k_freeapp(void *ap);
s32 ddi_misc_sleep_with_alarm(s32 sec, s32 type, s32 status);
s32 ddi_misc_reboot(void);
s32 ddi_misc_probe_dev(u32 nDev);
s32 ddi_user_timer_open(u32 *timer);
s32 ddi_user_timer_close(u32 *timer);
u32 ddi_pci_readtamperstatus(u8 *unlocktimes);
u32 ddi_pci_getcurrenttamperstatus(void);
u32 ddi_pci_unlock(void);
u32 ddi_pci_opendryice(u8 dryicetype);
u32 ddi_pci_getdryiceconfig(void);
SPSTATUS* ddi_pci_getspstatuspointer(void);
s32 ddi_misc_ioctl(u32 nCmd, u32 lParam, u32 wParam);


//=======================================================
typedef void (*core_ddi_misc_appprinf)(const char *fmt,...);
typedef s32 (*core_ddi_misc_msleep)(u32 nMs);
typedef s32 (*core_ddi_misc_set_time)(const u8 *lpTime);
typedef s32 (*core_ddi_misc_get_time)(u8 *lpTime);
typedef s32 (*core_ddi_misc_get_tick)(u32 *nTick);  //ϵͳ�δ�
typedef s32 (*core_ddi_misc_read_dsn)(SNType sntype, u8 *lpOut);
typedef s32 (*core_ddi_misc_sleep) (s32 status);
typedef s32 (*core_ddi_misc_get_firmwareversion)(FIRMWARETYPE firmwaretype, u8 *lpOut);
typedef s32 (*core_ddi_misc_bat_status) (power_status_t *lp_power_status );
typedef s32 (*core_ddi_misc_poweroff)(void);
typedef void *(*core_k_mallocapp)(unsigned nbytes);
typedef void (*core_k_freeapp)(void *ap);
typedef s32 (*core_ddi_misc_sleep_with_alarm)(s32 sec, s32 type, s32 status);
typedef s32 (*core_ddi_misc_reboot)(void);
typedef s32 (*core_ddi_misc_probe_dev)(u32 nDev);
typedef s32 (*core_ddi_user_timer_open)(u32 *timer);
typedef s32 (*core_ddi_user_timer_close)(u32 *timer);
typedef u32  (*core_ddi_pci_readtamperstatus)(u8 *unlocktimes);
typedef u32  (*core_ddi_pci_getcurrenttamperstatus)(void);
typedef u32  (*core_ddi_pci_unlock)(void);
typedef u32  (*core_ddi_pci_opendryice)(u8 dryicetype);
typedef u32  (*core_ddi_pci_getdryiceconfig)(void);
typedef SPSTATUS* (*core_ddi_pci_getspstatuspointer)(void);
typedef s32 (*core_ddi_misc_ioctl)(u32 nCmd, u32 lParam, u32 wParam);

#endif


