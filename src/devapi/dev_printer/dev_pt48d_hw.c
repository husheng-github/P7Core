/**
 * @file dev_pt48d_hw.c
 * @brief 打印机硬件初始化代码
 * @since 2021.3.20  胡圣整理代码
 */

#include "devglobal.h"
#include "sdk/mhscpu_sdk.h"

#if(PRINTER_EXIST==1)
#include "dev_pt48d_hw.h"
#include "dev_pt48d_detect.h"
#include "dev_pt48d.h"

static volatile u8 g_printf_prn_power_flg=0;
static volatile u8 g_printf_moto_flg=0;

static u32 g_pt_line = 0;  
static u32 g_print_moto_timerid=0;            
static u8 g_prt_slp_flg;            //bit0表示打印机，bit1表示非接，对应为1表示打开,0表示关闭
static u8 g_prt_existflg=0;         //记录是否有打印机标志：0：没有打印机 1:有打印机
                                    //高4位为打印机控制方式,0：旧打印方式,1:T3打印方式

static iomux_pin_name_t  g_prt_mot_pwr_pin;  //空 
static iomux_pin_name_t  g_prt_mot_phase2B_pin;   
static iomux_pin_name_t  g_prt_power_pin;  //打印逻辑电源控制
static iomux_pin_name_t  g_prt_slp_pin; 

int print_nopaper_count = 0;  

#ifdef PT48D_DEV_DEBUG_
void DBG_DAT(u8 *data,u16 len)
{
    u16 i = 0;
    u16 j;

    while(i<len)
    {
        if((len-i)>16)
        {
            for(j=0;j<16;j++)
            {
                dev_debug_printf("%02X ", data[j+i]);
            }

            i+=16;

            dev_debug_printf("\r\n");
        }
        else
        {
            while(i<len)
            {
                dev_debug_printf("%02X ", data[i++]);
            }
        }
    }
}
#endif

/**
 * @brief  判断打印机是否存在
 * @retval 0-不存在  1-存在
 * @retval 高4位表示打印机控制方式：0-旧打印方式  1-T3打印方式
 */
u8 pt_get_exist(void)
{
    return g_prt_existflg;
}

/**
 * @brief  打印机管脚初始化
 */
void pt_gpio_init(void)
{   
    g_prt_mot_pwr_pin = GPIO_PRT_MOT_PWR; 
    g_prt_mot_phase2B_pin = GPIO_PRT_MOT_PHASE_2B;     
    g_prt_power_pin = GPIO_PRT_POWER; 
    g_prt_slp_pin = GPIO_PRT_SLP;  
    g_prt_existflg = 1;  
    
    dev_gpio_config_mux(g_prt_mot_pwr_pin, MUX_CONFIG_GPIO);   
    dev_gpio_set_pad(g_prt_mot_pwr_pin, PAD_CTL_PULL_UP);
    dev_gpio_direction_output(g_prt_mot_pwr_pin, 0);
  
    dev_gpio_config_mux(GPIO_PRT_MOT_PHASE_1A, MUX_CONFIG_GPIO);   
    dev_gpio_set_pad(GPIO_PRT_MOT_PHASE_1A, PAD_CTL_PULL_UP);
    dev_gpio_direction_output(GPIO_PRT_MOT_PHASE_1A, 0);  
 
    dev_gpio_config_mux(GPIO_PRT_MOT_PHASE_2A, MUX_CONFIG_GPIO);   
    dev_gpio_set_pad(GPIO_PRT_MOT_PHASE_2A, PAD_CTL_PULL_UP);
    dev_gpio_direction_output(GPIO_PRT_MOT_PHASE_2A, 0);        
  
    dev_gpio_config_mux(GPIO_PRT_MOT_PHASE_1B, MUX_CONFIG_GPIO);   
    dev_gpio_set_pad(GPIO_PRT_MOT_PHASE_1B, PAD_CTL_PULL_UP);
    dev_gpio_direction_output(GPIO_PRT_MOT_PHASE_1B, 0);          

    dev_gpio_config_mux(g_prt_mot_phase2B_pin, MUX_CONFIG_GPIO);   
    dev_gpio_set_pad(g_prt_mot_phase2B_pin, PAD_CTL_PULL_UP);
    dev_gpio_direction_output(g_prt_mot_phase2B_pin, 0); 
  
    dev_gpio_config_mux(GPIO_PRT_LAT, MUX_CONFIG_GPIO);   
    dev_gpio_set_pad(GPIO_PRT_LAT, PAD_CTL_PULL_UP);
    dev_gpio_direction_output(GPIO_PRT_LAT, 0);      
  
    dev_gpio_config_mux(GPIO_PRT_STB1, MUX_CONFIG_GPIO);   
    dev_gpio_set_pad(GPIO_PRT_STB1, PAD_CTL_PULL_UP);
    dev_gpio_direction_output(GPIO_PRT_STB1, 0);      
 
    dev_gpio_config_mux(g_prt_power_pin, MUX_CONFIG_GPIO);   
    dev_gpio_set_pad(g_prt_power_pin, PAD_CTL_PULL_UP);
    dev_gpio_direction_output(g_prt_power_pin, 1);
    g_printf_prn_power_flg = 0;   

    dev_gpio_config_mux(g_prt_slp_pin, MUX_CONFIG_GPIO);   
    dev_gpio_set_pad(g_prt_slp_pin, PAD_CTL_PULL_UP);

    //唤醒打印机
    if((pt_get_exist()&0xF0)==0x10)
    {
        dev_gpio_direction_output(g_prt_slp_pin, 0);   
        g_prt_slp_flg = 0;
    }
    else
    {
        dev_gpio_direction_output(g_prt_slp_pin, 1);
    }
    
    g_printf_moto_flg = 0;     
    
    dev_gpio_config_mux(GPIO_PRT_PAPER_DET, MUX_CONFIG_GPIO);   
    dev_gpio_set_pad(GPIO_PRT_PAPER_DET, PAD_CTL_PULL_NONE);
    dev_gpio_direction_input(GPIO_PRT_PAPER_DET);      
}

/**
 * @brief  打印机休眠控制
 * @param [in]  flg:1-进入休眠  0-退出休眠
 */
void pt_ctl_slp(u8 type, u8 flg)
{
	dev_gpio_set_value(g_prt_slp_pin, flg);
}

/**
 * @brief  打印机休眠控制
 * @note  不常用
 * @note  T3打印方式
 * @param [in]  flg:1-进入休眠  0-退出休眠
 */
void pt_ctl_slp_sleep(u8 flg)
{
    //T3打印方式
    if((pt_get_exist()&0xF0)==0x10)
    {
        if(flg == 1)   //进入休眠
        {          
            dev_gpio_set_value(g_prt_slp_pin, 1);

            //PRT_SPI_MOSI设置为GPIO 
            dev_gpio_config_mux(PRT_SPI_MOSI, MUX_CONFIG_GPIO);
            dev_gpio_set_pad(PRT_SPI_MOSI, PAD_CTL_PULL_NONE);
            dev_gpio_direction_output(PRT_SPI_MOSI, 0);
        }
        else  //退出休眠
        {  
            //配置回PRT_SPI_MOSI模式
            dev_gpio_config_mux(PRT_SPI_MOSI, MUX_CONFIG_ALT0);
            dev_gpio_set_pad(PRT_SPI_MOSI, PAD_CTL_PULL_UP);
            
            dev_gpio_set_value(g_prt_slp_pin, 0);
        }
    }
}

/**
 * @brief  单点计数
 */
void pt_printline_count(void)
{
    g_pt_line++;
}

/**
 * @brief  点数清除
 */
void pt_printline_clear(void)
{
    g_pt_line = 0;
}

/**
 * @brief  获取打印长度
 * @note  单位：mm
 * @retval 打印长度
 */
u32 pt_get_printline_len(void)
{
    u32 len;

    len =g_pt_line * 125 / 2000;  //全步，每两点走一步，每步0.125mm
    return len;
}

/**
 * @brief  打开IC卡电源 
 * @note  应该默认是高电平
 */
void icc_open_power()
{
    SYSCTRL->PHER_CTRL &= (~BIT(20));  //SCI0 VCCEN信号有效电平选择,高电平有效
}

/**
 * @brief  关闭IC卡电源
 */
void icc_close_power()
{
    SYSCTRL->PHER_CTRL |= BIT(20);  //SCI0 VCCEN信号有效电平选择,低电平有效
}

/**
 * @brief  打印机休眠
 */
void pt_sleep(void)
{
    if(pt_get_exist())
    {
        //不加热
        STROBE_0_OFF();
        STROBE_1_OFF();

        //马达驱动控制，不转
        MOTOR_PHASE_1A_LOW();
        MOTOR_PHASE_1B_LOW();
        MOTOR_PHASE_2A_LOW();
        MOTOR_PHASE_2B_LOW();

        //打印机下电
        MOTOR_PWR_OFF();
        LATCH_LOW();
        PRN_POWER_DISCHARGE();

        //进入休眠
        pt_ctl_slp(0, 1);          
        dev_spi_close(SPI_DEV_PRINT);
        dev_adc_close(DEV_ADC_PTR_TM);        
    }
}

/**
 * @brief  休眠唤醒
 */
void pt_resume(void)
{
    if(pt_get_exist())
    {
        //休眠唤醒
        pt48d_dev_init();        
        pt_ctl_slp(0, 0);
        dev_spi_open(SPI_DEV_PRINT);
        dev_adc_open(DEV_ADC_PTR_TM);

        //上电
        PRN_POWER_CHARGE();
        LATCH_HIGH();
        
        SetDesity();       
        print_nopaper_count = 0;
        dev_printer_setpaper_state(PT_STATUS_IDLE);
        TPPaperSNSDetect(0);
        pt_printline_clear();

      #ifndef TRENDIT_BOOT
        Queueinit();  //初始化队列
      #endif
    }
}


void CHECKPAPER_LED_ON(void)
{
     if((pt_get_exist()&0xf0) == 0x10)
    {
        dev_gpio_set_value(g_prt_mot_pwr_pin,1);
        dev_user_delay_us(1000);
    }
}

void CHECKPAPER_LED_OFF(void)
{
    if((pt_get_exist()&0xf0) == 0x10)
    {
        dev_gpio_set_value(g_prt_mot_pwr_pin,0);
    }
}

/**
 * @brief  检测是否有纸
 * @retval 0:有纸  1:没纸
 */
u8 pt_check_paper(void)
{
    s32 i;
    s32 ret = 0;

    //检测是否有纸
    for(i=0; i<3; i++)
    {
        if(0 == dev_gpio_get_value(GPIO_PRT_PAPER_DET))
        {            
            break;
        }
        dev_user_delay_us(5);
    }

    //无纸
    if(i>=3)
    {
        //无纸
        //dev_debug_printf("no paper \r\n");
        print_nopaper_count ++;
        if(print_nopaper_count>=PAPER_SNSDETECT_MAX)
        {
            //dev_debug_printf("--------------------------------------------no paper \r\n");
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else  //有纸
    {        
        print_nopaper_count = 0;
        return 0;
    }
}


/**
 * @brief  检测是否有纸
 * @retval 0:有纸  1:没纸
 */
uint8_t pt_get_paper_status(void)
{
    s32 ret;
    s32 i;
    
    if(pt_get_exist())
    {
        if(g_printf_moto_flg)
        {
            //pt_check_paper();
        }
        else
        {
            CHECKPAPER_LED_ON();
         
            for(i=0; i<PAPER_SNSDETECT_MAX+2; i++)
            {
                pt_check_paper();
                dev_user_delay_us(500);
            }
            
            if(!g_printf_moto_flg)
            {
                CHECKPAPER_LED_OFF();
            }         
        }
        
        if(print_nopaper_count>=PAPER_SNSDETECT_MAX)
        {
            return 1;
        }
        else
        {
            return 0;  
        }
    }
    else
    {
        return 0;
    }
    
}

/**
 * @brief  打印机电源锁存拉高
 */
void LATCH_HIGH(void) 
{
    if(pt_get_exist())
    {
        dev_gpio_set_value(GPIO_PRT_LAT,1);
    }
}

/**
 * @brief  打印机电源锁存拉低
 */
void LATCH_LOW(void)                     
{    
    if(pt_get_exist())
    {
        dev_gpio_set_value(GPIO_PRT_LAT,0);
    }    
}

/**
 * @brief  打印机上电
 */
void MOTOR_PWR_ON(void)          
{   
    if(pt_get_exist())
    {
        if(g_printf_moto_flg == 0)
        {
            g_printf_moto_flg = 1;
            g_print_moto_timerid = dev_user_gettimeID();
            dev_gpio_set_value(g_prt_mot_pwr_pin,1);
            
            if((pt_get_exist()&0xF0)==0x10)
            {
//                dev_gpio_set_value(g_prt_slp_pin,0);
            }
            else
            {                
                dev_gpio_set_value(g_prt_slp_pin, 0);
            }
        }
    }
}

/**
 * @brief  打印机下电
 */
void MOTOR_PWR_OFF(void)         
{
    if(pt_get_exist())
    {
        g_printf_moto_flg = 0;
        dev_gpio_set_value(g_prt_mot_pwr_pin,1);
    }    
}

/**
 * @brief  1A控制管脚拉高
 */
void MOTOR_PHASE_1A_HIGH(void)  
{
    if(pt_get_exist())
    {
        dev_gpio_set_value(GPIO_PRT_MOT_PHASE_1A,1);
    }
}

/**
 * @brief  1A控制管脚拉低
 */
void MOTOR_PHASE_1A_LOW(void)   
{
    if(pt_get_exist())
    {
        dev_gpio_set_value(GPIO_PRT_MOT_PHASE_1A,0);
    }
}

/**
 * @brief  1B控制管脚拉高
 */
void MOTOR_PHASE_1B_HIGH(void)  
{
    if(pt_get_exist())
    {
        dev_gpio_set_value(GPIO_PRT_MOT_PHASE_1B,1);
    }
}

/**
 * @brief  1B控制管脚拉低
 */
void MOTOR_PHASE_1B_LOW(void)   
{
    if(pt_get_exist())
    {
        dev_gpio_set_value(GPIO_PRT_MOT_PHASE_1B,0);
    }
}

/**
 * @brief  2A控制管脚拉高
 */
void MOTOR_PHASE_2A_HIGH(void)  
{
    if(pt_get_exist())
    {
        dev_gpio_set_value(GPIO_PRT_MOT_PHASE_2A,1);
    }    
}

/**
 * @brief  2A控制管脚拉低
 */
void MOTOR_PHASE_2A_LOW(void)   
{
    if(pt_get_exist())
    {
        dev_gpio_set_value(GPIO_PRT_MOT_PHASE_2A,0);
    }    
}

/**
 * @brief  2B控制管脚拉高
 */
void MOTOR_PHASE_2B_HIGH(void)  
{
    if(pt_get_exist())
    {
        dev_gpio_set_value(g_prt_mot_phase2B_pin,1);
    }
}

/**
 * @brief  2B控制管脚拉低
 */
void MOTOR_PHASE_2B_LOW(void)   
{
    if(pt_get_exist())
    {
        dev_gpio_set_value(g_prt_mot_phase2B_pin,0);
    }
}

/**
 * @brief  打印机上电
 */
void PRN_POWER_CHARGE(void) 
{
    if(g_printf_prn_power_flg == 0)
    {
   	    dev_gpio_set_value(g_prt_power_pin,1);
        g_printf_prn_power_flg = 1;
    }
}

/**
 * @brief  打印机下电
 */
void PRN_POWER_DISCHARGE(void) 
{  
    dev_gpio_set_value(g_prt_power_pin,0);
    g_printf_prn_power_flg = 0;   
}

void STROBE_0_ON(void)     
{
    
}

void STROBE_0_OFF(void)    
{
    
}

/**
 * @brief  加热控制
 */

void STROBE_1_ON(void) 
{
    if(pt_get_exist())
    {
        dev_gpio_set_value(GPIO_PRT_STB1,1);
    }
}

void STROBE_1_OFF(void)
{
    if(pt_get_exist())
    {
        dev_gpio_set_value(GPIO_PRT_STB1,0);
    }    
}

/**
 * @brief  打印机SPI接口初始化
 */
void pt_spi_init(void)
{
    if(pt_get_exist())
    {
        dev_spi_open(SPI_DEV_PRINT);
    }
}

/**
 * @brief  打印机传输数据
 */
void TPDataShiftOut(uint8_t *p, uint16_t len)
{   
    dev_spi_master_transceive_polling(SPI_DEV_PRINT,p,NULL,len);
}

/**
 * @brief  启动定时器
 * @note  打印机使用
 */
void pt_timer_start(void)
{        
    dev_timer_run(TIMER_TP);
}

/**
 * @brief  停止定时器
 * @note  打印机使用
 */
void pt_timer_stop(void)
{        
    dev_timer_stop(TIMER_TP);
}

/**
 * @brief  设置定时时间
 * @param [in]  pHuiPeriodUsash:定时时间
 */
void pt_timer_set_periodUs(uint32_t uiPeriodUs)
{
    dev_timer_setvalue(TIMER_TP,uiPeriodUs);
}

/**
 * @brief  启动定时器
 * @note  打印纸及温度检测使用
 */
void pt_detect_timer_start(void)
{
    dev_timer_run(TIMER_TP_DET);
}

/**
 * @brief  停止定时器
 * @note  打印纸及温度检测使用
 */
void pt_detect_timer_stop(void)
{
    dev_timer_stop(TIMER_TP_DET);
}

/**
 * @brief  设置定时时间 
 * @note  打印纸及温度检测时钟
 * @param [in]  pHuiPeriodUsash:定时时间
 */
void pt_detect_timer_set_period(uint32_t uiPeriodMs)
{
    uint32_t uiPeriodUs;

    uiPeriodUs = uiPeriodMs * 1000;    
    dev_timer_setvalue(TIMER_TP_DET,uiPeriodUs);
}

/**
 * @brief  打印机定时器中断
 */
void PRT_TP_IRQHandler(void *data)
{    
    pt_timer_stop();        
    TPPaperSNSDetect_interrupt(0);
    TPISRProc();   
}

/**
 * @brief  打印纸及温度检测定时器中断
 */
void pt_DET_IRQHandler(void *data)
{
    unsigned char ch=0;
    
    TPPaperSNSDetect(ch);
    pt_detect_timer_set_period(1000);
    pt_detect_timer_start();
}

/**
 * @brief  打印机定时器初始化
 */
void pt_timer_init(void)
{
    str_timer_cfg_t t_cfg;

    t_cfg.m_reload = FALSE;    
    t_cfg.m_tus = 800;
   
    pt_timer_stop();     
    dev_timer_request(TIMER_TP,t_cfg,PRT_TP_IRQHandler,NULL);    
    dev_timer_int_enable(TIMER_TP);        
}

/**
 * @brief  打印机温度检测
 * @retval 检测温度
 */
uint32_t TPHTemperatureADTest(void)
{
    u16 temp;
    u16 adbuf[5];
    uint32_t tResult=0;
    int16_t cTemperature = 0;        

    dev_adc_open(DEV_ADC_PTR_TM);
    if(dev_adc_get_value(DEV_ADC_PTR_TM, adbuf, 3) < 0)
    {
    	dev_adc_close(DEV_ADC_PTR_TM);
        return DEVSTATUS_ERR_FAIL;
    }    
    dev_adc_close(DEV_ADC_PTR_TM);
	
    temp = adbuf[0] + adbuf[1] + adbuf[2];
    temp = temp/3;
  
    dev_debug_printf("---- adcaa = %d  ---- \r\n",temp);        

    tResult = (temp*188*2)/0xFFF;
    dev_debug_printf("---- V = %d  ---- \r\n",tResult);
    
    tResult = PrinterVolToR(tResult, 300, 200, 0);
    dev_debug_printf("---- R = %d  ---- \r\n",tResult);   
    
    cTemperature = TranRtoDegree(tResult); 
	dev_debug_printf("ct=%d\r\n", cTemperature);  
    
    return cTemperature;    
}

#endif

