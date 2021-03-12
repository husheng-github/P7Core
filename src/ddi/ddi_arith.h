

#ifndef _DDI_ARITH_H_
#define _DDI_ARITH_H_

#include "ddi_common.h"




//hardware arith
#define DDI_HASHTYPE_1     0
#define DDI_HASHTYPE_256   1
#define DDI_DES_DECRYPT    0
#define DDI_DES_ENCRYPT    1



//=====================================================
//对外函数声明
extern void ddi_arith_hash(u8 *data,u32 datalen,u8 *hashvalue,u8 type);
extern void ddi_arith_des(short encrypt,u8 *input,u8 *deskey);
extern s32 ddi_arith_getrand(u8 *randdata,u32 randdatalen);
extern s32 ddi_arith_rsaskencrypt(u8 *output, u32 *outputLen,u8 *input,u32 inputLen,u8 *SK, u32 skLen);
extern s32 ddi_arith_rsaskdecrypt(u8 *output, u32 *outputLen,u8 *input,u32 inputLen,u8 *SK, u32 skLen);
extern s32 ddi_arith_rsaencrypt(u8 *output, u32 *outputLen,u8 *input,u32 inputLen,u8 *PK, u32 pkLen);
extern s32 ddi_arith_rsadecrypt(u8 *output, u32 *outputLen,u8 *input,u32 inputLen,u8 *PK, u32 pkLen);
extern s32 ddi_arith_rsarecover(u8 *m, u32 mLen, u8 *e, u32 eLen,u8 *input,u8 *output);
extern s32 ddi_arith_sm2veritysignature(const char *pPkeyX, const char *pPkeyY, const char *pSignR, const char *pSignS,u8 *message,u32 messagelen);
extern s32 ddi_arith_sm3(u8 *pInput, u32 ilen, u8 *pOutput);
extern s32 ddi_arith_sm4(u8 arithtype,u8 *pInput, u32 ilen, u8 *pOutput,u8 *cKey);
s32 ddi_arith_file_hash(u32 fileorgaddr,u32 inlen,u8 *hashvalue);
s32 ddi_arith_md5(u8 *pheDest,  const u8 *pheSrc, s32 siLen);

s32 ddi_arith_sm2encrypt(u8 *output, u32 *outputLen,u8 *input,u32 inputLen,u8 *PK, u32 pkLen);
s32 ddi_arith_sm2decrypt(u8 *output, u32 *outputLen,u8 *input,u32 inputLen,u8 *PK, u32 pkLen);
s32 ddi_arith_sm2skencrypt(u8 *output, u32 *outputLen,u8 *input,u32 inputLen,u8 *SK, u32 skLen);
s32 ddi_arith_sm2skdecrypt(u8 *output, u32 *outputLen,u8 *input,u32 inputLen,u8 *SK, u32 skLen);
s32 ddi_arith_sm2_digital_sign(const char *pPkeyD, const char *pPkeyX, const char *pPkeyY, const char *pSignR, const char *pSignS,u8 *message,u32 messagelen);
s32 ddi_arith_sm2_gen_PubAndPri_keys(char *pPkeyD, char *pPkeyX, char *pPkeyY);

//===================================================
typedef void (*core_ddi_arith_hash)(u8 *data,u32 datalen,u8 *hashvalue,u8 type);
typedef void (*core_ddi_arith_des)(short encrypt,u8 *input,u8 *deskey);
typedef s32 (*core_ddi_arith_getrand)(u8 *randdata,u32 randdatalen);
typedef s32 (*core_ddi_arith_rsaencrypt)(u8 *output, u32 *outputLen,u8 *input,u32 inputLen,u8 *PK, u32 pkLen);
typedef s32 (*core_ddi_arith_rsadecrypt)(u8 *output, u32 *outputLen,u8 *input,u32 inputLen,u8 *PK, u32 pkLen);
typedef s32 (*core_ddi_arith_rsaskencrypt)(u8 *output, u32 *outputLen,u8 *input,u32 inputLen,u8 *SK, u32 skLen);
typedef s32 (*core_ddi_arith_rsaskdecrypt)(u8 *output, u32 *outputLen,u8 *input,u32 inputLen,u8 *SK, u32 skLen);
typedef s32 (*core_ddi_arith_rsarecover)(u8 *m, u32 mLen, u8 *e, u32 eLen,u8 *input,u8 *output);
typedef s32 (*core_ddi_arith_sm2veritysignature)(const char *pPkeyX, const char *pPkeyY, const char *pSignR, const char *pSignS,u8 *message,u32 messagelen);
typedef s32 (*core_ddi_arith_sm3)(u8 *pInput, u32 ilen, u8 *pOutput);
typedef s32 (*core_ddi_arith_sm4)(u8 arithtype,u8 *pInput, u32 ilen, u8 *pOutput,u8 *cKey);
typedef s32 (*core_ddi_arith_file_hash)(u32 fileorgaddr,u32 inlen,u8 *hashvalue);
typedef s32 (*core_ddi_arith_md5)(u8 *pheDest,  const u8 *pheSrc, s32 siLen);

typedef s32 (*core_ddi_arith_sm2encrypt)(u8 *output, u32 *outputLen,u8 *input,u32 inputLen,u8 *PK, u32 pkLen);
typedef s32 (*core_ddi_arith_sm2decrypt)(u8 *output, u32 *outputLen,u8 *input,u32 inputLen,u8 *PK, u32 pkLen);
typedef s32 (*core_ddi_arith_sm2skencrypt)(u8 *output, u32 *outputLen,u8 *input,u32 inputLen,u8 *SK, u32 skLen);
typedef s32 (*core_ddi_arith_sm2skdecrypt)(u8 *output, u32 *outputLen,u8 *input,u32 inputLen,u8 *SK, u32 skLen);
typedef s32 (*core_ddi_arith_sm2_digital_sign)(const char *pPkeyD, const char *pPkeyX, const char *pPkeyY, const char *pSignR, const char *pSignS,u8 *message,u32 messagelen);
typedef s32 (*core_ddi_arith_sm2_gen_PubAndPri_keys)(char *pPkeyD, char *pPkeyX, char *pPkeyY);


#endif

