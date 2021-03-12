/********************************Copyright ( c ) ********************************
**                  DAQU 
**                         
**                        
**
**Created By:         
** Created Date:     
** Version:        
** Description:    ���ļ�����psam��Ӳ����������ӿ�
  
****************************************************************************/
#include "devglobal.h"
#include "sdk/mhscpu_sdk.h"
#include "drv_psam_io.h"

#if(PSAM_EXIST==1)
#define PSAM_EN_PIN     GPIO_PIN_EX3//GPIO_PIN_PTA14  
#define PSAM1_IO_PIN    GPIO_PIN_PTA4//GPIO_PIN_PTA13

#define PSAM1_RST_PIN   GPIO_PIN_EX4//GPIO_PIN_PTC10
#define PSAM1_CLK_PIN   GPIO_PIN_PTA5//GPIO_PIN_PTA2
#define PSAM1_CLK_PWM_CH   5//PWM5

#define PSAM2_IO_PIN    GPIO_PIN_PTD8
#define PSAM2_RST_PIN   GPIO_PIN_PTC11
#define PSAM2_CLK_PIN   GPIO_PIN_PTB0
#define PSAM2_CLK_PWM_CH   0//PWM0



typedef struct _str_PSAM_PINCFG
{
    iomux_pin_name_t m_iopin;       //IO��
    iomux_pin_name_t m_rstpin;      //rst��
    u32              m_clkpwmch;       //pwmͨ��
}str_psam_pincfg_t;

const str_psam_pincfg_t str_psam_pincfg_tab[PSAM_NUM_MAX] = 
{
#if(MACHINETYPE==MACHINE_S1)
    {PSAM1_IO_PIN, PSAM1_RST_PIN, PSAM1_CLK_PWM_CH},
//    {GPIO_PIN_NONE, GPIO_PIN_NONE, PSAM2_CLK_PWM_CH},
#elif(MACHINETYPE==MACHINE_M2) 
    {GPIO_PIN_NONE, GPIO_PIN_NONE, PSAM1_CLK_PWM_CH},
//    {GPIO_PIN_NONE, GPIO_PIN_NONE, PSAM2_CLK_PWM_CH},   
#endif
};
static u8 g_psam_power_flg;
#if 0
#define PSAMPIN_NUM_MAX   7
const iomux_pin_name_t psamtestpin_tab[PSAMPIN_NUM_MAX]=
{
    PSAM_EN_PIN,
    PSAM1_IO_PIN,  
    PSAM1_RST_PIN,
    PSAM1_CLK_PIN,
    PSAM2_IO_PIN,  
    PSAM2_RST_PIN,
    PSAM2_CLK_PIN,
    
};
#if 0
s32 drv_psam_io_test(void)
{
    s32 i,j;

    for(i=0; i<PSAMPIN_NUM_MAX; i++)
    {
        //����PSAM_EN_PIN���0��PSAM����ѹ���0
        dev_gpio_config_mux(psamtestpin_tab[i], MUX_CONFIG_GPIO);
        dev_gpio_set_pad(psamtestpin_tab[i], PAD_CTL_PULL_NONE);
        dev_gpio_direction_output(psamtestpin_tab[i], 0);
    }
  #if 0
    for(i=0; i<100; i++)
    {
        for(j=0; j<PSAMPIN_NUM_MAX; j++)
        {
            dev_gpio_set_value(psamtestpin_tab[j], 1);
        }
        dev_user_delay_ms(200);
        for(j=0; j<PSAMPIN_NUM_MAX; j++)
        {
            dev_gpio_set_value(psamtestpin_tab[j], 0);
        }
        dev_user_delay_ms(200);
    }
  #endif    
}
#endif
#define TESTPIN_OFFSET      4
#define TESTPIN_NUM_MAX     3
u8 g_testpin_flg[TESTPIN_NUM_MAX];
void drv_psam_testpin_set(s32 n, s32 flg)
{
    dev_gpio_set_value(psamtestpin_tab[TESTPIN_OFFSET+n], flg);
    g_testpin_flg[n] = flg;
}
void drv_psam_testpin_xor(s32 n)
{
    if(g_testpin_flg[n])
    {
        g_testpin_flg[n] = 0;
    }
    else
    {
        g_testpin_flg[n] = 1;
    }
    dev_gpio_set_value(psamtestpin_tab[TESTPIN_OFFSET+n], g_testpin_flg[n]);
    
}
#endif
/****************************************************************************
**Description:       PSAM���ܽų�ʼ��
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170617
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_psam_io_init(void)
{
//    static u8 flg=0;
    str_pwm_cfg_t pwm_cfg;
    s32 ret;
    s32 i;
   #if 0 
    if(flg == 1)
    {
        return DEVSTATUS_SUCCESS;
    }
   #endif 
    //����PSAM_EN_PIN���0��PSAM����ѹ���0
    dev_gpio_config_mux(PSAM_EN_PIN, MUX_CONFIG_GPIO);
    dev_gpio_set_pad(PSAM_EN_PIN, PAD_CTL_PULL_NONE);
    dev_gpio_direction_output(PSAM_EN_PIN, 1);
    g_psam_power_flg = 0; 
    
    pwm_cfg.m_pwmhz = PSAM_CLK_Frequency;
    pwm_cfg.m_LowPeriod = PSAM_LowLevelPeriod;
    pwm_cfg.m_HighPeriod = PSAM_HighLevelPeriod;

    for(i=0; i<PSAM_NUM_MAX; i++)
    {
        if(str_psam_pincfg_tab[i].m_iopin!=GPIO_PIN_NONE)
        {
            //����IOΪ����
            dev_gpio_config_mux(str_psam_pincfg_tab[i].m_iopin, MUX_CONFIG_GPIO);
            dev_gpio_set_pad(str_psam_pincfg_tab[i].m_iopin, PAD_CTL_PULL_UP);  //��������
            //dev_gpio_direction_input(str_psam_pincfg_tab[i].m_iopin);
            dev_gpio_direction_output(str_psam_pincfg_tab[i].m_iopin, 0);
            //����RST���0
            dev_gpio_config_mux(str_psam_pincfg_tab[i].m_rstpin, MUX_CONFIG_GPIO);
            dev_gpio_set_pad(str_psam_pincfg_tab[i].m_rstpin, PAD_CTL_PULL_NONE);
            dev_gpio_direction_output(str_psam_pincfg_tab[i].m_rstpin, 0);
//PSAM_DEBUG("\r\n");            
            ret = dev_pwm_open(str_psam_pincfg_tab[i].m_clkpwmch, pwm_cfg);
            if(ret < 0)
            {
                PSAM_DEBUG("ERR(ret=%d)\r\n", ret);
                return DEVSTATUS_ERR_FAIL;
            }
            //dev_pwm_run(str_psam_pincfg_tab[i].m_clkpwmch);
        }
        else
        {
            PSAM_DEBUG("PIN err!\r\n");
            return DEVSTATUS_ERR_FAIL;
        }
    }
//drv_psam_io_test();    
//    flg = 1;
    return DEVSTATUS_SUCCESS;
}
/****************************************************************************
**Description:       IO�ܽ��ж�����
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170617
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_psam_io_irq_request(s32 psamno, void (*handler)(void* ))
{
    s32 ret;
    
    if(psamno < PSAM_NUM_MAX)
    {
        ret = dev_gpio_irq_request(str_psam_pincfg_tab[psamno].m_iopin, handler, IRQ_ISR_FALLING, NULL);
    }
    else
    {
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    return ret;
}
/****************************************************************************
**Description:       IO�ܽ��ж�ʹ�ܿ���
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170617
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void drv_psam_io_irq_enable(s32 psamno, s32 mod)
{
    if(psamno < PSAM_NUM_MAX)
    {
        if(mod == 0)
        {
            //��ֹ�ж�
            dev_gpio_irq_disable(str_psam_pincfg_tab[psamno].m_iopin);
        }
        else
        {
            //���жϱ�־
            dev_gpio_irq_clrflg(str_psam_pincfg_tab[psamno].m_iopin);
            //ʹ���ж�
            dev_gpio_irq_enable(str_psam_pincfg_tab[psamno].m_iopin);
        }
    }
}

/****************************************************************************
**Description:       PSAM����Դ����
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170617
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void drv_psam_power_ctl(s32 psamno, s32 flg)
{
    if(psamno < PSAM_NUM_MAX)
    {
        if(flg)
        {   //�ϵ�
            dev_gpio_set_value(PSAM_EN_PIN, 0);
            g_psam_power_flg |= (1<<psamno);
        }
        else
        {
            g_psam_power_flg &= ~(1<<psamno);
            if(g_psam_power_flg == 0)
            {
                dev_gpio_set_value(PSAM_EN_PIN, 1);
            } 
        }
    }
}
/****************************************************************************
**Description:       RST�����
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170617
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void drv_psam_rst_ctl(s32 psamno, s32 flg)
{
    if(psamno < PSAM_NUM_MAX)
    {
        dev_gpio_set_value(str_psam_pincfg_tab[psamno].m_rstpin, flg);
    }
}
/****************************************************************************
**Description:       CLKʹ�ܿ���
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170617
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void drv_psam_clk_ctl(s32 psamno, s32 flg)
{
    if(psamno < PSAM_NUM_MAX)
    {
        if(GPIO_PIN_NONE != str_psam_pincfg_tab[psamno].m_iopin)
        {
            if(flg)
            {   //��ʱ��
                dev_pwm_run(str_psam_pincfg_tab[psamno].m_clkpwmch);
            }
            else
            {   //�ر�ʱ��
                dev_pwm_stop(str_psam_pincfg_tab[psamno].m_clkpwmch);
            }
        }
    }
}
/****************************************************************************
**Description:       IO����\�������
**Input parameters:    
                    0: ����
                    1: ���
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170617
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void drv_psam_io_direction_set(s32 psamno, s32 mod, s32 value)
{
    if(psamno < PSAM_NUM_MAX)
    {
        if(0 == mod)
        {
            dev_gpio_direction_input(str_psam_pincfg_tab[psamno].m_iopin);
        }
        else
        {
            dev_gpio_direction_output(str_psam_pincfg_tab[psamno].m_iopin, value);
        }
    }    
}
/****************************************************************************
**Description:       IO���
**Input parameters:    
                    0: ����
                    1: ���
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170617
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void drv_psam_io_set_value(s32 psamno, s32 value)
{
    if(psamno < PSAM_NUM_MAX)
    {
        dev_gpio_set_value(str_psam_pincfg_tab[psamno].m_iopin, value);
    }
}
/****************************************************************************
**Description:       ��IO��ֵ
**Input parameters:    
                    0: ����
                    1: ���
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170617
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_psam_io_get_value(s32 psamno)
{
    if(psamno < PSAM_NUM_MAX)
    {
        return dev_gpio_get_value(str_psam_pincfg_tab[psamno].m_iopin);
    }
    else
    {
        return 0;
    }
}
#endif

