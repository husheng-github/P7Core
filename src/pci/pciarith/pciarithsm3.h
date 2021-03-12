

#ifndef _PCIARITHSM3_H_
#define _PCIARITHSM3_H_

extern int dev_sm3_getsummary(unsigned char *pInput, unsigned int ilen, unsigned char *pOutput);
extern int pciarithsm3_GetSummary (const char *pPkeyX, const char *pPkeyY,const char *pSrcData, unsigned short pSrcDataLen,const char *pIDA, unsigned char *pSummary);

#endif


