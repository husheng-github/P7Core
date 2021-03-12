

#ifndef _PCIAPICALL_H_
#define _PCIAPICALL_H_

#include "pciapi.h"
#include "pciglobal.h"


#define PCI_24H_RESET
#define PCI_24H_VALUE 86400000


//extern PCI_FIRMWARE_API *pcicore;
//extern u8 g_pciinitialflag;



#define  pcifirmware_api_open                    (pcicore->pcifirmwareapi_open)
#define  pcifirmware_api_readandcheckappkey      (pcicore->pcifirmwareapi_readandcheckappkey)
#define  pcifirmware_api_saveappkey              (pcicore->pcifirmwareapi_saveappkey)
#define  pcifirmware_api_dryice_timerhandler     (pcicore->pcifirmwareapi_dryice_timerhandler)
#define  pcifirmware_api_dryice_isrhandler       (pcicore->pcifirmwareapi_dryice_isrhandler)
#define  pcifirmware_api_dukptk_load             (pcicore->pcifirmwareapi_dukptk_load)
#define  pcifirmware_api_dukpt_check_keytmp      (pcicore->pcifirmwareapi_dukpt_check_keytmp)


#define  pcifirmware_api_devarith_desencrypt      (pcicore->pcifirmwareapi_devarith_desencrypt)
#define  pcifirmware_api_devarith_hash            (pcicore->pcifirmwareapi_devarith_hash)
#define  pcifirmware_api_dev_RSA_PKDecrypt        (pcicore->pcifirmwareapi_dev_RSA_PKDecrypt)
#define  pcifirmware_api_dev_RSA_PKEncrypt        (pcicore->pcifirmwareapi_dev_RSA_PKEncrypt)
#define  pcifirmware_api_pciarith_file_hash        (pcicore->pcifirmwareapi_pciarith_file_hash)
#define  pcifirmware_api_dev_trng_read             (pcicore->pcifirmwareapi_dev_trng_read)
#define  pcifirmware_api_devsm3_getsummary         (pcicore->pcifirmwareapi_devsm3_getsummary)
#define  pcifirmware_api_devsm2_verity             (pcicore->pcifirmwareapi_devsm2_verity)



extern void pcimodule_init(void);
extern void pcitamper_manage_tampertask(void);

#endif


