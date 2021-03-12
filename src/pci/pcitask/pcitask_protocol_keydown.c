

#include "pciglobal.h"


void pcitask_protocol_communicationinit(void)
{
	gCommData = (COMMUNICATEDATA *)pcicorecallfunc.mem_malloc(sizeof(COMMUNICATEDATA));
	if(gCommData == NULL)
	{
		//ddi_misc_reboot();  //系统资源问题，直接重启   //sxl?2017
	}
	memset((u8 *)gCommData,0,sizeof(COMMUNICATEDATA));
	
}

void pcitask_protocol_communicationexit(void)
{
	pcicorecallfunc.mem_free(gCommData);
}


void pcitask_protocol_parse(void)
{
	u8 data;
	u32 filedatalen;
	u8 tmpcrc16data[2];
	s32 i;
	u8 startread = 0;
	u32 comtimer;
	u32 iocomtime = 20; 
	u8 CommParseStep;
	u8 CountSOH = 0;
	u8 countRFU = 0;
    
    pcicorecallfunc.pci_dev_timer_user_open(&comtimer);
	
	gCommData->Rx_Valid = 0;
	while((gCommData->Rx_Valid == 0)||comtimer)
	{
		//i= ddi_com_read(UARTPORT_PCI,&data,1);   //sxl?2017
		if(i == 0)
		{
		    if(startread == 0)
		    {
		    	pcicorecallfunc.pci_dev_timer_user_close(&comtimer);
				gCommData->Rx_Valid = 0;
				CommParseStep = 0;
				return;	
		    }
			if(comtimer == 0)  //接收数据超时，认为串口没有数据了
			{
				pcicorecallfunc.pci_dev_timer_user_close(&comtimer);
				CommParseStep = 0;
				break;
			}
			else
			{
				continue;
			}
		}
		else  //接收到数据，重新开启定时器
		{
			
			startread = 1;
			comtimer = iocomtime;
			pcicorecallfunc.pci_dev_timer_user_open(&comtimer);
			
		}
		
        switch(CommParseStep)
        {
        	case 0:
				if(data == 0x02)
				{
				    CountSOH = 0;
					CommParseStep = 1;
				}
				else
				{
					CountSOH = 0;
				}
                break;
			case 1:
                gCommData->data[0] = data;
				CommParseStep = 2;
				countRFU = 0;
				break;
			case 2:						
				countRFU++;
				gCommData->data[countRFU] = data;
				if(countRFU> 3)
				{
					countRFU= 0;
					CommParseStep = 3;
				}
				break;
			case 3:	
				gCommData->data[5] = data;
				//PCI安全协议
				if(memcmp(gCommData->data,"XGDPCI",6) == 0)
				{
				    gCommData->data[0] = 0x02;
					memcpy(&gCommData->data[1],"XGDPCI",6);
					gCommData->RxCNT = 7;
					CommParseStep = 30;
				}
				else
				{
					CommParseStep = 0;
					CountSOH = 0;
				}
				break;
			case 30:  //command
                gCommData->data[(gCommData->RxCNT)++] = data;
				if(gCommData->RxCNT >= 9) 
				{
					CommParseStep = 31;
				}
				break;
			case 31: //len
                gCommData->data[(gCommData->RxCNT)++] = data;
				if((gCommData->RxCNT) >= 11)
				{
					filedatalen = gCommData->data[gCommData->RxCNT - 2];
					filedatalen = (filedatalen << 8)+gCommData->data[gCommData->RxCNT-1];
					CommParseStep = 32;
					gCommData->datalen = filedatalen;
					if(filedatalen > COMMDATABUFMAXLEN)
					{
						CommParseStep = 0;
					}
					if(filedatalen == 0)
					{
						CommParseStep = 33;
					}
				}
				break;
			case 32:
                gCommData->data[(gCommData->RxCNT)++] = data;
				if((gCommData->RxCNT) >= (gCommData->datalen + 11))
				{
					CommParseStep = 33;
				}
				break;
			case 33:
				gCommData->data[(gCommData->RxCNT)++] = data;
                if(data == 0x03)
                {
                    CountSOH = 0;
                	CommParseStep = 34;
                }
				else
				{
					CommParseStep = 0;
					//Rx_buflen = 0;
				}
				break;
			case 34:
                gCommData->data[(gCommData->RxCNT)++] = data;
				CountSOH++;
				if(CountSOH >=2)
				{
				    CommParseStep = 0;
					CountSOH = 0;
					//Rx_buflen = 0;
					gCommData->command = gCommData->data[7];
					gCommData->command = (gCommData->command<<8) + gCommData->data[8];
					
					pciarithCrc16CCITT(&gCommData->data[7],gCommData->datalen + 4,tmpcrc16data);
					if(memcmp(&gCommData->data[gCommData->RxCNT-2],tmpcrc16data,2) == 0)
					{
						gCommData->Rx_Valid = 4;
					}
					else
					{
						gCommData->Rx_Valid = 3;
					}
					pcicorecallfunc.pci_dev_timer_user_close(&comtimer);
					return;	
				}
				break;
			default:
				break;
        }		
	}
	
	
}



void pcitask_protocol_keydownload(void)
{
	pcitask_protocol_parse();
    if(gCommData->Rx_Valid == 4)
	{
		pcitask_dealcommanddata(gCommData->command,&gCommData->data[COMMUNICATEDATAPOS-2],gCommData->datalen);
		gCommData->Rx_Valid = 0;
    }
	
}


