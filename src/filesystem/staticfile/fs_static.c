
#include "fs.h"

#define MTD2PATHLEN 6
/*
  名称               起始地址          长度
*/
//mtd1 上的文件数
//20190530:修改boot.bin->apextboot.bin core.bin->  rom.bin   app.bin->apapp.bin    ——pengxuebin
const struct _sFileSysMtd mtd2File[] = 
{
    //系统文件,内部flash
    {"apextboot.bin",BOOTBINADDR, BOOT_SIZE_MAX}, // boot 最多 24K
    {"machineparam",MACHINEPARAM_START_ADDR, MACHINEPARAM_SIZE_MAX},  
    {"rom.bin",COREBINADDR, CORE_SIZE_MAX},                                     // 168K core
    {"apapp.bin",APPBINADDR, APP_SIZE_MAX},                                       // 312K       app

    {"machineparambak",MACHINEPARAMBAK_START_ADDR, MACHINEPARAMBAK_SIZE_MAX},
    {"00.key", KEY_START_ADDR, KEY_SIZE_MAX},                        //64K+128K
    
    {"gbk2424.bin",SPIFLASH_ADDRSTA+GBK2424_START_ADDR,GBK2424_SIZE_MAX},                                          //    768K
    {"gbk1616.bin",SPIFLASH_ADDRSTA+GBK1616_START_ADDR,GBK1616_SIZE_MAX},                                          //    768K
    {"tts.bin",SPIFLASH_ADDRSTA+TTS_START_ADDR,TTS_SIZE_MAX}, 
   #if(APUPDATE_SPACE_EXIST==1)
    {"apupdate",SPIFLASH_ADDRSTA+APUPDATE_START_ADDR, APUPDATE_SIZE_MAX},                           //         
   #endif
    {"lfs",SPIFLASH_ADDRSTA+LFS_START_ADDR,LFS_SIZE_MAX},
    {NULL,0,-1}	
}; //最多10个资源文件，文件指针,指向第一个文件，也是文件列表的头


s32 static_vfs_check(const s8 *pathname)
{
	s8 filename[32];
	s32 i = 0;
	
	strcpy(filename,&pathname[MTD2PATHLEN]);
	filename[31] = 0;

    while(mtd2File[i].addr != 0)//0xFFFF)
    {
    	if(strcmp(filename,(s8 *)mtd2File[i].name) == 0)
    	{
			return i;
    	}
		i++;
    }

	return -1;
	
}

s32 static_vfs_checkareaexceeded(const s8 *pathname,u16 addr,u16 len)
{
	s32 filepos;

	filepos = static_vfs_check(pathname);
    if(filepos < 0)
    {
    	return VFS_RESULT_FILENOTEXIST;
    }
	
	if((addr+len)<=mtd2File[filepos].len)
	{
		return filepos;
	}
	else
	{
		return VFS_RESULT_FILEEND;
	}
	
	
}

DQFILE static_vfs_open(const s8 *pathname,const s8 *mode,int *result)
{
	*result = VFS_RESULT_FILENOTEXIST;
	
	if(static_vfs_check(pathname) >= 0)
	{
		*result = VFS_RESULT_SUCCESS;
	}
	
	
	return (-1);  //返回-1的时候，表示不是linux标准文件系统格式，需要查看result的值来判断操作结果
	
}

//资源文件，关一定成功
int static_vfs_close(const char *pathname,DQFILE fp)//以文件名称来close,防止出错
{
	return 0;
}

int static_vfs_read(const s8 *pathname, u8 *readdata, u32 readlen, u32 readaddr,DQFILE fp)
{
    
    s32 filepos;
    filepos = static_vfs_checkareaexceeded(pathname,readaddr,readlen);
	if(filepos < 0)
	{
		return filepos;
	}
//dev_debug_printf("%s:addr=%08X\r\n", __FUNCTION__, mtd2File[filepos].addr + readaddr);	
	filepos = dev_flash_read(mtd2File[filepos].addr + readaddr,readdata,readlen);
	
	return filepos;
	
}

//返回写成功的长度
int static_vfs_write(const s8 *pathname, u8 *writedata, u32 writelen, u32 writeaddr,DQFILE fp)
{
	s32 filepos;
    filepos = static_vfs_checkareaexceeded(pathname,writeaddr,writelen);
	if(filepos < 0)
	{
		return filepos;
	}
	
//dev_debug_printf("%s:addr=%08X\r\n", __FUNCTION__, mtd2File[filepos].addr + writeaddr);	
	filepos = dev_flash_write(mtd2File[filepos].addr + writeaddr,writedata,writelen);
	if(filepos != 0)
	{
		return VFS_RESULT_FILEERR;
	}
	return writelen;
	
}

//静态文件系统，不实现这个功能
int static_vfs_seek(DQFILE File, int offset, int whence)
{
	return 0;
}


//静态文件系统，返回文件最大长度
int static_vfs_tell(const char * pathname,DQFILE fp)
{
	s32 filepos;
    filepos = static_vfs_check(pathname);
	if(filepos < 0)
	{
		return filepos;
	}
	
	return mtd2File[filepos].len;
	
}



int static_vfs_getarea(const char * pathname,DQFILE fp,u32 *length)
{
	s32 filepos;

	*length = 0;
    filepos = static_vfs_check(pathname);
	if(filepos < 0)
	{
		return filepos;
	}

	*length = mtd2File[filepos].len;
	return mtd2File[filepos].addr;
	
}



