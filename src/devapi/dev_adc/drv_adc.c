/********************************Copyright ( c ) ********************************
**                  DAQU 
**                         
**                        
**
**Created By:     pengxuebin,20170422      
** Created Date:     
** Version:        
** Description:    ADC��������  
****************************************************************************/
#include "devglobal.h"
#include "sdk/mhscpu_sdk.h"
#include "drv_adc.h"

#define ADC_CHANNEL_MAX     7   //ADCͨ������
#define ADC_CHANNEL_NONE    0xff          
static u8 g_drv_adc_fd = 0;
static s8 g_drv_adc_init = 0;
static u8 g_drv_adc_flg = 0;
//static u8 g_drv_adc_chold = ADC_CHANNEL_NONE;

const str_adc_param_t str_adc_param_tab[DEV_ADC_MAX] = 
{
    //Ӳ���汾(ADC_IN1-->PC1)
    {1, GPIO_PIN_PTC0},
#if(MACHINETYPE==MACHINE_M2)
    //���(ADC_IN5-->PC5)
    {5, GPIO_PIN_PTC5},
    {ADC_CHANNEL_NONE, GPIO_PIN_NONE},  //������
#elif(MACHINETYPE==MACHINE_S1)
    {6, GPIO_PIN_PTC5},  //P6 OVP
    //��ӡ���¶�(ADC_IN5-->PC5)
    {5, GPIO_PIN_PTC4},
#endif        
};
void delay_adc(void)
{
    //uint32_t delay = 0xFFFF; //sxl20180721
    uint32_t delay = 0x5FFFFF;
    
    while(delay--);
}



void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	NVIC_SetPriorityGrouping(NVIC_PriorityGroup_3);
	
	NVIC_InitStructure.NVIC_IRQChannel = ADC0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}
void ADC_Configuration(void)
{
    ADC_InitTypeDef ADC_InitStruct;
    
	GPIO_PinRemapConfig(GPIOC, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5, GPIO_Remap_2);
    GPIO_PullUpCmd(GPIOC,GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5,DISABLE);
    
    //ADC channel ChargeVbat��Charge_Vbat�ܽţ�channel1��ӦPC0��channel2��ӦPC1���Դ�����
    ADC_InitStruct.ADC_Channel = ADC_CHANNEL_5;
    ADC_InitStruct.ADC_SampSpeed = ADC_SpeedPrescaler_2;
    ADC_InitStruct.ADC_IRQ_EN = DISABLE;
    ADC_InitStruct.ADC_FIFO_EN = DISABLE;

    ADC_Init(&ADC_InitStruct);
}
void UART_Configuration(void)
{
	UART_InitTypeDef UART_InitStructure;

	GPIO_PinRemapConfig(GPIOA, GPIO_Pin_0 | GPIO_Pin_1, GPIO_Remap_0);

    //GPIO_PinRemapConfig(GPIOB, GPIO_Pin_12 | GPIO_Pin_13, GPIO_Remap_3);
	UART_InitStructure.UART_BaudRate = 115200;
	UART_InitStructure.UART_WordLength = UART_WordLength_8b;
	UART_InitStructure.UART_StopBits = UART_StopBits_1;
	UART_InitStructure.UART_Parity = UART_Parity_No;

	UART_Init(UART0, &UART_InitStructure);	
}

/**
 * @brief  ��ʼ��adc����
 * @param [in] ��
 * @param [out] ��
 * @retval ��
 * @since ��
 */
void drv_adc_init()
{
    ADC_InitTypeDef ADC_InitStruct;
    u32 i;
    uint32_t u32_PointNumber = 200;
    uint16_t ADC_Buffer[200];
    if(g_drv_adc_init == 0)
    {
        SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_ADC, ENABLE);
        SYSCTRL_APBPeriphResetCmd(SYSCTRL_APBPeriph_ADC, ENABLE);

        for(i  = 0; i < DEV_ADC_MAX-1; i++)
        {
            dev_gpio_config_mux(str_adc_param_tab[i].m_gpiopin, MUX_CONFIG_ALT2);
            dev_gpio_set_pad(str_adc_param_tab[i].m_gpiopin, PAD_CTL_PULL_NONE);
        }
        
        ADC_InitStruct.ADC_Channel = ADC_CHANNEL_1;
        ADC_InitStruct.ADC_SampSpeed = ADC_SpeedPrescaler_2;//ADC_SAMP_SEL_500K;  ADC_SpeedPrescaler_None
        ADC_InitStruct.ADC_IRQ_EN = DISABLE;
        ADC_InitStruct.ADC_FIFO_EN = DISABLE;
//        ADC_InitStruct.ADC_IRQ_EN = ENABLE;
        ADC_Init(&ADC_InitStruct);

        ADC_StartCmd(ENABLE);
        ADC_BuffCmd(ENABLE); 
        ADC_DivResistorCmd(ENABLE);
        g_drv_adc_init = -1;

    }
}
/**
 * @brief  �ر�adc����
 * @param [in] ��
 * @param [out] ��
 * @retval ��
 * @since ��
 */
void drv_adc_deinit()
{
    if(g_drv_adc_init < 0)
    {
        ADC_StartCmd(DISABLE);
        ADC_BuffCmd(DISABLE);
        SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_ADC, DISABLE);
        g_drv_adc_init = 0;
    }

}

/****************************************************************************
**Description:       ��ADC
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170422
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_adc_open(ADC_DEVICE_t adcdev)
{
    ADC_InitTypeDef ADC_InitStruct;
    
    if(adcdev >= DEV_ADC_MAX)
    {
        return -1;
    }
    if(0 != (g_drv_adc_fd&(1<<adcdev)))
    {
        return 0;
    }
    if(str_adc_param_tab[adcdev].m_ch >= ADC_CHANNEL_MAX)
    {
        return -1;
    }

    ADC0->ADC_CR1 = (ADC0->ADC_CR1 & ~(ADC_CR1_CHANNEL_MASK)) | ((ADC_ChxTypeDef)str_adc_param_tab[adcdev].m_ch);
    if(g_drv_adc_flg == 0)
    {
        dev_user_delay_ms(10);
        g_drv_adc_flg = 1;
    }
    
    g_drv_adc_fd |= (1<<adcdev);  
    return 0;
}
/****************************************************************************
**Description:       �ر�ADC�豸
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170422
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_adc_close(ADC_DEVICE_t adcdev)
{
    if(adcdev >= DEV_ADC_MAX)
    {
        return -1;
    }
    if(0 == (g_drv_adc_fd&(1<<adcdev)))
    {
        return 0;
    }
    g_drv_adc_fd &= (~(1<<adcdev));
    return 0;
}

/****************************************************************************
**Description:       ��ȡADCֵ
**Input parameters:    
**Output parameters: 
**Returned value:
                    >=0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
                    DEVSTATUS_ERR_FAIL:  ʧ��
                    
**Created by:        pengxuebin,20170422
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_adc_get_value(ADC_DEVICE_t adcdev,u16 *adcbuffer,u32 sampletimes)
{
    s32 val;
	u32 i;
    u32 timerid;
    u8 flg;
    
    if(adcdev >= DEV_ADC_MAX)
    {
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    if(0 == (g_drv_adc_fd&(1<<adcdev)))
    {
        return -1;      //ADCû�д�
    }
	//chanage ADC channel
	//ADC_StartCmd(DISABLE);
    //ADC_ClrStatus();

    //ADC0->ADC_CR1 = (ADC0->ADC_CR1 & ~(0x7)) | ((ADC_ChxTypeDef)str_adc_param_tab[adcdev].m_ch); 

    //ADC0->ADC_CR1 = (ADC0->ADC_CR1 & ~(ADC_CR1_CHANNEL_MASK)) | ((ADC_ChxTypeDef)str_adc_param_tab[adcdev].m_ch); 

    if((adcdev == DEV_ADC_OVP) || (adcdev == DEV_ADC_PTR_TM))
    {
        //delay_adc();
    }
    #if 0
        adcbuffer[i] = ADC_GetResult();
    #else
	for(i = 0;i < sampletimes;i++)
	{
        timerid = dev_user_gettimeID();
        flg = 0;
        while((!(ADC0->ADC_SR & ADC_SR_DONE_FLAG)))
        {
            if(dev_user_querrytimer(timerid, 100))
            {
                //dev_debug_printf("%s(%d):ADC err!\r\n", __FUNCTION__, __LINE__);
                flg = 1;
                break;
            }
        }
        if(flg)
        {
           // continue;
        }
    	adcbuffer[i] = ADC0->ADC_DATA & 0xFFF;//ADC_GetResult(); 
	}
    #endif
    //ADC_StartCmd(DISABLE);//�˽ӿڹرջ�Ӱ��audio�����õ�adc close��
    return 0;
}


