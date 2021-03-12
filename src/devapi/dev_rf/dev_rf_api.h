#ifndef __DEV_RF_H
#define __DEV_RF_H

#define CARD_TYPE_A     0x01
#define CARD_TYPE_B     0x02
#define CARD_TYPE_AB    0x03

/**@{*/
/*! \ingroup emv
 */
#define EMV_ERR_OK               0 /*!< No error occured. */
#define EMV_ERR_COLLISION       -1 /*!< Received a card response with a bit collision. */
#define EMV_ERR_PROTOCOL        -2 /*!< Received a card response with a protocol error. */
#define EMV_ERR_TRANSMISSION    -3 /*!< Received a card response with a transmission error. */
#define EMV_ERR_TIMEOUT         -4 /*!< Timeout occured while waiting for a card response. */
#define EMV_ERR_INTERNAL        -5 /*!< EMV software stack internal error. */
#define EMV_ERR_STOPPED         -6 /*!< Stop current operation request received. */
/**@}*/

/* Get status return values */
#define PICC_STATUS_NOTPRESENT      1
#define PICC_STATUS_MULTICARD           2
#define PICC_STATUS_SINGLECPUA      3
#define PICC_STATUS_SINGLECPUB      4
#define PICC_STATUS_MEMA                5
#define PICC_STATUS_MEMB                6
#define PICC_STATUS_FELICA              7

typedef struct _strMfAuth
{ 
    u8 m_authmode;          //0x60 for KEYA，0x61 for KEYB
    u8 m_key[6];            //Mifare key,6 bytes
    u8 m_uid[10];           //UID
    u8 m_block;                 //Block number
}strMfAuth;

#if 0
typedef __packed struct _CL_PARAM
{
    u8 ModGsP;           //MODGSP  TypeB卡的调制深度()
    u8 RFCfg_A;          //RFCfgReg,控制TypeA卡的接收增益(b7~b5)  
    u8 RFCfg_B;          //RFCfgReg,控制TypeB卡的接收增益(b4~b2)
    u8 RFOLevel;         //RFONormalLevel,调节RFO输出功率
    //u8 RxTreshold_A;
    u8 RFU[27];//6];             //预留
    u8 crc;  
}CL_PARAM;
#endif

s32 api_rf_init(void);
s32 api_rf_open(void);
s32 api_rf_close(void);
s32 api_rf_poweron (u32 nType);
s32 api_rf_poweroff(void);
s32 api_rf_get_status(void);
s32 api_rf_remove(void);
s32 api_rf_activate(void);
s32 api_rf_exchange_apdu(const u8*lpCApdu, u32 nCApduLen, u8*lpRApdu, u32 *lpRApduLen, u32 nRApduSize);
s32 api_rf_ioctl(u32 nCmd, u32 lParm, u32 wParm);

#endif
