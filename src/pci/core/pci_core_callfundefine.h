

#ifndef PCI_CORE_CALLFUNDEFINE_H_
#define PCI_CORE_CALLFUNDEFINE_H_


#define PCIINITIALEDFLAG         "pciinitialedflag"
#define PCITESTEDFLAG            "pciopentestflag1"



typedef void* (*pci_core_malloc)(u32 size);
typedef void (*pci_core_free)(void *block);
typedef s32 (*core_pci_dryice_writeprogramkey)(u8 *data,u32 datalen);
typedef s32 (*core_pci_dryice_readprogramkey)(u8 *data);
typedef s32 (*core_pci_get_random_bytes)(u8 *buf, u32 nbytes);
typedef s32 (*core_pci_kern_create_file)(const char *filename, s32 len,u8 initialvalue);
typedef s32 (*core_pci_kern_read_file)(const char *filename,u8 *rbuf, s32 rlen, s32 startaddr);
typedef s32 (*core_pci_kern_write_file)(const char *filename,u8 *writedata,s32 writedatalen);
typedef s32 (*core_pci_kern_insert_file)(const char *filename,u8 *wbuf, s32 wlen, s32 startaddr);
typedef s32 (*core_pci_kern_delete_file)(const char *filename);
typedef s32 (*core_pci_kern_rename_file)(const char *srcfilename,const char *dstfilename);
typedef s32 (*core_pcidrv_readCAK)(u8 *CAKdata);
typedef s32 (*core_pcidrv_processwithCAK)(u8 mode,u8 *data,u32 datalen);
typedef s32 (*core_pci_dev_timer_user_open)(u32 *timer);
typedef s32 (*core_pci_dev_timer_user_close)(u32 *timer);
typedef int (*core_pci_kern_access_file)(const s8 *filename);
typedef void (*core_pci_security_locked_hold)(u8 type);
typedef int (*core_SM3Encrypt)(unsigned char *pInput, unsigned int ilen, unsigned char *pOutput);


typedef void (*core_pciarith_desencrypt)(short encrypt,unsigned char *input,unsigned char *deskey);
typedef void (*core_pciarith_hash)(u8 *data,u32 datalen,u8 *hashvalue,u8 type);
typedef int (*core_RSA_PKEncrypt)(unsigned char *output, unsigned int *outputLen,unsigned char *input,  unsigned int inputLen,unsigned char *PK, unsigned int pkLen);
typedef int (*core_RSA_PKDecrypt)(unsigned char * output, unsigned int *outputLen,unsigned char * input,  unsigned int inputLen,unsigned char * PK, unsigned int pkLen);
typedef s32    (*core_file_hash)(u32 fileorgaddr,u32 filestaoffset,u32 inlen,u8 *hashvalue,u8 hashtype);
typedef int (*core_RSA_SKEncrypt)(unsigned char *output, unsigned int *outputLen,unsigned char *input,  unsigned int inputLen,unsigned char *SK, unsigned int skLen);
typedef int (*core_RSA_SKDecrypt)(unsigned char * output, unsigned int *outputLen,unsigned char * input,  unsigned int inputLen,unsigned char * SK, unsigned int skLen);


typedef struct{
    
    pci_core_malloc mem_malloc;
    pci_core_free   mem_free;
    core_pci_dryice_writeprogramkey pci_dryice_writeprogramkey;
    core_pci_dryice_readprogramkey pci_dryice_readprogramkey;
    core_pci_get_random_bytes pci_get_random_bytes;
    core_pci_kern_create_file pci_kern_create_file;
    core_pci_kern_read_file pci_kern_read_file;
    core_pci_kern_write_file pci_kern_write_file;
	core_pci_kern_insert_file pci_kern_insert_file;
	core_pci_kern_access_file pci_kern_access_file;
	core_pci_kern_delete_file pci_kern_delete_file;
	core_pci_kern_rename_file pci_kern_rename_file;
    core_pcidrv_readCAK pcidrv_readCAK;
	core_pcidrv_processwithCAK pcidrv_processusedCAK;
    core_pci_dev_timer_user_open pci_dev_timer_user_open;
	core_pci_dev_timer_user_close pci_dev_timer_user_close;
    core_pci_security_locked_hold pci_security_locked_hold;
	
    core_pciarith_desencrypt pciarith_desencrypt;
	core_pciarith_hash pciarith_hash;
	core_RSA_PKEncrypt RSA_PKEncrypt;
	core_RSA_PKDecrypt RSA_PKDecrypt;
	core_SM3Encrypt    SM3Encrypt;
	core_file_hash     file_hash;
	core_RSA_SKEncrypt RSA_SKEncrypt;
	core_RSA_SKDecrypt RSA_SKDecrypt;
	
}PCI_CORE_CALLFUNC;


extern const PCI_CORE_CALLFUNC pcicorecallfunc;


extern void pci_core_callfunc_set(void);
extern s32 temp_pcidrv_readCAK(u8 *CAKdata);
extern s32 temp_pcidrv_processwithCAK(u8 mode,u8 *data,u32 datalen);
extern s32 pci_core_saveinitialflag(u8 dryicetype);
extern s32 pci_core_checkinitialflag(void);

#endif


