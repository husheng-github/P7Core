


#include "pciglobal.h"



#if 1


//20140327
void pcikeymanageauthen_authendata_init(void)
{
    appkeylockbits = 0;
    memset(&gPinKeyUseInternal,0,sizeof(PINKEYUSEINTERNAL));
    timerstartflag = 0;
    appkeylocktimer = 0;
	
    memset(&gSecurityAuthInfo,0,sizeof(SECURITYAUTHENTICATEINFO));
	
}
/*******************************************************************
Author:   sxl
Function Name:    void pcikeymanageauthen_securityauthinfo_initial(void)
Function Purpose: security authentication information global variable initial 
Input Paramters:  N/A
Output Paramters:N/A
Return Value:       N/A
Remark: N/A
Modify: N/A         //20140327
********************************************************************/
void pcikeymanageauthen_securityauthinfo_initial(void)
{
    memset(&gSecurityAuthInfo,0,sizeof(SECURITYAUTHENTICATEINFO));
}


/*******************************************************************
Author:   sxl
Function Name:    void pcidrv_initialuseinternal(void)
Function Purpose: initial use internal
Input Paramters:   N/A
Output Paramters:N/A
Return Value:      N/A
Remark: N/A
Modify: N/A   //20140327
********************************************************************/
void pcikeymanageauthen_initialuseinternal(void)
{
    memset(&gPinKeyUseInternal,0,sizeof(PINKEYUSEINTERNAL));
}

/*******************************************************************
Author:   sxl
Function Name:    s32 pcikeymanageauthen_appgroupkey_checkissuerSN(u8 groupindex,u8 *issuerSN)
Function Purpose: check if the app issuerSN compact with the group key issuer SN
Input Paramters:  groupindex - group key index
                          *issuerSN  - issuer and SN
Output Paramters:N/A
Return Value:      PCI_PROCESSCMD_SUCCESS  -      success
                         other - failed
Remark: N/A
Modify: N/A        //20140327
********************************************************************/
s32 pcikeymanageauthen_appgroupkey_checkissuerSN(u8 groupindex,u8 *issuerSN)
{
	
    s32 ret;
    u8 tmpcmp[AUTHENTICATION_APPKEYHEADINFO_SIZE];
    u8 tmpissuerSN[AUTHENTICATION_APPKEYHEADINFO_SIZE];

    //sxlremark
    //读取密钥文件所使用的证书信息
    //Read certificate information used by the key file
    ret = pcikeymanage_readappkeyheadinfostatus(groupindex,tmpissuerSN);
    DDEBUG("\r\n pcidrv_authen_readappkeyheadinfo ret = %d \r\n",ret);
    if(ret != PCI_PROCESSCMD_SUCCESS)
    {
        return ret;
    }
    
    memset(tmpcmp,0,sizeof(tmpcmp));
    //sxlremark
    //该组密钥还没有使用过，返回使用的证书一致
    //The group keys are not used ,return the certificate use the same
    if(Lib_memcmp(tmpcmp,tmpissuerSN,AUTHENTICATION_APPKEYHEADINFO_SIZE) == 0)
    {
        return PCI_PROCESSCMD_SUCCESS;
    }

    
    #ifdef SXL_DEBUG
    DDEBUG("input issuer \r\n");
    for(ret = 0;ret < AUTHENTICATION_APPKEYHEADINFO_SIZE;ret++)
    {
        DDEBUG("%02x ",issuerSN[ret]);
    }
    DDEBUG("\r\n");
    DDEBUG("read issuer \r\n");
    for(ret = 0;ret < AUTHENTICATION_APPKEYHEADINFO_SIZE;ret++)
    {
        DDEBUG("%02x ",tmpissuerSN[ret]);
    }
    DDEBUG("\r\n");
    #endif

    //sxlremark
    // 比较使用的证书是否一致
    //Compare if use the same certificate
    if(Lib_memcmp(tmpissuerSN,issuerSN,AUTHENTICATION_APPKEYHEADINFO_SIZE))
    {
        return PCI_AUTHENKEYISSUER_ERROR;
    }

    return PCI_PROCESSCMD_SUCCESS;
	
}
/*******************************************************************
Author:   sxl
Function Name:    s32 pcidrv_applockinfo_check(APPKEYLOCKINFO *lockinfo)
Function Purpose: check if application information be changed
Input Paramters: *lockinfo - lock information
Output Paramters:N/A
Return Value:      PCI_PROCESSCMD_SUCCESS  -      success
                         other - failed
Remark: N/A      sxl 1130
Modify: N/A //20140327
********************************************************************/
s32 pcikeymanageauthen_applockinfo_check(APPKEYLOCKINFO *lockinfo)
{
    
    u8 tmp[100];
    u8 tmp1[100];
    //u8 i;
    u8 tmpCAK[DRV_PCI_CAK_LEN];
    s32 ret;

    //sxlremark
    //读取CAK
    //read CAK
    ret = pcicorecallfunc.pcidrv_readCAK(tmpCAK);
    if(ret != PCI_PROCESSCMD_SUCCESS)
    {
        DDEBUG("%s:read cak fail \r\n",__FUNCTION__);
        return ret;
    }
    memset(tmp,0,sizeof(tmp));
    memcpy(tmp,(u8 *)lockinfo,sizeof(APPKEYLOCKINFO));
    //sxlremark
    //使用CAk对密钥锁数据进行解密
    //decrypt the key lock file using the CAK
    drv_tdes(DECRYPT,tmp,32,tmpCAK,DRV_PCI_CAK_LEN);
    memset(tmpCAK,0,sizeof(tmpCAK));

    memset(tmp1,0,sizeof(tmp1));
    memcpy(tmp1,tmp,27);
    pciarith_hash(tmp1,27,&tmp[32],HASHTYPE_256);

    //sxlremark
    //校验密钥锁文件的完整性
    //Check the integrity of the key lock file
    if(Lib_memcmp(&tmp[27],&tmp[32],5))
    {
        return PCI_AUTHENAPPLOCKINFO_ERR;
    }

    //sxlremark
    //返回密钥锁文件
    //return the group key lock file
    memcpy((u8 *)lockinfo,tmp,sizeof(APPKEYLOCKINFO));
    return PCI_PROCESSCMD_SUCCESS;
	
}

/*******************************************************************
Author:   sxl
Function Name:    s32 pcidrv_applockfile_initial(void)
Function Purpose: application 
Input Paramters:  N/A
Output Paramters:N/A
Return Value:      PCI_PROCESSCMD_SUCCESS  -      success
                         other - failed
Remark: N/A
Modify: N/A   //20140327
********************************************************************/
s32 pcikeymanageauthen_applockfile_initial(void)
{
    u32 pos;
    s32 ret;
    APPKEYLOCKINFO tmplockinfo;
    s32 i;

	DDEBUG("\r\n%s 1\r\n",__FUNCTION__);
	
    pos = 0;  //sxl?  需要测试在后面写的情况
    for(i = 0;i < PCIPOS_APPKEYGROUP_MAXNUM;i++)
    {
        memset(&tmplockinfo,0,sizeof(APPKEYLOCKINFO));
        tmplockinfo.appkeyindex = i+1;
        ret = pcikeymanageauthen_applockinfo_get(&tmplockinfo);
		DDEBUG("\r\n%s ret = %d\r\n",__FUNCTION__,ret);
        if(ret != PCI_PROCESSCMD_SUCCESS)
        {
            return ret;
        }
        ret = pcicorecallfunc.pci_kern_insert_file(PCIAPPKEYLOCKINFOFILE,(u8 *)&tmplockinfo,sizeof(APPKEYLOCKINFO),pos);
        if(ret != sizeof(APPKEYLOCKINFO))
        {
            return PCI_DEALPCIFILE_ERR;
        }
		pos += ret;
		
    }

	DDEBUG("\r\n%s 2\r\n",__FUNCTION__);

    appkeylockbits = 0;
    return PCI_PROCESSCMD_SUCCESS;
    
}

/*******************************************************************
Author:   sxl
Function Name:    s32 pcikeymanageauthen_applockinfo_get(APPKEYLOCKINFO *lockinfo)
Function Purpose: change application key lock information to encrypted data
Input Paramters: *lockinfo - lock information
Output Paramters:N/A
Return Value:      PCI_PROCESSCMD_SUCCESS  -      success
                         other - failed
Remark: N/A
Modify: N/A  //20140327
********************************************************************/
s32 pcikeymanageauthen_applockinfo_get(APPKEYLOCKINFO *lockinfo)
{
    u8 tmp[100];
    u8 tmp1[100];
    //u8 i;
    u8 tmpCAK[DRV_PCI_CAK_LEN];
    s32 ret;

    //sxlremark
    //读取CAK
    //read CAK
    ret = pcicorecallfunc.pcidrv_readCAK(tmpCAK);
    if(ret != PCI_PROCESSCMD_SUCCESS)
    {
        DDEBUG("%s:read cak fail \r\n",__FUNCTION__);
        return ret;
    }

    //sxlremark
    //生成8字节随机数,使用随机数以防止锁文件数据造假
    //8-byte random number generated using the random number to prevent false lock file data
    pcicorecallfunc.pci_get_random_bytes(lockinfo->random,8);
    memset(tmp1,0,sizeof(tmp1));
    memset(tmp,0,sizeof(tmp));
    memcpy(tmp1,(u8 *)lockinfo,27);
    //sxlremark
    //计算前27个字节的HASH值,取前5个字节
    //calculating the value of first 27 bytes, take the first 5 bytes
    pciarith_hash(tmp1,27,&tmp[27],HASHTYPE_256);
    memcpy(tmp,(u8 *)lockinfo,27);
    
    //sxlremark
    //使用CAK对密钥锁文件进行加密
    //encrypt the key lock file using the CAK
    drv_tdes(ENCRYPT,tmp,32,tmpCAK,DRV_PCI_CAK_LEN);
    
    memset(tmpCAK,0,sizeof(tmpCAK));
    memcpy((u8 *)lockinfo,tmp,32);
    return PCI_PROCESSCMD_SUCCESS;
	
	
}

/*******************************************************************
Author:   sxl
Function Name:    s32 pcikeymanageauthen_applockinfo_save(u8 groupindex,APPKEYLOCKINFO *lockinfo)
Function Purpose: save application 
Input Paramters: *lockinfo - lock information
Output Paramters:N/A
Return Value:      PCI_PROCESSCMD_SUCCESS  -      success
                         other - failed
Remark: N/A
Modify: N/A         sxl 1130  //20140327
********************************************************************/
s32 pcikeymanageauthen_applockinfo_save(u8 groupindex,APPKEYLOCKINFO *lockinfo)
{
    unsigned int pos;
    s32 ret;
    APPKEYLOCKINFO tmplockinfo;
    //u8 tmp[APPKEYLOCKINFO_SIZE];
    u8 i;

    if((groupindex >= PCIPOS_APPKEYGROUP_MAXNUM)||(lockinfo == NULL))
    {
        return PCI_KEYGROUPINDEX_ERR;
    }
    
    //sxlremark
    //因为要对密钥锁文件进行加密，所以将密钥锁数据保存到一个缓存中
    //he key lock data stored in a cache
    memcpy((u8 *)&tmplockinfo,(u8 *)lockinfo,sizeof(APPKEYLOCKINFO));

    for(i = 0;i < 2;i++)
    {
        if(pcicorecallfunc.pci_kern_access_file(PCIAPPKEYLOCKINFOFILE) < sizeof(APPKEYLOCKINFO)*PCIPOS_APPKEYGROUP_MAXNUM)
        {
           DDEBUG("\r\n%s access failed%d \r\n",__FUNCTION__,i);
           if(i == 0)
           {
                ret = pcikeymanageauthen_applockfile_initial();
                if(ret != PCI_PROCESSCMD_SUCCESS)
                {
                    #ifdef SXL_DEBUG
			        DDEBUG("\r\n%s lock ino initial failed %d \r\n",__FUNCTION__,ret);
					#endif
                    return ret;
                }
           }
           else
           {
                #ifdef SXL_DEBUG
		        DDEBUG("\r\n%s lock ino create failed \r\n",__FUNCTION__);
				#endif
                return PCI_APPLOCKINFOSAVE_ERR;
           }
        }
        else
        {
            DDEBUG("\r\n open dukptk success %d \r\n",i);
            break;
        }
		
    }
	
    
    pos = ((s32)groupindex)*sizeof(APPKEYLOCKINFO); 
    //sxlremark
    //对密钥锁文件使用CAK进行加密
    //encrypted the key lock file using the CAK
    ret = pcikeymanageauthen_applockinfo_get(&tmplockinfo);
    if(ret != PCI_PROCESSCMD_SUCCESS)
    {
        #ifdef SXL_DEBUG
        DDEBUG("\r\n%s lock ino get failed %d \r\n",__FUNCTION__,ret);
		#endif
        return ret;
    }
	
    //sxlremark
    //保存密钥锁文件
    //save key lock file
    ret = pcicorecallfunc.pci_kern_insert_file(PCIAPPKEYLOCKINFOFILE,(u8 *)&tmplockinfo,sizeof(APPKEYLOCKINFO),pos);
    if(ret != sizeof(APPKEYLOCKINFO))
    {
        #ifdef SXL_DEBUG
        DDEBUG("\r\n%s insertfile failed %d \r\n",__FUNCTION__,ret);
		#endif
        return PCI_DEALPCIFILE_ERR;
    }
	
	#ifdef SXL_DEBUG
	DDEBUG("\r\n%s insertfile success\r\n",__FUNCTION__);
	#endif
 	
    return PCI_PROCESSCMD_SUCCESS;
	
}

/*******************************************************************
Author:   sxl
Function Name:    s32 pcikeymanageauthen_appkeylockfile_read(u8 groupindex,APPKEYLOCKINFO *lockinfo)
Function Purpose: read application key lock file
Input Paramters: groupindex   - app key group index
                        lockinfo        -application lock information file
Output Paramters:N/A
Return Value:      PCI_PROCESSCMD_SUCCESS  -      success
                         other - failed
Remark: N/A
Modify: N/A        sxl 0124  //20140327
********************************************************************/
s32 pcikeymanageauthen_appkeylockfile_read(u8 groupindex,APPKEYLOCKINFO *lockinfo)
{
    s32 ret;
    u32 pos;
	
    if(groupindex >= PCIPOS_APPKEYGROUP_MAXNUM)
    {
        return PCI_KEYGROUPINDEX_ERR;
    }

	pos = ((s32)groupindex)*APPKEYLOCKINFO_SIZE;
	
    //sxlremark
    //读取该组密钥锁文件
    //read the group key file
    ret = pcicorecallfunc.pci_kern_read_file(PCIAPPKEYLOCKINFOFILE,(u8 *)lockinfo,APPKEYLOCKINFO_SIZE,pos);
    if(ret == APPKEYLOCKINFO_SIZE)  
    {
        //check the lock informaion 
        ret = pcikeymanageauthen_applockinfo_check(lockinfo);
        if(ret == PCI_PROCESSCMD_SUCCESS)  //read lock information success
        {
            if(lockinfo->appkeyindex == (groupindex + 1)) //check index first
            {
                //set_fs(old_fs);
                return PCI_PROCESSCMD_SUCCESS;
            }
        }
    }

    //testggroupindex();
    //处理lock信息
    //读出来出错或是错误的处理,防止文件写错了 
    //sxlremark
    //读取的锁文件格式错误或不存在,重新初始化该组密钥
    //
    //set_fs(old_fs);
    //pci_setfs(old_fs);
    memset(lockinfo,0,sizeof(APPKEYLOCKINFO));
    lockinfo->appkeyindex = groupindex + 1;
    //lockinfo->failedtimes = MAXFAILEDTIMES;
    //memcpy(lockinfo->tickslocked,APPKEYLOCKTIMES,8);
    //pcidrv_cmd_synclockflag(groupindex);
    ret = pcikeymanageauthen_applockinfo_save(groupindex,lockinfo);
    pcikeymanage_groupkeyfile_initial(groupindex);
    return ret;

    
}

/*******************************************************************
Author:   sxl
Function Name:    s32 pcikeymanageauthen_groupapplockfile_initial(u8 groupindex)
Function Purpose: group key lock file initial 
Input Paramters:  N/A
Output Paramters:N/A
Return Value:      PCI_PROCESSCMD_SUCCESS  -      success
                         other - failed
Remark: N/A
Modify: N/A         sxl //20140327
********************************************************************/
s32 pcikeymanageauthen_groupapplockfile_initial(u8 groupindex)
{
    unsigned int pos;
    s32 ret;
    APPKEYLOCKINFO tmplockinfo;
    s32 i;
    u32 key1ockbits = 1;
    
    if(groupindex >= PCIPOS_APPKEYGROUP_MAXNUM)
    {
        return PCI_KEYGROUPINDEX_ERR;
    }
    
    pos = groupindex;
    
    pos = pos*sizeof(APPKEYLOCKINFO);
    
    i = groupindex;
    memset(&tmplockinfo,0,sizeof(APPKEYLOCKINFO));
    tmplockinfo.appkeyindex = i+1;
    ret = pcikeymanageauthen_applockinfo_get(&tmplockinfo);
    if(ret != PCI_PROCESSCMD_SUCCESS)
    {
        return ret;
    }
    ret = pcicorecallfunc.pci_kern_insert_file(PCIAPPKEYLOCKINFOFILE,(u8 *)&tmplockinfo,sizeof(APPKEYLOCKINFO),pos);
    if(ret != sizeof(APPKEYLOCKINFO))
    {
        return PCI_DEALPCIFILE_ERR;
    }
    
    key1ockbits = key1ockbits<<groupindex;
    appkeylockbits &= (~(key1ockbits));
    return PCI_PROCESSCMD_SUCCESS;
	
}

/*******************************************************************
Author:   sxl
Function Name:    void pcikeymanageauthen_cmd_synclockflag(u8 groupindex)
Function Purpose: check if groupindex  according appkeylockbits lock flag is set
Input Paramters: 
                          groupindex - group index
Output Paramters:N/A
Return Value:       N/A
Remark: N/A        sxl 1130
Modify: N/A         //20140327
********************************************************************/
void pcikeymanageauthen_cmd_synclockflag(u8 groupindex)
{
    u32 key1ockbits = 1;

    if(groupindex < PCIPOS_APPKEYGROUP_MAXNUM)
    {
        key1ockbits = key1ockbits << groupindex;
        if((appkeylockbits&key1ockbits) == 0)
        {
            appkeylockbits |= key1ockbits;
        }
    }
}


/*******************************************************************
Author:   sxl
Function Name:    s32 pcidrv_authen_downappkeyreq(u8 *appinfo,u8 groupindex)
Function Purpose: the authenticate step of download application key request
Input Paramters:   *appinfo   - application information
                          groupindex   - app key group index
Output Paramters:N/A
Return Value:      PCI_PROCESSCMD_SUCCESS  -      success
                         other - failed
Remark: N/A
Modify: N/A     sxl 1130  //20140327
********************************************************************/
s32 pcikeymanageauthen_downappkeyreq(u8 *appinfo,u8 groupindex)
{
    s32 ret;
    u8 *appissuerSN;
    //u8 *appname;

    if(groupindex >= PCIPOS_APPKEYGROUP_MAXNUM)
    {
        return PCI_KEYGROUPINDEX_ERR;
    }

    appissuerSN = appinfo;
    //appname     = &appinfo[AUTHENTICATION_APPKEYHEADINFO_SIZE];

    //sxlremark
    //检查该组密钥签发者与请求下载者所使用的公钥证书是否一样
    //Check if the group key file and the requester use the same public key certificate
    ret = pcikeymanageauthen_appgroupkey_checkissuerSN(groupindex,appissuerSN);
    if(ret != PCI_PROCESSCMD_SUCCESS)
    {
        //sxlremark
        //
        return ret;
    }
    
    //memset(gSecurityAuthInfo.Appname,0,sizeof(gSecurityAuthInfo.Appname));
    //strcpy(gSecurityAuthInfo.Appname,appname);
    //sxlremark
    //保存所使用的公钥证书信息到缓存中
    //
    memset(gSecurityAuthInfo.IssuerSN,0,sizeof(gSecurityAuthInfo.IssuerSN));
    memcpy(gSecurityAuthInfo.IssuerSN,appissuerSN,AUTHENTICATION_APPKEYHEADINFO_SIZE);


    //need read the group app key information from file  //sxl modify read lock info every time
    //if((gSecurityAuthInfo.appkeylockinfo.appkeyindex != (groupindex+1))||(gSecurityAuthInfo.appkeylockinfo.failedtimes == MAXFAILEDTIMES))
    //{
        //sxlremark
        //检查该组密钥是否被锁
        //Check whether the group key is locked
        ret = pcikeymanageauthen_appkeylockfile_read(groupindex,&(gSecurityAuthInfo.appkeylockinfo));
        DDEBUG("\r\n %s ret = %d \r\n",__FUNCTION__,ret);
        if(ret != PCI_PROCESSCMD_SUCCESS)
        {
            return ret;
        }
    //}
    

    //this group app key is locked   系统重新设置时间就无效了
    //sxlremark
    //检查该组密钥是否被锁
    //Check whether the group key is locked
    if(gSecurityAuthInfo.appkeylockinfo.failedtimes >= MAXFAILEDTIMES)
    {
        pcikeymanageauthen_cmd_synclockflag(groupindex);
        return PCI_AUTHENAPPKEYLOCK_ERR;
    }
    return PCI_PROCESSCMD_SUCCESS;  //application unlocked

}


/*******************************************************************
Author:   sxl
Function Name:    s32 pcikeymanageauthen_authen_downappkeycheck(u8 groupindex)
Function Purpose:  down the application key check 
Input Paramters:   groupindex          - app key group index
Output Paramters:N/A
Return Value:      PCI_PROCESSCMD_SUCCESS  -      success
                         other - failed
Remark: N/A       sxl 1130
Modify: N/A        //20140327
********************************************************************/
s32 pcikeymanageauthen_authen_downappkeycheck(u8 groupindex)
{
    if(gSecurityAuthInfo.appkeylockinfo.appkeyindex != (groupindex+1))
    {
        DDEBUG("%s: %d %d app key group index is different !\r\n",__FUNCTION__,gSecurityAuthInfo.appkeylockinfo.appkeyindex,groupindex);
        return PCI_KEYGROUPINDEX_ERR;
    }

    //check if certified
    if(gSecurityAuthInfo.IfCertified == 0)
    {
        DDEBUG("%s:not certified \r\n",__FUNCTION__);
        return PCI_AUTHEN_NEED;
    }

    
    //sxl?还要比较时间，看认证有效期是否过了
    if(gSecurityAuthInfo.effectivetime == 0)
    {
        gSecurityAuthInfo.IfCertified = 0;
        gSecurityAuthInfo.effectivetime = 0;
        DDEBUG("%s:not certified1 \r\n",__FUNCTION__);
        return PCI_AUTHEN_NEED;
    }

    return PCI_PROCESSCMD_SUCCESS;
}

/*******************************************************************
Author:   sxl
Function Name:     s32 pcikeymanageauthen_authen_downmagkeycheck(u8 void)
Function Purpose:  down the mag key check 
Input Paramters:   N/A
Output Paramters:N/A
Return Value:      PCI_PROCESSCMD_SUCCESS  -      success
                         other - failed
Remark: N/A       sxl 1130
Modify: N/A        //20140327
********************************************************************/
s32 pcikeymanageauthen_authen_downmagkeycheck(void)
{
    //check if certified
    if(gSecurityAuthInfo.IfCertified == 0)
    {
        DDEBUG("%s:not certified \r\n",__FUNCTION__);
        return PCI_AUTHEN_NEED;
    }

    //sxl?还要比较时间，看认证有效期是否过了
    if(gSecurityAuthInfo.effectivetime == 0)
    {
        gSecurityAuthInfo.IfCertified = 0;
        gSecurityAuthInfo.effectivetime = 0;
        DDEBUG("%s:not certified1 \r\n",__FUNCTION__);
        return PCI_AUTHEN_NEED;
    }

    return PCI_PROCESSCMD_SUCCESS;
	
}
/*******************************************************************
Author:   sxl
Function Name:     s32 pcikeymanageauthen_authen_setwkcheck(u8 groupindex)
Function Purpose:  set work key check
Input Paramters:   groupindex          - app key group index
Output Paramters:N/A
Return Value:      PCI_PROCESSCMD_SUCCESS  -      success
                         other - failed
Remark: N/A      
Modify: N/A       1130  //20140327
********************************************************************/
s32 pcikeymanageauthen_authen_setwkcheck(u8 groupindex)
{
    APPKEYLOCKINFO lockinfo;
    
    if(groupindex >= PCIPOS_APPKEYGROUP_MAXNUM)
    {
        return PCI_KEYGROUPINDEX_ERR;
    }

    pcikeymanageauthen_appkeylockfile_read(groupindex,&lockinfo);

    if(lockinfo.failedtimes >= MAXFAILEDTIMES)
    {
        return PCI_AUTHENAPPKEYLOCK_ERR;
    }

    return PCI_PROCESSCMD_SUCCESS;
    
}

/*******************************************************************
Author:   sxl
Function Name:    s32 pcikeymanageauthen_authen_saveappkeyheadinfo(u8 groupindex)
Function Purpose: the authenticate step of download application key file head information save 
Input Paramters:  
                          groupindex          - app key group index
Output Paramters:N/A
Return Value:      PCI_PROCESSCMD_SUCCESS  -      success
                         other - failed
Remark: N/A
Modify: N/A     sxl 1130   //20140327
********************************************************************/
s32 pcikeymanageauthen_authen_saveappkeyheadinfo(u8 groupindex)
{
    s8 keyfilepath[128];
    s32 ret;
    u8 tmp[AUTHENTICATION_APPKEYHEAD_SIZE];
    s32 addr;
    //u8 certdata[24];
    u8 tmpCAK[DRV_PCI_CAK_LEN];
    
    #if 1   //removed by weijiakai 2014-07-16
    if(gSecurityAuthInfo.appkeylockinfo.appkeyindex != (groupindex+1))
    {
        DDEBUG("%s:app key group index is different !\r\n",__FUNCTION__);
        return PCI_KEYGROUPINDEX_ERR;
    }
    #endif

    memset(keyfilepath,0,sizeof(keyfilepath));
    ret = pcikeymanage_getappkeyfileabsolutepath(groupindex,keyfilepath);
    if(ret != PCI_PROCESSCMD_SUCCESS)
    {
        return ret;
    }

    ret = pcicorecallfunc.pcidrv_readCAK(tmpCAK);
    if(ret != PCI_PROCESSCMD_SUCCESS)
    {
        DDEBUG("%s:read cak fail \r\n",__FUNCTION__);
        return ret;
    }
    
    memset(tmp,0,sizeof(tmp));
    memcpy(tmp,gSecurityAuthInfo.IssuerSN,AUTHENTICATION_APPKEYHEADINFO_SIZE);
    //memset(certdata,0,sizeof(certdata));
    pciarith_hash(tmp,AUTHENTICATION_APPKEYNAMEHEADINFO_SIZE,&tmp[AUTHENTICATION_APPKEYNAMEHEADINFO_SIZE],HASHTYPE_256);
    
    ret = drv_tdes(ENCRYPT,&tmp[AUTHENTICATION_APPKEYNAMEHEADINFO_SIZE],32,tmpCAK,DRV_PCI_CAK_LEN);
    memset(tmpCAK,0,sizeof(tmpCAK));
    if(ret != 0)
    {
        return PCI_DEAL_ERR;
    }
    
    addr = 0;
    ret = pcicorecallfunc.pci_kern_insert_file(keyfilepath,tmp,AUTHENTICATION_APPKEYHEAD_SIZE,addr);
    if(ret != PCI_PROCESSCMD_SUCCESS)
    {
        return PCI_WRITEKEY_ERR;
    }

    return PCI_PROCESSCMD_SUCCESS;
    
}
/*******************************************************************
Author:   sxl
Function Name:    s32 pcikeymanageauthen_readappkeyheadinfoappname(u8 groupindex,u8 *issuerSN)
Function Purpose: application key file head information app name read 
Input Paramters:  groupindex - app key group index
Output Paramters:*appname  - the group key service object appname
Return Value:      PCI_PROCESSCMD_SUCCESS  -      success
                         other - failed
Remark: N/A
Modify: N/A        sxl 1130   //20140327
********************************************************************/
s32 pcikeymanageauthen_readappkeyheadinfoappname(u8 groupindex,u8 *appname)
{
    u8 tmpCAK[DRV_PCI_CAK_LEN];
	s8 keyfilepath[128];
	s32 ret;
	u8 tmp[AUTHENTICATION_APPKEYHEAD_SIZE];
	u8 tmphash[32];
	u8 tmpcmp[AUTHENTICATION_APPKEYHEAD_SIZE];
	
    unsigned int pos;

    if(groupindex >= PCIPOS_APPKEYGROUP_MAXNUM)
    {
    	return PCI_KEYGROUPINDEX_ERR;
    }
    
	//sxlremark
	//根据密钥组号读取密钥文件所使用公钥证书信息
	//read the public key certificate information used by the secret key file  according to Group number
	memset(keyfilepath,0,sizeof(keyfilepath));
	ret = pcikeymanage_getappkeyfileabsolutepath(groupindex,keyfilepath);
	if(ret != PCI_PROCESSCMD_SUCCESS)
	{
		return ret;
	}

	
    pos = 0; 
    ret = pcicorecallfunc.pci_kern_read_file(keyfilepath, tmp, AUTHENTICATION_APPKEYHEAD_SIZE, pos);
	//sxlremark
	//读取文件失败. 重新初始化该组密钥文件
	//Read the file failed. Re-initialize the group key file
    if(ret != AUTHENTICATION_APPKEYHEAD_SIZE)
    {
    	return PCI_DEALPCIFILE_ERR;
    }

    
	memset(tmpcmp,0xff,sizeof(tmpcmp));
	//sxlremark
	//该组密钥还没有使用过
	//The group key has not been used
	if(Lib_memcmp(tmp,tmpcmp,AUTHENTICATION_APPKEYHEAD_SIZE) == 0)
	{
       return PCI_APPKEYHEADINFONOTEXIST;
	}

	//sxlremark
	//读取CAK
	//read CAK
	ret = pcicorecallfunc.pcidrv_readCAK(tmpCAK);
	if(ret != PCI_PROCESSCMD_SUCCESS)
	{
	    DDEBUG("%s:read cak fail \r\n",__FUNCTION__);
		return ret;
	}
	memset(tmphash,0,sizeof(tmphash));
	//sxlremark
	//计算公钥证书标识HASH值
	//Calculate the HASH value of public-key certificate identifies 
    pciarith_hash(tmp,AUTHENTICATION_APPKEYNAMEHEADINFO_SIZE,tmphash,HASHTYPE_256);
	//sxlremark
	//使用CAK对hash值进行加密
	//encrypt the hash value using the CAK
	ret = drv_tdes(ENCRYPT,tmphash,32,tmpCAK,DRV_PCI_CAK_LEN);
	memset(tmpCAK,0,sizeof(tmpCAK));
	if(ret != 0)
	{
		return PCI_DEAL_ERR;
	}
	//sxlremark
	//比较值是否一样
	//check if the value is the same
	if(Lib_memcmp(tmphash,&tmp[AUTHENTICATION_APPKEYNAMEHEADINFO_SIZE],32))
	{
		return PCI_DEALPCIFILE_ERR;
	}
	
    //sxlremark
    //返回该组密钥文件支持的应用名称
    //return the app name supported by the group key file
	memcpy(appname,&tmp[AUTHENTICATION_APPKEYHEADINFO_SIZE],AUTHENTICATION_APPKEYHEADINFO_SIZE);
	return PCI_PROCESSCMD_SUCCESS;
	
	
}

/*******************************************************************
Author:   sxl
Function Name:    s32 pcikeymanageauthen_authen_authenticatefail(u8 groupindex)
Function Purpose: pos authenticate fail
Input Paramters:  groupindex - the application key group index
Output Paramters:N/A
Return Value:      PCI_PROCESSCMD_SUCCESS  -      success
                         other - failed
Remark: N/A
Modify: N/A  1130  //20140327
********************************************************************/
s32 pcikeymanageauthen_authen_authenticatefail(u8 groupindex)
{
    APPKEYLOCKINFO lockinfo;
    s32 ret = PCI_PROCESSCMD_SUCCESS;
    u8 failedtimes;
    
    if(groupindex >= PCIPOS_APPKEYGROUP_MAXNUM)
    {
        return PCI_KEYGROUPINDEX_ERR;
    }

    memcpy(&lockinfo,&(gSecurityAuthInfo.appkeylockinfo),sizeof(APPKEYLOCKINFO));

    failedtimes = lockinfo.failedtimes;
    memset(&lockinfo,0,sizeof(APPKEYLOCKINFO));
    lockinfo.failedtimes = failedtimes;
    lockinfo.appkeyindex = groupindex + 1;
    (lockinfo.failedtimes)++;  //MAXFAILEDTIMES
    if(lockinfo.failedtimes >= MAXFAILEDTIMES)
    {
        lockinfo.failedtimes = MAXFAILEDTIMES;
        memcpy(lockinfo.tickslocked,APPKEYLOCKTIMES,8);
        pcikeymanageauthen_cmd_synclockflag(groupindex);
    }

    memset((u8 *)&gSecurityAuthInfo,0,sizeof(SECURITYAUTHENTICATEINFO));
    
    ret = pcikeymanageauthen_applockinfo_save(groupindex,&lockinfo);
    return ret;
}


/*******************************************************************
Author:   sxl
Function Name:    s32 pcikeymanageauthen_authen_authenticatesuc(u8 groupindex)
Function Purpose: pos authenticate success
Input Paramters:  groupindex - the application key group index
Output Paramters:N/A
Return Value:      PCI_PROCESSCMD_SUCCESS  -      success
                         other - failed
Remark: N/A
Modify: N/A   1130   //20140327
********************************************************************/
s32 pcikeymanageauthen_authen_authenticatesuc(u8 groupindex)
{
    APPKEYLOCKINFO *lockinfo;
    s32 ret = PCI_PROCESSCMD_SUCCESS;
    
    if(groupindex >= PCIPOS_APPKEYGROUP_MAXNUM)
    {
        return PCI_KEYGROUPINDEX_ERR;
    }

    lockinfo = &gSecurityAuthInfo.appkeylockinfo;

    if(lockinfo->failedtimes)
    {
        lockinfo->failedtimes = 0;
        ret = pcikeymanageauthen_applockinfo_save(groupindex,lockinfo);
    }
    
    return ret;
}
/*******************************************************************
Author:   sxl
Function Name:    s32 pcikeymanageauthen_getpin_checkpininterval(u8 keytype,u8 groupindex)
Function Purpose: check the interval of use get pinblock function 
Input Paramters:  groupindex - the application key group index
Output Paramters:N/A
Return Value:      PCI_PROCESSCMD_SUCCESS  -      success
                         other - failed
Remark: N/A
Modify: N/A        1130  //20140327
********************************************************************/
s32 pcikeymanageauthen_getpin_checkpininterval(u8 keytype,u8 groupindex)
{
    //sxl?  是否要检查应用密钥是否被锁
    u32 tmpinterval;

	
	if(keytype == KEYTYPE_GETPIN_PINK)
	{
		tmpinterval = gPinKeyUseInternal.pinktimelastuse;
	}
	else if(keytype == KEYTYPE_GETPIN_FIXEDK)
	{
		tmpinterval = gPinKeyUseInternal.fixedktimelastuse;
	}
	else if(keytype == KEYTYPE_GETPIN_DUKPTK)
	{
		tmpinterval = gPinKeyUseInternal.dukptktimelatuse;
	}
	else
	{
		return PCI_GETPIN_INTERVALUNREACH;
	}
	 
    
    if(tmpinterval == 0)
    {
        return PCI_PROCESSCMD_SUCCESS;
    }
   
    
    return PCI_GETPIN_INTERVALUNREACH;

    
    
}


/*******************************************************************
Author:   sxl
Function Name:    s32 pcikeymanageauthen_authen_deleteallkey(void)
Function Purpose: check if can delete all key
Input Paramters:  cmpdata - compare data
Output Paramters:N/A
Return Value:      PCI_PROCESSCMD_SUCCESS  -      success
                         other - failed
Remark: N/A
Modify: N/A        1130   //20140327
********************************************************************/
s32 pcikeymanageauthen_authen_deleteallkey(u8 *cmpdata)
{
	
    #ifdef SXL_DEBUG
    u32 i;
	#endif

	
    if(gSecurityAuthInfo.effectivetime == 0)
    {
        return PCI_AUTHEN_NEED;
    }

	
	#ifdef SXL_DEBUG
    DDEBUG("\r\nPCIPOSAUTHEN_DOWNMAGKEY_REQUEST:\r\n");
	for(i = 0;i < 24;i++)
	{
		DDEBUG("%02x ",gSecurityAuthInfo.PTKey[i]);
	}
	DDEBUG("\r\n");
	for(i = 0;i < 24;i++)
	{
		DDEBUG("%02x ",cmpdata[i]);
	}
	DDEBUG("\r\n");
	for(i = 0;i < 24;i++)
	{
		DDEBUG("%02x ",gSecurityAuthInfo.TMacKey[i]);
	}
	DDEBUG("\r\n");
	for(i = 0;i < 24;i++)
	{
		DDEBUG("%02x ",cmpdata[i+24]);
	}
	DDEBUG("\r\n");
	#endif
    
	
	pcikeymanageauthen_managekey_transfer(cmpdata);  //sxl20160121
	
	
    
    if(Lib_memcmp(gSecurityAuthInfo.PTKey,cmpdata,24)||Lib_memcmp(gSecurityAuthInfo.TMacKey,&cmpdata[24],24))
    {
        return PCI_AUTHEN_NEED;
    }

    return PCI_PROCESSCMD_SUCCESS;
	
}


/*******************************************************************
Author:   sxl
Function Name:    s32 pcikeymanageauthen_posinsselect_authen(u32 authenstep,u8 *appinfo,u8 groupindex,u8 *data,s32 datalen)
Function Purpose: pos authentication 
Input Paramters:  authenstep  - authenticate step
                          *appinfo   - application information
                          groupindex          - app key group index
                          *data         - data need to processing
                          datalen      -  len of the data need to processing 
Output Paramters:N/A
Return Value:      PCI_PROCESSCMD_SUCCESS  -      success
                         other - failed
Remark: N/A
Modify: N/A  1130   //20140327
********************************************************************/
s32 pcikeymanageauthen_posinsselect_authen(u32 authenstep,u8 *appinfo,u8 groupindex,u8 *data,s32 datalen)
{
    s32 ret = PCI_PROCESSCMD_SUCCESS;
    u8 tmp[25];

    #ifdef PCIPOSAUTHEN
    if(authenstep == PCIPOSAUTHEN_DOWNAPPKEY_REQUEST)
    {
        ret = pcikeymanageauthen_downappkeyreq(appinfo,groupindex);
    }
    else if(authenstep == PCIPOSAUTHEN_DOWNAPPKEY_AUTH)
    {
    	
    	#ifdef SXL_DEBUG
        DDEBUG("\r\nPCIPOSAUTHEN_DOWNAPPKEY_AUTH11:\r\n");
		#endif
		
        memset(tmp,0,sizeof(tmp));
        if((Lib_memcmp(gSecurityAuthInfo.PTKey,tmp,25))&&(Lib_memcmp(gSecurityAuthInfo.TMacKey,tmp,25)))
        {
            gSecurityAuthInfo.IfCertified = 1;   //sxl? 测试用，要删除
            //sxl?  需要修改看是否超过30s  //sxl?  //开启定时器
            gSecurityAuthInfo.effectivetime = FUNCTIONUSEINTERVAL; // The effective time of 30s
            pcicorecallfunc.pci_dev_timer_user_open(&(gSecurityAuthInfo.effectivetime));
        }
        else
        {
            ret =  PCI_AUTHENPTK_ERR;
        }
        
        //sxl?  test要放开
        //ret = pcidrv_authen_downappkeyauthentication(groupindex,data,datalen);
    }
    else if(authenstep == PCIPOSAUTHEN_DOWNAPPKEY_CHECK)
    {
        ret = pcikeymanageauthen_authen_downappkeycheck(groupindex);
    }
    else if(authenstep == PCIPOSAUTHEN_DOWNMAGKEY_CHECK || authenstep == PCIPOSAUTHEN_DOWNPOSAUTHENKEY_CHECK)
    {
        ret = pcikeymanageauthen_authen_downmagkeycheck();
    }
    else if(authenstep == PCIPOSAUTHEN_SETWK_CHECK)
    {
        ret = pcikeymanageauthen_authen_setwkcheck(groupindex);
    }
    else if(authenstep == PCIPOSAUTHEN_DOWNAPPKEY_SAVE)
    {
        ret = pcikeymanageauthen_authen_saveappkeyheadinfo(groupindex);
    }
    else if(authenstep == PCIPOSAUTHEN_DOWNAPPKEY_CHECKFAIL)
    {
        ret = pcikeymanageauthen_authen_authenticatefail(groupindex);
    }
    else if(authenstep == PCIPOSAUTHEN_DOWNAPPKEY_CHECKSUC)
    {
        ret = pcikeymanageauthen_authen_authenticatesuc(groupindex);
    }
    else if(authenstep == PCIPOSAUTHEN_GETPIN_CHECKPININTERVAL)
    {
        if(data == NULL)
        {
            return PCI_GETPIN_INTERVALUNREACH;
        }
        ret = pcikeymanageauthen_getpin_checkpininterval(data[0],groupindex);
    }
    else if(authenstep == PCIPOSAUTHEN_DELETEALLKEY)  //sxl?20160121    这里需要修改
    {
        ret = pcikeymanageauthen_authen_deleteallkey(data);
    }
    #endif 
    
    
    #ifdef SXL_DEBUG
    DDEBUG("\r\npcidrv_posinsselect_authen ret = %d\r\n",ret);
    #endif
    
    return ret;
	
	
}



/*******************************************************************
Author:   sxl
Function Name:    s32 pcikeymanageauthen_posins_authen(u32 authenstep,u8 *appinfo,u8 groupindex,u8 *data,s32 datalen)
Function Purpose: key download authen
Input Paramters:  authenstep  - authenticate step
                          *appinfo   - application information
                          groupindex          - app key group index
                          *data         - data need to processing
                          datalen      -  len of the data need to processing 
Output Paramters:N/A
Return Value:      PCI_PROCESSCMD_SUCCESS  -      success
                         other - failed
Remark: N/A       sxl 1130
Modify: N/A     //20140327
********************************************************************/
s32 pcikeymanageauthen_posins_authen(u32 authenstep,u8 *appinfo,u8 groupindex,u8 *data,s32 datalen)
{
	
    //u8 appinfo[30];

	#ifdef PCIPOSAUTHEN
    s32 ret = PCI_PROCESSCMD_SUCCESS;

	#ifdef SXL_DEBUG
	u32 i;
    DDEBUG("\r\npcidrv_posins_authen:%d\r\n",authenstep);
	#endif

	//memset(appinfo,0,sizeof(appinfo));
	//strcpy(appinfo,"/mtd0/default");
	//strcpy(appinfo,appinfoname);
    
    //down app key authenticate data initial
    if(authenstep == PCIPOSAUTHEN_DOWNAPPKEY_REQUEST)
    {
        gSecurityAuthInfo.IfCertified = 0;        // Set Not Certified
        memcpy(gSecurityAuthInfo.PTKey,data,24);
        memcpy(gSecurityAuthInfo.TMacKey,&data[24],24);

        ret = pcikeymanageauthen_posinsselect_authen(authenstep,appinfo,groupindex,data,datalen);

		#ifdef SXL_DEBUG
        DDEBUG("\r\nPCIPOSAUTHEN_DOWNAPPKEY_REQUEST:\r\n");
		for(i = 0;i < 24;i++)
		{
			DDEBUG("%02x ",gSecurityAuthInfo.PTKey[i]);
		}
		DDEBUG("\r\n");
		for(i = 0;i < 24;i++)
		{
			DDEBUG("%02x ",gSecurityAuthInfo.TMacKey[i]);
		}
		DDEBUG("\r\n");
		#endif
		
    }
    else if(authenstep == PCIPOSAUTHEN_DOWNMAGKEY_REQUEST)
    {
        gSecurityAuthInfo.IfCertified = 0;        // Set Not Certified
        memcpy(gSecurityAuthInfo.PTKey,data,24);
        memcpy(gSecurityAuthInfo.TMacKey,&data[24],24);

		#ifdef SXL_DEBUG
        DDEBUG("\r\nPCIPOSAUTHEN_DOWNMAGKEY_REQUEST:\r\n");
		for(i = 0;i < 24;i++)
		{
			DDEBUG("%02x ",gSecurityAuthInfo.PTKey[i]);
		}
		DDEBUG("\r\n");
		for(i = 0;i < 24;i++)
		{
			DDEBUG("%02x ",gSecurityAuthInfo.TMacKey[i]);
		}
		DDEBUG("\r\n");
		#endif
    }
    //pos legitimacy authentication
    else if(authenstep == PCIPOSAUTHEN_DOWNAPPKEY_AUTH)
    {
        //PCI_AUTHENDOWNAPPKEY_FAILED
        #ifdef SXL_DEBUG
        DDEBUG("\r\nPCIPOSAUTHEN_DOWNAPPKEY_AUTH:\r\n");
		#endif
		
        ret = pcikeymanageauthen_posinsselect_authen(authenstep,appinfo,groupindex,data,datalen);
    }

    //check if authenticate
    else if(authenstep == PCIPOSAUTHEN_DOWNAPPKEY_CHECK || authenstep == PCIPOSAUTHEN_DOWNMAGKEY_CHECK || authenstep == PCIPOSAUTHEN_DOWNPOSAUTHENKEY_CHECK)
    {
        memcpy(data,gSecurityAuthInfo.PTKey,24);
        memcpy(&data[24],gSecurityAuthInfo.TMacKey,24);

		if(authenstep != PCIPOSAUTHEN_DOWNPOSAUTHENKEY_CHECK)
		{
			pcikeymanageauthen_managekey_transfer(data);
		}

        ret = pcikeymanageauthen_posinsselect_authen(authenstep,appinfo,groupindex,data,datalen);
    }
    
    //check if set key locked
    else if(authenstep == PCIPOSAUTHEN_SETWK_CHECK)
    {
        ret = pcikeymanageauthen_posinsselect_authen(authenstep,appinfo,groupindex,data,datalen);
    }

    //save 
    else if(authenstep == PCIPOSAUTHEN_DOWNAPPKEY_SAVE)
    {
        ret = pcikeymanageauthen_posinsselect_authen(authenstep,appinfo,groupindex,data,datalen);
    }

   // processing authenticate fai
   else if(authenstep == PCIPOSAUTHEN_DOWNAPPKEY_CHECKFAIL)
   {
        //pcikeymanageauthen_securityauthinfo_initial();  //initial security authenticate information
        ret = pcikeymanageauthen_posinsselect_authen(authenstep,appinfo,groupindex,data,datalen);
   }

   //processing authenticate success
    else if(authenstep == PCIPOSAUTHEN_DOWNAPPKEY_CHECKSUC)
    {
        ret = pcikeymanageauthen_posinsselect_authen(authenstep,appinfo,groupindex,data,datalen);
    }

    //
    else if(authenstep == PCIPOSAUTHEN_GETPIN_CHECKPININTERVAL)
    {
        ret = pcikeymanageauthen_posinsselect_authen(authenstep,appinfo,groupindex,data,datalen);
    }

    //delete all key
    else if(authenstep == PCIPOSAUTHEN_DELETEALLKEY)
    {
        ret = pcikeymanageauthen_posinsselect_authen(authenstep,appinfo,groupindex,data,datalen);
    }

    //initial security authentication information global variable if result is error
    //it is the security method and simple
    if(ret != PCI_PROCESSCMD_SUCCESS)
    {
        pcikeymanageauthen_securityauthinfo_initial();
    }
    
    #ifdef SXL_DEBUG
    DDEBUG("\r\npcidrv_posins_authen ret = %d\r\n",ret);
    #endif
    return ret;


	#else
	return PCI_PROCESSCMD_SUCCESS;
	#endif

	
	
}


/*******************************************************************
Author:   sxl
Function Name: s32 pcidrv_checkmaingroupfile(u8 groupindex)
Function Purpose:check if main group app key exist,if not exist create it
Input Paramters: groupindex - group index
Output Paramters:N/A
Return Value:    PCI_PROCESSCMD_SUCCESS  -      success
                       other - failed
Remark: N/A
Modify: N/A   //20140327
********************************************************************/
s32 pcikeymanageauthen_checkmaingroupfile(u8 groupindex)
{
    s32 ret;

    ret = pcikeymanage_readappkeyheadinfostatus(groupindex,NULL);
    DDEBUG("\r\n pcidrv_readappkeyheadinfostatus ret = %d \r\n", ret);
    if(ret == PCI_KEYFILENOTUSE)
    {
        //need to write ISSUER SN
        //sxl authen
        ret = pcikeymanageauthen_posins_authen(PCIPOSAUTHEN_DOWNAPPKEY_SAVE,NULL,groupindex,0,0);
        DDEBUG("\r\n%s11 = %d \r\n",__FUNCTION__,ret);
        if(ret != PCI_PROCESSCMD_SUCCESS)
        {
            return ret;
        }
    }
    else if(ret == PCI_PROCESSCMD_SUCCESS)
    {
        
    }
    else
    {
        return ret;
    }
    
    return ret;
	
	
}



/*******************************************************************
Author:   sxl
Function Name:    s32 pcikeymanageauthen_savegroupkeyappname(u8 groupindex,u8 *appname)
Function Purpose: save groupkey appname
Input Paramters:  
                          groupindex          - app key group index
                           *appname   - application name
Output Paramters:N/A
Return Value:      PCI_PROCESSCMD_SUCCESS  -      success
                         other - failed
Remark: N/A
Modify: N/A     sxl 1130  //20140327
********************************************************************/
s32 pcikeymanageauthen_savegroupkeyappname(u8 groupindex,u8 *appname)
{
    s8 keyfilepath[128];
    s32 ret;
    u8 tmp[AUTHENTICATION_APPKEYHEAD_SIZE];
    s32 addr;
    //u8 certdata[24];
    u8 tmpCAK[DRV_PCI_CAK_LEN];
    u8 tmphash[32];
    
    if(gSecurityAuthInfo.appkeylockinfo.appkeyindex != (groupindex+1))
    {
        DDEBUG("%s:app key group index is different !\r\n",__FUNCTION__);
        return PCI_KEYGROUPINDEX_ERR;
    }

    memset(keyfilepath,0,sizeof(keyfilepath));
    ret = pcikeymanage_getappkeyfileabsolutepath(groupindex,keyfilepath);
    if(ret != PCI_PROCESSCMD_SUCCESS)
    {
        return ret;
    }

    ret = pcicorecallfunc.pcidrv_readCAK(tmpCAK);
    if(ret != PCI_PROCESSCMD_SUCCESS)
    {
        DDEBUG("%s:read cak fail \r\n",__FUNCTION__);
        return ret;
    }
    memset(tmp,0,sizeof(tmp));
    
    ret = pcicorecallfunc.pci_kern_read_file(keyfilepath,tmp,AUTHENTICATION_APPKEYHEAD_SIZE,0);
    if(ret != AUTHENTICATION_APPKEYHEAD_SIZE)
    {
        DDEBUG("\r\n%s read file error\r\n",__FUNCTION__);
        memset(tmpCAK,0,sizeof(tmpCAK));
        return PCI_DEALPCIFILE_ERR;
    }
    memset(tmphash,0,sizeof(tmphash));
    pciarith_hash(tmp,AUTHENTICATION_APPKEYNAMEHEADINFO_SIZE,tmphash,HASHTYPE_256);
    ret = drv_tdes(ENCRYPT,tmphash,32,tmpCAK,DRV_PCI_CAK_LEN);
    if(ret != 0)
    {
        memset(tmpCAK,0,sizeof(tmpCAK));
        return PCI_DEAL_ERR;
    }
    if(Lib_memcmp(tmphash,&tmp[AUTHENTICATION_APPKEYNAMEHEADINFO_SIZE],32))
    {
        DDEBUG("\r\n%s:hash compare err!\r\n",__FUNCTION__);
        memset(tmpCAK,0,sizeof(tmpCAK));
        pcikeymanage_groupkeyfile_initial(groupindex);
        return PCI_KEYFILENOTUSE;
    }
    
    //memset(certdata,0,sizeof(certdata));
    memcpy(&tmp[AUTHENTICATION_APPKEYHEADINFO_SIZE],appname,64);
    memset(&tmp[AUTHENTICATION_APPKEYNAMEHEADINFO_SIZE],0,32);
    pciarith_hash(tmp,AUTHENTICATION_APPKEYNAMEHEADINFO_SIZE,&tmp[AUTHENTICATION_APPKEYNAMEHEADINFO_SIZE],HASHTYPE_256);
    
    ret = drv_tdes(ENCRYPT,&tmp[AUTHENTICATION_APPKEYNAMEHEADINFO_SIZE],32,tmpCAK,DRV_PCI_CAK_LEN);
    memset(tmpCAK,0,sizeof(tmpCAK));
    if(ret != 0)
    {
        return PCI_DEAL_ERR;
    }
    
    addr = 0;
    ret = pcicorecallfunc.pci_kern_insert_file(keyfilepath,tmp,AUTHENTICATION_APPKEYHEAD_SIZE,addr);
    if(ret != PCI_PROCESSCMD_SUCCESS)
    {
        //DDEBUG("\r\nwrite file error\r\n");
        return PCI_DEALPCIFILE_ERR;
    }

    return PCI_PROCESSCMD_SUCCESS;
    
}


/*******************************************************************
Author:   sxl
Function Name:    void pcikeymanageauthen_getpin_updatelastusetime(u8 keytype)
Function Purpose: 
Input Paramters:  key type  - the key type of function getpin use
Output Paramters:N/A
Return Value:      PCI_PROCESSCMD_SUCCESS  -      success
                         other - failed
Remark: N/A
Modify: N/A      1130   //20140327
********************************************************************/
void pcikeymanageauthen_getpin_updatelastusetime(u8 keytype)
{
    if(keytype == KEYTYPE_GETPIN_PINK)
    {
        gPinKeyUseInternal.pinktimelastuse = FUNCTIONUSEINTERVAL;
        pcicorecallfunc.pci_dev_timer_user_open(&(gPinKeyUseInternal.pinktimelastuse ));
    }
    else if(keytype == KEYTYPE_GETPIN_FIXEDK)
    {
        gPinKeyUseInternal.fixedktimelastuse = FUNCTIONUSEINTERVAL;
        pcicorecallfunc.pci_dev_timer_user_open(&(gPinKeyUseInternal.fixedktimelastuse ));
    }
    else if(keytype == KEYTYPE_GETPIN_DUKPTK)
    {
        gPinKeyUseInternal.dukptktimelatuse = FUNCTIONUSEINTERVAL;
        pcicorecallfunc.pci_dev_timer_user_open(&(gPinKeyUseInternal.dukptktimelatuse ));
		
    }
	
	
}



/*******************************************************************
Author:   sxl
Function Name:   s32 pcikeymanageauthen_sensitiveservicheck(u8 groupindex,u8 *appname)
Function Purpose:  sentive service check
Input Paramters:   groupindex          - app key group index
                           appname             - application name
Output Paramters:N/A
Return Value:      PCI_PROCESSCMD_SUCCESS  -      success
                         other - failed
Remark: N/A       sxl 1230
Modify: N/A         //20140327
********************************************************************/
s32 pcikeymanageauthen_sensitiveservicheck(u8 groupindex,u8 *appname)
{
    u8 groupkeyappname[100];
	APPKEYLOCKINFO lockinfo;
    //s32 ret = PCI_PROCESSCMD_SUCCESS;
	//u8 failedtimes;
	
	if(groupindex >= PCIPOS_APPKEYGROUP_MAXNUM)
	{
		return PCI_KEYGROUPINDEX_ERR;
	}

    memset(groupkeyappname,0,sizeof(groupkeyappname));
	pcikeymanageauthen_readappkeyheadinfoappname(groupindex,groupkeyappname);
	#ifdef SXL_DEBUG
    //DDEBUG("\r\n%s:%s %s\r\n",__FUNCTION__,appname,groupkeyappname);
	#endif
    if(strlen((char *)groupkeyappname) == 0)
    {
        DDEBUG("\r\ngroupappname not exist\r\n");
    	return PCI_PROCESSCMD_SUCCESS;
    }

	
	if(appname != NULL)
	{
		if(strcmp((s8 *)groupkeyappname,(s8 *)appname))
		{
			return PCI_KEYAPPNAMENOTMATCH;
		}
	}
	
    pcikeymanageauthen_appkeylockfile_read(groupindex,&lockinfo);
	if(lockinfo.failedtimes >= MAXFAILEDTIMES)
	{
		return PCI_AUTHENAPPKEYLOCK_ERR;
	}
	
	return PCI_PROCESSCMD_SUCCESS;
	
}

/*******************************************************************
Author:   sxl
Function Name:    void pcikeymanageauthen_cmd_decreaselocktimer(void)
Function Purpose: decrease lock timer
Input Paramters:  NULL 
Output Paramters:N/A
Return Value:       N/A
Remark: N/A
Modify: N/A   1130   //20140327
********************************************************************/
void pcikeymanageauthen_cmd_decreaselocktimer(void) //sxl 不能放到中断里去处理
{
    u8 groupindex;
    s32 ret;
    APPKEYLOCKINFO lockinfo;
    u32 key1ockbits;
    u8 tmp[8];
    
    if(timerstartflag == 0)
    {
        timerstartflag = 1;
        appkeylockbits = 0;
        key1ockbits = 1;
        for(groupindex = 0;groupindex < PCIPOS_APPKEYGROUP_MAXNUM;groupindex++)
        {
            ret = pcikeymanageauthen_appkeylockfile_read(groupindex,&lockinfo);
            if(ret == PCI_PROCESSCMD_SUCCESS)
            {
                //app key is locked
                if(lockinfo.failedtimes >= MAXFAILEDTIMES)
                {
                    appkeylockbits |= key1ockbits;
                }
            }
            key1ockbits = key1ockbits << 1;
        }

        appkeylocktimer = PCIMINTIMERCHECK;
        pcicorecallfunc.pci_dev_timer_user_open(&appkeylocktimer);
    }

    //DDEBUG("\r\n pcidrv_cmd_decreaselocktimer \r\n");

    if(appkeylocktimer == 0)
    {
        appkeylocktimer = PCIMINTIMERCHECK;
        pcicorecallfunc.pci_dev_timer_user_open(&appkeylocktimer);
        
        if(appkeylockbits)
        {
            key1ockbits = 1;
            memset(tmp,0,sizeof(tmp));
            for(groupindex = 0;groupindex < PCIPOS_APPKEYGROUP_MAXNUM;groupindex++)
            {
                DDEBUG("\r\n %08x %08x %02x \r\n",appkeylockbits,key1ockbits,groupindex);
                if(appkeylockbits&key1ockbits)
                {
                    ret = pcikeymanageauthen_appkeylockfile_read(groupindex,&lockinfo);
                    if(ret == PCI_PROCESSCMD_SUCCESS)
                    {
                        //app key is locked
                        if(lockinfo.failedtimes >= MAXFAILEDTIMES)
                        {
                            if(Lib_memcmp(lockinfo.tickslocked,tmp,8))
                            {
                                pciarith_BcdSub(lockinfo.tickslocked,"\x00\x00\x00\x00\x00\x00\x00\x01",8);
                                if(Lib_memcmp(lockinfo.tickslocked,tmp,8) == 0)
                                {
                                    lockinfo.failedtimes = 0;
                                }
                            }
                            else
                            {
                                lockinfo.failedtimes = 0;
                            }
                            ret = pcikeymanageauthen_applockinfo_save(groupindex,&lockinfo);
                            if(ret == PCI_PROCESSCMD_SUCCESS)
                            {
                                if(lockinfo.failedtimes == 0)
                                {
                                    appkeylockbits &= (~(key1ockbits));
                                }
                            }
                        }
                        else
                        {
                            appkeylockbits &= (~(key1ockbits));
                        }

                        DDEBUG("\r\nlock info:%02x %02x %02x  \r\n",groupindex,lockinfo.tickslocked[6],lockinfo.tickslocked[7]);
                    }
                }
                key1ockbits = key1ockbits<<1;
            }
        }
    }
}



void pcikeymanageauthen_managekey_transfer(u8 *tempptk)
{
	u8 i;

	for(i = 0; i < 24;i++)
	{
		tempptk[i] ^= gPciManagerInfo.admin1key[i];
		tempptk[i] ^= gPciManagerInfo.admin2key[i];
	}

	#ifdef SXL_DEBUG
    DDEBUG("\r\nadmin1key:\r\n");
	for(i = 0;i < 24;i++)
	{
		DDEBUG("%02x ",gPciManagerInfo.admin1key[i]);
	}
	DDEBUG("\r\n");
	
	
	
	DDEBUG("\r\nadmin2key:\r\n");
	for(i = 0;i < 24;i++)
	{
		DDEBUG("%02x ",gPciManagerInfo.admin1key[i]);
	}
	DDEBUG("\r\n");


	
	DDEBUG("\r\ntempptk:\r\n");
	for(i = 0;i < 24;i++)
	{
		DDEBUG("%02x ",tempptk[i]);
	}
	DDEBUG("\r\n");
	#endif
	
}


s32 pcikeymanageauthen_savepcimanagefile_file(PCIMANAGE *temppcimanagefile)
{
	
	//u8 tempdata[100];
	u32 crc;
	s32 ret;
    
    memcpy(temppcimanagefile->tag,"\x55\xAA\x55\xAA",4);
	crc = 0xffffffff;
	crc = GetHash(crc,temppcimanagefile->tag,sizeof(PCIMANAGE)-4);
	

	LongToByte(crc,temppcimanagefile->crc);
	pcicorecallfunc.pcidrv_processusedCAK(ENCRYPT,temppcimanagefile->admin1key,24);
	pcicorecallfunc.pcidrv_processusedCAK(ENCRYPT,temppcimanagefile->admin2key,24);
	ret = pcicorecallfunc.pci_kern_write_file(PCIMANAGEINFOFILE,temppcimanagefile->tag,sizeof(PCIMANAGE));
	if(ret != 0)
	{
	    #ifdef SXL_DEBUG
		DDEBUG("\r\npcidrv_savepcimanagefile fail\r\n");
		#endif
		return 1;
	}
	
	
    #ifdef SXL_DEBUG
	DDEBUG("\r\npcidrv_savepcimanagefile suc\r\n");
	#endif


	//pcidrv_readpcimanageinfo();   //sxl?
	
	return 0;

	
}




s32 pcikeymanageauthen_savepcimanagefile(u8 adminno,u8 *adminkey)
{
	
    if(adminno!=0&&adminno!=1)
    {
    	return 1;
    }
	
	memcpy(gPciManagerInfo.admin1key,adminkey,24);   //使用MMK ELRCK加密存放  //sxl?需要进行修改
	
	return pcikeymanageauthen_savepcimanagefile_file(&gPciManagerInfo);
	
	
}



s32 pcikeymanageauthen_readpcimanagefile(PCIMANAGE *temppcimanagefile)
{
	
	u8 tempdata[100];
	s32 ret;
	u32 crc,tmpcrc;


	memset(tempdata,0,sizeof(PCIMANAGE));
	ret = pcicorecallfunc.pci_kern_read_file(PCIMANAGEINFOFILE,tempdata,sizeof(PCIMANAGE),0);
    if(ret != sizeof(PCIMANAGE))
    {
        #ifdef SXL_DEBUG
		DDEBUG("\r\npcidrv_readpcimanagefile read file err\r\n");
		#endif
    	return 1;
    }

	if(memcmp(tempdata,"\x55\xAA\x55\xAA",4))
	{
	    #ifdef SXL_DEBUG
		DDEBUG("\r\npcidrv_readpcimanagefile tag err\r\n");
		#endif
		return 1;
	}

	pcicorecallfunc.pcidrv_processusedCAK(DECRYPT,temppcimanagefile->admin1key,24);
	pcicorecallfunc.pcidrv_processusedCAK(DECRYPT,temppcimanagefile->admin2key,24);
	
	
    crc = 0xffffffff;
	crc = GetHash(crc,tempdata,sizeof(PCIMANAGE)-4);
	tmpcrc = Long2Byte(&tempdata[sizeof(PCIMANAGE)-4]);
	
	if(tmpcrc != crc)
	{
	    #ifdef SXL_DEBUG
		DDEBUG("\r\npcidrv_readpcimanagefile crc err\r\n");
		#endif
		return 1;
	}
	
	memcpy((u8 *)temppcimanagefile,tempdata,sizeof(PCIMANAGE));
	
	
	#ifdef SXL_DEBUG
	DDEBUG("\r\npcidrv_readpcimanagefile suc %08x %08x!\r\n",tmpcrc,crc);
	#endif
	
	
	return 0;
	
	
	
}



void pcikeymanageauthen_readpcimanageinfo(void)
{
	memset(&gPciManagerInfo,0,sizeof(PCIMANAGE));
    pcikeymanageauthen_readpcimanagefile(&gPciManagerInfo);
	
}



#else

void pcikeymanageauthen_initialuseinternal(void)
{
	
}

void pcikeymanageauthen_securityauthinfo_initial(void)
{
	
}

s32 pcikeymanageauthen_posins_authen(u32 authenstep,u8 *appinfo,u8 groupindex,u8 *data,s32 datalen)
{
	return 0;
}

s32 pcikeymanageauthen_applockfile_initial(void)
{
	return 0;
}

#endif


