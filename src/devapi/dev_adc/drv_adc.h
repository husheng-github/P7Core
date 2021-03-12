#ifndef __DRV_ADC_H
#define __DRV_ADC_H

typedef struct _str_ADC_PARAM
{
    s32 m_ch;               //ADC通道号
    iomux_pin_name_t m_gpiopin;     //GPIO管脚做片选
}str_adc_param_t;

void drv_adc_init(void);
void drv_adc_deinit(void);

s32 drv_adc_open(ADC_DEVICE_t adcdev);
s32 drv_adc_close(ADC_DEVICE_t adcdev);
s32 drv_adc_get_value(ADC_DEVICE_t adcdev,u16 *adcbuffer,u32 sampletimes);

#endif
