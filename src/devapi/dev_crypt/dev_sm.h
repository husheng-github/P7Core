

#ifndef _DEV_SM_H_
#define _DEV_SM_H_

extern int dev_sm3_getsummary(unsigned char *pInput, unsigned int ilen, unsigned char *pOutput);
extern int dev_sm2_verity(const char *pPkeyX, const char *pPkeyY, const char *pSignR, const char *pSignS,u8 *message,u32 messagelen);

int dev_sm2_PKEncrypt(unsigned char *output, unsigned int *outputLen,unsigned char *input,  unsigned int inputLen,unsigned char *PK, unsigned int pkLen);
int dev_sm2_PKDecrypt(unsigned char * output, unsigned int *outputLen,unsigned char * input,  unsigned int inputLen,unsigned char * PK, unsigned int pkLen);
int dev_sm2_SKEncrypt(unsigned char *output, unsigned int *outputLen,unsigned char *input,  unsigned int inputLen,unsigned char *SK, unsigned int skLen);
int dev_sm2_SKDecrypt(unsigned char * output, unsigned int *outputLen,unsigned char * input,  unsigned int inputLen,unsigned char * SK, unsigned int skLen);
int dev_sm2_digital_sign(const char *pPkeyD, const char *pPkeyX, const char *pPkeyY, const char *pSignR, const char *pSignS,u8 *message,u32 messagelen);
int dev_sm2_gen_PubAndPri_keys(char *pPkeyD, char *pPkeyX, char *pPkeyY);

#endif


