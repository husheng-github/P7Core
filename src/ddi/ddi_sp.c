
#include "ddi_sp.h"
s32 ddi_spnormalmanage_reset(void)
{
	return DDI_ENODEV;
}
void ddi_spnormalmanage_hardwarereset(void)
{
}
s32 ddi_spnormalmanage_cancel(void)
{
	return DDI_ENODEV;
}
s32 ddi_spnormalmanage_beep(u8 times,u32 beepfreq,u32 beepenabletimems,u32 beepdisabletimems)
{
	return DDI_ENODEV;
}
s32 ddi_spnormalmanage_setsleeptime(u32 mssleeptime)
{
	return DDI_ENODEV;
}
s32 ddi_spnormalmanage_getsleeptime(u32 *mssleeptime)
{
	return DDI_ENODEV;
}
s32 ddi_spnormalmanage_setdevicesn(u8 *sn,u32 snlen)
{
	return DDI_ENODEV;
}
s32 ddi_spnormalmanage_getdevicesn(u8 *sn,u32 *snlen)
{
	return DDI_ENODEV;
}
s32 ddi_spnormalmanage_getversion(u8 versiontype,u8 *version,u32 *versionlen)
{
	return DDI_ENODEV;
}
s32 ddi_spnormalmanage_randnum(u16 randnumlen,u8 *randnum,u16 *resprandnumlen)
{
	return DDI_ENODEV;
}
s32 ddi_spnormalmanage_intosleep(void)
{
	return DDI_ENODEV;
}

s32 ddi_spsecuremanage_getspstatus(SPSTATUS *spstatus)
{
	return DDI_ENODEV;
}
s32 ddi_spsecuremanage_gettamperinfo(SPSTATUS *spstatus)
{
	return DDI_ENODEV;
}
s32 ddi_spsecuremanage_requestsecuredata(u8 securetype,u8 *securedata,u32 *securedatalen)
{
	return DDI_ENODEV;
}
s32 ddi_spsecuremanage_setsecuredata(u8 *securedata,u32 securedatalen)
{
	return DDI_ENODEV;
}
s32 ddi_spsecuremanage_getdebugflag(u8 *encrypteddata,u32 encrypteddatalen,u8 *debugflagdata)
{
	return DDI_ENODEV;
}
s32 ddi_spsecuremanage_opendryice(SPSECURITYLEVEL *securitylevel)
{
	return DDI_ENODEV;
}
s32 ddi_spsecuremanage_getencryptedranddata(u8 *encryptedranddata,u32 *encryptedranddatalen)
{
	return DDI_ENODEV;
}
s32 ddi_dev_apspcomm_command(u16 sendcmd,u8 *senddata,u32 senddatalen,u8 *recdata,u32 *recdatalen,u32 maxreclen,u32 mstimeout)
{
	return DDI_ENODEV;
}

s32 ddi_secureprevent_getspinfo(u8 *spinfo,u32 *spinfolen)
{
	return DDI_ENODEV;
}
s32 ddi_secureprevent_sendbinddatatosp(u8 *binddata,u32 binddatalen)
{
	return DDI_ENODEV;
}
s32 ddi_secureprevent_saveappcertinfo(u8 *appcertinfo,u32 appcertinfolen)
{
	return DDI_ENODEV;
}
s32 ddi_secureprevent_readappcertinfo(u8 *appcertinfo,u32 *appcertinfolen)
{
	return DDI_ENODEV;
}
s32 ddi_secureprevent_downloadsnkey(u8 *snkeydata ,u32 snkeydatalen)
{
	return DDI_ENODEV;
}
s32 ddi_secureprevent_getsnkeycheckvalue(u8 *snkeycheckvalue,u16 *snkeycheckvaluelen)
{
	return DDI_ENODEV;
}

s32 ddi_spfirmwaremanage_getspcertinfo(u8 *certname,u32 certnamelen,u8 *certinfo,u32 *certinfolen)
{
	return DDI_ENODEV;
}
s32 ddi_spfirmwaremanage_update(u8 *updatedata,u32 updatedatalen,u8 *promptinfo,u32 *promptinfolen)
{
	return DDI_ENODEV;
}
s32 ddi_spfirmwaremanage_intobootmode(void)
{
	return DDI_ENODEV;
}
s32 ddi_spfirmwaremanage_exitbootmode(void)
{
	return DDI_ENODEV;
}
s32 ddi_spfirmwaremanage_intocdcdatatospimode(u8 mode,u8 *cdcdata,u32 *cdcdatalen)
{
	return DDI_ENODEV;
}
s32 ddi_spfirmwaremanage_spireadcdcdata(void)
{
	return DDI_ENODEV;
}
#if 0
s32 ddi_secureprevent_downloadsnkey(u8 *snkeydata ,u32 snkeydatalen)
{
    return DDI_ENODEV;
}
s32 ddi_spsecuremanage_getencryptedranddata(u8 *encryptedranddata,u16 *encryptedranddatalen)
{
    return DDI_ENODEV; 
}
#endif
void ddi_spsecuremanage_dump_memoryinfo(u8 type)
{
    return;//DDI_ENODEV; 
}


