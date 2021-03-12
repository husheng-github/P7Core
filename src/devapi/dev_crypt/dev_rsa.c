


#include "devglobal.h"
#include "sdk/mhscpu_sdk.h"
#include "mh_rsa.h"


uint32_t dev_rsa_rand_p(void *rand, uint32_t bytes, void *p_rng)
{
//    memset(rand, 0x01, bytes);
//    return bytes;
    return dev_trng_read(rand, bytes);
    
}


//用公钥进行RSA加密
//pk:公钥
int dev_RSA_PKEncrypt(unsigned char *output, unsigned int *outputLen,unsigned char *input,  unsigned int inputLen,unsigned char *PK, unsigned int pkLen)
{
	u32 ret,modulusLen;
	mh_rsa_public_key pub_key;	//RSA公钥
	u8 pkcsBlock[256];
	u8 byte;

	u32 i;  //sxl?2017 测试用，要删除

    //现在先只支持2048位RSA算法
    //pk长度判断
    if(pkLen > 128&&pkLen < 256)
    {
    	pkLen = 128;
    }
	else if(pkLen > 256)
	{
		pkLen = 256;
	}
	else
	{
		return 1;
	}
	
	modulusLen = pkLen;
    if (inputLen + 11 > modulusLen)
        return (1);
	
	//加密数据赋值
    pkcsBlock[0] = 0;
    /* block type 2 */
    pkcsBlock[1] = 2;

  #if 1
	//取非0的随机数
	for (i = 2; i < modulusLen - inputLen - 1; i++) {
        /* Find nonzero random byte.
         */
        do {
            dev_trng_read ((void *)&byte, 1);	
        }
        while (byte == 0);
        pkcsBlock[i] = byte;
    }
  #else
    dev_trng_read(&pkcsBlock[2], modulusLen - inputLen - 3);
  #endif
  
    /* separator */
    pkcsBlock[i++] = 0;	
    
	memset(&pub_key, 0, sizeof(pub_key));
	//输入数据
    memcpy (&pkcsBlock[i],input, inputLen);
	
	//公钥赋值
	pub_key.bytes = pkLen;
    memcpy(pub_key.e,"\x00\x01\x00\x01",4);		//固定公钥指数
	memcpy(pub_key.n,&PK[4],pkLen);		//模数n	
	
    //模数n参数C、Q
	ret = mh_rsa_get_public_key_C_Q(&pub_key,dev_rsa_rand_p,NULL);
    if(ret != MH_RET_RSA_CKEY_SUCCESS)
    {
    	return 1;
    }

	//用公钥进行RSA加密
	ret = mh_rsa_public(pkcsBlock,pkcsBlock,&pub_key,dev_rsa_rand_p,NULL);
	if(ret != MH_RET_RSA_SUCCESS)
    {
        dev_debug_printf("%s(%d):ret = %08X\r\n", __FUNCTION__, __LINE__,ret);
    	return 1;
    }

    *outputLen = pkLen;
	//输出output
    memcpy(output,pkcsBlock, *outputLen);
	
	return 0;	
}


int dev_RSA_PKDecrypt(unsigned char * output, unsigned int *outputLen,unsigned char * input,  unsigned int inputLen,unsigned char * PK, unsigned int pkLen)
{
    
	u32 ret,i;
	mh_rsa_public_key pub_key;
	u8 pkcsBlock[256];
    
	memset(&pub_key, 0, sizeof(pub_key));

	//现在先只支持2048 位RSA算法
	if(pkLen > 128&&pkLen < 256)
    {
    	pkLen = 128;
    }
	else if(pkLen > 256)
	{
		pkLen = 256;
	}
	else
	{
		return 1;
	}
	pub_key.bytes = pkLen;
    memcpy(pub_key.e,"\x00\x01\x00\x01",4);//固定公钥指数
	memcpy(pub_key.n,&PK[4],pkLen);

	
	
	ret = mh_rsa_get_public_key_C_Q(&pub_key,dev_rsa_rand_p,NULL);
	#ifdef SXL_DEBUG
    dev_debug_printf("\r\nmh_rsa_get_public_key_C_Q = %08x\r\n",ret);
	#endif
    if(ret != MH_RET_RSA_CKEY_SUCCESS)
    {
    	return 1;
    }

	memset(pkcsBlock,0,sizeof(pkcsBlock));
    ret = mh_rsa_public(pkcsBlock,input,&pub_key,dev_rsa_rand_p,NULL);

	if(ret != MH_RET_RSA_SUCCESS)
    {
        #ifdef SXL_DEBUG
        dev_debug_printf("%s(%d):ret = %08X\r\n", __FUNCTION__, __LINE__,ret);
		#endif
    	return 1;
    }
	#ifdef SXL_DEBUG
    dev_debug_printf("\r\npublic rsa result:%08X\r\n",ret);
	for(ret = 0;ret < 256;ret++)
	{
		dev_debug_printf("%02x ",pkcsBlock[ret]);
	}
	dev_debug_printf("\r\n");
	#endif

	 if ((pkcsBlock[0] != 0) || (pkcsBlock[1] != 1))
        return (1);

    for (i = 2; i < pkLen - 1; i++)
        if (pkcsBlock[i] != 0xff)
            break;

    /* separator */
    if (pkcsBlock[i++] != 0)
        return (1);
    
    *outputLen = pkLen - i;
    
    if (*outputLen + 11 > pkLen)
        return (1);
    
    memcpy (output,&pkcsBlock[i], *outputLen);
	return 0;
	
}



int dev_RSA_SKEncrypt(unsigned char *output, unsigned int *outputLen,unsigned char *input,  unsigned int inputLen,unsigned char *SK, unsigned int skLen)
{
	u32 ret,modulusLen,offset;
	mh_rsa_private_key pri_key;
	u8 pkcsBlock[256];
	u8 byte;

	u32 i;  //sxl?2017 测试用，要删除

    //现在先只支持2048 位RSA算法
    if(skLen > 256)
	{
		skLen = 256;
	}
	else
	{
		return 1;
	}
	
	modulusLen = skLen;
    if (inputLen + 11 > modulusLen)
        return (1);

    pkcsBlock[0] = 0;
    /* block type 2 */
    pkcsBlock[1] = 1;

    for (i = 2; i < modulusLen - inputLen - 1; i++) {
        /* Find nonzero random byte.
         */
        //do {
        //    dev_trng_read ((void *)&byte, 1);
        //}
        //while (byte == 0);
        pkcsBlock[i] = 0xff;
    }
    /* separator */
    pkcsBlock[i++] = 0;
	
    
	memset(&pri_key, 0, sizeof(pri_key));
    memcpy (&pkcsBlock[i],input, inputLen);
	
	 pri_key.bytes = 2048>>3;
	memcpy(pri_key.e,"\x00\x01\x00\x01",4);

	offset = 4;
	memcpy(pri_key.n,&SK[offset],pri_key.bytes);
	//ReadBigIntToChar(pri_key.n,pri_key.bytes,&m2teskpk[offset]);
	offset += pri_key.bytes;
	memcpy(pri_key.d,&SK[offset],pri_key.bytes);
	//ReadBigIntToChar(pri_key.d,pri_key.bytes,&m2teskpk[offset]);
	offset += pri_key.bytes;
    memcpy(pri_key.p,&SK[offset],pri_key.bytes>>1);
    //ReadBigIntToChar(pri_key.p,pri_key.bytes>>1,&m2teskpk[offset]);
	offset += (pri_key.bytes>>1);
	memcpy(pri_key.q,&SK[offset],pri_key.bytes>>1);
	//ReadBigIntToChar(pri_key.q,pri_key.bytes>>1,&m2teskpk[offset]);
	offset += (pri_key.bytes>>1);
	//ReadBigIntToChar(pri_key.dp,pri_key.bytes>>1,&m2teskpk[offset]);
	memcpy(pri_key.dp,&SK[offset],pri_key.bytes>>1);
	offset += (pri_key.bytes>>1);
	memcpy(pri_key.dq,&SK[offset],pri_key.bytes>>1);
	//ReadBigIntToChar(pri_key.dq,pri_key.bytes>>1,&m2teskpk[offset]);
	offset += (pri_key.bytes>>1);

	memcpy(pri_key.qp,&SK[offset],pri_key.bytes>>1);
	//ReadBigIntToChar(pri_key.qp,pri_key.bytes>>1,&m2teskpk[offset]);
	offset += (pri_key.bytes>>1);
	
	
	ret = mh_rsa_get_private_key_C_Q(&pri_key,dev_rsa_rand_p,NULL);
    if(ret != MH_RET_RSA_CKEY_SUCCESS)
    {
    	return 1;
    }

    
	ret = mh_rsa_private_crt(pkcsBlock,pkcsBlock,&pri_key,dev_rsa_rand_p,NULL);
	if(ret != MH_RET_RSA_SUCCESS)
    {
        dev_debug_printf("%s(%d):ret = %08X\r\n", __FUNCTION__, __LINE__,ret);
    	return 1;
    }

    *outputLen = pri_key.bytes;
    memcpy (output,pkcsBlock, *outputLen);
	
	return 0;
	
}


int dev_RSA_SKDecrypt(unsigned char * output, unsigned int *outputLen,unsigned char * input,  unsigned int inputLen,unsigned char * SK, unsigned int skLen)
{
    
	u32 ret,i,offset;
	mh_rsa_private_key pri_key;
	u8 pkcsBlock[256];
    
	memset(&pri_key, 0, sizeof(pri_key));

	//现在先只支持2048 位RSA算法
	if(skLen > 256)
	{
		skLen = 256;
	}
	else
	{
		return 1;
	}

	//sxl?2017
	//dev_debug_printf("\r\npri_key:\r\n");
	//for(offset = 0;offset < 1156;offset++)
	//{
	//	dev_debug_printf("%02x ",SK[offset]);
	//}
	//dev_debug_printf("\r\n");

	
	pri_key.bytes = skLen;
	memcpy(pri_key.e,"\x00\x01\x00\x01",4);

	offset = 4;
	memcpy(pri_key.n,&SK[offset],pri_key.bytes);
	//ReadBigIntToChar(pri_key.n,pri_key.bytes,&m2teskpk[offset]);
	offset += pri_key.bytes;
	memcpy(pri_key.d,&SK[offset],pri_key.bytes);
	//ReadBigIntToChar(pri_key.d,pri_key.bytes,&m2teskpk[offset]);
	offset += pri_key.bytes;
    memcpy(pri_key.p,&SK[offset],pri_key.bytes>>1);
    //ReadBigIntToChar(pri_key.p,pri_key.bytes>>1,&m2teskpk[offset]);
	offset += (pri_key.bytes>>1);
	memcpy(pri_key.q,&SK[offset],pri_key.bytes>>1);
	//ReadBigIntToChar(pri_key.q,pri_key.bytes>>1,&m2teskpk[offset]);
	offset += (pri_key.bytes>>1);
	//ReadBigIntToChar(pri_key.dp,pri_key.bytes>>1,&m2teskpk[offset]);
	memcpy(pri_key.dp,&SK[offset],pri_key.bytes>>1);
	offset += (pri_key.bytes>>1);
	memcpy(pri_key.dq,&SK[offset],pri_key.bytes>>1);
	//ReadBigIntToChar(pri_key.dq,pri_key.bytes>>1,&m2teskpk[offset]);
	offset += (pri_key.bytes>>1);

	memcpy(pri_key.qp,&SK[offset],pri_key.bytes>>1);
	//ReadBigIntToChar(pri_key.qp,pri_key.bytes>>1,&m2teskpk[offset]);
	offset += (pri_key.bytes>>1);
	
	
	ret = mh_rsa_get_private_key_C_Q(&pri_key,dev_rsa_rand_p,NULL);
	//sxl?2017
	//dev_debug_printf("\r\nret = %d\r\n",ret);
    if(ret != MH_RET_RSA_CKEY_SUCCESS)
    {
    	return 1;
    }

	memset(pkcsBlock,0,sizeof(pkcsBlock));
	ret = mh_rsa_private_crt(pkcsBlock,input,&pri_key,dev_rsa_rand_p,NULL);
    if(ret != MH_RET_RSA_SUCCESS)
    {
        dev_debug_printf("%s(%d):ret = %08X\r\n", __FUNCTION__, __LINE__,ret);
    	return 1;
    }
	/*
	dev_debug_printf("\r\noutput:\r\n");
	for(offset = 0;offset < 256;offset++)
	{
		dev_debug_printf("%02x ",pkcsBlock[offset]);
	}
	dev_debug_printf("\r\n");
	*/

	if ((pkcsBlock[0] != 0) || (pkcsBlock[1] != 2))
	{
        return (1);
	}

    for (i = 2; i < pri_key.bytes - 1; i++)
        /* separator */
        if (pkcsBlock[i] == 0)
            break;

    i++;
    if (i >=  pri_key.bytes)
    {
        return (2);
    }

    *outputLen =  pri_key.bytes - i;

    if (*outputLen + 11 >  pri_key.bytes)
    {
        return (3);
    }
	
    memcpy( output, &pkcsBlock[i], *outputLen );
	return 0;
	
}









s32 dev_rsarecover(u8 *m, u32 mLen, u8 *e, u32 eLen,u8 *input,u8 *output)
{
	u8 rsa_e[4];
	int ret = 0;
    mh_rsa_public_key pub_key;
	
	
	if(mLen > 256)
	{
		return 1;
	}

    memset(rsa_e,0,sizeof(rsa_e));
	
	if(eLen == 0x01)
	{
		rsa_e[3] = e[0];
		eLen = 1;
	}
	else if(eLen == 0x03)
	{
	    if(memcmp(e,"\x00\x00\x03",3) == 0)
		{
		    rsa_e[3] = 0x03;
			eLen = 1;
	    }
		else
		{
			memcpy(&rsa_e[1],e,eLen);
		}
	}
	else
	{
	    dev_debug_printf("%s(%d)\r\n", __FUNCTION__, __LINE__);
		return 1;
	}

	memset(&pub_key, 0, sizeof(pub_key));
	
	pub_key.bytes = mLen;
    memcpy(pub_key.e,rsa_e,4);//固定公钥指数
	memcpy(pub_key.n,m,mLen);
    
	ret = mh_rsa_get_public_key_C_Q(&pub_key,dev_rsa_rand_p,NULL);
	
    if(ret != MH_RET_RSA_CKEY_SUCCESS)
    {
        dev_debug_printf("%s(%d):ret = %08X\r\n", __FUNCTION__, __LINE__,ret);
    	return 1;
    }

	
	ret = mh_rsa_public(output,input,&pub_key,dev_rsa_rand_p,NULL);
	if(ret != MH_RET_RSA_SUCCESS)
    {
        dev_debug_printf("%s(%d):ret = %08X\r\n", __FUNCTION__, __LINE__,ret);
    	return 1;
    }
	return 0;
	
}

