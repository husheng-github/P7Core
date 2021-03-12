/********************************Copyright ( c ) ********************************
**                  DAQU 
**                         
**                        
**
**Created By:         
** Created Date:     
** Version:        
** Description:    SYSTEN CONTROL 
****************************************************************************/
#include "devglobal.h"
#include "sdk/mhscpu_sdk.h"

void dev_sys_NVIC_CloseAllIrq(void)
{
    s32 i;

    for(i=0; i<IRQn_MAX; i++)
    {
        NVIC_DisableIRQ((IRQn_Type)i);
    }
        
    NVIC->ICER[0] = 0xFFFFFFFF;  //sxl?2017
    NVIC->ICER[1] = 0xFFFFFFFF;
    NVIC->ICER[2] = 0xFFFFFFFF;
    NVIC->ICER[3] = 0xFFFFFFFF;
    NVIC->ICER[4] = 0xFFFFFFFF;
    NVIC->ICER[5] = 0xFFFFFFFF;
    NVIC->ICER[6] = 0xFFFFFFFF;
    NVIC->ICER[7] = 0xFFFFFFFF;
}
void dev_sys_NVIC_ClearAllIrqPanding(void)
{
    s32 i;

    for(i=0; i<IRQn_MAX; i++)
    {
        NVIC_ClearPendingIRQ((IRQn_Type)i);
    }
}

typedef struct _str_priority_param 
{
    IRQn_Type m_irqn;
    u32      m_priority; 
}str_priority_param;
#define NVIC_PRIORITY_SET_MAX  8      //��Ҫ���ø���
const str_priority_param priority_param_tab[NVIC_PRIORITY_SET_MAX] = 
{
    {SysTick_IRQn, 5},    //���ȼ����
    {UART0_IRQn, 2},
    {TIM0_3_IRQn, 0},
    {TIM0_4_IRQn, 1},   //PSAM��λ����
    {DMA_IRQn,  4},   // sxladd20180619 for mag
    {TIM0_5_IRQn, 5}, 
    {USB_IRQn, 3},    
    {MSR_IRQn, 5},
//    {SPI0_IRQn, 0},        //���ȼ����
};

void dev_sys_NVIC_Configuration(void)
{
    u32 tmpPriority ,tmpPriorityGrouping;
    s32 i;
    

    //���ȼ�����
    NVIC_SetPriorityGrouping(NVIC_PriorityGroup_3);

    
    /* ��ʼ���ж����ȼ���Ĭ��Ϊ(2,0)*/    
    tmpPriorityGrouping = NVIC_GetPriorityGrouping();
    tmpPriority = NVIC_EncodePriority(tmpPriorityGrouping, 3, 0);
    for(i=0; i<IRQn_MAX; i++)
    {
        NVIC_SetPriority((IRQn_Type)i,tmpPriority);
    }
    //���������ȼ�����
    for(i=0; i<NVIC_PRIORITY_SET_MAX; i++)
    {
        tmpPriority = NVIC_EncodePriority(tmpPriorityGrouping, priority_param_tab[i].m_priority, 0);
        NVIC_SetPriority(priority_param_tab[i].m_irqn, tmpPriority);
    }
}
//�����ж����ȼ�
void dev_sys_NVIC_setPriority(IRQn_Type IRQn, u32 PreemptPriority, u32 SubPriority)
{
    u32 tmpPriority ,tmpPriorityGrouping; 
    
    tmpPriorityGrouping = NVIC_GetPriorityGrouping();
    tmpPriority = NVIC_EncodePriority(tmpPriorityGrouping, PreemptPriority, SubPriority);
    NVIC_SetPriority(IRQn, tmpPriority);
}

void dev_systick_ctl(u8 flg)
{
    drv_systick_ctl(flg);
}
#ifdef TRENDIT_CORE
void core_main_check_ovp_status()
{
    u16 adcbuff[5] = 0;
    s32 adclen = 1;
    s32 i = 0;
    u16 ovp_tmp = 0;
    u32 ovp_value = 0;

    dev_adc_open(DEV_ADC_OVP);
    dev_adc_get_value(DEV_ADC_OVP,adcbuff,5);
    dev_adc_close(DEV_ADC_OVP);

    for(i = 0; i < 5; i++)
    {
        ovp_tmp = ovp_tmp + adcbuff[i];
        //dev_debug_printf("i:%d,value:%d\r\n",i,adcbuff[i]);
    }
    ovp_tmp = ovp_tmp/5;
    ovp_value = (ovp_tmp*1880)/0xFFF;
    //dev_debug_printf("ovp_value:%d",ovp_value);
    //if((ovp_value > 1280) || (ovp_value < 750))
    if((ovp_value > 1080))
    {
        //audio_tts.m_text = "�¶��쳣,�ػ�����";
        dev_audio_playcharacters(1, "��ѹ�쳣,�ػ�����");
        while(dev_dac_is_playing())
        {
            
        }
        //dev_power_switch_ctl(0);
        ddi_misc_poweroff();
    }
}
#endif

void dev_sys_init(void)
{
    u8 tmp[32];
    CACHE_InitTypeDef test_aes;


    __disable_fiq();
    __disable_irq();
    (*((uint32_t *)0x40000000)) = (uint32_t)0x00;
    dev_sys_NVIC_CloseAllIrq();

    //ѡ��ʱ��Դ
  #if(MCU_EXT12M_EN==1)
    //�ⲿʱ��Դ
    SYSCTRL_SYSCLKSourceSelect(SELECT_EXT12M);//SELECT_INC12M
  #else
    //�ڲ�ʱ��Դ
    SYSCTRL_SYSCLKSourceSelect(SELECT_INC12M);//SELECT_INC12M
  #endif  
    //����ʱ��
    SYSCTRL_PLLConfig(SYSCTRL_PLL_168MHz); 
    SYSCTRL_PLLDivConfig(SYSCTRL_PLL_Div_None);
    SYSCTRL_HCLKConfig(SYSCTRL_HCLK_Div2); //SYSCTRL_HCLK_Div2  //SYSCTRL_HCLK_Div4 
    SYSCTRL_PCLKConfig(SYSCTRL_PCLK_Div2); //SYSCTRL_PCLK_Div4 

    //QSPI->DEVICE_PARA = (QSPI->DEVICE_PARA & 0xFFFF) | ((SYSCTRL->HCLK_1MS_VAL*2/1000) << 16);      //������볬�͹��Ļ��Ѻ��и�������������

    dev_sys_NVIC_Configuration();
    
    dev_sys_NVIC_ClearAllIrqPanding();
    //open crypt clock 
    SYSCTRL_AHBPeriphClockCmd(SYSCTRL_AHBPeriph_CRYPT, ENABLE);
    SYSCTRL_AHBPeriphResetCmd(SYSCTRL_AHBPeriph_CRYPT, ENABLE);
    mh_crypt_it_clear();
    SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_TRNG, ENABLE);

    //sxl20180611 
    QSPI_SetLatency(0);
    SYSCTRL_AHBPeriphClockCmd(SYSCTRL_AHBPeriph_DMA, ENABLE);
    SYSCTRL_AHBPeriphResetCmd(SYSCTRL_AHBPeriph_DMA, ENABLE);
    SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_BPU,ENABLE); 
    

  #if 1
    //cash
    memset(&test_aes, 0x00, sizeof(test_aes));

    CACHE_CleanAll(CACHE);
    test_aes.aes_enable = DISABLE;
    CACHE_Init(CACHE, &test_aes);
    QSPI_Init(NULL);
  #endif

    dev_user_timer_init();
    __enable_fiq();
    __enable_irq();

    dev_gpio_init();
    dev_timer_init();
//gpio_test();    
    dev_com_init();  
    dev_flash_init();
    fs_system_init();
    dev_misc_init();
    
  #ifdef TRENDIT_CORE
    dev_com_open(PORT_CDCD_NO, MCU_UART_BAUD, 8, 'n', 0, MCU_FLOWCTL_FLG);
    dev_com_open(PORT_UART0_NO, MCU_UART_BAUD, 8, 'n', 0, MCU_FLOWCTL_FLG);
//    dev_user_delay_ms(500);  
  #endif
//    dev_com_open(DEBUG_PORT_NO, MCU_UART_BAUD, 8, 'n', 0, MCU_FLOWCTL_FLG);

//dev_user_delay_ms(10);
dev_debug_printf("%s(%d):MAIN(%s %s)...\r\n", __FUNCTION__, __LINE__, __DATE__, __TIME__);
//    dev_user_delay_ms(100);
  //��ǰ��ADC����,��ȻAD������׼
  dev_adc_init();

  #ifdef TRENDIT_CORE
    icc_open_power();//��Ҫ�ȴ�IC����Դ����Ȼ��ӡ���Ͱ���������
    drv_gprs_io_init();
    dev_wireless_init();

    dev_wifi_init();
  #endif

    
    dev_keypad_open();
    dev_led_open();



    #ifdef TRENDIT_CORE
        dev_audio_open();

        //cgj test
        //core_main_check_ovp_status();
        printer_init();
    #endif

#if 0
    //dev_misc_setmachinetypeid(MACHINE_TYPE_ID_T1);    
    dev_misc_getmachinetypeid();                           //��ȡmachine ID
  #ifdef TRENDIT_BOOT
    sprintf(tmp, "macid=0x%02X\r\n", dev_misc_getmachinetypeid());
    dev_com_write(PORT_UART0_NO, tmp, strlen(tmp));
  #endif
    

    dev_spi_init();
//    dev_gpioex_init();
//MISC_DEBUG("\r\n");
    dev_power_init();
//MISC_DEBUG("\r\n");
  #ifdef TRENDIT_CORE   
//    dev_mag_init();      //��ʼ���ر�mag��Դ
  #endif
    dev_icc_init();
    api_rf_init();
//MISC_DEBUG("\r\n");
    dev_keypad_open();
    dev_audio_open();
//MISC_DEBUG("\r\n");
    dev_lcd_open();
    dev_led_open();
//MISC_DEBUG("\r\n");
//dev_com_open(3, 115200, 8, 'n', 0, 0);
//   uart_test();
    
  #if(PRINTER_EXIST==1)
    printer_init();
  #endif 
//dev_debug_printf("hclk=%d, pclk=%d", (SYSCTRL->HCLK_1MS_VAL), (SYSCTRL->PCLK_1MS_VAL));
    //���������
    //dev_misc_get_hwver();
//    dev_trng_open();
//dev_com_open(3, 115200, 8, 'n', 0, 0);
//  uart_test();
//MISC_DEBUG("\r\n");
#endif
    #ifdef TRENDIT_CORE
    dev_rtc_open();
    #endif
}


