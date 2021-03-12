

#include "fs.h"
struct _fs_mtd1_str FsMtd1Pra;

//extern u32 ascii_2_u32(u8 *pb,u8 asciidatalen);
#define ascii_2_u32(x,y)   dev_maths_asc_to_u32(x, y, MODULUS_HEX)
/****************************************************************************
**Description:      在指针  pData 开始位置查找一个[...]字符串，并把中括号之间
                    的内容拷贝到seg中，len说明seg buf的长度，找到后返回字符串长度
                    同时返回已经查询的长度index
**Input parameters:
**Output parameters:
**
**Returned value:  0 找到，-1 没找到
**
**Created by: 
**--------------------------------------------------------------------------
**Modified by:
**Modified by:
****************************************************************************/
static s32 find_seg(const u8 *pData, u32 DataLen,u8 *seg, u32 *len ,u32 *ScanLen)
{
    u32 index = 0;
    u32 seg_buf_len = *len;
    u32 str_len = 0;
    s32 res = -1;

    while(index < DataLen)
    {
        if(*(pData+index) == '[')
            break;
        index++;
    }

    index++;

    while((index < DataLen) && (str_len < seg_buf_len))
    {
        if(*(pData+index) == ']')
        {
            res = 0;
            break;
        }

        *(seg+str_len) = *(pData+index);
        index++;
        str_len++;
    }

    if(res == 0)
        *len = str_len;
    else
        *len = 0;

    *ScanLen = index;
    return res;
}



//上电的时候读取动态配置文件,小于4K
s32 dynamic_vfs_initial(void)
{
	u8 *pdata;
	s32 ret;
	u16 index;
	u8 nameflag = 0,addrflag = 0;
	u32 tmpvalue;
	u8 data[32];
	u32 len,scan_len;
	
	memset(&FsMtd1Pra,0,sizeof(struct _fs_mtd1_str));
	
	pdata = (u8*)k_malloc(4096);
    if(pdata == NULL)
    {
    	return -1;
    }
	
	ret = DQ_vfs_read("/mtd2/vfsconfig",pdata,4096,0,0);
    if(ret != 4096)
    {
    	return -1;
    }
	//strcpy( pdata, "[name:M1][addr:0x70000][len:0x7C000][end]");   config文件格式
	index = 0;
    while((index+4) < 4096)
    {
        memset(data, 0, sizeof(data));
        len = sizeof(data);
        ret = find_seg(pdata + index, 4096-index, data, &len, &scan_len);
        if(ret == 0)
        {
            VfsDebug("找到一个字符串 %s\r\n", data);
            index += scan_len;
        }
        else
        {
            k_free(pdata);//查询完整个配置文件
            return -1;//不完整的配置文件
        }
		
        if(0 == memcmp(data, "name:", 5))
        {
            if(strlen((s8 *)&data[5]) < FILESYSMTDNAMELEN-1)
            {
                nameflag = 1;
				addrflag = 0;
	            strcpy((s8 *)FsMtd1Pra.File[FsMtd1Pra.num].name,(s8 *)&data[5]);
	            VfsDebug("找到名字:%s\r\n", FsMtd1Pra.File[FsMtd1Pra.num].name);
            }
        }
        else if(0 == memcmp(data, "addr:", 5))
        {
        	if(len > 7)
        	{
				tmpvalue = ascii_2_u32(data + 7, len-7);
				
	            addrflag = 1;
	            FsMtd1Pra.File[FsMtd1Pra.num].addr = tmpvalue;
	            VfsDebug("找到addr:%08x\r\n", FsMtd1Pra.File[FsMtd1Pra.num].addr);
        	}
            
        }
		else if(0 == memcmp(data, "len:", 4))
        {
            if(len > 7)
            {
				tmpvalue = ascii_2_u32(data + 6, len-6);
				
				FsMtd1Pra.File[FsMtd1Pra.num].len= tmpvalue;
				VfsDebug("找到len:%08x\r\n", FsMtd1Pra.File[FsMtd1Pra.num].len);

				if(addrflag&&nameflag)
				{
					FsMtd1Pra.num++;
					if(FsMtd1Pra.num >= MAXMTD1FILENUM)
				    {
				    	k_free(pdata);//查询完整个配置文件
				    	return 0;
				    }
				}
				addrflag = 0;
	            nameflag = 0;
            }
			
        }
        else if(0 == memcmp(data, "end", 3))
        {
            VfsDebug("找到配置文件结尾\r\n");
            k_free(pdata);//查询完整个配置文件
            return 0;//查找成功，退出
        }
       	
        
    }
	
	k_free(pdata);//查询完整个配置文件
	return 0;//查找成功，退出
     
}


s32 dynamic_vfs_check(const s8 *pathname)
{
	s8 filename[32];
	s32 i = 0;
	
	strcpy(filename,&pathname[MTD1PATHLEN]);
	filename[31] = 0;
    
    while(i < FsMtd1Pra.num)
    {
    	if(strcmp(filename,(s8 *)FsMtd1Pra.File[i].name) == 0)
    	{
			return i;
    	}
		i++;
    }

	return -1;
	
}

s32 dynamic_vfs_checkareaexceeded(const s8 *pathname,u16 addr,u16 len)
{
	s32 filepos;

	filepos = dynamic_vfs_check(pathname);
    if(filepos < 0)
    {
    	return VFS_RESULT_FILENOTEXIST;
    }
	
	if((addr+len)<=FsMtd1Pra.File[filepos].len)
	{
		return filepos;
	}
	else
	{
		return VFS_RESULT_FILEEND;
	}
	
	
}



DQFILE dynamic_vfs_open(const s8 *pathname,const s8 *mode,int *result)
{
	
	*result = VFS_RESULT_FILENOTEXIST;
	
	if(dynamic_vfs_check(pathname) >= 0)
	{
		*result = VFS_RESULT_SUCCESS;
	}
	
	
	return (-1);  //返回-1的时候，表示不是linux标准文件系统格式，需要查看result的值来判断操作结果
	
}



int dynamic_vfs_close(const char *pathname,DQFILE fp)//以文件名称来close,防止出错
{
	return 0;
}


int dynamic_vfs_read(const s8 *pathname, u8 *readdata, u32 readlen, u32 readaddr,DQFILE fp)
{
	 s32 filepos;
    filepos = dynamic_vfs_checkareaexceeded(pathname,readaddr,readlen);
	if(filepos < 0)
	{
		return filepos;
	}
	
	filepos = dev_flash_read(FsMtd1Pra.File[filepos].addr+ readaddr,readdata,readlen);
	
	return filepos;
}

//资源文件，不能写
int dynamic_vfs_write(const s8 *pathname, u8 *writedata, u32 writelen, u32 writeaddr,DQFILE fp)
{
    s32 filepos;

    filepos = dynamic_vfs_checkareaexceeded(pathname,writeaddr,writelen);
	if(filepos < 0)
	{
		return filepos;
	}
	filepos = dev_flash_write(FsMtd1Pra.File[filepos].addr+ writeaddr, writedata, writelen);
	return 0;
}


//资源文件，不做定位
int dynamic_vfs_seek(DQFILE File, int offset, int whence)
{
	return 0;
}


//资源文件，返回文件长度
int dynamic_vfs_tell(const char * pathname,DQFILE fp)
{
	s32 filepos;
    filepos = dynamic_vfs_check(pathname);
	if(filepos < 0)
	{
		return filepos;
	}
	
	return FsMtd1Pra.File[filepos].len;
	
}

//返回文件起始位置，和最大长度
s32 dynamic_vfs_get_fileinfo(const char *pathname, u32 *addr, u32 *len)
{
    s32 filepos;

    filepos = dynamic_vfs_check(pathname);
    if(filepos < 0)
    {
        return filepos;
    }
    *addr = FsMtd1Pra.File[filepos].addr;
    *len = FsMtd1Pra.File[filepos].len;
    return 0;
}
