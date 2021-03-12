

#ifndef _DEV_RSA_H_
#define _DEV_RSA_H_


extern int dev_RSA_SKEncrypt(unsigned char *output, unsigned int *outputLen,unsigned char *input,  unsigned int inputLen,unsigned char *SK, unsigned int skLen);
extern int dev_RSA_SKDecrypt(unsigned char * output, unsigned int *outputLen,unsigned char * input,  unsigned int inputLen,unsigned char * SK, unsigned int skLen);
extern int dev_RSA_PKEncrypt(unsigned char *output, unsigned int *outputLen,unsigned char *input,  unsigned int inputLen,unsigned char *PK, unsigned int pkLen);
extern int dev_RSA_PKDecrypt(unsigned char * output, unsigned int *outputLen,unsigned char * input,  unsigned int inputLen,unsigned char * PK, unsigned int pkLen);
extern s32 dev_rsarecover(u8 *m, u32 mLen, u8 *e, u32 eLen,u8 *input,u8 *output);


#endif

