

#include "devglobal.h"
#include "sdk/mhscpu_sdk.h"

static u32 RTCcnt = 0;

u32 __mon_yday[2][13] = {
    { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365},
    { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366}
};   //闰年和正常年之间的月加区别
#define __isleap(y) (((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))

u32 Month_day_Max_C[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
u32 Month_day_Max_L[13] = {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
u32 Month_day_Accu_C[13] = {0,31, 59, 90,120,151,181,212,243,273,304,334,365};  
u32 Month_day_Accu_L[13] = {0,31, 60, 91,121,152,182,213,244,274,305,335,366};


#define YEAR_STA      1950
#define year_LAST     2085
#define DOW_STA       0   //星期天

#define SecsPerComYear  3153600//(365*3600*24)
#define SecsPerLeapYear 31622400//(366*3600*24)
#define SecsPerFourYear 126230400//((365*3600*24)*3+(366*3600*24))
#define SecsPerDay      (3600*24)
#define SecsPerHour     3600
#define SecsPerMin      60


//从1950年1月1日开始计时,可以记录135年，月表示1~12，日表示1~..,时表示0~23，分0~59，秒0~59
s32 dev_rtc_date_to_days(struct DATE Dt, u32 *days)
{
    u32 i;
    u32 j;

    if((Dt.year<YEAR_STA) || (Dt.year>year_LAST))
    {
        return -1;
    }
    if((Dt.mon>12) || (Dt.mon == 0) || (0 == Dt.day))
    {
        return -1;
    }
    if(__isleap(Dt.year))
    {
        if(Dt.day>Month_day_Max_L[Dt.mon])
        {
            return -1;
        }
    } 
    else
    {
        if(Dt.day>Month_day_Max_C[Dt.mon])
        {
            return -1;
        }
    }
    //计算年---->天
    j = 0;
    for(i=YEAR_STA; i<Dt.year; i++)
    {
        if(__isleap(i))
        {
            //闰年
            j += 366;
        }
        else
        {
            j += 365;
        }
    }
    //计算月转天
    if(__isleap(Dt.year))
    {   //闰年LeapYear
        j += Month_day_Accu_L[Dt.mon-1];
    }
    else
    {
        j += Month_day_Accu_C[Dt.mon-1];
    }
    //天
    *days = j+(Dt.day-1);
    return 0;
}

s32 dev_rtc_days_accu_dow(u32 days)
{
    return (((days+DOW_STA)%7));// + 1);
}
s32 dev_rtc_time_to_sec(struct TIME Tm, u32 *sec)
{
    if((Tm.hour>23) || (Tm.min>59) || (Tm.sec>59))
    {
        return -1;
    }
    *sec = (Tm.hour*SecsPerHour) + (Tm.min*SecsPerMin) + Tm.sec;
    return 0;
}         
//总天数计算秒
u32 dev_rtc_days_accu_sec(u32 days)
{
    return(days * SecsPerDay);
}


s32 dev_rtc_date_to_sec(struct DATE *Dt, struct TIME Tm, u32 *sec)
{
    u32 days;
    s32 ret;

    
    ret = dev_rtc_date_to_days(*Dt, &days);
    if(ret < 0)
    {
        return ret;    
    }

    ret = dev_rtc_time_to_sec(Tm, sec); 
    if(ret < 0)
    {
        return ret;    
    }
    Dt->dow = dev_rtc_days_accu_dow(days);

    *sec += dev_rtc_days_accu_sec(days);
    return 0;
}



u32 dev_rtc_readcnt(void)
{
    u8 tempdata[16];
    s32 ret;
    u32 cnt = 0;

    memset(tempdata,0,sizeof(tempdata));
    ret = fs_read_file("/mtd0/rtccnt",tempdata,16,0);
    if(ret == 16)
    {
        if(memcmp(tempdata,"rtcflag0",8) == 0)
        {
            #ifdef SXL_DEBUG
            dev_debug_printf("\r\ndev_rtc_readcnt flag set\r\n");
            #endif
            for(ret = 0;ret < 4;ret++)
            {
                cnt = (cnt<<8) + tempdata[8+ret];
            }
            
            
        }
        
    }
    
    
    #ifdef SXL_DEBUG
    dev_debug_printf("\r\ndev_rtc_readcnt cnt = %d\r\n",cnt);
    #endif
    
    return cnt;
    
    
}



s32 dev_rtc_savecnt(u32 rtccnt)
{
    
    u8 tempdata[16];
    s32 ret;

    memset(tempdata,0,sizeof(tempdata));
    memcpy(tempdata,"rtcflag0",8);
    tempdata[8] = (u8)(rtccnt>>24);
    tempdata[9] = (u8)(rtccnt>>16);
    tempdata[10] = (u8)(rtccnt>>8);
    tempdata[11] = (u8)(rtccnt);
    
    
    ret = fs_write_file("/mtd0/rtccnt",tempdata,16);
    #ifdef SXL_DEBUG
    dev_debug_printf("\r\ndev_rtc_savecnt cnt = %d\r\n",ret);
    #endif
    
    if(ret == 16)
    {
        return DEVSTATUS_SUCCESS;
    }
    
    
  
    return DEVSTATUS_ERR_FAIL;
    
    
}

/*******************************************************************
Author:   
Function Name:  void dev_rtc_open(void)
Function Purpose:open RTC
Input Paramters: 
Output Paramters:N/A
Return Value:      N/A
Remark: N/A
Modify: N/A  //sxl?需要完善
********************************************************************/
s32 dev_rtc_open(void)
{
    //读取初始时间日期
    
    dev_debug_printf("\r\ndev_rtc_open adjust\r\n");
    RTCcnt = dev_rtc_readcnt();
    
    dev_debug_printf("\r\ndev_rtc_open RTCcnt = %d\r\n",RTCcnt);
    /* start Tbc count */
    if(RTC_GetRefRegister() == 0)
    {
        dev_debug_printf("\r\n reset cnt 1\r\n");
        RTC_ResetCounter();        //rtc stop counting and counter cleared to 0
        RTC_SetRefRegister(RTCcnt);
    }
    else if(RTC_GetRefRegister()!=RTCcnt)
    {
        dev_debug_printf("\r\n reset cnt 2\r\n");
        RTC_ResetCounter();        //rtc stop counting and counter cleared to 0
        RTC_SetRefRegister(RTCcnt);
    }

    
    RTC_waitstartcount();
    
    
    dev_debug_printf("dev_rtc_open cnt start:%08x\r\n",RTC_GetCounter());
    return 0;
    
}



/*******************************************************************
Author:   
Function Name:  s32 dev_set_time(struct DATE * Dt,struct TIME * Tm)
Function Purpose:set date & time
Input Paramters:  DATE - date   TIME- time
Output Paramters:N/A
Return Value:      DEVSTATUS_SUCCESS        - success
                 DEVSTATUS_ERR_PARAM_ERR    -  parameters error
Remark: N/A
Modify: N/A
********************************************************************/
s32 dev_set_time(struct DATE * Dt,struct TIME * Tm)
{
    u32 sec;
    s32 ret;
    uint32_t i = 0xffff;
    
    ret = dev_rtc_date_to_sec(Dt, *Tm, &sec);
    if(ret < 0)
    {
        return ret;
    } 

    if(sec == 0)
    {
        sec += 1;
    }
    //保存second值
    if(dev_rtc_savecnt(sec) != DEVSTATUS_SUCCESS)
    {
        return DEVSTATUS_ERR_FAIL;
    }

    RTCcnt = sec;
    while(i--);
    RTC_ResetCounter();        //rtc stop counting and counter cleared to 0
    RTC_SetRefRegister(RTCcnt);
    
    
    return DEVSTATUS_SUCCESS;
    
}



s32 dev_rtc_sec_to_date(struct DATE *Dt, struct TIME *Tm, u32 sec)
{
    u32 i,j;
    u32 days;
    u32 *ap;

    days = (sec/SecsPerDay);
    //处理时间
    i = (sec%SecsPerDay);
    Tm->hour = (i/SecsPerHour);
    i = (i%SecsPerHour);
    Tm->min = (i/SecsPerMin);
    Tm->sec = (i%SecsPerMin);
    //处理周
    Dt->dow = dev_rtc_days_accu_dow(days);
    //处理日期
    //处理年
    i = YEAR_STA;
    while(1)  //sxl?   这里会不会慢点,要再进行处理
    {    
        if(__isleap(i))
        {
            //闰年
            j = 366;
        }
        else
        {
            j = 365;
        }
        if(days>=j)
        {
            days -= j;
            i ++;
        }
        else
        {
            break;
        }
    }
    Dt->year = i;
    //处理月
    if(__isleap(i))
    {
        ap = Month_day_Accu_L;
    }
    else
    {
        ap = Month_day_Accu_C;
    }
    for(i=1; i<13; i++)
    {
        if(days < ap[i])
        {
            Dt->mon = i;
            days -= ap[i-1];
            break;
        }
    }
    if(i>=13)return (-1);
    //天
    Dt->day = days+1;
    return 0;
}


/*******************************************************************
Author:   
Function Name:   s32 dev_get_time(struct DATE * Dt,struct TIME * Tm)
Function Purpose:get date & time
Input Paramters:  DATE - date   TIME- time
Output Paramters:N/A
Return Value:      DEVSTATUS_SUCCESS        - success
                 DEVSTATUS_ERR_PARAM_ERR    -  parameters error
Remark: N/A
Modify: N/A
********************************************************************/
s32 dev_get_time(struct DATE * Dt,struct TIME * Tm)
{

     u32 sec;

    sec = RTC_GetCounter();
    sec += RTC_GetRefRegister();
    return dev_rtc_sec_to_date(Dt, Tm, sec);
    
}

void RTC_IRQHandler(void)
{
    dev_debug_printf("RTC_IRQHandler In\n");
    RTC_ITConfig(DISABLE);
    RTC_ClearITPendingBit();
    NVIC_ClearPendingIRQ(RTC_IRQn);
}
/****************************************************************************
**Description:      设置RTC n秒后报警 
**Input parameters:    
**Output parameters: 
**Returned value:
**Created by:        pengxuebin,20170629
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_rtc_set_alarm(s32 flg, u32 sec)
{
    u32 t;

    if(flg)
    {
        t = RTC_GetRefRegister();
        RTC_SetAlarm(t+sec);
        RTC_ITConfig(ENABLE);
    }
    else
    {
        RTC_ITConfig(DISABLE);    
    }
}

void Test_rtc(void)
{
    int count,i;
    
    
    struct DATE Dt;
    struct TIME Tm;
    
    dev_debug_printf("%s(%d):\r\n", __FUNCTION__, __LINE__);
    dev_user_delay_ms(20);
    
    dev_get_time(&Dt,&Tm);
    
    dev_debug_printf("%s(%d):ncurrent date&time:%04d-%02d-%02d %02d:%02d:%02d\r\n", __FUNCTION__, __LINE__,Dt.year,Dt.mon,Dt.day,Tm.hour,Tm.min,Tm.sec);
    dev_user_delay_ms(20);

    Dt.year = 2000;
    Dt.mon = 2;
    Dt.day= 28;

    Tm.hour = 23;
    Tm.min = 59;
    Tm.sec = 57;
    
    dev_set_time(&Dt,&Tm);
    
    for(i = 0;i < 3;i++)
    {
        /*for(count = 0;count < 10000000;count++)
        {
            
        }*/
        dev_user_delay_ms(500);
        dev_get_time(&Dt,&Tm);
    
        dev_debug_printf("%s(%d):current date&time:%04d-%02d-%02d %02d:%02d:%02d\r\n", __FUNCTION__, __LINE__, Dt.year,Dt.mon,Dt.day,Tm.hour,Tm.min,Tm.sec);
        
    }

    Dt.year = 2000;
    Dt.mon = 12;
    Dt.day= 31;

    Tm.hour = 23;
    Tm.min = 59;
    Tm.sec = 57;
    
    dev_set_time(&Dt,&Tm);
    
    for(i = 0;i < 3;i++)
    {
        /*for(count = 0;count < 10000000;count++)
        {
            
        }*/
        dev_user_delay_ms(500);
        dev_get_time(&Dt,&Tm);
    
        dev_debug_printf("%s(%d):current date&time:%04d-%02d-%02d %02d:%02d:%02d\r\n", __FUNCTION__, __LINE__, Dt.year,Dt.mon,Dt.day,Tm.hour,Tm.min,Tm.sec);
        
    }
    
    
}

