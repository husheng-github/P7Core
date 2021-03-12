

#ifndef  _PCIKEYMANAGEAUTHEN_H_
#define  _PCIKEYMANAGEAUTHEN_H_

#define PCIAPPKEYLOCKINFOFILE      "appkeylock.info"   


#define KEYTYPE_GETPIN_PINK   0x01
#define KEYTYPE_GETPIN_FIXEDK 0x02
#define KEYTYPE_GETPIN_DUKPTK 0x03



#define MAXINPUTPINTIMES      10  //5 minutes max input pin times



//#define PCIPOSAUTHEN

#define MAXFAILEDTIMES        5
#define PCIMINTIMERCHECK     (5*60*1000)   //5 minutes
#define APPKEYLOCKTIMES      "\x00\x00\x00\x00\x00\x00\x01\x20" 

#define PCIPOSAUTHEN_DOWNAPPKEY_REQUEST 0
#define PCIPOSAUTHEN_DOWNAPPKEY_SAVE    1
#define PCIPOSAUTHEN_DOWNAPPKEY_AUTH    2
#define PCIPOSAUTHEN_DOWNAPPKEY_CHECK   3
#define PCIPOSAUTHEN_DOWNAPPKEY_CHECKFAIL 4
#define PCIPOSAUTHEN_GETPIN_CHECKPININTERVAL 5
#define PCIPOSAUTHEN_DOWNAPPKEY_CHECKSUC  6
#define PCIPOSAUTHEN_SETWK_CHECK          7
#define PCIPOSAUTHEN_DELETEALLKEY            9
#define PCIPOSAUTHEN_DOWNMAGKEY_CHECK        10
#define PCIPOSAUTHEN_DOWNMAGKEY_REQUEST     11
#define PCIPOSAUTHEN_DOWNPOSAUTHENKEY_CHECK   12



#define AUTHENTICATION_APPKEYHEADINFO_SIZE 48
#define AUTHENTICATION_APPKEYNAMEHEADINFO_SIZE (48+64)
#define AUTHENTICATION_APPKEYHEAD_SIZE  (48+64+32) //add appname



typedef struct __APPKEYLOCKINFO
{
    u8 appkeyindex;         //application key index 
	u8 reserve1;            // reserve for future use
	u8 failedtimes;         //
	u8 tickslocked[8];      //ticks when locked
	u8 reserve2[8];         //reserve for future use
	u8 random[8];          //random number
	u8 checkvalue[5];            
}APPKEYLOCKINFO;
#define APPKEYLOCKINFO_SIZE   sizeof(struct __APPKEYLOCKINFO)


typedef struct _SECURITYAUTHENTICATEINFO
{
    struct __APPKEYLOCKINFO appkeylockinfo;
    u8 IfCertified;
	u8 PTKey[25];
	u8 TMacKey[25];  
	u8 Appname[80];  //record the app name when certified
	u8 IssuerSN[AUTHENTICATION_APPKEYHEADINFO_SIZE];
	u32 effectivetime;
}SECURITYAUTHENTICATEINFO;


#define FUNCTIONUSEINTERVAL   30000 //30s
typedef struct _PINKEYUSEINTERNAL
{
	u32 pinktimelastuse;
	u32 fixedktimelastuse;
	u32 dukptktimelatuse;
	
}PINKEYUSEINTERNAL;


#define PCIMANAGEINFOFILE      "/mtd0/pcimanage" 
typedef struct _PCIMANAGE
{
	
	u8 tag[4];
	u8 admin1[8];
	u8 admin2[8];
	u8 admin1key[24];
	u8 admin2key[24];
	u8 reserved[8];
	u8 crc[4];
	
}PCIMANAGE;



extern void pcikeymanageauthen_initialuseinternal(void);
extern s32 pcikeymanageauthen_posins_authen(u32 authenstep,u8 *appinfo,u8 groupindex,u8 *data,s32 datalen);
extern void pcikeymanageauthen_authendata_init(void);
extern s32 pcikeymanageauthen_posinsselect_authen(u32 authenstep,u8 *appinfo,u8 groupindex,u8 *data,s32 datalen);
extern void pcikeymanageauthen_managekey_transfer(u8 *tempptk);
extern void pcikeymanageauthen_securityauthinfo_initial(void);
extern s32 pcikeymanageauthen_appkeylockfile_read(u8 groupindex,APPKEYLOCKINFO *lockinfo);
extern s32 pcikeymanageauthen_checkmaingroupfile(u8 groupindex);
extern s32 pcikeymanageauthen_groupapplockfile_initial(u8 groupindex);
extern s32 pcikeymanageauthen_authen_downappkeycheck(u8 groupindex);
extern s32 pcikeymanageauthen_savegroupkeyappname(u8 groupindex,u8 *appname);
extern void pcikeymanageauthen_getpin_updatelastusetime(u8 keytype);
extern void pcikeymanageauthen_cmd_decreaselocktimer(void);
extern void pcikeymanageauthen_readpcimanageinfo(void);
extern s32 pcikeymanageauthen_savepcimanagefile(u8 adminno,u8 *adminkey);
extern s32 pcikeymanageauthen_savepcimanagefile_file(PCIMANAGE *temppcimanagefile);
extern s32 pcikeymanageauthen_readpcimanagefile(PCIMANAGE *temppcimanagefile);
extern s32 pcikeymanageauthen_applockinfo_get(APPKEYLOCKINFO *lockinfo);
extern s32 pcikeymanageauthen_applockfile_initial(void);
extern s32 pcikeymanageauthen_sensitiveservicheck(u8 groupindex,u8 *appname);
extern s32 pcikeymanageauthen_readappkeyheadinfoappname(u8 groupindex,u8 *appname);


#endif

