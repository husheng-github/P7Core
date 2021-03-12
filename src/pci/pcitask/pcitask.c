

#include "pciglobal.h"



void pcitask_commu_SendPacket(s32 protocolcmd,u8 *senddata,s32 senddatalen,u32 arg)
{
    s32 i;
	u8 *gPCIdrvsendbuf;    //[2048] 
    s32 gPCIdrvsendbufLen;
    
    
	pcilog_savekeymanagelog(gDownLoadKeyErr); //sxl?
    
    
	gPCIdrvsendbuf = (u8 *)pcicorecallfunc.mem_malloc(2048);
	//DDEBUG("\r\n SendPacket1 \r\n");
	gPCIdrvsendbuf[0] = 0x02;
	memcpy(&gPCIdrvsendbuf[1],"XGDPCI",6);
	i = 7;
	gPCIdrvsendbuf[i++] = (u8)(protocolcmd>>8);
	gPCIdrvsendbuf[i++] = (u8)(protocolcmd);
	gPCIdrvsendbuf[i++] = (u8)(senddatalen>>8);
	gPCIdrvsendbuf[i++] = (u8)(senddatalen);
	memcpy(&gPCIdrvsendbuf[i],senddata,senddatalen);
	i += senddatalen;
	gPCIdrvsendbuf[i++] = 0x03;
	pciarithCrc16CCITT(&gPCIdrvsendbuf[7],senddatalen+4,&gPCIdrvsendbuf[i]);
	gPCIdrvsendbufLen = i+2;
	
	
	#ifdef SXL_DEBUG  //sxl?
    DDEBUG("\r\nSendPacketkk:%d\r\n",gPCIdrvsendbufLen);
    for(i = 0;i < gPCIdrvsendbufLen;i++)
    {
    	DDEBUG("%02x ",gPCIdrvsendbuf[i]);
    }
	DDEBUG("\r\n");
	#endif
	
	
	//pci_drv_com_write(PORT_PC,gPCIdrvsendbuf,gPCIdrvsendbufLen);    //sxl?
	//SpiAndroidProtocol_SendPciData(gPCIdrvsendbuf,gPCIdrvsendbufLen);
	
    
	pcicorecallfunc.mem_free(gPCIdrvsendbuf);
    
	
}

/*******************************************************************
Author:   sxl
Function Name: void pcitask_dealresult(s32 ret,u8 *result)
Function Purpose:Processing download key command
Input Paramters: 
                        ret  - deal result
Output Paramters:*result - truansfer result
Return Value:  N/A
Remark: N/A
Modify: N/A 	 0722  //20140327
********************************************************************/
void pcitask_dealresult(s32 ret,u8 *result)
{

    if(ret == PCI_WRITEKEY_ERR)
    {
    	result[0] = KM_APP_KEY_WRITE_ERROR;
    }
	else if(ret == PCI_KEYGROUPINDEX_ERR)
	{
		result[0] = KM_APP_KEY_GROUPINDEX_ERR;
	}
	else if(ret == PCI_KEYINDEX_ERR)
	{
		result[0] = KM_APP_KEY_INDEX_ERROR;
	}
	else if(ret == PCI_KEYTYPE_ERR)
	{
		result[0] = KM_APP_KEY_TYPE_ERROR;
	}
	else if(ret == PCI_KEYLEN_ERR)
	{
		result[0] = KM_APP_KEY_LEN_ERROR;
	}
	else if(ret == PCI_AUTHEN_NEED)
	{
		result[0] = KM_APPKEY_NOTAUTH;
	}
	else if(ret == PCI_PUBLICKKEY_UNFIND)
	{
		result[0] = KM_PUBLICKKEY_UNFIND;
	}
	else if(ret == PCI_AUTHENAPPKEYLOCK_ERR)
	{
		result[0] = KM_APPKEY_LOCKED;
	}
	else if(ret == PCI_RSACALCULATE_ERR || ret == PCI_AUTHENRSAPK_ERR)
	{
		result[0] = KM_RSACALCULATE_ERR;
	}
	else if(ret == PCI_KEYSAME_ERR)
	{
		result[0] = KM_APPKEYSAME_ERR;
	}
	else if(ret == PCI_KEYMODE_ERR)
	{
		result[0] = KM_ARITHMETIC_ERR;
	}
	else if(ret == PCI_READKEY_ERR)
	{
		result[0] = KM_READKEY_ERR;
	}
	else if(ret == PCI_READSESSIONK_ERR)
	{
		result[0] = KM_READSESSIONK_ERR;
	}
	else if(ret == PCI_GETPIN_INTERVALUNREACH)
	{
		result[0] = KM_GETPIN_INTERVALUNREACH;
	}
	else if(ret == PCI_MACALGORITHMINDEX_ERR || ret == PCI_PINENTRYPTMODE_ERR)
	{
		result[0] = KM_ARITHMETICINDEX_ERR;
	}
	else if(ret == PCI_AUTHENKEYISSUER_ERROR)
	{
		result[0] = KM_AUTHENKEYISSUER_ERROR;
	}
	else if(ret == PCI_KEYFILENOTUSE)
	{
		result[0] = KM_KEYFILENOTUSE;
	}
	else if(ret == PCI_MAGDATA_WAITSWIPE)
	{
		result[0] = KM_MAGDATA_WAITSWIPE;
	}
	else if(ret == PCI_MAGDATA_ERR)
	{
		result[0] = KM_MAGDATA_ERR;
	}
	else
	{
		result[0] = KM_OTHER_ERR;
	}
	
}


s32 pcitask_workkeystatus_check(u8 *appname,u8 groupindex,u8 keytype,u8 keyindex)
{
	u8 keydata[100],keylen;
	s32 ret;
	u8 tmpkeytype;
	
	
	if(keytype == TYPE_PINK)
    {
    	tmpkeytype = KEYTYPE_GETPIN_PINK;
    }
	else if(keytype == TYPE_FIXEDK)
	{
		tmpkeytype = KEYTYPE_GETPIN_FIXEDK;
	}
	else if(keytype == TYPE_DUKPTK)
	{
		tmpkeytype = KEYTYPE_GETPIN_DUKPTK;
	}
	else
	{
	    DDEBUG("%s:Input paramter error ! \r\n",__FUNCTION__);
		return PCI_INPUTPARAMS_ERR;
	}
	
	
	ret = pcikeymanageauthen_sensitiveservicheck(groupindex,appname);
	if(ret != PCI_PROCESSCMD_SUCCESS)
	{
		return ret;
	}

    ret = pcikeymanageauthen_posins_authen(PCIPOSAUTHEN_GETPIN_CHECKPININTERVAL,NULL,groupindex,&tmpkeytype,1);
	if(ret != PCI_PROCESSCMD_SUCCESS)
	{
		return ret;
	}

	#ifdef DDI_DUKPT_ENABLE
    if(keytype == TYPE_DUKPTK)  //sxl20110608
    {
    	ret = pcikeymanage_set_currappdukptk(groupindex);
		if(ret != PCI_PROCESSCMD_SUCCESS)
		{
		    DDEBUG("%s:set current dukpt key error! \r\n",__FUNCTION__);
			return ret;
		}
		ret = pcikeymanage_check_dukptkexist(keyindex);
    }
	else
	#endif
	{
		ret = pcikeymanage_readandcheckappkey(groupindex,keytype,keyindex,&keylen,keydata);
	}
	Lib_memset(keydata,0,sizeof(keydata));
	return ret;
}


s32 pcitask_dealsecurityservice(u32 OperateInstruction,u8 *rxbuf,s32 rxbuflen,u32 arg)
{
	
	u8 senddata[100];
	u32 senddatalen = 0;
	u8 pinblockresultlen;
	s32 ret;
	u32 sendcmd = 0;
	
	
	memset(senddata,0,sizeof(senddata));
	
	switch(OperateInstruction)
	{
		
		case COMMAND_GETMAC:
		case COMMAND_GETDUKPTKMAC:
			ret = pcitask_calcmac_getmac(OperateInstruction,rxbuf,rxbuflen,&senddata[1]);
			senddatalen = 1;
			if(OperateInstruction == COMMAND_GETDUKPTKMAC)
			{
				sendcmd = COMMAND_GETDUKPTKMAC_RESP;
				pinblockresultlen = 18;
				
			}
			else
			{
				sendcmd = COMMAND_GETMAC_RESP;
				pinblockresultlen = 8;
				
			}
			
			if(ret == 0)
			{
				senddata[0] = 0;
			}
			else
			{
				pinblockresultlen = 0;
				pcitask_dealresult(ret,senddata);
			}
			senddatalen = 1 + pinblockresultlen;
			break;
			
		case COMMAND_GETFIXEDPIN:
		case COMMAND_GETDUKPTKPIN:
		case COMMAND_GETPIN:
			#if 0 //sxl?
			ret = dev_PciGetPin_encrypted(OperateInstruction,rxbuf,rxbuflen,&senddata[1]);
			senddatalen = 1;
			if(OperateInstruction == COMMAND_GETDUKPTKPIN)
			{
				sendcmd = COMMAND_GETDUKPTKPIN_RESP;
				pinblockresultlen = 18;
				
			}
			else if(OperateInstruction == COMMAND_GETFIXEDPIN)
			{
				sendcmd = COMMAND_GETFIXEDPIN_RESP;
				pinblockresultlen = 8;
			}
			else
			{
				sendcmd = COMMAND_GETPIN_RESP;
				pinblockresultlen = 8;
				
			}
			
			if(ret == 0)
			{
				senddata[0] = 0;
			}
			else
			{
				pinblockresultlen = 0;
				senddata[0] = ret;
			}
			senddatalen = 1 + pinblockresultlen;
			#endif
			break;
		
      case COMMAND_GETMAGENCRYPTEDDATA:
	  	    #if 0//sxl?
			ret = dev_PciGetMagdata_encrypted(OperateInstruction,rxbuf,rxbuflen,&senddata[1],&senddatalen);
			if(ret == 0)
			{
				senddata[0] = 0;
			}
			else
			{
				senddatalen = 0;
				pcitask_dealresult(ret,senddata);
			}
			
			senddatalen = senddatalen + 1;
			sendcmd = COMMAND_GETMAGENCRYPTEDDATA_RESP;
			#endif
			
			break;
			
			
		case COMMAND_CHECKPINKEYSTATUS:
			ret = pcitask_workkeystatus_check(NULL,rxbuf[0],rxbuf[1],rxbuf[2]);
		    if(ret == 0)
			{
				senddata[0] = 0;
			}
			else
			{
				senddatalen = 0;
				pcitask_dealresult(ret,senddata);
			}
			
			senddatalen = 1;
			sendcmd = COMMAND_CHECKPINKEYSTATUS_RESP;
			break;
			
		default:
			break;
		
	}
	
	
	pcitask_commu_SendPacket(sendcmd,senddata,senddatalen,arg);
	
	return 0;
	
	
}




/*******************************************************************
Author:   sxl
Function Name: s32 pcitask_delete_groupallkey(u8 *cmpdata)
Function Purpose:delete one group all key
Input Paramters: 
                        *cmpdata - compare data
Output Paramters:N/A
Return Value:    PCI_PROCESSCMD_SUCCESS  -      success
                       other - failed
Remark: N/A
Modify: N/A 	  1130	//20140327
********************************************************************/
s32 pcitask_delete_groupallkey(u8 *cmpdata)
{
	s32 ret;
	u8 groupindex;

    groupindex = cmpdata[0];
	ret = pcikeymanageauthen_posins_authen(PCIPOSAUTHEN_DELETEALLKEY,&cmpdata[1],0,NULL,0);
	
	#ifdef SXL_DEBUG
    DDEBUG("\r\n pcidrv_delete_allkey = %d \r\n",ret);
	#endif
	if(ret == PCI_PROCESSCMD_SUCCESS)
	{
	   pcikeymanage_groupkeyfile_initial(groupindex);
	}
	pcikeymanageauthen_securityauthinfo_initial();
	return ret;
}


/*******************************************************************
Author:   sxl
Function Name: s32 pcitask_delete_allkey(u8 *cmpdata)
Function Purpose:delete  all key
Input Paramters: 
                        *cmpdata - compare data
Output Paramters:N/A
Return Value:    PCI_PROCESSCMD_SUCCESS  -      success
                       other - failed
Remark: N/A
Modify: N/A 	  1130	//20140327
********************************************************************/
s32 pcitask_delete_allkey(u8 *cmpdata)
{
	s32 ret;
	
	ret = pcikeymanageauthen_posins_authen(PCIPOSAUTHEN_DELETEALLKEY,NULL,0,cmpdata,48);
	
	#ifdef SXL_DEBUG
    DDEBUG("\r\n pcidrv_delete_allkey = %d \r\n",ret);
	#endif
	if(ret == PCI_PROCESSCMD_SUCCESS)
	{
		pcikeymanage_initkeydata();
	}
	pcikeymanageauthen_securityauthinfo_initial();
	return ret;
	
}

/*******************************************************************
Author:   sxl
Function Name: s32 pcitask_process_downloadkeycmd(u8 *rxbuf,s32 rxbuflen,ulong arg)
Function Purpose:Processing download key command
Input Paramters: 
                        *rxbuf - Data from user space
                        rxbuflen  - Data len
                        arg   - user space pointer address
Output Paramters:If output parameters exist,by arg to the user space.
Return Value:    PCI_PROCESSCMD_SUCCESS  -      success
                       other - failed
Remark: N/A
Modify: N/A 	//sxl? 1130 dukptk  //20140327
********************************************************************/
s32 pcitask_process_downloadkeycmd(u32 OperateInstruction,u8 *rxbuf,s32 rxbuflen,u32 arg)
{
	//s32 OperateInstruction;
	u8 tmpdata[300];
	u8 resultdata[300];
	u32 resultlen;
	s32 iRet;
	u8 groupindex;
	s32 ret;
	//u8 i;          
    //u8 ksn[50],ksnlen;
	//u32 counter;
	//u8 templen;
	u8 hashvaluelen;
	u32 signdatalen;
    
	switch(OperateInstruction)
	{
	    //sxlremark
	    //请求下载密钥
	    //Download the key request
		case AUTHENTICATE_REQUEST:
			//sxlremark
			//生成48个字节随机数作为PTK和TMACK
			//generated 48-bytes random number 
        	pcicorecallfunc.pci_get_random_bytes(tmpdata,48);
			//sxl? 测试用，要删除
			//memset(tmpdata,0x11,48);
            
			//sxlremark
			//获取密钥组号
			//get download key group index
			groupindex = rxbuf[0];
			//sxlremark
			//检查要下载密钥功能是否被锁，检查密钥组号，检查将要使用的公钥证书是否存在
			//Check whether download the key functions are locked, check the key group number, check the public key certificate to be used if there
			if(groupindex != 0xFF)
			{
				ret = pcikeymanageauthen_posins_authen(PCIPOSAUTHEN_DOWNAPPKEY_REQUEST,&rxbuf[1],groupindex,tmpdata,48);
                gDownLoadKeyErr = ret;
				if(ret != PCI_PROCESSCMD_SUCCESS)
				{
				    //sxlremark
				    //认证失败，清除认证信息(清除PTK、TMACK 、置为没有认证状态)
				    //Authentication fails, clear the authentication information (clear PTK, TMACK, set to no authentication state)
				    pcikeymanageauthen_securityauthinfo_initial();     //initial load app key status
				    pcitask_dealresult(ret,resultdata);
					pcitask_commu_SendPacket(AUTHENTICATE_STEP2,resultdata,1,arg);
					return ret;
				}
				//sxlremark
				//认证成功，对PTK、TMACK算HASH值
				//Authentication is successful, calculated the PTK, TMACK  HASH value
				iRet = pciarith_rsa_checkrsatype(&rxbuf[1]);
				DDEBUG("\r\npcidrv_rsa_checkrsatype = %d\r\n",iRet);
				if(iRet == PCI_PROCESSCMD_SUCCESS)
				{
				    
			    	hashvaluelen = 32;
					signdatalen = 256;
					pciarith_hash(tmpdata,48,&tmpdata[48],HASHTYPE_256);
				   
					
					//sxl? 测试用，要删除
		            DDEBUG("\r\n downloadkey1 \r\n");
					//sxlremark
					//对PTK、TAMCK、HASH值使用对应的公钥进行加密
					iRet = pciarith_rsa_pkencrypt(NULL,&rxbuf[1],tmpdata,48+hashvaluelen,resultdata,&resultlen);  //sxl?
					
				    //sxl? 测试用，要删除
		            DDEBUG("\r\n downloadkey2 iRet = %d\r\n",iRet);

					
					
				}
			}
			else  //load magkey authenticate
			{
			    ret = pcikeymanageauthen_posins_authen(PCIPOSAUTHEN_DOWNMAGKEY_REQUEST,&rxbuf[1],groupindex,tmpdata,48);
                gDownLoadKeyErr = ret;
				if(ret != PCI_PROCESSCMD_SUCCESS)
				{
				    //sxlremark
				    //认证失败，清除认证信息(清除PTK、TMACK 、置为没有认证状态)
				    //Authentication fails, clear the authentication information (clear PTK, TMACK, set to no authentication state)
				    pcikeymanageauthen_securityauthinfo_initial();     //initial load app key status
				    pcitask_dealresult(ret,resultdata);
					pcitask_commu_SendPacket(AUTHENTICATE_STEP2,resultdata,1,arg);
					return ret;
				}
                
				
				iRet = pciarith_rsa_checkrsatype(&rxbuf[1]);
				if(iRet == PCI_PROCESSCMD_SUCCESS)
				{
					
			    	hashvaluelen = 32;
					signdatalen = 256;
					pciarith_hash(tmpdata,48,&tmpdata[48],HASHTYPE_256);
				    
					//sxl? 测试用，要删除
		            DDEBUG("\r\n downloadkey1 \r\n");
                    
					//sxlremark
					//对PTK、TAMCK、HASH值使用对应的公钥进行加密
					//sxl?
					//iRet = pcidrv_magkey_pkencrypt(NULL,&rxbuf[1],tmpdata,48+hashvaluelen,resultdata,&resultlen);
					iRet = KM_AUTH_ERROR;
					
					
				}
			}
			gDownLoadKeyErr = iRet;
			if(iRet != PCI_PROCESSCMD_SUCCESS)
			{
			    //sxlremark
			    //加密失败，清除认证信息(清除PTK、TMACK 置为没有认证)
			    //Authentication fails, clear the authentication information (clear PTK, TMACK, set to no authentication state)
			    pcikeymanageauthen_securityauthinfo_initial();     //initial load app key status
			    pcitask_dealresult(iRet,resultdata);
				pcitask_commu_SendPacket(AUTHENTICATE_STEP2,resultdata,1,arg);
			}
			else
			{
			    //sxlremark
			    //将加密后的数据给密钥下载机进行解密
			    //
				DDEBUG("\r\n downloadkeykk %02x \r\n",resultdata[0]);
				
				pcitask_commu_SendPacket(AUTHENTICATE_STEP2,resultdata,signdatalen,arg);
			}
			return iRet;
			break;
		case AUTHENTICATE_STEP3:
            if(rxbuf[0] != 0)
            {
            	pcikeymanageauthen_securityauthinfo_initial();     //initial load app key status
            	resultdata[0] = KM_OTHER_ERR;
				gDownLoadKeyErr = KM_OTHER_ERR;
				pcitask_commu_SendPacket(AUTHENTICATE_RESPOND,resultdata,1,arg);
				return KM_AUTH_ERROR;
            }
			else
			{    
			    groupindex = 0;  //sxl modify  //rxbuf[1]
			    ret = pcikeymanageauthen_posins_authen(PCIPOSAUTHEN_DOWNAPPKEY_AUTH,NULL,groupindex,&rxbuf[0],0);  //sxl?20151218
			    //ret = 0;
                gDownLoadKeyErr = ret;

				#ifdef SXL_DEBUG
                DDEBUG("\r\nAUTHENTICATE_STEP3 ret = %d\r\n",ret);
				#endif
				if(ret != PCI_PROCESSCMD_SUCCESS)
				{
				    
					pcikeymanageauthen_securityauthinfo_initial();     //initial load app key status
					//resultdata[0] = 1;
					pcitask_dealresult(ret,resultdata);
				    pcitask_commu_SendPacket(AUTHENTICATE_RESPOND,resultdata,1,arg);
				    return KM_AUTH_ERROR;
				}
                
				resultdata[0] = 0;
				pcitask_commu_SendPacket(AUTHENTICATE_RESPOND,resultdata,1,arg);
				return PCI_PROCESSCMD_SUCCESS;
			}
			break;
		case LOAD_POSAUTHENK_REQUEST:
			#ifdef SXL_DEBUG
            DDEBUG("\r\nLOAD_POSAUTHENK_REQUEST rxbuflen = %d\r\n",rxbuflen);
			#endif
			if(rxbuflen != 29)
			{
				iRet = KM_APP_KEY_WRITE_ERROR;
			}
			else
			{
				iRet = pcikeydownload_posauthenkey(&rxbuf[0]);
			}
			gDownLoadKeyErr = iRet;
			#ifdef SXL_DEBUG
            DDEBUG("\r\nLOAD_POSAUTHENK_REQUEST loadkey = %d\r\n",iRet);
			#endif
			pcikeymanageauthen_securityauthinfo_initial();
			if(iRet < 0)
			{
				pcitask_dealresult(iRet,resultdata);
			}
			else
			{
				resultdata[0] = (u8)(iRet&0xff);
			}
			pcitask_commu_SendPacket(LOAD_POSAUTHENK_RESP,resultdata, 1,arg);
            return iRet;
			break;
		case LOAD_APPKEY_REQUEST:
			#ifdef SXL_DEBUG

            DDEBUG("\r\nrxbuflen = %d\r\n",rxbuflen);
			#endif
			if(rxbuflen != 32)
			{
				iRet = KM_APP_KEY_WRITE_ERROR;
			}
			else
			{
				iRet = pcikeydownload_appkey(&rxbuf[0]);
			}
			gDownLoadKeyErr = iRet;
			#ifdef SXL_DEBUG
            DDEBUG("\r\nloadkey = %d\r\n",iRet);
			#endif
			pcikeymanageauthen_securityauthinfo_initial();
			if(iRet < 0)
			{
				pcitask_dealresult(iRet,resultdata);
			}
			else
			{
				resultdata[0] = (u8)(iRet&0xff);
			}
			pcitask_commu_SendPacket(LOAD_APPKEY_RESPOND,resultdata, 1,arg);
            return iRet;
			break;
		case COMAMND_SETGROUPKEYAPPNAME:
			if(rxbuflen != 69)
			{
			    DDEBUG("\r\n%s input parameter err\r\n",__FUNCTION__);
				iRet = KM_OTHER_ERR;
			}
			else
			{
            	iRet = pcikeydownload_groupkeyappname_set(&rxbuf[0]);
			}
			gDownLoadKeyErr = iRet;
			pcikeymanageauthen_securityauthinfo_initial();
			if(iRet < 0)
			{
				pcitask_dealresult(iRet,resultdata);
			}
			else
			{
				resultdata[0] = (u8)(iRet&0xff);
			}
			pcitask_commu_SendPacket(COMAMND_SETGROUPKEYAPPNAME_RESP,resultdata, 1,arg);
            return iRet;
			break;
		case COMMAND_DELETEALLKEY:
            ret = pcitask_delete_allkey(&rxbuf[0]);
			pcikeymanageauthen_securityauthinfo_initial();
			gDownLoadKeyErr = ret;
			if(ret == PCI_PROCESSCMD_SUCCESS)
			{
				resultdata[0] = 0;
				pcitask_commu_SendPacket(COMMAND_DELETEALLKEY_RESP,resultdata,1,arg);
			}
			else
			{
			    resultdata[0] = 1;
				pcitask_commu_SendPacket(COMMAND_DELETEALLKEY_RESP,resultdata,1,arg);
			}
			return ret;
			break;
		case COMMAND_DELETEGROUPALLKEY:
			ret = pcitask_delete_groupallkey(&rxbuf[0]);
			pcikeymanageauthen_securityauthinfo_initial();
			gDownLoadKeyErr = ret;
			if(ret == PCI_PROCESSCMD_SUCCESS)
			{
				resultdata[0] = 0;
				pcitask_commu_SendPacket(COMMAND_DELETEGROUPALLKEY_RESP,resultdata,1,arg);
			}
			else
			{
			    resultdata[0] = 1;
				pcitask_commu_SendPacket(COMMAND_DELETEGROUPALLKEY_RESP,resultdata,1,arg);
			}
			return ret;
			break;
			#ifdef SUPPORTDUKPTK
		case LOAD_APPDUKPTKEY_REQUEST:  //sxl
		    DDEBUG("\r\nLOAD_APPDUKPTKEY_REQUEST rxbuflen = %d\r\n",rxbuflen);
            if(rxbuflen != 49)
			{
				iRet = KM_APP_KEY_WRITE_ERROR;
			}
			else
			{
				iRet = pcikeydownload_appdukptkey(&rxbuf[0]);
			}
			gDownLoadKeyErr = iRet;
			resultdata[0] = (u8)(iRet&0xff);
			pcikeymanageauthen_securityauthinfo_initial();
			pcitask_commu_SendPacket(LOAD_APPKEY_RESPOND,resultdata, 1,arg);
            return iRet;
			break;
			#endif
			
		case COMMAND_LOADMAGKEY:
			#if 0
            ret = pci_dev_encryptedmag_readksn(ksn,&ksnlen,&counter);
			gDownLoadKeyErr = ret;
			if(ret != 0)
			{
				resultdata[0] = 13;
				pcitask_commu_SendPacket(COMMAND_LOADMAGKEY_RESP,resultdata, 1,arg);
				Lib_memset(resultdata,0,sizeof(resultdata));
				return PCI_READKEY_ERR;
			}
			//get KSN
			memcpy(&rxbuf[rxbuflen],ksn,ksnlen);
			
            iRet = pcidrv_prog_loadmagkey(&rxbuf[0],&resultdata[1],&resultlen);
			if(resultlen == 40)   //需要重新初始化磁头
			{
			    pcikeymanageauthen_securityauthinfo_initial();
				iRet = pci_dev_encryptedmag_readstatus();
                #ifdef SXL_DEBUG
				DDEBUG("\r\ninitial mag\r\n");
				#endif
				gDownLoadKeyErr = iRet;
				
				
				if(iRet != 0)
				{
					iRet = pci_InitMagHead(&resultdata[1+24],&resultdata[1]);
				}
				else
				{
					iRet = pci_ResetMagHead(&resultdata[1+24],&resultdata[1]);
				}
				if(iRet != 0)
				{
					resultdata[0] = 12;
				}
				else
				{
					resultdata[0] = 0;
				}
				pcitask_commu_SendPacket(COMMAND_LOADMAGKEY_RESP,resultdata, 1+resultlen,arg);	
			}
			else
			{
				pcikeymanageauthen_securityauthinfo_initial();
				gDownLoadKeyErr = iRet;
				if(iRet != PCI_PROCESSCMD_SUCCESS)
				{
					pcitask_dealresult(iRet,resultdata);
					//resultdata[0] = (u8)(iRet&0xff);
					pcitask_commu_SendPacket(COMMAND_LOADMAGKEY_RESP,resultdata, 1,arg);
					Lib_memset(resultdata,0,sizeof(resultdata));
				}
				else
				{
				    resultdata[0] = 0;
					pcitask_commu_SendPacket(COMMAND_LOADMAGKEY_RESP,resultdata, 1+resultlen,arg);
				}
			}
			return iRet;
			#endif
			break;
		case APPKEY_SETPINK:
            iRet =  pcikeydownload_setpink(&rxbuf[0],rxbuflen);
			resultdata[0] = (u8)(iRet&0xff);
			gDownLoadKeyErr = iRet;
			DDEBUG("\r\n setpink9 \r\n");
			pcitask_commu_SendPacket(APPKEY_SETPINK_RESPONSE,resultdata, 1,arg);
			DDEBUG("\r\n setpink10 \r\n");
			return iRet;
			break;
		case APPKEY_SETMACK:
			iRet =  pcikeydownload_setmack(&rxbuf[0],rxbuflen);
			resultdata[0] = (u8)(iRet&0xff);
			gDownLoadKeyErr = iRet;
			pcitask_commu_SendPacket(APPKEY_SETMACK_RESPONSE,resultdata, 1,arg);
			return iRet;
			break;
        case APPKEY_SETTDK:
			iRet =  pcikeydownload_settdk(&rxbuf[0],rxbuflen);
			resultdata[0] = (u8)(iRet&0xff);
			gDownLoadKeyErr = iRet;
			pcitask_commu_SendPacket(APPKEY_SETTDK_RESPONSE,resultdata, 1,arg);
			return iRet;
			break;
		
		default:
			break;
		
		
	}
	
	
	return KM_APP_KEY_WRITE_ERROR;
	
	
	
}




void pcitask_dealdpatest(u32 OperateInstruction,u8 *rxbuf,s32 rxbuflen,u32 arg)
{
	
    s32 ret = KM_OTHER_ERR;
    u8 keylen;
	u8 *key;
    u8 resultlen = 0;
	u8 result[9];
	
	
	keylen = rxbuf[0];
	key = &rxbuf[1];
	
    
	if((keylen+1+8 == rxbuflen)&&(keylen == 8||keylen == 16||keylen == 24)) 
	{
	    ret = 0;
		DES_TDES(key, keylen, &rxbuf[1+keylen], 8, 1);
	}
	result[0] = ret;
	resultlen = 1;
	if(ret == 0)
	{
		memcpy(&result[1],&rxbuf[1+keylen],8);
		resultlen = 9;
		
		#ifdef SXL_DEBUG
	    DDEBUG("\r\npcitask_dealdpatest:\r\n");
		for(ret = 0;ret < 8;ret++)
		{
			DDEBUG("%02x ",result[ret+1]);
		}
		DDEBUG("\r\n");
		#endif
		
	}
	pcitask_commu_SendPacket(COMMAND_DPATEST_RESP,result, resultlen,arg);
	
}



#ifdef SAVEPCILOG
s32 pcitask_deallogfile(u32 OperateInstruction,u8 *rxbuf,s32 rxbuflen,u32 arg)
{
	u8 *senddata;
	u32 senddatalen = 0;
	
	
	senddata = (u8 *)pcicorecallfunc.mem_malloc(1024);
	
	switch(OperateInstruction)
	{
		case COMMAND_LOGGETLENGTH:
			senddatalen = pcilog_getlogfilelength(senddata);
			break;
		case COMMAND_LOGGETDATA:
			senddatalen = pcilog_getlogfiledata(rxbuf,senddata);
			break;
		case COMMAND_LOGDELETE:
			senddatalen = pcilog_logfile_delete(senddata);
			break;
			
			
	}
	
	
	pcitask_commu_SendPacket(OperateInstruction,senddata,senddatalen,arg);


    pcicorecallfunc.mem_free(senddata);
	return 0;
	
	
}
#endif




void pcitask_dealcommanddata(unsigned int command,u8 *data,unsigned int datalen)
{
	//s32 ret;   //保存操作LOG     
    u32 tmplen;
	
	#ifdef SXL_DEBUG
    DDEBUG("\r\ncommand = %04x %d\r\n",command,datalen);
	#endif
	
	switch(command)
	{
		case AUTHENTICATE_REQUEST:
		case AUTHENTICATE_STEP3:
		case LOAD_APPKEY_REQUEST:
		case COMAMND_SETGROUPKEYAPPNAME:
		case COMMAND_DELETEALLKEY:
		case COMMAND_DELETEGROUPALLKEY:
		case APPKEY_SETPINK:
		case APPKEY_SETMACK:
		case APPKEY_SETTDK:
		#ifdef SUPPORTDUKPTK
		case LOAD_APPDUKPTKEY_REQUEST:
		#endif
		case COMMAND_LOADMAGKEY:  //sxl?
		case LOAD_POSAUTHENK_REQUEST:
		    //dev_encryptedmag_enable();   //sxl?
		    pcilog_record(command,data);
			pcitask_process_downloadkeycmd(command,data,datalen,0); 
            //dev_encryptedmag_disable();
			//pcihl_savekeymanagelog(ret); //sxl? 写内部FLASH的时候会关中断 //sxl?写文件的时候屏蔽了中断，要小心,会丢数据
		    break;
	    case COMMAND_GETRANDOMNUMBER:
            tmplen = data[0];
		    tmplen = (tmplen << 8) + data[1];
			#ifdef SXL_DEBUG
            DDEBUG("\r\nGet Random data %d\r\n",tmplen);
			#endif
            //pcihl_GetRandomNumbertouart(tmplen);  //sxl?
			break;
		case COMMAND_SYNCMAGKEY:
			//dev_encryptedmag_enable();  //sxl?
            //pcihl_magkeysync();
			//dev_encryptedmag_disable();
			break;
		case COMMAND_GETMAC:
		case COMMAND_GETDUKPTKMAC:
		case COMMAND_GETFIXEDPIN:
		case COMMAND_GETDUKPTKPIN:
		case COMMAND_GETPIN:
		case COMMAND_CHECKPINKEYSTATUS:
		case COMMAND_GETMAGENCRYPTEDDATA:
        	pcitask_dealsecurityservice(command,data,datalen,0);
			break;
		case COMMAND_LOGGETLENGTH:
		case COMMAND_LOGGETDATA:
		case COMMAND_LOGDELETE:
			#ifdef SAVEPCILOG
			pcitask_deallogfile(command,data,datalen,0);
			#endif
			break;
		case COMMAND_DPATEST:
			pcitask_dealdpatest(command,data,datalen,0);
			break;
		default:
			break;
		
	}
	
}



