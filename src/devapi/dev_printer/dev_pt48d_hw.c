
#include "devglobal.h"
#include "sdk/mhscpu_sdk.h"

#if(PRINTER_EXIST==1)
#include "dev_pt48d_hw.h"
#include "dev_pt48d_detect.h"
#include "dev_pt48d.h"
//#define TEST_GPIO 

#if 0
/*! @brief pt48d printer Pin names */
enum _pt48dPin_pinNames
{
        pinPRT_MOT_PWR              = GPIO_MAKE_PIN(GPIOC_IDX, 0U),
        pinPRT_MOT_PHASE_1A  = GPIO_MAKE_PIN(GPIOC_IDX, 1U),
        pinPRT_MOT_PHASE_2A  = GPIO_MAKE_PIN(GPIOC_IDX, 2U),
        pinPRT_MOT_PHASE_1B  = GPIO_MAKE_PIN(GPIOC_IDX, 3U),
        pinPRT_MOT_PHASE_2B  = GPIO_MAKE_PIN(GPIOC_IDX, 4U),
        
        pinPRT_PAPER_DET          = GPIO_MAKE_PIN(GPIOB_IDX, 0U),   //打印低检测脚
        pinPRT_TM                             = GPIO_MAKE_PIN(GPIOB_IDX, 1U),  //打印机温度脚
        
        pinPRT_LAT                           = GPIO_MAKE_PIN(GPIOB_IDX, 10U), 
        pinPRT_CLK                          = GPIO_MAKE_PIN(GPIOB_IDX, 11U),
        
        pinPRT_DI                               = GPIO_MAKE_PIN(GPIOB_IDX, 16U),
        pinPRT_STB1                        = GPIO_MAKE_PIN(GPIOB_IDX, 18U),
        pinPRT_POWER                   = GPIO_MAKE_PIN(GPIOB_IDX, 19U),
};
#endif
static volatile u8 g_printf_prn_power_flg=0;
static volatile u8 g_printf_moto_flg=0;
static u32 g_print_moto_timerid=0;
int print_nopaper_count = 0;

//打印机驱动电压(大电流)
#define GPIO_PRT_MOT_PWR                        GPIO_PIN_NONE//??pengxuebin,20180825//GPIO_PIN_PTC6

#define GPIO_PRT_MOT_PHASE_1A                   GPIO_PIN_PTD2//GPIO_PIN_PTA7

#define GPIO_PRT_MOT_PHASE_2A                      GPIO_PIN_PTD3//GPIO_PIN_PTA11
 
//#define GPIO_PRT_MOT_PHASE_1B                      GPIO_PIN_PTB6
//#define GPIO_PRT_MOT_PHASE_2B                      GPIO_PIN_PTB7

#define GPIO_PRT_MOT_PHASE_1B                   GPIO_PIN_PTE0//GPIO_PIN_PTB7
#define GPIO_PRT_MOT_PHASE_2B                   GPIO_PIN_PTD11//GPIO_PIN_PTB6



#define GPIO_PRT_PAPER_DET                      GPIO_PIN_PTD6//GPIO_PIN_PTC3

#define GPIO_PRT_TM_DET                         GPIO_PIN_PTC4//GPIO_PIN_PTC4

#define GPIO_PRT_LAT                            GPIO_PIN_PTD1 //GPIO_PIN_PTA6

//#define GPIO_PRT_CLK                                          GPIO_PIN_PTB8

//#define GPIO_PRT_DI                                              GPIO_PIN_PTB10

#define GPIO_PRT_STB1                           GPIO_PIN_PTB1//GPIO_PIN_PTC7

//逻辑电压
#define GPIO_PRT_POWER                          GPIO_PIN_PTD7//GPIO_PIN_PTC8

#define GPIO_PRT_SLP                            GPIO_PIN_PTA10//GPIO_PIN_PTB11

//T3与T1打印不通的管脚定义
#define GPIO_PRT_MOT_PWR_T3                     GPIO_PIN_PTB0 //pengxuebin,20190818 T3使用
#define GPIO_PRT_MOT_PHASE_2B_T3                GPIO_PIN_PTB2  //pengxuebin,20190818 T3使用      
#define GPIO_PRT_POWER_T3                       GPIO_PIN_NONE //pengxuebin,20190818 T3没有该管脚 
#define GPIO_PRT_SLP_T3                         GPIO_PIN_NONE  //pengxuebin,20190818 T3使用      

static iomux_pin_name_t  g_prt_mot_pwr_pin; 
static iomux_pin_name_t  g_prt_mot_phase2B_pin;         //MOTB#
static iomux_pin_name_t  g_prt_power_pin;               //打印逻辑电源控制
static iomux_pin_name_t  g_prt_slp_pin;                 
static u8 g_prt_slp_flg;            //bit0表示打印机，bit1表示非接，对应为1表示打开,0表示关闭
static u8 g_prt_existflg=0;         //记录是否有打印机标志：0：没有打印机 1:有打印机
                                    //高4位为打印机控制方式,0：旧打印方式,1:T3打印方式，
#ifdef TEST_GPIO
 iomux_pin_name_t  g_prt_loop_test_pin; 
iomux_pin_name_t  g_prt_line_test_pin;
iomux_pin_name_t  g_prt_hdl_test_pin; 
iomux_pin_name_t  g_prt_while_test_pin; 

#endif


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

u8 pt_get_exist(void)
{
    return g_prt_existflg;
}
//**********************************************************************************************************
//** 函数名称 ：void pt_gpio_init(void)
//** 函数功能 ：
//** 入口参数 ：
//** 出口参数 ：无
//** 返    回 : 无
//** 备    注 :   
//**********************************************************************************************************
void pt_gpio_init(void)
{
    s32 ret;
    u8 machineid;
    
  #if 0
    machineid = dev_misc_getmachinetypeid();
    if(machineid == MACHINE_TYPE_ID_T1)
  #endif
    {
        g_prt_mot_pwr_pin = GPIO_PRT_MOT_PWR; 
        g_prt_mot_phase2B_pin = GPIO_PRT_MOT_PHASE_2B;         //MOTB#
        g_prt_power_pin = GPIO_PRT_POWER;               //打印逻辑电源控制
        g_prt_slp_pin = GPIO_PRT_SLP;  
        g_prt_existflg = 1;

#ifdef TEST_GPIO
        g_prt_loop_test_pin =  GPIO_PIN_PTH0; 
        g_prt_line_test_pin =  GPIO_PIN_PTH1;
        g_prt_hdl_test_pin  =  GPIO_PIN_PTH2 ; 
        g_prt_while_test_pin = GPIO_PIN_PTH3;
#endif
    }
  #if 0  
    else 
    if((machineid == MACHINE_TYPE_ID_T3)||(machineid == MACHINE_TYPE_ID_T1))
    {
        g_prt_mot_pwr_pin = GPIO_PRT_MOT_PWR_T3; 
        g_prt_mot_phase2B_pin = GPIO_PRT_MOT_PHASE_2B_T3;         //MOTB#
        g_prt_power_pin = GPIO_PRT_POWER_T3;               //打印逻辑电源控制
        g_prt_slp_pin = GPIO_PRT_SLP_T3;  
        g_prt_existflg = 0x11;
    }
    else
    {
        //其它机型不支持打印
        g_prt_mot_pwr_pin = GPIO_PIN_NONE; 
        g_prt_mot_phase2B_pin = GPIO_PIN_NONE;         //MOTB#
        g_prt_power_pin = GPIO_PIN_NONE;               //打印逻辑电源控制
        g_prt_slp_pin = GPIO_PIN_NONE;  
        g_prt_existflg = 0;
        return;
    }   
  #endif
#ifdef TEST_GPIO

    dev_gpio_config_mux(g_prt_loop_test_pin, MUX_CONFIG_GPIO);   
    dev_gpio_set_pad(g_prt_loop_test_pin, PAD_CTL_PULL_UP);
    dev_gpio_direction_output(g_prt_loop_test_pin, 0);

    dev_gpio_config_mux(g_prt_line_test_pin, MUX_CONFIG_GPIO);   
    dev_gpio_set_pad(g_prt_line_test_pin, PAD_CTL_PULL_UP);
    dev_gpio_direction_output(g_prt_line_test_pin, 0);

    dev_gpio_config_mux(g_prt_hdl_test_pin, MUX_CONFIG_GPIO);   
    dev_gpio_set_pad(g_prt_hdl_test_pin, PAD_CTL_PULL_UP);
    dev_gpio_direction_output(g_prt_hdl_test_pin, 0);

    dev_gpio_config_mux(g_prt_while_test_pin, MUX_CONFIG_GPIO);   
    dev_gpio_set_pad(g_prt_while_test_pin, PAD_CTL_PULL_UP);
    dev_gpio_direction_output(g_prt_while_test_pin, 0);
#endif
    
  
    //配置为GPIO口输出0
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

  #if 1
    dev_gpio_config_mux(g_prt_power_pin, MUX_CONFIG_GPIO);   
    dev_gpio_set_pad(g_prt_power_pin, PAD_CTL_PULL_UP);
    dev_gpio_direction_output(g_prt_power_pin, 1);
    g_printf_prn_power_flg = 0;
  #endif  

    dev_gpio_config_mux(g_prt_slp_pin, MUX_CONFIG_GPIO);   
    dev_gpio_set_pad(g_prt_slp_pin, PAD_CTL_PULL_UP);
    if((pt_get_exist()&0xF0)==0x10)
    {
        dev_gpio_direction_output(g_prt_slp_pin, 0);    //打开3V
        g_prt_slp_flg = 0;
    }
    else
    {
        dev_gpio_direction_output(g_prt_slp_pin, 1);
    }
    g_printf_moto_flg = 0;    
  
    //输入
    dev_gpio_config_mux(GPIO_PRT_PAPER_DET, MUX_CONFIG_GPIO);   
    dev_gpio_set_pad(GPIO_PRT_PAPER_DET, PAD_CTL_PULL_NONE);
    dev_gpio_direction_input(GPIO_PRT_PAPER_DET);     

    //dev_gpio_config_mux(GPIO_PRT_TM_DET, MUX_CONFIG_GPIO);   
    //dev_gpio_set_pad(GPIO_PRT_TM_DET, PAD_CTL_PULL_NONE);
    //dev_gpio_direction_input(GPIO_PRT_TM_DET);
    
}
#define PRT_SPI_MOSI    GPIO_PIN_PTA8
void pt_ctl_slp(u8 type, u8 flg)
{
	dev_gpio_set_value(g_prt_slp_pin, flg);
	return;

/*
    if((pt_get_exist()&0xF0)==0x10)
    {
#if 1
        if(flg == 0)
        {
            g_prt_slp_flg &= ~(1<<type);
            if(g_prt_slp_flg == 0)
            {
                dev_gpio_set_value(g_prt_slp_pin, 1);
            }
        }
        else
        {
            if(g_prt_slp_flg == 0)
            {
                dev_gpio_set_value(g_prt_slp_pin, 0);
            }
            g_prt_slp_flg |= (1<<type);
        }
#endif
    }
		*/
}
void pt_ctl_slp_sleep(u8 flg)
{
    if((pt_get_exist()&0xF0)==0x10)
    {
        if(flg == 1)
        {   //进入SLEEP
            dev_gpio_set_value(g_prt_slp_pin, 1);
            dev_gpio_config_mux(PRT_SPI_MOSI, MUX_CONFIG_GPIO);
            dev_gpio_set_pad(PRT_SPI_MOSI, PAD_CTL_PULL_NONE);
            dev_gpio_direction_output(PRT_SPI_MOSI, 0);
        }
        else
        {
            //进入SLEEP
            dev_gpio_config_mux(PRT_SPI_MOSI, MUX_CONFIG_ALT0);
            dev_gpio_set_pad(PRT_SPI_MOSI, PAD_CTL_PULL_UP);
            dev_gpio_set_value(g_prt_slp_pin, 0);
        }
    }
}

static u32 g_pt_line = 0;
void pt_printline_count(void)
{
    g_pt_line++;
}

void pt_printline_clear(void)
{
    g_pt_line = 0;
}

//返回打印长度，单位：mm
u32 pt_get_printline_len(void)
{
    u32 len;

    len =g_pt_line * 125 / 2000;    //全步，每两点走一步，每步0.125mm
    return len;
}

void icc_open_power()
{
    //暂不做IC卡兼容
    SYSCTRL->PHER_CTRL &= (~BIT(20));
}
void icc_close_power()
{
    //暂不做IC卡兼容
    SYSCTRL->PHER_CTRL |= BIT(20);
}

void pt_sleep(void)
{
    if(pt_get_exist())
    {
        dev_debug_printf("---- %s ---- %d -------\r\n",__func__, __LINE__);
//        LATCH_HIGH();
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
        #ifdef MACHINE_P7
            //icc_close_power();
        #endif
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
          #if 0
            if(dev_user_querrytimer(g_print_moto_timerid, 2))
            {
                return pt_check_paper();
            }
            else
            {   //在2ms范围内,强制认为有纸
                return 0;//
            }
          #endif
//          pt_check_paper();
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
  #if 1 
    if(g_printf_prn_power_flg == 0)
    {
   //     dev_gpio_set_value(g_prt_power_pin,0);
   	dev_gpio_set_value(g_prt_power_pin,1);
        g_printf_prn_power_flg = 1;
    }
  #endif  
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

//**********************************************************************************************************
//** 函数名称 ：void pt_timer_init(void)
//** 函数功能 ：
//** 入口参数 ：
//** 出口参数 ：无
//** 返    回 : 无
//** 备    注 :   
//**********************************************************************************************************
void pt_timer_init(void)
{
    str_timer_cfg_t t_cfg;

    t_cfg.m_reload = FALSE;//TRUE;//
    
    t_cfg.m_tus = 800;

    pt_timer_stop();
    
    //马达定时器
    dev_timer_request(TIMER_TP,t_cfg,PRT_TP_IRQHandler,NULL);
    
    dev_timer_int_enable(TIMER_TP);
    
       //定时检测纸
    //dev_timer_request(TIMER_TP_DET,t_cfg,pt_DET_IRQHandler,NULL);
    
    //pt_detect_timer_set_period(1000);        // 1s
    
    //pt_detect_timer_start();
        
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

