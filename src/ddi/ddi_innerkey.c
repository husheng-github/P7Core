#include "devglobal.h"
#include "pciglobal.h"
#include "ddi_innerkey.h"
//#include "ddi_manage.h"
#include "ddi_arith.h"

extern PCI_FIRMWARE_API *pcicore;

s32 ddi_innerkey_open (void)
{
	return DDI_OK;
}


s32 ddi_innerkey_close(void)
{
	return DDI_OK;
}


s32 ddi_innerkey_delete(u32 nKeygroupindex,u32 nKeytype,u32 nIndex)
{
   #if 0    
	s32 ret;
	
	ret = pcicore->pcifirmwareapi_deleteppkey(nKeygroupindex,nKeytype,nIndex);
    if(ret == PCI_PROCESSCMD_SUCCESS)
    {
    	return DDI_OK;
    }
	else
	{
		return DDI_ERR;
	}
  #else
    return DDI_ERR;
  #endif
}


void ddi_DES_process(u8 *key,u8 keylen,u8 *dat,u8 mode)
{
	if(keylen == 8)  //des
	{
		ddi_arith_des(mode,dat,key);
	}
    else if(keylen == 16)
    {
    	if(mode == DDI_DES_DECRYPT)
    	{
    		
    		ddi_arith_des(DDI_DES_DECRYPT,dat,key);
			ddi_arith_des(DDI_DES_ENCRYPT,dat,&key[8]);
			ddi_arith_des(DDI_DES_DECRYPT,dat,key);
			
    	}
		else
		{
			ddi_arith_des(DDI_DES_ENCRYPT,dat,key);
			ddi_arith_des(DDI_DES_DECRYPT,dat,&key[8]);
			ddi_arith_des(DDI_DES_ENCRYPT,dat,key);
		}
    }
	else if(keylen == 24)
	{
		if(mode == DDI_DES_DECRYPT)
		{
			ddi_arith_des(DDI_DES_DECRYPT,dat,key);
            ddi_arith_des(DDI_DES_ENCRYPT,dat,&key[8]);
            ddi_arith_des(DDI_DES_DECRYPT,dat,&key[16]);
		}
		else
		{
			ddi_arith_des(DDI_DES_ENCRYPT,dat,key);
            ddi_arith_des(DDI_DES_DECRYPT,dat,&key[8]);
            ddi_arith_des(DDI_DES_ENCRYPT,dat,&key[16]);
		}
	}

	
}


void ddi_DES(u8 *key,u8 keylen,u8 *dat,u32 datalen,u8 mode)
{
	u32 i;
    for(i = 0;i < datalen;i += 8)
    {
    	ddi_DES_process(key,keylen,&dat[i],mode);
    }
	
}


s32 ddi_innerkey_inject (u32 nKeygroupindex,u32 nKeytype,u32 nIndex, const u8* lpKeyData, u32 lpKeyDatalen,u32 ArithType)
{
  #if 0	
	s32 ret,retCode;
	u8 checkvaluelen;
	u8 keydata[32];
	u8 tempdata[8];
    
    
	dev_debug_printf("\r\ninnerkey_inject:%d %d %d %d %d\r\n",nKeygroupindex,nKeytype,nIndex,lpKeyDatalen,ArithType);
	
	if(nKeytype == TYPE_TMK)  //保存主密钥
	{
		ret = pcicore->pcifirmwareapi_saveappkey(nKeygroupindex,nKeytype,nIndex,lpKeyDatalen,(u8 *)lpKeyData);
	}
	else    // 保存工作密钥
	{
		if(ArithType == WKTYPE_TMKENCRYPTED_WITHCHECKVLAUE)
	    {
	        checkvaluelen = lpKeyDatalen%8;
	        if(checkvaluelen == 0)
	        {
	            checkvaluelen = 8;
	        }
			lpKeyDatalen -= checkvaluelen;
	    }
		if(lpKeyDatalen != 8&&lpKeyDatalen != 16&&lpKeyDatalen != 24)
		{
			
			dev_debug_printf("pcikey","\r\ndownload wk len error\r\n");
			return DDI_EDATA;
			
		}
		
		
		memcpy(keydata,lpKeyData,lpKeyDatalen);
	    if(ArithType == WKTYPE_TMKENCRYPTED_WITHCHECKVLAUE)
	    {
	        ret = PCI_WRITEKEY_ERR;
			retCode = ddi_innerkey_decrypt(0,TYPE_TMK,nKeygroupindex,keydata,lpKeyDatalen,0);
			dev_debug_printf("\r\nddi_innerkey_decrypt retCode = %d\r\n",retCode);
			if(retCode == 0)
			{
			    memset(tempdata,0,sizeof(tempdata));
				ddi_DES(keydata,lpKeyDatalen,tempdata,8,DDI_DES_ENCRYPT);
				dev_debug_printformat("mac value:",tempdata,8);
				if(memcmp(tempdata,&lpKeyData[lpKeyDatalen],checkvaluelen) == 0)  //check vaule success
				{
				    
					dev_debug_printformat("inject wk:",keydata,lpKeyDatalen);
					retCode = pcicore->pcifirmwareapi_saveappkey(0,nKeytype,nIndex,lpKeyDatalen,keydata);
	                dev_debug_printf("inject wk ret = %d",ret);
					if(retCode == 0)
					{
						ret = PCI_PROCESSCMD_SUCCESS;
					}
					
					
				}
				
			}
			
	    }
		else if(ArithType == WKTYPE_TMKENCRYPTED_NOCHECKVALUE)
		{
			ret = PCI_WRITEKEY_ERR;
			retCode = ddi_innerkey_decrypt(0,TYPE_TMK,nKeygroupindex,keydata,lpKeyDatalen,0);
			if(retCode == 0)
			{
				dev_debug_printf("\r\nno check value key inject:%d %d %d\r\n",nKeytype,nIndex,lpKeyDatalen);
				dev_debug_printformat("plain key:\r\n",keydata,lpKeyDatalen);
				retCode = pcicore->pcifirmwareapi_saveappkey(0,nKeytype,nIndex,lpKeyDatalen,keydata);
		        dev_debug_printf("inject wk ret1 = %d",retCode);
				if(retCode == 0)
				{
					ret = PCI_PROCESSCMD_SUCCESS;
				}
			}
			
		}
		else if(ArithType == WKTYPE_PLAIN)
		{
		    
			retCode = pcicore->pcifirmwareapi_saveappkey(0,nKeytype,nIndex,lpKeyDatalen,keydata);
			dev_debug_printf("inject wk ret2 = %d",retCode);
			if(retCode == 0)
			{
				ret = PCI_PROCESSCMD_SUCCESS;
			}
			
			
		}
		
		
	}

	
    if(ret == PCI_PROCESSCMD_SUCCESS)
    {
    	return DDI_OK;
    }
	else
	{
		return DDI_ERR;
	}
  #else
    return DDI_ERR;
  #endif
	
}

//extern const u8 gArAcquirerTMKPrivatekeyInfo[];
extern const u8 gArAcquirerTMKPublickeyInfo[];
//extern const u8 gUSBbootPrivatekeyInfo[];

s32 ddi_innerkey_encrypt(u32 nKeygroupindex,u32 nKeytype,u32 nIndex, u8* lpKeyData, u32 lpKeyDatalen,u32 ArithType)
{
  #if 0
	u8 keydata[24];
	u8 keylen;
	s32 ret,i,j,k;
    u8 tmp[8];
	u8 tmp16[16];    
	u8 tmp32[32];
#if 0 //密钥机加密程序。
	if(0xF1 == nKeytype)//RSA加密方式。
	{
		memset(output, 0, 256);
		if(0 == nIndex)//主密钥密文加密。
		{
			ret = dev_RSA_SKEncrypt(output, &outputlen, lpKeyData, lpKeyDatalen, (u8*)gArAcquirerTMKPrivatekeyInfo, 1156);
		}
		else if(1 == nIndex) //usb下载随机数加密。
		{
			ret = dev_RSA_SKEncrypt(output, &outputlen, lpKeyData, lpKeyDatalen, (u8*)gUSBbootPrivatekeyInfo, 1156);
		}
		 if(ret == 0)
	     {	
	     	memcpy(lpKeyData,output,outputlen);
			//dev_debug_printf("%s(%d):\r\n", __FUNCTION__, __LINE__);
	     	return DDI_OK;
		 }
		 else
		 {		
			dev_debug_printf("%s(%d):\r\n", __FUNCTION__, __LINE__);
			return DDI_ERR;
		 }
	}
#endif	
	if(lpKeyDatalen%8 != 0)
    {
        return DDI_EINVAL;
    }    

	ret = pcicore->pcifirmwareapi_readandcheckappkey(nKeygroupindex,nKeytype,nIndex,&keylen,keydata);
  #if 0
    dev_debug_printf("%s(%d):ret = %d %d\r\n", __FUNCTION__, __LINE__,ret,lpKeyDatalen);
	dev_debug_printf("key = %d:\r\n", __FUNCTION__, __LINE__,keylen);
    dev_debug_printformat(NULL, keydata, keylen);
    
	dev_debug_printf("data:\r\n", __FUNCTION__, __LINE__);
    dev_debug_printformat(NULL, lpKeyData, lpKeyDatalen);
  #endif  
	if(ret == PCI_PROCESSCMD_SUCCESS)
	{
        if(ArithType == 0)
        {   //ECB算法
    		for(i=0; i < lpKeyDatalen; i+=8)
    	    {
    	        DES_TDES(keydata, keylen,lpKeyData+i,8,ENCRYPT);
    	    }
        }
        else if(ArithType == 1)
        {   //  CBC算法
            memset(tmp, 0, 8);
            for(i=0; i<lpKeyDatalen; i+=8)
            {
                for(j=0; j<8; j++)
                {
                    tmp[j]^=lpKeyData[i+j];
                }
                DES_TDES(keydata, keylen,tmp,8,ENCRYPT);
                memcpy(lpKeyData+i, tmp, 8);
            }
        }
        else if(ArithType == 2)
        {   //E919算法
            memset(tmp, 0, 8);
            i = 0;
            k = lpKeyDatalen>>3;        //加密次数
            for(; k>0; k--)
            {
                for(j=0; j<8; j++)
                {
                    tmp[j] ^= lpKeyData[i++];
                }
                if(k == 1)
                {
                    //最后一次加密
                    DES_TDES(keydata, 8,tmp,8,ENCRYPT);
                }
                else
                {
                    DES_TDES(keydata, keylen,tmp,8,ENCRYPT); 
                }
            }
            memcpy(lpKeyData, tmp, 8);
        }
        else if(ArithType == 3)
        {   //E99算法
            memset(tmp, 0, 8);
            i = 0;
            k = lpKeyDatalen>>3;        
            for(; k>0; k--)
            {
                for(j=0; j<8; j++)
                {
                    tmp[j] ^= lpKeyData[i++];
                }
                DES_TDES(keydata, 8,tmp,8,ENCRYPT);
            }
            memcpy(lpKeyData, tmp, 8);
        }
		else if(ArithType == 4)//基于SM4的硬件序列号加密算法
		{
			//SM4
			if(lpKeyDatalen%16 != 0)
			{
				return DDI_EINVAL;
			}    
			memset(tmp16, 0, 16);
            memset(tmp32,0,32);
            for(i=0; i<lpKeyDatalen; i+=16)
            {
                for(j=0; j<16; j++)
                {
                    tmp16[j]^=lpKeyData[i+j];
                }
            }
            //扩展成32个字节。
			dev_maths_bcd_to_asc(tmp32,tmp16,16);			
			ret = dev_sm4_deal(ENCRYPT, 0, tmp32, 16, keydata, keylen, NULL,tmp32);
			if(ret != 0)
			{
                dev_debug_printf("%s(%d):\r\n", __FUNCTION__, __LINE__);
				return DDI_ERR;
			}
			for(i = 0; i < 16; i++)
			{
				tmp32[i] ^= tmp32[i+16];
			}			
			ret = dev_sm4_deal(ENCRYPT, 0, tmp32, 16, keydata, keylen, NULL,tmp32);
			if(ret != 0)
			{
                dev_debug_printf("%s(%d):\r\n", __FUNCTION__, __LINE__);
				return DDI_ERR;
			}
            memcpy(lpKeyData, tmp32, 16);
		}
        else
        {
            dev_debug_printf("%s(%d):\r\n", __FUNCTION__, __LINE__);
            ret = PCI_INPUTPARAMS_ERR;  //参数错误
        }
	}
    memset(keydata,0,sizeof(keydata));
  #if 0	
	dev_debug_printf("%s(%d):data after encrypted:%d\r\n", __FUNCTION__, __LINE__,ret);
	dev_debug_printformat(NULL, lpKeyData, 8);
 #endif
 
	if(ret == PCI_PROCESSCMD_SUCCESS)
	{
		return DDI_OK;
	}
    else
    {
    	return DDI_ERR;
    }
  #else
    return DDI_ERR;
  #endif
	
}


s32 ddi_innerkey_decrypt(u32 nKeygroupindex,u32 nKeytype,u32 nIndex, u8* lpKeyData, u32 lpKeyDatalen,u32 ArithType)
{
  #if 0
	u8 keydata[24];
	u8 keylen;
	s32 ret,i;
	#if 1
	u8 output[256];
    u32 outputlen;
	u8 pk[300];

	if(0xF1 == nKeytype)//RSA解密方式。
	{
		
		//ret = pcipkmanage_readpkdata(PK_USBBOOT, pk);	
		memset(output, 0, 256);
        ret = RSA_PKDecrypt(output, &outputlen, lpKeyData,  lpKeyDatalen, (u8*)gArAcquirerTMKPublickeyInfo, 292);
		 if(ret == 0)
	     {	
	     	memcpy(lpKeyData,output,outputlen);
			//dev_debug_printf("%s(%d):\r\n", __FUNCTION__, __LINE__);
	     	return DDI_OK;
		 }
		 else
		 {		
			dev_debug_printf("%s(%d):\r\n", __FUNCTION__, __LINE__);
			return DDI_ERR;
		 }
	}
	#endif
	if(lpKeyDatalen%8 != 0)
    {
        return DDI_EINVAL;
    }    

	ret = pcicore->pcifirmwareapi_readandcheckappkey(nKeygroupindex,nKeytype,nIndex,&keylen,keydata);
	if(ret == PCI_PROCESSCMD_SUCCESS)
	{
		for(i=0; i < lpKeyDatalen; )
	    {
	        DES_TDES(keydata, keylen,lpKeyData+i,8,DECRYPT);
	        i += 8;          
	    }
	}
    memset(keydata,0,sizeof(keydata));
	
	if(ret == PCI_PROCESSCMD_SUCCESS)
	{
		return DDI_OK;
	}
    else
    {
    	return DDI_ERR;
    }
  
  #else
    return DDI_ERR;
  #endif
}


static s32 ddi_inndrkey_mac_x919(u32 keytype, u32  index, u8 *wbuf, u32 wlen, u8 *rbuf)
{
  #if 0
	s32 ret;
	u8 keydata[24];
	u8 keylen = 16;
	s32 i,j,k;
	s32 lastdes=0;
    
	
	ret = pcicore->pcifirmwareapi_readandcheckappkey(0,keytype,index,&keylen,keydata);
	if(ret != PCI_PROCESSCMD_SUCCESS)
	{
		return DDI_ERR;
	}

	k = 0;
	memset(rbuf, 0, 8);
	for(i=0; i<wlen; i+=8)
	{
		for(j=0; j<8; j++)
		{
			rbuf[j] = rbuf[j]^wbuf[i+j];
		}
		lastdes++;
		if(lastdes == wlen/8)
		{
			ret = drv_tdes(ENCRYPT, rbuf, 8, keydata, keylen);
		}
		else
		{
			ret = drv_tdes(ENCRYPT, rbuf, 8, keydata, 8);
		}
		if(ret != 0)
		{
			return DDI_ERR;
		}
		
	}
	return DDI_OK;
  #else
    return DDI_ERR;
  #endif
	
}

static s32 ddi_inndrkey_mac_x9_9(u32 keytype, u32  index, u8 *wbuf, u32 wlen, u8 *rbuf)
{
  #if 0
	s32 ret;
	u8 keydata[24];
	u8 keylen = 16;
	s32 i,j;//,k;
	
	ret = pcicore->pcifirmwareapi_readandcheckappkey(0,keytype,index,&keylen,keydata);
	if(ret != PCI_PROCESSCMD_SUCCESS)
	{
		return DDI_ERR;
	}
	memset(rbuf, 0, 8);
	for(i=0; i<wlen; i+=8)
	{
		for(j=0; j<8; j++)
		{
			rbuf[j] = rbuf[j]^wbuf[i+j];
		}
		ret = drv_tdes(ENCRYPT, rbuf, 8, keydata, keylen);
		if(ret != 0)
		{
			return DDI_ERR;
		}		
	}
	return DDI_OK;
  #else
    return DDI_ERR;
  #endif
	
}
const u8 FILE_CERT_CSR[] =   "/mtd0/cercsr.bin";
const u8  FILE_RSA_PK[]  =   "/mtd0/rst_pk.bin";
const u8  FILE_RSA_SK[]  =   "/mtd0/rst_sk.bin";
//#define FILE_CERT_LENMAX     4096
static s32 ddi_innerkey_injectcert(str_injectcert_t *lp_injectcert)
{
  #if 1
    u32 ret;
    u8 tmp[8];
    const u8 *filename;

    if(lp_injectcert->m_type == 0)
    {
        filename = FILE_CERT_CSR;
    }
    else if(lp_injectcert->m_type == 1)
    {
        filename = FILE_RSA_PK;
    }
    else if(lp_injectcert->m_type == 2)
    {
        filename = FILE_RSA_SK;
    }
    else
    {
        dev_debug_printf("%s(%d):\r\n", __FUNCTION__, __LINE__);
        return DDI_ERR;
    }
    if((lp_injectcert->m_offset >= lp_injectcert->m_filelen) || 
       ((lp_injectcert->m_offset+lp_injectcert->m_len) > lp_injectcert->m_filelen))
    {
        dev_debug_printf("%s(%d):\r\n", __FUNCTION__, __LINE__);        
        return DDI_ERR;
    }
    if(lp_injectcert->m_offset == 0)        //第一包,需填入总长度
    {
//dev_debug_printf("\r\nfilename=%s, filelen=%d\r\n", filename, lp_injectcert->m_filelen);        
        tmp[0] = (lp_injectcert->m_filelen>>8)&0xff;
        tmp[1] = (lp_injectcert->m_filelen)&0xff;
        ret = ddi_file_write(filename, tmp, 2);
        if(ret != 2)
        {
            dev_debug_printf("%s(%d):\r\n", __FUNCTION__, __LINE__);
            return DDI_ERR;  
        }
    }
//dev_debug_printf("\r\n offset=%d\r\n", lp_injectcert->m_offset);        
//dev_printformat(NULL, lp_injectcert->m_data, 16);
    ret = ddi_file_insert(filename, lp_injectcert->m_data, lp_injectcert->m_len, lp_injectcert->m_offset+2);
    if(ret != lp_injectcert->m_len)
    {
        dev_debug_printf("%s(%d):\r\n", __FUNCTION__, __LINE__);
        return DDI_ERR;
    }
/*    memset(lp_injectcert->m_data, 0, 64);   
dev_debug_printf("%s(%d):\r\n", __FUNCTION__, __LINE__);     
dev_printformat(NULL, lp_injectcert->m_data, 16);
    ret = ddi_file_read(filename, lp_injectcert->m_data, lp_injectcert->m_len+2, 0);    
dev_debug_printf("%s(%d):\r\n", __FUNCTION__, __LINE__); 
dev_printformat(NULL, lp_injectcert->m_data, 16);*/

    return DDI_OK;
  #else
    return DDI_ERR;
  #endif
}

static s32 ddi_innerkey_getcert(u8 type, str_injectcert_t *lp_injectcert)
{
  #if 0
    u32 ret;
    u8 tmp[2];
    const u8 *filename;

    if(lp_injectcert->m_type == 0)
    {
        filename = FILE_CERT_CSR;
    }
    else if(lp_injectcert->m_type == 1)
    {
        filename = FILE_RSA_PK;
    }
    else if(lp_injectcert->m_type == 2)
    {
        if(type == 0)
        {
            dev_debug_printf("%s(%d):\r\n", __FUNCTION__, __LINE__);
            return DDI_ERR;
        }
        else
        {
            filename = FILE_RSA_SK; 
        }
//        return DDI_ERR;
    }
    else
    {
        dev_debug_printf("%s(%d):\r\n", __FUNCTION__, __LINE__);        
        return DDI_ERR;
    }
    ret = ddi_file_read(filename, tmp, 2, 0);
    if(ret != 2)
    {
        dev_debug_printf("%s(%d):ret = %d\r\n", __FUNCTION__, __LINE__, ret); 
        return DDI_ERR;
    }

    lp_injectcert->m_filelen = (((u16)tmp[0])<<8) + tmp[1];
    if(lp_injectcert->m_offset >= lp_injectcert->m_filelen)
    {
        dev_debug_printf("%s(%d):offset=%d,filelen=%d\r\n", __FUNCTION__, __LINE__, lp_injectcert->m_offset, lp_injectcert->m_filelen); 
        return DDI_ERR;
    }
    
    ret = ddi_file_read(filename, lp_injectcert->m_data, lp_injectcert->m_len, lp_injectcert->m_offset+2);
    if((ret <= 0) || (ret > lp_injectcert->m_len))
    {
        dev_debug_printf("%s(%d):ret=%d, m_len=%d\r\n", __FUNCTION__, __LINE__, ret, lp_injectcert->m_len); 
        return DDI_ERR;
    }
//dev_debug_printf("\r\n filename=%s,filelen=%d,offset=%d\r\n", filename, lp_injectcert->m_filelen, lp_injectcert->m_offset);
//dev_printformat(NULL, lp_injectcert->m_data, 16);
    lp_injectcert->m_len = ret;
    return DDI_OK;
  #else
    return DDI_ERR;
  #endif
}


//对称算法
static s32 innerkey_cryptography_symmetric(INNERKEY_CRYPTOGRAPHY_t* lp_cpyptography)
{
  #if 0
    s32 ret = DDI_EINVAL;
    u8 keydata[24];
    u8 keylen;
    u8 encrypt;
    u8 mod;
    
    if((lp_cpyptography->m_datainlen&0x07) != 0)
    {
        return DDI_EINVAL;
    }

    ret = pcicore->pcifirmwareapi_readandcheckappkey(lp_cpyptography->m_group,lp_cpyptography->m_type,lp_cpyptography->m_index,&keylen,keydata);
    if(ret != PCI_PROCESSCMD_SUCCESS)
    {
        return DDI_ERR;
    }
    if(lp_cpyptography->m_mod&0x80)
    {
        //加密
        encrypt = ENCRYPT;
    }
    else
    {
        //解密
        encrypt = DECRYPT;
    }
    mod = lp_cpyptography->m_mod&0x0F; 
    if(mod==0)
    {
        //SM4算法
        ret = dev_sm4_deal(encrypt, 0, lp_cpyptography->m_datain, lp_cpyptography->m_datainlen, keydata, keylen, NULL, lp_cpyptography->m_datain);
    }
    else if(mod==1)
    {
        //DES算法
        ret = drv_tdes(encrypt, lp_cpyptography->m_datain, lp_cpyptography->m_datainlen, keydata, keylen);
    }
    
    if(ret != 0)
    {
        return DDI_ERR;
    }
    else
    {
        return DDI_OK;
    }
  #else
    return DDI_ERR;
  #endif
    
}
//RSA算法
#define RSA_KEY_SPACE_MAX    512
static s32 innerkey_cryptography_RSA(INNERKEY_CRYPTOGRAPHY_t* lp_cpyptography)
{
  #if 1
    s32 ret = DDI_EINVAL;
    u8 encrypt;
    u8 rsatype;
    s32 i;
    u8 *rsakey=NULL;
    u8 *sk;
    u8 *pk;
    s32 rsakeylen = 0;
    u32 rlen;
    u8 mod;
    str_injectcert_t l_injectcert;

    if(lp_cpyptography->m_mod&0x80)
    {
        //加密
        encrypt = ENCRYPT;
    }
    else
    {
        //解密
        encrypt = DECRYPT;
    }
    i = (lp_cpyptography->m_mod&0x0f);
    if(i == 2)
    {
        //公钥
        rsatype = 0;
    }
    else if(i == 3)
    {
        //私钥
        rsatype = 1;
		return DDI_ERR;  //sxl 去 flash的不支持私钥解密
    }
    else
    {
        return DDI_ERR;
    }
    rsakey = k_malloc(RSA_KEY_SPACE_MAX);
    if(rsakey==NULL)
    {
        return DDI_ERR;
    }
    sk = rsakey;
    pk = rsakey;
    mod = 1;
    while(1)
    {
        //根据密钥类型
        if(rsatype == 0)
        {
            if(lp_cpyptography->m_key != NULL)
            {
                pk = lp_cpyptography->m_key;
                rsakeylen = lp_cpyptography->m_keylen;
            }
            else
            {
                //公钥
                //m_type: 0,使用固定密钥(对应密钥机公钥)
                if(lp_cpyptography->m_type == 0)
                {
                    memcpy(pk, (u8*)gArAcquirerTMKPublickeyInfo, 292); 
                    rsakeylen = 292;
                }
              #if 0  
                else if(lp_cpyptography->m_type == 1)
                {
                    //使用嘉联方式公钥
                    
                }
              #endif 
                else
                {
                    ret = DDI_ERR;
                    break;
                }
            }
            //memset(lp_cpyptography->m_out, 0, 
            if(encrypt == DECRYPT)
            {
                //sxl20170731   去flash的暂时只支持软件算法
                ret = pcicorecallfunc.RSA_PKDecrypt(lp_cpyptography->m_dataout, &rlen, lp_cpyptography->m_datain, lp_cpyptography->m_datainlen, pk, rsakeylen);
            }
            else
            {
                //sxl20170731   
                ret = pcicorecallfunc.RSA_PKEncrypt(lp_cpyptography->m_dataout, &rlen, lp_cpyptography->m_datain, lp_cpyptography->m_datainlen, pk, rsakeylen);
            }
            if(ret == 0)
            {
                lp_cpyptography->m_dataoutlen = rlen;
            }
            else
            {
                ret = DDI_ERR;
            }
        }
        else
        {           
            if(lp_cpyptography->m_key != NULL)
            {
                pk = lp_cpyptography->m_key;
                rsakeylen = lp_cpyptography->m_keylen;
            }
            else
            {
                //私钥
              #if 0
                //m_type: 0,使用固定密钥(对应密钥机公钥)
                if(lp_cpyptography->m_type == 0)
                {
                    memcpy(pk, (u8*)gArAcquirerTMKPublickeyInfo, 292); 
                    rsakeylen = 292;
                }  
                else 
              #endif 
                if(lp_cpyptography->m_type == 1)
                {
                    //使用嘉联方式私钥
                    mod = 0;        //嘉联方式私钥处理不需要解析
                    l_injectcert.m_data = sk;
                    l_injectcert.m_type = 2;
                    l_injectcert.m_offset = 0;
                    l_injectcert.m_len = RSA_KEY_SPACE_MAX;
                    ret = ddi_innerkey_getcert(1, &l_injectcert);
                    if(ret < 0)
                    {
                        
                        dev_debug_printf("%s(%d):ERR!\r\n", __FUNCTION__, __LINE__);
                        ret = DDI_ERR;
                        break;
                    }
                    else
                    {
                        rsakeylen = l_injectcert.m_len;
                    }
                }
                else
                {
                    ret = DDI_ERR;
                    break;
                }
            }
            //memset(lp_cpyptography->m_out, 0, 
            if(encrypt == DECRYPT)
            {
                //ret = dev_RSA_SKDecrypt(lp_cpyptography->m_dataout, &rlen, lp_cpyptography->m_datain, lp_cpyptography->m_datainlen, sk, rsakeylen, mod);
                ret = -1;  //sxl20170731   去flash的暂时不支持私钥算法
            }
            else
            {
                //ret = dev_RSA_SKEncrypt(lp_cpyptography->m_dataout, &rlen, lp_cpyptography->m_datain, lp_cpyptography->m_datainlen, sk, rsakeylen);
                ret = -1;  //sxl20170731   去flash的暂时不支持私钥算法
            }
            if(ret == 0)
            {
                lp_cpyptography->m_dataoutlen = rlen;
            }
            else
            {
                ret = DDI_ERR;
            }
        }
        break;
    }
    k_free(rsakey);
    rsakey = NULL;
    return ret;
  #else
    return DDI_ERR;
  #endif
}
static s32 innerkey_cryptography_deal(INNERKEY_CRYPTOGRAPHY_t* lp_cpyptography)
{
    s32 i;
    
  #if 1
    i = (lp_cpyptography->m_mod)&0x0f;
    if((i==0) || (i==1))
    {
        //对称算法
        return innerkey_cryptography_symmetric(lp_cpyptography);
    }
    else if((i==2) || (i==3))
    {
        //非对称算法
        return innerkey_cryptography_RSA(lp_cpyptography);
        
    }
    return DDI_EINVAL;
  #else
    return DDI_ERR;
  #endif
}
extern u16 processcmd_trendit_requestsecureinfo(u8 *recdata,u16 recdatalen,u8 *respdata,u16 *respdatalen);
static s32 innerkey_requestsecureinfo(u8 *respdata, u32 *resplen)
{
    u16 rlen;
    u16 ret;
  #if 0 
    ret = processcmd_trendit_requestsecureinfo("\x30", 1, respdata, &rlen);
    if(ret == 0x3030)
    {
        *resplen = rlen;
        return DDI_OK;
    }
    else
    {
        *resplen = 0;
        return DDI_ERR;
    }
  #else
    return DDI_ERR;
  #endif
}

extern u16 processcmd_trendit_setsecureinfo(u8 *recdata,u16 recdatalen,u8 *respdata,u16 *respdatalen);
static s32 innerkey_setsecureinfo(u8* recdata, u32 recdatalen)
{
  #if 0
    u16 ret;
    u8 tmp[20];
    u16 rlen;

    ret = processcmd_trendit_setsecureinfo(recdata, recdatalen, tmp, &rlen);
    if(ret == 0x3030)
    {
        if(recdata[0] == 0x30)
        {
            ddi_misc_set_mcuparam(MACHINE_PARAM_TMPRAND, (u8*)"\xFF\xFF\xFF\xFF", 4, 0);
            pcitamper_manage_unlock();
            processtask_infoprompt_init(1);//TASKPROMPTINFO_TYPE_UNLOCKPINSUC);
        }
    }
    else
    {
        return DDI_ERR;
    }
  #else
    return DDI_ERR;
  #endif
}
s32 innerkey_veritysignature(INNERKEY_VERITYSIGNATURE_t* lp_veritysignature)
{
    s32 ret;
    
    if(lp_veritysignature->m_filename != NULL)
    {
        //根据文件名检验
        ret = DDI_ERR;     //暂不处理
    }
    else
    {
        //根据绝对地址校验
        ret = ddi_downloadauthen_fileauthen(lp_veritysignature->m_addr, lp_veritysignature->m_len, lp_veritysignature->m_type);
    }
    return ret;
}

s32 ddi_innerkey_ioctl(u32 nCmd, u32 lParam, u32 wParam)
{
  #if 1
	s32 ret = DDI_EINVAL;
	macencrymod_t *lp_macencrymod;
	
	switch(nCmd)
	{
	case INNERKEY_IOCTL_INITIAL:
        //pcikeymanage_initkeydata();
		ret = DDI_OK;
		break;
	case INNERKEY_IOCTL_MACX919:
		lp_macencrymod = (macencrymod_t*)lParam;
		if(DDI_OK == ddi_inndrkey_mac_x919(lp_macencrymod->m_keytype, lp_macencrymod->m_index, lp_macencrymod->m_buf, lp_macencrymod->m_len, (u8*)wParam))
		{
			ret = DDI_OK; 
		}
		else
		{
			ret = DDI_ERR;	
		}
		break;
	case INNERKEY_IOCTL_MACX9_9:
		lp_macencrymod = (macencrymod_t*)lParam;
		if(DDI_OK == ddi_inndrkey_mac_x9_9(lp_macencrymod->m_keytype, lp_macencrymod->m_index, lp_macencrymod->m_buf, lp_macencrymod->m_len, (u8*)wParam))
		{
			ret = DDI_OK; 
		}
		else
		{
			ret = DDI_ERR;	
		}
		break;
    case DDI_INNERKEY_CTL_INJECTCERT:
        ret = ddi_innerkey_injectcert((str_injectcert_t *)lParam);
        break;
    case DDI_INNERKEY_CTL_GETCERT:
        ret = ddi_innerkey_getcert(0, (str_injectcert_t *)wParam);
        break;
    case DDI_INNERKEY_CTL_CRYPTOGRAPHY:
        ret = innerkey_cryptography_deal((INNERKEY_CRYPTOGRAPHY_t*)lParam);
        break;
    case DDI_INNERKEY_CTL_REQ_SECURE_DATA:
        ret = innerkey_requestsecureinfo((u8*)lParam, (u32*)wParam);
        break;
    case DDI_INNERKEY_CTL_SET_SECURE_DATA:
        ret = innerkey_setsecureinfo((u8*)lParam, wParam);
        break;
    case DDI_INNERKEY_CTL_VERITYSIGNATURE:
        ret = innerkey_veritysignature((INNERKEY_VERITYSIGNATURE_t*)lParam);
        break;
        
	default:
		break;
	}
	return ret;
  #else
    return DDI_ERR;
  #endif
}






s32 ddi_innerkey_getHW21SNinfo(u8 *randfact,u32 randfactlen,u8 *HW21SNinfo,u32 *HW21SNinfolen)
{
  #if 1
	s32 retCode = DDI_EINVAL;
    s32 ret;
	u8 snno[80];
	u8 snlen = 0;
	u8 encryptdata[80];
	u32 encryptdatalen = 0,modlen;
	u8 i;
	u8 asciidata[32];
	hwsn_info_t l_hwsn_info;


	if(randfact[0] > 10+8||randfact[0] < 8)
    {
    	return DDI_EINVAL;
    }
		
	
		
	//6位厂商编号
    memset(snno,0,sizeof(snno));
    memset(&l_hwsn_info,0,sizeof(hwsn_info_t));
    ret = ddi_manage_get_hwsninfo(&l_hwsn_info);
	if(ret != 0 || l_hwsn_info.m_snlen < 4)
	{
		return DDI_ERR;
	}
	
	
	
	//
	memcpy(snno,&randfact[1],8);
	snlen += 8;
	encryptdatalen = snlen;
	memcpy(encryptdata,snno,snlen);
	memcpy(&encryptdata[encryptdatalen],&randfact[1+8],randfact[0]-8);
	encryptdatalen += randfact[0]-8;
    //补齐16个字节
	modlen = encryptdatalen%16;
	if(modlen)
	{
		memset(&encryptdata[encryptdatalen],0,16-modlen);
		encryptdatalen += 16-modlen;
	}
	
	for(modlen = 16;modlen < encryptdatalen;modlen+= 16)
	{
		for(i = 0;i < 16;i++)
		{
			encryptdata[i] ^= encryptdata[modlen+i];
		}
	}
	memset(asciidata,0,sizeof(asciidata));
	dev_maths_bcd_to_asc(asciidata,encryptdata,16);
	//取前16个字节进行SM4算法
	#if 0
	ddi_core_printf("asc data:\r\n");  
	for(ret = 0;ret < 32;ret++)
	{
		ddi_core_printf("%02x ",asciidata[ret]);
	}
	ddi_core_printf("\r\n");
	#endif
	
	ret = ddi_arith_sm4(1,asciidata,16,asciidata,l_hwsn_info.m_snkey);
	if(ret == 0)
	{
	    #if 0
	    ddi_core_printf("sm4 1:\r\n");
		for(ret = 0;ret < 16;ret++)
		{
			ddi_core_printf("%02x ",asciidata[ret]);
		}
		ddi_core_printf("\r\n");
		#endif
		
		for(i = 0;i < 16;i++)
		{
			asciidata[i] ^= asciidata[16+i];
		}

		#if 0 
		ddi_core_printf("or 2:\r\n");
		for(ret = 0;ret < 16;ret++)
		{
			ddi_core_printf("%02x ",asciidata[ret]);
		}
		ddi_core_printf("\r\n");
		#endif
		
		//异或的结果再进行一次SM4算法
		ret = ddi_arith_sm4(1,asciidata,16,asciidata,l_hwsn_info.m_snkey);
		if(ret == 0)
		{
		    #if 0
			ddi_core_printf("sm4 2:\r\n");
			for(ret = 0;ret < 16;ret++)
			{
				ddi_core_printf("%02x ",asciidata[ret]);
			}
			ddi_core_printf("\r\n");
			#endif
			
			
			i = 0;
			HW21SNinfo[i++] = snlen;
			memcpy(&HW21SNinfo[i],snno,snlen);
			i += snlen;
			HW21SNinfo[i++] = 8;
			memcpy(&HW21SNinfo[i],asciidata,8);
			i += 8;
			*HW21SNinfolen = i;
			
			retCode = DDI_OK;
			
		}
		else
		{
			retCode = DDI_ERR;
		}
		
	}
	else
	{
		retCode = DDI_ERR;
	}
	
	
	return retCode;
  #else
    return DDI_ERR;
  #endif
	
}


