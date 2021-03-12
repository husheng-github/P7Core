#ifndef _DDI_FILE_H_
#define _DDI_FILE_H_


#include "ddi_common.h"


/**------------------------------------------------------------------*
 **     mcfile access Mode(bit4-bit7)                                   *
 **     only have following choice:                                  *
 **------------------------------------------------------------------*/
#define     FILE_ACCESS_MODE_CREATE_NEW       ((u32)(0x1<<4))
#define     FILE_ACCESS_MODE_CREATE_ALWAYS    ((u32)(0x2<<4))
#define     FILE_ACCESS_MODE_OPEN_EXISTING    ((u32)(0x3<<4))
#define     FILE_ACCESS_MODE_OPEN_ALWAYS      ((u32)(0x4<<4))
#define     FILE_ACCESS_MODE_APPEND           ((u32)(0x5<<4))
/**------------------------------------------------------------------*
 **     mcfile operation Mode(bit0-bit4)                              \
 **------------------------------------------------------------------*/
#define     FILE_OPERATE_MODE_READ            ((u32)(0x1<<0))
#define     FILE_OPERATE_MODE_WRITE           ((u32)(0x1<<1))
#define     FILE_OPERATE_MODE_SHARE_READ      ((u32)(0x1<<2))
#define     FILE_OPERATE_MODE_SHARE_WRITE     ((u32)(0x1<<3))

enum vfs_whence_flags {
    VFS_SEEK_SET = 0,   // Seek relative to an absolute position
    VFS_SEEK_CUR = 1,   // Seek relative to the current file position
    VFS_SEEK_END = 2,   // Seek relative to the end of the file
};

#define FILE_SEEK_BEGIN       0  /*Beginning of file */
#define FILE_SEEK_CUR         1  /*Current position of file */
#define FILE_SEEK_END         2  /*End of file   */

/*虚拟文件系统控制命令*/
//#define     DDI_VFS_CTL_VER                     (0) //获取虚拟文件系统程序版本
//#define     DDI_VFS_CTL_FLUSH                   (1) //保存文件命令

//=====================================================
//对外函数声明
extern s32 ddi_file_create(const char *filename, s32 len,u8 initialvalue);
extern s32 ddi_file_write(const char *filename,u8 *writedata,s32 writedatalen);
extern s32 ddi_file_read(const char *filename,u8 *rbuf, s32 rlen, s32 startaddr);
extern s32 ddi_file_insert(const char *filename,u8 *wbuf, s32 wlen, s32 startaddr);
extern s32 ddi_file_delete(const char *filename);
extern s32 ddi_file_rename(const char *srcfilename,const char *dstfilename);
extern s32 ddi_file_getlength(const s8 *lpFileName);
extern s32 ddi_file_veritysignature(const s8 *lpFileName,u8 mode,u32 fileaddr,u32 filelen);
s32 ddi_flash_read(u32 addrsta, u8* rbuf, u32 rlen);
s32 ddi_flash_write(u32 addrsta, u8* wbuf, u32 wlen);
//===================================================
typedef s32 (*core_ddi_file_create) (const char *filename, s32 len,u8 initialvalue);  
typedef s32 (*core_ddi_file_write) (const char *filename,u8 *writedata,s32 writedatalen);
typedef s32 (*core_ddi_file_read) (const char *filename,u8 *rbuf, s32 rlen, s32 startaddr);
typedef s32 (*core_ddi_file_insert) (const char *filename,u8 *wbuf, s32 wlen, s32 startaddr);
typedef s32 (*core_ddi_file_delete)(const char *filename);
typedef s32 (*core_ddi_file_rename) (const char *srcfilename,const char *dstfilename);
typedef s32 (*core_ddi_file_getlength) (const s8 *lpFileName);
typedef s32 (*core_ddi_file_veritysignature)(const s8 *lpFileName,u8 mode,u32 fileaddr,u32 filelen);
typedef s32 (*core_ddi_static_vfs_getarea)(const char * pathname,int fp,u32 *length);
typedef int (*core_ddi_vfs_open)(const char *pathname,const unsigned char * mode);
typedef int (*core_ddi_vfs_close)(int fp);
typedef int (*core_ddi_vfs_seek)(int fp, int offset, int whence);
typedef int (*core_ddi_vfs_tell)(int fp);
typedef int (*core_ddi_vfs_read)(int fp, void *buf, u32 count);
typedef int (*core_ddi_vfs_write)(int fp, void *buf, u32 count);
typedef int (*core_ddi_vfs_sync)(int fp);
typedef int (*core_ddi_vfs_deletefile)(const char *pathname);
typedef int (*core_ddi_vfs_renamefile)(const char *oldpath,const char *newpath);
typedef void (*core_ddi_vfs_free_space)(u32 *free_space);
typedef void (*core_ddi_vfs_factory_initial)(void);
typedef s32 (*core_ddi_flash_read)(u32 addrsta, u8* rbuf, u32 rlen);
typedef s32 (*core_ddi_flash_write)(u32 addrsta, u8* wbuf, u32 wlen);

#endif






