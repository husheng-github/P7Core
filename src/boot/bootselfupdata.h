#ifndef __BOOTSELFUPDATA_H
#define __BOOTSELFUPDATA_H

typedef union _inFlashWriteBuf
{
   u8                 Buf[512];
   u16                U16Buf[256];
   u32                U32Buf[128];
}inFlashWriteBuf_t;




//__ramfunc s32 bootselfupdata_deal(u32 startaddr, u32 destaddr, u32 bootlen);
s32 bootselfupdata_main(u32 startaddr, u32 destaddr, u32 bootlen);
void bootselfupdata_setflg(u8 num, u8 flg);


#endif
