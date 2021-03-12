/**
 * @file drv_wireless_io.c
 * @author pengxuebin
 * @date   2020/2/11
 * @brief  本文件处理无线模块跟硬件管脚相关的接口，隔离硬件相关部分。
 * @note  
 * @since 
 */
#include "devglobal.h"
#include "drv_wireless_io.h"

#if(WIRELESS_EXIST == 1)
/**
 * @brief GPRS管脚初始化
 * @param [in/out] 
 * @param [in/out] 
 * @retval 
 * @retval 
 * @since 
 */
#ifdef TRENDIT_BOOT
void dev_wireless_set_power_flag(s32 power_flag)
{
}

s32 dev_wireless_get_power_flag(void)
{
    return TRUE;
}

#endif
void drv_gprs_io_init(void)
{
    if(WIRELESS_WITH_POWERKEY)
    {
        //使能
        dev_gpio_config_mux(GPRS_POWEREN_PIN, MUX_CONFIG_GPIO);
        dev_gpio_set_pad(GPRS_POWEREN_PIN, PAD_CTL_PULL_NONE);
        dev_gpio_direction_output(GPRS_POWEREN_PIN, 1); 
        
        //上电
        dev_gpio_config_mux(GPRS_POWERON_PIN, MUX_CONFIG_GPIO);
        dev_gpio_set_pad(GPRS_POWERON_PIN, PAD_CTL_PULL_NONE);
        dev_gpio_direction_output(GPRS_POWERON_PIN, 1);
        #ifndef TRENDIT_BOOT
        dev_wireless_set_power_flag(TRUE);
        #endif
    }
    else
    {
        //使能
        dev_gpio_config_mux(GPRS_POWEREN_PIN, MUX_CONFIG_GPIO);
        dev_gpio_set_pad(GPRS_POWEREN_PIN, PAD_CTL_PULL_NONE);
        dev_gpio_direction_output(GPRS_POWEREN_PIN, 0);                   //插外电不开机会充电，打开会影响充电时间
        #ifndef TRENDIT_BOOT
        dev_wireless_set_power_flag(FALSE);
        #endif
    }

    if(WIRELESS_NEED_SLEEP)
    {
        dev_gpio_config_mux(GPRS_WAKEUP_PIN, MUX_CONFIG_GPIO);
        dev_gpio_set_pad(GPRS_WAKEUP_PIN, PAD_CTL_PULL_NONE);
        dev_gpio_direction_output(GPRS_WAKEUP_PIN, 1);
    }
}

void drv_gprs_io_switch_ctl(u8 flg)
{
    dev_gpio_set_value(GPRS_POWEREN_PIN, flg);
    dev_wireless_set_power_flag(flg);
}


void drv_wireless_set_ringanddtrpin_init(void)
{
    
    #if WIRELESS_USE_UART_RING
    dev_gpio_config_mux(GPRS_RING_PIN, MUX_CONFIG_GPIO);
    dev_gpio_set_pad(GPRS_RING_PIN, PAD_CTL_PULL_NONE);
    dev_gpio_direction_input(GPRS_RING_PIN);  


    dev_gpio_config_mux(GPRS_DTR_PIN, MUX_CONFIG_GPIO);
    dev_gpio_set_pad(GPRS_DTR_PIN, PAD_CTL_PULL_NONE);
    dev_gpio_direction_output(GPRS_DTR_PIN, 0);  // 低电平表示未休眠
    #endif

    
}



/**
 * @brief 进入休眠
 * @retval 
 * @retval 
 */
void drv_wireless_ringanddtrpin_enter_sleep(void)
{
    
    #if WIRELESS_USE_UART_RING
    dev_gpio_direction_output(GPRS_DTR_PIN, 1); 
    dev_gpio_wakeup_ctl(GPRS_RING_PIN, 1);
    #endif
    
}


/**
 * @brief 退出休眠
 * @retval 
 * @retval 
 */
void drv_wireless_ringanddtrpin_exit_sleep(void)
{
    
    #if WIRELESS_USE_UART_RING
    dev_gpio_direction_output(GPRS_DTR_PIN, 0); 
    dev_gpio_wakeup_ctl(GPRS_RING_PIN, 0);
    #endif
    
}
#else
void drv_gprs_io_init(void)
{
    
}

void drv_gprs_io_switch_ctl(u8 flg)
{
}
#endif
