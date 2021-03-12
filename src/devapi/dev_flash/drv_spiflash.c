/********************************Copyright ( c ) ********************************
**                  DAQU 
**                         
**                        
**
**Created By:     pengxuebin,20170324      
** Created Date:     
** Version:        
** Description:    该文件处理SPIFLASH驱动接口
  
****************************************************************************/
#include "devglobal.h"
#include "sdk/mhscpu_sdk.h"
#include "drv_spiflash.h"
#if(SPIFLASH_EXIST==1)
#ifdef DEBUG_SPIFLASH_EN
#define SPIFLASH_DEBUG(format,...)   dev_debug_printf("%s(%d):"format, __func__, __LINE__, ##__VA_ARGS__) 
#define SPIFLASH_DEBUGHEX        dev_debug_printformat
#else
#define SPIFLASH_DEBUG(...) 
#define SPIFLASH_DEBUGHEX(...)
#endif

#define SPIFLASH_SECTOR_SIZE   4096  
#define SPIFLASH_PAGE_SIZE      256
#define SPIFLASH_CHIP_SIZE      0x800000    //芯片大小2MB           

static s32 g_spiflash_fd = -1;

#define SPIFLASH_CS_PIN     GPIO_PIN_PTA7  //GPIO_PIN_PTC4
#define SPIFLASH_CS_LOW     drv_dac_irq_ctl(0);dev_gpio_set_value(SPIFLASH_CS_PIN, 0)
#define SPIFLASH_CS_HIGH    dev_gpio_set_value(SPIFLASH_CS_PIN, 1);drv_dac_irq_ctl(1)

//Flash ID
const static str_norflash_info_t spiflash_tablist[]=
{
    {0xef,0x0100,0x00001000,0x01000000,"Winbond-W25Q128BV Flash"},
    {0xc8,0x0100,0x00001000,0x00100000,"Giga-GD25Q80 Flash"},
    {0x1f,0x0100,0x00001000,0x00400000,"Atmel-AT25DQ321 Flash"},
    {0x5E,0x0100,0x00001000,0x00200000,"XTX-PN25F16B Flash"},   //芯天下
};
/****************************************************************************
**Description:        校验SPIFLASH忙状态
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
**Created by:        pengxuebin,20170324 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
static s32 drv_spiflash_check_busy(void)
{
    u8 ins;
    u8 data_buf;
    s32 ret;
    u32 timeid;

    timeid = dev_user_gettimeID();
    while(1)
    {
        SPIFLASH_CS_LOW;
        ins = SPIFLASH_Rd_STATUS;// = 0x05,
        ret = dev_spi_master_transceive_polling(SPI_DEV_FLASH, &ins, NULL, 1);
        if(ret >= 0)
        {
            data_buf = 0;
            ret = dev_spi_master_transceive_polling(SPI_DEV_FLASH, NULL, &data_buf, 1);
        }
        SPIFLASH_CS_HIGH;
        if(ret < 0)
        {
            SPIFLASH_DEBUG("spi err!(ret=%d)\r\n", ret);
            return DEVSTATUS_ERR_FAIL;
        }
        if(!(data_buf&SPIFLASH_BUSY))
        {
            break;
        }
        if(dev_user_querrytimer(timeid, 200))
        {
            //200ms超时,
            SPIFLASH_DEBUG("timerout!\r\n");
            return DEVSTATUS_ERR_FAIL;
        }
    }
    return 0;
}
/****************************************************************************
**Description:        写使能
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
**Created by:        pengxuebin,20170325 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
static s32 drv_spiflash_write_enable(void)
{
    u8 ins;
    s32 ret;

    SPIFLASH_CS_LOW;
    ins = SPIFLASH_Wr_EN;// = 0x06,
    ret = dev_spi_master_transceive_polling(SPI_DEV_FLASH, &ins, NULL, 1);
    SPIFLASH_CS_HIGH;
    if(ret < 0)
    {
        SPIFLASH_DEBUG("spi err!(ret=%d)\r\n", ret);
        return DEVSTATUS_ERR_FAIL;
    }
    return 0;
}

/****************************************************************************
**Description:        检查SPIFLASH写使能
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
**Created by:        pengxuebin,20170325 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
static s32 drv_spiflash_check_writeen(void)
{
    u8 ins;
    u8 data_buf;
    s32 ret;
    u32 timeid;

    timeid = dev_user_gettimeID();
    while(1)
    {
        SPIFLASH_CS_LOW;
        ins = SPIFLASH_Rd_STATUS;// = 0x05,
        ret = dev_spi_master_transceive_polling(SPI_DEV_FLASH, &ins, NULL, 1);
        if(ret >= 0)
        {
            data_buf = 0;
            ret = dev_spi_master_transceive_polling(SPI_DEV_FLASH, NULL, &data_buf, 1);
        }
        SPIFLASH_CS_HIGH;
        if(ret < 0)
        {
            SPIFLASH_DEBUG("spi err!(ret=%d)\r\n", ret);
            return DEVSTATUS_ERR_FAIL;
        }
        
        if((data_buf&SPIFLASH_WEL))
        {
            break;
        }
        else
        {
            SPIFLASH_CS_LOW;
            ins = SPIFLASH_Wr_EN;// = 0x06,
            ret = dev_spi_master_transceive_polling(SPI_DEV_FLASH, &ins, NULL, 1);
            SPIFLASH_CS_HIGH;
            if(ret < 0)
            {
                SPIFLASH_DEBUG("spi err!(ret=%d)\r\n", ret);
                return DEVSTATUS_ERR_FAIL;
            }
        }
        if(dev_user_querrytimer(timeid, 200))
        {
            //200ms超时,
            SPIFLASH_DEBUG("timerout!\r\n");
            return DEVSTATUS_ERR_FAIL;
        }
    }
    return 0;
}
/****************************************************************************
**Description:        写禁能
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
**Created by:        pengxuebin,20170325 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
static s32 drv_spiflash_write_disable(void)
{
    u8 ins;
    s32 ret;

    SPIFLASH_CS_LOW;
    ins = SPIFLASH_Wr_DIS;// = 0x04,
    ret = dev_spi_master_transceive_polling(SPI_DEV_FLASH, &ins, NULL, 1);
    SPIFLASH_CS_HIGH;
    if(ret < 0)
    {
        SPIFLASH_DEBUG("spi err!(ret=%d)\r\n", ret);
        return DEVSTATUS_ERR_FAIL;
    }
    return 0;
}
/****************************************************************************
**Description:        读ID
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
**Created by:        pengxuebin,20170325 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
static s32 drv_spiflash_read_JEDEC_ID(u8 *rbuf)
{
    u8 ins = 0;
    s32 ret;

    SPIFLASH_CS_LOW;
    ins = SPIFLASH_JEDEC_ID;// = 0x9F,
    ret = dev_spi_master_transceive_polling(SPI_DEV_FLASH, &ins, NULL, 1);
    if(ret >= 0)
    {
        ret = dev_spi_master_transceive_polling(SPI_DEV_FLASH, NULL, rbuf, 3);
    }
    SPIFLASH_CS_HIGH;
    if(ret < 0)
    {
        SPIFLASH_DEBUG("spi err!(ret=%d)\r\n", ret);
        return DEVSTATUS_ERR_FAIL;
    }
  #if 0  
    for(i=0; i<sizeof(spiflash_tablist); i++)
    {
        if(rbuf[0] == spiflash_tablist[i].m_manuID)
        {
            SPIFLASH_DEBUG("%s:(%02X,%02X,%02X)\r\n", spiflash_tablist[i].m_flashname, rbuf[0], rbuf[1], rbuf[2]);
            return 0;
        }
    }
    return DEVSTATUS_ERR_FAIL;
  #endif  
    return DEVSTATUS_SUCCESS;
}
s32 test_spiflash()
{
    u8 ins = 0;
    u8 wbuf[10] = {0};
    u8 rbuf[10] = {0};
    s32 ret;
    s32 i;

    SPIFLASH_CS_LOW;
    //dev_gpio_set_value(GPIO_PIN_PTA7, 0);
    ins = SPIFLASH_JEDEC_ID;// = 0x9F,
    memset(wbuf,0x55,sizeof(wbuf));
    memset(rbuf,0,sizeof(rbuf));
    //dev_user_delay_ms(10);
    ret = dev_spi_master_transceive_polling(SPI_DEV_FLASH, wbuf, rbuf, 10);

    //dev_gpio_set_value(GPIO_PIN_PTA7, 1);
    SPIFLASH_CS_HIGH;
    //dev_user_delay_ms(10);
    //SPIFLASH_CS_HIGH;
    #if 0
    if(ret < 0)
    {
        SPIFLASH_DEBUG("spi err!(ret=%d)\r\n", ret);
        return DEVSTATUS_ERR_FAIL;
    }
    else
    {
        dev_debug_printformat(NULL, rbuf, 10);
    }
    #endif
    return ret;
}
/****************************************************************************
**Description:        扇区擦除
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
**Created by:        pengxuebin,20170325 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
static s32 drv_spiflash_erase_sector(u32 addrsta)
{
    u8 cmd[4];
    s32 ret;

    cmd[0] = SPIFLASH_SECTOR_ERASE;// = 0x20,
    cmd[1] = (u8)((addrsta>>16)&0xff);
    cmd[2] = (u8)((addrsta>>8)&0xff);
    cmd[3] = (u8)((addrsta>>0)&0xff);

    ret = drv_spiflash_write_enable();
    if(ret < 0)
    {
        return ret;
    }
    ret = drv_spiflash_check_busy();
    if(ret < 0)
    {
        return ret;
    }
    SPIFLASH_CS_LOW;
    ret = dev_spi_master_transceive_polling(SPI_DEV_FLASH, cmd, NULL, 4);
    SPIFLASH_CS_HIGH;
    if(ret < 0)
    {
        SPIFLASH_DEBUG("spi err!(ret=%d)\r\n", ret);
        return DEVSTATUS_ERR_FAIL;
    }
    ret = drv_spiflash_check_busy();
    return ret;
}
/****************************************************************************
**Description:        标准模式读数据
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
**Created by:        pengxuebin,20170325 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
static s32 drv_spiflash_read_stdmode(u32 addrsta, u8 *rbuf, u32 rlen)
{
    u32 readaddr;
    u32 tmplen, totallen=0;
    u8 cmd[4];
    s32 ret;

    if(rlen == 0)
    {
        return 0;
    }
    //控制修改的地址
    if((addrsta + rlen) > SPIFLASH_CHIP_SIZE)
    {
        SPIFLASH_DEBUG("Space Over!(addr=%08X,rlen=%08X)\r\n", addrsta, rlen);
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    
    while(totallen < rlen)
    {
        tmplen = rlen - totallen;
        if(tmplen > SPIFLASH_PAGE_SIZE)
        {
            tmplen = SPIFLASH_PAGE_SIZE;
        }

        readaddr = addrsta+totallen;
        cmd[0] = SPIFLASH_Rd_DATA;//   = 0x03,
        cmd[1] = (u8)((readaddr>>16)&0xff);
        cmd[2] = (u8)((readaddr>>8)&0xff);
        cmd[3] = (u8)((readaddr>>0)&0xff);

        ret = drv_spiflash_check_busy();
        if(ret < 0)
        {
            return ret;
        }    
        
        SPIFLASH_CS_LOW;
        ret = dev_spi_master_transceive_polling(SPI_DEV_FLASH, cmd, NULL, 4);
        if(ret >= 0)
        {
            ret = dev_spi_master_transceive_polling(SPI_DEV_FLASH, NULL, &rbuf[totallen], tmplen);
        }
        SPIFLASH_CS_HIGH;
        if(ret < 0)
        {
            SPIFLASH_DEBUG("spi err!(ret=%d)\r\n", ret);
            return DEVSTATUS_ERR_FAIL;
        }

        totallen += tmplen;
    }
    return rlen;
}
/****************************************************************************
**Description:        标准模式页编程
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
**Created by:        pengxuebin,20170325 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
static s32 drv_spiflash_pageprogram_stdmode(u32 addrsta, u8 *wbuf, u32 wlen)
{
    u8 cmd[4];
    s32 ret;

    if(wlen > SPIFLASH_PAGE_SIZE)
    {
        SPIFLASH_DEBUG("Param err!(rlen=%d)\r\n", wlen);
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    if(((addrsta&(SPIFLASH_PAGE_SIZE-1))+wlen)>SPIFLASH_PAGE_SIZE)
    {
        SPIFLASH_DEBUG("Param err!(addr=%08X,%d)\r\n", addrsta, wlen);
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    cmd[0] = SPIFLASH_PAGE_PROGRAM;// = 0x02,
    cmd[1] = (u8)((addrsta>>16)&0xff);
    cmd[2] = (u8)((addrsta>>8)&0xff);
    cmd[3] = (u8)((addrsta>>0)&0xff);

    ret = drv_spiflash_write_enable();
    if(ret < 0)
    {
        return ret;
    }
    ret = drv_spiflash_check_busy();
    if(ret < 0)
    {
        return ret;
    }
    SPIFLASH_CS_LOW;
    ret = dev_spi_master_transceive_polling(SPI_DEV_FLASH, cmd, NULL, 4);
    if(ret >= 0)
    {
        ret = dev_spi_master_transceive_polling(SPI_DEV_FLASH, wbuf, NULL, wlen);
    }
    SPIFLASH_CS_HIGH;
    if(ret < 0)
    {
        SPIFLASH_DEBUG("spi err!(ret=%d)\r\n", ret);
        return DEVSTATUS_ERR_FAIL;
    }
    ret = drv_spiflash_check_busy();
    return ret;
}
/****************************************************************************
**Description:        标准模式编程
**Input parameters:    
**Output parameters: 
**Returned value:
                    < 0:失败
                    >=0:成功
**Created by:        pengxuebin,20170325 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
static s32 drv_spiflash_program_stdmod(u32 addrsta, u8 *wbuf, u32 wlen)
{
    u32 writeaddr, tmplen, total_len;
    s32 i;
    s32 ret;
    u8  tmp[SPIFLASH_PAGE_SIZE];

    if(wlen == 0)
    {
        return 0;
    }
    //控制修改的地址
    if((addrsta + wlen) > SPIFLASH_CHIP_SIZE)
    {
        SPIFLASH_DEBUG("Space Over!(addr=%08X,wlen=%08X)\r\n", addrsta, wlen);
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    
    total_len = 0;
    while(total_len < wlen)
    {
        tmplen = wlen-total_len;
        if(tmplen > SPIFLASH_PAGE_SIZE)
        {
            tmplen = SPIFLASH_PAGE_SIZE;
        }
        i = (writeaddr&(SPIFLASH_PAGE_SIZE-1));
        if((i+tmplen) > SPIFLASH_PAGE_SIZE)
        {
            tmplen = SPIFLASH_PAGE_SIZE - i;
        }
      #if 0  
        //查空，确保指定空间能写
        ret = drv_spiflash_read_stdmode(addrsta+total_len, tmp, tmplen);
        if(ret < 0)
        {
            return ret;
        }
        
        for(i=0; i<tmplen; i++)
        {
            if(tmp[i] != 0xFF)
            {
                SPIFLASH_DEBUG("BlankCheckErr!(addr=%08X,rd=%02X,wd=%02X)\r\n", addrsta+total_len+i, tmp[i], wbuf[total_len+i]);
                return DEVSTATUS_ERR_FAIL;
            }
        }
      #endif  
        //写入
        ret = drv_spiflash_pageprogram_stdmode(addrsta+total_len, &wbuf[total_len], tmplen);
        if(ret < 0)
        {
            return ret;
        }
        //读出来校验
        ret = drv_spiflash_read_stdmode(addrsta+total_len, tmp, tmplen);
        if(ret < 0)
        {
            return ret;
        }
        //比较结果
        for(i=0; i<tmplen; i++)
        {
            if(tmp[i] != wbuf[total_len+i])
            {
                SPIFLASH_DEBUG("CheckErr!(addr=%08X,rd=%02X,wd=%02X)\r\n", addrsta+total_len+i, tmp[i], wbuf[total_len+i]);
                return DEVSTATUS_ERR_FAIL;
            }
        }

        total_len += tmplen;
    }
    return wlen;
}
/****************************************************************************
**Description:        打开SPIFLASH设备
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170324 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_spiflash_open(void)
{
    if(g_spiflash_fd < 0)
    {
        //由驱动控制片选,配置为GPIO,输出1
        dev_gpio_config_mux(SPIFLASH_CS_PIN, MUX_CONFIG_ALT1);   //配置为GPIO口
        dev_gpio_set_pad(SPIFLASH_CS_PIN, PAD_CTL_PULL_UP);
        dev_gpio_direction_output(SPIFLASH_CS_PIN, 1);
    
        g_spiflash_fd = dev_spi_open(SPI_DEV_FLASH);
        if(g_spiflash_fd < 0)
        {
            SPIFLASH_DEBUG("Open Err!(ret=%d)\r\n", g_spiflash_fd);
        }
    }
    return g_spiflash_fd;
}
/****************************************************************************
**Description:        关闭SPIFLASH设备
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
**Created by:        pengxuebin,20170324 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_spiflash_close(void)
{
    dev_spi_close(SPI_DEV_FLASH);
    g_spiflash_fd = -1;
    return 0;
}
/****************************************************************************
**Description:        读spiflash ID
**Input parameters:    
**Output parameters: 
**Returned value:
                    < 0:失败
                    >=0:成功
**Created by:        pengxuebin,20170405 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_spiflash_read_ID(u8 *rbuf)
{
    s32 ret;

    ret = drv_spiflash_open();
    if(ret < 0)
    {
        return ret;
    }
    return drv_spiflash_read_JEDEC_ID(rbuf);
}
/****************************************************************************
**Description:        读spiflash ID
**Input parameters:    
**Output parameters: 
**Returned value:
                    < 0:失败
                    >=0:成功
**Created by:        pengxuebin,20170405 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_spiflash_check_ID(void)
{
    u8 tmp[3];
    s32 i;
    s32 ret;

    ret = drv_spiflash_read_ID(tmp);
    if(ret < 0)
    {
        return ret;
    }
    
    for(i=0; i<sizeof(spiflash_tablist); i++)
    {
        if(tmp[0] == spiflash_tablist[i].m_manuID)
        {
            SPIFLASH_DEBUG("%s:(%02X,%02X,%02X)\r\n", spiflash_tablist[i].m_flashname, tmp[0], tmp[1], tmp[2]);
            return DEVSTATUS_SUCCESS;
        }
    }
    return DEVSTATUS_ERR_FAIL;
}
/****************************************************************************
**Description:        spiflash读数据
**Input parameters:    
**Output parameters: 
**Returned value:
                    < 0:失败
                    >=0:成功
**Created by:        pengxuebin,20170405 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_spiflash_read(u32 addrsta, u8 *rbuf, u32 rlen)
{
    s32 ret;

    ret = drv_spiflash_open();
    if(ret < 0)
    {
        return ret;
    }
    return drv_spiflash_read_stdmode(addrsta, rbuf, rlen);
}
/****************************************************************************
**Description:        标准模式编程
**Input parameters:    
**Output parameters: 
**Returned value:
                    < 0:失败
                    >=0:成功
**Created by:        pengxuebin,20170405 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_spiflash_write(u32 addrsta, u8 *wbuf, u32 wlen)
{
    s32 ret;
    ret = drv_spiflash_open();
    if(ret < 0)
    {
        return ret;
    }
    return drv_spiflash_program_stdmod(addrsta, wbuf, wlen);
}

/****************************************************************************
**Description:        spiflash擦除块
**Input parameters:    
                    addrsta:起始地址
**Output parameters: 
**Returned value:
                    < 0:失败
                    = 0:成功
**Created by:        pengxuebin,20170405 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_spiflash_erase(u32 addrsta)
{
    s32 ret;
    u32 addr;

    ret = drv_spiflash_open();
    if(ret < 0)
    {
        return ret;
    }
    if(addrsta >= SPIFLASH_CHIP_SIZE)
    {
        SPIFLASH_DEBUG("Space Over!(addrsta=%08X)\r\n", addrsta);
        return DEVSTATUS_ERR_PARAM_ERR;
    }

    addr = addrsta&(~(SPIFLASH_SECTOR_SIZE-1));
    ret = drv_spiflash_erase_sector(addr);
    if(ret < 0)
    {
        return ret;
    }
    return DEVSTATUS_SUCCESS;
}
#if 0
/****************************************************************************
**Description:        spiflash擦除块
**Input parameters:    
                    addrsta:起始地址
                    addrend:结束地址,
                    擦除从起始地址到结束地址所在块，如addrend&(SPIFLASH_SECTOR_SIZE-1)==0,
                    则该块不被擦除，
                    如addrsta= 0x4100,addrend=0x5000,则只擦除0x4000~0x4FFF这块;
                    如addrsta= 0x4100,addrend=0x5001,则擦除0x4000~0X5FFF这两块.
                    
**Output parameters: 
**Returned value:
                    < 0:失败
                    = 0:成功
**Created by:        pengxuebin,20170405 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_spiflash_erase(u32 addrsta, u32 addrend)
{
    s32 ret;
    u32 addr;

    ret = drv_spiflash_open();
    if(ret < 0)
    {
        return ret;
    }
    if((addrsta >= SPIFLASH_CHIP_SIZE)||(addrend>=SPIFLASH_CHIP_SIZE))
    {
        SPIFLASH_DEBUG("Space Over!(addrsta=%08X,addrend=%08X)\r\n", addrsta, addrend);
        return DEVSTATUS_ERR_PARAM_ERR;
    }

    addr = addrsta&(~(SPIFLASH_SECTOR_SIZE-1));
    while(addr < addrend)
    {
        ret = drv_spiflash_erase_sector(addr);
        if(ret < 0)
        {
            
            return ret;
        }
        addr += SPIFLASH_SECTOR_SIZE;
    }
    return DEVSTATUS_SUCCESS;
}
#endif
/****************************************************************************
**Description:        spiflash写数据
**Input parameters:    
**Output parameters: 
**Returned value:
                    < 0:失败
                    >=0:成功
**Created by:        pengxuebin,20170405 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_spiflash_writewitherase(u32 addrsta, u8 *wbuf, u32 wlen)
{
    s32 ret;
    u32 offset;
    u8 *FSW;
    u32 flashblocksize, writelen, writepos;
    u32 addr=0;
    u8  neederaseflg = 0;
    u32 i;
    u8 cmpvalue[SPIFLASH_PAGE_SIZE];
    u8 retrycnt = 0;

    ret = drv_spiflash_open();
    if(ret < 0)
    {
        return ret;
    }
    if(wlen == 0)
    {
        return 0;
    }
    if((addrsta + wlen) >= SPIFLASH_CHIP_SIZE)
    {
        SPIFLASH_DEBUG("Space Over!(addr=%08X,wlen=%08X)\r\n", addrsta, wlen);
        return DEVSTATUS_ERR_PARAM_ERR;
    }

    addr = addrsta;
    flashblocksize = SPIFLASH_SECTOR_SIZE;

    offset = addr&(flashblocksize-1);
    addr = addr&(~(flashblocksize-1));

    //动态申请SECTOR空间
    FSW = (u8 *)k_malloc(flashblocksize);
    if(FSW == NULL)
    {
        SPIFLASH_DEBUG("malloc ERR!(%d)\r\n", flashblocksize);
        return DEVSTATUS_ERR_FAIL;
    }
    
    for(writepos=0; writepos<wlen;)
    {
        if((flashblocksize-offset) > (wlen-writepos))
        {
            writelen = wlen-writepos;
        }
        else
        {
            writelen = flashblocksize - offset;
        }

        //要保证flash块完整
        //if(writelen <= flashblocksize)
        //读出该SECTOR数据
        ret = drv_spiflash_read(addr, FSW, flashblocksize);
        if(ret < 0)
        {
            SPIFLASH_DEBUG("spiflash_read err!(ret=%d,addr=x%08X,len=%d)\r\n", ret, addrsta, flashblocksize);
            k_free(FSW);
            return ret;
        }

        //检查是否需要写入
        if(0 == memcmp(&FSW[offset], &wbuf[writepos], writelen))
        {
            //读出的数据和需要写入的数据一致
          #if 1
            SPIFLASH_DEBUG("SectorWrite OK!(addr=x%d)\r\n", addr);
          #endif
            writepos += writelen;
            addr += flashblocksize;
            offset = 0;
            continue;
        }

        //检查是否要进行擦除FLASH的动作
        neederaseflg = 0;
        for(i=0; i<flashblocksize; i++)
        {
            if(FSW[i] != 0xFF)
            {
                neederaseflg = 1;
                break;
            }
        }

        if(neederaseflg)
        {
            ret = drv_spiflash_erase_sector(addr);
            if(ret < 0)
            {
                SPIFLASH_DEBUG("spiflash_erase_sector err!(ret=%d,addr=x%08X)\r\n", ret, addrsta);
                k_free(FSW);
                return ret;    
            }
        }

        //填充需要写入的数据
        memcpy(&FSW[offset], &wbuf[writepos], writelen);
        //按页写数据
        for(i=0; i<flashblocksize; i+=SPIFLASH_PAGE_SIZE)
        {
            ret = drv_spiflash_pageprogram_stdmode(addr+i, &FSW[i], SPIFLASH_PAGE_SIZE);
            if(ret < 0)
            {
                SPIFLASH_DEBUG("spiflash_pageprogram_stdmode err!(ret=%d,addr=x%08X)\r\n", ret, addrsta);
                k_free(FSW);
                return ret;   
            }
            //校验
            memset(cmpvalue, 0, sizeof(cmpvalue));
            ret = drv_spiflash_read(addr+i, cmpvalue, SPIFLASH_PAGE_SIZE);
            if(ret < 0)
            {
                SPIFLASH_DEBUG("spiflash_read err!(ret=%d,addr=x%08X)\r\n", ret, addrsta+i);
                k_free(FSW);
                return ret;
            }
            if(memcmp(&FSW[i], cmpvalue, SPIFLASH_PAGE_SIZE))
            {
                retrycnt++;
                if(retrycnt < 3)
                {
                    continue;
                }
                k_free(FSW);
                SPIFLASH_DEBUG("Data Check ERR!\r\n");
                return DEVSTATUS_ERR_FAIL;
            }  
        }
      #if 1
        SPIFLASH_DEBUG("SectorWrite OK!(addr=x%d)\r\n", addr);
      #endif
        retrycnt = 0;
        writepos += writelen;
        addr += flashblocksize;
        offset = 0;
    }  
    k_free(FSW);
  #if 0  
    SPIFLASH_DEBUG("Write ok!\r\n");
  #endif
    return DEVSTATUS_SUCCESS;
}
#endif


