

#include "pciglobal.h"
#include "ddi/ddi_dukpt.h"
#ifdef DDI_DUKPT_ENABLE
const static unsigned short crctableA[16] =
{
	0x0000,0x1081,0x2102,0x3183,0x4204,0x5285,0x6306,0x7387,
	0x8408,0x9489,0xA50A,0xB58B,0xC60C,0xD68D,0xE70E,0xF78F
};

const static unsigned short crctableB[16]=
{
	0x0000,0x1189,0x2312,0x329B,0x4624,0x57AD,0x6536,0x74BF,
	0x8C48,0x9DC1,0xAF5A,0xBED3,0xCA6C,0xDBE5,0xE97E,0xF8F7
};
/*******************************************************************
Author:   sxl
Function Name: void pcikeymanage_dukptk_calculatecrc(u8 *data,u32 datalen,u8 *crcdata)
Function Purpose:calculate dukpt key crc
Input Paramters: 
                       data     - 
                       datalen        - keydata to calculate check value 
Output Paramters:crcdata - the crc data calculated out
Return Value:    N/A
Remark: N/A
Modify: N/A     20110608  //20140327    
********************************************************************/
void pcikeymanage_dukptk_calculatecrc(u8 *data,u32 datalen,u8 *crcdata)
{
    u8 origdata;
    u16 crc;

    crc = 0x1021;
    for(;datalen>0;datalen--)
    {
        origdata = *data++;
        origdata ^= crc;
        crc = ((crctableA[(origdata & 0xF0) >>4]^crctableB[origdata & 0x0F])^(crc >>8));
    }
    crcdata[0] = (u8)(crc>>8);
    crcdata[1] = (u8)crc;
}


/*******************************************************************
Author:   sxl
Function Name: s32 pcikeymanage_getdukptkeyfile_absolutepath(u8 groupindex,s8 *absolutepath)
Function Purpose: get key data file absolutepath using app name
Input Paramters: 
                         *appname      - app name
Output Paramters:*absolutepath        - absolute path
Return Value:    PCI_PROCESSCMD_SUCCESS  -      success
                       other - failed
Remark: N/A     sxl 0522
Modify: N/A        20110608  //20140327
********************************************************************/
s32 pcikeymanage_getdukptkeyfile_absolutepath(u8 groupindex,s8 *absolutepath)
{
    s32 len;
    s32 i;
    u8 tmp[5];

    if(groupindex >= PCIPOS_APPKEYGROUP_MAXNUM)
    {
        return PCI_KEYGROUPINDEX_ERR;
    }
    memset(tmp,0,sizeof(tmp));
    tmp[0] = (groupindex/10) + 0x30;
    tmp[1] = (groupindex%10) + 0x30;
    len = 2;
    strcpy(absolutepath,DRV_PCI_APPKEY_PATH);
    i = strlen(DRV_PCI_APPKEY_PATH);
    memcpy(&absolutepath[i], tmp, len); 
    i += len;
    absolutepath[i] = 0;
    strcat(absolutepath,DRV_PCI_DUKPTKEY_SUFFIX);

    return PCI_PROCESSCMD_SUCCESS;
}
/*******************************************************************
Author:   sxl
Function Name: s32 pcikeymanage_dukptk_read(u8 *dukptkdata,s32 dukptkdatalen,unsigned int pos)
Function Purpose:save dukpt key
Input Paramters: 
                        databuf    - key data
Output Paramters:N/A
Return Value:    PCI_PROCESSCMD_SUCCESS  - save successed
                       other - save failed
Remark: N/A
Modify: N/A       20110608 //20140327
********************************************************************/
s32 pcikeymanage_dukptk_read(u8 *dukptkdata,s32 dukptkdatalen,unsigned int pos)
{
    s32 ret;
    s32 i;

    i = strlen((s8 *)gDukptkInfo.currappname);
    if(i == 0)   //sxl20110608
    {
        DDEBUG("%s:The Parameter is err! \r\n ",__FUNCTION__);
        return PCI_INPUTPARAMS_ERR;
    }
   

    ret = pcicorecallfunc.pci_kern_read_file(gDukptkInfo.currappname,(u8 *)dukptkdata,dukptkdatalen,pos);
    if(ret != dukptkdatalen)
    {
        DDEBUG("%s:The file read error! \r\n",__FUNCTION__);
        return PCI_DEALPCIFILE_ERR;
    }

    return PCI_PROCESSCMD_SUCCESS;
	
	
}
/*******************************************************************
Author:   sxl
Function Name: s32 pcidrv_dukptk_read(u8 *dukptkdata,s32 dukptkdatalen,unsigned int pos)
Function Purpose:save dukpt key
Input Paramters: 
                        databuf    - key data
Output Paramters:N/A
Return Value:    PCI_PROCESSCMD_SUCCESS  - save successed
                       other - save failed
Remark: N/A
Modify: N/A       20110608  //20140327
********************************************************************/
s32 pcikeymanage_dukptk_write(u8 *dukptkdata,s32 dukptkdatalen,unsigned int pos)
{
    s32 ret;
    s32 i;

    i = strlen((s8 *)gDukptkInfo.currappname);
    if((i == 0)||(i >= DRV_PCI_APP_FILELEN))
    {
        DDEBUG("%s:The Parameter is err! \r\n ",__FUNCTION__);
        return PCI_INPUTPARAMS_ERR;
    }
  

    ret = pcicorecallfunc.pci_kern_insert_file(gDukptkInfo.currappname,dukptkdata,dukptkdatalen,pos);
    if(ret != dukptkdatalen)
    {
        DDEBUG("%s:The file read error! \r\n",__FUNCTION__);
        return PCI_DEALPCIFILE_ERR;
    }

    return PCI_PROCESSCMD_SUCCESS;
    
}
/*******************************************************************
Author:   sxl
Function Name: s32 pcikeymanage_dukptk_readKSN(u8 dukptkindex,u8 *ksndata)
Function Purpose:save KSN data
Input Paramters: 
                        dukptkindex   - dukpt key index
                        ksndata        -  KSN data
Output Paramters:N/A
Return Value:    PCI_PROCESSCMD_SUCCESS  - save successed
                       other - save failed
Remark: N/A
Modify: N/A       20110608  //20140327
********************************************************************/
s32 pcikeymanage_dukptk_readKSN(u8 dukptkindex,u8 *ksndata)
{
    DUKPT_KEY *dukpt_key;
    s32 ret;
    unsigned int pos;
    u8 tmpcrc[2];

    if(dukptkindex >= DUKPTKEY_MAXNUM)
    {
        return PCI_DUKPTK_INVALIDKEYINDEX;
    }

    pos = ((s32)dukptkindex)*DUKPTKSIZE;
    dukpt_key = (DUKPT_KEY *)pcicorecallfunc.mem_malloc(sizeof(DUKPT_KEY));
    if(dukpt_key == NULL)
    {
        return PCI_KMALLOCMEM_ERR;
    }
    ret = pcikeymanage_dukptk_read((u8 *)dukpt_key,DUKPTKSIZE,pos);
    if(ret != PCI_PROCESSCMD_SUCCESS)
    {
        DDEBUG("%s1:The file read err! \r\n",__FUNCTION__);
        pcicorecallfunc.mem_free(dukpt_key);
        return ret;
    }

    if(dukpt_key->keyflag == DUKPTKFLAG_CREATE)
    {
        DDEBUG("%s:dukptk future key does not generate! \r\n",__FUNCTION__);
        pcicorecallfunc.mem_free(dukpt_key);
        return PCI_FUTUREKEY_NOTGENERATE;
    }
    else if(dukpt_key->keyflag > DUKPTKFLAG_INITIAL)
    {
        DDEBUG("%s:dukptk future key counter overflow!\r\n",__FUNCTION__);
        pcicorecallfunc.mem_free(dukpt_key);
        return PCI_FUTUREKEY_COUNTEROVERFLOW;
    }

    pcikeymanage_dukptk_calculatecrc(dukpt_key->KSN,MAXKSNDATALEN,tmpcrc);
    if(Lib_memcmp(tmpcrc,dukpt_key->KSNcrc,2))
    {
        DDEBUG("%s:dukpt key KSN crc error! \r\n",__FUNCTION__);
        pcicorecallfunc.mem_free(dukpt_key);
        return PCI_DUKPTK_INVALIDKSNCRC;
    }
    memcpy(ksndata,dukpt_key->KSN,MAXKSNDATALEN);

    #ifdef SXL_DEBUG
    DDEBUG("\r\npcidrv_dukptk_readKSN :\r\n");
    for(ret = 0;ret < MAXKSNDATALEN;ret++)
    {
        DDEBUG("%02x ",ksndata[ret]);
    }
    DDEBUG("\r\n");
    #endif

    pcicorecallfunc.mem_free(dukpt_key);
    return PCI_PROCESSCMD_SUCCESS;
    
}
/*******************************************************************
Author:   sxl
Function Name: s32 pcikeymanage_dukptk_saveKSN(u8 dukptkindex,u8 *ksndata)
Function Purpose:save KSN data
Input Paramters: 
                        dukptkindex   - dukpt key index
                        ksndata        -  KSN data
Output Paramters:N/A
Return Value:    PCI_PROCESSCMD_SUCCESS  - save successed
                       other - save failed
Remark: N/A
Modify: N/A   20110608  //20140327
********************************************************************/
s32 pcikeymanage_dukptk_saveKSN(u8 dukptkindex,u8 *ksndata)
{
    DUKPT_KEY *dukpt_key;
    s32 ret;
    unsigned int pos;

    if(dukptkindex >= DUKPTKEY_MAXNUM)
    {
        return PCI_DUKPTK_INVALIDKEYINDEX;
    }

    pos = ((s32)dukptkindex)*DUKPTKSIZE;
    dukpt_key = (DUKPT_KEY *)pcicorecallfunc.mem_malloc(sizeof(DUKPT_KEY));
    if(dukpt_key == NULL)
    {
        return PCI_KMALLOCMEM_ERR;
    }
    ret = pcikeymanage_dukptk_read((u8 *)dukpt_key,DUKPTKSIZE,pos);
    if(ret != PCI_PROCESSCMD_SUCCESS)
    {
        DDEBUG("%s1:The file read err! \r\n",__FUNCTION__);
        pcicorecallfunc.mem_free(dukpt_key);
        return ret;
    }

    #ifdef SXL_DEBUG
    DDEBUG("\r\npcidrv_dukptk_saveKSN :\r\n");
    for(ret = 0;ret < MAXKSNDATALEN;ret++)
    {
        DDEBUG("%02x ",ksndata[ret]);
    }
    DDEBUG("\r\n");
    #endif

    pcikeymanage_dukptk_calculatecrc(ksndata,MAXKSNDATALEN,dukpt_key->KSNcrc);
    dukpt_key->keyindex = dukptkindex;
    dukpt_key->keyflag  = DUKPTKFLAG_INITIAL;
    memcpy(dukpt_key->KSN,ksndata,MAXKSNDATALEN);

    ret = pcikeymanage_dukptk_write((u8 *)dukpt_key,DUKPTKHEADLEN,pos);
    if(ret != PCI_PROCESSCMD_SUCCESS)
    {
        DDEBUG("%s2:The file read err! \r\n",__FUNCTION__);
        pcicorecallfunc.mem_free(dukpt_key);
        return ret;
    }

    pcicorecallfunc.mem_free(dukpt_key);
    return PCI_PROCESSCMD_SUCCESS;
    
}
/*******************************************************************
Author:   sxl
Function Name: s32 pcikeymanage_dukptk_savefuturekey(u8 dukptkindex,u8 futurekeyindex,u8 keylen,u8 *keydata)
Function Purpose:save future key
Input Paramters: dukptkindex - dukpt key index
                        futurekeyindex    - future key index
                        key len              - future key len
                        keydata             - future key data
Output Paramters:N/A
Return Value:    PCI_PROCESSCMD_SUCCESS  - save successed
                       other - save failed
Remark: N/A
Modify: N/A      20110608  //20140327
********************************************************************/
s32 pcikeymanage_dukptk_savefuturekey(u8 dukptkindex,u8 futurekeyindex,u8 keylen,u8 *keydata)
{
    DUKPT_FUTUREKEY future_key;
    DUKPT_KEY       *dukpt_key;
    s32             ret;
    unsigned int pos;
    u8 encryptkeydata[24];
    

    if(dukptkindex >= DUKPTKEY_MAXNUM)
    {
        return PCI_DUKPTK_INVALIDKEYINDEX;
    }

    if((futurekeyindex<1)||(futurekeyindex>FUTUREKEY_MAXINDEX))
    {
        return PCI_FUTUREKEY_INVALIDKEYINDEX;
    }
    
    memset(&future_key,0,sizeof(future_key));
    pcikeymanage_dukptk_calculatecrc(keydata,keylen,future_key.keycrc);
    future_key.keylen = keylen;

    //sxl20110608 encrypt dukptk use MMK
    memset(encryptkeydata,0,sizeof(encryptkeydata));
    memcpy(encryptkeydata,keydata,keylen);
    ret = pcikeymanage_keyprocessusingMMK(ENCRYPT,encryptkeydata,keylen);
    if(ret != PCI_PROCESSCMD_SUCCESS)
    {
        DDEBUG("\r\n pcidrv_keyprocess_usingMMK ret = %d \r\n",ret);
        return ret;
    }
    memcpy(future_key.keydata,encryptkeydata,keylen);

    pos = ((u32)dukptkindex)*DUKPTKSIZE;
    dukpt_key = (DUKPT_KEY *)pcicorecallfunc.mem_malloc(sizeof(DUKPT_KEY));
    if(dukpt_key == NULL)
    {
        return PCI_KMALLOCMEM_ERR;
    }
    ret = pcikeymanage_dukptk_read((u8 *)dukpt_key,DUKPTKSIZE,pos);
    if(ret != PCI_PROCESSCMD_SUCCESS)
    {
        DDEBUG("%s:read file error! \r\n",__FUNCTION__);
        pcicorecallfunc.mem_free(dukpt_key);
        return ret;
    }
    
    memcpy(&dukpt_key->futurekey[futurekeyindex-1],&future_key,DUKPTFUTUREKSIZE);
    pos += DUKPTKHEADLEN + ((u32)futurekeyindex - 1)*DUKPTFUTUREKSIZE;
    ret = pcikeymanage_dukptk_write((u8 *)&future_key,DUKPTFUTUREKSIZE,pos);
    if(ret != PCI_PROCESSCMD_SUCCESS)
    {
        DDEBUG("%s1:read file error! \r\n",__FUNCTION__);
        pcicorecallfunc.mem_free(dukpt_key);
        return ret;
    }
    pcicorecallfunc.mem_free(dukpt_key);
    return PCI_PROCESSCMD_SUCCESS;
	
	
}
/*******************************************************************
Author:   sxl
Function Name:s32 pcikeymanage_dukptk_readfuturekey(u8 dukptkindex,u8 futurekeyindex,u8 *futurekeylen,u8 *futurekeydata)
Function Purpose:read future key 
Input Paramters: dukptkindex - dukpt key index
                         futurekeyindex - future key index
Output Paramters: futurekeylen    - future key len
                           futurekeydata  - future key data
Return Value:    PCI_PROCESSCMD_SUCCESS  - save successed
                       other - save failed
Remark: N/A
Modify: N/A       20110608  //20140327
********************************************************************/
s32 pcikeymanage_dukptk_readfuturekey(u8 dukptkindex,u8 futurekeyindex,u8 *futurekeylen,u8 *futurekeydata)
{
    DUKPT_FUTUREKEY future_key;
    DUKPT_KEY       *dukpt_key;
    s32             ret;
    unsigned int pos;
    u8 tempcrc[2];
    u8 encryptkeydata[24];

    if(dukptkindex >= DUKPTKEY_MAXNUM)
    {
        return PCI_DUKPTK_INVALIDKEYINDEX;
    }

    if((futurekeyindex<1)||(futurekeyindex>FUTUREKEY_MAXINDEX))
    {
        return PCI_FUTUREKEY_INVALIDKEYINDEX;
    }

    pos = ((u32)dukptkindex)*DUKPTKSIZE;
    dukpt_key = (DUKPT_KEY *)pcicorecallfunc.mem_malloc(sizeof(DUKPT_KEY));
    if(dukpt_key == NULL)
    {
        return PCI_KMALLOCMEM_ERR;
    }
    ret = pcikeymanage_dukptk_read((u8 *)dukpt_key,DUKPTKSIZE,pos);
    if(ret != PCI_PROCESSCMD_SUCCESS)
    {
        DDEBUG("%s:read file error! \r\n",__FUNCTION__);
        pcicorecallfunc.mem_free(dukpt_key);
        return ret;
    }

    memcpy(&future_key,&dukpt_key->futurekey[futurekeyindex - 1],DUKPTFUTUREKSIZE);
    if(future_key.keylen != FUTUREKEY_LEN)
    {
        pcicorecallfunc.mem_free(dukpt_key);
        return PCI_FUTUREKEY_INVALIDKEYLEN;
    }
    pcicorecallfunc.mem_free(dukpt_key);

    //sxl20110608 encrypt dukptk use MMK
    memset(encryptkeydata,0,sizeof(encryptkeydata));
    memcpy(encryptkeydata,future_key.keydata,future_key.keylen);
    ret = pcikeymanage_keyprocessusingMMK(DECRYPT,encryptkeydata,future_key.keylen);
    if(ret != PCI_PROCESSCMD_SUCCESS)
    {
        DDEBUG("\r\n pcidrv_keyprocess_usingMMK ret = %d \r\n",ret);
        return ret;
    }
    memcpy(future_key.keydata,encryptkeydata,future_key.keylen);

    
    pcikeymanage_dukptk_calculatecrc(future_key.keydata,future_key.keylen,tempcrc);
    if(Lib_memcmp(tempcrc,future_key.keycrc,2))
    {
        Lib_memset((u8 *)&future_key,0,DUKPTFUTUREKSIZE);
        return PCI_FUTUREKEY_INVALIDCRC;
    }
    *futurekeylen = future_key.keylen;
    memcpy(futurekeydata,future_key.keydata,future_key.keylen);
    Lib_memset((u8 *)&future_key,0,DUKPTFUTUREKSIZE);
    return PCI_PROCESSCMD_SUCCESS;
	
}

/*******************************************************************
Author:   sxl
Function Name:s32 pcikeymanage_dukptk_savekeyflag(u8 dukptkindex,u8 dukptkflag)
Function Purpose:save dukpt key flag
Input Paramters: dukptkindex - dukpt key index
                         dukptkflag   - dukpt key index
Output Paramters: N/A
Return Value:    PCI_PROCESSCMD_SUCCESS  - save successed
                       other - save failed
Remark: N/A
Modify: N/A       20110608 //20140327
********************************************************************/
s32 pcikeymanage_dukptk_savekeyflag(u8 dukptkindex,u8 dukptkflag)
{
    DUKPT_KEY *dukpt_key;
    unsigned int pos;
    s32 ret;
    
    if(dukptkindex >= DUKPTKEY_MAXNUM)
    {
        return PCI_DUKPTK_INVALIDKEYINDEX;
    }

    dukpt_key = (DUKPT_KEY *)pcicorecallfunc.mem_malloc(sizeof(DUKPT_KEY));
    if(dukpt_key == NULL)
    {
        return PCI_KMALLOCMEM_ERR;
    }
    pos = ((u32)dukptkindex)*DUKPTKSIZE;
    ret = pcikeymanage_dukptk_read((u8 *)dukpt_key,DUKPTKHEADLEN,pos);
    if(ret != PCI_PROCESSCMD_SUCCESS)
    {
        DDEBUG("%s1:the file read error! \r\n",__FUNCTION__);
        pcicorecallfunc.mem_free(dukpt_key);
        return ret;
    }
    dukpt_key->keyflag = dukptkflag;
    ret = pcikeymanage_dukptk_write((u8 *)dukpt_key,DUKPTKHEADLEN,pos);
    if(ret != PCI_PROCESSCMD_SUCCESS)
    {
        DDEBUG("%s2:the file read error! \r\n",__FUNCTION__);
        pcicorecallfunc.mem_free(dukpt_key);
        return ret;
    }

    pcicorecallfunc.mem_free(dukpt_key);
    return PCI_PROCESSCMD_SUCCESS;
	
	
}
/*******************************************************************
Author:   sxl
Function Name: void pcikeymanage_dukptk_futurekeygeneratealgorithm(void)
Function Purpose:dukpt key's future key generate algorithm
Input Paramters: N/A
Output Paramters:N/A
Return Value:    N/A
Remark: N/A
Modify: N/A       20110608  //20140327
********************************************************************/
void pcikeymanage_dukptk_futurekeygeneratealgorithm(void)
{
    u8 i,buf[24];

    #ifdef SXL_DEBUG
    DDEBUG("\r\npcidrv_dukptk_futurekeygeneratealgorithm:\r\n");
    DDEBUG("\r\ngDukptkInfo KSN:\r\n");
    for(i = 0 ;i < 10;i++)
    {
        DDEBUG("%02x ",gDukptkInfo.KSN[i]);
    }
    DDEBUG("\r\ngDukptkInfo cryptreg1:\r\n");
    for(i = 0 ;i < 8;i++)
    {
        DDEBUG("%02x ",gDukptkInfo.cryptreg1[i]);
    }
    DDEBUG("\r\ngDukptkInfo keyreg:\r\n");
    for(i = 0 ;i < 16;i++)
    {
        DDEBUG("%02x ",gDukptkInfo.keyreg[i]);
    }
    
    #endif

    for(i = 0;i < 8;i++)
    {
        gDukptkInfo.cryptreg2[i] = gDukptkInfo.cryptreg1[i]^gDukptkInfo.keyreg[i+8];
    }
    DES_TDES(gDukptkInfo.keyreg,8,gDukptkInfo.cryptreg2,8,1);
    for(i = 0;i<8;i++)
    {
        gDukptkInfo.cryptreg2[i] = gDukptkInfo.cryptreg2[i]^gDukptkInfo.keyreg[i+8];
    }
    memcpy(buf,(u8 *)"\xC0\xC0\xC0\xC0\x00\x00\x00\x00\xC0\xC0\xC0\xC0\x00\x00\x00\x00",16);
    for(i = 0;i<16;i++)
    {
        gDukptkInfo.keyreg[i] = buf[i]^gDukptkInfo.keyreg[i];
    }
    for(i = 0;i<8;i++)
    {
        gDukptkInfo.cryptreg1[i] = gDukptkInfo.cryptreg1[i]^gDukptkInfo.keyreg[i+8];
    }
    DES_TDES(gDukptkInfo.keyreg,8,gDukptkInfo.cryptreg1,8,1);
    for(i = 0;i < 8;i++)
    {
        gDukptkInfo.cryptreg1[i] = gDukptkInfo.cryptreg1[i]^gDukptkInfo.keyreg[i+8];
    }


    #ifdef SXL_DEBUG
    DDEBUG("\r\npcidrv_dukptk_futurekeygeneratealgorithm111:\r\n");
    DDEBUG("\r\ngDukptkInfo cryptreg1:\r\n");
    for(i = 0 ;i < 8;i++)
    {
        DDEBUG("%02x ",gDukptkInfo.cryptreg1[i]);
    }
    DDEBUG("\r\ngDukptkInfo cryptreg2:\r\n");
    for(i = 0 ;i < 8;i++)
    {
        DDEBUG("%02x ",gDukptkInfo.cryptreg2[i]);
    }
    
    #endif
}
/*******************************************************************
Author:   sxl
Function Name: s32 pcikeymanage_dukptk_generatenewfuturekey1(void)
Function Purpose:generate new future key 
Input Paramters: N/A
Output Paramters:N/A
Return Value:    NEEDCALL_NEWKEY3 - 
                       NEEDCALL_NEWKEY4 - 
Remark: N/A
Modify: N/A      20110608  //20140327
********************************************************************/
s32 pcikeymanage_dukptk_generatenewfuturekey1(void)
{
    gDukptkInfo.shiftreg = gDukptkInfo.shiftreg >> 1;
    if(gDukptkInfo.shiftreg == 0)
    {
        return NEEDCALL_NEWKEY4;
    }
    return NEEDCALL_NEWKEY3;
}
/*******************************************************************
Author:   sxl
Function Name: s32 pcikeymanage_dukptk_generatenewfuturekey2(void)
Function Purpose:generate new future key 
Input Paramters: N/A
Output Paramters:N/A
Return Value:    PCI_PROCESSCMD_SUCCESS  - save successed
                       other - save failed
Remark: N/A
Modify: N/A       //sxl20110608 check if dukptk index overflow  //20140327
********************************************************************/
s32 pcikeymanage_dukptk_generatenewfuturekey2(void)
{
    if((gDukptkInfo.encryptcnt&0x001fffff) == 0)
    {
        pcikeymanage_dukptk_savekeyflag(gDukptkInfo.currentkeyindex,DUKPTKFLAG_OVERFLOW);
        return PCI_FUTUREKEY_COUNTEROVERFLOW;
    }
    return PCI_PROCESSCMD_SUCCESS;
}

/*******************************************************************
Author:   sxl
Function Name: s32 pcikeymanage_dukptk_generatenewfuturekey3(void)
Function Purpose:generate new future key 
Input Paramters: N/A
Output Paramters:N/A
Return Value:    PCI_PROCESSCMD_SUCCESS  - save successed
                       other - save failed
Remark: N/A
Modify: N/A       20110608  //20140327
********************************************************************/
s32 pcikeymanage_dukptk_generatenewfuturekey3(void)
{
    u8 shiftbuf[8];
    u8 i,j;
    s32 ret;
    u8 futurekeylen,futurekey[24];
    u32 l;

    memset(shiftbuf,0,sizeof(shiftbuf));

    #ifdef SXL_DEBUG
    DDEBUG("\r\ngDukptkInfo.shiftreg= %08x\r\n",gDukptkInfo.shiftreg);
    #endif
    ulong_to_bit21(gDukptkInfo.shiftreg,&shiftbuf[5]);

    #ifdef SXL_DEBUG
    DDEBUG("\r\nshiftbuf:\r\n");
    for(i = 0; i < 8;i++)
    {
        DDEBUG("%02x ",shiftbuf[i]);
    }
    DDEBUG("\r\n");
    #endif
    for(i = 0;i<8;i++)
    {
        gDukptkInfo.cryptreg1[i] = shiftbuf[i]|gDukptkInfo.KSN[2+i];
    }

    ret = pcikeymanage_dukptk_readfuturekey(gDukptkInfo.currentkeyindex,gDukptkInfo.curfuturekeyindex,&futurekeylen,gDukptkInfo.keyreg);
    if(ret != PCI_PROCESSCMD_SUCCESS)
    {
        DDEBUG("%s:dukptk generate new future key error!",__FUNCTION__);
        return ret;
    }

    pcikeymanage_dukptk_futurekeygeneratealgorithm();
    memcpy(futurekey,gDukptkInfo.cryptreg1,8);
    memcpy(&futurekey[8],gDukptkInfo.cryptreg2,8);
    l = gDukptkInfo.shiftreg;
    j = 0;
    for(i=0;i<21;i++)
    {
        if(l&0x00100000)
        {
            j = i+1;
            break;
        }
        l = l<<1;
    }
    ret = pcikeymanage_dukptk_savefuturekey(gDukptkInfo.currentkeyindex,j,FUTUREKEY_LEN,futurekey);
    if(ret != PCI_PROCESSCMD_SUCCESS)
    {
        DDEBUG("%s:write future key error! \r\n",__FUNCTION__);
        return ret;
    }
    return PCI_PROCESSCMD_SUCCESS;
    
}
/*******************************************************************
Author:   sxl
Function Name: s32 pcikeymanage_dukptk_addksnchangecnt(ulong counter)
Function Purpose:add DUKPT key KSN change counter
Input Paramters: counter - counter to add
Output Paramters:N/A
Return Value:    PCI_PROCESSCMD_SUCCESS  - save successed
                       other - save failed
Remark: N/A
Modify: N/A   20110608  //20140327
********************************************************************/
s32 pcikeymanage_dukptk_addksnchangecnt(u32 counter)
{
    u8 buf[3];
    s32 ret;

    gDukptkInfo.encryptcnt = gDukptkInfo.encryptcnt + counter;
    ulong_to_bit21(gDukptkInfo.encryptcnt,buf);
    gDukptkInfo.KSN[7] = (gDukptkInfo.KSN[7]&0xE0)|buf[0];
    gDukptkInfo.KSN[8] = buf[1];
    gDukptkInfo.KSN[9] = buf[2];
    ret = pcikeymanage_dukptk_saveKSN(gDukptkInfo.currentkeyindex,gDukptkInfo.KSN);
    return ret;
}
/*******************************************************************
Author:   sxl
Function Name: s32 pcikeymanage_dukptk_checkifgennewfuturekey(void)
Function Purpose:check if generate new future key
Input Paramters:  N/A
Output Paramters:N/A
Return Value:      NEEDCALL_NEWKEY1 - 
                         NEEDCALL_NEWKEY2 -
Remark: N/A
Modify:  N/A        20110608  //20140327
********************************************************************/
s32 pcikeymanage_dukptk_checkifgennewfuturekey(void)
{
    u32 l;
    u8 i,j;
    u8 tmpkey[24];

    l = gDukptkInfo.encryptcnt;
    j = 0;
    for(i = 0;i < 21;i++)
    {
        if(l&0x01)
        {
            j++;
        }

        l = l>>1;
    }
    if(j < 10)
    {
        return NEEDCALL_NEWKEY1;
    }

    memset(tmpkey,0,sizeof(tmpkey));
    pcikeymanage_dukptk_savefuturekey(gDukptkInfo.currentkeyindex,gDukptkInfo.curfuturekeyindex,0,tmpkey);
    pcikeymanage_dukptk_addksnchangecnt(gDukptkInfo.shiftreg);

    return NEEDCALL_NEWKEY2;
}
/*******************************************************************
Author:   sxl
Function Name: s32 pcikeymanage_dukptk_generatenewfuturekey4(u8 mode)
Function Purpose:generate new future key 
Input Paramters: mode - 1(delete current future key)
                         
Output Paramters:N/A
Return Value:    PCI_PROCESSCMD_SUCCESS  - save successed
                       other - save failed
Remark: N/A
Modify: N/A      20110608  //20140327
********************************************************************/
s32 pcikeymanage_dukptk_generatenewfuturekey4(u8 mode)
{
    s32 ret;
    u8 tempkeydata[24];

    if(mode)
    {
        //clear current future key
        memset(tempkeydata,0,sizeof(tempkeydata));
        ret = pcikeymanage_dukptk_savefuturekey(gDukptkInfo.currentkeyindex,gDukptkInfo.curfuturekeyindex,0,tempkeydata);
        if(ret != PCI_PROCESSCMD_SUCCESS)
        {
            return ret;
        }
    }
    ret = pcikeymanage_dukptk_addksnchangecnt(1);
    return ret;
}
/*******************************************************************
Author:   sxl
Function Name: s32 pcikeymanage_initial_dukptkfile(void)
Function Purpose:initial current app's dukpt key file
Input Paramters: N/A
Output Paramters:N/A
Return Value:    PCI_PROCESSCMD_SUCCESS  - save successed
                       other - save failed
Remark: N/A
Modify: N/A       20110608  //20140327
********************************************************************/
s32 pcikeymanage_initial_dukptkfile(void)
{
    unsigned int pos;
    DUKPT_KEY *dukpt_key;
    s32 filelen;
    s32 i,ret;
	
    dukpt_key = (DUKPT_KEY *)pcicorecallfunc.mem_malloc(sizeof(DUKPT_KEY));
    if(dukpt_key == NULL)
    {
        return PCI_KMALLOCMEM_ERR;
    }
    memset((u8 *)dukpt_key,0xff,sizeof(DUKPT_KEY));
    filelen = strlen(gDukptkInfo.currappname);
    //if((filelen == 0) ||(filelen >= DRV_PCI_APP_FILELEN))
    if(filelen == 0)  //sxl20110608
    {
        DDEBUG("%s:The parameter is err! \r\n",__FUNCTION__);
        pcicorecallfunc.mem_free(dukpt_key);
        return PCI_INPUTPARAMS_ERR;
    }

    if(pcicorecallfunc.pci_kern_access_file((s8 *)gDukptkInfo.currappname) < DUKPTKSIZE)
    {
        pos = 0;

        for(i = 0;i < DUKPTKEY_MAXNUM;i++)
        {
            ret = pcicorecallfunc.pci_kern_insert_file(gDukptkInfo.currappname,(u8 *)dukpt_key,DUKPTKSIZE,pos);
            if(ret != DUKPTKSIZE)
            {
                DDEBUG("%s:The file write err! \r\n",__FUNCTION__);
                //set_fs(old_fs);
                pcicorecallfunc.mem_free(dukpt_key);
                return PCI_DEALPCIFILE_ERR;
            }
            pos += DUKPTKSIZE;
        }
    }
    pcicorecallfunc.mem_free(dukpt_key);
    return PCI_PROCESSCMD_SUCCESS;
	
}
/*******************************************************************
Author:   sxl
Function Name: s32 pcikeymanage_dukptk_futurekeyinitial(u8 dukptkindex,u8 *ksndata)
Function Purpose:dukpt key's future key initial
Input Paramters: dukptkindex - dukpt key index
                        ksndata       - KSN data
Output Paramters:N/A
Return Value:    PCI_PROCESSCMD_SUCCESS  - save successed
                       other - save failed
Remark: N/A
Modify: N/A       20110608  //20140327
********************************************************************/
s32 pcikeymanage_dukptk_futurekeyinitial(u8 dukptkindex,u8 *ksndata)
{
    s32 ret;

    memcpy(gDukptkInfo.KSN,ksndata,MAXKSNDATALEN);
    gDukptkInfo.currentkeyindex = dukptkindex;
    gDukptkInfo.curfuturekeyindex = FUTUREKEY_MAXINDEX;
    gDukptkInfo.encryptcnt        = 0;
    gDukptkInfo.shiftreg          = ((u32)1<<20);
    
    while(1)
    {
        ret = pcikeymanage_dukptk_generatenewfuturekey3();  //generate all 21 future key  //sxl?这里老是过不去
        if(ret != PCI_PROCESSCMD_SUCCESS)
        {
            DDEBUG("%s:newkey3 error \r\n",__FUNCTION__);
            return ret;
        }
        ret = pcikeymanage_dukptk_generatenewfuturekey1();
        if(ret == NEEDCALL_NEWKEY3)
        {
            continue;
        }
        ret = pcikeymanage_dukptk_generatenewfuturekey4(0);
        if(ret != PCI_PROCESSCMD_SUCCESS)
        {
            DDEBUG("%s:newfuturekey4 err \r\n",__FUNCTION__);
            return ret;
        }
        ret = pcikeymanage_dukptk_generatenewfuturekey2();
        if(ret != PCI_PROCESSCMD_SUCCESS)
        {
            DDEBUG("%s:newfuturekey2 err \r\n",__FUNCTION__);
            return ret;
        }
        break;
    }
    return PCI_PROCESSCMD_SUCCESS;
	
	
}
/*******************************************************************
Author:   sxl
Function Name: s32 pcikeymanage_pos_savedukptkey(u8 groupindex,u8 keyindex,u8 keylen,u8 ksnlen,u8 *keyksndata)
Function Purpose:save dukpt key
Input Paramters: groupindex - application key group index
                        keyindex    - keyindex
                        keylen       - key len
                        ksnlen       - KSN len
                        keyksndata - KSN data
Output Paramters:N/A
Return Value:    PCI_PROCESSCMD_SUCCESS  - save successed
                       other - save failed
Remark: N/A
Modify: N/A    20110608  //20140327
********************************************************************/
s32 pcikeymanage_pos_savedukptkey(u8 groupindex,u8 keyindex,u8 keylen,u8 ksnlen,u8 *keyksndata)
{
    s8 keyfilepath[128];
    s32 ret;
    unsigned int pos;
    DUKPT_KEY *dukpt_key;
    s32 j,k;
    u8 tempksn[MAXKSNDATALEN];
    u8 tempbuf[24];

    gDukptkInfo.currappname[0] = 0; 
    memset(keyfilepath,0,sizeof(keyfilepath));
    ret = pcikeymanage_getdukptkeyfile_absolutepath(groupindex,keyfilepath);
    if(ret != PCI_PROCESSCMD_SUCCESS)
    {
        return ret;
    }

    //dukptk initial value is 0xff
    if(pcicorecallfunc.pci_kern_access_file(keyfilepath) < DUKPTKFILESIZE)
    {
        ret = pcicorecallfunc.pci_kern_create_file(keyfilepath,DUKPTKFILESIZE,0xff);
        if(ret < 0)
        {
            return PCI_WRITEKEY_ERR;
        }
    }

    #if 1   //removed by weijiakai 2014-07-17 for test   //sxl?
    ret = pcikeymanageauthen_checkmaingroupfile(groupindex);  //sxl modify
    DDEBUG("\r\npcidrv_checkmaingroupfileret = %d\r\n",ret);
    if(ret != PCI_PROCESSCMD_SUCCESS)
    {
        return ret;
    }
    #endif

    
    memset(gDukptkInfo.currappname,0,sizeof(gDukptkInfo.currappname));//record current app name
    strcpy(gDukptkInfo.currappname,keyfilepath);


    if(keyindex >= DUKPTKEY_MAXNUM)
    {
        DDEBUG("%s1:keyindex %d is err \r\n",__FUNCTION__,keyindex);
        return PCI_DUKPTK_INVALIDKEYINDEX;
    }
    if(keylen != FUTUREKEY_LEN)
    {
        DDEBUG("%s2:keylen %d is error \r\n",__FUNCTION__,keylen);
        return PCI_DUKPTK_INVALIDBDK;
    }
    if(ksnlen > MAXKSNDATALEN)
    {
        DDEBUG("%s3:ksnlen %d is error \r\n",__FUNCTION__,ksnlen);
        return PCI_DUKPTK_INVALIDKSN;
    }

    dukpt_key = (DUKPT_KEY *)pcicorecallfunc.mem_malloc(sizeof(DUKPT_KEY));
    if(dukpt_key == NULL)
    {
        return PCI_KMALLOCMEM_ERR;
    }
    pos = ((s32)keyindex)*DUKPTKSIZE;

    ret = pcikeymanage_dukptk_read((u8 *)dukpt_key,DUKPTKSIZE,pos);
    if(ret != PCI_PROCESSCMD_SUCCESS)
    {
        pcicorecallfunc.mem_free(dukpt_key);
        return ret;
    }

    dukpt_key->keyindex = keyindex;
    dukpt_key->keyflag = DUKPTKFLAG_CREATE;

    ret = pcikeymanage_dukptk_write((u8 *)dukpt_key,DUKPTKHEADLEN,pos);
    if(ret != PCI_PROCESSCMD_SUCCESS)
    {
        DDEBUG("%s4:The dukpt key write failed \r\n",__FUNCTION__);
        pcicorecallfunc.mem_free(dukpt_key);
        return ret;
    }

    j = ksnlen%MAXKSNDATALEN;
    if(j)
    {
        k = MAXKSNDATALEN - j;
    }
    else
    {
        k = 0;
    }
    if(k)
    {
        memset(tempksn,0xff,k);

    }
    memcpy(tempksn+k,keyksndata+keylen,ksnlen);
    tempksn[9] =  0x00;
    tempksn[8] =  0x00;
    tempksn[7] &= 0xe0;

    ret = pcikeymanage_dukptk_saveKSN(keyindex,tempksn);
    if(ret != PCI_PROCESSCMD_SUCCESS)
    {
        DDEBUG("%s5:The dukpt key write failed \r\n",__FUNCTION__);
        pcicorecallfunc.mem_free(dukpt_key);
        return ret;
    }
    memcpy(tempbuf,keyksndata,FUTUREKEY_LEN); 
    
    ret = pcikeymanage_dukptk_savefuturekey(keyindex,FUTUREKEY_MAXINDEX,FUTUREKEY_LEN,tempbuf);
    if(ret != PCI_PROCESSCMD_SUCCESS)
    {
        DDEBUG("%s6:The dukpt key write failed \r\n",__FUNCTION__);
        pcicorecallfunc.mem_free(dukpt_key);
        return ret;
    }

    ret = pcikeymanage_dukptk_futurekeyinitial(keyindex,tempksn);
    pcicorecallfunc.mem_free(dukpt_key);
    return ret;
	
	
}

/*******************************************************************
Author:   sxl
Function Name: s32 pcikeymanage_set_currappdukptk(u8 groupindex)
Function Purpose:set current app's dukpt key
Input Paramters: groupindex - the application key group index
                         appname   - application name
Output Paramters:N/A
Return Value:    PCI_PROCESSCMD_SUCCESS  - save successed
                       other - save failed
Remark: N/A
Modify: N/A      20110608  //20140327
********************************************************************/
s32 pcikeymanage_set_currappdukptk(u8 groupindex)
{
    s8 keyfilepath[128];
    //u8 tmpbuf[PUBLICKEY_ISSUER_SIZE+PUBLICKEY_SN_SIZE+1];
    s32 ret;
    
    gDukptkInfo.currappname[0] = 0; 

    /* //sxl modify
    ret = pcidrv_rsa_readregisterfile(appname,tmpbuf,&tmpbuf[PUBLICKEY_ISSUER_SIZE]);
    if(ret != PCI_PROCESSCMD_SUCCESS)
    {
        return ret;
    }
    */

    memset(keyfilepath,0,sizeof(keyfilepath));
    ret = pcikeymanage_getdukptkeyfile_absolutepath(groupindex,keyfilepath);
    if(ret != PCI_PROCESSCMD_SUCCESS)
    {
        return ret;
    }

    memset(gDukptkInfo.currappname,0,sizeof(gDukptkInfo.currappname));
    strcpy(gDukptkInfo.currappname,keyfilepath);

    return pcikeymanage_initial_dukptkfile();
	
}

/*******************************************************************
Author:   sxl
Function Name: void pcikeymanage_dukptk_setshiftreg(void)
Function Purpose:set global varible data
Input Paramters: 
Output Paramters:
Return Value:    PCI_PROCESSCMD_SUCCESS  - save successed
                       other - save failed
Remark: N/A
Modify: N/A     20110608  //20140327
********************************************************************/
void pcikeymanage_dukptk_setshiftreg(void)
{
    u8 i,j;
    u32 l;

    l = gDukptkInfo.encryptcnt;
    j = 0;

    for(i = 0;i < 21;i++)
    {
        if(l&0x1)
        {
            j = i;
            break;
        }
        l = l>>1;
    }

    gDukptkInfo.shiftreg = 1<<j;
    gDukptkInfo.curfuturekeyindex = 21 - j;
    
}
/*******************************************************************
Author:   sxl
Function Name: s32 pcikeymanage_dukptk_getandscatter(u8 pinormac,u8 *keydata,u8 *keylen,u8 *ksndata)
Function Purpose:get current transaction dukpt key information and scatter new future key
Input Paramters: 
                        pinormac  -  0:pin dukpt key
                                          1:mac dukpt key
Output Paramters:*keydata - DUKPT key data
                          *keylen  -  DUKPT key len
                          *ksndata - DUKPT KSN data
Return Value:    PCI_PROCESSCMD_SUCCESS  - save successed
                       other - save failed
Remark: N/A
Modify: N/A       20110608  //20140327
********************************************************************/
s32 pcikeymanage_dukptk_getandscatter(u8 pinormac,u8 *keydata,u8 *keylen,u8 *ksndata)
{
    s32 ret;
    u8 tmpkeylen;
    u8 buf[24];
    u32 i;
    
    pcikeymanage_dukptk_setshiftreg();
    ret = pcikeymanage_dukptk_readfuturekey(gDukptkInfo.currentkeyindex,gDukptkInfo.curfuturekeyindex,&tmpkeylen,gDukptkInfo.keyreg);
    if(ret == PCI_PROCESSCMD_SUCCESS)
    {
        if(pinormac == 0)
        {
            memcpy(buf,"\x00\x00\x00\x00\x00\x00\x00\xff\x00\x00\x00\x00\x00\x00\x00\xff",16);
        }
        else
        {
            memcpy(buf,"\x00\x00\x00\x00\x00\x00\xff\x00\x00\x00\x00\x00\x00\x00\xff\x00",16);
        }

        for(i = 0;i<16;i++)
        {
            gDukptkInfo.keyreg[i] = gDukptkInfo.keyreg[i]^buf[i];
        }
        memcpy(ksndata,gDukptkInfo.KSN,MAXKSNDATALEN);
        memcpy(keydata,gDukptkInfo.keyreg,FUTUREKEY_LEN);

        #ifdef SXL_DEBUG
        DDEBUG("\r\npcidrv_dukptk_getandscatter:\r\n");
        for(i = 0;i < MAXKSNDATALEN;i++)
        {
            DDEBUG("%02x ",ksndata[i]);
        }
        DDEBUG("\r\n");
        for(i = 0;i < FUTUREKEY_LEN;i++)
        {
            DDEBUG("%02x ",keydata[i]);
        }
        DDEBUG("\r\n");
        #endif
        
        *keylen = tmpkeylen;

        //check if generate new future key
        ret = pcikeymanage_dukptk_checkifgennewfuturekey();
        if(ret == NEEDCALL_NEWKEY1)
        {
            ret = pcikeymanage_dukptk_generatenewfuturekey1();
            if(ret == NEEDCALL_NEWKEY3)
            {
                while(1)
                {
                    ret = pcikeymanage_dukptk_generatenewfuturekey3();
                    if(ret != PCI_PROCESSCMD_SUCCESS)
                    {
                        return ret;
                    }
                    ret = pcikeymanage_dukptk_generatenewfuturekey1();
                    if(ret == NEEDCALL_NEWKEY3)
                    {
                        continue;
                    }
                    break;
                }
            }
            ret = pcikeymanage_dukptk_generatenewfuturekey4(1);
            if(ret != PCI_PROCESSCMD_SUCCESS)
            {
                return ret;
            }
        }
        ret = pcikeymanage_dukptk_generatenewfuturekey2();
    }

    return ret;
}
/*******************************************************************
Author:   sxl
Function Name: void pcikeymanage_dukptk_clrglobalvariable(void)
Function Purpose:clear dukpt key global variable
Input Paramters:  N/A
Output Paramters:N/A
Return Value:       N/A
Remark: N/A
Modify: N/A         20110608  //20140327
********************************************************************/
void pcikeymanage_dukptk_clrglobalvariable(void)
{
    memset(gDukptkInfo.cryptreg1,0,sizeof(gDukptkInfo.cryptreg1));
    memset(gDukptkInfo.cryptreg2,0,sizeof(gDukptkInfo.cryptreg2));
    memset(gDukptkInfo.cryptreg3,0,sizeof(gDukptkInfo.cryptreg3));
    memset(gDukptkInfo.keyreg,0,sizeof(gDukptkInfo.keyreg));
    memset(gDukptkInfo.KSN,0,sizeof(gDukptkInfo.KSN));
}
/*******************************************************************
Author:   sxl
Function Name: s32 pcikeymanage_getcurrtrans_dukptkinfo(u8 pinormac,u8 keyindex,u8 *keydata,u8 *keylen,u8 *ksndata)
Function Purpose:get current transaction dukpt key information
Input Paramters: 
                        pinormac  -  0:pin dukpt key
                                          1:mac dukpt key
                        keyindex  -  DUKPT key index
Output Paramters:*keydata - DUKPT key data
                          *keylen  -  DUKPT key len
                          *ksndata - DUKPT KSN data
Return Value:    PCI_PROCESSCMD_SUCCESS  - save successed
                       other - save failed
Remark: N/A
Modify: N/A     20110608  //20140327
********************************************************************/
s32 pcikeymanage_getcurrtrans_dukptkinfo(u8 pinormac,u8 keyindex,u8 *keydata,u8 *keylen,u8 *ksndata)
{
    s32 retcode;
    u8 tempKSN[MAXKSNDATALEN];

    if(keyindex >= DUKPTKEY_MAXNUM)
    {
        return PCI_KEYINDEX_ERR;
    }
    gDukptkInfo.currentkeyindex = keyindex;
    retcode = pcikeymanage_dukptk_readKSN(gDukptkInfo.currentkeyindex,tempKSN);
    if(retcode != PCI_PROCESSCMD_SUCCESS)
    {
        DDEBUG("%s: read dukptk KSN error!\r\n",__FUNCTION__);
        return retcode;
    }
    memcpy(gDukptkInfo.KSN,tempKSN,MAXKSNDATALEN);
    tempKSN[7] &= 0x1F;
    gDukptkInfo.encryptcnt = bit21_to_ulong(&tempKSN[7]);
    
    #ifdef SXL_DEBUG
    DDEBUG("\r\npcidrv_getcurrtrans_dukptkinfo :\r\n");
    for(retcode = 0;retcode < MAXKSNDATALEN;retcode++)
    {
        DDEBUG("%02x ",gDukptkInfo.KSN[retcode]);
    }
    DDEBUG("\r\n");
    #endif

    retcode = pcikeymanage_dukptk_getandscatter(pinormac,keydata,keylen,ksndata);

    //key len error and crc err  //sxl20110608
    if(retcode == PCI_FUTUREKEY_INVALIDKEYLEN ||retcode == PCI_FUTUREKEY_INVALIDCRC)
    {
        pcicorecallfunc.pci_security_locked_hold(0);
    }
    pcikeymanage_dukptk_clrglobalvariable();
    return retcode;
    
}

/*******************************************************************
Author:   sxl
Function Name: s32 pcikeymanage_checkread_dukptkey(void)
Function Purpose: check dukpt key
Input Paramters: N/A
Output Paramters:N/A
Return Value:    PCI_PROCESSCMD_SUCCESS  - check successed
                       other - save failed
Remark: N/A
Modify: N/A     20110608  //20140327
********************************************************************/
s32 pcikeymanage_checkread_dukptkey(void)
{
    s32 ret;
    //u8 tmpkeylen;
    u32 groupindex,dukptkindex,futurekeyindex;
    //u8 i;
    DUKPT_FUTUREKEY future_key,tmpfuture_key;
    DUKPT_KEY       *dukpt_key;
    unsigned int pos;
    u8 tempcrc[2];
    u8 encryptkeydata[24];

    memset(&tmpfuture_key,0xff,sizeof(DUKPT_FUTUREKEY));
    for(groupindex = 0;groupindex < PCIPOS_APPKEYGROUP_MAXNUM;groupindex++)
    {
        pcikeymanage_set_currappdukptk(groupindex);

        for(dukptkindex = 0;dukptkindex<DUKPTKEY_MAXNUM;dukptkindex++)
        {
            pos = ((u32)dukptkindex)*DUKPTKSIZE;
            dukpt_key = (DUKPT_KEY *)pcicorecallfunc.mem_malloc(sizeof(DUKPT_KEY));
            if(dukpt_key == NULL)
            {
                return PCI_KMALLOCMEM_ERR;
            }
            ret = pcikeymanage_dukptk_read((u8 *)dukpt_key,DUKPTKSIZE,pos);
            if(ret != PCI_PROCESSCMD_SUCCESS)
            {
                DDEBUG("%s:read file error! \r\n",__FUNCTION__);
                pcicorecallfunc.mem_free(dukpt_key);
                return ret;
            }
            
            for(futurekeyindex = 1;futurekeyindex <= FUTUREKEY_MAXINDEX;futurekeyindex++)
            {
                memcpy(&future_key,&dukpt_key->futurekey[futurekeyindex - 1],DUKPTFUTUREKSIZE);
                if(Lib_memcmp((u8 *)&tmpfuture_key,(u8 *)&future_key,sizeof(DUKPT_FUTUREKEY)) == 0)
                {
                    continue;
                }

                if(future_key.keylen == 0)
                {
                    continue;
                }
                
                if(future_key.keylen != FUTUREKEY_LEN)
                {
                    pcicorecallfunc.mem_free(dukpt_key);
                    return PCI_FUTUREKEY_INVALIDKEYLEN;
                }
                
                //sxl20110608 encrypt dukptk use MMK
                memset(encryptkeydata,0,sizeof(encryptkeydata));
                memcpy(encryptkeydata,future_key.keydata,future_key.keylen);
                ret = pcikeymanage_keyprocessusingMMK(DECRYPT,encryptkeydata,future_key.keylen);
                if(ret != PCI_PROCESSCMD_SUCCESS)
                {
                    DDEBUG("\r\n pcidrv_keyprocess_usingMMK ret = %d \r\n",ret);
                    pcicorecallfunc.mem_free(dukpt_key);
                    return ret;
                }
                memcpy(future_key.keydata,encryptkeydata,future_key.keylen);

                
                pcikeymanage_dukptk_calculatecrc(future_key.keydata,future_key.keylen,tempcrc);
                if(Lib_memcmp(tempcrc,future_key.keycrc,2))
                {
                    Lib_memset((u8 *)&future_key,0,DUKPTFUTUREKSIZE);
                    pcicorecallfunc.mem_free(dukpt_key);
                    return PCI_FUTUREKEY_INVALIDCRC;
                }
            }

            pcicorecallfunc.mem_free(dukpt_key);
            
        }

        
    }

    return PCI_PROCESSCMD_SUCCESS;
}


/*******************************************************************
Author:   sxl
Function Name:    s32 pcikeymanage_check_dukptkey(void)
Function Purpose: check dukpt key
Input Paramters: N/A
Output Paramters:N/A
Return Value:    PCI_PROCESSCMD_SUCCESS  - check successed
                       other - save failed
Remark: N/A
Modify: N/A     20110608 //20140327
********************************************************************/
s32 pcikeymanage_check_dukptkey(void)
{
    s32 retcode;
    
    retcode = pcikeymanage_checkread_dukptkey();
    if(retcode == PCI_FUTUREKEY_INVALIDKEYLEN ||retcode == PCI_FUTUREKEY_INVALIDCRC)
    {
        return PCI_FUTUREKEY_INVALIDCRC;
    }

    return PCI_PROCESSCMD_SUCCESS;
}



/*******************************************************************
Author:   sxl
Function Name:    s32 pcikeymanage_check_dukptkexist(u8 keyindex)
Function Purpose: check dukpt key
Input Paramters: N/A
Output Paramters:N/A
Return Value:    PCI_PROCESSCMD_SUCCESS  - check successed
                       other - save failed
Remark: N/A
Modify: N/A     20110608  //20140327
********************************************************************/
s32 pcikeymanage_check_dukptkexist(u8 keyindex)
{
    s32 retcode;
    u8 tmpkeylen;
    u8 tempKSN[MAXKSNDATALEN];

    if(keyindex >= DUKPTKEY_MAXNUM)
    {
        return PCI_KEYINDEX_ERR;
    }
    gDukptkInfo.currentkeyindex = keyindex;
    retcode = pcikeymanage_dukptk_readKSN(gDukptkInfo.currentkeyindex,tempKSN);
    if(retcode != PCI_PROCESSCMD_SUCCESS)
    {
        DDEBUG("%s: read dukptk KSN error!\r\n",__FUNCTION__);
        return retcode;
    }
    memcpy(gDukptkInfo.KSN,tempKSN,MAXKSNDATALEN);
    tempKSN[7] &= 0x1F;
    gDukptkInfo.encryptcnt = bit21_to_ulong(&tempKSN[7]);

    pcikeymanage_dukptk_setshiftreg();
    retcode = pcikeymanage_dukptk_readfuturekey(gDukptkInfo.currentkeyindex,gDukptkInfo.curfuturekeyindex,&tmpkeylen,gDukptkInfo.keyreg);

    //key len error and crc err  //sxl20110608
    tmpkeylen = 0;
    memset(gDukptkInfo.keyreg,0,sizeof(gDukptkInfo.keyreg));
    
    if(retcode == PCI_FUTUREKEY_INVALIDKEYLEN ||retcode == PCI_FUTUREKEY_INVALIDCRC)
    {
        pcicorecallfunc.pci_security_locked_hold(0);
    }
    pcikeymanage_dukptk_clrglobalvariable();
    return retcode;
    
}


/*******************************************************************
Author:   sxl
Function Name: s32 pcikeymanage_poskey_saveappdukptkey(u8 *databuf)
Function Purpose:save dukpt key
Input Paramters: 
                        databuf    - key data
Output Paramters:N/A
Return Value:    PCI_PROCESSCMD_SUCCESS  - save successed
                       other - save failed
Remark: N/A
Modify: N/A       20110608  //20140327
********************************************************************/
s32 pcikeymanage_poskey_saveappdukptkey(u8 *databuf)
{
    u8 ksnlen,keyindex,keylen;  //keytype,
    u8 ksnbuf[100];
    //u8 *appname;
    s32 ret;
    u8 groupindex;
    
    groupindex = databuf[0];
    //keytype  = databuf[1];
    ksnlen   = databuf[2];
    keyindex = databuf[3];
    keylen   = databuf[4];
    //appname  = &databuf[45];
    //appname[32] = 0;    //sxl?
    
    DDEBUG("\r\nsave appdukptk %02x %02x %02x %02x\r\n",groupindex,ksnlen,keyindex,keylen);
    
    if((keylen != 16) &&(keylen != 24))
    {
        return KM_APP_KEY_LEN_ERROR;
    }
    if(ksnlen != 10)
    {
        return KM_APP_KEY_LEN_ERROR;
    }

    memset(ksnbuf,0,sizeof(ksnbuf));
    memcpy(ksnbuf,&databuf[5],keylen);
    memcpy(ksnbuf+keylen,&databuf[5+24],ksnlen);

    ret = pcikeymanage_pos_savedukptkey(groupindex,keyindex,keylen,ksnlen,ksnbuf);

    if(ret < 0)
    {
        return KM_APP_KEY_WRITE_ERROR;
    }
    
    return ret;
}



void pcikeymanage_dukptk_GetXORKey(const u8 *pBKey1,const u8 *pBKey2,u8 *pBOutResult,s32 iLength)
{
    u8 *bTemp;
    u8 ch1;
    u8 ch2;
    s32 loop = 0;
    bTemp = pBOutResult;
    for(loop = 0 ; loop < iLength; loop++)
    {
        ch1 = *(pBKey1 + loop);
        ch2 = *(pBKey2 + loop);
        *bTemp = (ch1)^(ch2);
        bTemp++;
    }
}



void pcikeymanage_dukptk_GetIK(const u8 *pBKSN,u8 *pBDK,u8 *pBIK)
{
    u8 tmpKEY[8];
    u8 TMP[16];
    u8 toXO[] = {0xc0, 0xc0, 0xc0, 0xc0, 0x00, 0x00, 0x00, 0x00, 
                                              0xc0, 0xc0, 0xc0, 0xc0, 0x00, 0x00, 0x00, 0x00};
    memcpy(tmpKEY,(unsigned char *)pBKSN,8);
    tmpKEY[7] &= 0xE0;
    //triple_des_enc(tmpKEY,pBDK);
    drv_tdes(ENCRYPT,tmpKEY,8,pBDK,16);
    memcpy(pBIK,tmpKEY,8);
    pcikeymanage_dukptk_GetXORKey(pBDK,toXO,TMP,16);
    memcpy(tmpKEY,(unsigned char *)pBKSN,8);
    tmpKEY[7] &= 0xE0;
    //triple_des_enc(tmpKEY,TMP);
    drv_tdes(ENCRYPT,tmpKEY,8,TMP,16);
    memcpy(pBIK + 8,tmpKEY,8);
}






s32 pcikeymanage_dukpt_get_pinkey(strDevDukpt *lpstrDevDukpt)
{   
    s32 ret;
    u8 dukptkey[24] = {0};
    u8 dukptksn[24] = {0};
    u8 keylen = 0;
    //u8 pindata[16] = {0};
    //u8 i;


    ////设置密钥分组
    ret = pcikeymanage_set_currappdukptk(lpstrDevDukpt->m_groupindex);
    if(ret != PCI_PROCESSCMD_SUCCESS)
    {
        DDEBUG("\r\n pcidrv_set_currappdukptk error ret = %d \r\n", ret);
        return ret;
    }
    
    //get pin key 从DUKPT模块获取到的是PIN KEY
    ret = pcikeymanage_getcurrtrans_dukptkinfo(0, lpstrDevDukpt->m_keyindex, dukptkey, &keylen, dukptksn);
    if(ret != PCI_PROCESSCMD_SUCCESS)
    {
        DDEBUG("\r\n pcidrv_getcurrtrans_dukptkinfo error ret = %d \r\n", ret);
        return ret;
    }

    lpstrDevDukpt->m_keylen = keylen;
    memcpy(lpstrDevDukpt->m_pinkey, dukptkey, keylen);
    
    DDEBUG("-------- pin key---------\r\n");
    
    lpstrDevDukpt->m_ksnlen = 10;
    memcpy(lpstrDevDukpt->m_ksn, dukptksn, 10); 

    DDEBUG("-------- ksn---------\r\n");

    return 0;//获取成功
}


s32 pcikeymanage_dukptk_load(strDukptInitInfo* lpstrDukptInitInfo)
{
   s32 ret;
   u8 keyksnbuf[100];
   u8 IK[24] = {0};

   pcikeymanage_dukptk_GetIK(lpstrDukptInitInfo->m_initksn, lpstrDukptInitInfo->m_initkey, IK);
 
   memset(keyksnbuf, 0, sizeof(keyksnbuf));
   memcpy(keyksnbuf, IK, lpstrDukptInitInfo->m_keylen);
   memcpy(keyksnbuf+lpstrDukptInitInfo->m_keylen, lpstrDukptInitInfo->m_initksn, lpstrDukptInitInfo->m_ksnlen);

   ret = pcikeymanage_pos_savedukptkey(lpstrDukptInitInfo->m_groupindex, lpstrDukptInitInfo->m_ksnindex, lpstrDukptInitInfo->m_keylen, lpstrDukptInitInfo->m_ksnlen, keyksnbuf);

   return ret;

}



s32 pcikeymanage_dukpt_check_keytmp(u32 nKeyGroup, u32 nKsnIndex,strDevDukpt *lpstrDevDukpt)
{
    s32 ret;

    if(nKeyGroup != lpstrDevDukpt->m_groupindex
        || nKsnIndex != lpstrDevDukpt->m_keyindex)
    {
        DDEBUG("换DUKPT\r\n");
        lpstrDevDukpt->flag = -1;   
    }    

    
    if(lpstrDevDukpt->flag == -1)
    {
        ret = pcikeymanage_dukpt_get_pinkey(lpstrDevDukpt);
        if(ret == 0)
        {
            lpstrDevDukpt->flag = 0;
            return 0;//获取密钥成功
        }
        else
        {
            return -1;//获取密钥失败
        }
		
    }

    return 0;
	
}
#else
s32 pcikeymanage_dukpt_check_keytmp(u32 nKeyGroup, u32 nKsnIndex,strDevDukpt *lpstrDevDukpt)
{
    return -1;
}
s32 pcikeymanage_dukptk_load(strDukptInitInfo* lpstrDukptInitInfo)
{
    return -1;
}
s32 pcikeymanage_getdukptkeyfile_absolutepath(u8 groupindex,s8 *absolutepath)
{
    return -1;
}
#endif




