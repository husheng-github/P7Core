

#include "bootglobal.h"



s32 bootdownload_getadrtitlestr(u8 **parsepos,u8 *titlestr,u8 *valuestr,u8 bufmaxlen)
{
    u8 startvalue = 0;
    u8 i= 0,j = 0;

    while(*(*parsepos) != 0)
    {
        if((*(*parsepos) != ' ')&&(*(*parsepos) != '\r')&&(*(*parsepos) != '\n'))
        {
            if(*(*parsepos) == '=')
            {
                startvalue = 1;
                j = 0;
            }
            else
            {
                if(startvalue == 0)
                {
                    if(i < bufmaxlen)
                    {
                        titlestr[i++] = *(*parsepos);
                    }
                }
                else
                {
                    if(j < bufmaxlen)
                    {
                        valuestr[j++] = *(*parsepos);
                    }
                }
            }
        }
        else
        {
            (*parsepos)++;
            if(startvalue == 0)
            {
                return 0;
            }
            else
            {
                return 1;
            }
        }
        (*parsepos)++;
    }

    return 0;
}


s32 bootdownload_checkifcandownfileexist(s8 *filename)
{
    u8 i;

    for(i = 0; i<gCurcandownfilenum; i++)
    {
        if(strcmp((s8 *)gCanDownFileInfo[i].filename,filename)==0)
        {
            return 1;
        }
    }
    return 0;
}

#if 0
s32 bootdownload_parseadrfile(void)
{
	u32 length,readlength;
    u8 *tmp;
    u8 *parsepos;
    u8 titlestr[100],valuestr[100],dir[100],filename[100];
    s32 ret;//,i;
    u8 ifstartaddr = 0;
    s32 i;

    tmp = (u8 *)k_malloc(0x800);
    memset(tmp,0,0x800);

    length = gCurFileIndex->filelength;
    if(length >= 0x800)
    {
        length = 0x800-1;
    }

    readlength = bootdownload_fread_specifyaddr(tmp,0,length,gCanDownFileInfo[gReservedaddr].fileaddr);
    if(readlength != length)
    {
        k_free(tmp);
        return 1;
    }
	
	
    tmp[2047] = 0;
    parsepos = &tmp[0];
    while((*parsepos != 0xff)&&(*parsepos != 0)&&(parsepos <= &tmp[2047]))
    {
        memset(titlestr,0,sizeof(titlestr));
        memset(valuestr,0,sizeof(valuestr));
        ret = bootdownload_getadrtitlestr(&parsepos,titlestr,valuestr,99);
		
        //uart_printf("ret =%d %08x\r\n",ret,(u32)parsepos);
		
        if(ret)
        {
        	
            if(strcmp((s8 *)titlestr,"Name") == 0)  //文件名称
            {
                bootdownload_GetDirAndName(valuestr,dir,filename);
                filename[MAXFILENAMELEN - 1] = 0;
                if(bootdownload_checkifcandownfileexist((s8 *)filename) == 0)  //文件名称不存在
                {
                    if(gCurcandownfilenum < MAXFILE_NUM)
                    {
                        memset((s8 *)&gCanDownFileInfo[gCurcandownfilenum],0,sizeof(CANDOWNFILEINFO));
                        strcpy((s8 *)gCanDownFileInfo[gCurcandownfilenum++].filename,(s8 *)filename);
						gCanDownFileInfo[gCurcandownfilenum-1].filetype = BOOTDOWNLOAD_FILE_TYPE_RESOURCE;
                        ifstartaddr = 1;
						
                    }
                }
            }
            else if(strcmp((s8 *)titlestr,"StartAddr") == 0)
            {
                if(ifstartaddr)
                {
                    gCanDownFileInfo[gCurcandownfilenum-1].fileaddr = ascii_2_u32(&valuestr[2],strlen((s8 *)&valuestr[2]));

                    //BOOT_DEBUG("StartAddr:%d %08x\r\n",gCurcandownfilenum-1,gCanDownFileInfo[gCurcandownfilenum-1].fileaddr);

                }
            }
			
			
        }
    }
   /* BOOT_DEBUG("filenum=%d \r\n", gCurcandownfilenum);
	for(i=0; i<gCurcandownfilenum; i++)
	{
        dev_debug_printf("fname= %s \r\n", gCanDownFileInfo[i].filename);
	}*/
    
	
    k_free(tmp);
    return 0;
	
}
#endif

