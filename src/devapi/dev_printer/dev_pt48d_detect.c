/**
 * @file dev_pt48d_detect.c
 * @brief 打印机检测代码
 * @note  检测是否有纸、温度
 * @since 2021.3.20  胡圣整理代码
 */

#include "devglobal.h"
#include "sdk/mhscpu_sdk.h"

#if(PRINTER_EXIST==1)
#include "dev_pt48d_detect.h"
#include "dev_pt48d_hw.h"

uint8_t printersts;

#if defined(NEW_HEAT_TIME)
    uint8_t PrintOn_Flag = FALSE;
#endif

extern int print_nopaper_count;

const uint16_t restbl[25] = {8430,6230,4660,3520,2690,2080,
                        1610,1240, 968, 757, 595, 471,
                         375, 300, 242, 196, 159, 131,
                         108,  89,  74,  62,  52,  44,37};

/**
 * @brief  检测是否有纸
 */
void TPPaperSNSDetect(uint8_t c)
{   
    if (TPGetPaperDetect() == 0)  //有纸
    {          
        if((printersts & PAPER_READY) == 0)
        {
            printersts &= ~PAPER_SNS;
            printersts |= PAPER_READY;
            //DBG_STR("进纸\r\n");
        }
    }
    else  //无纸
    {       
        if((printersts & PAPER_READY) == PAPER_READY)
        {
            printersts |= PAPER_SNS;
            printersts &= ~PAPER_READY;
            //DBG_STR("出纸\r\n");
        }
    }
}

/**
 * @brief  判断打印机是否准备好
 * @return  TRUE:准备好了    FALSE:没有准备好
 */
uint8_t TPPrinterReady(void)
{
    if(printersts & PAPER_READY)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

uint8_t TPPrinterMark(void)
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

/**
 * @brief  把热敏机芯的热敏电阻值转换为对应温度
 * @param [in]  res:热敏电阻值
 * @retval 转换的温度值
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

/**
 * @brief  电压值转换为电阻值
 * @param [in]  cVol:热敏电阻值
 * @retval 转换的温度值
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

/**
 * @brief  打印机温度检测
 * @retval 检测温度
 */
int16_t TPHTemperature(void)
{
    u16 adbuf[3] = {0};
    uint32_t temp = 0;
    uint32_t tResult = 0;
    int16_t cTemperature = 0;

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
        
        tResult = (temp*188*2)/0xFFF;      
        tResult = PrinterVolToR(tResult, 300, 200, 0);
        
        cTemperature = TranRtoDegree(tResult);           
        return cTemperature;
    }
    else
    {
        return DEVSTATUS_ERR_FAIL;
    }
    
}

/**
 * @brief  检测是否有纸
 * @retval 0:有纸  1:没纸
 */
uint8_t TPGetPaperDetect(void)
{        
    return pt_get_paper_status();
}

/**
 * @brief  定时检测是否有纸，设置状态 
 * @param [in]  c:未使用
 */
void TPPaperSNSDetect_interrupt(uint8_t c)
{ 
    if(print_nopaper_count >= PAPER_SNSDETECT_MAX)
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

