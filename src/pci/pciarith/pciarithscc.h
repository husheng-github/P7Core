

#ifndef _PCIARITHSCC_H_
#define _PCIARITHSCC_H_


#define DRV_PCI_CAK_LEN                    24 //certificate authentication key
#define DRV_PCI_CAK_FLAG    (('C' << 24) +('A' << 16) + ('K' << 8) + ('F' << 0)) 

typedef struct _PCICAK
{
    u32 flag;
    u8 keydata[DRV_PCI_CAK_LEN];
    u8 crc[4];
}PCICAK;



extern s32 scc_decryptdata(u8 *data,u32 *datalen,u32 memsize);

#endif

