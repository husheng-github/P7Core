#ifndef __DEV_FLASH_H
#define __DEV_FLASH_H

//FLASH统一编址,
#define SPIFLASH_ADDRSTA        0x68000000  //SPIFLASH的相对起始地址

#define dev_flash_writespec    dev_flash_writewitherase

union _FlashWriteBuf
{
   u8   Buf[4096];
   u16  U16Buf[2048];
   u32  U32Buf[1024];
};
//
s32 dev_flash_init(void);
s32 dev_flash_read(u32 addrsta, u8* rbuf, u32 rlen);
s32 dev_flash_write(u32 addrsta, u8 *wbuf, u32 wlen);
s32 dev_flash_erasesector(u32 addrsta);
s32 dev_flash_writewitherase(u32 addrsta, u8 *wbuf, u32 wlen);



#endif
