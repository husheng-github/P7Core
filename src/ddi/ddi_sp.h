#ifndef __DDI_SP_H
#define __DDI_SP_H


#include "ddi_common.h"
#include "ddi_misc.h"

typedef enum _SPSECURITYLEVEL
{
	SECURITYLEVEL_NONE = 0,    //触发未启用
	SECURITYLEVEL_NORMAL,      //后台解触发
	SECURITYLEVEL_DEBUG        //任意数据解触发
} SPSECURITYLEVEL;

//============================================
//对外函数声明
s32 ddi_spnormalmanage_reset(void);
void ddi_spnormalmanage_hardwarereset(void);
s32 ddi_spnormalmanage_cancel(void);
s32 ddi_spnormalmanage_beep(u8 times,u32 beepfreq,u32 beepenabletimems,u32 beepdisabletimems);
s32 ddi_spnormalmanage_setsleeptime(u32 mssleeptime);
s32 ddi_spnormalmanage_getsleeptime(u32 *mssleeptime);
s32 ddi_spnormalmanage_setdevicesn(u8 *sn,u32 snlen);
s32 ddi_spnormalmanage_getdevicesn(u8 *sn,u32 *snlen);
s32 ddi_spnormalmanage_getversion(u8 versiontype,u8 *version,u32 *versionlen);
s32 ddi_spnormalmanage_randnum(u16 randnumlen,u8 *randnum,u16 *resprandnumlen);
s32 ddi_spnormalmanage_intosleep(void);

s32 ddi_spsecuremanage_getspstatus(SPSTATUS *spstatus);
s32 ddi_spsecuremanage_gettamperinfo(SPSTATUS *spstatus);
s32 ddi_spsecuremanage_requestsecuredata(u8 securetype,u8 *securedata,u32 *securedatalen);
s32 ddi_spsecuremanage_setsecuredata(u8 *securedata,u32 securedatalen);
s32 ddi_spsecuremanage_getdebugflag(u8 *encrypteddata,u32 encrypteddatalen,u8 *debugflagdata);
s32 ddi_spsecuremanage_opendryice(SPSECURITYLEVEL *securitylevel);
s32 ddi_spsecuremanage_getencryptedranddata(u8 *encryptedranddata,u32 *encryptedranddatalen);
s32 ddi_dev_apspcomm_command(u16 sendcmd,u8 *senddata,u32 senddatalen,u8 *recdata,u32 *recdatalen,u32 maxreclen,u32 mstimeout);

s32 ddi_secureprevent_getspinfo(u8 *spinfo,u32 *spinfolen);
s32 ddi_secureprevent_sendbinddatatosp(u8 *binddata,u32 binddatalen);
s32 ddi_secureprevent_saveappcertinfo(u8 *appcertinfo,u32 appcertinfolen);
s32 ddi_secureprevent_readappcertinfo(u8 *appcertinfo,u32 *appcertinfolen);
s32 ddi_secureprevent_downloadsnkey(u8 *snkeydata ,u32 snkeydatalen);
s32 ddi_secureprevent_getsnkeycheckvalue(u8 *snkeycheckvalue,u16 *snkeycheckvaluelen);

s32 ddi_spfirmwaremanage_getspcertinfo(u8 *certname,u32 certnamelen,u8 *certinfo,u32 *certinfolen);
s32 ddi_spfirmwaremanage_update(u8 *updatedata,u32 updatedatalen,u8 *promptinfo,u32 *promptinfolen);
s32 ddi_spfirmwaremanage_intobootmode(void);
s32 ddi_spfirmwaremanage_exitbootmode(void);
s32 ddi_spfirmwaremanage_intocdcdatatospimode(u8 mode,u8 *cdcdata,u32 *cdcdatalen);
s32 ddi_spfirmwaremanage_spireadcdcdata(void);
void ddi_spsecuremanage_dump_memoryinfo(u8 type);

typedef s32 (*core_ddi_spnormalmanage_reset)(void);
typedef void (*core_ddi_spnormalmanage_hardwarereset)(void);
typedef s32 (*core_ddi_spnormalmanage_cancel)(void);
typedef	s32 (*core_ddi_spnormalmanage_beep)(u8 times,u32 beepfreq,u32 beepenabletimems,u32 beepdisabletimems);
typedef s32 (*core_ddi_spnormalmanage_setsleeptime)(u32 mssleeptime);
typedef s32 (*core_ddi_spnormalmanage_getsleeptime)(u32 *mssleeptime);
typedef s32 (*core_ddi_spnormalmanage_setdevicesn)(u8 *sn,u32 snlen);
typedef s32 (*core_ddi_spnormalmanage_getdevicesn)(u8 *sn,u32 *snlen);
typedef s32 (*core_ddi_spnormalmanage_getversion)(u8 versiontype,u8 *version,u32 *versionlen);
typedef s32 (*core_ddi_spnormalmanage_randnum)(u16 randnumlen,u8 *randnum,u16 *resprandnumlen);
typedef s32 (*core_ddi_spnormalmanage_intosleep)(void);

typedef s32 (*core_ddi_spsecuremanage_getspstatus)(SPSTATUS *spstatus);
typedef s32 (*core_ddi_spsecuremanage_gettamperinfo)(SPSTATUS *spstatus);
typedef s32 (*core_ddi_spsecuremanage_requestsecuredata)(u8 securetype,u8 *securedata,u32 *securedatalen);
typedef s32 (*core_ddi_spsecuremanage_setsecuredata)(u8 *securedata,u32 securedatalen);
typedef s32 (*core_ddi_spsecuremanage_getdebugflag)(u8 *encrypteddata,u32 encrypteddatalen,u8 *debugflagdata);
typedef s32 (*core_ddi_spsecuremanage_opendryice)(SPSECURITYLEVEL *securitylevel);
typedef s32 (*core_ddi_spsecuremanage_getencryptedranddata)(u8 *encryptedranddata,u32 *encryptedranddatalen);
typedef s32 (*core_ddi_dev_apspcomm_command)(u16 sendcmd,u8 *senddata,u32 senddatalen,u8 *recdata,u32 *recdatalen,u32 maxreclen,u32 mstimeout);

typedef s32 (*core_ddi_secureprevent_getspinfo)(u8 *spinfo,u32 *spinfolen);
typedef s32 (*core_ddi_secureprevent_sendbinddatatosp)(u8 *binddata,u32 binddatalen);
typedef s32 (*core_ddi_secureprevent_saveappcertinfo)(u8 *appcertinfo,u32 appcertinfolen);
typedef s32 (*core_ddi_secureprevent_readappcertinfo)(u8 *appcertinfo,u32 *appcertinfolen);
typedef s32 (*core_ddi_secureprevent_downloadsnkey)(u8 *snkeydata ,u32 snkeydatalen);
typedef s32 (*core_ddi_secureprevent_getsnkeycheckvalue)(u8 *snkeycheckvalue,u16 *snkeycheckvaluelen);

typedef s32 (*core_ddi_spfirmwaremanage_getspcertinfo)(u8 *certname,u32 certnamelen,u8 *certinfo,u32 *certinfolen);
typedef s32 (*core_ddi_spfirmwaremanage_update)(u8 *updatedata,u32 updatedatalen,u8 *promptinfo,u32 *promptinfolen);
typedef s32 (*core_ddi_spfirmwaremanage_intobootmode)(void);
typedef s32 (*core_ddi_spfirmwaremanage_exitbootmode)(void);
typedef s32 (*core_ddi_spfirmwaremanage_intocdcdatatospimode)(u8 mode,u8 *cdcdata,u32 *cdcdatalen);
typedef s32 (*core_ddi_spfirmwaremanage_spireadcdcdata)(void);
typedef void (*core_ddi_spsecuremanage_dump_memoryinfo)(u8 type);
#endif
