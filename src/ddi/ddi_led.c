

#include "devglobal.h"
#include "ddi_led.h"




#ifdef DDI_LED_ENABLE


s32 ddi_led_open (void)
{
	return dev_led_open();
}


s32 ddi_led_close (void)
{
	return dev_led_close();
}


s32 ddi_led_sta_set(u32 nLed, u32 nSta)
{
	return dev_led_sta_set(nLed, nSta);
}


s32 ddi_led_ioctl(u32 nCmd, u32 lParam, u32 wParam)
{
	return dev_led_ioctl(nCmd, lParam, wParam);
}

#else

s32 ddi_led_open (void)
{
	return DDI_ENODEV;
}


s32 ddi_led_close (void)
{
	return DDI_ENODEV;
}


s32 ddi_led_sta_set(u32 nLed, u32 nSta)
{
	return DDI_ENODEV;
}


s32 ddi_led_ioctl(u32 nCmd, u32 lParam, u32 wParam)
{
	return DDI_ENODEV;
}




#endif


