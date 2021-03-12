

#ifndef _DEV_TRNG_H_
#define _DEV_TRNG_H_



extern s32 dev_trng_open (void);
extern u32 dev_trng_read(void *trngdata,u32 trngdatalen);



extern void test_trng(void);


#endif

