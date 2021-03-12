#ifndef DEV_RF_FLOW_H
#define DEV_RF_FLOW_H

//#include "g101drs_spiflash_allocation.h"
//#define PCD_DEBUG   

#define PICC_TYPEA                  'A'//0x00
#define PICC_TYPEB                  'B'//0x08
#define PICC_TYPEC                  'C'//0x0C
#define PICC_MIFARE                 'M'
#define PICC_FELICA                 'F'
#define PICC_TYPE24                 'D'//0x0D
#define PICC_UNKONWN                'N'

//#define EMV_TEST_FLOW       1//如果用于EMV测试，请定义为1，出货程序定义为0
extern u8 g_rf_lp_step;
extern u8 g_rf_lp_flg;

//#define MODULATION_DEPTH_ADDR 0x52fff
//extern u8 modulation_depth;
#define DEV_RF_TXCRC_MASK     0x01
#define DEV_RF_TXCRC_EN       0x01
#define DEV_RF_TXCRC_DIS      0x00
#define DEV_RF_RXCRC_MASK     0x02
#define DEV_RF_RXCRC_EN       0x02
#define DEV_RF_RXCRC_DIS      0x00


#define POLLING_INTERVAL    80
#define BACKLIGHT_FLASH_INTERVAL    400


/*! ISO14434-A anticollision response cascade tag value. */
#define EMV_CASCADE_TAG         0x88

/* \ingroup mifare
 *  \name Commands
 *  Command definitions for Mifare operation. These command bytes comply to the MIFARE
 *  specification and serve as one parameter for the MIFARE transaction commands defined
 *  within the scope of this implementation.
 */

typedef struct
{
    u16 nBytesToSend;        //!< how many bytes to send
    u16 nBytesReceived;      //!< how many bytes received
    u16 nBitsReceived;       //!< how many bits received
    u16 nReceivedmax;       //接收允许的最大长度
    u32 timeout;            //超时时间
    u8  timeoutmode;        //超时定义类型
    u8  timeoutcnt;         //超时次数,针对扩展等待时有用
    u8 cmd;                 //!< command code
    u8 status;              //!< communication status
    u8  ErrorReg;            //collision position 
    u8 nmifareflg;           //操作Mifare标志
} MfCmdInfo; 


///////////////////////////////////////////////
typedef struct
{
    u8  mode;                //0:不查卡
                            //bit0:查询A卡
                            //bit1:查询B卡
                            //bit2:查询C卡
    u8 picctype;            //PICC_TYPEA:下次查询typeA卡
                            //PICC_TYPEB:下次查询typeB卡
                            //PICC_TYPEC:下次查询typeC卡
                            //其它跳转到PICC_TYPEA
    u8 checkcontinue;       //0:继续查卡  
    u8 checkremovflg;       //检查是否移除卡                   
}_nCHECKPARAM;
extern _nCHECKPARAM checkparam;


typedef struct
{
    s8 status;           
    u8 picctype;
    u8 AnticollGrade;    //防冲突等级
    u8 PiccSAK;          //
    u8 MLastSelectedsnrLen;
    u8 MLastSelectedSnr[20];//MLastSelectedSnr[0]保存长度
    u8 AtsLen;           //ATS长度
    u8 AtsData[20];      //ATS数据内容
    u8 TA;
    u8 TB;              //
    u8 TC;
    u8 FSCI;
                        //1etu = fc/128(106kbps)
    u8 FWI;           //FWI = (TB>>4);帧等待时间FWT =(256*16/fc) * (2**FWI) = 32 * (2**FWI) (etu) =32 * (2**(TB>>4)) (etu)
    u8 SFGI;          //帧保护时间SFGT =(256*16/fc) * (2**SFGI) , SFGI = (TB&0x0f);
    u8 atqflg;        
    u8 pupi[4];
    u8 app_dat[4];
    u8 prtcl_inf[4];
    u8 prtcl_inflen;       
    u8 TxIPCB;
    u8 RxIPCB; 
}nPICCData;

#define PICC_CIDCNTMAX          1   //16  //20090821_1

#define MI_OK                           0

typedef struct _RF_PARAM
{
    u8 m_type;             //芯片类型
    u8 rfmodgsp;           //MODGSP  TypeB卡的调制深度(b5~b0,0x00~0x1f)
    u8 rfcfgregA;          //RFCfgReg,控制TypeA卡的接收增益和检测电平  
    u8 rfcfgregB;          //RFCfgReg,控制TypeB卡的接收增益和检测电平
    u8 CWGsP;               //接收增益和检测电平
    u8 Q;                   //Q通道，控制低功耗检测卡对比范围
    u8 I;                   //I通道，控制低功耗检测卡对比范围
    u8 rxthreshold_a;
    u8 rxthreshold_b;
    u8 rfu[21];             //预留
    u8 bcc;                 //暂时使用累加和和异或校验  
    u8 lrc;              
    //u8 crc[2];//crc16校验
}RF_PARAM;
#define RF_PARAM_LEN_MAX     32

extern nPICCData PICCData[PICC_CIDCNTMAX];
extern u8  mifare_ack[2];
extern RF_PARAM g_rf_param;
//extern u8 poll_enable;
//extern u8 blfs;



//定义PICC的状态
#define PICC_STATUS_IDLE        0x00
#define PICC_STATUS_HALT        0X01//pause
#define PICC_STATUS_ATQA        0x02//correct recived ATQA
#define PICC_STATUS_SAK         0x03//correct recived SAK
        
#define PICC_STATUS_ATS         0x04//correct reset
#define PICC_STATUS_PPSS        0x05//correct active




#define MI_NOTAGERR                     (1)  //(-1)
//#define MI_CHK_FAILED                   (1)  //(-1)
#define MI_CRCERR                       (2)  //(-2)
//#define MI_CHK_COMPERR                  (2)  //(-2)
#define MI_EMPTY                        (3)  //(-3)
#define MI_AUTHERR                      (4)  //(-4)
#define MI_PARITYERR                    (5)  //(-5)
#define MI_CODEERR                      (6)  //(-6)

#define MI_CRC_NOTZERO                  7

#define MI_SERNRERR                     (8)  //(-8)
#define MI_KEYERR                       (9)  //(-9)
#define MI_NOTAUTHERR                   (10) //(-10)
#define MI_BITCOUNTERR                  (11) //(-11)
#define MI_BYTECOUNTERR                 (12) //(-12)
#define MI_IDLE                         (13) //(-13)
#define MI_TRANSERR                     (14) //(-14)
#define MI_WRITEERR                     (15) //(-15)
#define MI_INCRERR                      (16) //(-16)
#define MI_DECRERR                      (17) //(-17)
#define MI_READERR                      (18) //(-18)
#define MI_OVFLERR                      (19) //(-19)
#define MI_POLLING                      (20) //(-20)
#define MI_FRAMINGERR                   (21) //(-21)
#define MI_ACCESSERR                    (22) //(-22)
#define MI_UNKNOWN_COMMAND              (23) //(-23)
#define MI_COLLERR                      (24) //(-24)
#define MI_RESETERR                     (25) //(-25)
#define MI_INITERR                      (25) //(-25)
#define MI_INTERFACEERR                 (26) //(-26)
#define MI_ACCESSTIMEOUT                (27) //(-27)
#define MI_NOBITWISEANTICOLL            (28) //(-28)
#define MI_QUIT                         (30) //(-30)

#define MI_MORECARD                     (31)    //有多张卡
#define MI_OTHERERR                     (32)
                                             //
#define MI_RECBUF_OVERFLOW              (50) //(-50)
#define MI_SENDBYTENR                   (51) //(-51)
                                             //
#define MI_SENDBUF_OVERFLOW             (53) //(-53)
#define MI_BAUDRATE_NOT_SUPPORTED       (54) //(-54)
#define MI_SAME_BAUDRATE_REQUIRED       (55) //(-55)
                                             //
#define MI_WRONG_PARAMETER_VALUE        (60) //(-60)    //参数错误
                                             //
#define MI_BREAK                        (99) //(-99)
#define MI_NY_IMPLEMENTED               (100)//(-100)
#define MI_NO_MFRC                      (101)//(-101)
#define MI_MFRC_NOTAUTH                 (102)//(-102)
#define MI_WRONG_DES_MODE               (103)//(-103)
#define MI_HOST_AUTH_FAILED             (104)//(-104)
                                             //
#define MI_WRONG_LOAD_MODE              (106)//(-106)
#define MI_WRONG_DESKEY                 (107)//(-107)
#define MI_MKLOAD_FAILED                (108)//(-108)
#define MI_FIFOERR                      (109)//(-109)
#define MI_WRONG_ADDR                   (110)//(-110)
#define MI_DESKEYLOAD_FAILED            (111)//(-111)
                                             //
#define MI_ANTICOLL_ERR                 (112)//(-112)
                                             //
#define MI_WRONG_SEL_CNT                (114)//(-114)
                                             //
#define MI_WRONG_TEST_MODE              (117)//(-117)
#define MI_TEST_FAILED                  (118)//(-118)
#define MI_TOC_ERROR                    (119)//(-119)
#define MI_COMM_ABORT                   (120)//(-120)
#define MI_INVALID_BASE                 (121)//(-121)
#define MI_MFRC_RESET                   (122)//(-122)
#define MI_WRONG_VALUE                  (123)//(-123)//错误的值
#define MI_VALERR                       (124)//(-124)
#define MI_PROTOCOL_ERROR               (125)//(-125)
#define MI_ACK_SUPPOSED                 (126)//(-126)
#define MI_TEMP_ERROR                   (127)//(-127)


#define PCD_TIMER_MODE_100US_0      0           //不受协议的影响
#define PCD_TIMER_MODE_100US_1      1           //发送完成后启动定时

#define PCD_TIMER_MODE_ETU_0        2
#define PCD_TIMER_MODE_ETU_1        3

#define PCD_TIMER_MODE_nETU_0       4       //35etu模式
#define PCD_TIMER_MODE_nETU_1       5

#define PCD_TIMER_MODE_nETU1_0       6      //32etu模式
#define PCD_TIMER_MODE_nETU1_1       7


#define PICC_CIDCNTMAX          1   //16  //20090821_1
#define PCDFSDI                 8   //定义PCD能收到的帧的最大长度
#define PCDFSD                 256  //64   //定义PCD能收到的帧的最大长度
#define PCDWORKBUFLEN          256


//定义PHASE错误类型////////////////////////////
#define PCD_PHASE_NO_ERR            0   //没有错误
#define PCD_PHASE_PCB_ERR           3   //PCB错
#define PCD_PHASE_BLOCKNUM_ERR      4   //块序号错
#define PCD_PHASE_CID_ERR           5   //CID错
#define PCD_PHASE_PARA_ERR          6   //数据包参数错
#define PCD_PHASE_ITX_ERR           7   //I块发送错误
#define PCD_PHASE_PROTOCOL_ERR      8   //协议错
#define PCD_PHASE_PROTOCOL_ERR_2    9   //协议错,下电处
#define PCD_PHASE_I_RETRANS         10   //重发I块


#define RF_MODE_PICCA        0x01
#define RF_MODE_PICCB        0x02
#define RF_MODE_PICCC        0x04
#define RF_MODE_24           0x08  //2.4G卡
#define RF_MODE_AB          (RF_MODE_PICCA|RF_MODE_PICCB)



/* Definitions for Request command. */
#define REQUEST_BITS                    0x07
#define ATQA_LENGTH                     0x02


/* Command and Parameter byte definitions for HaltA function. */
#define HALTA_CMD                       0x50
#define HALTA_PARAM                     0x00
#define HALTA_CMD_LENGTH                0x02


/* Definitions for lower Anticollision / Select functions. */
#define BITS_PER_BYTE                   0x08
#define UPPER_NIBBLE_SHIFT              0x04
#define COMPLETE_UID_BITS               0x28
#define NVB_MIN_PARAMETER               0x20
#define NVB_MAX_PARAMETER               0x70

/* Definitions for Select functions. */
#define SAK_LENGTH                      0x01

#define RF_CMDNUM_TRANSCEIVE           0
#define RF_CMDNUM_AUTHENT              1

#define RF_CMDNUM_MAX               2

#define RF_LP_STEP_POWERON   0
#define RF_LP_STEP_QUERY     1
#define RF_LP_STEP_OPERATE   2
#define RF_LP_STEP_REMOVE    3
#define RF_LP_STEP_REMOVE1   4
#define RF_LP_STEP_POWERON1  5

//u32 dev_pcd_read_intpin(void);

#if 1
//void dev_rf_set_lpmod(u8 mod);
//u8 dev_rf_get_lpmod(void);
//u8 dev_rf_get_r_s_flg(void);
u8 dev_rf_get_cwgsp_ctl(void);
void rf_delayms(u32 ms);
s32 dev_rf_readallparam(u8 *rbuf, u8 rlen);
s32 dev_rf_writeallparam(u8 *wbuf, u8 wlen);

void dev_pcd_param_set_default(void);
u8 dev_pcd_set_param(u8 *wbuf, u16 wlen);
u8 dev_pcd_read_param(u8 *rbuf, u8 *rlen);
s32 dev_pcd_set_para(u8 mode, u8 type);
void dev_pcd_export_init(void);
void dev_pcd_export_deinit(void);
void dev_pcd_export_ctl(u8 num, u8 flg);
s32 dev_pcd_open(void);
void dev_pcd_sleep_ctl(u8 flg);
s32 dev_pcd_close(void);
s32 dev_pcd_check_picc(u8 *cid, u8 *rbuf, u16 *rlen);
s32 dev_pcd_check_picc1(u8 *cid, u8 *rbuf, u16 *rlen);
s32 dev_pcd_active_picc(u8 cid, u8 *rbuf, u16 *rlen);
s8 dev_pcd_checkremov_picc(void);
s8 dev_pcd_exchange_data(u8 cid, u8 *wbuf, u16 wlen, u8 *rbuf, u16 *rlen);
s32 dev_pcd_get_uid(u8 *rbuf, u8 rle);           

s32 dev_pcd_powerup(u8 type);
s32 dev_pcd_powerdown(void);
u8 dev_pcd_query_picc(u8 *cid, u8* rbuf, u16 *rlen);

s32 dev_pcd_ctrl_carrier(u8 mode);  
s32 dev_pcd_loopback_reset(u16 timers);


void rf_loopback_init(void);
void rf_loopback_ctl(u8 flg);

s32 dev_pcd_ctl_crc(u8 mask, u8 mod);
void dev_pcd_reset_mfinfo(MfCmdInfo *p_mfcmdinfo);
s8 dev_pcd_execute_cmd(u8 cmd, MfCmdInfo *p_mfcmdinfo, u8 *wbuf, u8 *rbuf);
s32 dev_pcd_mifare_auth(u8 key_type, u8 *key, u8 *snr, u8 block);
s32 dev_rf_read_param(void);
s32 dev_rf_write_param(void);
void pcd_remov_picc(void);
void dev_pcd_reset(s32 _ms);
void dev_pcd_set_timeout(u8 mode, u16 timer);
s8 dev_pcd_checkremov_sta(void);

s32 dev_pcd_check_typeApresent(void);
s32 dev_pcd_check_typeAAnticollision(void);
s32 dev_pcd_check_typeBpresent(void);
s32 dev_pcd_check_typeBAnticollision(void);
void dev_pcd_get_piccparam(nPICCData* lpPICCData);
s32 dev_pcd_typeARemove(void);
s32 dev_pcd_typeBRemove(void);
s32 dev_pcd_poll(u8 card_type, u8 *rbuf, u32 *rlen, u32 rbufsize);
s32 dev_pcd_poll_1(u8 card_type, u8 *rbuf, u32 *rlen, u32 rbufsize);
u8 dev_pcd_get_cardtype(void);
void dev_rf_test_rtsa(s32 mod);
#endif



#endif
