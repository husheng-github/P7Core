

#include "pciglobal.h"


/*******************************************************************
Author:   sxl
Function Name:void pcitask_calc_ecb1(u8 *mack,u8 macklen,u8 *srcdata,u32 srcdatalen)
Function Purpose:mac algorithm ECB1
Input Paramters: 
                         mack - mac key
                         macklen - mac key len
                         srcdata  - source data
                         srcdatalen - source data len
Output Paramters:*srcdata - mac result 
Return Value:    PCI_PROCESSCMD_SUCCESS  -      success
                       other - failed
Remark: N/A
Modify: N/A     1130 //20140327 
********************************************************************/
void pcitask_calc_ecb1(u8 *mack,u8 macklen,u8 *srcdata,u32 srcdatalen)
{
    u8 appendlen;
    u32 datalen;
    u32 datablock;
    u32 i,j,offset;
    

    appendlen = srcdatalen%8;
    if(appendlen)
    {
        memset(&srcdata[srcdatalen],0,8-appendlen);
        datalen= srcdatalen + 8 - appendlen;
    }
    else
    {
        datalen = srcdatalen;
    }
    datablock = datalen/8;
    offset = 8;
    for(i=1;i<datablock;i++)
    {
        for(j = 0;j<8;j++)
        {
            srcdata[j] ^= srcdata[j+offset];
        }
         offset += 8; 
    }
    DES_TDES(mack, macklen, srcdata, 8, 1);
    
}
/*******************************************************************
Author:   sxl
Function Name:void pcidrv_macalgorithm_ecb1(u8 *mack,u8 macklen,u8 *srcdata,u32 srcdatalen)
Function Purpose:mac algorithm ECB2
Input Paramters: 
                         mack - mac key
                         macklen - mac key len
                         srcdata  - source data
                         srcdatalen - source data len
Output Paramters:*srcdata - mac result 
Return Value:    PCI_PROCESSCMD_SUCCESS  -      success
                       other - failed
Remark: N/A
Modify: N/A         1130  //20140327
********************************************************************/
s32 pcitask_calc_ecb2(u8 *mack,u8 macklen,u8 *srcdata,u32 srcdatalen)
{
    switch(macklen/8)//ÃÜÔ¿ÀàÐÍ
    {
        case 0x01:
            DesMac(mack,srcdata,srcdatalen);
            break;
        case 0x02:
            Des3_16Mac_2(mack,srcdata,srcdatalen);
            break;
        case 0x03:
            Des3_24Mac_2(mack,srcdata,srcdatalen);
            break;
        default:
            return PCI_KEYLEN_ERR;
            break;
    }
    return PCI_PROCESSCMD_SUCCESS;
}
/*******************************************************************
Author:   sxl
Function Name:void pcidrv_macalgorithm_ecb3(u8 *mack,u8 macklen,u8 *srcdata,u32 srcdatalen)
Function Purpose:mac algorithm ECB2
Input Paramters: 
                         mack - mac key
                         macklen - mac key len
                         srcdata  - source data
                         srcdatalen - source data len
Output Paramters:*srcdata - mac result 
Return Value:    PCI_PROCESSCMD_SUCCESS  -      success
                       other - failed
Remark: N/A
Modify: N/A         1130  //20140327
********************************************************************/
s32 pcitask_calc_ecb3(u8 *mack,u8 macklen,u8 *srcdata,u32 srcdatalen)
{
    switch(macklen/8)
    {
        case 0x01:
            DesMac(mack,srcdata,srcdatalen);
            break;
        case 0x02:
            Des3_16Mac(mack,srcdata,srcdatalen);
            break;
        case 0x03:
            Des3_24Mac(mack,srcdata,srcdatalen);
            break;
        default:
            return PCI_KEYLEN_ERR;
            break;
    }

    return PCI_PROCESSCMD_SUCCESS;
}


/*******************************************************************
Author:   sxl
Function Name:s32 pcitask_calcmac_dataprocessmacalgorithm(u8 groupindex,u8 macalgorithmindex,u8 keyindex,u32 srcdatalen,u8 *srcdata,u8 *macresult)
Function Purpose:mac algorithm 
Input Paramters: 
                        *appname  - app name
                        macalgorithmindex - mac algorithm index
                        keyindex   - key index
                        srcdatalen - source data len calculate 
                        srcdata     - source data to 
Output Paramters:*macresult - mac result
Return Value:    PCI_PROCESSCMD_SUCCESS  -      success
                       other - failed
Remark: N/A
Modify: N/A       1130  //20140327  
********************************************************************/
s32 pcitask_calcmac_dataprocessmacalgorithm(u8 groupindex,u8 macalgorithmindex,u8 keyindex,u32 srcdatalen,u8 *srcdata,u8 *macresult)
{
    s32 ret;
    u8 macklen,mack[24];
    u8 *tmpdata;//[2048];
    
    if(keyindex >= MACK_MAXINDEX)
    {
        return PCI_KEYINDEX_ERR;
    }

    ret = pcikeymanage_readandcheckappkey(groupindex,TYPE_MACK,keyindex,&macklen,mack);
    if(ret != PCI_PROCESSCMD_SUCCESS)
    {
        return PCI_READKEY_ERR;
    }
    DDEBUG("\r\npcidrv_readandcheck_appkey read mack success\r\n");

    if((srcdatalen == 0)||(srcdatalen > 2048))
    {
        memset(mack,0,macklen);
        return PCI_DATALEN_ERR;
    }
    tmpdata = (u8 *)pcicorecallfunc.mem_malloc(2048);
    if(tmpdata == NULL)
    {
        return PCI_KMALLOCMEM_ERR;
    }
    memset(tmpdata,0,2048);
    memcpy(tmpdata,srcdata,srcdatalen);

    
    if(macalgorithmindex == 0)
    {
        pcitask_calc_ecb1(mack,macklen,tmpdata,srcdatalen);
    }
    else if(macalgorithmindex == 1)
    {
        ret = pcitask_calc_ecb2(mack,macklen,tmpdata,srcdatalen);
        if(ret != PCI_PROCESSCMD_SUCCESS)
        {
            memset(mack,0,macklen);
            pcicorecallfunc.mem_free(tmpdata);
            return ret;
        }
    }
    else if(macalgorithmindex == 2)
    {
        ret = pcitask_calc_ecb3(mack,macklen,tmpdata,srcdatalen);
        if(ret != PCI_PROCESSCMD_SUCCESS)
        {
            memset(mack,0,macklen);
            pcicorecallfunc.mem_free(tmpdata);
            return ret;
        }
    }
    else
    {
        memset(mack,0,macklen);
        pcicorecallfunc.mem_free(tmpdata);
        return PCI_MACALGORITHMINDEX_ERR;
    }
    memset(mack,0,macklen);
    memcpy(macresult,tmpdata,8);
    pcicorecallfunc.mem_free(tmpdata);
    return PCI_PROCESSCMD_SUCCESS;
	
	
}


/*******************************************************************
Author:   sxl
Function Name: s32 pcitask_calcmac_dataprocess(u8 *appname,u8 groupindex,u8 macalgorithmindex,u8 keyindex,u32 srcdatalen,u8 *srcdata,u8 *macresult)
Function Purpose:get mac data parse
Input Paramters: 
                        *rxbuf - Data from user space
                        rxbuflen   - Data len
Output Paramters:*macresult - mac result
Return Value:    PCI_PROCESSCMD_SUCCESS  -      success
                       other - failed
Remark: N/A
Modify: N/A 	sxl 1130	  //20140327
********************************************************************/
s32 pcitask_calcmac_dataprocess(u8 *appname,u8 groupindex,u8 macalgorithmindex,u8 keyindex,u32 srcdatalen,u8 *srcdata,u8 *macresult)
{
	s32 ret;
    
	ret = pcikeymanageauthen_sensitiveservicheck(groupindex,appname);
	if(ret != PCI_PROCESSCMD_SUCCESS)
	{
		return ret;
	}
	
	ret = pcitask_calcmac_dataprocessmacalgorithm(groupindex,macalgorithmindex,keyindex,srcdatalen,srcdata,macresult);
	return ret;
	
}


////20140327
#ifdef DDI_DUKPT_ENABLE
s32 pcitask_calcmac_usedukptk(u8 *appname,u8 groupindex,u8 macalgorithmindex,u8 dukptkindex,u32 srcdatalen,u8 *srcdata,u8 *macresult)
{
	s32 ret;
	u8 dukptkdata[24];
	u8 dukptkdatalen;
	u8 ksndata[MAXKSNDATALEN];
	u8 *tmpdata;//[2048];
    
    
	ret = pcikeymanageauthen_sensitiveservicheck(groupindex,appname);
	if(ret != PCI_PROCESSCMD_SUCCESS)
	{
		return ret;
	}
	
	if((srcdatalen == 0)||(srcdatalen > 2048))
	{
	    DDEBUG("%s:source data len is error",__FUNCTION__);
		return PCI_INPUTPARAMS_ERR;
	}

	ret = pcikeymanage_set_currappdukptk(groupindex);
	if(ret != PCI_PROCESSCMD_SUCCESS)
	{
	    DDEBUG("%s:set current dukpt key error! \r\n",__FUNCTION__);
		return ret;
	}
	Lib_memset(dukptkdata,0,sizeof(dukptkdata));
	dukptkdatalen = 0;
	Lib_memset(ksndata,0,sizeof(ksndata));
    ret = pcikeymanage_getcurrtrans_dukptkinfo(1,dukptkindex,dukptkdata,&dukptkdatalen,ksndata);
    if(ret != PCI_PROCESSCMD_SUCCESS)
	{
		return ret;
	}
	
    tmpdata = (u8 *)pcicorecallfunc.mem_malloc(2048);
	if(tmpdata == NULL)
	{
		return PCI_KMALLOCMEM_ERR;
	}
	memset(tmpdata,0,2048);
	memcpy(tmpdata,srcdata,srcdatalen);
	ret = PCI_PROCESSCMD_SUCCESS;
	if(macalgorithmindex == 0)
	{
		pcitask_calc_ecb1(dukptkdata,dukptkdatalen,tmpdata,srcdatalen);
	}
	else if(macalgorithmindex == 1)
	{
		ret = pcitask_calc_ecb2(dukptkdata,dukptkdatalen,tmpdata,srcdatalen);
	}
	else if(macalgorithmindex == 2)
	{
		ret = pcitask_calc_ecb3(dukptkdata,dukptkdatalen,tmpdata,srcdatalen);
	}
	else
	{
		ret = PCI_MACALGORITHMINDEX_ERR;
	}
	if(ret == PCI_PROCESSCMD_SUCCESS)
	{
		memcpy(macresult,tmpdata,8);
		memcpy(&macresult[8],ksndata,MAXKSNDATALEN);
	}
	Lib_memset(ksndata,0,sizeof(ksndata));
	Lib_memset(dukptkdata,0,dukptkdatalen);
	
	pcicorecallfunc.mem_free(tmpdata);
	return ret;
	
}
#endif

s32 pcitask_calcmac_getmac(u32 command,u8 *recdata,u32 recdatalen,u8 *macblock)
{
    //s32 ret = 0;

    u8 macalgorithmindex;
	u8 keyindex;
	u32 srcdatalen;
	u8 *srcdata;
    u8 groupindex;
	
	if(recdatalen >1024 || recdatalen < 6)
	{
		return PCI_INPUTPARAMS_ERR;
	}
	
	if(command == COMMAND_GETMAC)
	{
		
	}
	#ifdef SUPPORTDUKPTK
    else if(command == COMMAND_GETDUKPTKMAC)
    {
    	
    }
	#endif
	else
	{
		return PCI_INPUTPARAMS_ERR;
	}


    
	groupindex = recdata[0];
	macalgorithmindex = recdata[1];
	keyindex = recdata[2];
	srcdatalen = (((u32)recdata[3])<<8) + recdata[4];
	srcdata = &recdata[5];
    
	if(command == COMMAND_GETMAC)
	{
		return pcitask_calcmac_dataprocess(NULL,groupindex,macalgorithmindex,keyindex,srcdatalen,srcdata,macblock);
	}
	#ifdef DDI_DUKPT_ENABLE
	else
	{
		return pcitask_calcmac_usedukptk(NULL,groupindex,macalgorithmindex,keyindex,srcdatalen,srcdata,macblock);
	}
    #endif
	
    
	//return ret;
    
    
	
}

