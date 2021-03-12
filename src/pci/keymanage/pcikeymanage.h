

#ifndef _PCIKEYMANAGE_H_
#define _PCIKEYMANAGE_H_



/*PCI driver down key err */
#define KM_APP_KEY_WRITE_ERROR               1
#define KM_APP_KEY_GROUPINDEX_ERR            2
#define KM_APP_KEY_INDEX_ERROR               3
#define KM_APP_KEY_TYPE_ERROR                4  
#define KM_APP_KEY_LEN_ERROR                 5
#define KM_AUTH_ERROR                        6
#define KM_APPKEY_NOTAUTH                    7   
#define KM_PUBLICKKEY_UNFIND                 8
#define KM_APPKEY_LOCKED                     9
#define KM_RSACALCULATE_ERR                  10
#define KM_APPKEYSAME_ERR                    11
#define KM_ARITHMETIC_ERR                    12
#define KM_READKEY_ERR                       13
#define KM_READSESSIONK_ERR                  14
#define KM_GETPIN_INTERVALUNREACH            15
#define KM_ARITHMETICINDEX_ERR               16
#define KM_PININPUTCANCEL                    17
#define KM_PININPUTOVERTIME                  18
#define KM_PININPUTLENERR                    19
#define KM_PININPUTBYPASS                    20
#define KM_AUTHENKEYISSUER_ERROR             21
#define KM_KEYFILENOTUSE                     22
#define KM_OTHER_ERR                         30
    
#define KM_CREATE_TPFILE_ERROR               31    //add by 2011-07-31 
#define KM_WRITE_TPFILE_ERROR                32   
#define KM_MAGDATA_WAITSWIPE                 33
#define KM_MAGDATA_ERR                       34



/*app key definition*/
#define TYPE_TMK            0x04  //TMK
#define TYPE_PINK           0x05  //PIN key
#define TYPE_MACK           0x06  //MAC key
#define TYPE_TDK            0x07  //TDK key
#define TYPE_FIXEDK         0x08  //fixed key
#define TYPE_BPK            0x09  //authentication key of session key(PINK and MACK TDK)
#define TYPE_DUKPTK         0x0A
#define TYPE_KEYRESERVED    0x10

#define TYPE_NULL           0xff


typedef struct _PCIAPPKEY
{

	u8 keytype;
    u8 keyindex;
	u8 keylen;
	u8 keydata[24];
	u8 keylrc[5];
}PCIAPPKEY;
#define PCIAPPKEYSIZE   sizeof(PCIAPPKEY)



/*app key address offset and  max index NO.*/
#if 0
#define APPKEY_HEADOFFSET         0
#define APPKEY_HEADSIZE           0x100
#define TMK_ADDROFFSET            0x100   
#define TMK_MAXINDEX              32
#define PINK_ADDROFFSET           (0x400+APPKEY_HEADSIZE)
#define PINK_MAXINDEX             32
#define MACK_ADDROFFSET           (0x800+APPKEY_HEADSIZE)
#define MACK_MAXINDEX             32
#define TDK_ADDROFFSET           (0xC00+APPKEY_HEADSIZE)
#define TDK_MAXINDEX              32
#define FIXEDK_ADDROFFSET         (0x1000+APPKEY_HEADSIZE)
#define FIXEDK_MAXINDEX           32
#define BPK_ADDROFFSET           (0x1400+APPKEY_HEADSIZE)
#define BPK_MAXINDEX              32

#define KEYRESERVED_ADDROFFSET    (0x1800+APPKEY_HEADSIZE)
#define KEYRESERVED_MAXINDEX      64


#define APPKEY_LENGTH_MAX         (0x2000+APPKEY_HEADSIZE)
#define PCIPOS_APPKEYGROUP_MAXNUM  1  //只支持一组密钥
#else
#define APPKEY_HEADOFFSET         0
#define APPKEY_HEADSIZE           0x100
#define TMK_ADDROFFSET            0x100   //内部flash 512个字节一个块   
#define TMK_MAXINDEX              24
#define PINK_ADDROFFSET           (0x300+APPKEY_HEADSIZE)
#define PINK_MAXINDEX             16
#define MACK_ADDROFFSET           (0x500+APPKEY_HEADSIZE)
#define MACK_MAXINDEX             16
#define TDK_ADDROFFSET           (0x700+APPKEY_HEADSIZE)
#define TDK_MAXINDEX              16
#define FIXEDK_ADDROFFSET         (0x900+APPKEY_HEADSIZE)
#define FIXEDK_MAXINDEX           16
#define BPK_ADDROFFSET           (0xB00+APPKEY_HEADSIZE)
#define BPK_MAXINDEX              16

#define KEYRESERVED_ADDROFFSET    (0xD00+APPKEY_HEADSIZE)
#define KEYRESERVED_MAXINDEX      8


#define APPKEY_LENGTH_MAX         (0x1000)
#define PCIPOS_APPKEYGROUP_MAXNUM  1  //只支持一组密钥
#endif


#define DRV_PCI_APPKEY_PATH        "/mtd0/"   //"/mtd2/" 
#define DRV_PCI_APPKEY_SUFFIX      ".key"








extern s32 pcikeymanage_checkallappkey(u8 cmpkeygroupindex,PCIAPPKEY *cmpkey);
extern s32 pcikeymanage_readappkeyheadinfostatus(u8 groupindex,u8 *headinfo);
extern s32 pcikeymanage_groupkeyfile_initial(u8 groupindex);
extern s32 pcikeymanage_keyprocessusingMMK(u8 encryptmode,u8 *keydata,s32 keylen);
extern s32 pcikeymanage_getappkeyfileabsolutepath(u8 groupindex,s8 *absolutepath);
extern void pcikeymanage_initkeydata(void);

extern s32 pcikeymanage_readandcheckappkey(u8 groupindex,u8 keytype,u8 keyindex,u8 *keylen,u8 *keydata);
extern s32 pcikeymanage_saveappkey(u8 groupindex,u8 keytype,u8 keyindex,u8 keylen,u8 *keydata);
extern s32 pcikeymanage_deleteappkey(u8 groupindex,u8 keytype,u8 keyindex);


#endif


