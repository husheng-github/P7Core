/**
 * @file dev_pt48d_hw.c
 * @brief ��ӡ��Ӳ����ʼ������
 * @since 2021.3.20  ��ʥ�������
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
static u8 g_prt_slp_flg;            //bit0��ʾ��ӡ����bit1��ʾ�ǽӣ���ӦΪ1��ʾ��,0��ʾ�ر�
static u8 g_prt_existflg=0;         //��¼�Ƿ��д�ӡ����־��0��û�д�ӡ�� 1:�д�ӡ��
                                    //��4λΪ��ӡ�����Ʒ�ʽ,0���ɴ�ӡ��ʽ,1:T3��ӡ��ʽ

static iomux_pin_name_t  g_prt_mot_pwr_pin;  //�� 
static iomux_pin_name_t  g_prt_mot_phase2B_pin;   
static iomux_pin_name_t  g_prt_power_pin;  //��ӡ�߼���Դ����
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
 * @brief  �жϴ�ӡ���Ƿ����
 * @retval 0-������  1-����
 * @retval ��4λ��ʾ��ӡ�����Ʒ�ʽ��0-�ɴ�ӡ��ʽ  1-T3��ӡ��ʽ
 */
u8 pt_get_exist(void)
{
    return g_prt_existflg;
}

/**
 * @brief  ��ӡ���ܽų�ʼ��
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

    //���Ѵ�ӡ��
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
 * @brief  ��ӡ�����߿���
 * @param [in]  flg:1-��������  0-�˳�����
 */
void pt_ctl_slp(u8 type, u8 flg)
{
	dev_gpio_set_value(g_prt_slp_pin, flg);
}

/**
 * @brief  ��ӡ�����߿���
 * @note  ������
 * @note  T3��ӡ��ʽ
 * @param [in]  flg:1-��������  0-�˳�����
 */
void pt_ctl_slp_sleep(u8 flg)
{
    //T3��ӡ��ʽ
    if((pt_get_exist()&0xF0)==0x10)
    {
        if(flg == 1)   //��������
        {          
            dev_gpio_set_value(g_prt_slp_pin, 1);

            //PRT_SPI_MOSI����ΪGPIO 
            dev_gpio_config_mux(PRT_SPI_MOSI, MUX_CONFIG_GPIO);
            dev_gpio_set_pad(PRT_SPI_MOSI, PAD_CTL_PULL_NONE);
            dev_gpio_direction_output(PRT_SPI_MOSI, 0);
        }
        else  //�˳�����
        {  
            //���û�PRT_SPI_MOSIģʽ
            dev_gpio_config_mux(PRT_SPI_MOSI, MUX_CONFIG_ALT0);
            dev_gpio_set_pad(PRT_SPI_MOSI, PAD_CTL_PULL_UP);
            
            dev_gpio_set_value(g_prt_slp_pin, 0);
        }
    }
}

/**
 * @brief  �������
 */
void pt_printline_count(void)
{
    g_pt_line++;
}

/**
 * @brief  �������
 */
void pt_printline_clear(void)
{
    g_pt_line = 0;
}

/**
 * @brief  ��ȡ��ӡ����
 * @note  ��λ��mm
 * @retval ��ӡ����
 */
u32 pt_get_printline_len(void)
{
    u32 len;

    len =g_pt_line * 125 / 2000;  //ȫ����ÿ������һ����ÿ��0.125mm
    return len;
}

/**
 * @brief  ��IC����Դ 
 * @note  Ӧ��Ĭ���Ǹߵ�ƽ
 */
void icc_open_power()
{
    SYSCTRL->PHER_CTRL &= (~BIT(20));  //SCI0 VCCEN�ź���Ч��ƽѡ��,�ߵ�ƽ��Ч
}

/**
 * @brief  �ر�IC����Դ
 */
void icc_close_power()
{
    SYSCTRL->PHER_CTRL |= BIT(20);  //SCI0 VCCEN�ź���Ч��ƽѡ��,�͵�ƽ��Ч
}

/**
 * @brief  ��ӡ������
 */
void pt_sleep(void)
{
    if(pt_get_exist())
    {
        //������
        STROBE_0_OFF();
        STROBE_1_OFF();

        //����������ƣ���ת
        MOTOR_PHASE_1A_LOW();
        MOTOR_PHASE_1B_LOW();
        MOTOR_PHASE_2A_LOW();
        MOTOR_PHASE_2B_LOW();

        //��ӡ���µ�
        MOTOR_PWR_OFF();
        LATCH_LOW();
        PRN_POWER_DISCHARGE();

        //��������
        pt_ctl_slp(0, 1);          
        dev_spi_close(SPI_DEV_PRINT);
        dev_adc_close(DEV_ADC_PTR_TM);        
    }
}

/**
 * @brief  ���߻���
 */
void pt_resume(void)
{
    if(pt_get_exist())
    {
        //���߻���
        pt48d_dev_init();        
        pt_ctl_slp(0, 0);
        dev_spi_open(SPI_DEV_PRINT);
        dev_adc_open(DEV_ADC_PTR_TM);

        //�ϵ�
        PRN_POWER_CHARGE();
        LATCH_HIGH();
        
        SetDesity();       
        print_nopaper_count = 0;
        dev_printer_setpaper_state(PT_STATUS_IDLE);
        TPPaperSNSDetect(0);
        pt_printline_clear();

      #ifndef TRENDIT_BOOT
        Queueinit();  //��ʼ������
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
 * @brief  ����Ƿ���ֽ
 * @retval 0:��ֽ  1:ûֽ
 */
u8 pt_check_paper(void)
{
    s32 i;
    s32 ret = 0;

    //����Ƿ���ֽ
    for(i=0; i<3; i++)
    {
        if(0 == dev_gpio_get_value(GPIO_PRT_PAPER_DET))
        {            
            break;
        }
        dev_user_delay_us(5);
    }

    //��ֽ
    if(i>=3)
    {
        //��ֽ
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
    else  //��ֽ
    {        
        print_nopaper_count = 0;
        return 0;
    }
}


/**
 * @brief  ����Ƿ���ֽ
 * @retval 0:��ֽ  1:ûֽ
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
 * @brief  ��ӡ����Դ��������
 */
void LATCH_HIGH(void) 
{
    if(pt_get_exist())
    {
        dev_gpio_set_value(GPIO_PRT_LAT,1);
    }
}

/**
 * @brief  ��ӡ����Դ��������
 */
void LATCH_LOW(void)                     
{    
    if(pt_get_exist())
    {
        dev_gpio_set_value(GPIO_PRT_LAT,0);
    }    
}

/**
 * @brief  ��ӡ���ϵ�
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
 * @brief  ��ӡ���µ�
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
 * @brief  1A���ƹܽ�����
 */
void MOTOR_PHASE_1A_HIGH(void)  
{
    if(pt_get_exist())
    {
        dev_gpio_set_value(GPIO_PRT_MOT_PHASE_1A,1);
    }
}

/**
 * @brief  1A���ƹܽ�����
 */
void MOTOR_PHASE_1A_LOW(void)   
{
    if(pt_get_exist())
    {
        dev_gpio_set_value(GPIO_PRT_MOT_PHASE_1A,0);
    }
}

/**
 * @brief  1B���ƹܽ�����
 */
void MOTOR_PHASE_1B_HIGH(void)  
{
    if(pt_get_exist())
    {
        dev_gpio_set_value(GPIO_PRT_MOT_PHASE_1B,1);
    }
}

/**
 * @brief  1B���ƹܽ�����
 */
void MOTOR_PHASE_1B_LOW(void)   
{
    if(pt_get_exist())
    {
        dev_gpio_set_value(GPIO_PRT_MOT_PHASE_1B,0);
    }
}

/**
 * @brief  2A���ƹܽ�����
 */
void MOTOR_PHASE_2A_HIGH(void)  
{
    if(pt_get_exist())
    {
        dev_gpio_set_value(GPIO_PRT_MOT_PHASE_2A,1);
    }    
}

/**
 * @brief  2A���ƹܽ�����
 */
void MOTOR_PHASE_2A_LOW(void)   
{
    if(pt_get_exist())
    {
        dev_gpio_set_value(GPIO_PRT_MOT_PHASE_2A,0);
    }    
}

/**
 * @brief  2B���ƹܽ�����
 */
void MOTOR_PHASE_2B_HIGH(void)  
{
    if(pt_get_exist())
    {
        dev_gpio_set_value(g_prt_mot_phase2B_pin,1);
    }
}

/**
 * @brief  2B���ƹܽ�����
 */
void MOTOR_PHASE_2B_LOW(void)   
{
    if(pt_get_exist())
    {
        dev_gpio_set_value(g_prt_mot_phase2B_pin,0);
    }
}

/**
 * @brief  ��ӡ���ϵ�
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
 * @brief  ��ӡ���µ�
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
 * @brief  ���ȿ���
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
 * @brief  ��ӡ��SPI�ӿڳ�ʼ��
 */
void pt_spi_init(void)
{
    if(pt_get_exist())
    {
        dev_spi_open(SPI_DEV_PRINT);
    }
}

/**
 * @brief  ��ӡ����������
 */
void TPDataShiftOut(uint8_t *p, uint16_t len)
{   
    dev_spi_master_transceive_polling(SPI_DEV_PRINT,p,NULL,len);
}

/**
 * @brief  ������ʱ��
 * @note  ��ӡ��ʹ��
 */
void pt_timer_start(void)
{        
    dev_timer_run(TIMER_TP);
}

/**
 * @brief  ֹͣ��ʱ��
 * @note  ��ӡ��ʹ��
 */
void pt_timer_stop(void)
{        
    dev_timer_stop(TIMER_TP);
}

/**
 * @brief  ���ö�ʱʱ��
 * @param [in]  pHuiPeriodUsash:��ʱʱ��
 */
void pt_timer_set_periodUs(uint32_t uiPeriodUs)
{
    dev_timer_setvalue(TIMER_TP,uiPeriodUs);
}

/**
 * @brief  ������ʱ��
 * @note  ��ӡֽ���¶ȼ��ʹ��
 */
void pt_detect_timer_start(void)
{
    dev_timer_run(TIMER_TP_DET);
}

/**
 * @brief  ֹͣ��ʱ��
 * @note  ��ӡֽ���¶ȼ��ʹ��
 */
void pt_detect_timer_stop(void)
{
    dev_timer_stop(TIMER_TP_DET);
}

/**
 * @brief  ���ö�ʱʱ�� 
 * @note  ��ӡֽ���¶ȼ��ʱ��
 * @param [in]  pHuiPeriodUsash:��ʱʱ��
 */
void pt_detect_timer_set_period(uint32_t uiPeriodMs)
{
    uint32_t uiPeriodUs;

    uiPeriodUs = uiPeriodMs * 1000;    
    dev_timer_setvalue(TIMER_TP_DET,uiPeriodUs);
}

/**
 * @brief  ��ӡ����ʱ���ж�
 */
void PRT_TP_IRQHandler(void *data)
{    
    pt_timer_stop();        
    TPPaperSNSDetect_interrupt(0);
    TPISRProc();   
}

/**
 * @brief  ��ӡֽ���¶ȼ�ⶨʱ���ж�
 */
void pt_DET_IRQHandler(void *data)
{
    unsigned char ch=0;
    
    TPPaperSNSDetect(ch);
    pt_detect_timer_set_period(1000);
    pt_detect_timer_start();
}

/**
 * @brief  ��ӡ����ʱ����ʼ��
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
 * @brief  ��ӡ���¶ȼ��
 * @retval ����¶�
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

