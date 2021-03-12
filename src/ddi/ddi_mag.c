

#include "ddi_mag.h"

#ifdef DDI_MAG_ENABLE

s32 ddi_mag_open (void)
{
	return dev_mag_open();
}



s32 ddi_mag_close (void)
{
	return dev_mag_close();
}


s32 ddi_mag_clear(void)
{
	return dev_mag_clear();
}


s32 ddi_mag_cfg(u32 cfg, void *p)
{
	return DDI_OK;
}


s32 ddi_mag_ioctl(u32 nCmd, u32 lParam, u32 wParam)
{
    u32 i;
    //
    switch(nCmd)
    {
        case DDI_MAG_CTL_VER:
            i = dev_mag_get_ver((u8*)wParam);
            break;
    	case DDI_MAG_CTL_GETMAGSTATUS:
			return dev_mag_ioctl_getmagswipestatus();
			break;
		default:
			return DDI_ENODEV;
			break;
    }
}



s32 ddi_mag_read(u8 *lpTrack1, u8 *lpTrack2, u8 *lpTrack3, u8 *trackstatus)
{
	return dev_mag_read(lpTrack1,lpTrack2,lpTrack3, trackstatus);
}


#else

s32 ddi_mag_open(void)
{
	return DDI_ENODEV;
}



s32 ddi_mag_close (void)
{
	return DDI_ENODEV;
}


s32 ddi_mag_clear(void)
{
	return DDI_ENODEV;
}


s32 ddi_mag_cfg(u32 cfg, void *p)
{
	return DDI_ENODEV;
}


s32 ddi_mag_ioctl(u32 nCmd, u32 lParam, u32 wParam)
{
	return DDI_ENODEV;
}



s32 ddi_mag_read(u8 *lpTrack1, u8 *lpTrack2, u8 *lpTrack3, u8 *trackstatus)
{
	return DDI_ENODEV;
}


#endif


