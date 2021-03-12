#ifndef __DEV_PN512_H
#define __DEV_PN512_H


#define PCD_CHIP_TYPE_PN512     0
#define PCD_CHIP_TYPE_MH1608    1   //Õ×Ñ¶
#define PCD_CHIP_TYPE_ZH3801    2  //¹úÃñ
#define PCD_CHIP_TYPE_F17550    3  //¸´µ©Î¢

#define DEV_PN512_WATERLEVEL     32

//extern u8 g_pn512_ver;
//extern u8 g_rf_special_B_flag; 
extern u8 g_rf_emd_flg;
//extern u8 g_rf_morecard_cnt;
//void dev_pn512_set_debug_flg(u8 flg);

void dev_pn512_export_init(void);
void dev_pn512_export_deinit(void);
void dev_pn512_export_ctl(u8 num, u8 flg);
s32 dev_pn512_read_version(u8 *ver);
void dev_pn512_reset(s32 _ms);
s32 dev_pn512_ctl_crc(u8 mask, u8 mod);
s8 dev_pn512_select_type(u8 type);
void dev_pn512_set_timeout(u8 mode, u16 timer);
void dev_pn512_wait_etu(u32 etu);
s8 dev_pn512_execute_cmd(MfCmdInfo *p_mfcmdinfo, u8 *wbuf, u8 *rbuf);
s32 dev_pn512_mifare_auth(u8 key_type, u8 *key, u8 *snr, u8 block);
//void dev_pn512_ctl_rxcrc(u8 type);
void dev_pn512_wait_netu(u16 netu);
s8 dev_pn512_softpowerdown(void);
s32 dev_pn512_softpoweron(void);
s8 dev_pn512_send_reqa(u8 req_code, u8 *atq);
s8 dev_pn512_halt_piccA(void);
s8 dev_pn512_selectanticoll_picc(u8 grade, u8 grademax, u8 bitcount, u8 *snr, u8 *sak);
s8 dev_pn512_req_piccB(u8 afi, u8 N, u8 req_code, u8 *atqB, u8 *atqBlen);
s8 dev_pn512_send_rats_piccA(u8 cid, u8 *rbuf, u16 *rlen);
s8 dev_pn512_attrib(u8 cid, u8 *pupi, u8 brTx, u8 brRx, u8 param3, u32 fwt);
s8 dev_pn512_exchange_piccA(u8 *wbuf, u16 wlen, u8 *rbuf, u16 *rlen, u32 etu, u8 wtx);
s8 dev_hm1608_execute_cmd_send(u8 *wbuf, u32 wlen);
#endif

