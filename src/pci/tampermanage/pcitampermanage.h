

#ifndef _PCITAMPERMANAGE_H_
#define _PCITAMPERMANAGE_H_


//#define TAMPERMANAGEINFO_FILENAME         "/mtd2/tamperman"
//#define TAMPERMANAGEINFO_BAKFILENAME      "/mtd2/tampermanbak"
#define TAMPERMANAGEINFO_FILEFLAG         "TAMP"
#define DEFAULT_TAMPERENABLEVALUE         "\xFF\x05\xFF\x0F"   
#define DEFAULT_MMKTAMPERVALUE            (1<<31)



typedef enum _PCISECURITYSTATUS {
	SYSTEMERROR_NONE = 0,
	SYSTEMERROR_NOTSECURITY,
	SYSTEMERROR_MMKREGENERATE,
	SYSTEMERROR_EXECUTION,
	SYSTEMERROR_MAGKEYNEEDINIT,
	SYSTEMERROR_MAGREPLACED,
	SYSTEMERROR_MAGNEEDREPLACED,
	SYSTEMERROR_MAGNEEDINIT,
	SYSTEMERROR_MAGERR,
	SYSTEMERROR_FANQIJIEFAIL

}PCISECURITYSTATUS;



typedef struct{
	u32 gTAMPERENABLEVALUE;
	u32 gTamperDetectValue;
	u32 gTamperCurrentValue;
}PCITAMPERMANAGE;

//触发管理文件
typedef struct
{
	
	u8 fileflag[4];
    u8 pciflag[20];
	u8 tamperenablevalue[4];
	u8 tamperreg[4];
	u8 mmkdata[36];
	u8 unlocktimes;
	u8 tamperopenflag;
	u8 sen_exttype_reg[4];
	u8 dryicereg[36];
//	u8 reserved[28];
//	u8 crc[2];
	
	
}PCITAMPERMANAGEFILEINFO;




extern void pcitamper_manage_savetamperregdata(void);
extern void pcitamper_manage_savetamper (u32 tamperstatusvalue);
extern s32 pcitampermanage_poweroncheckMMK(void);
extern s32 pcitampermanage_readELRCK(u8 *elrckdata,s32 *elrckdatalen);
extern void pcitampermanage_powerondelmmkbpkfile(void);
extern s32 pcitampermanage_regeneratemmkelrck(void);
extern void pcitampermanage_tamperunlock(void);
extern u32 pcitampermanage_save_tampertimes(u8 type,u32 tampervalue);
extern s32 pcitampermanage_readMMK(u8 *mmkdata,s32 *mmkdatalen);
extern void pcitampermanage_regeneratemmkelrck_initialvalue(void);
extern s32 pcitampermanage_poweroncheckmmk(u8 poweronflag);
extern void pcitampermanage_initial(void);
extern u32 pcitampermanage_getsavedsenexttype(void);
extern void pcitampermanage_savesenexttype(u32 senexttyperegvalue);
extern void pcitamper_manage_clrbpkporstatus(void);
extern void pcitamper_manage_poweronreadporstatus(void);

#endif

