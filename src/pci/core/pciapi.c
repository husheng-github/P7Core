

#include "pciglobal.h"
#include "pciapi.h"

// @ ".pcicorevec" 
const PCI_FIRMWARE_API constpcicore =
{
	
	pci_core_open,
	pcikeymanage_readandcheckappkey,
	pcikeymanage_saveappkey,
	pcikeymanage_deleteappkey,
	pcitamper_manage_dryice_timerhandler,
	pcitamper_manage_dryice_isrhandler,
	NULL,//pcikeymanage_dukptk_load,
	NULL,//pcikeymanage_dukpt_check_keytmp,
	devarith_desencrypt,
	pciarith_hash,
	dev_RSA_PKDecrypt,
	dev_RSA_PKEncrypt,
	pciarith_file_hash,
	dev_trng_read,
	NULL,//dev_sm3_getsummary,
	NULL//dev_sm2_verity
	
	
};


