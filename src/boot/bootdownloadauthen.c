


#include "bootglobal.h"



void bootdownloadauthen_GetAppInfoFromUserSpace(unsigned int hashrsatype,PCIDRIVER_APPFILEINFO *tmpappfileinfo,u8 *userspacedata)
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



s32 bootdownloadauthen_getfileautheninfo(u32 fileorgaddr,u32 filelen,PCIDRIVER_APPFILEINFO *appautheninfo)
{
    u32 length;
    u8 tmp[PCIAPPFILEINFOLEN];
    u32 hashrsatype;
    u32 readpos,readlen;

    #ifdef BOOTDOWNLOAD_DEBUG
    BOOT_DEBUG("bootdownloadauthen_getfileautheninfo = %d %d\r\n",filelen,PCIAPPFILEINFOLEN - PCIDRIVER_APPFILEINFO2048ADDLEN);
    #endif
	
    if(filelen <(PCIAPPFILEINFOLEN - PCIDRIVER_APPFILEINFO2048ADDLEN))
    {
        return 1;
    }


    length = bootdownload_fread_specifyaddr(tmp,filelen-8,4,fileorgaddr);
    if(length != 4)
    {
        return 1;
    }

    //计算文件长度
    length = tmp[3];  //little mode
    length = (length<<8) + tmp[2];
	length = (length<<8) + tmp[1];
	length = (length<<8) + tmp[0];

	#ifdef SXL_DEBUG
    BOOT_DEBUG("\r\nfilelen = %d %d %d %d\r\n",filelen,length,PCIAPPFILEINFOLEN,PCI_APPINFO_CASN_LEN);
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


    length = bootdownload_fread_specifyaddr(tmp,readpos,readlen,fileorgaddr);
    if(length != readlen)
    {
        return 1;
    }

    bootdownloadauthen_GetAppInfoFromUserSpace(hashrsatype,appautheninfo,tmp);
    return 0;
	
}



s32 bootdownloadauthen_fileauthen(u32 fileorgaddr,u32 filelen,TERMINAL_PK_TYPE pktype)
{
    PCIDRIVER_APPFILEINFO *appautheninfo;
    s32 ret;
    u8 hashvalue[32];
    u8 *pkdata;//[512];
    u8 tmpfilebindata[256];
    u32 tmpfilebindatalen;
	
	
    #ifdef APPPROGRAM_DEBUG
    return 0;
    #endif
    
    #ifdef BOOTDOWNLOAD_DEBUG
    BOOT_DEBUG("PCIAPPFILEINFOLEN = %d \r\n",PCIAPPFILEINFOLEN);
    #endif

    appautheninfo = (PCIDRIVER_APPFILEINFO *)k_malloc(sizeof(PCIDRIVER_APPFILEINFO));
    
    ret = bootdownloadauthen_getfileautheninfo(fileorgaddr,filelen,appautheninfo);
    if(ret != 0)
    {
        k_free(appautheninfo);
        return 1;
    }

    #ifdef BOOTDOWNLOAD_DEBUG
    BOOT_DEBUG("file len = %d %d\r\n",appautheninfo->filebin_len,filelen);
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
        BOOT_DEBUG("bios_fileauthen hash err22 \r\n");
        #endif
        k_free(appautheninfo);
        return 1;
    }

    #ifdef BOOTDOWNLOAD_DEBUG
    BOOT_DEBUG("hash value:%d\r\n",appautheninfo->hashdatalen);
    BOOT_DEBUGHEX(NULL, hashvalue, appautheninfo->hashdatalen);
    BOOT_DEBUG("hash value(filebin_data):%d\r\n",appautheninfo->hashdatalen);
    BOOT_DEBUGHEX(NULL, appautheninfo->filebin_data, appautheninfo->hashdatalen);    
    #endif
    
    if(memcmp(hashvalue,appautheninfo->filebin_data,appautheninfo->hashdatalen))
    {
        #ifdef BOOTDOWNLOAD_DEBUG
        BOOT_DEBUG("bios_fileauthen hash err11 \r\n");
        #endif
        k_free(appautheninfo);
        return 1;
    }
  #ifdef EXTERNAL_SPIFLASH_ENABLE
    pkdata = (u8 *)k_malloc(512);
    
    ret = pcipkmanage_readpkdata(pktype,pkdata);  //不管CASN,直接计算
    if(ret != 0)
    {
        #ifdef BOOTDOWNLOAD_DEBUG
        BOOT_DEBUG("bios_readpkdata err11 \r\n");
        #endif
        k_free(pkdata);
        k_free(appautheninfo);
        return 1;
    }


    
    
    #ifdef BOOTDOWNLOAD_DEBUG
    BOOT_DEBUG("pcidrv_rsa_decrypt1 pktype=%d, pklen=%d \r\n", pktype, RSA2048PUBLICKEYDATALEN);
    BOOT_DEBUGHEX(NULL, pkdata, RSA2048PUBLICKEYDATALEN);    
    #endif
    
    
    memset(tmpfilebindata,0,sizeof(tmpfilebindata));
    ret = pciarith_rsa_encryptalgorithm(DECRYPT,pkdata,appautheninfo->filebin_mac,appautheninfo->macdatalen,tmpfilebindata,&tmpfilebindatalen);
    if(ret != 0)
    {
        #ifdef BOOTDOWNLOAD_DEBUG
        BOOT_DEBUG("bios_fileauthen rsa decrypt err33 \r\n");
        #endif
        k_free(pkdata);
        k_free(appautheninfo);
        return 1;
    }
    k_free(pkdata);


    #ifdef BOOTDOWNLOAD_DEBUG
    BOOT_DEBUG("hash value:%d\r\n",appautheninfo->hashdatalen);
    BOOT_DEBUGHEX(NULL, tmpfilebindata, appautheninfo->hashdatalen);
    
    BOOT_DEBUG("hash value(filebin_data):%d\r\n",appautheninfo->hashdatalen);
    BOOT_DEBUGHEX(NULL, appautheninfo->filebin_data, appautheninfo->hashdatalen);
    #endif
    
    
    #ifdef BOOTDOWNLOAD_DEBUG
    BOOT_DEBUG("pcidrv_rsa_decrypt2 \r\n");
    #endif
    if(memcmp(appautheninfo->filebin_data,tmpfilebindata,appautheninfo->hashdatalen))
    {
        #ifdef BOOTDOWNLOAD_DEBUG
        BOOT_DEBUG("bios_fileauthen  err44 \r\n");
        #endif
        k_free(appautheninfo);
        return 1;
    }

    
    
    #ifdef BOOTDOWNLOAD_DEBUG
    BOOT_DEBUG("authenticate success \r\n");
    #endif
  #endif  
    k_free(appautheninfo);
    return 0;
    
}



s32 bootdownloadauthen_processfileauthentication(void)
{
    s32 ret;
    
    #ifdef APPPROGRAM_DEBUG
    return 0;//测试，不校验
    #endif
    
	
    bootdownload_prompt(DOWNLOADAUTHENTICATE,NULL,NULL);
	
    ret = bootdownloadauthen_fileauthen(gCurFileIndex->addr,gCurFileIndex->filelength,PK_TRENDIT_ROOT);
    
    
    return ret;
	
}

