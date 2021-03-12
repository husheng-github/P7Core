


#include "ddi_iccpsam.h"
#define ICC_NEW     1


#ifdef DDI_ICCPSAM_ENABLE


s32 ddi_iccpsam_open (u32 nSlot)
{
    s32 ret;

  #ifdef ICC_NEW    
	ret = dev_icc_open(nSlot);
  #else
	ret = posdev_iccard_open(nSlot);
  #endif
	if(ret == DEVSTATUS_SUCCESS)
	{
		return DDI_OK;
	}
	else
	{
		return DDI_ERR;
	}
}



s32 ddi_iccpsam_close(u32 nSlot)
{
    s32 ret;
  #ifdef ICC_NEW 
	ret = dev_icc_close(nSlot);
  #else
	ret = posdev_iccard_close(nSlot);
  #endif
	if(ret == DEVSTATUS_SUCCESS)
	{
		return DDI_OK;
	}
	else
	{
		return DDI_ERR;
	}
}


s32 ddi_iccpsam_poweroff (u32 nSlot)
{
    s32 ret;
  #ifdef ICC_NEW   
	ret = dev_icc_poweroff(nSlot);
  #else
	ret = posdev_iccard_poweroff(nSlot);
  #endif
	if(ret == DEVSTATUS_SUCCESS)
	{
		return DDI_OK;
	}
	else
	{
		return DDI_ERR;
	}
	
}



s32 ddi_iccpsam_poweron (u32 nSlot, u8 *lpAtr)
{
    s32 ret;
  #ifdef ICC_NEW   
	ret = dev_icc_reset(nSlot,&lpAtr[1]);
  #else
	ret = posdev_iccard_poweron(nSlot,&lpAtr[1]);
  #endif
    if(ret>0)
	{
	    lpAtr[0] = ret;
		return DDI_OK;
	}
	else
	{
		return DDI_ERR;
	}
}


s32 ddi_iccpsam_exchange_apdu (u32 nSlot, const u8* lpCApdu, u32 nCApduLen, u8*lpRApdu, u32* lpRApduLen, u32 nRApduSize)
{
    s32 ret;
	
//dev_debug_printf("%s(%d):nslot=%d, wlen=%d\r\n", __FUNCTION__, __LINE__, nSlot, nCApduLen);  
//dev_debug_printformat(NULL, (u8*)lpCApdu, nCApduLen);
  #ifdef ICC_NEW   
	ret = dev_icc_exchange_apdu(nSlot,lpCApdu,nCApduLen,lpRApdu,lpRApduLen,nRApduSize);
  #else
	ret = posdev_iccard_exchange_apdu(nSlot,lpCApdu,nCApduLen,lpRApdu,lpRApduLen,nRApduSize);
  #endif
//dev_debug_printf("%s(%d):ret=%d, rlen=%d\r\n", __FUNCTION__, __LINE__, ret, *lpRApduLen);  
    if(ret == 0)
	{
//dev_debug_printformat(NULL, lpRApdu, *lpRApduLen);
		return DDI_OK;
	}
	else
	{
	    *lpRApduLen = 0;
		return DDI_ERR;
	}
}

s32 ddi_iccpsam_get_status (u32 nSlot)
{
    s32 ret;
    
  #ifdef ICC_NEW    
	ret = dev_icc_getstatus(nSlot);
	if(ret == 1)
  #else
	ret = posdev_iccard_getstatus(nSlot);
	if(ret == DEVSTATUS_SUCCESS)
  #endif
	{
		return 2;
	}
	else
	{
		return DDI_ERR;
	}
}

s32 ddi_iccpsam_ioctl(u32 nCmd, u32 lParam, u32 wParam)
{
	return DDI_OK;
}


#else

s32 ddi_iccpsam_open (u32 nSlot)
{
	return DDI_ENODEV;
}



s32 ddi_iccpsam_close(u32 nSlot)
{
	return DDI_ENODEV;
}


s32 ddi_iccpsam_poweroff (u32 nSlot)
{
	return DDI_ENODEV;
}



s32 ddi_iccpsam_poweron (u32 nSlot, u8 *lpAtr)
{
	return DDI_ENODEV;
}


s32 ddi_iccpsam_exchange_apdu (u32 nSlot, const u8* lpCApdu, u32 nCApduLen, u8*lpRApdu, u32* lpRApduLen, u32 nRApduSize)
{
	return DDI_ENODEV;
}

s32 ddi_iccpsam_get_status (u32 nSlot)
{
	return DDI_ENODEV;
}

s32 ddi_iccpsam_ioctl(u32 nCmd, u32 lParam, u32 wParam)
{
	return DDI_ENODEV;
}


#endif




