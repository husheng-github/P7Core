#ifndef __DEV_ADC_H
#define __DEV_ADC_H

typedef enum _ADC_DEVICE
{
    DEV_ADC_HWVER = 0,  //硬件版本
    DEV_ADC_OVP = 1,   //P6 OVP
    DEV_ADC_PTR_TM = 2, //打印机温度
    DEV_DAC_AUDIO = 3,
#ifdef MACHINE_M6// by cgj 20190326 
    DEV_ADC_TPX = 3,
    DEV_ADC_TPY = 4,
    DEV_ADC_MAX = 5,
#else
    DEV_ADC_MAX  = 4,
#endif
}ADC_DEVICE_t;

void dev_adc_init(void);
void dev_adc_deinit(void);

s32 dev_adc_open(ADC_DEVICE_t adcdev);
s32 dev_adc_close(ADC_DEVICE_t adcdev);
s32 dev_adc_get_value(ADC_DEVICE_t adcdev,u16 *adcbuffer,u32 sampletimes);
s32 dev_adc_get_arrage_value(ADC_DEVICE_t adcdev, u8 times);
#endif

