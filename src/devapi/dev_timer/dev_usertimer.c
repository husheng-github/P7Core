/********************************Copyright ( c ) ********************************
**                  DAQU 
**                         
**                        
**
**Created By:     pengxuebin,20170322      
** Created Date:     
** Version:        
** Description:    �û���ʱ���ӿں���
  
****************************************************************************/
#include "devglobal.h"
#include "drv_systick.h"

#define USER_TIMER_MAX        20
static volatile u32 *g_timer_p[USER_TIMER_MAX] = {0};
#define TIMER_BASE        10   //10ms��ʱ

static u8 g_timer_initflg = 0;
/****************************************************************************
**Description:        �û���ʱ������
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170320 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void dev_user_timer_init(void)
{
    
    s32 i;

    if(g_timer_initflg == 0)
    {
        for(i=0; i<USER_TIMER_MAX; i++)
        {
            g_timer_p[i] = NULL ;
        }
        //��tick��ϵͳ��ʱ��
        drv_systick_int();
        g_timer_initflg = 1;
        
    }
}
/****************************************************************************
**Description:        �û���ʱ������
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170320 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
// 10ms ��ʱ����
void dev_user_timer_process(void)
{
    u8 i;
    u32 cnt;   
    
    if(g_timer_initflg == 0)
    {
        return;
    }
    
    for(i=0; i<USER_TIMER_MAX; i++)
    {
        if((u32)(g_timer_p[i]) != NULL)
        {
            cnt = (*g_timer_p[i])-1;
            *g_timer_p[i] = cnt;

            if(0 == *g_timer_p[i])
            {
                g_timer_p[i] = NULL;
            }
        }
    }
}
/****************************************************************************
**Description:        ���û���ʱ��
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170320 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_user_timer_open(u32 *timer)
{
    s32 i,j;
    u32 *t;
    
    j = USER_TIMER_MAX;
    if(timer==NULL)
    {             
        return -1;
    }
    if(*timer == 0)
    {
        return 0;
    }

    for(i=0; i<USER_TIMER_MAX; i++)
    {
        if(NULL == g_timer_p[i])
        {
            j = i;
        }
        else if(timer == g_timer_p[i])
        {
            t = timer;
            *t = (*t + TIMER_BASE - 1)/TIMER_BASE;
            #ifdef FSL_RTOS_UCOSIII
            CPU_CRITICAL_EXIT();
            #endif
            return 0;
        }
    }
    if(j >= USER_TIMER_MAX)
    {
        //uart_printf("TimerNUM Over!\r\n");
        #ifdef FSL_RTOS_UCOSIII
        CPU_CRITICAL_EXIT();
        #endif
        return -1;
    }
    t = timer;
    *t = (*t + TIMER_BASE - 1)/TIMER_BASE;
    g_timer_p[j] = timer;
    #ifdef FSL_RTOS_UCOSIII
    CPU_CRITICAL_EXIT();
    #endif
    return 0;
}
/****************************************************************************
**Description:        �ر��û���ʱ��
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170320 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_user_timer_close(u32 *timer)
{     
    s32 i;
    
    if(timer == NULL)
    {
        return 0;
    }

    #ifdef FSL_RTOS_UCOSIII
    CPU_SR_ALLOC();
    CPU_CRITICAL_ENTER();
    #endif
    
    for(i=0; i<USER_TIMER_MAX; i++)
    {
        if(g_timer_p[i] == timer)
        {
             g_timer_p[i] = NULL;
        }
    }
    
    #ifdef FSL_RTOS_UCOSIII
    CPU_CRITICAL_EXIT();
    #endif
    
    return 0;
}
/****************************************************************************
**Description:        ��ȡtimerID
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170320 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
u32 dev_user_gettimeID(void)
{
    return drv_systick_get_msid();
}
/****************************************************************************
**Description:        ��鳬ʱʱ�䵽
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:δ��
                    1:ʱ�䵽
**Created by:        pengxuebin,20170320 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
u32 dev_user_querrytimer(u32 Time, u32 Delay)
{
    return drv_systick_querry_ms(Time, Delay);
}

/**
 * @brief  �ж��Ƿ�ʱ
 * @brief  ΢��
 * @param [in]  Time:ʱ��ID
 * @param [in]  Delay:��ʱʱ��
 * @retval 1:�Ѿ���ʱ  0:û�г�ʱ
 */
u32 dev_user_querrytimer_us(u32 Time, u32 Delay)
{
    return drv_systick_querry_us(Time, Delay);
}

/****************************************************************************
**Description:        usӲ��ʱ
**Input parameters:    
**Output parameters: 
**Returned value:
                   
**Created by:        pengxuebin,20170320 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void dev_user_delay_us(u32 nus)
{
    drv_systick_delay_us(nus);
}
/****************************************************************************
**Description:        msӲ��ʱ
**Input parameters:    
**Output parameters: 
**Returned value:
                   
**Created by:        pengxuebin,20170320 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void dev_user_delay_ms(u32 nms)
{
    drv_systick_delay_ms(nms);
}

