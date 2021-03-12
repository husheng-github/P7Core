

#include "devglobal.h"
#include "ddi_misc.h"
#include "table/PitTable.h"
#include "pciglobal.h"
#include "ddi_audio.h"








void ddi_misc_appprinf(const char *fmt,...)
{
	
	#ifdef SYS_APP_DEBUG_EN
    s32 length = 0;
    s8 *pt;

	
    
    pt = (s8*)fmt;
    while(*pt!='\0')
    {
        length++;
        pt++;
    }

    dev_com_write(DEBUG_PORT_NO, (u8*)fmt, length);  //写串口成功  

	
	#endif
	
}

s32 ddi_misc_msleep(u32 nMs)
{
  #if 0  
	u32 timeid; 
    #if 0   //sxl?
    OS_ERR err;

    OSTimeDly(nMs,
              OS_OPT_TIME_DLY,
              &err);
	#endif
	if(nMs < 10)
	{
		dev_user_delay_ms(nMs);
	}
	else
	{
		timeid = dev_user_gettimeID();
		while(1)
		{
			if(dev_user_querrytimer(timeid,nMs))
			{
				break;
			}
		}
	}
  #endif
	dev_user_delay_ms(nMs);
    return DDI_OK;
}


s32 ddi_misc_set_time(const u8 *lpTime)
{
    #ifdef RTC_ENABLE
    u8 tmp, tmp_h, tmp_l;

    u16 year;
    u8 mon;
    u8 day;

    u8 hour;
    u8 min;
    u8 sec;
    
    struct DATE  Dt;
    struct TIME  Tm;
     

    tmp = *(lpTime+0);
    tmp_h = tmp>>4;
    tmp_l = tmp&0x0f;
    if(tmp_h > 9)
        return DDI_EINVAL;
    if(tmp_l > 9)
        return DDI_EINVAL;
    year = tmp_h*10+tmp_l;

    tmp = *(lpTime+1);
    tmp_h = tmp>>4;
    tmp_l = tmp&0x0f;
    if(tmp_h > 9)
        return DDI_EINVAL;
    if(tmp_l > 9)
        return DDI_EINVAL;
    mon = tmp_h*10+tmp_l;

    tmp = *(lpTime+2);
    tmp_h = tmp>>4;
    tmp_l = tmp&0x0f;
    if(tmp_h > 9)
        return DDI_EINVAL;
    if(tmp_l > 9)
        return DDI_EINVAL;
    day = tmp_h*10+tmp_l;

    tmp = *(lpTime+3);
    tmp_h = tmp>>4;
    tmp_l = tmp&0x0f;
    if(tmp_h > 9)
        return DDI_EINVAL;
    if(tmp_l > 9)
        return DDI_EINVAL;
    hour = tmp_h*10+tmp_l;

    tmp = *(lpTime+4);
    tmp_h = tmp>>4;
    tmp_l = tmp&0x0f;
    if(tmp_h > 9)
        return DDI_EINVAL;
    if(tmp_l > 9)
        return DDI_EINVAL;
    min = tmp_h*10+tmp_l;

    tmp = *(lpTime+5);
    tmp_h = tmp>>4;
    tmp_l = tmp&0x0f;
    if(tmp_h > 9)
        return DDI_EINVAL;
    if(tmp_l > 9)
        return DDI_EINVAL;
    sec = tmp_h*10+tmp_l;

    //uart_printf("%d-%d-%d, %d:%d:%d\r\n", year, mon, day, hour, min, sec);

    if(year >=50)
        year += 1900;
    else
        year += 2000;

    Dt.year = year;
    Dt.mon = mon;
    Dt.day = day;
    
    Tm.hour = hour;
    Tm.min = min;
    Tm.sec = sec;
    
    dev_set_time(&Dt, &Tm);
    
    return DDI_OK;
	#else
    return DDI_ERR;
	#endif
}


s32 ddi_misc_get_time(u8 *lpTime)
{
    #ifdef RTC_ENABLE
    struct DATE Dt;
    struct TIME Tm;

    if( dev_get_time(&Dt, &Tm) != 0)
    {
        return DDI_ERR;
    }
	

    //年
    *(lpTime+0) = ((Dt.year%100/10)<<4) + Dt.year%10;
    *(lpTime+1) = ((Dt.mon/10)<<4) + (Dt.mon%10);
    *(lpTime+2) = ((Dt.day/10)<<4) + (Dt.day%10);
    *(lpTime+3) = ((Tm.hour/10)<<4) + (Tm.hour%10);
    *(lpTime+4) = ((Tm.min/10)<<4) + (Tm.min%10);
    *(lpTime+5) = ((Tm.sec/10)<<4) + (Tm.sec%10);

    return DDI_OK;

	#else
    return DDI_ERR;
	#endif
	
}


s32 ddi_misc_get_tick(u32 *nTick)  //系统滴答
{
    *nTick = dev_user_gettimeID();
    return DDI_OK;
}



s32 ddi_misc_sleep (s32 status)
{
//    dev_user_delay_ms(status);  
//    dev_power_sleep(status);
    return DDI_OK;
	
}

s32 ddi_misc_get_batvol(void)
{
//    return dev_power_battery_vol();
}
s32 ddi_misc_bat_status(power_status_t *lp_power_status)
{
  #if 0
    u8 flg;
    u16 status;
    u16 level;
    s32 ret;

    if(lp_power_status == NULL)
    {
        return DDI_ERR;
    }
    memset(lp_power_status, 0, sizeof(power_status_t));
    dev_power_recharge(&flg);
	//dev_debug_printf("\r\ncharge flag = %d\r\n",flg);  //sxl?20180731
    if(2 == flg)
    {
        //充满,有外电，未充电
        status = DDI_POWER_AD_CHARGE_FULL;
        lp_power_status->m_aclinestate = 1;
        lp_power_status->m_batteryflg = 0; 
        //return DDI_BAT_CHGNO;
    }
    else if(1 == flg)
    {
        //充电中
        status = DDI_POWER_AD_CHARGE;
        lp_power_status->m_aclinestate = 1;
        lp_power_status->m_batteryflg = 1; 
        //return DDI_BAT_CHGING;
    }
    else
    {
        //无外电
        status = DDI_POWER_NO_AD;
        lp_power_status->m_aclinestate = 0;
        lp_power_status->m_batteryflg = 0; 
    }
    dev_power_battery(&flg);
    if(0 == flg)
    {
        level = DDI_POWER_NULL;    //电池电量不足  
    }
    else if(1 == flg)
    {
        level = DDI_POWER_ZERO;  //电池电量0格
    }
    else if(2 == flg)
    {
        level = DDI_POWER_ONE;  
    }
    else if(3 == flg)
    {
        level = DDI_POWER_TWO;  
    }
    else if(4 == flg)
    {
        level = DDI_POWER_THREE;  
    } 
    else if(5==flg)
    {
        level = DDI_POWER_FOUR;
    }
    else if(6==flg)
    {
        level = DDI_POWER_FIVE;
    }
    else //if(7 == flg)
    {
        level = DDI_POWER_FIVE;
        if(status == DDI_POWER_AD_CHARGE)
        {   //修正1902S报充不满的问题
            status = DDI_POWER_AD_CHARGE_FULL; 
        }
    }
    lp_power_status->m_batterylevel = level;
    
    
	//sxl?
	return (status<<8)|level;
  #else
    return 0;
  #endif
}



s32 dev_misc_close_all(void)
{
//	dev_lcd_clear_ram();
//    dev_lcd_brush_screen();
	#ifdef DDI_BT_ENABLE
	dev_bt_close();
	#endif
//    dev_watchdog_close();
//    dev_lcd_close();
//    dev_lcd_ioctl(DDI_LCD_CTL_BKLIGHT, 0, 0);
//    api_rf_close();
    dev_led_close();
    dev_user_delay_ms(10);
    dev_wireless_poweroff();
}

s32 ddi_misc_poweroff(void)
{
    u32 i;
    u32 cnt;

    #if 0
    dev_misc_close_all();
    while(1)
    {
        //等待松键
        i = drv_keypad_get_powerkey();
        if(i == 0)
        {
            //dev_debug_printf("set latch power low\r\n");
            dev_power_switch_ctl(0);
            break;
        }  
    }
    #endif
#if 1//pengxuebin,20191026,??    
//dev_debug_printf("%s(%d):\r\n", __FUNCTION__, __LINE__);
//dev_user_delay_ms(20);
    //dev_audio_ioctl(DDI_AUDIO_CTL_BUZZER, 0, 200);
    dev_misc_close_all();
    while(1)
    {
        //等待松键
        i = drv_keypad_get_powerkey();
        if(i == 0)
        {
            dev_power_switch_ctl(0);
            break;
        }  
    }
  #if 0  
    cnt = 0;
    while(1)
    {
        //有外电时程序关不掉，等待按键按下重启
        dev_user_delay_ms(10);
        i = drv_keypad_get_powerkey();
        if(i == 1)
        {
            cnt ++;
            if(cnt >20)
            {
                dev_misc_reboot(0);
            }
        }
        else
        {
            cnt == 0;
        } 
    }
  #else
    //有外电，重启
    dev_misc_reboot(0);
  #endif  
#endif  
	return DDI_OK;
}


s32 ddi_misc_reboot(void)
{
	dev_misc_close_all();
//    dev_power_switch_ctl(0);
    //关闭外设3.3V电VRFLAHEN(b2)
    //dev_smc_systemsoftreset();
    dev_misc_reboot(ST_REBOOT);
	return DDI_OK;
}

s32 ddi_misc_probe_dev(u32 nDev)
{    
    s32 ret = DDI_OK;

    switch(nDev)
    {
    case DEV_IC_CARD:
        break;

    case DEV_MAG_CARD:
        break;

    case DEV_RF_CARD:
            #ifndef DDI_RF_ENABLE
        	ret = DDI_ENODEV;
			#endif
        break;

    case DEV_TOUCH_SCREEN:
        //ret = DDI_ENODEV;
        break;

    case DEV_BLUETOOTH:
        ret = DDI_ENODEV;
        break;

    case DEV_WIRELESS_MODULE:
        ret = DDI_ENODEV;
        break;

    case DEV_MODEM:
        ret = DDI_ENODEV;
        break;

    case DEV_WIFI:
        //ret = DDI_ENODEV;
        break;

    case DEV_ETHERNET:
        ret = DDI_ENODEV;
        break;

    case DEV_GPS:
        ret = DDI_ENODEV;
        break;

    case DEV_SD_CARD:
        ret = DDI_ENODEV;
        break;

    case DEV_BARCODE_GUN:
        ret = DDI_ENODEV;
        break;

    case DEV_FINGERPRINT_ID:
        ret = DDI_ENODEV;
        break;

    case DEV_CAMERA:
        ret = DDI_ENODEV;
        break;
    case DEV_PRINT:
        ret = DDI_ENODEV;
        break;
    case DEV_SPMCU:
        ret = DDI_ENODEV;
        break;
    default:
        ret = DDI_ENODEV;
        break;
    }
    return ret;
}

s32 ddi_user_timer_open(u32 *timer)
{
    return dev_user_timer_open(timer);
}
s32 ddi_user_timer_close(u32 *timer)
{
    return dev_user_timer_close(timer);
}


s32 ddi_misc_sleep_with_alarm(s32 sec, s32 type, s32 status)
{
    return DDI_ERR;
}




s32 ddi_misc_bat(struct _strBatSta *sta)
{
    if(sta == NULL)
        return DDI_ERR;
	//sxl?
	/*
	sta->vol = SysPwr.batvol;
	sta->per = 50;//need fix
	sta->exp = SysPwr.ex_pwr;
	sta->chg = SysPwr.chg_sta;
	sta->stag = SysPwr.stage;
	sta->bottonvol = SysPwr.bottonvol;
	*/
    return DDI_OK;
}


s32 ddi_misc_rtcbatterybat_check(s32 en)
{
	//sxl?
	return DDI_OK;
}


s32 ddi_misc_key_check_f_3(void)
{
	return DDI_OK;
}

#if 0
void ddi_misc_get_corever(u8 *p)
{
	pitver_get_corever(p);
}
void ddi_misc_get_bootver(u8 *p)
{
   pitver_get_bootver(p);   
}



void ddi_misc_get_hardwarever(u8 *p)  //sxl?2017 硬件版本号还要处理
{
    pitver_get_hardwarever(p);   
}
#endif

void ddi_misc_get_ver(u8 type, u8 *p)
{
    if(type == 0)
    {
        pitver_get_hardwarever(p);
    }
    else if(type == 1)
    {
        pitver_get_bootver(p);
        
    }
    else if(type == 2)
    {
	    pitver_get_corever(p);
    }
	else if(type == 3)
    {
	    pitver_get_appver(p);
    }
    else
    {
        *p = 0;
    }
}

s32 ddi_misc_get_firmwareversion(FIRMWARETYPE firmwaretype, u8 *lpOut)
{
    s32 res = DDI_OK;
    
    if(lpOut  == NULL)
        return DDI_EINVAL;
    
    switch(firmwaretype)
    {
    case FIRMWARETYPE_HW:
        ddi_misc_get_ver(0, lpOut);
        break;
    case FIRMWARETYPE_APBOOT://boot 版本
        pitver_get_bootver(lpOut);
        break;

    case FIRMWARETYPE_APCORE://core
        pitver_get_corever(lpOut);
        break;
    case FIRMWARETYPE_APAPP:
        pitver_get_appver(lpOut);
        break;
    case FIRMWARETYPE_APVIVA:
    case FIRMWARETYPE_APMANAGE:
    case FIRMWARETYPE_SPBOOT:
    case FIRMWARETYPE_SPCORE:
    case FIRMWARETYPE_SPAPP:
    case FIRMWARETYPE_ROMTYPE:
    default:
        res = DDI_ERR;
        break;
    }

    return res;
}

s32 ddi_misc_read_dsn(SNType sntype, u8 *lpOut)
{
    s32 ret=DDI_ERR;

    switch(sntype)
    {
    case SNTYPE_POSSN:
        ret = ddi_manage_read_hwsn(0,&lpOut[1]);
        if(ret == DDI_OK)
        {
            lpOut[0] = strlen(&lpOut[1]);
        }
        break;
    case SNTYPE_APCUPID:
        ret = dev_ReadPosCPUID(lpOut);
        break;
    case SNTYPE_SPCUPID:
        break;
    }
    return ret;
}

u32 ddi_pci_readtamperstatus(u8 *unlocktimes)
{
//    return pcitamper_manage_readtamperstatus(unlocktimes);
}
u32 ddi_pci_getcurrenttamperstatus(void)
{
//    return pcitamper_manage_getcurrenttamperstatus();
}
u32 ddi_pci_unlock(void)
{
//    return pcitamper_manage_unlock();
}
u32 ddi_pci_opendryice(u8 dryicetype)
{
//    return pcitamper_manage_dryiceopen(dryicetype);
}
u32 ddi_pci_getdryiceconfig(void)
{
//    return pcitamper_manage_getdryiceconfig();
}
SPSTATUS* ddi_pci_getspstatuspointer(void)
{
//    return pci_core_spstatusget();
}


s32 dev_ReadPosCPUID(u8 *lpOut)
{
    u8 *cpuid = (u8 *)0x40008C04;//0x4000900;设备序列号       //1903 

    OTP_PowerOn();
    lpOut[0]= 16;
    memcpy(&lpOut[1],cpuid,16);
    OTP_PowerOff();
    return DDI_OK;

}


//#define HWSN_FILENAME       "/mtd2/hwsninfo"
//const u8 TRENDIT_HWSN_FLG[] = {0x54,0x52,0x45,0x4E,0x44,0x49,0x54,0X00};//"TRENDIT"
//#define TRENDIT_HWSN_FLG_LEN   8;
s32 ddi_manage_get_hwsninfo(hwsn_info_t *lp_hwsn_info)
{
    s32 ret;
    u8 crc[2];

  #if 0  
    ret = ddi_file_read(HWSN_FILENAME, (u8*)lp_hwsn_info, HWSN_INFO_LEN, 0);
//dev_debug_printf("%s(%d):ret=%d\r\n", __FUNCTION__, __LINE__, ret);
    if(ret <= 0)
    {
        return -1;
    }
    if(memcmp(lp_hwsn_info->m_flg, TRENDIT_HWSN_FLG, 8))
    {
        return -1;
    }
  #else
    ret = dev_misc_machineparam_get(MACHINE_PARAM_HWSNINFO, (u8*)lp_hwsn_info, HWSN_INFO_LEN, 0);
    if(ret < 0)
    {
        return -1;
    }
  #endif
    if((lp_hwsn_info->m_snlen == 0) || (lp_hwsn_info->m_snlen>42))
    {
        return -1;
    }
 
 #if 0      
    //crc校验
    dev_maths_calc_crc16(0, (u8*)lp_hwsn_info, HWSN_INFO_LEN-2, crc);
    if(memcmp(lp_hwsn_info->m_crc, crc, 2))
    {
        return -1;
    }
  #endif  
    return 0;
    
}

s32 ddi_manage_set_hwsninfo(u8 type, hwsn_info_t *lp_hwsn_info)
{
    s32 ret;
    u8 crc[2];
    hwsn_info_t l_hwsn_info;

    if(type == 1)
    {
        //判断机身号是否存在
        ret = ddi_manage_get_hwsninfo(&l_hwsn_info);
        if(ret == 0)
        {
            //存在机身号,但机身号不一致
            if(memcmp(lp_hwsn_info->m_vendorid, l_hwsn_info.m_vendorid, 8))
            {
                return -1;
            }
            //
            if(lp_hwsn_info->m_snlen != l_hwsn_info.m_snlen)
            {
                return -1;
            }
            //
            if(memcmp(lp_hwsn_info->m_sn, l_hwsn_info.m_sn, lp_hwsn_info->m_snlen))
            {
                return -1;
            }
            
        }
    }
   #if 0 
    memcpy(lp_hwsn_info->m_flg, TRENDIT_HWSN_FLG, 8); 

    //计算CRC
    dev_maths_calc_crc16(0, (u8*)lp_hwsn_info, HWSN_INFO_LEN-2, lp_hwsn_info->m_crc);

    ret = ddi_file_write(HWSN_FILENAME, (u8*)lp_hwsn_info, HWSN_INFO_LEN);
    if(ret <= 0)
    {
        return -1;
    }
    //校验
    ret = ddi_manage_get_hwsninfo(&l_hwsn_info);
    if(ret == 0)
    {
        return 0;
    }
    else
    {
        return -1;
    }
   #else
    ret = dev_misc_machineparam_set(MACHINE_PARAM_HWSNINFO, (u8*)lp_hwsn_info, HWSN_INFO_LEN, 0);
    return ret;
   #endif
}

s32 ddi_manage_read_hwsn(u8 type, u8 *sn)
{
    s32 ret;
    hwsn_info_t l_hwsn_info;
    s32 i;

    ret = ddi_manage_get_hwsninfo(&l_hwsn_info);
//dev_debug_printf("%s(%d):ret=%d, snlen=%d\r\n", __FUNCTION__, __LINE__, ret, l_hwsn_info.m_snlen);    
//dev_printformat(NULL, (u8*)&l_hwsn_info, HWSN_INFO_LEN);
    if((ret == 0) && (l_hwsn_info.m_snlen>=4))
    {
        i = 0;
        if(type==1)
        {
            //厂商代码
            memcpy(sn, l_hwsn_info.m_vendorid, 6);
            i += 6;
            //终端类型
            memcpy(&sn[i], l_hwsn_info.m_termialno, 2);
            i += 2;
        }
        memcpy(&sn[i], l_hwsn_info.m_sn, l_hwsn_info.m_snlen);
        i += l_hwsn_info.m_snlen;
        sn[i] = 0;
//dev_debug_printf("%s(%d):SN=%s\r\n", __FUNCTION__, __LINE__, sn);    
        return DDI_OK;
    }
    else
    {
        return DDI_ERR;
    }
}

s32 ddi_manage_clean_hwsn(void)
{
    hwsn_info_t l_hwsn_info;
    s32 ret;

    memset((u8*)&l_hwsn_info, 0, HWSN_INFO_LEN);
    
//    ret = ddi_file_write(HWSN_FILENAME, (u8*)&l_hwsn_info, HWSN_INFO_LEN);
    ret = dev_misc_machineparam_del(MACHINE_PARAM_HWSNINFO, (u8*)&l_hwsn_info, HWSN_INFO_LEN);
//dev_debug_printf("%s(%d):\r\n", __FUNCTION__, __LINE__); 
    if(ret < 0)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}



s32 ddi_misc_ioctl(u32 nCmd, u32 lParam, u32 wParam)
{
    s32 ret = DDI_EINVAL;
    strMCUparam_ioctl_t *lp_mcuparam;
    mem_info_t *mem_info = NULL; 
    
    switch(nCmd)
    {
    case MISC_IOCTL_SET_UPDATEFLG:       //设置UPDATE标志
      #if 0
        ret = bootdownload_checkupdatefile();
        if(ret != 0)
        {
            ret = DDI_EDATA;
            break;
        }
      #endif
        ret = dev_misc_updateflg_set(1);
        break;
    case MISC_IOCTL_CLR_UPDATEFLG:       //清除UPDATE标志    
        ret = dev_misc_updateflg_set(0);
        break;
    case MISC_IOCTL_GET_WAKEUPPIN:
 //       ret = drv_power_get_wakuppin();
        break;
    case MISC_IOCTL_SETHWSNANDSNKEY:     //设置硬件序列号及序列号密钥
        ret = ddi_manage_set_hwsninfo(0, (hwsn_info_t*)lParam);
        break;
    case MISC_IOCTL_READHWSN:            //读SN
        ret = ddi_manage_read_hwsn(lParam, (u8*)wParam);
        break;
    case MISC_IOCTL_CLEARHWSN:           //清硬件序列号
        ret = ddi_manage_clean_hwsn();
        break;
    
//    case MISC_IOCTL_SETHWSNANDSNKEY:  0x82      //设置硬件序列号及序列号密钥
    case MISC_IOCTL_GETHWSNANDSNKEY:  //0x83      //读取硬件序列号及序列号密钥
        ret = ddi_manage_get_hwsninfo((hwsn_info_t*)wParam);
        break;
    case MISC_IOCTL_SETMCUPARAM:         //设置参数
        lp_mcuparam = (strMCUparam_ioctl_t*)lParam;
        ret = dev_misc_machineparam_set(lp_mcuparam->m_sn, lp_mcuparam->m_param, lp_mcuparam->m_paramlen, lp_mcuparam->m_offset);
        break;
    case MISC_IOCTL_GETMCUPARAM:         //获取参数
        lp_mcuparam = (strMCUparam_ioctl_t*)lParam;
        ret = dev_misc_machineparam_get(lp_mcuparam->m_sn, lp_mcuparam->m_param, lp_mcuparam->m_paramlen, lp_mcuparam->m_offset);
        break;
    case MISC_IOCTL_MCUPARAMINITIAL:      //参数初始化
        break;
    case MISC_IOCTL_GETHWVER:           //获取硬件版本号
        ret = dev_misc_get_hwver();
        break;
    case MISC_IOCTL_GETGPRSMODULEVERSION:    //获取GPRS模块版本号
        break;
    case MISC_IOCTL_GETTMSMACHINETYPEID:  //获取TMS升级机器类型设别号
        sprintf((u8*)&lParam, "%02d%02d", CODE_HEX(MACHINE_P7_VALUE), 0);
        ret = (CODE_HEX(MACHINE_P7_VALUE)<<8)|0;
        TRACE_DBG("ret:%x %x", ret, (CODE_HEX(MACHINE_P7_VALUE)<<8));
        break;
        
    case MISC_IOCTL_GET_DEBUGFLAG:      //获取DEBUG标志
      #ifdef DEBUG_ALL_EN
        ret = 1;
      #else
        ret = 0;
      #endif
        break;
    case MISC_IOCTL_SPCANCEL:  
        break;
    case MISC_IOCTL_SETCHARGESTATUS:
        ret = DDI_OK;
        break;
    case MISC_IOCTL_GET_SYSTEM_MEM:      //获取内存空间
        mem_info = (mem_info_t *)lParam;
        if(NULL == mem_info)
        {
            ret = DDI_EINVAL;
            break;
        }
        ret = ddi_misc_get_mem_info(mem_info);
        break;
    case MISC_IOCTL_OPENCASHBOX:
        dev_misc_cashbox_open();
        ret = DDI_OK;
        break;
    case  MISC_IOCTL_GET_LANGUAGETYPE:    ///获取语言模式
        ret = dev_misc_language_get();
        break;
    case MISC_IOCTL_SET_LANGUAGETYPE:    ///设置语言模式
        ret = dev_misc_language_set(lParam);
        break;

    case MISC_IOCTL_GET_RESET_FLAG:  //1 reboot 2看门狗复位
        ret = dev_misc_reset_get_from_flash();
        break;

    case MISC_IOCTL_CLEAR_RESET_FLAG:  //清除标志
        ret = dev_misc_reset_set_to_flash(0);
        break;

    case MISC_IOCTL_CDC_OUTPUT_SWITCH:
        trendit_cdc_output_switch(lParam);
        ret = DDI_OK;
        break;
    default:
        break;
    }
	return ret;
}


s32 ddi_misc_set_mcuparam(u16 sn, u8 *param, u16 paramlen, u16 offset)
{
    strMCUparam_ioctl_t l_mcuparam;

    l_mcuparam.m_sn = sn;
    l_mcuparam.m_paramlen = paramlen;
    l_mcuparam.m_param = param;
    l_mcuparam.m_offset = offset;
    return ddi_misc_ioctl(MISC_IOCTL_SETMCUPARAM, (u32)&l_mcuparam, 0);
}

s32 ddi_misc_get_mcuparam(u16 sn, u8 *param, u16 paramlen, u16 offset)
{
    strMCUparam_ioctl_t l_mcuparam;

    l_mcuparam.m_sn = sn;
    l_mcuparam.m_paramlen = paramlen;
    l_mcuparam.m_param = param;
    l_mcuparam.m_offset = offset;
    return ddi_misc_ioctl(MISC_IOCTL_GETMCUPARAM, (u32)&l_mcuparam, 0);
}

