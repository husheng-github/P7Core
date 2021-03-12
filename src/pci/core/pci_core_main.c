

//#define AFX_PCI_VARIABLES
#include "pciglobal.h"
//#undef AFX_PCI_VARIABLES


s32  gPciInitState;
DUKPTK_INFO gDukptkInfo;
unsigned int appkeylockbits;
PINKEYUSEINTERNAL gPinKeyUseInternal;
SECURITYAUTHENTICATEINFO gSecurityAuthInfo;
u8 timerstartflag;
u32 appkeylocktimer;
PCIMANAGE gPciManagerInfo;
PCITAMPERMANAGE gPCITamperManage;  //sxl? tamper����??D����a��???��??????���|����,D����a?��?Y��2?t��?��?o?����
s32 gDownLoadKeyErr;
u8 pownonflag;



#ifdef SAVEPCILOG
PCILKEYLOGINFO gPciKeyLogInfo;
#endif
COMMUNICATEDATA *gCommData;
static volatile u32 g_pcitimerid=0;
static volatile u32 g_pcitampervalue;
static volatile u8 g_pcitampercnt;


/*******************************************************************
Author:   
Function Name:void pci_core_initialsecuritystatus(void)
Function Purpose:check security status
Input Paramters: 
Output Paramters:N/A
Return Value:      N/A
Remark: N/A
Modify: N/A
********************************************************************/
void pci_core_initialsecuritystatus(void)
{
    u8 mmkbak[50];
    s32 ret;
	
	
	{
		//û�д����ˣ���Ҫ��mmk file���flagɾ��
		pcitampermanage_powerondelmmkbpkfile();
        //�ϵ���������Կ
		ret = peikeymanage_poweroncheckappkey();
        #ifdef SXL_DEBUG
		dev_debug_printf("\r\npci_core_initialsecuritystatus ret = %08x\r\n",ret);
		#endif
		
		if(ret == PCI_PROCESSCMD_SUCCESS)
		{
			//20170616
		}
		else
		{
		    //sxl?2017 ���ﻹû����֤
		    ret = pcitamper_manage_mmkbakrecover(mmkbak);
			#ifdef PCI_DEBUG
			dev_debug_printf("\r\npcitampermanage_poweroncheckmmk = %d\r\n",ret);
			#endif
			if(ret != PCI_PROCESSCMD_SUCCESS&&memcmp(gPciTamperManageFileInfo.mmkdata,PCIMMKBPKFILEFLAG,4))
			{
				 pcitampermanage_regeneratemmkelrck_initialvalue();  //���³�ʼ��
			}
            ret = pcitampermanage_poweroncheckMMK(); // check mmk again //sxl?��Ҫ�鿴�Ƿ�û���¹�����Կ
			if(ret != PCI_PROCESSCMD_SUCCESS&&ret !=PCI_MMKNOTINITIAL)
			{
				
			}
            
			
		}
		
	}
	
	//pcikeymanageauthen_readpcimanageinfo();
	
}


void pcitamper_manage_tampercfg(DRYICETAMPERCONFIG *dryicetamperset)
{
  #if 0
	dryicetamperset->dryice_tamperpin0_config = EXTTAMPER_DYNAMIC_OUT;
	dryicetamperset->dryice_tamperpin1_config = EXTTAMPER_DYNAMIC_IN;
	dryicetamperset->dryice_tamperpin2_config = EXTTAMPER_STATIC_HIGH;//EXTTAMPER_STATIC_LOW;
	dryicetamperset->dryice_tamperpin3_config = EXTTAMPER_STATIC_LOW;//EXTTAMPER_STATIC_HIGH;
  #else
  	dryicetamperset->dryice_tamperpin0_config = EXTTAMPER_STATIC_HIGH;
	dryicetamperset->dryice_tamperpin1_config = EXTTAMPER_STATIC_HIGH;
	dryicetamperset->dryice_tamperpin2_config = EXTTAMPER_STATIC_LOW;
	dryicetamperset->dryice_tamperpin3_config = EXTTAMPER_STATIC_LOW;
  #endif
    dryicetamperset->dryice_tamperpin4_config = EXTTAMPER_STATIC_HIGH;//EXTTAMPER_DYNAMIC_OUT;
	dryicetamperset->dryice_tamperpin5_config = EXTTAMPER_STATIC_HIGH;//EXTTAMPER_DYNAMIC_IN;
	dryicetamperset->dryice_tamperpin6_config = EXTTAMPER_STATIC_LOW;//EXTTAMPER_STATIC_LOW;
	dryicetamperset->dryice_tamperpin7_config = EXTTAMPER_STATIC_LOW;//EXTTAMPER_STATIC_HIGH;
	
}
/*******************************************************************
Author:   
Function Name:void pcitamper_manage_tamperset(void)
Function Purpose:dryice tamper config
Input Paramters: 
Output Paramters:N/A
Return Value:      N/A
Remark: N/A
Modify: N/A
********************************************************************/
void pcitamper_manage_tamperset(u8 type)
{
    DRYICETAMPERCONFIG dryicetamperset;
	
	memset(&dryicetamperset,0,sizeof(DRYICETAMPERCONFIG));
//	dev_dryice_open(&dryicetamperset,type);

    pcitamper_manage_tampercfg(&dryicetamperset);

#if 0	
	dryicetamperset.dryice_tamperpin0 = 1;    //��tamper0
	dryicetamperset.dryice_tamperpin0_config = EXTTAMPER_DYNAMIC_OUT;
	dryicetamperset.dryice_tamperpin1 = 1;    //��tamper1
	dryicetamperset.dryice_tamperpin1_config = EXTTAMPER_DYNAMIC_IN;

    
	dryicetamperset.dryice_tamperpin2 = 1;    //��tamper2
	dryicetamperset.dryice_tamperpin2_config = EXTTAMPER_STATIC_HIGH; //EXTTAMPER_STATIC_LOW;

	dryicetamperset.dryice_tamperpin3 = 1;    //��tamper3  TH0
	dryicetamperset.dryice_tamperpin3_config = EXTTAMPER_STATIC_LOW;//EXTTAMPER_STATIC_HIGH;
	

    dryicetamperset.dryice_tamperpin4 = 0;//1;    //��tamper4
	dryicetamperset.dryice_tamperpin4_config = EXTTAMPER_DYNAMIC_OUT;
	dryicetamperset.dryice_tamperpin5 = 0;//1;    //��tamper5
	dryicetamperset.dryice_tamperpin5_config = EXTTAMPER_DYNAMIC_IN;

    
	dryicetamperset.dryice_tamperpin6 = 0;//1;    //��tamper6
	dryicetamperset.dryice_tamperpin6_config = EXTTAMPER_STATIC_LOW;

	dryicetamperset.dryice_tamperpin7 = 0;//1;    //��tamper7  TH1
	dryicetamperset.dryice_tamperpin7_config = EXTTAMPER_STATIC_HIGH;
	
#endif	

#ifndef PCI_TAMPLE_DISABLE //�رմ���״̬ pengxuebin,20171116   
	dryicetamperset.dryice_tamperpin0 = 1;    //��tamper0 
	dryicetamperset.dryice_tamperpin1 = 1;    //��tamper1   
	dryicetamperset.dryice_tamperpin2 = 1;    //��tamper2 
	dryicetamperset.dryice_tamperpin3 = 1;    //��tamper3 

  #if 0
	dryicetamperset.dryice_tamperxtal32k = 0;   // 32Kʱ��Ҫ����
	
    dryicetamperset.dryice_tampervgdetected = 1;
	dryicetamperset.dryice_tampervte = 1;

	dryicetamperset.dryice_tampertte = 1;
	dryicetamperset.dryice_tampermesh = 1;
  #endif
#endif    
	
	dev_dryice_open(&dryicetamperset,type);
	
}


/*******************************************************************
Author:   
Function Name:void pcitamper_manage_selfcheck_securitylocked(u8 type)
Function Purpose:self check tamper detected
Input Paramters: N/A
Output Paramters:N/A
Return Value:      N/A
Remark: N/A
Modify: N/A
********************************************************************/
void pcitamper_manage_selfcheck_securitylocked(u8 type)
{
	//pcikeymanage_initkeydata();
	//gPCITamperManage.gTamperCurrentValue = 0xFFFFFFFF;
	gPCITamperManage.gTamperDetectValue |= DEFAULT_MMKTAMPERVALUE;
}


/*******************************************************************
Author:   
Function Name:s32 pcitamper_manage_dryice_timerhandler(void)
Function Purpose:dryice timer handler 
Input Paramters: 
Output Paramters:N/A
Return Value:      N/A
Remark: N/A
Modify: N/A
********************************************************************/
s32 pcitamper_manage_dryice_timerhandler(void)
{
   #if 0
    u32 tamperstatus;
	
	
	//DDEBUG("\r\npcitamper_manage_dryice_timerhandler %d\r\n",gPCITamperManage.gCheckDryiceStatusflag);  //sxl?
	
	
    if(gPCITamperManage.gCheckDryiceStatusflag)
    {
		if((gPCITamperManage.gCheckDryiceStatusTimer%10) == 0)
		{
		    tamperstatus = dev_dryice_gettamperstatusonly();
			if(tamperstatus&gPCITamperManage.gTAMPERENABLEVALUE)
			{
				if(gPCITamperManage.gCheckDryiceStatusTimer == 0)
				{
					gPCITamperManage.gTamperCurrentValue = tamperstatus;
				}
				else
				{
					DRYICE_ClearStatusFlags(DRY,kDRYICE_StatusAll);
				}
				
			}
			else
			{
				gPCITamperManage.gCheckDryiceStatusflag = 0;
				pcicorecallfunc.pci_dev_timer_user_close(&gPCITamperManage.gCheckDryiceStatusTimer);
				//disable dryice interrupt
		        pcitamper_manage_tamperset();
				return 1;
				
			} 

		}
    }
	#endif

	return 0;

	
}
/*******************************************************************
Author:   
Function Name:   s32 pcitamper_manage_dryice_isrhandler(void)
Function Purpose:dryice interrupt handler
Input Paramters: N/A
Output Paramters:N/A
Return Value:      N/A
Remark: N/A
Modify: N/A
********************************************************************/
s32 pcitamper_manage_dryice_isrhandler(u32 drysrreg)
{
    u32 i;
    #ifdef SXL_DEBUG
//	dev_debug_printf("%s(%d):drysrreg = %08x %08x\r\n", __FUNCTION__, __LINE__,drysrreg,gPCITamperManage.gTAMPERENABLEVALUE);
	#endif
    i = drysrreg&gPCITamperManage.gTAMPERENABLEVALUE;
	if(i)  //tamper detected
	{
//dev_debug_printf("%s(%d):\r\n", __FUNCTION__, __LINE__);
		gPCITamperManage.gTamperDetectValue = i;//drysrreg;
        gPCITamperManage.gTamperCurrentValue = i;//drysrreg;
        g_pcitimerid = dev_user_gettimeID();  
        g_pcitampervalue = i;//drysrreg;
        g_pcitampercnt = 0;
		//pcitamper_manage_savetamper(i);  //sxl2018  不放到中断里处理保存触发信息
	}
	return 0;
	
}

/*******************************************************************
Author:   
Function Name:   void pcitamper_manage_tampertask(void)
Function Purpose:check if tamper detected
Input Paramters: 
Output Paramters:N/A
Return Value:      N/A
Remark: N/A
Modify: N/A  ��δʹ��
********************************************************************/
void pcitamper_manage_tampertask(void)
{
    #if 0
	u32 ticks;
	
    //check if new tamper detected
    //if(gPCITamperManage.gSecurityStatus != SYSTEMERROR_NOTSECURITY&&(gPCITamperManage.gTamperCurrentValue&gPCITamperManage.gTAMPERENABLEVALUE))
    {
        #ifdef SXL_DEBUG
		DDEBUG("\r\n%s new tamper detected\r\n",gPCITamperManage.gTamperCurrentValue);
		#endif
		pcitamper_manage_savetamper(gPCITamperManage.gTamperCurrentValue);
    }
	#endif
	
	
	
}


void pci_core_initvalue(void)
{
	
	gPciInitState = -1;

	
	//memset(&pcilibmemcalldef,0,sizeof(PCI_CORE_CALLFUNCDEFINE));
	#if 1
	memset(&gDukptkInfo,0,sizeof(DUKPTK_INFO));
	appkeylockbits = 0;
	memset(&gPinKeyUseInternal,0,sizeof(PINKEYUSEINTERNAL));
	memset(&gSecurityAuthInfo,0,sizeof(SECURITYAUTHENTICATEINFO));
	timerstartflag = 0;
	appkeylocktimer = 0;
	memset(&gPciManagerInfo,0,sizeof(PCIMANAGE));
    memset(&gPCITamperManage,0,sizeof(PCITAMPERMANAGE));
	gDownLoadKeyErr = 0;
	
	gCommData = NULL;
	#endif
    
    
	pownonflag = 1;
	
}



u32 pci_core_readtamperstatus(void)  //sxl?2017 ��ȡ����״̬
{
	u8 i;
    //u32 dryicestatus;
    s32 ret;
	
    //�Ȳ鿴��ǰ�Ĵ���״̬
    gPCITamperManage.gTamperDetectValue = 0;
    for(i = 0;i < 4;i++)
    {
    	gPCITamperManage.gTamperDetectValue = (gPCITamperManage.gTamperDetectValue<<8) + gPciTamperManageFileInfo.tamperreg[i];
    }
	
	
	#ifdef PCI_DEBUG
	dev_debug_printf("\r\npci_core_readtamperstatus12:%08x %08x\r\n",gPCITamperManage.gTamperDetectValue,gPCITamperManage.gTAMPERENABLEVALUE);
	#endif
	
    if(gPCITamperManage.gTamperDetectValue&gPCITamperManage.gTAMPERENABLEVALUE) //sxl?20180713查看密钥是否丢失
    {
        gPCITamperManage.gTamperCurrentValue = dev_dryice_poweronreadstatus();   //�ԼĴ�������һ�����ã��Ա����ʹ��

		#ifdef PCI_DEBUG
		dev_debug_printf("\r\npoweron dryice reg status1:%08x \r\n",gPCITamperManage.gTamperCurrentValue);
		#endif
		
		return gPCITamperManage.gTamperDetectValue;
		
    }


	
	
	//�鿴���ڴ���״̬
	#if 1   // ���ﲻ�ܶ�����Ҫ���ú���ܶ�
	gPCITamperManage.gTamperCurrentValue = dev_dryice_poweronreadstatus();
	#ifdef PCI_DEBUG
	dev_debug_printf("\r\npoweron dryice reg status:%08x %08x\r\n",gPCITamperManage.gTamperCurrentValue,gPCITamperManage.gTAMPERENABLEVALUE);
	#endif
	if(gPCITamperManage.gTamperCurrentValue&gPCITamperManage.gTAMPERENABLEVALUE)
	{
	    gPCITamperManage.gTamperDetectValue = 0;
	    ret = pcitampermanage_poweroncheckmmk(1);// != PCI_PROCESSCMD_SUCCESS)
	    #ifdef PCI_DEBUG
		dev_debug_printf("\r\n read mmk ret = %d\r\n",ret);
		#endif
        if(ret != PCI_PROCESSCMD_SUCCESS)
        {
			gPCITamperManage.gTamperDetectValue = gPCITamperManage.gTamperCurrentValue;
        }
	}
	else
	{
		
		gPCITamperManage.gTamperDetectValue = 0;
		
	}
	#endif
	
	return gPCITamperManage.gTamperDetectValue;
	
}


/*******************************************************************
Author:   
Function Name: void pci_core_open (void)
Function Purpose:open pci
Input Paramters: 
Output Paramters:N/A
Return Value:      N/A
Remark: N/A
Modify: N/A
********************************************************************/
PCI_FIRMWARE_API *pcicore;
extern const PCI_FIRMWARE_API constpcicore;
void pci_core_open (void)
{
	u32 regdata;
	s32 ret;
	//if(gPciInitState < 0)
	{
	    
		//u8 *data;
		u8 i;

		pci_core_initvalue();
		
		pcicore = (PCI_FIRMWARE_API *)&constpcicore;
		
		#ifndef EMVTEST_CORE
		#ifdef PCI_DEBUG
		dev_debug_printf("\r\npoweron check tamper\r\n");
		#endif

		
		//��ȡtamper�����ļ�
		pcitampermanage_initial();
		dev_dryice_poweron();   //dryice ģ���ϵ�

	
		
		//���ô� dryice
		if(pci_core_checkinitialflag()) // check if tamper open
		{
			#ifdef PCI_DEBUG
	        dev_debug_printf("%s(%d):poweron dryice set \r\n", __FUNCTION__, __LINE__);
			#endif

			if(gPciTamperManageFileInfo.tamperopenflag)
			{
				if(pci_core_readtamperstatus())
				{
					regdata = gPCITamperManage.gTamperDetectValue;
				}
				else
				{
					regdata = 0;
				}
				#ifdef PCI_DEBUG
				dev_debug_printf("\r\n poweron dryice set regdata = %08x\r\n",regdata);
				#endif
				pcitamper_manage_savetamper(regdata);
				if(pcitamper_manage_gettamperstatus() == 0)   // tamper not detected
				{
					#ifdef SXL_DEBUG
					dev_debug_printf("\r\npcitampermanage_poweroncheckmmk\r\n");
					#endif
					//�������dryice �Ƿ񱻴۸�
					//if(pcitampermanage_poweroncheckmmk() != PCI_PROCESSCMD_SUCCESS)
					ret = pcitampermanage_poweroncheckmmk(0);// != PCI_PROCESSCMD_SUCCESS)
                    if(ret != PCI_PROCESSCMD_SUCCESS)
					{
						regdata = DEFAULT_MMKTAMPERVALUE;
						if(ret == PCI_READMMK_ALLZERO)
						{
							regdata |= 0x40000000;
						}

						if(gBpkResetFlag == 1)
						{
							regdata |= 0x20000000;
						}
						
						
						pcitamper_manage_savetamper(regdata);
						
					}
					
				}
			}
			else
			{
				
			    #ifdef PCI_DEBUG
                dev_debug_printf("\r\ntamper first open\r\n");
				#endif
				
				
				//��һ�ο���tamper  
				gPciTamperManageFileInfo.tamperopenflag = 1;
				pcitampermanage_filesave(&gPciTamperManageFileInfo);
				
				
			}
			if(pcitamper_manage_gettamperstatus() == 0)   // tamper not detected
			{
				#ifdef SXL_DEBUG
				dev_debug_printf("%s(%d):pcitamper_manage_tamperset\r\n", __FUNCTION__, __LINE__);
				#endif
//				pcitamper_manage_tamperset(DRYICE_SET_NORMAL);
				pci_core_initialsecuritystatus();        // check saved tamper status
				
			}
			else
			{
//				pcitamper_manage_tamperset(DRYICE_SET_NORMAL);
				#ifdef PCI_DEBUG
				dev_debug_printf("\r\n tamper detected \r\n");
				#endif
			//	 pci_core_close();  //�ȹش������  //sxl20170918
			}
			pcitamper_manage_tamperset(DRYICE_SET_NORMAL);
			
		}
		else  //ֻ����ļ���û�д���
		{
		    #ifdef PCI_DEBUG
			dev_debug_printf("\r\npci_core_initialsecuritystatus\r\n");
			#endif
            //����ҲҪ���ô���
            pci_core_close();
			
			pci_core_initialsecuritystatus();
		}

		
		//��ʼ��������Ҫ��bpk_por
        pcitamper_manage_clrbpkporstatus();
		
        #ifdef PCI_DEBUG
		dev_debug_printf("\r\n pci core open end \r\n");
		#endif
		#endif
		gPciInitState = 0;
		
		
	}
	
	
}


void pci_core_close(void)
{
	DRYICETAMPERCONFIG dryicetamperset;
	
	memset(&dryicetamperset,0,sizeof(DRYICETAMPERCONFIG));
    pcitamper_manage_tampercfg(&dryicetamperset);
	dev_dryice_open(&dryicetamperset,0);  //�����д���
	
}

/*
int main(void)
{
	
	pci_core_open();
	pcitask_protocol_communicationinit();
	pcitask_protocol_keydownload();
	pcitask_protocol_communicationexit();
	
}
*/


/*******************************************************************
Author:   
Function Name: u32 pcitamper_manage_gettamperstatus(void)
Function Purpose:get tamper status register value
Input Paramters: 
Output Paramters:N/A
Return Value:      tamper status register value
Remark: N/A
Modify: N/A
********************************************************************/
u32 pcitamper_manage_gettamperstatus(void)
{
	if(gPCITamperManage.gTAMPERENABLEVALUE&gPCITamperManage.gTamperDetectValue)
	{
	    //���津��״̬
	    pcitamper_manage_savetamper(gPCITamperManage.gTamperDetectValue);
		return 1;
		
	}
    
	
	return 0;
	
}



u32 pcitamper_manage_readtamperstatus(u8 *unlocktimes)
{
	u32 currenttamperstatus;
	
    memset(unlocktimes,0,12);
#ifndef PCI_TAMPLE_DISABLE//�رմ���״̬ pengxuebin,20171116   
	if(pcitamper_manage_gettamperstatus())
	{
//	    pci_core_close();  //sxl20170918
		
		//���Ӵ����Ĵ���״̬
		unlocktimes[0] = (u8)(gPCITamperManage.gTamperDetectValue>>24);
		unlocktimes[1] = (u8)(gPCITamperManage.gTamperDetectValue>>16);
		unlocktimes[2] = (u8)(gPCITamperManage.gTamperDetectValue>>8);
		unlocktimes[3] = (u8)(gPCITamperManage.gTamperDetectValue);
		currenttamperstatus = pcitamper_manage_getcurrenttamperstatus();
		unlocktimes[4] = (u8)(currenttamperstatus>>24);
		unlocktimes[5] = (u8)(currenttamperstatus>>16);
		unlocktimes[6] = (u8)(currenttamperstatus>>8);
		unlocktimes[7] = (u8)(currenttamperstatus);

		unlocktimes[11] = pcitamper_manage_readunlocktimes();
		#ifdef SXL_DEBUG
	    dev_debug_printf("\r\nread unlocktimes11 = %d\r\n",unlocktimes[11]);
		#endif
		return 1;
		
	}
#endif    

	return 0;
	
}


u8 pcitamper_manage_readunlocktimes(void)
{
	
    if(gPciTamperManageFileInfo.unlocktimes > 99)
    {
    	
    	gPciTamperManageFileInfo.unlocktimes = 0;
		pcitampermanage_filesave(&gPciTamperManageFileInfo);
		
    }
	
	return gPciTamperManageFileInfo.unlocktimes;
	
	
}




u32 pcitamper_manage_saveunlocktimes(u8 unlocktimes)
{
	
	s32 ret;
    
	#ifdef SXL_DEBUG
	dev_debug_printf("\r\nsave unlock times111 ret = %d\r\n",unlocktimes);
	#endif
	
	if(unlocktimes > 99)
    {
    	unlocktimes = 0;
    }
	gPciTamperManageFileInfo.unlocktimes = unlocktimes;
	pcitampermanage_filesave(&gPciTamperManageFileInfo);
	
	#ifdef SXL_DEBUG
	//dev_debug_printf("\r\nsave unlock times ret = %d\r\n",ret);
	#endif
	
	return 0;
	
}


u32 pcitamper_manage_unlock(void)
{
	
    u8 unlocktimes;
	s32 ret;
	u8 mmkbak[50];
	
	if(gPCITamperManage.gTamperDetectValue&gPCITamperManage.gTAMPERENABLEVALUE)
	{
	    //���³�ʼ��������Կ,Ҫ���Ѵ���״̬�´���
        
		pci_core_close();  //�ȹش���
		//pcikeymanage_initkeydata();
		ret = pcitamper_manage_mmkbakrecover(mmkbak);
		#ifdef PCI_DEBUG
		dev_debug_printf("\r\npcitampermanage_poweroncheckmmk = %d\r\n",ret);
		#endif
		if(ret != PCI_PROCESSCMD_SUCCESS&&memcmp(gPciTamperManageFileInfo.mmkdata,PCIMMKBPKFILEFLAG,4))
		{
			 pcitampermanage_regeneratemmkelrck_initialvalue();  //���³�ʼ��
		}
		
		//pcitampermanage_powerondelmmkbpkfile();
		gPCITamperManage.gTamperDetectValue = 0;
		memset(gPciTamperManageFileInfo.tamperreg,0,4);
        pcitampermanage_filesave(&gPciTamperManageFileInfo);
		
		//save unlock times
		unlocktimes = pcitamper_manage_readunlocktimes();
		unlocktimes++;
		if(unlocktimes > 99)
		{
			unlocktimes = 0;
		}

		#ifdef SXL_DEBUG
	    dev_debug_printf("\r\nsave unlocktimes = %d\r\n",unlocktimes);
		#endif
		
		pcitamper_manage_saveunlocktimes(unlocktimes);

		gPCITamperManage.gTamperDetectValue = 0;
		
	}

	
	//������������ֹAPû������SP
	//dev_smc_systemsoftreset();
    //pci_core_open();  //��������
	
	return 0;
	
}


u32 getcurrenttamperstatus(void)
{
    u32 i;
    if(dev_user_querrytimer(g_pcitimerid, 1000))
    {
        g_pcitimerid = dev_user_gettimeID();
        i = SENSOR_GetITStatusReg()&gPCITamperManage.gTAMPERENABLEVALUE;
      #if 0  
        if(i!=gPCITamperManage.gTamperCurrentValue)
        { 
            if(i==g_pcitampervalue)
            {
                g_pcitampercnt ++;
                
                if(g_pcitampercnt>=3)
                {
                    gPCITamperManage.gTamperCurrentValue=g_pcitampervalue;
                    g_pcitampercnt = 0;
                }
            }
            else
            {
                g_pcitampervalue = i;
                g_pcitampercnt = 0;
            }
        }
        else
        {
            g_pcitampercnt = 0;
            g_pcitampervalue = i;
        }
      #else
        gPCITamperManage.gTamperCurrentValue = i;
      #endif        
//dev_debug_printf("drysrreg = %08x\r\n", i);
    }
    return gPCITamperManage.gTamperCurrentValue;
}


u32 pcitamper_manage_getcurrenttamperstatus(void)
{
	
	u32 drysrreg = 0;
#ifndef PCI_TAMPLE_DISABLE //�رմ���״̬ pengxuebin,20171116   
    //�Ѿ��������Ĳ��������ô���������ǰ����״̬
	if(gPCITamperManage.gTamperDetectValue&gPCITamperManage.gTAMPERENABLEVALUE)
	{
	    //ֻ�ܻ�ȡ����ʱ�Ĵ���״̬
		drysrreg = getcurrenttamperstatus();
        //drysrreg = gPCITamperManage.gTamperCurrentValue;
		#ifdef PCI_DEBUG
		dev_debug_printf("\r\ndrysrreg = %08x\r\n",drysrreg);
		#endif
		//����ʱ���ȡ״̬����0
	    /*
		drysrreg = dev_dryice_gettamperstatusonly();
		#ifdef PCI_DEBUG
		dev_debug_printf("\r\ndrysrreg = %08x\r\n",drysrreg);
		#endif
		if((drysrreg&gPCITamperManage.gTAMPERENABLEVALUE) == 0)  //tamper detected
		{
			drysrreg = 0;
		}
		*/
	}
#endif    
	return drysrreg;
	
	
}



u32 pcitamper_manage_dryiceopen(u8 dryicetype)
{
	s32 curdryicetype;

    #ifdef SXL_DEBUG
    dev_debug_printf("\r\npcitamper_manage_dryiceopen dryicetype = %d\r\n",dryicetype);
	#endif
	
	
	curdryicetype = pci_core_checkinitialflag();
	if(curdryicetype == dryicetype)
	{
		return 0;
	}
	
	
	if(dryicetype > 2||(curdryicetype == 1&&dryicetype == 2))
	{
		return 1;
	}

	#ifdef SXL_DEBUG
    dev_debug_printf("\r\npcitamper_manage_dryiceopen dryicetype = %d %d\r\n",dryicetype,curdryicetype);
	#endif

	
	
	//���õ�ǰ����״̬
    if(curdryicetype != dryicetype)
    {
    	if(pci_core_saveinitialflag(dryicetype))
    	{
    		
    	    #ifdef PCI_DEBUG
			dev_debug_printf("\r\nsave pci flag error\r\n");
			#endif
    		return 1;
			
    	}
    }
	
	
	//���dryice �Ƿ��Ѿ��򿪣�δ�򿪵Ļ������¿�dryice
	//����������������һ������
	if(dryicetype)
	{
		
    	#ifdef SXL_DEBUG
		dev_debug_printf("\r\npci core open\r\n");
		#endif
		pci_core_open();
		
	}
    else
    {
    	
    	//Ϊ0�Ļ���������
    	pci_core_close();
    }
	
	return 0;
	
	
}


u32 pcitamper_manage_getdryiceconfig(void)
{
	return pci_core_checkinitialflag();
}




void pcitest(void)
{
	u8 refreshflag = 1;
    u8 data;
    s32 ret;
	u8 key[16],readkey[16],keylen;
    u8 tamperstatus[16];
	u8 rand[32];
    
	memset(key,0x15,sizeof(key));
	while(1)
	{
		if(refreshflag)
		{
			
			refreshflag = 0;
			dev_debug_printf("========dryice test========\r\n");
			dev_debug_printf("1.dryice system initial 2.write key test\r\n");
			dev_debug_printf("3.read key test         4.MMK regenerate\r\n");
            dev_debug_printf("5.dryice key rewrite    6. enable tamper\r\n");
			dev_debug_printf("7.disable tamper        8. mmkkeyregenerate\r\n");
			dev_debug_printf("9.tamper unlock         0. read tamper status\r\n");
			
		}
		
		
		if(dev_com_read(0,&data,1) == 1)
    	{
    		
            if(data == 0x30)
            {
            	memset(tamperstatus,0,sizeof(tamperstatus));
				ret = pcitamper_manage_readtamperstatus(tamperstatus);
				if(ret)
				{
					dev_debug_printf("\r\ntamper detected!\r\n");
					for(ret = 0;ret < 12;ret++)
					{
						dev_debug_printf("%02x ",tamperstatus[ret]);
					}
					dev_debug_printf("\r\n");
				}
				else
				{
					dev_debug_printf("\r\ntamper not detected!\r\n");
				}
					
            }
			else if(data == 0x31)
    		{
    			
    			pcitampermanage_file_initial(&gPciTamperManageFileInfo);
				dev_debug_printf("\r\nbattery need pull out!\r\n");
				
    		}
			else if(data == 0x32)
    		{
    			ret = pcikeymanage_saveappkey(0,TYPE_TMK,0,16,key);
				if(ret == PCI_PROCESSCMD_SUCCESS)
				{
					dev_debug_printf("\r\nsave app key success\r\n");
				}
				else
				{
					dev_debug_printf("\r\nsave app key fail\r\n");
				}
    		}
			else if(data == 0x33)
    		{
    			ret = pcikeymanage_readappkey(0,TYPE_TMK,0,&keylen,readkey);
				if(ret != PCI_PROCESSCMD_SUCCESS||keylen != 16)
				{
					dev_debug_printf("\r\nread app key fail 1\r\n");
				}
				else
				{
					if(memcmp(readkey,key,16) == 0)
					{
						dev_debug_printf("\r\nread app key success\r\n");
					}
					else
					{
						dev_debug_printf("\r\nread app key fail 2\r\n");
					}
					
				}
    		}
			else if(data == 0x34)
    		{
    			memset(gPciTamperManageFileInfo.mmkdata,0,sizeof(gPciTamperManageFileInfo.mmkdata));
				pcitampermanage_filesave(&gPciTamperManageFileInfo);
				dev_debug_printf("\r\nmmk bak reset\r\n");
    		}
            else if(data == 0x35)
    		{
    			dev_trng_read(rand,32);
    			dev_dryice_writesecurekey(rand,32);
				dev_debug_printf("\r\ndryice key reset\r\n");
    		}
			else if(data == 0x36)
    		{
    			pcitamper_manage_dryiceopen(2);
				dev_debug_printf("\r\ntamper open\r\n");
    		}
            else if(data == 0x37)
    		{
    			pcitamper_manage_dryiceopen(0);
				dev_debug_printf("\r\ntamper close\r\n");
    		}
			else if(data == 0x38)
			{
				memset(gPciTamperManageFileInfo.mmkdata,0,sizeof(gPciTamperManageFileInfo.mmkdata));
				pcitampermanage_filesave(&gPciTamperManageFileInfo);
				dev_debug_printf("\r\nmmk bak reset\r\n");
				
			}
			else if(data == 0x39)
			{
				
				dev_debug_printf("\r\n tamper unlock\r\n");
				pcitamper_manage_unlock();
				dev_debug_printf("\r\nsystem need reset\r\n");
				dev_trng_read(rand,32);
    			dev_dryice_writesecurekey(rand,32);
				dev_debug_printf("\r\nmmk bak reset\r\n");
				
			}
 			else if(data == 0x0e)
 			{
 				break;
 			}
			
			dev_user_delay_ms(1000);
			refreshflag = 1;
			

			
		}


		
	}
	
	
}
