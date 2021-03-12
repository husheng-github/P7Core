

#ifndef _PCIKEYMANAGE_DUKPTK_H_
#define _PCIKEYMANAGE_DUKPTK_H_




#define NEEDCALL_NEWKEY1          0
#define NEEDCALL_NEWKEY2          1
#define NEEDCALL_NEWKEY3          0
#define NEEDCALL_NEWKEY4          1


#define DUKPTKFLAG_CREATE         0
#define DUKPTKFLAG_INITIAL        1
#define DUKPTKFLAG_OVERFLOW       2

#define DRV_PCI_APP_FILELEN       20

//#define DRV_PCI_DUKPTKKEY_PATH   "/mtd1/"
#define DRV_PCI_DUKPTKKEY_PATH   "/mtd0/"    //add by ghz 2011-07-08
#define DRV_PCI_DUKPTKEY_SUFFIX  ".dukpt"


typedef struct _DUKPTK_INFO
{
	s8 currappname[100];
	u8 KSN[10];
	u8 currentkeyindex;
	u8 curfuturekeyindex;
	u8 cryptreg1[8];
	u8 cryptreg2[8];
	u8 cryptreg3[8];
	u8 keyreg[24];
	u32 encryptcnt;
	u32 shiftreg;
}DUKPTK_INFO;

typedef struct _DUKPT_FUTUREKEY
{
	u8 keylen;
	u8 keydata[24];
    u8 keycrc[2];
	u8 reserved;
}DUKPT_FUTUREKEY;

typedef struct _DUKPT_KEY
{
    u8 appno;
    u8 keyindex;
	u8 keyflag;
	u8 KSN[10];
	u8 KSNcrc[2];
	u8 reversed;
	DUKPT_FUTUREKEY futurekey[21];
	
}DUKPT_KEY;







#define MAXKSNDATALEN          10
#define FUTUREKEY_MAXINDEX 21
#define FUTUREKEY_LEN      16
#define DUKPTKHEADLEN     16
#define DUKPTKEY_MAXNUM   10
#define DUKPTFUTUREKSIZE  sizeof(DUKPT_FUTUREKEY) 
#define DUKPTKSIZE        sizeof(DUKPT_KEY)
#define DUKPTKFILESIZE    (DUKPTKEY_MAXNUM*sizeof(DUKPT_KEY))



extern s32 pcikeymanage_getdukptkeyfile_absolutepath(u8 groupindex,s8 *absolutepath);
extern s32 pcikeymanage_dukptk_read(u8 *dukptkdata,s32 dukptkdatalen,unsigned int pos);
extern s32 pcikeymanage_dukptk_write(u8 *dukptkdata,s32 dukptkdatalen,unsigned int pos);
extern s32 pcikeymanage_dukptk_readKSN(u8 dukptkindex,u8 *ksndata);
extern s32 pcikeymanage_dukptk_saveKSN(u8 dukptkindex,u8 *ksndata);
extern s32 pcikeymanage_dukptk_savefuturekey(u8 dukptkindex,u8 futurekeyindex,u8 keylen,u8 *keydata);
extern s32 pcikeymanage_dukptk_readfuturekey(u8 dukptkindex,u8 futurekeyindex,u8 *futurekeylen,u8 *futurekeydata);
extern s32 pcikeymanage_dukptk_savekeyflag(u8 dukptkindex,u8 dukptkflag);
extern void pcikeymanage_dukptk_futurekeygeneratealgorithm(void);
extern s32 pcikeymanage_dukptk_generatenewfuturekey1(void);
extern s32 pcikeymanage_dukptk_generatenewfuturekey2(void);
extern s32 pcikeymanage_dukptk_generatenewfuturekey3(void);
extern s32 pcikeymanage_dukptk_addksnchangecnt(u32 counter);
extern s32 pcikeymanage_dukptk_checkifgennewfuturekey(void);
extern s32 pcikeymanage_dukptk_generatenewfuturekey4(u8 mode);
extern s32 pcikeymanage_initial_dukptkfile(void);
extern s32 pcikeymanage_dukptk_futurekeyinitial(u8 dukptkindex,u8 *ksndata);
extern s32 pcikeymanage_pos_savedukptkey(u8 groupindex,u8 keyindex,u8 keylen,u8 ksnlen,u8 *keyksndata);
extern s32 pcikeymanage_set_currappdukptk(u8 groupindex);
extern void pcikeymanage_dukptk_setshiftreg(void);
extern s32 pcikeymanage_dukptk_getandscatter(u8 pinormac,u8 *keydata,u8 *keylen,u8 *ksndata);
extern void pcikeymanage_dukptk_clrglobalvariable(void);
extern s32 pcikeymanage_getcurrtrans_dukptkinfo(u8 pinormac,u8 keyindex,u8 *keydata,u8 *keylen,u8 *ksndata);
extern s32 pcikeymanage_check_dukptkey(void);

//sxl20110608
extern s32 pcikeymanage_check_dukptkexist(u8 keyindex);
extern s32 pcikeymanage_poskey_saveappdukptkey(u8 *databuf);
//extern s32 pcikeymanage_dukptk_load(strDukptInitInfo* lpstrDukptInitInfo);  //sxl?2017
//extern s32 pcikeymanage_dukpt_get_pinkey(strDevDukpt *lpstrDevDukpt);
//extern s32 pcikeymanage_dukpt_check_keytmp(u32 nKeyGroup, u32 nKsnIndex,strDevDukpt *lpstrDevDukpt);
extern void pcikeymanage_dukptk_PedPinOrMac(u8 keyflag, u8 *keydata);


#endif



