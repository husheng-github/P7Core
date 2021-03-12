#ifndef __DEV_RF_IO_H
#define __DEV_RF_IO_H

#ifdef DEBUG_RF_EN
#define RF_DEBUG(format,...)    dev_debug_printf("%s(%d):"format, __func__, __LINE__, ##__VA_ARGS__)
#define RF_DEBUGHEX             dev_debug_printformat
#else
#define RF_DEBUG(...) 
#define RF_DEBUGHEX(...)
#endif
#define delayms    rf_delayms  //dev_delay_ms // dev_user_delay_ms //

#define RF_SPI_FIFOMAX   64



s32 dev_pn512_write_reg(u8 regaddr, u8 regval);
s32 dev_pn512_read_reg(u8 regaddr, u8 *regval);
s32 dev_pn512_modify_reg(u8 regaddr, u8 mode, u8 maskbyte);
s32 dev_pn512_write_fifo(u8 *wbuf, s32 wlen);
s32 dev_pn512_read_fifo(u8 *rbuf, s32 rlen);
//------------------------------------------------
s32 dev_rc663_write_reg(u8 regaddr, u8 regval);
s32 dev_rc663_write_serial(u8 addrsta, u8 *wbuf, u32 wlen);
s32 dev_rc663_read_reg(u8 regaddr, u8 *regval);
s32 dev_rc663_modify_reg(u8 regaddr, u8 mode, u8 maskbyte);
s32 dev_rc663_write_fifo(u8 *wbuf, s32 wlen);
s32 dev_rc663_read_fifo(u8 *rbuf, s32 rlen);
void dev_rf_sleep_ctl(u8 flg);

void dev_rf_io_init(void);
void dev_rf_5v_ctl(u8 flg);
void dev_rf_rst_ctl(u8 flg);
s32 dev_rf_support_judge(void);
u8 dev_rf_int_get(void);
s32 drv_spi_rfcomm(u8 *snd, u8 *rsv, s32 size);
void PrintFormat(u8 *src, s32 srclen);

#endif

