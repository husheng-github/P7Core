/********************************Copyright ( c ) ********************************
**                  DAQU 
**                         
**                        
**
**Created By:     pengxuebin,20170322      
** Created Date:     
** Version:        
** Description:    用户定时器接口函数
  
****************************************************************************/
#include "devglobal.h"
#include "drv_systick.h"

#define USER_TIMER_MAX        20
static volatile u32 *g_timer_p[USER_TIMER_MAX] = {0};
#define TIMER_BASE        10   //10ms定时

static u8 g_timer_initflg = 0;
/****************************************************************************
**Description:        用户定时器处理
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
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
        //用tick做系统定时器
        drv_systick_int();
        g_timer_initflg = 1;
        
    }
}
/****************************************************************************
**Description:        用户定时器处理
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170320 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
// 10ms 定时处理
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
**Description:        打开用户定时器
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
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
**Description:        关闭用户定时器
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
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
**Description:        获取timerID
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170320 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
u32 dev_user_gettimeID(void)
{
    return drv_systick_get_msid();
}
/****************************************************************************
**Description:        检查超时时间到
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:未到
                    1:时间到
**Created by:        pengxuebin,20170320 
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
u32 dev_user_querrytimer(u32 Time, u32 Delay)
{
    return drv_systick_querry_ms(Time, Delay);
}

/**
 * @brief  判断是否超时
 * @brief  微秒
 * @param [in]  Time:时间ID
 * @param [in]  Delay:超时时间
 * @retval 1:已经超时  0:没有超时
 */
u32 dev_user_querrytimer_us(u32 Time, u32 Delay)
{
    return drv_systick_querry_us(Time, Delay);
}

/****************************************************************************
**Description:        us硬延时
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
**Description:        ms硬延时
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

