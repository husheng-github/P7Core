


#include "bootglobal.h"



s32 bootdownload_parserecdata_teibag(u8* Src, u16 len)
{
	u8 data;
    u16 index = 0;
	u8 checkcrc[2];

	
   	
    while(1)
    {
        if(index >= len)
        {
            break;
        }
    
        data = *(Src+index);
        index++;
		
		
		#ifdef SXL_DEBUG
		if(gParseData.CommParseCmdStep>=33)
		{
	    	BOOT_DEBUG("%02x %d\r\n",data,gParseData.CommParseCmdStep);
		}
		#endif
		
        
        switch(gParseData.CommParseCmdStep)
        {
            #if 0
            case 20:
                if(data == 0x02)
                {
                    gParseData.CommParseCmdStep++;
                }
                break;
			#endif
           //标志     
           case 21:
                if(data == 'T')
                    gParseData.CommParseCmdStep++;
                else
                    gParseData.CommParseCmdStep = 0;
                break;
                
            case 22:
                if(data == 'R')
                    gParseData.CommParseCmdStep++;
                else
                    gParseData.CommParseCmdStep = 0;
                break;
            case 23:
                if(data == 'E')
                    gParseData.CommParseCmdStep++;
                else
                    gParseData.CommParseCmdStep = 0;
                break;
            case 24:
                if(data == 'T')
                    gParseData.CommParseCmdStep++;
                else
                    gParseData.CommParseCmdStep = 0;
                break;
            case 25:
                if(data == 'E')
                    gParseData.CommParseCmdStep++;
                else
                    gParseData.CommParseCmdStep = 0;
                break;
            case 26:
                if(data == 'I')
                {
                    gParseData.RxCNT = 0;
                    gParseData.CommParseCmdStep++;
                }
                else
                    gParseData.CommParseCmdStep = 0;
                break;
                           
            //流水号    
            case 27:
				gParseData.pdownfiledata[gParseData.RxCNT++] = data;
                gParseData.frame.sn = data;
                gParseData.CommParseCmdStep++;
                break;

            //命令高8位
            case 28:       
				gParseData.pdownfiledata[gParseData.RxCNT++] = data;
                gParseData.frame.cmd = data<<8;
                gParseData.CommParseCmdStep++;
                break;
            //命令低8位
            case 29:   
				gParseData.pdownfiledata[gParseData.RxCNT++] = data;
                gParseData.frame.cmd += data;
                gParseData.CommParseCmdStep++;
                break;
            //数据长度高8位
            case 30:    
				gParseData.pdownfiledata[gParseData.RxCNT++] = data;
                gParseData.frame.len = data<<8;
                gParseData.CommParseCmdStep++;
                break;
            //数据长度低8位
            case 31:        
				gParseData.pdownfiledata[gParseData.RxCNT++] = data;
                gParseData.frame.len += data;
                //gParseData.RxCNT = 0;
				if(gParseData.frame.len > (PC_DL_BLOCK_SIZE+200))
				{
					gParseData.CommParseCmdStep = 0;
				}
				else if(gParseData.frame.len == 0)
				{
					gParseData.CommParseCmdStep += 2;
				}
				else
				{
					gParseData.CommParseCmdStep++;
				}
                break;

            //收数据
            case 32:
                gParseData.pdownfiledata[gParseData.RxCNT++] = data;
                if(gParseData.RxCNT >= (gParseData.frame.len+5))
                {
                    gParseData.CommParseCmdStep++;    
                }
                break;
                
            case 33:
                if(data != 0x03)
                {
                    gParseData.CommParseCmdStep = 0;
                }
                else
                {
                   gParseData.CommParseCmdStep++;
                }
                break;

            case 34://校验和，先忽略   
                gParseData.crc[0] = data;
                gParseData.CommParseCmdStep++; 
                break;
                
            case 35:
				gParseData.crc[1] = data;
				pciarithCrc16CCITT(&gParseData.pdownfiledata[1],gParseData.frame.len+4,checkcrc);
                #ifdef SXL_DEBUG
                BOOT_DEBUG("uart:%02x %02x %02x %02x\r\n",checkcrc[0],checkcrc[1],gParseData.crc[0],gParseData.crc[1]);
				#endif
				if(memcmp(checkcrc,gParseData.crc,2) == 0)
				{
					gParseData.Rx_Valid = 1;
				}
                else
                {
                	gParseData.Rx_Valid = 2;   // format error
                }
				gParseData.protocoltype= PROTOCOL_TYPE_TEI;
                gParseData.CommParseCmdStep = 0;
                return 1;
                break;
                
            default:
                gParseData.CommParseCmdStep = 0;
                break;
			
        }
    }
	
	return 0;
	
}



s32 bootdownload_parserecdata_updatebag(u8* Src, u16 len)
{
    u8 data;
    u16 index = 0;
   	
    while(1)
    {
        if(index >= len)
        {
            break;
        }
    
        data = *(Src+index);
        index++;
		
        
        switch(gParseData.CommParseCmdStep)
        {
            case 0:
                if(data == 0x55)
                {
                    gParseData.CommParseCmdStep = 1;
                }
                break;
           case 1:
                if(data == 0x55)
                    gParseData.CommParseCmdStep = 2;
                else
                    gParseData.CommParseCmdStep = 0;
                break;

			
                
            case 2:
                if(data == 0xaa)
                    gParseData.CommParseCmdStep = 3;
                else
                    gParseData.CommParseCmdStep = 0;
                break;
                
            case 3:
                if(data == 0xaa)
                {
                    gParseData.RxCNT = 0;
                    gParseData.checksum = 0;
                    gParseData.CommParseCmdStep = 4;
                }
                else
                {
                    gParseData.CommParseCmdStep = 0;
                }
                break;
                
            case 4:
                gParseData.pdownfiledata[gParseData.RxCNT++] = data;
                gParseData.checksum += data;
                if(gParseData.RxCNT >= PC_DL_FRAME_SIZE_MAX)
                {
                    gParseData.CommParseCmdStep = 5;
                }
                break;
            case 5:
                if(data == 0x55)
                {
                    gParseData.CommParseCmdStep = 6;
                }
                else
                {
                    gParseData.CommParseCmdStep = 0;
                }
                break;
            case 6:
                if(data == 0xaa)
                {
                    gParseData.CommParseCmdStep = 7;
                }
                else
                {
                    gParseData.CommParseCmdStep = 0;
                }
                break;
            case 7:
                if(data == 0x55)
                {
                    gParseData.CommParseCmdStep = 8;
                }
                else
                {
                    gParseData.CommParseCmdStep = 0;
                }
                break;
            case 8:
                if(data == 0xaa)
                {
                    gParseData.CommParseCmdStep = 9;
                }
                else
                {
                    gParseData.CommParseCmdStep = 0;
                }
                break;
            case 9:
                gParseData.CommParseCmdStep = 0;
                if(data == gParseData.checksum)
                {
                    gParseData.protocoltype= PROTOCOL_TYPE_55AA;
					gParseData.Rx_Valid = 1;
                    return 1;//收到一个完整的包
                }
                break;
            default:
                break;
			
        } 
		
		
    }    
	
	return 0;
	
}



s32 bootdownload_parserecdata(void)
{
    s32 i = 0;
	u8 data;
	s32 ret = 0;
	u32 timeoutvalue = 200;  //20ms内没有收到数据的话，清除上一包
	

	while((gParseData.Rx_Valid == 0))
	{
		
        i= dev_com_read(UARTDOWNLOAD_PORTNUM,&data,1);
		
		
		
		if(i <= 0)
		{
		    if(gParseData.rectimer == 0)  //超时时间到，清数据
		    {
		    	gParseData.CommParseCmdStep = 0;
		    }        
			break;
		}

        gParseData.rectimer = timeoutvalue;
		dev_user_timer_open(&gParseData.rectimer); //重开定时
		
		switch(gParseData.CommParseCmdStep)
        {
            case 0:
                if(data == 0x55)
                {
                    gParseData.CommParseCmdStep = 1;
                }
				else if(data == 0x02)  // spi 协议
				{
					gParseData.CommParseCmdStep = 21;
				}
                break;
			default:
				if(gParseData.CommParseCmdStep<=20)
				{
					ret = bootdownload_parserecdata_updatebag(&data,1);
				}
				else if(gParseData.CommParseCmdStep>20&&gParseData.CommParseCmdStep<40)
				{
					ret = bootdownload_parserecdata_teibag(&data,1);
				}
				if(ret)
				{
					return ret;
				}
				break;
				
				
		}
		
		
	}
	
    return ret;
	
}

