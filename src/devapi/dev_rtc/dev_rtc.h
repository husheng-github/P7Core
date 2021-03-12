

#ifndef _DEV_RTC_H_
#define _DEV_RTC_H_


struct DATE
{
   uint16_t year;  //Range from 1970 to 2099.
   uint8_t  mon;   //Range from 1 to 12.
   uint8_t  day;   //Range from 1 to 31.
   uint8_t  dow;
};


struct TIME
{
   uint8_t  hour;   //Range from 0 to 23.
   uint8_t  min;    //Range from 0 to 59.
   uint8_t  sec;    //Range from 0 to 59.
};



extern s32 dev_rtc_open(void);
extern s32 dev_set_time(struct DATE * Dt,struct TIME * Tm);
extern s32 dev_get_time(struct DATE * Dt,struct TIME * Tm);
s32 dev_rtc_set_alarm(s32 flg, u32 sec);


extern void Test_rtc(void);




#endif

