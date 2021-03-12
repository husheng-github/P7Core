


#include "pciglobal.h"


/*******************************************************************
Author:   sxl
Function Name: s32 pciarith_rsa_readIPKdata(u8 checkmode,u8 *checkdata,RSA_PUBLICKEY_INFO *publickeyinfo)
Function Purpose:read public key information from 
Input Paramters: 
                        *IssuerSN - Issuer and serial number
Output Paramters:
                       *publickeyinfo - public key information
Return Value:    PCI_PROCESSCMD_SUCCESS  -      success
                       other - failed
Remark: N/A     sxl 
Modify: N/A 	   not use for module   //20140327
********************************************************************/  //sxl?
s32 pciarith_rsa_readIPKdata(u8 checkmode,u8 *checkdata,RSA_PUBLICKEY_INFO *readpublickeyinfo)  //sxl?
{
	
	return 1;
	
    #if 0
    //u8 tmphashvalue[32];
	//s8 pkfilename[32];
	u32 i;
	RSA_PUBLICKEY_INFO *publickeyinfo;
	#ifdef SXL_DEBUG
    u32 j;
	#endif
	s32 ret;
	
	
    #ifdef SXL_DEBUG
    DDEBUG("\r\n src check data:\r\n");
	for(j = 0;j < PUBLICKEY_ISSUER_SIZE+PUBLICKEY_SN_SIZE;j++)
	{
		DDEBUG("%02x ",checkdata[j]);
	}
	DDEBUG("\r\n");
	#endif
	
	
    publickeyinfo = (RSA_PUBLICKEY_INFO *)pcicorecallfunc.mem_malloc(sizeof(RSA_PUBLICKEY_INFO));
	
	for(i = 0;i < 4;i++)
	{
	    ret = pcipkmanage_readpk(PK_AUTHEN1+i,publickeyinfo);
		if(ret == PCI_PROCESSCMD_SUCCESS)
		{
			if(Lib_memcmp(publickeyinfo->Issuer,checkdata,PUBLICKEY_ISSUER_SIZE+PUBLICKEY_SN_SIZE) == 0)
		    {
		    	memcpy(readpublickeyinfo,publickeyinfo,sizeof(RSA_PUBLICKEY_INFO));
				pcicorecallfunc.mem_free(publickeyinfo);
				return 0;
		    }
		}
		/*
	    	memset(pkfilename,0,sizeof(pkfilename));
		sprintf(pkfilename,"/mtd2/authen%d",i);
		
		ret = pcicorecallfunc.pci_kern_read_file(pkfilename,(u8 *)publickeyinfo,PUBLICKEY_EPCERTSIGN_OFFSET,0);
		if(ret != PUBLICKEY_EPCERTSIGN_OFFSET)
		{
			continue;
		}
		memset(tmphashvalue,0,sizeof(tmphashvalue));
	    pciarith_hash((u8 *)publickeyinfo,PUBLICKEY_CERTSIGN_OFFSET,tmphashvalue,HASHTYPE_256);
        
		#ifdef SXL_DEBUG
	    DDEBUG("\r\nhash1:\r\n");
		for(j = 0;j < 32;j++)
		{
			DDEBUG("%02x ",tmphashvalue[j]);
		}
		DDEBUG("\r\n");

		DDEBUG("\r\nhash2:\r\n");
		for(j = 0;j < 32;j++)
		{
			DDEBUG("%02x ",publickeyinfo->CertSign[j]);
		}
		DDEBUG("\r\n");
        
		DDEBUG("\r\n Dst check data:\r\n");
		for(j = 0;j < 16;j++)
		{
			DDEBUG("%02x ",publickeyinfo->Issuer[j]);
		}
		DDEBUG("\r\n");
		#endif
		if(Lib_memcmp(publickeyinfo->CertSign,tmphashvalue,32) == 0)
		{
		    if(Lib_memcmp(publickeyinfo->Issuer,checkdata,PUBLICKEY_ISSUER_SIZE+PUBLICKEY_SN_SIZE) == 0)
		    {
		    	memcpy(readpublickeyinfo,publickeyinfo,sizeof(RSA_PUBLICKEY_INFO));
				pcicorecallfunc.mem_free(publickeyinfo);
				return 0;
		    }
			
		}
		*/
		
	}

    pcicorecallfunc.mem_free(publickeyinfo);
	return 1;
	#endif
	
}



////20140327
s32 pciarith_rsa_checkrsatype(u8 *IssuerSN)
{
	s32 ret;
	RSA_PUBLICKEY_INFO *tmppublickeyinfo;
	u32 pkdatalen;
	

    
	tmppublickeyinfo = (RSA_PUBLICKEY_INFO *)pcicorecallfunc.mem_malloc(sizeof(RSA_PUBLICKEY_INFO));
	if(tmppublickeyinfo == NULL)
	{
		return PCI_KMALLOCMEM_ERR;
	}

    
	ret = pciarith_rsa_readIPKdata(0,IssuerSN,tmppublickeyinfo);
    if(ret != PCI_PROCESSCMD_SUCCESS)
    {
        pcicorecallfunc.mem_free(tmppublickeyinfo);
    	return ret;
    }
    
	pkdatalen = *((u32 *)tmppublickeyinfo->IPKData);
    
	if(pkdatalen == 2048)
	{
	    
	}
	else
	{
		return -1;
	}

    
	pcicorecallfunc.mem_free(tmppublickeyinfo);
	return ret;
	
	
}


/*******************************************************************
Author:   sxl
Function Name: s32 pciarith_rsa_encryptalgorithm(u8 EncryptMode,u8 *PKdata,u8 *originaldata,u32 originaldatalen,u8 *encrypteddata,u32 *encrypteddatalen)
Function Purpose:use public key to encrypt or dencrypt data with rsa arithmetic
Input Paramters: EncryptMode  - ENCRYPT or DECRYPT
                        *PKdata - Public key data
                        *originaldata - original data need to encrypted
                        originaldatalen - original data len
Output Paramters:
                       *encrypteddata - output data after encrypted
                       *encrypteddatalen - output data len
Return Value:    PCI_PROCESSCMD_SUCCESS  -      success
                       other - failed
Remark: N/A
Modify: N/A 	   not use for module  //20140327
********************************************************************/
s32 pciarith_rsa_encryptalgorithm(u8 EncryptMode,u8 *PKdata,u8 *originaldata,u32 originaldatalen,u8 *encrypteddata,u32 *encrypteddatalen)
{
	s32 blockoffset;
    s32 dataoffset;
	u32 encryptdatalen;
	u8 tmpdata[256];
	s32 ret;
	
	
	u32 pkdatalen;
	u32 encryptblocklen;
    
	
    
    //pkdatalen = *((u32 *)PKdata);
    pkdatalen = PKdata[3];
	pkdatalen = (pkdatalen<<8) + PKdata[2];
	pkdatalen = (pkdatalen<<8) + PKdata[1];
	pkdatalen = (pkdatalen<<8) + PKdata[0];
	
	
	if(pkdatalen == 1024)
	{
	    encryptblocklen = 128;
		pkdatalen = IPKDATALEN;
	}
	else if(pkdatalen == 2048)
	{
	    encryptblocklen = 256;
		pkdatalen = 292;
	}
	else
	{
		return PCI_RSACALCULATE_ERR;
	}
	
	
	blockoffset = 0;
	
	if(EncryptMode == ENCRYPT)
    {
		for(dataoffset = 0;dataoffset<originaldatalen;dataoffset += (encryptblocklen-11),blockoffset += encryptblocklen)
		{
			if((originaldatalen - dataoffset) <(encryptblocklen-11))
			{
				memset(tmpdata,0,(encryptblocklen-11));
				memcpy(tmpdata,&originaldata[dataoffset],(originaldatalen-dataoffset));
				encryptdatalen = originaldatalen-dataoffset;
			}
			else
			{
				memcpy(tmpdata,&originaldata[dataoffset],(encryptblocklen-11));
				encryptdatalen = (encryptblocklen-11);
			}
			
			#ifdef RSASOFTUSED
			ret = RSA_PKEncrypt(&encrypteddata[blockoffset],&encryptdatalen,tmpdata,encryptdatalen,PKdata,pkdatalen);
			#else
			ret = pcicorecallfunc.RSA_PKEncrypt(&encrypteddata[blockoffset],&encryptdatalen,tmpdata,encryptdatalen,PKdata,pkdatalen);
			#endif
			if(ret != 0)
			{
				return PCI_RSACALCULATE_ERR;
			}
		}

		*encrypteddatalen = blockoffset;
	}
	else
	{
		for(dataoffset = 0;dataoffset<originaldatalen;dataoffset += encryptblocklen)
		{
			if((originaldatalen - dataoffset) <encryptblocklen)
			{
				memset(tmpdata,0,encryptblocklen);
				memcpy(tmpdata,&originaldata[dataoffset],(originaldatalen-dataoffset));
				encryptdatalen = originaldatalen-dataoffset;
			}
			else
			{
				memcpy(tmpdata,&originaldata[dataoffset],encryptblocklen);
				encryptdatalen = encryptblocklen;
			}

			#ifdef RSASOFTUSED
			ret = RSA_PKDecrypt(&encrypteddata[blockoffset],&encryptdatalen,tmpdata,encryptdatalen,PKdata,pkdatalen);
            #else
			ret = pcicorecallfunc.RSA_PKDecrypt(&encrypteddata[blockoffset],&encryptdatalen,tmpdata,encryptdatalen,PKdata,pkdatalen);
			#endif
			if(ret != 0)
			{
				return PCI_RSACALCULATE_ERR;
			}

			blockoffset += encryptdatalen;
		}

		*encrypteddatalen = blockoffset;
	}

	return PCI_PROCESSCMD_SUCCESS;
	
}


/*******************************************************************
Author:   sxl
Function Name: s32 pciarith_rsa_pkencrypt(u8 *appname,u8 ,u8 *originaldata,u32 originaldatalen,u8 *encrypteddata,u32 *encrypteddatalen)
Function Purpose:use public key to encrypt data with rsa arithmetic
Input Paramters: *appname  - app name
                        *IssuerSN - Issuer and serial number
                        *originaldata - original data need to encrypted
                        originaldatalen - original data len
Output Paramters:
                       *encrypteddata - output data after encrypted
                       *encrypteddatalen - output data len
Return Value:    PCI_PROCESSCMD_SUCCESS  -      success
                       other - failed
Remark: N/A     //sxl 0521
Modify: N/A 		//20140327
********************************************************************/
s32 pciarith_rsa_pkencrypt(u8 *appname,u8 *IssuerSN,u8 *originaldata,u32 originaldatalen,u8 *encrypteddata,u32 *encrypteddatalen)
{
	s32 ret;
	RSA_PUBLICKEY_INFO *tmppublickeyinfo;

    
	tmppublickeyinfo = (RSA_PUBLICKEY_INFO *)pcicorecallfunc.mem_malloc(sizeof(RSA_PUBLICKEY_INFO));
	if(tmppublickeyinfo == NULL)
	{
		return PCI_KMALLOCMEM_ERR;
	}

    
	ret = pciarith_rsa_readIPKdata(0,IssuerSN,tmppublickeyinfo);
    if(ret != PCI_PROCESSCMD_SUCCESS)
    {
        pcicorecallfunc.mem_free(tmppublickeyinfo);
    	return ret;
    }

	ret = pciarith_rsa_encryptalgorithm(ENCRYPT,tmppublickeyinfo->IPKData,originaldata,originaldatalen,encrypteddata,encrypteddatalen);

	pcicorecallfunc.mem_free(tmppublickeyinfo);
	return ret;
	
	
}

