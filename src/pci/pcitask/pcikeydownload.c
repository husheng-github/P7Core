


#include "pciglobal.h"


/*******************************************************************
Author:   sxl
Function Name: s32 pcikeydownload_loadposauthenkey(u8 *rxbuf)
Function Purpose:Processing download POS authen key command
Input Paramters: 
                        *rxbuf - Data from user space
Output Paramters:N/A
Return Value:    PCI_PROCESSCMD_SUCCESS  -      success
                       other - failed
Remark: N/A
Modify: N/A 	  1130	//20140327
********************************************************************/
s32 pcikeydownload_posauthenkey(u8 *rxbuf)
{
	u8 tmpdata[300];
	u8 tmpmacdata[300];
	s32 ret;
	u8 PTKey[24];
	u8 TMacKey[24];
	u8 groupindex = 0;
    
	
	ret = pcikeymanageauthen_posins_authen(PCIPOSAUTHEN_DOWNPOSAUTHENKEY_CHECK,NULL,0,tmpdata,0);
	#ifdef SXL_DEBUG
    DDEBUG("\r\npcidrv_prog_loadposauthenkey ret = %d\r\n",ret);
	#endif
	if(ret != PCI_PROCESSCMD_SUCCESS)
	{
		return KM_APPKEY_NOTAUTH;
	}
	memcpy(PTKey,tmpdata,24);
	memcpy(TMacKey,&tmpdata[24],24);

	tmpdata[0] = rxbuf[0];
    
    //sxlremark
    //使用PTK对密钥进行解密
    //decrypt the data using PTK
    #ifdef SXL_DEBUG
    DDEBUG("\r\npcidrv_prog_loadposauthenkey134\r\n");
	#endif
	Lib_Des24_DataProcessing(&rxbuf[1],&tmpdata[1],24,PTKey,DECRYPT);
	#ifdef SXL_DEBUG
    DDEBUG("\r\npcidrv_prog_loadposauthenkey1235\r\n");
	#endif
	memcpy(tmpmacdata,tmpdata,25);

    //sxlremark
    //使用TMACK计算密钥数据MAC
    //calculate the MAC data of the key using TMACK
	Des3_24Mac(TMacKey,tmpmacdata,25);
	DDEBUG("\r\n%s1 = %02x %02x %02x %02x \r\n",__FUNCTION__,rxbuf[25],rxbuf[26],rxbuf[27],rxbuf[28]);
	DDEBUG("\r\n%s2 = %02x %02x %02x %02x \r\n",__FUNCTION__,tmpmacdata[0],tmpmacdata[1],tmpmacdata[2],tmpmacdata[3]);
	if(Lib_memcmp(&rxbuf[25],tmpmacdata,4) == 0)
	{
		//sxlremark
		//保存POSAUTHENKEY
		//save key
		ret = pcikeymanageauthen_savepcimanagefile(0,&tmpdata[1]);
		DDEBUG("\r\n %s ret = %d \r\n",__FUNCTION__,ret);
	}
	else
	{
	    //sxlremark
	    //该组认证失败次数加1
	    //
	    pcikeymanageauthen_posins_authen(PCIPOSAUTHEN_DOWNAPPKEY_CHECKFAIL,NULL,groupindex,NULL,0);
		ret = KM_AUTH_ERROR;
	}

	return ret;
}





/*******************************************************************
Author:   sxl
Function Name: s32 pcikeydownload_setpink(u8 *rxbuf,s32 rxbuflen)
Function Purpose:Processing set PIN key command
Input Paramters: *rxbuf - Data from user space
                        rxbuflen  - Data len
Output Paramters:N/A
Return Value:    PCI_PROCESSCMD_SUCCESS  -      success
                       other - failed
Remark: N/A      sxl 1130
Modify: N/A 		sxl modify   //20140327
********************************************************************/
s32 pcikeydownload_setpink(u8 *rxbuf,s32 rxbuflen)
{
    s32 ret;
	u8 pinkdecryptedmode;
	u8 mainkeyindex;
	u8 keyindex;
	u8 keylen;
	u8 *keydata;
	u8 *keycrc;
	u8 ak_sessionkey[24],ak_sessionkeylen;
	u8 tk_pin[24],tk_pinlen;
	u8 macsrcdata[8+24],mac[8];
	u8 groupindex;
    
	DDEBUG("\r\n setpink1 \r\n");
	
	if(rxbuflen < 5)
	{
		return PCI_INPUTPARAMS_ERR;
	}
    
	DDEBUG("\r\n setpink2 %d\r\n",rxbuf[4]);
	if(rxbuflen <(5+rxbuf[4]+4))
	{
		return PCI_INPUTPARAMS_ERR;
	}
    
	DDEBUG("\r\n setpink3 \r\n");
    
	pinkdecryptedmode = rxbuf[0];
	groupindex        = rxbuf[1];   //sxl modify
	mainkeyindex      = rxbuf[2];
	keyindex          = rxbuf[3];
	keylen            = rxbuf[4];
	
    if((keylen != 8)&&(keylen != 16)&&(keylen != 24))
	{
		return PCI_INPUTPARAMS_ERR;
	}
    //sxlremark
    //设置PINK认证
    //set pink authenticate
	ret = pcikeymanageauthen_posins_authen(PCIPOSAUTHEN_SETWK_CHECK,NULL,groupindex,NULL,0);
	if(ret != PCI_PROCESSCMD_SUCCESS)
	{
		return ret;
	}

	DDEBUG("\r\n setpink4 \r\n");
	
	keydata           = &rxbuf[5];
	keycrc            = &rxbuf[5+keylen];
	
	switch(pinkdecryptedmode)
	{
		case 0x00:
		case 0x01:
		case 0x81:
			//sxlremark
			//读取TK_PIN
			//read TK_PIN key
            ret = pcikeymanage_readandcheckappkey(groupindex,TYPE_TMK,mainkeyindex,&tk_pinlen,tk_pin);
			if(ret != PCI_PROCESSCMD_SUCCESS)
			{
				memset(tk_pin,0,sizeof(tk_pin));
				memset(keydata,0,keylen);
				return ret;
			}

			DDEBUG("\r\n setpink5 \r\n");
            //sxlremark
            //使用TK_PIN对加密的密钥数据进行解密
            //decrypt the Encryption key use TK_PIN
			ret = DES_TDES(tk_pin,tk_pinlen,keydata,keylen,0x00); //sxl20160218
			if(ret != PCI_PROCESSCMD_SUCCESS)
			{
				memset(tk_pin,0,sizeof(tk_pin));
				memset(keydata,0,keylen);
				return ret;
			}
			DDEBUG("\r\n setpink6 \r\n");
			break;
		default:
			return PCI_KEYMODE_ERR;
	}
	//sxlremark
	//使用ak_sessionkey对密钥数据的完整性进行校验
	//using ak_sessionkey verify the integrity of the key data
	if(pinkdecryptedmode == 0x01)
	{
		memset(macsrcdata,0,sizeof(macsrcdata));
		memset(mac,0,sizeof(mac));

		ret = DES_TDES(keydata,keylen,mac,8,0x01); //sxl20160218
		if(ret != PCI_PROCESSCMD_SUCCESS)
		{
			memset(tk_pin,0,sizeof(tk_pin));
			memset(keydata,0,keylen);
			return ret;
		}
	}
	else
	{
	    memset((u8 *)ak_sessionkey,0,sizeof(ak_sessionkey));
		ret = pcikeymanage_readandcheckappkey(groupindex,TYPE_BPK,0,&ak_sessionkeylen,ak_sessionkey);
		if(ret != PCI_PROCESSCMD_SUCCESS)
		{
		    Lib_memset(keydata,0,keylen);
			return PCI_READSESSIONK_ERR;
		}
		DDEBUG("\r\n setpink7 \r\n");
		
		memset(macsrcdata,0,sizeof(macsrcdata));
		memset(mac,0,sizeof(mac));
		macsrcdata[0] = pinkdecryptedmode;
	    macsrcdata[1] = groupindex;
		macsrcdata[2] = mainkeyindex;
		macsrcdata[3] = keyindex;
		macsrcdata[4] = keylen;
		memcpy(&macsrcdata[8],keydata,keylen);
		DesMacWithTCBC(macsrcdata,8+keylen,ak_sessionkey,ak_sessionkeylen,mac);
	}

	#ifdef SXL_DEBUG
    printk("\r\n 解出的密钥1  %d ,%d \r\n",keyindex,keylen);
	for(ret = 0;ret < keylen;ret++)
	{
		printk("%02x ",keydata[ret]);
	}
	printk("\r\n");
	#endif

	
	DDEBUG("\r\n%s %02x %02x %02x %02x\r\n",__FUNCTION__,keycrc[0],keycrc[1],keycrc[2],keycrc[3]);
	DDEBUG("\r\n%s %02x %02x %02x %02x\r\n",__FUNCTION__,mac[0],mac[1],mac[2],mac[3]);
	//sxlremark
	//检查MAC值是否一致
	//Check if the MAC value is consistent
	if(Lib_memcmp(mac,keycrc,4))
	{
	    Lib_memset(keydata,0,keylen);
		memset(ak_sessionkey,0,sizeof(ak_sessionkey));
		//sxl modify authenticate 
		//sxlremark
		//认证失败,认证失败次数加1
		//Authentication failure, authentication failures plus 1
		pcikeymanageauthen_posins_authen(PCIPOSAUTHEN_DOWNAPPKEY_CHECKFAIL,NULL,groupindex,NULL,0);
		return KM_AUTH_ERROR;
	}
	else
	{
	    //sxlremark
	    //认证成功, 清认证失败次数
	    //Authentication success, authentication failures clear
		pcikeymanageauthen_posins_authen(PCIPOSAUTHEN_DOWNAPPKEY_CHECKSUC,NULL,groupindex,NULL,0);
	}
	Lib_memset(ak_sessionkey,0,sizeof(ak_sessionkey));
	
	
    #ifdef SXL_DEBUG
    printk("\r\n 解出的密钥  %d ,%d \r\n",keyindex,keylen);
	for(ret = 0;ret < keylen;ret++)
	{
		printk("%02x ",keydata[ret]);
	}
	printk("\r\n");
	#endif
	//sxlremark
	//保存PINK工作密钥
	//save PINK work key
	ret = pcikeymanage_saveappkey(groupindex,TYPE_PINK,keyindex,keylen,keydata);
	DDEBUG("\r\n setpink8 %d\r\n",ret);
    //sxlremark
    //清除缓存数据
    //clear Temporary buffer data
	Lib_memset(tk_pin,0,sizeof(tk_pin));
	Lib_memset(keydata,0,keylen);
	return ret;
	
	
	
	
}

/*******************************************************************
Author:   sxl
Function Name: s32 pcikeydownload_setmack(u8 *rxbuf,s32 rxbuflen)
Function Purpose:Processing set PIN key command
Input Paramters: 
                        *rxbuf - Data from user space
                        rxbuflen  - Data len
Output Paramters:If output parameters exist,by arg to the user space.
Return Value:    PCI_PROCESSCMD_SUCCESS  -      success
                       other - failed
Remark: N/A     sxl1130
Modify: N/A      sxl modify  //20140327
********************************************************************/
s32 pcikeydownload_setmack(u8 *rxbuf,s32 rxbuflen)
{
	s32 ret;
	u8 pinkdecryptedmode;
	u8 mainkeyindex;
	u8 keyindex;
	u8 keylen;
	u8 *keydata;
	u8 *keycrc;
	u8 ak_sessionkey[24],ak_sessionkeylen;
	u8 tk_mac[24],tk_maclen;
	u8 macsrcdata[8+24],mac[8];
	u8 groupindex;
	
	if(rxbuflen < 5)
	{
		return PCI_INPUTPARAMS_ERR;
	}
	if(rxbuflen <(5+rxbuf[4]+4))
	{
		return PCI_INPUTPARAMS_ERR;
	}

    pinkdecryptedmode = rxbuf[0];
    groupindex        = rxbuf[1];   //sxl modify
	mainkeyindex      = rxbuf[2];
	keyindex          = rxbuf[3];
	keylen	          = rxbuf[4];
	
	if((keylen != 8)&&(keylen != 16)&&(keylen != 24))
	{
		return PCI_INPUTPARAMS_ERR;
	}
	
	keydata           = &rxbuf[5];
	keycrc	          = &rxbuf[5+keylen];
	
    ret = pcikeymanageauthen_posins_authen(PCIPOSAUTHEN_SETWK_CHECK,NULL,groupindex,NULL,0);
	if(ret != PCI_PROCESSCMD_SUCCESS)
	{
		return ret;
	}

	
	switch(pinkdecryptedmode)
	{
		case 0x00:
		case 0x01:
		case 0x81:
			ret = pcikeymanage_readandcheckappkey(groupindex,TYPE_TMK,mainkeyindex,&tk_maclen,tk_mac);
			if(ret != PCI_PROCESSCMD_SUCCESS)
			{
				Lib_memset(tk_mac,0,sizeof(tk_mac));
				Lib_memset(keydata,0,keylen);
				return ret;
			}

			ret = DES_TDES(tk_mac,tk_maclen,keydata,keylen,0x00);
			if(ret != PCI_PROCESSCMD_SUCCESS)
			{
				Lib_memset(tk_mac,0,sizeof(tk_mac));
				Lib_memset(keydata,0,keylen);
				return ret;
			}
			break;
		default:
			return PCI_KEYMODE_ERR;
	}

	if(pinkdecryptedmode == 0x01)
	{
		memset(macsrcdata,0,sizeof(macsrcdata));
		memset(mac,0,sizeof(mac));

		ret = DES_TDES(keydata,keylen,mac,8,0x01); //sxl20160218
		if(ret != PCI_PROCESSCMD_SUCCESS)
		{
			memset(tk_mac,0,sizeof(tk_mac));
			memset(keydata,0,keylen);
			return ret;
		}
	}
	else
	{
		Lib_memset(tk_mac,0,sizeof(tk_mac));
		Lib_memset((u8 *)ak_sessionkey,0,sizeof(ak_sessionkey));
		ret = pcikeymanage_readandcheckappkey(groupindex,TYPE_BPK,0,&ak_sessionkeylen,ak_sessionkey);
		if(ret != PCI_PROCESSCMD_SUCCESS)
		{
		    Lib_memset(keydata,0,keylen);
			return PCI_READSESSIONK_ERR;
		}
		
		memset(macsrcdata,0,sizeof(macsrcdata));
		memset(mac,0,sizeof(mac));
		memset(mac,0,sizeof(mac));
		macsrcdata[0] = pinkdecryptedmode;
	    macsrcdata[1] = groupindex;
		macsrcdata[2] = mainkeyindex;
		macsrcdata[3] = keyindex;
		macsrcdata[4] = keylen;
		memcpy(&macsrcdata[8],keydata,keylen);
		DesMacWithTCBC(macsrcdata,8+keylen,ak_sessionkey,ak_sessionkeylen,mac);
	}
	DDEBUG("\r\n%s %02x %02x %02x %02x\r\n",__FUNCTION__,keycrc[0],keycrc[1],keycrc[2],keycrc[3]);
	DDEBUG("\r\n%s %02x %02x %02x %02x\r\n",__FUNCTION__,mac[0],mac[1],mac[2],mac[3]);
	if(Lib_memcmp(mac,keycrc,4))
	{
	    Lib_memset(keydata,0,keylen);
		Lib_memset(ak_sessionkey,0,sizeof(ak_sessionkey));
		//sxl modify authenticate
		pcikeymanageauthen_posins_authen(PCIPOSAUTHEN_DOWNAPPKEY_CHECKFAIL,NULL,groupindex,NULL,0);
		return KM_AUTH_ERROR;
	}
	else
	{
		pcikeymanageauthen_posins_authen(PCIPOSAUTHEN_DOWNAPPKEY_CHECKSUC,NULL,groupindex,NULL,0);
	}

	#ifdef SXL_DEBUG
    printk("\r\n 解出的密钥  %d ,%d \r\n",keyindex,keylen);
	for(ret = 0;ret < keylen;ret++)
	{
		printk("%02x ",keydata[ret]);
	}
	printk("\r\n");
	#endif
	
	Lib_memset(ak_sessionkey,0,sizeof(ak_sessionkey));
	ret = pcikeymanage_saveappkey(groupindex,TYPE_MACK,keyindex,keylen,keydata);
	Lib_memset(keydata,0,keylen);
	return ret;
}




/*******************************************************************
Author:   sxl
Function Name: s32 pcikeydownload_settdk(u8 *rxbuf,s32 rxbuflen)
Function Purpose:Processing set PIN key command
Input Paramters: 
                        *rxbuf - Data from user space
                        rxbuflen  - Data len
Output Paramters:If output parameters exist,by arg to the user space.
Return Value:    PCI_PROCESSCMD_SUCCESS  -      success
                       other - failed
Remark: N/A     sxl1130
Modify: N/A      sxl modify  //20140327
********************************************************************/
s32 pcikeydownload_settdk(u8 *rxbuf,s32 rxbuflen)
{
	s32 ret;
	u8 pinkdecryptedmode;
	u8 mainkeyindex;
	u8 keyindex;
	u8 keylen;
	u8 *keydata;
	u8 *keycrc;
	u8 ak_sessionkey[24],ak_sessionkeylen;
	u8 tk_mac[24],tk_maclen;
	u8 macsrcdata[8+24],mac[8];
	u8 groupindex;
	
	if(rxbuflen < 5)
	{
		return PCI_INPUTPARAMS_ERR;
	}
	if(rxbuflen <(5+rxbuf[4]+4))
	{
		return PCI_INPUTPARAMS_ERR;
	}

    pinkdecryptedmode = rxbuf[0];
    groupindex        = rxbuf[1];   //sxl modify
	mainkeyindex      = rxbuf[2];
	keyindex          = rxbuf[3];
	keylen	          = rxbuf[4];
	
	if((keylen != 8)&&(keylen != 16)&&(keylen != 24))
	{
		return PCI_INPUTPARAMS_ERR;
	}
	
	keydata           = &rxbuf[5];
	keycrc	          = &rxbuf[5+keylen];
	
    ret = pcikeymanageauthen_posins_authen(PCIPOSAUTHEN_SETWK_CHECK,NULL,groupindex,NULL,0);
	if(ret != PCI_PROCESSCMD_SUCCESS)
	{
		return ret;
	}

	
	switch(pinkdecryptedmode)
	{
		case 0x00:
		case 0x01:
		case 0x81:
			ret = pcikeymanage_readandcheckappkey(groupindex,TYPE_TMK,mainkeyindex,&tk_maclen,tk_mac);
			if(ret != PCI_PROCESSCMD_SUCCESS)
			{
				Lib_memset(tk_mac,0,sizeof(tk_mac));
				Lib_memset(keydata,0,keylen);
				return ret;
			}

			ret = DES_TDES(tk_mac,tk_maclen,keydata,keylen,0x00);
			if(ret != PCI_PROCESSCMD_SUCCESS)
			{
				Lib_memset(tk_mac,0,sizeof(tk_mac));
				Lib_memset(keydata,0,keylen);
				return ret;
			}
			break;
		default:
			return PCI_KEYMODE_ERR;
	}

    if(pinkdecryptedmode == 0x01)
	{
		memset(macsrcdata,0,sizeof(macsrcdata));
		memset(mac,0,sizeof(mac));

		ret = DES_TDES(keydata,keylen,mac,8,0x01); //sxl20160218
		if(ret != PCI_PROCESSCMD_SUCCESS)
		{
			memset(tk_mac,0,sizeof(tk_mac));
			memset(keydata,0,keylen);
			return ret;
		}
	}
	else
	{
		Lib_memset(tk_mac,0,sizeof(tk_mac));
		Lib_memset((u8 *)ak_sessionkey,0,sizeof(ak_sessionkey));
		ret = pcikeymanage_readandcheckappkey(groupindex,TYPE_BPK,0,&ak_sessionkeylen,ak_sessionkey);
		if(ret != PCI_PROCESSCMD_SUCCESS)
		{
		    Lib_memset(keydata,0,keylen);
			return PCI_READSESSIONK_ERR;
		}
		
		memset(macsrcdata,0,sizeof(macsrcdata));
		memset(mac,0,sizeof(mac));
		memset(mac,0,sizeof(mac));
		macsrcdata[0] = pinkdecryptedmode;
	    macsrcdata[1] = groupindex;
		macsrcdata[2] = mainkeyindex;
		macsrcdata[3] = keyindex;
		macsrcdata[4] = keylen;
		memcpy(&macsrcdata[8],keydata,keylen);
		DesMacWithTCBC(macsrcdata,8+keylen,ak_sessionkey,ak_sessionkeylen,mac);
	}
	DDEBUG("\r\n%s %02x %02x %02x %02x\r\n",__FUNCTION__,keycrc[0],keycrc[1],keycrc[2],keycrc[3]);
	DDEBUG("\r\n%s %02x %02x %02x %02x\r\n",__FUNCTION__,mac[0],mac[1],mac[2],mac[3]);
	if(Lib_memcmp(mac,keycrc,4))
	{
	    Lib_memset(keydata,0,keylen);
		Lib_memset(ak_sessionkey,0,sizeof(ak_sessionkey));
		//sxl modify authenticate
		pcikeymanageauthen_posins_authen(PCIPOSAUTHEN_DOWNAPPKEY_CHECKFAIL,NULL,groupindex,NULL,0);
		return KM_AUTH_ERROR;
	}
	else
	{
		pcikeymanageauthen_posins_authen(PCIPOSAUTHEN_DOWNAPPKEY_CHECKSUC,NULL,groupindex,NULL,0);
	}
	
	#ifdef SXL_DEBUG
    printk("\r\n 解出的密钥  %d ,%d \r\n",keyindex,keylen);
	for(ret = 0;ret < keylen;ret++)
	{
		printk("%02x ",keydata[ret]);
	}
	printk("\r\n");
	#endif
	
	Lib_memset(ak_sessionkey,0,sizeof(ak_sessionkey));
	ret = pcikeymanage_saveappkey(groupindex,TYPE_TDK,keyindex,keylen,keydata);
	Lib_memset(keydata,0,keylen);
	return ret;
	
}

/*******************************************************************
Author:   sxl
Function Name: s32 pcidrv_prog_loadappdukptkey(u8 *rxbuf)
Function Purpose:Processing download app dukpt key command
Input Paramters: 
                        *rxbuf - Data from user space
Output Paramters:N/A
Return Value:    PCI_PROCESSCMD_SUCCESS  -      success
                       other - failed
Remark: N/A
Modify: N/A 		20110608 //20140327
********************************************************************/
#ifdef SUPPORTDUKPTK

s32 pcikeydownload_appdukptkey(u8 *rxbuf)
{
	u8 tmpdata[300];
	u8 tmpmacdata[300];
	s32 ret;
	u8 PTKey[25];
	u8 TMacKey[25];
	u8 groupindex;

    
    DDEBUG("\r\n pcidrv_prog_loadappdukptkey111 \r\n");
	
    groupindex = rxbuf[0];
	ret = pcikeymanageauthen_posins_authen(PCIPOSAUTHEN_DOWNAPPKEY_CHECK,NULL,groupindex,tmpdata,0);
	if(ret != PCI_PROCESSCMD_SUCCESS)
	{
		return KM_APPKEY_NOTAUTH;
	}

    DDEBUG("\r\n pcidrv_prog_loadappdukptkey222 \r\n");
	
	memcpy(PTKey,tmpdata,24);
	memcpy(TMacKey,&tmpdata[24],24);


    tmpdata[0] = rxbuf[0];
	tmpdata[1] = rxbuf[1];
	tmpdata[2] = rxbuf[2];
	tmpdata[3] = rxbuf[3];
	tmpdata[4] = rxbuf[4];

	Lib_Des24_DataProcessing(&rxbuf[5],&tmpdata[5],40,PTKey,DECRYPT);

	memset(tmpmacdata,0,sizeof(tmpmacdata));
	memcpy(tmpmacdata,rxbuf,5);
	memcpy(&tmpmacdata[5],&tmpdata[5],40);

	#ifdef SXL_DEBUG
    DDEBUG("\r\ntmpmacdata:\r\n");
    for(ret= 0;ret < 45;ret++)
	{
		DDEBUG("%02x ",tmpmacdata[ret]);
	}
	DDEBUG("\r\n");
	#endif

	Des3_24Mac(TMacKey,tmpmacdata,45);
	#ifdef SXL_DEBUG
    DDEBUG("\r\nTMacKey:\r\n");
	for(ret= 0;ret < 24;ret++)
	{
		DDEBUG("%02x ",TMacKey[ret]);
	}
	DDEBUG("\r\n");
	#endif
    DDEBUG("\r\n pcidrv_prog_loadappdukptkey MAC %02x %02x %02x %02x \r\n ",tmpmacdata[0],tmpmacdata[1],tmpmacdata[2],tmpmacdata[3]);
	
	if(Lib_memcmp(&rxbuf[45],tmpmacdata,4) == 0)
	{
	    pcikeymanageauthen_posins_authen(PCIPOSAUTHEN_DOWNAPPKEY_CHECKSUC,NULL,groupindex,NULL,0);
		ret = pcikeymanage_poskey_saveappdukptkey(tmpdata);

		DDEBUG("\r\nsave dukptk ret = %d\r\n",ret);
	}
	else
	{
	    //sxl?
	    pcikeymanageauthen_posins_authen(PCIPOSAUTHEN_DOWNAPPKEY_CHECKFAIL,NULL,groupindex,NULL,0);
		ret = KM_AUTH_ERROR;
	}

	//sxl? 测试用，要删除
    DDEBUG("\r\n pcidrv_prog_loadappdukptkey333 \r\n");

	return ret;
}

#endif
/*******************************************************************
Author:   sxl
Function Name: s32 pcikeydownload_savemainkey(u8 groupindex,u8 keyindex,u8 keylen,u8 *keydata)
Function Purpose:Save app PIN key
Input Paramters: 
                        groupindex - group index
                        keyindex   - key index NO.
                        keylen      - key len
                        keydata    - key data
Output Paramters:N/A
Return Value:    PCI_PROCESSCMD_SUCCESS  -      success
                       other - failed
Remark: N/A
Modify: N/A 		//1130  //20140327
********************************************************************/
s32 pcikeydownload_savemainkey(u8 groupindex,u8 keyindex,u8 keylen,u8 *keydata)
{
	return pcikeymanage_saveappkey(groupindex,TYPE_TMK,keyindex,keylen,keydata);
}

/*******************************************************************
Author:   sxl
Function Name: s32 pcikeydownload_savesessionkak(u8 groupindex,u8 keyindex,u8 keylen,u8 *keydata)
Function Purpose:Save app autheticate key of session key
Input Paramters: 
                        groupindex - group index
                        keyindex   - key index NO.
                        keylen      - key len
                        keydata    - key data
Output Paramters:N/A
Return Value:    PCI_PROCESSCMD_SUCCESS  -      success
                       other - failed
Remark: N/A
Modify: N/A     1130  //20140327
********************************************************************/
s32 pcikeydownload_savesessionkak(u8 groupindex,u8 keyindex,u8 keylen,u8 *keydata)
{
	return pcikeymanage_saveappkey(groupindex,TYPE_BPK,keyindex,keylen,keydata);
}

/*******************************************************************
Author:   sxl
Function Name: s32 pcikeydownload_savefixedkey(u8 groupindex,u8 keyindex,u8 keylen,u8 *keydata)
Function Purpose:Save app fixed key
Input Paramters: 
                        groupindex - group index
                        keyindex   - key index NO.
                        keylen      - key len
                        keydata    - key data
Output Paramters:N/A
Return Value:    PCI_PROCESSCMD_SUCCESS  -      success
                       other - failed
Remark: N/A
Modify: N/A 		1130  //20140327
********************************************************************/
s32 pcikeydownload_savefixedkey(u8 groupindex,u8 keyindex,u8 keylen,u8 *keydata)
{
	return pcikeymanage_saveappkey(groupindex,TYPE_FIXEDK,keyindex,keylen,keydata);
}

/*******************************************************************
Author:   sxl
Function Name: s32 pcidrv_poskey_saveappkey(u8 *keydata)
Function Purpose:Save app key
Input Paramters: 
                        *databuf - key data need parse
Output Paramters:N/A
Return Value:    PCI_PROCESSCMD_SUCCESS  -      success
                       other - failed
Remark: N/A
Modify: N/A 	  1130	//20140327
********************************************************************/
s32 pcikeydownload_saveappkey(u8 *databuf)
{
	u8 *keydata;
	u8 groupindex,keytype,keyindex,keylen;
	s32 ret = PCI_PROCESSCMD_SUCCESS;

    groupindex = databuf[0];  //sxl modify
	keytype  = databuf[1];
	keyindex = databuf[2];
	keylen   = databuf[3];
    keydata  = &databuf[4];

    
	if((keylen != 8)&&(keylen != 16)&&(keylen != 24))
	{
		return KM_APP_KEY_LEN_ERROR;
	}

	switch(keytype)
	{
		case KM_PCI_MAIN_PIN_KEY:
            ret = pcikeydownload_savemainkey(groupindex,keyindex,keylen,keydata);
			break;
		case KM_PCI_PIN_KEY:
			break;
		//case KM_PCI_MAIN_MAC_KEY:
            //ret = pcidrv_pos_savemackey(groupindex,keyindex,keylen,keydata);
			//break;
		case KM_PCI_SK_MACK:
            ret = pcikeydownload_savesessionkak(groupindex,keyindex,keylen,keydata);
			break;
		case KM_PCI_MAC_KEY:
			break;
		case KM_PCI_FKEY:
            ret = pcikeydownload_savefixedkey(groupindex,keyindex,keylen,keydata);
			break;
		default:
            return KM_APP_KEY_TYPE_ERROR;
			break;
	}

	
	return ret;
}


/*******************************************************************
Author:   sxl
Function Name: s32 pcikeydownload_appkey(u8 *rxbuf,ulong arg)
Function Purpose:Processing download app key command
Input Paramters: 
                        *rxbuf - Data from user space
Output Paramters:N/A
Return Value:    PCI_PROCESSCMD_SUCCESS  -      success
                       other - failed
Remark: N/A
Modify: N/A 	  1130	//20140327
********************************************************************/
s32 pcikeydownload_appkey(u8 *rxbuf)
{
	u8 tmpdata[300];
	u8 tmpmacdata[300];
	s32 ret;
	u8 PTKey[24];
	u8 TMacKey[24];
	u8 groupindex;

    groupindex = rxbuf[0];
	ret = pcikeymanageauthen_posins_authen(PCIPOSAUTHEN_DOWNAPPKEY_CHECK,NULL,groupindex,tmpdata,0);
	#ifdef SXL_DEBUG
    DDEBUG("\r\npcidrv_prog_loadappkey ret = %d\r\n",ret);
	#endif
	if(ret != PCI_PROCESSCMD_SUCCESS)
	{
		return KM_APPKEY_NOTAUTH;
	}
	memcpy(PTKey,tmpdata,24);
	memcpy(TMacKey,&tmpdata[24],24);

	tmpdata[0] = rxbuf[0];
	tmpdata[1] = rxbuf[1];
	tmpdata[2] = rxbuf[2];
	tmpdata[3] = rxbuf[3];
    
    //sxlremark
    //使用PTK对密钥进行解密
    //decrypt the data using PTK
    #ifdef SXL_DEBUG
    DDEBUG("\r\npcidrv_prog_loadappkey134\r\n");
	#endif
	Lib_Des24_DataProcessing(&rxbuf[4],&tmpdata[4],24,PTKey,DECRYPT);
	#ifdef SXL_DEBUG
    DDEBUG("\r\npcidrv_prog_loadappkey1235\r\n");
	#endif
	memcpy(tmpmacdata,tmpdata,28);

    //sxlremark
    //使用TMACK计算密钥数据MAC
    //calculate the MAC data of the key using TMACK
	Des3_24Mac(TMacKey,tmpmacdata,28);
	DDEBUG("\r\n%s1 = %02x %02x %02x %02x \r\n",__FUNCTION__,rxbuf[28],rxbuf[29],rxbuf[30],rxbuf[31]);
	DDEBUG("\r\n%s2 = %02x %02x %02x %02x \r\n",__FUNCTION__,tmpmacdata[0],tmpmacdata[1],tmpmacdata[2],tmpmacdata[3]);
	if(Lib_memcmp(&rxbuf[28],tmpmacdata,4) == 0)
	{
	    //sxlremark
	    //检查失败次数是否不为0，不为0要清0
	    //
		pcikeymanageauthen_posins_authen(PCIPOSAUTHEN_DOWNAPPKEY_CHECKSUC,NULL,groupindex,NULL,0);
		//sxlremark
		//保存密钥
		//save key
		ret = pcikeydownload_saveappkey(tmpdata);  //sxl? savepink还有问题
		DDEBUG("\r\n %s ret = %d \r\n",__FUNCTION__,ret);
	}
	else
	{
	    //sxlremark
	    //该组认证失败次数加1
	    //
	    pcikeymanageauthen_posins_authen(PCIPOSAUTHEN_DOWNAPPKEY_CHECKFAIL,NULL,groupindex,NULL,0);
		ret = KM_AUTH_ERROR;
	}

	return ret;
}

/*******************************************************************
Author:   sxl
Function Name:  s32 pcikeydownload_magkey(u8 *rxbuf) 
Function Purpose:Processing download mag key command
Input Paramters: 
                        *rxbuf - Data from user space
Output Paramters:N/A
Return Value:    PCI_PROCESSCMD_SUCCESS  -      success
                       other - failed
Remark: N/A
Modify: N/A 	  1130	//20140327
********************************************************************/
s32 pcikeydownload_magkey(u8 *rxbuf,u8 *returndata,u32 *returndatalen)
{
	u8 tmpdata[100];
	u8 tmpmacdata[100];
	s32 ret;
	u8 PTKey[24];
	u8 TMacKey[24];
	//u8 groupindex;
	u8 initialflag,ksnlen,keyindex,keylen;
	u8 magksn[10];

    *returndatalen = 0;

	
	ret = pcikeymanageauthen_posins_authen(PCIPOSAUTHEN_DOWNMAGKEY_CHECK,NULL,0,tmpdata,0);
	//TEMPDDEBUG("\r\npcidrv_prog_loadmagkey = %d\r\n",ret);
	if(ret != PCI_PROCESSCMD_SUCCESS)
	{
		return KM_APPKEY_NOTAUTH;
	}
	memcpy(PTKey,tmpdata,24);
	memcpy(TMacKey,&tmpdata[24],24);

	initialflag = tmpdata[0] = rxbuf[0];
	ksnlen = tmpdata[1] = rxbuf[1];
	keyindex = tmpdata[2] = rxbuf[2];
	keylen = tmpdata[3] = rxbuf[3];

    if((ksnlen != 10)||(keyindex>=MAXMAGKEYINDEX)||(keylen != 16))
    {
    	return PCI_INPUTPARAMS_ERR;
    }
    //sxlremark
    //使用PTK对密钥进行解密
    //decrypt the data using PTK
	Lib_Des24_DataProcessing(&rxbuf[4],&tmpdata[4],24,PTKey,DECRYPT);
	Lib_Des24_DataProcessing(&rxbuf[28],&tmpdata[28],16,PTKey,DECRYPT);

	memcpy(magksn,&rxbuf[48],10);
	
	memcpy(tmpmacdata,tmpdata,28+16);

    //sxlremark
    //使用TMACK计算密钥数据MAC
    //calculate the MAC data of the key using TMACK
	Des3_24Mac(TMacKey,tmpmacdata,28+16);
	//TEMPDDEBUG("\r\n%s1 = %02x %02x %02x %02x \r\n",__FUNCTION__,rxbuf[28+16],rxbuf[29+16],rxbuf[30+16],rxbuf[31+16]);
	//TEMPDDEBUG("\r\n%s2 = %02x %02x %02x %02x \r\n",__FUNCTION__,tmpmacdata[0],tmpmacdata[1],tmpmacdata[2],tmpmacdata[3]);
	if(Lib_memcmp(&rxbuf[28+16],tmpmacdata,4) == 0)
	{
		//sxlremark
		//保存密钥
		//save key
		//ret = pcidrv_magkey_save(initialflag,keyindex,&tmpdata[4],&tmpdata[28],magksn); //sxl?暂时不处理磁头
		ret = PCI_WRITEKEY_ERR;
		if(ret == PCI_PROCESSCMD_SUCCESS)
		{
		    if(initialflag  == 1)
		    {
				memcpy(returndata,&tmpdata[4],40);
				*returndatalen = 40;
		    }
		}
		//TEMPDDEBUG("\r\n %s ret = %d \r\n",__FUNCTION__,ret);
	}
	else
	{
	    //sxlremark
	    //该组认证失败次数加1
	    //
		ret = KM_AUTH_ERROR;
	}
    //TEMPDDEBUG("\r\nloadmagret = %d\r\n",ret);
	return ret;
	
}

/*******************************************************************
Author:   sxl
Function Name:   s32 pcidrv_groupkeyappname_set(u8 *rxbuf)
Function Purpose:set group key appname
Input Paramters: 
                        *rxbuf - Data from user space
Output Paramters:N/A
Return Value:    PCI_PROCESSCMD_SUCCESS  -      success
                       other - failed
Remark: N/A
Modify: N/A 	  1130	//20140327
********************************************************************/
s32 pcikeydownload_groupkeyappname_set(u8 *rxbuf)
{
	u8 groupindex;
	u8 *encryptedappname,*mac;
	s32 ret;
	u8 appname[100];
	u8 tmpdata[100];
	u8 PTKey[24];
	u8 TMacKey[24];
	u8 tmpmacdata[100];

	groupindex = rxbuf[0];
	encryptedappname = &rxbuf[1];
    mac = &rxbuf[1+64];

    //sxlremark
    //检查是否认证过
    //check if authened
	ret = pcikeymanageauthen_posins_authen(PCIPOSAUTHEN_DOWNAPPKEY_CHECK,NULL,groupindex,tmpdata,0);
	if(ret != PCI_PROCESSCMD_SUCCESS)
	{
		return KM_APPKEY_NOTAUTH;
	}

	memcpy(PTKey,tmpdata,24);
	memcpy(TMacKey,&tmpdata[24],24);
	memset(appname,0,sizeof(appname));
	Lib_Des24_DataProcessing(encryptedappname,appname,64,PTKey,DECRYPT);
	if(strlen((s8 *)appname) >= 64)
	{
	    appname[99] = 0;
		DDEBUG("\r\n%s appname err!\r\n",__FUNCTION__);
		DDEBUG("\r\n%s\r\n",appname);
		return PCI_INPUTPARAMS_ERR;
	}

	DDEBUG("\r\npcidrv_groupkeyappname_set:%s\r\n",appname);

    memset(tmpmacdata,0,sizeof(tmpmacdata));
	tmpmacdata[0] =groupindex;
	memcpy(&tmpmacdata[1],appname,64);
	Des3_24Mac(TMacKey,tmpmacdata,65);

    ret = PCI_PROCESSCMD_SUCCESS;
	if(Lib_memcmp(tmpmacdata,mac,4))
	{
		 pcikeymanageauthen_posins_authen(PCIPOSAUTHEN_DOWNAPPKEY_CHECKFAIL,NULL,groupindex,NULL,0);
		 ret = KM_AUTH_ERROR;
	}
	else
	{
	     pcikeymanageauthen_posins_authen(PCIPOSAUTHEN_DOWNAPPKEY_CHECKSUC,NULL,groupindex,NULL,0);
		ret = pcikeymanageauthen_savegroupkeyappname(groupindex,appname);
	}

	return ret;
}



