


#include "bootglobal.h"



void bootdownload_saveappinfo(void)
{
	
    memcpy(gAppInfo.flag,"\xAA\x55\x55\xAA",4);
    gAppInfo.crcdata = 0xffffffff;
    gAppInfo.crcdata = GetHash(gAppInfo.crcdata,(u8 *)&gAppInfo,APPINFOFILELEN - 4);

    pcicorecallfunc.pci_kern_write_file(FIRMWAREINFOFILENAME,(u8 *)&gAppInfo,APPINFOFILELEN);
	
}



void bootdownload_readappinfo(void)
{
    u32 crc;
	#ifdef SXL_DEBUG
	u32 i;
    u8 *appdata;
	#endif
	
    pcicorecallfunc.pci_kern_read_file(FIRMWAREINFOFILENAME,(u8 *)&gAppInfo,APPINFOFILELEN,0);
    crc = 0xffffffff;
    crc = GetHash(crc,(u8 *)&gAppInfo,APPINFOFILELEN - 4);
    if((memcmp(gAppInfo.flag,"\xAA\x55\x55\xAA",4) == 0)&&(crc == gAppInfo.crcdata))
    {
        #ifdef SXL_DEBUG
	    appdata = (u8 *)&gAppInfo;
		BOOT_DEBUG("appinfo:\r\n");
        BOOT_DEBUGHEX(NULL, appdata, APPINFOFILELEN);
		#endif
        return;
    }
	
	#ifdef SXL_DEBUG
    BOOT_DEBUG("\r\nappinfo error\r\n");
	#endif
	
    memset(&gAppInfo,0,APPINFOFILELEN);
    bootdownload_saveappinfo();
	
}


s32 bootdownload_savecorefileinfo(u8 filetype, u32 filelen)
{
	if(filetype == BOOTDOWNLOAD_FILE_TYPE_COREBIN||filetype == BOOTDOWNLOAD_FILE_TYPE_APPBIN||filetype == BOOTDOWNLOAD_FILE_TYPE_PCICOREBIN)
	{
		gAppInfo.appinfo[filetype].filelen = filelen;//gCurFileIndex->filelength;
		bootdownload_saveappinfo();
	}
	else
	{
		return 1;
	}
    return 0;
	
}


s32 bootdownload_getpkstruct(RSA_PUBLICKEY_INFO *tmppublickeyinfo)
{
    u32 offset;


    //sxl?2019
	offset = 0;
	//bootdownload_fread_specifyaddr((u8 *)tmppublickeyinfo,offset,PUBLICKEY_UPPERISSUER_OFFSET,gCanDownFileInfo[gReservedaddr].fileaddr);
    //offset += PUBLICKEY_CERTSIGN_SIZE+PUBLICKEY_EPCERTSIGN_SIZE;
    //bootdownload_fread_specifyaddr(tmppublickeyinfo->Upperissuer,offset,PUBLICKEY_UPPERISSUER_SIZE+PUBLICKEY_UPPERSN_SIZE,gCanDownFileInfo[gReservedaddr].fileaddr);

	#ifdef SXL_DEBUG
    BOOT_DEBUG("\r\nbios_getpkstruct issuer:%s\r\n",tmppublickeyinfo->Upperissuer);
    BOOT_DEBUGHEX(NULL, tmppublickeyinfo->Upperissuer,PUBLICKEY_UPPERISSUER_SIZE);
	#endif
	
    //sxl?hash值需要加密
    pciarith_hash((u8 *)tmppublickeyinfo,PUBLICKEY_CERTSIGN_OFFSET,tmppublickeyinfo->CertSign,HASHTYPE_256);
	
	
	#ifdef SXL_DEBUG
    BOOT_DEBUG("\r\n bios_getpkstruct hash value: \r\n");
    BOOT_DEBUGHEX(NULL, tmppublickeyinfo->CertSign, 32);
	#endif
	
	
    return 0;
	
}


s32 bootdownload_chcekifpksupport(RSA_PUBLICKEY_INFO *tmppublickeyinfo)
{
    s32 keylen;

    keylen = tmppublickeyinfo->IPKData[3];
    //keylen = *pkmoduluslen;
    keylen = (keylen<<8) + tmppublickeyinfo->IPKData[2];
	keylen = (keylen<<8) + tmppublickeyinfo->IPKData[1];
	keylen = (keylen<<8) + tmppublickeyinfo->IPKData[0];

    if(keylen != 2048)  //只支持2048位公钥证书
    {
        return 0;
    }

    
    return 1;
}



s32 bootdownload_savepkfile(u8 filetype)//void)
{
	s32 ret;
    RSA_PUBLICKEY_INFO *tmppublickeyinfo;
    //u8 i;
    //u32 addr;
	TERMINAL_PK_TYPE pktype;
	
	
    tmppublickeyinfo = (RSA_PUBLICKEY_INFO *)k_malloc(sizeof(RSA_PUBLICKEY_INFO));

//BOOT_DEBUG("\r\n");    
    ret = bootdownload_getpkstruct(tmppublickeyinfo);
    if(ret != 0)
    {
        k_free(tmppublickeyinfo);
        return ret;
    }

//BOOT_DEBUG("\r\n");
    ret = bootdownload_chcekifpksupport(tmppublickeyinfo);
    if(ret != 1)
    {
        k_free(tmppublickeyinfo);
        return 2;
    }



    if(filetype == BOOTDOWNLOAD_FILE_TYPE_ACQUIRERPK)
    {
    	pktype = PK_ACQUIRER;
    }
    else
    {
        k_free(tmppublickeyinfo);
        return 2;
    }

	BOOT_DEBUG("pktype=%d\r\n", pktype);
	ret = pcipkmanage_savepk(pktype,tmppublickeyinfo);

	BOOT_DEBUG("ret=%d\r\n", ret);
	dev_user_delay_ms(50);
	
    k_free(tmppublickeyinfo);
    return ret;
	
	
}


s32 bootdownload_getcorefileinfo(u8 filetype, u32 *filelen)
{
    bootdownload_readappinfo();
	if(filetype == BOOTDOWNLOAD_FILE_TYPE_COREBIN||filetype == BOOTDOWNLOAD_FILE_TYPE_APPBIN||filetype == BOOTDOWNLOAD_FILE_TYPE_PCICOREBIN)
	{
        //*fileaddr = gAppInfo.appinfo[filetype].startaddr;
		*filelen = gAppInfo.appinfo[filetype].filelen;
	}
	else
	{
		return 1;
	}
    return 0;
	
}




