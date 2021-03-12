


#ifndef _BOOTDOWNLOADAPPINFO_H_
#define _BOOTDOWNLOADAPPINFO_H_


#define FIRMWAREINFOFILENAME "/mtd2/firmwareinfo"
#define FIRMWAREINFOBAKFILENAME "/mtd2/firmwareinfobak"



#define K21SUPPORTMAXAPPNUM 6
#define K21SUPPORTMAXAPPNAMELEN 32


//版本信息//sxl?
typedef struct _APPINFODEF
{
    u8 appname[K21SUPPORTMAXAPPNAMELEN];
    //u8 appfilename[K21SUPPORTMAXAPPNAMELEN];
    u32 startaddr;
    u32 filelen;
    u8 reserved[64-8-K21SUPPORTMAXAPPNAMELEN];
}APPINFODEF;


//放1K的空间来保存
typedef struct _APPINFO
{
    u8 flag[4];
    u32 apptotalnum;
    APPINFODEF appinfo[K21SUPPORTMAXAPPNUM];
    u8 reserved[512 - K21SUPPORTMAXAPPNUM *64 - 12];
    u32 crcdata;
}APPINFO;

#define APPINFOFILELEN sizeof(APPINFO)


extern void bootdownload_readappinfo(void);
extern void bootdownload_saveappinfo(void);
extern s32 bootdownload_savecorefileinfo(u8 filetype, u32 filelen);
extern s32 bootdownload_savepkfile(u8 filetype);
extern s32 bootdownload_getcorefileinfo(u8 filetype, u32 *filelen);


#endif

