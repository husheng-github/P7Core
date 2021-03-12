
#include "ddi_rf.h"

#ifdef DDI_RF_ENABLE

s32 ddi_rf_open (void)
{
	return api_rf_open();
}


s32 ddi_rf_close(void)
{
	return api_rf_close();
}


s32 ddi_rf_poweron (u32 nType)
{
	return api_rf_poweron(nType);
}


s32 ddi_rf_poweroff (void)
{
    
	return api_rf_poweroff();
}


s32 ddi_rf_get_status (void)
{
	return api_rf_get_status();
}

s32 ddi_rf_activate (void)
{
	return api_rf_activate();
}


s32 ddi_rf_exchange_apdu (const u8*lpCApdu, u32 nCApduLen, u8*lpRApdu, u32 *lpRApduLen, u32 nRApduSize)
{
	return api_rf_exchange_apdu(lpCApdu, nCApduLen, lpRApdu, lpRApduLen, nRApduSize);
}

s32 ddi_rf_remove (void)
{
	return api_rf_remove();
}

s32 ddi_rf_ioctl(u32 nCmd, u32 lParam, u32 wParam)
{
	return api_rf_ioctl(nCmd, lParam, wParam);
}


#else

s32 ddi_rf_open (void)
{
	return DDI_ENODEV;
}


s32 ddi_rf_close(void)
{
	return DDI_ENODEV;
}


s32 ddi_rf_poweron (u32 nType)
{
	return DDI_ENODEV;
}


s32 ddi_rf_poweroff (void)
{
	return DDI_ENODEV;
}


s32 ddi_rf_get_status (void)
{
	return DDI_ENODEV;
}

s32 ddi_rf_activate (void)
{
	return DDI_ENODEV;
}


s32 ddi_rf_exchange_apdu (const u8*lpCApdu, u32 nCApduLen, u8*lpRApdu, u32 *lpRApduLen, u32 nRApduSize)
{
	return DDI_ENODEV;
}

s32 ddi_rf_remove (void)
{
	return DDI_ENODEV;
}

s32 ddi_rf_ioctl(u32 nCmd, u32 lParam, u32 wParam)
{
	return DDI_ENODEV;
}


#endif

