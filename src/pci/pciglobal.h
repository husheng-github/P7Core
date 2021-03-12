

#ifndef _PCIGLOBAL_H_
#define _PCIGLOBAL_H_


//#ifdef AFX_PCI_VARIABLES
//    #define EXTERN
//#else
//    #define EXTERN extern
//#endif





//#define SUPPORTDUKPTK



#define PCI_PROCESSCMD_SUCCESS                0
#define PCI_KEYTYPE_ERR                      (-7001)
#define PCI_KEYLRC_ERR                       (-7002)
#define PCI_KEYINDEX_ERR                     (-7003)
#define PCI_KEYLEN_ERR                       (-7004)
#define PCI_KEYMODE_ERR                      (-7005) 
#define PCI_ADMINPWDFORMAT_ERR               (-7011)
#define PCI_WRITEKEY_ERR                     (-7012)
#define PCI_READKEY_ERR                      (-7013)
#define PCI_DATALEN_ERR                      (-7015)
#define PCI_READMMK_ERR                      (-7020)
#define PCI_RSAKEY_ERR                       (-7031)
#define PCI_KEYSAME_ERR                      (-7040)
#define PCI_INPUTPARAMS_ERR                  (-7041)
#define PCI_DEALPCIFILE_ERR                  (-7042)
#define PCI_DEAL_ERR                         (-7043)
#define PCI_FILEUNFIND_ERR                   (-7044)
#define PCI_ADMINPWDVERIFY_ERR               (-7045)
#define PCI_PKCERTEXIST_ERR                  (-7046)
#define PCI_PKCERTNUM_OVERFLOW               (-7048)
#define PCI_PKUNFIND_ERR                     (-7049)
#define PCI_RSACALCULATE_ERR                 (-7050)
#define PCI_RSAKEYPAIRGEN_ERR                (-7051)
#define PCI_READCAK_ERR                      (-7052)


#define PCI_KMALLOCMEM_ERR                   (-7053)
#define PCI_APPLISTNODE_UNFIND               (-7054)
#define PCI_OPENFILE_ERR                     (-7055)
#define PCI_ADMINISTRATOR_LOCKED             (-7056)
#define PCI_DUKPTK_INVALIDKEYINDEX           (-7057)
#define PCI_DUKPTK_INVALIDBDK                (-7058)
#define PCI_DUKPTK_INVALIDKSN                (-7059)
#define PCI_DUKPTK_INVALIDKSNCRC             (-7060)
#define PCI_FUTUREKEY_INVALIDKEYINDEX        (-7061)
#define PCI_FUTUREKEY_INVALIDKEYLEN          (-7062)
#define PCI_FUTUREKEY_INVALIDCRC             (-7063)
#define PCI_FUTUREKEY_COUNTEROVERFLOW        (-7064)
#define PCI_FUTUREKEY_NOTGENERATE            (-7065)
#define PCI_MACALGORITHMINDEX_ERR            (-7066)
#define PCI_FILELEN_ZERO                     (-7067)
#define PCI_RSAKEYINDEX_OVERFLOW             (-7068)
#define PCI_TRSMAPP_EXIST                    (-7069)
#define PCI_TRSMAPP_OVERFLOW                 (-7070)
#define PCI_CURINDEXTRSMAPP_EMPTY            (-7071)
#define PCI_TRSMAPP_NOTEXIST                 (-7072)
#define PCI_TRSMAPPKEYNOTEXIST_ERR           (-7073)
#define PCI_REGISTERAPPNUM_OVERFLOW          (-7074)
#define PCI_AUTHENRSAKEYLEN_ERR              (-7075)
#define PCI_AUTHENRSAKEYEXIST_ERR            (-7076)
#define PCI_AUTHENRSAPK_ERR                  (-7077)
#define PCI_AUTHENRSASK_ERR                  (-7078)
#define PCI_AUTHENRSABOTHKEY_ERR             (-7079)
#define PCI_AUTHENRSAKEY_OTHERERR            (-7080)
#define PCI_AUTHENAPPLOCKINFO_ERR            (-7081)
#define PCI_APPLOCKINFOSAVE_ERR              (-7082)
#define PCI_AUTHENKEYISSUER_ERROR            (-7083)
#define PCI_AUTHENDOWNAPPKEY_FAILED          (-7084)
#define PCI_AUTHENPTK_ERR                    (-7085)
#define PCI_AUTHEN_NEED                      (-7086)
#define PCI_AUTHENAPPKEYLOCK_ERR             (-7087)
#define PCI_GETPIN_INTERVALUNREACH           (-7088)
#define PCI_AUTHENRSAKEY_NOTEXIST            (-7089)
#define PCI_FILECERTFORMAT_ERR               (-7090)
#define PCI_FILEAUTHENFAIL                   (-7091)
#define PCI_FILEISCERTIFICATEFILE            (-7092)
#define PCI_APPREADNEEDCONTINUE              (-7093)
#define PCI_KEYGROUPINDEX_ERR                (-7094)
#define PCI_PINENTRYPTMODE_ERR               (-7095)
#define PCI_READCPUID_ERR                    (-7096)
#define PCI_READELRCK_ERR                    (-7097)
#define PCI_ADMINPWDFILE_ERR                 (-7098)  
#define PCI_ADMINPWDVERIFYFAIL               (-7099)
#define PCI_PUBLICKKEY_UNFIND                (-7100)
#define PCI_READSESSIONK_ERR                 (-7101)
#define PCI_KEYINDEXMATCH_ERR                (-7102)
#define PCI_KEYFILENOTUSE                    (-7103)
#define PCI_KEYEXIST_ERR                     (-7104)
#define PCI_KEYAPPNAMENOTMATCH               (-7105)
#define PCI_MAGKEYFILEERR                    (-7106)
#define PCI_MAGKEYFILNOTEXIST                (-7107)
#define PCI_MAGREADERREPLACED                (-7108)
#define PCI_MAGREADERUNVALID                 (-7109)
#define PCI_MAGREADERSYNCFAILURE             (-7110)
#define PCI_HAVEAPPCAPKEXIST                 (-7111)
#define PCI_MMKREGENERATE                    (-7112)


#define PCI_APPDUKPTKLISTFULL                (-7113)
#define PCI_FILEAUTHENREGISTERERR            (-7114)


#define PCI_TOUCHSCREENIDFILEERR             (-7115)   
#define PCI_TOUCHSCREENIDFILENOTEXIST        (-7116)


#define PCI_MMKBPKFILEERR                    (-7117)
#define PCI_MMKBPKFILENOTEXIST               (-7118)
#define PCI_MMKRECOVERFAILED                 (-7119)
#define PCI_MMKBPKDATANOTCONSISTENCY         (-7120)
#define PCI_APPKEYHEADINFONOTEXIST           (-7121)
#define PCI_MMKNOTINITIAL                    (-7122)
#define PCI_MAGDATA_WAITSWIPE                (-7123)
#define PCI_MAGDATA_ERR                      (-7124)
#define PCI_READMMK_ALLZERO                  (-7125)

//#include "ddi.h"
#include "devglobal.h"
#include "fs.h"
#ifndef NULL
#define NULL 0
#endif


#define SXL_DEBUG
#undef  SXL_DEBUG

#define PCI_DEBUG
#undef  PCI_DEBUG

#ifdef PCI_DEBUG
#define DDEBUG(format,...)    dev_debug_printf("%s(%d):"format, __func__, __LINE__, ##__VA_ARGS__)
#define printk dev_debug_printf
#define PCI_DEBUGHEX    dev_debug_printformat
#else
#define DDEBUG(fmt,arg...) 
#define printk(fmt,arg...)
#define PCI_DEBUGHEX    
#endif

//#include "driverglobal.h"
//#include "deviceglobal.h"



#include "./core/pci_core_callfundefine.h"
#include "./core/pciapi.h"


#include "./tampermanage/pcitampermanage.h"
#include "./tampermanage/pcitampermanagemmkbak.h"



#include "./pciarith/pciarithnormal.h"
#include "./pciarith/pciarithscc.h"
#include "./pciarith/pciarithdes.h"
#include "./pciarith/pciarithhash.h"
#include "./pciarith/pciarithrsa.h"
#include "./pciarith/pciarithsm3.h"



#include "./keymanage/pcikeymanage.h"
#include "./keymanage/pcikeymanageapi.h"
#include "./keymanage/pcikeymanageauthen.h"
#include "./keymanage/pcikeymanage_dukpt.h"
#include "./keymanage/pcipkmanage.h"


#include "./pcitask/pcikeydownload.h"
#include "./pcitask/pcitask.h"
#include "./pcitask/pcitask_calcmac.h"
#include "./pcitask/pcilog.h"
#include "./pcitask/pcitask_protocol_keydown.h"


#include "./core/pci_core_main.h"
#include "./core/pci_core_spstatus.h"
#include "./core/pci_core_callfundefine.h"





extern s32  gPciInitState;
extern const PCI_CORE_CALLFUNC pcicorecallfunc;
extern DUKPTK_INFO gDukptkInfo;
extern unsigned int appkeylockbits;
extern PINKEYUSEINTERNAL gPinKeyUseInternal;
extern SECURITYAUTHENTICATEINFO gSecurityAuthInfo;
extern u8 timerstartflag;
extern u32 appkeylocktimer;
extern PCIMANAGE gPciManagerInfo;
extern PCITAMPERMANAGE gPCITamperManage;  //sxl? tamper¨¦¨¨??D¨¨¨°a¨¬???¨°??????¡ä¡ä|¨¤¨ª,D¨¨¨°a?¨´?Y¨®2?t¡ã?¡À?o?¨¤¡ä
extern s32 gDownLoadKeyErr;
extern u8 pownonflag;
extern u8 gBpkResetFlag;

extern SPSTATUS  gSpStatus;
extern PCITAMPERMANAGEFILEINFO   gPciTamperManageFileInfo;

#ifdef SAVEPCILOG
extern PCILKEYLOGINFO gPciKeyLogInfo;
#endif
extern COMMUNICATEDATA *gCommData;

#endif

