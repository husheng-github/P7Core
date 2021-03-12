/**
 * @file drv_misc.c
 * @author pengxuebin
 * @date  2019.12.6
 * @brief 混合设备和硬件相关的控制函数，包含非主流的、不成体系的设备控制，如钱箱等
 * @note  需引用devglobal.h、drv_misc.h头文件
 * @since 2019.12.6  初始代码
 */
 #include "devglobal.h"

#define CASHBOX_EN_PIN      GPIO_PIN_PTC8


/**
 * @brief CASHBOX(钱箱)管脚初始化
 * @param [in/out] 无
 * @param [in/out] 无
 * @retval 无
 * @since 修改的历史说明
 */
void drv_misc_cashbox_init(void)
{
    //Cashbox使能脚初始化为输出低,
    dev_gpio_config_mux(CASHBOX_EN_PIN, MUX_CONFIG_GPIO);
    dev_gpio_set_pad(CASHBOX_EN_PIN, PAD_CTL_PULL_NONE);
    dev_gpio_direction_output(CASHBOX_EN_PIN, 1);
}

/**
 * @brief 控制CASHBOX(钱箱)输出
 * @param [in] mod: 0:输出低、1输出高
 * @param [in/out] 无
 * @retval 无
 * @since 修改的历史说明
 */
void drv_misc_cashbox_ctl(u8 flg)
{
    dev_gpio_set_value(CASHBOX_EN_PIN, flg);
}

 