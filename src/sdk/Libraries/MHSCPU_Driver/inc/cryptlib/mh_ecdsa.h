#ifndef __MH_ECDSA_H
#define __MH_ECDSA_H

#ifdef __cplusplus
 extern "C" {
#endif 

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "mh_ecc.h"
#include "mh_sm2.h"
#include "mh_sm3.h"
#include "mh_misc.h"
#include "mh_rand.h"



typedef struct
{
    ecc_bignum_key eb_r;
    ecc_bignum_key eb_s;
}mh_ecc_sign;





uint32_t mh_ecdsa_digital_sign(mh_ecc_sign *pSign, ecc_bignum_key *pBigE, mh_ecc_private_key *pKey, mh_ecc_curve_para *pEccPara,
                                mh_rng_callback f_rng, void *p_rng);

uint32_t mh_ecdsa_verify_sign(mh_ecc_sign *pSign, ecc_bignum_key *pBigE, mh_ecc_public_key *pubKey, 
                                mh_ecc_curve_para *pEccPara, mh_rng_callback f_rng, void *p_rng);

uint32_t mh_ecc_hash_e(ecc_bignum_key *pBigE, ecc_bignum_data *pBigMsg, mh_ecc_curve_para *para);
#ifdef __cplusplus
}
#endif

#endif 

