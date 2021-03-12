

#include "pciglobal.h"


void pcikeymanage_CalculateLRCCheckvalue(PCIAPPKEY *appkey,u8 *elrckdata)
{
	u8 k;
	
	k = pciarithgetlrc(appkey->keydata,24);
	elrckdata[0] = k;
	elrckdata[1] = appkey->keytype;
	elrckdata[2] = appkey->keyindex;
	elrckdata[3] = appkey->keylen;
	elrckdata[4] = 0;
	elrckdata[5] = 0;
	elrckdata[6] = 0;
	elrckdata[7] = 0;
	
}


u8 pcikeymanage_getkeyarea(u8 keytype,s32 *keyaddr,s32 *maxkeyindexret)
{
    s32 addr,maxkeyindex;
    u8 nextkeytype;
	
	switch(keytype)
	{
		case TYPE_TMK:
            addr = TMK_ADDROFFSET;
			maxkeyindex = TMK_MAXINDEX;
			nextkeytype = TYPE_PINK;
			break;
		case TYPE_PINK:
            addr = PINK_ADDROFFSET;
			maxkeyindex = PINK_MAXINDEX;
			nextkeytype = TYPE_MACK;
			break;
		case TYPE_MACK:
            addr = MACK_ADDROFFSET;
			maxkeyindex = MACK_MAXINDEX;
			nextkeytype = TYPE_TDK;
			break;
		case TYPE_TDK:
            addr = TDK_ADDROFFSET;
			maxkeyindex = TDK_MAXINDEX;
			nextkeytype = TYPE_FIXEDK;
			break;
		case TYPE_FIXEDK:
            addr = FIXEDK_ADDROFFSET;
			maxkeyindex = FIXEDK_MAXINDEX;
			nextkeytype = TYPE_BPK;
			break;
		case TYPE_BPK:
            addr = BPK_ADDROFFSET;
			maxkeyindex = BPK_MAXINDEX;
			nextkeytype = TYPE_KEYRESERVED;
			break;
		case TYPE_KEYRESERVED:
            addr = KEYRESERVED_ADDROFFSET;
			maxkeyindex = KEYRESERVED_MAXINDEX;
			nextkeytype = TYPE_NULL;
			break;
		default:
			addr = -1;
			maxkeyindex = 0;
			nextkeytype = TYPE_NULL;
			break;
	}

	*keyaddr = addr;
	*maxkeyindexret = maxkeyindex;

	return nextkeytype;
	
}
/*******************************************************************
Author:   sxl
Function Name: s32 pcikeymanage_keyprocessusingMMK(u8 encryptmode,u8 *keydata,s32 keylen)
Function Purpose:Processing key using MMK
Input Paramters: 
                       encryptmode - ENCRYPT or DECRYPT
                       keydata        - keydata to encrypted or decrypted
Output Paramters:N/A
Return Value:    PCI_PROCESSCMD_SUCCESS  -      success
                       other - failed
Remark: N/A
Modify: N/A 		//20140327
********************************************************************/
s32 pcikeymanage_keyprocessusingMMK(u8 encryptmode,u8 *keydata,s32 keylen)
{
    s32 ret;
	u8 tmp[64];
	s32 i;
    s32 j;
    
	if(keylen &0x07)
	{
		return PCI_DATALEN_ERR;
	}

	
	for(i =0;i < 2;i++)
	{
		ret = pcitampermanage_readMMK(tmp,&j);
		if(ret != PCI_PROCESSCMD_SUCCESS)
		{
			
			if(ret == PCI_MMKNOTINITIAL)
			{
				Lib_memset(tmp,0,sizeof(tmp));
				return ret;
			}
			ret = pcitamper_manage_mmkbakrecover(tmp);
			if(ret != PCI_PROCESSCMD_SUCCESS)
			{
			    Lib_memset(tmp,0,sizeof(tmp));
				return ret;
			}
		}
        else
        {
            break;
        }
	}

	
	if(ret != PCI_PROCESSCMD_SUCCESS)
	{
		Lib_memset(tmp,0,sizeof(tmp));
		return ret;
	}

	
	if(drv_tdes(encryptmode,keydata,keylen,tmp,8) < 0)  //sxl?  16
	{
	    Lib_memset(tmp,0,sizeof(tmp));
		return PCI_DATALEN_ERR;
	}
	
    Lib_memset(tmp,0,sizeof(tmp));
	return PCI_PROCESSCMD_SUCCESS;
	
}


/*******************************************************************
Author:   sxl
Function Name: s32 pcikeymanage_keyprocessusingELRCK(u8 *checkvalue,s32 checkvaluelen)
Function Purpose:Processing key saved struct data using ELRCK
Input Paramters: 
                       checkvalue        - check value data
                       checkvaluelen    - check value data len
Output Paramters:checkvalue     - check value encrypted by ELRCK
Return Value:    PCI_PROCESSCMD_SUCCESS  -      success
                       other - failed
Remark: N/A
Modify: N/A 	//20140327	//20140327
********************************************************************/
s32 pcikeymanage_keyprocessusingELRCK(u8 *checkvalue,s32 checkvaluelen)
{
    
    s32 ret;
	u8 tmp[64];
	s32 i;

    
	if(checkvaluelen &0x07)
	{
		return PCI_DATALEN_ERR;
	}

    for(i =0;i < 2;i++)
	{
		ret = pcitampermanage_readELRCK(tmp,&i);
		if(ret != PCI_PROCESSCMD_SUCCESS)
		{
			if(ret == PCI_MMKNOTINITIAL)
			{
				Lib_memset(tmp,0,sizeof(tmp));
				return ret;
			}
			ret = pcitamper_manage_mmkbakrecover(tmp);
			if(ret != PCI_PROCESSCMD_SUCCESS)
			{
			    Lib_memset(tmp,0,sizeof(tmp));
				return ret;
			}
			
		}
	}

	
	if(ret != PCI_PROCESSCMD_SUCCESS)
	{
		Lib_memset(tmp,0,sizeof(tmp));
		return ret;
	}

	
	if(drv_tdes(ENCRYPT,checkvalue,checkvaluelen,tmp,16) < 0)
	{
	    Lib_memset(tmp,0,sizeof(tmp));
		return PCI_DATALEN_ERR;
	}
    
    Lib_memset(tmp,0,sizeof(tmp));
    
	return PCI_PROCESSCMD_SUCCESS;
	
}

/*******************************************************************
Author:   sxl
Function Name: s32 pcidrv_getappkeyfile_absolutepath(u8 groupindex,s8 *absolutepath)
Function Purpose: get key data file absolutepath using app name
Input Paramters: 
                         groupindex      - application key group index
Output Paramters:*absolutepath        - absolute path
Return Value:    PCI_PROCESSCMD_SUCCESS  -      success
                       other - failed
Remark: N/A     sxl 1130
Modify: N/A 		//20140327
********************************************************************/
s32 pcikeymanage_getappkeyfileabsolutepath(u8 groupindex,s8 *absolutepath)
{
	s32 len;
	s32 i;
	u8 tmp[5];

	if(groupindex >= PCIPOS_APPKEYGROUP_MAXNUM)
	{
		return PCI_KEYGROUPINDEX_ERR;
	}
    memset(tmp,0,sizeof(tmp));
    tmp[0] = (groupindex/10) + 0x30;
	tmp[1] = (groupindex%10) + 0x30;
	len = 2;
	strcpy((char *)absolutepath,DRV_PCI_APPKEY_PATH);
	i = strlen((char *)DRV_PCI_APPKEY_PATH);
    memcpy(&absolutepath[i], tmp, len); 
    i += len;
	absolutepath[i] = 0;
	strcat((char *)absolutepath,DRV_PCI_APPKEY_SUFFIX);

	return PCI_PROCESSCMD_SUCCESS;
}


/*******************************************************************
Author:   sxl
Function Name: s32 pcikeymanage_checkappkey(PCIAPPKEY *appkey,u8 keytype,u8 keyindex)
Function Purpose: check app key format
Input Paramters:  appkey  -  key data
			     keytype  -  key type
			     keyindex -  key index
Output Paramters:
Return Value:    PCI_PROCESSCMD_SUCCESS  -      success
                       other - failed
Remark: N/A     sxl 1130
Modify: N/A 		//20140327
********************************************************************/
s32 pcikeymanage_checkappkey(PCIAPPKEY *appkey,u8 keytype,u8 keyindex)
{
	
	u8 tmpcmpdata[32];
	u8 elrckdata[8];
	s32 ret;
	#ifdef SXL_DEBUG
    u8 k;
	#endif
	
	memset(tmpcmpdata,0xff,sizeof(tmpcmpdata));
	//sxlremark
	//该索引位置没有保存密钥
	//no key saved at this key index
	if(Lib_memcmp(tmpcmpdata,(u8 *)appkey,sizeof(tmpcmpdata)) == 0)
	{
	    return PCI_KEYEXIST_ERR;
	}

	DDEBUG("\r\n%s 1111 %02x %02x\r\n",__FUNCTION__,appkey->keytype,keytype);
    //sxlremark
    //密钥类型不符
    //key type does not match
	if(appkey->keytype != keytype)
	{
		return PCI_KEYTYPE_ERR;
	}
	//DDEBUG("\r\n %s 1112 \r\n",__FUNCITON__);
	//sxlremark
	//密钥索引号不符
	//key index does not match
	if(appkey->keyindex != keyindex)
	{
		DDEBUG("\r\n %s key index error \r\n",__FUNCTION__);
		return PCI_KEYINDEXMATCH_ERR;
	}
	DDEBUG("\r\n %s 1113 \r\n",__FUNCTION__);
	//sxlremark
	//密钥长度不符
	//ley length does not match
	if((appkey->keylen != 8)&&(appkey->keylen != 16)&&(appkey->keylen != 24))
	{
		return PCI_KEYLEN_ERR;
	}
    DDEBUG("\r\n %s 1114 \r\n",__FUNCTION__);
    //sxlremark
	//计算密钥数据MAC
	//key data MAC calculated
	pcikeymanage_CalculateLRCCheckvalue(appkey,elrckdata);
    
	//sxlremark
	//使用ELRCK对密钥 MAC进行加密
	//use ELRCK to encrypt the key MAC
	ret = pcikeymanage_keyprocessusingELRCK(elrckdata,8);
	if(ret != PCI_PROCESSCMD_SUCCESS)
	{
		return ret;
	}
	DDEBUG("\r\n %s 1115 \r\n",__FUNCTION__);

    //sxlremark
    //比较MAC是否一致
    //Compare if  the MACvalue  are the same
	if(Lib_memcmp(appkey->keylrc,elrckdata,5))
	{
		return PCI_KEYLRC_ERR;
	}
	DDEBUG("\r\n %s 1116 \r\n",__FUNCTION__);
    
	//sxlremark
	//使用MMK解析密钥数据是否正确
	//Analysis if decrypted data using MMK is correct
	ret = pcikeymanage_keyprocessusingMMK(DECRYPT,appkey->keydata,24);
	if(ret != PCI_PROCESSCMD_SUCCESS)
	{
		return ret;
	}
	DDEBUG("\r\n%s 1117 \r\n",__FUNCTION__);
	
	
	#ifdef SXL_DEBUG
    DDEBUG("\r\nget app key:%02x\r\n",appkey->keytype);
    for(k = 0;k<appkey->keylen;k++)
    {
    	DDEBUG("%02x",appkey->keydata[k]);
    }
	DDEBUG("\r\n");
	#endif
	
	return PCI_PROCESSCMD_SUCCESS;
	
}


/*******************************************************************
Author:   sxl
Function Name:    s32 pcidrv_readappkeyheadinfostatus(u8 groupindex,u8 *issuerSN)
Function Purpose: read key file head information status 
Input Paramters:  s32 pcidrv_readappkeyheadinfostatus(u8 groupindex)
Output Paramters:*issuerSN  - the group key issuer and SN
Return Value:      PCI_PROCESSCMD_SUCCESS  -      success
                         other - failed
Remark: N/A
Modify: N/A        sxl //20140327
********************************************************************/
s32 pcikeymanage_readappkeyheadinfostatus(u8 groupindex,u8 *headinfo)
{
    #ifdef PCIPOSAUTHEN
    u8 tmpCAK[DRV_PCI_CAK_LEN];
    s8 keyfilepath[128];
    s32 ret;
    u8 tmp[AUTHENTICATION_APPKEYHEAD_SIZE];
    u8 tmphash[32];
    u8 tmpcmp[AUTHENTICATION_APPKEYHEAD_SIZE];
    
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
    
    DDEBUG("\r\npcidrv_readappkeyheadinfostatus:%s\r\n",keyfilepath);
    
    
    ret = pcicorecallfunc.pci_kern_read_file(keyfilepath, tmp, AUTHENTICATION_APPKEYHEAD_SIZE, 0);
    //sxlremark
    //读取文件失败
    //Read file fail
    DDEBUG("\r\npcidrv_readappkeyheadinfostatus2 %d\r\n",ret);
    if(ret != AUTHENTICATION_APPKEYHEAD_SIZE)
    {
        return PCI_DEALPCIFILE_ERR;
    }
    
    DDEBUG("\r\npcidrv_readappkeyheadinfostatus3\r\n");
    memset(tmpcmp,0xff,sizeof(tmpcmp));
    //sxlremark
    //该组密钥还没有使用过
    //The group key has not been used
    if(Lib_memcmp(tmp,tmpcmp,AUTHENTICATION_APPKEYHEAD_SIZE) == 0)
    {
       return PCI_KEYFILENOTUSE;
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
        return PCI_KEYFILENOTUSE;
    }
	
	if(headinfo != NULL)
	{
		//sxlremark
	    //返回该组密钥文件使用的公钥证书标识
	    //return the public key certificate used by the group key file
	    memcpy(headinfo,tmp,AUTHENTICATION_APPKEYHEADINFO_SIZE);
	}
    
    return PCI_PROCESSCMD_SUCCESS;

	#else
    return PCI_PROCESSCMD_SUCCESS;
	#endif
	
	
}

/*******************************************************************
Author:   sxl
Function Name: s32 pcikeymanage_checkifkeyisthsame(PCIAPPKEY *cmpkey,PCIAPPKEY *curkey)
Function Purpose:check if the key data is the same
Input Paramters: N/A
Output Paramters:N/A
Return Value:  N/A
Remark: N/A
Modify: N/A   //20140327
********************************************************************/
s32 pcikeymanage_checkifkeyisthsame(PCIAPPKEY *cmpkey,PCIAPPKEY *curkey,u8 cmpkeygroupindex,u8 curkeygroupindex)
{
	u8 keysameflag = 0,j;
	
	if((cmpkey->keytype != curkey->keytype)||(cmpkey->keyindex != curkey->keyindex)||(cmpkeygroupindex != curkeygroupindex))
	{
		keysameflag = 1;
        for(j = 0;j<24;j++)
        {
        	if((curkey->keydata[3+j]&0xfe) != (cmpkey->keydata[j]&0xfe))
        	{
        		keysameflag = 0;
				break;
        	}
        }
	}

	return keysameflag;
}

/*******************************************************************
Author:   sxl
Function Name: s32 pcikeymanage_checkallappkey(u8 cmpkeygroupindex,PCIAPPKEY *cmpkey)
Function Purpose:check all app key
Input Paramters: N/A
Output Paramters:N/A
Return Value:  N/A
Remark: N/A
Modify: N/A   //20140327
********************************************************************/
s32 pcikeymanage_checkallappkey(u8 cmpkeygroupindex,PCIAPPKEY *cmpkey)
{
	#if 1  //def CLIENTCUSTIMIZEKEY 
    	return PCI_PROCESSCMD_SUCCESS;   //add by 2012-04-07
	#else
    s8 keyfilepath[20];
	s32 ret;
    s32 addr = 0,maxkeyindex;
	PCIAPPKEY tmpappkey;
	u8 keytype,nextkeytype;
	u8 groupindex;
	u32 keyindex;
	u32 currentreadnum,i,readlength;
	u8 *tmpkeydata;//[4096];
    u32 maxreadnum;
	u32 tmpaddr;
	u32 memlen = 0;

	
    //sxlremark
	//为了加快读取速度，,使用一个缓存来一次读取多个密钥,计算每次可以读取的密钥的最多个数
	//To speed up the read speed, using a cache to a read more than one key, you can count the maximum number of keys to read
    memlen = 2048;
	maxreadnum = memlen/sizeof(PCIAPPKEY);
	DDEBUG("\r\n maxreadnum = %d \r\n",maxreadnum);

    tmpkeydata = (u8 *)pcicorecallfunc.mem_malloc(memlen);
	if(tmpkeydata == NULL)
	{
		while(1)
		{
			#ifdef SXL_DEBUG
            DDEBUG("\r\n kmalloc memory error \r\n");
			#endif
		}
	}
	
	//sxlremark
	//从第0组开始检查，一直到(PCIPOS_APPKEYGROUP_MAXNUM-1)组
	//Start checking from the group 0, until (PCIPOS_APPKEYGROUP_MAXNUM-1) group
	for(groupindex = 0;groupindex < PCIPOS_APPKEYGROUP_MAXNUM;groupindex++)
	{
	    //sxlremark
	    //从TK_PIN密钥开始检查
	    //Start checking from the TK_PIN key
	    keytype = TYPE_TMK; 
		//sxlremark
		//获取密钥文件名称
		//get the key file name
	    memset(keyfilepath,0,sizeof(keyfilepath));
		ret = pcikeymanage_getappkeyfileabsolutepath(groupindex,keyfilepath);
		if(ret != PCI_PROCESSCMD_SUCCESS)
		{
		    pcicorecallfunc.mem_free(tmpkeydata);
			return ret;
		}
		
		
		if(pcikeymanage_readappkeyheadinfostatus(groupindex,NULL) == PCI_KEYFILENOTUSE)
		{
            #ifdef SXL_DEBUG
            DDEBUG("\r\n %d groupindex key not use!\r\n",groupindex);
			#endif
			continue;
		}
		
		
		
		while(keytype != TYPE_NULL)
		{

		    #ifdef SXL_DEBUG
            DDEBUG("\r\n keytype = %02x \r\n",keytype);
			#endif
			nextkeytype = pcikeymanage_getkeyarea(keytype,&addr,&maxkeyindex);
			
			//sxlremark
			//addr中记录该类型密钥开始的地址,maxkeyindex记录着该类型密钥的最大索引号,从索引0开始读
			//start  address of the type key recorded in the addr, maxkeyindex recorded the biggest index number  of the type of key, starting Reading from index 0 
            tmpaddr = addr;
	        for(keyindex = 0;keyindex < maxkeyindex;)
	        {
			    addr = tmpaddr + keyindex*PCIAPPKEYSIZE;
				//sxlremark
				//计算一次读取的密钥的个数
				//Calculate the number of key to read once
                if((maxreadnum+keyindex) <= maxkeyindex)
                {
                	currentreadnum = maxreadnum;
                }
				else
				{
					currentreadnum = maxkeyindex - keyindex;
				}

				readlength = currentreadnum*PCIAPPKEYSIZE;
				//sxlremark
			    //读取密钥数据
			    //read key data
				ret = pcicorecallfunc.pci_kern_read_file((s8 *)keyfilepath,tmpkeydata,readlength,addr);
				if(ret != readlength)
				{
					DDEBUG("\r\n%s app key file read error111! \r\n",keyfilepath);
					pcicorecallfunc.mem_free(tmpkeydata);
					return PCI_READKEY_ERR;
				}

				
                for(i = 0;i<currentreadnum;i++)
                {
                    
                    memcpy((u8 *)(&tmpappkey),&tmpkeydata[i*PCIAPPKEYSIZE],PCIAPPKEYSIZE);
					ret = pcikeymanage_checkappkey(&tmpappkey,keytype,keyindex);
                    if(ret != PCI_PROCESSCMD_SUCCESS&&ret != PCI_KEYEXIST_ERR)
                    {
                    	Lib_memset(tmpkeydata,0,memlen);
						pcicorecallfunc.mem_free(tmpkeydata);
						return ret;
                    }
					
					//检查密钥文件中是否已经存在相同的密钥
					if(cmpkey != NULL&&ret == PCI_PROCESSCMD_SUCCESS)
					{
						if(pcikeymanage_checkifkeyisthsame(cmpkey,&tmpappkey,cmpkeygroupindex,groupindex))
						{
							pcicorecallfunc.mem_free(tmpkeydata);
							return PCI_KEYSAME_ERR;
						}
						
					}
					
					keyindex++;
                }
				
				
	        }
			
			keytype = nextkeytype;
		}
	}
	Lib_memset(tmpkeydata,0,memlen);
    Lib_memset((u8 *)&tmpappkey,0,sizeof(PCIAPPKEY));
	pcicorecallfunc.mem_free(tmpkeydata);
	return PCI_PROCESSCMD_SUCCESS;
#endif
}



/*******************************************************************
Author:   sxl
Function Name: s32 pcikeymanage_saveappkey(u8 groupindex,u8 keytype,u8 keyindex,u8 keylen,u8 *keydata)
Function Purpose:Save app key
Input Paramters: 
                        groupindex - group index
                        keytype    -  app key type
                        keyindex   - key index NO.
                        keylen      - key len
                        keydata    - key data
Output Paramters:N/A
Return Value:    PCI_PROCESSCMD_SUCCESS  -      success
                       other - failed
Remark: N/A
Modify: N/A      1130   //20140327
********************************************************************/
s32 pcikeymanage_saveappkey(u8 groupindex,u8 keytype,u8 keyindex,u8 keylen,u8 *keydata)
{
    s32 ret;
	s8 keyfilepath[128];
	s32 addr,maxkeyindex;
	PCIAPPKEY tmpappkey;
	PCIAPPKEY cmpappkey;
	u8 elrckdata[8];
    u8 cnt= 0;
    
	DDEBUG("write key 8\r\n");  //sxl?

    //sxlremark
    //检查密钥组号不对
    //key group index error
	if(groupindex >= PCIPOS_APPKEYGROUP_MAXNUM)
	{
	    DDEBUG("save app key exceed the max group index \r\n");
		return KM_APP_KEY_GROUPINDEX_ERR;
	}

	//sxlremark
	//密钥长度不对
	//key length error
	if((keylen != 8)&&(keylen != 16)&&(keylen != 24))
	{
		return KM_APP_KEY_LEN_ERROR;
	}

    memset(keyfilepath,0,sizeof(keyfilepath));
	ret = pcikeymanage_getappkeyfileabsolutepath(groupindex,keyfilepath);
	if(ret != PCI_PROCESSCMD_SUCCESS)
	{
		return KM_APP_KEY_GROUPINDEX_ERR;
	}

	DDEBUG("write key 6\r\n");  //sxl?
	
	
	pcikeymanage_getkeyarea(keytype,&addr,&maxkeyindex);
	if(addr < 0)
	{
		DDEBUG("%swrite key type err\r\n",__FUNCTION__);  //sxl?
		return KM_APP_KEY_TYPE_ERROR;
	}
	

    //sxlremark
    //密钥索引号错误
    //key index error
	if(keyindex >= maxkeyindex)
	{
		return KM_APP_KEY_INDEX_ERROR;
	}

	DDEBUG("\r\n write key 7 %s\r\n",keyfilepath);  //sxl?
    //sxlremark
    //检查该组密钥文件是否存在?
    //check if the group key file exist
	if(pcicorecallfunc.pci_kern_access_file(keyfilepath) < APPKEY_LENGTH_MAX)   //sxl?还好检查密钥文件是否存在
	{
	    DDEBUG("\r\n write key 12dfa\r\n");
		ret = pcicorecallfunc.pci_kern_create_file(keyfilepath,APPKEY_LENGTH_MAX,0xff);
		if(ret < 0)
		{
			return PCI_WRITEKEY_ERR;
		}

		//need to write ISSUER SN
		//sxl authen
		DDEBUG("\r\n write key 12dff\r\n");
		ret = pcikeymanageauthen_posins_authen(PCIPOSAUTHEN_DOWNAPPKEY_SAVE,NULL,groupindex,0,0);
		DDEBUG("\r\n%s11 = %d \r\n",__FUNCTION__,ret);
		if(ret != PCI_PROCESSCMD_SUCCESS)
		{
			return ret;
		}
	}
	DDEBUG("\r\n write key 123\r\n");  //sxl?
	
    ret = pcikeymanage_readappkeyheadinfostatus(groupindex,NULL);
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
	
	tmpappkey.keytype  = keytype;
	tmpappkey.keyindex = keyindex;
	tmpappkey.keylen   = keylen;
	memset(tmpappkey.keydata,0xff,sizeof(tmpappkey.keydata));
	memcpy(tmpappkey.keydata,keydata,keylen);

    memcpy(&cmpappkey,&tmpappkey,sizeof(PCIAPPKEY));
	
	DDEBUG("\r\n write key 5\r\n");  //sxl?
	#ifdef SXL_DEBUG
	printk("\r\nsavekey:%02x %02x %02x ",keytype,keyindex,keylen);
    for(ret = 0;ret < keylen;ret++)
    {
    	printk("%02x ",tmpappkey.keydata[ret]);
    }
	printk("\r\n");
	#endif

    while(1)
    {
		ret = pcikeymanage_keyprocessusingMMK(ENCRYPT,tmpappkey.keydata,24);
		if(ret != PCI_PROCESSCMD_SUCCESS)
		{
			//MMK 不存在需要重新生成
		    {
		    	ret = pcitampermanage_regeneratemmkelrck();
				if(ret != PCI_PROCESSCMD_SUCCESS)
				{
					return ret;
				}
                cnt++;
                if(cnt>3)
                {
                    return PCI_DEAL_ERR;
                }
                continue;
		    }
		    DDEBUG("\r\n pcidrv_keyprocess_usingMMK ret = %d \r\n",ret);
			//return ret;
		}
		break;
    }
	DDEBUG("\r\n write key 1\r\n");  //sxl?

	
    pcikeymanage_CalculateLRCCheckvalue(&tmpappkey,elrckdata);
	ret = pcikeymanage_keyprocessusingELRCK(elrckdata,8);
	if(ret != PCI_PROCESSCMD_SUCCESS)
	{
		return ret;
	}
	memcpy(tmpappkey.keylrc,elrckdata,5);

	DDEBUG("\r\n write key 2\r\n");  //sxl?
    //sxlremark
    //检查是否有一样的密钥
	ret = pcikeymanage_checkallappkey(groupindex,&cmpappkey);
	DDEBUG("\r\n pcidrv_determine_sameappkey ret= %d\r\n",ret); 
	if(ret != PCI_PROCESSCMD_SUCCESS)
	{
		return ret;
	}

	DDEBUG("\r\n write key 3\r\n");  //sxl?
	//sxl 测试用删除
	#ifdef SXL_DEBUG
    for(ret = 0;ret < PCIAPPKEYSIZE;ret++)
    {
    	DDEBUG("%02x ",*(((u8 *)&tmpappkey)+ret));
    }
	DDEBUG("\r\n");
	#endif
    
	
	addr += ((s32)keyindex)*PCIAPPKEYSIZE;
	ret = pcicorecallfunc.pci_kern_insert_file(keyfilepath,(u8 *)&tmpappkey,PCIAPPKEYSIZE,addr);
    #ifdef PCI_DEBUG
    DDEBUG("\r\nsave key file = %d %d\r\n",keyindex,ret);
	#endif
	if(ret != PCIAPPKEYSIZE)
	{
		return PCI_WRITEKEY_ERR;
	}
    
	DDEBUG("\r\n write key 4KK\r\n");  //sxl?
    
    
	return PCI_PROCESSCMD_SUCCESS;
	
	
}


s32 pcikeymanage_deleteappkey(u8 groupindex,u8 keytype,u8 keyindex)
{
	s32 addr,maxkeyindex;
	s8 keyfilepath[128];
    s32 ret;
	PCIAPPKEY tmpappkey;
	
	if(groupindex >= PCIPOS_APPKEYGROUP_MAXNUM)
	{
	    DDEBUG("%s:save app key exceed the max group index \r\n",__FUNCTION__);
		return KM_APP_KEY_GROUPINDEX_ERR;
	}
	
	memset(keyfilepath,0,sizeof(keyfilepath));
	ret = pcikeymanage_getappkeyfileabsolutepath(groupindex,keyfilepath);
	if(ret != PCI_PROCESSCMD_SUCCESS)
	{
		return KM_APP_KEY_GROUPINDEX_ERR;
	}

	pcikeymanage_getkeyarea(keytype,&addr,&maxkeyindex);
	if(addr < 0)
	{
		DDEBUG("\r\n %swrite key type err\r\n",__FUNCTION__);  //sxl?
		return KM_APP_KEY_TYPE_ERROR;
	}
	
	memset((u8 *)&tmpappkey,0xff,sizeof(PCIAPPKEY));
	
	addr += ((s32)keyindex)*PCIAPPKEYSIZE;
	ret = pcicorecallfunc.pci_kern_insert_file(keyfilepath,(u8 *)&tmpappkey,PCIAPPKEYSIZE,addr);
	if(ret != PCIAPPKEYSIZE)
	{
		return PCI_WRITEKEY_ERR;
	}
	
	
	return PCI_PROCESSCMD_SUCCESS;
	
}


/*******************************************************************
Author:   sxl
Function Name: s32 pcidrv_read_appkey(u8 groupindex,u8 keytype,u8 keyindex,u8 *keylen,u8 *keydata)
Function Purpose:Save app PIN key
Input Paramters: 
                        groupindex - group index
                        keytype    - app key type
                        keyindex   - key index NO.
Output Paramters:
                        keylen      - key len
                        keydata    - key data
Return Value:    PCI_PROCESSCMD_SUCCESS  -      success
                       other - failed
Remark: N/A
Modify: N/A     1130  //20140327
********************************************************************/
s32 pcikeymanage_readappkey(u8 groupindex,u8 keytype,u8 keyindex,u8 *keylen,u8 *keydata)
{
	s8 keyfilepath[128];
	s32 ret;
    s32 addr,maxkeyindex;
	PCIAPPKEY tmpappkey;
    u8 tmpkeydata[32];
	
	
	memset(keyfilepath,0,sizeof(keyfilepath));
	ret = pcikeymanage_getappkeyfileabsolutepath(groupindex,keyfilepath);
	if(ret != PCI_PROCESSCMD_SUCCESS)
	{
        DDEBUG("%s(%d):ret=%d\r\n", __FUNCTION__, __LINE__, ret);
		return ret;
	}

	
	pcikeymanage_getkeyarea(keytype,&addr,&maxkeyindex);
	if(addr < 0)
	{
        DDEBUG("%s(%d):addr=%d\r\n", __FUNCTION__, __LINE__, addr);
		return addr;
	}
	
	if(keyindex >= maxkeyindex)
	{
        DDEBUG("%s(%d):keyindex=%d,maxkeyindex=%d\r\n", __FUNCTION__, __LINE__, keyindex, maxkeyindex);
		return PCI_KEYINDEX_ERR;
	}

	

	
    addr += ((s32)keyindex)*PCIAPPKEYSIZE;
	ret = pcicorecallfunc.pci_kern_read_file(keyfilepath,(u8 *)&tmpappkey,PCIAPPKEYSIZE,addr);

	
 
	if(ret != PCIAPPKEYSIZE)
	{
		DDEBUG("%s:%s app key file read error! \r\n",__FUNCTION__,keyfilepath);
		return PCI_READKEY_ERR;
	}
    
	memset(tmpkeydata,0xff,sizeof(tmpkeydata));
	if(Lib_memcmp(tmpkeydata,(u8 *)(&tmpappkey),sizeof(tmpkeydata)) == 0)
	{
		DDEBUG("%s(%d):size=%d,app key file read error! \r\n",__FUNCTION__, __LINE__, sizeof(tmpkeydata));
        dev_debug_printformat(NULL, tmpkeydata, 32);
		DDEBUG("%s(%d):\r\n",__FUNCTION__, __LINE__);
        dev_debug_printformat(NULL, (u8 *)(&tmpappkey), 32);
		return PCI_KEYEXIST_ERR;
	}


	//sxl 测试用删除
	#ifdef SXL_DEBUG
	DDEBUG("\r\n read key data: \r\n");
    for(ret = 0;ret < PCIAPPKEYSIZE;ret++)
    {
    	DDEBUG("%02x ",*(((u8 *)&tmpappkey)+ret));
    }
	DDEBUG("\r\n");
    #endif

	ret = pcikeymanage_checkappkey(&tmpappkey,keytype,keyindex);
	if(ret != PCI_PROCESSCMD_SUCCESS)
	{
		return ret;
	}
    
	
	*keylen = tmpappkey.keylen;
	memcpy(keydata,tmpappkey.keydata,tmpappkey.keylen);
	return PCI_PROCESSCMD_SUCCESS;
	
}



////20140327
s32 pcikeymanage_readandcheckappkey(u8 groupindex,u8 keytype,u8 keyindex,u8 *keylen,u8 *keydata)
{
	s32 ret;

    //sxlremark
    //读取密钥
    //read key
	ret = pcikeymanage_readappkey(groupindex,keytype,keyindex,keylen,keydata);
    if((ret == PCI_KEYTYPE_ERR)||(ret == PCI_KEYINDEXMATCH_ERR)||(ret == PCI_KEYLEN_ERR)||(ret == PCI_KEYLRC_ERR)||(ret == PCI_READKEY_ERR)||(ret == PCI_READMMK_ERR))
    {
		//sxl? 要显示触发事件
		pcicorecallfunc.pci_security_locked_hold(0);  //sxl? 在lock函数里删除密钥
    }

	return ret;
}


/*******************************************************************
Author:   sxl
Function Name: s32 pcikeymanage_groupkeyfile_initial(u8 groupindex)
Function Purpose:group key file initial
Input Paramters: 
                        groupindex - group index
Output Paramters:N/A
Return Value:    PCI_PROCESSCMD_SUCCESS  -      success
                       other - failed
Remark: N/A
Modify: N/A 	  //20140327	
********************************************************************/
s32 pcikeymanage_groupkeyfile_initial(u8 groupindex)
{
	
    s8 keyfilepath[20];
    s32 ret;
	
	ret = pcikeymanageauthen_groupapplockfile_initial(groupindex);
	if(ret != PCI_PROCESSCMD_SUCCESS)
	{
		return ret;
	}
        
	memset(keyfilepath,0,sizeof(keyfilepath));
	ret = pcikeymanage_getappkeyfileabsolutepath(groupindex,keyfilepath);
	if(ret != PCI_PROCESSCMD_SUCCESS)
	{
		return ret;
	}
	pcicorecallfunc.pci_kern_create_file(keyfilepath,APPKEY_LENGTH_MAX,0xff);
    
    
    #ifdef SUPPORTDUKPTK
    memset(keyfilepath,0,sizeof(keyfilepath));
	ret = pcikeymanage_getdukptkeyfile_absolutepath(groupindex,keyfilepath);   //sxl?
	if(ret != PCI_PROCESSCMD_SUCCESS)
	{
		return ret;
	}
	pcicorecallfunc.pci_kern_create_file(keyfilepath,DUKPTKFILESIZE,0xff);
	#endif
	
	return PCI_PROCESSCMD_SUCCESS;

	
}


/*******************************************************************
Author:   sxl
Function Name: void pcikeymanage_initkeydata(void)
Function Purpose:initial key data
Input Paramters: N/A
Output Paramters:N/A
Return Value:    PCI_PROCESSCMD_SUCCESS  -  success
                       other - failed
Remark: N/A     sxl1130
Modify: N/A //20140327
********************************************************************/
void pcikeymanage_initkeydata(void)
{
    s8 keyfilepath[100];
	s32 ret;
	u8 groupindex;

    //initial lock file
    //sxlremark
    //初始化所有密钥状态文件
    //Initialization all key state files
    DDEBUG("\r\npcidrv_initkeydata1\r\n");
	
    pcikeymanageauthen_applockfile_initial();

	DDEBUG("\r\napplockfile_initial 1\r\n");
	//sxlremark
	//初始化密钥使用间隔时间
	//Initialization key using the interval timer
    pcikeymanageauthen_initialuseinternal();
	//initial key file 
	//sxlremark
	//初始化所有密钥文件
	//initail all key files 
	for(groupindex = 0;groupindex < PCIPOS_APPKEYGROUP_MAXNUM;groupindex++)
	{
		memset(keyfilepath,0,sizeof(keyfilepath));
		ret = pcikeymanage_getappkeyfileabsolutepath(groupindex,keyfilepath);
		if(ret != PCI_PROCESSCMD_SUCCESS)
		{
			continue;
		}
		DDEBUG("\r\n%s 111\r\n",__FUNCTION__);
		pcicorecallfunc.pci_kern_create_file(keyfilepath,APPKEY_LENGTH_MAX,0xff);
		DDEBUG("\r\n%s 222\r\n",__FUNCTION__);

		#ifdef DDI_DUKPT_ENABLE
		memset(keyfilepath,0,sizeof(keyfilepath));
		ret = pcikeymanage_getdukptkeyfile_absolutepath(groupindex,keyfilepath);   //sxl?
		if(ret != PCI_PROCESSCMD_SUCCESS)
		{
			return;
		}
		pcicorecallfunc.pci_kern_create_file(keyfilepath,DUKPTKFILESIZE,0xff);
		#endif
	}

	//sxlremark
	//初始化认证状态
	//intial authenticate status
    pcikeymanageauthen_securityauthinfo_initial();
	
    //initial MMK 
    //sxlremark
    //初始化MMK、ELRCK
    //initial MMK and ELRCK
    //pcidrv_resetmmkelrck(); 
    pcitampermanage_regeneratemmkelrck_initialvalue();
	
	DDEBUG("\r\npcidrv_initkeydata2\r\n");
	
}

