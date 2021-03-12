/* R_RANDOM.H - header file for R_RANDOM.C */

/* Copyright (C) RSA Laboratories, a division of RSA Data Security, Inc., created 1991. All rights reserved. */
#ifndef _R_RANDOM_H_
#define _R_RANDOM_H_    1

#include "global.h"
#include "rsaref.h"

#ifdef __cplusplus
extern "C" {
#endif

int R_GenerateBytes PROTO_LIST ((unsigned char *, unsigned int, R_RANDOM_STRUCT *));

#ifdef __cplusplus
}
#endif

#endif // _RSA_H_
