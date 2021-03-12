

#include "pciglobal.h"



static const char GroupP[] = "FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFF";
static const char GroupA[] = "FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFC";
static const char GroupB[] = "28E9FA9E9D9F5E344D5A9E4BCF6509A7F39789F515AB8F92DDBCBD414D940E93";
static const char GroupX[] = "32C4AE2C1F1981195F9904466A39C9948FE30BBFF2660BE1715A4589334C74C7";
static const char GroupY[] = "BC3736A2F4F6779C59BDCEE36B692153D0A9877CC62A474002DF32E52139F0A0";
static const char GroupN[] = "FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFF7203DF6B21C6052B53BBF40939D54123";




int pciarithsm3_GetSummary (const char *pPkeyX, const char *pPkeyY,const char *pSrcData, unsigned short pSrcDataLen,const char *pIDA, unsigned char *pSummary)
{
	int nLenSum = 0;
	int nSrcLen = 0;
	int nIDALen = 0;
	int nLenKeyX = 0;
	int nLenKeyY = 0;
	unsigned char *pTmp = NULL;
	unsigned char *pZaTmp = NULL;
	int nLen = 0;
	
	if ((!pSrcData && !pIDA) ||!pSummary)
	{
		return nLenSum;
	}
	
	nSrcLen = pSrcDataLen;
	nIDALen = strlen(pIDA);
	if (nSrcLen == 0 && nIDALen == 0)
	{
		return nLenSum;
	}
	nLenKeyX = strlen(pPkeyX);
	nLenKeyY = strlen(pPkeyY);
	if (!pSrcData || nSrcLen == 0)
	{
	    if(nIDALen >256)
	    {
	    	return 0;
	    }
		pciarith_AscToBcd(pSummary, (u8*)pIDA, nIDALen);
		nLenSum = (nIDALen + 1) / 2;
		return nLenSum;
	}
	if (nSrcLen < 512)
	{
		nLen = 512+66;
	}
	else
	{
		nLen = nSrcLen + 66;
	}
	pTmp = (u8*)pcicorecallfunc.mem_malloc(sizeof(u8) * nLen);
	if (!pTmp)
	{
		return 0;
	}
	pZaTmp = (u8*)pcicorecallfunc.mem_malloc(sizeof(u8) * nLen); ///2
	if (!pZaTmp)
	{
		if (pZaTmp)
		{
			pcicorecallfunc.mem_free(pZaTmp);
			pZaTmp = NULL;
		}
		return 0;
	}
	nLen = 0;
	memcpy(pTmp, (unsigned char *)pIDA, nIDALen);
	nLen += nIDALen;
	memcpy(pTmp + nLen, (unsigned char *)GroupA, 64);
	nLen += 64;
    
	memcpy(pTmp + nLen, (unsigned char *)GroupB, 64);
	nLen += 64;
    
	memcpy(pTmp + nLen, (unsigned char *)GroupX, 64);
	nLen += 64;
    
	memcpy(pTmp + nLen, (unsigned char *)GroupY, 64);
	nLen += 64;
    
	memcpy(pTmp + nLen, (unsigned char *)pPkeyX, nLenKeyX);
	nLen += nLenKeyX;
    
	memcpy(pTmp + nLen, (unsigned char *)pPkeyY, nLenKeyY);
	nLen += nLenKeyY;
    
	pciarith_AscToBcd(pZaTmp, pTmp, nLen);  
	nLen = (nLen + 1) / 2;
	if (nIDALen != 0)
	{
		pciarith_U32ToHex(pTmp, nIDALen *4, 2);
		memcpy(pTmp + 2, pZaTmp, nLen);
		nLen += 2;
	}
	else
	{
		memcpy(pTmp, pZaTmp, nLen);
	}
	
	pTmp[nLen] = '\0';
	nLenSum = pcicorecallfunc.SM3Encrypt(pTmp, nLen, pZaTmp);
	
	memcpy(&pZaTmp[nLenSum],(unsigned char *)pSrcData,nSrcLen);
	nLen = nLenSum+nSrcLen;
    
	nLenSum = pcicorecallfunc.SM3Encrypt(pZaTmp, nLen, pTmp);
	if(nLenSum <128)
	{
		memcpy(pSummary, pTmp, nLenSum);
	}
	
	if (pTmp)
	{
		pcicorecallfunc.mem_free(pTmp);
		pTmp = NULL;
	}
	
	if (pZaTmp)
	{
		pcicorecallfunc.mem_free(pZaTmp);
		pZaTmp = NULL;
	}
	
	return nLenSum;
	
}




