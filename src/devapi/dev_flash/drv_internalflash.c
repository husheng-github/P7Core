


#include "devglobal.h"
#include "sdk/mhscpu_sdk.h"
#include "drv_internalflash.h"
#if 0
#ifdef DEBUG_INNERFLASH_EN
#define INTERNALFLASH_DEBUG
#endif

#define PFlashSectorSize   4096


static s32 g_internalflash_fd = -1;


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
s32 drv_internalflash_open(void)
{
    if(g_internalflash_fd < 0)
    {
        g_internalflash_fd = 0;
        FLASH_Lock(); //锁定内部flash,防止程序跑飞误操作
    }
    
    return DEVSTATUS_SUCCESS;
}


/*******************************************************************
Author:   
Function Name:  s32 drv_internalflash_erasesector(u32 startaddr)
Function Purpose:erase sector
Input Paramters: 
Output Paramters:N/A
Return Value:      N/A
Remark: N/A  
Modify: N/A
********************************************************************/
s32 drv_internalflash_erasesector(u32 startaddr)
{
    
    FLASH_Status ret;
    u32 eraseaddr;
    
    if(drv_internalflash_open() != DEVSTATUS_SUCCESS)
    {
        return DEVSTATUS_ERR_DEVICE_NOTOPEN;
    }

    startaddr += INTERNALFLASH_ADDRSTA;
    
    eraseaddr = startaddr&(~(PFlashSectorSize - 1));
    FLASH_Unlock();
    ret = FLASH_ErasePage(eraseaddr);
    FLASH_Lock();
    if(ret != FLASH_COMPLETE)
    {
        return DEVSTATUS_ERR_FAIL;
    }
    
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
s32 drv_internalflash_program(u32 startaddr,u32* rbuf,u32 rlen)
{
    
    FLASH_Status status;

    startaddr += INTERNALFLASH_ADDRSTA;
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
s32 drv_internalflash_write(u32 startaddr, u8 *wbuf, u32 wlen)
{
    u32 offset;
    union _FlashWriteBuf *FSW;
    u32 flashblocksize,writelen,writepos;
    u32 addr = 0;
    //s32 ret;
    u8 neederaseflag = 0;
    u32 i;
    u8 cmpvalue[256];
    
    
    drv_internalflash_printinfo(4);
    if(drv_internalflash_open() != DEVSTATUS_SUCCESS)
    {
        return DEVSTATUS_ERR_DEVICE_NOTOPEN;
    }
    
    addr = startaddr;
    flashblocksize = PFlashSectorSize;
    
    
    #ifdef INTERNALFLASH_DEBUG
    dev_debug_printf("\r\npcidev_flash_write:%08x %08x\r\n",startaddr,flashblocksize);
    #endif
    
    drv_internalflash_printinfo(5);
    offset = addr&(flashblocksize-1);     
    addr  = addr&(~(flashblocksize-1));


    #ifdef INTERNALFLASH_DEBUG
    dev_debug_printf("\r\npcidev_flash_write1:%08x %08x %08x\r\n",offset,addr,wlen);
    #endif

    drv_internalflash_printinfo(6);
    FSW = (union _FlashWriteBuf *)k_malloc(sizeof(union _FlashWriteBuf));
    if(FSW == NULL)
    {
        #ifdef INTERNALFLASH_DEBUG
        dev_debug_printf("\r\nmalloc error\r\n");
        #endif
        return -2;
    }

    #ifdef INTERNALFLASH_DEBUG
    dev_debug_printf("\r\npcidev_flash_write2:\r\n");
    #endif

    drv_internalflash_printinfo(7);
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

        //要保证FLASH块完整
        //if(writelen < flashblocksize)   // alway read the block
        {
            #ifdef INTERNALFLASH_DEBUG
            dev_debug_printf("\r\n start write123 \r\n");
            #endif
            drv_internalflash_printinfo(8);
            if(drv_internalflash_read(addr,FSW->Buf,flashblocksize) != flashblocksize)
            {
                #ifdef INTERNALFLASH_DEBUG
                dev_debug_printf("\r\n write fail1 \r\n");
                #endif

                k_free(FSW);
                return -1;
            }
        }

        drv_internalflash_printinfo(9);
        //检查是否需要写入
        if(memcmp(&FSW->Buf[offset],&wbuf[writepos],writelen) == 0)
        {
            #ifdef INTERNALFLASH_DEBUG
            dev_debug_printf("\r\nthe data is the same, write success \r\n");
            #endif
            writepos += writelen;
            addr += flashblocksize;
            offset = 0;
            continue;
            
        }
        
         drv_internalflash_printinfo(10);
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
        
        
        drv_internalflash_printinfo(11);
        if(neederaseflag)
        {
            if(drv_internalflash_erasesector(addr) != DEVSTATUS_SUCCESS)
            {
                #ifdef INTERNALFLASH_DEBUG
                dev_debug_printf("\r\n write fail2 \r\n");
                #endif
                
                k_free(FSW);
                return -1;
            }
        }
        
        
        

        memcpy(&FSW->Buf[offset],&wbuf[writepos],writelen);

        #ifdef INTERNALFLASH_DEBUG
        dev_debug_printf("\r\n start write456 %08x %08x\r\n",addr,flashblocksize);
        #endif
        
        

        
        #ifdef INTERNALFLASH_DEBUG
        dev_debug_printf("\r\n start write \r\n");
        #endif
        
        
        if(drv_internalflash_program(addr,FSW->U32Buf,flashblocksize) != DEVSTATUS_SUCCESS)
        {
            
            #ifdef INTERNALFLASH_DEBUG
            dev_debug_printf("\r\n write fail3 \r\n");
            #endif
            
            k_free(FSW);
            return -1;
            
            
        }
        #ifdef INTERNALFLASH_DEBUG
        dev_debug_printf("\r\n end write \r\n");
        #endif
        writepos += writelen;
        addr += flashblocksize;
        offset = 0;

        
    }

    #ifdef INTERNALFLASH_DEBUG
    dev_debug_printf("\r\n write success \r\n");
    #endif

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
s32 drv_internalflash_read(u32 startaddr,u8* rbuf,u32 rlen)
{
    u32 i;
    u8 *ap_addr;

    if(drv_internalflash_open() != DEVSTATUS_SUCCESS)
    {
        return 0;
    }

    startaddr += INTERNALFLASH_ADDRSTA;
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
s32 drv_internalflash_writespec(u32 startaddr, u8 *wbuf, u32 wlen)
{
    u32 offset;
    u32 flashblocksize,writelen,writepos;
    u32 addr = 0;
    //s32 ret;
    u8 neederaseflag = 0;
    u8 cmpvalue[256];
    
    
    
    if(drv_internalflash_open() != DEVSTATUS_SUCCESS)
    {
        return DEVSTATUS_ERR_DEVICE_NOTOPEN;
    }

    addr = startaddr;
    flashblocksize = PFlashSectorSize;
    
    
    #ifdef INTERNALFLASH_DEBUG
    dev_debug_printf("\r\npcidev_flash_write:%08x %08x\r\n",startaddr,flashblocksize);
    #endif
    
    
    offset = addr&(flashblocksize-1);
    if(offset != 0)
    {
        return DEVSTATUS_ERR_FAIL;
    }
    
    addr  = addr&(~(flashblocksize-1));
    
    
    #ifdef INTERNALFLASH_DEBUG
    dev_debug_printf("\r\npcidev_flash_write1:%08x %08x %08x\r\n",offset,addr,wlen);
    #endif

    
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
            if(drv_internalflash_erasesector(addr) != DEVSTATUS_SUCCESS)
            {
                #ifdef INTERNALFLASH_DEBUG
                dev_debug_printf("\r\n write fail2 \r\n");
                #endif
                
                return -1;
            }
        }
        
        #ifdef INTERNALFLASH_DEBUG
        dev_debug_printf("\r\n start write456 %08x %08x\r\n",addr,flashblocksize);
        #endif
        
        
        if(drv_internalflash_program(addr,(u32*)&wbuf[writepos],writelen) != DEVSTATUS_SUCCESS)
        {
            
            #ifdef INTERNALFLASH_DEBUG
            dev_debug_printf("\r\n write fail3 \r\n");
            #endif
            
            return -1;
            
        }
        #ifdef INTERNALFLASH_DEBUG
        dev_debug_printf("\r\n end write \r\n");
        #endif
        writepos += writelen;
        addr += flashblocksize;
        offset = 0;

        
    }

    #ifdef INTERNALFLASH_DEBUG
    dev_debug_printf("\r\n write success \r\n");
    #endif

    
    return 0;
    
}




void drv_internalflash_printinfo(u8 i)
{
    
}


void testinternalflash(void)
{
    
    u8 flashdata[1025];//,flashdata1[4097];
    u32 addr = 0x20000,i;  //120K address
    u8 writefailflag = 0;
    s32 ret;

    if(drv_internalflash_open() == DEVSTATUS_SUCCESS)
    {
        //dev_debug_printf("\r\ninternal flash info:\r\n");
        //dev_debug_printf("\r\nflash size:%08x sector size:%08x\r\n",s_flashDriver.PFlashTotalSize,s_flashDriver.PFlashSectorSize);

        drv_internalflash_read(addr,flashdata,1024);
        //dev_debug_printf("\r\nread before write:\r\n");
        //for(i = 0;i < 4097;i++)
        //{
        //    dev_debug_printf("%02x ",flashdata[i]);
        //}
        
        for(i = 0;i < 1025;i++)
        {
            flashdata[i] = i;
        }
        
        
        ret = drv_internalflash_erasesector(addr);
        if(ret == DEVSTATUS_SUCCESS)
        {
            
            if(drv_internalflash_write(addr,flashdata,1025) == DEVSTATUS_SUCCESS)
            {
                memset(&flashdata,0,sizeof(flashdata));
                drv_internalflash_read(addr,flashdata,1025);

                dev_debug_printf("\r\n");
                for(i = 0;i < 1025;i++)
                {
                    dev_debug_printf("%02x ",flashdata[i]);
                }
                dev_debug_printf("\r\n");

                writefailflag = 0;
                for(i = 0;i < 1025;i++)
                {
                    
                    if(flashdata[i] != ((u8)i))
                    {
                        dev_debug_printf("\r\nfailed:%02x %02x\r\n",flashdata[i],(u8)i);
                        writefailflag = 1;
                    }
                    
                    
                }

                if(writefailflag)
                {
                    dev_debug_printf("\r\nwrite internal flash failed!111\r\n");
                }
                
            }
            else
            {
                dev_debug_printf("\r\nwrite internal flash failed!222\r\n");
            }
            
        }
        else
        {
            dev_debug_printf("\r\nerase internal flash failed!333\r\n");
        }
        
        
    }
    
}
#endif
