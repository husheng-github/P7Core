
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
    if (TPGetPaperDetect() == 0) // ��ֽ
    {
            //DBG_STR("��ֽ\r\n");

            //dev_led_sta_set(LED_BLUE_NO, 1);
            
            if((printersts & PAPER_READY) == 0)
            {
                    printersts &= ~PAPER_SNS;
                    printersts |= PAPER_READY;
                    //DBG_STR("��ֽ\r\n");
            }
    }
    else  // ��ֽ
    {
            //DBG_STR("��ֽ\r\n");
            
            if((printersts & PAPER_READY) == PAPER_READY)
            {
                    printersts |= PAPER_SNS;
                    printersts &= ~PAPER_READY;
                    //DBG_STR("��ֽ\r\n");
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
����    �Ѷ�ȡ����ADֵ������������������������Ӧ�ڲ��ĵ���ֵ
����    ad:     ��ȡ����adֵ,��λ:100ŷķ
        uRes:   �ⲿ��������ֵ
        dRes:   �ⲿ��������ֵ
*/
uint32_t TranVtoR(uint32_t ad,uint32_t adMax,uint32_t uRes,uint32_t dRes)
{
    if (uRes)
    {   // ��������
            if (ad >= adMax)
                    return (ad*uRes);
            else
                    return ((ad*uRes)/(adMax-ad));
    }
    else
    {   // ��������
            if (ad == 0) ad = 1;
            return ((adMax-ad)*dRes/ad);
    }
}

/*
����    ��������о����������ֵת��Ϊ��Ӧ�¶�
����    res     ��������ֵ����λ:100ŷķ
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

/*
****************************************************************************************************
** ��    �� ��APP_Math_Chr2Hex()
** ��    �� ���Ѷ�ȡ����AD��ѹֵ������������������������Ӧ�ڲ��ĵ���ֵ
** ��ڲ��� ��    ad:     ��ȡ����ad��ѹֵ,��λ:100ŷķ
                        uRes:   �ⲿ��������ֵ
                        dRes:   �ⲿ��������ֵ
** ���ڲ��� ����
** ��    �� ��
** ��    ע :
****************************************************************************************************
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
        

        
        
        cTemperature = TranRtoDegree(tResult); // 10λADC
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

        //DBG_STR("��ֽ\r\n");
        if(print_nopaper_count>=PAPER_SNSDETECT_MAX)
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

