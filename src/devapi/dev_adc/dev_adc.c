/*****************************************************************************
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


/**
 * @brief  ��ʼ��adc����
 * @param [in] ��
 * @param [out] ��
 * @retval ��
 * @since ��
 */
void dev_adc_init()
{
    drv_adc_init();
}

/**
 * @brief  �ر�adc����
 * @param [in] ��
 * @param [out] ��
 * @retval ��
 * @since ��
 */
void dev_adc_deinit()
{
    drv_adc_deinit();
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
s32 dev_adc_open(ADC_DEVICE_t adcdev)
{
    return drv_adc_open(adcdev);
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
s32 dev_adc_close(ADC_DEVICE_t adcdev)
{
    return drv_adc_close(adcdev);
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
s32 dev_adc_get_value(ADC_DEVICE_t adcdev,u16 *adcbuffer,u32 sampletimes)
{
    return drv_adc_get_value(adcdev,adcbuffer,sampletimes);
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
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_adc_get_arrage_value(ADC_DEVICE_t adcdev, u8 times)
{
    u16 tmp[100];
    u32 i,j;
    u32 result;

    if(times < 50)
    {
        times = 50;
    }
    else if(times >= 100)
    {
        times = 100;
    }
    dev_adc_open(adcdev);
    dev_adc_get_value(adcdev,tmp,times);
    
    dev_adc_close(adcdev);  //sxl?
    //����
    for(i=0; i<(times-1); i++)
    {
        for(j=0; j<(times-1-i); j++)
        {
            if(tmp[j]>tmp[j+1])
            {
                result = tmp[j];
                tmp[j] = tmp[j+1];
                tmp[j+1] = result;
            }
        }
    }
    //ȡƽ����
    result = 0;
    for(i=10; i<(times-10); i++)
    {
        result += tmp[i];
        dev_debug_printf("%x ",tmp[i]);
    }
    result = result/(times-20);
    return result;
}

#if 0
/****************************************************************************
**Description:       ��ȡ��ص���
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
#define ADC_BAT_TIMES       18  
s32 dev_adc_get_bat_value(void)
{
    u32 sum=0;
    u32 max = 0;
    u32 min = 0xffff;
    s32 val;
    u16 i;
    
    for(i=0; i<ADC_BAT_TIMES; i++)
    {
        val = dev_adc_get_value(DEV_ADC_VBAT);
        if(val < 0)
        {       //��ʧ��
            return DEVSTATUS_ERR_PARAM_ERR;
        }
        sum += val;
        if(max<val)max = val;
        if(min>val)min = val;
        dev_user_delay_us(5);
    }
    sum -= max;
    sum -= min;
    return (sum/(ADC_BAT_TIMES-2));
}
#endif
