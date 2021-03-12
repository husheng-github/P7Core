
#include "devglobal.h"
#include "sdk/mhscpu_sdk.h"
#include "mh_crypt.h"
#include "mh_rand.h"




uint32_t mh_trrand_p(void *rand, uint32_t bytes, void *p_rng)
{
    //return mh_rand(rand, bytes);
    
    return bytes;  //sxl?2017 要使用随机数
}

void devarith_desencrypt(short encrypt,unsigned char *input,unsigned char *deskey)
{
    mh_rng_callback f_rng = mh_trrand_p;
    
    if(encrypt == ENCRYPT)  // hardware is 0 encrypted
    {
        mh_des_enc(ECB, input, 8, input, 8, deskey, NULL, f_rng, NULL);
    }
    else
    {
        mh_des_dec(ECB, input, 8, input, 8, deskey, NULL, f_rng, NULL);
    }
    
}


