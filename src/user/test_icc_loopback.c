#include "posdeviceglobal.h"
#include "dev_icc_loopback.h"
#if 0
#define TRACE(format,...)   dev_debug_printf("%s(%d):"format, __func__, __LINE__, ##__VA_ARGS__)
//#define ICC_DEBUG(format, ...)  dev_debug_printf("[ERROR:%s,Line%d]\n" format "\n", __func__, __LINE__, ## __VA_ARGS__)
#else
#define TRACE(...)
#endif

//#define MAXCOL 64

#define ICCARD_NO1      0
/*#define ICCARD_SIM1     1
#define ICCARD_SIM2     2
#define ICCARD_SIM3     3
#define ICCARD_SIM4     4 
#define ICCARD_SIM5     5
#define ICCARD_SIM6     6*/

enum
{
	ICC_NO_EXIST = 0,    //无卡
	ICC_IN_POWEROFF,     //有卡未上电&已经下电      
	ICC_IN_POWERON,      //有卡&已上电
};

#define ELECTRICSIGLE  	'1'//单次触发模式
#define ELECTRICTIMED   '2'//循环触发模式
#define PROTOCOLTEST	'3'//协议测试
#define PROTOCOLTEST1   '4'//循环协议测试
#define MODERESET       '5'//测复位应答
#define MODESELECTPSE   '6'//测选PSE
#define MODEREADREC     '7'//测读记录
#define MODEDEACTIVATE  '8'//测下电

#define EMVCARD_NO      ICCARD_NO1
#define LCDROW_MAX       MAXROW//5
#define LCDLINEBYTE_MAX  21 

#define	ELECTRIC_CYCLE_DEF	5
#define	PROTOCOL_CYCLE_DEF	5

int electric_cycle = ELECTRIC_CYCLE_DEF;       //电气测试循环时间
int protocol_cycle = PROTOCOL_CYCLE_DEF;       //协议测试循环时间
int EMVTestMode;

u8 const ic_PSETab1[]={0x14,0x00,0xA4,0x04,0x00,0x0E,0x31,0x50,0x41,0x59,0x2E,0x53,0x59,0x53,
                    0x2E,0x44,0x44,0x46,0x30,0x31,0x00};
u8 const ic_PSETab2[]={0x0d,0x00,0xa4,0x04,0x00,0x07,0xa0,0x00,0x00,0x00,0x03,0x10,0x10,0x00}; 
/*u8 const PSETab3[]={0x14,0x00,0xA4,0x04,0x00,0x0E,0x31,0x50,0x41,0x59,0x2E,0x53,0x59,0x53,
                    0x2E,0x44,0x44,0x46,0x30,0x31,0x00};*/
u8 const ic_PSETab4[]={0x05,0x00,0xb2,0x01,0x0c,0x00};
u8 const ic_PSETab5[]={0x05,0x00,0xA4,0x04,0x00,0x80};


/*********************************************************************************************************
                                                              
*********************************************************************************************************/
void SetResetWaitTime(int type)
{
    u8 dispflg = 1;
    u8 key = 0;
	u8 value = 0;
	u8 tmp[32];
    s32 ret;

	if(type == 1)
	{
		value = electric_cycle;
	}
	else
	{
		value = protocol_cycle;
	}
	
    while(1)
    {
        if(dispflg == 1)
        {
            dispflg = 0;
            lcd_interface_clear_ram();								//清屏幕显示
            lcd_interface_fill_rowram(0, 0, "设置循环触发时间", NOFDISP|CDISP);
          #if 0  
            lcd_interface_fill_rowram(1, 0, "1. 1秒  2. 3秒  ", FDISP);
            lcd_interface_fill_rowram(2, 0, "3. 5秒  4. 10秒 ", FDISP);
            lcd_interface_fill_rowram(3, 0, "5. 30秒 6. 35秒 ", FDISP);
          #else
            lcd_interface_fill_rowram(1, 0, "1. 5秒 ", FDISP);
            lcd_interface_fill_rowram(2, 0, "2. 30秒", FDISP);
            
          #endif  
		    sprintf(tmp, "当前设定:%2d秒", value);
            lcd_interface_fill_rowram(4, 0, tmp, FDISP|CDISP);
            lcd_interface_brush_screen();
            keypad_interface_clear();
        }
	    ret = keypad_interface_read_beep(&key); 
        if(ret > 0)
        {
    	    switch(key)
    	    {
           #if 0     
    	    case DIGITAL1:
    	        value = 1;
    	        dispflg = 1;
    	        break;
    	    case DIGITAL2:
    	        value = 3;
    	        dispflg = 1;
    	        break;
    	    case DIGITAL3:
    	        value = 5;
    	        dispflg = 1;
    	        break;
    	    case DIGITAL4:
    	        value = 10;
    	        dispflg = 1;
    	        break;
    	    case DIGITAL5:
    	        value = 30;
    	        dispflg = 1;
    	        break;
    	    case DIGITAL6:
    	        value = 35;
    	        dispflg = 1;
    	        break;
          #else
    	    case DIGITAL1:
    	        value = 5;
    	        dispflg = 1;
    	        break;
    	    case DIGITAL2:
    	        value = 30;
    	        dispflg = 1;
    	        break;
            
          #endif
    	    case ESC://0x3f:   //ESC  huchaoyan 2015-4-3
    		case ENTER:
    			if(type == 1)
    			{
    				 electric_cycle = value;
    			}
    			else
    			{
    				protocol_cycle = value;
    			}
    	        return;
    		default:
    			break;
    	    }
        }
    }
}

void DealOthersFunction(void)//modify by Yana    140403
{
//    u8 tmp[256];
    int flag=1;
    int dispflg=1;
//    int i;
    U8 key;
    s32 ret;
    
    while(flag)
	{
        if(dispflg)
		{
            dispflg=0;
            lcd_interface_clear_ram();								//清屏幕显示
            lcd_interface_fill_rowram(0, 0, "辅助功能", FDISP|CDISP);
            lcd_interface_fill_rowram(1, 0, "1.设置电气循环时间", FDISP);
			lcd_interface_fill_rowram(2, 0, "2.设置协议循环时间", FDISP);
            lcd_interface_brush_screen();
          	keypad_interface_clear();
        }
	    ret = keypad_interface_read_beep(&key);
		//printf("key=%x\r\n",key);
		if(ret > 0)
		{
        	if(key == DIGITAL1)
    		{  
                SetResetWaitTime(1);
                dispflg = 1;
            }
    		else if(key == DIGITAL2)
    		{  
                SetResetWaitTime(2);
                dispflg = 1;
            }
            else if(key == ESC) 
    	    //else if(key == 0x3f)	 
    		{
                flag = 0;
    			return;
            }
		}
    }    
}

void DisplayTxt(u8 startrow, u8 endrow, u8 *src, int srclen)
{
    int startrowtmp;
    int endrowtmp;
    int len;
    int i;
    int j;
    u8 tmp[601];
    tmp[0] = 0;
    if(endrow>=LCDROW_MAX)endrowtmp = LCDROW_MAX-1;
    else endrowtmp = endrow;
    startrowtmp=startrow;      
    if(startrowtmp>endrowtmp)return;
    if(srclen>300)len=300;
    else len = srclen;
    for(i=0; i<len; i++){
        sprintf(&tmp[i<<1], "%02x", src[i]);
    }
    len = i<<1;
    j = 0;
    for(i=startrow; i<=endrow; i++){
        lcd_interface_clear_rowram(i);
    } 
    for(i=startrow; i<=endrow; i++){
        lcd_interface_fill_rowram(i,     0, &tmp[j], FDISP);   
        j += LCDLINEBYTE_MAX;
        if(j >= len)break;
    }
    lcd_interface_brush_screen(); 
}
/***********************************************
    处理IC复位
***********************************************/
s32 DealResetIC(void)
{
    u8 tmp[200]={0},atr[200]={0};
    u16 len;
    s32 ret; 
    //if(EMVTestMode==MODERESET){
    lcd_interface_clear_ram();	
    lcd_interface_fill_rowram(0,     0, "正在复位...", FDISP|CDISP);
    lcd_interface_brush_screen();
    //}
    ret = ddi_iccpsam_poweron(EMVCARD_NO,tmp);//iccard_reset(EMVCARD_NO, tmp, &len);
    if(DDI_OK !=ret){ 
        lcd_interface_clear_ram();	
        lcd_interface_fill_rowram(0,     0, "复位失败", FDISP|CDISP);
        lcd_interface_brush_screen(); 
        //---->延时1秒
        dev_delay_ms(1000);
    }
    else
    {  
    	dev_maths_bcd_to_u16(&len, tmp, 1);
        memcpy(atr,&tmp[1],len);
        lcd_interface_clear_ram();	
        lcd_interface_fill_rowram(0,     0, "复位成功", FDISP|CDISP);
        DisplayTxt(1, 4, atr, len);
        lcd_interface_brush_screen(); 
        if(EMVTestMode==MODERESET){ 
            //---->延时1秒
            dev_delay_ms(1000);
        }
    }
//    dev_key_flush();
    return ret;
}
/***********************************************
    处理IC复位
***********************************************/
s32 DealResetIC1(void)
{
    u8 tmp[200];
 //   u16 len;
    s32 ret; 
    //if(EMVTestMode==MODERESET){
    lcd_interface_clear_ram();	
    lcd_interface_fill_rowram(0,     0, "正在复位...", FDISP|CDISP);
    lcd_interface_brush_screen();
    //}
    TRACE("正在复位...");
    ret = ddi_iccpsam_poweron(EMVCARD_NO,tmp);//iccard_reset(EMVCARD_NO, tmp, &len);
    if(DDI_OK != ret)
    { 
        lcd_interface_clear_ram();	
        lcd_interface_fill_rowram(0,     0, "复位失败", FDISP|CDISP);
        lcd_interface_brush_screen(); 
        //---->延时1秒
        dev_delay_ms(1000);
        TRACE("复位失败");
    }
    else
    {  
    	TRACE("复位成功");	
        dev_delay_ms(500);
    }
//    dev_key_flush();
    return ret;
}
/**********************************************
    处理IC卡
**********************************************/
void DealOperateIC(void)
{
    u8 tmp[500];
    u8 wbuf[300];
    u8 buf[32];
    u32 wlen;
    u32 len;
    s32 ret;
    int flag=1;
    int i;
    //int k;
    int loopcnt=0;
    s32 sw;
    //u8 LRC;
    //u8 lc,le;
    u8 key;

    lcd_interface_clear_rowram(0);
    lcd_interface_fill_rowram(0,     0, "正在操作卡...", FDISP|CDISP);
    lcd_interface_brush_screen();
	
	//modify by Yana  20140402
    for(i=0; i<ic_PSETab1[0]; i++)
    {
        wbuf[i]=ic_PSETab1[i+1]; 
    }
    wlen = ic_PSETab1[0];

    while(flag)
    {
        len=0;
		
    #if 1	
	    ret = keypad_interface_read_beep(&key); 
        if(ret>0)
        {
    		if(key == ESC)
    		{
    			//printf("取消\r\n");  Yana
    			return;
    		}
        }
    #endif
		
        ret = ddi_iccpsam_exchange_apdu(EMVCARD_NO,  wbuf, wlen, tmp, &len,500);//iccard_send_command(EMVCARD_NO, wbuf, wlen, tmp, &len);
        if(ret != DDI_OK)
        {//操作失败 
            TRACE("exchange apdu fail,return to upper func");
            lcd_interface_clear_ram();	
            lcd_interface_fill_rowram(1, 0, "操作IC卡失败", FDISP|CDISP);
            lcd_interface_brush_screen(); 
            dev_delay_ms(1000);
            ddi_iccpsam_poweroff(EMVCARD_NO);//qdy add
            return;
        }
        else
        {   //操作成功
            sw = 0;
            sw = tmp[len-2]<<8 | tmp[len-1];
            len -= 2;       
          #if 0
            printf("sw=0x%04x\r\n",sw);
            int i ;
            for(i = 0;i < len ;i++)
            {
                if(i!=0 && i%16 == 0)
                {
                    printf("\r\n");
                }
                printf("%02x ",tmp[i]);
            }
            printf("\r\n");
            sleep(10);
          #endif
            lcd_interface_clear_rowram(1);
            sprintf(buf, "sw:%04x", sw);
            lcd_interface_fill_rowram(1, 0, buf, FDISP); 
            DisplayTxt(2, 4, tmp, len);
            if((ELECTRICSIGLE==EMVTestMode)||(ELECTRICTIMED==EMVTestMode))//电气特性测试
            {
				if((0 == loopcnt) && (0x9000==sw) )
				{
					//读记录
					if(tmp[1] == 0x70)
					{   //操作完成，发下电命令
	                    flag = 0;
	                    //lcd_interface_clear_ram();	
	                    lcd_interface_clear_rowram(0);
	                    lcd_interface_fill_rowram(0, 0, "操作IC卡成功", FDISP|CDISP);
	                    //dev_lcd_fill_row_ram(1,     0, "1秒后下电", FDISP|CDISP);
	                    lcd_interface_brush_screen(); 
	                    dev_delay_ms(1000);
                        TRACE("操作IC卡成功,下电");
	                    ddi_iccpsam_poweroff(EMVCARD_NO);
	                    return;
					}
					for(i=0; i<ic_PSETab5[0]; i++)
					{
						wbuf[i] = ic_PSETab5[i+1];
					}
					for( ; i<ic_PSETab5[0]+0x80; i++)
					{
						wbuf[i] = i - ic_PSETab5[0];
					}
					wlen = ic_PSETab5[0] + 0x80;				
					continue;
				}
                else
                {   //操作完成，发下电命令
                    flag = 0;
                    //lcd_interface_clear_ram();	
                    lcd_interface_clear_rowram(0);
                    lcd_interface_fill_rowram(0,     0, "操作IC卡成功", FDISP|CDISP);
                    //dev_lcd_fill_row_ram(1,     0, "1秒后下电", FDISP|CDISP);
                    lcd_interface_brush_screen(); 
                    TRACE("操作IC卡成功,下电");
                    dev_delay_ms(1000);
                    ddi_iccpsam_poweroff(EMVCARD_NO);
                    return;
                }
            }
            else//协议测试	//modify by Yana 140402
            {
				if(len<4)//if R-APDU<6 bytes     len没有包含sw//Select APDU
				{
					for(i=0; i<ic_PSETab1[0];i++)
					{
						wbuf[i]=ic_PSETab1[i+1];
					}
					wlen = ic_PSETab1[0];
				}
				else//if R-APDU>=6 bytes
				{
					if(tmp[1] == 0x70)
					{
						//操作完成,发下电命令
						TRACE("下电");
						ddi_iccpsam_poweroff(EMVCARD_NO);
						return;
					}
					else
					{// R-APDU 
						memcpy(wbuf, tmp, len);
						wlen = len;
					}
				}                
            }
        }
    }
}
/*
#0 MODERESET       '5'//测复位应答
#1 MODESELECTPSE   '6'//测选PSE
#2 MODEREADREC     '7'//测读记录
#3 MODEDEACTIVATE  '8'//测下电
*/
void DealSingleMode(int type)
{
    u8 tmp[500];
    u8 wbuf[300];
    u8 buf[32];
    u16 wlen;
    u32 len;
    int i;
    u16 sw; 
    s32 ret;
    
    //lcd_interface_clear_ram();	
    if(type==0)
	{
        DealResetIC();
        return;
    }
    else if(type==3)
    {
        ddi_iccpsam_poweroff(EMVCARD_NO);
        lcd_interface_clear_ram();
        lcd_interface_fill_rowram(0,     0, "下电完成", FDISP|CDISP);
        lcd_interface_brush_screen();
        dev_delay_ms(1000);
        return;
    }
    lcd_interface_clear_ram();
    lcd_interface_fill_rowram(0,     0, "正在操作卡...", FDISP|CDISP);
    lcd_interface_brush_screen();
    if(1 == type)
	{   //选文件
        for(i=0; i<ic_PSETab1[0]; i++)
		{
            wbuf[i]=ic_PSETab1[i+1]; 
        }
        wlen = ic_PSETab1[0];
    }
    else if(2 == type)
	{
        for(i=0; i<ic_PSETab4[0]; i++)
		{
            wbuf[i] = ic_PSETab4[i+1];
        }
        wlen = ic_PSETab2[0];
    }
    ret =  ddi_iccpsam_exchange_apdu(EMVCARD_NO,  wbuf, wlen, tmp, &len,500);//iccard_send_command(EMVCARD_NO, wbuf, wlen, tmp, &len);
    if(ret != DDI_OK)
	{//操作失败 
        lcd_interface_clear_ram();	
        lcd_interface_fill_rowram(1,     0, "操作IC卡失败", FDISP|CDISP);
        lcd_interface_brush_screen(); 
    }
    else
	{               //操作成功
        sw = 0;
        sw = tmp[len-2]<<8 | tmp[len-1];
        len -= 2; 
        lcd_interface_clear_rowram(0);
        lcd_interface_clear_rowram(1);
        lcd_interface_fill_rowram(0,     0, "操作IC卡成功", FDISP|CDISP);
        sprintf(buf, "sw:%04x", sw);
        lcd_interface_fill_rowram(1,     0, buf, FDISP); 
        DisplayTxt(2, 4, tmp, len);  
    }
    dev_delay_ms(1000); 
}
/***********************************************
    LOOPBACK处理入口
***********************************************/
//EMVDeal状态定义
#define EMVDEAL_STATUS_OUT                          0
#define EMVDEAL_STATUS_QUERY                        1
#define EMVDEAL_STATUS_NOCARD                       2 
#define EMVDEAL_STATUS_HAVECARD                     3   
#define EMVDEAL_STATUS_RESET                        4
#define EMVDEAL_STATUS_OPERATE                      5
#define EMVDEAL_STATUS_COMMERR                      -1  //
/*
#define ELECTRICSIGLE  	'1'//单次触发模式
#define ELECTRICTIMED   '2'//循环触发模式
#define PROTOCOLTEST	'3'//协议测试
#define PROTOCOLTEST1   '4'//循环协议测试
*/
void EMVDeal(void)
{
    u8  tmp[300];
    int ret;
 //   u16 len;
    int status;
//    int step=0;
    int flag;
    int disflushflg;  //显示刷新标志
    int timecnt=0; //多少个单位时间
    int timenum=1; //决定定时单位(使用定时器的时候会*100),故定时单位为100*timenum毫秒
    int timerID; 
    int queryflg=1; //是否查询卡状态寄存器 
    U8 key;
    u8 cmd;
 //   u8 count = 0;  
    flag = 1;

    while(flag)
    {
        if(queryflg)
        {
            queryflg=0;   
            //查询IC卡
            ret = dev_icc_getstatus(EMVCARD_NO);//iccard_query_state(EMVCARD_NO, tmp, &len); 
            if(ret >= 0)
            {      
                if(ICC_NO_EXIST != ret)   //说明有卡   
                {                       
                    status=EMVDEAL_STATUS_HAVECARD;  
                    if(EMVTestMode == ELECTRICTIMED)  //'2'//循环触发模式
                    {
                        timecnt=(electric_cycle*10);//40;  //electric_cycle秒后继续 
                        timenum=1;
                        timerID = dev_user_gettimeID();

                    }
                    else if(EMVTestMode == PROTOCOLTEST1)  //'4'//循环协议测试
                    {  
                        timecnt=(protocol_cycle*10);  //protocol_cycle秒后继续   //Yana 协议循环周期
                        timenum=1;
                        timerID = dev_user_gettimeID();
                    }
                    else
                    {
                        timecnt=0;  //按确认继续
                    }
                }
                else 
                {          //说明没卡
                    status=EMVDEAL_STATUS_NOCARD;
                }
            }
            else
            {               //与51通讯失败
                status = EMVDEAL_STATUS_COMMERR;
            }
            
            disflushflg=1; 
        }
        if(disflushflg)
        {          
            disflushflg=0;           
TRACE("status=%d\r\n", status);
            lcd_interface_clear_ram();	
            switch(status)
            {
                case EMVDEAL_STATUS_OUT:
                    lcd_interface_fill_rowram(2,     0, "IC卡被拔出", FDISP|CDISP);
                    TRACE("IC卡被拔出");
                    break;
                case EMVDEAL_STATUS_NOCARD:     //没有卡							//清屏幕显示
                    lcd_interface_fill_rowram(2,     0, "请插入IC卡", FDISP|CDISP);
                    lcd_interface_fill_rowram(3,     0, "按取消返回模式选择", FDISP|CDISP);
                    TRACE("请插入IC卡,按取消返回模式选择");
                    break;
                case EMVDEAL_STATUS_HAVECARD:     //有卡          	
                    lcd_interface_fill_rowram(2,     0, "有IC卡插入", FDISP|CDISP);
                    TRACE("有IC卡插入");
                    if((EMVTestMode==ELECTRICTIMED) || (EMVTestMode == PROTOCOLTEST1))
                    {
                        //'2'//循环触发模式             //'4'//循环协议测试 
                        sprintf(tmp, "%d秒后继续", (timecnt+9)/10);
                        lcd_interface_fill_rowram(3,     0, tmp, FDISP|CDISP);
                        lcd_interface_fill_rowram(4,     0, "按取消返回模式选择", FDISP|CDISP);
                        TRACE("%s",tmp);
                        TRACE("按取消返回模式选择");
                    }
                    else
                    {  //????
                        lcd_interface_fill_rowram(3,     0, "按确认继续", FDISP|CDISP);
                        lcd_interface_fill_rowram(4,     0, "按取消返回模式选择", FDISP|CDISP);
                        TRACE("按确认继续,按取消返回模式选择");
                    }
                    break;
                case EMVDEAL_STATUS_RESET:     
                    lcd_interface_fill_rowram(2,     0, "正在复位IC卡...", FDISP|CDISP);
                    TRACE("正在复位IC卡...");
                    break;
                case EMVDEAL_STATUS_COMMERR:    //与51通讯失败
                    lcd_interface_fill_rowram(2,     0, "协处理器通讯失败", FDISP|CDISP);
                    lcd_interface_fill_rowram(3,     0, "按确认继续", FDISP|CDISP);
                    lcd_interface_fill_rowram(4,     0, "按取消返回模式选择", FDISP|CDISP);
                    TRACE("协处理器通讯失败,按确认继续,按取消返回模式选择");
    				#if 0
                    key = 0;
                    while(1)
                    {
                    	key = key_get_beep(NULL);
                    	if(key)
                    	{
                    		break;
                    	}
                    }
                    #endif
                    break;
            }
            lcd_interface_brush_screen();
            if(status!=EMVDEAL_STATUS_HAVECARD)
            {
            	keypad_interface_clear();
            }
        }
        //处理操作         
        switch(status)
        {
            case EMVDEAL_STATUS_RESET:
                ret = DealResetIC1();//iccard_reset(EMVCARD_NO, tmp, &len);
                if(ret >= 0)
                {
                    //ddi_iccpsam_poweroff(EMVCARD_NO);//qdy debug    
                    DealOperateIC();
                    queryflg=1;
                    #if 0
                    count ++;
                 	//if(count == 
                    if(count % 16 == 0)
                    {
                    	dev_lcd_close();
                    	Delay_100us(100);
                    	dev_lcd_open();
                    	Delay_100us(100);
                    }
                    #endif
                }
                else
                {//复位失败，重新查询IC卡
                    queryflg=1; 
                }
                break; 
        }

	    ret = keypad_interface_read_beep(&key);
	    if(ret>0)
	    {
	    	//printf("press key-y is %x\r\n",key);
	        switch(key)
    	    {
    	        case ESC:	//ESC  huchaoyan 2015-4-3
    	        
    	        	TRACE("ESC key is press");
    	            if((status == EMVDEAL_STATUS_OUT) || (status == EMVDEAL_STATUS_NOCARD)||
    	               (status == EMVDEAL_STATUS_HAVECARD))
    	            {
                        ddi_iccpsam_poweroff(EMVCARD_NO);
    	                return;
    	            }
                 	else if(status == EMVDEAL_STATUS_COMMERR)
                 	{
                        ddi_iccpsam_poweroff(EMVCARD_NO);
                    	return;
                 	}
    	            break;
    	        case ENTER:
                    TRACE("ENTER key is press");
                    if((EMVTestMode == ELECTRICSIGLE)||(EMVTestMode == PROTOCOLTEST))
                    {//'1'//循环触发模式 '3'//循环协议
                        if(status == EMVDEAL_STATUS_HAVECARD)
                        {
                            status = EMVDEAL_STATUS_RESET;    
                        }
                    }
                    else if(status == EMVDEAL_STATUS_COMMERR)
                    {
                    		queryflg=1;
                    }
    	            break;
    	    }
	    }
	    //处理卡插入，拔出
	    cmd = dev_icc_getstatus(EMVCARD_NO);//iccard_read_report(tmp, &len);

	    if(cmd != ICC_NO_EXIST)   //有卡插入
	    {   
	        if((status == EMVDEAL_STATUS_OUT) || (status == EMVDEAL_STATUS_NOCARD))
	        {
	            status = EMVDEAL_STATUS_HAVECARD;
                if(EMVTestMode == ELECTRICTIMED)  //'2'//循环触发模式
                {
                    timecnt=40;  //4秒后继续
                    timenum=1;
                    timerID = dev_user_gettimeID();
                    //Timeflg=1;
                }
                else if(EMVTestMode == PROTOCOLTEST1)   //'4'//循环协议测试
                {  
                    timecnt=10;  //1秒后继续   //Yana 协议循环周期
                    timenum=1;
                    timerID = dev_user_gettimeID();
                    //Timeflg=1;
                }
                else
                {
                    timecnt=0;  //按确认继续
                }   
                disflushflg = 1; 
                keypad_interface_clear();
	        }
	    } 
	    else //if(cmd == DDRAWICCARD)  //有卡拔出
	    {  
	        if((status == EMVDEAL_STATUS_HAVECARD))
	        {//(status == EMVDEAL_STATUS_NOCARD)||
	            status = EMVDEAL_STATUS_OUT;
                disflushflg=1;
                timecnt=20;   //2秒后
                timenum=1;
                timerID = dev_user_gettimeID();
                //Timeflg=1;
            }
	    }
	    //处理超时

        if(timecnt!=0)
        {
            if(dev_user_querrytimer(timerID, 100*timenum)==1)
            { //500ms超时
                timenum++;
                timecnt--; 
                if(timecnt==0)
                {
                    if(status ==  EMVDEAL_STATUS_OUT)
                    {//卡被拔出
                        status = EMVDEAL_STATUS_NOCARD;  //没有卡
                        disflushflg = 1;
                    }
                    else if(status == EMVDEAL_STATUS_HAVECARD)
                    {
                        status = EMVDEAL_STATUS_RESET;    
						//disflushflg = 1;      //
                    }
                }
                if((status ==  EMVDEAL_STATUS_HAVECARD) && (0 == (timecnt%10)))
                {   
                    disflushflg = 1;
                }
            }
        }    
    }    
}
/*******************************************************************
********************************************************************/
int emvl1_main(void)
{
    u8 key;
    int flag = 1;
	s32 ret = 0;
 	u8  tmp[20]={0};
    u16 len;

    dev_lcd_open();
    posdev_keypad_open();
	ddi_audio_open();
    ddi_iccpsam_open(EMVCARD_NO);
    ddi_led_open();
    
    lcd_interface_bl_ctl(1);
dev_debug_printf("%s(%d):%s %s\r\n", __FUNCTION__, __LINE__, __DATE__, __TIME__);    
    while(1) 
    {
	    if(flag == 1)
	    {
	        flag = 0;
	        lcd_interface_clear_ram();								//清屏幕显示
	        lcd_interface_fill_rowram(0, 0, "EMV L1(20161124)", NOFDISP|CDISP);
          #if 0
            //lcd_interface_fill_rowram(1, 0, "1.单次触发", FDISP);
	        //lcd_interface_fill_rowram(1, MAXCOL>>1, "2.循环触发", FDISP);
	        lcd_interface_fill_rowram(2, 0, "3.单次协议", FDISP);
	        lcd_interface_fill_rowram(2, MAXCOL>>1, "4.循环协议", FDISP);
	        lcd_interface_fill_rowram(3, 0, "5.复位", FDISP);
	        lcd_interface_fill_rowram(3, MAXCOL>>1, "6.选择PSE", FDISP);
	        lcd_interface_fill_rowram(4, 0, "7.读记录", FDISP);
	        lcd_interface_fill_rowram(4, MAXCOL>>1, "8.下电", FDISP);
          #else
            lcd_interface_fill_rowram(1, 0, "1.单次协议", FDISP);
	        lcd_interface_fill_rowram(2, 0, "2.循环协议", FDISP);
	        lcd_interface_fill_rowram(3, 0, "0.时间设置", FDISP);
          #endif
	        lcd_interface_brush_screen();
	     	//KeyFlush();
	        keypad_interface_clear();
	    }  
        ret = keypad_interface_read_beep(&key);
        if(ret>0)
        {
          #if 0  
        	if(key == DIGITAL1)
        	{ 
                TRACE("1.单次触发");
        	    EMVTestMode = ELECTRICSIGLE;
        	    EMVDeal();
                flag = 1;
            }
            else if(key == DIGITAL2)
            {
                TRACE("2.循环触发");
        	    EMVTestMode = ELECTRICTIMED;
        	    EMVDeal();
                flag = 1;
            }
            else if(key == DIGITAL3)
    		{
                TRACE("3.单次协议");
        	    EMVTestMode = PROTOCOLTEST;
        	    EMVDeal();
                flag = 1;
            }
            else if(key == DIGITAL4)
            { 
                TRACE("4.循环协议");
        	    EMVTestMode = PROTOCOLTEST1;
        	    EMVDeal();
                flag = 1;
            }  
            else if(key == DIGITAL5)
            { 
                TRACE("5.复位");
        	    EMVTestMode = MODERESET;
        	    DealSingleMode(0);
                flag = 1;
            }
            else if(key == DIGITAL6)
            { 
                TRACE("6.选择PSE");
        	    EMVTestMode = MODESELECTPSE;
        	    DealSingleMode(1);
                flag = 1;
            }
            else if(key == DIGITAL7)
            { 
                TRACE("7.读记录");
        	    EMVTestMode = MODEREADREC;
        	    DealSingleMode(2);
                flag = 1;
            }
            else if(key == DIGITAL8)
            { 
                TRACE("8.下电");
        	    EMVTestMode = MODEDEACTIVATE;
        	    DealSingleMode(3);
                flag = 1;
            }
            else if(key == DIGITAL9)
            { 
                TRACE("9.查询是否有卡存在");
    			ret = dev_icc_getstatus(EMVCARD_NO); 
    			if(ret >= 0)
    			{	   
    				if(ICC_NO_EXIST != ret)   //说明有卡  
    				{
    					TRACE("有卡");
    				}
    				else
    				{
    					TRACE("无卡");
    				}
    			}
            }
            else if(key == DIGITAL0)
            { 
                TRACE("0.设置电器、协议循环时间"); 
                DealOthersFunction();
                flag = 1;
            }   
           #else
            if(key == DIGITAL1)
    		{
                TRACE("1.单次协议");
        	    EMVTestMode = PROTOCOLTEST;
        	    EMVDeal();
                flag = 1;
            }
            else if(key == DIGITAL2)
            { 
                TRACE("2.循环协议");
        	    EMVTestMode = PROTOCOLTEST1;
        	    EMVDeal();
                flag = 1;
            }  
            else if(key == DIGITAL0)
            { 
                TRACE("0.设置电器、协议循环时间"); 
                //DealOthersFunction();
                SetResetWaitTime(2);
                flag = 1;
            }   
           #endif
          /*  
            else if(key == ESC)
            {
                TRACE("out");
                break;
            }*/
            else if(key == POWER)
            {
                while(1)
                {
                    //判断松键
                    drv_power_check_powerkey(&key);
                    if(key==0)
                    {
                        lcd_interface_bl_ctl(0);
                        lcd_interface_clear_ram();
                        lcd_interface_brush_screen();
                        ddi_misc_poweroff();
                        break;
                    }
                }
                while(1)
                {
                    //有外电时程序关不掉,等待按键按下重启
		            ret = drv_power_check_powerkey(&key);
                    if(key == 1)
                    {
                        dev_delay_ms(200);
                        ret = drv_power_check_powerkey(&key);
                        if(key == 1)
                        {
                            ddi_misc_reboot();
                        }
                    }
                }
            }
        }
    }

    ddi_iccpsam_close(EMVCARD_NO);
} 



