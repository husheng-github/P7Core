#ifndef __DRV_ICC_H
#define __DRV_ICC_H


#ifdef DEBUG_ICC_EN
#define ICC_DEBUG(format,...)    dev_debug_printf("%s(%d):"format, __func__, __LINE__, ##__VA_ARGS__)
#define ICC_DEBUGHEX             dev_debug_printformat
#else
#define ICC_DEBUG(...) 
#define ICC_DEBUGHEX(...)
#endif

void drv_icc_init(void);
s32 drv_icc_open(void);
s32 drv_icc_close(void);
s32 drv_icc_poweroff(void);
s32 drv_icc_getstatus();
s32 drv_icc_reset(u8 *lpAtr);
s32 drv_icc_exchange_apdu(u8 *wbuf, u32 wlen, u8 *rbuf, u32 *rlen, u32 rxmax);

#endif
