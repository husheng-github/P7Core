

#ifndef _FS_H_
#define _FS_H_

#include "devglobal.h"
//#define FS_DEBUG              //sxl?20170626
//#define SYSTEMFS_DEBUG

#ifdef FS_DEBUG
#define VfsDebug dev_debug_printf
#else
#define VfsDebug(fmt,arg...)
#endif


#ifdef SYSTEMFS_DEBUG
#define FsDebug dev_debug_printf
#define FsDebugLog dev_debug_printf
#else
#define FsDebug(fmt,arg...) 
#define FsDebugLog(fmt,arg...) 
#endif


#ifdef LINUXVFS
typedef struct file* DQFILE; 
#else

typedef unsigned int loff_t;
typedef unsigned int mm_segment_t;


typedef     int     DQFILE; 
#endif


#define FS_TYPE_STATIC    0
#define FS_TYPE_DYNAMIC   1
#define FS_TYPE_SYSTEM    2


typedef enum 
{
	
	VFS_RESULT_SUCCESS = 0,
	VFS_RESULT_FILENOTEXIST = -1,
	VFS_RESULT_FILEEND = -2,
	VFS_RESULT_FILEERR = -3,
	VFS_RESULT_PARAMERR = -4
}VFS_RESULT;

#define FILESYSMTDNAMELEN 16
struct _sFileSysMtd//文件信息字段
{
    u8 name[FILESYSMTDNAMELEN];    //名字
    u32 addr;         //地址
    s32 len;          //长度
};



#include "./dynamicfile/fs_dynamic.h"
#include "./staticfile/fs_static.h"
#include "./sysfile/fs_system.h"
#include "./sysfile/fs_systembase.h"






typedef DQFILE (*vfs_core_open)(const s8 *pathname,const s8 *mode,int *result);
typedef int  (*vfs_core_read)(const s8 *pathname, u8 *readdata, u32 readlen, u32 readaddr,DQFILE fp);
typedef int  (*vfs_core_write)(const s8 *pathname, u8 *writedata, u32 writelen, u32 writeaddr,DQFILE fp);
typedef int  (*vfs_core_seek)(DQFILE File, int offset, int whence);
typedef int  (*vfs_core_close)(const char *pathname,DQFILE fp);
typedef int  (*vfs_core_tell)(const char * pathname,DQFILE fp);



typedef struct{
	
	vfs_core_open  virtual_vfs_open;
	vfs_core_close virtual_vfs_close;
	vfs_core_read  virtual_vfs_read;
	vfs_core_write virtual_vfs_write;
	vfs_core_seek  virtual_vfs_seek;
	vfs_core_tell  virtual_vfs_tell;
	
}VFS_VIRTUAL_FUNCITON;




extern DQFILE DQ_vfs_open(const s8 *pathname,const s8 *mode,int *result);
extern int DQ_vfs_read(const s8 *pathname, u8 *readdata, u32 readlen, u32 readaddr,DQFILE fp);
extern int DQ_vfs_write(const s8 *pathname, u8 *writedata, u32 writelen, u32 writeaddr,DQFILE fp);
extern int DQ_vfs_seek(DQFILE File, int offset, int whence);
extern int DQ_vfs_close(const char *pathname,DQFILE fp);
extern int DQ_vfs_tell(const char * pathname,DQFILE fp);

extern s32 fs_create_file(const char *filename, s32 len,u8 initialvalue);
extern s32 fs_read_file(const char *filename,u8 *rbuf, s32 rlen, s32 startaddr);
extern s32 fs_write_file(const char *filename,u8 *writedata,s32 writedatalen);
extern s32 fs_insert_file(const char *filename,u8 *wbuf, s32 wlen, s32 startaddr);
extern s32 fs_access_file(const s8 *filename);
extern s32 fs_delete_file(const char *filename);
extern s32 fs_rename_file(const char *srcfilename,const char *dstfilename);

extern void fs_system_init(void);
#endif


