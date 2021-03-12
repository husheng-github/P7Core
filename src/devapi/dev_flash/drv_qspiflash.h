#ifndef __DRV_QSPIFLASH_H
#define __DRV_QSPIFLASH_H
#define QSPIFLSH_DEBUG_EN

#if 0//def DEBUG_QSPIFLASH_EN
#define QSPIFLASH_DEBUG(format,...)    dev_debug_printf("%s(%d):"format, __func__, __LINE__, ##__VA_ARGS__)//;dev_user_delay_ms(20)
#define QSPIFLASH_DEBUGHEX             dev_debug_printformat
#else
#define QSPIFLASH_DEBUG(...) 
#define QSPIFLASH_DEBUGHEX(...)
#endif



s32 drv_qspiflash_open(void);
s32 drv_qspiflash_writespec(u32 startaddr, u8 *wbuf, u32 wlen);
s32 drv_qspiflash_read(u32 startaddr,u8* rbuf,u32 rlen);


#endif
