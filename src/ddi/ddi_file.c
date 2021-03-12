

#include "ddi_file.h"
#include "pci/pciglobal.h"



s32 ddi_file_create(const char *filename, s32 len,u8 initialvalue)
{
	s32 ret;
	
	ret = fs_create_file(filename,len,initialvalue);
	if(ret == VFS_RESULT_SUCCESS)
	{
		return DDI_OK;
	}
    else
    {
    	return DDI_ERR;
    }
	
	
}

//return write data size
s32 ddi_file_write(const char *filename,u8 *writedata,s32 writedatalen)
{
	s32 ret;

	ret = fs_write_file(filename,writedata,writedatalen);
    if(ret < 0)
	{
		return 0;
	}
    else
    {
    	return ret;
    }
	
}

// return read data size
s32 ddi_file_read(const char *filename,u8 *rbuf, s32 rlen, s32 startaddr)
{
	s32 ret;
    
	ret = fs_read_file(filename,rbuf,rlen,startaddr);
    if(ret < 0)
	{
		return 0;
	}
    else
    {
    	return ret;
    }
	
}

s32 ddi_file_insert(const char *filename,u8 *wbuf, s32 wlen, s32 startaddr)
{
	s32 ret;
    
	ret = fs_insert_file(filename,wbuf,wlen,startaddr);
    if(ret < 0)
	{
		return 0;
	}
    else
    {
    	return ret;
    }
}


s32 ddi_file_delete(const char *filename)
{
	s32 ret;
	ret = fs_delete_file(filename);
	
	if(ret == VFS_RESULT_SUCCESS)
	{
		return DDI_OK;
	}
    else
    {
    	return DDI_ERR;
    }
	
	
}

s32 ddi_file_rename(const char *srcfilename,const char *dstfilename)
{
	s32 ret;
	ret = fs_rename_file(srcfilename,dstfilename);
	
	if(ret == VFS_RESULT_SUCCESS)
	{
		return DDI_OK;
	}
    else
    {
    	return DDI_ERR;
    }
}

s32 ddi_file_getlength(const s8 *lpFileName)
{
	s32 ret;
	ret = fs_access_file((const s8 *)lpFileName);
	
	return ret;
	
}



s32 ddi_fread_specifyaddr(u8 *data,s32 fileoffset,u32 length,u32 fileorgaddr)
{
    u32 addr;

    addr = fileorgaddr;
    addr += fileoffset;


    return dev_flash_read(addr,data,length);

}



void ddi_GetAppInfoFromUserSpace(unsigned int hashrsatype,PCIDRIVER_APPFILEINFO *tmpappfileinfo,u8 *userspacedata)
{
    int i;
    unsigned int temphashlen,tempmaclen;



    if(hashrsatype == HASH256RSA2048)
    {
        tmpappfileinfo->hashrsatype = HASH256RSA2048;
        tmpappfileinfo->hashdatalen = 32;
        tmpappfileinfo->macdatalen = PCI_APPINFO_MAC_LEN;
        temphashlen = PCI_APPINFO_DATA_LEN;
        tempmaclen = PCI_APPINFO_MAC_LEN;
    }
    else
    {
        tmpappfileinfo->hashrsatype = HASH160RSA1024;
        tmpappfileinfo->hashdatalen = 20;
        tmpappfileinfo->macdatalen = PCI_APPINFO_MAC_LEN - 128;
        temphashlen = PCI_APPINFO_DATA_LEN - 8;
        tempmaclen = PCI_APPINFO_MAC_LEN - 128;
    }

#ifdef SXL_DEBUG
    //uart_printf("\r\npcidrv_GetAppInfoFromUserSpace:\r\n");
    //for(i = 0;i < PCIAPPFILEINFOLEN -(16+256);i++)
    //{
    //  uart_printf("%02x ",userspacedata[i]);
    //}
    //uart_printf("\r\n");
#endif

    memcpy((s8*)tmpappfileinfo->casn, (s8*)&userspacedata[0], PCI_APPINFO_CASN_LEN);
    i = PCI_APPINFO_CASN_LEN;
    //flag[PCI_APPINFO_FLAG_LEN]
    memcpy((s8*)tmpappfileinfo->flag, (s8*)&userspacedata[i], PCI_APPINFO_FLAG_LEN);
    i += PCI_APPINFO_FLAG_LEN;
    //filebin_data[PCI_APPINFO_DATA_LEN]
    memcpy((s8*)tmpappfileinfo->filebin_data, (s8*)&userspacedata[i], temphashlen);
    i += temphashlen;
    //elf_mac[PCI_APPINFO_MAC_LEN]
    memcpy((s8*)tmpappfileinfo->filebin_mac, (s8*)&userspacedata[i], tempmaclen);
    i += tempmaclen;
    //fileelf_data[temphashlen];
    memcpy((s8*)tmpappfileinfo->fileelf_data, (s8*)&userspacedata[i], temphashlen);
    i += temphashlen;
    //fileelf_mac[PCI_APPINFO_MAC_LEN];
    memcpy((s8*)tmpappfileinfo->fileelf_mac, (s8*)&userspacedata[i], tempmaclen);
    i += tempmaclen;
    //filebin_len;
    tmpappfileinfo->filebin_len = ((u32)userspacedata[i+3]<<24)+((u32)userspacedata[i+2]<<16)+((u32)userspacedata[i+1]<<8)+((u32)userspacedata[i+0]);
    i += 4;
    //bin_len;
    tmpappfileinfo->fileelf_len = ((u32)userspacedata[i+3]<<24)+((u32)userspacedata[i+2]<<16)+((u32)userspacedata[i+1]<<8)+((u32)userspacedata[i+0]);
    i += 4;
}



s32 ddi_downloadauthen_getfileautheninfo(u32 fileorgaddr,u32 filelen,PCIDRIVER_APPFILEINFO *appautheninfo)
{
    u32 length;
    u8 tmp[PCIAPPFILEINFOLEN];
    u32 hashrsatype;
    u32 readpos,readlen;
    
	
    if(filelen <(PCIAPPFILEINFOLEN - PCIDRIVER_APPFILEINFO2048ADDLEN))
    {
        return 1;
    }


    length = ddi_fread_specifyaddr(tmp,filelen-8,4,fileorgaddr);
    if(length != 4)
    {
        return 1;
    }

    //计算文件长度
    length = tmp[3];  //little mode
    length = (length<<8) + tmp[2];
	length = (length<<8) + tmp[1];
	length = (length<<8) + tmp[0];
	
    
    #ifdef BOOTDOWNLOAD_DEBUG
    dev_debug_printf("\r\n ddi_downloadauthen_getfileautheninfo length = %d \r\n",length,filelen);
    #endif
    if(filelen == length + PCIAPPFILEINFOLEN - PCI_APPINFO_CASN_LEN)
    {
        hashrsatype = HASH256RSA2048;
        readpos = filelen-PCIAPPFILEINFOLEN;
        readlen = PCIAPPFILEINFOLEN;
    }
    else
    {
      #if 0 
	    hashrsatype = HASH160RSA1024;
        readpos = filelen + PCIDRIVER_APPFILEINFO2048ADDLEN - PCIAPPFILEINFOLEN;
     #else
       return 1;
	 #endif
    }


    length = ddi_fread_specifyaddr(tmp,readpos,readlen,fileorgaddr);
    if(length != readlen)
    {
        return 1;
    }

    ddi_GetAppInfoFromUserSpace(hashrsatype,appautheninfo,tmp);
    return 0;
	
}


s32 ddi_downloadauthen_fileauthen(u32 fileorgaddr,u32 filelen,TERMINAL_PK_TYPE pktype)
{
    PCIDRIVER_APPFILEINFO *appautheninfo;
    s32 ret;
    u8 hashvalue[32];
    u8 *pkdata;//[512];
    u8 tmpfilebindata[256];
    u32 tmpfilebindatalen;

	#ifdef BOOTDOWNLOAD_DEBUG
    dev_debug_printf("\r\n ddi_downloadauthen_fileauthen \r\n");
    #endif
    appautheninfo = (PCIDRIVER_APPFILEINFO *)k_malloc(sizeof(PCIDRIVER_APPFILEINFO));
    ret = ddi_downloadauthen_getfileautheninfo(fileorgaddr,filelen,appautheninfo);
	#ifdef BOOTDOWNLOAD_DEBUG
    dev_debug_printf("\r\n ddi_downloadauthen_getfileautheninfo ret = %d \r\n",ret);
    #endif
    if(ret != 0)
    {
        k_free(appautheninfo);
        return 1;
    }

  #if 0
	if(memcmp(appautheninfo->flag,"M5S",3))
    {
        #ifdef BOOTDOWNLOAD_DEBUG
        dev_debug_printf("\r\nsignature flag error\r\n");
		#endif
        k_free(appautheninfo);
        return 1;
    }
  #endif
	#ifdef BOOTDOWNLOAD_DEBUG
    dev_debug_printf("\r\n ddi_downloadauthen_fileauthen filebin_len = %d %d\r\n",appautheninfo->filebin_len,filelen);
    #endif
    if(appautheninfo->filebin_len > filelen)
    {
        k_free(appautheninfo);
        return 1;
    }
    ret = pciarith_file_hash(fileorgaddr,0,appautheninfo->filebin_len,hashvalue,appautheninfo->hashrsatype);
    if(ret != 0)
    {
        #ifdef BOOTDOWNLOAD_DEBUG
        dev_debug_printf("\r\n bios_fileauthen hash err22 \r\n");
        #endif
        k_free(appautheninfo);
        return 1;
    }
    #ifdef BOOTDOWNLOAD_DEBUG
    dev_debug_printf("\r\nhash value:%d\r\n",appautheninfo->hashdatalen);
    for(ret = 0;ret < appautheninfo->hashdatalen;ret++)
    {
        dev_debug_printf("%02x ",hashvalue[ret]);
    }
    dev_debug_printf("\r\n");
    for(ret = 0;ret < appautheninfo->hashdatalen;ret++)
    {
        dev_debug_printf("%02x ",appautheninfo->filebin_data[ret]);
    }
    dev_debug_printf("\r\n");
    #endif
    if(memcmp(hashvalue,appautheninfo->filebin_data,appautheninfo->hashdatalen))
    {
        #ifdef BOOTDOWNLOAD_DEBUG
        dev_debug_printf("\r\n bios_fileauthen hash err11 \r\n");
        #endif
        k_free(appautheninfo);
        return 1;
    }
    pkdata = (u8 *)k_malloc(512);
    ret = pcipkmanage_readpkdata(pktype,pkdata);  //不管CASN,直接计算
    if(ret != 0)
    {
        #ifdef BOOTDOWNLOAD_DEBUG
        dev_debug_printf("\r\n bios_readpkdata err11 \r\n");
        #endif
        k_free(pkdata);
        k_free(appautheninfo);
        return 1;
    }
    #ifdef BOOTDOWNLOAD_DEBUG
    dev_debug_printf("\r\n pcidrv_rsa_decrypt1 \r\n");
    #endif
    memset(tmpfilebindata,0,sizeof(tmpfilebindata));
    ret = pciarith_rsa_encryptalgorithm(DECRYPT,pkdata,appautheninfo->filebin_mac,appautheninfo->macdatalen,tmpfilebindata,&tmpfilebindatalen);
    if(ret != 0)
    {
        #ifdef BOOTDOWNLOAD_DEBUG
        dev_debug_printf("\r\n bios_fileauthen rsa decrypt err33 \r\n");
        #endif
        k_free(pkdata);
        k_free(appautheninfo);
        return 1;
    }
    k_free(pkdata);
    #ifdef BOOTDOWNLOAD_DEBUG
    dev_debug_printf("\r\nhash value:%d\r\n",appautheninfo->hashdatalen);
    for(ret = 0;ret < appautheninfo->hashdatalen;ret++)
    {
        dev_debug_printf("%02x ",tmpfilebindata[ret]);
    }
    dev_debug_printf("\r\n");
    for(ret = 0;ret < appautheninfo->hashdatalen;ret++)
    {
        dev_debug_printf("%02x ",appautheninfo->filebin_data[ret]);
    }
    dev_debug_printf("\r\n");
    #endif
    #ifdef BOOTDOWNLOAD_DEBUG
    dev_debug_printf("\r\n pcidrv_rsa_decrypt2 \r\n");
    #endif
    if(memcmp(appautheninfo->filebin_data,tmpfilebindata,appautheninfo->hashdatalen))
    {
        #ifdef BOOTDOWNLOAD_DEBUG
        dev_debug_printf("\r\n bios_fileauthen  err44 \r\n");
        #endif
        k_free(appautheninfo);
        return 1;
    }
    k_free(appautheninfo);
    return 0;
}


s32 ddi_file_veritysignature(const s8 *lpFileName,u8 mode,u32 fileaddr,u32 filelen)  //sxl? 需要完善
{
	s32 ret;
	s32 addr;
	u32 arealen;

    if(mode == 0)
    {
    	addr = static_vfs_getarea((const char *)lpFileName,0,&arealen);
    	if(addr < 0)
    	{
    		return DDI_ERR;
    	}
        ret = ddi_downloadauthen_fileauthen(addr,filelen,PK_TRENDIT_ROOT);
        if(ret == 0)
        {
    	    return DDI_OK;
        }
        else
        {
        	return DDI_ERR;
        }
    }
    else if(mode == 1)
    {
        //addr = fileaddr;
        //arealen = filelen;
        return DDI_ERR;     //暂不支持
    }
    else
    {
        return DDI_ERR;
    }
    
}

s32 ddi_flash_read(u32 addrsta, u8* rbuf, u32 rlen)
{
    return dev_flash_read(addrsta, rbuf, rlen);
}

s32 ddi_flash_write(u32 addrsta, u8* wbuf, u32 wlen)
{
    return dev_flash_write(addrsta, wbuf, wlen);
}

