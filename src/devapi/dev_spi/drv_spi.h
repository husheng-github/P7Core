#ifndef __DRV_SPI_H
#define __DRV_SPI_H

#ifdef DEBUG_SPI_EN
#define SPI_DEBUG(format,...)    dev_debug_printf("%s(%d):"format, __func__, __LINE__, ##__VA_ARGS__)
#define SPI_DEBUGHEX             dev_debug_printformat
#else
#define SPI_DEBUG(...) 
#define SPI_DEBUGHEX(...)
#endif

#define    SPI_MODE_CPHA    0x01            /* clock phase */
#define    SPI_MODE_CPOL    0x02            /* clock polarity */
#define    SPI_MODE_0    (0|0)            /* (original MicroWire) */
#define    SPI_MODE_1    (0|SPI_MODE_CPHA)
#define    SPI_MODE_2    (SPI_MODE_CPOL|0)
#define    SPI_MODE_3    (SPI_MODE_CPOL|SPI_MODE_CPHA)
#if 0
#define    SPI_CS_HIGH    0x04            /* chipselect active high? */
#define    SPI_LSB_FIRST    0x08            /* per-word bits-on-wire */
#define    SPI_3WIRE    0x10            /* SI/SO signals shared */
#define    SPI_LOOP    0x20            /* loopback mode */
#define    SPI_NO_CS    0x40            /* 1 dev/bus, no chipselect */
#define    SPI_READY    0x80            /* slave pulls low to pause */
#endif
typedef enum{
    SPI_PORT0 = 0,
    SPI_PORT1 = 1,
    SPI_PORT2 = 2,
    SPI_PORT3 = 3,
//    SPI_PORT4 = 4,
    SPI_PORT_MAX = 4,
}SPI_PORT_t;
#define SPIMODE_MASTER      1
#define SPIMODE_SLAVE       0

#define SPI_DIRECTION_2L_TxRx       0
#define SPI_DIRECTION_1L_Tx            1
#define SPI_DIRECTION_1L_Rx            2
#define SPI_DIRECTION_EEPROM_Rx     3

#define SPI_CSMOD_HD0          0    //硬件片选
#define SPI_CSMOD_GPIO         5    //GPIO片选
#define SPI_CSMOD_NONE         10   //没有片选

typedef struct _str_SPI_PARAM
{
    u32 m_speed;            //SPI速率
    iomux_pin_name_t m_gpiopin;     //GPIO管脚做片选
    u8  m_port;             //SPI端口号
    u8  m_master;           //SPI主从关系   0:主, 1:从
    u8  m_direction;        //
    u8  m_mode;             //SPI模式，(CPHA,CPOL)
    u8  m_cs;               //SPI片选
    u8  m_bits_per_word;    //每个字节的位数
}str_spi_param_t;


void drv_spi_init(void);
s32 drv_spi_open(SPI_DEVICE_t spidev);
s32 drv_spi_close(SPI_DEVICE_t spidev);
s32 drv_spi_master_transceive_polling(SPI_DEVICE_t spidev, void* txbuf, void* rxbuf, s32 le);
s32 drv_spi_slave_write(u8 *wbuf, u16 wlen);
s32 drv_spi_slave_read(u8 *rbuf, u16 rlen);
s32 drv_spi_slave_gettxstatus(u32 *txspacelen);

#endif

