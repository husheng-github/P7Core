
#include "fs.h"
static u8 g_fs_type=0;


const VFS_VIRTUAL_FUNCITON staticvirtualfunc = 
{
	static_vfs_open,
	static_vfs_close,
	static_vfs_read,
	static_vfs_write,
	static_vfs_seek,
	static_vfs_tell
};


#if 0//(EXFS_EXIST == 1)
const VFS_VIRTUAL_FUNCITON dynamicvirtualfunc = 
{
	dynamic_vfs_open,
	dynamic_vfs_close,
	dynamic_vfs_read,
	dynamic_vfs_write,
	dynamic_vfs_seek,
	dynamic_vfs_tell
};

#endif

//#ifndef TRENDIT_BOOT
const VFS_VIRTUAL_FUNCITON sysvirtualfunc = 
{
	sys_vfs_open,
	sys_vfs_close,
	sys_vfs_read,
	sys_vfs_write,
	sys_vfs_seek,
	sys_vfs_tell
	
};
//#endif



void DQ_vfs_getvirtualfunc(const s8 *pathname,VFS_VIRTUAL_FUNCITON **vfsvirtualfunc)
{
    if(pathname == NULL)
    {
        *vfsvirtualfunc = NULL;
    	return;
    }
#if 0//(EXFS_EXIST == 1)
    *vfsvirtualfunc = NULL;
	
	if(memcmp(pathname,"/mtd2/",6) == 0)
	{
		*vfsvirtualfunc = (VFS_VIRTUAL_FUNCITON *)&staticvirtualfunc;	
        g_fs_type = FS_TYPE_STATIC;
	}
	else if(memcmp(pathname,"/mtd1/",6) == 0)
	{
		*vfsvirtualfunc = (VFS_VIRTUAL_FUNCITON *)&dynamicvirtualfunc;	
        g_fs_type = FS_TYPE_DYNAMIC;
	}

	//#if (defined(TRENDIT_CORE) ||defined(FSL_RTOS_UCOSIII))
    else
    {
    	*vfsvirtualfunc = (VFS_VIRTUAL_FUNCITON *)&sysvirtualfunc;	
        g_fs_type = FS_TYPE_DYNAMIC;
    }
	//#endif
#else
    *vfsvirtualfunc = NULL;
	
	if(memcmp(pathname,"/mtd2/",6) == 0)
	{
	    *vfsvirtualfunc = (VFS_VIRTUAL_FUNCITON *)&staticvirtualfunc;	
	    g_fs_type = FS_TYPE_STATIC;
	}
//    #ifdef TRENDIT_CORE
    else if(memcmp(pathname,"/mtd0/",6) == 0)
    {
    	*vfsvirtualfunc = (VFS_VIRTUAL_FUNCITON *)&sysvirtualfunc;	
        g_fs_type = FS_TYPE_SYSTEM;
    }
//    #endif
#endif	
}



DQFILE DQ_vfs_open(const s8 *pathname,const s8 *mode,int *result)
{
	
	VFS_VIRTUAL_FUNCITON *vfsvirtualfunc =NULL;
	
	DQ_vfs_getvirtualfunc(pathname,&vfsvirtualfunc);
	if(vfsvirtualfunc == NULL)
	{
		*result = VFS_RESULT_FILEERR;
		return -1;
	}

	return vfsvirtualfunc->virtual_vfs_open(pathname,mode,result);  //返回-1的时候，表示不是linux标准文件系统格式，需要查看result的值来判断操作结果
	
}



int DQ_vfs_close(const char *pathname,DQFILE fp)//以文件名称来close,防止出错
{
	VFS_VIRTUAL_FUNCITON *vfsvirtualfunc = NULL;
	DQ_vfs_getvirtualfunc(pathname,&vfsvirtualfunc);
	if(vfsvirtualfunc == NULL)
	{
		return -1;
	}

	return vfsvirtualfunc->virtual_vfs_close(pathname,fp);
	
}


int DQ_vfs_read(const s8 *pathname, u8 *readdata, u32 readlen, u32 readaddr,DQFILE fp)
{
	VFS_VIRTUAL_FUNCITON *vfsvirtualfunc = NULL;
	DQ_vfs_getvirtualfunc(pathname,&vfsvirtualfunc);
	if(vfsvirtualfunc == NULL)
	{
		return -1;
	}
	
	return vfsvirtualfunc->virtual_vfs_read(pathname,readdata,readlen,readaddr,fp);
}


int DQ_vfs_write(const s8 *pathname, u8 *writedata, u32 writelen, u32 writeaddr,DQFILE fp)
{
	VFS_VIRTUAL_FUNCITON *vfsvirtualfunc = NULL;
	DQ_vfs_getvirtualfunc(pathname,&vfsvirtualfunc);
	if(vfsvirtualfunc == NULL)
	{
		return -1;
	}
	
	return vfsvirtualfunc->virtual_vfs_write(pathname,writedata,writelen,writeaddr,fp);
	
}


//现在不支持seek指令
int DQ_vfs_seek(DQFILE File, int offset, int whence)
{
    //VFS_VIRTUAL_FUNCITON *vfsvirtualfunc;
	//DQ_vfs_getvirtualfunc(pathname,vfsvirtualfunc);
	
	//return vfsvirtualfunc->virtual_vfs_seek(File,offset,whence);
	return 0;
}

int DQ_vfs_tell(const char * pathname,DQFILE fp)
{
    VFS_VIRTUAL_FUNCITON *vfsvirtualfunc = NULL;
	DQ_vfs_getvirtualfunc(pathname,&vfsvirtualfunc);
	if(vfsvirtualfunc == NULL)
	{
		return 0;
	}
	
	return vfsvirtualfunc->virtual_vfs_tell(pathname,fp);
}



s32 fs_create_file(const char *filename, s32 len,u8 initialvalue)
{
	//4k,4k的写
    s32 openresult;
	u8 *writedata;
	s32 writebuflen = 4096;
    s32 ret = VFS_RESULT_FILEERR;
	u32 writeaddr = 0,writelen;
	DQFILE fp;

	#ifdef FS_DEBUG
    VfsDebug("\r\ncreate file  111 %d %s\r\n",len,filename);
	#endif
	fp = DQ_vfs_open(filename,"w+",&openresult);
	if(openresult == VFS_RESULT_SUCCESS)
	{
	    #ifdef FS_DEBUG
        VfsDebug("\r\ncreate file  222 %d\r\n",len);
		#endif
		if(len)
		{
			writedata = (u8 *)k_malloc(writebuflen);
			if(writedata != NULL)
			{
			    memset(writedata,initialvalue,writebuflen);
				ret = VFS_RESULT_SUCCESS;
			    for(writeaddr = 0;writeaddr < len;writeaddr += writelen)
			    {
			        if((writeaddr+writebuflen) < len)
			        {
			        	writelen = writebuflen;
			        }
					else
					{
						writelen = len - writeaddr;
					}
					
					openresult = DQ_vfs_write(filename,writedata,writelen,writeaddr,fp);
					#ifdef FS_DEBUG
			        VfsDebug("\r\ncreate file  333 %d %d %08x %08x\r\n",openresult,writelen,writeaddr,len);
					#endif
					if(openresult != writelen)
					{
					    #ifdef FS_DEBUG
						VfsDebug("\r\n write error:%d %d\r\n",openresult,writelen);
						#endif
						ret = VFS_RESULT_FILEERR;
						break;
					}
					
			    }

				k_free(writedata);
				
			}
			
		}
		DQ_vfs_close(filename,fp);
		//SysFile_Show_detai(); //sxl?
	}
    

	return ret;
}


//返回实际读取到的字节数
s32 fs_read_file(const char *filename,u8 *rbuf, s32 rlen, s32 startaddr)
{
	
	s32 openresult;
	s32 ret = VFS_RESULT_FILEERR;
	DQFILE fp;

	
	fp = DQ_vfs_open(filename,"r",&openresult);
	if(openresult == VFS_RESULT_SUCCESS)
	{
		
		DQ_vfs_seek(fp,startaddr,0);
		ret = DQ_vfs_read(filename,rbuf,rlen,startaddr,fp);
		
		DQ_vfs_close(filename,fp);
	}
	else
	{
		ret = VFS_RESULT_FILENOTEXIST;
	}
	
	return ret;
	
}


//会先删除文件后重新创建文件
s32 fs_write_file(const char *filename,u8 *writedata,s32 writedatalen)
{
	s32 openresult;
	s32 ret = VFS_RESULT_FILEERR;
	DQFILE fp;
	
//dev_debug_printf("%s(%d):name=%s,len=%d\r\n", __FUNCTION__, __LINE__, filename, writedatalen);	
	fp = DQ_vfs_open(filename,"w",&openresult);
   //dev_debug_printf("\r\nopen file ret = %d %s %d\r\n",openresult,filename,writedatalen);
	if(openresult == VFS_RESULT_SUCCESS)
	{
//dev_debug_printf("%s(%d):name=%s,len=%d\r\n", __FUNCTION__, __LINE__, filename, writedatalen);	
//dev_user_delay_ms(50);
		DQ_vfs_seek(fp,0,0);
		ret = DQ_vfs_write(filename,writedata,writedatalen,0,fp);
        //dev_debug_printf("\r\nfs write file ret = %d\r\n",ret);
		DQ_vfs_close(filename,fp);
	}
	else
	{
		ret = VFS_RESULT_FILENOTEXIST;
	}
	
	return ret;
	
}


s32 fs_insert_file(const char *filename,u8 *wbuf, s32 wlen, s32 startaddr)
{
	
	s32 openresult;
	s32 ret = VFS_RESULT_FILEERR;
	DQFILE fp;
	u32 filelen;

    #ifdef FS_DEBUG
    //VfsDebug("\r\n%s :%s\r\n",__FUNCTION__,filename);
	#endif
		
	u8 *writedata;
	s32 writebuflen = 4096;
	u32 writeaddr = 0,writelen;
	
	fp = DQ_vfs_open(filename,"r+",&openresult);
	#ifdef FS_DEBUG
    VfsDebug("\r\n%s :openresult = %d\r\n",__FUNCTION__,openresult);
	#endif
	if(openresult != VFS_RESULT_SUCCESS)//文件不存在，需要先创建文件
	{
		fs_create_file(filename,startaddr,0xFF);
		fp = DQ_vfs_open(filename,"r+",&openresult);
		#ifdef FS_DEBUG
	    VfsDebug("\r\n%s :openresult kkk = %d\r\n",__FUNCTION__,openresult);
		#endif
	
	}
	if(openresult == VFS_RESULT_SUCCESS)
	{
		filelen = DQ_vfs_tell(filename,fp);
		#ifdef FS_DEBUG
	    VfsDebug("\r\n%s :filelen = %d %d\r\n",__FUNCTION__,filelen,startaddr);
		#endif
        if(g_fs_type == FS_TYPE_STATIC)
        {
            if(filelen < (startaddr+wlen))
            {
                return VFS_RESULT_FILEERR;
            }
        }
		if(filelen < startaddr)   //文件长度小于
		{
			writedata = (u8 *)k_malloc(writebuflen);
			if(writedata != NULL)
			{
			    memset(writedata,0xFF,writebuflen);
				ret = VFS_RESULT_SUCCESS;
			    for(writeaddr = filelen;writeaddr < startaddr;writeaddr += writelen)
			    {
			        if((writeaddr+writebuflen) < startaddr)
			        {
			        	writelen = writebuflen;
			        }
					else
					{
						writelen = startaddr - writeaddr;
					}
					
					openresult = DQ_vfs_write(filename,writedata,writelen,writeaddr,fp);
					#ifdef FS_DEBUG
				    VfsDebug("\r\n%s :openresult111 = %d\r\n",__FUNCTION__,openresult);
					#endif
					if(openresult != writelen)
					{
						ret = VFS_RESULT_FILEERR;
						break;
					}
					
			    }

				k_free(writedata);
			}
		}
		else
		{
			ret = VFS_RESULT_SUCCESS;
		}

		#ifdef FS_DEBUG
	    VfsDebug("\r\n%s :ret555 = %d\r\n",__FUNCTION__,ret);
		#endif

		if(ret == VFS_RESULT_SUCCESS)
		{
			DQ_vfs_seek(fp,startaddr,0);
			ret = DQ_vfs_write(filename,wbuf,wlen,startaddr,fp);
			
			#ifdef FS_DEBUG
		    VfsDebug("\r\n%s :ret666 = %d\r\n",__FUNCTION__,ret);
			#endif
		
		}

		#ifdef FS_DEBUG
	    VfsDebug("\r\n%s :ret111 = %d\r\n",__FUNCTION__,ret);
		#endif
		
		DQ_vfs_close(filename,fp);
		
	}
	else
	{
		ret = VFS_RESULT_FILENOTEXIST;
	}

	#ifdef FS_DEBUG
    VfsDebug("\r\n%s :ret222 = %d\r\n",__FUNCTION__,ret);
	#endif
		
	return ret;
	
}


s32 fs_access_file(const s8 *filename)
{
	s32 openresult;
	s32 ret = VFS_RESULT_FILEERR;
	DQFILE fp;

	
	fp = DQ_vfs_open(filename,"r",&openresult);
	#ifdef FS_DEBUG
    VfsDebug("\r\n%s :%s %d\r\n",__FUNCTION__,filename,openresult);
	#endif
	if(openresult == VFS_RESULT_SUCCESS)
	{
		ret = DQ_vfs_tell(filename,fp);
		#ifdef FS_DEBUG
	    VfsDebug("\r\n%s :%s len = %d\r\n",__FUNCTION__,filename,ret);
		#endif
		DQ_vfs_close(filename,fp);
	}
	else
	{
		ret = 0;
	}
	
	return ret;
	
}


s32 fs_delete_file(const char *filename)
{
    #if 1//(defined(TRENDIT_CORE) ||defined(FSL_RTOS_UCOSIII))
	s32 openresult;
	//s32 ret = VFS_RESULT_FILEERR;
	DQFILE fp;
	
	fp = DQ_vfs_open(filename,"r",&openresult);
	if(openresult == VFS_RESULT_SUCCESS)
	{
		if(g_fs_type == FS_TYPE_SYSTEM)
		{
			DQ_vfs_close(filename,fp);
            sys_vfs_deletefile(filename);
		}
	}

	return VFS_RESULT_SUCCESS;
    #else
    return VFS_RESULT_FILEERR;
	#endif
	
}


s32 fs_rename_file(const char *srcfilename,const char *dstfilename)
{
    #if 1//(defined(TRENDIT_CORE) ||defined(FSL_RTOS_UCOSIII))
	s32 openresult;
	//s32 ret = VFS_RESULT_FILEERR;
	DQFILE fp;
	
	fp = DQ_vfs_open(srcfilename,"r",&openresult);
	if(openresult == VFS_RESULT_SUCCESS)
	{
		if(g_fs_type == FS_TYPE_SYSTEM)
		{
		    DQ_vfs_close(srcfilename,fp);
            openresult = sys_vfs_renamefile(srcfilename,dstfilename);
			if(openresult == 0)
			{
				return VFS_RESULT_SUCCESS;
			}
			
			
		}
	}
	return VFS_RESULT_FILEERR;

	#else
	return VFS_RESULT_FILEERR;
	#endif

	
}



void fs_system_init(void)
{
	
	
	#if 1//def TRENDIT_CORE
	sys_fs_init();
	#endif
    
	
}

