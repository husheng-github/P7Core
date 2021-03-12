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
 ** Descriotions:   控制PN512扩展GPIO口
 ** parameters:                                                              
 ** Returned value: 0:失败
                    -1:成功  
                                                                                              
 ** Created By:     彭学斌 
 ** Remarks:                                                                                  
*******************************************************************************/
void dev_pn512_export_init(void)
{
  #if 0  
    u8 reg;
    
    //配置IO输出
    reg = PN512_EXPORT_EN;
    dev_pn512_write_reg(JREG_TESTPINEN, reg);
    //输出清0
 //   g_rf_export = 0;
    dev_pn512_write_reg(JREG_TESTPINVALUE, g_rf_export|0x80);
  #endif  
}
void dev_pn512_export_deinit(void)
{
  #if 0  
    u8 reg;
    
    //输出清0
    g_rf_export = 0;
    dev_pn512_write_reg(JREG_TESTPINVALUE, g_rf_export|0x80);

    //配置IO输出
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
 ** Descriotions:   读PN512版本
 ** parameters:                                                              
 ** Returned value: 0:失败
                    -1:成功  
                                                                                              
 ** Created By:     彭学斌 
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
    if((*ver== 0x82)        //pn512版本
     ||(*ver == 0x12))      //兆讯MH1608
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
 ** Descriotions:   读PN512版本
 ** parameters:                                                              
 ** Returned value: 0:失败
                    1:成功  
                                                                                              
 ** Created By:     彭学斌 
 ** Remarks:                                                                                  
*******************************************************************************/
void dev_pn512_reset(s32 _ms)
{
    dev_pn512_write_reg(JREG_COMMAND, JCMD_SOFTRESET);
    dev_pn512_export_init();
//    delayms(_ms);
}

/*---------------------------------------
    //控制CRC使能
    mask  : 需要控制的部分
       bit0: 控制TXCRC，该位为1，表示需要控制，为0不需要控制
       bit1: 控制RXCRC, 该位为1，表示需要控制，为0不需要控制
    mod   :指定控制的模式，
       bit0: 控制TXCRC，如果mask的bit0为1，则该位为1，打开TXCRC，为0，则关闭TXCRC
                        如果mask的bit0为0，则不处理
       bit1: 控制RXCRC，如果mask的bit1为1，则该位为1，打开RXCRC，为0，则关闭RXCRC
                        如果mask的bit1为0，则不处理                 
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
 ** Descriotions:   设置PCD工作模式
 ** parameters:     type:PICC_TYPEA:Type A
                         PICC_TYPEB:TypeB 
                         PICC_TYPEC: TypeC(FeliCa)  
                    confpara:配置相关参数
                    {对于TypeB: cfgpara[0]<->JREG_MODGSP
                                cfgpara[1]<->JREG_GSN
                                cfgpara[2]<->JREG_GSNOFF
                    }                                              
 ** Returned value: 0: 成功
                   其它:失败                                                                           
 ** Created By:     彭学斌           
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
        dev_pn512_write_reg(JREG_CWGSP, 0x3f);          // 发送最大射频场

        dev_pn512_write_reg(JREG_MODGSP, 0x0F);     // 10%ASK
        dev_pn512_write_reg(JREG_TXAUTO, 0x07);
        dev_pn512_read_reg(JREG_TXAUTO, &regval);
        //if(regval != 0x37)
        //{
        //    return MI_INITERR;                      //初始化错误
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
            dev_pn512_write_reg(JREG_TXAUTO, 0x40);     /*强制100%ASK*/
            dev_pn512_write_reg(JREG_TXMODE, 0x00); // TypeA baud 106kbps
            dev_pn512_write_reg(JREG_RXMODE, 0x08); // 少于4位的接收无效，TypeA baud 106kbps
            dev_pn512_write_reg(JREG_DEMOD, 0x4D);
            dev_pn512_write_reg(JREG_CWGSP, 0x3f);//0x20);//0x3f);      // 发送最大射频场
            dev_pn512_write_reg(JREG_RXTRESHOLD, g_rf_param.rxthreshold_a);//0x84); /* MinLevel = 5; CollLevel = 5 */
            dev_pn512_write_reg(JREG_RFCFG, g_rf_param.rfcfgregA);//0x48);       /* RxGain = 6*/
            dev_pn512_write_reg(JREG_TYPEB, 0x00);
            dev_pn512_read_reg(JREG_RXSEL,&regval);
            dev_pn512_write_reg(JREG_RXSEL,(regval&0xC0)|0x08);
            dev_pn512_read_reg(JREG_TXAUTO, &regval);

            if(regval != 0x40)
            {                
                return MI_INITERR;                      //初始化错误
            }
        }
        else if(PICC_TYPEB == type)
        {
            dev_pn512_write_reg(JREG_TXAUTO, 0x00);     /*取消100%ASK*/
            dev_pn512_write_reg(JREG_TXMODE, 0x03);     // TypeB baud 106kbps
            dev_pn512_write_reg(JREG_RXMODE, 0x0b); // 少于4位的接收无效，TypeB baud 106kbps
            dev_pn512_write_reg(JREG_TYPEB, 0xC0);  //0x1f); // // 接收需要SOF和EOF,用最小长度的SOF和EOF,发送SOF和EOF,EGT=0

            dev_pn512_write_reg(JREG_DEMOD, 0x4d);
            dev_pn512_write_reg(JREG_CWGSP, 0x3f);      // 发送最大射频场
            // JREG_MODGSP: 0--0x1f B型卡信号调制深度，需要改成接收命令可调整，
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
    dev_pn512_write_reg(JREG_WATERLEVEL, DEV_PN512_WATERLEVEL);       //报警设为32字节
    regval = dev_rf_get_chiptype();
    if(regval == PCD_CHIP_TYPE_MH1608)
    {
      #if 0
        /*//过EMV 时的配置
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
        /*//读卡时的配置
        {
            write_reg(0x37, 0x5e);
            write_reg(0x3B, 0xE5); //differ
            write_reg(0x35, 0xED); // Auto AGC //是否使用自动agc根据实际刷卡效果选则
            //write_reg(0x35, 0x08); // Manul AGC
            write_reg(0x37, 0xAE);
            write_reg(0x3b, 0x72); //differ
            write_reg(0x37, 0x00);
        }
      
        */
        dev_pn512_write_reg(JREG_VERSION, 0x5E);//(0x37, 0x5E);
        dev_pn512_write_reg(JREG_TESTADC, 0xE5);//(0x3B, 0xE5) //differ
        dev_pn512_write_reg(JREG_TESTBUS, 0xED);//(0x35, 0xED); // Auto AGC
      #if 1  //20181221 兆讯欧工提议增加
        dev_pn512_write_reg(JREG_TESTDAC2, 0x00);//(0x3A, );  0x00或0x03  
      #endif  
        //dev_pn512_write_reg(JREG_TESTBUS, 0x08);//(0x35, 0x08); // Manul AGC
        dev_pn512_write_reg(JREG_VERSION, 0xAE);//(0x37, 0xAE);
        dev_pn512_write_reg(JREG_TESTDAC2, 0x72);//(0x3A, 0x72);//differ
        dev_pn512_write_reg(JREG_VERSION, 0x00);//(0x37, 0x00);

      #endif
    }
 //   delayms(6);    //5      //等待场域稳定 //20090824_1
    return MI_OK;
  
}
/*******************************************************************************
 ** Descriotions:   设置超时时间
 ** parameters:     mode:超时时间方式
                         0: 100us计时
                         1: 以etu为单位计时
                         其它保留
                    timer:定时值                                                         
 ** Returned value: 无                                                                         
 ** Created By:     彭学斌 2009-6-30 15:32:05           
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
 ** Descriotions:   设置超时时间
 ** parameters:     mode:超时时间方式
                         0: 100us计时
                         1: 以etu为单位计时
                         其它保留
                    timer:定时值                                                         
 ** Returned value: 无                                                                         
 ** Created By:     彭学斌 2009-6-30 15:32:05           
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
    dev_pn512_modify_reg(JREG_CONTROL, 1, JBIT_TSTOPNOW);//关闭定时器
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
    //清除所有中断标志
    dev_pn512_write_reg(JREG_COMMIRQ, 0x7F);
    dev_pn512_write_reg(JREG_DIVIRQ,  0x1F);
    dev_pn512_read_reg(JREG_COMMIEN, &regval);
    dev_pn512_write_reg(JREG_COMMIEN, regval|JBIT_TIMERI);//使能定时器中断
    dev_pn512_set_timeout_etu(etu);
     //启动定时器
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
    //关闭定时器
    dev_pn512_modify_reg(JREG_CONTROL, 1, JBIT_TSTOPNOW);
    //禁止定时器中断，清除定时器中断标志 
    dev_pn512_modify_reg(JREG_COMMIEN, 0, JBIT_TIMERI);
    dev_pn512_write_reg(JREG_COMMIRQ, JBIT_TIMERI);
    
}
/*******************************************************************************
 ** Descriotions:  卡片交互 
 ** parameters:                                                           
 ** Returned value: 无                                                                         
 ** Created By:     彭学斌 2009-6-30 15:32:05           
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
    //    u8 doReceive = 0;         //处理接收标志
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
    清除所有中断标志
    */   
    dev_pn512_write_reg(JREG_COMMIRQ, 0x7F);
    dev_pn512_write_reg(JREG_DIVIRQ,  0x1F);
    //取消命令
    dev_pn512_read_reg(JREG_COMMAND, &regval);
    regval = (regval & (~JMASK_COMMAND));
    dev_pn512_write_reg(JREG_COMMAND, regval);

    buf =(u8*)k_malloc(300);
    if(buf == NULL)
    {
        dev_debug_printf("申请内存失败\r\n");
        return MI_UNKNOWN_COMMAND;
    }
    timeoutcnttmp = p_mfcmdinfo->timeoutcnt;
    //timeouttmp = p_mfcmdinfo->timeout;
    //-----------------------------------------------
    //根据指令处理数据
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
            //设置等待MASK
            commIrqEn   = JBIT_RXI | JBIT_IDLEI| JBIT_LOALERTI | JBIT_ERRI | JBIT_TIMERI | JBIT_TXI; //JBIT_RXI | JBIT_TIMERI;  //| JBIT_ERRI;
            waitForComm = JBIT_RXI | JBIT_IDLEI| JBIT_LOALERTI | JBIT_ERRI | JBIT_TIMERI | JBIT_TXI; //所有的中断信号处理
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
        //发送命令
        dev_pn512_read_reg(JREG_COMMAND, &regval);
        regval = (regval & (~JMASK_COMMAND)) | p_mfcmdinfo->cmd;
        dev_pn512_write_reg(JREG_COMMAND, regval);
    }
    else
    {
        //清FIFO
        dev_pn512_write_reg(JREG_FIFOLEVEL, JBIT_FLUSHBUFFER);
        regval = 64;                              //FIFO为64Bytes
        if(regval >= p_mfcmdinfo->nBytesToSend)
        {
            regval = p_mfcmdinfo->nBytesToSend;
            //关低报警
            commIrqEn &= (~JBIT_LOALERTI);
            waitForComm &= (~JBIT_LOALERTI);        //数据1次写FIFO就可以发送完
        }
        dev_pn512_write_fifo(wbuf, regval);

        wcnt = regval;
        dev_pn512_write_reg(JREG_COMMIRQ, JBIT_LOALERTI|JBIT_HIALERTI);//清除低报警标志

        p_mfcmdinfo->nBytesReceived = 0;
        p_mfcmdinfo->nBitsReceived = 0;
        p_mfcmdinfo->ErrorReg = 0;

        //设置定时器
        dev_pn512_set_timeout(p_mfcmdinfo->timeoutmode, p_mfcmdinfo->timeout);
        commIrqEn |= JBIT_TIMERI;
        waitForComm |= JBIT_TIMERI;
        
        //使能所需要的中断
        dev_pn512_write_reg(JREG_COMMIEN, commIrqEn);
        //发送命令
        dev_pn512_read_reg(JREG_COMMAND, &regval);
        regval = (regval & (~JMASK_COMMAND))|p_mfcmdinfo->cmd;


        // 若错误发生，则在IRQ之前清除错误寄存器  
        // dev_pn512_write_reg(JREG_ERROR, 0);//?
        dev_pn512_write_reg(JREG_COMMAND, regval);

        //对于TRANSCEIVE命令，开始发送数据
        if(JCMD_TRANSCEIVE == p_mfcmdinfo->cmd)
        {               
          #if 0  //1608对于106kbps没有多帧接收
            //这里可以考虑使用RxMulotiple 
            if(1 == g_rf_emd_flg)
            {
                dev_pn512_modify_reg(JREG_RXMODE, 1, JBIT_RXMULOTIPLE);
            }
          #endif  
            /*TRx is always an endless loop, Initiator and Target must set STARTSEND.*/
            dev_pn512_modify_reg(JREG_BITFRAMING, 1, JBIT_STARTSEND);   //开始发送
            dev_pn512_write_reg(JREG_COMMIRQ, 0x7F);
       }
        else
        {
            //启动定时器
            dev_pn512_modify_reg(JREG_CONTROL, 1, JBIT_TSTARTNOW); 
        }
        timerid = dev_get_timeID();
        timerover = 100;
        //===================下面开始循环======================
        while(1)
        {
            //if(0 != dev_rf_int_get())
            {              
                //读状态寄存器
                dev_pn512_read_reg(JREG_COMMIRQ, &commirq_reg); 
                commirq_reg &= waitForComm;

                if(0 != commirq_reg)
                {
                    //----------------------------------------
                    //超时处理
                    if(commirq_reg&JBIT_TIMERI)
                    {
                        //请超时报警标志
                        dev_pn512_write_reg(JREG_COMMIRQ, JBIT_TIMERI);
                        if(p_mfcmdinfo->timeoutcnt==0)
                        {
                            //增加判断是否收到数据
                            dev_pn512_read_reg(JREG_FIFOLEVEL, &regval);
                            if(((regval&0x7)==0) && (0 == buflen))
                            {
                                if(0 != statusbak)
                                {           
                                    status = statusbak;
                                }
                                else
                                {
                                    status = MI_NOTAGERR;//没有目标//MI_ACCESSTIMEOUT;          //超时
                                }
                                if(emdretry != 0)
                                {
                                    status = emdretry;
                                }
                                break;
                            }
                            else
                            {
                                //说明正在接收  
                                waitForComm &= ~JBIT_TIMERI;    //可以考虑继续用PN512的定时器定时
                                //停止定时器timeflag
                                dev_pn512_modify_reg(JREG_CONTROL, 1, JBIT_TSTOPNOW);  
                                //设置超时
                                timerid = dev_get_timeID();
                                timerover = 200;      //等待接收数据

                            }
                        }//<----if(p_mfcmdinfo->timeoutcnt==0)
                        else
                        {
                            p_mfcmdinfo->timeoutcnt--;
                            //重新设置超时
                            dev_pn512_set_timeout((p_mfcmdinfo->timeoutmode&0xfe), 0x8000);
                            dev_pn512_modify_reg(JREG_CONTROL, 1, JBIT_TSTARTNOW); 

                        }
                    }//<----if(commirq_reg&JBIT_TIMERI)
                    //=========================================
                    //读错误寄存器
                    if((commirq_reg&JBIT_ERRI))
                    {
                        //读错误寄存器
                        if(1 == g_rf_emd_flg)
                        {
                        }
                        else    
                        { 
                            //读取所有接收的字节数和位数,用于错误检测
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


                            //清除错误标志
                            dev_pn512_write_reg(JREG_COMMIRQ, JBIT_ERRI);
                            // 若错误发生，则在IRQ之前清除错误寄存器  
                            dev_pn512_write_reg(JREG_ERROR, 0);
                            //防碰撞错误
                            if(err_reg&JBIT_COLLERR)
                            {
                                status = MI_COLLERR;
                            }

                            //奇偶校验错误
                            else if(err_reg&JBIT_PARITYERR)
                            {
                                status = MI_PARITYERR;
                            }

                            //协议错误
                            else if(err_reg&JBIT_PROTERR)
                            {
                                status = MI_PROTOCOL_ERROR;
                            }

                            //缓冲区溢出错误
                            else if(err_reg&JBIT_BUFFEROVFL)
                            {
                                status = MI_RECBUF_OVERFLOW;
                            }

                            //CRC错误
                            else if(err_reg&JBIT_CRCERR)
                            {
                                //CRC错误且只接收到1个字节认为是Mifare (N)ACK 
                                if(((0x04 == nbitreceived)||(0x00==nbitreceived)))//stacy
                                {                             
                                    buflen = 1;
                                    status = MI_ACK_SUPPOSED;   //(N)  ACK
                                }
                                else
                                {
                                    status = MI_CRCERR;             //CRC错误
                                    //break;
                                } 
                            }

                            //温度过高错误
                            else if(err_reg&JBIT_TEMPERR)
                            {
                                status = MI_TEMP_ERROR;
                            }

                            //写错误
                            else if(err_reg&JBIT_WRERR)
                            {
                                status = MI_WRITEERR;
                            }

                            //读错误
                            else if(err_reg&JBIT_RDERR)
                            {
                                status = MI_READERR;
                            }
                            //其它错误
                            else
                            {
                                //if(commirq_reg&JBIT_ERRI)
                                status = MI_OTHERERR;
                            }
                            ///////////////////////////////////////
                            //总的判断错误的处理
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
                    //发送完成
                    if(commirq_reg&JBIT_TXI)
                    {
                        waitForComm &= ~JBIT_TXI;    //关发送完成处理   ?peng
                        //开高报警
                        commIrqEn |= JBIT_HIALERTI;
                        waitForComm |= JBIT_HIALERTI;
                        dev_pn512_write_reg(JREG_COMMIEN, commIrqEn);       //开高报警
                        dev_pn512_write_reg(JREG_COMMIRQ, JBIT_HIALERTI|JBIT_TXI);   //清除发送报警标志           
                        timerid = dev_get_timeID();
                        timerover = (p_mfcmdinfo->timeout+(((u32)p_mfcmdinfo->timeoutcnt)<<15))/2 + 300;//30000;          //30s
                    }
                    //--------------------------------------------------
                    //接收完成
                    if(commirq_reg&JBIT_RXI)
                    {
                        dev_pn512_read_reg(JREG_ERROR, &err_reg);
                        
                        //读接收bits数
                        dev_pn512_read_reg(JREG_CONTROL, &regval);
                        nbitreceived= (regval & 0x07);
                        //接收数据
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
                            //重新启动接收
                            dev_pn512_write_reg(JREG_COMMAND, JCMD_RECEIVE);
                            //FLUSH FIFO
                            dev_pn512_write_reg(JREG_FIFOLEVEL, JBIT_FLUSHBUFFER);
                            //清除中断标志
                            dev_pn512_write_reg(JREG_COMMIRQ, 0x7F);
                            emdretry = MI_CRCERR;
                            nbitreceived = 0;
                            buflen = 0;
                            //设置PCD定时 1280/fc=10etu
                            p_mfcmdinfo->timeoutcnt=0;
                            dev_pn512_set_timeout(PCD_TIMER_MODE_ETU_0, 2000);  //保证能够收到1个完整字节
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
                                // 若错误发生，则在IRQ之前清除错误寄存器  
                                dev_pn512_write_reg(JREG_ERROR, 0);
                                
                                //防碰撞错误
                                if(err_reg&JBIT_COLLERR)
                                {
                                    status = MI_COLLERR;
                                }

                                //奇偶校验错误
                                else if(err_reg&JBIT_PARITYERR)
                                {
                                    status = MI_PARITYERR;
                                }

                                //协议错误
                                else if(err_reg&JBIT_PROTERR)
                                {
                                    status = MI_PROTOCOL_ERROR;
                                }

                                //缓冲区溢出错误
                                else if(err_reg&JBIT_BUFFEROVFL)
                                {
                                    status = MI_RECBUF_OVERFLOW;
                                }

                                //CRC错误
                                else if(err_reg&JBIT_CRCERR)
                                {
                                    //CRC错误且只接收到1个字节认为是Mifare (N)ACK 
                                    if((0x01 == buflen) && ((0x04 == nbitreceived)||(0x00==nbitreceived)))
                                    {
                                        buflen = 1;
                                        status = MI_ACK_SUPPOSED;   //(N)  ACK
                                    }
                                    else
                                    {
                                        status = MI_CRCERR;             //CRC错误
                                        //break;
                                    }
                                }

                                //温度过高错误
                                else if(err_reg&JBIT_TEMPERR)
                                {
                                    status = MI_TEMP_ERROR;
                                }

                                //写错误
                                else if(err_reg&JBIT_WRERR)
                                {
                                    status = MI_WRITEERR;
                                }

                                //读错误
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
                    //指令执行完成
                    if(commirq_reg&JBIT_IDLEI)
                    {
                        
                        //uart_printf("JBIT_IDLEI:\r\n"); 
                        status = MI_OK;
                        break;
                    }
                    //---------------------------------------------
                    //FIFO低报警,发送
                    if(commirq_reg&JBIT_LOALERTI)    
                    {
                        dev_pn512_read_reg(JREG_FIFOLEVEL, &regval);     //读FIFO中数据长度
                        regval &= 0x7f;
                        if(regval > 64)
                        {
                            //错误,暂不处理
                        }              
                        else
                        {
                            regval = (64-regval);
                            if(regval >= (p_mfcmdinfo->nBytesToSend-wcnt))
                            {                          
                                regval = p_mfcmdinfo->nBytesToSend - wcnt;
                                //关低报警
                                commIrqEn &= (~JBIT_LOALERTI);
                                waitForComm &= (~JBIT_LOALERTI);        //数据1次写FIFO就可以发送完
                                dev_pn512_write_reg(JREG_COMMIEN, commIrqEn);
                            } 
                            dev_pn512_write_fifo(&wbuf[wcnt], regval);
                            wcnt += regval;
                        }

                        //清低报警标志
                        dev_pn512_write_reg(JREG_COMMIRQ, JBIT_LOALERTI);
                    }
                    //---------------------------------------------
                    //FIFO高报警，接收
                    if(commirq_reg&JBIT_HIALERTI)    
                    {
                        //接收数据
                        dev_pn512_read_reg(JREG_FIFOLEVEL, &regval);
                        regval &= 0x7f;
                        dev_pn512_read_fifo(&buf[buflen], regval);
                        buflen += regval; 
                        //清高报警标志
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
            //判断超时,防止PN512死锁
            if(dev_querry_time(timerid, timerover))
            {           
                //时间到   
                status = MI_ACCESSTIMEOUT;          //超时
                break;
            }
        }//<----while(1)
        
    }  
    //关闭定时器
    dev_pn512_modify_reg(JREG_CONTROL, 1, JBIT_TSTOPNOW);

    dev_pn512_read_reg(JREG_COMMAND, &regval);
    dev_pn512_write_reg(JREG_COMMAND, regval&0xf0);
    //清除所有中断及IRQ标志
    dev_pn512_write_reg(JREG_COMMIRQ, 0x7f); //waitForComm);
    dev_pn512_write_reg(JREG_DIVIRQ, 0x1f); //waitForDiv);
    //清除FIFO
    dev_pn512_write_reg(JREG_FIFOLEVEL, JBIT_FLUSHBUFFER);
    //禁止中断
    dev_pn512_write_reg(JREG_COMMIEN, 0x80);
    dev_pn512_write_reg(JREG_DIVIEN,  0x80); 
    //清BITFAMEING寄存器
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
    
    //准备数据
    databfr[0] = key_type;     //key A or key B
    databfr[1] = block;          //address to authentication
    memcpy(databfr+2, key, 6);  //6bytes key
    memcpy(databfr+8, snr, 4);  //4bytes UID
    
    //prepare data for common transceive
    dev_pcd_reset_mfinfo(&mfcmdinfotmp);    //复位结构
    mfcmdinfotmp.nBytesToSend = 12;         //数据长度
    mfcmdinfotmp.timeoutmode = PCD_TIMER_MODE_100US_1;
    mfcmdinfotmp.timeout = 50;              //待定，pengxuebin，????
    mfcmdinfotmp.cmd = JCMD_AUTHENT;
    status = dev_pn512_execute_cmd(&mfcmdinfotmp, databfr, rtmp);
    //   改到函数内部处理  
    if(MI_OK == status)
    {
        dev_pn512_read_reg(JREG_STATUS2, &regval);
        if(0x08 != (regval&0x0f))
        {
            status = MI_AUTHERR;            //认证错
        }
    }
    return status;
    
}
/*******************************************************************************
 ** Descriotions: 等待netu   
 ** parameters:     
                     
 ** Returned value: 
 ** Created By:     
 ** Remarks:       
*******************************************************************************/
void dev_pn512_wait_netu(u16 netu)
{
    u8 regval;
    u32 id;
    //清除所有中断标志
    dev_pn512_write_reg(JREG_COMMIRQ, 0x7F);
    dev_pn512_write_reg(JREG_DIVIRQ,  0x1F);
    dev_pn512_read_reg(JREG_COMMIEN, &regval);
    dev_pn512_write_reg(JREG_COMMIEN, regval|JBIT_TIMERI);//使能定时器中断
    dev_pn512_set_timeout(PCD_TIMER_MODE_nETU_0, netu);
     //启动定时器
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
    //关闭定时器
    dev_pn512_modify_reg(JREG_CONTROL, 1, JBIT_TSTOPNOW);
    //禁止定时器中断，清除定时器中断标志 
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
 ** Descriotions:   发送REQA，查询磁场中是否有符合ISO14443A卡
 ** parameters:     无                                              
 ** Returned value: 0: 成功
                   其它:失败                                                                           
 ** Created By:     彭学斌 2009-7-2 11:14:35
 ** Remarks:     
///////////////////////////////////////////////////////////////////////
// Request Command defined in ISO14443(MIFARE)
// Request,Anticoll,Select,return CardType(2 bytes)+CardSerialNo(4 bytes)    
// 寻卡，防冲突，选择卡    返回卡类型（2 bytes）+ 卡系列号(4 bytes)
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
    //初始化
    dev_pn512_modify_reg(JREG_STATUS2, 0, JBIT_CRYPTO1ON);  //disable Crypto if activated before 
    dev_pn512_write_reg(JREG_COLL, JBIT_VALUESAFTERCOLL);   //active values after coll 激活
    dev_pn512_modify_reg(JREG_TXMODE, 0, JBIT_CRCEN);       //disable TxCRC and RxCRC
    dev_pn512_modify_reg(JREG_RXMODE, 0, JBIT_CRCEN);
    dev_pn512_write_reg(JREG_BITFRAMING, REQUEST_BITS);     //设置最后一字节传输的位数(7位)
    
    //设置需要传输的参数
    dev_pcd_reset_mfinfo(&mfcmdinfotmp);
    wtmp[0] = req_code;
    mfcmdinfotmp.cmd = JCMD_TRANSCEIVE;
    mfcmdinfotmp.nBytesToSend = 1;
    //设置超时
    mfcmdinfotmp.timeoutmode = PCD_TIMER_MODE_ETU_1;//PCD_TIMER_MODE_100US_1;
    mfcmdinfotmp.timeout = 40;       //10etu     //400us, 待定 pengxuebin，  ????
    
    status = dev_pn512_execute_cmd(&mfcmdinfotmp, wtmp, rtmp);

   #ifdef PCD_DEBUG  
    rf_debug_printf("reqa:%d\r\n", status);    
   #endif                               
    if(MI_OK == status)
    {
        if(mfcmdinfotmp.nBytesReceived != 2)
        {
            status = MI_BYTECOUNTERR;           //字节长度错误
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
 ** Descriotions:   使piccA卡进入HALT状态
 ** parameters:     
                     sel_code   command code
                     bitcount   the bit counter of known UID
                     snr        the UID have known
                     sak        the byte to save the ACK from card                                              
 ** Returned value:  0: 成功
                    其它:失败 
 ** Created By:     彭学斌 2009-7-2 11:14:35
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

    dev_pcd_reset_mfinfo(&mfcmdinfotmp);    //复位结构
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
 ** parameters:     afi : 代表由PCD所瞄准的应用类型
                    param:参数
 ** Returned value:  0: 成功
                    其它:失败 
 ** Created By:     彭学斌 2009-7-7 11:00:17
 ** Remarks:       
*******************************************************************************/
s8 dev_pn512_req_piccB(u8 afi, u8 N, u8 req_code, u8 *atqB, u8 *atqBlen)
{
    s8 status = MI_OK; 
    u8 tmp[16];
    u8 tmp1[64];
    MfCmdInfo mfcmdinfotmp;

    g_rf_emd_flg = 0;
    //activate_deletion of bits after coll //激活冲突检测
    dev_pn512_write_reg(JREG_COLL, 0);
    
    *atqBlen = 0;
    
    tmp[0] = 0x05;          //Apf
    tmp[1] = afi;           //afi
    //param:bit5=1,扩展ATQB支持
    tmp[2] = (req_code&0x08)|(N&0X07);//param;         //afi
    //prepare data for common transceive
    dev_pcd_reset_mfinfo(&mfcmdinfotmp);        //复位结构
    mfcmdinfotmp.nBytesToSend = 3;              //数据长度
    mfcmdinfotmp.timeoutmode = PCD_TIMER_MODE_ETU_1;
    mfcmdinfotmp.timeout = 100;//50;
    mfcmdinfotmp.cmd = JCMD_TRANSCEIVE; 
    status = dev_pn512_execute_cmd(&mfcmdinfotmp, tmp, tmp1);

    if(MI_OK == status)
    {
        if(mfcmdinfotmp.nBytesReceived >= 12)
        {          //字节长度错误
            memcpy(atqB, tmp1, mfcmdinfotmp.nBytesReceived);
            *atqBlen = mfcmdinfotmp.nBytesReceived;
        }
        else 
        {
            status = MI_BITCOUNTERR;    //接收错误
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
 ** Descriotions:   防冲突选择卡(Functions to split anticollission and select internally)  
 ** parameters:     
                     sel_code   command code
                     bitcount   the bit counter of known UID
                     snr        the UID have known
                     sak        the byte to save the ACK from card                                              
 ** Returned value:  0: 成功
                    其它:失败 
 ** Created By:     彭学斌 2009-7-2 11:14:35
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
    else if(bitcount==COMPLETE_UID_BITS)           //发送的数据
    {
        complete = 1;                          
    }
    else
    {
        dev_pn512_modify_reg(JREG_TXMODE, 0, JBIT_CRCEN);
        dev_pn512_modify_reg(JREG_RXMODE, 0, JBIT_CRCEN);
        dev_pn512_write_reg(JREG_COLL, 0);
        complete = 1;//32;                          //最大32次循环
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
            dev_pn512_write_reg(JREG_COLL, JBIT_VALUESAFTERCOLL);  //该位只有在106kb/s的位防冲突时置0，其他要置1
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
        
        nbits = (bitcount%BITS_PER_BYTE);           //需要发送的非完整位
        nbytes = ((bitcount+7) / BITS_PER_BYTE);    //需要发送的数据
        
        databfr[0] = PCDTypeASel[grade&0x03];       //不同等级的SEL代码
        databfr[1] = 0x20 + (((bitcount/8)<<4)&0x70) + nbits;//NVB
        memcpy(&databfr[2], snr, nbytes);           //需要发送的数据
        
        //set TxLastBits and RxAlign to number of bits sent
        dev_pn512_write_reg(JREG_BITFRAMING, ((nbits << UPPER_NIBBLE_SHIFT) | nbits));
        
        //prepare data for common transceive
        dev_pcd_reset_mfinfo(&mfcmdinfotmp);        //复位结构
        mfcmdinfotmp.nBytesToSend = (nbytes+2);
        mfcmdinfotmp.timeoutmode = PCD_TIMER_MODE_ETU_1;//PCD_TIMER_MODE_100US_1;
        mfcmdinfotmp.timeout = 20;//19;//1000;//暂时定100etu  //5000;                  //待定，pengxuebin，????
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
        {   //如果执行成功或碰撞错误, 则执行

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


            rbits = (mfcmdinfotmp.nBitsReceived + (mfcmdinfotmp.nBytesReceived << 3) - nbits);  //计算接收的位数
            if((rbits+bitcount) > COMPLETE_UID_BITS)   //40
            {
                status = MI_BITCOUNTERR;                //位统计错误
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
            else    //没有冲突
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
                      
                        //说明防冲突成功
                        bitcount = COMPLETE_UID_BITS;// += rbits;
                        complete = 1;                //执行select
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

    
    if((0 == complete))             //循环次数到了32次还没有成功
    {
        status = MI_ANTICOLL_ERR;
        
    }
    return status;
}
/*******************************************************************************
 ** Descriotions:   RATS(请求ATS)
 ** parameters:     rbuf: 保存ATS
                    rlen: ATS长度 
 ** Returned value:  0: 成功
                    其它:失败 
 ** Created By:     彭学斌 2009-7-7 11:00:17
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
        dev_pcd_reset_mfinfo(&mfcmdinfotmp);        //复位结构
        mfcmdinfotmp.nBytesToSend = 2;              //数据长度
        mfcmdinfotmp.timeoutmode = PCD_TIMER_MODE_ETU_1;//PCD_TIMER_MODE_100US_1;
        mfcmdinfotmp.timeout = 600;//20090822_1
       
        mfcmdinfotmp.cmd = JCMD_TRANSCEIVE;
        tmp1[0]=0x00;
        status = dev_pn512_execute_cmd(&mfcmdinfotmp, tmp, tmp1);
 
        if((MI_OK == status)&&(0 != mfcmdinfotmp.nBytesReceived))
        {
            //解析ATS
            //TL   (长度字节)
            k = tmp1[0];
            if((mfcmdinfotmp.nBitsReceived!=0) || (mfcmdinfotmp.nBytesReceived != k))   //??pengxuebin, 2009-7-13 14:52:27 mfcmdinfotmp.nBytesReceived中没有CRC
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
                    //针对
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
 ** Descriotions:   Attrib  选择PICC
 ** parameters:     PUPI，PICC标识符。
                      CID，PICC逻辑地址，取值范围：0--14。
                      brTx，PCD--->PICC速率选择。
                      brRx，PICC--->PCD速率选择。
                      PARAM3，参数3。
 ** Returned value:  0: 成功
                    其它:失败 
 ** Created By:     彭学斌 
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
        dev_pcd_reset_mfinfo(&mfcmdinfotmp);        //复位结构
        mfcmdinfotmp.nBytesToSend = 9;              //数据长度
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
                status = MI_BITCOUNTERR;    //接收错误
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
                    
 ** Returned value:  0: 成功
                    其它:失败 
 ** Created By:     彭学斌 2009-8-22 23:48:56
 ** Remarks:       
*******************************************************************************/
s8 dev_pn512_exchange_piccA(u8 *wbuf, u16 wlen, u8 *rbuf, u16 *rlen, u32 etu, u8 wtx)
{
    s32 status;
    u32 tmp;
    MfCmdInfo mfcmdinfotmp;
    dev_pcd_wait_etu(54);      //pengxuebin,20090822_1
    *rlen = 0;
    dev_pcd_reset_mfinfo(&mfcmdinfotmp);                //复位结构
    mfcmdinfotmp.nBytesToSend = wlen;                   //数据长度
    mfcmdinfotmp.timeoutmode = PCD_TIMER_MODE_nETU1_1;  //以32etu为单位(32*(fc/128))
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
    //    u8 doReceive = 0;         //处理接收标志
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
    清除所有中断标志
    */   
    dev_pn512_write_reg(JREG_COMMIRQ, 0x7F);
    dev_pn512_write_reg(JREG_DIVIRQ,  0x1F);
    //取消命令
    dev_pn512_read_reg(JREG_COMMAND, &regval);
    regval = (regval & (~JMASK_COMMAND));
    dev_pn512_write_reg(JREG_COMMAND, regval);

    /*buf =(u8*)k_malloc(300);
    if(buf == NULL)
    {
        dev_debug_printf("申请内存失败\r\n");
        return MI_UNKNOWN_COMMAND;
    }*/
//    timeoutcnttmp = p_mfcmdinfo->timeoutcnt;
    //timeouttmp = p_mfcmdinfo->timeout;
    //-----------------------------------------------
    #if 0
    //根据指令处理数据
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
            //设置等待MASK
            commIrqEn   = JBIT_RXI | JBIT_IDLEI| JBIT_LOALERTI | JBIT_ERRI | JBIT_TIMERI | JBIT_TXI; //JBIT_RXI | JBIT_TIMERI;  //| JBIT_ERRI;
            waitForComm = JBIT_RXI | JBIT_IDLEI| JBIT_LOALERTI | JBIT_ERRI | JBIT_TIMERI | JBIT_TXI; //所有的中断信号处理
            break;
        default:
            status = MI_UNKNOWN_COMMAND;
            break; 
    }
    #endif
    //===============================================
   
    {
        //设置等待MASK
        commIrqEn   = JBIT_RXI | JBIT_IDLEI| JBIT_LOALERTI | JBIT_ERRI | JBIT_TIMERI | JBIT_TXI; //JBIT_RXI | JBIT_TIMERI;  //| JBIT_ERRI;
        waitForComm = JBIT_RXI | JBIT_IDLEI| JBIT_LOALERTI | JBIT_ERRI | JBIT_TIMERI | JBIT_TXI; //所有的中断信号处理
            
        //清FIFO
        dev_pn512_write_reg(JREG_FIFOLEVEL, JBIT_FLUSHBUFFER);
        regval = 64;                              //FIFO为64Bytes
        if(regval >= wlen)
        {
            regval = wlen;
            //关低报警
            commIrqEn &= (~JBIT_LOALERTI);
            waitForComm &= (~JBIT_LOALERTI);        //数据1次写FIFO就可以发送完
        }
        dev_pn512_write_fifo(wbuf, regval);

        wcnt = regval;
        dev_pn512_write_reg(JREG_COMMIRQ, JBIT_LOALERTI|JBIT_HIALERTI);//清除低报警标志


        //设置定时器
        dev_pn512_set_timeout(PCD_TIMER_MODE_nETU1_1, 200);
        commIrqEn |= JBIT_TIMERI;
        waitForComm |= JBIT_TIMERI;
        
        //使能所需要的中断
        dev_pn512_write_reg(JREG_COMMIEN, commIrqEn);
        //发送命令
        dev_pn512_read_reg(JREG_COMMAND, &regval);
        regval = (regval & (~JMASK_COMMAND))|JCMD_TRANSCEIVE;


        // 若错误发生，则在IRQ之前清除错误寄存器  
        // dev_pn512_write_reg(JREG_ERROR, 0);//?
        dev_pn512_write_reg(JREG_COMMAND, regval);

        //对于TRANSCEIVE命令，开始发送数据
        //if(JCMD_TRANSCEIVE == p_mfcmdinfo->cmd)
        {               
          #if 0  //1608对于106kbps没有多帧接收
            //这里可以考虑使用RxMulotiple 
            if(1 == g_rf_emd_flg)
            {
                dev_pn512_modify_reg(JREG_RXMODE, 1, JBIT_RXMULOTIPLE);
            }
          #endif  
//drv_psam_testpin_set(1, 0);
            /*TRx is always an endless loop, Initiator and Target must set STARTSEND.*/
            dev_pn512_modify_reg(JREG_BITFRAMING, 1, JBIT_STARTSEND);   //开始发送
            dev_pn512_write_reg(JREG_COMMIRQ, 0x7F);
       }
      #if 0  
        else
        {
            //启动定时器
            dev_pn512_modify_reg(JREG_CONTROL, 1, JBIT_TSTARTNOW); 
        }
      #endif  
        timerid = dev_get_timeID();
        timerover = 100;
        //===================下面开始循环======================
        while(1)
        {
            //if(0 != dev_rf_int_get())
            {              
                //读状态寄存器
                dev_pn512_read_reg(JREG_COMMIRQ, &commirq_reg); 
                commirq_reg &= waitForComm;

                if(0 != commirq_reg)
                {
                    //----------------------------------------
                    //超时处理
                    if(commirq_reg&JBIT_TIMERI)
                    {
                      #if 0  
                        //请超时报警标志
                        dev_pn512_write_reg(JREG_COMMIRQ, JBIT_TIMERI);
                        if(p_mfcmdinfo->timeoutcnt==0)
                        {
                            //增加判断是否收到数据
                            dev_pn512_read_reg(JREG_FIFOLEVEL, &regval);
                            if(((regval&0x7)==0) && (0 == buflen))
                            {
                                if(0 != statusbak)
                                {           
                                    status = statusbak;
                                }
                                else
                                {
                                    status = MI_NOTAGERR;//没有目标//MI_ACCESSTIMEOUT;          //超时
                                }
                                if(emdretry != 0)
                                {
                                    status = emdretry;
                                }
                                break;
                            }
                            else
                            {
                                //说明正在接收  
                                waitForComm &= ~JBIT_TIMERI;    //可以考虑继续用PN512的定时器定时
                                //停止定时器timeflag
                                dev_pn512_modify_reg(JREG_CONTROL, 1, JBIT_TSTOPNOW);  
                                //设置超时
                                timerid = dev_get_timeID();
                                timerover = 200;      //等待接收数据

                            }  
                        }//<----if(p_mfcmdinfo->timeoutcnt==0)
                        else
                        {
                            p_mfcmdinfo->timeoutcnt--;
                            //重新设置超时
                            dev_pn512_set_timeout((p_mfcmdinfo->timeoutmode&0xfe), 0x8000);
                            dev_pn512_modify_reg(JREG_CONTROL, 1, JBIT_TSTARTNOW); 

                        }
                      #endif
                        status = MI_NOTAGERR;//没有目标//MI_ACCESSTIMEOUT;          //超时
                        break;        
                    }//<----if(commirq_reg&JBIT_TIMERI)
                    //=========================================
                    //读错误寄存器
                    if((commirq_reg&JBIT_ERRI))
                    {
                      #if 0  
                        //读错误寄存器
                        if(1 == g_rf_emd_flg)
                        {
                        }
                        else    
                        { 
                            //读取所有接收的字节数和位数,用于错误检测
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


                            //清除错误标志
                            dev_pn512_write_reg(JREG_COMMIRQ, JBIT_ERRI);
                            // 若错误发生，则在IRQ之前清除错误寄存器  
                            dev_pn512_write_reg(JREG_ERROR, 0);
                            //防碰撞错误
                            if(err_reg&JBIT_COLLERR)
                            {
                                status = MI_COLLERR;
                            }

                            //奇偶校验错误
                            else if(err_reg&JBIT_PARITYERR)
                            {
                                status = MI_PARITYERR;
                            }

                            //协议错误
                            else if(err_reg&JBIT_PROTERR)
                            {
                                status = MI_PROTOCOL_ERROR;
                            }

                            //缓冲区溢出错误
                            else if(err_reg&JBIT_BUFFEROVFL)
                            {
                                status = MI_RECBUF_OVERFLOW;
                            }

                            //CRC错误
                            else if(err_reg&JBIT_CRCERR)
                            {
                                //CRC错误且只接收到1个字节认为是Mifare (N)ACK 
                                if(((0x04 == nbitreceived)||(0x00==nbitreceived)))//stacy
                                {                             
                                    buflen = 1;
                                    status = MI_ACK_SUPPOSED;   //(N)  ACK
                                }
                                else
                                {
                                    status = MI_CRCERR;             //CRC错误
                                    //break;
                                } 
                            }

                            //温度过高错误
                            else if(err_reg&JBIT_TEMPERR)
                            {
                                status = MI_TEMP_ERROR;
                            }

                            //写错误
                            else if(err_reg&JBIT_WRERR)
                            {
                                status = MI_WRITEERR;
                            }

                            //读错误
                            else if(err_reg&JBIT_RDERR)
                            {
                                status = MI_READERR;
                            }
                            //其它错误
                            else
                            {
                                //if(commirq_reg&JBIT_ERRI)
                                status = MI_OTHERERR;
                            }
                            ///////////////////////////////////////
                            //总的判断错误的处理
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
                    //发送完成
                    if(commirq_reg&JBIT_TXI)
                    {
//drv_psam_testpin_xor(2);
                      #if 0  
                        waitForComm &= ~JBIT_TXI;    //关发送完成处理   ?peng
                        //开高报警
                        commIrqEn |= JBIT_HIALERTI;
                        waitForComm |= JBIT_HIALERTI;
                        dev_pn512_write_reg(JREG_COMMIEN, commIrqEn);       //开高报警
                        dev_pn512_write_reg(JREG_COMMIRQ, JBIT_HIALERTI|JBIT_TXI);   //清除发送报警标志           
                        timerid = dev_get_timeID();
                        timerover = (p_mfcmdinfo->timeout+(((u32)p_mfcmdinfo->timeoutcnt)<<15))/2 + 300;//30000;          //30s
                      #endif
                        status = MI_OK;
                        break;
                    }
                  #if 0  
                    //--------------------------------------------------
                    //接收完成
                    if(commirq_reg&JBIT_RXI)
                    {
                        dev_pn512_read_reg(JREG_ERROR, &err_reg);
                        
                        //读接收bits数
                        dev_pn512_read_reg(JREG_CONTROL, &regval);
                        nbitreceived= (regval & 0x07);
                        //接收数据
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
                            //重新启动接收
                            dev_pn512_write_reg(JREG_COMMAND, JCMD_RECEIVE);
                            //FLUSH FIFO
                            dev_pn512_write_reg(JREG_FIFOLEVEL, JBIT_FLUSHBUFFER);
                            //清除中断标志
                            dev_pn512_write_reg(JREG_COMMIRQ, 0x7F);
                            emdretry = MI_CRCERR;
                            nbitreceived = 0;
                            buflen = 0;
                            //设置PCD定时 1280/fc=10etu
                          #if 0  
                            if(dev_rf_get_r_s_flg())
                            {
                                p_mfcmdinfo->timeoutcnt=0;
                                dev_pn512_set_timeout(PCD_TIMER_MODE_ETU_0, 200);//2000);  //保证能够收到1个完整字节
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
                                //重新启动接收
                                dev_pn512_write_reg(JREG_COMMAND, JCMD_RECEIVE);
                                //FLUSH FIFO
                                dev_pn512_write_reg(JREG_FIFOLEVEL, JBIT_FLUSHBUFFER);
                                //清除中断标志
                                dev_pn512_write_reg(JREG_COMMIRQ, 0x7F);
                                emdretry = MI_CRCERR;
                                nbitreceived = 0;
                                buflen = 0;
                                //设置PCD定时 1280/fc=10etu
                                if(dev_rf_get_r_s_flg())
                                {
                                    p_mfcmdinfo->timeoutcnt=0;
                                    dev_pn512_set_timeout(PCD_TIMER_MODE_ETU_0, 200);//2000);  //保证能够收到1个完整字节
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
                                // 若错误发生，则在IRQ之前清除错误寄存器  
                                dev_pn512_write_reg(JREG_ERROR, 0);
                                
                                //防碰撞错误
                                if(err_reg&JBIT_COLLERR)
                                {
                                    status = MI_COLLERR;
                                }

                                //奇偶校验错误
                                else if(err_reg&JBIT_PARITYERR)
                                {
                                    status = MI_PARITYERR;
                                }

                                //协议错误
                                else if(err_reg&JBIT_PROTERR)
                                {
                                    status = MI_PROTOCOL_ERROR;
                                }

                                //缓冲区溢出错误
                                else if(err_reg&JBIT_BUFFEROVFL)
                                {
                                    status = MI_RECBUF_OVERFLOW;
                                }

                                //CRC错误
                                else if(err_reg&JBIT_CRCERR)
                                {
                                    //CRC错误且只接收到1个字节认为是Mifare (N)ACK 
                                    if((0x01 == buflen) && ((0x04 == nbitreceived)||(0x00==nbitreceived)))
                                    {
                                        buflen = 1;
                                        status = MI_ACK_SUPPOSED;   //(N)  ACK
                                    }
                                    else
                                    {
                                        status = MI_CRCERR;             //CRC错误
                                        //break;
                                    }
                                }

                                //温度过高错误
                                else if(err_reg&JBIT_TEMPERR)
                                {
                                    status = MI_TEMP_ERROR;
                                }

                                //写错误
                                else if(err_reg&JBIT_WRERR)
                                {
                                    status = MI_WRITEERR;
                                }

                                //读错误
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
                    //指令执行完成
                    if(commirq_reg&JBIT_IDLEI)
                    {
                        
                        //uart_printf("JBIT_IDLEI:\r\n"); 
                        status = MI_OK;
                        break;
                    }
                    //---------------------------------------------
                    //FIFO低报警,发送
                    if(commirq_reg&JBIT_LOALERTI)    
                    {
                        dev_pn512_read_reg(JREG_FIFOLEVEL, &regval);     //读FIFO中数据长度
                        regval &= 0x7f;
                        if(regval > 64)
                        {
                            //错误,暂不处理
                        }              
                        else
                        {
                            regval = (64-regval);
                            if(regval >= (wlen-wcnt))
                            {                          
                                regval = wlen - wcnt;
                                //关低报警
                                commIrqEn &= (~JBIT_LOALERTI);
                                waitForComm &= (~JBIT_LOALERTI);        //数据1次写FIFO就可以发送完
                                dev_pn512_write_reg(JREG_COMMIEN, commIrqEn);
                            } 
                            dev_pn512_write_fifo(&wbuf[wcnt], regval);
                            wcnt += regval;
                        }

                        //清低报警标志
                        dev_pn512_write_reg(JREG_COMMIRQ, JBIT_LOALERTI);
                    }
                  #if 0  
                    //---------------------------------------------
                    //FIFO高报警，接收
                    if(commirq_reg&JBIT_HIALERTI)    
                    {
                        //接收数据
                        dev_pn512_read_reg(JREG_FIFOLEVEL, &regval);
                        regval &= 0x7f;
                        dev_pn512_read_fifo(&buf[buflen], regval);
                        buflen += regval; 
                        //清高报警标志
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
            //判断超时,防止PN512死锁
            if(dev_querry_time(timerid, timerover))
            {           
                //时间到   
                status = MI_ACCESSTIMEOUT;          //超时
                break;
            }
        }//<----while(1)
        
    }  
    //关闭定时器
    dev_pn512_modify_reg(JREG_CONTROL, 1, JBIT_TSTOPNOW);

    dev_pn512_read_reg(JREG_COMMAND, &regval);
    dev_pn512_write_reg(JREG_COMMAND, regval&0xf0);
    //清除所有中断及IRQ标志
    dev_pn512_write_reg(JREG_COMMIRQ, 0x7f); //waitForComm);
    dev_pn512_write_reg(JREG_DIVIRQ, 0x1f); //waitForDiv);
    //清除FIFO
    dev_pn512_write_reg(JREG_FIFOLEVEL, JBIT_FLUSHBUFFER);
    //禁止中断
    dev_pn512_write_reg(JREG_COMMIEN, 0x80);
    dev_pn512_write_reg(JREG_DIVIEN,  0x80); 
    //清BITFAMEING寄存器
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
        dev_pn512_write_reg(JREG_COLL, JBIT_VALUESAFTERCOLL);  //该位只有在106kb/s的位防冲突时置0，其他要置1
    
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
            dev_pn512_write_reg(JREG_COLL, JBIT_VALUESAFTERCOLL);  //该位只有在106kb/s的位防冲突时置0，其他要置1
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
        
        nbits = (bitcount%BITS_PER_BYTE);           //需要发送的非完整位
        nbytes = ((bitcount+7) / BITS_PER_BYTE);    //需要发送的数据
        
        databfr[0] = PCDTypeASel[grade&0x03];       //不同等级的SEL代码
        databfr[1] = 0x20 + (((bitcount/8)<<4)&0x70) + nbits;//NVB
        memcpy(&databfr[2], snr, nbytes);           //需要发送的数据
        
        //set TxLastBits and RxAlign to number of bits sent
        dev_pn512_write_reg(JREG_BITFRAMING, ((nbits << UPPER_NIBBLE_SHIFT) | nbits));
        
        //prepare data for common transceive
        dev_pcd_reset_mfinfo(&mfcmdinfotmp);        //复位结构
        mfcmdinfotmp.nBytesToSend = (nbytes+2);
        mfcmdinfotmp.timeoutmode = PCD_TIMER_MODE_ETU_1;//PCD_TIMER_MODE_100US_1;
        mfcmdinfotmp.timeout = 20;//19;//1000;//暂时定100etu  //5000;                  //待定，pengxuebin，????
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
        {   //如果执行成功或碰撞错误, 则执行

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


            rbits = (mfcmdinfotmp.nBitsReceived + (mfcmdinfotmp.nBytesReceived << 3) - nbits);  //计算接收的位数
            if((rbits+bitcount) > COMPLETE_UID_BITS)   //40
            {
                status = MI_BITCOUNTERR;                //位统计错误
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
            else    //没有冲突
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
                      
                        //说明防冲突成功
                        bitcount = COMPLETE_UID_BITS;// += rbits;
                        complete = 1;                //执行select
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

    
    if((0 == complete))             //循环次数到了32次还没有成功
    {
        status = MI_ANTICOLL_ERR;
        
    }
    return status;
}
#endif
