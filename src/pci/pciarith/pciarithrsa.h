

#ifndef _PCIARITHRSA_H_
#define _PCIARITHRSA_H_



//#define RSASOFTUSED







#define PCI_APPINFO_CASN_LEN    48
#define PCI_APPINFO_FLAG_LEN    16
#define PCI_APPINFO_DATA_LEN    32    //24   
#define PCI_APPINFO_MAC_LEN     256   //128


#define PCIDRIVER_APPFILEINFO2048ADDLEN (16+256)


typedef struct _PCIDRIVER_APPFILEINFO{
	u8 casn[PCI_APPINFO_CASN_LEN];
    u8 flag[PCI_APPINFO_FLAG_LEN];
    u8 filebin_data[PCI_APPINFO_DATA_LEN];
    u8 filebin_mac[PCI_APPINFO_MAC_LEN];
    u8 fileelf_data[PCI_APPINFO_DATA_LEN];
    u8 fileelf_mac[PCI_APPINFO_MAC_LEN];
    u32 filebin_len;
    u32 fileelf_len;
	u32 hashdatalen;
	u32 macdatalen;
	u8 hashrsatype;
}PCIDRIVER_APPFILEINFO;
#define PCIAPPFILEINFOLEN (PCI_APPINFO_CASN_LEN+PCI_APPINFO_FLAG_LEN+PCI_APPINFO_DATA_LEN+PCI_APPINFO_DATA_LEN+PCI_APPINFO_MAC_LEN+PCI_APPINFO_MAC_LEN+8)


extern int dev_RSA_PKEncrypt(unsigned char *output, unsigned int *outputLen,unsigned char *input,  unsigned int inputLen,unsigned char *PK, unsigned int pkLen);
extern int dev_RSA_PKDecrypt(unsigned char * output, unsigned int *outputLen,unsigned char * input,  unsigned int inputLen,unsigned char * PK, unsigned int pkLen);

extern s32 pciarith_rsa_checkrsatype(u8 *IssuerSN);
extern s32 pciarith_rsa_pkencrypt(u8 *appname,u8 *IssuerSN,u8 *originaldata,u32 originaldatalen,u8 *encrypteddata,u32 *encrypteddatalen);
extern s32 pciarith_rsa_encryptalgorithm(u8 EncryptMode,u8 *PKdata,u8 *originaldata,u32 originaldatalen,u8 *encrypteddata,u32 *encrypteddatalen);

extern int RSA_PKEncrypt(unsigned char *output, unsigned int *outputLen,
                  unsigned char *input,  unsigned int inputLen,
                  unsigned char *PK, unsigned int pkLen);

extern int RSA_PKDecrypt(unsigned char * output, unsigned int *outputLen,
                  unsigned char * input,  unsigned int inputLen,
                  unsigned char * PK, unsigned int pkLen);

#endif


