/********************************Copyright ( c ) ********************************
**                  DAQU 
** 						
**                        
**
**Created By:		 
** Created Date:	 
** Version:		
** Description:	该文件混合设备相关接口
  
****************************************************************************/
#include "devglobal.h"
#include "drv_misc.h"
#if 1
/****************************************************************************
**Description:	   读机器参数
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170614
**---------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
static s32 dev_misc_machineparam_read(u8 *param, u16 *paramlen, u8 type)
{
    s32 ret;
    u16 len;
    u8  crc[2];

    if(*paramlen > MACHINE_PARAM_SPACE)
    {
		MISC_DEBUG("paramlen=%d\r\n", *paramlen);
        return -1;
    }
    if(type==0)
    {
        ret = fs_read_file(MACHINE_PARAM_FILENAME, param, 4, 0);
    }
    else
    {
        ret = fs_read_file(MACHINE_PARAM_FILENAMEBAK, param, 4, 0);
    }
    if(ret <= 0)
    {
        MISC_DEBUG("File(%d) read  err(%d)!\r\n", type, ret);
        return -1;
    }
    len = (((u16)param[2])<<8) + param[3];
    if(((len+4)>=MACHINE_PARAM_SPACE) || (len<MCU_TAMPERMAN_STA))
    {
        MISC_DEBUG("paramlen(%d) err!\r\n", len);
        return -1;
    }
    if((len+4) > (*paramlen))
    {
        MISC_DEBUG("len(%d,%d) err!\r\n", len+4, *paramlen);
        return -1;
    }
    *paramlen = len+4;
    if(type==0)
    {
        ret = fs_read_file(MACHINE_PARAM_FILENAME, (u8*)&param[4], len, 4);
    }
    else
    {
        ret = fs_read_file(MACHINE_PARAM_FILENAMEBAK, (u8*)&param[4], len, 4);
    }
    if(ret <= 0)
    {
        MISC_DEBUG("File(%d) read  err(%d)!\r\n", type, ret);
        return -1;
    }
    //crc校验
    dev_maths_calc_crc16(0, &param[4], len, crc);
    if(0 != memcmp(crc, &param[0], 2))
    {
        MISC_DEBUG("crc(%02X%02X,%02X%02X) ERR!\r\n", crc[0], crc[1], param[0], param[1]);
        return -1;
    }
    if(0 == strcmp(&param[4], MACHINE_PARAM_FLG0))
    {
        return 0;
    }
    if(0 == strcmp(&param[4], MACHINE_PARAM_FLG1))
    {
        return 1;
    }
    return -1;
}
/****************************************************************************
**Description:	   写机器参数
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170614
**---------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
static s32 dev_misc_machineparam_write(u8* param, u16 paramlen, u8 type)
{
    s32 ret;
    u16 len;

    len = paramlen-4;
    param[2] = (len>>8)&0xFF;
    param[3] = len&0xFF;
    //写标志
    strcpy(&param[4], MACHINE_PARAM_FLG1);
    memcpy(&param[10], MACHINE_PARAM_VER, 2);
    //计算CRC
    dev_maths_calc_crc16(0, &param[4], len, &param[0]);
    //写参数 
    if(type==0)
    {
        ret = fs_write_file(MACHINE_PARAM_FILENAME, param, paramlen);
    }
    else
    {
        ret = fs_write_file(MACHINE_PARAM_FILENAMEBAK, param, paramlen);
    }
	MISC_DEBUG("machineparam write = %d %d\r\n",ret,type);
    if(ret != paramlen)
    {
        MISC_DEBUG("File(%s) write err(%d)!\r\n", MACHINE_PARAM_FILENAME, ret);
        return -1;
    }
    return 0;
}
/****************************************************************************
**Description:	   写机器参数
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170614
**---------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
s32 dev_misc_machineparam_init_cos(void)
{
    s32 ret0;
    s32 ret1;
    str_machine_param_t *lp_machineparam;
    str_machine_param_t *lp_machineparam1;
    u16 len0;
    u16 len1;
    u16 crc0;
    u16 crc1;
    u8 initflg;
   
    lp_machineparam = k_malloc(STR_MACHINE_PARAM_LEN);
    lp_machineparam1 = k_malloc(STR_MACHINE_PARAM_LEN);
    
    memset(lp_machineparam1, 0xFF, STR_MACHINE_PARAM_LEN);
    len1 = STR_MACHINE_PARAM_LEN;
    ret1 = dev_misc_machineparam_read((u8*)lp_machineparam1, &len1, 1);
	MISC_DEBUG("\r\nmachineparam_read:%d\r\n",ret1);
    if(ret1 >= 0)
    {
        crc1 = (lp_machineparam1->m_crc[0]<<8)+lp_machineparam1->m_crc[1];
        //len1 = (lp_machineparam1->m_paramlen[0]<<8)+lp_machineparam1->m_paramlen[1];
    }
    memset(lp_machineparam, 0xFF, STR_MACHINE_PARAM_LEN);
    len0 = STR_MACHINE_PARAM_LEN;
    ret0 = dev_misc_machineparam_read((u8*)lp_machineparam, &len0, 0);
	MISC_DEBUG("\r\nmachineparam_read1:%d\r\n",ret1);
    if(ret0 >= 0)
    {
        crc0 = (lp_machineparam->m_crc[0]<<8)+lp_machineparam->m_crc[1];
        //len0 = (lp_machineparam->m_paramlen[0]<<8)+lp_machineparam->m_paramlen[1];
    }
    initflg = 0;
    while(1)
    {
        if(ret1 < 0)
        {
            if(ret0<=0)
            {
                //两组参数均错，需要初始化
                //备份参数错误,主参数是初始化版本
                initflg = 1;
                break;
            }
            else // if(ret0==1)
            {
                //备份参数错误，主参数正确，需要备份
                dev_misc_machineparam_write((u8*)lp_machineparam, STR_MACHINE_PARAM_LEN, 1);
                break;
            }
        }
        else 
        {
            if(ret0 < 0)
            {
                //主参数错，备份参数正确
                dev_misc_machineparam_write((u8*)lp_machineparam1, STR_MACHINE_PARAM_LEN, 0);
                break;
            }
            else if(ret0 > 0)
            {
                //两个参数都正确,且不是初始化版本
                //检查是否一致，不一致，写备份
                if(memcmp(lp_machineparam,lp_machineparam1,sizeof(str_machine_param_t)))
                {
                	dev_misc_machineparam_write((u8*)lp_machineparam, STR_MACHINE_PARAM_LEN, 1);
                }
                break;
            }
            else
            {
                //主参数是初始化版本,备份参数不是初始化版本,合并参数
                memcpy(lp_machineparam->m_tamperman, lp_machineparam1->m_tamperman, len1-MCU_TAMPERMAN_STA);
                dev_misc_machineparam_write((u8*)lp_machineparam, STR_MACHINE_PARAM_LEN, 0);
                dev_misc_machineparam_write((u8*)lp_machineparam, STR_MACHINE_PARAM_LEN, 1);
                break;
            }
        }        
    }
	
//	MISC_DEBUG("\r\ninit flag = %d\r\n",initflg);
    if(initflg)
    {
        //初始化参数
        memset(lp_machineparam, 0xFF, STR_MACHINE_PARAM_LEN);
        memset(lp_machineparam->m_paramvaild, 0, 8);
        //lp_machineparam->m_paramvaild[0] |= 0x01;   //appinfo设为1
		
        dev_misc_machineparam_write((u8*)lp_machineparam, STR_MACHINE_PARAM_LEN, 0);
        dev_misc_machineparam_write((u8*)lp_machineparam, STR_MACHINE_PARAM_LEN, 1);
        
    }
    else
    {
        ret0 = 0;
    }
    k_free(lp_machineparam);
    k_free(lp_machineparam1);
    return ret0;
}
/****************************************************************************
**Description:	   写机器参数
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20180102
**---------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
s32 dev_misc_machineparam_init_boot(void)
{
    s32 ret0;
    s32 ret1;
    u8 *lp_machineparam;
    u8 *lp_machineparam1;
    u16 len0;
    u16 len1;
    u16 crc0;
    u16 crc1;
    u8 initflg;
   
    lp_machineparam = k_malloc(MACHINE_PARAM_SPACE);
    lp_machineparam1 = k_malloc(MACHINE_PARAM_SPACE);
    
    memset(lp_machineparam1, 0xFF, MACHINE_PARAM_SPACE);
    len1 = MACHINE_PARAM_SPACE;
    ret1 = dev_misc_machineparam_read(lp_machineparam1, &len1, 1);
    if(ret1 >= 0)
    {
        crc1 = (lp_machineparam1[0]<<8)+lp_machineparam1[1];
        //len1 = (lp_machineparam1->m_paramlen[0]<<8)+lp_machineparam1->m_paramlen[1];
    }
    memset(lp_machineparam, 0xFF, MACHINE_PARAM_SPACE);
    len0 = MACHINE_PARAM_SPACE;
    ret0 = dev_misc_machineparam_read((u8*)lp_machineparam, &len0, 0);
    if(ret0 >= 0)
    {
        crc0 = (lp_machineparam[0]<<8)+lp_machineparam[1];
        //len0 = (lp_machineparam->m_paramlen[0]<<8)+lp_machineparam->m_paramlen[1];
    }
    initflg = 0;
    while(1)
    {
        if(ret1 < 0)
        {
            if(ret0<=0)
            {
                //两组参数均错，需要初始化
                //备份参数错误,主参数是初始化版本
                initflg = 1;
                break;
            }
            else // if(ret0==1)
            {
                //备份参数错误，主参数正确，需要备份
                dev_misc_machineparam_write((u8*)lp_machineparam, len0, 1);
                break;
            }
        }
        else 
        {
            if(ret0 < 0)
            {
                //主参数错，备份参数正确
                dev_misc_machineparam_write((u8*)lp_machineparam1, len1, 0);
                break;
            }
            else if(ret0 > 0)
            {
                //两个参数都正确,且不是初始化版本
                break;
            }
            else
            {
                //主参数是初始化版本,备份参数不是初始化版本,合并参数
                memcpy(&lp_machineparam[MCU_TAMPERMAN_STA], &lp_machineparam1[MCU_TAMPERMAN_STA], len1-MCU_TAMPERMAN_STA);
                dev_misc_machineparam_write((u8*)lp_machineparam, len1, 0);
                dev_misc_machineparam_write((u8*)lp_machineparam, len1, 1);
                break;
            }
        }        
    }
    if(initflg)
    {
      #if 0  
        //初始化参数
        memset(lp_machineparam->m_paramvaild, 0, 8);
        lp_machineparam->m_paramvaild[0] |= 0x01;   //appinfo设为1
        dev_misc_machineparam_write((u8*)lp_machineparam, STR_MACHINE_PARAM_LEN, 0);
        dev_misc_machineparam_write((u8*)lp_machineparam, STR_MACHINE_PARAM_LEN, 1);
      #endif
        //BOOT下不初始化,直接报错
        ret0 = -1;
    }
    else
    {
        ret0 = 0;
    }
    k_free(lp_machineparam);
    k_free(lp_machineparam1);
    return ret0;
}
s32 dev_misc_machineparam_init(void)
{
    static u8 flg=0;
    s32 ret=0;
    
    if(flg==0)
    {
      #ifdef TRENDIT_BOOT
        ret = dev_misc_machineparam_init_boot();
      #else
        ret = dev_misc_machineparam_init_cos();
      #endif
        if(ret < 0)
        {
            flg = 1;
        }
    }
    return ret;
}

/****************************************************************************
**Description:	   读指定参数
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170805
**---------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
const strMachineparam_t strMachineparam_tab[MACHINE_PARAM_SN_MAX]=
{
	
    {MACHINE_PARAM_APPINFO    ,MCUAPPINFO_STA      ,MCUAPPINFO_MAX      },
    {ACQUIRERPKDATA,ACQUIRERPKDATA_STA,ACQUIRERPKDATA_MAX},
    {MACHINE_PARAM_TAMPERMAN  ,MCU_TAMPERMAN_STA   ,MCU_TAMPERMAN_MAX   },
    {MACHINE_PARAM_UPDATEFLG  ,UPDATEFLG_STA       ,UPDATEFLG_MAX       },
    {MACHINE_PARAM_RF         ,RFPARAMLEN_STA      ,RFPARAMLEN_MAX      },
    {MACHINE_PARAM_PRINT      ,PRINTPARAMLEN_STA   ,PRINTPARAMLEN_MAX   },
    {MACHINE_PARAM_LCD        ,LCDPARAMLEN_STA     ,LCDPARAMLEN_MAX     },
    {MACHINE_PARAM_BT         ,BTPARAMLEN_STA      ,BTPARAMLEN_MAX      },
//    {MACHINE_PARAM_BPR        ,MCU_BPR_STA         ,MCU_BPR_MAX         },
    {MACHINE_PARAM_LANGUAGE   ,MCULANGUAGE_STA     ,MCULANGUAGE_MAX     },
    {MACHINE_PARAM_RESET      ,MCURESET_STA        ,MCURESET_MAX        },
    {MACHINE_PARAM_HWSNINFO   ,MCU_HWSNINFO_STA    ,MCU_HWSNINFO_MAX    },
    {MACHINE_PARAM_TRENDSN    ,MCU_TRENDSN_STA     ,MCU_TRENDSN_MAX     },
    {MACHINE_PARAM_POSSN      ,MCU_POSSN_STA       ,MCU_POSSN_MAX       },
    {MACHINE_PARAM_TMPRAND    ,MCU_TMPRAND_STA     ,MCU_TMPRAND_MAX     },
    {MACHINE_PARAM_RSAKEY     ,MCU_RSAKEY_STA      ,MCU_RSAKEY_MAX      },
    {MACHINE_PARAM_DEVTESTINFO,MCU_DEVTESTINFO_STA ,MCU_DEVTESTINFO_MAX },
    {MACHINE_PARAM_SECUREAUTHENINFO,MCU_SECUREAUTHENINFO_STA ,MCU_SECUREAUTHENINFO_MAX },
    {MCU_CAK                  ,MCU_CAK_STA         ,MCU_CAK_MAX         },
    {MCU_PREVENTINFO          ,MCU_PREVENTINFO_STA ,MCU_PREVENTINFO_MAX },
    {MACHINE_PARAM_TP         ,MCU_TPINFO_STA      ,MCU_TPINFO_MAX      },
};
static s32 dev_misc_machineparam_search(u8 *src, u16 *srclen, machine_param_sn_t sn, u8 *param, u16 paramlen, u16 offset)
{
    s32 ret;
    s32 i;
    u16 len;
    
    if(sn>=MACHINE_PARAM_SN_MAX)
    {
        MISC_DEBUG("SN err(%d)!\r\n", sn);
        return -1;
    }   
    //dev_misc_machineparam_init();
    len = *srclen;
    ret = dev_misc_machineparam_read((u8*)src, &len, 0);
    if(ret < 0)
    {
        MISC_DEBUG("read err(%d)!\r\n", ret);
        return -1;      
    }
    *srclen = len;
    for(i=0; i<MACHINE_PARAM_SN_MAX; i++)
    {
        if(strMachineparam_tab[i].m_sn == sn)
        {
            break;
        }
    }
    if(i >= MACHINE_PARAM_SN_MAX)
    {
        MISC_DEBUG("SN(%d) not find\r\n", sn);
        return -1;
    }
    if((paramlen+offset) > strMachineparam_tab[i].m_spacemax)
    {
        MISC_DEBUG("Paramlen(%d,%d)is too big!\r\n", paramlen, strMachineparam_tab[i].m_spacemax);
        return -1;
    }
    return i;
}
s32 dev_misc_machineparam_get(machine_param_sn_t sn, u8 *param, u16 paramlen, u16 offset)
{
  #ifdef TRENDIT_BOOT
    u8 lp_machineparam[MACHINE_PARAM_SPACE];
  #else
    u8 *lp_machineparam;//[MACHINE_PARAM_SPACE];
  #endif
    s32 i;
    u8 j,k;
    u16 len;


	//MISC_DEBUG("\r\nget machineparam:%d\r\n",sn);
    
    
  #ifdef TRENDIT_BOOT
    len = MACHINE_PARAM_SPACE ;
  #else
    lp_machineparam = (u8 *)k_malloc(MACHINE_PARAM_SPACE);
    if(lp_machineparam == NULL)
    {
    	return -1;
    }
    len = MACHINE_PARAM_SPACE;
  #endif
    i = dev_misc_machineparam_search(lp_machineparam, &len, sn, param, paramlen, offset);
    if(i < 0)
    {
        #ifdef TRENDIT_CORE
		k_free(lp_machineparam);
		#endif
        return -1;
    }
//MISC_DEBUG("SN=%d, i=%d\r\n", sn, i);
    //
    j = i>>3;
    k = i&0x07;
    if(0 == (lp_machineparam[12+j]&(1<<k)))
    {
        //说明参数无效
        //MISC_DEBUG("Param(%d,%02X) not vaild!\r\n", sn, lp_machineparam[12+j]);
		#ifdef TRENDIT_CORE
		k_free(lp_machineparam);
		#endif
        return -1;
    }
    memcpy(param, ((u8*)lp_machineparam)+strMachineparam_tab[i].m_offset+offset, paramlen);
	#ifdef TRENDIT_CORE
	k_free(lp_machineparam);
	#endif
    return 0;
}
/****************************************************************************
**Description:	   写指定参数
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170614
**---------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
s32 dev_misc_machineparam_set(machine_param_sn_t sn, u8 *param, u16 paramlen, u16 offset)
{
    s32 ret;
    //str_machine_param_t l_machineparam;
//    u8* lp_machineparam;
  #ifdef TRENDIT_BOOT
    u8 lp_machineparam[MACHINE_PARAM_SPACE];
  #else
    u8 *lp_machineparam;//[MACHINE_PARAM_SPACE];
  #endif
    s32 i;
    u8 j,k;
    u16 len;
    

  #ifdef TRENDIT_BOOT
    len = MACHINE_PARAM_SPACE ;
  #else
    lp_machineparam = (u8 *)k_malloc(MACHINE_PARAM_SPACE);
    if(lp_machineparam == NULL)
    {
    	return -1;
    }
    len = MACHINE_PARAM_SPACE;
  #endif
    i = dev_misc_machineparam_search(lp_machineparam, &len, sn, param, paramlen, offset);
    if(i < 0)
    {
		#ifdef TRENDIT_CORE
		k_free(lp_machineparam);
		#endif
        return -1;
    }
//MISC_DEBUG("SN=%d, i=%d\r\n", sn, i);  //sxl?
    //
    j = i>>3;
    k = i&0x07;
    //设置参数标志
    lp_machineparam[12+j] |= (1<<k);
    memcpy(((u8*)lp_machineparam)+strMachineparam_tab[i].m_offset+offset, param, paramlen);
    if(len < STR_MACHINE_PARAM_LEN)
    {
        len = STR_MACHINE_PARAM_LEN;
    }
    ret = dev_misc_machineparam_write(lp_machineparam, len, 0);
    ret = dev_misc_machineparam_write(lp_machineparam, len, 1);
    if(ret < 0)
    {
        MISC_DEBUG("write err(%d)!\r\n", ret);
		#ifdef TRENDIT_CORE
		k_free(lp_machineparam);
		#endif
        return -1;      
    }
	#ifdef TRENDIT_CORE
	k_free(lp_machineparam);
	#endif
    return 0;
}
/****************************************************************************
**Description:	   清除指定参数
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170614
**---------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
s32 dev_misc_machineparam_del(machine_param_sn_t sn, u8 *param, u16 paramlen)
{
    s32 ret;
    //str_machine_param_t l_machineparam;
  #ifdef TRENDIT_BOOT
    u8 lp_machineparam[STR_MACHINE_PARAM_LEN];
  #else
    u8 *lp_machineparam;//[MACHINE_PARAM_SPACE];
  #endif
    s32 i;
    u8 j,k;
    u16 len;
    

  #ifdef TRENDIT_BOOT
    len = STR_MACHINE_PARAM_LEN ;
  #else
    lp_machineparam = (u8 *)k_malloc(MACHINE_PARAM_SPACE);
    if(lp_machineparam == NULL)
    {
    	return -1;
    }
    len = MACHINE_PARAM_SPACE;
  #endif
    i = dev_misc_machineparam_search(lp_machineparam, &len, sn, param, paramlen, 0);
    if(i < 0)
    {
        #ifdef TRENDIT_CORE
		k_free(lp_machineparam);
		#endif
        return -1;
    }
//MISC_DEBUG("SN=%d, i=%d\r\n", sn, i);
    //
    j = i>>3;
    k = i&0x07;
    //设置参数标志
    lp_machineparam[12+j] &= ~(1<<k);
    memcpy(((u8*)lp_machineparam)+strMachineparam_tab[i].m_offset, param, paramlen);

    ret = dev_misc_machineparam_write(lp_machineparam, len, 0);
    ret = dev_misc_machineparam_write(lp_machineparam, len, 1);
    if(ret < 0)
    {
        MISC_DEBUG("write err(%d)!\r\n", ret);
		#ifdef TRENDIT_CORE
		k_free(lp_machineparam);
		#endif
        return -1;      
    }
	#ifdef TRENDIT_CORE
	k_free(lp_machineparam);
	#endif
    return 0;
}
/****************************************************************************
**Description:	   设置更新标志
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170807
**---------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
const u8 updateflg_tab[] = "updateflag";
s32 dev_misc_updateflg_set(u8 flg)
{
    u8 updateflg[10];
    if(flg)
    {
        memcpy(updateflg, updateflg_tab, 10);    
    }
    else
    {
        memset(updateflg, 0xff, 10); 
    }
    
    return dev_misc_machineparam_set(MACHINE_PARAM_UPDATEFLG, updateflg, 10, 0);
}
/****************************************************************************
**Description:	   upd
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170807
**---------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
s32 dev_misc_updateflg_check(void)
{
    u8 updateflg[12];
    s32 ret;
    
    ret = dev_misc_machineparam_get(MACHINE_PARAM_UPDATEFLG, updateflg, 10, 0);
    if(ret < 0)
    {
        MISC_DEBUG("GET ERR\r\n");
        return -1;
    }
    if(0 != memcmp(updateflg, updateflg_tab, 10))
    {
        updateflg[10] = 0;
        MISC_DEBUG("updateflg(%s)\r\n", updateflg);
        return -1;
    }
    return 0;    
}

/**
 * @brief 获取语言类型
 * @param [in/out] 无
 * @retval 返回值 strLanguageparam_t
 * @since 2019-12-16  
 */
s32 dev_misc_language_get(void)
{
    s32 ret;
    u8 tmp[4];
    
    ret = dev_misc_machineparam_get(MACHINE_PARAM_LANGUAGE, tmp, 1, 0);
    if(ret<0)
    {
        ret = LANGUAGE_SIMPLIFIED_CHINESE;    //默认为中文
    }
    else
    {
        if(tmp[0]<LANGUAGE_TYPE_MAX)
        {
            ret = LANGUAGE_SIMPLIFIED_CHINESE;    //默认为中文
        }
        else
        {
            ret = tmp[0];
        }
    }
    return ret;
}
/**
 * @brief 设置语言类型
 * @param [in/out] type:语言类型，strLanguageparam_t
 * @retval 返回值 0:成功  <0:失败
 * @since 2019-12-16  
 */
s32 dev_misc_language_set(u8 type)
{
    s32 ret;
    u8 tmp[4];

    if(type >= LANGUAGE_TYPE_MAX)
    {
        return DDI_EINVAL;     //参数错
    }
    tmp[0] = type;
    ret = dev_misc_machineparam_set(MACHINE_PARAM_LANGUAGE, tmp, 1, 0);
    return ret;
}
/**
 * @brief 获取复位模式
 * @param [in/out] 无
 * @retval 返回值 1: 程序重启复位
 * @since 2019-12-16  
 */
 #define MISC_RSTMOD_SOFTRST    0x5A
 #define MISC_RSTMOD_WTDRST     0xA5
 #define MISC_RSTMOD_HARD_FAULT_RST   0x7E
s32 dev_misc_reset_get(void)
{
  #if 0
    s32 ret;
    u8 tmp[4];
    
    ret = dev_misc_machineparam_get(MACHINE_PARAM_RESET, tmp, 1, 0);
    if(ret<0)
    {
        ret = 0;
    }
    else
    {
        if(MISC_RSTMOD_SOFTRST == tmp[0])
        {
            ret = 1;
        }
        else if(MISC_RSTMOD_WTDRST == tmp[0])
        {
            ret = 2;
        }
        else
        {
            ret = 0;
        }
    }
    return ret;
  #else
    return dev_dryice_rstmod_get();
  #endif
}
/**
 * @brief 设置语言类型
 * @param [in/out] flg: 0:清除复位标志，1:设置复位标志 2:设置Watchdog复位标志
 * @retval 返回值 0:成功  <0:失败
 * @since 2019-12-16  
 */
s32 dev_misc_reset_set(SYSTEM_RESET_TYPE_E st_type)
{
  #if 0
    s32 ret;
    u8 tmp[4];

    ret = dev_misc_reset_get();
    if(ret == flg)
    {
        return DDI_OK;
    }
    if(flg==1)
    {
        tmp[0] = MISC_RSTMOD_SOFTRST;
    }
    else if(flg == 2)
    {
        tmp[0] = MISC_RSTMOD_WTDRST;
    }
    else
    {
        tmp[0] = 0;
    }
    ret = dev_misc_machineparam_set(MACHINE_PARAM_RESET, tmp, 1, 0);
    return ret;
  #else
    return dev_dryice_rstmod_set(st_type);
  #endif
}

/**
 * @brief 获取boot里设置的复位模式
 * @param [in/out] 无
 * @retval 返回值 1: 程序重启复位
 * @since 2019-12-16  
 */
s32 dev_misc_reset_get_from_flash(void)
{
    s32 ret;
    u8 tmp[4];
    
    ret = dev_misc_machineparam_get(MACHINE_PARAM_RESET, tmp, 1, 0);
    if(ret<0)
    {
        ret = 0;
    }
    else
    {
        if(MISC_RSTMOD_SOFTRST == tmp[0])
        {
            ret = 1;
        }
        else if(MISC_RSTMOD_WTDRST == tmp[0])
        {
            ret = 2;
        }
        else if(MISC_RSTMOD_HARD_FAULT_RST == tmp[0])
        {
            ret = 3;
        }
        else
        {
            ret = 0;
        }
    }

    TRACE_DBG("ret:%d", ret);
    return ret;
}
/**
 * @brief 设置复位模式，在boot里调用，供应用层给后台上送开机原因
 * @param [in/out] flg: 0:清除复位标志，1:设置复位标志 2:设置Watchdog复位标志
 * @retval 返回值 0:成功  <0:失败
 * @since 2019-12-16  
 */
s32 dev_misc_reset_set_to_flash(u8 flg)
{
    s32 ret;
    u8 tmp[4];

    if(flg==1)
    {
        tmp[0] = MISC_RSTMOD_SOFTRST;
    }
    else if(flg == 2)
    {
        tmp[0] = MISC_RSTMOD_WTDRST;
    }
    else if(flg == 3)
    {
        tmp[0] = MISC_RSTMOD_HARD_FAULT_RST;
    }
    else
    {
        tmp[0] = 0;
    }
    ret = dev_misc_machineparam_set(MACHINE_PARAM_RESET, tmp, 1, 0);
    return ret;
}

/****************************************************************************
**Description:	   混合设备初始化
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170614
**---------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
void dev_misc_init(void)
{
    dev_misc_machineparam_init();
    drv_misc_cashbox_init();
}

/**
 * @brief 打开钱箱
 * @param [in/out] 无
 * @param [in/out] 无
 * @retval 无
 * @since 修改的历史说明
 */
#define CASHBOX_TIME_DEFAULT     160    //160ms
void dev_misc_cashbox_open(void)
{
    drv_misc_cashbox_ctl(0);
    dev_user_delay_ms(CASHBOX_TIME_DEFAULT);
    drv_misc_cashbox_ctl(1);
}

void dev_misc_poweroff(void)
{
    //关蓝牙
    //关机
    //dev_power_switch_ctl(0);   //sxl?2019
}
/**
    //增加标志，为1:设置重启标志,重启不需要等待按键
**/
void dev_misc_reboot(SYSTEM_RESET_TYPE_E st_type)
{
    if(st_type)
    {
        dev_misc_reset_set(st_type);
    }
    //dev_power_switch_ctl(0);
    dev_smc_systemsoftreset();
}
#define MISC_HWVER_150MV      128
#define MISC_HWVER_250MV      213
#define MISC_HWVER_350MV      298
#define MISC_HWVER_450MV      384
#define MISC_HWVER_550MV      496
#define MISC_HWVER_650MV      554        //这是M3/M5的分界线
#define MISC_HWVER_750MV      640
#define MISC_HWVER_850MV      725      
#define MISC_HWVER_950MV      810
#define MISC_HWVER_1050MV     896
#define MISC_HWVER_1150MV     981
#define MISC_HWVER_1200MV     1024

#ifdef MACHINE_P7
#define MISC_HWVER_300mv      326
#define MISC_HWVER_600mv      653
#define MISC_HWVER_900mv      980
#endif

s32 dev_misc_get_hwver(void)
{
    static s32 hwver=-1;

    #ifdef MACHINE_P7
        s16 result;
        u8 tmp[32];
        if(hwver < 0)
        {
            result = dev_adc_get_arrage_value(DEV_ADC_HWVER, 50);

            //dev_debug_printf_spr("harver:%d\r\n",(result*1880*2)/0xFFF);
            if(result <= MISC_HWVER_300mv)
            {
                hwver = MACHINE_HWVER_M3V1;
            }
            else if(result <= MISC_HWVER_600mv)
            {
                hwver = MACHINE_HWVER_M3V2;
            }
            else if(result <= MISC_HWVER_900mv)
            {
                hwver = MACHINE_HWVER_M3V3;
            }
        }

    #endif

    return hwver;
}
s32 dev_misc_bt_check(void)
{
    static u8 flg=0xff;
    s32 i;

    if(flg == 0xFF)
    {
        dev_gpio_config_mux(GPIO_PIN_PTC1, MUX_CONFIG_GPIO);
        dev_gpio_set_pad(GPIO_PIN_PTC1, PAD_CTL_PULL_NONE);
        dev_gpio_direction_input(GPIO_PIN_PTC1);
        for(i=1000; i>0; i--);
        if(dev_gpio_get_value(GPIO_PIN_PTC1))
        {
            flg = 1;        //YC
        }
        else
        {
            flg = 0;        //BK
        }
		#if 0
        dev_gpio_config_mux(GPIO_PIN_PTC0, MUX_CONFIG_GPIO);
        dev_gpio_config_mux(GPIO_PIN_PTC1, MUX_CONFIG_GPIO);
        dev_gpio_set_pad(GPIO_PIN_PTC0, PAD_CTL_PULL_DOWN);
        dev_gpio_set_pad(GPIO_PIN_PTC1, PAD_CTL_PULL_DOWN);
        dev_gpio_direction_input(GPIO_PIN_PTC0);
        dev_gpio_direction_input(GPIO_PIN_PTC1);
		#endif
    }
    return flg;
}

/**
 * @brief 获取机器当前工作模式，从app代码里拷贝
 * @param[in] 无
 * @retval  0  正常发布模式
 * @retval  1  集成测试模式（一次测试）
 * @retval  2  smt测试
 */
s32 dev_misc_get_workmod(void)
{
    s32 ret;
    TRENDITDEV_TEST_INFO devtestinfo;

    memset(&devtestinfo,0,sizeof(TRENDITDEV_TEST_INFO));
    ret = dev_misc_machineparam_get(MACHINE_PARAM_DEVTESTINFO,(u8 *)&devtestinfo,sizeof(TRENDITDEV_TEST_INFO),0);
    if(0 == memcmp("INTEGRAT", devtestinfo.pcbatestflag, 8))
    {
        return 1;  //集成测试模式
    }
    else if(0 == memcmp("WORKMODE", devtestinfo.pcbatestflag, 8))
    {
        return 0;   //工作模式
    }
    else
    {
        return 2;   //板卡测试模式
    }
}



#endif
