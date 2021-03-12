/*
 */
#include "devglobal.h"


#define LITTLEFSSTARTOFFSET  LFS_START_ADDR+SPIFLASH_ADDRSTA//0x100000     //从512K的地址开始文件系统



int block_device_read(const struct lfs_config *c,lfs_block_t block,lfs_off_t off,void* buffer,lfs_size_t size)
{
	u32 addr;
    s32 ret;
	
	addr = LITTLEFSSTARTOFFSET + block*(c->block_size) + off;
	
	ret = dev_flash_read(addr,(u8 *)buffer,size);
	if(ret == size)
	{
		ret = 0;
	}
	else
	{
		ret = -5;
	}
	
	return ret;
	
}




int block_device_prog(const struct lfs_config *c,lfs_block_t block,lfs_off_t off,void* buffer,lfs_size_t size)
{
    
	u32 addr;
    s32 ret;
	
	addr = LITTLEFSSTARTOFFSET + block*(c->block_size) + off;
	//dev_debug_printf("\r\nspiflash_write addr = %x %d\r\n",addr,size);
	ret = dev_flash_write(addr,(u8 *)buffer,size);
	//dev_debug_printf("\r\nspiflash_write ret = %d\r\n",ret);
	if(ret == 0)//size)
	{
		ret = 0;
	}
	else
	{
		ret = -5;
	}
	
	return ret;
	
	
}


int block_device_erase(const struct lfs_config *c,lfs_block_t block)
{
	u32 addr;
    s32 ret;
    u8 trytimes;
	
	addr = LITTLEFSSTARTOFFSET + block*(c->block_size);
	for(trytimes = 0;trytimes < 3;trytimes++)
	{
	    //dev_debug_printf("\r\nspiflash_erase addr = %x\r\n",addr);
		ret = dev_flash_erasesector(addr);
		//dev_debug_printf("\r\nspiflash_erase ret = %d\r\n",ret);
		if(ret == 0)
		{
			break;
		}
		
	}
	
	return ret;
	
}


int block_device_sync(const struct lfs_config *c)
{
	return 0;
}

