

#ifndef _PCITAMPERMANAGEMMKBAK_H_
#define _PCITAMPERMANAGEMMKBAK_H_


#define USEMMKBAKFILE



#define PCIMMKBAKFILENMAE      "/mtd2/mmk"
#define PCIMMKBAKBAKFILENMAE   "/mtd2/mmkbpk"


#define PCIMMKBPKADDR  0x1E000  //sxl?要统一定一个接口
#define INTERNALSECTORSIZE  0x800


#define PCIMMKBPKFILEFLAG       "\xaa\xbb\xaa\xbb"

#define MMKBPKDATALENGTH  32
#define MMKBPKCOVERTIMES  3

typedef struct _PCIMMKBPKINFO
{
    u8 flag[4];
	u8 mmkdata[32];
	u8 dsr_sr[8];
    u8 crc[4];
}PCIMMKBPKINFO;

#define PCIMMKBPKFILELENGTH sizeof(PCIMMKBPKINFO)
#define ENCRYPTBLOCKLENGTH PCIMMKBPKFILELENGTH



extern s32 pcitamper_manage_mmkbakfileread(u8 *mmkbpkdata);
extern s32 pcitamper_manage_mmkbakfilesave(u8 *mmkdata,u8 mmkdatalen);
extern s32 pcitamper_manage_mmkbakcheckifconsistency(u8 *originalmmkdata,s32 originalmmkdatalen);
extern s32 pcitamper_manage_mmkbakfilecheckifexist(void);
extern s32 pcitamper_manage_mmkbakrecover(u8 *mmkbpk);

#endif

