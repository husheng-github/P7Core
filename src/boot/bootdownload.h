

#ifndef _BOOTDOWNLOAD_H_
#define _BOOTDOWNLOAD_H_


#define HARDWAREVER  0x10
#define MACHINECODE  0x51



#define BOOT_CORE_OFFSET 2048

typedef enum _BOOTDOWNLOAD_BEEP_TYPE
{
	BOOTDOWNLOAD_BEEP_NORMAL = 0,
	BOOTDOWNLOAD_BEEP_ERR,
}BOOTDOWNLOAD_BEEP_TYPE;

typedef enum _BOOTDOWNLOAD_FILE_TYPE
{
	
    BOOTDOWNLOAD_FILE_TYPE_BOOTBIN,
	BOOTDOWNLOAD_FILE_TYPE_COREBIN,
	BOOTDOWNLOAD_FILE_TYPE_PCICOREBIN,
	BOOTDOWNLOAD_FILE_TYPE_APPBIN,
	BOOTDOWNLOAD_FILE_TYPE_TREDNITPK,
	BOOTDOWNLOAD_FILE_TYPE_ACQUIRERPK,
	BOOTDOWNLOAD_FILE_TYPE_AUTHEN1PK,
	BOOTDOWNLOAD_FILE_TYPE_AUTHEN2PK,
	BOOTDOWNLOAD_FILE_TYPE_AUTHEN3PK,
	BOOTDOWNLOAD_FILE_TYPE_AUTHEN4PK,
	BOOTDOWNLOAD_FILE_TYPE_PREVENTPK,
	BOOTDOWNLOAD_FILE_TYPE_VFSCONFIG,
	BOOTDOWNLOAD_FILE_TYPE_RESOURCE,
	BOOTDOWNLOAD_FILE_TYPE_RESERVED,
	BOOTDOWNLOAD_FILE_TYPE_UPDATEBAK,
	BOOTDOWNLOAD_FILE_TYPE_NULL,
	
}BOOTDOWNLOAD_FILE_TYPE;







#define MAXFILE_NUM  6   //一次最多可以下载30个文件



#define DOWNLOADBLOCKOFFSET       12
#define PC_DL_BLOCK_SIZE    (4096)
#define PC_DL_FRAME_SIZE_MAX    (68+PC_DL_BLOCK_SIZE)



#define MAXFILENAMELEN  20
typedef struct _CANDOWNFILEINFO
{
    u8  filename[MAXFILENAMELEN];//文件名
    u8  filetype;
    s32 fileaddr;       //下载的地址
    u32 maxfilesize;
}CANDOWNFILEINFO;



typedef struct 
{  
    u32 filelength;    
    u32 flag;       
    u32 crc;
    s32 addr;
    u32 failedflag;
	u8  filetype;
    u8  filename[51];
}DOWNFILE;




typedef struct {
    u8 filenum;         //file number need to download
    u8 bagno;           //current download file bag NO.
    u8 bagnosum;        //current download file total bag number
    u8 machtyp;         //machine tyoe
    u8 hardver;         //hardware version
    u8 filename[51];    //file name must not overflow 50 bytes
    u8 filetype;
    u32 filelength;     //current download file length
    u32 crc;            //file CRC
    u32 offset;         //offset of the current download bag in file
    u8 *p;              //pointer to download file data
}FILEBAGINFO;



extern void bootdownload_init(void);
extern void bootdownload_exit(void);
extern void bootdownload_initialcandownfileinfo(void);
extern s32 bootdownload_task(void);
extern void bootdownload_processrecbag(FILEBAGINFO *lp_filebaginfo);

extern void bootdownload_beep(u8 beeptype);
extern void bootdownload_processerr(u8 bagno,u8 errtype);
extern s32 bootdownload_dealdownloadlist(FILEBAGINFO *lp_filebaginfo);
extern s32 bootdownload_fread_specifyaddr(u8 *data,s32 fileoffset,u32 length,u32 fileorgaddr);
extern s32 bootdownload_GetDirAndName(u8 *tempname,u8 *dir,u8 *name);
extern s32 bootdownload_GetFiletype(u8 *path, u8 *filetype);
extern s32 bootdownload_checkacquirerpkexist(void);
extern void bootdownload_processerr_response(u8 errtype,u8 *promptlen,u8 *prompt);

extern void core_debug_wait(u32 step);
extern s32 bootdownload_savebincrc(u8 *filename,u32 crc);


#endif

