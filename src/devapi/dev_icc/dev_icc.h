#ifndef __DEV_ICC_H
#define __DEV_ICC_H

typedef enum _ICC_SLOT
{
    ICC_SLOT_ICCARD = 0,
    ICC_SLOT_PSAM1 = 1,
    ICC_SLOT_PSAM2 = 2,
//    ICC_SLOT_PSAM3 = 3,
    ICC_SLOT_MAX   = 3,
}icc_slot_t;

void dev_icc_init(void);
s32 dev_icc_open(s32 nslot);
s32 dev_icc_close(s32 nslot);
s32 dev_icc_poweroff(s32 nslot);
s32 dev_icc_getstatus(s32 nslot);
s32 dev_icc_reset(s32 nslot, u8 *lpAtr);
s32 dev_icc_exchange_apdu(s32 nslot, const u8* lpCApdu, u32 nCApduLen, u8*lpRApdu, u32* lpRApduLen, u32 nRApduSize);

#endif

