

#include "ddi_key.h"


#ifdef DDI_KEY_ENABLE

s32 ddi_key_open (void)
{
	return dev_keypad_open();
}



s32 ddi_key_close (void)
{
	return dev_keypad_close();
}


s32 ddi_key_clear (void)
{
	return dev_keypad_clear();
}



s32 ddi_key_read (u32 *lpKey)
{
	
	u32 key;
    
	*lpKey = 0;
	if(dev_keypad_read(&key))
	{
		*lpKey = key;
		return 1;
	}
	
	return 0;
	
}

s32 ddi_key_read_withoutdelay(u32 *lpKey)
{
	u32 key;
    
	*lpKey = 0;
	if(dev_keypad_read(&key))
	{
		*lpKey = key;
		return 1;
	}
	
	return 0;
}

s32 ddi_key_ioctl(u32 nCmd, u32 lParam, u32 wParam)
{
	return dev_keypad_ioctl(nCmd, lParam, wParam);
}

//*lpKey = 1,电源键被按下，*lpKey = 0 未按下。
s32 ddi_powerkey_check (u8 *lpKey)
{
	//drv_power_check_powerkey(&key);
//	*lpKey = dev_keypad_read_power();
	return 0;
}


#else

s32 ddi_key_open (void)
{
	return DDI_ENODEV;
}



s32 ddi_key_close (void)
{
	return DDI_ENODEV;
}


s32 ddi_key_clear (void)
{
	return DDI_ENODEV;
}



s32 ddi_key_read (u32 *lpKey)
{
	
	return DDI_ENODEV;
	
}
s32 ddi_key_read_withoutdelay(u32 *lpKey)
{
    return DDI_ENODEV;
}


s32 ddi_key_ioctl(u32 nCmd, u32 lParam, u32 wParam)
{
	return DDI_ENODEV;
}


#endif


