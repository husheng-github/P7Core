

#include "devglobal.h"
#include "mhscpu_bpk.h"
#include "mhscpu_sensor.h"
#include "mhscpu_ssc.h"

#include "mhscpu.h"

#define DRYICE_DISABLE

#ifdef DRYICE_DISABLE
s32 pcitamper_manage_dryice_isrhandler(u32 dryice)
{
}
void pcitamper_manage_tamperset(u8 type)
{
}
void pcitampermanage_disable(void)
{
}
u32 pcitampermanage_getsavedsenexttype(void)
{
}
void pcitampermanage_recover_dryicereg(void)
{
}
void pcitampermanage_savesenexttype(u32 senexttyperegvalue)
{
}
void pcitampermanage_savetempdryicereg(u8 *dryicereg)
{
}
#endif

extern u32 pcitampermanage_getsavedsenexttype(void);
extern void pcitampermanage_savesenexttype(u32 senexttyperegvalue);
extern void pcitampermanage_recover_dryicereg(void);
extern void pcitampermanage_savetempdryicereg(u8 *dryicereg);
extern void pcitampermanage_disable(void);


void SENSOR_IRQHandler(void)
{

    u32 drysrreg;
    
    //先不处理寄存器值
    drysrreg = SENSOR_GetITStatusReg();
    pcitamper_manage_dryice_isrhandler(drysrreg);

    NVIC_DisableIRQ(SSC_IRQn);
    NVIC_DisableIRQ(SENSOR_IRQn);
    NVIC_ClearPendingIRQ(SENSOR_IRQn);
    
}



void SSC_IRQHandler(void)
{
    //先关中断
    NVIC_DisableIRQ(SSC_IRQn);
    NVIC_DisableIRQ(SENSOR_IRQn);
    
    SSC_ClearITPendingBit(SSC_ITSysXTAL12M|SSC_ITSysGlitch|SSC_ITSysVolHigh|SSC_ITSysVolLow);
    
    
	
	NVIC_ClearPendingIRQ(SSC_IRQn);
	
}

void dev_dryicd_task(void)
{
    
    int32_t cnt = 0;
	int32_t reg;
	
	
    if((SENSOR->SEN_BRIDGE&0x02) == 0)
    {
    	SENSOR->SEN_BRIDGE |= 0x01;
		while((SENSOR->SEN_BRIDGE&0x02) == 0)
		{
			cnt++;
			if(cnt >= 2000)
			{
				return;
			}
		}
    }
	
	
	reg = SENSOR->SEN_STATE;
    SENSOR->SEN_BRIDGE &= (~0x01);  //clear rd_start
	
    pcitamper_manage_dryice_isrhandler(reg);
dev_debug_printf("ret=%8X\r\n", reg);    
}

void dev_dryice_interruptopen(void)
{
  #if 0  
	NVIC_InitTypeDef NVIC_InitStructure;
    
	NVIC_SetPriorityGrouping(NVIC_PriorityGroup_3);
	
	NVIC_InitStructure.NVIC_IRQChannel = SENSOR_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	
	NVIC_InitStructure.NVIC_IRQChannel = SSC_IRQn;
	NVIC_Init(&NVIC_InitStructure);
  #endif
//    SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_BPU, ENABLE);
//	SYSCTRL_APBPeriphResetCmd(SYSCTRL_APBPeriph_BPU, ENABLE);
	
//SENSOR->SEN_EXT_CFG |= (1<<15);
//SENSOR->SEN_SOFT_EN = 1;
    NVIC_EnableIRQ(SENSOR_IRQn);  
    //NVIC_EnableIRQ(SSC_IRQn);  
//SSC_SENSORAttackRespMode(SSC_SENSOR_Interrupt);
    
//dev_debug_printf("%s(%d):EXT_CFG=%08X,SOFT_CFG=%08X\r\n", __FUNCTION__, __LINE__,
//            SENSOR->SEN_EXT_CFG, SENSOR->SEN_SOFT_EN);  
//dev_debug_printf("EXT_START=%08X\r\n", SENSOR->SEN_EXTS_START);  

}


void dev_dryice_poweron(void)
{
    SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_BPU,ENABLE);  //先开电池
}


void dev_dryice_reset(void)
{
    SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_BPU,ENABLE); 
	SYSCTRL_APBPeriphResetCmd(SYSCTRL_APBPeriph_BPU, ENABLE);
}

//动态模式只支持 模式0
s32 dev_dryice_open (DRYICETAMPERCONFIG *dryicetamperconfig,u8 type)
{
    
    u8 i;
    u32 senexttype;
    u32 senextcfg;
    u32 SEN_EN[19];
    u8 exttamperenableflag[12];
    u8 exttamperconfig[12];
    u8 modifyflag = 0;
    u32 waitcnt = 0;
    u32 status;
    u32 saveedsen_exttype_reg;
    u8 dryicereg[32];
    u8 tamperdisableflag = 1;
    //open clock

    memset(exttamperenableflag,0,sizeof(exttamperenableflag));
    memset(exttamperconfig,0,sizeof(exttamperconfig));
    
    exttamperenableflag[0] = dryicetamperconfig->dryice_tamperpin0;
    exttamperenableflag[1] = dryicetamperconfig->dryice_tamperpin1;
    exttamperenableflag[2] = dryicetamperconfig->dryice_tamperpin2;
    exttamperenableflag[3] = dryicetamperconfig->dryice_tamperpin3;
    exttamperenableflag[4] = dryicetamperconfig->dryice_tamperpin4;
    exttamperenableflag[5] = dryicetamperconfig->dryice_tamperpin5;
    exttamperenableflag[6] = dryicetamperconfig->dryice_tamperpin6;
    exttamperenableflag[7] = dryicetamperconfig->dryice_tamperpin7;
    
    exttamperconfig[0] = dryicetamperconfig->dryice_tamperpin0_config;
    exttamperconfig[1] = dryicetamperconfig->dryice_tamperpin1_config;
    exttamperconfig[2] = dryicetamperconfig->dryice_tamperpin2_config;
    exttamperconfig[3] = dryicetamperconfig->dryice_tamperpin3_config;
    exttamperconfig[4] = dryicetamperconfig->dryice_tamperpin4_config;
    exttamperconfig[5] = dryicetamperconfig->dryice_tamperpin5_config;
    exttamperconfig[6] = dryicetamperconfig->dryice_tamperpin6_config;
    exttamperconfig[7] = dryicetamperconfig->dryice_tamperpin7_config;
    
    senextcfg = 0;
    senexttype = 0;
    for(i = 0;i < 19;i++)
    {
        SEN_EN[i] = 0;
    }
    
    //freq 先31.25ms 一次测试，后改成1s一次
	
    
	//脉冲上拉模式关闭

	

    
    
    //上拉采用间隔上拉
    // 1. 使能Tamper在检测到攻击时上拉
//    senextcfg |= (1<<26);
	senextcfg |= (1<<24);//(1<<24);  // 1S 输出一次
    senextcfg |= (3<<22); 
	senextcfg |= (3<<20); 
	//senextcfg |= (1<<19); //不能间隙上拉

	senextcfg |= (1<<18); 
	senextcfg |= (3<<16); 

	senextcfg |= (1<<15);  //触发后产生中断

	//动态传感器/Mesh的输出频率取中值 500ms一次
    senextcfg |=(0x02<<12);
		
	
    for(i = 0;i < 8;i++)
    {
        
    	if(exttamperenableflag[i])  //ext tamper enable
    	{
    		SEN_EN[SENSOR_EN_EXT0+i] = SEN_ENABLE;
    	    // dynamic
        if(exttamperconfig[i] == EXTTAMPER_DYNAMIC_OUT||exttamperconfig[i] == EXTTAMPER_DYNAMIC_IN) //动态
        {
            
            //配置为动态传感器模式0
            senexttype &= (~(0x03<<((i>>1)<<1)));
            senexttype |=  (0x01<<((i>>1)<<1));

				//不上拉也不下拉
				//初始化值是0，不用做处理
				
    		}
			else    //static
			{
			    //两个位清0
				senexttype &= (~(0x03<<((i>>1)<<1)));
				//senexttype |=  (0x01<<(i+12));  //使能拉电阻
				if(exttamperconfig[i] == EXTTAMPER_STATIC_HIGH) 
				{
				    //非正常电平为1
					
					//senextcfg   |=  (0x01<<i);
					
				}
				else
				{
					//非正常电平为0
					
				}
				
			}
			
    	}
		else
		{
		    senexttype &= (~(0x03<<((i>>1)<<1)));  //默认是静态管脚，拉电阻开启
			senexttype |=  (0x01<<(i+12));  //使能拉电阻
			SEN_EN[SENSOR_EN_EXT0+i] = SEN_DISABLE;
			
		}
		
    }
    
    
    if(dryicetamperconfig->dryice_tampervte)
    {
        SEN_EN[SENSOR_EN_VH] = SEN_ENABLE;
        SEN_EN[SENSOR_EN_VL] = SEN_ENABLE;
    }
    else
    {
        SEN_EN[SENSOR_EN_VH] = SEN_DISABLE;
        SEN_EN[SENSOR_EN_VL] = SEN_DISABLE;
    }
    
    
    if(dryicetamperconfig->dryice_tampertte)
    {
        SEN_EN[SENSOR_EN_TH] = SEN_ENABLE;
        SEN_EN[SENSOR_EN_TL] = SEN_ENABLE;
    }
    else
    {
        SEN_EN[SENSOR_EN_TH] = SEN_DISABLE;
        SEN_EN[SENSOR_EN_TL] = SEN_DISABLE;
    }

    
    if(dryicetamperconfig->dryice_tamperxtal32k)
    {
        SEN_EN[SENSOR_EN_XTAL32] = SEN_ENABLE;
    }
    else
    {
        SEN_EN[SENSOR_EN_XTAL32] = SEN_DISABLE;
    }

    
    if(dryicetamperconfig->dryice_tampermesh)
    {
        SEN_EN[SENSOR_EN_MESH] = SEN_ENABLE;
    }
    else
    {
        SEN_EN[SENSOR_EN_MESH] = SEN_DISABLE;
    }

    
    if(dryicetamperconfig->dryice_tampervgdetected)
    {
        //SENSOR->SEN_VG_DETECT |= 0x03;    //16 ms的毛刺这次设置不进去，只能在后面设置
        SEN_EN[SENSOR_EN_VOLGLITCH] = SEN_ENABLE;
    }
    else
    {
        SEN_EN[SENSOR_EN_VOLGLITCH] = SEN_DISABLE;
    }


    saveedsen_exttype_reg = pcitampermanage_getsavedsenexttype();
    
    
    
    //判断配置是否已经修改，是否需要重新配置
    #ifdef DRYICE_DEBUG
    dev_debug_printf("\r\nsenexttype = %08x\r\n",senexttype);
    dev_debug_printf("\r\nsenextcfg = %08x %08x\r\n",senextcfg,SENSOR->SEN_EXT_CFG);
    for(i = 0;i < 19;i++)
    {
        dev_debug_printf("%08x ",SEN_EN[i]);
    }
    dev_debug_printf("SEN_EN2:\r\n");
    for(i = 0;i < 19;i++)
    {
        dev_debug_printf("%08x ",SENSOR->SEN_EN[i]);
    }
    dev_debug_printf("\r\n");
    #endif

    
    if(senexttype != saveedsen_exttype_reg)
    {
        #ifdef DRYICE_DEBUG
        dev_debug_printf("\r\nsenexttype modify:%08x %08x\r\n",senexttype,saveedsen_exttype_reg);
        #endif
        modifyflag = 1;
    }
    else if(senextcfg != SENSOR->SEN_EXT_CFG)
    {
        #ifdef DRYICE_DEBUG
        dev_debug_printf("\r\nsenextcfg modify:%08x %08x\r\n",senextcfg,SENSOR->SEN_EXT_CFG);
        #endif
        modifyflag = 1;
    }
    else
    {
    	
    	for(i = 0;i < 8;i++)
    	{
    		if(SEN_EN[i] != SENSOR->SEN_EN[i])
    		{
    		    #ifdef DRYICE_DEBUG
				dev_debug_printf("\r\nSENSOR %d modify:%08x %08x\r\n",i,SEN_EN[i],SENSOR->SEN_EN[i]);
				#endif
    			modifyflag = 1;
				break;
    		}
    	}
		
		
		if(modifyflag == 0)
		{
			for(i = 12;i < 19;i++)
	    	{
	    		if(SEN_EN[i] != SENSOR->SEN_EN[i])
	    		{
	    		    #ifdef DRYICE_DEBUG
					dev_debug_printf("\r\nSENSOR %d modify:%08x %08x\r\n",i,SEN_EN[i],SENSOR->SEN_EN[i]);
					#endif
	    			modifyflag = 1;
					break;
	    		}
	    	}
		}
		
		
    }
    
    
    #ifdef DRYICE_DEBUG
    dev_debug_printf("\r\nmodifyflag = %d\r\n",modifyflag);
    #endif
    
    pcitampermanage_recover_dryicereg();
    
    //modifyflag||
    if((SENSOR_EXTIsRuning() == RESET)||modifyflag||type == DRYICE_SET_QUERY) //||type == DRYICE_SET_POWERON
    {
        
        //BPK->BPK_RR |= 1;
        //while (BPK->BPK_RR);
        //先关中断
        NVIC_DisableIRQ(SENSOR_IRQn);
        NVIC_DisableIRQ(SSC_IRQn);
        
        
        //清触发中断
        SSC_ClearITPendingBit(SSC_ITSysXTAL12M|SSC_ITSysGlitch|SSC_ITSysVolHigh|SSC_ITSysVolLow);
        NVIC_ClearPendingIRQ(SENSOR_IRQn);
        NVIC_ClearPendingIRQ(SSC_IRQn);
        
        
        //先读取dryice寄存器的值
        #ifdef DRYICE_DEBUG
        dev_debug_printf("\r\nreset read dryice key");
        #endif

		if(type != DRYICE_SET_QUERY&&(dev_dryice_waitBPKREADY() == 0)) //为2时会一直查询是否触发状态
		{
			dev_dryice_readsecurekey(dryicereg);
			
			pcitampermanage_savetempdryicereg(dryicereg);
		}
        #ifdef DRYICE_DEBUG
        dev_debug_printf("\r\nreset read dryice key1");
        #endif
        

		//BPU重启  //sxl? 2017 没有起作用
		SYSCTRL_APBPeriphResetCmd(SYSCTRL_APBPeriph_BPU, ENABLE);
        SYSCTRL_APBPeriphResetCmd(SYSCTRL_APBPeriph_BPU, RESET);
		if(SENSOR_EXTIsRuning() == SET)  //sxl?2017在什么情况下会DISABLE不了
		{
			#ifdef DRYICE_DEBUG
            dev_debug_printf("\r\ndryice is runing!");
            #endif
            //stop dryice first
            SENSOR->SEN_EXTS_START = SEN_DISABLE;
        }
        else
        {
            #ifdef DRYICE_DEBUG
            dev_debug_printf("\r\ndryice is stop!");
            #endif
        }

        waitcnt = 0;
        //wait dryice stop
        //exts_start启动/关闭Tamper后，需等待3个32K时钟，该状态位才会更新
        while(SENSOR_EXTIsRuning() == SET)
        {
        	
        	waitcnt++;
			
			if(waitcnt >= 100000)
			{
				#ifdef DRYICE_DEBUG
				dev_debug_printf("\r\ndryice reset1,need hardware reset\r\n");
				#endif
				pcitampermanage_disable();  //所有触发不开启
				break;
			}
        }
        
        SENSOR->SEN_EXT_TYPE = senexttype;
        SENSOR->SEN_EXT_CFG  = senextcfg;
		
		for(i = 0;i < 8;i++)
		{
			SENSOR->SEN_EN[i] = SEN_EN[i];
		}

		for(i = 12;i < 19;i++)
		{
			SENSOR->SEN_EN[i] = SEN_EN[i];
		}
		
		
        //如果开启了电压毛刺检测，需要设置毛刺检测间隔
        if(SEN_EN[SENSOR_EN_VOLGLITCH] == SEN_ENABLE)
        {
            SENSOR->SEN_VG_DETECT = 0x03;
        }
        
        //先清state
        //dev_dryice_gettamperstatusonly();
        //SENSOR->SEN_STATE = 0;
        #ifdef DRYICE_DEBUG
        //dev_debug_printf("\r\n SENSOR->SEN_EXT_TYPE = %08x\r\n",SENSOR->SEN_EXT_TYPE);
        #endif

        //初始化过，保存初始化时的配置
        pcitampermanage_savesenexttype(senexttype);

        //将状态先读走
        SENSOR_ClearITPendingBit();
        tamperdisableflag = 1;

        for(i = 0;i < 19;i++)
        {
            if(SEN_EN[i] == SEN_ENABLE)
            {
                tamperdisableflag = 0;
                break;
            }
        }
        
        //
        //start dryice first
        //有触发开启的时候才启动
		if(tamperdisableflag == 0)
		{
			SENSOR->SEN_EXTS_START = SEN_ENABLE;
			waitcnt = 0;
			while(SENSOR_EXTIsRuning() == RESET)
	        {
	        	
	        	waitcnt++;
				if(waitcnt >= 100000)
				{
					#ifdef DRYICE_DEBUG
					dev_debug_printf("\r\ndryice reset2,need hardware reset\r\n");
					#endif
					pcitampermanage_disable();  //所有触发不开启
					break;
				}
				
	        }
		}
		else
		{
			
			#ifdef DRYICE_DEBUG
            dev_debug_printf("\r\ntamper dected stop\r\n");
            #endif
            SENSOR->SEN_EXTS_START = SEN_DISABLE;
            waitcnt = 0;
            while(1)
            {
                waitcnt++;
                if(waitcnt >= 1000)
                {
                    break;
                }
                
            }
			
		}
		
		if(type != DRYICE_SET_QUERY)
		{
			pcitampermanage_recover_dryicereg();
		}
		#ifdef DRYICE_DEBUG
    	dev_debug_printf("\r\ndryice initial complete!");
		#endif
		
    }
    else
    {
        
        #ifdef DRYICE_DEBUG
        dev_debug_printf("\r\ndryice initialed!");
        #endif
        
    }

    //config interrupt ,open interrupt after power on
    if(type == DRYICE_SET_NORMAL)
    {
        dev_dryice_interruptopen();
    }
    
    
}


s32 dev_dryice_writesecurekey(u8 *securekey,u32 securekeylen)
{
	u8 readskrdata[32];
	u32 buf[16];
    u8 i,j;
    s32 ret = DEVSTATUS_ERR_FAIL;
    u32 cnt = 0;
    ErrorStatus status;
    
    if(securekeylen != 32)
    {
        #ifdef DRYICE_DEBUG
        dev_debug_printf("\r\n %s securekeylen %d(!=32) error \r\n",__FUNCTION__,securekeylen);
        #endif
        return DEVSTATUS_ERR_PARAM_ERR;
    }

    BPK_KeyWriteLock(BPK_KEY_REGION_0 | BPK_KEY_REGION_1 , DISABLE);
	BPK_KeyReadLock(BPK_KEY_REGION_0 | BPK_KEY_REGION_1 , DISABLE);

    memset(buf,0,sizeof(buf));
    for(j = 0;j < 8;j++)
    {
        for(i = 0;i < 4;i++)
        {
            buf[j] = (buf[j]<<8) + securekey[(j<<2) + i];
        }
    }

	status = dev_dryice_waitBPKREADY();
	if(status != 0)
	{
		return DEVSTATUS_ERR_FAIL;
	}
	
    
    status = BPK_WriteKey(buf,sizeof(buf)/sizeof(buf[0]),0);
    if(status != SUCCESS)
    {
        
        #ifdef DRYICE_DEBUG
        dev_debug_printf("\r\nwrite BPK status error\r\n",__FUNCTION__);
        #endif
        return DEVSTATUS_ERR_FAIL; //错误不用加锁，直接返回错误
        
    }
    
    
    //check if secure key write success
    memset(readskrdata,0,sizeof(readskrdata));
    memset(buf,0,sizeof(buf));
    status = BPK_ReadKey(buf,sizeof(buf)/sizeof(buf[0]),0);
    if(status != SUCCESS)
    {
        
        #ifdef DRYICE_DEBUG
        dev_debug_printf("\r\nread BPK status error\r\n",__FUNCTION__);
        #endif
        return DEVSTATUS_ERR_FAIL; //错误不用加锁，直接返回错误
        
    }
    for(j = 0;j < 8;j++)
    {
        
        readskrdata[(j<<2)] = (u8)(buf[j]>>24);
        readskrdata[(j<<2)+1] = (u8)(buf[j]>>16);
        readskrdata[(j<<2)+2] = (u8)(buf[j]>>8);
        readskrdata[(j<<2)+3] = (u8)(buf[j]);
        
    }
    
    if(memcmp(readskrdata,securekey,32))
    {
        #ifdef DRYICE_DEBUG
        dev_debug_printf("\r\n %s securekey write fail\r\n",__FUNCTION__);
        #endif
        memset(readskrdata,0,sizeof(readskrdata));
        ret = DEVSTATUS_ERR_FAIL;
    }
    else
    {
        #ifdef DRYICE_DEBUG
        dev_debug_printf("\r\n %s securekey write success\r\n",__FUNCTION__);
        for(ret = 0;ret < 32;ret++)
        {
            dev_debug_printf("%02x ",readskrdata[ret]);
        }
        dev_debug_printf("\r\n");
        #endif
        ret = DEVSTATUS_SUCCESS;
    }
    memset(readskrdata,0,sizeof(readskrdata));
    
	BPK_KeyWriteLock(BPK_KEY_REGION_0 | BPK_KEY_REGION_1, ENABLE);
	BPK_KeyReadLock(BPK_KEY_REGION_0 | BPK_KEY_REGION_1, ENABLE);
	
	return ret;
	
}


s32 dev_dryice_waitBPKREADY(void)
{
    u32 cnt = 0;
	
	while(BPK_IsReady() == RESET)
	{
		cnt++;
		if(cnt > 20000)
		{
			return 1;
		}
		
	}

	return 0;
	
}


s32 dev_dryice_readsecurekey(u8 *securekey)
{
	u32 buf[16];
    u8  j;
	s32 status;
	
	
	memset(buf,0,sizeof(buf));
	BPK_KeyReadLock(BPK_KEY_REGION_0 | BPK_KEY_REGION_1, DISABLE);
    //ready 之后才能读
    
	dev_dryice_waitBPKREADY();
	status = dev_dryice_waitBPKREADY();
	if(status != 0)
	{
		return DEVSTATUS_ERR_FAIL;
	}
	
	BPK_ReadKey(buf,sizeof(buf)/sizeof(buf[0]),0);
    BPK_KeyReadLock(BPK_KEY_REGION_0 | BPK_KEY_REGION_1, ENABLE);
	for(j = 0;j < 8;j++)
	{
		
		securekey[(j<<2)] =    (u8)(buf[j]>>24);
		securekey[(j<<2)+1] = (u8)(buf[j]>>16);
		securekey[(j<<2)+2] = (u8)(buf[j]>>8);
		securekey[(j<<2)+3] = (u8)(buf[j]);
		
	}

    #ifdef DRYICE_DEBUG
    dev_debug_printf("\r\n %s securekey read success\r\n",__FUNCTION__);
    for(j = 0;j < 32;j++)
    {
        dev_debug_printf("%02x ",securekey[j]);
    }
    dev_debug_printf("\r\n");
    #endif
    
    
    return DEVSTATUS_SUCCESS;
    
}

u32 dev_dryice_gettamperstatusonly(void)
{
    
    //暂时未使用
    #if 0
    u32 reg;
    //需要重新配置才能获取到当前触发状态
    //先stop再run  试一下
    #ifdef DRYICE_DEBUG
    dev_debug_printf("\r\ndev_dryice_gettamperstatusonly\r\n");
    #endif
    SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_BPU,DISABLE);
    dev_user_delay_ms(5);
    SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_BPU,ENABLE);
    pcitamper_manage_tamperset(DRYICE_SET_QUERY);
    //dev_user_delay_ms(100);
    SENSOR_ClearITPendingBit();
    reg = SENSOR_GetITStatusReg_Spec();  //读两次
    #ifdef DRYICE_DEBUG
    dev_debug_printf("\r\ndev_dryice_gettamperstatusonly reg = %08x\r\n",reg);
    #endif
    return SENSOR_GetITStatusReg_Spec();
    #endif

    return 0;
    
}



u32 dev_dryice_poweronreadstatus(void)
{
    //开机的时候配置寄存器，看是否存在触发
    pcitamper_manage_tamperset(DRYICE_SET_POWERON);
    return SENSOR_GetITStatusReg();
    
}

//检查dryice模块是否重新上电或收到过攻击
s32 dev_dryice_checkresetstatus(void)
{
    
    #ifdef DRYICE_DEBUG
    dev_debug_printf("\r\nBPK->BPK_RDY = %08x\r\n",BPK->BPK_RDY);
    #endif
    
    if(BPK->BPK_RDY & 0x02)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
//用于设置重启标志, 使用bpk最后1Word,对应锁区域1
//0: 冷复位      1:reset   2:watchdog复位
#define BPK_RST_FLAG    0x4567BA98
#define BPK_WDTRST_FLAG 0x1234EDCB
#define BPK_HARD_FAULT_RST_FLAG  0x3234EDCB
#if 1
#define BPK_RST_OFFSET  15          //1903S
#else
#defien BPK_RST_OFFSET  15          //1902/1902S
#endif
void SENSOR_DisableAll(void)
{
    static u8 flg=1;

    if(flg)
    {
        flg = 0;
	SENSOR_EXTCmd(DISABLE);
	while (RESET != SENSOR_EXTIsRuning());
	SENSOR_ClearITPendingBit();
	
	SENSOR_ANACmd(SENSOR_ANA_VOL_HIGH | SENSOR_ANA_VOL_LOW | SENSOR_ANA_TEMPER_HIGH | SENSOR_ANA_TEMPER_LOW
	              | SENSOR_ANA_XTAL32K | SENSOR_ANA_MESH | SENSOR_ANA_VOLGLITCH, DISABLE);

    }
}  
//获取模式
s32 dev_dryice_rstmod_get(void)
{
    s32 ret;
    u32 buf[1];
    
    SENSOR_DisableAll();
    BPK_KeyReadLock(BPK_KEY_REGION_1 , DISABLE);
//    BPK_KeyWriteLock(BPK_KEY_REGION_0|BPK_KEY_REGION_1 , DISABLE);
    ret = dev_dryice_waitBPKREADY();
    if(ret != 0)
    {
        return DEVSTATUS_ERR_FAIL;
    }
    ret = BPK_ReadKey(buf, 1, BPK_RST_OFFSET);
	BPK_KeyReadLock(BPK_KEY_REGION_1, ENABLE);
//    BPK_KeyWriteLock(BPK_KEY_REGION_0|BPK_KEY_REGION_1 , ENABLE);
    if(ret != SUCCESS)
    {
        return DEVSTATUS_ERR_FAIL;
    }
    if(buf[0] == BPK_RST_FLAG)
    {
        ret = ST_REBOOT;
    }
    else if(buf[0] == BPK_WDTRST_FLAG)
    {
        ret = ST_WATCHDOG;
    }
    else if(buf[0] == BPK_HARD_FAULT_RST_FLAG)
    {
        ret = ST_HARD_FAULT;
    }
    else
    {
        ret = 0;
    }
    return ret;
}
s32 dev_dryice_rstmod_set(SYSTEM_RESET_TYPE_E st_type)
{
	s32 status;
    u32 buf[1];
    s32 ret;

    if(st_type == ST_REBOOT)
    {
        //rst
        buf[0] = BPK_RST_FLAG;
    }
    else if(st_type == ST_WATCHDOG)
    {
        //watchdog
        buf[0] = BPK_WDTRST_FLAG;
    }
    else if(st_type == ST_HARD_FAULT)
    {
        buf[0] = BPK_HARD_FAULT_RST_FLAG;
    }
    else
    {   //冷复位、其它
        buf[0] = 0x00;  
    }
    //1.关闭区域1的锁
//    BPK_KeyReadLock(BPK_KEY_REGION_0|BPK_KEY_REGION_1 , DISABLE);
    BPK_KeyWriteLock(BPK_KEY_REGION_1 , DISABLE);
//    BPK_KeyReadLock(BPK_KEY_REGION_1 , DISABLE);
    status = dev_dryice_waitBPKREADY();
    if(status != 0)
    {
        return DEVSTATUS_ERR_FAIL;
    }
    //设置BPK数据
    status = BPK_WriteKey(buf, 1, BPK_RST_OFFSET);
//    BPK_KeyReadLock(BPK_KEY_REGION_0|BPK_KEY_REGION_1 , ENABLE);
    BPK_KeyWriteLock(BPK_KEY_REGION_1 , ENABLE);
    if(status != SUCCESS)
    {
        return DEVSTATUS_ERR_FAIL; //错误不用加锁，直接返回错误
        
    }

    ret = dev_dryice_rstmod_get();
    if(ret != st_type)
    {
        return DEVSTATUS_ERR_FAIL;
    }
    else
    {
        return DEVSTATUS_SUCCESS;
    }
}


void testdryice(void)
{
    
    u8 testbuf[32];
    u8 zerobuf[32];
    u8 bbbuf[32];
    u8 i;
    s32 ret;
    u32 tamperstatus;

    memset(testbuf,0,sizeof(testbuf));
    memset(zerobuf,0,sizeof(zerobuf));
    memset(bbbuf,0xBB,sizeof(bbbuf));
    dev_dryice_readsecurekey(testbuf);
    
    if(memcmp(testbuf,zerobuf,32) == 0||memcmp(testbuf,bbbuf,32) == 0)
    {
        
        for(i = 0;i < 32;i++)
        {
            testbuf[i] = i;
        }
        
        
        ret = dev_dryice_writesecurekey(testbuf,32);
        if(ret == DEVSTATUS_SUCCESS)
        {
            dev_dryice_readsecurekey(testbuf);
        }
        
    }
    
    for(i = 0;i < 32;i++)
    {
        dev_debug_printf("%02x ",testbuf[i]);
    }

    while(1)
    {
        tamperstatus = dev_dryice_gettamperstatusonly();
        dev_debug_printf("\r\ntamperstatus = %08x %08x %08x %08x %08x\r\n",tamperstatus,SENSOR->SEN_EXT_TYPE,SENSOR->SEN_EXT_CFG,SENSOR->SEN_EN[0],SENSOR->SEN_EN[1]);
    }
    
    
}


