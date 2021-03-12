/**
 * @file drv_misc.c
 * @author pengxuebin
 * @date  2019.12.6
 * @brief ����豸��Ӳ����صĿ��ƺ����������������ġ�������ϵ���豸���ƣ���Ǯ���
 * @note  ������devglobal.h��drv_misc.hͷ�ļ�
 * @since 2019.12.6  ��ʼ����
 */
 #include "devglobal.h"

#define CASHBOX_EN_PIN      GPIO_PIN_PTC8


/**
 * @brief CASHBOX(Ǯ��)�ܽų�ʼ��
 * @param [in/out] ��
 * @param [in/out] ��
 * @retval ��
 * @since �޸ĵ���ʷ˵��
 */
void drv_misc_cashbox_init(void)
{
    //Cashboxʹ�ܽų�ʼ��Ϊ�����,
    dev_gpio_config_mux(CASHBOX_EN_PIN, MUX_CONFIG_GPIO);
    dev_gpio_set_pad(CASHBOX_EN_PIN, PAD_CTL_PULL_NONE);
    dev_gpio_direction_output(CASHBOX_EN_PIN, 1);
}

/**
 * @brief ����CASHBOX(Ǯ��)���
 * @param [in] mod: 0:����͡�1�����
 * @param [in/out] ��
 * @retval ��
 * @since �޸ĵ���ʷ˵��
 */
void drv_misc_cashbox_ctl(u8 flg)
{
    dev_gpio_set_value(CASHBOX_EN_PIN, flg);
}

 