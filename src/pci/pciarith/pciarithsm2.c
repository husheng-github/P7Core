

#include "pciglobal.h"




//sxl修改，返回0是正确的
int pciarith_CheckSM2Sign(const char *pPkeyX, const char *pPkeyY, const char *pSignR, const char *pSignS,const char *pSrcData, unsigned short pSrcDataLen, const char *pIDA)
{
	u8 pDstBuf[32] = {0};
	int nLen;
	int ret = -1;

	nLen = pciarithsm3_GetSummary(pPkeyX, pPkeyY, pSrcData, pSrcDataLen,pIDA, pDstBuf);
	if(nLen == 32)
	{
		
	}
	
}


