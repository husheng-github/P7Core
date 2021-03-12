/********************************Copyright ( c ) ********************************
**                  DAQU 
**                         
**                        
**
**Created By:     pengxuebin,20191017      
** Created Date:     
** Version:        
** Description:    该文件处理QSPIFLASH驱动接口
  
****************************************************************************/
#include "devglobal.h"
#include "sdk/mhscpu_sdk.h"
#include "drv_qspiflash.h"
#if(QSPIFLASH_EXIST==1)
#define QSPI_PROG_USEDMA

#define QSPI_INTERNAL     1     //内部QSPI FLASH方式(如1903S等)

//#define ROM_QSPI_EraseSector    (*((uint8_t (*)(QSPI_CommandTypeDef *, uint32_t))(*(uint32_t *)0x0024)))
//#define ROM_QSPI_ProgramPage    (*((uint8_t (*)(QSPI_CommandTypeDef *, DMA_TypeDef *, uint32_t, uint32_t, uint8_t *))(*(uint32_t *)0x0028)))   

//#define PFlashSectorSize        4096
//#define FLASH_ONESECTOR_PAGE_NUM   16

#define QSPIFLASH_ADDRSTA       0x1000000
#define QSPIFLASH_SECTOR_SIZE       0x1000
#define QSPIFLASH_PAGE_SIZE     X25Q_PAGE_SIZE   //0x100
#define QSPIFLASH_SECTOR_PAGE_NUM    (QSPIFLASH_SECTOR_SIZE/QSPIFLASH_PAGE_SIZE)//16
#if(QSPI_INTERNAL==1)
#define QSPIFLASH_SPACE_MAX    0x100000     //1MB  (1903S)
#else
#define QSPIFLASH_SPACE_MAX    0x800000     //8MB  (1903 外置FLASH)
#endif

static s32 g_qspiflash_fd = -1;
static s32 g_qspiflash_chiptype = -1;

#if 0
void drv_qspiflash_printinfo(u8 num)
{
    
}
#endif


s32 drv_qspiflash_checktype(void)
{
    QSPI_CommandTypeDef l_cmd;
    u32 chip_type;
    
    if(g_qspiflash_chiptype < 0)
    {    
#if(QSPI_INTERNAL==1)
        //内部FLASH
//        QSPI_Init(NULL);
        g_qspiflash_chiptype = QSPI_SUPPORT_CHIP_JEDEC_ID_GD;   //指定一个1903支持的FLASH型号
#else
        ROM_QSPI_ReleaseDeepPowerDown(NULL);
        chip_type = ROM_QSPI_ReadID(NULL);
        if(0xFFFFFF == chip_type)
        {
            l_cmd.Instruction = 0x9F;
            l_cmd.BusMode = QSPI_BUSMODE_444;
            l_cmd.CmdFormat = QSPI_CMDFORMAT_CMD8_RREG24;
            chip_type = ROM_QSPI_ReadID(&l_cmd);
        }
        g_qspiflash_chiptype = (chip_type>>16);
        switch(g_qspiflash_chiptype)
        {
        case QSPI_SUPPORT_CHIP_JEDEC_ID_MICR:      //MICRON
            QSPIFLASH_DEBUG("QSPI Flash chip is MICRON\n");

            //burst
            l_cmd.Instruction = 0x81;
            l_cmd.BusMode = QSPI_BUSMODE_111;
            l_cmd.CmdFormat = QSPI_CMDFORMAT_CMD8_WREG8;
            ROM_QSPI_WriteParam(&l_cmd, 0xF1);

            CACHE->CACHE_CONFIG = (CACHE->CACHE_CONFIG & 0xFF00FFFF) | (0xA5 << 16);				

            l_cmd.Instruction = 0x61;
            l_cmd.BusMode = QSPI_BUSMODE_111;
            l_cmd.CmdFormat = QSPI_CMDFORMAT_CMD8_WREG16;	
            ROM_QSPI_WriteParam(&l_cmd, 0x4F);		
            break;

        case QSPI_SUPPORT_CHIP_JEDEC_ID_MXIC:    //MXIC
            QSPIFLASH_DEBUG("QSPI Flash chip is MXIC\n");

            l_cmd.Instruction = 0xC0;
            l_cmd.BusMode = QSPI_BUSMODE_111;
            l_cmd.CmdFormat = QSPI_CMDFORMAT_CMD8_WREG8;
            ROM_QSPI_WriteParam(&l_cmd, 0x02);       ///< 32Bytes

            CACHE->CACHE_CONFIG = (CACHE->CACHE_CONFIG & 0xFF00FFFF) | (0xA5 << 16);

            l_cmd.Instruction = 0x01;
            l_cmd.BusMode = QSPI_BUSMODE_111;
            l_cmd.CmdFormat = QSPI_CMDFORMAT_CMD8_WREG16;
            ROM_QSPI_WriteParam(&l_cmd, 0x0042);	    ///< QE WEL			
            break;

        case QSPI_SUPPORT_CHIP_JEDEC_ID_WD:     //Winbond
            QSPIFLASH_DEBUG("QSPI Flash chip is Winbond\n");

            l_cmd.Instruction = WRITE_STATUS_REG1_CMD;
            l_cmd.BusMode = QSPI_BUSMODE_111;
            l_cmd.CmdFormat = QSPI_CMDFORMAT_CMD8_WREG16;
            ROM_QSPI_WriteParam(&l_cmd, 0x0202);

            l_cmd.Instruction = SET_BURST_WITH_WRAP;
            l_cmd.BusMode = QSPI_BUSMODE_144;
            l_cmd.CmdFormat = QSPI_CMDFORMAT_CMD8_DMY24_WREG8;
            ROM_QSPI_WriteParam(&l_cmd, 0x40);

            CACHE->CACHE_CONFIG = (CACHE->CACHE_CONFIG & 0xFF00FFFF) | (0xA5 << 16);	
            break;

        case QSPI_SUPPORT_CHIP_JEDEC_ID_GD:      ///< GD
            QSPIFLASH_DEBUG("QSPI Flash chip is GD\n");

            l_cmd.Instruction = WRITE_STATUS_REG1_CMD;         ///< Write Status Register
            l_cmd.BusMode = QSPI_BUSMODE_111;
            l_cmd.CmdFormat = QSPI_CMDFORMAT_CMD8_WREG16;
            ROM_QSPI_WriteParam(&l_cmd, 0x0200);  ///< QE(low byte first, MSB first)

            l_cmd.Instruction = SET_BURST_WITH_WRAP;         ///< Set Burst with Wrap
            l_cmd.BusMode = QSPI_BUSMODE_144;
            l_cmd.CmdFormat = QSPI_CMDFORMAT_CMD8_DMY24_WREG8;
            ROM_QSPI_WriteParam(&l_cmd, 0x40);  ///< Wrap Length:32Bytes	

            CACHE->CACHE_CONFIG = (CACHE->CACHE_CONFIG & 0xFF00FFFF) | (0xA5 << 16);
            break;

        default:
            QSPIFLASH_DEBUG("QSPI Flash chip Not Support\n");
            g_qspiflash_chiptype = -1;
            break;
        }
#endif
    }
    return g_qspiflash_chiptype;
}
/*******************************************************************
Author:   
Function Name:  s32 drv_internalflash_open(void)
Function Purpose:open internal flash
Input Paramters: 
Output Paramters:N/A
Return Value:      N/A
Remark: N/A
Modify: N/A
********************************************************************/
s32 drv_qspiflash_open(void)
{
    if(g_qspiflash_fd < 0)
    {
        
        //QSPI_SetLatency(0);
        if(drv_qspiflash_checktype()<0)
        {
            QSPIFLASH_DEBUG("open err!\r\n");
        }
        g_qspiflash_fd = 0;
        //FLASH_Lock(); //锁定内部flash,防止程序跑飞误操作
    }
    
    return DEVSTATUS_SUCCESS;
}

/*******************************************************************
Author:   
Function Name:  static int drv_internalflash_DataCheck(void *src, void *dst, uint32_t size)
Function Purpose:check if internal flash the data is the same
Input Paramters: addr - internal flash addr,pagNum - page num
Output Paramters:N/A
Return Value:    0 -success   other -fail
Remark: N/A  
Modify: N/A
********************************************************************/
static int drv_qspiflash_DataCheck(void *src, void *dst, uint32_t size)
{
    uint8_t *pSrc = (uint8_t *)src, *pDst = (uint8_t *)dst;

    if(memcmp(pDst, pSrc, size))
    {
        return -1;
    }

    return 0;
}

/*******************************************************************
Author:   
Function Name:  s32 drv_qspiflash_erasesector(u32 startaddr)
Function Purpose:check if internal flash is erase success
Input Paramters: addr - internal flash addr,pagNum - page num
Output Paramters:N/A
Return Value:    0 -success   other -fail
Remark: N/A  
Modify: N/A
********************************************************************/
static int drv_qspiflash_EraseCheck(uint32_t addr, uint32_t pagNum)
{
    uint32_t i = 0;
    uint8_t erase_Buf[QSPIFLASH_PAGE_SIZE];

    memset(erase_Buf, 0xFF, QSPIFLASH_PAGE_SIZE);

    CACHE_CleanAll(CACHE);    
    for (i = 0; i < pagNum; i++)
    {
        if (-1 == drv_qspiflash_DataCheck(erase_Buf, (uint8_t *)(addr + i * QSPIFLASH_PAGE_SIZE), sizeof(erase_Buf)))
        {
            QSPIFLASH_DEBUG("Check err(i=%d, addr=%08X)!\r\n", i, addr + i * QSPIFLASH_PAGE_SIZE);
            return -1;
        }
    }	

    return 0;

}


/*******************************************************************
Author:   
Function Name:  s32 drv_qspiflash_erasesector(u32 startaddr)
Function Purpose:erase sector
Input Paramters: 
Output Paramters:N/A
Return Value:      N/A
Remark: N/A  
Modify: N/A
********************************************************************/
s32 drv_qspiflash_erasesector(u32 startaddr)
{
    s32 ret;
    u32 eraseaddr;
  #ifdef DEBUG_INNERFLASH_EN
    u32 t1,t2;
  #endif

    if(drv_qspiflash_open() != DEVSTATUS_SUCCESS)
    {
        return DEVSTATUS_ERR_DEVICE_NOTOPEN;
    }

    startaddr += QSPIFLASH_ADDRSTA;

    eraseaddr = startaddr&(~(QSPIFLASH_SECTOR_SIZE - 1));
    //check the erase time
    ret = FLASH_EraseSector(eraseaddr);
    
    //CACHE_CleanAll(CACHE);
    ret = drv_qspiflash_EraseCheck(eraseaddr,QSPIFLASH_SECTOR_PAGE_NUM);
    if(ret != 0)
    {
        return DEVSTATUS_ERR_FAIL;
    }
    return DEVSTATUS_SUCCESS;
}


/*******************************************************************
Author:   
Function Name:  s32 dev_qspiflash_program(u32 startaddr,u8* rbuf,u32 rlen)
Function Purpose:internal flash write
Input Paramters: 
Output Paramters:N/A
Return Value:      N/A
Remark: N/A      //sxl?写也是要8个字节对齐 //sxl?需要重新整理，2K  2K的写
Modify: N/A
********************************************************************/
s32 drv_qspiflash_program(u32 startaddr,u8* rbuf,u32 rlen)
{
    
    //FLASH_Status status;
    QSPI_CommandTypeDef cmdType;
    int pagNum,i;
  #if 0//def DEBUG_QSPIFLASH_EN
    u32 t1,t2;
  #endif
    u32 chip_type;
    s32 ret;
    
    startaddr += QSPIFLASH_ADDRSTA;

    
  #if(QSPI_INTERNAL==1)
  #else
    chip_type = drv_qspiflash_checktype();
    if(chip_type == QSPI_SUPPORT_CHIP_JEDEC_ID_MICR)
    {
        cmdType.Instruction = PAGE_PROG_CMD;
        cmdType.BusMode = QSPI_BUSMODE_111;
        cmdType.CmdFormat = QSPI_CMDFORMAT_CMD8_ADDR24_PDAT;
    }
    else
    {
        cmdType.Instruction = QUAD_INPUT_PAGE_PROG_CMD;
        cmdType.BusMode = QSPI_BUSMODE_114; 
        cmdType.CmdFormat = QSPI_CMDFORMAT_CMD8_ADDR24_PDAT;
    }
  #endif
    
    pagNum = (rlen + QSPIFLASH_PAGE_SIZE-1)/QSPIFLASH_PAGE_SIZE;

    #if 0//def DEBUG_QSPIFLASH_EN
    t1 = dev_user_gettimeID();
    #endif

    for(i = 0;i < pagNum;i++)
    {
      #if(QSPI_INTERNAL==1)
        ret = FLASH_ProgramPage(startaddr + i * QSPIFLASH_PAGE_SIZE, QSPIFLASH_PAGE_SIZE, (uint8_t*)(rbuf + i * QSPIFLASH_PAGE_SIZE));
      #else
        #ifdef QSPI_PROG_USEDMA
        ret = FLASH_ProgramPage(&cmdType, DMA_Channel_0, startaddr + i * QSPIFLASH_PAGE_SIZE, QSPIFLASH_PAGE_SIZE, (uint8_t*)(rbuf + i * QSPIFLASH_PAGE_SIZE));
        #else
        ret = FLASH_ProgramPage(&cmdType, NULL, startaddr + i * QSPIFLASH_PAGE_SIZE, QSPIFLASH_PAGE_SIZE, (uint8_t*)(rbuf + i * QSPIFLASH_PAGE_SIZE));
        #endif
      #endif
        if(ret != QSPI_STATUS_OK)
        {
            QSPIFLASH_DEBUG("ProgramPage Err(%d)\r\n", ret);   
            return DEVSTATUS_ERR_FAIL;         
        }

        
        CACHE_CleanAll(CACHE);
        if (-1 == drv_qspiflash_DataCheck((uint8_t*)(rbuf + i * QSPIFLASH_PAGE_SIZE), (uint8_t *)(startaddr + i * QSPIFLASH_PAGE_SIZE), QSPIFLASH_PAGE_SIZE))
        {
            QSPIFLASH_DEBUG("DataCheck err(%d, %08X)\r\n", i, startaddr + i * QSPIFLASH_PAGE_SIZE);
            return DEVSTATUS_ERR_FAIL;
        }
    }

    
    #if 0//def DEBUG_QSPIFLASH_EN
    t2 = dev_user_gettimeID();
    QSPIFLASH_DEBUG("write time:%d %d %08x\r\n",t1,t2,startaddr);
    #endif

    //sxl?20180607
    /*
    FLASH_Unlock();
    status = FLASH_WriteBlock(startaddr,rbuf,rlen);// 底下写的是字符长度
    FLASH_Lock();

    #ifdef INTERNALFLASH_DEBUG
    dev_debug_printf("\r\ndev_internalflash_program ret = %08x\r\n",status);
    #endif

    if(status != FLASH_COMPLETE)
    {
        return DEVSTATUS_ERR_FAIL;
    }
    */

    return DEVSTATUS_SUCCESS;

}



/*******************************************************************
Author:   
Function Name:  s32 dev_internalflash_program(u32 startaddr,u8* rbuf,u32 rlen)
Function Purpose:internal flash write
Input Paramters: 
Output Paramters:N/A
Return Value:      N/A
Remark: N/A      //sxl?写也是要8个字节对齐 //sxl?需要重新整理，2K  2K的写
Modify: N/A
********************************************************************/
s32 drv_qspiflash_write(u32 startaddr, u8 *wbuf, u32 wlen)
{
    u32 offset;
    union _FlashWriteBuf *FSW;
    u32 flashblocksize,writelen,writepos;
    u32 addr = 0;
    //s32 ret;
    u8 neederaseflag = 0;
    u32 i;
    u8 cmpvalue[256];
    
    //drv_qspiflash_printinfo(4);
    if(drv_qspiflash_open() != DEVSTATUS_SUCCESS)
    {
        return DEVSTATUS_ERR_DEVICE_NOTOPEN;
    }
    
    if((startaddr+wlen) > QSPIFLASH_SPACE_MAX)
    {
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    
    addr = startaddr;
    flashblocksize = QSPIFLASH_SECTOR_SIZE;
    
    
    //QSPIFLASH_DEBUG("%08x %08x\r\n",startaddr,flashblocksize);
    
    //drv_qspiflash_printinfo(5);
    offset = addr&(flashblocksize-1);     
    addr  = addr&(~(flashblocksize-1));


    //QSPIFLASH_DEBUG("pcidev_flash_write1:%08x %08x %08x\r\n",offset,addr,wlen);

    //drv_qspiflash_printinfo(6);
    FSW = (union _FlashWriteBuf *)k_malloc(sizeof(union _FlashWriteBuf));
    if(FSW == NULL)
    {
        QSPIFLASH_DEBUG("nmalloc error\r\n");
        return -2;
    }
    //QSPIFLASH_DEBUG("pcidev_flash_write2:\r\n");

    //drv_qspiflash_printinfo(7);
    memset(cmpvalue, 0xff, sizeof(cmpvalue));
    for(writepos = 0; writepos < wlen;)
    {
        if((flashblocksize - offset)>(wlen - writepos))
        {
            writelen = wlen - writepos;
        }
        else
        {
            writelen = flashblocksize - offset;
        }

        //要保证FLASH块完整
        //if(writelen < flashblocksize)   // alway read the block
        {
            //QSPIFLASH_DEBUG("start write123(addr=%08X) \r\n", addr);
            //drv_qspiflash_printinfo(8);
            if(drv_qspiflash_read(addr,FSW->Buf,flashblocksize) != flashblocksize)
            {
                QSPIFLASH_DEBUG("write fail1 \r\n");

                k_free(FSW);
                return -1;
            }
        }

        //drv_qspiflash_printinfo(9);
        //检查是否需要写入
        if(memcmp(&FSW->Buf[offset],&wbuf[writepos],writelen) == 0)
        {
            QSPIFLASH_DEBUG("the data is the same, write success \r\n");
            
            writepos += writelen;
            addr += flashblocksize;
            offset = 0;
            continue;
            
        }
        
         //drv_qspiflash_printinfo(10);
        //检查是否要进行擦flash的动作
        neederaseflag = 0;
        for(i = 0;i < flashblocksize;)
        {
            if(memcmp(&FSW->Buf[i],cmpvalue,256))
            {
                neederaseflag = 1;
                break;
            }
            i+=256;
        }
        
        
        //drv_qspiflash_printinfo(11);
        if(neederaseflag)
        {
            if(drv_qspiflash_erasesector(addr) != DEVSTATUS_SUCCESS)
            {
                QSPIFLASH_DEBUG("write fail2 \r\n");
                
                k_free(FSW);
                return -1;
            }
        }
        
        
        

        memcpy(&FSW->Buf[offset],&wbuf[writepos],writelen);

        QSPIFLASH_DEBUG("start write456 %08x %08x\r\n",addr,flashblocksize);
        
        

        QSPIFLASH_DEBUG("start write \r\n");


        if(drv_qspiflash_program(addr, FSW->Buf,flashblocksize) != DEVSTATUS_SUCCESS)
        {
            
            QSPIFLASH_DEBUG("\r\n write fail3 \r\n");
            
            k_free(FSW);
            return -1;
            
            
        }
        QSPIFLASH_DEBUG("end write \r\n");
        
        writepos += writelen;
        addr += flashblocksize;
        offset = 0;

        
    }

    QSPIFLASH_DEBUG("write success \r\n");

    #if 0
    //sxl?2019将写进入的数据读出来
    dev_debug_printf("\r\nread write flash data:%x %d\r\n",startaddr,wlen);
    for(i = 0;i < wlen;)
    {
        if(i+4096 < wlen)
        {
            writelen = 4096;
        }
        else
        {
            writelen = wlen - i;
        }
        drv_internalflash_read(startaddr+i,FSW->Buf,writelen);
        i += writelen;
     }
    #endif

    k_free(FSW);
    
    return 0;
    
}



/*******************************************************************
Author:   
Function Name:  s32 dev_internalflash_read(u32 startaddr,u8* rbuf,u32 rlen)
Function Purpose:internal flash read
Input Paramters: 
Output Paramters:N/A
Return Value:      N/A
Remark: N/A
Modify: N/A
********************************************************************/
s32 drv_qspiflash_read(u32 startaddr,u8* rbuf,u32 rlen)
{
    u32 i;
    u8 *ap_addr;

    if(drv_qspiflash_open() != DEVSTATUS_SUCCESS)
    {
        return 0;
    }
    
    if((startaddr+rlen) > QSPIFLASH_SPACE_MAX)
    {
        return DEVSTATUS_ERR_PARAM_ERR;
    }

    startaddr += QSPIFLASH_ADDRSTA;
    ap_addr = (u8*)(startaddr);
    for(i=0; i<rlen; i++)
    {
        *rbuf++ = *ap_addr++;
    }

    #if 0
    //sxl?2019把读出来的flash的值打印出来
    ap_addr = (u8*)(startaddr);
    dev_debug_printf("\r\nread flash:%x %d\r\n",startaddr,rlen);
    for(i=0; i<rlen; i++)
    {
        dev_debug_printf("%02x ",*ap_addr++);
    }
    dev_debug_printf("\r\n");
    #endif
    
    return rlen;
    
}



/*******************************************************************
Author:   
Function Name:  s32 dev_internalflash_writespec(u32 startaddr,u8* rbuf,u32 rlen)
Function Purpose:internal flash write
Input Paramters: 
Output Paramters:N/A
Return Value:      N/A
Remark: N/A      //sxl?写也是要8个字节对齐 //sxl?需要重新整理，2K  2K的写
Modify: N/A
********************************************************************/
s32 drv_qspiflash_writespec(u32 startaddr, u8 *wbuf, u32 wlen)
{
    u32 offset;
    u32 flashblocksize,writelen,writepos;
    u32 addr = 0;
    //s32 ret;
    u8 neederaseflag = 0;
    u8 cmpvalue[256];
    
    
    if(drv_qspiflash_open() != DEVSTATUS_SUCCESS)
    {
        return DEVSTATUS_ERR_DEVICE_NOTOPEN;
    }

    addr = startaddr;
    flashblocksize = QSPIFLASH_SECTOR_SIZE;
    
    
    QSPIFLASH_DEBUG("pcidev_flash_write:%08x %08x\r\n",startaddr,flashblocksize);
    
    
    offset = addr&(flashblocksize-1);
    if(offset != 0)
    {
        return DEVSTATUS_ERR_FAIL;
    }
    
    addr  = addr&(~(flashblocksize-1));
    
    
    QSPIFLASH_DEBUG("pcidev_flash_write1:%08x %08x %08x\r\n",offset,addr,wlen);
    
    
    memset(cmpvalue,0xff,sizeof(cmpvalue));
    for(writepos = 0;writepos < wlen;)
    {
        if((flashblocksize - offset)>(wlen - writepos))
        {
            writelen = wlen - writepos;
        }
        else
        {
            writelen = flashblocksize - offset;
        }
        
        
        neederaseflag = 1;
        if(neederaseflag)
        {
            if(drv_qspiflash_erasesector(addr) != DEVSTATUS_SUCCESS)
            {
                QSPIFLASH_DEBUG("\r\n write fail2 \r\n");
                
                return -1;
            }
        }
        
        QSPIFLASH_DEBUG("start write456 %08x %08x\r\n",addr,flashblocksize);
		
		
        if(drv_qspiflash_program(addr,(u8*)&wbuf[writepos],writelen) != DEVSTATUS_SUCCESS)
        {
            
            QSPIFLASH_DEBUG("write fail3 \r\n");
            
            return -1;
            
        }
        QSPIFLASH_DEBUG("\r\n end write \r\n");
        
        writepos += writelen;
        addr += flashblocksize;
        offset = 0;
    }

    QSPIFLASH_DEBUG("write success \r\n");

    
    return 0;
    
}





void testqspiflash(void)
{
    
    u8 flashdata[1025];//,flashdata1[4097];
    u32 addr = 0x20000,i;  //120K address
    u8 writefailflag = 0;
    s32 ret;

    if(drv_qspiflash_open() == DEVSTATUS_SUCCESS)
    {
        //dev_debug_printf("\r\ninternal flash info:\r\n");
        //dev_debug_printf("\r\nflash size:%08x sector size:%08x\r\n",s_flashDriver.PFlashTotalSize,s_flashDriver.PFlashSectorSize);

        drv_qspiflash_read(addr,flashdata,1024);
        //dev_debug_printf("\r\nread before write:\r\n");
        //for(i = 0;i < 4097;i++)
        //{
        //    dev_debug_printf("%02x ",flashdata[i]);
        //}
        
        for(i = 0;i < 1025;i++)
        {
            flashdata[i] = i;
        }
        
        
        ret = drv_qspiflash_erasesector(addr);
        if(ret == DEVSTATUS_SUCCESS)
        {
            
            if(drv_qspiflash_write(addr,flashdata,1025) == DEVSTATUS_SUCCESS)
            {
                memset(&flashdata,0,sizeof(flashdata));
                drv_qspiflash_read(addr,flashdata,1025);

                QSPIFLASH_DEBUG("read addr=%08X, len=%d\r\n", addr, 1025);
                QSPIFLASH_DEBUGHEX(NULL, flashdata, 1025);

                writefailflag = 0;
                for(i = 0;i < 1025;i++)
                {
                    
                    if(flashdata[i] != ((u8)i))
                    {
                        QSPIFLASH_DEBUG("failed:%02x %02x\r\n",flashdata[i], i);
                        writefailflag = 1;
                    }
                    
                    
                }

                if(writefailflag)
                {
                    QSPIFLASH_DEBUG("write internal flash failed!111\r\n");
                }
                
            }
            else
            {
                QSPIFLASH_DEBUG("write internal flash failed!222\r\n");
            }
            
        }
        else
        {
            QSPIFLASH_DEBUG("erase internal flash failed!333\r\n");
        }
        
        
    }
    
}
#endif


