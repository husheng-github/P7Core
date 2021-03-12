/********************************Copyright ( c ) ********************************
**                  DAQU 
**                         
**                        
**
**Created By:     pengxuebin,20170322      
** Created Date:     
** Version:        
** Description:    SPI驱动程序  
****************************************************************************/
#include "devglobal.h"
#include "sdk/mhscpu_sdk.h"
#include "drv_spi.h"

#define SPI_CSN_MAX     1   //硬件片选
//static SPI_TypeDef const tim_type_tab[SPI_PORT_MAX+1] = {SPIM0, SPIM1, SPIM2};
//, SPIS0
//SPI0
#define SPI0_PIN_SCK     GPIO_PIN_NONE
#define SPI0_PIN_CSN0    GPIO_PIN_NONE
#define SPI0_PIN_MOSI    GPIO_PIN_PTB14
#define SPI0_PIN_MISO    GPIO_PIN_PTB15
//SPI1
#define SPI1_PIN_SCK     GPIO_PIN_PTA6
#define SPI1_PIN_CSN0    GPIO_PIN_PTA7
#define SPI1_PIN_MOSI    GPIO_PIN_PTA8
#define SPI1_PIN_MISO    GPIO_PIN_PTA9
//SPI2                                 
#define SPI2_PIN_SCK     GPIO_PIN_PTB2 
#define SPI2_PIN_CSN0    GPIO_PIN_PTB3
#define SPI2_PIN_MOSI    GPIO_PIN_PTB4
#define SPI2_PIN_MISO    GPIO_PIN_PTB5

#if(MH1903_TYPE==MH1903_121)
//SPI3                                 
#define SPI3_PIN_SCK     GPIO_PIN_NONE 
#define SPI3_PIN_CSN0    GPIO_PIN_NONE
#define SPI3_PIN_MOSI    GPIO_PIN_NONE
#define SPI3_PIN_MISO    GPIO_PIN_NONE//GPIO_PIN_NONE
#endif
//#define SP_SEND_TO_AP    GPIO_PIN_PTC0
const str_spi_param_t str_spi_param_tab[SPI_DEV_MAX] = 
{
    //speed    gpiopin       port       master          direction              mode        cs             bpw 
    {8000000, GPIO_PIN_NONE, SPI_PORT3, SPIMODE_MASTER, SPI_DIRECTION_2L_TxRx, SPI_MODE_0, SPI_CSMOD_GPIO, 8 },  //RF
    {8000000, GPIO_PIN_NONE,SPI_PORT2, SPIMODE_MASTER, SPI_DIRECTION_2L_TxRx, SPI_MODE_0, SPI_CSMOD_GPIO, 8 },  //扩张IO设备595
    {8000000, GPIO_PIN_NONE, SPI_PORT1, SPIMODE_MASTER, SPI_DIRECTION_1L_Tx, SPI_MODE_0, SPI_CSMOD_NONE, 8 },  //PRINT
    {8000000, GPIO_PIN_NONE, SPI_PORT3, SPIMODE_MASTER,  SPI_DIRECTION_2L_TxRx, SPI_MODE_1, SPI_CSMOD_HD0,  8 },  //与AP通讯
};
static u32 g_drvspi_flg[SPI_PORT_MAX];      //做为SPI设备打开标志，每一位标记一个设备,分布到不同端口，最多支持32个SPI设备,
static u32 g_spi_oldspeeds[SPI_PORT_MAX];   //为了提高SPI速度，记录该端口上一次配置的速率
static SPI_DEVICE_t g_spi_devive[SPI_PORT_MAX];
//slave   
#if(SPI_SLAVE_EN==1)
//#ifdef TRENDIT_BOOT
//#define SPIS_TXBUF_MAX      1024
//#define SPIS_RXBUF_MAX      5120//1024

//#else
#define SPIS_TXBUF_MAX      (1024+512)
#define SPIS_RXBUF_MAX      2048//1024

//#endif
static volatile u8 g_spis_txdflg=0;
static str_CircleQueue g_spis_txqueue;
static str_CircleQueue g_spis_rxqueue;
#endif
/****************************************************************************
**Description:        spi初始化
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170323
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void drv_spi_init(void)
{
    static u8 flg=1;
    s32 i,j;

    if(flg == 1)
    {
        flg = 0;
        for(i=0; i<SPI_PORT_MAX; i++)
        {            
            g_drvspi_flg[i] = 0;
            g_spi_oldspeeds[i] = 0;
            g_spi_devive[i]= SPI_DEV_MAX;      
        }

        for(j=0; j<SPI_DEV_MAX; j++)
        {
            if(SPI_CSMOD_GPIO == str_spi_param_tab[j].m_cs)
            { 
                //if((j != SPI_DEV_595) || (dev_misc_getmachinetypeid() == MACHINE_TYPE_ID_T1))
                {
                    //由驱动控制片选，配置GPIO，输出为1
                    dev_gpio_config_mux(str_spi_param_tab[i].m_gpiopin, MUX_CONFIG_ALT1);   //???GPIO?
                    dev_gpio_set_pad(str_spi_param_tab[i].m_gpiopin, PAD_CTL_PULL_UP);
                    dev_gpio_direction_output(str_spi_param_tab[i].m_gpiopin, 1);
                }                            
            }
        }
        
        //关闭SPI时钟
        SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_SPI0|SYSCTRL_APBPeriph_SPI2, DISABLE);        
      #if(SPI_SLAVE_EN==1)
        g_spis_txqueue.m_size = 0;
        g_spis_txqueue.m_in = 0;
        g_spis_txqueue.m_out = 0;
        g_spis_txqueue.m_buf = NULL;
        g_spis_rxqueue.m_size = 0;
        g_spis_rxqueue.m_in = 0;
        g_spis_rxqueue.m_out = 0;
        g_spis_txdflg = 0;
      #endif   
        
        
    }
}
/****************************************************************************
**Description:            打开spi设备
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170323
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
u32 drv_spi_cale_BaudRatePrescaler(u32 speedhz)
{
    u32 i;
    
    i = (SYSCTRL->PCLK_1MS_VAL * 1000 +speedhz-1)/speedhz;
    if(i&0x1)
    {
        i++;
    }
    if(i<2)
    {
        i = 2;
    }
    else if(i>65534)
    {
        i = 65534;
    }
    return i;    
}
s32 drv_spi_open(SPI_DEVICE_t spidev)
{
    const str_spi_param_t *lp_spiparam;
    SPI_InitTypeDef SPI_InitStructure;
    SPI_TypeDef *l_spi_typedef;

    if(spidev >= SPI_DEV_MAX)
    {
        SPI_DEBUG("Param err!(spidev=%d)\r\n", spidev);
        return DEVSTATUS_ERR_PARAM_ERR;
    }    
    lp_spiparam = &str_spi_param_tab[spidev];
    if(lp_spiparam->m_port >= SPI_PORT_MAX)
    {
        SPI_DEBUG("Param err!(spi_port=%d)\r\n", lp_spiparam->m_port);
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    if((lp_spiparam->m_bits_per_word<4)||(lp_spiparam->m_bits_per_word>16))
    {
        SPI_DEBUG("Param err!(bits_per_word=%d)\r\n", lp_spiparam->m_bits_per_word);
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    if(g_drvspi_flg[lp_spiparam->m_port]&(1<<(u32)spidev))
    {       
        return 0;   //设备已经打开了，
    }
    switch(lp_spiparam->m_port)
    {
    case 0:
//SPI_DEBUG("Param(speed=%d, port=%d, master=%d,directon=%d,mod=%d, cs=%d, bits=%d)\r\n",
//         lp_spiparam->m_speed, lp_spiparam->m_port, lp_spiparam->m_master, lp_spiparam->m_direction,
//         lp_spiparam->m_mode, lp_spiparam->m_cs, lp_spiparam->m_bits_per_word);        
        if(lp_spiparam->m_master == SPIMODE_SLAVE)
        {
          #if 0 //1902S不支持SPI从模式  
            //从模式
            l_spi_typedef = SPIS0;
            if(g_spis_txqueue.m_buf==NULL)
            {
                if(dev_circlequeue_init(&g_spis_txqueue, SPIS_TXBUF_MAX)<0)
                {       
                    return DEVSTATUS_ERR_FAIL;
                }
            }
            if(g_spis_rxqueue.m_buf==NULL)
            {
                if(dev_circlequeue_init(&g_spis_rxqueue, SPIS_RXBUF_MAX)<0)
                {       
                    return DEVSTATUS_ERR_FAIL;
                }
            }    

            
            //SP_SENDTO_AP配置输出1
            dev_gpio_config_mux(SP_SEND_TO_AP, MUX_CONFIG_ALT1);   //配置为GPIO口
            dev_gpio_set_pad(SP_SEND_TO_AP, PAD_CTL_PULL_UP);
            dev_gpio_direction_output(SP_SEND_TO_AP, 1);
         #else
            return DEVSTATUS_ERR_PARAM_ERR;
         #endif   
            
        }
        else if(lp_spiparam->m_master == SPIMODE_MASTER)
        {
            l_spi_typedef = SPIM0;
        }
        else
        {
            SPI_DEBUG("Param err!(master=%d)\r\n", lp_spiparam->m_master);
            return DEVSTATUS_ERR_PARAM_ERR;
        }    
        //配置管脚
        dev_gpio_config_mux(SPI0_PIN_SCK, MUX_CONFIG_ALT0);
        //片选
        if(SPI_CSMOD_HD0 == lp_spiparam->m_cs)
        {
            //硬件控制片选
            dev_gpio_config_mux(SPI0_PIN_CSN0, MUX_CONFIG_ALT0);
            dev_gpio_set_pad(SPI0_PIN_CSN0, PAD_CTL_PULL_NONE);
            
        }
        else if(SPI_CSMOD_GPIO == lp_spiparam->m_cs)
        {  
            //由驱动控制片选,配置为GPIO,输出1
            dev_gpio_config_mux(lp_spiparam->m_gpiopin, MUX_CONFIG_ALT1);   //配置为GPIO口
            dev_gpio_set_pad(lp_spiparam->m_gpiopin, PAD_CTL_PULL_UP);
            dev_gpio_direction_output(lp_spiparam->m_gpiopin, 1);
        }
        //MOSI
        if(SPI_DIRECTION_1L_Rx != lp_spiparam->m_direction)
        {
            dev_gpio_config_mux(SPI0_PIN_MOSI, MUX_CONFIG_ALT0);
            dev_gpio_set_pad(SPI0_PIN_MOSI, PAD_CTL_PULL_UP);
        }
        //MISO
        if(SPI_DIRECTION_1L_Tx != lp_spiparam->m_direction)
        {
            dev_gpio_config_mux(SPI0_PIN_MISO, MUX_CONFIG_ALT0);
            dev_gpio_set_pad(SPI0_PIN_MISO, PAD_CTL_PULL_UP);
        }
        if(g_drvspi_flg[lp_spiparam->m_port] == 0)
        {
            SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_SPI0, ENABLE);
            SYSCTRL_APBPeriphResetCmd(SYSCTRL_APBPeriph_SPI0, ENABLE);
        }
                
        break;  
    case 1: 
        #if 1//def MH1902_88PINS    //1902S不支持SPI1  
            if(lp_spiparam->m_master == SPIMODE_MASTER)
            {
                l_spi_typedef = SPIM1;
            }
            else
            {
                SPI_DEBUG("Param err!(master=%d)\r\n", lp_spiparam->m_master);
                return DEVSTATUS_ERR_PARAM_ERR;
            }
            //配置管脚
            dev_gpio_config_mux(SPI1_PIN_SCK, MUX_CONFIG_ALT3);
            //片选
            if(SPI_CSMOD_HD0 == lp_spiparam->m_cs)
            {
                //硬件控制片选
                dev_gpio_config_mux(SPI1_PIN_CSN0, MUX_CONFIG_ALT3);
            }
            else if(SPI_CSMOD_GPIO == lp_spiparam->m_cs)
            {  
                //由驱动控制片选,配置为GPIO,输出1
                dev_gpio_config_mux(lp_spiparam->m_gpiopin, MUX_CONFIG_ALT1);   //配置为GPIO口
                dev_gpio_set_pad(lp_spiparam->m_gpiopin, PAD_CTL_PULL_UP);
                dev_gpio_direction_output(lp_spiparam->m_gpiopin, 1);
            }
            //MOSI
            if(SPI_DIRECTION_1L_Rx != lp_spiparam->m_direction)
            {
                dev_gpio_config_mux(SPI1_PIN_MOSI, MUX_CONFIG_ALT3);
                dev_gpio_set_pad(SPI1_PIN_MOSI, PAD_CTL_PULL_UP);
            }
            //MISO
            if(SPI_DIRECTION_1L_Tx != lp_spiparam->m_direction)
            {
                dev_gpio_config_mux(SPI1_PIN_MISO, MUX_CONFIG_ALT3);
                dev_gpio_set_pad(SPI1_PIN_MISO, PAD_CTL_PULL_UP);
            }
            if(g_drvspi_flg[lp_spiparam->m_port] == 0)
            {
                SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_SPI1, ENABLE);
                SYSCTRL_APBPeriphResetCmd(SYSCTRL_APBPeriph_SPI1, ENABLE);
            }
            break;
         #else
            return DEVSTATUS_ERR_PARAM_ERR;
         #endif
    case 2:
        if(lp_spiparam->m_master == SPIMODE_MASTER)
        {
            l_spi_typedef = SPIM2;
        }
        else
        {
            SPI_DEBUG("Param err!(master=%d)\r\n", lp_spiparam->m_master);
            return DEVSTATUS_ERR_PARAM_ERR;
        }
        //配置管脚
        dev_gpio_config_mux(SPI2_PIN_SCK, MUX_CONFIG_ALT0);
        //片选
        if(SPI_CSMOD_HD0 == lp_spiparam->m_cs)
        {
            //硬件控制片选
            dev_gpio_config_mux(SPI2_PIN_CSN0, MUX_CONFIG_ALT0);
        }
        else if(SPI_CSMOD_GPIO == lp_spiparam->m_cs)
        {  
            //由驱动控制片选,配置为GPIO,输出1
            dev_gpio_config_mux(lp_spiparam->m_gpiopin, MUX_CONFIG_ALT1);   //配置为GPIO口
            dev_gpio_set_pad(lp_spiparam->m_gpiopin, PAD_CTL_PULL_UP);
            dev_gpio_direction_output(lp_spiparam->m_gpiopin, 1);
        }
        //MOSI
        if(SPI_DIRECTION_1L_Rx != lp_spiparam->m_direction)
        {
            dev_gpio_config_mux(SPI2_PIN_MOSI, MUX_CONFIG_ALT0);
            dev_gpio_set_pad(SPI2_PIN_MOSI, PAD_CTL_PULL_UP);
        }
        //MISO
        if(SPI_DIRECTION_1L_Tx != lp_spiparam->m_direction)
        {
            dev_gpio_config_mux(SPI2_PIN_MISO, MUX_CONFIG_ALT0);
            dev_gpio_set_pad(SPI2_PIN_MISO, PAD_CTL_PULL_UP);
        }
        if(g_drvspi_flg[lp_spiparam->m_port] == 0)
        {
            SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_SPI2, ENABLE);
            SYSCTRL_APBPeriphResetCmd(SYSCTRL_APBPeriph_SPI2, ENABLE);
        }
        break; 
#if(MH1903_TYPE==MH1903_121)
    case 3:      
        if(lp_spiparam->m_master == SPIMODE_MASTER)
        {
            l_spi_typedef = SPIM3;
        }
        else
        {
            SPI_DEBUG("Param err!(master=%d)\r\n", lp_spiparam->m_master);
            return DEVSTATUS_ERR_PARAM_ERR;
        }
        //配置管脚
        dev_gpio_config_mux(SPI3_PIN_SCK, MUX_CONFIG_ALT0);
        //片选
        if(SPI_CSMOD_HD0 == lp_spiparam->m_cs)
        {
            //硬件控制片选
            dev_gpio_config_mux(SPI3_PIN_CSN0, MUX_CONFIG_ALT0);
        }
        else if(SPI_CSMOD_GPIO == lp_spiparam->m_cs)
        {  
            //由驱动控制片选,配置为GPIO,输出1
            dev_gpio_config_mux(lp_spiparam->m_gpiopin, MUX_CONFIG_ALT1);   //配置为GPIO口
            dev_gpio_set_pad(lp_spiparam->m_gpiopin, PAD_CTL_PULL_UP);
            dev_gpio_direction_output(lp_spiparam->m_gpiopin, 1);
        }
        //MOSI
        if(SPI_DIRECTION_1L_Rx != lp_spiparam->m_direction)
        {
            dev_gpio_config_mux(SPI3_PIN_MOSI, MUX_CONFIG_ALT0);
            dev_gpio_set_pad(SPI3_PIN_MOSI, PAD_CTL_PULL_UP);
        }
        //MISO
        if(SPI_DIRECTION_1L_Tx != lp_spiparam->m_direction)
        {
            dev_gpio_config_mux(SPI3_PIN_MISO, MUX_CONFIG_ALT0);
            dev_gpio_set_pad(SPI3_PIN_MISO, PAD_CTL_PULL_UP);
        }
        if(g_drvspi_flg[lp_spiparam->m_port] == 0)
        {
            SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_SPI3, ENABLE);
            SYSCTRL_APBPeriphResetCmd(SYSCTRL_APBPeriph_SPI3, ENABLE);
        }
        break; 
  #endif
    default:
        return DEVSTATUS_ERR_PARAM_ERR;
//        break;
    }
  #if 0
    if(lp_spiparam->m_master == SPIMODE_SLAVE)
    {
        l_spi_typedef = SPIS0;
        //GPIO_PinRemapConfig(GPIOA, GPIO_Pin_0|GPIO_Pin_1, GPIO_Remap_0);
        GPIO_PinRemapConfig(GPIOB, GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5, GPIO_Remap_0);
    
        SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
        SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
        SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;//SPI_CPOL_High;
        SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
        SPI_InitStructure.SPI_NSS = SPI_NSS_Null;
        SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
        SPI_InitStructure.SPI_RXFIFOFullThreshold = SPI_RXFIFOFullThreshold_1;
        SPI_InitStructure.SPI_TXFIFOEmptyThreshold = SPI_TXFIFOEmptyThreshold_0;
        
        SPI_Init(l_spi_typedef, &SPI_InitStructure);
        SPI_ITConfig(l_spi_typedef, SPI_IT_RXF, ENABLE);
        SPI_Cmd(l_spi_typedef, ENABLE); 
        g_drvspi_flg[lp_spiparam->m_port] |= (1<<(u32)spidev);
        
        NVIC_EnableIRQ(SPI0_IRQn);  
        return 0;
        
    }
  #endif  
    if(SPI_DIRECTION_1L_Tx == lp_spiparam->m_direction)
    {
        SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;//SPI_Direction_1Line_Tx;
    }
    else if(SPI_DIRECTION_1L_Rx == lp_spiparam->m_direction)
    {
        SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Rx;
    }
    else if(SPI_DIRECTION_EEPROM_Rx == lp_spiparam->m_direction)
    {
        SPI_InitStructure.SPI_Direction = SPI_Direction_EEPROM_Read;
    }
    else
    {
        SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    }
    
    SPI_InitStructure.SPI_DataSize = lp_spiparam->m_bits_per_word-1;
    if((lp_spiparam->m_mode)&SPI_MODE_CPOL)
    { 
        SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
    }
    else
    {
        SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    }
    if((lp_spiparam->m_mode)&SPI_MODE_CPHA)
    { 
        SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
    }
    else
    {
        SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    }
    /*if(SPI_CSMOD_HD0  == lp_spiparam->m_cs)
    {
        SPI_InitStructure.SPI_NSS = SPI_NSS_0;
    }
    else*/
    {
        SPI_InitStructure.SPI_NSS = SPI_NSS_Null;
    }
    if(lp_spiparam->m_master == SPIMODE_SLAVE)
    {   
        SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
    }
    else
    {
        SPI_InitStructure.SPI_BaudRatePrescaler = drv_spi_cale_BaudRatePrescaler(lp_spiparam->m_speed);
    }
    g_spi_oldspeeds[lp_spiparam->m_port] = lp_spiparam->m_speed;
    g_spi_devive[lp_spiparam->m_port] = spidev;
    SPI_InitStructure.SPI_RXFIFOFullThreshold = SPI_RXFIFOFullThreshold_1;
    SPI_InitStructure.SPI_TXFIFOEmptyThreshold = SPI_TXFIFOEmptyThreshold_0;
    if(lp_spiparam->m_master == SPIMODE_SLAVE)
    {
        SPI_InitStructure.SPI_RXFIFOFullThreshold = SPI_RXFIFOFullThreshold_1;
        SPI_InitStructure.SPI_TXFIFOEmptyThreshold = SPI_TXFIFOEmptyThreshold_15;
    }
    SPI_Init(l_spi_typedef, &SPI_InitStructure);
    SPI_Cmd(l_spi_typedef, ENABLE);
    if(lp_spiparam->m_master == SPIMODE_SLAVE)
    {
        SPI_ITConfig(l_spi_typedef, SPI_IT_RXF, ENABLE); //允许接收满中断
        SPI_ITConfig(l_spi_typedef, SPI_IT_TXE, ENABLE); //允许发送空中断
        NVIC_EnableIRQ(SPI0_IRQn);
    }
//SPI_DEBUG("speedhz=%d,i=%d\r\n", g_spi_oldspeeds[lp_spiparam->m_port], SPI_InitStructure.SPI_BaudRatePrescaler);    
    g_drvspi_flg[lp_spiparam->m_port] |= (1<<(u32)spidev);

    return 0;
}
/****************************************************************************
**Description:        关闭spi设备
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170323
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_spi_close(SPI_DEVICE_t spidev)
{
    const str_spi_param_t *lp_spiparam;
    SPI_TypeDef *l_spi_typedef;

    if(spidev >= SPI_DEV_MAX)
    {
        SPI_DEBUG("Param err!(spidev=%d)\r\n", spidev);
        return DEVSTATUS_ERR_PARAM_ERR;
    }    
    lp_spiparam = &str_spi_param_tab[spidev];
    if(lp_spiparam->m_port >= SPI_PORT_MAX)
    {
        SPI_DEBUG("Param err!(port=%d)\r\n", lp_spiparam->m_port);
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    if(0 == g_drvspi_flg[lp_spiparam->m_port]&(1<<(u32)spidev))
    {
        return 0;           //没有打开
    }
    switch(lp_spiparam->m_port)
    {
    case 0:
      #if 0  
        if(lp_spiparam->m_master == SPIMODE_SLAVE)
        {
            //从模式
            l_spi_typedef = SPIS0;
            NVIC_DisableIRQ(SPI0_IRQn); 
            
        }
        else if(lp_spiparam->m_master == SPIMODE_MASTER)
        {
            l_spi_typedef = SPIM0;
        }
        else
        {
            return DEVSTATUS_ERR_PARAM_ERR;
        }
      #else
        return DEVSTATUS_ERR_PARAM_ERR;
      #endif  
        break;
    case 1:
      #if 1  
        if(lp_spiparam->m_master == SPIMODE_MASTER)
        {
            l_spi_typedef = SPIM1;
        }
        else
        {
            return DEVSTATUS_ERR_PARAM_ERR;
        }
      #endif  
        break;
    case 2:
        if(lp_spiparam->m_master == SPIMODE_MASTER)
        {
            l_spi_typedef = SPIM2;
        }
        else
        {
            return DEVSTATUS_ERR_PARAM_ERR;
        }
        break;
#if(MH1903_TYPE==MH1903_121)
    case 3:
        if(lp_spiparam->m_master == SPIMODE_MASTER)
        {
            l_spi_typedef = SPIM3;
        }
        else
        {
            return DEVSTATUS_ERR_PARAM_ERR;
        }
        break;
#endif
    default:
        return DEVSTATUS_ERR_PARAM_ERR;
//        break;
    }
    g_drvspi_flg[lp_spiparam->m_port] &= ~(1<<(u32)spidev);
    if(g_drvspi_flg[lp_spiparam->m_port] == 0)
    {
        SPI_Cmd(l_spi_typedef, ENABLE);
        if(0 == lp_spiparam->m_port)
        {
            SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_SPI0, DISABLE);
        }
        else if(1 == lp_spiparam->m_port)
        {
          #if 1 //pengxubin 20190822 
            SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_SPI1, DISABLE);
          #endif 
        }
        else if(2 == lp_spiparam->m_port)
        {
            SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_SPI2, DISABLE);
        }
#if(MH1903_TYPE==MH1903_121)
        else if(3 == lp_spiparam->m_port)
        {
            SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_SPI3, DISABLE);
        }
#endif
    }
    return 0;
}
/****************************************************************************
**Description:        spi主方式传输(查询方式)
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170323
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
static s32 drv_spi_master_transceivebyte_polling(SPI_TypeDef *l_spi_typedef, u8 *txbuf, u8 *rxbuf, s32 le)
{
    s32 i;
    s32 j;
    u32 k;
    u32 timeid=0;
    u32 timecnt=0;

    i=0; 
    j=0;

    k = (~(l_spi_typedef->TXFLR))&0x0F;
    
    if(k>le)
    {
        k = le;
    }
//SPI_DEBUG("k=%d\r\n", k);    
    for(i=0; i<k; i++)
    {
        /* Write in the DR register the data to be sent */
        if(txbuf==NULL)
        {
            l_spi_typedef->DR = 0xff;
        }
        else
        {
            l_spi_typedef->DR = txbuf[i];
        }
    }
    j = 0;
    //timeid = dev_user_gettimeID();
    timecnt = 0;
    while(j<le)
    {
        if(RESET != ((l_spi_typedef->SR)&SPI_FLAG_RXNE))
        {
            k = l_spi_typedef->DR;
            if(rxbuf != NULL)
            {
                rxbuf[j] = k;
            }
            j++;
            if(i < le)
            {
                if(txbuf==NULL)
                {
                    l_spi_typedef->DR = 0xff;
                }
                else
                {
                    l_spi_typedef->DR = txbuf[i++];
                }
            }
            timecnt = 0;
            //timeid = dev_user_gettimeID();
        }
        //if(dev_user_querrytimer(timeid, 10))
        if(timecnt++ > 960000)
        {
            SPI_DEBUG("timerout!(%d)\r\n", j);
            return DEVSTATUS_ERR_FAIL; 
        }
    }
    return 0;
}
static s32 drv_spi_master_transceiveshort_polling(SPI_TypeDef *l_spi_typedef, u16 *txbuf, u16 *rxbuf, s32 le)
{
    s32 i;
    s32 j;
    u32 k;
    u32 timeid;

    i=0; 
    j=0;

    k = (~(l_spi_typedef->TXFLR))&0x0F;
    for(i=0; i<k; i++)
    {
        /* Write in the DR register the data to be sent */
        if(txbuf==NULL)
        {
            l_spi_typedef->DR = 0xff;
        }
        else
        {
            l_spi_typedef->DR = txbuf[i];
        }
    }
    j = 0;
    timeid = dev_user_gettimeID();
    while(j<le)
    {
        if(RESET != ((l_spi_typedef->SR)&SPI_FLAG_RXNE))
        {
            k = l_spi_typedef->DR;
            if(rxbuf != NULL)
            {
                rxbuf[j] = k;
            }
            j++;
            if(i < le)
            {
                if(txbuf==NULL)
                {
                    l_spi_typedef->DR = 0xff;
                }
                else
                {
                    l_spi_typedef->DR = txbuf[i++];
                }
            }
            timeid = dev_user_gettimeID();
        }
        if(dev_user_querrytimer(timeid, 20))
        {
            SPI_DEBUG("timerout!(%d)\r\n", j);
            return DEVSTATUS_ERR_FAIL; 
        }
    }
    return 0;
}
s32 drv_spi_master_transceive_polling(SPI_DEVICE_t spidev, void* txbuf, void* rxbuf, s32 le)
{
    const str_spi_param_t *lp_spiparam;
    //SPI_InitTypeDef SPI_InitStructure;
    SPI_TypeDef *l_spi_typedef;
    s32 i;
    u32 n;
    s32 ret;

    if(le == 0)
    {   //不需要传输
        return 0;
    }
    if(spidev >= SPI_DEV_MAX)
    {
        SPI_DEBUG("Param err!(spidev=%d)\r\n", spidev);
        return DEVSTATUS_ERR_PARAM_ERR;
    }    
    lp_spiparam = &str_spi_param_tab[spidev];
    if(lp_spiparam->m_port >= SPI_PORT_MAX)
    {
        SPI_DEBUG("Param err!(port=%d)\r\n", lp_spiparam->m_port);
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    if(0 == g_drvspi_flg[lp_spiparam->m_port]&(1<<(u32)spidev))
    {
        SPI_DEBUG("SPI not open!(port=%d,spidev=%d)\r\n", lp_spiparam->m_port, spidev);
        return DEVSTATUS_ERR_DEVICE_NOTOPEN;           //没有打开
    }
    switch(lp_spiparam->m_port)
    {
    case 0:
        if(lp_spiparam->m_master == SPIMODE_MASTER)
        {
            l_spi_typedef = SPIM0;
        }
        else
        {
            SPI_DEBUG("Param err!(master=%d)\r\n", lp_spiparam->m_master);
            return DEVSTATUS_ERR_PARAM_ERR;
        }
        break;
    case 1:
      #if 1 //pengxubin 20190822   
        if(lp_spiparam->m_master == SPIMODE_MASTER)
        {
            l_spi_typedef = SPIM1;
        }
        else
        {
            SPI_DEBUG("Param err!(master=%d)\r\n", lp_spiparam->m_master);
            return DEVSTATUS_ERR_PARAM_ERR;
        }
      #endif  
        break;
    case 2:
        if(lp_spiparam->m_master == SPIMODE_MASTER)
        {
            l_spi_typedef = SPIM2;
        }
        else
        {
            SPI_DEBUG("Param err!(master=%d)\r\n", lp_spiparam->m_master);
            return DEVSTATUS_ERR_PARAM_ERR;
        }
        break;
#if(MH1903_TYPE==MH1903_121)
    case 3:
        if(lp_spiparam->m_master == SPIMODE_MASTER)
        {
            l_spi_typedef = SPIM3;
        }
        else
        {
            SPI_DEBUG("Param err!(master=%d)\r\n", lp_spiparam->m_master);
            return DEVSTATUS_ERR_PARAM_ERR;
        }
        break;
#endif
    default:
        return DEVSTATUS_ERR_PARAM_ERR;
//        break;
    }
    
    //配置
    if(g_spi_devive[lp_spiparam->m_port] != spidev)
    {
//SPI_DEBUG("\r\n");
        n = 0;
        if(SPI_DIRECTION_1L_Tx == lp_spiparam->m_direction)
        {
            n = SPI_Direction_1Line_Tx;
        }
        else if(SPI_DIRECTION_1L_Rx == lp_spiparam->m_direction)
        {
            n = SPI_Direction_1Line_Rx;
        }
        else if(SPI_DIRECTION_EEPROM_Rx == lp_spiparam->m_direction)
        {
            n = SPI_Direction_EEPROM_Read;
        }
        else
        {
            n = SPI_Direction_2Lines_FullDuplex;
        }
        n |= (lp_spiparam->m_bits_per_word-1);
        if((lp_spiparam->m_mode)&SPI_MODE_CPOL)
        { 
            n |= SPI_CPOL_High;
        }
        if((lp_spiparam->m_mode)&SPI_MODE_CPHA)
        { 
            n |= SPI_CPHA_2Edge;
        }
        
//        SPI_Init(l_spi_typedef, &SPI_InitStructure);
        l_spi_typedef->SSIENR = 0;      //DISABLE current SPI before configure CONTROL registers
        l_spi_typedef->IMR = 0;         //关中断
        l_spi_typedef->CTRLR0 = n;
        if(g_spi_oldspeeds[lp_spiparam->m_port] != lp_spiparam->m_speed)
        {
            g_spi_oldspeeds[lp_spiparam->m_port] = lp_spiparam->m_speed;
            n = drv_spi_cale_BaudRatePrescaler(lp_spiparam->m_speed);
            l_spi_typedef->BAUDR = n;
        }
        //l_spi_typedef->RXFTLR = SPI_RXFIFOFullThreshold_1;
        //l_spi_typedef->TXFTLR = SPI_TXFIFOEmptyThreshold_0;
        //SPI_MasterSlaveModeSet(l_spi_typedef);
        
        g_spi_devive[lp_spiparam->m_port] = spidev;
    }
    
    i = 0;
    //Clear RX FIFO
    while(SPI_GetFlagStatus(l_spi_typedef, SPI_FLAG_RXNE))
    {
        SPI_ReceiveData(l_spi_typedef);
        i++;
        if(i>20)
        {
            SPI_DEBUG("SPI Err!\r\n");
            return DEVSTATUS_ERR_FAIL;
        }
    }  
    
    //初始化片选信号
    if(SPI_CSMOD_HD0  == lp_spiparam->m_cs)
    {
        l_spi_typedef->SER = SPI_NSS_0;
    }
    else
    {
        l_spi_typedef->SER = SPI_NSS_0;
    }

    if(SPI_CSMOD_GPIO == lp_spiparam->m_cs)
    {
        dev_gpio_set_value(lp_spiparam->m_gpiopin, 0);

//SPI_DEBUG("gpiopin=%08X,%08X\r\n", GPIO_PIN_PTB12, lp_spiparam->m_gpiopin);        
    }
    
    /* Enable the selected SPI peripheral */
    l_spi_typedef->SSIENR = 1;        //ENABLE current SPI

    if(lp_spiparam->m_bits_per_word>8)
    {
        ret = drv_spi_master_transceiveshort_polling(l_spi_typedef, (u16*)txbuf, (u16*)rxbuf, le);
    }
    else
    {
        ret = drv_spi_master_transceivebyte_polling(l_spi_typedef, (u8*)txbuf, (u8*)rxbuf, le);
    }
    //释放片选
    if(SPI_CSMOD_HD0  == lp_spiparam->m_cs)
    {
        l_spi_typedef->SER = SPI_NSS_Null;
    }
    else if(SPI_CSMOD_GPIO == lp_spiparam->m_cs)
    {
        dev_gpio_set_value(lp_spiparam->m_gpiopin, 1);
    }
    /* Disable the selected SPI peripheral */
    l_spi_typedef->SSIENR = 0;        //Disable current SPI
    
    return ret;
}
#if(SPI_SLAVE_EN==1)
void SPI_Slave_Isr(SPI_TypeDef *SPIx)
{
    u32 i,j;
    u8 fifotmp[16];
    u32 status;

//dev_debug_printf("s0\r\n");    
    status = SPIx->ISR;
    //if(SPI_GetITStatus(SPIx,SPI_IT_TXE) == SET && is_start_rxd)
    if((status&SPI_IT_TXE) != RESET)
    {
//dev_debug_printf("t"); 
        //发送
        j = SPIx->TXFLR&0x1f;
        if(j<16)
        {
            i = 16-j;
            memset(fifotmp, 0xff, i);
            j = dev_circlequeue_read(&g_spis_txqueue, fifotmp, i);
          #if 1  
            if(j<i)
            {
                //发送数据已经全部推送到缓冲，
                g_spis_txdflg = 0;                
//                SPI_ITConfig(SPIx, SPI_IT_TXE, DISABLE); //禁止发送空中断
                dev_gpio_set_value(SP_SEND_TO_AP, 1); 
            }
          #endif  
            for(j=0; j<i; j++)
            {
                //SPI_SendData(SPIx, data_buf[send_buf_index++]);
                SPIx->DR = fifotmp[j];
            }
        }
        SPI_ClearITPendingBit(SPIx, SPI_IT_TXE);   //??
    }
    //if(SPI_GetITStatus(SPIx,SPI_IT_RXF) == SET)
    if((status&SPI_IT_RXF) != RESET)
    {
      #if 0  
        data_buf[rece_buf_index++] = SPI_ReceiveData(SPIx);
        if(rece_buf_index == DATA_BUF_SIZE)
            is_start_rxd = 1;
        rece_buf_index = rece_buf_index % DATA_BUF_SIZE;
      #endif 
//dev_debug_printf("r");      
        //接收数据
        i = SPIx->RXFLR&0x1f;
        if(i>16)
        {
            i = 16;
        }
        for(j=0; j<i; j++)
        {
            fifotmp[j] = SPIx->DR;
        }
        dev_circlequeue_write(&g_spis_rxqueue, fifotmp, i);
        SPI_ClearITPendingBit(SPIx, SPI_IT_RXF);  //??
    }

    

    NVIC_ClearPendingIRQ(SPI0_IRQn);
    
}
#if 0//pengxuebin,20190822 1902S不支持SPI从模式
void SPI0_IRQHandler(void)
{  
    SPI_Slave_Isr(SPIS0);
}
#endif

s32 drv_spi_slave_write(u8 *wbuf, u16 wlen)
{
  #if 0  
    s32 cnt;
    u32 i,j;
    u8 fifotmp[16];
    SPI_TypeDef *SPIx;
    u8 flg = 0;

  #if 0
    if(g_spis_txdflg)
    {
        //说明上一包没有发送出去
        return 0;
    }
  #endif  
    i = dev_circlequeue_getspace(&g_spis_txqueue);
    if(i<wlen)
    {
        return 0;
    }
    cnt = dev_circlequeue_write(&g_spis_txqueue, wbuf, wlen);
    //发送
    if(g_spis_txdflg==0)
    {
       #if 0
        SPIx = SPIS0;
        j = (SPIx->TXFLR)&0x1f;
        if(j<16)
        {
            i = 16-j;
            memset(fifotmp, 0xff, i);
            j = dev_circlequeue_read(&g_spis_txqueue, fifotmp, i);
          #if 0  
            if(j<i)
            {
                g_spis_txdflg = 0;
            }
            else   
           #endif  
            {
 //               SPI_ITConfig(SPIx, SPI_IT_TXE, ENABLE); //允许发送空中断
            }
            for(i=0; i<j; i++)
            {
                //SPI_SendData(SPIx, data_buf[send_buf_index++]);
                SPIx->DR = fifotmp[i];
            }
            //g_spis_txdflg = 1;
            //SPI_ITConfig(SPIx, SPI_IT_TXE, ENABLE); //允许发送空中断
//SPI_DEBUG("\r\n");
//SPI_DEBUGHEX(NULL, fifotmp, i);        
        }
        g_spis_txdflg = 1;
        dev_gpio_set_value(SP_SEND_TO_AP, 0); 
      #endif  
//SPI_DEBUG("\r\n");
    }
    //发送脉冲
    return cnt;
  #else
    return DEVSTATUS_ERR_PARAM_ERR;
  #endif
}

s32 drv_spi_slave_read(u8 *rbuf, u16 rlen)
{
  #if 0  
    s32 ret;

    ret = dev_circlequeue_read(&g_spis_rxqueue, rbuf, rlen);
    return ret;
  #else
    return 0;
  #endif  
}

s32 drv_spi_slave_check_txbusying(void)
{
  #if 0
    return g_spis_txdflg;
  #else
    return 1;
  #endif
}


s32 drv_spi_slave_gettxstatus(u32 *txspacelen)
{
  #if 0
    if(g_spis_txdflg == 0)
    {
        *txspacelen = dev_circlequeue_getspace(&g_spis_txqueue);
    }

    return g_spis_txdflg;
  #else
   *txspacelen = 0;
    return 1;
  #endif
}


#endif

