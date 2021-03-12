
#ifndef     _ON_PT48D_DETECT_H_
#define     _ON_PT48D_DETECT_H_

#include "dev_pt48d_config.h"

#define PAPER_SNS          (1ul<<0)
#define PAPER_READY        (1ul<<1)
#define BLACKMARKR_FLAG    (1ul<<3)
/*
#define AD_BLACKMARK_HIGH  (0X0300UL)
#define AD_BLACKMARK_LOW   (0X0050UL)
#define PAPER_AD_LTHRESHOLD  (AD_BLACKMARK_HIGH)
*/

#if defined(PT723)
#define AD_BLACKMARK_HIGH  (0X0200UL)
#define AD_BLACKMARK_LOW   (0X00A0UL)
#define PAPER_AD_LTHRESHOLD  (AD_BLACKMARK_LOW)
#elif defined(PT486)
#define AD_BLACKMARK_HIGH  (0X0200UL)
#define AD_BLACKMARK_LOW   (0X0050UL)
#define PAPER_AD_LTHRESHOLD  (AD_BLACKMARK_LOW)
#elif defined(PT48D) || defined(PT48D_NEW_DENSITY)
#define AD_BLACKMARK_HIGH  (0X0200UL)
#define AD_BLACKMARK_LOW   (0X0050UL)
#define PAPER_AD_LTHRESHOLD  (0X00200UL)//无纸时一般是小于0xA0 1.5V
#else
#define AD_BLACKMARK_HIGH  (0X0300UL)
#define AD_BLACKMARK_LOW   (0X0050UL)
#define PAPER_AD_LTHRESHOLD  (AD_BLACKMARK_HIGH)
#endif

#define BMSNS()     //((((LPC_ADC->DR[0]>>6) & 0x3ff) < ((uint16_t)AD_BLACKMARK_HIGH))&&(((LPC_ADC->DR[0]>>6) & 0x3ff) > ((uint16_t)AD_BLACKMARK_LOW))?1:0)
#define PAPERSNS()  //((((LPC_ADC->DR[0]>>6) & 0x3ff)<PAPER_AD_LTHRESHOLD)?1:0)

#define PAPER_SNSDETECT_MAX 80




/////==========================================================

extern volatile uint16_t Power_AD;
#if defined(NEW_HEAT_TIME)
    extern uint8_t PrintOn_Flag;
#endif



////=======================================================
extern uint8_t TPPrinterReady(void);
extern int16_t TPHTemperature(void);
extern uint8_t TPPrinterMark(void);

extern uint8_t TPGetPaperDetect(void);
extern void TPPaperSNSDetect(uint8_t c);


void  adc_test(void);
#endif


