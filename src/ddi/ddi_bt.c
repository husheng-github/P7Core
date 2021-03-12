

#include "ddi_bt.h"


#ifdef DDI_BT_ENABLE

s32 ddi_bt_open(void)
{
    return dev_bt_open();
}


s32 ddi_bt_close(void)
{
    return dev_bt_close();
}


s32 ddi_bt_write(u8 *lpIn, s32 nLe)
{
    return dev_bt_write(lpIn, nLe);
}


s32 ddi_bt_read (u8 *lpOut, s32 nLe)
{
    return dev_bt_read(lpOut, nLe);
}


s32 ddi_bt_get_status(void)
{
    return dev_bt_get_status();
}


s32 ddi_bt_disconnect(const u8 *lpMac)
{
    return dev_bt_disconnect(lpMac);
}


s32 ddi_bt_ioctl(u32 nCmd, u32 lParam, u32 wParam)
{
    return dev_bt_ioctl(nCmd, lParam, wParam);
}



#else


s32 ddi_bt_open(void)
{
	return DDI_ENODEV;
}


s32 ddi_bt_close(void)
{
	return DDI_ENODEV;
}


s32 ddi_bt_write(u8 *lpIn, s32 nLe)
{
	return DDI_ENODEV;
}


s32 ddi_bt_read (u8 *lpOut, s32 nLe)
{
	return DDI_ENODEV;
}


s32 ddi_bt_get_status(void)
{
	return DDI_ENODEV;
}


s32 ddi_bt_disconnect(const u8 *lpMac)
{
	return DDI_ENODEV;
}


s32 ddi_bt_ioctl(u32 nCmd, u32 lParam, u32 wParam)
{
	return DDI_ENODEV;
}


#endif




