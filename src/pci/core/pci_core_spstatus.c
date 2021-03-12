

#include "pciglobal.h"


SPSTATUS  gSpStatus;

void pci_core_spstatusinit(void)
{
	memset(&gSpStatus,0,sizeof(SPSTATUS));
}


SPSTATUS* pci_core_spstatusget(void)
{
  #if 0//def TRENDIT_CORE  
    gSpStatus.unlockmode = pcitamper_manage_getdryiceconfig();
  #endif
	return &gSpStatus;
}



