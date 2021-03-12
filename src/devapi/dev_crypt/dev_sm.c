

#include "devglobal.h"
#include "sdk/mhscpu_sdk.h"
#include "mh_crypt.h"
//#include "mh_bignum_tool.h"
#include "mh_sm2.h"
#include "mh_sm4.h"
#include "mh_des.h"

#ifdef TRENDIT_CORE

#define GroupP "FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFF"
#define GroupA "FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFC"
#define GroupB "28E9FA9E9D9F5E344D5A9E4BCF6509A7F39789F515AB8F92DDBCBD414D940E93"
#define GroupX "32C4AE2C1F1981195F9904466A39C9948FE30BBFF2660BE1715A4589334C74C7"
#define GroupY "BC3736A2F4F6779C59BDCEE36B692153D0A9877CC62A474002DF32E52139F0A0"
#define GroupN "FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFF7203DF6B21C6052B53BBF40939D54123"

#define cIDa    "31323334353637383132333435363738"

//sm2 add
#define ckeyd   "C46841E769D7F6A7599046C029900CB6D47BBD54BE83AF50284584BC1E390BF9"
#define cex     "1CDFD6D0375D84164EA5CBCD929874F19384743E20407EF1CCFDC07D960643F7"
#define cey     "F0053CCD7FE91230A12003A084964928DE12DC97E6F51F2B46C43935DB9F7857"


#if 1
void CRYPT0_IRQHandler(void)
{
//	printf("crypt handler in\n");
	mh_crypt_it_clear();
	NVIC_ClearPendingIRQ(CRYPT0_IRQn);
}

int dev_sm3_getsummary(unsigned char *pInput, unsigned int ilen, unsigned char *pOutput)
{
	u8 output[32];
	mh_sm3(output,pInput,ilen);

	memcpy(pOutput,output,32);
	
	return 32;
	
}



static int bn_get_digit( uint32_t *d, char c )
{
    *d = 255;

    if( c >= 0x30 && c <= 0x39 ) *d = c - 0x30;
    if( c >= 0x41 && c <= 0x46 ) *d = c - 0x37;
    if( c >= 0x61 && c <= 0x66 ) *d = c - 0x57;

    if( *d >= (uint32_t) 16 )
        return( -1 );

    return( 0 );
}



int bn_read_string_to_bytes( uint8_t *r, uint32_t len, const char *s )
{
    uint32_t i, j, slen;
    uint32_t d;
	
    slen = strlen( s );

	if ((len * 2) < slen)
		return(-1);

	memset(r, 0, len);
	
	len = len - 1;
	
	for( i = slen, j = 0; i > 0; i--, j++)
	{
		if( -1 == bn_get_digit( &d, s[i - 1] ) )
			return (-1);
		r[len - j / 2] |= d << ( (j % 2) << 2 );
	}

    return( 0 );
}

#endif


int dev_sm2_verity(const char *pPkeyX, const char *pPkeyY, const char *pSignR, const char *pSignS,u8 *message,u32 messagelen)
{	
	int ret;
	u8 signdata1[64];// = {0xab,0xcb,0x7c,0xff,0x24,0xc7,0x8e,0x7b,0xe2,0x64,0x67,0x3d,0xe0,0x6d,0xc1,0x21,0x9e,0x47,0xb6,0x55,0x1d,0xc9,0xc4,0xdd,0x5f,0xda,0x04,0x80,0x75,0x95,0x47,0x4c,0x14,0x5c,0x15,0xa7,0x7d,0x6a,0x5b,0x69,0x2f,0xf0,0x7b,0x34,0xcc,0x8c,0xa1,0x81,0xdd,0x23,0x08,0x44,0xf8,0xcc,0x22,0xd2,0x66,0x1c,0x60,0x5e,0x59,0x51,0x9d,0x53};
	u8 pubkey[65];
    unsigned char sm2_digest[32];// = "\xB5\x24\xF5\x52\xCD\x82\xB8\xB0\x28\x47\x6E\x00\x5C\x37\x7F\xB1\x9A\x87\xE6\xFC\x68\x2D\x48\xBB\x5D\x42\xE3\xD9\xB9\xEF\xFE\x76"; 
    u32 resp;
	u8 IDa[16];
	u8 Za[32];
	u8 _Za[32];
	u32 _e[8];
	
	
    mh_sm2_sign mhsm3sign;
	mh_sm2_public_key mhsm2publickey;
	mh_sm2_ellipse_para para;
	
	memcpy(mhsm3sign.r,pSignR,MH_SM2_KEY_BYTES);
    memcpy(mhsm3sign.s,pSignS,MH_SM2_KEY_BYTES);
    
	memcpy(mhsm2publickey.x,pPkeyX,MH_SM2_KEY_BYTES);
	memcpy(mhsm2publickey.y,pPkeyY,MH_SM2_KEY_BYTES);
	
	
	bn_read_string_to_bytes(para.p, MH_SM2_KEY_BYTES, GroupP);
	bn_read_string_to_bytes(para.a, MH_SM2_KEY_BYTES, GroupA);
	bn_read_string_to_bytes(para.b, MH_SM2_KEY_BYTES, GroupB);
	bn_read_string_to_bytes(para.n, MH_SM2_KEY_BYTES, GroupN);
	bn_read_string_to_bytes(para.g.x, MH_SM2_KEY_BYTES, GroupX);
	bn_read_string_to_bytes(para.g.y, MH_SM2_KEY_BYTES, GroupY);


	bn_read_string_to_bytes(IDa, sizeof(IDa), cIDa);

	mh_sm2_hash_z(_Za, IDa, 0x0080, &para, &mhsm2publickey);
	mh_sm2_hash_e(_e, _Za, message, messagelen);
	
	resp = mh_sm2_verify_sign_with_e(&mhsm3sign, _e, message,messagelen, &mhsm2publickey, &para, mh_rand_p, NULL);
	if(resp == MH_RET_SM2_VERIFY_SUCCESS)
	{
		ret = 0;
	}
	else
	{
		ret = 1;
	}
	
	return ret;
	
}

u32 dev_sm_get_rand(void *rand, u32 bytes, void *p_rng)
{
  #if 0  
    u8 *ap;
    ap = (u8*)rand;
    memset(ap, 0x31, bytes);
    return bytes;
  #endif
  
    return dev_trng_read((u8*)rand, bytes);
}

s32 dev_sm4_deal(u8 encrypt, u8 mode, u8 *data, u32 datalen,  u8 *keyt, u32 keylen, u8 *iv, u8 *out)
{
    s32 i;
    mh_pack_mode_def type;
    s32 ret;
    
    //输入数据长度必须为16的倍数且不为0,keylen必须为16
    if((keylen != 16) || ((datalen&0x0F)!=0) || datalen==0)
    {
        return -1;
    }
    if(mode == 0)
    {
        type = ECB;
    }
    else if(mode == 1)
    {
        type = CBC;
    }
    else
    {
        return -1;
    }
    if(encrypt == DECRYPT)
    {
        //解密
        ret = mh_sm4_dec(type, out, datalen,  data, datalen, keyt, iv, dev_sm_get_rand, NULL);
    }
    else if(encrypt == ENCRYPT)
    {
        //加密
        ret = mh_sm4_enc(type, out, datalen,  data, datalen, keyt, iv, dev_sm_get_rand, NULL);
    }
    else
    {
        return -1;
    }
    if(MH_RET_SM4_SUCCESS == ret)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

#if 0
void sm2_test(void)
{
	u8 signdata1[71] = {0x30,0x45,0x02,0x21,0x00,0xab,0xcb,0x7c,0xff,0x24,0xc7,0x8e,0x7b,0xe2,0x64,0x67,0x3d,0xe0,0x6d,0xc1,0x21,0x9e,0x47,0xb6,0x55,0x1d,0xc9,0xc4,0xdd,0x5f,0xda,0x04,0x80,0x75,0x95,0x47,0x4c,0x02,0x20,0x14,0x5c,0x15,0xa7,0x7d,0x6a,0x5b,0x69,0x2f,0xf0,0x7b,0x34,0xcc,0x8c,0xa1,0x81,0xdd,0x23,0x08,0x44,0xf8,0xcc,0x22,0xd2,0x66,0x1c,0x60,0x5e,0x59,0x51,0x9d,0x53};
    u8 pointx[32] = {0xd5,0x54,0x8c,0x78,0x25,0xcb,0xb5,0x61,0x50,0xa3,0x50,0x6c,0xd5,0x74,0x64,0xaf,0x8a,0x1a,0xe0,0x51,0x9d,0xfa,0xf3,0xc5,0x82,0x21,0xdc,0x81,0x0c,0xaf,0x28,0xdd};
	u8 pointy[32] = {0x92,0x10,0x73,0x76,0x8f,0xe3,0xd5,0x9c,0xe5,0x4e,0x79,0xa4,0x94,0x45,0xcf,0x73,0xfe,0xd2,0x30,0x86,0x53,0x70,0x27,0x26,0x4d,0x16,0x89,0x46,0xd4,0x79,0x53,0x3e};

	//devsm2_verity();	
}

void test_sm4(void)
{
    u8 tmp[32];
//    u8 tmp1[32];
//    u8 tmp2[32];
    u8 key[16];
    s32 ret;
    u32 ver;
//    u8 Iv[16] = {0xA3, 0xB1, 0xBA, 0xC6, 0x56, 0xAA, 0x33, 0x50, 0x67, 0x7D, 0x91, 0x97, 0xB2, 0x70, 0x22, 0xDC};
	

    memcpy(key, "2222222222222222", 16);
//    memcpy(tmp, "5555555555555555", 16);
    memset(tmp, 0x35, 16);
    memset(&tmp[16], 0x36, 16);
    ver = mh_get_version();
dev_debug_printf("%s(%d):ver=%08X\r\n", __FUNCTION__, __LINE__, ver); 
    
//	mh_crypt_it_clear();          // 加解密完成都会产生中断
    ret = dev_sm4_deal(ENCRYPT, 0, tmp, 32,  key, 16, NULL, tmp);
    if(ret<0)
    {
dev_debug_printf("%s(%d):ret=%d\r\n", __FUNCTION__, __LINE__, ret);        
    }
    else
    {
dev_debug_printf("%s(%d):ret=%d\r\n", __FUNCTION__, __LINE__, ret);        
dev_debug_printformat(NULL, tmp, 32);
    }
    ret = dev_sm4_deal(DECRYPT, 0, tmp, 32,  key, 16, NULL, tmp);
    if(ret<0)
    {
dev_debug_printf("%s(%d):ret=%d\r\n", __FUNCTION__, __LINE__, ret);        
    }
    else
    {
dev_debug_printf("%s(%d):ret=%d\r\n", __FUNCTION__, __LINE__, ret);        
dev_debug_printformat(NULL, tmp, 32);
    }
    ret = dev_sm4_deal(ENCRYPT, 1, tmp, 32,  key, 16, NULL, tmp);
    if(ret<0)
    {
dev_debug_printf("%s(%d):ret=%d\r\n", __FUNCTION__, __LINE__, ret);        
    }
    else
    {
dev_debug_printf("%s(%d):ret=%d\r\n", __FUNCTION__, __LINE__, ret);        
dev_debug_printformat(NULL, tmp, 32);
    }
    ret = dev_sm4_deal(DECRYPT, 1, tmp, 32,  key, 16, NULL, tmp);
    if(ret<0)
    {
dev_debug_printf("%s(%d):ret=%d\r\n", __FUNCTION__, __LINE__, ret);        
    }
    else
    {
dev_debug_printf("%s(%d):ret=%d\r\n", __FUNCTION__, __LINE__, ret);        
dev_debug_printformat(NULL, tmp, 32);
    }

}

#endif


/****************************************************************************
**Description:	    基于国密算法sm2的PK加密
**Input parameters:	
**Output parameters: 
**Returned value:
                    0:成功
**Created by:		husheng 20190625
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
int dev_sm2_PKEncrypt(unsigned char *output, unsigned int *outputLen,unsigned char *input,  unsigned int inputLen,unsigned char *PK, unsigned int pkLen)
{
	uint8_t  *tmp;
	uint32_t lenth;
	mh_sm2_public_key pub_key;
	mh_sm2_ellipse_para ell_para;

	tmp = (uint8_t*)k_malloc(2048);
	if(tmp == NULL)
	{	  
		return -1;
	}

	//填充pub_key	
	memcpy(pub_key.x,&PK[0],MH_SM2_KEY_BYTES);
	memcpy(pub_key.y,&PK[MH_SM2_KEY_BYTES],MH_SM2_KEY_BYTES);
	
	//外部传入mh_sm2_ellipse_para
	bn_read_string_to_bytes(ell_para.p, MH_SM2_KEY_BYTES, GroupP);
    bn_read_string_to_bytes(ell_para.a, MH_SM2_KEY_BYTES, GroupA);
    bn_read_string_to_bytes(ell_para.b, MH_SM2_KEY_BYTES, GroupB);
    bn_read_string_to_bytes(ell_para.n, MH_SM2_KEY_BYTES, GroupN);
    bn_read_string_to_bytes(ell_para.g.x, MH_SM2_KEY_BYTES, GroupX);
    bn_read_string_to_bytes(ell_para.g.y, MH_SM2_KEY_BYTES, GroupY);	

	//调用兆讯的加密接口
	if(MH_RET_SM2_ENC_SUCCESS != mh_sm2_enc(tmp, &lenth, input, inputLen, &pub_key, &ell_para, dev_trng_read, NULL))
	{ 
		k_free(tmp);
		return 1;
	}
	else
	{
		memcpy(output,&tmp[1],64);
		memcpy(output+64,&tmp[lenth-32],32);
		memcpy(output+64+32,&tmp[65],lenth-1-32-64);
		*outputLen = lenth -1;
	}	
		  
	k_free(tmp);
	return 0;	
}

/****************************************************************************
**Description:	    基于国密算法sm2的PK解密
**Input parameters:	
**Output parameters: 
**Returned value:
                    0:成功
**Created by:		husheng 20190625
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
int dev_sm2_PKDecrypt(unsigned char * output, unsigned int *outputLen,unsigned char * input,  unsigned int inputLen,unsigned char * PK, unsigned int pkLen)
{
	uint8_t  *tmp;
	uint32_t lenth;
	uint32_t ret;
	mh_sm2_private_key pri_key;
	mh_sm2_ellipse_para ell_para;	

	tmp = (uint8_t*)k_malloc(2048);
	if(tmp == NULL)
	{	  	
		return -1;
	}

	//填充pri_key	
	memcpy(pri_key.d,&PK[0],MH_SM2_KEY_BYTES);
	memcpy(pri_key.e.x,&PK[MH_SM2_KEY_BYTES],MH_SM2_KEY_BYTES);
	memcpy(pri_key.e.y,&PK[MH_SM2_KEY_BYTES*2],MH_SM2_KEY_BYTES);

	//外部传入mh_sm2_ellipse_para
	bn_read_string_to_bytes(ell_para.p, MH_SM2_KEY_BYTES, GroupP);
    bn_read_string_to_bytes(ell_para.a, MH_SM2_KEY_BYTES, GroupA);
    bn_read_string_to_bytes(ell_para.b, MH_SM2_KEY_BYTES, GroupB);
    bn_read_string_to_bytes(ell_para.n, MH_SM2_KEY_BYTES, GroupN);
    bn_read_string_to_bytes(ell_para.g.x, MH_SM2_KEY_BYTES, GroupX);
    bn_read_string_to_bytes(ell_para.g.y, MH_SM2_KEY_BYTES, GroupY);

	tmp[0] = 0x04;
	memcpy(&tmp[1],input,64);
	memcpy(&tmp[65],input+64+32,inputLen-64-32);
	memcpy(&tmp[1+inputLen-32],input+64,32);
	lenth = inputLen + 1;
	
	//调用兆讯的解密接口
	if(MH_RET_SM2_ENC_SUCCESS != mh_sm2_dec(output, outputLen, tmp, lenth, &pri_key, &ell_para, dev_trng_read, NULL))
	{ 
		k_free(tmp);
		return 1;
	}
	else
	{
		k_free(tmp);
		return 0;
	}	
}

/****************************************************************************
**Description:	    基于国密算法sm2的SK加密
**Input parameters:	
**Output parameters: 
**Returned value:
                    0:成功
**Created by:		husheng 20190625
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
int dev_sm2_SKEncrypt(unsigned char *output, unsigned int *outputLen,unsigned char *input,  unsigned int inputLen,unsigned char *SK, unsigned int skLen)
{
	uint8_t  *tmp;
	uint32_t lenth;
	mh_sm2_public_key pub_key;
	mh_sm2_ellipse_para ell_para;

	tmp = (uint8_t*)k_malloc(2048);
	if(tmp == NULL)
	{	  
		return -1;
	}

	//填充pub_key结构体	
	memcpy(pub_key.x,&SK[0],MH_SM2_KEY_BYTES);
	memcpy(pub_key.y,&SK[MH_SM2_KEY_BYTES],MH_SM2_KEY_BYTES);

	//外部传入mh_sm2_ellipse_para
	bn_read_string_to_bytes(ell_para.p, MH_SM2_KEY_BYTES, GroupP);
    bn_read_string_to_bytes(ell_para.a, MH_SM2_KEY_BYTES, GroupA);
    bn_read_string_to_bytes(ell_para.b, MH_SM2_KEY_BYTES, GroupB);
    bn_read_string_to_bytes(ell_para.n, MH_SM2_KEY_BYTES, GroupN);
    bn_read_string_to_bytes(ell_para.g.x, MH_SM2_KEY_BYTES, GroupX);
    bn_read_string_to_bytes(ell_para.g.y, MH_SM2_KEY_BYTES, GroupY);
	
	//调用兆讯的加密接口
	if(MH_RET_SM2_ENC_SUCCESS != mh_sm2_enc(tmp, &lenth, input, inputLen, &pub_key, &ell_para, dev_trng_read, NULL))
	{ 
		k_free(tmp);
		return 1;
	}
	else
	{
		memcpy(output,&tmp[1],64);
		memcpy(output+64,&tmp[lenth-32],32);
		memcpy(output+64+32,&tmp[65],lenth-1-32-64);
		*outputLen = lenth -1;
	}	
		  
	k_free(tmp);
	return 0;	
}

/****************************************************************************
**Description:	    基于国密算法sm2的SK解密
**Input parameters:	
**Output parameters: 
**Returned value:
                    0:成功
**Created by:		husheng 20190625
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
int dev_sm2_SKDecrypt(unsigned char * output, unsigned int *outputLen,unsigned char * input,  unsigned int inputLen,unsigned char * SK, unsigned int skLen)
{
	uint8_t  *tmp;
	uint32_t lenth;
	uint32_t ret;
	mh_sm2_private_key pri_key;
	mh_sm2_ellipse_para ell_para;	

	tmp = (uint8_t*)k_malloc(2048);
	if(tmp == NULL)
	{	  	
		return -1;
	}

	//填充pri_key结构体		
	memcpy(pri_key.d,&SK[0],MH_SM2_KEY_BYTES);
	memcpy(pri_key.e.x,&SK[MH_SM2_KEY_BYTES],MH_SM2_KEY_BYTES);
	memcpy(pri_key.e.y,&SK[MH_SM2_KEY_BYTES*2],MH_SM2_KEY_BYTES);

	//外部传入mh_sm2_ellipse_para
	bn_read_string_to_bytes(ell_para.p, MH_SM2_KEY_BYTES, GroupP);
    bn_read_string_to_bytes(ell_para.a, MH_SM2_KEY_BYTES, GroupA);
    bn_read_string_to_bytes(ell_para.b, MH_SM2_KEY_BYTES, GroupB);
    bn_read_string_to_bytes(ell_para.n, MH_SM2_KEY_BYTES, GroupN);
    bn_read_string_to_bytes(ell_para.g.x, MH_SM2_KEY_BYTES, GroupX);
    bn_read_string_to_bytes(ell_para.g.y, MH_SM2_KEY_BYTES, GroupY);

	tmp[0] = 0x04;
	memcpy(&tmp[1],input,64);
	memcpy(&tmp[65],input+64+32,inputLen-64-32);
	memcpy(&tmp[1+inputLen-32],input+64,32);
	lenth = inputLen + 1;

	//调用兆讯的解密接口
	if(MH_RET_SM2_ENC_SUCCESS != mh_sm2_dec(output, outputLen, tmp, lenth, &pri_key, &ell_para, dev_trng_read, NULL))
	{ 
		k_free(tmp);
		return 1;
	}
	else
	{
		k_free(tmp);
		return 0;
	}	
}

/****************************************************************************
**Description:	    基于国密算法sm2的数字签名
**Input parameters:	
**Output parameters: 
**Returned value:
                    0:成功
**Created by:		husheng 20190625
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
int dev_sm2_digital_sign(const char *pPkeyD, const char *pPkeyX, const char *pPkeyY, const char *pSignR, const char *pSignS,u8 *message,u32 messagelen)
{
	mh_sm2_sign mhsm2sign;
	uint32_t _e[8];
	uint8_t _Za[32];
	u8 IDa[16];	
	u32 resp;
	int ret;
	
	mh_sm2_private_key pri_key;
	mh_sm2_ellipse_para para;
	
	//填充mh_sm2_sign
	memcpy(mhsm2sign.r,pSignR,MH_SM2_KEY_BYTES);
    memcpy(mhsm2sign.s,pSignS,MH_SM2_KEY_BYTES);
	
	//填充mh_sm2_private_key
	memcpy(pri_key.d,pPkeyD,MH_SM2_KEY_BYTES);
	memcpy(pri_key.e.x,pPkeyX,MH_SM2_KEY_BYTES);
	memcpy(pri_key.e.y,pPkeyY,MH_SM2_KEY_BYTES);

	//外部传入mh_sm2_ellipse_para
	bn_read_string_to_bytes(para.p, MH_SM2_KEY_BYTES, GroupP);
    bn_read_string_to_bytes(para.a, MH_SM2_KEY_BYTES, GroupA);
    bn_read_string_to_bytes(para.b, MH_SM2_KEY_BYTES, GroupB);
    bn_read_string_to_bytes(para.n, MH_SM2_KEY_BYTES, GroupN);
    bn_read_string_to_bytes(para.g.x, MH_SM2_KEY_BYTES, GroupX);
    bn_read_string_to_bytes(para.g.y, MH_SM2_KEY_BYTES, GroupY);
	
	//计算hash
	bn_read_string_to_bytes(IDa, sizeof(IDa), cIDa);
	mh_sm2_hash_z(_Za, IDa, 0x0080, &para, &pri_key.e);
	mh_sm2_hash_e(_e, _Za, message, messagelen);	
	
	//调用兆讯的数字签名接口	
	//mh_sm2_digital_sign(&mhsm2sign, _Za, message, messagelen, &pri_key, &para, dev_trng_read, NULL);	
	if(MH_RET_SM2_VERIFY_SUCCESS != mh_sm2_digital_sign_with_e(&mhsm2sign, _e, NULL, 0, &pri_key, &para, dev_trng_read, NULL))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/****************************************************************************
**Description:	    基于国密算法sm2的产生公私钥对
**Input parameters:	
**Output parameters: Generating public and private keys
**Returned value:
                    0:成功
**Created by:		husheng 20190625
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
int dev_sm2_gen_PubAndPri_keys(char *pPkeyD, char *pPkeyX, char *pPkeyY)
{
	mh_sm2_private_key keyTmp;
	mh_sm2_ellipse_para para;

	//外部传入mh_sm2_ellipse_para
	bn_read_string_to_bytes(para.p, MH_SM2_KEY_BYTES, GroupP);
    bn_read_string_to_bytes(para.a, MH_SM2_KEY_BYTES, GroupA);
    bn_read_string_to_bytes(para.b, MH_SM2_KEY_BYTES, GroupB);
    bn_read_string_to_bytes(para.n, MH_SM2_KEY_BYTES, GroupN);
    bn_read_string_to_bytes(para.g.x, MH_SM2_KEY_BYTES, GroupX);
    bn_read_string_to_bytes(para.g.y, MH_SM2_KEY_BYTES, GroupY);

	//调用兆讯的产生公私钥对接口
	if(MH_RET_SM2_VERIFY_SUCCESS != sm2_genkey(&keyTmp, &para, dev_trng_read, NULL))	
	{
		return 1;
	}
	else
	{
		memcpy(pPkeyD,keyTmp.d,MH_SM2_KEY_BYTES);
	    memcpy(pPkeyX,keyTmp.e.x,MH_SM2_KEY_BYTES);
		memcpy(pPkeyY,keyTmp.e.y,MH_SM2_KEY_BYTES);

		return 0;
	}	
}

#if 0

//sm2加密解密测试
void dev_sm2_encrypt_decrypt_self_test(void)
{
	mh_sm2_ellipse_para para;
	mh_sm2_private_key key, keyTmp;

	uint8_t test_cplain[40];
	uint8_t test_ccipher[40 + 65 + 32];

	uint8_t message[40];
	uint8_t cipher[40 + 65 + 32];
    
	uint32_t mlen;
	uint32_t clen, u32Tmp;
//	uint8_t rand[32];

	DBG_PRINTF("\nSM2 crypt Test In\n");	

	//填充mh_sm2_ellipse_para结构体
	bn_read_string_to_bytes(para.p, MH_SM2_KEY_BYTES, GroupP);
	bn_read_string_to_bytes(para.a, MH_SM2_KEY_BYTES, GroupA);
	bn_read_string_to_bytes(para.b, MH_SM2_KEY_BYTES, GroupB);
	bn_read_string_to_bytes(para.n, MH_SM2_KEY_BYTES, GroupN);
	bn_read_string_to_bytes(para.g.x, MH_SM2_KEY_BYTES, GroupX);
	bn_read_string_to_bytes(para.g.y, MH_SM2_KEY_BYTES, GroupY);

	bn_read_string_to_bytes(key.d, MH_SM2_KEY_BYTES, ckeyd);
	bn_read_string_to_bytes(key.e.x, MH_SM2_KEY_BYTES, cex);
	bn_read_string_to_bytes(key.e.y, MH_SM2_KEY_BYTES, cey);

    sm2_genkey(&keyTmp, &para, mh_rand_p, NULL);
    ouputRes("genKey\n", keyTmp.d, MH_SM2_KEY_BYTES);
    ouputRes("genKey\n", keyTmp.e.x, MH_SM2_KEY_BYTES);
    ouputRes("genKey\n", keyTmp.e.y, MH_SM2_KEY_BYTES);
    
    memset(&keyTmp, 0, sizeof(keyTmp));
    memcpy(keyTmp.d, key.d, sizeof(key.d));
    
    sm2_completeKey(&keyTmp, &para, mh_rand_p, NULL);
    r_printf((0 == memcmp(key.e.x, keyTmp.e.x, sizeof(key.e.x))), "ecc_completeKey X\n");
    r_printf((0 == memcmp(key.e.y, keyTmp.e.y, sizeof(key.e.y))), "ecc_completeKey Y\n");

    clen = bn_read_string_from_head(cipher, sizeof(cipher), ccipher);
	mlen = bn_read_string_from_head(message, sizeof(message), cm);
	memset(test_ccipher, 0, sizeof(test_ccipher));
	memset(test_cplain, 0, sizeof(test_cplain));
	
    if (0 == mlen)
    {
        DBG_PRINT("NULL plain\n");
        while(1);
    }
    
    //?§aa2???§???§1?¨o???§22??a??§1
    mh_sm2_dec(test_cplain, &u32Tmp, cipher, clen, &key, &para, mh_rand_p, NULL);
    ouputRes("text\n", message, mlen);
    ouputRes("text1\n", test_cplain, u32Tmp);
    
    r_printf((0 == memcmp(message, test_cplain, mlen)), "sm2 decrypt\n");

    //?¨￠???§???¨|?¨￠??a??§1
	r_printf((MH_RET_SM2_ENC_SUCCESS == mh_sm2_enc(test_ccipher, &u32Tmp, message, mlen, &key.e, &para, mh_rand_p, NULL)), "sm2 encrypt\n");
    ouputRes("SM2\n", test_ccipher, u32Tmp);
    memset(test_cplain, 0, sizeof(test_cplain));
	mh_sm2_dec(test_cplain, &u32Tmp, test_ccipher, u32Tmp, &key, &para, mh_rand_p, NULL);
	r_printf((0 == memcmp(message, test_cplain, mlen)), "sm2 decrypt\n");
    
    
    clen = bn_read_string_from_head(cipher, sizeof(cipher), std_ccipher);
	mlen = bn_read_string_from_head(message, sizeof(message), cm);
    memset(test_ccipher, 0, sizeof(test_ccipher));
	memset(test_cplain, 0, sizeof(test_cplain));
    
    if (0 == mlen)
    {
        DBG_PRINT("NULL plain\n");
        while(1);
    }
    
    mh_sm2_dec_std(test_cplain, &u32Tmp, cipher, clen, &key, &para, mh_rand_p, NULL);
	r_printf((0 == memcmp(message, test_cplain, mlen)), "sm2 decrypt\n");
    
    //?¨￠???§???¨|?¨￠??a??§1
	r_printf((MH_RET_SM2_ENC_SUCCESS == mh_sm2_enc_std(test_ccipher, &u32Tmp, message, mlen, &key.e, &para, mh_rand_p, NULL)), "sm2 encrypt\n");
    ouputRes("SM2\n", test_ccipher, clen);
    memset(test_cplain, 0, sizeof(test_cplain));
	mh_sm2_dec_std(test_cplain, &u32Tmp, test_ccipher, u32Tmp, &key, &para, mh_rand_p, NULL);
	r_printf((0 == memcmp(message, test_cplain, mlen)), "sm2 decrypt\n");
    //?§aa2???§???§1?¨o???§22??a??§1

}
#endif
#endif

