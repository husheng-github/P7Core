#include "bootglobal.h"



#include "./staticfile/fs_static.h"
#include "ddi/ddi_led.h"

#if 0
//=====================================================
u32 bootupdata_check_flg(void)
{
    s32 ret;
//    u8 tmp[32];

//    ret = fs_read_file("/mtd2/updateflag", tmp, 10, 0);
//    if((ret == 10) && (0 == memcmp(tmp, "updateflag", 10)))
    ret = dev_misc_updateflg_check();
    if(ret >= 0)
    {
        //У��ɹ�,��Ҫ����
        #ifdef SXL_DEBUG
        BOOT_DEBUG("bootupdata_check_flg:true\r\n");
		#endif
        return 0;
    }
    else
    {
        #ifdef SXL_DEBUG
        BOOT_DEBUG("bootupdata_check_flg:false\r\n");
		#endif
        return 1;
    }
}
void bootupdata_clear_flg(void)
{
  #if 0
    u8 tmp[32];

    memset(tmp, 0xff, 10);
    fs_write_file("/mtd2/updateflag", tmp, 10);
  #endif
    dev_misc_updateflg_set(0);
}


//=====================================================
u32 bootupdata_deal(void)
{
    u8 charge;
    u8 vollv;

    //�жϸ��±�־
    if(0 == bootupdata_check_flg())    
    {   
        // ��Ҫ����
        return 1;
    }
    
    return 0;
}
#endif
#if 1

updata_info boot_updata_information[MAXFILE_NUM];



/*******************************************************************
 * Function Nam        : s32 get_updata_file_addr(u8 *file_name)
 * Function Purpose    : 
 * Input Paramters     : 
 * Return Value        : N/A
 * Remark              : created by RAY#20150307
 *******************************************************************/
 #if 0
s32 get_updata_file_addr(u8 *file_name, u32 *file_addr)
{
    s32 ret = -1;
    int fd;
    int file_len;
    u8 *file_data = NULL;
    u16 i;
    u8 name_flag = 0;

    if(0 == memcmp(file_name, "rom.bin", 8))
    {
        *file_addr = COREBINADDR;//APP_CORE_FILE_ADDR;
        return 0;
    }

    file_data = (u8 *)k_malloc(2048);
    if(file_data == NULL)
    {
        return -1;
    }
    
    memset(file_data, 0, 2048);

    file_len = ddi_file_read("mtd0/file.adr", file_data, 2048, 0);
    if(file_len > 0)
    {
        for(i=0; i<file_len; i++)
        {
            if(0 == name_flag)
            {
                if(0 == memcmp(&file_data[i], file_name, strlen(file_name)))
                {
                    i += strlen(file_name);
                    name_flag = 1;
                }
            }
            else
            {
                if(0 == memcmp(&file_data[i], "StartAddr=0x", 12)
                &&(file_data[i+20]=='\r'))
                {
                    *file_addr = dev_maths_asc_to_u32(&file_data[i+12], 8, MODULUS_DEC);
                    ret = 0;
                    break;
                }
            }
        }
    }
end:
    k_free(file_data);
    return ret;    
}
#endif 
//#define COMPATIBLE_MODE
//u8 last_no = 0;
int get_updatainfo(s8 *text, u16 len,updata_info *updata_information)
{
    u16 i, cnt, le;
    s8 *p = text;
    s8 *dirp = NULL, *listp = NULL, *lenp = NULL, *crcp = NULL, *adrp = NULL,*endp = NULL;//, *zipp = NULL, *endp = NULL;
    u8 adrvalid[30], lenvalid[30], crcvalid[30], namevalid[30];//, zipvalid[30], dirvalid[30];

    for(i=0; i<len; i++)
    {
        #if 0  
        if(memcmp(p, "bootdown = 1", 12) == 0)
        {
            return 1;
        }
      
        if(memcmp(p, "[dirlist]", 9) == 0)
        {
            dirp = p;
        }
      #endif  

        if(memcmp(p, "[namelist]", 10) == 0)
        {
            listp = p;
        }
        
        if(memcmp(p, "[adrlist]", 9) == 0)
        {
            adrp = p;
        }
        
        if(memcmp(p, "[lenlist]", 9) == 0)
        {
            lenp = p;
        }
        
        if(memcmp(p, "[crclist]", 9) == 0)
        {
            crcp = p;
        }

		#if 0
        if(memcmp(p, "[ziplist]", 9) == 0)
        {
            zipp = p;
        }
		#endif
        
        p++;
    }    
    endp = p;
    
    //if((dirp==NULL)||(listp==NULL)||(lenp==NULL)||(crcp==NULL))
    if((adrp==NULL)||(lenp==NULL)||(crcp==NULL))
        return -1;
    #if 0
    if(zipp == NULL)
    {
        zipp = endp;
    }
	#endif
#if 0    
#if 1//def COMPATIBLE_MODE
    if(adrp == NULL)
    {
        adrp = endp;
    }
#else
    adrp = endp;
#endif
#endif
    
    memset((u8 *)updata_information, 0, sizeof(updata_info)*MAXFILE_NUM);
    memset(adrvalid, 0, 30);
    memset(lenvalid, 0, 30);
    memset(crcvalid, 0, 30);
    memset(namevalid, 0, 30);
    //memset(dirvalid, 0, 30);
    
   #if 0 
    for(p=dirp; p<listp; p++)
    {
        if((memcmp(p, "dir", 3) == 0)&&(*(p-1) == '\n'))
        {
            if((*(p+3) <= '9')&&(*(p+3) >= '0'))
            {
                if (*(p+4) == '=')
                {
                    cnt = *(p+3)-'0';
                    for(i=0; i<DIRLEN; i++)
                    {
                        if(*(p+5+i) == '\r')
                            break;
                    }

                    memcpy(&updata_information[cnt].filedir[0], (p+5), i);
                    updata_printf("dir%d:%s\r\n", cnt, &updata_information[cnt].filedir[0]);
                    
                    dirvalid[cnt] = 1;
                }
                else if((*(p+4) <= '9')&&(*(p+4) >= '0'))
                {  
                    if (*(p+5) == '=')
                    {
                        cnt = (*(p+3)-'0')*10 + (*(p+4)-'0');
                        for(i=0; i<DIRLEN; i++)
                        {
                            if(*(p+6+i) == '\r')
                                break;
                        }
                        
                        memcpy(&updata_information[cnt].filedir[0], (p+6), i);
                        updata_printf("dir%d:%s\r\n", cnt, &updata_information[cnt].filedir[0]);

                        dirvalid[cnt] = 1;
                    }
                }
            }
        }
    }
   #endif 
    
    for(p=listp; p<lenp; p++)
    {
        if((memcmp(p, "file", 4) == 0)&&(*(p-1) == '\n'))
        {
            if((*(p+4) <= '9')&&(*(p+4) >= '0'))
            {
                if (*(p+5) == '=')
                {
                    cnt = *(p+4)-'0';
                    for(i=0; i<FNLEN; i++)
                    {
                        if(*(p+6+i) == '\r')
                            break;
                    }

                    memcpy(&updata_information[cnt].filename[0], (p+6), i);
                    updata_printf("name%d:%s\r\n", cnt, &updata_information[cnt].filename[0]);
                    
                    namevalid[cnt] = 1;
                  #if 0  
                    if(0 == get_updata_file_addr(updata_information[cnt].filename, \
                                                &updata_information[cnt].fileadr))
                    {
                        adrvalid[cnt] = 1;
                        updata_printf("addr%d:0x%08x\r\n", cnt, updata_information[cnt].fileadr);
                    }
                  #endif  
                }
                else if((*(p+5) <= '9')&&(*(p+5) >= '0'))
                {  
                    if (*(p+6) == '=')
                    {
                        cnt = (*(p+4)-'0')*10 + (*(p+5)-'0');
                        for(i=0; i<FNLEN; i++)
                        {
                            if(*(p+7+i) == '\r')
                                break;
                        }
                        
                        memcpy(&updata_information[cnt].filename[0], (p+7), i);
                        updata_printf("name%d:%s\r\n", cnt, &updata_information[cnt].filename[0]);

                        namevalid[cnt] = 1;

                      #if 0  
                        if(0 == get_updata_file_addr(updata_information[cnt].filename, \
                                                    &updata_information[cnt].fileadr))
                        {
                            adrvalid[cnt] = 1;
                            updata_printf("addr%d:0x%08x\r\n", cnt, updata_information[cnt].fileadr);
                        }   
                      #endif  
                    }
                }
            }
        }
    }   
#if 1//def COMPATIBLE_MODE
    for(p=adrp; p<lenp; p++)
    {
        if((memcmp(p, "adr", 3) == 0)&&(*(p-1) == '\n'))
        {
            if((*(p+3) <= '9')&&(*(p+3) >= '0'))
            {
                if (memcmp(p+4, "=0x", 3) == 0)
                {
                    cnt = *(p+3)-'0';
                    
                    le = ((endp-(p+8))>8? 8:endp-(p+7));
                    updata_information[cnt].fileadr = dev_maths_asc_to_u32((p+7), le, MODULUS_HEX);

                    updata_printf("adr%d:0x%08x\r\n", cnt, updata_information[cnt].fileadr);
                    
                    adrvalid[cnt] = 1;
                }
                else if((*(p+4) <= '9')&&(*(p+4) >= '0'))
                {  
                    if  (memcmp(p+5, "=0x", 3) == 0)
                    {
                        cnt = (*(p+3)-'0')*10 + (*(p+4)-'0');

                        le = ((endp-(p+8))>8? 8:endp-(p+8));
                        updata_information[cnt].fileadr = dev_maths_asc_to_u32((p+8), le, MODULUS_HEX);

                        updata_printf("adr%d:0x%08x\r\n", cnt, updata_information[cnt].fileadr);
                        
                        adrvalid[cnt] = 1;
                    }
                }
            }
        }
    }
#endif
    //len
    for(p=lenp; p<crcp; p++)
    {
        if((memcmp(p, "len", 3) == 0)&&(*(p-1) == '\n'))
        {
            if((*(p+3) <= '9')&&(*(p+3) >= '0'))
            {
                if (*(p+4) == '=')
                {
                    cnt = *(p+3)-'0';

                    updata_information[cnt].filelen = dev_maths_asc_to_u32((p+5), 8, MODULUS_DEC);

                    updata_printf("len%d:%d\r\n", cnt, updata_information[cnt].filelen);
                    
                    lenvalid[cnt] = 1;
                }
                else if((*(p+4) <= '9')&&(*(p+4) >= '0'))
                {  
                    if (*(p+5) == '=')
                    {
                        cnt = (*(p+3)-'0')*10 + (*(p+4)-'0');
                        
                        updata_information[cnt].filelen = dev_maths_asc_to_u32((p+6), 8, MODULUS_DEC);

                        updata_printf("len%d:%d\r\n", cnt, updata_information[cnt].filelen);
                        
                        lenvalid[cnt] = 1;
                    }
                }
            }
        }
    }       
    
    for(p=crcp; p<endp; p++)
    {
        if((memcmp(p, "crc", 3) == 0)&&(*(p-1) == '\n'))
        {
            if((*(p+3) <= '9')&&(*(p+3) >= '0'))
            {
                if (memcmp(p+4, "=0x", 3) == 0)
                {
                    cnt = *(p+3)-'0';
                    
                    updata_information[cnt].filecrc = dev_maths_asc_to_u32((p+7), 8, MODULUS_HEX);

                    updata_printf("crc%d:0x%08x\r\n", cnt, updata_information[cnt].filecrc);
                    
                    crcvalid[cnt] = 1;
                }
                else if((*(p+4) <= '9')&&(*(p+4) >= '0'))
                {  
                    if  (memcmp(p+5, "=0x", 3) == 0)
                    {
                        cnt = (*(p+3)-'0')*10 + (*(p+4)-'0');
                        
                        updata_information[cnt].filecrc = dev_maths_asc_to_u32((p+8), 8, MODULUS_HEX);

                        updata_printf("crc%d:0x%08x\r\n", cnt, updata_information[cnt].filecrc);
                        
                        crcvalid[cnt] = 1;
                    }
                }
            }
        }
    }

	#if 0
    for(p=zipp; p<endp; p++)
    {
        if((memcmp(p, "zip", 3) == 0)&&(*(p-1) == '\n'))
        {
            if((*(p+3) <= '9')&&(*(p+3) >= '0'))
            {
                if (*(p+4) == '=')
                {
                    cnt = *(p+3)-'0';
                    
                    updata_information[cnt].zipflag = *(p+5) - '0';

                    updata_printf("zip%d:%d\r\n", cnt, updata_information[cnt].zipflag);
                    
                    zipvalid[cnt] = 1;
                    adrvalid[cnt] = 1; // may packed in file.adr
                }
                else if((*(p+4) <= '9')&&(*(p+4) >= '0'))
                {  
                    if (*(p+5) == '=')
                    {
                        cnt = (*(p+3)-'0')*10 + (*(p+4)-'0');
                        
                        updata_information[cnt].zipflag = *(p+6) - '0';

                        updata_printf("zip%d:%d\r\n", cnt, updata_information[cnt].zipflag);
                        
                        zipvalid[cnt] = 1;
                        adrvalid[cnt] = 1; // may packed in file.adr
                    }
                }
            }
        }
    }
    #endif

    for(i=0; i<MAXFILE_NUM; i++)
    {
        if(adrvalid[i] && lenvalid[i] && crcvalid[i] && namevalid[i])// && dirvalid[i])// && zipvalid[i])
        {
            
            strcpy((s8*)updata_information[i].filedir, "/mtd2");
            updata_printf("valid data%d\r\n", i);
            updata_information[i].cfgvalidflg = 1;
            //last_no = i;
        }
    }
    
    return 0;
}

#define READSIZE (8*512)
int bootupdata_config_Search(const char *path)
{
    int fd;    
    int file_len;
    u8 *tmp;
    int ret = -1;
    u8 boottmp[READSIZE];
    
    if(NULL == path)
    {
        return ret;
    }

	tmp = &boottmp[0];
    
    updata_printf("\r\n-UpdataConfig_Search-\r\n");

    file_len = fs_read_file(path, tmp, READSIZE, 0);
    if(file_len > 0)
    {
        updata_printf("���ɹ�\r\n");
    }
    else
    {
        updata_printf("������\r\n", file_len);
    }
    if(0 == get_updatainfo((s8*)tmp, file_len,&boot_updata_information[0]))
    {
        ret = 0;
    }

    return ret;
}
//=====================================================
s32 bootupdata_save(u32 srcadr, u32 destadr, s32 filelen, u8 *databuf, u32 databufmax)
{
    s32 i;
    s32 j;
    s32 ret;

    i = 0;
//BOOT_DEBUG("%08X, %08X, %d\r\n", srcadr, destadr, filelen);     
    while(i<filelen)
    {
        if((i+databufmax) > filelen)
        {
            j = filelen - i;
        }
        else
        {
            j = databufmax;
        }
        ret = dev_flash_read(srcadr+i, databuf, j);
        if(ret != j)
        {
            updata_printf("%s(%d):err(i=%d, j=%d, flielen=%d,ret=%d)\r\n", __FUNCTION__, __LINE__,
                i, j, filelen, ret);
            return -1;
        }
//BOOT_DEBUG("%08X, %d\r\n",  destadr+i, j);        
        ret = dev_flash_write(destadr+i, databuf, j);
        if(ret != 0)
        {
            updata_printf("%s(%d):err(i=%d, j=%d, flielen=%d,ret=%d)\r\n", __FUNCTION__, __LINE__,
                i, j, filelen, ret);
            return -1;
        }
        i += j;
    }
    return 0;
}
s32 bootupdata_filesave(u8 *filename, u32 srcadr, s32 filelen, u8 *databuf, u32 databufmax)
{
    u8 *data;
    s32 destadr;
    u32 i;
    s32 ret;
    
    
    //if(filetype == BOOTDOWNLOAD_FILE_TYPE_BOOTBIN)//����file.adr�ļ�
    if(0 == strcmp(filename, "/mtd2/apextboot.bin"))
    {
      #if 0  
        //�Ȱ�boot������APP��λ��
        ret = static_vfs_getarea("/mtd2/apapp.bin", 0, &i);
        if((filelen > i)||(ret<0))
        {
            updata_printf("%s(%d):err(name=%s, i=%d, flielen=%d,ret=%d)\r\n", __FUNCTION__, __LINE__,
                filename, i, filelen, ret);
            return -1;   
        }
        destadr = ret;
        ret = bootupdata_save(srcadr, destadr, filelen, databuf, databufmax);
        if(ret < 0)
        {
            updata_printf("%s(%d):err(ret=%d)\r\n", __FUNCTION__, __LINE__, ret);
            return ret;
        }
        bootdownload_savecorefileinfo(BOOTDOWNLOAD_FILE_TYPE_APPBIN, filelen);//gCurFileIndex->filelength);
        //�ٿ�����BOOTλ��
        //ret = static_vfs_getarea("/mtd2/rom.bin", 0, &i);
        ret = static_vfs_getarea("/mtd2/apextboot.bin", 0, &i);
        if((filelen > i) || (ret < 0))
        {
            updata_printf("%s(%d):err(name=%s, i=%d, flielen=%d,ret=%d)\r\n", __FUNCTION__, __LINE__,
                filename, i, filelen, ret);
            return -1;
        }
        //���־
        bootupdata_clear_flg();
//BOOT_DEBUG("srcadr=%08X, destadr=%08X,filelen=%d\r\n", destadr, ret, filelen);        
        ret = bootselfupdata_main(destadr, ret, filelen);       
        return ret;
      #else
        return -1;
      #endif
    }
    else
    {
        ret = static_vfs_getarea(filename, 0, &i);
        if((filelen > i) || (ret < 0))
        {
            updata_printf("%s(%d):err(name=%s, i=%d, flielen=%d,ret=%d)\r\n", __FUNCTION__, __LINE__,
                filename, i, filelen, ret);
            return -1;
        }
        destadr = ret;
//destadr+i("srcadr=%08X, destadr=%08X,filelen=%d\r\n", destadr, ret, filelen);        
        ret = bootupdata_save(srcadr, destadr, filelen, databuf, databufmax);
        if(ret < 0)
        {
            updata_printf("%s(%d):err(ret=%d)\r\n", __FUNCTION__, __LINE__, ret);
            return ret;
        }
    }
    return 0;
      
}
/*************************************************
*************************************************/
s32 bootupdata_checkversion(u8 *filename, u32 srcadr, s32 filelen)
{
  #if 1//(SPIFLASH_EXIST == 1)
    u8 ver_old[32];
    u8 ver_new[32];
    u8 filetype;
    u32 i;
    s32 ret;
	
	
	#ifdef APPPROGRAM_DEBUG
    return 0;
	#endif
	
	
    
    if(0 == strcmp(filename, "/mtd2/apextboot.bin"))
    {
        filetype = 0;
        
    }
    else if(0 == strcmp(filename, "/mtd2/rom.bin")) 
    {
        filetype = 1;
        
    }
	#if 0 //Ӧ�ò��жϰ汾�ţ���������
    else if(0 == strcmp(filename, "/mtd2/apapp.bin"))
    {
        filetype = 2;
        
    }
	#endif
    else
    {
        return 0;
    }
    //����֮ǰ���еĳ���汾��
    ret = static_vfs_getarea(filename, 0, &i);
    if((filelen > i)||(ret<0))
    {
        updata_printf("%s(%d):err(name=%s, i=%d, flielen=%d,ret=%d)\r\n", __FUNCTION__, __LINE__,
            filename, i, filelen, ret);
        return 0;   
    }
    ret = bootupdata_find_fileversion(ret, i, ver_old,filetype);
    if(ret != 1)
    {
        //û�ҵ����������
        updata_printf("%s(%d):old code(%s) no version(addr=%08X)\r\n", __FUNCTION__, __LINE__,
            filename, ret);
        return 0;
    }
    ret = bootupdata_find_fileversion(srcadr, filelen,  ver_new,filetype);
    if(ret != 1)
    {
        //�³���û���ҵ��汾
        updata_printf("%s(%d):new code no version(addr=%08X,len=%d)\r\n", __FUNCTION__, __LINE__,
            srcadr, filelen);
        return -1;
    }
    ret = bootupdata_check_ifnewver(ver_old, ver_new);
    if(0 == ret)
    {
        return 0;
    }
    else
    {
        updata_printf("%s(%d):oldver=%s, newver=%s\r\n", __FUNCTION__, __LINE__,
            ver_old, ver_new);
        return -1;
    }


  #else
	return 0;
  #endif
}
/*************************************************
*************************************************/
s32 bootupdata_file_to_flash(const char *path,updata_info *updata_information)
{
    DOWNFILE DownFileInfo[MAXFILE_NUM];  //16*20 = 320bytes
    int fd, i, count;  
    s32 j;
    u32 total_len, file_crc;
    int file_len;
    u8 *data;
    u16 bagno;
    int ret = 0;
    u8 flg;
    u32 file_addr;
	TERMINAL_PK_TYPE pktype;
    u8 bootdata[READSIZE];
	

    updata_printf("\r\n--------------------------\r\n");
    //data = (u8 *)k_malloc(READSIZE);
	data = (u8 *)&bootdata[0];
	#if 0  //sxl20170731 boot��ʹ�ö�̬�ڴ�
    if(data == NULL)
    {
        updata_printf("�����ڴ�ʧ��\r\n");
        return -1;
    }
	#endif

    for(i=0; i<MAXFILE_NUM; i++)
    {        
        if(updata_information[i].cfgvalidflg == 0)
            continue;
        
        memset(DownFileInfo[i].filename, 0, 50);
        strcpy((s8*)DownFileInfo[i].filename, (s8*)updata_information[i].filedir);
//BOOT_DEBUG("dir=%s, name=%s\r\n", (s8*)updata_information[i].filedir, (s8*)updata_information[i].filename);        
        if(updata_information[i].filename[0] != '/')
        {
            DownFileInfo[i].filename[strlen((s8*)DownFileInfo[i].filename)] = '/';
        }
        strcpy(&DownFileInfo[i].filename[strlen((s8*)DownFileInfo[i].filename)], (s8*)updata_information[i].filename);
//BOOT_DEBUG("updata file dir:%s\r\n", DownFileInfo[i].filename);
        //
        file_addr = updata_information[i].fileadr;
        total_len = 0;
        file_crc = 0xFFFFFFFF;
        flg = 0; 
        while(total_len<updata_information[i].filelen)
        {
            
            if((updata_information[i].filelen - total_len) > READSIZE)
            {
                j = READSIZE;
            }
            else
            {
                j = updata_information[i].filelen - total_len;
            }
            //file_len = fs_read_file((s8*)DownFileInfo[i].filename, data, READSIZE, total_len);
            
            file_len = fs_read_file(path, data, j, total_len+file_addr);
            if(file_len>0)
            {
                total_len += file_len;
                file_crc = dev_maths_calc_crc32(file_crc, data, file_len);   //���ص�ʱ����ǩ����������CRC
                //file_crc = updata_information[i].filecrc;
				//file_crc ^= 0xffffffff;
            }
            else
            {
                BOOT_DEBUG("������\r\n", file_len);
                BOOT_DEBUG("name=%s,total_len=%d\r\n", DownFileInfo[i].filename, total_len);
                if(total_len==0)
                {
                    flg = 1;    //�ļ������ڣ���û����Ч����
                }
                break;                
            }
        } 
        if(flg)
        {
            updata_information[i].updata_state = 8;   //file not exist
            continue;
        }
        
        //У�������ļ�����ʵ�ļ��ĳ���
        if(total_len != updata_information[i].filelen)
        {
            BOOT_DEBUG("calculate filelen :%d, record len:%d\r\n", total_len, updata_information[i].filelen);
            updata_information[i].updata_state = 7;   //file len failed
            ret = -1;
        }
            
        //У�������ļ�����ʵ�ļ���CRC
        file_crc ^= 0xffffffff;
        if(file_crc != updata_information[i].filecrc)
        {
            BOOT_DEBUG("calculate crc 1:%08x, record crc:%08x\r\n", file_crc, updata_information[i].filecrc);
            updata_information[i].updata_state = 6;   //file crc failed
            ret = -1;
        }  
        DownFileInfo[i].filelength = total_len;
        DownFileInfo[i].crc = file_crc;
      #if 0  
        DownFileInfo[i].addr = static_vfs_getarea((const char *)DownFileInfo[i].filename, 0, &j);//updata_information[i].fileadr;
        ret = bootdownload_GetFiletype(DownFileInfo[i].filename, &DownFileInfo[i].filetype);
BOOT_DEBUG("name=%s,ret=%d,filetype=%d\r\n", DownFileInfo[i].filename, ret, DownFileInfo[i].filetype);    
        if((DownFileInfo[i].addr<0) || (ret<0))
        {
            updata_printf("%s(%d):addr=%d, ret=%d\r\n", __FUNCTION__, __LINE__, DownFileInfo[i].addr, ret);
            updata_information[i].updata_state = 6;   //file crc failed
            ret = -1;
        }
      #else  
        DownFileInfo[i].addr = bootdownload_GetFileAddr(DownFileInfo[i].filename, NULL, &DownFileInfo[i].filetype);
        //ret = bootdownload_GetFiletype(DownFileInfo[i].filename, &DownFileInfo[i].filetype);
//BOOT_DEBUG("name=%s,addr=%d,filetype=%d\r\n", DownFileInfo[i].filename, DownFileInfo[i].addr, DownFileInfo[i].filetype);    
        if(DownFileInfo[i].addr<0)
        {
            //����Ҫ�������ļ���У�飬��Ϊ���سɹ�
            BOOT_DEBUG("addr=%d\r\n", DownFileInfo[i].addr);
            updata_information[i].updata_state = 1;   //file crc failed
            continue;
            //ret = -1;
        }
      #endif 
        gCurFileIndex = &DownFileInfo[i];
        //RSAУ���ļ���������
        
        if(ret == 0)
        { 
          #if 0
            updata_printf("��%d���ļ��Ϸ�\r\n", i);
            //�����ļ�������ļ��Ϸ�
            total_len = 0;
            bagno = 0;
        BOOT_DEBUG("filelen=%d, adr=%08X\r\n", updata_information[i].filelen, DownFileInfo[i].addr);
            while(total_len<updata_information[i].filelen)
            {
            
                if((updata_information[i].filelen - total_len) > READSIZE)
                {
                    j = READSIZE;
                }
                else
                {
                    j = updata_information[i].filelen - total_len;
                }
                //file_len = fs_read_file((s8*)DownFileInfo[i].filename, data, READSIZE, total_len);
                file_len = fs_read_file(path, data, j, total_len+updata_information[i].fileadr);
                if(file_len>0)
                {
                    total_len += file_len;
BOOT_DEBUG("write:addr=%08X,type=%d\r\n", gCurFileIndex->addr, gCurFileIndex->filetype);                    
                   #if 1  
                    if(bootdownload_fwrite(data, bagno, file_len, gCurFileIndex->addr, gCurFileIndex->filetype) <= 0)
                    //if(file_len != ddi_file_insert(gCurFileIndex->filename, data, file_len, bagno*READSIZE))
                    {
                        ret = -1;
                        updata_information[i].updata_state = 5;   //write FLS failed
                        break;
                    }
                   #endif
BOOT_DEBUG("bagno=%d,len=%d, addr=%08X, filetype=%d\r\n", bagno, file_len, gCurFileIndex->addr, gCurFileIndex->filetype);  
BOOT_DEBUGHEX(NULL, data, file_len);                   
                }
                else
                {
                    updata_printf("������\r\n", file_len);
                    break;                
                }
                bagno ++;
            }
            if(ret == 0)
            {
                
                //������У�飬
                updata_printf("��%d���ļ�д��ɹ�\r\n", i);
                //calculate the CRC
                file_crc = 0xffffffff;
                file_len = 0;
                bagno = 0;
                while(1)
                {
                    if((total_len-file_len) > READSIZE)
                    {
                        j = READSIZE;
                    }
                    else
                    {
                        j = total_len-file_len;
                    }
                    count = bootdownload_fread(data, bagno, j, gCurFileIndex->addr, gCurFileIndex->filetype);
                    //count = ddi_file_read(gCurFileIndex->filename, data, j, bagno*READSIZE);
                    if(count<=0)
                    {
                        ret = -1;
                        break;
                    }
                    //sys_uart_printf_format_u8(data, count);
                    file_len += count;
                    file_crc = dev_maths_calc_crc32(file_crc, data, count);
                    //bios_promptexecution(EXECUTION_DOWNFILEOPS);

                    bagno ++;
                }
                file_crc ^= 0xffffffff;
                if(file_crc != updata_information[i].filecrc)
                {
                    updata_printf("flash file crc :%08x, config file crc:%08x\r\n", file_crc, updata_information[i].filecrc);
                    updata_information[i].updata_state = 4;   //check crc failed
                    ret = -1;
                }  
//BOOT_DEBUG("CRC=%08X, %08X\r\n", file_crc, updata_information[i].filecrc);                
                //----------------------------------
                //check file authentication 
                if(ret == 0)
                {
                    updata_printf("��%d���ļ�У��ɹ�\r\n", i);
                    
                    updata_printf("process file authentication \r\n");
                  
                    //ret = bootdownloadauthen_processfileauthentication();
                    ret = bootdownloadauthen_fileauthen(updata_information[i].fileadr, updata_information[i].filelen,PK_TRENDIT);
                    
                    updata_printf("\r\nbios_process file authentication ret = %d\r\n",ret);

                    if(ret == 1)
                    {

                        //bios_eraseappfile(gCurFileIndex->addr);

                        updata_information[i].updata_state = 3;   //authentication failed
                        updata_printf("����ʧ�� У��ʧ��\r\n");
                    }
                    else
                    {
                      #if 0  
                        dryice_authen_success();
                      #endif
                        //deal download a file success
                        ret = bootdownload_downloadfilesave();

                        updata_printf("\r\nbios_downloadfilesave ret = %d\r\n",ret);

                        if(ret != 0)
                        {   
                            if(ret == 2)
                            {
                                updata_information[i].updata_state = 2; //save authentication failed
                                updata_printf("����ʧ�� ����ʧ��\r\n");
                            }
                            else
                            {
                                continue;
                            }
                        }
                        else
                        {
                            updata_information[i].updata_state = 1; //updata ok
                        }

                        updata_printf("\r\n down file %08x %d success! \r\n",gCurFileIndex->addr,gCurFileIndex->failedflag);

                    }
                }
            }
          #else
            BOOT_DEBUG("��%d���ļ�У��ɹ�\r\n", i);
                
            BOOT_DEBUG("process file authentication \r\n");
//BOOT_DEBUG(%08X, len=%d\r\n", updata_information[i].fileadr, updata_information[i].filelen);              
            j = static_vfs_getarea(path, 0, &file_len);
          
            if(j > 0)
            {
                //ret = bootdownloadauthen_processfileauthentication();
              #ifdef EXTERNAL_SPIFLASH_ENABLE
				if(DownFileInfo[i].filetype == BOOTDOWNLOAD_FILE_TYPE_TREDNITPK)   //download XGD public key
			    {
					pktype = PK_TRENDIT_ROOT;
			    }
			    else if(DownFileInfo[i].filetype == BOOTDOWNLOAD_FILE_TYPE_ACQUIRERPK)
			    {
					pktype = PK_ACQUIRER_ROOT;
			    }
			 #else
				pktype = PK_TRENDIT_ROOT;
			 #endif
				BOOT_DEBUG("pktype = %d\r\n",pktype);
                ret = bootdownloadauthen_fileauthen((j+updata_information[i].fileadr), updata_information[i].filelen,pktype);
                
                
                BOOT_DEBUG("bios_process file authentication ret = %d\r\n",ret);

                if(ret == 1)
                {

                    //bios_eraseappfile(gCurFileIndex->addr);

                    updata_information[i].updata_state = 3;   //authentication failed
                    BOOT_DEBUG("����ʧ�� У��ʧ��\r\n");
                }
                else
                {
                  #if 0  
                    dryice_authen_success();
                  #endif
                    //deal download a file success
                    //���Ӱ汾����
                    ret = bootupdata_checkversion(gCurFileIndex->filename, (j+updata_information[i].fileadr), updata_information[i].filelen);
                    if(ret != 0)
                    {
                        //�汾У������������
                        updata_information[i].updata_state = 9;
                        BOOT_DEBUG("version old\r\n");
                        continue;
                    }
                    //ret = bootdownload_downloadfilesave();
                    //#ifdef EXTERNAL_SPIFLASH_ENABLE
                    ret = bootupdata_filesave(gCurFileIndex->filename, (j+updata_information[i].fileadr), updata_information[i].filelen, data, READSIZE);
                    //#else
                    //ret = 0;
					//#endif
					BOOT_DEBUG("bootupdata_filesave ret = %d\r\n",ret);

                    if(ret != 0)
                    {   
                        if(ret == 2)
                        {
                            updata_information[i].updata_state = 2; //save authentication failed
                            BOOT_DEBUG("����ʧ�� ����ʧ��\r\n");
                        }
                        else
                        {
                            continue;
                        }
                    }
                    else
                    {
BOOT_DEBUG("filetype=%d\r\n", gCurFileIndex->filetype);   
                        #ifdef EXTERNAL_SPIFLASH_ENABLE
                        ret = bootdownload_downloadfilesave();
						#else
						ret = 0;
						#endif
                        if(ret == 0)
                        {
                            updata_information[i].updata_state = 1; //updata ok
                        }
                        else
                        {
                            updata_information[i].updata_state = 2; //save authentication failed
                            BOOT_DEBUG("����ʧ�� ����ʧ��\r\n");
                        }
                    }

                    BOOT_DEBUG("down file %08x %d success! \r\n",gCurFileIndex->addr, updata_information[i].filelen);

                }
            }   
          #endif
        }
        //ɾ���ļ�
        fs_delete_file((s8*)DownFileInfo[i].filename);
    }

    k_free(data);

    
    return ret;
}

//=====================================================
u32 bootupdata_check_flg(void)
{
  #if 0  
    s32 ret;
    u8 tmp[32];

    ret = fs_read_file("/mtd2/updateflag", tmp, 10, 0);
    if((ret == 10) && (0 == memcmp(tmp, "updateflag", 10)))
    {
        //У��ɹ�,��Ҫ����
        return 0;
    }
    else
    {
        return 1;
    }
  #endif
    return dev_misc_updateflg_check();
}
void bootupdata_clear_flg(void)
{
  #if 0  
    u8 tmp[32];

    memset(tmp, 0xff, 10);
    fs_write_file("/mtd2/updateflag", tmp, 10);
  #endif
    dev_misc_updateflg_set(0);
}

//=====================================================
void bootupdata_deal_result(updata_info *updata_information)
{
    s32 i,j;
    s32 k;
    u8 disp[4][32];
    s32 ret;
    u32 key;
    u8 flg = 1;

    
    for(i=0, j=0; i< MAXFILE_NUM; i++)
    {
        if(updata_information[i].cfgvalidflg == 1)
        {
            switch(updata_information[i].updata_state)
            {
            case 1:
                sprintf((s8*)disp[j], "%s OK", updata_information[i].filename);
                break;
            case 2:
                sprintf((s8*)disp[j], "%s save NG", updata_information[i].filename);
                break;
            case 3:
                sprintf((s8*)disp[j], "%s authen NG", updata_information[i].filename);
                break;
            case 4:
                sprintf((s8*)disp[j], "%s crc NG", updata_information[i].filename);
                break;
            case 5:
                sprintf((s8*)disp[j], "%s write FLS NG", updata_information[i].filename);
                break;
            case 6:
                sprintf((s8*)disp[j], "%s file crc NG", updata_information[i].filename);
                break;
            case 7:
                sprintf((s8*)disp[j], "%s file len NG", updata_information[i].filename);
                break;
            case 8:
                sprintf((s8*)disp[j], "%s file not exist", updata_information[i].filename);
                break;
            case 9:
                sprintf((s8*)disp[j], "%s file VER ERR", updata_information[i].filename);
                break;
            default:
                sprintf((s8*)disp[j], "%s NG", updata_information[i].filename);
                break;
            }
            j++; 
        }
    }
    if(j == 0)
    {
        dev_lcd_clear_ram();
        dev_lcd_brush_screen();
        return;
    }
        
    i = 0;
    while(1)
    {
        if(flg == 1)
        {
            flg = 0;
            dev_lcd_clear_ram();
            dev_lcd_fill_rowram(0, 0, disp[i], FDISP); 
            k = j-i;
            if(k>1)
            {
                dev_lcd_fill_rowram(1, 0, disp[i+1], FDISP); 
            }
            #if(LCD_MAX_LINE == 64)
                if(k>2)
                {
                    dev_lcd_fill_rowram(2, 0, disp[i+2], FDISP); 
                }
                if(k>3)
                {
                    dev_lcd_fill_rowram(3, 0, disp[i+3], FDISP); 
                }
                if(k>4)
                {
                    dev_lcd_fill_rowram(4, 0, disp[i+4], FDISP); 
                }
            #endif
            dev_lcd_brush_screen();
            dev_keypad_clear();
        }
        ret = dev_keypad_read_beep(&key);
        if(ret)
        {
            switch(key)
            {
            case DIGITAL2:      //�Ϸ�
                if(i<=2)
                {
                    i = 0;
                }
                else
                {
                    i -= 2;
                }
                flg = 1;
                break;
            case DIGITAL8:
              #if(LCD_MAX_LINE == 64)
                k = 5;  
              #else
                k = 2;
              #endif
                if((i+k)<j)
                {
                    i += k;
                }
                flg = 1;
                break;
            case ENTER:
              #if(LCD_MAX_LINE == 64)
                k = 5;  
              #else
                k = 2;
              #endif
                if((i+k)<j)
                {
                    i += k;
                }
                else
                {
                    return;
                }
                flg = 1;
                break;
            case ESC:
                return;
            }
        }
    }
}
//=====================================================
u32 bootupdata_deal(void)
{  
    u8 charge;
    u8 vollv;

    //�жϸ��±�־
    if(0 != bootupdata_check_flg())    
    {   //����Ҫ����
        
        return 1;
    }
        
    bootdownload_initialcandownfileinfo();
    if(0 == bootupdata_config_Search("/mtd2/apupdate"))
    {
      #if 0
        dev_power_recharge(&charge);
        if(0 == charge)
        {
            //û�����
            dev_power_battery(&vollv);
            if(vollv <= 1)
            {
                dev_user_delay_ms(100);
                dev_power_battery(&vollv);
                if(vollv<=1)
                {
                    dev_lcd_clear_ram();
                    dev_lcd_fill_rowram(0, 0, "��ص�����", CDISP);
                    dev_lcd_fill_rowram(1, 0, "���������!", CDISP);
                    dev_lcd_brush_screen();
                    drv_lcd_bl_ctl(1);
                    dev_user_delay_ms(2000);
                    //SCU->SCFGVCCACTRL |= 0x09;//��VRmain
                    dev_misc_poweroff();
                    return 1;
                }
            }
        }
        dev_lcd_clear_ram();
        dev_lcd_fill_rowram(0, 0, "Updating...", CDISP);
        dev_lcd_fill_rowram(1, 0, "Do not power off!", CDISP);
        dev_lcd_brush_screen();
        drv_lcd_bl_ctl(1);
      #endif

        BOOT_DEBUG("��ʼ��������\r\n");  
        bootupdata_file_to_flash("/mtd2/apupdate",&boot_updata_information[0]);
        //���־
        bootupdata_clear_flg();
        bootupdata_deal_result(&boot_updata_information[0]);
		
		
    }  
    return 0;
}
/*******************************************************************
Author:   
Function Name:  u8 bootupdata_find_fileversion(u32 addr,u32 length,u8 *version,u8 type)
Function Purpose:
Input Paramters:  type:0 boot, 1 core,2 app
Output Paramters:N/A
Return Value:      N/A
Remark: N/A
Modify: N/A
********************************************************************/
u8 bootupdata_find_fileversion(u32 addr,u32 length,u8 *version,u8 type)
{
	u8 parsestep = 0;
	u8 findrlt = 0;
	u32 i;
	u8 readdata;
	u8 tempver[16];

	if(0 == type)//boot �汾����ƫ��0x820λ�á�
	{
		length = 20;
		addr = addr + 0x820;
	}
	else if(1 == type)//core �汾����ƫ��0x820λ�á�
	{
		length = 20;
		addr = addr + 0x820;
	}
	else if(2 == type)//app �汾���ڿ�ͷ��
	{
		length = 20;
	}

	for(i = 0;i < length; i++)
	{
		
		if(dev_flash_read(addr+i,&readdata,1) <= 0)
		{			
			findrlt = -1;
			break;
		}
		switch(parsestep)
		{
			case 0:	// V
				if(readdata == 'V')
				{				
					tempver[parsestep] = readdata;
					parsestep = 1;					
				}
				break;
			case 1: //����
				if((readdata >= '0') && (readdata <= '9'))
				{					
					tempver[parsestep] = readdata;
					parsestep = 2;		
				}
				else if(readdata == 'V')
				{
					
					parsestep = 1;	
				}
				else
				{
					parsestep = 0;
				}
				break;
			case 2: //����
				if((readdata >= '0') && (readdata <= '9'))
				{				
					tempver[parsestep] = readdata;
					parsestep = 3;		
				}
				else
				{
					parsestep = 0;
				}
				break;
			case 3: //����
				if(((type == 0)&&((readdata == 'B')||(readdata == 'b')))||
					((type == 1)&&((readdata == 'C')||(readdata == 'c')))||
					((type == 2)&&((readdata == 'A')||(readdata == 'a'))))
				{				
					tempver[parsestep] = readdata;
					parsestep = 4;		
				}
				else
				{
					parsestep = 0;
				}
				break;
			case 4: //Ԥ��
			
			
				tempver[parsestep] = readdata;
				parsestep = 5;
				break;
			case 5: //����
				if((readdata >= '0') && (readdata <= '9'))
				{					
					tempver[parsestep] = readdata;
					parsestep = 6;		
				}
				else
				{
					parsestep = 0;
				}
				break;
			case 6: //����
				if((readdata >= '0') && (readdata <= '9'))
				{
					
					tempver[parsestep] = readdata;
					parsestep = 7;		
				}
				else
				{
					parsestep = 0;
				}
				break;
			case 7: //����
				if((readdata >= '0') && (readdata <= '9'))
				{					
					tempver[parsestep] = readdata;
					parsestep = 8;		
				}
				else
				{
					parsestep = 0;
				}
				break;
			case 8: //����
				if((readdata >= '0') && (readdata <= '9'))
				{					
					tempver[parsestep] = readdata;
					parsestep = 9;		
				}
				else
				{
					parsestep = 0;
				}
				break;
			case 9: //����
				if((readdata >= '0') && (readdata <= '9'))
				{					
					tempver[parsestep] = readdata;
					parsestep = 10;		
				}
				else
				{
					parsestep = 0;
				}
				break;
			case 10: //����
				if((readdata >= '0') && (readdata <= '9'))
				{
					tempver[parsestep] = readdata;
					parsestep = 11;		
				}
				else
				{
					parsestep = 0;
				}
				break;
			case 11: //�汾
				if((readdata >= '0') && (readdata <= '9'))
				{				
					tempver[parsestep] = readdata;
					parsestep = 12;		
				}
				else
				{
					parsestep = 0;
				}
				break;
			case 12: //�汾
				if((readdata >= '0') && (readdata <= '9'))
				{						
					tempver[parsestep] = readdata;
					parsestep = 13;		
				}
				else
				{
					parsestep = 0;
				}
				break;
			case 13: //�汾
				if((readdata >= '0') && (readdata <= '9'))
				{
					tempver[parsestep] = readdata;
					parsestep = 0;
					findrlt = 1;
					memcpy(version,tempver,14);
					version[14] = 0;
					//BOOT_DEBUG("updatever:%s\r\n",tempver);
					return findrlt;					
				}
				else
				{					
					parsestep = 0;
				}
				break;
			default:
				break;

		}

	}

	return findrlt;
}

//ddi_misc_get_bootver(u8 * p);
/*******************************************************************
Author:   
Function Name: u8 bootupdata_check_ifnewver(u8 *currver,u8 *updatever)
Function Purpose:
Input Paramters: 
Output Paramters:N/A
Return Value:  0 updatever is newer than currver.
Remark: N/A
Modify: N/A
********************************************************************/
u8 bootupdata_check_ifnewver(u8 *currver,u8 *updatever)
{
  #if 0  
	if(updatever[0] != currver[0])// V
	{
		return 1;
	}
	if((updatever[1] != currver[1]) || (updatever[2] != currver[2])) // ����
	{
		return 1;
	}
	if(updatever[3] != currver[3])  // B C A
	{
		return 1;
	}
	if(updatever[4] != currver[4])  
	{
		return 1;
	}
   #endif
    if(memcmp(updatever, currver, 5)!=0)
    {
        return 1;
    }
    
	if(strcmp(&updatever[5],&currver[5]) < 0) //160919
	{
		BOOT_DEBUG("\r\n");
		return 1;
	}
	if(strcmp(&updatever[11],&currver[11]) < 0) //100
	{
	    BOOT_DEBUG("\r\n");
		return 1;
	}
    
	return 0;
}


s32 bootupdata_checkbinfilecrc(u8 *filename,u32 crc)
{
	
	s32 ret = 0;
	s32 offset = 0;
	u8 appinfo[MCUAPPINFO_MAX];
	u8 tempdata[5];
	
	if(strcmp(filename,"rom.bin") == 0)
    {
		offset = 0;
    }
    else if(strcmp(filename,"apapp.bin") == 0)
    {
		offset = 4;
    }
	else
	{
		return 0;
	}
	
	
	tempdata[0] = (u8)(crc>>24);
	tempdata[1] = (u8)(crc>>16);
	tempdata[2] = (u8)(crc>>8);
	tempdata[3] = (u8)(crc);
	
	
	memset(appinfo,0,sizeof(appinfo));
	ret = dev_misc_machineparam_get(MACHINE_PARAM_APPINFO,appinfo,MCUAPPINFO_MAX,0);
	if(ret == 0)
	{
		if(memcmp(tempdata,&appinfo[offset],4))
		{
			ret = -1;
		}
		
	}
	

	return ret;

	
}
//#################################################################
//#################################################################
s32 bootupdata_file_to_flash_1(const char *path,updata_info *updata_information)
{
    DOWNFILE DownFileInfo[MAXFILE_NUM];  //16*20 = 320bytes
    int fd, i, count;  
    s32 j;
    u32 total_len, file_crc;
    int file_len;
    u8 *data;
    u16 bagno;
    int ret = 0;
    u8 flg;
    u32 file_addr;
	TERMINAL_PK_TYPE pktype;
    u8 bootdata[READSIZE];
	
    
    data = (u8*)&bootdata[0];
    for(i=0; i<MAXFILE_NUM; i++)
    {
        if(updata_information[i].cfgvalidflg == 0)
            continue;

        memset(DownFileInfo[i].filename, 0, 50);
        strcpy((s8*)DownFileInfo[i].filename, (S8*)updata_information[i].filedir);
        if(updata_information[i].filename[0] != '/')
        {
            DownFileInfo[i].filename[strlen((s8*)DownFileInfo[i].filename)] = '/';
        }
        strcpy(&DownFileInfo[i].filename[strlen((s8*)DownFileInfo[i].filename)], (s8*)updata_information[i].filename);

        file_addr = updata_information[i].fileadr;
        total_len = 0;
        file_crc = 0xFFFFFFFF;
        flg = 0; 
        //�����ļ���CRC
        while(total_len<updata_information[i].filelen)
        {
            ret = 0;
            if((updata_information[i].filelen-total_len) > READSIZE)
            {
                j = READSIZE;
            }
            else
            {
                j = updata_information[i].filelen-total_len;
            }

            file_len = fs_read_file(path, data, j, total_len+file_addr);
            if(file_len > 0)
            { 
                total_len += file_len;
                file_crc = dev_maths_calc_crc32(file_crc, data, file_len);   
            }
            else
            {
                BOOT_DEBUG("������\r\n", file_len);
                BOOT_DEBUG("name=%s,total_len=%d\r\n", DownFileInfo[i].filename, total_len);
                if(total_len==0)
                {
                    flg = 1;    //�ļ������ڣ���û����Ч����
                }
                break; 
            }
        }
        if(flg)
        {
            updata_information[i].updata_state = 8;
            continue;
        }
        //У�������ļ�����ʵ�ļ��ĳ���
        if(total_len != updata_information[i].filelen)
        {
            BOOT_DEBUG("calculate filelen :%d, record len:%d\r\n", total_len, updata_information[i].filelen);
            updata_information[i].updata_state = 7;   //file len failed
            ret = -1;
			continue;
        }
            
        //У�������ļ�����ʵ�ļ���CRC
        file_crc ^= 0xffffffff;
        if(file_crc != updata_information[i].filecrc)
        {
            BOOT_DEBUG("calculate crc 1:%08x, record crc:%08x\r\n", file_crc, updata_information[i].filecrc);
            updata_information[i].updata_state = 6;   //file crc failed
            ret = -1;
			continue;
        }  


  #if 0		
		if(bootupdata_checkbinfilecrc(updata_information[i].filename,file_crc))
		{
			BOOT_DEBUG("\r\nfile crc is modified\r\m");
			updata_information[i].updata_state = 6;   //file crc failed
            ret = -1;
			continue;
		}
  #endif	
        DownFileInfo[i].filelength = total_len;
        DownFileInfo[i].crc = file_crc;
        
        DownFileInfo[i].addr = bootdownload_GetFileAddr(DownFileInfo[i].filename, NULL, &DownFileInfo[i].filetype);
        //ret = bootdownload_GetFiletype(DownFileInfo[i].filename, &DownFileInfo[i].filetype);
//BOOT_DEBUG("name=%s,addr=%d,filetype=%d\r\n", DownFileInfo[i].filename, DownFileInfo[i].addr, DownFileInfo[i].filetype);    
        if(DownFileInfo[i].addr<0)
        {
            BOOT_DEBUG("addr=%d\r\n", DownFileInfo[i].addr);
            updata_information[i].updata_state = 6;   //file crc failed
            ret = -1;
			continue;
        }

        gCurFileIndex = &DownFileInfo[i];
        if(ret == 0)
        {
            //��ȡԴ�ļ���ַ,����
            j = static_vfs_getarea(path, 0, &file_len);

            //�жϰ汾
            ret = bootupdata_checkversion(DownFileInfo[i].filename, (j+updata_information[i].fileadr), updata_information[i].filelen);
			if(ret != 0)
            {
                //�汾У������������
                updata_information[i].updata_state = 9;
                BOOT_DEBUG("version old\r\n");
                continue;
            }
            ret = bootupdata_filesave(gCurFileIndex->filename, (j+updata_information[i].fileadr), updata_information[i].filelen, data, READSIZE);
            BOOT_DEBUG("bootupdata_filesave ret = %d\r\n",ret);
            if(ret != 0)
            {   
                if(ret == 2)
                {
                    updata_information[i].updata_state = 2; //save authentication failed
                    BOOT_DEBUG("����ʧ�� ����ʧ��\r\n");
                }
                else
                {
                    continue;
                }
            }
            else
            {
                updata_information[i].updata_state = 1;
            }
        }
        //ɾ���ļ�
        fs_delete_file((s8*)DownFileInfo[i].filename);
    }
    return ret;
}

u32 bootupdata_deal_withoutdisplay(void)
{
    u8 charge;
    u8 vollv;

    //�жϸ��±�־
    if(0 != bootupdata_check_flg())    
    {   //����Ҫ����
        
        return 1;
    }
    
    //dev_power_switch_ctl(1);        //�����Դ  //sxl?2019
    
    bootdownload_initialcandownfileinfo();
    if(0 == bootupdata_config_Search("/mtd2/apupdate"))   //sxl?2019Ҫ�Ƚ�hash,���Ƿ��ⲿ����
    {         
      #if 0
        dev_power_recharge(&charge);
//BOOT_DEBUG("charge=%d\r\n", charge); 
//        charge = 1;
        if(0 == charge)
        {
            //û�����
            dev_power_battery(&vollv);
            if(vollv<=1)
            {
                dev_user_delay_ms(100);
                dev_power_battery(&vollv);
                if(vollv<=1)
                {
                    //dev_led_red_ctl(1);
                    //dev_led_green_ctl(0);
                    //drv_lcd_bl_ctl(1);
                    dev_user_delay_ms(2000);
                    //SCU->SCFGVCCACTRL |= 0x09;//��VRmain
                    //dev_power_switch_ctl(0);        //�ػ�????   //sxl?2019
                    return 1;
                }
            }
        }
      #endif
	    //dev_led_red_ctl(0);
        //dev_led_green_ctl(1);

        BOOT_DEBUG("��ʼ��������\r\n");
        bootupdata_file_to_flash_1("/mtd2/apupdate",&boot_updata_information[0]);
        //���־
        bootupdata_clear_flg();
        //bootupdata_deal_result();   
//        dev_led_sta_set(LED_RED_NO, 1);
        //dev_led_green_ctl(1);
        //����
        dev_misc_reboot(ST_REBOOT);//dev_smc_systemsoftreset();
        
    }   
    return 0;
}

#endif




