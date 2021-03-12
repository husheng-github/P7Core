

#ifndef _PCIKEYDOWNLOAD_H_
#define _PCIKEYDOWNLOAD_H_

#define MAXMAGKEYINDEX        10     //sxl?


#define KM_APP_SIGN_KEY             0x00 // Application program signature key
#define KM_APP_AUTH_LCD_KEY         0x01
#define KM_APP_AUTH_MAIN_PIN_KEY    0x02
#define KM_APP_AUTH_MAIN_MAC_KEY    0x04
#define KM_APP_AUTH_MAC_KEY         0x05
#define KM_APP_AUTH_OFFPIN_KEY      0x06
#define KM_PCI_SK_MACK              0x09
#define KM_PCI_MAIN_PIN_KEY         0x10
#define KM_PCI_PIN_KEY              0x11
#define KM_PCI_MAIN_MAC_KEY         0x12
#define KM_PCI_MAC_KEY              0x13
#define KM_PCI_FKEY                 0x14

extern s32 gDownLoadKeyErr;



extern s32 pcikeydownload_posauthenkey(u8 *rxbuf);
extern s32 pcikeydownload_appkey(u8 *rxbuf);
extern s32 pcikeydownload_groupkeyappname_set(u8 *rxbuf);
extern s32 pcikeydownload_appdukptkey(u8 *rxbuf);
extern s32 pcikeydownload_setpink(u8 *rxbuf,s32 rxbuflen);
extern s32 pcikeydownload_setmack(u8 *rxbuf,s32 rxbuflen);
extern s32 pcikeydownload_settdk(u8 *rxbuf,s32 rxbuflen);

#endif


