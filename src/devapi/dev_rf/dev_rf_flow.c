#include "devglobal.h"

#include "devapi/dev_rf/dev_rf_flow.h"
#include "devapi/dev_rf/dev_rf_io.h"
#include "devapi/dev_rf/pn512/pn512reg.h"
#include "devapi/dev_rf/pn512/dev_pn512.h"
#include "devapi/dev_rf/rc663/mfrc663reg.h"
#include "devapi/dev_rf/rc663/dev_rc663.h"
#include "devapi/dev_rf/dev_rf_api.h"



u8 g_rf_morecard_cnt = 0;
s32 g_dev_pcd_fd = -1;
//u8 poll_enable = 1;
_nCHECKPARAM checkparam;
nPICCData PICCData[PICC_CIDCNTMAX];
u16 const PCDFSCIToFSCTab[16]={
                                0x10, 0x18, 0x20, 0x28,
                                0x30, 0x40, 0x60, 0x80,
                                0x100,0x100,0x100,0x100,
                                0x100,0x100,0x100,0x100,
                               };
static u8 picctype_exist_flg[3]={0, 0, 0};
u8  mifare_ack[2];
//static 
u8 g_rf_lp_step = 0;
u8 g_rf_lp_flg  = 0;
u8 g_rf_lp_mod = 0;
u8 card_present = 0;
//u32 g_search_timerid=0;
//u8 g_search_type=0;
//#define RF_SEARCH_TIME    6 //200

u8  const PCDTypeASel[3] = {0x93, 0x95, 0x97};


//static 
//u8 modulation_depth =  0x10;
//static u8 rfcfgreg_valueA   = 0xF8;    
//static u8 rfcfgreg_valueB   = 0xF8;
RF_PARAM g_rf_param;

#define PCD_DRIVER_TYPE_NONE    0
#define PCD_DRIVER_TYPE_MFRC663 1
#define PCD_DRIVER_TYPE_PN512   2
static s32 g_pcd_driver_type = -1;
static u8 g_pcd_chip_type=PCD_CHIP_TYPE_PN512;   //针对PN512 PIN对PIN芯片的不同型号

u8 dev_rf_get_chiptype(void)
{
    return g_pcd_chip_type;
}

void rf_delayms(u32 ms)
{
    dev_user_delay_ms(ms);
}
void rf_delayus(u32 us)
{
    dev_user_delay_us(us);
}
s32 dev_rf_store_param(u8 type, u8 *wbuf, s32 wlen)
{
  #if 0  
    u32 addr;
    u32 len;
    s32 ret;
    ret = dynamic_vfs_get_fileinfo(PARAMINI_PATH, &addr, &len);
    if(ret < 0)
    {
        //文件没有找到
        return -1;
    }
    if(type == 0)   //PN512
    {
        
    }
    else if(type == 1)  //RC663
    {
        addr += RF_PARAM_LEN_MAX;
    }
    ret = dev_flash_write(addr, wbuf, wlen);
    if(ret >= 0)
    {
        return 0;
    }
    else
  #endif 
    s32 ret;
    ret = dev_misc_machineparam_set(MACHINE_PARAM_RF, wbuf, wlen);
    if(ret < 0)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}
/*******************************************************************************
*******************************************************************************/
s32 dev_rf_write_param(void)
{
  #if 0  
    s8 param[32];
    s32 ret;
    u8 paramnum;    
    
    if(PCD_DRIVER_TYPE_MFRC663==g_pcd_driver_type)
    {    
        paramnum = 3;
        dev_maths_bcd_to_asc((u8*)param, (u8*)&g_rf_param, paramnum);
        ret = dev_sys_set_key_value(PARAMINI_PATH, PARAMINI_SECTION, PARAM_RFRC663, param);
    }
    else if(PCD_DRIVER_TYPE_PN512==g_pcd_driver_type)
    {
        paramnum = 8;
        dev_maths_bcd_to_asc(param, (u8*)&g_rf_param, paramnum);
        ret = dev_sys_set_key_value(PARAMINI_PATH, PARAMINI_SECTION, PARAM_RFPN512, param);
    }
    else
    {
        ret = -1;
    }
    if(ret < 0)
    {
        return -1;
    }
    else
    {
        return 0;
    }
  #else
    u32 i;
    u8 *ap;
    ap = (u8*)&g_rf_param;
    //暂时不考虑g_pn512_param中的参数合法性判断,开机初始化参数时处理
    //计算校验
    if(PCD_DRIVER_TYPE_PN512==g_pcd_driver_type)
    {
        g_rf_param.m_type = 0;
    }
    else if(PCD_DRIVER_TYPE_MFRC663==g_pcd_driver_type)
    {
        g_rf_param.m_type = 1;
    }
    g_rf_param.bcc = 0;
    g_rf_param.lrc = 0;                      
    for(i=0; i<(RF_PARAM_LEN_MAX-2); i++)
    {
        g_rf_param.bcc += ap[i];
        g_rf_param.lrc ^= ap[i];
    }
    //暂时没有考虑4K中包含其他数据的保护
    if(PCD_DRIVER_TYPE_PN512==g_pcd_driver_type)
    {
        return dev_rf_store_param(0, ap, RF_PARAM_LEN_MAX);
    }
    else if(PCD_DRIVER_TYPE_MFRC663==g_pcd_driver_type)
    {
        return dev_rf_store_param(1, ap, RF_PARAM_LEN_MAX);  
    }
    else
    {
        return -1;
    }
  #endif
}              
/*******************************************************************************
*******************************************************************************/
#if 0
s32 dev_rf_phase_param(s8 *src, u32 srclen, u8 *param, u8 paramnum)
{
    s32 i;

    if(srclen>=(paramnum<<1))
    {
        i = paramnum<<1;
    }
    else
    {
        return -1;
    }
    dev_maths_asc_to_bcd((s8*)param, (s8*)src, i);
    return 0;
}
#endif
s32 dev_rf_get_param(u8 type, u8 *rbuf, s32 rlen)
{
  #if 0  
    u32 addr;
    u32 len;
    s32 ret;

    ret = dynamic_vfs_get_fileinfo(PARAMINI_PATH, &addr, &len);
    if(ret < 0)
    {
        //文件没有找到
        return -1;
    }
    if(type == 0)   //PN512
    {
        
    }
    else if(type == 1)  //RC663
    {
        addr += RF_PARAM_LEN_MAX;
    }

    ret = dev_flash_read(addr, rbuf, rlen);
    if(ret == rlen)
    {
        return 0;
    }
    else
  #endif   
    s32 ret;

    ret = dev_misc_machineparam_get(MACHINE_PARAM_RF, rbuf, rlen);
    if(ret < 0)
    {
        return -1;
    }    
    if(rbuf[0] != type)
    {
        return -1;
    }
    return 0;
}


s32 dev_rf_read_param(void)
{  
  #if 0  
    s8 param[128];
    s32 ret;
    u8 paramnum; 
    s32 i;
    
    if(PCD_DRIVER_TYPE_MFRC663==g_pcd_driver_type)
    {
        ret = dev_sys_get_key_value(PARAMINI_PATH, PARAMINI_SECTION, PARAM_RFRC663, param);
        paramnum = 3;
    }
    else if(PCD_DRIVER_TYPE_PN512==g_pcd_driver_type)
    {
        ret = dev_sys_get_key_value(PARAMINI_PATH, PARAMINI_SECTION, PARAM_RFPN512, param);
        paramnum = 8;
    }
    else
    {
        ret = -1;
    }
dev_debug_printf("%s(%d):ret=%d\r\n", __FUNCTION__, __LINE__, ret);    
    if(ret < 0)
    {
        return -1;
    }        
    i = strlen(param);
    ret = dev_rf_phase_param(param, i, (u8*)&g_rf_param, paramnum);
    if(ret < 0)
    {
        return -1;
    }
    else
    {
        return 0;
    }
  #else    
    u32 i;  
    u8 *ap; 
    u8 bcc;
    u8 lrc;
    s32 ret;
    
    //PN512_PARAM l_pn512_param;
    ap = (u8*)&g_rf_param;

    //读数据
    //if(1 == SpiFlashReadMoreByteData(PN512_PARAM_ADDR_STA, ap, PN512_PARAM_LEN_MAX))
    
    //暂时没有考虑4K中包含其他数据的保护
    if(PCD_DRIVER_TYPE_PN512==g_pcd_driver_type)
    {
        ret = dev_rf_get_param(0, ap, RF_PARAM_LEN_MAX);
    }
    else if(PCD_DRIVER_TYPE_MFRC663==g_pcd_driver_type)
    {
        ret = dev_rf_get_param(1, ap, RF_PARAM_LEN_MAX);  
    }
    else
    {
        return -1;
    }
    if(0 == ret)
    {
        //校验
        bcc = 0;
        lrc = 0;
        for(i=0; i<(RF_PARAM_LEN_MAX-2); i++)
        {               
            bcc += ap[i];
            lrc ^= ap[i];
        }
        if((bcc == g_rf_param.bcc) && (lrc == g_rf_param.lrc))
        {
        /*if((g_pn512_param.rfmodgsp == 0) && (g_pn512_param.rfcfgregA == 0) && (g_pn512_param.rfcfgregB == 0))
            return -1;
        else */           
                    return 0;
        }
    }
    return -1;
  #endif  
}
/*******************************************************************************
 ** Descriotions:   打开RF设备
 ** parameters:                                                              
 ** Returned value: 0:失败
                    1:成功  
                                                                                              
 ** Created By:     彭学斌 2009-6-30 15:32:05           
 ** Remarks:                                                                                  
*******************************************************************************/
s32 dev_pcd_init(void)
{       
    u8 tmp[32];
    u8 regval;
    s32 ret;
    
    dev_rf_io_init();   
    //可以通过版本来判断是PN512或RC663
    if(-1 == g_pcd_driver_type)
    {  
        
        if(0 == dev_rf_support_judge())
        {
            g_pcd_driver_type = PCD_DRIVER_TYPE_NONE;  
        }
        else
        {
            //暂时默认为PN512
          //  g_pcd_driver_type = PCD_DRIVER_TYPE_PN512; 
            while(1)
            { 
                ret = dev_pn512_read_version(&regval);
                if(ret>=0)
                { 
                    g_pcd_driver_type = PCD_DRIVER_TYPE_PN512;//PCD_DRIVER_TYPE_NONE;
                    //sprintf((char*)tmp, "RF DRIVER ERR!\r\n");
                    if(regval==0x12)
                    {
                        g_pcd_chip_type = PCD_CHIP_TYPE_MH1608;
                        sprintf((char*)tmp, "HM1608 VER:%x\r\n", regval);
                    }
                    else if(regval == 0xB2)
                    {
                        g_pcd_chip_type = PCD_CHIP_TYPE_ZH3801;
                        sprintf((char*)tmp, "PN512 VER:%x\r\n", regval);
                    }
                    else if(regval == 0x88)
                    {
                        g_pcd_chip_type = PCD_CHIP_TYPE_F17550;
                        sprintf((char*)tmp, "PN512 VER:%x\r\n", regval);
                    }
                    else //if(regval==0x82)
                    {
                        sprintf((char*)tmp, "PN512 VER:%x\r\n", regval);
                    }
                    break;
                }
                ret = dev_rc663_read_version(&regval);
                if(ret>=0)
                {
                    g_pcd_driver_type = PCD_DRIVER_TYPE_MFRC663;
                    sprintf((char*)tmp, "MFRC VER:%x\r\n", regval);
                    break;
                } 
                else    
                {
                    g_pcd_driver_type = PCD_DRIVER_TYPE_NONE;
                    sprintf((char*)tmp, "RF DRIVER ERR!\r\n");
                } 
                break;
            }
            //dev_com_write(DEBUG_PORT_NO, tmp, strlen((char*)tmp));
            
        }
        
    }
    else
    {
        ret = 0;
    }
//    test_init();
    return ret;
}
#if 0
u32 dev_pcd_read_intpin(void)
{
  #if 0  
    return PCD_INT_READ;
  #endif
}
#endif

void dev_pcd_param_set_default(void)
{      
    dev_pcd_init();
    memset((s8*)&g_rf_param, 0, RF_PARAM_LEN_MAX);
    if(PCD_DRIVER_TYPE_MFRC663==g_pcd_driver_type)
    {
        g_rf_param.rfmodgsp = 0x0C;
        g_rf_param.rfcfgregA = 0x06; 
        g_rf_param.rfcfgregB = 0x02;
        g_rf_param.rxthreshold_a = 0x46;
        g_rf_param.rxthreshold_b = 0x36;
    }
    else
    {
        if((PCD_CHIP_TYPE_ZH3801==g_pcd_chip_type)
         ||(PCD_CHIP_TYPE_F17550==g_pcd_chip_type))
        {
            g_rf_param.rfmodgsp = 0x1A;   
            g_rf_param.rfcfgregA = 0x58;
            g_rf_param.rfcfgregB = 0x58;
        }
        else
        {
          #if 0  
            g_rf_param.rfmodgsp = 0x03;
          #else
            g_rf_param.rfmodgsp = 0x1E;                  
          #endif
            g_rf_param.rfcfgregA = 0x68;//0x68; 
            g_rf_param.rfcfgregB = 0x68;//0x68;
        }
        g_rf_param.rxthreshold_a = 0x84;
        g_rf_param.rxthreshold_b = 0x55;
//dev_debug_printf("%s(%d):\r\n", __FUNCTION__, __LINE__); 
    }
    dev_rf_write_param();
    
}


void dev_pcd_initparam(void)
{
    u8 i;
    
    #ifdef PCD_DEBUG   
        Uart_Printf("dev_pcd_initparam:\r\n");  
    #endif     
    for(i=0; i<PICC_CIDCNTMAX; i++)
    {
        PICCData[i].status = PICC_STATUS_IDLE;      //回收所有的卡号
    }
    // 设置TYPEB卡调制深度
    // SpiFlashReadMoreByteData(MODULATION_DEPTH_ADDR, &modulation_depth, 1);
    //modulation_depth = SpiFlashReadByteData(MODULATION_DEPTH_ADDR);
    //if(modulation_depth > 0x1f)   modulation_depth = 0x10;
    if(dev_rf_read_param() < 0)
    {
        dev_pcd_param_set_default();
    }
}
#if 0
u8 dev_pcd_set_param(u8 *wbuf, u16 wlen)
{
    u8 m;//,n;
    m = wbuf[0];
    if(m>0x1f)
    {
        //g_rf_param.rfmodgsp = 0x03;
        return 2;
    }
    else
    {
        g_rf_param.rfmodgsp = m;
    }
//    SpiFlashSectorErase(MODULATION_DEPTH_ADDR);
//    SpiFlashByteWrite(MODULATION_DEPTH_ADDR, modulation_depth);
    //------------------
    if(wlen>1)
    {
        g_rf_param.rfcfgregA = wbuf[1]; 
        g_rf_param.rfcfgregB = wbuf[2];
    }
    /*
    //------------------
    n = 0xFF;
    n = SpiFlashReadByteData(MODULATION_DEPTH_ADDR);
    if((n == wbuf[0]) && (n<=0x1f))
    {
        return 0;
    }
    else
    {
        return 1;
    }
    */
    if(dev_rf_write_param() < 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
u8 dev_pcd_read_param(u8 *rbuf, u8 *rlen)
{
    //u8 m,n;
    
    *rlen = 0;
    #if 0
    //调制深度
    n = 0xff;
    n = SpiFlashReadByteData(MODULATION_DEPTH_ADDR);
    /*if(n != modulation_depth)
    {
        return 1;
    } */
    if(n>0x1f)
    {
        return 2;       //参数错误
    } 
    #endif
    rbuf[0] = g_rf_param.rfmodgsp;//n;
    rbuf[1] = g_rf_param.rfcfgregA;//rfcfgreg_valueA;
    rbuf[2] = g_rf_param.rfcfgregB;//rfcfgreg_valueB;
    *rlen = 3; 
    return 0;
}
#endif

s32 dev_pcd_readallparam(u8 *rbuf, u8 rlen)
{
    if(rlen<(RF_PARAM_LEN_MAX-2))
    {
        memcpy(rbuf, (u8*)&g_rf_param.rfmodgsp, rlen);
        return 0;
    }
    return -1;
}
s32 dev_pcd_writeallparam(u8 *wbuf, u8 wlen)
{
    if(wlen < (RF_PARAM_LEN_MAX-2))
    {
        memcpy((u8*)&g_rf_param.rfmodgsp, wbuf, wlen);
        return dev_rf_write_param();
    }
        
    return -1;
}

void dev_pcd_export_init(void)
{
    
    if(PCD_DRIVER_TYPE_PN512 == g_pcd_driver_type)
    {
        dev_pn512_export_init();
    }
    else if(PCD_DRIVER_TYPE_MFRC663 == g_pcd_driver_type)
    {
    }    
}
void dev_pcd_export_deinit(void)
{
    
    if(PCD_DRIVER_TYPE_PN512 == g_pcd_driver_type)
    {
        dev_pn512_export_deinit();
    }
    else if(PCD_DRIVER_TYPE_MFRC663 == g_pcd_driver_type)
    {
    }    
}
void dev_pcd_export_ctl(u8 num, u8 flg)
{    
    dev_pcd_open();
    if(PCD_DRIVER_TYPE_PN512 == g_pcd_driver_type)
    {
        dev_pn512_export_ctl(num, flg);
    }
    else if(PCD_DRIVER_TYPE_MFRC663 == g_pcd_driver_type)
    {
    }
    
}
void dev_pcd_reset(s32 _ms)
{
    
    if(PCD_DRIVER_TYPE_PN512 == g_pcd_driver_type)
    {
        dev_pn512_reset(_ms);
    }
    else if(PCD_DRIVER_TYPE_MFRC663 == g_pcd_driver_type)
    {
        dev_rc663_reset(_ms);
    }
    rf_delayms(_ms);
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
s32 dev_pcd_ctl_crc(u8 mask, u8 mod)
{
    if(PCD_DRIVER_TYPE_PN512 == g_pcd_driver_type)
    {
        return dev_pn512_ctl_crc( mask, mod);
    }
    else if(PCD_DRIVER_TYPE_MFRC663 == g_pcd_driver_type)
    {
        return dev_rc663_ctl_crc( mask, mod);
    }
    else
    {
        return -1;
    }
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
 ** Created By:     彭学斌 2009-7-2 9:43:03           
 ** Remarks:                                                                                 
*******************************************************************************/
s32 dev_pcd_select_type(u8 type)
{
#ifdef PCD_DEBUG   
    Uart_Printf("dev_pcd_select_type:%x\r\n\r\n", type);  
#endif
    if(PCD_DRIVER_TYPE_PN512 == g_pcd_driver_type)
    {
        return dev_pn512_select_type(type);
    }
    else if(PCD_DRIVER_TYPE_MFRC663 == g_pcd_driver_type)
    {
        return dev_rc663_select_type(type);
    }
    else
    {
        return -1;
    }
}

/*******************************************************************************/
s32 dev_pcd_set_para(u8 mode, u8 type)
{
    u8 tmp[10];
    s32 status=MI_OK;
    memset(tmp, 0, 10);
    checkparam.mode = mode;
    checkparam.picctype = type;
    if(checkparam.mode!=0)
    {
      #if 0  
        if(PCD_DRIVER_TYPE_PN512 == g_pcd_driver_type)
        {
            status = dev_pn512_select_type(checkparam.picctype);
        }
        else if(PCD_DRIVER_TYPE_MFRC663 == g_pcd_driver_type)
        {
            status = dev_rc663_select_type(checkparam.picctype);
        }
        else
        {
            status = -1;
        }
      #endif
        status = dev_pcd_select_type(checkparam.picctype);
//        rf_delayms(5);
    }
//uart_printf("set_para:mode=%x,type=%x\r\n", checkparam.mode,checkparam.picctype);
    return status;
}

//***********************打开PN512芯片*****************************/s
s32  dev_pcd_softpoweron(void)
{
    if(PCD_DRIVER_TYPE_PN512 == g_pcd_driver_type)
    {
        return dev_pn512_softpoweron();
    }
    else if(PCD_DRIVER_TYPE_MFRC663 == g_pcd_driver_type)
    {
        return dev_rc663_softpoweron();
    }
    return -1;   
}
s32 dev_pcd_open(void)
{
    if(0 > g_dev_pcd_fd)
    {         
        
        dev_pcd_init();
        if(dev_rf_support_judge())
        {
            dev_rf_5v_ctl(1);
            dev_pcd_softpoweron();
            dev_pcd_reset(5);
            dev_pcd_export_init();
            checkparam.mode = 0;          
            checkparam.picctype = PICC_TYPEA;
            dev_pcd_initparam();
            rf_loopback_init();
            g_dev_pcd_fd = 0;
            g_rf_morecard_cnt = 0;
            g_rf_lp_mod = 0;
        }
    }
    return g_dev_pcd_fd;
}

void dev_pcd_sleep_ctl(u8 flg)
{
    dev_rf_sleep_ctl(flg);
}
///////////////////////////////////////// 
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

void dev_pcd_set_timeout(u8 mode, u16 timer)
{
    if(PCD_DRIVER_TYPE_PN512 == g_pcd_driver_type)
    {
        dev_pn512_set_timeout(mode, timer); 
    }
    else if(PCD_DRIVER_TYPE_MFRC663 == g_pcd_driver_type)
    {
        dev_rc663_set_timeout(mode, timer); 
    }
    else
    {
    }
}

void dev_pcd_wait_etu(u16 etu)
{
    if(PCD_DRIVER_TYPE_PN512 == g_pcd_driver_type)
    {
        dev_pn512_wait_etu(etu);
    }
    else if(PCD_DRIVER_TYPE_MFRC663 == g_pcd_driver_type)
    {
        dev_rc663_wait_etu(etu);
    }
    else
    {
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
                                                                                              
 ** Created By:     彭学斌 2009-6-30 15:32:05           
 ** Remarks:                                                                                  
*******************************************************************************/
void dev_pcd_reset_mfinfo(MfCmdInfo *p_mfcmdinfo)
{
    p_mfcmdinfo->nBytesToSend = 0;
    p_mfcmdinfo->nBytesReceived = 0;
    p_mfcmdinfo->nBitsReceived = 0;
    p_mfcmdinfo->nReceivedmax= 0;
    p_mfcmdinfo->timeout = 0;
    p_mfcmdinfo->timeoutmode = 0;
    p_mfcmdinfo->timeoutcnt = 0;                //默认为0
    p_mfcmdinfo->cmd = 0;
    p_mfcmdinfo->status = 0;
    p_mfcmdinfo->ErrorReg = 0;
    p_mfcmdinfo->nmifareflg = 0;
}
u8 const rf_cmdnum_tab[RF_CMDNUM_MAX][2] = {
                                {JCMD_TRANSCEIVE,   J663CMD_Transceive},
                                {JCMD_AUTHENT,      J663CMD_MFAuthent},
                                 };
s8 dev_pcd_execute_cmd(u8 cmdnum, MfCmdInfo *p_mfcmdinfo, u8 *wbuf, u8 *rbuf)
{
//    s8 ret = 0;
    if(cmdnum >= RF_CMDNUM_MAX)
    {
        return -1;
    }
    if(PCD_DRIVER_TYPE_PN512 == g_pcd_driver_type)
    {
        p_mfcmdinfo->cmd = rf_cmdnum_tab[cmdnum][0];
        return dev_pn512_execute_cmd(p_mfcmdinfo, wbuf, rbuf);
        
    }
    else if(PCD_DRIVER_TYPE_MFRC663 == g_pcd_driver_type)
    {
        p_mfcmdinfo->cmd = rf_cmdnum_tab[cmdnum][1];
        return dev_rc663_execute_cmd(p_mfcmdinfo, wbuf, rbuf);
    }
    else
    {
        return -1;
    }
}
S8 dev_pcd_execute_cmd_send(u8 *wbuf, u32 wlen)
{
//    s8 ret = 0;
    if(PCD_DRIVER_TYPE_PN512 == g_pcd_driver_type)
    {
        return dev_hm1608_execute_cmd_send(wbuf, wlen);
        
    }
    else if(PCD_DRIVER_TYPE_MFRC663 == g_pcd_driver_type)
    {
    }
    else
    {
        return -1;
    }
}
/*******************************************************************************
*******************************************************************************/
s32 dev_pcd_mifare_auth(u8 key_type, u8 *key, u8 *snr, u8 block)
{
    if(PCD_DRIVER_TYPE_PN512 == g_pcd_driver_type)
    {
        return dev_pn512_mifare_auth(key_type, key, snr, block);
    }
    else if(PCD_DRIVER_TYPE_MFRC663 == g_pcd_driver_type)
    {
        return dev_rc663_mifare_auth(key_type, key, snr, block);
    }
    else
    {
        return -1;
    }
    
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
s8 dev_pcd_send_rats_piccA(u8 cid, u8 *rbuf, u16 *rlen)
{
    s8 status = MI_OK;
    u8 i,j; 
    u8 k;
    *rlen = 0;

    if(PCD_DRIVER_TYPE_NONE == g_pcd_driver_type)
    {
        return -1;
    }
    
    if(cid < PICC_CIDCNTMAX)
    {
        if(PCD_DRIVER_TYPE_PN512 == g_pcd_driver_type)
        {
            status = dev_pn512_send_rats_piccA(cid, rbuf, rlen);
        }
        else if(PCD_DRIVER_TYPE_MFRC663 == g_pcd_driver_type)
        {
            
            status = dev_rc663_send_rats_piccA(cid, rbuf, rlen);
        }
        else
        {
            return -1;
        }
        
    }
    else            //参数错
    { 
        status = MI_WRONG_PARAMETER_VALUE; 
    }
    if(MI_OK == status)
    {
        k = rbuf[0];
        if(k<2)
        {
            j = 0;
            PICCData[cid].FSCI = 0x02;
        }
        else
        {
            //T0   (格式字节)
            j = rbuf[1];
            if((j&0x0f)>0x08)PICCData[cid].FSCI = 0x08;
            else PICCData[cid].FSCI = j&0x0f;
        }
        i=2;
        //TA
        if(j&0x10)
        {
            PICCData[cid].TA = rbuf[i++];
        }
        else
        {
            PICCData[cid].TA = 0x80;
        }
        //TB
        if(j&0x20)
        {
            PICCData[cid].TB = rbuf[i];
            i++;
        }
        else
        {
            PICCData[cid].TB = 0x40;        //设置缺省值
            //PICCData[cid].FWI = 4;
        }
        //TC
        if(j&0x40)
        {
            PICCData[cid].TC = rbuf[i++];
        }
        else
        {
            PICCData[cid].TC = 2;
        }
        
        PICCData[cid].status = PICC_STATUS_PPSS;
        PICCData[cid].TxIPCB = 0x02;
        PICCData[cid].RxIPCB = 0x00;
        PICCData[cid].FWI = (PICCData[cid].TB>>4)&0x0F;
        
        if(PICCData[cid].FWI == 15)
        {
            PICCData[cid].FWI = 4;
        }
        PICCData[cid].SFGI = PICCData[cid].TB&0x0F;
       
        if(PICCData[cid].SFGI == 15)
        {
            PICCData[cid].SFGI = 0;
        }
        PICCData[cid].atqflg = 1;
    }
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
 ** Created By:     彭学斌 2009-7-31 18:25:07
 ** Remarks:       
*******************************************************************************/
s8 dev_pcd_attrib(u8 cid, u8 *pupi, u8 brTx, u8 brRx, u8 param3)
{
    s8 status = MI_OK; 
//    u8 tmp[16];
//    u8 tmp1[256];
//    u8 i;
//    MfCmdInfo mfcmdinfotmp;
    u32 fwt;

    //fwttmp = (0x01 << PICCData[cid].FWI);              //32 * (2**(TB>>4))etu  以32etu为单位  PCD_TIMER_MODE_nETU_1
    fwt = 32*(0x01 << PICCData[cid].FWI)+384+318;  //[(FWT+ ΔFWT) + tMIN,RETRANSMISSION(3ms)]    
    if(cid < PICC_CIDCNTMAX)
    {
        if(PCD_DRIVER_TYPE_PN512 == g_pcd_driver_type)
        {
            status = dev_pn512_attrib(cid, pupi, brTx, brRx, param3, fwt);
        }
        else if(PCD_DRIVER_TYPE_MFRC663 == g_pcd_driver_type)
        {
            
            status = dev_rc663_attrib(cid, pupi, brTx, brRx, param3, fwt);
        }
        else
        {
            return -1;
        }
        
    }
    else            //参数错
    { 
        status = MI_WRONG_PARAMETER_VALUE; 
    }
    return status; 
}

/********************激活PICC模块************************************/
//NOTE: For type A,ATS is returned,for type B card,nothing returned.
s32 dev_pcd_active_picc(u8 cid, u8 *rbuf, u16 *rlen)
{
//    u8 tmp[100];    
    u16 len;
    u8 i;
    s32 status = -1;
    *rlen = 0;

    if(PCD_DRIVER_TYPE_NONE == g_pcd_driver_type)
    {
        return -1;
    }
    //g_rf_test_flag = 1;   
    if(PICC_TYPEA == PICCData[cid].picctype)
    {       
//        for(i=0; i<3; i++)
        {
            //uart_printf("rats\r\n");
            //g_rf_emd_flg = 1;
            status = dev_pcd_send_rats_piccA(cid, rbuf, &len);
            //g_rf_emd_flg = 0;
#ifdef PCD_DEBUG   
            Uart_Printf("rats:status=%d\r\n", status);
#endif
            if(MI_COLLERR == status)
            { 
                //rf_delayms(4);//dev_delay_ms(6); 
            }
            else if(MI_OK != status)
            {
                //rf_delayms(2);//dev_delay_ms(6);
                dev_pcd_reset(6); 
                checkparam.checkcontinue=1;
                return status; 
            }
            else
            {
#ifdef PCD_DEBUG  
                PrintFormat(rbuf, len);
#endif
                PICCData[cid].status = PICC_STATUS_PPSS;
                *rlen = len; 
                //break;
            }      
        }
    } 
    else if(PICC_TYPEB == PICCData[cid].picctype)
    {
        //g_rf_emd_flg = 1;
        status = dev_pcd_attrib(cid, PICCData[cid].pupi, 0x00, 0x00, 0x01);  
        //g_rf_emd_flg = 0;
#ifdef PCD_DEBUG   
        Uart_Printf("pcd_Attrib:status=%x\r\n", status);
#endif  
        if(MI_OK == status)
        {

            PICCData[cid].TxIPCB = 0x02; 
            PICCData[cid].RxIPCB = PICCData[cid].TxIPCB&0x01;
            PICCData[cid].status = PICC_STATUS_PPSS;

            PICCData[cid].atqflg = 1;
        }
        else
        {
            rf_delayms(6);//dev_delay_ms(6);
            dev_pcd_reset(6); 
            checkparam.checkcontinue=1;
        }
    }
    else
    {
        return MI_WRONG_PARAMETER_VALUE;
    }

    return status;
}

void dev_pcd_wait_netu(u32 netu)
{
    if(PCD_DRIVER_TYPE_PN512 == g_pcd_driver_type)
    {
        dev_pn512_wait_etu(netu);
    }
    else if(PCD_DRIVER_TYPE_MFRC663 == g_pcd_driver_type)
    {
        dev_rc663_wait_etu(netu);
    }
    else
    {
    }
}
/*******************************************************************************
 ** Descriotions:    
 ** parameters:      
                    
 ** Returned value:  0: 成功
                    其它:失败 
 ** Created By:     彭学斌 2009-8-22 23:48:56
 ** Remarks:       
*******************************************************************************/
s8 __pcd_exchange_data_piccA(u8 *wbuf, u16 wlen, u8 *rbuf, u16 *rlen, u32 etu, u8 wtx)
{
    s8 status;
    
    if(PCD_DRIVER_TYPE_PN512 == g_pcd_driver_type)
    {
        status = dev_pn512_exchange_piccA(wbuf, wlen, rbuf, rlen, etu, wtx);
    }
    else if(PCD_DRIVER_TYPE_MFRC663 == g_pcd_driver_type)
    {
        status = dev_rc663_exchange_piccA(wbuf, wlen, rbuf, rlen, etu, wtx);        
    }
    else
    {
        status = MI_WRONG_PARAMETER_VALUE;
    }
    return status;
}
///////////////////////////////////////////////////////////////////////////////
////TYPEB  
s8 dev_pcd_exchange_data_piccA(u8 cid, u8 *wbuf, u16 wlen, u8 *rbuf, u16 *rlen)
{
    s8 status = MI_OK;
    u8 i,n;
    u8 cidvalid=0, nadvalid=0;
    u16 y,z,a;
    //u8 txtmp[300];
    u8 *txtmp = NULL;
    u16 txtmplen;
    //u8 rxtmp[300];
    u8 *rxtmp = NULL;
   //u8 rxtmp[50];
    u16  rxtmplen;
    u8 step;
    u8 flag;
    s8 errcnt;
    u8 errtype;
    u8 pcbnext;      //用于确定发送R块的
    u8 stext;       //保存S块内容
    u8 TxIPCBflag;
    u8 IPCBbak;
    u8 wtxtmp; 
    //u32 fwttmp;
    u32 fwttmp;
    s8 Iretrancnt;
    s8 Sblockcnt;
    //pengxuebin, 20110526

    if(PCD_DRIVER_TYPE_NONE == g_pcd_driver_type)
    {
        return -1;
    }
#if 0
    txtmp = (u8 *)k_malloc(300);
    if(NULL == txtmp)
    {
        return MI_OTHERERR;
    }

    rxtmp = (u8 *)k_malloc(300);
    if(NULL == rxtmp)
    {
        k_free(txtmp);
        return MI_OTHERERR;
    }
#endif
    txtmp = (u8*)k_malloc(600);
    if(NULL == txtmp)
    {
        return MI_OTHERERR;
    }
    rxtmp = &txtmp[300];
    
  #if 1
    if(PICCData[cid].FWI>12) 
    {
        PICCData[cid].FWI = 12;
    }
  #endif  
    fwttmp = (0x01 << PICCData[cid].FWI);              //32 * (2**(TB>>4))etu  以32etu为单位  PCD_TIMER_MODE_nETU_1
                                                        //(32+3) * (2**(TB>>4))etu  以35etu为单位  PCD_TIMER_MODE_nETU_1

    #ifdef PCD_DEBUG 
        dev_debug_printf("exchange_data_piccA:wlen=%d\r\n", wlen);
        PrintFormat(wbuf, wlen);
        //dev_pn512_read_reg(JREG_WATERLEVEL, &i);
        //
        Uart_Printf("JREG_WATERLEVEL=%x\r\n", i);   
    #endif     
    *rlen = 0;
    if(cid >= PICC_CIDCNTMAX)
    {                                       //参数错
        //return MI_WRONG_PARAMETER_VALUE;
        status = MI_WRONG_PARAMETER_VALUE;
        goto EXIT;
    }
    if(PICC_STATUS_PPSS != PICCData[cid].status)
    {                                       //条件不满足
        //return MI_WRONG_PARAMETER_VALUE;
        status = MI_WRONG_PARAMETER_VALUE;
        goto EXIT;
    }
    y = PCDFSCIToFSCTab[(PICCData[cid].FSCI)&0x0f];
    if(0 == y)
    {                                       //参数错
        //return MI_WRONG_PARAMETER_VALUE;
        status = MI_WRONG_PARAMETER_VALUE;
        goto EXIT;
    }
    z=0;
    //--------------------------------------
    if(PICCData[cid].TxIPCB&0x08)cidvalid = 1;
    if(PICCData[cid].TxIPCB&0x04)nadvalid = 1;
    flag = 1;
    step = 0;
    errcnt = 3;
    Iretrancnt=3;
    Sblockcnt = 3;
    errtype = PCD_PHASE_NO_ERR;        //没有错误
    #ifdef PCD_DEBUG
        Uart_Printf("exchange_data_piccA:1111\r\n");
    #endif    
    while(flag)
    {
        wtxtmp = 0;
        switch(step)
        {
        case 0:                 //发送I块
        {
            if((wlen+3+cidvalid+nadvalid) <= (y+z))
            {
                txtmp[0] = PICCData[cid].TxIPCB;
                a = wlen - z;
            }
            else
            {
                txtmp[0] = PICCData[cid].TxIPCB|0x10;
                a = y-3-cidvalid-nadvalid;
            }
            TxIPCBflag = 1;
            i = 1;
            if(1 == cidvalid)
            {
                txtmp[i++] = cid;
            }
            if(1 == nadvalid)
            {
                txtmp[i++] = 0x00;
            }
            memcpy(&txtmp[i], &wbuf[z], a);
            txtmplen = i+a;
            IPCBbak = txtmp[0];                 //备份I块(PCB)
            Sblockcnt = 3;
        }
            break;
        case 1:                 //发送R块
        {
            txtmp[0] = pcbnext;
            i = 1;
            if(1 == cidvalid)
            {
                txtmp[i++] = cid;
            }
            txtmplen = i;
        }
            break;
        case 2:                 //发送S块
        {
            txtmp[0] = pcbnext;
            i = 1;
            if(1 == cidvalid)
            {
                txtmp[i++] = cid;
            }
            txtmp[i++] = stext;
            txtmplen = i;
            if((txtmp[0]&0x30) == 0x30)       //发送WTX响应块
            {
                if(stext>59)wtxtmp = 58;
                else if(stext>0)wtxtmp = stext-1;
                else wtxtmp = 0;
            }
        }
            break;
        default:                //错误
            flag = 0;        
            break;                
        }
        if(0 == flag)break;   
        //----收发数据--------------------------
        rxtmplen = 300;
        status = __pcd_exchange_data_piccA(txtmp, txtmplen, rxtmp, &rxtmplen, fwttmp, wtxtmp);      
        //解析数据   
        if(MI_OK == status)
        {
            #ifdef PCD_DEBUG
            Uart_Printf("111111:%x\r\n", rxtmplen);
            PrintFormat(rxtmp, rxtmplen);  
            #endif          
            switch(rxtmp[0]&0xc2)
            {
                case 0x02:              //收到I块
                {
                    i = 1;
                    if(rxtmp[0]&0x08)
                    {
                        //                        i++;
                        //CID跟随
                        errtype = PCD_PHASE_PROTOCOL_ERR;
                        step = 7;
                        errcnt = 1;
                        break;
                    }   
                    if(rxtmp[0]&0x04)
                    {
                        //                        i++;
                        //NAK跟随
                        errtype = PCD_PHASE_PROTOCOL_ERR;
                        step = 7;
                        errcnt = 1;
                        break;
                    }
                    if(rxtmplen>(256-2))
                    {   //I块长度大于FSD
                        errtype = PCD_PHASE_PROTOCOL_ERR;
                        step = 7;
                        errcnt = 1;
                        break;
                    }
                    //------------------------
                    if(rxtmp[0]&0x20)
                    {
                        #ifdef PCD_DEBUG  
                        Uart_Printf("1PCD_PHASE_PCB_ERR"); 
                        #endif 
                        errtype = PCD_PHASE_PROTOCOL_ERR;
                        step = 7;
                        errcnt = 1;
                        break;
                    }
                    n = rxtmp[0]^PICCData[cid].RxIPCB;
                    if(0 != (n&0x01))
                    {
                        #ifdef PCD_DEBUG  
                        Uart_Printf("2PCD_PHASE_BLOCKNUM_ERR:RxIPCD=%x\r\n", PICCData[cid].RxIPCB);  
                        #endif
                        errtype = PCD_PHASE_PROTOCOL_ERR;
                        step = 7;
                        errcnt = 1;
                        
                        break;
                    }
                    if((1==cidvalid) && ((rxtmp[1]&0x3f) != cid))
                    {                                       //不会执行
                      #ifdef PCD_DEBUG            
                        Uart_Printf("3PCD_PHASE_CID_ERR:cidvalid=%x\r\n", cidvalid);  
                      #endif
                        if(PCD_PHASE_CID_ERR != errtype)
                        {
                            errtype = PCD_PHASE_CID_ERR;              //CID错
                            errcnt = 3;
                        }
                        break;
                    }
                    if(rxtmplen < i)
                    {                                      //不会执行
                        #ifdef PCD_DEBUG  
                        Uart_Printf("4PCD_PHASE_PARA_ERR\r\n" );  
                        #endif
                        if(PCD_PHASE_PARA_ERR != errtype)
                        {
                            errtype = PCD_PHASE_PARA_ERR;             //数据长度错
                            errcnt = 3;
                        }
                        break;
                    }
                    //成功接收I块
                    errcnt = 3;
                    errtype = PCD_PHASE_NO_ERR;                //没有错误
                
                    TxIPCBflag = 0;  
                    //改为锁定当前序号
                    PICCData[cid].TxIPCB = (rxtmp[0]&0x03)^0x01; //20090825_1 
                    PICCData[cid].RxIPCB ^= 0x01;
                    memcpy(&rbuf[*rlen], rxtmp+i, rxtmplen-i);
                    *rlen += (rxtmplen-i);
                    if(0 == (rxtmp[0]&0x10))      
                    {                           //没有链接
                        flag = 0;               //接收完成，退出循环
                    }
                    else
                    {                           //有链接,发送R(ack)块
                        //rf_delayms(2);//dev_delay_ms(2);         //6780clock,pengxuebin,20150331
                        
                        pcbnext = 0xa2;
                        if(PICCData[cid].RxIPCB&0x01)
                        {
                            pcbnext |= 0x01;
                        }
                        step = 1;               //需要发送R块
                    }
         
                }
                break;
                //===============================
                case 0x82:              //收到R块
                {
                    if(rxtmp[0]&0x08)
                    {
                        //CID跟随
                        errtype = PCD_PHASE_PROTOCOL_ERR;
                        step = 7;
                        errcnt = 1;
                        break;
                    }   
                    if(rxtmp[0]&0x04)
                    {
                        //NAK跟随
                        errtype = PCD_PHASE_PROTOCOL_ERR;
                        step = 7;
                        errcnt = 1;
                        break;
                    }
                    if(0==(rxtmp[0]&0x20))
                    {
                        errtype = PCD_PHASE_PROTOCOL_ERR;
                        step = 7;
                        errcnt = 1;
                        break; 
                    }
                    if(0 == (rxtmp[0]&0x10))
                    {                           //R(ACK)
                        if(step==0x00)
                        {                       //说明上一块是I块
                            
                            if(0 == (txtmp[0]&0x10))
                            {   
                                //说明上一块没有连接
                                if(0 == ((PICCData[cid].RxIPCB^rxtmp[0])&0x01))
                                { 
                                    errtype = PCD_PHASE_PROTOCOL_ERR;
                                    step = 7;
                                    errcnt = 1;
                                    break;
                                }
                            }
                            if(1==TxIPCBflag)
                            {
                                TxIPCBflag = 0;
                                PICCData[cid].TxIPCB ^= 0x01;     //修改TxIPCB序号
                            }  
                            PICCData[cid].RxIPCB ^= 0x01;
                            if(0 != (txtmp[0]&0x10))
                            {                               //说明上一块有链接，需要发送下一块
                                z += a;
                                errcnt = 3;
                                errtype = PCD_PHASE_NO_ERR;     //没有错误
                                step = 0;                       //下一步发送I块
                                n = 0;
                                Iretrancnt = 3;
                                nadvalid = 0;                   //发链接块时不需要NAD
                            }
                            else                                //
                            {
                                if(PCD_PHASE_PROTOCOL_ERR != errtype)
                                {
                                    errtype = PCD_PHASE_PROTOCOL_ERR;          //协议错 
                                    errcnt = 3;
                                }
                                break;
                            }
                        }
                        else if(step == 0x01)
                        {                     
                            if(1 == TxIPCBflag)
                            {
                                if(0 != (IPCBbak&0x10))
                                {                //说明上一个I块有链接
                                    if(0 != ((rxtmp[0]^(PICCData[cid].TxIPCB))&0x01))
                                    {           //重传上一块 
                                        Iretrancnt--; 
                                        if(Iretrancnt>0)
                                        {    
                                            errtype = PCD_PHASE_NO_ERR;     //没有错误 
                                            errcnt = 3;
                                        }
                                        else
                                        {
                                            errtype = PCD_PHASE_PROTOCOL_ERR;
                                            step = 7;
                                            errcnt = 1;
                                            break;
                                        }
                                    }
                                    else  //发送下一个链接块
                                    {
                                        TxIPCBflag = 0;
                                        PICCData[cid].TxIPCB ^= 0x01;       //修改TxIPCB
                                        z += a;
                                        errcnt = 3;
                                        errtype = PCD_PHASE_NO_ERR;     //没有错误
                                        Iretrancnt = 3;       
                                    }
                                    step = 0;                       //下一步发送I块
                                    nadvalid = 0;                   //发链接块时不需要NAD
                                    break;
                                }
                                else
                                {       //重发上一块

                                    //if(0 != ((rxtmp[0]^(PICCData[cid].TxIPCB))&0x01)) 
                                    if(0 == ((PICCData[cid].RxIPCB^rxtmp[0])&0x01)) 
                                    {                       //收到的R(ACK)序号不等于上一块收到的I块序号
                                        errtype = PCD_PHASE_PROTOCOL_ERR;
                                        step = 7;
                                        errcnt = 1;
                                        break;
                                    }
                                    else
                                    {
                                        if(PCD_PHASE_I_RETRANS != errtype)
                                        {
                                            errtype = PCD_PHASE_I_RETRANS;
                                            errcnt = 3;
                                        }
                                        //errcnt--;
                                        Iretrancnt--;
     //                                   if((errcnt>0) 
     //                                   { 
                                        if(Iretrancnt>0)
                                        {    
                                            step = 0;
                                            break;
                                        }
                                        else
                                        {
                                            errtype = PCD_PHASE_PROTOCOL_ERR;
                                            step = 7;
                                            errcnt = 1;
                                            break;
                                        }
                                    }
                                }
                            }
                            else        //处于接收I块状态
                            {           
                                if(0 == ((PICCData[cid].RxIPCB^rxtmp[0])&0x01))
                                {       //接收R块序列号不等于PICC最后发送的I块序号
                                    errtype = PCD_PHASE_PROTOCOL_ERR;
                                    step = 7;
                                    errcnt = 1;
                                    break;
                                }
                                
                            }
                            //----有错误------------- 
                            if(PCD_PHASE_PROTOCOL_ERR != errtype)
                            {
                                errtype = PCD_PHASE_PROTOCOL_ERR;          //协议错 
                                errcnt = 3;
                            }
                            n = 0x07;                               //R块错
                            break;
                        }
                        else //if(step == 0x02)
                        {
                            if(0x30 == (txtmp[0]&0x30))
                            {   //说明上一块是WTX块
                                if(1==TxIPCBflag)
                                {
                                    TxIPCBflag = 0;
                                    PICCData[cid].TxIPCB ^= 0x01; 
                                }
                                //改为锁定当前序号
                                //PICCData[cid].TxIPCB = (rxtmp[0]&0x03);//^0x01; //20090825_1 
                                PICCData[cid].RxIPCB ^= 0x01;
                                if(0 != (IPCBbak&0x10))
                                {
                                    z += a;
                                    errcnt = 3;
                                    errtype = PCD_PHASE_NO_ERR;     //没有错误
                                    step = 0;                       //下一步发送I块
                                    n = 0;
                                    nadvalid = 0;                   //发链接块时不需要NAD
                                    break;
                                }
                                else  //协议错
                                {
                                    if(PCD_PHASE_PROTOCOL_ERR != errtype)
                                    {
                                        errtype = PCD_PHASE_PROTOCOL_ERR;          //协议错 
                                        errcnt = 3;
                                    }
                                }
                                
                            }
//                            if(0==(txtmp[0]&0x30))
//                            {                                       //说明上一块是S(DESELECT)块 
//                            }
                            //----针对S块回R(ACK)均有错误-------------
                            if(PCD_PHASE_PROTOCOL_ERR != errtype)
                            {
                                errtype = PCD_PHASE_PROTOCOL_ERR;          //协议错 
                                errcnt = 3;
                            }
//                            n = 0x07;                               //R块错
                            break;
                        }     
                    }
                    else                    //R(nak)块
                    {
                        //当做协议错 
//                        if(PCD_PHASE_PROTOCOL_ERR != errtype)
//                        {
//                            errtype = PCD_PHASE_PROTOCOL_ERR;          //协议错 
//                            errcnt = 3;
//                        }
                        errtype = PCD_PHASE_PROTOCOL_ERR;
                        step = 7;
                        errcnt = 1;
                        break;
                    }
                }
                    break;
                //===============================
                case 0xc2:                          //收到S块
                {
                    if(0x30==(rxtmp[0]&0x30))
                    {
                        if(0 != (rxtmp[1]&0xc0))
                        {
                            errtype = PCD_PHASE_PROTOCOL_ERR;
                            step = 7;
                            errcnt = 1;
                            break;
                        }
                        i = rxtmp[1]&0x3f;
                        if(0 == i)
                        {   //协议错误
                            errtype = PCD_PHASE_PROTOCOL_ERR;
                            step = 7;
                            errcnt = 1;
                            break;
                        }
                        else if(i > 59)
                        {   //协议错误
                            errtype = PCD_PHASE_PROTOCOL_ERR;
                            step = 7;
                            errcnt = 1;
                            break;
                        }
                        else
                        {
                            stext = i;
                        }
                        //rf_delayms(2);
                        pcbnext = 0xf2;
                        step = 2;
                        errtype = PCD_PHASE_NO_ERR;
                    }
                    else
                    {
                        status = MI_FRAMINGERR;       //帧错误
                        flag = 0;           //退出循环
                        break;
                    }
                }
                    break;
                //===============================  
                default:
                    //其它错误
                    //NAK跟随
                    errtype = PCD_PHASE_PROTOCOL_ERR;
                    step = 7;
                    errcnt = 1;
                    break;
            }
            /////////////////////////////////////
            /////////////////////////////////////
//            if(PCD_PHASE_NO_ERR != errtype)
            //对于协议错误包统一处理
            if(PCD_PHASE_PROTOCOL_ERR == errtype)
            {
                errcnt--;
                if(errcnt>0)
                { 
                    //发送NACK
                    step = 1;   //发送R(NAK)块
                    pcbnext = 0xb2|(rxtmp[0]&0x01);         //20090826
                    status = MI_OK; 
                }
                else
                {
                    flag = 0;
                    status = MI_FRAMINGERR;       //帧错误
                    step = 7;
                    break;
                }
            }
            
        }
        else if((MI_PARITYERR == status)
              ||(MI_CRCERR == status)
              ||(MI_ACK_SUPPOSED == status)
              ||(MI_PROTOCOL_ERROR == status)
              ||(MI_NOTAGERR == status)
              ||(MI_OTHERERR == status) //超时
              ||(MI_COLLERR == status) 
              ||(MI_INITERR == status)) 
        {         
            if(PCD_PHASE_PROTOCOL_ERR != errtype)
            {
                errtype = PCD_PHASE_PROTOCOL_ERR;          //协议错 
                errcnt = 3;
            }  
            errcnt--;
            if(0 != errcnt)
            {
              #if 0  
                if(MI_NOTAGERR != status)
                { 
                    if(rxtmplen<2)
                    {
                        dev_pcd_wait_netu(fwttmp*35 + 384+10);
                    }
                }
              #endif  
                step = 1;   //发送R(NAK)块
                if(0xa2 == (txtmp[0]&0xf6))
                {           //如果上一块是R(ACK)块，则重发R(ACK)块
                    pcbnext = 0xa2|(txtmp[0]&0x01); 
                }
                else if(0xc0 == ((txtmp[0]&0xc0)))//如果上一块是S块，则以I块的序号发送R(NAK)
                {
                    Sblockcnt--;
                    if(Sblockcnt>0){
                        pcbnext = 0xb2|(IPCBbak&0x01); 
                    }
                    else
                    {
                        step = 7;
                        break;
                    }
                }
                else
                {
                    pcbnext = 0xb2|(txtmp[0]&0x01); 
                }
                status = MI_OK;
            }
            else{
                break;              //暂时这样处理
            } 
        }
        else
        {                       //其它错误
            break;
        }
    }
    
EXIT:    
    k_free(txtmp);
//    k_free(rxtmp);
    #ifdef PCD_DEBUG  
      dev_debug_printf("return:status=%x, rlen=%d\r\n", status, *rlen); 
      PrintFormat(rbuf, *rlen);
    #endif
    return status;
}  
//////////////////////////////////////////////////////////
s8 dev_pcd_exchange_data(u8 cid, u8 *wbuf, u16 wlen, u8 *rbuf, u16 *rlen)
{
    *rlen = 0;
    
    if(PCD_DRIVER_TYPE_NONE == g_pcd_driver_type)
    {
        return -1;
    }
    if(cid < PICC_CIDCNTMAX)
    {
        if((1 == PICCData[cid].atqflg) && (PICCData[cid].SFGI!=0))
        {
            if(PICCData[cid].SFGI>=15)
            {
                PICCData[cid].SFGI=14;
            }
            dev_pcd_wait_netu((0x01 << PICCData[cid].SFGI)*35);
            PICCData[cid].atqflg = 0;
        }
        return dev_pcd_exchange_data_piccA(cid, wbuf, wlen, rbuf, rlen);
    }
    return MI_WRONG_PARAMETER_VALUE;//参数错
}

////////////////////////////////////////////////////////////////////////////////
 /***********************************************************
  Function: long PowerDownExample(phcsBflRegCtl_t* rc_reg_ctl)
  Author:    zhaoying
  Version : 1.0
  Date:9/10/2008
  Input: void
  Output:void
  Return:void
  Description: This example shows how to set the PN51x into soft power down mode.
***********************************************************/
s32  dev_pcd_softpowerdown(void)
{
    if(PCD_DRIVER_TYPE_PN512 == g_pcd_driver_type)
    {
        return dev_pn512_softpowerdown();
    }
    else if(PCD_DRIVER_TYPE_MFRC663 == g_pcd_driver_type)
    {
        return dev_rc663_softpowerdown();
    }
    return -1;   
}


s32 dev_pcd_close(void)
{ 
//    dev_pcd_export_deinit();
    dev_pcd_softpowerdown();
    checkparam.mode = 0;          
    checkparam.picctype = PICC_TYPEA;
    checkparam.checkcontinue = 0;
    checkparam.checkremovflg = 0;
    
    dev_rf_5v_ctl(0);
    g_dev_pcd_fd = -1;
    return 0;
}
/////////////////////////////////////////
s32 dev_pcd_ctrl_carrier(u8 mode)
{
    s32 ret;
    if(mode)            //打开载波
    {
        checkparam.mode = 0;          
        checkparam.picctype = PICC_TYPEA;
        checkparam.checkcontinue = 0;
        checkparam.checkremovflg = 0;
        rf_loopback_init();
        dev_pcd_reset(5);
        ret = dev_pcd_select_type(PICC_TYPEA);
//        rf_delayms(5);
        return ret;
    }
    else                //关闭载波
    {
        dev_pcd_reset(0);
    }
    return 0;
}
////////////////////////////////////////
s32 dev_pcd_loopback_reset(u16 timers)
{
    s32 i;
    
    if(PCD_DRIVER_TYPE_NONE == g_pcd_driver_type)
    {
        return -1;
    }
    for(i=0; i<timers; i++)
    {
        dev_pcd_softpowerdown();
        rf_delayms(2);
        dev_pcd_reset(5);
        dev_pcd_select_type(PICC_TYPEA);
        rf_delayms(100);
    }
    
    dev_pcd_softpowerdown();
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
s8 dev_pcd_send_reqa(u8 req_code, u8 *atq)
{
    if(PCD_DRIVER_TYPE_PN512 == g_pcd_driver_type)
    {
        return dev_pn512_send_reqa(req_code, atq);
    }
    else if(PCD_DRIVER_TYPE_MFRC663 == g_pcd_driver_type)
    {
        return dev_rc663_send_reqa(req_code, atq);
    }
    else
    {
        return -1;
    }
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
s8 dev_pcd_halt_piccA(void)
{
    if(PCD_DRIVER_TYPE_PN512 == g_pcd_driver_type)
    {
        return dev_pn512_halt_piccA();
    }
    else if(PCD_DRIVER_TYPE_MFRC663 == g_pcd_driver_type)
    {
        return dev_rc663_halt_piccA();
    }
    else
    {
        return -1;
    }
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
s8 dev_pcd_req_piccB(u8 afi, u8 N, u8 req_code, u8 *atqB, u8 *atqBlen)
{  
    if(PCD_DRIVER_TYPE_PN512 == g_pcd_driver_type)
    {
        return dev_pn512_req_piccB(afi, N, req_code, atqB, atqBlen);
    }
    else if(PCD_DRIVER_TYPE_MFRC663 == g_pcd_driver_type)
    {
        return dev_rc663_req_piccB(afi, N, req_code, atqB, atqBlen);
    }
    else
    {
        return -1;
    }
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
s8 dev_pcd_selectanticoll_picc(u8 grade, u8 grademax, u8 bitcount, u8 *snr, u8 *sak)
{
    if(PCD_DRIVER_TYPE_PN512 == g_pcd_driver_type)
    {
        return dev_pn512_selectanticoll_picc(grade, grademax, bitcount, snr, sak);
    
    }
    else if(PCD_DRIVER_TYPE_MFRC663 == g_pcd_driver_type)
    {
        return dev_rc663_selectanticoll_picc(grade, grademax, bitcount, snr, sak);
    
    }
    else
    {
        return -1;
    }
}
/********************************************
    //查询是否有A卡
********************************************/
s32 dev_pcd_check_typeApresent(void)
{
    s32 status;
    u8 tmp[50];

//test_set1();    
//dev_misc_led_green(1);
    dev_pcd_select_type(PICC_TYPEA);
    rf_delayms(5);
    status = dev_pcd_send_reqa(0x52, tmp);   
    if((MI_OK == status) || (MI_COLLERR == status)
      || (MI_PARITYERR == status) || (MI_CRCERR == status)
      || (MI_BYTECOUNTERR == status) || (MI_PROTOCOL_ERROR == status))
    {           //说明有A卡 
        memcpy(mifare_ack,tmp,2);
        return 1;               
    }
    else
    {
        return 0;
    }
}
/********************************************/
    //查询是否有A卡
/********************************************/
s32 dev_pcd_check_typeAAnticollision(void)
{
    s32 status;
    u8 i;
    u8 j;
    u8 tmp[64];
    u8 cidtmp = 0;
    u8 tmpbuf;
    
    dev_pcd_select_type(PICC_TYPEA);       //设置TYPE A场域
    rf_delayms(5);
    //发送WUPA
    for(i=0; i<3; i++)
    {     
        status = dev_pcd_send_reqa(0x52, tmp);       
        if(MI_NOTAGERR == status)
        {
            if(i<2)
            {
                rf_delayms(6);
            }
        }
        else
        {
            break;
        }
    }
    if(MI_OK != status)return status;
    if(0 != (tmp[1]&0xF0))
    {
        return MI_WRONG_VALUE;
    }
    rf_delayms(6);
    PICCData[cidtmp].MLastSelectedsnrLen = 0;
    j = 2;
    i = 0;
    for(i=0; i<=j; i++)
    {
        status = dev_pcd_selectanticoll_picc(i, j, 0, tmp, &tmpbuf);
        if(MI_OK != status)
        {         
            //msleep(5);
            break;
        }
        else
        {
            if((EMV_CASCADE_TAG == tmp[0])
             &&(i<j)) //串联比特设置不完整,进入下一级防冲突
            {
                memcpy(PICCData[cidtmp].MLastSelectedSnr+(i*3), &tmp[1], 3);
                PICCData[cidtmp].MLastSelectedsnrLen += 3; 
            }
            else
            {
                memcpy(PICCData[cidtmp].MLastSelectedSnr+(i*3), &tmp[0], 4);
                PICCData[cidtmp].MLastSelectedsnrLen += 4;
                PICCData[cidtmp].PiccSAK = tmpbuf;
                PICCData[cidtmp].picctype = PICC_TYPEA; // mifare 卡属于特殊A卡
               break;
            }
        }
    }
    return status;
}

/********************************************/
    //查询是否有B卡
/********************************************/
s32 dev_pcd_check_typeBpresent(void)
{
    s32 status;
    u8 tmp[50];
    u8 i;

    dev_pcd_select_type(PICC_TYPEB);
    rf_delayms(5);
    status = dev_pcd_req_piccB(0x00, 0, 0x08, tmp, &i);
    
    if((MI_OK == status) || (MI_COLLERR == status)
      || (MI_PARITYERR == status) || (MI_CRCERR == status)
      || (MI_BYTECOUNTERR == status) || (MI_BITCOUNTERR == status) 
      || (MI_PROTOCOL_ERROR == status)|| (MI_OTHERERR == status)) 
    {           //说明有B卡 
        return 1;               
    }
    else
    {
        return 0;
    }
}
s32 dev_pcd_check_typeBAnticollision(void)
{
    s32 status;
    u8 i;
    u8 j;
    u8 tmp[64];
    u8 cidtmp = 0;
    
    //给B卡上电 
    dev_pcd_select_type(PICC_TYPEB);       //设置TYPE B场域
    rf_delayms(5);
    //WUPB
    for(j=0; j<3; j++)
    {   
        status = dev_pcd_req_piccB(0x00, 0, 0x08, tmp, &i);
        if(MI_OK == status)
        {   //说明有B型卡     
            if((0x50 != tmp[0])||(i<11))
            {
                status = MI_READERR;    //读错
                break;
            }
            memcpy(PICCData[cidtmp].pupi, &tmp[1], 4);
            memcpy(PICCData[cidtmp].app_dat, &tmp[5], 4);
            memcpy(PICCData[cidtmp].prtcl_inf, &tmp[9], 2);
            if((tmp[10]&0x08)!=0)
            {//
                return MI_PROTOCOL_ERROR;
            }
            //<----
            if(i>12)
            {
                PICCData[cidtmp].prtcl_inf[2] = tmp[11];
                PICCData[cidtmp].prtcl_inf[3] = tmp[12];
            }
            else if(i==12)
            {
                PICCData[cidtmp].prtcl_inf[2] = tmp[11];
                PICCData[cidtmp].prtcl_inf[3]=0;
            }
            else
            {
                PICCData[cidtmp].prtcl_inf[2] = 0x40;   //FWI缺省值为4
                PICCData[cidtmp].prtcl_inf[3]=0;
            }
            PICCData[cidtmp].picctype = PICC_TYPEB;
           
            PICCData[cidtmp].TA = PICCData[cidtmp].prtcl_inf[0];    //比特速率      
            PICCData[cidtmp].FSCI = (PICCData[cidtmp].prtcl_inf[1]>>4)&0x0F;  //最大帧字节数
            if(PICCData[cidtmp].FSCI>8)PICCData[cidtmp].FSCI=8;
            PICCData[cidtmp].FWI = (PICCData[cidtmp].prtcl_inf[2]>>4)&0x0F;   //FWI   
            
            if(PICCData[cidtmp].FWI == 15)
            {
                PICCData[cidtmp].FWI = 4;
            }           
            PICCData[cidtmp].SFGI = (PICCData[cidtmp].prtcl_inf[3]>>4)&0x0F;  //缺省值为0
           
            if(PICCData[cidtmp].SFGI == 15)
            {
                PICCData[cidtmp].SFGI = 0;
            }
            memcpy(PICCData[cidtmp].AtsData, &tmp[0], i);
            PICCData[cidtmp].AtsLen = i;
            PICCData[cidtmp].picctype = PICC_TYPEB; 
            
            rf_delayms(6);
            break;
        }
        else if(MI_NOTAGERR == status)
        {
            if(j<2)
            {
                rf_delayms(6);
            }
        }
        else
        {
            break;
        }
    }
    return status;
}


void dev_pcd_get_piccparam(nPICCData* lpPICCData)
{
    memcpy((u8*)lpPICCData, (u8*)&PICCData[0], sizeof(nPICCData));
}


s32 dev_pcd_check_picc(u8 *cid, u8 *rbuf, u16 *rlen)
{
    s32 status;
    u8 i,j;
    u8 cidtmp = 0;
    
    
    *cid = 0;
    *rlen = 0;

    if(1 == checkparam.checkcontinue)
    {
        while(1)
        {
            //搜寻A卡
            if(0 != (checkparam.mode & RF_MODE_PICCA))
            {   //说明需要查询A卡
                //给A卡上电 
                if(0 != picctype_exist_flg[0])
                {
                    break;
                }
                else
                {    
                    if(1 == dev_pcd_check_typeApresent())
                    {           
                        picctype_exist_flg[0] = 1;  //PICC_TYPEA
                        
                        rf_delayms(5);//dev_delay_ms(5);
                        
                        //让A卡进入HALT状态
                        status = dev_pcd_halt_piccA();
                        if(MI_OK == status)
                        {
                            
                        }
                    }
                }
            }
            //搜寻B卡
            if(0 != (checkparam.mode&RF_MODE_PICCB))
            {//说明需要查询B卡
                //给B卡上电
                if(0 != picctype_exist_flg[1])
                {
                    break;
                }
                else
                {
                    if(1 == dev_pcd_check_typeBpresent())
                    {           
                        picctype_exist_flg[1] = 1;  
                    }
                }
            }
            //搜寻C卡
            if(0 != (checkparam.mode&RF_MODE_PICCC))
            {//说明需要查询C卡

            }
            return -1;
        }
        /////////////////////////////////////////
        //说明有卡响应
        //----->所有需要支持的卡都查询完
        j = 0; 
        for(i=0; i<3; i++)
        {
            if(picctype_exist_flg[i] != 0)j++;
        }
        if(j!=1)
        {   //下电
            if(j > 1)   //说明有多张卡
            {
                status = MI_COLLERR;//MI_MORECARD;  //冲突
            }
            else
            {           
                status = -1;
            }
        }
        else    
        {       //有卡在场域附近
            if(1 == picctype_exist_flg[0])
            {
                status = dev_pcd_check_typeAAnticollision();
                if(MI_OK == status)
                { 
                    if(0 != (PICCData[cidtmp].PiccSAK&0x20))      //UID完整,PICC遵循ISO/IEC 14443-4
                    {
                        rbuf[0] = PICC_TYPEA;
                    }
                    else                        //UID完整,PICC不遵循ISO/IEC 14443-4
                    {
                        rbuf[0] = PICC_MIFARE;  // mifare 卡
                    } 
                    memcpy(&rbuf[1], PICCData[cidtmp].MLastSelectedSnr, PICCData[cidtmp].MLastSelectedsnrLen);
                    rbuf[PICCData[cidtmp].MLastSelectedsnrLen+1] = PICCData[cidtmp].PiccSAK; // SAK
                    *rlen = PICCData[cidtmp].MLastSelectedsnrLen + 2;
                }
            }
            else if(1 == picctype_exist_flg[1])
            {   //有B型卡
                status = dev_pcd_check_typeBAnticollision();
                if(MI_OK == status)
                {
                    rbuf[0] = PICC_TYPEB;
                    memcpy(&rbuf[1], &PICCData[cidtmp].AtsData[1], PICCData[cidtmp].AtsLen-1);
                    *rlen = (PICCData[cidtmp].AtsLen-1) + 1;
                }
                
            }
        }
        //清除标志
        memset(picctype_exist_flg, 0, 3); 
        ////////////////////////////////////////////////////
        if(MI_OK == status)
        {
            checkparam.checkcontinue = 0;           //j
            checkparam.checkremovflg = 0;
        }
        else if((-1) != status)//if(MI_COLLERR == status)
        {         
//dev_debug_printf("%s(%d):\r\n", __FUNCTION__, __LINE__);            
            rf_delayms(1);
            dev_pcd_reset(6); 
        }
    }
    return status;
}
///////////////////////////////////////////////////////////

s32 dev_pcd_check_picc1(u8 *cid, u8 *rbuf, u16 *rlen)
{     
    u8 cidtmp;
    s32 status= -1;
    *rlen = 0;
    *cid = 0;
    cidtmp = 0;
    
    if(1 == checkparam.checkcontinue)
    {     
        while(1)
        {
            if(0 != (checkparam.mode & RF_MODE_PICCA))
            {//说明需要查询A卡
                
                status = dev_pcd_check_typeAAnticollision();
                if(MI_OK == status)
                { 
                    if(0 != (PICCData[cidtmp].PiccSAK&0x20))      //UID完整,PICC遵循ISO/IEC 14443-4
                    {
                        rbuf[0] = PICC_TYPEA;
                    }
                    else                        //UID完整,PICC不遵循ISO/IEC 14443-4
                    {
                        rbuf[0] = PICC_MIFARE;  // mifare 卡
                    } 
                    memcpy(&rbuf[1], PICCData[cidtmp].MLastSelectedSnr, PICCData[cidtmp].MLastSelectedsnrLen);
                    rbuf[PICCData[cidtmp].MLastSelectedsnrLen+1] = PICCData[cidtmp].PiccSAK; // SAK
                    *rlen = PICCData[cidtmp].MLastSelectedsnrLen + 2;
                }
                break;
            }
            if(0 != (checkparam.mode&RF_MODE_PICCB))
            {
                status = dev_pcd_check_typeBAnticollision();
                if(MI_OK == status)
                {
                    rbuf[0] = PICC_TYPEB;
                    memcpy(&rbuf[1], &PICCData[cidtmp].AtsData[1], PICCData[cidtmp].AtsLen-1);
                    *rlen = (PICCData[cidtmp].AtsLen-1) + 1;
                }  
                break;
            }
            
        }
        if(MI_OK == status)
        {
            checkparam.checkcontinue = 0;           //j
            checkparam.checkremovflg = 0;
        }
        else if((-1) != status)//if(MI_COLLERR == status)
        {            
            rf_delayms(1);
            dev_pcd_reset(6); 
        }
    }
    return status;
} 

s32 dev_pcd_get_uid(u8 *rbuf, u8 rle)
{
    s32 i;

    if(rle > PICCData[0].MLastSelectedsnrLen)
    {
        i = PICCData[0].MLastSelectedsnrLen;
    }
    else
    {
        i = rle;
    }
    
    memcpy(rbuf, PICCData[0].MLastSelectedSnr, i);
    return i;
}

s32 dev_pcd_typeARemove(void)
{
    s32 i;
    s32 status;
    u8 tmp[64];
    
    dev_pcd_select_type(PICC_TYPEA); 
    rf_delayms(5);
    for(i=0; i<3; i++)
    {                
      //发送WUPA
       status = dev_pcd_send_reqa(0x52, tmp);
       rf_delayms(6);              
        if(MI_NOTAGERR != status)
        {                           
            //让A卡进入HALT状态
            status = dev_pcd_halt_piccA();
            return 1;       
        }
    }
    return 0;                     //卡已经离开
}

s32 dev_pcd_typeBRemove(void)
{
    s32 i;
    s32 status;
    u8 tmp[64];
    //给B卡上电
    dev_pcd_select_type(PICC_TYPEB);       //设置TYPE A场域
    rf_delayms(5);
    for(i=0; i<3; i++)
    {
        //发送WUPB
        status = dev_pcd_req_piccB(0x00, 0, 0x08, tmp, &tmp[15]);
        if(MI_NOTAGERR != status)
        {   //说明有B型卡 
            return 1;
        } 
    } 
    return 0;
}
///////////////////////////////////////////////////////////
s8 dev_pcd_checkremov_sta(void)
{
    rf_delayms(2);//dev_delay_ms(2); 
    dev_pcd_reset(6);
    checkparam.checkcontinue = 0; 
    checkparam.checkremovflg = 1;   //需要检测卡被移除
    return 0;
}
s8 dev_pcd_checkremov_picc(void)
{
    s8 status;
    u8 cidtmp=0;
    u8 i;
    u8 ret = 1;
    u8 tmp[16]; 
    
    if((0 == checkparam.checkcontinue) && (1 == checkparam.checkremovflg))
    {
        if(PICC_TYPEA == PICCData[cidtmp].picctype) 
        {   //说明之前有A卡，需要等待A卡移除
            
            dev_pcd_select_type(PICC_TYPEA);       //设置TYPE A 场域
            rf_delayms(5);
            for(i=0; i<3; i++)
            {                
              //发送WUPA
               status = dev_pcd_send_reqa(0x52, tmp);
               rf_delayms(4);//dev_delay_ms(4);                   
                if(MI_NOTAGERR != status)
                {                            //说明有A卡  
                    //让A卡进入HALT状态
                    status = dev_pcd_halt_piccA();
                    break;
                }
            }
        }
        else 
        {   //说明之前有B卡，需要等待B卡移除
            //给B卡上电
            dev_pcd_select_type(PICC_TYPEB);       //设置TYPE A场域
            rf_delayms(5);
            for(i=0; i<3; i++)
            {
                //发送WUPB
                status = dev_pcd_req_piccB(0x00, 0, 0x08, tmp, &tmp[15]);
                if(MI_NOTAGERR != status)
                {   //说明有B型卡
                    break;
                } 
            }
        }
        if(i>=3)
        {       //卡被移除
            
            //dev_pcd_softpowerdown();
            checkparam.checkremovflg = 0;
            ret = 0;
            
        }
    }
    
    return ret;
}
//-----------------------------------
s32 dev_pcd_powerup(u8 type)
{
    u8 i;
    if(0 != (type&0x01))
    {
        i = PICC_TYPEA; 
    }
    else if(0 != (type&0x02))
    {
        i = PICC_TYPEB;
    }
    else if(0 != (type&0x04))
    {
        i = PICC_TYPEC;
    }
    else if(0 != (type&0x08))
    {
        i = PICC_TYPE24;
    }
    else
    {
        return -1;
    }
    dev_pcd_open();
    if(MI_OK == dev_pcd_set_para(type, i))
    {
        if(type&0x0F)
        {
            checkparam.checkcontinue = 1;
        }                          
        return 0;
    } 
    else
    {
        return -1;
    }
}
//

s32 dev_pcd_powerdown(void)
{
    dev_pcd_close();
    rf_loopback_init();
    return 0;
}
#if 0
/*-----------------------------------
return 0:无卡
       1：有一张卡
       2：有多张卡
/-----------------------------------*/
u8 dev_pcd_query_picc(u8 *cid, u8* rbuf, u16 *rlen)
{
    s32 ret;
                
    ret = dev_pcd_check_picc(cid, rbuf, rlen);
    if(MI_OK == ret)
    {
       return 1;       //有卡
    }
    else if((MI_COLLERR == ret) || (MI_ANTICOLL_ERR == ret))  
    {  
        g_rf_morecard_cnt++;
//        if(g_rf_morecard_cnt>10)
        {
            g_rf_morecard_cnt = 0;
            return 2;       //多张卡
        }
    }
    return 0;
}
#endif


s32 dev_pcd_poll(u8 card_type, u8 *rbuf, u32 *rlen, u32 rbufsize)
{
    s32 error;
    u8 tmp;
//    static u8 coll = 0;
    static u32 ts1 = 0;
    nPICCData lpPICCData;
    static u8 pollcnt = 0;

    dev_pcd_open();
    if(PCD_DRIVER_TYPE_NONE == g_pcd_driver_type)
    {
        return -1;
    }
    card_present = 0;
    pollcnt++;

    if(card_type&RF_MODE_PICCA)
    {
        if(1 == dev_pcd_check_typeApresent())
        {
            pollcnt = 0;
            /* ISO14443-A card(s) found. */
            card_present |= RF_MODE_PICCA;
            /* Send HLTA command. */
            rf_delayms(2);//dev_delay_ms(5);
            error = dev_pcd_halt_piccA();

            if(MI_OK == error)
            {
            }
        }
    }

    if(card_type&RF_MODE_PICCB)
    {
        if(1 == dev_pcd_check_typeBpresent())
        {
            pollcnt = 0;
            /* ISO14443-B card(s) found. */
            card_present |= RF_MODE_PICCB;

            if((0 == (card_present & RF_MODE_PICCA)) && (card_type & RF_MODE_PICCA))
            {
                if(1 == dev_pcd_check_typeApresent())
                {
                    //pollcnt = 0;
                    /* ISO14443-A card(s) found. */
                    card_present |= RF_MODE_PICCA;
                    /* Send HLTA command. */
                    rf_delayms(2);//dev_delay_ms(5);
                    error = dev_pcd_halt_piccA();

                    if(MI_OK == error)
                    {
                    }
                }
            }
        }
    }

    if((card_present&RF_MODE_AB) == RF_MODE_AB)
    {
        error = -2;
    }
    else if(0 != (card_present & RF_MODE_AB))
    {
        /* Anticollision. */
        //sleepMilliseconds(EMV_T_P);

        if(card_present == RF_MODE_PICCA)
        {
            error = dev_pcd_check_typeAAnticollision();
        }
        else if(card_present == RF_MODE_PICCB)
        {
            error = dev_pcd_check_typeBAnticollision();
        }
        dev_pcd_get_piccparam(&lpPICCData);
        
        if(MI_OK == error)
        {
            rbuf[0] = 0;

            if(card_present == RF_MODE_PICCA)
            {
                rbuf[1] = 'A';
                tmp = lpPICCData.MLastSelectedsnrLen > (rbufsize - 3) ? (rbufsize - 3) : lpPICCData.MLastSelectedsnrLen;
                memcpy(&rbuf[2], lpPICCData.MLastSelectedSnr, tmp);
                rbuf[2 + tmp] = lpPICCData.PiccSAK; //SAK
                *rlen = tmp + 3;
            }
            else
            {
                rbuf[1] = 'B';
                tmp = (lpPICCData.AtsLen - 1) > (rbufsize - 2) ? (rbufsize - 2) : (lpPICCData.AtsLen - 1);
                memcpy(&rbuf[2], &lpPICCData.AtsData[1], tmp);
                *rlen = 2 + tmp;
            }
        }
        else if((MI_COLLERR == error) || (MI_ANTICOLL_ERR == error))
        {
#if 0
            coll++;

            if(coll > 3)
            {
                coll = 0;

                if((dev_get_timeID() - ts1) > 1000)
                {
                    ts1 = dev_get_timeID();
                }
                else
                {
                    error = -2;
                }
            }
#else
//uart_printf("More Card2:err=%d\r\n", error);
//uart_printf("error3=%d.\r\n", error);
            error = -2;

#endif
        }
    }
    else
    {
        error = EMV_ERR_TIMEOUT;
    }
#if 0

    if(error == 2)
    {
        if(card_present == CARD_TYPE_A)
        {
            //error =
        }
    }
#endif
    if(error != 0)
    {
        if(0 != (card_present & RF_MODE_AB))
        {
            rf_delayms(6);
            dev_pcd_reset(6); 
        }
    }

    if(error == -2)
    {   //1s钟报一次超时
        if(dev_user_querrytimer(ts1, 1000)) //(dev_get_timeID()-ts1)>1000)
        {
            ts1 = dev_user_gettimeID();
        }
        else
        {
            error = EMV_ERR_TIMEOUT;   //不报超时
        }
    }
    return error;
}
#if 0
s32 dev_pcd_poll_1(u8 card_type, u8 *rbuf, u32 *rlen, u32 rbufsize)
{
    s32 error=-1;
    u8 tmp;
//    static u8 coll = 0;
    static u32 ts1 = 0;
    nPICCData lpPICCData;
    static u8 pollcnt = 0;

    dev_pcd_open();
    if(PCD_DRIVER_TYPE_NONE == g_pcd_driver_type)
    {
        return -1;
    }
    card_present = 0;
    pollcnt++;

    while(1)
    {
        if(card_type&RF_MODE_PICCA)
        {   //说明需要查询A卡
            error = dev_pcd_check_typeAAnticollision();
            if(MI_OK == error)
            {
                rbuf[1] = 'A';
                tmp = lpPICCData.MLastSelectedsnrLen > (rbufsize - 3) ? (rbufsize - 3) : lpPICCData.MLastSelectedsnrLen;
                memcpy(&rbuf[2], lpPICCData.MLastSelectedSnr, tmp);
                rbuf[2 + tmp] = lpPICCData.PiccSAK; //SAK
                *rlen = tmp + 3;
            }
            break;
        }
        if(card_type&RF_MODE_PICCB)
        {   //说明需要查询B卡
            error = dev_pcd_check_typeBAnticollision();
            if(MI_OK == error)
            {
                rbuf[1] = 'B';
                tmp = (lpPICCData.AtsLen - 1) > (rbufsize - 2) ? (rbufsize - 2) : (lpPICCData.AtsLen - 1);
                memcpy(&rbuf[2], &lpPICCData.AtsData[1], tmp);
                *rlen = 2 + tmp;
            }
            break;
        }
    }

    return error;
}
#endif


u8 dev_pcd_get_cardtype(void)
{
    return card_present;
}
//=======================================
//const u8 test_rtsa_wupa[] = {1, 0x51};
//const u8 test_rtsa_hlta[] = {1, 0x50,0x00,0x57,0x7D};
//const u8 test_rtsa_anticollision[] = {2, 0x93, 0x20};
//const u8 test_rtsa_select[] = {7, 0x93,0x70,0x27,0xE9,0x3B,0x11,0xE4};//,0x53,0x46};
const u8 test_rtsa_rats[] = {2, 0xE0, 0x80};//,0x31,0x73};
const u8 test_rtsa_I_block1[] = {21, 0x02,0x00,0xA4,0x04,0x00,0x0E,0x32,0x50,
                                    0x41,0x59,0x2E,0x53,0x59,0x53,0x2E,0x44,
                                    0x44,0x46,0x30,0x31,0x00};//,0xE0,0x42};
const u8 test_rtsa_I_block2[] = {19, 0x03,0x00,0xA4,0x04,0x00,0x0C,0x01,0x02,
                                    0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,
                                    0x0B,0x0C,0x00};//,0x4D,0xC9};
//const u8 test_rtsa_wupb[] = {3, 0x05, 0x00, 0x08};//0x39,0x73};
const u8 test_rtsa_attrib[] = {9, 0x1D,0x46,0xB5,0xC7,0xA0,0x00,0x08,0x01,0x00};//,0x69,0xB5};

s32 dev_rf_test_rtsa_cmd_send(u8 crcflg, const u8* cmd)
{
    u8 data[32];
    u32 datalen;
    u32 i;
    s32 status;

    if(crcflg)
    {
        //要校验
        dev_pcd_ctl_crc(0x03, 0x03);
    }
    else
    {
        //不要校验
        dev_pcd_ctl_crc(0x03, 0x00);
    }
    datalen = cmd[0];
    for(i=0; i<datalen; i++)
    {
        data[i] = cmd[i+1];
    }
    status = dev_pcd_execute_cmd_send(data, datalen);
    
    return status;
    
}

void dev_rf_test_rtsa(s32 mod)
{
    s32 i;
    u8 num;
    u8 tmp[32];
    
    if(mod&0x01)
    {//typeA
        
        //WUPA
        dev_pcd_check_typeApresent();
        //dev_pcd_select_type(PICC_TYPEA);
        //dev_user_delay_us(5000);
        //dev_rf_test_rtsa_cmd_send(0, test_rtsa_wupa);
        dev_user_delay_us(2500);
        
        //HLTA
        //dev_rf_test_rtsa_cmd_send(1, test_rtsa_hlta);
        //dev_user_delay_us(2500);
        dev_pcd_halt_piccA();

        
        
        //WUPB
        dev_pcd_check_typeBpresent();
        //dev_pcd_select_type(PICC_TYPEB);
        //dev_user_delay_us(5000);
        //dev_rf_test_rtsa_cmd_send(1, test_rtsa_wupb);
        //dev_user_delay_us(2500);
        
        //WUPA
        dev_pcd_check_typeApresent();
        //dev_pcd_select_type(PICC_TYPEA);
        //dev_user_delay_us(5000);
        //dev_rf_test_rtsa_cmd_send(0, test_rtsa_wupa);
        dev_user_delay_us(2500);
        
        //ANTICOLLISION  (93 20)
        //dev_rf_test_rtsa_cmd_send(0, test_rtsa_anticollision);
        dev_pcd_selectanticoll_picc(0, 0, 0, tmp, tmp);
        dev_user_delay_us(2500);
        
        //SELECT (93 70 27 E9 3B 11 E4)
        memcpy(tmp, "\x27\xE9\x3B\x11\xE4", 5);
        dev_pcd_selectanticoll_picc(0, 0, 40, tmp, tmp);
        //dev_rf_test_rtsa_cmd_send(1, test_rtsa_select);
        dev_user_delay_us(2500);
        
        //RATS (E0 80)
        dev_rf_test_rtsa_cmd_send(1, test_rtsa_rats);
        //dev_user_delay_us(2500);
        
    }
    else //if(mod&0x02)
    {   //typeB
        //WUPB
        dev_pcd_check_typeBpresent();
        //dev_pcd_select_type(PICC_TYPEB);
        //dev_user_delay_us(5000);
        //dev_rf_test_rtsa_cmd_send(1, test_rtsa_wupb);
        //dev_user_delay_us(2500);
        //WUPA
        dev_pcd_check_typeApresent();
        //dev_pcd_select_type(PICC_TYPEA);
        //dev_user_delay_us(5000);
        //dev_rf_test_rtsa_cmd_send(0, test_rtsa_wupa);
        //dev_user_delay_us(2500);
        //WUPB
        dev_pcd_check_typeBpresent();
        //dev_pcd_select_type(PICC_TYPEB);
        //dev_user_delay_us(5000);
        //dev_rf_test_rtsa_cmd_send(1, test_rtsa_wupb);
        dev_user_delay_us(2500);
        
        //ATTRIB
        dev_rf_test_rtsa_cmd_send(1, test_rtsa_attrib);
        //dev_user_delay_us(2500);
    }
    dev_user_delay_us(2500);
    //I-BLOCK1(02 00 A4 04 00 0E 32 50 41 59 2E 53 59 53 2E 44 44 46 30 31 00)
    dev_rf_test_rtsa_cmd_send(1, test_rtsa_I_block1);
    dev_user_delay_us(2500);
    //I-BLOCK2(03 00 A4 04 00 0C 01 02 03 04 05 06 07 08 09 0A 0B 0C 00)
    dev_rf_test_rtsa_cmd_send(1, test_rtsa_I_block2);
}
//=======================================
void rf_loopback_init(void)
{
    g_rf_lp_step=RF_LP_STEP_POWERON;
    g_rf_lp_flg = 0;
    checkparam.checkcontinue = 0;
    checkparam.checkremovflg = 0;
}

void rf_loopback_ctl(u8 flg)
{
    g_rf_lp_flg = flg;
    checkparam.checkcontinue=flg;
}

#if 1
extern void emvcl_test(void);
void dev_rf_test(void)
{
    u8 flg=1;
    u8 key;
    s32 ret;
 //   u32 value;
    u8 tmp[32];
    u8 cnt=0;
    s32 i;

RF_DEBUG("\r\n");
//    posdev_keypad_open();
//    ddi_audio_open();
    dev_pcd_open();
    while(1)
    {
        if(flg)
        {
            flg = 0;
            dev_debug_printf("====%s====\r\n", __FUNCTION__);
            dev_debug_printf("1.读版本\r\n");
            dev_debug_printf("2.上电\r\n");
            dev_debug_printf("3.LOOPBACK\r\n");
            
            //posdev_keypad_clear();
            dev_com_flush(DEBUG_PORT_NO);
        }
        //ret = posdev_keypad_read_beep(&key);
        ret = dev_com_read(DEBUG_PORT_NO, &key, 1);
        if(ret==1)
        {
            switch(key)
            {
            case '1'://DIGITAL1:
                ret = dev_pn512_read_version(tmp);
dev_debug_printf("%s(%d):ret=%d, v=%02X \r\n", __FUNCTION__, __LINE__, ret, tmp[0]);                
                break;
            case '2'://DIGITAL2:
                ret = dev_pcd_powerup(0x03);
dev_debug_printf("%s(%d):ret=%d\r\n", __FUNCTION__, __LINE__, ret);                
                break;
            case '3'://DIGITAL3:
                emvcl_test();
                break;
            case '4'://DIGITAL4:
                cnt ++;
                switch(cnt)
                {
                case 1:
                    memset(tmp, 0x55, 32);
                    break;
                case 2:
                    memset(tmp, 0xaa, 32);
                    break;
                case 3:
                    memset(tmp, 0x5a, 32);
                    break;
                case 4:
                    memset(tmp, 0xa5, 32);
                    break;
                case 5:
                    cnt = 0;
                    for(i=0; i<32; i++)
                    {
                        tmp[i] = (i&0x0f)+0x30;
                    }
                    break;
                default:
                    cnt = 0;
                    for(i=0; i<32; i++)
                    {
                        tmp[i] = i;
                    }
                    break;
                }
                dev_pn512_write_fifo(tmp, 32);
 dev_debug_printf("%s(%d):\r\n", __FUNCTION__, __LINE__);                
 PrintFormat(tmp, 32);
              break;
            case '5'://DIGITAL5:
                memset(tmp, 0, 32);
                dev_pn512_read_fifo(tmp, 32);
dev_debug_printf("%s(%d):\r\n", __FUNCTION__, __LINE__);                
PrintFormat(tmp, 32);
                break;
            case '0'://DIGITAL0:
                flg = 1;
                break;
            default:
                break;
            }
        }
    }
}
#endif

