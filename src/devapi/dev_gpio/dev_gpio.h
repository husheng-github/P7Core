#ifndef __DEV_GPIO_H
#define __DEV_GPIO_H
#include "gpio_pins.h"

typedef struct _str_pincfg
{
    s8 m_lable[32];             //保存管脚名称标号，字符串格式，以"\0"接收，有效字节不能大于31个
    iomux_pin_name_t m_pin;     //管脚名称
    iomux_pin_cfg_t  m_pinfun;  //管脚功能
    u32 m_padcfg;               //管脚属性
    u32 m_ddir;                 //输入/输出
    u32 m_vlue;                 //输出值
    u32 m_extcfg;               //扩展配置，如SPI的高4bit表示使用那个bus，低4bit表示选用那个名称
                                           //对于I2C高4bit表示使用哪个bus，低4bit表示选用哪个名称
    u32 m_rfu;                  //预留 
}str_pincfg_t;

#if 0
typedef struct _str_gpioirq_handler{
    void    *m_data;
    void    (*m_func)(void *data);
}str_gpioirq_handler_t;
#endif

#if 0
typedef enum gpio_irq_config {
    IRQ_ISF_NONE = 0,    /*!< Interrupt Status Flag (ISF) is disabled. */
    IRQ_ISF_DMA_RISING = 1,    /*!< ISF flag and DMA request on rising edge */
    IRQ_ISF_DMQ_FALLING = 2,/*!< ISF flag and DMA request on falling edge */
    IRQ_,    /*!<  */
    IRQ_,    /*!<  */
    IRQ_,    /*!<  */
    IRQ_,    /*!<  */
    IRQ_,    /*!< */
    IRQ_,    /*!< */
    IRQ_ = 8,     
//    MUX_CONFIG_FUNC = MUX_CONFIG_ALT0,
} gpio_irq_config_t;
#endif
#define IRQ_ISR_NONE        0x00000000  //Interrupt Status Flag (ISF) is disabled
#define IRQ_ISR_RISING      0x00000001  //上升沿
#define IRQ_ISR_FALLING     0x00000002  //下下降沿
#define IRQ_ISR_LEVEL_HI    0x00000004  //高电平
#define IRQ_ISR_LEVEL_LO    0x00000008  //低电平
#define IRQ_ISR_DMA         0x00000010  //DMA请求


s32 dev_gpio_config_mux(iomux_pin_name_t pin, iomux_pin_cfg_t cfg);
s32 dev_gpio_set_pad(iomux_pin_name_t pin, u32 config);
s32 dev_gpio_direction_output(iomux_pin_name_t pin, u32 value);
s32 dev_gpio_direction_input(iomux_pin_name_t pin);
s32 dev_gpio_set_value(iomux_pin_name_t pin, s32 value);
s32 dev_gpio_get_value(iomux_pin_name_t pin);
s32 dev_gpio_config(iomux_pin_name_t pin, iomux_pin_cfg_t cfg, u32 config, u8 direct, u8 value);

s32 dev_gpio_irq_request(iomux_pin_name_t pin, void (*handler)(void* ), u32 flags, void *data);
void dev_gpio_irq_clrflg(iomux_pin_name_t pin);
void dev_gpio_irq_disable(iomux_pin_name_t pin);
void dev_gpio_irq_enable(iomux_pin_name_t pin);
s32 dev_gpio_irq_free(iomux_pin_name_t pin);
s32 dev_gpio_wakeup_ctl(iomux_pin_name_t pin, u8 flg);
void dev_gpio_init(void);
void dev_gpio_test(void);

s32 dev_gpioex_set_value(u32 pin, u32 value);

#endif
