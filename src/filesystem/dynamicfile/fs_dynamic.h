

#ifndef _FS_DYNAMIC_H_
#define _FS_DYNAMIC_H_


#define MTD1PATHLEN 6

#define MAXMTD1FILENUM 8
struct _fs_mtd1_str
{
	u8 num;         //mtd1 �ϵ��ļ���
	struct _sFileSysMtd File[MAXMTD1FILENUM]; //���10����Դ�ļ����ļ�ָ��,ָ���һ���ļ���Ҳ���ļ��б��ͷ
};



extern s32 dynamic_vfs_initial(void);
extern DQFILE dynamic_vfs_open(const s8 *pathname,const s8 *mode,int *result);
extern int dynamic_vfs_close(const char *pathname,DQFILE fp);
extern int dynamic_vfs_read(const s8 *pathname, u8 *readdata, u32 readlen, u32 readaddr,DQFILE fp);
extern int dynamic_vfs_write(const s8 *pathname, u8 *writedata, u32 writelen, u32 writeaddr,DQFILE fp);
extern int dynamic_vfs_seek(DQFILE File, int offset, int whence);
extern int dynamic_vfs_tell(const char * pathname,DQFILE fp);
extern s32 dynamic_vfs_get_fileinfo(const char *pathname, u32 *addr, u32 *len);



#endif


