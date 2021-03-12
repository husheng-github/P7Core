
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

static iomux_pin_name_t  g_prt_mot_pwr_pin; 
static iomux_pin_name_t  g_prt_mot_phase2B_pin;   //MOTB#
static iomux_pin_name_t  g_prt_power_pin;         //打印逻辑电源控制
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
        dev_debug_printf("---- %s ---- %d -------\r\n",__func__, __LINE__);
        
        STROBE_0_OFF();
        STROBE_1_OFF();

        MOTOR_PHASE_1A_LOW();
        MOTOR_PHASE_1B_LOW();
        MOTOR_PHASE_2A_LOW();
        MOTOR_PHASE_2B_LOW();
        
        MOTOR_PWR_OFF();
        LATCH_LOW();
        PRN_POWER_DISCHARGE();
        pt_ctl_slp(0, 1);
        dev_spi_close(SPI_DEV_PRINT);
        dev_adc_close(DEV_ADC_PTR_TM);
        dev_debug_printf("print line: %d  \r\n",pt_get_printline_len());       
    }
}

void pt_resume(void)
{
    uint32_t i;

    if(pt_get_exist())
    {
        dev_debug_printf("---- %s ---- %d -------\r\n",__func__, __LINE__);
        pt48d_dev_init();
        
        pt_ctl_slp(0, 0);
        dev_spi_open(SPI_DEV_PRINT);
        dev_adc_open(DEV_ADC_PTR_TM);

        PRN_POWER_CHARGE();
        LATCH_HIGH();
        SetDesity();

        //for(i=0;i<50000;i++);
        //dev_user_delay_ms(20);
        print_nopaper_count = 0;
        dev_printer_setpaper_state(PT_STATUS_IDLE);

        TPPaperSNSDetect(0);
        pt_printline_clear();

    #ifndef TRENDIT_BOOT
        Queueinit();    //Add by caishaojiang, 解决报缺纸后，下一单打印之前一单的后部分内容
    #endif

    #ifdef MACHINE_P7
        //icc_open_power();
    #endif
    }
}

void CHECKPAPER_LED_ON(void)
{
     if((pt_get_exist()&0xf0) == 0x10)
    {
        dev_gpio_set_value(g_prt_mot_pwr_pin,1);
        dev_user_delay_us(1000);
//dev_debug_printf("LED ON\r\n");
    }
}

void CHECKPAPER_LED_OFF(void)
{
    if((pt_get_exist()&0xf0) == 0x10)
    {
        dev_gpio_set_value(g_prt_mot_pwr_pin,0);
//dev_debug_printf("LED OFF\r\n");
    }
}

u8 pt_check_paper(void)
{
    s32 i;
    s32 ret = 0;

    for(i=0; i<3; i++)
    {
        //if(1 == dev_gpio_get_value(GPIO_PRT_PAPER_DET))
        if(0 == dev_gpio_get_value(GPIO_PRT_PAPER_DET))
        {
            //有纸,
            break;
        }
        dev_user_delay_us(5);
    }
    if(i>=3)
    {
        //无纸
 //       dev_debug_printf("no paper \r\n");
        print_nopaper_count ++;
        if(print_nopaper_count>=PAPER_SNSDETECT_MAX)
        {
//            dev_debug_printf("--------------------------------------------no paper \r\n");
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        //有纸
 //       dev_debug_printf("has paper \r\n");
        print_nopaper_count = 0;
        return 0;
    }
}
uint8_t pt_get_paper_status(void)
{
    s32 ret;
    s32 i;
    if(pt_get_exist())
    {
        if(g_printf_moto_flg)
        {
            // pt_check_paper();
        }
        else
        {
            CHECKPAPER_LED_ON();
          #if 0
            ret = pt_check_paper();
            if(!g_printf_moto_flg)
            {
                CHECKPAPER_LED_OFF();
            }
            return ret;
          #else
            for(i=0; i<PAPER_SNSDETECT_MAX+2; i++)
            {
                pt_check_paper();
                dev_user_delay_us(500);
            }
            if(!g_printf_moto_flg)
            {
                CHECKPAPER_LED_OFF();
            }
          #endif
        }
        
        if(print_nopaper_count>=PAPER_SNSDETECT_MAX)
        {
            return 1;
        }
        else
        {
            return 0;       //有纸
        }
    }
    else
    {
        return 0;
    }
    
}

void LATCH_HIGH(void) 
{
    if(pt_get_exist())
    {
        dev_gpio_set_value(GPIO_PRT_LAT,1);
    }
}

void LATCH_LOW(void)                     
{    
    if(pt_get_exist())
    {
        dev_gpio_set_value(GPIO_PRT_LAT,0);
    }
    
}

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

void MOTOR_PWR_OFF(void)         
{
    if(pt_get_exist())
    {
        g_printf_moto_flg = 0;
        if((pt_get_exist()&0xF0)==0x10)
        {
//            dev_gpio_set_value(g_prt_slp_pin,1);
        }
        else
        {
 //           dev_gpio_set_value(g_prt_slp_pin,1);
        }
        dev_gpio_set_value(g_prt_mot_pwr_pin,1);
    }
    
}

void MOTOR_PHASE_1A_HIGH(void)  
{
    if(pt_get_exist())
    {
        dev_gpio_set_value(GPIO_PRT_MOT_PHASE_1A,1);
    }
}

void MOTOR_PHASE_1A_LOW(void)   
{
    if(pt_get_exist())
    {
        dev_gpio_set_value(GPIO_PRT_MOT_PHASE_1A,0);
    }
}

void MOTOR_PHASE_1B_HIGH(void)  
{
    if(pt_get_exist())
    {
        dev_gpio_set_value(GPIO_PRT_MOT_PHASE_1B,1);
    }
}

void MOTOR_PHASE_1B_LOW(void)   
{
    if(pt_get_exist())
    {
        dev_gpio_set_value(GPIO_PRT_MOT_PHASE_1B,0);
    }
}

void MOTOR_PHASE_2A_HIGH(void)  
{
    if(pt_get_exist())
    {
        dev_gpio_set_value(GPIO_PRT_MOT_PHASE_2A,1);
    }
    
}

void MOTOR_PHASE_2A_LOW(void)   
{
    if(pt_get_exist())
    {
        dev_gpio_set_value(GPIO_PRT_MOT_PHASE_2A,0);
    }
    
}

void MOTOR_PHASE_2B_HIGH(void)  
{
    if(pt_get_exist())
    {
        dev_gpio_set_value(g_prt_mot_phase2B_pin,1);
    }
}

void MOTOR_PHASE_2B_LOW(void)   
{
    if(pt_get_exist())
    {
        dev_gpio_set_value(g_prt_mot_phase2B_pin,0);
    }
}

void PRN_POWER_CHARGE(void) 
{
    if(g_printf_prn_power_flg == 0)
    {
   	    dev_gpio_set_value(g_prt_power_pin,1);
        g_printf_prn_power_flg = 1;
    }
}

void PRN_POWER_DISCHARGE(void) 
{
  #if 1
    //dev_gpio_set_value(g_prt_power_pin,1);
    dev_gpio_set_value(g_prt_power_pin,0);
    g_printf_prn_power_flg = 0;
  #endif  
}

void STROBE_0_ON(void)     
{
    
}

void STROBE_0_OFF(void)    
{
    
}

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

void pt_spi_init(void)
{
    if(pt_get_exist())
    {
        dev_spi_open(SPI_DEV_PRINT);
    }
}


 void TPDataShiftOut(uint8_t *p, uint16_t len)
{       
    #if 0
    s32 i = 0;

    for(i = 0; i < len; i++)
    {
        dev_debug_printf("%x ",p[i]);
    }
    #endif
    dev_spi_master_transceive_polling(SPI_DEV_PRINT,p,NULL,len);
}

//**********************************************************************************************************
//** 函数名称 ：void pt_timer_start(void)
//** 函数功能 ：马达步进时钟
//** 入口参数 ：
//** 出口参数 ：无
//** 返    回 : 无
//** 备    注 :   
//**********************************************************************************************************
void pt_timer_start(void)
{
        //DBG_STR("####  pt_timer_start()\r\n");
        
        dev_timer_run(TIMER_TP);
}


//**********************************************************************************************************
//** 函数名称 ：void pt_timer_stop(void)
//** 函数功能 ：马达步进时钟
//** 入口参数 ：
//** 出口参数 ：无
//** 返    回 : 无
//** 备    注 :   
//**********************************************************************************************************
void pt_timer_stop(void)
{
        //DBG_STR("####  pt_timer_stop()\r\n");
        
        dev_timer_stop(TIMER_TP);
}

//**********************************************************************************************************
//** 函数名称 ：void pt_timer_set_periodUs(uint32_ t uiPeriodUs)
//** 函数功能 ：马达步进时钟
//** 入口参数 ：
//** 出口参数 ：无
//** 返    回 : 无
//** 备    注 :   
//**********************************************************************************************************
void pt_timer_set_periodUs(uint32_t uiPeriodUs)
{
        dev_timer_setvalue(TIMER_TP,uiPeriodUs);
//dev_debug_printf("t=%d\r\n", uiPeriodUs);       
        
    //    char test_buf[32] = {0};
    //        sprintf(test_buf,"tm = %d\r\n",uiPeriodUs);
    //     uart_send_str(test_buf);

    //DBG_STR("######             ### pt_timer_set_periodUs:%ld \r\n",uiPeriodUs);
}

//**********************************************************************************************************
//** 函数名称 ：void pt_timer_start(void)
//** 函数功能 ：定时检测时钟
//** 入口参数 ：
//** 出口参数 ：无
//** 返    回 : 无
//** 备    注 :   
//**********************************************************************************************************
void pt_detect_timer_start(void)
{
        dev_timer_run(TIMER_TP_DET);
}
//**********************************************************************************************************
//** 函数名称 ：void pt_detect_timer_stop(void)
//** 函数功能 ：定时检测时钟
//** 入口参数 ：
//** 出口参数 ：无
//** 返    回 : 无
//** 备    注 :   
//**********************************************************************************************************
void pt_detect_timer_stop(void)
{
        dev_timer_stop(TIMER_TP_DET);
}

//**********************************************************************************************************
//** 函数名称 ：void pt_detect_timer_set_period(uint32_ t uiPeriodMs)
//** 函数功能 ：定时检测时钟 ms
//** 入口参数 ：
//** 出口参数 ：无
//** 返    回 : 无
//** 备    注 :   
//**********************************************************************************************************
void pt_detect_timer_set_period(uint32_t uiPeriodMs)
{
        uint32_t uiPeriodUs ;

        uiPeriodUs = uiPeriodMs * 1000;
        
        dev_timer_setvalue(TIMER_TP_DET,uiPeriodUs);
}

void PRT_TP_IRQHandler(void *data)
{      
#ifdef TEST_GPIO
    dev_gpio_direction_output(g_prt_hdl_test_pin, 1);
#endif
    pt_timer_stop();
    
//    PRN_POWER_DISCHARGE();
        
//    PRN_POWER_CHARGE();
        
    TPPaperSNSDetect_interrupt(0);

    TPISRProc();     

#ifdef TEST_GPIO
    dev_gpio_direction_output(g_prt_hdl_test_pin, 0);
#endif
//dev_debug_printf("3"); 
}

void pt_DET_IRQHandler(void *data)
{
        unsigned char ch=0;
        
        TPPaperSNSDetect(ch);
        pt_detect_timer_set_period(1000);

        pt_detect_timer_start();
//dev_debug_printf("4");
}

/**
 * @brief  马达定时器初始化
 */
void pt_timer_init(void)
{
    str_timer_cfg_t t_cfg;

    t_cfg.m_reload = FALSE;    
    t_cfg.m_tus = 800;

    //马达定时器
    pt_timer_stop();     
    dev_timer_request(TIMER_TP,t_cfg,PRT_TP_IRQHandler,NULL);
    
    dev_timer_int_enable(TIMER_TP);        
}

uint32_t TPHTemperatureADTest(void)
{
    uint8_t i;
    u16 temp;
    u16 adbuf[5];
    uint32_t tResult=0;
    int16_t cTemperature = 0;

#if 1
{
        #if 0
            temp = dev_adc_get_arrage_value(DEV_ADC_PTR_TM, 50);
        #else

        dev_adc_open(DEV_ADC_PTR_TM);



        if(dev_adc_get_value(DEV_ADC_PTR_TM, adbuf, 3) < 0)
        {
        	dev_adc_close(DEV_ADC_PTR_TM);
            return DEVSTATUS_ERR_FAIL;
        }

        
        dev_adc_close(DEV_ADC_PTR_TM);
		
        temp = adbuf[0] + adbuf[1] + adbuf[2];

        temp = temp/3;
        #endif
        dev_debug_printf("---- adcaa = %d  ---- \r\n",temp);

        
        #if 1 

        //tResult = (temp*180)/0xFFF;
        tResult = (temp*188*2)/0xFFF;
        dev_debug_printf("---- V = %d  ---- \r\n",tResult);


        //tResult = PrinterVolToR(tResult, 180, 300, 0);
        tResult = PrinterVolToR(tResult, 300, 200, 0);
        dev_debug_printf("---- R = %d  ---- \r\n",tResult);
        

        
        
        cTemperature = TranRtoDegree(tResult); // 10位ADC
      #endif
        //cTemperature = 16;
		dev_debug_printf("ct=%d\r\n", cTemperature);        
        return cTemperature;
    }

#else
 /*   for(i=0;i<5;i++)
    {
        ad = (uint32_t)dev_adc_get_value(DEV_ADC_PTR_TM); 

        temp += ad;
    }
*/
    if(dev_adc_get_value(DEV_ADC_PTR_TM, adbuf, 5) < 0)
    {
        return DEVSTATUS_ERR_FAIL;
    }
    temp = adbuf[0] + adbuf[1]+ adbuf[2]+ adbuf[3]+ adbuf[4];
    
    tResult = (temp*120)/(5*0xFFF);
    
    dev_debug_printf("ad valule:%d \r\n",(temp/5));
        
    dev_debug_printf("voltage:%d \r\n",tResult);

    tResult = TranVtoR((temp/5),0x0FFF,300,0);

    dev_debug_printf("resistance:%d \r\n",tResult);
    
    return tResult;
#endif
}

#endif

