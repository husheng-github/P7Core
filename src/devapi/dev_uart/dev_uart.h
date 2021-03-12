#ifndef __DEV_UART_H
#define __DEV_UART_H



//int SER_PutChar (int ch);
//int fputc(int c, FILE *f);
//u8 drv_com_fgetc(void);
//void UART_Configuration(void);
//void drv_com_setuart(s32 com);
//s32 drv_com_getnum(void);
//s32 drv_com_open(s32 nCom);

typedef enum _ComPortNo
{
    PORT_UART0_NO = 0,
    PORT_UART1_NO = 1,
    PORT_UART2_NO = 2,
    PORT_UART3_NO = 3,
    
    PORT_HID_NO   = 4,
    PORT_CDCD_NO  = 5,
    PORT_SPI_NO   = 6,
    PORT_PRINT_NO = 7,
    
    PORT_NO_MAX   = 8,
    PORT_UART_PHY_MAX = 4,
}ComPortNo_t;

void dev_com_init(void);
s32 dev_com_setbaud(s32 nCom,  s32 baud, s32 databits, u32 parity, s32 stopbits, s32 flowctl);
s32 dev_com_open(s32 nCom, s32 baud, s32 databits, u32 parity, s32 stopbits, s32 flowctl);
s32 dev_com_close(s32 nCom);
s32 dev_com_write(s32 nCom, u8 *wbuf, u16 wlen);
s32 dev_com_read(s32 nCom, u8 *rbuf, u16 rlen);
s32 dev_com_tcdrain(s32 nCom);
s32 dev_com_flush(s32 nCom);
s32 dev_com_get_baud(u32 nCom, u32 *baud);

extern void dev_com_writedirect(u8 data);
#endif

