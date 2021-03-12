/********************************Copyright ( c ) ********************************
**                  DAQU 
**                         
**                        
**
**Created By:     pengxuebin,20170320      
** Created Date:     
** Version:        
** Description:    ���ļ�����ϵͳtick��ϵͳ��ʱ��Ӳ��ʱ�ȹ���
  
****************************************************************************/
#include "devglobal.h"
#include "sdk/mhscpu_sdk.h"

#define SYSTICK_1MS_PRESCALE    ((SYSCTRL->HCLK_1MS_VAL)<<((SYSCTRL->FREQ_SEL>>4)&0x01))//(SYSCTRL->PCLK_1MS_VAL) //48000

//static s32 g_systick_fd = -1;
static u32 g_systick_cnt = 0;
static u32 g_systick_run_switch = 1;
/****************************************************************************
**Description:        tick�豸�жϷ������
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170320 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
extern void dev_user_timer_process(void);
extern void drv_mag_decode(void);

s32 drv_systick_get_run_switch(void)
{
    return g_systick_run_switch;
}
    
void drv_systick_set_run_switch(s32 flag)
{
    g_systick_run_switch = flag;
}

void SysTick_Handler(void)
{
    s32 i;
    
    g_systick_cnt++;
    i = g_systick_cnt%10;

    if(!g_systick_run_switch)
    {
        return ;
    }
    
  #if(BT_EXIST==1)  
    dev_bt_task_process();
  #endif
    switch(i)
    {
    case 0:
    case 1:
    case 2:
    case 3:
  #if(KEYPAD_EXIST==1)  
        dev_keypad_scan(i);
  #endif
        break;
    case 5:
//        dev_audio_task();
        break;
    case 6:
        dev_user_timer_process();
        break;
    case 7:
      #if(WIFI_EXIST == 1)
        dev_wifi_thread();
      #endif
        break;
#ifdef TRENDIT_CORE
    case 4:
        dev_wireless_thread();
        break;
    
    case 8:
        dev_led_loop();
        break;
#endif
    }
    
}
/****************************************************************************
**Description:        ��ʼ��tick�豸
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170320 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void drv_systick_int(void)
{
    SysTick_Config(SYSTICK_1MS_PRESCALE);      //1ms
}
void drv_systick_ctl(u8 flg)
{
    if(flg)
    {
       #if 1 
        SysTick->CTRL  |= (SysTick_CTRL_ENABLE_Msk); 
       #else
        SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
                           SysTick_CTRL_TICKINT_Msk |
                           SysTick_CTRL_ENABLE_Msk;  
       #endif
    }
    else
    {
       #if 1
        SysTick->CTRL  &= ~(SysTick_CTRL_ENABLE_Msk); 
       #else
        SysTick->CTRL = 0;
       #endif
    }
}
/****************************************************************************
**Description:        ��ȡ��us������ID
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170320 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
u32 drv_systick_get_usid(void)
{
    u32 i,j;
    u32 n;
    
    n = SysTick->VAL;
    i = g_systick_cnt;
    j = SysTick->VAL;
    /*if(j>SYSTICK_1MS_PRESCALE)
    {
        dev_debug_printf("systick err!\r\n");
    }*/
    if(j>n)
    {
        i = g_systick_cnt;
    }
    return ((i+1)*1000 -((j*1000)/SYSTICK_1MS_PRESCALE));
}
/****************************************************************************
**Description:        ��ѯ��ʱʱ���Ƿ�(��usΪ��λ)
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:δ��
                    1:ʱ�䵽
**Created by:        pengxuebin,20170320 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
u32 drv_systick_querry_us(u32 tus, u32 delayus)
{
    u32 pass_us;

    pass_us = drv_systick_get_usid() - tus;
    if(pass_us >= delayus)
    {
        return 1;           //ʱ�䵽
    }
    else
    {
        return 0;           
    }
}
/****************************************************************************
**Description:        ��ȡ��ms������IDֵ
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170320 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
u32 drv_systick_get_msid(void)
{
    return g_systick_cnt;
}
/****************************************************************************
**Description:        ��ѯ��ʱʱ���Ƿ�(��msΪ��λ)
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:δ��
                    1:ʱ�䵽
**Created by:        pengxuebin,20170320 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
u32 drv_systick_querry_ms(u32 tms, u32 delayms)
{     
    u32 pass_ms;
  
    pass_ms = drv_systick_get_msid()-tms;

    if(pass_ms >= delayms)
    {
        return 1;           //ʱ�䵽
    }
    else
    {
        return 0;           
    }
}
/****************************************************************************
**Description:       us��ʱ
**Input parameters:    
**Output parameters: 
**Returned value:
**Created by:        pengxuebin,20170320 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void drv_systick_delay_us(u32 nus)
{
    u32 i;

    i = drv_systick_get_usid();
    while(1)
    {
        if(drv_systick_querry_us(i, nus))
        {
            break;
        }
    }
}
/****************************************************************************
**Description:       ms��ʱ
**Input parameters:    
**Output parameters: 
**Returned value:
**Created by:        pengxuebin,20170320 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void drv_systick_delay_ms(u32 nms)
{
    u32 i;

    if(nms<100)
    {
        //100ms����ʱ����ȷ��us
        drv_systick_delay_us(nms*1000);
    }
    else
    {
        i = drv_systick_get_msid();
        while(1)
        {
            if(drv_systick_querry_ms(i, nms))
            {
                break;
            }

#ifdef TRENDIT_CORE
            if(drv_systick_querry_ms(i, 200))
            {
                drv_watchdog_feed();
            }
#endif
        }
    }
}

