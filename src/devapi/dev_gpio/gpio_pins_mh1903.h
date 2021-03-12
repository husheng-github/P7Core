#ifndef __GPIO_PINS_MH1903_H
#define __GPIO_PINS_MH1903_H
/***********************************************************
*  31 | 30 - 27 | 26 - 16 | 15 - 10  |   9 - 0  
* RUF   IO_TYPE    RUF      PORT_NO       RUF
* 说明: IO_TYPE:管脚类型代码： 0：PTA 对应GPIO编号  (PTA0~PTA15)
                               1：PTB 对应GPIO编号  (PTB0~PTB15)
                               2：PTC 对应GPIO编号  (PTC0~PTC15)
                               3：PTD 对应GPIO编号  (PTD0~PTD15)
                               4：PTE 对应GPIO编号  (PTE0~PTE15)
                               5：PTF 对应GPIO编号  (PTF0~PTF15)
                               6：PTG 对应GPIO编号  (PTG0~PTG15)
                               7：PTH 对应GPIO编号  (PTH0~PTH15)
                               10: 扩展GPIO口          
        PORT_NO:管脚组编号:    管脚对应PTA、PTB...的编号
        PAD_NUM:管脚统一编号(保留)
***********************************************************/
#define GPIOPIN_TYPE           27
#define GPIOPIN_TYPE_nb         4    
#define GPIOPIN_TYPE_PTA        0  
#define GPIOPIN_TYPE_PTB        1  
#define GPIOPIN_TYPE_PTC        2  
#define GPIOPIN_TYPE_PTD        3  
#define GPIOPIN_TYPE_PTE        4 
#define GPIOPIN_TYPE_PTF        5 
#define GPIOPIN_TYPE_PTG        6 
#define GPIOPIN_TYPE_PTH        7
#define GPIOPIN_TYPE_EXT        8

#if(MH1903_TYPE == MH1903_88)
#define GPIOPIN_TYPE_MAX        6   
#else
#define GPIOPIN_TYPE_MAX        8   
#endif

#define GPIOPIN_PORTNO         10
#define GPIOPIN_PORTNO_nb        6
#define GPIOPIN_PORTNO_MAX      16  //32

#define GPIOPIN_PADNO          0
//#define GPIOPIN_PADNO_MAX    (24+143)
#define GPIOPIN_PADNO_nb             10 
//管脚编号分类定义

//
#define IOMUX_TO_TYPE(pin)      ((pin>>GPIOPIN_TYPE)&((1<<GPIOPIN_TYPE_nb)-1))
#define IOMUX_TO_PORT(pin)      ((pin>>GPIOPIN_PORTNO)&((1<<GPIOPIN_PORTNO_nb)-1))
#define IOMUX_TO_PAD(pin)       ((pin>>GPIOPIN_PADNO)&((1<<GPIOPIN_PADNO_nb)-1))

#define _BUILD_GPIO_PIN(t, portno, padno)  \
                ((t<<GPIOPIN_TYPE) | (portno<<GPIOPIN_PORTNO) | (padno<<GPIOPIN_PADNO))



/*!
 * IOMUX functions
 */
typedef enum iomux_pin_config {
    MUX_CONFIG_ALT0 = 0,    /*!< used as alternate function 0 */
    MUX_CONFIG_ALT1,    /*!< used as alternate function 1 */
    MUX_CONFIG_ALT2,    /*!< used as alternate function 2 */
    MUX_CONFIG_ALT3,    /*!< used as alternate function 3 */
//    MUX_CONFIG_ALT4,    /*!< used as alternate function 4 */
//    MUX_CONFIG_ALT5,    /*!< used as alternate function 5 */
//    MUX_CONFIG_ALT6,    /*!< used as alternate function 6 */
//    MUX_CONFIG_ALT7,    /*!< used as alternate function 7 */
    MUX_CONFIG_GPIO = MUX_CONFIG_ALT1,    /*!< used as GPIO */
    MUX_CONFIG_END = 4,     
//    MUX_CONFIG_FUNC = MUX_CONFIG_ALT0,
} iomux_pin_cfg_t;

/*!
 * IOMUX pad 上下拉、迟滞、转化率等属性
 * SW_PAD_CTL
 */
typedef enum iomux_pad_config {
    //控制上下拉(b1~0)    
    PAD_CTL_PULL_NONE = (0),
    PAD_CTL_PULL_UP   = (1<<0),
    PAD_CTL_PULL_DOWN = (2<<0),
    PAD_CTL_PULL_MASK = (3<<0),
    PAD_CTL_PULL_FLOAT = PAD_CTL_PULL_NONE,
    //转换速率(b2)
    PAD_CTL_SLEW_HI   = (1<<2),   //转换速率高
    PAD_CTL_SLEW_LO   = (0),      //转换速率低
    //Passive Filter Enable无源滤波使能(b3)
    PAD_CTL_PFE_EN   = (1<<3),   //无源滤波使能
    PAD_CTL_PFE_DIS  = (0),      //无源滤波禁能
    //Open Drain Enable开路使能(b4)
    PAD_CTL_ODE_EN   = (1<<4),   //开路使能
    PAD_CTL_ODE_DIS  = (0),      //开路禁能
    
}iomux_pad_config_t;

typedef enum iomux_pins{
    //PTA
    GPIO_PIN_PTA0    =    _BUILD_GPIO_PIN(0, 0 , 0 ),  
    GPIO_PIN_PTA1   =    _BUILD_GPIO_PIN(0, 1 , 0 ),
    GPIO_PIN_PTA2   =    _BUILD_GPIO_PIN(0, 2 , 0 ),
    GPIO_PIN_PTA3   =    _BUILD_GPIO_PIN(0, 3 , 0 ),
    GPIO_PIN_PTA4   =    _BUILD_GPIO_PIN(0, 4 , 0 ),
    GPIO_PIN_PTA5   =    _BUILD_GPIO_PIN(0, 5 , 0 ),
    GPIO_PIN_PTA6   =    _BUILD_GPIO_PIN(0, 6 , 0 ),
    GPIO_PIN_PTA7   =    _BUILD_GPIO_PIN(0, 7 , 0 ),
    GPIO_PIN_PTA8   =    _BUILD_GPIO_PIN(0, 8 , 0 ),
    GPIO_PIN_PTA9   =    _BUILD_GPIO_PIN(0, 9 , 0 ),
    GPIO_PIN_PTA10  =    _BUILD_GPIO_PIN(0, 10, 0 ),
    GPIO_PIN_PTA11  =    _BUILD_GPIO_PIN(0, 11, 0 ),
    GPIO_PIN_PTA12  =    _BUILD_GPIO_PIN(0, 12, 0 ),
    GPIO_PIN_PTA13  =    _BUILD_GPIO_PIN(0, 13, 0 ),
    GPIO_PIN_PTA14  =    _BUILD_GPIO_PIN(0, 14, 0 ),
    GPIO_PIN_PTA15  =    _BUILD_GPIO_PIN(0, 15, 0 ),
    //PTB
    GPIO_PIN_PTB0   =    _BUILD_GPIO_PIN(1, 0 , 0 ),
    GPIO_PIN_PTB1   =    _BUILD_GPIO_PIN(1, 1 , 0 ),
    GPIO_PIN_PTB2   =    _BUILD_GPIO_PIN(1, 2 , 0 ),
    GPIO_PIN_PTB3   =    _BUILD_GPIO_PIN(1, 3 , 0 ),
    GPIO_PIN_PTB4   =    _BUILD_GPIO_PIN(1, 4 , 0 ),
    GPIO_PIN_PTB5   =    _BUILD_GPIO_PIN(1, 5 , 0 ),
    GPIO_PIN_PTB6   =    _BUILD_GPIO_PIN(1, 6 , 0 ),
    GPIO_PIN_PTB7   =    _BUILD_GPIO_PIN(1, 7 , 0 ),
    GPIO_PIN_PTB8   =    _BUILD_GPIO_PIN(1, 8 , 0 ),
    GPIO_PIN_PTB9   =    _BUILD_GPIO_PIN(1, 9 , 0 ),
    GPIO_PIN_PTB10  =    _BUILD_GPIO_PIN(1, 10, 0 ),
    GPIO_PIN_PTB11  =    _BUILD_GPIO_PIN(1, 11, 0 ),
    GPIO_PIN_PTB12  =    _BUILD_GPIO_PIN(1, 12, 0 ),
    GPIO_PIN_PTB13  =    _BUILD_GPIO_PIN(1, 13, 0 ),
    GPIO_PIN_PTB14  =    _BUILD_GPIO_PIN(1, 14, 0 ),
    GPIO_PIN_PTB15  =    _BUILD_GPIO_PIN(1, 15, 0 ),
    //PTC
    GPIO_PIN_PTC0   =    _BUILD_GPIO_PIN(2, 0 , 0 ),
    GPIO_PIN_PTC1   =    _BUILD_GPIO_PIN(2, 1 , 0 ),
    GPIO_PIN_PTC2   =    _BUILD_GPIO_PIN(2, 2 , 0 ),
    GPIO_PIN_PTC3   =    _BUILD_GPIO_PIN(2, 3 , 0 ),
    GPIO_PIN_PTC4   =    _BUILD_GPIO_PIN(2, 4 , 0 ),
    GPIO_PIN_PTC5   =    _BUILD_GPIO_PIN(2, 5 , 0 ),
    GPIO_PIN_PTC6   =    _BUILD_GPIO_PIN(2, 6 , 0 ),
    GPIO_PIN_PTC7   =    _BUILD_GPIO_PIN(2, 7 , 0 ),
    GPIO_PIN_PTC8   =    _BUILD_GPIO_PIN(2, 8 , 0 ),
    GPIO_PIN_PTC9   =    _BUILD_GPIO_PIN(2, 9 , 0 ),
    GPIO_PIN_PTC10  =    _BUILD_GPIO_PIN(2, 10, 0 ),
    GPIO_PIN_PTC11  =    _BUILD_GPIO_PIN(2, 11, 0 ),
    GPIO_PIN_PTC12  =    _BUILD_GPIO_PIN(2, 12, 0 ),
    GPIO_PIN_PTC13  =    _BUILD_GPIO_PIN(2, 13, 0 ),
    GPIO_PIN_PTC14  =    _BUILD_GPIO_PIN(2, 14, 0 ),
    GPIO_PIN_PTC15  =    _BUILD_GPIO_PIN(2, 15, 0 ),
    //PTD
    GPIO_PIN_PTD0   =    _BUILD_GPIO_PIN(3, 0 , 0 ),
    GPIO_PIN_PTD1   =    _BUILD_GPIO_PIN(3, 1 , 0 ),
    GPIO_PIN_PTD2   =    _BUILD_GPIO_PIN(3, 2 , 0 ),
    GPIO_PIN_PTD3   =    _BUILD_GPIO_PIN(3, 3 , 0 ),
    GPIO_PIN_PTD4   =    _BUILD_GPIO_PIN(3, 4 , 0 ),
    GPIO_PIN_PTD5   =    _BUILD_GPIO_PIN(3, 5 , 0 ),
    GPIO_PIN_PTD6   =    _BUILD_GPIO_PIN(3, 6 , 0 ),
    GPIO_PIN_PTD7   =    _BUILD_GPIO_PIN(3, 7 , 0 ),
    GPIO_PIN_PTD8   =    _BUILD_GPIO_PIN(3, 8 , 0 ),
    GPIO_PIN_PTD9   =    _BUILD_GPIO_PIN(3, 9 , 0 ),
    GPIO_PIN_PTD10  =    _BUILD_GPIO_PIN(3, 10, 0 ),
    GPIO_PIN_PTD11  =    _BUILD_GPIO_PIN(3, 11, 0 ),
    GPIO_PIN_PTD12  =    _BUILD_GPIO_PIN(3, 12, 0 ),
    GPIO_PIN_PTD13  =    _BUILD_GPIO_PIN(3, 13, 0 ),
    GPIO_PIN_PTD14  =    _BUILD_GPIO_PIN(3, 14, 0 ),
    GPIO_PIN_PTD15  =    _BUILD_GPIO_PIN(3, 15, 0 ),    
    //PTE                                           
    GPIO_PIN_PTE0   =    _BUILD_GPIO_PIN(4, 0 , 0 ),
    GPIO_PIN_PTE1   =    _BUILD_GPIO_PIN(4, 1 , 0 ),
    GPIO_PIN_PTE2   =    _BUILD_GPIO_PIN(4, 2 , 0 ),
    GPIO_PIN_PTE3   =    _BUILD_GPIO_PIN(4, 3 , 0 ),
    GPIO_PIN_PTE4   =    _BUILD_GPIO_PIN(4, 4 , 0 ),
    GPIO_PIN_PTE5   =    _BUILD_GPIO_PIN(4, 5 , 0 ),
    GPIO_PIN_PTE6   =    _BUILD_GPIO_PIN(4, 6 , 0 ),
    GPIO_PIN_PTE7   =    _BUILD_GPIO_PIN(4, 7 , 0 ),
    GPIO_PIN_PTE8   =    _BUILD_GPIO_PIN(4, 8 , 0 ),
    GPIO_PIN_PTE9   =    _BUILD_GPIO_PIN(4, 9 , 0 ),
    GPIO_PIN_PTE10  =    _BUILD_GPIO_PIN(4, 10, 0 ),
    GPIO_PIN_PTE11  =    _BUILD_GPIO_PIN(4, 11, 0 ),
    GPIO_PIN_PTE12  =    _BUILD_GPIO_PIN(4, 12, 0 ),
    GPIO_PIN_PTE13  =    _BUILD_GPIO_PIN(4, 13, 0 ),
    GPIO_PIN_PTE14  =    _BUILD_GPIO_PIN(4, 14, 0 ),
    GPIO_PIN_PTE15  =    _BUILD_GPIO_PIN(4, 15, 0 ),
    //PTF                                            
    GPIO_PIN_PTF0   =    _BUILD_GPIO_PIN(5, 0 , 0 ), 
    GPIO_PIN_PTF1   =    _BUILD_GPIO_PIN(5, 1 , 0 ), 
    GPIO_PIN_PTF2   =    _BUILD_GPIO_PIN(5, 2 , 0 ), 
    GPIO_PIN_PTF3   =    _BUILD_GPIO_PIN(5, 3 , 0 ), 
    GPIO_PIN_PTF4   =    _BUILD_GPIO_PIN(5, 4 , 0 ), 
    GPIO_PIN_PTF5   =    _BUILD_GPIO_PIN(5, 5 , 0 ), 
    GPIO_PIN_PTF6   =    _BUILD_GPIO_PIN(5, 6 , 0 ), 
    GPIO_PIN_PTF7   =    _BUILD_GPIO_PIN(5, 7 , 0 ), 
    GPIO_PIN_PTF8   =    _BUILD_GPIO_PIN(5, 8 , 0 ), 
    GPIO_PIN_PTF9   =    _BUILD_GPIO_PIN(5, 9 , 0 ), 
    GPIO_PIN_PTF10  =    _BUILD_GPIO_PIN(5, 10, 0 ), 
    GPIO_PIN_PTF11  =    _BUILD_GPIO_PIN(5, 11, 0 ), 
    GPIO_PIN_PTF12  =    _BUILD_GPIO_PIN(5, 12, 0 ), 
    GPIO_PIN_PTF13  =    _BUILD_GPIO_PIN(5, 13, 0 ), 
    GPIO_PIN_PTF14  =    _BUILD_GPIO_PIN(5, 14, 0 ), 
    GPIO_PIN_PTF15  =    _BUILD_GPIO_PIN(5, 15, 0 ), 
#if 0//(MH1903_TYPE == MH1903_121)
    //PTG                                           
    GPIO_PIN_PTG0   =    _BUILD_GPIO_PIN(6, 0 , 0 ),
    GPIO_PIN_PTG1   =    _BUILD_GPIO_PIN(6, 1 , 0 ),
    GPIO_PIN_PTG2   =    _BUILD_GPIO_PIN(6, 2 , 0 ),
    GPIO_PIN_PTG3   =    _BUILD_GPIO_PIN(6, 3 , 0 ),
    GPIO_PIN_PTG4   =    _BUILD_GPIO_PIN(6, 4 , 0 ),
    GPIO_PIN_PTG5   =    _BUILD_GPIO_PIN(6, 5 , 0 ),
    GPIO_PIN_PTG6   =    _BUILD_GPIO_PIN(6, 6 , 0 ),
    GPIO_PIN_PTG7   =    _BUILD_GPIO_PIN(6, 7 , 0 ),
    GPIO_PIN_PTG8   =    _BUILD_GPIO_PIN(6, 8 , 0 ),
    GPIO_PIN_PTG9   =    _BUILD_GPIO_PIN(6, 9 , 0 ),
    GPIO_PIN_PTG10  =    _BUILD_GPIO_PIN(6, 10, 0 ),
    GPIO_PIN_PTG11  =    _BUILD_GPIO_PIN(6, 11, 0 ),
    GPIO_PIN_PTG12  =    _BUILD_GPIO_PIN(6, 12, 0 ),
    GPIO_PIN_PTG13  =    _BUILD_GPIO_PIN(6, 13, 0 ),
    GPIO_PIN_PTG14  =    _BUILD_GPIO_PIN(6, 14, 0 ),
    GPIO_PIN_PTG15  =    _BUILD_GPIO_PIN(6, 15, 0 ),
    //PTH                                           
    GPIO_PIN_PTH0   =    _BUILD_GPIO_PIN(7, 0 , 0 ),
    GPIO_PIN_PTH1   =    _BUILD_GPIO_PIN(7, 1 , 0 ),
    GPIO_PIN_PTH2   =    _BUILD_GPIO_PIN(7, 2 , 0 ),
    GPIO_PIN_PTH3   =    _BUILD_GPIO_PIN(7, 3 , 0 ),
    GPIO_PIN_PTH4   =    _BUILD_GPIO_PIN(7, 4 , 0 ),
    GPIO_PIN_PTH5   =    _BUILD_GPIO_PIN(7, 5 , 0 ),
    GPIO_PIN_PTH6   =    _BUILD_GPIO_PIN(7, 6 , 0 ),
    GPIO_PIN_PTH7   =    _BUILD_GPIO_PIN(7, 7 , 0 ),
    GPIO_PIN_PTH8   =    _BUILD_GPIO_PIN(7, 8 , 0 ),
    GPIO_PIN_PTH9   =    _BUILD_GPIO_PIN(7, 9 , 0 ),
    GPIO_PIN_PTH10  =    _BUILD_GPIO_PIN(7, 10, 0 ),
    GPIO_PIN_PTH11  =    _BUILD_GPIO_PIN(7, 11, 0 ),
    GPIO_PIN_PTH12  =    _BUILD_GPIO_PIN(7, 12, 0 ),
    GPIO_PIN_PTH13  =    _BUILD_GPIO_PIN(7, 13, 0 ),
    GPIO_PIN_PTH14  =    _BUILD_GPIO_PIN(7, 14, 0 ),
    GPIO_PIN_PTH15  =    _BUILD_GPIO_PIN(7, 15, 0 ),
#endif

  #if 0
    GPIO_PIN_EX0      =  _BUILD_GPIO_PIN(8, 0 , 0 ),
    GPIO_PIN_EX1      =  _BUILD_GPIO_PIN(8, 1 , 0 ),
    GPIO_PIN_EX2      =  _BUILD_GPIO_PIN(8, 2 , 0 ),
    GPIO_PIN_EX3      =  _BUILD_GPIO_PIN(8, 3 , 0 ),
    GPIO_PIN_EX4      =  _BUILD_GPIO_PIN(8, 4 , 0 ),
    GPIO_PIN_EX5      =  _BUILD_GPIO_PIN(8, 5 , 0 ),
    GPIO_PIN_EX6      =  _BUILD_GPIO_PIN(8, 6 , 0 ),
    GPIO_PIN_EX7      =  _BUILD_GPIO_PIN(8, 7 , 0 ),
  #endif
    GPIO_PIN_NONE    =    0x7FFFFFFF,  
}iomux_pin_name_t;
#endif
