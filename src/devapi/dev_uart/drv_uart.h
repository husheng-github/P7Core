#ifndef __DRV_UART_H
#define __DRV_UART_H

#define DRVUARTNUM_MAX     PORT_UART_PHY_MAX
typedef struct _str_UART_PARAM
{
    iomux_pin_name_t m_txpin;
    iomux_pin_name_t m_rxpin;
    iomux_pin_name_t m_rtspin;
    iomux_pin_name_t m_ctspin;
    u32 m_baud;
    //各管脚的ALT功能选择
    u8 m_txpinalt;      
    u8 m_rxpinalt;
    u8 m_rtspinalt;
    u8 m_ctspinalt;
}str_uart_param_t;


void drv_com_init(void);
s32 drv_com_setbaud(s32 nCom,  s32 baud, s32 databits, u32 parity, s32 stopbits, s32 flowctl);
s32 drv_com_open(s32 nCom, s32 baud, s32 databits, u32 parity, s32 stopbits, s32 flowctl);
s32 drv_com_close(s32 nCom);
s32 drv_com_write(s32 nCom, u8 *wbuf, u16 wlen);
s32 drv_com_read(s32 nCom, u8 *rbuf, u16 rlen);
s32 drv_com_tcdrain(s32 nCom);
s32 drv_com_flush(s32 nCom);
s32 drv_com_get_baud(u8 nCom, u32 *bps);

void drv_com_writedirect(u8 data);

#endif
