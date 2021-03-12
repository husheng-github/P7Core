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
	ICC_NO_EXIST = 0,    //�޿�
	ICC_IN_POWEROFF,     //�п�δ�ϵ�&�Ѿ��µ�      
	ICC_IN_POWERON,      //�п�&���ϵ�
};

#define ELECTRICSIGLE  	'1'//���δ���ģʽ
#define ELECTRICTIMED   '2'//ѭ������ģʽ
#define PROTOCOLTEST	'3'//Э�����
#define PROTOCOLTEST1   '4'//ѭ��Э�����
#define MODERESET       '5'//�⸴λӦ��
#define MODESELECTPSE   '6'//��ѡPSE
#define MODEREADREC     '7'//�����¼
#define MODEDEACTIVATE  '8'//���µ�

#define EMVCARD_NO      ICCARD_NO1
#define LCDROW_MAX       MAXROW//5
#define LCDLINEBYTE_MAX  21 

#define	ELECTRIC_CYCLE_DEF	5
#define	PROTOCOL_CYCLE_DEF	5

int electric_cycle = ELECTRIC_CYCLE_DEF;       //��������ѭ��ʱ��
int protocol_cycle = PROTOCOL_CYCLE_DEF;       //Э�����ѭ��ʱ��
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
            lcd_interface_clear_ram();								//����Ļ��ʾ
            lcd_interface_fill_rowram(0, 0, "����ѭ������ʱ��", NOFDISP|CDISP);
          #if 0  
            lcd_interface_fill_rowram(1, 0, "1. 1��  2. 3��  ", FDISP);
            lcd_interface_fill_rowram(2, 0, "3. 5��  4. 10�� ", FDISP);
            lcd_interface_fill_rowram(3, 0, "5. 30�� 6. 35�� ", FDISP);
          #else
            lcd_interface_fill_rowram(1, 0, "1. 5�� ", FDISP);
            lcd_interface_fill_rowram(2, 0, "2. 30��", FDISP);
            
          #endif  
		    sprintf(tmp, "��ǰ�趨:%2d��", value);
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
            lcd_interface_clear_ram();								//����Ļ��ʾ
            lcd_interface_fill_rowram(0, 0, "��������", FDISP|CDISP);
            lcd_interface_fill_rowram(1, 0, "1.���õ���ѭ��ʱ��", FDISP);
			lcd_interface_fill_rowram(2, 0, "2.����Э��ѭ��ʱ��", FDISP);
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
    ����IC��λ
***********************************************/
s32 DealResetIC(void)
{
    u8 tmp[200]={0},atr[200]={0};
    u16 len;
    s32 ret; 
    //if(EMVTestMode==MODERESET){
    lcd_interface_clear_ram();	
    lcd_interface_fill_rowram(0,     0, "���ڸ�λ...", FDISP|CDISP);
    lcd_interface_brush_screen();
    //}
    ret = ddi_iccpsam_poweron(EMVCARD_NO,tmp);//iccard_reset(EMVCARD_NO, tmp, &len);
    if(DDI_OK !=ret){ 
        lcd_interface_clear_ram();	
        lcd_interface_fill_rowram(0,     0, "��λʧ��", FDISP|CDISP);
        lcd_interface_brush_screen(); 
        //---->��ʱ1��
        dev_delay_ms(1000);
    }
    else
    {  
    	dev_maths_bcd_to_u16(&len, tmp, 1);
        memcpy(atr,&tmp[1],len);
        lcd_interface_clear_ram();	
        lcd_interface_fill_rowram(0,     0, "��λ�ɹ�", FDISP|CDISP);
        DisplayTxt(1, 4, atr, len);
        lcd_interface_brush_screen(); 
        if(EMVTestMode==MODERESET){ 
            //---->��ʱ1��
            dev_delay_ms(1000);
        }
    }
//    dev_key_flush();
    return ret;
}
/***********************************************
    ����IC��λ
***********************************************/
s32 DealResetIC1(void)
{
    u8 tmp[200];
 //   u16 len;
    s32 ret; 
    //if(EMVTestMode==MODERESET){
    lcd_interface_clear_ram();	
    lcd_interface_fill_rowram(0,     0, "���ڸ�λ...", FDISP|CDISP);
    lcd_interface_brush_screen();
    //}
    TRACE("���ڸ�λ...");
    ret = ddi_iccpsam_poweron(EMVCARD_NO,tmp);//iccard_reset(EMVCARD_NO, tmp, &len);
    if(DDI_OK != ret)
    { 
        lcd_interface_clear_ram();	
        lcd_interface_fill_rowram(0,     0, "��λʧ��", FDISP|CDISP);
        lcd_interface_brush_screen(); 
        //---->��ʱ1��
        dev_delay_ms(1000);
        TRACE("��λʧ��");
    }
    else
    {  
    	TRACE("��λ�ɹ�");	
        dev_delay_ms(500);
    }
//    dev_key_flush();
    return ret;
}
/**********************************************
    ����IC��
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
    lcd_interface_fill_rowram(0,     0, "���ڲ�����...", FDISP|CDISP);
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
    			//printf("ȡ��\r\n");  Yana
    			return;
    		}
        }
    #endif
		
        ret = ddi_iccpsam_exchange_apdu(EMVCARD_NO,  wbuf, wlen, tmp, &len,500);//iccard_send_command(EMVCARD_NO, wbuf, wlen, tmp, &len);
        if(ret != DDI_OK)
        {//����ʧ�� 
            TRACE("exchange apdu fail,return to upper func");
            lcd_interface_clear_ram();	
            lcd_interface_fill_rowram(1, 0, "����IC��ʧ��", FDISP|CDISP);
            lcd_interface_brush_screen(); 
            dev_delay_ms(1000);
            ddi_iccpsam_poweroff(EMVCARD_NO);//qdy add
            return;
        }
        else
        {   //�����ɹ�
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
            if((ELECTRICSIGLE==EMVTestMode)||(ELECTRICTIMED==EMVTestMode))//�������Բ���
            {
				if((0 == loopcnt) && (0x9000==sw) )
				{
					//����¼
					if(tmp[1] == 0x70)
					{   //������ɣ����µ�����
	                    flag = 0;
	                    //lcd_interface_clear_ram();	
	                    lcd_interface_clear_rowram(0);
	                    lcd_interface_fill_rowram(0, 0, "����IC���ɹ�", FDISP|CDISP);
	                    //dev_lcd_fill_row_ram(1,     0, "1����µ�", FDISP|CDISP);
	                    lcd_interface_brush_screen(); 
	                    dev_delay_ms(1000);
                        TRACE("����IC���ɹ�,�µ�");
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
                {   //������ɣ����µ�����
                    flag = 0;
                    //lcd_interface_clear_ram();	
                    lcd_interface_clear_rowram(0);
                    lcd_interface_fill_rowram(0,     0, "����IC���ɹ�", FDISP|CDISP);
                    //dev_lcd_fill_row_ram(1,     0, "1����µ�", FDISP|CDISP);
                    lcd_interface_brush_screen(); 
                    TRACE("����IC���ɹ�,�µ�");
                    dev_delay_ms(1000);
                    ddi_iccpsam_poweroff(EMVCARD_NO);
                    return;
                }
            }
            else//Э�����	//modify by Yana 140402
            {
				if(len<4)//if R-APDU<6 bytes     lenû�а���sw//Select APDU
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
						//�������,���µ�����
						TRACE("�µ�");
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
#0 MODERESET       '5'//�⸴λӦ��
#1 MODESELECTPSE   '6'//��ѡPSE
#2 MODEREADREC     '7'//�����¼
#3 MODEDEACTIVATE  '8'//���µ�
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
        lcd_interface_fill_rowram(0,     0, "�µ����", FDISP|CDISP);
        lcd_interface_brush_screen();
        dev_delay_ms(1000);
        return;
    }
    lcd_interface_clear_ram();
    lcd_interface_fill_rowram(0,     0, "���ڲ�����...", FDISP|CDISP);
    lcd_interface_brush_screen();
    if(1 == type)
	{   //ѡ�ļ�
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
	{//����ʧ�� 
        lcd_interface_clear_ram();	
        lcd_interface_fill_rowram(1,     0, "����IC��ʧ��", FDISP|CDISP);
        lcd_interface_brush_screen(); 
    }
    else
	{               //�����ɹ�
        sw = 0;
        sw = tmp[len-2]<<8 | tmp[len-1];
        len -= 2; 
        lcd_interface_clear_rowram(0);
        lcd_interface_clear_rowram(1);
        lcd_interface_fill_rowram(0,     0, "����IC���ɹ�", FDISP|CDISP);
        sprintf(buf, "sw:%04x", sw);
        lcd_interface_fill_rowram(1,     0, buf, FDISP); 
        DisplayTxt(2, 4, tmp, len);  
    }
    dev_delay_ms(1000); 
}
/***********************************************
    LOOPBACK�������
***********************************************/
//EMVDeal״̬����
#define EMVDEAL_STATUS_OUT                          0
#define EMVDEAL_STATUS_QUERY                        1
#define EMVDEAL_STATUS_NOCARD                       2 
#define EMVDEAL_STATUS_HAVECARD                     3   
#define EMVDEAL_STATUS_RESET                        4
#define EMVDEAL_STATUS_OPERATE                      5
#define EMVDEAL_STATUS_COMMERR                      -1  //
/*
#define ELECTRICSIGLE  	'1'//���δ���ģʽ
#define ELECTRICTIMED   '2'//ѭ������ģʽ
#define PROTOCOLTEST	'3'//Э�����
#define PROTOCOLTEST1   '4'//ѭ��Э�����
*/
void EMVDeal(void)
{
    u8  tmp[300];
    int ret;
 //   u16 len;
    int status;
//    int step=0;
    int flag;
    int disflushflg;  //��ʾˢ�±�־
    int timecnt=0; //���ٸ���λʱ��
    int timenum=1; //������ʱ��λ(ʹ�ö�ʱ����ʱ���*100),�ʶ�ʱ��λΪ100*timenum����
    int timerID; 
    int queryflg=1; //�Ƿ��ѯ��״̬�Ĵ��� 
    U8 key;
    u8 cmd;
 //   u8 count = 0;  
    flag = 1;

    while(flag)
    {
        if(queryflg)
        {
            queryflg=0;   
            //��ѯIC��
            ret = dev_icc_getstatus(EMVCARD_NO);//iccard_query_state(EMVCARD_NO, tmp, &len); 
            if(ret >= 0)
            {      
                if(ICC_NO_EXIST != ret)   //˵���п�   
                {                       
                    status=EMVDEAL_STATUS_HAVECARD;  
                    if(EMVTestMode == ELECTRICTIMED)  //'2'//ѭ������ģʽ
                    {
                        timecnt=(electric_cycle*10);//40;  //electric_cycle������ 
                        timenum=1;
                        timerID = dev_user_gettimeID();

                    }
                    else if(EMVTestMode == PROTOCOLTEST1)  //'4'//ѭ��Э�����
                    {  
                        timecnt=(protocol_cycle*10);  //protocol_cycle������   //Yana Э��ѭ������
                        timenum=1;
                        timerID = dev_user_gettimeID();
                    }
                    else
                    {
                        timecnt=0;  //��ȷ�ϼ���
                    }
                }
                else 
                {          //˵��û��
                    status=EMVDEAL_STATUS_NOCARD;
                }
            }
            else
            {               //��51ͨѶʧ��
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
                    lcd_interface_fill_rowram(2,     0, "IC�����γ�", FDISP|CDISP);
                    TRACE("IC�����γ�");
                    break;
                case EMVDEAL_STATUS_NOCARD:     //û�п�							//����Ļ��ʾ
                    lcd_interface_fill_rowram(2,     0, "�����IC��", FDISP|CDISP);
                    lcd_interface_fill_rowram(3,     0, "��ȡ������ģʽѡ��", FDISP|CDISP);
                    TRACE("�����IC��,��ȡ������ģʽѡ��");
                    break;
                case EMVDEAL_STATUS_HAVECARD:     //�п�          	
                    lcd_interface_fill_rowram(2,     0, "��IC������", FDISP|CDISP);
                    TRACE("��IC������");
                    if((EMVTestMode==ELECTRICTIMED) || (EMVTestMode == PROTOCOLTEST1))
                    {
                        //'2'//ѭ������ģʽ             //'4'//ѭ��Э����� 
                        sprintf(tmp, "%d������", (timecnt+9)/10);
                        lcd_interface_fill_rowram(3,     0, tmp, FDISP|CDISP);
                        lcd_interface_fill_rowram(4,     0, "��ȡ������ģʽѡ��", FDISP|CDISP);
                        TRACE("%s",tmp);
                        TRACE("��ȡ������ģʽѡ��");
                    }
                    else
                    {  //????
                        lcd_interface_fill_rowram(3,     0, "��ȷ�ϼ���", FDISP|CDISP);
                        lcd_interface_fill_rowram(4,     0, "��ȡ������ģʽѡ��", FDISP|CDISP);
                        TRACE("��ȷ�ϼ���,��ȡ������ģʽѡ��");
                    }
                    break;
                case EMVDEAL_STATUS_RESET:     
                    lcd_interface_fill_rowram(2,     0, "���ڸ�λIC��...", FDISP|CDISP);
                    TRACE("���ڸ�λIC��...");
                    break;
                case EMVDEAL_STATUS_COMMERR:    //��51ͨѶʧ��
                    lcd_interface_fill_rowram(2,     0, "Э������ͨѶʧ��", FDISP|CDISP);
                    lcd_interface_fill_rowram(3,     0, "��ȷ�ϼ���", FDISP|CDISP);
                    lcd_interface_fill_rowram(4,     0, "��ȡ������ģʽѡ��", FDISP|CDISP);
                    TRACE("Э������ͨѶʧ��,��ȷ�ϼ���,��ȡ������ģʽѡ��");
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
        //�������         
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
                {//��λʧ�ܣ����²�ѯIC��
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
                    {//'1'//ѭ������ģʽ '3'//ѭ��Э��
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
	    //�������룬�γ�
	    cmd = dev_icc_getstatus(EMVCARD_NO);//iccard_read_report(tmp, &len);

	    if(cmd != ICC_NO_EXIST)   //�п�����
	    {   
	        if((status == EMVDEAL_STATUS_OUT) || (status == EMVDEAL_STATUS_NOCARD))
	        {
	            status = EMVDEAL_STATUS_HAVECARD;
                if(EMVTestMode == ELECTRICTIMED)  //'2'//ѭ������ģʽ
                {
                    timecnt=40;  //4������
                    timenum=1;
                    timerID = dev_user_gettimeID();
                    //Timeflg=1;
                }
                else if(EMVTestMode == PROTOCOLTEST1)   //'4'//ѭ��Э�����
                {  
                    timecnt=10;  //1������   //Yana Э��ѭ������
                    timenum=1;
                    timerID = dev_user_gettimeID();
                    //Timeflg=1;
                }
                else
                {
                    timecnt=0;  //��ȷ�ϼ���
                }   
                disflushflg = 1; 
                keypad_interface_clear();
	        }
	    } 
	    else //if(cmd == DDRAWICCARD)  //�п��γ�
	    {  
	        if((status == EMVDEAL_STATUS_HAVECARD))
	        {//(status == EMVDEAL_STATUS_NOCARD)||
	            status = EMVDEAL_STATUS_OUT;
                disflushflg=1;
                timecnt=20;   //2���
                timenum=1;
                timerID = dev_user_gettimeID();
                //Timeflg=1;
            }
	    }
	    //����ʱ

        if(timecnt!=0)
        {
            if(dev_user_querrytimer(timerID, 100*timenum)==1)
            { //500ms��ʱ
                timenum++;
                timecnt--; 
                if(timecnt==0)
                {
                    if(status ==  EMVDEAL_STATUS_OUT)
                    {//�����γ�
                        status = EMVDEAL_STATUS_NOCARD;  //û�п�
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
	        lcd_interface_clear_ram();								//����Ļ��ʾ
	        lcd_interface_fill_rowram(0, 0, "EMV L1(20161124)", NOFDISP|CDISP);
          #if 0
            //lcd_interface_fill_rowram(1, 0, "1.���δ���", FDISP);
	        //lcd_interface_fill_rowram(1, MAXCOL>>1, "2.ѭ������", FDISP);
	        lcd_interface_fill_rowram(2, 0, "3.����Э��", FDISP);
	        lcd_interface_fill_rowram(2, MAXCOL>>1, "4.ѭ��Э��", FDISP);
	        lcd_interface_fill_rowram(3, 0, "5.��λ", FDISP);
	        lcd_interface_fill_rowram(3, MAXCOL>>1, "6.ѡ��PSE", FDISP);
	        lcd_interface_fill_rowram(4, 0, "7.����¼", FDISP);
	        lcd_interface_fill_rowram(4, MAXCOL>>1, "8.�µ�", FDISP);
          #else
            lcd_interface_fill_rowram(1, 0, "1.����Э��", FDISP);
	        lcd_interface_fill_rowram(2, 0, "2.ѭ��Э��", FDISP);
	        lcd_interface_fill_rowram(3, 0, "0.ʱ������", FDISP);
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
                TRACE("1.���δ���");
        	    EMVTestMode = ELECTRICSIGLE;
        	    EMVDeal();
                flag = 1;
            }
            else if(key == DIGITAL2)
            {
                TRACE("2.ѭ������");
        	    EMVTestMode = ELECTRICTIMED;
        	    EMVDeal();
                flag = 1;
            }
            else if(key == DIGITAL3)
    		{
                TRACE("3.����Э��");
        	    EMVTestMode = PROTOCOLTEST;
        	    EMVDeal();
                flag = 1;
            }
            else if(key == DIGITAL4)
            { 
                TRACE("4.ѭ��Э��");
        	    EMVTestMode = PROTOCOLTEST1;
        	    EMVDeal();
                flag = 1;
            }  
            else if(key == DIGITAL5)
            { 
                TRACE("5.��λ");
        	    EMVTestMode = MODERESET;
        	    DealSingleMode(0);
                flag = 1;
            }
            else if(key == DIGITAL6)
            { 
                TRACE("6.ѡ��PSE");
        	    EMVTestMode = MODESELECTPSE;
        	    DealSingleMode(1);
                flag = 1;
            }
            else if(key == DIGITAL7)
            { 
                TRACE("7.����¼");
        	    EMVTestMode = MODEREADREC;
        	    DealSingleMode(2);
                flag = 1;
            }
            else if(key == DIGITAL8)
            { 
                TRACE("8.�µ�");
        	    EMVTestMode = MODEDEACTIVATE;
        	    DealSingleMode(3);
                flag = 1;
            }
            else if(key == DIGITAL9)
            { 
                TRACE("9.��ѯ�Ƿ��п�����");
    			ret = dev_icc_getstatus(EMVCARD_NO); 
    			if(ret >= 0)
    			{	   
    				if(ICC_NO_EXIST != ret)   //˵���п�  
    				{
    					TRACE("�п�");
    				}
    				else
    				{
    					TRACE("�޿�");
    				}
    			}
            }
            else if(key == DIGITAL0)
            { 
                TRACE("0.���õ�����Э��ѭ��ʱ��"); 
                DealOthersFunction();
                flag = 1;
            }   
           #else
            if(key == DIGITAL1)
    		{
                TRACE("1.����Э��");
        	    EMVTestMode = PROTOCOLTEST;
        	    EMVDeal();
                flag = 1;
            }
            else if(key == DIGITAL2)
            { 
                TRACE("2.ѭ��Э��");
        	    EMVTestMode = PROTOCOLTEST1;
        	    EMVDeal();
                flag = 1;
            }  
            else if(key == DIGITAL0)
            { 
                TRACE("0.���õ�����Э��ѭ��ʱ��"); 
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
                    //�ж��ɼ�
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
                    //�����ʱ����ز���,�ȴ�������������
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



