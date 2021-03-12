
#include "devglobal.h"
#include "sdk/mhscpu_sdk.h"

#if(PRINTER_EXIST==1)
#include "dev_pt48d_detect.h"
#include "dev_pt48d_hw.h"

volatile uint16_t Power_AD;
volatile uint16_t battery_value;

uint8_t printersts,papercnt,platencnt,bm_cnt;

#if defined(NEW_HEAT_TIME)
    uint8_t PrintOn_Flag = FALSE;
#endif

static int g_paper_snsdetect_count = 0;
extern int print_nopaper_count;

const uint16_t restbl[25] = {8430,6230,4660,3520,2690,2080,
                        1610,1240, 968, 757, 595, 471,
                         375, 300, 242, 196, 159, 131,
                         108,  89,  74,  62,  52,  44,37};


void TPPaperSNSDetect(uint8_t c)
{   
    if (TPGetPaperDetect() == 0) // 有纸
    {
            //DBG_STR("有纸\r\n");

            //dev_led_sta_set(LED_BLUE_NO, 1);
            
            if((printersts & PAPER_READY) == 0)
            {
                    printersts &= ~PAPER_SNS;
                    printersts |= PAPER_READY;
                    //DBG_STR("进纸\r\n");
            }
    }
    else  // 无纸
    {
            //DBG_STR("无纸\r\n");
            
            if((printersts & PAPER_READY) == PAPER_READY)
            {
                    printersts |= PAPER_SNS;
                    printersts &= ~PAPER_READY;
                    //DBG_STR("出纸\r\n");
            }
    }

}

uint8_t TPPrinterReady(void)
{
#if  0
    return TRUE;
#else
    if(printersts & PAPER_READY)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

#endif
}

extern uint8_t TPPrinterMark(void)
{
    if(printersts & BLACKMARKR_FLAG)
    {
         return FALSE;
    }
    else
    {
         return TRUE;
    }
}

/*
函数    把读取到的AD值，根据上拉或下拉电阻计算对应内部的电阻值
输入    ad:     读取到的ad值,单位:100欧姆
        uRes:   外部上拉电阻值
        dRes:   外部下拉电阻值
*/
uint32_t TranVtoR(uint32_t ad,uint32_t adMax,uint32_t uRes,uint32_t dRes)
{
    if (uRes)
    {   // 上拉电阻
            if (ad >= adMax)
                    return (ad*uRes);
            else
                    return ((ad*uRes)/(adMax-ad));
    }
    else
    {   // 下拉电阻
            if (ad == 0) ad = 1;
            return ((adMax-ad)*dRes/ad);
    }
}

/*
函数    把热敏机芯的热敏电阻值转换为对应温度
输入    res     热敏电阻值，单位:100欧姆
*/                         
int16_t TranRtoDegree(uint32_t res)
{
    uint16_t i;
    int16_t degree;

    // 热敏电阻对应温度表 -40 --> 75,间隔5度
    i = 0;
    while(i < 24)
    {
        if (res >= restbl[i+1]) break;
        i++;
    }
    if (i >= 24)
    {   // 超过80度
        degree = ((restbl[24]-res)*5)/(restbl[23]-restbl[24]) + 80;
    }
    else
    {
        degree = 5*(i+1) - ((res-restbl[i+1])*5)/(restbl[i]-restbl[i+1]) - 40;
    }

    return (degree);
}

/*
****************************************************************************************************
** 名    称 ：APP_Math_Chr2Hex()
** 功    能 ：把读取到的AD电压值，根据上拉或下拉电阻计算对应内部的电阻值
** 入口参数 ：    ad:     读取到的ad电压值,单位:100欧姆
                        uRes:   外部上拉电阻值
                        dRes:   外部下拉电阻值
** 出口参数 ：无
** 返    回 ：
** 备    注 :
****************************************************************************************************
*/
uint32_t PrinterVolToR(uint32_t cVol,uint32_t cRefVol,uint32_t uRes,uint32_t dRes)
{
    if (uRes)
    {  
        // 上拉电阻
        return ((cVol*uRes)/(cRefVol-cVol));
    }
    else
    {  
        // 下拉电阻
        return ((cRefVol-cVol)*dRes/cVol);
    }
}


int16_t TPHTemperature(void)
{
    uint8_t i;
    u16 adbuf[3] = {0};
    uint32_t temp = 0;
    uint32_t tResult = 0;
    int16_t cTemperature = 0;
    char cTemp[64];


    if(pt_get_exist())
    {
        dev_adc_open(DEV_ADC_PTR_TM);

        if(dev_adc_get_value(DEV_ADC_PTR_TM, adbuf, 3) < 0)
        {
        	dev_adc_close(DEV_ADC_PTR_TM);
            return DEVSTATUS_ERR_FAIL;
        }
        dev_adc_close(DEV_ADC_PTR_TM);
		
        temp = adbuf[0] + adbuf[1] + adbuf[2];

        temp = temp/3;
        //dev_debug_printf("---- adcaa = %d  ---- \r\n",temp);

        
        #if 1 

        //tResult = (temp*180)/0xFFF;
        tResult = (temp*188*2)/0xFFF;
        //dev_debug_printf("---- V = %d  ---- \r\n",tResult);


        //tResult = PrinterVolToR(tResult, 180, 300, 0);
        tResult = PrinterVolToR(tResult, 300, 200, 0);
        //dev_debug_printf("---- R = %d  ---- \r\n",tResult);
        

        
        
        cTemperature = TranRtoDegree(tResult); // 10位ADC
      #endif
        //cTemperature = 16;
		//dev_debug_printf("ct=%d\r\n", cTemperature);        
        return cTemperature;
    }
    else
    {
        return DEVSTATUS_ERR_FAIL;
    }
    
}

uint8_t TPGetPaperDetect(void)
{        
        return pt_get_paper_status();
}

extern void TPPaperSNSDetect_interrupt(uint8_t c)//488?????,486?????
{

        //DBG_STR("无纸\r\n");
        if(print_nopaper_count>=PAPER_SNSDETECT_MAX)
        {
            //一开始认为是没纸，清掉状态
            if((printersts & PAPER_READY) == PAPER_READY)
            {
                printersts |= PAPER_SNS;
                printersts &= ~PAPER_READY;
                //DBG_STR("出纸\r\n");
            }
        }
        else
        {
            if((printersts & PAPER_READY) == 0)
            {
                printersts &= ~PAPER_SNS;
                printersts |= PAPER_READY;
                //DBG_STR("进纸\r\n");
            }
        }
}

#endif

