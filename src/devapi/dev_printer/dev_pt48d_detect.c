/**
 * @file dev_pt48d_detect.c
 * @brief ��ӡ��������
 * @note  ����Ƿ���ֽ���¶�
 * @since 2021.3.20  ��ʥ�������
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
 * @brief  ����Ƿ���ֽ
 */
void TPPaperSNSDetect(uint8_t c)
{   
    if (TPGetPaperDetect() == 0)  //��ֽ
    {          
        if((printersts & PAPER_READY) == 0)
        {
            printersts &= ~PAPER_SNS;
            printersts |= PAPER_READY;
            //DBG_STR("��ֽ\r\n");
        }
    }
    else  //��ֽ
    {       
        if((printersts & PAPER_READY) == PAPER_READY)
        {
            printersts |= PAPER_SNS;
            printersts &= ~PAPER_READY;
            //DBG_STR("��ֽ\r\n");
        }
    }
}

/**
 * @brief  �жϴ�ӡ���Ƿ�׼����
 * @return  TRUE:׼������    FALSE:û��׼����
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
 * @brief  ��������о����������ֵת��Ϊ��Ӧ�¶�
 * @param [in]  res:��������ֵ
 * @retval ת�����¶�ֵ
 */
int16_t TranRtoDegree(uint32_t res)
{
    uint16_t i;
    int16_t degree;

    // ���������Ӧ�¶ȱ� -40 --> 75,���5��
    i = 0;
    while(i < 24)
    {
        if (res >= restbl[i+1]) break;
        i++;
    }
    if (i >= 24)
    {   // ����80��
        degree = ((restbl[24]-res)*5)/(restbl[23]-restbl[24]) + 80;
    }
    else
    {
        degree = 5*(i+1) - ((res-restbl[i+1])*5)/(restbl[i]-restbl[i+1]) - 40;
    }

    return (degree);
}

/**
 * @brief  ��ѹֵת��Ϊ����ֵ
 * @param [in]  cVol:��������ֵ
 * @retval ת�����¶�ֵ
 */
uint32_t PrinterVolToR(uint32_t cVol,uint32_t cRefVol,uint32_t uRes,uint32_t dRes)
{
    if (uRes)
    {  
        // ��������
        return ((cVol*uRes)/(cRefVol-cVol));
    }
    else
    {  
        // ��������
        return ((cRefVol-cVol)*dRes/cVol);
    }
}

/**
 * @brief  ��ӡ���¶ȼ��
 * @retval ����¶�
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
 * @brief  ����Ƿ���ֽ
 * @retval 0:��ֽ  1:ûֽ
 */
uint8_t TPGetPaperDetect(void)
{        
    return pt_get_paper_status();
}

/**
 * @brief  ��ʱ����Ƿ���ֽ������״̬ 
 * @param [in]  c:δʹ��
 */
void TPPaperSNSDetect_interrupt(uint8_t c)
{ 
    if(print_nopaper_count >= PAPER_SNSDETECT_MAX)
    {
        //һ��ʼ��Ϊ��ûֽ�����״̬
        if((printersts & PAPER_READY) == PAPER_READY)
        {
            printersts |= PAPER_SNS;
            printersts &= ~PAPER_READY;
            //DBG_STR("��ֽ\r\n");
        }
    }
    else
    {
        if((printersts & PAPER_READY) == 0)
        {
            printersts &= ~PAPER_SNS;
            printersts |= PAPER_READY;
            //DBG_STR("��ֽ\r\n");
        }
    }
}

#endif

