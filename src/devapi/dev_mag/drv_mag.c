/********************************Copyright ( c ) ********************************
**                  DAQU 
**                         
**                        
**
**Created By:         
** Created Date:     
** Version:        
** Description:    该文件包含磁卡的驱动相关接口
  
****************************************************************************/
#include "devglobal.h"
#include "sdk/mhscpu_sdk.h"
#include "drv_mag.h"



#define MAG_TIMER_ID      5     //磁卡和PSAM卡CLK时钟冲突

static s32 g_mag_fd = -1;
static s32 g_msr_irq = 0;
#define MAG_TK_MAX_NUM    3
#define MAG_OPTIONS     (TRACK_SELECT_1 | TRACK_SELECT_2 | TRACK_SELECT_3)
#define MAG_TIMEROUT    500
#define MAG_vaild_TIMEROUT    200       //防止正确后面跟随错误

static u8 *g_Track_mallocram = NULL;

typedef enum
{
    MAG_STATUS_NONE = 0,
    MAG_STATUS_Err  = 1,
    MAG_STATUS_OK   = 2,
    MAG_STATUS_NULL = 0xAA,
}drv_MAG_STATUS_t;


typedef enum
{
    MAG_STATUS_NOERR       = 0,       //本磁道刷卡没有错,刷卡成功
    MAG_STATUS_NOSTRIPE    = 1,    //本磁道没有任何数据，包括起始位
    MAG_STATUS_NODATA      = 2,      //本磁道没有数据信息体，只有起始位
    MAG_STATUS_LRCERR      = 3,
    MAG_STATUS_PARITYERR   = 4,
    MAG_STATUS_OTHERERR    = 5
    
}drv_MAGTRACK_SWIPESTATUS_t;

typedef struct _str_MAG_PARAM
{
    u32 m_timerid;
    volatile drv_MAG_STATUS_t  m_status;
    track_data *m_tk;
    drv_MAGTRACK_SWIPESTATUS_t m_swipestatus[3];
}str_mag_param_t;

static str_mag_param_t g_mag_param;
void mag_data_clear(void)
{
    if(g_mag_param.m_status != MAG_STATUS_NULL)
    {
        memset(g_mag_param.m_tk, 0, sizeof(track_data)*MAG_TK_MAX_NUM);
    }
}


//sxl add 20180619
void DMA0_IRQHandler(void)
{
	if (DMA_GetITStatus(DMA_Channel_0, DMA_IT_BlockTransferComplete) == SET)
	{
		DMA_ClearITPendingBit(DMA_Channel_0, DMA_IT_BlockTransferComplete);
		MSR_ADIRQHandler();
    }
	
}

/****************************************************************************
**Description:	   打开MAG设备
**Input parameters:	
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:		pengxuebin,20170424
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
#if(MAG_EXIST==1)
void MSR_IRQHandler(void)
{
    g_msr_irq = 1;
	MSR_ClearDpuInpt();
	#if 0
    s32 ret;
    s32 i;
    u8 cfmt = 0, tflag = 0;
    track_data tkdata[MAX_TRACK_NUM];

    MSR_ClearDpuInpt();
    ret = MSR_GetDecodeData(tkdata, &cfmt, &tflag);		
    MAG_DEBUG("\r\nmsr ret:%d\r\n",ret);
	if(ret == DETECT_SWIPING_CARD)
	{
	    MAG_DEBUG("\r\nmsr data:\r\n");
		if(g_mag_param.m_status != MAG_STATUS_NULL)
        {
            g_mag_param.m_status = MAG_STATUS_OK;
            g_mag_param.m_timerid = dev_user_gettimeID();
            for(i=0; i<MAX_TRACK_NUM; i++)
            {
                g_mag_param.m_tk[i].len = tkdata[i].len;
                memcpy(g_mag_param.m_tk[i].buf, tkdata[i].buf, tkdata[i].len);
            }

			
			if(g_mag_param.m_tk[0].len)
			{
				if(tflag&0x01)  // 1成功
				{
					g_mag_param.m_swipestatus[0] = MAG_STATUS_NOERR;
				}
				else
				{
					g_mag_param.m_swipestatus[0] = MAG_STATUS_OTHERERR;
				}
			}

			if(g_mag_param.m_tk[1].len)
			{
				if(tflag&0x02)  // 1成功
				{
					g_mag_param.m_swipestatus[1] = MAG_STATUS_NOERR;
				}
				else
				{
					g_mag_param.m_swipestatus[1] = MAG_STATUS_OTHERERR;
				}
			}

			
			if(g_mag_param.m_tk[2].len)
			{
				if(tflag&0x04)  // 1成功
				{
					g_mag_param.m_swipestatus[2] = MAG_STATUS_NOERR;
				}
				else
				{
					g_mag_param.m_swipestatus[2] = MAG_STATUS_OTHERERR;
				}
			}
			
            g_mag_param.m_timerid = dev_user_gettimeID();
        }
		
		MSR_WaitSwipeCard();
	}
    #endif


	#if 0
    switch(detect_swiping_card())
    {
    case DETECT_SWIPING_CARD:
        cfmt = 0;
        tflag = 0;       
        ret = get_decode_data(tkdata, MAG_OPTIONS, &cfmt, &tflag);
//MAG_DEBUG("mag:ret=%d\r\n"); 
        if(ret == SUCCESS)
        {
            if(g_mag_param.m_status != MAG_STATUS_NULL)
            {
                g_mag_param.m_status = MAG_STATUS_OK;
                g_mag_param.m_timerid = dev_user_gettimeID();
                for(i=0; i<MAX_TRACK_NUM; i++)
                {
                    g_mag_param.m_tk[i].len = tkdata[i].len;
                    memcpy(g_mag_param.m_tk[i].buf, tkdata[i].buf, tkdata[i].len);
                }

				
				if(g_mag_param.m_tk[0].len)
				{
					if(tflag&0x01)  // 1成功
					{
						g_mag_param.m_swipestatus[0] = MAG_STATUS_NOERR;
					}
					else
					{
						g_mag_param.m_swipestatus[0] = MAG_STATUS_OTHERERR;
					}
				}

				if(g_mag_param.m_tk[1].len)
				{
					if(tflag&0x02)  // 1成功
					{
						g_mag_param.m_swipestatus[1] = MAG_STATUS_NOERR;
					}
					else
					{
						g_mag_param.m_swipestatus[1] = MAG_STATUS_OTHERERR;
					}
				}

				
				if(g_mag_param.m_tk[2].len)
				{
					if(tflag&0x04)  // 1成功
					{
						g_mag_param.m_swipestatus[2] = MAG_STATUS_NOERR;
					}
					else
					{
						g_mag_param.m_swipestatus[2] = MAG_STATUS_OTHERERR;
					}
				}
				
                g_mag_param.m_timerid = dev_user_gettimeID();
            }
        }
        else if((ret==INVALID_ADDR)||(ret==PARITY_ERR)||(ret==LENGTH_ERR))
        {
            //if((g_mag_param.m_status!=MAG_STATUS_OK)||dev_user_gettimeID(g_mag_param.m_timerid, MAG_vaild_TIMEROUT))
            if(dev_user_querrytimer(g_mag_param.m_timerid, MAG_vaild_TIMEROUT))
            {
                g_mag_param.m_status = MAG_STATUS_Err;
                g_mag_param.m_timerid = dev_user_gettimeID();
            }
        }
        /*else if(ret == TRACK_NO_DATA)
        {
            
        }*/
        break;
    case DETECT_HARD_WAKEUP:
//MAG_DEBUG("INT:sc_sleep\r\n");   
        //clear_dpu_int();         //请中断
        //sc_sleep();              //??
        MSR_WaitSwipeCard();  //sxl20180608
        break;
    case DETECT_NO_SWIPING_CARD:
    default:
        //clear_dpu_int();
		MSR_WaitSwipeCard(); //sxl20180608
        break;
		
    }
	#endif
	
}
#endif



void TIMER_Configuration(void)
{
	TIM_InitTypeDef TIM_InitStruct;
	TIM_InitStruct.TIM_Period = 1;	//SYSCTRL->PCLK_1MS_VAL;
	
	TIM_InitStruct.TIMx = TIM_5;
	TIM_Init(TIMM0, &TIM_InitStruct);
	TIM_ITConfig(TIMM0, TIM_InitStruct.TIMx, ENABLE);
}
#if 0
void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	NVIC_SetPriorityGrouping(NVIC_PriorityGroup_3);
	
	NVIC_InitStructure.NVIC_IRQChannel = DMA_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = TIM0_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = MSR_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVIC_InitStructure);
}
#endif

/****************************************************************************
**Description:	   MAG设备初始化
**Input parameters:	
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:		pengxuebin,20170424
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
void drv_mag_init(void)
{
    static u8 flg=0;
	str_timer_cfg_t t_cfg;

    if(flg==0)
    {
		flg = 1;
		//open timer 5
		t_cfg.m_tus = 100;
		t_cfg.m_reload = 0; 
		dev_timer_request(MAG_TIMER_ID, t_cfg, NULL, NULL);
		//close timer 5 interrupt
		

		SYSCTRL_AHBPeriphClockCmd(SYSCTLR_AHBPeriph_MSRFC, ENABLE);  //MSR FIFO
		SYSCTRL_AHBPeriphResetCmd(SYSCTLR_AHBPeriph_MSRFC, ENABLE);

		//DMA is enable at initial
		g_Track_mallocram = (u8 *)k_malloc(MAX_TRACK_RAM_BUFF_SIZE);
		MSR_Powerup();
		MSR_Init(1, ADC_BAUDRATE_150K,g_Track_mallocram,MAX_TRACK_RAM_BUFF_SIZE);
		MSR_Powerdown();
		k_free(g_Track_mallocram);
		g_Track_mallocram = NULL;
        //msr下电
        //?SYSCTRL->MSR_CR1 |= BIT27;      //pd_msr: MSR关电使能： 0 ：不关电； 1 ：关电且将电源拉到地
		
		TIMER_Configuration();
		NVIC_DisableIRQ(MSR_IRQn);   
		NVIC_DisableIRQ(TIM0_5_IRQn);
		NVIC_DisableIRQ(DMA_IRQn);
		
	}
	
}
/****************************************************************************
**Description:	   打开MAG设备
**Input parameters:	
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:		pengxuebin,20170424
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
s32 drv_mag_open(void)
{
    u8 i;
    s32 ret;
    track_data tdata[MAX_TRACK_NUM];
	uint8_t cfmt = 0, tflag = 0;
	
    if(g_mag_fd < 0)
    {
        g_msr_irq = 0;
        drv_mag_init();
        g_mag_param.m_tk = (track_data*)k_malloc(sizeof(track_data)*MAG_TK_MAX_NUM);
        if(g_mag_param.m_tk==NULL)
        {
            MAG_DEBUG("MALLOC Err!\r\n");
            return -1;
        }       
        mag_data_clear();

		SYSCTRL_AHBPeriphClockCmd(SYSCTLR_AHBPeriph_MSRFC, ENABLE);
		SYSCTRL_AHBPeriphResetCmd(SYSCTLR_AHBPeriph_MSRFC, ENABLE);
	    //DMA CLK is enable at initial
		
		

        //MSR上电
        //sxl?20180608
        if(g_Track_mallocram == NULL)
        {
        	g_Track_mallocram = (u8 *)k_malloc(MAX_TRACK_RAM_BUFF_SIZE);
        }
		
		if(g_Track_mallocram == NULL)
        {
        	k_free(g_mag_param.m_tk);
        	g_mag_param.m_tk  = NULL;
        	MAG_DEBUG("MALLOC Err!\r\n");
            return -1;
		}

        
		TIMER_Configuration();

		NVIC_ClearPendingIRQ(MSR_IRQn);
        NVIC_EnableIRQ(MSR_IRQn);

		NVIC_ClearPendingIRQ(DMA_IRQn);
        NVIC_EnableIRQ(DMA_IRQn);

		
		NVIC_ClearPendingIRQ(TIM0_5_IRQn);
        NVIC_EnableIRQ(TIM0_5_IRQn);

		MSR_Powerup();
	    MSR_ResetAdc();
	    memset(g_Track_mallocram,0,MAX_TRACK_RAM_BUFF_SIZE);
		MSR_Init(1, ADC_BAUDRATE_150K, g_Track_mallocram, MAX_TRACK_RAM_BUFF_SIZE);
		
        //MAG_DEBUG("\r\nmag open2 \r\n");
        
        g_mag_fd = 0;
        g_mag_param.m_status = MAG_STATUS_NONE;
		for(i = 0;i < 3;i++)
		{
			g_mag_param.m_swipestatus[i] = MAG_STATUS_NOSTRIPE;
		}
        //g_mag_param.m_timeid = 0;
    }
    return g_mag_fd;
}
/****************************************************************************
**Description:	   关闭MAG设备
**Input parameters:	
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:		pengxuebin,20170424
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
s32 drv_mag_close(void)
{
    if(g_mag_fd >= 0)
    {
        NVIC_DisableIRQ(MSR_IRQn);   
		NVIC_DisableIRQ(TIM0_5_IRQn);
		NVIC_DisableIRQ(DMA_IRQn);
		
        //msr下电
        SYSCTRL_AHBPeriphClockCmd(SYSCTLR_AHBPeriph_MSRFC, DISABLE);
        MSR_Powerdown();
        //?SYSCTRL->MSR_CR1 |= BIT27;
        g_mag_fd = -1;
        g_mag_param.m_status = MAG_STATUS_NULL;
        k_free(g_mag_param.m_tk);
        g_mag_param.m_tk  = NULL;
		g_msr_irq = 0;

		if(g_Track_mallocram != NULL)
		{
			k_free(g_Track_mallocram);
			g_Track_mallocram = NULL;
		}
		

		MAG_DEBUG("\r\nmag close \r\n");
		
    }
    return 0;
}
u8 drv_mag_read_irqflg(void)
{
    return g_msr_irq;
}
/****************************************************************************
**Description:	   
**Input parameters:	
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:		pengxuebin,20170424
**--------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
void drv_mag_decode(void)
{
    s32 ret;
    s32 i;
    u8 cfmt=0,tflag = 0;
    track_data tkdata[MAX_TRACK_NUM];

    
    if(g_mag_fd < 0)
    {
        return;
    }
	if(g_msr_irq)
	{
		ret = MSR_GetDecodeData(tkdata, &cfmt, &tflag);
		if(ret == DETECT_HAVE_SWIPING)
		{
		    g_msr_irq = 0;
		    MAG_DEBUG("\r\nmsr data:%02x %02x\r\n",tflag,g_mag_param.m_status);
			if(g_mag_param.m_status != MAG_STATUS_NULL)
	        {
	            g_mag_param.m_status = MAG_STATUS_OK;
	            g_mag_param.m_timerid = dev_user_gettimeID();
	            for(i=0; i<MAX_TRACK_NUM; i++)
	            {
	                g_mag_param.m_tk[i].len = tkdata[i].len;
	                memcpy(g_mag_param.m_tk[i].buf, tkdata[i].buf, tkdata[i].len);
	            }

				
				if(g_mag_param.m_tk[0].len) 
				{
					if(tflag&0x02)  // 2成功
					{
						g_mag_param.m_swipestatus[0] = MAG_STATUS_NOERR;
					}
					else
					{
						g_mag_param.m_swipestatus[0] = MAG_STATUS_OTHERERR;
						g_mag_param.m_status = MAG_STATUS_NONE;
						for(i = 0;i < 3;i++)
						{
							g_mag_param.m_swipestatus[i] = MAG_STATUS_NOSTRIPE;
						}
        				mag_data_clear();
						//return ;//DEVSTATUS_ERR_FAIL;
					}
					
				}
				else
				{
					if(tflag&0x02)
					{
						g_mag_param.m_swipestatus[0] = MAG_STATUS_OTHERERR;
						g_mag_param.m_status = MAG_STATUS_NONE;
						for(i = 0;i < 3;i++)
						{
							g_mag_param.m_swipestatus[i] = MAG_STATUS_NOSTRIPE;
						}
        				mag_data_clear();
						//return ;//DEVSTATUS_ERR_FAIL;
					}
				}
				

				if(g_mag_param.m_tk[1].len) //涓夌閬?
				{
					if(tflag&0x04)  // 1成功
					{
						g_mag_param.m_swipestatus[1] = MAG_STATUS_NOERR;
					}
					else
					{
						g_mag_param.m_swipestatus[1] = MAG_STATUS_OTHERERR;
					}
				}

				#if 0
				if(g_mag_param.m_tk[2].len)
				{
					if(tflag&0x04)  // 1成功
					{
						g_mag_param.m_swipestatus[2] = MAG_STATUS_NOERR;
					}
					else
					{
						g_mag_param.m_swipestatus[2] = MAG_STATUS_OTHERERR;
						g_mag_param.m_tk[2].len = 0;
					}
				}
				#endif
	            g_mag_param.m_timerid = dev_user_gettimeID(); 
              #if 0
                for(i=0; i<MAX_TRACK_NUM; i++)
                {
                    MAG_DEBUG("tk%d.len=%d,%s\r\n", i+2, g_mag_param.m_tk[i].len, g_mag_param.m_tk[i].buf);
                }
              #endif
	        }
			MSR_WaitSwipeCard();
		}
	}

}
/****************************************************************************
**Description:	   
**Input parameters:	
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:		pengxuebin,20170424
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
s32 drv_mag_read(u8 *tk1, u8 *tk2, u8 *tk3)
{
	s32 ret;
	s32 i;
	u8 cfmt = 0, tflag = 0;
	track_data tkdata[MAX_TRACK_NUM];
    
    if(g_mag_fd < 0)
    {
        return DEVSTATUS_ERR_DEVICE_NOTOPEN;
    }

//    drv_mag_decode();
    if(g_mag_param.m_status==MAG_STATUS_OK)
    {
        if(0 == (dev_user_querrytimer(g_mag_param.m_timerid, MAG_TIMEROUT)))  //sxl?2017
        {
            #if 0
            for(i=0; i<MAX_TRACK_NUM; i++)
            {
                MAG_DEBUG("tk%d.len=%d,%s\r\n", i+2, g_mag_param.m_tk[i].len, g_mag_param.m_tk[i].buf);
                MAG_DEBUGHEX(NULL, g_mag_param.m_tk[i].buf, g_mag_param.m_tk[i].len);
            }
			#endif
			
            for(i=0; i<MAX_TRACK_NUM; i++)
            {
                if((i==0)&&(tk2!= NULL)&&(g_mag_param.m_tk[i].len>=2))
                {
                    memcpy(tk2, &g_mag_param.m_tk[i].buf[1], g_mag_param.m_tk[i].len-2);
                    tk2[g_mag_param.m_tk[i].len-2] = 0;
                }
                else if((i==1)&&(tk3 !=NULL)&&(g_mag_param.m_tk[i].len>=2))
                {
                    memcpy(tk3, &g_mag_param.m_tk[i].buf[1], g_mag_param.m_tk[i].len-2);
                    tk3[g_mag_param.m_tk[i].len-2] = 0;
                }
				#if 0
                else if(i==2)
                {
                    memcpy(tk3, g_mag_param.m_tk[i].buf, g_mag_param.m_tk[i].len);
                    tk3[g_mag_param.m_tk[i].len] = 0;
                }
				#endif
            }
        }
        g_mag_param.m_status = MAG_STATUS_NONE;
        for(i = 0;i < 3;i++)
        {
            g_mag_param.m_swipestatus[i] = MAG_STATUS_NOSTRIPE;
        }
//        mag_data_clear();
        return DEVSTATUS_SUCCESS;
    }
    else if(g_mag_param.m_status==MAG_STATUS_Err)
    {
        g_mag_param.m_status = MAG_STATUS_NONE;
		for(i = 0;i < 3;i++)
		{
			g_mag_param.m_swipestatus[i] = MAG_STATUS_NOSTRIPE;
		}
        return DEVSTATUS_ERR_FAIL;
    }
    else
    {
        tk1[0] = 0;
        tk2[0] = 0;
        tk3[0] = 0;
        return DEVSTATUS_SUCCESS;
    }
}
/****************************************************************************
**Description:	   关闭MAG设备
**Input parameters:	
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_DEVICE_NOTOPEN:   设备未打开
**Created by:		pengxuebin,20170424
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
s32 drv_mag_clear(void)
{
    u8 i;
	
    if(g_mag_fd < 0)
    {
        return DEVSTATUS_ERR_DEVICE_NOTOPEN;
    } 
    g_mag_param.m_status = MAG_STATUS_NONE;
	for(i = 0;i < 3;i++)
	{
		g_mag_param.m_swipestatus[i] = MAG_STATUS_NOSTRIPE; //未刷卡
	}
    mag_data_clear();
    return 0;
}



s32 drv_mag_ioctl_getmagswipestatus(void)
{
    s32 status;
    s32 i;

//    drv_mag_decode();
    if(g_mag_param.m_status==MAG_STATUS_OK)
    {
        if(0 != (dev_user_querrytimer(g_mag_param.m_timerid, MAG_TIMEROUT)))  //sxl?2017
        {
            g_mag_param.m_status = MAG_STATUS_NONE;
            for(i = 0;i < 3;i++)
            {
                g_mag_param.m_swipestatus[i] = MAG_STATUS_NOSTRIPE;
            }
        }
    }

//    MAG_DEBUG("mag status  = %02x %02x %02x\r\n",g_mag_param.m_swipestatus[0],g_mag_param.m_swipestatus[1],g_mag_param.m_swipestatus[2]);
	status = g_mag_param.m_swipestatus[2];
	status = (status<<8) + g_mag_param.m_swipestatus[1];
	status = (status<<8) + g_mag_param.m_swipestatus[0];
	
	return status;
}


s32 dev_mag_getmsrstatus(void)
{
	return g_msr_irq;
}
s32 drv_mag_get_ver(u8 *ver)
{
    u32 i;

    i = MSR_GetVersion();
    sprintf(ver, "%08X", i);
    return 0;
}

#if 1
void testmag(void)
{

    u8 tk1[255],tk2[255],tk3[255];
	u32 rkey;
	
    dev_debug_printf("\r\ntest mag:");

    memset(tk1,0,sizeof(tk1));
	memset(tk2,0,sizeof(tk2));
	memset(tk3,0,sizeof(tk3));

	//drv_mag_close(); 
	while(1)
	{
	    #if 1
		if(drv_mag_open() == 0)
		{
			if(drv_mag_read(tk1,tk2,tk3) == DEVSTATUS_SUCCESS)
			{
			    if(tk1[0]||tk2[0]||tk3[0])
			    {
				    if(tk1[0])
				    {
						dev_debug_printf("\r\n track1:%s\r\n",tk1);
				    }
					if(tk2[0])
				    {
						dev_debug_printf("\r\n track2:%s\r\n",tk2);
				    }
					if(tk3[0])
				    {
						dev_debug_printf("\r\n track3:%s\r\n",tk3);
				    }
				    memset(tk1,0,sizeof(tk1));
					memset(tk2,0,sizeof(tk2));
					memset(tk3,0,sizeof(tk3));
					drv_mag_close();

					//dev_keypad_close();//sxl?
					//dev_keypad_open();
			    }
			}
			else 
			{
			    memset(tk1,0,sizeof(tk1));
				memset(tk2,0,sizeof(tk2));
				memset(tk3,0,sizeof(tk3));
				drv_mag_close();
			}
		}
        #endif

		if(dev_keypad_read(&rkey))
		{
			dev_debug_printf("\r\nkey = %08x\r\n",rkey);
		}
		
	}
	
}

#endif

