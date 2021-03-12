


#include "pciglobal.h"


PCITAMPERMANAGEFILEINFO   gPciTamperManageFileInfo;
u8 gBpkResetFlag;


/*******************************************************************
Author:   
Function Name: void pcitamper_manage_savetamperregdata(void)
Function Purpose:void pcitampermanage_regeneratemmkelrck_initialvalue(void)
Input Paramters:  reset the mmk elrck to 0xbb
Output Paramters:N/A
Return Value:      N/A
Remark: N/A
Modify: N/A    20170616
********************************************************************/
void pcitampermanage_regeneratemmkelrck_initialvalue(void)
{
	u8 mmkelrck[32];

	memset(mmkelrck,0xbb,sizeof(mmkelrck));
    
	pcicorecallfunc.pci_dryice_writeprogramkey(mmkelrck,32);
	
}

/*******************************************************************
Author:   
Function Name: s32 pcitampermanage_regeneratemmkelrck(void)
Function Purpose:regenerate the mmk and elrck
Input Paramters:  
Output Paramters:N/A
Return Value:      N/A
Remark: N/A
Modify: N/A  20170616
********************************************************************/
s32 pcitampermanage_regeneratemmkelrck(void)
{
	s32 ret;
    u8 tmp[32];
	
	pcicorecallfunc.pci_get_random_bytes(tmp,32);
	memcpy(&tmp[28],"\x55\xaa",2);   //add by 2011-07-09

	pciarithCrc16CCITT(tmp,28,&tmp[30]);     //add by 2011-07-09
	ret = pcicorecallfunc.pci_dryice_writeprogramkey(tmp,32);  //add by 2011-07-09
	//save the mmk data to mmk bpk file   //add by 2012-03-31
	if(ret == PCI_PROCESSCMD_SUCCESS)
	{
		pcitamper_manage_mmkbakfilesave(tmp,32);  //save mmk to bak file
		
	}
    else
    {
    	
    	//gPCITamperManage.gSecurityStatus = SYSTEMERROR_MMKREGENERATE;
		memset(tmp,0,sizeof(tmp));
		pcitamper_manage_mmkbakfilesave(tmp,32); 
		
    }
	return ret;
	
}
/*******************************************************************
Author:   
Function Name: void pcitamper_manage_savetamperregdata(void)
Function Purpose:save all tamper register value,not used now
Input Paramters: tamperstatusvalue - tamper status register value
Output Paramters:N/A
Return Value:      N/A
Remark: N/A
Modify: N/A   暂时未使用该函数
********************************************************************/
void pcitamper_manage_savetamperregdata(void)
{
	
}

/*******************************************************************
Author:   
Function Name: void pcitamper_manage_savetamper (u32 tamperstatusvalue)
Function Purpose:save tamper status value
Input Paramters: tamperstatusvalue - tamper status register value
Output Paramters:N/A
Return Value:      N/A
Remark: N/A
Modify: N/A   20170616
********************************************************************/
void pcitamper_manage_savetamper (u32 tamperstatusvalue)
{
	u32 TamperDSRReg;
	u8 i;

	#ifdef PCI_DEBUG
	u32 tempvalue;
	#endif

	#ifdef SXL_DEBUG
	DDEBUG("\r\ntamperstatusvalue = %08x %08x\r\n",tamperstatusvalue,gPCITamperManage.gTAMPERENABLEVALUE);
	#endif
	
	gPCITamperManage.gTamperDetectValue = tamperstatusvalue;
	if((tamperstatusvalue&gPCITamperManage.gTAMPERENABLEVALUE) == 0)
    {
        return;
    }
	
	
	TamperDSRReg = 0;
    for(i = 0; i < 4; i++)
    {
        TamperDSRReg = (TamperDSRReg<<8) + gPciTamperManageFileInfo.tamperreg[i];
    }
	
    if((TamperDSRReg&gPCITamperManage.gTAMPERENABLEVALUE)&&((TamperDSRReg&gPCITamperManage.gTAMPERENABLEVALUE) == (tamperstatusvalue&gPCITamperManage.gTAMPERENABLEVALUE)))  //已经保存了触发状态
    {
        return;
    }

	
	#ifdef PCI_DEBUG
	tempvalue = 1;
	for(i = 0;i<12;i++)
	{
		if(TamperDSRReg&(1<<i))
		{
			dev_debug_printf("\r\ntamper %d detected\r\n",i);
		}
	}

	if(TamperDSRReg&(1<<12))
	{
		dev_debug_printf("\r\nhigh voltage detected\r\n",i);
	}

	if(TamperDSRReg&(1<<13))
	{
		dev_debug_printf("\r\nlow voltage detected\r\n",i);
	}

	if(TamperDSRReg&(1<<14))
	{
		dev_debug_printf("\r\nhigh temperature detected\r\n",i);
	}

	if(TamperDSRReg&(1<<15))
	{
		dev_debug_printf("\r\nlow temperature detected\r\n",i);
	}
	
	
	if(TamperDSRReg&(1<<16))
	{
		dev_debug_printf("\r\nvol glitch detected\r\n",i);
	}

	if(TamperDSRReg&(1<<17))
	{
		dev_debug_printf("\r\n32k clk detected\r\n",i);
	}

	
	if(TamperDSRReg&(1<<18))
	{
		dev_debug_printf("\r\nmesh tamper detected\r\n",i);
	}

	if(TamperDSRReg&(1<<19))
	{
		dev_debug_printf("\r\nsoft tamper detected\r\n",i);
	}


	if(TamperDSRReg&(1<<20))
	{
		dev_debug_printf("\r\nscc tamper detected\r\n",i);
	}
	
	#endif
    
    
    gPciTamperManageFileInfo.tamperreg[0] = (u8)(tamperstatusvalue>>24);
    gPciTamperManageFileInfo.tamperreg[1] = (u8)(tamperstatusvalue>>16);
    gPciTamperManageFileInfo.tamperreg[2] = (u8)(tamperstatusvalue>>8);
    gPciTamperManageFileInfo.tamperreg[3] = (u8)(tamperstatusvalue);
    #ifdef PCI_DEBUG
	dev_debug_printf("\r\nsave tamper value:");
    for(i = 0;i < 4;i++)
    {
    	dev_debug_printf("%02x ",gPciTamperManageFileInfo.tamperreg[i]);
    }
	dev_debug_printf("\r\n");
	#endif
    pcitampermanage_filesave(&gPciTamperManageFileInfo);
	
	
}

/*******************************************************************
Author:   sxl
Function Name: s32 pcitampermanage_poweroncheckMMK(void)
Function Purpose:check  MMK when power on
Input Paramters: 
Output Paramters:N/A
Return Value:  N/A
Remark: N/A
Modify: by //20170616
********************************************************************/
s32 pcitampermanage_poweroncheckMMK(void)
{
	s32 ret = 1;
	u8 tmp[64];
	u8 checkvalue[2];
	u8 tmpdata[64];
	
	ret = pcicorecallfunc.pci_dryice_readprogramkey(tmp);
	if(ret != 0)
	{
		
		#ifdef SXL_DEBUG
		DDEBUG("\r\n%s 111\r\n",__FUNCTION__);
		#endif
		return PCI_READMMK_ERR;
		
	}
	
    memset(tmpdata,0xbb,sizeof(tmpdata));  //当为0xbb的时候，表示没有下载过密钥,每次上电要提示
	if(memcmp(tmp,tmpdata,32) == 0)
	{
	    #ifdef SXL_DEBUG
        DDEBUG("\r\n%s tamper download success\r\n",__FUNCTION__);
		#endif
		return PCI_MMKNOTINITIAL;
	}

	memcpy(tmpdata,"\xae\x05\x47\x7c\xab\xa7\x6b\x3e\xa5\x45\x1f\xf4\xa0\xe7\x33\xb6\xac\x15\x57\xe5\xa9\xb7\x7b\xa7\xa7\x55\x0f\x6d\xa2\xf7\x23\x2f",32);      //检查是不是全0
	if(Lib_memcmp(tmp,tmpdata,32) == 0) //all zero
	{
		return PCI_READMMK_ALLZERO;
	}

	memset(tmpdata,0,32);      //检查是不是全0
	if(Lib_memcmp(tmp,tmpdata,32) == 0) //all zero
	{
		return PCI_READMMK_ALLZERO;
	}
    
	//check crc
	if(Lib_memcmp(&tmp[28],"\x55\xaa",2))
	{
	    #ifdef SXL_DEBUG
	    DDEBUG("\r\n %s 222\r\n",__FUNCTION__);
		#endif
	    return PCI_READMMK_ERR;
	}
	
    pciarithCrc16CCITT(tmp,28,checkvalue);   
	if(Lib_memcmp(&tmp[30],checkvalue,2))   
	{
	    #ifdef SXL_DEBUG
	    DDEBUG("\r\n%s 333\r\n",__FUNCTION__);
		#endif
	    return PCI_READMMK_ERR;
	}
	
	return PCI_PROCESSCMD_SUCCESS;
    
	
}


/*******************************************************************
Author:   sxl
Function Name: s32 pcidrv_keyprocess_readMMK(u8 *mmkdata,s32 *mmkdatalen)
Function Purpose:read MMK
Input Paramters: *mmkdate      - MMK data
                        *mmkdatalen  - MMK data len
Output Paramters:N/A
Return Value:  N/A
Remark: N/A
Modify: by //20170616
********************************************************************/
s32 pcitampermanage_readMMK(u8 *mmkdata,s32 *mmkdatalen)
{
	s32 ret = 1;
	u8 tmp[64];
	u8 tmpdata[32];
	u8 checkvalue[2];

    ret = pcicorecallfunc.pci_dryice_readprogramkey(tmp);
	if(ret != PCI_PROCESSCMD_SUCCESS)
	{
	    #ifdef SXL_DEBUG
	    DDEBUG("\r\n%s 111\r\n",__FUNCTION__);
		#endif
	    return PCI_READMMK_ERR;
	}

	memset(tmpdata,0xbb,sizeof(tmpdata));
	if(memcmp(tmp,tmpdata,32) == 0)
	{
		#ifdef SXL_DEBUG
		DDEBUG("\r\n%s tamper download success\r\n",__FUNCTION__);
		#endif
		return PCI_MMKNOTINITIAL;
	}
	
    
	//check crc
	if(Lib_memcmp(&tmp[28],"\x55\xaa",2))
	{
	    #ifdef SXL_DEBUG
	    DDEBUG("\r\n%s 222\r\n",__FUNCTION__);
		#endif
	    return PCI_READMMK_ERR;
	}
	
    pciarithCrc16CCITT(tmp,28,checkvalue);   //add by 2011-07-09
	
	//if(memcmp(&tmp[34],checkvalue,2))
	if(Lib_memcmp(&tmp[30],checkvalue,2))   //add by 2011-07-09
	{
	    #ifdef SXL_DEBUG
	    DDEBUG("\r\n%s 333\r\n",__FUNCTION__);
		#endif
		return PCI_READMMK_ERR;
	}
	
    ret = pcitamper_manage_mmkbakcheckifconsistency(tmp,32);
	if(ret != PCI_PROCESSCMD_SUCCESS)   //20120411
	{
		pcitamper_manage_mmkbakfilesave(tmp,32);  //save mmk to bak file
	}
	
	
	memcpy(mmkdata,tmp,32);   //add by 2012-03-23
	*mmkdatalen = 32;
	#ifdef SXL_DEBUG
    DDEBUG("\r\n%s 444\r\n",__FUNCTION__);
	#endif
	return PCI_PROCESSCMD_SUCCESS;
    
	
}

//20170616
s32 pcitampermanage_readELRCK(u8 *elrckdata,s32 *elrckdatalen)
{
	
	s32 ret = 1;
	u8 tmp[64];
    s32 mmklen;
	
    ret = pcitampermanage_readMMK(tmp,&mmklen);
    if(ret != PCI_PROCESSCMD_SUCCESS)
    {
    	
    	#ifdef SXL_DEBUG
		DDEBUG("\r\n%s read err %d\r\n",__FUNCTION__,ret);
		#endif
    	return ret;
		
    }
    memcpy(elrckdata,&tmp[16],16);   
	        
	*elrckdatalen = 16;
	#ifdef SXL_DEBUG
	DDEBUG("\r\n%s read ok\r\n",__FUNCTION__);
	#endif
	return PCI_PROCESSCMD_SUCCESS;
	
	
}

/*******************************************************************
Author:   
Function Name: s32 pcitampermanage_checkmmk(void)
Function Purpose:check MMK
Input Paramters: tamperstatusvalue - tamper status register value
Output Paramters:N/A
Return Value:      N/A
Remark: N/A
Modify: N/A   20170616
********************************************************************/
s32 pcitampermanage_poweroncheckmmk(u8 poweronflag)
{
	s32 ret = PCI_PROCESSCMD_SUCCESS;
    u8 mmkbak[50];
	
    //触发肯定已开启，需要检查MMK是否被篡改
	ret = pcitampermanage_poweroncheckMMK();
	#ifdef PCI_DEBUG

	dev_debug_printf("\r\n%s ret = %d\r\n",__FUNCTION__, ret);
	#endif
	if(ret != PCI_MMKNOTINITIAL&&ret != PCI_PROCESSCMD_SUCCESS)
	{
		
		//检查dryice 模块是否复位过
		if( gBpkResetFlag == 0&&poweronflag == 0)
		{
			//还原MMK
			ret = pcitamper_manage_mmkbakrecover(mmkbak);
			#ifdef PCI_DEBUG
			dev_debug_printf("\r\npcitampermanage_poweroncheckmmk = %d\r\n",ret);
			#endif
			if(ret != PCI_PROCESSCMD_SUCCESS)
			{
				 pcitampermanage_regeneratemmkelrck_initialvalue();  //重新初始化
			}
		}
		
	}
	else if(ret == PCI_MMKNOTINITIAL)
	{
	    #ifdef PCI_DEBUG
		dev_debug_printf("\r\nmmk not initial\r\n");
		#endif
		ret = PCI_PROCESSCMD_SUCCESS;
		
	}
	
	return ret;
	
}


/*******************************************************************
Author:   
Function Name:void pcitampermanage_powerondelmmkbpkfile(void)
Function Purpose:check MMK
Input Paramters: tamperstatusvalue - tamper status register value
Output Paramters:N/A
Return Value:      N/A
Remark: N/A
Modify: N/A20170616
********************************************************************/
void pcitampermanage_powerondelmmkbpkfile(void)
{
    s32 i;
    u32 TamperDSRReg;
	
	
    TamperDSRReg = 0;
    for(i = 0;i < 4;i++)
    {
        TamperDSRReg = (TamperDSRReg<<8) + gPciTamperManageFileInfo.tamperreg[i];
    }
	
	
    #ifdef PCI_DEBUG
	dev_debug_printf("\r\nTamperDSRReg = %08x %08x\r\n",TamperDSRReg,gPCITamperManage.gTAMPERENABLEVALUE);
	#endif
	
	
    if(TamperDSRReg&gPCITamperManage.gTAMPERENABLEVALUE)
    {
    	
        memset(gPciTamperManageFileInfo.tamperreg,0,4);
        pcitampermanage_filesave(&gPciTamperManageFileInfo);
		
    }
	
}


/*******************************************************************
Author:   
Function Name:u32 pcitampermanage_save_tampertimes(u8 type,u32 tampervalue)
Function Purpose:save unlock tamper times
Input Paramters: N/A
Output Paramters:N/A
Return Value:      N/A
Remark: N/A
Modify: N/A    暂未使用
********************************************************************/
u32 pcitampermanage_save_tampertimes(u8 type,u32 tampervalue)
{
	
}


/*******************************************************************
Author:   
Function Name:void pcitampermanage_tamperunlock(void)
Function Purpose:unlock tamper times
Input Paramters: N/A
Output Paramters:N/A
Return Value:      N/A
Remark: N/A
Modify: N/A     暂未使用
********************************************************************/
void pcitampermanage_tamperunlock(void)
{
	pcitampermanage_save_tampertimes(0,gPCITamperManage.gTamperDetectValue);
	//add tampertimes
	pcitampermanage_powerondelmmkbpkfile();
	//系统重启
	
}



//20170616
void pcitampermanage_filesave(PCITAMPERMANAGEFILEINFO *pcitampermanagefileinfo)
{
	
	memcpy(pcitampermanagefileinfo->fileflag,TAMPERMANAGEINFO_FILEFLAG,4);
    dev_misc_machineparam_set(MACHINE_PARAM_TAMPERMAN, (u8 *)pcitampermanagefileinfo,sizeof(PCITAMPERMANAGEFILEINFO), 0);
  #if 0  
    pciarithCrc16CCITT(pcitampermanagefileinfo->fileflag,sizeof(PCITAMPERMANAGEFILEINFO)-2,pcitampermanagefileinfo->crc);

	pcicorecallfunc.pci_kern_write_file(TAMPERMANAGEINFO_FILENAME,(u8 *)pcitampermanagefileinfo,sizeof(PCITAMPERMANAGEFILEINFO)); 
	pcicorecallfunc.pci_kern_write_file(TAMPERMANAGEINFO_BAKFILENAME,(u8 *)pcitampermanagefileinfo,sizeof(PCITAMPERMANAGEFILEINFO)); 
  #endif	
}


//20170616
s32 pcitampermanage_fileread(PCITAMPERMANAGEFILEINFO *pcitampermanagefileinfo,u8 type)
{
    s32 ret;
  #if 0  
	u8 crc[2];
	u8 managefilename[50];
    
	memset(managefilename,0,sizeof(managefilename));
    if(type == 0)
    {
    	strcpy(managefilename,TAMPERMANAGEINFO_FILENAME);
    }
	else
	{
		strcpy(managefilename,TAMPERMANAGEINFO_BAKFILENAME);
	}
	
	ret = pcicorecallfunc.pci_kern_read_file(managefilename,(u8 *)pcitampermanagefileinfo,sizeof(PCITAMPERMANAGEFILEINFO),0);    
    if(ret == sizeof(PCITAMPERMANAGEFILEINFO))
    {
    	
    	if(memcmp(pcitampermanagefileinfo->fileflag,TAMPERMANAGEINFO_FILEFLAG,4) == 0)
    	{
            //check crc
			pciarithCrc16CCITT(pcitampermanagefileinfo->fileflag,sizeof(PCITAMPERMANAGEFILEINFO)-2,crc);
			if(memcmp(pcitampermanagefileinfo->crc,crc,2) == 0)
			{
				return PCI_PROCESSCMD_SUCCESS;
			}
    		
			
    	}
		
    }
  #endif
     ret = dev_misc_machineparam_get(MACHINE_PARAM_TAMPERMAN, (u8 *)pcitampermanagefileinfo,sizeof(PCITAMPERMANAGEFILEINFO), 0);

     if(ret >= 0)
     {
        return PCI_PROCESSCMD_SUCCESS;
     }
     
	 return PCI_DEAL_ERR;
	
}

//20170616
void pcitampermanage_file_initial(PCITAMPERMANAGEFILEINFO *pcitampermanagefileinfo)
{

	#ifdef PCI_DEBUG
    dev_debug_printf("\r\n%s\r\n",__FUNCTION__);
	#endif
	memset((u8 *)pcitampermanagefileinfo,0,sizeof(PCITAMPERMANAGEFILEINFO));
	
	memcpy(pcitampermanagefileinfo->tamperenablevalue,DEFAULT_TAMPERENABLEVALUE,4);
	
	pcitampermanage_filesave(pcitampermanagefileinfo);
	
}

//20170616
void pcitampermanage_initial(void)
{
    u8 i;
    s32 rst1,rst2;
    //PCITAMPERMANAGEFILEINFO   tempPciTamperManageFileInfo;
	
	memset(&gPciTamperManageFileInfo,0,sizeof(PCITAMPERMANAGEFILEINFO));
    rst1 = pcitampermanage_fileread(&gPciTamperManageFileInfo,0);

	
  #if 0
	memset(&tempPciTamperManageFileInfo,0,sizeof(PCITAMPERMANAGEFILEINFO));
	rst2 = pcitampermanage_fileread(&tempPciTamperManageFileInfo,1);
    
    #ifdef PCI_DEBUG
    dev_debug_printf("\r\nrslt1 = %d rstl2 = %d\r\n",rst1,rst2);
	#endif
    if(rst1 == PCI_PROCESSCMD_SUCCESS&&rst2 != PCI_PROCESSCMD_SUCCESS)
    {
    	//write bak file
    	pcicorecallfunc.pci_kern_write_file(TAMPERMANAGEINFO_BAKFILENAME,(u8 *)&gPciTamperManageFileInfo,sizeof(PCITAMPERMANAGEFILEINFO));
    }
    else if(rst1 != PCI_PROCESSCMD_SUCCESS&&rst2 == PCI_PROCESSCMD_SUCCESS)
    {
    	//write org file
    	memcpy(&gPciTamperManageFileInfo,&tempPciTamperManageFileInfo,sizeof(PCITAMPERMANAGEFILEINFO));
    	pcicorecallfunc.pci_kern_write_file(TAMPERMANAGEINFO_FILENAME,(u8 *)&tempPciTamperManageFileInfo,sizeof(PCITAMPERMANAGEFILEINFO));
    }
	else if(rst1 == PCI_PROCESSCMD_SUCCESS&&rst2 == PCI_PROCESSCMD_SUCCESS)
	{
		//check if the value is the same
		if(memcmp(&gPciTamperManageFileInfo,&tempPciTamperManageFileInfo,sizeof(PCITAMPERMANAGEFILEINFO)))
		{
			pcicorecallfunc.pci_kern_write_file(TAMPERMANAGEINFO_BAKFILENAME,(u8 *)&gPciTamperManageFileInfo,sizeof(PCITAMPERMANAGEFILEINFO));
		}
		
	}
    else
  #endif 
    if(rst1 < 0)
    {
    	pcitampermanage_file_initial(&gPciTamperManageFileInfo);
    }

	
    //暂时以默认的tamper值为主
    if(memcmp(gPciTamperManageFileInfo.tamperenablevalue,DEFAULT_TAMPERENABLEVALUE,4))
    {
    	#ifdef PCI_DEBUG
        dev_debug_printf("\r\n save default tamper value\r\n");
		#endif
		memcpy(gPciTamperManageFileInfo.tamperenablevalue,DEFAULT_TAMPERENABLEVALUE,4);
		pcitampermanage_filesave(&gPciTamperManageFileInfo);
		
		
    }
	
	
	gPCITamperManage.gTAMPERENABLEVALUE = 0;
	for(i = 0;i < 4;i++)
	{
		gPCITamperManage.gTAMPERENABLEVALUE = (gPCITamperManage.gTAMPERENABLEVALUE<<8) + gPciTamperManageFileInfo.tamperenablevalue[i];
	}

	
    #ifdef PCI_DEBUG
	dev_debug_printf("\r\ngPCITamperManage.gTAMPERENABLEVALUE = %08x\r\n",gPCITamperManage.gTAMPERENABLEVALUE);
	#endif
	
}

//20170616
u32 pcitampermanage_getsavedsenexttype(void)
{
	u32 temp = 0;
	u8 i;

	for(i = 0;i < 4;i++)
	{
		temp = (temp<<8) + gPciTamperManageFileInfo.sen_exttype_reg[i];
	}

    #ifdef PCI_DEBUG
	dev_debug_printf("\r\npcitampermanage_getsavedsenexttype = %08x\r\n",temp);
	#endif
	
	return temp;
	
}


//20170616
void pcitampermanage_savesenexttype(u32 senexttyperegvalue)
{
    u32 reg;
	
	
	reg = pcitampermanage_getsavedsenexttype();
	if(reg == senexttyperegvalue)
	{
		return;
	}
	
	
	gPciTamperManageFileInfo.sen_exttype_reg[0] = (u8)(senexttyperegvalue>>24);
	gPciTamperManageFileInfo.sen_exttype_reg[1] = (u8)(senexttyperegvalue>>16);
	gPciTamperManageFileInfo.sen_exttype_reg[2] = (u8)(senexttyperegvalue>>8);
	gPciTamperManageFileInfo.sen_exttype_reg[3] = (u8)(senexttyperegvalue);
	
	#ifdef PCI_DEBUG
	dev_debug_printf("\r\npcitampermanage_savesenexttype = %08x\r\n",senexttyperegvalue);
	#endif
	
	pcitampermanage_filesave(&gPciTamperManageFileInfo);
	
	
}

//20170616
void pcitampermanage_savetempdryicereg(u8 *dryicereg)
{

	if(gPciTamperManageFileInfo.dryicereg[0] == 0||memcmp(&gPciTamperManageFileInfo.dryicereg[1],dryicereg,32))
	{
		#ifdef PCI_DEBUG
		dev_debug_printf("\r\npcitampermanage_savetempdryicereg\r\n");
		#endif
		
		gPciTamperManageFileInfo.dryicereg[0] =1;
		memcpy(&gPciTamperManageFileInfo.dryicereg[1],dryicereg,32);
	    pcitampermanage_filesave(&gPciTamperManageFileInfo);
		
	}
	
}

//20170616
void pcitampermanage_clrtempdryicereg(void)
{
    #ifdef PCI_DEBUG
	dev_debug_printf("\r\npcitampermanage_clrtempdryicereg\r\n");
	#endif
	memset(gPciTamperManageFileInfo.dryicereg,0,sizeof(gPciTamperManageFileInfo.dryicereg));
    pcitampermanage_filesave(&gPciTamperManageFileInfo);
}

//20170616
void pcitampermanage_recover_dryicereg(void)
{
	if(gPciTamperManageFileInfo.dryicereg[0])
	{
	    #ifdef PCI_DEBUG
		dev_debug_printf("\r\npcitampermanage_recover dryice reg\r\n");
		#endif
		dev_dryice_writesecurekey(&gPciTamperManageFileInfo.dryicereg[1],32);
		pcitampermanage_clrtempdryicereg();
	}
}





//20170616
void pcitamper_manage_clrbpkporstatus(void)
{
	
	if(BPK_IsPORReset())
	{
		#ifdef PCI_DEBUG
		dev_debug_printf("\r\nBPK is reset\r\n");
		#endif
		BPK_PORClear();
	}
	else
	{
		#ifdef PCI_DEBUG
		dev_debug_printf("\r\nBPK not reset\r\n");
		#endif
	}
	gBpkResetFlag = 0;
}


//20170616
void pcitamper_manage_poweronreadporstatus(void)
{
	gBpkResetFlag = 0;
	if(BPK_IsPORReset())
	{
		
		#ifdef PCI_DEBUG
		dev_debug_printf("\r\ngBpkResetFlag set\r\n");
		#endif
		gBpkResetFlag = 1;
		 
	}
}

void pcitampermanage_disable(void)
{
	gPCITamperManage.gTAMPERENABLEVALUE = 0;
}

