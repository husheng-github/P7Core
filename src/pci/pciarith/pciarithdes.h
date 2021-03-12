

#ifndef _PCIARITHDES_H_
#define _PCIARITHDES_H_


#define DES_KEY_LEN8            8
#define DES_KEY_LEN16           16
#define DES_KEY_LEN24           24




extern void devarith_desencrypt(short encrypt,unsigned char *input,unsigned char *deskey);


extern int drv_tdes(short encrypt, unsigned char *datat, unsigned int datalen, unsigned char *keyt, unsigned int keylen);
extern int DES_TDES(unsigned char *key,unsigned char keylen,unsigned char *dat,unsigned char datalen,unsigned char mode);
extern void Lib_Des(unsigned char *input,unsigned char *output,unsigned char *deskey,int mode);
extern void Lib_Des24(unsigned char *input,unsigned char *output,unsigned char *deskey,int mode);
extern void Lib_Des24_DataProcessing(unsigned char *input,unsigned char *output,int datalen,unsigned char *deskey,int mode);
extern void DesMac(unsigned char *key,unsigned char *mdat,unsigned short length);	
extern void Lib_DES3_16(unsigned char *dat,unsigned char *key,int mode);
extern void Des3_16Mac(unsigned char *key,unsigned char *mdat,unsigned short length);
extern void Lib_DES3_24(unsigned char *dat,unsigned char *key,int mode);
extern void Des3_24Mac(unsigned char *key,unsigned char *mdat,unsigned short length);
extern void DesMacWithTCBC(u8 *data,u8 datalen,u8 *keydata,u8 keylen,u8 *mac);
extern void Des3_16Mac_2(unsigned char *key,unsigned char *mdat,unsigned short length);
extern void Des3_24Mac_2(unsigned char *key,unsigned char *mdat,unsigned short length);


#endif


