#include "devglobal.h"

#include "devapi/dev_rf/dev_rf_flow.h"
#include "devapi/dev_rf/dev_rf_io.h"
#include "devapi/dev_rf/rc663/mfrc663reg.h"
#include "devapi/dev_rf/rc663/dev_rc663.h"
extern u8  const PCDTypeASel[3];
static u32 g_emden=0; //0:不处理EMD, 1:处理TYPEA EMD，2:处理TYPEB EMD
static u8 g_rc663_special_B_f = 0;  

#define RC663_NRSTPD_OFF     dev_rf_rst_ctl(1)  
#define RC663_NRSTPD_RST     dev_rf_rst_ctl(0) 

extern s32 drv_spi_rfcomm(u8 *snd, u8 *rsv, s32 size); 
#if 0
//static 
u8 g_pn512_ver = 0;
u8 g_rf_special_B_flag = 0;  
//static u8 g_rf_loopback_mode = 0;      //0:表示正常loopback    1:对于
//static u8 g_rf_test_flag = 0;
u8 g_rf_emd_flg = 0;
//static u8 g_rf_emd_recflg;
u8 g_rf_morecard_cnt = 0;
#endif
#if 0
u8 g_debug_flg=0;
void rf_debug_printf(s8 *fmt,...)
{
    s32 length = 0;
    va_list ap;
    s8 string[256];
    s8 *pt;
    
    if(g_debug_flg)
    {
        va_start(ap,fmt);
        vsprintf((char *)string,(const char *)fmt,ap);
        pt = &string[0];
        while(*pt!='\0')
        {
            length++;
            pt++;
        }
        dev_com_write(DEBUG_PORT, (u8*)&string[0], length);  //写串口成功
        //dev_com_write(PC_PORT, (u8*)&string[0], length);  //写串口成功
        
        va_end(ap);
    }
}
void rf_PrintFormat(u8 *wbuf, s32 wlen)
{   
    s32 i;
    if(g_debug_flg)
    {
        for(i=0; i<wlen; i++)
        {
            if((0 == (i&0x0f)))
            {
                uart_printf("\r\n");
            }
            uart_printf("%02x ", wbuf[i]);
        }
        uart_printf("\r\n");
    }
}
void dev_pcd_setdebug(u8 mode)
{
    g_debug_flg = mode;
}
#else
//#define rf_debug_printf
//#define rf_PrintFormat
//void dev_pcd_setdebug(u8 mode)
//{
//}
#endif

u32 dev_get_timeID(void)
{
    return dev_user_gettimeID();
    //return 0;
}
s32 dev_querry_time(u32 timeid, s32 ms)
{
    return dev_user_querrytimer(timeid, ms);
    //return 0;
}


/*******************************************************************************
 ** Descriotions:   读rc663版本
 ** parameters:                                                              
 ** Returned value: 
                     0:成功  
                    -1:失败
                                                                                              
 ** Created By:     彭学斌 
 ** Remarks:                                                                                  
*******************************************************************************/
s32 dev_rc663_read_version(u8 *ver)
{    
    static u8 flg=1;

    if(flg)
    {
        flg = 0;
        RC663_NRSTPD_OFF;
        dev_user_delay_ms(1); //(1);
        RC663_NRSTPD_RST;
        dev_user_delay_ms(1); //(1);
    }
    dev_rc663_read_reg(J663REG_Version, ver);     
//dev_debug_printf("dev_rc663_read_version:%02X\r\n", ver[0]); 
    if(*ver == 0x18)
    {
        return 0;
    }
    else
    {
        return -1;
    }
    
}
/*******************************************************************************
 ** Descriotions:   RC663复位
 ** parameters:                                                              
 ** Returned value:   
                                                                                              
 ** Created By:     彭学斌 
 ** Remarks:                                                                                  
*******************************************************************************/
void dev_rc663_reset(s32 _ms)
{
   #if 0 
        /*soft reset*/
    // Flush any running command and FIFO
    dev_rc663_write_reg(J663REG_Command,J663CMD_IDLE);
    dev_rc663_write_reg(J663REG_IRQ0En,0x00);
    dev_rc663_write_reg(J663REG_IRQ1En,0x00);    
    dev_rc663_write_reg(J663REG_FIFOControl, 0xB0); 
        
    // Clear Mix2Adc bit 
    dev_rc663_write_reg(J663REG_Rcv, 0x12); 
    
    // Stop Timer4 
    dev_rc663_write_reg(J663REG_T4Control, 0x5F);
    dev_rc663_write_reg(J663REG_DrvMod, 0x86);
 //   dev_rc663_write_reg(J663REG_Command, J663CMD_SoftReset);
  #else
    dev_rc663_write_reg(J663REG_DrvMod, 0x86);
  #endif
  
//    rf_delayms(_ms);//dev_delay_ms(_ms);
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
s32 dev_rc663_ctl_crc(u8 mask, u8 mod)
{
    //TXCRC 
    if(mask&DEV_RF_TXCRC_MASK)
    {
        if(mod&DEV_RF_TXCRC_MASK)
        {
            dev_rc663_modify_reg(J663REG_TxCrcPreset, 1, J663BIT_CRCEN);       //Enable TxCRC
        }
        else
        {
            dev_rc663_modify_reg(J663REG_TxCrcPreset, 0, J663BIT_CRCEN);       //Enable TxCRC    
        }
    }
    //RXCRC
    if(mask&DEV_RF_RXCRC_MASK)
    {
        if(mod&DEV_RF_RXCRC_MASK)
        {
            dev_rc663_modify_reg(J663REG_RxCrcPreset, 1, J663BIT_CRCEN);
        }
        else
        {
            dev_rc663_modify_reg(J663REG_RxCrcPreset, 0, J663BIT_CRCEN);           
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
//addr:0x28~0x39#if 1
#if 0                         
//生产参数
                                  //28
u8 const rc663_typea_param1[18] = {0x89,0x15,0x09,0x0A,0x18,0x18,0x0F,0x27, 
                                  //30
                                   0x00,0xC0,0x12,0xCF,0x00,0x04,0x90,0xAF,
                                  //38
                                   0x12,0x06//0x0A
                                  };
#else 
//EMV参数
                                  //28
u8 const rc663_typea_param1[18] = {0x89,0x15,0x09,0x0A,0x18,0x18,0x0F,0x27, 
                                  //30
                                   0x00,0xC0,0x12,0xCF,0x00,0x04,0x90,0x36,
                                  //38
                                   0x12,0x07
                                  };

#endif
//addr:0x48~0x5f                   //48
u8 const rc663_typea_param2[24] = { 0x20,0x00,0x04,0x50,0x40,0x00,0x00,0x00,
                                   //50
                                    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x50,
                                   //58 
                                    0x02,0x00,0x00,0x01,0x00,0x08,0x80,0xB2,
                                  };
//TypeB
#if 0
//生产参数
//addr:0x28~0x39                 
u8 const rc663_typeb_param1[18] = { 0x8F,0x4F,0x01,0x0A,0x7B,0x7B,0x08,0x00,
                                    0x00,0x01,0x00,0x05,0x00,0x34,0x90,0x66,
                                    0x12,0x0A,
                                  };
#else
//EMV参数
//addr:0x28~0x39                 
u8 const rc663_typeb_param1[18] = { 0x8F,0x4C,0x01,0x0A,0x7B,0x7B,0x08,0x00,
                                    0x00,0x01,0x00,0x05,0x00,0x34,0x90,0x36,
                                    0x12,0x0a,
                                  };

#endif
//addr:0x48~0x5f
u8 const rc663_typeb_param2[24] = { 0x09,0x00,0x04,0x08,0x04,0x00,0x03,0x00,
                                    0x01,0x00,0x00,0xAB,0x00,0x00,0x08,0x00,
                                    0x04,0x00,0x00,0x00,0x02,0x0D,0x80,0xF0,
                                  };
s8 dev_rc663_select_type(u8 type)
{
//    u8 regval;
    u8 tmp[32];
    s32 i;
//    unsigned char protocol_arg[10]={0};
#ifdef PCD_DEBUG   
PCD_PRINTF("dev_pcd_select_type:%c\r\n\r\n", type);  
#endif

//    dev_pcd_initparam();
    if(PICC_TYPEC == type)
    {
    }
    else
    {
        dev_rc663_write_reg(J663REG_FIFOControl, 0x90);     
        //dev_rc663_write_reg(J663REG_WaterLevel,  0xFE);
        dev_rc663_write_reg(J663REG_WaterLevel, DEV_RC663_WATERLEVEL);
        dev_rc663_write_reg(J663REG_RxBitCtrl,   0x80);
        dev_rc663_write_reg(J663REG_RxAna, 0x0a);//g_rf_param.CWGsP); //设置接受增益
        //dev_rc663_modify_reg(J663REG_Status, 0, 0x20);
        dev_rc663_write_reg(J663REG_Status, 0x00);
        if(PICC_TYPEA == type)
        {
            for(i=0; i<18; i++)
            {
                tmp[i] = rc663_typea_param1[i];
            }
            //设定剩余载波百分比 TxAmp[0x29][4:0](0~31)
            //tmp[1] = (tmp[1]&0xe0)|(g_rf_param.rfmodgsp&0x1f);
            //RxThreshold寄存器 [0x37]   [0x37-0x28]=0x0f=15
            tmp[15] = g_rf_param.rxthreshold_a;
            //设置增益（rcv_gain）[1~0]和高通转角频率（rcv_hpcf）[3~2]
            //RXANA[0x39]    [0x39-0X28] = 0x11 = 17
            tmp[17] = (tmp[1]&0xf0)|(g_rf_param.rfcfgregA&0x0f);
//printf("typeA:%02X, %02X,%02X\r\n", tmp[1], tmp[15], tmp[17]);            
            dev_rc663_write_serial(J663REG_DrvMod, tmp, 18);
            
            for(i=0; i<24; i++)
            {
                tmp[i] = rc663_typea_param2[i];
            }
            dev_rc663_write_serial(J663REG_TxBitMod, tmp, 24);
            
            //dev_rc663_write_reg(J663REG_RxThreshold  , 0xf2); //pengxuebin,20140811
            //dev_rc663_write_reg(J663REG_RxAna  , 0x06);
            //dev_rc663_write_reg(J663REG_DrvCon  , 0x01);//0x5D); //pengxuebin,20140811
            //dev_rc663_write_reg(J663REG_Txl  , 0x0A);//0x2D);
              
            
//uart_printf("dev_rc663_select_type:TypeA\r\n");            
        }
        else if(PICC_TYPEB == type)
        {
            for(i=0; i<18; i++)
            {
                tmp[i] = rc663_typeb_param1[i];
            }
            //设定剩余载波百分比 TxAmp[0x29][4:0](0~31)
            tmp[1] = (tmp[1]&0xe0)|(g_rf_param.rfmodgsp&0x1f);
            //RxThreshold寄存器 [0x37]   [0x37-0x28]=0x0f=15
            tmp[15] = g_rf_param.rxthreshold_b;
            //设置增益（rcv_gain）[1~0]和高通转角频率（rcv_hpcf）[3~2]
            //RXANA[0x39]    [0x39-0X28] = 0x11 = 17
            tmp[17] = (tmp[1]&0xf0)|(g_rf_param.rfcfgregB&0x0f);
            
//printf("typeB:%02X, %02X,%02X\r\n", tmp[1], tmp[15], tmp[17]);        
            dev_rc663_write_serial(J663REG_DrvMod, tmp, 18);
            for(i=0; i<24; i++)
            {
                tmp[i] = rc663_typeb_param2[i];
            }
            dev_rc663_write_serial(J663REG_TxBitMod, tmp, 24);
        }
        
    
    }
    /* Activate receiver for communication
       The RcvOff bit and the PowerDown bit are cleared, the command is not changed. */
    //报警设为128字节
//    dev_rc663_modify_reg(J663REG_DrvMod, 1, J663BIT_TXEN);    //注意RC663和RC631命令定义不一样   
    return MI_OK;
}
void test_spi_trans(void)
{
    u8 tmp[32];
    s32 i;

    for(i=0; i<18; i++)
    {
        tmp[i] = rc663_typea_param1[i];
    }
    dev_rc663_write_serial(J663REG_DrvMod, tmp, 18);
}
/*******************************************************************************
 ** Descriotions:   设置定时器，13.56MHz为时钟源，T2T0为定时器组合
 ** parameters:     clk为13.56MHz时钟周期为单位,u32最大可以定时316,738,000us
 ** Returned value: 无                                                                         
 ** Created By:     彭学斌    2014/1/14 16:29:37      
 ** Remarks:                                                                                  
*******************************************************************************/
void dev_rc663_set_time(u8 mode, u32 clk)
{
    u16 tprescal;
    u16 tcount;
    
    if(clk<=65536)
    {
        tprescal = clk-1;
        tcount = 0;
    }
    else
    {
        tprescal = clk>>16;
        tcount = (clk/(((u32)tprescal)+1))-1;
    }
    //关闭定时器
    dev_rc663_write_reg(J663REG_TControl, 0x05);
    

    //设置定时器
    dev_rc663_write_reg(J663REG_T0ReloadHi, tprescal>>8); 
    dev_rc663_write_reg(J663REG_T0ReloadLo, tprescal);

    dev_rc663_write_reg(J663REG_T2ReloadHi, tcount>>8);
    dev_rc663_write_reg(J663REG_T2ReloadLo, tcount); 
    
    //采用定时器0和2级联，定时器0做预分频，定时器2做定时
    dev_rc663_write_reg(J663REG_T2Control, 0x02);
    if(mode)
    {
        //T0自动重载,在发送结束后自动重启,收到第一个字符后自动停止
        dev_rc663_write_reg(J663REG_T0Control, 0x08|0x90); 
        dev_rc663_write_reg(J663REG_TControl, 0x44);    //启动定时器T2
    }
    else
    {
        dev_rc663_write_reg(J663REG_T0Control, 0x08); //T0自动重载
        dev_rc663_write_reg(J663REG_TControl, 0x55);  //启动定时器T0T2
    }
}
/*******************************************************************************
 ** Descriotions:   设置超时时间
 ** parameters:     mode:超时时间方式
                         0: 100us计时
                         1: 以etu为单位计时
                         其它保留
                    timer:定时值                                                         
 ** Returned value: 无                                                                         
 ** Created By:     彭学斌          
 ** Remarks:                                                                                  
*******************************************************************************/
void dev_rc663_set_timeout(u8 mode, u32 timer)
{
    u32 i;
//    u8 regval;
    
    if(0 == timer)return;
    if((PCD_TIMER_MODE_ETU_0 == mode) || (PCD_TIMER_MODE_ETU_1 == mode))
    {
        i = timer*128;
    }
    else if((PCD_TIMER_MODE_nETU_0 == mode) || (PCD_TIMER_MODE_nETU_1 == mode))
    {       
        i = (timer*128);
    }
    else //if(PCD_TIMER_MODE_US == mode)
    {                              //100us :  21.1875
       i = timer*1356;
    }
    dev_rc663_set_time((mode&0x01) , i);
      
}

void dev_rc663_wait_etu(u32 etu)
{
    u32 i;
    u32 timerID;
    u32 timeover;
    u8 irq0reg,irq1reg;
//    u32 yflg=1;

    i = etu*128;          //etu不能大于0x2000000=33554432;最大定时316,738,001us

//dev_misc_led_yellow(1);
    //dev_delay_ms(5);
    //设置控制中断
    dev_rc663_write_reg(J663REG_IRQ0En, J663BIT_IRQINV);
    dev_rc663_write_reg(J663REG_IRQ1En, 0xc4);
    
    //清除中断
    dev_rc663_write_reg(J663REG_IRQ0, 0x7F);
    dev_rc663_write_reg(J663REG_IRQ1, 0x7F);

    timeover = (i/13560)+50; //(etu/100)+2;
    //启动定时器T0T2
    dev_rc663_set_time(0, i);
    //dev_rc663_write_reg(J663REG_TControl, 0x55);  
    timerID = dev_get_timeID();
//dev_misc_led_yellow((++yflg)&0x01);
    while(1)
    {
        //if(0 == dev_rf_int_get())
        {
//dev_misc_led_yellow((++yflg)&0x01);
            dev_rc663_read_reg(J663REG_IRQ0, &irq0reg);
            dev_rc663_read_reg(J663REG_IRQ1, &irq1reg);
            dev_rc663_write_reg(J663REG_IRQ0, irq0reg);
            dev_rc663_write_reg(J663REG_IRQ1, irq1reg);
            if(irq1reg&J663BIT_Timer2Irq1)
            {
//uart_printf("tt:%02X %02X\r\n", irq0reg, irq1reg);                
                break;
            }
        }
        if(dev_querry_time(timerID, timeover))
        {
//uart_printf("timerout!\r\n"); 
            break;
        }
    }
    
    //关闭定时器
    dev_rc663_write_reg(J663REG_TControl, 0x05);

    //关闭定时中断,清除中断标志
    dev_rc663_write_reg(J663REG_IRQ0En, J663BIT_IRQINV);
    dev_rc663_write_reg(J663REG_IRQ1En, 0xc0);
    //清除中断
    dev_rc663_write_reg(J663REG_IRQ0, 0x7F);
    dev_rc663_write_reg(J663REG_IRQ1, 0x7F);
    
}
/*******************************************************************************
 ** Descriotions:  卡片交互 
 ** parameters:                                                           
 ** Returned value: 无                                                                         
 ** Created By:          
 ** Remarks:                                                                                  
*******************************************************************************/
s8 dev_rc663_execute_cmd(MfCmdInfo *p_mfcmdinfo, u8 *wbuf, u8 *rbuf)
{ 
    u32 i;
    u32 timerid;
    u32 timerover;
    s32 buflen=0;
    //u8 buf[512];
    u8 *buf = NULL;
    u16 wcnt;
    u8  irq0en;
    u8  irq1en;
    u8  irq0reg;
    u8  irq1reg;
    u8  errreg;
//    u8  errcnt=0;
    u8  regval;

    s8  status = MI_OK;
//    s8  statusbak= MI_OK;
//    s8  statusbuflen=0;
    u8 nbitreceived=0;
//    u8 errflg;
    u32 endflg;
    u32 remainnum;
//    u8 tmp[512];
//    u16 tmplen=0;
    
//g_debug_flg=1;
    buf = (u8 *)k_malloc(512);
    if(NULL == buf)
    {
        return MI_OTHERERR;
    }
    //==============================================
    //errcnt = 0;
    //关闭所有中断源
    irq0en = 0x00;
    irq1en = 0x00;
    dev_rc663_write_reg(J663REG_IRQ0En, irq0en|0x80);
    dev_rc663_write_reg(J663REG_IRQ1En, irq1en|0xc0);

    //取消命令
    dev_rc663_read_reg(J663REG_Command, &regval);
    dev_rc663_write_reg(J663REG_Command, (regval&(~J663CMD_MASK)));
    //根据指令处理数据
    switch(p_mfcmdinfo->cmd)
    {
        case J663CMD_IDLE:            //0x00
        case J663CMD_LPCD:            //0x01
        case J663CMD_LoadKey:         //0x02
            irq0en = J663BIT_IdleIrq0;//|J663BIT_TxIrq0|J663BIT_ErrIrq0;
            irq1en = J663BIT_Timer2Irq1; 
            break;
        case J663CMD_Receive:         //0x05
        case J663CMD_Transmit:        //0x06
            status = MI_UNKNOWN_COMMAND;
            break;
        case J663CMD_MFAuthent:       //0x03
            irq0en = J663BIT_IdleIrq0|J663BIT_TxIrq0;//|J663BIT_ErrIrq0;
            irq1en = J663BIT_Timer2Irq1;            
            break;
        case J663CMD_Transceive:      //0x07    //收发
            //irq0en = J663BIT_IdleIrq0|J663BIT_TxIrq0|J663BIT_ErrIrq0;//J663BIT_RxIrq0|
            irq0en = J663BIT_TxIrq0|J663BIT_RxIrq0;//J663BIT_ErrIrq0;
            irq1en = J663BIT_Timer2Irq1|J663BIT_Timer1Irq1;
            break;                          
        case J663CMD_WriteE2:         //0x08
        case J663CMD_WriteE2Page:     //0x09
        case J663CMD_ReadE2:          //0x0A
        case J663CMD_LoadReg:         //0x0C
        case J663CMD_LoadProtocol:    //0x0D
        case J663CMD_LoadKeyE2:       //0x0E
        case J663CMD_StoreKeyE2:      //0x0F
        case J663CMD_ReadRNR:         //0x1C
        default:
            status = MI_UNKNOWN_COMMAND;
            break;
    }
    //=========================================
    if(MI_OK == status)
    {
//rf_debug_printf("rc663 cmd: %02X, %d\r\n", p_mfcmdinfo->cmd, p_mfcmdinfo->nBytesToSend);
//rf_PrintFormat(wbuf, p_mfcmdinfo->nBytesToSend);

        //清FIFO, FiFo使用255Bytes模式
        dev_rc663_write_reg(J663REG_FIFOControl, 0x90);
        i = 255;
        if(i >= p_mfcmdinfo->nBytesToSend)
        {
            i = p_mfcmdinfo->nBytesToSend;
            
        }
        else
        {   
            //开低报警  
            irq0en |= J663BIT_LoAlertIrq0;
        }
        //写FIFO
        dev_rc663_write_fifo(wbuf, i);
        wcnt = i;
        //清除中断标志
        dev_rc663_write_reg(J663REG_IRQ0, 0x7F);
        dev_rc663_write_reg(J663REG_IRQ1, 0x7F);
        //开中断
        dev_rc663_write_reg(J663REG_IRQ0En, irq0en|0x80);
        dev_rc663_write_reg(J663REG_IRQ1En, irq1en|0xc0);
        
        p_mfcmdinfo->nBytesReceived = 0;
        p_mfcmdinfo->nBitsReceived = 0;
        p_mfcmdinfo->ErrorReg = 0;

        //设置定时器
        i = p_mfcmdinfo->timeoutcnt;
        i <<= 15;
        i += p_mfcmdinfo->timeout;
        dev_rc663_set_timeout(p_mfcmdinfo->timeoutmode, i);

        //执行命令
        dev_rc663_write_reg(J663REG_Command, p_mfcmdinfo->cmd); 
        /*if(J663CMD_MFAuthent == p_mfcmdinfo->cmd)
        {
            rf_delayms(2);//dev_delay_ms(2);
        }*/
        
        timerid = dev_get_timeID();
        timerover = 100;
        endflg = 0;
        remainnum = 0;
        while(1)
        {
            //if(0 == dev_rf_int_get())  
            {
                while(1)
                {
                    //读中断1寄存器
                    dev_rc663_read_reg(J663REG_IRQ1, &irq1reg);
//rf_debug_printf("i1=%02X", irq1reg);                    
                   #if 0
                    if(0 == (irq1reg&J663BIT_GlobalIrq1))
                    {
                        //无效中断
                        break;
                    }
                   #endif                    
                   irq1reg &= irq1en;
                    //清中断
                    dev_rc663_write_reg(J663REG_IRQ1, J663BIT_GlobalIrq1);
                   
                    //clearing LPCDIRQ interrupt
                    if(irq1reg & J663BIT_LPCD_Irq1)
                    {
                        dev_rc663_write_reg(J663REG_IRQ1, J663BIT_LPCD_Irq1);
                    }

                    /*TimerOver interruptions*/
                    if(irq1reg&J663BIT_Timer2Irq1)
                    {
                        if(MI_OK == status)
                        {
                            status = MI_NOTAGERR;
                        }
                        endflg = 1;
                    }
                    /*read interrupt status */
                    //读中断1寄存器
                    dev_rc663_read_reg(J663REG_IRQ0, &irq0reg);
//rf_debug_printf("i0=%02X", irq0reg);     
                    irq0reg &= irq0en;
                    //Hialerirq
                    if(irq0reg&J663BIT_HiAlerIrq0)
                    {
                        //接收数据，暂时只考虑255FIFO
                        dev_rc663_read_reg(J663REG_FIFOLength, &regval);
                        if((buflen+regval)>512)
                        {
                            status = MI_OVFLERR;
                            endflg = 1;
                        }
                        else
                        {
                            dev_rc663_read_fifo(&buf[buflen], regval);
                            buflen += regval;
                        }
                        //clearing interrupt
                        dev_rc663_write_reg(J663REG_IRQ0, J663BIT_HiAlerIrq0);
                    }
                    //LoAlerirq
                    if(irq0reg&J663BIT_LoAlertIrq0)
                    {
                        //暂时只考虑255FIFO
                        dev_rc663_read_reg(J663REG_FIFOLength, &regval);

                        i = 255-regval;
                        if(i >= (p_mfcmdinfo->nBytesToSend-wcnt))
                        {
                            i = (p_mfcmdinfo->nBytesToSend-wcnt);
                        }
                        dev_rc663_write_fifo(&wbuf[wcnt], i);
                        wcnt += i;
                        if(wcnt >= p_mfcmdinfo->nBytesToSend)
                        {
                            //关低报警
                            irq0en &= (~J663BIT_LoAlertIrq0);
                            dev_rc663_write_reg(J663REG_IRQ0En, irq0en|0x80);
                        }
                        //clear LoAlertIrq0 
                        dev_rc663_write_reg(J663REG_IRQ0, J663BIT_LoAlertIrq0);
                    }
                    /*transmission finished*/
                    if(irq0reg&J663BIT_TxIrq0)
                    {
                        //clear TxIrq0 
                        dev_rc663_write_reg(J663REG_IRQ0, J663BIT_TxIrq0);
                      #if 0  
                        //关发送完成中断
                        irq0en &= (~J663BIT_TxIrq0);
                        //开高报警中断
                        irq0en |= (J663BIT_IRQINV|J663BIT_HiAlerIrq0|J663BIT_RxIrq0);
                      #else
                        irq0en = (J663BIT_IRQINV|J663BIT_HiAlerIrq0|J663BIT_RxIrq0);
                      #endif
                        dev_rc663_write_reg(J663REG_IRQ0En, irq0en|0x80);
                        timerid = dev_get_timeID();
                        timerover = (p_mfcmdinfo->timeout+(((u32)p_mfcmdinfo->timeoutcnt)<<15))/2 + 300;//30000;          //30s
                    }
                    /*reception finished */
                    if(irq0reg&J663BIT_RxIrq0)
                    {
                        //clear J663BIT_RxIrq0
                        dev_rc663_write_reg(J663REG_IRQ0, J663BIT_RxIrq0);
                        //接收数据，暂时只考虑255FIFO
                        dev_rc663_read_reg(J663REG_FIFOLength, &regval);
                      #if 0  
                        if((buflen+regval)>512)
                        {
                            status = MI_OVFLERR;
                            break;
                        }
                        dev_rc663_read_fifo(&buf[buflen], regval);
                        buflen += regval;
                        //读接收的bits数
                        dev_rc663_read_reg(J663REG_RxBitCtrl, &regval);
                        nbitreceived = (regval&0x07);
                      #else
                        //FIFO中剩余的数据暂不接收
                        remainnum = regval;
                      #endif

                        /*process the error case*/
                        dev_rc663_read_reg(J663REG_Error, &errreg);
//rf_debug_printf("i2=%02X", errreg);     
                        if(errreg)/*Receiption completed with error*/
                        {
                            if(errreg&0x13) 
                            {
                                dev_rc663_read_reg(J663REG_RxBitCtrl, &regval);
                                //对于EMD案例判断:1.奇偶校验错或
                                //                2.完整性错且接收长度<2或字节接收不完整
                                //                继续接收
//rf_debug_printf("i2=%02X,%02x,%02x", errreg, regval, remainnum+buflen);     
                                if((  (0x12&errreg)     
                                    ||((0x01&errreg)
                                      &&(((remainnum+buflen)<5)||(regval&0x07))
                                      )
                                    )&&g_emden)
                                {   
                                    if(((0x17&errreg)==0x10)&&((remainnum+buflen)>4)&&(g_emden==1))
                                    {
                                        endflg = 1;
                                        status = MI_OK;
                                        break;
                                    }
                                    
                                    /*EMV TB306_12 this case is special*/
                                    if(1 == g_rc663_special_B_f)
                                    {
                                        if((1 == (remainnum+buflen))
                                         ||(10 < (remainnum+buflen)))
                                        {
                                            endflg = 1;
                                            status = MI_CRCERR;
                                            break;
                                        }
                                    }
                                  
                                  
                                    //发送重收命令
                                    dev_rc663_write_reg(J663REG_Command, J663CMD_Receive);
                                    //清FIFO
                                    dev_rc663_write_reg(J663REG_FIFOControl, 0x10);
                                    //启动定时器0   //,2
                                    dev_rc663_write_reg(J663REG_TControl, 0x11);    //启动定时器T0
                                  
//sprintf(&tmp[tmplen],"\r\ncrc:%02X,%02X,%d,%d\r\n", errreg, regval, remainnum+buflen, g_emden); 
//tmplen = strlen(tmp);                          
                                   if(regval&0x07)
                                    {
                                        if((errreg==0x05)
                                         ||((errreg==0x01)&&(4 < (remainnum+buflen))))
                                        {
                                            status = MI_INITERR;
                                        }
                                    }        
                                    buflen = 0;
                                }
                                else
                                {                                    
                                    endflg = 1;
                                    status = MI_CRCERR;
                                }
                            }
                            if(errreg&0x04) /*Collission error*/
                            {
                               #if 1 
                                //处理冲突位数及冲突为的结果
                                dev_rc663_read_reg(J663REG_RxBitCtrl, &regval);
                                p_mfcmdinfo->nBitsReceived= (regval&0x07);
                                dev_rc663_read_reg(J663REG_RxColl, &regval);
                                if(regval&0x80)p_mfcmdinfo->ErrorReg = (regval&0x7f);
                               #endif 
                                endflg = 1;
                                status = MI_COLLERR;
                            }
                        }
                        else/*Receiption completed without error*/
                        {
                            endflg = 1;
                            status = MI_OK;
                        }
                    }
                    /*IDlE interrupt*/
                    if(irq0reg&J663BIT_IdleIrq0)
                    {
                        //清标志
                        dev_rc663_write_reg(J663REG_IRQ0, J663BIT_RxIrq0);
                        endflg = 1;
                        status = MI_OK;
                    }
                    break;
                    
                }
                if(endflg == 1)
                {
                    //接收数据，暂时只考虑255FIFO
                    dev_rc663_read_reg(J663REG_FIFOLength, &regval);
                    if((buflen+regval)>512)
                    {
                        status = MI_OVFLERR;
                        break;
                    }
                    dev_rc663_read_fifo(&buf[buflen], regval);
                    buflen += regval;
                   #if 1 
                    //读接收的bits数
                    dev_rc663_read_reg(J663REG_RxBitCtrl, &regval);
                    nbitreceived = (regval&0x07);
                    //接收冲突数据
                   #endif 
                    
                    break;
                }
                
            }
            //==============================================
            //判断超时，防止RC663死锁
            if(dev_querry_time(timerid, timerover))
            {
//rf_debug_printf("t"); 
                //时间到
                status = MI_ACCESSTIMEOUT;   //超时
                break;
            }
        }
    }
    //关定时器
    dev_rc663_write_reg(J663REG_TControl, 0x0F);
    //禁止中断使能
    dev_rc663_write_reg(J663REG_IRQ0En, 0x80);
    dev_rc663_write_reg(J663REG_IRQ1En, 0xc0);
//rf_debug_printf("\r\nstatus=%d, len=%d\r\n", status, buflen);
//    if(MI_OK == status)
    {
        p_mfcmdinfo->nBytesReceived = buflen;
        p_mfcmdinfo->nBitsReceived = nbitreceived;
        //memcpy(rbuf, buf, buflen); 
      #if 0  
        memcpy(rbuf, buf, p_mfcmdinfo->nBitsReceived?buflen+1:buflen);//buflen should add by 1 due to it was decreased previously when the lsat byte received is a bit byte.
      #else
      #if 0
        if(p_mfcmdinfo->nBitsReceived)
        {
            i=buflen+1;
        }
        else
        {
            i=buflen;
        }
      #else
        i = buflen;
      #endif
        if(p_mfcmdinfo->nReceivedmax!=0)    //p_mfcmdinfo->nReceivedmax==0,说明该参数没有设置
        {
            if(i>p_mfcmdinfo->nReceivedmax)
            {
                i = p_mfcmdinfo->nReceivedmax;
            }
        }
        memcpy(rbuf, buf, i);//buflen should add by 1 due to it was decreased previously when the lsat byte received is a bit byte.
      #endif
//rf_PrintFormat(rbuf, buflen);
    }
//if(tmplen)uart_printf("%s", tmp);
    k_free(buf);
    return status;
}

s32 dev_rc663_mifare_auth(u8 key_type, u8 *key, u8 *snr, u8 block)
{
    s32 status = MI_OK;
    u8 databfr[12];
    u8 rtmp[100];
    u8 regval;
    MfCmdInfo mfcmdinfotmp;

    //准备数据
    memcpy(databfr, key, 6);

    dev_pcd_reset_mfinfo(&mfcmdinfotmp);
    mfcmdinfotmp.nBytesToSend = 6;         //数据长度
    mfcmdinfotmp.nReceivedmax = 100;
    mfcmdinfotmp.timeoutmode = PCD_TIMER_MODE_100US_1;
    mfcmdinfotmp.timeout = 50;              //待定，pengxuebin，????
    mfcmdinfotmp.cmd = J663CMD_LoadKey;
    status = dev_rc663_execute_cmd(&mfcmdinfotmp, databfr, rtmp);
//uart_printf("cmd11:status=%d\r\n", status);
    if(MI_OK != status)
    {
        return status;
    }
    
    //准备数据
    databfr[0] = key_type;     //key A or key B
    databfr[1] = block;          //address to authentication
    //memcpy(databfr+2, key, 6);  //6bytes key
    memcpy(databfr+2, snr, 4);  //4bytes UID
    
    //prepare data for common transceive
    dev_pcd_reset_mfinfo(&mfcmdinfotmp);    //复位结构
    mfcmdinfotmp.nBytesToSend = 6;         //数据长度
    mfcmdinfotmp.nReceivedmax = 100;
    mfcmdinfotmp.timeoutmode = PCD_TIMER_MODE_100US_1;
    mfcmdinfotmp.timeout = 500;              //待定，pengxuebin，????
    mfcmdinfotmp.cmd = J663CMD_MFAuthent;
    status = dev_rc663_execute_cmd(&mfcmdinfotmp, databfr, rtmp);
//uart_printf("cmd22:status=%d\r\n", status);
    //*   改到函数内部处理
    if(MI_OK == status)
    {
        dev_rc663_read_reg(J663REG_Status, &regval);
//uart_printf("reg_status=%02X\r\n", regval);        
        if(0 == (regval&0x20))//bit5
        {
            status = MI_AUTHERR;            //认证错
        } 
    }
    return status;
    
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
s8 dev_rc663_softpowerdown(void)
{
    s8  status =   MI_OK;

    //uart_printf("33333333333333333333333\r\n");
    /*soft reset*/
//    dev_rc663_reset(2);
    
    dev_rc663_modify_reg(J663REG_Command, 1, J663BIT_Standby); //软件POWERDOWN模式，3~6uA
    RC663_NRSTPD_OFF;
    return status;
    
}
s8 dev_rc663_softpoweron(void)
{
    
    RC663_NRSTPD_RST;
    delayms(1);
    dev_rc663_write_reg(J663REG_Command,J663CMD_IDLE);
    return 0;
}

/*******************************************************************************
 ** Descriotions:   发送REQA，查询磁场中是否有符合ISO14443A卡
 ** parameters:     无                                              
 ** Returned value: 0: 成功
                   其它:失败                                                                           
 ** Created By:     彭学斌
 ** Remarks:     
///////////////////////////////////////////////////////////////////////
// Request Command defined in ISO14443(MIFARE)
// Request,Anticoll,Select,return CardType(2 bytes)+CardSerialNo(4 bytes)    
// 寻卡，防冲突，选择卡    返回卡类型（2 bytes）+ 卡系列号(4 bytes)
///////////////////////////////////////////////////////////////////////                                                                            
*******************************************************************************/
s8 dev_rc663_send_reqa(u8 req_code, u8 *atq)
{
    s8 status = MI_OK;
    u8 wtmp[10];
    u8 rtmp[100];
    MfCmdInfo mfcmdinfotmp;
    
    *atq = 0;
    g_emden = 0;
    ////初始化
    //dev_pn512_write_reg(JREG_STATUS2, 0, JBIT_CRYPTO1ON);  //disable Crypto if activated before 
    //dev_pn512_write_reg(JREG_COLL, JBIT_VALUESAFTERCOLL);   //active values after coll 激活
    //dev_pn512_write_reg(JREG_TXMODE, 0, JBIT_CRCEN);       //disable TxCRC and RxCRC
    //dev_pn512_write_reg(JREG_RXMODE, 0, JBIT_CRCEN);
    //dev_pn512_write_reg(JREG_BITFRAMING, REQUEST_BITS);     //设置最后一字节传输的位数(7位)
    
    //设置需要传输的参数
    dev_pcd_reset_mfinfo(&mfcmdinfotmp);
    wtmp[0] = req_code;
    mfcmdinfotmp.cmd = J663CMD_Transceive;
    mfcmdinfotmp.nBytesToSend = 1;
    mfcmdinfotmp.nReceivedmax = 100;
    //设置超时
    mfcmdinfotmp.timeoutmode = PCD_TIMER_MODE_ETU_1;//PCD_TIMER_MODE_100US_1;
    mfcmdinfotmp.timeout = 40;       //10etu     //400us, 待定 pengxuebin，  ????
    
    status = dev_rc663_execute_cmd(&mfcmdinfotmp, wtmp, rtmp);
//uart_printf("reqa:status=%d\r\n", status);                             
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
    //dev_pn512_write_reg(JREG_BITFRAMING, 0);
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
s8 dev_rc663_halt_piccA(void)
{
    s8 status = MI_OK;
    u8 databfr[16];
    u8 rtmp[16];
    u8 regval;
    MfCmdInfo mfcmdinfotmp;
    
    //initialise data buffer
    databfr[0] = HALTA_CMD;
    databfr[1] = HALTA_PARAM;
    //databfr[2] = 0x57;  //The CRC bytes need not to be added,Stacy
    //databfr[3] = 0xcd;
    dev_rc663_modify_reg(J663REG_TxCrcPreset, 1, J663BIT_CRCEN);       //Enable TxCRC and RxCRC
    dev_rc663_modify_reg(J663REG_RxCrcPreset, 1, J663BIT_CRCEN);
    //dev_pn512_write_reg(JREG_STATUS2, 0, JBIT_CRYPTO1ON);  //disable Crypto if activated before 

    //set TxLastBits  RxLastBits and RxAlign to number of bits sent
    dev_rc663_read_reg(J663REG_TxDataNum,&regval);
    regval = regval &0xf8;      //send all bits
    dev_rc663_write_reg(J663REG_TxDataNum,regval);
    
    dev_pcd_reset_mfinfo(&mfcmdinfotmp);    //复位结构
    mfcmdinfotmp.nBytesToSend = HALTA_CMD_LENGTH;
    mfcmdinfotmp.nReceivedmax = 16;
    mfcmdinfotmp.timeoutmode = PCD_TIMER_MODE_ETU_1;
    mfcmdinfotmp.timeout = 106;         //1ms,
    mfcmdinfotmp.cmd = J663CMD_Transceive;
    status = dev_rc663_execute_cmd(&mfcmdinfotmp, databfr, rtmp);
    //dev_pn512_write_reg(JREG_TXMODE, 0, JBIT_CRCEN);       //Enable TxCRC and RxCRC
    //dev_pn512_write_reg(JREG_RXMODE, 0, JBIT_CRCEN);       
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
 ** Created By:     彭学斌 
 ** Remarks:       
*******************************************************************************/
s8 dev_rc663_req_piccB(u8 afi, u8 N, u8 req_code, u8 *atqB, u8 *atqBlen)
{
    s8 status = MI_OK; 
    u8 tmp[16];
    u8 tmp1[64];
    MfCmdInfo mfcmdinfotmp;

    //activate_deletion of bits after coll //激活冲突检测
//    dev_pn512_write_reg(JREG_COLL, 0);
    
    *atqBlen = 0;
    g_emden = 0;
    
    tmp[0] = 0x05;          //Apf
    tmp[1] = afi;           //afi
    //param:bit5=1,扩展ATQB支持
    tmp[2] = (req_code&0x08)|(N&0X07);//param;         //afi
    //prepare data for common transceive
    dev_pcd_reset_mfinfo(&mfcmdinfotmp);        //复位结构
    mfcmdinfotmp.nBytesToSend = 3;              //数据长度
    mfcmdinfotmp.nReceivedmax = 64;
    mfcmdinfotmp.timeoutmode = PCD_TIMER_MODE_ETU_1;//PCD_TIMER_MODE_100US_1;
    mfcmdinfotmp.timeout = 200;//64;//20090822_1  
                            //200;    //待定，pengxuebin，????
    mfcmdinfotmp.cmd = J663CMD_Transceive; 
    status = dev_rc663_execute_cmd(&mfcmdinfotmp, tmp, tmp1);
//uart_printf("reqb:%d\r\n", status);    

    #ifdef PCD_DEBUG  
    #endif    
  /*  
    if(MI_PROTOCOL_ERROR == status)
    {
        g_rf_special_flag++;
        if(g_rf_special_flag==2)
        {
            status = MI_OK;
        }
    }*/
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
//uart_printf("reqb:%d\r\n",status);  
        status = MI_COLLERR;            // collision occurs
    }
    
    //activate values after coll
//    dev_pn512_write_reg(JREG_COLL, JBIT_VALUESAFTERCOLL);   
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
 ** Created By:     彭学斌 
 ** Remarks:       this founction is used internal only, and cannot call by application program                                                                             
*******************************************************************************/
s8 dev_rc663_selectanticoll_picc(u8 grade, u8 grademax, u8 bitcount, u8 *snr, u8 *sak)
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
    u8 regval;
    
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
//        dev_rc663_write_reg(J663REG_RxThreshold       , 0x5F); //20140911
        //disable TxCRC and RxCRC  禁止发送接收CRC校验
        dev_rc663_modify_reg(J663REG_TxCrcPreset, 0, J663BIT_CRCEN);   
        dev_rc663_modify_reg(J663REG_RxCrcPreset, 0, J663BIT_CRCEN);

        //activate_deletion of bits after coll //激活冲突检测
        dev_rc663_modify_reg(J663REG_RxBitCtrl,  1, J663BIT_TSTOPNOW);
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
            //enable TxCRC and RxCRC  发送接收CRC校验
            dev_rc663_modify_reg(J663REG_TxCrcPreset, 1, J663BIT_CRCEN);   
            dev_rc663_modify_reg(J663REG_RxCrcPreset, 1, J663BIT_CRCEN);

            //dev_pn512_write_reg(JREG_COLL, JBIT_VALUESAFTERCOLL);  //该位只有在106kb/s的位防冲突时置0，其他要置1
        }
        
        dev_rc663_wait_etu(100);//(54);      //pengxuebin,20090822_1
        
        nbits = (bitcount%BITS_PER_BYTE);           //需要发送的非完整位
        nbytes = ((bitcount+7) / BITS_PER_BYTE);    //需要发送的数据
        
        databfr[0] = PCDTypeASel[grade&0x03];       //不同等级的SEL代码
        databfr[1] = 0x20 + (((bitcount/8)<<4)&0x70) + nbits;//NVB
        memcpy(&databfr[2], snr, nbytes);           //需要发送的数据
        
        //set TxLastBits and RxAlign to number of bits sent
        dev_rc663_read_reg(J663REG_TxDataNum,&regval);
        regval = regval &0xf8;
        dev_rc663_write_reg(J663REG_TxDataNum, (regval | nbits));
      //---->20141009
        dev_rc663_read_reg(J663REG_RxBitCtrl, &regval);
        regval = regval &0x88;
        dev_rc663_write_reg(J663REG_RxBitCtrl, (regval | (nbits<<4)));
      //<----
        
        
        //prepare data for common transceive
        dev_pcd_reset_mfinfo(&mfcmdinfotmp);        //复位结构
        mfcmdinfotmp.nBytesToSend = (nbytes+2);
        mfcmdinfotmp.nReceivedmax = 100;
        mfcmdinfotmp.timeoutmode = PCD_TIMER_MODE_ETU_1;//PCD_TIMER_MODE_100US_1;
        mfcmdinfotmp.timeout = 40;//19;//1000;//暂时定100etu  //5000;                  //待定，pengxuebin，????
        mfcmdinfotmp.cmd = J663CMD_Transceive; 
        
#ifdef PCD_DEBUG   
uart_printf("cascanticoll:send:bytes=%x, bits=%x\r\n", mfcmdinfotmp.nBytesToSend, ((nbits << UPPER_NIBBLE_SHIFT) | nbits));        
PrintFormat(databfr, mfcmdinfotmp.nBytesToSend);
#endif

//g_debug_flg = 1;
        status = dev_rc663_execute_cmd(&mfcmdinfotmp, databfr, rtmp);
//g_debug_flg = 0;

#ifdef PCD_DEBUG    
uart_printf("cascanticoll:status=%d\r\n", status);        
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
            Uart_Printf("rec:bytes=%x,Bits=%x\r\n", mfcmdinfotmp.nBytesReceived, mfcmdinfotmp.nBitsReceived);
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
//                status = MI_OK;  //pengxuebin,2009-8-20 16:19:31
            }
            else    //没有冲突
            {
                if((rbits+bitcount) == COMPLETE_UID_BITS)   //40
                {
            
                    //nbits = rtmp[4+byteOffset-nbytes]; 
                    #ifdef PCD_DEBUG   
                        Uart_Printf("BCC:nbits=%x\r\n", nbits);
                        PrintFormat(snr, (4-j));   
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
//uart_printf("dev_rc663_selectanticoll_picc:\r\n");        
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
 ** Created By:     彭学斌 
 ** Remarks:       
*******************************************************************************/
s8 dev_rc663_send_rats_piccA(u8 cid, u8 *rbuf, u16 *rlen)
{
    s8 status = MI_OK;
//    u8 i,j; 
    u8 k;
    u8 n;
    u8 tmp[10];
    //u8 tmp1[256]; 
    u8 *tmp1=NULL;
    MfCmdInfo mfcmdinfotmp;
    *rlen = 0;

    
    tmp1 = (u8 *)k_malloc(256);
    if(NULL == tmp1)
    {
        return MI_OTHERERR;
    }
    g_emden = 1;
//    dev_rc663_modify_reg(J663REG_RxCtrl, 1, J663BIT_EMDSup);
    for(n=0; n<3; n++)
    {
        dev_rc663_wait_etu(60);     
        tmp[0] = 0xE0;
        tmp[1] = (cid&0x0f)|(PCDFSDI<<4);
        dev_pcd_reset_mfinfo(&mfcmdinfotmp);        //复位结构
        mfcmdinfotmp.nBytesToSend = 2;              //数据长度
        mfcmdinfotmp.nReceivedmax = 256;
        mfcmdinfotmp.timeoutmode = PCD_TIMER_MODE_ETU_1;//PCD_TIMER_MODE_100US_1;
        mfcmdinfotmp.timeout = 600;//600;//20090822_1
       
        mfcmdinfotmp.cmd = J663CMD_Transceive;
        tmp1[0]=0x00;
//dev_misc_led_green(0);
//dev_misc_led_red(0);
//   g_debug_flg = 1;  
        status = dev_rc663_execute_cmd(&mfcmdinfotmp, tmp, tmp1);
//   g_debug_flg = 0;     
//uart_printf("rats:%x, %x, %x\r\n", status, mfcmdinfotmp.nBitsReceived, mfcmdinfotmp.nBytesReceived);
//PrintFormat(tmp1, mfcmdinfotmp.nBytesReceived);
        //---->针对TA335-6处理,pengxuebin,20150509
      #if 0  
        if((MI_CRCERR == status) && (mfcmdinfotmp.nBitsReceived==0) && (mfcmdinfotmp.nBytesReceived==5))
        {
            //
            if((tmp1[0]==0xA0)&&(tmp1[1]==0x2E)
              &&(tmp1[2]==0x10)&&(tmp1[3]==0x88)
              &&(tmp1[4]==0x40))
            {
                memcpy(tmp1, "\x05\x72\x80\x40\x02", 5);
                status = MI_OK;
            }
        }
      #endif  
//uart_printf("rats:%x, %x, %x\r\n", status, mfcmdinfotmp.nBitsReceived, mfcmdinfotmp.nBytesReceived);
//PrintFormat(tmp1, mfcmdinfotmp.nBytesReceived);

        //<----
        if((MI_OK == status))
        {
            if(0 != mfcmdinfotmp.nBytesReceived)
            {
                //解析ATS
                //TL   (长度字节)
                k = tmp1[0];
//    uart_printf("Bits=%02X, %02X, %02X\r\n", mfcmdinfotmp.nBitsReceived, mfcmdinfotmp.nBytesReceived, k);
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
            else
            {
                status = MI_OTHERERR; //?
                break;
            }
        }
        else if((MI_PARITYERR == status)||(MI_CRCERR == status)||(MI_ACK_SUPPOSED == status)||(MI_NOTAGERR == status)||(MI_OTHERERR==status)) 
        {
//uart_printf("nBytes=%x,tmp1=%x\r\n", mfcmdinfotmp.nBytesReceived, tmp1[0]);
//PrintFormat(tmp1, mfcmdinfotmp.nBytesReceived);
            if((MI_CRCERR == status)&&(6 == mfcmdinfotmp.nBytesReceived) && (5 == tmp1[0]))  //针对TA307_4处理
            {
            }
            else if((mfcmdinfotmp.nBytesReceived >=1) && (tmp1[0] >= 4))
            {
                break;
            }

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
s8 dev_rc663_attrib(u8 cid, u8 *pupi, u8 brTx, u8 brRx, u8 param3, u32 fwt)
{
    s8 status = MI_OK; 
    u8 tmp[16];
    //u8 tmp1[256];
    u8 *tmp1 = NULL;
    u8 i;
    MfCmdInfo mfcmdinfotmp;
    
    
    tmp1 = (u8 *)k_malloc(256);
    if(NULL == tmp1)
    {
        return MI_OTHERERR;
    }

    g_emden = 2;
    dev_rc663_modify_reg(J663REG_RxCrcPreset, 1, J663BIT_RxForcecWrite);
    for(i=0; i<3; i++)
    {
        dev_rc663_wait_etu(60);
  
        tmp[0] = 0x1d;              //command
        memcpy(&tmp[1], pupi, 4);   //Identifier
        tmp[5] = 0x00;              // EOF/SOF required, default TR0/TR1        
        tmp[6] = 0x08|((brTx&0x03)<<4)|((brRx&0x03)<<6);  // Max frame 256 
        tmp[7] = param3;        // Param3, ISO/IEC 14443-4 compliant?
        tmp[8] = cid&0x0f;          //CID
         
        //prepare data for common transceive
        dev_pcd_reset_mfinfo(&mfcmdinfotmp);        //复位结构
        mfcmdinfotmp.nBytesToSend = 9;              //数据长度
        mfcmdinfotmp.nReceivedmax = 256;
        mfcmdinfotmp.timeoutmode = PCD_TIMER_MODE_nETU_1;//PCD_TIMER_MODE_100US_1;
      
        mfcmdinfotmp.timeout = ((0x01 << PICCData[cid].FWI) *32) + 384+500;
        mfcmdinfotmp.cmd = J663CMD_Transceive;
      #ifdef PCD_DEBUF
        Uart_Printf("dev_pcd_attrib:wlen=%d\r\n", mfcmdinfotmp.nBytesToSend);
        PrintFormat(tmp, mfcmdinfotmp.nBytesToSend);
      #endif    
        g_rc663_special_B_f=1;
        status = dev_rc663_execute_cmd(&mfcmdinfotmp, tmp, tmp1);    
        g_rc663_special_B_f=0;
//uart_printf("a:status=%x\r\n", status);
//PrintFormat(tmp1, mfcmdinfotmp.nBytesReceived);
        /*if(MI_PROTOCOL_ERROR == status)
        {
            if(g_rf_special_flag==2)
            {
                status = MI_OK;
            }
        }*/
#if 1        
        if((MI_OK == status)||(MI_CRCERR == status))
        {
//PrintFormat(tmp1, mfcmdinfotmp.nBytesReceived);
            if(mfcmdinfotmp.nBytesReceived==0)
            {
                status = MI_OTHERERR;
            }
            else if(mfcmdinfotmp.nBytesReceived==3)
            {
                if(tmp1[0]==0x00)
                {
                    if((tmp1[1]!=0x78)||(tmp1[2]!=0xf0))
                    {       //CRC校验错
                        status = MI_CRCERR;
                    }
                }
            }
            else if(3 < mfcmdinfotmp.nBytesReceived)
            {
                status = MI_OTHERERR;
                break;
            }
        }
#endif        
        if(MI_OK == status)
        {
#ifdef PCD_DEBUG   
Uart_Printf("dev_pcd_attrib:rlen=%d\r\n", mfcmdinfotmp.nBytesReceived);
PrintFormat(tmp1, mfcmdinfotmp.nBytesReceived);
#endif     
            
            if(mfcmdinfotmp.nBytesReceived > 3) //!= 1) //
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
        else if((MI_PARITYERR == status)||(MI_CRCERR == status)||(MI_ACK_SUPPOSED == status)
              ||(MI_NOTAGERR == status)||(MI_OTHERERR==status)||(MI_PROTOCOL_ERROR == status)||(MI_COLLERR==status))//
        {
            //rf_delayms(6);//dev_delay_ms(6); 
            dev_rc663_wait_etu(fwt);
        }
      #if 0  
        else if(MI_COLLERR==status)
        {
            //判断CRC
            
            
        }
      #endif  
        else
        {
            break;
        }
    }
    dev_rc663_modify_reg(J663REG_RxCrcPreset, 0, J663BIT_RxForcecWrite);
    
    k_free(tmp1);
    return status; 
}
/*******************************************************************************
 ** Descriotions:    
 ** parameters:      
                    
 ** Returned value:  0: 成功
                    其它:失败 
 ** Created By:     彭学斌 2014/1/16 9:45:05
 ** Remarks:       
*******************************************************************************/
extern void test_set0(void);
s8 dev_rc663_exchange_piccA(u8 *wbuf, u16 wlen, u8 *rbuf, u16 *rlen, u32 etu, u8 wtx)
{
    s32 status;
    u32 tmp;
    MfCmdInfo mfcmdinfotmp;
    
    dev_rc663_wait_etu(60);      //pengxuebin,20160324
    dev_pcd_reset_mfinfo(&mfcmdinfotmp);                //复位结构
    mfcmdinfotmp.nBytesToSend = wlen;                   //数据长度
    mfcmdinfotmp.nReceivedmax = *rlen;
    *rlen = 0;
    mfcmdinfotmp.timeoutmode = PCD_TIMER_MODE_nETU_1;  //以32etu为单位(32*(fc/128))
    tmp = (etu*(wtx+1)*32)+384+500;//10;
 
//    tmp += 3000;  //sxl20110401
//    dev_rc663_write_reg(J663REG_RxThreshold       , 0x5F); //20140911
    mfcmdinfotmp.timeout = tmp;//&0x7fff;
    mfcmdinfotmp.timeoutcnt = 0;//(tmp>>15);
//    if(0 == mfcmdinfotmp.timeout)mfcmdinfotmp.timeout++;
    mfcmdinfotmp.cmd = J663CMD_Transceive;
    #ifdef PCD_DEBUG  
    uart_printf("__pcd_exchange:wlen=%x\r\n", mfcmdinfotmp.nBytesToSend); 
    rf_PrintFormat(wbuf, mfcmdinfotmp.nBytesToSend);  
    #endif 
    
//g_debug_flg=1;     
    status = dev_rc663_execute_cmd(&mfcmdinfotmp, wbuf, rbuf); 
//g_debug_flg=0; 
/*if(0 != status)    
{
//test_set0(); 
dev_misc_led_green(0);
}*/

    #ifdef PCD_DEBUG  
    uart_printf("__pcd_exchange:status=%d, rlen=%x, bits=%x\r\n", status, mfcmdinfotmp.nBytesReceived, mfcmdinfotmp.nBitsReceived); 
    rf_PrintFormat(rbuf, mfcmdinfotmp.nBytesReceived);
    #endif
  /*  
    if(MI_PROTOCOL_ERROR == status)
    {
        if(g_rf_special_flag==2)
        {
            status = MI_OK;
        }
    }*/
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
//===================================================
s32 dev_rc663_test(void)
{  
#if 0    
    u32 timerid;
    u32 timerid1;
    u8 tmp[8];

 #if 0
dev_misc_led_yellow(1);    
//     dev_rc663_reset(2);
dev_misc_led_yellow(0);  
    
    memset(tmp, 0, 8);
    dev_rc663_read_reg(J663REG_IRQ0, &tmp[0]);
    dev_rc663_read_reg(J663REG_IRQ1, &tmp[1]);
    dev_rc663_read_reg(J663REG_IRQ0En, &tmp[2]);
    dev_rc663_read_reg(J663REG_IRQ1En, &tmp[3]);  
uart_printf("TT1:\r\n");
//PrintFormat(tmp, 4);
uart_printf("%02X,%02X,%02X,%02X\r\n", tmp[0],tmp[1],tmp[2],tmp[3]);
dev_misc_led_yellow(1); 
    dev_rc663_write_reg(J663REG_IRQ0En, 0x80);
    rf_delayms(5);//dev_delay_ms(5);
    dev_rc663_write_reg(J663REG_IRQ0, 0x7F);
    dev_rc663_write_reg(J663REG_IRQ1, 0x7F);
    dev_rc663_write_reg(J663REG_IRQ1En, 0xc0);  //
    memset(tmp, 0, 8);
    dev_rc663_read_reg(J663REG_IRQ0, &tmp[0]);
    dev_rc663_read_reg(J663REG_IRQ1, &tmp[1]);
    dev_rc663_read_reg(J663REG_IRQ0En, &tmp[2]);
    dev_rc663_read_reg(J663REG_IRQ1En, &tmp[3]);  
uart_printf("TT2:\r\n");
//PrintFormat(tmp, 4);
uart_printf("%02X,%02X,%02X,%02X\r\n", tmp[0],tmp[1],tmp[2],tmp[3]);
    dev_rc663_write_reg(J663REG_IRQ0, 0x7F);
    dev_rc663_write_reg(J663REG_IRQ1, 0x7F);
dev_misc_led_yellow(0);  
    
    dev_rc663_read_reg(J663REG_IRQ0, &tmp[0]);
    dev_rc663_read_reg(J663REG_IRQ1, &tmp[1]);  
uart_printf("TT3:\r\n");
//PrintFormat(tmp, 2);
uart_printf("%02X,%02X\r\n", tmp[0],tmp[1]);
    
    //关闭定时器
    dev_rc663_write_reg(J663REG_TControl, 0x0F);

    //设置T2T0一组  T3T1一组
    dev_rc663_write_reg(J663REG_T0Control, 0x08);
    dev_rc663_write_reg(J663REG_T2Control, 0x02);

    dev_rc663_write_reg(J663REG_T0ReloadHi, 0xFF);//0x05);
    dev_rc663_write_reg(J663REG_T0ReloadLo, 0xff);//0x4c); //0x54c=1356=100us

    //dev_rc663_write_reg(J663REG_T2ReloadHi, 0x03);
    //dev_rc663_write_reg(J663REG_T2ReloadLo, 0xe8); //0x3E8=1000
    dev_rc663_write_reg(J663REG_T2ReloadHi, 0x00);
    dev_rc663_write_reg(J663REG_T2ReloadLo, 0x05); //0x3E8=1000


    dev_rc663_read_reg(J663REG_T0ReloadHi, &tmp[0]);
    dev_rc663_read_reg(J663REG_T0ReloadLo, &tmp[1]);  

    dev_rc663_read_reg(J663REG_T2ReloadHi, &tmp[2]);
    dev_rc663_read_reg(J663REG_T2ReloadLo, &tmp[3]);
    dev_rc663_read_reg(J663REG_IRQ0, &tmp[4]);
    dev_rc663_read_reg(J663REG_IRQ1, &tmp[5]);  
uart_printf("reg:\r\n");
//PrintFormat(tmp, 6);
uart_printf("%02X,%02X,%02X,%02X,%02X,%02X\r\n", tmp[0],tmp[1],tmp[2],tmp[3],tmp[4],tmp[5]);

    
    dev_rc663_write_reg(J663REG_IRQ0En, 0x80);
    dev_rc663_write_reg(J663REG_IRQ1En, 0xc4);  //
    dev_rc663_write_reg(J663REG_TControl, 0x55);  //启动定时器T0T2
    dev_rc663_write_reg(J663REG_T0ReloadHi, 0xFF);//0x05);
    dev_rc663_write_reg(J663REG_T0ReloadLo, 0xff);
    timerid = dev_get_timeID();
    while(1)
    {
        if(0 == dev_rf_int_get())
        {
            dev_rc663_read_reg(J663REG_IRQ0, &tmp[0]);
            dev_rc663_read_reg(J663REG_IRQ1, &tmp[1]);
            dev_rc663_write_reg(J663REG_IRQ0, tmp[0]);
            dev_rc663_write_reg(J663REG_IRQ1, tmp[1]);
            
//            dev_rc663_write_reg(J663REG_TControl, 0x05); 
uart_printf("t:%02X, %02X\r\n", tmp[0], tmp[1]);
            if(tmp[1]&0x04)
            {
                timerid1 = dev_get_timeID();
uart_printf("testtimer:%d, %d, %d\r\n", timerid, timerid1, timerid1-timerid);
                return 0;
            }        
        }
        if(dev_querry_time(timerid, 5000))
        {
uart_printf("testtimer: time out!\r\n");  
            return 0;
        }
    }
    dev_rc663_wait_etu(40);
#endif
#if 0
    //关闭定时器
    dev_rc663_write_reg(J663REG_TControl, 0x0F);

    dev_rc663_write_reg(J663REG_T0ReloadHi, 0xFF);//0x05);
    dev_rc663_write_reg(J663REG_T0ReloadLo, 0xff);//0x4c); //0x54c=1356=100us

    //dev_rc663_write_reg(J663REG_T2ReloadHi, 0x03);
    //dev_rc663_write_reg(J663REG_T2ReloadLo, 0xe8); //0x3E8=1000
    dev_rc663_write_reg(J663REG_T2ReloadHi, 0x00);
    dev_rc663_write_reg(J663REG_T2ReloadLo, 0x05); //0x3E8=1000

    memset(tmp, 0, 6);
    dev_rc663_read_reg(J663REG_T0ReloadHi, &tmp[0]);
    dev_rc663_read_reg(J663REG_T0ReloadLo, &tmp[1]);  

    dev_rc663_read_reg(J663REG_T2ReloadHi, &tmp[2]);
    dev_rc663_read_reg(J663REG_T2ReloadLo, &tmp[3]);
    dev_rc663_read_reg(J663REG_IRQ0, &tmp[4]);
    dev_rc663_read_reg(J663REG_IRQ1, &tmp[5]);  
uart_printf("reg:\r\n");
//PrintFormat(tmp, 6);
uart_printf("%02X,%02X,%02X,%02X,%02X,%02X\r\n", tmp[0],tmp[1],tmp[2],tmp[3],tmp[4],tmp[5]);
#endif
  while(1)
  {
    dev_rc663_write_reg(J663REG_TControl, 0x0F);
    dev_rc663_write_reg(J663REG_T0ReloadHi, 0xFF);//0x05);
    dev_rc663_read_reg(J663REG_T0ReloadHi, &tmp[0]);
uart_printf("reg:%02X\r\n", tmp[0]);
    rf_delayms(500);//dev_delay_ms(500);
  } 
#endif  
    return 0;
}

