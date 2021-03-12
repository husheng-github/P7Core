

#ifndef _PCIARITHHASH_H_
#define _PCIARITHHASH_H_




//#define USESOFTHASHALTH
#ifdef  USESOFTHASHALTH

#define SHA_VERSION 1

#define SHA_BYTE_ORDER 1234

typedef unsigned long SHALONG;   


#define SHA_DIGESTSIZE        20

typedef struct //__attribute__ ((__packed__))
{
	SHALONG digest[5];      
	SHALONG count_lo, count_hi;  
	unsigned char data[SHA_BLOCKSIZE]; 
	int local;           
}PCISHA_INFO;



typedef struct {
   unsigned char data[64];
   unsigned int datalen;
   unsigned int bitlen[2];
   unsigned int state[8];
} SHA256_CTX;


extern void pcisha_init(PCISHA_INFO *sha_info);
extern void PCIsha_update(PCISHA_INFO *sha_info, unsigned char *buffer, int count);
extern void pcisha_final(unsigned char digest[20], PCISHA_INFO *sha_info);
extern void sha256_init(SHA256_CTX *ctx);

#endif

extern void devarith_hash(u8 *data,u32 datalen,u8 *hashvalue,u8 type);

extern void pciarith_hash(u8 *data,u32 datalen,u8 *hashvalue,u8 type);
extern s32 dev_file_hash(u32 fileorgaddr,u32 filestaoffset,u32 inlen,u8 *hashvalue,u8 hashtype);
extern s32 pciarith_file_hash(u32 fileorgaddr,u32 filestaoffset,u32 inlen,u8 *hashvalue,u8 hashtype);
extern s32 bootdownload_fread_specifyaddr(u8 *data,s32 fileoffset,u32 length,u32 fileorgaddr);

#endif

