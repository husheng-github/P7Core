

#include "devglobal.h"
#include "sdk/mhscpu_sdk.h"
#include "mh_sha.h"


void devarith_hash(u8 *data,u32 datalen,u8 *hashvalue,u8 type)
{
    
    if(type == HASHTYPE_256)
    {
        mh_sha(SHA_256, hashvalue, data, datalen);
    }
    else
    {
        mh_sha(SHA_160, hashvalue, data, datalen);
    }
    
}


s32 hash_fread_specifyaddr(u8 *data,s32 fileoffset,u32 length,u32 fileorgaddr)
{
    u32 addr;

    addr = fileorgaddr;
    addr += fileoffset;


    return dev_flash_read(addr,data,length);

}


s32 dev_file_hash(u32 fileorgaddr,u32 filestaoffset,u32 inlen,u8 *hashvalue,u8 hashtype)
{
     u32 i,j,k,m; 
    sha1_context sha_info;
    sha2_context ctx;
    u8 tmp[128]; 
    u32 pos;
    

    pos = filestaoffset;
    
    
    if(hashtype == 0)
    {
        mh_sha256_starts(&ctx);
    }
    else
    {
        mh_sha1_starts(&sha_info);
    }

    k=inlen/SHA_BLOCKSIZE;
    m=inlen%SHA_BLOCKSIZE;
    
    
    for( j=0;j<k;j++ )
    {
        
        i = hash_fread_specifyaddr(tmp,pos,SHA_BLOCKSIZE,fileorgaddr);
        pos += i;
        if(i != SHA_BLOCKSIZE)
        {
            return -1;
        }

        if(hashtype == 0)
        {
            mh_sha256_update(&ctx, &tmp[0], SHA_BLOCKSIZE);
        }
        else
        {
            mh_sha1_update(&sha_info, &tmp[0], SHA_BLOCKSIZE);
        }
        
    }
    if( m!=0 )
    {
        i = hash_fread_specifyaddr(tmp,pos,m,fileorgaddr);
        pos += i;
        if(i != m)
        {
            return -1;
        }

        if(hashtype == 0)
        {
            mh_sha256_update(&ctx, &tmp[0], m);
        }
        else
        {
            mh_sha1_update(&sha_info, &tmp[0], m);
        }
        
        
    }
    
    if(hashtype == 0)
    {
        mh_sha256_finish(&ctx,tmp);
        memcpy(hashvalue,tmp,32);
    }
    else
    {
        mh_sha1_finish(&sha_info,tmp);
        memcpy(hashvalue,tmp,20);
    }
    
    return 0;
    
}


#if 0
extern s32 bootdownload_fread_specifyaddr(u8 *data,s32 fileoffset,u32 length,u32 fileorgaddr);
s32 dev_file_hash(u32 fileorgaddr,u32 filestaoffset,u32 inlen,u8 *hashvalue,u8 hashtype)
{
    u32 i,j,k,m; 
    sha2_context info256;
    sha1_context infosha1;
    u8 tmp[128]; 
    u32 pos;
    
   
   
    pos = filestaoffset;

    if(hashtype == 0)
    {
        sha256_starts(&info256);
    }
    else
    {
        sha1_starts(&infosha1);
    }


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

        //seclib_printf("\r\npos = %08x \r\n",pos);  //sxl?

        if(hashtype == 0)
        {
            sha256_update(&info256, (const u8*)&tmp[0], SHA_BLOCKSIZE );
        }
        else
        {
            sha1_update(&infosha1, (const u8*)&tmp[0], SHA_BLOCKSIZE );
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

        if(hashtype == 0)
        {
            sha256_update(&info256, (const u8*)&tmp[0], m );
        }
        else
        {
            sha1_update(&infosha1, (const u8*)&tmp[0], m );
        }
        
        
    }
    
    if(hashtype == 0)
    {
        sha256_finish( &info256,tmp);
        memcpy(hashvalue,tmp,32);
    }
    else
    {
        sha1_finish( &infosha1,tmp);
        memcpy(hashvalue,tmp,20);
    }
    
    return 0;
    
}

#endif


void dev_mh_read_ver(u32 *ver)
{
    *ver = mh_crypt_version();
    dev_debug_printf("mh_version=%08X\r\n", *ver);    
}

