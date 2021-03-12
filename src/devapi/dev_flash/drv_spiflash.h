#ifndef __DRV_SPIFLASH_H
#define __DRV_SPIFLASH_H
typedef struct _str_norflash_info
{
    u8  m_manuID;
    u32 m_pagesize;
    u32 m_sectorsize;
    u32 m_chipsize;
    u8 *m_flashname;
}str_norflash_info_t;


/*Flash Command*/
typedef enum
{
    SPIFLASH_Wr_EN  = 0x06,
    SPIFLASH_Wr_DIS = 0x04, 
    SPIFLASH_Rd_STATUS = 0x05,
    SPIFLASH_Wr_STATUS = 0x01,
    SPIFLASH_Rd_DATA   = 0x03,
    SPIFLASH_Rd_FAST   = 0x0B,
    SPIFLASH_Rd_FAST_DUAL_OUT = 0x3B,
    SPIFLASH_PAGE_PROGRAM = 0x02,
    SPIFLASH_BLOCKERASE = 0xD8,
    SPIFLASH_HALF_BLOCKERASE = 0x52,
    SPIFLASH_SECTOR_ERASE = 0x20,
    SPIFLASH_CHIPERASE = 0x60,
    SPIFLASH_POWERDOWN = 0xB9,
    SPIFLASH_RELEASE_POWERDOWN = 0xAB,
    SPIFLASH_MANU_ID = 0x90,
    SPIFLASH_JEDEC_ID = 0x9F,
    
}SPIFLASH_CMD_t;


//Register1
#define SPIFLASH_BUSY        (1<<0)
#define    SPIFLASH_WEL        (1<<1)/*write enable latch*/

//Register2
#define SPIFLASH_REG2_QE    (1<<1)


//¶ÔÍâÉùÃ÷
s32 drv_spiflash_open(void);
s32 drv_spiflash_close(void);
s32 drv_spiflash_read_ID(u8 *rbuf);
s32 drv_spiflash_check_ID(void);
s32 drv_spiflash_read(u32 addrsta, u8 *rbuf, u32 rlen);
s32 drv_spiflash_write(u32 addrsta, u8 *wbuf, u32 wlen);
s32 drv_spiflash_erase(u32 addrsta);
s32 drv_spiflash_writewitherase(u32 addrsta, u8 *wbuf, u32 wlen);


#endif

