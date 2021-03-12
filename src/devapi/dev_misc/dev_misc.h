#ifndef __DEV_MISC_H
#define __DEV_MISC_H



#ifdef DEBUG_MISC_EN
#define MISC_DEBUG(format,...)    dev_debug_printf("%s(%d):"format, __func__, __LINE__, ##__VA_ARGS__)
#define MISC_DEBUGHEX             dev_debug_printformat
#else
#define MISC_DEBUG(...) 
#define MISC_DEBUGHEX(...)
#endif


#define MACHINE_PARAM_FLG0      "Init0"
#define MACHINE_PARAM_FLG1      "Norm1"
#define MACHINE_PARAM_VER       "01"  
#define MACHINE_PARAM_FILENAME  "/mtd2/machineparam"
#define MACHINE_PARAM_FILENAMEBAK  "/mtd2/machineparambak"
#define MACHINE_PARAM_SPACE      4096
//ע��:�Ľṹ��ֻ����β������,�����м����,
//������Ҫ�޸ı�־,�����������ָ�ΪĬ�ϲ���
//�����ṹ�峤�Ȳ��ܳ���MACHINE_PARAM_SPACE
#define MCUAPPINFO_STA      20//(2+2+6+2+8)    
#define MCUAPPINFO_MAX      48
#define ACQUIRERPKDATA_STA  (MCUAPPINFO_STA+MCUAPPINFO_MAX)  
#define ACQUIRERPKDATA_MAX  512
#define MCU_TAMPERMAN_STA   (ACQUIRERPKDATA_STA+ACQUIRERPKDATA_MAX)  // 68
#define MCU_TAMPERMAN_MAX    110    //
#define UPDATEFLG_STA       (MCU_TAMPERMAN_STA+MCU_TAMPERMAN_MAX) //178
#define UPDATEFLG_MAX       10
#define RFPARAMLEN_STA      (UPDATEFLG_STA+UPDATEFLG_MAX)    //188
#define RFPARAMLEN_MAX      32
#define PRINTPARAMLEN_STA   (RFPARAMLEN_STA+RFPARAMLEN_MAX)  // 220
#define PRINTPARAMLEN_MAX   1
#define LCDPARAMLEN_STA     (PRINTPARAMLEN_STA+PRINTPARAMLEN_MAX)  // 221
#define LCDPARAMLEN_MAX     1
#define BTPARAMLEN_STA      (LCDPARAMLEN_STA+LCDPARAMLEN_MAX) // 222
#define BTPARAMLEN_MAX      256
//#define MCU_BPR_STA         (BTPARAMLEN_STA+BTPARAMLEN_MAX)    //478
//#define MCU_BPR_MAX         4       //??
#define MCULANGUAGE_STA     (BTPARAMLEN_STA+BTPARAMLEN_MAX)
#define MCULANGUAGE_MAX     1
#define MCURESET_STA        (MCULANGUAGE_STA+MCULANGUAGE_MAX)
#define MCURESET_MAX        1

#define MCU_HWSNINFO_STA    (MCURESET_STA+MCURESET_MAX)              //482
#define MCU_HWSNINFO_MAX    120//75      //??
#define MCU_TRENDSN_STA     (MCU_HWSNINFO_STA+MCU_HWSNINFO_MAX) // 557
#define MCU_TRENDSN_MAX     109     //??
#define MCU_POSSN_STA       (MCU_TRENDSN_STA+MCU_TRENDSN_MAX)    // 666
#define MCU_POSSN_MAX       68  //+32
#define MCU_TMPRAND_STA     (MCU_POSSN_STA+MCU_POSSN_MAX)           // 730
#define MCU_TMPRAND_MAX     4
#define MCU_RSAKEY_STA      (MCU_TMPRAND_STA+MCU_TMPRAND_MAX)    // 734
#define MCU_RSAKEY_MAX      (256+2)*2
#define MCU_DEVTESTINFO_STA (MCU_RSAKEY_STA+MCU_RSAKEY_MAX)       //1250
#define MCU_DEVTESTINFO_MAX  80
#define MCU_SECUREAUTHENINFO_STA (MCU_DEVTESTINFO_STA+MCU_DEVTESTINFO_MAX) // 1270
#define MCU_SECUREAUTHENINFO_MAX  256

#define MCU_CAK_STA         (MCU_SECUREAUTHENINFO_STA+MCU_SECUREAUTHENINFO_MAX)
#define MCU_CAK_MAX         32

#define MCU_PREVENTINFO_STA (MCU_CAK_STA+MCU_CAK_MAX)
#define MCU_PREVENTINFO_MAX (256+1)


#define MCU_TPINFO_STA      (MCU_PREVENTINFO_STA+MCU_PREVENTINFO_MAX)
#define MCU_TPINFO_MAX      (32)



//#define MCU_MMKINFO_STA     (MCUAPPINFO_STA+MCUAPPINFO_MAX)     
//#define MCU_MMKINFO_MAX     32  //MMK[16] ELRCK[16]

typedef struct _str_machine_param 
{
    //CRC
    u8 m_crc[2];
    u8 m_paramlen[2];
    //������־[6]
    u8 m_paramflg[6];      //��������м����,�޸ĸ�ֵ����ʼ���ǻ�ָ���Ĭ�ϲ�����
    u8 m_paramver[2];      //����汾�ű���β��׷�Ӳ���ʱ����
    //������Ч��־
    u8 m_paramvaild[8];    //ÿһbit��Ӧһ��������,1:��ʾ����������
    //�ļ���Ϣ
    u8 m_appinfo[MCUAPPINFO_MAX];//���Ա���6���ļ�,ÿ���ļ��ĳ���(4B)+CRC32(4B)
    //��������  ??
    u8 m_acquirerpk[ACQUIRERPKDATA_MAX];
    u8 m_tamperman[MCU_TAMPERMAN_MAX];
//    //MMK       ??
//    u8 m_mmk[MCU_MMKINFO_MAX];           
    //���±�־
    u8 m_updateflg[UPDATEFLG_MAX];
    //�ǽӲ���[32B]
    u8 m_rf[RFPARAMLEN_MAX]; 
    //��ӡ����(��ӡ�ٶ�)
    u8 m_print[PRINTPARAMLEN_MAX];          
    //LCD����(�Աȶ�)
    u8 m_lcd[LCDPARAMLEN_MAX];   //lcd�Աȶ�  m_lcdcontrast    
    //��������
    u8 m_bt[BTPARAMLEN_MAX];
//    //����������ֵ  ??
//    u8 m_bpr[MCU_BPR_MAX];    
    //����
    u8 m_language[MCULANGUAGE_MAX];
    //RESET��־
    u8 m_reset[MCURESET_MAX];
    //hwsninfo  Ӳ�����к�  ??
    u8 m_hwsninfo[MCU_HWSNINFO_MAX];
    //trendsn   �����
    u8 m_trendsn[MCU_TRENDSN_MAX];
    //possn     ������ӵ�SN
    u8 m_possn[MCU_POSSN_MAX];
    //tmprand
    u8 m_tmprand[MCU_TMPRAND_MAX];
    //RSAkey
    u8 m_rsakey[MCU_RSAKEY_MAX];
    //������Ϣ   ?? 
    u8 m_devtestinfo[MCU_DEVTESTINFO_MAX];
	u8 m_secureauthinfo[MCU_SECUREAUTHENINFO_MAX];
 //   //CAK        ??
    u8 m_cak[MCU_CAK_MAX];
    //���л���Ϣ ??
    u8 m_preventinfo[MCU_PREVENTINFO_MAX];
    //tp��Ϣ
    u8 m_tpinfo[MCU_TPINFO_MAX];
//    //PCI��־    ?? 
//    u8 m_pciflg[MCU_PCIFLG_MAX];
    //ע��:ֻ�����ӣ������޸�ǰ��Ķ���ͱ�����С

 
    
}str_machine_param_t;
#define STR_MACHINE_PARAM_LEN   sizeof(str_machine_param_t)
typedef enum _MACHINE_PARAM_SN
{
    MACHINE_PARAM_APPINFO   = 0,  //���������ص�core.bin ��app.bin��crc
	ACQUIRERPKDATA           ,
    MACHINE_PARAM_TAMPERMAN  ,
    MACHINE_PARAM_UPDATEFLG  ,
    MACHINE_PARAM_RF         ,
    MACHINE_PARAM_PRINT      ,
    MACHINE_PARAM_LCD        ,
    MACHINE_PARAM_BT         ,
//    MACHINE_PARAM_BPR        ,
    MACHINE_PARAM_LANGUAGE   ,
    MACHINE_PARAM_RESET      ,
    MACHINE_PARAM_HWSNINFO   ,
    MACHINE_PARAM_TRENDSN    ,
    MACHINE_PARAM_POSSN      ,
    MACHINE_PARAM_TMPRAND    ,
    MACHINE_PARAM_RSAKEY     ,
    MACHINE_PARAM_DEVTESTINFO ,
//    MACHINE_PARAM_CAK           ,
//    MACHINE_PARAM_PREVENTINFO ,
//    MACHINE_PARAM_PCIFLG      ,
    MACHINE_PARAM_SECUREAUTHENINFO,
    MCU_CAK,
    MCU_PREVENTINFO,
    MACHINE_PARAM_TP,
    MACHINE_PARAM_SN_MAX ,
}machine_param_sn_t; 

typedef struct _strMACHINE_PARAM
{
    u16 m_sn;           //���
    u16 m_offset;       //��Ӧ�ṹƫ��
    u16 m_spacemax;     //��󳤶�
}strMachineparam_t;


typedef enum _MACHINE_HWVER
{
    MACHINE_HWVER_M3V1  = 0x00,
    MACHINE_HWVER_M3V2  = 0x01,
    MACHINE_HWVER_M3V3  = 0x02,
    MACHINE_HWVER_M3V4  = 0x03,
    MACHINE_HWVER_M3V5  = 0x04,
    MACHINE_HWVER_M3V6  = 0x05,
    
    MACHINE_HWVER_M5V1  = 0x10,
    MACHINE_HWVER_M5V2  = 0x11,
    MACHINE_HWVER_M5V3  = 0x12,
    MACHINE_HWVER_M5V4  = 0x13,
    MACHINE_HWVER_M5V5  = 0x14,
    MACHINE_HWVER_M5V6  = 0x15,
    
}MACHINE_HWVER_t;



typedef struct __HWSN_INFO
{
//    u8 m_flg[8];
    u8 m_vendorid[6];
    u8 m_termialno[2];
    u8 m_sn[42];
    u8 m_snlen;
    u8 m_snkey[32];     //��Կ
    u8 m_snkeylen;
    u8 m_cfg[32];       //����
    u8 m_cfglen;
//    u8 m_crc[2];
}hwsn_info_t;
#define HWSN_INFO_LEN   sizeof(hwsn_info_t)

typedef struct{
    u8 flag[2];
    u8 led;  //0 δ���ԣ�1���Գɹ���2 ����ʧ�ܡ�3��֧�֡�
    u8 lcd;
    u8 beeper;
    u8 key;
    u8 magcard;    
    u8 iccard;
    u8 rfcard;
    u8 spiflash;
    u8 bt;
    u8 wifi;
    u8 printer;
    u8 gprs;
    u8 rtc;
    u8 camera;
    u8 tp;
    u8 sleep;
    u8 audio;
    u8 cashbox;
    u8 sencond_test;
    u8 audiofilesyncresult;
    u8 reserved[56-8-22];
    u8 pcbatestflag[8];  //�ܹ�֧��56���ֽڣ����ܳ���56���ֽ�
}TRENDITDEV_TEST_INFO;

typedef enum{
    ST_REBOOT = 1,         //reboot��λ
    ST_WATCHDOG = 2,       //���Ź���λ
    ST_HARD_FAULT = 3          //hard fault��λ
}SYSTEM_RESET_TYPE_E;

//������������
void dev_sys_NVIC_ClearAllEXTIPanding(void);
void dev_systick_ctl(u8 flg);

s32 dev_misc_machineparam_init(void);
s32 dev_misc_machineparam_get(machine_param_sn_t sn, u8 *param, u16 paramlen, u16 offset);
s32 dev_misc_machineparam_set(machine_param_sn_t sn, u8 *param, u16 paramlen, u16 offset);
s32 dev_misc_machineparam_del(machine_param_sn_t sn, u8 *param, u16 paramlen);
s32 dev_misc_updateflg_set(u8 flg);
s32 dev_misc_updateflg_check(void);
void dev_misc_init(void);
void dev_misc_cashbox_open(void);
void dev_misc_poweroff(void);
void dev_misc_reboot(SYSTEM_RESET_TYPE_E st_type);
s32 dev_misc_bt_check(void);
s32 dev_misc_get_workmod(void);

#endif

