#include "devglobal.h"
#include "devapi/dev_rf/dev_rf_io.h"

#define RF_SUPPORT_NULL 0
#define RF_SUPPORT_NOT  1
#define RF_SUPPORT_YES  2
static u8 g_rf_support_flg = RF_SUPPORT_NULL;

//#define RF_CS_PIN     GPIO_PIN_PTB12  

#if(MACHINETYPE==MACHINE_S1)
#define RF_INT_PIN    //GPIO_PIN_PTA13 

#define RF_RST_PIN_T1  GPIO_PIN_EX2
#define RF_RST_PIN_M6  GPIO_PIN_PTB1
iomux_pin_name_t RF_RST_PIN = GPIO_PIN_NONE;

#define RF_POWER_5V_PIN    GPIO_PIN_EX7//GPIO_PIN_PTC5 
#define RF_POWER_5V_PIN_T3  GPIO_PIN_PTB0 //pengxuebin,20190818 T3使用
static iomux_pin_name_t g_rf_power_5v_pin;
#else
#define RF_INT_PIN    GPIO_PIN_PTB13 
#define RF_RST_PIN    GPIO_PIN_PTB14
#endif

//SPI_DEV g_rfspidev;


#if 0    
#define RF_INT_PIN     KL81_PIN_PTC16
#define RF_CS_PIN      KL81_PIN_PTC12


void dev_rf_io_init(void)
{
    //INT
    dev_gpio_config_mux(RF_INT_PIN, MUX_CONFIG_GPIO);
    dev_gpio_set_pad(RF_INT_PIN, PAD_CTL_PULL_UP|PAD_CTL_SLEW_HI);
    dev_gpio_direction_input(RF_INT_PIN);
    //CS
    dev_gpio_config_mux(RF_CS_PIN, MUX_CONFIG_GPIO);
    dev_gpio_set_pad(RF_CS_PIN, PAD_CTL_PULL_UP|PAD_CTL_SLEW_HI);
    dev_gpio_direction_output(RF_CS_PIN, 1);
}
#else
void dev_rf_io_init(void)
{   
    s32 ret;
    u8 machineid;
    
    if(g_rf_support_flg==RF_SUPPORT_NULL)
    {
//dev_debug_printf("%s(%d):\r\n", __FUNCTION__, __LINE__); 
      #if 0
        //1.识别RF是否存在，通过RF_CS_PIN管脚识别
        Iomconfig(RF_CS_PIN);      
        IomInOutSet(RF_CS_PIN, 1);      //输入
        dev_delay_ms(1);
        if(IomGetVal(RF_CS_PIN))
        {
            g_rf_support_flg = RF_SUPPORT_YES;
        }
        else
        {
            g_rf_support_flg = RF_SUPPORT_NOT;
        }
//    g_rf_support_flg = RF_SUPPORT_NOT;     
        //针对支持非接的处理
        if(RF_SUPPORT_YES == g_rf_support_flg)
        {
            //INT  input
            Iomconfig(RF_INT_PIN);
            IomInOutSet(RF_INT_PIN, 1);
            //RF_CS     output
            IomInOutSet(RF_CS_PIN, 0);
            IomSetVal(RF_CS_PIN, 1);
            //RF_RST    output
            IomInOutSet(RF_RST_PIN, 0);
            IomSetVal(RF_RST_PIN, 0);
            dev_delay_ms(1);
          #if 0  
            //复位
            IomSetVal(RF_RST_PIN, 1);
            dev_delay_ms(2);
          #endif  
            //spi参数
            g_rfspidev.spiport = 0;
            g_rfspidev.bits = 8;
            //gLcdSpi.cpol = kDSPI_ClockPolarityActiveHigh;
            //gLcdSpi.cpha = kDSPI_ClockPhaseFirstEdge;
            g_rfspidev.fhz = 8000000;  //8MHZ
            g_rfspidev.cstype = SPICSTYPE_RF;//1; 
//dev_debug_printf("%s(%d):\r\n", __FUNCTION__, __LINE__);            
        }
      #endif  
        g_rf_support_flg = RF_SUPPORT_YES;
        machineid = dev_misc_getmachinetypeid();
        
        if(machineid == MACHINE_TYPE_ID_T1)
        {
          #if 0
            RF_RST_PIN = RF_RST_PIN_T1;
            g_rf_power_5v_pin = RF_POWER_5V_PIN;
          #endif
            RF_RST_PIN = GPIO_PIN_NONE;         //T3没有管脚控制RF复位
            g_rf_power_5v_pin = GPIO_PIN_NONE;//RF_POWER_5V_PIN_T3;
          
        }
        else if((machineid == MACHINE_TYPE_ID_M6)
              ||(machineid == MACHINE_TYPE_ID_T5)
              ||(machineid == MACHINE_TYPE_ID_T6))
        {
            RF_RST_PIN = RF_RST_PIN_M6;
            g_rf_power_5v_pin = GPIO_PIN_NONE;
        } 
        else if(machineid == MACHINE_TYPE_ID_T3)
        {
            RF_RST_PIN = GPIO_PIN_NONE;         //T3没有管脚控制RF复位
            g_rf_power_5v_pin = GPIO_PIN_NONE;//RF_POWER_5V_PIN_T3;
        }
        else
        {
            return;
        }
      #if 0 
        //INT
        dev_gpio_config_mux(RF_INT_PIN, MUX_CONFIG_GPIO);
        dev_gpio_set_pad(RF_INT_PIN, PAD_CTL_PULL_UP|PAD_CTL_SLEW_HI);
        dev_gpio_direction_input(RF_INT_PIN);
      #endif  
      
      #if(MACHINETYPE==MACHINE_S1) 
        //5V电源控制，初始化输出1, 关闭5V 
        dev_gpio_config_mux(g_rf_power_5v_pin, MUX_CONFIG_ALT1);   //配置为GPIO口
        dev_gpio_set_pad(g_rf_power_5v_pin, PAD_CTL_PULL_UP);
        dev_gpio_direction_output(g_rf_power_5v_pin, 0);
      #endif  
      #if 0  
        //CS
        dev_gpio_config_mux(RF_CS_PIN, MUX_CONFIG_GPIO);
        dev_gpio_set_pad(RF_CS_PIN, PAD_CTL_PULL_UP|PAD_CTL_SLEW_HI);
        dev_gpio_direction_output(RF_CS_PIN, 1);
      #endif
        //RST
        dev_gpio_config_mux(RF_RST_PIN, MUX_CONFIG_GPIO);
        dev_gpio_set_pad(RF_RST_PIN, PAD_CTL_PULL_UP|PAD_CTL_SLEW_HI);
        dev_gpio_direction_output(RF_RST_PIN, 0);
      #if 1  
        dev_user_delay_ms(2);
        dev_gpio_set_value(RF_RST_PIN, 1);
        dev_user_delay_ms(2);
      #endif
        dev_spi_open(SPI_DEV_RF);
        
    }   
}
void dev_rf_5v_ctl(u8 flg)
{
#if(MACHINETYPE==MACHINE_S1)
    dev_gpio_set_value(g_rf_power_5v_pin, flg);
#endif
}
void dev_rf_rst_ctl(u8 flg)
{
#if 0    
    dev_rf_io_init();
    if(flg)
    {
        dev_gpio_set_value(RF_RST_PIN, 1);
    }
    else
    {
        dev_gpio_set_value(RF_RST_PIN, 0);
    }
#endif    
}
void dev_rf_sleep_ctl(u8 flg)
{
    s32 ret;
    
    dev_rf_io_init();

    //读取1902硬件版本
  #if 0    //重复判断
    machineid = dev_misc_getmachinetypeid();
    if(machineid == MACHINE_TYPE_ID_T1)
    {
        RF_RST_PIN = RF_RST_PIN_T1;
    }
    else if(machineid == MACHINE_TYPE_ID_M6
          ||machineid == MACHINE_TYPE_ID_T6)//sxl 20190626 增加T6机型
    {
        RF_RST_PIN = RF_RST_PIN_M6;
    }
    else
    {
        return;
    }
  #endif  
    
    
    
    if(flg)
    {
        dev_gpio_set_value(RF_RST_PIN, 0);      //sleep
    }
    else
    {
        dev_gpio_set_value(RF_RST_PIN, 1);      //wakeup
        //初始化led
    }
    
    
}
/***********************************************
return :    0: 不支持非接
            1: 支持非接
***********************************************/
s32 dev_rf_support_judge(void)
{
    dev_rf_io_init();
    if(RF_SUPPORT_YES == g_rf_support_flg)
    {
        return 1;
    }
    else
    {
        return 0;   
    }
}
#if 0
u8 dev_rf_int_get(void)
{   
    if(0 == dev_gpio_get_value(RF_INT_PIN))
    {
        return 0;
    }
    else
    {
        return 1;
    }   
}
#endif
/***********************************************
return :    0: 不支持非接
            1: 支持非接
***********************************************/
#if 0
s32 drv_spi_rfcomm(u8 *snd, u8 *rsv, s32 size)
{
    RF_SPICS_CTL(0);
    dev_spi_exchange(&g_rfspidev, snd, rsv, size);
    RF_SPICS_CTL(1);
}
#endif
/*******************************************************************************
 ** Descriotions:   设置寄存器
 ** parameters:     regaddr: 寄存器地址
                    regval : 寄存器值                                                         
 ** Returned value: 0:成功
                    <0:失败                                                                    
 ** Created By:     pengxuebin 2017/4/15
 ** Remarks:                                                                                  
*******************************************************************************/
s32 dev_pn512_write_reg(u8 regaddr, u8 regval)
{
#if 0    
    //RF_SPICS_CTL(0);
    RF_SPICS_CTL0;
    Spim0RfWriteData(SPI_DATA_MODE, regaddr<<1, &regval, 1);
    //RF_SPICS_CTL(1);  
    RF_SPICS_CTL1;
#endif  
    u8 snd[2];
    s32 ret;

    pt_ctl_slp(1, 1);
    snd[0] = (regaddr<<1)&0x7F;
    snd[1] = regval;
    ret = dev_spi_master_transceive_polling(SPI_DEV_RF, &snd, NULL, 2);
    pt_ctl_slp(1, 0);
    return ret;
}
/*******************************************************************************
 ** Descriotions:  读寄存器
 ** parameters:     regaddr: 寄存器地址
                    RegVal :  返回寄存器值                                                           
 ** Returned value: 0:成功
                    <0:失败                                                                   
 ** Created By:    pengxuebin 2017/4/15
 ** Remarks:                                                                                  
*******************************************************************************/
s32 dev_pn512_read_reg(u8 regaddr, u8 *regval)
{
#if 0    
    //RF_SPICS_CTL(0);
    RF_SPICS_CTL0;
    Spim0RfReadData(SPI_DATA_MODE, (regaddr<<1)|0x80, regval, 1);
    //RF_SPICS_CTL(1);
    RF_SPICS_CTL1;
#endif  
    u8 snd[2];
    u8 rsv[2];
    s32 ret;

    pt_ctl_slp(1, 1);
    snd[0] = (regaddr<<1)|0x80;
    snd[1] = 0;
    ret = dev_spi_master_transceive_polling(SPI_DEV_RF, &snd, rsv, 2);
    if(ret < 0)
    {
        RF_DEBUG("spi err!(ret=%d)\r\n", ret);
        return ret;
    }
    *regval = rsv[1];
    pt_ctl_slp(1, 0);
    return 0;  
}
/*******************************************************************************
 ** Descriotions:   修改寄存器
 ** parameters:     regaddr: 寄存器地址 
                    mode:    修改方式 0:将指定位清0
                                      1:将指定位置1
                    maskbyte:修改修改的位掩码                                                        
 ** Returned value: 0:成功
                    <0:失败   
                                                                                              
 ** Created By:     pengxuebin 2017/4/15          
 ** Remarks:                                                                                  
*******************************************************************************/
s32 dev_pn512_modify_reg(u8 regaddr, u8 mode, u8 maskbyte)
{   
    u8 regval;
    
    dev_pn512_read_reg(regaddr, &regval);   //读寄存器的值
    if(mode)
    {
        regval |= maskbyte;
    }
    else
    {
        regval &= (~maskbyte);
    }
    dev_pn512_write_reg(regaddr, regval);
    return 1;
}

/*******************************************************************************
 ** Descriotions:   写FIFO寄存器(连续写)
 ** parameters:     *wbuf: 需要写的数据指针
                    wlen : 写的数据长度
 ** Returned value: 0:成功
                    <0:失败                                                                          
 ** Created By:     pengxuebin 2017/4/15        
 ** Remarks:                                                                                  
*******************************************************************************/
#define     JREG_FIFODATA        0x09 
s32 dev_pn512_write_fifo(u8 *wbuf, s32 wlen)
{
#if 0    
    if(wlen!=0)
    {
        //RF_SPICS_CTL(0);
        RF_SPICS_CTL0;
        Spim0RfWriteData(SPI_DATA_MODE, (JREG_FIFODATA<< 1), wbuf, wlen);
        //RF_SPICS_CTL(1);
        RF_SPICS_CTL1;
    }
    return 1;
#endif 
    u8 snd[RF_SPI_FIFOMAX];
    s32 ret=0;
    s32 len;
    s32 i;
    
    if((wbuf == NULL) || (wlen < 0))
    {
        RF_DEBUG("Param err!(wlen=%d,wbuf=%08X)\r\n", wlen, wbuf);
        return -1;
    }
    pt_ctl_slp(1, 1);
    i = 0;
    while(i<wlen)
    {
        if((i+RF_SPI_FIFOMAX-1)<wlen)
        {
            len = RF_SPI_FIFOMAX-1;
        }
        else
        {
            len = wlen-i;
        }
        snd[0] = (JREG_FIFODATA<<1);
        memcpy(&snd[1], &wbuf[i], len);
        ret = dev_spi_master_transceive_polling(SPI_DEV_RF, snd, NULL, len+1);
        if(ret < 0)
        {
            RF_DEBUG("spi err!(ret=%d)\r\n", ret);
            pt_ctl_slp(1, 0);
            return ret;
        }
        i += len;
    }
    pt_ctl_slp(1, 0);
    return 0;     
}
/*******************************************************************************
 ** Descriotions:   读FIFO寄存器(连续读)
 ** parameters:     *rbuf: 需要写的数据指针
                    rlen : 写的数据长度
 ** Returned value: 0:成功
                    <0:失败 
                                                                                              
 ** Created By:     pengxuebin 2017/4/15         
 ** Remarks:                                                                                  
*******************************************************************************/
s32 dev_pn512_read_fifo(u8 *rbuf, s32 rlen)
{
#if 0    
    if(rlen!=0)
    {
            //RF_SPICS_CTL(0);
            RF_SPICS_CTL0;
            Spim0RfReadData(SPI_DATA_MODE, ((JREG_FIFODATA << 1) | 0x80), rbuf, rlen);
            //RF_SPICS_CTL(1);
            RF_SPICS_CTL1;
    }
    return 1;
#endif  
    u8 snd[RF_SPI_FIFOMAX];
    u8 rsv[RF_SPI_FIFOMAX];
    u8 ch;
    s32 ret=0;
    s32 len;
    s32 i;
    
    if((rbuf == NULL) || (rlen < 0))
    {
        RF_DEBUG("Param err!(rlen=%d, rbuf=%08X)\r\n", rlen, rbuf);
        return -1;
    }
    pt_ctl_slp(1, 1);
    i = 0;
    ch = ((JREG_FIFODATA<<1)|0x80);
    while(i<rlen)
    {
        if((i+RF_SPI_FIFOMAX-1)<rlen)
        {
            len = RF_SPI_FIFOMAX-1;
        }
        else
        {
            len = rlen-i;
        }
        memset(&snd[0], ch, len);
        snd[len] = 0;
        memset(&rsv[0], 0, len+1);
        ret = dev_spi_master_transceive_polling(SPI_DEV_RF, snd, rsv, len+1);
        if(ret < 0)
        {
            RF_DEBUG("spi err!(ret=%d)\r\n", ret);
            pt_ctl_slp(1, 0);
            return ret;
        }
        memcpy(&rbuf[i], &rsv[1], len);
        i += len;
    }
    pt_ctl_slp(1, 0);
    return 0;  
}
//=======================================================
/*******************************************************************************
 ** Descriotions:   设置寄存器
 ** parameters:     regaddr: 寄存器地址
                    regval : 寄存器值                                                         
 ** Returned value: 0:成功
                    <0:失败                                                                     
 ** Created By:     pengxuebin 2017/4/15   
 ** Remarks:                                                                                  
*******************************************************************************/
s32 dev_rc663_write_reg(u8 regaddr, u8 regval)
{
#if 0    
    //RF_SPICS_CTL(0);
    RF_SPICS_CTL0;
    Spim0RfWriteData(SPI_DATA_MODE, regaddr<<1, &regval, 1);
    //RF_SPICS_CTL(1); 
    RF_SPICS_CTL1; 
    return 1;
#endif  
    u8 snd[2];
    s32 ret;

    snd[0] = (regaddr<<1);
    snd[1] = regval;
    ret = dev_spi_master_transceive_polling(SPI_DEV_RF, &snd, NULL, 2);
    return ret; 
}
s32 dev_rc663_write_serial(u8 addrsta, u8 *wbuf, u32 wlen)
{
#if 0    
    //RF_SPICS_CTL(0);
    RF_SPICS_CTL0;
    Spim0RfWriteData(SPI_DATA_MODE, (addrsta<<1), wbuf, wlen);
    //RF_SPICS_CTL(1); 
    RF_SPICS_CTL1;
    return 1;
#endif    
    s32 i;
    s32 len;
    s32 ret = 0;
    u8 snd[RF_SPI_FIFOMAX];
    

    if((wbuf == NULL) || (wlen == 0))// || (wlen > (260 -2)))
    {
        RF_DEBUG("Param err!(wlen=%d, wbuf=%08X)\r\n", wlen, wbuf);
        return -1;
    }
    i = 0;
    while(i<wlen)
    {
        if((i+RF_SPI_FIFOMAX-1)<wlen)
        {
            len = RF_SPI_FIFOMAX-1;
        }
        else
        {
            len = wlen-i;
        }
        snd[0] = ((addrsta+i)<<1);
        memcpy(&snd[1], &wbuf[i], len);
        ret = dev_spi_master_transceive_polling(SPI_DEV_RF, &snd, NULL, 2);
        if(ret < 0)
        {
            RF_DEBUG("spi err!(ret=%d)\r\n", ret);
            return ret;
        }
        i += len;
        
    }
    return 0;
}

/*******************************************************************************
 ** Descriotions:  读寄存器
 ** parameters:     regaddr: 寄存器地址
                    RegVal :  返回寄存器值                                                           
 ** Returned value: 0:成功
                    <0:失败                                                                   
 ** Created By:     pengxuebin 2017/4/15
 ** Remarks:                                                                                  
*******************************************************************************/
s32 dev_rc663_read_reg(u8 regaddr, u8 *regval)
{
#if 0    
    //RF_SPICS_CTL(0);
    RF_SPICS_CTL0;
    Spim0RfReadData(SPI_DATA_MODE, (regaddr<<1)|0x01, regval, 1);
    //RF_SPICS_CTL(1); 
    RF_SPICS_CTL1;
    return 1;
#endif   
    s32 len;
    s32 ret = 0;
    u8 snd[2];
    u8 rsv[2];
        
    if(regval == NULL)
    {
        RF_DEBUG("Param err!\r\n");
        return -1;
    }
    
    snd[0] = (regaddr<<1)|0x01;
    snd[1] = 0;
    memset(rsv,0,2);
    len = 2;

    ret = dev_spi_master_transceive_polling(SPI_DEV_RF, snd, rsv, len);

    if(ret < 0)
    {
        RF_DEBUG("spi err!(ret=%d)\r\n", ret);
        return ret;
    }
    *regval = rsv[1];
    
    return 0; 
}
/*******************************************************************************
 ** Descriotions:   修改寄存器
 ** parameters:     regaddr: 寄存器地址 
                    mode:    修改方式 0:将指定位清0
                                      1:将指定位置1
                    maskbyte:修改修改的位掩码                                                        
 ** Returned value: 0:成功
                    <0:失败   
                                                                                              
 ** Created By:     pengxuebin 2017/4/15
 ** Remarks:                                                                                  
*******************************************************************************/
s32 dev_rc663_modify_reg(u8 regaddr, u8 mode, u8 maskbyte)
{
    u8 regval;
    
    dev_rc663_read_reg(regaddr, &regval);   //读寄存器的值
    if(mode)
    {
        regval |= maskbyte;
    }
    else
    {
        regval &= (~maskbyte);
    }
    dev_rc663_write_reg(regaddr, regval);
    return 1;
}

/*******************************************************************************
 ** Descriotions:   写FIFO寄存器(连续写)
 ** parameters:     *wbuf: 需要写的数据指针
                    wlen : 写的数据长度
 ** Returned value: 0:成功
                    <0:失败                                                                           
 ** Created By:    pengxuebin 2017/4/15      
 ** Remarks:                                                                                  
*******************************************************************************/
#define J663REG_FIFOData            0x05    
s32 dev_rc663_write_fifo(u8 *wbuf, s32 wlen)
{
#if 0    
    if(wlen!=0)
    {
        //RF_SPICS_CTL(0);
        RF_SPICS_CTL0;
        Spim0RfWriteData(SPI_DATA_MODE, (J663REG_FIFOData<< 1), wbuf, wlen);
        //RF_SPICS_CTL(1); 
        RF_SPICS_CTL1;
    }
    return 1;
#endif    
    s32 i;
    s32 len;
    s32 ret = 0;
    u8 snd[RF_SPI_FIFOMAX];

    if( (wbuf == NULL) || (wlen < 0))// || (wlen > (260 -2)))
    {
        RF_DEBUG("Param err!(wlen=%d, wbuf=%08X)\r\n", ret, wbuf);
        return -1;
    }
    i = 0;
    while(i<wlen)
    {
        if((i+RF_SPI_FIFOMAX-1)<wlen)
        {
            len = RF_SPI_FIFOMAX-1;
        }
        else
        {
            len = wlen-i;
        }
        snd[0] =  (J663REG_FIFOData<< 1);
        memcpy(&snd[1], &wbuf[i], len);
        ret = dev_spi_master_transceive_polling(SPI_DEV_RF, snd, NULL, len+1);
        if(ret < 0)
        {
            RF_DEBUG("spi err!(ret=%d)\r\n", ret);
            return ret;
        }
        i += len;
    }
    return 0;
}
/*******************************************************************************
 ** Descriotions:   读FIFO寄存器(连续读)
 ** parameters:     *rbuf: 需要写的数据指针
                    rlen : 写的数据长度
 ** Returned value: 0:成功
                    <0:失败  
                                                                                              
 ** Created By:     pengxuebin 2017/4/15         
 ** Remarks:                                                                                  
*******************************************************************************/
s32 dev_rc663_read_fifo(u8 *rbuf, s32 rlen)
{
#if 0    
    if(rlen!=0)
    {
        //RF_SPICS_CTL(0);
        RF_SPICS_CTL0;
        Spim0RfReadData(SPI_DATA_MODE, ((J663REG_FIFOData << 1) | 0x01), rbuf, rlen);
        //RF_SPICS_CTL(1); 
        RF_SPICS_CTL1;
    }
    return 1;
#endif   
    s32 i;
    s32 len;
    s32 ret = 0;
    u8 snd[RF_SPI_FIFOMAX];
    u8 rsv[RF_SPI_FIFOMAX];
    u8 ch;

    if((rbuf == NULL) || (rlen == 0))// || (wlen > (260 -2)))
    {
        RF_DEBUG("Param err!(rlen=%d, rbuf=%08X)\r\n", rlen, rbuf);
        return -1;
    }
    i = 0;
    
    ch = ((J663REG_FIFOData<<1)|0x01);
    while(i<rlen)
    {
        if((i+RF_SPI_FIFOMAX-1)<rlen)
        {
            len = RF_SPI_FIFOMAX-1;
        }
        else
        {
            len = rlen-i;
        }
        memset(&snd[0], ch, len);
        snd[len] = 0;
        memset(&rsv[0], 0,  len+1);
        ret = dev_spi_master_transceive_polling(SPI_DEV_RF, snd, rsv, len+1);
        if(ret < 0)
        {
            RF_DEBUG("spi err!(ret=%d)\r\n", ret);
            return ret;
        }
        memcpy(&rbuf[i], &rsv[1], len);
        i += len;
        
    }
    return 0;
}



#endif

void PrintFormat(u8 *src, s32 srclen)
{
  #ifdef RF_DEBUG_EN  
    RF_DEBUG(NULL, src, srclen);
  #endif
}

#if 0
void drv_rf_io_ctl(void)
{
    
}
#endif

