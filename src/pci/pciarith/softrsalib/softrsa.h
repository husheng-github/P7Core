/* RSA.H - header file for RSA.C */

/* Copyright (C) RSA Laboratories, a division of RSA Data Security, Inc., created 1991. All rights reserved. */

#ifndef _SOFTRSA_H_
#define _SOFTRSA_H_ 1

#include "rsaref.h"

#ifdef __cplusplus
extern "C" {
#endif


int RSAPublicEncrypt  PROTO_LIST((unsigned char *, unsigned int *, unsigned char *, unsigned int, R_RSA_PUBLIC_KEY *, R_RANDOM_STRUCT *));
int RSAPrivateDecrypt PROTO_LIST((unsigned char *, unsigned int *, unsigned char *, unsigned int, R_RSA_PRIVATE_KEY *));

int RSAPrivateEncrypt PROTO_LIST((unsigned char *, unsigned int *, unsigned char *, unsigned int, R_RSA_PRIVATE_KEY *));
int RSAPublicDecrypt  PROTO_LIST((unsigned char *, unsigned int *, unsigned char *, unsigned int, R_RSA_PUBLIC_KEY *));
int RSAPublicDecrypt1  PROTO_LIST((unsigned char *,unsigned int *, unsigned char *,unsigned int, R_RSA_PUBLIC_KEY *)); 
#ifdef __cplusplus
}
#endif

#endif // _RSA_H_
