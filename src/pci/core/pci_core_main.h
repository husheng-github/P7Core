

#ifndef _PCI_CORE_MAIN_H_
#define _PCI_CORE_MAIN_H_

#define FSEVALUEDEFFILE  "/mtd2/fsevalue"





#define FSEVALUEDEF        0x08
#define FSEVALUEADDR       0x40C



extern void pci_core_open (void);

extern void pcitamper_manage_tampertask(void);
extern void pcitamper_manage_tamperset(u8 type);
extern s32 pcitamper_manage_dryice_isrhandler(u32 drysrreg);
extern s32 pcitamper_manage_dryice_timerhandler(void);
extern void pcitamper_manage_selfcheck_securitylocked(u8 type);
extern u32 pcitamper_manage_unlock(void);
extern u32 pcitamper_manage_getcurrenttamperstatus(void);
extern u32 pcitamper_manage_gettamperstatus(void);
extern u32 pcitamper_manage_dryiceopen(u8 dryicetype);
extern u32 pcitamper_manage_getdryiceconfig(void);


extern u8 pcitamper_manage_readunlocktimes(void);
extern u32 pcitamper_manage_saveunlocktimes(u8 locktimes);
extern u32 pcitamper_manage_readtamperstatus(u8 *unlocktimes);
extern u32 pci_core_readtamperstatus(void);
extern void pci_core_close(void);
extern void pcitampermanage_filesave(PCITAMPERMANAGEFILEINFO *pcitampermanagefileinfo);
extern void pcitamper_manage_tampercfg(DRYICETAMPERCONFIG *dryicetamperset);
extern void pcitest(void);

#endif


