#ifndef __rsalib_H_
#define __rsalib_H_

#include "define.h"


#include "pciglobal.h"

//RSA(512~4096)
 int RSA_GenPaires(unsigned char *PK, unsigned int *pkLen,
                             unsigned char *SK, unsigned int *skLen,
                             unsigned int bits );

 int RSA_PKEncrypt(unsigned char *output, unsigned int *outputLen,
                             unsigned char *input,  unsigned int inputLen,
                             unsigned char *PK, unsigned int pkLen);
 int RSA_SKDecrypt(unsigned char *output, unsigned int *outputLen,
                             unsigned char *input,  unsigned int inputLen,
                             unsigned char *SK, unsigned int skLen);

 int RSA_SKEncrypt(unsigned char *output, unsigned int *outputLen,
                             unsigned char *input, unsigned int inputLen,
                             unsigned char *SK, unsigned int skLen);
 int RSA_PKDecrypt(unsigned char * output, unsigned int *outputLen,
                             unsigned char * input,  unsigned int inputLen,
                             unsigned char * PK, unsigned int pkLen);
 unsigned char RSARecover(unsigned char *m, unsigned int mLen, unsigned char *e, unsigned int eLen,unsigned char *input,unsigned char *output);
//-----------------------------------------------------------------------------


#endif // __rsalib_H_
