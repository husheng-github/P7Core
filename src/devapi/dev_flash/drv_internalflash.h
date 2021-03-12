#ifndef __DRV_INTERNALFLASH_H
#define __DRV_INTERNALFLASH_H



#define INTERNALFLASH_ADDRSTA   0x1000000


extern void drv_internalflash_printinfo(u8 i);


extern s32 drv_internalflash_erasesector(u32 startaddr);
extern s32 drv_internalflash_program(u32 startaddr,u32* rbuf,u32 rlen);
extern s32 drv_internalflash_write(u32 startaddr, u8 *wbuf, u32 wlen);
extern s32 drv_internalflash_read(u32 startaddr,u8* rbuf,u32 rlen);


#endif


