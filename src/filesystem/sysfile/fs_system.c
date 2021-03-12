
#include "fs.h"


#define  LFSPATH  "mtd0/"
#define  LFSDIR   "mtd0"


static lfs_t g_lfs_t;
static struct lfs_config	g_lfs_cfg_t;
static lfs_dir_t g_dir_t = {0};


#define MAXOPENFILENUM    3
#define FILEPATHLEN     50
typedef struct _LFS_FILE
{
	lfs_file_t g_lfs_file[MAXOPENFILENUM];
	u8 g_lfs_filestatus[MAXOPENFILENUM];
	u8 g_lfs_filepath[MAXOPENFILENUM][FILEPATHLEN];
}LFS_FILE;

static LFS_FILE glfs_fileinfo;

//uint8_t lfs_read_buf[256];
//uint8_t lfs_prog_buf[256];
//uint8_t lfs_lookahead_buf[256];
//uint8_t lfs_file_buf[256];




int block_device_read(const struct lfs_config *c,lfs_block_t block,lfs_off_t off,void* buffer,lfs_size_t size);
int block_device_prog(const struct lfs_config *c,lfs_block_t block,lfs_off_t off,void* buffer,lfs_size_t size);
int block_device_erase(const struct lfs_config *c,lfs_block_t block);
int block_device_sync(const struct lfs_config *c);


void sys_fs_deinit(void)
{
    //lfs_dir_close(&g_lfs_t, &g_dir_t);
	lfs_unmount(&g_lfs_t);
}



void sys_fs_config(void)
{

    memset(&g_lfs_cfg_t,0,sizeof(struct lfs_config));
	g_lfs_cfg_t.read  = block_device_read,
	g_lfs_cfg_t.prog  = block_device_prog,
	g_lfs_cfg_t.erase = block_device_erase,
	g_lfs_cfg_t.sync  = block_device_sync,
	
	
	g_lfs_cfg_t.read_size = 256,
	g_lfs_cfg_t.prog_size = 256,
	g_lfs_cfg_t.block_size = 4096,
	g_lfs_cfg_t.block_count = (LFS_SIZE_MAX/g_lfs_cfg_t.block_size);//((2*1024*1024)/g_lfs_cfg_t.block_size),//2M空间，要配合底层偏移来处理
	g_lfs_cfg_t.lookahead = 256,
	
	
	g_lfs_cfg_t.read_buffer = 		           NULL;  //动态申请可以处理多个文件，固定则不行
	g_lfs_cfg_t.prog_buffer =            		NULL;
	g_lfs_cfg_t.lookahead_buffer = 	                    NULL;
	g_lfs_cfg_t.file_buffer = 		           NULL;	
    
    
	memset(&glfs_fileinfo,0,sizeof(LFS_FILE));

	
}


s32 sys_fs_init(void)
{
	s32 err;
	
	
	sys_fs_config();
	memset(&g_lfs_t,0,sizeof(lfs_t));
    err = lfs_mount(&g_lfs_t, &g_lfs_cfg_t);
    if (err) {  //sxl?2018
    //if(1)
    //{
    	//FS_DEBUG("lfs need formart flash to mount filesystem LINE: %d\n", __LINE__);
        lfs_format(&g_lfs_t, &g_lfs_cfg_t);
        err = lfs_mount(&g_lfs_t, &g_lfs_cfg_t);
        //return err;
    }	
    #if 1
	if(err == 0)
	{
		err = lfs_dir_open(&g_lfs_t, &g_dir_t, LFSDIR);
		LFS_DEBUG("lfs dir open error:%d  LINE %d\n",err, __LINE__);
		if(err == LFS_ERR_NOENT)
		{
			lfs_mkdir(&g_lfs_t ,LFSDIR);
			LFS_DEBUG("lfs makedir LINE %d\n", __LINE__);
			err = lfs_dir_open(&g_lfs_t, &g_dir_t, LFSDIR);
			if(err == 0)
			{
				lfs_dir_close(&g_lfs_t, &g_dir_t);
			}
		}
        #if 1
		if(err = 0)
		{
			lfs_dir_close(&g_lfs_t, &g_dir_t);
		}
		#endif
		
	}
    #endif
	return err;
	
}


int sys_vfs_getlfs_filep(const s8 *pathname)
{
	
	u8 i;
	int notusedfileno = -1;
	
	
	for(i = 0;i < MAXOPENFILENUM;i++)
	{
		if(strcmp(pathname,&glfs_fileinfo.g_lfs_filepath[i][0]) == 0)
		{
			if(glfs_fileinfo.g_lfs_filestatus[i])  //文件已打开 
			{
				sys_vfs_close(pathname,i+1);//以文件名称来close,防止出错
				
			}
			return i+1;  //使用原来文件名的位置
		}
		else
		{
		    //当前文件位置未定义
			if(glfs_fileinfo.g_lfs_filestatus[i] == 0)
			{
				if(notusedfileno < 0)
				{
					notusedfileno = i+1;
				}
			}
			
			
		}
		
	}
	
	return notusedfileno;

	
}


DQFILE sys_vfs_open(const s8 *pathname,const s8 *mode,int *result)
{
    int flags;
	int ret;
	int filepindex;
	int pathnameoffset = 0;
	s8 filedirpath[128];

	if(memcmp(pathname,"/mtd0/",6) == 0)
	{
		pathnameoffset = 6;
	}


	//FS_DEBUG("\r\n lfs open file:%s %s\r\n",pathname,mode);
	
    //先判断 mode参数
    if(strcmp(mode,"r") != 0&&strcmp(mode,"w") != 0&&strcmp(mode,"a") != 0&&strcmp(mode,"r+") != 0
		&&strcmp(mode,"w+") != 0&&strcmp(mode,"a+") != 0)
    {
         //FS_DEBUG("\r\n lfs open file mode err\r\n");
    	*result = VFS_RESULT_PARAMERR; 
		return 0;
    }

	
    if(strcmp(mode,"r") == 0)
    {
    	flags = LFS_O_RDONLY;
    }
	else if(strcmp(mode,"w") == 0)
	{
		flags = LFS_O_WRONLY|LFS_O_CREAT|LFS_O_TRUNC;
	}
	else if(strcmp(mode,"a") == 0)
	{
		flags = LFS_O_APPEND|LFS_O_CREAT;
	}
	else if(strcmp(mode,"r+") == 0)
	{
		flags = LFS_O_RDWR|LFS_O_CREAT;
	}
	else if(strcmp(mode,"w+") == 0)
	{
		flags = LFS_O_RDWR|LFS_O_CREAT|LFS_O_TRUNC;
	}
	else if(strcmp(mode,"a+") == 0)
	{
		flags = LFS_O_RDWR|LFS_O_CREAT|LFS_O_APPEND;
	}
	else
	{
		*result = VFS_RESULT_PARAMERR; 
		return 0;
	}


	filepindex = sys_vfs_getlfs_filep(pathname);
	//dev_debug_printf("\r\nfilepindex = %d\r\n",filepindex);
	if(filepindex <= 0)
	{
		*result = VFS_RESULT_PARAMERR; 
		return 0;
	}
    
	
	
	memset(filedirpath,0,sizeof(filedirpath));
	strcpy(filedirpath,LFSPATH);
    strcat(filedirpath,&pathname[pathnameoffset]);
	ret = lfs_file_open(&g_lfs_t, &glfs_fileinfo.g_lfs_file[filepindex - 1],filedirpath, flags);
	//dev_debug_printf("\r\n lfs open file:%s %d %d %d\r\n",pathname,filepindex,flags,ret);
	if(ret == LFS_ERR_OK)
	{
		
		glfs_fileinfo.g_lfs_filestatus[filepindex - 1] = 1;
		strcpy(&glfs_fileinfo.g_lfs_filepath[filepindex - 1][0],pathname);
		*result = VFS_RESULT_SUCCESS;
		//dev_debug_printf("\r\n file open success\r\n");
		return filepindex;
		
	}
	else
	{
		memset(&glfs_fileinfo.g_lfs_file[filepindex - 1],0,sizeof(lfs_file_t));
		glfs_fileinfo.g_lfs_filestatus[filepindex - 1] = 0;
		memset(&glfs_fileinfo.g_lfs_filepath[filepindex - 1][0],0,FILEPATHLEN);
		
		*result = VFS_RESULT_PARAMERR; 
		//dev_debug_printf("\r\n file open failed\r\n");
		return 0;
		
	}
	
	
}



int sys_vfs_read(const s8 *pathname, u8 *readdata, u32 readlen, u32 readaddr,DQFILE fp)
{
    s32 ret;
	s32 curreadlen = 0;
	//s32 read_len;
	
	
	//FS_DEBUG("\r\n lfs read file:%s %d %d\r\n",pathname,fp,glfs_fileinfo.g_lfs_filestatus[fp-1]);
    if(fp > 0)
    {
    	if(glfs_fileinfo.g_lfs_filestatus[fp-1])
    	{
    		
    	    ret = lfs_file_seek(&g_lfs_t, &glfs_fileinfo.g_lfs_file[fp - 1],readaddr,LFS_SEEK_SET);
			//FS_DEBUG("\r\n lfs read file seek:%d\r\n",ret);
			if(ret >= 0)
			{
    			ret = lfs_file_read(&g_lfs_t, &glfs_fileinfo.g_lfs_file[fp - 1],readdata,readlen);
				//FS_DEBUG("\r\n lfs read file ret:%d\r\n",ret);
				curreadlen = ret;
			}
			
    	}
    }
	
	return curreadlen;
	
}



int sys_vfs_seek(DQFILE fp, int offset, int whence)
{
	s32 ret = -1;
	
	//FS_DEBUG("\r\n lfs write file:%d %d %d\r\n",fp,offset,whence);
	if(fp > 0)
	{
		if(glfs_fileinfo.g_lfs_filestatus[fp-1])
    	{
    		ret = lfs_file_seek(&g_lfs_t, &glfs_fileinfo.g_lfs_file[fp - 1],offset,whence);
		}
	}

	//dev_debug_printf("\r\nvfs_seek = %d\r\n",ret);
	return ret;
	
}



int sys_vfs_write(const s8 *pathname, u8 *writedata, u32 writelen, u32 writeaddr,DQFILE fp)
{
	
	s32 ret;
	s32 curreadlen = 0;
	//FS_DEBUG("\r\n lfs write file:%s %d %d\r\n",pathname,fp,glfs_fileinfo.g_lfs_filestatus[fp-1]);
	if(fp > 0)
	{
		if(glfs_fileinfo.g_lfs_filestatus[fp-1])
    	{
    		
    	    ret = lfs_file_seek(&g_lfs_t, &glfs_fileinfo.g_lfs_file[fp - 1],writeaddr,LFS_SEEK_SET);
			//dev_debug_printf("\r\n lfs write file seek:%d\r\n",ret);
			if(ret >= 0)
			{
    			ret = lfs_file_write(&g_lfs_t, &glfs_fileinfo.g_lfs_file[fp - 1],writedata,writelen);
				//dev_debug_printf("\r\n lfs write file ret:%d\r\n",ret);
				curreadlen = ret;
			}
			
    	}
	}
	
	
	return curreadlen;

	
}



int sys_vfs_tell(const char * pathname,DQFILE fp)
{
    s32 filelen = 0;
	s32 ret;
	
	
	//FS_DEBUG("\r\n lfs tell file:%s %d\r\n",pathname,fp);
	if(fp > 0)
	{
		if(glfs_fileinfo.g_lfs_filestatus[fp-1])
    	{
    		ret = lfs_file_seek(&g_lfs_t, &glfs_fileinfo.g_lfs_file[fp - 1],0,LFS_SEEK_END);
			//FS_DEBUG("\r\n lfs tell file ret:%d\r\n",ret);
			if(ret >= 0)
			{
				filelen = ret;
			}
			
		}
		
	}
	
	
	return filelen;
	
}


int sys_vfs_deletefile(const char *pathname)
{
    int pathnameoffset = 0;
	s8 filedirpath[128];

	if(memcmp(pathname,"/mtd0/",6) == 0)
	{
		pathnameoffset = 6;
	}


	memset(filedirpath,0,sizeof(filedirpath));
	strcpy(filedirpath,LFSPATH);
    strcat(filedirpath,&pathname[pathnameoffset]);
	
	return lfs_remove(&g_lfs_t,filedirpath);

}


int sys_vfs_renamefile(const char *oldpath,const char *newpath)
{
    int oldpathnameoffset = 0;
	int newpathnameoffset = 0;
	s8 srcfiledirpath[128],dstfiledirpath[128];
	
	
	if(memcmp(oldpath,"/mtd0/",6) == 0)
	{
		oldpathnameoffset = 6;
	}
	if(memcmp(newpath,"/mtd0/",6) == 0)
	{
		newpathnameoffset = 6;
	}


	memset(srcfiledirpath,0,sizeof(srcfiledirpath));
	strcpy(srcfiledirpath,LFSPATH);
    strcat(srcfiledirpath,&oldpath[oldpathnameoffset]);

	memset(dstfiledirpath,0,sizeof(dstfiledirpath));
	strcpy(dstfiledirpath,LFSPATH);
    strcat(dstfiledirpath,&newpath[newpathnameoffset]);
	
	return lfs_rename(&g_lfs_t,srcfiledirpath,dstfiledirpath);
	
	
}


int sys_vfs_close(const char *pathname,DQFILE fp)
{
	if(fp > 0 &&fp <= MAXOPENFILENUM)
	{
		memset(&glfs_fileinfo.g_lfs_filepath[fp-1][0],0,FILEPATHLEN);
		glfs_fileinfo.g_lfs_filestatus[fp-1] = 0;
		//lfs_file_flush(&g_lfs_t,&glfs_fileinfo.g_lfs_file[fp-1]);
		lfs_file_close(&g_lfs_t,&glfs_fileinfo.g_lfs_file[fp-1]);
		memset((u8 *)&glfs_fileinfo.g_lfs_file[fp-1],0,sizeof(lfs_file_t));
	}
    
	return 0;
	
}



int vfs_open(const char *pathname,const unsigned char * mode)
{
	
	DQFILE fp;
	int result;
    
	fp = sys_vfs_open(pathname,mode,&result);
	if(fp <= 0)
	{
		return -1;
	}
    else
    {
    	return fp;
    }
	 
}



int vfs_close(int fp)
{
	if(fp > 0 &&fp <= MAXOPENFILENUM)
	{
		memset(&glfs_fileinfo.g_lfs_filepath[fp-1][0],0,FILEPATHLEN);
		glfs_fileinfo.g_lfs_filestatus[fp-1] = 0;
		//lfs_file_flush(&g_lfs_t,&glfs_fileinfo.g_lfs_file[fp-1]);
		lfs_file_close(&g_lfs_t,&glfs_fileinfo.g_lfs_file[fp-1]);
		memset((u8 *)&glfs_fileinfo.g_lfs_file[fp-1],0,sizeof(lfs_file_t));
	}
    
	return 0;
	
}



int vfs_seek(int fp, int offset, int whence)
{
	s32 ret = -1;
	
	//FS_DEBUG("\r\n lfs write file:%d %d %d\r\n",fp,offset,whence);
	if(fp > 0)
	{
		if(glfs_fileinfo.g_lfs_filestatus[fp-1])
    	{
    		ret = lfs_file_seek(&g_lfs_t, &glfs_fileinfo.g_lfs_file[fp - 1],offset,whence);
		}
	}
	
	return ret;
	
}



int vfs_tell(int fp)
{
	s32 ret = 0;
	
	
	//FS_DEBUG("\r\n lfs tell file:%d\r\n",fp);
	if(fp > 0)
	{
		if(glfs_fileinfo.g_lfs_filestatus[fp-1])
    	{
    		ret = lfs_file_tell(&g_lfs_t, &glfs_fileinfo.g_lfs_file[fp - 1]);
			//FS_DEBUG("\r\n lfs tell file ret:%d\r\n",ret);
			
		}
		
	}
	
	
	return ret;
	
}




int vfs_read(int fp, void *buf, u32 count)
{
	s32 ret;
	s32 curreadlen = 0;
	//s32 read_len;
	
	
	//FS_DEBUG("\r\n lfs read file:%d %d\r\n",fp,glfs_fileinfo.g_lfs_filestatus[fp-1]);
    if(fp > 0)
    {
    	if(glfs_fileinfo.g_lfs_filestatus[fp-1])
    	{
    		
			ret = lfs_file_read(&g_lfs_t, &glfs_fileinfo.g_lfs_file[fp - 1],buf,count);
			//FS_DEBUG("\r\n lfs read file ret:%d\r\n",ret);
			if(ret >= 0)
			{
				curreadlen = ret;
			}
			
    	}
    }
	
	return curreadlen;
	
}




int vfs_write(int fp, void *buf, u32 count)
{
	int ret = 0;
	
	
	//FS_DEBUG("\r\n lfs read file:%d %d\r\n",fp,glfs_fileinfo.g_lfs_filestatus[fp-1]);
    if(fp > 0)
    {
    	if(glfs_fileinfo.g_lfs_filestatus[fp-1])
    	{
    		
			ret = lfs_file_write(&g_lfs_t, &glfs_fileinfo.g_lfs_file[fp - 1],buf,count);
			//FS_DEBUG("\r\n lfs read file ret:%d\r\n",ret);
			
    	}
    }
	
	return ret;
	
}



int vfs_sync(int fp)
{
    int ret = 0;
	
	if(fp > 0)
    {
    	if(glfs_fileinfo.g_lfs_filestatus[fp-1])
    	{
    		
			ret = lfs_file_sync(&g_lfs_t, &glfs_fileinfo.g_lfs_file[fp - 1]);
			//FS_DEBUG("\r\n lfs read file ret:%d\r\n",ret);
			
    	}
    }

	return ret;
}



int vfs_deletefile(const char *pathname)
{
	return sys_vfs_deletefile(pathname);
}



int vfs_renamefile(const char *oldpath,const char *newpath)
{
	
return sys_vfs_renamefile(oldpath,newpath);
}



static int lfs_fuse_statfs_count(void* p,lfs_block_t block)
{
	*((lfs_size_t *)p) += 1;
	return 0;
	
}


void vfs_free_space(size_t *free_space)
{
	int err;
	lfs_size_t use_space = 0;


	*free_space = 0;
	//err = lfs_dir_open(&g_lfs_t, &g_dir_t, "E:");
	//if(err == 0)
	//{
		err = lfs_traverse(&g_lfs_t,lfs_fuse_statfs_count,&use_space);
		if(err)
		{
		    //lfs_dir_close(&g_lfs_t, &g_dir_t);
			return ;
		}
		*free_space = (g_lfs_cfg_t.block_count - use_space)*FileSysBlock_BUFSIZE;
		//lfs_dir_close(&g_lfs_t, &g_dir_t);
	//}
	
}



void vfs_factory_initial(void)
{
	s32 err;

	
    sys_fs_config();
	memset(&g_lfs_t,0,sizeof(lfs_t));
	memset(&g_dir_t,0,sizeof(lfs_dir_t));
	
	lfs_format(&g_lfs_t, &g_lfs_cfg_t);
	err = lfs_mount(&g_lfs_t, &g_lfs_cfg_t);
	#if 1
	if(err == 0)
	{
		err = lfs_dir_open(&g_lfs_t, &g_dir_t, LFSDIR);
		LFS_DEBUG("lfs dir open error:%d  LINE %d\n",err, __LINE__);
		if(err == LFS_ERR_NOENT)
		{
			lfs_mkdir(&g_lfs_t ,LFSDIR);
			LFS_DEBUG("lfs makedir LINE %d\n", __LINE__);
			err = lfs_dir_open(&g_lfs_t, &g_dir_t, LFSDIR);
			
		}
		if(err == 0)
		{
			lfs_dir_close(&g_lfs_t, &g_dir_t);
		}
        
	}
	#endif
}

