

#include "ddi_arith.h"
#include "mh_crypt.h"
#include "mh_sm3.h"
#include "mh_sm4.h"
#include "mh_rand.h"
#include "pci/pciglobal.h"
//#include "devapi/dev_maths/dev_maths.h"

void ddi_arith_hash(u8 *data,u32 datalen,u8 *hashvalue,u8 type)
{
	pcicorecallfunc.pciarith_hash(data,datalen,hashvalue,type);
}


void ddi_arith_des(short encrypt,u8 *input,u8 *deskey)
{
	pcicorecallfunc.pciarith_desencrypt(encrypt,input,deskey);
}


s32 ddi_arith_getrand(u8 *randdata,u32 randdatalen)
{
	return pcicorecallfunc.pci_get_random_bytes(randdata,randdatalen);
}

s32 ddi_arith_rsaencrypt(u8 *output, u32 *outputLen,u8 *input,u32 inputLen,u8 *PK, u32 pkLen)
{
	return pcicorecallfunc.RSA_PKEncrypt(output,outputLen,input,inputLen,PK,pkLen);
}

s32 ddi_arith_rsadecrypt(u8 *output, u32 *outputLen,u8 *input,u32 inputLen,u8 *PK, u32 pkLen)
{
	return pcicorecallfunc.RSA_PKDecrypt(output,outputLen,input,inputLen,PK,pkLen);
}


s32 ddi_arith_rsaskencrypt(u8 *output, u32 *outputLen,u8 *input,u32 inputLen,u8 *SK, u32 skLen)
{
	return pcicorecallfunc.RSA_SKEncrypt(output,outputLen,input,inputLen,SK,skLen);
}

s32 ddi_arith_rsaskdecrypt(u8 *output, u32 *outputLen,u8 *input,u32 inputLen,u8 *SK, u32 skLen)
{
	return pcicorecallfunc.RSA_SKDecrypt(output,outputLen,input,inputLen,SK,skLen);
}


unsigned char RSARecover(unsigned char *m, unsigned int mLen, unsigned char *e, unsigned int eLen,unsigned char *input,unsigned char *output)
{
	return dev_rsarecover(m,mLen,e,eLen,input,output);
}

s32 ddi_arith_rsarecover(u8 *m, u32 mLen, u8 *e, u32 eLen,u8 *input,u8 *output)
{
	return RSARecover(m,mLen,e,eLen,input,output);
}

s32 ddi_arith_sm2veritysignature(const char *pPkeyX, const char *pPkeyY, const char *pSignR, const char *pSignS,u8 *message,u32 messagelen)
{
	return dev_sm2_verity(pPkeyX,pPkeyY,pSignR,pSignS,message,messagelen);
}

s32 ddi_arith_sm2encrypt(u8 *output, u32 *outputLen,u8 *input,u32 inputLen,u8 *PK, u32 pkLen)
{
    return dev_sm2_PKEncrypt(output,outputLen,input,inputLen,PK,pkLen);
}

s32 ddi_arith_sm2decrypt(u8 *output, u32 *outputLen,u8 *input,u32 inputLen,u8 *PK, u32 pkLen)
{
    return dev_sm2_PKDecrypt(output,outputLen,input,inputLen,PK,pkLen);
}

s32 ddi_arith_sm2skencrypt(u8 *output, u32 *outputLen,u8 *input,u32 inputLen,u8 *SK, u32 skLen)
{
    return dev_sm2_SKEncrypt(output,outputLen,input,inputLen,SK,skLen);
}

s32 ddi_arith_sm2skdecrypt(u8 *output, u32 *outputLen,u8 *input,u32 inputLen,u8 *SK, u32 skLen)
{
    return dev_sm2_SKDecrypt(output,outputLen,input,inputLen,SK,skLen);
}

s32 ddi_arith_sm2_digital_sign(const char *pPkeyD, const char *pPkeyX, const char *pPkeyY, const char *pSignR, const char *pSignS,u8 *message,u32 messagelen)
{
    return dev_sm2_digital_sign(pPkeyD,pPkeyX,pPkeyY,pSignR,pSignS,message,messagelen);
}

s32 ddi_arith_sm2_gen_PubAndPri_keys(char *pPkeyD, char *pPkeyX, char *pPkeyY)
{
    return dev_sm2_gen_PubAndPri_keys(pPkeyD,pPkeyX,pPkeyY);
}

s32 ddi_arith_sm3(u8 *pInput, u32 ilen, u8 *pOutput)
{
    u32 ret;
    ret = mh_sm3(pOutput, pInput, ilen);
    if(ret == MH_RET_SM3_SUCCESS)
    {
        return DDI_OK;
    }
    else
    {
        return DDI_EDATA;
    }
}


s32 ddi_arith_sm4(u8 arithtype,u8 *pInput, u32 ilen, u8 *pOutput,u8 *cKey)
{
	s32 ret;
	uint8_t Iv[16] = {0xA3, 0xB1, 0xBA, 0xC6, 0x56, 0xAA, 0x33, 0x50, 0x67, 0x7D, 0x91, 0x97, 0xB2, 0x70, 0x22, 0xDC};
    u8 m,n;

    m = (arithtype>>4)&0x0f;
    n = arithtype&0x0f;
	//ECB
    ret = DDI_EDATA;
	if(m == 0)
	{
    	if(n == 1)
    	{
    		ret = mh_sm4_enc(ECB, pOutput, ilen, pInput, ilen, cKey, Iv, mh_rand_p, NULL);
    	}
    	else if(n == 0)
    	{
    		ret= mh_sm4_dec(ECB, pOutput, ilen, pInput, ilen, cKey, Iv, mh_rand_p, NULL);
    	}
	}
    else if(m == 1)
    {
	    if(n == 1)
	    {
        	#if 0
        	//CBC
        	mh_sm4_enc(CBC, mCrypt, sizeof(mCrypt), Plain, sizeof(Plain), Key, Iv, mh_rand_p, NULL);
        	mh_sm4_dec(CBC, mPlain, sizeof(mPlain), mCrypt, sizeof(mCrypt), Key, Iv, mh_rand_p, NULL);
        	
        	//xCBC

        	mh_sm4_enc(CBC, xCrypt, sizeof(xCrypt), xPlain[0], sizeof(xPlain[0]), Key, Iv, mh_rand_p, NULL);
        	mh_sm4_dec(CBC, xPlain[1], sizeof(xPlain[1]), xCrypt, sizeof(xCrypt), Key, Iv, mh_rand_p, NULL);
    	    #endif
            ret = mh_sm4_enc(CBC, pOutput, ilen, pInput, ilen, cKey, Iv, mh_rand_p, NULL);
    	}
    	else if(n == 0)
    	{
    		ret= mh_sm4_dec(CBC, pOutput, ilen, pInput, ilen, cKey, Iv, mh_rand_p, NULL);
    	}
    }

	if(ret == MH_RET_SM4_SUCCESS)
	{
		ret = 0;
	}
	else
	{
		ret = -1;
	}

	return ret;
}
s32 ddi_arith_file_hash(u32 fileorgaddr,u32 inlen,u8 *hashvalue)
{
    return dev_file_hash(fileorgaddr, 0, inlen,hashvalue,0);//   sha256
}
s32 ddi_arith_md5(u8 *pheDest,  const u8 *pheSrc, s32 siLen)
{
//    dev_maths_caleMD5(pheDest, (u8*)pheSrc, siLen);//pengxuebin, 20191112
    return DDI_OK;
}


