/**
 * @file drv_wifi_io.c
 * @author pengxuebin
 * @date   2020/2/11
 * @brief  ���ļ���������ģ���Ӳ���ܽ���صĽӿڣ�����Ӳ����ز��֡�
 * @note  
 * @since 
 */
#include "devglobal.h"
#include "drv_wifi_io.h"


/**
 * @brief GPRS�ܽų�ʼ��
 * @param [in/out] 
 * @param [in/out] 
 * @retval 
 * @retval 
 * @since 
 */
void drv_wifi_io_init(void)
{
    //ʹ�ܽ�����
    dev_gpio_config_mux(WIFI_EN_PIN, MUX_CONFIG_GPIO);
    dev_gpio_set_pad(WIFI_EN_PIN, PAD_CTL_PULL_NONE);
    dev_gpio_direction_output(WIFI_EN_PIN, 1);

    //���ѽ�����
    dev_gpio_config_mux(WIFI_WAKE_PIN, MUX_CONFIG_GPIO);
    dev_gpio_set_pad(WIFI_WAKE_PIN, PAD_CTL_PULL_NONE);
    dev_gpio_direction_output(WIFI_WAKE_PIN, 1);
}


