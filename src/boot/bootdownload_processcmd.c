


#include "bootglobal.h"
#include "ddi/ddi_misc.h"
#include "ddi/ddi_arith.h"
#include "ddi/ddi_led.h"


s32 processcmd_bootmanage_spsoftreset(teiframe *frame,u8 *respdata,u16 *respdatalen)
{
    frame->res = BOOT_RESPONSECODE_SUCCESS;
	
	processcmd_bootload_senddata(frame,NULL,0);
    
	dev_user_delay_ms(100);   //delay 100 ms
    
    dev_misc_reboot(ST_REBOOT);//dev_smc_systemsoftreset();

	return BOOT_RESPONSECODE_SUCCESS;
	
}


s32 processcmd_bootmanage_spverget(teiframe *frame,u8 *respdata,u16 *respdatalen)
{
	
	u32 offset = 0;
	u8 ver[100];
	u32 verlen;
	
	
	if(frame->len < 1)
	{
		return BOOT_RESPONSECODE_PARAMERR;
	}
	
	
	if(frame->data[0]&0x01)  //  get boot version
	{
		
	    memset(ver,0,sizeof(ver));
		pitver_get_bootver(ver);
		verlen = strlen(ver);
		if(verlen > 20)
		{
			verlen = 20;
		}
		respdata[offset++] = verlen;
		memcpy(&respdata[offset],ver,verlen);
		offset += verlen;
		
	}
	
	
	if(frame->data[0]&0x02)  //  get core version
	{
		
		memset(ver,0,sizeof(ver));
		pitver_get_corever(ver);
		verlen = strlen(ver);
		if(verlen > 20)
		{
			verlen = 20;
		}
		respdata[offset++] = verlen;
		memcpy(&respdata[offset],ver,verlen);
		offset += verlen;
		
	}
	
	
	if(frame->data[0]&0x04)  //  get app version
	{
		
	    memset(ver,0,sizeof(ver));
		//pittable_getappversion(ver);                          //sxl?2017
		verlen = strlen(ver);
		if(verlen > 20)
		{
			verlen = 20;
		}
		respdata[offset++] = verlen;
		memcpy(&respdata[offset],ver,verlen);
		offset += verlen;
		
	}
	
	
	if(frame->data[0]&0x08)  //  get hardware version 
	{
		memset(ver,0,sizeof(ver));
		pitver_get_hardwarever(ver);
		verlen = strlen(ver);
		if(verlen > 20)
		{
			verlen = 20;
		}
		respdata[offset++] = verlen;
		memcpy(&respdata[offset],ver,verlen);
		offset += verlen;
	}

    *respdatalen = offset;
	
	return BOOT_RESPONSECODE_SUCCESS;

}


s32 processcmd_bootmanage_spbeep(teiframe *frame,u8 *respdata,u16 *respdatalen)
{
	u8 beeptimes,i;
	u32 beepfreq = 0,beepopentime = 0,beepclosetime = 0;
    
	if(frame->len < 13)
	{
		return BOOT_RESPONSECODE_PARAMERR;
	}

	beeptimes = frame->data[0];
	if(beeptimes > 5)   // beep 次数不超过5次
	{
		beeptimes = 5;
	}

	
    for(i = 0;i < 4;i++)
    {
    	beepfreq = (beepfreq<<8) + frame->data[1+i];
		beepopentime = (beepopentime<<8) + frame->data[5+i];
		beepclosetime = (beepclosetime<<8) + frame->data[9+i];
    }
	
	
	//频率暂时无效
	for(i = 0;i < beeptimes;i++)
	{
		//ddi_audio_ioctl(DDI_AUDIO_CTL_BUZZER, 2, beepopentime);  //sxl?2017
		if((i+1)<beeptimes)
		{
			dev_user_delay_ms(beepclosetime);   //sxl?2017
		}
		
	}
	
	return BOOT_RESPONSECODE_SUCCESS;
	
}


s32 processcmd_bootmanage_spledctrl(teiframe *frame,u8 *respdata,u16 *respdatalen)
{
	u32 lednum;
	u8 i;
	u32 offset = 0;
	strLedGleamPara ledctrl;
	u32 ledopentime,ledclosetime,ledopstotaltime;
	
	if(frame->len < 11)
	{
		return BOOT_RESPONSECODE_PARAMERR;
	}
	
	lednum = frame->len /11;
	if(lednum > 4)
	{
		lednum = 4;
	}

	for(i = 0;i < lednum;i++)
	{
	    //超过灯范围，或者灯保持不变
		if(frame->data[offset] > 3 ||frame->data[offset+1] > 2 )
		{
			offset += 11;
		}
		
		
		//开LED
		if(frame->data[offset+1] == 1)
		{
			//ddi_led_sta_set(frame->data[offset],LEDSTATUS_ON);   //sxl?2017
		}
        else if(frame->data[offset] == 2) // LED 闪
        {
        	//ddi_led_sta_set(frame->data[offset],LEDSTATUS_OFF);  //sxl?2017
        }
		else  // LED 关
		{
			ledopentime = frame->data[offset+3];
			ledopentime = (ledopentime<<8) + frame->data[offset+4];

			ledclosetime = frame->data[offset+5];
			ledclosetime = (ledopentime<<8) + frame->data[offset+6];

			ledopstotaltime = frame->data[offset+7];
			ledopstotaltime = (ledopstotaltime<<8) + frame->data[offset+8];
			ledopstotaltime = (ledopstotaltime<<8) + frame->data[offset+9];
			ledopstotaltime = (ledopstotaltime<<8) + frame->data[offset+10];
			
	
			//ddi_led_sta_set(frame->data[offset],LEDSTATUS_ON);    //sxl?2017
			ledctrl.m_led      = frame->data[offset];
			ledctrl.m_duration = ledopstotaltime;  //一直闪
			ledctrl.m_ontime   = ledopentime;
			ledctrl.m_offtime  = ledclosetime;

			//ddi_led_ioctl(DDI_LED_CTL_GLEAM,(u32)&ledctrl,0);   //sxl?2017
			
		}

		
	}
	
	
	return BOOT_RESPONSECODE_SUCCESS;
	
	
}



s32 processcmd_boot_firmwaremanage_getcertinfo(teiframe *frame,u8 *respdata,u16 *respdatalen)
{
    s32 ret;
	s32 retCode;
	u8 pkfilename[100];
	u8 tmphashvalue[32];
	RSA_PUBLICKEY_INFO *publickeyinfo;
	u32 pkfilenamelen;

    *respdatalen = 0;
	if(frame->len < 0||frame->data[0] == 0)
	{
		return BOOT_RESPONSECODE_PARAMERR;
	}

	memset(pkfilename,0,sizeof(pkfilename));
	strcpy(pkfilename,"/mtd2/");
	
	
	pkfilenamelen = frame->data[0];
	if(pkfilenamelen>90)
	{
		pkfilenamelen = 90;
	}
	memcpy(&pkfilename[6],&frame->data[1],pkfilenamelen);
	pkfilename[6+pkfilenamelen] = 0;

	
	publickeyinfo = (RSA_PUBLICKEY_INFO *)k_malloc(sizeof(RSA_PUBLICKEY_INFO));
	ret = fs_read_file((const char *)pkfilename,(u8 *)publickeyinfo,sizeof(RSA_PUBLICKEY_INFO),0);
	if(ret == sizeof(RSA_PUBLICKEY_INFO))
	{
		devarith_hash((u8 *)publickeyinfo,PUBLICKEY_CERTSIGN_OFFSET,tmphashvalue,DDI_HASHTYPE_256);
		if(memcmp(publickeyinfo->CertSign,tmphashvalue,32) == 0)
    	{
    	    *respdatalen = 48;
			memcpy(respdata,publickeyinfo->Issuer,48);
    		retCode =  BOOT_RESPONSECODE_SUCCESS;
    	}
		else
		{
			retCode =       BOOT_RESPONSECODE_FAIL;
		}
		
	}
	else
	{
		retCode =       BOOT_RESPONSECODE_FAIL;
	}

    k_free(publickeyinfo);
	return retCode;
	
}


s32 processcmd_boot_firmwaremanage_updateflagset(u8 *respdata,u16 *respdatalen)
{
	
//	u8 tmp[32];
    s32 ret;
	
	
    *respdatalen = 0;
//    memcpy(tmp, "updateflag", 10);
//    ret = fs_write_file("/mtd2/updateflag", tmp, 10);
//    if(ret == 10)
    ret = dev_misc_updateflg_set(1);
    if(ret >= 0)
    {
		return BOOT_RESPONSECODE_SUCCESS;
    }
	else
	{
		return BOOT_RESPONSECODE_FAIL;
	}
	
}

s32 processcmd_boot_firmwaremanage_updateflagclr(u8 *respdata,u16 *respdatalen)
{
//	u8 tmp[32];
	s32 ret;
    
    *respdatalen = 0;
    
//    memset(tmp, 0xff, 10);
//    ret = fs_write_file("/mtd2/updateflag", tmp, 10);
//    if(ret == 10)
    ret = dev_misc_updateflg_set(0);
    if(ret >= 0)
    {
		return BOOT_RESPONSECODE_SUCCESS;
    }
	else
	{
		return BOOT_RESPONSECODE_FAIL;
	}
	
	
}


void bootdownload_processrecapbag(void)
{
	gFileBagInfo->filenum = gParseData.pdownfiledata[0+5];
    memset(gFileBagInfo->filename,0,51);
    memcpy(gFileBagInfo->filename,&gParseData.pdownfiledata[1+5],51);
    gFileBagInfo->filelength = Long2Byte(&gParseData.pdownfiledata[52+5]);
    gFileBagInfo->crc = Long2Byte(&gParseData.pdownfiledata[56+5]);
    gFileBagInfo->offset = Long2Byte(&gParseData.pdownfiledata[60+5]);
    gFileBagInfo->p = &gParseData.pdownfiledata[64+5];
    
	
}


//固件升级
s32 processcmd_boot_firmwaremanage_sendfirwarebin(teiframe *frame,u8 *respdata,u16 *respdatalen)
{
    u8 errflag = 0;
    u8 promptlen;
	s32 ret;
    FILEBAGINFO FileBagInfo;

    promptlen = 0;
	*respdatalen = 0;
	gFileBagInfo = &FileBagInfo;
	bootdownload_processrecapbag();
	
	ret = bootdownload_dealdownloadlist(&FileBagInfo);
	#ifdef SXL_DEBUG
    BOOT_DEBUG("bootdownload_dealdownloadlist = %d\r\n",ret);
	#endif
	if(ret != 0)
	{
		 errflag = 1;
    	bootdownload_processerr_response((u8 )ret,&promptlen,respdata);
	}


	if(errflag == 0)
	{
		ret = bootdownload_dealdownloadbag(&FileBagInfo);
		#ifdef SXL_DEBUG
        BOOT_DEBUG("bootdownload_dealdownloadbag ret = %d\r\n",ret);
		#endif
	    if(ret == 1)
	    {
	        errflag = 1;
	    	bootdownload_processerr_response(EXECUTION_DOWNFILEOPS,&promptlen,respdata);
	    }
	    else if(ret == 2)
	    {
	        errflag = 1;
			bootdownload_processerr_response(EXECUTION_CRCERR,&promptlen,respdata);
	    }
		else if(ret == 3)  //没有接收完，继续下载
		{
			return BOOT_RESPONSECODE_SUCCESS;
		}
	}
    			
	if(errflag == 0)
	{
		ret = bootdownloadauthen_processfileauthentication();
		#ifdef SXL_DEBUG
        BOOT_DEBUG("authen ret = %d\r\n",ret);
		#endif
	    if(ret != 0)
	    {
	        errflag = 1;
			bootdownload_processerr_response(AUTHENTICATEFAIL,&promptlen,respdata);
	    }
		else //sxl? 还需要增加版本比较，低版本程序不能覆盖高版本程序
		{
			
			ret = bootdownload_downloadfilesave();
			if(ret != 0)
			{
			    errflag = 1;
				bootdownload_processerr_response(EXECUTION_SAVEINFOERROR,&promptlen,respdata);
			}
		}
		
	}
	
	if(errflag)
	{
		return BOOT_RESPONSECODE_FAIL;
	}
	else
	{
		return BOOT_RESPONSECODE_SUCCESS;
	}
	
}


s32 processcmd_boot_getspstatus(u8 *respdata,u16 *respdatalen)
{
	memcpy(respdata,(u8 *)&gSpStatus,4);
	*respdatalen = 4;
	return BOOT_RESPONSECODE_SUCCESS;
}




s32 processcmd_bootmanage(teiframe *frame,u8 *respdata,u16 *respdatalen,u16 maxbufflen)
{
	
	s32 ret = BOOT_RESPONSECODE_CMDNOTSUPPORT;
	
	
	#ifdef SXL_DEBUG
    BOOT_DEBUG("frame->cmd = %04x\r\n",frame->cmd);
	#endif
	switch(frame->cmd)
	{
		case COMMAND_BOOTMANAGE_SPSOFTRESET:
			ret = processcmd_bootmanage_spsoftreset(frame,respdata,respdatalen);
			break;
		case COMMAND_BOOTMANAGE_SPBEEP:
			ret = processcmd_bootmanage_spbeep(frame,respdata,respdatalen);
			break;
		case COMMAND_BOOTMANAGE_SPLEDCTRL:
			ret = processcmd_bootmanage_spledctrl(frame,respdata,respdatalen);
			break;
		case COMMAND_BOOTMANAGE_SPVERSIONGET:
			ret = processcmd_bootmanage_spverget(frame,respdata,respdatalen);
			break;
        case COMMAND_BOOT_FIRMWAREMANAGE_GETCERTINFO:
			ret = processcmd_boot_firmwaremanage_getcertinfo(frame,respdata,respdatalen);
			break;
		case COMMAND_BOOT_FIRMWAREMANAGE_SENDFIRMWAREBINDATA:
//			ret = processcmd_boot_firmwaremanage_sendfirwarebin(frame,respdata,respdatalen);;
			break;
		case COMMAND_BOOT_FIRMWAREMANAGE_UPDATEFLAGSET:
			ret = processcmd_boot_firmwaremanage_updateflagset(respdata,respdatalen);
			break;
		case COMMAND_BOOT_FIRMWAREMANAGE_UPDATEFLAGCLR:
			ret = processcmd_boot_firmwaremanage_updateflagclr(respdata,respdatalen);
			break;
		case COMMAND_BOOT_GETSPSTATUS:
			ret = processcmd_boot_getspstatus(respdata,respdatalen);
			break;
		
		
		
		default:
			break;
		
		
	}
	
	return ret;
	
	
}














void processcmd_bootload_senddata(teiframe *frame,u8 *respdata,u16 respdatalen)
{
	
	u8 *senddatabuf;
	u16 maxdatalen,offset = 0;
	
	
	senddatabuf = (u8 *)k_malloc(respdatalen+32);
	
	//STX
	senddatabuf[offset++] = 0x02;
	
	
	//flag
	if(frame->type == MESSAGE_TYPE_ACK)
	{
		memcpy(&senddatabuf[offset],COMMPROTOCOL_TRENDITSP_ACKFLAG,6);
	}
	else
	{
		memcpy(&senddatabuf[offset],COMMPROTOCOL_TRENDITSP_FLAG,6);
	}
	offset += 6;
	
	//SN
	senddatabuf[offset++] = frame->sn;
    
	
	//command
	senddatabuf[offset++] = (u8)((frame->cmd)>>8);
	senddatabuf[offset++] = (u8)(frame->cmd);
	
    
	//send data length + 一个字节的响应码
	senddatabuf[offset++] = (u8)((respdatalen+1)>>8);
	senddatabuf[offset++] = (u8)(respdatalen+1);
	


	//response code
	senddatabuf[offset++] = (u8)frame->res;
	
	//var data
	if(respdatalen&&respdata!= NULL)
	{
		memcpy(&senddatabuf[offset],respdata,respdatalen);
		offset += respdatalen;
	}

	//EXT
	senddatabuf[offset++] = 0x03;

	
	//CRC
    pciarithCrc16CCITT(&senddatabuf[8],offset- 8 - 1,&senddatabuf[offset]);
	offset += 2;
	
	
	#ifdef SXL_DEBUG
    BOOT_DEBUG("processcmd_bootload_senddata:%d\r\n",offset);
    BOOT_DEBUGHEX(NULL, senddatabuf, offset);
	#endif
	
    dev_com_write(UARTDOWNLOAD_PORTNUM,senddatabuf,offset);
    
	
	
	k_free(senddatabuf);
	
	
}




void processcmd_bootload_sendack(teiframe *frame,u32 status)
{
	if(status == 1)
	{
		frame->res = BOOT_RESPONSECODE_SUCCESS;
	}
	else
	{
		frame->res = BOOT_RESPONSECODE_LRCERR;
	}
	frame->type = MESSAGE_TYPE_ACK;
	processcmd_bootload_senddata(frame,NULL,0);
	
}



void processcmd_bootdownload(void)
{
	s32 response = BOOT_RESPONSECODE_CMDNOTSUPPORT;
	u16 respdatalen = 0;
	u8 *respdata;
    
	s32 i;
	
	if(gParseData.RxCNT < (5 + gParseData.RxTotalLen))
	{
		
		gParseData.Rx_Valid = BOOT_RESPONSECODE_VARFIELDLENERR;
		
	}
    
	
	//解析失败的
	if(gParseData.Rx_Valid != 1)
	{
	    processcmd_bootload_sendack(&gParseData.frame,gParseData.Rx_Valid);
		return;
	}
	
	
    #ifdef SXL_DEBUG
    BOOT_DEBUG("processcmd_bootdownload rec:%d %d\r\n",gParseData.RxCNT,gParseData.RxTotalLen);
    BOOT_DEBUGHEX(NULL, gParseData.pdownfiledata, gParseData.RxCNT);
	#endif
	
    
	processcmd_bootload_sendack(&gParseData.frame,gParseData.Rx_Valid);  //只发送一次

	gParseData.frame.type = MESSAGE_TYPE_RESPONSE;
	gParseData.frame.data = (u8 *)&gParseData.pdownfiledata[5];
	
	//发送数据的长度不会超过1024
	respdata = (u8 *)k_malloc(1024);

	response = processcmd_bootmanage(&gParseData.frame,respdata,&respdatalen,1024);

    gParseData.frame.res = (u8)response;
	if(response != BOOT_RESPONSECODE_WAIT)  // response data
	{
		processcmd_bootload_senddata(&gParseData.frame,respdata,respdatalen);
	}
	k_free(respdata);
	
}



void bootdownload_processcmd(void)
{
	
	if(!gParseData.Rx_Valid)
	{
		return;
	}
	
 #if(MACHINETYPE == MACHINE_S1)
	processcmd_bootdownload();
 #endif
	
	gParseData.Rx_Valid = 0;
	gParseData.CommParseCmdStep = 0;
	
	
}

