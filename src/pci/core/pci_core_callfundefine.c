


#include "pciglobal.h"


//sxl?2017
int RSA_PKDecrypt(unsigned char * output, unsigned int *outputLen,
                  unsigned char * input,  unsigned int inputLen,
                  unsigned char * PK, unsigned int pkLen)
{
	return 0;
}
	


const PCI_CORE_CALLFUNC pcicorecallfunc = 
{      
      #if (!(defined(TRENDIT_COS) ||defined(TRENDIT_CORE)))
		k_malloc,
		k_free,
        NULL,
		NULL,
		dev_trng_read,
		fs_create_file,
		fs_read_file,
		fs_write_file,
		fs_insert_file,
		fs_access_file,
		fs_delete_file,
		fs_rename_file,
		temp_pcidrv_readCAK,
		temp_pcidrv_processwithCAK,
		dev_user_timer_open,
		dev_user_timer_close,
		NULL,
		devarith_desencrypt,
		devarith_hash,
		dev_RSA_PKEncrypt,
		dev_RSA_PKDecrypt,
		NULL,
		dev_file_hash,
		NULL,
		NULL
      #else
		k_malloc,
		k_free,
        NULL,//dev_dryice_writesecurekey,
		NULL,//dev_dryice_readsecurekey,
		dev_trng_read,
		fs_create_file,
		fs_read_file,
		fs_write_file,
		fs_insert_file,
		fs_access_file,
		fs_delete_file,
		fs_rename_file,
		temp_pcidrv_readCAK,
		temp_pcidrv_processwithCAK,
		dev_user_timer_open,
		dev_user_timer_close,
		NULL,//pcitamper_manage_selfcheck_securitylocked,
		devarith_desencrypt,
		devarith_hash,
		dev_RSA_PKEncrypt,
		dev_RSA_PKDecrypt,
		NULL,
		dev_file_hash,
		dev_RSA_SKEncrypt,
		dev_RSA_SKDecrypt
		#endif
};
//PCI_CORE_CALLFUNC pcicorecallfunc;
//PCI_CORE_CALLFUNCDEFINE pcilibmemcalldef;

s32 temp_pcidrv_readCAK(u8 *CAKdata)
{
	return 0;
}


s32 temp_pcidrv_processwithCAK(u8 mode,u8 *data,u32 datalen)
{
	return 0;
}

#if 1//(!(defined(TRENDIT_COS) ||defined(TRENDIT_CORE)))


#else
void pci_core_clearinitialflag(void)
{
	
	#ifdef SXL_DEBUG
	dev_debug_printf("\r\nclear PCI flag\r\n");
	#endif
	
	memset(gPciTamperManageFileInfo.pciflag,0,sizeof(gPciTamperManageFileInfo.pciflag));
	pcitampermanage_filesave(&gPciTamperManageFileInfo);
	
}


s32 pci_core_saveinitialflag(u8 dryicetype)
{
	
	u8 tempdata[20];
	s32 ret;
	
	#ifdef SXL_DEBUG
	dev_debug_printf("\r\nset PCI flag:%d\r\n",dryicetype);
	#endif
	
	memset(tempdata,0,sizeof(tempdata));
	if(dryicetype == 1)
	{
		strcpy(tempdata,PCIINITIALEDFLAG);
	}
	else if(dryicetype == 2)
	{
		strcpy(tempdata,PCITESTEDFLAG);
	}
    else
    {
    	//return 1;
    	memset(tempdata,0,sizeof(tempdata));  //sxl?2017要在上层设置不能降级
    }
	
	memset(gPciTamperManageFileInfo.pciflag,0,sizeof(gPciTamperManageFileInfo.pciflag));
    strcpy(gPciTamperManageFileInfo.pciflag,tempdata);
	pcitampermanage_filesave(&gPciTamperManageFileInfo);
	
	return 0;
	
}


s32 pci_core_checkinitialflag(void)
{
  #if 1     //关闭触发检查标志,20170913 test
	s32 ret;
	
	if(memcmp(gPciTamperManageFileInfo.pciflag,PCIINITIALEDFLAG,16) == 0)
	{
	    #ifdef SXL_DEBUG
        dev_debug_printf("%s(%d):pciflag set 1\r\n", __FUNCTION__, __LINE__);
		#endif
		return 1;
	}
    else if(memcmp(gPciTamperManageFileInfo.pciflag,PCITESTEDFLAG,16) == 0)
    {
    	#ifdef SXL_DEBUG
        dev_debug_printf("%s(%d):pciflag set 2\r\n", __FUNCTION__, __LINE__);
		#endif
    	return 2;
		
    }
	
	
    #ifdef SXL_DEBUG
    dev_debug_printf("%s(%d):pciflag not set\r\n", __FUNCTION__, __LINE__);
	#endif
  #endif  
	return 0;  //sxl?2017
	
}
/*******************************************************************
Author:   
Function Name: void pci_core_callfunc_set (void)
Function Purpose:define pci security call function
Input Paramters: 
Output Paramters:N/A
Return Value:      N/A
Remark: N/A
Modify: N/A
********************************************************************/
void pci_core_callfunc_set(void)
{
	
}


#endif



