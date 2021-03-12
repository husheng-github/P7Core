/********************************Copyright ( c ) ********************************
**                  DAQU 
**                         
**                        
**
**Created By:         
** Created Date:     
** Version:        
** Description:    该文件包含IC卡的驱动接口
  
****************************************************************************/
#include "devglobal.h"
#include "sdk/mhscpu_sdk.h"
#include "drv_icc.h"
#define ICC_DEV_NSLOT       0    //ic卡对应物理端口
//#define ICC_AUX0_PIN        GPIO_PIN_PTA4 //sci0_aux0
//#define ICC_AUX1_PIN        GPIO_PIN_PTA5 //sci0_aux1
#define ICC_CLK_PIN         GPIO_PIN_PTA8 //sci0_CLK
#define ICC_RSTN_PIN        GPIO_PIN_PTA9 //sci0_RSTN
#define ICC_IO_PIN          GPIO_PIN_PTA10 //sci0_IO
#define ICC_DET_PIN         GPIO_PIN_PTA6 //sci0_DET
        
static u8 g_icc_active=0;
/****************************************************************************
**Description:       打开ICC设备
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170425
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void drv_icc_init(void)
{
    //Choose active level(Low level active).
    SYSCTRL->PHER_CTRL |= BIT(20);
//    SYSCTRL->PHER_CTRL &= (~BIT(20));
}
/****************************************************************************
**Description:       打开ICC设备
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170425
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_icc_open(void)
{
    s32 i;
    
    //Enable clock SCI0, SCI1, SCI2, UART0, and GPIO.
    SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_SCI0, ENABLE);
    SYSCTRL_APBPeriphResetCmd(SYSCTRL_APBPeriph_SCI0, ENABLE);

    //Set Bit9 use internal clock.
    SYSCTRL->FREQ_SEL |= BIT(9);
    
    //card detect
    SYSCTRL->PHER_CTRL |= BIT(16);
    //Choose active level(Low level active).
    SYSCTRL->PHER_CTRL |= BIT(20);

   #if 0
    //ICC_AUX0_PIN       
    dev_gpio_config_mux(ICC_AUX0_PIN, MUX_CONFIG_ALT0);
    //ICC_AUX1_PIN      
    dev_gpio_config_mux(ICC_AUX1_PIN, MUX_CONFIG_ALT0);
  #endif  
    //ICC_CLK_PIN     
    dev_gpio_config_mux(ICC_CLK_PIN, MUX_CONFIG_ALT0);
    //ICC_RSTN_PIN 
    dev_gpio_config_mux(ICC_RSTN_PIN, MUX_CONFIG_ALT0);
    //ICC_IO_PIN 
    dev_gpio_config_mux(ICC_IO_PIN, MUX_CONFIG_ALT0);
    //ICC_DET_PIN 
    dev_gpio_config_mux(ICC_DET_PIN, MUX_CONFIG_ALT0);
    
    //GPIO_PinRemapConfig(GPIOA, GPIO_Pin_4  | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_14, GPIO_Remap_0);
    SYSCTRL->SCI_GLF = SYSCTRL->SCI_GLF & ~BIT(29) | BIT(28);  //5V
    //SYSCTRL->SCI_GLF = SYSCTRL->SCI_GLF & ~(0x03 << 28) | (0x02 << 28);   //3V
    //虑毛刺使能
    //SYSCTRL->SCI_GLF |= BIT(31);
    SYSCTRL->SCI_GLF &= ~0xFFFFF;
    SYSCTRL->SCI_GLF |= SYSCTRL->PCLK_1MS_VAL>>2;
    //SYSCTRL->CARD_RSVD &= ~BIT(1);
    //SYSCTRL->CARD_RSVD |= BIT(1); //High TH(400mA protect)
    //PA6-CARD_DECTECT PA8-CLK PA9-RSTN PA10-IO PA14-VCC_EN
    //GPIO_GROUP[0].PUE |= BIT(10) | BIT(9);
    //GPIO_GROUP[0].PUE &= ~BIT(10);
    SCI_ConfigEMV(0x01, 4000000);
    
    iso7816_device_init();
    g_icc_active = 0;
    
    NVIC_ClearPendingIRQ(SCI0_IRQn);
    NVIC_EnableIRQ(SCI0_IRQn);
    i = iso7816_get_version();//iso7816_getlibversion();
    ICC_DEBUG("icc_ver=%08X\r\n", i);    
    return 0;
}
/****************************************************************************
**Description:       关闭ICC设备
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
**Created by:        pengxuebin,20170425
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_icc_close(void)
{
    iso7816_close(ICC_DEV_NSLOT);
//    SYSCTRL->PHER_CTRL &= (~BIT(20));
    SYSCTRL->PHER_CTRL |= (BIT(20));
    g_icc_active = 0;
    return 0;
}
/****************************************************************************
**Description:       下电
**Input parameters:    
**Output parameters: 
**Returned value:
                    
**Created by:        pengxuebin,20170425
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_icc_poweroff(void)
{
    iso7816_close(ICC_DEV_NSLOT);
//    SYSCTRL->PHER_CTRL &= (~BIT(20));
    g_icc_active = 0;
    return 0;
}
/****************************************************************************
**Description:       获取IC卡状态
**Input parameters:    
**Output parameters: 
**Returned value:
                    0: 无卡
                    1: 有卡
**Created by:        pengxuebin,20170425
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_icc_getstatus(void)
{
    if(0 != iso7816_detect(ICC_DEV_NSLOT))
    {
        if(g_icc_active==1)
        {
            g_icc_active = 0;
            drv_icc_poweroff();
        }
        return 0;
    }
    else
    {
        return 1;
    }
}
/****************************************************************************
**Description:       复位
**Input parameters:    
**Output parameters: 
**Returned value:
                    >0:复位成功,返回ATR长度
                    <0:失败
**Created by:        pengxuebin,20170425
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_icc_reset(u8 *lpAtr)
{
    s32 ret;
    u8 atr[65];
    
    if(0 == drv_icc_getstatus())
    {
        ICC_DEBUG("NO CARD!\r\n");
        //没有卡片
        return DEVSTATUS_ERR_FAIL;
    } 
    //上电
    ret = iso7816_init(ICC_DEV_NSLOT, VCC_3000mV | SPD_1X, atr); 
    if(ret == 0)
    {
        g_icc_active = 1;
        //复位成功
        memcpy(lpAtr, &atr[1], atr[0]); 
        return atr[0];
    }
    else
    {
        //复位成功
        ICC_DEBUG("reset fail!\r\n");
        return DEVSTATUS_ERR_FAIL;
    }
}
/****************************************************************************
**Description:       下电
**Input parameters:    
**Output parameters: 
**Returned value:
                    >0:复位成功,返回ATR长度
                    <0:失败
**Created by:        pengxuebin,20170425
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_icc_exchange_apdu(u8 *wbuf, u32 wlen, u8 *rbuf, u32 *rlen, u32 rxmax)
{
    ST_APDU_RSP rsp;
    ST_APDU_REQ apdu_req;
    s32 ret;

    *rlen = 0;
    //判断卡片是否存在
    if(0 == drv_icc_getstatus())
    {
        ICC_DEBUG("NO CARD!\r\n");
        //没有卡片
        return DEVSTATUS_ERR_FAIL;
    }
    if(g_icc_active==0)
    {
        ICC_DEBUG("NOT RESET!\r\n");
        //卡片没有复位
        return DEVSTATUS_ERR_FAIL;
    }
    if((wlen > (255+6)) || (wlen < 4))
    {
        ICC_DEBUG("wlen=%d\r\n", wlen);
        //卡片没有复位
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    if(rxmax < 2)
    {
        //期望的长度错
        ICC_DEBUG("rxmax=%d\r\n", rxmax);
        return DEVSTATUS_ERR_PARAM_ERR; 
    } 
    memcpy(apdu_req.cmd, wbuf, 4);
    if(4 == wlen)
    {
        //case 1
        apdu_req.lc = 0;
        apdu_req.le = 0;
    }
    else if(5 == wlen)
    {
        //case 2
        apdu_req.lc = 0;
        apdu_req.le = wbuf[4];
        if(apdu_req.le == 0)
        {
            apdu_req.le = 256;
        }
    }
    else if(wlen > 5)
    {
        apdu_req.lc = wbuf[4];
        memcpy(apdu_req.data_in, &wbuf[5], apdu_req.lc);
        if(wlen == (apdu_req.lc+5))
        {
            //case3
            apdu_req.le = 0;
        }
        else
        {
            //case4
            apdu_req.le = wbuf[apdu_req.lc+5];
            if(apdu_req.le == 0)
            {
                apdu_req.le = 256;
            }
        }        
    }
    ret = iso7816_exchange(ICC_DEV_NSLOT, AUTO_GET_RSP, &apdu_req, &rsp);
    if(ret != 0)
    {
        //出错
        ICC_DEBUG("exchange err!(%d)\r\n", ret);
        ret = DEVSTATUS_ERR_PARAM_ERR;
    }
    else
    {
        ret = DEVSTATUS_SUCCESS;
        if((rsp.len_out+2)>rxmax)
        {
            //出错
            ICC_DEBUG("len_out over!(len_out=%d, rxmax=%d)\r\n", rsp.len_out, rxmax);
            ret = DEVSTATUS_ERR_PARAM_ERR;
        }
        else
        {
            memcpy(rbuf, rsp.data_out, rsp.len_out);
            rbuf[rsp.len_out++] = rsp.swa;
            rbuf[rsp.len_out++] = rsp.swb;
            *rlen = rsp.len_out;
            ret = DEVSTATUS_SUCCESS;
        }
    }
    return ret;
}

