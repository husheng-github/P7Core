

#include "pciglobal.h"
#include "ddi_watchdog.h"


#if(WATCHDOG_EN  == 1)      

void ddi_watchdog_open(void)
{
	dev_watchdog_open();
}


void ddi_watchdog_close (void)
{
	dev_watchdog_close();
}


void ddi_watchdog_feed(void)
{
    dev_watchdog_feed();
}

void ddi_watchdog_ioctl(u8 nCmd, u32 lParam, u32 wParam)
{
    
}

#else
void ddi_watchdog_open(void)
{
	
}


void ddi_watchdog_close (void)
{
	
}


void ddi_watchdog_feed(void)
{
	
}

#endif


