

#include "ddi_dukpt.h"
#include "pci/pciglobal.h"
//extern PCI_FIRMWARE_API *pcicore;


#ifdef DDI_DUKPT_ENABLE


strDevDukpt DukptKeyTmp = 
{
    -1,//务必初始化为-1
};



void pcikeymanage_dukptk_PedPinOrMac(u8 keyflag, u8 *keydata)
{
    u8 buf[16];
    u8 i = 0;
    
        
    if(keyflag == 0)  //dukpt key --->  pin key
    {
        memcpy(buf,"\x00\x00\x00\x00\x00\x00\x00\xff\x00\x00\x00\x00\x00\x00\x00\xff",16);
        for(i = 0;i<16;i++)
        {
            keydata[i] = keydata[i]^buf[i];
        }
    }
    else if(keyflag == 1)   //dukpt key ---> mac key
    {
        memcpy(buf,"\x00\x00\x00\x00\x00\x00\xff\x00\x00\x00\x00\x00\x00\x00\xff\x00",16);
        for(i = 0;i<16;i++)
        {
            keydata[i] = keydata[i]^buf[i];
        }
    }
    else if(keyflag == 2)   //pin key ---> mac key
    {
        memcpy(buf,"\x00\x00\x00\x00\x00\x00\x00\xff\x00\x00\x00\x00\x00\x00\x00\xff",16);
        for(i = 0;i<16;i++)
        {
            keydata[i] = keydata[i]^buf[i];
        }

        memset(buf, 0, 16);
        memcpy(buf,"\x00\x00\x00\x00\x00\x00\xff\x00\x00\x00\x00\x00\x00\x00\xff\x00",16);
        for(i = 0;i<16;i++)
        {
            keydata[i] = keydata[i]^buf[i];
        }

    }
}




s32 ddi_dukpt_open(void)
{
    return DDI_OK;
}



s32 ddi_dukpt_close(void)
{
    return DDI_OK;

}


s32 ddi_dukpt_inject (strDukptInitInfo* lpstrDukptInitInfo)
{
    s32 ret;
	
	ret = pcicore->pcifirmwareapi_dukptk_load(lpstrDukptInitInfo);
	if(ret == PCI_PROCESSCMD_SUCCESS)
	{
		return DDI_OK;
	}
	else
	{
		return DDI_ERR;
	}
	
	
}


s32 dev_dukpt_arithprocess(u32 nKeyGroup, u32 nKeyIndex, u32 nLen, const u8* lpIn, u8* lpOut, u32 nMode,u8 encryptemode)
{
	s32 ret;
    u8 keylen;
    u8 keytmp[32];
    s32 i;
    //u32 des_type;


    if(nLen%8 != 0)
    {
        return DDI_EINVAL;
    }    
    
    ret = pcicore->pcifirmwareapi_dukpt_check_keytmp(nKeyGroup, nKeyIndex,&DukptKeyTmp);
    if(ret != 0)
    {
        dukpt_printf("检测DUKPT失败\r\n");
        return DDI_ERR;
    }
    
    memcpy(keytmp, DukptKeyTmp.m_pinkey, DukptKeyTmp.m_keylen);
    keylen = DukptKeyTmp.m_keylen;
    

    if(nMode == 0)
    {
        dukpt_printf("dukpt pin encrypt\r\n");
        dukpt_printf("-------- pin key---------\r\n");
    }
    else if(nMode == 1)
    {
        dukpt_printf("dukpt mac decrypt\r\n"); 
        pcikeymanage_dukptk_PedPinOrMac(2, keytmp);//pin key ---> mac key

        dukpt_printf("-------- mac key---------\r\n");
    }
    else
    {
        dukpt_printf("dukpt mac decrypt mode err\r\n");
        ret = DDI_EINVAL;
        return ret;
    }

    memcpy(lpOut, (u8*)lpIn, nLen);//拷贝到目的空间
    for(i=0; i < nLen; )
    {
        DES_TDES(keytmp, keylen,lpOut+i,8,encryptemode);
        i += 8;          
    }

    return ret;
}


s32 ddi_dukpt_encrypt(u32 nKeyGroup, u32 nKeyIndex, u32 nLen, const u8* lpIn, u8* lpOut, u32 nMode)
{
    return dev_dukpt_arithprocess(nKeyGroup,nKeyIndex,nLen,lpIn,lpOut,nMode,0x01);
}



s32 ddi_dukpt_decrypt(u32 nKeyGroup, u32 nKeyIndex, u32 nLen, const u8* lpIn, u8* lpOut, u32 nMode)
{
	  return dev_dukpt_arithprocess(nKeyGroup,nKeyIndex,nLen,lpIn,lpOut,nMode,0x00);
}


s32 ddi_dukpt_getksn(u32 nKeyGroup, u32 nKeyIndex, u32 *lpLen, u8* lpOut)
{
    s32 ret;
    
    ret = pcicore->pcifirmwareapi_dukpt_check_keytmp(nKeyGroup, nKeyIndex,&DukptKeyTmp);
    if(ret != 0)
    {
        return DDI_ERR;
    }

    memcpy(lpOut, DukptKeyTmp.m_ksn, DukptKeyTmp.m_ksnlen);
    *lpLen = DukptKeyTmp.m_ksnlen;

    return DDI_OK;
}


const u8 dukpt_ver[]="VDUKPT0001";
s32 ddi_dukpt_ioctl(u32 nCmd, u32 lParam, u32 wParam)
{
	s32 ret;
    
    switch(nCmd)
    {
        case DDI_DUKPT_CTL_VER:// 获取DUKPT设备版本
            if(wParam == NULL)
            {
                ret = DDI_EINVAL;
            }
            else
            {
                strncpy((s8*)wParam, (s8*)dukpt_ver, strlen((s8 *)dukpt_ver));

                ret = DDI_OK;
            }
            break;
            
        case DDI_DUKPT_CTL_KSN_NEXT://   DUKPT密钥递增
            memset(&DukptKeyTmp,0,sizeof(strDevDukpt));  //sxl?20170601
			DukptKeyTmp.flag = -1;
            ret = DDI_OK;
            break;
			
        default:
            ret = DDI_EINVAL;
            break;
			
    }
    
    return ret;
	
}



#else



#if 1

s32 ddi_dukpt_open(void)
{
    return DDI_ENODEV;
}



s32 ddi_dukpt_close(void)
{
    return DDI_ENODEV;

}

s32 ddi_dukpt_inject(strDukptInitInfo* lpstrDukptInitInfo)
{
	return DDI_ENODEV;
}


s32 ddi_dukpt_encrypt(u32 nKeyGroup, u32 nKeyIndex, u32 nLen, const u8* lpIn, u8* lpOut, u32 nMode)
{
	return DDI_ENODEV;
}


s32 ddi_dukpt_decrypt(u32 nKeyGroup, u32 nKeyIndex, u32 nLen, const u8* lpIn, u8* lpOut, u32 nMode)
{
	return DDI_ENODEV;
}


s32 ddi_dukpt_getksn(u32 nKeyGroup, u32 nKeyIndex, u32 *lpLen, u8* lpOut)
{
	return DDI_ENODEV;
}



s32 ddi_dukpt_ioctl(u32 nCmd, u32 lParam, u32 wParam)
{
	return DDI_ENODEV;
}

#endif


#endif



