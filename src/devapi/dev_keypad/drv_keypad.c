/********************************Copyright ( c ) ********************************
**                  DAQU 
**                         
**                        
**
**Created By:         
** Created Date:     
** Version:        
** Description:    该文件包含按键的驱动相关接口
  
****************************************************************************/
#include "devglobal.h"
#include "drv_keypad.h"
#if 0
#define KEYPAD_ROW0     GPIO_PIN_PTC12
#define KEYPAD_ROW1     GPIO_PIN_PTC13
#define KEYPAD_ROW2     GPIO_PIN_PTC14
#define KEYPAD_ROW3     GPIO_PIN_PTC15
#define KEYPAD_ROW4     GPIO_PIN_PTC10

#define KEYPAD_COL0    GPIO_PIN_PTC3
#define KEYPAD_COL1    GPIO_PIN_PTC4
#define KEYPAD_COL2    GPIO_PIN_PTC9
#endif

#if(KEYPAD_EXIST==1)
//#define POWKEY_PIN      GPIO_PIN_PTH9
//#define POWONOFF_PIN    GPIO_PIN_PTH8//GPIO_PIN_PTC5    //GPIO_PIN_PTC1

#if 1
const str_keypad_pin_t str_keypad_pin_row_tab[KEYPAD_ROW_MAX] =
{
    {GPIO_PIN_PTC1,   1},         //ROW0
    {GPIO_PIN_PTA5,   5},         //ROW1
    {GPIO_PIN_PTC3,   3},         //ROW2
    {GPIO_PIN_PTA9,   9},         //ROW3

};
const str_keypad_pin_t str_keypad_pin_col_tab[KEYPAD_COL_MAX] =
{
    {GPIO_PIN_NONE, 15},       //COL0
    {GPIO_PIN_NONE, 14},       //COL1
    {GPIO_PIN_NONE,   5},       //COL2
    {GPIO_PIN_NONE,   9},       //COL3
};


#else
const str_keypad_pin_t str_keypad_pin_row_tab[KEYPAD_ROW_MAX] =
{
    {GPIO_PIN_PTC12, 12},       //ROW0
    {GPIO_PIN_PTC13, 13},       //ROW1
    {GPIO_PIN_PTC10, 10},       //ROW2
    {GPIO_PIN_PTA4,   4},       //ROW3
    {GPIO_PIN_PTB0,   0},       //ROW4  
};
const str_keypad_pin_t str_keypad_pin_col_tab[KEYPAD_COL_MAX] =
{
    {GPIO_PIN_PTA15, 15},       //COL0
    {GPIO_PIN_PTC14, 14},       //COL1
    {GPIO_PIN_PTA5,   5},       //COL2
    {GPIO_PIN_PTC9,   9},       //COL3
};
#endif
str_keypad_pin_t g_str_keypad_pin_row[KEYPAD_ROW_MAX];
static u8 g_keypad_intflg = 0;

/*

*/
void drv_keypad_irq_enable(void)
{
    s32 i;

    g_keypad_intflg = 0;
    drv_keypad_rowout(0);   //输出0;
    drv_keypad_colin();     //输入
    for(i=0; i<KEYPAD_COL_MAX; i++)
    {
        dev_gpio_irq_clrflg(str_keypad_pin_col_tab[i].m_gpiopin);
        dev_gpio_irq_enable(str_keypad_pin_col_tab[i].m_gpiopin);
    }
}
void drv_keypad_irq_disable(void)
{
    s32 i;
    
    for(i=0; i<KEYPAD_COL_MAX; i++)
    {
        dev_gpio_irq_disable(str_keypad_pin_col_tab[i].m_gpiopin);
        dev_gpio_irq_clrflg(str_keypad_pin_col_tab[i].m_gpiopin);
    }
}
u8 drv_keypad_get_intflg(void)
{
    return g_keypad_intflg;
}
u8 drv_keypad_clear_intflg(void)
{
    g_keypad_intflg = 0;
}
void drv_keypad_handler(void *param)
{
    g_keypad_intflg = 1;
    drv_keypad_irq_disable();
}

/****************************************************************************
**Description:        keypad初始化
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170524
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void drv_keypad_init(void)
{
    s32 i;
    u8 machineid;

    for(i=0; i<KEYPAD_ROW_MAX; i++)
    {
        g_str_keypad_pin_row[i].m_gpiopin = str_keypad_pin_row_tab[i].m_gpiopin;
        g_str_keypad_pin_row[i].m_pinmask = str_keypad_pin_row_tab[i].m_pinmask;
    }
    #if 0
    machineid = dev_misc_getmachinetypeid();
    if((machineid == MACHINE_TYPE_ID_T3)
     ||(machineid == MACHINE_TYPE_ID_T1))
    {
        g_str_keypad_pin_row[4].m_gpiopin = GPIO_PIN_NONE;
        g_str_keypad_pin_row[4].m_pinmask = 32;
    }
    #endif
    
    //ROW管脚配置
    for(i=0; i<KEYPAD_ROW_MAX; i++)
    {
        //GPIO口配置成输入状态
        dev_gpio_config_mux(g_str_keypad_pin_row[i].m_gpiopin, MUX_CONFIG_GPIO);
        if(g_str_keypad_pin_row[i].m_gpiopin == GPIO_PIN_PTA9)//SCI 接口需要特殊处理
        {
            dev_gpio_set_pad(g_str_keypad_pin_row[i].m_gpiopin, PAD_CTL_PULL_NONE);
        }
        else
        {
            dev_gpio_set_pad(g_str_keypad_pin_row[i].m_gpiopin, PAD_CTL_PULL_UP);
        }
        
        dev_gpio_direction_input(g_str_keypad_pin_row[i].m_gpiopin);
    }
    //COL管脚配置
    for(i=0; i<KEYPAD_COL_MAX; i++)
    {
        //GPIO口配置成输入状态
        dev_gpio_config_mux(str_keypad_pin_col_tab[i].m_gpiopin, MUX_CONFIG_GPIO);
        dev_gpio_set_pad(str_keypad_pin_col_tab[i].m_gpiopin, PAD_CTL_PULL_UP);
        dev_gpio_direction_input(str_keypad_pin_col_tab[i].m_gpiopin);
        //申请中断
        dev_gpio_irq_request(str_keypad_pin_col_tab[i].m_gpiopin, drv_keypad_handler, IRQ_ISR_FALLING, NULL);
    }
    g_keypad_intflg = 0;

}
/****************************************************************************
**Description:        ROW输入
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170524
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void drv_keypad_rowin(void)
{
    s32 i;

    for(i=0; i<KEYPAD_ROW_MAX; i++)
    {
        //GPIO口配置成输入状态
        dev_gpio_direction_input(g_str_keypad_pin_row[i].m_gpiopin);
    }  
}
/****************************************************************************
**Description:        ROW输出
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170524
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void drv_keypad_rowout(u8 value)
{
    #if 0
    s32 i;

    for(i=0; i<KEYPAD_ROW_MAX; i++)
    {
        //GPIO口配置成输入状态
        dev_gpio_direction_output(g_str_keypad_pin_row[i].m_gpiopin, value);
    }  
    #endif
}
/****************************************************************************
**Description:        ROW read
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170524
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
u32 drv_keypad_rowread(void)
{
    s32 i;
    u32 ret=0;

    for(i=0; i<KEYPAD_ROW_MAX; i++)
    {
        if(g_str_keypad_pin_row[i].m_gpiopin != GPIO_PIN_NONE)  //兼容T3 4x4键盘
        {
            //GPIO口配置成输入状态
            if(0 == dev_gpio_get_value(g_str_keypad_pin_row[i].m_gpiopin))
            {
                ret |= (1<<i);
            }
        }
    } 
    return ret;
}

/**
 * @brief 判断keycode键是否是多按键
 * @param[in] key_value
 * @retval  DDI_OK 是
 * @retval  DDI_ERR 否
 */
s32 drv_keypad_row_ismultiplekey(u32 key_value)
{
    s32 i;
    s32 cnt = 0;
    u32 ret=0;

    for(i=0; i<KEYPAD_ROW_MAX; i++)
    {
        if((key_value >> i)&0x01)
        {
            cnt++;
        }
        
        if(cnt > 1)
        {
            return 1;
        }
    } 
    return ret;
}

/****************************************************************************
**Description:        COL输入
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170524
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void drv_keypad_colin(void)
{
    s32 i;

    for(i=0; i<KEYPAD_COL_MAX; i++)
    {
        //GPIO口配置成输入状态
        dev_gpio_direction_input(str_keypad_pin_col_tab[i].m_gpiopin);
    }  
}
/****************************************************************************
**Description:        COL输出
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170524
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void drv_keypad_colout(u8 value)
{
    s32 i;

    for(i=0; i<KEYPAD_COL_MAX; i++)
    {
        //GPIO口配置成输出状态
        dev_gpio_direction_output(str_keypad_pin_col_tab[i].m_gpiopin, value);
    }  
}
/****************************************************************************
**Description:        COL read
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170524
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
u32 drv_keypad_colread(void)
{
    s32 i;
    u32 ret=0;

    for(i=0; i<KEYPAD_COL_MAX; i++)
    {
        //GPIO口配置成输入状态
        if(0 == dev_gpio_get_value(str_keypad_pin_col_tab[i].m_gpiopin))
        {
            ret |= (1<<i);
        }
    }  
    return ret;
}
/****************************************************************************
**Description:        获取电源键状态
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170525
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
u32 drv_keypad_get_powerkey(void)
{
    return 0;
}
/****************************************************************************
**Description:        控制电源键锁存
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170525
**----------------------------------------------------------------------------------------------------------------
** Modified by:    sxl 20190626 T6不能通过POWONOFF_PIN管脚来复位AP
****************************************************************************/
void drv_power_set_latch(u8 flg)
{

}


#endif

