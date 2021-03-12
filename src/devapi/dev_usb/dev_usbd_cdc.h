#ifndef __DEV_USBD_CDC_H
#define __DEV_USBD_CDC_H


#define USB_DEV_PRINT_ONLY      0
#define USB_DEV_PRINT_CDC       1

s32 dev_usbd_cdc_open(u8 type);
s32 dev_usbd_cdc_close(void);
void dev_usbd_cdc_settimerid(void);
s32 dev_usbd_cdc_write(u8 *wbuf, u16 wlen);
s32 dev_usbd_cdc_read(u8 *rbuf, u16 rlen);
s32 dev_usbd_cdc_tcdrain(void);
s32 dev_usbd_cdc_flush(void);
s32 dev_usbd_cdc_get_status(void);
void usbd_cdc_test(void);
s32 dev_usbd_print_write(u8 *wbuf, u16 wlen);
s32 dev_usbd_print_read(u8 *rbuf, u16 rlen);
s32 dev_usbd_print_flush(void);
#endif
