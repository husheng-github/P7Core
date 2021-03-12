#ifndef __DEV_SPI_H
#define __DEV_SPI_H


typedef enum _SPI_DEVICE
{
  #if 0  
    SPI_DEV_RF       = 0,   //�ǽ��豸
    SPI_DEV_FLASH    = 1,   //SPIFLASH�豸
    SPI_DEV_LCD      = 2,   //LCD�豸
    SPI_DEV_PRINT    = 3,   //��ӡ�豸
  #endif  
    SPI_DEV_RF       = 0,   //�ǽ��豸
    SPI_DEV_FLASH      = 1, 
    SPI_DEV_PRINT    = 2,   //��ӡ�豸
    SPI_DEV_AP       = 3,   //��APͨѶ
    SPI_DEV_MAX      = 4,
}SPI_DEVICE_t;


void dev_spi_init(void);
s32 dev_spi_open(SPI_DEVICE_t spidev);
s32 dev_spi_close(SPI_DEVICE_t spidev);
s32 dev_spi_master_transceive_polling(SPI_DEVICE_t spidev, void* txbuf, void* rxbuf, s32 le);
s32 dev_spi_slave_write(u8 *wbuf, u32 wlen);
s32 dev_spi_slave_read(u8 *rbuf, u32 rlen);
s32 dev_spi_slave_tcdrain(void);

#endif

