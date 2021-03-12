

#include "ddi_com.h"


#ifdef DDI_COM_ENABLE

s32 ddi_com_open (u32 nCom, const strComAttr *lpstrAttr)
{
	return dev_com_open(nCom,lpstrAttr->m_baud,lpstrAttr->m_databits,lpstrAttr->m_parity,lpstrAttr->m_stopbits,0);
}


s32 ddi_com_close (u32 nCom)
{
	return dev_com_close(nCom);
}



s32 ddi_com_clear (u32 nCom)
{
	return dev_com_flush(nCom);
}

s32 ddi_com_tcdrain (u32 nCom)
{
	return dev_com_tcdrain(nCom);
}


s32 ddi_com_read (u32 nCom, u8 *lpOut, s32 nLe )
{
	return dev_com_read(nCom,lpOut,nLe);
}

s32 ddi_com_write (u32 nCom, u8 *lpIn, s32 nLe )
{
	return dev_com_write(nCom,lpIn,nLe);
}


s32 ddi_com_ioctl(u32 nCom, u32 nCmd, u32 lParam, u32 wParam)
{

    return DDI_ENODEV;
}

#else

s32 ddi_com_open (u32 nCom, const strComAttr *lpstrAttr)
{
	return DDI_ENODEV;
}


s32 ddi_com_close (u32 nCom)
{
	return DDI_ENODEV;
}



s32 ddi_com_clear (u32 nCom)
{
	return DDI_ENODEV;
}


s32 ddi_com_read (u32 nCom, u8 *lpOut, s32 nLe )
{
	return DDI_ENODEV;
}

s32 ddi_com_write (u32 nCom, u8 *lpIn, s32 nLe )
{
	return DDI_ENODEV;
}


s32 ddi_com_ioctl(u32 nCom, u32 nCmd, u32 lParam, u32 wParam)
{

    return DDI_ENODEV;
}




#endif

