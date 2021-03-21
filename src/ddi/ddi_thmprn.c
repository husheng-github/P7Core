

#include "ddi_thmprn.h"

#ifdef DDI_THMPRN_ENABLE

static s8 paper_state = 0; 
extern  int print_nopaper_count;
extern iomux_pin_name_t  g_prt_loop_test_pin; 

s32 ddi_thmprn_open (void)
{
    pt_resume();
    return DDI_OK;
}

s32 ddi_thmprn_close(void)
{
	pt_sleep();	
	return DDI_OK;
}

s32 ddi_thmprn_feed_paper(s32 nPixels)
{  
    s32 ret;
  
    ret = dev_printer_get_status();
    if(ret == PT_STATUS_IDLE)
    {
        TPFeedLine(nPixels);
        return DDI_OK;
    }
    else if(ret == PT_STATUS_NOPAPER)
    {
        return DDI_EACCES;
    }
    else 
    {
        return DDI_EBUSY;
    }
}

s32 ddi_thmprn_print_image_file(u32 nOrgLeft, u32 nImageWidth, u32 nImageHeight, const u8 *lpImageName)
{
	return DDI_ERR;
}

s32 ddi_thmprn_print_image (u32 nOrgLeft, u32 nImageWidth, u32 nImageHeight, const u8 *lpImage)
{
    if (TPGetPaperDetect() != 0) //无纸
    {
    	return DDI_EACCES;  //无纸返回
    }

	if(nImageWidth > (LineDot/8))
	{
		return DDI_EDATA;
	}

	PrtPrintStart(lpImage,nImageWidth,nImageHeight);
	return DDI_OK;
}

s32 ddi_thmprn_print_comb_text (u32 nNum, const strPrnCombTextCtrl* lpPrnCombTextCtrl[])
{
	return DDI_ERR;
}

s32 ddi_thmprn_print_text (strPrnTextCtrl *lpPrnCtrl, const u8 *lpText )
{
	return DDI_ERR;
}

s32 ddi_thmprn_get_status (void)
{
    s32 ret;
  
    if(TPPrintIsOvertime())
    {
        TPPrintClearOvertimeFlag();
        return DDI_ETIMEOUT;
    }
    
    ret = dev_printer_get_status();
    if(ret == PT_STATUS_NOPAPER)
    {
    	return DDI_EACCES;  //无纸返回
    }
    else if(ret == PT_STATUS_BUSYING)
    {
        return DDI_EBUSY;
    } 
	else if(ret == PT_STATUS_HIGH_TEMP)
    {
        return DDI_EHIGHTEMP;
    }
    return DDI_OK;
}

/**
 * @brief 开逻辑电平直接获取纸的状态，不去open close打印机
 * @retval  DDI_OK  正常
 * @retval  DDI_EACCES  缺纸
 * @retval  DDI_EHIGHTEMP 过温
 */
s32 ddi_thmprn_get_status_with_open(void)
{
    s32 ret;
    s32 status = DDI_OK;
  
    pt_ctl_slp(0, 0);   //开逻辑电平
    dev_user_delay_ms(5);
    
    ret = dev_printer_get_status();
    if((ret == PT_STATUS_NOPAPER) || (dev_printer_getpaper_state() == PT_STATUS_NOPAPER))
    {
    	status =  DDI_EACCES;  //无纸返回
    }
    else if(ret == PT_STATUS_BUSYING)
    {
        status = DDI_EBUSY;
    } 
	else if(ret == PT_STATUS_HIGH_TEMP)
    {
        status = DDI_EHIGHTEMP;
    }
    
    pt_ctl_slp(0, 1);  //关逻辑电平
    return status;
}

s32 ddi_thmprn_esc_p(u8 *inEsc, u32 inLen)
{
    s32 ret;

    esc_p_init();   //Add by xiaohonghui 每次打印前把格式初始化
    ret = esc_set_data(inEsc, inLen);

    return ret;  //返回插入到第几个data
}

s32 ddi_thmprn_esc_loop(void)
{
    s32 ret = DDI_OK;
    
    do{
        if(TPPrintIsOvertime())
        {
            ret = DDI_ERR;
            break;
        }
        
        paper_state = TPPrinterReady();
        if((esc_fifo_isEmpty() == 0) && (paper_state == 1))// && (ret == DDI_OK))
        {
            esc_p();
            ret = DDI_OK;
            break;
        } 
        else
        {
            if(paper_state == 0)
            {
                dev_debug_printf("----- %s ----- %d ----paper_state = %d \r\n", __func__, __LINE__, paper_state);
            }
            ret = DDI_ERR;  //fifo为空
            break;
        }
    }while(0);

    return ret;
}

s32 ddi_thmprn_esc_init(void)
{
    esc_init();
    return 0;
}

s32 ddi_thmprn_ioctl(u32 nCmd, u32 lParam, u32 wParam)
{
    u32 *print_len_p;
    switch(nCmd)
    {
//    case DDI_THMPRN_CTL_VER:          (0) //获取热敏打印版本
    case DDI_THMPRN_CTL_GRAY:         //(1) //设置打印灰度
        PrtSetSpeed(lParam);
        return DDI_OK;
        
//    case DDI_THMPRN_CTL_LINEGAP:      (2) //设置打印文本行间距
//    case DDI_THMPRN_CTL_COLGAP:       (3) //设置打印文本列间距
    case DDI_THMPRN_CTL_SETBATVOL:    //(4) //设置电池电压
        BatteryVoltageADJ(lParam);
        return DDI_OK;

	case DDI_THMPRN_GET_PRINT_LEN:
        print_len_p = (u32 *)lParam;
        *print_len_p = pt_get_printline_len();
        return DDI_OK;
		
    case DDI_THMPRN_CTL_BLOCK_TEST:
        TPSelfTest_5();
        return DDI_OK;

    case DDI_THMPRN_CTL_GET_PAPER_STATUS:
        return ddi_thmprn_get_status_with_open();
    }
    
    return DDI_EINVAL;
}

#else
s32 ddi_thmprn_open (void)
{
	return DDI_ENODEV;
}


s32 ddi_thmprn_close(void)
{
	return DDI_ENODEV;
}



s32 ddi_thmprn_feed_paper(s32 nPixels)
{
	return DDI_ENODEV;
}

s32 ddi_thmprn_print_image_file(u32 nOrgLeft, u32 nImageWidth, u32 nImageHeight, const u8 *lpImageName)
{
	return DDI_ENODEV;
}

s32 ddi_thmprn_print_image (u32 nOrgLeft, u32 nImageWidth, u32 nImageHeight, const u8 *lpImage)
{
	return DDI_ENODEV;
}


s32 ddi_thmprn_print_comb_text (u32 nNum, const strPrnCombTextCtrl* lpPrnCombTextCtrl[])
{
	return DDI_ENODEV;
}


s32 ddi_thmprn_print_text (strPrnTextCtrl *lpPrnCtrl, const u8 *lpText )
{
	return DDI_ENODEV;
}

s32 ddi_thmprn_get_status (void)
{
	return DDI_ENODEV;
}

s32 ddi_thmprn_ioctl(u32 nCmd, u32 lParam, u32 wParam)
{
	return DDI_ENODEV;
}

#endif

