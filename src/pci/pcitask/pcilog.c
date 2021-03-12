

#include "pciglobal.h"






void pcilog_record(u32 command,u8 *data)
{
    #ifdef SAVEPCILOG
	gPciKeyLogInfo.step = command;
	if(command == AUTHENTICATE_REQUEST)
	{
		memset(gPciKeyLogInfo.issuersn,0,sizeof(gPciKeyLogInfo.issuersn));
		memcpy(gPciKeyLogInfo.issuersn,&data[1],48);
	}
    else if(command == AUTHENTICATE_STEP3)
    {
		gPciKeyLogInfo.result = data[0];
    }
	else if(command == LOAD_APPKEY_REQUEST)
    {
        
		gPciKeyLogInfo.groupindex = data[0];
		gPciKeyLogInfo.keytype = data[1];
		gPciKeyLogInfo.keyindex= data[2];
    }
	else if(command == LOAD_APPDUKPTKEY_REQUEST)
	{
		gPciKeyLogInfo.groupindex = data[0];
		gPciKeyLogInfo.keyindex= data[3];  
	}
	else if(command == COMMAND_DELETEGROUPALLKEY)
	{
		gPciKeyLogInfo.groupindex = data[0];
	}
	else if(command == COMAMND_SETGROUPKEYAPPNAME)
	{
		gPciKeyLogInfo.groupindex = data[0];
	}
	else if(command == APPKEY_SETPINK)
	{
	    #ifdef SXL_DEBUG
        DDEBUG("\r\nAPPKEY_SET:%02x %02x %02x %02x \r\n",data[1],data[2],data[3],data[4]);
		#endif
		gPciKeyLogInfo.groupindex = data[1];
		gPciKeyLogInfo.mainkeyindex = data[2];
		gPciKeyLogInfo.keytype = data[3];
		gPciKeyLogInfo.keyindex= data[4];
	}
	else if(command == APPKEY_SETMACK)
	{
		gPciKeyLogInfo.groupindex = data[1];
		gPciKeyLogInfo.mainkeyindex = data[2];
		gPciKeyLogInfo.keytype = data[3];
		gPciKeyLogInfo.keyindex= data[4];
	}
	else if(command == COMMAND_LOADMAGKEY)
	{
	     #ifdef SXL_DEBUG
         DDEBUG("\r\nlog load mag key:%02x\r\n",data[2]);
		 #endif
		 //gPciKeyLogInfo.groupindex = 0xff; //sxl?  暂时不处理
		 //gPciKeyLogInfo.keytype = KEYTYPE_MAGKEY;
		 //gPciKeyLogInfo.keyindex= data[2];
		 
	}
    #endif
	
	
}

/*******************************************************************
Author:   sxl
Function Name:    void pcilog_downkeyerrprompttransfer(s32 errtype,u8 *promptdata)
Function Purpose: prompt down key error
Input Paramters:  errtype - error type
Output Paramters:promptdata - prompt data
Return Value:      N/A
Remark: N/A      
Modify: N/A 		//20140327
********************************************************************/
void pcilog_downkeyerrprompttransfer(s32 errtype,s8 *promptdata)
{
    #ifdef SAVEPCILOG
	
	if(errtype == PCI_AUTHENKEYISSUER_ERROR)
	{
		strcpy(promptdata,PROMPT_DOWNKEY_CERTIFICATEERR);
	}
	else if(errtype == PCI_AUTHENAPPKEYLOCK_ERR)
	{
		strcpy(promptdata,PROMPT_DOWNKEY_KEYLOCKED);
	}
	else if(errtype == PCI_PKUNFIND_ERR)
	{
		strcpy(promptdata,PROMPT_DOWNKEY_PKNOTFIND);
	}
	else if(errtype == PCI_AUTHENPTK_ERR)
	{
		strcpy(promptdata,PROMPT_DOWNKEYPOSPKERR);
	}
	else if(errtype == KM_APP_KEY_WRITE_ERROR)
	{
		strcpy(promptdata,PROMPT_DOWNKEY_SAVEERR);
	}
	else if(errtype == KM_APP_KEY_GROUPINDEX_ERR)
	{
		strcpy(promptdata,PROMPT_DOWNKEY_GROUPINDEX_ERR);
	}
	else if(errtype == KM_APP_KEY_INDEX_ERROR)
	{
		strcpy(promptdata,PROMPT_DOWNKEY_INDEX_ERR);
	}
	else if(errtype == KM_APP_KEY_TYPE_ERROR)
	{
		strcpy(promptdata,PROMPT_DOWNKEY_TYPE_ERR);
	}
	else if(errtype == KM_APP_KEY_LEN_ERROR)
	{	
		strcpy(promptdata,PROMPT_DOWNKEY_LENGTHERR);
	}
	else if(errtype == KM_AUTH_ERROR)
	{
		strcpy(promptdata,PROMPT_DOWNKEY_AUTHENFAIL);
	}
	else if(errtype == KM_APPKEY_NOTAUTH)
	{
		strcpy(promptdata,PROMPT_DOWNKEY_UNAUTHENTICATE);
	}
	else if(errtype == KM_AUTHENKEYISSUER_ERROR)
	{
		strcpy(promptdata,PROMPT_AUTHENKEYISSUER_ERROR);
	}
	else if(errtype == KM_KEYFILENOTUSE)
	{
		strcpy(promptdata,PROMPT_KEYFILENOTUSE);
	}
	else
	{
		strcpy(promptdata,PROMPT_DOWNKEY_OTHERERROR);
	}
	
	#endif
}


/*******************************************************************
Author:   sxl
Function Name:    void pcilog_getmanagekeycaption(u8 keytype,u8 *caption)
Function Purpose: get manage key caption
Input Paramters:  N/A
Output Paramters:N/A
Return Value:      N/A
Remark: N/A      
Modify: N/A 		1130  //20140327
********************************************************************/
void pcilog_getmanagekeycaption(u8 keytype,s8 *caption)
{
    #ifdef SAVEPCILOG
	
	s8 dispdata[20];

	#ifdef SXL_DEBUG
    dev_debug_printf("\r\nkeytype = %02x\r\n",keytype);
	#endif

    memset(dispdata,0,sizeof(dispdata));
	if(keytype == TYPE_BPK)
	{
		strcpy(dispdata,"BPK");
	}
	else if(keytype == KEYTYPE_PIN_MK)
	{
		strcpy(dispdata,"TK_PIN");
	}
	else if(keytype == KEYTYPE_MAC_MK)
	{
		strcpy(dispdata,"TK_MAC");
	}
	else if(keytype == KEYTYPE_FIXEDK)
	{
		strcpy(dispdata,"FIXEDK");
	}
	else if(keytype == KEYTYPE_MAGKEY)
	{
		strcpy(dispdata,"MAGKEY");
	}
	strcpy(caption,dispdata);

	#endif
}


u8 pcilog_AppendExternalFile(u8 *filename, u8 *buf,u32 length)  //
{
	
	s32 filelen;
	
    filelen = pcicorecallfunc.pci_kern_access_file((const s8 *)filename);
	if(filelen >= 0)
	{
		filelen = pcicorecallfunc.pci_kern_insert_file((const s8 *)filename,buf,length,filelen);
	}
    else
    {
    	filelen =pcicorecallfunc.pci_kern_write_file((const s8 *)filename,buf,length);
    }
    
    if(filelen != length)
    {
    	return 1;
    }
    else
    {
    	return 0;
    }
}

/*******************************************************************
Author:   sxl
Function Name:    void pcilog_tidylogfile(void)
Function Purpose: tidy log file
Input Paramters:N/A
Output Paramters:N/A
Return Value:      N/A
Remark: N/A      
Modify: N/A 		1130   //20140327
********************************************************************/
void pcilog_tidylogfile(void)
{
	
	s8 *tmp;   //sxl?  //[EXTBlockLen]
	s8 *str;
	s8 tmp1[256];
	u32 len,start,filelen,tmplen,oplen;  //start1,
	s32 ret;
	u8 Re,err = 0;

    filelen = pcicorecallfunc.pci_kern_access_file(PCIHLLOGFILE);	
	if(MAXLOGFILELEN > (filelen+512))
	{
		return;
	}

    tmp = (s8 *)pcicorecallfunc.mem_malloc(4096);
	memset(tmp,0,4096);
	start = MAXLOGFILELEN>>1;
    len = 1024;
	ret = pcicorecallfunc.pci_kern_read_file((const char *)PCIHLLOGFILE,(u8 *)tmp,start,len);
	if(ret != len)
	{
	    pcicorecallfunc.mem_free(tmp);
		return;
	}
	
	
    memset(tmp1,0,sizeof(tmp1));
	strcpy(tmp1,"**** LOG TIME:");
	str = strstr(tmp,tmp1);
	if(str == NULL)
	{
	    pcicorecallfunc.pci_kern_delete_file(PCIHLLOGFILE);
	}
	else
	{
	    pcicorecallfunc.pci_kern_delete_file(TMPPCIHLLOGFILE);  //delete  tmp  log  file  first
		
        start = (MAXLOGFILELEN>>1)+str-tmp;
        len = filelen - start;
		filelen = len;
        //start1 = 0;
		
		for(len = 0;len<filelen;)
		{
			if((len+4096)<filelen)
			{
				tmplen = 4096;
			}
			else
			{
				tmplen = filelen-len;
			}
			
            Re = pcicorecallfunc.pci_kern_read_file((const char *)PCIHLLOGFILE,(u8 *)tmp,start,tmplen);
			if(Re != tmplen)
			{
				#ifdef SXL_DEBUG
                DDEBUG("\r\nPCIPOS_tidylogfile read fail\r\n");
				#endif
				err = 1;
				break;
			}
			Re = pcilog_AppendExternalFile((u8 *)TMPPCIHLLOGFILE,(u8 *)tmp,oplen);
			if(Re != 0)
			{
				#ifdef SXL_DEBUG
                DDEBUG("\r\nPCIPOS_tidylogfile write fail\r\n");
				#endif
				err = 1;
				break;
			}
			len += tmplen;
		}
		
		
		
        if(err)
        {
        	pcicorecallfunc.pci_kern_delete_file(PCIHLLOGFILE);
			pcicorecallfunc.pci_kern_delete_file(TMPPCIHLLOGFILE);
        }
        else
        {
        	#ifdef SXL_DEBUG
            DDEBUG("\r\ntidy log file complete\r\n");
			#endif
			pcicorecallfunc.pci_kern_delete_file(PCIHLLOGFILE);
			pcicorecallfunc.pci_kern_rename_file(TMPPCIHLLOGFILE,PCIHLLOGFILE);
			
        }
		
		
	}
    
	pcicorecallfunc.mem_free(tmp);

	
}


s32 pcilog_get_asciitime(u8 *timebuf)   //20140327
{
	
    u8 datetime[10];

	memset(datetime,0,sizeof(datetime));
    //ddi_misc_get_time(datetime);   //sxl?2017
    sprintf((s8 *)timebuf,"%02d%02d-%02d-02%d %02d:%02d:02%d",datetime[0],datetime[1],datetime[2],datetime[3],datetime[4],datetime[5],datetime[5]);
    
    
    return 0;
    
    
}

/*******************************************************************
Author:   sxl
Function Name:    void pcilog_file_log(u8 *buf, u32 len, u8 dev)
Function Purpose: PCI LOG
Input Paramters:N/A
Output Paramters:N/A
Return Value:      N/A
Remark: N/A      
Modify: N/A 		//20140327
********************************************************************/
void pcilog_file_log(s8 *buf, u32 len, u8 dev)
{
    s16 i;
    s16 offs = 0; 
    s8 tmp[80] = {0};
    s8 stream[512];
	
    //DDEBUG("\r\npcilog_file_log1\r\n");   //sxl?

	if(len > 511)
	{
		return;
	}

	if(strlen(buf) > 511)
	{
		return;
	}

	
    if (dev == 1)
    {
        DDEBUG("\r\npcilog_file_log 1\r\n");
		pcilog_tidylogfile();
		DDEBUG("\r\npcilog_file_log 2\r\n");
    }
	
	
	memset(tmp,0,sizeof(tmp));
	pcilog_get_asciitime((u8 *)tmp);		
    DDEBUG("\r\npcilog_file_log 3\r\n");
	
	
    memset(stream,0,sizeof(stream));
    if (0 == len) 					
    {
        sprintf(stream, "**** LOG TIME:%s ****\n", tmp);
		i = strlen(stream);
        sprintf(&stream[i], "%s\n", buf);
		//strcat(stream,tmp);
    }
    else 
    {								
        sprintf(stream, "**** LOG TIME:%s (LEN=%08xh) ****\n", tmp, len);
    	sprintf(stream, "%08xh: ", offs);
		strcat(stream,tmp);
        for (i = 0; i < len; i++) 
		{
            sprintf(stream, "%02X ", (u8)buf[i]);
			strcat(stream,tmp);
            if (((i + 1) % 8) == 0) strcat(stream, " ");
            if ((((i + 1) % 16) == 0) || (i == len - 1)) 
			{
            	strcat(stream, "\n");
            	if (i < len - 1) 
				{
        	    	if (((i + 1) % 256) == 0) strcat(stream, "\n");
            		offs += 16;
    				sprintf(stream, "%08xh: ", offs);
					strcat(stream,tmp);
    			}
            }
        }
    }
    strcat(stream, "\n");

	DDEBUG("\r\npcilog_file_log 4\r\n");
    if (dev == 1)
    {
        pcilog_AppendExternalFile((u8 *)PCIHLLOGFILE,(u8 *)stream,strlen(stream));
    }
    DDEBUG("\r\npcilog_file_log 5\r\n");
	
}


void pcilog_savekeymanagelog(s32 gDownLoadKeyErr)
{
    #ifdef SAVEPCILOG
    s8 g_strLogBuff[512];
	s8 tmpissuersn[100];
	s8 tmpbuf[100];

	memset(g_strLogBuff,0,sizeof(g_strLogBuff));
	if(gPciKeyLogInfo.step == AUTHENTICATE_REQUEST)
	{
	    memset(tmpissuersn,0,sizeof(tmpissuersn));
		memcpy(tmpissuersn,gPciKeyLogInfo.issuersn,48);
		strcpy(g_strLogBuff,tmpissuersn);
		strcat(g_strLogBuff," " );
		strcat(g_strLogBuff,&tmpissuersn[16]);
		strcat(g_strLogBuff,"authenticate: \n");

		if(gDownLoadKeyErr == PCI_PROCESSCMD_SUCCESS)
		{
			strcat(g_strLogBuff,"pos authenticate success!");
		}
		else
		{
		    memset(tmpbuf,0,sizeof(tmpbuf));
			pcilog_downkeyerrprompttransfer(gDownLoadKeyErr,(s8 *)tmpbuf);
			strcat(g_strLogBuff,tmpbuf);
		}
		pcilog_file_log(g_strLogBuff,0,1);
	}
	else if(gPciKeyLogInfo.step == AUTHENTICATE_STEP3)
	{
	    //sxl?   gPciKeyLogInfo.result 还没有赋值
		if((gPciKeyLogInfo.result != PCI_PROCESSCMD_SUCCESS)||(gDownLoadKeyErr != PCI_PROCESSCMD_SUCCESS))
		{
			strcpy(g_strLogBuff,"trsm authenticate failed!");
		}
		else
		{
			strcpy(g_strLogBuff,"trsm authenticate success!");
		}
		pcilog_file_log(g_strLogBuff,0,1);
	}
	else if(gPciKeyLogInfo.step == LOAD_APPKEY_REQUEST)
	{
	    memset(tmpbuf,0,sizeof(tmpbuf));
		pcilog_getmanagekeycaption(gPciKeyLogInfo.keytype,(s8 *)tmpbuf);
		sprintf(g_strLogBuff,"Down main key groupindex(%d) keytype(%s)keyindex(%d) \n",gPciKeyLogInfo.groupindex,tmpbuf,gPciKeyLogInfo.keyindex);
		if(gDownLoadKeyErr == PCI_PROCESSCMD_SUCCESS)
		{
			strcat(g_strLogBuff,"Down key success!");
		}
		else
		{
		    memset(tmpbuf,0,sizeof(tmpbuf));
			pcilog_downkeyerrprompttransfer(gDownLoadKeyErr,(s8 *)tmpbuf);
			strcat(g_strLogBuff,tmpbuf);
		}
		pcilog_file_log(g_strLogBuff,0,1);
	}
	else if(gPciKeyLogInfo.step == LOAD_APPDUKPTKEY_REQUEST)
	{
		memset(tmpbuf,0,sizeof(tmpbuf));
		strcpy(tmpbuf,"DUKPTK");
		sprintf(g_strLogBuff,"Down main key groupindex(%d) keytype(%s)keyindex(%d) \n",gPciKeyLogInfo.groupindex,tmpbuf,gPciKeyLogInfo.keyindex);
		if(gDownLoadKeyErr == PCI_PROCESSCMD_SUCCESS)
		{
			strcat(g_strLogBuff,"Down key success!");
		}
		else
		{
		    memset(tmpbuf,0,sizeof(tmpbuf));
			pcilog_downkeyerrprompttransfer(gDownLoadKeyErr,tmpbuf);
			strcat(g_strLogBuff,tmpbuf);
		}
		pcilog_file_log(g_strLogBuff,0,1);
	}
	else if(gPciKeyLogInfo.step == COMMAND_DELETEALLKEY)
	{
		if(gDownLoadKeyErr == PCI_PROCESSCMD_SUCCESS)
		{
			strcpy(g_strLogBuff,"Delete all key success!");
		}
		else
		{
			strcpy(g_strLogBuff,"Delete all key failed!");
		}
		pcilog_file_log(g_strLogBuff,0,1);
	}
	else if(gPciKeyLogInfo.step == COMMAND_DELETEGROUPALLKEY)
	{
		if(gDownLoadKeyErr == PCI_PROCESSCMD_SUCCESS)
		{
			sprintf(g_strLogBuff,"Delete group(%d) key success!",gPciKeyLogInfo.groupindex);
		}
		else
		{
			sprintf(g_strLogBuff,"Delete group(%d) key failed!",gPciKeyLogInfo.groupindex);
		}
		pcilog_file_log(g_strLogBuff,0,1);
	}
	else if(gPciKeyLogInfo.step == APPKEY_SETPINK)
	{
	    DDEBUG("\r\nlog APPKEY_SETPINK 1\r\n");
		memset(tmpbuf,0,sizeof(tmpbuf));
		strcpy(tmpbuf,"PINK");
		sprintf(g_strLogBuff,"Down work key groupindex(%d) keytype(%s) mainkeyindex(%d) keyindex(%d) \n",gPciKeyLogInfo.groupindex,tmpbuf,gPciKeyLogInfo.mainkeyindex,gPciKeyLogInfo.keyindex);
		if(gDownLoadKeyErr == PCI_PROCESSCMD_SUCCESS)
		{
			strcat(g_strLogBuff,"Down key success!");
		}
		else
		{
		    memset(tmpbuf,0,sizeof(tmpbuf));
			pcilog_downkeyerrprompttransfer(gDownLoadKeyErr,tmpbuf);
			strcat(g_strLogBuff,tmpbuf);
		}
		DDEBUG("\r\nlog APPKEY_SETPINK 2\r\n");
		pcilog_file_log(g_strLogBuff,0,1);
		DDEBUG("\r\nlog APPKEY_SETPINK 3\r\n");
	}
	else if(gPciKeyLogInfo.step == APPKEY_SETMACK)
	{
		memset(tmpbuf,0,sizeof(tmpbuf));
		strcpy(tmpbuf,"MACK");
		sprintf(g_strLogBuff,"Down work key groupindex(%d) keytype(%s) mainkeyindex(%d) keyindex(%d) \n",gPciKeyLogInfo.groupindex,tmpbuf,gPciKeyLogInfo.mainkeyindex,gPciKeyLogInfo.keyindex);
		if(gDownLoadKeyErr == PCI_PROCESSCMD_SUCCESS)
		{
			strcat(g_strLogBuff,"Down key success!");
		}
		else
		{
		    memset(tmpbuf,0,sizeof(tmpbuf));
			pcilog_downkeyerrprompttransfer(gDownLoadKeyErr,tmpbuf);
			strcat(g_strLogBuff,tmpbuf);
		}
		pcilog_file_log(g_strLogBuff,0,1);
	}

	gPciKeyLogInfo.step = 0;
	#endif
}


u32 pcilog_getlogfilelength(u8 *filelendata)
{
    u32 logfilelen;
	
	
	
	logfilelen = pcicorecallfunc.pci_kern_access_file(PCIHLLOGFILE);
	
	filelendata[0] = (u8)(logfilelen>>24);
	filelendata[1] = (u8)(logfilelen>>16);
	filelendata[2] = (u8)(logfilelen>>8);
	filelendata[3] = (u8)(logfilelen);
	
	return 4;
	
}



u32 pcilog_getlogfiledata(u8 *inputdata,u8 *filelendata)
{

	
    u32 dwOffset;
    u32 readlen;
	u32 logfilelen;
	s32 iret;
    u32 senddatalen = 0;
	
	
	logfilelen = pcicorecallfunc.pci_kern_access_file(PCIHLLOGFILE);
	dwOffset = dwOffset+(inputdata[0]<<24)+(inputdata[1]<<16) + (inputdata[2]<<8) + inputdata[3];

	#ifdef SXL_DEBUG
    DDEBUG("\r\ndwOffset = %04x %04x\r\n",logfilelen,dwOffset);
	#endif
	if(dwOffset < logfilelen)
	{   
	    if(dwOffset+1000 < logfilelen)
	    {
	    	readlen = 1000;
	    }
		else
		{
			
			readlen = logfilelen - dwOffset;
		}
        iret = pcicorecallfunc.pci_kern_read_file(PCIHLLOGFILE,&filelendata[1],dwOffset,readlen);
		#ifdef SXL_DEBUG
        DDEBUG("\r\niret = %d\r\n",iret);
		#endif
        if (iret != readlen)
        {
			filelendata[0] = 1;
			senddatalen = 1;
        }
        else
        {
            filelendata[0] = 0;
            senddatalen = readlen + 1;
			
        }
	}
	
	return senddatalen;
	
}


u32 pcilog_logfile_delete(u8 *senddata)
{
	
    u8 iret;
	
	iret = pcicorecallfunc.pci_kern_delete_file(PCIHLLOGFILE);
	if(iret == 0)
	{
		senddata[0] = 0;
	}
	else
	{
		senddata[0] = 1;
	}
	
	return 1;
	
}


