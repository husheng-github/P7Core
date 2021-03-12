/********************************Copyright ( c ) ********************************
**                  DAQU 
**                         
**                        
**
**Created By:         
** Created Date:     
** Version:        
** Description:    该文件包含电源管理的驱动相关接口
  
****************************************************************************/
#include "devglobal.h"
#include "sdk/mhscpu_sdk.h"
#include "drv_power.h"
#if(MACHINETYPE==MACHINE_M2)
#define POWERONOFF_PIN     GPIO_PIN_PTC1 
#endif
#if(MACHINETYPE==MACHINE_S1)
#define SP_WAKEUP_PIN       GPIO_PIN_PTB3//和SPI的片选复用

#define SP_STATUS_PIN_T1  GPIO_PIN_NONE
#define SP_STATUS_PIN_M6  GPIO_PIN_PTD4
#define SP_STATUS_PIN_T3  GPIO_PIN_NONE     //T3没有休眠转态指示
iomux_pin_name_t SP_STATUS_PIN = GPIO_PIN_NONE;
#define PRT_POWER_PIN       GPIO_PIN_NONE//GPIO_PIN_EX1
  
#endif
/****************************************************************************
**Description:       电源初始化
**Input parameters:    
**Output parameters: 
**Returned value:
**Created by:        pengxuebin,20170606
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_power_init(void)
{
#if 0
#if(MACHINETYPE == MACHINE_S1)
    //唤醒管脚初始化为上拉输入, 下降沿触发,不中断
  #if 0  
    dev_gpio_config_mux(SP_WAKEUP_PIN, MUX_CONFIG_GPIO);
    dev_gpio_set_pad(SP_WAKEUP_PIN, PAD_CTL_PULL_UP|PAD_CTL_ODE_EN);
    dev_gpio_direction_input(SP_WAKEUP_PIN);
    dev_gpio_irq_request(SP_WAKEUP_PIN, NULL, (IRQ_ISR_FALLING), NULL); //IRQ_ISR_RISING|
  #endif  
    
    s32 ret;
    u8 machineid;

    machineid = dev_misc_getmachinetypeid();
    if(machineid == MACHINE_TYPE_ID_T1)
    {
        SP_STATUS_PIN = SP_STATUS_PIN_T3;   //SP_STATUS_PIN_T1;
    }
    else if((machineid == MACHINE_TYPE_ID_M6)    //M6:2
          ||(machineid == MACHINE_TYPE_ID_T5)  //T5
          ||(machineid == MACHINE_TYPE_ID_T6)) //T6
    {
        SP_STATUS_PIN = SP_STATUS_PIN_M6;
    } 
    else if(machineid == MACHINE_TYPE_ID_T3)
    {
        SP_STATUS_PIN = SP_STATUS_PIN_T3;
    }
    else  //sxl?2019  这个管脚要控制，看如何处理
    {
        return;
    }
    
    //SP的状态脚设置为输出低,
    dev_gpio_config_mux(SP_STATUS_PIN, MUX_CONFIG_GPIO);
    dev_gpio_set_pad(SP_STATUS_PIN, PAD_CTL_PULL_NONE);
    dev_gpio_direction_output(SP_STATUS_PIN, 0);

    //PRN PWR 初始化为0,打开打印机逻辑电源
    dev_gpio_config_mux(PRT_POWER_PIN, MUX_CONFIG_GPIO);   
    dev_gpio_set_pad(PRT_POWER_PIN, PAD_CTL_PULL_UP);
    dev_gpio_direction_output(PRT_POWER_PIN, 0);
#endif

#if(MACHINETYPE==MACHINE_M2)
  #ifdef TRENDIT_BOOT
    //初始化关闭电源,仅在BOOT初始化
    dev_gpio_config_mux(POWERONOFF_PIN, MUX_CONFIG_GPIO);
    dev_gpio_set_pad(POWERONOFF_PIN, PAD_CTL_PULL_NONE);
    dev_gpio_direction_output(POWERONOFF_PIN, 0);
  #endif  
#endif
#endif
    return 0;
}
/****************************************************************************
**Description:       电源开关
**Input parameters:    
                    flg:  1.POWERON
                          2.POWEROFF
**Output parameters: 
**Returned value:
**Created by:        pengxuebin,20170606
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void drv_power_switch_ctl(u8 flg)
{
#if(MACHINETYPE==MACHINE_M2)
    dev_gpio_set_value(POWERONOFF_PIN, flg);
#else
    drv_power_set_latch(flg);
#endif
}
/****************************************************************************
**Description:       获取wakeup管脚状态
**Input parameters:    
**Output parameters: 
                    
**Returned value:
**Created by:        pengxuebin,20170629
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_power_get_wakuppin(void)
{
    s32 ret = -1;
#if(MACHINETYPE==MACHINE_S1)
    ret = dev_gpio_get_value(SP_WAKEUP_PIN);
#endif
    return ret;
}
/****************************************************************************
**Description:       SLEEP
**Input parameters:    
                   mod:     sleep模式
                            0:      sleep模式
                            1:      deep sleep模式
**Output parameters: 
**Returned value:
**Created by:        pengxuebin,20170629
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_tample_close(void)
{
    u32 SEN_ENtmp[19];
    s32 i;

//    SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_BPU, ENABLE);
//    SYSCTRL_APBPeriphResetCmd(SYSCTRL_APBPeriph_BPU, ENABLE);    

    SENSOR_ANACmd(SENSOR_ANA_VOL_HIGH|
                    SENSOR_ANA_VOL_LOW|
                    SENSOR_ANA_TEMPER_HIGH|
                    SENSOR_ANA_TEMPER_LOW|
                    SENSOR_ANA_MESH |
                    SENSOR_ANA_XTAL32K, DISABLE);
    SENSOR_EXTCmd(DISABLE);
    
    SENSOR_ClearITPendingBit();
    SENSOR_EXTCmd(DISABLE);
    while(SET == SENSOR_EXTIsRuning());
  #if 0  
    for(i=0; i<19; i++)
    {
        SEN_ENtmp[i] = SENSOR->SEN_EN[i];
dev_debug_printf("SEN_EN[%2d] = %08X\r\n", i, SEN_ENtmp[i]);
    }
  #endif  
}
/****************************************************************************
**Description:       SLEEP
**Input parameters:    
                   mod:  bit0  sleep模式
                            0:      sleep模式
                            1:      deep sleep模式
**Output parameters: 
**Returned value: 
                    1:GPIO口唤醒
                    2:串口通讯唤醒
                    3:RTC唤醒
                    4:Sensor触发唤醒
                    5:磁卡唤醒
                    6:SCI中断唤醒
                    7:定时器中断唤醒
                    -1:错误中断源
                    
**Created by:        pengxuebin,20170629
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
extern u32 pcitamper_manage_gettamperstatus(void);
extern void pci_core_close(void);
s32 drv_power_sleep(u32 mod)
{
    u32 wakeflg1=0;
    u32 wakeflg2=0;
    s32 ret;
//    u8 setspstatuspin = 0;
    
#if 0
#if 0
   #ifdef DEBUG_POWER_EN
    drv_tample_close();
   #endif   
#endif
POWER_DEBUG("EnterSleep\r\n");
#ifdef TRENDIT_BOOT
    drv_tample_close();
#endif
#ifdef TRENDIT_CORE
    //if(mod&0x02)
//    drv_tample_close();
    if(pcitamper_manage_gettamperstatus())
    {   //关触发
        //pci_core_close();  //sxl不关触发
    }
#endif  
#if(PRINTER_EXIST==1)
    pt_sleep();
#endif
    dev_com_tcdrain(PORT_UART0_NO);
    dev_com_close(PORT_CDCD_NO);
    dev_user_delay_ms(10);
    dev_pcd_powerdown();
    dev_gpio_set_value(PRT_POWER_PIN, 1);
    dev_pcd_sleep_ctl(1);
    pt_ctl_slp_sleep(1);
    dev_systick_ctl(0);       
    
  #if 0  
    u8 machineid;

    machineid = dev_misc_getmachinetypeid();
    if(machineid == MACHINE_TYPE_ID_T1)
    {
        SP_STATUS_PIN = SP_STATUS_PIN_T1;
        setspstatuspin = 1;
    }
    else if((machineid == MACHINE_TYPE_ID_M6)
          ||(machineid == MACHINE_TYPE_ID_T5)  //增加T5机型
          ||(machineid == MACHINE_TYPE_ID_T6))
    {
        SP_STATUS_PIN = SP_STATUS_PIN_M6;
        setspstatuspin = 1;
    } 
  #endif
    
    if((mod&0x01)==0)
    {
        SYSCTRL_EnterSleep(SleepMode_CpuOff);//SleepMode_CpuSleep//SleepMode_CpuOff);
    }
    else //if(mod==1)
    {
        //GPIO_WakeEvenDeInit();
        //GPIO->WAKE_EVEN_TYPE_EN = 1; 
        //------>配置SPI CS下降沿唤醒
        //dev_spi_close(SPI_DEV_AP); //sxl20190925
        dev_gpio_config_mux(SP_WAKEUP_PIN, MUX_CONFIG_GPIO);
        dev_gpio_set_pad(SP_WAKEUP_PIN, PAD_CTL_PULL_UP|PAD_CTL_ODE_EN);
        dev_gpio_direction_input(SP_WAKEUP_PIN);
        dev_gpio_irq_request(SP_WAKEUP_PIN, NULL, (IRQ_ISR_FALLING), NULL);
        
        
        GPIO->WAKE_TYPE_EN = 0x00005001;   //WAKE_EVEN_TYPE_EN//Sensor、RTC、GPIO中断唤醒使能
        dev_gpio_wakeup_ctl(SP_WAKEUP_PIN, 1);
        GPIO_WakeModeConfig(GPIO_WakeMode_Now);
        //if(setspstatuspin)
        {
            dev_gpio_set_value(SP_STATUS_PIN, 1);
        }
        SYSCTRL_EnterSleep(SleepMode_DeepSleep);//SleepMode_CpuMemOff); 
        //if(setspstatuspin)
        {
            dev_gpio_set_value(SP_STATUS_PIN, 0);
        }

       //恢复SPI CS口
        //dev_spi_open(SPI_DEV_AP); //sxl20190925
    }
    wakeflg1 = SYSCTRL_GetWakeupFlag1Status(); 
    wakeflg2 = SYSCTRL_GetWakeupFlag2Status();   
    dev_systick_ctl(1); 
    pt_ctl_slp_sleep(0);
    dev_com_open(PORT_CDCD_NO, MCU_UART_BAUD, 8, 'n', 0, MCU_FLOWCTL_FLG);
    dev_gpio_set_value(PRT_POWER_PIN, 0);
    dev_pcd_sleep_ctl(0);
//    POWER_DEBUG("%08X, %08X\r\n", wakeflg1, wakeflg2);    
    //判断中断唤醒源
    if(wakeflg1&(0x0f<<20))
    {
        ret = 1;        //GPIO口唤醒
    }
    else if(wakeflg1&(0x03<<0))
    {
        ret = 2;        //串口唤醒
    }
    else if(wakeflg2&(1<<2))
    {
        ret = 3;        //RTC唤醒   
    }
    else if(wakeflg2&(1<<0))
    {
        ret = 4;        //RTC唤醒
    }
    else if(wakeflg1&(1<<24))
    {
        ret = 5;        //磁卡中断唤醒
    }
    else if(wakeflg1&(1<<14))
    {
        ret = 6;        //SCI中断(IC卡)
    }
    else if(wakeflg2&(0x3f<<8))
    {
        ret = 7;        //定时器中断
    }
    else
    {
        ret = -1;       //其他中断源
        POWER_DEBUG("%08X, %08X\r\n", wakeflg1, wakeflg2);    
    }
POWER_DEBUG("ret=%d\r\n", ret); 
#endif
    return ret;
}

