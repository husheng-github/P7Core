


#include "pciglobal.h"


/*******************************************************************
Author:   
Function Name:s32 peikeymanage_poweroncheckappkey(void)
Function Purpose:power on check all key
Input Paramters: 
Output Paramters:N/A
Return Value:      N/A
Remark: N/A
Modify: N/A
********************************************************************/
s32 peikeymanage_poweroncheckappkey(void)
{
    
 	s32 ret;
    u8 tmp[64];
    s32 i;
    
    //sxlremark
    //先检查MMK ELRCK是否合法
    //Check the legality of MMK and ELRCK
    ret = pcitampermanage_readMMK(tmp,&i);
    if(ret != PCI_PROCESSCMD_SUCCESS&&ret != PCI_MMKNOTINITIAL)
    {
        DDEBUG("\r\n check MMK error %d \r\n",ret);
        Lib_memset(tmp,0,sizeof(tmp));
        return ret;    
    }
    Lib_memset(tmp,0,sizeof(tmp));
	
	
    #ifdef CHINAUNIONPAYKEY
    	return 0;
	#else
    
    
    //sxlremark
    //再检查所有密钥
    //Then check all the keys
    if(pcikeymanage_checkallappkey(0,NULL) != PCI_PROCESSCMD_SUCCESS)
    {
        #ifdef SXL_DEBUG
        DDEBUG("\r\npcidrv_deal24hourscheckappkey2\r\n");
        #endif
        return PCI_READKEY_ERR;
    }
    

    //sxlremark
    //检查所有的 DUKPTK
    //check all dukpt key  //sxl20110608
    #ifdef SUPPORTDUKPTK
    if(pcikeymanage_check_dukptkey() != PCI_PROCESSCMD_SUCCESS)
    {
        #ifdef SXL_DEBUG
        DDEBUG("\r\npcidrv_deal24hourscheckappkey3\r\n");
        #endif
        return PCI_READKEY_ERR;
    }
    #endif

   
    return PCI_PROCESSCMD_SUCCESS;
		
	#endif
	
	
}


