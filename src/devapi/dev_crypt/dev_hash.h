

#ifndef _DEV_HASH_
#define _DEV_HASH_

#define HASHTYPE_1    0
#define HASHTYPE_256  1

#define SHA_BLOCKSIZE         64

extern void devarith_hash(u8 *data,u32 datalen,u8 *hashvalue,u8 type);

#endif



