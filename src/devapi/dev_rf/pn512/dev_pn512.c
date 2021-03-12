#include "devglobal.h"

#include "devapi/dev_rf/dev_rf_flow.h"
#include "devapi/dev_rf/dev_rf_io.h"
#include "devapi/dev_rf/pn512/pn512reg.h"
#include "devapi/dev_rf/pn512/dev_pn512.h"



extern u8  const PCDTypeASel[3];// = {0x93, 0x95, 0x97};
static u8 g_pn512_ver = 0;
u8 g_rf_special_B_flag = 0;  
u8 g_rf_emd_flg = 0;
static u8 g_rf_export = 0;
#define PN512_EXPORT_EN    0x018
#define PN512_NRSTPD_OFF     dev_rf_rst_ctl(0)  
#define PN512_NRSTPD_RST     dev_rf_rst_ctl(1)  

/*******************************************************************************
 ** Descriotions:   ����PN512��չGPIO��
 ** parameters:                                                              
 ** Returned value: 0:ʧ��
                    -1:�ɹ�  
                                                                                              
 ** Created By:     ��ѧ�� 
 ** Remarks:                                                                                  
*******************************************************************************/
void dev_pn512_export_init(void)
{
  #if 0  
    u8 reg;
    
    //����IO���
    reg = PN512_EXPORT_EN;
    dev_pn512_write_reg(JREG_TESTPINEN, reg);
    //�����0
 //   g_rf_export = 0;
    dev_pn512_write_reg(JREG_TESTPINVALUE, g_rf_export|0x80);
  #endif  
}
void dev_pn512_export_deinit(void)
{
  #if 0  
    u8 reg;
    
    //�����0
    g_rf_export = 0;
    dev_pn512_write_reg(JREG_TESTPINVALUE, g_rf_export|0x80);

    //����IO���
    reg = PN512_EXPORT_EN;
    dev_pn512_write_reg(JREG_TESTPINEN, reg);
  #endif  
}
void dev_pn512_export_ctl(u8 num, u8 flg)
{
  #if 0  
    if(flg==0)
    {
        g_rf_export &= ~(1<<num);
    }
    else
    {
        g_rf_export |= (1<<num);
    }
    dev_pn512_write_reg(JREG_TESTPINVALUE, g_rf_export|0x80);
  #endif  
}
/*******************************************************************************
 ** Descriotions:   ��PN512�汾
 ** parameters:                                                              
 ** Returned value: 0:ʧ��
                    -1:�ɹ�  
                                                                                              
 ** Created By:     ��ѧ�� 
 ** Remarks:                                                                                  
*******************************************************************************/
s32 dev_pn512_read_version(u8 *ver)
{
    u8 tmp[8];
    static u8 flg=1;

    if(flg)
    {
        flg = 0;
        PN512_NRSTPD_OFF;
        dev_user_delay_ms(1);
        PN512_NRSTPD_RST;
        dev_user_delay_ms(1);
    }
    dev_pn512_read_reg(JREG_VERSION, ver);  
    g_pn512_ver = *ver; 
dev_debug_printf("%s(%d):ver=%02X\r\n", __FUNCTION__, __LINE__, ver[0]);    
    if((*ver== 0x82)        //pn512�汾
     ||(*ver == 0x12))      //��ѶMH1608
    {
      #if 0  
        dev_pn512_write_reg(JREG_TRELOADHI, 0x55);
        dev_pn512_write_reg(JREG_TRELOADLO, 0xaa);
        
        dev_pn512_read_reg(JREG_TRELOADHI, &tmp[0]);
        dev_pn512_read_reg(JREG_TRELOADLO, &tmp[1]); 
dev_debug_printf("%s(%d):%02X,%02X\r\n", __FUNCTION__, __LINE__, tmp[0], tmp[1]);    
      #endif  
        return 0;
    }
    else
    {
        return -1;
    }
    
}
/*******************************************************************************
 ** Descriotions:   ��PN512�汾
 ** parameters:                                                              
 ** Returned value: 0:ʧ��
                    1:�ɹ�  
                                                                                              
 ** Created By:     ��ѧ�� 
 ** Remarks:                                                                                  
*******************************************************************************/
void dev_pn512_reset(s32 _ms)
{
    dev_pn512_write_reg(JREG_COMMAND, JCMD_SOFTRESET);
    dev_pn512_export_init();
//    delayms(_ms);
}

/*---------------------------------------
    //����CRCʹ��
    mask  : ��Ҫ���ƵĲ���
       bit0: ����TXCRC����λΪ1����ʾ��Ҫ���ƣ�Ϊ0����Ҫ����
       bit1: ����RXCRC, ��λΪ1����ʾ��Ҫ���ƣ�Ϊ0����Ҫ����
    mod   :ָ�����Ƶ�ģʽ��
       bit0: ����TXCRC�����mask��bit0Ϊ1�����λΪ1����TXCRC��Ϊ0����ر�TXCRC
                        ���mask��bit0Ϊ0���򲻴���
       bit1: ����RXCRC�����mask��bit1Ϊ1�����λΪ1����RXCRC��Ϊ0����ر�RXCRC
                        ���mask��bit1Ϊ0���򲻴���                 
---------------------------------------*/
s32 dev_pn512_ctl_crc(u8 mask, u8 mod)
{
    //TXCRC 
    if(mask&DEV_RF_TXCRC_MASK)
    {
        if(mod&DEV_RF_TXCRC_MASK)
        {
            dev_pn512_modify_reg(JREG_TXMODE, 1, JBIT_CRCEN);
        }
        else
        {
            dev_pn512_modify_reg(JREG_TXMODE, 0, JBIT_CRCEN);            
        }
    }
    //RXCRC
    if(mask&DEV_RF_RXCRC_MASK)
    {
        if(mod&DEV_RF_RXCRC_MASK)
        {
            dev_pn512_modify_reg(JREG_RXMODE, 1, JBIT_CRCEN);
        }
        else
        {
            dev_pn512_modify_reg(JREG_RXMODE, 0, JBIT_CRCEN);            
        }
    }
    return 0;
}
/*******************************************************************************
 ** Descriotions:   ����PCD����ģʽ
 ** parameters:     type:PICC_TYPEA:Type A
                         PICC_TYPEB:TypeB 
                         PICC_TYPEC: TypeC(FeliCa)  
                    confpara:������ز���
                    {����TypeB: cfgpara[0]<->JREG_MODGSP
                                cfgpara[1]<->JREG_GSN
                                cfgpara[2]<->JREG_GSNOFF
                    }                                              
 ** Returned value: 0: �ɹ�
                   ����:ʧ��                                                                           
 ** Created By:     ��ѧ��           
 ** Remarks:                                                                                 
*******************************************************************************/
s8 dev_pn512_select_type(u8 type)
{ 
    u8 regval;

    if(PICC_TYPEC == type)
    {
        dev_pn512_write_reg(JREG_CONTROL, 0x10);
        dev_pn512_write_reg(JREG_TXMODE, 0x92);
        dev_pn512_write_reg(JREG_RXMODE, 0x96);

        dev_pn512_write_reg(JREG_RXTRESHOLD, 0x55);   ///MinLevel = 5; CollLevel = 5
        dev_pn512_write_reg(JREG_DEMOD, 0x61);

        dev_pn512_write_reg(JREG_RFCFG, 0x59);  
        dev_pn512_write_reg(JREG_GSNON, 0xff);
        dev_pn512_write_reg(JREG_CWGSP, 0x3f);          // ���������Ƶ��

        dev_pn512_write_reg(JREG_MODGSP, 0x0F);     // 10%ASK
        dev_pn512_write_reg(JREG_TXAUTO, 0x07);
        dev_pn512_read_reg(JREG_TXAUTO, &regval);
        //if(regval != 0x37)
        //{
        //    return MI_INITERR;                      //��ʼ������
        //}
    }
    else
    {
        dev_pn512_write_reg(JREG_CONTROL, 0x10);    /*Initator*/
        dev_pn512_write_reg(JREG_MIFNFC, 0x62);
        //dev_pn512_read_reg(JREG_TXAUTO, &regval);
        //dev_pn512_write_reg(JREG_TXBITPHASE, ( regval&0x80)|0x0f);
        dev_pn512_write_reg(JREG_GSNON, 0xf8);//0xff);
        dev_pn512_write_reg(JREG_MODE, 0x3f);       /*ModeDetOff*/
        dev_pn512_write_reg(JREG_GSNOFF, 0x00);
        dev_pn512_write_reg(JREG_AUTOTEST, 0x40);

        if(PICC_TYPEA == type)
        {
            dev_pn512_write_reg(JREG_TXAUTO, 0x40);     /*ǿ��100%ASK*/
            dev_pn512_write_reg(JREG_TXMODE, 0x00); // TypeA baud 106kbps
            dev_pn512_write_reg(JREG_RXMODE, 0x08); // ����4λ�Ľ�����Ч��TypeA baud 106kbps
            dev_pn512_write_reg(JREG_DEMOD, 0x4D);
            dev_pn512_write_reg(JREG_CWGSP, 0x3f);//0x20);//0x3f);      // ���������Ƶ��
            dev_pn512_write_reg(JREG_RXTRESHOLD, g_rf_param.rxthreshold_a);//0x84); /* MinLevel = 5; CollLevel = 5 */
            dev_pn512_write_reg(JREG_RFCFG, g_rf_param.rfcfgregA);//0x48);       /* RxGain = 6*/
            dev_pn512_write_reg(JREG_TYPEB, 0x00);
            dev_pn512_read_reg(JREG_RXSEL,&regval);
            dev_pn512_write_reg(JREG_RXSEL,(regval&0xC0)|0x08);
            dev_pn512_read_reg(JREG_TXAUTO, &regval);

            if(regval != 0x40)
            {                
                return MI_INITERR;                      //��ʼ������
            }
        }
        else if(PICC_TYPEB == type)
        {
            dev_pn512_write_reg(JREG_TXAUTO, 0x00);     /*ȡ��100%ASK*/
            dev_pn512_write_reg(JREG_TXMODE, 0x03);     // TypeB baud 106kbps
            dev_pn512_write_reg(JREG_RXMODE, 0x0b); // ����4λ�Ľ�����Ч��TypeB baud 106kbps
            dev_pn512_write_reg(JREG_TYPEB, 0xC0);  //0x1f); // // ������ҪSOF��EOF,����С���ȵ�SOF��EOF,����SOF��EOF,EGT=0

            dev_pn512_write_reg(JREG_DEMOD, 0x4d);
            dev_pn512_write_reg(JREG_CWGSP, 0x3f);      // ���������Ƶ��
            // JREG_MODGSP: 0--0x1f B�Ϳ��źŵ�����ȣ���Ҫ�ĳɽ�������ɵ�����
            dev_pn512_write_reg(JREG_MODGSP, g_rf_param.rfmodgsp);
            dev_pn512_write_reg(JREG_RXTRESHOLD, g_rf_param.rxthreshold_b);//0x55); /* MinLevel = 5; CollLevel = 5 */
            dev_pn512_write_reg(JREG_RFCFG, g_rf_param.rfcfgregB);//0x68);       /* RxGain = 7*/

            dev_pn512_read_reg(JREG_RXSEL,&regval);
            dev_pn512_write_reg(JREG_RXSEL,(regval&0xC0)|0x09);

            dev_pn512_read_reg(JREG_MODE, &regval);
            dev_pn512_write_reg(JREG_MODE, (regval&0xfc)|0x01);
            dev_pn512_modify_reg(JREG_RXMODE, 1, JBIT_CRCEN);
            dev_pn512_modify_reg(JREG_TXMODE, 1, JBIT_CRCEN);
        }
        /* Activate the field  */
        dev_pn512_modify_reg(JREG_TXCONTROL, 1, JBIT_TX2RFEN | JBIT_TX1RFEN);
        /* Clear the status flag afterwards */
        dev_pn512_write_reg(JREG_COMMIRQ, JBIT_TIMERI);
        dev_pn512_write_reg(JREG_TPRESCALER, 0xa6-1);
        dev_pn512_write_reg(JREG_TMODE, 0x82);
    }
    /* Activate receiver for communication
    The RcvOff bit and the PowerDown bit are cleared, the command is not changed. */
    dev_pn512_write_reg(JREG_COMMAND, JCMD_IDLE);
    dev_pn512_write_reg(JREG_WATERLEVEL, DEV_PN512_WATERLEVEL);       //������Ϊ32�ֽ�
    regval = dev_rf_get_chiptype();
    if(regval == PCD_CHIP_TYPE_MH1608)
    {
      #if 0
        /*//��EMV ʱ������
        {
            write_reg(0x37, 0x5E);
            write_reg(0x3B, 0xFD);
            //write_reg(0x35, 0xE8); // Auto AGC
            write_reg(0x35, 0x08); // Manul AGC
            write_reg(0x37, 0xAE);
            write_reg(0x3A, 0x33);//differ
            write_reg(0x37, 0x00);
        }
        */
        dev_pn512_write_reg(JREG_VERSION, 0x5E);//(0x37, 0x5E);
        dev_pn512_write_reg(JREG_TESTADC, 0xFD);//(0x3B, 0xFD)
        //dev_pn512_write_reg(JREG_TESTBUS, 0xE8);//(0x35, 0xE8); // Auto AGC
        dev_pn512_write_reg(JREG_TESTBUS, 0x08);//(0x35, 0x08); // Manul AGC
        dev_pn512_write_reg(JREG_TESTDAC2, 0x03);//(0x3A, 0x03); 
        dev_pn512_write_reg(JREG_VERSION, 0xAE);//(0x37, 0xAE);
        dev_pn512_write_reg(JREG_TESTDAC2, 0x33);//(0x3A, 0x33);//differ
        dev_pn512_write_reg(JREG_VERSION, 0x00);//(0x37, 0x00);
      #else
        /*//����ʱ������
        {
            write_reg(0x37, 0x5e);
            write_reg(0x3B, 0xE5); //differ
            write_reg(0x35, 0xED); // Auto AGC //�Ƿ�ʹ���Զ�agc����ʵ��ˢ��Ч��ѡ��
            //write_reg(0x35, 0x08); // Manul AGC
            write_reg(0x37, 0xAE);
            write_reg(0x3b, 0x72); //differ
            write_reg(0x37, 0x00);
        }
      
        */
        dev_pn512_write_reg(JREG_VERSION, 0x5E);//(0x37, 0x5E);
        dev_pn512_write_reg(JREG_TESTADC, 0xE5);//(0x3B, 0xE5) //differ
        dev_pn512_write_reg(JREG_TESTBUS, 0xED);//(0x35, 0xED); // Auto AGC
      #if 1  //20181221 ��Ѷŷ����������
        dev_pn512_write_reg(JREG_TESTDAC2, 0x00);//(0x3A, );  0x00��0x03  
      #endif  
        //dev_pn512_write_reg(JREG_TESTBUS, 0x08);//(0x35, 0x08); // Manul AGC
        dev_pn512_write_reg(JREG_VERSION, 0xAE);//(0x37, 0xAE);
        dev_pn512_write_reg(JREG_TESTDAC2, 0x72);//(0x3A, 0x72);//differ
        dev_pn512_write_reg(JREG_VERSION, 0x00);//(0x37, 0x00);

      #endif
    }
 //   delayms(6);    //5      //�ȴ������ȶ� //20090824_1
    return MI_OK;
  
}
/*******************************************************************************
 ** Descriotions:   ���ó�ʱʱ��
 ** parameters:     mode:��ʱʱ�䷽ʽ
                         0: 100us��ʱ
                         1: ��etuΪ��λ��ʱ
                         ��������
                    timer:��ʱֵ                                                         
 ** Returned value: ��                                                                         
 ** Created By:     ��ѧ�� 2009-6-30 15:32:05           
 ** Remarks:                                                                                  
*******************************************************************************/
void dev_pn512_set_timeout(u8 mode, u16 timer)
{
    u8 tprescaler, tmode;
    if(0 == timer)return;
    if((PCD_TIMER_MODE_ETU_0 == mode) || (PCD_TIMER_MODE_ETU_1 == mode))
    {
        tprescaler = 63;  //1etu = 64      
        tmode = 0;
    }
    else if((PCD_TIMER_MODE_nETU_0 == mode) || (PCD_TIMER_MODE_nETU_1 == mode))
    {       
        tprescaler = 0xbf;//35etu=8C0      
        tmode = 0x08;
    }
    else if((PCD_TIMER_MODE_nETU1_0 == mode) || (PCD_TIMER_MODE_nETU1_1 == mode))
    {
        tprescaler = 0xFF;//35etu=800      
        tmode = 0x07;
    }
    else 
    {                               //100us :  678
        tprescaler = 0xa5;          //677 = 0x2a5
        tmode = 0x02;
    }
    if(0 != (mode&0x01))
    {
        tmode |= 0x80;
    }

    dev_pn512_write_reg(JREG_TPRESCALER, tprescaler);
    dev_pn512_write_reg(JREG_TMODE, tmode);//82
    dev_pn512_write_reg(JREG_TRELOADLO, timer&0xFF);
    dev_pn512_write_reg(JREG_TRELOADHI, (timer>>8)&0xFF); 
}
/*******************************************************************************
 ** Descriotions:   ���ó�ʱʱ��
 ** parameters:     mode:��ʱʱ�䷽ʽ
                         0: 100us��ʱ
                         1: ��etuΪ��λ��ʱ
                         ��������
                    timer:��ʱֵ                                                         
 ** Returned value: ��                                                                         
 ** Created By:     ��ѧ�� 2009-6-30 15:32:05           
 ** Remarks:                                                                                  
*******************************************************************************/
void dev_pn512_set_timeout_etu(s32 etu)
{
    u8 tprescaler, tmode;
    u32 tprescal;
    u32 treload;
    u32 tmp;
    u8 nByte;
    
    if(0 == etu)return;
    dev_pn512_modify_reg(JREG_CONTROL, 1, JBIT_TSTOPNOW);//�رն�ʱ��
    tmp = etu*128;
    tprescal = tmp<65536? 0: ((tmp>>16)-1)>>1;
    do{
        treload = tmp-1;
        //do_div(treload, (tprescal<<1)+1);
        treload /= ((tprescal<<1)+1);
        if(treload >65535)tprescal++;
    }while(treload > 65535);

    dev_pn512_read_reg(JREG_TMODE, &nByte);
    nByte &= 0xf0;
    nByte |= (tprescal>>8);
    
    dev_pn512_write_reg(JREG_TPRESCALER, tprescal & 0xFF);
    dev_pn512_write_reg(JREG_TMODE, nByte);//82
    dev_pn512_write_reg(JREG_TRELOADLO, treload&0xFF);
    dev_pn512_write_reg(JREG_TRELOADHI, (treload>>8)&0xFF); 
}

void dev_pn512_wait_etu(u32 etu)
{
    u8 regval;
    u32 id;
    //��������жϱ�־
    dev_pn512_write_reg(JREG_COMMIRQ, 0x7F);
    dev_pn512_write_reg(JREG_DIVIRQ,  0x1F);
    dev_pn512_read_reg(JREG_COMMIEN, &regval);
    dev_pn512_write_reg(JREG_COMMIEN, regval|JBIT_TIMERI);//ʹ�ܶ�ʱ���ж�
    dev_pn512_set_timeout_etu(etu);
     //������ʱ��
    dev_pn512_modify_reg(JREG_CONTROL, 1, JBIT_TSTARTNOW);
    id = dev_get_timeID();
    while(1)
    {
        dev_pn512_read_reg(JREG_COMMIRQ, &regval);
        if(regval&JBIT_TIMERI)
        {

            break;
        }
        if(dev_querry_time(id, 20000))   //20s
        {   

            break;
        }
        
        
    }
    //�رն�ʱ��
    dev_pn512_modify_reg(JREG_CONTROL, 1, JBIT_TSTOPNOW);
    //��ֹ��ʱ���жϣ������ʱ���жϱ�־ 
    dev_pn512_modify_reg(JREG_COMMIEN, 0, JBIT_TIMERI);
    dev_pn512_write_reg(JREG_COMMIRQ, JBIT_TIMERI);
    
}
/*******************************************************************************
 ** Descriotions:  ��Ƭ���� 
 ** parameters:                                                           
 ** Returned value: ��                                                                         
 ** Created By:     ��ѧ�� 2009-6-30 15:32:05           
 ** Remarks:                                                                                  
*******************************************************************************/
s8 dev_hm1608_execute_cmd(MfCmdInfo *p_mfcmdinfo, u8 *wbuf, u8 *rbuf)
{
    //s32 i;
    s32 timerid;
    s32 timerover;
    s32 buflen=0;
    u8* buf;//[300];
    u16 wcnt;
    u8 waitForDiv;
    u8 regval;
    u8 commirq_reg;
    u8 err_reg;
    u8 errcnt;
    //    u8 err1cnt;
    s8 status = MI_OK;
    s8 statusbak=MI_OK;
    u8 commIrqEn = 0;
    //    u8 doReceive = 0;         //������ձ�־
    u8 waitForComm = (JBIT_ERRI | JBIT_TXI);
    u8 nbitreceived=0;
    u8 emdretry=MI_OK;
    u32 timeoutcnttmp;
    //u32 timeouttmp;

    //=============================================    
    waitForDiv = 0;
    errcnt = 0;   
    /*remove all Interrupt request flags that are used during function,
    keep all other like they are
    ��������жϱ�־
    */   
    dev_pn512_write_reg(JREG_COMMIRQ, 0x7F);
    dev_pn512_write_reg(JREG_DIVIRQ,  0x1F);
    //ȡ������
    dev_pn512_read_reg(JREG_COMMAND, &regval);
    regval = (regval & (~JMASK_COMMAND));
    dev_pn512_write_reg(JREG_COMMAND, regval);

    buf =(u8*)k_malloc(300);
    if(buf == NULL)
    {
        dev_debug_printf("�����ڴ�ʧ��\r\n");
        return MI_UNKNOWN_COMMAND;
    }
    timeoutcnttmp = p_mfcmdinfo->timeoutcnt;
    //timeouttmp = p_mfcmdinfo->timeout;
    //-----------------------------------------------
    //����ָ�������
    switch(p_mfcmdinfo->cmd)
    {
        case JCMD_IDLE:         //0x00 
        case JCMD_CALCCRC:      //0x03 
        case JCMD_SOFTRESET:    //0x0F 
            waitForComm = 0;
            waitForDiv  = 0;
            break;
        //---------------------------------------
        case JCMD_CONFIG:       //0x01
            commIrqEn = JBIT_IDLEI;
            waitForComm = JBIT_IDLEI;

            break;
        //---------------------------------------
        case JCMD_TRANSMIT:     //0x04 
            commIrqEn = JBIT_TXI | JBIT_TIMERI;
            waitForComm = JBIT_TXI;
            break;
        //---------------------------------------
        case JCMD_RECEIVE:      //0x08 
            commIrqEn = JBIT_RXI | JBIT_TIMERI;     //| JBIT_ERRI;
            waitForComm = JBIT_RXI | JBIT_TIMERI;   //| JBIT_ERRI;
            //            doReceive = 1;
            break;
        //---------------------------------------
        case JCMD_TRANSCEIVE:   //0x0C 
            //regval = (regval & (~JMASK_COMMAND)) | JCMD_TRANSCEIVE;
            //���õȴ�MASK
            commIrqEn   = JBIT_RXI | JBIT_IDLEI| JBIT_LOALERTI | JBIT_ERRI | JBIT_TIMERI | JBIT_TXI; //JBIT_RXI | JBIT_TIMERI;  //| JBIT_ERRI;
            waitForComm = JBIT_RXI | JBIT_IDLEI| JBIT_LOALERTI | JBIT_ERRI | JBIT_TIMERI | JBIT_TXI; //���е��ж��źŴ���
            break;
        //---------------------------------------
        case JCMD_AUTHENT:      //0x0E 
            commIrqEn = JBIT_IDLEI | JBIT_TIMERI;   //| JBIT_ERRI;
            waitForComm = JBIT_IDLEI | JBIT_TIMERI; //| JBIT_ERRI;
            break;
        //---------------------------------------
        case JCMD_GENRANDOM:    //0x02
        case JCMD_NOCMDCHANGE:  //0x07 
        case JCMD_AUTOCOLL:     //0x0d
        default:
            status = MI_UNKNOWN_COMMAND;
            break; 
    }
    //===============================================
    if(status != MI_OK)
    {
        //��������
        dev_pn512_read_reg(JREG_COMMAND, &regval);
        regval = (regval & (~JMASK_COMMAND)) | p_mfcmdinfo->cmd;
        dev_pn512_write_reg(JREG_COMMAND, regval);
    }
    else
    {
        //��FIFO
        dev_pn512_write_reg(JREG_FIFOLEVEL, JBIT_FLUSHBUFFER);
        regval = 64;                              //FIFOΪ64Bytes
        if(regval >= p_mfcmdinfo->nBytesToSend)
        {
            regval = p_mfcmdinfo->nBytesToSend;
            //�صͱ���
            commIrqEn &= (~JBIT_LOALERTI);
            waitForComm &= (~JBIT_LOALERTI);        //����1��дFIFO�Ϳ��Է�����
        }
        dev_pn512_write_fifo(wbuf, regval);

        wcnt = regval;
        dev_pn512_write_reg(JREG_COMMIRQ, JBIT_LOALERTI|JBIT_HIALERTI);//����ͱ�����־

        p_mfcmdinfo->nBytesReceived = 0;
        p_mfcmdinfo->nBitsReceived = 0;
        p_mfcmdinfo->ErrorReg = 0;

        //���ö�ʱ��
        dev_pn512_set_timeout(p_mfcmdinfo->timeoutmode, p_mfcmdinfo->timeout);
        commIrqEn |= JBIT_TIMERI;
        waitForComm |= JBIT_TIMERI;
        
        //ʹ������Ҫ���ж�
        dev_pn512_write_reg(JREG_COMMIEN, commIrqEn);
        //��������
        dev_pn512_read_reg(JREG_COMMAND, &regval);
        regval = (regval & (~JMASK_COMMAND))|p_mfcmdinfo->cmd;


        // ��������������IRQ֮ǰ�������Ĵ���  
        // dev_pn512_write_reg(JREG_ERROR, 0);//?
        dev_pn512_write_reg(JREG_COMMAND, regval);

        //����TRANSCEIVE�����ʼ��������
        if(JCMD_TRANSCEIVE == p_mfcmdinfo->cmd)
        {               
          #if 0  //1608����106kbpsû�ж�֡����
            //������Կ���ʹ��RxMulotiple 
            if(1 == g_rf_emd_flg)
            {
                dev_pn512_modify_reg(JREG_RXMODE, 1, JBIT_RXMULOTIPLE);
            }
          #endif  
            /*TRx is always an endless loop, Initiator and Target must set STARTSEND.*/
            dev_pn512_modify_reg(JREG_BITFRAMING, 1, JBIT_STARTSEND);   //��ʼ����
            dev_pn512_write_reg(JREG_COMMIRQ, 0x7F);
       }
        else
        {
            //������ʱ��
            dev_pn512_modify_reg(JREG_CONTROL, 1, JBIT_TSTARTNOW); 
        }
        timerid = dev_get_timeID();
        timerover = 100;
        //===================���濪ʼѭ��======================
        while(1)
        {
            //if(0 != dev_rf_int_get())
            {              
                //��״̬�Ĵ���
                dev_pn512_read_reg(JREG_COMMIRQ, &commirq_reg); 
                commirq_reg &= waitForComm;

                if(0 != commirq_reg)
                {
                    //----------------------------------------
                    //��ʱ����
                    if(commirq_reg&JBIT_TIMERI)
                    {
                        //�볬ʱ������־
                        dev_pn512_write_reg(JREG_COMMIRQ, JBIT_TIMERI);
                        if(p_mfcmdinfo->timeoutcnt==0)
                        {
                            //�����ж��Ƿ��յ�����
                            dev_pn512_read_reg(JREG_FIFOLEVEL, &regval);
                            if(((regval&0x7)==0) && (0 == buflen))
                            {
                                if(0 != statusbak)
                                {           
                                    status = statusbak;
                                }
                                else
                                {
                                    status = MI_NOTAGERR;//û��Ŀ��//MI_ACCESSTIMEOUT;          //��ʱ
                                }
                                if(emdretry != 0)
                                {
                                    status = emdretry;
                                }
                                break;
                            }
                            else
                            {
                                //˵�����ڽ���  
                                waitForComm &= ~JBIT_TIMERI;    //���Կ��Ǽ�����PN512�Ķ�ʱ����ʱ
                                //ֹͣ��ʱ��timeflag
                                dev_pn512_modify_reg(JREG_CONTROL, 1, JBIT_TSTOPNOW);  
                                //���ó�ʱ
                                timerid = dev_get_timeID();
                                timerover = 200;      //�ȴ���������

                            }
                        }//<----if(p_mfcmdinfo->timeoutcnt==0)
                        else
                        {
                            p_mfcmdinfo->timeoutcnt--;
                            //�������ó�ʱ
                            dev_pn512_set_timeout((p_mfcmdinfo->timeoutmode&0xfe), 0x8000);
                            dev_pn512_modify_reg(JREG_CONTROL, 1, JBIT_TSTARTNOW); 

                        }
                    }//<----if(commirq_reg&JBIT_TIMERI)
                    //=========================================
                    //������Ĵ���
                    if((commirq_reg&JBIT_ERRI))
                    {
                        //������Ĵ���
                        if(1 == g_rf_emd_flg)
                        {
                        }
                        else    
                        { 
                            //��ȡ���н��յ��ֽ�����λ��,���ڴ�����
                            dev_pn512_read_reg(JREG_CONTROL, &regval);
                            nbitreceived = (regval & 0x07);

                            dev_pn512_read_reg(JREG_FIFOLEVEL, &regval);
                            regval &= 0x7f;
                            dev_pn512_read_fifo(&buf[buflen], regval);
                            buflen += regval;
                            if((0 != nbitreceived) && (0!=buflen))
                            {
                                buflen--;
                            }
                            dev_pn512_read_reg(JREG_ERROR, &err_reg);


                            //��������־
                            dev_pn512_write_reg(JREG_COMMIRQ, JBIT_ERRI);
                            // ��������������IRQ֮ǰ�������Ĵ���  
                            dev_pn512_write_reg(JREG_ERROR, 0);
                            //����ײ����
                            if(err_reg&JBIT_COLLERR)
                            {
                                status = MI_COLLERR;
                            }

                            //��żУ�����
                            else if(err_reg&JBIT_PARITYERR)
                            {
                                status = MI_PARITYERR;
                            }

                            //Э�����
                            else if(err_reg&JBIT_PROTERR)
                            {
                                status = MI_PROTOCOL_ERROR;
                            }

                            //�������������
                            else if(err_reg&JBIT_BUFFEROVFL)
                            {
                                status = MI_RECBUF_OVERFLOW;
                            }

                            //CRC����
                            else if(err_reg&JBIT_CRCERR)
                            {
                                //CRC������ֻ���յ�1���ֽ���Ϊ��Mifare (N)ACK 
                                if(((0x04 == nbitreceived)||(0x00==nbitreceived)))//stacy
                                {                             
                                    buflen = 1;
                                    status = MI_ACK_SUPPOSED;   //(N)  ACK
                                }
                                else
                                {
                                    status = MI_CRCERR;             //CRC����
                                    //break;
                                } 
                            }

                            //�¶ȹ��ߴ���
                            else if(err_reg&JBIT_TEMPERR)
                            {
                                status = MI_TEMP_ERROR;
                            }

                            //д����
                            else if(err_reg&JBIT_WRERR)
                            {
                                status = MI_WRITEERR;
                            }

                            //������
                            else if(err_reg&JBIT_RDERR)
                            {
                                status = MI_READERR;
                            }
                            //��������
                            else
                            {
                                //if(commirq_reg&JBIT_ERRI)
                                status = MI_OTHERERR;
                            }
                            ///////////////////////////////////////
                            //�ܵ��жϴ���Ĵ���
                            if(MI_OK != status)
                            {    
                                statusbak = status;
                                p_mfcmdinfo->nBitsReceived = nbitreceived;
                                p_mfcmdinfo->nBytesReceived = buflen;
                                memcpy(rbuf, buf, p_mfcmdinfo->nBitsReceived?buflen+1:buflen);
                                break;
                            }                   
                        }
                        err_reg = 0;   
                    }   

                    //======================================================
                    //�������
                    if(commirq_reg&JBIT_TXI)
                    {
                        waitForComm &= ~JBIT_TXI;    //�ط�����ɴ���   ?peng
                        //���߱���
                        commIrqEn |= JBIT_HIALERTI;
                        waitForComm |= JBIT_HIALERTI;
                        dev_pn512_write_reg(JREG_COMMIEN, commIrqEn);       //���߱���
                        dev_pn512_write_reg(JREG_COMMIRQ, JBIT_HIALERTI|JBIT_TXI);   //������ͱ�����־           
                        timerid = dev_get_timeID();
                        timerover = (p_mfcmdinfo->timeout+(((u32)p_mfcmdinfo->timeoutcnt)<<15))/2 + 300;//30000;          //30s
                    }
                    //--------------------------------------------------
                    //�������
                    if(commirq_reg&JBIT_RXI)
                    {
                        dev_pn512_read_reg(JREG_ERROR, &err_reg);
                        
                        //������bits��
                        dev_pn512_read_reg(JREG_CONTROL, &regval);
                        nbitreceived= (regval & 0x07);
                        //��������
                        dev_pn512_read_reg(JREG_FIFOLEVEL, &regval);
                        regval &= 0x7f;
                        
                        if((1 == g_rf_emd_flg)&&
                           (((err_reg&JBIT_PROTERR)&&((regval+buflen)<=4))
                              ||((err_reg&(JBIT_CRCERR|JBIT_PARITYERR))&&(nbitreceived != 0)&&((regval+buflen)<=3))
                              ||((err_reg&(JBIT_CRCERR|JBIT_PARITYERR))&&((regval+buflen)<3)&&((regval+buflen)!=0))
                              ||((err_reg==0)&&(nbitreceived == 0)&&((regval+buflen)==3)&&(g_rf_special_B_flag))
                              ||((err_reg==4)&&(nbitreceived == 0)&&((regval+buflen)==0)&&(g_rf_special_B_flag))
                            ))
                        {
                            //������������
                            dev_pn512_write_reg(JREG_COMMAND, JCMD_RECEIVE);
                            //FLUSH FIFO
                            dev_pn512_write_reg(JREG_FIFOLEVEL, JBIT_FLUSHBUFFER);
                            //����жϱ�־
                            dev_pn512_write_reg(JREG_COMMIRQ, 0x7F);
                            emdretry = MI_CRCERR;
                            nbitreceived = 0;
                            buflen = 0;
                            //����PCD��ʱ 1280/fc=10etu
                            p_mfcmdinfo->timeoutcnt=0;
                            dev_pn512_set_timeout(PCD_TIMER_MODE_ETU_0, 2000);  //��֤�ܹ��յ�1�������ֽ�
                            {
                                p_mfcmdinfo->timeoutcnt = timeoutcnttmp;
                                dev_pn512_set_timeout(p_mfcmdinfo->timeoutmode, p_mfcmdinfo->timeout);
                            }
                            dev_pn512_modify_reg(JREG_CONTROL, 1, JBIT_TSTARTNOW); 
                            timerid = dev_get_timeID();
                            //timerover = 3000;
                            timerover = (p_mfcmdinfo->timeout+(((u32)p_mfcmdinfo->timeoutcnt)<<15))/2 + 300;//30000;          //30s
                            //while(0 == dev_rf_int_get());
                            continue;
                        }
                        else
                        {
                            
                            dev_pn512_read_fifo(&buf[buflen], regval);
                            buflen += regval;
                            if((0 != nbitreceived) && (0!=buflen))
                            {
                                buflen--;
                            }
                            if(err_reg!=0)
                            {
                                // ��������������IRQ֮ǰ�������Ĵ���  
                                dev_pn512_write_reg(JREG_ERROR, 0);
                                
                                //����ײ����
                                if(err_reg&JBIT_COLLERR)
                                {
                                    status = MI_COLLERR;
                                }

                                //��żУ�����
                                else if(err_reg&JBIT_PARITYERR)
                                {
                                    status = MI_PARITYERR;
                                }

                                //Э�����
                                else if(err_reg&JBIT_PROTERR)
                                {
                                    status = MI_PROTOCOL_ERROR;
                                }

                                //�������������
                                else if(err_reg&JBIT_BUFFEROVFL)
                                {
                                    status = MI_RECBUF_OVERFLOW;
                                }

                                //CRC����
                                else if(err_reg&JBIT_CRCERR)
                                {
                                    //CRC������ֻ���յ�1���ֽ���Ϊ��Mifare (N)ACK 
                                    if((0x01 == buflen) && ((0x04 == nbitreceived)||(0x00==nbitreceived)))
                                    {
                                        buflen = 1;
                                        status = MI_ACK_SUPPOSED;   //(N)  ACK
                                    }
                                    else
                                    {
                                        status = MI_CRCERR;             //CRC����
                                        //break;
                                    }
                                }

                                //�¶ȹ��ߴ���
                                else if(err_reg&JBIT_TEMPERR)
                                {
                                    status = MI_TEMP_ERROR;
                                }

                                //д����
                                else if(err_reg&JBIT_WRERR)
                                {
                                    status = MI_WRITEERR;
                                }

                                //������
                                else if(err_reg&JBIT_RDERR)
                                {
                                    status = MI_READERR;
                                }
                                
                            }
                            else
                            {
                                status = MI_OK;
                            }
                            break;
                        }
                    }
                    //---------------------------------------------
                    //ָ��ִ�����
                    if(commirq_reg&JBIT_IDLEI)
                    {
                        
                        //uart_printf("JBIT_IDLEI:\r\n"); 
                        status = MI_OK;
                        break;
                    }
                    //---------------------------------------------
                    //FIFO�ͱ���,����
                    if(commirq_reg&JBIT_LOALERTI)    
                    {
                        dev_pn512_read_reg(JREG_FIFOLEVEL, &regval);     //��FIFO�����ݳ���
                        regval &= 0x7f;
                        if(regval > 64)
                        {
                            //����,�ݲ�����
                        }              
                        else
                        {
                            regval = (64-regval);
                            if(regval >= (p_mfcmdinfo->nBytesToSend-wcnt))
                            {                          
                                regval = p_mfcmdinfo->nBytesToSend - wcnt;
                                //�صͱ���
                                commIrqEn &= (~JBIT_LOALERTI);
                                waitForComm &= (~JBIT_LOALERTI);        //����1��дFIFO�Ϳ��Է�����
                                dev_pn512_write_reg(JREG_COMMIEN, commIrqEn);
                            } 
                            dev_pn512_write_fifo(&wbuf[wcnt], regval);
                            wcnt += regval;
                        }

                        //��ͱ�����־
                        dev_pn512_write_reg(JREG_COMMIRQ, JBIT_LOALERTI);
                    }
                    //---------------------------------------------
                    //FIFO�߱���������
                    if(commirq_reg&JBIT_HIALERTI)    
                    {
                        //��������
                        dev_pn512_read_reg(JREG_FIFOLEVEL, &regval);
                        regval &= 0x7f;
                        dev_pn512_read_fifo(&buf[buflen], regval);
                        buflen += regval; 
                        //��߱�����־
                        dev_pn512_write_reg(JREG_COMMIRQ, JBIT_HIALERTI);
                        if(buflen>256)
                        {
                            status = MI_OVFLERR;
                            break;
                        }
                    }
                }//<-----if(0 != commirq_reg)
            }//if(0 != dev_rf_int_get())
            //===============================================
            //�жϳ�ʱ,��ֹPN512����
            if(dev_querry_time(timerid, timerover))
            {           
                //ʱ�䵽   
                status = MI_ACCESSTIMEOUT;          //��ʱ
                break;
            }
        }//<----while(1)
        
    }  
    //�رն�ʱ��
    dev_pn512_modify_reg(JREG_CONTROL, 1, JBIT_TSTOPNOW);

    dev_pn512_read_reg(JREG_COMMAND, &regval);
    dev_pn512_write_reg(JREG_COMMAND, regval&0xf0);
    //��������жϼ�IRQ��־
    dev_pn512_write_reg(JREG_COMMIRQ, 0x7f); //waitForComm);
    dev_pn512_write_reg(JREG_DIVIRQ, 0x1f); //waitForDiv);
    //���FIFO
    dev_pn512_write_reg(JREG_FIFOLEVEL, JBIT_FLUSHBUFFER);
    //��ֹ�ж�
    dev_pn512_write_reg(JREG_COMMIEN, 0x80);
    dev_pn512_write_reg(JREG_DIVIEN,  0x80); 
    //��BITFAMEING�Ĵ���
    dev_pn512_write_reg(JREG_BITFRAMING, 0);  
    if(MI_OK == statusbak)
    {        
        p_mfcmdinfo->nBytesReceived = buflen;
        p_mfcmdinfo->nBitsReceived = nbitreceived;
        memcpy(rbuf, buf, p_mfcmdinfo->nBitsReceived?buflen+1:buflen);
    }
    
    //===============================================
    k_free(buf);
    return status;
}
s8 dev_pn512_execute_cmd(MfCmdInfo *p_mfcmdinfo, u8 *wbuf, u8 *rbuf)
{
    s8 ret; 
    ret = dev_hm1608_execute_cmd(p_mfcmdinfo, wbuf,  rbuf);
    return ret;
}
s32 dev_pn512_mifare_auth(u8 key_type, u8 *key, u8 *snr, u8 block)
{
    S32 status = MI_OK;
    u8 databfr[12];
    u8 rtmp[100];
    u8 regval;
    MfCmdInfo mfcmdinfotmp;
    
    //׼������
    databfr[0] = key_type;     //key A or key B
    databfr[1] = block;          //address to authentication
    memcpy(databfr+2, key, 6);  //6bytes key
    memcpy(databfr+8, snr, 4);  //4bytes UID
    
    //prepare data for common transceive
    dev_pcd_reset_mfinfo(&mfcmdinfotmp);    //��λ�ṹ
    mfcmdinfotmp.nBytesToSend = 12;         //���ݳ���
    mfcmdinfotmp.timeoutmode = PCD_TIMER_MODE_100US_1;
    mfcmdinfotmp.timeout = 50;              //������pengxuebin��????
    mfcmdinfotmp.cmd = JCMD_AUTHENT;
    status = dev_pn512_execute_cmd(&mfcmdinfotmp, databfr, rtmp);
    //   �ĵ������ڲ�����  
    if(MI_OK == status)
    {
        dev_pn512_read_reg(JREG_STATUS2, &regval);
        if(0x08 != (regval&0x0f))
        {
            status = MI_AUTHERR;            //��֤��
        }
    }
    return status;
    
}
/*******************************************************************************
 ** Descriotions: �ȴ�netu   
 ** parameters:     
                     
 ** Returned value: 
 ** Created By:     
 ** Remarks:       
*******************************************************************************/
void dev_pn512_wait_netu(u16 netu)
{
    u8 regval;
    u32 id;
    //��������жϱ�־
    dev_pn512_write_reg(JREG_COMMIRQ, 0x7F);
    dev_pn512_write_reg(JREG_DIVIRQ,  0x1F);
    dev_pn512_read_reg(JREG_COMMIEN, &regval);
    dev_pn512_write_reg(JREG_COMMIEN, regval|JBIT_TIMERI);//ʹ�ܶ�ʱ���ж�
    dev_pn512_set_timeout(PCD_TIMER_MODE_nETU_0, netu);
     //������ʱ��
    dev_pn512_modify_reg(JREG_CONTROL, 1, JBIT_TSTARTNOW);
    id = dev_get_timeID();
    while(1)
    {
        dev_pn512_read_reg(JREG_COMMIRQ, &regval);
        if(regval&JBIT_TIMERI)
        {
            break;
        }
        if(dev_querry_time(id, 50000))
        {   
  
            break;
        }
        
        
    }
    //�رն�ʱ��
    dev_pn512_modify_reg(JREG_CONTROL, 1, JBIT_TSTOPNOW);
    //��ֹ��ʱ���жϣ������ʱ���жϱ�־ 
    dev_pn512_modify_reg(JREG_COMMIEN, 0, JBIT_TIMERI);
    dev_pn512_write_reg(JREG_COMMIRQ, JBIT_TIMERI);
    
}
/***********************************************************
  Function:  
  Author:     
  Version : 1.0
  Date: 
  Input: void
  Output:void
  Return:void
  Description: 
***********************************************************/
s8 dev_pn512_softpowerdown(void)
{
    s8  status =   MI_OK;

  #if 0
    /*soft reset*/
     dev_pn512_write_reg(JREG_COMMAND, JCMD_SOFTRESET);

     /* Set AutoWakeUp so that the chip wakes up automatically */
     dev_pn512_modify_reg(JREG_TXAUTO, 0x20, 0x20);
   #endif
    /* Set the arguments for Power Down */
     //dev_pn512_modify_reg(JREG_COMMAND, JBIT_POWERDOWN, JBIT_POWERDOWN);
    dev_pn512_write_reg(JREG_COMMAND, JBIT_POWERDOWN);

    dev_pn512_export_init();
    PN512_NRSTPD_OFF;
    return status;
    
}
s32 dev_pn512_softpoweron(void)
{
    
    PN512_NRSTPD_RST;
    dev_user_delay_ms(1); //(1);
    dev_pn512_write_reg(JREG_COMMAND, JCMD_IDLE);
    return 0;
}
/*******************************************************************************
 ** Descriotions:   ����REQA����ѯ�ų����Ƿ��з���ISO14443A��
 ** parameters:     ��                                              
 ** Returned value: 0: �ɹ�
                   ����:ʧ��                                                                           
 ** Created By:     ��ѧ�� 2009-7-2 11:14:35
 ** Remarks:     
///////////////////////////////////////////////////////////////////////
// Request Command defined in ISO14443(MIFARE)
// Request,Anticoll,Select,return CardType(2 bytes)+CardSerialNo(4 bytes)    
// Ѱ��������ͻ��ѡ��    ���ؿ����ͣ�2 bytes��+ ��ϵ�к�(4 bytes)
///////////////////////////////////////////////////////////////////////                                                                            
*******************************************************************************/
s8 dev_pn512_send_reqa(u8 req_code, u8 *atq)
{
    s8 status = MI_OK;
    u8 wtmp[10];
    u8 rtmp[100];
    MfCmdInfo mfcmdinfotmp;
    
    g_rf_emd_flg = 0;
    *atq = 0;
    //��ʼ��
    dev_pn512_modify_reg(JREG_STATUS2, 0, JBIT_CRYPTO1ON);  //disable Crypto if activated before 
    dev_pn512_write_reg(JREG_COLL, JBIT_VALUESAFTERCOLL);   //active values after coll ����
    dev_pn512_modify_reg(JREG_TXMODE, 0, JBIT_CRCEN);       //disable TxCRC and RxCRC
    dev_pn512_modify_reg(JREG_RXMODE, 0, JBIT_CRCEN);
    dev_pn512_write_reg(JREG_BITFRAMING, REQUEST_BITS);     //�������һ�ֽڴ����λ��(7λ)
    
    //������Ҫ����Ĳ���
    dev_pcd_reset_mfinfo(&mfcmdinfotmp);
    wtmp[0] = req_code;
    mfcmdinfotmp.cmd = JCMD_TRANSCEIVE;
    mfcmdinfotmp.nBytesToSend = 1;
    //���ó�ʱ
    mfcmdinfotmp.timeoutmode = PCD_TIMER_MODE_ETU_1;//PCD_TIMER_MODE_100US_1;
    mfcmdinfotmp.timeout = 40;       //10etu     //400us, ���� pengxuebin��  ????
    
    status = dev_pn512_execute_cmd(&mfcmdinfotmp, wtmp, rtmp);

   #ifdef PCD_DEBUG  
    rf_debug_printf("reqa:%d\r\n", status);    
   #endif                               
    if(MI_OK == status)
    {
        if(mfcmdinfotmp.nBytesReceived != 2)
        {
            status = MI_BYTECOUNTERR;           //�ֽڳ��ȴ���
        }
        else
        {
            memcpy(atq, rtmp, mfcmdinfotmp.nBytesReceived);
        }
    }
    dev_pn512_write_reg(JREG_BITFRAMING, 0);
    return status;
}
/*******************************************************************************
 ** Descriotions:   ʹpiccA������HALT״̬
 ** parameters:     
                     sel_code   command code
                     bitcount   the bit counter of known UID
                     snr        the UID have known
                     sak        the byte to save the ACK from card                                              
 ** Returned value:  0: �ɹ�
                    ����:ʧ�� 
 ** Created By:     ��ѧ�� 2009-7-2 11:14:35
 ** Remarks:       this founction is used internal only, and cannot call by application program                                                                             
*******************************************************************************/
s8 dev_pn512_halt_piccA(void)
{
    s8 status = MI_OK;
    u8 databfr[16];
    u8 rtmp[16];
    MfCmdInfo mfcmdinfotmp;
    
    //initialise data buffer
    databfr[0] = HALTA_CMD;
    databfr[1] = HALTA_PARAM;
    dev_pn512_modify_reg(JREG_TXMODE, 1, JBIT_CRCEN);      
    dev_pn512_modify_reg(JREG_RXMODE, 1, JBIT_CRCEN);
    dev_pn512_modify_reg(JREG_STATUS2, 0, JBIT_CRYPTO1ON);  

    dev_pcd_reset_mfinfo(&mfcmdinfotmp);    //��λ�ṹ
    mfcmdinfotmp.nBytesToSend = HALTA_CMD_LENGTH;
    mfcmdinfotmp.timeoutmode = PCD_TIMER_MODE_ETU_1;
    mfcmdinfotmp.timeout = 106;         //1ms,
    mfcmdinfotmp.cmd = JCMD_TRANSCEIVE;
    status = dev_pn512_execute_cmd(&mfcmdinfotmp, databfr, rtmp);
    dev_pn512_modify_reg(JREG_TXMODE, 0, JBIT_CRCEN);      
    dev_pn512_modify_reg(JREG_RXMODE, 0, JBIT_CRCEN);       
    if(MI_NOTAGERR == status)
    {
        status = MI_OK;
    }
    return status;
}
/*******************************************************************************
 ** Descriotions:   REQB
 ** parameters:     afi : ������PCD����׼��Ӧ������
                    param:����
 ** Returned value:  0: �ɹ�
                    ����:ʧ�� 
 ** Created By:     ��ѧ�� 2009-7-7 11:00:17
 ** Remarks:       
*******************************************************************************/
s8 dev_pn512_req_piccB(u8 afi, u8 N, u8 req_code, u8 *atqB, u8 *atqBlen)
{
    s8 status = MI_OK; 
    u8 tmp[16];
    u8 tmp1[64];
    MfCmdInfo mfcmdinfotmp;

    g_rf_emd_flg = 0;
    //activate_deletion of bits after coll //�����ͻ���
    dev_pn512_write_reg(JREG_COLL, 0);
    
    *atqBlen = 0;
    
    tmp[0] = 0x05;          //Apf
    tmp[1] = afi;           //afi
    //param:bit5=1,��չATQB֧��
    tmp[2] = (req_code&0x08)|(N&0X07);//param;         //afi
    //prepare data for common transceive
    dev_pcd_reset_mfinfo(&mfcmdinfotmp);        //��λ�ṹ
    mfcmdinfotmp.nBytesToSend = 3;              //���ݳ���
    mfcmdinfotmp.timeoutmode = PCD_TIMER_MODE_ETU_1;
    mfcmdinfotmp.timeout = 100;//50;
    mfcmdinfotmp.cmd = JCMD_TRANSCEIVE; 
    status = dev_pn512_execute_cmd(&mfcmdinfotmp, tmp, tmp1);

    if(MI_OK == status)
    {
        if(mfcmdinfotmp.nBytesReceived >= 12)
        {          //�ֽڳ��ȴ���
            memcpy(atqB, tmp1, mfcmdinfotmp.nBytesReceived);
            *atqBlen = mfcmdinfotmp.nBytesReceived;
        }
        else 
        {
            status = MI_BITCOUNTERR;    //���մ���
        }
    }
    else if((MI_NOTAGERR != status) && (MI_ACCESSTIMEOUT != status))
    {
        status = MI_COLLERR;            // collision occurs
    }
    
    //activate values after coll
    dev_pn512_write_reg(JREG_COLL, JBIT_VALUESAFTERCOLL);   
    return status;
}

/*******************************************************************************
 ** Descriotions:   ����ͻѡ��(Functions to split anticollission and select internally)  
 ** parameters:     
                     sel_code   command code
                     bitcount   the bit counter of known UID
                     snr        the UID have known
                     sak        the byte to save the ACK from card                                              
 ** Returned value:  0: �ɹ�
                    ����:ʧ�� 
 ** Created By:     ��ѧ�� 2009-7-2 11:14:35
 ** Remarks:       this founction is used internal only, and cannot call by application program                                                                             
*******************************************************************************/
s8 dev_pn512_selectanticoll_picc(u8 grade, u8 grademax, u8 bitcount, u8 *snr, u8 *sak)
{
    s8 status = MI_OK;
    u8 databfr[20];
    u8 rtmp[100];
    u8 nbytes;
    u8 nbits;
    u8 rbits;           // number of total received bits
    u8 complete;
    s8 completecnt=3;
    u8 i;
    u8 byteOffset;
    u8 flg=0;
    
    MfCmdInfo mfcmdinfotmp;
    
    if(bitcount>COMPLETE_UID_BITS)
    {
        return MI_WRONG_PARAMETER_VALUE;
    }
    else if(bitcount==COMPLETE_UID_BITS)           //���͵�����
    {
        complete = 1;                          
    }
    else
    {
        dev_pn512_modify_reg(JREG_TXMODE, 0, JBIT_CRCEN);
        dev_pn512_modify_reg(JREG_RXMODE, 0, JBIT_CRCEN);
        dev_pn512_write_reg(JREG_COLL, 0);
        complete = 1;//32;                          //���32��ѭ��
    }
    
    while(complete && (status == MI_OK))
    {
        if(bitcount>COMPLETE_UID_BITS)
        {
            return MI_WRONG_PARAMETER_VALUE;
        }
        if(bitcount==COMPLETE_UID_BITS)
        {
            dev_pn512_modify_reg(JREG_TXMODE, 1, JBIT_CRCEN);
            dev_pn512_modify_reg(JREG_RXMODE, 1, JBIT_CRCEN);
            dev_pn512_write_reg(JREG_COLL, JBIT_VALUESAFTERCOLL);  //��λֻ����106kb/s��λ����ͻʱ��0������Ҫ��1
        }
        if(flg)
        {
            dev_pn512_wait_etu(300);//(54);      //pengxuebin,20090822_1
        }
        else
        {
            flg = 1;
            dev_pn512_wait_etu(60);
        }
        
        nbits = (bitcount%BITS_PER_BYTE);           //��Ҫ���͵ķ�����λ
        nbytes = ((bitcount+7) / BITS_PER_BYTE);    //��Ҫ���͵�����
        
        databfr[0] = PCDTypeASel[grade&0x03];       //��ͬ�ȼ���SEL����
        databfr[1] = 0x20 + (((bitcount/8)<<4)&0x70) + nbits;//NVB
        memcpy(&databfr[2], snr, nbytes);           //��Ҫ���͵�����
        
        //set TxLastBits and RxAlign to number of bits sent
        dev_pn512_write_reg(JREG_BITFRAMING, ((nbits << UPPER_NIBBLE_SHIFT) | nbits));
        
        //prepare data for common transceive
        dev_pcd_reset_mfinfo(&mfcmdinfotmp);        //��λ�ṹ
        mfcmdinfotmp.nBytesToSend = (nbytes+2);
        mfcmdinfotmp.timeoutmode = PCD_TIMER_MODE_ETU_1;//PCD_TIMER_MODE_100US_1;
        mfcmdinfotmp.timeout = 20;//19;//1000;//��ʱ��100etu  //5000;                  //������pengxuebin��????
        mfcmdinfotmp.cmd = JCMD_TRANSCEIVE; 
        
#ifdef PCD_DEBUG   
dev_debug_printf("cascanticoll:send:bytes=%x, bits=%x\r\n", mfcmdinfotmp.nBytesToSend, ((nbits << UPPER_NIBBLE_SHIFT) | nbits));        
//PrintFormat(databfr, mfcmdinfotmp.nBytesToSend);
#endif


        status = dev_pn512_execute_cmd(&mfcmdinfotmp, databfr, rtmp);

#ifdef PCD_DEBUG    
dev_debug_printf("cascanticoll:status=%d,%d,%d\r\n", status, mfcmdinfotmp.nBytesReceived, mfcmdinfotmp.nBitsReceived);        
dev_debug_printf("grade=%d,%d bitcount=%d\r\n", grade, grademax, bitcount);        
PrintFormat(rtmp, mfcmdinfotmp.nBytesReceived); 
#endif

        if(40 == bitcount)
        {
            if(MI_OK == status)
            {               
                if((SAK_LENGTH == mfcmdinfotmp.nBytesReceived)&&(0 == mfcmdinfotmp.nBitsReceived))
                { 
                    *sak = rtmp[0]; 
                }
                else
                {
                    status = MI_BITCOUNTERR;
                }
            }
            else if(MI_NOTAGERR == status)      //20090826_1
            {
                completecnt--;
                if(completecnt<=0)break;
                else{
                    complete=1;
                    status = MI_OK;
                    continue;
                }
            } 
            break;
        }

        
        if((MI_COLLERR == status) || (MI_OK == status))
        {   //���ִ�гɹ�����ײ����, ��ִ��

            #ifdef PCD_DEBUG   
            dev_debug_printf("rec:bytes=%x,Bits=%x\r\n", mfcmdinfotmp.nBytesReceived, mfcmdinfotmp.nBitsReceived);
            if(0 != mfcmdinfotmp.nBitsReceived)
            {
                PrintFormat(rtmp, mfcmdinfotmp.nBytesReceived+1);
            }
            else
            {
                PrintFormat(rtmp, mfcmdinfotmp.nBytesReceived);
            }
            #endif


            rbits = (mfcmdinfotmp.nBitsReceived + (mfcmdinfotmp.nBytesReceived << 3) - nbits);  //������յ�λ��
            if((rbits+bitcount) > COMPLETE_UID_BITS)   //40
            {
                status = MI_BITCOUNTERR;                //λͳ�ƴ���
                break;
            }
            
            //increment number of bytes received if also some bits received
            if(mfcmdinfotmp.nBitsReceived)
            {
                mfcmdinfotmp.nBytesReceived++;
            }
            
            //reset offset for data copying
            byteOffset = 0;
            
            //if number of bits sent are not 0, write first received byte in last of sent
            if(nbits)
            {   //last byte transmitted and first byte received are the same
                snr[nbytes-1] |= rtmp[0];           //pengxuebin, ??
                byteOffset++;
            }

            memcpy(&snr[nbytes], &rtmp[byteOffset], mfcmdinfotmp.nBytesReceived);//4-nbytes);  //??
            
            if(MI_COLLERR == status)
            {
                //calculate new bitcount value
                bitcount = (bitcount + rbits);
            }
            else    //û�г�ͻ
            {
                if((rbits+bitcount) == COMPLETE_UID_BITS)   //40
                {
            
                    #ifdef PCD_DEBUG   
                        Uart_Printf("BCC:nbits=%x\r\n", nbits);
 //                       PrintFormat(snr, (4-j));   
                    #endif             

                    nbits = 0;
                    for(i=0; i<5; i++)
                    {
                        nbits ^= snr[i];
                    }
                    if(0 == nbits)
                    {  
                      
                        //˵������ͻ�ɹ�
                        bitcount = COMPLETE_UID_BITS;// += rbits;
                        complete = 1;                //ִ��select
                        continue;
                    }
                    else
                    {
                        status = MI_SERNRERR;
                        break;
                    }
                }
                else
                {
                    status = MI_SERNRERR;
                    break;
                }
            } 
        }
        else if(MI_NOTAGERR == status)
        {
            completecnt--;
            if(completecnt<=0)break;
            else{
                complete++;
                status = MI_OK;
            }
        }
        else
        {
            break;
        }
        complete --;
    }

    
    if((0 == complete))             //ѭ����������32�λ�û�гɹ�
    {
        status = MI_ANTICOLL_ERR;
        
    }
    return status;
}
/*******************************************************************************
 ** Descriotions:   RATS(����ATS)
 ** parameters:     rbuf: ����ATS
                    rlen: ATS���� 
 ** Returned value:  0: �ɹ�
                    ����:ʧ�� 
 ** Created By:     ��ѧ�� 2009-7-7 11:00:17
 ** Remarks:       
*******************************************************************************/
s8 dev_pn512_send_rats_piccA(u8 cid, u8 *rbuf, u16 *rlen)
{
    s8 status = MI_OK;
    u8 i,j; 
    u8 k;
    u8 n;
    u8 tmp[10];
//    u8 tmp1[256]; 
    u8 *tmp1=NULL;
    MfCmdInfo mfcmdinfotmp;
    *rlen = 0;

    tmp1 = (u8 *)k_malloc(256);
    if(NULL == tmp1)
    {
        return MI_OTHERERR;
    }
    
    g_rf_emd_flg = 1;
    for(n=0; n<3; n++)
    {
        dev_pn512_wait_etu(54);      //pengxuebin,20090824_1
        tmp[0] = 0xE0;
        tmp[1] = (cid&0x0f)|(PCDFSDI<<4);
        dev_pcd_reset_mfinfo(&mfcmdinfotmp);        //��λ�ṹ
        mfcmdinfotmp.nBytesToSend = 2;              //���ݳ���
        mfcmdinfotmp.timeoutmode = PCD_TIMER_MODE_ETU_1;//PCD_TIMER_MODE_100US_1;
        mfcmdinfotmp.timeout = 600;//20090822_1
       
        mfcmdinfotmp.cmd = JCMD_TRANSCEIVE;
        tmp1[0]=0x00;
        status = dev_pn512_execute_cmd(&mfcmdinfotmp, tmp, tmp1);
 
        if((MI_OK == status)&&(0 != mfcmdinfotmp.nBytesReceived))
        {
            //����ATS
            //TL   (�����ֽ�)
            k = tmp1[0];
            if((mfcmdinfotmp.nBitsReceived!=0) || (mfcmdinfotmp.nBytesReceived != k))   //??pengxuebin, 2009-7-13 14:52:27 mfcmdinfotmp.nBytesReceived��û��CRC
            {
                status = MI_BITCOUNTERR;
            }
            else
            {
                *rlen = tmp1[0];
                memcpy(rbuf, tmp1, tmp1[0]);
            } 
            break;
        }
        else if((MI_PARITYERR == status)||(MI_CRCERR == status)||(MI_ACK_SUPPOSED == status)||(MI_NOTAGERR == status)||(MI_OTHERERR==status)||(MI_OK == status)) 
        {
            if((0 == mfcmdinfotmp.nBytesReceived) && (0x05 != tmp1[0]))//(mfcmdinfotmp.nBitsReceived!=0))   //20090901
            {
                
            }
            else if((1 == mfcmdinfotmp.nBytesReceived) && ((0x01 == tmp1[0]) || (0x00 == tmp1[0])))
            {
                
            }
            else if((MI_CRCERR==status))
            {   
                
                dev_pn512_read_reg(JREG_CRCRESULT1, &tmp[0]);     //A9
                dev_pn512_read_reg(JREG_CRCRESULT2, &tmp[1]);     //AC
                if((5 == mfcmdinfotmp.nBytesReceived) && (0x05 == tmp1[0])
                    &&(tmp1[1]==0x72)&&(tmp1[2]==0x80)&&(tmp1[3]==0x40)&&(tmp1[4]==0x02))
                {
                    //���
                    if(((0x84 == tmp[0])&&(0x08 == tmp[1]))  
                     ||((0x57 == tmp[0])&&(0xad == tmp[1]))) 
                    {
                        break;
                    }
                    else if((0x63 == tmp[0]) && (0x63 == tmp[1]))   
                    {
                        break;
                    }
                }
                else if((6 == mfcmdinfotmp.nBytesReceived) && (5 == tmp1[0]))
                {
                }
                else if((2 == mfcmdinfotmp.nBytesReceived) && (01 == tmp1[0])&&(0x77 == tmp1[1]))
                {
                }
                else
                {
                    break;
                }

            }
            else
            {
                break;
            }     
            delayms(6);
        }
        else
        {
            break;
        }        
    }

    k_free(tmp1);
    return status;
}
/*******************************************************************************
 ** Descriotions:   Attrib  ѡ��PICC
 ** parameters:     PUPI��PICC��ʶ����
                      CID��PICC�߼���ַ��ȡֵ��Χ��0--14��
                      brTx��PCD--->PICC����ѡ��
                      brRx��PICC--->PCD����ѡ��
                      PARAM3������3��
 ** Returned value:  0: �ɹ�
                    ����:ʧ�� 
 ** Created By:     ��ѧ�� 
 ** Remarks:       
*******************************************************************************/
s8 dev_pn512_attrib(u8 cid, u8 *pupi, u8 brTx, u8 brRx, u8 param3, u32 fwt)
{
    s8 status = MI_OK; 
    u8 tmp[16];
    u8 tmp1[256];
    u8 i;
    MfCmdInfo mfcmdinfotmp;
    u32 fwttmp;
    
    g_rf_emd_flg = 1;
    dev_pn512_modify_reg(JREG_RXMODE, 1, JBIT_CRCEN); 
    
    fwttmp = (fwt+31)>>5;
    
    for(i=0; i<3; i++)
    {
        dev_pn512_wait_etu(54);      //pengxuebin,20090822_1
        
        tmp[0] = 0x1d;              //command
        memcpy(&tmp[1], pupi, 4);   //Identifier
        tmp[5] = 0x00;              // EOF/SOF required, default TR0/TR1        
        tmp[6] = 0x08|((brTx&0x03)<<4)|((brRx&0x03)<<6);  // Max frame 256 
        tmp[7] = param3;        // Param3, ISO/IEC 14443-4 compliant?
        tmp[8] = cid&0x0f;          //CID
         
        //prepare data for common transceive
        dev_pcd_reset_mfinfo(&mfcmdinfotmp);        //��λ�ṹ
        mfcmdinfotmp.nBytesToSend = 9;              //���ݳ���
        mfcmdinfotmp.timeoutmode = PCD_TIMER_MODE_nETU1_1;
        
        mfcmdinfotmp.timeout = fwttmp&0x7FFF;//(0x01 << PICCData[cid].FWI) + 12+16;
        mfcmdinfotmp.timeoutcnt = (fwttmp>>15);
        mfcmdinfotmp.cmd = JCMD_TRANSCEIVE;
      #ifdef PCD_DEBUF
        Uart_Printf("dev_pcd_attrib:wlen=%d,FWI=%d\r\n", mfcmdinfotmp.nBytesToSend, PICCData[cid].FWI);
        PrintFormat(tmp, mfcmdinfotmp.nBytesToSend);
      #endif  
       
        g_rf_special_B_flag=1;
        status = dev_pn512_execute_cmd(&mfcmdinfotmp, tmp, tmp1);    
        g_rf_special_B_flag=0;
        if(MI_OK == status)
        { 
            if(mfcmdinfotmp.nBytesReceived != 1) 
            {   
                status = MI_BITCOUNTERR;    //���մ���
            }
            else
            {
                if((tmp1[0] &0x0f) != cid)
                {
                    status = MI_WRONG_PARAMETER_VALUE;
                }
            }
            break;
        }
        else if((MI_CRCERR == status)||(MI_PROTOCOL_ERROR == status))
        {
            if(0 == memcmp(tmp1,"\x02\x00\xB2\x01\x04\x00\x90\x00", 8))
            {
                break;
            }
            delayms(6);
        }
        else if((MI_PARITYERR == status)||(MI_CRCERR == status)||(MI_ACK_SUPPOSED == status)
              ||(MI_NOTAGERR == status)||(MI_OTHERERR==status)||(MI_PROTOCOL_ERROR == status)||(MI_COLLERR==status))//
        {
            
            delayms(6); 
        } 
        else
        {
            break;
        }
    }
    
    return status; 
}
/*******************************************************************************
 ** Descriotions:    
 ** parameters:      
                    
 ** Returned value:  0: �ɹ�
                    ����:ʧ�� 
 ** Created By:     ��ѧ�� 2009-8-22 23:48:56
 ** Remarks:       
*******************************************************************************/
s8 dev_pn512_exchange_piccA(u8 *wbuf, u16 wlen, u8 *rbuf, u16 *rlen, u32 etu, u8 wtx)
{
    s32 status;
    u32 tmp;
    MfCmdInfo mfcmdinfotmp;
    dev_pcd_wait_etu(54);      //pengxuebin,20090822_1
    *rlen = 0;
    dev_pcd_reset_mfinfo(&mfcmdinfotmp);                //��λ�ṹ
    mfcmdinfotmp.nBytesToSend = wlen;                   //���ݳ���
    mfcmdinfotmp.timeoutmode = PCD_TIMER_MODE_nETU1_1;  //��32etuΪ��λ(32*(fc/128))
    tmp = etu*(wtx+1) + 12+16; //+ 384 + 12+16;
    mfcmdinfotmp.timeout = tmp&0x7fff;
    mfcmdinfotmp.timeoutcnt = (tmp>>15);
    if(0 == mfcmdinfotmp.timeout)mfcmdinfotmp.timeout++;
    mfcmdinfotmp.cmd = JCMD_TRANSCEIVE;
    #ifdef PCD_DEBUG  
    RF_DEBUG("__pcd_exchange:wlen=%x\r\n", mfcmdinfotmp.nBytesToSend); 
    RF_DEBUGHEX(NULL, wbuf, mfcmdinfotmp.nBytesToSend);  
    #endif 
      
    status = dev_pn512_execute_cmd(&mfcmdinfotmp, wbuf, rbuf); 
    
    #ifdef PCD_DEBUG  
    RF_DEBUG("__pcd_exchange:status=%d, rlen=%x, bits=%x\r\n", status, mfcmdinfotmp.nBytesReceived, mfcmdinfotmp.nBitsReceived); 
    RF_DEBUGHEX(NULL, rbuf, mfcmdinfotmp.nBytesReceived);
    #endif
    if(MI_OK == status)
    {
        if(mfcmdinfotmp.nBitsReceived!=0) 
        {
            status = MI_BITCOUNTERR;
        }
        else if(0 == mfcmdinfotmp.nBytesReceived)
        {
            status = MI_PROTOCOL_ERROR;
        }
        else
        {
            *rlen = mfcmdinfotmp.nBytesReceived;
            #ifdef PCD_DEBUG  
            PrintFormat(rbuf, mfcmdinfotmp.nBytesReceived);   
            #endif
        }
    }
    return status;
}

s8 dev_hm1608_execute_cmd_send(u8 *wbuf, u32 wlen)
{
    //s32 i;
    s32 timerid;
    s32 timerover;
//    s32 buflen=0;
//    u8* buf;//[300];
    u16 wcnt;
    u8 waitForDiv;
    u8 regval;
    u8 commirq_reg;
    u8 err_reg;
    u8 errcnt;
    //    u8 err1cnt;
    s8 status = MI_OK;
    s8 statusbak=MI_OK;
    u8 commIrqEn = 0;
    //    u8 doReceive = 0;         //������ձ�־
    u8 waitForComm = (JBIT_ERRI | JBIT_TXI);
    u8 nbitreceived=0;
    u8 emdretry=MI_OK;
//    u32 timeoutcnttmp;
    //u32 timeouttmp;

    //=============================================    
    waitForDiv = 0;
    errcnt = 0;   
    /*remove all Interrupt request flags that are used during function,
    keep all other like they are
    ��������жϱ�־
    */   
    dev_pn512_write_reg(JREG_COMMIRQ, 0x7F);
    dev_pn512_write_reg(JREG_DIVIRQ,  0x1F);
    //ȡ������
    dev_pn512_read_reg(JREG_COMMAND, &regval);
    regval = (regval & (~JMASK_COMMAND));
    dev_pn512_write_reg(JREG_COMMAND, regval);

    /*buf =(u8*)k_malloc(300);
    if(buf == NULL)
    {
        dev_debug_printf("�����ڴ�ʧ��\r\n");
        return MI_UNKNOWN_COMMAND;
    }*/
//    timeoutcnttmp = p_mfcmdinfo->timeoutcnt;
    //timeouttmp = p_mfcmdinfo->timeout;
    //-----------------------------------------------
    #if 0
    //����ָ�������
    switch(p_mfcmdinfo->cmd)
    {
        //---------------------------------------
        case JCMD_TRANSMIT:     //0x04 
            commIrqEn = JBIT_TXI | JBIT_TIMERI;
            waitForComm = JBIT_TXI;
            break;
        //---------------------------------------
        case JCMD_RECEIVE:      //0x08 
            commIrqEn = JBIT_RXI | JBIT_TIMERI;     //| JBIT_ERRI;
            waitForComm = JBIT_RXI | JBIT_TIMERI;   //| JBIT_ERRI;
            //            doReceive = 1;
            break;
        //---------------------------------------
        case JCMD_TRANSCEIVE:   //0x0C 
            //regval = (regval & (~JMASK_COMMAND)) | JCMD_TRANSCEIVE;
            //���õȴ�MASK
            commIrqEn   = JBIT_RXI | JBIT_IDLEI| JBIT_LOALERTI | JBIT_ERRI | JBIT_TIMERI | JBIT_TXI; //JBIT_RXI | JBIT_TIMERI;  //| JBIT_ERRI;
            waitForComm = JBIT_RXI | JBIT_IDLEI| JBIT_LOALERTI | JBIT_ERRI | JBIT_TIMERI | JBIT_TXI; //���е��ж��źŴ���
            break;
        default:
            status = MI_UNKNOWN_COMMAND;
            break; 
    }
    #endif
    //===============================================
   
    {
        //���õȴ�MASK
        commIrqEn   = JBIT_RXI | JBIT_IDLEI| JBIT_LOALERTI | JBIT_ERRI | JBIT_TIMERI | JBIT_TXI; //JBIT_RXI | JBIT_TIMERI;  //| JBIT_ERRI;
        waitForComm = JBIT_RXI | JBIT_IDLEI| JBIT_LOALERTI | JBIT_ERRI | JBIT_TIMERI | JBIT_TXI; //���е��ж��źŴ���
            
        //��FIFO
        dev_pn512_write_reg(JREG_FIFOLEVEL, JBIT_FLUSHBUFFER);
        regval = 64;                              //FIFOΪ64Bytes
        if(regval >= wlen)
        {
            regval = wlen;
            //�صͱ���
            commIrqEn &= (~JBIT_LOALERTI);
            waitForComm &= (~JBIT_LOALERTI);        //����1��дFIFO�Ϳ��Է�����
        }
        dev_pn512_write_fifo(wbuf, regval);

        wcnt = regval;
        dev_pn512_write_reg(JREG_COMMIRQ, JBIT_LOALERTI|JBIT_HIALERTI);//����ͱ�����־


        //���ö�ʱ��
        dev_pn512_set_timeout(PCD_TIMER_MODE_nETU1_1, 200);
        commIrqEn |= JBIT_TIMERI;
        waitForComm |= JBIT_TIMERI;
        
        //ʹ������Ҫ���ж�
        dev_pn512_write_reg(JREG_COMMIEN, commIrqEn);
        //��������
        dev_pn512_read_reg(JREG_COMMAND, &regval);
        regval = (regval & (~JMASK_COMMAND))|JCMD_TRANSCEIVE;


        // ��������������IRQ֮ǰ�������Ĵ���  
        // dev_pn512_write_reg(JREG_ERROR, 0);//?
        dev_pn512_write_reg(JREG_COMMAND, regval);

        //����TRANSCEIVE�����ʼ��������
        //if(JCMD_TRANSCEIVE == p_mfcmdinfo->cmd)
        {               
          #if 0  //1608����106kbpsû�ж�֡����
            //������Կ���ʹ��RxMulotiple 
            if(1 == g_rf_emd_flg)
            {
                dev_pn512_modify_reg(JREG_RXMODE, 1, JBIT_RXMULOTIPLE);
            }
          #endif  
//drv_psam_testpin_set(1, 0);
            /*TRx is always an endless loop, Initiator and Target must set STARTSEND.*/
            dev_pn512_modify_reg(JREG_BITFRAMING, 1, JBIT_STARTSEND);   //��ʼ����
            dev_pn512_write_reg(JREG_COMMIRQ, 0x7F);
       }
      #if 0  
        else
        {
            //������ʱ��
            dev_pn512_modify_reg(JREG_CONTROL, 1, JBIT_TSTARTNOW); 
        }
      #endif  
        timerid = dev_get_timeID();
        timerover = 100;
        //===================���濪ʼѭ��======================
        while(1)
        {
            //if(0 != dev_rf_int_get())
            {              
                //��״̬�Ĵ���
                dev_pn512_read_reg(JREG_COMMIRQ, &commirq_reg); 
                commirq_reg &= waitForComm;

                if(0 != commirq_reg)
                {
                    //----------------------------------------
                    //��ʱ����
                    if(commirq_reg&JBIT_TIMERI)
                    {
                      #if 0  
                        //�볬ʱ������־
                        dev_pn512_write_reg(JREG_COMMIRQ, JBIT_TIMERI);
                        if(p_mfcmdinfo->timeoutcnt==0)
                        {
                            //�����ж��Ƿ��յ�����
                            dev_pn512_read_reg(JREG_FIFOLEVEL, &regval);
                            if(((regval&0x7)==0) && (0 == buflen))
                            {
                                if(0 != statusbak)
                                {           
                                    status = statusbak;
                                }
                                else
                                {
                                    status = MI_NOTAGERR;//û��Ŀ��//MI_ACCESSTIMEOUT;          //��ʱ
                                }
                                if(emdretry != 0)
                                {
                                    status = emdretry;
                                }
                                break;
                            }
                            else
                            {
                                //˵�����ڽ���  
                                waitForComm &= ~JBIT_TIMERI;    //���Կ��Ǽ�����PN512�Ķ�ʱ����ʱ
                                //ֹͣ��ʱ��timeflag
                                dev_pn512_modify_reg(JREG_CONTROL, 1, JBIT_TSTOPNOW);  
                                //���ó�ʱ
                                timerid = dev_get_timeID();
                                timerover = 200;      //�ȴ���������

                            }  
                        }//<----if(p_mfcmdinfo->timeoutcnt==0)
                        else
                        {
                            p_mfcmdinfo->timeoutcnt--;
                            //�������ó�ʱ
                            dev_pn512_set_timeout((p_mfcmdinfo->timeoutmode&0xfe), 0x8000);
                            dev_pn512_modify_reg(JREG_CONTROL, 1, JBIT_TSTARTNOW); 

                        }
                      #endif
                        status = MI_NOTAGERR;//û��Ŀ��//MI_ACCESSTIMEOUT;          //��ʱ
                        break;        
                    }//<----if(commirq_reg&JBIT_TIMERI)
                    //=========================================
                    //������Ĵ���
                    if((commirq_reg&JBIT_ERRI))
                    {
                      #if 0  
                        //������Ĵ���
                        if(1 == g_rf_emd_flg)
                        {
                        }
                        else    
                        { 
                            //��ȡ���н��յ��ֽ�����λ��,���ڴ�����
                            dev_pn512_read_reg(JREG_CONTROL, &regval);
                            nbitreceived = (regval & 0x07);

                            dev_pn512_read_reg(JREG_FIFOLEVEL, &regval);
                            regval &= 0x7f;
                            dev_pn512_read_fifo(&buf[buflen], regval);
                            buflen += regval;
                            if((0 != nbitreceived) && (0!=buflen))
                            {
                                buflen--;
                            }
                            dev_pn512_read_reg(JREG_ERROR, &err_reg);


                            //��������־
                            dev_pn512_write_reg(JREG_COMMIRQ, JBIT_ERRI);
                            // ��������������IRQ֮ǰ�������Ĵ���  
                            dev_pn512_write_reg(JREG_ERROR, 0);
                            //����ײ����
                            if(err_reg&JBIT_COLLERR)
                            {
                                status = MI_COLLERR;
                            }

                            //��żУ�����
                            else if(err_reg&JBIT_PARITYERR)
                            {
                                status = MI_PARITYERR;
                            }

                            //Э�����
                            else if(err_reg&JBIT_PROTERR)
                            {
                                status = MI_PROTOCOL_ERROR;
                            }

                            //�������������
                            else if(err_reg&JBIT_BUFFEROVFL)
                            {
                                status = MI_RECBUF_OVERFLOW;
                            }

                            //CRC����
                            else if(err_reg&JBIT_CRCERR)
                            {
                                //CRC������ֻ���յ�1���ֽ���Ϊ��Mifare (N)ACK 
                                if(((0x04 == nbitreceived)||(0x00==nbitreceived)))//stacy
                                {                             
                                    buflen = 1;
                                    status = MI_ACK_SUPPOSED;   //(N)  ACK
                                }
                                else
                                {
                                    status = MI_CRCERR;             //CRC����
                                    //break;
                                } 
                            }

                            //�¶ȹ��ߴ���
                            else if(err_reg&JBIT_TEMPERR)
                            {
                                status = MI_TEMP_ERROR;
                            }

                            //д����
                            else if(err_reg&JBIT_WRERR)
                            {
                                status = MI_WRITEERR;
                            }

                            //������
                            else if(err_reg&JBIT_RDERR)
                            {
                                status = MI_READERR;
                            }
                            //��������
                            else
                            {
                                //if(commirq_reg&JBIT_ERRI)
                                status = MI_OTHERERR;
                            }
                            ///////////////////////////////////////
                            //�ܵ��жϴ���Ĵ���
                            if(MI_OK != status)
                            {    
                                statusbak = status;
                                p_mfcmdinfo->nBitsReceived = nbitreceived;
                                p_mfcmdinfo->nBytesReceived = buflen;
                                memcpy(rbuf, buf, p_mfcmdinfo->nBitsReceived?buflen+1:buflen);
                                break;
                            }                   
                        }
                      #endif  
                        err_reg = 0;   
                    }   

                    //======================================================
                    //�������
                    if(commirq_reg&JBIT_TXI)
                    {
//drv_psam_testpin_xor(2);
                      #if 0  
                        waitForComm &= ~JBIT_TXI;    //�ط�����ɴ���   ?peng
                        //���߱���
                        commIrqEn |= JBIT_HIALERTI;
                        waitForComm |= JBIT_HIALERTI;
                        dev_pn512_write_reg(JREG_COMMIEN, commIrqEn);       //���߱���
                        dev_pn512_write_reg(JREG_COMMIRQ, JBIT_HIALERTI|JBIT_TXI);   //������ͱ�����־           
                        timerid = dev_get_timeID();
                        timerover = (p_mfcmdinfo->timeout+(((u32)p_mfcmdinfo->timeoutcnt)<<15))/2 + 300;//30000;          //30s
                      #endif
                        status = MI_OK;
                        break;
                    }
                  #if 0  
                    //--------------------------------------------------
                    //�������
                    if(commirq_reg&JBIT_RXI)
                    {
                        dev_pn512_read_reg(JREG_ERROR, &err_reg);
                        
                        //������bits��
                        dev_pn512_read_reg(JREG_CONTROL, &regval);
                        nbitreceived= (regval & 0x07);
                        //��������
                        dev_pn512_read_reg(JREG_FIFOLEVEL, &regval);
                        regval &= 0x7f;
                      #if 1  
                        if((1 == g_rf_emd_flg)&&
                           (((err_reg&JBIT_PROTERR)&&((regval+buflen)<=4))
                              ||((err_reg&(JBIT_CRCERR|JBIT_PARITYERR))&&(nbitreceived != 0)&&((regval+buflen)<=3))
                              ||((err_reg&(JBIT_CRCERR|JBIT_PARITYERR))&&((regval+buflen)<3)&&((regval+buflen)!=0))
                              ||((err_reg==0)&&(nbitreceived == 0)&&((regval+buflen)==3)&&(g_rf_special_B_flag))
                              ||((err_reg==4)&&(nbitreceived == 0)&&((regval+buflen)==0)&&(g_rf_special_B_flag))
                            ))
                        {
//drv_psam_testpin_xor(3);
                            //������������
                            dev_pn512_write_reg(JREG_COMMAND, JCMD_RECEIVE);
                            //FLUSH FIFO
                            dev_pn512_write_reg(JREG_FIFOLEVEL, JBIT_FLUSHBUFFER);
                            //����жϱ�־
                            dev_pn512_write_reg(JREG_COMMIRQ, 0x7F);
                            emdretry = MI_CRCERR;
                            nbitreceived = 0;
                            buflen = 0;
                            //����PCD��ʱ 1280/fc=10etu
                          #if 0  
                            if(dev_rf_get_r_s_flg())
                            {
                                p_mfcmdinfo->timeoutcnt=0;
                                dev_pn512_set_timeout(PCD_TIMER_MODE_ETU_0, 200);//2000);  //��֤�ܹ��յ�1�������ֽ�
                            }
                            else
                           #endif     
                            {
                                p_mfcmdinfo->timeoutcnt = timeoutcnttmp;
                                dev_pn512_set_timeout(p_mfcmdinfo->timeoutmode, p_mfcmdinfo->timeout);
                            }
                            dev_pn512_modify_reg(JREG_CONTROL, 1, JBIT_TSTARTNOW); 
                            timerid = dev_get_timeID();
                            //timerover = 3000;
                            timerover = (p_mfcmdinfo->timeout+(((u32)p_mfcmdinfo->timeoutcnt)<<15))/2 + 300;//30000;          //30s
                            //while(0 == dev_rf_int_get());
                            continue;
                        }
                        else
                      #else
                        if((1 == g_rf_emd_flg))
                        {
dev_debug_printf("err=%02X,rlen=%d, bit=%d\r\n", err_reg, regval+buflen, nbitreceived);                            
                            if(((err_reg&JBIT_PROTERR)&&((regval+buflen)<=4))
                                ||((err_reg&(JBIT_CRCERR|JBIT_PARITYERR))&&(nbitreceived != 0)&&((regval+buflen)<=3))
                                ||((err_reg&(JBIT_CRCERR|JBIT_PARITYERR))&&((regval+buflen)<3)&&((regval+buflen)!=0))
                                ||((err_reg==0)&&(nbitreceived == 0)&&((regval+buflen)==3)&&(g_rf_special_B_flag))
                                ||((err_reg==4)&&(nbitreceived == 0)&&((regval+buflen)==0)&&(g_rf_special_B_flag))
                            )
                            {
//drv_psam_testpin_xor(3);
                                //������������
                                dev_pn512_write_reg(JREG_COMMAND, JCMD_RECEIVE);
                                //FLUSH FIFO
                                dev_pn512_write_reg(JREG_FIFOLEVEL, JBIT_FLUSHBUFFER);
                                //����жϱ�־
                                dev_pn512_write_reg(JREG_COMMIRQ, 0x7F);
                                emdretry = MI_CRCERR;
                                nbitreceived = 0;
                                buflen = 0;
                                //����PCD��ʱ 1280/fc=10etu
                                if(dev_rf_get_r_s_flg())
                                {
                                    p_mfcmdinfo->timeoutcnt=0;
                                    dev_pn512_set_timeout(PCD_TIMER_MODE_ETU_0, 200);//2000);  //��֤�ܹ��յ�1�������ֽ�
                                }
                                else
                                {
                                    p_mfcmdinfo->timeoutcnt = timeoutcnttmp;
                                    dev_pn512_set_timeout(p_mfcmdinfo->timeoutmode, p_mfcmdinfo->timeout);
                                }
                                dev_pn512_modify_reg(JREG_CONTROL, 1, JBIT_TSTARTNOW); 
                                timerid = dev_get_timeID();
                                //timerover = 3000;
                                timerover = (p_mfcmdinfo->timeout+(((u32)p_mfcmdinfo->timeoutcnt)<<15))/2 + 300;//30000;          //30s
                                //while(0 == dev_rf_int_get());
                                continue;
                            }
                        }
                      #endif
                        {
                            
                            dev_pn512_read_fifo(&buf[buflen], regval);
                            buflen += regval;
                            if((0 != nbitreceived) && (0!=buflen))
                            {
                                buflen--;
                            }
                            if(err_reg!=0)
                            {
                                // ��������������IRQ֮ǰ�������Ĵ���  
                                dev_pn512_write_reg(JREG_ERROR, 0);
                                
                                //����ײ����
                                if(err_reg&JBIT_COLLERR)
                                {
                                    status = MI_COLLERR;
                                }

                                //��żУ�����
                                else if(err_reg&JBIT_PARITYERR)
                                {
                                    status = MI_PARITYERR;
                                }

                                //Э�����
                                else if(err_reg&JBIT_PROTERR)
                                {
                                    status = MI_PROTOCOL_ERROR;
                                }

                                //�������������
                                else if(err_reg&JBIT_BUFFEROVFL)
                                {
                                    status = MI_RECBUF_OVERFLOW;
                                }

                                //CRC����
                                else if(err_reg&JBIT_CRCERR)
                                {
                                    //CRC������ֻ���յ�1���ֽ���Ϊ��Mifare (N)ACK 
                                    if((0x01 == buflen) && ((0x04 == nbitreceived)||(0x00==nbitreceived)))
                                    {
                                        buflen = 1;
                                        status = MI_ACK_SUPPOSED;   //(N)  ACK
                                    }
                                    else
                                    {
                                        status = MI_CRCERR;             //CRC����
                                        //break;
                                    }
                                }

                                //�¶ȹ��ߴ���
                                else if(err_reg&JBIT_TEMPERR)
                                {
                                    status = MI_TEMP_ERROR;
                                }

                                //д����
                                else if(err_reg&JBIT_WRERR)
                                {
                                    status = MI_WRITEERR;
                                }

                                //������
                                else if(err_reg&JBIT_RDERR)
                                {
                                    status = MI_READERR;
                                }
                                
                            }
                            else
                            {
                                status = MI_OK;
                            }
                            break;
                        }
                    }
                  #endif  
                    //---------------------------------------------
                    //ָ��ִ�����
                    if(commirq_reg&JBIT_IDLEI)
                    {
                        
                        //uart_printf("JBIT_IDLEI:\r\n"); 
                        status = MI_OK;
                        break;
                    }
                    //---------------------------------------------
                    //FIFO�ͱ���,����
                    if(commirq_reg&JBIT_LOALERTI)    
                    {
                        dev_pn512_read_reg(JREG_FIFOLEVEL, &regval);     //��FIFO�����ݳ���
                        regval &= 0x7f;
                        if(regval > 64)
                        {
                            //����,�ݲ�����
                        }              
                        else
                        {
                            regval = (64-regval);
                            if(regval >= (wlen-wcnt))
                            {                          
                                regval = wlen - wcnt;
                                //�صͱ���
                                commIrqEn &= (~JBIT_LOALERTI);
                                waitForComm &= (~JBIT_LOALERTI);        //����1��дFIFO�Ϳ��Է�����
                                dev_pn512_write_reg(JREG_COMMIEN, commIrqEn);
                            } 
                            dev_pn512_write_fifo(&wbuf[wcnt], regval);
                            wcnt += regval;
                        }

                        //��ͱ�����־
                        dev_pn512_write_reg(JREG_COMMIRQ, JBIT_LOALERTI);
                    }
                  #if 0  
                    //---------------------------------------------
                    //FIFO�߱���������
                    if(commirq_reg&JBIT_HIALERTI)    
                    {
                        //��������
                        dev_pn512_read_reg(JREG_FIFOLEVEL, &regval);
                        regval &= 0x7f;
                        dev_pn512_read_fifo(&buf[buflen], regval);
                        buflen += regval; 
                        //��߱�����־
                        dev_pn512_write_reg(JREG_COMMIRQ, JBIT_HIALERTI);
                        if(buflen>256)
                        {
                            status = MI_OVFLERR;
                            break;
                        }
                    }
                  #endif  
                }//<-----if(0 != commirq_reg)
            }//if(0 != dev_rf_int_get())
            //===============================================
            //�жϳ�ʱ,��ֹPN512����
            if(dev_querry_time(timerid, timerover))
            {           
                //ʱ�䵽   
                status = MI_ACCESSTIMEOUT;          //��ʱ
                break;
            }
        }//<----while(1)
        
    }  
    //�رն�ʱ��
    dev_pn512_modify_reg(JREG_CONTROL, 1, JBIT_TSTOPNOW);

    dev_pn512_read_reg(JREG_COMMAND, &regval);
    dev_pn512_write_reg(JREG_COMMAND, regval&0xf0);
    //��������жϼ�IRQ��־
    dev_pn512_write_reg(JREG_COMMIRQ, 0x7f); //waitForComm);
    dev_pn512_write_reg(JREG_DIVIRQ, 0x1f); //waitForDiv);
    //���FIFO
    dev_pn512_write_reg(JREG_FIFOLEVEL, JBIT_FLUSHBUFFER);
    //��ֹ�ж�
    dev_pn512_write_reg(JREG_COMMIEN, 0x80);
    dev_pn512_write_reg(JREG_DIVIEN,  0x80); 
    //��BITFAMEING�Ĵ���
    dev_pn512_write_reg(JREG_BITFRAMING, 0);  
    if(MI_OK == statusbak)
    {        
        //p_mfcmdinfo->nBytesReceived = buflen;
        //p_mfcmdinfo->nBitsReceived = nbitreceived;
        //memcpy(rbuf, buf, p_mfcmdinfo->nBitsReceived?buflen+1:buflen);
    }
    
    //===============================================
//    k_free(buf);
//drv_psam_testpin_set(1, 1);
    return status;
}
#if 0
s8 dev_pn512_selectanticoll_picc_send(u8 mod)
{
    s8 status = MI_OK;
    u8 databfr[20];
    u8 datalen;

    if(0 == mod)
    {
        //93 20
        dev_pn512_modify_reg(JREG_TXMODE, 0, JBIT_CRCEN);
        dev_pn512_modify_reg(JREG_RXMODE, 0, JBIT_CRCEN);
        dev_pn512_write_reg(JREG_COLL, 0);
        databfr[0] = 0x93;
        databfr[1] = 0x20;
        datalen = 2;
    }
    else
    {
        dev_pn512_modify_reg(JREG_TXMODE, 1, JBIT_CRCEN);
        dev_pn512_modify_reg(JREG_RXMODE, 1, JBIT_CRCEN);
        dev_pn512_write_reg(JREG_COLL, JBIT_VALUESAFTERCOLL);  //��λֻ����106kb/s��λ����ͻʱ��0������Ҫ��1
    
        databfr[0] = 0x93;
        databfr[1] = 0x70;
        databfr[2] = 0x27;
        databfr[3] = 0xE9;
        databfr[4] = 0x3B;
        datalen = 5;
    }
    while(complete && (status == MI_OK))
    {
        if(bitcount>COMPLETE_UID_BITS)
        {
            return MI_WRONG_PARAMETER_VALUE;
        }
        if(bitcount==COMPLETE_UID_BITS)
        {
            dev_pn512_modify_reg(JREG_TXMODE, 1, JBIT_CRCEN);
            dev_pn512_modify_reg(JREG_RXMODE, 1, JBIT_CRCEN);
            dev_pn512_write_reg(JREG_COLL, JBIT_VALUESAFTERCOLL);  //��λֻ����106kb/s��λ����ͻʱ��0������Ҫ��1
        }
        if(flg)
        {
            dev_pn512_wait_etu(300);//(54);      //pengxuebin,20090822_1
        }
        else
        {
            flg = 1;
            dev_pn512_wait_etu(60);
        }
        
        nbits = (bitcount%BITS_PER_BYTE);           //��Ҫ���͵ķ�����λ
        nbytes = ((bitcount+7) / BITS_PER_BYTE);    //��Ҫ���͵�����
        
        databfr[0] = PCDTypeASel[grade&0x03];       //��ͬ�ȼ���SEL����
        databfr[1] = 0x20 + (((bitcount/8)<<4)&0x70) + nbits;//NVB
        memcpy(&databfr[2], snr, nbytes);           //��Ҫ���͵�����
        
        //set TxLastBits and RxAlign to number of bits sent
        dev_pn512_write_reg(JREG_BITFRAMING, ((nbits << UPPER_NIBBLE_SHIFT) | nbits));
        
        //prepare data for common transceive
        dev_pcd_reset_mfinfo(&mfcmdinfotmp);        //��λ�ṹ
        mfcmdinfotmp.nBytesToSend = (nbytes+2);
        mfcmdinfotmp.timeoutmode = PCD_TIMER_MODE_ETU_1;//PCD_TIMER_MODE_100US_1;
        mfcmdinfotmp.timeout = 20;//19;//1000;//��ʱ��100etu  //5000;                  //������pengxuebin��????
        mfcmdinfotmp.cmd = JCMD_TRANSCEIVE; 
        
#ifdef PCD_DEBUG   
dev_debug_printf("cascanticoll:send:bytes=%x, bits=%x\r\n", mfcmdinfotmp.nBytesToSend, ((nbits << UPPER_NIBBLE_SHIFT) | nbits));        
//PrintFormat(databfr, mfcmdinfotmp.nBytesToSend);
#endif


        status = dev_pn512_execute_cmd(&mfcmdinfotmp, databfr, rtmp);

#ifdef PCD_DEBUG    
dev_debug_printf("cascanticoll:status=%d,%d,%d\r\n", status, mfcmdinfotmp.nBytesReceived, mfcmdinfotmp.nBitsReceived);        
dev_debug_printf("grade=%d,%d bitcount=%d\r\n", grade, grademax, bitcount);        
PrintFormat(rtmp, mfcmdinfotmp.nBytesReceived); 
#endif

        if(40 == bitcount)
        {
            if(MI_OK == status)
            {               
                if((SAK_LENGTH == mfcmdinfotmp.nBytesReceived)&&(0 == mfcmdinfotmp.nBitsReceived))
                { 
                    *sak = rtmp[0]; 
                }
                else
                {
                    status = MI_BITCOUNTERR;
                }
            }
            else if(MI_NOTAGERR == status)      //20090826_1
            {
                completecnt--;
                if(completecnt<=0)break;
                else{
                    complete=1;
                    status = MI_OK;
                    continue;
                }
            } 
            break;
        }

        
        if((MI_COLLERR == status) || (MI_OK == status))
        {   //���ִ�гɹ�����ײ����, ��ִ��

            #ifdef PCD_DEBUG   
            dev_debug_printf("rec:bytes=%x,Bits=%x\r\n", mfcmdinfotmp.nBytesReceived, mfcmdinfotmp.nBitsReceived);
            if(0 != mfcmdinfotmp.nBitsReceived)
            {
                PrintFormat(rtmp, mfcmdinfotmp.nBytesReceived+1);
            }
            else
            {
                PrintFormat(rtmp, mfcmdinfotmp.nBytesReceived);
            }
            #endif


            rbits = (mfcmdinfotmp.nBitsReceived + (mfcmdinfotmp.nBytesReceived << 3) - nbits);  //������յ�λ��
            if((rbits+bitcount) > COMPLETE_UID_BITS)   //40
            {
                status = MI_BITCOUNTERR;                //λͳ�ƴ���
                break;
            }
            
            //increment number of bytes received if also some bits received
            if(mfcmdinfotmp.nBitsReceived)
            {
                mfcmdinfotmp.nBytesReceived++;
            }
            
            //reset offset for data copying
            byteOffset = 0;
            
            //if number of bits sent are not 0, write first received byte in last of sent
            if(nbits)
            {   //last byte transmitted and first byte received are the same
                snr[nbytes-1] |= rtmp[0];           //pengxuebin, ??
                byteOffset++;
            }

            memcpy(&snr[nbytes], &rtmp[byteOffset], mfcmdinfotmp.nBytesReceived);//4-nbytes);  //??
            
            if(MI_COLLERR == status)
            {
                //calculate new bitcount value
                bitcount = (bitcount + rbits);
            }
            else    //û�г�ͻ
            {
                if((rbits+bitcount) == COMPLETE_UID_BITS)   //40
                {
            
                    #ifdef PCD_DEBUG   
                        Uart_Printf("BCC:nbits=%x\r\n", nbits);
 //                       PrintFormat(snr, (4-j));   
                    #endif             

                    nbits = 0;
                    for(i=0; i<5; i++)
                    {
                        nbits ^= snr[i];
                    }
                    if(0 == nbits)
                    {  
                      
                        //˵������ͻ�ɹ�
                        bitcount = COMPLETE_UID_BITS;// += rbits;
                        complete = 1;                //ִ��select
                        continue;
                    }
                    else
                    {
                        status = MI_SERNRERR;
                        break;
                    }
                }
                else
                {
                    status = MI_SERNRERR;
                    break;
                }
            } 
        }
        else if(MI_NOTAGERR == status)
        {
            completecnt--;
            if(completecnt<=0)break;
            else{
                complete++;
                status = MI_OK;
            }
        }
        else
        {
            break;
        }
        complete --;
    }

    
    if((0 == complete))             //ѭ����������32�λ�û�гɹ�
    {
        status = MI_ANTICOLL_ERR;
        
    }
    return status;
}
#endif
