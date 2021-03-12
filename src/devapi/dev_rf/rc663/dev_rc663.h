#ifndef __DEV_MFRC663_H
#define __DEV_MFRC663_H

#if 0
#define PCD_DEBUG 
#undef  PCD_DEBUG

#ifdef PCD_DEBUG
#define PCD_PRINTF   
#else
#define PCD_PRINTF
#endif
#endif

#define DEV_RC663_WATERLEVEL   128




s32 dev_rc663_read_version(u8 *ver);
void dev_rc663_reset(s32 _ms);
s32 dev_rc663_ctl_crc(u8 mask, u8 mod);
s8 dev_rc663_select_type(u8 type);
void dev_rc663_set_timeout(u8 mode, u32 timer);
void dev_rc663_wait_etu(u32 etu);
//void dev_rc663_wait_netu(u32 netu);
s8 dev_rc663_execute_cmd(MfCmdInfo *p_mfcmdinfo, u8 *wbuf, u8 *rbuf);
s32  dev_rc663_mifare_auth(u8 key_type, u8 *key, u8 *snr, u8 block);
s8 dev_rc663_softpowerdown(void);
s8 dev_rc663_softpoweron(void);
s8 dev_rc663_send_reqa(u8 req_code, u8 *atq);
s8 dev_rc663_halt_piccA(void);
s8 dev_rc663_req_piccB(u8 afi, u8 N, u8 req_code, u8 *atqB, u8 *atqBlen);
s8 dev_rc663_selectanticoll_picc(u8 grade, u8 grademax, u8 bitcount, u8 *snr, u8 *sak);
s8 dev_rc663_send_rats_piccA(u8 cid, u8 *rbuf, u16 *rlen);
s8 dev_rc663_attrib(u8 cid, u8 *pupi, u8 brTx, u8 brRx, u8 param3, u32 fwt);
s8 dev_rc663_exchange_piccA(u8 *wbuf, u16 wlen, u8 *rbuf, u16 *rlen, u32 etu, u8 wtx);

#endif

