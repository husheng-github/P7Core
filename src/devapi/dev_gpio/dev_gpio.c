/********************************Copyright ( c ) ********************************
**                  DAQU 
**                         
**                        
**
**Created By:         
** Created Date:     
** Version:        
** Description:    该文件包含IO的申请、配置、输入、输出、释放等管脚相关的函数重定义，
                统一内部IO的调用、配置接口，便于不同机型的IO口调配。
  
****************************************************************************/
#include "devglobal.h"
#include "sdk/mhscpu_sdk.h"

#define PORT_TYPE_MAX GPIOPIN_TYPE_MAX
#define PORT_PIN_MAX  GPIOPIN_PORTNO_MAX

#if(MH1903_TYPE==MH1903_121)
static GPIO_TypeDef *const port_type_tab[PORT_TYPE_MAX] = {GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF, GPIOG, GPIOH};
const IRQn_Type gpio_irqn_type_tab[PORT_TYPE_MAX] =
{
    EXTI0_IRQn, EXTI1_IRQn, EXTI2_IRQn, EXTI3_IRQn, EXTI4_IRQn, EXTI5_IRQn, EXTI6_IRQn, EXTI7_IRQn
};
#else
static GPIO_TypeDef *const port_type_tab[PORT_TYPE_MAX] = {GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF};
const IRQn_Type gpio_irqn_type_tab[PORT_TYPE_MAX] =
{
    EXTI0_IRQn, EXTI1_IRQn, EXTI2_IRQn, EXTI3_IRQn, EXTI4_IRQn, EXTI5_IRQn
};
#endif
static str_irq_handler_t g_gpioirq_handler[PORT_TYPE_MAX][PORT_PIN_MAX];
/****************************************************************************
**Description:        管脚功能配置申请
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:         
**-------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_gpio_config_mux(iomux_pin_name_t pin, iomux_pin_cfg_t cfg)
{  
    u32 portnum;  
    u32 pin_num;
    //GPIO_TypeDef *lp_port_type;
    
    //获取端口
    portnum = IOMUX_TO_TYPE(pin);
  #if 0
    if(portnum == GPIOPIN_TYPE_EXT)
    {
        return DEVSTATUS_SUCCESS;
    }
  #endif
    if(portnum >= PORT_TYPE_MAX)
    {
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    //lp_port_type = port_type_tab[i];

    //判断配置
    if(cfg >= MUX_CONFIG_END)
    {
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    //获取对应管脚号
    pin_num = IOMUX_TO_PORT(pin);
    if(pin_num >= GPIOPIN_PORTNO_MAX)
    {
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    #if 0  
    GPIO_PinRemapConfig(lp_port_type, pin_num, (GPIORemap_TypeDef)cfg);
    #else
    GPIO->ALT[portnum] &= ~(0x03U<<(pin_num<<1));
    GPIO->ALT[portnum] |= (cfg<<(pin_num<<1));
    #endif
    
    return DEVSTATUS_SUCCESS;
    
}
/****************************************************************************
**Description:        管脚属性配置
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:         
**-------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_gpio_set_pad(iomux_pin_name_t pin, u32 config)
{
    u32 i;
    GPIO_TypeDef *lp_port_type;
    u32 pin_num;

    //获取端口
    i = IOMUX_TO_TYPE(pin);
  #if 0
    if(i == GPIOPIN_TYPE_EXT)
    {
        return DEVSTATUS_SUCCESS;
    }
  #endif
    if(i >= PORT_TYPE_MAX)
    {
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    lp_port_type = port_type_tab[i];

    //获取对应管脚号
    pin_num = IOMUX_TO_PORT(pin);
    if(pin_num >= GPIOPIN_PORTNO_MAX)
    {
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    //配置管脚
    //------------------------------
  #if 0  
    //控制上下拉
    if(config&PAD_CTL_PULL_MASK)
    {
        //需要上下拉(PE=1)
        if(config&PAD_CTL_PULL_UP)
        {
            //上拉(PS=1)
            lp_port_type->PUE |= (0x01<<pin_num);
        }
        else
        {
            lp_port_type->PUE &= ~(0x01<<pin_num);
        }
    }
    else
    {
        lp_port_type->PUE &= pin_num;
    }
  #endif
    //MH1903只有上拉使能配置
    if(config&PAD_CTL_PULL_UP)
    {
        //上拉(PS=1)
        lp_port_type->PUE |= (0x01<<pin_num);
    }
    else
    {
        lp_port_type->PUE &= ~(0x01<<pin_num);
    }
    //开路使能
    if(config&PAD_CTL_ODE_EN)
    {
        lp_port_type->OEN |= (0x01<<pin_num);    
        lp_port_type->IODR &= ~(0x01<<pin_num);
    }
    else
    {
        lp_port_type->IODR |= (0x01<<pin_num);
    }
    //------------------------------
   #if 0 
    //转换速率(b2)
    if(config&PAD_CTL_SLEW_HI)
    {
    }
    else
    {
    }
    //------------------------------
    //无源滤波使能(b3)
    if(config&PAD_CTL_PFE_EN)
    {
    }
    else
    {
    } 
    //------------------------------
    //开路使能(b4)
    if(config&PAD_CTL_ODE_EN)
    {
        lp_port_type->OEN |= pin_num;
    }
    else
    {
        lp_port_type->OEN &= ~pin_num;
    }
  #endif 
    
    return DEVSTATUS_SUCCESS;
    
}
/****************************************************************************
**Description:        GPIO口配置成输出
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:         
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_gpio_direction_output(iomux_pin_name_t pin, u32 value)
{
    u32 i;
    GPIO_TypeDef *lp_gpio_type;
    u32 pin_num;
    
    //获取端口
    i = IOMUX_TO_TYPE(pin);
  #if 0
    if(i == GPIOPIN_TYPE_EXT)
    {
        //获取对应管脚号
        pin_num = IOMUX_TO_PORT(pin);
        return dev_gpioex_set_value(pin_num, value);
        //return DEVSTATUS_SUCCESS;
    }
  #endif
    if(i >= PORT_TYPE_MAX)  //扩展IO口暂不处理
    {
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    lp_gpio_type = port_type_tab[i];

    //获取对应管脚号
    pin_num = IOMUX_TO_PORT(pin);
    if(pin_num >= GPIOPIN_PORTNO_MAX)
    {
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    
    //lp_gpio_type->IODR &= ~(1<<pin_num);//OD输出
    //lp_gpio_type->IODR |= (1<<pin_num);//PP输出
    
    //BSSR[15:0]:对应为1的位置1,其它不变
    //BSST[31:16]:对应为1的位清0,其它不变
    if(value)
    {
        //输出1
        lp_gpio_type->BSRR = (1<<pin_num);
    }
    else
    {
        //输出0
        lp_gpio_type->BSRR = (0x10000<<pin_num);  
    }
    lp_gpio_type->OEN &= ~(1<<pin_num);
    
    return DEVSTATUS_SUCCESS;
    
}
/****************************************************************************
**Description:        GPIO口配置成输入
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:         
**---------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_gpio_direction_input(iomux_pin_name_t pin)
{
    u32 i;
    GPIO_TypeDef *lp_gpio_type;
    u32 pin_num;
    
    //获取端口
    i = IOMUX_TO_TYPE(pin);
    if(i >= PORT_TYPE_MAX)  //扩展IO口暂不处理
    {
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    lp_gpio_type = port_type_tab[i];

    //获取对应管脚号
    pin_num = IOMUX_TO_PORT(pin);
    if(pin_num >= GPIOPIN_PORTNO_MAX)
    {
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    
    lp_gpio_type->OEN |= (1<<pin_num);
    
    return DEVSTATUS_SUCCESS;
    
}
/****************************************************************************
**Description:        GPIO口输出
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:         
**---------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_gpio_set_value(iomux_pin_name_t pin, s32 value)
{
    u32 i;
    GPIO_TypeDef *lp_gpio_type;
    u32 pin_num;
    
    if(pin == GPIO_PIN_NONE)
    {
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    //获取端口
    i = IOMUX_TO_TYPE(pin);
  #if 0
    if(i == GPIOPIN_TYPE_EXT)
    {
        //获取对应管脚号
        pin_num = IOMUX_TO_PORT(pin);
        return dev_gpioex_set_value(pin_num, value);
    }
  #endif
    if(i >= PORT_TYPE_MAX)  
    {
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    lp_gpio_type = port_type_tab[i];

    //获取对应管脚号
    pin_num = IOMUX_TO_PORT(pin);
  #if 0   //减少判断，提高速度
    if(pin_num >= GPIOPIN_PORTNO_MAX)
    {
        return DEVSTATUS_ERR_PARAM_ERR;
    }
  #endif 
    //BSSR[15:0]:对应为1的位置1,其它不变
    //BSST[31:16]:对应为1的位清0,其它不变
    if(value)
    {
        //输出1
        lp_gpio_type->BSRR = (1<<pin_num);
    }
    else
    {
        //输出0
        lp_gpio_type->BSRR = (0x10000<<pin_num);  
    }
    
    return DEVSTATUS_SUCCESS;
    
}
/****************************************************************************
**Description:         读取GPIO口值
**Input parameters:    
**Output parameters: 
**Returned value:   0: 低
                    非0: 高
**Created by:         
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_gpio_get_value(iomux_pin_name_t pin)
{
    u32 i;
    GPIO_TypeDef *lp_gpio_type;
    u32 pin_num;
    
    if(pin == GPIO_PIN_NONE)
    {
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    //获取端口
    i = IOMUX_TO_TYPE(pin);
    if(i >= PORT_TYPE_MAX)
    {
        return 0;
    }
 
    lp_gpio_type = port_type_tab[i];

    //获取对应管脚号
    pin_num = IOMUX_TO_PORT(pin);

    //IODR[15~0]:对应输出数据寄存器
    //IODR[31:16]:对应输入数据寄存器
    return ((lp_gpio_type->IODR)>>(pin_num+16))&0x01;
    
}
/****************************************************************************
**Description:     管脚统一配置
**Input parameters:
**Output parameters: 
**Returned value:  
**Created by:         
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_gpio_config(iomux_pin_name_t pin, iomux_pin_cfg_t cfg, u32 config, u8 direct, u8 value)
{
    dev_gpio_config_mux(pin, cfg);
    dev_gpio_set_pad(pin, config);
    if(MUX_CONFIG_GPIO == cfg)
    {
        if(direct)
        {
            //输入
            dev_gpio_direction_input(pin);
        }
        else
        {
            //输出
            dev_gpio_direction_output(pin, value);
        }
    }
    return 0;
}
/****************************************************************************
**Description:         gpio PORT中断总处理函数入口
**Input parameters:    
**Output parameters: 
**Returned value:    
**Created by:         
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
static void EXTIx_IRQHandler(u8 portnum)
{
    s32 i;
    u32 val;
    u32 pinmask = 1;
//    GPIO_MODULE_TypeDef *lp_port_type;
    u32 data[2];

//    lp_port_type = GPIO;//port_type_tab[portnum];
    val = GPIO->INTP[portnum]; 
    if(0 == (val&0x01))
    {
        //没有中断,暂不处理
        
    }
    data[0] = gpio_irqn_type_tab[portnum];            //中断号
    //获取中断标志ISF
    val = GPIO->INTP_TYPE_STA[portnum].INTP_STA;
    for(i=0; i<PORT_PIN_MAX; i++)
    {
        if(val&pinmask)
        {
            data[1] = i;        //管脚号
            //执行中断
            g_gpioirq_handler[portnum][i].m_func((void *)data);
            //清中断标志
            GPIO->INTP_TYPE_STA[portnum].INTP_STA = pinmask;
        }
        pinmask <<=1;
    }    
}
/****************************************************************************
**Description:         gpio PORTA中断入口
**Input parameters:    
**Output parameters: 
**Returned value:    
**Created by:         
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void EXTI0_IRQHandler(void)
{    
    //dev_debug_printf("%s(%d):enter EXTI0_IRQHandler\r\n", __FUNCTION__, __LINE__);   
    EXTIx_IRQHandler(0);
    EXTI_ClearITPendingBit(EXTI_Line0);
	NVIC_ClearPendingIRQ(EXTI0_IRQn);
}
/****************************************************************************
**Description:         gpio PORTB中断入口
**Input parameters:    
**Output parameters: 
**Returned value:    
**Created by:         
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void EXTI1_IRQHandler(void)
{    
    //dev_debug_printf("%s(%d):enter EXTI1_IRQHandler\r\n", __FUNCTION__, __LINE__);   
    EXTIx_IRQHandler(1);
    EXTI_ClearITPendingBit(EXTI_Line1);
	NVIC_ClearPendingIRQ(EXTI1_IRQn);
}
/****************************************************************************
**Description:         gpio PORTC中断入口
**Input parameters:    
**Output parameters: 
**Returned value:    
**Created by:         
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void EXTI2_IRQHandler(void)
{
    //dev_debug_printf("%s(%d):enter EXTI2_IRQHandler\r\n", __FUNCTION__, __LINE__);   
    EXTIx_IRQHandler(2);
    EXTI_ClearITPendingBit(EXTI_Line2);
	NVIC_ClearPendingIRQ(EXTI2_IRQn);
}
/****************************************************************************
**Description:         gpio PORTD中断入口
**Input parameters:    
**Output parameters: 
**Returned value:    
**Created by:         
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void EXTI3_IRQHandler(void)
{    
    //dev_debug_printf("%s(%d):enter EXTI3_IRQHandler\r\n", __FUNCTION__, __LINE__);   
    EXTIx_IRQHandler(3);
    EXTI_ClearITPendingBit(EXTI_Line3);
	NVIC_ClearPendingIRQ(EXTI3_IRQn);
}
void EXTI4_IRQHandler(void)
{    
    //dev_debug_printf("%s(%d):enter EXTI4_IRQHandler\r\n", __FUNCTION__, __LINE__);   
    EXTIx_IRQHandler(4);
    EXTI_ClearITPendingBit(EXTI_Line4);
	NVIC_ClearPendingIRQ(EXTI4_IRQn);
}
void EXTI5_IRQHandler(void)
{    
//    dev_debug_printf("%s(%d):enter EXTI5_IRQHandler\r\n", __FUNCTION__, __LINE__);   
    //dev_debug_printf("EXTI5_GPIO_Status %08X\n", EXTI_GetITLineStatus(EXTI_Line5));
    
    EXTIx_IRQHandler(5);
    EXTI_ClearITPendingBit(EXTI_Line5);
    NVIC_ClearPendingIRQ(EXTI5_IRQn);   
}

#if(MH1903_TYPE==MH1903_121)
void EXTI6_IRQHandler(void)
{    
    //dev_debug_printf("%s(%d):enter EXTI6_IRQHandler\r\n", __FUNCTION__, __LINE__);   
    EXTIx_IRQHandler(6);
    EXTI_ClearITPendingBit(EXTI_Line6);
    NVIC_ClearPendingIRQ(EXTI6_IRQn);
}

void EXTI7_IRQHandler(void)
{
    
    //dev_debug_printf("%s(%d):enter EXTI7_IRQHandler\r\n", __FUNCTION__, __LINE__); 
    //dev_debug_printf("EXTI7_GPIO_Status %08X\n", EXTI_GetITLineStatus(EXTI_Line7));
    
    EXTIx_IRQHandler(7);

     EXTI_ClearITPendingBit(EXTI_Line7);
    NVIC_ClearPendingIRQ(EXTI7_IRQn);
}
#endif
/****************************************************************************
**Description:         gpio中断申请
**Input parameters:    
**Output parameters: 
**Returned value:    
**Created by:         
**-------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_gpio_irq_request(iomux_pin_name_t pin, void (*handler)(void* ), u32 flags, void *data)
{  
    u32 portnum;
    u32 pin_num;    
    u32 mode;

    dev_gpio_init();
    //获取端口
    portnum = IOMUX_TO_TYPE(pin);
    if(portnum >= PORT_TYPE_MAX)
    {
        return DEVSTATUS_ERR_PARAM_ERR;
    }    

    //获取对应管脚号
    pin_num = IOMUX_TO_PORT(pin);
    if(pin_num >= GPIOPIN_PORTNO_MAX)
    {
        return DEVSTATUS_ERR_PARAM_ERR;
    }  
    
    //关闭中断
    GPIO->INTP_TYPE_STA[portnum].INTP_TYPE &= ~(0x03<<(pin_num<<1));
    //清中断(写1清0)
    GPIO->INTP_TYPE_STA[portnum].INTP_STA = (1<<pin_num);
    //配置中断模式
    mode = 0;
    if(flags&IRQ_ISR_RISING)
    {
        //上升沿触发
        mode |= 0x01;
    }
    if(flags&IRQ_ISR_FALLING)
    {
        //下降沿触发
        mode |= 0x02;
    }
    //配置中断
    GPIO->INTP_TYPE_STA[portnum].INTP_TYPE |= (mode<<(pin_num<<1));
    //配置中断服务程序
    if(handler != NULL)
    {
        g_gpioirq_handler[portnum][pin_num].m_data = data;
        g_gpioirq_handler[portnum][pin_num].m_func = handler;
        g_gpioirq_handler[portnum][pin_num].m_param = mode;
    }
    
    return 0;
}
/****************************************************************************
**Description:         gpio清除中断标志
**Input parameters:    
**Output parameters: 
**Returned value:    
**Created by:         
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void dev_gpio_irq_clrflg(iomux_pin_name_t pin)
{
    u32 portnum;
    u32 pin_num;  
    
     //获取端口
    portnum = IOMUX_TO_TYPE(pin);
     
    //获取对应管脚号
    pin_num = IOMUX_TO_PORT(pin);
    //清中断标志
    GPIO->INTP_TYPE_STA[portnum].INTP_STA = (0x01<<pin_num);
}
/****************************************************************************
**Description:         gpio禁止中断
**Input parameters:    
**Output parameters: 
**Returned value:    
**Created by:         
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void dev_gpio_irq_disable(iomux_pin_name_t pin)
{
    u32 portnum;
    u32 pin_num;  
    
     //获取端口
    portnum = IOMUX_TO_TYPE(pin);
     
    //获取对应管脚号
    pin_num = IOMUX_TO_PORT(pin);
    GPIO->INTP_TYPE_STA[portnum].INTP_TYPE &= ~(0x03<<(pin_num<<1));
}
/****************************************************************************
**Description:         gpio允许中断
**Input parameters:    
**Output parameters: 
**Returned value:    
**Created by:         
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void dev_gpio_irq_enable(iomux_pin_name_t pin)
{
    s32 mode;
    u32 portnum;
    u32 pin_num; 
    
    //获取端口
    portnum = IOMUX_TO_TYPE(pin);
    
    //获取对应管脚号
    pin_num = IOMUX_TO_PORT(pin);
    
    //配置/开中断
    mode = g_gpioirq_handler[portnum][pin_num].m_param;
    
    GPIO->INTP_TYPE_STA[portnum].INTP_TYPE |= (mode<<(pin_num<<1));
}
/****************************************************************************
**Description:         gpio释放中断
**Input parameters:    
**Output parameters: 
**Returned value:    
**Created by:         
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_gpio_irq_free(iomux_pin_name_t pin)
{
    u32 portnum;
    u32 pin_num;  

    //获取端口
    portnum = IOMUX_TO_TYPE(pin);
    if(portnum >= PORT_TYPE_MAX)
    {
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    //获取对应管脚号
    pin_num = IOMUX_TO_PORT(pin);
    if(pin_num >= GPIOPIN_PORTNO_MAX)
    {
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    //关闭中断
    GPIO->INTP_TYPE_STA[portnum].INTP_TYPE &= ~(0x03<<(pin_num<<1));

    //配置中断服务程序
    g_gpioirq_handler[portnum][pin_num].m_data = NULL;
    g_gpioirq_handler[portnum][pin_num].m_func = dev_irq_default_isr;
    g_gpioirq_handler[portnum][pin_num].m_param = 0;
    return 0;
}
/****************************************************************************
**Description:         gpio中断唤醒使能控制
**Input parameters:    
**Output parameters: 
**Returned value:    
**Created by:         
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_gpio_wakeup_ctl(iomux_pin_name_t pin, u8 flg)
{
    u32 portnum;
    u32 pin_num; 
    u32 pin_mask;

    //获取端口
    portnum = IOMUX_TO_TYPE(pin);
    if(portnum >= PORT_TYPE_MAX)
    {
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    //获取对应管脚号
    pin_num = IOMUX_TO_PORT(pin);
    if(pin_num >= GPIOPIN_PORTNO_MAX)
    {
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    //GPIO_WeakEvenConfig(GPIOC, GPIO_Pin_0, ENABLE);
    pin_mask = 0x01<<pin_num;
    
    switch(portnum)
    {
    case 0:     //GPIOA
        if(flg) //允许唤醒
        {
            GPIO->WAKE_P0_EN |= (pin_mask);
        }
        else
        {
            GPIO->WAKE_P0_EN &= ~(pin_mask);
        }
        break;
    case 1:    //GPIOB
        if(flg) //允许唤醒
        {
            GPIO->WAKE_P0_EN |= (pin_mask<<16);
        }
        else
        {
            GPIO->WAKE_P0_EN &= ~(pin_mask<<16);
        }
        break;

    case 2:    // GPIOC
        if(flg) //允许唤醒
        {
            GPIO->WAKE_P1_EN |= (pin_mask);
        }
        else
        {
            GPIO->WAKE_P1_EN &= ~(pin_mask);
        }
        break;
    case 3:// GPIOD
        if(flg) //允许唤醒
        {
            GPIO->WAKE_P1_EN |= (pin_mask<<16);
        }
        else
        {
            GPIO->WAKE_P1_EN &= ~(pin_mask<<16);
        }
        break;
    case 4:    // GPIOE
        if(flg) //允许唤醒
        {
            GPIO->WAKE_P2_EN |= (pin_mask);
        }
        else
        {
            GPIO->WAKE_P2_EN &= ~(pin_mask);
        }
        break;
    case 5:// GPIOF
        if(flg) //允许唤醒
        {
            GPIO->WAKE_P2_EN |= (pin_mask<<16);
        }
        else
        {
            GPIO->WAKE_P2_EN &= ~(pin_mask<<16);
        }
        break;
#if(MH1903_TYPE==MH1903_121)
    case 6:    // GPIOG
        if(flg) //允许唤醒
        {
            GPIO->WAKE_P3_EN |= (pin_mask);
        }
        else
        {
            GPIO->WAKE_P3_EN &= ~(pin_mask);
        }
        break;
    case 7:// GPIOH
        if(flg) //允许唤醒
        {
            GPIO->WAKE_P3_EN |= (pin_mask<<16);
        }
        else
        {
            GPIO->WAKE_P3_EN &= ~(pin_mask<<16);
        }
        break;
#endif
    }     
    return 0;
    
}
/****************************************************************************
**Description:         gpio中断初始化
**Input parameters:    
**Output parameters: 
**Returned value:    
**Created by:         
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void dev_gpio_init(void)
{
    static u8 flag=1;
    u32 i,j;

    if(!flag)
    {
        return;
    }
    flag = 0;
    //打开GPIO时钟
    SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_GPIO,ENABLE);
    //SYSCTRL_APBPeriphResetCmd(SYSCTRL_APBPeriph_GPIO,ENABLE);
    
    //关闭所有外部中断使能
    EXTI_DeInit();
    //清除所有外部挂起中断
    NVIC_ClearPendingIRQ(EXTI0_IRQn);
    NVIC_ClearPendingIRQ(EXTI1_IRQn);
    NVIC_ClearPendingIRQ(EXTI2_IRQn);
    NVIC_ClearPendingIRQ(EXTI3_IRQn);
    NVIC_ClearPendingIRQ(EXTI4_IRQn);
    NVIC_ClearPendingIRQ(EXTI5_IRQn);
#if(MH1903_TYPE==MH1903_121)
    NVIC_ClearPendingIRQ(EXTI6_IRQn);
    NVIC_ClearPendingIRQ(EXTI7_IRQn);
#endif
    //清除所有的GPIO中断唤醒源
    GPIO->WAKE_P0_EN = 0;
    GPIO->WAKE_P1_EN = 0;
    GPIO->WAKE_P2_EN = 0;
    GPIO->WAKE_P3_EN = 0;
    //初始化参数
    for(i=0; i<PORT_TYPE_MAX; i++)
    {
        j = GPIO->INTP[i];      
        for(j=0; j<PORT_PIN_MAX; j++)
        {
            g_gpioirq_handler[i][j].m_data = NULL;
            g_gpioirq_handler[i][j].m_func = dev_irq_default_isr;
            g_gpioirq_handler[i][j].m_param = 0;
        }
        GPIO->INTP_TYPE_STA[i].INTP_TYPE = 0x0;  //禁止所有GPIO口中断
    }
    
    //打开GPIO总中断
    NVIC_EnableIRQ(EXTI0_IRQn);
    NVIC_EnableIRQ(EXTI1_IRQn);
    NVIC_EnableIRQ(EXTI2_IRQn);
    NVIC_EnableIRQ(EXTI3_IRQn);
    NVIC_EnableIRQ(EXTI4_IRQn);
    NVIC_EnableIRQ(EXTI5_IRQn);
#if(MH1903_TYPE==MH1903_121)
    NVIC_EnableIRQ(EXTI6_IRQn);
    NVIC_EnableIRQ(EXTI7_IRQn);
#endif
}


#if 0
#define TEST_GPIOTAB_MAX    5
const iomux_pin_name_t test_gpio_tab[]=
{
    GPIO_PIN_PTC2,
    GPIO_PIN_PTB13,
    GPIO_PIN_PTC13,
    GPIO_PIN_PTC14,
    GPIO_PIN_PTC15,
};
#endif

void dev_gpio_test(void)
{
#if 0   
    u8 tmp[8];
    s32 disflg=1;
    s32 num=0;
    s32 ret;

    while(1)
    {
        if(disflg==1)
        {
            dev_debug_printf("====num=%d,(port:%d, pin:%d)====\r\n", num, 
            IOMUX_TO_TYPE(test_gpio_tab[num]), IOMUX_TO_PORT(test_gpio_tab[num]));
            dev_debug_printf("1.config input\r\n");
            dev_debug_printf("2.config output\r\n");
            dev_debug_printf("3.datain\r\n");
            dev_debug_printf("4.dataout0\r\n");
            dev_debug_printf("5.dataout1\r\n");
            dev_debug_printf("+.select pin(+)\r\n");
            dev_debug_printf("-.select pin(-)\r\n");
            disflg = 0;
        }
        ret = dev_com_read(0, tmp, 1);
        if(ret==1)
        {
            switch(tmp[0])
            {
            case '1':  
                dev_gpio_config_mux(test_gpio_tab[num], MUX_CONFIG_GPIO);
                dev_gpio_set_pad(test_gpio_tab[num], PAD_CTL_PULL_UP);
                dev_gpio_direction_input(test_gpio_tab[num]);
                dev_debug_printf("direction_input\r\n");
                break;
            case '2':   
                dev_gpio_config_mux(test_gpio_tab[num], MUX_CONFIG_GPIO);
                dev_gpio_set_pad(test_gpio_tab[num], PAD_CTL_PULL_UP);
                dev_gpio_direction_output(test_gpio_tab[num], 1);
                dev_debug_printf("direction_output1\r\n");
                break; 
            case '3':   
                ret = dev_gpio_get_value(test_gpio_tab[num]);
                dev_debug_printf("read:ret=%d\r\n", ret);
                break; 
            case '4':   
                dev_gpio_set_value(test_gpio_tab[num], 0);
                dev_debug_printf("Output 0\r\n");
                break; 
            case '5':  
                dev_gpio_set_value(test_gpio_tab[num], 1);
                dev_debug_printf("Output 1\r\n");
                break; 
            case '+':   
                num++;
                if(num>=TEST_GPIOTAB_MAX)
                {
                    num = TEST_GPIOTAB_MAX-1;
                }
                dev_debug_printf("====num=%d,(port:%d, pin:%d)====\r\n", num, 
                    IOMUX_TO_TYPE(test_gpio_tab[num]), IOMUX_TO_PORT(test_gpio_tab[num]));
                break; 
            case '-':   
                if(num>0)
                {
                    num--;
                }
                dev_debug_printf("====num=%d,(port:%d, pin:%d)====\r\n", num, 
                    IOMUX_TO_TYPE(test_gpio_tab[num]), IOMUX_TO_PORT(test_gpio_tab[num]));
                
                break; 
            case 'E':
            case 'e':
                return;
                break;
            }
        }
        
    }
#endif    
}


