

#ifndef _FS_STATIC_H_
#define _FS_STATIC_H_

#define BOOTBINADDR BOOT_START_ADDR//0x1000
#define COREBINADDR CORE_START_ADDR//0x18000
#define APPBINADDR  APP_START_ADDR//   (COREBINADDR+0x30000)


#define FS_STATICDATA_STAADDR   0x70000                       //后面64K数据先保留使用




extern DQFILE static_vfs_open(const s8 *pathname,const s8 *mode,int *result);
extern int static_vfs_close(const char *pathname,DQFILE fp);
extern int static_vfs_read(const s8 *pathname, u8 *readdata, u32 readlen, u32 readaddr,DQFILE fp);
extern int static_vfs_write(const s8 *pathname, u8 *writedata, u32 writelen, u32 writeaddr,DQFILE fp);
extern int static_vfs_seek(DQFILE File, int offset, int whence);
extern int static_vfs_tell(const char * pathname,DQFILE fp);
extern int static_vfs_getarea(const char * pathname,DQFILE fp,u32 *length);


#endif


