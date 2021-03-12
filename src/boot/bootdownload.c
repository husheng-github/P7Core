

#include "bootglobal.h"







//文件起始地址和长度从静态文件中获取，读到null文件就结束
const CANDOWNFILEINFO CanDownFileStore[] =   //读取pk  file   info
{  
    //{"apextboot.bin", BOOTDOWNLOAD_FILE_TYPE_BOOTBIN, 0, 0},  //boot位置不能动
	{"rom.bin", BOOTDOWNLOAD_FILE_TYPE_COREBIN, 0,0},
	{"apapp.bin", BOOTDOWNLOAD_FILE_TYPE_APPBIN, 0,0},
    //{"acquirer.pk", BOOTDOWNLOAD_FILE_TYPE_ACQUIRERPK, 0, 0}, //在 core里升级，这里不处理
  #if(MACHINETYPE==MACHINE_M2)
    {"logo.bmp", BOOTDOWNLOAD_FILE_TYPE_RESOURCE, 0,0},
    {"hz1212.bin", BOOTDOWNLOAD_FILE_TYPE_RESOURCE, 0,0},
    {"t9py.bin", BOOTDOWNLOAD_FILE_TYPE_RESOURCE, 0,0},
    {"utftogbk.bin", BOOTDOWNLOAD_FILE_TYPE_RESOURCE, 0,0},
  #endif	
    {"null", 0, 0,0}
	
};



void bootdownload_init(void)
{
	
	memset(&gParseData,0,sizeof(BOOTDOWNLOAD_PARSEDATA));
	gParseData.pdownfiledata = (u8 *)k_malloc(PC_DL_BLOCK_SIZE+200);
	gDownFileInfo = NULL;
}

void bootdownload_exit(void)
{
	
	if(gDownFileInfo != NULL)
	{
		k_free(gDownFileInfo);
		gDownFileInfo = NULL;
	}
	
}

void bootdownload_initialcandownfileinfo(void)
{
	 u8 i;
	 u8 filename[60];
	 u8 ret;

    memset((u8 *)gCanDownFileInfo, 0, MAXFILE_NUM*sizeof(CANDOWNFILEINFO));
    for(i = 0; i<MAXFILE_NUM; i++)
    {
        gCanDownFileInfo[i].fileaddr = -1;
    }
	
	
    i = 0;
	gCurcandownfilenum = 0;
    while(strcmp((s8 *)CanDownFileStore[i].filename,"null"))
    {
		memcpy((u8 *)&gCanDownFileInfo[i],(u8 *)&CanDownFileStore[i],sizeof(CANDOWNFILEINFO));
		
		strcpy((s8 *)filename,"/mtd2/");
		strcat((s8 *)filename,(s8 *)gCanDownFileInfo[i].filename);
        gCanDownFileInfo[i].fileaddr = static_vfs_getarea((const char *)filename,-1,&gCanDownFileInfo[i].maxfilesize);
		
		gCurcandownfilenum++;
		i++;
		
		
    }

    if(gDownFileInfo == NULL)
    {
    	
		gDownFileInfo = (DOWNFILE *)k_malloc(sizeof(DOWNFILE)*MAXFILE_NUM);
	    memset((u8 *)gDownFileInfo,0,sizeof(DOWNFILE)*MAXFILE_NUM);
		
		
    }
	
	
	for(ret = 0; ret < MAXFILE_NUM; ret++)
    {
        gDownFileInfo[ret].addr = -1;
    }
	
	
}



void bootdownload_processrecbag(FILEBAGINFO *lp_filebaginfo)
{
	lp_filebaginfo->filenum = gParseData.pdownfiledata[0];
    lp_filebaginfo->bagno = gParseData.pdownfiledata[1];
    lp_filebaginfo->bagnosum = gParseData.pdownfiledata[2];
    lp_filebaginfo->machtyp = gParseData.pdownfiledata[3];
    lp_filebaginfo->hardver = gParseData.pdownfiledata[4];
    memset(lp_filebaginfo->filename,0,51);
    memcpy(lp_filebaginfo->filename,&gParseData.pdownfiledata[5],51);
    lp_filebaginfo->filelength = Long2Byte(&gParseData.pdownfiledata[56]);
    lp_filebaginfo->crc = Long2Byte(&gParseData.pdownfiledata[60]);
    lp_filebaginfo->offset = Long2Byte(&gParseData.pdownfiledata[64]);
    lp_filebaginfo->p = &gParseData.pdownfiledata[68];
}



void bootdownload_beep(u8 beeptype)
{
//	dev_audio_ioctl(DDI_AUDIO_CTL_BUZZER, 0, 50);
}



void bootdownload_processerr(u8 bagno,u8 errtype)
{
	
	bootdownload_beep(BOOTDOWNLOAD_BEEP_ERR);
    bootdownload_prompt(errtype,NULL ,NULL);
	bootdownload_spiresponse(bagno,errtype);
	
}


void bootdownload_processerr_response(u8 errtype,u8 *promptlen,u8 *prompt)
{
	bootdownload_beep(BOOTDOWNLOAD_BEEP_ERR);
    bootdownload_prompt(errtype,promptlen ,prompt);
}


s32 bootdownload_GetDirAndName(u8 *tempname,u8 *dir,u8 *name)
{
    int i;
    u8 ret = 0;

    if(tempname[0] == 0x2F)
    {
        for(i = (strlen((s8 *)tempname)-1); i >= 0; i--)
        {
            if(tempname[i] == 0x2F)
            {
                memcpy(dir,tempname,i+1);
                strcpy((s8 *)name,(s8 *)&tempname[i+1]);
                dir[i+1] = 0;
                ret = 1;
                break;
            }
        }
    }
    else
    {
        for(i = (strlen((s8 *)tempname)-1); i >= 0; i--)
        {
            if(tempname[i] == 0x2F)
            {
                strncat((s8 *)dir,(s8 *)tempname,i+1);
                strcpy((s8 *)name,(s8 *)&tempname[i+1]);
                dir[i+1] = 0;
                ret = 1;
                break;
            }
        }
        if(i < 0) strcpy((s8 *)name,(s8 *)tempname);
    }

    return ret;
	
}



s32 bootdownload_GetFileAddr(s8 *filename,u8 *tempname, u8 *filetype)
{
    u8 i;
    u8 dir[100],name[100];

    dir[0] = 0;
    bootdownload_GetDirAndName((u8 *)filename,dir,name);
	*filetype = BOOTDOWNLOAD_FILE_TYPE_RESOURCE;
	#ifdef SXL_DEBUG
    BOOT_DEBUG("down name:%s\r\n",filename);
	#endif
    for(i = 0; i < gCurcandownfilenum; i++)
    {
        if(strcmp((s8 *)gCanDownFileInfo[i].filename,(s8 *)name)==0)
        {
            if(tempname!=NULL)
            {
                strcpy((char *)tempname,(char *)name);
            }
			*filetype = gCanDownFileInfo[i].filetype;
            return gCanDownFileInfo[i].fileaddr;
        }
		
    }
    return -1;
	
}


u8 bootdownload_checkifappineffectiveflashfield(u32 startadd,u32 filelen)
{
    //u8 filed1status = 0;
    u8 i;
	#ifdef BOOTDOWNLOAD_DEBUG
	BOOT_DEBUG("\r\n%s check file spacer\n",__FUNCTION__);
	#endif
    //如果小于 并且大于  ，则是默认下载地址，其他则是通过file.adr来决定地址
    for(i = 0; i<gCurcandownfilenum; i++)
    {
        if(startadd == gCanDownFileInfo[i].fileaddr)
        {
            if(filelen <= gCanDownFileInfo[i].maxfilesize)
            {
                return 1;
            }
        }
		
    }
	
	
    //地址检验结束
    #ifdef BOOTDOWNLOAD_DEBUG 
	BOOT_DEBUG("\r\n%s space not engouth\r\n",__FUNCTION__);
	#endif
	
    return 0;
	
	
}




u8 bootdownload_iffilecannotsave(void)
{
	
	u8 ret;
	
	ret = bootdownload_checkifappineffectiveflashfield(gCurFileIndex->addr,gCurFileIndex->filelength);
	if(1 == ret)
	{
		return 0;
	}
	
    return 1;
	
}



s32 coredownload_GetFileAddr(s8 *filename,u8 *tempname, u8 *filetype)
{
    u8 i;
    u8 dir[100],name[100];
	s32 ret = -1;
	u32 len;
	

    dir[0] = 0;
    bootdownload_GetDirAndName((u8 *)filename,dir,name);
	*filetype = BOOTDOWNLOAD_FILE_TYPE_RESOURCE;
	#ifdef SXL_DEBUG
    BOOT_DEBUG("down name:%s\r\n",filename);
	#endif
	
	
	if(strcmp(name,"apupdate") == 0)
	{
		
		*filetype = BOOTDOWNLOAD_FILE_TYPE_UPDATEBAK;
        ret =    0;
	}
    else if((strcmp(name,"hz1212.bin") == 0)
          ||(strcmp(name, "t9py.bin")==0)
          ||(strcmp(name, "utftogbk.bin")==0))
	{
		*filetype = BOOTDOWNLOAD_FILE_TYPE_RESOURCE;
		ret = 1;
	}
	else
	{
		return ret;
	}
	
	if(tempname!=NULL)
    {
        strcpy((char *)tempname,"/mtd2/");
        strcat((char *)tempname,(char *)name);
    }

	ret = static_vfs_getarea(tempname, 0, &len);
	
	return ret;
	
}


s32 bootdownload_dealdownloadlist(FILEBAGINFO *lp_filebaginfo)
{
	
	u8 i;
	s32 addr;
	u8 filename[52];
	u8 name[52];
	u8 filetype;
    s32 ret;
    u32 len;
    u8 fileflg=0;   //0:正常文件
                    //1:mtd1下文件，
	
	
    memset(filename,0,sizeof(filename));
    memset(name,0,sizeof(name));
    strcpy((s8 *)name,(s8 *)lp_filebaginfo->filename);
    pciarith_StrChr(name,'\\','/');//字符替代
	
    addr = coredownload_GetFileAddr((s8 *)name,filename,&filetype);
    
    if((addr < 0)&&(0 == fileflg))
    {
        #ifdef SXL_DEBUG
        BOOT_DEBUG("文件不在下载list内，\r\n");
		#endif
        return EXECUTION_DOWNFILENAME;
    }
	
    //---检查是否在gDownFileInfo下载列表内
    //如不在，说明是第一次下载这个文件
    for(i = 0; i < MAXFILE_NUM; i++)
    {
        if(addr == gDownFileInfo[i].addr)
        {
            gCurFileIndex = &gDownFileInfo[i];
            return 0;
        }
    }
    
	#ifdef SXL_DEBUG
    BOOT_DEBUG("check add file\r\n");
	#endif
    // --- 文件未下载 ----
    for(i = 0; i < MAXFILE_NUM; i++) //boot的地址要特殊为0x0FFFFFFF
    {
        if(gDownFileInfo[i].addr == -1)
        {
            break;
        }
    }

    if(i == MAXFILE_NUM)
    {
        return EXECUTION_DOWNFILENUM;
    }
	
	#ifdef SXL_DEBUG
    BOOT_DEBUG("check add file 11\r\n");
	#endif
    //uart_printf("第一包，登记到下载信息内\r\n");
    gDownFileInfo[i].filelength = lp_filebaginfo->filelength;//Long2Byte(&gParseData.pdownfiledata[56+offset]);
    #ifdef SXL_DEBUG
    BOOT_DEBUG("\r\ngDownFileInfo[i].filelength = %d %08x %s\r\n",i,gDownFileInfo[i].filelength,filename);
	#endif
	if((1 ==fileflg) && (len < gDownFileInfo[i].filelength))
    {
        return EXECUTION_DISKNOTENOUGH;
    }

	gDownFileInfo[i].addr = static_vfs_getarea(filename, 0, &len);
	if(gDownFileInfo[i].addr == -1)
	{
	    gDownFileInfo[i].addr = -1;
		return EXECUTION_DOWNFILENAME;
	}
    if(gDownFileInfo[i].filelength>len)
    {
    	gDownFileInfo[i].addr = -1;
        return EXECUTION_DISKNOTENOUGH;
    }
	
    
    gDownFileInfo[i].flag = 1;   //need download
    gDownFileInfo[i].crc  = lp_filebaginfo->crc;//Long2Byte(&gParseData.pdownfiledata[60+offset]);
    memset(gDownFileInfo[i].filename,0,sizeof(gDownFileInfo[i].filename));
    strcpy((s8 *)gDownFileInfo[i].filename,(s8 *)filename);
    gDownFileInfo[i].filetype = filetype;
    
    
    
    //check disk space
    gCurFileIndex = &gDownFileInfo[i];
    #ifdef SXL_DEBUG
    BOOT_DEBUG("check if file can save:%x\r\n",gDownFileInfo[i].addr);
	#endif

    
  
    

    return 0;

}


s32 bootdownload_fread_specifyaddr(u8 *data,s32 fileoffset,u32 length,u32 fileorgaddr)
{
    u32 addr;

    addr = fileorgaddr;
    addr += fileoffset;


    return dev_flash_read(addr,data,length);

}




s32 bootdownload_fwrite(u8 *data,u32 curbag,u32 length,u32 fileorgaddr,u8 filetype)
{

    u32 addr;
    u32 fileaddr;
    s32 ret;
	
	
    
    addr = fileorgaddr;
    

    fileaddr = curbag;
    fileaddr = fileaddr<<DOWNLOADBLOCKOFFSET;


    addr += fileaddr;
	
    //直接写地址
//BOOT_DEBUG("%d, %08X,%d\r\n", gReservedaddr, addr, length);     
    ret =  dev_flash_write(addr,data,length);
    if(ret == 0)
    {
    	ret = length;
    }
	else
	{
		ret = 0;
	}
	
    return ret;

}


s32 bootdownload_fread(u8 *data,u32 curbag,u32 length,u32 fileorgaddr,u8 filetype)
{
	
    //union _FileSysWrite FSW;
    u32 addr;
    u32 fileaddr;
    //u32 offset;
	
    
    addr = fileorgaddr;
    
	
    fileaddr = curbag;
	
    fileaddr = fileaddr<<DOWNLOADBLOCKOFFSET;
	
    addr += fileaddr;
	
    return dev_flash_read(addr,data,length);
	
}


s32 bootdownload_dealdownloadbag(FILEBAGINFO *lp_filebaginfo)
{
    s32 curbags,filebags;
    u32 length;
    u32 crc;
    s32 i,count;
    s32 ret;
    u8 disp[32];

    curbags  = (lp_filebaginfo->offset)>>DOWNLOADBLOCKOFFSET;
    filebags = (lp_filebaginfo->filelength)>>DOWNLOADBLOCKOFFSET;

    if(lp_filebaginfo->filelength%PC_DL_BLOCK_SIZE > 0)
    {
        filebags += 1;
    }
  #if(LCD_EXIST==1) 
    dev_lcd_clear_rowram(DISPAGE2);
    sprintf(disp, "%s(%d/%d)", lp_filebaginfo->filename, curbags+1, filebags);
    dev_lcd_fill_rowram(DISPAGE2, 0, disp, LDISP);
    dev_lcd_brush_screen();
  #endif
  
    if((curbags + 1) != filebags)
    {
        ret = bootdownload_fwrite(lp_filebaginfo->p,curbags,PC_DL_BLOCK_SIZE,gCurFileIndex->addr,gCurFileIndex->filetype);
//BOOT_DEBUG("ret=%d,len=%d\r\n", ret, PC_DL_BLOCK_SIZE);
        if(ret != PC_DL_BLOCK_SIZE)
        {
            //uart_printf("下载写文件失败 1\r\n");
          #if(LCD_EXIST==1)
            bootdownload_processerr(lp_filebaginfo->bagno,EXECUTION_DOWNFILEOPS);
          #endif
            return 1;
        }
      
        return 3;
    }
	
	
    //uart_printf("本文件最后一包\r\n");
    //---将数据写到指定的位置----
    length = lp_filebaginfo->filelength - lp_filebaginfo->offset;

        
    ret = bootdownload_fwrite(lp_filebaginfo->p, curbags, length, gCurFileIndex->addr,gCurFileIndex->filetype);
//BOOT_DEBUG("ret=%d,len=%d\r\n", ret, length);
    if( ret != length)
    {
        //uart_printf("下载写文件失败 2\r\n");
      #if(LCD_EXIST==1)
        bootdownload_processerr(lp_filebaginfo->bagno,EXECUTION_DOWNFILEOPS);
      #endif
        return 1;
    }

	
    BOOT_DEBUG("\r\ncheck crc\r\n");
    

    //calculate the CRC
    crc = 0xffffffff;
    i = 0;
    if(filebags > 1)
    {
        for(i = 0; i < (filebags - 1); i++)
        {
            count = bootdownload_fread(gParseData.pdownfiledata,i,PC_DL_BLOCK_SIZE,gCurFileIndex->addr,gCurFileIndex->filetype);
            if(count < PC_DL_BLOCK_SIZE)
            {
                //uart_printf("EXECUTION_DOWNFILEOPS 3\r\n");
              #if(LCD_EXIST==1)
                bootdownload_processerr(lp_filebaginfo->bagno,EXECUTION_DOWNFILEOPS);
              #endif
      
                return 1;
            }
            crc = GetHash(crc,gParseData.pdownfiledata,PC_DL_BLOCK_SIZE);
        }

    }
    //---最后一包
    count = bootdownload_fread(gParseData.pdownfiledata,i,length,gCurFileIndex->addr,gCurFileIndex->filetype);
    if(count < length)
    {
        //uart_printf("EXECUTION_DOWNFILEOPS 4\r\n");
      #if(LCD_EXIST==1)
        bootdownload_processerr(lp_filebaginfo->bagno,EXECUTION_DOWNFILEOPS);
      #endif
      
        return 1;
    }
    crc = GetHash(crc,gParseData.pdownfiledata,length);
    crc ^= 0xffffffff;
	
	
	BOOT_DEBUG("\r\ncheck crc:%x %x\r\n",crc,lp_filebaginfo->crc);
	
    //check crc
    if(crc != lp_filebaginfo->crc)
    {
        //uart_printf("校验错\r\n");
      #if(LCD_EXIST==1)
        bootdownload_processerr(lp_filebaginfo->bagno,EXECUTION_CRCERR);
      #endif
        return 2;
    }
    return 0;
	
	
}



s32 bootdownload_downloadfilesave(void)
{
	
    //只保存一个updatebak文件
    return 0;
	
}

s32 bootdownload_GetFiletype(u8 *path, u8 *filetype)
{
    s32 i;
    u8 filename[52];
    u8 dir[52];
    
    
    *filetype = BOOTDOWNLOAD_FILE_TYPE_NULL;
    bootdownload_GetDirAndName(path, dir, filename);
    
    while(strcmp((s8 *)CanDownFileStore[i].filename,"null"))
    {
    	
    	if(strcmp((s8 *)CanDownFileStore[i].filename,filename) == 0)
    	{
            *filetype = CanDownFileStore[i].filetype;
            return 0;
    	}
		i++;
    }
    return -1;
}

static u8 g_framesn = 0;
void boot_trendit_response(u8 frametype, u16 framecmd, u8 response, u8 *respdata, u16 respdatalen)
{
    u8 head[20];
    u8 lrc;
    u8 tail[2];
    s32 i;

    //整理数据
    head[0] = 0x4C;
    head[1] = 0x4B;
    //长度
  #if 0  
    head[2] = (respdatalen+6)>>8;
    head[3] = (respdatalen+6)&0xff;
  #endif  
    //指示位
    head[4] = frametype;
    //指令号
    head[5] = framecmd>>8;
    head[6] = framecmd&0xff;
    //序列号
    head[7] = g_framesn;
    g_framesn++;
    //应答码
    if(frametype == 0x4F)
    {
        head[2] = (respdatalen+6)>>8;
        head[3] = (respdatalen+6)&0xff;
        dev_maths_bcd_to_asc(&head[8], &response, 1);
        i = 10;
    }
    else
    {
        head[2] = (respdatalen+4)>>8;
        head[3] = (respdatalen+4)&0xff;
        i = 8;
    }
    lrc = dev_maths_getlrc(0, &head[2], i-2);
//BOOT_DEBUG("lrc=%03X,i=%d\r\n", lrc, i);
//BOOT_DEBUGHEX(NULL,head, i);
    lrc = dev_maths_getlrc(lrc, respdata, respdatalen);
//BOOT_DEBUG("lrc11=%03X,i=%d\r\n", lrc, respdatalen);
//BOOT_DEBUGHEX(head, i);
    tail[0] = 0x03;
    tail[1] = lrc^0x03;  
    dev_com_write(UARTDOWNLOAD_PORTNUM, head, i);
    dev_com_write(UARTDOWNLOAD_PORTNUM, respdata, respdatalen);
    dev_com_write(UARTDOWNLOAD_PORTNUM, tail, 2);
}

static u8 g_get_rand[17] = {0};
s32 boot_trendit_cmd_getrand(u8 *framedata, u16 framedatalen)
{
    g_get_rand[0] = 8;
    dev_trng_read(&g_get_rand[1], 8);
//memset(&g_get_rand[1], 0x30, 8);
    boot_trendit_response(0x4F, 0xF102, 0x00, &g_get_rand[1], 8);
    return 0;
}
s32 boot_trendit_cmd_encrytype(u8 *framedata, u16 framedatalen)
{
    u8 tmp[2];
    
    tmp[0] = 0xF1;
    tmp[1] = 0x00;
    boot_trendit_response(0x4F, 0x1B01, 0x00, tmp, 2);
    return 0;
}


s32 boot_trendit_deal(void)
{
    u8 frametype;
    u16 framecmd;
    u8 framesn;
    u8 frameresp;
    u8 *framedata;
    u16 framedatalen;
    s32 i;

    frametype = gParseData.pdownfiledata[0];
    framecmd = (((u16)gParseData.pdownfiledata[1])<<8)+gParseData.pdownfiledata[2];
    framesn = gParseData.pdownfiledata[3];
    if(0x4F == frametype) //响应报文
    {
        frameresp = gParseData.pdownfiledata[4];
        i = 5;
    }
    else
    {
        i = 4;
    }
    if(gParseData.RxTotalLen<i)
    {
        return -1;
    }
    framedata = &gParseData.pdownfiledata[i];
    framedatalen = gParseData.RxTotalLen-i;
    switch(framecmd)
    {
        case 0xF102:    //获取随机数
            boot_trendit_response(0x6F, framecmd, 0x00, framedata, 0);
            boot_trendit_cmd_getrand(framedata, framedatalen);
            break;
        case 0x1B01:    //获取终端加密类型
          	boot_trendit_response(0x6F, framecmd, 0x00, framedata, 0);
            boot_trendit_cmd_encrytype(framedata, framedatalen);
            break;
    }
    return 0;
}



s32 bootdownload_checkacquirerpkexist(void)
{
	
	s32 ret;
	u8 *pkdata;
	
	
    pkdata = (u8 *)k_malloc(ACQUIRERPKDATA_MAX);
	ret = dev_misc_machineparam_get(ACQUIRERPKDATA,pkdata, ACQUIRERPKDATA_MAX, 0);
	if(ret == 0&&memcmp(pkdata,"PKV101",6) == 0)
	{
		ret = 0;
	}
	else
	{
		ret = -1;
	}
	k_free(pkdata);
	
	
	return ret;
	
	
	
}


s32 bootdownload_saveacquirerpk(u32 downpkaddr,u32 downpklen)
{
	
	s32 ret;
	u8 *pkdata;
	u8 *savedpkdata;
    
	
	
    pkdata = (u8 *)k_malloc(ACQUIRERPKDATA_MAX);
	memset(pkdata,0,ACQUIRERPKDATA_MAX);
    
	savedpkdata = (u8 *)k_malloc(ACQUIRERPKDATA_MAX);
	memset(savedpkdata,0,ACQUIRERPKDATA_MAX);
	
	
	
	ret = dev_misc_machineparam_get(ACQUIRERPKDATA,pkdata, ACQUIRERPKDATA_MAX, 0);
	dev_debug_printf("\r\nsaveacquirerpk read apk 1 = %d\r\n",ret);
	if(ret == 0&&memcmp(pkdata,"PKV101",6) == 0)
	{
		
		//比较发行者以及版本号
		ret = dev_flash_read(downpkaddr,savedpkdata,ACQUIRERPKDATA_MAX);
		dev_debug_printf("\r\nISSUER:%s\r\n",&savedpkdata[PUBLICKEY_ISSUER_OFFSET]);
		dev_debug_printf("\r\nISSUER1:%s\r\n",&pkdata[PUBLICKEY_ISSUER_OFFSET]);
		dev_debug_printf("\r\nSN:%s\r\n",&savedpkdata[PUBLICKEY_SN_OFFSET]);
		dev_debug_printf("\r\nSN1:%s\r\n",&pkdata[PUBLICKEY_SN_OFFSET]);
		if(memcmp(&savedpkdata[PUBLICKEY_ISSUER_OFFSET],&pkdata[PUBLICKEY_ISSUER_OFFSET],PUBLICKEY_ISSUER_SIZE)&&strlen(&pkdata[PUBLICKEY_ISSUER_OFFSET]) != 0)
		{
			ret = -1;
		}
		else
		{
			
			if(memcmp(&savedpkdata[PUBLICKEY_SN_OFFSET],&pkdata[PUBLICKEY_SN_OFFSET],PUBLICKEY_SN_SIZE) >= 0||strlen(&pkdata[PUBLICKEY_SN_OFFSET]) == 0)
			{
				ret = 0;
			}
			else
			{
				ret = -2;
			}
			
		}
		
		
	}
	else
	{
	    //读取保存的下载PK数据直接保存
	    ret = dev_flash_read(downpkaddr,savedpkdata,ACQUIRERPKDATA_MAX);
		if(ret == ACQUIRERPKDATA_MAX)
		{
		    dev_debug_printf("\r\nacquirer pk not exist\r\n");
			memset(pkdata,0,ACQUIRERPKDATA_MAX);
			ret = 0;
		}
		else
		{
			
		}
		
	}
    
	
	
	if(ret == 0)
	{
		//值不一样，需要保存
		if(memcmp(pkdata,savedpkdata,ACQUIRERPKDATA_MAX))
		{
			ret = dev_misc_machineparam_set(ACQUIRERPKDATA,savedpkdata, ACQUIRERPKDATA_MAX, 0);
			dev_debug_printf("\r\nacquirer pk not same = %d\r\n",ret);
		}
		else
		{
			dev_debug_printf("\r\nacquirer pk same\r\n");
		}
		
	}



	
	k_free(pkdata);
	k_free(savedpkdata);
	
	return ret;


	
}




s32 bootdownload_savebincrc(u8 *filename,u32 crc)
{
	u8 appinfo[MCUAPPINFO_MAX];
	u8 saveflag = 0;
	u8 tempdata[5];
	s32 offset;
    s32 ret = 0;
    
	
    
	tempdata[0] = (u8)(crc>>24);
	tempdata[1] = (u8)(crc>>16);
	tempdata[2] = (u8)(crc>>8);
	tempdata[3] = (u8)(crc);
	
	
	memset(appinfo,0,sizeof(appinfo));
	dev_misc_machineparam_get(MACHINE_PARAM_APPINFO,appinfo,MCUAPPINFO_MAX,0);
    if(strcmp(filename,"rom.bin") == 0)
    {
    	saveflag = 1;
		offset = 0;
    }
    else if(strcmp(filename,"apapp.bin") == 0)
    {
    	saveflag = 1;
		offset = 4;
    }
	
	
	if(saveflag)
	{
		if(memcmp(tempdata,&appinfo[offset],4))
		{
			memcpy(&appinfo[offset],tempdata,4);
			ret = dev_misc_machineparam_set(MACHINE_PARAM_APPINFO,appinfo,MCUAPPINFO_MAX,0);
		}
	}
	

	return ret;
	
	
}
s32 bootdownload_get_updateinfo(updata_info *lp_updata_info)
{
    s32 ret;
    u8 *tmp;

    tmp = (u8*)k_malloc(4096);
    if(tmp == NULL)
    {
        BOOT_DEBUG("malloc err!\r\n");
        return -1;
    }
    while(1)
    {
        ret = fs_read_file("/mtd2/apupdate", tmp, 4096, 0);
        if(ret<=0)
        {
            BOOT_DEBUG("read err!(%d)\r\n", ret);
            break;
        }
        ret = get_updatainfo((s8*)tmp, ret, lp_updata_info);
        if(ret != 0)
        {
            ret = -1;
            break;
        }
        ret = 0;
        break;
    }    

    k_free(tmp);
    return ret;
}
#if 0
s32 bootdownload_checkupdatefile(void)
{ 
    int file_len;
    u8 *tmp;
    int ret = 0;
 	updata_info *tempupdata_information;
	u8 i;
	TERMINAL_PK_TYPE pktype;
	s32 j,k;

	tmp = (u8 *)k_malloc(4096);
    
    dev_debug_printf("\r\n-UpdataConfig_Search-\r\n");
	
    file_len = fs_read_file("/mtd2/apupdate", tmp, 4096, 0);
    if(file_len > 0)
    {
        dev_debug_printf("读成功\r\n");
    }
    else
    {
        dev_debug_printf("无数据\r\n", file_len);
    }
	
	
	
	tempupdata_information = (updata_info *)k_malloc(sizeof(updata_info)*MAXFILE_NUM);
    if(0 == get_updatainfo((s8*)tmp, file_len,tempupdata_information))
    {

		
		j = static_vfs_getarea("/mtd2/apupdate", 0, &file_len);
		if(j > 0)
		{
			//检查是否有acquirer pk 和APP下载
			//调整顺序，把PK放到前面下载
			for(k = 0;k < 2;k++)
			{
				for(i=0; i<MAXFILE_NUM; i++)
				{
					if(tempupdata_information[i].cfgvalidflg)
					{
					    //先校验签名，再校验防切机
						dev_debug_printf("\r\nverity the downfile:%s %x\r\n",tempupdata_information[i].filename,tempupdata_information[i].filelen);
						if(strcmp(tempupdata_information[i].filename,"acquirer.pk") == 0)
						{
							pktype = PK_ACQUIRER_ROOT;
							
						}
						else if(strcmp(tempupdata_information[i].filename,"apapp.bin") == 0)
						{
							//保存app.bin的crc值
							ret = bootdownload_savebincrc(tempupdata_information[i].filename,tempupdata_information[i].filecrc);
							pktype = PK_ACQUIRER;
							
							
							
						}
						else if(strcmp(tempupdata_information[i].filename,"rom.bin") == 0)
						{
							//保存core.bin的crc值
							ret = bootdownload_savebincrc(tempupdata_information[i].filename,tempupdata_information[i].filecrc);
							pktype = PK_TRENDIT_ROOT;
							
							
						}
						else
						{
							pktype = PK_TRENDIT_ROOT;
						}
						
						if((pktype == PK_ACQUIRER_ROOT&&k == 0) ||(pktype == PK_ACQUIRER&&bootdownload_checkacquirerpkexist() == 0&&k == 1))
						{
							ret = bootdownloadauthen_fileauthen((j+tempupdata_information[i].fileadr), tempupdata_information[i].filelen,pktype);
							dev_debug_printf("\r\nfileauthen aaa = %d\r\n",ret);
							if(ret != 0)  //认证失败，退出
							{
								break;
							}

							//保存acquirerpk
							if(pktype == PK_ACQUIRER_ROOT)
							{
								ret = bootdownload_saveacquirerpk(j+tempupdata_information[i].fileadr,tempupdata_information[i].filelen);
								dev_debug_printf("\r\nsaveacquirerpk aaa = %d\r\n",ret);
								if(ret != 0)
								{
									break;
								}
								
							}
							
							
						}
						

						
					}
					
				}
				
				
				if(ret != 0)
				{
					break;
				}
				
				
				
			}
			
		}
		
		
    }
	
	
	
	k_free(tmp);
	
    return ret;
	
	
}
#else
s32 bootdownload_checkupdatefile(void)
{ 
    int ret = 0;
    u32 file_len;
 	updata_info *tempupdata_information;
	u8 i;
	TERMINAL_PK_TYPE pktype;
	s32 j,k;
    u8 wifiuserflg=0xff;
    u8 flg;

	
	tempupdata_information = (updata_info *)k_malloc(sizeof(updata_info)*MAXFILE_NUM);
    if(tempupdata_information == NULL)
    {
        BOOT_DEBUG("malloc err!\r\n");
        return -1;
    }
    ret = bootdownload_get_updateinfo(tempupdata_information);
    if(0 == ret)
    {		
		j = static_vfs_getarea("/mtd2/apupdate", 0, &file_len);
		if(j > 0)
		{
			//检查是否有acquirer pk 和APP下载
			//调整顺序，把PK放到前面下载
			for(k = 0;k < 2;k++)
			{
				for(i=0; i<MAXFILE_NUM; i++)
				{
					if(tempupdata_information[i].cfgvalidflg)
					{
					    //先校验签名，再校验防切机
						dev_debug_printf("\r\nverity the downfile:%s %x\r\n",tempupdata_information[i].filename,tempupdata_information[i].filelen);
						if(strcmp(tempupdata_information[i].filename,"acquirer.pk") == 0)
						{
							pktype = PK_ACQUIRER_ROOT;
							
						}
						else if(strcmp(tempupdata_information[i].filename,"apapp.bin") == 0)
						{
							//保存app.bin的crc值
							ret = bootdownload_savebincrc(tempupdata_information[i].filename,tempupdata_information[i].filecrc);
							pktype = PK_ACQUIRER;
							
							
							
						}
						else if(strcmp(tempupdata_information[i].filename,"rom.bin") == 0)
						{
							//保存core.bin的crc值
							ret = bootdownload_savebincrc(tempupdata_information[i].filename,tempupdata_information[i].filecrc);
							pktype = PK_TRENDIT_ROOT;
							
							
						}
                        else if(strcmp(tempupdata_information[i].filename, "wifiuser.bin") == 0)
                        {
                            if(wifiuserflg==0xff)
                            {
                                wifiuserflg = i;
                            }
                            continue;
                        }
						else
						{
							pktype = PK_TRENDIT_ROOT;
						}	
                        
					/*	if((pktype == PK_ACQUIRER_ROOT&&k == 0) 
                         ||(pktype == PK_ACQUIRER&&bootdownload_checkacquirerpkexist() == 0&&k == 1))
                         */
                         //逻辑有问题,条件外的文件没有校验就更新了
                         flg = 1;
                        //第一次只搜索acqure.pk
                        if(k == 0)
                        {
                            if(pktype == PK_ACQUIRER_ROOT)
                            {
                                flg = 1;
                            }
                            else
                            {
                                flg = 0;
                            }
                        }
                        else
                        {
                            //第二次对其他文件更新
                            if(pktype == PK_ACQUIRER_ROOT)
                            {
                                flg = 0;
                            }
                            else
                            {
                                flg = 1;
                            }
                        }
                        
                        if(flg == 1)
						{
							ret = bootdownloadauthen_fileauthen((j+tempupdata_information[i].fileadr), tempupdata_information[i].filelen,pktype);
							BOOT_DEBUG("fileauthen ret = %d, pktype=%d\r\n",ret, pktype);
							if(ret != 0)  //认证失败，退出
							{
                                BOOT_DEBUG("authen err!\r\n");
								break;
							}

							//保存acquirerpk
							if(pktype == PK_ACQUIRER_ROOT)
							{
								ret = bootdownload_saveacquirerpk(j+tempupdata_information[i].fileadr,tempupdata_information[i].filelen);
								dev_debug_printf("\r\nsaveacquirerpk aaa = %d\r\n",ret);
								if(ret != 0)
								{
									break;
								}
								
							}							
						}                        
					}
				}
                
				if(ret != 0)
				{
					break;
				}				
			}
            //处理WIFI下载
            if(wifiuserflg != 0xff)
            {
              #ifdef TRENDIT_CORE
                dev_debug_printf("%s(%d):len=%d,offset=%d, addr=%d\r\n", __FUNCTION__, __LINE__, tempupdata_information[wifiuserflg].filelen, tempupdata_information[wifiuserflg].fileadr, 0x1000);
                ret = dev_wifi_update_file("/mtd2/apupdate", tempupdata_information[wifiuserflg].filelen, tempupdata_information[wifiuserflg].fileadr, 0x1000);
              #endif  
            }
		}
    }
	k_free(tempupdata_information);
    return ret;
	
	
}
#endif

s32 bootdownload_protocol_55AA_deal(void)
{
    FILEBAGINFO FileBagInfo;
    s32 ret;
    s32 updataflg=0;
    
    
	memset((u8 *)&FileBagInfo,0,sizeof(FILEBAGINFO));
    //将接收的数据转到FileBagInfo中
    bootdownload_processrecbag(&FileBagInfo);
    //    
  	gFileBagInfo = &FileBagInfo;
	
	#if 0
    //判断机器型号
    if(FileBagInfo.machtyp != MACHINECODE)
    {
        bootdownload_processerr(FileBagInfo.bagno,EXECUTION_MACHINEMODE);
        BOOT_DEBUG("machtyp(0x%02X) err!\r\n", FileBagInfo.machtyp);
        return -1; 
    }
    //判断硬件版本号
    if(FileBagInfo.hardver != HARDWAREVER)
    {
        bootdownload_processerr(FileBagInfo.bagno,EXECUTION_HARDWAREVERSION);
        BOOT_DEBUG("hardver(0x%02X) err!\r\n", FileBagInfo.hardver);
        return -1; 
    }
	#endif

	
	
    //第一包对于第一包显示
	if(FileBagInfo.bagno == 0)
	{
		bootdownload_prompt_downloading(FileBagInfo.filename);	
	}
	
    //搜寻文件是否允许下载，获取空间长度
    ret = bootdownload_dealdownloadlist(&FileBagInfo);
    if(ret != 0)
    {
        bootdownload_processerr(FileBagInfo.bagno,ret);
        BOOT_DEBUG("dealdownloadlist(ret=%d) err!\r\n", ret);
        return -1; 
    }
	
	
    //处理下载包
    ret = bootdownload_dealdownloadbag(&FileBagInfo);
    if(ret == 1)
    {
        //文件操作错误,接收下载 
        bootdownload_processerr(FileBagInfo.bagno,EXECUTION_DOWNFILEOPS);
        BOOT_DEBUG("dealdownloadbag(ret=%d) err!\r\n", ret);
        return -2;
    }
    else if(ret == 2)
    {
        //文件校验错
        bootdownload_processerr(FileBagInfo.bagno,EXECUTION_CRCERR);
        BOOT_DEBUG("dealdownloadbag(ret=%d) err!\r\n", ret);
        return -1;
    }
    else if(ret == 3)
    {
        //该包正常，继续接收后续包
        BOOT_DEBUG("\r\n");
        return 0;
    }
    //该文件接收完成，校验签名
    if(0 == strcmp(gCurFileIndex->filename,"apupdate")||0 == strcmp(gCurFileIndex->filename,"/mtd2/apupdate"))
    {
        //apupdate不用校验，其内部问题需要校验
        ret = 0;
    }
    else
    {
        ret = bootdownloadauthen_processfileauthentication();
    }
    if(ret != 0)
    {
        //文件校验错
		gCurFileIndex->failedflag = 2;
		bootdownload_processerr(gFileBagInfo->bagno,AUTHENTICATEFAIL);
        BOOT_DEBUG("File authen(ret=%d) err!\r\n", ret);
        return -1;
    }
    else
    {
        
        //保存文件
        ret = bootdownload_downloadfilesave();
        if(ret != 0)
        {
            bootdownload_processerr(gFileBagInfo->bagno,EXECUTION_SAVEINFOERROR);
    		BOOT_DEBUG("File save(ret=%d) err!\r\n", ret);
            return -1;
        }
        else
        {
            BOOT_DEBUG("\r\nsave success:%s\r\n",gCurFileIndex->filename);
            gCurFileIndex->failedflag = 0;  // save success
			if(0 == strcmp(gCurFileIndex->filename,"apupdate")||0 == strcmp(gCurFileIndex->filename,"/mtd2/apupdate"))
			{
				
			    BOOT_DEBUG("\r\nsave update flag\r\n");
				//检查下载文件中是否包含acquirer.pk文件,是否满足要求,不能随意切换
				//acquier.pk
				if(bootdownload_checkupdatefile() == 0)
				{
	                dev_misc_updateflg_set(1);
	                updataflg = 1;
					
				}
				else
				{
				    //提示认证失败
					gCurFileIndex->failedflag = 2;
					bootdownload_processerr(gFileBagInfo->bagno,AUTHENTICATEFAIL);
			        BOOT_DEBUG("File authen(ret=%d) err!\r\n", ret);
			        return -1;
				}
				
			}  
        }
    }
    
    
	BOOT_DEBUG("\r\ngRecFileNum = %d %d\r\n",gRecFileNum,gFileBagInfo->filenum);
    //下载完成
    gRecFileNum++;
    if(gRecFileNum >= gFileBagInfo->filenum)
    {
        //下载完成
        BOOT_DEBUG("Download Success!\r\n");
        return 1;
    }
	
	return 0;
	
}

s32 bootdownload_task(void)
{
//    FILEBAGINFO FileBagInfo;
    s32 flag;
    s32 ret;
    u8 updataflg;
    u32 key;
	
	
	updataflg = 0;
  
  #if 0//??pengxuebin,20171017  
	//initial hardware
	if(gBootExecutionStep.executionstep == BOOT_EXECUTION_STEP_DOWN_USB)
	{
		
	}
	else if(gBootExecutionStep.executionstep == BOOT_EXECUTION_STEP_DOWN_SPI)
	{
		
	}
	else if(gBootExecutionStep.executionstep == BOOT_EXECUTION_STEP_UPDATEFROMSPIFLASH)
	{
		
	}
	else  //其他默认是串口下载
	{
		
		dev_uart_open(UARTDOWNLOAD_PORTNUM);
		dev_com_set_baud(0,115200,8,0,1);
		
	}
  #else
//    dev_com_open(UARTDOWNLOAD_PORTNUM, 115200, 8,0,1,0);
//    dev_com_setbaud(UARTDOWNLOAD_PORTNUM,115200,8,0,1, 0);
  #endif  

	fs_system_init();
	bootdownload_initialcandownfileinfo();

//	gFileBagInfo = &FileBagInfo;
//	memset((u8 *)gFileBagInfo,0,sizeof(FILEBAGINFO));

	
    gParseData.Rx_Valid = 0;
	gParseData.CommParseCmdStep = 0;
	gParseData.SynHeadCnt = 0;
	gCurFileIndex = NULL;
    
  #if 1//def SXL_DEBUG
    BOOT_DEBUG("enter download mode\r\n");
  #endif 
  #if 0  
    dev_lcd_open();
BOOT_DEBUG("====%s====\r\n", __FUNCTION__);
    dev_lcd_clear_ram();
    dev_lcd_fill_rowram(0, 0, "DOWNLOAD MODE", NOFDISP|CDISP);//
    dev_lcd_brush_screen();
    drv_lcd_bl_ctl(1);
    dev_keypad_clear();
  #endif  
//bootdownload_test();  
	while(1)
	{		
		flag = bootdownload_parserecdata();
		if(flag == 1)
		{   
            if(gParseData.protocoltype == PROTOCOL_TYPE_55AA)
            {
    		    gParseData.Rx_Valid = 0;
                ret = bootdownload_protocol_55AA_deal();
                if(ret == -2)
                {
                    //操作文件错
                    break;
                }
                else if(ret == 1)
                {
                    //下载成功
                    updataflg = 1;
                    break;
                }
                else if(ret == 2)
                {
                    updataflg = 1;
                    break;
                }
              #if 0  
    			//dev_delay_ms(10000);
    		    bootdownload_processrecbag();
     		  #ifdef SXL_DEBUG
    			BOOT_DEBUG("rec bag:mach=%02X,bagno=%d\r\n", gFileBagInfo->machtyp, gFileBagInfo->bagno);
    		  #endif
                //判断机器型号
    			if(gFileBagInfo->machtyp != MACHINECODE)    //check if machine type match
                {
    				bootdownload_processerr(gFileBagInfo->bagno,EXECUTION_MACHINEMODE);
                    continue;
                }
                
    			if(gFileBagInfo->bagno == 1)  //sxl?测试用，要删除   
    			{
    				gCurFileIndex->failedflag = 0; 
    			}

                //判断硬件版本号
                if(gFileBagInfo->hardver != HARDWAREVER)   //hardware not match
                {
    				bootdownload_processerr(gFileBagInfo->bagno,EXECUTION_HARDWAREVERSION);
                    continue;
                }
    			
    			if(gFileBagInfo->bagno == 0)
    			{
    				bootdownload_prompt_downloading(gFileBagInfo->filename);
    			}

    			ret = bootdownload_dealdownloadlist();
    			if(ret != 0)
    			{
    				continue;  //检查出错
    			}
    			
    			
    			if(gCurFileIndex->flag != 1)
    			{
    				continue;
    			}

    			
 
BOOT_DEBUG("\r\n");   			
    			ret = bootdownload_dealdownloadbag();
                if(ret == 1)
                {
                    break;//return 0; //sxl?
                }
                else if(ret == 2)
                {
                    continue;
                }
    			
BOOT_DEBUG("type=%d, name=%s\r\n", gCurFileIndex->filetype, gCurFileIndex->filename);          
    			ret = bootdownloadauthen_processfileauthentication();
                if(ret != 0)
                {
            		gCurFileIndex->failedflag = 2;
    				bootdownload_processerr(gFileBagInfo->bagno,AUTHENTICATEFAIL);
                }
    			else //sxl? 还需要增加版本比较，低版本程序不能覆盖高版本程序
    			{
    				
    				ret = bootdownload_downloadfilesave();
    				if(ret != 0)
    				{
    					bootdownload_processerr(gFileBagInfo->bagno,EXECUTION_SAVEINFOERROR);
    				}
    				else
    				{
    					gCurFileIndex->failedflag = 0;  // save success
    					if(0 == strcmp(gCurFileIndex->filename,"apupdate"))
    					{
                            fs_insert_file("/mtd2/updateflag", (u8 *)"updateflag",10, 0);
                            updataflg = 1;
    					}
    				}
    			}
    			
    			gRecFileNum++;
    			if(gRecFileNum >= gFileBagInfo->filenum)//download success
                {              
                    break;
                }

    			continue;
              #endif  
            }
            else if(gParseData.protocoltype == PROTOCOL_TYPE_TRENDIT)
            {
                
    		    gParseData.Rx_Valid = 0;
        //BOOT_DEBUG("rxlen=%d\r\n", gParseData.RxTotalLen);
        //BOOT_DEBUGHEX(NULL, gParseData.pdownfiledata, gParseData.RxTotalLen);
                boot_trendit_deal();
            }
			
		}
		
		//sxl? 加按取消键退出
		ret = dev_keypad_read_beep(&key);
        if(ret)
        {

          #if 0
            if(key == ESC)
            {
                break;
            }
            else if(key == POWER)
            {
                dev_lcd_clear_ram();
                dev_lcd_brush_screen();
              #if 0
                drv_lcd_bl_ctl(0);
                while(1)
                {
                    //判断松键
                    //drv_power_check_powerkey(&key);
                    key = dev_keypad_read_power();
                    if(key==0)
                    {
                        dev_misc_poweroff();
                        break;
                    }
                }
              #endif
                while(1)
                {
                    //有外电时程序关不掉,等待按键按下重启
		            key = drv_keypad_get_powerkey();
                    if(key == 1)
                    {
                        dev_user_delay_ms(200);
                        key = drv_keypad_get_powerkey();
                        if(key == 1)
                        {
                            dev_misc_reboot(0);
                        }
                    }
                }
            }
          #endif
        }
		
	}

	#ifdef SXL_DEBUG
	BOOT_DEBUG("download result:\r\n");
	for(ret=0; ret<gRecFileNum; ret++)
	{
		dev_debug_printf("%02x ",gDownFileInfo[ret].failedflag);
	}
	dev_debug_printf("\r\n");
	#endif
    if(updataflg==1)
    {
        return 1;
    }
    else
    {
        return 0;
    }
	
}

