


#include "pciglobal.h"


#ifdef USESOFTHASHALTH


#define UNRAVEL

#define f1(x,y,z)   ((x & y) | (~x & z))
#define f2(x,y,z)   (x ^ y ^ z)
#define f3(x,y,z)   ((x & y) | (x & z) | (y & z))
#define f4(x,y,z)   (x ^ y ^ z)
#define CONST1      0x5a827999L
#define CONST2      0x6ed9eba1L
#define CONST3      0x8f1bbcdcL
#define CONST4      0xca62c1d6L

#define T32(x) ((x) & 0xffffffffL)
#define R32(x,n)    T32(((x << n) | (x >> (32 - n))))
#define FG(n)  \
    T = T32(R32(A,5) + f##n(B,C,D) + E + *WP++ + CONST##n); \
    E = D; D = C; C = R32(B,30); B = A; A = T
#define FA(n)  \
    T = T32(R32(A,5) + f##n(B,C,D) + E + *WP++ + CONST##n); B = R32(B,30)
#define FB(n)  \
    E = T32(R32(T,5) + f##n(A,B,C) + D + *WP++ + CONST##n); A = R32(A,30)
#define FC(n)  \
    D = T32(R32(E,5) + f##n(T,A,B) + C + *WP++ + CONST##n); T = R32(T,30)
#define FD(n)  \
    C = T32(R32(D,5) + f##n(E,T,A) + B + *WP++ + CONST##n); E = R32(E,30)
#define FE(n)  \
    B = T32(R32(C,5) + f##n(D,E,T) + A + *WP++ + CONST##n); D = R32(D,30)
#define FT(n)  \
    A = T32(R32(B,5) + f##n(C,D,E) + T + *WP++ + CONST##n); C = R32(C,30)
static void sha_transform(PCISHA_INFO *sha_info)
{
	int i;
	unsigned char *dp;
	SHALONG T, A, B, C, D, E, W[80], *WP;

	dp = sha_info->data;


#undef SWAP_DONE

#if (SHA_BYTE_ORDER == 1234)
#define SWAP_DONE
	for( i = 0; i < 16; ++i )
	{
		T = *((SHALONG *) dp);
		dp += 4;
		W[i] =  ((T << 24) & 0xff000000) | ((T <<  8) & 0x00ff0000) |
				((T >>  8) & 0x0000ff00) | ((T >> 24) & 0x000000ff);
	}
#endif 

#if (SHA_BYTE_ORDER == 4321)
#define SWAP_DONE
	for( i = 0; i < 16; ++i )
	{
		T = *((SHALONG *) dp);
		dp += 4;
		W[i] = T32(T);
	}
#endif 

#if (SHA_BYTE_ORDER == 12345678)
#define SWAP_DONE
	for( i = 0; i < 16; i += 2 )
	{
		T = *((SHALONG *) dp);
		dp += 8;
		W[i] =  ((T << 24) & 0xff000000) | ((T <<  8) & 0x00ff0000) |
				((T >>  8) & 0x0000ff00) | ((T >> 24) & 0x000000ff);
		T >>= 32;
		W[i+1] = ((T << 24) & 0xff000000) | ((T <<  8) & 0x00ff0000) |
				 ((T >>  8) & 0x0000ff00) | ((T >> 24) & 0x000000ff);
	}
#endif 

#if (SHA_BYTE_ORDER == 87654321)
#define SWAP_DONE
	for( i = 0; i < 16; i += 2 )
	{
		T = *((SHALONG *) dp);
		dp += 8;
		W[i] = T32(T >> 32);
		W[i+1] = T32(T);
	}
#endif 


	for( i = 16; i < 80; ++i )
	{
		W[i] = W[i-3] ^ W[i-8] ^ W[i-14] ^ W[i-16];
#if (SHA_VERSION == 1)
		W[i] = R32(W[i], 1);
#endif 
	}
	A = sha_info->digest[0];
	B = sha_info->digest[1];
	C = sha_info->digest[2];
	D = sha_info->digest[3];
	E = sha_info->digest[4];
	WP = W;
#ifdef UNRAVEL
	FA(1); FB(1); FC(1); FD(1); FE(1); FT(1); FA(1); FB(1); FC(1); FD(1);
	FE(1); FT(1); FA(1); FB(1); FC(1); FD(1); FE(1); FT(1); FA(1); FB(1);
	FC(2); FD(2); FE(2); FT(2); FA(2); FB(2); FC(2); FD(2); FE(2); FT(2);
	FA(2); FB(2); FC(2); FD(2); FE(2); FT(2); FA(2); FB(2); FC(2); FD(2);
	FE(3); FT(3); FA(3); FB(3); FC(3); FD(3); FE(3); FT(3); FA(3); FB(3);
	FC(3); FD(3); FE(3); FT(3); FA(3); FB(3); FC(3); FD(3); FE(3); FT(3);
	FA(4); FB(4); FC(4); FD(4); FE(4); FT(4); FA(4); FB(4); FC(4); FD(4);
	FE(4); FT(4); FA(4); FB(4); FC(4); FD(4); FE(4); FT(4); FA(4); FB(4);
	sha_info->digest[0] = T32(sha_info->digest[0] + E);
	sha_info->digest[1] = T32(sha_info->digest[1] + T);
	sha_info->digest[2] = T32(sha_info->digest[2] + A);
	sha_info->digest[3] = T32(sha_info->digest[3] + B);
	sha_info->digest[4] = T32(sha_info->digest[4] + C);
#else 
#ifdef UNROLL_LOOPS
	FG(1); FG(1); FG(1); FG(1); FG(1); FG(1); FG(1); FG(1); FG(1); FG(1);
	FG(1); FG(1); FG(1); FG(1); FG(1); FG(1); FG(1); FG(1); FG(1); FG(1);
	FG(2); FG(2); FG(2); FG(2); FG(2); FG(2); FG(2); FG(2); FG(2); FG(2);
	FG(2); FG(2); FG(2); FG(2); FG(2); FG(2); FG(2); FG(2); FG(2); FG(2);
	FG(3); FG(3); FG(3); FG(3); FG(3); FG(3); FG(3); FG(3); FG(3); FG(3);
	FG(3); FG(3); FG(3); FG(3); FG(3); FG(3); FG(3); FG(3); FG(3); FG(3);
	FG(4); FG(4); FG(4); FG(4); FG(4); FG(4); FG(4); FG(4); FG(4); FG(4);
	FG(4); FG(4); FG(4); FG(4); FG(4); FG(4); FG(4); FG(4); FG(4); FG(4);
#else 
	for( i =  0; i < 20; ++i )
	{
		FG(1);
	}
	for( i = 20; i < 40; ++i )
	{
		FG(2);
	}
	for( i = 40; i < 60; ++i )
	{
		FG(3);
	}
	for( i = 60; i < 80; ++i )
	{
		FG(4);
	}
#endif
	sha_info->digest[0] = T32(sha_info->digest[0] + A);
	sha_info->digest[1] = T32(sha_info->digest[1] + B);
	sha_info->digest[2] = T32(sha_info->digest[2] + C);
	sha_info->digest[3] = T32(sha_info->digest[3] + D);
	sha_info->digest[4] = T32(sha_info->digest[4] + E);
#endif 
}

void pcisha_init(PCISHA_INFO *sha_info)
{
	sha_info->digest[0] = 0x67452301L;
	sha_info->digest[1] = 0xefcdab89L;
	sha_info->digest[2] = 0x98badcfeL;
	sha_info->digest[3] = 0x10325476L;
	sha_info->digest[4] = 0xc3d2e1f0L;
	sha_info->count_lo = 0L;
	sha_info->count_hi = 0L;
	sha_info->local = 0;
}


void PCIsha_update(PCISHA_INFO *sha_info, unsigned char *buffer, int count)
{
	int i;
	SHALONG clo;

	clo = T32(sha_info->count_lo + ((SHALONG) count << 3));
	if( clo < sha_info->count_lo )
	{
		++sha_info->count_hi;
	}
	sha_info->count_lo = clo;
	sha_info->count_hi += (SHALONG) count >> 29;
	if( sha_info->local )
	{
		i = SHA_BLOCKSIZE - sha_info->local;
		if( i > count )
		{
			i = count;
		}
		memcpy(((unsigned char *) sha_info->data) + sha_info->local, buffer, i);
		count -= i;
		buffer += i;
		sha_info->local += i;
		if( sha_info->local == SHA_BLOCKSIZE )
		{
			sha_transform(sha_info);
		}
		else
		{
			return;
		}
	}
	while( count >= SHA_BLOCKSIZE )
	{
		memcpy(sha_info->data, buffer, SHA_BLOCKSIZE);
		buffer += SHA_BLOCKSIZE;
		count -= SHA_BLOCKSIZE;
		sha_transform(sha_info);
	}
	memcpy(sha_info->data, buffer, count);
	sha_info->local = count;
}



void pcisha_final(unsigned char digest[20], PCISHA_INFO *sha_info)
{
	int count;
	SHALONG lo_bit_count, hi_bit_count;

	lo_bit_count = sha_info->count_lo;
	hi_bit_count = sha_info->count_hi;
	count = (int) ((lo_bit_count >> 3) & 0x3f);
	((unsigned char *) sha_info->data)[count++] = 0x80;
	if( count > SHA_BLOCKSIZE - 8 )
	{
		memset(((unsigned char *) sha_info->data) + count, 0, SHA_BLOCKSIZE - count);
		sha_transform(sha_info);
		memset((unsigned char *) sha_info->data, 0, SHA_BLOCKSIZE - 8);
	}
	else
	{
		memset(((unsigned char *) sha_info->data) + count, 0,
			   SHA_BLOCKSIZE - 8 - count);
	}
	sha_info->data[56] = (hi_bit_count >> 24) & 0xff;
	sha_info->data[57] = (hi_bit_count >> 16) & 0xff;
	sha_info->data[58] = (hi_bit_count >>  8) & 0xff;
	sha_info->data[59] = (hi_bit_count >>  0) & 0xff;
	sha_info->data[60] = (lo_bit_count >> 24) & 0xff;
	sha_info->data[61] = (lo_bit_count >> 16) & 0xff;
	sha_info->data[62] = (lo_bit_count >>  8) & 0xff;
	sha_info->data[63] = (lo_bit_count >>  0) & 0xff;
	sha_transform(sha_info);
	digest[ 0] = (unsigned char) ((sha_info->digest[0] >> 24) & 0xff);
	digest[ 1] = (unsigned char) ((sha_info->digest[0] >> 16) & 0xff);
	digest[ 2] = (unsigned char) ((sha_info->digest[0] >>  8) & 0xff);
	digest[ 3] = (unsigned char) ((sha_info->digest[0]      ) & 0xff);
	digest[ 4] = (unsigned char) ((sha_info->digest[1] >> 24) & 0xff);
	digest[ 5] = (unsigned char) ((sha_info->digest[1] >> 16) & 0xff);
	digest[ 6] = (unsigned char) ((sha_info->digest[1] >>  8) & 0xff);
	digest[ 7] = (unsigned char) ((sha_info->digest[1]      ) & 0xff);
	digest[ 8] = (unsigned char) ((sha_info->digest[2] >> 24) & 0xff);
	digest[ 9] = (unsigned char) ((sha_info->digest[2] >> 16) & 0xff);
	digest[10] = (unsigned char) ((sha_info->digest[2] >>  8) & 0xff);
	digest[11] = (unsigned char) ((sha_info->digest[2]      ) & 0xff);
	digest[12] = (unsigned char) ((sha_info->digest[3] >> 24) & 0xff);
	digest[13] = (unsigned char) ((sha_info->digest[3] >> 16) & 0xff);
	digest[14] = (unsigned char) ((sha_info->digest[3] >>  8) & 0xff);
	digest[15] = (unsigned char) ((sha_info->digest[3]      ) & 0xff);
	digest[16] = (unsigned char) ((sha_info->digest[4] >> 24) & 0xff);
	digest[17] = (unsigned char) ((sha_info->digest[4] >> 16) & 0xff);
	digest[18] = (unsigned char) ((sha_info->digest[4] >>  8) & 0xff);
	digest[19] = (unsigned char) ((sha_info->digest[4]      ) & 0xff);
}









#define DBL_INT_ADD(a,b,c) if (a > 0xffffffff - (c)) ++b; a += c;
#define ROTLEFT(a,b) (((a) << (b)) | ((a) >> (32-(b))))
#define ROTRIGHT(a,b) (((a) >> (b)) | ((a) << (32-(b))))

#define CH(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTRIGHT(x,2) ^ ROTRIGHT(x,13) ^ ROTRIGHT(x,22))
#define EP1(x) (ROTRIGHT(x,6) ^ ROTRIGHT(x,11) ^ ROTRIGHT(x,25))
#define SIG0(x) (ROTRIGHT(x,7) ^ ROTRIGHT(x,18) ^ ((x) >> 3))
#define SIG1(x) (ROTRIGHT(x,17) ^ ROTRIGHT(x,19) ^ ((x) >> 10))

const unsigned int k[64] = {
   0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
   0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
   0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
   0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
   0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
   0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
   0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
   0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};


void sha256_init(SHA256_CTX *ctx)
{  
   ctx->datalen = 0; 
   ctx->bitlen[0] = 0; 
   ctx->bitlen[1] = 0; 
   ctx->state[0] = 0x6a09e667;
   ctx->state[1] = 0xbb67ae85;
   ctx->state[2] = 0x3c6ef372;
   ctx->state[3] = 0xa54ff53a;
   ctx->state[4] = 0x510e527f;
   ctx->state[5] = 0x9b05688c;
   ctx->state[6] = 0x1f83d9ab;
   ctx->state[7] = 0x5be0cd19;
}


void sha256_transform(SHA256_CTX *ctx, unsigned char *data)
{  
   unsigned int a,b,c,d,e,f,g,h,i,j,t1,t2,m[64];
      
   for (i=0,j=0; i < 16; ++i, j += 4)
      m[i] = (data[j] << 24) | (data[j+1] << 16) | (data[j+2] << 8) | (data[j+3]);
   for ( ; i < 64; ++i)
      m[i] = SIG1(m[i-2]) + m[i-7] + SIG0(m[i-15]) + m[i-16];

   a = ctx->state[0];
   b = ctx->state[1];
   c = ctx->state[2];
   d = ctx->state[3];
   e = ctx->state[4];
   f = ctx->state[5];
   g = ctx->state[6];
   h = ctx->state[7];
   
   for (i = 0; i < 64; ++i) {
      t1 = h + EP1(e) + CH(e,f,g) + k[i] + m[i];
      t2 = EP0(a) + MAJ(a,b,c);
      h = g;
      g = f;
      f = e;
      e = d + t1;
      d = c;
      c = b;
      b = a;
      a = t1 + t2;
   }
   
   ctx->state[0] += a;
   ctx->state[1] += b;
   ctx->state[2] += c;
   ctx->state[3] += d;
   ctx->state[4] += e;
   ctx->state[5] += f;
   ctx->state[6] += g;
   ctx->state[7] += h;
} 



void softsha256_update(SHA256_CTX *ctx, unsigned char *data, unsigned int len)
{  
   unsigned int i;
   
   for (i=0; i < len; ++i) { 
      ctx->data[ctx->datalen] = data[i]; 
      ctx->datalen++; 
      if (ctx->datalen == 64) { 
         sha256_transform(ctx,ctx->data);
         DBL_INT_ADD(ctx->bitlen[0],ctx->bitlen[1],512); 
         ctx->datalen = 0; 
      }  
   }  
}


void softsha256_final(SHA256_CTX *ctx, unsigned char *hash)
{  
   unsigned int i; 
   
   i = ctx->datalen; 
   
   // Pad whatever data is left in the buffer. 
   if (ctx->datalen < 56) { 
      ctx->data[i++] = 0x80; 
      while (i < 56) 
         ctx->data[i++] = 0x00; 
   }  
   else { 
      ctx->data[i++] = 0x80; 
      while (i < 64) 
         ctx->data[i++] = 0x00; 
      sha256_transform(ctx,ctx->data);
      memset(ctx->data,0,56); 
   }  
   
   // Append to the padding the total message's length in bits and transform. 
   DBL_INT_ADD(ctx->bitlen[0],ctx->bitlen[1],ctx->datalen * 8);
   ctx->data[63] = ctx->bitlen[0]; 
   ctx->data[62] = ctx->bitlen[0] >> 8; 
   ctx->data[61] = ctx->bitlen[0] >> 16; 
   ctx->data[60] = ctx->bitlen[0] >> 24; 
   ctx->data[59] = ctx->bitlen[1]; 
   ctx->data[58] = ctx->bitlen[1] >> 8; 
   ctx->data[57] = ctx->bitlen[1] >> 16;  
   ctx->data[56] = ctx->bitlen[1] >> 24; 
   sha256_transform(ctx,ctx->data);
   
   // Since this implementation uses little endian byte ordering and SHA uses big endian,
   // reverse all the bytes when copying the final state to the output hash. 
   for (i=0; i < 4; ++i) { 
      hash[i]    = (ctx->state[0] >> (24-i*8)) & 0x000000ff; 
      hash[i+4]  = (ctx->state[1] >> (24-i*8)) & 0x000000ff; 
      hash[i+8]  = (ctx->state[2] >> (24-i*8)) & 0x000000ff;
      hash[i+12] = (ctx->state[3] >> (24-i*8)) & 0x000000ff;
      hash[i+16] = (ctx->state[4] >> (24-i*8)) & 0x000000ff;
      hash[i+20] = (ctx->state[5] >> (24-i*8)) & 0x000000ff;
      hash[i+24] = (ctx->state[6] >> (24-i*8)) & 0x000000ff;
      hash[i+28] = (ctx->state[7] >> (24-i*8)) & 0x000000ff;
   }  
} 


//HASH  256
void pciarith_hash(unsigned char * DataIn,unsigned int DataInLen,unsigned char* DataOut,u8 type)
{
    /*
	unsigned int i,k,m;
	PCISHA_INFO sha_info;


	pcisha_init(&sha_info);
	k=DataInLen/SHA_BLOCKSIZE;
	m=DataInLen%SHA_BLOCKSIZE;
	for( i=0;i<k;i++ )
	{
		PCIsha_update(&sha_info, (unsigned char*)&DataIn[i*SHA_BLOCKSIZE], SHA_BLOCKSIZE);
	}
	if( m!=0 )
		PCIsha_update(&sha_info, (unsigned char*)&DataIn[i*SHA_BLOCKSIZE], m);

	pcisha_final(DataOut, &sha_info);
	*/
	unsigned int i,k,m;
	SHA256_CTX ctx;
	PCISHA_INFO sha_info;


	if(type == HASHTYPE_256)
	{
		sha256_init(&ctx);
		
		k=DataInLen/SHA_BLOCKSIZE;
		m=DataInLen%SHA_BLOCKSIZE;
		for( i=0;i<k;i++ )
		{
			softsha256_update(&ctx, (unsigned char*)&DataIn[i*SHA_BLOCKSIZE], SHA_BLOCKSIZE);
		}
		if( m!=0 )
			softsha256_update(&ctx, (unsigned char*)&DataIn[i*SHA_BLOCKSIZE], m);

		softsha256_final(&ctx, DataOut);
	}
	else
	{
		pcisha_init(&sha_info);
		k=DataInLen/SHA_BLOCKSIZE;
		m=DataInLen%SHA_BLOCKSIZE;
		for( i=0;i<k;i++ )
		{
			PCIsha_update(&sha_info, (unsigned char*)&DataIn[i*SHA_BLOCKSIZE], SHA_BLOCKSIZE);
		}
		if( m!=0 )
			PCIsha_update(&sha_info, (unsigned char*)&DataIn[i*SHA_BLOCKSIZE], m);

		pcisha_final(DataOut, &sha_info);
	}
    
	
} 


#else


void pciarith_hash(unsigned char * DataIn,unsigned int DataInLen,unsigned char* DataOut,u8 type)
{
    pcicorecallfunc.pciarith_hash(DataIn,DataInLen,DataOut,type);
}

#endif



s32 pciarith_file_hash(u32 fileorgaddr,u32 filestaoffset,u32 inlen,u8 *hashvalue,u8 hashtype)
{
    #ifdef USESOFTHASHALTH
    u32 i,j,k,m; 
    PCISHA_INFO sha_info;
    SHA256_CTX ctx;
    u8 tmp[128]; 
    u32 pos;
    

    pos = filestaoffset;
	
    pcisha_init(&sha_info);
    sha256_init(&ctx);

    k=inlen/SHA_BLOCKSIZE;
    m=inlen%SHA_BLOCKSIZE;
    
    
    for( j=0;j<k;j++ )
    {
        
        i = bootdownload_fread_specifyaddr(tmp,pos,SHA_BLOCKSIZE,fileorgaddr);
        pos += i;
        if(i != SHA_BLOCKSIZE)
        {
            return -1;
        }

        if(hashtype == HASH256RSA2048)
        {
            softsha256_update(&ctx, &tmp[0], SHA_BLOCKSIZE);
        }
        else
        {
            PCIsha_update(&sha_info, &tmp[0], SHA_BLOCKSIZE);
        }
        
    }
    if( m!=0 )
    {
        i = bootdownload_fread_specifyaddr(tmp,pos,m,fileorgaddr);
        pos += i;
        if(i != m)
        {
            return -1;
        }

        if(hashtype == HASH256RSA2048)
        {
            softsha256_update(&ctx, &tmp[0], m);
        }
        else
        {
            PCIsha_update(&sha_info, &tmp[0], m);
        }
		
		
    }
    
    if(hashtype == HASH256RSA2048)
    {
        softsha256_final(&ctx,hashvalue);
    }
    else
    {
        pcisha_final(hashvalue, &sha_info);
    }
    
    return 0;

	#else
	pcicorecallfunc.file_hash(fileorgaddr,filestaoffset,inlen,hashvalue,hashtype);
	#endif
}

