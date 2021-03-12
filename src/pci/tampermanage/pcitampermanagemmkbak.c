

#include "pciglobal.h"


/*******************************************************************
Author:   
Function Name: s32 pcitamper_manage_mmkbakfileread(u8 *mmkbpkdata)
Function Purpose:read mmk bpk file
Input Paramters: tamperstatusvalue - tamper status register value
Output Paramters:N/A
Return Value:      N/A
Remark: N/A
Modify: N/A
********************************************************************/
s32 pcitamper_manage_mmkbakfileread(u8 *mmkbpkdata)
{
	
    #ifndef USEMMKBAKFILE
    return PCI_MMKBPKFILENOTEXIST;
    #endif
	
	if(memcmp(gPciTamperManageFileInfo.mmkdata,PCIMMKBPKFILEFLAG,4) == 0)
	{
		#ifdef PCI_DEBUG
		dev_debug_printf("\r\nmmkbak file read success\r\n");
		#endif
		memcpy(mmkbpkdata,&gPciTamperManageFileInfo.mmkdata[4],32);
		return PCI_PROCESSCMD_SUCCESS;
		
	}
   
    #ifdef PCI_DEBUG
	dev_debug_printf("\r\nmmkbak file read fail\r\n");
	#endif
    return PCI_MMKBPKFILEERR;
    
}


/*******************************************************************
Author:   sxl
Function Name:   s32 pcitamper_manage_mmkbakfilesave(u8 *mmkdata,u8 mmkdatalen)
Function Purpose: save mmk bak file
Input Paramters: mmkdata: mmk data 
                         mmkdatalen: mmk data length
                         dsr_srreg tamper status
Output Paramters: N/A
Return Value: TRUE or FALSE
Remark: 
Modify: N/A         //20140327
********************************************************************/
s32 pcitamper_manage_mmkbakfilesave(u8 *mmkdata,u8 mmkdatalen)
{
	    
    #ifndef USEMMKBAKFILE
    return PCI_MMKBPKFILEERR;
    #endif
	
    memcpy(&gPciTamperManageFileInfo.mmkdata,PCIMMKBPKFILEFLAG,4);
	if(mmkdatalen != 32)
	{
		return PCI_MMKBPKFILEERR;
	}

    #ifdef PCI_DEBUG
	dev_debug_printf("\r\nsave mmk\r\n");
	#endif
	
	memcpy(&gPciTamperManageFileInfo.mmkdata[4],mmkdata,32);
	pcitampermanage_filesave(&gPciTamperManageFileInfo);
	
	
    return PCI_PROCESSCMD_SUCCESS;
	
}



/*******************************************************************
Author:  sxl
Function Name: s32 pcitamper_manage_mmkbakcheckifconsistency(u8 *originalmmkdata,s32 originalmmkdatalen)
Function Purpose: check if the mmk bak file is consistency with the original mmk data
Input Paramters: N/A      
Output Paramters: N/A
Return Value: TURE or FALSE
Remark: 
Modify: N/A //20140327
********************************************************************/
s32 pcitamper_manage_mmkbakcheckifconsistency(u8 *originalmmkdata,s32 originalmmkdatalen)
{
    s32 ret;
    u8 mmkbpkdata[32];
	
	
    #ifndef USEMMKBAKFILE
    return PCI_MMKBPKDATANOTCONSISTENCY;
    #endif
    
    if(memcmp(gPciTamperManageFileInfo.mmkdata,PCIMMKBPKFILEFLAG,4))
    {
        #ifdef PCI_DEBUG
		dev_debug_printf("\r\npcitamper_manage_mmkbakcheckifconsistency flag err\r\n");
		#endif
        return PCI_MMKBPKDATANOTCONSISTENCY;
    }
	
    //ompare the whole mmk and elrck data  //the total 32 Bytes
    if(Lib_memcmp(&gPciTamperManageFileInfo.mmkdata[4],originalmmkdata,originalmmkdatalen) == 0)
    {
        #ifdef PCI_DEBUG
		dev_debug_printf("\r\npcitamper_manage_mmkbakcheckifconsistency same\r\n");
		#endif
        return PCI_PROCESSCMD_SUCCESS;
    }
    else
    {
        #ifdef PCI_DEBUG
		dev_debug_printf("\r\npcitamper_manage_mmkbakcheckifconsistency not same\r\n");
		#endif
        return PCI_MMKBPKDATANOTCONSISTENCY;
    }
}



s32 pcitamper_manage_mmkbakfilecheckifexist(void)
{
	#ifndef USEMMKBAKFILE
	return -1;
	#endif

    if(memcmp(gPciTamperManageFileInfo.mmkdata,PCIMMKBPKFILEFLAG,4) == 0)
    {
		return PCI_PROCESSCMD_SUCCESS;
    }
	else
	{
		return -1;
	}
}


/*******************************************************************
Author:  sxl
Function Name: s32 pcidrv_mmk_recover(u8 *mmkbpk)
Function Purpose: recover mmk from mmk bak file
Input Paramters: N/A      
Output Paramters: N/A
Return Value: TURE or FALSE
Remark: 2012-03-22
Modify: N/A //20140327
********************************************************************/
s32 pcitamper_manage_mmkbakrecover(u8 *mmkbpk)
{
    s32 ret;
	
	
    #ifndef USEMMKBAKFILE
    return PCI_MMKRECOVERFAILED;
    #endif

	#ifdef PCI_DEBUG
    DDEBUG("\r\n pcidrv_mmk_recover \r\n");
	for(ret = 0;ret < 36;ret++)
	{
		DDEBUG("%02x ",gPciTamperManageFileInfo.mmkdata[ret]);
	}
	DDEBUG("\r\n");
    #endif
	
	if(memcmp(gPciTamperManageFileInfo.mmkdata,PCIMMKBPKFILEFLAG,4))
	{
		return PCI_MMKRECOVERFAILED;
	}
    
    memcpy(mmkbpk,&gPciTamperManageFileInfo.mmkdata[4],32);
    ret = pcicorecallfunc.pci_dryice_writeprogramkey(mmkbpk,32);
    
    return ret;
    
}

