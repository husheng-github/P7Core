

#include "fs.h"
struct _fs_mtd1_str FsMtd1Pra;

//extern u32 ascii_2_u32(u8 *pb,u8 asciidatalen);
#define ascii_2_u32(x,y)   dev_maths_asc_to_u32(x, y, MODULUS_HEX)
/****************************************************************************
**Description:      ��ָ��  pData ��ʼλ�ò���һ��[...]�ַ���������������֮��
                    �����ݿ�����seg�У�len˵��seg buf�ĳ��ȣ��ҵ��󷵻��ַ�������
                    ͬʱ�����Ѿ���ѯ�ĳ���index
**Input parameters:
**Output parameters:
**
**Returned value:  0 �ҵ���-1 û�ҵ�
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



//�ϵ��ʱ���ȡ��̬�����ļ�,С��4K
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
	//strcpy( pdata, "[name:M1][addr:0x70000][len:0x7C000][end]");   config�ļ���ʽ
	index = 0;
    while((index+4) < 4096)
    {
        memset(data, 0, sizeof(data));
        len = sizeof(data);
        ret = find_seg(pdata + index, 4096-index, data, &len, &scan_len);
        if(ret == 0)
        {
            VfsDebug("�ҵ�һ���ַ��� %s\r\n", data);
            index += scan_len;
        }
        else
        {
            k_free(pdata);//��ѯ�����������ļ�
            return -1;//�������������ļ�
        }
		
        if(0 == memcmp(data, "name:", 5))
        {
            if(strlen((s8 *)&data[5]) < FILESYSMTDNAMELEN-1)
            {
                nameflag = 1;
				addrflag = 0;
	            strcpy((s8 *)FsMtd1Pra.File[FsMtd1Pra.num].name,(s8 *)&data[5]);
	            VfsDebug("�ҵ�����:%s\r\n", FsMtd1Pra.File[FsMtd1Pra.num].name);
            }
        }
        else if(0 == memcmp(data, "addr:", 5))
        {
        	if(len > 7)
        	{
				tmpvalue = ascii_2_u32(data + 7, len-7);
				
	            addrflag = 1;
	            FsMtd1Pra.File[FsMtd1Pra.num].addr = tmpvalue;
	            VfsDebug("�ҵ�addr:%08x\r\n", FsMtd1Pra.File[FsMtd1Pra.num].addr);
        	}
            
        }
		else if(0 == memcmp(data, "len:", 4))
        {
            if(len > 7)
            {
				tmpvalue = ascii_2_u32(data + 6, len-6);
				
				FsMtd1Pra.File[FsMtd1Pra.num].len= tmpvalue;
				VfsDebug("�ҵ�len:%08x\r\n", FsMtd1Pra.File[FsMtd1Pra.num].len);

				if(addrflag&&nameflag)
				{
					FsMtd1Pra.num++;
					if(FsMtd1Pra.num >= MAXMTD1FILENUM)
				    {
				    	k_free(pdata);//��ѯ�����������ļ�
				    	return 0;
				    }
				}
				addrflag = 0;
	            nameflag = 0;
            }
			
        }
        else if(0 == memcmp(data, "end", 3))
        {
            VfsDebug("�ҵ������ļ���β\r\n");
            k_free(pdata);//��ѯ�����������ļ�
            return 0;//���ҳɹ����˳�
        }
       	
        
    }
	
	k_free(pdata);//��ѯ�����������ļ�
	return 0;//���ҳɹ����˳�
     
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
	
	
	return (-1);  //����-1��ʱ�򣬱�ʾ����linux��׼�ļ�ϵͳ��ʽ����Ҫ�鿴result��ֵ���жϲ������
	
}



int dynamic_vfs_close(const char *pathname,DQFILE fp)//���ļ�������close,��ֹ����
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

//��Դ�ļ�������д
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


//��Դ�ļ���������λ
int dynamic_vfs_seek(DQFILE File, int offset, int whence)
{
	return 0;
}


//��Դ�ļ��������ļ�����
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

//�����ļ���ʼλ�ã�����󳤶�
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
