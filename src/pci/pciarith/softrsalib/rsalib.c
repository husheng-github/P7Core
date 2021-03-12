/**
 *Unit Name: crc32.cpp
 *Current Version: 1.0
 *Description:
 *   CRC32算法.
 *Author:
 *   fanzhiqiang@xinguodu.com
 *Copyright:
 *   Copyright(C) 2009.10
 *History:
 *TODO:
**/
//#include <linux/slab.h>
//#include <linux/kernel.h>
//#include <linux/init.h>
//#include <linux/module.h>
// #include <math.h>
// #include <time.h>
// #include <linux/string.h>

#include <string.h>
#include <stdlib.h>
#include <string.h>
#include "rsalib.h"



/**
 * RSA import function.
 **/
#include "global.h"
#include "softrsa.h"

#ifndef NULL
#define NULL 0
#endif


#define RSA_PUBLIC_EXPONENT     65537       //E: RSA public exponent

static int WriteBigInt(unsigned char *pBuf, unsigned char *BitInt, unsigned int Len)
{
	int iRet = 0;

    while (*BitInt == 0 && Len > 0)
    {
        BitInt++;
        Len--;
    }
    if (Len == 0) return 0;

    for (; Len > 0; Len--, iRet++)
    {
        *pBuf++ = (unsigned char)(*BitInt++);
    }

    return iRet;
}

static void ReadBigInt(unsigned char *BigInt, unsigned int sizeBigInt, unsigned char *pBuf, unsigned int Len)
{
	int i;
    memset(BigInt, 0, sizeBigInt);

    BigInt += sizeBigInt - Len;
    for (i=0; i<(int)Len; i++)
    {
        *BigInt++ = *pBuf++;
    }
}

//输出公钥及随机数
static void OutputPK(unsigned char *PK, unsigned int *pkLen, R_RSA_PUBLIC_KEY *publicKey, R_RANDOM_STRUCT *randomStruct)
{
	int ret;
    *pkLen = 0;

    DW2P(PK, publicKey->bits);
    PK += 4;
    *pkLen += 4;
    ret = WriteBigInt(PK, publicKey->modulus, sizeof(publicKey->modulus));
    PK += ret;
    *pkLen += ret;
    ret = WriteBigInt(PK, randomStruct->state, sizeof(randomStruct->state));
    PK += ret;
    *pkLen += ret;
    ret = WriteBigInt(PK, randomStruct->output, sizeof(randomStruct->output));
    PK += ret;
    *pkLen += ret;
}

//输入公钥及随机数
static void InputPK(R_RSA_PUBLIC_KEY *publicKey, R_RANDOM_STRUCT *randomStruct, unsigned char *PK, unsigned int pkLen)
{
	int modulusLen;
	unsigned char *tmp;

    P2DW(publicKey->bits, PK);
    PK += 4;

    modulusLen = (publicKey->bits + 7) / 8;

    //E: public exponent
    memset(publicKey->exponent, 0, sizeof(publicKey->exponent));
    tmp = &publicKey->exponent[sizeof(publicKey->exponent) - 4 + 1];
    DW3P(tmp, RSA_PUBLIC_EXPONENT);

    ReadBigInt(publicKey->modulus, sizeof(publicKey->modulus), PK, modulusLen);
    PK += modulusLen;

    ReadBigInt(randomStruct->state, sizeof(randomStruct->state), PK, 16);
    PK += 16;
    ReadBigInt(randomStruct->output, sizeof(randomStruct->output), PK, 16);
    PK += 16;

    randomStruct->bytesNeeded = 0;
    randomStruct->outputAvailable = 0;
}

//输出私钥
static void OutputSK(unsigned char *SK, unsigned int *skLen, R_RSA_PRIVATE_KEY *privateKey)
{
	int ret;
    *skLen = 0;

    DW2P(SK, privateKey->bits);
    SK += 4;
    *skLen += 4;
    ret = WriteBigInt(SK, privateKey->modulus, sizeof(privateKey->modulus));
    SK += ret;
    *skLen += ret;
    ret = WriteBigInt(SK, privateKey->exponent, sizeof(privateKey->exponent));
    SK += ret;
    *skLen += ret;
    ret = WriteBigInt(SK, privateKey->prime[0], sizeof(privateKey->prime[0]));
    SK += ret;
    *skLen += ret;
    ret = WriteBigInt(SK, privateKey->prime[1], sizeof(privateKey->prime[1]));
    SK += ret;
    *skLen += ret;
    ret = WriteBigInt(SK, privateKey->primeExponent[0], sizeof(privateKey->primeExponent[0]));
    SK += ret;
    *skLen += ret;
    ret = WriteBigInt(SK, privateKey->primeExponent[1], sizeof(privateKey->primeExponent[1]));
    SK += ret;
    *skLen += ret;
    ret = WriteBigInt(SK, privateKey->coefficient, sizeof(privateKey->coefficient));
    SK += ret;
    *skLen += ret;
}

static void InputSK(R_RSA_PRIVATE_KEY *privateKey, unsigned char *SK, unsigned int skLen)
{
	unsigned char *tmp;
	int modulusLen;
	int primeLen;

    P2DW(privateKey->bits, SK);
    SK += 4;

    //E: public exponent
    memset(privateKey->publicExponent, 0, sizeof(privateKey->publicExponent));
    tmp = &privateKey->publicExponent[sizeof(privateKey->publicExponent) - 4 + 1];
    DW3P(tmp, RSA_PUBLIC_EXPONENT);
	modulusLen = (privateKey->bits + 7) / 8;
	primeLen = modulusLen / 2;

    ReadBigInt(privateKey->modulus, sizeof(privateKey->modulus), SK, modulusLen);
    SK += modulusLen;
    ReadBigInt(privateKey->exponent, sizeof(privateKey->exponent), SK, modulusLen);
    SK += modulusLen;
    ReadBigInt(privateKey->prime[0], sizeof(privateKey->prime[0]), SK, primeLen);
    SK += primeLen;
    ReadBigInt(privateKey->prime[1], sizeof(privateKey->prime[1]), SK, primeLen);
    SK += primeLen;
    ReadBigInt(privateKey->primeExponent[0], sizeof(privateKey->primeExponent[0]), SK, primeLen);
    SK += primeLen;
    ReadBigInt(privateKey->primeExponent[1], sizeof(privateKey->primeExponent[1]), SK, primeLen);
    SK += primeLen;
    ReadBigInt(privateKey->coefficient, sizeof(privateKey->coefficient), SK, primeLen);
    SK += primeLen;
}

/**
 *RSA 产生密钥对(512~4096bits)
 *512-bits:
 *   PK: 100 bytes; SK:  292 bytes; Block:  64 bytes; TIME: 0s
 *1024-bits:
 *   PK: 164 bytes; SK:  580 bytes; Block: 128 bytes; TIME: 1s
 *2048-bits:
 *   PK: 292 bytes; SK: 1156 bytes; Block: 256 bytes; TIME: 2~20s
 *4096-bits:
 *   PK: 548 bytes; SK: 2308 bytes; Block: 512 bytes; TIME: 180s~380s
 *Parameters:
 *  PK/pkLen[out] - 输出的公钥
 *  SK/skLen[out] - 输出的私钥
 *  bits[in] - 密钥位数
 *Return:
 *  0 - 成功
 * >0 - 失败
 **/
extern void usip_get_random_bytes(unsigned char *buf, int nbytes);  //sxl?
int RSA_GenPaires(unsigned char *PK, unsigned int *pkLen,
                  unsigned char *SK, unsigned int *skLen,
                  unsigned int bits)
{
    R_RANDOM_STRUCT   randomStruct;
    R_RSA_PROTO_KEY   protoKey;
    R_RSA_PUBLIC_KEY  *publicKey;
    R_RSA_PRIVATE_KEY *privateKey;
	int status; 
	//int i;
	static unsigned char seedByte[RANDOM_BYTES_NEEDED / 8] ;
	unsigned int bytesNeeded;

	if(bits == 0 || bits > 4096)
		bits = 1024;

    protoKey.bits = bits;
    protoKey.useFermat4 = 1;            //E:65537
	
    // srand((unsigned int)time(NULL));
	memset(seedByte, 0x00,sizeof(seedByte)/sizeof(seedByte[0]));
    R_RandomInit(&randomStruct);
    /* Initialize with all zero seed bytes, which will not yield an actual random number output. */
    while (1)
    {
        R_GetRandomBytesNeeded(&bytesNeeded, &randomStruct);
        if (bytesNeeded == 0) 
			break;

        //for (i=0; i<(int)sizeof(seedByte); i++)
        //     seedByte[i] = (unsigned char)((int)(gaussRand() * 1000) % 0xFF);
		usip_get_random_bytes(seedByte,sizeof(seedByte)/sizeof(seedByte[0])); 
        R_RandomUpdate(&randomStruct, seedByte, sizeof(seedByte));
    }
	

	publicKey = (R_RSA_PUBLIC_KEY *)pcicorecallfunc.mem_malloc(sizeof(R_RSA_PUBLIC_KEY));
	if(publicKey == NULL)
	{
		return -1;
	}

	privateKey = (R_RSA_PRIVATE_KEY *)pcicorecallfunc.mem_malloc(sizeof(R_RSA_PRIVATE_KEY));
	if(privateKey == NULL)
	{
		pcicorecallfunc.mem_free(publicKey);
		return -1;
	}

    status = R_GeneratePEMKeys(publicKey, privateKey, &protoKey, &randomStruct);
	//DEBUG_INFO();
    if (0 == status)
    {
		//DEBUG_INFO();
        OutputPK(PK, pkLen, publicKey, &randomStruct);
		//DEBUG_INFO();
        OutputSK(SK, skLen, privateKey);
		//DEBUG_INFO();
    }

    R_RandomFinal (&randomStruct);
	//DEBUG_INFO();

    pcicorecallfunc.mem_free(publicKey);
	pcicorecallfunc.mem_free(privateKey);
    return status;
}
//EXPORT_SYMBOL_GPL(RSA_GenPaires);

/**
 *RSA 公钥加密.
 *Parameters:
 *  output/outputLen[out] - 输出加密数据
 *  input/inputLen[in] - 源数据
 *  PK/pkLen[in] - 公钥
 *Return:
 *  0 - 成功
 * >0 - 失败
 *Memo:
 *  使用RSA_SKDecrypt()进行解密.
 **/
int RSA_PKEncrypt(unsigned char *output, unsigned int *outputLen,
                  unsigned char *input,  unsigned int inputLen,
                  unsigned char *PK, unsigned int pkLen)
{
    R_RANDOM_STRUCT   randomStruct;
    R_RSA_PUBLIC_KEY  *publicKey;
	int ret;

    publicKey = (R_RSA_PUBLIC_KEY *)pcicorecallfunc.mem_malloc(sizeof(R_RSA_PUBLIC_KEY));
	if(publicKey == NULL)
	{
		return -1;
	}
    InputPK(publicKey, &randomStruct, PK, pkLen);

    ret = RSAPublicEncrypt(output, outputLen, input, inputLen, publicKey, &randomStruct);

	pcicorecallfunc.mem_free(publicKey);
	return ret;
}
//EXPORT_SYMBOL_GPL(RSA_PKEncrypt);

/**
 *RSA 私钥解密.
 *Parameters:
 *  output/outputLen[out] - 输出解密数据
 *  input/inputLen[in] - 源数据
 *  SK/skLen[in] - 私钥
 *Return:
 *  0 - 成功
 * >0 - 失败
 *Memo:
 *  对RSA_PKEncrypt()的加密数据解密.
 **/
int RSA_SKDecrypt(unsigned char *output, unsigned int *outputLen,
                  unsigned char *input,  unsigned int inputLen,
                  unsigned char *SK, unsigned int skLen)
{
    R_RSA_PRIVATE_KEY *privateKey;
	int ret;

	privateKey = (R_RSA_PRIVATE_KEY *)pcicorecallfunc.mem_malloc(sizeof(R_RSA_PRIVATE_KEY));
	if(privateKey == NULL)
	{
		return -1;
	}

    InputSK(privateKey, SK, skLen);

    ret =  RSAPrivateDecrypt(output, outputLen, input, inputLen, privateKey);
	pcicorecallfunc.mem_free(privateKey);
	return ret;
}
//EXPORT_SYMBOL_GPL(RSA_SKDecrypt);
//-----------------------------------------------------------------------------

/**
 *RSA 私钥加密.
 *Parameters:
 *  output/outputLen[out] - 输出加密数据
 *  input/inputLen[in] - 源数据
 *  SK/skLen[in] - 私钥
 *Return:
 *  0 - 成功
 * >0 - 失败
 *Memo:
 *  使用RSA_PKDecrypt()进行解密.
 **/
int RSA_SKEncrypt(unsigned char *output, unsigned int *outputLen,
                  unsigned char *input, unsigned int inputLen,
                  unsigned char *SK, unsigned int skLen)
{
    R_RSA_PRIVATE_KEY *privateKey;
	int ret;

    privateKey = (R_RSA_PRIVATE_KEY *)pcicorecallfunc.mem_malloc(sizeof(R_RSA_PRIVATE_KEY));
	if(privateKey == NULL)
	{
		return -1;
	}
	
    InputSK(privateKey, SK, skLen);

    ret = RSAPrivateEncrypt(output, outputLen, input, inputLen, privateKey);
    pcicorecallfunc.mem_free(privateKey);
	return ret;
}
//EXPORT_SYMBOL_GPL(RSA_SKEncrypt);

/**
 *RSA 公钥解密.
 *Parameters:
 *  output/outputLen[out] - 输出解密数据
 *  input/inputLen[in] - 源数据
 *  PK/pkLen[in] - 公钥
 *Return:
 *  0 - 成功
 * >0 - 失败
 *Memo:
 *  对RSA_SKEncrypt()的加密数据解密.
 **///sxl1220
//static spinlock_t rsapkdecryptlock = SPIN_LOCK_UNLOCKED;

int RSA_PKDecrypt(unsigned char * output, unsigned int *outputLen,
                  unsigned char * input,  unsigned int inputLen,
                  unsigned char * PK, unsigned int pkLen)
{
    R_RANDOM_STRUCT   *randomStruct;  //sxl?
    R_RSA_PUBLIC_KEY  *publicKey;     //sxl?
	int ret;

	//spin_lock(&rsapkdecryptlock);

    randomStruct = (R_RANDOM_STRUCT *)pcicorecallfunc.mem_malloc(sizeof(R_RANDOM_STRUCT));
	publicKey = (R_RSA_PUBLIC_KEY *)pcicorecallfunc.mem_malloc(sizeof(R_RSA_PUBLIC_KEY));
    InputPK(publicKey, randomStruct, PK, pkLen);
	pcicorecallfunc.mem_free(randomStruct);

    ret = RSAPublicDecrypt(output, outputLen, input, inputLen, publicKey);
	pcicorecallfunc.mem_free(publicKey);

	//spin_unlock(&rsapkdecryptlock);
	return ret;
}
//EXPORT_SYMBOL_GPL(RSA_PKDecrypt);
//-----------------------------------------------------------------------------

unsigned char RSARecover(unsigned char *m, unsigned int mLen, unsigned char *e, unsigned int eLen,unsigned char *input,unsigned char *output)
{
	unsigned int i,out_len;
	R_RSA_PUBLIC_KEY *publicKey;

	publicKey = (R_RSA_PUBLIC_KEY *)pcicorecallfunc.mem_malloc(sizeof(R_RSA_PUBLIC_KEY));
	if(publicKey == NULL)
	{
		return 1;
	}
	memset((unsigned char*)publicKey,0,sizeof(R_RSA_PUBLIC_KEY));
	publicKey->bits = mLen*8;
	if(eLen == 0x01)
		publicKey->exponent[MAX_RSA_MODULUS_LEN-1] = *e;	
	else if(eLen == 0x03)
	{
		if(memcmp(e,"\x00\x00\x03",3) == 0)
	    {
	    	publicKey->exponent[MAX_RSA_MODULUS_LEN-1] = 0x03;
	    }
		else
		{
			publicKey->exponent[MAX_RSA_MODULUS_LEN-1] = *e;
			publicKey->exponent[MAX_RSA_MODULUS_LEN-2] = *(e+1);
			publicKey->exponent[MAX_RSA_MODULUS_LEN-3] = *(e+2);
		}
	}
	else 
	{
	    pcicorecallfunc.mem_free(publicKey);
		return 1;
	}
	memcpy(&publicKey->modulus[MAX_RSA_MODULUS_LEN-mLen],m,mLen);
	i = RSAPublicDecrypt1(output,&out_len,input,mLen,publicKey);
	pcicorecallfunc.mem_free(publicKey);
	return i;
}

//EXPORT_SYMBOL_GPL(RSARecover);

