

#ifndef _FS_SYSTEM_H_
#define _FS_SYSTEM_H_


extern DQFILE sys_vfs_open(const s8 *pathname,const s8 *mode,int *result);
extern int sys_vfs_close(const char *pathname,DQFILE fp);
extern int sys_vfs_read(const s8 *pathname, u8 *readdata, u32 readlen, u32 readaddr,DQFILE fp);
extern int sys_vfs_write(const s8 *pathname, u8 *writedata, u32 writelen, u32 writeaddr,DQFILE fp);
extern int sys_vfs_seek(DQFILE File, int offset, int whence);
extern int sys_vfs_tell(const char * pathname,DQFILE fp);
extern int sys_vfs_deletefile(const char *pathname);
extern int sys_vfs_renamefile(const char *oldpath,const char *newpath);


extern int vfs_open(const char *pathname,const unsigned char * mode);
extern int vfs_close(int fp);
extern int vfs_seek(int fp, int offset, int whence);
extern int vfs_tell(int fp);
extern int vfs_read(int fp, void *buf, u32 count);
extern int vfs_write(int fp, void *buf, u32 count);
extern int vfs_sync(int fp);
extern int vfs_deletefile(const char *pathname);
extern int vfs_renamefile(const char *oldpath,const char *newpath);
extern void vfs_free_space(u32 *free_space);
extern void vfs_factory_initial(void);
extern void sys_fs_deinit(void);


#endif

