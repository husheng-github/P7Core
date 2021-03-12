/********************************Copyright ( c ) ********************************
**                  DAQU 
**                         
**                        
**
**Created By:     pengxuebin,20170320      
** Created Date:     
** Version:        
** Description:    该文件处理系统tick做系统定时、硬延时等功能
****************************************************************************/
#include "devglobal.h"
#include "sdk/mhscpu_sdk.h"
#include "drv_timer.h"

static TIM_NumTypeDef const tim_type_tab[DRVTIMNUM_MAX] = {TIM_0, TIM_1, TIM_2, TIM_3, TIM_4, TIM_5, TIM_6, TIM_7};
static u8 g_drvtimer_flg = 0;
static str_irq_handler_t g_timerirq_handler[DRVTIMNUM_MAX];
const IRQn_Type timer_irqn_tab[DRVTIMNUM_MAX]={TIM0_0_IRQn, TIM0_1_IRQn, TIM0_2_IRQn, TIM0_3_IRQn, TIM0_4_IRQn, TIM0_5_IRQn, TIM0_6_IRQn, TIM0_7_IRQn};
//static DRVTIMER_CFG_t g_timer_cfg[DRVTIMNUM_MAX];
typedef struct _str_PWM_PINCFG
{
    iomux_pin_name_t m_gpiopin;     //管脚
    iomux_pin_cfg_t  m_alt;         //作为PWM的重映射熟悉
    u8               m_out;         //pwm关闭时的输出电平
}str_pwm_pincfg_t;

const str_pwm_pincfg_t str_pwm_pincfg_tab[DRVTIMNUM_MAX] = 
{
    {GPIO_PIN_PTB0, MUX_CONFIG_ALT2, 0},      
    {GPIO_PIN_PTB1, MUX_CONFIG_ALT2, 0},     
    {GPIO_PIN_PTA2, MUX_CONFIG_ALT2, 0},    
    {GPIO_PIN_PTB3, MUX_CONFIG_ALT2, 0},   
    {GPIO_PIN_PTA4, MUX_CONFIG_ALT2, 0},    
    {GPIO_PIN_PTA5, MUX_CONFIG_ALT2, 0},  
    {GPIO_PIN_PTA6, MUX_CONFIG_ALT2, 0},  
    {GPIO_PIN_PTA7, MUX_CONFIG_ALT2, 0},
};
/****************************************************************************
**Description:        定时器中断服务程序
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170320 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void TIM0_IRQHandler(u32 timid, void *data)
{
    g_timerirq_handler[timid].m_func(data);
}
void TIM0_0_IRQHandler(void)
{
    TIM_ClearITPendingBit(TIMM0, TIM_0);
    TIM0_IRQHandler(0, NULL);
}
void TIM0_1_IRQHandler(void)
{
    TIM_ClearITPendingBit(TIMM0, TIM_1);
    TIM0_IRQHandler(1, NULL);
}
void TIM0_2_IRQHandler(void)
{
    TIM_ClearITPendingBit(TIMM0, TIM_2);
    TIM0_IRQHandler(2, NULL);
}
void TIM0_3_IRQHandler(void)
{
    TIM_ClearITPendingBit(TIMM0, TIM_3);
    TIM0_IRQHandler(3, NULL);
}
void TIM0_4_IRQHandler(void)
{
    TIM_ClearITPendingBit(TIMM0, TIM_4);
    TIM0_IRQHandler(4, NULL);
}
void TIM0_5_IRQHandler(void)
{
    TIM_ClearITPendingBit(TIMM0, TIM_5);
    TIM0_IRQHandler(5, NULL);
}
void TIM0_6_IRQHandler(void)
{
    TIM_ClearITPendingBit(TIMM0, TIM_6);
    TIM0_IRQHandler(6, NULL);
}
void TIM0_7_IRQHandler(void)
{
    TIM_ClearITPendingBit(TIMM0, TIM_7);
    TIM0_IRQHandler(7, NULL);
}
/****************************************************************************
**Description:        定时器设备初始化
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170320 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void drv_timer_init(void)
{
    s32 i;
    static u8 flg=1;
    
    if(flg == 1)
    {
        flg = 0;
        for(i=0; i<DRVTIMNUM_MAX; i++)
        {
            //关中断
            TIM_ITConfig(TIMM0, tim_type_tab[i], DISABLE);
            //清标志
            TIM_ClearITPendingBit(TIMM0, tim_type_tab[i]);
            g_timerirq_handler[i].m_data = NULL;
            g_timerirq_handler[i].m_func = NULL;
            //g_timerirq_handler[i].m_param
            g_drvtimer_flg = 0;
        }
    }
}

/****************************************************************************
**Description:        定时器设备初始化
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170320 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_timer_request(s32 timid, const DRVTIMER_CFG_t t_cfg, void(*pFun)(void*), void *data)
{ 
//    s32 i;
    TIM_InitTypeDef TIM_InitStruct;
    u32 Period = 0;
    u32 DutyCyclePeriod = 0;
    SYSCTRL_ClocksTypeDef  clocks;
    TIM_PWMInitTypeDef tim_pwminit;
    
    if(timid >= DRVTIMNUM_MAX)
    {
        TIMER_DEBUG("timid=%d\r\n", timid);
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    
    drv_timer_init();
    if(g_drvtimer_flg == 0)
    {
        SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_TIMM0, ENABLE);
        SYSCTRL_APBPeriphResetCmd(SYSCTRL_APBPeriph_TIMM0, ENABLE);
    }
    
//TIMER_DEBUG("timid=%d\r\n", timid);  

    if(g_drvtimer_flg&(1<<timid))
    {
        return DEVSTATUS_ERR_BUSY;      //已经被申请
    }
    
    if(TIMER_MODE_CNT == t_cfg.m_mode)
    {
        //定时器
        g_timerirq_handler[timid].m_data = NULL;
        g_timerirq_handler[timid].m_func = pFun;
            
        TIM_InitStruct.TIMx = (TIM_NumTypeDef)timid;
        TIM_InitStruct.TIM_Period = (SYSCTRL->PCLK_1MS_VAL/1000)*t_cfg.m_tus;
        TIM_Init(TIMM0, &TIM_InitStruct);
        if(t_cfg.m_reload == 0)
        {
            TIMM0->TIM[timid].ControlReg &= (~0x02);       //自由运行模式
        }
        else
        {
            TIMM0->TIM[timid].ControlReg |= 0x02;       //用户定义模式
        }
        TIM_ITConfig(TIMM0, TIM_InitStruct.TIMx, ENABLE);
        NVIC_EnableIRQ(timer_irqn_tab[timid]);   
    }
    else 
    {
        SYSCTRL_GetClocksFreq(&clocks);
        //PWM
        Period = clocks.PCLK_Frequency/t_cfg.m_pwmhz;
        //DutyCyclePeriod = Period /();
        tim_pwminit.TIM_LowLevelPeriod = (((Period*t_cfg.m_LowLevelPeriod)/(t_cfg.m_LowLevelPeriod+t_cfg.m_HighLevelPeriod))-1);
        tim_pwminit.TIM_HighLevelPeriod = (((Period*t_cfg.m_HighLevelPeriod)/(t_cfg.m_LowLevelPeriod+t_cfg.m_HighLevelPeriod))-1);
        tim_pwminit.TIMx = (TIM_NumTypeDef)timid;
        TIM_PWMInit(TIMM0, &tim_pwminit);
        //管脚配置为输出
        //drv_pwm_pincfg(timid, 0);   
    }
    g_drvtimer_flg |= (1<<timid);
    return DEVSTATUS_SUCCESS;
}
/****************************************************************************
**Description:        定时器释放
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170321 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_timer_free(u32 timid)
{
    
    if(timid >= DRVTIMNUM_MAX)
    {
        return DEVSTATUS_ERR_PARAM_ERR;
    }

    NVIC_DisableIRQ(timer_irqn_tab[timid]);
    TIM_Cmd(TIMM0, (TIM_NumTypeDef)timid, DISABLE);
    TIM_ITConfig(TIMM0, (TIM_NumTypeDef)timid, DISABLE);
    g_timerirq_handler[timid].m_data = NULL;
    g_timerirq_handler[timid].m_func = NULL;
           
    g_drvtimer_flg &= ~(1<<timid);
    if(0 == g_drvtimer_flg)
    {
        SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_TIMM0, DISABLE);
        SYSCTRL_APBPeriphResetCmd(SYSCTRL_APBPeriph_TIMM0, DISABLE);
    }
    return 0;
}
/****************************************************************************
**Description:        关闭定时器
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170321 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void drv_timer_stop(u32 timid)
{
  #if 0  
    if(timid >= DRVTIMNUM_MAX)
    {
        return DEVSTATUS_ERR_PARAM_ERR;
    }
  #endif  
    TIM_Cmd(TIMM0, (TIM_NumTypeDef)timid, DISABLE);
}
/****************************************************************************
**Description:        启动定时器
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170321 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void drv_timer_run(u32 timid)
{
  #if 0  
    if(timid >= DRVTIMNUM_MAX)
    {
        return DEVSTATUS_ERR_PARAM_ERR;
    }
  #endif    
    TIM_Cmd(TIMM0, (TIM_NumTypeDef)timid, ENABLE);
}
/****************************************************************************
**Description:        设置定时器值
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170621
**---------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void drv_timer_setvalue(u32 timid, u32 tus)
{
    u32 period; 
    
    period = (SYSCTRL->PCLK_1MS_VAL/1000)*tus;
    TIM_SetPeriod(TIMM0, (TIM_NumTypeDef)timid, period);
}
/****************************************************************************
**Description:        设置定时器值(以时钟CLK为单位)
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170621
**---------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void drv_timer_setvalue_clk(u32 timid, u32 tclk)
{
    TIM_SetPeriod(TIMM0, (TIM_NumTypeDef)timid, tclk);
}
/****************************************************************************
**Description:        使能中断
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170321 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void drv_timer_int_enable(u32 timid)
{
    TIM_ITConfig(TIMM0, (TIM_NumTypeDef)timid, ENABLE);
}
/****************************************************************************
**Description:        禁能中断
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170321 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void drv_timer_int_disable(u32 timid)
{
    TIM_ITConfig(TIMM0, (TIM_NumTypeDef)timid, DISABLE);
}
/****************************************************************************
**Description:        清除中断标志
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170321 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void drv_timer_int_clear(u32 timid)
{
    TIM_ClearITPendingBit(TIMM0, timid);
}
/****************************************************************************
**Description:        PWM参数设置
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170321 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void drv_timer_pwmset(s32 pwmid, const DRVTIMER_CFG_t t_cfg)
{
    uint32_t Period = 0;
    SYSCTRL_ClocksTypeDef  clocks;
    u32 low,high;
    
    
    SYSCTRL_GetClocksFreq(&clocks);
    //PWM
    Period = clocks.PCLK_Frequency/t_cfg.m_pwmhz;
    low = (((Period*t_cfg.m_LowLevelPeriod)/(t_cfg.m_LowLevelPeriod+t_cfg.m_HighLevelPeriod))-1);
    high = (((Period*t_cfg.m_HighLevelPeriod)/(t_cfg.m_LowLevelPeriod+t_cfg.m_HighLevelPeriod))-1);
    TIM_SetPWMPeriod(TIMM0, (TIM_NumTypeDef)pwmid, low, high);
}
/****************************************************************************
**Description:        PWM管脚配置
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170321 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_pwm_pincfg(u32 pwmid, u8 mod)
{
    if(0 == mod)
    {
        dev_gpio_config_mux(str_pwm_pincfg_tab[pwmid].m_gpiopin, MUX_CONFIG_GPIO);
        dev_gpio_direction_output(str_pwm_pincfg_tab[pwmid].m_gpiopin, str_pwm_pincfg_tab[pwmid].m_out);
    }
    else
    {
        dev_gpio_config_mux(str_pwm_pincfg_tab[pwmid].m_gpiopin, str_pwm_pincfg_tab[pwmid].m_alt);
    }
    return 0;
}
#if 0
s32 drv_pwm_pincfg(s32 pwmid)
{ 
    if(pwmid >= DRVTIMNUM_MAX)
    {
        TIMER_DEBUG("pwmid=%d\r\n", pwmid); 
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    switch(pwmid)
    {
    case 0:      
        dev_gpio_config_mux(PWM0_PIN, MUX_CONFIG_ALT2);
        break;
    case 1:
        dev_gpio_config_mux(PWM1_PIN, MUX_CONFIG_ALT2);
        break;
    case 2:
        dev_gpio_config_mux(PWM2_PIN, MUX_CONFIG_ALT2);
        break;
    case 3:
        dev_gpio_config_mux(PWM3_PIN, MUX_CONFIG_ALT2);
        break;
    case 4:
        dev_gpio_config_mux(PWM4_PIN, MUX_CONFIG_ALT2);
        break;
    case 5:
        dev_gpio_config_mux(PWM5_PIN, MUX_CONFIG_ALT2);
        break;
    } 
    return 0;
}
#endif

