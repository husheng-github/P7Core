
#include "pciglobal.h"


void pciarith_DesEncrypt(short encrypt,unsigned char *input,unsigned char *deskey)
{
 	//sxl? ∫Û–¯ÕÍ…∆
 	pcicorecallfunc.pciarith_desencrypt(encrypt,input,deskey);
}



//20140327
void pciarith_mathlock(void)
{
}

//20140327
void pciarith_mathunlock(void)
{
}


//20140327
int drv_tdes(short encrypt, unsigned char *datat, unsigned int datalen, unsigned char *keyt, unsigned int keylen)
{
    int i;
    int encryptmode1, encryptmode2;

    pciarith_mathlock();
    if(0 != (datalen&0x07))
    {
        pciarith_mathunlock();
        return(-1);
    }
    if(0 == encrypt)
    {
        encryptmode1 = 0;
        encryptmode2 = 1;
    }
    else
    {
        encryptmode1 = 1;
        encryptmode2 = 0;
    }
    if(DES_KEY_LEN8 == keylen)
    {
        for(i=0; i<datalen; i+=8)
        {
            pciarith_DesEncrypt(encryptmode1, &datat[i], keyt);
        }   
    }
    else if(DES_KEY_LEN16 == keylen)
    {
        for(i=0; i<datalen; i+=8)
        {
            pciarith_DesEncrypt(encryptmode1, &datat[i], &keyt[0]);
            pciarith_DesEncrypt(encryptmode2, &datat[i], &keyt[8]);
            pciarith_DesEncrypt(encryptmode1, &datat[i], &keyt[0]);
        }  
    }
    else if(DES_KEY_LEN24 == keylen)
    {
        if(0 == encryptmode1)   //decrypt
        {
            for(i=0; i<datalen; i+=8)
            {
                pciarith_DesEncrypt(encryptmode1, &datat[i], &keyt[16]);
                pciarith_DesEncrypt(encryptmode2, &datat[i], &keyt[8]);
                pciarith_DesEncrypt(encryptmode1, &datat[i], &keyt[0]);
            }    
        }
        else                    //encrypt
        {
            
            for(i=0; i<datalen; i+=8)
            {
                pciarith_DesEncrypt(encryptmode1, &datat[i], &keyt[0]);
                pciarith_DesEncrypt(encryptmode2, &datat[i], &keyt[8]);
                pciarith_DesEncrypt(encryptmode1, &datat[i], &keyt[16]);
            }    
        }
    }
    else
    {
        pciarith_mathlock();
        return(-1);
    }
    pciarith_mathunlock();
    return 0;
	
}


void Lib_DES(unsigned char *dat,unsigned char *key,int mode)
{
	pciarith_DesEncrypt(mode,dat,key);
}



////20140327
int DES_TDES(unsigned char *key,unsigned char keylen,unsigned char *dat,unsigned char datalen,unsigned char mode)
{
    unsigned char chmode;
    chmode=((keylen/8)<<4)+(datalen/8);
    if(mode!=0x01 && mode!=0x81 && mode!=0) 
        return PCI_KEYMODE_ERR;
    switch(chmode)
    {
        case 0x11:
            if(mode==0x01)
                Lib_DES(dat,key,1);
            else
                Lib_DES(dat,key,0);
            break;
        case 0x21:
            if(mode==0x01)
            {
                Lib_DES(dat,key,1);
                Lib_DES(dat,&key[8],0);
                Lib_DES(dat,key,1);
            }
            else
            {
                Lib_DES(dat,key,0);
                Lib_DES(dat,&key[8],1);
                Lib_DES(dat,key,0);
            }
            break;
        case 0x31:
            if(mode==0x01)
            {
                Lib_DES(dat,key,1);
                Lib_DES(dat,&key[8],0);
                Lib_DES(dat,&key[16],1);
            }
            else
            {
                Lib_DES(dat,&key[16],0);
                Lib_DES(dat,&key[8],1);
                Lib_DES(dat,key,0);
            }
            break;
        case 0x22:
            if(mode==0x01)
            {
                Lib_DES(dat,key,1);
                Lib_DES(dat,&key[8],0);
                Lib_DES(dat,key,1);
                Lib_DES(&dat[8],key,1);
                Lib_DES(&dat[8],&key[8],0);
                Lib_DES(&dat[8],key,1);
            }
            else
            {
                Lib_DES(dat,key,0);
                Lib_DES(dat,&key[8],1);
                Lib_DES(dat,key,0);
                Lib_DES(&dat[8],key,0);
                Lib_DES(&dat[8],&key[8],1);
                Lib_DES(&dat[8],key,0);
            }
            break;
        case 0x32:
            if(mode==0x01)
            {
                Lib_DES(dat,key,1);
                Lib_DES(dat,&key[8],0);
                Lib_DES(dat,&key[16],1);
                Lib_DES(&dat[8],key,1);
                Lib_DES(&dat[8],&key[8],0);
                Lib_DES(&dat[8],&key[16],1);
            }
            else
            {
                Lib_DES(dat,&key[16],0);
                Lib_DES(dat,&key[8],1);
                Lib_DES(dat,key,0);
                Lib_DES(&dat[8],&key[16],0);
                Lib_DES(&dat[8],&key[8],1);
                Lib_DES(&dat[8],key,0);
            }
            break;
        case 0x33:
            if(mode==0x01)
            {
                Lib_DES(dat,key,1);
                Lib_DES(dat,&key[8],0);
                Lib_DES(dat,&key[16],1);
                Lib_DES(&dat[8],key,1);
                Lib_DES(&dat[8],&key[8],0);
                Lib_DES(&dat[8],&key[16],1);
                Lib_DES(&dat[16],key,1);
                Lib_DES(&dat[16],&key[8],0);
                Lib_DES(&dat[16],&key[16],1);
            }
            else
            {
                Lib_DES(dat,&key[16],0);
                Lib_DES(dat,&key[8],1);
                Lib_DES(dat,key,0);
                Lib_DES(&dat[8],&key[16],0);
                Lib_DES(&dat[8],&key[8],1);
                Lib_DES(&dat[8],key,0);
                Lib_DES(&dat[16],&key[16],0);
                Lib_DES(&dat[16],&key[8],1);
                Lib_DES(&dat[16],key,0);
            }
            break;
        default : 
            return PCI_KEYMODE_ERR;
    }
    return 0;
}


void Lib_Des(unsigned char *input,unsigned char *output,unsigned char *deskey,int mode)
{
    #ifdef SXL_DEBUG
    //DDEBUG("\r\nLib_Des1\r\n");
    #endif
    memcpy(output,input,8);
    //drv_DesEncrypt(mode,output,deskey);  //sxl?

    #ifdef SXL_DEBUG
    //DDEBUG("\r\nLib_Des2\r\n");
    #endif
}


void Lib_Des24(unsigned char *input,unsigned char *output,unsigned char *deskey,int mode)
{
    unsigned char strTemp[8];

    if(mode)
    {
        Lib_Des(input, output, deskey, 1);
        Lib_Des(output, strTemp, &deskey[8], 0);
        Lib_Des(strTemp, output, &deskey[16], 1);
    }
    else
    {
        Lib_Des(input, output, &deskey[16], 0);
        Lib_Des(output, strTemp, &deskey[8], 1);
        Lib_Des(strTemp, output, deskey, 0);
    }
}



void Lib_Des24_DataProcessing(unsigned char *input,unsigned char *output,int datalen,unsigned char *deskey,int mode)
{
    int i,j,k;
    
    k = datalen>>3;
    
    j = 0;
    for(i = 0;i<k;i++)
    {
        Lib_Des24(&input[j],&output[j],deskey,mode);
        j += 8;
    }
}

//20140327
void DesMac(unsigned char *key,unsigned char *mdat,unsigned short length)
{
    unsigned short i,j,l;
    l=length/8;
    i =length%8;
    if(i)
    {
        memset(&mdat[length],0x00,8-i);
        l ++;
    }
    Lib_DES(mdat,key,1);
    for(i=1;i<l;i++)
    {
        for(j=0;j<8;j++)  mdat[j] ^= mdat[j+8*i];
        Lib_DES(mdat,key,1);
    }
}
////20140327
void Lib_DES3_16(unsigned char *dat,unsigned char *key,int mode)
{
    if(mode)
    {
        Lib_DES(dat,key,1);
        Lib_DES(dat,&key[8],0);
        Lib_DES(dat,key,1);
    }
    else
    {
        Lib_DES(dat,key,0);
        Lib_DES(dat,&key[8],1);
        Lib_DES(dat,key,0);
    }
}
////20140327
void Des3_16Mac(unsigned char *key,unsigned char *mdat,unsigned short length)
{
    unsigned short i,j,l;
    l=length/8;
    i =length%8;
    if(i)
    {
        memset(&mdat[length],0x00,8-i);
        l ++;
    }
    Lib_DES3_16(mdat,key,1);
    for(i=1;i<l;i++)
    {
        for(j=0;j<8;j++)   mdat[j] ^= mdat[j+8*i];
        Lib_DES3_16(mdat,key,1);
    }
}

////20140327
void Lib_DES3_24(unsigned char *dat,unsigned char *key,int mode)
{
    if(mode)
    {
        Lib_DES(dat,key,1);
        Lib_DES(dat,&key[8],0);
        Lib_DES(dat,&key[16],1);
    }
    else
    {
        Lib_DES(dat,&key[16],0);
        Lib_DES(dat,&key[8],1);
        Lib_DES(dat,key,0);
    }

}

////20140327
void Des3_24Mac(unsigned char *key,unsigned char *mdat,unsigned short length)
{
    unsigned short i,j,l;
    l=length/8;
    i =length%8;
    if(i)
    {
        memset(&mdat[length],0x00,8-i);
        l ++;
    }
    Lib_DES3_24(mdat,key,1);
    for(i=1;i<l;i++)
    {
        for(j=0;j<8;j++)        
            mdat[j] ^= mdat[j+8*i];
        Lib_DES3_24(mdat,key,1);
    }
}


void DesMacWithTCBC(u8 *data,u8 datalen,u8 *keydata,u8 keylen,u8 *mac)
{
    u8 tmpdata[256];
    
    memset(tmpdata,0,sizeof(tmpdata));
    memcpy(tmpdata,data,datalen);
    
    switch(keylen/8)
    {
        case 0x01:
            DesMac(keydata,tmpdata,datalen);
            break;
        case 0x02:
            Des3_16Mac(keydata,tmpdata,datalen);
            break;
        case 0x03:
            Des3_24Mac(keydata,tmpdata,datalen);
            break;
        default:
            break;
    }
    
    memcpy(mac,tmpdata,8);
}

////20140327
void Des3_16Mac_2(unsigned char *key,unsigned char *mdat,unsigned short length)
{
    unsigned short i,j,l;
    l=length/8;
    i =length%8;
    if(i)
    {
        memset(&mdat[length],0x00,8-i);
        l ++;
    }
    if(l<=1)
    {
        Lib_DES3_16(mdat,key,1);
        return;
    }

    Lib_DES(mdat,key,1);
    for(i=1;i<l-1;i++)
    {
        for(j=0;j<8;j++)   
            mdat[j] ^= mdat[j+8*i];
        Lib_DES(mdat,key,1);
    }
    for(j=0;j<8;j++)   
        mdat[j] ^= mdat[j+8*i];
    Lib_DES3_16(mdat,key,1);

}
////20140327
void Des3_24Mac_2(unsigned char *key,unsigned char *mdat,unsigned short length)
{
    unsigned short i,j,l;
    l=length/8;
    i =length%8;
    if(i)
    {
        memset(&mdat[length],0x00,8-i);
        l ++;
    }

    if(l<=1)
    {
        Lib_DES3_24(mdat,key,1);
        return;
    }

    Lib_DES(mdat,key,1);
    for(i=1;i<l-1;i++)
    {
        for(j=0;j<8;j++)   
            mdat[j] ^= mdat[j+8*i];
        Lib_DES(mdat,key,1);
    }
    for(j=0;j<8;j++)   
        mdat[j] ^= mdat[j+8*i];
    Lib_DES3_24(mdat,key,1);
}

