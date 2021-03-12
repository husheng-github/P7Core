
#ifndef     _PT48D_DEV_H_
#define     _PT48D_DEV_H_

//#define PT48D_DEV_DEBUG_

#ifdef PT48D_DEV_DEBUG_
#define DBG_STR(str)            dev_debug_printf("%s", str)
extern void DBG_DAT(u8 *data,u16 len);
#else
#define DBG_STR(str)  
#define DBG_DAT(data,len) 
#endif

#define GPIO_PRT_MOT_PWR                        GPIO_PIN_NONE
#define GPIO_PRT_MOT_PHASE_1A                   GPIO_PIN_PTD2
#define GPIO_PRT_MOT_PHASE_2A                   GPIO_PIN_PTD3

#define GPIO_PRT_MOT_PHASE_1B                   GPIO_PIN_PTE0
#define GPIO_PRT_MOT_PHASE_2B                   GPIO_PIN_PTD11

#define GPIO_PRT_PAPER_DET                      GPIO_PIN_PTD6
#define GPIO_PRT_TM_DET                         GPIO_PIN_PTC4
#define GPIO_PRT_LAT                            GPIO_PIN_PTD1 

#define GPIO_PRT_STB1                           GPIO_PIN_PTB1
#define GPIO_PRT_POWER                          GPIO_PIN_PTD7
#define GPIO_PRT_SLP                            GPIO_PIN_PTA10

//T3与T1打印不通的管脚定义
#define GPIO_PRT_MOT_PWR_T3                     GPIO_PIN_PTB0 //pengxuebin,20190818 T3使用
#define GPIO_PRT_MOT_PHASE_2B_T3                GPIO_PIN_PTB2  //pengxuebin,20190818 T3使用      
#define GPIO_PRT_POWER_T3                       GPIO_PIN_NONE //pengxuebin,20190818 T3没有该管脚 
#define GPIO_PRT_SLP_T3                         GPIO_PIN_NONE  //pengxuebin,20190818 T3使用  

#define PRT_SPI_MOSI                            GPIO_PIN_PTA8

#define ARRAY_SIZE(x)    (sizeof(x) / sizeof((x)[0]))

#define TIMER_TP         0x03  //马达步进使用时钟
#define TIMER_TP_DET     0x02  //打印纸及温度检测时钟//定时器冲突，改为定时器2
#define TIMER1_MS_TO_CNT(ms)    ((uint32_t)(ms*1000)) // to us

u8 pt_get_exist(void);
void pt_ctl_slp_sleep(u8 flg);

extern void pt_timer_init(void);
extern void pt_timer_start(void);
extern void pt_timer_stop(void);
extern void pt_timer_set_periodUs(uint32_t uiPeriodUs);
extern void pt_gpio_init(void);
extern void TPDataShiftOut(uint8_t *p, uint16_t len);
extern void pt_detect_timer_start(void);
extern void pt_detect_timer_set_period(uint32_t uiPeriodMs);
extern uint8_t pt_get_paper_status(void);
extern void LATCH_HIGH(void) ;
extern void LATCH_LOW(void);
extern void MOTOR_PWR_ON(void) ;
extern void MOTOR_PWR_OFF(void);
extern void MOTOR_PHASE_1A_HIGH(void);
extern void MOTOR_PHASE_1A_LOW(void);
extern void MOTOR_PHASE_1B_HIGH(void);
extern void MOTOR_PHASE_1B_LOW(void) ;
extern void MOTOR_PHASE_2A_HIGH(void);
extern void MOTOR_PHASE_2A_LOW(void);
extern void MOTOR_PHASE_2B_HIGH(void);
extern void MOTOR_PHASE_2B_LOW(void) ;
extern void STROBE_0_ON(void) ;
extern void STROBE_0_OFF(void);
extern void PRN_POWER_CHARGE(void) ;
extern void PRN_POWER_DISCHARGE(void) ;
extern void STROBE_1_ON(void) ;
extern void STROBE_1_OFF(void);
extern void pt_spi_init(void);
extern void pt_sleep(void);
extern void pt_resume(void);
extern uint32_t TPHTemperatureADTest(void);
extern u32 pt_get_printline_len(void);

#endif


