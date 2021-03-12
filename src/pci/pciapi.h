

#ifndef PCI_API_H_
#define PCI_API_H_


#include "ddi_common.h"
#include "ddi_dukpt.h"
typedef void (*pci_firmware_api_open)(void);
typedef s32     (*pci_firmware_api_deleteppkey)(u8 groupindex,u8 keytype,u8 keyindex);
typedef s32     (*pci_firmware_api_readandcheckappkey)(u8 groupindex,u8 keytype,u8 keyindex,u8 *keylen,u8 *keydata);
typedef s32     (*pci_firmware_api_saveappkey)(u8 groupindex,u8 keytype,u8 keyindex,u8 keylen,u8 *keydata);
typedef s32 (*pci_firmware_api_dryice_isrhandler)(void);
typedef s32     (*pci_firmware_api_dukptk_load)(strDukptInitInfo* lpstrDukptInitInfo);
typedef s32     (*pci_firmware_api_dukpt_check_keytmp)(u32 nKeyGroup, u32 nKsnIndex,strDevDukpt *lpstrDevDukpt);

typedef void (*pci_firmware_api_devarith_desencrypt)(short encrypt,unsigned char *input,unsigned char *deskey);
typedef void (*pci_firmware_api_devarith_hash)(u8 *data,u32 datalen,u8 *hashvalue,u8 type);
typedef int   (*pci_firmware_api_dev_RSA_PKDecrypt)(unsigned char * output, unsigned int *outputLen,unsigned char * input,  unsigned int inputLen,unsigned char * PK, unsigned int pkLen);
typedef int   (*pci_firmware_api_dev_RSA_PKEncrypt)(unsigned char *output, unsigned int *outputLen,unsigned char *input,  unsigned int inputLen,unsigned char *PK, unsigned int pkLen);
typedef s32     (*pci_firmware_api_pciarith_file_hash)(u32 fileorgaddr,u32 filestaoffset,u32 inlen,u8 *hashvalue,u8 hashtype);
typedef s32     (*pci_firmware_api_dev_trng_read)(u8 *trngdata,u32 trngdatalen);
typedef int   (*pci_firmware_api_devsm3_getsummary)(unsigned char *pInput, unsigned int ilen, unsigned char *pOutput);
typedef int   (*pci_firmware_api_devsm2_verity)(const char *pPkeyX, const char *pPkeyY, const char *pSignR, const char *pSignS,const char *sm3result);


#if 0
typedef struct _PCI_FIRMWARE_API
{
	
	pci_firmware_api_open  pcifirmwareapi_open;
	pci_firmware_api_readandcheckappkey pcifirmwareapi_readandcheckappkey;
	pci_firmware_api_saveappkey pcifirmwareapi_saveappkey;
	pci_firmware_api_deleteppkey pcifirmwareapi_deleteppkey;
	pci_firmware_api_dryice_isrhandler pcifirmwareapi_dryice_timerhandler;
	pci_firmware_api_dryice_isrhandler pcifirmwareapi_dryice_isrhandler;
	pci_firmware_api_dukptk_load pcifirmwareapi_dukptk_load;
	pci_firmware_api_dukpt_check_keytmp pcifirmwareapi_dukpt_check_keytmp;

	
	pci_firmware_api_devarith_desencrypt pcifirmwareapi_devarith_desencrypt;
	pci_firmware_api_devarith_hash pcifirmwareapi_devarith_hash;
	pci_firmware_api_dev_RSA_PKDecrypt pcifirmwareapi_dev_RSA_PKDecrypt;
	pci_firmware_api_dev_RSA_PKEncrypt pcifirmwareapi_dev_RSA_PKEncrypt;
	pci_firmware_api_pciarith_file_hash pcifirmwareapi_pciarith_file_hash;
	pci_firmware_api_dev_trng_read pcifirmwareapi_dev_trng_read;
	pci_firmware_api_devsm3_getsummary pcifirmwareapi_devsm3_getsummary;
	pci_firmware_api_devsm2_verity pcifirmwareapi_devsm2_verity;

	
}PCI_FIRMWARE_API;
#endif


#endif

